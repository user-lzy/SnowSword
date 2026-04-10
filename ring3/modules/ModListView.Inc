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
    WindowTimer
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

Type TempTextNode
    Buf As WCHAR Ptr
End Type

Type ListViewContext
    hLV As HWND

    Snap As ListViewSnapshot Ptr

    VisibleIndex(Any) As Long
    VisibleCount As Long

    Filter As Wstring Ptr
End Type

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
    For i As Integer = TreeList.GetColumnCount - 1 To 1 Step -1
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
                .AddColumn "命令行参数",, 500
                .AddColumn "公司名",, 200
            Case Module
                .AddColumn "序号",, 50
                .AddColumn "名称",, 200
                .AddColumn "基址",, 230
                .AddColumn "大小",, 150
                .AddColumn "路径",, 500
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
            Case Privilege
                .AddColumn "LUID",, 100
                .AddColumn "名称",, 400
                .AddColumn "状态",, 200
                .AddColumn "描述",, 400
                .AddColumn "属性",, 100
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
                If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "CPU", 100
                Else
                    TreeList.SetColumn 1, "CPU", 100
                End If
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
                If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "CPU", 100
                Else
                    TreeList.SetColumn 1, "CPU", 100
                End If
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
                If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "名称", 250
                Else
                    TreeList.SetColumn 1, "名称", 250
                End If
                TreeList.AddColumn "回调函数地址", 280
                TreeList.AddColumn "所属模块", 500
                TreeList.AddColumn "PFLT_FILTER", 280
                TreeList.AddColumn "Altitude", 150
            Case FilterDriver
                If TreeList.GetColumnCount = 0 Then
                    TreeList.AddColumn "驱动名称", 250
                Else
                    TreeList.SetColumn 1, "驱动名称", 250
                End If
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
            Case File
                .AddColumn "名称",, 250
                .ADdColumn "类型",, 200
                .AddColumn "大小",, 100
            Case UnlockTheFile
                .AddColumn "进程名",, 200
                .AddColumn "PID",, 100
                .AddColumn "文件路径",, 300
                .AddColumn "进程路径",, 300
                .AddColumn "文件句柄",, 150
            Case Memory
                .AddColumn "地址",, 250
                .AddColumn "大小",, 100
                .AddColumn "属性",, 150
                .AddColumn "状态",, 150
                .AddColumn "类型",, 150
                .AddColumn "基址",, 250
                .AddColumn "对应模块",, 400
            Case Windows
                .AddColumn "句柄",, 150
                .AddColumn "名称",, 300
                .AddColumn "类名",, 300
                .AddColumn "TID",, 100
                .AddColumn "所属模块",, 400
            Case WindowTimer
                .AddColumn "定时器ID",, 150
                .AddColumn "所属TID",, 100
                .AddColumn "超时时间",, 150
                .AddColumn "回调地址",, 250
            Case Handles
                .AddColumn "类型",, 200
                .AddColumn "名称",, 300
                .AddColumn "值",, 100
                .AddColumn "访问权限",, 150
                .AddColumn "对象地址",, 200
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
                .AddColumn "地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "AddressReference",, 300
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
        End Select
    End With
End Sub

Function GetIndexByListViewHwnd(ByVal hListView As HWND) As Integer
    For i As Integer = 0 To UBound(CurrentInformationArray)
        If CurrentInformationArray(i).hListView = hListView Then Return i
    Next
    Return -1
End Function

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

Private Function GetCtx(ListView As Class_ListView) As ListViewContext Ptr
    Return Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
End Function

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

    SetWindowLongPtr(ListView.hWnd, GWLP_USERDATA, Cast(LONG_PTR, ctx))

    ' 必须
    ListView.DeleteAllItems
    ListView_SetUnicodeFormat(ListView.hWnd, True)
    ListView_SetExtendedListViewStyleEx(ListView.hWnd, LVS_OWNERDATA, LVS_OWNERDATA)
End Sub

Private Sub FreeLVRow(row As LVRow Ptr)
    If row = 0 Then Exit Sub

    For i As Long = 0 To row->ColCount - 1
        If row->Cols[i] <> 0 Then
            Deallocate(row->Cols[i])
        End If
    Next

    Deallocate(row->Cols)
    Deallocate(row)
End Sub

Private Function CreateLVRow( _
    ColCount As Long, _
    Cols As StringW Ptr, _
    Fore As COLORREF, _
    Back As COLORREF _
) As LVRow Ptr

    Dim r As LVRow Ptr = New LVRow
    r->ColCount = ColCount
    r->Cols = Callocate(ColCount, SizeOf(StringW))
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

Sub GetItemColor(ListView As Class_ListView, Row As Long, ByRef Fore As COLORREF, ByRef Back As COLORREF)
    Dim r As LVRow Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))->Snap->Rows(Row)
    Fore = r->Fore
    Back = r->Back
End Sub

Sub CommitListViewView(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))

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
            Delete r
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
Sub SetMenuText(Menu As Class_Menu, ByVal iditem As Long, ByVal newText As StringW)
    Dim mii As MENUITEMINFO
    Dim textBuffer(255) As Byte ' 字符串缓冲区

    ' 设置 MENUITEMINFO 结构体
    With mii
        .cbSize = SizeOf(MENUITEMINFO)
        .fMask = MIIM_TYPE
        .dwTypeData = newText.WstrPtr
        .cch = newText.WstrLen * 2
    End With

    ' 修改菜单项信息
    SetMenuItemInfo Menu.HMENU, iditem, True, @mii
    Print WinErrorMsg(GetLastError)

    ' 刷新菜单显示
    DrawMenuBar Cast(hWnd, Menu.HMENU)
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