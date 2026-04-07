#pragma once
#include <ntdef.h>
#include <intrin.h>
#include <ntimage.h>
#include "EnumDriverInfo.h"
#include "global.h"

typedef struct _INJECT_INFO {
	HANDLE ProcessId;
	HANDLE ThreadId;
	PWSTR DllPath;
} INJECT_INFO, *PINJECT_INFO;

extern PLIST_ENTRY PsLoadedModuleList;

// 拒绝加载驱动
NTSTATUS DenyLoadDriver(PVOID pImageBase);

// 拒绝加载 DLL 模块
BOOLEAN DenyLoadDll(PVOID pLoadImageBase);

void SetLoadImageNotifyRoutine(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,
	_In_ PIMAGE_INFO ImageInfo
);

NTSTATUS SetImageMonitorStatus(BOOLEAN flag);

PVOID GetModuleBase(UNICODE_STRING ModuleName, PULONG pModuleSize);

PVOID GetProcAddress(PCHAR FunctionName, PVOID ModuleBase);

void EnumerateFilterDrivers();

NTSTATUS InjectDllByApc(
	HANDLE ProcessId,
	HANDLE ThreadId,
	PCWSTR DllPath
);

NTSTATUS DumpKernelModule(
	PVOID ModuleBase,
	PVOID pOutputBuffer,
	SIZE_T BufferSize,          // 传入目标缓冲区大小（原 stMemory.Size）
	PSIZE_T pBytesWritten
);

NTSYSAPI NTSTATUS NTAPI NtReadVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, 
	OUT PVOID Buffer, IN ULONG NumberOfBytesToRead, OUT PULONG NumberOfBytesReaded OPTIONAL);