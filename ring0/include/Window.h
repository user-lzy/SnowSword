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

// 从反汇编逆向的Win11 x64 Win32k内部EVENTHOOK结构（大小0x50字节）
typedef struct _INTERNAL_EVENT_HOOK
{
    // 0x00: HM对象头开始
    HANDLE                      HookHandle;         // 0x00: 钩子句柄 (你的输出: 0x60FD9)
    ULONGLONG                   Reserved1;          // 0x08: 未知 (全0)
    PETHREAD*                   pInstallerThread;   // 0x10: 安装钩子的线程 PETHREAD 指针
    // 0x18: 链表和事件信息
    struct _INTERNAL_EVENT_HOOK* pNext;             // 0x18: 下一个钩子
    DWORD                       EventMin;           // 0x20: 最小事件ID (0x8000)
    DWORD                       EventMax;           // 0x24: 最大事件ID (0x8001)
    DWORD                       Flags;              // 0x28: 标志位 (0x0)
    DWORD                       ReservedPad1;       // 0x2C: 对齐填充
    ULONGLONG                   TargetProcessId;    // 0x30: 目标进程ID (0)
    ULONGLONG                   TargetThreadId;     // 0x38: 目标线程ID (0)
    PVOID                       pfnWinEventProc;    // 0x40: 回调函数指针 (你的输出: 0x00007FF7242813B1)
	ULONGLONG                   Unknown48;          // 0x48: 未知 (0x00006010FFFFFFFF)()可能是hmodWinEventProc的64位版本，或者其他数据
} INTERNAL_EVENT_HOOK, * PINTERNAL_EVENT_HOOK;

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
#define WIN11_TI_THREAD_HOOK_ARRAY      0x3C8   // xxxCallHook 验证了这个是对的

// 全局钩子容器内数组基址
#define GLOBAL_HOOK_ARRAY_BASE          0x30

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

#define WIN10_1703_BUILD_NUMBER 0x3AD7

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

typedef struct _HEAD {
    PVOID unk[3];
    PVOID threadInfo;
}HEAD;

// win32k!tagTHREADINFO 结构体（文章中直接给出）
typedef struct _tagTHREADINFO {
    PETHREAD    pEThread;       // +0x00 【第一个成员】→ 直接指向 NT 内核线程对象
    ULONG       RefCount;       // +0x08
    PVOID64     ptlW32;         // +0x10
    PVOID64     pgdiDcattr;     // +0x18
    // ... 其他成员
} tagTHREADINFO, * PtagTHREADINFO;

typedef struct _tagHOTKEY {
    PVOID64       pThreadInfo;      // +0x00
    PVOID64       pCallback;        // +0x08
    _HWND         hWnd;             // +0x10
    WORD          fsModifiers;      // +0x18
    WORD          flags;            // +0x1A
    DWORD         vk;               // +0x1C
    DWORD         id;               // +0x20
    struct _tagHOTKEY* pNext;       // +0x28
    // 以下为 +0x30 开始的双向链表头，枚举时无需关心
    PVOID64       pChildListHead;   // +0x30
    PVOID64       pChildListTail;   // +0x38
} tagHOTKEY, * PTAG_HOTKEY;

// 热键哈希表结构（0x80个桶，固定大小）
typedef struct _HOTKEY_HASH_TABLE
{
    PTAG_HOTKEY   Buckets[0x80];    // 128个哈希桶，拉链法存储
} HOTKEY_HASH_TABLE, * PHOTKEY_HASH_TABLE;

// ==============================================
// 【修正】Win11 tagHOTKEY 结构体（匹配IDA反汇编）
// ==============================================
typedef struct _tagHOTKEY_WIN11 {
    PVOID64  pThreadInfo;   // 0x00
    PVOID64  pCallback;     // 0x08
    PVOID64  pWnd;          // 0x10
    PVOID64  hWnd;          // 0x18
    WORD     fsModifiers;   // 0x20  <-- 对应 w20（真实 mod，如 0x000a=Win+Ctrl）
    WORD     unk_22;        // 0x22  <-- 对应 w22（扩展标志，暂不处理）
    DWORD    vk;            // 0x24  <-- 对应 d24（真实 vk，如 0x7d=F14）
    PVOID64  id;            // 0x28  <-- 对应 q28（热键 ID）
    PVOID64  pNext;         // 0x30
    LIST_ENTRY HashLink;    // 0x38
} tagHOTKEY_WIN11, * PTAG_HOTKEY_WIN11;

// 哈希表结构（128个桶，匹配id&0x7F的索引计算）
typedef struct _HOTKEY_HASH_TABLE_WIN11
{
    PTAG_HOTKEY_WIN11 Buckets[0x80];
} HOTKEY_HASH_TABLE_WIN11, * PHOTKEY_HASH_TABLE_WIN11;

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

// 统一的定时器条目结构（Win10/Win11通用布局）
typedef struct _TIMER_ENTRY_WIN10 {
    // 0x00-0x17: 保留字段
    //UCHAR Reserved1[0x18];

    // 0x18: tagPROCESSINFO (Win11) 或 EPROCESS (Win10)
    //PVOID Process;

    HEAD head;

    // 0x20: 回调函数地址
    WNDPROC pfn;

    // 0x28: 超时时间(ms)
    ULONG nTimeout;

    // 0x2C: 合并超时时间
    ULONG nTimeoutCoalesced;

    // 0x30: 标志位 (0x1000=待删除, 0x40=TIF_SYSTEM_TIMER)
    ULONG flags;

    // 0x34: 超时时间副本
    ULONG nTimeout2;

    // 0x38-0x47: 保留
    UCHAR Reserved2[0x10];

    // 0x48: 全局链表 (gtmrListHead)
    LIST_ENTRY TmrListEntry;

    // 0x58: 窗口对象 (tagWND*)
    PVOID windowPtr;

    // 0x60: 定时器ID
    ULONG64 nIDEvent;

    // 0x68: 回调参数
    ULONG_PTR param;

    // 0x70: 哈希表链表 (gTimerHashTable)
    LIST_ENTRY HashListEntry;
} TIMER_ENTRY_WIN10, * PTIMER_ENTRY_WIN10;

typedef struct _TIMER_ENTRY_WIN11 {
    UCHAR Reserved1[0x18];        // 0x00-0x17: 保留
    PVOID threadInfo;              // 0x18: tagTHREADINFO*
    WNDPROC pfn;                   // 0x20: 回调函数
    ULONG nTimeout;                // 0x28: 超时时间
    ULONG Reserved2;               // 0x2C
    ULONG flags;                   // 0x30: 标志位 (0x1000=待删除)
    ULONG nTimeout2;               // 0x34: 超时时间副本
    UCHAR Reserved3[0x10];         // 0x38-0x47
    LIST_ENTRY TmrListEntry;       // 0x48-0x57: 全局链表
    PVOID windowPtr;                // 0x58: tagWND*
    UCHAR Reserved4[0x10];         // 0x60-0x6F
    ULONG64 nIDEvent;               // 0x70: 定时器ID
    LIST_ENTRY HashListEntry;       // 0x78-0x87: 哈希表链表 (修正偏移！)
} TIMER_ENTRY_WIN11, * PTIMER_ENTRY_WIN11;

// 联合体：同时容纳 Win10/Win11 结构体
typedef union _TIMER_ENTRY {
    PTIMER_ENTRY_WIN10 Win10;
    PTIMER_ENTRY_WIN11 Win11;
} TIMER_ENTRY, * PTIMER_ENTRY;

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
//typedef PHOOK(*PFN_HMValidateHandle)(HANDLE hHandle, HANDLE_TYPE hType);
//typedef PHOOK(*PFN_HMValidateHandleWithDescriptor)(HANDLE hHandle, BYTE DescriptorType);
//typedef PHOOK(*PFN_PhkFirstGlobalValid)(PTHREADINFO pti, int HookType);
//typedef PHOOK(*PFN_PhkNextValid)(PHOOK Current);

// 枚举上下文（运行时自动检测版本）
typedef struct _TIMER_ENUM_CONTEXT {
    BOOLEAN isV2;                   // TRUE=Win11, FALSE=Win10

    // 动态获取的函数地址
    union {
        PFN_W32GetUserSessionState W32GetUserSessionState;  // Win11
        PVOID gTimerHashTable;            // Win10 备用
    };

    // 函数指针（通用）
    PFN_EnterCrit EnterCrit;
    PFN_UserSessionSwitchLeaveCrit UserSessionSwitchLeaveCrit;
    PFN_ValidateHwnd ValidateHwnd;
    //PVOID PsGetCurrentProcessWin32Process;
} TIMER_ENUM_CONTEXT, * PTIMER_ENUM_CONTEXT;

typedef struct _WINDOW_TIMER {
    WNDPROC pfn;               // 回调函数地址
    ULONG nTimeout;            // 超时时间(ms)
    ULONG64 nIDEvent;         // 定时器ID
	HANDLE ThreadId;           // 线程ID
    HANDLE ProcessId;          // 进程ID
    PVOID hWnd;                // 所属窗口
    ULONG_PTR param;          // 回调参数
} WINDOW_TIMER, * PWINDOW_TIMER;

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

NTSTATUS
EnumHotkey(
    OUT PWIN32K_HOTKEY_INFO* ppHotkeyList,
    OUT PULONG pulHotkeyCount
);