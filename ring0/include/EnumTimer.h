#pragma once
#include "OtherFunctions.h"
#include "global.h"
#include "ntstrsafe.h"

typedef struct _SYSTEM_TIMER {
    wchar_t Name[10];
    PVOID TimerObject;
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
    LONG32       Unknown;
    LIST_ENTRY   TimerList;
    PVOID        TimerRoutine;
    PVOID        Context;
    PVOID        DeviceObject;
}IO_TIMER, *PIO_TIMER;

typedef struct _KTIMER_TABLE_ENTRY
{
    ULONG_PTR   Lock;
    LIST_ENTRY  Entry;
    ULONG_PTR   Time;
}KTIMER_TABLE_ENTRY, * PKTIMER_TABLE_ENTRY;

typedef struct _KTIMER_TABLE
{
    ULONG_PTR           TimerExpiry[64];
    KTIMER_TABLE_ENTRY  TimerEntries[256];
}KTIMER_TABLE, * PKTIMER_TABLE;

typedef struct _PROCESS_TIMER {
    HANDLE ThreadId;
    ULONG Period;
    PVOID Func;
}PROCESS_TIMER, * PPROCESS_TIMER;

typedef struct _PROCESS_TIMER_INFO {
    ULONG NumOfTimer;
    PROCESS_TIMER Timers[1];
}PROCESS_TIMER_INFO, * PPROCESS_TIMER_INFO;

PVOID FindIopTimerQueueHead();

void EnumIoTimers(PSYSTEM_TIMER SystemTimers);

PVOID FindKeSetTimerEx();

VOID FindKiWaitXXX(PVOID KeSetTimerEx, PVOID* KiWaitNever, PVOID* KiWaitAlways);

void EnumDpcTimers(PSYSTEM_TIMER SystemTimers);