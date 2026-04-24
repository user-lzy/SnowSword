' 消息钩子结构体（对应内核 WIN32K_MSG_HOOK_INFO）
Type WIN32K_MSG_HOOK_INFO
    HookHandle      As ULONG64   ' 钩子句柄
    HookType    As DWORD      ' 子类型(WH_MOUSE/LL等)
    HookFlags       As DWORD     ' 钩子标志
    IsGlobal        As Boolean   ' 是否全局
    pfn        As ULONG64 ' 回调地址(RVA/VA)
    'hMod            As ULONG64 ' 模块句柄/索引
    ModulePath      As Wstring * 260 ' 模块路径
    ProcessId       As DWORD     ' PID
    ThreadId        As DWORD     ' TID
End Type

' 事件钩子结构体（对应内核 WIN32K_WINEVENT_HOOK_INFO）
Type WIN32K_WINEVENT_HOOK_INFO
    HookHandle      As ULONG64
    EventMin        As DWORD
    EventMax        As DWORD
    hmodWinEventProc As ULONG64
    pfnWinEventProc As ULONG64
    ActualCallback  As ULONG64
    ModulePath As Wstring * 260
    TargetProcessId       As DWORD
    TargetThreadId        As DWORD
    ProcessId       As DWORD
    ThreadId        As DWORD
    dwFlags           As DWORD
End Type

' 热键结构体（对应内核 WIN32K_HOTKEY_INFO）
Type WIN32K_HOTKEY_INFO
    HotkeyHandle As HANDLE ' 热键句柄
    vk As DWORD                  ' 虚拟键码
    uMod As DWORD                 ' 功能键
    id As DWORD                  ' 热键id
    hWnd As ULONG64               ' 窗口句柄
    ProcessId As DWORD           ' 进程ID
    ThreadId As DWORD            ' 线程ID
    pfn As ULONG64                  ' 回调函数
End Type

' ====================== 获取钩子类型名称 ======================
' 对应 Win32 标准 WH_* 钩子ID
Private Function GetHookTypeName(ByVal hookType As Long) As StringW
    Select Case hookType
        Case -1: Return "WH_MSGFILTER"
        Case 0: Return "WH_JOURNALRECORD"
        Case 1: Return "WH_JOURNALPLAYBACK"
        Case 2: Return "WH_KEYBOARD"
        Case 3: Return "WH_GETMESSAGE"
        Case 4: Return "WH_CALLWNDPROC"
        Case 5: Return "WH_CBT"
        Case 6: Return "WH_SYSMSGFILTER"
        Case 7: Return "WH_MOUSE"
        Case 8: Return "WH_HARDWARE"
        Case 9: Return "WH_DEBUG"
        Case 10: Return "WH_SHELL"
        Case 11: Return "WH_FOREGROUNDIDLE"
        Case 12: Return "WH_CALLWNDPROCRET"
        Case 13: Return "WH_KEYBOARD_LL"
        Case 14: Return "WH_MOUSE_LL"
        Case Else: Return "未知钩子"
    End Select
End Function

' 辅助函数：转换热键修饰键为字符串（你原代码没有，我补充，对齐体验）
Function GetHotkeyModStr(ByVal modFlags As DWORD) As StringW
    Dim sMod As StringW = ""
    If (modFlags And &H1) Then sMod &= "Alt+"    ' MOD_ALT
    If (modFlags And &H2) Then sMod &= "Ctrl+"   ' MOD_CONTROL
    If (modFlags And &H4) Then sMod &= "Shift+"  ' MOD_SHIFT
    If (modFlags And &H8) Then sMod &= "Win+"    ' MOD_WIN
    If sMod <> "" Then sMod = LeftW(sMod, LenW(sMod)-1) ' 去掉最后一个+
    Return sMod
End Function

' 辅助函数：转换虚拟键码为字符串（简化版，可自行扩展）
Function GetVkKeyStr(ByVal vkCode As DWORD) As StringW
    Select Case vkCode
        Case &H30 To &H39 : Return Chr(vkCode)
        Case &H41 To &H5A : Return Chr(vkCode)
        Case &H70 To &H87 : Return "F" & (vkCode - &H6F)
        Case &H0 : Return ""
        Case &H20 : Return "Space"
        Case &H0D : Return "Enter"
        Case &H1B : Return "Esc"
        Case &H09 : Return "Tab"
        Case Else : Return "VK_0x" & Hex(vkCode)
    End Select
End Function

' 消息钩子枚举 - 改造版（PID列动态显示/隐藏）
Public Sub GetMsgHookList(ByVal PID As DWORD, lvwMsgHook As Class_ListView)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    
    Dim ret As Long, lpRet As DWORD
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' 第一次IOCTL：获取缓冲区大小
    IoControl hDrv, IOCTL_EnumMsgHook, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetMsgHookList] 第一次IOCTL失败: " & WinErrorMsg(GetLastError)
        Exit Sub
    End If

    dataSize = lpRet
    If dataSize = 0 Then
        Print "未发现任何消息钩子"
        'InitListViewData lvwMsgHook
        '' ====================== 消息钩子 PID列处理 ======================
        '' PID列索引：5 | 列名：PID
        'Const MSGHOOK_PID_COL_INDEX = 5
        'If PID <> Cast(DWORD, -1) Then
        '    ' 删除前校验：确保该列是PID列
        '    If lvwMsgHook.ColumnText(MSGHOOK_PID_COL_INDEX) = "PID" Then
        '        lvwMsgHook.DeleteColumn(MSGHOOK_PID_COL_INDEX)
        '    End If
        'Else
        '    ' 添加前校验：确保PID列不存在
        '    If lvwMsgHook.ColumnText(MSGHOOK_PID_COL_INDEX) <> "PID" Then
        '        lvwMsgHook.InsertColumn(MSGHOOK_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
        '    End If
        'End If
        '' =================================================================
        'CommitListViewView lvwMsgHook
        Exit Sub
    End If

    ' 分配内存
    pData = Allocate(dataSize)
    If pData = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If

    ' 第二次IOCTL：获取钩子数据
    IoControl hDrv, IOCTL_EnumMsgHook, NULL, 0, pData, dataSize, @lpRet
    If GetLastError <> 0 Then
        Print "[GetMsgHookList] 第二次IOCTL失败: " & WinErrorMsg(GetLastError)
        Deallocate pData
        Exit Sub
    End If

    ' 解析数据
    Dim hookCount As ULong = dataSize / SizeOf(WIN32K_MSG_HOOK_INFO)
    Dim pHook As WIN32K_MSG_HOOK_INFO Ptr = pData

    InitListViewData lvwMsgHook
    ' ====================== 消息钩子 PID列处理 ======================
    Const MSGHOOK_PID_COL_INDEX = 5
    If PID <> Cast(DWORD, -1) Then
        If lvwMsgHook.ColumnText(MSGHOOK_PID_COL_INDEX) = "PID" Then
            lvwMsgHook.DeleteColumn(MSGHOOK_PID_COL_INDEX)
        End If
    Else
        If lvwMsgHook.ColumnText(MSGHOOK_PID_COL_INDEX) <> "PID" Then
            lvwMsgHook.InsertColumn(MSGHOOK_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
        End If
    End If
    ' =================================================================
    'Print "==================== 消息钩子列表 ===================="

    For i As Integer = 0 To hookCount - 1
        Dim info As WIN32K_MSG_HOOK_INFO = pHook[i]
        Dim realVA As ULONG64 = 0
        Dim hProcess As HANDLE = NULL
        Dim finalModulePath As WString * 260
        Dim hookTypeStr As StringW = GetHookTypeName(info.HookType)
        Dim scopeStr As StringW = IIf(info.IsGlobal, "全局钩子", "线程钩子")
        
        If (PID <> Cast(DWORD, -1)) AndAlso (PID <> info.ProcessId) Then Continue For ' 过滤
        
        If IsDriverLoaded Then
            hProcess = SnowSword_OpenProcess(info.ProcessId)
        Else
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, info.ProcessId)
        End If
        Print "正在解析钩子数据..."
        ' ====================== 核心：地址解析逻辑 ======================
        If LenW(info.ModulePath) = 0 Then
            realVA = info.pfn
            finalModulePath = GetModuleOfAddress(hProcess, Cast(PVOID, realVA)) 
        ElseIf LenW(info.ModulePath) > 0 Then
            Dim baseAddr As ULONG64 = Cast(ULONG64, GetModuleBaseByPath(hProcess, info.ModulePath))
            realVA = baseAddr + info.pfn
            finalModulePath = info.ModulePath
        Else
            realVA = info.pfn
            finalModulePath = info.ModulePath
        End If

        ' 填充ListView
        AddItemColListEx lvwMsgHook, 7,,, _
            "0x" & WHex(info.HookHandle), _
            WStr(hookTypeStr), _
            WStr(scopeStr), _
            "0x" & WHex(realVA), _
            finalModulePath, _
            WStr(info.ProcessId), _
            WStr(info.ThreadId)

        ' 调试打印
        MyLog.PrintLog LOG_INFO, "GetMsgHook",, "[" & i & "] " & hookTypeStr & " | " & WHex(info.HookHandle) & " | PID:" & info.ProcessId & _
        " | 模块:" & finalModulePath
        CloseHandle hProcess
    Next

    CommitListViewView lvwMsgHook
    Deallocate pData
    Print "======================================================"
End Sub

' 事件钩子枚举 - 改造版（PID列动态显示/隐藏）
Public Sub GetEventHookList(ByVal PID As DWORD, lvwEventHook As Class_ListView)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    
    Dim ret As Long, lpRet As DWORD
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' 第一次IOCTL：获取缓冲区大小
    IoControl hDrv, IOCTL_EnumEventHook, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetEventHookList] 第一次IOCTL失败: " & WinErrorMsg(GetLastError)
        Exit Sub
    End If

    dataSize = lpRet
    If dataSize = 0 Then
        Print "未发现任何事件钩子"
        InitListViewData lvwEventHook
        ' ====================== 事件钩子 PID列处理 ======================
        ' PID列索引：5 | 列名：PID
        Const EVENTHOOK_PID_COL_INDEX = 5
        If PID <> Cast(DWORD, -1) Then
            If lvwEventHook.ColumnText(EVENTHOOK_PID_COL_INDEX) = "PID" Then
                lvwEventHook.DeleteColumn(EVENTHOOK_PID_COL_INDEX)
            End If
        Else
            If lvwEventHook.ColumnText(EVENTHOOK_PID_COL_INDEX) <> "PID" Then
                lvwEventHook.InsertColumn(EVENTHOOK_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
            End If
        End If
        ' =================================================================
        CommitListViewView lvwEventHook
        Exit Sub
    End If

    ' 分配内存
    pData = Allocate(dataSize)
    If pData = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If

    ' 第二次IOCTL：获取数据
    IoControl hDrv, IOCTL_EnumEventHook, NULL, 0, pData, dataSize, @lpRet
    If GetLastError <> 0 Then
        Print "[GetEventHookList] 第二次IOCTL失败: " & WinErrorMsg(GetLastError)
        Deallocate pData
        Exit Sub
    End If

    ' 解析数据
    Dim hookCount As ULong = dataSize / SizeOf(WIN32K_WINEVENT_HOOK_INFO)
    Dim pHook As WIN32K_WINEVENT_HOOK_INFO Ptr = pData

    InitListViewData lvwEventHook
    ' ====================== 事件钩子 PID列处理 ======================
    Const EVENTHOOK_PID_COL_INDEX = 5
    If PID <> Cast(DWORD, -1) Then
        If lvwEventHook.ColumnText(EVENTHOOK_PID_COL_INDEX) = "PID" Then
            lvwEventHook.DeleteColumn(EVENTHOOK_PID_COL_INDEX)
        End If
    Else
        If lvwEventHook.ColumnText(EVENTHOOK_PID_COL_INDEX) <> "PID" Then
            lvwEventHook.InsertColumn(EVENTHOOK_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
        End If
    End If
    ' =================================================================
    'Print "==================== 事件钩子列表 ===================="

    For i As Integer = 0 To hookCount - 1
        Dim info As WIN32K_WINEVENT_HOOK_INFO = pHook[i]
        Dim realVA As ULONG64 = Cast(ULONG64, info.ActualCallback)
        Dim finalModulePath As WString * 260
        Dim hProcess As HANDLE = NULL
        
        If (PID <> Cast(DWORD, -1)) AndAlso (PID <> info.ProcessId) Then Continue For ' 过滤
        
        If IsDriverLoaded Then
            hProcess = SnowSword_OpenProcess(info.ProcessId)
        Else
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, info.ProcessId)
        End If
        
        ' ====================== 核心：地址解析逻辑 ======================
        If LenW(info.ModulePath) <> 0 Then ' Cast(ULONG_PTR, info.hmodWinEventProc) < &H10000
            Dim baseAddr As ULONG64 = Cast(ULONG64, GetModuleBaseByPath(hProcess, info.ModulePath))
            realVA = baseAddr + Cast(ULONG64, info.pfnWinEventProc)
            finalModulePath = info.ModulePath
        Else
            realVA = Cast(ULONG64, info.pfnWinEventProc)
            finalModulePath = GetModuleOfAddress(hProcess, Cast(PVOID, realVA))
        End If

        ' 填充ListView
        AddItemColListEx lvwEventHook, 9,,, _
            "0x" & WHex(info.HookHandle), _
            "0x" & WHex(info.EventMin) & "~0x" & WHex(info.EventMax), _
            WStr(info.dwFlags), _
            "0x" & WHex(realVA), _
            WStr(finalModulePath), _
            WStr(info.ProcessId), _
            WStr(info.ThreadId), _
            WStr(info.TargetProcessId), _
            WStr(info.TargetThreadId)

        ' 调试打印
        MyLog.PrintLog LOG_INFO, "GetEventHookList",, "[" & i & "] 事件范围:0x" & Hex(info.EventMin) & "~0x" & Hex(info.EventMax) & _
        " | PID:" & info.ProcessId & " | 模块:" & finalModulePath & " | VA:0x" & Hex(realVA)
        CloseHandle hProcess
    Next

    CommitListViewView lvwEventHook
    Deallocate pData
    Print "======================================================"
End Sub

' 热键枚举 - 改造版（PID列动态显示/隐藏）
Public Sub GetHotkeyList(ByVal PID As DWORD, lvwHotkey As Class_ListView)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    
    Dim ret As Long, lpRet As DWORD
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' 第一次IOCTL：获取缓冲区大小
    IoControl hDrv, IOCTL_EnumHotkey, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetHotkeyList] 第一次IOCTL失败: " & WinErrorMsg(GetLastError)
        Print "lpRet:" & lpRet
        Exit Sub
    End If

    dataSize = lpRet
    If dataSize = 0 Then
        Print "未发现任何系统热键"
        InitListViewData lvwHotkey
        ' ====================== 热键 PID列处理 ======================
        ' PID列索引：4 | 列名：PID
        Const HOTKEY_PID_COL_INDEX = 4
        If PID <> Cast(DWORD, -1) Then
            If lvwHotkey.ColumnText(HOTKEY_PID_COL_INDEX) = "PID" Then
                lvwHotkey.DeleteColumn(HOTKEY_PID_COL_INDEX)
            End If
        Else
            If lvwHotkey.ColumnText(HOTKEY_PID_COL_INDEX) <> "PID" Then
                lvwHotkey.InsertColumn(HOTKEY_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
            End If
        End If
        ' =================================================================
        CommitListViewView lvwHotkey
        Exit Sub
    End If

    ' 分配内存
    pData = Allocate(dataSize)
    If pData = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If

    ' 第二次IOCTL：获取热键数据
    IoControl hDrv, IOCTL_EnumHotkey, NULL, 0, pData, dataSize, @lpRet
    If GetLastError <> 0 Then
        Print "[GetHotkeyList] 第二次IOCTL失败: " & WinErrorMsg(GetLastError)
        Deallocate pData
        Exit Sub
    End If

    ' 解析数据
    Dim hotkeyCount As ULong = dataSize / SizeOf(WIN32K_HOTKEY_INFO)
    Dim pHotkey As WIN32K_HOTKEY_INFO Ptr = pData

    InitListViewData lvwHotkey
    ' ====================== 热键 PID列处理 ======================
    Const HOTKEY_PID_COL_INDEX = 4
    If PID <> Cast(DWORD, -1) Then
        If lvwHotkey.ColumnText(HOTKEY_PID_COL_INDEX) = "PID" Then
            lvwHotkey.DeleteColumn(HOTKEY_PID_COL_INDEX)
        End If
    Else
        If lvwHotkey.ColumnText(HOTKEY_PID_COL_INDEX) <> "PID" Then
            lvwHotkey.InsertColumn(HOTKEY_PID_COL_INDEX, "PID", LVCFMT_LEFT, 80)
        End If
    End If
    ' =================================================================
    Print "==================== 系统热键列表 ===================="

    ' 遍历所有热键
    For i As Integer = 0 To hotkeyCount - 1
        Dim info As WIN32K_HOTKEY_INFO = pHotkey[i]
        
        If (PID <> Cast(DWORD, -1)) AndAlso (PID <> info.ProcessId) Then Continue For
        
        ' 热键解析
        Dim sMod As StringW = GetHotkeyModStr(info.uMod)
        Dim sKey As StringW = GetVkKeyStr(info.vk)
        Dim sHotkey As StringW = ""
        If (sMod <> "") AndAlso (sKey <> "") Then
            sHotkey = sMod & "+" & sKey
        ElseIf sMod = "" Then
            sHotkey = sKey
        ElseIf sKey = "" Then
            sHotkey = sMod
        End If
        
        Dim Note As StringW = ""
        If info.pfn <> NULL Then ' 属于内核热键
            Note = "内核热键(pfnCallback = 0x" & WHex(info.pfn) & ")"
        End If
        
        ' 填充ListView
        AddItemColListEx lvwHotkey, 7,,, _
        "0x" & WHex(info.HotkeyHandle), _
        "0x" & WHex(info.id), _
        WStr(sHotkey), _
        "0x" & WHex(info.hWnd), _
        WStr(info.ProcessId), _
        WStr(info.ThreadId), _
        WStr(Note)

        ' 调试打印
        /'Print "[" & i & "] 热键: " & sHotkey & " | 句柄:0x" & WHex(info.HotkeyHandle) & _
              " | PID:" & info.ProcessId & " | TID:" & info.ThreadId & _
              " | hWnd:0x" & WHex(info.hWnd)'/
    Next

    CommitListViewView lvwHotkey
    Deallocate pData
    Print "======================================================"
End Sub