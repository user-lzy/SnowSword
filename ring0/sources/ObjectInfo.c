#pragma once

#include "ObjectInfo.h"
#include "EnumDriverInfo.h"
#include "OtherFunctions.h"

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

/*NTSTATUS ObQueryObject(PHANDLE_INFO HandleInfo) {
	PEPROCESS Process = NULL;
	PsLookupProcessByProcessId(HandleInfo->dwProcessId, &Process);
	if (!Process) return STATUS_UNSUCCESSFUL;

	PHANDLE_TABLE HandleTable = (PHANDLE_TABLE)Process->ObjectTable;

	// 清除句柄的低三位（通常用于标识句柄的属性）
	HANDLE CleanedHandle = HandleInfo->Handle & ~((HANDLE)0x7);

	// 计算句柄表项的索引
	ULONG_PTR Index = CleanedHandle / 4;

	// 获取句柄表项
	PHANDLE_TABLE_ENTRY HandleTableEntry = (PHANDLE_TABLE_ENTRY)(HandleTable->TableCode + Index * sizeof(HANDLE_TABLE_ENTRY));

	POBJECT_TYPE ObjectType = ObTypeIndexTable[HandleTableEntry->TypeIndex ^ OB_HEADER_COOKIE];

	// 获取内核对象
	//HandleInfo->Object = HandleTableEntry->Object;
	//HandleInfo->Type = HandleTableEntry->Object->Type->Name;
	//HandleInfo->Name = HandleTableEntry->Object->NameInfo.Name;

	ObDereferenceObject(Process);
	return STATUS_SUCCESS;
}

/*NTSTATUS ObQueryObject(PHANDLE_INFO HandleInfo) {
	PEPROCESS hProcess = NULL;
	NTSTATUS status;
	
	// 查找进程
	status = PsLookupProcessByProcessId((HANDLE)HandleInfo->dwProcessId, &hProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("PsLookupProcessByProcessId failed for Process ID %d, status: 0x%X\n", HandleInfo->dwProcessId, status);
		return status;
	}

	// 进程上下文切换
	KAPC_STATE ApcState;
	KeStackAttachProcess(hProcess, &ApcState);

	// 初始化 Object 指针为 NULL
	HandleInfo->Object = NULL;

	// 获取句柄对应的对象
	status = ObReferenceObjectByHandle((HANDLE)HandleInfo->Handle, 0, NULL, KernelMode, &HandleInfo->Object, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObReferenceObjectByHandle failed for Handle %d, status: 0x%X\n", HandleInfo->Handle, status);
		HandleInfo->Object = NULL;  // 确保 Object 指针被正确设置为 NULL
		KeUnstackDetachProcess(&ApcState);
		ObDereferenceObject(hProcess);
		return STATUS_UNSUCCESSFUL;
	}
	else {
		DbgPrint("The Object of Handle: 0x%p\n", HandleInfo->Object);
	}

	// 如果成功获取对象，查询对象名称
	/*if (HandleInfo->Object) {
		ULONG nameLength = 0;
		status = ObQueryNameString(HandleInfo->Object, NULL, 0, &nameLength);

		if (status == STATUS_INFO_LENGTH_MISMATCH) {
			POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, nameLength, 'MyTg');
			if (nameInfo) {
				status = ObQueryNameString(HandleInfo->Object, nameInfo, nameLength, &nameLength);
				if (NT_SUCCESS(status)) {
					HandleInfo->Name = nameInfo->Name;
					DbgPrint("The Name of Handle: %wZ\n", HandleInfo->Name);
				}
				else {
					DbgPrint("ObQueryNameString failed, status: 0x%X\n", status);
				}
				ExFreePool(nameInfo);
			}
			else {
				DbgPrint("Failed to allocate memory for OBJECT_NAME_INFORMATION\n");
			}
		}
		else if (!NT_SUCCESS(status)) {
			DbgPrint("ObQueryNameString failed to get name length, status: 0x%X\n", status);
		}
	}

	// 获取对象类型
	if (HandleInfo->Object) {
		POBJECT_TYPE objectType = *(POBJECT_TYPE*)((PUCHAR)HandleInfo->Object + 0x20);
		//HandleInfo->Name = objectType->Name;
		// 复制字符串内容
		RtlCopyMemory(HandleInfo->Name, &objectType->Name, objectType->Name.Length);
		// 添加字符串结束符
		HandleInfo->Name[objectType->Name.Length / sizeof(WCHAR)] = UNICODE_NULL;
		DbgPrint("The Name of Handle: %wZ\n", HandleInfo->Name);

		POBJECT_HEADER objectHeader = OBJECT_TO_OBJECT_HEADER(HandleInfo->Object);
		objectType = *(POBJECT_TYPE*)((PUCHAR)objectHeader + 0x20); // 偏移 0x20 获取对象类型指针

		// 获取对象类型名称
		if (objectType->Name.Length > 0 && objectType->Name.Length < 256 * sizeof(WCHAR))
		{
			DbgPrint("The Type of Handle: %wZ", &objectType->Name);
			RtlCopyMemory(&HandleInfo->Type, objectType->Name.Buffer, objectType->Name.Length);
			HandleInfo->Type[objectType->Name.Length / sizeof(WCHAR)] = UNICODE_NULL;
			return STATUS_SUCCESS;
		}
		else
		{
			return STATUS_INVALID_PARAMETER;
		}
	}
	DbgPrint("2");

	// 释放对象引用
	if (HandleInfo->Object) {
		ObDereferenceObject(HandleInfo->Object);
	}

	// 恢复上下文
	KeUnstackDetachProcess(&ApcState);
	ObDereferenceObject(hProcess);

	return status;
}*/

NTSTATUS QueryObject(PHANDLE_INFO HandleInfo)
{
	PEPROCESS hProcess = NULL;
	NTSTATUS status;

	// 查找进程
	status = PsLookupProcessByProcessId(HandleInfo->dwProcessId, &hProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("PsLookupProcessByProcessId failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)HandleInfo->dwProcessId, status);
		return status;
	}

	// 打开源进程句柄 
	HANDLE hSourceProcess;
	status = ObOpenObjectByPointer(hProcess, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &hSourceProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObOpenObjectByPointer failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)HandleInfo->dwProcessId, status);
		ObDereferenceObject(hProcess);
		return status;
	}

	// 复制句柄(EtwRegistration返回C00000BB,WindowStation和Desktop返回C0000022)
	HANDLE hTargetHandle;
	status = ZwDuplicateObject(hSourceProcess, HandleInfo->Handle, NtCurrentProcess(), &hTargetHandle, PROCESS_ALL_ACCESS, 0, DUPLICATE_SAME_ACCESS);
	if (!NT_SUCCESS(status)) {
		if (status == STATUS_NOT_SUPPORTED) {
			DbgPrint("这是一个不支持的句柄类型，可能是EtwRegistration\n");
			//设置句柄类型为EtwRegistration,使用RtlStringCbCopyW
			RtlStringCbCopyW(HandleInfo->Type, sizeof(HandleInfo->Type), L"EtwRegistration");
			//清理并退出
			ZwClose(hSourceProcess);
			ObDereferenceObject(hProcess);
			return STATUS_SUCCESS;
		}
		else if (status == STATUS_ACCESS_DENIED) {
			DbgPrint("访问被拒绝，可能是WindowStation或Desktop\n");
			ZwClose(hSourceProcess);
			ObDereferenceObject(hProcess);
			return status;
		}
		else {
			DbgPrint("ZwDuplicateObject failed for Process ID %lld at Handle %lld, status: 0x%X\n", (ULONG_PTR)HandleInfo->dwProcessId, (ULONG_PTR)HandleInfo->Handle, status);
			ZwClose(hSourceProcess);
			ObDereferenceObject(hProcess);
			return status;
		}
	}

	// 进程上下文切换
	//KAPC_STATE ApcState;
	//KeStackAttachProcess(hProcess, &ApcState);

	// 初始化 Object 指针为 NULL
	HandleInfo->Object = NULL;

	// 获取句柄对应的对象
	status = ObReferenceObjectByHandle(hTargetHandle, 0, NULL, KernelMode, &HandleInfo->Object, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObReferenceObjectByHandle failed for Handle 0x%p, PID:%lld, status: %X\n", HandleInfo->Handle, (ULONG_PTR)HandleInfo->dwProcessId, status);
	}
	else {
		ObDereferenceObject(HandleInfo->Object);
	}

	OBJECT_BASIC_INFORMATION BasicInfo;
	OBJECT_NAME_INFORMATION* NameInfo = NULL;
	//OBJECT_TYPE_INFORMATION* TypeInfo = NULL;
	//ULONG ReturnLength;

	/*if (KeGetCurrentIrql() == DISPATCH_LEVEL) {
		DbgPrint("当前IRQL为2,退出MyQueryObject");
		return STATUS_SUCCESS;
	}*/

	// 查询句柄的基本信息
	status = ZwQueryObject(hTargetHandle, ObjectBasicInformation, &BasicInfo, sizeof(BasicInfo), NULL);
	if (!NT_SUCCESS(status)) DbgPrint("查询0x%p基本信息失败:%X", HandleInfo->Handle, status);

	// 查询句柄的名称信息
	// 初始查询，获取所需的缓冲区大小
	ULONG returnLength = 0;
	ULONG bufferSize = 0;
	status = ZwQueryObject(hTargetHandle, ObjectNameInformation, NameInfo, bufferSize, &returnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		DbgPrint("第一次查询0x%p名称失败:%X", HandleInfo->Handle, status);
		goto QueryType;
	}
	bufferSize = returnLength;
	NameInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, 'aaaa');
	if (!NameInfo)
	{
		DbgPrint("查询0x%p名称分配空间失败:%X", HandleInfo->Handle, STATUS_INSUFFICIENT_RESOURCES);
		goto QueryType;
	}

	status = ZwQueryObject(hTargetHandle, ObjectNameInformation, NameInfo, bufferSize, &returnLength);
	if (!NT_SUCCESS(status)) {
		ExFreePool(NameInfo);
		DbgPrint("第二次查询0x%p名称失败:%X", HandleInfo->Handle, status);
		goto QueryType;
	}
	// 检查复制的长度是否超过缓冲区大小
	if (NameInfo->Name.Length <= sizeof(HandleInfo->Name) - sizeof(WCHAR)) {
		memcpy(HandleInfo->Name, NameInfo->Name.Buffer, NameInfo->Name.Length);
		HandleInfo->Name[NameInfo->Name.Length / sizeof(WCHAR)] = L'\0';
		DbgPrint("The Name of Handle 0x%p:%wZ", HandleInfo->Handle, NameInfo->Name);
	}
	ExFreePool(NameInfo);

	// 查询句柄的类型信息
QueryType:
	/*status = ZwQueryObject(hTargetHandle, ObjectTypeInformation, TypeInfo, bufferSize, &returnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		DbgPrint("第一次查询0x%p类型失败:%X", HandleInfo->Handle, status);
		status = STATUS_SUCCESS;
		goto Cleanup;
	}
	bufferSize = returnLength;
	TypeInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, 'aaaa');
	if (!TypeInfo)
	{
		DbgPrint("查询类型分配空间失败:%X", STATUS_INSUFFICIENT_RESOURCES);
		status = STATUS_SUCCESS;
		goto Cleanup;
	}

	status = ZwQueryObject(hTargetHandle, ObjectTypeInformation, TypeInfo, bufferSize, &returnLength);
	if (!NT_SUCCESS(status)) {
		ExFreePool(TypeInfo);
		DbgPrint("第二次查询0x%p类型失败:%X", HandleInfo->Handle, status);
		status = STATUS_SUCCESS;
		goto Cleanup;
	}

	if (TypeInfo->TypeName.Length <= sizeof(HandleInfo->Type) - sizeof(WCHAR)) {
		memcpy(HandleInfo->Type, TypeInfo->TypeName.Buffer, TypeInfo->TypeName.Length);
		HandleInfo->Type[TypeInfo->TypeName.Length / sizeof(WCHAR)] = L'\0';
		DbgPrint("The Type of Handle 0x%p:%wZ", HandleInfo->Handle, TypeInfo->TypeName);
	}
	ExFreePool(TypeInfo);*/
	typedef POBJECT_TYPE ObGetObjectTypeFunc(PVOID Object);
	UNICODE_STRING ObGetObjectTypeName = RTL_CONSTANT_STRING(L"ObGetObjectType");
	ObGetObjectTypeFunc* ObGetObjectType = (ObGetObjectTypeFunc*)MmGetSystemRoutineAddress(&ObGetObjectTypeName);
	if (ObGetObjectType && HandleInfo->Object) {
		POBJECT_TYPE ObjectType = ObGetObjectType(HandleInfo->Object);
		if (ObjectType && ObjectType->Name.Buffer) {
			RtlCopyMemory(HandleInfo->Type, ObjectType->Name.Buffer, ObjectType->Name.Length);
			HandleInfo->Type[ObjectType->Name.Length / sizeof(WCHAR)] = L'\0';
			DbgPrint("The Type of Handle 0x%p:%wZ", HandleInfo->Handle, ObjectType->Name);
		}
		else {
			DbgPrint("获取对象类型失败");
		}
	}
	else {
		DbgPrint("获取ObGetObjectType失败");
	}
	if (HandleInfo->Handle == (HANDLE)0x1cc) DbgPrint("status:%X", status);
//Cleanup:
	ZwClose(hTargetHandle);
	ZwClose(hSourceProcess);
	//KeUnstackDetachProcess(&ApcState);
	ObDereferenceObject(hProcess);
	return status;
}

// QueryObject 函数
/*NTSTATUS QueryObject(PHANDLE_INFO HandleInfo, PDEVICE_OBJECT DeviceObject)
{
	NTSTATUS status;
	KIRQL currentIrql = KeGetCurrentIrql();

	// 检查当前 IRQL 级别
	if (KeGetCurrentIrql() == DISPATCH_LEVEL) {
		// 当前 IRQL 为 DISPATCH_LEVEL，需要通过工作线程异步执行任务
		return QueryObjectAsync(HandleInfo, DeviceObject);
	}
	else {
		// 当前 IRQL 为 PASSIVE_LEVEL，可以直接执行任务
		QueryObjectSync(HandleInfo);
		return STATUS_SUCCESS;
	}
}*/

NTSTATUS CloseHandle(PDO_SOMETHING DoSomething) {
	PEPROCESS TargetProcess;
	HANDLE TargetHandle = (HANDLE)DoSomething->Context;
	KAPC_STATE ApcState = { 0 };
	NTSTATUS status;

	// 获取目标进程 
	status = PsLookupProcessByProcessId(DoSomething->ProcessId, &TargetProcess);
	if (!NT_SUCCESS(status)) return status;

	// 附加到进程上下文 
		// 判断操作系统版本
	RTL_OSVERSIONINFOEXW OSVersion = { 0 };
	OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);
	if (OSVersion.dwBuildNumber >= 19041)
	{
		//动态调用
		typedef VOID(*KeStackAttachProcessFunc)(PEPROCESS, PKAPC_STATE);
		UNICODE_STRING KeStackAttachProcessName = RTL_CONSTANT_STRING(L"KeStackAttachProcess");
		KeStackAttachProcessFunc KeStackAttachProcess = (KeStackAttachProcessFunc)MmGetSystemRoutineAddress(&KeStackAttachProcessName);
		KeStackAttachProcess(TargetProcess, &ApcState);
	}
	else
	{
		KeAttachProcess(TargetProcess);
	}
	// 修改句柄属性
	OBJECT_HANDLE_FLAG_INFORMATION ohfi;
	ohfi.Inherit = 0;
	ohfi.ProtectFromClose = 0;
	status = ObSetHandleAttributes(TargetHandle, &ohfi, KernelMode);

	// 强制关闭句柄 
	if (NT_SUCCESS(status)) {
		ZwClose(TargetHandle);
		DbgPrint("Handle %p closed successfully\n", TargetHandle);
	}

	// 恢复上下文并清理 
	if (OSVersion.dwBuildNumber >= 19041)
	{
		//动态调用
		typedef VOID(*KeUnstackDetachProcessFunc)(PKAPC_STATE);
		UNICODE_STRING KeUnstackDetachProcessName = RTL_CONSTANT_STRING(L"KeUnstackDetachProcess");
		KeUnstackDetachProcessFunc KeUnstackDetachProcess = (KeUnstackDetachProcessFunc)MmGetSystemRoutineAddress(&KeUnstackDetachProcessName);
		KeUnstackDetachProcess(&ApcState);
	}
	else
	{
		KeDetachProcess();
	}
	ObDereferenceObject(TargetProcess);
	return status;
}

/*HANDLE GetCsrPid() {
	HANDLE Process, hObject;
	HANDLE CsrId = (HANDLE)0;
	OBJECT_ATTRIBUTES obj;
	CLIENT_ID cid;
	UCHAR Buff[0x100];
	POBJECT_NAME_INFORMATION ObjName = (PVOID)&Buff;
	PSYSTEM_HANDLE_INFORMATION Handles;
	ULONG r;

	ULONG bufferSize = 0;
	PVOID pBuffer = NULL;
	NTSTATUS status = ZwQuerySystemInformation(SystemHandleInformation, NULL, 0, &bufferSize);

	// 处理 STATUS_INFO_LENGTH_MISMATCH 错误 
	if (status == STATUS_INFO_LENGTH_MISMATCH) {
		pBuffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, 'HdlT');
		status = ZwQuerySystemInformation(SystemHandleInformation, pBuffer, bufferSize, NULL);
		if (!NT_SUCCESS(status)) {
			DbgPrint("ZwQuerySysytemInformation failed!status=%X", status);
			return NULL;
		}
	}
	Handles = (PSYSTEM_HANDLE_INFORMATION)pBuffer;
	if (!Handles) return CsrId;
	for (r = 0; r < Handles->NumberOfHandles; r++) {
		//Port object
		InitializeObjectAttributes(&obj, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

		cid.UniqueProcess = (HANDLE)Handles->Information[r].ProcessId;
		cid.UniqueThread = 0;

		if (NT_SUCCESS(NtOpenProcess(&Process, PROCESS_DUP_HANDLE, &obj, &cid))) {
			if (NT_SUCCESS(ZwDuplicateObject(Process, (HANDLE)Handles->Information[r].Handle, NtCurrentProcess(), &hObject, 0, 0, DUPLICATE_SAME_ACCESS))) {
				if (NT_SUCCESS(ZwQueryObject(hObject, ObjectNameInformation, ObjName, 0x100, NULL))) {
					if (ObjName->Name.Buffer && !wcsncmp(L"\\Windows\\ApiPort", ObjName->Name.Buffer, 20)) {
						CsrId = (HANDLE)Handles->Information[r].ProcessId;
						DbgPrint("ZwQueryObject：%wZ ID:%d  Type::%d\n", &ObjName->Name, Handles->Information[r].ProcessId, Handles->Information[r].ObjectTypeNumber);
					}
				}
				ZwClose(hObject);
			}
			ZwClose(Process);
		}
	}
	ExFreePool(Handles);
	return CsrId;
}*/

PVOID FindObTypeIndexTable() {
	UNICODE_STRING ObGetObjectTypeName = RTL_CONSTANT_STRING(L"ObGetObjectType");
	PVOID ObGetObjectTypeAddr = MmGetSystemRoutineAddress(&ObGetObjectTypeName);
	if (ObGetObjectTypeAddr == NULL) return NULL;

	UCHAR ulSpecialCode[3] = { 0x48, 0x8d, 0x0d };
	PVOID result = SearchSpecialCode(ObGetObjectTypeAddr, 0x30, ulSpecialCode, sizeof(ulSpecialCode));
	if (result == NULL) return NULL;

	ULONG offset = *(PULONG)((PUCHAR)result + sizeof(ulSpecialCode));
	return (PVOID)((PUCHAR)result + 7 + offset);
}

NTSTATUS GetObjectInfo(ULONG Index, POBJECT_INFORMATION Array) {
	POBJECT_TYPE* ObTypeIndexTable = (POBJECT_TYPE*)FindObTypeIndexTable();
	if (ObTypeIndexTable == NULL) return STATUS_UNSUCCESSFUL;

	ULONG ObTypeMaxIndex = ObTypeIndexTable[2]->TotalNumberOfObjects + 1;
	if (Index > ObTypeMaxIndex) return STATUS_NO_MORE_ENTRIES;

#define offsetof(struct_type, member) ((size_t)&((struct_type*)0)->member)
	POBJECT_TYPE ObType = ObTypeIndexTable[Index];
	RtlStringCbCopyUnicodeString(Array->ObjectName, sizeof(Array->ObjectName), &ObType->Name);
	for (int i = 0, offset = offsetof(OBJECT_TYPE_INITIALIZER, DumpProcedure); offset <= offsetof(OBJECT_TYPE_INITIALIZER, OkayToCloseProcedure); offset += sizeof(PVOID))
		Array->Procedure[i++] = *(PVOID*)((PUCHAR)&ObType->TypeInfo + offset);
	
	return STATUS_SUCCESS;
}

POBJECT_TYPE* FindExCallbackObjectType() {
	/*
	fffff803`97672296 488b1d7b3d5500  mov     rbx,qword ptr [nt!ExCallbackObjectType (fffff803`97bc6018)]
	*/
	UCHAR ulSpecialCode[3] = {0x48, 0x8B, 0x1D};
	PVOID result = SearchSpecialCode((PVOID)ExCreateCallback, 0x100, ulSpecialCode, 3);
	if (result == NULL) return NULL;
	LONG offset = *(PLONG)((PUCHAR)result + 3);
	return *(POBJECT_TYPE**)((PUCHAR)result + 7 + offset);
}

/*NTSTATUS QueryCallbackNameByPointer(PVOID pObject, PWCHAR CallbackName) {
	NTSTATUS status;
	HANDLE hCallbackDir;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING callbackDirPath = RTL_CONSTANT_STRING(L"\\Callback");

	POBJECT_TYPE* ExCallbackObjectType = FindExCallbackObjectType();
	if (!ExCallbackObjectType) return STATUS_UNSUCCESSFUL;

	// 打开 \Callback 目录
	InitializeObjectAttributes(&oa, &callbackDirPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenDirectoryObject(&hCallbackDir, DIRECTORY_QUERY, &oa);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ZwOpenDirectoryObject failed!status=%X", status);
		return status;
	}

	// 枚举目录中的回调对象
	PDIRECTORY_BASIC_INFORMATION   pBuffer = NULL;
	PDIRECTORY_BASIC_INFORMATION   pBuffer2 = NULL;
	ULONG    ulLength = 0x800;    // 2048  
	ULONG    ulContext = 0;
	ULONG    ulRet = 0;

	// 查询目录对象  
	do
	{
		if (pBuffer != NULL) ExFreePoolWithTag(pBuffer, 'pbuf');

		ulLength = ulLength * 2;
		pBuffer = (PDIRECTORY_BASIC_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, ulLength, 'pbuf');
		if (NULL == pBuffer)
		{
			if (pBuffer != NULL) ExFreePoolWithTag(pBuffer, 'pbuf');

			if (hCallbackDir != NULL) ZwClose(hCallbackDir);

			return STATUS_INSUFFICIENT_RESOURCES;
		}
		status = ZwQueryDirectoryObject(hCallbackDir, pBuffer, ulLength, FALSE, TRUE, &ulContext, &ulRet);
	} while (status == STATUS_MORE_ENTRIES || status == STATUS_BUFFER_TOO_SMALL);

	if (status != STATUS_SUCCESS)
	{
		DbgPrint("ZwQueryDirectoryObject failed!status=%X", status);
		if (pBuffer != NULL) ExFreePoolWithTag(pBuffer, 'pbuf');

		if (hCallbackDir != NULL) ZwClose(hCallbackDir);
		return status;
	}

	pBuffer2 = pBuffer;
	while ((pBuffer2->ObjectName.Length != 0) && (pBuffer2->ObjectType.Length != 0)) {
		// 构造回调对象路径（如 \Callback\PowerState）
		UNICODE_STRING callbackPath;
		WCHAR callbackPathBuf[256];
		RtlInitEmptyUnicodeString(&callbackPath, callbackPathBuf, sizeof(callbackPathBuf));
		RtlAppendUnicodeToString(&callbackPath, L"\\Callback\\");
		RtlAppendUnicodeStringToString(&callbackPath, &pBuffer2->ObjectName);
		//DbgPrint("Processing Driver Object: %wZ", &driverPath);

		// 获取回调对象
		PCALLBACK_OBJECT callbackObject;
		status = ObReferenceObjectByName(&callbackPath, OBJ_CASE_INSENSITIVE, NULL, 0, *ExCallbackObjectType, KernelMode, NULL, (PVOID*)&callbackObject);
		if (NT_SUCCESS(status)) {
			ObDereferenceObject(callbackObject);
			if (callbackObject == pObject) {
				// 找到匹配的回调对象
				DbgPrint("Found Callback Object: %wZ", &callbackPath);
				if (CallbackName) {
					RtlStringCbCopyUnicodeString(CallbackName, callbackPath.Length + sizeof(WCHAR), &callbackPath);
					status = STATUS_SUCCESS;
				}
				else {
					status = STATUS_INSUFFICIENT_RESOURCES;
				}
				break;
			}
		}
		else {
			DbgPrint("ObReferenceObjectByName failed for %wZ, status=%X", &callbackPath, status);
		}

		pBuffer2++;
	}

	if (pBuffer != NULL) ExFreePoolWithTag(pBuffer, 'pbuf');

	if (hCallbackDir != NULL) ZwClose(hCallbackDir);

	return status;
}*/