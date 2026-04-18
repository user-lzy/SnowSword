#include "Window.h"
#include "OtherFunctions.h"
#include "Module.h"
#include "ntstrsafe.h"

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

// ====================== 【终极修复】复刻旧版正确解析逻辑 ======================
NTSTATUS EnumProcessTimers(
    _In_ HANDLE ProcessId,
    _Out_ PWINDOW_TIMER* pArray,
    _Out_ PULONG pCount
) {
    NTSTATUS status = STATUS_SUCCESS;
    PVOID hashBase = NULL;
    ULONG timerCount = 0;
    PWINDOW_TIMER pResultArray = NULL;

    if (!pArray || !pCount || !ProcessId) {
        DbgPrint("[TIMER_ENUM] 输入参数无效！\n");
        return STATUS_INVALID_PARAMETER;
    }
    *pArray = NULL;
    *pCount = 0;

    DbgPrint("[TIMER_ENUM] === 开始枚举进程定时器，目标PID: %llu ===", (ULONG64)ProcessId);

    status = InitializeTimerContext();
    if (!NT_SUCCESS(status)) {
        DbgPrint("[TIMER_ENUM] 初始化上下文失败！0x%X\n", status);
        return status;
    }

    __try {
        g_TimerCtx.EnterCrit(0, 0);
        DbgPrint("[TIMER_ENUM] 已进入临界区\n");

        __try {
            // 哈希表基址（和旧版完全一致）
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

            // 遍历桶（和旧版一致）
            for (UCHAR bucket = 0; bucket < 64; bucket++) {
                PLIST_ENTRY* bucketPtr = (PLIST_ENTRY*)((PUCHAR)hashBase + 16 * bucket);
                if (!MmIsAddressValid(bucketPtr)) continue;
                PLIST_ENTRY head = *bucketPtr;
                if (!head || !MmIsAddressValid(head)) continue;

                // 遍历链表（和旧版一致）
                for (PLIST_ENTRY entry = head; entry != (PLIST_ENTRY)bucketPtr; entry = entry->Flink) {
                    if (!MmIsAddressValid(entry) || !MmIsAddressValid(entry->Flink)) break;

                    PTIMER_ENTRY pTimer = CONTAINING_RECORD(entry, TIMER_ENTRY, HashListEntry);
                    if (!MmIsAddressValid(pTimer) || (pTimer->flags & 0x1000)) continue;

                    // ====================== 【修复】1:1 复刻旧版正确的 PID 解析逻辑！======================
                    PVOID pti = pTimer->head.threadInfo;  // 旧版原版：直接取 threadInfo +0x18
                    if (!pti) continue;

                    // Win10/11 偏移（旧版原版逻辑）
                    ULONG processOffset = g_TimerCtx.isV2 ? 0x1D0 : 0x1A8;
                    PVOID ProcessInfo = *(PVOID*)((PUCHAR)pti + processOffset); // 第一层解引用
                    PEPROCESS pEprocess = *(PEPROCESS*)ProcessInfo;            // 第二层解引用（新版之前漏了这行！）
                    HANDLE currentPid = PsGetProcessId(pEprocess);

                    // 线程ID（旧版原版逻辑）
                    PETHREAD pEthread = *(PETHREAD*)((PUCHAR)pti + 0x0);
                    HANDLE dwThreadId = PsGetThreadId(pEthread);

                    DbgPrint("[TIMER_ENUM] 找到定时器！PID=%llu, TID=%llu, nIDEvent=%lld\n", (ULONG64)currentPid, (ULONG64)dwThreadId, pTimer->nIDEvent);

                    // PID匹配过滤
                    if (currentPid != ProcessId) {
                        continue;
                    }

                    // ====================== 匹配成功，分配内存 ======================
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

                    // 填充数据
                    pNewArray[timerCount].pfn = pTimer->pfn;
                    pNewArray[timerCount].nTimeout = pTimer->nTimeout;
                    pNewArray[timerCount].nIDEvent = pTimer->nIDEvent;
                    pNewArray[timerCount].ThreadId = dwThreadId;
                    //pNewArray[timerCount].param = 0;

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
    // ====================== 修改点1：替换基址函数 ======================
    // 原错误：NtUserUnhookWindowsHookEx
    // 新正确：NtUserSetWindowsHookEx（你的汇编明确使用这个函数）
    PVOID NtUserSetWindowsHookExAddr = KernelGetProcAddress("win32kfull.sys", "NtUserSetWindowsHookEx");
    if (!NtUserSetWindowsHookExAddr) {
        DbgPrint("NtUserSetWindowsHookEx 未找到！\n");
        return NULL;
    }

    // ====================== 修改点2：特征码1 → 匹配 call zzzSetWindowsHookEx ======================
    // 汇编：call zzzSetWindowsHookEx + 后续 test rax, rax
    // 机器码：E8 xx xx xx xx 48 85 C0
    UCHAR pattern1[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0 };
    UCHAR mask1[] = { 1,    0,    0,    0,    0,    1,    1,    1 };
    PVOID result = SearchSpecialCodeWithMask(NtUserSetWindowsHookExAddr, 0x400, pattern1, mask1, sizeof(pattern1));
    if (!result) {
        DbgPrint("特征码1(call zzzSetWindowsHookEx) 未找到！\n");
        return NULL;
    }

    // 解析call指令 → 获取 zzzSetWindowsHookEx 函数地址
    ULONG call_offset = *(ULONG*)((PUCHAR)result + 1);
    PVOID zzzSetWindowsHookExAddr = (PVOID)((PUCHAR)result + 5 + call_offset);
    DbgPrint("zzzSetWindowsHookEx: 0x%p\n", zzzSetWindowsHookExAddr);

    // ====================== 保留：特征码2 → 匹配 call GetHmodTableIndex ======================
    // 你的汇编：call GetHmodTableIndex + mov [rdi+44h], eax
    // 机器码：E8 xx xx xx xx 89 47 44
    UCHAR pattern2[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x89, 0x47, 0x44 };
    UCHAR mask2[] = { 1,    0,    0,    0,    0,    1,    1,    1 };
	// 之前是0x2000，现在改回0x200，足够找到目标且更快
    result = SearchSpecialCodeWithMask(zzzSetWindowsHookExAddr, 0x200, pattern2, mask2, sizeof(pattern2));
    if (!result) {
        DbgPrint("特征码2(call GetHmodTableIndex) 未找到！\n");
        return NULL;
    }

    // 解析call指令 → 获取 GetHmodTableIndex 函数地址
    call_offset = *(ULONG*)((PUCHAR)result + 1);
    PVOID GetHmodTableIndexAddr = (PVOID)((PUCHAR)result + 5 + call_offset);
    DbgPrint("GetHmodTableIndex: 0x%p\n", GetHmodTableIndexAddr);
    return GetHmodTableIndexAddr;
}

PVOID FindaatomSysLoaded() {
    // ====================== 保留：特征码3 → 匹配 lea r8, aatomSysLoaded ======================
    // 你的汇编：lea r8, ?aatomSysLoaded@@3PAGA + test eax,eax
    UCHAR pattern3[] = {
        0x4C, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00,  // lea r8, [rip+offset]
        0x85, 0xC0                                  // test eax, eax
    };
    UCHAR mask3[] = {
        1, 1, 1, 0, 0, 0, 0,
        1, 1
    };
    PVOID result = SearchSpecialCodeWithMask(FindGetHmodTableIndex(), 0x200, pattern3, mask3, sizeof(pattern3));
    if (!result) {
        DbgPrint("特征码3(lea aatomSysLoaded) 未找到！\n");
        return NULL;
    }

    // 解析lea指令 → 最终目标：aatomSysLoaded
    ULONG lea_offset = *(ULONG*)((PUCHAR)result + 3);
    PVOID aatomSysLoadedAddr = (PVOID)((PUCHAR)result + 7 + lea_offset);

    DbgPrint("成功找到 aatomSysLoaded: 0x%p\n", aatomSysLoadedAddr);
    return aatomSysLoadedAddr;
}

// 从模块索引获取基址
NTSTATUS GetModuleNameFromihMod(BOOLEAN bWin11, int ihmod, LPWSTR NameBuffer)
{
#define MAX_PATH 260
    // 在win11上变为*(_WORD *)(W32GetUserSessionState() + 0xA17C)
    static ATOM* aatomSysLoaded = NULL;
    static PFN_W32GetUserSessionState W32GetUserSessionState = NULL;
    if (!aatomSysLoaded || !MmIsAddressValid(aatomSysLoaded)) {
        if (bWin11) {
            if (!W32GetUserSessionState || !MmIsAddressValid((PVOID)W32GetUserSessionState)) {
                W32GetUserSessionState = (PFN_W32GetUserSessionState)KernelGetProcAddress("win32k.sys", "W32GetUserSessionState");
                if (!W32GetUserSessionState || !MmIsAddressValid((PVOID)W32GetUserSessionState)) {
                    DbgPrint("W32GetUserSessionState not found!\n");
                    return STATUS_INVALID_ADDRESS;
                }
            }
            aatomSysLoaded = (ATOM*)(((PUCHAR)W32GetUserSessionState()) + 0xA17C);
        }
        else {
            aatomSysLoaded = (ATOM*)FindaatomSysLoaded();
        }
        if (!aatomSysLoaded || !MmIsAddressValid(aatomSysLoaded)) return STATUS_INVALID_ADDRESS;
    }
    static PVOID UserLibmgmtAtomTableHandle = NULL;
    if (!UserLibmgmtAtomTableHandle || !MmIsAddressValid(UserLibmgmtAtomTableHandle)) {
        UserLibmgmtAtomTableHandle = (PVOID)KernelGetProcAddress("win32kbase.sys", "UserLibmgmtAtomTableHandle");
        if (!UserLibmgmtAtomTableHandle || !MmIsAddressValid(UserLibmgmtAtomTableHandle)) {
            UserLibmgmtAtomTableHandle = (PVOID)(((PUCHAR)W32GetUserSessionState()) + 0xA170);
            if (!UserLibmgmtAtomTableHandle || !MmIsAddressValid(UserLibmgmtAtomTableHandle)) return STATUS_INVALID_ADDRESS;
        }
    }
    static PFN_RtlQueryAtomInAtomTable RtlQueryAtomInAtomTable = NULL;
    if (!RtlQueryAtomInAtomTable || !MmIsAddressValid((PVOID)RtlQueryAtomInAtomTable)) {
        RtlQueryAtomInAtomTable = (PFN_RtlQueryAtomInAtomTable)KernelGetProcAddress("ntoskrnl.exe", "RtlQueryAtomInAtomTable");
        if (!RtlQueryAtomInAtomTable || !MmIsAddressValid((PVOID)RtlQueryAtomInAtomTable)) return STATUS_INVALID_ADDRESS;
    }
    ULONG MaxLength = sizeof(WCHAR) * MAX_PATH;
    //DbgPrint("aatomSysLoaded[%d]:%d", ihmod, aatomSysLoaded[ihmod]);
    return RtlQueryAtomInAtomTable(*(PVOID*)UserLibmgmtAtomTableHandle, aatomSysLoaded[ihmod], 0, 0, NameBuffer, &MaxLength);
}

PVOID FindHMValidateHandle() {
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

BOOLEAN IsValidHook(PVOID pHookBase)
{
    if (pHookBase == NULL)
        return FALSE;

    __try {
        // 1. 校验钩子类型范围 (0~15)
        if (!MmIsAddressValid((PUCHAR)pHookBase + WIN11_HOOK_OFFSET_TYPE))
            return FALSE;
        LONG hookType = *(PLONG)((PUCHAR)pHookBase + WIN11_HOOK_OFFSET_TYPE);
        if (hookType < 0 || hookType >= MAX_HOOK_TYPES)
            return FALSE;

        // 2. HookProc 非空即可（允许用户态地址）
        if (!MmIsAddressValid((PUCHAR)pHookBase + WIN11_HOOK_OFFSET_HOOKPROC))
            return FALSE;
        ULONG64 hookProc = *(PULONG64)((PUCHAR)pHookBase + WIN11_HOOK_OFFSET_HOOKPROC);
        if (hookProc == 0)
            return FALSE;

        // 3. 【修正】仅校验 next 指针域本身可读取，不校验其指向的地址
        //    (因为最后一个钩子的 next 是 NULL，这是合法的)
        if (!MmIsAddressValid((PUCHAR)pHookBase + WIN11_HOOK_OFFSET_NEXT_HOOK))
            return FALSE;

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
NTSTATUS GetHookOwnerId(PVOID pTagHook, OUT PULONG pPid, OUT PULONG pTid)
{
    if (!pTagHook || !pPid || !pTid)
        return STATUS_INVALID_PARAMETER;

    *pPid = 0;
    *pTid = 0;

    __try {
        // 1. 读取二级指针：tagHOOK+0x10 -> PETHREAD*
        PVOID pEthreadPtr = *(PVOID*)((PUCHAR)pTagHook + WIN11_HOOK_OFFSET_PETHREAD_PTR);
        if (!pEthreadPtr || !MmIsAddressValid(pEthreadPtr))
            return STATUS_INVALID_ADDRESS;

        // 2. 解引用拿到真正的ETHREAD地址
        PETHREAD pEthread = *(PETHREAD*)pEthreadPtr;
        if (!pEthread || !MmIsAddressValid(pEthread))
            return STATUS_INVALID_ADDRESS;

        // 3. 调用内核导出函数获取TID/PID（ntoskrnl.exe直接导出，无需手动解析）
        *pTid = (ULONG)(ULONG_PTR)PsGetThreadId(pEthread);
        *pPid = (ULONG)(ULONG_PTR)PsGetThreadProcessId(pEthread);

        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_UNSUCCESSFUL;
    }
}

// ==========================================
// 【完整修正版】EnumerateHooksFromPti
// ==========================================
NTSTATUS EnumerateHooksFromPti(
    PVOID pContext,
    BOOLEAN IsGlobal,
    HANDLE hPid,
    HANDLE hTid,
    PWIN32K_MSG_HOOK_INFO pHookArray,
    PULONG puFoundHookCount,
    ULONG uMaxCount
)
{
    if (!pContext || !pHookArray || !puFoundHookCount || uMaxCount == 0)
        return STATUS_INVALID_PARAMETER;

    // 全局钩子：PLIST_ENTRY 数组（每个元素是链表头指针）
    // 线程钩子：tagHOOK* 数组（每个元素是直接指针，可能为NULL）

    if (IsGlobal) {
        // ==========================================
        // 全局钩子枚举（链表结构）
        // ==========================================
        PVOID* pHookHeadArray = (PVOID*)((PUCHAR)pContext + GLOBAL_HOOK_ARRAY_BASE);

        DbgPrint("[调试] 全局钩子容器: 0x%p, 数组基址: 0x%p\n", pContext, pHookHeadArray);

        for (ULONG hookTypeIdx = 0; hookTypeIdx < MAX_HOOK_TYPES; hookTypeIdx++)
        {
            __try {
                PVOID pCurrentHook = *(PVOID*)&pHookHeadArray[hookTypeIdx];
                ULONG dwDepth = 0;
                const ULONG MAX_HOOK_DEPTH = 256;

                while (pCurrentHook != NULL && dwDepth < MAX_HOOK_DEPTH)
                {
                    dwDepth++;

                    if (!MmIsAddressValid(pCurrentHook)) {
                        DbgPrint("[!] 全局类型[%2d] tagHOOK无效: 0x%p\n", hookTypeIdx, pCurrentHook);
                        break;
                    }

                    // 验证钩子有效性
                    if (!IsValidHook(pCurrentHook)) {
                        pCurrentHook = *(PVOID*)((PUCHAR)pCurrentHook + WIN11_HOOK_OFFSET_NEXT_HOOK);
                        continue;
                    }

#if HOOK_DEBUG_PRINT_MEMORY
                    DbgPrint("\n[调试] 全局合法钩子，类型[%2d] 深度[%d] tagHOOK: 0x%p\n",
                        hookTypeIdx, dwDepth, pCurrentHook);
                    DebugPrintHookMemory(pCurrentHook, 0x80);
#endif

                    if (*puFoundHookCount >= uMaxCount)
                        break;

                    PWIN32K_MSG_HOOK_INFO pEntry = &pHookArray[*puFoundHookCount];
                    PUCHAR pHookBase = (PUCHAR)pCurrentHook;

                    __try {
                        RtlZeroMemory(pEntry, sizeof(WIN32K_MSG_HOOK_INFO));

                        // 读取钩子成员
                        pEntry->HookType = *(PLONG)(pHookBase + WIN11_HOOK_OFFSET_TYPE);
                        pEntry->HookProc = *(PULONG64)(pHookBase + WIN11_HOOK_OFFSET_HOOKPROC);
                        pEntry->HookFlags = *(PULONG)(pHookBase + WIN11_HOOK_OFFSET_FLAGS);
                        ULONG ihmod = *(PULONG)(pHookBase + WIN11_HOOK_OFFSET_IHMOD);

                        RtlZeroMemory(pEntry->ModulePath, sizeof(pEntry->ModulePath));

                        // ihmod: 0xFFFFFFFF 为系统钩子
                        if (ihmod != 0xFFFFFFFF && ihmod < 0x1000) {
                            GetModuleNameFromihMod(TRUE, (INT)ihmod, pEntry->ModulePath);
                        }

                        pEntry->IsGlobal = TRUE;
                        GetHookOwnerId(pCurrentHook, &pEntry->ProcessId, &pEntry->ThreadId);
                        pEntry->HookHandle = *(HANDLE*)(pHookBase + WIN11_HOOK_OFFSET_HHOOK);

                        DbgPrint("[Global] Type: %s (%d), Flags: 0x%08X, HookProc: 0x%p, hHook=0x%p, ihmod=%d, Module: %ws\n",
                            GetHookTypeName(pEntry->HookType),
                            pEntry->HookType,
                            pEntry->HookFlags,
                            (PVOID)pEntry->HookProc,
                            pEntry->HookHandle,
                            ihmod,
                            pEntry->ModulePath[0] ? pEntry->ModulePath : L"(System/LL Hook)");

                        (*puFoundHookCount)++;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        DbgPrint("[!] 读取全局钩子成员异常\n");
                    }

                    // 取下一个钩子
                    pCurrentHook = *(PVOID*)((PUCHAR)pCurrentHook + WIN11_HOOK_OFFSET_NEXT_HOOK);
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
        // 【修正】线程钩子枚举（直接指针数组，非链表）
        // ==========================================
        PVOID* pHookPtrArray = (PVOID*)((PUCHAR)pContext + WIN11_TI_THREAD_HOOK_ARRAY);

        //DbgPrint("[调试] 线程PTI: 0x%p, 数组基址: 0x%p\n", pContext, pHookPtrArray);

        for (ULONG hookTypeIdx = 0; hookTypeIdx < MAX_HOOK_TYPES; hookTypeIdx++)
        {
            __try {
                // 直接取tagHOOK*指针
                PVOID pHook = pHookPtrArray[hookTypeIdx];

                // 调试：打印所有非空指针
                if (pHook != NULL) {
                    DbgPrint("[调试] 线程类型[%2d] 原始指针: 0x%p\n", hookTypeIdx, pHook);
                }

                if (!pHook || !MmIsAddressValid(pHook)) {
                    continue;
                }

                // 验证钩子有效性
                if (!IsValidHook(pHook)) {
                    continue;
                }
#if HOOK_DEBUG_PRINT_MEMORY
                DbgPrint("\n[调试] 线程直接钩子，类型[%2d] tagHOOK: 0x%p\n", hookTypeIdx, pHook);
                DebugPrintHookMemory(pHook, 0x80);
#endif

                if (*puFoundHookCount >= uMaxCount)
                    continue;

                PWIN32K_MSG_HOOK_INFO pEntry = &pHookArray[*puFoundHookCount];
                PUCHAR pHookBase = (PUCHAR)pHook;

                __try {
                    RtlZeroMemory(pEntry, sizeof(WIN32K_MSG_HOOK_INFO));

                    pEntry->HookType = *(PLONG)(pHookBase + WIN11_HOOK_OFFSET_TYPE);
                    pEntry->HookProc = *(PULONG64)(pHookBase + WIN11_HOOK_OFFSET_HOOKPROC);
                    pEntry->HookFlags = *(PULONG)(pHookBase + WIN11_HOOK_OFFSET_FLAGS);
                    ULONG ihmod = *(PULONG)(pHookBase + WIN11_HOOK_OFFSET_IHMOD);

                    RtlZeroMemory(pEntry->ModulePath, sizeof(pEntry->ModulePath));

                    if (ihmod != 0xFFFFFFFF && ihmod < 0x1000) {
                        GetModuleNameFromihMod(TRUE, (INT)ihmod, pEntry->ModulePath);
                    }

                    pEntry->IsGlobal = FALSE;
                    pEntry->ProcessId = (ULONG)(ULONG_PTR)hPid;
                    pEntry->ThreadId = (ULONG)(ULONG_PTR)hTid;
                    pEntry->HookHandle = *(HANDLE*)(pHookBase + WIN11_HOOK_OFFSET_HHOOK);

                    DbgPrint("[Thread] Type: %s (%d), Flags: 0x%08X, HookProc: 0x%p, hHook=0x%p, ihmod=%d, Module: %ws\n",
                        GetHookTypeName(pEntry->HookType),
                        pEntry->HookType,
                        pEntry->HookFlags,
                        (PVOID)pEntry->HookProc,
						pEntry->HookHandle,
                        ihmod,
                        pEntry->ModulePath[0] ? pEntry->ModulePath : L"(System/LL Hook)");

                    (*puFoundHookCount)++;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DbgPrint("[!] 读取线程直接钩子成员异常\n");
                }
                
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                DbgPrint("[!] 线程类型[%2d] 访问异常\n", hookTypeIdx);
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
#if HOOK_DEBUG_PRINT_MEMORY
        DbgPrint("\n[调试] tagTHREADINFO 基地址: 0x%p\n", pCurrentPti);
        DebugPrintHookMemory(pCurrentPti, 0x500);
#endif

        __try {
            // 读取全局钩子容器指针（pti + 0x1F8）
            PVOID pGlobalHookContainer = *(PVOID*)((PUCHAR)pCurrentPti + WIN11_TI_GLOBAL_HOOK_CONTAINER);
            if (pGlobalHookContainer)
            {
#if HOOK_DEBUG_PRINT_MEMORY
                DbgPrint("\n[调试] pGlobalHookContainer 基地址: 0x%p\n", pGlobalHookContainer);
                DebugPrintHookMemory(pGlobalHookContainer, 0x100);
#endif

                EnumerateHooksFromPti(
                    pGlobalHookContainer,    // 传入容器，函数内部会 +0x28
                    TRUE,
                    NULL, NULL,
                    pHookArray,
                    &uFoundHookCount,
                    uAllocatedCount
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
                    // 【关键修正】从 W32THREAD 头部解引用获取 tagTHREADINFO*
                    PVOID pTargetPti = *(PVOID*)pW32Thread;
                    EnumerateHooksFromPti(
                        pTargetPti,      // 传入 pti，函数内部会 +0x3C0
                        FALSE,
                        hPid, hTid,
                        pHookArray,
                        &uFoundHookCount,
                        uAllocatedCount
                    );
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
    OUT PWIN32K_MSG_HOOK_INFO* ppHookList,
    OUT PULONG                   pulHookCount
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PWIN32K_MSG_HOOK_INFO pHookArray = NULL;
    ULONG uFoundHookCount = 0;
    ULONG uHookCount = 0;

    // 初始化输出
    *ppHookList = NULL;
    *pulHookCount = 0;

    // ====================== 原有逻辑不变 ======================
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

    RTL_OSVERSIONINFOW Version = { sizeof(Version) };
    if (!NT_SUCCESS(RtlGetVersion(&Version))) {
        DbgPrint("获取系统版本失败!\n");
        return status;
    }
    BOOLEAN bIsOldSystem = (Version.dwMajorVersion < 0xA || Version.dwBuildNumber < WIN10_1703_BUILD_NUMBER);

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

    DbgPrint("开始遍历数组，钩子数量=%d\n", uHookCount);

    // ====================== 遍历 + 填充结构体 ======================
    for (ULONG i = 0; i < uHookCount; i++)
    {
        PUCHAR pMetaEntry = NULL;
        UCHAR HookType = 0;
        USHORT TableIndex = 0;

        if (bIsOldSystem) {
            pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * 24);
            HookType = *(PUCHAR)(pMetaEntry + 16);
            TableIndex = *(PUSHORT)(pMetaEntry + 18);
        }
        else {
            pMetaEntry = (PUCHAR)pSharedInfo->pHookMetadata + (i * 32);
            HookType = *(PUCHAR)(pMetaEntry + 24);
            TableIndex = *(PUSHORT)(pMetaEntry + 26);
        }

        if (HookType != TYPE_HOOK) continue;

        // 绑定当前钩子结构体
        PWIN32K_MSG_HOOK_INFO pCurHook = &pHookArray[uFoundHookCount++];
        pCurHook->HookType = HookType; // 补全字段赋值

        // 构造句柄
        HANDLE hHookHandle = (HANDLE)(i | ((ULONG)TableIndex << 16));
        pCurHook->HookHandle = hHookHandle;

        DbgPrint("找到消息钩子，索引=%d，TableIndex=%d\n", i, TableIndex);

        PVOID pHookObject = HMValidateHandle(hHookHandle, TYPE_HOOK);
        if (!pHookObject) {
            DbgPrint("HMValidateHandle 验证失败，句柄=0x%p\n", hHookHandle);
            continue;
        }
        if (!MmIsAddressValid(pHookObject) || !MmIsAddressValid((PUCHAR)pHookObject + 0x80)) {
            DbgPrint("钩子对象内存无效，地址=0x%p\n", pHookObject);
            continue;
        }
        // 打印钩子对象内存布局（保留调试用）
        for (int j = 0; j < 128; j += 8) {
            if (MmIsAddressValid((PUCHAR)pHookObject + j)) {
                DbgPrint("  offset %02X: %016llX\n", j, *(PULONG64)((PUCHAR)pHookObject + j));
            }
        }

        // ====================== 偏移读取（原有逻辑） ======================
        ULONG offsetFlags, offsetType, offsetHookProc;
        if (bIsOldSystem) {
            offsetFlags = HOOK_OFFSET_FLAGS_OLD;
            offsetType = HOOK_OFFSET_TYPE_OLD;
            offsetHookProc = HOOK_OFFSET_HOOKPROC_OLD;
        }
        else {
            offsetFlags = HOOK_OFFSET_FLAGS_NEW;
            offsetType = HOOK_OFFSET_TYPE_NEW;
            offsetHookProc = HOOK_OFFSET_HOOKPROC_NEW;
        }

        // 读取核心数据 → 存入结构体
        pCurHook->HookType = *(PLONG)((PUCHAR)pHookObject + offsetType);
        ULONG64 qwFlags = *(PULONG64)((PUCHAR)pHookObject + offsetFlags);
        pCurHook->HookFlags = (ULONG)(qwFlags & 0xFFFFFFFF); // ✅ 正确：取低32位
        pCurHook->HookProc = *(PULONG64)((PUCHAR)pHookObject + offsetHookProc);
        PVOID hMod = *(PVOID64*)((PUCHAR)pHookObject + offsetHookProc + 0x8);

        // ====================== 补全：全局/低级钩子判断（取消注释） ======================
        BOOLEAN IsLowLevel = (TableIndex == 13 || TableIndex == 14);
        pCurHook->IsGlobal = ((pCurHook->HookFlags & HF_GLOBAL) != 0) || IsLowLevel;

        // PID/TID
        PETHREAD* ppThread = *(PETHREAD**)((PUCHAR)pHookObject + 0x10);
        if (ppThread && *ppThread) {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            pCurHook->ProcessId = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            pCurHook->ThreadId = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
        }

        // ====================== 🔥 核心修复：x64 正确解析 hMod（适配 0x100000000） ======================
        //pCurHook->IsResolved = FALSE;
        RtlZeroMemory(pCurHook->ModulePath, sizeof(pCurHook->ModulePath));
        ULONG64 hModValue = (ULONG64)hMod;

        // 👇 【唯一正确规则】Win32k x64 真ihmod格式：
        // 高32位 = 0~32（有效模块索引） + 低32位 = 0
        ULONG ihmod = (ULONG)((hModValue >> 32) & 0xFFFFFFFF);
        ULONG low32 = (ULONG)(hModValue & 0xFFFFFFFF);

        // ✅ 严格判断：有效模块索引（仅这里需要查原子表）
        if (low32 == 0 && ihmod > 0 && ihmod <= 32)
        {
            DbgPrint("  → 真模块索引 ihmod=%lu\n", ihmod);
            if (NT_SUCCESS(GetModuleNameFromihMod(FALSE, (int)ihmod, pCurHook->ModulePath)))
            {
                DbgPrint("  → 模块名: %ws\n", pCurHook->ModulePath);
            }
        }
        // ❌ 全局/低级钩子（0xFFFFFFFF 或 0xFFFF 开头）→ 不查原子表
        else if ((hModValue & 0xFFFF00000000) == 0xFFFF00000000)
        {
            DbgPrint("  → 全局钩子，无模块索引\n");
        }
        // ❌ 无效值 → 跳过
        else
        {
            DbgPrint("  → 无效hMod值: %llx\n", hModValue);
        }

        // ====================== 原有打印保留 ======================
        DbgPrint("消息钩子：Type=%s(%d), 标志=%d, 句柄=0x%p, 范围=%s, Proc=0x%llX, PID=%d, TID=%d\n",
            GetHookTypeName(pCurHook->HookType),
            pCurHook->HookType,
            pCurHook->HookFlags,
            pCurHook->HookHandle,
            pCurHook->IsGlobal ? "全局钩子" : "线程钩子",
            pCurHook->HookProc,
            pCurHook->ProcessId, pCurHook->ThreadId);
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

        DbgPrint("\n=============================================\n");
        DbgPrint("钩子 [%lu] 地址: 0x%p\n", currentIndex, pCurrentHook);
        DbgPrint("=============================================\n");

        // 打印pCurrentHook附近的内存以验证结构体
        DbgPrint("\n--- 钩子结构体内存转储 ---\n");
        DumpMemoryHex(pCurrentHook, 0x100);

        // 打印各个字段的原始值
        DbgPrint("\n--- 原始字段值解析 ---\n");
        DbgPrint("[0x00] hHandle:             0x%p\n", pCurrentHook->HookHandle);
        DbgPrint("[0x20] EventMin:            0x%X\n", pCurrentHook->EventMin);
        DbgPrint("[0x24] EventMax:            0x%X\n", pCurrentHook->EventMax);
        DbgPrint("[0x28] Flags:               0x%X\n", pCurrentHook->Flags);
        DbgPrint("[0x30] TargetProcessId:     0x%p (%lu)\n", (PVOID)pCurrentHook->TargetProcessId, (ULONG)pCurrentHook->TargetProcessId);
        DbgPrint("[0x38] TargetThreadId:      0x%p (%lu)\n", (PVOID)pCurrentHook->TargetThreadId, (ULONG)pCurrentHook->TargetThreadId);
        DbgPrint("[0x40] pfnWinEventProc:     0x%p\n", pCurrentHook->pfnWinEventProc);
        DbgPrint("[0x48] hmodWinEventProc:    0x%llx\n", pCurrentHook->Unknown48);

        HANDLE hInstallerPid = NULL, hInstallerTid = NULL;

        if (pCurrentHook->pInstallerThread && MmIsAddressValid(pCurrentHook->pInstallerThread))
        {
            // 从 ETHREAD 获取 CID (Client ID)
            // 注意: ETHREAD 的 Cid 偏移在 Win11 上通常是 0x4e8
            // 但为了稳定性，我们使用 PsGetThreadId 和 PsGetThreadProcessId
            hInstallerPid = PsGetThreadProcessId(*pCurrentHook->pInstallerThread);
            hInstallerTid = PsGetThreadId(*pCurrentHook->pInstallerThread);

            DbgPrint("安装者 PETHREAD: 0x%p\n", *pCurrentHook->pInstallerThread);
            DbgPrint("安装者 PID: %d\n", (ULONG)(ULONG_PTR)hInstallerPid);
            DbgPrint("安装者 TID: %d\n", (ULONG)(ULONG_PTR)hInstallerTid);
        }
        else
        {
            DbgPrint("安装者 ETHREAD: 无效或为空\n");
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
                DbgPrint(" -> 模块名: %ws\n", pCurrentInfo->ModulePath);
            }
        }
        // 情况2：跨进程钩子
        else if ((ULONG)(ULONG_PTR)pCurrentInfo->hmodWinEventProc == (ULONG)-1)
        {
            DbgPrint("  跨进程钩子，hmod=0x%p\n", pCurrentInfo->hmodWinEventProc);
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

        DbgPrint("[%d] 找到事件钩子，TableIndex=%d\n", uFoundHookCount, TableIndex);

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

        // 读取进程/线程ID
        ULONG idProcess = 0, idThread = 0;
        PETHREAD* ppThread = *(PETHREAD**)((PUCHAR)pEventHookObj + EVENTHOOK_OFFSET_THREAD);
        if (ppThread && MmIsAddressValid(ppThread) && *ppThread)
        {
            PETHREAD pThread = *ppThread;
            PKPROCESS pProcess = IoThreadToProcess(pThread);
            idProcess = (ULONG)(ULONG_PTR)PsGetProcessId(pProcess);
            idThread = (ULONG)(ULONG_PTR)PsGetThreadId(pThread);
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
                    DbgPrint(" -> 模块名: %ws\n", pCurHook->ModulePath);
                }
            }
        }
        // 情况2：跨进程钩子
        else if ((ULONG_PTR)pCurHook->hmodWinEventProc == (ULONG)-1)
        {
            DbgPrint("  跨进程钩子，hmod=0x%p\n", pCurHook->hmodWinEventProc);
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

            DbgPrint("[Win10][%03d] hWnd=0x%p, vk=0x%02X, mod=0x%04X, id=0x%x, PID=%Iu, TID=%Iu\n",
                index, hWnd, pHotkey->vk, pHotkey->fsModifiers, pHotkey->id,
                (ULONG_PTR)PID, (ULONG_PTR)TID);

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
                DbgPrint("HOTKEY=%p\n", pHotkey);
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
                    *(PVOID*)((PUCHAR)pHotkey + 0x38));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                DbgPrint("[Win11] 热键信息读取异常: %p\n", pHotkey);
            }
            DbgPrint("[Win11][%03d] HOTKEY=%p, vk=0x%02X, mod=0x%04X, id=0x%p, hWnd=%p\n",
                index, pHotkey, vk, mod, (PVOID)id, hWnd);

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
    status = EnumerateMsgHook_Win10(ppMsgHookList, pulMsgHookCount);
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