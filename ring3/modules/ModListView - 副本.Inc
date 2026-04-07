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
    KernelModule
    KernelThread
    File
    Callbacks
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

Type ListViewItemInfo
    ForeColor As COLORREF
    BackColor As COLORREF
End Type

' 新增：数据快照类型
Type ListViewDataSnapshot
    DataArray(Any, Any) As StringW
    ItemColor(Any) As ListViewItemInfo
    Filter_ItemColor(Any) As ListViewItemInfo
    RowCount As Integer      ' ← 显式跟踪行数
    ColCount As Integer      ' ← 显式跟踪列数
End Type

Type ListViewDataNode
    hListView As hWnd 'ListView句柄,用于区分不同ListView
    strFilter As StringW '筛选字符串
    Snapshot As ListViewDataSnapshot Ptr  ' ← 只有这里是指针
    NextNode As ListViewDataNode Ptr ' 指向下一个节点的指针
End Type

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

Const FB_Red As COLORREF = BGR(255, 0, 0)
Const FB_LightPinkRed As COLORREF = BGR(240, 210, 210)
Const FB_LightRed As COLORREF = BGR(250, 225, 225)
Const FB_GoldenYellow As COLORREF = BGR(255, 240, 128)
Const FB_White As COLORREF = BGR(255, 255, 255)
Const FB_Black As COLORREF = BGR(0, 0, 0)
Const FB_LightGreen As COLORREF = BGR(50, 200, 50)

Dim Shared ListViewDataList As ListViewDataNode Ptr ' 链表头指针
Dim Shared ListViewDataListEnd As ListViewDataNode Ptr ' 链表尾指针
Dim Shared CurrentInformationArray() As CURRENT_INFORMATION
Dim Shared pOldWindowProc As PVOID

'#define DEBUG_LV 1

#ifdef DEBUG_LV
    Dim Shared g_lvDebugCount As Integer = 0
    Sub LV_Debug(msg As String)
        g_lvDebugCount += 1
        Print "[" & g_lvDebugCount & "] " & msg
    End Sub
#Else
    Sub LV_Debug(msg As String)  ' 空函数，调用无开销
    End Sub
#EndIf

/'Sub DeleteItemExWithColor(ListView As Class_ListView, ByVal ItemIndex As Long)
    DeleteItemEx ListView, ItemIndex
    If ItemIndex < ListView.ItemCount - 1 Then
        For i As Long = ItemIndex To ListView.ItemCount - 1
            memcpy @CurrentListViewItem(i), @CurrentListViewItem(i + 1), SizeOf(ListViewItemInfo)
        Next
    End If
    If ItemIndex <= ListView.ItemCount - 1 Then ReDim Preserve CurrentListViewItem(ListView.ItemCount - 2) As ListViewItemInfo
End Sub '/

Public Sub InitializeListView(ByVal ListView As Class_ListView, ByVal TreeList As Class_mCtrlTreeList, ByVal TreeView As Class_TreeView)
    ListView.DeleteAllColumn
    ListView.DeleteAllItems
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
                TreeList.AddColumn "驱动对象", 250
                TreeList.AddColumn "驱动路径", 500
                TreeList.AddColumn "设备名", 500
                TreeList.AddColumn "设备对象", 250
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
                .AddColumn "TID",, 100
                .AddColumn "周期",, 150
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

Function EnsureSnapshot(Node As ListViewDataNode Ptr) As ListViewDataSnapshot Ptr
    #ifdef DEBUG_LV
        LV_Debug("[EnsureSnapshot] Node @" & Hex(Node) & ", Snapshot=" & Hex(Node->Snapshot))
    #endif
    
    If Node = NULL Then 
        LV_Debug("[EnsureSnapshot] Node 为 NULL")
        Return NULL
    End If
    
    If Node->Snapshot = NULL Then
        LV_Debug("[EnsureSnapshot] 懒创建新快照")
        Node->Snapshot = New ListViewDataSnapshot
        
        ' 初始化为空数组（UBound 会返回 -1）
        ReDim Node->Snapshot->DataArray(-1 To -1, -1 To -1) As StringW
        ReDim Node->Snapshot->ItemColor(0) As ListViewItemInfo
        ReDim Node->Snapshot->Filter_ItemColor(0) As ListViewItemInfo
        
        LV_Debug("[EnsureSnapshot] 创建完成 @" & Hex(Node->Snapshot))
    End If
    
    Return Node->Snapshot
End Function

Function GetIndexByListViewHwnd(ByVal hListView As HWND) As Integer
    For i As Integer = 0 To UBound(CurrentInformationArray)
        If CurrentInformationArray(i).hListView = hListView Then Return i
    Next
    Return -1
End Function

Function FindListViewHwnd(ByVal ListView As Class_ListView) As ListViewDataNode Ptr
    LV_Debug("[FindListViewHwnd] Start hListView=" & Hex(ListView.hWnd))
    
    Dim CurrentNode As ListViewDataNode Ptr = ListViewDataList
    Dim Step1 As Integer = 0
    
    Do While CurrentNode <> NULL
        LV_Debug("[FindListViewHwnd] 步骤 " & Step1 & " Node @" & Hex(CurrentNode) & " hListView=" & Hex(CurrentNode->hListView))
        
        ' 跳过已删除的节点（hListView 为 NULL）
        If CurrentNode->hListView = ListView.hWnd And CurrentNode->hListView <> NULL Then
            LV_Debug("[FindListViewHwnd] 找到 @" & Hex(CurrentNode))
            Return CurrentNode
        End If
        
        CurrentNode = CurrentNode->NextNode
        Step1 += 1
    Loop
    
    LV_Debug("[FindListViewHwnd] 未找到")
    Return NULL
End Function

Function FindTrueRow(ListView As Class_ListView, RowIndex As Integer) As Integer
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Or Node->Snapshot = NULL Then Return -1
    
    Dim snapshot As ListViewDataSnapshot Ptr = Node->Snapshot
    
    ' 未筛选：直接返回
    If Node->strFilter = "" Then
        If RowIndex >= 0 And RowIndex < snapshot->RowCount Then
            Return RowIndex
        Else
            Return -1
        End If
    End If
    
    ' 筛选模式：遍历原始数据
    Dim filteredIndex As Integer = 0
    For i As Integer = 0 To snapshot->RowCount - 1
        For j As Integer = 0 To snapshot->ColCount - 1
            If InStrW(LCaseW(snapshot->DataArray(i, j)), LCaseW(Node->strFilter)) > 0 Then
                If filteredIndex = RowIndex Then Return i
                filteredIndex += 1
                Exit For  ' 已找到匹配，检查下一行
            End If
        Next
    Next
    
    Return -1
End Function

Function InitDataNode(ListView As Class_ListView) As ListViewDataNode Ptr
    LV_Debug("[InitDataNode] Start hListView=" & Hex(ListView.hWnd))
    
    ' 幂等删除：删除任何残留的旧节点
    DeleteListViewData(ListView)
    
    ' 创建新节点
    Dim Node As ListViewDataNode Ptr = New ListViewDataNode
    Node->hListView = ListView.hWnd
    Node->strFilter = ""
    Node->Snapshot = NULL  ' 初始为空，等待首次访问时懒创建
    Node->NextNode = NULL
    
    ' 添加到链表
    If ListViewDataList = NULL Then
        ListViewDataList = Node
        ListViewDataListEnd = Node
    Else
        ListViewDataListEnd->NextNode = Node
        ListViewDataListEnd = Node
    End If
    
    LV_Debug("[InitDataNode] Done Node @" & Hex(Node))
    Return Node
End Function

'Sub SetListViewData(ListView As Class_ListView)
'    LV_Debug("[SetListViewData] Start")
'    
'    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
'    If Node = NULL Then
'        LV_Debug("[SetListViewData] 节点为空，退出")
'        Exit Sub
'    End If
'    
'    ' 创建新快照（原子替换）
'    Dim NewSnapshot As ListViewDataSnapshot Ptr = New ListViewDataSnapshot
'    
'    ' 填充数据（在快照内部使用 ReDim Preserve 是安全的）
'    ReDim NewSnapshot->DataArray(ListView.ItemCount - 1, ListView.ColumnCount - 1) As StringW
'    ReDim NewSnapshot->ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
'    ReDim NewSnapshot->Filter_ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
'    
'    LV_Debug("[SetListViewData] 创建新快照 @" & Hex(NewSnapshot) & "，数组大小 " & ListView.ItemCount & "x" & ListView.ColumnCount)
'    
'    ' **正确初始化颜色（关键！）**
'    For i As Integer = 0 To ListView.ItemCount - 1
'        NewSnapshot->ItemColor(i).BackColor = FB_White
'        NewSnapshot->ItemColor(i).ForeColor = FB_Black
'        NewSnapshot->Filter_ItemColor(i).BackColor = FB_White
'        NewSnapshot->Filter_ItemColor(i).ForeColor = FB_Black
'    Next
'    
'    For i As Integer = 0 To ListView.ItemCount - 1
'        For j As Integer = 0 To ListView.ColumnCount - 1
'            NewSnapshot->DataArray(i, j) = ListView.GetItemText(i, j)
'        Next
'    Next
'    
'    ' 释放旧快照
'    If Node->Snapshot <> NULL Then
'        LV_Debug("[SetListViewData] 删除旧快照 @" & Hex(Node->Snapshot))
'        Delete Node->Snapshot
'    End If
'    
'    ' 原子替换（无中间状态）
'    Node->Snapshot = NewSnapshot
'    LV_Debug("[SetListViewData] Done")
'End Sub

Sub SetListViewData(ListView As Class_ListView)
    LV_Debug("[SetListViewData] Start")
    
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        LV_Debug("[SetListViewData] 节点为空，退出")
        Exit Sub
    End If
    
    ' 创建新快照（原子替换）
    Dim NewSnapshot As ListViewDataSnapshot Ptr = New ListViewDataSnapshot
    
    ' 填充数据
    ReDim NewSnapshot->DataArray(ListView.ItemCount - 1, ListView.ColumnCount - 1) As StringW
    ReDim NewSnapshot->ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
    ReDim NewSnapshot->Filter_ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
    
    LV_Debug("[SetListViewData] 创建新快照 @" & Hex(NewSnapshot) & "，数组大小 " & ListView.ItemCount & "x" & ListView.ColumnCount)
    
    ' 从现有快照复制颜色数据（如果存在）
    If Node->Snapshot <> NULL Then
        Dim OldSnapshot As ListViewDataSnapshot Ptr = Node->Snapshot
        
        ' 复制ItemColor
        For i As Integer = 0 To Min(ListView.ItemCount - 1, UBound(OldSnapshot->ItemColor))
            NewSnapshot->ItemColor(i) = OldSnapshot->ItemColor(i)
            NewSnapshot->Filter_ItemColor(i) = OldSnapshot->Filter_ItemColor(i)
        Next
        
        ' 对于超出旧数组范围的新项目，使用默认颜色
        For i As Integer = UBound(OldSnapshot->ItemColor) + 1 To ListView.ItemCount - 1
            NewSnapshot->ItemColor(i).BackColor = FB_White
            NewSnapshot->ItemColor(i).ForeColor = FB_Black
            NewSnapshot->Filter_ItemColor(i).BackColor = FB_White
            NewSnapshot->Filter_ItemColor(i).ForeColor = FB_Black
        Next
    Else
        ' 如果没有旧快照，全部使用默认颜色
        For i As Integer = 0 To ListView.ItemCount - 1
            NewSnapshot->ItemColor(i).BackColor = FB_White
            NewSnapshot->ItemColor(i).ForeColor = FB_Black
            NewSnapshot->Filter_ItemColor(i).BackColor = FB_White
            NewSnapshot->Filter_ItemColor(i).ForeColor = FB_Black
        Next
    End If
    
    ' 复制文本数据
    For i As Integer = 0 To ListView.ItemCount - 1
        For j As Integer = 0 To ListView.ColumnCount - 1
            NewSnapshot->DataArray(i, j) = ListView.GetItemText(i, j)
        Next
    Next
    
    ' 释放旧快照
    If Node->Snapshot <> NULL Then
        LV_Debug("[SetListViewData] 删除旧快照 @" & Hex(Node->Snapshot))
        Delete Node->Snapshot
    End If
    
    ' 原子替换
    Node->Snapshot = NewSnapshot
    LV_Debug("[SetListViewData] Done")
End Sub

/'Sub ClearListViewData(ListView As Class_ListView) '将ListView原始数据存储至ListViewDataTable中
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView) '寻找对应节点指针
    If Node = NULL Then Node = InitDataNode(ListView) '如果未找到对应的节点，添加一个新的节点并初始化数据
    
    '重新调整DataArray的边界使其适应ListView的行与列
    ReDim Node->DataArray(ListView.ItemCount - 1, ListView.ColumnCount - 1) As String
    For i As Integer = 0 To ListView.ItemCount - 1
        For j As Integer = 0 To ListView.ColumnCount - 1
            Node->DataArray(i, j) = ListView.GetItemText(i, j) '遍历二维数组每个元素并填充数据
        Next
    Next
End Sub'/

Function SetFilter(ListView As Class_ListView, ByVal SubStr As String) As Boolean '设置ListView的筛选字符串
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView) '寻找对应节点指针
    If Node = NULL Then Return FALSE
    Node->strFilter = SubStr '设置筛选字符串
    Return TRUE
End Function

Function FilterAllItem(ListView As Class_ListView) As Integer
    LV_Debug("[FilterAllItem] Start")
    
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Or Node->Snapshot = NULL Then
        LV_Debug("[FilterAllItem] 节点或快照为空")
        Return 0
    End If
    
    Dim snapshot As ListViewDataSnapshot Ptr = Node->Snapshot
    
    ' 检查数组是否已分配（UBound 返回 -1 表示未分配）
    If UBound(snapshot->DataArray, 1) = -1 Then
        LV_Debug("[FilterAllItem] DataArray 未分配")
        Return 0
    End If
    
    Dim SubStr As StringW = Node->strFilter
    Dim ItemCount As Integer = 0
    
    ListView.ReDraw = FALSE
    ListView.DeleteAllItems
    
    If SubStr = "" Then
        ' 无筛选，显示全部
        For i As Integer = 0 To UBound(snapshot->DataArray, 1)
            ' 直接访问成员（不使用 With）
            snapshot->Filter_ItemColor(i).BackColor = snapshot->ItemColor(i).BackColor
            snapshot->Filter_ItemColor(i).ForeColor = snapshot->ItemColor(i).ForeColor
            
            ListView.AddItem snapshot->DataArray(i, 0)
            For j As Integer = 1 To UBound(snapshot->DataArray, 2)
                ListView.SetItemText i, j, snapshot->DataArray(i, j)
            Next
            ItemCount += 1
        Next
    Else
        ' 有筛选
        For i As Integer = 0 To UBound(snapshot->DataArray, 1)
            For j As Integer = 0 To UBound(snapshot->DataArray, 2)
                If InStrW(LCaseW(snapshot->DataArray(i, j)), LCaseW(SubStr)) > 0 Then
                    snapshot->Filter_ItemColor(ItemCount).BackColor = snapshot->ItemColor(i).BackColor
                    snapshot->Filter_ItemColor(ItemCount).ForeColor = snapshot->ItemColor(i).ForeColor
                    
                    Dim newIndex As Integer = ListView.AddItem(snapshot->DataArray(i, 0))
                    For k As Integer = 1 To UBound(snapshot->DataArray, 2)
                        ListView.SetItemText newIndex, k, snapshot->DataArray(i, k)
                    Next
                    ItemCount += 1
                    Exit For
                End If
            Next
        Next
    End If
    
    ListView.ReDraw = TRUE
    LV_Debug("[FilterAllItem] Done，显示 " & ItemCount & " 项")
    Return ItemCount
End Function

Sub DeleteListViewData(ListView As Class_ListView)
    LV_Debug("[DeleteListViewData] Start hListView=" & Hex(ListView.hWnd))
    
    If ListViewDataList = NULL Then
        LV_Debug("[DeleteListViewData] 链表为空")
        Exit Sub
    End If
    
    Dim CurrentNode As ListViewDataNode Ptr = ListViewDataList
    Dim PreviousNode As ListViewDataNode Ptr = NULL
    Dim Step1 As Integer = 0
    
    Do While CurrentNode <> NULL
        LV_Debug("[DeleteListViewData] 步骤 " & Step1 & " Node @" & Hex(CurrentNode))
        
        ' 移除 CUlong，直接比较
        If CurrentNode->hListView = ListView.hWnd Then
            LV_Debug("[DeleteListViewData] 找到目标")
            
            Dim NextNode As ListViewDataNode Ptr = CurrentNode->NextNode
            
            ' 释放快照
            If CurrentNode->Snapshot <> NULL Then
                LV_Debug("[DeleteListViewData] 释放快照 @" & Hex(CurrentNode->Snapshot))
                Delete CurrentNode->Snapshot
                CurrentNode->Snapshot = NULL
            End If
            
            ' 从链表移除
            If PreviousNode = NULL Then
                ListViewDataList = NextNode
                If ListViewDataList = NULL Then ListViewDataListEnd = NULL
            Else
                PreviousNode->NextNode = NextNode
                If CurrentNode = ListViewDataListEnd Then ListViewDataListEnd = PreviousNode
            End If
            
            LV_Debug("[DeleteListViewData] 释放节点 @" & Hex(CurrentNode))
            Delete CurrentNode
            
            LV_Debug("[DeleteListViewData] Done")
            Exit Sub
        End If
        
        PreviousNode = CurrentNode
        CurrentNode = CurrentNode->NextNode
        Step1 += 1
    Loop
    
    LV_Debug("[DeleteListViewData] 未找到")
End Sub

Function AddItemEx(ListView As Class_ListView, RowIndex As Integer = -1, ColText As StringW, ByVal ForeColor As COLORREF = FB_Black, ByVal BackColor As COLORREF = FB_White) As Integer
    LV_Debug("[AddItemEx] Start RowIndex=" & RowIndex & " Text=" & LeftW(ColText, 20))
    
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        LV_Debug("[AddItemEx] 未找到节点")
        Return -1
    End If
    
    Dim snapshot As ListViewDataSnapshot Ptr = EnsureSnapshot(Node)
    If snapshot = NULL Then
        LV_Debug("[AddItemEx] 快照创建失败")
        Return -1
    End If
    
    ' 如果数组未分配，初始化第一行
    If UBound(snapshot->DataArray, 1) = -1 Then
        ReDim snapshot->DataArray(0, ListView.ColumnCount - 1) As StringW
        ReDim snapshot->ItemColor(0)
        ReDim snapshot->Filter_ItemColor(0)
        RowIndex = 0
    End If
    
    ' 默认添加到末尾
    If RowIndex = -1 Then RowIndex = ListView.ItemCount
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then 
        LV_Debug("[AddItemEx] 行号越界")
        Return -1
    End If
    
    ' 安全扩展数组
    LV_Debug("[AddItemEx] 扩展数组到 " & ListView.ItemCount & " 行")
    ReDim Preserve snapshot->DataArray(ListView.ItemCount, ListView.ColumnCount - 1)
    ReDim Preserve snapshot->ItemColor(ListView.ItemCount)
    ReDim Preserve snapshot->Filter_ItemColor(ListView.ItemCount)
    
    ' 移动现有数据（从后往前）
    For i As Integer = ListView.ItemCount - 1 To RowIndex + 1 Step -1
        For j As Integer = 0 To ListView.ColumnCount - 1
            snapshot->DataArray(i, j) = snapshot->DataArray(i - 1, j)
        Next
        snapshot->ItemColor(i).BackColor = snapshot->ItemColor(i - 1).BackColor
        snapshot->ItemColor(i).ForeColor = snapshot->ItemColor(i - 1).ForeColor
        snapshot->Filter_ItemColor(i).BackColor = snapshot->Filter_ItemColor(i - 1).BackColor
        snapshot->Filter_ItemColor(i).ForeColor = snapshot->Filter_ItemColor(i - 1).ForeColor
    Next
    
    ' 设置新行数据
    snapshot->DataArray(RowIndex, 0) = ColText
    snapshot->ItemColor(RowIndex).BackColor = BackColor
    snapshot->ItemColor(RowIndex).ForeColor = ForeColor
    snapshot->Filter_ItemColor(RowIndex).BackColor = BackColor
    snapshot->Filter_ItemColor(RowIndex).ForeColor = ForeColor
    
    ' 添加到 ListView 控件（仅当通过筛选）
    Dim SubStr As StringW = Node->strFilter
    If InStr(LCaseW(ColText), LCaseW(SubStr)) > 0 Or SubStr = "" Then
        Return ListView.InsertItem(RowIndex, ColText)
    End If
    
    LV_Debug("[AddItemEx] Done")
    Return RowIndex
End Function

Function SetItemTextEx(ListView As Class_ListView, RowIndex As Integer, ColumnIndex As Integer, ColText As StringW) As Boolean
    LV_Debug("[SetItemTextEx] Row=" & RowIndex & " Col=" & ColumnIndex & " Text=" & LeftW(ColText, 20))
    
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then 
        LV_Debug("[SetItemTextEx] 节点为空")
        Return FALSE
    End If
    
    Dim snapshot As ListViewDataSnapshot Ptr = EnsureSnapshot(Node)
    If snapshot = NULL Then 
        LV_Debug("[SetItemTextEx] 快照为空")
        Return FALSE
    End If
    
    If RowIndex < 0 Or RowIndex > ListView.ItemCount - 1 Then 
        LV_Debug("[SetItemTextEx] 行号越界")
        Return FALSE
    End If
    
    ' 动态扩展列数
    ColumnIndex -= 1 ' 转换为 0 基索引
    If ColumnIndex >= ListView.ColumnCount Then
        LV_Debug("[SetItemTextEx] 列索引超出现有列数")
        Return FALSE
    End If
    
    ' 确保数组维度足够
    If UBound(snapshot->DataArray, 1) < RowIndex Or UBound(snapshot->DataArray, 2) < ColumnIndex Then
        LV_Debug("[SetItemTextEx] 数组维度不足")
        Return FALSE
    End If
    
    snapshot->DataArray(RowIndex, ColumnIndex) = ColText
    
    Dim SubStr As StringW = Node->strFilter
    If InStrW(LCaseW(ColText), LCaseW(SubStr)) > 0 Or SubStr = "" Then
        ListView.SetItemText RowIndex, ColumnIndex + 1, ColText
    End If
    
    LV_Debug("[SetItemTextEx] Done")
    Return TRUE
End Function

Function SetItemColor(ListView As Class_ListView, ByVal RowIndex As Integer, ByVal ForeColor As COLORREF = FB_Black, ByVal BackColor As COLORREF = FB_White) As Boolean
    LV_Debug("[SetItemColor] Row=" & RowIndex)
    
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        LV_Debug("[SetItemColor] 节点为空")
        Return FALSE
    End If
    
    Dim snapshot As ListViewDataSnapshot Ptr = EnsureSnapshot(Node)
    If snapshot = NULL Then
        LV_Debug("[SetItemColor] 快照为空")
        Return FALSE
    End If
    
    ' 安全扩展颜色数组
    If RowIndex > UBound(snapshot->ItemColor) Then
        LV_Debug("[SetItemColor] 扩展颜色数组到 " & RowIndex + 10)
        ReDim Preserve snapshot->ItemColor(RowIndex + 10)
        ReDim Preserve snapshot->Filter_ItemColor(RowIndex + 10)
    End If
    
    snapshot->ItemColor(RowIndex).ForeColor = ForeColor
    snapshot->ItemColor(RowIndex).BackColor = BackColor
    snapshot->Filter_ItemColor(RowIndex).ForeColor = ForeColor
    snapshot->Filter_ItemColor(RowIndex).BackColor = BackColor
    
    LV_Debug("[SetItemColor] Done")
    Return TRUE
End Function

Function GetItemColor(ListView As Class_ListView, ByVal RowIndex As Integer, ByRef ForeColor As COLORREF, ByRef BackColor As COLORREF) As Boolean
    ' 1. 获取节点和快照（防御性检查）
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Or Node->Snapshot = NULL Then 
        Print "[GetItemColor] 错误：节点或快照为空"
        Return FALSE
    End If
    
    If RowIndex < 0 Or RowIndex > UBound(Node->Snapshot->ItemColor) Then
        Print "[GetItemColor] 错误：RowIndex=" & RowIndex & " 越界 (Max=" & UBound(Node->Snapshot->ItemColor) & ")"
        Return FALSE
    End If
    
    ' 2. 获取颜色
    Dim snapshot As ListViewDataSnapshot Ptr = Node->Snapshot
    ForeColor = snapshot->ItemColor(RowIndex).ForeColor
    BackColor = snapshot->ItemColor(RowIndex).BackColor
    
    Print "[GetItemColor] 获取 RowIndex=" & RowIndex & " 颜色: Fore=" & Hex(ForeColor) & ", Back=" & Hex(BackColor)
    Return True
End Function

Function AddItemColListEx(ListView As Class_ListView, RowIndex As Integer = -1, iImage As Long = 0, C0 As WString = "", C1 As WString = "", C2 As WString = "", C3 As WString = "", C4 As WString = "", C5 As WString = "", C6 As WString = "", C7 As WString = "", C8 As WString = "", C9 As WString = "") As Long
    LV_Debug("[AddItemColListEx] Start RowIndex=" & RowIndex & " Text=" & LeftW(C0, 20))
    
    ' 1. 获取或创建节点
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        LV_Debug("[AddItemColListEx] 创建新节点")
        Node = InitDataNode(ListView)
    End If
    If Node = NULL Then Return -1
    
    ' 2. 确保快照存在
    Dim snapshot As ListViewDataSnapshot Ptr = EnsureSnapshot(Node)
    If snapshot = NULL Then
        LV_Debug("[AddItemColListEx] 快照创建失败")
        Return -1
    End If
    
    ' 3. 计算当前逻辑行数和目标索引
    Dim CurrentLogicalRows As Integer
    If UBound(snapshot->DataArray, 1) = -1 Then
        CurrentLogicalRows = 0
    Else
        CurrentLogicalRows = UBound(snapshot->DataArray, 1) + 1
    End If
    
    Dim TargetIndex As Integer = RowIndex
    If TargetIndex = -1 Then TargetIndex = CurrentLogicalRows ' 追加到末尾
    
    ' 验证索引有效性
    If TargetIndex < 0 Or TargetIndex > CurrentLogicalRows Then
        LV_Debug("[AddItemColListEx] 索引越界: " & TargetIndex & " (有效范围: 0-" & CurrentLogicalRows & ")")
        Return -1
    End If
    
    LV_Debug("[AddItemColListEx] 当前逻辑行数=" & CurrentLogicalRows & " 目标插入索引=" & TargetIndex)
    
    ' 4. 筛选检查（检查所有列文本）
    Dim SubStr As StringW = Node->strFilter
    Dim FullText As StringW = C0 & C1 & C2 & C3 & C4 & C5 & C6 & C7 & C8 & C9
    Dim IsVisible As Boolean = (SubStr = "" Or InStrW(LCaseW(FullText), LCaseW(SubStr)) > 0)
    
    ' 5. 获取列数（限制为10列）
    Dim ColCount As Integer = ListView.ColumnCount
    If ColCount > 10 Then ColCount = 10
    
    ' 6. 调整快照数组大小
    ' 6.1 DataArray
    ReDim Preserve snapshot->DataArray(CurrentLogicalRows, ColCount - 1) As StringW
    
    ' 6.2 插入时移动数据
    If TargetIndex < CurrentLogicalRows Then
        For i As Integer = CurrentLogicalRows - 1 To TargetIndex Step -1
            For j As Integer = 0 To ColCount - 1
                snapshot->DataArray(i + 1, j) = snapshot->DataArray(i, j)
            Next
        Next
    End If
    
    ' 6.3 ItemColor数组
    ReDim Preserve snapshot->ItemColor(CurrentLogicalRows)
    ReDim Preserve snapshot->Filter_ItemColor(CurrentLogicalRows)
    
    ' 插入时移动颜色
    If TargetIndex < CurrentLogicalRows Then
        For i As Integer = CurrentLogicalRows - 1 To TargetIndex Step -1
            snapshot->ItemColor(i + 1) = snapshot->ItemColor(i)
            snapshot->Filter_ItemColor(i + 1) = snapshot->Filter_ItemColor(i)
        Next
    End If
    
    ' 7. 填充新行数据
    Dim ColTexts(0 To 9) As StringW = {C0, C1, C2, C3, C4, C5, C6, C7, C8, C9}
    For j As Integer = 0 To ColCount - 1
        snapshot->DataArray(TargetIndex, j) = ColTexts(j)
    Next
    
    ' 8. 初始化新行颜色（默认黑色前景，白色背景）
    snapshot->ItemColor(TargetIndex).BackColor = FB_White
    snapshot->ItemColor(TargetIndex).ForeColor = FB_Black
    snapshot->Filter_ItemColor(TargetIndex).BackColor = FB_White
    snapshot->Filter_ItemColor(TargetIndex).ForeColor = FB_Black
    
    ' 9. 如果通过筛选，更新UI
    Dim UIIndex As Integer = -1
    If IsVisible Then
        UIIndex = ListView.InsertItem(TargetIndex, C0)
        If UIIndex >= 0 Then
            For j As Integer = 1 To ColCount - 1
                ListView.SetItemText UIIndex, j, ColTexts(j)
            Next
            
            ' 如需支持图像，取消下面注释并确保控件已关联图像列表
            ' If iImage > 0 Then
            '     Dim LVI As LVITEM
            '     LVI.mask = LVIF_IMAGE
            '     LVI.iItem = UIIndex
            '     LVI.iSubItem = 0
            '     LVI.iImage = iImage
            '     SendMessage(ListView.hWnd, LVM_SETITEM, 0, Cast(LPARAM, @LVI))
            ' End If
        End If
    End If
    
    LV_Debug("[AddItemColListEx] 完成. 逻辑索引=" & TargetIndex & " UI索引=" & UIIndex)
    Return TargetIndex ' 返回逻辑索引，便于后续用SetItemColor修改颜色
    
End Function

/'Function AddItemColListEx(ListView As Class_ListView, RowIndex As Integer = -1, ColText1 As StringW = "", ColText2 As StringW = "", ColText3 As StringW = "", ColText4 As StringW = "", ColText5 As StringW = "", ColText6 As StringW = "", ColText7 As StringW = "", ColText8 As StringW = "", ColText9 As StringW = "", ColText10 As StringW = "") As Boolean '添加(插入)完整的一行(只有满足筛选条件才被显示,但一定会被添加到ListViewDataTable中)
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        '如果未找到对应的节点，添加一个新的节点并初始化数据
        Node = InitDataNode(ListView)
        '重新调整DataArray的边界使其适应ListView的行与列
        'ReDim Node->DataArray(0, ListView.ColumnCount - 1) As String
    End If
    
    Dim SubStr As StringW = Node->strFilter
    If RowIndex = -1 Then RowIndex = ListView.ItemCount
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then Return FALSE
    ' 插入新行
    ReDim Preserve Node->DataArray(ListView.ItemCount, ListView.ColumnCount - 1) As String
    For i As Integer = RowIndex + 1 To ListView.ItemCount
        For j As Integer = 0 To ListView.ColumnCount - 1
            Node->DataArray(i, j) = Node->DataArray(i - 1, j)
        Next
    Next
    Node->DataArray(RowIndex, 0) = ColText1
    Node->DataArray(RowIndex, 1) = ColText2
    Node->DataArray(RowIndex, 2) = ColText3
    Node->DataArray(RowIndex, 3) = ColText4
    Node->DataArray(RowIndex, 4) = ColText5
    Node->DataArray(RowIndex, 5) = ColText6
    Node->DataArray(RowIndex, 6) = ColText7
    Node->DataArray(RowIndex, 7) = ColText8
    Node->DataArray(RowIndex, 8) = ColText9
    Node->DataArray(RowIndex, 9) = ColText10
    If InStr(LCase(ColText1 & ColText2 & ColText3 & ColText4 & ColText5 & ColText6 & ColText7 & ColText8 & ColText9 & ColText10), LCase(SubStr)) > 0 Or SubStr = "" Then
        Dim NewIndex As Integer = ListView.InsertItem(RowIndex, ColText1)
        ListView.SetItemText NewIndex, 1, ColText1
        ListView.SetItemText NewIndex, 2, ColText2
        ListView.SetItemText NewIndex, 3, ColText3
        ListView.SetItemText NewIndex, 4, ColText4
        ListView.SetItemText NewIndex, 5, ColText5
        ListView.SetItemText NewIndex, 6, ColText6
        ListView.SetItemText NewIndex, 7, ColText7
        ListView.SetItemText NewIndex, 8, ColText8
        ListView.SetItemText NewIndex, 9, ColText9
        ListView.SetItemText NewIndex, 10, ColText10
    End If
    Print "SetItemTextEx,RowIndex = " & RowIndex & " " & "ColumnIndex = " & ColumnIndex - 1 & " " & Node->DataArray(RowIndex, ColumnIndex)
    Return TRUE
End Function'/

Function DeleteItemEx(ListView As Class_ListView, ByVal RowIndex As Integer) As Boolean
    ' 删除一行（以筛选后的ListView项索引为基准）
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Or Node->Snapshot = NULL Then Return FALSE
    
    Dim snapshot As ListViewDataSnapshot Ptr = Node->Snapshot
    
    ' 找到真实行索引（如果有过滤）
    Dim TrueRow As Integer = RowIndex
    If Node->strFilter <> "" Then
        TrueRow = FindTrueRow(ListView, RowIndex)
        If TrueRow = -1 Then Return False
    End If
    
    ' 检查边界
    Dim RowCount As Integer = UBound(snapshot->DataArray, 1) + 1
    If TrueRow < 0 Or TrueRow >= RowCount Then Return False
    
    ' 如果只剩一行，删除整个快照
    If RowCount = 1 Then
        Delete snapshot
        Node->Snapshot = NULL
        ListView.DeleteItem RowIndex
        Return TRUE
    End If
    
    ' 移动数据（覆盖被删除的行）
    For i As Integer = TrueRow To RowCount - 2
        For j As Integer = 0 To UBound(snapshot->DataArray, 2)
            snapshot->DataArray(i, j) = snapshot->DataArray(i + 1, j)
        Next
        snapshot->ItemColor(i) = snapshot->ItemColor(i + 1)
        snapshot->Filter_ItemColor(i) = snapshot->Filter_ItemColor(i + 1)
    Next
    
    ' 缩小数组（在快照内部操作，不影响节点）
    ReDim Preserve snapshot->DataArray(RowCount - 2, UBound(snapshot->DataArray, 2)) As StringW
    ReDim Preserve snapshot->ItemColor(RowCount - 2) As ListViewItemInfo
    ReDim Preserve snapshot->Filter_ItemColor(RowCount - 2) As ListViewItemInfo
    
    ' 删除ListView中的可视行
    ListView.DeleteItem RowIndex
    
    Return TRUE
End Function

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

/'Function FrmMain_WndProc(hwnd As HWND, uMsg As UINT, wParam As WPARAM, lParam As LPARAM) As LRESULT
    Select Case uMsg
        Case WM_NOTIFY
            ' 解析 NMHDR 结构体（lParam 指向它）
            Dim As LPNMHDR pNMHDR = Cast(LPNMHDR, lParam)
            ' 判断：是否来自目标 ListView，且是 NM_CUSTOMDRAW 通知
            If pNMHDR->hwndFrom = FrmMain.ListView1.hwnd AndAlso pNMHDR->code = NM_CUSTOMDRAW Then
                Dim As LPNMLVCUSTOMDRAW pLVCD = Cast(LPNMLVCUSTOMDRAW, lParam)
                Dim As LRESULT result = CDRF_DODEFAULT
                Select Case pLVCD->nmcd.dwDrawStage
                    Case CDDS_PREPAINT
                        result = CDRF_NOTIFYITEMDRAW ' 通知项绘制
                    Case CDDS_ITEMPREPAINT
                        ' 第3项设置红色背景
                        pLVCD->clrTextBk = CurrentListViewItem(pLVCD->nmcd.dwItemSpec).BackColor
                        pLVCD->clrText = CurrentListViewItem(pLVCD->nmcd.dwItemSpec).ForeColor
                        result = CDRF_DODEFAULT
                End Select
                Function = result
            End If
        Case Else
        ' 其他消息交给默认窗口过程处理
        Function = DefWindowProc(hwnd, uMsg, wParam, lParam)
    End Select
End Function'/

' 启用/禁用OwnerDraw的优化设置
Sub OptimizeListViewForOwnerDraw(hWndControl As HWND)
    ' 设置双缓冲
    SendMessage(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
    
    ' 禁用动画效果
    SendMessage(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_LABELTIP, LVS_EX_LABELTIP)
    
    ' 设置项目高度（可选）
    'SendMessage(hWndControl, LVM_SETITEMHEIGHT, 0, 20)
End Sub