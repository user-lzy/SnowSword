#include "win/winsvc.bi"

'#define SC_ENUM_PROCESS_INFO 0

Private Function StripTerminator(sInput As StringW) As String
    Dim ZeroPos As Integer
    ZeroPos = InStrW(1, sInput, Chr(0))
    If ZeroPos > 0 Then
        StripTerminator = LeftW(sInput, ZeroPos - 1)
    Else
        StripTerminator = sInput
    End If
End Function

Private Function GetServiceType(ByVal ServiceType As DWORD) As StringW
    Dim strServiceType As StringW
    If ServiceType And SERVICE_KERNEL_DRIVER Then strServiceType = strServiceType & "Driver" ' 驱动程序服务
    If ServiceType And SERVICE_FILE_SYSTEM_DRIVER Then strServiceType = strServiceType & "FileDriver" ' 文件系统驱动程序服务
    'SERVICE_ADAPTER = &h00000004           ' 适配器服务
    'SERVICE_RECOGNIZER_DRIVER = &h00000008 ' 识别器驱动程序服务
    If ServiceType And SERVICE_WIN32_OWN_PROCESS Then ' Win32 服务（独立进程）
        If ServiceType And SERVICE_INTERACTIVE_PROCESS Then ' 交互式服务（必须与 SERVICE_WIN32_OWN_PROCESS 或 SERVICE_WIN32_SHARE_PROCESS 结合使用）
            If strServiceType <> "" Then strServiceType = strServiceType & "|OwnInteract" Else strServiceType = "OwnInteract"
        Else
            If strServiceType <> "" Then strServiceType = strServiceType & "|Own" Else strServiceType = "Own"
        End If
    End If
    If ServiceType And SERVICE_WIN32_SHARE_PROCESS Then ' Win32 服务（共享进程）
        If ServiceType And SERVICE_INTERACTIVE_PROCESS Then ' 交互式服务（必须与 SERVICE_WIN32_OWN_PROCESS 或 SERVICE_WIN32_SHARE_PROCESS 结合使用）
            If strServiceType <> "" Then strServiceType = strServiceType & "|ShareInteract" Else strServiceType = "Own"
        Else
            If strServiceType <> "" Then strServiceType = strServiceType & "|Share" Else strServiceType = "Share"
        End If
    End If
    Return strServiceType
End Function

Private Function GetServiceState(ByVal ServiceState As Long) As StringW
    Select Case ServiceState
        Case SERVICE_STOPPED
            GetServiceState = "Stopped"
        Case SERVICE_START_PENDING
            GetServiceState = "Starting"
        Case SERVICE_STOP_PENDING
            GetServiceState = "Stopping"
        Case SERVICE_RUNNING
            GetServiceState = "Running"
        Case SERVICE_CONTINUE_PENDING
            GetServiceState = "Continuing"
        Case SERVICE_PAUSE_PENDING
            GetServiceState = "Pausing"
        Case SERVICE_PAUSED
            GetServiceState = "Paused"
    End Select
End Function

Private Function GetServiceStartType(ByVal hSCM As SC_HANDLE, ByVal strServiceName As LPCWSTR) As StringW
    Dim hService As SC_HANDLE, lngBytesNeeded As Long, pConfig As QUERY_SERVICE_CONFIG Ptr
    GetServiceStartType = "Unknown"
    
    hService = OpenService(hSCM, strServiceName, SERVICE_QUERY_CONFIG)
    If hService = 0 Then Exit Function
    
    ' 第一次调用：获取所需缓冲区大小
    If QueryServiceConfig(hService, NULL, 0, @lngBytesNeeded) = 0 AndAlso GetLastError() = ERROR_INSUFFICIENT_BUFFER Then
        pConfig = Allocate(lngBytesNeeded)  ' 分配足够内存
        If pConfig <> NULL Then
            ' 第二次调用：真正查询数据
            If QueryServiceConfig(hService, pConfig, lngBytesNeeded, @lngBytesNeeded) <> 0 Then
                Select Case pConfig->dwStartType
                    Case SERVICE_BOOT_START: GetServiceStartType = "Boot Start"
                    Case SERVICE_SYSTEM_START: GetServiceStartType = "System Start"
                    Case SERVICE_AUTO_START: GetServiceStartType = "Automatic"
                    Case SERVICE_DEMAND_START: GetServiceStartType = "Manual"
                    Case SERVICE_DISABLED: GetServiceStartType = "Disabled"
                End Select
            End If
            Deallocate(pConfig)
        End If
    End If
    
    CloseServiceHandle(hService)
End Function

Private Function GetServiceModulePath(ByVal hSCM As SC_HANDLE, ByVal strServiceName As LPCWSTR) As StringW
    Dim hService As SC_HANDLE, lngBytesNeeded As Long, pConfig As QUERY_SERVICE_CONFIG Ptr
    Dim ModulePath As StringW = "-"
    
    hService = OpenService(hSCM, strServiceName, SERVICE_QUERY_CONFIG)
    If hService = 0 Then Exit Function
    
    ' 第一次调用：获取所需缓冲区大小
    If QueryServiceConfig(hService, NULL, 0, @lngBytesNeeded) = 0 AndAlso GetLastError() = ERROR_INSUFFICIENT_BUFFER Then
        pConfig = Allocate(lngBytesNeeded)  ' 分配足够内存
        If pConfig <> NULL Then
            ' 第二次调用：真正查询数据
            If QueryServiceConfig(hService, pConfig, lngBytesNeeded, @lngBytesNeeded) <> 0 Then
                If pConfig->lpBinaryPathName <> NULL Then
                    ' 直接复制字符串指针（已经在分配的内存块中）
                    ModulePath = *pConfig->lpBinaryPathName
                    ModulePath = YF_ReplaceW(ModulePath, WStr("\SystemRoot"), WStr(Environ("SystemRoot")))
                    ModulePath = YF_ReplaceW(ModulePath, WStr("\??\"), WStr(""))
                    If LenW(ModulePath) <> 0 AndAlso InStrW(ModulePath, ":") = 0 Then ModulePath = Environ("SystemRoot") & "\" & ModulePath
                    
                    'ModulePath = GetCorrectPathW(ModulePath)
                End If
            End If
            Deallocate(pConfig)
        End If
    End If
    
    CloseServiceHandle(hService)
    Function = ModulePath
End Function

Function SafeGetWString(ByVal pWStr As WString Ptr) As StringW
    Dim As WString Ptr p = pWStr
    Dim As StringW result = ""
    Dim i As Integer = 0
    
    If p = NULL Then Return result ' 如果返参是ByRef As Wstring会导致悬空引用,编译器会报错
    
    ' 按NULL终止符遍历，不依赖长度
    Do While p[i] <> 0
        result &= WChr(CInt(p[i]))  ' 逐个读取宽字符，直到NULL
        'p += 1
        i += 1
    Loop
    
    Return result
End Function

Public Sub GetServiceList1(lvwService As Class_ListView)
    Dim hSCM As SC_HANDLE, bytServices() As Byte, lpServices As ENUM_SERVICE_STATUS_PROCESS Ptr
    Dim lngBytesNeeded As Long, lngServicesReturned As Long, lngResumeHandle As Long
    Dim lngResult As Long, i As Long
    Dim strServiceName As WString * MAX_PATH = "", strDisplayName As WString * MAX_PATH = "", strState As StringW
    Dim strServiceType As StringW, strStartType As StringW, strPID As StringW
    Dim strProcessPath As StringW, strModulePath As StringW
    
    ' 1. 打开SCM管理器（枚举所有服务需要足够权限）
    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)
    If hSCM = NULL Then
        Print "OpenSCManager失败: " & GetLastError
        Exit Sub
    End If
    
    ' 2. 第一次调用EnumServicesStatusEx获取所需缓冲区大小
    lngResumeHandle = 0
    lngResult = EnumServicesStatusEx( _
        hSCM, _
        SC_ENUM_PROCESS_INFO, _
        SERVICE_WIN32 Or SERVICE_DRIVER, _
        SERVICE_ACTIVE Or SERVICE_INACTIVE, _
        0, _
        0, _
        @lngBytesNeeded, _
        @lngServicesReturned, _
        @lngResumeHandle, _
        NULL _
    )
    
    If GetLastError <> ERROR_MORE_DATA  Then
        Print "EnumServicesStatusEx第一步失败: " & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Exit Sub
    End If
    
    ' 3. 分配缓冲区并获取服务列表
    ReDim bytServices(lngBytesNeeded - 1) As Byte
    lngResult = EnumServicesStatusEx( _
        hSCM, _
        SC_ENUM_PROCESS_INFO, _
        SERVICE_WIN32 Or SERVICE_DRIVER, _
        SERVICE_ACTIVE Or SERVICE_INACTIVE, _
        @bytServices(0), _
        lngBytesNeeded, _
        @lngBytesNeeded, _
        @lngServicesReturned, _
        @lngResumeHandle, _
        NULL _
    )
    
    If lngResult = 0 Then
        Print "EnumServicesStatusEx第二步失败: " & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Exit Sub
    End If
    
    lpServices = CPtr(ENUM_SERVICE_STATUS_PROCESS Ptr, @bytServices(0))
    
    ' 4. 清空ListView并填充数据
    InitListViewData lvwService
    
    'Print lngServicesReturned
    For i = 0 To lngServicesReturned - 1
        ' 基础信息
        'Dim lpServiceName As StringW = *lpServices->lpServiceName
        Dim lpServiceName As WString * MAX_PATH = *lpServices->lpServiceName ' 这里有个问题,用StringW就崩
        'Print *lpServices->lpServiceName
        /'If lpServices->lpServiceName <> NULL Then
            'Dim lpServiceName As StringW = *lpServices->lpServiceName ' 这里有一个问题...
            strServiceName = *lpServices->lpServiceName
            Print strServiceName
            'Print strServiceName
            'strServiceName = StripTerminator(lpServiceName)
            'strServiceName = StripTerminator(strServiceName)
        End If'/
        Dim lpDisplayName As StringW = *lpServices->lpDisplayName
        /'If lpServices->lpDisplayName <> NULL Then
            'Dim lpDisplayName As StringW = *lpServices->lpDisplayName
            strDisplayName = *lpServices->lpDisplayName
            'Print lpDisplayName
            'Print strDisplayName
            'strDisplayName = StripTerminator(lpDisplayName)
            'lpDisplayName = StripTerminator(lpDisplayName)
        End If '/
        'Print StripTerminator(*lpServices->lpServiceName)
        strState = GetServiceState(lpServices->ServiceStatusProcess.dwCurrentState)
        strServiceType = GetServiceType(lpServices->ServiceStatusProcess.dwServiceType)
        
        ' 新增信息
        strStartType = GetServiceStartType(hSCM, lpServices->lpServiceName) ' 启动类型
        strPID = WStr(lpServices->ServiceStatusProcess.dwProcessId) ' 进程ID（驱动类为0）
        strProcessPath = GetProcessPath(ValULng(strPID)) ' 进程路径
        strModulePath = GetServiceModulePath(hSCM, lpServices->lpServiceName) ' 服务模块路径
        
        Dim ItemIndex As Long = AddItemColListEx(lvwService, 8,,, lpServiceName, WStr(strState), WStr(strServiceType), WStr(strStartType), WStr(strPID), WStr(strProcessPath), _
        WStr(strModulePath), WStr(lpDisplayName))
        VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", ItemIndex, strModulePath
        lpServices += 1
        FF_DoEvents
    Next
    
    ' 5. 释放资源
    CloseServiceHandle hSCM
    CommitListViewView lvwService
End Sub

Public Sub GetServiceList(lvwService As Class_ListView)
    ' 计时总起点
    Dim totalStart As Double = Timer()
    Dim segStart As Double, segFinish As Double
    Dim timeOpenSCM As Double = 0.0    ' 环节1耗时：打开SCM+第一次EnumServicesStatusEx
    Dim timeAllocBuf As Double = 0.0   ' 环节2耗时：重分配缓冲区+第二次EnumServicesStatusEx
    Dim timeListInit As Double = 0.0   ' 环节3耗时：清空ListView初始化
    Dim timeEnumLoop As Double = 0.0    ' 环节4总耗时：遍历所有服务循环处理
    Dim timeListCommit As Double = 0.0  ' 环节5耗时：关闭句柄+提交视图刷新
    
    Dim hSCM As SC_HANDLE, bytServices() As Byte, lpServices As ENUM_SERVICE_STATUS_PROCESS Ptr
    Dim lngBytesNeeded As Long, lngServicesReturned As Long, lngResumeHandle As Long
    Dim lngResult As Long, i As Long
    Dim strServiceName As WString * MAX_PATH = "", strDisplayName As WString * MAX_PATH = "", strState As StringW
    Dim strServiceType As StringW, strStartType As StringW, strPID As StringW
    Dim strProcessPath As StringW, strModulePath As StringW
    
    ' ====================== 环节1：打开SCM管理器、第一次Enum查询缓冲区大小 ======================
    segStart = Timer()
    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)
    If hSCM = NULL Then
        Print "OpenSCManager失败: " & GetLastError
        MyLog.PrintLog LOG_ERROR,,, "GetServiceList - OpenSCManager失败，错误码：" & GetLastError
        Exit Sub
    End If
    
    ' 第一次调用EnumServicesStatusEx获取所需缓冲区大小
    lngResumeHandle = 0
    lngResult = EnumServicesStatusEx( _
        hSCM, _
        SC_ENUM_PROCESS_INFO, _
        SERVICE_WIN32 Or SERVICE_DRIVER, _
        SERVICE_ACTIVE Or SERVICE_INACTIVE, _
        0, _
        0, _
        @lngBytesNeeded, _
        @lngServicesReturned, _
        @lngResumeHandle, _
        NULL _
    )
    
    If GetLastError <> ERROR_MORE_DATA  Then
        Print "EnumServicesStatusEx第一步失败: " & WinErrorMsg(GetLastError) & GetLastError
        MyLog.PrintLog LOG_ERROR,,, "GetServiceList - 首次枚举获取缓冲区失败：" & WinErrorMsg(GetLastError) & " 错误码=" & GetLastError
        CloseServiceHandle hSCM
        Exit Sub
    End If
    segFinish = Timer()
    timeOpenSCM = segFinish - segStart
    
    ' ====================== 环节2：分配缓冲区、完整枚举服务列表 ======================
    segStart = Timer()
    ReDim bytServices(lngBytesNeeded - 1) As Byte
    lngResult = EnumServicesStatusEx( _
        hSCM, _
        SC_ENUM_PROCESS_INFO, _
        SERVICE_WIN32 Or SERVICE_DRIVER, _
        SERVICE_ACTIVE Or SERVICE_INACTIVE, _
        @bytServices(0), _
        lngBytesNeeded, _
        @lngBytesNeeded, _
        @lngServicesReturned, _
        @lngResumeHandle, _
        NULL _
    )
    
    If lngResult = 0 Then
        Print "EnumServicesStatusEx第二步失败: " & WinErrorMsg(GetLastError) & GetLastError
        MyLog.PrintLog LOG_ERROR,,, "GetServiceList - 二次枚举服务列表失败：" & WinErrorMsg(GetLastError) & " 错误码=" & GetLastError
        CloseServiceHandle hSCM
        Exit Sub
    End If
    
    lpServices = CPtr(ENUM_SERVICE_STATUS_PROCESS Ptr, @bytServices(0))
    segFinish = Timer()
    timeAllocBuf = segFinish - segStart
    
    ' ====================== 环节3：清空ListView初始化 ======================
    segStart = Timer()
    InitListViewData lvwService
    segFinish = Timer()
    timeListInit = segFinish - segStart
    
    ' ====================== 环节4：循环遍历所有服务、解析字段、填充列表 ======================
    segStart = Timer()
    For i = 0 To lngServicesReturned - 1
        ' 基础信息
        Dim lpServiceName As WString * MAX_PATH = *lpServices->lpServiceName
        strServiceName = *lpServices->lpServiceName
        
        Dim lpDisplayName As StringW = *lpServices->lpDisplayName
        strDisplayName = *lpServices->lpDisplayName
        
        strState = GetServiceState(lpServices->ServiceStatusProcess.dwCurrentState)
        strServiceType = GetServiceType(lpServices->ServiceStatusProcess.dwServiceType)
        
        ' 新增信息
        strStartType = GetServiceStartType(hSCM, lpServices->lpServiceName) ' 启动类型
        strPID = WStr(lpServices->ServiceStatusProcess.dwProcessId) ' 进程ID（驱动类为0）
        strProcessPath = GetProcessPath(ValULng(strPID)) ' 进程路径
        strModulePath = GetServiceModulePath(hSCM, lpServices->lpServiceName) ' 服务模块路径
        
        Dim ItemIndex As Long = AddItemColListEx(lvwService, 8,,, lpServiceName, WStr(strState), WStr(strServiceType), WStr(strStartType), WStr(strPID), WStr(strProcessPath), _
        WStr(strModulePath), WStr(lpDisplayName))
        ' 异步校验签名（异步逻辑不计入单服务同步耗时）
        VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", ItemIndex, strModulePath
        lpServices += 1
        FF_DoEvents
    Next
    segFinish = Timer()
    timeEnumLoop = segFinish - segStart
    
    ' ====================== 环节5：释放SCM句柄、提交ListView视图刷新 ======================
    segStart = Timer()
    CloseServiceHandle hSCM
    CommitListViewView lvwService
    segFinish = Timer()
    timeListCommit = segFinish - segStart
    
    ' ====================== 统一输出各分段耗时、总耗时、单服务平均耗时 ======================
    Dim totalCost As Double = Timer() - totalStart
    Dim avgSingleService As Double = 0.0
    If lngServicesReturned > 0 Then
        ' 仅用循环总耗时计算单条平均（排除SCM打开、缓冲区、列表初始化、视图提交）
        avgSingleService = timeEnumLoop / lngServicesReturned
    End If
    
    ' 分段日志输出，保留4位小数
    MyLog.PrintLog LOG_INFO,,, "===== GetServiceList 分段耗时统计 ====="
    MyLog.PrintLog LOG_INFO,,, "1.打开SCM+首次枚举缓冲区：" & Format(timeOpenSCM, "0.0000") & " s"
    MyLog.PrintLog LOG_INFO,,, "2.分配缓冲区+全量枚举服务：" & Format(timeAllocBuf, "0.0000") & " s"
    MyLog.PrintLog LOG_INFO,,, "3.ListView清空初始化：" & Format(timeListInit, "0.0000") & " s"
    MyLog.PrintLog LOG_INFO,,, "4.遍历解析所有服务循环总耗时：" & Format(timeEnumLoop, "0.0000") & " s"
    MyLog.PrintLog LOG_INFO,,, "5.关闭句柄+提交列表视图：" & Format(timeListCommit, "0.0000") & " s"
    MyLog.PrintLog LOG_INFO,,, "服务总数：" & lngServicesReturned & " 条"
    MyLog.PrintLog LOG_INFO,,, "单条服务平均解析耗时(仅循环同步逻辑)：" & Format(avgSingleService, "0.000000") & " s"
    MyLog.PrintLog LOG_INFO,,, "函数整体执行总耗时：" & Format(totalCost, "0.0000") & " 秒"
    MyLog.PrintLog LOG_INFO,,, "======================================"
End Sub

Function MyPauseService(ByVal ServiceName As StringW) As Boolean
    Dim hSCM As SC_HANDLE, hService As SC_HANDLE, ServiceStatus As SERVICE_STATUS
    
    Function = True

    hSCM = OpenSCManager("", SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)
    If hSCM = NULL Then
        Print "[MyPauseService]OpenSCManager(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS)
    If hService = NULL Then
        Print "[MyPauseService]OpenService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Return False
    End If
    
    If ControlService(hService, SERVICE_CONTROL_PAUSE, @ServiceStatus) = 0 Then
        Print "[MyPauseService]ControlService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Function = False
    End If
    
    CloseServiceHandle hService
    CloseServiceHandle hSCM
End Function

Function MyContinueService(ByVal ServiceName As StringW) As Boolean
    Dim hSCM As SC_HANDLE, hService As SC_HANDLE, ServiceStatus As SERVICE_STATUS
    
    Function = True

    hSCM = OpenSCManager("", SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)
    If hSCM = NULL Then
        Print "[MyContinueService]OpenSCManager(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS)
    If hService = NULL Then
        Print "[MyContinueService]OpenService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Return False
    End If
    
    If ControlService(hService, SERVICE_CONTROL_CONTINUE, @ServiceStatus) = 0 Then
        Print "[MyContinueService]ControlService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Function = False
    End If
    
    CloseServiceHandle hService
    CloseServiceHandle hSCM
End Function

Function MyStartService(ByVal ServiceName As StringW) As Boolean
    Dim hSCM As SC_HANDLE, hService As SC_HANDLE, ServiceStatus As SERVICE_STATUS
    
    Function = True

    hSCM = OpenSCManager("", SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)
    If hSCM = NULL Then
        Print "[MyStartService]OpenSCManager(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS)
    If hService = NULL Then
        Print "[MyStartService]OpenService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Return False
    End If
    
    If StartService(hService, 0, NULL) = 0 Then
        Print "[MyStartService]ControlService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Function = False
    End If
    
    CloseServiceHandle hService
    CloseServiceHandle hSCM
End Function

Function MyStopService(ByVal ServiceName As StringW) As Boolean
    Dim hSCM As SC_HANDLE, hService As SC_HANDLE, ServiceStatus As SERVICE_STATUS
    
    Function = True

    hSCM = OpenSCManager("", SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)
    If hSCM = NULL Then
        Print "[MyStopService]OpenSCManager(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    hService = OpenService(hSCM, ServiceName, SERVICE_ALL_ACCESS)
    If hService = NULL Then
        Print "[MyStopService]OpenService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        CloseServiceHandle hSCM
        Return False
    End If
    
    If ControlService(hService, SERVICE_CONTROL_STOP, @ServiceStatus) = 0 Then
        Print "[MyStopService]ControlService(" & ServiceName & "):" & WinErrorMsg(GetLastError) & GetLastError
        Function = False
    End If
    
    CloseServiceHandle hService
    CloseServiceHandle hSCM
End Function