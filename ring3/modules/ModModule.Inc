#include Once "win/winnt.bi"

Enum MEMORY_INFORMATION_CLASS
    MemoryBasicInformation
    MemoryWorkingSetList
    MemorySectionName
End Enum

/'Type UNICODE_STRING32
    Length As WORD
    MaximumLength As WORD
    Buffer As DWORD ' 4字节指针
End Type'/

Type PEB_LDR_DATA32 Field = 4
    Length As DWORD
    Initialized As Byte
    SsHandle As DWORD
    InLoadOrderModuleList As LIST_ENTRY32
    InMemoryOrderModuleList As LIST_ENTRY32
    InInitializationOrderModuleList As LIST_ENTRY32
    EntryInProgress As DWORD
    ShutdownInProgress As Byte
    ShutdownThreadId As DWORD
End Type

Type PEB32
    InheritedAddressSpace As Byte    ' +0x00
    ReadImageFileExecOptions As Byte ' +0x01
    BeingDebugged As Byte            ' +0x02
    SpareBool As Byte                 ' +0x03 (4字节对齐)
    Mutant As DWORD                   ' +0x04
    ImageBaseAddress As DWORD         ' +0x08 (关键字段！之前漏掉了)
    Ldr As DWORD                      ' +0x0C
    ProcessParameters As DWORD        ' +0x10 【官方正确偏移！！！】
    Reserved4(104) As Byte
    SessionId As DWORD
End Type

Type LDR_DATA_TABLE_ENTRY32 Field = 4
    InLoadOrderLinks As LIST_ENTRY32
    InMemoryOrderLinks As LIST_ENTRY32
    InInitializationOrderLinks As LIST_ENTRY32
    DllBase As DWORD
    EntryPoint As DWORD
    SizeOfImage As DWORD
    FullDllName As UNICODE_STRING32
    BaseDllName As UNICODE_STRING32
    ' ... 省略其他字段
End Type

Type MODULE_INFORMATION
    szName As Wstring * MAX_PATH ' SafeStringW_NTS ' StringW
    BaseAddr As ULONG64
    dwSize As ULONG64
    szPath As Wstring * MAX_PATH ' SafeStringW_NTS ' StringW
End Type

Type PMEMORY_BASIC_INFORMATION As MEMORY_BASIC_INFORMATION Ptr
Type PVOID As Any Ptr

'Declare Function ZwQueryVirtualMemory Lib "ntdll.dll" Alias "ZwQueryVirtualMemory"(ByVal ProcessHandle As HANDLE, ByVal BaseAddress As PVOID, ByVal MemoryInformationClass As MEMORY_INFORMATION_CLASS,  ByVal MemoryInformation As ULong, ByVal MemoryInformationLength As ULong, ByVal ReturnLength As PULONG) As NTSTATUS
Declare Function RtlNtStatusToDosError Lib "ntdll.dll" Alias "RtlNtStatusToDosError"(ByVal st As NTSTATUS) As ULong
Declare Function ZwUnmapViewOfSection Lib "ntdll.dll" Alias "ZwUnmapViewOfSection"(ByVal hProcess As HANDLE, ByVal HMODULE As HMODULE) As NTSTATUS

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
    
    'Do While True
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
    'Loop

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
        Print "Unload Module 0x" & WHex(hModule) & "Failed!status = " & WHex(status)
    Else
        Print "Success!"
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
    'Print "0"
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
                'Print "mbi.BaseAddress:" & WHex(mbi.BaseAddress) & " MemorySize:" & WHex(MemorySize) & " MemoryType:" & WHex(MemoryType) & " mbi.State:" & WHex(mbi.State)
                If i > UBound(ModuleInformation) Then ReDim Preserve ModuleInformation(i + 10) As MODULE_INFORMATION
                With ModuleInformation(i)
                    .szName = GetNameByPath(szModuleName)
                    .BaseAddr = Cast(ULONG64, mbi.AllocationBase)
                    .dwSize = MemorySize
                    .szPath = szModuleName
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
            .szPath = szModuleName
        End With
        i += 1
    End If
Exit1:
    CloseHandle hProcess
    ReDim Preserve ModuleInformation(i - 1) As MODULE_INFORMATION
    Return i
End Function

' ========== 辅助函数：遍历32位进程模块 ==========
Private Function EnumModules32(ByVal hProcess As HANDLE, ByVal pebAddr As ULONG_PTR, ModuleList() As MODULE_INFORMATION) As Long
    Dim peb As PEB32
    Dim ldrData As PEB_LDR_DATA32
    Dim listEntry As LIST_ENTRY32
    Dim currentNode As ULong
    Dim ldrEntry As LDR_DATA_TABLE_ENTRY32
    Dim unicodeBuffer As WString * 512
    Dim bytesRead As SIZE_T = 0
    Dim moduleIdx As Long = 0
    ReDim ModuleList(50) As MODULE_INFORMATION
    
    ' 读取PEB
    If ReadProcessMemory(hProcess, Cast(Any Ptr, pebAddr), @peb, SizeOf(peb), @bytesRead) = 0 Or bytesRead <> SizeOf(peb) Then
        Print "[-] 读取32位PEB失败"
        Return 0
    End If
    
    ' 读取PEB_LDR_DATA
    If ReadProcessMemory(hProcess, Cast(Any Ptr, Cast(ULONG_PTR, peb.Ldr)), @ldrData, SizeOf(ldrData), @bytesRead) = 0 Or bytesRead <> SizeOf(ldrData) Then
        Print "[-] 读取32位LDR数据失败"
        Return 0
    End If
    
    ' 遍历InMemoryOrderModuleList链表
    Dim firstNode As ULONG_PTR = Cast(ULONG_PTR, ldrData.InMemoryOrderModuleList.Flink)
    CurrentNode = firstNode
    Do
        ' 读取LDR_DATA_TABLE_ENTRY（节点地址偏移调整）
        If ReadProcessMemory(hProcess, Cast(Any Ptr, CurrentNode - OffsetOf(LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks)), @ldrEntry, SizeOf(ldrEntry), @bytesRead) = 0 Then
            Exit Do
        End If
        
        ' 读取模块名（BaseDllName）
        Dim moduleName As WString * MAX_PATH = ""
        If ldrEntry.BaseDllName.Buffer <> 0 And ldrEntry.BaseDllName.Length > 0 Then
            ReadProcessMemory(hProcess, Cast(Any Ptr, Cast(ULONG_PTR, ldrEntry.BaseDllName.Buffer)), @moduleName, ldrEntry.BaseDllName.Length, @bytesRead)
            moduleName[ldrEntry.BaseDllName.Length / 2] = 0
        End If
        
        ' 读取模块路径（FullDllName）
        Dim modulePath As WString * MAX_PATH = ""
        If ldrEntry.FullDllName.Buffer <> 0 And ldrEntry.FullDllName.Length > 0 Then
            ReadProcessMemory(hProcess, Cast(Any Ptr, Cast(ULONG_PTR, ldrEntry.FullDllName.Buffer)), @modulePath, ldrEntry.FullDllName.Length, @bytesRead)
            modulePath[ldrEntry.FullDllName.Length / 2] = 0
        End If
        'Print "moduleIdx:" & moduleIdx & " modulePath:" & modulePath
        ' 动态扩容数组并存储模块信息
        If moduleIdx > UBound(ModuleList) Then ReDim Preserve ModuleList(UBound(ModuleList) + 10) As MODULE_INFORMATION
        
        With ModuleList(moduleIdx)
            .BaseAddr = ldrEntry.DllBase
            .dwSize = ldrEntry.SizeOfImage
            .szName = moduleName
            .szPath = modulePath
        End With
        
        moduleIdx += 1
        
        ' 移动到下一个节点
        ReadProcessMemory(hProcess, Cast(Any Ptr, Cast(ULONG_PTR, CurrentNode)), @listEntry, SizeOf(listEntry), @bytesRead)
        CurrentNode = listEntry.Flink
    Loop While CurrentNode <> firstNode  ' 回到头部时退出
    
    Return moduleIdx - 1
End Function

' ========== 辅助函数：遍历64位进程模块 ==========
Private Function EnumModules64(ByVal hProcess As HANDLE, ByVal pebAddr As ULONG_PTR, ModuleList() As MODULE_INFORMATION) As Long
    Dim peb As PEB64
    Dim ldrData As PEB_LDR_DATA64
    Dim listEntry As LIST_ENTRY64
    Dim currentNode As ULONG_PTR
    Dim ldrEntry As LDR_DATA_TABLE_ENTRY64
    Dim unicodeBuffer As WString * 512
    Dim bytesRead As SIZE_T = 0
    Dim moduleIdx As Long = 0
    ReDim ModuleList(50) As MODULE_INFORMATION
    
    ' 读取PEB
    If ReadProcessMemory(hProcess, Cast(Any Ptr, pebAddr), @peb, SizeOf(peb), @bytesRead) = 0 Or bytesRead <> SizeOf(peb) Then
        Print "[-] 读取64位PEB失败"
        Return 0
    End If
    
    ' 读取PEB_LDR_DATA
    If ReadProcessMemory(hProcess, peb.Ldr, @ldrData, SizeOf(ldrData), @bytesRead) = 0 Or bytesRead <> SizeOf(ldrData) Then
        Print "[-] 读取64位LDR数据失败"
        Return 0
    End If
    
    ' 遍历InMemoryOrderModuleList链表
    Dim firstNode As ULONG_PTR = Cast(ULONG_PTR, ldrData.InMemoryOrderModuleList.Flink)
    CurrentNode = firstNode
    Do
        ' 读取LDR_DATA_TABLE_ENTRY（节点地址偏移调整）
        If ReadProcessMemory(hProcess, Cast(Any Ptr, CurrentNode - OffsetOf(LDR_DATA_TABLE_ENTRY64, InMemoryOrderLinks)), @ldrEntry, SizeOf(ldrEntry), @bytesRead) = 0 Then
            Exit Do
        End If
        
        ' 读取模块名（BaseDllName）
        Dim moduleName As WString * MAX_PATH = ""
        If ldrEntry.BaseDllName.Buffer <> 0 And ldrEntry.BaseDllName.Length > 0 Then
            ReadProcessMemory(hProcess, ldrEntry.BaseDllName.Buffer, @moduleName, ldrEntry.BaseDllName.Length, @bytesRead)
            moduleName[ldrEntry.BaseDllName.Length / 2] = 0
        End If
        
        ' 读取模块路径（FullDllName）
        Dim modulePath As WString * MAX_PATH = ""
        If ldrEntry.FullDllName.Buffer <> 0 And ldrEntry.FullDllName.Length > 0 Then
            ReadProcessMemory(hProcess, ldrEntry.FullDllName.Buffer, @modulePath, ldrEntry.FullDllName.Length, @bytesRead)
            modulePath[ldrEntry.FullDllName.Length / 2] = 0
        End If
        'Print "moduleName:" & moduleName & " modulePath:" & modulePath
        ' 动态扩容数组并存储模块信息
        If moduleIdx > UBound(ModuleList) Then ReDim Preserve ModuleList(UBound(ModuleList) + 10) As MODULE_INFORMATION
        
        With ModuleList(moduleIdx)
            .BaseAddr = ldrEntry.DllBase
            .dwSize = ldrEntry.SizeOfImage
            .szName = moduleName
            .szPath = modulePath
        End With
        moduleIdx += 1
        
        ' 移动到下一个节点
        ReadProcessMemory(hProcess, Cast(Any Ptr, CurrentNode), @listEntry, SizeOf(listEntry), @bytesRead)
        currentNode = listEntry.Flink
    Loop While CurrentNode <> firstNode  ' 回到头部时退出
    
    Return moduleIdx - 1
End Function

Function EnumModuleListEx(ByVal hProcess As HANDLE, ModuleList() As MODULE_INFORMATION) As Long
    Dim isWow64 As BOOL = False
    Dim bytesRead As SIZE_T = 0
    Dim moduleCount As Long = 0
    Dim peb_32 As ULONG_PTR
    Dim pbi64 As PROCESS_BASIC_INFORMATION
    Dim retLen As DWORD = 0
    Dim st As NTSTATUS
    
    ' 检查是否为Wow64进程
    If IsProcessWow64_Ex(hProcess) Then
        ' ========== Wow64进程：必须读取两个PEB ==========
        Dim list32() As MODULE_INFORMATION
        Dim list64() As MODULE_INFORMATION
        Dim count32 As Long = 0
        Dim count64 As Long = 0
        
        ' 获取32位PEB地址
        st = ZwQueryInformationProcess(hProcess, ProcessWow64Information, @peb_32, SizeOf(peb_32), retLen)
        If Not NT_SUCCESS(st) Then
            Print "[-] 读取32位PEB地址失败, st=" & WHex(st)
            Return -1
        End If
        
        ' 获取64位PEB地址（关键！）
        st = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, @pbi64, SizeOf(pbi64), retLen)
        If Not NT_SUCCESS(st) Then
            Print "[-] 读取64位PEB地址失败, st=" & WHex(st)
            Return -1
        End If
        
        ' 步骤3：遍历32位模块链表
        count32 = EnumModules32(hProcess, peb_32, list32())
        
        ' 步骤4：遍历64位模块链表（关键！）
        count64 = EnumModules64(hProcess, Cast(ULONG_PTR, pbi64.PebBaseAddress), list64())
        
        ' 步骤5：合并列表（按你要求的顺序）
        Dim totalCount As Long = count32 + count64
        
        ' 重新分配最终数组
        ReDim ModuleList(totalCount - 1) As MODULE_INFORMATION
        
        Dim idx As Long = 0
        
        ' 2. 添加所有64位模块
        For i As Long = 0 To count64 - 1
            Print "list64(i):" & list64(i).szName
            ModuleList(idx) = list64(i)
            idx += 1
        Next
        
        ' 3. 添加剩余的32位模块（跳过第1个）
        For i As Long = 1 To count32 - 1
            ModuleList(idx) = list32(i)
            idx += 1
        Next
        
        Return idx
    Else
        ' ========== 原生64位进程 ==========
        ' 读取64位进程基本信息
        st = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, @pbi64, SizeOf(pbi64), retLen)
        If Not NT_SUCCESS(st) Then
            Print "[-] 读取64位进程基本信息失败, st=" & WHex(st)
            Return -1
        End If
        
        ' 步骤3：遍历64位模块链表
        moduleCount = EnumModules64(hProcess, Cast(ULONG_PTR, pbi64.PebBaseAddress), ModuleList())
        
        Return moduleCount
    End If
End Function

Public Function GetModuleList(ByVal ProcessId As DWORD, lvwModule As Class_ListView) As Integer
    Dim i As Long, hProcess As HANDLE, ModuleCount As Long, ItemIndex As Long
    Dim ModuleInformation1() As MODULE_INFORMATION
    
    InitListViewData lvwModule

    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(ProcessId)
    Else
        hProcess = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, False, ProcessId)
    End If
    If hProcess = NULL Then Return 0
    
    ModuleCount = EnumModuleListEx(hProcess, ModuleInformation1())
    If ModuleCount >= 0 Then
        For i As Integer = 0 To ModuleCount - 1
            ItemIndex = AddItemColListEx(lvwModule, 6,,, WStr(i + 1), WStr(ModuleInformation1(i).szName), WStr("0x" & WHex(ModuleInformation1(i).BaseAddr)), _
            WStr("0x" & WHex(ModuleInformation1(i).dwSize)), WStr(ModuleInformation1(i).szPath), WStr(GetFileCompany(ModuleInformation1(i).szPath)))
            VerifyFileSignAsync FrmListView.hWnd, FrmListView.ListView.hWnd, "ListView", ItemIndex, ModuleInformation1(i).szPath
            'SetItemColor lvwModule, ItemIndex, FB_Red, FB_White ' FB_Black, FB_White
        Next
    End If
    ' 第二种枚举方法
    Dim ModuleInformation2() As MODULE_INFORMATION
    ReDim ModuleInformation2(100) As MODULE_INFORMATION
    ModuleCount = GetModuleList1(ProcessId, ModuleInformation2())
    For j As Integer = 0 To ModuleCount - 1
        If (Not IsInArray(ModuleInformation2(j).BaseAddr, ModuleInformation1())) Then ' 找到隐藏模块
            Print "隐藏模块:" & ModuleInformation2(j).szPath
            ItemIndex = AddItemColListEx(lvwModule, 6, FB_Red, , WStr(lvwModule.ItemCount + 1), WStr(ModuleInformation2(j).szName), WStr("0x" & WHex(ModuleInformation2(j).BaseAddr)), _
            WStr("0x" & WHex(ModuleInformation2(j).dwSize)), WStr(ModuleInformation2(j).szPath), WStr(GetFileCompany(ModuleInformation1(i).szPath)))
            VerifyFileSignAsync FrmListView.hWnd, FrmListView.ListView.hWnd, "ListView", ItemIndex, ModuleInformation2(i).szPath
            'SetItemColor lvwModule, lvwModule.ItemCount, FB_Red, FB_White
            i += 1
        End If
    Next
    CommitListViewView lvwModule
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

Function GetModuleOfAddress(hProc As HANDLE, addr As Any Ptr) As StringW
    Dim sz As ZString * MAX_PATH, wsz As WString * MAX_PATH, result As WString * MAX_PATH
    GetMappedFileNameA(hProc, addr, @sz, MAX_PATH)
    Print sz
    wsz = sz
    NtPathToDosPathW @wsz, @result
    Print result
    Return result
End Function

Function GetModuleBaseByPath(ByVal hProcess As HANDLE, ByVal szModulePath As String) As PVOID
    Dim cbNeeded As DWORD
    Dim nModuleCount As Long
    Dim szModPath As WString * 4096
    Dim dwPathLen As DWORD
    Dim hModArray As HMODULE Ptr  ' 动态数组指针（64位核心！）

    ' ==============================================
    ' 【关键1】第一次调用：仅获取所需缓冲区大小（不传数组）
    ' ==============================================
    If EnumProcessModulesEx(hProcess, NULL, 0, @cbNeeded, LIST_MODULES_64BIT) = 0 Then
        Print "[错误] 获取模块大小失败："; WinErrorMsg(GetLastError)
        Return NULL
    End If

    ' ==============================================
    ' 【关键2】动态分配内存（适配任意数量模块，告别静态数组限制）
    ' ==============================================
    hModArray = Allocate(cbNeeded)
    If hModArray = NULL Then
        Print "[错误] 内存分配失败"
        Return NULL
    End If

    ' ==============================================
    ' 【关键3】第二次调用：真正枚举所有64位模块
    ' ==============================================
    If EnumProcessModulesEx(hProcess, hModArray, cbNeeded, @cbNeeded, LIST_MODULES_64BIT) = 0 Then
        Print "[错误] 枚举模块失败："; WinErrorMsg(GetLastError)
        DeAllocate(hModArray)
        Return NULL
    End If

    ' 计算正确的模块数量（64位精准计算）
    nModuleCount = cbNeeded / SizeOf(HMODULE)
    If nModuleCount = 0 Then
        Deallocate(hModArray)
        Return 0
    End If

    'Print "成功枚举模块总数："; nModuleCount
    ' ==============================================
    ' 遍历所有模块（动态数组索引，无重复）
    ' ==============================================
    For i As Long = 0 To nModuleCount - 1
        dwPathLen = GetModuleFileNameExW(hProcess, hModArray[i], szModPath, 4096)

        If dwPathLen > 0 Then
            'Print "模块路径："; szModPath
            ' 路径匹配
            If UCase(szModulePath) = UCase(szModPath) Then
                Dim pBase As PVOID = hModArray[i]
                DeAllocate(hModArray)
                Return pBase
            End If
        End If
    Next

    ' 释放动态内存
    DeAllocate(hModArray)
    Print "[提示] 未找到匹配模块"
    Return 0
End Function

' ==============================================
' 核心函数：根据模块名、函数名、基址 获取 函数虚拟地址(VA)
' 参数：
'   szModuleName  模块名(如 ntoskrnl.exe)
'   szFuncName    导出函数名(如 IopInvalidDeviceRequest)
'   ModuleBase    模块加载基址(内核/R3基址)
' 返回：函数虚拟地址(失败返回0)
' ==============================================
Function GetProcVA( _
    ByVal szModuleName As String, _
    ByVal szFuncName As String, _
    ByVal ModuleBase As ULONG_PTR _
) As ULONG_PTR
    Dim As HANDLE hFile, hMapping
    Dim As UByte Ptr pMapBase = 0
    Dim As ULONG_PTR FuncVA = 0
    Dim As String szModulePath
    
    Dim As IMAGE_DOS_HEADER Ptr pDos
    Dim As IMAGE_NT_HEADERS64 Ptr pNt
    Dim As IMAGE_DATA_DIRECTORY Ptr pExportDir
    Dim As IMAGE_EXPORT_DIRECTORY Ptr pExport
    
    Dim As ULong Ptr pNames
    Dim As UShort Ptr pOrdinals
    Dim As ULong Ptr pFuncs
    
    ' 1. 拼接系统模块完整路径（System32目录）
    Dim As ZString * MAX_PATH sysDir
    GetSystemDirectoryA(sysDir, MAX_PATH)
    szModulePath = sysDir & "\" & szModuleName

    ' 2. 打开模块文件
    hFile = CreateFileA(_
        szModulePath, _
        GENERIC_READ, _
        FILE_SHARE_READ Or FILE_SHARE_WRITE, _
        NULL, _
        OPEN_EXISTING, _
        FILE_ATTRIBUTE_NORMAL, _
        NULL _
    )
    If hFile = INVALID_HANDLE_VALUE Then Return 0

    ' 3. 创建文件映射
    hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL)
    If hMapping = NULL Then
        CloseHandle(hFile)
        Return 0
    End If

    ' 4. 映射文件到内存
    pMapBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0)
    If pMapBase = NULL Then
        CloseHandle(hMapping)
        CloseHandle(hFile)
        Return 0
    End If

    ' 5. 解析PE头
    pDos = Cast(IMAGE_DOS_HEADER Ptr, pMapBase)
    If pDos->e_magic <> IMAGE_DOS_SIGNATURE Then Goto Cleanup ' 无效DOS头

    pNt = Cast(IMAGE_NT_HEADERS64 Ptr, pMapBase + pDos->e_lfanew)
    If pNt->Signature <> IMAGE_NT_SIGNATURE Then Goto Cleanup ' 无效NT头

    ' 6. 获取导出表
    pExportDir = @pNt->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)
    If pExportDir->VirtualAddress = 0 Then Goto Cleanup ' 无导出表

    pExport = Cast(IMAGE_EXPORT_DIRECTORY Ptr, pMapBase + pExportDir->VirtualAddress)
    
    ' 7. 遍历导出表 查找函数名
    pNames     = Cast(ULong Ptr, pMapBase + pExport->AddressOfNames)
    pOrdinals   = Cast(UShort Ptr, pMapBase + pExport->AddressOfNameOrdinals)
    pFuncs       = Cast(ULong Ptr, pMapBase + pExport->AddressOfFunctions)

    For i As ULong = 0 To pExport->NumberOfNames - 1
        Dim As ZString Ptr pFuncName = Cast(ZString Ptr, pMapBase + pNames[i])
        Print *pFuncName
        If *pFuncName = szFuncName Then
            ' 找到函数 → 获取RVA → 计算VA
            Dim As ULong FuncRVA = pFuncs[pOrdinals[i]]
            FuncVA = ModuleBase + FuncRVA
            Exit For
        End If
    Next

Cleanup:
    ' 释放资源
    UnmapViewOfFile(pMapBase)
    CloseHandle(hMapping)
    CloseHandle(hFile)

    Return FuncVA
End Function