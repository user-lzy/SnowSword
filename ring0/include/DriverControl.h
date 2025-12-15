#pragma once

#include "global.h"
//typedef struct _DRIVER_OBJECT* PDRIVER_OBJECT;

typedef struct _SYSINFO {
    ULONG U;
    PVOID X[2];
    PVOID BaseAddress;
    PVOID Size;
} SYSINFO, * PSYSINFO;

//NTSTATUS NTKERNELAPI ObMakeTemporaryObject(PVOID pObject);

//typedef VOID(*_MiProcessLoaderEntry)(IN PKLDR_DATA_TABLE_ENTRY DataTableEntry, IN LOGICAL Insert);

/*VOID HideDriver(PDRIVER_OBJECT DriverObject, ULONG Offset) {
    ULONG NeededSize;
    PSYSINFO SysInfo = (PSYSINFO)&SysInfo;
    ZwQuerySystemInformation(11, NULL, 0, &NeededSize);
    SysInfo = (PSYSINFO)ExAllocatePool(PagedPool, NeededSize);
    ZwQuerySystemInformation(11, SysInfo, NeededSize, NULL);
    _MiProcessLoaderEntry MiProcessLoaderEntry = (_MiProcessLoaderEntry)((PUCHAR)SysInfo->BaseAddress + Offset);
    MiProcessLoaderEntry((PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection, 0);
}*/

VOID EmptyDriverUnload(PDRIVER_OBJECT pDriverObject);
VOID UnloadDriver(PDRIVER_OBJECT pDriverObject);