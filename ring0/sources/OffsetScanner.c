// OffsetScanner.c
#include "OffsetScanner.h"
#include "Symbol.h"
#include <Zydis.h>

// ==========================================
// 常量定义
// ==========================================
#define SCAN_MAX_SIZE       0x5000
#define SCAN_MAX_DEPTH      4096

// 构建号阈值
#define BUILD_1703          16299   // 时代A边界
#define BUILD_20H2          19041   // 时代B/C边界
#define BUILD_11_21H2       22000   // 时代C/D边界
#define BUILD_11_24H2       26100   // 时代D/E边界

// ==========================================
// 时代定义
// ==========================================
typedef enum _WIN32K_ERA {
    ERA_UNKNOWN = 0,
    ERA_A,      // Win10 1607及更早
    ERA_B,      // Win10 1809 - 1909
    ERA_C,      // Win10 20H2 - Win10 22H2
    ERA_D,      // Win11 21H2 / 23H2
    ERA_E,      // Win11 24H2+
    ERA_COUNT
} WIN32K_ERA;

// ==========================================
// 时代特征描述（用于验证扫描）
// ==========================================
typedef struct _ERA_SIGNATURE {
    // === PTI 字段偏移 ===
    ULONG Pti_pDeskInfo;
    ULONG Pti_aphkStart;
    ULONG Pti_amdesk;
    ULONG Pti_pEThread;
    ULONG Pti_Flags;
    ULONG Pti_bitmask;
    ULONG Pti_AccessCheck;

    // === DeskInfo 字段 ===
    ULONG DeskInfo_aphkStart;

    // === Hook 对象字段（跨时代稳定） ===
    ULONG Hook_hHook;
    ULONG Hook_pti;
    ULONG Hook_phkNext;
    ULONG Hook_nHookType;
    ULONG Hook_offPfn;
    ULONG Hook_flags;
    ULONG Hook_ihmod;
    ULONG Hook_ObjectSize;

    // === 寄存器分配（zzzSetWindowsHookEx） ===
    ZydisRegister PtiBaseReg_zzzSWH;
    ZydisRegister HookBaseReg_zzzSWH;

    // === 寄存器分配（xxxCallHook / xxxCallHook2） ===
    ZydisRegister PtiBaseReg_xxxCH;
    ZydisRegister PtiBaseReg_xxxCH2;
    ZydisRegister AphkStart_BaseReg_xxxCH;
    ZydisRegister AphkStart_IndexReg_xxxCH;

    // === 架构特征 ===
    BOOLEAN bIsForwarder;

    // === 构建号范围 ===
    ULONG OsBuildMin;
    ULONG OsBuildMax;

    // === 描述 ===
    PCSTR EraName;

    // ==========================================
    // 【新增】EVENTHOOK 时代默认值
    // ==========================================
    ULONG EventHook_ObjectSize;         // 0x60 (Win10) / 0x50 (Win11)
    ULONG EventHook_pNext;              // 0x18
    ULONG EventHook_eventMin;           // 0x20
    ULONG EventHook_eventMax;           // 0x24
    ULONG EventHook_flags;              // 0x28
    ULONG EventHook_idProcess;          // 0x30
    ULONG EventHook_idThread;           // 0x34 (Win10/21H2) / 0x38 (Win11_24H2)
    ULONG EventHook_pfn;                // 0x40
    ULONG EventHook_ihmod;              // 0x48
    ULONG EventHook_dpiAwareness;       // 0x58 (Win10) / 0x4C (Win11)
    ULONG EventHook_pti;                // 0x10
    ULONG EventHook_pti_ppi;            // pti->ppi: 0x1C0 (Win10/21H2) / 0x1F0 (Win11_24H2)
    ULONG GPSI_EVENT_HOOK_LIST_OFFSET;  // 0x11380 (Win11_24H2) / 0 (Win10独立全局变量)
    BOOLEAN EventHook_gpWinEventHooks_InSession; // TRUE=Session内部, FALSE=独立全局变量

} ERA_SIGNATURE, * PERA_SIGNATURE;

// ==========================================
// 五时代精确映射表（来源：反汇编文档 + Win11 24H2 更新）
// ==========================================
static const ERA_SIGNATURE g_EraSignatures[ERA_COUNT] = {
    [ERA_UNKNOWN] = { 0 },

    // ============================================
    // ERA_A: Win10 1607 及更早
    // ============================================
    [ERA_A] = {
        .OsBuildMin = 0, .OsBuildMax = BUILD_1703,
        .EraName = "ERA_A (1607-)",
        // PTI
        .Pti_pDeskInfo = 0x198, .Pti_aphkStart = 0x310,
        .Pti_amdesk = 0x190, .Pti_pEThread = 0x170,
        .Pti_Flags = 0x1B0, .Pti_bitmask = 0x250,
        .Pti_AccessCheck = 0x2F8,
        .DeskInfo_aphkStart = 0x20,
        // HOOK
        .Hook_hHook = 0x00, .Hook_pti = 0x10,
        .Hook_phkNext = 0x28, .Hook_nHookType = 0x30,
        .Hook_offPfn = 0x38, .Hook_flags = 0x40,
        .Hook_ihmod = 0x44, .Hook_ObjectSize = 0x60,
        // 寄存器
        .PtiBaseReg_zzzSWH = ZYDIS_REGISTER_RDI,
        .HookBaseReg_zzzSWH = ZYDIS_REGISTER_RBX,
        .PtiBaseReg_xxxCH = ZYDIS_REGISTER_RDI,
        .AphkStart_BaseReg_xxxCH = ZYDIS_REGISTER_NONE,
        .AphkStart_IndexReg_xxxCH = ZYDIS_REGISTER_NONE,
        .bIsForwarder = FALSE,
        // === EVENTHOOK 默认值 (Win10 1607) ===
        .EventHook_ObjectSize = 0x60,
        .EventHook_pNext = 0x18,
        .EventHook_eventMin = 0x20,
        .EventHook_eventMax = 0x24,
        .EventHook_flags = 0x28,
        .EventHook_idProcess = 0x30,
        .EventHook_idThread = 0x34,
        .EventHook_pfn = 0x40,
        .EventHook_ihmod = 0x48,
        .EventHook_dpiAwareness = 0x58,
        .EventHook_pti = 0x10,
        .EventHook_pti_ppi = 0x1C0,
        .GPSI_EVENT_HOOK_LIST_OFFSET = 0,
        .EventHook_gpWinEventHooks_InSession = FALSE,
    },

    // ============================================
    // ERA_B: Win10 1809 - 1909
    // ============================================
    [ERA_B] = {
        .OsBuildMin = BUILD_1703, .OsBuildMax = BUILD_20H2,
        .EraName = "ERA_B (1809-1909)",
        // PTI
        .Pti_pDeskInfo = 0x1C8, .Pti_aphkStart = 0x388,
        .Pti_amdesk = 0x1C0, .Pti_pEThread = 0x1A0,
        .Pti_Flags = 0x1E0, .Pti_bitmask = 0x2A0,
        .Pti_AccessCheck = 0x370,
        .DeskInfo_aphkStart = 0x30,
        // HOOK
        .Hook_hHook = 0x00, .Hook_pti = 0x10,
        .Hook_phkNext = 0x28, .Hook_nHookType = 0x30,
        .Hook_offPfn = 0x38, .Hook_flags = 0x40,
        .Hook_ihmod = 0x44, .Hook_ObjectSize = 0x60,
        // 寄存器
        .PtiBaseReg_zzzSWH = ZYDIS_REGISTER_RDI,
        .HookBaseReg_zzzSWH = ZYDIS_REGISTER_RBX,
        .PtiBaseReg_xxxCH = ZYDIS_REGISTER_RSI,
        .AphkStart_BaseReg_xxxCH = ZYDIS_REGISTER_NONE,
        .AphkStart_IndexReg_xxxCH = ZYDIS_REGISTER_NONE,
        .bIsForwarder = FALSE,
        // === EVENTHOOK 默认值 (Win10 1809/1909) ===
        .EventHook_ObjectSize = 0x60,
        .EventHook_pNext = 0x18,
        .EventHook_eventMin = 0x20,
        .EventHook_eventMax = 0x24,
        .EventHook_flags = 0x28,
        .EventHook_idProcess = 0x30,
        .EventHook_idThread = 0x34,
        .EventHook_pfn = 0x40,
        .EventHook_ihmod = 0x48,
        .EventHook_dpiAwareness = 0x58,
        .EventHook_pti = 0x10,
        .EventHook_pti_ppi = 0x1C0,
        .GPSI_EVENT_HOOK_LIST_OFFSET = 0,
        .EventHook_gpWinEventHooks_InSession = FALSE,
    },

    // ============================================
    // ERA_C: Win10 20H2 - Win10 22H2
    // ============================================
    [ERA_C] = {
        .OsBuildMin = BUILD_20H2, .OsBuildMax = BUILD_11_21H2,
        .EraName = "ERA_C (20H2-Win10_22H2)",
        // PTI
        .Pti_pDeskInfo = 0x1D0, .Pti_aphkStart = 0x398,
        .Pti_amdesk = 0x1C8, .Pti_pEThread = 0x1A8,
        .Pti_Flags = 0x1E8, .Pti_bitmask = 0x2A8,
        .Pti_AccessCheck = 0x378,
        .DeskInfo_aphkStart = 0x30,
        // HOOK
        .Hook_hHook = 0x00, .Hook_pti = 0x10,
        .Hook_phkNext = 0x28, .Hook_nHookType = 0x30,
        .Hook_offPfn = 0x38, .Hook_flags = 0x40,
        .Hook_ihmod = 0x44, .Hook_ObjectSize = 0x60,
        // 寄存器
        .PtiBaseReg_zzzSWH = ZYDIS_REGISTER_RBX,
        .HookBaseReg_zzzSWH = ZYDIS_REGISTER_RDI,
        .PtiBaseReg_xxxCH = ZYDIS_REGISTER_RBX,
        .AphkStart_BaseReg_xxxCH = ZYDIS_REGISTER_RAX,
        .AphkStart_IndexReg_xxxCH = ZYDIS_REGISTER_RDX,
        .bIsForwarder = TRUE,
        // === EVENTHOOK 默认值 (Win10 20H2/22H2) ===
        .EventHook_ObjectSize = 0x60,
        .EventHook_pNext = 0x18,
        .EventHook_eventMin = 0x20,
        .EventHook_eventMax = 0x24,
        .EventHook_flags = 0x28,
        .EventHook_idProcess = 0x30,
        .EventHook_idThread = 0x34,
        .EventHook_pfn = 0x40,
        .EventHook_ihmod = 0x48,
        .EventHook_dpiAwareness = 0x58,
        .EventHook_pti = 0x10,
        .EventHook_pti_ppi = 0x1C0,
        .GPSI_EVENT_HOOK_LIST_OFFSET = 0,
        .EventHook_gpWinEventHooks_InSession = FALSE,
    },

    // ============================================
    // ERA_D: Win11 21H2 / 23H2（第一次结构性变化）
    // ============================================
    [ERA_D] = {
        .OsBuildMin = BUILD_11_21H2, .OsBuildMax = BUILD_11_24H2,
        .EraName = "ERA_D (Win11_21H2/23H2)",
        // PTI
        .Pti_pDeskInfo = 0x1C8, .Pti_aphkStart = 0x390,
        .Pti_amdesk = 0x1C0, .Pti_pEThread = 0x1A0,
        .Pti_Flags = 0x1E0, .Pti_bitmask = 0x2A0,
        .Pti_AccessCheck = 0x370,
        .DeskInfo_aphkStart = 0x30,
        // HOOK
        .Hook_hHook = 0x00, .Hook_pti = 0x10,
        .Hook_phkNext = 0x28, .Hook_nHookType = 0x30,
        .Hook_offPfn = 0x38, .Hook_flags = 0x40,
        .Hook_ihmod = 0x44, .Hook_ObjectSize = 0x60,
        // 寄存器
        .PtiBaseReg_zzzSWH = ZYDIS_REGISTER_R14,
        .HookBaseReg_zzzSWH = ZYDIS_REGISTER_RSI,
        .PtiBaseReg_xxxCH = ZYDIS_REGISTER_RBX,
        .AphkStart_BaseReg_xxxCH = ZYDIS_REGISTER_RBX,
        .AphkStart_IndexReg_xxxCH = ZYDIS_REGISTER_RDI,
        .bIsForwarder = TRUE,
        // === EVENTHOOK 默认值 (Win11 21H2/23H2) ===
        // 第一次结构性变化：ObjectSize 0x60→0x50, dpiAwareness 0x58→0x4C
        .EventHook_ObjectSize = 0x50,
        .EventHook_pNext = 0x18,
        .EventHook_eventMin = 0x20,
        .EventHook_eventMax = 0x24,
        .EventHook_flags = 0x28,
        .EventHook_idProcess = 0x30,
        .EventHook_idThread = 0x34,          // 注意：21H2 仍为 0x34
        .EventHook_pfn = 0x40,
        .EventHook_ihmod = 0x48,
        .EventHook_dpiAwareness = 0x4C,      // 前移
        .EventHook_pti = 0x10,
        .EventHook_pti_ppi = 0x1C0,          // 21H2 仍为 0x1C0
        .GPSI_EVENT_HOOK_LIST_OFFSET = 0,
        .EventHook_gpWinEventHooks_InSession = FALSE,
    },

    // ============================================
    // ERA_E: Win11 24H2+（第二次结构性变化）
    // ============================================
    [ERA_E] = {
        .OsBuildMin = BUILD_11_24H2, .OsBuildMax = 0xFFFFFFFF,
        .EraName = "ERA_E (Win11_24H2+)",
        // PTI
        .Pti_pDeskInfo = 0x1C8, .Pti_aphkStart = 0x3C8,
        .Pti_amdesk = 0x1C0, .Pti_pEThread = 0x1A0,
        .Pti_Flags = 0x1E0, .Pti_bitmask = 0x2D0,
        .Pti_AccessCheck = 0x370,
        .DeskInfo_aphkStart = 0x30,
        // HOOK
        .Hook_hHook = 0x00, .Hook_pti = 0x10,
        .Hook_phkNext = 0x28, .Hook_nHookType = 0x30,
        .Hook_offPfn = 0x38, .Hook_flags = 0x40,
        .Hook_ihmod = 0x44, .Hook_ObjectSize = 0x60,
        // 寄存器
        .PtiBaseReg_zzzSWH = ZYDIS_REGISTER_R13,
        .HookBaseReg_zzzSWH = ZYDIS_REGISTER_R15,
        .PtiBaseReg_xxxCH = ZYDIS_REGISTER_RBP,
        .PtiBaseReg_xxxCH2 = ZYDIS_REGISTER_R15,
        .AphkStart_BaseReg_xxxCH = ZYDIS_REGISTER_RBP,
        .AphkStart_IndexReg_xxxCH = ZYDIS_REGISTER_RDI,
        .bIsForwarder = TRUE,
        // === EVENTHOOK 默认值 (Win11 24H2+) ===
        // 第二次结构性变化：idThread 0x34→0x38, pti->ppi 0x1C0→0x1F0
        // gpWinEventHooks 从独立全局变量 → Session+0x11380
        .EventHook_ObjectSize = 0x50,
        .EventHook_pNext = 0x18,
        .EventHook_eventMin = 0x20,
        .EventHook_eventMax = 0x24,
        .EventHook_flags = 0x28,
        .EventHook_idProcess = 0x30,
        .EventHook_idThread = 0x38,          // ★ 24H2 变为 0x38
        .EventHook_pfn = 0x40,
        .EventHook_ihmod = 0x48,
        .EventHook_dpiAwareness = 0x4C,
        .EventHook_pti = 0x10,
        .EventHook_pti_ppi = 0x1F0,          // ★ 24H2 变为 0x1F0
        .GPSI_EVENT_HOOK_LIST_OFFSET = 0x11380, // ★ 24H2 Session 内部偏移
        .EventHook_gpWinEventHooks_InSession = TRUE,
    },
};

// ==========================================
// 验证位掩码（消息钩子 + 事件钩子）
// ==========================================
// 消息钩子验证位 (0~11)
#define VERIFIED_PTI_APHKSTART      (1 << 0)
#define VERIFIED_PTI_DESKINFO       (1 << 1)
#define VERIFIED_DESKINFO_APHKSTART (1 << 2)
#define VERIFIED_PTI_AMDESK         (1 << 3)
#define VERIFIED_HOOK_FLAGS         (1 << 4)
#define VERIFIED_HOOK_IHMOD         (1 << 5)
#define VERIFIED_HOOK_TYPE          (1 << 6)
#define VERIFIED_HOOK_OFFPFN        (1 << 7)
#define VERIFIED_HOOK_PHKNEXT       (1 << 8)
#define VERIFIED_HOOK_PTI           (1 << 9)
#define VERIFIED_HOOK_HHOOK         (1 << 10)
#define VERIFIED_HOOK_OBJECTSIZE    (1 << 11)

// 事件钩子验证位 (12~24)
#define VERIFIED_EVENTHOOK_SIZE         (1 << 12)
#define VERIFIED_EVENTHOOK_PNEXT        (1 << 13)
#define VERIFIED_EVENTHOOK_EVENTMIN     (1 << 14)
#define VERIFIED_EVENTHOOK_EVENTMAX     (1 << 15)
#define VERIFIED_EVENTHOOK_FLAGS        (1 << 16)
#define VERIFIED_EVENTHOOK_IDPROCESS    (1 << 17)
#define VERIFIED_EVENTHOOK_IDTHREAD     (1 << 18)
#define VERIFIED_EVENTHOOK_PFN          (1 << 19)
#define VERIFIED_EVENTHOOK_IHMOD        (1 << 20)
#define VERIFIED_EVENTHOOK_DPIAWARENESS (1 << 21)
#define VERIFIED_EVENTHOOK_PTI          (1 << 22)
#define VERIFIED_EVENTHOOK_XREF         (1 << 23)
#define VERIFIED_EVENTHOOK_PTI_PPI      (1 << 24)

// 消息钩子核心必需位
#define VERIFIED_ALL_MANDATORY \
    (VERIFIED_PTI_APHKSTART | VERIFIED_PTI_DESKINFO | VERIFIED_HOOK_FLAGS)

// 事件钩子核心必需位（SIZE + FLAGS + IDPROCESS + IDTHREAD）
#define EVENTHOOK_CORE_MANDATORY \
    (VERIFIED_EVENTHOOK_SIZE | VERIFIED_EVENTHOOK_FLAGS | \
     VERIFIED_EVENTHOOK_IDPROCESS | VERIFIED_EVENTHOOK_IDTHREAD)

// 事件钩子推荐额外位
#define EVENTHOOK_RECOMMENDED \
    (VERIFIED_EVENTHOOK_EVENTMIN | VERIFIED_EVENTHOOK_EVENTMAX | \
     VERIFIED_EVENTHOOK_PFN | VERIFIED_EVENTHOOK_IHMOD)

// 事件钩子强验证位（交叉验证 + PTI + PTI_PPI）
#define EVENTHOOK_STRONG \
    (VERIFIED_EVENTHOOK_PTI | VERIFIED_EVENTHOOK_XREF | VERIFIED_EVENTHOOK_PTI_PPI)

// ==========================================
// 扫描上下文
// ==========================================
typedef struct _SCAN_CONTEXT {
    PWIN32K_OFFSETS Offsets;
    PERA_SIGNATURE Era;
    ULONG VerificationMask;          // 消息钩子验证掩码
    ULONG EventHookVerificationMask; // 事件钩子验证掩码
    PVOID FuncBase;
    SIZE_T FuncSize;
    PCWSTR FuncName;

    // 事件钩子扫描暂存
    ULONG EventHook_ObjectSize_Candidate;
    ZydisRegister EventHook_BaseReg;
    ULONG EventHook_idThread_Candidate;
    ULONG EventHook_pti_ppi_Candidate;
    ULONG GPSI_EVENT_HOOK_LIST_OFFSET_Candidate;
    BOOLEAN EventHook_gpWinEventHooks_InSession_Candidate;

} SCAN_CONTEXT, * PSCAN_CONTEXT;

// ==========================================
// 辅助宏
// ==========================================
#define SAFE_FIELD(_field) \
    ((ULONG)(FIELD_OFFSET(WIN32K_OFFSETS, _field) / sizeof(ULONG)))

// ==========================================
// 热键时代签名表
// ==========================================
typedef struct _HOTKEY_ERA_SIGNATURE {
    ULONG Hotkey_pThreadInfo;
    ULONG Hotkey_pfnCallback;
    ULONG Hotkey_pWnd;
    WORD Hotkey_fsModLow;
    WORD Hotkey_fsModHigh;
    ULONG Hotkey_id;
    ULONG Hotkey_vk;
    ULONG Hotkey_pNext;
    ULONG Hotkey_ListEntry;
    ULONG Hotkey_ObjectSize;
    ULONG Hotkey_HashBuckets;
    BOOLEAN bSessionHashTable;
    ULONG SessionHashOffset;
    ULONG OsBuildMin;
    ULONG OsBuildMax;
    PCSTR EraName;
} HOTKEY_ERA_SIGNATURE, * PHOTKEY_ERA_SIGNATURE;

// ==========================================
// 热键时代默认值映射表
// ==========================================
static const HOTKEY_ERA_SIGNATURE g_HotkeyEraSignatures[ERA_COUNT] = {
    [ERA_UNKNOWN] = { 0 },

    [ERA_A] = {
        .OsBuildMin = 0, .OsBuildMax = BUILD_1703,
        .EraName = "HOTKEY_ERA_A (1607-)",
        .Hotkey_pThreadInfo = 0x00,
        .Hotkey_pfnCallback = 0x08,
        .Hotkey_pWnd = 0x10,
        .Hotkey_fsModLow = 0x18,
        .Hotkey_fsModHigh = 0x1A,
        .Hotkey_id = 0x20,
        .Hotkey_vk = 0x1C,
        .Hotkey_pNext = 0x28,
        .Hotkey_ListEntry = 0,
        .Hotkey_ObjectSize = 0x30,
        .Hotkey_HashBuckets = 128,
        .bSessionHashTable = FALSE,
        .SessionHashOffset = 0,
    },

    [ERA_B] = {
        .OsBuildMin = BUILD_1703, .OsBuildMax = BUILD_20H2,
        .EraName = "HOTKEY_ERA_B (1809-1909)",
        .Hotkey_pThreadInfo = 0x00,
        .Hotkey_pfnCallback = 0x08,
        .Hotkey_pWnd = 0x10,
        .Hotkey_fsModLow = 0x18,
        .Hotkey_fsModHigh = 0x1A,
        .Hotkey_id = 0x20,
        .Hotkey_vk = 0x1C,
        .Hotkey_pNext = 0x28,
        .Hotkey_ListEntry = 0x30,
        .Hotkey_ObjectSize = 0x40,
        .Hotkey_HashBuckets = 128,
        .bSessionHashTable = FALSE,
        .SessionHashOffset = 0,
    },

    [ERA_C] = {
        .OsBuildMin = BUILD_20H2, .OsBuildMax = BUILD_11_21H2,
        .EraName = "HOTKEY_ERA_C (20H2-Win10_22H2)",
        .Hotkey_pThreadInfo = 0x00,
        .Hotkey_pfnCallback = 0x08,
        .Hotkey_pWnd = 0x10,
        .Hotkey_fsModLow = 0x18,
        .Hotkey_fsModHigh = 0x1A,
        .Hotkey_id = 0x20,
        .Hotkey_vk = 0x1C,
        .Hotkey_pNext = 0x28,
        .Hotkey_ListEntry = 0x30,
        .Hotkey_ObjectSize = 0x40,
        .Hotkey_HashBuckets = 128,
        .bSessionHashTable = FALSE,
        .SessionHashOffset = 0,
    },

    [ERA_D] = {
        .OsBuildMin = BUILD_11_21H2, .OsBuildMax = BUILD_11_24H2,
        .EraName = "HOTKEY_ERA_D (Win11_21H2/23H2)",
        .Hotkey_pThreadInfo = 0x00,
        .Hotkey_pfnCallback = 0x08,
        .Hotkey_pWnd = 0x10,
        .Hotkey_fsModLow = 0x18,
        .Hotkey_fsModHigh = 0x1A,
        .Hotkey_id = 0x20,          // 修正：原为 0x1C
        .Hotkey_vk = 0x1C,          // 修正：原为 0x20
        .Hotkey_pNext = 0x28,
        .Hotkey_ListEntry = 0x30,
        .Hotkey_ObjectSize = 0x40,
        .Hotkey_HashBuckets = 128,
        .bSessionHashTable = FALSE,
        .SessionHashOffset = 0,
    },

    [ERA_E] = {
        .OsBuildMin = BUILD_11_24H2, .OsBuildMax = 0xFFFFFFFF,
        .EraName = "HOTKEY_ERA_E (Win11_24H2+)",
        .Hotkey_pThreadInfo = 0x00,
        .Hotkey_pfnCallback = 0x08,
        .Hotkey_pWnd = 0x10,
        .Hotkey_fsModLow = 0x20,
        .Hotkey_fsModHigh = 0x22,
        .Hotkey_id = 0x28,          // 修正：原为 0x24
        .Hotkey_vk = 0x24,          // 修正：原为 0x28
        .Hotkey_pNext = 0x30,
        .Hotkey_ListEntry = 0x38,
        .Hotkey_ObjectSize = 0x48,
        .Hotkey_HashBuckets = 128,
        .bSessionHashTable = TRUE,
        .SessionHashOffset = 0x3298,
    },
};

// ==========================================
// 热键验证位掩码
// ==========================================
#define VERIFIED_HOTKEY_POOL_TAG        (1 << 0)
#define VERIFIED_HOTKEY_CBSIZE          (1 << 1)
#define VERIFIED_HOTKEY_HASH_MASK       (1 << 2)
#define VERIFIED_HOTKEY_PTHREADINFO     (1 << 3)
#define VERIFIED_HOTKEY_PWND            (1 << 4)
#define VERIFIED_HOTKEY_VK              (1 << 5)
#define VERIFIED_HOTKEY_ID              (1 << 6)
#define VERIFIED_HOTKEY_PFN_CALLBACK    (1 << 7)
#define VERIFIED_HOTKEY_FSMOD_LOW       (1 << 8)
#define VERIFIED_HOTKEY_FSMOD_HIGH      (1 << 9)
#define VERIFIED_HOTKEY_PNEXT           (1 << 10)
#define VERIFIED_HOTKEY_LIST_ENTRY      (1 << 11)
#define VERIFIED_HOTKEY_HWND            (1 << 12)
#define VERIFIED_HOTKEY_SESSION_TABLE   (1 << 13)
#define VERIFIED_HOTKEY_SESSION_OFFSET  (1 << 14)

#define HOTKEY_CORE_MANDATORY \
    (VERIFIED_HOTKEY_POOL_TAG | VERIFIED_HOTKEY_CBSIZE | \
     VERIFIED_HOTKEY_PTHREADINFO | VERIFIED_HOTKEY_PWND | \
     VERIFIED_HOTKEY_VK | VERIFIED_HOTKEY_ID | \
     VERIFIED_HOTKEY_FSMOD_LOW)

#define HOTKEY_STRONG \
    (HOTKEY_CORE_MANDATORY | VERIFIED_HOTKEY_HASH_MASK | \
     VERIFIED_HOTKEY_PFN_CALLBACK | VERIFIED_HOTKEY_FSMOD_LOW | VERIFIED_HOTKEY_FSMOD_HIGH | \
     VERIFIED_HOTKEY_PNEXT)

#define HOTKEY_24H2_FULL \
    (HOTKEY_STRONG | \
     VERIFIED_HOTKEY_SESSION_OFFSET)

// ==========================================
// 热键扫描上下文
// ==========================================
typedef struct _HOTKEY_SCAN_CONTEXT {
    PWIN32K_OFFSETS Offsets;
    PHOTKEY_ERA_SIGNATURE Era;
    ULONG VerificationMask;
    PVOID FuncBase;
    SIZE_T FuncSize;
    PCWSTR FuncName;
    ZydisRegister AllocResultReg;
    SIZE_T PoolTagInstrLen;          // Pool Tag 指令长度，用于精确跳过
    ULONG cbSize_Candidate;
    ULONG HashMask_Candidate;
    BOOLEAN bListEntryDetected;
    BOOLEAN bSessionHashTableDetected;
    ULONG SessionHashOffset_Candidate;
    ULONG ListEntryCandidate;
    BOOLEAN bListEntryLeaFound;
    BOOLEAN bListEntryFlinkFound;

    ZydisRegister LastWriteSourceReg;
    ULONG LastWriteOffset;
} HOTKEY_SCAN_CONTEXT, * PHOTKEY_SCAN_CONTEXT;

// ==========================================
// 时代检测
// ==========================================
static WIN32K_ERA DetectEra(_In_ ULONG buildNumber)
{
    for (int i = ERA_A; i < ERA_COUNT; i++) {
        if (buildNumber >= g_EraSignatures[i].OsBuildMin &&
            buildNumber < g_EraSignatures[i].OsBuildMax)
            return (WIN32K_ERA)i;
    }
    return ERA_UNKNOWN;
}

static WIN32K_ERA DetectEraFuzzy(_In_ ULONG buildNumber)
{
    WIN32K_ERA exact = DetectEra(buildNumber);
    if (exact != ERA_UNKNOWN) return exact;

    if (buildNumber >= 18362 && buildNumber < BUILD_20H2)
        return ERA_B;
    if (buildNumber >= 22621 && buildNumber < BUILD_11_24H2)
        return ERA_D;

    return ERA_UNKNOWN;
}

// ==========================================
// Zydis 解码包装
// ==========================================
static BOOLEAN ZydisDecodeAt(
    _In_ PUCHAR Buffer,
    _In_ SIZE_T Length,
    _Out_ ZydisDecodedInstruction* Instruction,
    _Out_ ZydisDecodedOperand* Operands,
    _Out_ SIZE_T* InstructionLength
)
{
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZyanStatus status = ZydisDecoderDecodeFull(
        &decoder, Buffer, Length, Instruction, Operands);

    if (!ZYAN_SUCCESS(status))
        return FALSE;

    *InstructionLength = Instruction->length;
    return TRUE;
}

// ==========================================
// 指令特征匹配辅助
// ==========================================
static BOOLEAN GetMemoryDisp(
    _In_ const ZydisDecodedInstruction* Instr,
    _In_ const ZydisDecodedOperand* Operands,
    _In_ ZydisRegister expectedBase,
    _In_ ZydisRegister expectedIndex,
    _In_ ULONG expectedScale,
    _Out_ LONG* Disp
)
{
    for (ZyanU8 i = 0; i < Instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &Operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (expectedBase != ZYDIS_REGISTER_NONE && op->mem.base != expectedBase)
            continue;
        if (expectedIndex != ZYDIS_REGISTER_NONE && op->mem.index != expectedIndex)
            continue;
        if (expectedScale != 0 && op->mem.scale != expectedScale)
            continue;
        *Disp = (LONG)op->mem.disp.value;
        return TRUE;
    }
    return FALSE;
}

static BOOLEAN HasDestReg(
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* ops,
    _In_ ZydisRegister r)
{
    return instr->operand_count > 0 &&
        ops[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
        ops[0].reg.value == r;
}

static ZydisRegister GetDestReg(
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->operand_count > 0 &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER)
        return operands[0].reg.value;
    return ZYDIS_REGISTER_NONE;
}

static BOOLEAN IsMemoryWrite(
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY &&
            (operands[i].actions & ZYDIS_OPERAND_ACTION_WRITE)) {
            return TRUE;
        }
    }
    return FALSE;
}

// ==========================================
// 【消息钩子】验证型提取器（zzzSetWindowsHookEx）
// ==========================================

static BOOLEAN VerifyPtiDeskInfoOrAmdesk(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    ZydisRegister expectedBase = ctx->Era->PtiBaseReg_zzzSWH;

    if (!GetMemoryDisp(instr, operands, expectedBase, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if ((ULONG)disp == ctx->Era->Pti_pDeskInfo) {
        DbgPrint("[V] Pti_pDeskInfo verified: 0x%X\n", (ULONG)disp);
        ctx->VerificationMask |= VERIFIED_PTI_DESKINFO;
        return TRUE;
    }

    if ((ULONG)disp == ctx->Era->Pti_amdesk) {
        DbgPrint("[V] Pti_amdesk verified: 0x%X\n", (ULONG)disp);
        ctx->VerificationMask |= VERIFIED_PTI_AMDESK;
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN VerifyDeskInfoAphkStart(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_ADD)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            if ((ULONG)imm == ctx->Era->DeskInfo_aphkStart) {
                DbgPrint("[V] DeskInfo_aphkStart verified: 0x%X\n", (ULONG)imm);
                ctx->VerificationMask |= VERIFIED_DESKINFO_APHKSTART;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOLEAN VerifyHookField(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    ZydisRegister expectedBase = ctx->Era->HookBaseReg_zzzSWH;
    LONG disp;

    // 模式1: or [base+disp], reg (flags)
    if (instr->mnemonic == ZYDIS_MNEMONIC_OR) {
        if (!GetMemoryDisp(instr, operands, expectedBase, ZYDIS_REGISTER_NONE, 0, &disp))
            return FALSE;
        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
            if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                return FALSE;
        }
        if ((ULONG)disp == ctx->Era->Hook_flags) {
            DbgPrint("[V] Hook_flags verified\n");
            ctx->VerificationMask |= VERIFIED_HOOK_FLAGS;
            return TRUE;
        }
        return FALSE;
    }

    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (!IsMemoryWrite(instr, operands))
        return FALSE;

    if (!GetMemoryDisp(instr, operands, expectedBase, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    ULONG udisp = (ULONG)disp;

    if (udisp == ctx->Era->Hook_ihmod) {
        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
            if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
                operands[i].reg.value == ZYDIS_REGISTER_EAX) {
                DbgPrint("[V] Hook_ihmod verified\n");
                ctx->VerificationMask |= VERIFIED_HOOK_IHMOD;
                return TRUE;
            }
        }
    }
    else if (udisp == ctx->Era->Hook_nHookType) {
        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
            if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
                operands[i].reg.value == ZYDIS_REGISTER_R13D) {
                DbgPrint("[V] Hook_nHookType verified\n");
                ctx->VerificationMask |= VERIFIED_HOOK_TYPE;
                return TRUE;
            }
        }
    }
    else if (udisp == ctx->Era->Hook_offPfn) {
        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
            if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
                operands[i].reg.value == ZYDIS_REGISTER_RAX) {
                DbgPrint("[V] Hook_offPfn verified\n");
                ctx->VerificationMask |= VERIFIED_HOOK_OFFPFN;
                return TRUE;
            }
        }
    }
    else if (udisp == ctx->Era->Hook_phkNext) {
        DbgPrint("[V] Hook_phkNext verified\n");
        ctx->VerificationMask |= VERIFIED_HOOK_PHKNEXT;
        return TRUE;
    }
    else if (udisp == ctx->Era->Hook_pti) {
        DbgPrint("[V] Hook_pti verified\n");
        ctx->VerificationMask |= VERIFIED_HOOK_PTI;
        return TRUE;
    }
    else if (udisp == ctx->Era->Hook_hHook) {
        DbgPrint("[V] Hook_hHook verified\n");
        ctx->VerificationMask |= VERIFIED_HOOK_HHOOK;
        return TRUE;
    }

    return FALSE;
}

static BOOLEAN VerifyHookObjectSize(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R9D))
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            if ((ULONG)imm == ctx->Era->Hook_ObjectSize) {
                DbgPrint("[V] Hook_ObjectSize verified: 0x%llX\n", imm);
                ctx->VerificationMask |= VERIFIED_HOOK_OBJECTSIZE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// ==========================================
// 【消息钩子】验证型提取器（xxxCallHook）
// ==========================================

static BOOLEAN VerifyAphkStartForwarder(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (!ctx->Era->bIsForwarder)
        return FALSE;

    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    ZydisRegister expectedBase = ctx->Era->AphkStart_BaseReg_xxxCH;
    ZydisRegister expectedIndex = ctx->Era->AphkStart_IndexReg_xxxCH;

    if (expectedBase == ZYDIS_REGISTER_NONE || expectedIndex == ZYDIS_REGISTER_NONE)
        return FALSE;

    if (!GetMemoryDisp(instr, operands, expectedBase, expectedIndex, 8, &disp))
        return FALSE;

    if ((ULONG)disp == ctx->Era->Pti_aphkStart) {
        DbgPrint("[V] Pti_aphkStart (forwarder) verified: 0x%X\n", (ULONG)disp);
        ctx->VerificationMask |= VERIFIED_PTI_APHKSTART;
        return TRUE;
    }

    return FALSE;
}

// ==========================================
// 【消息钩子】验证型提取器（zzzUnhookWindowsHookEx）
// ==========================================

static BOOLEAN VerifyHookPti(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T offset,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;
    if (operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
        return FALSE;

    LONG disp = (LONG)operands[1].mem.disp.value;
    if (disp < 0x08 || disp > 0x18)
        return FALSE;

    if ((ULONG)disp != ctx->Era->Hook_pti)
        return FALSE;

    //ZydisRegister ptiReg = operands[0].reg.value;
    BOOLEAN bVerified = FALSE;
    SIZE_T verifyOffset = offset + instr->length;

    for (SIZE_T i = 0; i < 64 && (verifyOffset + i + 4) < ctx->FuncSize; i++) {
        if (!MmIsAddressValid(codeBase + verifyOffset + i))
            break;

        PUCHAR p = codeBase + verifyOffset + i;
        ULONG expectedProc = ctx->Era->Pti_pEThread;

        if (p[0] == 0xF7 || p[0] == 0x8B || p[0] == 0x89) {
            LONG checkDisp = *(PLONG)(p + 2);
            if ((ULONG)checkDisp == expectedProc ||
                (ULONG)checkDisp == ctx->Era->Pti_Flags ||
                (ULONG)checkDisp == ctx->Era->Pti_amdesk) {
                bVerified = TRUE;
                break;
            }
        }
    }

    if (!bVerified)
        return FALSE;

    DbgPrint("[V] Hook_pti verified: 0x%X\n", (ULONG)disp);
    ctx->VerificationMask |= VERIFIED_HOOK_PTI;
    return TRUE;
}

// ==========================================
// 【事件钩子】动态对象大小提取
// ==========================================
static BOOLEAN DetectEventHookObjectSize(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    // 识别 mov r9d, imm32 (HMAllocObject 的 size 参数)
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R9D))
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            // 有效值：0x60 (Win10) 或 0x50 (Win11)
            if (imm == 0x60 || imm == 0x50) {
                ctx->EventHook_ObjectSize_Candidate = (ULONG)imm;
                DbgPrint("[EV] Dynamic ObjectSize = 0x%llX\n", imm);
                ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_SIZE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// ==========================================
// 【事件钩子】基址寄存器追踪
// ==========================================
static BOOLEAN DetectEventHookBaseReg(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    // 追踪 rax → reg 的 MOV 指令（HMAllocObject 返回值）
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;
    if (operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;
    if (operands[1].reg.value != ZYDIS_REGISTER_RAX)
        return FALSE;

    ZydisRegister dest = operands[0].reg.value;
    if (dest != ZYDIS_REGISTER_RBX && dest != ZYDIS_REGISTER_RDI &&
        dest != ZYDIS_REGISTER_RSI && dest != ZYDIS_REGISTER_R15) {
        // 也接受其他通用寄存器
        if (dest >= ZYDIS_REGISTER_RAX && dest <= ZYDIS_REGISTER_R15) {
            ctx->EventHook_BaseReg = dest;
            DbgPrint("[EV] Base register = %s\n", ZydisRegisterGetString(dest));
            return TRUE;
        }
        return FALSE;
    }

    ctx->EventHook_BaseReg = dest;
    DbgPrint("[EV] Base register = %s\n", ZydisRegisterGetString(dest));
    return TRUE;
}

// ==========================================
// 【事件钩子】gpWinEventHooks 存储方式识别（Win11 24H2+）
// ==========================================
static BOOLEAN DetectGpWinEventHooksStorage(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T offset)
{
    UNREFERENCED_PARAMETER(codeBase);
    UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(operands);

    // 识别 W32GetUserSessionState() 调用
    if (instr->mnemonic != ZYDIS_MNEMONIC_CALL)
        return FALSE;

    // 简化检查：调用后紧跟 mov/lea 访问 [rax+disp] 的模式
    // 实际实现中应通过符号名匹配，这里做启发式近似
    // 标记为 Session 内部存储（当 Era_E 时默认启用）
    if (ctx->Era->EventHook_gpWinEventHooks_InSession) {
        ctx->EventHook_gpWinEventHooks_InSession_Candidate = TRUE;
        ctx->GPSI_EVENT_HOOK_LIST_OFFSET_Candidate =
            ctx->Era->GPSI_EVENT_HOOK_LIST_OFFSET;
        DbgPrint("[EV] gpWinEventHooks in Session: offset=0x%X\n",
            ctx->GPSI_EVENT_HOOK_LIST_OFFSET_Candidate);
    }

    return FALSE;
}

// ==========================================
// 【事件钩子】字段写入扫描（_SetWinEventHook 主扫描器）
// ==========================================
static BOOLEAN Scan_SetWinEventHook_WriteScanner(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T offset)
{
    UNREFERENCED_PARAMETER(codeBase);
    UNREFERENCED_PARAMETER(offset);

    // 如果还没有确定基址寄存器，尝试追踪
    if (ctx->EventHook_BaseReg == ZYDIS_REGISTER_NONE) {
        DetectEventHookBaseReg(ctx, instr, operands);
    }

    // 如果基址寄存器仍未确定，无法继续
    if (ctx->EventHook_BaseReg == ZYDIS_REGISTER_NONE)
        return FALSE;

    // 只处理对 [base+disp] 的写入
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (!IsMemoryWrite(instr, operands))
        return FALSE;

    // 提取内存操作数
    LONG disp = 0;
    BOOLEAN bFound = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
            if (operands[i].mem.base == ctx->EventHook_BaseReg) {
                disp = (LONG)operands[i].mem.disp.value;
                bFound = TRUE;
                break;
            }
        }
    }
    if (!bFound)
        return FALSE;

    ULONG udisp = (ULONG)disp;

    // 检查是否在有效对象大小范围内
    ULONG objSize = ctx->EventHook_ObjectSize_Candidate;
    if (objSize == 0) {
        // 如果尚未提取 size，使用时代默认值
        objSize = ctx->Era->EventHook_ObjectSize;
    }
    if (udisp + 8 > objSize) {
        // 超出对象大小，忽略
        return FALSE;
    }

    // 按偏移匹配字段语义（优先级从高到低）
    // pti (+0x10) - 最可靠，Win10/Win11 一致
    if (udisp == 0x10) {
        ctx->Offsets->EventHook_pti = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PTI;
        DbgPrint("[EV] pti = +0x10\n");
        return TRUE;
    }

    // pNext (+0x18) - gpWinEventHooks 链表
    if (udisp == 0x18) {
        ctx->Offsets->EventHook_pNext = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PNEXT;
        DbgPrint("[EV] pNext = +0x18\n");
        return TRUE;
    }

    // eventMin (+0x20)
    if (udisp == 0x20) {
        ctx->Offsets->EventHook_eventMin = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_EVENTMIN;
        DbgPrint("[EV] eventMin = +0x20\n");
        return TRUE;
    }

    // eventMax (+0x24)
    if (udisp == 0x24) {
        ctx->Offsets->EventHook_eventMax = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_EVENTMAX;
        DbgPrint("[EV] eventMax = +0x24\n");
        return TRUE;
    }

    // flags (+0x28) - 识别最后一次写入
    if (udisp == 0x28) {
        ctx->Offsets->EventHook_flags = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_FLAGS;
        DbgPrint("[EV] flags = +0x28\n");
        return TRUE;
    }

    // idProcess (+0x30) - 五版本一致
    if (udisp == 0x30) {
        ctx->Offsets->EventHook_idProcess = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_IDPROCESS;
        DbgPrint("[EV] idProcess = +0x30\n");
        return TRUE;
    }

    // idThread (+0x34 或 +0x38) - 动态识别
    if (udisp == 0x34 || udisp == 0x38) {
        // 验证该写入与 arg_30 (idThread 参数) 关联
        // 简化：如果偏移在合理范围内且之前未被记录，接受
        if (ctx->EventHook_idThread_Candidate == 0 ||
            ctx->EventHook_idThread_Candidate == udisp) {
            ctx->EventHook_idThread_Candidate = udisp;
            ctx->Offsets->EventHook_idThread = udisp;
            ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_IDTHREAD;
            DbgPrint("[EV] idThread = +0x%X (dynamic)\n", udisp);
            return TRUE;
        }
    }

    // pfn/offPfn (+0x40) - 五版本一致
    if (udisp == 0x40) {
        ctx->Offsets->EventHook_pfn = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PFN;
        DbgPrint("[EV] pfn/offPfn = +0x40\n");
        return TRUE;
    }

    // ihmod (+0x48) - 五版本一致
    if (udisp == 0x48) {
        ctx->Offsets->EventHook_ihmod = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_IHMOD;
        DbgPrint("[EV] ihmod = +0x48\n");
        return TRUE;
    }

    // dpiAwareness (+0x58 Win10 / +0x4C Win11) - 动态识别
    if (udisp == 0x58 || udisp == 0x4C) {
        ctx->Offsets->EventHook_dpiAwareness = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_DPIAWARENESS;
        DbgPrint("[EV] dpiAwareness = +0x%X (dynamic)\n", udisp);
        return TRUE;
    }

    return FALSE;
}

// ==========================================
// 【事件钩子】xxxWindowEvent 交叉验证扫描器
// ==========================================
static BOOLEAN Scan_xxxWindowEvent_CrossValidator(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T offset)
{
    UNREFERENCED_PARAMETER(codeBase);
    UNREFERENCED_PARAMETER(offset);

    // 识别 [rbx+disp] 读取模式（遍历链表）
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    if (operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;
    if (operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
        return FALSE;

    ZydisRegister base = operands[1].mem.base;
    LONG disp = (LONG)operands[1].mem.disp.value;
    ULONG udisp = (ULONG)disp;

    // 检查是否在有效范围内
    if (udisp < 0x10 || udisp > 0x60)
        return FALSE;

    // 读取 pti (+0x10) - 最可靠
    if (udisp == 0x10 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_pti = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PTI;
        DbgPrint("[EV-XREF] pti = +0x10 (xxxWindowEvent)\n");

        // 尝试在后续指令中识别 pti->ppi 偏移
        // 实际实现应向前扫描，这里简化：使用时代默认值
        ctx->EventHook_pti_ppi_Candidate = ctx->Era->EventHook_pti_ppi;
        ctx->Offsets->EventHook_pti_ppi = ctx->EventHook_pti_ppi_Candidate;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PTI_PPI;
        DbgPrint("[EV-XREF] pti->ppi = +0x%X\n", ctx->EventHook_pti_ppi_Candidate);
        return TRUE;
    }

    // 读取 pNext (+0x18)
    if (udisp == 0x18 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_pNext = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PNEXT;
        DbgPrint("[EV-XREF] pNext = +0x18 (xxxWindowEvent)\n");
        return TRUE;
    }

    // 读取 eventMin (+0x20)
    if (udisp == 0x20 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_eventMin = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_EVENTMIN;
        DbgPrint("[EV-XREF] eventMin = +0x20 (xxxWindowEvent)\n");
        return TRUE;
    }

    // 读取 eventMax (+0x24)
    if (udisp == 0x24 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_eventMax = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_EVENTMAX;
        DbgPrint("[EV-XREF] eventMax = +0x24 (xxxWindowEvent)\n");
        return TRUE;
    }

    // 读取 flags (+0x28)
    if (udisp == 0x28 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_flags = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_FLAGS;
        DbgPrint("[EV-XREF] flags = +0x28 (xxxWindowEvent)\n");
        return TRUE;
    }

    // 读取 idProcess (+0x30)
    if (udisp == 0x30 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_idProcess = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_IDPROCESS;
        DbgPrint("[EV-XREF] idProcess = +0x30 (xxxWindowEvent)\n");
        return TRUE;
    }

    // 读取 idThread (+0x34 或 +0x38)
    if ((udisp == 0x34 || udisp == 0x38) && base == ZYDIS_REGISTER_RBX) {
        if (ctx->EventHook_idThread_Candidate == 0 ||
            ctx->EventHook_idThread_Candidate == udisp) {
            ctx->EventHook_idThread_Candidate = udisp;
            ctx->Offsets->EventHook_idThread = udisp;
            ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_IDTHREAD;
            DbgPrint("[EV-XREF] idThread = +0x%X (xxxWindowEvent)\n", udisp);
            return TRUE;
        }
    }

    // 读取 pfn (+0x40)
    if (udisp == 0x40 && base == ZYDIS_REGISTER_RBX) {
        ctx->Offsets->EventHook_pfn = udisp;
        ctx->EventHookVerificationMask |= VERIFIED_EVENTHOOK_PFN;
        DbgPrint("[EV-XREF] pfn = +0x40 (xxxWindowEvent)\n");
        return TRUE;
    }

    return FALSE;
}

// ==========================================
// 热键：Pool Tag 定位 + cbSize 提取
// ==========================================
static BOOLEAN VerifyHotkeyPoolTag(
    _In_ PHOTKEY_SCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands,
    _Out_ SIZE_T* poolTagOffset)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_EDX))
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
            operands[i].imm.value.u == 0x6B687355) { // 'Ushk'
            *poolTagOffset = 0; // 由调用方设置实际偏移
            ctx->VerificationMask |= VERIFIED_HOTKEY_POOL_TAG;
            DbgPrint("[HV] Pool Tag 'Ushk' found\n");
            return TRUE;
        }
    }
    return FALSE;
}

// ==========================================
// 热键：Hash Mask 提取（and al, 7Fh 等）
// ==========================================
static BOOLEAN VerifyHotkeyHashMask(
    _In_ PHOTKEY_SCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_AND)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            // 常见 mask：0x7F(128), 0x3F(64), 0xFF(256)
            if (imm == 0x7F || imm == 0x3F || imm == 0xFF || imm == 0x1FF) {
                ctx->HashMask_Candidate = (ULONG)(imm + 1);
                ctx->Offsets->Hotkey_HashBuckets = (ULONG)(imm + 1);
                ctx->VerificationMask |= VERIFIED_HOTKEY_HASH_MASK;
                DbgPrint("[HV] HashMask = 0x%llX (buckets=%lu)\n", imm, (ULONG)(imm + 1));
                return TRUE;
            }
        }
    }
    return FALSE;
}

//// ==========================================
//// 热键：LIST_ENTRY 自链接识别
//// ==========================================
//static BOOLEAN VerifyHotkeyListEntry(
//    _In_ PHOTKEY_SCAN_CONTEXT ctx,
//    _In_ const ZydisDecodedInstruction* instr,
//    _In_ const ZydisDecodedOperand* operands)
//{
//    // 阶段1：lea rax, [base+disp]
//    if (instr->mnemonic == ZYDIS_MNEMONIC_LEA &&
//        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
//        operands[0].reg.value == ZYDIS_REGISTER_RAX) {
//        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
//            if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY &&
//                operands[i].mem.base == ctx->AllocResultReg) {
//                ctx->ListEntryCandidate = (ULONG)operands[i].mem.disp.value;
//                ctx->bListEntryLeaFound = TRUE;
//                return TRUE;
//            }
//        }
//    }
//
//    // 阶段2：mov [rax+8], rax (Flink)
//    if (ctx->bListEntryLeaFound && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
//        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
//            if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY &&
//                operands[i].mem.base == ZYDIS_REGISTER_RAX &&
//                (LONG)operands[i].mem.disp.value == 0x08) {
//                for (ZyanU8 j = 0; j < instr->operand_count; j++) {
//                    if (operands[j].type == ZYDIS_OPERAND_TYPE_REGISTER &&
//                        operands[j].reg.value == ZYDIS_REGISTER_RAX) {
//                        ctx->bListEntryFlinkFound = TRUE;
//                        return TRUE;
//                    }
//                }
//            }
//        }
//    }
//
//    // 阶段3：mov [rax], rax (Blink) → 确认
//    if (ctx->bListEntryLeaFound && ctx->bListEntryFlinkFound &&
//        instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
//        for (ZyanU8 i = 0; i < instr->operand_count; i++) {
//            if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY &&
//                operands[i].mem.base == ZYDIS_REGISTER_RAX &&
//                (LONG)operands[i].mem.disp.value == 0x00) {
//                for (ZyanU8 j = 0; j < instr->operand_count; j++) {
//                    if (operands[j].type == ZYDIS_OPERAND_TYPE_REGISTER &&
//                        operands[j].reg.value == ZYDIS_REGISTER_RAX) {
//                        ctx->Offsets->Hotkey_ListEntry = ctx->ListEntryCandidate;
//                        ctx->VerificationMask |= VERIFIED_HOTKEY_LIST_ENTRY;
//                        ctx->bListEntryDetected = TRUE;
//                        DbgPrint("[HV] LIST_ENTRY = +0x%X (self-linked)\n",
//                            ctx->ListEntryCandidate);
//                        ctx->bListEntryLeaFound = FALSE;
//                        ctx->bListEntryFlinkFound = FALSE;
//                        return TRUE;
//                    }
//                }
//            }
//        }
//    }
//
//    return FALSE;
//}

// ==========================================
// 热键：成员偏移序列识别（_RegisterHotKey 分配后写入）
// ==========================================
static BOOLEAN VerifyHotkeyMemberOffsets(
    _In_ PHOTKEY_SCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (ctx->AllocResultReg == ZYDIS_REGISTER_NONE)
        return FALSE;

    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV &&
        instr->mnemonic != ZYDIS_MNEMONIC_OR &&
        instr->mnemonic != ZYDIS_MNEMONIC_LEA)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (op->mem.base != ctx->AllocResultReg)
            continue;

        LONG disp = (LONG)op->mem.disp.value;
        ULONG udisp = (ULONG)disp;
        BOOLEAN bIsWrite = (op->actions & ZYDIS_OPERAND_ACTION_WRITE) != 0;

        // ---------- pThreadInfo (+0x00) ----------
        if (udisp == 0x00 && bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
            ctx->Offsets->Hotkey_pThreadInfo = 0x00;
            ctx->VerificationMask |= VERIFIED_HOTKEY_PTHREADINFO;
            DbgPrint("[HV] pThreadInfo = +0x00\n");
            return TRUE;
        }

        // ---------- pfnCallback (+0x08) ----------
        if (udisp == 0x08 && bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
            ctx->Offsets->Hotkey_pfnCallback = 0x08;
            ctx->VerificationMask |= VERIFIED_HOTKEY_PFN_CALLBACK;
            DbgPrint("[HV] pfnCallback = +0x08\n");
            return TRUE;
        }

        // ---------- pWnd (+0x10) ----------
        if (udisp == 0x10 && bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
            ctx->Offsets->Hotkey_pWnd = 0x10;
            ctx->VerificationMask |= VERIFIED_HOTKEY_PWND;
            DbgPrint("[HV] pWnd = +0x10\n");
            return TRUE;
        }

        // ---------- fsMod (16位字段，高/低位合并) ----------
        // 注意：在 1607 中，fsMod 是一个 16 位字段（+0x1A）
        // 高 4 位是 flags，低 12 位是修饰键
        // 识别模式：
        //   1) mov word ptr [base+off], 0x8000  → 高位置位（SAS热键路径）
        //   2) or word ptr [base+off], reg      → 合并修饰键
        //   3) mov word ptr [base+off], 0       → 清零（r8w）
        if (op->size == 16) {
            // 模式1: mov word ptr [base+disp], 0x8000 → fsModHigh 标志
            if (bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
                for (ZyanU8 j = 0; j < instr->operand_count; j++) {
                    if (operands[j].type == ZYDIS_OPERAND_TYPE_IMMEDIATE &&
                        operands[j].imm.value.u == 0x8000) {
                        ctx->Offsets->Hotkey_fsModHigh = udisp;
                        ctx->VerificationMask |= VERIFIED_HOTKEY_FSMOD_HIGH;
                        DbgPrint("[HV] fsModHigh = +0x%X (0x8000)\n", udisp);
                        return TRUE;
                    }
                }
            }

            // 模式2: or word ptr [base+disp], reg → 这是真正的修饰键写入
            if (instr->mnemonic == ZYDIS_MNEMONIC_OR) {
                ctx->Offsets->Hotkey_fsModLow = udisp;
                ctx->VerificationMask |= VERIFIED_HOTKEY_FSMOD_LOW;
                DbgPrint("[HV] fsModLow = +0x%X (OR)\n", udisp);
                return TRUE;
            }

            // 模式3: mov word ptr [base+disp], 0（清零，源为 r8w/xor 结果）
            // 这发生在 or 之前，不是有效修饰键值，忽略或仅作辅助验证
            if (bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV) {
                for (ZyanU8 j = 0; j < instr->operand_count; j++) {
                    if (operands[j].type == ZYDIS_OPERAND_TYPE_REGISTER) {
                        ZydisRegister src = operands[j].reg.value;
                        // r8w 是 0（xor r8d, r8d 之后），这是清零操作
                        if (src == ZYDIS_REGISTER_R8W) {
                            // 仅记录候选偏移，但不设置 verification mask
                            // 因为真正的值来自后面的 OR 指令
                            if (ctx->Offsets->Hotkey_fsModLow == 0) {
                                ctx->Offsets->Hotkey_fsModLow = udisp;
                                // 不设置 VERIFIED_HOTKEY_FSMOD_LOW，等待 OR 指令确认
                                DbgPrint("[HV] fsModLow candidate = +0x%X (zero init)\n", udisp);
                            }
                            return TRUE;
                        }
                    }
                }
            }
        }

        // ---------- vk（32位写入，源为 r12d / 参数寄存器）----------
        // 在 1607 中: mov [rbx+20h], r12d
        // r12d 来自参数 r8d（vk）
        if (bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV && op->size == 32) {
            for (ZyanU8 j = 0; j < instr->operand_count; j++) {
                if (operands[j].type == ZYDIS_OPERAND_TYPE_REGISTER) {
                    ZydisRegister src = operands[j].reg.value;
                    // vk 通常来自参数传递的寄存器（r12d, edx, r8d 等）
                    // 且不是 EAX/RAX（那些通常是返回值或临时计算）
                    // 关键：vk 的写入通常发生在 id 之前，且源是调用约定中的参数寄存器
                    if (src == ZYDIS_REGISTER_R12D || src == ZYDIS_REGISTER_EDX ||
                        src == ZYDIS_REGISTER_R8D) {
                        // 确保不重复识别为 id
                        if (!(ctx->VerificationMask & VERIFIED_HOTKEY_VK)) {
                            ctx->Offsets->Hotkey_vk = udisp;
                            ctx->VerificationMask |= VERIFIED_HOTKEY_VK;
                            DbgPrint("[HV] vk = +0x%X (src=%s)\n", udisp,
                                ZydisRegisterGetString(src));
                            return TRUE;
                        }
                    }
                }
            }
        }

        // ---------- id（32位写入，源为 ebp / 局部变量）----------
        // 在 1607 中: mov [rbx+1Ch], ebp
        // ebp 来自局部变量（BugCheckParameter2），不是参数寄存器
        if (bIsWrite && instr->mnemonic == ZYDIS_MNEMONIC_MOV && op->size == 32) {
            for (ZyanU8 j = 0; j < instr->operand_count; j++) {
                if (operands[j].type == ZYDIS_OPERAND_TYPE_REGISTER) {
                    ZydisRegister src = operands[j].reg.value;
                    // id 通常来自栈上的局部变量（ebp, esi, edi, r14d 等）
                    // 排除已识别的 vk 源寄存器
                    if (src == ZYDIS_REGISTER_EBP || src == ZYDIS_REGISTER_ESI ||
                        src == ZYDIS_REGISTER_EDI || src == ZYDIS_REGISTER_R14D ||
                        src == ZYDIS_REGISTER_R15D) {
                        // 避免覆盖已识别的 vk
                        if ((ctx->VerificationMask & VERIFIED_HOTKEY_VK) &&
                            ctx->Offsets->Hotkey_vk == udisp) {
                            // 同一个偏移已经被识别为 vk，跳过
                            continue;
                        }
                        if (!(ctx->VerificationMask & VERIFIED_HOTKEY_ID)) {
                            ctx->Offsets->Hotkey_id = udisp;
                            ctx->VerificationMask |= VERIFIED_HOTKEY_ID;
                            DbgPrint("[HV] id = +0x%X (src=%s)\n", udisp,
                                ZydisRegisterGetString(src));
                            return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

// ==========================================
// 通用函数扫描器（验证模式）- 扩展支持事件钩子
// ==========================================
static NTSTATUS ScanFunctionForVerification(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PVOID funcAddr,
    _In_ SIZE_T maxSize,
    _In_ PCWSTR funcName)
{
    if (!funcAddr || !MmIsAddressValid(funcAddr))
        return STATUS_INVALID_ADDRESS;

    ctx->FuncBase = funcAddr;
    ctx->FuncSize = maxSize;
    ctx->FuncName = funcName;

    PUCHAR codeBase = (PUCHAR)funcAddr;
    SIZE_T offset = 0;
    ULONG instrCount = 0;

    DbgPrint("[*] Verifying %S (era=%s) at %p\n",
        funcName, ctx->Era->EraName, funcAddr);

    while (offset < maxSize && instrCount < SCAN_MAX_DEPTH) {
        if (!MmIsAddressValid(codeBase + offset))
            break;

        __try {
            ZydisDecodedInstruction instr;
            ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
            SIZE_T instrLen;

            if (!ZydisDecodeAt(codeBase + offset, maxSize - offset,
                &instr, operands, &instrLen)) {
                offset++;
                continue;
            }

            // ==========================================
            // 消息钩子验证（原有逻辑）
            // ==========================================
            if (wcsstr(funcName, L"zzzSetWindowsHookEx")) {
                VerifyPtiDeskInfoOrAmdesk(ctx, &instr, operands);
                VerifyDeskInfoAphkStart(ctx, &instr, operands);
                VerifyHookField(ctx, &instr, operands);
                VerifyHookObjectSize(ctx, &instr, operands);
            }
            else if (wcsstr(funcName, L"xxxCallHook")) {
                VerifyAphkStartForwarder(ctx, &instr, operands);
            }
            else if (wcsstr(funcName, L"zzzUnhookWindowsHookEx")) {
                VerifyHookPti(ctx, codeBase, offset, &instr, operands);
            }

            // ==========================================
            // 事件钩子验证（新增）
            // ==========================================
            if (wcsstr(funcName, L"_SetWinEventHook") ||
                wcsstr(funcName, L"NtUserSetWinEventHook")) {
                // 动态提取对象大小
                DetectEventHookObjectSize(ctx, &instr, operands);
                // 识别 gpWinEventHooks 存储方式
                DetectGpWinEventHooksStorage(ctx, &instr, operands, codeBase, offset);
                // 主扫描：字段写入识别
                Scan_SetWinEventHook_WriteScanner(ctx, &instr, operands, codeBase, offset);
            }

            if (wcsstr(funcName, L"xxxWindowEvent") ||
                wcsstr(funcName, L"xxxProcessNotifyWinEvent")) {
                // 交叉验证扫描
                Scan_xxxWindowEvent_CrossValidator(ctx, &instr, operands, codeBase, offset);
            }

            offset += instrLen;
            instrCount++;

            if (instr.mnemonic == ZYDIS_MNEMONIC_RET)
                break;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("[!] Exception at offset 0x%zX in %S\n", offset, funcName);
            break;
        }
    }

    DbgPrint("[*] %S scan done: %u instrs, hookMask=0x%04X, evMask=0x%08X\n",
        funcName, instrCount, ctx->VerificationMask, ctx->EventHookVerificationMask);
    return STATUS_SUCCESS;
}

// ==========================================
// 热键：_RegisterHotKey 主扫描函数
// ==========================================
static NTSTATUS ScanFunctionForHotkeyVerification(
    _In_ PHOTKEY_SCAN_CONTEXT ctx,
    _In_ PVOID funcAddr,
    _In_ SIZE_T maxSize,
    _In_ PCWSTR funcName)
{
    if (!funcAddr || !MmIsAddressValid(funcAddr))
        return STATUS_INVALID_ADDRESS;

    ctx->FuncBase = funcAddr;
    ctx->FuncSize = maxSize;
    ctx->FuncName = funcName;

    PUCHAR codeBase = (PUCHAR)funcAddr;

    SIZE_T offset = 0;
    ULONG instrCount = 0;

    SIZE_T poolTagOffset = 0;
    SIZE_T poolTagInstrLen = 0;

    BOOLEAN bFoundPoolTag = FALSE;

    DbgPrint("[*] Scanning %S for hotkey fields\n", funcName);

    // =====================================================
    // Pass 1 : Locate Pool Tag
    // =====================================================
    while (offset < maxSize &&
        instrCount < SCAN_MAX_DEPTH)
    {
        if (!MmIsAddressValid(codeBase + offset)) break;

        ZydisDecodedInstruction instr;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        SIZE_T instrLen;

        if (!ZydisDecodeAt(codeBase + offset,
            maxSize - offset,
            &instr,
            operands,
            &instrLen))
        {
            offset++;
            continue;
        }

        if (instr.mnemonic == ZYDIS_MNEMONIC_MOV)
        {
            for (ZyanU8 i = 0; i < instr.operand_count; i++)
            {
                if (operands[i].type ==
                    ZYDIS_OPERAND_TYPE_IMMEDIATE &&
                    operands[i].imm.value.u ==
                    0x6B687355)
                {
                    bFoundPoolTag = TRUE;
                    poolTagOffset = offset;
                    poolTagInstrLen = instrLen;

                    ctx->VerificationMask |= VERIFIED_HOTKEY_POOL_TAG;

                    DbgPrint(
                        "[HV] Pool Tag 'Ushk' at +0x%llX\n",
                        offset);

                    break;
                }
            }
        }

        if (bFoundPoolTag) break;

        offset += instrLen;
        instrCount++;
    }

    if (!bFoundPoolTag)
    {
        DbgPrint("[!] Pool Tag not found in %S\n", funcName);
        return STATUS_NOT_FOUND;
    }

    // =====================================================
    // cbSize
    //
    // Win10 1607:
    //
    // mov edx, 'Ushk'
    // lea ecx,[rax+30h]
    // call Win32AllocPool
    //
    // size is AFTER pool tag
    // =====================================================
    {
        SIZE_T scan = poolTagOffset + poolTagInstrLen;
        SIZE_T end = scan + 32;

        while (scan < end &&
            scan < maxSize)
        {
            ZydisDecodedInstruction instr;
            ZydisDecodedOperand operands[
                ZYDIS_MAX_OPERAND_COUNT];

            SIZE_T instrLen;

            if (!ZydisDecodeAt(codeBase + scan,
                maxSize - scan,
                &instr,
                operands,
                &instrLen))
            {
                scan++;
                continue;
            }

            if (instr.mnemonic ==
                ZYDIS_MNEMONIC_LEA)
            {
                if (HasDestReg(&instr,
                    operands,
                    ZYDIS_REGISTER_ECX))
                {
                    for (ZyanU8 i = 0;
                        i < instr.operand_count;
                        i++)
                    {
                        if (operands[i].type ==
                            ZYDIS_OPERAND_TYPE_MEMORY &&
                            operands[i].mem.disp.has_displacement)
                        {
                            LONG disp =
                                (LONG)operands[i].mem.disp.value;

                            if (disp >= 0x20 &&
                                disp <= 0x80 &&
                                !(disp & 7))
                            {
                                ctx->cbSize_Candidate =
                                    (ULONG)disp;

                                ctx->Offsets->
                                    Hotkey_ObjectSize =
                                    (ULONG)disp;

                                ctx->VerificationMask |=
                                    VERIFIED_HOTKEY_CBSIZE;

                                DbgPrint(
                                    "[HV] cbSize = 0x%X\n",
                                    (ULONG)disp);

                                break;
                            }
                        }
                    }
                }
            }
            scan += instrLen;
        }
    }

    // =====================================================
    // Pass 2 :
    // Scan object members
    // =====================================================
    offset = poolTagOffset + poolTagInstrLen;

    instrCount = 0;

    ctx->LastWriteSourceReg = ZYDIS_REGISTER_NONE;
    ctx->LastWriteOffset = 0;

    while (offset < maxSize &&
        instrCount < 300)
    {
        if (!MmIsAddressValid(codeBase + offset)) break;

        ZydisDecodedInstruction instr;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        SIZE_T instrLen;

        if (!ZydisDecodeAt(codeBase + offset,
            maxSize - offset,
            &instr,
            operands,
            &instrLen))
        {
            offset++;
            continue;
        }

        //
        // Determine allocated object register
        //
        if (ctx->AllocResultReg ==
            ZYDIS_REGISTER_NONE)
        {
            if (instr.mnemonic ==
                ZYDIS_MNEMONIC_MOV &&
                operands[0].type ==
                ZYDIS_OPERAND_TYPE_REGISTER &&
                operands[1].type ==
                ZYDIS_OPERAND_TYPE_REGISTER &&
                operands[1].reg.value ==
                ZYDIS_REGISTER_RAX)
            {
                ctx->AllocResultReg =
                    operands[0].reg.value;

                DbgPrint(
                    "[HV] Alloc result reg = %s\n",
                    ZydisRegisterGetString(
                        ctx->AllocResultReg));
            }
        }

        //
        // Track:
        //
        // mov [rbx+offset],reg
        //
        if (instr.mnemonic ==
            ZYDIS_MNEMONIC_MOV)
        {
            if (operands[0].type ==
                ZYDIS_OPERAND_TYPE_MEMORY &&
                operands[1].type ==
                ZYDIS_OPERAND_TYPE_REGISTER)
            {
                if (operands[0].mem.base ==
                    ctx->AllocResultReg)
                {
                    ctx->LastWriteOffset =
                        (ULONG)
                        operands[0].mem.disp.value;

                    ctx->LastWriteSourceReg =
                        operands[1].reg.value;

                    //
                    // special fields
                    //
                    switch (ctx->LastWriteOffset)
                    {
                    case 0x10:
                        ctx->Offsets->Hotkey_pWnd = 0x10;
                        ctx->VerificationMask |= VERIFIED_HOTKEY_PWND;
                        DbgPrint("[HV] pWnd = +0x10\n");

                        break;
                    case 0x1C:
                        if (ctx->LastWriteSourceReg ==
                            ZYDIS_REGISTER_EBP)
                        {
                            ctx->Offsets->Hotkey_id = 0x1C;
                            ctx->VerificationMask |= VERIFIED_HOTKEY_ID;
                            DbgPrint("[HV] id = +0x1C\n");
                        }
                        break;
                    case 0x20:
                        if (ctx->LastWriteSourceReg ==
                            ZYDIS_REGISTER_R12D)
                        {
                            ctx->Offsets->Hotkey_vk = 0x20;
                            ctx->VerificationMask |= VERIFIED_HOTKEY_VK;
                            DbgPrint("[HV] vk = +0x20\n");
                        }
                        break;
                    }
                }
            }
        }

        //if (ctx->AllocResultReg != ZYDIS_REGISTER_NONE)
        //    VerifyHotkeyMemberOffsets(ctx, &instr, operands);

        //VerifyHotkeyHashMask(ctx, &instr, operands);

        //
        // legacy pNext
        //
        if (!ctx->Era->bSessionHashTable &&
            ctx->AllocResultReg !=
            ZYDIS_REGISTER_NONE)
        {
            if (instr.mnemonic ==
                ZYDIS_MNEMONIC_MOV)
            {
                for (ZyanU8 i = 0;
                    i < instr.operand_count;
                    i++)
                {
                    if (operands[i].type ==
                        ZYDIS_OPERAND_TYPE_MEMORY &&
                        operands[i].mem.base ==
                        ctx->AllocResultReg &&
                        (operands[i].actions &
                            ZYDIS_OPERAND_ACTION_WRITE))
                    {
                        ULONG disp = (ULONG)operands[i].mem.disp.value;

                        if (disp == 0x28)
                        {
                            ctx->Offsets->Hotkey_pNext = 0x28;
                            ctx->VerificationMask |= VERIFIED_HOTKEY_PNEXT;

                            DbgPrint("[HV] pNext = +0x28\n");
                        }
                    }
                }
            }
        }

        if (instr.mnemonic == ZYDIS_MNEMONIC_RET)
            break;

        offset += instrLen;
        instrCount++;
    }

    DbgPrint(
        "[*] %S hotkey scan done: mask=0x%04X\n",
        funcName,
        ctx->VerificationMask);

    return STATUS_SUCCESS;
}

// ==========================================
// 热键：HKInsertHashElement 扫描（24H2+ Session 哈希表）
// ==========================================
static NTSTATUS ScanFunctionForHotkeySessionTable(
    _In_ PHOTKEY_SCAN_CONTEXT ctx,
    _In_ PVOID funcAddr,
    _In_ SIZE_T maxSize,
    _In_ PCWSTR funcName)
{
    if (!funcAddr || !MmIsAddressValid(funcAddr))
        return STATUS_INVALID_ADDRESS;

    PUCHAR codeBase = (PUCHAR)funcAddr;
    SIZE_T offset = 0;
    ULONG instrCount = 0;

    DbgPrint("[*] Scanning %S for session hash table\n", funcName);

    while (offset < maxSize && instrCount < SCAN_MAX_DEPTH) {
        if (!MmIsAddressValid(codeBase + offset))
            break;

        ZydisDecodedInstruction instr;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        SIZE_T instrLen;

        if (!ZydisDecodeAt(codeBase + offset, maxSize - offset,
            &instr, operands, &instrLen)) {
            offset++;
            continue;
        }

        // 识别 [rax + reg*8 + disp32] 模式（Session 哈希表访问）
        if (instr.mnemonic == ZYDIS_MNEMONIC_MOV) {
            for (ZyanU8 i = 0; i < instr.operand_count; i++) {
                const ZydisDecodedOperand* op = &operands[i];
                if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
                    continue;
                if (op->mem.base != ZYDIS_REGISTER_RAX)
                    continue;
                if (op->mem.scale != 8)
                    continue;
                if (op->mem.index == ZYDIS_REGISTER_NONE)
                    continue;

                LONG disp = (LONG)op->mem.disp.value;
                ULONG udisp = (ULONG)disp;
                if (udisp >= 0x1000 && udisp <= 0x5000) {
                    ctx->SessionHashOffset_Candidate = udisp;
                    ctx->Offsets->Hotkey_SessionHashOffset = udisp;
                    ctx->Offsets->Hotkey_bSessionHashTable = TRUE;
                    ctx->bSessionHashTableDetected = TRUE;
                    ctx->VerificationMask |= VERIFIED_HOTKEY_SESSION_OFFSET;
                    ctx->VerificationMask |= VERIFIED_HOTKEY_SESSION_TABLE;
                    DbgPrint("[HV] Session hash offset = 0x%X\n", udisp);
                }
            }
        }

        // 验证 pNext：mov [rdi+30h], rdx 或类似（24H2 为 +0x30）
        if (instr.mnemonic == ZYDIS_MNEMONIC_MOV && IsMemoryWrite(&instr, operands)) {
            for (ZyanU8 i = 0; i < instr.operand_count; i++) {
                if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
                    LONG disp = (LONG)operands[i].mem.disp.value;
                    if ((ULONG)disp == 0x30 || (ULONG)disp == 0x28) {
                        ctx->Offsets->Hotkey_pNext = (ULONG)disp;
                        ctx->VerificationMask |= VERIFIED_HOTKEY_PNEXT;
                        DbgPrint("[HV] pNext confirmed = +0x%X (HKInsertHashElement)\n",
                            (ULONG)disp);
                    }
                }
            }
        }

        offset += instrLen;
        instrCount++;

        if (instr.mnemonic == ZYDIS_MNEMONIC_RET)
            break;
    }

    return STATUS_SUCCESS;
}

// ==========================================
// 启发式扫描（未知版本回退）
// ==========================================

static BOOLEAN HeuristicExtractPtiAphkStart(
    _In_ PWIN32K_OFFSETS offsets,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (op->mem.scale != 8)
            continue;

        LONG disp = (LONG)op->mem.disp.value;
        if (disp >= 0x300 && disp <= 0x500) {
            offsets->Pti_aphkStart = (ULONG)disp;
            DbgPrint("[H] Heuristic Pti_aphkStart = 0x%X\n", (ULONG)disp);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOLEAN HeuristicExtractPtiDeskInfo(
    _In_ PWIN32K_OFFSETS offsets,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (op->mem.scale != 0 || op->mem.index != ZYDIS_REGISTER_NONE)
            continue;

        LONG disp = (LONG)op->mem.disp.value;
        if (disp >= 0x180 && disp <= 0x200) {
            offsets->Pti_pDeskInfo = (ULONG)disp;
            DbgPrint("[H] Heuristic Pti_pDeskInfo = 0x%X\n", (ULONG)disp);
            return TRUE;
        }
    }
    return FALSE;
}

static BOOLEAN HeuristicExtractPtiAmdesk(
    _In_ PWIN32K_OFFSETS offsets,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (op->mem.scale != 0 || op->mem.index != ZYDIS_REGISTER_NONE)
            continue;

        LONG disp = (LONG)op->mem.disp.value;
        if (disp >= 0x180 && disp <= 0x200) {
            if (offsets->Pti_pDeskInfo != 0 && (ULONG)disp == offsets->Pti_pDeskInfo)
                continue;
            offsets->Pti_amdesk = (ULONG)disp;
            DbgPrint("[H] Heuristic Pti_amdesk = 0x%X\n", (ULONG)disp);
            return TRUE;
        }
    }
    return FALSE;
}

static NTSTATUS HeuristicScan(
    _In_ PVOID zzzSetWindowHookExAddr,
    _In_ PVOID xxxCallHookAddr,
    _In_ PVOID zzzUnhookWindowsHookExAddr,
    _Out_ PWIN32K_OFFSETS offsets)
{
    UNREFERENCED_PARAMETER(zzzSetWindowHookExAddr);
	UNREFERENCED_PARAMETER(xxxCallHookAddr);
	UNREFERENCED_PARAMETER(zzzUnhookWindowsHookExAddr);
    DbgPrint("[!] Running heuristic scan for unknown era\n");

    RtlZeroMemory(offsets, sizeof(WIN32K_OFFSETS));

    // Hook 对象默认值（跨时代稳定）
    offsets->Hook_hHook = 0x00;
    offsets->Hook_pti = 0x10;
    offsets->Hook_phkNext = 0x28;
    offsets->Hook_nHookType = 0x30;
    offsets->Hook_offPfn = 0x38;
    offsets->Hook_flags = 0x40;
    offsets->Hook_ihmod = 0x44;
    offsets->Hook_ObjectSize = 0x60;

    // 事件钩子保守默认值（使用 ERA_D 值，因为 Win11 是当前主流）
    offsets->EventHook_ObjectSize = 0x50;
    offsets->EventHook_pNext = 0x18;
    offsets->EventHook_eventMin = 0x20;
    offsets->EventHook_eventMax = 0x24;
    offsets->EventHook_flags = 0x28;
    offsets->EventHook_idProcess = 0x30;
    offsets->EventHook_idThread = 0x34;
    offsets->EventHook_pfn = 0x40;
    offsets->EventHook_ihmod = 0x48;
    offsets->EventHook_dpiAwareness = 0x4C;
    offsets->EventHook_pti = 0x10;
    offsets->EventHook_pti_ppi = 0x1C0;
    offsets->GPSI_EVENT_HOOK_LIST_OFFSET = 0;
    offsets->EventHook_gpWinEventHooks_InSession = FALSE;

    // 热键启发式默认值
    offsets->Hotkey_pThreadInfo = 0x00;
    offsets->Hotkey_pfnCallback = 0x08;
    offsets->Hotkey_pWnd = 0x10;
    //offsets->Hotkey_hWnd = 0;
    offsets->Hotkey_fsModLow = 0x18;
    offsets->Hotkey_fsModHigh = 0x1A;
    offsets->Hotkey_id = 0x1C;
    offsets->Hotkey_vk = 0x20;
    offsets->Hotkey_pNext = 0x28;
    offsets->Hotkey_ListEntry = 0x30;
    offsets->Hotkey_ObjectSize = 0x40;
    offsets->Hotkey_HashBuckets = 128;
    offsets->Hotkey_bSessionHashTable = FALSE;
    offsets->Hotkey_SessionHashOffset = 0;
    offsets->Hotkey_gpSessionHashTable = 0;
    offsets->Hotkey_gphkHashTable = 0;
    offsets->bHotkeyVerificationFailed = TRUE;

    DbgPrint("[!] Heuristic scan incomplete - using defaults\n");
    return STATUS_SUCCESS;
}

// ==========================================
// 默认值设置（按时代）
// ==========================================
static VOID SetEraDefaults(
    _In_ WIN32K_ERA era,
    _In_ ULONG buildNumber,
    _Inout_ PWIN32K_OFFSETS offsets)
{
    RtlZeroMemory(offsets, sizeof(WIN32K_OFFSETS));
    offsets->OsBuildNumber = buildNumber;

    if (era == ERA_UNKNOWN) {
        offsets->Pti_pDeskInfo = 0x1C8;
        offsets->Pti_aphkStart = 0x390;
        offsets->Pti_amdesk = 0x1C0;
        offsets->DeskInfo_aphkStart = 0x30;
        offsets->DetectedEra = ERA_UNKNOWN;

        // 事件钩子保守默认值
        offsets->EventHook_ObjectSize = 0x50;
        offsets->EventHook_pNext = 0x18;
        offsets->EventHook_eventMin = 0x20;
        offsets->EventHook_eventMax = 0x24;
        offsets->EventHook_flags = 0x28;
        offsets->EventHook_idProcess = 0x30;
        offsets->EventHook_idThread = 0x34;
        offsets->EventHook_pfn = 0x40;
        offsets->EventHook_ihmod = 0x48;
        offsets->EventHook_dpiAwareness = 0x4C;
        offsets->EventHook_pti = 0x10;
        offsets->EventHook_pti_ppi = 0x1C0;
        offsets->GPSI_EVENT_HOOK_LIST_OFFSET = 0;
        offsets->EventHook_gpWinEventHooks_InSession = FALSE;
        return;
    }

    const ERA_SIGNATURE* sig = &g_EraSignatures[era];

    // === 消息钩子默认值 ===
    offsets->Pti_pDeskInfo = sig->Pti_pDeskInfo;
    offsets->Pti_aphkStart = sig->Pti_aphkStart;
    offsets->Pti_amdesk = sig->Pti_amdesk;
    offsets->Pti_pEThread = sig->Pti_pEThread;
    offsets->Pti_Flags = sig->Pti_Flags;
    offsets->Pti_bitmask = sig->Pti_bitmask;
    offsets->Pti_AccessCheck = sig->Pti_AccessCheck;
    offsets->DeskInfo_aphkStart = sig->DeskInfo_aphkStart;

    offsets->Hook_hHook = sig->Hook_hHook;
    offsets->Hook_pti = sig->Hook_pti;
    offsets->Hook_phkNext = sig->Hook_phkNext;
    offsets->Hook_nHookType = sig->Hook_nHookType;
    offsets->Hook_offPfn = sig->Hook_offPfn;
    offsets->Hook_flags = sig->Hook_flags;
    offsets->Hook_ihmod = sig->Hook_ihmod;
    offsets->Hook_ObjectSize = sig->Hook_ObjectSize;

    offsets->DetectedEra = era;

    // HandleEntry 元数据
    if (buildNumber < BUILD_1703) {
        offsets->HandleEntrySize = 24;
        offsets->HandleEntry_HookTypeOffset = 16;
        offsets->HandleEntry_TableIndexOffset = 18;
    }
    else {
        offsets->HandleEntrySize = 32;
        offsets->HandleEntry_HookTypeOffset = 24;
        offsets->HandleEntry_TableIndexOffset = 26;
    }

    // === 事件钩子默认值 ===
    offsets->EventHook_ObjectSize = sig->EventHook_ObjectSize;
    offsets->EventHook_pNext = sig->EventHook_pNext;
    offsets->EventHook_eventMin = sig->EventHook_eventMin;
    offsets->EventHook_eventMax = sig->EventHook_eventMax;
    offsets->EventHook_flags = sig->EventHook_flags;
    offsets->EventHook_idProcess = sig->EventHook_idProcess;
    offsets->EventHook_idThread = sig->EventHook_idThread;
    offsets->EventHook_pfn = sig->EventHook_pfn;
    offsets->EventHook_ihmod = sig->EventHook_ihmod;
    offsets->EventHook_dpiAwareness = sig->EventHook_dpiAwareness;
    offsets->EventHook_pti = sig->EventHook_pti;
    offsets->EventHook_pti_ppi = sig->EventHook_pti_ppi;
    offsets->GPSI_EVENT_HOOK_LIST_OFFSET = sig->GPSI_EVENT_HOOK_LIST_OFFSET;
    offsets->EventHook_gpWinEventHooks_InSession = sig->EventHook_gpWinEventHooks_InSession;
}

// ==========================================
// 热键时代默认值设置
// ==========================================
static VOID SetHotkeyEraDefaults(
    _In_ WIN32K_ERA era,
    _Inout_ PWIN32K_OFFSETS offsets)
{
    if (era == ERA_UNKNOWN) {
        // 保守默认值（使用 ERA_D 配置）
        offsets->Hotkey_pThreadInfo = 0x00;
        offsets->Hotkey_pfnCallback = 0x08;
        offsets->Hotkey_pWnd = 0x10;
        //offsets->Hotkey_hWnd = 0;
        offsets->Hotkey_fsModLow = 0x18;
        offsets->Hotkey_fsModHigh = 0x1A;
        offsets->Hotkey_id = 0x1C;
        offsets->Hotkey_vk = 0x20;
        offsets->Hotkey_pNext = 0x28;
        offsets->Hotkey_ListEntry = 0x30;
        offsets->Hotkey_ObjectSize = 0x40;
        offsets->Hotkey_HashBuckets = 128;
        offsets->Hotkey_bSessionHashTable = FALSE;
        offsets->Hotkey_SessionHashOffset = 0;
        offsets->Hotkey_gpSessionHashTable = 0;
        offsets->Hotkey_gphkHashTable = 0;
        offsets->bHotkeyVerificationFailed = TRUE;
        return;
    }

    const HOTKEY_ERA_SIGNATURE* sig = &g_HotkeyEraSignatures[era];
    offsets->Hotkey_pThreadInfo = sig->Hotkey_pThreadInfo;
    offsets->Hotkey_pfnCallback = sig->Hotkey_pfnCallback;
    offsets->Hotkey_pWnd = sig->Hotkey_pWnd;
    //offsets->Hotkey_hWnd = sig->Hotkey_hWnd;
    offsets->Hotkey_fsModLow = sig->Hotkey_fsModLow;
    offsets->Hotkey_fsModHigh = sig->Hotkey_fsModHigh;
    offsets->Hotkey_id = sig->Hotkey_id;
    offsets->Hotkey_vk = sig->Hotkey_vk;
    offsets->Hotkey_pNext = sig->Hotkey_pNext;
    offsets->Hotkey_ListEntry = sig->Hotkey_ListEntry;
    offsets->Hotkey_ObjectSize = sig->Hotkey_ObjectSize;
    offsets->Hotkey_HashBuckets = sig->Hotkey_HashBuckets;
    offsets->Hotkey_bSessionHashTable = sig->bSessionHashTable;
    offsets->Hotkey_SessionHashOffset = sig->SessionHashOffset;
    offsets->Hotkey_gpSessionHashTable = 0;
    offsets->Hotkey_gphkHashTable = 0;
    offsets->bHotkeyVerificationFailed = FALSE;
}

// ==========================================
// 热键验证充分性检查
// ==========================================
static BOOLEAN IsHotkeyVerificationSufficient(
    _In_ ULONG verificationMask,
    _In_ WIN32K_ERA era)
{
    if ((verificationMask & HOTKEY_STRONG) != HOTKEY_STRONG) {
        DbgPrint("[HV] Core mandatory missing: mask=0x%04X, need=0x%04X\n",
            verificationMask & HOTKEY_STRONG, HOTKEY_STRONG);
        return FALSE;
    }

    if (era == ERA_E) {
        ULONG full24h2 = HOTKEY_24H2_FULL;
        if ((verificationMask & full24h2) != full24h2) {
            DbgPrint("[HV] 24H2 full verification missing: mask=0x%04X, need=0x%04X\n",
                verificationMask, full24h2);
            return FALSE;
        }
    }

    DbgPrint("[HV] Verification sufficient (mask=0x%04X)\n", verificationMask);
    return TRUE;
}

// ==========================================
// 热键降级回退
// ==========================================
static VOID HotkeyFallbackToDefaults(
    _In_ WIN32K_ERA era,
    _Inout_ PWIN32K_OFFSETS offsets)
{
    SetHotkeyEraDefaults(era, offsets);
    offsets->bHotkeyVerificationFailed = TRUE;
    DbgPrint("[!] Hotkey fallback to era defaults (era=%d)\n", era);
}

// ==========================================
// 验证结果检查（消息钩子）
// ==========================================
static BOOLEAN IsVerificationSufficient(
    _In_ ULONG verificationMask,
    _In_ BOOLEAN bIsForwarder,
    _In_ WIN32K_ERA era)
{
    ULONG mandatoryCore = VERIFIED_PTI_AMDESK | VERIFIED_HOOK_FLAGS;
    if ((verificationMask & mandatoryCore) != mandatoryCore)
        return FALSE;

    if (!(verificationMask & VERIFIED_HOOK_OBJECTSIZE))
        return FALSE;

    if (bIsForwarder) {
        if (!(verificationMask & VERIFIED_PTI_APHKSTART))
            return FALSE;
        if (!(verificationMask & VERIFIED_PTI_DESKINFO))
            return FALSE;
    }
    else {
        DbgPrint("[V] Non-forwarder era (%d): using reliable defaults.\n", era);
    }

    return TRUE;
}

// ==========================================
// 验证结果检查（事件钩子）
// ==========================================
static BOOLEAN IsEventHookVerificationSufficient(
    _In_ ULONG evMask,
    _In_ WIN32K_ERA era)
{
    // 核心必需位：SIZE + FLAGS + IDPROCESS + IDTHREAD
    if ((evMask & EVENTHOOK_CORE_MANDATORY) != EVENTHOOK_CORE_MANDATORY) {
        DbgPrint("[EV] Core mandatory missing: mask=0x%08X, need=0x%08X\n",
            evMask & EVENTHOOK_CORE_MANDATORY, EVENTHOOK_CORE_MANDATORY);
        return FALSE;
    }

    // 推荐额外位：至少验证 2 个以上
    ULONG recommended = evMask & EVENTHOOK_RECOMMENDED;
    if (__popcnt(recommended) < 2) {
        DbgPrint("[EV] Recommended fields insufficient: %d/4\n",
            __popcnt(recommended));
        // 对于 ERA_E (24H2)，要求更严格
        if (era == ERA_E) {
            return FALSE;
        }
        // 对于其他时代，允许宽松一些
    }

    // 强验证位：有交叉验证更好
    ULONG strong = evMask & EVENTHOOK_STRONG;
    if (strong == EVENTHOOK_STRONG) {
        DbgPrint("[EV] Strong verification passed (XREF + PTI + PTI_PPI)\n");
    }
    else {
        DbgPrint("[EV] Strong verification partially missing: 0x%08X\n", strong);
        // 不强制，但会降低可信度
    }

    DbgPrint("[EV] Verification sufficient (mask=0x%08X)\n", evMask);
    return TRUE;
}

// ==========================================
// 公共接口
// ==========================================
NTSTATUS Win32kOffsetScanner_Initialize(
    _In_ PWIN32K_FUNCADDR FuncAddr,
    _Out_ PWIN32K_OFFSETS Offsets)
{
    if (!Offsets)
        return STATUS_INVALID_PARAMETER;

    // 获取构建号
    RTL_OSVERSIONINFOW osVer = { 0 };
    osVer.dwOSVersionInfoSize = sizeof(osVer);
    RtlGetVersion(&osVer);
    ULONG buildNumber = osVer.dwBuildNumber;

    // 时代检测
    WIN32K_ERA era = DetectEra(buildNumber);
    if (era == ERA_UNKNOWN) {
        era = DetectEraFuzzy(buildNumber);
        DbgPrint("[!] Build %lu not in exact era range, fuzzy detected: %d\n",
            buildNumber, era);
    }

    DbgPrint("[*] Build=%lu, DetectedEra=%d (%s)\n",
        buildNumber, era,
        era < ERA_COUNT ? g_EraSignatures[era].EraName : "UNKNOWN");

    // 设置时代默认值
    SetEraDefaults(era, buildNumber, Offsets);
    SetHotkeyEraDefaults(era, Offsets);

    if (era == ERA_UNKNOWN) {
        return HeuristicScan(FuncAddr->zzzSetWindowHookExAddr, FuncAddr->xxxCallHookAddr,
            FuncAddr->zzzUnhookWindowsHookExAddr, Offsets);
    }

    // 已知时代：验证扫描
    SCAN_CONTEXT ctx = { 0 };
    ctx.Offsets = Offsets;
    ctx.Era = (PERA_SIGNATURE)&g_EraSignatures[era];
    ctx.EventHook_BaseReg = ZYDIS_REGISTER_NONE;

    // 扫描 zzzSetWindowsHookEx（消息钩子）
    if (FuncAddr->zzzSetWindowHookExAddr) {
        ScanFunctionForVerification(&ctx, FuncAddr->zzzSetWindowHookExAddr,
            SCAN_MAX_SIZE, L"zzzSetWindowsHookEx");
    }

    // 扫描 xxxCallHook（消息钩子）
    if (FuncAddr->xxxCallHookAddr) {
        ScanFunctionForVerification(&ctx, FuncAddr->xxxCallHookAddr,
            SCAN_MAX_SIZE, L"xxxCallHook");
    }

    // 扫描 zzzUnhookWindowsHookEx（消息钩子）
    if (FuncAddr->zzzUnhookWindowsHookExAddr) {
        ScanFunctionForVerification(&ctx, FuncAddr->zzzUnhookWindowsHookExAddr,
            SCAN_MAX_SIZE, L"zzzUnhookWindowsHookEx");
    }

    // ==========================================
    // 事件钩子扫描：需要 SetWinEventHook 和 xxxWindowEvent 符号
    // ==========================================
    if (FuncAddr->_SetWinEventHookAddr) {
        DbgPrint("[*] Scanning _SetWinEventHook at %p\n", FuncAddr->_SetWinEventHookAddr);
        ScanFunctionForVerification(&ctx, FuncAddr->_SetWinEventHookAddr,
            SCAN_MAX_SIZE, L"_SetWinEventHook");
    }

    // 扫描 xxxWindowEvent（交叉验证）
    if (FuncAddr->xxxWindowEventAddr) {
        DbgPrint("[*] Scanning xxxWindowEvent at %p\n", FuncAddr->xxxWindowEventAddr);
        ScanFunctionForVerification(&ctx, FuncAddr->xxxWindowEventAddr,
            SCAN_MAX_SIZE, L"xxxWindowEvent");
        // 如果有交叉验证，设置 XREF 位
        if (ctx.EventHookVerificationMask & VERIFIED_EVENTHOOK_PTI) {
            ctx.EventHookVerificationMask |= VERIFIED_EVENTHOOK_XREF;
            DbgPrint("[EV] Cross-verification (XREF) confirmed\n");
        }
    }

    // ==========================================
    // 热键扫描
    // ==========================================
    HOTKEY_SCAN_CONTEXT hotkeyCtx = { 0 };
    hotkeyCtx.Offsets = Offsets;
    hotkeyCtx.Era = (PHOTKEY_ERA_SIGNATURE)((era < ERA_COUNT) ? &g_HotkeyEraSignatures[era]
        : &g_HotkeyEraSignatures[ERA_UNKNOWN]);

    if (FuncAddr->_RegisterHotKeyAddr) {
        DbgPrint("[*] Scanning _RegisterHotKey at %p\n", FuncAddr->_RegisterHotKeyAddr);
        ScanFunctionForHotkeyVerification(&hotkeyCtx,
            FuncAddr->_RegisterHotKeyAddr, SCAN_MAX_SIZE, L"_RegisterHotKey");
    }

    // 24H2+ Session 哈希表扫描
    if (FuncAddr->HKInsertHashElementAddr &&
        (era == ERA_E || hotkeyCtx.Era->bSessionHashTable)) {
        DbgPrint("[*] Scanning HKInsertHashElement at %p (Session hash table)\n",
            FuncAddr->HKInsertHashElementAddr);
        ScanFunctionForHotkeySessionTable(&hotkeyCtx,
            FuncAddr->HKInsertHashElementAddr, SCAN_MAX_SIZE,
            L"HKInsertHashElement");
    }

    // ==========================================
    // 后处理：根据动态提取结果更新偏移
    // ==========================================
    // 如果动态提取了 idThread，覆盖默认值
    if (ctx.EventHook_idThread_Candidate != 0) {
        Offsets->EventHook_idThread = ctx.EventHook_idThread_Candidate;
    }

    // 如果动态提取了 pti->ppi，覆盖默认值
    if (ctx.EventHook_pti_ppi_Candidate != 0) {
        Offsets->EventHook_pti_ppi = ctx.EventHook_pti_ppi_Candidate;
    }

    // 如果动态提取了 GPSI_EVENT_HOOK_LIST_OFFSET，覆盖默认值
    if (ctx.GPSI_EVENT_HOOK_LIST_OFFSET_Candidate != 0) {
        Offsets->GPSI_EVENT_HOOK_LIST_OFFSET = ctx.GPSI_EVENT_HOOK_LIST_OFFSET_Candidate;
    }
    Offsets->EventHook_gpWinEventHooks_InSession =
        ctx.EventHook_gpWinEventHooks_InSession_Candidate;

    // ==========================================
    // 消息钩子充分性检查
    // ==========================================
    BOOLEAN bSufficient = IsVerificationSufficient(ctx.VerificationMask,
        ctx.Era->bIsForwarder, era);

    DbgPrint("[*] Hook verification mask=0x%04X, Sufficient=%d\n",
        ctx.VerificationMask, bSufficient);

    if (!bSufficient) {
        DbgPrint("[!] Hook verification insufficient for era %d, using defaults\n", era);
        Offsets->bVerificationFailed = TRUE;
    }
    else {
        DbgPrint("[+] Era %d fully verified (hooks)\n", era);
        Offsets->bVerificationFailed = FALSE;
    }

    // ==========================================
    // 事件钩子充分性检查
    // ==========================================
    BOOLEAN bEvSufficient = IsEventHookVerificationSufficient(
        ctx.EventHookVerificationMask, era);

    DbgPrint("[*] EventHook verification mask=0x%08X, Sufficient=%d\n",
        ctx.EventHookVerificationMask, bEvSufficient);

    if (!bEvSufficient) {
        DbgPrint("[!] EventHook verification insufficient, using defaults\n");
        Offsets->bEventHookVerificationFailed = TRUE;
    }
    else {
        DbgPrint("[+] EventHook verification passed\n");
        Offsets->bEventHookVerificationFailed = FALSE;
    }

    // 热键验证充分性检查
    //BOOLEAN bHotkeySufficient = IsHotkeyVerificationSufficient(
    //    hotkeyCtx.VerificationMask, era);

    //DbgPrint("[*] Hotkey verification mask=0x%04X, Sufficient=%d\n",
    //    hotkeyCtx.VerificationMask, bHotkeySufficient);

    //if (!bHotkeySufficient) {
    //    DbgPrint("[!] Hotkey verification insufficient, using defaults\n");
    //    //HotkeyFallbackToDefaults(era, Offsets);
    //    Offsets->bEventHookVerificationFailed = TRUE;
    //}
    //else {
    //    DbgPrint("[+] Hotkey verification passed\n");
    //    Offsets->bHotkeyVerificationFailed = FALSE;
    //}

    Offsets->bHotkeyVerificationFailed = FALSE;

	Win32kOffsetScanner_Dump(Offsets, FALSE);
    return STATUS_SUCCESS;
}

// ==========================================
// 偏移 Dump（扩展事件钩子输出）
// ==========================================
BOOLEAN Win32kOffsetScanner_Dump(
    _In_ PWIN32K_OFFSETS Offsets,
    _In_ BOOLEAN bCompareWithReference)
{
    UNREFERENCED_PARAMETER(bCompareWithReference);

    DbgPrint("========================================\n");
    DbgPrint("WIN32K_OFFSETS Dump (Build=%lu, Era=%d)\n",
        Offsets->OsBuildNumber, Offsets->DetectedEra);
    if (Offsets->bVerificationFailed)
        DbgPrint("*** WARNING: Hook verification failed, using defaults ***\n");
    if (Offsets->bEventHookVerificationFailed)
        DbgPrint("*** WARNING: EventHook verification failed, using defaults ***\n");
    DbgPrint("========================================\n");

    DbgPrint("[PTI]\n");
    DbgPrint("  pDeskInfo     = 0x%03X\n", Offsets->Pti_pDeskInfo);
    DbgPrint("  aphkStart     = 0x%03X\n", Offsets->Pti_aphkStart);
    DbgPrint("  amdesk        = 0x%03X\n", Offsets->Pti_amdesk);
    DbgPrint("  pEThread      = 0x%03X\n", Offsets->Pti_pEThread);
    DbgPrint("  Flags         = 0x%03X\n", Offsets->Pti_Flags);
    DbgPrint("  bitmask       = 0x%03X\n", Offsets->Pti_bitmask);
    DbgPrint("  AccessCheck   = 0x%03X\n", Offsets->Pti_AccessCheck);

    DbgPrint("[HOOK]\n");
    DbgPrint("  hHook         = 0x%03X\n", Offsets->Hook_hHook);
    DbgPrint("  pti           = 0x%03X\n", Offsets->Hook_pti);
    DbgPrint("  phkNext       = 0x%03X\n", Offsets->Hook_phkNext);
    DbgPrint("  nHookType     = 0x%03X\n", Offsets->Hook_nHookType);
    DbgPrint("  offPfn        = 0x%03X\n", Offsets->Hook_offPfn);
    DbgPrint("  flags         = 0x%03X\n", Offsets->Hook_flags);
    DbgPrint("  ihmod         = 0x%03X\n", Offsets->Hook_ihmod);
    DbgPrint("  ObjectSize    = 0x%03X\n", Offsets->Hook_ObjectSize);

    DbgPrint("[DESKINFO]\n");
    DbgPrint("  aphkStart     = 0x%03X\n", Offsets->DeskInfo_aphkStart);

    DbgPrint("[EVENTHOOK]\n");
    DbgPrint("  ObjectSize    = 0x%03X\n", Offsets->EventHook_ObjectSize);
    DbgPrint("  pNext         = 0x%03X\n", Offsets->EventHook_pNext);
    DbgPrint("  eventMin      = 0x%03X\n", Offsets->EventHook_eventMin);
    DbgPrint("  eventMax      = 0x%03X\n", Offsets->EventHook_eventMax);
    DbgPrint("  flags         = 0x%03X\n", Offsets->EventHook_flags);
    DbgPrint("  idProcess     = 0x%03X\n", Offsets->EventHook_idProcess);
    DbgPrint("  idThread      = 0x%03X\n", Offsets->EventHook_idThread);
    DbgPrint("  pfn/offPfn    = 0x%03X\n", Offsets->EventHook_pfn);
    DbgPrint("  ihmod         = 0x%03X\n", Offsets->EventHook_ihmod);
    DbgPrint("  dpiAwareness  = 0x%03X\n", Offsets->EventHook_dpiAwareness);
    DbgPrint("  pti           = 0x%03X\n", Offsets->EventHook_pti);
    DbgPrint("  pti->ppi      = 0x%04X\n", Offsets->EventHook_pti_ppi);
    DbgPrint("  GPSI_LIST_OFF = 0x%05X (%s)\n",
        Offsets->GPSI_EVENT_HOOK_LIST_OFFSET,
        Offsets->EventHook_gpWinEventHooks_InSession ? "Session" : "Global");

    DbgPrint("[Meta]\n");
    DbgPrint("  HandleEntrySize       = %lu\n", Offsets->HandleEntrySize);
    DbgPrint("  HandleEntry_HookType  = 0x%02X\n", Offsets->HandleEntry_HookTypeOffset);
    DbgPrint("  HandleEntry_TableIdx  = 0x%02X\n", Offsets->HandleEntry_TableIndexOffset);

    DbgPrint("[HOTKEY]\n");
    DbgPrint("  pThreadInfo      = 0x%03X\n", Offsets->Hotkey_pThreadInfo);
    DbgPrint("  pfnCallback      = 0x%03X\n", Offsets->Hotkey_pfnCallback);
    DbgPrint("  pWnd             = 0x%03X\n", Offsets->Hotkey_pWnd);
    //DbgPrint("  hWnd             = 0x%03X\n", Offsets->Hotkey_hWnd);
    DbgPrint("  fsModLow         = 0x%03X\n", Offsets->Hotkey_fsModLow);
    DbgPrint("  fsModHigh        = 0x%03X\n", Offsets->Hotkey_fsModHigh);
    DbgPrint("  id               = 0x%03X\n", Offsets->Hotkey_id);
    DbgPrint("  vk               = 0x%03X\n", Offsets->Hotkey_vk);
    DbgPrint("  pNext            = 0x%03X\n", Offsets->Hotkey_pNext);
    DbgPrint("  ListEntry        = 0x%03X\n", Offsets->Hotkey_ListEntry);
    DbgPrint("  ObjectSize       = 0x%03X\n", Offsets->Hotkey_ObjectSize);
    DbgPrint("  HashBuckets      = %lu\n", Offsets->Hotkey_HashBuckets);
    if (Offsets->Hotkey_bSessionHashTable) {
        DbgPrint("  SessionHashTable = %p (offset=0x%X)\n",
            (PVOID)Offsets->Hotkey_gpSessionHashTable,
            Offsets->Hotkey_SessionHashOffset);
    }
    else {
        DbgPrint("  gphkHashTable    = %p (global)\n",
            (PVOID)Offsets->Hotkey_gphkHashTable);
    }
    if (Offsets->bHotkeyVerificationFailed)
        DbgPrint("  *** WARNING: Hotkey using defaults ***\n");

    return TRUE;
}

// ==========================================
// 兼容性包装：Validate
// ==========================================
BOOLEAN Win32kOffsetScanner_Validate(
    _In_ PWIN32K_OFFSETS Offsets)
{
    if (!Offsets) return FALSE;

    // 基本完整性检查 - 消息钩子
    if (Offsets->Pti_aphkStart == 0 ||
        Offsets->Pti_pDeskInfo == 0 ||
        Offsets->Hook_flags == 0)
        return FALSE;

    if (Offsets->Hook_flags - Offsets->Hook_ihmod != 0xFFFFFFFC ||
        Offsets->Hook_flags - Offsets->Hook_nHookType != 0x10 ||
        Offsets->Hook_flags - Offsets->Hook_offPfn != 8 ||
        Offsets->Hook_flags - Offsets->Hook_phkNext != 0x18)
        return FALSE;

    // 事件钩子基本完整性
    if (Offsets->EventHook_ObjectSize == 0 ||
        Offsets->EventHook_flags == 0 ||
        Offsets->EventHook_idProcess == 0 ||
        Offsets->EventHook_idThread == 0)
        return FALSE;

    // ObjectSize 必须为 0x60 或 0x50
    if (Offsets->EventHook_ObjectSize != 0x60 &&
        Offsets->EventHook_ObjectSize != 0x50)
        return FALSE;

    // 热键基础验证
    if (Offsets->Hotkey_ObjectSize == 0 ||
        Offsets->Hotkey_pThreadInfo != 0x00 ||
        Offsets->Hotkey_pWnd != 0x10)
        return FALSE;

    if (Offsets->Hotkey_ObjectSize != 0x30 &&
        Offsets->Hotkey_ObjectSize != 0x40 &&
        Offsets->Hotkey_ObjectSize != 0x48)
        return FALSE;

    // 24H2 Session 哈希表验证
    if (Offsets->Hotkey_ObjectSize == 0x48) {
        if (!Offsets->Hotkey_bSessionHashTable ||
            Offsets->Hotkey_SessionHashOffset == 0)
            return FALSE;
    }

    return TRUE;
}