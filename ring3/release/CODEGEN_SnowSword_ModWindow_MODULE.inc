'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModWindow.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/winuser.bi" '已经搬到定义文件处

'10010100000000000000000000000000 异常

'00010100000000000000000000000000 正常

Private Function GetPIDFromHWND(ByVal hWnd As HWND) As DWORD
    Dim dwProcessId As DWORD
    GetWindowThreadProcessId hWnd, @dwProcessId
    Return dwProcessId
End Function

Public Sub GetVisibleWindowList(lvwForegroundWindow As Class_ListView)
    Dim hWindow As hWnd = GetWindow(GetDesktopWindow, GW_CHILD)
    Dim i As Integer, wText As WString * 260, dwProcessId As DWORD, ProcessName As String, ProcessPath As String
    Dim szClass As WString * 260
    Do While (hWindow <> 0)
        If ((GetWindow(hWindow, GW_OWNER) = NULL) And (IsWindowVisible(hWindow))) Then
            GetClassName hWindow, szClass, 256
            If  (Not((szClass = "Shell_TrayWnd") Or (szClass = "Progman"))) Then '滤掉桌面与任务栏
                GetWindowTextW hWindow, @wText, 100
                lvwForegroundWindow.AddItem wText
                GetWindowThreadProcessId hWindow, @dwProcessId
                'AfxMsg dwProcessId
                'GetProcessPathAndName dwProcessId, ProcessName, ProcessPath
                'AfxMsg ProcessName
                lvwForegroundWindow.SetItemText i, 1, *GetWString(ProcessName)
                lvwForegroundWindow.SetItemText i, 2, IIf(IsHungAppWindow(hWindow), "未响应", "正在运行")
                i += 1
            End If
        End If
        hWindow = GetWindow(hWindow, GW_HWNDNEXT)
    Loop
End Sub