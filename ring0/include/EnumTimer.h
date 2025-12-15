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

typedef struct _WIN32THREAD {
    PVOID pEThread; //该指针用以获得进程ID和线程ID
    ULONG RefCount;
    ULONG ptlW32;
    ULONG pgdiDcattr;
    ULONG pgdiBrushAttr;
    ULONG pUMPDObjs;
    ULONG pUMPDHeap;
    ULONG dwEngAcquireCount;
    ULONG pSemTable;
    ULONG pUMPDObj;
    PVOID ptl;
    PVOID ppi; //该指针用以获得模块基址
}WIN32THREAD;

typedef struct _threadInfo {
    WIN32THREAD w32thread;
    CHAR UNK; // 不确定类型
}threadInfo;

typedef struct _HEAD {
    void* unk[3];
    void* threadInfo;
}HEAD;

typedef struct _timer_t {
    HEAD head;//0
    void* pfn;//20
    DWORD32 elapse;//28
    DWORD32 flags;//2c
    DWORD32 unkFlags;//30
    DWORD32 elapse1;//34
    char padding[0x10];//38
    LIST_ENTRY list1;//链接的是gtmrListHead //48
    void* spwnd;//58
    UINT64 id;//60
    void* threadObject;//68
    LIST_ENTRY list2;//Hash链接gTimerHashTable
}timer_t, *ptimer_t;

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

ULONG EnumProcessTimers(HANDLE ProcessId, PPROCESS_TIMER Array);

PVOID FindKeSetTimerEx();

VOID FindKiWaitXXX(PVOID KeSetTimerEx, PVOID* KiWaitNever, PVOID* KiWaitAlways);

void EnumDpcTimers(PSYSTEM_TIMER SystemTimers);