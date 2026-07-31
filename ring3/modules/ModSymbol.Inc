#include Once "win/imagehlp.bi"
#inclib "dbghelp"
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
        ' 结构体偏移查询（平铺两个数组,FB不支持嵌套Type）
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

Declare Function SymRegisterCallbackW64 Lib "dbghelp.dll" Alias "SymRegisterCallbackW64"(ByVal hProcess As HANDLE, ByVal CallbackFunction As PSYMBOL_REGISTERED_CALLBACK64, ByVal UserContext As ULONG64) As WINBOOL
Declare Function CancelIoEx Lib "kernel32" Alias "CancelIoEx"(ByVal HFILE As HANDLE, ByVal LPOVERLAPPED As LPOVERLAPPED) As WINBOOL

Dim Shared hSymbolDrv As HANDLE = INVALID_HANDLE_VALUE

' ==============================
' 配置：并发线程数 = 内核队列容量
' ==============================
#define WORKER_THREAD_COUNT 1

Dim Shared g_StopWorkerThreads As Boolean = FALSE
Dim Shared m_hThreads(1 To WORKER_THREAD_COUNT) As HANDLE

Type IMAGEHLP_CBA_EVENTW
    severity As DWORD
    code As DWORD
    desc As WString Ptr   ' 注意：这里是 WString 指针,对应 Unicode 字符串
    Object As Any Ptr
End Type

' 用于接收进度事件
Dim Shared g_hProgressEvent As HANDLE
Dim Shared g_SymbolPercent As DWORD

' ###########################################################################
' 【新增】线程安全 / 符号缓存 全局定义 → ✅【允许修改,新增核心】
' ###########################################################################
' 符号引擎单例句柄
Dim Shared g_hSymProcess As HANDLE = 0
' 已加载模块缓存（避免重复加载）
Const MAX_LOADED_MODULES = 32

Type LOADED_MODULE
    modulePath As Wstring * 1024  ' 模块全路径
    baseAddr As ULONG64 ' 符号基址
    SymBase As ULONG64 ' DbgHelp返回地址
    isLoaded   As Boolean         ' 加载标记
End Type
Dim Shared g_LoadedModules(0 To MAX_LOADED_MODULES - 1) As LOADED_MODULE

Type LOAD_MODULE_PARAMS
    modulePath As WString Ptr
    imageBase As ULONG64
    resultBase As ULONG64
    completed As BOOL
    hEvent As HANDLE   ' 完成事件
End Type

Const MAX_SYM_NAME = 2000
/'Const STATUS_INVALID_PARAMETER = &HC000000D
Const STATUS_NOT_FOUND = &HC0000225
Const STATUS_UNSUCCESSFUL = &HC0000001'/

' ###########################################################################
' 3. 符号引擎核心（线程安全 + 无硬编码 + 模块缓存）→ ✅【允许修改,重写优化】
' ###########################################################################
Function SymbolCallback( _
    ByVal hProcess As HANDLE, _
    ByVal ActionCode As ULONG, _
    ByVal CallbackData As ULONG64, _
    ByVal UserContext As ULONG64 _
) As BOOL
    
    Function = True

    Select Case ActionCode

        Case CBA_EVENT

            Dim pEvent As IMAGEHLP_CBA_EVENTW Ptr
            pEvent = Cast(IMAGEHLP_CBA_EVENTW Ptr, CallbackData)

            If pEvent <> 0 Then
                If pEvent->desc <> 0 Then
                    ' 保存进度
                    Dim ProgressPrecent As Integer = ExtractPercent(pEvent->desc)
                    If ProgressPrecent <> -1 Then g_SymbolPercent = ProgressPrecent

                    ' 通知UI刷新
                    If g_hProgressEvent <> 0 Then
                        SetEvent(g_hProgressEvent)
                    End If
                End If
            End If

        Case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE
            'Print "[LOAD_COMPLETE]"
            
        Case CBA_DEFERRED_SYMBOL_LOAD_CANCEL
            Function = False
            
        Case Else
            Function = False

    End Select
End Function

Function SymEngine_InitInternal() As Boolean
    Dim hProcess As HANDLE = GetCurrentProcess()
    Dim symPath As WString * 1024 = "srv*C:\Windows\Symbols*https://msdl.microsoft.com/download/symbols"
    
    SymSetOptions(SYMOPT_UNDNAME Or SYMOPT_LOAD_LINES Or SYMOPT_DEBUG Or SYMOPT_LOAD_ANYTHING)
    If SymInitializeW(hProcess, symPath, False) = 0 Then
        MyLog.PrintWin32Error "SymEngine_InitInternal", "SymInitialize失败"
        Return False
    End If

    g_hSymProcess = hProcess
    g_hProgressEvent = CreateEvent(NULL, False, False, NULL)
    SymRegisterCallbackW64(hProcess, @SymbolCallback, 0)
    MyLog.PrintInfo "SymEngine_InitInternal", "SymInitialize", "符号引擎初始化成功!"
    Return True
End Function

Sub SymEngine_CleanupInternal()
    If g_hSymProcess <> 0 Then
        SymCleanup(g_hSymProcess)
        g_hSymProcess = 0
        For i As Integer = 0 To MAX_LOADED_MODULES - 1
            g_LoadedModules(i).isLoaded = False
        Next
        MyLog.PrintInfo "SymEngine_CleanupInternal",, "符号引擎已清理!"
    End If
End Sub

' 通用加载模块（增强诊断版）
Function SymbolActor_LoadModuleInternal(ByVal modulePath As WString Ptr) As ULONG64
    If modulePath = 0 Then
        MyLog.PrintLog LOG_ERROR, "SymbolActor_LoadModuleInternal",, "传入模块路径为空指针"
        Return 0
    End If

    MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "=========================================================="
    MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "Module : " & *modulePath

    '------------------------------------------------------
    ' 1. 缓存检查
    '------------------------------------------------------
    For i As Integer = 0 To MAX_LOADED_MODULES - 1
        If g_LoadedModules(i).isLoaded AndAlso _
           wcscmp(@g_LoadedModules(i).modulePath, modulePath) = 0 Then

            MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "Cache Hit"
            MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "SymBase = 0x" & WHex(g_LoadedModules(i).SymBase)
            Return g_LoadedModules(i).SymBase
        End If
    Next

    MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "Cache Miss"

    '------------------------------------------------------
    ' 2. 加载模块（尝试加载符号）
    '------------------------------------------------------
    SetLastError 0
    Dim symBaseTmp As ULONG64 = SymLoadModuleExW(g_hSymProcess, 0, modulePath, 0, 0, 0, 0, 0)
    Dim err1 As DWORD = GetLastError

    If symBaseTmp = 0 AndAlso err1 <> 0 Then
        ' SymLoadModuleExW 失败,但后续可能通过 BaseOfImage 补救,记录警告
        MyLog.PrintLog LOG_WARN, "SymbolActor_LoadModuleInternal",, "SymLoadModuleExW 返回0, LastError=" & err1 & " (" & WinErrorMsg(err1) & "),尝试通过模块信息获取基址"
    End If

    '------------------------------------------------------
    ' 3. 查询模块信息（获取基址等）
    '------------------------------------------------------
    Dim modInfo As IMAGEHLP_MODULEW64
    memset(@modInfo, 0, SizeOf(modInfo))
    modInfo.SizeOfStruct = SizeOf(modInfo)

    Dim bRet As Boolean = SymGetModuleInfoW64(g_hSymProcess, IIf(symBaseTmp <> 0, symBaseTmp, 0), @modInfo)

    If bRet Then
        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "SymGetModuleInfo 成功"
        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "BaseOfImage      : 0x" & WHex(modInfo.BaseOfImage)
        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "ImageName        : " & modInfo.ImageName
        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "LoadedImageName  : " & modInfo.LoadedImageName
        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "LoadedPdbName    : " & modInfo.LoadedPdbName

        ' 如果 SymLoadModuleExW 返回0,但查询成功,则使用 BaseOfImage
        If symBaseTmp = 0 Then
            symBaseTmp = modInfo.BaseOfImage
            MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "使用 BaseOfImage 作为 SymBase"
        End If
    Else
        Dim err2 As DWORD = GetLastError
        MyLog.PrintLog LOG_ERROR, "SymbolActor_LoadModuleInternal",, "SymGetModuleInfoW64 失败, LastError=" & err2 & " (" & WinErrorMsg(err2) & ")"
        ' 查询模块信息失败,无法获取基址,函数最终会失败
    End If

    '------------------------------------------------------
    ' 4. 写入缓存（仅当成功获取到有效基址）
    '------------------------------------------------------
    If symBaseTmp <> 0 Then
        For i As Integer = 0 To MAX_LOADED_MODULES - 1
            If g_LoadedModules(i).isLoaded = FALSE Then
                g_LoadedModules(i).modulePath = *modulePath
                g_LoadedModules(i).baseAddr = 0
                g_LoadedModules(i).SymBase = symBaseTmp
                g_LoadedModules(i).isLoaded = TRUE

                MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "Cache Saved -> Slot " & i
                Exit For
            End If
        Next

        MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "SUCCESS, SymBase = 0x" & WHex(symBaseTmp)
    Else
        MyLog.PrintLog LOG_ERROR, "SymbolActor_LoadModuleInternal",, "FAILED: 无法获取有效模块基址"
    End If

    MyLog.PrintLog LOG_INFO, "SymbolActor_LoadModuleInternal",, "=========================================================="
    Return symBaseTmp
End Function

' ====================== 1. 符号 → 地址（线程安全） ======================
Function SymbolActor_SymbolToAddrInternal(ByVal modulePath As WString Ptr, ByVal symbolName As WString Ptr) As ULONG64
    If symbolName = 0 Or modulePath = 0 Then Return 0
    Dim symBase As ULONG64 = SymbolActor_LoadModuleInternal(modulePath)
    If symBase = 0 Then
        MyLog.PrintLog LOG_ERROR, "SymEngine_SymbolToAddr", "SymbolActor_LoadModuleInternal", "symBase=0"
        Return 0
    End If

    Dim symbolOffset As ULONG64 = 0
    Dim symInfoSize As ULONG = SizeOf(SYMBOL_INFOW) + MAX_SYM_NAME * SizeOf(WCHAR)
    Dim pSym As PSYMBOL_INFOW = CAllocate(symInfoSize)
    
    If pSym Then
        pSym->SizeOfStruct = SizeOf(SYMBOL_INFOW)
        pSym->MaxNameLen = MAX_SYM_NAME
        If SymFromNameW(g_hSymProcess, symbolName, pSym) Then
            ' 【核心改动】返回 符号相对模块基址的偏移,不是绝对地址!
            symbolOffset = pSym->ADDRESS - symBase
        Else
            MyLog.PrintWin32Error "SymEngine_SymbolToAddr", "SymFromNameW", "modulePath=" & *modulePath & ",symbolName=" & *symbolName
        End If
        Deallocate(pSym)
    End If
    Return symbolOffset
End Function

' ====================== 2. 地址 → 符号（✅【新增】线程安全） ======================
Function SymbolActor_AddrToSymbolInternal(ByVal modulePath As WString Ptr, ByVal offset As ULONG64, ByVal outSymbolName As WString Ptr, ByVal nameLen As ULong) As Boolean
    If outSymbolName = 0 Or nameLen = 0 Then Return False
    Dim baseAddr As ULONG64 = SymbolActor_LoadModuleInternal(modulePath)
    If baseAddr = 0 Then Return 0

    Dim result As Boolean = False
    Dim symInfoSize As ULONG = SizeOf(SYMBOL_INFOW) + MAX_SYM_NAME * SizeOf(WCHAR)
    Dim pSym As PSYMBOL_INFOW = CAllocate(symInfoSize)
    Dim displacement As ULONG64 = 0

    If pSym Then
        pSym->SizeOfStruct = SizeOf(SYMBOL_INFOW)
        pSym->MaxNameLen = MAX_SYM_NAME
        ' 【核心】R3传入的是偏移,转成R3模拟地址查询
        Dim r3SimAddress As ULONG64 = baseAddr + offset
        If SymFromAddrW(g_hSymProcess, r3SimAddress, @displacement, pSym) Then
            wcsncpy(outSymbolName, @pSym->Name, nameLen - 1)
            result = True
        Else
            MyLog.PrintWin32Error "SymbolActor_AddrToSymbolInternal", "SymFromAddrW", "modulePath=" & *modulePath & ",offset=" & offset
        End If
        Deallocate(pSym)
    End If
    Return result
End Function

' 结构体偏移（【修复】内存泄漏 + 全锁保护）
Function SymbolActor_GetStructOffsetInternal( _
    ByVal modulePath As WString Ptr, _
    ByVal structName As WString Ptr, _
    ByVal memberName As WString Ptr _
) As Long
    If modulePath = 0 Or structName = 0 Or memberName = 0 Then Return -1
    Dim baseAddr As ULONG64 = SymbolActor_LoadModuleInternal(modulePath)
    If baseAddr = 0 Then Return -1

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
                                LocalFree(pName) ' 【修复】必释放,杜绝内存泄漏
                                pName = 0
                            End If
                        End If
                    Next
                End If
            End If
        End If
    End If

Cleanup:
    ' 【修复】兜底释放所有内存,连续查询无泄漏
    If buffer Then Deallocate(buffer)
    If pSym Then Deallocate(pSym)
    Return offset
End Function

Function OpenSymbolDevice() As Boolean
    hSymbolDrv = CreateFile("\\.\SnowSymbol", GENERIC_READ Or GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL)

    If hSymbolDrv = INVALID_HANDLE_VALUE Then
        MyLog.PrintWin32Error "OpenSymbolDevice", "CreateFile", "lpFileName=\\.\SnowSymbol"
        Return False
    End If

    MyLog.PrintInfo "OpenSymbolDevice", "CreateFile", "Symbol设备打开成功"
    Return True
End Function

' ###########################################################################
' 5. 【核心】处理内核请求 → ✅【允许修改,填充业务逻辑】
' ###########################################################################
Sub SymbolService_ProcessRequest(ByRef req As SYMBOL_QUERY_REQUEST, ByRef resp As SYMBOL_QUERY_RESPONSE)
#define STATUS_INVALID_DEVICE_REQUEST Cast(NTSTATUS, &HC0000010L)
    resp.Status = STATUS_INVALID_PARAMETER
    If req.ModulePath(0) = 0 Then Return

    Dim fullPath As WString * MAX_PATH
    If (Not GetKernelModulePath(@req.ModulePath(0), @fullPath)) OrElse (fullPath = "") Then
        resp.Status = STATUS_NOT_FOUND
        Return
    End If
    fullPath = YF_ReplaceW(fullPath, "\SystemRoot", WStr(Environ("SystemRoot")))

    Dim pReq As SYMBOL_REQUEST Ptr = 0

    Select Case req.qtype
        Case QuerySymbolToAddress
            pReq = SymbolActor_SubmitRequest(REQ_SYMBOL_TO_ADDR, @fullPath, @req.SymbolName(0))
            If pReq Then
                WaitForSingleObject(pReq->hComplete, INFINITE)
                If pReq->ResultStatus = STATUS_SUCCESS Then
                    MyLog.PrintInfo "QuerySymbolToAddress", "QuerySymbolToAddress", "符号" & *CPtr(Wstring Ptr, @req.SymbolName(0)) & "解析成功!"
                    resp.ADDRESS = pReq->ResultAddress
                    resp.Status = STATUS_SUCCESS
                Else
                    MyLog.PrintLog LOG_ERROR, "QuerySymbolToAddress", "QuerySymbolToAddress", "符号" & *CPtr(Wstring Ptr, @req.SymbolName(0)) & "解析失败"
                    resp.Status = STATUS_NOT_FOUND
                End If
                Deallocate(pReq)
            End If

        Case QueryAddressToSymbol
            pReq = SymbolActor_SubmitRequest(REQ_ADDR_TO_SYMBOL, @fullPath, , req.Address, , , _
                                             @resp.SymbolName(0), 256)
            If pReq Then
                WaitForSingleObject(pReq->hComplete, INFINITE)
                resp.Status = pReq->ResultStatus
                Deallocate(pReq)
            End If

        Case QueryStructOffset
            pReq = SymbolActor_SubmitRequest(REQ_STRUCT_OFFSET, @fullPath, , , _
                                             @req.StructOffset.StructName(0), _
                                             @req.StructOffset.MemberName(0))
            If pReq Then
                WaitForSingleObject(pReq->hComplete, INFINITE)
                If pReq->ResultStatus = STATUS_SUCCESS Then
                    resp.Offset = pReq->ResultOffset
                    resp.Status = STATUS_SUCCESS
                Else
                    resp.Status = STATUS_NOT_FOUND
                End If
                Deallocate(pReq)
            End If

        Case Else
            resp.Status = STATUS_INVALID_DEVICE_REQUEST
    End Select
End Sub

' 核心工作线程
Function SymbolService_WorkerThread(ByVal p As Any Ptr) As DWORD
    Dim pkt As SYMBOL_QUERY_PACKET
    Dim respPkt As SYMBOL_RESPONSE_PACKET
    Dim bytesRet As DWORD

    MyLog.PrintInfo "SymbolService_WorkerThread",, "Symbol Worker启动"

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
        MyLog.PrintInfo "SymbolService_WorkerThread",, "调用WaitRequest..."
        If IoControl(hSymbolDrv, IOCTL_WaitRequest, NULL, 0, @pkt, SizeOf(pkt), @bytesRet) = 0 Then
            MyLog.PrintWin32Error "SymbolService_WorkerThread", "IOCTL_WaitRequest"
            If g_StopWorkerThreads Then Exit Do
            Sleep 10
            Continue Do
        End If

        MyLog.PrintInfo "SymbolService_WorkerThread",, "收到请求,ID=" & pkt.RequestId

        ' ============================
        ' 处理请求
        ' ============================
        SymbolService_ProcessRequest(pkt.req, respPkt.resp)

        respPkt.RequestId = pkt.RequestId

        ' ============================
        ' 回传结果
        ' ============================
        If IoControl(hSymbolDrv, IOCTL_SubmitResponse, @respPkt, SizeOf(respPkt), NULL, 0, @bytesRet) = 0 Then
            MyLog.PrintWin32Error "SymbolService_WorkerThread", "IOCTL_SubmitResponse", "请求回复失败"
        Else
            MyLog.PrintInfo "SymbolService_WorkerThread", "IOCTL_SubmitResponse", "请求回复成功!"
        End If

    Loop

    MyLog.PrintInfo "SymbolService_WorkerThread",, "Symbol Worker退出"
    Return 0
End Function

' 启动所有工作线程
Function SymbolService_StartWorkers() As Boolean
    g_StopWorkerThreads = False
    m_hThreads(1) = CreateThread(NULL, 0, @SymbolService_WorkerThread, NULL, 0, NULL)
    Return True
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

' ============================================================================
' Actor 模型：所有符号操作都通过 SymbolThread 串行执行
' ============================================================================
Enum SymbolRequestType
    REQ_PRELOAD_WIN32KFULL = 0
    REQ_LOAD_MODULE        = 1
    REQ_SYMBOL_TO_ADDR     = 2
    REQ_ADDR_TO_SYMBOL     = 3
    REQ_STRUCT_OFFSET      = 4
    REQ_SHUTDOWN           = 99
End Enum

Type SYMBOL_REQUEST
    reqType        As SymbolRequestType
    ' 输入
    ModulePath     As WString * 260
    SymbolName     As WString * 256
    StructName     As WString * 64
    MemberName     As WString * 64
    Address        As ULONG64
    NameLen        As ULong               ' 用于 AddrToSymbol 的输出缓冲区长度
    pOutputName    As WString Ptr         ' AddrToSymbol 输出缓冲区指针
    ' 输出
    ResultStatus   As Long
    ResultAddress  As ULONG64
    ResultOffset   As Long
    ' 同步/通知
    hComplete      As HANDLE             ' 事件,调用者等待完成信号
    pNext          As SYMBOL_REQUEST Ptr ' 链表指针
End Type

Dim Shared g_hQueueNotEmpty As HANDLE      ' 自动重置事件,队列非空时触发
Dim Shared g_csQueue       As CRITICAL_SECTION
Dim Shared g_pQueueHead    As SYMBOL_REQUEST Ptr = 0
Dim Shared g_pQueueTail    As SYMBOL_REQUEST Ptr = 0
Dim Shared g_hSymbolThread As HANDLE

' ----------------------------------------------------------------------------
' 队列操作（仅被 Submit 函数和 Actor 线程调用）
' ----------------------------------------------------------------------------
Sub Queue_Push(ByVal pReq As SYMBOL_REQUEST Ptr)
    EnterCriticalSection(@g_csQueue)
    pReq->pNext = 0
    If g_pQueueTail Then
        g_pQueueTail->pNext = pReq
    Else
        g_pQueueHead = pReq
    End If
    g_pQueueTail = pReq
    SetEvent(g_hQueueNotEmpty)           ' 通知 Actor 线程
    LeaveCriticalSection(@g_csQueue)
End Sub

Function Queue_Pop() As SYMBOL_REQUEST Ptr
    EnterCriticalSection(@g_csQueue)
    Dim pReq As SYMBOL_REQUEST Ptr = g_pQueueHead
    If pReq Then
        g_pQueueHead = pReq->pNext
        If g_pQueueHead = 0 Then g_pQueueTail = 0
        ' 如果还有请求,重新触发事件,让线程继续处理
        If g_pQueueHead Then SetEvent(g_hQueueNotEmpty)
    End If
    LeaveCriticalSection(@g_csQueue)
    Return pReq
End Function

' ----------------------------------------------------------------------------
' 提交请求（支持同步等待）
' ----------------------------------------------------------------------------
Function SymbolActor_SubmitRequest(ByVal reqType As SymbolRequestType, _
                                   ByVal pModulePath As WString Ptr = 0, _
                                   ByVal pSymbolName As WString Ptr = 0, _
                                   ByVal address As ULONG64 = 0, _
                                   ByVal pStructName As WString Ptr = 0, _
                                   ByVal pMemberName As WString Ptr = 0, _
                                   ByVal pOutName As WString Ptr = 0, _
                                   ByVal nameLen As ULong = 0) As SYMBOL_REQUEST Ptr
    Dim pReq As SYMBOL_REQUEST Ptr = CAllocate(SizeOf(SYMBOL_REQUEST))
    pReq->reqType   = reqType
    If pModulePath  Then pReq->ModulePath = *pModulePath
    If pSymbolName  Then pReq->SymbolName = *pSymbolName
    If pStructName  Then pReq->StructName = *pStructName
    If pMemberName  Then pReq->MemberName = *pMemberName
    pReq->Address   = address
    pReq->pOutputName = pOutName
    pReq->NameLen   = nameLen
    pReq->hComplete = CreateEvent(NULL, FALSE, FALSE, NULL)  ' 自动重置

    Queue_Push(pReq)
    Return pReq   ' 调用者可 WaitForSingleObject 并释放
End Function

' ----------------------------------------------------------------------------
' SymbolThread 主循环
' ----------------------------------------------------------------------------
Function SymbolActor_Thread(ByVal p As Any Ptr) As DWORD
    MyLog.PrintInfo "SymbolActor_Thread",, "线程启动,等待请求..."

    ' 初始化符号引擎（在 Actor 线程内!）
    If Not SymEngine_InitInternal() Then
        MyLog.PrintLog LOG_ERROR, "SymbolActor_Thread", "SymEngine_InitInternal"
        Return 1
    End If

    Do
        WaitForSingleObject(g_hQueueNotEmpty, INFINITE)
        Dim pReq As SYMBOL_REQUEST Ptr = Queue_Pop()
        If pReq = 0 Then Continue Do

        Select Case pReq->reqType
            Case REQ_PRELOAD_WIN32KFULL
                Dim fullPath As WString * 260
                fullPath = Environ("SystemRoot") & "\System32\win32kfull.sys"
                Dim loadResult As ULONG64 = SymbolActor_LoadModuleInternal(@fullPath)
                If loadResult <> 0 Then
                    pReq->ResultStatus = STATUS_SUCCESS
                Else
                    pReq->ResultStatus = STATUS_UNSUCCESSFUL
                    MyLog.PrintLog LOG_ERROR, "SymbolActor_Thread", "SymbolActor_LoadModuleInternal"
                End If

            Case REQ_LOAD_MODULE
                pReq->ResultAddress = SymbolActor_LoadModuleInternal(@pReq->ModulePath)
                pReq->ResultStatus = STATUS_SUCCESS

            Case REQ_SYMBOL_TO_ADDR
                pReq->ResultAddress = SymbolActor_SymbolToAddrInternal(@pReq->ModulePath, @pReq->SymbolName)
                If pReq->ResultAddress Then
                    pReq->ResultStatus = STATUS_SUCCESS
                Else
                    pReq->ResultStatus = STATUS_NOT_FOUND
                End If

            Case REQ_ADDR_TO_SYMBOL
                If SymbolActor_AddrToSymbolInternal(@pReq->ModulePath, pReq->ADDRESS, pReq->pOutputName, pReq->NameLen) Then
                    pReq->ResultStatus = STATUS_SUCCESS
                Else
                    pReq->ResultStatus = STATUS_NOT_FOUND
                End If

            Case REQ_STRUCT_OFFSET
                pReq->ResultOffset = SymbolActor_GetStructOffsetInternal(@pReq->ModulePath, @pReq->StructName, @pReq->MemberName)
                If pReq->ResultOffset <> -1 Then
                    pReq->ResultStatus = STATUS_SUCCESS
                Else
                    pReq->ResultStatus = STATUS_NOT_FOUND
                End If

            Case REQ_SHUTDOWN
                SetEvent(pReq->hComplete)
                ' 不再 Deallocate,由调用者释放
                Exit Do
        End Select

        ' 通知完成
        If pReq->hComplete Then SetEvent(pReq->hComplete)
        ' 注意：不立即释放 pReq,因为调用者可能还在读取结果。
        ' 调用者负责在 WaitForSingleObject 后 Deallocate(pReq)
    Loop

    SymEngine_CleanupInternal()
    MyLog.PrintInfo "SymbolActor_Thread",, "线程退出"
    Return 0
End Function

' ----------------------------------------------------------------------------
' Actor 生命周期
' ----------------------------------------------------------------------------
Function SymbolActor_Init() As Boolean
    g_hQueueNotEmpty = CreateEvent(NULL, FALSE, FALSE, NULL)
    If g_hQueueNotEmpty = 0 Then Return False
    InitializeCriticalSection(@g_csQueue)

    g_hSymbolThread = CreateThread(NULL, 0, @SymbolActor_Thread, NULL, 0, NULL)
    If g_hSymbolThread = 0 Then
        DeleteCriticalSection(@g_csQueue)
        CloseHandle(g_hQueueNotEmpty)
        Return False
    End If
    Return True
End Function

Sub SymbolActor_Shutdown()
    Dim pReq As SYMBOL_REQUEST Ptr = SymbolActor_SubmitRequest(REQ_SHUTDOWN)
    If pReq Then
        WaitForSingleObject(pReq->hComplete, INFINITE)
        Deallocate(pReq)
    End If
    WaitForSingleObject(g_hSymbolThread, INFINITE)
    CloseHandle(g_hSymbolThread)
    g_hSymbolThread = 0
    DeleteCriticalSection(@g_csQueue)
    CloseHandle(g_hQueueNotEmpty)
End Sub

Function ExtractPercent(ByVal msg As WString Ptr) As Integer
    If msg = NULL Then Return -1

    Dim s As WString * 1024
    s = *msg

    Dim i As Integer
    Dim ch As UShort
    Dim num As Integer = 0
    Dim found As Boolean = False

    For i = 0 To Len(s) - 1
        ch = s[i]
        ' ASCII 0-9
        If ch >= Asc("0") AndAlso ch <= Asc("9") Then
            found = True
            num = 0
            ' 连续读取数字
            While i < Len(s)
                ch = s[i]
                If ch < Asc("0") OrElse ch > Asc("9") Then
                    Exit While
                End If
                num = num * 10 + (ch - Asc("0"))
                i += 1
            Wend
            Exit For
        End If
    Next

    If found Then
        If num < 0 Then num = 0
        If num > 100 Then num = 100
        Return num
    End If
    Return -1
End Function

' ----------------------------------------------------------------------------
' 模糊枚举回调的上下文
' ----------------------------------------------------------------------------
Type FuzzyEnumCtx
    symBase As ULONG64      ' 模块基址（用于计算偏移）
    count   As Integer      ' 匹配符号计数
End Type

' ----------------------------------------------------------------------------
' 模糊搜索回调
' ----------------------------------------------------------------------------
Function FuzzyEnumCallbackInternal( _
    ByVal pSymInfo As PSYMBOL_INFOW, _
    ByVal SymbolSize As ULONG, _
    ByVal UserContext As PVOID) As BOOL

    Dim ctx As FuzzyEnumCtx Ptr = Cast(FuzzyEnumCtx Ptr, UserContext)
    If ctx Then
        Dim offset As ULONG64 = pSymInfo->Address - ctx->symBase
        MyLog.PrintInfo "FuzzyEnumCallbackInternal",, "[Fuzzy] " & pSymInfo->Name & " @ Offset=0x" & Hex(offset, 8)
        ctx->count += 1
    End If
    Return True   ' 继续枚举
End Function

' ----------------------------------------------------------------------------
' 模糊符号枚举（R3 测试用）
' 参数：
'   modulePath  - 模块全路径（如 "C:\Windows\System32\win32kfull.sys"）
'   pattern     - 通配符模式（如 "*SetWinEventHook*"）
' 注意：必须在符号引擎已初始化后调用（比如在 SymbolActor_Init 之后）
' ----------------------------------------------------------------------------
Function FuzzySymbolSearch(ByVal modulePath As WString Ptr, ByVal pattern As WString Ptr) As Boolean
    MyLog.PrintInfo "FuzzySymbolSearch",, "模块 : " & *modulePath
    MyLog.PrintInfo "FuzzySymbolSearch",, "模式 : " & *pattern

    ' 加载模块（缓存机制,不会重复加载）
    Dim symBase As ULONG64 = SymbolActor_LoadModuleInternal(modulePath)
    If symBase = 0 Then
        MyLog.PrintLog LOG_ERROR, "FuzzySymbolSearch", "SymbolActor_LoadModuleInternal"
        Return False
    End If

    Dim ctx As FuzzyEnumCtx
    ctx.symBase = symBase
    ctx.count   = 0

    ' 执行搜索
    Dim result As BOOL = SymSearchW( _
        g_hSymProcess, _
        symBase, _
        0, _                ' Index = 0
        0, _                ' SymTag = 0（所有类型）
        pattern, _
        0, _                ' Address = 0
        @FuzzyEnumCallbackInternal, _
        @ctx, _
        SYMSEARCH_GLOBALSONLY)

    MyLog.PrintInfo "FuzzySymbolSearch", "SymSearchW", "找到" & ctx.count & "个符号"
    If result = 0 Then MyLog.PrintWin32Error "FuzzySymbolSearch", "SymSearchW"
    Return result
End Function