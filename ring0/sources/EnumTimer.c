#include "EnumTimer.h"
#include "Module.h"
#include "ObjectInfo.h"
#include "OtherFunctions.h"
#include "Symbol.h"

NTKERNELAPI PVOID KeQueryPrcbAddress(int Index);

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

ULONG EnumIoTimers(PSYSTEM_TIMER SystemTimers, ULONG MaxCount)
{
    PLIST_ENTRY IopTimerQueueHead = (PLIST_ENTRY)FindIopTimerQueueHead();
    // 枚举列表
    KIRQL OldIrql;
    ULONG i = 0;

    if (!(IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))) return 0;

    // 获得特权级
    OldIrql = KeRaiseIrqlToDpcLevel();

    __try
    {
        PLIST_ENTRY NextEntry = IopTimerQueueHead->Flink;
        while (MmIsAddressValid(NextEntry) && NextEntry != (PLIST_ENTRY)IopTimerQueueHead && i < MaxCount)
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
    return i;
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

static __forceinline PKDPC DecodeTimerDpc(
    PKTIMER Timer,
    ULONG_PTR Never,
    ULONG_PTR Always
)
{
    ULONG_PTR Dpc = (ULONG_PTR)Timer->Dpc;
    ULONG Shift = (ULONG)(Never & 0xFF);

    Dpc ^= Never;
    Dpc = _rotl64(Dpc, Shift);
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

    ULONG64 addr1 = 0;
    NTSTATUS status = GetNtSymbolAddress(L"KiWaitNever", &addr1);
    if (status == STATUS_SUCCESS && addr1)
    {
        DbgPrint("Symbol: KiWaitNever=%p\n", (PVOID)addr1);
        *KiWaitNever = (PVOID)addr1;

        status = GetNtSymbolAddress(L"KiWaitAlways", &addr1);
        if (status == STATUS_SUCCESS && addr1)
        {
            DbgPrint("Symbol: KiWaitAlways=%p\n", (PVOID)addr1);
            *KiWaitAlways = (PVOID)addr1;
            return TRUE;
        }

    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    // -------------------------------------------

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

static __forceinline VOID AcquireTimerTableEntryLock(volatile LONG64* Lock)
{
    ULONG SpinCount = 0;

    while (_interlockedbittestandset64(Lock, 0))
    {
        do
        {
            _mm_pause();

            if (++SpinCount == 0)
                SpinCount = 1;
        } while (*Lock != 0);
    }
}

static __forceinline VOID ReleaseTimerTableEntryLock(volatile LONG64* Lock)
{
    _InterlockedAnd64(Lock, 0);
}

static __forceinline BOOLEAN IsKernelPointer(PVOID Address)
{
#ifdef _WIN64
    return (ULONG_PTR)Address >= 0xFFFF800000000000ULL;
#else
    return Address != NULL;
#endif
}

ULONG EnumDpcTimers(PSYSTEM_TIMER SystemTimers, ULONG MaxCount)
{
    PVOID KiWaitNever = NULL, KiWaitAlways = NULL;

    __try
    {
        PVOID KiSetTimerEx = FindKiSetTimerEx();

        if (!FindKiWaitXXX(KiSetTimerEx, &KiWaitNever, &KiWaitAlways))
        {
            DbgPrint("Find KiWait failed\n");
            return 0;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("Find KiWait exception:%X\n", GetExceptionCode());
        return 0;
    }

    RTL_OSVERSIONINFOEXW OSVersion = { 0 };
    OSVersion.dwOSVersionInfoSize = sizeof(OSVersion);
    RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

    ULONG TimerTableOffset = 0;

    if (OSVersion.dwMajorVersion == 10)
    {
        if (OSVersion.dwBuildNumber >= 22000)
            TimerTableOffset = 0x4100;
        else
            TimerTableOffset = 0x3C00;
    }
    else if (OSVersion.dwMajorVersion == 6 &&
        OSVersion.dwMinorVersion == 1)
    {
        TimerTableOffset = 0x2200;
    }
    else
    {
        DbgPrint("Unsupported OS version\n");
        return 0;
    }

    ULONG CpuCount = KeNumberProcessors;
    ULONG k = 0;

    for (ULONG Cpu = 0; Cpu < CpuCount; Cpu++)
    {
        PVOID Prcb = KeQueryPrcbAddress(Cpu);
        if (!Prcb)
            continue;

        PKTIMER_TABLE TimerTable =
            (PKTIMER_TABLE)((PUCHAR)Prcb + TimerTableOffset);

        for (ULONG Table = 0; Table < 2; Table++)
        {
            for (ULONG Bucket = 0; Bucket < 256; Bucket++)
            {
                PKTIMER_TABLE_ENTRY TimerEntry =
                    &TimerTable->TimerEntries[Table][Bucket];

                KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

                AcquireTimerTableEntryLock(&TimerEntry->Lock);

                PLIST_ENTRY Head = &TimerEntry->Entry;
                PLIST_ENTRY Entry = NULL;

                __try
                {
                    Entry = Head->Flink;
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    DbgPrint(
                        "BAD TIMER HEAD: CPU=%lu Table=%lu Bucket=%lu Head=%p Code=%08X\n",
                        Cpu,
                        Table,
                        Bucket,
                        Head,
                        GetExceptionCode());

                    ReleaseTimerTableEntryLock(&TimerEntry->Lock);
                    KeLowerIrql(OldIrql);
                    continue;
                }

                ULONG WalkCount = 0;

                while (Entry != Head)
                {
                    // 先验证 Entry 本身是否有效
                    if (!IsKernelPointer(Entry))
                    {
                        DbgPrint("BAD ENTRY POINTER: CPU=%lu Table=%lu Bucket=%lu Entry=%p\n",
                            Cpu, Table, Bucket, Entry);
                        break;
                    }

                    if (++WalkCount > 0x10000)
                    {
                        DbgPrint(
                            "TIMER LIST LOOP: CPU=%lu Table=%lu Bucket=%lu Head=%p Entry=%p\n",
                            Cpu,
                            Table,
                            Bucket,
                            Head,
                            Entry);

                        break;
                    }

                    if (!IsKernelPointer(Entry))
                    {
                        DbgPrint(
                            "BAD TIMER ENTRY: CPU=%lu Table=%lu Bucket=%lu "
                            "Head=%p Entry=%p Lock=%llX\n",
                            Cpu,
                            Table,
                            Bucket,
                            Head,
                            Entry,
                            TimerEntry->Lock);

                        break;
                    }

                    PKTIMER Timer =
                        CONTAINING_RECORD(
                            Entry,
                            KTIMER,
                            TimerListEntry);

                    /*
                     * TimerListEntry 属于 KTIMER。
                     * 当前 Timer 必须与正在枚举的 table / bucket 对应。
                     */
                    if (Timer->TimerType != Table ||
                        Timer->Header.Size != Bucket)
                    {
                        DbgPrint(
                            "BAD TIMER BUCKET: CPU=%lu Table=%lu Bucket=%lu "
                            "Timer=%p Type=%u Size=%u Entry=%p\n",
                            Cpu,
                            Table,
                            Bucket,
                            Timer,
                            Timer->TimerType,
                            Timer->Header.Size,
                            Entry);

                        break;
                    }

                    PLIST_ENTRY Next = NULL;

                    /*
                     * 先读取 Next，再验证。
                     * 不要最后直接 Entry = Entry->Flink。
                     */
                    __try
                    {
                        Next = Entry->Flink;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        DbgPrint(
                            "TIMER FLINK EXCEPTION: CPU=%lu Table=%lu Bucket=%lu "
                            "Entry=%p Code=%08X\n",
                            Cpu,
                            Table,
                            Bucket,
                            Entry,
                            GetExceptionCode());

                        break;
                    }

                    if (Next == NULL)
                    {
                        DbgPrint(
                            "NULL TIMER FLINK: CPU=%lu Table=%lu Bucket=%lu "
                            "Entry=%p\n",
                            Cpu,
                            Table,
                            Bucket,
                            Entry);

                        break;
                    }

                    if (Next != Head)
                    {
                        if (!IsKernelPointer(Next))
                        {
                            DbgPrint(
                                "BAD TIMER FLINK: CPU=%lu Table=%lu Bucket=%lu "
                                "Head=%p Entry=%p Next=%p\n",
                                Cpu,
                                Table,
                                Bucket,
                                Head,
                                Entry,
                                Next);

                            break;
                        }

                        PLIST_ENTRY Back = NULL;

                        __try
                        {
                            Back = Next->Blink;
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER)
                        {
                            DbgPrint(
                                "TIMER BLINK EXCEPTION: CPU=%lu Table=%lu Bucket=%lu "
                                "Entry=%p Next=%p Code=%08X\n",
                                Cpu,
                                Table,
                                Bucket,
                                Entry,
                                Next,
                                GetExceptionCode());

                            break;
                        }

                        if (Back != Entry)
                        {
                            DbgPrint(
                                "CORRUPT TIMER LIST: CPU=%lu Table=%lu Bucket=%lu "
                                "Entry=%p Next=%p NextBlink=%p\n",
                                Cpu,
                                Table,
                                Bucket,
                                Entry,
                                Next,
                                Back);

                            break;
                        }
                    }
                    else
                    {
                        /*
                         * Next 已经回到链表头。
                         * 顺手验证 Head->Blink 是否确实指向当前项。
                         */
                        PLIST_ENTRY HeadBlink = NULL;

                        __try
                        {
                            HeadBlink = Head->Blink;
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER)
                        {
                            DbgPrint(
                                "HEAD BLINK EXCEPTION: CPU=%lu Table=%lu Bucket=%lu "
                                "Head=%p Code=%08X\n",
                                Cpu,
                                Table,
                                Bucket,
                                Head,
                                GetExceptionCode());

                            break;
                        }

                        if (HeadBlink != Entry)
                        {
                            DbgPrint(
                                "CORRUPT TIMER TAIL: CPU=%lu Table=%lu Bucket=%lu "
                                "Head=%p Entry=%p HeadBlink=%p\n",
                                Cpu,
                                Table,
                                Bucket,
                                Head,
                                Entry,
                                HeadBlink);

                            break;
                        }
                    }

                    /*
                     * 只有链表结构已经验证后，才读取 Timer / DPC 内容。
                     *
                     * 注意这些输出操作不应该影响第一遍
                     * EnumDpcTimers(NULL, 0) 的完整性验证。
                     */
                    if (SystemTimers != NULL && k < MaxCount)
                    {
                        PKDPC Dpc = NULL;

                        __try
                        {
                            Dpc = DecodeTimerDpc(
                                Timer,
                                (ULONG_PTR)KiWaitNever,
                                (ULONG_PTR)KiWaitAlways);

                            RtlStringCbCopyW(
                                SystemTimers[k].Name,
                                sizeof(SystemTimers[k].Name),
                                L"DpcTimer");

                            SystemTimers[k].TimerObject = Timer;
                            SystemTimers[k].pDpc = Dpc;
                            SystemTimers[k].Period = Timer->Period;
                            SystemTimers[k].Type = Timer->TimerType;

                            if (Dpc && IsKernelPointer(Dpc))
                                SystemTimers[k].Func = (PVOID)Dpc->DeferredRoutine;
                            else
                                SystemTimers[k].Func = NULL;
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER)
                        {
                            SystemTimers[k].pDpc = NULL;
                            SystemTimers[k].Func = NULL;

                            DbgPrint(
                                "TIMER DATA EXCEPTION: CPU=%lu Table=%lu Bucket=%lu "
                                "Timer=%p Dpc=%p Code=%08X\n",
                                Cpu,
                                Table,
                                Bucket,
                                Timer,
                                Dpc,
                                GetExceptionCode());
                        }
                    }

                    k++;
                    Entry = Next;
                }

                ReleaseTimerTableEntryLock(&TimerEntry->Lock);

                KeLowerIrql(OldIrql);
            }
        }
    }

    return k;
}