#pragma once

#include "ntifs.h"
#include "Process.h"
#include "Module.h"

struct MemoryStruct {
	HANDLE dwProcessId;
    ULONG64 Addr;
    SIZE_T Size;
    PVOID pData;
};

typedef enum _MEMORY_SCAN_RESULT {
    Normal = 0,
    Shellcode,
    HideDriver,
	UnknownCode
} MEMORY_SCAN_RESULT, * PMEMORY_SCAN_RESULT;

typedef struct _HARDWARE_PTE {
    union {
        struct {
            ULONG64 Valid : 1;
            ULONG64 Write : 1;
            ULONG64 Owner : 1;
            ULONG64 WriteThrough : 1;
            ULONG64 CacheDisable : 1;
            ULONG64 Accessed : 1;
            ULONG64 Dirty : 1;
            ULONG64 LargePage : 1;
            ULONG64 Global : 1;
            ULONG64 CopyOnWrite : 1;
            ULONG64 Prototype : 1;
            ULONG64 Reserved0 : 1;
            ULONG64 PageFrameNumber : 36;
            ULONG64 Reserved1 : 4;
            ULONG64 SoftwareWsIndex : 11;
            ULONG64 NoExecute : 1;
        } Bits;
        ULONG64 QuadPart;
    } u;
} HARDWARE_PTE, * PHARDWARE_PTE;

typedef struct MemoryStruct* PMemoryStruct;

NTSTATUS VxkCopyMemory(PVOID pDestination, PVOID pSourceAddress, SIZE_T SizeOfCopy, PSIZE_T pBytesTransferred);
NTSTATUS ReadKernelMemory(PVOID pAddress, PVOID pBuffer, ULONG SizeOfRead);
NTSTATUS NTKERNELAPI MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
//PULONG64 NTKERNELAPI MiGetPteAddress(PVOID VirtualAddress);

//MmCopyMemory
NTSTATUS CopyMemory(HANDLE dwProcessId, PVOID pSourceAddress, PVOID pDestinationAddress, SIZE_T SizeOfCopy, PSIZE_T pBytesTransferred);

NTSTATUS DisableCopyOnWrite(PVOID pMem, SIZE_T ulMemSize);

NTSTATUS EnableCopyOnWrite(PVOID pMem, SIZE_T ulMemSize);

MEMORY_SCAN_RESULT OptimizedScanKernelMemory(PVOID pMem, ULONG PageSize);

extern PLIST_ENTRY PsLoadedModuleList;
extern PVOID MmNonPagedPoolStart;
extern PVOID MmNonPagedPoolEnd;
#define PTE_BASE_ADDR 0xFFFFF6FB7DBED000ULL
//#define MiGetPteAddress(va) ((PMMPTE)(((((ULONG)(va)) >> 12) << 2) + PTE_BASE))