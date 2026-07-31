// OffsetScanner.h
#pragma once
#include <global.h>

typedef struct _WIN32K_FUNCADDR {
    // 消息钩子枚举相关
    PVOID zzzSetWindowHookExAddr;
    PVOID xxxCallHookAddr;
    PVOID HMAllocObjectAddr;
    PVOID zzzUnhookWindowsHookExAddr;

    // 事件钩子枚举相关
    PVOID _SetWinEventHookAddr;
    PVOID xxxWindowEventAddr;

    // 【新增】热键枚举扫描目标
    PVOID _RegisterHotKeyAddr;          // 主扫描目标
    PVOID HKInsertHashElementAddr;      // 24H2+ Session 哈希表扫描目标
}WIN32K_FUNCADDR, * PWIN32K_FUNCADDR;

// ==========================================
// 版本化偏移结构体（不用 union，用实例区分）
// ==========================================
typedef struct _WIN32K_OFFSETS {
    ULONG OsBuildNumber;            // 如 22621(Win11 22H2), 19045(Win10 22H2)
    ULONG DetectedEra;
    BOOLEAN bVerificationFailed;
    BOOLEAN bEventHookVerificationFailed;  // 事件钩子验证失败标记

    // tagTHREADINFO 偏移
    ULONG Pti_pDeskInfo;            // pti->pDeskInfo (全局钩子容器)
    ULONG Pti_aphkStart;            // pti->aphkStart[idHook] 数组基址
    ULONG Pti_amdesk;               // pti->amdesk (桌面访问掩码)
    ULONG Pti_pEThread;             // pti->pEThread (ETHREAD*)
    ULONG Pti_Flags;                // pti->Flags (线程标志)
    ULONG Pti_bitmask;              // pti->bitmask (线程位掩码)
    ULONG Pti_AccessCheck;          // pti->AccessCheck (访问检查函数指针)

    // tagHOOK 偏移
    ULONG Hook_hHook;               // +0x00 句柄
    ULONG Hook_pti;                 // +0x10 指向 ETHREAD 的二级指针
    ULONG Hook_phkNext;             // +0x28 单向链表 next
    ULONG Hook_nHookType;           // +0x30 钩子类型 (WH_*)
    ULONG Hook_offPfn;              // +0x38 钩子过程偏移 (pfn - hmod)
    ULONG Hook_flags;               // +0x40 标志 (HF_GLOBAL=1, HF_ANSI=2)
    ULONG Hook_ihmod;               // +0x44 模块表索引
    ULONG Hook_ObjectSize;          // HMAllocObject 分配大小 (默认 0x60)

    // DESKTOPINFO 偏移
    ULONG DeskInfo_aphkStart;       // pDeskInfo->aphkStart[0] (全局钩子数组)
    ULONG DeskInfo_spwnd;           // pDeskInfo->spwnd (桌面窗口)

    // 会话状态 (xxxCallHook 中 W32GetUserSessionState 返回)
    ULONG SessionState_HookArray;   // +0xA510 钩子数组指针
    ULONG SessionState_Flag;        // +0x4CF8 某个标志

    // HOOK 对象元数据
    ULONG HandleEntrySize;          // 元数据条目大小 (24 或 32)
    ULONG HandleEntry_HookTypeOffset;      // 元数据中 HookType 字段偏移
    ULONG HandleEntry_TableIndexOffset;    // 元数据中 TableIndex 字段偏移

    // 路径标记
    BOOLEAN bIsWin10Path;
    BOOLEAN bHasW32GetUserSessionState;

    // ==========================================
    // 【新增】EVENTHOOK 字段族（WinEvent 钩子）
    // ==========================================
    ULONG EventHook_pNext;              // +0x18, gpWinEventHooks 链表
    ULONG EventHook_eventMin;           // +0x20
    ULONG EventHook_eventMax;           // +0x24
    ULONG EventHook_flags;              // +0x28
    ULONG EventHook_idProcess;          // +0x30, 目标进程ID
    ULONG EventHook_idThread;           // +0x34 (Win10/21H2) / +0x38 (Win11 24H2)
    ULONG EventHook_pfn;                // +0x40, INCONTEXT=绝对地址, OUTOFCONTEXT=offPfn
    ULONG EventHook_ihmod;              // +0x48, 模块表索引
    ULONG EventHook_dpiAwareness;       // +0x58 (Win10) / +0x4C (Win11)
    ULONG EventHook_pti;                // +0x10, THREADINFO*
    ULONG EventHook_ObjectSize;         // 0x60 (Win10) / 0x50 (Win11)
    ULONG EventHook_pti_ppi;            // pti->ppi 偏移: Win10=0x1C0, Win11_24H2=0x1F0
    ULONG GPSI_EVENT_HOOK_LIST_OFFSET;  // Win10=N/A, Win11_24H2=0x11380
    BOOLEAN EventHook_gpWinEventHooks_InSession; // TRUE=Session结构体内部, FALSE=独立全局变量

    // ==========================================
    // 【新增】热键枚举偏移
    // ==========================================
    ULONG Hotkey_pThreadInfo;           // +0x00
    ULONG Hotkey_pfnCallback;           // +0x08
    ULONG Hotkey_pWnd;                  // +0x10
    //ULONG Hotkey_hWnd;                  // +0x18 (24H2+)
    ULONG Hotkey_fsModLow;
    ULONG Hotkey_fsModHigh;
    ULONG Hotkey_id;                    // +0x1C (A~D) / +0x24 (E, 32位)
    ULONG Hotkey_vk;                    // +0x20 (A~D) / +0x28 (E)
    ULONG Hotkey_pNext;                 // +0x28 (A~D) / +0x30 (E)
    ULONG Hotkey_ListEntry;             // 0 (A) / +0x30 (B~D) / +0x38 (E)

    // 对象元数据
    ULONG Hotkey_ObjectSize;            // 0x30 (A) / 0x40 (B~D) / 0x48 (E)
    ULONG Hotkey_HashBuckets;           // 128 (0x80)

    // 哈希表位置
    BOOLEAN Hotkey_bSessionHashTable;   // FALSE (A~D) / TRUE (E)
    ULONG_PTR Hotkey_gpSessionHashTable;// 24H2+ 运行时解析
    ULONG Hotkey_SessionHashOffset;     // 0 (A~D) / 0x3298 (E)
    ULONG_PTR Hotkey_gphkHashTable;     // 全局哈希表地址（1607~23H2）

    // 验证状态
    BOOLEAN bHotkeyVerificationFailed;

} WIN32K_OFFSETS, * PWIN32K_OFFSETS;

// ==========================================
// 构建号阈值
// ==========================================
#define WIN10_1703_BUILD_NUMBER   16299
#define WIN10_20H2_BUILD_NUMBER   19041
#define WIN11_21H2_BUILD_NUMBER   22000
#define WIN11_24H2_BUILD_NUMBER   26100

// 初始化函数：从已解析的符号地址中提取所有偏移
NTSTATUS Win32kOffsetScanner_Initialize(
    _In_ PWIN32K_FUNCADDR FuncAddr,
    _Out_ PWIN32K_OFFSETS Offsets
);

// 验证提取的偏移是否合理
BOOLEAN Win32kOffsetScanner_Validate(
    _In_ PWIN32K_OFFSETS Offsets
);

// 打印调试信息
BOOLEAN Win32kOffsetScanner_Dump(
    _In_ PWIN32K_OFFSETS Offsets,
    _In_ BOOLEAN bCompareWithReference
);