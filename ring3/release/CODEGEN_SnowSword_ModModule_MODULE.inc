'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/winnt.bi" '已经搬到定义文件处

' Enum MEMORY_INFORMATION_CLASS '已经搬到定义文件处
'     MemoryBasicInformation '已经搬到定义文件处
'     MemoryWorkingSetList '已经搬到定义文件处
'     MemorySectionName '已经搬到定义文件处
' End Enum '已经搬到定义文件处

' Type PMEMORY_BASIC_INFORMATION As MEMORY_BASIC_INFORMATION Ptr '已经搬到定义文件处
' Type PVOID As Any Ptr '已经搬到定义文件处

'Declare Function ZwQueryVirtualMemory Lib "ntdll.dll"(ByVal ProcessHandle As HANDLE, ByVal BaseAddress As PVOID, ByVal MemoryInformationClass As MEMORY_INFORMATION_CLASS,??ByVal MemoryInformation As PVOID, ByVal MemoryInformationLength As ULong, ByVal ReturnLength As PULONG) As NTSTATUS
Extern "C"
Declare Function RtlNtStatusToDosError Lib "ntdll.dll"(ByVal Status As NTSTATUS) As ULong
End Extern

Public Function MyPrintA(ByVal TheStr As CWSTR) As BOOLEAN
    If Len(TheStr) = 0 Then
        Return True
    Else
        PrintA TheStr
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
        PrintA errorStr
        ' 返回 False 表示有错误发生
        MyPrintLog = False
    Else
        ' 没有错误发生，返回 True
        MyPrintLog = True
    End If
End Function

' #define PrintLog(nt_Status, Reason) MyPrintLog(nt_Status, Reason, __FUNCTION__, __LINE__) '已经搬到定义文件处
' #define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")" '已经搬到定义文件处

Public Sub PrintProcessModules()
    Dim Addr As PULONG, status As NTSTATUS, hProcess As HANDLE = Cast(HANDLE, -1)
    Dim Information As MEMORY_BASIC_INFORMATION, wName As WString * 260
    Dim library As Any Ptr = DyLibLoad("ntdll.dll")
    Dim ZwQueryVirtualMemory As Function(ByVal As HANDLE, ByVal As PVOID, ByVal As MEMORY_INFORMATION_CLASS,CH_C2A0C2A0_ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS
    ZwQueryVirtualMemory = DyLibSymbol(library, "ZwQueryVirtualMemory") 
    For Addr = 0 To &H80000000 Step &H1000
        status = ZwQueryVirtualMemory(hProcess, Addr, MemoryBasicInformation, @Information, SizeOf(Information), NULL)
        If (NT_SUCCESS(status)) Then
            If (Information.Type = MEM_IMAGE) Then
                If (Information.AllocationBase = Addr) Then
                    status = ZwQueryVirtualMemory(hProcess, Addr, MemorySectionName, @wName, 600, NULL)
                    If NT_SUCCESS(status) Then
                        AfxMsg wName
                    End If
                End If
            End If
        End If
    Next
    DyLibFree library
End Sub

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

Private Function IsInArray(ByVal ModulePath As String, Array() As MODULEENTRY32) As BOOLEAN
    Dim i As Integer
    For i = LBound(Array) To UBound(Array)
        If (Array(i).szExePath = ModulePath) Then Return True
    Next
    Return False
End Function

Public Sub UnloadModule(ByVal dwPID As DWORD, ByVal ModuleName As String)
    Dim hProcess As HANDLE
    hProcess = OpenProcess(PROCESS_VM_OPERATION, False, dwPID)
    If hProcess = 0 Then
        PrintLastError
        Exit Sub
    End If
        /'If ZwUnmapViewOfSection(hProcess, GetModuleHandle(ModuleName)) <> 0 Then
            MyMsgBox "Unload Module Failed!" & ModuleName
        Else
            MyMsgBox "Success!"
        End If
    ZwClose (hProcess)'/
End Sub

Public Sub GetModuleList(ByVal ProcessId As DWORD, lvwModule As Class_ListView)
    Dim i As Long
    Dim Mode As MODULEENTRY32
    Dim hSnapshot As HANDLE
    Dim ModuleList1() As MODULEENTRY32
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE Or TH32CS_SNAPMODULE32,ProcessId) 
    If hSnapshot > 0 Then
        Mode.dwSize = SizeOf(MODULEENTRY32) 
        If Module32First(hSnapshot,@Mode)  Then
            Do
                lvwModule.AddItem WStr(i + 1)
                lvwModule.SetItemText i,1,WStr(Mode.szModule)
                'lvwModule.SetItemText(i,2,"0x" & WStr(Hex(GetModuleEntryPoint(ProcessId,Mode.HMODULE))))
                lvwModule.SetItemText i,2,"0x" & WStr(Hex(Mode.modBaseAddr))
                lvwModule.SetItemText i,3,"0x" & WStr(Hex(Mode.modBaseSize))
                lvwModule.SetItemText i,4,WStr(Mode.szExePath)
                'MoSize = Mode.modBaseSize
                'ModuleList1(i) = Mode
                i+=1
            Loop Until Module32Next(hSnapshot,@Mode) = 0
        End If
        CloseHandle hSnapshot   '关闭模块快照句柄
    End If
    Exit Sub
    '-----------------------------------------------------------------------
    Dim pPEB As PPEB64 = GetPEB(ProcessId) '获取 PEB 指针
    Dim ldrData As PPEB64_LDR_DATA, currentEntry As PLIST_ENTRY
    If (pPEB = NULL) Then Goto Exit1:
    ldrData = pPEB->Ldr

    currentEntry = ldrData->InMemoryOrderModuleList.Flink
    Do While (currentEntry <> @(ldrData->InMemoryOrderModuleList))
        Dim entry As PLDR_DATA_TABLE_ENTRY = Cast(PLDR_DATA_TABLE_ENTRY, currentEntry)
        AfxMsg "Module:" & entry->FullDllName.Buffer
        currentEntry = currentEntry->Flink
    Loop
    Exit1:
End Sub