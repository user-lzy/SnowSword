#pragma once
#include "Module.h"
#include "OtherFunctions.h"
#include "ObjectInfo.h"
#include "ntstrsafe.h"
#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>

typedef struct _CallbackInfo
{
	wchar_t Type[50];
	PVOID Func;
	PVOID Context;
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

// 进线程对象回调函数结构体定义
#pragma pack(1)
typedef struct _OB_CALLBACK
{
	LIST_ENTRY ListEntry;
	ULONGLONG Unknown;
	HANDLE ObHandle;
	PVOID ObTypeAddr;
	PVOID PreCall;
	PVOID PostCall;
}OB_CALLBACK, * POB_CALLBACK;
#pragma pack()

typedef struct _SHUTDOWN_PACKET {
	LIST_ENTRY ListEntry;
	PDEVICE_OBJECT DeviceObject;
} SHUTDOWN_PACKET, * PSHUTDOWN_PACKET;

typedef NTSTATUS(*PSE_LOGON_SESSION_TERMINATED_ROUTINE) (
	IN PLUID LogonId
	);

typedef struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION {
	struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION* Next;
	PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine;
} SEP_LOGON_SESSION_TERMINATED_NOTIFICATION, * PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION;

typedef struct _NOTIFICATION_PACKET
{
	LIST_ENTRY ListEntry;
	PDRIVER_OBJECT DriverObject;
	PVOID NotificationRoutine;
} NOTIFICATION_PACKET, * PNOTIFICATION_PACKET;

typedef struct _SETUP_NOTIFY_DATA
{
	LIST_ENTRY ListEntry;
	IO_NOTIFICATION_EVENT_CATEGORY EventCategory;
	PDRIVER_NOTIFICATION_CALLBACK_ROUTINE Callback;
	PVOID Context;
	PDRIVER_OBJECT DriverObject;
	USHORT RefCount;
	BOOLEAN Unregistered;
	PFAST_MUTEX Lock;
} SETUP_NOTIFY_DATA, * PSETUP_NOTIFY_DATA;

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

PVOID FindPspCreateProcessNotifyRoutine();

PVOID FindPspCreateThreadNotifyRoutine();

PVOID FindPspLoadImageNotifyRoutine();

PVOID FindCmCallbackListHead();

NTSTATUS ControlCallback(PVOID pCallbackFunc, PUCHAR OldCode, BOOLEAN Status);

VOID EnumAttachDevice();

ULONG EnumMiniFilter(PMINIFILTER_OBJECT pArray);

PVOID FindKeBugCheckCallbackListHead();

PVOID FindKeBugCheckReasonCallbackListHead();

PVOID FindPspLegoNotifyRoutine();

VOID FindIopNotifyShutdownQueueHead(PVOID* pIopNotifyShutdownQueueHead, PVOID* pIopNotifyLastChanceShutdownQueueHead);

PVOID FindLogonSessionTerminatedRoutinueHead();

PVOID FindPnpDeviceClassNotifyList();

PVOID FindIopFsNotifyChangeQueueHead();

ULONG EnumCallbacks(PCallbackInfo* pArray);

NTSTATUS RemoveCreateProcessNotifyRoutine(PVOID CreateProcessNotifyRoutine);

NTSTATUS RemoveCreateThreadNotifyRoutine(PVOID CreateThreadNotifyRoutine);

NTSTATUS UnregisterCmpCallback(LARGE_INTEGER Cookie);

NTSTATUS RemoveLoadImageNotifyRoutine(PVOID LoadImageNotifyRoutine);

VOID UnregisterObCallback(PVOID ObHandle);

//extern PLIST_ENTRY ExpCallbackListHead;
//extern PKSPIN_LOCK ExpCallbackListLock;

extern CALLBACK_LOOKUP_TABLE g_CallbackTable;