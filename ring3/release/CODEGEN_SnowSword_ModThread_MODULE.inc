'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


Private Type THREAD_BASIC_INFORMATION
    ExitStatus As NTSTATUS
    TebBaseAddress As Any Ptr                                                      'PPEB
    AffinityMask As ULong Ptr
    BasePriority As Long                                                        'KPRIORITY
    UniqueThreadId As ULong Ptr
    InheritedFromUniqueThreadId As ULong Ptr
End Type

' Type TEB '已经搬到定义文件处
'     Reserved1(11) As PVOID '已经搬到定义文件处
'     ProcessEnvironmentBlock As PPEB64 '已经搬到定义文件处
'     Reserved2(398)As PVOID '已经搬到定义文件处
'     Reserved3(1951) As Byte '已经搬到定义文件处
'     TlsSlots(63) As PVOID '已经搬到定义文件处
'     Reserved4(7) As Byte '已经搬到定义文件处
'     Reserved5(25) As PVOID '已经搬到定义文件处
'     ReservedForOle As PVOID  ' Windows 2000 only '已经搬到定义文件处
'     Reserved6(3) As PVOID '已经搬到定义文件处
'     TlsExpansionSlots As PVOID '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type PTEB As TEB Ptr '已经搬到定义文件处
    
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

' #define STILL_ALIVE 259 '已经搬到定义文件处

Extern "C"
    Declare Function ZwQueryInformationThread Lib "ntdll.dll"(ByVal ThreadHandle As HANDLE,ByVal ThreadInformationClass As THREADINFOCLASS,_
    ByVal ThreadInformation As LPVOID, ByVal ThreadInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
    Declare Function ZwSetInformationThread Lib "ntdll.dll"(ByVal ThreadHandle As HANDLE,ByVal ThreadInformationClass As THREADINFOCLASS,_
    ByVal ThreadInformation As LPVOID, ByVal ThreadInformationLength As DWORD, ByRef ReturnLength As DWORD) As NTSTATUS
    Declare Function ZwOpenThread Lib "ntdll.dll"(ByVal ThreadHandle As PHANDLE, ByVal DesiredAccess As ACCESS_MASK, ByVal ObjectAttributes As POBJECT_ATTRIBUTES, ByVal ClientId As PCLIENT_ID) As NTSTATUS
    Declare Function ZwTerminateThread Lib "ntdll.dll"(ByVal ThreadHandle As HANDLE, ByVal ExitStatus As NTSTATUS) As NTSTATUS
    Declare Function ZwSuspendThread Lib "ntdll.dll"(ByVal ThreadHandle As HANDLE,ByVal PreviousSuspendCount As PULONG) As NTSTATUS
    Declare Function ZwResumeThread Lib "ntdll.dll"(ByVal ThreadHandle As HANDLE, ByVal PreviousSuspendCount As PULONG) As NTSTATUS
    'Declare Function RtlNtStatusToDosError Lib "ntdll.dll"(ByVal Status As NTSTATUS) As ULong
    'Declare Function ZwClose Lib "ntdll.dll"(ByVal HANDLE As HANDLE) As NTSTATUS
End Extern

#ifdef PrintLog
'#define PrintLog(nt_Status, Reason) MyPrintLog(nt_Status, Reason, __FUNCTION__, __LINE__)'取消注释就炸
#endif

#ifdef PrintLastError
' #define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")" '已经搬到定义文件处
#endif

Private Function GetTeb(ByVal dwThreadId As DWORD) As Any Ptr
    Dim PTeb As Any Ptr = NULL
    Dim hThread As HANDLE = OpenThread(THREAD_QUERY_INFORMATION,False,dwThreadId)
    Dim tbi As THREAD_BASIC_INFORMATION, ret As DWORD, st As NTSTATUS
    st = ZwQueryInformationThread(hThread,ThreadBasicInformation,@tbi,SizeOf(tbi),ret)
    'If PrintLog(st, "") Then
    If Not NT_SUCCESS(st) Then PrintA __FUNCTION__ & " " & Hex(st)
    Return tbi.TebBaseAddress
End Function

Public Function NtPathToDosPathW(ByVal FullNtPath As LPWSTR, ByVal FullDosPath As LPWSTR) As BOOLEAN

    Dim DosDevice As WString * 4         'dos设备名最大长度为4
    Dim NtPath As WString * 64         'nt设备名最大长度为64
    Dim RetStr As LPWSTR = NULL
    Dim NtPathLen As ULong
    
    For i As Integer = 65 To 65 + 25
        DosDevice = Chr(i)& ":"
        If (QueryDosDeviceW(@DosDevice, @NtPath, 64)) Then
            If (NtPath <> "") Then
                NtPathLen = wcslen(@NtPath)
                'PrintA NtPathLen & " " & NtPath & " " & InStrW(*FullNtPath, NtPath)
                If (InStrW(*FullNtPath, NtPath) > 0) Then
                'If (Not _wcsnicmp(@NtPath, FullNtPath, NtPathLen)) Then
                    wcscpy(FullDosPath, @DosDevice)
                    wcscat(FullDosPath, FullNtPath + NtPathLen)
                    'PrintA *FullDosPath
                    Return True
                End If
            End If
        End If
    Next
End Function

Private Function GetThreadState(ByVal dwThreadId As DWORD) As LPWSTR
    Dim hThread As HANDLE, State As DWORD, lpExitCode As DWORD, ThreadState As WString * 20
    If dwThreadId = GetCurrentThreadId Then
        ThreadState = "Running"
        Return @ThreadState
    End If
    hThread = OpenThread(THREAD_ALL_ACCESS, False, dwThreadId)
    GetExitCodeThread hThread, @lpExitCode
    'PrintA lpExitCode
    If lpExitCode = STILL_ALIVE Then
        State = ZwSuspendThread(hThread, 0)
        If State = 0 Then
            ThreadState = "Running"
        Else
            ThreadState = "Suspending(Count:" & State & ")"'(Waiting?)
        End If
    Else
        ThreadState = "Terminated"
    End If
    ZwResumeThread hThread, 0 '666, 之前挂起线程之后忘记恢复了,我说怎么看一下explorer的线程列表桌面就挂了[捂脸]
    CloseHandle hThread
    Return @ThreadState
End Function

Private Function GetThreadStartAddr(ByVal dwThreadId As DWORD) As PVOID
    Dim hThread As HANDLE = OpenThread(THREAD_QUERY_INFORMATION,FALSE,dwThreadId)
    Dim StartAddr As PVOID = NULL, st As NTSTATUS, ret As DWORD
    st = ZwQueryInformationThread(hThread,ThreadQuerySetWin32StartAddress,@StartAddr,SizeOf(StartAddr),ret)
    If (StartAddr = 0) Then PrintA __FUNCTION__ & " " & Hex(st)
    Return StartAddr
End Function

Private Sub GetModuleByAddress(ByVal dwProcessId As DWORD, ByVal Addr As PVOID, ByVal pstrName As LPWSTR)
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    Dim NtPath As WString * MAX_PATH
    If (hProcess = 0) Then PrintLastError'PrintA "1"'PrintLastError
    'PrintA "0x" & Hex(Addr)'删掉这句会导致所有线程所在模块都变成一样的乱码
    'PrintA "1"
    Dim ret As DWORD = GetMappedFileNameW(hProcess, Addr, @NtPath, MAX_PATH)
    'PrintA "2"
    If (ret > 0) Then
        Dim ModulePath As WString * MAX_PATH
        If (NtPathToDosPathW(NtPath, @ModulePath)) Then
            'PrintA "3"
            ZwClose hProcess
            *pstrName = ModulePath
            Exit Sub
        End If
        PrintA "NtPathToDosPathW Failed"
    End If
End Sub

Private Function SetCriticalThread(ByVal dwThreadId As DWORD, ByVal IsCritical As Boolean) As NTSTATUS
    Dim hThread As HANDLE, ret As NTSTATUS, retLen As DWORD
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

Public Sub GetThreadList(ByVal ProcessId As DWORD, lvwThread As Class_ListView)
    Dim i As Long
    Dim thre As THREADENTRY32
    Dim hSnapshot As HANDLE, strModulePath As WString * 260
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPthread, ProcessId)
    If hSnapshot > 0 Then
        thre.dwSize = SizeOf(THREADENTRY32)
        If Thread32First(hSnapshot, @thre) Then
            Do
                If thre.th32OwnerProcessID = ProcessId Then
                    Dim ThreadStartAddress As PVOID
                    lvwThread.AddItem WStr(thre.th32ThreadID)'WStr(i + 1)
                    'lvwThread.SetItemText(i, 1, )
                    lvwThread.SetItemText i, 1, WStr(thre.tpBasePri)
                    lvwThread.SetItemText i, 2, *GetThreadState(thre.th32ThreadID)
                    ThreadStartAddress = GetThreadStartAddr(thre.th32ThreadID)
                    lvwThread.SetItemText i, 3, "0x" & WStr(Hex(ThreadStartAddress))
                    lvwThread.SetItemText i, 4, "0x" & WStr(Hex(GetTeb(thre.th32ThreadID)))
                    'lvwThread.SetItemText i, 6, "0x" & WStr(Hex(thre.dwSize))
                    GetModuleByAddress ProcessId, ThreadStartAddress, @strModulePath'如果直接使用 *GetModuleByAddress(ProcessId, ThreadStartAddress)会导致GetModuleByAddress中的诡异问题
                    lvwThread.SetItemText i, 5, strModulePath
                    i += 1
                End If
            Loop Until Thread32Next(hSnapshot, @thre) = 0
        End If
        CloseHandle hSnapshot '关闭模块快照句柄
    End If
End Sub

Public Function MySuspendThread(ByVal dwThreadId As DWORD) As BOOLEAN
    PrintA "代码行数基准:" & __FUNCTION__
    Dim hThread As HANDLE = OpenThread(THREAD_SUSPEND_RESUME, False, dwThreadId)
    If (hThread = 0) Then
        PrintLastError
        Return False
    End If
    SuspendThread hThread
    If (Not PrintLog(0, "")) Then
        CloseHandle  hThread
        Return False
    End If
    CloseHandle hThread
    Return True
End Function

Public Function MyResumeThread(ByVal dwThreadId As DWORD) As BOOLEAN
    PrintA "代码行数基准:" & __FUNCTION__
    Dim hThread As HANDLE = OpenThread(THREAD_SUSPEND_RESUME, False, dwThreadId)
    If (hThread = 0) Then
        PrintLastError
        Return False
    End If
    ResumeThread hThread
    If (Not PrintLog(0, "")) Then
        CloseHandle  hThread
        Return False
    End If
    CloseHandle hThread
    Return True
End Function

'ZwSetInformationThread ThreadAttachContainer

Public Sub KillThread(ByVal dwThreadId As DWORD)
    Dim IsCritical As BOOLEAN
    If (NT_SUCCESS(IsThreadCritical(dwThreadId, @IsCritical))) Then
        If (IsCritical) Then
            If (AfxMsg("此线程处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDNO) Then
                Exit Sub 
            Else
                SetCriticalThread(dwThreadId, False)
            End If
        End If
    End If
    Dim hThread As HANDLE = OpenThread(THREAD_TERMINATE,False,dwThreadId)
    If (hThread = 0) Then
        MsgBox "打开线程失败,原因:" & WinErrorMsg(GetLastError),"提示"
        Goto Err1
    End If
    If (TerminateThread(hThread,0) = 0) Then
        MsgBox "结束线程失败,原因:" & WinErrorMsg(GetLastError),"提示"
        Goto Err1
    End If
    MsgBox "结束线程成功!","提示"
    Err1 : 
    CloseHandle hThread
End Sub