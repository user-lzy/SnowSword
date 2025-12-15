#pragma once
#include "ntifs.h"
#include "global.h"
#include "ObjectInfo.h"
#include "OtherFunctions.h"

struct AdvancedOptions MyAdvancedOptions;
extern PVOID ObHandle1;

#define PROCESS_TERMINATE 0x0001
#define PROCESS_CREATE_THREAD 0x0002
//#define PROCESS_SET_SESSIONID &h0004
#define PROCESS_VM_OPERATION 0x0008
#define PROCESS_VM_WRITE 0x0020
#define PROCESS_SET_INFORMATION 0x0200
#define PROCESS_SUSPEND_RESUME 0x0800
#define PROCESS_DENIED_ACCESS (PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_DUP_HANDLE | PROCESS_SET_INFORMATION | PROCESS_SUSPEND_RESUME)

//Fn+P
/*NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(
	IN HANDLE ProcessId,   //进程ID
	OUT PEPROCESS* Process //返回的EPROCESS
);*/

typedef struct _ENUM_PROCESSES_CONTEXT {
    PHANDLE ProcessArray;
	ULONG ProcessCount;
}ENUM_PROCESSES_CONTEXT, * PENUM_PROCESSES_CONTEXT;

//NTKERNELAPI NTSTATUS PsTerminateProcess(IN PEPROCESS Process, IN NTSTATUS ExitStatus);
NTSYSCALLAPI NTSTATUS PsSuspendProcess(IN PEPROCESS Process);
NTSYSCALLAPI NTSTATUS PsResumeProcess(IN PEPROCESS Process);

NTSTATUS ZwQueryInformationProcess(
    __in HANDLE ProcessHandle,
    __in PROCESSINFOCLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength,
    __out_opt PULONG ReturnLength
);
NTKERNELAPI UCHAR* PsGetProcessImageFileName(__in PEPROCESS Process);
NTKERNELAPI NTSTATUS PspTerminateProcess(IN PEPROCESS Process, IN NTSTATUS ExitStatus);
BOOLEAN IsProcessAlive(HANDLE ProcessId);
//extern PVOID MmGetHighestUserAddress();

// 定义保护进程列表的结构
typedef struct _PROTECTED_PROCESS {
    LIST_ENTRY ListEntry;
    HANDLE ProcessId;
} PROTECTED_PROCESS, * PPROTECTED_PROCESS;

// 初始化保护进程列表头
PROTECTED_PROCESS ProtectedProcessList;

VOID SuspendProcess(HANDLE PID);

VOID ResumeProcess(HANDLE PID);

// 添加进程到保护列表
NTSTATUS AddProcessToProtectedList(HANDLE ProcessId);

// 从保护列表中删除进程
NTSTATUS RemoveProcessFromProtectedList(HANDLE ProcessId);

// 检查进程是否在保护列表中
BOOLEAN IsProcessProtected(HANDLE ProcessId);

BOOLEAN IsProcessAlive(HANDLE ProcessId);

//NTHALAPI KIRQL KeGetCurrentIRQL();

NTSTATUS GetProcessChangedPath(HANDLE PID, PWCHAR* FilePath);

VOID RemoveProcessHandleAccess(PACCESS_MASK pDesiredAccess);

POB_PREOP_CALLBACK_STATUS ProtectProcessCallBack(PVOID Context, POB_PRE_OPERATION_INFORMATION pObPreOperationInformation);

NTSTATUS SetProcessProtectionStatus(BOOLEAN flag);

VOID ProcessNotifyRoutineEx(
    PEPROCESS Process,
    HANDLE ProcessId,
    PPS_CREATE_NOTIFY_INFO CreateInfo
);

void KillProcess(HANDLE dwProcessId);

void ForceKillProcess(HANDLE dwProcessId);

NTSTATUS MemKillProcess(HANDLE pid);

void KillThread(HANDLE dwThreadId);

NTSTATUS SetProcessMonitorStatus(BOOLEAN flag);

NTSTATUS OpenProcess(HANDLE ProcessId, PHANDLE hTargetProcess);

NTSTATUS GetProcessHandleCount(HANDLE ProcessId, PULONG pHandleCount);

PEPROCESS GetEProcess(HANDLE ProcessId);

NTSTATUS GetProcessImageName(
    HANDLE ProcessId,
    LPWSTR ProcessImageName
);

NTSTATUS MyPsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
ULONG EnumProcesses(PHANDLE Array);

//extern POBJECT_TYPE ObGetObjectType(VOID* Object);