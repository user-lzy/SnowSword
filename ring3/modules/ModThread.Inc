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

' 线程状态枚举（Windows内核未公开，需准确匹配）
Enum THREAD_STATE
    StateInitialized = 0      ' 线程已初始化但未启动
    StateReady = 1            ' 线程就绪（等待CPU调度）
    StateRunning = 2          ' 线程正在运行
    StateStandby = 3          ' 线程即将运行（仅一个线程处于此状态）
    StateTerminated = 4       ' 线程已终止
    StateWait = 5             ' 线程等待中
    StateTransition = 6       ' 线程状态转换中
    StateUnknown = 7          ' 未知状态
End Enum

' 等待原因枚举（KWAIT_REASON，Windows内核未公开）
Enum KWAIT_REASON
    WaitReasonExecutive = 0   ' 内核执行体等待
    WaitReasonFreePage = 1    ' 等待空闲内存页
    WaitReasonPageIn = 2      ' 等待页面调入内存
    WaitReasonPoolAllocation = 3 ' 等待池分配
    WaitReasonDelayExecution = 4 ' 延迟执行（Sleep）
    WaitReasonSuspended = 5   ' 线程被挂起
    WaitReasonUserRequest = 6 ' 用户请求
    WaitReasonWrExecutive = 7 ' 等待内核执行体（可唤醒）
    WaitReasonWrFreePage = 8  ' 等待空闲页（可唤醒）
    WaitReasonWrPageIn = 9    ' 等待页面调入（可唤醒）
    WaitReasonWrPoolAllocation = 10 ' 等待池分配（可唤醒）
    WaitReasonWrDelayExecution = 11 ' 延迟执行（可唤醒）
    WaitReasonWrSuspended = 12 ' 挂起（可唤醒）
    WaitReasonWrUserRequest = 13 ' 用户请求（可唤醒）
    WaitReasonWrEventPair = 14 ' 等待事件对
    WaitReasonWrQueue = 15    ' 等待队列
    WaitReasonWrLpcReceive = 16 ' 等待LPC接收
    WaitReasonWrLpcReply = 17 ' 等待LPC回复
    WaitReasonWrVirtualMemory = 18 ' 等待虚拟内存
    WaitReasonWrPageOut = 19  ' 等待页面调出
    WaitReasonWrRendezvous = 20 ' 等待会合
    WaitReasonSpare21 = 21    ' 备用
    WaitReasonSpare22 = 22    ' 备用
    WaitReasonSpare23 = 23    ' 备用
    WaitReasonSpare24 = 24    ' 备用
    WaitReasonWrKernel = 25   ' 等待内核
    WaitReasonMaximum = 26    ' 最大值（边界）
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

' ==============================================================================
' 3. 解析导出表（稳定版）
' ==============================================================================
Private Function Get32BitFuncAddress(ByVal hProcess As HANDLE, ByVal moduleBase As DWORD, ByVal funcName As String) As DWORD
    Dim dosHeader As IMAGE_DOS_HEADER
    Dim ntHeaders32 As IMAGE_NT_HEADERS32
    Dim bytRead As size_t = 0
    
    ' 读取DOS头
    If (ReadProcessMemory(hProcess, Cast(LPCVOID, Cast(ULONG_PTR, moduleBase)), @dosHeader, SizeOf(IMAGE_DOS_HEADER), @bytRead) = 0) OrElse (bytRead <> SizeOf(IMAGE_DOS_HEADER)) Then
        Print "[-] 读取DOS头失败: " & GetLastError
        Print bytRead & "/" & SizeOf(IMAGE_DOS_HEADER)
        Return 0
    End If
    
    If dosHeader.e_magic <> &h5A4D Then Return 0
    ''Print "10"
    ' 读取NT头
    Dim ntHeaderAddr As ULONG_PTR = moduleBase + dosHeader.e_lfanew
    If (ReadProcessMemory(hProcess, Cast(LPCVOID, ntHeaderAddr), @ntHeaders32, SizeOf(IMAGE_NT_HEADERS32), @bytRead) = 0) OrElse (bytRead <> SizeOf(IMAGE_NT_HEADERS32)) Then
        Print "[-] 读取NT头失败: " & GetLastError
        Print bytRead & "/" & SizeOf(IMAGE_NT_HEADERS32)
        Return 0
    End If
    
    If ntHeaders32.Signature <> &h00004550 Then Return 0
    
    ' 读取导出目录
    Dim exportRVA As DWORD = ntHeaders32.OptionalHeader.DataDirectory(0).VirtualAddress
    If exportRVA = 0 Then Return 0
    
    Dim exportDir As IMAGE_EXPORT_DIRECTORY
    Dim exportDirAddr As ULONG_PTR = moduleBase + exportRVA
    If (ReadProcessMemory(hProcess, Cast(LPCVOID, exportDirAddr), @exportDir, SizeOf(IMAGE_EXPORT_DIRECTORY), @bytRead) = 0) OrElse (bytRead <> SizeOf(IMAGE_EXPORT_DIRECTORY)) Then
        Print "[-] 读取导出目录失败: " & GetLastError
        Print bytRead & "/" & SizeOf(IMAGE_EXPORT_DIRECTORY)
        Return 0
    End If
    
    ' 遍历名称表
    Dim namesAddr As ULONG_PTR = moduleBase + exportDir.AddressOfNames
    Dim ordinalsAddr As ULONG_PTR = moduleBase + exportDir.AddressOfNameOrdinals
    Dim functionsAddr As ULONG_PTR = moduleBase + exportDir.AddressOfFunctions
    Print exportDir.NumberOfNames
    For i As DWORD = 0 To exportDir.NumberOfNames - 1
        ' 读取函数名RVA
        Dim nameRVA As DWORD
        If (ReadProcessMemory(hProcess, Cast(LPCVOID, namesAddr + i * 4), @nameRVA, 4, @bytRead) = 0) OrElse (bytRead < 4) Then
            Print bytRead & "/4" 
            Continue For
        End If
        
        ' 读取函数名（只读前64字节快速匹配）
        Dim nameAddr As ULONG_PTR = moduleBase + nameRVA
        Dim nameBuffer As ZString * 64
        If (ReadProcessMemory(hProcess, Cast(LPCVOID, nameAddr), @nameBuffer, 64, @bytRead) = 0) OrElse (bytRead < 64) Then
            Print bytRead & "/64"
            Continue For
        End If
        ''Print nameBuffer
        ' 快速比较
        If Left(nameBuffer, Len(funcName)) = funcName Then
            ' 读取序号
            Dim ordinal As WORD
            ReadProcessMemory(hProcess, Cast(LPCVOID, ordinalsAddr + i*2), @ordinal, 2, @bytRead)
            
            ' 读取函数地址
            Dim funcRVA As DWORD
            ReadProcessMemory(hProcess, Cast(LPCVOID, functionsAddr + ordinal*4), @funcRVA, 4, @bytRead)
            
            Dim finalAddr As DWORD = moduleBase + funcRVA
            Print "[+] 找到 " & funcName & " @ 0x" & Hex(finalAddr)
            Return finalAddr
        End If
    Next
    
    Print "[-] 未找到函数: " & funcName
    Return 0
End Function

' ==============================================================================
' 2. 获取32位进程LoadLibraryW地址（修复版）
' ==============================================================================
Private Function Get32BitProcAddress(ByVal hProcess As HANDLE, moduleName As String, FuncName As String) As DWORD
    Const ProcessWow64Information = 26
    Dim wow64Peb As ULONG_PTR
    Dim st As NTSTATUS
    Dim bytRead As size_t = 0
    
    ' 获取WOW64 PEB指针
    st = NtQueryInformationProcess(hProcess, ProcessWow64Information, @wow64Peb, SizeOf(wow64Peb), NULL)
    If Not NT_SUCCESS(st) Then
        Print "[-] NtQueryInformationProcess失败: 0x" & Hex(st)
        Return 0
    End If
    
    If wow64Peb = NULL Then
        Print "[-] WOW64 PEB为NULL"
        Return 0
    End If
    
    Dim peb32Addr As DWORD = Cast(DWORD, wow64Peb)  ' 32位地址
    Print "[*] WOW64 PEB地址: 0x" & Hex(peb32Addr)
    
    ' 读取PEB（只读必要的字段）
    Dim peb As PEB32
    If (ReadProcessMemory(hProcess, Cast(LPCVOID, Cast(ULONG_PTR, peb32Addr)), @peb, SizeOf(PEB32), @bytRead) = 0) OrElse (bytRead <> SizeOf(PEB32)) Then
        Print "[-] 读取PEB失败: " & GetLastError
        Print bytRead & "/" & SizeOf(PEB32)
        Return 0
    End If
    
    If peb.Ldr = 0 Then
        Print "[-] Ldr指针为NULL"
        Return 0
    End If
    
    Print "[*] PEB_LDR_DATA地址: 0x" & Hex(peb.Ldr)
    
    ' 读取LDR头
    Dim ldr32 As PEB_LDR_DATA32
    If (ReadProcessMemory(hProcess, Cast(LPCVOID, Cast(ULONG_PTR, peb.Ldr)), @ldr32, SizeOf(PEB_LDR_DATA32), @bytRead) = 0) OrElse (bytRead <> SizeOf(PEB_LDR_DATA32)) Then
        Print "[-] 读取LDR失败: " & GetLastError
        Print bytRead & "/" & SizeOf(PEB_LDR_DATA32)
        Return 0
    End If
    
    Print "[*] LDR.InMemoryOrder.Flink: 0x" & Hex(ldr32.InMemoryOrderModuleList.Flink)
    Print "[*] LDR.InMemoryOrder.Blink: 0x" & Hex(ldr32.InMemoryOrderModuleList.Blink)
    
    ' 关键修复：计算远程链表头地址
    Dim listHeadAddr As DWORD = peb.Ldr + offsetof(PEB_LDR_DATA32, InMemoryOrderModuleList)
    Dim currentLink As DWORD = ldr32.InMemoryOrderModuleList.Flink
    
    Print "[*] 链表头地址: 0x" & Hex(listHeadAddr)
    Print "[*] 开始遍历模块（最多50个）..."
    
    Dim i As Integer = 0
    Do While currentLink <> 0 And currentLink <> listHeadAddr And i < 50
        ' 计算条目基址（减去InMemoryOrderLinks偏移）
        Dim entryAddr As DWORD = currentLink - offsetof(LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks)
        
        Dim entry32 As LDR_DATA_TABLE_ENTRY32
        If (ReadProcessMemory(hProcess, Cast(LPCVOID, Cast(ULONG_PTR, entryAddr)), @entry32, SizeOf(LDR_DATA_TABLE_ENTRY32), @bytRead) = 0) OrElse (bytRead <> SizeOf(LDR_DATA_TABLE_ENTRY32)) Then
            Print "[-] 读取条目失败: " & GetLastError & " @ 0x" & Hex(entryAddr)
            Print bytRead & "/" & SizeOf(LDR_DATA_TABLE_ENTRY32)
            Exit Do
        End If
        
        ' 读取并显示模块名
        If entry32.BaseDllName.Length > 0 And entry32.BaseDllName.Buffer <> 0 Then
            Dim nameLen As Integer = entry32.BaseDllName.Length \ 2
            Dim nameBuffer As WString * 260
            
            If (ReadProcessMemory(hProcess, Cast(LPCVOID, Cast(ULONG_PTR, entry32.BaseDllName.Buffer)), @nameBuffer, entry32.BaseDllName.Length, NULL) <> 0) AndAlso (bytRead >= entry32.BaseDllName.Length) Then
                nameBuffer[nameLen] = 0  ' 确保终止
                ''Print "模块:" & nameBuffer
                Dim strmodule As StringW = LeftW(nameBuffer, nameLen)
                ''Print "  [" & i & "] " & moduleName & " (基址: 0x" & WHex(entry32.DllBase) & ")"
                
                ' 关键修复：不区分大小写比较
                If LCaseW(strmodule) = LCaseW(moduleName) Then
                    Print "[+] 找到kernel32.dll!地址:" & WHex(entry32.DllBase)
                    Return Get32BitFuncAddress(hProcess, entry32.DllBase, FuncName)
                End If
            Else
                Print "  [" & i & "] 读取名称失败: " & WinErrorMsg(GetLastError) & GetLastError
                Print bytRead & "/" & entry32.BaseDllName.Length
            End If
        Else
            Print "  [" & i & "] 名称为空或缓冲区无效"
        End If
        
        ' 下一个链接
        currentLink = entry32.InMemoryOrderLinks.Flink
        i += 1
    Loop
    
    Print "[-] 遍历完成，未找到kernel32.dll (检查" & i & "个模块)"
    Return 0
End Function

Function RemoteInjectDll(ByVal ProcessId As DWORD, ByVal wszInjectDllPathName As StringW) As Boolean
    Dim hProcess As HANDLE
    Dim lpBuffer As LPVOID
    Dim dwDllPathSize As size_t = (wszInjectDllPathName.WstrLen + 1) * 2
    Dim ThreadHandle As HANDLE
    Dim pfnLoadLibraryW As ULONG_PTR
    Dim pfnNtTerminateThread As ULONG_PTR
    Dim st As NTSTATUS
    Dim dwExitCode As DWORD
    Dim IsWow64 As Boolean
    Dim written As size_t
    Dim verifyBuf As WString * 260
    /'
    push lpDllPath
    mov  eax, LoadLibraryW
    call eax

    push 0
    push 0
    mov  eax, NtTerminateThread
    Call eax
    '/
    Dim shellcode(22) As Byte = {&h68,0,0,0,0, &hB8,0,0,0,0, &hFF,&hD0, &h6A,&h00, &h6A,&h00, &hB8,0,0,0,0, &hFF,&hD0}
    Dim lpBuffer32 As DWORD
    
    Function = False
    
    Print "======================================"
    Print "[*] 目标进程PID: " & ProcessId
    Print "[*] DLL路径: " & *wszInjectDllPathName.wstrPtr
    Print "======================================"
    
    ' 打开目标进程
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, False, ProcessId)
    If hProcess = NULL Then
        Print "[-] OpenProcess失败: 错误码 " & GetLastError
        Goto Cleanup
    End If
    
    ' 判断位数
    IsWow64 = IsProcessWow64_Ex(hProcess)
    Print "[*] 进程类型: " & IIf(IsWow64, "32位(WOW64)", "64位(原生)")
    
    ' 分配内存并写入DLL路径
    lpBuffer = VirtualAllocEx(hProcess, NULL, dwDllPathSize, MEM_COMMIT, PAGE_READWRITE)
    If lpBuffer = NULL Then
        Print "[-] VirtualAllocEx失败: " & GetLastError
        Goto Cleanup
    End If
    
    If WriteProcessMemory(hProcess, lpBuffer, wszInjectDllPathName.wstrPtr, dwDllPathSize, @written) = 0 Then
        Print "[-] WriteProcessMemory失败: " & GetLastError
        Goto Cleanup
    End If
    
    ' 验证写入
    ReadProcessMemory(hProcess, lpBuffer, @verifyBuf, dwDllPathSize, NULL)
    Print "[+] 路径写入验证: " & verifyBuf
    
    ' 区分处理
    If IsWow64 Then
        ' 32位进程
        Print "[*] 注入32位进程..."
        pfnLoadLibraryW = Get32BitProcAddress(hProcess, "kernel32.dll", "LoadLibraryW")
        
        If pfnLoadLibraryW = 0 Then
            Print "[-] 获取LoadLibraryW地址失败"
            Goto Cleanup
        End If
        
        Print "[+] LoadLibraryW地址: 0x" & Hex(pfnLoadLibraryW)
        
        pfnNtTerminateThread = Get32BitProcAddress(hProcess, "ntdll.dll", "NtTerminateThread")
        
        If pfnNtTerminateThread = 0 Then
            Print "[-] 获取NtTerminateThread地址失败"
            Goto Cleanup
        End If
        
        Print "[+] NtTerminateThread地址: 0x" & Hex(pfnNtTerminateThread)
        
        ' 构造x86 Shellcode
        lpBuffer32 = Cast(DWORD, Cast(ULONG_PTR, lpBuffer))
        
        memcpy @shellcode(1), @lpBuffer32, 4
        memcpy @shellcode(6), @pfnLoadLibraryW, 4
        memcpy @shellcode(17), @pfnNtTerminateThread, 4
        
        ' 分配可执行内存
        Dim lpShellcode As LPVOID = VirtualAllocEx(hProcess, NULL, UBound(shellcode)+1, MEM_COMMIT, PAGE_EXECUTE_READ)
        If lpShellcode = NULL Then
            Print "[-] Shellcode内存分配失败: " & GetLastError
            Goto Cleanup
        End If
        
        If WriteProcessMemory(hProcess, lpShellcode, @shellcode(0), Ubound(shellcode)+1, NULL) = 0 Then
            Print "[-] Shellcode写入失败: " & GetLastError
            VirtualFreeEx(hProcess, lpShellcode, 0, MEM_RELEASE)
            Goto Cleanup
        End If
        
        Print "[+] Shellcode已写入: 0x" & Hex(lpShellcode)
        
        ' 执行Shellcode
        st = ZwCreateThreadEx(@ThreadHandle, THREAD_ALL_ACCESS, NULL, hProcess, _
            Cast(LPTHREAD_START_ROUTINE, lpShellcode), NULL, 0, 0, 0, 0, NULL)
        
        If Not NT_SUCCESS(st) Then
            Print "[-] ZwCreateThreadEx失败: 0x" & Hex(st)
            VirtualFreeEx(hProcess, lpShellcode, 0, MEM_RELEASE)
            Goto Cleanup
        End If
        
        'WaitForSingleObject(ThreadHandle, INFINITE)
        'VirtualFreeEx(hProcess, lpShellcode, 0, MEM_RELEASE)
    Else
        ' 64位进程
        Print "[*] 注入64位进程..."
        pfnLoadLibraryW = Cast(ULONG_PTR, GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryW"))
        
        Print "[+] LoadLibraryW地址: 0x" & Hex(pfnLoadLibraryW)
        Print "[+] 参数地址: 0x" & Hex(lpBuffer)
        
        ' 关键：验证参数指针在目标进程中的有效性
        Dim memInfo As MEMORY_BASIC_INFORMATION
        VirtualQueryEx(hProcess, lpBuffer, @memInfo, SizeOf(memInfo))
        Print "[+] 内存状态: State=" & Hex(memInfo.State) & " Protect=" & Hex(memInfo.Protect)
        
        st = ZwCreateThreadEx(@ThreadHandle, THREAD_ALL_ACCESS, NULL, hProcess, _
            Cast(LPTHREAD_START_ROUTINE, pfnLoadLibraryW), lpBuffer, 0, 0, 0, 0, NULL)
        
        If Not NT_SUCCESS(st) Then
            Print "[-] ZwCreateThreadEx失败: 0x" & Hex(st)
            Goto Cleanup
        End If
        
        'WaitForSingleObject(ThreadHandle, INFINITE)
    End If
    
    ' 获取退出码
    /'If GetExitCodeThread(ThreadHandle, @dwExitCode) = 0 Then
        Print "[-] GetExitCodeThread失败: " & GetLastError
        Goto Cleanup
    End If
    
    If dwExitCode = 0 Then
        Print "[-] DLL加载失败! 退出码: 0"
        Print "[!] 可能原因:"
        Print "    1. DLL路径在目标进程中无效"
        Print "    2. DLL依赖项缺失（如C++运行库）"
        Print "    3. DllMain返回FALSE"
    Else
        Print "[+] 注入成功! 模块句柄: 0x" & Hex(dwExitCode)
        Function = True
    End If'/
    Function = True
    
Cleanup:
    'If lpBuffer Then VirtualFreeEx(hProcess, lpBuffer, 0, MEM_RELEASE)
    If hProcess Then CloseHandle(hProcess)
    If ThreadHandle Then CloseHandle(ThreadHandle)
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

Function WaitReasonToString(ByVal reason As KWAIT_REASON) As String
    Select Case reason
        Case WaitReasonExecutive: Return "Executive"
        Case WaitReasonFreePage: Return "FreePage"
        Case WaitReasonPageIn: Return "PageIn"
        Case WaitReasonPoolAllocation: Return "PoolAllocation"
        Case WaitReasonDelayExecution: Return "DelayExecution(Sleep)"
        Case WaitReasonSuspended: Return "Suspended"
        Case WaitReasonUserRequest: Return "UserRequest"
        Case WaitReasonWrExecutive: Return "WrExecutive"
        Case WaitReasonWrFreePage: Return "WrFreePage"
        Case WaitReasonWrPageIn: Return "WrPageIn"
        Case WaitReasonWrPoolAllocation: Return "WrPoolAllocation"
        Case WaitReasonWrDelayExecution: Return "WrDelayExecution"
        Case WaitReasonWrSuspended: Return "WrSuspended"
        Case WaitReasonWrUserRequest: Return "WrUserRequest"
        Case WaitReasonWrEventPair: Return "WrEventPair"
        Case WaitReasonWrQueue: Return "WrQueue"
        Case WaitReasonWrLpcReceive: Return "WrLpcReceive"
        Case WaitReasonWrLpcReply: Return "WrLpcReply"
        Case WaitReasonWrVirtualMemory: Return "WrVirtualMemory"
        Case WaitReasonWrPageOut: Return "WrPageOut"
        Case WaitReasonWrRendezvous: Return "WrRendezvous"
        Case WaitReasonWrKernel: Return "WrKernel"
        Case Else: Return "Unknown(" & Str(reason) & ")"
    End Select
End Function

Function ThreadStateToString(ByVal threadInfo As SYSTEM_THREAD_INFORMATION Ptr) As StringW
    Dim stateStr As StringW
    Dim suspendCount As ULONG = 0
    Dim threadHandle As HANDLE = NULL
    Dim ntStatus As NTSTATUS
    
    ' 第一步：转换基础状态
    Select Case threadInfo->ThreadState
        Case StateInitialized: stateStr = "Initialized"
        Case StateReady: stateStr = "Ready"
        Case StateRunning: stateStr = "Running"
        Case StateStandby: stateStr = "Standby"
        Case StateTerminated: stateStr = "Terminated"
        Case StateWait: 
            stateStr = "Wait (" & WaitReasonToString(threadInfo->WaitReason) & ")"
        Case StateTransition: stateStr = "Transition"
        Case Else: stateStr = "UnknownState(" & WStr(threadInfo->ThreadState) & ")"
    End Select
    
    ' 第二步：处理挂起状态（获取挂起次数）
    ' 仅当等待原因为Suspended/WrSuspended，或状态为Wait且关联挂起时处理
    If (threadInfo->WaitReason = WaitReasonSuspended) Or (threadInfo->WaitReason = WaitReasonWrSuspended) Then
        ' 打开线程句柄（需要THREAD_SUSPEND_RESUME权限）
        If IsDriverLoaded Then
            threadHandle = SnowSword_OpenThread(Cast(DWORD, Cast(UInteger, threadInfo->ClientId.UniqueThread)))
        Else
            threadHandle = OpenThread(THREAD_SUSPEND_RESUME, False, Cast(DWORD, Cast(UInteger, threadInfo->ClientId.UniqueThread)))
        End If
        
        If threadHandle <> NULL Then
            ' 调用ZwResumeThread获取挂起计数（此调用会减少挂起计数1）
            ntStatus = ZwResumeThread(threadHandle, @suspendCount)
            If NT_SUCCESS(ntStatus) Then ' NTSTATUS_SUCCESS = 0
                ' 重新挂起线程，恢复原有挂起状态
                ZwSuspendThread threadHandle, @suspendCount
                ' 拼接挂起次数（注意：suspendCount是调用Resume前的计数）
                stateStr &= " [Suspended x" & Str(suspendCount) & "]"
            Else
                stateStr &= " [Suspended x?]"
                Print "[ThreadStateToString]获取挂起次数失败，错误码：" & WStr(ntStatus)
            End If
            ' 关闭线程句柄
            CloseHandle threadHandle
        Else
            stateStr &= " [Suspended x?]"
            Print "[ThreadStateToString]无法打开线程句柄，错误码：" & WStr(GetLastError)
        End If
    End If
    
    Return stateStr
End Function

Sub GetThreadList2()
    If Not IsDriverLoaded Then Exit Sub
    For TID As DWORD = 0 To &H20000
        Dim pEThread As ULONG64 = GetEThread(TID)
        If pEThread <> NULL Then Print "Thread " & TID
    Next
End Sub

Function NtPathToDosPathW(ByVal FullNtPath As LPWSTR, ByVal FullDosPath As LPWSTR) As Boolean
    Dim MupPrefix As WString * 12 = "\Device\Mup\"
    Dim RedirectorPrefix As WString * 28 = "\Device\LanmanRedirector\"
    Dim DosDevice As WString * 4
    Dim NtPath As WString * 64
    Dim NtPathLen As ULong
    
    '--- 第一步：处理网络路径（无盘符）---
    If (InStrW(*FullNtPath, MupPrefix) = 1) Then
        wcscpy(FullDosPath, StrPtrW("\"))
        wcscat(FullDosPath, FullNtPath + wcslen(@MupPrefix))
        Return True
    End If
    
    If (InStrW(*FullNtPath, RedirectorPrefix) = 1) Then
        ' LanmanRedirector格式: \Device\LanmanRedirector\;Z:0000000\Server\Share
        ' 跳过前缀到第一个反斜杠后的服务器名
        Dim ServerStart As LPWSTR = FullNtPath + wcslen(@RedirectorPrefix)
        While (*ServerStart <> 0 AndAlso *ServerStart <> AscW("\"))
            ServerStart += 1
        Wend
        If (*ServerStart = AscW("\")) Then
            wcscpy(FullDosPath, StrPtrW("\\"))
            wcscat(FullDosPath, ServerStart + 1) ' +1跳过反斜杠
            Return True
        End If
    End If
    
    '--- 第二步：处理本地路径（有盘符）---
    For i As Integer = 65 To 90
        DosDevice = Chr(i) & ":"
        If (QueryDosDeviceW(@DosDevice, @NtPath, 64)) Then
            NtPathLen = wcslen(@NtPath)
            If (NtPathLen > 0 And InStrW(*FullNtPath, NtPath) = 1) Then
                wcscpy(FullDosPath, @DosDevice)
                wcscat(FullDosPath, FullNtPath + NtPathLen)
                Return True
            End If
        End If
    Next
    
    Return False
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
    If (StartAddr = 0) Then Print "[GetThreadStartAddr]" & Hex(st)
    Return StartAddr
End Function

'Sub GetModuleByAddress(ByVal dwProcessId As DWORD, ByVal Addr As PVOID, ByVal pstrName As LPWSTR)
'    Dim hProcess As HANDLE
'    If dwProcessId = 4 Then
'        *pstrName = GetDriverPathByAddr(Cast(ULONG64, Addr))
'        Exit Sub
'    End If
'    
'    If IsDriverLoaded Then
'        hProcess = SnowSword_OpenProcess(dwProcessId)
'    Else
'        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
'    End If
'    Dim NtPath As WString * MAX_PATH
'    If (hProcess = NULL) Then Exit Sub'PrintLastError '"1"'PrintLastError
'    'Print "0x" & Hex(Addr)'删掉这句会导致所有线程所在模块都变成一样的乱码
'    'Print "1"
'    Dim ret As DWORD = GetMappedFileNameW(hProcess, Addr, @NtPath, MAX_PATH)
'    'Print "2"
'    If (ret > 0) Then
'        Dim ModulePath As WString * MAX_PATH
'        If (NtPathToDosPathW(NtPath, @ModulePath)) Then
'            'Print "3"
'            ZwClose hProcess
'            *pstrName = ModulePath
'            Exit Sub
'        End If
'        Print "NtPathToDosPathW Failed"
'    End If
'    ZwClose hProcess
'End Sub

Sub GetModuleByAddress(ByVal dwProcessId As DWORD, ByVal Addr As PVOID, ByVal pstrName As LPWSTR)
    Dim hProcess As HANDLE
    
    ' 特殊处理System进程
    If dwProcessId = 4 Then
        *pstrName = GetDriverPathByAddr(Cast(ULONG64, Addr))
        Exit Sub
    End If
    
    ' 根据是否加载驱动选择打开进程方式
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    End If
    
    If hProcess = NULL Then 
        *pstrName = ""
        Exit Sub
    End If
    
    ' 初始化输出为空字符串
    *pstrName = ""
    
    ' 动态加载 ntdll.dll 和 ZwQueryVirtualMemory 函数
    Dim library As Any Ptr = DyLibLoad("ntdll.dll")
    If library = NULL Then
        If IsDriverLoaded Then ZwClose(hProcess) Else CloseHandle(hProcess)
        Exit Sub
    End If
    
    Dim ZwQueryVirtualMemory As Function(ByVal As HANDLE, ByVal As PVOID, ByVal As MEMORY_INFORMATION_CLASS, _
                                        ByVal As PVOID, ByVal As SIZE_T, ByVal As SIZE_T Ptr) As NTSTATUS
    ZwQueryVirtualMemory = DyLibSymbol(library, "ZwQueryVirtualMemory")
    
    If ZwQueryVirtualMemory = NULL Then
        DyLibFree(library)
        If IsDriverLoaded Then ZwClose(hProcess) Else CloseHandle(hProcess)
        Exit Sub
    End If
    
    ' 第一次调用：获取所需缓冲区大小
    Dim requiredSize As SIZE_T = 0
    Dim dummyBuffer(0) As Byte  ' 极小缓冲区触发STATUS_BUFFER_OVERFLOW
    
    Dim status As NTSTATUS = ZwQueryVirtualMemory(hProcess, Addr, MemorySectionName, _
                                                   @dummyBuffer(0), 0, @requiredSize)
    
    ' 检查是否成功获取所需大小
    If (status = &hC0000023 OrElse status = &h80000005 OrElse status = STATUS_INFO_LENGTH_MISMATCH) And requiredSize > 0 Then
        ' 分配足够大的缓冲区（结构体 + 字符串数据）
        Dim buffer(requiredSize - 1) As Byte
        
        ' 第二次调用：获取实际数据
        status = ZwQueryVirtualMemory(hProcess, Addr, MemorySectionName, _
                                       @buffer(0), requiredSize, @requiredSize)
        
        If NT_SUCCESS(status) Then
            ' 从缓冲区解析UNICODE_STRING结构
            Dim sectionName As PUNICODE_STRING = CPtr(PUNICODE_STRING, @buffer(0))
            
            ' 检查结构有效性
            If sectionName->buffer <> NULL And sectionName->Length > 0 Then
                ' 提取NT路径（Length是字节数，需转换为字符数）
                Dim charCount As Integer = sectionName->Length \ SizeOf(WCHAR)
                If charCount > 0 And charCount < MAX_PATH Then
                    Dim NtPath As WString * MAX_PATH
                    wcsncpy(@NtPath, sectionName->Buffer, charCount)
                    NtPath[charCount] = 0
                    
                    ' 转换为DOS路径
                    Dim ModulePath As WString * MAX_PATH
                    If NtPathToDosPathW(NtPath, @ModulePath) Then
                        *pstrName = ModulePath
                    End If
                End If
            Else
                Print "invalid sectionName"
            End If
        Else
            Print "[GetModuleByAddress]ZwQueryVirtualMemory2(" & WHex(Addr) & ")" & WHex(status)
        End If
    Else
        Print "[GetModuleByAddress]ZwQueryVirtualMemory1(" & WHex(Addr) & ")" & WHex(status)
    End If
    
    ' 清理资源
    DyLibFree(library)
    If IsDriverLoaded Then ZwClose(hProcess) Else CloseHandle(hProcess)
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

Function GetThreadList(ByVal ProcessId As DWORD, lvwThread As Class_ListView) As Integer
    Dim status As Long
    Dim bufferSize As ULong = 0, returnLength As ULong= 0
    Dim buffer As Any Ptr = NULL
    Dim procInfo As SYSTEM_PROCESS_INFORMATION Ptr
    Dim i As Long
    
    InitListViewData lvwThread
    
    ' 第一次调用：获取所需缓冲区大小
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, 0, 0, @returnLength)
    If status <> STATUS_INFO_LENGTH_MISMATCH Then  ' 表示需要更大缓冲区
        Print "[GetThreadList]获取缓冲区大小失败，错误码: " & Hex(status)
        Return -1
    End If
    
    ' 分配缓冲区（适当增大避免溢出）
    bufferSize = returnLength + 4096
    buffer = Allocate(bufferSize)
    If buffer = NULL Then
        Print "[GetThreadList]内存分配失败"
        Return -1
    End If
    'Print returnLength & " " & returnLength / SizeOf(SYSTEM_PROCESS_INFORMATION)
    ' 第二次调用：获取进程信息
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, buffer, bufferSize, @returnLength)
    If status <> 0 Then  ' 0表示STATUS_SUCCESS
        Print "[GetThreadList]获取进程信息失败，错误码: " & Hex(status)
        Deallocate buffer
        Return -1
    End If
    
    ' 遍历进程信息链表
    procInfo = buffer
    Do While True
        ' 遍历当前进程的线程
        If procInfo->ProcessId = ProcessId Then
            Dim threadInfo As SYSTEM_THREAD_INFORMATION Ptr = @procInfo->Threads(0)
            For i As ULong = 0 To procInfo->ThreadCount - 1
                Dim strEThread As StringW
                If IsDriverLoaded Then strEThread = "0x" & WHex(GetEThread(Cast(DWORD, Cast(UInteger, threadInfo->ClientID.UniqueThread))))
                
                Dim strModulePath As WString * MAX_PATH
                GetModuleByAddress ProcessId, threadInfo->StartAddress, @strModulePath
                'Print "线程ID: " & threadInfo->ClientID.UniqueThread & ", 切换次数: " & threadInfo->ContextSwitchCount & ", 起始地址:0x" & WHex(threadInfo->StartAddress)
                AddItemColListEx lvwThread, 7,,, WStr(Cast(UInteger, threadInfo->ClientID.UniqueThread)), WStr(threadInfo->Priority), WStr(ThreadStateToString(threadInfo)), _
                WStr(strEThread), WStr("0x" & WHex(threadInfo->StartAddress)), WStr("0x" & WHex(GetTeb(Cast(DWORD, Cast(UInteger, threadInfo->ClientID.UniqueThread))))), WStr(strModulePath)
                threadInfo += 1
                i += 1
            Next
        End If
        
        ' 移动到下一个进程（偏移量为0表示结束）
        If procInfo->NextEntryDelta = 0 Then Exit Do
        procInfo = Cast(SYSTEM_PROCESS_INFORMATION Ptr, Cast(Byte Ptr, procInfo) + procInfo->NextEntryDelta)
    Loop
    
    Deallocate buffer
    CommitListViewView lvwThread
    /'Dim i As Long
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
    End If'/
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

'Function GetKernelThreadList(lvwKernelThread As Class_ListView) As DWORD
'    Dim thre As THREADENTRY32
'    Dim hSnapshot As HANDLE, strModulePath As WString * 260, ThreadStartAddress As ULONG64
'    Dim pEThread As HANDLE
'    Dim i As DWORD
'    
'    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 4)
'    If hSnapshot > 0 Then
'        thre.dwSize = SizeOf(THREADENTRY32)
'        If Thread32First(hSnapshot, @thre) Then
'            Do
'                If thre.th32OwnerProcessID = 4 Then
'                    pEThread = CPtr(HANDLE, GetEThread(thre.th32ThreadID))
'                    ThreadStartAddress = Cast(ULONG64, GetThreadStartAddr(thre.th32ThreadID))
'                    'If ThreadStartAddress = NULL Then Print "TID:" & thre.th32ThreadID & " " & WinErrorMsg(GetLastError) & GetLastError
'                    'Print "TID:" & TID & " StartAddr:0x" & WHex(StartAddress)
'                    Dim DriverPath As StringW = GetDriverPathByAddr(ThreadStartAddress)
'                    If DriverPath <> "" Then
'                        lvwKernelThread.AddItemColList,, WStr(thre.th32ThreadID), WStr(thre.tpBasePri), _
'                        GetThreadState(thre.th32ThreadID), "0x" & WHex(pEThread), "0x" & WHex(ThreadStartAddress), DriverPath
'                        i += 1
'                    End If
'                End If
'            Loop Until Thread32Next(hSnapshot, @thre) = 0
'        End If
'    End If
'    Return i
'End Function

Function GetKernelThreadList(lvwKernelThread As Class_ListView) As Integer
    Dim status As Long
    Dim bufferSize As ULong = 0, returnLength As ULong= 0
    Dim buffer As Any Ptr = NULL
    Dim procInfo As SYSTEM_PROCESS_INFORMATION Ptr
    Dim i As Long
    
    InitListViewData lvwKernelThread
    
    ' 第一次调用：获取所需缓冲区大小
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, 0, 0, @returnLength)
    If status <> STATUS_INFO_LENGTH_MISMATCH Then  ' 表示需要更大缓冲区
        Print "[GetThreadList]获取缓冲区大小失败，错误码: " & Hex(status)
        Return -1
    End If
    
    ' 分配缓冲区（适当增大避免溢出）
    bufferSize = returnLength + 4096
    buffer = Allocate(bufferSize)
    If buffer = NULL Then
        Print "[GetThreadList]内存分配失败"
        Return -1
    End If
    'Print returnLength & " " & returnLength / SizeOf(SYSTEM_PROCESS_INFORMATION)
    ' 第二次调用：获取进程信息
    status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, buffer, bufferSize, @returnLength)
    If status <> 0 Then  ' 0表示STATUS_SUCCESS
        Print "[GetThreadList]获取进程信息失败，错误码: " & Hex(status)
        Deallocate buffer
        Return -1
    End If
    
    ' 遍历进程信息链表
    procInfo = buffer
    Do While True
        ' 遍历当前进程的线程
        If procInfo->ProcessId = 4 Then ' System进程
            Dim threadInfo As SYSTEM_THREAD_INFORMATION Ptr = @procInfo->Threads(0)
            For i As ULong = 0 To procInfo->ThreadCount - 1
                Dim strEThread As StringW
                If IsDriverLoaded Then strEThread = "0x" & WHex(GetEThread(Cast(DWORD, Cast(UInteger, threadInfo->ClientID.UniqueThread))))
                
                Dim strModulePath As WString * MAX_PATH
                GetModuleByAddress 4, threadInfo->StartAddress, @strModulePath
                'Print "线程ID: " & threadInfo->ClientID.UniqueThread & ", 切换次数: " & threadInfo->ContextSwitchCount & ", 起始地址:0x" & WHex(threadInfo->StartAddress)
                AddItemColListEx lvwKernelThread, 6,,, WStr(Cast(UInteger, threadInfo->ClientID.UniqueThread)), WStr(threadInfo->Priority), WStr(ThreadStateToString(threadInfo)), _
                WStr(strEThread), WStr("0x" & WHex(threadInfo->StartAddress)), WStr(strModulePath)
                threadInfo += 1
                i += 1
            Next
        End If
        
        ' 移动到下一个进程（偏移量为0表示结束）
        If procInfo->NextEntryDelta = 0 Then Exit Do
        procInfo = Cast(SYSTEM_PROCESS_INFORMATION Ptr, Cast(Byte Ptr, procInfo) + procInfo->NextEntryDelta)
    Loop
    
    Deallocate buffer
    CommitListViewView lvwKernelThread
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
        'PrintLastError
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
        'PrintLastError
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