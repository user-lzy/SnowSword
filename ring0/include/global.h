#pragma once

#include <ntifs.h>

typedef unsigned long ULONG;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned _int64 ULONG_PTR;
typedef void* PVOID;
typedef unsigned long DWORD;
#ifndef VOID
typedef void VOID;
#endif

// 声明自定义结构体
struct AdvancedOptions {
	BOOLEAN DenyCreateProcess;
	BOOLEAN DenyCreateThread;
	BOOLEAN DenyRemoteThread;
	BOOLEAN DenyLoadDriver;
	BOOLEAN DenyLoadDll;
	BOOLEAN DenyAccessRegistry;
	BOOLEAN IsProtectProcess;
	BOOLEAN IsProtectThread;
};
typedef struct AdvancedOptions* PAdvancedOptions;

typedef struct _HANDLEINFO {
	HANDLE Handle;
	PVOID Object;
} HANDLEINFO, * PHANDLEINFO;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	PVOID Section;
	PVOID MappedBase;
	PVOID ImageBase;            //映射基地址
	ULONG ImageSize;            //映射大小
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR FullPathName[0x0100];  //模块路径名称
}RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[ANYSIZE_ARRAY];
}RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

/*typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	ULONG64 __Undefined1;
	ULONG64 __Undefined2;
	ULONG64 __Undefined3;
	ULONG64 NonPagedDebugInfo;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG   Flags;
	USHORT  LoadCount;
	USHORT  __Undefined5;
	ULONG64 __Undefined6;
	ULONG   CheckSum;
	ULONG   __padding1;
	ULONG   TimeDateStamp;
	ULONG   __padding2;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;*/

// 完全匹配你系统原生的_KLDR_DATA_TABLE_ENTRY（基于你的dt输出）
typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY64 InLoadOrderLinks;      // 0x000
	ULONG64      ExceptionTable;        // 0x010
	ULONG        ExceptionTableSize;    // 0x018
	ULONG64      GpValue;               // 0x020
	ULONG64      NonPagedDebugInfo;     // 0x028
	ULONG64      DllBase;               // 0x030 （和系统一致）
	ULONG64      EntryPoint;            // 0x038
	ULONG        SizeOfImage;           // 0x040
	UNICODE_STRING FullDllName;         // 0x048
	UNICODE_STRING BaseDllName;         // 0x058 （和系统一致）
	ULONG        Flags;                 // 0x068
	USHORT       LoadCount;             // 0x06c
	USHORT       u1;                    // 0x06e（系统的匿名字段）
	ULONG64      SectionPointer;        // 0x070
	ULONG        CheckSum;              // 0x078
	ULONG        CoverageSectionSize;   // 0x07c
	ULONG64      CoverageSection;       // 0x080
	ULONG64      LoadedImports;         // 0x088
	ULONG64      Spare;                 // 0x090
	ULONG        SizeOfImageNotRounded; // 0x098
	ULONG        TimeDateStamp;         // 0x09c
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;            // +0x00 加载顺序链表
	LIST_ENTRY InMemoryOrderLinks;          // +0x10 内存顺序链表
	LIST_ENTRY InInitializationOrderLinks;  // +0x20 初始化顺序链表
	PVOID DllBase;                          // +0x30 模块基址
	PVOID EntryPoint;                       // +0x38 入口点（DllMain）
	ULONG SizeOfImage;                      // +0x40 模块映像大小
	UNICODE_STRING FullDllName;             // +0x48 完整路径
	UNICODE_STRING BaseDllName;             // +0x58 模块名称（短名）
	ULONG Flags;                            // +0x68 标志位
	WORD LoadCount;                         // +0x6C 加载计数
	WORD TlsIndex;                          // +0x6E TLS索引
	union {
		LIST_ENTRY HashLinks;               // +0x70 哈希表链表
		struct {
			PVOID SectionPointer;           // +0x70 节区指针
			ULONG CheckSum;                 // +0x78 校验和
		};
	};
	union {
		ULONG TimeDateStamp;                // +0x80 时间戳
		PVOID LoadedImports;                // +0x80 已加载导入
	};
	// Win10新增字段
	PVOID EntryPointActivationContext;      // +0x88 激活上下文
	PVOID PatchInformation;                 // +0x90 补丁信息
	LIST_ENTRY ForwarderLinks;              // +0x98 转发链表
	LIST_ENTRY ServiceTagLinks;             // +0xA8 服务标签链表
	LIST_ENTRY StaticLinks;                 // +0xB8 静态链接链表
	PVOID ContextInformation;               // +0xC8 上下文信息
	ULONG_PTR OriginalBase;                 // +0xD0 原始基址
	LARGE_INTEGER LoadTime;                 // +0xD8 加载时间
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef enum SYSTEM_INFORMATION_CLASS {
	SystemModuleInformation = 11
} SYSTEM_INFORMATION_CLASS;

extern PKEVENT pKernelEvent;
extern struct AdvancedOptions MyAdvancedOptions;

NTKERNELAPI NTSTATUS ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

// 假设pUserUnicodeString是用户模式传来的UNICODE_STRING指针
NTSTATUS ValidateUserUnicodeString(PUNICODE_STRING pUserUnicodeString, PUNICODE_STRING pKernelUnicodeString);

PVOID GetPspCidTable();

VOID MyExEnumHandleTable(
	PVOID TableBase,
	BOOLEAN(*CallbackFunction)(HANDLEINFO*, PVOID),
	PVOID Context
);

// 内核版 GetProcAddress
PVOID KernelGetProcAddress(
    _In_ PCHAR ModuleName,
    _In_ PCHAR FunctionName
);

// 内核全局变量：已加载模块列表头
extern PLIST_ENTRY PsLoadedModuleList;
// 内核全局变量：模块列表同步资源
extern ERESOURCE PsLoadedModuleResource;
