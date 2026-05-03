#pragma once

// 🔴 第一步：必须先定义 Windows 版本宏 (适配 Win10+ / SDK 22621)
//#define NTDDI_VERSION    NTDDI_WIN10_19H1
//#define WINVER           0x0A00           // Win10
//#define _WIN32_WINNT     0x0A00           // Win10

// 🔴 第二步：必须定义 NDIS 6.x 宏 (WFP 强制依赖)
#define NDIS_SUPPORT_NDIS6  1             // 启用NDIS6核心类型

// 🔴 第三步：按规范顺序包含内核头文件
#include <ntifs.h>            // 内核基础头文件（必须第一）
#include <ndis.h>             // NDIS网络类型（fwpsk依赖，必须在fwpsk之前）
#include <ntstrsafe.h>        // 安全字符串函数
#include <fwpsk.h>    // ✅ 内核数据层：FWPS_CALLOUT0、FwpsCalloutEnumerateCallbacks0
#include <fwpmk.h>    // ✅ 内核管理层：FWPM_CALLOUT0、FwpmCalloutEnum0、FwpmEngineOpen0

#include "global.h"
#include "Module.h"
#include "OtherFunctions.h"
#include "ObjectInfo.h"

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

typedef struct _CallbackInfo
{
	wchar_t Type[50];
	PVOID Func;
	PVOID Context;
	ULONG64 Others[4]; // 预留字段，存储额外信息（如Ex2上下文等）
	//wchar_t DriverName[260];
}CallbackInfo, *PCallbackInfo;

typedef struct _CallbackTable
{
	ULONG Num;
	CallbackInfo Callbacks[1];
}CallbackTable, * PCallbackTable;

typedef struct _ObCallbackInfo
{
	wchar_t Type[50];
	HANDLE ObHandle;
	PVOID Func;
	//wchar_t DriverName[260];
}ObCallbackInfo, * PObCallbackInfo;

typedef struct _ObCallbackTable
{
	ULONG Num;
	ObCallbackInfo ObCallbacks[1];
}ObCallbackTable, * PObCallbackTable;

// 注册表回调函数结构体定义
typedef struct _CM_NOTIFY_ENTRY
{
	LIST_ENTRY  ListEntryHead;
	ULONG   UnKnown1;
	ULONG   UnKnown2;
	LARGE_INTEGER Cookie;
	PVOID   Context;
	PVOID   Function;
}CM_NOTIFY_ENTRY, * PCM_NOTIFY_ENTRY;

/*typedef enum _OB_CALLBACK_TYPE
{
	Process,
	Thread,
	DeskTop,
}OB_CALLBACK_TYPE;*/
//OB_CALLBACK_REGISTRATION a;
// 【正确】Windows 内核原生对象回调结构体（x64，Win7~Win11通用）
// 禁用1字节对齐！内核结构体必须自然对齐（8字节）
//typedef struct _OB_CALLBACK
//{
//	LIST_ENTRY          ListEntry;      // 0x00  双向链表节点
//	PVOID               Unknown;        // 0x10  保留字段
//	PVOID               RegistrationContext; // 0x18  ✅ 你要的【回调上下文】
//	HANDLE              ObHandle;       // 0x20  回调注册句柄
//	PVOID               ObTypeAddr;     // 0x28  对象类型地址(Process/Thread)
//	PVOID               PreCall;        // 0x30  前置回调函数
//	PVOID               PostCall;       // 0x38  后置回调函数
//} OB_CALLBACK, * POB_CALLBACK;

//#pragma pack(1)
//typedef struct _OB_CALLBACK
//{
//	LIST_ENTRY ListEntry;
//	ULONGLONG Unknown;
//	HANDLE ObHandle;
//	PVOID ObTypeAddr;
//	PVOID PreCall;
//	PVOID PostCall;
//}OB_CALLBACK, * POB_CALLBACK;
//#pragma pack()

//typedef struct _OB_CALLBACK
//{
//	LIST_ENTRY      ListEntry;
//	HANDLE          RegistrationHandle;
//	PVOID           RegistrationContext;
//	POBJECT_TYPE    ObjectType;
//	PVOID           PreOperation;
//	PVOID           PostOperation;
//	ULONG           Attributes;
//	ULONG           Operations;
//} OB_CALLBACK, * POB_CALLBACK;

typedef struct _OB_CALLBACK
{
	LIST_ENTRY      ListEntry;            // 0x00
	ULONG           Operations;           // 0x10
	ULONG           Padding0;             // 0x14 (对齐填充)
	PVOID           RegistrationHandle;   // 0x18 <-- 这里才是真正的 Handle
	POBJECT_TYPE    ObjectType;           // 0x20
	PVOID           PreOperation;         // 0x28
	PVOID           PostOperation;        // 0x30
	ULONG           Enabled;              // 0x38
	ULONG           Padding1;             // 0x3C
} OB_CALLBACK, * POB_CALLBACK;

// 严格对应你的 ObRegisterCallbacks 汇编
typedef struct _OB_REGISTRATION_BLOCK
{
	USHORT Version;        // 0x00 固定 0x100 (你汇编里 mov r12d, 100h)
	USHORT Reserved;       // 0x02
	ULONG  Flags;          // 0x04
	POBJECT_TYPE ObjectType;//0x08 进程/线程类型
	USHORT AltitudeLength;  // 0x10
	USHORT CallbackCount;  // 0x12 回调个数
	ULONG  Reserved2;      // 0x14
	POB_OPERATION_REGISTRATION Callbacks; // 0x18 回调数组（含RegistrationContext）
	WCHAR  Altitude[1];    // 0x20 高度字符串
} OB_REGISTRATION_BLOCK, * POB_REGISTRATION_BLOCK;

// 【完整真实结构】匹配内核反汇编，包含你要的 Context
typedef struct _EX_CALLBACK_ROUTINE_BLOCK
{
	EX_RUNDOWN_REF RundownProtect;  // 0x00  运行保护
	PVOID          Function;        // 0x08  回调函数（所有类型通用）
	ULONG          Flags;           // 0x10  类型标志（0=普通,2=Ex,6=Ex2）
	ULONG          _Padding;        // 0x14  对齐填充
	PVOID          Context;         // 0x18  ✅【关键】Ex/Ex2 专用上下文
} EX_CALLBACK_ROUTINE_BLOCK, * PEX_CALLBACK_ROUTINE_BLOCK;

// Windows 内核原生回调块结构体（x64）
//typedef struct _EX_CALLBACK_ROUTINE_BLOCK_EX
//{
//	PVOID  Function;    // 0x00  回调函数地址（你原本读的是对的）
//	PVOID  Context;     // 0x08  回调上下文
//	ULONG  Flags;       // 0x10  ✅ 核心：就是反汇编里的 a2（1=普通，2=Ex）
//	ULONG  Reserved;    // 0x14  内存对齐填充
//} EX_CALLBACK_ROUTINE_BLOCK_EX, * PEX_CALLBACK_ROUTINE_BLOCK_EX;

typedef struct _SHUTDOWN_PACKET {
	LIST_ENTRY ListEntry;
	PDEVICE_OBJECT DeviceObject;
} SHUTDOWN_PACKET, * PSHUTDOWN_PACKET;

typedef NTSTATUS(*PSE_LOGON_SESSION_TERMINATED_ROUTINE) (
	IN PLUID LogonId
	);

typedef struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION
{
	struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION* Next;
	PVOID CallbackRoutine;
} SEP_LOGON_SESSION_TERMINATED_NOTIFICATION, * PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION;

typedef struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION_EX
{
	struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION_EX* Next;
	PVOID CallbackRoutine;
	PVOID CallbackContext;
} SEP_LOGON_SESSION_TERMINATED_NOTIFICATION_EX, * PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION_EX;

typedef struct _NOTIFICATION_PACKET
{
	LIST_ENTRY ListEntry;
	PDRIVER_OBJECT DriverObject;
	PVOID NotificationRoutine;
} NOTIFICATION_PACKET, * PNOTIFICATION_PACKET;

// ============================================================================
// 结构体定义
// ============================================================================

// 严格匹配你 WinDbg 实测的内存偏移
typedef struct _PNP_NOTIFY_ENTRY {
	LIST_ENTRY      ListEntry;        // 0x00   (固定链表节点)
	ULONG64         Reserved0;        // 0x10   (保留/未知字段，必须占位)
	ULONG64         Reserved1;        // 0x18   (保留/未知字段，必须占位)
	PVOID           Callback;         // 0x20   ✅ 真实回调函数(你验证过的地址)
	PVOID           Context;          // 0x28   ✅ 真实回调上下文
	//PDRIVER_OBJECT  DriverObject;     // 0x30   (可选，驱动对象)
	//ULONG           RefCount;         // 0x38
	//BOOLEAN         Unregistered;     // 0x3C
	//UCHAR           Pad[3];           // 0x3D~0x3F 对齐
	//PFAST_MUTEX     Lock;             // 0x40
} PNP_NOTIFY_ENTRY, * PPNP_NOTIFY_ENTRY;

typedef struct _DEVICE_CLASS_NOTIFY_INFO {
	PLIST_ENTRY  Buckets;      // 桶数组基址
	ULONG        NumBuckets;   // 固定 13
	PFAST_MUTEX  Lock;
} DEVICE_CLASS_NOTIFY_INFO, * PDEVICE_CLASS_NOTIFY_INFO;

typedef struct _HWPROFILE_NOTIFY_INFO {
	PLIST_ENTRY  ListHead;
	PFAST_MUTEX  Lock;
} HWPROFILE_NOTIFY_INFO, * PHWPROFILE_NOTIFY_INFO;
#define CALCULATE_RIP_TARGET(pInstruction, instructionLength) \
    ((PVOID)((ULONG_PTR)(pInstruction) + (instructionLength) + *(PLONG)((PUCHAR)(pInstruction) + 3)))

typedef struct _CALLBACK_OBJECT {
	ULONG Tag;                       // 偏移0: 签名标识，例如0x6C6C6143 ('llaC')
	KSPIN_LOCK SpinLock;             // 偏移8: 自旋锁，用于同步操作（代码中`(PKSPIN_LOCK)CallbackObject + 1`对应此字段）
	LIST_ENTRY ListEntry;            // 偏移16: 链表头，用于连接已注册的CALLBACK_REGISTRATION条目（代码中`v10 = (_QWORD *)((char *)CallbackObject + 16)`）
	BOOLEAN AllowMultipleCallbacks;  // 偏移32: 布尔值，控制是否允许多个回调注册（代码中`v15[32] = AllowMultipleCallbacks`）
	UCHAR Reserved[7];               // 填充字节，用于对齐（根据代码偏移推算）
	LIST_ENTRY ListEntry2;           // 偏移40: 第二个链表条目，用于将回调对象链接到全局列表`ExpCallbackListHead`（代码中`*((_QWORD *)v15 + 3) = v15 + 40`）
} CALLBACK_OBJECT, * PCALLBACK_OBJECT;

typedef struct _CALLBACK_REGISTRATION {
	LIST_ENTRY ListEntry;           // 偏移0: 链表条目，用于链接到CALLBACK_OBJECT的ListEntry链表（代码中`*v7 = v10`和`v7[1] = v12`操作此字段）
	PCALLBACK_OBJECT CallbackObject; // 偏移16: 指向所属回调对象的指针（代码中`*(_QWORD *)(Pool2 + 16) = CallbackObject`）
	PCALLBACK_FUNCTION CallbackFunction; // 偏移24: 回调函数指针（代码中`*(_QWORD *)(Pool2 + 24) = CallbackFunction`）
	PVOID CallbackContext;           // 偏移32: 回调上下文数据，传递给回调函数（代码中`*(_QWORD *)(Pool2 + 32) = CallbackContext`）
	ULONG Unknown1;                  // 偏移40: 未知字段（可能为引用计数或状态），代码中初始化为0
	UCHAR Unknown2;                  // 偏移44: 未知标志，代码中初始化为0
} CALLBACK_REGISTRATION, * PCALLBACK_REGISTRATION;

// 导出函数原型
// NMI 回调函数原型（匹配内核）
typedef VOID(*PKNCALLBACK)(PVOID Context, ULONG Type);

// NMI 回调节点结构（严格从反汇编还原，偏移完全匹配）
typedef struct _KI_NMI_CALLBACK_ENTRY
{
	struct _KI_NMI_CALLBACK_ENTRY* Next;    // 0x00  单链表指针
	PKNCALLBACK                       Callback;  // 0x08  回调函数
	PVOID                             Context;   // 0x10  回调上下文
	struct _KI_NMI_CALLBACK_ENTRY* Self;    // 0x18  自引用
} KI_NMI_CALLBACK_ENTRY, * PKI_NMI_CALLBACK_ENTRY;

// 🔥 修复：DbgPrint 回调正确结构体（x64）
/*typedef struct _DBG_PRINT_CALLBACK_ENTRY
{
	LIST_ENTRY             ListEntry;        // 0x00
	PDEBUG_PRINT_CALLBACK  CallbackFunction; // 0x10 【核心修复】
	PVOID                  CallbackContext;  // 0x18
	ULONG                  Flags;            // 0x20
} DBG_PRINT_CALLBACK_ENTRY, * PDBG_PRINT_CALLBACK_ENTRY;*/

// 存储链表头+自旋锁
typedef struct _DBG_PRINT_INFO
{
	PLIST_ENTRY     CallbackList;
	PEX_SPIN_LOCK   CallbackLock;
} DBG_PRINT_INFO, * PDBG_PRINT_INFO;

#pragma pack(1)
typedef struct _MINIFILTER_MAJORFUNCTION
{
	PVOID PreOperation;
	PVOID PostOperation;
} MINIFILTER_MAJORFUNCTION, * PMINIFILTER_MAJORFUNCTION;

typedef struct _MINIFILTER_OBJECT
{
	PFLT_FILTER hFilter;
	WCHAR Altitude[10];
	WCHAR Name[260];
	WCHAR Path[260];
	PVOID FilterFunc[11];
	BOOLEAN bMajorFunction;
	MINIFILTER_MAJORFUNCTION MajorFunction[FLT_INTERNAL_OPERATION_COUNT + 1];
} MINIFILTER_OBJECT, * PMINIFILTER_OBJECT;
#pragma pack()

// 单个回调条目
typedef struct _CALLBACK_ENTRY {
	PVOID  CallbackObject;      // 回调对象指针
	WCHAR  Name[256];           // 名称缓存（固定大小简化设计）
} CALLBACK_ENTRY, * PCALLBACK_ENTRY;

// 全局查找表（非分页内存）
typedef struct _CALLBACK_LOOKUP_TABLE {
	PCALLBACK_ENTRY Entries;    // 动态数组
	ULONG           Count;      // 当前条目数
	ULONG           Capacity;   // 数组容量
	BOOLEAN         Initialized; // 初始化标志
	KSPIN_LOCK      Lock;       // 自旋锁（保护写入）
} CALLBACK_LOOKUP_TABLE, * PCALLBACK_LOOKUP_TABLE;

// Callout 信息结构
typedef struct _WFP_CALLOUT_INFO {
	GUID    CalloutGuid;        // Callout 的 GUID
	UINT32  CalloutId;          // 运行时 Callout ID（索引）
	PVOID   ClassifyFn;         // classify 回调地址
	PVOID   NotifyFn;           // notify 回调地址
	PVOID   FlowDeleteFn;       // flowDelete 回调地址
	WCHAR   Name[256];          // Callout 名称（可选，FWPM_CALLOUT 中可能有）
} WFP_CALLOUT_INFO, * PWFP_CALLOUT_INFO;

// Filter 信息结构
typedef struct _WFP_FILTER_INFO {
	GUID    FilterKey;          // Filter 的 GUID
	UINT64  FilterId;           // Filter ID
	UINT32  ActionType;         // Filter 动作类型
	GUID    CalloutGuid;        // 关联的 Callout GUID
	UINT32  CalloutId;          // 关联的 Callout ID（通过 GUID 查表得到）
	WCHAR   Name[256];          // Filter 名称
} WFP_FILTER_INFO, * PWFP_FILTER_INFO;

NTSTATUS ControlCallback(PVOID pCallbackFunc, PUCHAR OldCode, BOOLEAN Status);
NTSTATUS DeleteCallback(PCallbackInfo pCallbackInfo);
ULONG EnumMiniFilter(PMINIFILTER_OBJECT* Array, PULONG InOutCount);

ULONG EnumCallbacks(PCallbackInfo* pArray);

NTSTATUS RemoveCreateProcessNotifyRoutine(PVOID CreateProcessNotifyRoutine);

NTSTATUS RemoveCreateThreadNotifyRoutine(PVOID CreateThreadNotifyRoutine);

NTSTATUS UnregisterCmpCallback(LARGE_INTEGER Cookie);

NTSTATUS RemoveLoadImageNotifyRoutine(PVOID LoadImageNotifyRoutine);

VOID UnregisterObCallback(PVOID ObHandle);

NTSTATUS EnumWfpCallouts(
	__out PWFP_CALLOUT_INFO* ppCalloutArray,
	__out ULONG* pCount
);
NTSTATUS EnumWfpFilters(
	__out PWFP_FILTER_INFO* ppFilterArray,
	__out ULONG* pCount
);

//extern PLIST_ENTRY ExpCallbackListHead;
//extern PKSPIN_LOCK ExpCallbackListLock;

extern CALLBACK_LOOKUP_TABLE g_CallbackTable;