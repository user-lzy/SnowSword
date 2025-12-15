#include Once "win/winuser.bi"

Dim Shared hWindows() As HWND
Dim Shared hChildWindows() As HWND
Dim Shared hwin32uDll As Any Ptr
Dim Shared WindowCount As DWORD
Dim Shared ChildWindowCount As DWORD

'Declare Sub SetWindowTop Lib "user32"(ByVal hWin As hWnd, ByVal state As Integer)
Dim Shared NtUserShowWindow As Function(ByVal hWnd As hWnd, ByVal nCmdShow As Integer) As WINBOOL
Dim Shared NtUserDestroyWindow As Function(ByVal hWnd As hWnd) As WINBOOL

Sub InitNtUserFunction()
    hwin32uDll = DyLibLoad("win32u.dll")
    If (hwin32uDll = NULL) Then
        Print "加载win32u.dll失败!" & Err
        Exit Sub
    End If
    NtUserShowWindow = DyLibSymbol(hwin32uDll, "NtUserShowWindow")
    NtUserDestroyWindow = DyLibSymbol(hwin32uDll, "NtUserDestroyWindow")
End Sub

Sub UninitNtUserFunction()
    DyLibFree hwin32uDll
End Sub

Private Function GetPIDFromHWND(ByVal hWnd As HWND) As DWORD
    Dim dwProcessId As DWORD
    GetWindowThreadProcessId hWnd, @dwProcessId
    Return dwProcessId
End Function

Public Sub GetVisibleWindowList(lvwForegroundWindow As Class_ListView)
    Dim hWindow As hWnd = GetWindow(GetDesktopWindow, GW_CHILD)
    Dim i As Integer, wText As WString * 260, dwProcessId As DWORD, ProcessName As StringW, ProcessPath As StringW
    Dim szClass As WString * 260
    Dim windowStyle As Long
    
    Do While (hWindow <> 0)
        ' 检查窗口是否可见并且没有所有者窗口
        If ((GetWindow(hWindow, GW_OWNER) = NULL) And (IsWindowVisible(hWindow))) Then
            GetClassName hWindow, szClass, 256
            
            ' 获取窗口样式以进一步过滤
            windowStyle = GetWindowLong(hWindow, GWL_STYLE)
            
            ' 扩展过滤条件，但仍然保留无标题窗口
            If Not((szClass = "Shell_TrayWnd") Or (szClass = "Progman") Or _
                   (szClass = "Button") Or (szClass = "MSCTFIME UI") Or _
                   (szClass = "SysShadow") Or (szClass = "Cursor") Or _
                   (windowStyle And WS_CHILD) = WS_CHILD) Then
                
                GetWindowTextW hWindow, @wText, 100
                
                ' 处理无标题窗口
                If LenW(wText) = 0 Then
                    ' 尝试获取进程名作为窗口标识
                    GetWindowThreadProcessId hWindow, @dwProcessId
                    ProcessPath = GetProcessPath(dwProcessId)
                    ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                    
                    ' 移除文件扩展名
                    If InStrW(ProcessName, ".") > 0 Then
                        ProcessName = LeftW(ProcessName, InStrW(ProcessName, ".") - 1)
                    End If
                    
                    ' 使用进程名 + 窗口句柄作为显示文本
                    wText = ProcessName & " [" & Hex(hWindow) & "]"
                End If
                
                lvwForegroundWindow.AddItem wText
                GetWindowThreadProcessId hWindow, @dwProcessId
                ProcessPath = GetProcessPath(dwProcessId)
                ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                lvwForegroundWindow.SetItemText i, 1, ProcessName
                lvwForegroundWindow.SetItemText i, 2, IIf(IsHungAppWindow(hWindow), "未响应", "正在运行")
                
                ' 可选：在第三列显示窗口类名，便于调试
                'lvwForegroundWindow.SetItemText i, 3, szClass
                
                i += 1
            End If
        End If
        hWindow = GetWindow(hWindow, GW_HWNDNEXT)
    Loop
End Sub

' 回调函数
Private Function EnumProc(ByVal hWnd As HWND, ByVal lParam As LPARAM) As WINBOOL
    '存储窗口句柄
    Dim ProcessId As DWORD = Cast(DWORD, lParam)
    If GetPIDFromHWND(hWnd) = ProcessId Then
        'Print hWnd
        If WindowCount > UBound(hWindows) Then ReDim Preserve hWindows(WindowCount + 10) As hWnd
        hWindows(WindowCount) = hWnd
        WindowCount += 1
    End If
    ' 继续枚举
    EnumProc = True
End Function

' 回调函数
Private Function EnumChildProc(ByVal hWnd As HWND, ByVal lParam As LPARAM) As WINBOOL
    '存储窗口句柄
    Dim ProcessId As DWORD = Cast(DWORD, lParam)
    If GetPIDFromHWND(hWnd) = ProcessId Then
        'Print hWnd
        If ChildWindowCount > UBound(hChildWindows) Then ReDim Preserve hChildWindows(ChildWindowCount + 10) As hWnd
        hChildWindows(ChildWindowCount) = hWnd
        ChildWindowCount += 1
    End If
    ' 继续枚举
    EnumChildProc = True
End Function

Public Sub GetWindowList(ByVal ProcessId As DWORD, lvwWindow As Class_ListView)
    ' 枚举所有子窗口
    Dim window_Title As WString * MAX_PATH
    Dim window_Class As WString * MAX_PATH
    Dim titleLength As Long
    Dim classLength As Long
    'Dim PID As DWORD
    
    memset @hWindows(0), 0, SizeOf(hWnd) * (UBound(hWindows) + 1)
    EnumWindows @EnumProc, Cast(lParam, ProcessId)
    lvwWindow.DeleteAllItems
    WindowCount = 0
    For i As Integer = 0 To UBound(hWindows)
        If hWindows(i) = 0 Then Continue For
        ' 获取窗口标题
        titleLength = GetWindowTextLength(hWindows(i))
        window_Title = Wstring(titleLength + 1, 0)
        GetWindowText hWindows(i), @window_Title, titleLength + 1
        window_Title = Left(window_Title, titleLength)

        ' 获取窗口类名
        window_Class = Wstring(MAX_PATH, 0)
        GetClassName hWindows(i), @window_Class, MAX_PATH
        window_Class = Left(window_Class, MAX_PATH - 1)
        Print "0x" & WHex(hWindows(i)) & " " & window_Title & " " & window_Class
        '获取TID和所属模块
        Dim dwProcessId As DWORD
        Dim dwThreadId As DWORD = GetWindowThreadProcessId(hWIndows(i), @dwProcessId)
        Print dwProcessId & " " & dwThreadId
        Print ""
        'Dim ThreadStartAddress As PVOID = GetThreadStartAddr(dwThreadId)
        Dim strModulePath As WString * MAX_PATH
        'PID = GetPIDFromHWND(hWindows(i))
        strModulePath = GetProcessPath(dwProcessId)
        'GetModuleByAddress dwProcessId, ThreadStartAddress, @strModulePath
        'GetWindowModuleFileName hWindows(i), @strModulePath, MAX_PATH
        
        lvwWindow.AddItemColList,, "0x" & WHex(hWindows(i)), window_Title, window_Class, WStr(dwThreadId), strModulePath
        
        ChildWindowCount = 0
        memset @hChildWindows(0), 0, SizeOf(hWnd) * (UBound(hChildWindows) + 1)
        EnumChildWindows hWindows(i), @EnumChildProc, Cast(lParam, ProcessId)
        'Dim ItemIndex As Integer = lvwWindow.ItemCount
        For j As Integer = 0 To UBound(hChildWindows)
            If hChildWindows(j) = NULL Then Continue For
            ' 获取窗口标题
            titleLength = GetWindowTextLength(hChildWindows(j))
            window_Title = Wstring(titleLength + 1, 0)
            GetWindowText hChildWindows(j), @window_Title, titleLength + 1
            window_Title = Left(window_Title, titleLength)
            
            ' 获取窗口类名
            window_Class = Wstring(MAX_PATH, 0)
            GetClassName hChildWindows(j), @window_Class, MAX_PATH
            window_Class = Left(window_Class, MAX_PATH - 1)
            Print "0x" & WHex(hChildWindows(j)) & " " & window_Title & " " & window_Class
            '获取TID和所属模块
            dwThreadId = GetWindowThreadProcessId(hChildWindows(j), @dwProcessId)
            Print dwProcessId & " " & dwThreadId
            Print ""
            'Dim ThreadStartAddress As PVOID = GetThreadStartAddr(dwThreadId)
            Dim strModulePath As WString * MAX_PATH
            'GetModuleByAddress dwProcessId, ThreadStartAddress, @strModulePath
            'PID = GetPIDFromHWND(hChildWindows(j))
            strModulePath = GetProcessPath(dwProcessId)
            'GetWindowModuleFileName hChildWindows(j), @strModulePath, MAX_PATH
            
            lvwWindow.AddItemColList,, "0x" & WHex(hChildWindows(j)), window_Title, window_Class, WStr(dwThreadId), strModulePath
        Next
    Next
End Sub

Function MyCloseWindow(ByVal hWnd As HWND) As WINBOOL
    Return NtUserShowWindow(hWnd, SW_SHOWMINIMIZED)
End Function

Function MyDestroyWindow(ByVal hWnd As HWND) As WINBOOL
    Return NtUserDestroyWindow(hWnd)
End Function
