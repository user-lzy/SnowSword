#include Once "win/tlhelp32.bi"
#include Once "win/psapi.bi"
#include Once "win/winnt.bi"
'#include Once "Afx/CWmiDisp.inc"
'#include Once "win/wintrust.bi"
#include Once "win/winver.bi"
'#include "win/ddk/winddk.bi"

Private Const MAX_PATH As Integer = 260
Dim Shared hThread As Any Ptr

Type LANGANDCODEPAGE
    wLanguage As WORD
    wCodePage As WORD
End Type

Type UNICODE_STRING
    Length As UShort
    MaximumLength As UShort
    Buffer As PWSTR
End Type
    
Type PROCESS_BASIC_INFORMATION
    ExitStatus As NTSTATUS                                                          'NTSTATUS
    PebBaseAddress As Any Ptr                                                      'PPEB
    AffinityMask As ULong Ptr                                                        'ULONG_PTR
    BasePriority As Any Ptr                                                        'KPRIORITY
    UniqueProcessId As HANDLE
    ParentProcessId As HANDLE
End Type

Type PLIST_ENTRY As LIST_ENTRY Ptr

Type PEB64_LDR_DATA
    Reserved1(7) As Byte
    Reserved2(2) As PVOID
    InMemoryOrderModuleList As LIST_ENTRY
End Type
Type PPEB64_LDR_DATA As PEB64_LDR_DATA Ptr

Type LDR_DATA_TABLE_ENTRY
    Reserved1(1) As PVOID
    InMemoryOrderLinks As LIST_ENTRY
    Reserved2(1) As PVOID
    DllBase As PVOID
    Reserved3(1) As PVOID
    FullDllName As UNICODE_STRING
    Reserved4(7) As Byte
    Reserved5(2) As PVOID
    Union DUMMYUNIONNAME
        CheckSum As ULong
        Reserved6 As PVOID
    End Union
    TimeDateStamp As ULong
End Type
Type PLDR_DATA_TABLE_ENTRY As LDR_DATA_TABLE_ENTRY Ptr

Type RTL_USER_PROCESS_PARAMETERS
    Reserved1(15) As Byte
    Reserved2(9) As PVOID
    ImagePathName As UNICODE_STRING
    CommandLine As UNICODE_STRING
End Type
Type PRTL_USER_PROCESS_PARAMETERS As RTL_USER_PROCESS_PARAMETERS Ptr

Type PEB64
    Reserved1(1) As Byte
    BeingDebugged As Byte
    Reserved2 As Byte
    Reserved3(1) As PVOID
    Ldr As PPEB64_LDR_DATA
    ProcessParameters As PRTL_USER_PROCESS_PARAMETERS
    Reserved4(2) As PVOID
    AtlThunkSListPtr As PVOID
    Reserved5 As PVOID
    Reserved6 As ULong
    Reserved7 As PVOID
    Reserved8 As ULong
    AtlThunkSListPtr32 As ULong
    Reserved9(44) As PVOID
    Reserved10(95) As Byte
    PostProcessInitRoutine As Any Ptr'PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine
    Reserved11(127) As Byte
    Reserved12 As PVOID
    SessionId As ULong
End Type
Type PPEB64 As PEB64 Ptr

Type OBJECT_ATTRIBUTES
    Length As ULong
    RootDirectory As HANDLE
    ObjectName As UNICODE_STRING Ptr
    Attributes As ULong
    SecurityDescriptor As PVOID
    SecurityQualityOfService As PVOID
End Type
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr

Type CLIENT_ID
    UniqueProcess As HANDLE
    UniqueThread As HANDLE
End Type
Type PCLIENT_ID As CLIENT_ID Ptr
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr

Type SYSTEM_THREAD_INFORMATION
    KernelTime As LARGE_INTEGER
    UserTime As LARGE_INTEGER
    CreateTime As LARGE_INTEGER
    WaitTime As ULong
    StartAddress As PVOID
    ClientID As CLIENT_ID
    Priority As Integer 'KPRIORITY
    BasePriority As Long ' KPRIORITY
    ContextSwitchCount As ULong
    ThreadState As ULong
    WaitReason As ULong ' KWAIT_REASON
    '#ifdef _WIN64
    Reserved As ULong'Add
    '#else
    '#endif
End Type

Type VM_COUNTERS
    PeakVirtualSize As size_t
    VirtualSize As size_t
    PageFaultCount As ULong
    PeakWorkingSetSize As size_t
    WorkingSetSize As size_t
    QuotaPeakPagedPoolUsage As size_t
    QuotaPagedPoolUsage As size_t
    QuotaPeakNonPagedPoolUsage As size_t
    QuotaNonPagedPoolUsage As size_t
    PagefileUsage As size_t
    PeakPagefileUsage As size_t
End Type

Type SYSTEM_PROCESS_INFORMATION
    NextEntryDelta As ULong
    ThreadCount As ULong
    Reserved(0 To 5) As ULong
    CreateTime As LARGE_INTEGER
    UserTime As LARGE_INTEGER
    KernelTime As LARGE_INTEGER
    ProcessName As UNICODE_STRING
    BasePriority As Long ' KPRIORITY
    ProcessId As UInteger ' HANDLE
    InheritedFromProcessId As UInteger ' HANDLE
    HandleCount As ULong
    SessionId As ULong
    PageDirectoryBase As ULONG_PTR
    VmCounters As VM_COUNTERS
    PrivatePageCount As size_t
    IoCounters As IO_COUNTERS
    Threads(0) As SYSTEM_THREAD_INFORMATION
End Type

Type PROCESS_TIMER
    ThreadId As HANDLE
    Period As ULong
    Func As ULONG64
End Type

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
    MaxProcessInfoClass '// MaxProcessInfoClass should always be the last enum
    ProcessHandleInformation = 51
End Enum

'Private Function Replace
'32位编译需使用别名,且不使用Extern "C"
'Extern "C"
Declare Function ZwSuspendProcess Lib "ntdll.dll" Alias "ZwSuspendProcess"(ByVal hProcess As HANDLE) As NTSTATUS
Declare Function ZwResumeProcess Lib "ntdll.dll" Alias "ZwResumeProcess"(ByVal hProcess As HANDLE) As NTSTATUS
Declare Function ZwOpenProcess Lib "ntdll.dll" Alias "ZwOpenProcess"(ByVal ProcessHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
Declare Function ZwTerminateProcess Lib "ntdll.dll" Alias "ZwTerminateProcess"(ByVal ProcessHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
Declare Function ZwClose Lib "ntdll.dll" Alias "ZwClose"(ByVal HANDLE As HANDLE) As NTSTATUS
Declare Function ZwQuerySystemInformation1 Lib "ntdll.dll" Alias "ZwQuerySystemInformation"(ByVal SystemInformationClass As SYSTEM_INFORMATION_CLASS,ByVal SystemInformation As PVOID,ByVal SystemInformationLength As ULong,ByVal ReturnLength As PULONG) As NTSTATUS
Declare Function ZwQueryInformationProcess Lib "ntdll.dll" Alias "ZwQueryInformationProcess"(ByVal ProcessHandle As HANDLE,ByVal ProcessInformationClass As PROCESSINFOCLASS,_
ByVal ProcessInformation As LPVOID, ByVal ProcessInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
Declare Function _IsProcessCritical Lib "kernel32" Alias "IsProcessCritical"(ByVal hProcess As HANDLE, ByVal Critical As PBOOLEAN) As BOOLEAN
'End Extern
CONST OBJ_KERNEL_HANDLE = &h00000200
#define SYSTEM_CALL_OPENPROCESS &H26
#define SYSTEM_CALL_TERMINATEPROCESS &H2C

/'Private Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong)
    p->Length = SizeOf(OBJECT_ATTRIBUTES)
    p->RootDirectory = NULL
    p->Attributes = a
    p->ObjectName = n
    p->SecurityDescriptor = NULL
    p->SecurityQualityOfService = NULL
End Sub'/

Function MyZwOpenProcess Naked(_
    ByVal ProcessHandle As PHANDLE, _
    ByVal dwDesiredAccess As ACCESS_MASK, _
    ByVal ObjectAttributes As POBJECT_ATTRIBUTES, _
    ByVal ClientId As PCLIENT_ID) As NTSTATUS
    Asm
        ' 保存非易失性寄存器
        /'push rbx
        push rdi
        push rsi
        push rbp
        push r12
        push r13
        push r14
        push r15'/
        
        ' 在x64调用约定中，前4个参数已经在正确的寄存器中：
        ' rcx = ProcessHandle
        ' rdx = dwDesiredAccess
        ' r8 = ObjectAttributes
        ' r9 = ClientId
        
        mov r10, rcx
        
        ' 设置系统调用号
        mov eax, SYSTEM_CALL_OPENPROCESS  ' 需要根据你的系统定义正确的值
        
        ' 执行系统调用
        syscall
        
        ' 恢复非易失性寄存器
        /'pop r15
        pop r14
        pop r13
        pop r12
        pop rbp
        pop rsi
        pop rdi
        pop rbx'/
        
        ' 返回 (返回值已经在rax中)
        ret
    End Asm
End Function

Private Function MyZwTerminateProcess Naked(ByVal ProcessHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
    'Dim st As NTSTATUS
    /'Asm
        mov rcx, ProcessHandle  '将进程句柄放入 rcx
        mov rdx, ExitStatus     '将退出状态放入 rdx
        mov eax, SYSTEM_CALL_TERMINATEPROCESS  '设置系统调用号
        syscall                 '执行系统调用
        mov st, rax         '将返回值存储到 st
    End Asm '/
    Asm
        'mov rcx, ProcessHandle
        'mov rdx, ExitStatus
        mov r10, rcx
        mov eax, SYSTEM_CALL_TERMINATEPROCESS
        syscall
        '_emit &HF
        '_emit &H5
        'mov st, rax
        ret
    End Asm
    'Return st
End Function

Function MyOpenProcess(ByVal dwDesiredAccess As DWORD = PROCESS_ALL_ACCESS, ByVal bInheritHandle As WINBOOL = False, ByVal dwProcessId As DWORD) As HANDLE
    Dim ProcessHandle As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim st As NTSTATUS = MyZwOpenProcess(@ProcessHandle, dwDesiredAccess, @ObjectAttributes, @ClientId)
    'Print Hex(st)
    If (Not NT_SUCCESS(st)) Then
        Print "MyZwOpenProcess Failed!(" & Hex(st) & ")"
        Return NULL
    End If
    Return ProcessHandle
End Function

Function SnowSword_OpenProcess(ByVal ProcessId As DWORD) As HANDLE
    'If Not IsDriverLoaded Then Return NULL
    Dim PID As DWORD = ProcessId
    Dim hProcess As HANDLE = NULL
    IoControl hDrv, IOCTL_OpenProcess, @PID, SizeOf(DWORD), @hProcess, SizeOf(hProcess)
    Return hProcess
End Function

Function ForceKillProcess(ByVal dwProcessId As DWORD) As Boolean
    Dim ProcessHandle As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim st As NTSTATUS = MyZwOpenProcess(@ProcessHandle, PROCESS_TERMINATE, @ObjectAttributes, @ClientId)
    'Print Hex(st)
    If (Not NT_SUCCESS(st)) Then
        Print "ZwOpenProcess Failed!(" & Hex(st) & ")"
        Return False
    End If
    'Print ProcessHandle
    '调用 ZwTerminateProcess 结束进程
    st = MyZwTerminateProcess(ProcessHandle, 0)
    'Print Hex(st)
    If (Not NT_SUCCESS(st)) Then
        Print "ZwTerminateProcess Failed!(" & Hex(st) & ")"
        ZwClose ProcessHandle
        Return False
    End If
    ZwClose ProcessHandle
    Return True
End Function

Private Function SetCriticalProcess(ByVal dwProcessId As DWORD, ByVal IsCritical As BOOLEAN) As NTSTATUS
    Dim hProcess As HANDLE, ReturnLength As Long, ret As NTSTATUS, dStatus As DWORD = IsCritical
    Dim ZwSetInformationProcess As Function(ByVal ProcessHandle As HANDLE,ByVal ProcessInformationClass As PROCESSINFOCLASS,_
    ByVal ProcessInformation As LPVOID, ByVal ProcessInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS = DyLibSymbol(GetModuleHandle("ntdll.dll"), "ZwSetInformationProcess")
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_SET_INFORMATION, False, dwProcessId)
    End If
    Function = ZwSetInformationProcess(hProcess, ProcessBreakOnTermination, @dStatus, SizeOf(DWORD), ReturnLength)
    ZwClose hProcess
End Function

Function IsProcessCritical(ByVal dwProcessId As DWORD, ByVal IsCritical As PBOOLEAN) As Boolean
    Dim hProcess As HANDLE
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    If hProcess = NULL Then
        Print "OpenProcess" & dwProcessId & "failed:" & WinErrorMsg(GetLastError)
        Return False
    End If
    Function = _IsProcessCritical(hProcess, IsCritical)
    Print *IsCritical
End Function
    
Private Function GetPEB(ByVal dwProcessId As DWORD) As Any Ptr
    Dim pbi As PROCESS_BASIC_INFORMATION
    Dim hProcess As HANDLE
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    If (hProcess <= 0) Then
        AfxMsg "OpenProcess Failed!" & vbCrLf & WinErrorMsg(GetLastError)
        Return 0
    End If
    Dim st As NTSTATUS = ZwQueryInformationProcess(hProcess, BasicInformation, @pbi, SizeOf(pbi), 0)
    If (st <> 0) Then
        AfxMsg "OpenProcess Failed!" & vbCrLf & WinErrorMsg(st)
        Return 0
    End If
    Return pbi.PebBaseAddress
End Function

Private Function GetEProcess(ByVal ProcessId As DWORD) As ULONG64
    Dim pEProcess As ULONG64, lpRet As DWORD
    IoControl hDrv, IOCTL_GetEProcess, @ProcessId, SizeOf(DWORD), @pEProcess, SizeOf(ULONG64), @lpRet
    If ProcessId = 1892 Then Print "The EPROCESS Of Process 1852:" & Hex(pEProcess)
    Return pEProcess
End Function

Private Function GetPIDList2(PID() As DWORD) As Integer
    Dim hProcess As HANDLE, j As Integer, k As Integer = 100
    ReDim PID(k)
    For dwPID As DWORD = 0 To &H20000 Step 4
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, dwPID)
        If j > k Then
            k += 100
            ReDim Preserve PID(k)
        End If
        If (hProcess <> 0) Or (GetLastError <> ERROR_INVALID_PARAMETER) Then ' 找到一个进程
            PID(j) = dwPID
            j += 1
            CloseHandle hProcess
        End If
        FF_DoEvents
    Next
    Return j + 1
End Function

Private Function GetPIDList3(PID() As DWORD) As Integer
    Dim pEProcess As ULONG64, lpRet As DWORD, j As Integer, k As Integer = 100
    ReDim PID(k)
    For i As DWORD = 0 To &H20000 Step 4
        pEProcess = GetEProcess(i)
        If j > k Then
            k += 100
            ReDim Preserve PID(k)
        End If
        If pEProcess <> 0 Then
            'Print i & " 0x" & Hex(pEProcess)
            PID(j) = i
            j += 1
        End If
        FF_DoEvents
    Next
    Return j + 1
End Function

Function GetNameByPath(ByVal ProcessPath As StringW) As StringW
    If ProcessPath = "" Then
        Return ""
    ElseIf InStr(ProcessPath, ":") = 0 Then
        Return ProcessPath
    Else
        Return RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
    End If
End Function

Public Function GetProcessPath(ByVal ProcessId As DWORD) As StringW
    Dim ClientId As CLIENT_ID, ObjectAttributes As OBJECT_ATTRIBUTES, hProcess As HANDLE
    Select Case ProcessId
        Case 0
            Return "[System Idle Process]"
        Case 4
            Return "System"
    End Select
    '设置要打开的进程的客户端 ID
    ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, ProcessId))
    ClientId.UniqueThread = NULL

    '初始化 OBJECT_ATTRIBUTES 结构
    InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

    '调用 ZwOpenProcess 打开进程
    Dim st As NTSTATUS = ZwOpenProcess(@hProcess, PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, @ObjectAttributes, @ClientId)
    If (Not NT_SUCCESS(st)) Then
        'Print "ZwOpenProcess Failed!(" & Hex(st) & ")"
        If IsDriverLoaded Then
            hProcess = SnowSword_OpenProcess(ProcessId)
            If hProcess <= 0 Then Return ""
        Else
            Return ""
        End If
    End If
    /'If hProcess Then
        Dim FilePath As WString * MAX_PATH
        GetModuleFileNameExW(hProcess, NULL, @FilePath, MAX_PATH)
        Return FilePath
    End If'/
    
    If hProcess Then
        Dim pathBuffer As WString * MAX_PATH
        Dim length As DWORD
        length = GetProcessImageFileNameW(hProcess, @pathBuffer, MAX_PATH)
        If length = 0 Or pathBuffer = "" Then
            Print "Failed to get process image file name." & WinErrorMsg(GetLastError) & GetLastError
            Return ""
        End If
        Dim ProcessPath As WString * MAX_PATH
        NtPathToDosPathW @pathBuffer, @ProcessPath
        Return ProcessPath
    End If
    
    /'If hProcess Then
        Dim retLength As ULong
        Dim st As NTSTATUS
        Dim imageName As UNICODE_STRING
        Dim pathBuffer As WString * MAX_PATH
        Dim nameLength As UShort

        st = ZwQueryInformationProcess(hProcess, ProcessImageFileName, @imageName, SizeOf(UNICODE_STRING), retLength)
        If st <> 0 Then
            Print "Failed to get process image name."
            Return ""
        End If

        If imageName.Buffer = 0 Then
            Print "Image name buffer is null."
            Return ""
        End If

        nameLength = imageName.Length
        If nameLength = 0 Then
            Print "Image name is empty."
            Return ""
        End If

        CopyMemory(@pathBuffer, imageName.Buffer, nameLength)
        pathBuffer[nameLength / 2] = 0
        Return pathBuffer
    End If'/
    Return ""
End Function

Function GetParentProcessId(ByVal PID As DWORD, ByRef ParentPID As DWORD) As BOOLEAN
    Dim pbi As PROCESS_BASIC_INFORMATION, st As NTSTATUS, ret As DWORD, hProcess As HANDLE
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(PID)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, PID)
    End If
    If (hProcess <= 0) Then
        'Print "[GetParentProcessId]OpenProcess:" & WinErrorMsg(GetLastError)
        Return False
    End If
    st = ZwQueryInformationProcess(hProcess, BasicInformation, @pbi, SizeOf(pbi), ret)
    If (NT_SUCCESS(st)) Then
        ParentPID = Cast(DWORD, Cast(UInteger, pbi.ParentProcessId))
        Return True
    Else
        Return False
    End If
End Function

' 枚举进程及其线程
Sub EnumThread(ByVal dwProcessId As DWORD)
    Dim As Long status
    Dim As ULong bufferSize = 0, returnLength = 0
    Dim As Any Ptr buffer = NULL
    Dim As SYSTEM_PROCESS_INFORMATION Ptr procInfo
    
    ' 第一次调用：获取所需缓冲区大小
    status = ZwQuerySystemInformation1(SystemProcessesAndThreadsInformation, 0, 0, @returnLength)
    If status <> STATUS_INFO_LENGTH_MISMATCH Then  ' 表示需要更大缓冲区
        Print "获取缓冲区大小失败，错误码: " & Hex(status)
        Return
    End If
    
    ' 分配缓冲区（适当增大避免溢出）
    bufferSize = returnLength + 4096
    buffer = Allocate(bufferSize)
    If buffer = NULL Then
        Print "内存分配失败"
        Return
    End If
    Print returnLength
    ' 第二次调用：获取进程信息
    status = ZwQuerySystemInformation1(SystemProcessesAndThreadsInformation, buffer, bufferSize, @returnLength)
    If status <> 0 Then  ' 0表示STATUS_SUCCESS
        Print "获取进程信息失败，错误码: " & Hex(status)
        Deallocate buffer
        Return
    End If
    
    ' 遍历进程信息链表
    /'procInfo = CPtr(SYSTEM_PROCESS_INFORMATION Ptr, buffer)
    For j As ULong = 0 To returnLength / SizeOf(SYSTEM_PROCESS_INFORMATION)
        'If procInfo[j].ProcessId <> dwProcessId Then Continue For
        Print "PID:" & procInfo[j].ProcessId
        ' 遍历当前进程的线程
        For i As ULong = 0 To procInfo[j].ThreadCount - 1
            Print "线程ID: " & procInfo[j].Threads(i).ClientID.UniqueThread & ", 切换次数: " & procInfo[j].Threads(i).ContextSwitchCount
        Next
        If procInfo[j].NextEntryDelta = NULL Then Exit For
    Next'/
' 遍历进程信息链表
    procInfo = buffer
    Do
        ' 解析进程信息
        'Dim As LPWSTR processName = Allocate(SizeOf(WString) * procInfo->ProcessName.Length)
        '*processName = *(procInfo->ImageName.buffer)
        Print "0x" & Hex(procInfo)
        Print "进程ID: " & procInfo->ProcessId' & ", 名称: " & IIf(processName = "", "(未知)", processName)
        Print "  线程数: " & procInfo->ThreadCount
        'AfxMsg "1"
        ' 遍历当前进程的线程
        'For i As ULong = 0 To procInfo->ThreadCount - 1
        '    Print "线程ID: " & procInfo->Threads(i).ClientID.UniqueProcess' & ", 切换次数: " & procInfo->Threads(i).ContextSwitchCount
        'Next
        
        ' 移动到下一个进程（偏移量为0表示结束）
        If procInfo->NextEntryDelta = 0 Then Exit Do
        procInfo = Cast(SYSTEM_PROCESS_INFORMATION Ptr, Cast(Byte Ptr, procInfo) + procInfo->NextEntryDelta)
    Loop While True
    
    Deallocate buffer
End Sub

Public Sub GetProcessList(lvwProcess As Class_ListView, ByVal IsCheckHideProcess As Boolean) ' As Integer
    Dim PID1() As DWORD, ProcessPath() As String, PID2() As DWORD, k As Integer
    Dim dwPID As DWORD
    Dim strEProcess As StringW
    Dim dwParentPID As DWORD
    Dim strParentPID As StringW
    Dim strPath As StringW
    Dim strPath1 As StringW
    Dim strName As StringW
    Dim strCommandLine As StringW
    Dim strCompany As StringW
    Dim errCode As Long
    
    'lvwProcess.ReDraw = False
    lvwProcess.DeleteAllItems
    InitDataNode lvwProcess
    GetAllPID ProcessPath(), PID1(), 0
    
    If IsCheckHideProcess Then
        If IsDriverLoaded Then GetPIDList3 PID2() Else GetPIDList2 PID2()
        For i As Integer = 0 To UBound(PID1) - 1
            Dim ForeColor As COLORREF = FB_Black
            Dim BackColor As COLORREF = FB_White
            dwPID = PID1(i + 1)
            If GetParentProcessId(PID1(i + 1), dwParentPID) Then strParentPID = WStr(dwParentPID)
            strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = ProcessPath(i + 1)
            If InStr(strPath, ":") = 0 Then
                strPath = GetProcessPath(dwPID)
                If InStr(strPath, ":") = 0Then
                    strName = ProcessPath(i + 1)
                Else
                    strName = GetNameByPath(strPath)
                End If
            Else
                strPath = GetProcessPath(dwPID)
                strName = GetNameByPath(strPath)
                strCompany = GetFileCompany(strPath)
            End If
            'Print strPath
            If (strCompany <> "Microsoft Corporation") AndAlso (dwPID <> 0) OrElse _
            (strCompany <> "") AndAlso (Not VerifyFileSign(strPath)) Then SetItemColor lvwProcess, i, ForeColor, FB_GoldenYellow
            lvwProcess.AddItem WStr(dwPID)
            lvwProcess.SetItemText i, 1, WStr(dwParentPID)
            lvwProcess.SetItemText i, 2, strEProcess
            lvwProcess.SetItemText i, 3, strName
            lvwProcess.SetItemText i, 4, strPath
            lvwProcess.SetItemText i, 5, strCompany
            strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strCompany = ""
        Next
        For i As Integer = 0 To UBound(PID2)
            For j As Integer = 1 To UBound(PID1)
                If PID2(i) = PID1(j) Then Continue For, For ' 如果找到了证明不是隐藏进程
            Next
            dwPID = PID2(i)
            If GetParentProcessId(dwPID, dwParentPID) Then strParentPID = WStr(dwParentPID)
            If IsDriverLoaded Then strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = GetProcessPath(dwPID)
            If InStr(strPath, ":") = 0 Then
                strPath = GetProcessPath(dwPID)
                If InStr(strPath, ":") = 0 Then
                    strName = ProcessPath(i + 1)
                Else
                    strName = GetNameByPath(strPath)
                End If
            Else
                strName = GetNameByPath(strPath)
                strCompany = GetFileCompany(strPath)
            End If
            'strCommandLine = GetComman
            'CurrentListViewItem.BackColor = BGR(0, 255, 0) ' 绿色
            SetItemColor lvwProcess, lvwProcess.ItemCount - 1, FB_Red, FB_White
            lvwProcess.AddItem WStr(dwPID)
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 1, WStr(dwParentPID)
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 2, strEProcess
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 3, strName
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 4, strPath
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 6, strCompany
            'CurrentListViewItem.BackColor = FB_White
            strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strCompany = ""
        Next
    Else
        'ReDim CurrentListViewItem(UBound(PID1) - 1) As ListViewItemInfo
        'For i As Long = 0 To UBound(CurrentListViewItem)
            'CurrentListViewItem(i).BackColor = FB_White
            'If i Mod 2 = 0 Then CurrentListViewItem(i).BackColor = FB_GoldenYellow
        'Next
        Dim ItemIndex As Integer
        lvwProcess.AddItemCount UBound(PID1)
        For i As Integer = 0 To UBound(PID1) - 1
            Dim ForeColor As COLORREF = FB_Black
            Dim BackColor As COLORREF = FB_White
            dwPID = PID1(i + 1)
            If PID1(i + 1) = 1004 Then Print GetProcessPath(dwPID) & " " & ProcessPath(i + 1)
            If GetParentProcessId(PID1(i + 1), dwParentPID) Then strParentPID = WStr(dwParentPID)
            If IsDriverLoaded Then strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = ProcessPath(i + 1)
            strPath1 = GetProcessPath(dwPID)
            'Print strPath & " " & strPath1
            If InStr(strPath, ":") = 0 Then
                If InStr(strPath1, ":") = 0 Then
                    strName = ProcessPath(i + 1)
                Else
                    strPath = strPath1
                End If
            Else
                If strPath1 <> "" AndAlso strPath <> strPath1 Then
                    'strPath = strPath1
                    ForeColor = FB_LightRed
                End If
            End If
            strName = GetNameByPath(strPath)
            strCompany = GetFileCompany(strPath)
            
            'If ItemIndex = 1 Then SetItemColor lvwProcess, ItemIndex, ForeColor, FB_GoldenYellow
            /'Dim Node As ListViewDataNode Ptr = FindListViewHwnd(lvwProcess)
            If Node <> NULL Then Node->ItemColor(1).BackColor = FB_GoldenYellow'/
            
            lvwProcess.AddItemColList(,, WStr(dwPID), WStr(dwParentPID), strEProcess, strName, strPath,, strCompany)
            'InvalidateRect lvwProcess.hWnd, NULL, True
            strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strCompany = ""
            ItemIndex += 1
            FF_DoEvents
        Next
    End If
    'lvwProcess.ReDraw = True
End Sub
        /'If IsCheckHideProcess Then GetPIDList2 PID2()
        For i As Integer = 0 To UBound(PID1) - 1
            dwPID = WStr(PID1(i + 1))
            If GetParentProcessId(PID1(i + 1), dwParentPID) Then strParentPID = WStr(dwParentPID)
            'strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = ProcessPath(i + 1)
            If InStr(strPath, ":") = 0 Then
                strName = strPath
            Else
                strName = Right(strPath, Len(strPath) - InStrRev(strPath, "\"))
                strCompany = GetFileCompany(strPath)
            End If
            'If (strCompany <> "") AndAlso (Not VerifyPeSignature(strPath, errCode)) Then CurrentListViewItem.BackColor = BGR(255, 0, 0) ' 可疑进程
            If (strPath <> "") AndAlso (Not VerifyPeSignature(strPath, errCode)) Then CurrentListViewItem.BackColor = FB_Red ' 可疑进程
            lvwProcess.AddItem WStr(dwPID)
            lvwProcess.SetItemText i, 1, WStr(ParentPID)
            lvwProcess.SetItemText i, 2, strEProcess
            lvwProcess.SetItemText i, 3, strName
            lvwProcess.SetItemText i, 4, strPath
            lvwProcess.SetItemText i, 5, strCompany
            CurrentListViewItem.BackColor = FB_White
        Next
        If (Not IsCheckHideProcess) Then Exit 
        For i As Integer = 0 To UBound(PID2)
            For j As Integer = 1 To UBound(PID1)
                If PID2(i) = PID1(j) Then Continue For, For ' 如果找到了证明不是隐藏进程
            Next
            dwPID = PID2(i)
            If GetParentProcessId(dwPID, dwParentPID) Then strParentPID = WStr(dwParentPID)
            'strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = GetProcessPath(dwPID)
            If InStr(strPath, ":") = 0 Then
                strName = strPath
            Else
                strName = Right(strPath, Len(strPath) - InStrRev(strPath, "\"))
                strCompany = GetFileCompany(strPath)
            End If
            CurrentListViewItem.BackColor = BGR(0, 255, 0) ' 绿色
            lvwProcess.AddItem WStr(dwPID)
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 1, WStr(dwParentPID)
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 2, strEProcess
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 3, strName
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 4, strPath
            lvwProcess.SetItemText lvwProcess.ItemCount - 1, 5, strCompany
            CurrentListViewItem.BackColor = FB_White
        Next'/

Public Function SuspendProcess(ByVal dwProcessId As DWORD) As Boolean
    Dim hProcess As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    Dim st As NTSTATUS
    
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        '设置要打开的进程的客户端 ID
        ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
        ClientId.UniqueThread = NULL

        '初始化 OBJECT_ATTRIBUTES 结构
        InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

        '调用 ZwOpenProcess 打开进程
        st = ZwOpenProcess(@hProcess, PROCESS_SUSPEND_RESUME, @ObjectAttributes, @ClientId)
    End If
    If (hProcess = 0) Then
        Print "打开进程失败,原因:" & Hex(st)
        Goto Err1
    End If
    If (ZwSuspendProcess(hProcess) <> 0) Then
        Print "挂起进程失败,原因:" & Hex(st)
        Goto Err1
    End If
    Return True
    Err1 : 
    ZwClose hProcess
    Return False
End Function

Public Function ResumeProcess(ByVal dwProcessId As DWORD) As Boolean
    Dim hProcess As HANDLE = NULL
    Dim ClientId As CLIENT_ID
    Dim ObjectAttributes As OBJECT_ATTRIBUTES
    Dim st As NTSTATUS
    
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        '设置要打开的进程的客户端 ID
        ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, dwProcessId))
        ClientId.UniqueThread = NULL

        '初始化 OBJECT_ATTRIBUTES 结构
        InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

        '调用 ZwOpenProcess 打开进程
        st = ZwOpenProcess(@hProcess, PROCESS_SUSPEND_RESUME, @ObjectAttributes, @ClientId)
    End If
    If (hProcess = 0) Then
        Print "打开进程失败,原因:" & Hex(st)
        Goto Err1
    End If
    If (ZwResumeProcess(hProcess) <> 0) Then
        Print "恢复进程失败,原因:" & Hex(st)
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
    Return True;
}'/

Public Function KillProcess(ByVal dwProcessId As DWORD) As Boolean
    Dim IsCritical As BOOLEAN
    If (IsProcessCritical(dwProcessId, @IsCritical)) Then
        If IsCritical Then
            If (AfxMsg("此进程处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDNO) Then
                Return False
            Else
                SetCriticalProcess dwProcessId, False
            End If
        End If
    End If
    Dim hProcess As HANDLE = OpenProcess(PROCESS_TERMINATE, False, dwProcessId)
    If (hProcess = 0) Then
        Print "打开进程失败,原因:" & WinErrorMsg(GetLastError)
        Goto Err1
    End If
    If (TerminateProcess(hProcess, 0) = 0) Then
        Print "结束进程失败,原因:" & WinErrorMsg(GetLastError)
        Goto Err1
    End If
    Print "结束进程成功!"
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

Sub GetProcessTimerList(ByVal dwProcessId As DWORD, lvwWindowTimer As Class_ListView)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    Dim SomeThing As DO_SOMETHING, ProcessTimer() As PROCESS_TIMER, lpRet As DWORD
    SomeThing.ProcessId = dwProcessId
    IoControl hDrv, IOCTL_EnumProcessTimers, @SomeThing, SizeOf(DO_SOMETHING), NULL, 0, @lpRet
    If (GetLastError <> 0) Then
        Print "[GetProcessTimerList]第一次IoControl失败:" & WinErrorMsg(GetLastError) & GetLastError
        Exit Sub
    End If
    Print lpRet & " " & SizeOf(PROCESS_TIMER)
    'ReDim ProcessTimer(lpRet / SizeOf(PROCESS_TIMER) - 1) As PROCESS_TIMER
    'IoControl hDrv, IOCTL_EnumProcessTimers,,,@ProcessTimer(0), (UBound(ProcessTimer) + 1) * SizeOf(PROCESS_TIMER), @lpRet
    'For i As DWORD = 0 To UBound(ProcessTimer)
        'lvwWindowTimer.AddItemColList ,,WStr(ProcessTimer(i).ThreadId), WStr(ProcessTimer(i).Period), "0x" & WHex(ProcessTimer(i).Func)
    'Next
    Erase ProcessTimer
End Sub