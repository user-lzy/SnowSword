#include "Window.h"
#include "OtherFunctions.h"
#include "Module.h"
#include "ntstrsafe.h"
#include "Symbol.h"
#include "OffsetScanner.h"

// SYSTEM_THREAD_INFORMATION 结构体定义（适用于大部分Windows版本）
typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;          // 包含 UniqueProcess (PID) 和 UniqueThread (TID)
    LONG Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

// SYSTEM_PROCESS_INFORMATION 结构体定义
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    LONG BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR PageDirectoryBase;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
    SYSTEM_THREAD_INFORMATION Threads[1];   // 变长数组
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

NTKERNELAPI PVOID PsGetProcessWin32Process(_In_ PEPROCESS Process);
NTKERNELAPI PVOID PsGetThreadWin32Thread(PETHREAD Thread);
NTKERNELAPI PVOID PsGetProcessPeb(PEPROCESS Process);
NTKERNELAPI PVOID PsGetCurrentThreadWin32Thread(VOID);

TIMER_ENUM_CONTEXT g_TimerCtx = { 0 };
//ULONG g_EprocessOffset = 0x1A0;  // 默认值，优先尝试

// 初始化（在DriverEntry中调用）
NTSTATUS InitializeTimerContext() {
    if (g_TimerCtx.EnterCrit) return STATUS_SUCCESS; // 已初始化

    // 获取通用函数（两个版本都存在）
    g_TimerCtx.EnterCrit = (PFN_EnterCrit)KernelGetProcAddress("win32kbase.sys", "EnterCrit");
    g_TimerCtx.UserSessionSwitchLeaveCrit = (PFN_UserSessionSwitchLeaveCrit)KernelGetProcAddress("win32kbase.sys", "UserSessionSwitchLeaveCrit");
    g_TimerCtx.ValidateHwnd = (PFN_ValidateHwnd)KernelGetProcAddress("win32kbase.sys", "ValidateHwnd");
    //g_TimerCtx.PsGetCurrentProcessWin32Process = KernelGetProcAddress("ntoskrnl.exe", "PsGetCurrentProcessWin32Process");
    
    if (!g_TimerCtx.EnterCrit || !g_TimerCtx.UserSessionSwitchLeaveCrit || !g_TimerCtx.ValidateHwnd) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    // **优先检测 Win11 特征**
    g_TimerCtx.W32GetUserSessionState = (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
    if (g_TimerCtx.W32GetUserSessionState) {
        g_TimerCtx.isV2 = TRUE;
        DbgPrint("[Timer] Win11 V2 mode initialized\n");
        return STATUS_SUCCESS;
    }

    // **降级到 Win10**
    g_TimerCtx.gTimerHashTable = (PVOID)KernelGetProcAddress("win32kbase.sys", "gTimerHashTable");
    if (g_TimerCtx.gTimerHashTable) {
        g_TimerCtx.isV2 = FALSE;
        DbgPrint("[Timer] Win10 V1 mode initialized\n");
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS EnumProcessTimers(
    _Out_ PWINDOW_TIMER* pArray,
    _Out_ PULONG pCount
) {
    NTSTATUS status = STATUS_SUCCESS;
    PVOID hashBase = NULL;
    ULONG timerCount = 0;
    PWINDOW_TIMER pResultArray = NULL;

    if (!pArray || !pCount) {
        DbgPrint("[TIMER_ENUM] 输入参数无效！\n");
        return STATUS_INVALID_PARAMETER;
    }
    *pArray = NULL;
    *pCount = 0;

    DbgPrint("[TIMER_ENUM] === 开始枚举进程定时器 ===");

    status = InitializeTimerContext();
    if (!NT_SUCCESS(status)) {
        DbgPrint("[TIMER_ENUM] 初始化上下文失败！0x%X\n", status);
        return status;
    }

    __try {
        g_TimerCtx.EnterCrit(0, 0);
        DbgPrint("[TIMER_ENUM] 已进入临界区\n");

        __try {
            // 哈希表基址（和旧版完全一致，不修改）
            if (g_TimerCtx.isV2) {
                hashBase = (PVOID)((PUCHAR)g_TimerCtx.W32GetUserSessionState() + 16 * 3850);
            }
            else {
                hashBase = g_TimerCtx.gTimerHashTable;
            }

            if (!hashBase || !MmIsAddressValid(hashBase)) {
                status = STATUS_INVALID_ADDRESS;
                goto EXIT_LABEL;
            }

            DbgPrint("[TIMER_ENUM] 哈希表基址: %p\n", hashBase);
            DbgPrint("[TIMER_ENUM] 开始遍历 64 个哈希桶...\n");

            // ==============================================
            // 【重要】你的 Win10 遍历哈希桶逻辑 → 完全不动！
            // ==============================================
            for (UCHAR bucket = 0; bucket < 64; bucket++) {
                PLIST_ENTRY* bucketPtr = (PLIST_ENTRY*)((PUCHAR)hashBase + 16 * bucket);
                if (!MmIsAddressValid(bucketPtr)) continue;
                PLIST_ENTRY head = *bucketPtr;
                if (!head || !MmIsAddressValid(head)) continue;

                // 遍历链表（原有逻辑完全保留）
                for (PLIST_ENTRY entry = head; entry != (PLIST_ENTRY)bucketPtr; entry = entry->Flink) {
                    if (!MmIsAddressValid(entry) || !MmIsAddressValid(entry->Flink)) break;

                    // ====================== 核心修复：统一联合体指针 ======================
                    TIMER_ENTRY pTimer = { 0 };
                    if (g_TimerCtx.isV2) {
                        // Win11：使用修正后的结构体
                        pTimer.Win11 = CONTAINING_RECORD(entry, TIMER_ENTRY_WIN11, HashListEntry);
                        if (!MmIsAddressValid(pTimer.Win11)) continue;
                        // 跳过已删除的定时器
                        if ((pTimer.Win11->flags & 0x1000)) continue;
                    }
                    else {
                        // Win10：完全保留你的原有代码，不修改任何逻辑
                        pTimer.Win10 = CONTAINING_RECORD(entry, TIMER_ENTRY_WIN10, HashListEntry);
                        if (!MmIsAddressValid(pTimer.Win10)) continue;
                        if ((pTimer.Win10->flags & 0x1000)) continue;
                    }

                    // ====================== PID/TID 解析（你的原有逻辑完全保留）======================
                    PVOID pti = NULL;
                    if (g_TimerCtx.isV2) {
                        pti = pTimer.Win11->pti;
                    }
                    else {
                        pti = pTimer.Win10->head.threadInfo;
                    }
                    if (!pti) continue;

                    // Win10/11 偏移（你的原有逻辑）
                    /*ULONG processOffset = g_TimerCtx.isV2 ? 0x1D0 : 0x1A8;
                    if (!MmIsAddressValid((PUCHAR)pti + processOffset)) continue;
                    PVOID ProcessInfo = *(PVOID*)((PUCHAR)pti + processOffset);
                    if (!ProcessInfo || !MmIsAddressValid(ProcessInfo)) continue;
                    PEPROCESS pEprocess = *(PEPROCESS*)ProcessInfo;
                    if (!pEprocess) continue;
                    HANDLE currentPid = PsGetProcessId(pEprocess);*/

                    // 线程ID（你的原有逻辑）
                    PETHREAD pEthread = *(PETHREAD*)((PUCHAR)pti + 0x0);
                    if (!pEthread) continue;
					HANDLE dwProcessId = PsGetThreadProcessId(pEthread);
                    HANDLE dwThreadId = PsGetThreadId(pEthread);

                    // ====================== 内存分配（你的原有逻辑完全保留）======================
                    PWINDOW_TIMER pNewArray = (PWINDOW_TIMER)ExAllocatePool2(
                        POOL_FLAG_NON_PAGED,
                        sizeof(WINDOW_TIMER) * (timerCount + 1),
                        'meT'
                    );
                    if (!pNewArray) {
                        status = STATUS_NO_MEMORY;
                        goto EXIT_LABEL;
                    }

                    if (pResultArray) {
                        memcpy(pNewArray, pResultArray, sizeof(WINDOW_TIMER) * timerCount);
                        ExFreePoolWithTag(pResultArray, 'meT');
                    }

                    // ====================== 填充数据（修复致命hWnd笔误）======================
                    pNewArray[timerCount].pfn = g_TimerCtx.isV2 ? pTimer.Win11->pfn : pTimer.Win10->pfn;
                    pNewArray[timerCount].nTimeout = g_TimerCtx.isV2 ? pTimer.Win11->nTimeout : pTimer.Win10->nTimeout;
                    pNewArray[timerCount].nIDEvent = g_TimerCtx.isV2 ? pTimer.Win11->nIDEvent : pTimer.Win10->nIDEvent;
                    pNewArray[timerCount].ThreadId = dwThreadId;
                    pNewArray[timerCount].ProcessId = dwProcessId;
                    
                    PVOID pWnd = NULL;
                    pNewArray[timerCount].hWnd = NULL;
                    if (g_TimerCtx.isV2) {
                        pWnd = pTimer.Win11->pWnd;
                        if (pWnd && MmIsAddressValid(pWnd))
                            pNewArray[timerCount].hWnd = *(PVOID*)pWnd;
                    }
                    else {
                        pWnd = pTimer.Win10->windowPtr;
                        if (pWnd && MmIsAddressValid(pWnd))
                            pNewArray[timerCount].hWnd = *(PVOID*)pWnd;
                    }

                    // 打印日志（保留）
                    DbgPrint("[TIMER_ENUM] 找到定时器！pTimer=0x%p, PID=%llu, TID=%llu, nIDEvent=%lld, pWnd=0x%p, hWnd=0x%p\n",
                        pTimer, (ULONG64)dwProcessId, (ULONG64)dwThreadId,
                        g_TimerCtx.isV2 ? pTimer.Win11->nIDEvent : pTimer.Win10->nIDEvent, 
                        pWnd, pNewArray[timerCount].hWnd);


                    pResultArray = pNewArray;
                    timerCount++;
                    DbgPrint("[TIMER_ENUM] 匹配成功！总数：%lu\n", timerCount);
                }
            }

            *pArray = pResultArray;
            *pCount = timerCount;
            DbgPrint("[TIMER_ENUM] 遍历完成，总匹配数量: %lu\n", timerCount);

        EXIT_LABEL:;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            DbgPrint("[TIMER_ENUM] 异常！代码: 0x%X\n", status);
        }
    }
    __finally {
        g_TimerCtx.UserSessionSwitchLeaveCrit();
        DbgPrint("[TIMER_ENUM] 已退出临界区\n");

        if (!NT_SUCCESS(status) && pResultArray) {
            ExFreePoolWithTag(pResultArray, 'meT');
            *pArray = NULL;
            *pCount = 0;
        }
    }

    return status;
}

PVOID FindGetHmodTableIndex() {
    ULONG64 addr = 0;
    NTSTATUS status = KernelQuerySymbolAddress(L"win32kfull.sys", L"GetHmodTableIndex", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: GetHmodTableIndex=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    return NULL;
 //   // ====================== 修改点1：替换基址函数 ======================
 //   // 原错误：NtUserUnhookWindowsHookEx
 //   // 新正确：NtUserSetWindowsHookEx（你的汇编明确使用这个函数）
 //   PVOID NtUserSetWindowsHookExAddr = KernelGetProcAddress("win32kfull.sys", "NtUserSetWindowsHookEx");
 //   if (!NtUserSetWindowsHookExAddr) {
 //       DbgPrint("NtUserSetWindowsHookEx 未找到！\n");
 //       return NULL;
 //   }

 //   // ====================== 修改点2：特征码1 → 匹配 call zzzSetWindowsHookEx ======================
 //   // 汇编：call zzzSetWindowsHookEx + 后续 test rax, rax
 //   // 机器码：E8 xx xx xx xx 48 85 C0
 //   UCHAR pattern1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0 };
 //   UCHAR mask1[] = { 1,    0,    0,    0,    0,    1,    1,    1 };
 //   PVOID result = SearchSpecialCodeWithMask(NtUserSetWindowsHookExAddr, 0x400, pattern1, mask1, sizeof(pattern1));
 //   if (!result) {
 //       DbgPrint("特征码1(call zzzSetWindowsHookEx) 未找到！\n");
 //       return NULL;
 //   }

 //   // 解析call指令 → 获取 zzzSetWindowsHookEx 函数地址
 //   ULONG call_offset = *(ULONG*)((PUCHAR)result + 1);
 //   PVOID zzzSetWindowsHookExAddr = (PVOID)((PUCHAR)result + 5 + call_offset);
 //   DbgPrint("zzzSetWindowsHookEx: 0x%p\n", zzzSetWindowsHookExAddr);

 //   // ====================== 保留：特征码2 → 匹配 call GetHmodTableIndex ======================
 //   // 你的汇编：call GetHmodTableIndex + mov [rdi+44h], eax
 //   // 机器码：E8 xx xx xx xx 89 47 44
 //   UCHAR pattern2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x89, 0x47, 0x44 };
 //   UCHAR mask2[] = { 1,    0,    0,    0,    0,    1,    1,    1 };
	//// 之前是0x2000，现在改回0x200，足够找到目标且更快
 //   result = SearchSpecialCodeWithMask(zzzSetWindowsHookExAddr, 0x200, pattern2, mask2, sizeof(pattern2));
 //   if (!result) {
 //       DbgPrint("特征码2(call GetHmodTableIndex) 未找到！\n");
 //       return NULL;
 //   }

 //   // 解析call指令 → 获取 GetHmodTableIndex 函数地址
 //   call_offset = *(ULONG*)((PUCHAR)result + 1);
 //   PVOID GetHmodTableIndexAddr = (PVOID)((PUCHAR)result + 5 + call_offset);
 //   DbgPrint("GetHmodTableIndex: 0x%p\n", GetHmodTableIndexAddr);
 //   return GetHmodTableIndexAddr;
}

PVOID FindaatomSysLoaded() {
    ULONG64 addr = 0;
    NTSTATUS status = KernelQuerySymbolAddress(L"win32kfull.sys", L"aatomSysLoaded", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: aatomSysLoaded=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    return NULL;
    // -------------------------------------------
    // ====================== 保留：特征码3 → 匹配 lea r8, aatomSysLoaded ======================
    // 你的汇编：lea r8, ?aatomSysLoaded@@3PAGA + test eax,eax
    //UCHAR pattern3[] = {
    //    0x4C, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00,  // lea r8, [rip+offset]
    //    0x85, 0xC0                                  // test eax, eax
    //};
    //UCHAR mask3[] = {
    //    1, 1, 1, 0, 0, 0, 0,
    //    1, 1
    //};
    //PVOID result = SearchSpecialCodeWithMask(FindGetHmodTableIndex(), 0x200, pattern3, mask3, sizeof(pattern3));
    //if (!result) {
    //    DbgPrint("特征码3(lea aatomSysLoaded) 未找到！\n");
    //    return NULL;
    //}

    //// 解析lea指令 → 最终目标：aatomSysLoaded
    //ULONG lea_offset = *(ULONG*)((PUCHAR)result + 3);
    //PVOID aatomSysLoadedAddr = (PVOID)((PUCHAR)result + 7 + lea_offset);

    //DbgPrint("成功找到 aatomSysLoaded: 0x%p\n", aatomSysLoadedAddr);
    //return aatomSysLoadedAddr;
}

ULONG FindAtomArrayOffset(
    PUCHAR FunctionBase,
    ULONG FunctionSize
)
{
    for (ULONG i = 0; i < FunctionSize - 8; i++)
    {
        PUCHAR p = FunctionBase + i;

        _try{
            //
            // cmp word ptr [rax+rbx+disp32], si
            //
            if (p[0] == 0x66 &&
                p[1] == 0x39 &&
                p[2] == 0xB4 &&
                p[3] == 0x18)
            {
                return *(ULONG*)(p + 4);
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("err1");
        }

        _try{
            //
            // mov [rax+rbx*2+disp32], si
            //
            if (p[0] == 0x66 &&
                p[1] == 0x89 &&
                p[2] == 0xB4 &&
                p[3] == 0x58)
            {
                return *(ULONG*)(p + 4);
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("err2");
        }
    }

    return 0;
}

ULONG FindUserLibmgmtAtomTableOffset(
    PUCHAR FunctionBase,
    ULONG FunctionSize
)
{
    for (ULONG i = 0; i < FunctionSize - 7; i++)
    {
        PUCHAR p = FunctionBase + i;


        //
        // mov rcx,[rax+disp32]
        //
        if (p[0] == 0x48 &&
            p[1] == 0x8B &&
            p[2] == 0x88)
        {
            ULONG Offset = *(ULONG*)(p + 3);


            //
            // 合理性检查
            //
            if (Offset > 0x8000 &&
                Offset < 0x20000)
            {
                return Offset;
            }
        }
    }

    return 0;
}

typedef struct _WIN32K_HMOD_LAYOUT
{
    BOOLEAN Initialized;

    ULONG AtomArrayOffset;
    ULONG AtomTableOffset;

    PFN_W32GetUserSessionState W32GetUserSessionState;
} WIN32K_HMOD_LAYOUT;

static WIN32K_HMOD_LAYOUT g_HmodLayout = { 0 };

NTSTATUS InitWin32kHmodLayout()
{
    if (g_HmodLayout.Initialized)
        return STATUS_SUCCESS;


    g_HmodLayout.W32GetUserSessionState =
        (PFN_W32GetUserSessionState)
        KernelGetProcAddress(
            "win32k.sys",
            "W32GetUserSessionState"
        );


    if (!g_HmodLayout.W32GetUserSessionState ||
        !MmIsAddressValid((PVOID)g_HmodLayout.W32GetUserSessionState))
    {
        DbgPrint("W32GetUserSessionState not found\n");
        return STATUS_NOT_FOUND;
    }


    PVOID HmodFunc = FindGetHmodTableIndex();

    if (!HmodFunc)
    {
        DbgPrint("GetHmodTableIndex not found\n");
        return STATUS_NOT_FOUND;
    }


    g_HmodLayout.AtomArrayOffset =
        FindAtomArrayOffset(
            (PUCHAR)HmodFunc,
            0x200
        );


    g_HmodLayout.AtomTableOffset =
        FindUserLibmgmtAtomTableOffset(
            (PUCHAR)HmodFunc,
            0x200
        );


    DbgPrint(
        "AtomArrayOffset=0x%X AtomTableOffset=0x%X\n",
        g_HmodLayout.AtomArrayOffset,
        g_HmodLayout.AtomTableOffset
    );


    if (!g_HmodLayout.AtomArrayOffset ||
        !g_HmodLayout.AtomTableOffset)
    {
        return STATUS_NOT_FOUND;
    }


    g_HmodLayout.Initialized = TRUE;

    return STATUS_SUCCESS;
}

// 从模块索引获取基址
NTSTATUS GetModuleNameFromihMod(
    BOOLEAN bWin11,
    int ihmod,
    LPWSTR NameBuffer
)
{
#define MAX_PATH 260

    static ATOM* aatomSysLoaded = NULL;
    static PVOID AtomTable = NULL;

    if (bWin11)
    {
        NTSTATUS Status = InitWin32kHmodLayout();
        if (!NT_SUCCESS(Status)) return Status;

        PVOID UserSessionState = g_HmodLayout.W32GetUserSessionState();

        if (!UserSessionState) return STATUS_INVALID_ADDRESS;

        aatomSysLoaded =
            (ATOM*)((PUCHAR)UserSessionState +
                g_HmodLayout.AtomArrayOffset);

        AtomTable =
            *(PVOID*)((PUCHAR)UserSessionState +
                g_HmodLayout.AtomTableOffset);
    }
    else
    {
        if (!aatomSysLoaded)
        {
            aatomSysLoaded = (ATOM*)FindaatomSysLoaded();
            if (!aatomSysLoaded) return STATUS_INVALID_ADDRESS;
        }

        if (!AtomTable)
        {
            AtomTable = KernelGetProcAddress(
                "win32kbase.sys", "UserLibmgmtAtomTableHandle");

            if (!AtomTable) return STATUS_INVALID_ADDRESS;

            AtomTable = *(PVOID*)AtomTable;
        }
    }

    if (!aatomSysLoaded || !AtomTable) return STATUS_INVALID_ADDRESS;

    ATOM Atom = aatomSysLoaded[ihmod];

    if (!Atom) return STATUS_NOT_FOUND;

    PFN_RtlQueryAtomInAtomTable RtlQueryAtomInAtomTable =
        (PFN_RtlQueryAtomInAtomTable)
        KernelGetProcAddress("ntoskrnl.exe","RtlQueryAtomInAtomTable");

    if (!RtlQueryAtomInAtomTable) return STATUS_NOT_FOUND;

    ULONG MaxLength = sizeof(WCHAR) * MAX_PATH;

    return RtlQueryAtomInAtomTable(AtomTable, Atom, 0,
        0, NameBuffer, &MaxLength);
}

PVOID FindHMValidateHandle() {
    ULONG64 addr = 0;
    NTSTATUS status = KernelQuerySymbolAddress(L"win32kfull.sys", L"HMValidateHandle", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: HMValidateHandle=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    // -------------------------------------------
    ULONG64 p = (ULONG64)KernelGetProcAddress("win32kfull.sys", "NtUserUnhookWindowsHookEx");
    if (!p || !MmIsAddressValid((PVOID)p)) {
        DbgPrint("NtUserUnhookWindowsHookEx not found!\n");
        return NULL;
    }
    DbgPrint("NtUserUnhookWindowsHookEx: 0x%p\n", (PVOID)p);
    //Pattern B2 05 ?? ?? ?? E8 ?? ?? ?? ??
    while ((*(PULONG64)p & 0xFF000000FFFF) != 0xE800000005B2) p++;
    PVOID HMValidateHandle = (PVOID)((p & 0xFFFFFFFF00000000) + (DWORD)(p + *(PULONG)(p + 6) + 10));
    //PHOOK Hook = OutputBuffer;
    if (!HMValidateHandle || !MmIsAddressValid((PVOID)HMValidateHandle)) {
        DbgPrint("HMValidateHandle not found!\n");
        return NULL;
    }
    DbgPrint("HMValidateHandle: 0x%p\n", HMValidateHandle);
    return HMValidateHandle;
}

// 钩子名称数组（支持-1的WH_MSGFILTER）
const char* GetHookTypeName(int iHookType)
{
    switch (iHookType)
    {
    case -1: return "WH_MSGFILTER";      // ✅ 你第一个钩子的正确类型
    case 0:  return "WH_JOURNALRECORD";
    case 1:  return "WH_JOURNALPLAYBACK";
    case 2:  return "WH_KEYBOARD";
    case 3:  return "WH_GETMESSAGE";
    case 4:  return "WH_CALLWNDPROC";
    case 5:  return "WH_CBT";
    case 6:  return "WH_SYSMSGFILTER";
    case 7:  return "WH_MOUSE";
    case 8:  return "WH_HARDWARE";
    case 9:  return "WH_DEBUG";
    case 10: return "WH_SHELL";
    case 11: return "WH_FOREGROUNDIDLE";
    case 12: return "WH_CALLWNDPROCRET";// ✅ 你第二个钩子的正确类型
    case 13: return "WH_KEYBOARD_LL";
    case 14: return "WH_MOUSE_LL";
    default: return "UNKNOWN_INTERNAL";
    }
}

// 定位 PtiCurrent 的函数（增强版）
PVOID FindPtiCurrent() {
    ULONG64 addr = 0;
    NTSTATUS status = KernelQuerySymbolAddress(L"win32kfull.sys", L"PtiCurrent", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: PtiCurrent=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    // -------------------------------------------
    // 1. 获取 NtUserShowWindowAsync 地址
    PVOID pNtUserShowWindowAsync = KernelGetProcAddress("win32kfull.sys", "NtUserShowWindowAsync");
    if (NULL == pNtUserShowWindowAsync) {
        DbgPrint("FindPtiCurrent: Failed to get NtUserShowWindowAsync\n");
        return NULL;
    }
    DbgPrint("NtUserShowWindowAsync: %p\n", pNtUserShowWindowAsync);

    // 2. 搜索特征码序列: mov edi, eax (8B F8) + call (E8)
    PUCHAR StartSearchAddress = (PUCHAR)pNtUserShowWindowAsync;
    ULONG SearchLength = 0x100;              // 扩展搜索范围至256字节
    // 特征码: 8B F8 E8
    UCHAR SpecialCode[] = { 0x8B, 0xF8, 0xE8 };
    ULONG SpecialCodeLen = sizeof(SpecialCode);

    PVOID pMatch = SearchSpecialCode(StartSearchAddress, SearchLength,
        SpecialCode, SpecialCodeLen);
    if (NULL == pMatch) {
        DbgPrint("FindPtiCurrent: No 'mov edi, eax; call' pattern found\n");
        return NULL;
    }

    // pMatch 指向 0x8B，call 指令起始地址为 pMatch + 2
    PUCHAR pCallInst = (PUCHAR)pMatch + 2;
    DbgPrint("Found call instruction at %p\n", pCallInst);

    // 3. 解析相对偏移并计算目标地址 (x64)
    LONG offset = *(LONG*)(pCallInst + 1);
    PVOID pPtiCurrent = (PVOID)(pCallInst + 5 + offset);
    DbgPrint("FindPtiCurrent: PtiCurrent found at %p\n", pPtiCurrent);

    return pPtiCurrent;
}

BOOLEAN IsValidHook(
    PVOID pHookBase,
    IN PWIN32K_OFFSETS pOffsets      // 【新增】传入偏移结构体
)
{
    if (pHookBase == NULL || !pOffsets)
        return FALSE;

    __try {
        // 【修正】使用结构体偏移：Hook_nHookType
        if (!MmIsAddressValid((PUCHAR)pHookBase + pOffsets->Hook_nHookType))
            return FALSE;
        LONG hookType = *(PLONG)((PUCHAR)pHookBase + pOffsets->Hook_nHookType);
        if (hookType < 0 || hookType >= MAX_HOOK_TYPES)
            return FALSE;

        // 【修正】使用结构体偏移：Hook_offPfn
        // 注意：Hook_offPfn 是偏移量(offPfn)，不是直接地址
        // 如果为0表示没有设置钩子过程，视为无效
        if (!MmIsAddressValid((PUCHAR)pHookBase + pOffsets->Hook_offPfn))
            return FALSE;
        ULONG64 offPfn = *(PULONG64)((PUCHAR)pHookBase + pOffsets->Hook_offPfn);
        if (offPfn == 0)
            return FALSE;

        // 【修正】使用结构体偏移：Hook_phkNext
        if (!MmIsAddressValid((PUCHAR)pHookBase + pOffsets->Hook_phkNext))
            return FALSE;

        // 【新增】可选：校验 ihmod 范围（0xFFFFFFFF 是系统钩子，其他应小于某个合理值）
        ULONG ihmod = *(PULONG)((PUCHAR)pHookBase + pOffsets->Hook_ihmod);
        if (ihmod != 0xFFFFFFFF && ihmod >= 0x1000) {
            // 可疑值，但不一定是错误，仅记录
            // return FALSE; 
        }

        return TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
}

// 完善后的内存打印辅助函数
VOID DebugPrintHookMemory(PVOID pBase, ULONG uPrintSize)
{
    if (!pBase || uPrintSize == 0)
        return;

    DbgPrint("\n===== [tagHOOK 内存快照] 基地址: 0x%p | 长度: 0x%X =====\n", pBase, uPrintSize);
    __try {
        for (ULONG i = 0; i < uPrintSize; i += 0x10)
        {
            PUCHAR pCurrent = (PUCHAR)pBase + i;
            // 打印偏移 + 16进制内容 + 简单的ASCII字符
            DbgPrint("0x%04X:  %016llX %016llX  ",
                i,
                *(PULONG64)pCurrent,
                *(PULONG64)(pCurrent + 8)
            );
            // 可选：打印可打印字符，辅助识别字符串
            for (ULONG j = 0; j < 16; j++) {
                UCHAR ch = pCurrent[j];
                DbgPrint("%c", (ch >= 0x20 && ch < 0x7F) ? ch : '.');
            }
            DbgPrint("\n");
        }
        DbgPrint("======================================================================\n");
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("[!] 内存访问异常，终止打印\n");
    }
}

// 【验证通过】从tagHOOK获取安装线程的TID/PID
NTSTATUS GetHookOwnerId(
    PVOID pTagHook,
    OUT PULONG pPid,
    OUT PULONG pTid,
    IN PWIN32K_OFFSETS pOffsets      // 【新增】传入偏移结构体
)
{
    if (!pTagHook || !pPid || !pTid || !pOffsets)
        return STATUS_INVALID_PARAMETER;

    *pPid = 0;
    *pTid = 0;

    __try {
        // 【修正】使用结构体偏移：Hook_pti
        PVOID pEthreadPtr = *(PVOID*)((PUCHAR)pTagHook + pOffsets->Hook_pti);
        if (!pEthreadPtr || !MmIsAddressValid(pEthreadPtr))
            return STATUS_INVALID_ADDRESS;

        PETHREAD pEthread = *(PETHREAD*)pEthreadPtr;
        if (!pEthread || !MmIsAddressValid(pEthread))
            return STATUS_INVALID_ADDRESS;

        *pTid = (ULONG)(ULONG_PTR)PsGetThreadId(pEthread);
        *pPid = (ULONG)(ULONG_PTR)PsGetThreadProcessId(pEthread);

        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS EnumerateHooksFromPti(
    PVOID pContext,
    BOOLEAN IsGlobal,
    HANDLE hPid,
    HANDLE hTid,
    PWIN32K_MSG_HOOK_INFO pHookArray,
    PULONG puFoundHookCount,
    ULONG uMaxCount,
    PWIN32K_OFFSETS pOffsets        // 【新增】传入偏移结构体
)
{
    if (!pContext || !pHookArray || !puFoundHookCount || uMaxCount == 0 || !pOffsets)
        return STATUS_INVALID_PARAMETER;

    if (IsGlobal) {
        // ==========================================
        // 全局钩子枚举（链表结构）
        // ==========================================

        // 【关键修正】pContext 是 pti，需要先解引用得到 pDeskInfo
        PVOID pDeskInfo = *(PVOID*)((PUCHAR)pContext + pOffsets->Pti_pDeskInfo);
        if (!pDeskInfo || !MmIsAddressValid(pDeskInfo)) {
            DbgPrint("[!] pDeskInfo 无效\n");
            return STATUS_INVALID_ADDRESS;
        }

        // 从 pDeskInfo 获取全局钩子数组基址
        PVOID* pHookHeadArray = (PVOID*)((PUCHAR)pDeskInfo + pOffsets->DeskInfo_aphkStart);
        if (!MmIsAddressValid(pHookHeadArray)) {
            DbgPrint("[!] aphkStart 数组无效\n");
            return STATUS_INVALID_ADDRESS;
        }

        for (ULONG hookTypeIdx = 0; hookTypeIdx < MAX_HOOK_TYPES; hookTypeIdx++)
        {
            __try {
                // 每个元素是 PLIST_ENTRY / tagHOOK* 链表头
                PVOID pCurrentHook = pHookHeadArray[hookTypeIdx];
                ULONG dwDepth = 0;
                const ULONG MAX_HOOK_DEPTH = 256;

                while (pCurrentHook != NULL && dwDepth < MAX_HOOK_DEPTH)
                {
                    dwDepth++;

                    if (!MmIsAddressValid(pCurrentHook)) {
                        DbgPrint("[!] 全局类型[%2d] tagHOOK无效: 0x%p\n", hookTypeIdx, pCurrentHook);
                        break;
                    }

                    if (!IsValidHook(pCurrentHook, pOffsets)) {
                        pCurrentHook = *(PVOID*)((PUCHAR)pCurrentHook + pOffsets->Hook_phkNext);
                        continue;
                    }

                    if (*puFoundHookCount >= uMaxCount)
                        break;

                    PWIN32K_MSG_HOOK_INFO pEntry = &pHookArray[*puFoundHookCount];
                    PUCHAR pHookBase = (PUCHAR)pCurrentHook;

                    __try {
                        RtlZeroMemory(pEntry, sizeof(WIN32K_MSG_HOOK_INFO));

                        // 读取钩子成员
                        pEntry->HookType = *(PLONG)(pHookBase + pOffsets->Hook_nHookType);
                        pEntry->HookProc = *(PULONG64)(pHookBase + pOffsets->Hook_offPfn);
                        pEntry->HookFlags = *(PULONG)(pHookBase + pOffsets->Hook_flags);
                        ULONG ihmod = *(PULONG)(pHookBase + pOffsets->Hook_ihmod);

                        RtlZeroMemory(pEntry->ModulePath, sizeof(pEntry->ModulePath));

                        if (ihmod != 0xFFFFFFFF && ihmod < 0x1000) {
                            GetModuleNameFromihMod(TRUE, (INT)ihmod, pEntry->ModulePath);
                        }

                        pEntry->IsGlobal = TRUE;
                        GetHookOwnerId(pCurrentHook, &pEntry->ProcessId, &pEntry->ThreadId, pOffsets);
                        pEntry->HookHandle = *(HANDLE*)(pHookBase + pOffsets->Hook_hHook);

                        (*puFoundHookCount)++;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DbgPrint("[!] 读取全局钩子成员异常\n");
                    }

                    // 取下一个钩子
                    pCurrentHook = *(PVOID*)((PUCHAR)pCurrentHook + pOffsets->Hook_phkNext);
                }

                if (dwDepth >= MAX_HOOK_DEPTH) {
                    DbgPrint("[!] 全局类型[%2d] 遍历超过最大深度\n", hookTypeIdx);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DbgPrint("[!] 全局类型[%2d] 链表访问异常\n", hookTypeIdx);
                continue;
            }
        }
    }
    else {
        // ==========================================
        // 线程钩子枚举（直接指针数组）
        // ==========================================
        // 【修正】使用结构体偏移：Pti_aphkStart
        PVOID* pHookPtrArray = (PVOID*)((PUCHAR)pContext + pOffsets->Pti_aphkStart);

        for (ULONG hookIndex = 0; hookIndex < MAX_HOOK_TYPES + 1; hookIndex++)
        {
            __try {
                PVOID pHook = pHookPtrArray[hookIndex];

                if (pHook != NULL) {
                    DbgPrint("[调试] 线程类型[%2d] 原始指针: 0x%p\n", hookIndex, pHook);
                }

                if (!pHook || !MmIsAddressValid(pHook)) {
                    continue;
                }

                if (!IsValidHook(pHook, pOffsets)) {    // 【修正】
                    continue;
                }

                if (*puFoundHookCount >= uMaxCount)
                    continue;

                PWIN32K_MSG_HOOK_INFO pEntry = &pHookArray[*puFoundHookCount];
                PUCHAR pHookBase = (PUCHAR)pHook;

                __try {
                    RtlZeroMemory(pEntry, sizeof(WIN32K_MSG_HOOK_INFO));

                    // 【修正】统一使用结构体偏移
                    pEntry->HookType = *(PLONG)(pHookBase + pOffsets->Hook_nHookType);
                    pEntry->HookProc = *(PULONG64)(pHookBase + pOffsets->Hook_offPfn);
                    pEntry->HookFlags = *(PULONG)(pHookBase + pOffsets->Hook_flags);
                    ULONG ihmod = *(PULONG)(pHookBase + pOffsets->Hook_ihmod);

                    RtlZeroMemory(pEntry->ModulePath, sizeof(pEntry->ModulePath));

                    if (ihmod != 0xFFFFFFFF && ihmod < 0x1000) {
                        GetModuleNameFromihMod(TRUE, (INT)ihmod, pEntry->ModulePath);
                    }

                    pEntry->IsGlobal = FALSE;
                    pEntry->ProcessId = (ULONG)(ULONG_PTR)hPid;
                    pEntry->ThreadId = (ULONG)(ULONG_PTR)hTid;
                    pEntry->HookHandle = *(HANDLE*)(pHookBase + pOffsets->Hook_hHook);

                    (*puFoundHookCount)++;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DbgPrint("[!] 读取线程直接钩子成员异常\n");
                }

            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DbgPrint("[!] 线程类型[%2d] 访问异常\n", hookIndex);
                continue;
            }
        }
    }

    return STATUS_SUCCESS;
}

// ==========================================
// 【修正后】主函数
// ==========================================
NTSTATUS EnumerateMsgHook_Win11(
    OUT PWIN32K_MSG_HOOK_INFO* ppHookList,
    OUT PULONG                   pulHookCount
)
{
    PVOID xxxCallHook = NULL, zzzSetWindowsHookEx = NULL;
    if (KernelQuerySymbolAddress(L"win32kfull.sys", L"xxxCallHook", (PULONG64)&xxxCallHook) != STATUS_SUCCESS || !xxxCallHook)
    {
        DbgPrint("[-] xxxCallHook not found!\n");
        return STATUS_NOT_FOUND;    // 【修正】返回错误码
    }
    if (KernelQuerySymbolAddress(L"win32kfull.sys", L"zzzSetWindowsHookEx", (PULONG64)&zzzSetWindowsHookEx) != STATUS_SUCCESS || !zzzSetWindowsHookEx)
    {
        DbgPrint("[-] zzzSetWindowsHookEx not found!\n");
        return STATUS_NOT_FOUND;
    }
    if (!MmIsAddressValid(xxxCallHook) || !MmIsAddressValid(zzzSetWindowsHookEx))
    {
        DbgPrint("[-] xxxCallHook or zzzSetWindowsHookEx is not valid!\n");
        return STATUS_INVALID_ADDRESS;
    }

    WIN32K_OFFSETS Offsets = { 0 };
    if (!NT_SUCCESS(Win32kOffsetScanner_Initialize(zzzSetWindowsHookEx, NULL, xxxCallHook, NULL, &Offsets)))
    {
        DbgPrint("[-] Win32kOffsetScanner_Initialize failed!\n");
        return STATUS_UNSUCCESSFUL;
    }
    //Win32kOffsetScanner_Dump(&Offsets);

    NTSTATUS status = STATUS_SUCCESS;
    PWIN32K_MSG_HOOK_INFO pHookArray = NULL;
    ULONG uFoundHookCount = 0;
    ULONG uAllocatedCount = 2048;
    PVOID buffer = NULL;
    ULONG bufferSize = 0x20000;
    PSYSTEM_PROCESS_INFORMATION pSysProcInfo = NULL;

    *ppHookList = NULL;
    *pulHookCount = 0;

    typedef PVOID(*PFN_PtiCurrent)();
    static PFN_PtiCurrent PtiCurrent = NULL;
    static BOOLEAN bFunctionsResolved = FALSE;

    if (!bFunctionsResolved) {
        PtiCurrent = (PFN_PtiCurrent)FindPtiCurrent();
        bFunctionsResolved = TRUE;
    }
    if (!PtiCurrent) {
        DbgPrint("[!] 无法获取PtiCurrent地址\n");
        return STATUS_NOT_FOUND;
    }

    pHookArray = (PWIN32K_MSG_HOOK_INFO)ExAllocatePool2(
        POOL_FLAG_NON_PAGED | POOL_FLAG_CACHE_ALIGNED,
        sizeof(WIN32K_MSG_HOOK_INFO) * uAllocatedCount,
        'MsHk'
    );
    if (!pHookArray) return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(pHookArray, sizeof(WIN32K_MSG_HOOK_INFO) * uAllocatedCount);

    // ==========================================
    // 全局钩子枚举
    // ==========================================
    DbgPrint("[*] 正在枚举全局钩子...\n");
    PVOID pCurrentPti = PtiCurrent();
    if (pCurrentPti)
    {
        __try {
            // WIN11_TI_GLOBAL_HOOK_CONTAINER
            // 读取全局钩子容器指针（pti + 0x1F8）
            PVOID pGlobalHookContainer = *(PVOID*)((PUCHAR)pCurrentPti + Offsets.Pti_pDeskInfo);
            if (pGlobalHookContainer)
            {
                EnumerateHooksFromPti(
                    pCurrentPti,             // 传入 pti
                    TRUE,                    // IsGlobal = TRUE
                    NULL, NULL,
                    pHookArray,
                    &uFoundHookCount,
                    uAllocatedCount,
                    &Offsets                 // 【新增】传入偏移结构体
                );
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("[!] 全局钩子枚举异常\n");
        }
    }

    // ==========================================
    // 线程局部钩子枚举
    // ==========================================
    DbgPrint("[*] 正在枚举线程局部钩子...\n");
    do {
        if (buffer) {
            ExFreePoolWithTag(buffer, 'enuT');
            buffer = NULL;
        }
        buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, bufferSize, 'enuT');
        if (!buffer) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

#define SystemProcessInformation 5
        status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, NULL);
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            bufferSize *= 2;
            continue;
        }
        else if (!NT_SUCCESS(status)) {
            break;
        }

        pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)buffer;
        while (TRUE) {
            for (ULONG i = 0; i < pSysProcInfo->NumberOfThreads; i++) {
                PSYSTEM_THREAD_INFORMATION pThread = &pSysProcInfo->Threads[i];
                HANDLE hTid = pThread->ClientId.UniqueThread;
                HANDLE hPid = pThread->ClientId.UniqueProcess;

                PETHREAD pEthread = NULL;
                status = PsLookupThreadByThreadId(hTid, &pEthread);
                if (!NT_SUCCESS(status)) continue;

                PVOID pW32Thread = PsGetThreadWin32Thread(pEthread);
                if (pW32Thread) {
                    // 【关键修正】W32THREAD 头部解引用获取 tagTHREADINFO*
                    PVOID pTargetPti = *(PVOID*)pW32Thread;

                    // 【新增】校验 pTargetPti 有效性
                    if (pTargetPti && MmIsAddressValid(pTargetPti)) {
                        EnumerateHooksFromPti(
                            pTargetPti,      // 传入 pti
                            FALSE,           // IsGlobal = FALSE
                            hPid, hTid,
                            pHookArray,
                            &uFoundHookCount,
                            uAllocatedCount,
                            &Offsets         // 【新增】传入偏移结构体
                        );
                    }
                }

                ObDereferenceObject(pEthread);
            }

            if (pSysProcInfo->NextEntryOffset == 0) break;
            pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pSysProcInfo + pSysProcInfo->NextEntryOffset);
        }
        break;
    } while (TRUE);

    if (buffer) ExFreePoolWithTag(buffer, 'enuT');

    if (uFoundHookCount == 0) {
        ExFreePoolWithTag(pHookArray, 'MsHk');
        return STATUS_NOT_FOUND;
    }

    *ppHookList = pHookArray;
    *pulHookCount = uFoundHookCount;
    DbgPrint("[+] 枚举完成，共找到 %d 个钩子\n", uFoundHookCount);
    return STATUS_SUCCESS;
}

NTSTATUS
EnumerateMsgHook_Win10(
    _In_opt_ PWIN32K_OFFSETS pOffsets,     // 【新增】允许外部传入偏移
    OUT PWIN32K_MSG_HOOK_INFO* ppHookList,
    OUT PULONG                   pulHookCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PWIN32K_MSG_HOOK_INFO pHookArray = NULL;
    ULONG uFoundHookCount = 0;
    ULONG uHookCount = 0;
    WIN32K_OFFSETS localOffsets = { 0 };

    // 初始化输出
    *ppHookList = NULL;
    *pulHookCount = 0;
    
    // ====================== 获取偏移 ======================
    if (!pOffsets) {
        PVOID zzzSetWindowsHookExAddr = NULL;
        if (KernelQuerySymbolAddress(L"win32kfull.sys", L"zzzSetWindowsHookEx",(PULONG64)&zzzSetWindowsHookExAddr) != STATUS_SUCCESS || !zzzSetWindowsHookExAddr) {
            DbgPrint("无法获取 zzzSetWindowsHookEx 地址!\n");
            return STATUS_NOT_FOUND;
		}
        PVOID xxxCallHookAddr = NULL;
        if (KernelQuerySymbolAddress(L"win32kfull.sys", L"xxxCallHook", (PULONG64)&xxxCallHookAddr) != STATUS_SUCCESS || !xxxCallHookAddr) {
            DbgPrint("无法获取 xxxCallHook 地址!\n");
            return STATUS_NOT_FOUND;
        }
        PVOID zzzUnhookWindowsHookExAddr = NULL;
        if (KernelQuerySymbolAddress(L"win32kfull.sys", L"zzzUnhookWindowsHookEx", (PULONG64)&zzzUnhookWindowsHookExAddr) != STATUS_SUCCESS || !zzzUnhookWindowsHookExAddr) {
            DbgPrint("无法获取 zzzUnhookWindowsHookEx 地址!\n");
            return STATUS_NOT_FOUND;
        }
        PVOID HMAllocObjectAddr = KernelGetProcAddress("win32kbase.sys", "HMAllocObject");

        status = Win32kOffsetScanner_Initialize(zzzSetWindowsHookExAddr, zzzUnhookWindowsHookExAddr, xxxCallHookAddr, HMAllocObjectAddr, &localOffsets);
        if (!NT_SUCCESS(status)) {
            DbgPrint("偏移扫描失败: 0x%08X\n", status);
            return status;
        }
        return STATUS_UNSUCCESSFUL;
        pOffsets = &localOffsets;
    }

    // ====================== 原有逻辑（动态化） ======================
    PWIN32K_GSHAREDINFO pSharedInfo =
        (PWIN32K_GSHAREDINFO)KernelGetProcAddress("win32kbase.sys", "gSharedInfo");
    if (!pSharedInfo || !pSharedInfo->pHookArray || !pSharedInfo->pHookMetadata) {
        DbgPrint("gSharedInfo 或钩子数组/元数据为空!\n");
        return STATUS_INVALID_PARAMETER;
    }

    uHookCount = *(PULONG)((PUCHAR)pSharedInfo->pHookArray + 8);
    if (uHookCount == 0 || uHookCount > 2048) {
        DbgPrint("无效的钩子数量: %lu\n", uHookCount);
        return STATUS_NOT_FOUND;
    }

    // 使用偏移结构中的元数据条目大小
    ULONG entrySize = pOffsets->HandleEntrySize;
    ULONG typeOffset = pOffsets->HandleEntry_HookTypeOffset;
    ULONG idxOffset = pOffsets->HandleEntry_TableIndexOffset;

    typedef PVOID(*PFN_HMValidateHandle)(HANDLE, UCHAR);
    static PFN_HMValidateHandle HMValidateHandle = NULL;
    if (!HMValidateHandle) {
        HMValidateHandle = (PFN_HMValidateHandle)FindHMValidateHandle();
    }
    if (!HMValidateHandle) {
        DbgPrint("HMValidateHandle 未找到!\n");
        return STATUS_NOT_FOUND;
    }

    // 分配内存
    pHookArray = (PWIN32K_MSG_HOOK_INFO)ExAllocatePool2(
        POOL_FLAG_NON_PAGED | POOL_FLAG_CACHE_ALIGNED,
        sizeof(WIN32K_MSG_HOOK_INFO) * uHookCount,
        'MsHk'
    );
    if (!pHookArray) {
        DbgPrint("内存分配失败!\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHookArray, sizeof(WIN32K_MSG_HOOK_INFO) * uHookCount);

    DbgPrint("开始遍历数组，钩子数量=%d, 元数据条目大小=%d\n", uHookCount, entrySize);

    // ====================== 遍历 + 填充结构体（全部动态化） ======================
    for (ULONG i = 0; i < uHookCount; i++)
    {
        PUCHAR pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * entrySize);
        UCHAR HookType = *(PUCHAR)(pMetaEntry + typeOffset);
        USHORT TableIndex = *(PUSHORT)(pMetaEntry + idxOffset);

        if (HookType != TYPE_HOOK) continue;

        PWIN32K_MSG_HOOK_INFO pCurHook = &pHookArray[uFoundHookCount++];
        pCurHook->HookType = HookType;

        // 构造句柄
        HANDLE hHookHandle = (HANDLE)(i | ((ULONG)TableIndex << 16));
        pCurHook->HookHandle = hHookHandle;

        PVOID pHookObject = HMValidateHandle(hHookHandle, TYPE_HOOK);
        if (!pHookObject) {
            DbgPrint("HMValidateHandle 验证失败，句柄=0x%p\n", hHookHandle);
            continue;
        }
        if (!MmIsAddressValid(pHookObject) ||
            !MmIsAddressValid((PUCHAR)pHookObject + pOffsets->Hook_ObjectSize)) {
            DbgPrint("钩子对象内存无效，地址=0x%p\n", pHookObject);
            continue;
        }

        // ====================== 动态偏移读取 ======================
        pCurHook->HookType = *(PLONG)((PUCHAR)pHookObject + pOffsets->Hook_nHookType);
        ULONG64 qwFlags = *(PULONG64)((PUCHAR)pHookObject + pOffsets->Hook_flags);
        pCurHook->HookFlags = (ULONG)(qwFlags & 0xFFFFFFFF);
        pCurHook->HookProc = *(PULONG64)((PUCHAR)pHookObject + pOffsets->Hook_offPfn);

        // ====================== 全局/低级钩子判断 ======================
        BOOLEAN IsLowLevel = (TableIndex == 13 || TableIndex == 14);
        pCurHook->IsGlobal = ((pCurHook->HookFlags & HF_GLOBAL) != 0) || IsLowLevel;

        // PID/TID
        PETHREAD* ppThread = *(PETHREAD**)((PUCHAR)pHookObject + pOffsets->Hook_pti);
        if (ppThread && MmIsAddressValid((PVOID)ppThread) && *ppThread && MmIsAddressValid(*ppThread)) {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            pCurHook->ProcessId = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            pCurHook->ThreadId = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
        }
        else {
			DbgPrint("无法获取钩子对象的线程信息，地址=0x%p\n", ppThread);
        }

        // ====================== ihmod 模块解析 ======================
        ULONG ihmod = *(PULONG)((PUCHAR)pHookObject + pOffsets->Hook_ihmod);

        if (ihmod != 0xFFFFFFFF && ihmod > 0 && ihmod <= 32)
        {
            GetModuleNameFromihMod(FALSE, (int)ihmod, pCurHook->ModulePath);
        }
    }

    // ====================== 返回结果 ======================
    if (uFoundHookCount == 0) {
        ExFreePool(pHookArray);
        return STATUS_NOT_FOUND;
    }

    *ppHookList = pHookArray;
    *pulHookCount = uFoundHookCount;
    DbgPrint("消息钩子枚举完成，共找到 %d 个\n", uFoundHookCount);
    return STATUS_SUCCESS;
}

// 辅助函数：内存十六进制转储（内核安全版，DebugView 紧凑输出）
VOID DumpMemoryHex(_In_ PVOID Address, _In_ ULONG Length)
{
    PUCHAR pBuffer = (PUCHAR)Address;
    DbgPrint("内存转储 [0x%p, 0x%X 字节]:\n", Address, Length);

    // 每行缓冲区（内核安全，足够大）
    CHAR lineBuffer[256] = { 0 };

    for (ULONG i = 0; i < Length; i += 16)
    {
        // 先清空行缓冲区
        RtlZeroMemory(lineBuffer, sizeof(lineBuffer));

        // 拼接偏移地址 0000:
        RtlStringCbPrintfA(
            lineBuffer,
            sizeof(lineBuffer),
            "%04X: ",
            i
        );
        
        // 获取当前已使用长度
        size_t usedLen = 0;
        RtlStringCbLengthA(lineBuffer, sizeof(lineBuffer), &usedLen);

        // 拼接 16 字节十六进制（带空格）
        for (ULONG j = 0; j < 16; j++)
        {
            if (i + j < Length)
            {
                // 内核安全拼接：XX （带空格）
                RtlStringCbPrintfA(
                    lineBuffer + usedLen,
                    sizeof(lineBuffer) - usedLen,
                    "%02X ",
                    pBuffer[i + j]
                );
            }
            else
            {
                // 不足补空格
                RtlStringCbPrintfA(
                    lineBuffer + usedLen,
                    sizeof(lineBuffer) - usedLen,
                    "   "
                );
            }

            // 更新已使用长度
            RtlStringCbLengthA(lineBuffer, sizeof(lineBuffer), &usedLen);
        }

        // ✅ 整行只打印一次，完美适配 DebugView
        DbgPrint("%s\n", lineBuffer);
    }
}

// 修改后的核心枚举函数
NTSTATUS EnumerateEventHook_Win11(
    OUT PWIN32K_EVENT_HOOK_INFO* ppHookList,
    OUT PULONG                       pulHookCount
)
{
    PFN_W32GetUserSessionState W32GetUserSessionState = NULL;
    PVOID pGpsi = NULL;
    PINTERNAL_EVENT_HOOK pHookHead = NULL;
    PINTERNAL_EVENT_HOOK pCurrentHook = NULL;
    PWIN32K_EVENT_HOOK_INFO pHookList = NULL;
    ULONG hookCount = 0;
    ULONG currentIndex = 0;

    // 1. 输入参数校验
    if (!ppHookList || !pulHookCount)
        return STATUS_INVALID_PARAMETER;

    *ppHookList = NULL;
    *pulHookCount = 0;

    // 2. 获取Win32k导出函数W32GetUserSessionState
    W32GetUserSessionState = (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys","W32GetUserSessionState");
    if (!W32GetUserSessionState || !MmIsAddressValid((PVOID)W32GetUserSessionState))
        return STATUS_PROCEDURE_NOT_FOUND;

    // 3. 获取全局会话信息结构体gpsi
    pGpsi = W32GetUserSessionState();
    if (!pGpsi)
        return STATUS_INVALID_ADDRESS;

    DbgPrint("\n=============================================\n");
    DbgPrint("gpsi 基址: 0x%p\n", pGpsi);
    DbgPrint("事件钩子链表头偏移: 0x%X\n", WIN11_GPSI_EVENT_HOOK_LIST_OFFSET);

    // 4. 获取事件钩子全局链表头
    pHookHead = *(PINTERNAL_EVENT_HOOK*)((PUCHAR)pGpsi + WIN11_GPSI_EVENT_HOOK_LIST_OFFSET);
    DbgPrint("事件钩子链表头: 0x%p\n", pHookHead);
    DbgPrint("=============================================\n\n");

    if (!pHookHead)
        return STATUS_SUCCESS; // 无钩子，返回成功

    // 5. 第一次遍历：统计钩子数量
    pCurrentHook = pHookHead;
    while (pCurrentHook && MmIsAddressValid(pCurrentHook))
    {
        hookCount++;
        pCurrentHook = pCurrentHook->pNext;
    }

    if (hookCount == 0)
        return STATUS_SUCCESS;

    // 6. 分配内存存储钩子信息
    pHookList = (PWIN32K_EVENT_HOOK_INFO)ExAllocatePool2(POOL_FLAG_NON_PAGED,
        hookCount * sizeof(WIN32K_EVENT_HOOK_INFO), EVENT_HOOK_POOL_TAG);
    if (!pHookList)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(pHookList, hookCount * sizeof(WIN32K_EVENT_HOOK_INFO));

    // 7. 第二次遍历：填充钩子信息并打印验证
    pCurrentHook = pHookHead;
    while (pCurrentHook && MmIsAddressValid(pCurrentHook) && currentIndex < hookCount)
    {
        PWIN32K_EVENT_HOOK_INFO pCurrentInfo = &pHookList[currentIndex];

        HANDLE hInstallerPid = NULL, hInstallerTid = NULL;

        if (pCurrentHook->pInstallerThread && MmIsAddressValid(pCurrentHook->pInstallerThread))
        {
            // 从 ETHREAD 获取 CID (Client ID)
            // 注意: ETHREAD 的 Cid 偏移在 Win11 上通常是 0x4e8
            // 但为了稳定性，我们使用 PsGetThreadId 和 PsGetThreadProcessId
            hInstallerPid = PsGetThreadProcessId(*pCurrentHook->pInstallerThread);
            hInstallerTid = PsGetThreadId(*pCurrentHook->pInstallerThread);

            /*DbgPrint("安装者 PETHREAD: 0x%p\n", *pCurrentHook->pInstallerThread);
            DbgPrint("安装者 PID: %d\n", (ULONG)(ULONG_PTR)hInstallerPid);
            DbgPrint("安装者 TID: %d\n", (ULONG)(ULONG_PTR)hInstallerTid);*/
        }
        else
        {
            //DbgPrint("安装者 ETHREAD: 无效或为空\n");
        }

        // 填充基础信息
		pCurrentInfo->HookHandle = pCurrentHook->HookHandle;
        pCurrentInfo->EventMin = pCurrentHook->EventMin;
        pCurrentInfo->EventMax = pCurrentHook->EventMax;
        pCurrentInfo->Flags = pCurrentHook->Flags;
        pCurrentInfo->ProcessId = (ULONG)(ULONG_PTR)hInstallerPid;
        pCurrentInfo->ThreadId = (ULONG)(ULONG_PTR)hInstallerTid;
		pCurrentInfo->TargetProcessId = (ULONG)pCurrentHook->TargetProcessId;
		pCurrentInfo->TargetThreadId = (ULONG)pCurrentHook->TargetThreadId;
        pCurrentInfo->hmodWinEventProc = (PVOID)pCurrentHook->Unknown48;
        pCurrentInfo->pfnWinEventProc = pCurrentHook->pfnWinEventProc;

        RtlZeroMemory(pCurrentInfo->ModulePath, sizeof(pCurrentInfo->ModulePath));
        USHORT idx = (USHORT)(ULONG_PTR)pCurrentInfo->hmodWinEventProc;

        // 情况1：ihmod索引（小整数）
        if (idx <= 32)
        {
            NTSTATUS modStatus = GetModuleNameFromihMod(TRUE, idx, pCurrentInfo->ModulePath);
            if (NT_SUCCESS(modStatus))
            {
                //DbgPrint(" -> 模块名: %ws\n", pCurrentInfo->ModulePath);
            }
        }
        // 情况2：跨进程钩子
        else if ((ULONG)(ULONG_PTR)pCurrentInfo->hmodWinEventProc == (ULONG)-1)
        {
            //DbgPrint("  跨进程钩子，hmod=0x%p\n", pCurrentInfo->hmodWinEventProc);
        }
        // 情况3：有效回调地址
        else if ((ULONG_PTR)pCurrentInfo->pfnWinEventProc >= 0x10000)
        {
            pCurrentInfo->ActualCallback = pCurrentInfo->pfnWinEventProc;
        }

        currentIndex++;
        pCurrentHook = pCurrentHook->pNext;
    }

    DbgPrint("\n=============================================\n");
    DbgPrint("枚举完成，共找到 %lu 个WinEventHook\n", currentIndex);
    DbgPrint("=============================================\n");

    // 8. 输出结果
    *ppHookList = pHookList;
    *pulHookCount = currentIndex;

    return STATUS_SUCCESS;
}

// ====================== 枚举事件钩子 → 返回结构体数组 ======================
NTSTATUS
EnumerateEventHook_Win10(
    OUT PWIN32K_EVENT_HOOK_INFO* ppHookList,
    OUT PULONG                       pulHookCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PWIN32K_EVENT_HOOK_INFO pHookArray = NULL;
    ULONG uFoundHookCount = 0;
    ULONG uHookCount = 0;

    // 初始化输出参数
    *ppHookList = NULL;
    *pulHookCount = 0;

    PWIN32K_GSHAREDINFO pSharedInfo =
        (PWIN32K_GSHAREDINFO)KernelGetProcAddress("win32kbase.sys", "gSharedInfo");
    // 1. 校验共享信息
    if (!pSharedInfo || pSharedInfo == (PVOID)-1)
    {
        DbgPrint("gSharedInfo 无效!\n");
        return STATUS_INVALID_PARAMETER;
    }
    if (!pSharedInfo->pHookArray || !pSharedInfo->pHookMetadata)
    {
        DbgPrint("钩子数组/元数据为空!\n");
        return STATUS_INVALID_PARAMETER;
    }

    // 2. 读取钩子总数
    uHookCount = *(PULONG)((PUCHAR)pSharedInfo->pHookArray + 8);
    if (uHookCount == 0 || uHookCount > 2048)
    {
        DbgPrint("无效钩子数量: %lu\n", uHookCount);
        return STATUS_NOT_FOUND;
    }

    // 3. 系统版本判断
    RTL_OSVERSIONINFOW osVer = { sizeof(osVer) };
    status = RtlGetVersion(&osVer);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("获取系统版本失败!\n");
        return status;
    }
    BOOLEAN bIsOldSystem = (osVer.dwMajorVersion < 10 || osVer.dwBuildNumber < WIN10_1703_BUILD_NUMBER);

    // 4. 初始化HMValidateHandle
    typedef PVOID(*PFN_HMValidateHandle)(HANDLE, UCHAR);
    static PFN_HMValidateHandle HMValidateHandle = NULL;
    if (!HMValidateHandle)
    {
        HMValidateHandle = (PFN_HMValidateHandle)FindHMValidateHandle();
    }
    if (!HMValidateHandle)
    {
        DbgPrint("HMValidateHandle 未找到!\n");
        return STATUS_NOT_FOUND;
    }

    // ====================== 预分配内存（最大可能钩子数） ======================
    pHookArray = (PWIN32K_EVENT_HOOK_INFO)ExAllocatePool2(
        POOL_FLAG_NON_PAGED | POOL_FLAG_CACHE_ALIGNED,
        sizeof(WIN32K_EVENT_HOOK_INFO) * uHookCount,
        'EvHk'  // 内存标签：EvHk (EventHook)
    );
    if (!pHookArray)
    {
        DbgPrint("内存分配失败!\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHookArray, sizeof(WIN32K_EVENT_HOOK_INFO) * uHookCount);

    DbgPrint("========== 开始枚举事件钩子 (总数=%d) ==========\n", uHookCount);

    // 5. 遍历钩子数组 → 填充结构体
    for (ULONG i = 0; i < uHookCount; i++)
    {
        PUCHAR pMetaEntry = NULL;
        UCHAR HookType = 0;
        USHORT TableIndex = 0;

        // 元数据解析
        if (bIsOldSystem)
        {
            pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * 24);
            HookType = *(PUCHAR)(pMetaEntry + 16);
            TableIndex = *(PUSHORT)(pMetaEntry + 18);
        }
        else
        {
            pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * 32);
            HookType = *(PUCHAR)(pMetaEntry + 24);
            TableIndex = *(PUSHORT)(pMetaEntry + 26);
        }

        // 只处理事件钩子
        if (HookType != TYPE_WINEVENTHOOK)
            continue;

        // ====================== 获取当前要填充的结构体对象 ======================
        PWIN32K_EVENT_HOOK_INFO pCurHook = &pHookArray[uFoundHookCount];
        uFoundHookCount++;

        // 构造钩子句柄
        HANDLE hEventHook = (HANDLE)(i | ((ULONG)TableIndex << 16));
        pCurHook->HookHandle = hEventHook;

        //DbgPrint("[%d] 找到事件钩子，TableIndex=%d\n", uFoundHookCount, TableIndex);

        // 验证句柄
        PVOID pEventHookObj = HMValidateHandle(hEventHook, TYPE_WINEVENTHOOK);
        if (!pEventHookObj || !MmIsAddressValid(pEventHookObj))
        {
            DbgPrint("  → 句柄验证失败: 0x%p\n", hEventHook);
            continue;
        }

        // ====================== 读取钩子核心字段 → 存入结构体 ======================
        pCurHook->EventMin = *(PULONG)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_EVENTMIN);
        pCurHook->EventMax = *(PULONG)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_EVENTMAX);
        pCurHook->Flags = *(PULONG)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_FLAGS);
        pCurHook->hmodWinEventProc = *(PVOID*)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_HMOD);
        pCurHook->pfnWinEventProc = *(PVOID*)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_PFN);
        pCurHook->TargetProcessId = *(PULONG)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_TARGET_PID);
        pCurHook->TargetThreadId = *(PULONG)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_TARGET_TID);

        // 读取进程/线程ID
        ULONG idProcess = 0, idThread = 0;
        PETHREAD* ppThread = *(PETHREAD**)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_THREAD);
        if (ppThread && MmIsAddressValid(ppThread) && *ppThread)
        {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            idProcess = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            idThread = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
			DbgPrint("  → 关联线程有效，PID=%d, TID=%d\n", idProcess, idThread);
        }
        pCurHook->ProcessId = idProcess;
        pCurHook->ThreadId = idThread;

        // ====================== 解析回调/模块 → 存入结构体 ======================
        BOOLEAN resolved = FALSE;
        RtlZeroMemory(pCurHook->ModulePath, sizeof(pCurHook->ModulePath));

        // 情况1：ihmod索引（小整数）
        if ((ULONG_PTR)pCurHook->hmodWinEventProc < 0x10000)
        {
            USHORT idx = (USHORT)(ULONG_PTR)pCurHook->hmodWinEventProc;
            if (idx != 0)
            {
                NTSTATUS modStatus = GetModuleNameFromihMod(FALSE, idx, pCurHook->ModulePath);
                if (NT_SUCCESS(modStatus))
                {
                    resolved = TRUE;
                    //DbgPrint(" -> 模块名: %ws\n", pCurHook->ModulePath);
                }
            }
        }
        // 情况2：跨进程钩子
        else if ((ULONG_PTR)pCurHook->hmodWinEventProc == (ULONG)-1)
        {
            //DbgPrint("  跨进程钩子，hmod=0x%p\n", pCurHook->hmodWinEventProc);
            resolved = TRUE;
        }
        // 情况3：有效回调地址
        else if ((ULONG_PTR)pCurHook->pfnWinEventProc >= 0x10000)
        {
            pCurHook->ActualCallback = pCurHook->pfnWinEventProc;
            resolved = TRUE;
        }

        //pCurHook->IsResolved = resolved;

        // ====================== 调试打印（保留原有输出） ======================
        DbgPrint("  ==================== 事件钩子详情 ====================\n");
        DbgPrint("  hHookHandle     = 0x%p\n", pCurHook->HookHandle);
        DbgPrint("  eventMin        = 0x%08X\n", pCurHook->EventMin);
        DbgPrint("  eventMax        = 0x%08X\n", pCurHook->EventMax);
        DbgPrint("  hmodWinEventProc= 0x%p\n", pCurHook->hmodWinEventProc);
        DbgPrint("  pfnWinEventProc = 0x%p\n", pCurHook->pfnWinEventProc);
        if (resolved)
        {
            if (pCurHook->ActualCallback)
                DbgPrint(" -> 实际回调地址: 0x%p\n", pCurHook->ActualCallback);
            else
                DbgPrint(" -> 模块名: %ws\n", pCurHook->ModulePath);
        }
        DbgPrint("  idProcess       = %d\n", pCurHook->ProcessId);
        DbgPrint("  idThread        = %d\n", pCurHook->ThreadId);
        DbgPrint("  dwFlags         = 0x%08X\n", pCurHook->Flags);
        DbgPrint("  ======================================================\n\n");
    }

    DbgPrint("========== 事件钩子枚举完成 (找到 %d 个) ==========\n", uFoundHookCount);

    // ====================== 返回结果给调用者 ======================
    if (uFoundHookCount == 0)
    {
        ExFreePoolWithTag(pHookArray, 'EvHk');
        return STATUS_NOT_FOUND;
    }

    *ppHookList = pHookArray;
    *pulHookCount = uFoundHookCount;

    return STATUS_SUCCESS;
}

PVOID FindgphkHashTable()
{
    ULONG64 addr = 0;
    NTSTATUS status = KernelQuerySymbolAddress(L"win32kfull.sys", L"gphkHashTable", &addr);
    if (status == STATUS_SUCCESS && addr)
    {
        DbgPrint("Symbol: gphkHashTable=%p\n", (PVOID)addr);
        return (PVOID)addr;
    }
    DbgPrint("Symbol failed, fallback to pattern scan\n");
    // -------------------------------------------
    PVOID NtUserRegisterHotKeyAddr = KernelGetProcAddress("win32kfull.sys", "NtUserRegisterHotKey");

    if (NULL == NtUserRegisterHotKeyAddr)
    {
        DbgPrint("NtUserRegisterHotKeyAddr is NULL");
        return NULL;
    }

    /*
    从 NtUserRegisterHotKey 搜索调用 _RegisterHotKey 的位置：

    .text:00000001C0032940                 mov     rcx, rax        ; struct tagWND *
    .text:00000001C0032943                 call    _RegisterHotKey

    特征码：48 8B C8 (mov rcx, rax) + E8 (call rel32)
    */
    UCHAR pCallRegisterHotKey[4] = { 0x48, 0x8B, 0xC8, 0xE8 };
    PVOID pCallSite = SearchSpecialCode(NtUserRegisterHotKeyAddr, 0x200, pCallRegisterHotKey, 4);

    if (NULL == pCallSite)
    {
        DbgPrint("Call _RegisterHotKey not found");
        return NULL;
    }

    // 计算 _RegisterHotKey 地址
    LONG callOffset = *(PLONG)((PUCHAR)pCallSite + 4);
    PVOID RegisterHotKeyAddr = (PVOID)((PUCHAR)pCallSite + 8 + callOffset);

    DbgPrint("_RegisterHotKey at: %p", RegisterHotKeyAddr);

    /*
    在 _RegisterHotKey 中搜索 gphkHashTable 访问：

    .text:00000001C0032DA8                 and     ecx, 7Fh        ; 83 E1 7F
    .text:00000001C0032DAB                 mov     rax, rva ?gphkHashTable@@3PAPEAUtagHOTKEY@@A[r13+rcx*8]
                                            ; 49 8B 84 CD XX XX XX XX

    特征码：83 E1 7F (and ecx, 7Fh) + 49 8B 84 CD (mov rax, [r13+rcx*8+disp32])
    */
    UCHAR pSpecialCode[8] = { 0x83, 0xE1, 0x7F, 0x49, 0x8B, 0x84, 0xCD };
    PVOID result = SearchSpecialCode(RegisterHotKeyAddr, 0x400, pSpecialCode, 7);

    if (NULL == result)
    {
        DbgPrint("gphkHashTable pattern not found");
        return NULL;
    }

    /*
    计算 gphkHashTable 地址：

    指令：mov rax, [r13+rcx*8+0xXXXXXXXX]
    偏移位置：result + 7 开始的4字节是相对RIP的偏移量

    实际地址 = R13 + disp32
    但 R13 在 x64 内核中通常指向 PCR/PRCB 或 KPCR

    实际上 gphkHashTable 是相对于 RIP 的地址（R13 是重定位基址）：
    在 Win10 x64 中，r13 通常用于存储模块基址或重定位信息

    指令格式：mov rax, [r13 + rcx*8 + disp32]
    disp32 位于 result + 7 的位置
    */

    LONG disp32 = *(PLONG)((PUCHAR)result + 7);

    /*
    计算 gphkHashTable 的 RVA（相对于模块基址）：
    由于指令使用 r13+rcx*8+disp32，其中 r13 是重定位基址
    disp32 就是 gphkHashTable 相对于 r13 的偏移

    我们需要获取当前模块（win32kbase.sys 或 win32kfull.sys）的基址
    然后加上 disp32 得到实际地址
    */

    // 获取包含 _RegisterHotKey 的模块基址
	UNICODE_STRING moduleName = RTL_CONSTANT_STRING(L"win32kfull.sys");
    PVOID moduleBase = GetModuleBase(moduleName, NULL);
    if (NULL == moduleBase)
    {
        DbgPrint("Failed to get module base");
        return NULL;
    }

    // gphkHashTable 地址 = 模块基址 + disp32（RVA）
    PVOID gphkHashTable = (PVOID)((PUCHAR)moduleBase + disp32);

    DbgPrint("gphkHashTable found at: %p", gphkHashTable);

    return gphkHashTable;
}

#define HOTKEY_POOL_TAG 'ktHo'
#define HOTKEY_HASH_BUCKET_COUNT 0x80

// 函数指针定义（仅Win11使用）
//typedef PVOID(*PFN_W32GetUserSessionState)();

// ==============================================
// 【保留】你原有的Win10枚举逻辑（完全不变，仅内部调用）
// ==============================================
static NTSTATUS EnumerateHotkey_Win10(
    OUT PWIN32K_HOTKEY_INFO* ppHotkeyList,
    OUT PULONG pulHotkeyCount
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PWIN32K_HOTKEY_INFO pHotkeyArray = NULL;
    ULONG hotkeyCount = 0;
    PTAG_HOTKEY* gphkHashTable = NULL;

    *ppHotkeyList = NULL;
    *pulHotkeyCount = 0;

    // 【保留】你原有的Win10全局哈希表获取逻辑
    gphkHashTable = (PTAG_HOTKEY*)FindgphkHashTable();
    if (!gphkHashTable || !MmIsAddressValid(gphkHashTable))
    {
        DbgPrint("[Win10] gphkHashTable 未找到\n");
        return STATUS_NOT_FOUND;
    }
    DbgPrint("[Win10] 成功获取哈希表: %p\n", gphkHashTable);

    // 【保留】你原有的统计逻辑
    for (ULONG i = 0; i < HOTKEY_HASH_BUCKET_COUNT; i++)
    {
        PTAG_HOTKEY pHotkey = gphkHashTable[i];
        while (pHotkey && MmIsAddressValid(pHotkey))
        {
            hotkeyCount++;
            pHotkey = pHotkey->pNext;
        }
    }

    if (hotkeyCount == 0)
    {
        DbgPrint("[Win10] 未枚举到任何热键\n");
        return STATUS_SUCCESS;
    }

    // 【保留】你原有的内存分配逻辑
    pHotkeyArray = (PWIN32K_HOTKEY_INFO)ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount,
        HOTKEY_POOL_TAG
    );
    if (!pHotkeyArray)
    {
        DbgPrint("[Win10] 内存分配失败\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHotkeyArray, sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount);

    // 【保留】你原有的遍历 + hWnd解析逻辑
    ULONG index = 0;
    for (ULONG i = 0; i < HOTKEY_HASH_BUCKET_COUNT; i++)
    {
        PTAG_HOTKEY pHotkey = gphkHashTable[i];
        while (pHotkey && MmIsAddressValid(pHotkey) && index < hotkeyCount)
        {
            PETHREAD pEthread = NULL;
            HANDLE TID = NULL, PID = NULL;
            _HWND hWnd = NULL;

            // 【保留】你原有的线程信息读取逻辑
            _try
            {
                tagTHREADINFO * pThreadInfo = (tagTHREADINFO*)pHotkey->pThreadInfo;
                if (MmIsAddressValid(pThreadInfo)) pEthread = (PETHREAD)pThreadInfo->pEThread;
                if (pEthread && MmIsAddressValid(pEthread))
                {
                    TID = PsGetThreadId(pEthread);
                    PID = PsGetProcessId(IoThreadToProcess(pEthread));
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("[Win10] Exception at read pThreadInfo\n");
            }

            // 【保留】你原有的hWnd解析逻辑（完全不变）
            _try
            {
                hWnd = *(_HWND*)((PUCHAR)pHotkey->hWnd + 0x0);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("[Win10] Exception at read hWnd\n");
            }

            // 【保留】你原有的数据填充逻辑
            pHotkeyArray[index].HotkeyHandle = NULL;
            pHotkeyArray[index].vk = pHotkey->vk;
            pHotkeyArray[index].mod = pHotkey->fsModifiers;
            pHotkeyArray[index].id = pHotkey->id;
            pHotkeyArray[index].hWnd = hWnd;
            pHotkeyArray[index].ProcessId = (ULONG)(ULONG_PTR)PID;
            pHotkeyArray[index].ThreadId = (ULONG)(ULONG_PTR)TID;

            /*DbgPrint("[Win10][%03d] hWnd=0x%p, vk=0x%02X, mod=0x%04X, id=0x%x, PID=%Iu, TID=%Iu\n",
                index, hWnd, pHotkey->vk, pHotkey->fsModifiers, pHotkey->id,
                (ULONG_PTR)PID, (ULONG_PTR)TID);*/

            index++;
            pHotkey = pHotkey->pNext;
        }
    }

    *ppHotkeyList = pHotkeyArray;
    *pulHotkeyCount = hotkeyCount;
    status = STATUS_SUCCESS;

    DbgPrint("[Win10] 枚举完成，总热键数: %lu\n", hotkeyCount);
    return status;
}

// ==============================================
// 【最终修正】Win11 热键枚举逻辑
// ==============================================
static NTSTATUS EnumerateHotkey_Win11(
    OUT PWIN32K_HOTKEY_INFO* ppHotkeyList,
    OUT PULONG pulHotkeyCount
)
{
    //NTSTATUS status = STATUS_UNSUCCESSFUL;
    PWIN32K_HOTKEY_INFO pHotkeyArray = NULL;
    ULONG hotkeyCount = 0;
    PFN_W32GetUserSessionState W32GetUserSessionState = NULL;
    PVOID pSession = NULL;

    *ppHotkeyList = NULL;
    *pulHotkeyCount = 0;

    // 1. 获取导出函数
    W32GetUserSessionState = (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
    if (!W32GetUserSessionState) return STATUS_NOT_FOUND;

    // 2. 获取会话与哈希表
    pSession = W32GetUserSessionState();
    if (!pSession || !MmIsAddressValid(pSession)) return STATUS_INVALID_ADDRESS;
    PHOTKEY_HASH_TABLE_WIN11 gphkHashTable = (PHOTKEY_HASH_TABLE_WIN11)((PUCHAR)pSession + 0x3298);
    if (!gphkHashTable || !MmIsAddressValid(gphkHashTable)) return STATUS_NOT_FOUND;

    // 3. 统计数量
    for (ULONG i = 0; i < 0x80; i++) {
        PTAG_HOTKEY_WIN11 pHotkey = gphkHashTable->Buckets[i];
        while (pHotkey) {
            if (!MmIsAddressValid(pHotkey)) break;
            PTAG_HOTKEY_WIN11 pNext = pHotkey->pNext;
            if (pNext && !MmIsAddressValid(pNext)) break;
            hotkeyCount++;
            pHotkey = pNext;
        }
    }
    if (hotkeyCount == 0) return STATUS_SUCCESS;

    // 4. 分配内存
    pHotkeyArray = (PWIN32K_HOTKEY_INFO)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount, HOTKEY_POOL_TAG);
    if (!pHotkeyArray) return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(pHotkeyArray, sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount);

    // 5. 遍历填充
    ULONG index = 0;
    for (ULONG i = 0; i < 0x80; i++) {
        PTAG_HOTKEY_WIN11 pHotkey = gphkHashTable->Buckets[i];
        while (pHotkey && index < hotkeyCount) {
            if (!MmIsAddressValid(pHotkey)) break;
            PTAG_HOTKEY_WIN11 pNext = pHotkey->pNext;
            if (pNext && !MmIsAddressValid(pNext)) break;

            // ==============================================
            // 【修正1】安全读取字段
            // ==============================================
            WORD vk = (WORD)pHotkey->vk;
            WORD mod = pHotkey->fsModifiers;
            ULONG_PTR id = (ULONG_PTR)pHotkey->id;  // 从0x28读取id

            // ==============================================
            // 【修正2】从 pWnd 推导 hWnd（关键！）
            // ==============================================
            PVOID64 hWnd = NULL;
            __try {
                if (pHotkey->hWnd && MmIsAddressValid(pHotkey->hWnd)) {
                    // 假设 tagWND+0x08 是 hWnd（需根据你的调试输出调整！）
                    // 可通过 DbgPrint 查看 pWnd 指向的内存来确认偏移
                    hWnd = *(_HWND*)pHotkey->hWnd;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                hWnd = NULL;
            }

            // ==============================================
            // 【修正3】调试输出（验证用）
            // ==============================================
                        // 【可选】调试输出，匹配你之前的日志格式
            __try
            {
                /*DbgPrint("HOTKEY=%p\n", pHotkey);
                DbgPrint("  q0=%p\n", *(PVOID*)((PUCHAR)pHotkey + 0x00));
                DbgPrint("  q1=%p\n", *(PVOID*)((PUCHAR)pHotkey + 0x08));
                DbgPrint("  q2=%p\n", *(PVOID*)((PUCHAR)pHotkey + 0x10));
                DbgPrint("  q3=%p\n", *(PVOID*)((PUCHAR)pHotkey + 0x18));
                DbgPrint("  w20=%04x w22=%04x d24=%08x\n",
                    *(PUSHORT)((PUCHAR)pHotkey + 0x20),
                    *(PUSHORT)((PUCHAR)pHotkey + 0x22),
                    *(PULONG)((PUCHAR)pHotkey + 0x24));
                DbgPrint("  q28=%p q30=%p q38=%p\n",
                    *(PVOID*)((PUCHAR)pHotkey + 0x28),
                    *(PVOID*)((PUCHAR)pHotkey + 0x30),
                    *(PVOID*)((PUCHAR)pHotkey + 0x38));*/
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("[Win11] 热键信息读取异常: %p\n", pHotkey);
            }
            /*DbgPrint("[Win11][%03d] HOTKEY=%p, vk=0x%02X, mod=0x%04X, id=0x%p, hWnd=%p\n",
                index, pHotkey, vk, mod, (PVOID)id, hWnd);*/

            // 填充数据
            pHotkeyArray[index].HotkeyHandle = pHotkey;
            pHotkeyArray[index].vk = vk;
            pHotkeyArray[index].mod = mod;
            pHotkeyArray[index].id = (ULONG)id; // 若id是64位，根据需求调整
            pHotkeyArray[index].hWnd = hWnd;
			pHotkeyArray[index].pfnCallback = pHotkey->pCallback;
            
            if (pHotkey->pThreadInfo) {
                PETHREAD pEthread = *(PETHREAD*)pHotkey->pThreadInfo;
                if (pEthread && MmIsAddressValid(pEthread)) {
                    pHotkeyArray[index].ThreadId = (ULONG)(ULONG_PTR)PsGetThreadId(pEthread);
                    pHotkeyArray[index].ProcessId = (ULONG)(ULONG_PTR)PsGetProcessId(IoThreadToProcess(pEthread));
                }
			}

            index++;
            pHotkey = pNext;
        }
    }

    *ppHotkeyList = pHotkeyArray;
    *pulHotkeyCount = index;
    return STATUS_SUCCESS;
}

// ==============================================
// 【主入口】你原有的EnumHotkey函数（仅增加Win11备选逻辑）
// ==============================================
NTSTATUS EnumMsgHook(
    OUT PWIN32K_MSG_HOOK_INFO* ppMsgHookList,
    OUT PULONG pulMsgHookCount
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // 初始化输出参数
    *ppMsgHookList = NULL;
    *pulMsgHookCount = 0;

    DbgPrint("==================== 开始枚举消息钩子 ====================\n");

    // 【优先】尝试你原有的Win10枚举逻辑
    status = EnumerateMsgHook_Win10(NULL, ppMsgHookList, pulMsgHookCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win10枚举成功 ====================\n");
        return status;
    }

    // 【备选】Win10失败时，尝试Win11枚举逻辑
    DbgPrint("Win10枚举失败，尝试Win11枚举...\n");
    status = EnumerateMsgHook_Win11(ppMsgHookList, pulMsgHookCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win11枚举成功 ====================\n");
        return status;
    }

    // 都失败
    DbgPrint("==================== 所有枚举方式均失败 ====================\n");
    return status;
}

// ==============================================
// 【主入口】你原有的EnumEventHook函数（仅增加Win11备选逻辑）
// ==============================================
NTSTATUS EnumEventHook(
    OUT PWIN32K_EVENT_HOOK_INFO* ppEventHookList,
    OUT PULONG pulEventHookCount
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // 初始化输出参数
    *ppEventHookList = NULL;
    *pulEventHookCount = 0;

    DbgPrint("==================== 开始枚举事件钩子 ====================\n");

    // 【优先】尝试你原有的Win10枚举逻辑
    status = EnumerateEventHook_Win10(ppEventHookList, pulEventHookCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win10枚举成功 ====================\n");
        return status;
    }

    // 【备选】Win10失败时，尝试Win11枚举逻辑
    DbgPrint("Win10枚举失败，尝试Win11枚举...\n");
    status = EnumerateEventHook_Win11(ppEventHookList, pulEventHookCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win11枚举成功 ====================\n");
        return status;
    }

    // 都失败
    DbgPrint("==================== 所有枚举方式均失败 ====================\n");
    return status;
}

// ==============================================
// 【主入口】你原有的EnumHotkey函数（仅增加Win11备选逻辑）
// ==============================================
NTSTATUS EnumHotkey(
    OUT PWIN32K_HOTKEY_INFO* ppHotkeyList,
    OUT PULONG pulHotkeyCount
)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // 初始化输出参数
    *ppHotkeyList = NULL;
    *pulHotkeyCount = 0;

    DbgPrint("==================== 开始枚举热键 ====================\n");

    // 【优先】尝试你原有的Win10枚举逻辑
    status = EnumerateHotkey_Win10(ppHotkeyList, pulHotkeyCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win10枚举成功 ====================\n");
        return status;
    }

    // 【备选】Win10失败时，尝试Win11枚举逻辑
    DbgPrint("Win10枚举失败，尝试Win11枚举...\n");
    status = EnumerateHotkey_Win11(ppHotkeyList, pulHotkeyCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win11枚举成功 ====================\n");
        return status;
    }

    // 都失败
    DbgPrint("==================== 所有枚举方式均失败 ====================\n");
    return status;
}