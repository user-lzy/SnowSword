#pragma once

#include "Thread.h"

HANDLE MyProcessId = 0;
PVOID ObHandle2 = NULL;

PETHREAD GetFirstThreadFromProcess(PEPROCESS TargetProcess)
{
    PETHREAD firstThread = NULL;

    // 方法1：使用PsGetNextProcessThread（推荐）
	// 声明函数指针PsGetNextProcessThread
	PETHREAD (*PsGetNextProcessThread)(PEPROCESS Process, PETHREAD Thread) = (PETHREAD(*)(PEPROCESS, PETHREAD))FindPsGetNextProcessThread();
    if (PsGetNextProcessThread == NULL) return NULL;

    firstThread = PsGetNextProcessThread(TargetProcess, NULL);
    if (firstThread)
    {
        // 增加引用计数
        ObReferenceObject(firstThread);
        return firstThread;
    }

	return NULL;
}

/*NTSTATUS InjectDllViaApc(
    ULONG TargetPid,
    PCWSTR DllPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS targetProcess = NULL;
    PETHREAD targetThread = NULL;
    PKAPC apc = NULL;
    PVOID remoteBuffer = NULL;

    // 1. 获取目标进程
    status = PsLookupProcessByProcessId(
        (HANDLE)TargetPid,
        &targetProcess);
    if (!NT_SUCCESS(status)) return status;

    // 2. 获取目标线程（示例：遍历所有线程）
    // 实际应用中需要选择合适的线程

    // 3. 分配内核内存存储DLL路径
    SIZE_T pathSize = (wcslen(DllPath) + 1) * sizeof(WCHAR);
    remoteBuffer = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        pathSize,
        'dllp');

    // 4. 复制路径到内核缓冲区
    RtlCopyMemory(remoteBuffer, DllPath, pathSize);

    // 5. 获取LoadLibraryW地址
    ULONG_PTR loadLibraryAddr = GetLoadLibraryAddress(targetProcess);
    if (!loadLibraryAddr)
    {
        status = STATUS_NOT_FOUND;
        goto cleanup;
    }

    // 6. 分配并初始化APC
    apc = (PKAPC)ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(KAPC),
        'apct');

    KeInitializeApc(
        apc,
        targetThread,
        OriginalApcEnvironment,
        KernelApcRoutine,
        NULL,
        (PKNORMAL_ROUTINE)loadLibraryAddr,
        UserMode,
        remoteBuffer);

    // 7. 插入APC队列
    if (!KeInsertQueueApc(apc, NULL, NULL, 0))
    {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

cleanup:
    if (!NT_SUCCESS(status))
    {
        if (remoteBuffer) ExFreePool(remoteBuffer);
        if (apc) ExFreePool(apc);
    }
    if (targetProcess) ObDereferenceObject(targetProcess);

    return status;
}*/

PVOID FindPsSuspendThread()
{
	UNICODE_STRING PsSuspendProcessName = RTL_CONSTANT_STRING(L"PsSuspendProcess");
	PVOID PsSuspendProcessAddr = MmGetSystemRoutineAddress(&PsSuspendProcessName);

    if (NULL == PsSuspendProcessAddr)
    {
        DbgPrint("PsSuspendProcessAddr is NULL");
        return NULL;
	}

    /*
    fffff805`d35cd59f 488bc8          mov     rcx,rax
    fffff805`d35cd5a2 e839010000      call    nt!PsSuspendThread (fffff805`d35cd6e0)
    */
	UCHAR pSpecialCode[4] = { 0x48, 0x8B, 0xC8, 0xE8 };
	PVOID result = SearchSpecialCode(PsSuspendProcessAddr, 0x100, pSpecialCode, 4);
    if (NULL == result)
    {
        DbgPrint("PsSuspendThread is NULL");
        return NULL;
	}
	// 计算目标地址
    ULONG offset = *(PULONG)((PUCHAR)result + 4);
	return (PVOID)((PUCHAR)result + 8 + offset);
    //offset = (ULONG(64)(0xFFFFFFFF00000000ULL | (offset & 0xFFFFFFFFULL));)
}

VOID RemoveThreadHandleAccess(PACCESS_MASK pDesiredAccess) {
    if (*pDesiredAccess & THREAD_TERMINATE) *pDesiredAccess &= ~THREAD_TERMINATE;
    if (*pDesiredAccess & THREAD_SUSPEND_RESUME) *pDesiredAccess &= ~THREAD_SUSPEND_RESUME;
    if (*pDesiredAccess & THREAD_SET_CONTEXT) *pDesiredAccess &= ~THREAD_SET_CONTEXT;
    if (*pDesiredAccess & THREAD_SET_INFORMATION) *pDesiredAccess &= ~THREAD_SET_INFORMATION;
}

// 回调函数
POB_PREOP_CALLBACK_STATUS ProtectThreadCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
	UNREFERENCED_PARAMETER(RegistrationContext);
    // 检查是否为线程创建操作
	switch (OperationInformation->Operation) {
    case OB_OPERATION_HANDLE_CREATE:
		if (PsGetProcessId(IoThreadToProcess((PETHREAD)OperationInformation->Object)) == MyProcessId) {
            // 修改操作结果以阻止创建句柄
            RemoveThreadHandleAccess(&OperationInformation->Parameters->CreateHandleInformation.DesiredAccess);
        }
	case OB_OPERATION_HANDLE_DUPLICATE:
		if (PsGetProcessId(IoThreadToProcess((PETHREAD)OperationInformation->Object)) == MyProcessId) {
			// 修改操作结果以阻止复制句柄
			RemoveThreadHandleAccess(&OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess);
		}
    }
    return OB_PREOP_SUCCESS;
}

NTSTATUS SetThreadProtectionStatus(BOOLEAN flag)
{
    NTSTATUS status = STATUS_SUCCESS;
    if (flag)
    {
        OB_CALLBACK_REGISTRATION ObCallBack = { 0 };
        OB_OPERATION_REGISTRATION ObOpera = { 0 };
        ObCallBack.OperationRegistrationCount = 1;
        ObCallBack.RegistrationContext = NULL;
        ObCallBack.Version = ObGetFilterVersion();
        RtlInitUnicodeString(&ObCallBack.Altitude, L"321000");
        ObOpera.ObjectType = PsThreadType;
        ObOpera.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        ObOpera.PreOperation = (POB_PRE_OPERATION_CALLBACK)(&ProtectThreadCallback);
        ObCallBack.OperationRegistration = &ObOpera;
        status = ObRegisterCallbacks(&ObCallBack, &ObHandle2);
    }
    else
    {
        ObUnRegisterCallbacks(ObHandle2);
    }
    return status;
}

VOID DenyCreateThread(HANDLE ThreadId)
{
    PETHREAD Thread = NULL;
    if (NT_SUCCESS(PsLookupThreadByThreadId(ThreadId, &Thread)))
    {
        // 获取线程的Win32StartAddress
        PVOID pWin32Address = *(UCHAR**)((UCHAR*)Thread + 0x1c8); // 64位系统下的偏移
        if (MmIsAddressValid(pWin32Address))
        {
            // 修改线程的起始地址为 ret 指令 (0xC3)
            *(PUCHAR)pWin32Address = 0xC3;
            DbgPrint("已修改线程起始地址为 ret 指令，阻止线程运行\n");
        }
        ObDereferenceObject(Thread);

    }
}

NTSTATUS OpenThread(HANDLE ThreadId, PHANDLE hTargetThreadHandle)
{
    NTSTATUS status;

    // 查找线程
    PETHREAD hThread = NULL;
    status = PsLookupThreadByThreadId(ThreadId, &hThread);
    if (!NT_SUCCESS(status)) {
        DbgPrint("PsLookupThreadByThreadId failed for Thread ID %lld, status: 0x%X\n", (ULONG_PTR)ThreadId, status);
        return status;
    }

    // 打开源线程句柄
    HANDLE hSourceThreadHandle = NULL;
    status = ObOpenObjectByPointer(hThread, OBJ_KERNEL_HANDLE, NULL, THREAD_ALL_ACCESS, *PsThreadType, KernelMode, &hSourceThreadHandle);
    if (!NT_SUCCESS(status)) {
        DbgPrint("ObOpenObjectByPointer failed for Thread ID %lld, status: 0x%X\n", (ULONG_PTR)ThreadId, status);
        ObDereferenceObject(hThread);
        return status;
    }
    
	//使用IoThreadToProcess获取线程所属进程
	PEPROCESS hProcess = IoThreadToProcess(hThread);
	if (hProcess == NULL) {
		DbgPrint("IoThreadToProcess failed for Thread ID %lld, status: 0x%X\n", (ULONG_PTR)ThreadId, status);
		ZwClose(hSourceThreadHandle);
		ObDereferenceObject(hThread);
		return status;
	}

	// 打开源进程句柄
	HANDLE hSourceProcessHandle = NULL;
	status = ObOpenObjectByPointer(hProcess, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &hSourceProcessHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("ObOpenObjectByPointer failed for Process ID %lld, status: 0x%X\n", (ULONG_PTR)PsGetProcessId(hProcess), status);
		ZwClose(hSourceThreadHandle);
		ObDereferenceObject(hThread);
        //ObDereferenceObject(hProcess);
		return status;
	}

    // 复制源线程句柄到目标进程
    status = ZwDuplicateObject(hSourceProcessHandle, hSourceThreadHandle, NtCurrentProcess(), hTargetThreadHandle, 0, 0, DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(status)) {
        DbgPrint("ZwDuplicateObject failed for Thread ID %lld at Handle %lld, status: 0x%X\n", (ULONG_PTR)ThreadId, (ULONG_PTR)hSourceThreadHandle, status);
        ZwClose(hSourceThreadHandle);
        ZwClose(hSourceProcessHandle);
        ObDereferenceObject(hThread);
        //ObDereferenceObject(hProcess);
        return status;
    }

    // 关闭源句柄
    ZwClose(hSourceThreadHandle);
    ZwClose(hSourceProcessHandle);
    ObDereferenceObject(hThread);
    //ObDereferenceObject(hProcess);
    return status;
}

// 线程创建通知回调函数 
VOID ThreadCreateNotifyRoutine(
    HANDLE ProcessId,
    HANDLE ThreadId,
    BOOLEAN Create
) {
	UNREFERENCED_PARAMETER(ThreadId);
    if (Create) {
        PEPROCESS eprocess = NULL;
        NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &eprocess);
        if (NT_SUCCESS(status)) {
            PUCHAR ProcessName1 = PsGetProcessImageFileName(eprocess);// 获取新添线程的进程名 
            PUCHAR ProcessName2 = PsGetProcessImageFileName(PsGetCurrentProcess());// 获取当前创建线程的进程名 
            if ((ProcessName1 == NULL) || (ProcessName2 == NULL)) return;

            // 检查是否为远程线程注入 
            // 这里简单通过判断创建线程的进程和当前进程是否一致来检测 
            // 实际应用中，需要更复杂的逻辑
            //DbgPrint("Process %s Create Thread %p", ProcessName1, ThreadId);
            //if (eprocess != PsGetCurrentProcess()) DbgPrint("可能存在远程线程注入！可疑进程: %s，目标进程: %s, 线程ID: %p\n", ProcessName2, ProcessName1, ThreadId);
            //if (ProcessId != PsGetCurrentProcessId() && PsGetCurrentProcessId() != (HANDLE)4) DbgPrint("可能存在远程线程注入！可疑进程: %s，目标进程: %s", ProcessName2, ProcessName1);
            ObDereferenceObject(eprocess);
        }
    }
}

NTSTATUS SetThreadMonitorStatus(BOOLEAN flag)
{
    if (flag)
        return PsSetCreateThreadNotifyRoutine(ThreadCreateNotifyRoutine);
    else
        return PsRemoveCreateThreadNotifyRoutine(ThreadCreateNotifyRoutine);
}

PVOID FindPspExitThread()
{
    UNICODE_STRING PsTerminateSystemThreadName = RTL_CONSTANT_STRING(L"PsTerminateSystemThread");
    PVOID PsTerminateSystemThreadAddr = MmGetSystemRoutineAddress(&PsTerminateSystemThreadName);
    if (NULL == PsTerminateSystemThreadAddr)
    {
        DbgPrint("PsTerminateSystemThreadAddr is NULL");
        return NULL;
    }

    // 设置起始位置
    PUCHAR StartSearchAddress = (PUCHAR)PsTerminateSystemThreadAddr;
    // 设置搜索长度
    ULONG size = 0x28;
    // 指定特征码
    UCHAR pSpecialCode[256] = { 0 };
    // 指定特征码长度
    ULONG ulSpecialCodeLength = 1;
    /*
    fffff802`3914d8bf e8ccfdf9ff      call    nt!PspTerminateThreadByPointer (fffff802`390ed690)
    */
    pSpecialCode[0] = 0xe8;
    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("PspTerminateThreadByPointer is NULL");
        return NULL;
    }
	DbgPrint("result: 0x%p\n", result);
    for (int i=0; i < 10; i++) {
        DbgPrint("byte[%d]: %02x\n", i, *((PUCHAR)result + i));
	}
    // 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 1);
	DbgPrint("offset: 0x%ld\n", offset);
    //offset = (ULONG64)(0xFFFFFFFF00000000ULL | (offset & 0xFFFFFFFFULL));
	PVOID PspTerminateThreadByPointer = (PVOID)((PUCHAR)result + 5 + offset);
    DbgPrint("PspTerminateThreadByPointer: 0x%p \n", PspTerminateThreadByPointer);
    //return NULL;
    // 设置起始位置
    StartSearchAddress = (PUCHAR)PspTerminateThreadByPointer;
    // 设置搜索长度
    size = 0x60;
    // 指定特征码长度
    ulSpecialCodeLength = 3;
    /*
    fffff805`774ed6dc 8bce            mov     ecx,esi
    fffff805`774ed6de e8d1eeffff      call    nt!PspExitThread (fffff805`774ec5b4)
    */
    // 指定特征码
    pSpecialCode[0] = 0x8b;
	pSpecialCode[1] = 0xce;
	pSpecialCode[2] = 0xe8;
    // 开始搜索,找到后返回首地址
    result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("PspExitThread is NULL");
        return NULL;
    }
	DbgPrint("result: 0x%p\n", result);
    // 计算目标地址
	//ULONG offset1 = *(PULONG)((PUCHAR)result + 3);
    LONG offset1 = *(PLONG)((PUCHAR)result + 3);
    //offset = (ULONG64)(0xFFFFFFFF00000000ULL | (offset & 0xFFFFFFFFULL));
	//DbgPrint("offset: 0x%p\n", offset);
    PVOID PspExitThread = (PVOID)((PUCHAR)result + 7 + offset1);
	DbgPrint("PspExitThread: 0x%p\n", PspExitThread);
    //return NULL;
    return PspExitThread;
}

/*BOOLEAN MmIsAddressExecutable(PVOID Address) {
    MEMORY_BASIC_INFORMATION MemoryInfo;
    SIZE_T ReturnLength = 0;

    // 查询内存保护属性
    NTSTATUS Status = ZwQueryVirtualMemory(
        NtCurrentProcess(),
        Address,
        MemoryBasicInformation,
        &MemoryInfo,
        sizeof(MemoryInfo),
        &ReturnLength
    );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("ZwQueryVirtualMemory failed with status: 0x%X\n", Status);
        return FALSE;
    }

    // 检查保护标志是否包含 PAGE_EXECUTE
    if (MemoryInfo.Protect & PAGE_EXECUTE) return TRUE;

    return FALSE;
}*/

VOID KillThreadRoutine(IN PKAPC Apc, IN PKNORMAL_ROUTINE* NormalRoutine, IN PVOID* NormalContext, IN PVOID* SystemArgument1, IN PVOID* SystemArgument2) {
	UNREFERENCED_PARAMETER(Apc);
	UNREFERENCED_PARAMETER(NormalRoutine);
	UNREFERENCED_PARAMETER(NormalContext);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);
    typedef VOID(*_PspExitThread)(IN NTSTATUS ExitStatus);
    _PspExitThread PspExitThread = (_PspExitThread)FindPspExitThread();
    //ExFreePoolWithTag(Apc, '1111');  // 释放 APC
	DbgPrint("准备终止线程...");
    if (NULL == PspExitThread) {
        DbgPrint("FindPspExitThread is NULL");
        return;
    }
    
    PspExitThread(STATUS_SUCCESS);   // 调用 PspExitThread 终止线程
}

PETHREAD GetEThread(HANDLE ThreadId)
{
    PETHREAD pEThread = NULL;
    NTSTATUS status = PsLookupThreadByThreadId(ThreadId, &pEThread);
    if (status != STATUS_SUCCESS)
    {
        DbgPrint("PsLookupThreadByThreadId TID:%Iu, Error:%d", (ULONG_PTR)ThreadId, status);
        return NULL;
    }
    ObDereferenceObject(pEThread);
    //DbgPrint("ETHREAD:0x%p", pEThread);
    return pEThread;
}

NTSTATUS ForceKillThread(HANDLE ThreadId)
{
    PETHREAD pEThread = GetEThread(ThreadId);//打开线程
    if (pEThread == NULL) return FALSE;

    BOOLEAN status;
    PKAPC ExitApc = NULL;
    ExitApc = (PKAPC)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KAPC), '1111');
    if (ExitApc == NULL)
    {
        DbgPrint("[KillProcessWithApc] malloc memory failed \n");
        return FALSE;
    }
    KeInitializeApc(ExitApc, pEThread, OriginalApcEnvironment, KillThreadRoutine, NULL, NULL, KernelMode, NULL);//为线程初始化APC
	//KeAlertThread(pEThread, KernelMode); //警告线程
    status = KeInsertQueueApc(ExitApc, ExitApc, NULL, 2);   //插入Apc到线程队列
    if (!status) DbgPrint("KeInsertQueueApc failed. status: %X\n", status);
    // 确保线程退出
    //KeWaitForSingleObject(pEThread, Executive, KernelMode, FALSE, NULL); // 结束主线程时会卡死
	return (status == TRUE);
}

static BOOLEAN ThreadEnumCallback(
    HANDLEINFO* pHandleInfo,
    PVOID Context
)
{
    UNREFERENCED_PARAMETER(Context);
    static ULONG ThreadCount = 0;
    if (!pHandleInfo->Object || !MmIsAddressValid(pHandleInfo->Object))
        return TRUE;  // 如果无效，继续枚举下一个
    _try{
        if (ObGetObjectType(pHandleInfo->Object) == *PsThreadType) {
            PENUM_THREAD_CONTEXT EnumContext = (PENUM_THREAD_CONTEXT)Context;
            EnumContext->ThreadArray[EnumContext->ThreadCount++] = pHandleInfo->Handle;
            DbgPrint("TID:%Iu, Object:%p", (ULONG_PTR)pHandleInfo->Handle, pHandleInfo->Object);
        }
    }
        _except(EXCEPTION_EXECUTE_HANDLER) {
        //DbgPrint("访问对象时发生异常");
    }
    return TRUE; // 返回 TRUE 以继续枚举
}

ULONG EnumThreads(PHANDLE Array)
{
    PVOID PspCidTable = GetPspCidTable();
    if (PspCidTable == NULL) return 0;

	ENUM_THREAD_CONTEXT EnumContext = { 0 };
	EnumContext.ThreadArray = Array;
    MyExEnumHandleTable(
        PspCidTable,
        ThreadEnumCallback,
        &EnumContext
    );
	return EnumContext.ThreadCount;
}