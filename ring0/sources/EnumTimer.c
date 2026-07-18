#include "EnumTimer.h"
#include "Module.h"
#include "ObjectInfo.h"
#include "OtherFunctions.h"
#include "Symbol.h"

PVOID FindIopTimerQueueHead()
{
    ULONG64 addr = 0;
    NTSTATUS status = GetNtSymbolAddress(L"IopTimerQueueHead", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: IopTimerQueueHead=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    // -------------------------------------------
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

	// 打印从IoInitializeTimerAddr
    //for (ULONG i = 0x26; i <= 0xa5; i++) DbgPrint("Byte %02x: %02x", i, *((PUCHAR)IoInitializeTimerAddr + i));

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

BOOLEAN EnumIoTimers(PSYSTEM_TIMER SystemTimers)
{
    PLIST_ENTRY IopTimerQueueHead = (PLIST_ENTRY)FindIopTimerQueueHead();
    // 枚举列表
    KIRQL OldIrql;
    ULONG i = 0;

    if (!(IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))) return FALSE;

    // 获得特权级
    OldIrql = KeRaiseIrqlToDpcLevel();

    __try
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
                DbgPrint("IoTimer, Timer=: 0x%p, Func=0x%p, Flag=%d, Type=%d, Context=0x%p\n",
                    Timer, Timer->TimerRoutine, Timer->TimerFlag, Timer->Type, Timer->Context);
                i++;
            }
            NextEntry = NextEntry->Flink;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("error status:%X", GetExceptionCode());
    }

    // 恢复特权级
    KeLowerIrql(OldIrql);
    return TRUE;
}

PVOID FindKiSetTimerEx()
{
    UNICODE_STRING name = RTL_CONSTANT_STRING(L"KeSetTimerEx");
    PUCHAR pKeSetTimerEx = (PUCHAR)MmGetSystemRoutineAddress(&name);

    if (pKeSetTimerEx == NULL)
    {
        DbgPrint("KeSetTimerEx is NULL\n");
        return NULL;
    }

    //
    // 计算函数长度（直到 RET）
    //
    ULONG FuncSize = 0;

    while (FuncSize < 0x400)
    {
        UCHAR op = pKeSetTimerEx[FuncSize];

        if (op == 0xC3)                 // ret
        {
            FuncSize++;
            break;
        }

        if (op == 0xC2)                 // ret xx
        {
            FuncSize += 3;
            break;
        }

        FuncSize++;
    }

    DbgPrint("KeSetTimerEx Size = 0x%X\n", FuncSize);

    //
    // Win10：函数很长，直接就是完整实现
    //
    if (FuncSize > 0x80)
    {
        DbgPrint("KeSetTimerEx is full implementation.\n");
        return pKeSetTimerEx;
    }

    //
    // Win11：wrapper，寻找 call rel32
    //
    for (ULONG i = 0; i + 5 <= FuncSize; i++)
    {
        if (pKeSetTimerEx[i] != 0xE8)
            continue;

        LONG Rel = *(PLONG)(pKeSetTimerEx + i + 1);

        PUCHAR Target = pKeSetTimerEx + i + 5 + Rel;

        DbgPrint("Found CALL at +0x%X -> %p\n", i, Target);

        return Target;
    }

    DbgPrint("Wrapper detected but CALL not found.\n");

    return NULL;
}

PKDPC DecodeTimerDpc(
    PKTIMER Timer,
    PVOID KiWaitNever,
    PVOID KiWaitAlways
)
{
    if (!Timer || !KiWaitNever || !KiWaitAlways) return NULL;

    ULONG_PTR Dpc = (ULONG_PTR)Timer->Dpc;

    ULONG_PTR Never =
        *(volatile ULONG_PTR*)KiWaitNever;
    ULONG_PTR Always =
        *(volatile ULONG_PTR*)KiWaitAlways;

    ULONG Shift = (ULONG)(Never & 0xFF);

    //
    // Win10 / Win11 23H2
    //
    Dpc ^= Never;
    Dpc = _rotl64(
        Dpc,
        Shift
    );
    Dpc ^= (ULONG_PTR)Timer;
    Dpc = _byteswap_uint64(Dpc);
    Dpc ^= Always;

    return (PKDPC)Dpc;
}

BOOLEAN FindKiWaitXXX(
    PVOID KiSetTimerEx,
    PVOID* KiWaitNever,
    PVOID* KiWaitAlways
)
{
    if (!KiSetTimerEx || !KiWaitNever || !KiWaitAlways)
        return FALSE;

    PUCHAR code = (PUCHAR)KiSetTimerEx;

    DbgPrint(
        "FindKiWaitXXX start: %p\n",
        KiSetTimerEx
    );

    PVOID Never = NULL;
    PVOID Always = NULL;
    ULONG movCount = 0;

    for (ULONG i = 0; i < 0x120; i++)
    {
        //
        // mov rax,[rip+xxxx]
        //
        if (code[i] == 0x48 &&
            code[i + 1] == 0x8B)
        {
            if ((code[i + 2] & 0xC7) == 0x05)
            {
                movCount++;

                if (movCount == 1)
                {
                    // __security_cookie
                    continue;
                }
                LONG offset =
                    *(PLONG)(code + i + 3);

                PVOID addr =
                    code + i + 7 + offset;

                DbgPrint(
                    "RIP MOV found @+0x%x ModRM=%02X -> %p\n",
                    i,
                    code[i + 2],
                    addr
                );

                if (movCount == 2)
                {
                    Never = addr;
                }
                else if (movCount == 3)
                {
                    Always = addr;
                }
            }
        }

        //
        // xor reg,[rip+xxxx]
        //
        if (code[i] == 0x48 &&
            code[i + 1] == 0x33)
        {

            //
            // 2D:
            // xor rbp,[rip+xxxx]
            //
            // 35:
            // xor rsi,[rip+xxxx]
            //
            if (code[i + 2] >= 0x05 &&
                code[i + 2] <= 0x3D)
            {

                LONG offset =
                    *(PLONG)(code + i + 3);


                PVOID addr =
                    code + i + 7 + offset;


                DbgPrint(
                    "RIP XOR found @+0x%x -> %p\n",
                    i,
                    addr
                );


                if (!Always)
                {
                    Always = addr;
                }
            }
        }
    }

    //
    // Win10:
    // mov KiWaitNever
    // mov KiWaitAlways
    //
    // Win11:
    // mov KiWaitNever
    // xor KiWaitAlways
    //
    if (Never && Always)
    {
        *KiWaitNever = Never;
        *KiWaitAlways = Always;


        DbgPrint(
            "KiWaitNever=%p\n",
            Never
        );

        DbgPrint(
            "KiWaitAlways=%p\n",
            Always
        );

        return TRUE;
    }


    DbgPrint(
        "Find failed Never=%p Always=%p\n",
        Never,
        Always
    );


    return FALSE;
}

void EnumDpcTimers(PSYSTEM_TIMER SystemTimers)
{
    PVOID KiWaitNever = NULL, KiWaitAlways = NULL;
    __try
    {
        PVOID KiSetTimerEx;

        KiSetTimerEx = FindKiSetTimerEx();
        //if (!KiSetTimerEx) KiSetTimerEx = (PVOID)KeSetTimerEx;

        if (!FindKiWaitXXX(
            KiSetTimerEx,
            &KiWaitNever,
            &KiWaitAlways))
        {
            DbgPrint("Find KiWait failed\n");
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("error1 status:%X", GetExceptionCode());
    }

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
		//DbgPrint("PRCB地址: 0x%llx \n", p_PRCB);

        // 取消绑定 CPU
        KeRevertToUserAffinityThread();

        // 判断操作系统版本
        RTL_OSVERSIONINFOEXW OSVersion = { 0 };
        OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
        RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

        BOOLEAN IsWin11 =
            (OSVersion.dwMajorVersion == 10 &&
                OSVersion.dwBuildNumber >= 22000);

        // 计算 TimerTable 在 _KPRCB 结构中的偏移
        PKTIMER_TABLE p_TimeTable = NULL;
        if (OSVersion.dwMajorVersion == 10)
        {
            if (IsWin11)
            {
                DbgPrint("Windows11\n");
                p_TimeTable =
                    (PKTIMER_TABLE)(*(PULONG64)p_PRCB + 0x4100);
            }
            else
            {
                DbgPrint("Windows10\n");
                p_TimeTable =
                    (PKTIMER_TABLE)(*(PULONG64)p_PRCB + 0x3c00);
            }
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
        }

        // 遍历 TimerEntries[] 数组（大小 256）
        for (int j = 0; j < 256; j++)
        {
            // 获取 Entry 双向链表地址
            if (!MmIsAddressValid((PVOID64)p_TimeTable)) continue;

            PLIST_ENTRY p_ListEntryHead = NULL;
            _try{
                p_ListEntryHead = &(p_TimeTable->TimerEntries[j].Entry);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("error2 status:%X", GetExceptionCode());
            }
            // 遍历 Entry 双向链表
            PLIST_ENTRY p_ListEntry = p_ListEntryHead->Flink;
            while (p_ListEntry && MmIsAddressValid(p_ListEntry) &&
                p_ListEntry != p_ListEntryHead)
            {
                // 根据 Entry 取 _KTIMER 对象地址
                PKTIMER p_Timer = CONTAINING_RECORD(p_ListEntry, KTIMER, TimerListEntry);
                if (k >= 512)
                {
                    DbgPrint("Timer buffer full\n");
                    return;
                }
                PKDPC p_Dpc = DecodeTimerDpc(p_Timer, KiWaitNever, KiWaitAlways);
                RtlStringCbCopyW(SystemTimers[k].Name, sizeof(SystemTimers[k].Name), L"DpcTimer");
                SystemTimers[k].TimerObject = p_Timer;
                SystemTimers[k].pDpc = p_Dpc;
                SystemTimers[k].Period = p_Timer->Period;
				SystemTimers[k].Type = p_Timer->TimerType;
                DbgPrint("定时器对象：0x%p | 触发周期: %d \n ", p_Timer, p_Timer->Period);
                if (p_Dpc && MmIsAddressValid((PVOID64)p_Dpc)) {
                    SystemTimers[k].Func = (PVOID)(p_Dpc->DeferredRoutine);
					DbgPrint("DPC对象：0x%p | 函数入口: 0x%p \n", p_Dpc, p_Dpc->DeferredRoutine);
                }
                k++;

                if (!MmIsAddressValid(p_ListEntry))
                    break;

                p_ListEntry = p_ListEntry->Flink;
            }
        }
    }
}