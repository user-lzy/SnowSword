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
        MyLog.PrintWin32Error "UnloadModule", "OpenProcess", "dwPID=" & dwPID & ",hModule=0x" & WHex(hModule)
        Return False
    End If

    ' 获取 FreeLibrary 函数地址
    pFreeLibrary = Cast(LPTHREAD_START_ROUTINE, GetProcAddress(GetModuleHandleW("kernel32.dll"), "FreeLibrary"))
    
    'Do While True
        ' 创建远程线程并调用 FreeLibrary
        hThread = CreateRemoteThread(hProcess, 0, 0, pFreeLibrary, hModule, 0, 0)
        If hThread = NULL Then
            CloseHandle hProcess
            MyLog.PrintWin32Error "UnloadModule", "CreateRemoteThread", "dwPID=" & dwPID & ",hModule=0x" & WHex(hModule)
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
        MyLog.PrintNtError "UnloadModule", "ZwUnmapViewOfSection", status, "dwPID=" & dwPID & ",hModule=0x" & WHex(hModule)
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
        MyLog.PrintWin32Error "GetModuleList1", "OpenProcess", "dwProcessId=" & dwProcessId
        Return 0
    End If
    
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

Private Function EnumModules32(ByVal hProcess As HANDLE, _
                               ByVal pebAddr As ULONG_PTR, _
                               ModuleList() As MODULE_INFORMATION) As Long

    Dim peb As PEB32
    Dim ldrData As PEB_LDR_DATA32
    Dim listEntry As LIST_ENTRY32
    Dim currentNode As ULONG_PTR
    Dim listHead As ULONG_PTR
    Dim ldrEntry As LDR_DATA_TABLE_ENTRY32

    Dim bytesRead As SIZE_T = 0
    Dim moduleIdx As Long = 0
    Dim capacity As Long = 50

    ReDim ModuleList(capacity - 1) As MODULE_INFORMATION

    '===========================================================
    ' 1. 读取 PEB32
    '===========================================================
    If ReadProcessMemory( _
        hProcess, _
        Cast(Any Ptr, pebAddr), _
        @peb, _
        SizeOf(peb), _
        @bytesRead) = 0 OrElse bytesRead <> SizeOf(peb) Then

        MyLog.PrintWin32Error "EnumModules32", _
                              "ReadProcessMemory(PEB)", _
                              "hProcess=0x" & WHex(hProcess) & _
                              ",Addr=0x" & WHex(pebAddr)

        Erase ModuleList
        Return -1
    End If

    If peb.Ldr = 0 Then
        Erase ModuleList
        Return 0
    End If

    '===========================================================
    ' 2. 读取 PEB_LDR_DATA32
    '===========================================================
    If ReadProcessMemory( _
        hProcess, _
        Cast(Any Ptr, Cast(ULONG_PTR, peb.Ldr)), _
        @ldrData, _
        SizeOf(ldrData), _
        @bytesRead) = 0 OrElse bytesRead <> SizeOf(ldrData) Then

        MyLog.PrintWin32Error "EnumModules32", _
                              "ReadProcessMemory(PEB_LDR_DATA)", _
                              "hProcess=0x" & WHex(hProcess) & _
                              ",Addr=0x" & WHex(peb.Ldr)

        Erase ModuleList
        Return -1
    End If

    '===========================================================
    ' 3. 计算真正的 InMemoryOrderModuleList 链表头地址
    '===========================================================
    listHead = Cast(ULONG_PTR, peb.Ldr) + _
               OffsetOf(PEB_LDR_DATA32, InMemoryOrderModuleList)

    ' 第一个节点
    currentNode = Cast(ULONG_PTR, ldrData.InMemoryOrderModuleList.Flink)

    '===========================================================
    ' 4. 遍历链表
    '===========================================================
    Do While currentNode <> listHead

        ' 防御性检查
        If CurrentNode = 0 Then Exit Do

        '=======================================================
        ' 5. 容量不足时扩容
        '=======================================================
        If moduleIdx >= capacity Then
            capacity += 50
            ReDim Preserve ModuleList(capacity - 1) As MODULE_INFORMATION
        End If

        '=======================================================
        ' 6. 根据 InMemoryOrderLinks 反推
        '    LDR_DATA_TABLE_ENTRY32 地址
        '=======================================================
        Dim entryAddr As ULONG_PTR

        entryAddr = currentNode - _
                    OffsetOf(LDR_DATA_TABLE_ENTRY32, InMemoryOrderLinks)

        If ReadProcessMemory( _
            hProcess, _
            Cast(Any Ptr, entryAddr), _
            @ldrEntry, _
            SizeOf(ldrEntry), _
            @bytesRead) = 0 Then

            Exit Do
        End If

        '=======================================================
        ' 7. 读取模块名
        '=======================================================
        Dim moduleName As WString * MAX_PATH

        moduleName = ""

        If ldrEntry.BaseDllName.Buffer <> 0 AndAlso _
           ldrEntry.BaseDllName.Length > 0 Then

            Dim nameBytes As DWORD = ldrEntry.BaseDllName.Length

            If nameBytes > (SizeOf(moduleName) - 2) Then
                nameBytes = SizeOf(moduleName) - 2
            End If

            If ReadProcessMemory( _
                hProcess, _
                Cast(Any Ptr, Cast(ULONG_PTR, ldrEntry.BaseDllName.Buffer)), _
                @moduleName, _
                nameBytes, _
                @bytesRead) <> 0 Then

                moduleName[nameBytes \ 2] = 0
            End If
        End If

        '=======================================================
        ' 8. 读取模块路径
        '=======================================================
        Dim modulePath As WString * MAX_PATH

        modulePath = ""

        If ldrEntry.FullDllName.Buffer <> 0 AndAlso _
           ldrEntry.FullDllName.Length > 0 Then

            Dim pathBytes As DWORD = ldrEntry.FullDllName.Length

            If pathBytes > (SizeOf(modulePath) - 2) Then
                pathBytes = SizeOf(modulePath) - 2
            End If

            If ReadProcessMemory( _
                hProcess, _
                Cast(Any Ptr, Cast(ULONG_PTR, ldrEntry.FullDllName.Buffer)), _
                @modulePath, _
                pathBytes, _
                @bytesRead) <> 0 Then

                modulePath[pathBytes \ 2] = 0
            End If
        End If

        '=======================================================
        ' 9. 保存模块
        '=======================================================
        With ModuleList(moduleIdx)
            .BaseAddr = ldrEntry.DllBase
            .dwSize = ldrEntry.SizeOfImage
            .szName = moduleName
            .szPath = modulePath
        End With

        moduleIdx += 1

        '=======================================================
        ' 10. 读取当前节点的 Flink
        '=======================================================
        If ReadProcessMemory( _
            hProcess, _
            Cast(Any Ptr, currentNode), _
            @listEntry, _
            SizeOf(listEntry), _
            @bytesRead) = 0 OrElse bytesRead <> SizeOf(listEntry) Then

            Exit Do
        End If

        currentNode = Cast(ULONG_PTR, listEntry.Flink)

        FF_DoEvents

    Loop

    '===========================================================
    ' 11. 返回实际数量
    '===========================================================
    If moduleIdx = 0 Then
        Erase ModuleList
        Return 0
    End If

    ReDim Preserve ModuleList(moduleIdx - 1) As MODULE_INFORMATION

    Return moduleIdx

End Function

Private Function EnumModules64(ByVal hProcess As HANDLE, _
                               ByVal pebAddr As ULONG_PTR, _
                               ModuleList() As MODULE_INFORMATION) As Long

    Dim peb As PEB64
    Dim ldrData As PEB_LDR_DATA64
    Dim listEntry As LIST_ENTRY64
    Dim currentNode As ULONG_PTR
    Dim listHead As ULONG_PTR
    Dim ldrEntry As LDR_DATA_TABLE_ENTRY64

    Dim bytesRead As SIZE_T = 0
    Dim moduleIdx As Long = 0
    Dim capacity As Long = 50

    Dim temp() As MODULE_INFORMATION
    ReDim temp(capacity - 1) As MODULE_INFORMATION

    '===========================================================
    ' 1. 读取 PEB
    '===========================================================
    If ReadProcessMemory( _
        hProcess, _
        Cast(Any Ptr, pebAddr), _
        @peb, _
        SizeOf(peb), _
        @bytesRead) = 0 OrElse bytesRead <> SizeOf(peb) Then

        MyLog.PrintWin32Error _
            "EnumModules64", _
            "ReadProcessMemory(PEB)", _
            "hProcess=0x" & WHex(hProcess) & _
            ",Addr=0x" & WHex(pebAddr)

        Erase ModuleList
        Return -1
    End If

    If peb.Ldr = 0 Then
        Erase ModuleList
        Return 0
    End If

    '===========================================================
    ' 2. 读取 PEB_LDR_DATA
    '===========================================================
    If ReadProcessMemory( _
        hProcess, _
        Cast(Any Ptr, peb.Ldr), _
        @ldrData, _
        SizeOf(ldrData), _
        @bytesRead) = 0 OrElse bytesRead <> SizeOf(ldrData) Then

        MyLog.PrintWin32Error _
            "EnumModules64", _
            "ReadProcessMemory(PEB_LDR_DATA)", _
            "hProcess=0x" & WHex(hProcess) & _
            ",Addr=0x" & WHex(peb.Ldr)

        Erase ModuleList
        Return -1
    End If

    '===========================================================
    ' 3. InMemoryOrderModuleList 的链表头
    '===========================================================
    listHead = Cast(ULONG_PTR, peb.Ldr) + _
               OffsetOf(PEB_LDR_DATA64, InMemoryOrderModuleList)

    currentNode = Cast(ULONG_PTR, _
                       ldrData.InMemoryOrderModuleList.Flink)

    '===========================================================
    ' 4. 遍历链表
    '===========================================================
    Do While currentNode <> listHead

        If currentNode = 0 Then Exit Do

        '=======================================================
        ' 5. 扩容
        '=======================================================
        If moduleIdx >= capacity Then
            capacity += 50
            ReDim Preserve temp(capacity - 1) _
                As MODULE_INFORMATION
        End If

        '=======================================================
        ' 6. 获取 LDR_DATA_TABLE_ENTRY
        '=======================================================
        Dim entryAddr As ULONG_PTR

        entryAddr = currentNode - _
                    OffsetOf(LDR_DATA_TABLE_ENTRY64, _
                             InMemoryOrderLinks)

        If ReadProcessMemory( _
            hProcess, _
            Cast(Any Ptr, entryAddr), _
            @ldrEntry, _
            SizeOf(ldrEntry), _
            @bytesRead) = 0 OrElse _
            bytesRead <> SizeOf(ldrEntry) Then

            Exit Do
        End If

        '=======================================================
        ' 7. BaseDllName
        '=======================================================
        Dim moduleName As WString * MAX_PATH
        moduleName = ""

        If ldrEntry.BaseDllName.Buffer <> 0 AndAlso _
           ldrEntry.BaseDllName.Length > 0 Then

            Dim nameBytes As DWORD = ldrEntry.BaseDllName.Length

            If nameBytes > SizeOf(moduleName) - 2 Then
                nameBytes = SizeOf(moduleName) - 2
            End If

            If ReadProcessMemory( _
                hProcess, _
                Cast(Any Ptr, ldrEntry.BaseDllName.Buffer), _
                @moduleName, _
                nameBytes, _
                @bytesRead) <> 0 Then

                moduleName[nameBytes \ 2] = 0
            End If
        End If

        '=======================================================
        ' 8. FullDllName
        '=======================================================
        Dim modulePath As WString * MAX_PATH
        modulePath = ""

        If ldrEntry.FullDllName.Buffer <> 0 AndAlso _
           ldrEntry.FullDllName.Length > 0 Then

            Dim pathBytes As DWORD = ldrEntry.FullDllName.Length

            If pathBytes > SizeOf(modulePath) - 2 Then
                pathBytes = SizeOf(modulePath) - 2
            End If

            If ReadProcessMemory( _
                hProcess, _
                Cast(Any Ptr, ldrEntry.FullDllName.Buffer), _
                @modulePath, _
                pathBytes, _
                @bytesRead) <> 0 Then

                modulePath[pathBytes \ 2] = 0
            End If
        End If

        '=======================================================
        ' 9. 保存
        '=======================================================
        With temp(moduleIdx)
            .BaseAddr = ldrEntry.DllBase
            .dwSize = ldrEntry.SizeOfImage
            .szName = moduleName
            .szPath = modulePath
        End With

        moduleIdx += 1

        '=======================================================
        ' 10. 下一个节点
        '=======================================================
        If ReadProcessMemory( _
            hProcess, _
            Cast(Any Ptr, currentNode), _
            @listEntry, _
            SizeOf(listEntry), _
            @bytesRead) = 0 OrElse _
            bytesRead <> SizeOf(listEntry) Then

            Exit Do
        End If

        currentNode = Cast(ULONG_PTR, listEntry.Flink)

        FF_DoEvents

    Loop

    '===========================================================
    ' 11. 没有模块
    '===========================================================
    If moduleIdx = 0 Then
        Erase ModuleList
        Return 0
    End If

    '===========================================================
    ' 12. 输出精确数量
    '===========================================================
    ReDim ModuleList(moduleIdx - 1) As MODULE_INFORMATION

    For i As Long = 0 To moduleIdx - 1
        ModuleList(i) = temp(i)
    Next

    Return moduleIdx

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
            MyLog.PrintNtError "EnumModuleListEx", "ZwQueryInformationProcess(ProcessWow64Information)", st, "hProcess=0x" & WHex(hProcess)
            Return -1
        End If
        
        ' 获取64位PEB地址（关键！）
        st = ZwQueryInformationProcess(hProcess, ProcessBasicInformation, @pbi64, SizeOf(pbi64), retLen)
        If Not NT_SUCCESS(st) Then
            MyLog.PrintNtError "EnumModuleListEx", "ZwQueryInformationProcess(ProcessBasicInformation)", st, "hProcess=0x" & WHex(hProcess)
            Return -1
        End If
        
        ' 步骤3：遍历32位模块链表
        count32 = EnumModules32(hProcess, peb_32, list32())
        
        ' 步骤4：遍历64位模块链表（关键！）
        count64 = EnumModules64(hProcess, Cast(ULONG_PTR, pbi64.PebBaseAddress), list64())
        
        ' 步骤5：合并列表（按你要求的顺序）
        Dim totalCount As Long = count32 + count64

        If totalCount <= 0 Then
            Erase ModuleList

            If count32 < 0 OrElse count64 < 0 Then
                Return -1
            End If

            Return 0
        End If

        ReDim ModuleList(totalCount - 1) As MODULE_INFORMATION
        
        Dim idx As Long = 0
        
        ' 2. 添加所有64位模块
        For i As Long = 0 To count64 - 1
            'Print "list64(i):" & list64(i).szName
            ModuleList(idx) = list64(i)
            idx += 1
        Next
        
        ' 3. 添加剩余的32位模块(跳过第一个,避免重复显示exe模块)
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
            MyLog.PrintNtError "EnumModuleListEx", "ZwQueryInformationProcess(ProcessBasicInformation)", st, "hProcess=0x" & WHex(hProcess)
            Return -1
        End If
        
        ' 步骤3：遍历64位模块链表
        moduleCount = EnumModules64(hProcess, Cast(ULONG_PTR, pbi64.PebBaseAddress), ModuleList())
        
        Return moduleCount
    End If
End Function

Type MODULE_NAME_CACHE
    HMODULE As HMODULE
    szPath As Wstring * MAX_PATH
    szName As WString * 256
    BaseAddr As ULONG64
    dwSize As ULONG64
End Type

' 缓存结构，避免重复枚举
Type MODULE_CACHE
    hProcess As HANDLE
    dwPid As DWORD
    bValid As Boolean
    Modules(Any) As MODULE_NAME_CACHE
    nCount As Long
    dwTickCount As DWORD  ' 缓存时间戳
End Type

' 带缓存的模块名查询（适合高频调用场景）
Function GetModuleFileNameEx2Cached( _
    ByVal hProcess As HANDLE, _
    ByVal hModule As HMODULE, _
    ByVal lpFilename As LPWSTR, _
    ByVal nSize As DWORD, _
    ByVal bUseCache As Boolean = True _
) As DWORD

    Static g_ModuleCache As MODULE_CACHE

    Dim i As Long
    Dim hModuleAddr As ULONG64 = Cast(ULONG64, hModule)
    Dim dwPid As DWORD
    Dim bNeedRefresh As Boolean = TRUE
    
    ' 参数校验
    If hProcess = NULL OrElse lpFilename = NULL OrElse nSize = 0 Then
        Return 0
    End If
    
    *lpFilename = 0
    
    ' 获取目标进程 PID
    dwPid = GetProcessId(hProcess)  ' 或你使用的等效函数
    
    ' 检查缓存有效性
    If bUseCache AndAlso g_ModuleCache.bValid Then
        If g_ModuleCache.hProcess = hProcess AndAlso _
           g_ModuleCache.dwPid = dwPid AndAlso _
           (GetTickCount() - g_ModuleCache.dwTickCount) < 500 Then  ' 500ms 缓存有效期
            
            bNeedRefresh = FALSE
            
            ' 在缓存中查找
            For i = 0 To g_ModuleCache.nCount - 1
                If g_ModuleCache.Modules(i).BaseAddr = hModuleAddr Then
                    Dim pathLen As Long = Len(g_ModuleCache.Modules(i).szPath)
                    If pathLen >= nSize Then pathLen = nSize - 1
                    memcpy(lpFilename, @g_ModuleCache.Modules(i).szPath, pathLen * SizeOf(WString))
                    lpFilename[pathLen] = 0
                    Return pathLen
                End If
            Next i
            
            ' 缓存中未找到，需要刷新
            bNeedRefresh = TRUE
        End If
    End If
    
    ' 刷新缓存或直接查询
    If bNeedRefresh Then
        Dim ModuleList() As MODULE_INFORMATION
        Dim nCount As Long = EnumModuleListEx(hProcess, ModuleList())
        
        If nCount <= 0 Then
            Return GetModuleFileNameExW(hProcess, hModule, lpFilename, nSize)
        End If
        
        ' 更新缓存
        If bUseCache Then
            g_ModuleCache.hProcess = hProcess
            g_ModuleCache.dwPid = dwPid
            ReDim g_ModuleCache.Modules(nCount - 1)
            g_ModuleCache.nCount = nCount
            g_ModuleCache.dwTickCount = GetTickCount()
        End If
        
        ' 查找并填充缓存
        For i = 0 To nCount - 1
            If bUseCache Then
                g_ModuleCache.Modules(i).hModule = Cast(HMODULE, ModuleList(i).BaseAddr)
                g_ModuleCache.Modules(i).BaseAddr = ModuleList(i).BaseAddr
                g_ModuleCache.Modules(i).dwSize = ModuleList(i).dwSize
                g_ModuleCache.Modules(i).szPath = *Cast(WString Ptr, @ModuleList(i).szPath)
                g_ModuleCache.Modules(i).szName = *Cast(WString Ptr, @ModuleList(i).szName)
            End If
            
            If ModuleList(i).BaseAddr = hModuleAddr Then
                Dim pathLen As Long = Len(*Cast(WString Ptr, @ModuleList(i).szPath))
                If pathLen >= nSize Then pathLen = nSize - 1
                memcpy(lpFilename, @ModuleList(i).szPath, pathLen * SizeOf(WString))
                lpFilename[pathLen] = 0
                
                If bUseCache Then g_ModuleCache.bValid = TRUE
                Erase ModuleList
                Return pathLen
            End If
        Next i
        
        Erase ModuleList
        If bUseCache Then g_ModuleCache.bValid = TRUE
    End If
    
    ' 后备方案
    Return GetModuleFileNameExW(hProcess, hModule, lpFilename, nSize)
    
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
            'Print "隐藏模块:" & ModuleInformation2(j).szPath
            ItemIndex = AddItemColListEx(lvwModule, 6, FB_Red, , WStr(lvwModule.ItemCount + 1), WStr(ModuleInformation2(j).szName), WStr("0x" & WHex(ModuleInformation2(j).BaseAddr)), _
            WStr("0x" & WHex(ModuleInformation2(j).dwSize)), WStr(ModuleInformation2(j).szPath), WStr(GetFileCompany(ModuleInformation1(j).szPath)))
            VerifyFileSignAsync FrmListView.hWnd, FrmListView.ListView.hWnd, "ListView", ItemIndex, ModuleInformation2(j).szPath
            'SetItemColor lvwModule, lvwModule.ItemCount, FB_Red, FB_White
            i += 1
        End If
    Next
    CommitListViewView lvwModule
    Return i
End Function

'===============================================================================
' 方案A: 跨进程搜索指定模块
' 功能: 查找系统中所有加载了指定模块(按名称匹配)的进程
'===============================================================================

Public Type PROCESS_MODULE_INFO
    dwProcessId     As DWORD
    szProcessName   As WString * 260
    BaseAddr        As ULONG_PTR
    dwSize          As DWORD
    szModulePath    As WString * 520
    szCompany       As WString * 260
End Type

'-------------------------------------------------------------------------------
' 主函数: 在所有进程中搜索加载了指定模块的进程
' 参数:
'   szTargetModule  - 目标模块名称(如 "sechost.dll")，支持部分匹配
'   lvwResult       - 结果列表视图
'   bExactMatch     - 是否精确匹配(默认False=包含匹配)
' 返回: 找到的进程数量
'-------------------------------------------------------------------------------
Public Function FindProcessesByModule(ByVal szTargetModule As WString Ptr, _
                                      lvwResult As Class_ListView, _
                                      ByVal bExactMatch As Boolean = False) As Integer
    Dim i As Long, j As Long
    Dim hSnapshot As HANDLE
    Dim pe32 As PROCESSENTRY32W
    Dim hProcess As HANDLE
    Dim ModuleCount As Long
    Dim ModuleInformation() As MODULE_INFORMATION
    Dim ResultCount As Integer = 0
    Dim ItemIndex As Long
    Dim szTargetLower As WString * 260
    Dim szModuleLower As WString * 260
    
    ' 初始化结果列表
    InitListViewData lvwResult
    
    ' 目标模块名转小写用于不区分大小写比较
    szTargetLower = LCase(*szTargetModule)
    If Len(szTargetLower) = 0 Then
        CommitListViewView lvwResult
        Return 0
    End If
    
    ' 创建进程快照
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    If hSnapshot = INVALID_HANDLE_VALUE Then
        CommitListViewView lvwResult
        Return 0
    End If
    
    pe32.dwSize = SizeOf(PROCESSENTRY32W)
    
    ' 遍历所有进程
    If Process32FirstW(hSnapshot, @pe32) Then
        Do
            ' 跳过System Idle Process (PID 0)
            If pe32.th32ProcessID = 0 Then Continue Do
            
            ' 打开目标进程
            If IsDriverLoaded Then
                hProcess = SnowSword_OpenProcess(pe32.th32ProcessID)
            Else
                hProcess = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, False, pe32.th32ProcessID)
            End If
            
            If hProcess = NULL Then Continue Do
            
            Erase ModuleInformation
            
            ' 枚举该进程的模块
            ModuleCount = EnumModuleListEx(hProcess, ModuleInformation())
            
            If ModuleCount > 0 Then
                For j = 0 To ModuleCount - 1
                    ' 提取模块文件名
                    Dim szModuleName As WString * 260
                    Dim nPos As Integer = InStrRev(ModuleInformation(j).szPath, "\")
                    If nPos > 0 Then
                        szModuleName = Mid(ModuleInformation(j).szPath, nPos + 1)
                    Else
                        szModuleName = ModuleInformation(j).szName
                    End If
                    
                    ' 模块名转小写
                    szModuleLower = LCase(szModuleName)
                    
                    ' 匹配判断
                    Dim bMatched As Boolean = False
                    If bExactMatch Then
                        bMatched = (szModuleLower = szTargetLower)
                    Else
                        bMatched = (InStr(szModuleLower, szTargetLower) > 0)
                    End If
                    
                    If bMatched Then
                        ' 找到匹配，添加到结果列表
                        ItemIndex = AddItemColListEx(lvwResult, 6, , , _
                            WStr(pe32.th32ProcessID), _
                            WStr(pe32.szExeFile), _
                            WStr("0x" & WHex(ModuleInformation(j).BaseAddr)), _
                            WStr("0x" & WHex(ModuleInformation(j).dwSize)), _
                            WStr(ModuleInformation(j).szPath), _
                            WStr(GetFileCompany(ModuleInformation(j).szPath)))
                        
                        ' 异步验证签名
                        VerifyFileSignAsync FrmListView.hWnd, FrmListView.ListView.hWnd, "ListView", ItemIndex, ModuleInformation(j).szPath
                        
                        ResultCount += 1
                        
                        ' 如果只需要知道"哪些进程加载了"，每个进程只记录一次即可
                        ' 如果要记录"同一个进程中的多个实例"，去掉下面的Exit For
                        ' Exit For
                    End If
                Next j
            End If
            
            ' 关闭进程句柄
            CloseHandle(hProcess)
            
        Loop While Process32NextW(hSnapshot, @pe32)
    End If
    
    CloseHandle(hSnapshot)
    CommitListViewView lvwResult
    
    Return ResultCount
End Function

'-------------------------------------------------------------------------------
' 便捷函数: 从当前选中的模块出发，搜索所有加载了同名模块的进程
' 用法: 在ListView的右键菜单"查找加载此模块的进程"中调用
'-------------------------------------------------------------------------------
Public Function FindProcessesBySelectedModule(lvwModule As Class_ListView, _
                                               lvwResult As Class_ListView) As Integer
    Dim szSelectedModule As WString * 520
    Dim nSelectedIndex As Integer
    Dim nPos As Integer
    
    ' 获取当前选中的行
    nSelectedIndex = lvwModule.SelectedItem
    If nSelectedIndex < 0 Then Return 0
    
    ' 从选中行获取模块路径 (假设路径在第5列，索引4)
    szSelectedModule = lvwModule.GetItemText(nSelectedIndex, 4)  ' 路径列
    
    ' 提取文件名
    nPos = InStrRev(szSelectedModule, "\")
    If nPos > 0 Then
        szSelectedModule = Mid(szSelectedModule, nPos + 1)
    End If
    
    ' 如果路径为空，尝试获取名称列 (第2列，索引1)
    If Len(szSelectedModule) = 0 Then
        szSelectedModule = lvwModule.GetItemText(nSelectedIndex, 1)
    End If
    
    ' 执行搜索
    Return FindProcessesByModule(@szSelectedModule, lvwResult, False)
End Function

'-------------------------------------------------------------------------------
' 增强版: 同时对比两种枚举方法，标记隐藏模块的加载情况
' 用于安全分析: 查找"被隐藏但仍然被某些进程加载"的模块
'-------------------------------------------------------------------------------
Public Function FindHiddenModuleProcesses(lvwResult As Class_ListView) As Integer
    Dim i As Long, j As Long
    Dim hSnapshot As HANDLE
    Dim pe32 As PROCESSENTRY32W
    Dim hProcess As HANDLE
    Dim ModuleCount1 As Long, ModuleCount2 As Long
    Dim ModuleInformation1() As MODULE_INFORMATION
    Dim ModuleInformation2() As MODULE_INFORMATION
    Dim ResultCount As Integer = 0
    Dim ItemIndex As Long
    
    InitListViewData lvwResult
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    If hSnapshot = INVALID_HANDLE_VALUE Then
        CommitListViewView lvwResult
        Return 0
    End If
    
    pe32.dwSize = SizeOf(PROCESSENTRY32W)
    
    If Process32FirstW(hSnapshot, @pe32) Then
        Do
            If pe32.th32ProcessID = 0 Then Continue Do
            
            If IsDriverLoaded Then
                hProcess = SnowSword_OpenProcess(pe32.th32ProcessID)
            Else
                hProcess = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, False, pe32.th32ProcessID)
            End If
            
            If hProcess = NULL Then Continue Do
            
            ' 两种方法枚举
            ModuleCount1 = EnumModuleListEx(hProcess, ModuleInformation1())
            
            ReDim ModuleInformation2(100) As MODULE_INFORMATION
            ModuleCount2 = GetModuleList1(pe32.th32ProcessID, ModuleInformation2())
            
            ' 查找方法2有但方法1没有的 = 隐藏模块
            For j = 0 To ModuleCount2 - 1
                If Not IsInArray(ModuleInformation2(j).BaseAddr, ModuleInformation1()) Then
                    ItemIndex = AddItemColListEx(lvwResult, 6, FB_Red, , _
                        WStr(pe32.th32ProcessID), _
                        WStr(pe32.szExeFile), _
                        WStr("0x" & WHex(ModuleInformation2(j).BaseAddr)), _
                        WStr("0x" & WHex(ModuleInformation2(j).dwSize)), _
                        WStr(ModuleInformation2(j).szPath), _
                        WStr(GetFileCompany(ModuleInformation2(j).szPath)))
                    
                    VerifyFileSignAsync FrmListView.hWnd, FrmListView.ListView.hWnd, "ListView", ItemIndex, ModuleInformation2(j).szPath
                    ResultCount += 1
                End If
            Next j
            
            CloseHandle(hProcess)
        Loop While Process32NextW(hSnapshot, @pe32)
    End If
    
    CloseHandle(hSnapshot)
    CommitListViewView lvwResult
    
    Return ResultCount
End Function

Function GetModuleOfAddress(hProc As HANDLE, addr As Any Ptr) As StringW
    Dim sz As ZString * MAX_PATH, wsz As WString * MAX_PATH, result As WString * MAX_PATH
    GetMappedFileNameA(hProc, addr, @sz, MAX_PATH)
    'Print sz
    wsz = sz
    NtPathToDosPathW @wsz, @result
    'Print result
    Return result
End Function

Function GetModuleBaseByPath(ByVal hProcess As HANDLE, ByVal szModulePath As String) As PVOID

    Dim Modules() As MODULE_INFORMATION
    Dim Count As Long

    Count = EnumModuleListEx(hProcess, Modules())

    If Count < 0 Then Return NULL

    For i As Long = 0 To Count
        If UCase(Trim(Modules(i).szPath)) = _
           UCase(Trim(szModulePath)) Then
            Return Cast(PVOID, Modules(i).BaseAddr)
        End If
    Next

    MyLog.PrintLog LOG_INFO, "GetModuleBaseByPath",, "szModulePath=" & szModulePath
    Return NULL
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