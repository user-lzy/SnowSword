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

Type ListViewDataNode
    hListView As hWnd 'ListView句柄,用于区分不同ListView
    strFilter As StringW '筛选字符串
    DataArray(Any, Any) As StringW '存储ListView原始数据
    ItemColor(Any) As ListViewItemInfo ' 存储ListView每一行的背景色和前景色
    Filter_ItemColor(Any) As ListViewItemInfo
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

Dim Shared ListViewDataList As ListViewDataNode Ptr ' 链表头指针
Dim Shared ListViewDataListEnd As ListViewDataNode Ptr ' 链表尾指针
Dim Shared CurrentInformationArray() As CURRENT_INFORMATION
Dim Shared pOldWindowProc As PVOID

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
    'DeleteAllMenu mnuMenu
    'Print "OriginalType:" & OriginalType & __FUNCTION__
    'OriginalType = IIf(CurrentType = -1, InterfaceType, CurrentType)
    'CurrentInformation.intType = InterfaceType
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
                TreeList.AddColumn "驱动对象地址", 250
                TreeList.AddColumn "路径", 500
            Case File
                .AddColumn "名称",, 200
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
                .AddColumn "类型",, 160
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

Function FindListViewHwnd(ListView As Class_ListView) As ListViewDataNode Ptr '用于寻找ListView对应的节点指针(如果ListView的数据未被存储过则会返回NULL)
    Dim CurrentNode As ListViewDataNode Ptr = ListViewDataList
    Do While CurrentNode <> NULL
        If CurrentNode->hListView = ListView.hWnd Then Return CurrentNode '符合条件则返回这个节点指针
        CurrentNode = CurrentNode->NextNode '接收下一个节点指针
    Loop
    Return NULL
End Function

Private Function FindTrueRow(ListView As Class_ListView, RowIndex As Integer) As Integer '寻找被筛选过的ListView某行在原始数据中对应的行
    Dim CurrentNode As ListViewDataNode Ptr = ListViewDataList
    Do While CurrentNode <> NULL
        If CurrentNode->hListView = ListView.hWnd Then Exit Do 
        CurrentNode = CurrentNode->NextNode
    Loop
    If CurrentNode = NULL Then Return -1 '未找到则返回-1
    '遍历二维数组中对应ListView的每一个数据
    For i As Integer = 0 To UBound(CurrentNode->DataArray, 1)
        For j As Integer = 0 To UBound(CurrentNode->DataArray, 2)
            If CurrentNode->DataArray(i, j) <> ListView.GetItemText(i, j) Then Exit For '不相等则搜索下一行
        Next
        Return i '找到则返回对应索引
    Next
    Return -1  '找不到则返回-1
End Function

Function InitDataNode(ListView As Class_ListView) As ListViewDataNode Ptr
    Dim Node As ListViewDataNode Ptr = New ListViewDataNode
    Node->hListView = ListView.hWnd
    Node->NextNode = NULL
    ReDim Node->ItemColor(250) As ListViewItemInfo ' 数组开小了就会出bug
    ReDim Node->Filter_ItemColor(250) As ListViewItemInfo ' 数组开小了就会出bug
    For i As Integer = 0 To 250
        Node->ItemColor(i).BackColor = FB_White
        Node->ItemColor(i).ForeColor = FB_Black
        Node->Filter_ItemColor(i).BackColor = FB_White
        Node->Filter_ItemColor(i).ForeColor = FB_Black
    Next
    'Print "5:" & Node->ItemColor(1).BackColor
    If ListViewDataList = NULL Then
        '如果头节点不存在则对其初始化
        ListViewDataList = Node
        ListViewDataListEnd = Node
    Else
        '否则在尾节点后添加一个新节点
        ListViewDataListEnd->NextNode = Node
        ListViewDataListEnd = Node
    End If
    Return Node
End Function

Sub SetListViewData(ListView As Class_ListView) '将ListView原始数据存储至ListViewDataTable中
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView) '寻找对应节点指针
    If Node = NULL Then Node = InitDataNode(ListView) '如果未找到对应的节点，添加一个新的节点并初始化数据
    'Print "6:" & Node->ItemColor(1).BackColor
    '重新调整DataArray的边界使其适应ListView的行与列
    ReDim Node->DataArray(ListView.ItemCount - 1, ListView.ColumnCount - 1) As StringW
    For i As Integer = 0 To ListView.ItemCount - 1
        For j As Integer = 0 To ListView.ColumnCount - 1
            Node->DataArray(i, j) = ListView.GetItemText(i, j) '遍历二维数组每个元素并填充数据
        Next
    Next
    Dim ItemUbound As Integer = UBound(Node->ItemColor)
    ReDim Preserve Node->ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
    ReDim Preserve Node->Filter_ItemColor(ListView.ItemCount - 1) As ListViewItemInfo
    For i As Integer = ItemUbound + 1 To ListView.ItemCount - 1
        Node->ItemColor(i).BackColor = FB_White
        Node->ItemColor(i).ForeColor = FB_Black
        Node->Filter_ItemColor(i).BackColor = FB_White
        Node->Filter_ItemColor(i).ForeColor = FB_Black
    Next
    'Print "1:" & Node->ItemColor(11).BackColor
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

Function FilterAllItem(ListView As Class_ListView) As Integer '开始筛选指定ListView
    Dim ItemCount As Integer = 0
    Dim NewIndex As Integer
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView) '寻找对应节点指针
    If Node = NULL Then Return 0
    Dim SubStr As StringW = Node->strFilter

    ListView.ReDraw = FALSE ' 关闭重绘
    ListView.DeleteAllItems

    If SubStr = "" Then
        ' 如果没有过滤字符串，直接重新添加所有项
        'Print UBound(Node->DataArray, 1)
        'Print UBound(Node->DataArray, 2)
        For i As Integer = 0 To UBound(Node->DataArray, 1)
            ' 恢复颜色映射关系
            Node->Filter_ItemColor(i).BackColor = Node->ItemColor(i).BackColor
            Node->Filter_ItemColor(i).ForeColor = Node->ItemColor(i).ForeColor
            
            ListView.AddItem Node->DataArray(i, 0)
            'Print Node->DataArray(i, 0) & " " & "Row:" & NewIndex
            For j As Integer = 1 To UBound(Node->DataArray, 2)
                'Print Node->DataArray(i, j) & " " & "Col:" & j
                ListView.SetItemText i, j, Node->DataArray(i, j)
            Next
            ItemCount += 1
        Next
    Else
        ' 根据过滤字符串添加匹配的项
        For i As Integer = 0 To UBound(Node->DataArray, 1)
            For j As Integer = 0 To UBound(Node->DataArray, 2)
                If InStrW(LCaseW(Node->DataArray(i, j)), LCaseW(SubStr)) > 0 Then
                    ' 如果找到一个包含指定字符串的项，则添加一整行
                    'Print "找到" & i & " " & j
                    Node->Filter_ItemColor(ItemCount).BackColor = Node->ItemColor(i).BackColor
                    Node->Filter_ItemColor(ItemCount).ForeColor = Node->ItemColor(i).ForeColor
                    
                    NewIndex = ListView.AddItem(Node->DataArray(i, 0))
                    For k As Integer = 1 To UBound(Node->DataArray, 2)
                        ListView.SetItemText NewIndex, k, Node->DataArray(i, k)
                    Next
                    '统计符合筛选条件的项数
                    ItemCount += 1
                    Exit For ' 只要有一个项匹配就不必检查这一行后面的项
                End If
            Next
        Next
    End If
    ListView.ReDraw = True ' 开启重绘
    Return ItemCount
End Function

Sub DeleteListViewData(ListView As Class_ListView) '在不需要筛选ListView时释放空间,但在这之前,如果有需要,请通过将筛选字符串置为空来恢复ListView至原始状态
    Dim CurrentNode As ListViewDataNode Ptr = ListViewDataList
    Dim PreviousNode As ListViewDataNode Ptr = NULL
    Do While CurrentNode <> NULL
        If CurrentNode->hListView = ListView.hWnd Then
            If PreviousNode = NULL Then
                ' 删除头节点
                ListViewDataList = CurrentNode->NextNode
                If ListViewDataList = NULL Then ListViewDataListEnd = NULL
            Else
                ' 删除中间节点或尾节点(使当前节点的上一个节点的NextNode指向当前节点的下一个节点)
                PreviousNode->NextNode = CurrentNode->NextNode
                If CurrentNode = ListViewDataListEnd Then ListViewDataListEnd = PreviousNode
            End If
            ' 释放当前节点内存
            Dim TempNode As ListViewDataNode Ptr = CurrentNode
            'CurrentNode = CurrentNode->NextNode
            Erase TempNode->DataArray
            Delete TempNode
            Exit Sub
        Else
            PreviousNode = CurrentNode
            CurrentNode = CurrentNode->NextNode '接收下一个节点
        End If
    Loop
End Sub

Function AddItemEx(ListView As Class_ListView, RowIndex As Integer = -1, ColText As StringW, ByVal BackColor As COLORREF = FB_White, ByVal ForeColor As COLORREF = FB_Black) As Integer '添加(插入)一行(只有满足筛选条件才被显示,但一定会被添加到ListViewDataTable中)
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then
        '如果未找到对应的节点，添加一个新的节点并初始化数据
        Node = InitDataNode(ListView)
        '重新调整DataArray的边界使其适应ListView的行与列
        'ReDim Node->DataArray(0, ListView.ColumnCount - 1) As String
    End If
    
    Dim SubStr As StringW = Node->strFilter
    If RowIndex = -1 Then RowIndex = ListView.ItemCount
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then Return -1
    ' 插入新行
    ReDim Preserve Node->DataArray(ListView.ItemCount, ListView.ColumnCount - 1) As StringW
    ReDim Preserve Node->ItemColor(ListView.ItemCount) As ListViewItemInfo
    For i As Integer = RowIndex + 1 To ListView.ItemCount
        For j As Integer = 0 To ListView.ColumnCount - 1
            If i > UBound(Node->ItemColor) Then ReDim Preserve Node->ItemColor(i + 10) As ListViewItemInfo
            Node->DataArray(i, j) = Node->DataArray(i - 1, j)
            Node->ItemColor(i).BackColor = BackColor
            Node->ItemColor(i).ForeColor = ForeColor
            Node->Filter_ItemColor(i).BackColor = BackColor
            Node->Filter_ItemColor(i).ForeColor = ForeColor
        Next
    Next
    Node->DataArray(RowIndex, 0) = ColText
    'CurrentListViewItem.BackColor = BackColor
    'CurrentListViewItem.ForeColor = ForeColor
    If InStr(LCase(ColText), LCase(SubStr)) > 0 Or SubStr = "" Then Function = ListView.InsertItem(RowIndex, ColText)
    'Print "ItemCount:" & ListView.ItemCount
    'Print "AddItemEx,RowIndex = " & RowIndex & " " & Node->DataArray(RowIndex, 0)    Return True
End Function

Function SetItemTextEx(ListView As Class_ListView, RowIndex As Integer, ColumnIndex As Integer, ColText As StringW) As Boolean '设置某一行的数据(只有满足筛选条件才被显示,但一定会被添加到ListViewDataTable中)
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then Return FALSE
    Dim SubStr As StringW = Node->strFilter
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then Return False
    
    '拓展新列
    If ListView.ColumnCount - 1 < ColumnIndex Then ReDim Preserve Node->DataArray(RowIndex, ColumnIndex) As String
    Node->DataArray(RowIndex, ColumnIndex) = ColText
    'CurrentListViewItem.BackColor = Node->ItemColor(RowIndex).BackColor
    'CurrentListViewItem.ForeColor = Node->ItemColor(RowIndex).ForeColor
    If InStr(LCase(ColText), LCase(SubStr)) > 0 Or SubStr = "" Then ListView.SetItemText RowIndex, ColumnIndex, ColText
    'Print "SetItemTextEx,RowIndex = " & RowIndex & " " & "ColumnIndex = " & ColumnIndex - 1 & " " & Node->DataArray(RowIndex, ColumnIndex)
    Return TRUE
End Function

Function SetItemColor(ListView As Class_ListView, ByVal RowIndex As Integer, ByVal ForeColor As COLORREF = FB_Black, ByVal BackColor As COLORREF = FB_White) As Boolean '设置某一行的颜色
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then Return False
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then Return False
    'If UBound(Node->ItemColor) > 10 Then Print "2:" & Node->ItemColor(11).BackColor
    ' 检查是否需要扩展数组
    If RowIndex > UBound(Node->ItemColor) Then
        ' 创建临时数组保存现有数据
        Dim OldUpper As Integer = UBound(Node->ItemColor)
        Dim Filter_OldUpper As Integer = UBound(Node->Filter_ItemColor)
        Dim TempArray(OldUpper) As ListViewItemInfo
        Dim Filter_TempArray(Filter_OldUpper) As ListViewItemInfo
        
        ' 备份现有数据
        For i As Integer = 0 To OldUpper
            TempArray(i).BackColor = Node->ItemColor(i).BackColor
            TempArray(i).ForeColor = Node->ItemColor(i).ForeColor
        Next
        For i As Integer = 0 To Filter_OldUpper
            Filter_TempArray(i).BackColor = Node->Filter_ItemColor(i).BackColor
            Filter_TempArray(i).ForeColor = Node->Filter_ItemColor(i).ForeColor
        Next
        
        ' 重新分配数组
        ReDim Node->ItemColor(RowIndex + 10) As ListViewItemInfo
        ReDim Node->Filter_ItemColor(RowIndex + 10) As ListViewItemInfo
        
        ' 恢复备份的数据
        For i As Integer = 0 To OldUpper
            Node->ItemColor(i).BackColor = TempArray(i).BackColor
            Node->ItemColor(i).ForeColor = TempArray(i).ForeColor
        Next
        For i As Integer = 0 To Filter_OldUpper
            Node->Filter_ItemColor(i).BackColor = Filter_TempArray(i).BackColor
            Node->Filter_ItemColor(i).ForeColor = Filter_TempArray(i).ForeColor
        Next
        
        ' 初始化新分配的部分
        For i As Integer = OldUpper + 1 To UBound(Node->ItemColor)
            Node->ItemColor(i).BackColor = FB_White
            Node->ItemColor(i).ForeColor = FB_Black
        Next
        For i As Integer = Filter_OldUpper + 1 To UBound(Node->Filter_ItemColor)
            Node->Filter_ItemColor(i).BackColor = FB_White
            Node->Filter_ItemColor(i).ForeColor = FB_Black
        Next
        
        Print "数组扩展: 从 " & OldUpper & " 到 " & UBound(Node->ItemColor)
        
    End If
    'If UBound(Node->ItemColor) > 10 Then Print "3:" & Node->ItemColor(11).BackColor
    Node->ItemColor(RowIndex).ForeColor = ForeColor
    Node->ItemColor(RowIndex).BackColor = BackColor
    Node->Filter_ItemColor(RowIndex).ForeColor = ForeColor
    Node->Filter_ItemColor(RowIndex).BackColor = BackColor
    'If UBound(Node->ItemColor) > 10 Then Print "4:" & Node->ItemColor(1).BackColor
    Return True
End Function

Function GetItemColor(ListView As Class_ListView, ByVal RowIndex As Integer, ForeColor As COLORREF, BackColor As COLORREF) As Boolean '获取某一行的颜色
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then Return False
    If RowIndex < 0 Or RowIndex > ListView.ItemCount Then Return False
    
    ForeColor = Node->ItemColor(RowIndex).ForeColor
    BackColor = Node->ItemColor(RowIndex).BackColor
    Return True
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
    'Print "SetItemTextEx,RowIndex = " & RowIndex & " " & "ColumnIndex = " & ColumnIndex - 1 & " " & Node->DataArray(RowIndex, ColumnIndex)
    Return TRUE
End Function'/

Function DeleteItemEx(ListView As Class_ListView, ByVal RowIndex As Integer) As Boolean '删除一行(以筛选后的ListView项索引为基准)
    Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView)
    If Node = NULL Then Return FALSE
    Dim TrueRow As Integer = RowIndex
    If Node->strFilter <> "" Then
        TrueRow = FindTrueRow(ListView, RowIndex)
        If TrueRow = -1 Then Return False
    End If
    If TrueRow < 0 Or TrueRow > ListView.ItemCount - 1 Then Return False
    If ListView.ItemCount = 1 Then ' 不能删除最后一行,你删光得了 
        DeleteListViewData ListView 'Return FALSE
        ListView.DeleteItem 0
    End If
    ' 删除指定行
    For i As Integer = TrueRow To ListView.ItemCount - 2
        For j As Integer = 0 To ListView.ColumnCount - 1
            Node->DataArray(i, j) = Node->DataArray(i + 1, j)
            If UBound(Node->ItemColor) > 0 Then
                Node->ItemColor(i).BackColor = Node->ItemColor(i + 1).BackColor
                Node->ItemColor(i).ForeColor = Node->ItemColor(i + 1).ForeColor
            End If
        Next
    Next
    '最后再调整大小
    ReDim Preserve Node->DataArray(ListView.ItemCount - 2, ListView.ColumnCount - 1) As StringW
    If UBound(Node->ItemColor) >0 Then ReDim Preserve Node->ItemColor(ListView.ItemCount - 2) As ListViewItemInfo
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

Sub SetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long, ByVal state As BOOLEAN)
    CheckMenuItem Menu.HMENU, iditem, MF_BYCOMMAND Or IIf(state, MF_CHECKED, MF_UNCHECKED)
End Sub

Function GetMenuText(Menu As Class_Menu, ByVal iditem As Long) As String
    Dim pBuffer As LPSTR = Allocate(MAX_PATH)
    GetMenuStringA Menu.HMENU, iditem, pBuffer, MAX_PATH, MF_BYCOMMAND
    'Print WinErrorMsg(GetLastError) ' 这里GetLastError会是"句柄无效",但我们不用管它,反正已经拿到了pBuffer
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