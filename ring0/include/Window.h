#pragma once
#include "ntifs.h"
#include "global.h"

typedef PVOID _HWND;
typedef PVOID PWND;
typedef PVOID WNDPROC;
typedef PVOID PRTL_USER_PROCESS_PARAMETERS;
typedef PVOID PPS_POST_PROCESS_INIT_ROUTINE;
typedef USHORT ATOM;

// 钩子类型总数（固定32种）
#define MAX_HOOK 32

// 对齐反编译：qword_140015660 = gSharedInfo
typedef struct _WIN32K_GSHAREDINFO
{
    PVOID64 pHookArray;     // *v2 -> 钩子数组基址
    PVOID64 pHookMetadata;  // v2[1] -> 钩子元数据
} WIN32K_GSHAREDINFO, * PWIN32K_GSHAREDINFO;

// ====================== 消息钩子 内核结构体 ======================
typedef struct _WIN32K_MSG_HOOK_INFO
{
    HANDLE        HookHandle;          // 钩子句柄
    ULONG         HookType;        // 具体子类型 (WH_KEYBOARD等)
    ULONG         HookFlags;           // 钩子标志
    BOOLEAN       IsGlobal;            // 是否全局钩子
    ULONG64       HookProc;            // 回调函数地址
    WCHAR         ModulePath[260];     // 模块路径
    ULONG         ProcessId;           // 进程ID
    ULONG         ThreadId;            // 线程ID
} WIN32K_MSG_HOOK_INFO, * PWIN32K_MSG_HOOK_INFO;

// Win11 全局事件钩子链表头偏移（从SetWinEventHook反汇编逆向）
#define WIN11_GPSI_EVENT_HOOK_LIST_OFFSET 0x11380

// 内存池标签（倒序，便于调试）
#define EVENT_HOOK_POOL_TAG 'kooh'

// 自动8字节对齐，兼容x64内核
typedef struct _WIN32K_EVENT_HOOK_INFO
{
    HANDLE        HookHandle;          // 钩子句柄
    DWORD         EventMin;            // 最小事件ID
    DWORD         EventMax;            // 最大事件ID
    PVOID         hmodWinEventProc;    // 模块句柄/ihmod索引
    PVOID         pfnWinEventProc;     // 回调函数指针
    PVOID         ActualCallback;      // 解析后的真实回调地址
    WCHAR         ModulePath[260];     // 模块完整路径/名称
    ULONG         TargetProcessId;     // 目标进程ID
    ULONG         TargetThreadId;      // 目标线程ID
    ULONG         ProcessId;           // 所属进程ID
    ULONG         ThreadId;            // 所属线程ID
    DWORD         Flags;               // 钩子标志
} WIN32K_EVENT_HOOK_INFO, * PWIN32K_EVENT_HOOK_INFO;

// 内部结构体前置声明
typedef struct _tagTHREADINFO* PTAG_THREADINFO;
typedef struct _tagDESKTOPINFO* PTAG_DESKTOPINFO;
typedef struct _tagHOOK* PTAG_HOOK;

// 64位Windows tagHOOK 关键成员偏移（Win10 1703+，新版）
#define HOOK_OFFSET_PTIHOOKED_NEW   0x28    // 不变
#define HOOK_OFFSET_RPDESK_NEW      0x50    // ✅ 修正：不再和Type冲突
#define HOOK_OFFSET_HOOKPROC_NEW    0x38    // 不变
// 新版：Win10 1703+
#define HOOK_OFFSET_TYPE_NEW     0x30    // ✅ 正确（调试验证）
#define HOOK_OFFSET_FLAGS_NEW    0x40    // ✅ 核心修正：真正的Flags偏移！

// Win10 1703之前 旧版系统偏移
#define HOOK_OFFSET_PTIHOOKED_OLD   0x24
#define HOOK_OFFSET_RPDESK_OLD      0x2C
#define HOOK_OFFSET_HOOKPROC_OLD    0x34
// 旧版：Win7/Win8/Win10 15063-
#define HOOK_OFFSET_TYPE_OLD     0x2C
#define HOOK_OFFSET_FLAGS_OLD    0x38    // ✅ 修正：旧版Flags偏移

//-----------------------------

// 【修正版】Win11 22H2+ 关键偏移
#define MAX_HOOK_TYPES              16

// tagTHREADINFO 偏移
#define WIN11_TI_GLOBAL_HOOK_CONTAINER  0x1F8   // 你的 Dump 验证了这个是对的
#define WIN11_TI_THREAD_HOOK_ARRAY      0x3C0   // xxxCallHook 验证了这个是对的

// 全局钩子容器内数组基址
#define GLOBAL_HOOK_ARRAY_BASE          0x28

// 【重新校准】tagHOOK 偏移 (极简版，先保证能跑通)
// 注意：这里的偏移需要你根据实际 WinDbg 结果微调，
// 但核心逻辑是：数组里拿出来的直接就是 tagHOOK*，不需要 CONTAINING_RECORD
#define WIN11_HOOK_OFFSET_HHOOK         0x00    // 确认！
#define WIN11_HOOK_OFFSET_PETHREAD_PTR  0x10    // 指向ETHREAD的二级指针，已验证
#define WIN11_HOOK_OFFSET_NEXT_HOOK     0x28    // 单向链表next指针（全局/低级钩子用）
#define WIN11_HOOK_OFFSET_TYPE          0x30   // 先沿用你原来的，用 Windbg 确认
#define WIN11_HOOK_OFFSET_HOOKPROC      0x38
#define WIN11_HOOK_OFFSET_FLAGS         0x40
#define WIN11_HOOK_OFFSET_IHMOD         0x44    // ihmod（4字节）
#define WIN11_HOOK_OFFSET_PTIOWNER      0x48    // ptiOwner（8字节）

#define HF_GLOBAL                       1
#define HOOK_DEBUG_PRINT_MEMORY         0

// 事件钩子内核对象偏移（64位，从伪代码反推，Win10全版本通用）
#define EVENTHOOK_OFFSET_THREAD        0x10  // PETHREAD* 线程对象指针
#define EVENTHOOK_OFFSET_EVENTMIN      0x20  // DWORD  eventMin
#define EVENTHOOK_OFFSET_EVENTMAX      0x24  // DWORD  eventMax
#define EVENTHOOK_OFFSET_FLAGS         0x28  // DWORD  dwFlags
#define EVENTHOOK_OFFSET_TARGET_PID    0x30  // DWORD  idProcess
#define EVENTHOOK_OFFSET_TARGET_TID    0x34  // DWORD  idThread
#define EVENTHOOK_OFFSET_PFN           0x40  // HMODULE hmodWinEventProc
#define EVENTHOOK_OFFSET_HMOD          0x48  // PVOID   pfnWinEventProc

typedef enum _HANDLE_TYPE {
    TYPE_FREE = 0,
    TYPE_WINDOW = 1,
    TYPE_MENU = 2,
    TYPE_CURSOR = 3,
    TYPE_SETWINDOWPOS = 4,
    TYPE_HOOK = 5,
    TYPE_CLIPDATA = 6,
    TYPE_CALLPROC = 7,
    TYPE_ACCELTABLE = 8,
    TYPE_DDEACCESS = 9,
    TYPE_DDECONV = 10,
    TYPE_DDEXACT = 11,
    TYPE_MONITOR = 12,
    TYPE_KBDLAYOUT = 13,
    TYPE_KBDFILE = 14,
    TYPE_WINEVENTHOOK = 15,
    TYPE_TIMER = 16,
    TYPE_INPUTCONTEXT = 17,
    TYPE_HIDDATA = 18,
    TYPE_DEVICEINFO = 19,
    TYPE_TOUCHINPUT = 20,
    TYPE_GESTUREINFO = 21,
    TYPE_CTYPES = 22,
    TYPE_GENERIC = 255
}HANDLE_TYPE, *PHANDLE_TYPE;

// 池标签（和事件钩子风格统一）
#define HOTKEY_POOL_TAG 'ktHo'

// 热键哈希表桶数量（固定 128）
#define HOTKEY_HASH_COUNT 0x80

typedef struct _WIN32K_HOTKEY_INFO
{
    HANDLE        HotkeyHandle;        // 热键句柄
    DWORD         vk;                  // 虚拟键码
    DWORD         mod;                 // 功能键
    DWORD         id;                  // 热键id
    PVOID         hWnd;                // 窗口句柄
    ULONG         ProcessId;           // 进程ID
    ULONG         ThreadId;            // 线程ID
	PVOID         pfnCallback;        // 回调函数地址
} WIN32K_HOTKEY_INFO, * PWIN32K_HOTKEY_INFO;

typedef struct _PEB_LDR_DATA {
    ULONG Length;                           // +0x00 结构体长度
    BOOLEAN Initialized;                    // +0x04 是否已初始化
    PVOID SsHandle;                         // +0x08 会话句柄
    LIST_ENTRY InLoadOrderModuleList;       // +0x10 按加载顺序的模块链表
    LIST_ENTRY InMemoryOrderModuleList;     // +0x20 按内存顺序的模块链表
    LIST_ENTRY InInitializationOrderModuleList; // +0x30 按初始化顺序的模块链表
    PVOID EntryInProgress;                  // +0x40 当前正在加载的条目
    UCHAR ShutdownInProgress;               // +0x48 是否正在关闭
    PVOID ShutdownThreadId;                 // +0x50 关闭线程ID
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
    BYTE                          Reserved1[2];
    BYTE                          BeingDebugged; //被调试状态
    BYTE                          Reserved2[1];
    PVOID                         Reserved3[2];
    PPEB_LDR_DATA                 Ldr;
    PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
    BYTE                          Reserved4[104];
    PVOID                         Reserved5[52];
    PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
    BYTE                          Reserved6[128];
    PVOID                         Reserved7[1];
    ULONG                         SessionId;
} PEB, * PPEB;

// 函数指针类型定义
typedef VOID(*PFN_EnterCrit)(PVOID, PVOID);
typedef VOID(*PFN_UserSessionSwitchLeaveCrit)(VOID);
typedef PWND(*PFN_ValidateHwnd)(HWND);
typedef PVOID(*PFN_W32GetUserSessionState)(VOID);
typedef NTSTATUS(*PFN_RtlQueryAtomInAtomTable)(PVOID AtomTable, ATOM Atom, PULONG RefCount, PULONG PinCount, PWSTR AtomName, PULONG NameLength);

typedef struct _WINDOW_TIMER {
    WNDPROC pfn;               // 回调函数地址
    ULONG nTimeout;            // 超时时间(ms)
    ULONG64 nIDEvent;         // 定时器ID
	HANDLE ThreadId;           // 线程ID
    HANDLE ProcessId;          // 进程ID
    PVOID hWnd;                // 所属窗口
    ULONG_PTR param;          // 回调参数
} WINDOW_TIMER, * PWINDOW_TIMER;

// 签名结构体 - 统一描述各版本 Timer 对象布局
typedef struct _TIMER_ERA_SIGNATURE {
    // === Timer 对象字段偏移 (相对对象基址) ===
    ULONG Timer_pThreadInfo;        // PTHREADINFO (pti) 指针偏移
    ULONG Timer_pfn;                // 回调函数指针偏移
    ULONG Timer_nTimeout;           // 超时时间偏移
    ULONG Timer_nIDEvent;           // 事件ID偏移
    ULONG Timer_flags;              // 标志位偏移
    ULONG Timer_nTimeoutDup;        // 超时时间副本偏移
    ULONG Timer_windowPtr;          // 窗口指针偏移 (tagWND*)
    ULONG Timer_nIDEventDup;        // 事件ID副本偏移
    ULONG Timer_TimeStamp;          // 时间戳偏移
    ULONG Timer_HashListEntry;      // 哈希桶链表节点 LIST_ENTRY 偏移
    ULONG Timer_ObjectSize;         // HMAllocObject 分配大小

    // === 哈希表配置 ===
    ULONG Timer_HashBuckets;        // 哈希桶数量 (通常为64)

    // === 哈希表获取方式 (二选一) ===
    BOOLEAN bUseSessionHashTable;   // TRUE = W32GetUserSessionState + 偏移
    ULONG SessionHashOffset;        // 相对 W32GetUserSessionState 的偏移
    PCSTR HashTableSymbol;          // NULL = 使用 SessionHashOffset

    // === PTHREADINFO 内部偏移 ===
    ULONG Pti_ProcessOffset;        // pti 结构体内进程指针偏移
    ULONG Pti_EthreadOffset;        // pti 结构体内 ETHREAD 指针偏移 (0 = 首成员)

    // === 标志位定义 ===
    ULONG Flag_InUse;               // 条目使用中标志 (bit 0 = 0x1)
    ULONG Flag_Deleted;             // 已删除标志

    // === 版本范围 ===
    ULONG OsBuildMin;               // 最小支持构建号
    ULONG OsBuildMax;               // 最大支持构建号 (0xFFFFFFFF = 无上限)

    // === 元数据 ===
    PCSTR EraName;                  // 版本名称
    PCSTR EnterCritSymbol;          // EnterCrit 符号名
    PCSTR LeaveCritSymbol;          // LeaveCrit 符号名
} TIMER_ERA_SIGNATURE, * PTIMER_ERA_SIGNATURE;
typedef const TIMER_ERA_SIGNATURE* PCTIMER_ERA_SIGNATURE;

typedef struct _TIMER_CONTEXT {
    PFN_EnterCrit EnterCrit;
    PFN_UserSessionSwitchLeaveCrit UserSessionSwitchLeaveCrit;
    PFN_ValidateHwnd ValidateHwnd;

    const TIMER_ERA_SIGNATURE* pSig;    // 当前匹配的签名
    PVOID HashTableBase;                // 哈希表基址

    // Win11 24H2+ 专用
    PFN_W32GetUserSessionState W32GetUserSessionState;
} TIMER_CONTEXT, * PTIMER_CONTEXT;

NTSTATUS EnumProcessTimers(
    _Out_ PWINDOW_TIMER* pArray,
    _Out_ PULONG pCount
);

NTSTATUS
EnumMsgHook(
    OUT PWIN32K_MSG_HOOK_INFO* ppHookList,
    OUT PULONG                   pulHookCount
);

// 函数声明：枚举钩子并返回结构体数组
NTSTATUS
EnumEventHook(
    OUT PWIN32K_EVENT_HOOK_INFO* ppHookList,    // 返回钩子数组
    OUT PULONG                       pulHookCount    // 返回钩子数量
);

NTSTATUS EnumHotkey(
    _Out_ PWIN32K_HOTKEY_INFO* ppHotkeyList,
    _Out_ PULONG pulHotkeyCount
);