// OffsetScanner.h
#pragma once
#include <global.h>

// ==========================================
// 版本化偏移结构体（不用 union，用实例区分）
// ==========================================
typedef struct _WIN32K_OFFSETS {
    ULONG OsBuildNumber;            // 如 22621(Win11 22H2), 19045(Win10 22H2)

    // tagTHREADINFO 偏移
    ULONG Pti_pDeskInfo;            // pti->pDeskInfo (全局钩子容器)
    ULONG Pti_aphkStart;            // pti->aphkStart[idHook] 数组基址
    //ULONG Pti_fsHooks;              // pti->fsHooks (位掩码)
    //ULONG Pti_ppi;                  // pti->ppi (PROCESSINFO*)
    //ULONG Pti_pEThread;             // pti->pEThread (ETHREAD*)
    ULONG Pti_amdesk;               // pti->amdesk (桌面访问掩码)

    // tagHOOK 偏移
    ULONG Hook_hHook;               // +0x00 句柄（通常直接就是对象地址）
    ULONG Hook_pti;                 // +0x10 指向 ETHREAD 的二级指针
    ULONG Hook_phkNext;             // +0x28 单向链表 next
    ULONG Hook_nHookType;           // +0x30 钩子类型 (WH_*)
    ULONG Hook_offPfn;              // +0x38 钩子过程偏移 (pfn - hmod)
    ULONG Hook_flags;               // +0x40 标志 (HF_GLOBAL=1, HF_ANSI=2)
    ULONG Hook_ihmod;               // +0x44 模块表索引
    //ULONG Hook_ptiHooked;           // +0x48 目标线程 (NULL=全局)
    //ULONG Hook_rpdesk;              // 【新增】Win10 中 0x50

    // DESKTOPINFO 偏移
    ULONG DeskInfo_aphkStart;       // pDeskInfo->aphkStart[0] (全局钩子数组)
    ULONG DeskInfo_spwnd;           // pDeskInfo->spwnd (桌面窗口)

    // PROCESSINFO 偏移
    //ULONG Ppi_uiPI;                 // ppi->uiPI (完整性级别)
    //ULONG Ppi_MandatoryLabel;       // ppi->MandatoryLabel

    // TEB 偏移
    //ULONG Teb_Win32ThreadInfo;      // TEB->Win32ThreadInfo (fsHooks 拷贝)

    // 会话状态 (xxxCallHook 中 W32GetUserSessionState 返回 (Win11 专用) )
    ULONG SessionState_HookArray;   // +0xA510 钩子数组指针
    ULONG SessionState_Flag;        // +0x4CF8 某个标志

    // === 【新增】HOOK 对象元数据 ===
    ULONG Hook_ObjectSize;          // HMAllocObject 分配大小 (默认 0x60)
    ULONG HandleEntrySize;        // 元数据条目大小 (24 或 32)
    ULONG HandleEntry_HookTypeOffset;      // 元数据中 HookType 字段偏移
    ULONG HandleEntry_TableIndexOffset;    // 元数据中 TableIndex 字段偏移

    // === 【新增】路径标记 ===
    BOOLEAN bIsWin10Path;           // TRUE = Win10 枚举路径
    BOOLEAN bHasW32GetUserSessionState; // 缓存 DetectSessionStateMethod 结果

} WIN32K_OFFSETS, * PWIN32K_OFFSETS;

#define WIN10_1703_BUILD_NUMBER 19041

// 初始化函数：从已解析的符号地址中提取所有偏移
NTSTATUS Win32kOffsetScanner_Initialize(
    _In_opt_ PVOID zzzSetWindowHookExAddr,
    _In_opt_ PVOID xxxCallHookAddr,
    _In_opt_ PVOID HMAllocObjectAddr,
	_In_opt_ PVOID zzzUnhookWindowsHookExAddr,
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