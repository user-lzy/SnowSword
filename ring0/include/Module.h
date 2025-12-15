#pragma once
#include <ntdef.h>
#include <intrin.h>
#include <ntimage.h>
#include "EnumDriverInfo.h"

typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY64 InLoadOrderLinks;
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
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

typedef struct _INJECT_INFO {
	HANDLE ProcessId;
	HANDLE ThreadId;
	PWSTR DllPath;
} INJECT_INFO, *PINJECT_INFO;

extern PLIST_ENTRY PsLoadedModuleList;

// ¾Ü¾ø¼ÓÔØÇý¶¯
NTSTATUS DenyLoadDriver(PVOID pImageBase);

// ¾Ü¾ø¼ÓÔØ DLL Ä£¿é
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
	PSIZE_T pBytesWritten
);

NTSYSAPI NTSTATUS NTAPI NtReadVirtualMemory(IN HANDLE ProcessHandle, IN PVOID BaseAddress, 
	OUT PVOID Buffer, IN ULONG NumberOfBytesToRead, OUT PULONG NumberOfBytesReaded OPTIONAL);