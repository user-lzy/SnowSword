#pragma once
#include "OtherFunctions.h"
#include "global.h"
#include "ntstrsafe.h"

typedef struct _SYSTEM_TIMER {
    wchar_t Name[10];
    PVOID TimerObject;
    PVOID pDpc;
    PVOID Func;
    ULONG Period;
    SHORT Type;
    SHORT Flag;
    PVOID Context;
}SYSTEM_TIMER, *PSYSTEM_TIMER;

typedef struct _IO_TIMER
{
    INT16        Type;
    INT16        TimerFlag;
    LIST_ENTRY   TimerList;
    PVOID        TimerRoutine;
    PVOID        Context;
    PVOID        DeviceObject;
}IO_TIMER, *PIO_TIMER;

typedef struct _KTIMER_TABLE_ENTRY
{
    volatile LONG64 Lock;
    LIST_ENTRY Entry;
    ULARGE_INTEGER Time;
}KTIMER_TABLE_ENTRY, * PKTIMER_TABLE_ENTRY;

typedef struct _KTIMER_TABLE
{
    ULONG_PTR           TimerExpiry[64];
    KTIMER_TABLE_ENTRY  TimerEntries[2][256];
}KTIMER_TABLE, * PKTIMER_TABLE;

static_assert(sizeof(KTIMER_TABLE_ENTRY) == 0x20, "bad entry size");
static_assert(FIELD_OFFSET(KTIMER_TABLE_ENTRY, Entry) == 0x08, "bad Entry");
static_assert(FIELD_OFFSET(KTIMER_TABLE_ENTRY, Time) == 0x18, "bad Time");

static_assert(
    FIELD_OFFSET(KTIMER_TABLE, TimerEntries) == 0x200,
    "bad TimerEntries");

typedef struct _PROCESS_TIMER {
    HANDLE ThreadId;
    ULONG Period;
    PVOID Func;
}PROCESS_TIMER, * PPROCESS_TIMER;

typedef struct _PROCESS_TIMER_INFO {
    ULONG NumOfTimer;
    PROCESS_TIMER Timers[1];
}PROCESS_TIMER_INFO, * PPROCESS_TIMER_INFO;

ULONG EnumIoTimers(PSYSTEM_TIMER SystemTimers, ULONG MaxCount);
ULONG EnumDpcTimers(PSYSTEM_TIMER SystemTimers, ULONG MaxCount);