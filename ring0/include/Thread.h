#pragma once

#include "global.h"
#include "OtherFunctions.h"
#include "ObjectInfo.h"

typedef struct _ENUM_THREAD_CONTEXT {
    PHANDLE ThreadArray;
    ULONG ThreadCount;
}ENUM_THREAD_CONTEXT, * PENUM_THREAD_CONTEXT;

typedef struct _WORKER_THREAD_INFO {
    PVOID        Thread;         // KTHREAD 内核地址
    ULONG64      ThreadId;       // 线程ID (TID)
    KPRIORITY    Priority;       // 线程优先级
    //ULONG        NumaNode;       // NUMA 节点
    WCHAR        PoolType[8];    // 队列类型: ExPool/IoPool
    PVOID        QueueAddress;   // 工作队列 EX_WORK_QUEUE 地址
	PVOID        StartAddress;    // 线程入口地址
} WORKER_THREAD_INFO, * PWORKER_THREAD_INFO;

typedef VOID(*PKNORMAL_ROUTINE)(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef VOID(*PKKERNEL_ROUTINE) (
    IN struct _KAPC* Apc,
    IN OUT PKNORMAL_ROUTINE* NormalRoutine,
    IN OUT PVOID* NormalContext,
    IN OUT PVOID* SystemArgument1,
    IN OUT PVOID* SystemArgument2
    );

typedef VOID(*PKRUNDOWN_ROUTINE) (
    IN struct _KAPC* Apc
    );

typedef enum _KAPC_ENVIRONMENT {
    OriginalApcEnvironment,
    AttachedApcEnvironment,
    CurrentApcEnvironment,
    InsertApcEnvironment
} KAPC_ENVIRONMENT;

NTKERNELAPI VOID KeInitializeApc(
    IN PRKAPC Apc,
    IN PKTHREAD Thread,
    IN KAPC_ENVIRONMENT Environment,
    IN PKKERNEL_ROUTINE KernelRoutine,
    IN PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
    IN PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
    IN KPROCESSOR_MODE ApcMode,
    IN PVOID NormalContext
);

NTKERNELAPI BOOLEAN KeInsertQueueApc(
    __inout PRKAPC Apc,
    __in_opt PVOID SystemArgument1,
    __in_opt PVOID SystemArgument2,
    __in KPRIORITY Increment
);

NTKERNELAPI NTSTATUS KeAlertThread(
    _Inout_ PKTHREAD Thread,     // 目标线程的内核对象指针（KTHREAD结构）
    _In_    BOOLEAN  AlertMode   // 警报模式（KernelMode或UserMode）
);

NTKERNELAPI UCHAR* PsGetProcessImageFileName(__in PEPROCESS Process);

/*NTKERNELAPI BOOLEAN MmIsAddressExecutable(
    _In_ PVOID VirtualAddress
);*/

extern HANDLE MyProcessId;
extern PVOID ObHandle2;

// 枚举工作队列线程
NTSTATUS EnumWorkItemThread(
    __out PWORKER_THREAD_INFO* ppThreadArray,
    __out ULONG* pCount
);

VOID RemoveThreadHandleAccess(PACCESS_MASK pDesiredAccess);

// 回调函数
POB_PREOP_CALLBACK_STATUS ProtectThreadCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);

NTSTATUS SetThreadProtectionStatus(BOOLEAN flag);

VOID DenyCreateThread(HANDLE ThreadId);

NTSTATUS OpenThread(HANDLE ThreadId, PHANDLE hTargetThreadHandle);

// 线程创建通知回调函数 
VOID ThreadCreateNotifyRoutine(
    HANDLE ProcessId,
    HANDLE ThreadId,
    BOOLEAN Create
);

NTSTATUS SetThreadMonitorStatus(BOOLEAN flag);

PETHREAD GetEThread(HANDLE ThreadId);

PVOID FindPspExitThread();

VOID KillThreadRoutine(IN PKAPC Apc, IN PKNORMAL_ROUTINE* NormalRoutine, IN PVOID* NormalContext, IN PVOID* SystemArgument1, IN PVOID* SystemArgument2);

NTSTATUS ForceKillThread(HANDLE ThreadId);
BOOLEAN ForceDestroyThread(HANDLE hThreadId);

//NTSTATUS MyPsLookupThreadByThreadId(HANDLE ProcessId, PEPROCESS* Process);
ULONG EnumThreads(PHANDLE Array);