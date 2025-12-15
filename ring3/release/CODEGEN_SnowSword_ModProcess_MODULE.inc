'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/tlhelp32.bi" '已经搬到定义文件处
' #include Once "win/psapi.bi" '已经搬到定义文件处
' #include Once "win/winnt.bi" '已经搬到定义文件处
' #include Once "Afx/CWmiDisp.inc" '已经搬到定义文件处
'#include "win/ddk/winddk.bi"

Private Const MAX_PATH As Integer = 260
' Dim Shared hThread As Any Ptr '已经搬到定义文件处

Public Enum Interface
    Module = 0
    Thread = 1
    Windows = 2
    Memory = 3
    Process = 4
    KernelModule = 5
    KernelThread = 6
    File = 7
    ForegroundWindow = 8
    Callbacks = 9
    UnlockTheFile = 10
End Enum

' Type UNICODE_STRING '已经搬到定义文件处
'     Length As UShort '已经搬到定义文件处
'     MaximumLength As UShort '已经搬到定义文件处
'     Buffer As PWSTR '已经搬到定义文件处
' End Type '已经搬到定义文件处
    
Private Type PROCESS_BASIC_INFORMATION
    ExitStatus As NTSTATUS                                                          'NTSTATUS
    PebBaseAddress As Any Ptr                                                      'PPEB
    AffinityMask As ULong Ptr                                                        'ULONG_PTR
    BasePriority As Any Ptr                                                        'KPRIORITY
    UniqueProcessId As ULong Ptr                                                     'ULONG_PTR
    InheritedFromUniqueProcessId As ULong Ptr                                        'ULONG_PTR
End Type

' Type PLIST_ENTRY As LIST_ENTRY Ptr '已经搬到定义文件处

' Type PEB64_LDR_DATA '已经搬到定义文件处
'     Reserved1(7) As Byte '已经搬到定义文件处
'     Reserved2(2) As PVOID '已经搬到定义文件处
'     InMemoryOrderModuleList As LIST_ENTRY '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PPEB64_LDR_DATA As PEB64_LDR_DATA Ptr '已经搬到定义文件处

' Type LDR_DATA_TABLE_ENTRY '已经搬到定义文件处
'     Reserved1(1) As PVOID '已经搬到定义文件处
'     InMemoryOrderLinks As LIST_ENTRY '已经搬到定义文件处
'     Reserved2(1) As PVOID '已经搬到定义文件处
'     DllBase As PVOID '已经搬到定义文件处
'     Reserved3(1) As PVOID '已经搬到定义文件处
'     FullDllName As UNICODE_STRING '已经搬到定义文件处
'     Reserved4(7) As Byte '已经搬到定义文件处
'     Reserved5(2) As PVOID '已经搬到定义文件处
'     Union DUMMYUNIONNAME '已经搬到定义文件处
'         CheckSum As ULong '已经搬到定义文件处
'         Reserved6 As PVOID '已经搬到定义文件处
'     End Union '已经搬到定义文件处
'     TimeDateStamp As ULong '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PLDR_DATA_TABLE_ENTRY As LDR_DATA_TABLE_ENTRY Ptr '已经搬到定义文件处

' Type RTL_USER_PROCESS_PARAMETERS '已经搬到定义文件处
'     Reserved1(15) As Byte '已经搬到定义文件处
'     Reserved2(9) As PVOID '已经搬到定义文件处
'     ImagePathName As UNICODE_STRING '已经搬到定义文件处
'     CommandLine As UNICODE_STRING '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PRTL_USER_PROCESS_PARAMETERS As RTL_USER_PROCESS_PARAMETERS Ptr '已经搬到定义文件处

' Type PEB64 '已经搬到定义文件处
'     Reserved1(1) As Byte '已经搬到定义文件处
'     BeingDebugged As Byte '已经搬到定义文件处
'     Reserved2 As Byte '已经搬到定义文件处
'     Reserved3(1) As PVOID '已经搬到定义文件处
'     Ldr As PPEB64_LDR_DATA '已经搬到定义文件处
'     ProcessParameters As PRTL_USER_PROCESS_PARAMETERS '已经搬到定义文件处
'     Reserved4(2) As PVOID '已经搬到定义文件处
'     AtlThunkSListPtr As PVOID '已经搬到定义文件处
'     Reserved5 As PVOID '已经搬到定义文件处
'     Reserved6 As ULong '已经搬到定义文件处
'     Reserved7 As PVOID '已经搬到定义文件处
'     Reserved8 As ULong '已经搬到定义文件处
'     AtlThunkSListPtr32 As ULong '已经搬到定义文件处
'     Reserved9(44) As PVOID '已经搬到定义文件处
'     Reserved10(95) As Byte '已经搬到定义文件处
'     PostProcessInitRoutine As Any Ptr'PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine '已经搬到定义文件处
'     Reserved11(127) As Byte '已经搬到定义文件处
'     Reserved12 As PVOID '已经搬到定义文件处
'     SessionId As ULong '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PPEB64 As PEB64 Ptr '已经搬到定义文件处

' Type OBJECT_ATTRIBUTES '已经搬到定义文件处
'     Length As ULong '已经搬到定义文件处
'     RootDirectory As HANDLE '已经搬到定义文件处
'     ObjectName As UNICODE_STRING Ptr '已经搬到定义文件处
'     Attributes As ULong '已经搬到定义文件处
'     SecurityDescriptor As PVOID '已经搬到定义文件处
'     SecurityQualityOfService As PVOID '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr '已经搬到定义文件处

' Type CLIENT_ID '已经搬到定义文件处
'     UniqueProcess As HANDLE '已经搬到定义文件处
'     UniqueThread As HANDLE '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PCLIENT_ID As CLIENT_ID Ptr '已经搬到定义文件处
' Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr '已经搬到定义文件处

' Type SYSTEM_THREAD '已经搬到定义文件处
'     KernelTime As LARGE_INTEGER '已经搬到定义文件处
'     UserTime As LARGE_INTEGER '已经搬到定义文件处
'     CreateTime As LARGE_INTEGER '已经搬到定义文件处
'     WaitTime As ULong '已经搬到定义文件处
'     StartAddress As PVOID '已经搬到定义文件处
'     ClientID As CLIENT_ID '已经搬到定义文件处
'     Priority As Long 'KPRIORITY '已经搬到定义文件处
'     BasePriority As Long ' KPRIORITY '已经搬到定义文件处
'     ContextSwitchCount As ULong '已经搬到定义文件处
'     ThreadState As ULong '已经搬到定义文件处
'     WaitReason As Long ' KWAIT_REASON '已经搬到定义文件处
'     #ifdef _WIN64 '已经搬到定义文件处
'     Reserved As ULong'Add '已经搬到定义文件处
'     #else '已经搬到定义文件处
'     #endif '已经搬到定义文件处
' End Type '已经搬到定义文件处

' Type VM_COUNTERS '已经搬到定义文件处
'     PeakVirtualSize As size_t '已经搬到定义文件处
'     VirtualSize As size_t '已经搬到定义文件处
'     PageFaultCount As ULong '已经搬到定义文件处
'     PeakWorkingSetSize As size_t '已经搬到定义文件处
'     WorkingSetSize As size_t '已经搬到定义文件处
'     QuotaPeakPagedPoolUsage As size_t '已经搬到定义文件处
'     QuotaPagedPoolUsage As size_t '已经搬到定义文件处
'     QuotaPeakNonPagedPoolUsage As size_t '已经搬到定义文件处
'     QuotaNonPagedPoolUsage As size_t '已经搬到定义文件处
'     PagefileUsage As size_t '已经搬到定义文件处
'     PeakPagefileUsage As size_t '已经搬到定义文件处
' End Type '已经搬到定义文件处

' Type SYSTEM_PROCESS '已经搬到定义文件处
'     NextEntryDelta As ULong '已经搬到定义文件处
'     ThreadCount As ULong '已经搬到定义文件处
'     Reserved(0 To 5) As ULong '已经搬到定义文件处
'     CreateTime As LARGE_INTEGER '已经搬到定义文件处
'     UserTime As LARGE_INTEGER '已经搬到定义文件处
'     KernelTime As LARGE_INTEGER '已经搬到定义文件处
'     ProcessName As UNICODE_STRING '已经搬到定义文件处
'     BasePriority As Integer ' KPRIORITY '已经搬到定义文件处
'     ProcessId As UInteger ' HANDLE '已经搬到定义文件处
'     InheritedFromProcessId As UInteger ' HANDLE '已经搬到定义文件处
'     HandleCount As ULong '已经搬到定义文件处
'     SessionId As ULong '已经搬到定义文件处
'     PageDirectoryBase As ULONG_PTR '已经搬到定义文件处
'     VmCounters As VM_COUNTERS '已经搬到定义文件处
'     PrivatePageCount As size_t '已经搬到定义文件处
'     IoCounters As IO_COUNTERS '已经搬到定义文件处
'     Threads(0) As SYSTEM_THREAD '已经搬到定义文件处
' End Type '已经搬到定义文件处

Public Enum PROCESSINFOCLASS
    BasicInformation = 0
    ProcessQuotaLimits
    ProcessIoCounters
    ProcessVmCounters
    ProcessTimes
    ProcessBasePriority
    ProcessRaisePriority
    ProcessDebugPort
    ProcessExceptionPort
    ProcessAccessToken
    ProcessLdtInformation
    ProcessLdtSize
    ProcessDefaultHardErrorMode
    ProcessIoPortHandlers                                                       '// Note: this is kernel mode only
    ProcessPooledUsageAndLimits
    ProcessWorkingSetWatch
    ProcessUserModeIOPL
    ProcessEnableAlignmentFaultFixup
    ProcessPriorityClass
    ProcessWx86Information
    ProcessHandleCount
    ProcessAffinityMask
    ProcessPriorityBoost
    ProcessDeviceMap
    ProcessSessionInformation
    ProcessForegroundInformation
    ProcessWow64Information
    ProcessImageFileName
    ProcessLUIDDeviceMapsEnabled
    ProcessBreakOnTermination
    ProcessDebugObjectHandle
    ProcessDebugFlags
    ProcessHandleTracing
    ProcessIoPriority
    ProcessExecuteFlags
    ProcessResourceManagement
    ProcessCookie
    ProcessImageInformation
    MaxProcessInfoClass                                                         '// MaxProcessInfoClass should always be the last enum
End Enum

'Private Function Replace
Extern "C"
    Declare Function ZwSuspendProcess Lib "ntdll.dll"(ByVal hProcess As HANDLE) As NTSTATUS
    Declare Function ZwResumeProcess Lib "ntdll.dll"(ByVal hProcess As HANDLE) As NTSTATUS
    Declare Function ZwOpenProcess Lib "ntdll.dll"(ByVal ProcessHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
    Declare Function ZwTerminateProcess Lib "ntdll.dll"(ByVal ProcessHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
    Declare Function ZwClose Lib "ntdll.dll"(ByVal HANDLE As HANDLE) As NTSTATUS
    Declare Function ZwQueryInformationProcess Lib "ntdll.dll"(ByVal ProcessHandle As HANDLE,ByVal ProcessInformationClass As PROCESSINFOCLASS,_
    ByVal ProcessInformation As LPVOID, ByVal ProcessInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
    Declare Function ZwSetInformationProcess Lib "ntdll.dll"(ByVal ProcessHandle As HANDLE,ByVal ProcessInformationClass As PROCESSINFOCLASS,_
    ByVal ProcessInformation As LPVOID, ByVal ProcessInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
    'Declare Sub ZwClose Lib "ntdll.dll"(ByVal HANDLE As HANDLE)
End Extern

' CONST OBJ_KERNEL_HANDLE = &h00000200 '已经搬到定义文件处
' #define SYSTEM_CALL_OPENPROCESS &H7A '已经搬到定义文件处
' #define SYSTEM_CALL_TERMINATEPROCESS &H2C '已经搬到定义文件处

/'Private Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong)
    p->Length = SizeOf(OBJECT_ATTRIBUTES)
    p->RootDirectory = NULL
    p->Attributes = a
    p->ObjectName = n
    p->SecurityDescriptor = NULL
    p->SecurityQualityOfService = NULL
End Sub'/

/'Private Function ZwOpenProcess(ByVal ProcessHandle As HANDLE Ptr, ByVal dwDesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
    Dim Status As NTSTATUS
    Asm
        mov rcx, ProcessHandle       '将第一个参数(进程句柄)放入 rcx
        mov rdx, dwDesiredAccess       '将第二个参数(访问权限)放入 rdx
        mov r8, ObjectAttributes     '将第三个参数（对象属性）放入 r8
        mov r9, ClientId             '将第四个参数（客户端 ID）放入 r9
        mov eax, SYSTEM_CALL_OPENPROCESS                '设置系统调用号
        syscall                      '执行系统调用
        mov Status, rax              '将返回值存储到 Status
    End Asm
    Return Status
End Function

Private Function ZwTerminateProcess(ByVal ProcessHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
    Dim Status As NTSTATUS
    Asm
        mov rcx, ProcessHandle  '将进程句柄放入 rcx
        mov rdx, ExitStatus     '将退出状态放入 rdx
        mov eax, SYSTEM_CALL_TERMINATEPROCESS  '设置系统调用号
        syscall                 '执行系统调用
        mov Status, rax         '将返回值存储到 Status
    End Asm
    Return Status
End Function'/

Function ForceKillProcess(ByVal dwProcessId As DWORD) As BOOLEAN
    Dim ProcessHandle As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim Status As NTSTATUS = ZwOpenProcess(@ProcessHandle, PROCESS_TERMINATE, @ObjectAttributes, @ClientId)
    If (Not NT_SUCCESS(Status)) Then
        PrintA "ZwOpenProcess Failed!(" & Hex(Status) & ")"
        Return False
    End If
    
        '调用 ZwTerminateProcess 结束进程
    Status = ZwTerminateProcess(ProcessHandle, 0)
    If (Not NT_SUCCESS(Status)) Then
        PrintA "ZwTerminateProcess Failed!(" & Hex(Status) & ")"
        ZwClose ProcessHandle
        Return False
    End If
    ZwClose ProcessHandle
    Return True
End Function

Private Function SetCriticalProcess(ByVal dwProcessId As DWORD, ByVal IsCritical As Boolean) As NTSTATUS
    Dim hProcess As HANDLE, ReturnLength As Long, ret As NTSTATUS
    hProcess = OpenProcess(PROCESS_SET_INFORMATION, False, dwProcessId)
    ret = ZwSetInformationProcess(hProcess, ProcessBreakOnTermination, @IsCritical, Len(IsCritical), ReturnLength)
    ZwClose hProcess
    Return ret
End Function

Private Function IsProcessCritical(ByVal dwProcessId As DWORD, ByVal IsCritical As PBOOLEAN) As NTSTATUS
    Dim hProcess As HANDLE, ReturnLength As Long, ret As NTSTATUS
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    ret = ZwSetInformationProcess(hProcess, ProcessBreakOnTermination, IsCritical, Len(*IsCritical), ReturnLength)
    ZwClose hProcess
    Return ret
End Function
    
Private Function GetPEB(ByVal dwProcessId As DWORD) As Any Ptr
    Dim pbi As PROCESS_BASIC_INFORMATION
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    If (hProcess <= 0) Then
        AfxMsg "OpenProcess Failed!" & vbCrLf & WinErrorMsg(GetLastError)
        Return 0
    End If
    Dim status As NTSTATUS = ZwQueryInformationProcess(hProcess, BasicInformation, @pbi, SizeOf(pbi), 0)
    If (status <> 0) Then
        AfxMsg "OpenProcess Failed!" & vbCrLf & WinErrorMsg(status)
        Return 0
    End If
    Return pbi.PebBaseAddress
End Function

Private Function GetPIDList3(PID() As DWORD) As Integer
    Dim pEProcess As HANDLE, lpRet As DWORD, j As Integer, k As Integer = 100
    ReDim PID(k)
    For i As DWORD = 0 To &H20000 Step 4
        'pEProcess = 0
        IoControl hDrv, CTL_CODE_GEN(&H801), @i, SizeOf(DWORD), @pEProcess, SizeOf(HANDLE), @lpRet
        If j > k Then
            k += 100
            ReDim Preserve PID(k)
        End If
        If pEProcess <> 0 Then
            'PrintA i
            PID(j) = i
            j += 1
        End If
    Next
    Return j + 1
End Function

Function GetProcessNameByPath(ByVal ProcessPath As StringW) As StringW
    If InStr(ProcessPath, ":") = 0 Then
        Return ""
    Else
        Return RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
    End If
End Function

Private Function GetProcessPath(ByVal ProcessId As DWORD) As StringW
    Dim ClientId As CLIENT_ID, ObjectAttributes As OBJECT_ATTRIBUTES, hProcess As HANDLE
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, ProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim Status As NTSTATUS = ZwOpenProcess(@hProcess, PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, @ObjectAttributes, @ClientId)
    If (Not NT_SUCCESS(Status)) Then
        PrintA "ZwOpenProcess Failed!(" & Hex(Status) & ")"
        Return ""
    End If
    If hProcess Then
        Dim FilePath As WString * MAX_PATH
        GetModuleFileNameExW(hProcess, NULL, @FilePath, MAX_PATH)
        Return FilePath
    End If
    Return ""
End Function

Public Sub GetProcessList(lvwProcess As Class_ListView, ByVal IsCheckHideProcess As BOOLEAN)
    Dim PID1() As DWORD, ProcessPath() As String, PID3() As DWORD, k As Integer
    lvwProcess.DeleteAllItems
    'GetPIDList1(PID1())
    GetAllPID ProcessPath(), PID1(), 0
    Dim Path As String, strProcessName As String
    If IsCheckHideProcess And IsDriverLoaded Then
        GetPIDList3 PID3()
        For i As Integer = 0 To UBound(PID1) - 1
            'PrintA PID1(i + 1)
            lvwProcess.AddItem WStr(PID1(i + 1))
            'GetProcessPathAndName PID1(i + 1), strProcess, ProcessPath
            'lvwProcess.SetItemText i, 2, strProcess
            'If InStr(ProcessPath(i), ":") = 0 Then ProcessPath(i) = Process_GetPath(PID1(i))
            If InStr(ProcessPath(i + 1), ":") = 0 Then
                lvwProcess.SetItemText i, 2, ProcessPath(i + 1)
                Continue For
            End If
            lvwProcess.SetItemText i, 2, Right(ProcessPath(i + 1), Len(ProcessPath(i + 1)) - InStrRev(ProcessPath(i + 1), "\"))
            lvwProcess.SetItemText i, 3, ProcessPath(i + 1)
        Next
        For i As Integer = 0 To UBound(PID3)
            /'lvwProcess.AddItem WStr(PID3(i + 1))
            GetProcessPathAndName PID3(i + 1), strProcess, ProcessPath
            lvwProcess.SetItemText i, 2, strProcess
            lvwProcess.SetItemText i, 3, ProcessPath'/
            For j As Integer = 1 To UBound(PID1)
                If PID3(i) = PID1(j) Then Continue For, For
            Next
            Dim strProcessPath As StringW = GetProcessPath(PID3(i))
            Dim ProcessName As StringW= GetProcessNameByPath(strProcessPath)
            'SetListViewItemBackColor lvwProcess, i, BGR(255, 0, 0)
            lvwProcess.AddItem WStr(PID3(i))
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 2, "(Hide)" & ProcessName
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 3, strProcessPath
        Next
    Else
        For i As Integer = 0 To UBound(PID1) - 1
            'PrintA PID1(i + 1)
            lvwProcess.AddItem WStr(PID1(i + 1))
            'GetProcessPathAndName PID1(i + 1), strProcess, ProcessPath
            'lvwProcess.SetItemText i, 2, strProcess
            'If InStr(ProcessPath(i), ":") = 0 Then ProcessPath(i) = Process_GetPath(PID1(i))
            If InStr(ProcessPath(i + 1), ":") = 0 Then
                lvwProcess.SetItemText i, 2, ProcessPath(i + 1)
                Continue For
            End If
            lvwProcess.SetItemText i, 2, Right(ProcessPath(i + 1), Len(ProcessPath(i + 1)) - InStrRev(ProcessPath(i + 1), "\"))
            lvwProcess.SetItemText i, 3, ProcessPath(i + 1)
        Next
        AfxMsg "总共有" & UBound(PID1) & "个进程"
    End If
End Sub

Public Function SuspendProcess(ByVal dwProcessId As DWORD) As BOOLEAN
    Dim hProcess As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim Status As NTSTATUS = ZwOpenProcess(@hProcess, PROCESS_SUSPEND_RESUME, @ObjectAttributes, @ClientId)
    If (hProcess = 0) Then
        PrintA "打开进程失败,原因:" & Hex(Status),"提示"
        Goto Err1
    End If
    If (ZwSuspendProcess(hProcess) <> 0) Then
        PrintA "挂起进程失败,原因:" & Hex(Status),"提示"
        Goto Err1
    End If
    Return True
    Err1 : 
    ZwClose hProcess
    Return False
End Function

Public Function ResumeProcess(ByVal dwProcessId As DWORD) As BOOLEAN
    Dim hProcess As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim Status As NTSTATUS = ZwOpenProcess(@hProcess, PROCESS_SUSPEND_RESUME, @ObjectAttributes, @ClientId)
    If (hProcess = 0) Then
        PrintA "打开进程失败,原因:" & Hex(Status),"提示"
        Goto Err1
    End If
    If (ZwResumeProcess(hProcess) <> 0) Then
        PrintA "恢复进程失败,原因:" & Hex(Status),"提示"
        Goto Err1
    End If
    Return True
    Err1 : 
    ZwClose hProcess
    Return False
End Function

/'#include <windows.h>
'#include <stdio.h>

BOOL KillProcessByJob(DWORD pid) {
    HANDLE hJob = CreateJobObject(NULL, NULL, 0);
    if (hJob == NULL) {
        printf("Failed to create job object.\n");
        return FALSE;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open process with PID %lu.\n", pid);
        CloseHandle(hJob);
        return FALSE;
    }

    if (!AssignProcessToJobObject(hJob, hProcess)) {
        DWORD dwError = GetLastError();
        printf("Failed to assign process to job, error %lu.\n", dwError);
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!TerminateJobObject(hJob, 0)) {
        DWORD dwError = GetLastError();
        printf("Failed to terminate job, error %lu.\n", dwError);
        CloseHandle(hJob);
        CloseHandle(hProcess);
        return FALSE;
    }

    CloseHandle(hJob);
    CloseHandle(hProcess);
    return TRUE;
}'/

Public Function KillProcess(ByVal dwProcessId As DWORD) As BOOLEAN
    Dim IsCritical As BOOLEAN
    If (NT_SUCCESS(IsProcessCritical(dwProcessId, @IsCritical))) Then
        If (IsCritical) Then
            If (AfxMsg("此进程处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDNO) Then
                Return False
            Else
                SetCriticalProcess(dwProcessId, False)
            End If
        End If
    End If
    Dim hProcess As HANDLE = OpenProcess(PROCESS_TERMINATE, False, dwProcessId)
    If (hProcess = 0) Then
        PrintA "打开进程失败,原因:" & WinErrorMsg(GetLastError)
        Goto Err1
    End If
    If (TerminateProcess(hProcess, 0) = 0) Then
        PrintA "结束进程失败,原因:" & WinErrorMsg(GetLastError)
        Goto Err1
    End If
    PrintA "结束进程成功!"
    Return True
    Err1 : 
    CloseHandle hProcess
    Return ForceKillProcess(dwProcessId)
End Function

Private Sub DeleteAllMenu(ByVal mnuMenu As Class_Menu)
    Dim i As Integer
    Do While (mnuMenu.DeleteItem(i) = True)
        i += 1
    Loop
End Sub
'Collection
Public Sub InitializeListView(ByVal ListView As Class_ListView, ByVal InterfaceType As Interface)
    ListView.DeleteAllColumn
    ListView.DeleteAllItems
    'DeleteAllMenu mnuMenu
    'PrintA "OriginalType:" & OriginalType & __FUNCTION__
    OriginalType = IIf(CurrentType = -1, InterfaceType, CurrentType)
    CurrentType = InterfaceType
    'PrintA "CurrentType:" & CurrentType & __FUNCTION__
    With ListView
        Select Case InterfaceType
            Case ForegroundWindow
                .AddColumn "窗口标题",, 400
                .AddColumn "进程名",, 200
                .AddColumn "状态",, 200
            Case Module
                .AddColumn "序号",, 50
                .AddColumn "名称",, 200
                .AddColumn "基址",, 230
                .AddColumn "大小",, 150
                .AddColumn "路径",, 500
            Case Thread
                '.AddColumn "序号",, 50
                .AddColumn "TID",, 100
                .AddColumn "优先级",, 120
                .AddColumn "状态",, 100
                .AddColumn "线程入口",, 150
                .AddColumn "Teb",, 150
                '.AddColumn "大小",, 50
                .AddColumn "所属模块",, 1000
            Case Process
                .AddColumn "PID",, 100
                .AddColumn "EPROCESS",, 250
                .AddColumn "名称",, 200
                .AddColumn "路径",, 500
            Case KernelModule
                .AddColumn "序号",, 100
                .AddColumn "名称",, 200
                .AddColumn "映像基址",, 230
                .AddColumn "映像大小",, 150
                .AddColumn "路径",, 500
            Case Callbacks
                .AddColumn "回调类型",, 200
                .AddColumn "回调函数地址",, 300
                .AddColumn "回调函数上下文",, 300
                .AddColumn "所属模块",, 300
            Case File
                .AddColumn "名称",, 200
                .AddColumn "大小",, 100
            Case UnlockTheFile
                .AddColumn "进程名",, 200
                .AddColumn "PID",, 100
                .AddColumn "文件路径",, 300
                .AddColumn "进程路径",, 300
                .AddColumn "文件句柄",, 150
            Case Memory
                .AddColumn "地址",, 250
                .AddColumn "大小",, 100
                .AddColumn "属性",, 150
                .AddColumn "状态",, 150
                .AddColumn "类型",, 150
                .AddColumn "基址",, 250
        End Select
    End With
End Sub

Sub ThreadProc(ByVal UserData As Any Ptr)
    'Dim objWMIService As AFX_ISWbemServices'IWbemObjectSink
    'objWMIService.Get "Winmgmts：{impersonationLevel = impersonate}"
    'objWMIService.ExecNotificationQuery("SELECT * FROM __InstanceCreationEvent WITHIN 10 WHERE" & "TargetInstance ISA"Win32_Process"")
    'Dim i As Integer,strLatestProcess As 
    'Do While True
    '    strLatestProcess = objWMIService.NextEvent
    '    WScript.Echo strLatestProcess.TargetInstance.Name
    '    WScript.Sleep 10000
    'Loop
End Sub

Public Sub MonitorCreateProcess(ByVal MonitorFlag As BOOLEAN = True)
    'If (MonitorFlag) Then hThread = ThreadCreate(@ThreadProc,1) Else Threaddetach hThread
End Sub
