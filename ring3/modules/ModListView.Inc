#include "win/commdlg.bi"

Enum Interface
    ForegroundWindow = 0
    Process
    Module
    Thread
    Windows
    Memory
    Handles
    WebConnect
    Hook
    Privilege
    MsgHook
    EventHook
    Hotkey
    
    KernelModule
    KernelThread
    WorkItemThread
    
    File
    Callbacks
    WfpCallout
    WfpFilter
    UnlockTheFile
    Registry
    Service
    KernelTimer
    IOFunction
    IPScan
    Sniffer
    ProcessTimer
    GDT
    IDT
    SSDT
    ShadowSSDT
    Minifilter
    FilterDriver
    ViolentCheck
    HalDispatch
    HalPrivateDispatch
    ObjectHook
    
    EtwProvider
    WinsockSPI
    TaskScheduler
    
    MaxInterfaceCount
End Enum

Type CurrentDriverInfo
    DriverBase As UInteger
    DriverSize As UInteger
    DriverObject As UInteger
    DriverName As SafeStringW_NTS ' StringW
    DriverPath As SafeStringW_NTS ' StringW
End Type

Type CurrentModuleInfo
    ModuleName As SafeStringW_NTS ' StringW
    ModuleHandle As Any Ptr
End Type

Type CURRENT_INFORMATION
    hListView As hWnd
    intType As Interface
    ProcessId As DWORD
    CurrentModule As CurrentModuleInfo
    CurrentDriver As CurrentDriverInfo
End Type

Type TempTextNode
    Buf As WCHAR Ptr
End Type

Type LVRow
    ColCount As Long
    Cols As Wstring Ptr Ptr   ' 指向 Wstring* 数组
    Fore As COLORREF
    Back As COLORREF
End Type

Type ListViewSnapshot
    Rows(Any) As LVRow Ptr
    RowCount As Long
End Type

Type ListViewContext
    hLV As HWND

    Snap As ListViewSnapshot Ptr

    VisibleIndex(Any) As Long
    VisibleCount As Long

    Filter As Wstring Ptr
    
    SortedColumn As Long       ' 当前排序列索引
    SortAscending As Boolean ' 升降序标记
    
    ' ===================== 新增：列类型缓存（性能优化核心） =====================
    ColumnIsNumber(Any) As Boolean  ' 缓存列是否为纯数字列
End Type

' 2. 模块缓存核心结构体：存储单个模块的所有状态
Type ViewModuleCache
    IsCached As Boolean          ' 标记：是否已缓存数据
    pSnap As ListViewSnapshot Ptr ' 【新增】深拷贝的列表数据快照
    SelectedIndex As Long        ' 选中项索引
    TopVisibleIndex As Long      ' 列表滚动顶部位置（恢复滚动条）
    ItemCount As Long            ' 项数量
End Type

' 3. 全局缓存数组：为每个模块分配独立缓存（核心）
Dim Shared g_ViewCache(MaxInterfaceCount - 1) As ViewModuleCache

Const FB_Red As COLORREF = BGR(255, 0, 0)
Const FB_LightPinkRed As COLORREF = BGR(240, 210, 210)
Const FB_LightRed As COLORREF = BGR(250, 225, 225)
Const FB_GoldenYellow As COLORREF = BGR(255, 240, 128)
Const FB_White As COLORREF = BGR(255, 255, 255)
Const FB_Black As COLORREF = BGR(0, 0, 0)
Const FB_LightGreen As COLORREF = BGR(50, 200, 50)

Dim Shared CurrentInformationArray() As CURRENT_INFORMATION
Dim Shared pOldWindowProc As PVOID

Public Sub InitializeListView(ByVal ListView As Class_ListView, ByVal TreeList As Class_mCtrlTreeList, ByVal TreeView As Class_TreeView)
    ListView.DeleteAllColumn
    ListView.DeleteAllItems ' 此处是否有问题
    TreeView.DeleteAllItems
    TreeList.DeleteItem MC_TLI_ROOT
    For i As Integer = TreeList.GetColumnCount - 1 To 0 Step -1
        TreeList.DeleteColumn i
    Next
    'Print "CurrentType:" & CurrentType & __FUNCTION__
    With ListView
        Select Case CurrentInformation.intType
            Case ForegroundWindow
                .AddColumn "窗口标题",, 400
                .AddColumn "进程名",, 200
                .AddColumn "状态",, 200
            Case Process
                .AddColumn "PID",, 100
                .AddColumn "PPID",, 100
                .AddColumn "EPROCESS",, 270
                .AddColumn "名称",, 200
                .AddColumn "路径",, 500
                .AddColumn "会话-用户名",, 200
                .AddColumn "命令行参数",, 500
                .AddColumn "公司名",, 200
            Case Module
                .AddColumn "序号",, 50
                .AddColumn "名称",, 200
                .AddColumn "基址",, 230
                .AddColumn "大小",, 150
                .AddColumn "路径",, 500
                .AddColumn "公司名",, 200
            Case Thread
                '.AddColumn "序号",, 50
                .AddColumn "TID",, 100
                .AddColumn "优先级",, 120
                .AddColumn "状态",, 100
                .AddColumn "ETHREAD",, 270
                .AddColumn "线程入口",, 150
                .AddColumn "Teb",, 150
                '.AddColumn "大小",, 50
                .AddColumn "所属模块",, 1000
                .AddColumn "公司名",, 200
            Case Memory
                .AddColumn "地址",, 250
                .AddColumn "大小",, 100
                .AddColumn "属性",, 150
                .AddColumn "状态",, 150
                .AddColumn "类型",, 150
                .AddColumn "基址",, 250
                .AddColumn "对应模块",, 400
                '.AddColumn "公司名",, 200
            Case Windows
                .AddColumn "句柄",, 150
                .AddColumn "名称",, 300
                .AddColumn "类名",, 300
                .AddColumn "TID",, 100
                .AddColumn "所属模块",, 400
                '.AddColumn "公司名",, 200
            Case Handles
                .AddColumn "类型",, 200
                .AddColumn "名称",, 300
                .AddColumn "值",, 100
                .AddColumn "访问权限",, 150
                .AddColumn "对象地址",, 200
            Case Privilege
                .AddColumn "LUID",, 100
                .AddColumn "名称",, 400
                .AddColumn "状态",, 200
                .AddColumn "描述",, 400
                .AddColumn "属性",, 100
            Case ProcessTimer
                .AddColumn "定时器ID",, 150
                .AddColumn "超时时间",, 150
                .AddColumn "回调地址",, 250
                .AddColumn "所属模块",, 250
                .AddColumn "所属窗口",, 150
                .AddColumn "所属TID",, 100
                .AddColumn "所属PID",, 100
            Case MsgHook
                .AddColumn "hHook",, 100
                .AddColumn "类型",, 150
                .AddColumn "作用域",, 200
                .AddColumn "回调函数",, 300
                .AddColumn "所属模块",, 350
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
            Case EventHook
                .AddColumn "hHook",, 100
                .AddColumn "作用域",, 200
                .AddColumn "Flags",, 200
                .AddColumn "回调函数",, 300
                .AddColumn "所属模块",, 350
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
                .AddColumn "目标PID",, 100
                .AddColumn "目标TID",, 100
            Case Hotkey
                .AddColumn "hHotkey",, 100
                .AddColumn "热键id",, 200
                .AddColumn "热键组合",, 300
                .AddColumn "所属窗口",, 100
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
                .AddColumn "备注",, 300
            Case KernelModule
                .AddColumn "序号",, 100
                .AddColumn "名称",, 200
                .AddColumn "映像基址",, 230
                .AddColumn "映像大小",, 150
                .AddColumn "路径",, 500
                .AddColumn "驱动对象",, 230
            Case KernelThread
                .AddColumn "TID",, 100
                .AddColumn "优先级",, 120
                .AddColumn "状态",, 100
                .AddColumn "ETHREAD",, 270
                .AddColumn "线程入口",, 150
                .AddColumn "所属模块",, 500
            Case WorkItemThread
                .AddColumn "TID",, 100
                .AddColumn "ETHREAD",, 200
                .AddColumn "优先级",, 100
                .AddColumn "状态",, 100
                .AddColumn "入口地址",, 200
                .AddColumn "所属模块",, 250
            Case SSDT
                .AddColumn "序号",, 100
                .AddColumn "索引",, 100
                .AddColumn "函数名",, 250
                .AddColumn "当前地址",, 250
                .AddColumn "所属模块",, 500
            Case ShadowSSDT
                .AddColumn "序号",, 100
                .AddColumn "Hex索引",, 100
                .AddColumn "函数名",, 250
                .AddColumn "当前地址",, 250
                .AddColumn "所属模块",, 500
            Case GDT
                'If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "CPU", 100
                /'Else
                    TreeList.SetColumn 1, "CPU", 100
                End If'/
                TreeList.AddColumn "GDT Entry/Base", 250
                TreeList.AddColumn "Limit", 100
                TreeList.AddColumn "SEL", 100
                TreeList.AddColumn "SEL Base", 250
                TreeList.AddColumn "SEL Limit", 100
                TreeList.AddColumn "DPL", 100
                TreeList.AddColumn "Present", 100
                TreeList.AddColumn "Granularity", 100
                TreeList.AddColumn "Description", 300
            Case IDT
                'If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "CPU", 100
                /'Else
                    TreeList.SetColumn 1, "CPU", 100
                End If'/
                TreeList.AddColumn "ID", 100
                TreeList.AddColumn "函数名称", 250
                TreeList.AddColumn "当前函数地址", 250
                TreeList.AddColumn "SEL", 100
                TreeList.AddColumn "DPL", 100
                TreeList.AddColumn "所属模块", 500
            Case Callbacks
                .AddColumn "回调类型",, 250
                .AddColumn "回调函数地址",, 280
                .AddColumn "所属模块",, 500
                .AddColumn "回调函数上下文",, 300
                .AddColumn "备注",, 300
            Case Minifilter
                'If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "名称", 250
                /'Else
                    TreeList.SetColumn 0, "名称", 250
                End If'/
                TreeList.AddColumn "回调函数地址", 280
                TreeList.AddColumn "所属模块", 500
                TreeList.AddColumn "PFLT_FILTER", 280
                TreeList.AddColumn "Altitude", 150
            Case FilterDriver
                'If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "驱动名称", 250
                /'Else
                    TreeList.SetColumn 1, "驱动名称", 250
                End If'/
                TreeList.AddColumn "驱动对象地址", 250
                TreeList.AddColumn "路径", 500
                TreeList.AddColumn "设备名称", 250
                TreeList.AddColumn "设备对象地址", 250
            Case WfpFilter
                .AddColumn "FilterId",, 200
                .AddColumn "CalloutId",, 80
                .AddColumn "名称",, 200
                .AddColumn "Action",, 100
                .AddColumn "Callout GUID",, 200
            Case WfpCallout
                .AddColumn "GUID",, 200
                .AddColumn "CalloutId",, 80
                .AddColumn "Classify",, 300
                .AddColumn "Notify",, 300
                .AddColumn "FlowDelete",, 300
                .AddColumn "名称",, 300
            Case 1 ' File
                .AddColumn "名称",, 250
                .ADdColumn "类型",, 200
                .AddColumn "大小",, 100
            Case UnlockTheFile
                .AddColumn "进程名",, 200
                .AddColumn "PID",, 100
                .AddColumn "文件路径",, 300
                .AddColumn "进程路径",, 300
                .AddColumn "文件句柄",, 150
            Case Registry
                .AddColumn "名称",, 200
                .AddColumn "类型",, 100
                .AddColumn "数据",, 200
            Case WebConnect
                .AddColumn "协议",, 100
                .AddColumn "PID",, 100
                .AddColumn "状态",, 150
                .AddColumn "SrcIP",, 200
                .AddColumn "DstIP",, 200
                .AddColumn "SrcPort",, 100
                .AddColumn "DstPort",, 100
                .AddColumn "路径",, 500
            Case Hook
                .AddColumn "Hook类型",, 200
                .AddColumn "地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "代理函数地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "当前字节",, 300
                .AddColumn "原始字节",, 300
            Case Service
                .AddColumn "服务名",, 250
                .AddColumn "状态",, 140
                .AddColumn "服务类型",, 160
                .AddColumn "启动类型",, 160
                .AddColumn "PID",, 150
                .AddColumn "进程路径",, 300
                .AddColumn "服务模块",, 300
                .AddColumn "描述",, 300
            Case KernelTimer
                .AddColumn "定时器类型",, 150
                .AddColumn "对象地址",, 300
                .AddColumn "回调函数地址",, 300
                .AddColumn "周期(ms)",, 150
                .AddColumn "状态",, 100
                .AddColumn "所属模块",, 300
            Case IOFunction
                .AddColumn "名称",, 250
                .AddColumn "当前地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "原始地址",, 300
                .AddColumn "所属模块",, 600
                ' .AddColumn "AddressReference",, 300
            Case IPScan
                .AddColumn "IP",, 250
                .AddColumn "主机名",, 300
                .AddColumn "MAC",, 300
            Case Sniffer
                .AddColumn "协议",, 100
                .AddColumn "SrcIP",, 200
                .AddColumn "DstIP",, 200
                .AddColumn "SrcPort",, 100
                .AddColumn "DstPort",, 100
                .AddColumn "数据长度",, 150
            Case ViolentCheck
                .AddColumn "索引",, 50
                .AddColumn "类型",, 100
                .AddColumn "地址",, 300
                .AddColumn "所属模块",, 600
            Case HalDispatch, HalPrivateDispatch
                .AddColumn "索引",, 100
                .AddColumn "名称",, 250
                .AddColumn "地址",, 300
                .AddColumn "所属模块",, 600
            Case ObjectHook
                .AddColumn "处理函数名称",, 250
                .AddColumn "地址",, 300
                .AddColumn "对象类型",, 250
                .AddColumn "所属模块",, 600
            Case EtwProvider
                'TreeList.AddColumn "序号", 100
                .AddColumn "GUID",, 600
                .AddColumn "名称",, 500
                'TreeList.AddColumn "类型", 150
                'TreeList.AddColumn "状态", 100
            Case WinsockSPI
                .AddColumn "序号",, 60
                .AddColumn "协议名称",, 180
                .AddColumn "提供者路径",, 320
                .AddColumn "文件厂商",, 120
                .AddColumn "CLSID",, 260
            Case TaskScheduler
                .AddColumn "序号",, 60
                .AddColumn "任务路径",, 320
                .AddColumn "启用状态",, 60
                .AddColumn "当前状态",, 80
                .AddColumn "上次运行",, 140
                .AddColumn "下次运行",, 140
                .AddColumn "运行结果",, 100
                .AddColumn "程序路径",, 260
                .AddColumn "任务描述",, 280
                .AddColumn "公司",, 120
                .AddColumn "版本",, 100
                .AddColumn "文件描述",, 180
        End Select
    End With
End Sub

Private Sub FreeLVRow(row As LVRow Ptr)
    If row = 0 Then Exit Sub

    Dim i As Long
    For i = 0 To row->ColCount - 1
        If row->Cols[i] <> 0 Then
            Deallocate row->Cols[i]
        End If
    Next

    Deallocate row->Cols
    Deallocate row
End Sub

Private Sub FreeListViewSnapshot(ByVal snap As ListViewSnapshot Ptr)
    If snap = 0 Then Exit Sub

    Dim i As Long
    If snap->RowCount > 0 Then
        For i = 0 To snap->RowCount - 1
            If snap->Rows(i) <> 0 Then FreeLVRow snap->Rows(i)
        Next
        Erase snap->Rows
    End If

    Deallocate snap
End Sub

Sub InitAllModuleCache()
    For i As Long = 0 To MaxInterfaceCount - 1
        g_ViewCache(i).IsCached = False
        g_ViewCache(i).SelectedIndex = -1
        g_ViewCache(i).TopVisibleIndex = 0
        g_ViewCache(i).ItemCount = 0
        If g_ViewCache(i).pSnap <> 0 Then
            FreeListViewSnapshot g_ViewCache(i).pSnap
            g_ViewCache(i).pSnap = 0
        End If
    Next
End Sub

Private Function DupWStr(ByVal src As WCHAR Ptr) As WCHAR Ptr
    If src = 0 Then Return 0

    Dim nLen As Long = lstrlenW(src)
    Dim dst As WCHAR Ptr = Callocate(nLen + 1, SizeOf(WCHAR))
    memcpy(dst, src, (nLen + 1) * SizeOf(WCHAR))
    Return dst
End Function

Private Function DupFromStringW(ByRef s As StringW) As WCHAR Ptr
    Dim src As WCHAR Ptr = StrPtrW(s)      ' StringW 内部 UTF-16
    If src = 0 Then Return 0

    Dim nLen As Long = lstrlenW(src)
    Dim buf As WCHAR Ptr = Callocate(nLen + 1, SizeOf(WCHAR))
    memcpy(buf, src, (nLen + 1) * SizeOf(WCHAR))
    Return buf
End Function

Private Function CloneListViewSnapshot(ByVal src As ListViewSnapshot Ptr) As ListViewSnapshot Ptr
    If src = 0 Then Return 0
    If src->RowCount <= 0 Then Return 0

    Dim dst As ListViewSnapshot Ptr = Callocate(1, SizeOf(ListViewSnapshot))
    dst->RowCount = src->RowCount
    ReDim dst->Rows(dst->RowCount - 1)

    Dim i As Long, c As Long
    For i = 0 To src->RowCount - 1
        Dim srow As LVRow Ptr = src->Rows(i)
        If srow = 0 Then Continue For

        Dim drow As LVRow Ptr = Callocate(1, SizeOf(LVRow))
        drow->ColCount = srow->ColCount
        drow->Fore = srow->Fore
        drow->Back = srow->Back

        If drow->ColCount > 0 Then
            drow->Cols = Callocate(drow->ColCount, SizeOf(WString Ptr))
            For c = 0 To drow->ColCount - 1
                If srow->Cols[c] <> 0 Then
                    drow->Cols[c] = DupWStr(srow->Cols[c])
                End If
            Next
        End If

        dst->Rows(i) = drow
    Next

    Return dst
End Function

Sub Cache_AddRow(ByVal moduleType As Interface, ByVal row As LVRow Ptr)
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache->pSnap = 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = cache->pSnap

    Dim idx As Long = snap->RowCount
    snap->RowCount += 1
    ReDim Preserve snap->Rows(idx)

    ' 深拷贝一行（避免指针共享）
    Dim newRow As LVRow Ptr = Callocate(1, SizeOf(LVRow))
    newRow->ColCount = row->ColCount
    newRow->Fore = row->Fore
    newRow->Back = row->Back
    newRow->Cols = Callocate(newRow->ColCount, SizeOf(WString Ptr))

    Dim i As Long
    For i = 0 To newRow->ColCount - 1
        If row->Cols[i] <> 0 Then
            newRow->Cols[i] = DupWStr(row->Cols[i])
        End If
    Next

    snap->Rows(idx) = newRow
End Sub

Sub Cache_DeleteRow(ByVal moduleType As Interface, ByVal index As Long)
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache->pSnap = 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = cache->pSnap
    If index < 0 Or index >= snap->RowCount Then Exit Sub

    ' 释放该行
    FreeLVRow snap->Rows(index)

    ' 前移数组
    Dim i As Long
    For i = index To snap->RowCount - 2
        snap->Rows(i) = snap->Rows(i + 1)
    Next

    snap->RowCount -= 1

    If snap->RowCount > 0 Then
        ReDim Preserve snap->Rows(snap->RowCount - 1)
    Else
        Erase snap->Rows
    End If
End Sub



' ==============================================
' 修正版：保存当前模块状态（深拷贝数据，无指针冲突）
' ==============================================
Sub SaveCurrentModuleState(ByVal lvw As Class_ListView, ByVal moduleType As Interface)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(lvw.hWnd, GWLP_USERDATA))
    If ctx = 0 Then Exit Sub
    If ctx->Snap = 0 Then Exit Sub

    ' 释放旧缓存
    If g_ViewCache(moduleType).pSnap <> 0 Then
        FreeListViewSnapshot g_ViewCache(moduleType).pSnap
        g_ViewCache(moduleType).pSnap = 0
    End If

    ' 深拷贝当前快照
    g_ViewCache(moduleType).pSnap = CloneListViewSnapshot(ctx->Snap)
    If g_ViewCache(moduleType).pSnap = 0 Then Exit Sub

    g_ViewCache(moduleType).IsCached = True
    g_ViewCache(moduleType).SelectedIndex = lvw.SelectedItem
    Print "moduleType:" & moduleType & " lvw.SelectedItem:" & lvw.SelectedItem
    g_ViewCache(moduleType).TopVisibleIndex = ListView_GetTopIndex(lvw.hWnd)
    g_ViewCache(moduleType).ItemCount = ctx->Snap->RowCount
End Sub

' ==============================================
' 修正版：从缓存恢复模块（重新填充数据，绝对不冲突）
' ==============================================
Sub RestoreModuleFromCache(ByVal lvw As Class_ListView, ByVal moduleType As Interface)
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache = 0 Or cache->IsCached = False Or cache->pSnap = 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = cache->pSnap
    If snap->RowCount <= 0 Then Exit Sub

    InitListViewData lvw

    Dim i As Long, c As Long
    For i = 0 To snap->RowCount - 1
        Dim row As LVRow Ptr = snap->Rows(i)
        If row = 0 Then Continue For
        If row->ColCount <= 0 Then Continue For

        Dim rowIdx As Long = AddItemEx(lvw, *row->Cols[0], row->Fore, row->Back)

        For c = 1 To row->ColCount - 1
            If row->Cols[c] <> 0 Then
                SetItemTextEx lvw, rowIdx, c, *row->Cols[c]
            End If
        Next
    Next
    CommitListViewView lvw
    
    'Print "moduleType:" & moduleType & " cache->SelectedIndex:" & cache->SelectedIndex & " lvw.ItemCount:" & lvw.ItemCount
    If cache->SelectedIndex >= 0 And cache->SelectedIndex < lvw.ItemCount Then
        lvw.SelectedItem = cache->SelectedIndex
        ListView_EnsureVisible(lvw.hWnd, cache->SelectedIndex + (lvw.CountPerPage - (cache->SelectedIndex - cache->TopVisibleIndex)), True)
        lvw.SetFocus
    End If
End Sub



Function TreeView_FindChild(tv As HWND, parent As HTREEITEM, ByRef text As WString) As HTREEITEM
    Dim hItem As HTREEITEM = TreeView_GetChild(tv, parent)

    Do While hItem <> 0
        Dim buf As WString * 260
        Dim tvi As TVITEM

        tvi.mask = TVIF_TEXT
        tvi.hItem = hItem
        tvi.pszText = @buf
        tvi.cchTextMax = 260

        TreeView_GetItem(tv, @tvi)

        If buf = text Then Return hItem

        hItem = TreeView_GetNextSibling(tv, hItem)
    Loop

    Return 0
End Function

Function InsertNodeByPath(tv As Class_TreeView, path As WString) As HTREEITEM

    Dim hwnd As HWND = tv.hWnd
    Dim parent As HTREEITEM = TVI_ROOT
    Dim current As HTREEITEM = 0

    Dim parts() As StringW
    Dim count As Long = 0

    ' =========================
    ' 1. 分割路径 A\B\C
    ' =========================
    Dim startPos As Long = 1
    Dim i As Long

    For i = 1 To Len(path)
        If Mid(path, i, 1) = "\" Then
            ReDim Preserve parts(count)
            parts(count) = Mid(path, startPos, i - startPos)
            count += 1
            startPos = i + 1
        End If
    Next

    ReDim Preserve parts(count)
    parts(count) = Mid(path, startPos)
    count += 1

    ' =========================
    ' 2. 逐级插入 / 查找
    ' =========================
    Dim level As Long
    For level = 0 To count - 1

        Dim strName As StringW = parts(level)

        ' 查找是否已存在
        current = TreeView_FindChild(hwnd, parent, strName)

        If current = 0 Then
            ' 不存在 -> 插入

            Dim tvis As TVINSERTSTRUCT
            Dim item As TVITEM

            item.mask = TVIF_TEXT
            item.pszText = strName.WstrPtr

            tvis.hParent = parent
            tvis.hInsertAfter = TVI_LAST
            tvis.item = item

            current = TreeView_InsertItem(hwnd, @tvis)
        End If

        parent = current
    Next

    Return current
End Function

Function GetIndexByListViewHwnd(ByVal hListView As HWND) As Integer
    For i As Integer = 0 To UBound(CurrentInformationArray)
        If CurrentInformationArray(i).hListView = hListView Then Return i
    Next
    Return -1
End Function

Private Function GetCtx(ListView As Class_ListView) As ListViewContext Ptr
    Return Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
End Function

' 排序：仅对VisibleIndex索引数组排序，不改动原始数据
' 自动识别：数字按数值排序，文本按字符串排序
' 【高性能自适应排序】缓存列类型，无重复校验，性能无损
Sub SortListViewVisibleIndex(ctx As ListViewContext Ptr)
    If ctx = NULL Or ctx->VisibleCount <= 1 Then Exit Sub
    If ctx->SortedColumn < 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    Dim sortCol As Long = ctx->SortedColumn
    Dim isAsc As Boolean = ctx->SortAscending
    Dim count As Long = ctx->VisibleCount

    ' 直接用缓存！0校验开销，和硬编码一样快
    If sortCol > UBound(ctx->ColumnIsNumber) Then Exit Sub
    Dim isNumber As Boolean = ctx->ColumnIsNumber(sortCol)

    ' 冒泡排序
    For i As Long = 0 To count - 2
        For j As Long = 0 To count - 2 - i
            Dim idx1 As Long = ctx->VisibleIndex(j)
            Dim idx2 As Long = ctx->VisibleIndex(j + 1)
            
            Dim row1 As LVRow Ptr = snap->Rows(idx1)
            Dim row2 As LVRow Ptr = snap->Rows(idx2)
            If row1 = NULL Or row2 = NULL Then Continue For
            If sortCol >= row1->ColCount Or sortCol >= row2->ColCount Then Continue For

            Dim cmp As Long = 0
            If isNumber Then
                ' 纯数字列：数值排序
                Dim val1 As Double = Val(*row1->Cols[sortCol])
                Dim val2 As Double = Val(*row2->Cols[sortCol])
                If val1 > val2 Then cmp = 1
                If val1 < val2 Then cmp = -1
            Else
                ' 文本列：字典序排序
                cmp = lstrcmpW(row1->Cols[sortCol], row2->Cols[sortCol])
            End If

            If Not isAsc Then cmp = -cmp
            If cmp > 0 Then Swap ctx->VisibleIndex(j), ctx->VisibleIndex(j + 1)
        Next j
    Next i
End Sub

Sub InitListViewData(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)
    If ctx <> NULL Then
        ' 已有上下文，清理旧数据而不是重新创建
        DeleteAllItemsEx(ListView)  ' ← 内部会检查ctx有效性
        Return  ' 不再重复初始化
    End If
    ctx = CAllocate(1, SizeOf(ListViewContext))
    
    ctx->hLV = ListView.hWnd

    ctx->Snap = CAllocate(1, SizeOf(ListViewSnapshot))
    ctx->Snap->RowCount = 0
    ReDim ctx->Snap->Rows(-1)

    ReDim ctx->VisibleIndex(-1)
    ctx->VisibleCount = 0
    ctx->Filter = 0
    ctx->SortedColumn = -1
    ctx->SortAscending = True

    SetWindowLongPtr(ListView.hWnd, GWLP_USERDATA, Cast(LONG_PTR, ctx))

    ' 必须
    ListView.DeleteAllItems
    ListView_SetUnicodeFormat(ListView.hWnd, True)
    ListView_SetExtendedListViewStyleEx(ListView.hWnd, LVS_OWNERDATA, LVS_OWNERDATA)
End Sub

Private Function CreateLVRow( _
    ColCount As Long, _
    Cols As StringW Ptr, _
    Fore As COLORREF, _
    Back As COLORREF _
) As LVRow Ptr

    Dim r As LVRow Ptr = New LVRow
    r->ColCount = ColCount
    r->Cols = CAllocate(ColCount, SizeOf(StringW))
    r->Fore = Fore
    r->Back = Back

    For i As Long = 0 To ColCount - 1
        r->Cols[i] = Cols[i]
    Next

    Return r
End Function

Function AddItemEx( _
    ListView As Class_ListView, _
    Text0 As StringW, _
    Fore As COLORREF = FB_Black, _
    Back As COLORREF = FB_White _
) As Long
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    ' 新增：ctx空指针校验
    If ctx = NULL Or ctx->Snap = NULL Then Return -1
    
    Dim row As LVRow Ptr = Callocate(1, SizeOf(LVRow))
    ' 新增：row分配失败校验
    If row = NULL Then Return -1
    
    row->ColCount = 1
    row->Fore = Fore
    row->Back = Back
    ' 核心修复：固定分配1个WCHAR Ptr（单列），与ColCount保持一致
    row->Cols = CAllocate(1, SizeOf(WCHAR Ptr))
    ' 新增：Cols分配失败校验
    If row->Cols = NULL Then
        Deallocate(row)
        Return -1
    End If
    
    row->Cols[0] = DupWStr(StrPtrW(Text0))
    ' 新增：DupWStr失败校验（避免写入NULL）
    If row->Cols[0] = NULL Then
        Deallocate(row->Cols)
        Deallocate(row)
        Return -1
    End If
    
    Dim idx As Long = ctx->Snap->RowCount
    ctx->Snap->RowCount += 1
    ReDim Preserve ctx->Snap->Rows(idx)
    ctx->Snap->Rows(idx) = row
    Return idx
End Function

Function AddItemColListEx CDecl ( _
    ListView As Class_ListView, _
    colCnt As Long, _
    Fore   As COLORREF = FB_Black, _
    Back   As COLORREF = FB_White, _
    ... _
) As Long

    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
        
    'If colCnt > ctx->Snap->RowCount Then Return -1
    Dim row As LVRow Ptr = Callocate(1, SizeOf(LVRow))
    row->ColCount = colCnt
    row->Fore = Fore
    row->Back = Back
    row->Cols = Callocate(colCnt, SizeOf(WCHAR Ptr))

    Dim ap As Any Ptr
    ap = va_first()

    For i As Long = 0 To colCnt - 1
        Dim src As WCHAR Ptr = va_arg(ap, WCHAR Ptr)
        ap = va_next(ap, WCHAR Ptr)
        row->Cols[i] = DupWStr(src)
    Next

    Dim idx As Long = ctx->Snap->RowCount
    ctx->Snap->RowCount += 1
    ReDim Preserve ctx->Snap->Rows(idx)
    ctx->Snap->Rows(idx) = row

    Return idx
End Function

Sub SetItemTextEx(ListView As Class_ListView, Row As Long, Col As Long, Text As StringW)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))

    If Row < 0 Or Row >= ctx->Snap->RowCount Then Exit Sub
    Dim r As LVRow Ptr = ctx->Snap->Rows(Row)
    If r = NULL Then Exit Sub

    ' 如果列索引超出当前列数，需要扩展
    If Col >= r->ColCount Then
        ' 分配新的列指针数组，并复制原内容
        Dim newColCount As Long = Col + 1
        Dim newCols As WCHAR Ptr Ptr = Callocate(newColCount, SizeOf(WCHAR Ptr))
        If newCols = NULL Then Exit Sub
        
        ' 复制原有列指针
        For i As Long = 0 To r->ColCount - 1
            newCols[i] = r->Cols[i]
        Next
        ' 新增列初始化为 NULL
        For i As Long = r->ColCount To newColCount - 1
            newCols[i] = NULL
        Next
        
        ' 释放原列数组指针（注意不要释放列内容，已复制）
        Deallocate(r->Cols)
        r->Cols = newCols
        r->ColCount = newColCount
    End If

    ' 释放该列原有文本（防止内存泄漏）
    If r->Cols[Col] <> NULL Then
        Deallocate(r->Cols[Col])
    End If
    
    ' 设置新文本
    r->Cols[Col] = DupWStr(StrPtrW(Text))
End Sub

Sub SetListViewFilter(ListView As Class_ListView, s As StringW)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))

    If ctx->Filter <> 0 Then
        Deallocate(ctx->Filter)
        ctx->Filter = 0
    End If

    If Len(s) > 0 Then
        ctx->Filter = DupWStr(StrPtrW(s))
    End If
End Sub

Sub SetItemColor(ListView As Class_ListView, Row As Long, Fore As COLORREF, Back As COLORREF)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    ctx->Snap->Rows(Row)->Fore = Fore
    ctx->Snap->Rows(Row)->Back = Back
End Sub

Sub SetItemColorByHwnd(hListView As hWnd, Row As Long, Fore As COLORREF, Back As COLORREF)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(hListView, GWLP_USERDATA))
    ctx->Snap->Rows(Row)->Fore = Fore
    ctx->Snap->Rows(Row)->Back = Back
End Sub

Sub GetItemColor(ListView As Class_ListView, Row As Long, ByRef Fore As COLORREF, ByRef Back As COLORREF)
    Dim r As LVRow Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))->Snap->Rows(Row)
    Fore = r->Fore
    Back = r->Back
End Sub

Sub GetItemColorByHwnd(hListView As hWnd, Row As Long, ByRef Fore As COLORREF, ByRef Back As COLORREF)
    Dim r As LVRow Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(hListView, GWLP_USERDATA))->Snap->Rows(Row)
    Fore = r->Fore
    Back = r->Back
End Sub

' 【极简高性能版】基于ValW的纯数字判断（完美解决进程名误判）
Private Function IsValidNumberW(pwStr As WString Ptr) As Boolean
    ' 空指针/空字符串 → 不是数字
    If pwStr = NULL Or *pwStr = WStr("") Then Return False
    
    ' 1. 转换为数值
    Dim num As Double = Val(*pwStr)
    
    ' 2. 核心逻辑：
    '    - 数字≠0 → 必须 原字符串 = 数值的字符串（杜绝360xxx）
    '    - 数字=0 → 必须 原字符串严格等于 "0"
    Function = (num <> 0 AndAlso WStr(num) = *pwStr) OrElse (*pwStr = WStr("0"))
End Function

' 一次性缓存所有列的类型（纯数字/文本），后续直接用，性能拉满
Private Sub CacheColumnTypes(ctx As ListViewContext Ptr)
    If ctx = NULL Or ctx->Snap = NULL Or ctx->Snap->RowCount = 0 Then Exit Sub
    
    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    If snap->RowCount = 0 Then Exit Sub
    
    ' 获取第一行数据（所有列类型固定）
    Dim firstRow As LVRow Ptr = snap->Rows(0)
    If firstRow = NULL Then Exit Sub
    
    ' 重置缓存数组
    Redim ctx->ColumnIsNumber(0 To firstRow->ColCount - 1)
    
    ' 遍历所有列，一次性判断并缓存
    For col As Long = 0 To firstRow->ColCount - 1
        ctx->ColumnIsNumber(col) = IsValidNumberW(firstRow->Cols[col])
    Next col
End Sub

Sub CommitListViewView(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    
    ' 安全校验：上下文/快照为空直接退出
    If ctx = NULL Then Exit Sub
    If ctx->Snap = NULL Then Exit Sub
    If ctx->Snap->RowCount = 0 Then
        ReDim ctx->VisibleIndex(-1)
        ctx->VisibleCount = 0
        SendMessage(ListView.hWnd, LVM_SETITEMCOUNT, 0, 0)
        InvalidateRect(ListView.hWnd, NULL, True)
        Exit Sub
    End If

    ' ===================== 【最佳位置】自动缓存列类型 =====================
    ' 每次刷新数据时，仅执行1次列类型校验（性能无损耗）
    CacheColumnTypes(ctx)

    ReDim ctx->VisibleIndex(-1)
    ctx->VisibleCount = 0

    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim r As LVRow Ptr = ctx->Snap->Rows(i)
        Dim hit As Boolean = (ctx->Filter = 0)

        If Not hit Then
            For c As Long = 0 To r->ColCount - 1
                If r->Cols[c] <> 0 Then
                    If wcsstr(r->Cols[c], ctx->Filter) <> 0 Then
                        hit = TRUE
                        Exit For
                    End If
                End If
            Next
        End If

        If hit Then
            ctx->VisibleCount += 1
            ReDim Preserve ctx->VisibleIndex(ctx->VisibleCount - 1)
            ctx->VisibleIndex(ctx->VisibleCount - 1) = i
        End If
    Next

    SendMessage(ListView.hWnd, LVM_SETITEMCOUNT, ctx->VisibleCount, 0)
    InvalidateRect(ListView.hWnd, NULL, True)
End Sub

Sub DeleteItemEx(ListView As Class_ListView, VisibleRow As Long, ByVal bCommit As Boolean = True)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    
    If ctx = NULL Or ctx->Snap = NULL Then Exit Sub
    If VisibleRow < 0 Or VisibleRow >= ctx->VisibleCount Then Exit Sub

    Dim real As Long = ctx->VisibleIndex(VisibleRow)
    If real < 0 Or real >= ctx->Snap->RowCount Then Exit Sub
    
    Dim r As LVRow Ptr = ctx->Snap->Rows(real)
    If r = NULL Then Exit Sub

    ' 正确释放：先释放每列的字符串，再释放列数组
    For i As Long = 0 To r->ColCount - 1
        If r->Cols[i] <> NULL Then
            Deallocate(r->Cols[i])
        End If
    Next
    Deallocate r->Cols
    Delete r

    ' 移动后续行
    For i As Long = real To ctx->Snap->RowCount - 2
        ctx->Snap->Rows(i) = ctx->Snap->Rows(i + 1)
    Next

    ctx->Snap->RowCount -= 1
    ReDim Preserve ctx->Snap->Rows(ctx->Snap->RowCount - 1)
    
    Cache_DeleteRow(CurrentInformation.intType, VisibleRow) ' 刷新索引
    
    ' 需要重新计算可见索引
    If bCommit Then CommitListViewView(ListView)
End Sub

Sub DeleteAllItemsEx(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    
    If ctx = NULL Or ctx->Snap = NULL Then Exit Sub
    
    ' 释放所有行数据
    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim r As LVRow Ptr = ctx->Snap->Rows(i)
        If r <> NULL Then
            ' 释放每列的字符串内存
            For c As Long = 0 To r->ColCount - 1
                If r->Cols[c] <> NULL Then
                    Deallocate(r->Cols[c])
                End If
            Next
            ' 释放列指针数组
            Deallocate(r->Cols)
            ' 释放行结构
            Delete r ' 是否应改为Deallocate?
        End If
    Next
    
    ' 清空行指针数组
    ReDim ctx->Snap->Rows(-1)
    ctx->Snap->RowCount = 0
    
    ' 清空可见行索引
    ReDim ctx->VisibleIndex(-1)
    ctx->VisibleCount = 0
    
    ' 重置过滤器
    /'If ctx->Filter <> NULL Then
        Deallocate(ctx->Filter)
        ctx->Filter = NULL
    End If'/
End Sub

Sub UninitListViewData(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))

    If ctx = 0 Then Exit Sub

    If ctx->Snap <> 0 Then
        For i As Long = 0 To ctx->Snap->RowCount - 1
            FreeLVRow(ctx->Snap->Rows(i))
        Next
        Deallocate(ctx->Snap)
    End If

    If ctx->Filter <> 0 Then Deallocate(ctx->Filter)

    SetWindowLongPtr(ListView.hWnd, GWLP_USERDATA, 0)
    Deallocate(ctx)
End Sub

Sub SetListViewItemBackColor(ListView As Class_ListView, ByVal ItemIndex As Integer, ByVal ItemColor As COLORREF)
    '设置背景色
    SendMessage ListView.hWnd, LVM_SETTEXTBKCOLOR, ItemIndex, ItemColor
End Sub

Function IsListViewItemSelected(ListView As Class_ListView, ByVal ItemIndex As Integer) As BOOLEAN
    Return SendMessage(ListView.hWnd, LVM_GETITEMSTATE, Cast(wParam, ItemIndex), Cast(lParam, LVIS_SELECTED)) And LVIS_SELECTED
End Function

Function GetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long) As BOOLEAN
    Dim state As UINT = GetMenuState(Menu.HMENU, iditem, MF_BYCOMMAND)
    Return (state And MF_CHECKED)
End Function

Sub SetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long, ByVal state As Boolean)
    CheckMenuItem Menu.HMENU, iditem, MF_BYCOMMAND Or IIf(state, MF_CHECKED, MF_UNCHECKED)
End Sub

Function GetMenuText(Menu As Class_Menu, ByVal iditem As Long) As String
    Dim pBuffer As LPSTR = Allocate(MAX_PATH)
    GetMenuStringA Menu.HMENU, iditem, pBuffer, MAX_PATH, MF_BYCOMMAND
    Print WinErrorMsg(GetLastError) ' 这里GetLastError会是"句柄无效",但我们不用管它,反正已经拿到了pBuffer
    Return *pBuffer
End Function

' 修改菜单项文本的函数
Sub SetMenuText(ByVal hWndForm As hWnd, Menu As Class_Menu, ByVal iditem As Long, ByVal newText As StringW)

    Dim mii As MENUITEMINFO

    memset(@mii, SizeOf(MENUITEMINFO), 0)

    With mii
        .cbSize = SizeOf(MENUITEMINFO)

        ' ✔ 关键修复：只保留 STRING
        .fMask = MIIM_STRING

        ' ✔ Unicode 安全指针
        .dwTypeData = newText.WstrPtr

        ' ✔ 推荐：交给系统推断
        .cch = 0
    End With

    If SetMenuItemInfo(Menu.HMENU, iditem, FALSE, @mii) = 0 Then
        Print "SetMenuItemInfo失败: "; GetLastError
        Print WinErrorMsg(GetLastError)
    End If

    ' ✔ 正确：刷新窗口菜单
    DrawMenuBar hWndForm
End Sub

Sub SetMenuStatus(Menu As Class_Menu, ByVal iditem As Long, ByVal bStatus As Boolean)
    EnableMenuItem Menu.HMENU, iditem, IIf(bStatus, MF_ENABLED, MF_GRAYED Or MF_DISABLED)
End Sub

' 启用/禁用OwnerDraw的优化设置
Sub OptimizeListViewForOwnerDraw(hWndControl As HWND)
    ' 设置双缓冲
    SendMessage(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
    
    ' 禁用动画效果
    SendMessage(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_LABELTIP, LVS_EX_LABELTIP)
    
    ' 设置项目高度（可选）
    'SendMessage(hWndControl, LVM_SETITEMHEIGHT, 0, 20)
End Sub

Sub TreeList_DeleteAllChildItem(TreeList As Class_mCtrlTreeList, Item As MC_HTREELISTITEM)
    Do While TreeList.DeleteItem(TreeList.GetNextItem(Item, MC_TLGN_CHILD)) = True
    Loop
End Sub

Sub TreeList_SubItemHitTest(TreeList As Class_mCtrlTreeList, ByVal xPos As Long, ByVal yPos As Long, ByRef SelectItem As MC_HTREELISTITEM, ByRef iSubItem As Long, ByVal bIsScreenCoord As Boolean)
    Dim pp As MC_TLHITTESTINFO
    Dim pt As Point
    Dim hWndControl As hWnd = TreeList.hWnd
    
    ' 1. 屏幕坐标 -> 客户区坐标
    pt.x = xPos
    pt.y = yPos
    If bIsScreenCoord = True Then
        ScreenToClient hWndControl, @pt ' 只有屏幕坐标才转
    End If
    
    ' 2. 获取节点（保持不变）
    pp.pt.x = pt.x
    pp.pt.y = pt.y
    pp.flags = MC_TLHT_ONITEM
    SendMessage hWndControl, MC_TLM_HITTEST, 0, Cast(LPARAM, @pp)
    SelectItem = Cast(MC_HTREELISTITEM, pp.hItem)
    If SelectItem = 0 Then Exit Sub

    ' ============== 【核心修复】获取水平滚动偏移量 ==============
    Dim si As SCROLLINFO
    si.cbSize = SizeOf(SCROLLINFO)
    si.fMask = SIF_POS
    GetScrollInfo hWndControl, SB_HORZ, @si
    Dim lScrollX As Long = si.nPos ' 水平滚动了多少像素
    
    ' 3. 【关键】用 客户区X + 滚动偏移 得到“逻辑坐标”来算列
    Dim colWidth As Long, sumWidth As Long, iColumnIndex As Long
    Dim logicX As Long = pt.x + lScrollX ' 修正后的逻辑X坐标
    
    For iColumnIndex = 0 To TreeList.GetColumnCount - 1
        colWidth = TreeList.GetColumnWidth(iColumnIndex)
        ' 用修正后的 logicX 判断
        If logicX >= sumWidth And logicX < sumWidth + colWidth Then Exit For
        sumWidth += colWidth
    Next
    Print "[TreeList_SubItemHitTest]iColumnIndex:" & iColumnIndex
    iSubItem = iColumnIndex
End Sub