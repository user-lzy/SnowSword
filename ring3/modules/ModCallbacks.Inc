Type CallbackInfo
    TheType As Wstring * 50
    Func As Integer
    TheContext As Integer
    'DriverName As Wstring * 260
End Type

Type ObCallbackInfo
    TheType As Wstring * 50
    ObHandle As Integer
    Func As Integer
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

Type ATTACHED_DEVICE_NODE
    DeviceObject As ULONG64
    DriverObject As ULONG64
    DriverName As Wstring * MAX_PATH
    DriverPath As Wstring * MAX_PATH
    ' struct _ATTACHED_DEVICE_NODE* NextAttached ' 指向下一个Attached设备
End Type

Type ATTACH_DEVICE_INFO
    ' ATTACHED_DEVICE_NODE DeviceChain  ' 设备链头节点
    DriverName As Wstring * MAX_PATH
    Devices(9) As ATTACHED_DEVICE_NODE ' 假设最多10个设备
    TotalDevices As ULong                ' 总设备数
End Type
Type PATTACH_DEVICE_INFO As ATTACH_DEVICE_INFO Ptr

Dim Shared CallbackArray(Any) As CallbackInfo

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
    IoControl hDrv, IOCTL_EnumCallbacks, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetCallbackList]第一次IOCTL失败:" & WinErrorMsg(GetLastError) & GetLastError
        Exit Sub
    End If
    ReDim CallbackArray(lpRet / SizeOf(CallbackInfo) - 1) As CallbackInfo
    IoControl hDrv, IOCTL_EnumCallbacks, NULL, 0, @CallbackArray(0), (UBound(CallbackArray) + 1) * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To UBound(CallbackArray)
        If Not (InStr(CallbackArray(i).TheType, "Ob") > 0 And CallbackArray(i).Func = 0) Then
            NewIndex = lvwCallbacks.AddItem(CallbackArray(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(CallbackArray(i).Func)
            lvwCallbacks.SetItemText NewIndex, 2, GetDriverPathByAddr(CallbackArray(i).Func)
            If InStr(CallbackArray(i).TheType, "Ob") <= 0 Then lvwCallbacks.SetItemText NewIndex, 3, "0x" & Hex(CallbackArray(i).TheContext)
        End If
    Next
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
    End If'/
    IoControl hDrv, IOCTL_EnumIoTimers, NULL, 0, @SystemTimers(0), 64 * SizeOf(SYSTEM_TIMER)
    For i As Integer = 0 To 63
        If (SystemTimers(i).Func <> 0) Then
            NewIndex = lvwKernelTimer.AddItem(SystemTimers(i).strType)
            lvwKernelTimer.SetItemText NewIndex, 1, "0x" & Hex(SystemTimers(i).TimerObject)
            lvwKernelTimer.SetItemText NewIndex, 2, "0x" & Hex(SystemTimers(i).Func)
            
            ' SystemTimers(i).uFlag
            lvwKernelTimer.SetItemText NewIndex, 4, WStr(SystemTimers(i).uType)'IIf(SystemTimers(i).st, "已启动", "已停止")
            lvwKernelTimer.SetItemText NewIndex, 5, GetDriverPathByAddr(SystemTimers(i).Func)'Callbacks(i).DriverName
        End If
    Next
    memset @SystemTimers(0), 0, 64 * SizeOf(SYSTEM_TIMER)
    IoControl hDrv, IOCTL_EnumDpcTimers, NULL, 0, @SystemTimers(0), 64 * SizeOf(SYSTEM_TIMER)
    For i As Integer = 0 To 63
        If (SystemTimers(i).strType <> "") Then
            NewIndex = lvwKernelTimer.AddItem(SystemTimers(i).strType)
            lvwKernelTimer.SetItemText NewIndex, 1, "0x" & WHex(SystemTimers(i).TimerObject)
            lvwKernelTimer.SetItemText NewIndex, 2, "0x" & WHex(SystemTimers(i).Func)
            lvwKernelTimer.SetItemText NewIndex, 3, WStr(SystemTimers(i).Period)
            lvwKernelTimer.SetItemText NewIndex, 4, WStr(SystemTimers(i).uType)'IIf(SystemTimers(i).st, True
            lvwKernelTimer.SetItemText NewIndex, 5, GetDriverPathByAddr(SystemTimers(i).Func)
        End If
    Next
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
        Print MinifilterArray(i).wName
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

'==============================================================
' 向驱动查询单个驱动对象上挂接的所有过滤设备
' pDriverObject  : 要查询的驱动对象地址（64 位）
' AttachInfo     : 调用者提供的缓冲区，由本函数填充
' 返回值         : 实际挂接的设备数量（>=0），<0 表示失败
'==============================================================
Function EnumAttachDevice(ByVal pDriverObject As ULONG64, ByRef AttachInfo As ATTACH_DEVICE_INFO) As Long

    ' 输入缓冲区只放一个 64 位地址
    Dim inBuf As ULONG64 = pDriverObject
    Dim cbRet  As DWORD

    ' 先清输出结构
    ZeroMemory(@AttachInfo, SizeOf(ATTACH_DEVICE_INFO))

    ' 第一次调用：驱动返回需要的大小（此处我们直接给结构大小即可）
    If IoControl(hDrv, IOCTL_EnumAttachDevices, @inBuf, SizeOf(inBuf), @AttachInfo, SizeOf(ATTACH_DEVICE_INFO), @cbRet) = 0 Then
        Print "EnumAttachDevice IoControl 失败：" & WinErrorMsg(GetLastError) & " " & SizeOf(ATTACH_DEVICE_INFO) & "-" & cbRet
        Return -1
    EndIf

    ' 驱动把实际数量写在 AttachInfo.TotalDevices
    Return AttachInfo.TotalDevices
End Function

'==============================================================
' 枚举系统中所有驱动各自的挂接设备，并填到 TreeList
' lvwAttachDevice : 事先已建好 3 列的 mCtrlTreeList 对象
' 返回值          : 总挂接设备数量（所有驱动累加）
'==============================================================
Function EnumAttachDevices(ByVal lvwAttachDevice As Class_mCtrlTreeList) As ULong
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
            FrmMain.Check3.Value = False
            Return 0
        End If
    End If

    ' 清空列表
    lvwAttachDevice.DeleteItem MC_TLI_ROOT

    ' 先取一遍系统模块列表，拿到所有内核驱动的基址
    Dim bufSize As ULong = &h4000
    Dim buffer  As PVOID = Allocate(bufSize)
    If buffer = NULL Then Return 0

    Dim st As NTSTATUS = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufSize, @bufSize)
    If st = STATUS_INFO_LENGTH_MISMATCH Then
        buffer = Reallocate(buffer, bufSize)
        If buffer = NULL Then Return 0
        st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufSize, NULL)
    End If
    If st < 0 Then
        Deallocate buffer
        Return 0
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    Dim TotalAttached As ULong = 0          ' 累加器

    For i As Integer = 0 To smi->Count - 1
        Dim baseAddr As PVOID = Cast(PVOID, smi->Modules(i).dwBase)
        Dim imgPath  As ZString * MAX_PATH  = smi->Modules(i).ImageName
        imgPath = GetCorrectPath(imgPath)

        ' 取驱动对象地址
        Dim drvObjAddr As Integer = GetDriverObjectAddr(baseAddr)
        If drvObjAddr = 0 Then Continue For
        
        ' 查询该驱动上挂了多少过滤设备
        Dim attachInfo As ATTACH_DEVICE_INFO
        RtlZeroMemory(@attachInfo, SizeOf(ATTACH_DEVICE_INFO))
        
        Dim nDev As Long = EnumAttachDevice(drvObjAddr, attachInfo)
        If nDev <= 0 Then Continue For ' 无挂接或出错
        'Print "imgPath:" & imgPath
        'Print "nDev:" & nDev
        
        ' 把本驱动的信息作为父节点
        Dim hGroup As MC_HTREELISTITEM
        hGroup = lvwAttachDevice.AddItem(MC_TLI_ROOT, attachInfo.DriverName)
        lvwAttachDevice.SetItemText hGroup, 1, "0x" & WHex(drvObjAddr)
        lvwAttachDevice.SetItemText hGroup, 2, imgPath

        ' 逐个挂接设备填到子节点
        For j As Integer = 0 To nDev - 1
            'Print "DriverName:" & attachInfo.Devices(j).DriverName & " DriverObject:" & "0x" & WHex(attachInfo.Devices(j).DriverObject)
            Dim hItem As MC_HTREELISTITEM
            hItem = lvwAttachDevice.AddItem(hGroup, attachInfo.Devices(j).DriverName)
            lvwAttachDevice.SetItemText hItem, 1, "0x" & WHex(attachInfo.Devices(j).DriverObject)
            lvwAttachDevice.SetItemText hItem, 2, GetCorrectPathW(attachInfo.Devices(j).DriverPath)
        Next

        TotalAttached += nDev
    Next

    Deallocate buffer
    Return TotalAttached
End Function