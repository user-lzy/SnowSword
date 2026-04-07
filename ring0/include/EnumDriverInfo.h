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

#define MAP_POOL_TAG                'paMG'  // GMap
#define ATTACH_INFO_POOL_TAG        'iaGA'  // AGia
#define MAX_FILTER_DEPTH            32
#define MAX_DRIVER_NAME_LEN         260
#define MAX_DEVICE_NAME_LEN         260
#define MAX_PATH_LEN                260
#define MAX_TARGET_DEVICES 10

// 单个设备节点信息（扁平化存储，无指针）
typedef struct _ATTACHED_DEVICE_NODE {
    BOOLEAN IsOriginalDevice;           // 是否为原始设备
    PDEVICE_OBJECT DeviceObject;        // 设备对象地址（R3需要转换）
    PDRIVER_OBJECT DriverObject;        // 驱动对象地址（R3需要转换）
    WCHAR DriverName[MAX_DRIVER_NAME_LEN];   // 驱动名称
    WCHAR DriverPath[MAX_PATH_LEN];          // 驱动完整路径
    WCHAR DeviceName[MAX_DEVICE_NAME_LEN];   // 设备名称
    ULONG Level;                        // 层级
    PDEVICE_OBJECT ParentDeviceObject;         // 父设备对象地址（0 表示根）
} ATTACHED_DEVICE_NODE, * PATTACHED_DEVICE_NODE;

// 驱动信息头（全局返回数据）
typedef struct _GLOBAL_ATTACH_INFO {
    ULONG   DriverCount;         // 驱动总数
    ULONG   TotalSize;           // 总数据大小（包括头部、所有驱动信息块及设备节点数组）
} GLOBAL_ATTACH_INFO, * PGLOBAL_ATTACH_INFO;

// 单个驱动信息
typedef struct _DRIVER_ATTACH_INFO {
    WCHAR   DriverName[260];     // 驱动名称
    UINT64  DriverObject;        // 驱动对象地址（64位）
    ULONG   DeviceCount;         // 该驱动拥有的设备节点数
    // 之后紧跟 DeviceCount 个 ATTACHED_DEVICE_NODE 结构
} DRIVER_ATTACH_INFO, * PDRIVER_ATTACH_INFO;

// 内部映射条目（链表）
typedef struct _DEVICE_ATTACHMENT_ENTRY {
    PDEVICE_OBJECT AttachedToDevice;      // 被附加的底层设备
    PDEVICE_OBJECT ParentDeviceObject;    // 父节点
    PDEVICE_OBJECT AttacherDevice;        // 附加者设备
    PDRIVER_OBJECT AttacherDriver;        // 附加者驱动
    WCHAR AttacherDriverName[MAX_DRIVER_NAME_LEN];
    WCHAR DeviceObjectName[MAX_DEVICE_NAME_LEN];
    ULONG Level;                          // 计算出的层级
    struct _DEVICE_ATTACHMENT_ENTRY* Next;
} DEVICE_ATTACHMENT_ENTRY, * PDEVICE_ATTACHMENT_ENTRY;

// 全局状态
static BOOLEAN g_MapInitialized = FALSE;
static KSPIN_LOCK g_AttachmentMapLock;
static PDEVICE_ATTACHMENT_ENTRY g_AttachmentMap = NULL;
static BOOLEAN g_GlobalMapBuilt = FALSE;  // 标记是否已构建全局映射

NTSTATUS BuildGlobalDeviceAttachmentMap();
VOID FreeGlobalDeviceAttachmentMap();
NTSTATUS CalculateGlobalDataSize(OUT PULONG pTotalSize);
ULONG CalculateDeviceLevel(PDEVICE_OBJECT StackTop, PDEVICE_OBJECT TargetDev);
NTSTATUS FillGlobalData(PVOID OutputBuffer, ULONG OutputBufferSize, PULONG pBytesWritten);
