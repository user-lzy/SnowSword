#pragma once
#include "Process.h"
PVOID ObHandle1 = NULL;

VOID SuspendProcess(HANDLE PID)
{
    PEPROCESS Process;
    NTSTATUS status = PsLookupProcessByProcessId(PID, &Process);
    if (NT_SUCCESS(status))
    {
        PsSuspendProcess(Process);
        ObDereferenceObject(Process);
    }
}

VOID ResumeProcess(HANDLE PID)
{
    PEPROCESS Process;
    NTSTATUS status = PsLookupProcessByProcessId(PID, &Process);
    if (NT_SUCCESS(status))
    {
        PsResumeProcess(Process);
        ObDereferenceObject(Process);
    }
}

// 添加进程到保护列表
NTSTATUS AddProcessToProtectedList(HANDLE ProcessId) {
    PPROTECTED_PROCESS newEntry;
    newEntry = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PROTECTED_PROCESS), 'cppP');
    if (!newEntry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    newEntry->ProcessId = ProcessId;
    InsertTailList(&ProtectedProcessList.ListEntry, &newEntry->ListEntry);
    DbgPrint("已添加PID:%llu", (ULONG_PTR)ProcessId);
    return STATUS_SUCCESS;
}

// 从保护列表中删除进程
NTSTATUS RemoveProcessFromProtectedList(HANDLE ProcessId) {
    PLIST_ENTRY currentEntry;
    PPROTECTED_PROCESS currentProcess;
    BOOLEAN found = FALSE;

	//if (!IsProcessAlive(ProcessId)) return STATUS_NOT_FOUND;

    currentEntry = ProtectedProcessList.ListEntry.Flink;
    while (currentEntry != &ProtectedProcessList.ListEntry) {
        currentProcess = CONTAINING_RECORD(currentEntry, PROTECTED_PROCESS, ListEntry);
        if (currentProcess->ProcessId == ProcessId) {
            RemoveEntryList(&currentProcess->ListEntry);
			DbgPrint("已移除PID:%llu", (ULONG_PTR)ProcessId);
            ExFreePoolWithTag(currentProcess, 'cppP');
            found = TRUE;
            break;
        }
        currentEntry = currentEntry->Flink;
    }
    return found ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

// 检查进程是否在保护列表中
BOOLEAN IsProcessProtected(HANDLE ProcessId) {
    PLIST_ENTRY currentEntry;
    PPROTECTED_PROCESS currentProcess;
    BOOLEAN found = FALSE;

    currentEntry = ProtectedProcessList.ListEntry.Flink;
    while (currentEntry != &ProtectedProcessList.ListEntry) {
        currentProcess = CONTAINING_RECORD(currentEntry, PROTECTED_PROCESS, ListEntry);
        if (currentProcess->ProcessId == ProcessId) {
            found = TRUE;
            break;
        }
        currentEntry = currentEntry->Flink;
    }
    return found;
}

BOOLEAN IsProcessAlive(HANDLE ProcessId) {
    PEPROCESS eProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &eProcess);
    if (NT_SUCCESS(status)) {
        ObDereferenceObject(eProcess);
        return TRUE;
    }
    return FALSE;
}

//NTHALAPI KIRQL KeGetCurrentIRQL();

NTSTATUS GetProcessChangedPath(HANDLE PID, PWCHAR* FilePath)
{
#define EPROCESS_IMAGEFILEOBJECT_OFFSET 0x330
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT pFileObject = NULL;
    POBJECT_NAME_INFORMATION pNameInfo = NULL;
	PEPROCESS Process = NULL;
    ULONG returnLength = 0;

	// 查找进程
	status = PsLookupProcessByProcessId(PID, &Process);
    if (!NT_SUCCESS(status)) {
        DbgPrint("PsLookupProcessByProcessId failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)PID, status);
        return status;
    }

    if (!FilePath) return STATUS_INVALID_PARAMETER;

    // 从EPROCESS获取ImageFileObject
    pFileObject = *(PFILE_OBJECT*)((PUCHAR)Process + EPROCESS_IMAGEFILEOBJECT_OFFSET);

    if (!pFileObject)
    {
        DbgPrint("Failed to get FileObject from EPROCESS\n");
        return STATUS_NOT_FOUND;
    }

    // 增加引用计数防止对象被释放
    ObReferenceObject(pFileObject);

    __try
    {
        // 获取文件路径信息
        status = ObQueryNameString(pFileObject, NULL, 0, &returnLength);
        if (status != STATUS_INFO_LENGTH_MISMATCH)
        {
            DbgPrint("ObQueryNameString failed: 0x%X\n", status);
            __leave;
        }

        // 分配缓冲区
        pNameInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, returnLength, 'Path');
        if (!pNameInfo) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Memory allocation failed\n");
            __leave;
        }
        // 获取完整路径
        status = ObQueryNameString(pFileObject, pNameInfo, returnLength, &returnLength);
        if (!NT_SUCCESS(status))
        {
            DbgPrint("ObQueryNameString failed: 0x%X\n", status);
            __leave;
        }
        // 分配输出缓冲区
        *FilePath = ExAllocatePool2(POOL_FLAG_NON_PAGED, pNameInfo->Name.Length + sizeof(WCHAR), 'Path');
        if (!*FilePath)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Output buffer allocation failed\n");
            __leave;
        }

        // 复制路径
        memcpy(*FilePath, pNameInfo->Name.Buffer, pNameInfo->Name.Length);
        (*FilePath)[pNameInfo->Name.Length / sizeof(WCHAR)] = L'\0';

        DbgPrint("Process path: %wZ\n", &pNameInfo->Name);
    }
    __finally
    {
        if (pNameInfo) ExFreePoolWithTag(pNameInfo, 'Path');
        ObDereferenceObject(pFileObject);
    }

    return status;
}

VOID RemoveProcessHandleAccess(PACCESS_MASK pDesiredAccess) {
	if (*pDesiredAccess & PROCESS_TERMINATE) *pDesiredAccess &= ~PROCESS_TERMINATE;
	if (*pDesiredAccess & PROCESS_CREATE_THREAD) *pDesiredAccess &= ~PROCESS_CREATE_THREAD;
	if (*pDesiredAccess & PROCESS_VM_OPERATION) *pDesiredAccess &= ~PROCESS_VM_OPERATION;
	if (*pDesiredAccess & PROCESS_VM_WRITE) *pDesiredAccess &= ~PROCESS_VM_WRITE;
	if (*pDesiredAccess & PROCESS_SUSPEND_RESUME) *pDesiredAccess &= ~PROCESS_SUSPEND_RESUME;
}

POB_PREOP_CALLBACK_STATUS ProtectProcessCallBack(PVOID Context, POB_PRE_OPERATION_INFORMATION pObPreOperationInformation) {
	UNREFERENCED_PARAMETER(Context);
    HANDLE ProcessId = PsGetProcessId((PEPROCESS)pObPreOperationInformation->Object);
    /*UNICODE_STRING ProcessName = RTL_CONSTANT_STRING(L"notepad.exe");
    ANSI_STRING CmpAnsiString;
    RtlInitString(&CmpAnsiString, GetProcessPath(ProcessId));
    UNICODE_STRING AnsiToUniName;
    NTSTATUS status = RtlAnsiStringToUnicodeString(&AnsiToUniName, &CmpAnsiString, TRUE);
    if (!NT_SUCCESS(status)) {
        DbgPrint("字符串转换失败\n");
        return OB_PREOP_SUCCESS;
    }*/
    if (IsProcessProtected(ProcessId)) {
        switch (pObPreOperationInformation->Operation)
        {
		case OB_OPERATION_HANDLE_CREATE:
			RemoveProcessHandleAccess(&pObPreOperationInformation->Parameters->CreateHandleInformation.DesiredAccess);
            DbgPrint("检测到HANDLE_CREATE事件,发起者PID:%p", PsGetCurrentProcessId());
            break;
        case OB_OPERATION_HANDLE_DUPLICATE:
			RemoveProcessHandleAccess(&pObPreOperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess);
            DbgPrint("检测到HANDLE_DUPLICATE事件,发起者PID:%p", PsGetCurrentProcessId());
            break;
        }
        //pObPreOperationInformation->ReturnValue = STATUS_NOT_IMPLEMENTED //表示功能未实现
        //return OB_PREOP_FAIL;
    }
    return OB_PREOP_SUCCESS;
}

NTSTATUS SetProcessProtectionStatus(BOOLEAN flag)
{
	NTSTATUS status = STATUS_SUCCESS;
    if (flag)
    {
		InitializeListHead(&ProtectedProcessList.ListEntry);
        OB_CALLBACK_REGISTRATION ObCallBack = { 0 };
        OB_OPERATION_REGISTRATION ObOpera = { 0 };
        ObCallBack.OperationRegistrationCount = 1;
        ObCallBack.RegistrationContext = NULL;
        ObCallBack.Version = ObGetFilterVersion();
        RtlInitUnicodeString(&ObCallBack.Altitude, L"321000");
        ObOpera.ObjectType = PsProcessType;
        ObOpera.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        ObOpera.PreOperation = (POB_PRE_OPERATION_CALLBACK)(&ProtectProcessCallBack);
        ObCallBack.OperationRegistration = &ObOpera;
        status = ObRegisterCallbacks(&ObCallBack, &ObHandle1);
    }
    else
	{
		PPROTECTED_PROCESS currentProcess = NULL;
		PLIST_ENTRY currentEntry = NULL, nextEntry = NULL;
        currentEntry = ProtectedProcessList.ListEntry.Flink;
        while (currentEntry != &ProtectedProcessList.ListEntry) {
            nextEntry = currentEntry->Flink;
            currentProcess = CONTAINING_RECORD(currentEntry, PROTECTED_PROCESS, ListEntry);
            RemoveEntryList(currentEntry);
            ExFreePoolWithTag(currentProcess, 'cppP');
            currentEntry = nextEntry;
        }
		ObUnRegisterCallbacks(ObHandle1);
	}
    return status;
}

VOID ProcessNotifyRoutineEx(
    PEPROCESS Process,
    HANDLE ProcessId,
    PPS_CREATE_NOTIFY_INFO CreateInfo
)
{

    UCHAR* pszImageFileName = PsGetProcessImageFileName(Process);
    if (CreateInfo == NULL)
    {
        DbgPrint("进程%s退出", pszImageFileName);
		if (MyAdvancedOptions.IsProtectProcess) RemoveProcessFromProtectedList(ProcessId);
    }
    else
    {
        DbgPrint("进程%s创建,父进程PID:%lld", pszImageFileName,(ULONG64)CreateInfo->ParentProcessId);
        if (MyAdvancedOptions.DenyCreateProcess)
        {
            CreateInfo->CreationStatus = STATUS_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY;//阻止进程创建且不会弹窗
            return;
        }
        /*if ((strcmp((const char*)pszImageFileName, "calc.exe") == 0) && 0)
        {
            DbgPrint("已拦截进程calc.exe创建");
            if (0)
            {
                KeWaitForSingleObject(&pKernelEvent, Executive, KernelMode, FALSE, 0);
            }
            //KeWaitForSingleObject(&pKernelEvent, Executive, KernelMode, FALSE, 0);
            //DbgPrint("当前IRQL级别:%d",KeGetCurrnetIRQL());
            CreateInfo->CreationStatus = STATUS_PRINT_CANCELLED;//打印被取消
        }*/
    }
}

void KillProcess(HANDLE dwProcessId)
{
    HANDLE ProcessHandle = NULL;
    OBJECT_ATTRIBUTES obj;
    CLIENT_ID cid = { 0 };
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    InitializeObjectAttributes(&obj, NULL, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    cid.UniqueProcess = dwProcessId;
    cid.UniqueThread = 0;

    ntStatus = ZwOpenProcess(&ProcessHandle, GENERIC_ALL, &obj, &cid);
    if (NT_SUCCESS(ntStatus))
    {
        // 获取进程EPROCESS指针
        PEPROCESS pEProcess = NULL;
        ntStatus = PsLookupProcessByProcessId(dwProcessId, &pEProcess);
        if (!NT_SUCCESS(ntStatus)) {
            DbgPrint("PsLookupProcessByProcessId Error:%d", ntStatus);
            ZwClose(ProcessHandle);
            return;
        }
        // 释放该进程的Rundown锁
        PEX_RUNDOWN_REF pRundownRef = (PEX_RUNDOWN_REF)((PUCHAR)pEProcess + 0x1e8);
        ExReleaseRundownProtection(pRundownRef);
		// +0x1f4 ProcessRundown   : Pos 25, 1 Bit 将该位清0
        (*(PUCHAR)((PUCHAR)pEProcess + 0x1f4)) &= ~0x02000000;

        ntStatus = ZwTerminateProcess(ProcessHandle, 0);
        if (NT_SUCCESS(ntStatus))
            DbgPrint("Process %p terminated successfully after releasing rundown protection.", dwProcessId);
        else
            DbgPrint("Failed to terminate process %p after releasing rundown protection. Error: %d", dwProcessId, ntStatus);
		ObDereferenceObject(pEProcess);
    }
    else
    {
        DbgPrint("ZwOpenProcess Error:%d",ntStatus);
    }
    ZwClose(ProcessHandle);
}

void ForceKillProcess(HANDLE dwProcessId)
{
    PEPROCESS hProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(dwProcessId, &hProcess);
    if (!NT_SUCCESS(status))
    {
		DbgPrint("PsLookupProcessByProcessId Error:%d", status);
		return;
    }
    KAPC_STATE ApcState = { 0 };
    KeStackAttachProcess(hProcess, &ApcState);
	status = ZwTerminateProcess(NULL, 0);
    KeUnstackDetachProcess(&ApcState);
	ObDereferenceObject(hProcess);
}

// 内核线程函数 - 在独立线程中执行内存清零
VOID MemKillProcessThread(PVOID Context)
{
    HANDLE pid = (HANDLE)Context;
    PEPROCESS proc = NULL;
    PKAPC_STATE apcState = NULL;
    MEMORY_BASIC_INFORMATION mbi = { 0 };
    PVOID addr = 0;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // 1. 获取进程对象
    status = PsLookupProcessByProcessId(pid, &proc);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[MemKill] 进程查找失败: 0x%X\n", status);
        PsTerminateSystemThread(status);
        return;
    }

    // 2. 分配APC状态结构
    apcState = (PKAPC_STATE)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KAPC_STATE), 'KPME');
    if (!apcState) {
        DbgPrint("[MemKill] 内存分配失败\n");
        ObDereferenceObject(proc);
        PsTerminateSystemThread(STATUS_NO_MEMORY);
        return;
    }

    __try {
        // 3. 附加到目标进程上下文
        KeStackAttachProcess(proc, apcState);
        DbgPrint("[MemKill] 已附加到进程: %p\n", pid);

        // 4. 精准遍历VAD，只清零已提交的私有内存
        while (ZwQueryVirtualMemory(ZwCurrentProcess(), addr,
            MemoryBasicInformation,
            &mbi, sizeof(mbi), NULL) == STATUS_SUCCESS)
        {
            // 只处理已提交且私有的可写内存（跳过映像/映射文件）
            if (mbi.State == MEM_COMMIT &&
                mbi.Type == MEM_PRIVATE &&
                (mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE)))
            {
                __try {
                    // 批量清零整个内存区域，效率极高
                    memset(mbi.BaseAddress, 0, mbi.RegionSize);
                    DbgPrint("[MemKill] 清零: %p [%llX]\n", mbi.BaseAddress, mbi.RegionSize);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    // 极少数情况可能仍触发异常（如页面属性突变）
                    DbgPrint("[MemKill] 异常跳过: %p\n", addr);
                }
            }

            // 移动到下一个内存区域
            addr = (PUCHAR)mbi.BaseAddress + mbi.RegionSize;
        }

        // 5. 分离进程上下文
        KeUnstackDetachProcess(apcState);
        DbgPrint("[MemKill] 已分离，目标进程 %p 即将崩溃\n", pid);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("[MemKill] 致命异常，强制分离\n");
        if (apcState) {
            KeUnstackDetachProcess(apcState);
        }
    }

    // 6. 释放资源
    ObDereferenceObject(proc);
    if (apcState) {
        ExFreePool(apcState);
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

// ARK调用入口 - 立即返回，不卡死
NTSTATUS MemKillProcess(HANDLE pid)
{
    HANDLE hThread = NULL;
    OBJECT_ATTRIBUTES objAttr = { 0 };
    //CLIENT_ID clientId = { 0 };
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // 验证PID合法性
    if (!pid || pid == PsGetCurrentProcessId()) {
        return STATUS_INVALID_PARAMETER;
    }

    InitializeObjectAttributes(&objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    // 创建独立系统线程执行清理，ARK调用立即返回
    status = PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, &objAttr,
        NULL, NULL, MemKillProcessThread, (PVOID)pid);
    if (NT_SUCCESS(status)) {
        DbgPrint("[ARK] 异步杀进程线程已启动 PID:%p\n", pid);
        ZwClose(hThread); // 关闭句柄，线程继续运行
        return STATUS_SUCCESS;
    }

    DbgPrint("[ARK] 创建线程失败: 0x%X\n", status);
    return status;
}

void KillThread(HANDLE dwThreadId)
{
    //HANDLE ThreadHandle = NULL;
    OBJECT_ATTRIBUTES obj;
    CLIENT_ID cid = { 0 };
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    InitializeObjectAttributes(&obj, NULL, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    cid.UniqueProcess = 0;
    cid.UniqueThread = dwThreadId;

    //ntStatus = ZwOpenThread(&ThreadHandle, GENERIC_ALL, &obj, &cid);
    if (NT_SUCCESS(ntStatus))
    {
        //ZwTerminateThread(ThreadHandle, 0);
    }
    else
    {
        //DbgPrint("ZwOpenThread Error:%d", ntStatus);
    }
    //ZwClose(ThreadHandle);
}

/*BOOLEAN ForceKillProcess(HANDLE dwProcessId)
{
    PEPROCESS hProcess;
    NTSTATUS status = 0;

    //status = PsLookupProcessByProcessId(dwProcessId, &hProcess);
    if (!NT_SUCCESS(status)) return FALSE;

    //status = PsTerminatProcess(hProcess, 0);
    if (!NT_SUCCESS(status))
    {
        //ObDereferenceObject(hProcess);
        return FALSE;
    }
    //ObDereferenceObject(hProcess);
    return TRUE;
}*/

NTSTATUS SetProcessMonitorStatus(BOOLEAN flag)
{
    return PsSetCreateProcessNotifyRoutineEx(ProcessNotifyRoutineEx, (!flag));
}

NTSTATUS OpenProcess(HANDLE ProcessId, PHANDLE hTargetProcess)
{
    PEPROCESS hProcess = NULL;
    NTSTATUS status;

    // 查找进程
    status = PsLookupProcessByProcessId(ProcessId, &hProcess);
    if (!NT_SUCCESS(status)) {
        DbgPrint("PsLookupProcessByProcessId failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)ProcessId, status);
        return status;
    }

    // 打开源进程句柄
	HANDLE hSourceProcess = NULL;
    status = ObOpenObjectByPointer(hProcess, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &hSourceProcess);
    if (!NT_SUCCESS(status)) {
        DbgPrint("ObOpenObjectByPointer failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)ProcessId, status);
        ObDereferenceObject(hProcess);
        return status;
    }

    status = ZwDuplicateObject(hSourceProcess, hSourceProcess, NtCurrentProcess(), hTargetProcess, 0, 0, DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(status)) {
        DbgPrint("ZwDuplicateObject failed for Process ID %lld at Handle %lld, status: 0x%X\n", (ULONG_PTR)ProcessId, (ULONG_PTR)hSourceProcess, status);
        ZwClose(hSourceProcess);
        ObDereferenceObject(hProcess);
        return status;
    }

    // 关闭源句柄
    ZwClose(hSourceProcess);
    ObDereferenceObject(hProcess);
	return status;
}

NTSTATUS GetProcessHandleCount(HANDLE ProcessId, PULONG pHandleCount) {
    NTSTATUS status;
	if (pHandleCount == NULL) return STATUS_INVALID_PARAMETER;
    *pHandleCount = 0;
    HANDLE ProcessHandle = NULL;
	status = OpenProcess(ProcessId, &ProcessHandle);
	if (!NT_SUCCESS(status)) return status;
    return ZwQueryInformationProcess(ProcessHandle, ProcessHandleCount, pHandleCount, sizeof(*pHandleCount), NULL);
}

PEPROCESS GetEProcess(HANDLE ProcessId)
{
	PEPROCESS pEProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &pEProcess);
	if (status != STATUS_SUCCESS || pEProcess == NULL)
    {
		DbgPrint("PsLookupProcessByProcessId ProcessId:%Iu Error:0x%X", (ULONG_PTR)ProcessId, status);
        return NULL;
    }
    if (PsGetProcessExitStatus(pEProcess) != STATUS_PENDING)
    {
        ObDereferenceObject(pEProcess);
        return NULL;
    }
	ObDereferenceObject(pEProcess);
	DbgPrint("EPROCESS of Process %Iu:0x%p", (ULONG_PTR)ProcessId, pEProcess);
	return pEProcess;
}

NTSTATUS GetProcessImageName(
    HANDLE ProcessId,
    LPWSTR ProcessImageName
)
{
    NTSTATUS status;
    ULONG returnedLength;
    //ULONG bufferLength;

	// Open the process handle
	HANDLE ProcessHandle = NULL;
	status = OpenProcess(ProcessId, &ProcessHandle);
	if (!NT_SUCCESS(status)) return status;
	// Check if the process handle is valid
	if (ProcessHandle == NULL) {
		DbgPrint("OpenProcess failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)ProcessId, status);
		return STATUS_ACCESS_DENIED;
	}

	// Initialize the process path
	RtlZeroMemory(ProcessImageName, 260 * sizeof(WCHAR));

    // Query the actual size of the process path
    status = ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, NULL, 0, &returnedLength);
    if (STATUS_INFO_LENGTH_MISMATCH != status) return status;

    // Retrieve the process path from the handle to the process
    status = ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, ProcessImageName, returnedLength, &returnedLength);
    if (NT_SUCCESS(status)) status = STATUS_SUCCESS;

    return status;
}

static BOOLEAN ProcessEnumCallback(
	HANDLEINFO* pHandleInfo,
	PVOID Context
)
{
    UNREFERENCED_PARAMETER(Context);
    static ULONG ProcessCount = 0;
    if (!pHandleInfo->Object || !MmIsAddressValid(pHandleInfo->Object))
        return TRUE;  // 如果无效，继续枚举下一个
    _try{
        if (ObGetObjectType(pHandleInfo->Object) == *PsProcessType) {
            PENUM_PROCESSES_CONTEXT EnumContext = (PENUM_PROCESSES_CONTEXT)Context;
            EnumContext->ProcessArray[EnumContext->ProcessCount++] = pHandleInfo->Handle;
            DbgPrint("PID:%Iu, Object:%p", (ULONG_PTR)pHandleInfo->Handle, pHandleInfo->Object);
        }
    }
    _except(EXCEPTION_EXECUTE_HANDLER){
        //DbgPrint("访问对象时发生异常");
	}
    return TRUE; // 返回 TRUE 以继续枚举
}

ULONG EnumProcesses(PHANDLE Array)
{
	PVOID PspCidTable = GetPspCidTable();
	if (PspCidTable == NULL) return 0;

	ENUM_PROCESSES_CONTEXT EnumContext = { 0 };
	EnumContext.ProcessArray = Array;
    MyExEnumHandleTable(
        PspCidTable,
        ProcessEnumCallback,
        &EnumContext
	);
	return EnumContext.ProcessCount;
}