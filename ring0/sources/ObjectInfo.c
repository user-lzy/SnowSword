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

NTSTATUS QueryObject(HANDLE dwProcessId, HANDLE Handle, LPWSTR Type, ULONG TypeLength, LPWSTR Name, ULONG NameLength, PVOID* pObject)
{
	PEPROCESS hProcess = NULL;
	NTSTATUS status;

	// 查找进程
	status = PsLookupProcessByProcessId(dwProcessId, &hProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("PsLookupProcessByProcessId failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)dwProcessId, status);
		return status;
	}

	// 打开源进程句柄 
	HANDLE hSourceProcess;
	status = ObOpenObjectByPointer(hProcess, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &hSourceProcess);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObOpenObjectByPointer failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)dwProcessId, status);
		ObDereferenceObject(hProcess);
		return status;
	}

	// 复制句柄(EtwRegistration返回C00000BB,WindowStation和Desktop返回C0000022)
	HANDLE hTargetHandle;
	status = ZwDuplicateObject(hSourceProcess, Handle, NtCurrentProcess(), &hTargetHandle, PROCESS_ALL_ACCESS, 0, DUPLICATE_SAME_ACCESS);
	if (!NT_SUCCESS(status)) {
		if (status == STATUS_NOT_SUPPORTED) {
			DbgPrint("这是一个不支持的句柄类型，可能是EtwRegistration\n");
			//设置句柄类型为EtwRegistration,使用RtlStringCbCopyW
			RtlStringCbCopyW(Type, TypeLength, L"EtwRegistration");
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
			DbgPrint("ZwDuplicateObject failed for Process ID %lld at Handle %lld, status: 0x%X\n", (ULONG_PTR)dwProcessId, (ULONG_PTR)Handle, status);
			ZwClose(hSourceProcess);
			ObDereferenceObject(hProcess);
			return status;
		}
	}

	// 初始化 Object 指针为 NULL
	*pObject = NULL;

	// 获取句柄对应的对象
	status = ObReferenceObjectByHandle(hTargetHandle, 0, NULL, UserMode, pObject, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObReferenceObjectByHandle failed for Handle 0x%p, PID:%lld, status: %X\n", Handle, (ULONG_PTR)dwProcessId, status);
	}
	else {
		ObDereferenceObject(*pObject);
	}

	OBJECT_NAME_INFORMATION* NameInfo = NULL;

	// 查询句柄的名称信息
	// 初始查询，获取所需的缓冲区大小
	ULONG returnLength = 0;
	ULONG bufferSize = 0;
	status = ZwQueryObject(hTargetHandle, ObjectNameInformation, NameInfo, bufferSize, &returnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		DbgPrint("第一次查询0x%p名称失败:%X", Handle, status);
		goto QueryType;
	}
	bufferSize = returnLength;
	NameInfo = KernelAlloc_NonPagedPoolNx(POOL_FLAG_NON_PAGED, bufferSize, 'aaaa');
	if (!NameInfo)
	{
		DbgPrint("查询0x%p名称分配空间失败:%X", Handle, STATUS_INSUFFICIENT_RESOURCES);
		goto QueryType;
	}

	status = ZwQueryObject(hTargetHandle, ObjectNameInformation, NameInfo, bufferSize, &returnLength);
	if (!NT_SUCCESS(status)) {
		ExFreePool(NameInfo);
		DbgPrint("第二次查询0x%p名称失败:%X", Handle, status);
		goto QueryType;
	}
	// 检查复制的长度是否超过缓冲区大小
	if (NameInfo->Name.Length <= NameLength - sizeof(WCHAR)) {
		memcpy(Name, NameInfo->Name.Buffer, NameInfo->Name.Length);
		Name[NameInfo->Name.Length / sizeof(WCHAR)] = L'\0';
		DbgPrint("The Name of Handle 0x%p:%wZ", Handle, NameInfo->Name);
	}
	ExFreePool(NameInfo);

	// 查询句柄的类型信息
QueryType:
	typedef POBJECT_TYPE ObGetObjectTypeFunc(PVOID Object);
	static UNICODE_STRING ObGetObjectTypeName = RTL_CONSTANT_STRING(L"ObGetObjectType");
	static ObGetObjectTypeFunc* ObGetObjectType = NULL;
	if (!ObGetObjectType)
		ObGetObjectType = (ObGetObjectTypeFunc*)MmGetSystemRoutineAddress(&ObGetObjectTypeName);
	
	if (ObGetObjectType && *pObject) {
		POBJECT_TYPE ObjectType = ObGetObjectType(*pObject);
		if (ObjectType && ObjectType->Name.Buffer) {
			RtlCopyMemory(Type, ObjectType->Name.Buffer, ObjectType->Name.Length);
			Type[ObjectType->Name.Length / sizeof(WCHAR)] = L'\0';
		}
		else {
			DbgPrint("获取对象类型失败");
		}
	}
	else {
		DbgPrint("获取ObGetObjectType失败");
	}
//Cleanup:
	ZwClose(hTargetHandle);
	ZwClose(hSourceProcess);
	ObDereferenceObject(hProcess);
	return status;
}

NTSTATUS QueryFileObject(
	HANDLE ProcessId,
	HANDLE Handle,
	PWCHAR Name,
	ULONG NameSize)
{
	PEPROCESS hProcess = NULL;
	NTSTATUS status;

	HANDLE hSourceProcess = NULL;
	HANDLE hTargetHandle = NULL;

	OBJECT_NAME_INFORMATION* NameInfo = NULL;

	ULONG returnLength = 0;
	ULONG bufferSize = 0;

	status = PsLookupProcessByProcessId(
		ProcessId,
		&hProcess);

	if (!NT_SUCCESS(status))
		return status;

	status = ObOpenObjectByPointer(
		hProcess,
		OBJ_KERNEL_HANDLE,
		NULL,
		PROCESS_ALL_ACCESS,
		*PsProcessType,
		KernelMode,
		&hSourceProcess);

	if (!NT_SUCCESS(status))
	{
		ObDereferenceObject(hProcess);
		return status;
	}

	status = ZwDuplicateObject(
		hSourceProcess,
		Handle,
		NtCurrentProcess(),
		&hTargetHandle,
		0,
		0,
		DUPLICATE_SAME_ACCESS);

	if (!NT_SUCCESS(status))
	{
		ZwClose(hSourceProcess);
		ObDereferenceObject(hProcess);
		return status;
	}

	/*
	 * 第一次查询名称，获取缓冲区大小
	 */
	status = ZwQueryObject(
		hTargetHandle,
		ObjectNameInformation,
		NULL,
		0,
		&returnLength);

	if (status != STATUS_INFO_LENGTH_MISMATCH &&
		status != STATUS_BUFFER_TOO_SMALL)
	{
		goto Exit;
	}

	bufferSize = returnLength;

	NameInfo = KernelAlloc_NonPagedPoolNx(
		POOL_FLAG_NON_PAGED,
		bufferSize,
		'aaaa');

	if (NameInfo == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	status = ZwQueryObject(
		hTargetHandle,
		ObjectNameInformation,
		NameInfo,
		bufferSize,
		&returnLength);

	if (!NT_SUCCESS(status))
		goto Exit;

	if (NameInfo->Name.Buffer != NULL &&
		NameInfo->Name.Length <=
		NameSize - sizeof(WCHAR))
	{
		memcpy(Name, NameInfo->Name.Buffer, NameInfo->Name.Length);
		Name[NameInfo->Name.Length / sizeof(WCHAR)] = L'\0';
	}

Exit:

	if (NameInfo)
		ExFreePool(NameInfo);

	if (hTargetHandle)
		ZwClose(hTargetHandle);

	if (hSourceProcess)
		ZwClose(hSourceProcess);

	if (hProcess)
		ObDereferenceObject(hProcess);

	return status;
}

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
		pBuffer = KernelAlloc_NonPagedPoolNx(POOL_FLAG_NON_PAGED, bufferSize, 'HdlT');
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

	LONG offset = *(PLONG)((PUCHAR)result + sizeof(ulSpecialCode));
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
		pBuffer = (PDIRECTORY_BASIC_INFORMATION)KernelAlloc_NonPagedPoolNx(POOL_FLAG_NON_PAGED, ulLength, 'pbuf');
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