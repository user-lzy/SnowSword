#pragma once

#include "global.h"
#include "OtherFunctions.h"
#include "ObjectInfo.h"

typedef struct _ENUM_THREAD_CONTEXT {
    PHANDLE ThreadArray;
    ULONG ThreadCount;
}ENUM_THREAD_CONTEXT, * PENUM_THREAD_CONTEXT;

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

//NTSTATUS MyPsLookupThreadByThreadId(HANDLE ProcessId, PEPROCESS* Process);
ULONG EnumThreads(PHANDLE Array);