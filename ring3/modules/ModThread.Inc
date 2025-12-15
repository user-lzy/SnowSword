Type THREAD_BASIC_INFORMATION
    ExitStatus As NTSTATUS
    TebBaseAddress As Any Ptr 'PPEB
    ClientId As CLIENT_ID
    AffinityMask As ULong Ptr
    Priority As Long
    BasePriority As Long 'KPRIORITY
End Type

Type TEB
    Reserved1(11) As PVOID
    ProcessEnvironmentBlock As PPEB64
    Reserved2(398)As PVOID
    Reserved3(1951) As Byte
    TlsSlots(63) As PVOID
    Reserved4(7) As Byte
    Reserved5(25) As PVOID
    ReservedForOle As PVOID  ' Windows 2000 only
    Reserved6(3) As PVOID
    TlsExpansionSlots As PVOID
End Type
Type PTEB As TEB Ptr
    
Private Enum THREADINFOCLASS
    ThreadBasicInformation
    ThreadTimes
    ThreadPriority
    ThreadBasePriority
    ThreadAffinityMask
    ThreadImpersonationToken
    ThreadDescriptorTableEntry
    ThreadEnableAlignmentFaultFixup
    ThreadEventPair_Reusable
    ThreadQuerySetWin32StartAddress
    ThreadZeroTlsCell
    ThreadPerformanceCount
    ThreadAmILastThread
    ThreadIdealProcessor
    ThreadPriorityBoost
    ThreadSetTlsArrayAddress                                                    ' Obsolete
    ThreadIsIoPending
    ThreadHideFromDebugger
    ThreadBreakOnTermination
    ThreadSwitchLegacyState
    ThreadIsTerminated
    ThreadLastSystemCall
    ThreadIoPriority
    ThreadCycleTime
    ThreadPagePriority
    ThreadActualBasePriority
    ThreadTebInformation
    ThreadCSwitchMon                                                            ' Obsolete
    ThreadCSwitchPmu
    ThreadWow64Context
    ThreadGroupInformation
    ThreadUmsInformation                                                        ' UMS
    ThreadCounterProfiling
    ThreadIdealProcessorEx
    MaxThreadInfoClass
End Enum

#define STILL_ALIVE 259
Declare Function ZwCreateThreadEx Lib "ntdll.dll" Alias "ZwCreateThreadEx"(ByVal ThreadHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As LPVOID, ByVal ProcessHandle As HANDLE, _
ByVal lpStartAddress As LPTHREAD_START_ROUTINE, ByVal lpParameter As LPVOID, ByVal CreateThreadFlags As DWORD, ByVal ZeroBits As size_t, ByVal StackSize As size_t, _
ByVal MaximunStackSize As size_t, ByVal pUnkown As LPVOID) As NTSTATUS ' CreateThreadFlags:在注入系统进程时不需ResumeThread(0),绕过DLL_ATTACH_THREAD(2),HideFromDebugger(4),
Declare Function ZwQueryInformationThread Lib "ntdll.dll" Alias "ZwQueryInformationThread"(ByVal ThreadHandle As HANDLE,ByVal ThreadInformationClass As THREADINFOCLASS,_
ByVal ThreadInformation As LPVOID, ByVal ThreadInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
'Declare Function ZwSetInformationThread Lib "ntdll.dll" Alias "ZwSetInformationThread"(ByVal ThreadHandle As HANDLE,ByVal ThreadInformationClass As THREADINFOCLASS,_
'ByVal ThreadInformation As LPVOID, ByVal ThreadInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
Declare Function ZwOpenThread Lib "ntdll.dll" Alias "ZwOpenThread"(ByVal ThreadHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
Declare Function ZwTerminateThread Lib "ntdll.dll" Alias "ZwTerminateThread"(ByVal ThreadHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
Declare Function ZwSuspendThread Lib "ntdll.dll" Alias "ZwSuspendThread"(ByVal ThreadHandle As HANDLE,ByVal PreviousSuspendCount As PULONG) As NTSTATUS
Declare Function ZwResumeThread Lib "ntdll.dll" Alias "ZwResumeThread"(ByVal ThreadHandle As HANDLE, ByVal PreviousSuspendCount As PULONG) As NTSTATUS

#ifdef PrintLog
'#define PrintLog(nt_Status, Reason) MyPrintLog(nt_Status, Reason, __FUNCTION__, __LINE__)'取消注释就炸
#endif

#ifdef PrintLastError
#define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")"
#endif

Function SnowSword_OpenThread(ByVal ThreadId As DWORD) As HANDLE
    'If Not IsDriverLoaded Then Return NULL
    Dim TID As DWORD = ThreadId
    Dim hThread As HANDLE = NULL
    IoControl hDrv, IOCTL_OpenThread, @TID, SizeOf(DWORD), @hThread, SizeOf(hThread)
    Return hThread
End Function

Function RemoteInjectDll(ByVal ProcessId As DWORD, ByVal wszInjectDllPathName As StringW) As Boolean
    Dim hProcess As HANDLE = NULL
    Dim lpBuffer As LPVOID = NULL
    Dim dwWriteBytes As size_t = 0
    Dim dwRetErrorCode As DWORD = 0
    Dim ThreadHandle As HANDLE = NULL
    Dim pfnLoadLibraryW As PVOID = Cast(PVOID, GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryW"))
    Dim st As NTSTATUS
    Dim dwExitCode As DWORD
    
    Function = True

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, False, ProcessId)
    If hProcess = NULL Then
        Print __FUNCTION__ & "OpenProcess失败!错误码:" & GetLastError & WinErrorMsg(GetLastError)
        Print ProcessId
        Function = False
        Goto Cleanup
    End If
    lpBuffer = VirtualAllocEx(hProcess, 0, &H1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
    If lpBuffer = NULL Then
        Print __FUNCTION__ & "VirtualAllocEx失败!错误码:" & GetLastError & WinErrorMsg(GetLastError)
        Function = False
        Goto Cleanup
    End If
    If WriteProcessMemory(hProcess, lpBuffer, wszInjectDllPathName, wszInjectDllPathName.WstrLen * 2, @dwWriteBytes) = 0 Then
        Print __FUNCTION__ & "WriteProcessMemory失败!错误码:" & GetLastError & WinErrorMsg(GetLastError)
        Function = False
        Goto Cleanup
    End If

    st = ZwCreateThreadEx(@ThreadHandle, PROCESS_ALL_ACCESS, NULL, hProcess, Cast(LPTHREAD_START_ROUTINE, pfnLoadLibraryW), lpBuffer, 0, 0, 0, 0, NULL)
    If Not NT_SUCCESS(st) Then
        Print __FUNCTION__ & "ZwCreataeThreadEx失败!错误码:" & Hex(st)
        Function = False
        Goto Cleanup
    End If
    
    'WaitForSingleObject ThreadHandle, INFINITE
    
    /'Do While GetExitCodeThread(ThreadHandle, @dwExitCode) <> 0' 获取线程退出代码
        ' 检查 LoadLibraryW 的返回值
        If dwExitCode = 0 Then
            Print "LoadLibraryW 失败! 错误码: " & GetLastError & " - " & WinErrorMsg(GetLastError)
            Function = False
            Goto Cleanup
        Else
            Function = True
            Goto Cleanup
        End If
        FF_DoEvents
    Loop'/
    'Print "dwExitCode:" & dwExitCode
    Function = True
    Cleanup:
    'If lpBuffer <> NULL Then VirtualFreeEx hProcess, lpBuffer, 0, MEM_RELEASE
    If hProcess <> NULL Then CloseHandle hProcess
    If ThreadHandle <> NULL Then CloseHandle ThreadHandle
End Function

Private Function GetTeb(ByVal dwThreadId As DWORD) As Any Ptr
    Dim PTeb As Any Ptr = NULL
    Dim hThread As HANDLE
    If IsDriverLoaded Then
        hThread = SnowSword_OpenThread(dwThreadId)
    Else
        hThread = OpenThread(THREAD_QUERY_INFORMATION, False, dwThreadId)
    End If
    Dim tbi As THREAD_BASIC_INFORMATION, ret As DWORD, st As NTSTATUS
    st = ZwQueryInformationThread(hThread, ThreadBasicInformation, @tbi, SizeOf(tbi), ret)
    'If PrintLog(st, "") Then
    Print "[GetTeb]TID" & dwThreadId & " " & Hex(st)
    Return tbi.TebBaseAddress
End Function

Private Function GetEThread(ByVal dwThreadId As DWORD) As ULONG64
    Dim pEThread As ULONG64, lpRet As DWORD
    IoControl hDrv, IOCTL_GetEThread, @dwThreadId, SizeOf(DWORD), @pEThread, SizeOf(ULONG64), @lpRet
    'Print "TID:" & dwThreadId & " " & lpRet
    Return pEThread
End Function

Function GetFirstThread(ByVal dwProcessId As DWORD) As DWORD
    Dim thre As THREADENTRY32
    Dim hSnapshot As HANDLE
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId)
    If hSnapshot > 0 Then
        thre.dwSize = SizeOf(THREADENTRY32)
        If Thread32First(hSnapshot, @thre) Then Function = thre.th32ThreadID
        CloseHandle hSnapshot '关闭模块快照句柄
    End If
End Function

Sub GetThreadList1()
    If Not IsDriverLoaded Then Exit Sub
    For TID As DWORD = 0 To &H20000
        Dim pEThread As ULONG64 = GetEThread(TID)
        If pEThread <> NULL Then Print "Thread " & TID
    Next
End Sub

Function NtPathToDosPathW(ByVal FullNtPath As LPWSTR, ByVal FullDosPath As LPWSTR) As Boolean

    Dim DosDevice As WString * 4         'dos设备名最大长度为4
    Dim NtPath As WString * 64         'nt设备名最大长度为64
    Dim RetStr As LPWSTR = NULL
    Dim NtPathLen As ULong
    
    For i As Integer = 65 To 65 + 25
        DosDevice = Chr(i)& ":"
        If (QueryDosDeviceW(@DosDevice, @NtPath, 64)) Then
            If (NtPath <> "") Then
                NtPathLen = wcslen(@NtPath)
                'Print NtPathLen & " " & NtPath & " " & InStrW(*FullNtPath, NtPath)
                If (InStrW(*FullNtPath, NtPath) > 0) Then
                'If (Not _wcsnicmp(@NtPath, FullNtPath, NtPathLen)) Then
                    wcscpy(FullDosPath, @DosDevice)
                    wcscat(FullDosPath, FullNtPath + NtPathLen)
                    'Print *FullDosPath
                    Return True
                End If
            End If
        End If
    Next
End Function

Private Function GetThreadState(ByVal dwThreadId As DWORD) As StringW
    Dim hThread As HANDLE, SuspendCount As Long, lpExitCode As DWORD, ThreadState As StringW, st As NTSTATUS
    If dwThreadId = GetCurrentThreadId Then
        Function = "Running"
        Exit Function
    End If
    If IsDriverLoaded Then
        hThread = SnowSword_OpenThread(dwThreadId)
    Else
        hThread = OpenThread(THREAD_QUERY_INFORMATION Or THREAD_SUSPEND_RESUME, False, dwThreadId)
    End If
    If GetExitCodeThread(hThread, @lpExitCode) = 0 Then
        Function = "Unknown"
        CloseHandle hThread
        Exit Function
    End If
    If lpExitCode = STILL_ALIVE Then
        st = ZwResumeThread(hThread, @SuspendCount)
        If NT_SUCCESS(st) Then ' 我真是逆天,如果用这个函数查询自身线程状态,那把自己线程挂起了谁来恢复[捂脸],应该先ZwResumeThread
            If SuspendCount = 0 Then
                CloseHandle hThread
                Function = "Running"
                Exit Function
            Else
                Function = "Suspended(Count:" & SuspendCount & ")"'(Waiting?)
            End If
        Else
            Print "[GetThreadState]ZwResumeThread " & dwThreadId & ":" & Hex(st)
            Function = "Unknown"
        End If
    Else
        Function = "Terminated"
    End If
    ZwSuspendThread hThread, 0 '666, 之前挂起线程之后忘记恢复了,我说怎么看一下explorer的线程列表桌面就挂了[捂脸]
    CloseHandle hThread
End Function

Function GetThreadStartAddr(ByVal dwThreadId As DWORD) As PVOID
    Dim hThread As HANDLE
    If IsDriverLoaded Then
        hThread = SnowSword_OpenThread(dwThreadId)
    Else
        hThread = OpenThread(THREAD_QUERY_INFORMATION, False, dwThreadId)
    End If
    Dim StartAddr As PVOID = NULL, st As NTSTATUS, ret As DWORD
    st = ZwQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, @StartAddr, SizeOf(StartAddr), ret)
    'If (StartAddr = 0) Then Print "[GetThreadStartAddr]" & Hex(st)
    Return StartAddr
End Function

Sub GetModuleByAddress(ByVal dwProcessId As DWORD, ByVal Addr As PVOID, ByVal pstrName As LPWSTR)
    Dim hProcess As HANDLE
    If dwProcessId = 4 Then
        *pstrName = GetDriverPathByAddr(Cast(ULONG64, Addr))
        Exit Sub
    End If
    
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    Dim NtPath As WString * MAX_PATH
    If (hProcess = NULL) Then PrintLastError '"1"'PrintLastError
    'Print "0x" & Hex(Addr)'删掉这句会导致所有线程所在模块都变成一样的乱码
    'Print "1"
    Dim ret As DWORD = GetMappedFileNameW(hProcess, Addr, @NtPath, MAX_PATH)
    'Print "2"
    If (ret > 0) Then
        Dim ModulePath As WString * MAX_PATH
        If (NtPathToDosPathW(NtPath, @ModulePath)) Then
            'Print "3"
            ZwClose hProcess
            *pstrName = ModulePath
            Exit Sub
        End If
        Print "NtPathToDosPathW Failed"
    End If
    ZwClose hProcess
End Sub

Private Function SetCriticalThread(ByVal dwThreadId As DWORD, ByVal IsCritical As Boolean) As NTSTATUS
    Dim hThread As HANDLE, ret As NTSTATUS, retLen As DWORD
    Dim ZwSetInformationThread As Function(ByVal ThreadHandle As HANDLE,ByVal ThreadInformationClass As ThreadINFOCLASS,_
    ByVal ThreadInformation As LPVOID, ByVal ThreadInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS = DyLibSymbol(GetModuleHandle("ntdll.dll"), "ZwSetInformationThread")
    hThread = OpenThread(THREAD_SET_INFORMATION, False, dwThreadId)
    ret = ZwSetInformationThread(hThread, ThreadBreakOnTermination, @IsCritical, Len(IsCritical), retLen)
    ZwClose hThread
    Return ret
End Function

Private Function IsThreadCritical(ByVal dwThreadId As DWORD, ByVal IsCritical As PBOOLEAN) As NTSTATUS
    Dim hThread As HANDLE, ret As NTSTATUS, retLen As DWORD
    hThread = OpenThread(THREAD_QUERY_INFORMATION, False, dwThreadId)
    ret = ZwQueryInformationThread(hThread, ThreadBreakOnTermination, IsCritical, Len(*IsCritical), retLen)
    ZwClose hThread
    Return ret
End Function

Public Function GetThreadList(ByVal ProcessId As DWORD, lvwThread As Class_ListView) As Integer
    Dim i As Long
    Dim thre As THREADENTRY32
    Dim hSnapshot As HANDLE, strModulePath As WString * 260, ThreadStartAddress As PVOID
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, ProcessId)
    If hSnapshot > 0 Then
        thre.dwSize = SizeOf(THREADENTRY32)
        If Thread32First(hSnapshot, @thre) Then
            Do
                If thre.th32OwnerProcessID = ProcessId Then
                    Dim strEThread As StringW
                    If IsDriverLoaded Then strEThread = "0x" & WHex(GetEThread(thre.th32ThreadID))
                    ThreadStartAddress = GetThreadStartAddr(thre.th32ThreadID)
                    GetModuleByAddress ProcessId, ThreadStartAddress, @strModulePath'如果直接使用 *GetModuleByAddress(ProcessId, ThreadStartAddress)会导致GetModuleByAddress中的诡异问题
                    'Print thre.th32ThreadID & " " & thre.tpBasePri
                    lvwThread.AddItemColList ,, WStr(thre.th32ThreadID), WStr(thre.tpBasePri), GetThreadState(thre.th32ThreadID), _
                    strEThread, "0x" & WHex(ThreadStartAddress), "0x" & WHex(GetTeb(thre.th32ThreadID)), strModulePath
                    i += 1
                End If
            Loop Until Thread32Next(hSnapshot, @thre) = 0
        End If
        CloseHandle hSnapshot '关闭模块快照句柄
    ElseIf IsDriverLoaded Then
        Dim OwnerPID As DWORD, pEThread As HANDLE
        For TID As DWORD = 0 To &H20000
            IoControl hDrv, IOCTL_GetEThread, @TID, SizeOf(DWORD), @pEThread, SizeOf(HANDLE)
            If pEThread > 0 Then
                Print "TID:" & TID
                'IoControl hDrv, IOCTL_GetPIDFromTID, @TID, SizeOf(DWORD), @OwnerPID, SizeOf(DWORD)
                If (GetLastError = 0) AndAlso (OwnerPID = ProcessId) Then
                    lvwThread.AddItemColList,, WStr(thre.th32ThreadID),,, "0x" & WHex(pEThread)
                    i += 1
                End If
            End If
        Next
    End If
    Return i
End Function

'这样总有随机几个线程的ETHREAD内核正常获取但应用层打印出来是0
/'Function GetKernelThreadList1(ByVal DriverBase As ULONG64, ByVal DriverSize As ULONG64, DriverPath As StringW, lvwKernelThread As Class_ListView) As DWORD
    Dim i As Long
    Dim StartAddress As ULONG64
    Print "DriverBase:0x" & WHex(DriverBase) & " DriverSize:0x" & WHex(DriverSize) & " DriverEndBase:0x" & WHex(DriverBase + DriverSize)
    'Print (&H7FF8B977B9A0>&HFFFFF80000000000)
    Dim OwnerPID As DWORD, pEThread As HANDLE
    For TID As DWORD = 0 To 1000 Step 4 ' &H20000 Step 4
        pEThread = CPtr(HANDLE, GetEThread(TID))
        If pEThread > 0 Then
            IoControl hDrv, IOCTL_GetPIDFromTID, @TID, SizeOf(DWORD), @OwnerPID, SizeOf(DWORD)
            If (GetLastError = 0) AndAlso (OwnerPID = 4) Then
                StartAddress = Cast(ULONG64, GetThreadStartAddr(TID))
                Print "TID:" & TID & " StartAddr:0x" & WHex(StartAddress)
                If (StartAddress >= DriverBase) AndAlso (StartAddress <= DriverBase + DriverSize) Then
                    lvwKernelThread.AddItemColList,, WStr(TID),,, "0x" & WHex(pEThread)
                    i += 1
                End If
            End If
        End If
    Next
    Return i
End Function'/

Function GetKernelThreadList(lvwKernelThread As Class_ListView) As DWORD
    Dim thre As THREADENTRY32
    Dim hSnapshot As HANDLE, strModulePath As WString * 260, ThreadStartAddress As ULONG64
    Dim pEThread As HANDLE
    Dim i As DWORD
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 4)
    If hSnapshot > 0 Then
        thre.dwSize = SizeOf(THREADENTRY32)
        If Thread32First(hSnapshot, @thre) Then
            Do
                If thre.th32OwnerProcessID = 4 Then
                    pEThread = CPtr(HANDLE, GetEThread(thre.th32ThreadID))
                    ThreadStartAddress = Cast(ULONG64, GetThreadStartAddr(thre.th32ThreadID))
                    'Print "TID:" & TID & " StartAddr:0x" & WHex(StartAddress)
                    Dim DriverPath As StringW = GetDriverPathByAddr(ThreadStartAddress)
                    If DriverPath <> "" Then
                        lvwKernelThread.AddItemColList,, WStr(thre.th32ThreadID), WStr(thre.tpBasePri), _
                        GetThreadState(thre.th32ThreadID), "0x" & WHex(pEThread), "0x" & WHex(ThreadStartAddress), DriverPath
                        i += 1
                    End If
                End If
            Loop Until Thread32Next(hSnapshot, @thre) = 0
        End If
    End If
    Return i
End Function

Public Function MySuspendThread(ByVal dwThreadId As DWORD) As Boolean
    'Print "代码行数基准:" & __FUNCTION__
    Dim hThread As HANDLE
    If IsDriverLoaded Then
        hThread = SnowSword_OpenThread(dwThreadId)
    Else
        hThread = OpenThread(THREAD_SUSPEND_RESUME, False, dwThreadId)
    End If
    
    If (hThread = NULL) Then
        PrintLastError
        Return False
    End If
    SuspendThread hThread
    CloseHandle  hThread
    If GetLastError <> 0 Then
        Print "[MySuspendThread]" & dwThreadId & ":" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    Else
        Return True
    End If
End Function

Public Function MyResumeThread(ByVal dwThreadId As DWORD) As Boolean
    'Print "代码行数基准:" & __FUNCTION__
    Dim hThread As HANDLE
    If IsDriverLoaded Then
        hThread = SnowSword_OpenThread(dwThreadId)
    Else
        hThread = OpenThread(THREAD_SUSPEND_RESUME, False, dwThreadId)
    End If
    
    If (hThread = NULL) Then
        PrintLastError
        Return False
    End If
    ResumeThread hThread
    CloseHandle  hThread
    If GetLastError <> 0 Then
        Print "[MyResumeThread]" & dwThreadId & ":" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    Else
        Return True
    End If
End Function

'ZwSetInformationThread ThreadAttachContainer

Public Function KillThread(ByVal dwThreadId As DWORD) As Boolean
    Dim IsCritical As BOOLEAN
    If (NT_SUCCESS(IsThreadCritical(dwThreadId, @IsCritical))) Then
        If (IsCritical) Then
            If (AfxMsg("此线程处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDNO) Then
                Return False
            Else
                SetCriticalThread dwThreadId, False
            End If
        End If
    End If
    Dim hThread As HANDLE = OpenThread(THREAD_TERMINATE,False,dwThreadId)
    If (hThread = 0) Then
        Print "打开线程失败,原因:" & WinErrorMsg(GetLastError),"提示"
        Goto Err1
    End If
    If (TerminateThread(hThread,0) = 0) Then
        Print "结束线程失败,原因:" & WinErrorMsg(GetLastError),"提示"
        Goto Err1
    End If
    Return True
Err1 : 
    CloseHandle hThread
    Return False
End Function

Function ForceKillThread(ByVal dwThreadId As HANDLE) As Boolean
    If Not IsDriverLoaded Then Return False
    Dim ThreadId As HANDLE = dwThreadId
    IoControl hDrv, IOCTL_ForceKillThread, @ThreadId, SizeOf(HANDLE)
    If GetLastError = 0 Then
        Function = True
    Else
        Function = False
        Print "[ForceKillThread]:" & GetLastError
    End If
End Function

/'Private Declare Function SymInitialize Lib "dbghelp.dll" (ByVal hProcess As Long, ByVal szSearchPath As String, ByVal bInvadeProcess As Long) As Long
Private Declare Function SymGetOptions Lib "dbghelp.dll" () As Long
Private Declare Function SymSetOptions Lib "dbghelp.dll" (ByVal SymOptions As Long) As Long
Private Declare Function SymEnumerateSymbols Lib "dbghelp.dll" (ByVal hProcess As Long, ByVal BaseAddress As Long, ByVal EnumSymbolsCallback As Long, ByVal UserContext As Long) As Long
Private Declare Function StackWalk Lib "dbghelp.dll" (ByVal MachineType As Long, ByVal hProcess As Long, ByVal hThread As Long, ByRef StackFrame As STACKFRAME, ByVal Context As Long, ByVal ReadMemoryRoutine As Long, ByVal FunctionTableAccessRoutine As Long, ByVal GetModuleBaseRoutine As Long, ByVal TranslateAddress As Long) As Long
Private Declare Function SymCleanup Lib "dbghelp.dll" (ByVal hProcess As Long) As Long

Private Type STACKFRAME
    AddrPC As Long
    AddrReturn As Long
    AddrFrame As Long
    AddrStack As Long
    FuncTableEntry As Long
    Params(3) As Long
    Far As Long
    Virtual As Long
    Reserved(3) As Long
    AddrBpAsAddrPC As Long
End Type

Sub GetCallStack(hProcess As Long, hThread As Long)
    Dim SymOptions As Long
    Dim SymInitialized As Long
    Dim StackFrame As STACKFRAME
    Dim Context As Long
    Dim ReadMemoryRoutine As Long
    Dim FunctionTableAccessRoutine As Long
    Dim GetModuleBaseRoutine As Long
    Dim TranslateAddress As Long

    SymOptions = SymGetOptions()
    SymOptions = SymOptions Or SYMOPT_DEBUG
    SymSetOptions SymOptions

    SymInitialized = SymInitialize(hProcess, "", 1)

    If SymInitialized Then
        StackFrame.AddrPC = 0
        StackFrame.AddrStack = 0
        StackFrame.AddrFrame = 0

        Do While StackWalk(IMAGE_FILE_MACHINE_I386, hProcess, hThread, StackFrame, Context, ReadMemoryRoutine, FunctionTableAccessRoutine, GetModuleBaseRoutine, TranslateAddress)
            ' Process each stack frame
            'Debug.Print "Address: " & Hex(StackFrame.AddrPC)
        Loop

        SymCleanup(hProcess)
    Else
        'Debug.Print "Failed to initialize symbols"
    End If
End Sub
'/