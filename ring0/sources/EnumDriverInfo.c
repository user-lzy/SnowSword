#pragma once

#include "EnumDriverInfo.h"
#include "ObjectInfo.h"

NTKERNELAPI NTSTATUS ObReferenceObjectByName(
	IN PUNICODE_STRING ObjectName,
	IN ULONG Attributes,
	IN PACCESS_STATE PassedAccessState OPTIONAL,
	IN ACCESS_MASK DesiredAccess OPTIONAL,
	IN POBJECT_TYPE ObjectType,
	IN KPROCESSOR_MODE AccessMode,
	IN OUT PVOID ParseContext OPTIONAL,
	OUT PVOID* Object
);

#define MAX_TARGET_DEVICES 10
#define MAP_POOL_TAG 'pamD'

// 全局设备附加映射表
typedef struct _DEVICE_ATTACHMENT_ENTRY {
    PDEVICE_OBJECT AttachedToDevice;  // 被附加的设备（key）
    PDEVICE_OBJECT AttacherDevice;    // 附加者的设备
    PDRIVER_OBJECT AttacherDriver;    // 附加者的驱动
    WCHAR AttacherDriverName[256];    // 附加者驱动名称
    struct _DEVICE_ATTACHMENT_ENTRY* Next;
} DEVICE_ATTACHMENT_ENTRY, * PDEVICE_ATTACHMENT_ENTRY;

// 全局映射表头和锁
PDEVICE_ATTACHMENT_ENTRY g_AttachmentMap = NULL;
KSPIN_LOCK g_AttachmentMapLock;
BOOLEAN g_MapInitialized = FALSE;

VOID GetDriverInfo(PDRIVER_OBJECT pDriverObject, PDRIVER_INFO pDriverInfo) {
	//PDRIVER_INFO pDriverInfo = (PDRIVER_INFO)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(DRIVER_INFO), 'pdio');
	if (!pDriverInfo) return;
	RtlZeroMemory(pDriverInfo, sizeof(DRIVER_INFO));
	pDriverInfo->DriverObjectAddr = pDriverObject;
	DbgPrint("DriverObjectAddr: 0x%p,TrueDriverObjectAddr: 0x%p", pDriverInfo->DriverObjectAddr, pDriverObject);
	pDriverInfo->DriverInitAddr = (PVOID)pDriverObject->DriverInit;
	DbgPrint("DriverInitAddr: 0x%p, TrueDriverInitAddr: 0x%p", pDriverInfo->DriverInitAddr, pDriverObject->DriverInit);
	pDriverInfo->DriverStartIoAddr = (PVOID)pDriverObject->DriverStartIo;
	DbgPrint("DriverStartIoAddr: 0x%p, TrueDriverStartIoAddr: 0x%p", pDriverInfo->DriverStartIoAddr, pDriverObject->DriverStartIo);
	pDriverInfo->FastIoDispatchAddr = (PVOID)pDriverObject->FastIoDispatch;
	DbgPrint("FastIoDispatchAddr: 0x%p, TrueFastIoDispatchAddr: 0x%p", pDriverInfo->FastIoDispatchAddr, pDriverObject->FastIoDispatch);
	pDriverInfo->DriverUnloadAddr = (PVOID)pDriverObject->DriverUnload;
	DbgPrint("DriverUnloadAddr: 0x%p, TrueDriverUnloadAddr: 0x%p", pDriverInfo->DriverUnloadAddr, pDriverObject->DriverUnload);
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		pDriverInfo->MajorFunctionAddr[i] = (PVOID)pDriverObject->MajorFunction[i];
		DbgPrint("MajorFunctionAddr[%d]: 0x%p, TrueMajorFunctionAddr[%d]: 0x%p", i, pDriverInfo->MajorFunctionAddr[i], i, pDriverObject->MajorFunction[i]);
	}
}

NTSTATUS EnumAttachDevices(
	_In_  PDRIVER_OBJECT TargetDriver,
	_Out_ PATTACH_DEVICE_INFO AttachInfo
)
{
	if (!TargetDriver || !AttachInfo)
		return STATUS_INVALID_PARAMETER;

	RtlZeroMemory(AttachInfo, sizeof(ATTACH_DEVICE_INFO));

	// 填充驱动名
	if (TargetDriver->DriverName.Length > 0)
	{
		RtlStringCbCopyUnicodeString(
			AttachInfo->DriverName,
			sizeof(AttachInfo->DriverName),
			&TargetDriver->DriverName
		);
	}

	ULONG Index = 0;
	PDEVICE_OBJECT DeviceObject = TargetDriver->DeviceObject;

	// 枚举该驱动创建的所有 DeviceObject
	while (DeviceObject)
	{
		// 枚举每个设备的附加链
		PDEVICE_OBJECT AttachedDevice = DeviceObject->AttachedDevice;

		while (AttachedDevice)
		{
			if (Index >= 10)
				goto _Exit;

			PATTACHED_DEVICE_NODE Node = &AttachInfo->Devices[Index];

			Node->DeviceObject = AttachedDevice;
			Node->DriverObject = AttachedDevice->DriverObject;

			// 复制驱动名
			if (AttachedDevice->DriverObject->DriverName.Length > 0)
			{
				RtlStringCbCopyUnicodeString(
					Node->DriverName,
					sizeof(Node->DriverName),
					&AttachedDevice->DriverObject->DriverName
				);
			}

			// 复制驱动路径（DriverSection->FullImageName）
			if (AttachedDevice->DriverObject->DriverSection)
			{
				PLDR_DATA_TABLE_ENTRY LdrEntry =
					(PLDR_DATA_TABLE_ENTRY)AttachedDevice->DriverObject->DriverSection;

				if (LdrEntry && LdrEntry->FullDllName.Length > 0)
				{
					RtlStringCbCopyUnicodeString(
						Node->DriverPath,
						sizeof(Node->DriverPath),
						&LdrEntry->FullDllName
					);
				}

			}

			Index++;

			AttachedDevice = AttachedDevice->AttachedDevice; // 下一个附加设备
		}

		DeviceObject = DeviceObject->NextDevice; // 下一个设备
	}

_Exit:
	AttachInfo->TotalDevices = Index;

	return STATUS_SUCCESS;
}

// 辅助函数：安全获取对象类型（避免直接访问对象头）
POBJECT_TYPE GetObjectType(_In_ PVOID Object) {
	if (Object == NULL) return NULL;
	// ObGetObjectType 是内核导出函数，用于获取对象的类型
	return ObGetObjectType(Object);
}

// 打开指定路径的目录（如 L"\\FileSystem"）
NTSTATUS OpenDirectoryObject(PCWSTR DirPath, POBJECT_DIRECTORY* OutDirObj) {
	UNICODE_STRING dirName;
	OBJECT_ATTRIBUTES objAttr;
	HANDLE dirHandle;
	NTSTATUS status;

	RtlInitUnicodeString(&dirName, DirPath);
	InitializeObjectAttributes(&objAttr, &dirName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

	// 打开目录，获取句柄
	status = ZwOpenDirectoryObject(&dirHandle, DIRECTORY_QUERY, &objAttr);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ZwOpenDirectoryObject failed!status=%X", status);
		return status;
	}
	
	// 将句柄转换为 OBJECT_DIRECTORY 指针（内核态）
	status = ObReferenceObjectByHandle(
		dirHandle,
		DIRECTORY_QUERY,
		NULL,
		KernelMode,
		(PVOID*)OutDirObj,
		NULL
	);

	//打印对象类型以验证
	POBJECT_TYPE objType = GetObjectType(*OutDirObj);
	if (objType) {
		//DbgPrint("Opened Directory Object Type: %wZ", &objType->Name);
	} else {
		//DbgPrint("Failed to get object type.");
	}
	ZwClose(dirHandle); // 关闭句柄，保留对象引用

	if (!NT_SUCCESS(status)) DbgPrint("ZwOpenDirectoryObject failed!status=%X", status);
	return status;
}

// 通用枚举函数：遍历指定目录下的所有驱动对象
PDRIVER_OBJECT GetDirectoryDrivers(POBJECT_DIRECTORY DirObj, PVOID BaseAddress) {
	if (!DirObj) return NULL;

	POBJECT_DIRECTORY_ENTRY dirEntry = NULL;
	POBJECT_DIRECTORY_ENTRY subEntry = NULL;
	PDRIVER_OBJECT drvObj = NULL;
	PDRIVER_OBJECT resultDrvObj = NULL; // 存储最终要返回的对象

	for (int i = 0; i < 37; i++) {
		dirEntry = DirObj->HashBuckets[i];
		if (!dirEntry) continue;

		subEntry = dirEntry;
		while (subEntry) {
			// 步骤1：对所有对象先加引用计数，确保访问安全
			if (subEntry->Object == NULL) {
				subEntry = subEntry->ChainLink;
				continue;
			}
			// 对当前对象增加引用计数（关键：无论是否为驱动对象）
			ObReferenceObject(subEntry->Object);

			// 步骤2：判断是否为驱动对象
			drvObj = (PDRIVER_OBJECT)subEntry->Object;
			if (GetObjectType(drvObj) == *IoDriverObjectType) {
				PLDR_DATA_TABLE_ENTRY ldrEntry = (PLDR_DATA_TABLE_ENTRY)drvObj->DriverSection;
				if (ldrEntry && ldrEntry->DllBase == BaseAddress) {
					//DbgPrint("Found Driver: %wZ at 0x%p", &ldrEntry->BaseDllName, drvObj);
					resultDrvObj = drvObj; // 匹配时暂存对象，不释放引用
				}
				else {
					// 未匹配，立即释放引用计数
					ObDereferenceObject(drvObj);
				}
			}
			else {
				// 不是驱动对象，立即释放引用计数
				ObDereferenceObject(subEntry->Object);
			}

			subEntry = subEntry->ChainLink;
		}
	}

	// 最终返回匹配的对象（仍带有1次引用计数，由调用者释放）
	return resultDrvObj;
}

PVOID GetDriverObjectByBaseAddress(PVOID BaseAddress) {
	NTSTATUS status;
	OBJECT_DIRECTORY* dirObj = NULL;
	PDRIVER_OBJECT drvObj = NULL;
	// 打开目录对象
	status = OpenDirectoryObject(L"\\Driver", &dirObj);
	if (!NT_SUCCESS(status) || !dirObj) {
		DbgPrint("OpenDirectoryObject failed!status=%X", status);
		return NULL;
	}
	// 枚举目录下的驱动对象
	drvObj = GetDirectoryDrivers(dirObj, BaseAddress);
	// 释放对目录对象的引用
	ObDereferenceObject(dirObj);
	if (drvObj) return (PVOID)drvObj;

	// 打开目录对象
	status = OpenDirectoryObject(L"\\FileSystem", &dirObj);
	if (!NT_SUCCESS(status) || !dirObj) {
		DbgPrint("OpenDirectoryObject failed!status=%X", status);
		return NULL;
	}
	// 枚举目录下的驱动对象
	drvObj = GetDirectoryDrivers(dirObj, BaseAddress);
	// 释放对目录对象的引用
	ObDereferenceObject(dirObj);
	if (drvObj) return (PVOID)drvObj;

	return drvObj;
}

// 辅助函数：获取驱动对象名称
VOID GetDriverObjectName(
    _In_ PDRIVER_OBJECT DriverObject,
    _Out_writes_(256) WCHAR* NameBuffer
)
{
    if (DriverObject->DriverName.Length > 0 && DriverObject->DriverName.Buffer) {
        USHORT copyLength = min(DriverObject->DriverName.Length, 255 * sizeof(WCHAR));
        RtlCopyMemory(NameBuffer, DriverObject->DriverName.Buffer, copyLength);
        NameBuffer[copyLength / sizeof(WCHAR)] = L'\0';
    }
    else {
        wcscpy_s(NameBuffer, 256, L"Unknown");
    }
}

// 释放全局映射表
VOID FreeGlobalDeviceAttachmentMap()
{
    KIRQL oldIrql;
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);

    PDEVICE_ATTACHMENT_ENTRY current = g_AttachmentMap;
    g_AttachmentMap = NULL;

    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

    // 释放所有条目（无锁，因为已断开链表）
    while (current) {
        PDEVICE_ATTACHMENT_ENTRY next = current->Next;
        ExFreePoolWithTag(current, MAP_POOL_TAG);
        current = next;
    }
}

// 构建全局设备附加映射表
NTSTATUS BuildGlobalDeviceAttachmentMap()
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING driverDirPath;
    HANDLE hDriverDir = NULL;
    OBJECT_ATTRIBUTES oa;
    PDIRECTORY_BASIC_INFORMATION pBuffer = NULL;
    ULONG ulContext = 0;
    ULONG ulRet = 0;
    ULONG ulLength = 0x800;
    KIRQL oldIrql;

    // 初始化自旋锁（仅一次）
    if (!g_MapInitialized) {
        KeInitializeSpinLock(&g_AttachmentMapLock);
        g_MapInitialized = TRUE;
    }

    // 清空现有映射表
    FreeGlobalDeviceAttachmentMap();

    // 枚举 \Driver 和 \FileSystem 两个目录
    PCWSTR dirPaths[] = { L"\\Driver", L"\\FileSystem" };
    ULONG dirCount = sizeof(dirPaths) / sizeof(dirPaths[0]);

    for (ULONG dirIdx = 0; dirIdx < dirCount; dirIdx++) {
        RtlInitUnicodeString(&driverDirPath, dirPaths[dirIdx]);
        InitializeObjectAttributes(&oa, &driverDirPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

        status = ZwOpenDirectoryObject(&hDriverDir, DIRECTORY_QUERY, &oa);
        if (!NT_SUCCESS(status)) {
            DbgPrint("ZwOpenDirectoryObject failed for %ws! status=%X", dirPaths[dirIdx], status);
            continue;
        }

        ulContext = 0;
        do {
            if (pBuffer) {
                ExFreePoolWithTag(pBuffer, MAP_POOL_TAG);
                pBuffer = NULL;
            }

            ulLength = ulLength * 2;
            pBuffer = (PDIRECTORY_BASIC_INFORMATION)ExAllocatePool2(
                POOL_FLAG_NON_PAGED,
                ulLength,
                MAP_POOL_TAG
            );
            if (!pBuffer) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            status = ZwQueryDirectoryObject(
                hDriverDir,
                pBuffer,
                ulLength,
                FALSE,
                TRUE,
                &ulContext,
                &ulRet
            );
        } while (status == STATUS_MORE_ENTRIES || status == STATUS_BUFFER_TOO_SMALL);

        if (status != STATUS_SUCCESS) {
            DbgPrint("ZwQueryDirectoryObject failed! status=%X", status);
            if (pBuffer) {
                ExFreePoolWithTag(pBuffer, MAP_POOL_TAG);
                pBuffer = NULL;
            }
            ZwClose(hDriverDir);
            continue;
        }

        PDIRECTORY_BASIC_INFORMATION pBuffer2 = pBuffer;
        while (pBuffer2->ObjectName.Length != 0 && pBuffer2->ObjectType.Length != 0) {
            UNICODE_STRING driverPath;
            WCHAR driverPathBuf[256];
            RtlInitEmptyUnicodeString(&driverPath, driverPathBuf, sizeof(driverPathBuf));
            RtlAppendUnicodeToString(&driverPath, dirPaths[dirIdx]);
            RtlAppendUnicodeToString(&driverPath, L"\\");
            RtlAppendUnicodeStringToString(&driverPath, &pBuffer2->ObjectName);

            PDRIVER_OBJECT driverObject = NULL;
            status = ObReferenceObjectByName(
                &driverPath,
                OBJ_CASE_INSENSITIVE,
                NULL,
                0,
                *IoDriverObjectType,
                KernelMode,
                NULL,
                (PVOID*)&driverObject
            );

            if (NT_SUCCESS(status)) {
                // 遍历该驱动的所有设备
                PDEVICE_OBJECT currentDevice = driverObject->DeviceObject;
                while (currentDevice != NULL) {
                    // 遍历附加链
                    PDEVICE_OBJECT attachedDevice = currentDevice->AttachedDevice;
                    while (attachedDevice != NULL) {
                        // 添加到全局映射表
                        PDEVICE_ATTACHMENT_ENTRY newEntry =
                            (PDEVICE_ATTACHMENT_ENTRY)ExAllocatePool2(
                                POOL_FLAG_NON_PAGED,
                                sizeof(DEVICE_ATTACHMENT_ENTRY),
                                MAP_POOL_TAG
                            );

                        if (newEntry) {
                            RtlZeroMemory(newEntry, sizeof(DEVICE_ATTACHMENT_ENTRY));
                            newEntry->AttachedToDevice = currentDevice;
                            newEntry->AttacherDevice = attachedDevice;

                            // 获取附加设备的驱动对象
                            PDRIVER_OBJECT attacherDriver = attachedDevice->DriverObject;
                            newEntry->AttacherDriver = attacherDriver;
                            GetDriverObjectName(attacherDriver, newEntry->AttacherDriverName);

                            // 插入到链表（加锁）
                            KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
                            newEntry->Next = g_AttachmentMap;
                            g_AttachmentMap = newEntry;
                            KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);
                        }

                        attachedDevice = attachedDevice->AttachedDevice;
                    }
                    currentDevice = currentDevice->NextDevice;
                }
                ObDereferenceObject(driverObject);
            }

            pBuffer2++;
        }

        if (pBuffer) {
            ExFreePoolWithTag(pBuffer, MAP_POOL_TAG);
            pBuffer = NULL;
        }
        ZwClose(hDriverDir);
        hDriverDir = NULL;
    }

    return STATUS_SUCCESS;
}

// 主函数：枚举目标驱动的过滤链
NTSTATUS EnumFilterChains(
    _In_ PDRIVER_OBJECT TargetDriver,
    _Out_ PATTACH_DEVICE_INFO AttachInfo
)
{
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL oldIrql;

    if (!TargetDriver || !AttachInfo) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(AttachInfo, sizeof(ATTACH_DEVICE_INFO));

    // 构建全局映射表
    status = BuildGlobalDeviceAttachmentMap();
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // 获取目标驱动名称
    GetDriverObjectName(TargetDriver, AttachInfo->DriverName);
    DbgPrint("DriverName:%ws", AttachInfo->DriverName);

    // 遍历目标驱动的所有设备
    PDEVICE_OBJECT currentDevice = TargetDriver->DeviceObject;
    ULONG deviceIndex = 0;
    ULONG attachedCount = 0;

    while (currentDevice != NULL && attachedCount < MAX_TARGET_DEVICES) {
        // 查询谁附加到了这个设备上
        //PDEVICE_OBJECT attachedByList[10] = { 0 };
        //ULONG attachedCount = 0;

        KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);

        PDEVICE_ATTACHMENT_ENTRY entry = g_AttachmentMap;
        while (entry && attachedCount < 10) {
            if (entry->AttachedToDevice == currentDevice) {
                DbgPrint("DeviceObject:0x%p, DriverObject:0x%p, DriverName:%ws", 
                    currentDevice, entry->AttacherDriver, entry->AttacherDriverName);
                AttachInfo->Devices[attachedCount].DeviceObject = currentDevice;
                AttachInfo->Devices[attachedCount].DriverObject = entry->AttacherDriver;
                wcscpy_s(AttachInfo->Devices[attachedCount].DriverName,
                    260,
                    entry->AttacherDriverName);
                attachedCount++;
            }
            entry = entry->Next;
        }

        KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

        //deviceIndex++;
        currentDevice = currentDevice->NextDevice;
    }

    //AttachInfo->TotalDevices = deviceIndex;
    AttachInfo->TotalDevices = attachedCount;

    // 清理全局映射表
    FreeGlobalDeviceAttachmentMap();

    return STATUS_SUCCESS;
}