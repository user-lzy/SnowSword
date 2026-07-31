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

// ============================================
// 各版本签名表 (按构建号排序)
// ============================================

// Win10 1607 (Build 14393)
static const TIMER_ERA_SIGNATURE g_TimerSig_Win10_1607 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x60,
    .Timer_TimeStamp = 0x80,
    .Timer_HashListEntry = 0x70,
    .Timer_ObjectSize = 0x88,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = FALSE,
    .SessionHashOffset = 0,
    .HashTableSymbol = "gTimerHashTable",
    .Pti_ProcessOffset = 0x170,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 14393,
    .OsBuildMax = 14393,
    .EraName = "Win10_1607",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// Win10 1809 (Build 17763)
static const TIMER_ERA_SIGNATURE g_TimerSig_Win10_1809 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x60,
    .Timer_TimeStamp = 0x80,
    .Timer_HashListEntry = 0x70,
    .Timer_ObjectSize = 0x88,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = FALSE,
    .SessionHashOffset = 0,
    .HashTableSymbol = "gTimerHashTable",
    .Pti_ProcessOffset = 0x1A0,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 17763,
    .OsBuildMax = 17763,
    .EraName = "Win10_1809",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// Win10 20H2 (Build 19042)
static const TIMER_ERA_SIGNATURE g_TimerSig_Win10_20H2 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x60,
    .Timer_TimeStamp = 0x80,
    .Timer_HashListEntry = 0x70,
    .Timer_ObjectSize = 0x88,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = FALSE,
    .SessionHashOffset = 0,
    .HashTableSymbol = "gTimerHashTable",
    .Pti_ProcessOffset = 0x1A8,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 19042,
    .OsBuildMax = 19042,
    .EraName = "Win10_20H2",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// Win11 21H2/22H2 (Build 22000-22631)
static const TIMER_ERA_SIGNATURE g_TimerSig_Win11_21H2_22H2 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x60,
    .Timer_TimeStamp = 0x80,
    .Timer_HashListEntry = 0x70,
    .Timer_ObjectSize = 0x88,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = FALSE,
    .SessionHashOffset = 0,
    .HashTableSymbol = "gTimerHashTable",
    .Pti_ProcessOffset = 0x1A0,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 22000,
    .OsBuildMax = 22631,
    .EraName = "Win11_21H2_22H2",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// Win11 23H2 (Build 22631)
static const TIMER_ERA_SIGNATURE g_TimerSig_Win11_23H2 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x60,
    .Timer_TimeStamp = 0x80,
    .Timer_HashListEntry = 0x70,
    .Timer_ObjectSize = 0x88,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = FALSE,
    .SessionHashOffset = 0,
    .HashTableSymbol = "gTimerHashTable",
    .Pti_ProcessOffset = 0x1A0,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 22631,
    .OsBuildMax = 26099,
    .EraName = "Win11_23H2",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// Win11 24H2+ (Build 26100+) - 已反汇编验证
static const TIMER_ERA_SIGNATURE g_TimerSig_Win11_24H2 = {
    .Timer_pThreadInfo = 0x18,
    .Timer_pfn = 0x20,
    .Timer_nTimeout = 0x28,
    .Timer_nIDEvent = 0x2C,
    .Timer_flags = 0x30,
    .Timer_nTimeoutDup = 0x34,
    .Timer_windowPtr = 0x58,
    .Timer_nIDEventDup = 0x70,
    .Timer_TimeStamp = 0x88,
    .Timer_HashListEntry = 0x78,
    .Timer_ObjectSize = 0x90,
    .Timer_HashBuckets = 64,
    .bUseSessionHashTable = TRUE,
    .SessionHashOffset = 0xF0A0,
    .HashTableSymbol = NULL,
    .Pti_ProcessOffset = 0x1D0,
    .Pti_EthreadOffset = 0x00,
    .Flag_InUse = 0x1,
    .Flag_Deleted = 0x1000,
    .OsBuildMin = 26100,
    .OsBuildMax = 0xFFFFFFFF,
    .EraName = "Win11_24H2+",
    .EnterCritSymbol = "EnterCrit",
    .LeaveCritSymbol = "UserSessionSwitchLeaveCrit",
};

// 签名表数组 (按构建号排序，必须以 NULL 结尾)
static const PCTIMER_ERA_SIGNATURE g_TimerSignatures[] = {
    &g_TimerSig_Win10_1607,
    &g_TimerSig_Win10_1809,
    &g_TimerSig_Win10_20H2,
    &g_TimerSig_Win11_21H2_22H2,
    &g_TimerSig_Win11_23H2,
    &g_TimerSig_Win11_24H2,
    NULL
};

static TIMER_CONTEXT g_TimerCtx = { 0 };

// ============================================
// 初始化
// ============================================

NTSTATUS InitializeTimerContext() {
    if (g_TimerCtx.EnterCrit) return STATUS_SUCCESS;

    // 1. 获取通用函数
    g_TimerCtx.EnterCrit = (PFN_EnterCrit)KernelGetProcAddress("win32kbase.sys", "EnterCrit");
    g_TimerCtx.UserSessionSwitchLeaveCrit = (PFN_UserSessionSwitchLeaveCrit)KernelGetProcAddress("win32kbase.sys", "UserSessionSwitchLeaveCrit");
    g_TimerCtx.ValidateHwnd = (PFN_ValidateHwnd)KernelGetProcAddress("win32kbase.sys", "ValidateHwnd");

    if (!g_TimerCtx.EnterCrit || !g_TimerCtx.UserSessionSwitchLeaveCrit || !g_TimerCtx.ValidateHwnd) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    // 2. 获取 OS 构建号
    RTL_OSVERSIONINFOW osVer = { 0 };
    osVer.dwOSVersionInfoSize = sizeof(osVer);
    RtlGetVersion(&osVer);
    ULONG osBuild = osVer.dwBuildNumber;

    // 3. 匹配签名
    for (ULONG i = 0; g_TimerSignatures[i] != NULL; i++) {
        const TIMER_ERA_SIGNATURE* sig = g_TimerSignatures[i];
        if (osBuild >= sig->OsBuildMin && osBuild <= sig->OsBuildMax) {
            g_TimerCtx.pSig = sig;
            break;
        }
    }

    // 降级策略：使用最高已知兼容签名
    if (!g_TimerCtx.pSig) {
        DbgPrint("[Timer] 不支持的 OS 构建号: %lu, 尝试使用最近兼容签名\n", osBuild);
        g_TimerCtx.pSig = g_TimerSignatures[0];
        for (ULONG i = 1; g_TimerSignatures[i] != NULL; i++) {
            if (g_TimerSignatures[i]->OsBuildMin <= osBuild) {
                g_TimerCtx.pSig = g_TimerSignatures[i];
            }
        }
    }

    DbgPrint("[Timer] 匹配签名: %s (Build %lu)\n", g_TimerCtx.pSig->EraName, osBuild);

    // 4. 获取哈希表基址
    if (g_TimerCtx.pSig->bUseSessionHashTable) {
        // Win11 24H2+: 通过 W32GetUserSessionState
        g_TimerCtx.W32GetUserSessionState = (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
        if (!g_TimerCtx.W32GetUserSessionState) {
            DbgPrint("[Timer] W32GetUserSessionState 未找到，尝试回退到 gTimerHashTable\n");
            // 回退：尝试全局符号
            g_TimerCtx.HashTableBase = (PVOID)KernelGetProcAddress("win32kbase.sys", "gTimerHashTable");
            if (!g_TimerCtx.HashTableBase) {
                return STATUS_PROCEDURE_NOT_FOUND;
            }
        }
        else {
            g_TimerCtx.HashTableBase = (PVOID)((PUCHAR)g_TimerCtx.W32GetUserSessionState() + g_TimerCtx.pSig->SessionHashOffset);
            DbgPrint("[Timer] SessionState = %p, HashTableBase = %p (Offset = 0x%X)\n",
                g_TimerCtx.W32GetUserSessionState(), g_TimerCtx.HashTableBase, g_TimerCtx.pSig->SessionHashOffset);
        }
    }
    else {
        // Win10 / Win11 早期: 直接导出符号
        g_TimerCtx.HashTableBase = (PVOID)KernelGetProcAddress("win32kbase.sys", g_TimerCtx.pSig->HashTableSymbol);
    }

    if (!g_TimerCtx.HashTableBase) {
        DbgPrint("[Timer] 无法获取哈希表基址\n");
        return STATUS_INVALID_ADDRESS;
    }

    DbgPrint("[Timer] 哈希表基址: %p\n", g_TimerCtx.HashTableBase);
    return STATUS_SUCCESS;
}

// ============================================
// 定时器枚举 (统一偏移方式，保留原有解析逻辑)
// ============================================

NTSTATUS EnumProcessTimers(
    _Out_ PWINDOW_TIMER* pArray,
    _Out_ PULONG pCount
) {
    NTSTATUS status = STATUS_SUCCESS;
    ULONG timerCount = 0;
    PWINDOW_TIMER pResultArray = NULL;
    const TIMER_ERA_SIGNATURE* sig = g_TimerCtx.pSig;
    if (!sig) {
        status = InitializeTimerContext();
        if (!NT_SUCCESS(status)) {
            DbgPrint("[TIMER_ENUM] 初始化上下文失败！0x%X\n", status);
            return status;
        }
        sig = g_TimerCtx.pSig;
	}
    if (!pArray || !pCount || !sig) {
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
            PVOID hashBase = g_TimerCtx.HashTableBase;
            if (!hashBase || !MmIsAddressValid(hashBase)) {
                status = STATUS_INVALID_ADDRESS;
                goto EXIT_LABEL;
            }

            DbgPrint("[TIMER_ENUM] 哈希表基址: %p\n", hashBase);
            DbgPrint("[TIMER_ENUM] 开始遍历 %lu 个哈希桶...\n", sig->Timer_HashBuckets);

            for (ULONG bucket = 0; bucket < sig->Timer_HashBuckets; bucket++) {
                PLIST_ENTRY* bucketPtr = (PLIST_ENTRY*)((PUCHAR)hashBase + sizeof(LIST_ENTRY) * bucket);
                if (!MmIsAddressValid(bucketPtr)) continue;
                PLIST_ENTRY head = *bucketPtr;
                if (!head || !MmIsAddressValid(head)) continue;

                // 遍历链表
                for (PLIST_ENTRY entry = head; entry != (PLIST_ENTRY)bucketPtr; entry = entry->Flink) {
                    if (!MmIsAddressValid(entry) || !MmIsAddressValid(entry->Flink)) break;

                    // ======================
                    // 统一偏移方式获取 Timer 对象基址
                    // ======================
                    PUCHAR pTimerObj = (PUCHAR)entry - sig->Timer_HashListEntry;
                    if (!MmIsAddressValid(pTimerObj)) continue;

                    // 验证对象大小范围 (防止越界)
                    if (!MmIsAddressValid(pTimerObj + sig->Timer_ObjectSize - 1)) continue;

                    // 获取 flags
                    ULONG flags = *(PULONG)(pTimerObj + sig->Timer_flags);

                    // 检查"使用中"标志 (bit 0)
                    /*if (!(flags & sig->Flag_InUse)) {
                        continue;
                    }*/

                    // 跳过已删除的定时器
                    if ((flags & sig->Flag_Deleted)) continue;
					DbgPrint("[TIMER_ENUM] 匹配到定时器对象: %p, flags=0x%X\n", pTimerObj, flags);

                    // ======================
                    // PID/TID 解析 (保留原有逻辑)
                    // ======================
                    PVOID pti = *(PVOID*)(pTimerObj + sig->Timer_pThreadInfo);
                    if (!pti) continue;

                    // 线程ID（原有逻辑）
                    PETHREAD pEthread = NULL;
                    if (sig->Pti_EthreadOffset == 0) {
                        pEthread = *(PETHREAD*)pti;
                    }
                    else {
                        pEthread = *(PETHREAD*)((PUCHAR)pti + sig->Pti_EthreadOffset);
                    }
                    if (!pEthread) continue;
                    HANDLE dwProcessId = PsGetThreadProcessId(pEthread);
                    HANDLE dwThreadId = PsGetThreadId(pEthread);

                    // ======================
                    // 内存分配（原有逻辑）
                    // ======================
                    PWINDOW_TIMER pNewArray = (PWINDOW_TIMER)KernelAlloc_NonPagedPoolNx(
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

                    // ======================
                    // 填充数据（使用签名偏移）
                    // ======================
                    pNewArray[timerCount].pfn = *(PVOID*)(pTimerObj + sig->Timer_pfn);
                    pNewArray[timerCount].nTimeout = *(PLONG)(pTimerObj + sig->Timer_nTimeout);
                    pNewArray[timerCount].nIDEvent = *(PLONG)(pTimerObj + sig->Timer_nIDEvent);
                    pNewArray[timerCount].ThreadId = dwThreadId;
                    pNewArray[timerCount].ProcessId = dwProcessId;

                    PVOID pWnd = *(PVOID*)(pTimerObj + sig->Timer_windowPtr);
                    pNewArray[timerCount].hWnd = NULL;
                    if (pWnd && MmIsAddressValid(pWnd))
                        pNewArray[timerCount].hWnd = *(PVOID*)pWnd;

                    // 打印日志（保留）
                    DbgPrint("[TIMER_ENUM] 找到定时器！pTimer=0x%p, PID=%llu, TID=%llu, nIDEvent=%lld, pWnd=0x%p, hWnd=0x%p\n",
                        pTimerObj, (ULONG64)dwProcessId, (ULONG64)dwThreadId,
                        pNewArray[timerCount].nIDEvent,
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
        KernelGetProcAddress("ntoskrnl.exe", "RtlQueryAtomInAtomTable");

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

    WIN32K_FUNCADDR FuncAddr = { 0 };
    WIN32K_OFFSETS Offsets = { 0 };

    FuncAddr.zzzSetWindowHookExAddr = zzzSetWindowsHookEx;
    FuncAddr.xxxCallHookAddr = xxxCallHook;
    if (!NT_SUCCESS(Win32kOffsetScanner_Initialize(&FuncAddr, &Offsets)))
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

    pHookArray = (PWIN32K_MSG_HOOK_INFO)KernelAlloc_NonPagedPoolNx(
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
        buffer = KernelAlloc_NonPagedPoolNx(POOL_FLAG_NON_PAGED, bufferSize, 'enuT');
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
        if (KernelQuerySymbolAddress(L"win32kfull.sys", L"zzzSetWindowsHookEx", (PULONG64)&zzzSetWindowsHookExAddr) != STATUS_SUCCESS || !zzzSetWindowsHookExAddr) {
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

        WIN32K_FUNCADDR FuncAddr = { 0 };

        FuncAddr.zzzSetWindowHookExAddr = zzzSetWindowsHookExAddr;
        FuncAddr.xxxCallHookAddr = xxxCallHookAddr;
        FuncAddr.zzzUnhookWindowsHookExAddr = zzzUnhookWindowsHookExAddr;
        FuncAddr.HMAllocObjectAddr = HMAllocObjectAddr;
        status = Win32kOffsetScanner_Initialize(&FuncAddr, &localOffsets);
        if (!NT_SUCCESS(status)) {
            DbgPrint("偏移扫描失败: 0x%08X\n", status);
            return status;
        }
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
    pHookArray = (PWIN32K_MSG_HOOK_INFO)KernelAlloc_NonPagedPoolNx(
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

// ============================================================
// EnumerateEventHook_Win10 - 使用动态偏移
// ============================================================
NTSTATUS EnumerateEventHook_Win10(
    IN PWIN32K_OFFSETS pOffsets,
    OUT PWIN32K_EVENT_HOOK_INFO* ppHookList,
    OUT PULONG pulHookCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PWIN32K_EVENT_HOOK_INFO pHookArray = NULL;
    ULONG uFoundHookCount = 0;
    ULONG uHookCount = 0;

    WIN32K_OFFSETS localOffsets = { 0 };
    BOOLEAN bUseLocal = FALSE;

    // 初始化输出
    *ppHookList = NULL;
    *pulHookCount = 0;

    // 若未提供偏移，自动扫描
    if (!pOffsets) {
        PVOID _SetWinEventHookAddr = NULL, xxxWindowEventAddr = NULL;
        KernelQuerySymbolAddress(L"win32kfull.sys", L"_SetWinEventHook", (PULONG64)&_SetWinEventHookAddr);
        KernelQuerySymbolAddress(L"win32kfull.sys", L"xxxWindowEvent", (PULONG64)&xxxWindowEventAddr);

        WIN32K_FUNCADDR FuncAddr = { 0 };

        FuncAddr._SetWinEventHookAddr = _SetWinEventHookAddr;
        FuncAddr.xxxWindowEventAddr = xxxWindowEventAddr;

        status = Win32kOffsetScanner_Initialize(&FuncAddr, &localOffsets);
        if (!NT_SUCCESS(status)) {
            DbgPrint("[EV] 偏移扫描失败: 0x%08X\n", status);
            return status;
        }
        pOffsets = &localOffsets;
        bUseLocal = TRUE;
    }

    // 获取共享信息
    PWIN32K_GSHAREDINFO pSharedInfo =
        (PWIN32K_GSHAREDINFO)KernelGetProcAddress("win32kbase.sys", "gSharedInfo");
    if (!pSharedInfo || pSharedInfo == (PVOID)-1) {
        DbgPrint("[EV] gSharedInfo 无效!\n");
        return STATUS_INVALID_PARAMETER;
    }
    if (!pSharedInfo->pHookArray || !pSharedInfo->pHookMetadata) {
        DbgPrint("[EV] 钩子数组/元数据为空!\n");
        return STATUS_INVALID_PARAMETER;
    }

    // 钩子总数
    uHookCount = *(PULONG)((PUCHAR)pSharedInfo->pHookArray + 8);
    if (uHookCount == 0 || uHookCount > 2048) {
        DbgPrint("[EV] 无效钩子数量: %lu\n", uHookCount);
        return STATUS_NOT_FOUND;
    }

    // 元数据大小和偏移
    ULONG entrySize = pOffsets->HandleEntrySize;
    ULONG typeOffset = pOffsets->HandleEntry_HookTypeOffset;
    ULONG idxOffset = pOffsets->HandleEntry_TableIndexOffset;

    // HMValidateHandle
    typedef PVOID(*PFN_HMValidateHandle)(HANDLE, UCHAR);
    static PFN_HMValidateHandle HMValidateHandle = NULL;
    if (!HMValidateHandle) {
        HMValidateHandle = (PFN_HMValidateHandle)FindHMValidateHandle();
    }
    if (!HMValidateHandle) {
        DbgPrint("[EV] HMValidateHandle 未找到!\n");
        return STATUS_NOT_FOUND;
    }

    // 分配结果数组
    pHookArray = (PWIN32K_EVENT_HOOK_INFO)KernelAlloc_NonPagedPoolNx(
        POOL_FLAG_NON_PAGED | POOL_FLAG_CACHE_ALIGNED,
        sizeof(WIN32K_EVENT_HOOK_INFO) * uHookCount,
        'EvHk');
    if (!pHookArray) {
        DbgPrint("[EV] 内存分配失败!\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHookArray, sizeof(WIN32K_EVENT_HOOK_INFO) * uHookCount);

    DbgPrint("[EV] 开始枚举事件钩子 (总数=%d)\n", uHookCount);

    // 遍历钩子数组
    for (ULONG i = 0; i < uHookCount; i++) {
        PUCHAR pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * entrySize);
        UCHAR HookType = *(PUCHAR)(pMetaEntry + typeOffset);
        USHORT TableIndex = *(PUSHORT)(pMetaEntry + idxOffset);

        if (HookType != TYPE_WINEVENTHOOK)
            continue;

        PWIN32K_EVENT_HOOK_INFO pCur = &pHookArray[uFoundHookCount++];
        HANDLE hEventHook = (HANDLE)(i | ((ULONG)TableIndex << 16));
        pCur->HookHandle = hEventHook;

        PVOID pObj = HMValidateHandle(hEventHook, TYPE_WINEVENTHOOK);
        if (!pObj || !MmIsAddressValid(pObj)) {
            continue;
        }

        PUCHAR pBase = (PUCHAR)pObj;

        // 使用动态偏移读取字段
        pCur->EventMin = *(PULONG)(pBase + pOffsets->EventHook_eventMin);
        pCur->EventMax = *(PULONG)(pBase + pOffsets->EventHook_eventMax);
        pCur->Flags = *(PULONG)(pBase + pOffsets->EventHook_flags);
        pCur->hmodWinEventProc = *(PVOID*)(pBase + pOffsets->EventHook_ihmod);
        pCur->pfnWinEventProc = *(PVOID*)(pBase + pOffsets->EventHook_pfn);
        pCur->TargetProcessId = *(PULONG)(pBase + pOffsets->EventHook_idProcess);
        pCur->TargetThreadId = *(PULONG)(pBase + pOffsets->EventHook_idThread);

        // 获取安装者 PID/TID（通过 pti）
        ULONG idProcess = 0, idThread = 0;
        PETHREAD* ppThread = *(PETHREAD**)(pBase + pOffsets->EventHook_pti);
        if (ppThread && MmIsAddressValid(ppThread) && *ppThread) {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            idProcess = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            idThread = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
        }
        pCur->ProcessId = idProcess;
        pCur->ThreadId = idThread;

        // 解析模块（ihmod 索引）
        BOOLEAN resolved = FALSE;
        RtlZeroMemory(pCur->ModulePath, sizeof(pCur->ModulePath));
        ULONG_PTR hmodVal = (ULONG_PTR)pCur->hmodWinEventProc;
        if (hmodVal < 0x10000) {
            USHORT idx = (USHORT)hmodVal;
            if (idx != 0 && NT_SUCCESS(GetModuleNameFromihMod(FALSE, idx, pCur->ModulePath))) {
                resolved = TRUE;
            }
        }
        else if (hmodVal == (ULONG)-1) {
            resolved = TRUE; // 跨进程标记
        }
        else if ((ULONG_PTR)pCur->pfnWinEventProc >= 0x10000) {
            pCur->ActualCallback = pCur->pfnWinEventProc;
            resolved = TRUE;
        }

        // 调试输出（可选）
        DbgPrint("[EV] Hook 0x%p: eventMin=0x%X, eventMax=0x%X, flags=0x%X, pid=%d, tid=%d\n",
            pCur->HookHandle, pCur->EventMin, pCur->EventMax, pCur->Flags,
            pCur->TargetProcessId, pCur->TargetThreadId);
    }

    DbgPrint("[EV] 枚举完成，找到 %d 个事件钩子\n", uFoundHookCount);

    if (uFoundHookCount == 0) {
        ExFreePoolWithTag(pHookArray, 'EvHk');
        return STATUS_NOT_FOUND;
    }

    *ppHookList = pHookArray;
    *pulHookCount = uFoundHookCount;
    return STATUS_SUCCESS;
}

// ============================================================
// EnumerateEventHook_Win11 - 使用动态偏移
// ============================================================
NTSTATUS EnumerateEventHook_Win11(
    IN PWIN32K_OFFSETS pOffsets,
    OUT PWIN32K_EVENT_HOOK_INFO* ppHookList,
    OUT PULONG pulHookCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    WIN32K_OFFSETS localOffsets = { 0 };
    BOOLEAN bUseLocal = FALSE;

    *ppHookList = NULL;
    *pulHookCount = 0;

    // 若未提供偏移，自动扫描
    if (!pOffsets) {
        PVOID _SetWinEventHookAddr = NULL, xxxWindowEventAddr = NULL;
        KernelQuerySymbolAddress(L"win32kfull.sys", L"_SetWinEventHook", (PULONG64)&_SetWinEventHookAddr);
        KernelQuerySymbolAddress(L"win32kfull.sys", L"xxxWindowEvent", (PULONG64)&xxxWindowEventAddr);
        if (_SetWinEventHookAddr == NULL) KernelQuerySymbolAddress(L"win32kfull.sys", L"SetWinEventHook", (PULONG64)&_SetWinEventHookAddr);

        WIN32K_FUNCADDR FuncAddr = { 0 };

        FuncAddr._SetWinEventHookAddr = _SetWinEventHookAddr;
        FuncAddr.xxxWindowEventAddr = xxxWindowEventAddr;

        status = Win32kOffsetScanner_Initialize(&FuncAddr, &localOffsets);
        if (!NT_SUCCESS(status)) {
            DbgPrint("[EV11] 偏移扫描失败: 0x%08X\n", status);
            return status;
        }
        pOffsets = &localOffsets;
        bUseLocal = TRUE;
    }

    // 获取 W32GetUserSessionState
    PFN_W32GetUserSessionState W32GetUserSessionState =
        (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
    if (!W32GetUserSessionState || !MmIsAddressValid((PVOID)W32GetUserSessionState)) {
        DbgPrint("[EV11] W32GetUserSessionState 未找到\n");
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    PVOID pGpsi = W32GetUserSessionState();
    if (!pGpsi) {
        DbgPrint("[EV11] gpsi 无效\n");
        return STATUS_INVALID_ADDRESS;
    }

    // 根据动态偏移获取链表头
    PVOID pHookHead = *(PVOID*)((PUCHAR)pGpsi + pOffsets->GPSI_EVENT_HOOK_LIST_OFFSET);
    if (!pHookHead) {
        DbgPrint("[EV11] 事件钩子链表为空\n");
        return STATUS_SUCCESS;
    }

    DbgPrint("[EV11] gpsi=0x%p, 链表头偏移=0x%X, head=0x%p\n",
        pGpsi, pOffsets->GPSI_EVENT_HOOK_LIST_OFFSET, pHookHead);

    // 第一次遍历统计数量
    ULONG hookCount = 0;
    PUCHAR pNode = (PUCHAR)pHookHead;
    while (pNode && MmIsAddressValid(pNode)) {
        hookCount++;
        pNode = *(PUCHAR*)(pNode + pOffsets->EventHook_pNext);
    }

    if (hookCount == 0)
        return STATUS_SUCCESS;

    // 分配结果数组
    PWIN32K_EVENT_HOOK_INFO pHookList = (PWIN32K_EVENT_HOOK_INFO)KernelAlloc_NonPagedPoolNx(
        POOL_FLAG_NON_PAGED,
        hookCount * sizeof(WIN32K_EVENT_HOOK_INFO),
        'EvHk');
    if (!pHookList) {
        DbgPrint("[EV11] 内存分配失败\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHookList, hookCount * sizeof(WIN32K_EVENT_HOOK_INFO));

    // 第二次遍历填充信息
    ULONG idx = 0;
    pNode = (PUCHAR)pHookHead;
    while (pNode && MmIsAddressValid(pNode) && idx < hookCount) {
        PWIN32K_EVENT_HOOK_INFO pCur = &pHookList[idx++];

        // 读取各个字段（使用动态偏移）
        pCur->HookHandle = *(HANDLE*)pNode;                       // 假设句柄在 +0x00
        pCur->EventMin = *(PULONG)(pNode + pOffsets->EventHook_eventMin);
        pCur->EventMax = *(PULONG)(pNode + pOffsets->EventHook_eventMax);
        pCur->Flags = *(PULONG)(pNode + pOffsets->EventHook_flags);
        pCur->TargetProcessId = *(PULONG)(pNode + pOffsets->EventHook_idProcess);
        pCur->TargetThreadId = *(PULONG)(pNode + pOffsets->EventHook_idThread);
        pCur->pfnWinEventProc = *(PVOID*)(pNode + pOffsets->EventHook_pfn);
        pCur->hmodWinEventProc = *(PVOID*)(pNode + pOffsets->EventHook_ihmod); // 可能是 ihmod 或 hmod

        // 安装者 PID/TID（通过 pti）
        ULONG pid = 0, tid = 0;
        PETHREAD* ppThread = *(PETHREAD**)(pNode + pOffsets->EventHook_pti);
        if (ppThread && MmIsAddressValid(ppThread) && *ppThread) {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            pid = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            tid = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
        }
        pCur->ProcessId = pid;
        pCur->ThreadId = tid;

        // 模块解析（ihmod）
        RtlZeroMemory(pCur->ModulePath, sizeof(pCur->ModulePath));
        ULONG_PTR hmodVal = (ULONG_PTR)pCur->hmodWinEventProc;
        if (hmodVal < 0x10000) {
            USHORT ih = (USHORT)hmodVal;
            if (ih != 0 && NT_SUCCESS(GetModuleNameFromihMod(TRUE, ih, pCur->ModulePath))) {
                // 已解析
            }
        }
        else if (hmodVal == (ULONG)-1) {
            // 跨进程
        }
        else if ((ULONG_PTR)pCur->pfnWinEventProc >= 0x10000) {
            pCur->ActualCallback = pCur->pfnWinEventProc;
        }

        // 移动到下一个节点
        pNode = *(PUCHAR*)(pNode + pOffsets->EventHook_pNext);
    }

    DbgPrint("[EV11] 枚举完成，共 %lu 个事件钩子\n", idx);

    *ppHookList = pHookList;
    *pulHookCount = idx;
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
    status = EnumerateEventHook_Win10(NULL, ppEventHookList, pulEventHookCount);
    if (NT_SUCCESS(status))
    {
        DbgPrint("==================== Win10枚举成功 ====================\n");
        return status;
    }

    // 【备选】Win10失败时，尝试Win11枚举逻辑
    DbgPrint("Win10枚举失败，尝试Win11枚举...\n");
    status = EnumerateEventHook_Win11(NULL, ppEventHookList, pulEventHookCount);
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
    WIN32K_OFFSETS offsets = { 0 };
    WIN32K_FUNCADDR funcAddr = { 0 };
    PWIN32K_HOTKEY_INFO pHotkeyArray = NULL;
    ULONG hotkeyCount = 0;
    PVOID pHashTable = NULL;

    *ppHotkeyList = NULL;
    *pulHotkeyCount = 0;

    // ---------- 1. 获取所有必要的函数地址 ----------
    KernelQuerySymbolAddress(L"win32kfull.sys", L"_RegisterHotKey", (PULONG64)&funcAddr._RegisterHotKeyAddr);
    KernelQuerySymbolAddress(L"win32kfull.sys", L"HKInsertHashElement", (PULONG64)&funcAddr.HKInsertHashElementAddr);

    // ---------- 2. 调用扫描器初始化偏移 ----------
    status = Win32kOffsetScanner_Initialize(&funcAddr, &offsets);
    //if (!NT_SUCCESS(status) || offsets.bHotkeyVerificationFailed) {
    //    DbgPrint("[EnumHotkey] 偏移扫描初始化失败 (0x%X), 验证标记=%d\n",
    //        status, offsets.bHotkeyVerificationFailed);
    //    return status;
    //}

    if (!NT_SUCCESS(status)) {
        DbgPrint("[EnumHotkey] 偏移扫描初始化失败 (0x%X)\n",
            status);
        return status;
    }

    // ---------- 3. 获取哈希表基址 ----------
    if (offsets.Hotkey_bSessionHashTable) {
        // Windows 11 24H2+ 使用会话内哈希表
        PFN_W32GetUserSessionState W32GetUserSessionState =
            (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
        if (!W32GetUserSessionState) {
            DbgPrint("[EnumHotkey] 无法获取 W32GetUserSessionState\n");
            return STATUS_PROCEDURE_NOT_FOUND;
        }
        PVOID pSession = W32GetUserSessionState();
        if (!pSession || !MmIsAddressValid(pSession)) {
            DbgPrint("[EnumHotkey] 会话指针无效\n");
            return STATUS_INVALID_ADDRESS;
        }
        pHashTable = (PUCHAR)pSession + offsets.Hotkey_SessionHashOffset;
        if (!pHashTable || !MmIsAddressValid(pHashTable)) {
            DbgPrint("[EnumHotkey] 会话哈希表地址无效\n");
            return STATUS_NOT_FOUND;
        }
    }
    else {
        // 其他版本使用全局 gphkHashTable
        KernelQuerySymbolAddress(L"win32kfull.sys", L"gphkHashTable", (PULONG64)&pHashTable);
        if (!pHashTable || !MmIsAddressValid(pHashTable)) {
            DbgPrint("[EnumHotkey] 全局哈希表地址无效\n");
            return STATUS_NOT_FOUND;
        }
    }

    // ---------- 4. 第一遍遍历：统计热键数量 ----------
    ULONG bucketCount = offsets.Hotkey_HashBuckets;   // 通常为 0x80
    for (ULONG i = 0; i < bucketCount; i++) {
        PVOID pNode = *(PVOID*)((PUCHAR)pHashTable + i * sizeof(PVOID));
        while (pNode) {
            __try {
                if (!MmIsAddressValid(pNode)) break;
                hotkeyCount++;

                // 单向链表
                PVOID pNext = *(PVOID*)((PUCHAR)pNode + offsets.Hotkey_pNext);
                if (pNext && !MmIsAddressValid(pNext)) break;
				DbgPrint("[EnumHotkey] Bucket %lu, Node 0x%p, Next 0x%p\n", i, pNode, pNext);
                pNode = pNext;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DbgPrint("[EnumHotkey] 遍历异常，终止当前桶\n");
                break;
            }
        }
    }

    if (hotkeyCount == 0) {
        DbgPrint("[EnumHotkey] 未发现热键\n");
        return STATUS_SUCCESS;
    }

    // ---------- 5. 分配输出缓冲区 ----------
    pHotkeyArray = (PWIN32K_HOTKEY_INFO)KernelAlloc_NonPagedPoolNx(
        POOL_FLAG_NON_PAGED,
        sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount,
        HOTKEY_POOL_TAG
    );
    if (!pHotkeyArray) {
        DbgPrint("[EnumHotkey] 内存分配失败\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(pHotkeyArray, sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount);

    // ---------- 6. 第二遍遍历：填充详细信息 ----------
    ULONG index = 0;
    for (ULONG i = 0; i < bucketCount; i++) {
        PVOID pNode = *(PVOID*)((PUCHAR)pHashTable + i * sizeof(PVOID));
        while (pNode && MmIsAddressValid(pNode) && index < hotkeyCount) {
            __try {
                // ---- 读取字段 ----
                PVOID pThreadInfo = *(PVOID*)((PUCHAR)pNode + offsets.Hotkey_pThreadInfo);
                PVOID pfnCallback = *(PVOID*)((PUCHAR)pNode + offsets.Hotkey_pfnCallback);
                WORD vk = *(WORD*)((PUCHAR)pNode + offsets.Hotkey_vk);
                //WORD mod = *(WORD*)((PUCHAR)pNode + offsets.Hotkey_fsModifiers);
                WORD modLow = *(WORD*)((PUCHAR)pNode + 0x18) & 0xF;      // 低 4 位
                WORD modHigh = *(WORD*)((PUCHAR)pNode + 0x1A) & 0x7800;   // 高位部分
                DWORD mod = (DWORD)(modHigh | modLow);              // 组合成完整值
                ULONG id = 0;
                if (offsets.Hotkey_id) {
                    id = *(ULONG*)((PUCHAR)pNode + offsets.Hotkey_id);
                }

                // 获取窗口句柄
                _HWND hWnd = NULL;
                /*if (offsets.Hotkey_hWnd) {
                    hWnd = *(_HWND*)((PUCHAR)pNode + offsets.Hotkey_hWnd);
                }*/
                _try{
                    if (offsets.Hotkey_pWnd) {
						PVOID pWnd = *(PVOID*)((PUCHAR)pNode + offsets.Hotkey_pWnd);
                        hWnd = *(_HWND*)((PUCHAR)pWnd);
                    }
                }
                _except(EXCEPTION_EXECUTE_HANDLER) {
                    DbgPrint("Exception at read hWnd\n");
                }

                // ---- 获取进程/线程 ID ----
                ULONG pid = 0, tid = 0;
                if (pThreadInfo && MmIsAddressValid(pThreadInfo)) {
                    PETHREAD pEThread = *(PETHREAD*)pThreadInfo;
                    if (pEThread && MmIsAddressValid(pEThread)) {
                        tid = (ULONG)(ULONG_PTR)PsGetThreadId(pEThread);
                        pid = (ULONG)(ULONG_PTR)PsGetProcessId(IoThreadToProcess(pEThread));
                    }
                }

                // ---- 填充输出 ----
                pHotkeyArray[index].HotkeyHandle = pNode;
                pHotkeyArray[index].vk = vk;
                pHotkeyArray[index].mod = mod;
                pHotkeyArray[index].id = id;
                pHotkeyArray[index].hWnd = hWnd;
                pHotkeyArray[index].ProcessId = pid;
                pHotkeyArray[index].ThreadId = tid;
                pHotkeyArray[index].pfnCallback = pfnCallback;
                DbgPrint("Hotkey %d, vk=%d, mod=%d", index, vk, mod);

                index++;

                // ---- 下一个节点 ----
                PVOID pNext = *(PVOID*)((PUCHAR)pNode + offsets.Hotkey_pNext);
				//DbgPrint("[EnumHotkey] Bucket %lu, Node 0x%p, Next 0x%p\n", i, pNode, pNext);
                if (pNext && !MmIsAddressValid(pNext)) break;
                pNode = pNext;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DbgPrint("[EnumHotkey] 读取字段异常，跳过当前节点\n");
                break;
            }
        }
    }

    *ppHotkeyList = pHotkeyArray;
    *pulHotkeyCount = index;
    DbgPrint("[EnumHotkey] 枚举完成，实际捕获热键数: %lu\n", index);
    return STATUS_SUCCESS;
}