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

Type INSTANCE_AGGREGATE_STANDARD_INFORMATION_MINIFILTER
    '' MiniFilter 部分（fltmc 只用到这个）
    dwFlags                   As ULong
    FrameID                 As ULong
    VolumeFileSystemType    As ULONG
    InstanceNameLength      As USHORT
    InstanceNameBufferOffset As USHORT
    AltitudeLength          As USHORT
    AltitudeBufferOffset    As USHORT
    VolumeNameLength        As USHORT
    VolumeNameBufferOffset  As USHORT
    FilterNameLength        As USHORT
    FilterNameBufferOffset  As USHORT
    SupportedFeatures       As ULONG
End Type

Type INSTANCE_AGGREGATE_STANDARD_INFORMATION_LEGACYFILTER
    '' LegacyFilter 部分（一般不用）
    dwFlags                   As ULong
    AltitudeLength          As USHORT
    AltitudeBufferOffset    As UShort
    VolumeNameLength        As UShort
    VolumeNameBufferOffset  As USHORT
    FilterNameLength        As USHORT
    FilterNameBufferOffset  As USHORT
    SupportedFeatures       As ULONG
End Type

'' 对应：INSTANCE_AGGREGATE_STANDARD_INFORMATION
Type INSTANCE_AGGREGATE_STANDARD_INFORMATION
    NextEntryOffset As ULONG
    Flags           As ULONG
    Union
        MinifilterType As INSTANCE_AGGREGATE_STANDARD_INFORMATION_MINIFILTER
        LegacyType As INSTANCE_AGGREGATE_STANDARD_INFORMATION_LEGACYFILTER
    End Union
End Type

Enum INSTANCE_INFORMATION_CLASS
    InstanceBasicInformation = 0         ' 如果你不需要 Basic，可以不写
    InstanceFullInformation = 1          ' ← 原先为 0，改正确
    InstanceNameInformation ' 如果你后面还用，可保留或相应调整数值
    InstanceAggregateStandardInformation
End Enum

#define NDIS_ENUM_MAX_STRING_LEN  128
#define NDIS_ENUM_MAX_FILTERS_PER_MINIPORT  8
#define NDIS_ENUM_MAX_OPENS_PER_MINIPORT    16

' ============================================================================
' 结构体定义（需与驱动完全一致，packed 对齐）
' ============================================================================
Type NDIS_ENUM_FILTER_ENTRY Field = 1
    FilterBlock     As ULONG64
    DriverObject    As ULONG64
    InstanceName    As Wstring * NDIS_ENUM_MAX_STRING_LEN
    FriendlyName    As Wstring * NDIS_ENUM_MAX_STRING_LEN
    DriverImageName As Wstring * NDIS_ENUM_MAX_STRING_LEN
End Type

Type NDIS_ENUM_OPEN_ENTRY Field = 1
    OpenBlock          As ULONG64
    ProtocolBlock      As ULONG64
    ProtocolName       As Wstring * NDIS_ENUM_MAX_STRING_LEN
    ProtocolImageName  As Wstring * NDIS_ENUM_MAX_STRING_LEN
End Type

Type NDIS_MINIPORT_ENUM_ENTRY Field = 1
    MiniportBlock            As ULONG64
    DriverPath               As WString * 260
    pAdapterInstanceName     As Wstring * NDIS_ENUM_MAX_STRING_LEN
    BaseName                 As Wstring * NDIS_ENUM_MAX_STRING_LEN
    FilterCount              As ULONG
    Filters(NDIS_ENUM_MAX_FILTERS_PER_MINIPORT - 1) As NDIS_ENUM_FILTER_ENTRY
    OpenCount                As ULONG
    Opens(NDIS_ENUM_MAX_OPENS_PER_MINIPORT - 1)     As NDIS_ENUM_OPEN_ENTRY
End Type

' 与 R0 保持一致的设备节点结构
Type ATTACHED_DEVICE_NODE Field = 8
    IsOriginalDevice As Boolean
    DeviceObject As ULONG64
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
            'Print "[GetCallbackList]Type:" & CallbackArray(i).TheType & " Address:" & "0x" & WHex(CallbackArray(i).Func)
            Dim DriverPath As StringW = GetDriverPathByAddr(CallbackArray(i).Func)
            NewIndex = AddItemEx(lvwCallbacks, CallbackArray(i).TheType)
            SetItemTextEx lvwCallbacks, NewIndex, 1, "0x" & WHex(CallbackArray(i).Func)
            SetItemTextEx lvwCallbacks, NewIndex, 2, DriverPath
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
            VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", NewIndex, DriverPath
        End If
    Next
    CommitListViewView lvwCallbacks
End Sub

Public Sub GetKernelTimerList(lvwKernelTimer As Class_ListView)
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
    Dim ret As Long, lpRet As DWORD = 0
    Dim pData As Any Ptr = NULL
    Dim dataSize As DWORD = 0

    ' ########################################################################
    ' 🔥 关键：第一次调用（获取大小），强制忽略 ERROR_INSUFFICIENT_BUFFER (122)
    ' ########################################################################
    IoControl hDrv, IOCTL_EnumWfpCallouts, NULL, 0, NULL, 0, @lpRet
    
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
                      "InstanceTeardownComplete", "PreVolumeMount", "PostVolumeMount", "GenerateFileName", _
                      "NormalizeNameComponent", "NormalizeNameComponentEx", "NormalizeContextCleanup"}

    Dim MinifilterArray() As MINIFILTER_OBJECT
    Dim lpRet As DWORD
    Dim i As Long, j As Long

    ' ----------------------------------
    ' 修复：第一次 IOCTL → 标准查询大小（传NULL+0长度）
    ' ----------------------------------
    SetLastError 0
    IoControl hDrv, IOCTL_EnumMiniFilter, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[EnumMinifilter] 第一次 IOCTL 失败:" & WinErrorMsg(GetLastError) & GetLastError
        Return 0
    End If

    ' 根据返回大小计算实际数量
    Dim filterCount As Long
    filterCount = lpRet / SizeOf(MINIFILTER_OBJECT)
    If filterCount <= 0 Then Return 0

    ' ----------------------------------
    ' 分配最终数组
    ' ----------------------------------
    ReDim MinifilterArray(filterCount - 1) As MINIFILTER_OBJECT

    ' 第二次 IOCTL 获取完整数据
    SetLastError 0
    IoControl hDrv, IOCTL_EnumMiniFilter, NULL, 0, @MinifilterArray(0), filterCount * SizeOf(MINIFILTER_OBJECT), @lpRet
    If GetLastError <> 0 Then
        Print "[EnumMinifilter] 第二次 IOCTL 失败:" & WinErrorMsg(GetLastError) & GetLastError
        Return 0
    End If
    ' ====== CLI 旁路输出 ======
    If g_CuiMode Then
        Dim colNames(4) As StringW = { _
            "名称", "回调函数地址", "所属模块", "PFLT_FILTER", "Altitude" _
        }
        
        Dim maxRows As Long = filterCount * (1 + 11 + 56)
        Dim rows(maxRows - 1) As CuiExportRow
        Dim rowIdx As Long = 0
        
        For i As Long = 0 To UBound(MinifilterArray)
            ' --- 根行：过滤器本身 (indent=0) ---
            rows(rowIdx).IndentLevel = 0
            rows(rowIdx).IsGroup = True
            rows(rowIdx).GroupLabel = MinifilterArray(i).wName
            rows(rowIdx).ColCount = 5
            rows(rowIdx).Cols(0) = MinifilterArray(i).wName
            rows(rowIdx).Cols(1) = ""
            rows(rowIdx).Cols(2) = GetCorrectPath(MinifilterArray(i).wPath)
            rows(rowIdx).Cols(3) = "0x" & WHex(MinifilterArray(i).hFilter)
            rows(rowIdx).Cols(4) = MinifilterArray(i).Altitude
            rowIdx += 1
            
            ' --- 一级子行：FilterFunc 回调 (indent=1) ---
            For j As Long = 0 To 10
                If MinifilterArray(i).FilterFunc(j) = NULL Then Continue For
                
                rows(rowIdx).IndentLevel = 1
                rows(rowIdx).IsGroup = False
                rows(rowIdx).ColCount = 5
                rows(rowIdx).Cols(0) = FilterFuncName(j)
                rows(rowIdx).Cols(1) = "0x" & WHex(MinifilterArray(i).FilterFunc(j))
                rows(rowIdx).Cols(2) = GetDriverPathByAddr(MinifilterArray(i).FilterFunc(j))
                rowIdx += 1
            Next
            
            ' --- 一级子行：MajorFunction (indent=1) ---
            If MinifilterArray(i).bMajorFunction Then
                For j As Long = 0 To UBound(MinifilterArray(i).MajorFunction)
                    Dim hasPre As Boolean = (MinifilterArray(i).MajorFunction(j).PreOperation <> NULL)
                    Dim hasPost As Boolean = (MinifilterArray(i).MajorFunction(j).PostOperation <> NULL)
                    If Not hasPre AndAlso Not hasPost Then Continue For
                    
                    If hasPre Then
                        rows(rowIdx).IndentLevel = 1
                        rows(rowIdx).IsGroup = False
                        rows(rowIdx).ColCount = 5
                        rows(rowIdx).Cols(0) = IRP_MJ_ToString(j) & "(Pre)"
                        rows(rowIdx).Cols(1) = "0x" & WHex(MinifilterArray(i).MajorFunction(j).PreOperation)
                        rows(rowIdx).Cols(2) = GetDriverPathByAddr(MinifilterArray(i).MajorFunction(j).PreOperation)
                        rowIdx += 1
                    End If
                    
                    If hasPost Then
                        rows(rowIdx).IndentLevel = 1
                        rows(rowIdx).IsGroup = False
                        rows(rowIdx).ColCount = 5
                        rows(rowIdx).Cols(0) = IRP_MJ_ToString(j) & "(Post)"
                        rows(rowIdx).Cols(1) = "0x" & WHex(MinifilterArray(i).MajorFunction(j).PostOperation)
                        rows(rowIdx).Cols(2) = GetDriverPathByAddr(MinifilterArray(i).MajorFunction(j).PostOperation)
                        rowIdx += 1
                    End If
                Next
            End If
        Next
        
        If rowIdx > 0 Then
            ReDim Preserve rows(rowIdx - 1) As CuiExportRow
            CuiPrintTreeListData("MINIFILTER", rows(), colNames(), 5)
        Else
            Dim emptyRows() As CuiExportRow
            ReDim emptyRows(-1) As CuiExportRow
            CuiPrintTreeListData("MINIFILTER", emptyRows(), colNames(), 5)
        End If
        Return filterCount
    End If
    ' ----------------------------------
    ' 清空 TreeList
    ' ----------------------------------
    lvwMinifilter.DeleteItem MC_TLI_ROOT

    ' ----------------------------------
    ' 遍历填充 TreeList
    ' ----------------------------------
    Dim hGroup As MC_HTREELISTITEM, hItem As MC_HTREELISTITEM
    For i = 0 To UBound(MinifilterArray)
        ' 添加根节点
        hGroup = lvwMinifilter.AddItem(MC_TLI_ROOT, MinifilterArray(i).wName)
        lvwMinifilter.SetItemText hGroup, 2, GetCorrectPath(MinifilterArray(i).wPath)
        lvwMinifilter.SetItemText hGroup, 3, "0x" & WHex(MinifilterArray(i).hFilter)
        lvwMinifilter.SetItemText hGroup, 4, MinifilterArray(i).Altitude

        ' FilterFunc 回调
        For j = 0 To 10
            If MinifilterArray(i).FilterFunc(j) = NULL Then Continue For
            hItem = lvwMinifilter.AddItem(hGroup, FilterFuncName(j))
            lvwMinifilter.SetItemText hItem, 1, "0x" & WHex(MinifilterArray(i).FilterFunc(j))
            lvwMinifilter.SetItemText hItem, 2, GetDriverPathByAddr(MinifilterArray(i).FilterFunc(j))
        Next

        ' MajorFunction 回调
        If MinifilterArray(i).bMajorFunction Then
            For j = 0 To UBound(MinifilterArray(i).MajorFunction)
                If MinifilterArray(i).MajorFunction(j).PreOperation = NULL AndAlso _
                   MinifilterArray(i).MajorFunction(j).PostOperation = NULL Then Continue For

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
        End If
    Next

    Return filterCount
End Function

'' 辅助函数：从缓冲区读取Unicode字符串（通用）
Sub ReadUnicodeString(ByVal pBase As Byte Ptr, ByVal offset As ULong, ByVal length As ULong, ByVal outStr As WString Ptr)
    *outStr = ""
    If length = 0 Then Exit Sub
    
    Dim charCnt As ULong = length \ 2
    Dim pSrc As WString Ptr = CPtr(WString Ptr, pBase + offset)
    
    *outStr = Wstring(charCnt, WStr(" "))
    memcpy outStr, pSrc, length
End Sub

Function EnumMinifilterInstances(lvwInstance As Class_ListView, ByVal FilterName As WString Ptr) As Integer
    Dim hFltdll   As HMODULE = NULL
    Dim hFind     As HANDLE = NULL
    Dim retLen    As DWORD = 0
    Dim count     As Integer = 0
    Dim hr        As HRESULT
    Dim buffer(0 To 4095) As Byte  '' 1024*4 足够存储所有实例信息
    Const bufSize As DWORD = 4096

    InitListViewData lvwInstance

    '' 【修复】允许 FilterName=NULL（枚举所有实例，fltmc默认行为）
    '' 仅禁止空字符串，不禁止NULL

    '' 加载fltlib.dll（系统筛选器库）
    hFltdll = DyLibLoad("FltLib.dll")
    If hFltdll = 0 Then
        Print "加载 FltLib.dll 失败"
        Return -1
    End If

    '' 绑定API函数
    Dim FilterInstanceFindFirst As Function( _
        ByVal lpFilterName As LPCWSTR, _
        ByVal dwInformationClass As INSTANCE_INFORMATION_CLASS, _
        ByVal lpBuffer As LPVOID, _
        ByVal dwBufferSize As DWORD, _
        ByRef lpBytesReturned As DWORD, _
        ByRef lpFindHandle As HANDLE) As HRESULT

    Dim FilterInstanceFindNext As Function( _
        ByVal hFindHandle As HANDLE, _
        ByVal dwInformationClass As INSTANCE_INFORMATION_CLASS, _
        ByVal lpBuffer As LPVOID, _
        ByVal dwBufferSize As DWORD, _
        ByRef lpBytesReturned As DWORD) As HRESULT

    Dim FilterInstanceFindClose As Sub(ByVal hFindHandle As HANDLE)

    FilterInstanceFindFirst = DyLibSymbol(hFltdll, "FilterInstanceFindFirst")
    FilterInstanceFindNext  = DyLibSymbol(hFltdll, "FilterInstanceFindNext")
    FilterInstanceFindClose = DyLibSymbol(hFltdll, "FilterInstanceFindClose")

    If FilterInstanceFindFirst = NULL Or FilterInstanceFindNext=NULL Or FilterInstanceFindClose=NULL Then
        Print "获取API地址失败"
        DyLibFree hFltdll
        Return -1
    End If

    '' ===================== 核心修复 =====================
    '' 1. 使用 fltmc 原生信息类：InstanceAggregateStandardInformation
    '' 2. 支持NULL枚举所有实例
    '' ====================================================
    ZeroMemory(@buffer(0), bufSize)
    hr = FilterInstanceFindFirst( _
        FilterName, _
        InstanceAggregateStandardInformation, _
        @buffer(0), _
        bufSize, _
        retLen, _
        hFind)

    '' 无实例/枚举结束（0x80070103 = ERROR_NO_MORE_ITEMS）
    If hr = &H80070103 Then
        Print "无筛选器实例"
        Goto CLEANUP
    End If
    '' 其他错误
    If hr <> 0 Then
        Print "FilterInstanceFindFirst 失败，错误码：0x"; Hex(hr)
        Goto CLEANUP
    End If

    '' 循环遍历所有实例
    Do
        '' 映射到正确的结构体
        Dim pInfo As INSTANCE_AGGREGATE_STANDARD_INFORMATION Ptr = _
            CPtr(INSTANCE_AGGREGATE_STANDARD_INFORMATION Ptr, @buffer(0))

        count += 1

        '' ===================== 读取真实数据（无硬编码） =====================
        Dim strFilter As StringW = ""
        ReadUnicodeString @buffer(0), pInfo->MiniFilterType.FilterNameBufferOffset, pInfo->MiniFilterType.FilterNameLength, strFilter.WstrPtr

        Dim strVolume As StringW = ""
        ReadUnicodeString @buffer(0), pInfo->MiniFilterType.VolumeNameBufferOffset, pInfo->MiniFilterType.VolumeNameLength, strVolume.WstrPtr

        Dim strInstance As StringW = ""
        ReadUnicodeString @buffer(0), pInfo->MiniFilterType.InstanceNameBufferOffset, pInfo->MiniFilterType.InstanceNameLength, strInstance.WstrPtr

        Dim strAltitude As StringW = ""
        ReadUnicodeString @buffer(0), pInfo->MiniFilterType.AltitudeBufferOffset, pInfo->MiniFilterType.AltitudeLength, strAltitude.WstrPtr
        Dim strFrame    As StringW = WStr(pInfo->MinifilterType.FrameID)           '' 框架
        Dim strFeatures As StringW = "0x" & WHex(pInfo->MinifilterType.SupportedFeatures) '' SprtFtrs
        Dim strStatus   As StringW = "0x" & WHex(pInfo->MiniFilterType.dwFlags)
        '' ====================================================================

        '' 控制台输出（和fltmc完全一致）
        Print "筛选器："; strFilter; _
              "  卷："; strVolume; _
              "  高度："; strAltitude; _
              "  实例："; strInstance

        '' 添加到列表视图（匹配fltmc所有列）
        AddItemColListEx lvwInstance, 7,,, _
            WStr(strFilter), _
            WStr(strVolume), _
            WStr(strAltitude), _
            WStr(strInstance), _
            WStr(strFrame), _
            WStr(strFeatures), _
            WStr(strStatus)

        '' 读取下一个实例
        ZeroMemory(@buffer(0), bufSize)
        hr = FilterInstanceFindNext(hFind, InstanceAggregateStandardInformation, @buffer(0), bufSize, retLen)

    Loop While hr = 0 '' 0=成功，继续循环

CLEANUP:
    '' 安全释放资源
    If hFind <> NULL Then FilterInstanceFindClose(hFind)
    If hFltdll <> NULL Then DyLibFree(hFltdll)
    CommitListViewView lvwInstance

    '' 错误：返回-1；成功：返回实例数量
    If hr <> 0 And hr <> &H80070103 Then Return -1 Else Return count
End Function

' 功能：删除指定实例
' 参数：筛选器名、实例名、卷名
' 返回：0=成功，非0=失败
Function DetachFilterInstance(ByVal pFilterName As WString Ptr, ByVal pVolumeName As WString Ptr, ByVal pInstanceName As WString Ptr) As Boolean
    Dim hFltLib As HMODULE = DyLibLoad("FltLib.dll")
    Dim hr As HRESULT = &H8000FFFF
    If hFltLib = NULL Then Return hr

    ' API：FilterDetach = 分离/删除实例
    Dim FilterDetach As Function(ByVal lpFilterName As WString Ptr, ByVal lpVolumeName As WString Ptr, ByVal lpInstanceName As WString Ptr) As HRESULT
    FilterDetach = DyLibSymbol(hFltLib, "FilterDetach")

    If FilterDetach Then
        hr = FilterDetach(pFilterName, pVolumeName, pInstanceName)
        Print "删除指定实例：" & IIf(hr=0,"成功","失败") & " 错误码: 0x" & Hex(hr)
    End If

    DyLibFree hFltLib
    Return (hr = S_OK)
End Function

' 功能：删除指定筛选器的全部实例
' 参数：筛选器名
' 返回：删除成功的数量
Function DetachAllFilterInstances(ByVal pFilterName As WString Ptr) As Integer
    Dim hFltLib As HMODULE = DyLibLoad("FltLib.dll")
    Dim hFind As HANDLE = NULL
    Dim retLen As DWORD, hr As HRESULT
    Dim buffer(0 To 4095) As Byte
    Dim count As Integer = 0
    If hFltLib = NULL Then Return 0

    Dim FilterInstanceFindFirst As Function(ByVal lpFilterName As WString Ptr, ByVal ic As INSTANCE_INFORMATION_CLASS, ByVal pBuf As LPVOID, ByVal bufSz As DWORD, ByRef ret As DWORD, ByRef hFind As HANDLE) As HRESULT
    Dim FilterInstanceFindNext As Function(ByVal hFind As HANDLE, ByVal ic As INSTANCE_INFORMATION_CLASS, ByVal pBuf As LPVOID, ByVal bufSz As DWORD, ByRef ret As DWORD) As HRESULT
    Dim FilterInstanceFindClose As Sub(ByVal hFind As HANDLE)
    Dim FilterDetach As Function(ByVal lpFilterName As WString Ptr, ByVal lpVolumeName As WString Ptr, ByVal lpInstanceName As WString Ptr) As HRESULT

    FilterInstanceFindFirst = DyLibSymbol(hFltLib, "FilterInstanceFindFirst")
    FilterInstanceFindNext = DyLibSymbol(hFltLib, "FilterInstanceFindNext")
    FilterInstanceFindClose = DyLibSymbol(hFltLib, "FilterInstanceFindClose")
    FilterDetach = DyLibSymbol(hFltLib, "FilterDetach")

    If FilterInstanceFindFirst = NULL OrElse FilterDetach = NULL Then Goto CLEANUP

    hr = FilterInstanceFindFirst(pFilterName, InstanceAggregateStandardInformation, @buffer(0), 4096, retLen, hFind)
    If hr = &H80070103 Then Goto CLEANUP

    ' 遍历所有实例并删除
    Do
        Dim pInfo As INSTANCE_AGGREGATE_STANDARD_INFORMATION Ptr = CPtr(INSTANCE_AGGREGATE_STANDARD_INFORMATION Ptr, @buffer(0))
        Dim strInst As StringW = "", strVol As StringW = ""
        ReadUnicodeString @buffer(0), pInfo->MinifilterType.InstanceNameBufferOffset, pInfo->MinifilterType.InstanceNameLength, strInst.WstrPtr
        ReadUnicodeString @buffer(0), pInfo->MinifilterType.VolumeNameBufferOffset, pInfo->MinifilterType.VolumeNameLength, strVol.WstrPtr

        ' 执行删除
        Dim result As HRESULT = FilterDetach(pFilterName, strVol.WstrPtr, strInst.WstrPtr)
        If result = S_OK Then
            count += 1
        Else
            Print "分离实例失败,strInst=" & strInst & " strVol=" & strVol & " ,Error=" & WinErrorMsg(result) & " result=0x" & WHex(result)
        End If

        ZeroMemory(@buffer(0), 4096)
        hr = FilterInstanceFindNext(hFind, InstanceAggregateStandardInformation, @buffer(0), 4096, retLen)
    Loop While hr = 0

CLEANUP:
    If hFind Then FilterInstanceFindClose(hFind)
    DyLibFree hFltLib
    Print "总计删除实例数量：" & count
    Return count
End Function

' 功能：卸载筛选器驱动
' 参数：筛选器名
' 返回：0=成功，非0=失败
Function UnloadFilter(ByVal pFilterName As WString Ptr) As Boolean
    Dim hFltLib As HMODULE = DyLibLoad("FltLib.dll")
    Dim hr As HRESULT = &H8000FFFF
    If hFltLib = NULL Then Return hr

    ' API：FilterUnload = 卸载筛选器
    Dim pUnload As Function(ByVal lpFilterName As WString Ptr) As HRESULT
    pUnload = DyLibSymbol(hFltLib, "FilterUnload")

    If pUnload Then
        hr = pUnload(pFilterName)
        Print "卸载筛选器：" & IIf(hr=0,"成功","失败") & " 错误码: 0x" & Hex(hr)
    End If

    DyLibFree hFltLib
    Return (hr = S_OK)
End Function

Function EnumNdisMiniport(lvwNdisMiniport As Class_mCtrlTreeList) As ULong
    Dim MiniportArray() As NDIS_MINIPORT_ENUM_ENTRY
    Dim lpRet As DWORD
    Dim i As Long, j As Long, k As Long

    ' ----------------------------------
    ' 第一次 IOCTL → 获取所需缓冲区大小
    ' ----------------------------------
    SetLastError 0
    IoControl hDrv, IOCTL_EnumNdisMiniport, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[EnumNdisMiniport] 第一次 IOCTL 失败: " & WinErrorMsg(GetLastError) & " (" & GetLastError & ")"
        Return 0
    End If
    
    Print "lpRet:" & lpRet
    Dim entryCount As Long
    entryCount = lpRet \ SizeOf(NDIS_MINIPORT_ENUM_ENTRY)
    Print "entryCount:" & entryCount
    If entryCount <= 0 Then Return 0

    ' ----------------------------------
    ' 分配数组并获取实际数据
    ' ----------------------------------
    ReDim MiniportArray(entryCount - 1) As NDIS_MINIPORT_ENUM_ENTRY

    SetLastError 0
    IoControl hDrv, IOCTL_EnumNdisMiniport, NULL, 0, @MiniportArray(0), lpRet, @lpRet
    If GetLastError <> 0 Then
        Print "[EnumNdisMiniport] 第二次 IOCTL 失败: " & WinErrorMsg(GetLastError) & " (" & GetLastError & ")"
        Return 0
    End If

    ' ====== CLI 旁路输出 ======
    If g_CuiMode Then
        Dim colNames(4) As StringW = { _
            "名称", "类型", "标识信息", "驱动/映像", "地址" _
        }
        Dim maxRows As Long = 0
        For i = 0 To entryCount - 1
            maxRows += 1                                 ' 根行
            maxRows += MiniportArray(i).FilterCount      ' Filter 子行
            maxRows += MiniportArray(i).OpenCount        ' Open 子行
        Next
        Dim rows(maxRows - 1) As CuiExportRow
        Dim rowIdx As Long = 0

        For i = 0 To entryCount - 1
            Dim entry As NDIS_MINIPORT_ENUM_ENTRY = MiniportArray(i)

            ' --- 根行：Miniport ---
            rows(rowIdx).IndentLevel = 0
            rows(rowIdx).IsGroup = True
            rows(rowIdx).GroupLabel = entry.pAdapterInstanceName
            rows(rowIdx).ColCount = 5
            rows(rowIdx).Cols(0) = entry.pAdapterInstanceName
            rows(rowIdx).Cols(1) = "Miniport"
            rows(rowIdx).Cols(2) = entry.BaseName
            rows(rowIdx).Cols(3) = GetCorrectPath(entry.DriverPath)
            rows(rowIdx).Cols(4) = "0x" & WHex(entry.MiniportBlock)
            rowIdx += 1

            ' --- Filter 子行 ---
            For j = 0 To entry.FilterCount - 1
                Dim f As NDIS_ENUM_FILTER_ENTRY = entry.Filters(j)
                If f.FilterBlock = NULL Then Continue For

                rows(rowIdx).IndentLevel = 1
                rows(rowIdx).IsGroup = False
                rows(rowIdx).ColCount = 5
                rows(rowIdx).Cols(0) = f.FriendlyName
                rows(rowIdx).Cols(1) = "Filter"
                rows(rowIdx).Cols(2) = f.InstanceName
                rows(rowIdx).Cols(3) = GetCorrectPath(f.DriverImageName)
                rows(rowIdx).Cols(4) = "0x" & WHex(f.FilterBlock)
                rowIdx += 1
            Next

            ' --- Open 子行 ---
            For k = 0 To entry.OpenCount - 1
                Dim o As NDIS_ENUM_OPEN_ENTRY = entry.Opens(k)
                If o.OpenBlock = NULL Then Continue For

                rows(rowIdx).IndentLevel = 1
                rows(rowIdx).IsGroup = False
                rows(rowIdx).ColCount = 5
                rows(rowIdx).Cols(0) = o.ProtocolName
                rows(rowIdx).Cols(1) = "Open"
                rows(rowIdx).Cols(2) = o.ProtocolImageName
                rows(rowIdx).Cols(3) = GetCorrectPath(o.ProtocolImageName)
                rows(rowIdx).Cols(4) = "0x" & WHex(o.OpenBlock)
                rowIdx += 1
            Next
        Next

        If rowIdx > 0 Then
            ReDim Preserve rows(rowIdx - 1) As CuiExportRow
            CuiPrintTreeListData("NDIS Miniport", rows(), colNames(), 5)
        Else
            Dim emptyRows() As CuiExportRow
            ReDim emptyRows(-1) As CuiExportRow
            CuiPrintTreeListData("NDIS Miniport", emptyRows(), colNames(), 5)
        End If
        Return entryCount
    End If

    ' ----------------------------------
    ' GUI TreeList 输出（含类型列）
    ' ----------------------------------
    lvwNdisMiniport.DeleteItem MC_TLI_ROOT

    Dim hGroup As MC_HTREELISTITEM, hItem As MC_HTREELISTITEM
    For i = 0 To entryCount - 1
        Dim entry As NDIS_MINIPORT_ENUM_ENTRY = MiniportArray(i)

        ' 根节点：Miniport
        hGroup = lvwNdisMiniport.AddItem(MC_TLI_ROOT, entry.pAdapterInstanceName)
        lvwNdisMiniport.SetItemText hGroup, 1, WStr("Miniport")
        lvwNdisMiniport.SetItemText hGroup, 2, entry.BaseName
        lvwNdisMiniport.SetItemText hGroup, 3, GetCorrectPath(entry.DriverPath)
        lvwNdisMiniport.SetItemText hGroup, 4, "0x" & WHex(entry.MiniportBlock)

        ' Filter 子项
        For j = 0 To entry.FilterCount - 1
            Dim f As NDIS_ENUM_FILTER_ENTRY = entry.Filters(j)
            If f.FilterBlock = NULL Then Continue For

            hItem = lvwNdisMiniport.AddItem(hGroup, f.FriendlyName)
            lvwNdisMiniport.SetItemText hItem, 1, WStr("Filter")
            lvwNdisMiniport.SetItemText hItem, 2, f.InstanceName
            lvwNdisMiniport.SetItemText hItem, 3, GetCorrectPath(f.DriverImageName)
            lvwNdisMiniport.SetItemText hItem, 4, "0x" & WHex(f.FilterBlock)
        Next

        ' Open 子项
        For k = 0 To entry.OpenCount - 1
            Dim o As NDIS_ENUM_OPEN_ENTRY = entry.Opens(k)
            If o.OpenBlock = NULL Then Continue For

            hItem = lvwNdisMiniport.AddItem(hGroup, o.ProtocolName)
            lvwNdisMiniport.SetItemText hItem, 1, WStr("Open")
            lvwNdisMiniport.SetItemText hItem, 2, o.ProtocolImageName
            lvwNdisMiniport.SetItemText hItem, 3, GetCorrectPath(o.ProtocolImageName)
            lvwNdisMiniport.SetItemText hItem, 4, "0x" & WHex(o.OpenBlock)
        Next
    Next

    Return entryCount
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

' ===================== 辅助：递归构建 FilterDriver CUI 行 =====================
Private Sub CuiBuildAttachTree( _
    nodes() As NODE_INFO, _
    ByVal nodeCount As ULong, _
    ByVal parentObj As ULONG64, _
    ByVal depth As Long, _
    rows() As CuiExportRow, _
    ByRef rowIdx As Long, _
    marked() As MARKED_DEVICE, _
    ByVal markedCount As ULong, _
    ByVal ShowAllDevices As Boolean, _
    ByVal ShowBusDevices As Boolean _
)
    For i As Long = 0 To nodeCount - 1
        Dim nd As ATTACHED_DEVICE_NODE = nodes(i).node
        If nd.ParentDeviceObject <> parentObj Then Continue For
        If IsDeviceMarked(nd.DeviceObject, marked(), markedCount) Then Continue For
        
        ' 根节点过滤（仅 depth=0 时生效）
        If depth = 0 Then
            If ShowAllDevices = False Then
                If ShowBusDevices = False AndAlso IsBusDevice(nd.DriverName) Then Continue For
                If HasValidChildren(nd.DeviceObject, nodes(), nodeCount, marked(), markedCount) = False Then Continue For
            End If
        End If
        
        ' 添加当前节点（5 列）
        rows(rowIdx).IndentLevel = depth
        rows(rowIdx).IsGroup = HasValidChildren(nd.DeviceObject, nodes(), nodeCount, marked(), markedCount)
        rows(rowIdx).ColCount = 5
        rows(rowIdx).Cols(0) = CleanDriverName(nd.DriverName)
        rows(rowIdx).Cols(1) = "0x" & WHex(nd.DriverObject)
        rows(rowIdx).Cols(2) = GetCorrectPathW(CleanDriverName(nd.DriverPath))
        rows(rowIdx).Cols(3) = CleanDriverName(nd.DeviceName)
        rows(rowIdx).Cols(4) = "0x" & WHex(nd.DeviceObject)
        rowIdx += 1
        
        ' 递归子节点
        CuiBuildAttachTree nodes(), nodeCount, nd.DeviceObject, depth + 1, _
            rows(), rowIdx, marked(), markedCount, ShowAllDevices, ShowBusDevices
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
    'Print "Marked " & markedCount & " devices for filtering"
    
    ' ====== CLI 旁路输出 ======
    If g_CuiMode Then
        Dim colNames(4) As StringW = { _
            "驱动名称", "驱动对象地址", "路径", "设备名称", "设备对象地址" _
        }
        
        Dim maxRows As Long = nodeCount
        Dim rows(maxRows - 1) As CuiExportRow
        Dim rowIdx As Long = 0
        
        ' 从根节点（Parent=0）开始递归构建
        CuiBuildAttachTree nodes(), nodeCount, 0, 0, rows(), rowIdx, _
            marked(), markedCount, ShowAllDevices, ShowBusDevices
        
        If rowIdx > 0 Then
            ReDim Preserve rows(rowIdx - 1) As CuiExportRow
            CuiPrintTreeListData("FILTERDRIVER", rows(), colNames(), 5)
        Else
            Dim emptyRows() As CuiExportRow
            ReDim emptyRows(-1) As CuiExportRow
            CuiPrintTreeListData("FILTERDRIVER", emptyRows(), colNames(), 5)
        End If
        
        Deallocate pBuffer
        Return rowIdx
    End If
    
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

