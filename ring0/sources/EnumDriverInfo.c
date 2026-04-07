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

// 全局映射表头和锁
KSPIN_LOCK g_AttachmentMapLock;

#define MAX_TRACKED_DEVICES 2048
PDEVICE_OBJECT g_AddedDevices[MAX_TRACKED_DEVICES] = { NULL };
ULONG g_AddedDeviceCount = 0;

// 在函数开始处定义
#define MAX_RECORDED_DEVICES 4096   // 足够大，实际设备数远小于此
#define MAX_PATH 260

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
    // 1. 清空缓冲区，避免残留数据
    RtlZeroMemory(NameBuffer, 256 * sizeof(WCHAR));

    if (DriverObject != NULL && DriverObject->DriverName.Length > 0 && DriverObject->DriverName.Buffer) {
        // 2. 按WCHAR计算长度，避免字节/字符混淆
        ULONG charCount = DriverObject->DriverName.Length / sizeof(WCHAR);
        ULONG copyCount = min(charCount, 255); // 留1位给\0
        RtlCopyMemory(NameBuffer, DriverObject->DriverName.Buffer, copyCount * sizeof(WCHAR));
        // 3. 强制终止字符串
        NameBuffer[copyCount] = L'\0';
    }
    else {
        wcscpy_s(NameBuffer, 256, L"Unknown");
    }
}

// 辅助函数：获取设备对象的名称（如 \Device\Harddisk0\DR0）
NTSTATUS GetDeviceObjectName(
    _In_ PDEVICE_OBJECT DeviceObject,
    _Out_writes_(NameBufferSize) PWCHAR NameBuffer,
    _In_ ULONG NameBufferSize
)
{
    NTSTATUS status;
    POBJECT_NAME_INFORMATION nameInfo = NULL;
    ULONG returnLength = 0;

    // 空指针检查
    if (DeviceObject == NULL) {
        if (NameBuffer && NameBufferSize >= sizeof(WCHAR)) {
            NameBuffer[0] = L'\0';  // 设置为空字符串
        }
        return STATUS_INVALID_PARAMETER;
    }

    // 查询所需大小
    status = ObQueryNameString(DeviceObject, NULL, 0, &returnLength);
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        return status;
    }

    // 分配缓冲区
    nameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        returnLength,
        MAP_POOL_TAG
    );
    if (!nameInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 获取名称
    status = ObQueryNameString(DeviceObject, nameInfo, returnLength, &returnLength);
    if (NT_SUCCESS(status)) {
        if (nameInfo->Name.Length < NameBufferSize * sizeof(WCHAR)) {
            RtlCopyMemory(NameBuffer, nameInfo->Name.Buffer, nameInfo->Name.Length);
            NameBuffer[nameInfo->Name.Length / sizeof(WCHAR)] = L'\0';
        }
        else {
            // 名称太长，使用地址作为标识
            RtlStringCchPrintfW(NameBuffer, NameBufferSize, L"\\Device\\%p", DeviceObject);
        }
        ExFreePoolWithTag(nameInfo, MAP_POOL_TAG);
    }
    else {
        // 无法获取名称，使用地址作为标识
        RtlStringCchPrintfW(NameBuffer, NameBufferSize, L"\\Device\\%p", DeviceObject);
        if (nameInfo) {
            ExFreePoolWithTag(nameInfo, MAP_POOL_TAG);
        }
    }

    return STATUS_SUCCESS;
}

// 通过映射表找到原始设备
PDEVICE_OBJECT FindBaseDeviceByFilter(PDEVICE_OBJECT filterDevice)
{
    KIRQL oldIrql;
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);

    PDEVICE_ATTACHMENT_ENTRY entry = g_AttachmentMap;
    while (entry) {
        if (entry->AttacherDevice == filterDevice && entry->AttachedToDevice != filterDevice) {
            KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);
            return entry->AttachedToDevice;  // 找到原始设备
        }
        entry = entry->Next;
    }

    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);
    return NULL;
}

// 获取驱动文件完整路径
VOID GetDriverFilePath(PDRIVER_OBJECT DriverObject, WCHAR* PathBuffer, SIZE_T BufferSize)
{
    if (!DriverObject || !DriverObject->DriverSection)
    {
        PathBuffer[0] = L'\0';
        return;
    }

    __try
    {
        // DriverSection 指向 LDR_DATA_TABLE_ENTRY 结构
        PLDR_DATA_TABLE_ENTRY ldrEntry = (PLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;

        if (ldrEntry->FullDllName.Buffer && ldrEntry->FullDllName.Length > 0)
        {
            RtlStringCbCopyNW(PathBuffer, BufferSize,
                ldrEntry->FullDllName.Buffer, ldrEntry->FullDllName.Length);
        }
        else
        {
            PathBuffer[0] = L'\0';
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        PathBuffer[0] = L'\0';
    }
}

// ============================================================================
// 初始化
// ============================================================================
NTSTATUS InitAttachmentMap(void)
{
    if (!g_MapInitialized) {
        KeInitializeSpinLock(&g_AttachmentMapLock);
        g_MapInitialized = TRUE;
        g_GlobalMapBuilt = FALSE;
        g_AttachmentMap = NULL;
    }
    return STATUS_SUCCESS;
}

// ============================================================================
// 释放全局映射
// ============================================================================
VOID FreeGlobalDeviceAttachmentMap()
{
    KIRQL oldIrql;
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);

    PDEVICE_ATTACHMENT_ENTRY pEntry = g_AttachmentMap;
    while (pEntry) {
        PDEVICE_ATTACHMENT_ENTRY pNext = pEntry->Next;
        ExFreePoolWithTag(pEntry, MAP_POOL_TAG);
        pEntry = pNext;
    }
    g_AttachmentMap = NULL;
    g_GlobalMapBuilt = FALSE;

    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);
}

// ============================================================================
// 清理
// ============================================================================
void CleanupAttachmentMap(void)
{
    FreeGlobalDeviceAttachmentMap();
    g_GlobalMapBuilt = FALSE;
}

// ============================================================================
// 修复版：构建全局设备附加映射（带调试输出 + 修复fltmgr遍历）
// ============================================================================
NTSTATUS BuildGlobalDeviceAttachmentMap(void)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING driverDirPath;
    HANDLE hDriverDir = NULL;
    OBJECT_ATTRIBUTES oa;
    KIRQL oldIrql;

    if (g_GlobalMapBuilt && g_AttachmentMap != NULL) {
        DbgPrint("[AttachMap] 全局映射已缓存，复用\n");
        return STATUS_SUCCESS;
    }

    if (!g_MapInitialized) {
        KeInitializeSpinLock(&g_AttachmentMapLock);
        g_MapInitialized = TRUE;
    }

    RtlZeroMemory(g_AddedDevices, sizeof(g_AddedDevices));
    g_AddedDeviceCount = 0;
    FreeGlobalDeviceAttachmentMap();

    // 【调试】枚举目录：\Driver 和 \FileSystem
    PCWSTR dirPaths[] = { L"\\Driver", L"\\FileSystem" };
    ULONG dirCount = sizeof(dirPaths) / sizeof(dirPaths[0]);

    for (ULONG dirIdx = 0; dirIdx < dirCount; dirIdx++)
    {
        RtlInitUnicodeString(&driverDirPath, dirPaths[dirIdx]);
        DbgPrint("[AttachMap] 打开目录: %ws\n", dirPaths[dirIdx]); // 调试输出

        InitializeObjectAttributes(&oa, &driverDirPath,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        status = ZwOpenDirectoryObject(&hDriverDir, DIRECTORY_QUERY, &oa);
        if (!NT_SUCCESS(status)) {
            DbgPrint("[AttachMap] 打开目录失败: %ws, 状态: %08X\n", dirPaths[dirIdx], status);
            continue;
        }

        PVOID buffer = NULL;
        ULONG bufferSize = 0;
        BOOLEAN restartScan = TRUE;
        ULONG context = 0;

        while (TRUE)
        {
            status = ZwQueryDirectoryObject(hDriverDir, NULL, 0, TRUE, restartScan, &context, &bufferSize);
            if (status != STATUS_BUFFER_TOO_SMALL) {
                if (status == STATUS_NO_MORE_ENTRIES) status = STATUS_SUCCESS;
                break;
            }

            if (buffer) ExFreePoolWithTag(buffer, MAP_POOL_TAG);
            bufferSize = max(bufferSize, sizeof(DIRECTORY_BASIC_INFORMATION) + 0x200);
            buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, MAP_POOL_TAG);
            if (!buffer) { status = STATUS_INSUFFICIENT_RESOURCES; break; }

            status = ZwQueryDirectoryObject(hDriverDir, buffer, bufferSize, TRUE, restartScan, &context, &bufferSize);
            restartScan = FALSE;
            if (status == STATUS_NO_MORE_ENTRIES || !NT_SUCCESS(status)) break;

            PDIRECTORY_BASIC_INFORMATION pEntry = (PDIRECTORY_BASIC_INFORMATION)buffer;
            if (pEntry->ObjectName.Length == 0 || pEntry->ObjectType.Length == 0) continue;

            UNICODE_STRING uDriver, uFileSystem;
            RtlInitUnicodeString(&uDriver, L"Driver");
            RtlInitUnicodeString(&uFileSystem, L"FileSystem");
            if (!RtlEqualUnicodeString(&pEntry->ObjectType, &uDriver, TRUE) &&
                !RtlEqualUnicodeString(&pEntry->ObjectType, &uFileSystem, TRUE))
                continue;

            // 【调试】输出遍历到的驱动名
            DbgPrint("[AttachMap] 找到驱动对象: %wZ\n", &pEntry->ObjectName);

            WCHAR szDriverPath[MAX_PATH] = { 0 };
            UNICODE_STRING usDriverPath;
            RtlInitEmptyUnicodeString(&usDriverPath, szDriverPath, MAX_PATH);
            RtlAppendUnicodeStringToString(&usDriverPath, &driverDirPath);
            RtlAppendUnicodeToString(&usDriverPath, L"\\");
            RtlAppendUnicodeStringToString(&usDriverPath, &pEntry->ObjectName);

            PDRIVER_OBJECT pDriverObj = NULL;
            status = ObReferenceObjectByName(&usDriverPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&pDriverObj);
            if (!NT_SUCCESS(status)) {
                DbgPrint("[AttachMap] 引用驱动失败: %wZ, 状态: %08X\n", &usDriverPath, status);
                continue;
            }

            PDEVICE_OBJECT pDevObj = pDriverObj->DeviceObject;
            while (pDevObj)
            {
                // ==============================================
                // 【修复1】标记根设备 = 原始设备
                // ==============================================
                PDEVICE_ATTACHMENT_ENTRY pRootEntry = ExAllocatePool2(POOL_FLAG_NON_PAGED,
                    sizeof(DEVICE_ATTACHMENT_ENTRY), MAP_POOL_TAG);
                if (pRootEntry)
                {
                    RtlZeroMemory(pRootEntry, sizeof(DEVICE_ATTACHMENT_ENTRY));
                    pRootEntry->AttachedToDevice = NULL;
                    pRootEntry->ParentDeviceObject = NULL;
                    pRootEntry->AttacherDevice = pDevObj;
                    pRootEntry->AttacherDriver = pDriverObj;
                    pRootEntry->Level = 0;

                    GetDriverObjectName(pDriverObj, pRootEntry->AttacherDriverName);
                    GetDeviceObjectName(pDevObj, pRootEntry->DeviceObjectName, MAX_PATH);

                    // 【调试】输出根设备信息
                    DbgPrint("[AttachMap] [根设备] 驱动: %ws | 设备: %ws | DevObj: %p\n",
                        pRootEntry->AttacherDriverName, pRootEntry->DeviceObjectName, pDevObj);

                    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
                    pRootEntry->Next = g_AttachmentMap;
                    g_AttachmentMap = pRootEntry;
                    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);
                }

                // ==============================================
                // 【修复2】遍历过滤链 + 修复break→continue
                // ==============================================
                PDEVICE_OBJECT pCurrent = pDevObj;
                PDEVICE_OBJECT pParent = pDevObj;
                ULONG level = 1;

                while (pCurrent->AttachedDevice && level < MAX_FILTER_DEPTH)
                {
                    pCurrent = pCurrent->AttachedDevice;

                    // 去重：跳过重复，不终止遍历
                    BOOLEAN isDup = FALSE;
                    for (ULONG i = 0; i < g_AddedDeviceCount; i++) {
                        if (g_AddedDevices[i] == pCurrent) { isDup = TRUE; break; }
                    }
                    if (isDup) {
                        DbgPrint("[AttachMap] 跳过重复设备: %p\n", pCurrent);
                        continue; // 【修复】break → continue，核心！
                    }

                    // 录入过滤设备（fltmgr在这里）
                    PDEVICE_ATTACHMENT_ENTRY pFilterEntry = ExAllocatePool2(POOL_FLAG_NON_PAGED,
                        sizeof(DEVICE_ATTACHMENT_ENTRY), MAP_POOL_TAG);
                    if (pFilterEntry)
                    {
                        RtlZeroMemory(pFilterEntry, sizeof(DEVICE_ATTACHMENT_ENTRY));
                        pFilterEntry->AttachedToDevice = pParent;
                        pFilterEntry->ParentDeviceObject = pParent;
                        pFilterEntry->AttacherDevice = pCurrent;
                        pFilterEntry->AttacherDriver = pCurrent->DriverObject;
                        pFilterEntry->Level = level;

                        GetDriverObjectName(pCurrent->DriverObject, pFilterEntry->AttacherDriverName);
                        GetDeviceObjectName(pCurrent, pFilterEntry->DeviceObjectName, MAX_PATH);

                        // 【调试】输出过滤设备（fltmgr会在这里打印）
                        DbgPrint("[AttachMap] [过滤设备] 驱动: %ws | 设备: %ws | DevObj: %p | 父设备: %p\n",
                            pFilterEntry->AttacherDriverName, pFilterEntry->DeviceObjectName, pCurrent, pParent);

                        KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
                        pFilterEntry->Next = g_AttachmentMap;
                        g_AttachmentMap = pFilterEntry;
                        KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

                        if (g_AddedDeviceCount < MAX_TRACKED_DEVICES)
                            g_AddedDevices[g_AddedDeviceCount++] = pCurrent;
                    }

                    pParent = pCurrent;
                    level++;
                }

                pDevObj = pDevObj->NextDevice;
            }

            ObDereferenceObject(pDriverObj);
        }

        if (buffer) ExFreePoolWithTag(buffer, MAP_POOL_TAG);
        ZwClose(hDriverDir);
        hDriverDir = NULL;
    }

    if (NT_SUCCESS(status)) {
        g_GlobalMapBuilt = TRUE;
        ULONG count = 0;
        PDEVICE_ATTACHMENT_ENTRY p = g_AttachmentMap;
        while (p) { count++; p = p->Next; }
        DbgPrint("[AttachMap] 构建完成！总节点数: %lu\n", count);
    }

    return status;
}

NTSTATUS CalculateGlobalDataSize(OUT PULONG pTotalSize)
{
    //NTSTATUS status = STATUS_SUCCESS;
    KIRQL oldIrql;
    ULONG driverCount = 0;
    ULONG totalDeviceNodes = 0;
    PDEVICE_ATTACHMENT_ENTRY pEntry;

    // 获取驱动总数和设备节点总数
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
    pEntry = g_AttachmentMap;
    while (pEntry) {
        totalDeviceNodes++;
        // 检查驱动是否已统计过
        PDRIVER_OBJECT currentDriver = pEntry->AttacherDriver;
        BOOLEAN found = FALSE;
        for (PDEVICE_ATTACHMENT_ENTRY tmp = g_AttachmentMap; tmp != pEntry; tmp = tmp->Next) {
            if (tmp->AttacherDriver == currentDriver) {
                found = TRUE;
                break;
            }
        }
        if (!found) driverCount++;
        pEntry = pEntry->Next;
    }
    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

    // 计算总大小
    *pTotalSize = sizeof(GLOBAL_ATTACH_INFO) +
        driverCount * sizeof(DRIVER_ATTACH_INFO) +
        totalDeviceNodes * sizeof(ATTACHED_DEVICE_NODE);
    return STATUS_SUCCESS;
}

// ============================================================================
// 【崩溃修复版】FillGlobalData - 修正逻辑顺序，数据完整合法，R3不再崩溃
// ============================================================================
NTSTATUS FillGlobalData(PVOID OutputBuffer, ULONG OutputBufferSize, PULONG pBytesWritten)
{
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL oldIrql;
    PDEVICE_ATTACHMENT_ENTRY pEntry;
    ULONG totalEntries = 0;
    ULONG i, j, d;

    typedef struct _TEMP_NODE {
        PDEVICE_OBJECT AttachedToDevice;
        PDEVICE_OBJECT AttacherDevice;
        PDEVICE_OBJECT ParentDeviceObject;
        PDRIVER_OBJECT AttacherDriver;
        BOOLEAN IsOriginalDevice;
        WCHAR DriverName[260];
        WCHAR DriverPath[260];
        WCHAR DeviceName[260];
        ULONG Level;
    } TEMP_NODE, * PTEMP_NODE;

    typedef struct _DRV_INFO {
        PDRIVER_OBJECT DriverObject;
        ULONG DeviceCount;
        WCHAR DriverName[260];
        WCHAR DriverPath[260];
    } DRV_INFO, * PDRV_INFO;

    typedef struct _DRV_NODES {
        PULONG Indices;
        ULONG Count;
    } DRV_NODES, * PDRV_NODES;

    PTEMP_NODE tempNodes = NULL;
    PDRV_INFO drvInfos = NULL;
    PDRV_NODES drvNodes = NULL;
    PULONG counters = NULL;

    // ==============================================
    // 【修复1】第一步：统计总节点数 (必须最先执行！)
    // ==============================================
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
    pEntry = g_AttachmentMap;
    while (pEntry)
    {
        totalEntries++;
        pEntry = pEntry->Next;
    }
    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

    // 无节点直接返回
    if (totalEntries == 0) {
        if (pBytesWritten) *pBytesWritten = 0;
        return STATUS_SUCCESS;
    }

    // ==============================================
    // 【修复2】第二步：分配临时节点内存 (统计后再分配！)
    // ==============================================
    tempNodes = ExAllocatePool2(POOL_FLAG_NON_PAGED, totalEntries * sizeof(TEMP_NODE), MAP_POOL_TAG);
    if (!tempNodes) { status = STATUS_INSUFFICIENT_RESOURCES; goto cleanup; }
    RtlZeroMemory(tempNodes, totalEntries * sizeof(TEMP_NODE));

    // ==============================================
    // 【修复3】第三步：一次性填充数据 (删除重复遍历！)
    // 正确设置 IsOriginalDevice (父设备=NULL → 原始设备)
    // ==============================================
    KeAcquireSpinLock(&g_AttachmentMapLock, &oldIrql);
    pEntry = g_AttachmentMap;
    i = 0;
    while (pEntry && i < totalEntries)
    {
        tempNodes[i].AttachedToDevice = pEntry->AttachedToDevice;
        tempNodes[i].AttacherDevice = pEntry->AttacherDevice;
        tempNodes[i].ParentDeviceObject = pEntry->ParentDeviceObject;
        tempNodes[i].AttacherDriver = pEntry->AttacherDriver;
        // 核心修复：仅这里设置一次，正确标记原始设备
        tempNodes[i].IsOriginalDevice = (pEntry->ParentDeviceObject == NULL);
        i++;
        pEntry = pEntry->Next;
    }
    KeReleaseSpinLock(&g_AttachmentMapLock, oldIrql);

    // 填充名称、路径
    for (i = 0; i < totalEntries; i++) {
        PTEMP_NODE pNode = &tempNodes[i];
        GetDriverObjectName(pNode->AttacherDriver, pNode->DriverName);
        GetDriverFilePath(pNode->AttacherDriver, pNode->DriverPath, RTL_NUMBER_OF(pNode->DriverPath));
        GetDeviceObjectName(pNode->AttacherDevice, pNode->DeviceName, RTL_NUMBER_OF(pNode->DeviceName));
        pNode->Level = pNode->Level;
    }

    // ==============================================
    // 驱动去重 (原有逻辑不变，完全正确)
    // ==============================================
    ULONG drvCount = 0;
    for (i = 0; i < totalEntries; i++) {
        PDRIVER_OBJECT currentDrv = tempNodes[i].AttacherDriver;
        BOOLEAN found = FALSE;

        for (j = 0; j < drvCount; j++) {
            if (drvInfos[j].DriverObject == currentDrv) {
                drvInfos[j].DeviceCount++;
                found = TRUE;
                break;
            }
        }
        if (!found) {
            PDRV_INFO newInfos = ExAllocatePool2(POOL_FLAG_NON_PAGED, (drvCount + 1) * sizeof(DRV_INFO), MAP_POOL_TAG);
            if (!newInfos) { status = STATUS_INSUFFICIENT_RESOURCES; goto cleanup; }
            if (drvInfos) {
                RtlCopyMemory(newInfos, drvInfos, drvCount * sizeof(DRV_INFO));
                ExFreePoolWithTag(drvInfos, MAP_POOL_TAG);
            }
            drvInfos = newInfos;
            drvInfos[drvCount].DriverObject = currentDrv;
            drvInfos[drvCount].DeviceCount = 1;
            RtlCopyMemory(drvInfos[drvCount].DriverName, tempNodes[i].DriverName, sizeof(drvInfos[drvCount].DriverName));
            RtlCopyMemory(drvInfos[drvCount].DriverPath, tempNodes[i].DriverPath, sizeof(drvInfos[drvCount].DriverPath));
            drvCount++;
        }
    }

    // 分配驱动节点索引
    drvNodes = ExAllocatePool2(POOL_FLAG_NON_PAGED, drvCount * sizeof(DRV_NODES), MAP_POOL_TAG);
    if (!drvNodes) { status = STATUS_INSUFFICIENT_RESOURCES; goto cleanup; }
    RtlZeroMemory(drvNodes, drvCount * sizeof(DRV_NODES));

    for (d = 0; d < drvCount; d++) {
        drvNodes[d].Count = drvInfos[d].DeviceCount;
        drvNodes[d].Indices = ExAllocatePool2(POOL_FLAG_NON_PAGED, drvInfos[d].DeviceCount * sizeof(ULONG), MAP_POOL_TAG);
        if (!drvNodes[d].Indices) { status = STATUS_INSUFFICIENT_RESOURCES; goto cleanup; }
    }

    counters = ExAllocatePool2(POOL_FLAG_NON_PAGED, drvCount * sizeof(ULONG), MAP_POOL_TAG);
    if (!counters) { status = STATUS_INSUFFICIENT_RESOURCES; goto cleanup; }
    RtlZeroMemory(counters, drvCount * sizeof(ULONG));

    // 绑定设备索引
    for (i = 0; i < totalEntries; i++) {
        PDRIVER_OBJECT currentDrv = tempNodes[i].AttacherDriver;
        for (d = 0; d < drvCount; d++) {
            if (drvInfos[d].DriverObject == currentDrv) {
                drvNodes[d].Indices[counters[d]++] = i;
                break;
            }
        }
    }

    // 计算总大小
    ULONG totalSize = sizeof(GLOBAL_ATTACH_INFO);
    for (d = 0; d < drvCount; d++) {
        totalSize += sizeof(DRIVER_ATTACH_INFO) + drvInfos[d].DeviceCount * sizeof(ATTACHED_DEVICE_NODE);
    }

    // 缓冲区检查
    if (OutputBufferSize < totalSize) {
        if (pBytesWritten) *pBytesWritten = totalSize;
        status = STATUS_BUFFER_TOO_SMALL;
        goto cleanup;
    }

    // 填充输出数据 (原有逻辑不变)
    PGLOBAL_ATTACH_INFO pGlobal = OutputBuffer;
    RtlZeroMemory(pGlobal, totalSize);
    pGlobal->DriverCount = drvCount;
    pGlobal->TotalSize = totalSize;

    ULONG currentOffset = sizeof(GLOBAL_ATTACH_INFO);
    for (d = 0; d < drvCount; d++) {
        PDRIVER_ATTACH_INFO pDrvInfo = (PDRIVER_ATTACH_INFO)((PUCHAR)pGlobal + currentOffset);
        RtlCopyMemory(pDrvInfo->DriverName, drvInfos[d].DriverName, sizeof(pDrvInfo->DriverName));
        pDrvInfo->DriverObject = (UINT64)drvInfos[d].DriverObject;
        pDrvInfo->DeviceCount = drvInfos[d].DeviceCount;
        currentOffset += sizeof(DRIVER_ATTACH_INFO);

        PATTACHED_DEVICE_NODE pDeviceNodes = (PATTACHED_DEVICE_NODE)((PUCHAR)pGlobal + currentOffset);
        for (ULONG n = 0; n < drvInfos[d].DeviceCount; n++) {
            PTEMP_NODE pSrc = &tempNodes[drvNodes[d].Indices[n]];
            PATTACHED_DEVICE_NODE pDst = &pDeviceNodes[n];
            pDst->IsOriginalDevice = pSrc->IsOriginalDevice; // 修复：传递正确值
            pDst->DeviceObject = pSrc->AttacherDevice;
            pDst->DriverObject =pSrc->AttacherDriver;
            RtlCopyMemory(pDst->DriverName, pSrc->DriverName, sizeof(pDst->DriverName));
            RtlCopyMemory(pDst->DriverPath, pSrc->DriverPath, sizeof(pDst->DriverPath));
            RtlCopyMemory(pDst->DeviceName, pSrc->DeviceName, sizeof(pDst->DeviceName));
            pDst->Level = pSrc->Level;
            pDst->ParentDeviceObject = pSrc->ParentDeviceObject;
        }
        currentOffset += drvInfos[d].DeviceCount * sizeof(ATTACHED_DEVICE_NODE);
    }

    if (pBytesWritten) *pBytesWritten = totalSize;

cleanup:
    // 释放内存
    if (tempNodes) ExFreePoolWithTag(tempNodes, MAP_POOL_TAG);
    if (drvInfos) ExFreePoolWithTag(drvInfos, MAP_POOL_TAG);
    if (drvNodes) {
        for (d = 0; d < drvCount; d++) if (drvNodes[d].Indices) ExFreePoolWithTag(drvNodes[d].Indices, MAP_POOL_TAG);
        ExFreePoolWithTag(drvNodes, MAP_POOL_TAG);
    }
    if (counters) ExFreePoolWithTag(counters, MAP_POOL_TAG);
    return status;
}

ULONG CalculateDeviceLevel(PDEVICE_OBJECT StackTop, PDEVICE_OBJECT TargetDev)
{
    if (!StackTop || !TargetDev) return 0;

    ULONG level = 0;
    PDEVICE_OBJECT current = StackTop;

    // 从设备栈顶部（最后附加的过滤器）向下遍历到底层设备
    while (current != NULL) {
        if (current == TargetDev) {
            return level;
        }
        level++;
        current = current->AttachedDevice;  // 向下一个设备（更接近底层）
    }

    return 0; // 未找到
}
