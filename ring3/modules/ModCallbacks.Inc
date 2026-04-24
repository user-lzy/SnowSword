Type CallbackInfo
    TheType As Wstring * 50
    Func As UInteger
    TheContext As UInteger
    Others(3) As UInteger' 预留字段，存储额外信息（如Ex2上下文等）
    'DriverName As Wstring * 260
End Type

Type ObCallbackInfo
    TheType As Wstring * 50
    ObHandle As UInteger
    Func As UInteger
    'DriverName As Wstring * 260
End Type

Type SYSTEM_TIMER
    strType As Wstring * 10
    TimerObject As ULONG64
    Func As ULONG64
    Period As ULong
    uType As Short
    uFlag As Short
    uContext As ULONG64
End Type

' WFP_CALLOUT_INFO 结构体（与驱动一致）
Type WFP_CALLOUT_INFO Field = 8
    CalloutGuid As GUID
    CalloutId As ULong
    ClassifyFn As ULONG64
    NotifyFn As ULONG64
    FlowDeleteFn As ULONG64
    wName As Wstring * 256
End Type

' WFP_FILTER_INFO 结构体
Type WFP_FILTER_INFO Field = 4
    FilterKey As GUID
    FilterId As ULONG64
    ActionType As ULong
    CalloutGuid As GUID
    CalloutId As ULong
    wName As Wstring * 256
End Type

Type MINIFILTER_MAJORFUNCTION
    PreOperation As ULONG64
    PostOperation As ULONG64
End Type
Type PMINIFILTER_MAJORFUNCTION As MINIFILTER_MAJORFUNCTION Ptr

#define FLT_INTERNAL_OPERATION_COUNT 22

Type MINIFILTER_OBJECT Field = 1
    hFilter As ULONG64
    Altitude As Wstring * 10
    wName As Wstring * MAX_PATH
    wPath As Wstring * MAX_PATH
    FilterFunc(10) As ULONG64
    bMajorFunction As Boolean
    MajorFunction(FLT_INTERNAL_OPERATION_COUNT) As MINIFILTER_MAJORFUNCTION
End Type
Type PMINIFILTER_OBJECT As MINIFILTER_OBJECT Ptr ' 指针类型别名

' 与 R0 保持一致的设备节点结构
Type ATTACHED_DEVICE_NODE Field = 8
    IsOriginalDevice As Boolean
    DeviceObject As ULong64
    DriverObject As ULong64
    DriverName As Wstring * 260
    DriverPath As WString * 260
    DeviceName As WString * 260
    Level As ULong
    ParentDeviceObject As ULONG64   ' 新增字段
End Type

' 全局头部
Type GLOBAL_ATTACH_INFO
    DriverCount As ULong
    TotalSize As ULong
End Type

' 单个驱动信息块
Type DRIVER_ATTACH_INFO
    DriverName As Wstring * 260
    DriverObject As ULONG64
    DeviceCount As ULong
    ' 之后紧跟 DeviceCount 个 ATTACHED_DEVICE_NODE
End Type

' 内部使用的驱动分组结构
Type DRIVER_GROUP
    DriverName As String
    DriverObject As ULong64
    DriverPath As String
    Devices(Any) As ATTACHED_DEVICE_NODE  ' 该驱动的所有设备
    DeviceCount As ULong
    ValidDeviceCount As ULong     ' 【新增】未被过滤的有效设备数量
    HasImportantDevice As Boolean      ' 是否有非总线设备
End Type

Type NODE_INFO
    node As ATTACHED_DEVICE_NODE
    parentDevObj As ULONG64   ' 父设备对象地址（0 表示根）
End Type

Type MARKED_DEVICE
    DeviceObject As ULong64
    DeviceName As String
    ParentDeviceObject As ULONG64
    DriverObject As ULong64
    IsMarked As Boolean
End Type

Dim Shared CallbackArray(Any) As CallbackInfo
Dim Shared g_FilteredDevObjs() As ULong64

Public Sub GetCallbackList(lvwCallbacks As Class_ListView)
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
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    InitListViewData lvwCallbacks
    
    IoControl hDrv, IOCTL_EnumCallbacks, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetCallbackList]第一次IOCTL失败:" & WinErrorMsg(GetLastError) & GetLastError
        Exit Sub
    End If
    ReDim CallbackArray(lpRet / SizeOf(CallbackInfo) - 1) As CallbackInfo
    IoControl hDrv, IOCTL_EnumCallbacks, NULL, 0, @CallbackArray(0), (UBound(CallbackArray) + 1) * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To UBound(CallbackArray)
        If Not (InStr(CallbackArray(i).TheType, "Ob") > 0 And CallbackArray(i).Func = 0) Then
            Print "[GetCallbackList]Type:" & CallbackArray(i).TheType & " Address:" & "0x" & WHex(CallbackArray(i).Func)
            NewIndex = AddItemEx(lvwCallbacks, CallbackArray(i).TheType)
            SetItemTextEx lvwCallbacks, NewIndex, 1, "0x" & WHex(CallbackArray(i).Func)
            SetItemTextEx lvwCallbacks, NewIndex, 2, GetDriverPathByAddr(CallbackArray(i).Func)
            If InStr(CallbackArray(i).TheType, "Ob") > 0 Then
                SetItemTextEx lvwCallbacks, NewIndex, 3, "0x" & WHex(CallbackArray(i).TheContext)
                SetItemTextEx lvwCallbacks, NewIndex, 4, "ObHandle=0x" & WHex(CallbackArray(i).Others(0))
            ElseIf CallbackArray(i).TheType = "CmpCallback" Then
                SetItemTextEx lvwCallbacks, NewIndex, 3, "0x" & WHex(CallbackArray(i).TheContext)
                SetItemTextEx lvwCallbacks, NewIndex, 4, "cookie.QuadPart=0x" & WHex(CallbackArray(i).Others(0))
            ElseIf InStr(CallbackArray(i).TheType, "\Callback\") > 0 Then
                SetItemTextEx lvwCallbacks, NewIndex, 3, "0x" & WHex(CallbackArray(i).TheContext)
                SetItemTextEx lvwCallbacks, NewIndex, 4, "CallbackRegistration=0x" & WHex(CallbackArray(i).Others(0))
            ElseIf CallbackArray(i).TheType = "BugCheck" OrElse CallbackArray(i).TheType = "BugCheckReason" Then
                SetItemTextEx lvwCallbacks, NewIndex, 3, "0x" & WHex(CallbackArray(i).TheContext)
                SetItemTextEx lvwCallbacks, NewIndex, 4, "CallbackRecord=0x" & WHex(CallbackArray(i).Others(0))
            Else
                SetItemTextEx lvwCallbacks, NewIndex, 3, "0x" & WHex(CallbackArray(i).TheContext)
                SetItemTextEx lvwCallbacks, NewIndex, 4, "无"
            End If
        End If
    Next
    CommitListViewView lvwCallbacks
End Sub

Public Sub GetKernelTimerList(lvwKernelTimer As Class_ListView)
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
    Dim SystemTimers(63) As SYSTEM_TIMER
    Dim ret As Long, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    
    InitListViewData lvwKernelTimer
    
    IoControl hDrv, IOCTL_EnumIoTimers, NULL, 0, @SystemTimers(0), 64 * SizeOf(SYSTEM_TIMER)
    For i As Integer = 0 To 63
        If (SystemTimers(i).Func <> 0) Then
            /'NewIndex = AddItemEx(lvwKernelTimer,, SystemTimers(i).strType)
            SetItemTextEx lvwKernelTimer, NewIndex, 1, "0x" & Hex(SystemTimers(i).TimerObject)
            SetItemTextEx lvwKernelTimer, NewIndex, 2, "0x" & Hex(SystemTimers(i).Func)
            
            ' SystemTimers(i).uFlag
            SetItemTextEx lvwKernelTimer, NewIndex, 4, WStr(SystemTimers(i).uType)'IIf(SystemTimers(i).st, "已启动", "已停止")
            SetItemTextEx lvwKernelTimer, NewIndex, 5, GetDriverPathByAddr(SystemTimers(i).Func) 'Callbacks(i).DriverName'/
            AddItemColListEx lvwKernelTimer, 6,,, SystemTimers(i).strType, "0x" & WHex(SystemTimers(i).TimerObject), "0x" & WHex(SystemTimers(i).Func), _
            WStr(""), WStr(SystemTimers(i).uType), WStr(GetDriverPathByAddr(SystemTimers(i).Func))
        End If
    Next
    memset @SystemTimers(0), 0, 64 * SizeOf(SYSTEM_TIMER)
    IoControl hDrv, IOCTL_EnumDpcTimers, NULL, 0, @SystemTimers(0), 64 * SizeOf(SYSTEM_TIMER)
    For i As Integer = 0 To 63
        If (SystemTimers(i).strType <> "") Then
            AddItemColListEx lvwKernelTimer, 6,,, SystemTimers(i).strType, "0x" & WHex(SystemTimers(i).TimerObject), "0x" & WHex(SystemTimers(i).Func), _
            WStr(SystemTimers(i).Period), WStr(SystemTimers(i).uType), WStr(GetDriverPathByAddr(SystemTimers(i).Func))
        End If
    Next
    CommitListViewView lvwKernelTimer
End Sub

' 将 GUID 转换为字符串
Private Function GuidToString(g As GUID) As String
    Dim s As String
    s = Hex(g.Data1, 8) & "-" & Hex(g.Data2, 4) & "-" & Hex(g.Data3, 4) & "-"
    s = s & Hex(g.Data4(0), 2) & Hex(g.Data4(1), 2) & "-"
    s = s & Hex(g.Data4(2), 2) & Hex(g.Data4(3), 2) & Hex(g.Data4(4), 2) & Hex(g.Data4(5), 2) & Hex(g.Data4(6), 2) & Hex(g.Data4(7), 2)
    Return s
End Function

Public Sub GetWfpCalloutList(lvwCallouts As Class_ListView)
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
    
    Dim ret As Long, lpRet As DWORD = 0
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' ########################################################################
    ' 🔥 关键：第一次调用（获取大小），强制忽略 ERROR_INSUFFICIENT_BUFFER (122)
    ' ########################################################################
    IoControl hDrv, IOCTL_EnumWfpCallouts, NULL, 0, NULL, 0, @lpRet
    Print "lpRet:" & lpRet
    lpRet = lpRet ' 确保读取到内核返回的长度
    
    ' 只要拿到长度 > 0，就继续，无视这个错误
    If lpRet = 0 Then
        Print "未发现任何 Callout 或获取长度失败"
        Exit Sub
    End If

    dataSize = lpRet
    Print "需要缓冲区大小: " & dataSize & "  单个结构体大小: " & SizeOf(WFP_CALLOUT_INFO)

    ' 分配内存 + 清零（必加，防止垃圾数据）
    pData = Allocate(dataSize)
    RtlZeroMemory(pData, dataSize)
    If pData = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If

    ' ########################################################################
    ' 🔥 第二次调用：正式读取数据（一定成功）
    ' ########################################################################
    If IoControl(hDrv, IOCTL_EnumWfpCallouts, NULL, 0, pData, dataSize, @lpRet) = 0 Then
        Print "[GetWfpCalloutList] 读取数据失败: " & WinErrorMsg(GetLastError)
        Deallocate pData
        Exit Sub
    End If

    ' 解析数据
    Dim count As ULong = dataSize / SizeOf(WFP_CALLOUT_INFO)
    Dim pInfo As WFP_CALLOUT_INFO Ptr = pData

    InitListViewData lvwCallouts

    For i As Integer = 0 To count - 1
        AddItemColListEx lvwCallouts, 6,,, _
            WStr(GuidToString(pInfo[i].CalloutGuid)), _
            WStr(pInfo[i].CalloutId), _
            "0x" & WHex(pInfo[i].ClassifyFn), _
            "0x" & WHex(pInfo[i].NotifyFn), _
            "0x" & WHex(pInfo[i].FlowDeleteFn), _
            pInfo[i].wName
    Next

    CommitListViewView lvwCallouts
    Deallocate pData
    Print "成功加载 " & count & " 个 WFP Callout"
End Sub

Public Sub GetWfpFilterList(lvwFilters As Class_ListView)
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
    
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' 第一次调用：获取所需缓冲区大小
    IoControl hDrv, IOCTL_EnumWfpFilters, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetWfpFilterList] 第一次IOCTL失败: " & WinErrorMsg(GetLastError)
        Exit Sub
    End If

    dataSize = lpRet
    If dataSize = 0 Then
        Print "未发现任何 Filter（Callout类型）"
        Exit Sub
    End If

    ' 分配缓冲区并获取数据
    pData = Allocate(dataSize)
    If pData = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If

    IoControl hDrv, IOCTL_EnumWfpFilters, NULL, 0, pData, dataSize, @lpRet
    If GetLastError <> 0 Then
        Print "[GetWfpFilterList] 第二次IOCTL失败: " & WinErrorMsg(GetLastError)
        Deallocate pData
        Exit Sub
    End If

    ' 解析并填充 ListView
    Dim count As ULong = dataSize / SizeOf(WFP_FILTER_INFO)
    Dim pInfo As WFP_FILTER_INFO Ptr = pData

    InitListViewData lvwFilters   ' 清空列表

    For i As Integer = 0 To count - 1
        AddItemColListEx lvwFilters, 5,,, WStr(pInfo[i].FilterId), WStr(pInfo[i].CalloutId), pInfo[i].wName, WStr(pInfo[i].ActionType), WStr(GuidToString(pInfo[i].CalloutGuid))
    Next

    CommitListViewView lvwFilters
    Deallocate pData
End Sub

Function EnumMinifilter(lvwMinifilter As Class_mCtrlTreeList) As ULong
    Dim FilterFuncName(10) As StringW = {"FilterUnload", "InstanceSetup", "InstanceQueryTeardown", "InstanceTeardownStart", _
    "InstanceTeardownComplete", "PreVolumeMount", "PostVolumeMount", "GenerateFileName", "NormalizeNameComponent", _
    "NormalizeNameComponentEx", "NormalizeContextCleanup"}
    Dim MinifilterArray() As MINIFILTER_OBJECT
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Return 0
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Return 0
        End If
    End If
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    SetLastError 0
    IoControl hDrv, IOCTL_EnumMiniFilter, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[EnumMinifilter]第一次IOCTL失败:" & WinErrorMsg(GetLastError) & GetLastError
        Return 0
    End If
    lvwMinifilter.DeleteItem MC_TLI_ROOT
    ReDim MinifilterArray(lpRet / SizeOf(MINIFILTER_OBJECT) - 1) As MINIFILTER_OBJECT
    IoControl hDrv, IOCTL_EnumMiniFilter, NULL, 0, @MinifilterArray(0), (UBound(MinifilterArray) + 1) * SizeOf(MINIFILTER_OBJECT), @lpRet
    
    Dim hGroup As MC_HTREELISTITEM, hItem As MC_HTREELISTITEM
    For i As Integer = 0 To UBound(MinifilterArray)
        'Print MinifilterArray(i).wName
        hGroup = lvwMinifilter.AddItem(MC_TLI_ROOT, MinifilterArray(i).wName)
        lvwMinifilter.SetItemText hGroup, 2, GetCorrectPath(MinifilterArray(i).wPath)
        lvwMinifilter.SetItemText hGroup, 3, "0x" & WHex(MinifilterArray(i).hFilter)
        lvwMinifilter.SetItemText hGroup, 4, MinifilterArray(i).Altitude
        For j As Integer = 0 To 10
            If MinifilterArray(i).FilterFunc(j) = NULL Then Continue For
            hItem = lvwMinifilter.AddItem(hGroup, FilterFuncName(j))
            lvwMinifilter.SetItemText hItem, 1, "0x" & WHex(MinifilterArray(i).FilterFunc(j))
            lvwMinifilter.SetItemText hItem, 2, GetDriverPathByAddr(MinifilterArray(i).FilterFunc(j))
        Next
        If Not MinifilterArray(i).bMajorFunction Then Continue For
        For j As Integer = 0 To UBound(MinifilterArray(i).MajorFunction)
            If (MinifilterArray(i).MajorFunction(j).PreOperation = NULL) AndAlso (MinifilterArray(i).MajorFunction(j).PostOperation = NULL) Then Continue For
            hItem = lvwMinifilter.AddItem(hGroup, "")
            If MinifilterArray(i).MajorFunction(j).PreOperation <> NULL Then
                lvwMinifilter.SetItemText hItem, 0, IRP_MJ_ToString(j) & "(Pre)"
                lvwMinifilter.SetItemText hItem, 1, "0x" & WHex(MinifilterArray(i).MajorFunction(j).PreOperation)
                lvwMinifilter.SetItemText hItem, 2, GetDriverPathByAddr(MinifilterArray(i).MajorFunction(j).PreOperation)
            End If
            If MinifilterArray(i).MajorFunction(j).PostOperation <> NULL Then
                lvwMinifilter.SetItemText hItem, 0, IRP_MJ_ToString(j) & "(Post)"
                lvwMinifilter.SetItemText hItem, 1, "0x" & WHex(MinifilterArray(i).MajorFunction(j).PostOperation)
                lvwMinifilter.SetItemText hItem, 2, GetDriverPathByAddr(MinifilterArray(i).MajorFunction(j).PostOperation)
            End If
        Next
    Next
    Return 0
End Function

' 功能：过滤乱码字符，仅保留可打印Unicode字符，确保驱动名称显示正常
Private Function CleanDriverName(ByVal rawName As String) As String
    Dim cleanStr As String = ""
    Dim i As Long
    For i = 1 To Len(rawName)
        Dim c As Integer = AscW(Mid(rawName, i, 1))
        ' 保留可打印字符（ASCII 32-126）和常见Unicode字符，遇到\0立即终止
        If c = 0 Then
            Exit For
        ElseIf (c >= 32 And c <= 126) Or (c >= &H410 And c <= &H2FA1F) Then
            cleanStr = cleanStr & Mid(rawName, i, 1)
        End If
    Next
    ' 过滤空字符串，兜底显示"Unknown"
    If Len(cleanStr) = 0 Then
        CleanDriverName = "Unknown"
    Else
        CleanDriverName = cleanStr
    End If
End Function

' ============================================================================
' 新增：判断设备名称是否属于"需要过滤的内部设备"模式
' ============================================================================
Private Function IsInternalDeviceName(ByVal DeviceName As String) As Boolean
    If Len(DeviceName) = 0 Then Return False
    
    ' 必须是 \Device\ 开头
    If InStr(DeviceName, "\Device\") = 0 Then Return False
    
    ' 提取 \Device\ 后面的部分
    Dim namePart As String = Mid(DeviceName, InStr(DeviceName, "\Device\") + 8)
    If Len(namePart) = 0 Then Return False
    
    ' 模式1: 纯数字命名 (如 00000001, 0000006a 等)
    Dim allDigits As Boolean = True
    For i As Integer = 1 To Len(namePart)
        Dim c As Integer = AscW(Mid(namePart, i, 1))
        ' 允许 0-9 和 a-f (因为可能有十六进制)
        If Not ((c >= 48 And c <= 57) Or (c >= 97 And c <= 102) Or (c >= 65 And c <= 70)) Then
            allDigits = False
            Exit For
        End If
    Next
    If allDigits And Len(namePart) <= 10 Then Return True
    
    ' 模式2: NTPNP_PCI 开头 (只过滤这些，不过滤其他 NTPNP_)
    If InStr(namePart, "NTPNP_PCI") = 1 Then Return True
    
    Return False
End Function

' 检查设备对象是否为 PnpManager 驱动
Private Function IsPnpManagerDevice(ByVal devObj As ULong64, _
                                    pnpManagerObjs() As ULong64, _
                                    ByVal count As ULong) As Boolean
    For i As ULong = 0 To count - 1
        If pnpManagerObjs(i) = devObj Then Return True
    Next
    Return False
End Function

Private Sub MarkFilteredDeviceChain(nodes() As NODE_INFO, ByVal nodeCount As ULong, _
                                    marked() As MARKED_DEVICE, ByRef markedCount As ULong)
    markedCount = 0

    ' 1. 收集所有 PnpManager 驱动设备对象
    Dim pnpManagerObjs() As ULong64
    Dim pnpCount As ULong = 0
    For i As Long = 0 To nodeCount - 1
        Dim nd As ATTACHED_DEVICE_NODE = nodes(i).node
        If nd.DriverName = "\Driver\PnpManager" Then
            ReDim Preserve pnpManagerObjs(pnpCount)
            pnpManagerObjs(pnpCount) = nd.DeviceObject
            pnpCount += 1
        End If
    Next

    ' 2. 遍历所有节点，标记内部匿名设备（但跳过 PnpManager 自身及其子设备）
    For i As Long = 0 To nodeCount - 1
        Dim nd As ATTACHED_DEVICE_NODE = nodes(i).node
        Dim devName As String = Trim(nd.DeviceName)

        ' 如果驱动是 PnpManager 本身，不标记
        If nd.DriverName = "\Driver\PnpManager" Then Continue For

        ' 如果设备名称符合内部匿名规则
        If IsInternalDeviceName(devName) Then
            ' 获取父设备对象
            Dim parentObj As ULong64 = nd.ParentDeviceObject
            ' 若父设备是 PnpManager 设备，则不标记（允许显示子设备）
            If parentObj <> 0 AndAlso IsPnpManagerDevice(parentObj, pnpManagerObjs(), pnpCount) Then
                Continue For
            End If
            ' 否则标记为内部匿名设备
            marked(markedCount).DeviceObject = nd.DeviceObject
            marked(markedCount).IsMarked = True
            markedCount += 1
        End If
    Next
End Sub

' ============================================================================
' 辅助：判断设备是否已被标记为内部匿名设备
' ============================================================================
Private Function IsDeviceMarked(ByVal devObj As ULong64, marked() As MARKED_DEVICE, ByVal markedCount As ULong) As Boolean
    For i As Long = 0 To markedCount - 1
        If marked(i).DeviceObject = devObj Then
            Return True
        End If
    Next
    Return False
End Function

' ============================================================================
' 辅助：判断设备是否有未被标记的子节点（用于决定根设备是否显示）
' ============================================================================
Private Function HasValidChildren(ByVal devObj As ULONG64, nodes() As NODE_INFO, ByVal nodeCount As ULong, _
                                 marked() As MARKED_DEVICE, ByVal markedCount As ULong) As Boolean
    For idx As Long = 0 To nodeCount - 1
        If nodes(idx).parentDevObj = devObj Then
            If Not IsDeviceMarked(nodes(idx).node.DeviceObject, marked(), markedCount) Then
                Return True
            End If
        End If
    Next
    Return False
End Function

' 判断驱动是否为总线设备（可根据需要扩展列表）
Private Function IsBusDevice(ByVal driverName As String) As Boolean
    Dim busDrivers(2) As String = {"\Driver\acpi", "\Driver\pci", "\Driver\hal"}
    For i As Integer = 0 To UBound(busDrivers)
        If UCase(driverName) = UCase(busDrivers(i)) Then Return True
    Next
    Return False
End Function

' ============================================================================
' 重写：递归添加节点，核心逻辑极简
' 规则：只要未被标记，就添加；有子节点就继续递归
' ============================================================================
Private Sub AddNodesByParent(ByVal lvw As Class_mCtrlTreeList, ByVal parentItem As MC_HTREELISTITEM, _
    nodes() As NODE_INFO, ByVal nodeCount As ULong, _
    ByVal parentDevObj As ULONG64, ByRef totalAdded As ULong, _
    marked() As MARKED_DEVICE, ByVal markedCount As ULong)
    
    'Print "AddNodesByParent: parentDevObj=0x" & WHex(parentDevObj)
    For idx As Long = 0 To nodeCount - 1
        If nodes(idx).parentDevObj <> parentDevObj Then Continue For
        
        Dim nd As ATTACHED_DEVICE_NODE = nodes(idx).node
        'Print "  Found child: " & nd.DriverName & " / " & nd.DeviceName & " (DevObj=0x" & WHex(nd.DeviceObject) & ")"
        
        ' 1. 过滤内部匿名设备
        If IsDeviceMarked(nd.DeviceObject, marked(), markedCount) Then
            'Print "    Skip: marked internal device"
            Continue For
        End If
        
        ' 2. 添加当前节点（只要未被标记就添加，不管有没有子节点）
        Dim drvName As String = CleanDriverName(nd.DriverName)
        Dim devName As String = CleanDriverName(nd.DeviceName)
        
        Dim hItem As MC_HTREELISTITEM = lvw.AddItem(parentItem, drvName)
        lvw.SetItemText hItem, 1, "0x" & WHex(nd.DriverObject)
        lvw.SetItemText hItem, 2, GetCorrectPathW(CleanDriverName(nd.DriverPath))
        lvw.SetItemText hItem, 3, devName
        lvw.SetItemText hItem, 4, "0x" & WHex(nd.DeviceObject)
        
        totalAdded += 1
        'Print "    Added to tree, total=" & totalAdded
        
        ' 3. 递归处理子节点
        AddNodesByParent lvw, hItem, nodes(), nodeCount, nd.DeviceObject, totalAdded, _
                        marked(), markedCount
    Next
End Sub

' ============================================================================
' 重写：主函数 EnumAttachDevices
' ============================================================================
Function EnumAttachDevices(ByVal lvwAttachDevice As Class_mCtrlTreeList, _
                           ByVal ShowAllDevices As Boolean = False, _
                           ByVal ShowBusDevices As Boolean = False) As ULong
    If hDrv = INVALID_HANDLE_VALUE Then
        Print "EnumAttachDevices: Driver handle invalid"
        Return 0
    End If
    
    lvwAttachDevice.DeleteItem MC_TLI_ROOT
    
    ' 1. 获取数据
    Dim cbRet As ULong
    If IoControl(hDrv, IOCTL_EnumAttachDevices, NULL, 0, NULL, 0, @cbRet) = 0 Then Return 0
    Dim requiredSize As ULong = cbRet
    If requiredSize = 0 Then Return 0
    
    Dim pBuffer As Byte Ptr = Allocate(requiredSize)
    If pBuffer = NULL Then Return 0
    If IoControl(hDrv, IOCTL_EnumAttachDevices, NULL, 0, pBuffer, requiredSize, @cbRet) = 0 Then
        Deallocate pBuffer
        Return 0
    End If
    
    ' 2. 解析数据
    Dim pGlobal As GLOBAL_ATTACH_INFO Ptr = CPtr(GLOBAL_ATTACH_INFO Ptr, pBuffer)
    Dim offset As ULong = sizeof(GLOBAL_ATTACH_INFO)
    
    Dim nodes() As NODE_INFO
    Dim nodeCount As ULong = 0
    
    For i As ULong = 0 To pGlobal->DriverCount - 1
        Dim pDrvInfo As DRIVER_ATTACH_INFO Ptr = CPtr(DRIVER_ATTACH_INFO Ptr, pBuffer + offset)
        offset += sizeof(DRIVER_ATTACH_INFO)
        
        Dim pDevices As ATTACHED_DEVICE_NODE Ptr = CPtr(ATTACHED_DEVICE_NODE Ptr, pBuffer + offset)
        Dim deviceCount As ULong = pDrvInfo->DeviceCount
        offset += deviceCount * sizeof(ATTACHED_DEVICE_NODE)
        
        For j As ULong = 0 To deviceCount - 1
            ReDim Preserve nodes(nodeCount)
            nodes(nodeCount).node = pDevices[j]
            nodes(nodeCount).parentDevObj = nodes(nodeCount).node.ParentDeviceObject
            nodeCount += 1
        Next
    Next
    
    ' 3. 标记内部匿名设备（仅标记这个，其他不做限制）
    Dim marked() As MARKED_DEVICE
    Dim markedCount As ULong = 0
    ReDim marked(nodeCount)
    MarkFilteredDeviceChain nodes(), nodeCount, marked(), markedCount
    Print "Marked " & markedCount & " devices for filtering"
    
    ' 4. 遍历并构建树（关键修复：仅遍历真正的根设备 Parent=0）
    Dim totalAdded As ULong = 0
    For i As Long = 0 To nodeCount - 1
        Dim nd As ATTACHED_DEVICE_NODE = nodes(i).node
        Dim devObj As ULONG64 = nd.DeviceObject
        Dim parentObj As ULONG64 = nd.ParentDeviceObject
        
        If parentObj <> 0 Then Continue For
        If IsDeviceMarked(devObj, marked(), markedCount) Then Continue For

        ' 决定是否显示当前根设备
        Dim shouldShowRoot As Boolean = False

        If ShowAllDevices Then
            ' 显示所有根设备（无论总线还是非总线）
            shouldShowRoot = True
        Else
            ' ShowAllDevices = False 时，依据总线开关和子节点决定
            Dim isBus As Boolean = IsBusDevice(nd.DriverName)
            If ShowBusDevices Then
                ' 显示总线设备：如果有子节点或强制显示（已由 ShowBusDevices 触发）
                ' 实际上 ShowBusDevices = True 时，总线设备只要有子节点就显示
                ' 这里与之前逻辑一致：如果没有子节点，则仍然不显示
                If isBus Then
                    shouldShowRoot = HasValidChildren(devObj, nodes(), nodeCount, marked(), markedCount)
                Else
                    shouldShowRoot = HasValidChildren(devObj, nodes(), nodeCount, marked(), markedCount)
                End If
            Else
                ' ShowBusDevices = False 时，总线设备直接隐藏，非总线设备按子节点决定
                If isBus Then
                    shouldShowRoot = False   ' 隐藏总线设备
                Else
                    shouldShowRoot = HasValidChildren(devObj, nodes(), nodeCount, marked(), markedCount)
                End If
            End If
        End If

        If Not shouldShowRoot Then
            'Print "Skip root device: condition not met, DevObj=0x" & WHex(devObj)
            Continue For
        End If
        
        ' 添加根设备本身
        Dim drvName As String = CleanDriverName(nd.DriverName)
        Dim devName As String = CleanDriverName(nd.DeviceName)
        
        'Print "Added root device: " & drvName & " / " & devName
        Dim rootItem As MC_HTREELISTITEM = lvwAttachDevice.AddItem(MC_TLI_ROOT, drvName)
        lvwAttachDevice.SetItemText rootItem, 1, "0x" & WHex(nd.DriverObject)
        lvwAttachDevice.SetItemText rootItem, 2, GetCorrectPathW(CleanDriverName(nd.DriverPath))
        lvwAttachDevice.SetItemText rootItem, 3, devName
        lvwAttachDevice.SetItemText rootItem, 4, "0x" & WHex(devObj)
        totalAdded += 1
        
        ' 递归添加子节点
        AddNodesByParent lvwAttachDevice, rootItem, nodes(), nodeCount, devObj, totalAdded, _
                        marked(), markedCount
    Next
    
    ' 5. 清理
    Deallocate pBuffer
    Print "EnumAttachDevices: Completed, total valid devices = " & totalAdded
    Return totalAdded
End Function

