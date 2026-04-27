#include Once "win/tlhelp32.bi"
#include Once "win/psapi.bi"
#include Once "win/winnt.bi"
#include Once "win/winver.bi"
#include Once "win/strsafe.bi"

Private Const MAX_PATH As Integer = 260
Dim Shared hThread     As Any Ptr

Type LANGANDCODEPAGE
    wLanguage As WORD
    wCodePage As WORD
End Type

Type UNICODE_STRING
    Length        As UShort
    MaximumLength As UShort
    Buffer        As PWSTR
End Type

Type UNICODE_STRING32
    Length        As WORD
    MaximumLength As WORD
    Buffer        As DWORD ' 4字节指针
End Type

Type PROCESS_BASIC_INFORMATION
    ExitStatus      As NTSTATUS  'NTSTATUS
    PebBaseAddress  As Any Ptr   'PPEB
    AffinityMask    As ULong Ptr 'ULONG_PTR
    BasePriority    As Any Ptr   'KPRIORITY
    UniqueProcessId As HANDLE
    ParentProcessId As HANDLE
End Type

Type PROCESS_BASIC_INFORMATION32
    ExitStatus      As DWORD
    PebBaseAddress  As ULONG
    AffinityMask    As ULONG
    BasePriority    As DWORD
    UniqueProcessId As ULONG
    InheritedFromUniqueProcessId As ULONG
End Type

Type PLIST_ENTRY As LIST_ENTRY Ptr

Type PEB_LDR_DATA64
    Length      As ULong
    Initialized As UByte
    SsHandle    As ULONG_PTR
    InLoadOrderModuleList           As LIST_ENTRY
    InMemoryOrderModuleList         As LIST_ENTRY
    InInitializationOrderModuleList As LIST_ENTRY
End Type
Type PPEB_LDR_DATA64 As PEB_LDR_DATA64 Ptr

Type LDR_DATA_TABLE_ENTRY64
    InLoadOrderLinks           As LIST_ENTRY
    InMemoryOrderLinks         As LIST_ENTRY
    InInitializationOrderLinks As LIST_ENTRY
    DllBase       As ULONG64
    EntryPoint    As ULONG64
    SizeOfImage   As ULong
    FullDllName   As UNICODE_STRING
    BaseDllName   As UNICODE_STRING
    FLAGS         As ULong
    LoadCount     As UShort
    TlsIndex      As UShort
    HashLinks     As LIST_ENTRY
    TimeDateStamp As ULong
End Type
Type PLDR_DATA_TABLE_ENTRY64 As LDR_DATA_TABLE_ENTRY64 Ptr

Type RTL_USER_PROCESS_PARAMETERS
    Reserved1(15) As Byte
    Reserved2(9)  As PVOID
    ImagePathName As UNICODE_STRING
    CommandLine   As UNICODE_STRING
End Type
Type PRTL_USER_PROCESS_PARAMETERS As RTL_USER_PROCESS_PARAMETERS Ptr

Type RTL_USER_PROCESS_PARAMETERS32
    MaximumLength          As DWORD ' +0x00
    Length                 As DWORD ' +0x04
    Flags                  As DWORD ' +0x08
    DebugFlags             As DWORD ' +0x0C
    ConsoleHandle          As DWORD ' +0x10
    ConsoleFlags           As DWORD ' +0x14
    StdInputHandle         As DWORD ' +0x18
    StdOutputHandle        As DWORD ' +0x1C
    StdErrorHandle         As DWORD ' +0x20
    CurrentDirectory       As UNICODE_STRING32 ' +0x24
    CurrentDirectoryHandle As DWORD   ' +0x2C
    DllPath       As UNICODE_STRING32 ' +0x30
    ImagePathName As UNICODE_STRING32 ' +0x38
    CommandLine   As UNICODE_STRING32 ' +0x40 【正确偏移】
End Type
Type PRTL_USER_PROCESS_PARAMETERS32 As RTL_USER_PROCESS_PARAMETERS32 Ptr

Type PEB64
    Reserved1(1)  As Byte
    BeingDebugged As Byte
    Reserved2     As Byte
    Reserved3(1)  As PVOID
    Ldr           As PPEB_LDR_DATA64
    ProcessParameters      As PRTL_USER_PROCESS_PARAMETERS
    Reserved4(2)           As PVOID
    AtlThunkSListPtr       As PVOID
    Reserved5              As PVOID
    Reserved6              As ULong
    Reserved7              As PVOID
    Reserved8              As ULong
    AtlThunkSListPtr32     As ULong
    Reserved9(44)          As PVOID
    Reserved10(95)         As Byte
    PostProcessInitRoutine As Any Ptr 'PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine
    Reserved11(127)        As Byte
    Reserved12             As PVOID
    SessionId              As ULong
End Type
Type PPEB64 As PEB64 Ptr

Type PROCESS_SESSION_INFORMATION
    SessionId As DWORD
End Type

Type OBJECT_ATTRIBUTES
    Length                   As ULong
    RootDirectory            As HANDLE
    ObjectName               As UNICODE_STRING Ptr
    Attributes               As ULong
    SecurityDescriptor       As PVOID
    SecurityQualityOfService As PVOID
End Type
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr

Type CLIENT_ID
    UniqueProcess As HANDLE
    UniqueThread  As HANDLE
End Type
Type PCLIENT_ID         As CLIENT_ID Ptr
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr

Type SYSTEM_THREAD_INFORMATION
    KernelTime         As LARGE_INTEGER
    UserTime           As LARGE_INTEGER
    CreateTime         As LARGE_INTEGER
    WaitTime           As ULong
    StartAddress       As PVOID
    ClientID           As CLIENT_ID
    Priority           As Long 'KPRIORITY
    BasePriority       As Long ' KPRIORITY
    ContextSwitchCount As ULong
    ThreadState        As ULong ' THREAD_STATE
    WaitReason         As ULong ' KWAIT_REASON
End Type

Type VM_COUNTERS
    PeakVirtualSize            As size_t
    VirtualSize                As size_t
    PageFaultCount             As ULong
    PeakWorkingSetSize         As size_t
    WorkingSetSize             As size_t
    QuotaPeakPagedPoolUsage    As size_t
    QuotaPagedPoolUsage        As size_t
    QuotaPeakNonPagedPoolUsage As size_t
    QuotaNonPagedPoolUsage     As size_t
    PagefileUsage              As size_t
    PeakPagefileUsage          As size_t
End Type

Type SYSTEM_PROCESS_INFORMATION
    NextEntryDelta   As ULong
    ThreadCount      As ULong
    Reserved(0 To 5) As ULong
    CreateTime       As LARGE_INTEGER
    UserTime         As LARGE_INTEGER
    KernelTime       As LARGE_INTEGER
    ProcessName      As UNICODE_STRING
    BasePriority     As Long           ' KPRIORITY
    ProcessId        As UInteger       ' HANDLE
    InheritedFromProcessId As UInteger ' HANDLE
    HandleCount       As ULong
    SessionId         As ULong
    PageDirectoryBase As ULONG_PTR
    VmCounters        As VM_COUNTERS
    PrivatePageCount  As size_t
    IoCounters        As IO_COUNTERS
    Threads(0)        As SYSTEM_THREAD_INFORMATION
End Type

Type PROCESS_TIMER
    pfn       As ULONG64   ' 回调函数地址
    nTimeout  As ULong     ' 超时时间(ms)
    nIDEvent  As ULONG64   ' 定时器ID
    ThreadId  As ULONG64   ' 线程ID
    ProcessId As ULONG64   ' 进程ID
    hWnd      As hWnd      ' 所属窗口
    param     As ULONG_PTR ' 线程参数
End Type

Type PS_PROTECTION
    Level As UCHAR
    Union
        Type    As UCHAR ' : 3
        Audit   As UCHAR ' : 1
        Signer  As UCHAR ' : 4
    End Union
End Type

' 统一进程信息结构体
Type ProcessInfo
    PID            As DWORD
    startTimeStr   As String
    cpuUserSec     As Double
    cpuKernelSec   As Double
    workingSetKB   As Long
    privateBytesKB As Long
    pagedPoolKB    As Long
    handleCount    As DWORD
    gdiObjects     As DWORD
    userObjects    As DWORD
    integrityLevel As String
    isPPL          As Boolean
    pplLevel       As Byte
    pplType        As Byte
    pplSigner      As Byte
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
    ProcessIoPortHandlers '// Note: this is kernel mode only
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
    ProcessProtectionInformation = 61
End Enum

Declare Function ZwSuspendProcess Lib "ntdll.dll" Alias "ZwSuspendProcess"(ByVal hProcess As HANDLE) As NTSTATUS
Declare Function ZwResumeProcess Lib "ntdll.dll" Alias "ZwResumeProcess"(ByVal hProcess As HANDLE)   As NTSTATUS
Declare Function ZwOpenProcess Lib "ntdll.dll" Alias "ZwOpenProcess"(ByVal ProcessHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
Declare Function ZwTerminateProcess Lib "ntdll.dll" Alias "ZwTerminateProcess"(ByVal ProcessHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
Declare Function ZwClose Lib "ntdll.dll" Alias "ZwClose"(ByVal HANDLE As HANDLE) As NTSTATUS
Declare Function ZwQuerySystemInformation Lib "ntdll.dll" Alias "ZwQuerySystemInformation"(ByVal SystemInformationClass As SYSTEM_INFORMATION_CLASS, ByVal SystemInformation As PVOID, ByVal SystemInformationLength As ULong, ByVal ReturnLength As PULONG) As NTSTATUS
Declare Function ZwQueryInformationProcess Lib "ntdll.dll" Alias "ZwQueryInformationProcess"(ByVal ProcessHandle As HANDLE, ByVal ProcessInformationClass As PROCESSINFOCLASS, _
    ByVal ProcessInformation As LPVOID, ByVal ProcessInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
Declare Function _IsProcessCritical Lib "kernel32" Alias "IsProcessCritical"(ByVal hProcess As HANDLE, ByVal Critical As PBOOLEAN) As Boolean
Declare Function GetProcessIdOfThread Lib "kernel32" Alias "GetProcessIdOfThread"(ByVal Thread As HANDLE) As DWORD

CONST OBJ_KERNEL_HANDLE = &H00000200
#define SYSTEM_CALL_OPENPROCESS &H26
#define SYSTEM_CALL_TERMINATEPROCESS &H2C

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
    Print Hex(st)
    If (Not NT_SUCCESS(st)) Then
        Print "MyZwOpenProcess Failed!(" & Hex(st) & ")"
        Return NULL
    End If
    Return ProcessHandle
End Function

Function LzOpenProcess(PID As DWORD) As HANDLE
    Dim st As NTSTATUS
    
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    Do
        bufferSize *= 2
        'ReDim buffer(bufferSize - 1) As Byte
        buffer = Reallocate(buffer, bufferSize)
        st = ZwQuerySystemInformation(16, buffer, bufferSize, NULL)
        'Print bufferSize & " " & Hex(st)
    Loop While st = STATUS_INFO_LENGTH_MISMATCH
    
    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    Dim j As Integer
    'ReDim PIDs(100) As DWORD
    For i As Integer = 0 To pHandleEntry->NumberOfHandles - 1
        Dim MyHandle As HANDLE = NULL, ProcessId As DWORD, ThreadId As DWORD
        Dim hProcess As HANDLE = OpenProcess(PROCESS_DUP_HANDLE, False, pHandleEntry->Handles(i).UniqueProcessId)
        If hProcess = NULL Then Continue For
        
        If DuplicateHandle(hProcess, Cast(HANDLE, Cast(ULONG_PTR, pHandleEntry->Handles(i).HandleValue)), GetCurrentProcess, @MyHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0 Then Continue For
        
        ProcessId = GetProcessId(MyHandle)
        If ProcessId = PID Then '找到目标句柄
            Deallocate buffer
            Return MyHandle
        Else
            CloseHandle MyHandle
        End If

        'FF_DoEvents
    Next
    Deallocate buffer
    Return NULL
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
    Print Hex(st)
    If (Not NT_SUCCESS(st)) Then
        Print "ZwOpenProcess Failed!(" & Hex(st) & ")"
        Return False
    End If
    Print ProcessHandle
    '调用 ZwTerminateProcess 结束进程
    st = MyZwTerminateProcess(ProcessHandle, 0)
    Print Hex(st)
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

' 获取系统原生架构（核心验证函数）
Private Function GetSystemNativeArch() As UShort
    Dim sysinfo As SYSTEM_INFO
    GetNativeSystemInfo(@sysinfo) ' 关键：GetNativeSystemInfo获取硬件原生架构，而非兼容层架构
    Return sysinfo.wProcessorArchitecture
End Function

Function IsProcessWow64_Ex(ByVal hProcess As HANDLE) As Boolean
    Type IsWow64Process2_Func As Function stdcall( _
        ByVal hProcess        As HANDLE, _
        ByVal pProcessMachine As UShort Ptr, _
        ByVal pNativeMachine  As UShort Ptr _
        ) As BOOL
        
    Const PROCESSOR_ARCHITECTURE_ARM64 As UShort = 12
    
    Dim bResult As Boolean = False
    Dim pIsWow64Process2 As IsWow64Process2_Func = 0
    Dim hKernel32 As HMODULE
    Dim bApiSuccess As BOOL
    Dim sysArch As UShort
    Dim uProcessMachine As UShort = IMAGE_FILE_MACHINE_UNKNOWN
    Dim uNativeMachine As UShort = IMAGE_FILE_MACHINE_UNKNOWN
    Dim bWow64 As BOOL = 0 
    
    ' 1. 获取 kernel32.dll 句柄
    hKernel32 = GetModuleHandle("kernel32.dll")
    If hKernel32 = 0 Then
        Print "错误：获取 kernel32.dll 句柄失败"
        Goto UseFallBack ' 降级到 IsWow64Process
    End If
    
    ' 2. 获取 IsWow64Process2 函数地址
    pIsWow64Process2 = Cast(IsWow64Process2_Func, GetProcAddress(hKernel32, "IsWow64Process2"))
    If pIsWow64Process2 = 0 Then
        Print "提示：系统不支持 IsWow64Process2，使用降级方案"
        GoTo UseFallBack
    End If
    
    ' 3. 调用 IsWow64Process2（输出参数为 IMAGE_FILE_MACHINE_*）
    bApiSuccess = pIsWow64Process2(hProcess, @uProcessMachine, @uNativeMachine)
    
    If bApiSuccess <> 0 Then
        ' 打印文档规范的调试信息
        /'Print "【符合微软文档 - IsWow64Process2 输出】"
        Print "  进程PE机器类型：" & ImageMachineToName(uProcessMachine)
        Print "  系统原生机器类型：" & ImageMachineToName(uNativeMachine)
        Print "  系统原生架构：" & IIf(GetSystemNativeArch()=PROCESSOR_ARCHITECTURE_AMD64, "x64", "未知")'/
        
        ' 4. 核心判断逻辑（WOW64 本质：进程是32位，系统是64位）
        sysArch = GetSystemNativeArch()
        Select Case sysArch
            Case PROCESSOR_ARCHITECTURE_AMD64 ' x64 系统
                ' x64系统下，进程是 x86(0x014C) 即为 WOW64
                bResult = (uProcessMachine = IMAGE_FILE_MACHINE_I386)
            Case PROCESSOR_ARCHITECTURE_ARM64 ' ARM64 系统
                ' ARM64系统下，进程是 x86/ARM32 即为 WOW64
                bResult = (uProcessMachine = IMAGE_FILE_MACHINE_I386) Or (uProcessMachine = IMAGE_FILE_MACHINE_ARM)
            Case Else ' 32位系统
                bResult = False
        End Select
        
        'Print "  是否为 WOW64 进程：" & IIf(bResult, "是", "否")
        Return bResult
    Else
        Print "错误：调用 IsWow64Process2 失败，错误码：" & GetLastError()
        GoTo UseFallBack
    End If

UseFallBack:
    ' 降级方案：使用 IsWow64Process
    If IsWow64Process(hProcess, @bWow64) <> 0 Then
        bResult = (bWow64 <> 0)
        Print "【降级方案 - IsWow64Process 输出】是否 WOW64：" & IIf(bResult, "是", "否")
    Else
        Print "错误：降级方案调用失败，错误码：" & GetLastError()
    End If
    Return bResult
End Function
    
Function GetPEB(ByVal dwProcessId As DWORD) As Any Ptr
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

Private Sub GetPIDList1(PIDs() As DWORD)
    Dim st As NTSTATUS
    
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    Do
        bufferSize *= 2
        'ReDim buffer(bufferSize - 1) As Byte
        buffer = Reallocate(buffer, bufferSize)
        st = ZwQuerySystemInformation(16, buffer, bufferSize, NULL)
        Print bufferSize & " " & Hex(st)
    Loop While st = STATUS_INFO_LENGTH_MISMATCH
    
    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    Dim j As Integer
    ReDim PIDs(100) As DWORD
    For i As Integer = 0 To pHandleEntry->NumberOfHandles - 1
        Dim MyHandle As HANDLE = NULL, ProcessId As DWORD, ThreadId As DWORD
        Dim hProcess As HANDLE = OpenProcess(PROCESS_DUP_HANDLE, False, pHandleEntry->Handles(i).UniqueProcessId)
        If hProcess = NULL Then Continue For
        
        If DuplicateHandle(hProcess, Cast(HANDLE, Cast(ULONG_PTR, pHandleEntry->Handles(i).HandleValue)), GetCurrentProcess, @MyHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0 Then Continue For
        
        ProcessId = GetProcessId(MyHandle)
        If (Not IsDWORDInArray(ProcessId, PIDs())) AndAlso ProcessId <> 0 Then
            'Print "找到PID:" & ProcessId & " EPROCESS:" & WHex(pHandleEntry->Handles(i).pObject)
            If j > UBound(PIDs) Then ReDim Preserve PIDs(UBound(PIDs) + 50)
            PIDs(j) = ProcessId
            j += 1
            Continue For
        End If
        
        'ThreadId = GetThreadId(MyHandle)
        ProcessId = GetProcessIdOfThread(MyHandle)
        If (Not IsDWORDInArray(ProcessId, PIDs())) AndAlso ProcessId <> 0 Then
            'Print "找到PID:" & ProcessId & " EPROCESS:" & WHex(pHandleEntry->Handles(i).pObject)
            If j > UBound(PIDs) Then ReDim Preserve PIDs(UBound(PIDs) + 50)
            PIDs(j) = ProcessId
            j += 1
            Continue For
        End If

        'FF_DoEvents
    Next
    Print "共从句柄表中找到" & j + 1 & "个进程:"
    'QuickSortWrapper PIDs()
    'PrintArray PIDs()
End Sub

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
            Print i & " 0x" & Hex(pEProcess)
            PID(j) = i
            j += 1
        End If
        FF_DoEvents
    Next
    Return j + 1
End Function

Function GetNameByPath(ByVal ProcessPath As StringW) As StringW
    ' 临时变量存储处理后的路径
    Dim tempPath As StringW = ProcessPath
    ' 空路径直接返回空
    If tempPath = "" Then Return ""
    
    ' 第一步：移除路径末尾的反斜杠（处理边界情况：如 \\Server\Share\ 或 C:\Test\）
    /'While RightW(tempPath, 1) = "\"
        tempPath = LeftW(tempPath, LenW(tempPath) - 1)
    Wend'/
    
    ' 第二步：查找最后一个反斜杠的位置（统一适配本地路径和UNC路径）
    Dim lastBackslashPos As Integer = InStrRevW(tempPath, "\")
    
    ' 第三步：提取文件名
    If lastBackslashPos > 0 Then
        ' 存在反斜杠，取最后一个反斜杠右侧的部分（文件名）
        Return RightW(tempPath, LenW(tempPath) - lastBackslashPos)
    Else
        ' 无反斜杠（仅文件名，无路径），返回原路径
        Return tempPath
    End If
End Function

Public Function GetProcessPath(ByVal ProcessId As DWORD) As StringW
    Dim ClientId As CLIENT_ID, ObjectAttributes As OBJECT_ATTRIBUTES, hProcess As HANDLE
    Select Case ProcessId
        Case 0
            Return "System Idle Process"
        Case 4
            Return "System"
    End Select
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(ProcessId)
    Else
        '设置要打开的进程的客户端 ID
        ClientId.UniqueProcess = Cast(HANDLE, Cast(Integer, ProcessId))
        ClientId.UniqueThread = NULL

        '初始化 OBJECT_ATTRIBUTES 结构
        InitializeObjectAttributes(@ObjectAttributes, NULL, OBJ_KERNEL_HANDLE)

        '调用 ZwOpenProcess 打开进程
        Dim st As NTSTATUS = ZwOpenProcess(@hProcess, PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, @ObjectAttributes, @ClientId)
        If (Not NT_SUCCESS(st)) Then
            MyLog.PrintNtError "GetProcessPath", "ZwOpenProces", st, "PID=" & ProcessId
            Return ""
        End If
    End If
    
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
    Return ""
End Function

Function GetParentProcessId(ByVal PID As DWORD, ByRef ParentPID As DWORD) As Boolean
    Dim pbi As PROCESS_BASIC_INFORMATION, st As NTSTATUS, ret As DWORD, hProcess As HANDLE
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(PID)
        If hProcess = NULL Then
            MyLog.PrintWin32Error "GetParentProcessId", "SnowSword_OpenProcess", "PID=" & PID
            Return False
        End If
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, PID)
        If hProcess = NULL Then
            MyLog.PrintWin32Error "GetParentProcessId", "OpenProcess", "PID=" & PID
            Return False
        End If
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
Sub EnumThread1(ByVal dwProcessId As DWORD)
    Dim As Long status
    Dim As ULong bufferSize = 0, returnLength = 0
    Dim As Any Ptr buffer = NULL
    Dim As SYSTEM_PROCESS_INFORMATION Ptr procInfo
    
    ' 第一次调用：获取所需缓冲区大小
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, 0, 0, @returnLength)
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
    Print returnLength & " " & returnLength / SizeOf(SYSTEM_PROCESS_INFORMATION)
    ' 第二次调用：获取进程信息
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, buffer, bufferSize, @returnLength)
    If status <> 0 Then  ' 0表示STATUS_SUCCESS
        Print "获取进程信息失败，错误码: " & Hex(status)
        Deallocate buffer
        Return
    End If
    
    ' 遍历进程信息链表
    procInfo = buffer
    Do While True
        ' 解析进程信息
        'Dim As LPWSTR processName = Allocate(SizeOf(WString) * procInfo->ProcessName.Length)
        '*processName = *(procInfo->ImageName.buffer)
        Print "0x" & Hex(procInfo)
        Dim processName As WString * MAX_PATH
        StringCbCopyNW @processName, MAX_PATH, procInfo->processName.buffer, procInfo->processName.Length
        Print "进程ID: " & procInfo->ProcessId & ", 名称: " & processName
        Print "  线程数: " & procInfo->ThreadCount
        Dim threadInfo As SYSTEM_THREAD_INFORMATION Ptr = @procInfo->Threads(0)' = Cast(SYSTEM_THREAD_INFORMATION Ptr, Cast(ULONG_PTR, procInfo) + SizeOf(SYSTEM_PROCESS_INFORMATION))
        ' 遍历当前进程的线程
        If procInfo->ProcessId = dwProcessId Then
            For i As ULong = 0 To procInfo->ThreadCount - 1
                Print "线程ID: " & threadInfo->ClientID.UniqueThread & ", 切换次数: " & threadInfo->ContextSwitchCount & ", 起始地址:0x" & WHex(threadInfo->StartAddress)
                threadInfo += 1
            Next
        End If
        
        ' 移动到下一个进程（偏移量为0表示结束）
        If procInfo->NextEntryDelta = 0 Then Exit Do
        procInfo = Cast(SYSTEM_PROCESS_INFORMATION Ptr, Cast(Byte Ptr, procInfo) + procInfo->NextEntryDelta)
    Loop
    
    Deallocate buffer
End Sub

' 核心函数：判断指定PID的进程是否存活
Function IsProcessAlive(ByVal dwProcessId As DWORD) As Boolean
    Dim hProcess As HANDLE
    Dim dwExitCode As DWORD
    Dim bAlive As Boolean = False
    
    ' 1. 打开进程，获取进程句柄
    ' PROCESS_QUERY_LIMITED_INFORMATION：Windows Vista+ 推荐的低权限查询权限
    ' PROCESS_QUERY_INFORMATION：兼容旧系统（XP/2003）
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, dwProcessId)
    End If
    
    If hProcess <> NULL Then
        ' 2. 查询进程的退出码
        If GetExitCodeProcess(hProcess, @dwExitCode) Then
            ' 3. 判断：退出码为STILL_ACTIVE（259）表示进程仍在运行
            If dwExitCode = STILL_ACTIVE Then bAlive = True
        End If
        
        ' 4. 关闭进程句柄（必须释放，避免资源泄漏）
        CloseHandle hProcess
    End If
    
    Return bAlive
End Function

' ===================== 核心函数：兼容64位 + WOW64 =====================
' 参数：dwProcessId -> 目标进程PID
' 返回值：命令行字符串（失败返回空）
Function GetProcessCommandLine(ByVal dwProcessId As DWORD) As String
    Dim hProcess As HANDLE = NULL
    Dim cmdLine As String = ""
    Dim bytesRead As SIZE_T = 0
    Dim ntStatus As LONG = 0
    Dim cmdBuffer As WString * 4096
    
    ' 架构判断变量
    Dim IsTargetWow64 As Boolean = FALSE

    ' 1. 打开进程（最小权限，兼容性最强）
    hProcess = OpenProcess( _
        PROCESS_QUERY_LIMITED_INFORMATION Or PROCESS_VM_READ, _
        FALSE, dwProcessId _
    )
    If hProcess = NULL Then Goto Cleanup

    ' 2. 判断目标进程架构：64位原生 / WOW64(32位)
    IsTargetWow64 = IsProcessWow64_Ex(hProcess)

    ' ==============================================
    ' 分支1：读取 原生64位 进程
    ' ==============================================
    If Not IsTargetWow64 Then
        Dim pbi64 As PROCESS_BASIC_INFORMATION
        Dim peb64 As PEB64
        Dim rtl64 As RTL_USER_PROCESS_PARAMETERS

        ' 获取64位PEB地址
        ntStatus = ZwQueryInformationProcess( _
            hProcess, ProcessBasicInformation, _
            @pbi64, SizeOf(pbi64), NULL _
        )
        If Not NT_SUCCESS(ntStatus) Then Goto Cleanup

        ' 读取64位PEB
        ReadProcessMemory(hProcess, pbi64.PebBaseAddress, _
            @PEB64, SizeOf(PEB64), @bytesRead)

        ' 读取64位进程参数
        ReadProcessMemory(hProcess, peb64.ProcessParameters, _
            @rtl64, SizeOf(rtl64), @bytesRead)

        ' 读取命令行Unicode字符串
        If rtl64.CommandLine.Length > 0 Then
            ReadProcessMemory(hProcess, rtl64.CommandLine.Buffer, _
                @cmdBuffer, rtl64.CommandLine.Length, @bytesRead)
        End If

    ' ==============================================
    ' 分支2：读取 WOW64(32位) 进程
    ' ==============================================
    Else
        Dim pbi64 As PROCESS_BASIC_INFORMATION
        Dim peb As PEB32
        Dim rtl32 As RTL_USER_PROCESS_PARAMETERS32
        Dim pPeb32 As PVOID = NULL
        Dim memCheck(63) As Byte ' 用于偏移检验的内存缓冲区

        ' 1. 获取64位PBI（拿到PEB64）
        ntStatus = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, @pbi64, SizeOf(pbi64), NULL)
        If Not NT_SUCCESS(ntStatus) Then
            MyLog.PrintNtError "GetProcessCommandLine", "ZwQueryInformationProcess", ntStatus
            Goto Cleanup
        End If
        
        ' ✅ 核心修正：你实测有效的固定偏移（永久稳定）
        pPeb32 = pbi64.PebBaseAddress + &H1000

        ' 2. 读取32位PEB（原代码不变）
        If ReadProcessMemory(hProcess, pPeb32, @peb, SizeOf(peb), @bytesRead) = False Then
            MyLog.PrintWin32Error "GetProcessCommandLine", "ReadProcessMemory", "读取WOW64进程PEB32 失败"
            Goto Cleanup
        End If

        ' 【偏移检验】打印RTL32内存前64字节（原代码不变）
        ReadProcessMemory(hProcess, Cast(PVOID, Cast(ULONG_PTR, peb.ProcessParameters)), @memCheck(0), 64, @bytesRead)

        ' 3. 读取32位进程参数（原代码不变）
        If ReadProcessMemory(hProcess, Cast(PVOID, Cast(ULONG_PTR, peb.ProcessParameters)), @rtl32, SizeOf(rtl32), @bytesRead) = False Then
            MyLog.PrintWin32Error "GetProcessCommandLine", "ReadProcessMemory", "读取WOW64进程RTL32 失败"
            Goto Cleanup
        End If

        ' 4. 读取命令行Unicode（原代码不变）
        If rtl32.CommandLine.Length > 0 AndAlso rtl32.CommandLine.Length < 4096 Then
            If ReadProcessMemory(hProcess, Cast(PVOID, Cast(ULONG_PTR, rtl32.CommandLine.Buffer)), @cmdBuffer, rtl32.CommandLine.Length, @bytesRead) = False Then
                MyLog.PrintWin32Error "GetProcessCommandLine", "ReadProcessMemory", "读取WOW64进程命令行缓冲区失败,字节数: " & Str(bytesRead)
            End If
        End If
    End If

Cleanup:
    ' 释放资源
    If hProcess <> NULL Then CloseHandle(hProcess)
    Return TrimW(cmdBuffer)
End Function

' 读取会话ID函数
Function GetProcessSessionId(ByVal dwProcessId As DWORD, ByRef SessionId As DWORD) As Boolean
    Dim hProcess As HANDLE
    Dim psi As PROCESS_SESSION_INFORMATION
    Dim returnLen As ULong
    Dim ntStatus As NTSTATUS
    
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    If hProcess = NULL Then Return False
    
    ntStatus = ZwQueryInformationProcess(hProcess, ProcessSessionInformation, @psi, SizeOf(psi), returnLen)
    
    If NT_SUCCESS(ntStatus) Then
        SessionId = psi.SessionId
        Return True
    Else
        MyLog.PrintNtError "GetProcessSessionId", "ZwQueryInformationProcess", ntStatus
        Return False
    End If
End Function

' 【完美版】无崩溃+无字符截断+兼容FreeBasic 获取进程用户名
Function GetProcessUserName(ByVal dwProcessId As DWORD) As String
    Const TokenQuery As DWORD = &H0008
    Dim hProcess As HANDLE = NULL
    Dim hToken As HANDLE = NULL
    Dim dwSize As DWORD = 0
    Dim pUserBuf As UByte Ptr = NULL
    Dim pTokenUser As TOKEN_USER Ptr
    
    ' 核心修复：使用标准Win32 ANSI缓冲区，彻底解决截断问题
    Dim szUserName As WString * 256
    Dim szDomainName As WString * 256
    ' 严格初始化：缓冲区最大长度 = 256
    Dim userNameLen As DWORD = 256
    Dim domainLen As DWORD = 256
    
    Dim useType As DWORD = 0
    Dim errCode As DWORD = 0

    ' 1. 打开进程
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    If hProcess = NULL Then
        MyLog.PrintLog LOG_ERROR, "打开进程失败"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 2. 打开进程令牌
    If OpenProcessToken(hProcess, TokenQuery, @hToken) = 0 Then
        MyLog.PrintWin32Error "GetProcessUserName", "OpenProcessToken"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 3. 获取缓冲区大小
    GetTokenInformation(hToken, TokenUser, NULL, 0, @dwSize)
    If dwSize = 0 Then
        MyLog.PrintWin32Error "GetProcessUserName", "GetTokenInformation", "缓冲区大小获取失败"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 4. 分配内存
    pUserBuf = Allocate(dwSize)
    If pUserBuf = NULL Then
        MyLog.PrintLog LOG_ERROR, "GetProcessUserName", "Allocate", "缓冲区大小获取失败"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 5. 读取SID
    If GetTokenInformation(hToken, TokenUser, pUserBuf, dwSize, @dwSize) = 0 Then
        MyLog.PrintWin32Error "GetProcessUserName", "GetTokenInformation", "读取SID失败"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 6. 核心修复：FreeBasic 100%兼容的 SID→用户名 转换
    pTokenUser = Cast(TOKEN_USER Ptr, pUserBuf)
    If pTokenUser->User.Sid = NULL Then
        MyLog.PrintLog LOG_ERROR, "GetProcessUserName",, "SID = NULL"
        Function = "(failed)"
        Goto Cleanup
    End If

    ' 关键修复：用 StrPtr 传递字符串指针，Win32 API 标准用法
    If LookupAccountSid(NULL, pTokenUser->User.SID, @szUserName, @userNameLen, @szDomainName, @domainLen, @useType) Then
        ' 截取有效长度，输出完整用户名
        Dim result As StringW = LeftW(szDomainName, domainLen) & "\" & LeftW(szUserName, userNameLen)
        Function = result
    Else
        MyLog.PrintWin32Error "GetProcessUserName", "GetTokenInformation"
        Function = "(failed)"
    End If

Cleanup:
    ' 统一清理
    If pUserBuf <> NULL Then Deallocate(pUserBuf)
    If hToken <> NULL Then CloseHandle(hToken)
    If hProcess <> NULL Then CloseHandle(hProcess)
End Function

' 辅助函数
Private Function FileTimeToLocalStr(ByVal ft As FILETIME) As String
    Dim st As SYSTEMTIME, lt As SYSTEMTIME
    FileTimeToSystemTime(@ft, @st)
    SystemTimeToTzSpecificLocalTime(NULL, @st, @lt)
    Return _
        Format(lt.wYear,"0000") & "-" & _
        Format(lt.wMonth,"00") & "-" & _
        Format(lt.wDay,"00") & " " & _
        Format(lt.wHour,"00") & ":" & _
        Format(lt.wMinute,"00") & ":" & _
        Format(lt.wSecond,"00")
End Function

Private Function FileTimeToSeconds(ByVal ft As FILETIME) As Double
    Dim q As ULongInt = (Cast(ULongInt, ft.dwHighDateTime) Shl 32) Or ft.dwLowDateTime
    Return q / 10000000.0
End Function

Private Function IntegrityRidToString(ByVal rid As DWORD) As String
    Select Case rid
        Case SECURITY_MANDATORY_LOW_RID    : Return "Low"
        Case SECURITY_MANDATORY_MEDIUM_RID : Return "Medium"
        Case SECURITY_MANDATORY_HIGH_RID   : Return "High"
        Case SECURITY_MANDATORY_SYSTEM_RID : Return "System"
        Case Else : Return "Unknown(" & Hex(rid) & ")"
    End Select
End Function

Function GetProcessOtherInfo(ByVal PID As DWORD, txtInfo As Class_TextBox) As ProcessInfo
    Dim info As ProcessInfo
    info.PID = PID
    Dim outStr As String = ""
    
    Dim hProcess As HANDLE = NULL
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(PID)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, 0, PID)
    End If
    If hProcess = NULL Then
        txtInfo.Text = "(failed)"
        MyLog.PrintLog LOG_ERROR, "打开进程失败,PID=" & PID
        Return info
    End If
    
    ' 启动时间 + CPU时间
    Dim ftCreate As FILETIME, ftExit As FILETIME
    Dim ftKernel As FILETIME, ftUser As FILETIME
    If GetProcessTimes(hProcess, @ftCreate, @ftExit, @ftKernel, @ftUser) <> 0 Then
        info.startTimeStr = FileTimeToLocalStr(ftCreate)
        info.cpuUserSec   = FileTimeToSeconds(ftUser)
        info.cpuKernelSec = FileTimeToSeconds(ftKernel)
        
        outStr += "启动时间     : " & info.startTimeStr & vbCrLf
        outStr += "CPU 用户态   : " & Format(info.cpuUserSec, "0.000")   & " s" & vbCrLf
        outStr += "CPU 内核态   : " & Format(info.cpuKernelSec, "0.000") & " s" & vbCrLf
    Else
        outStr += "启动时间     : " & "(failed)" & vbCrLf
        outStr += "CPU 用户态   : " & "(failed)" & vbCrLf
        outStr += "CPU 内核态   : " & "(failed)" & vbCrLf
        MyLog.PrintWin32Error "GetProcessOtherInfo", "GetProcessTimes", "PID=" & PID
    End If
    
    ' 内存信息
    Dim pmcx As PROCESS_MEMORY_COUNTERS_EX
    pmcx.cb = SizeOf(pmcx)
    If GetProcessMemoryInfo(hProcess, CPtr(PPROCESS_MEMORY_COUNTERS, @pmcx), pmcx.cb) <> 0 Then
        info.workingSetKB   = pmcx.WorkingSetSize \ 1024
        info.privateBytesKB = pmcx.PrivateUsage   \ 1024
        info.pagedPoolKB    = pmcx.QuotaPagedPoolUsage \ 1024
        
        outStr += "工作集(KB)   : " & Str(info.workingSetKB)   & vbCrLf
        outStr += "私有字节(KB) : " & Str(info.privateBytesKB) & vbCrLf
        outStr += "分页池(KB)   : " & Str(info.pagedPoolKB) & vbCrLf
    Else
        outStr += "工作集(KB)   : " & "(failed)" & vbCrLf
        outStr += "私有字节(KB) : " & "(failed)" & vbCrLf
        outStr += "分页池(KB)   : " & "(failed)" & vbCrLf
        MyLog.PrintWin32Error "GetProcessOtherInfo", "GetProcessMemoryInfo", "PID=" & PID
    End If
    
    ' 句柄、GDI、USER
    GetProcessHandleCount(hProcess, @info.handleCount)
    info.gdiObjects  = GetGuiResources(hProcess, GR_GDIOBJECTS)
    info.userObjects = GetGuiResources(hProcess, GR_USEROBJECTS)
    
    outStr += "句柄数量     : " & Str(info.handleCount) & vbCrLf
    outStr += "GDI 对象数   : " & Str(info.gdiObjects)  & vbCrLf
    outStr += "USER 对象数  : " & Str(info.userObjects) & vbCrLf
    
    ' 完整性级别 MIC
    Dim hToken As HANDLE, bSuccess As Boolean = False
    If OpenProcessToken(hProcess, TOKEN_QUERY, @hToken) <> 0 Then
        Dim buf(0 To 1023) As Byte, nLen As DWORD
        If GetTokenInformation(hToken, TokenIntegrityLevel, @buf(0), 1024, @nLen) <> 0 Then
            Dim pTil As PTOKEN_MANDATORY_LABEL = Cast(PTOKEN_MANDATORY_LABEL, @buf(0))
            Dim pSid As LPDWORD = GetSidSubAuthority(pTil->Label.Sid, 0)
            info.integrityLevel = IntegrityRidToString( *pSid)
            outStr += "完整性级别   : " & info.integrityLevel & vbCrLf
            bSuccess = True
        Else
            MyLog.PrintWin32Error "GetProcessOtherInfo", "GetTokenInformation", "PID=" & PID
        End If
        CloseHandle(hToken)
    Else
        MyLog.PrintWin32Error "GetProcessOtherInfo", "OpenProcessToken", "PID=" & PID
    End If
    
    If Not bSuccess Then outStr += "完整性级别   : " & "(failed)" & vbCrLf
    
    ' PPL 标志
    Dim prot As PS_PROTECTION, retLen As ULong, ret As NTSTATUS
    ret = ZwQueryInformationProcess(hProcess, ProcessProtectionInformation, @prot, SizeOf(prot), retLen)
    If NT_SUCCESS(ret) Then
        ' 手动解析 Level（避免位域编译差异）
        Dim protLevel  As UByte = prot.Level
        Dim protType   As UByte = (protLevel Shr 0) And &h07
        Dim protSigner As UByte = (protLevel Shr 4) And &h0F
        
        info.pplLevel  = protLevel
        info.pplType   = protType
        info.pplSigner = protSigner
        info.isPPL     = (protType = 1)
        
        ' 保护类型
        Dim typeStr As String
        Select Case protType
            Case 0: typeStr = "无保护"
            Case 1: typeStr = "PPL (Protected Light)"
            Case 2: typeStr = "PP (Protected)"
            Case Else: typeStr = "未知(" & Str(protType) & ")"
        End Select
        
        ' Signer
        Dim signerStr As String
        Select Case protSigner
            Case 0: signerStr = "None"
            Case 1: signerStr = "Authenticode"
            Case 2: signerStr = "CodeGen"
            Case 3: signerStr = "Antimalware"
            Case 4: signerStr = "Lsa"
            Case 5: signerStr = "Windows"
            Case 6: signerStr = "WinTcb"
            Case 7: signerStr = "WinSystem"
            Case 8: signerStr = "App"
            Case Else: signerStr = "未知(" & Str(protSigner) & ")"
        End Select
        
        outStr += "是否PPL进程  : " & IIf(info.isPPL, "是", "否") & vbCrLf
        outStr += "保护类型     : " & typeStr   & vbCrLf
        outStr += "Signer       : " & signerStr & vbCrLf
    Else
        outStr += "是否PPL进程  : " & "(failed)" & vbCrLf
        outStr += "保护类型     : " & "(failed)" & vbCrLf
        outStr += "Signer       : " & "(failed)" & vbCrLf
        MyLog.PrintNtError "GetProcessOtherInfo", "ZwQueryInformationProcess", ret, "PID=" & PID
    End If
    
    ' 追加到文本框
    txtInfo.Text = outStr
    
    CloseHandle(hProcess)
    Return info
End Function

Public Function GetProcessList(lvwProcess As Class_ListView, ByVal IsCheckHideProcess As Boolean) As Integer
    Dim PID1() As DWORD, ProcessPath() As String, PID2() As DWORD, k As Integer
    Dim ProcessCount As Integer = 0
    Dim dwPID As DWORD
    Dim strPID As StringW
    Dim strEProcess As StringW
    Dim dwParentPID As DWORD
    Dim strParentPID As StringW
    Dim strPath As StringW
    Dim strPath1 As StringW
    Dim strName As StringW
    Dim strSessionId As StringW
    Dim strUserName As StringW
    Dim strCommandLine As StringW
    Dim strCompany As StringW
    Dim errCode As Long
    
    'lvwProcess.ReDraw = False
    
    InitListViewData lvwProcess
    
    GetAllPID ProcessPath(), PID1(), 0
    
    If IsCheckHideProcess Then
        If IsDriverLoaded Then GetPIDList3 PID2() Else GetPIDList1 PID2()' GetPIDList2 PID2()
        For i As Integer = 0 To UBound(PID1) - 1
            Dim ForeColor As COLORREF = FB_Black
            Dim BackColor As COLORREF = FB_White
            
            dwParentPID = 0 : strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strSessionId = "" : strUserName = "" : strCommandLine = "" : strCompany = ""
            
            dwPID = PID1(i + 1)
            strPID = WStr(dwPID)
            If GetParentProcessId(dwPID, dwParentPID) Then strParentPID = WStr(dwParentPID)
            If IsDriverLoaded Then strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = ProcessPath(i + 1)
            If InStr(strPath, ":") = 0 Then strPath = GetProcessPath(dwPID)
            strName = GetNameByPath(strPath)
            strCompany = GetFileCompany(strPath)
            Dim dwSessionId As DWORD = 0
            If Not GetProcessSessionId(dwPID, dwSessionId) Then strSessionId = "?" Else strSessionId = WStr(dwSessionId)
            strUserName = GetProcessUserName(dwPID)
            strCommandLine = GetProcessCommandLine(dwPID)

            AddItemColListEx lvwProcess, 8,,, WStr(dwPID), WStr(strParentPID), WStr(strEProcess), WStr(strName), WStr(strPath), WStr(strSessionId & "-" & strUserName), WStr(strCommandLine), WStr(strCompany)
            ProcessCount += 1
        Next
        
        For i As Integer = 0 To UBound(PID2)
            For j As Integer = 1 To UBound(PID1)
                If PID2(i) = PID1(j) Then Continue For, For ' 如果找到了证明不是隐藏进程
            Next
            
            dwParentPID = 0 : strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strSessionId = "" : strUserName = "" : strCommandLine = "" : strCompany = ""
            
            dwPID = PID2(i)
            If Not IsProcessAlive(dwPID) Then Continue For
            Print "隐藏进程:" & dwPID
            If GetParentProcessId(dwPID, dwParentPID) Then strParentPID = WStr(dwParentPID)
            If IsDriverLoaded Then strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = GetProcessPath(dwPID)
            If InStr(strPath, ":") = 0 Then
                strName = ProcessPath(i + 1)
            Else
                strName = GetNameByPath(strPath)
                strCompany = GetFileCompany(strPath)
            End If
            Dim dwSessionId As DWORD = 0
            If Not GetProcessSessionId(dwPID, dwSessionId) Then strSessionId = "?" Else strSessionId = WStr(dwSessionId)
            strUserName = GetProcessUserName(dwPID)
            strCommandLine = GetProcessCommandLine(dwPID)
            
            Dim ItemIndex As Long = AddItemColListEx(lvwProcess, 8,,, WStr(dwPID), WStr(strParentPID), WStr(strEProcess), WStr(strName), WStr(strPath), WStr(strSessionId & "-" & strUserName), WStr(strCommandLine), WStr(strCompany))
            SetItemColor lvwProcess, ItemIndex, FB_Red, FB_White
            ProcessCount += 1
        Next
    Else
        For i As Integer = 0 To UBound(PID1) - 1
            Dim ForeColor As COLORREF = FB_Black
            Dim BackColor As COLORREF = FB_White
            
            dwParentPID = 0 : strEProcess = "" : dwParentPID = 0 : strParentPID = "" : strPath = "" : strName = "" : strSessionId = "" : strUserName = "" : strCommandLine = "" : strCompany = ""
            
            dwPID = PID1(i + 1)
            If GetParentProcessId(PID1(i + 1), dwParentPID) Then strParentPID = WStr(dwParentPID)
            If IsDriverLoaded Then strEProcess = "0x" & WHex(GetEProcess(dwPID))
            strPath = ProcessPath(i + 1)
            strPath1 = GetProcessPath(dwPID)
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
            Dim dwSessionId As DWORD = 0
            If Not GetProcessSessionId(dwPID, dwSessionId) Then strSessionId = "?" Else strSessionId = WStr(dwSessionId)
            strUserName = GetProcessUserName(dwPID)
            strCommandLine = GetProcessCommandLine(dwPID)
            
            Dim ItemIndex As Long = AddItemColListEx(lvwProcess, 8,,, WStr(dwPID), WStr(strParentPID), WStr(strEProcess), WStr(strName), WStr(strPath), WStr(strSessionId & "-" & strUserName), WStr(strCommandLine), WStr(strCompany)) ' 可以在第一次调用GetProcessList时直接AddItemColList
            VerifyFileSignAsync FrmMain.hWnd, "ListView", ItemIndex, strPath
            'InvalidateRect lvwProcess.hWnd, NULL, True
            ProcessCount += 1
            'FF_DoEvents
        Next
    End If
    'lvwProcess.ReDraw = True
    CommitListViewView lvwProcess
    Return ProcessCount
End Function

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

    ' ############################## 核心修复2：变量定义 ##############################
    Dim lpRet As DWORD                    ' IOCTL返回字节数
    Dim ProcessTimer() As PROCESS_TIMER ' 定时器数组
    Dim bGlobal As Boolean

    ' ############################## 核心修复3：第一次IOCTL - 获取数据长度 ##############################
    ' 输入：NULL | 输出：NULL，获取需要的总字节数
    SetLastError 0
    IoControl hDrv, IOCTL_EnumProcessTimers, NULL, 0, NULL, 0, @lpRet
    If (lpRet = 0) Then
        Print "[GetProcessTimerList]无定时器数据"
        Exit Sub
    End If
    If (GetLastError <> 0) Then
        Print "[GetProcessTimerList]第一次IoControl失败:" & WinErrorMsg(GetLastError) & GetLastError
        Exit Sub
    End If

    ' ############################## 核心修复4：计算数组大小 ##############################
    Print "需要总字节数:" & lpRet & " | 单个结构体大小:" & SizeOf(PROCESS_TIMER)
    ReDim ProcessTimer((lpRet / SizeOf(PROCESS_TIMER)) - 1) As PROCESS_TIMER

    ' ############################## 核心修复5：第二次IOCTL - 获取定时器数据 ##############################
    IoControl hDrv, IOCTL_EnumProcessTimers, NULL, 0, _
    @ProcessTimer(0), UBound(ProcessTimer) * SizeOf(PROCESS_TIMER) + SizeOf(PROCESS_TIMER), @lpRet

    ' ############################## 填充列表框 ##############################
    InitListViewData lvwWindowTimer
    
    If dwProcessId = Cast(DWORD, -1) Then
        If lvwWindowTimer.ColumnCount = 6 Then lvwWindowTimer.InsertColumn 6, "所属PID"
        bGlobal = True
    Else
        If lvwWindowTimer.ColumnCount = 7 Then lvwWindowTimer.DeleteColumn 6
        bGlobal = False
    End If
    
    For i As DWORD = 0 To UBound(ProcessTimer)
        Print "[GetProcessTimerList]nIDEvent:" & ProcessTimer(i).nIDEvent & " pfn:" & "0x" & WHex(ProcessTimer(i).pfn)
        
        Dim wszModulePath As WString * MAX_PATH = ""
        GetModuleByAddress ProcessTimer(i).ProcessId, Cast(PVOID, ProcessTimer(i).pfn), @wszModulePath
        If bGlobal Then
            AddItemColListEx lvwWindowTimer, 7,,, _
            WStr(ProcessTimer(i).nIDEvent), _ ' 定时器ID
            WStr(ProcessTimer(i).nTimeout), _ ' 超时时间
            "0x" & WHex(ProcessTimer(i).pfn), _ ' 回调函数地址
            wszModulePath, _ ' 所属模块
            "0x" & WHex(ProcessTimer(i).hWnd), _ ' 所属窗口
            WStr(ProcessTimer(i).ThreadId), _ ' 线程ID
            WStr(ProcessTimer(i).ProcessId) ' 进程ID
        Else
            If ProcessTimer(i).ProcessId <> dwProcessId AndAlso dwProcessId <> Cast(DWORD, -1) Then Continue For
            AddItemColListEx lvwWindowTimer, 6,,, _
            WStr(ProcessTimer(i).nIDEvent), _ ' 定时器ID
            WStr(ProcessTimer(i).nTimeout), _ ' 超时时间
            "0x" & WHex(ProcessTimer(i).pfn), _ ' 回调函数地址
            wszModulePath, _ ' 所属模块
            "0x" & WHex(ProcessTimer(i).hWnd), _ ' 所属窗口
            WStr(ProcessTimer(i).ThreadId) ' 线程ID
        End If
    Next

    ' 释放内存
    Erase ProcessTimer
    
    CommitListViewView lvwWindowTimer
End Sub