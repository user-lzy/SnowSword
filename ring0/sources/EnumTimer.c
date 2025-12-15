#include "EnumTimer.h"
#include "Module.h"
#include "ObjectInfo.h"

ULONG EnumProcessTimers(HANDLE PID, PPROCESS_TIMER Array)
{
    ULONG count = 0;
    //PETHREAD Thread = NULL ;
    PEPROCESS Process = NULL;
    NTSTATUS status;

    DbgPrint("3");

    status = PsLookupProcessByProcessId(PID, &Process);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Open Process %Iu Failed.status:%X.",(ULONG_PTR)PID, status);
        return 0;
    }

    typedef PETHREAD(*PsGetNextProcessThreadFunc)(
        _In_ PEPROCESS Process,  // 目标进程的EPROCESS对象
        _In_ PETHREAD  Thread    // 当前线程的ETHREAD对象（若为NULL则从头开始遍历）
        );

    PsGetNextProcessThreadFunc PsGetNextProcessThread = (PsGetNextProcessThreadFunc)FindPsGetNextProcessThread();
    if (!PsGetNextProcessThread) {
        DbgPrint("获取PsGetNextProcessThread失败!");
        goto Cleanup;
    }

    PEPROCESS CurrentProcess = PsGetCurrentProcess();
    /*HANDLE CsrPid = GetCsrPid();
    if (CsrPid == NULL) {
        DbgPrint("获取Csrss PID失败!");
        goto Cleanup;
    }
	DbgPrint("Csrss PID:%d", (ULONG)(ULONG_PTR)CsrPid);
    status = PsLookupProcessByProcessId(&CsrProcess, CsrPid);
    if (!NT_SUCCESS(status)) {
        DbgPrint("打开Csrss失败!status:%X", status);
        goto Cleanup;
    }*/
    KAPC_STATE ApcState;
    KeStackAttachProcess(CurrentProcess, &ApcState);
	//UNICODE_STRING win32kbaseName = RTL_CONSTANT_STRING(L"win32kbase.sys");
    PLIST_ENTRY gtmrListHead = NULL;// (PLIST_ENTRY)GetProcAddress("gtmrListHead", GetModuleBase(win32kbaseName));
    KeUnstackDetachProcess(&ApcState);

    if (gtmrListHead == NULL) {
        DbgPrint("获取gtmrListHead失败!");
        goto Cleanup;//这个不是hash链表
    }

    DbgPrint("gtmrListHead:0x%p", gtmrListHead);

    //do {
        //Thread = PsGetNextProcessThread(Process, Thread);

        for (PLIST_ENTRY entry = gtmrListHead->Flink; entry != gtmrListHead; entry = entry->Flink) {

            ptimer_t item = CONTAINING_RECORD(entry, timer_t, list1);
            //这个地方疑似不能解引用 有时候PageFault
            //注意 这里的定时器有可能属于hwnd==0的 因此最好判断threadInfo
			__try
            {

                //if ((*(PETHREAD*)(item->head.threadInfo)) == Thread) {
                    DbgPrint("找到一个定时器对象:0x%p,线程ID:%Iu,周期:%d,函数入口:0x%p", item, (ULONG_PTR)PsGetThreadId((PETHREAD)item->threadObject), item->elapse, item->pfn);
                    Array[count].ThreadId = PsGetThreadId((PETHREAD)item->threadObject);
                    Array[count].Period = item->elapse;
                    Array[count].Func = item->pfn;
                    count++;
                //}
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("error status:%X", GetExceptionCode());
            }
        }
        //if (Thread != NULL) ObDereferenceObject(Thread);
    //} while (Thread == NULL);
    Cleanup:
    //ObDereferenceObject(Process);
    return count;
}

PVOID FindIopTimerQueueHead()
{
    UNICODE_STRING IoInitializeTimerName = RTL_CONSTANT_STRING(L"IoInitializeTimer");
    PVOID IoInitializeTimerAddr = MmGetSystemRoutineAddress(&IoInitializeTimerName);
    if (NULL == IoInitializeTimerAddr)
    {
        DbgPrint("IoInitializeTimerAddr is NULL");
        return NULL;
    }
    DbgPrint("IoInitializeTimerAddr: %p", IoInitializeTimerAddr);

    // LyShark 开始定位特征

    // 设置起始位置
    PUCHAR StartSearchAddress = (PUCHAR)IoInitializeTimerAddr;

    // 设置搜索长度
    ULONG size = 0x100;

    // 指定特征码
    UCHAR pSpecialCode[256] = { 0x48,0x8d,0x0d };

    // 指定特征码长度
    ULONG ulSpecialCodeLength = 3;

	// 打印从IoInitializeTimerAddr + 0x4b开始的20字节内容
    for (ULONG i = 0x65; i <= 0xa5; i++) DbgPrint("Byte %02x: %02x", i, *((PUCHAR)IoInitializeTimerAddr + i));

    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("IopTimerQueueHeadAddr is NULL");
        return NULL;
    }
    // 计算目标地址
    ULONG offset = *(PULONG)((PUCHAR)result + 3);
    PVOID IopTimerQueueHeadAddr = (PVOID)((PUCHAR)result + 7 + offset);

    DbgPrint("IopTimerQueueHead首地址: 0x%p \n", IopTimerQueueHeadAddr);
    return IopTimerQueueHeadAddr;
}

void EnumIoTimers(PSYSTEM_TIMER SystemTimers)
{
    PLIST_ENTRY IopTimerQueueHead = (PLIST_ENTRY)FindIopTimerQueueHead();
    // 枚举列表
    KIRQL OldIrql;
    ULONG i = 0;

    // 获得特权级
    OldIrql = KeRaiseIrqlToDpcLevel();

    __try
    {
        if (IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))
        {
            PLIST_ENTRY NextEntry = IopTimerQueueHead->Flink;
            while (MmIsAddressValid(NextEntry) && NextEntry != (PLIST_ENTRY)IopTimerQueueHead)
            {
                PIO_TIMER Timer = CONTAINING_RECORD(NextEntry, IO_TIMER, TimerList);

                if (Timer && MmIsAddressValid(Timer))
                {
                    RtlStringCbCopyW(SystemTimers[i].Name, sizeof(SystemTimers[i].Name), L"IoTimer");
                    SystemTimers[i].TimerObject = (PVOID)Timer;
                    SystemTimers[i].Func = Timer->TimerRoutine;
                    SystemTimers[i].Flag = Timer->TimerFlag;
                    SystemTimers[i].Type = Timer->Type;
                    DbgPrint("IoTimer对象地址: 0x%p\n", Timer);
                    DbgPrint("IoTimer回调地址: 0x%p\n", Timer->TimerRoutine);
                    i++;
                }
                NextEntry = NextEntry->Flink;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("error status:%X", GetExceptionCode());
    }

    // 恢复特权级
    KeLowerIrql(OldIrql);
}

PVOID FindKeSetTimerEx()
{
    UNICODE_STRING KeSetTimerName = RTL_CONSTANT_STRING(L"KeSetTimer");
    PVOID KeSetTimerAddr = MmGetSystemRoutineAddress(&KeSetTimerName);
    if (NULL == KeSetTimerAddr)
    {
        DbgPrint("KeSetTimerAddr is NULL");
        return NULL;
    }
    //DbgPrint("KeSetTimerAddr: %p", KeSetTimerAddr);

    // LyShark 开始定位特征

    // 设置起始位置
    PUCHAR StartSearchAddress = (PUCHAR)KeSetTimerAddr;

    // 设置搜索长度
    ULONG size = 0x20;

    // 指定特征码
    UCHAR pSpecialCode[256] = { 0xe8 };

    // 指定特征码长度
    ULONG ulSpecialCodeLength = 1;

    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("KeSetTimerExAddr is NULL");
        return NULL;
    }
    DbgPrint("KeSetTimerEx result:0x%p", result);
    // 计算目标地址
    ULONG offset = *(PULONG)((PUCHAR)result + 1);
    PVOID KeSetTimerExAddr = (PVOID)((PUCHAR)result + 5 + offset);

    DbgPrint("KeSetTimerEx地址: 0x%p \n", KeSetTimerExAddr);
    return KeSetTimerExAddr;
}

VOID FindKiWaitXXX(PVOID KeSetTimerEx, PVOID* KiWaitNever, PVOID* KiWaitAlways)
{

    // LyShark 开始定位特征

    if (NULL == KeSetTimerEx) return;
    
    // 设置起始位置
    PUCHAR StartSearchAddress = (PUCHAR)KeSetTimerEx;

    // 设置搜索长度
    ULONG size = 0x60;

    // 指定特征码
    UCHAR pSpecialCode[256] = { 0x48,0x8b,0x05 };

    // 指定特征码长度
    ULONG ulSpecialCodeLength = 3;

    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("正在尝试第二种搜索方法...");
		UCHAR pSpecialCode1[1] = { 0xe8 };
        result = SearchSpecialCode(KeSetTimerEx, size, pSpecialCode1, 1);
        if (result == NULL)
        {
            DbgPrint("KiSetTimerExAddr is NULL");
            return;
		}
		DbgPrint("KiSetTimerEx第二种搜索方法 result:0x%p", result);
		ULONG offset = *(PULONG)((PUCHAR)result + 1);
		PVOID KiSetTimerEx = (PVOID)((PUCHAR)result + 5 + offset);
		result = SearchSpecialCode((PUCHAR)KiSetTimerEx, 0x60, pSpecialCode, ulSpecialCodeLength);
        if (result == NULL) {
			DbgPrint("KiWaitNeverAddr is NULL");
            return;
        }
		DbgPrint("KiWaitNever第二种搜索方法 result:0x%p", result);
		offset = *(PULONG)((PUCHAR)result + 3);
		*KiWaitNever = (PVOID)((PUCHAR)result + 7 + offset);
        DbgPrint("KiWaitNever首地址: 0x%p \n", *KiWaitNever);
        offset = *(PULONG)((PUCHAR)result + 10 + 3);
        *KiWaitAlways = (PVOID)((PUCHAR)result + 10 + 7 + offset);
		DbgPrint("KiWaitAlways首地址: 0x%p \n", *KiWaitAlways);
    }
    // 计算目标地址
	DbgPrint("KiWaitNever result:0x%p", result);
    ULONG offset = *(PULONG)((PUCHAR)result + 3);
    *KiWaitNever = (PVOID)((PUCHAR)result + 7 + offset);
    offset = *(PULONG)((PUCHAR)result + 10 + 3);
    *KiWaitAlways = (PVOID)((PUCHAR)result + 10 + 7 + offset);

    DbgPrint("KiWaitNever首地址: 0x%p \n", *KiWaitNever);
    DbgPrint("KiWaitAlways首地址: 0x%p \n", *KiWaitAlways);
}

void EnumDpcTimers(PSYSTEM_TIMER SystemTimers)
{
    // 获取 CPU 核心数
    int i_cpuNum = KeNumberProcessors;
    int k = 0;
    DbgPrint("CPU核心数: %d \n", i_cpuNum);

    for (KAFFINITY i = 0; i < i_cpuNum; i++)
    {
		DbgPrint("CPU核心: %llx \n", i);
        // 线程绑定特定 CPU
        KeSetSystemAffinityThread(i + 1);

        // 获得 KPRCB 的地址
        ULONG64 p_PRCB = (ULONG64)__readmsr(0xC0000101) + 0x20;
        if (!MmIsAddressValid((PVOID64)p_PRCB))
        {
			DbgPrint("Get PRCB Failed!");
            return;
        }
		DbgPrint("PRCB地址: 0x%llx \n", p_PRCB);

        // 取消绑定 CPU
        KeRevertToUserAffinityThread();

        // 判断操作系统版本
        RTL_OSVERSIONINFOEXW OSVersion = { 0 };
        OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
        RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

        // 计算 TimerTable 在 _KPRCB 结构中的偏移
        PKTIMER_TABLE p_TimeTable = (PKTIMER_TABLE)(*(PULONG64)p_PRCB + 0x3c00);
        /*if (OSVersion.dwMajorVersion == 10 && OSVersion.dwMinorVersion == 0)
        {
            // Windows 10
            p_TimeTable = (PKTIMER_TABLE)(*(PULONG64)p_PRCB + 0x3680);
        }
        else if (OSVersion.dwMajorVersion == 6 && OSVersion.dwMinorVersion == 1)
        {
            // Windows 7
            p_TimeTable = (PKTIMER_TABLE)(*(PULONG64)p_PRCB + 0x2200);
        }
        else
        {
			DbgPrint("Unsupported OS Version!");
            return;
        }*/

        __try
        {
            // 遍历 TimerEntries[] 数组（大小 256）
            for (int j = 0; j < 256; j++)
            {
                // 获取 Entry 双向链表地址
                if (!MmIsAddressValid((PVOID64)p_TimeTable)) continue;

                PLIST_ENTRY p_ListEntryHead = &(p_TimeTable->TimerEntries[j].Entry);

                // 遍历 Entry 双向链表
                for (PLIST_ENTRY p_ListEntry = p_ListEntryHead->Flink; p_ListEntry != p_ListEntryHead; p_ListEntry = p_ListEntry->Flink)
                {
                    // 根据 Entry 取 _KTIMER 对象地址
                    if (!MmIsAddressValid((PVOID64)p_ListEntry)) continue;

                    PKTIMER p_Timer = CONTAINING_RECORD(p_ListEntry, KTIMER, TimerListEntry);

                    // 硬编码取 KiWaitNever 和 KiWaitAlways 
                    ULONG_PTR never = 0, always = 0, KeSetTimerEx = (ULONG_PTR)FindKeSetTimerEx();
                    FindKiWaitXXX((PVOID)KeSetTimerEx, (PVOID*)&never, (PVOID*)&always);
                    if (never == 0 || always == 0)
                    {
                        DbgPrint("Get KiWaitNever or KiWaitAlways Failed!");
                        return;
                    }
					//DbgPrint("KiWaitNever: 0x%llX | KiWaitAlways: 0x%llX \n", never, always);

                    // 获取解密前的 Dpc 对象
                    if (!MmIsAddressValid((PVOID64)p_Timer)) continue;

                    ULONG_PTR ul_Dpc = (ULONG_PTR)p_Timer->Dpc;
                    INT i_Shift = (*((PULONG_PTR)never) & 0xFF);

                    // 解密 Dpc 对象
                    ul_Dpc ^= *((ULONG_PTR*)never);			// 异或
                    ul_Dpc = _rotl64(ul_Dpc, i_Shift);		// 循环左移
                    ul_Dpc ^= (ULONG_PTR)p_Timer;			// 异或
                    ul_Dpc = _byteswap_uint64(ul_Dpc);		// 颠倒顺序
                    ul_Dpc ^= *((ULONG_PTR*)always);		// 异或

                    // 对象类型转换
                    PKDPC p_Dpc = (PKDPC)ul_Dpc;
					RtlStringCbCopyW(SystemTimers[k].Name, sizeof(SystemTimers[k].Name), L"DpcTimer");
                    SystemTimers[k].TimerObject = p_Dpc;
                    SystemTimers[k].Period = p_Timer->Period;
					SystemTimers[k].Type = p_Timer->TimerType;
                    k++;
                    DbgPrint("定时器对象：0x%p | 触发周期: %d \n ", p_Timer, p_Timer->Period);
                    if (!MmIsAddressValid((PVOID64)p_Dpc)) continue;
                    SystemTimers[k].Func = (PVOID)(p_Dpc->DeferredRoutine);
                    DbgPrint("函数入口：0x%p", p_Dpc->DeferredRoutine);
                }
            }
        }
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrint("error status:%X", GetExceptionCode());
		}
    }
}