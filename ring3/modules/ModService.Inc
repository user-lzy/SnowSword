#include "win/winsvc.bi"

Private Function StripTerminator(sInput As String) As String
    Dim ZeroPos As Integer
    ZeroPos = InStr(1, sInput, Chr(0))
    If ZeroPos > 0 Then
        StripTerminator = Left(sInput, ZeroPos - 1)
    Else
        StripTerminator = sInput
    End If
End Function

Private Function GetServiceType(ByVal ServiceType As DWORD) As StringW
    Dim strServiceType As StringW
    If ServiceType And SERVICE_KERNEL_DRIVER Then strServiceType = strServiceType & "Driver" & "|" ' 驱动程序服务
    If ServiceType And SERVICE_FILE_SYSTEM_DRIVER Then strServiceType = strServiceType & "FileDriver" & "|" ' 文件系统驱动程序服务
    'SERVICE_ADAPTER = &h00000004           ' 适配器服务
    'SERVICE_RECOGNIZER_DRIVER = &h00000008 ' 识别器驱动程序服务
    If ServiceType And SERVICE_WIN32_OWN_PROCESS Then ' Win32 服务（独立进程）
        If ServiceType And SERVICE_INTERACTIVE_PROCESS Then ' 交互式服务（必须与 SERVICE_WIN32_OWN_PROCESS 或 SERVICE_WIN32_SHARE_PROCESS 结合使用）
            strServiceType = strServiceType & "OwnInteract"
        Else
            strServiceType = strServiceType & "Own"
        End If
    End If
    If ServiceType And SERVICE_WIN32_SHARE_PROCESS Then ' Win32 服务（共享进程）
        If ServiceType And SERVICE_INTERACTIVE_PROCESS Then ' 交互式服务（必须与 SERVICE_WIN32_OWN_PROCESS 或 SERVICE_WIN32_SHARE_PROCESS 结合使用）
            strServiceType = strServiceType & "ShareInteract"
        Else
            strServiceType = strServiceType & "Share"
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

Public Sub GetServiceList(lvwService As Class_ListView)
    Dim hSCM As SC_HANDLE, lpEnumServiceStatus() As ENUM_SERVICE_STATUS, lngServiceStatusInfoBuffer As Long
    Dim strServiceName As StringW, lngBytesNeeded As DWORD, lngServicesReturned As DWORD
    Dim hNextUnreadEntry As DWORD, lngStructsNeeded As DWORD, lngResult As Long
    hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)
    If hSCM = 0 Then
        Print "OpenSCManager failed." & GetLastError
        Exit Sub
    End If
    hNextUnreadEntry = 0
    lngResult = EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_ACTIVE Or SERVICE_INACTIVE, 0, 0, @lngBytesNeeded, @lngServicesReturned, @hNextUnreadEntry)
    If GetLastError <> ERROR_MORE_DATA Then
        Print "EnumServicesStatus failed1." & GetLastError
        Exit Sub
    End If
    lngStructsNeeded = lngBytesNeeded / Len(lpEnumServiceStatus(0)) + 1
    ReDim lpEnumServiceStatus(lngStructsNeeded - 1)
    lngServiceStatusInfoBuffer = lngStructsNeeded * Len(lpEnumServiceStatus(0))
    hNextUnreadEntry = 0
    lngResult = EnumServicesStatus(hSCM, SERVICE_WIN32, SERVICE_ACTIVE Or SERVICE_INACTIVE, @lpEnumServiceStatus(0), lngServiceStatusInfoBuffer, @lngBytesNeeded, @lngServicesReturned, @hNextUnreadEntry)
    If lngResult = 0 Then
        Print "EnumServicesStatus failed2." & GetLastError
        Exit Sub
    End If
    For i As DWORD = 0 To lngServicesReturned - 1
        lvwService.AddItem StripTerminator(*lpEnumServiceStatus(i).lpServiceName)
        lvwService.SetItemText i, 1, GetServiceState(lpEnumServiceStatus(i).ServiceStatus.dwCurrentState)
        lvwService.SetItemText i, 2, GetServiceType(lpEnumServiceStatus(i).ServiceStatus.dwServiceType)
        lvwService.SetItemText i, 3, StripTerminator(*lpEnumServiceStatus(i).lpDisplayName)
    Next
    CloseServiceHandle hSCM
End Sub

/'Public Function ContinueService(ByVal ServiceName As String) As Long
    
    Dim hService As Long, ServiceStatus As SERVICE_STATUS, lhService As Long
    
    hService = OpenSCManager(vbNullString, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)
    
    If hService = 0 Then Exit Function
    
    lhService = OpenService(hService, ServiceName, SERVICE_ALL_ACCESS)
    
    If lhService <> 0 Then
        
        ContinueService = ControlService(lhService, SERVICE_CONTROL_CONTINUE, ServiceStatus)
        
        If ContinueService = 0 Then MyMsgBox GetLastError
        
        CloseServiceHandle lhService
        
    End If
    
    CloseServiceHandle hService
    
End Function

Public Function PauseService(ByVal ServiceName As String) As Long
    Dim hService As Long, ServiceStatus As SERVICE_STATUS, hSCManager As Long
    hSCManager = OpenSCManager(vbNullString, vbNullString, SC_MANAGER_CONNECT)
    If hSCManager = 0 Then Exit Function
    hService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS)
    If hService <> 0 Then
        PauseService = ControlService(hService, SERVICE_CONTROL_PAUSE, ServiceStatus)
        If PauseService = 0 Then MyMsgBox GetLastError
        CloseServiceHandle hService
    End If
    CloseServiceHandle hSCManager
End Function

Public Function StartService(ByVal ServiceName As String) As Long
    Dim hService As Long, ServiceStatus As SERVICE_STATUS, hSCManager As Long
    hSCManager = OpenSCManager(vbNullString, vbNullString, SC_MANAGER_CONNECT)
    
    If hSCManager = 0 Then Exit Function
    
    hService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS)
    
    If hService <> 0 Then
        
        StartService = StartServiceA(0, 0, hService)
        
        If StartService = 0 Then MyMsgBox GetLastError
        
        CloseServiceHandle hService
        
    End If
    
    CloseServiceHandle hSCManager
    
End Function

Public Function StopService(ByVal ServiceName As String) As Long
    
    Dim hService As Long, ServiceStatus As SERVICE_STATUS, hSCManager As Long
    
    hSCManager = OpenSCManager(vbNullString, vbNullString, SC_MANAGER_CONNECT)
    
    If hSCManager = 0 Then Exit Function
    
    hService = OpenService(hSCManager, ServiceName, SERVICE_ALL_ACCESS)
    
    If hService <> 0 Then
        
        StopService = ControlService(hService, SERVICE_CONTROL_STOP, ServiceStatus)
        
        If StopService = 0 Then MyMsgBox GetLastError
        
        CloseServiceHandle hService
        
    End If
    
    CloseServiceHandle hSCManager
    
End Function'/