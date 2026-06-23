#include Once "win/winuser.bi"

' 进程汇总信息结构体（替代Dictionary）
Type Process_Info
    pid As DWORD           ' 进程ID
    name As WString * 260 ' 进程名称
    memory As SIZE_T      ' 内存占用（字节）
    is_hung As Boolean    ' 是否未响应
    window_count As Integer ' 窗口数量
End Type

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
        MyLog.PrintLog LOG_ERROR, "InitNtUserFunction", "DyLibLoad", "加载win32u.dll失败!"
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

/'Public Sub GetVisibleWindowList(lvwForegroundWindow As Class_ListView)
    Dim hWindow As hWnd = GetWindow(GetDesktopWindow, GW_CHILD)
    Dim i As Integer, wText As WString * 260, dwProcessId As DWORD, ProcessName As StringW, ProcessPath As StringW
    Dim szClass As WString * 260
    Dim windowStyle As Long
    
    'lvwForegroundWindow.DeleteAllItems
    
    InitListViewData lvwForegroundWindow
    
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
                
                GetWindowThreadProcessId hWindow, @dwProcessId
                ProcessPath = GetProcessPath(dwProcessId)
                ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                AddItemColListEx lvwForegroundWindow, 3,,, WStr(wText), WStr(ProcessName), WStr(IIf(IsHungAppWindow(hWindow), "未响应", "正在运行"))
                
                ' 可选：在第三列显示窗口类名，便于调试
                'lvwForegroundWindow.SetItemText i, 3, szClass
                
                i += 1
            End If
        End If
        hWindow = GetWindow(hWindow, GW_HWNDNEXT)
    Loop
    CommitListViewView lvwForegroundWindow
End Sub"'/

' ==============================================
' 函数1：在数组中查找进程ID（替代 Dictionary.ContainsKey）
' ==============================================
Function FindProcessIndex(arr() As Process_Info, pid As DWORD) As Integer
    Dim i As Integer
    For i = 0 To UBound(arr)
        If arr(i).pid = pid Then Return i
    Next
    Return -1 ' 未找到
End Function

' ==============================================
' 函数2：获取进程物理内存占用（工作集）
' ==============================================
Function GetProcessMemory(pid As DWORD) As SIZE_T
    Dim hProcess As HANDLE
    Dim pmc As PROCESS_MEMORY_COUNTERS
    
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, FALSE, pid)
    If hProcess = NULL Then Return 0
    
    pmc.cb = SizeOf(PROCESS_MEMORY_COUNTERS)
    GetProcessMemoryInfo(hProcess, @pmc, SizeOf(pmc))
    CloseHandle(hProcess)
    
    Return pmc.WorkingSetSize
End Function

' ==============================================
' 函数3：格式化内存（转为 MB/GB，和360一致）
' ==============================================
Function FormatMem(bytes As SIZE_T) As StringW
    If bytes = 0 Then Return "0.0 MB"
    
    Dim As Double size = bytes / 1024 / 1024 ' 转为MB
    If size >= 1024 Then
        size /= 1024
        Return TrimW(WStr(size)) & " GB"
    Else
        Return TrimW(WStr(Int(size * 10) / 10)) & " MB"
    End If
End Function

' ==============================================
' 核心：获取可见进程列表（进程级，无重复）
' ==============================================
Public Sub GetVisibleProcessList(lvwForegroundProcess As Class_ListView)
    Dim hWnd As hWnd = GetWindow(GetDesktopWindow(), GW_CHILD)
    Dim szClass As WString * 260
    Dim pid As DWORD, style As Long
    Dim path As WString * 260, strName As WString * 260
    Dim As Integer idx, count = -1
    Dim procList() As Process_Info ' 动态数组：存储汇总后的进程
    
    ' 清空列表
    Erase procList
    InitListViewData lvwForegroundProcess ' 初始化3列：进程名 | 内存 | 状态

    ' 第一步：枚举所有窗口 → 按进程汇总
    Do While hWnd <> NULL
        ' 过滤：顶层窗口 + 可见
        If GetWindow(hWnd, GW_OWNER) = NULL And IsWindowVisible(hWnd) Then
            GetClassName(hWnd, szClass, 255)
            style = GetWindowLong(hWnd, GWL_STYLE)
            
            ' 过滤系统无用窗口
            If Not( _
                szClass = "Shell_TrayWnd" Or szClass = "Progman" Or _
                szClass = "Button" Or szClass = "MSCTFIME UI" Or _
                szClass = "SysShadow" Or (style And WS_CHILD) = WS_CHILD _
            ) Then
                ' 获取进程ID
                GetWindowThreadProcessId(hWnd, @pid)
                idx = FindProcessIndex(procList(), PID)
                
                If idx = -1 Then
                    ' 新进程：加入数组
                    count += 1
                    ReDim Preserve procList(count) As ProcessInfo
                    procList(count).pid = pid
                    procList(count).memory = GetProcessMemory(PID)
                    procList(count).is_hung = FALSE
                    procList(count).window_count = 0
                    
                    ' 获取进程名称
                    path = GetProcessPath(pid)
                    strName = RightW(path, LenW(path) - InStrRevW(path, "\"))
                    If InStrW(strName, ".") Then strName = LeftW(strName, InStrW(strName, ".") - 1)
                    procList(count).Name = strName
                End If
                
                ' 更新进程状态
                procList(idx).window_count += 1
                If IsHungAppWindow(hWnd) Then procList(idx).is_hung = TRUE
            End If
        End If
        hWnd = GetWindow(hWnd, GW_HWNDNEXT)
    Loop

    ' 第二步：把汇总数据添加到ListView（和360完全一致）
    If count >= 0 Then
        For i As Integer = 0 To count
            Dim status As WString * 10
            If procList(i).is_hung Then
                status = "未响应"
            Else
                status = "正在运行"
            End If
            
            ' 添加3列：进程名 | 内存占用 | 运行状态
            AddItemColListEx lvwForegroundProcess, 3,,, _
                procList(i).Name, _
                WStr(FormatMem(procList(i).memory)), _
                status
        Next
    End If
    
    CommitListViewView lvwForegroundProcess
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
    
    memset @hWindows(0), 0, SizeOf(hWnd) * (UBound(hWindows) + 1)
    EnumWindows @EnumProc, Cast(lParam, ProcessId)
    WindowCount = 0
    
    InitListViewData lvwWindow
    
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
        ' 获取父窗口句柄
        Dim ParenthWnd As hWnd = GetParent(hWindows(i))
        ' 获取WndProc
        Dim pWndProc As LONG_PTR = GetClassLongPtr(hWindows(i), GCLP_WNDPROC) ' GetWindowLongPtr(hWindows(i), GWL_WNDPROC)
        'Print "0x" & WHex(pWndProc)
        '获取TID和所属模块
        Dim dwProcessId As DWORD
        Dim dwThreadId As DWORD = GetWindowThreadProcessId(hWindows(i), @dwProcessId)
        Dim strModulePath As WString * MAX_PATH
        strModulePath = GetProcessPath(dwProcessId)
        'Print "Window:" & "0x" & WHex(hWindows(i)) & " " & window_Class
        AddItemColListEx lvwWindow, 7,,, WStr("0x" & WHex(hWindows(i))), IIf(ParenthWnd <> NULL, "0x" & WHex(ParenthWnd), WStr("-")), WStr(window_Title), _
        "0x" & WHex(pWndProc), WStr(window_Class), WStr(dwThreadId), WStr(strModulePath)
        
        ChildWindowCount = 0
        memset @hChildWindows(0), 0, SizeOf(hWnd) * (UBound(hChildWindows) + 1)
        EnumChildWindows hWindows(i), @EnumChildProc, Cast(lParam, ProcessId)
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
            ' 获取父窗口句柄
            ParenthWnd = GetParent(hChildWindows(j))
            ' 获取WndProc
            pWndProc = GetClassLongPtr(hChildWindows(j), GCLP_WNDPROC) ' GetWindowLongPtr(hChildWindows(j), GWLP_WNDPROC)
            'Print "0x" & WHex(pWndProc)
            '获取TID和所属模块
            dwThreadId = GetWindowThreadProcessId(hChildWindows(j), @dwProcessId)
            Dim strModulePath As WString * MAX_PATH
            strModulePath = GetProcessPath(dwProcessId)
            'Print "Child Window:" & "0x" & WHex(hChildWindows(j)) & " " & window_Title & " " & window_Class & " " & dwThreadId & " " & strModulePath
            AddItemColListEx lvwWindow, 7,,, WStr("0x" & WHex(hChildWindows(j))), IIf(ParenthWnd <> NULL, "0x" & WHex(ParenthWnd), WStr("-")), WStr(window_Title), _
            "0x" & WHex(pWndProc), WStr(window_Class), WStr(dwThreadId), WStr(strModulePath)
        Next
        FF_DoEvents
    Next
    CommitListViewView lvwWindow
End Sub

Function MyCloseWindow(ByVal hWnd As HWND) As WINBOOL
    Return NtUserShowWindow(hWnd, SW_SHOWMINIMIZED)
End Function

Function MyDestroyWindow(ByVal hWnd As HWND) As WINBOOL
    Return NtUserDestroyWindow(hWnd)
End Function
