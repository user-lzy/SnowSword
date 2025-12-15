#pragma once

#include "global.h"

#define IRP_MJ_MAXIMUM_FUNCTION 0x1b
#define DO_PDO 0x00000008  // PDO标志位

typedef struct _DRIVER_INFO {
	PVOID DriverObjectAddr;
	PVOID AttachDevice;
	PVOID DriverInitAddr;
	PVOID DriverStartIoAddr;
	PVOID FastIoDispatchAddr;
	PVOID DriverUnloadAddr;
	PVOID MajorFunctionAddr[IRP_MJ_MAXIMUM_FUNCTION + 1];
}DRIVER_INFO, * PDRIVER_INFO;

typedef struct _AttachDeviceTable {
	LIST_ENTRY* AttachDevieList;
	PVOID AttachDevice;
}AttachDeviceTable, *PAttachDeviceTable;

typedef struct _ATTACHED_DEVICE_NODE {
    PDEVICE_OBJECT DeviceObject;
    PDRIVER_OBJECT DriverObject;
    WCHAR DriverName[260];
	WCHAR DriverPath[512];
    //struct _ATTACHED_DEVICE_NODE* NextAttached; // 指向下一个Attached设备
} ATTACHED_DEVICE_NODE, * PATTACHED_DEVICE_NODE;

typedef struct _ATTACH_DEVICE_INFO {
    //ATTACHED_DEVICE_NODE DeviceChain;  // 设备链头节点
	WCHAR DriverName[260];               // 驱动名称
	ATTACHED_DEVICE_NODE Devices[10];    // 假设最多10个设备
    ULONG TotalDevices;                  // 总设备数
} ATTACH_DEVICE_INFO, * PATTACH_DEVICE_INFO;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;
        };
        struct {
            PVOID LoadedImports;
        };
    };
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

//_OBJECT_HEADER 内两个成员的偏移
enum _OBJECT_HEADER_Offset {
    _OBJECT_HEADER_Body_Offset = 0x30,
    _OBJECT_HEADER_TypeIndex_Offset = 0x18
};

//0x18 bytes (sizeof)
typedef struct _OBJECT_DIRECTORY_ENTRY
{
    struct _OBJECT_DIRECTORY_ENTRY* ChainLink;                                      //0x0
    VOID* Object;                                                           //0x8
    ULONG HashValue;                                                        //0x10
}OBJECT_DIRECTORY_ENTRY, * POBJECT_DIRECTORY_ENTRY;

//0x158 bytes (sizeof)
typedef struct _OBJECT_DIRECTORY
{
    POBJECT_DIRECTORY_ENTRY HashBuckets[37];                                //0x0
    ULONG64 Lock;                                                           //0x128
    ULONG64 DeviceMap;                                                      //0x130
    struct _OBJECT_DIRECTORY* ShadowDirectory;                                      //0x138
    VOID* NamespaceEntry;                                                   //0x140
    VOID* SessionObject;                                                    //0x148
    ULONG Flags;                                                            //0x150
    ULONG SessionId;                                                        //0x154
}OBJECT_DIRECTORY, * POBJECT_DIRECTORY;

//0x20 bytes (sizeof)
typedef struct _OBJECT_HEADER_NAME_INFO
{
    POBJECT_DIRECTORY Directory;                                            //0x0
    UNICODE_STRING Name;                                                    //0x8
    LONG ReferenceCount;                                                    //0x18
    ULONG Reserved;                                                         //0x1c
}OBJECT_HEADER_NAME_INFO, * POBJECT_HEADER_NAME_INFO;

// 目录基本信息结构
typedef struct _DIRECTORY_BASIC_INFORMATION {
    //ULONG           NextEntryOffset;  // 下一个目录条目的偏移量（用于遍历）
    UNICODE_STRING  ObjectName;       // 目录中对象的名称
    UNICODE_STRING  ObjectType;       // 对象的类型名称（如"Directory"、"Driver"等）
} DIRECTORY_BASIC_INFORMATION, * PDIRECTORY_BASIC_INFORMATION;

extern PLIST_ENTRY PsLoadedModuleList;
extern POBJECT_TYPE* IoDriverObjectType;
//extern POBJECT_TYPE* DirectoryObjectType;

POBJECT_TYPE NTKERNELAPI ObGetObjectType(PVOID Object);
// 打开目录对象
NTSTATUS NTKERNELAPI ZwOpenDirectoryObject(
    OUT PHANDLE            pDirectoryHandle,
    IN ACCESS_MASK         DesiredAccess,
    IN POBJECT_ATTRIBUTES  pObjectAttributes
);
// 查询目录对象（获取目录内所有对象）
NTSTATUS NTKERNELAPI ZwQueryDirectoryObject(
    IN HANDLE              DirectoryHandle,
    OUT PVOID              pBuffer,
    IN ULONG               BufferLength,
    IN BOOLEAN             ReturnSingleEntry,
    IN BOOLEAN             RestartScan,
    IN OUT PULONG          pContext,
    OUT PULONG             pReturnLength OPTIONAL
);

VOID GetDriverInfo(PDRIVER_OBJECT pDriverObject, PDRIVER_INFO pDriverInfo);
NTSTATUS EnumAttachDevices(
    _In_ PDRIVER_OBJECT DriverObject,
    _Out_ PATTACH_DEVICE_INFO AttachInfo
);
BOOLEAN IsDeviceAttachedToDriver(
    _In_ PDEVICE_OBJECT Device,
    _In_ PDRIVER_OBJECT TargetDriver
);
NTSTATUS OpenDirectoryObject(PCWSTR DirPath, POBJECT_DIRECTORY* OutDirObj);
PDRIVER_OBJECT GetDirectoryDrivers(POBJECT_DIRECTORY DirObj, PVOID BaseAddress);
PVOID GetDriverObjectByBaseAddress(PVOID BaseAddress);
POBJECT_TYPE GetObjectType(_In_ PVOID Object);

// 声明ObReferenceObjectByName
/*NTSTATUS ObReferenceObjectByName(
    PUNICODE_STRING ObjectName,
    ULONG Attributes,
    PACCESS_STATE AccessState,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PVOID ParseContext,
    PVOID* Object
);*/

// 设备链信息结构
typedef struct _FILTER_CHAIN_INFO {
    UNICODE_STRING DriverName;      // 驱动名称
    UNICODE_STRING DeviceName;      // 设备名称
    PDEVICE_OBJECT AttachedDevice;  // 附加设备指针
    ULONG ChainDepth;               // 链中深度
} FILTER_CHAIN_INFO, * PFILTER_CHAIN_INFO;

NTSTATUS EnumFilterChains(
    _In_ PDRIVER_OBJECT TargetDriver,
    _Out_ PATTACH_DEVICE_INFO AttachInfo
);
