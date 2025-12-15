#include Once "win/winnt.bi"

Enum MEMORY_INFORMATION_CLASS
    MemoryBasicInformation
    MemoryWorkingSetList
    MemorySectionName
End Enum

Type MODULE_INFORMATION
    szName As StringW
    BaseAddr As ULONG64
    dwSize As ULONG64
    szExePath As StringW
End Type

Type PMEMORY_BASIC_INFORMATION As MEMORY_BASIC_INFORMATION Ptr
Type PVOID As Any Ptr

'Declare Function ZwQueryVirtualMemory Lib "ntdll.dll"(ByVal ProcessHandle As HANDLE, ByVal BaseAddress As PVOID, ByVal MemoryInformationClass As MEMORY_INFORMATION_CLASS,  ByVal MemoryInformation As PVOID, ByVal MemoryInformationLength As ULong, ByVal ReturnLength As PULONG) As NTSTATUS
Declare Function RtlNtStatusToDosError Lib "ntdll.dll" Alias "RtlNtStatusToDosError"(ByVal st As NTSTATUS) As ULong
Declare Function ZwUnmapViewOfSection Lib "ntdll.dll" Alias "ZwUnmapViewOfSection"(ByVal hProcess As HANDLE, ByVal HMODULE As HMODULE) As NTSTATUS

Public Function MyPrintA(ByVal TheStr As CWSTR) As BOOLEAN
    If Len(TheStr) = 0 Then
        Return True
    Else
        Print TheStr
        Return False
    End If
End Function

Public Function MyPrintLog(ByVal nt_Status As Long, ByVal Reason As String, ByVal FunctionName As String, ByVal LineNumber As Long) As Boolean
    Dim errorStr As String
    Dim winError As Long = GetLastError
    ' 检查 nt_Status 或 GetLastError 是否有错误
    If (nt_Status <> 0 Or winError <> 0 Or Len(Reason) <> 0) Then
        ' 构造错误信息
        errorStr = "Error In Function [" & FunctionName & "] At Line " & LineNumber & ": "
        ' 根据 nt_Status 和 GetLastError 的值生成详细的错误信息
        If Len(Reason) <> 0 Then
            errorStr = errorStr & Reason
        ElseIf nt_Status <> 0 Then
            winError = RtlNtStatusToDosError(nt_Status)
            errorStr = errorStr & WinErrorMsg(winError) & " (NTSTATUS = 0x" & Hex(nt_Status) & ")"
        ElseIf win32Error <> 0 Then
            errorStr = errorStr & WinErrorMsg(winError) & " (GetLastError = " & Str(winError) & ")"
        Else
            errorStr = errorStr & "Unknown"
        End If
        ' 打印错误信息
        Print errorStr
        ' 返回 False 表示有错误发生
        MyPrintLog = False
    Else
        ' 没有错误发生，返回 True
        MyPrintLog = True
    End If
End Function

#define PrintLog(nt_Status, Reason) MyPrintLog(nt_Status, Reason, __FUNCTION__, __LINE__)
#define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")"

/'Public Sub PrintProcessModules()
    Dim Addr As PVOID, st As NTSTATUS, hProcess As HANDLE = Cast(HANDLE, -1), ReturnLength As SIZE_T = 0
    Dim Information As MEMORY_BASIC_INFORMATION, wName As StringW
    Dim library As Any Ptr = DyLibLoad("ntdll.dll")
    Dim ZwQueryVirtualMemory As Function(ByVal As HANDLE, ByVal As PVOID, ByVal As MEMORY_INFORMATION_CLASS,  ByVal As PVOID, ByVal As SIZE_T, ByVal As SIZE_T Ptr) As NTSTATUS
    ZwQueryVirtualMemory = DyLibSymbol(library, "ZwQueryVirtualMemory") 
    For Addr = &H7FF0000000000000 To &H7FFFFFFFFFFFFFFF Step &H1000 '0
        st = ZwQueryVirtualMemory(hProcess, Addr, MemoryBasicInformation, @Information, SizeOf(MEMORY_BASIC_INFORMATION), @ReturnLength)
        If (NT_SUCCESS(st)) Then
            Print "Addr:0x" & Hex(Addr)
            If (Information.Type = MEM_IMAGE) Then
                Print "Addr1:0x" & Hex(Addr)
                If (Information.AllocationBase = Addr) Then
                    Print "ModuleAddr:0x" & Hex(Addr)
                    st = ZwQueryVirtualMemory(hProcess, Addr, MemorySectionName, wName, MAX_PATH, NULL)
                    If NT_SUCCESS(st) Then
                        Print wName
                    Else
                        Print "MemorySectionName:" & Hex(st)
                    End If
                End If
            End If
        Else
            Print Hex(st) & " " & ReturnLength & " " & SizeOf(Information)
        End If
        FF_DoEvents
    Next
    DyLibFree library
End Sub '/

Private Function GetModuleEntryPoint(ByVal ProcessId As DWORD,ByVal hModule As HMODULE) As DWORD
    Dim hProcess As HANDLE,pModuleInfo As LPMODULEINFO,EntryPoint As DWORD
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ,False,ProcessId)
    GetModuleInformation hProcess,hModule,pModuleInfo,SizeOf(MODULEINFO)
    'MsgBox(WinErrorMsg(GetLastError))
    CloseHandle hProcess
    'Return CULng( * (pModuleInfo->EntryPoint))
    memcpy @EntryPoint,pModuleInfo->EntryPoint,SizeOf(DWORD)
    Return EntryPoint
End Function

Private Function IsInArray(ByVal ModuleBase As ULONG64, Array() As MODULE_INFORMATION) As Boolean
    Dim i As Integer
    For i = LBound(Array) To UBound(Array)
        If (Array(i).BaseAddr = ModuleBase) Then Return True
    Next
    Return False
End Function

Public Function UnloadModule(ByVal dwPID As DWORD, ByVal hModule As HMODULE) As Boolean
    Dim hProcess As HANDLE
    Dim pFreeLibrary As LPTHREAD_START_ROUTINE
    Dim hThread As HANDLE

    ' 打开目标进程
    hProcess = OpenProcess(PROCESS_VM_OPERATION Or PROCESS_VM_WRITE Or PROCESS_QUERY_INFORMATION, False, dwPID)
    If hProcess = NULL Then
        Print "[UnloadModule]OpenProcess:" & WinErrorMsg(GetLastError) & " " & GetLastError
        Return False
    End If

    ' 获取 FreeLibrary 函数地址
    pFreeLibrary = Cast(LPTHREAD_START_ROUTINE, GetProcAddress(GetModuleHandleW("kernel32.dll"), "FreeLibrary"))
    
    Do While True
        ' 创建远程线程并调用 FreeLibrary
        hThread = CreateRemoteThread(hProcess, 0, 0, pFreeLibrary, hModule, 0, 0)
        If hThread = NULL Then
            CloseHandle hProcess
            Print "[UnloadModule]CreateRemoteThread:" & WinErrorMsg(GetLastError) & " " & GetLastError
            Return False
        End If
        
        /'Dim lpExitCode As DWORD = STILL_ALIVE
        Do While lpExitCode = STILL_ALIVE
            GetExitCodeThread hThread, @lpExitCode
            FF_DoEvents
        Loop
        If lpExitCode <> 0 Then Exit Do'/
    Loop

    ' 等待线程完成
    'WaitForSingleObject hThread, INFINITE

    ' 关闭句柄
    CloseHandle hThread
    CloseHandle hProcess

    Return True
End Function

Function ForceUnloadModule(ByVal dwPID As DWORD, ByVal hModule As HMODULE) As Boolean
    Dim hProcess As HANDLE, status As NTSTATUS
    status = ZwUnmapViewOfSection(hProcess, hModule)
    If Not(NT_SUCCESS(status)) Then
        AfxMsg "Unload Module 0x" & WHex(hModule) & "Failed!status = " & WHex(status)
    Else
        AfxMsg "Success!"
    End If
    ZwClose hProcess
    Return True
End Function

Private Function GetModuleList1(ByVal dwProcessId As DWORD, ModuleInformation() As MODULE_INFORMATION) As DWORD
    Dim mbi As MEMORY_BASIC_INFORMATION
    Dim Address As PVOID
    Dim AddressEnd As Long
    Dim MemorySize As Long
    Dim MemoryProtect As Long
    Dim MemoryType As Long
    Dim i As DWORD = 0
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, dwProcessId)
    If (hProcess <= 0) Then
        Print "OpenProcess " & dwProcessId & "Failed!"
        Return 0
    End If
    Print "0"
    Do While VirtualQueryEx(hProcess, Address, @mbi, Len(mbi)) <> 0
        FF_DoEvents
        ' 检查是否为 0x1000 大小的块
        /'If mbi.RegionSize = &H1000 Then
            ' 跳过 0x1000 大小的块
            Address = Address + mbi.RegionSize
            Continue Do
        End If'/

        ' 检查是否需要合并属性相同的块
        If MemoryProtect = mbi.Protect And MemoryType = mbi.Type Then
            ' 合并块
            MemorySize = MemorySize + mbi.RegionSize
        Else
            ' 添加新的块到 ListView
            If (MemorySize > 0) AndAlso (MemoryType = MEM_IMAGE) Then ' AndAlso (mbi.AllocationBase = Address) Then
                If IsInArray(Cast(ULONG64, mbi.AllocationBase), ModuleInformation()) Then Goto ContinueDo:
                Dim szModuleName As WString * MAX_PATH
                GetModuleByAddress dwProcessId, mbi.AllocationBase, @szModuleName
                'Print szModuleName
                If i > UBound(ModuleInformation) Then ReDim Preserve ModuleInformation(i + 10) As MODULE_INFORMATION
                With ModuleInformation(i)
                    .szName = GetNameByPath(szModuleName)
                    .BaseAddr = Cast(ULONG64, mbi.AllocationBase)
                    .dwSize = MemorySize
                    .szExePath = szModuleName
                End With
                i += 1
            End If
ContinueDo:
            ' 更新当前块信息
            Address = mbi.BaseAddress
            MemorySize = mbi.RegionSize
            MemoryProtect = mbi.Protect
            MemoryType = mbi.Type
        End If

        ' 更新地址
        Address = Address + mbi.RegionSize
    Loop

    ' 添加最后一个块
    If (MemorySize > 0) AndAlso (MemoryType = MEM_IMAGE) Then ' AndAlso (mbi.AllocationBase = Address) Then
        If IsInArray(Cast(ULONG64, mbi.AllocationBase), ModuleInformation()) Then Goto Exit1
        Dim szModuleName As WString * MAX_PATH
        GetModuleByAddress dwProcessId, mbi.AllocationBase, @szModuleName
        'Print szModuleName
        If i > UBound(ModuleInformation) Then ReDim Preserve ModuleInformation(i + 10) As MODULE_INFORMATION
        With ModuleInformation(i)
            .szName = GetNameByPath(szModuleName)
            .BaseAddr = Cast(ULONG64, mbi.AllocationBase)
            .dwSize = MemorySize
            .szExePath = szModuleName
        End With
        i += 1
    End If
Exit1:
    CloseHandle hProcess
    ReDim Preserve ModuleInformation(i - 1) As MODULE_INFORMATION
    Return i
End Function

Public Function GetModuleList(ByVal ProcessId As DWORD, lvwModule As Class_ListView) As Integer
    Dim i As Long
    Dim Mode As MODULEENTRY32
    Dim hSnapshot As HANDLE
    Dim ModuleInformation() As MODULE_INFORMATION
    ReDim ModuleInformation(100) As MODULE_INFORMATION
    InitDataNode lvwModule
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessId)
    If hSnapshot > 0 Then
        Mode.dwSize = SizeOf(MODULEENTRY32)
        If Module32First(hSnapshot, @Mode) Then
            Do
                With lvwModule
                    SetItemColor lvwModule, i, FB_Black, FB_White
                    .AddItem WStr(i + 1)
                    .SetItemText i, 1, Mode.szModule
                    'lvwModule.SetItemText(i,2,"0x" & WStr(Hex(GetModuleEntryPoint(ProcessId,Mode.HMODULE))))
                    .SetItemText i, 2, "0x" & WHex(Mode.modBaseAddr)
                    .SetItemText i, 3, "0x" & WHex(Mode.modBaseSize)
                    .SetItemText i, 4, Mode.szExePath
                    'MoSize = Mode.modBaseSize
                    'ModuleList1(i) = Mode
                    If i > UBound(ModuleInformation) Then ReDim Preserve ModuleInformation(i + 10) As MODULE_INFORMATION
                    ModuleInformation(i).BaseAddr = Cast(ULONG64, Mode.modBaseAddr)
                    ModuleInformation(i).dwSize = Mode.modBaseSize
                    ModuleInformation(i).szExePath = Mode.szExePath
                    ModuleInformation(i).szName = Mode.szModule
                End With
                i += 1
            Loop Until Module32Next(hSnapshot, @Mode) = 0
        End If
        CloseHandle hSnapshot '关闭模块快照句柄
    Else
        Print "[GetModuleList]CreateToolhelp32Snapshot:" & WinErrorMsg(GetLastError) & GetLastError
    End If
    ' 第二种枚举方法
    Dim ModuleInformation1() As MODULE_INFORMATION
    ReDim ModuleInformation1(100) As MODULE_INFORMATION
    Dim ModuleCount As DWORD = GetModuleList1(ProcessId, ModuleInformation1())
    For j As Integer = 0 To ModuleCount - 1
        If (Not IsInArray(ModuleInformation1(j).BaseAddr, ModuleInformation())) Then ' 找到隐藏模块
            Print ModuleInformation1(j).szExePath
            If Not SetItemColor(lvwModule, lvwModule.ItemCount, FB_Red, FB_White) Then Print "改色失败!"
            lvwModule.AddItemColList,, WStr(i + 1), ModuleInformation1(j).szName, "0x" & WHex(ModuleInformation1(j).BaseAddr), "0x" & WHex(ModuleInformation1(j).dwSize), ModuleInformation1(j).szExePath
            i += 1
        End If
    Next
    Return i
End Function

' 用户态下检查地址是否有效（基于SEH）
Function IsAddressValid(ByVal addr As Any Ptr) As Boolean
    Dim c As UByte
    On Error Goto Err1  ' 捕获访问异常
    c = Peek(UByte, addr) ' 尝试读取1字节
    Return True
    Err1:
    Return False
End Function

' 用户态特征码搜索函数
Function SearchSpecialCode(ByVal pSearchBeginAddr As Any Ptr, ByVal ulSearchLength As ULong, _
ByVal pSpecialCode As UByte Ptr, ByVal ulSpecialCodeLength As ULong) As Any Ptr
    Dim pDestAddr As Any Ptr = 0
    Dim pBeginAddr As UByte Ptr = pSearchBeginAddr
    ' 优化结束地址：避免超出范围（剩余长度不足特征码时无需检查）
    Dim pEndAddr As UByte Ptr = pBeginAddr + ulSearchLength - ulSpecialCodeLength
    Dim i As UByte Ptr
    Dim j As ULong
    
    For i = pBeginAddr To pEndAddr
        j = 0
        Do While j < ulSpecialCodeLength
            ' 用用户态方法检查地址有效性
            If Not IsAddressValid(i + j) Then
                Exit Do
            End If
            
            ' 比较特征码字节
            If *(i + j) <> pSpecialCode[j] Then Exit Do
            
            j += 1
        Loop
        
        ' 完全匹配时返回地址
        If j >= ulSpecialCodeLength Then
            pDestAddr = i
            Exit For
        End If
    Next i
    
    Return pDestAddr
End Function