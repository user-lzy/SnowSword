#pragma once

#include "ntstatus.h"
#include "ntstrsafe.h"
#include "OtherFunctions.h"
#include "Process.h"

typedef struct _object_type_flags {
	char CaseInsensitive : 1;
	char UnnamedObjectsOnly : 1;
	char UseDefaultObject : 1;
	char SecurityRequired : 1;
	char MaintainHandleCount : 1;
	char MaintainTypeList : 1;
	char SupportsObjectCallbacks : 1;
	char CacheAligned : 1;
}object_type_flags, * p_object_type_flags;

typedef struct _OBJECT_TYPE_INITIALIZER {
	USHORT				wLength;
	object_type_flags	ObjectTypeFlags;
	ULONG				ObjcetTypeCode;
	ULONG				InvalidAttributes;
	GENERIC_MAPPING		GenericMapping;
	ULONG				ValidAccessMask;
	ULONG				RetainAccess;
	ULONG				PoolType;
	ULONG				DefaultPagedPoolCharge;
	ULONG				DefaultNonPagedPoolCharge;
	PVOID				DumpProcedure;
	PVOID				OpenProcedure;
	PVOID				CloseProcedure;
	PVOID				DeleteProcedure;
	PVOID				ParseProcedure;
	PVOID				SecurityProcedure;
	PVOID				QueryNameProcedure;
	PVOID				OkayToCloseProcedure;
}OBJECT_TYPE_INITIALIZER, * POBJECT_TYPE_INITIALIZER;

typedef struct _OBJECT_TYPE {
	LIST_ENTRY					TypeList;
	UNICODE_STRING				Name;
	ULONGLONG					DefaultObject;
	ULONG						TypeIndex;
	ULONG						TotalNumberOfObjects;
	ULONG						TotalNumberOfHandles;
	ULONG						HighWaterNumberOfObjects;
	ULONG						HighWaterNumberOfHandles;
	OBJECT_TYPE_INITIALIZER		TypeInfo;
	ULONGLONG					TypeLock;
	ULONG						Key;
	LIST_ENTRY					CallbackList;
}OBJECT_TYPE, * POBJECT_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING TypeName; // 对象类型名称
	ULONG TotalNumberOfObjects; // 对象总数
	ULONG TotalNumberOfHandles; // 句柄总数
	ULONG TotalPagedPoolUsage;  // 分页池使用量
	ULONG TotalNonPagedPoolUsage; // 非分页池使用量
	ULONG TotalNamePoolUsage;    // 名称池使用量
	ULONG TotalHandleTableUsage; // 句柄表使用量
	ULONG HighWaterNumberOfObjects; // 最大对象数
	ULONG HighWaterNumberOfHandles; // 最大句柄数
	ULONG HighWaterPagedPoolUsage;  // 最大分页池使用量
	ULONG HighWaterNonPagedPoolUsage; // 最大非分页池使用量
	ULONG HighWaterNamePoolUsage;    // 最大名称池使用量
	ULONG HighWaterHandleTableUsage; // 最大句柄表使用量
	ULONG InvalidAttributes;         // 无效属性
	GENERIC_MAPPING GenericMapping;  // 通用映射
	ULONG ValidAccess;               // 有效访问权限
	BOOLEAN SecurityRequired;        // 是否需要安全检查
	BOOLEAN MaintainHandleCount;     // 是否维护句柄计数
	BOOLEAN MaintainTypeList;        // 是否维护类型列表
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_BASIC_INFORMATION
{
	ULONG Attributes;               // 对象属性
	ACCESS_MASK GrantedAccess;     // 授予的访问权限
	ULONG HandleCount;             // 句柄数量
	ULONG PointerCount;            // 引用计数
	ULONG PagedPoolUsage;          // 分页池使用量
	ULONG NonPagedPoolUsage;       // 非分页池使用量
	ULONG Reserved[3];             // 保留字段
	ULONG NameInformationLength;   // 名称信息长度
	ULONG TypeInformationLength;   // 类型信息长度
	ULONG SecurityDescriptorLength;// 安全描述符长度
	LARGE_INTEGER CreateTime;      // 创建时间
} OBJECT_BASIC_INFORMATION, * POBJECT_BASIC_INFORMATION;

/*typedef struct _EX_PUSH_LOCK {
	ULONG_PTR Lock;
} EX_PUSH_LOCK, * PEX_PUSH_LOCK;*/

typedef struct _OBJECT_HEADER {
	LONG64 PointerCount;          // +0x000
	union {
		LONG64 HandleCount;       // +0x008
		PVOID NextToFree;         // +0x008 (alternative field)
	}Union;
	EX_PUSH_LOCK Lock;            // +0x010
	UCHAR TypeIndex;              // +0x018
	UCHAR TraceFlags;             // +0x019
	UCHAR InfoMask;               // +0x01a
	UCHAR Flags;                  // +0x01b
	ULONG Reserved;               // +0x01c
	PVOID ObjectCreateInfo;       // +0x020 (or QuotaBlockCharged)
	PVOID SecurityDescriptor;     // +0x028
	ULONGLONG Body;               // +0x030
} OBJECT_HEADER, * POBJECT_HEADER;

typedef struct _HANDLE_INFO {
	//BOOLEAN IsUnlockFile;
	HANDLE dwProcessId;
	HANDLE Handle;
    PVOID Object;
    wchar_t Type[50];
	wchar_t Name[260];
	//ACCESS_MASK GrantedAccess;
	//ULONG ReferenceCount;
	//KEVENT ResultReadyEvent;
	//PIO_WORKITEM pWorkItem;
}HANDLE_INFO, *PHANDLE_INFO;

typedef struct _SYSTEM_HANDLE {
	ULONG ProcessId;       // 所属进程的PID 
	UCHAR ObjectTypeNumber;// 对象类型标识（如进程=0x7，线程=0x8）
	UCHAR Flags;           // 句柄属性（如是否可继承）
	USHORT Handle;         // 句柄值 
	PVOID Object;           // 指向内核对象的指针 
	ACCESS_MASK GrantedAccess; // 访问权限 
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG NumberOfHandles;
	SYSTEM_HANDLE Information[1]; // 动态数组 
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

/*NTKERNELAPI NTSTATUS ObQueryObjectType(
	PVOID Object,                // 指向内核对象的指针
	PUNICODE_STRING TypeName     // 输出：对象类型的名称
);*/

typedef struct _DO_SOMETHING {
	HANDLE ProcessId;
	PVOID Context;
}DO_SOMETHING, * PDO_SOMETHING;

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
	BOOLEAN Inherit;
	BOOLEAN ProtectFromClose;
}OBJECT_HANDLE_FLAG_INFORMATION, * POBJECT_HANDLE_FLAG_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemHandleInformation = 11,
}SYSTEM_INFORMATION_CLASS;

typedef struct _OBJECT_INFORMATION {
	WCHAR ObjectName[50];
	PVOID Procedure[8];
}OBJECT_INFORMATION, * POBJECT_INFORMATION;

NTKERNELAPI NTSTATUS ObSetHandleAttributes(HANDLE Handle, POBJECT_HANDLE_FLAG_INFORMATION HandleFlags, KPROCESSOR_MODE PreviousMode);
//NTSTATUS NTAPI ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);

#define ObjectNameInformation 1

// 获取对象的 OBJECT_HEADER
#define OBJECT_TO_OBJECT_HEADER(obj) \
    CONTAINING_RECORD((obj), OBJECT_HEADER, Body)

//ULONG* ObpInfoMaskToOffset = NULL;
#define MyDbgPrint(Format, ...) \
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, Format, ##__VA_ARGS__)

NTSTATUS QueryObject(PHANDLE_INFO HandleInfo);

NTSTATUS CloseHandle(PDO_SOMETHING DoSomething);

HANDLE GetCsrPid();
PVOID FindObTypeIndexTable();
NTSTATUS GetObjectInfo(ULONG Index, POBJECT_INFORMATION Array);
POBJECT_TYPE NTKERNELAPI ObGetObjectType(PVOID Object);

POBJECT_TYPE* FindExCallbackObjectType();
//NTSTATUS QueryCallbackNameByPointer(PVOID pObject, PWCHAR CallbackName);
