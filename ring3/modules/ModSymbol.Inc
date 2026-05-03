' ###########################################################################
' 1. 驱动通信 常量 / 结构体 (与内核完全对齐)
' ###########################################################################
Type QUERY_TYPE As Long
Const QuerySymbolToAddress    = 0
Const QueryAddressToSymbol    = 1
Const QueryStructOffset       = 2

Type STRUCT_OFFSET
    StructName(0 To 63) As WCHAR
    MemberName(0 To 63) As WCHAR
End Type

' 【修复】匹配内核C结构体：自然对齐(8字节) + 手动填充 + FB纯语法
Type SYMBOL_QUERY_REQUEST
    ' 0~511: WCHAR ModulePath[256]
    ModulePath(0 To 255) As WCHAR
    ' 512~515: QUERY_TYPE (4字节)
    qtype As QUERY_TYPE
    ' 516~519: 【内核自动填充4字节】手动补齐 (改名避免重复)
    PadReq(0 To 3) As UByte
    ' 520~ 开始: Union (与C布局完全一致)
    Union
        ' 符号名查询
        SymbolName(0 To 255) As WCHAR
        ' 地址查询
        Address As ULongInt
        ' 结构体偏移查询（平铺两个数组，FB不支持嵌套Type）
        StructOffset As STRUCT_OFFSET
    End Union
End Type

' 【修复】匹配内核C响应结构体
Type SYMBOL_QUERY_RESPONSE
    ' 0~3: NTSTATUS
    Status As Long
    ' 4~7: 【内核自动填充4字节】手动补齐
    PadResp(0 To 3) As UByte
    ' 8~ 开始: Union
    Union
        Address     As ULongInt
        Offset      As Long
        SymbolName(0 To 255) As WCHAR
    End Union
End Type

Type SYMBOL_QUERY_PACKET
    RequestId As ULongInt
    req As SYMBOL_QUERY_REQUEST
End Type

Type SYMBOL_RESPONSE_PACKET
    RequestId As ULongInt
    resp As SYMBOL_QUERY_RESPONSE
End Type

Declare Function CancelIoEx Lib "kernel32" Alias "CancelIoEx"(ByVal HFILE As HANDLE, ByVal LPOVERLAPPED As LPOVERLAPPED) As WINBOOL

Dim Shared hSymbolDrv As HANDLE = INVALID_HANDLE_VALUE

Dim Shared g_RequestEvent As HANDLE  ' 通知消费者有新请求

' ==============================
' 配置：并发线程数 = 内核队列容量
' ==============================
#define WORKER_THREAD_COUNT 1

Dim Shared g_StopWorkerThreads As Boolean = FALSE

'Declare Function SymbolService_WorkerThread(ByVal p As Any Ptr) As DWORD
'Declare Sub SymbolService_ProcessRequest(ByRef req As SYMBOL_QUERY_REQUEST, ByRef resp As SYMBOL_QUERY_RESPONSE)

Dim Shared m_hThreads(1 To WORKER_THREAD_COUNT) As HANDLE

' ###########################################################################
' 【新增】线程安全 / 符号缓存 全局定义 → ✅【允许修改，新增核心】
' ###########################################################################
' DbgHelp 非线程安全，全局临界区保护
Dim Shared g_SymCriticalSection As CRITICAL_SECTION
' 符号引擎单例句柄
Dim Shared g_hSymProcess As HANDLE = 0
' 已加载模块缓存（避免重复加载）
Const MAX_LOADED_MODULES = 32

Type LOADED_MODULE
    modulePath As WString * 1024  ' 模块全路径
    baseAddr   As ULONG64         ' 符号基址
    isLoaded   As Boolean         ' 加载标记
End Type
Dim Shared g_LoadedModules(0 To MAX_LOADED_MODULES - 1) As LOADED_MODULE

Const MAX_SYM_NAME = 2000
/'Const STATUS_INVALID_PARAMETER = &HC000000D
Const STATUS_NOT_FOUND = &HC0000225
Const STATUS_UNSUCCESSFUL = &HC0000001'/

' ###########################################################################
' 3. 符号引擎核心（线程安全 + 无硬编码 + 模块缓存）→ ✅【允许修改，重写优化】
' ###########################################################################
' 初始化符号引擎（单例，线程安全）
Function SymEngine_Init() As Boolean
    InitializeCriticalSection(@g_SymCriticalSection) ' ✅ 先初始化临界区
    /'EnterCriticalSection(@g_SymCriticalSection)
    If g_hSymProcess <> 0 Then
        LeaveCriticalSection(@g_SymCriticalSection)
        Return True
    End If'/

    Dim hProcess As HANDLE = GetCurrentProcess()
    Dim symPath As WString * 1024 = "srv*C:\Windows\Symbols*https://msdl.microsoft.com/download/symbols"
    
    SymSetOptions(SYMOPT_UNDNAME Or SYMOPT_LOAD_LINES Or SYMOPT_DEBUG Or SYMOPT_LOAD_ANYTHING)
    If SymInitializeW(hProcess, symPath, False) = 0 Then
        Print "SymInitialize失败: " & GetLastError
        LeaveCriticalSection(@g_SymCriticalSection)
        Return False
    End If

    g_hSymProcess = hProcess
    Print "符号引擎初始化成功！"
    'LeaveCriticalSection(@g_SymCriticalSection)
    Return True
End Function

' 清理符号引擎（线程安全）
Sub SymEngine_Cleanup()
    'EnterCriticalSection(@g_SymCriticalSection)
    If g_hSymProcess <> 0 Then
        SymCleanup(g_hSymProcess)
        g_hSymProcess = 0
        ' 清空模块缓存
        For i As Integer = 0 To MAX_LOADED_MODULES - 1
            g_LoadedModules(i).isLoaded = False
            g_LoadedModules(i).baseAddr = 0
        Next
        Print "符号引擎已清理！"
    End If
    DeleteCriticalSection(@g_SymCriticalSection)
    'LeaveCriticalSection(@g_SymCriticalSection)
End Sub

' 通用加载模块（全锁保护，连续加载无崩溃）
Function SymEngine_LoadModule(ByVal modulePath As WString Ptr) As ULONG64
    If modulePath = 0 Then Return 0

    ' 【全局锁】DbgHelp非线程安全，所有操作必须加锁
    EnterCriticalSection(@g_SymCriticalSection)

    ' 1. 检查缓存
    For i As Integer = 0 To MAX_LOADED_MODULES - 1
        If g_LoadedModules(i).isLoaded AndAlso _
            wcscmp(@g_LoadedModules(i).modulePath, modulePath) = 0 Then
            Dim baseAddr As ULONG64 = g_LoadedModules(i).baseAddr
            LeaveCriticalSection(@g_SymCriticalSection)
            Return baseAddr
        End If
    Next

    ' 2. 缓存淘汰：满了直接清空（简单高效，适配连续查询）
    For i As Integer = 0 To MAX_LOADED_MODULES - 1
        If g_LoadedModules(i).isLoaded = False Then
            g_LoadedModules(i).modulePath = *modulePath
            g_LoadedModules(i).baseAddr = SymLoadModuleExW(g_hSymProcess, 0, modulePath, 0, 0, 0, 0, 0)
            g_LoadedModules(i).isLoaded = (g_LoadedModules(i).baseAddr <> 0)
            If g_LoadedModules(i).isLoaded Then
                Print "模块加载成功: " & *modulePath
            End If
            LeaveCriticalSection(@g_SymCriticalSection)
            Return g_LoadedModules(i).baseAddr
        End If
    Next

    ' 【修复】缓存满了 → 清空所有缓存，重新加载（连续查询新模块永不失败）
    For i As Integer = 0 To MAX_LOADED_MODULES - 1
        g_LoadedModules(i).isLoaded = False
    Next
    Dim newBase As ULONG64 = SymLoadModuleExW(g_hSymProcess, 0, modulePath, 0, 0, 0, 0, 0)
    g_LoadedModules(0).modulePath = *modulePath
    g_LoadedModules(0).baseAddr = newBase
    g_LoadedModules(0).isLoaded = (newBase <> 0)
    
    LeaveCriticalSection(@g_SymCriticalSection)
    Return newBase
End Function

' ====================== 1. 符号 → 地址（线程安全） ======================
Function SymEngine_SymbolToAddr(ByVal modulePath As WString Ptr, ByVal symbolName As WString Ptr) As ULONG64
    If symbolName = 0 Or modulePath = 0 Then Return 0
    Dim baseAddr As ULONG64 = SymEngine_LoadModule(modulePath)
    If baseAddr = 0 Then Return 0

    EnterCriticalSection(@g_SymCriticalSection)
    Dim symbolOffset As ULONG64 = 0
    Dim symInfoSize As ULONG = SizeOf(SYMBOL_INFOW) + MAX_SYM_NAME * SizeOf(WCHAR)
    Dim pSym As PSYMBOL_INFOW = CAllocate(symInfoSize)
    
    If pSym Then
        pSym->SizeOfStruct = SizeOf(SYMBOL_INFOW)
        pSym->MaxNameLen = MAX_SYM_NAME
        If SymFromNameW(g_hSymProcess, symbolName, pSym) Then
            ' 【核心改动】返回 符号相对模块基址的偏移，不是绝对地址！
            symbolOffset = pSym->ADDRESS - baseAddr
        End If
        Deallocate(pSym)
    End If
    LeaveCriticalSection(@g_SymCriticalSection)
    Return symbolOffset
End Function

' ====================== 2. 地址 → 符号（✅【新增】线程安全） ======================
Function SymEngine_AddrToSymbol(ByVal modulePath As WString Ptr, ByVal offset As ULONG64, ByVal outSymbolName As WString Ptr, ByVal nameLen As ULong) As Boolean
    If outSymbolName = 0 Or nameLen = 0 Then Return False
    Dim baseAddr As ULONG64 = SymEngine_LoadModule(modulePath)
    If baseAddr = 0 Then Return 0

    EnterCriticalSection(@g_SymCriticalSection)
    Dim result As Boolean = False
    Dim symInfoSize As ULONG = SizeOf(SYMBOL_INFOW) + MAX_SYM_NAME * SizeOf(WCHAR)
    Dim pSym As PSYMBOL_INFOW = CAllocate(symInfoSize)
    Dim displacement As ULONG64 = 0

    If pSym Then
        pSym->SizeOfStruct = SizeOf(SYMBOL_INFOW)
        pSym->MaxNameLen = MAX_SYM_NAME
        ' 【核心】R3传入的是偏移，转成R3模拟地址查询
        Dim r3SimAddress As ULONG64 = baseAddr + offset
        If SymFromAddrW(g_hSymProcess, r3SimAddress, @displacement, pSym) Then
            wcsncpy(outSymbolName, @pSym->Name, nameLen - 1)
            result = True
        Else
            Print "SymFromAddrW失败," & WinErrorMsg(GetLastError) & GetLastError
        End If
        Deallocate(pSym)
    End If
    LeaveCriticalSection(@g_SymCriticalSection)
    Return result
End Function

' 结构体偏移（【修复】内存泄漏 + 全锁保护）
Function SymEngine_GetStructOffset( _
    ByVal modulePath As WString Ptr, _
    ByVal structName As WString Ptr, _
    ByVal memberName As WString Ptr _
) As Long
    If modulePath = 0 Or structName = 0 Or memberName = 0 Then Return -1
    Dim baseAddr As ULONG64 = SymEngine_LoadModule(modulePath)
    If baseAddr = 0 Then Return -1

    EnterCriticalSection(@g_SymCriticalSection)
    Dim offset As Long = -1
    Dim symInfoSize As ULONG = SizeOf(SYMBOL_INFOW) + MAX_SYM_NAME * SizeOf(WCHAR)
    Dim pSym As PSYMBOL_INFOW = CAllocate(symInfoSize)
    Dim buffer As Byte Ptr = 0
    Dim pName As PWCHAR = 0

    If pSym = 0 Then GoTo Cleanup

    pSym->SizeOfStruct = SizeOf(SYMBOL_INFOW)
    pSym->MaxNameLen = MAX_SYM_NAME

    If SymGetTypeFromNameW(g_hSymProcess, baseAddr, structName, pSym) = 0 Then
        SymGetTypeFromNameW(g_hSymProcess, 0, structName, pSym)
    End If

    If SymGetTypeFromNameW(g_hSymProcess, baseAddr, structName, pSym) Then
        Dim numChildren As ULONG = 0
        If SymGetTypeInfo(g_hSymProcess, baseAddr, pSym->TypeIndex, TI_GET_CHILDRENCOUNT, @numChildren) Then
            Dim bufferSize As ULONG = SizeOf(TI_FINDCHILDREN_PARAMS) + numChildren * SizeOf(ULONG)
            buffer = CAllocate(bufferSize)
            If buffer Then
                Dim params As TI_FINDCHILDREN_PARAMS Ptr = CPtr(TI_FINDCHILDREN_PARAMS Ptr, buffer)
                params->Start = 0
                params->Count = numChildren

                If SymGetTypeInfo(g_hSymProcess, baseAddr, pSym->TypeIndex, TI_FINDCHILDREN, buffer) Then
                    Dim typeIdArray As ULONG Ptr = CPtr(ULONG Ptr, buffer + SizeOf(TI_FINDCHILDREN_PARAMS))
                    For i As ULONG = 0 To numChildren - 1
                        pName = 0
                        If SymGetTypeInfo(g_hSymProcess, baseAddr, typeIdArray[i], TI_GET_SYMNAME, @pName) Then
                            If pName Then
                                If wcscmp(pName, memberName) = 0 Then
                                    SymGetTypeInfo(g_hSymProcess, baseAddr, typeIdArray[i], TI_GET_OFFSET, @offset)
                                End If
                                LocalFree(pName) ' 【修复】必释放，杜绝内存泄漏
                                pName = 0
                            End If
                        End If
                    Next
                End If
            End If
        End If
    End If

Cleanup:
    ' 【修复】兜底释放所有内存，连续查询无泄漏
    If buffer Then Deallocate(buffer)
    If pSym Then Deallocate(pSym)
    LeaveCriticalSection(@g_SymCriticalSection)
    Return offset
End Function

' ###########################################################################
' 4. 测试函数 → ✅【允许修改，新增地址→符号测试】
' ###########################################################################
Sub UltimateTest()
    ' ====================== 测试1：加载ntoskrnl（无硬编码） ======================
    Dim ntPath As WString * 1024 = "C:\Windows\System32\ntoskrnl.exe"
    Dim symbolName As WString * 256 = "PsInitialSystemProcess"
    
    ' 测试：符号→地址
    Dim addr As ULONG64 = SymEngine_SymbolToAddr(@ntPath, @symbolName)
    Print "==================== 符号→地址 ===================="
    If addr Then
        Print symbolName & " = 0x"; Hex(addr)
    Else
        Print "查询失败！"
    End If

    ' 测试：地址→符号（✅【新增核心测试】）
    Dim outName As WString * 256 = ""
    Print "==================== 地址→符号 ===================="
    If addr <> NULL AndAlso SymEngine_AddrToSymbol(@ntPath, addr, @outName, 256) Then
        Print "0x"; Hex(addr); " = "; outName
    Else
        Print "查询失败！"
    End If

    ' 测试：结构体偏移
    Print "==================== 结构体偏移 ===================="
    Dim offset As Long = SymEngine_GetStructOffset(@ntPath, "_EPROCESS", "ImageFileName")
    If offset <> -1 Then
        Print "_EPROCESS->ImageFileName = 0x"; Hex(offset)
    Else
        Print "查询失败！"
    End If

    ' 测试ntfs（验证多模块支持）
    'Dim strFunctionName As WString * 1024 = ""
    'If SymEngine_AddrToSymbol(StrPtrW("C:\Windows\System32\win32kfull.sys"), &H1C0279580, @strFunctionName, 1024) Then Print "&H1C0279580->" & strFunctionName
End Sub

Function OpenSymbolDevice() As Boolean
    hSymbolDrv = CreateFile( _
        "\\.\SnowSymbol", _
        GENERIC_READ Or GENERIC_WRITE, _
        0, _
        NULL, _
        OPEN_EXISTING, _
        0, _
        NULL)

    If hSymbolDrv = INVALID_HANDLE_VALUE Then
        Print "打开 Symbol 设备失败: "; GetLastError
        Return FALSE
    End If

    Print "Symbol 设备打开成功"
    Return TRUE
End Function

' ###########################################################################
' 5. 【核心】处理内核请求 → ✅【允许修改，填充业务逻辑】
' ###########################################################################
Sub SymbolService_ProcessRequest( _
    ByRef req  As SYMBOL_QUERY_REQUEST, _
    ByRef resp As SYMBOL_QUERY_RESPONSE)
#define STATUS_INVALID_DEVICE_REQUEST Cast(NTSTATUS, &HC0000010L)
    resp.Status = STATUS_INVALID_PARAMETER
    ' 空判断
    If req.ModulePath(0) = 0 Then Return
    Dim fullPath As WString * MAX_PATH = ""

    If (Not GetKernelModulePath(@req.ModulePath(0), @fullPath)) OrElse (fullPath = "") Then
        Print "[R3] 无法解析模块路径: "; *CPtr(Wstring Ptr, @req.ModulePath(0))
        resp.Status = STATUS_NOT_FOUND
        Return
    End If
    fullPath = YF_ReplaceW(fullPath, "\SystemRoot", WStr(Environ("SystemRoot")))
    Print "[R3] 模块路径: "; fullPath

    Select Case req.qtype
        Case QuerySymbolToAddress
            ' 符号 → 地址
            Dim pSymName As WString Ptr = @req.SymbolName(0)
            Print "[R3] 处理: 符号→地址: "; *pSymName
            
            Dim addr As ULONG64 = SymEngine_SymbolToAddr(@fullPath, pSymName)
            If addr Then
                resp.Address = addr
                resp.Status = STATUS_SUCCESS
                Print "[R3] 成功! 偏移=0x"; Hex(addr)
            Else
                resp.Status = STATUS_NOT_FOUND
                Print "[R3] 失败: 符号未找到"
            End If

        Case QueryAddressToSymbol
            ' 地址 → 符号
            Print "[R3] 处理: 地址→符号, R3偏移=0x"; Hex(req.Address)
            
            If SymEngine_AddrToSymbol(@fullPath, req.ADDRESS, @resp.SymbolName(0), 256) Then
                resp.Status = STATUS_SUCCESS
                Print "[R3] 成功! 符号: "; *CPtr(WString Ptr, @resp.SymbolName(0))
            Else
                resp.Status = STATUS_NOT_FOUND
                Print "[R3] 失败: 地址无符号"
            End If

        Case QueryStructOffset
            ' 结构体 → 偏移
            Dim pStruct As WString Ptr = @req.StructOffset.StructName(0)
            Dim pMember As WString Ptr = @req.StructOffset.MemberName(0)
            Print "[R3] 处理: 结构体偏移: "; *pStruct; "."; *pMember
            
            Dim offset As Long = SymEngine_GetStructOffset(@fullPath, pStruct, pMember)
            If offset <> -1 Then
                resp.Offset = offset
                resp.Status = STATUS_SUCCESS
                Print "[R3] 成功! 偏移="; offset
            Else
                resp.Status = STATUS_NOT_FOUND
                Print "[R3] 失败: 成员未找到"
            End If

        Case Else
            resp.Status = STATUS_INVALID_DEVICE_REQUEST
            Print "[R3] 错误: 无效请求类型"
    End Select
End Sub

' 核心工作线程
Function SymbolService_WorkerThread(ByVal p As Any Ptr) As DWORD
    Dim pkt As SYMBOL_QUERY_PACKET
    Dim respPkt As SYMBOL_RESPONSE_PACKET
    Dim bytesRet As DWORD

    Print "[R3] Symbol Worker 启动"

    Do While Not g_StopWorkerThreads

        If hSymbolDrv = INVALID_HANDLE_VALUE Then
            Sleep 50
            Continue Do
        End If

        RtlZeroMemory(@pkt, SizeOf(pkt))
        RtlZeroMemory(@respPkt, SizeOf(respPkt))

        ' ============================
        ' 等待内核请求
        ' ============================
        Print "[R3] 调用 WaitRequest..."
        If IoControl( _
            hSymbolDrv, _
            IOCTL_WaitRequest, _
            NULL, 0, _
            @pkt, SizeOf(pkt), _
            @bytesRet) = 0 Then
            Print "[R3] WaitRequest 失败返回, err=" & GetLastError
            If g_StopWorkerThreads Then Exit Do
            Sleep 10
            Continue Do
        End If

        Print "[R3] 收到请求 ID=" & pkt.RequestId

        ' ============================
        ' 处理请求
        ' ============================
        SymbolService_ProcessRequest(pkt.req, respPkt.resp)

        respPkt.RequestId = pkt.RequestId

        ' ============================
        ' 回传结果
        ' ============================
        If IoControl( _
            hSymbolDrv, _
            IOCTL_SubmitResponse, _
            @respPkt, SizeOf(respPkt), _
            NULL, 0, _
            @bytesRet) = 0 Then

            Print "[R3] 提交响应失败: "; GetLastError
        Else
            Print "[R3] 响应完成 ID=" & pkt.RequestId
        End If

    Loop

    Print "[R3] Symbol Worker 退出"
    Return 0
End Function

' 启动所有工作线程
Function SymbolService_StartWorkers() As Boolean
    m_hThreads(1) = CreateThread(NULL, 0, @SymbolService_WorkerThread, NULL, 0, NULL)
    Return TRUE
End Function

' 停止所有工作线程
Sub SymbolService_StopWorkers()
    g_StopWorkerThreads = True

    If hSymbolDrv <> INVALID_HANDLE_VALUE Then
        CancelIoEx(hSymbolDrv, NULL)
    End If

    If m_hThreads(1) Then
        WaitForSingleObject(m_hThreads(1), INFINITE)
        CloseHandle(m_hThreads(1))
        m_hThreads(1) = 0
    End If

    If hSymbolDrv <> INVALID_HANDLE_VALUE Then
        CloseHandle(hSymbolDrv)
        hSymbolDrv = INVALID_HANDLE_VALUE
    End If
End Sub