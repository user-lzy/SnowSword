#include "win/commdlg.bi"

Enum Interface
    ForegroundProcess = 0
    Process
    Module
    GlobalModule
    Thread
    Windows
    Memory
    Handles
    WebConnect
    Hook
    Privilege
    ProcessOtherInfo
    ThreadAllInfo
    MsgHook
    EventHook
    Hotkey
    
    ThreadCallStack
    
    KernelModule
    KernelThread
    WorkItemThread
    ObjectDirectory
    
    File
    FileStream
    
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
    MinifilterInstances
    FilterDriver
    Ndis
    ViolentCheck
    HalDispatch
    HalPrivateDispatch
    ObjectHook
    
    Etw
    EtwProvider
    WinsockSPI
    TaskScheduler
    
    MaxInterfaceCount
End Enum

Type CurrentDriverInfo
    DriverBase As UInteger
    DriverSize As UInteger
    DriverObject As Integer
    DriverName As Wstring * MAX_PATH ' SafeStringW_NTS
    DriverPath As Wstring * MAX_PATH ' SafeStringW_NTS
End Type

Type CurrentModuleInfo
    ModuleName As Wstring * MAX_PATH ' SafeStringW_NTS
    ModuleHandle As Any Ptr
End Type

Type CURRENT_INFORMATION
    hListView As hWnd
    intType As Interface
    ProcessId As DWORD
    ThreadId As DWORD
    CurrentModule As CurrentModuleInfo
    CurrentDriver As CurrentDriverInfo
    FilePath As Wstring * MAX_PATH
End Type

Type TempTextNode
    Buf As WCHAR Ptr
End Type

Type LVRow
    ColCount As Long
    Cols As Wstring Ptr Ptr   ' 二级指针（不变）
    Fore As COLORREF
    Back As COLORREF
    iconIndex As Long
End Type

' 🔥 移除 (Any) → 改为纯指针
Type ListViewSnapshot
    Rows As LVRow Ptr Ptr     ' 👈 替换 Rows(Any)
    RowCount As Long
End Type

Type TVNode
    Text As WString Ptr
    Fore As COLORREF
    Back As COLORREF
    Level As Long
    ParentIndex As Long
End Type

' 🔥 移除 (Any) → 纯指针
Type TreeViewSnapshot
    Nodes As TVNode Ptr Ptr   ' 👈 替换 Nodes(Any)
    NodeCount As Long
End Type

Type TLRow
    ColCount As Long
    Cols As Wstring Ptr Ptr

    cChildren As Long
    Depth As Long
    Expanded As Boolean

    '========== 新增 ==========
    ParentIndex As Long     '父节点索引，根节点=-1

    Match As Boolean        '自身是否匹配筛选
    Visible As Boolean      '最终是否显示

    Fore As COLORREF
    Back As COLORREF
    HasCustomColor As Boolean

    hItem As MC_HTREELISTITEM '当前UI节点句柄
End Type

' 🔥 移除 (Any) → 纯指针
Type TreeListSnapshot
    Rows As TLRow Ptr Ptr     ' 👈 替换 Rows(Any)
    RowCount As Long
    
    '========== 新增 ==========
    VisibleIndex As Long Ptr
    VisibleCount As Long
End Type

Type TreeListItemMap
    Capacity As Long
    Keys As ULONG_PTR Ptr
    Values As Long Ptr
End Type

Type TreeListContext
    hTreeList As hWnd
    Snap As TreeListSnapshot Ptr

    Filter As WString Ptr

    ' 当前控件 hItem -> 快照行索引
    ItemMap As TreeListItemMap Ptr

    SelectedItem As MC_HTREELISTITEM
    IsValid As Boolean
    IsFiltering As Boolean
    SavedSelectedIndex As Long
    SavedTopVisibleIndex As Long
    SavedVisibleCount As Long
End Type

' 2. 模块缓存核心结构体：存储单个模块的所有状态
Type ViewModuleCache
    IsCached As Boolean          ' 标记：是否已缓存数据
    ' 拆分快照：ListView/TreeList 用 pListSnap，TreeView 用 pTreeSnap
    pListSnap As ListViewSnapshot Ptr   ' ListView快照
    pTreeSnap As TreeViewSnapshot Ptr ' TreeView 快照
    pTreeListSnap As TreeListSnapshot Ptr ' 🔥 新增：专属于TreeList
    
    ' 🔥 拆分：ListView 专用
    List_SelectedIndex As Long
    List_TopVisibleIndex As Long
    List_SavedCountPerPage As Long
    
    ' 🔥 拆分：TreeView 专用
    Tree_SelectedIndex As Long
    Tree_TopVisibleIndex As Long
    
    ' 🔥 TreeList 独立字段（新增！）
    TreeList_SelectedIndex As Long
    TreeList_TopVisibleIndex As Long
    TreeList_SavedVisibleCount As Long  ' 🔥 新增：保存TreeList当时的可见项数
    
    ' ===================== 新增：列宽缓存 =====================
    ColumnWidths(Any) As Long      ' 每列宽度数组
    ColumnCount As Long            ' 列数（用于校验）
    ItemCount As Long ' 项数量
    
    ' ===================== 新增：排序状态缓存 =====================
    SortedColumn As Long       ' 保存时的排序列
    SortAscending As Boolean   ' 保存时的排序方向
    
    ' ===================== 新增：可见索引映射（排序+筛选后的顺序）====================
    pVisibleIndex As Long Ptr  ' 排序后的可见行索引数组（指向 Snap 的索引）
    VisibleCount As Long       ' 可见项数量
    
    ' ===================== 新增：筛选条件 =====================
    FilterText As Wstring Ptr  ' 保存时的筛选文本（可选，用于恢复时自动应用）
End Type

' ===================== 列配置：列表头右键菜单"选择列"功能 =====================
Type ColumnConfig
    Names(Any) As WString Ptr     ' 列名（宽字符串堆分配）
    Widths(Any) As Long           ' 保存的列宽（隐藏前备份，恢复时使用）
    Visible(Any) As Boolean       ' 当前可见状态
    Count As Long                 ' 列数
End Type

Type ListViewContext
    hLV As hWnd

    Snap As ListViewSnapshot Ptr

    VisibleIndex(Any) As Long
    VisibleCount As Long

    Filter As Wstring Ptr
    
    SortedColumn As Long       ' 当前排序列索引
    SortAscending As Boolean ' 升降序标记
    
    ' ===================== 新增：列类型缓存（性能优化核心） =====================
    ColumnIsNumber(Any) As Boolean ' 缓存列是否为纯数字列
    
    ' -------------- 新增修复字段 --------------
    IsValid As Boolean         ' 上下文有效标记（核心）
    InstanceId As ULong         ' 唯一实例ID（核心）
End Type

' 3. 全局缓存数组：为每个模块分配独立缓存（核心）
Dim Shared g_ViewCache(MaxInterfaceCount - 1) As ViewModuleCache
' 列配置全局数组（列表头右键菜单"选择列"功能）
Dim Shared g_ColumnConfig(MaxInterfaceCount - 1) As ColumnConfig

' 动态列选择菜单的命令 ID 基数（避免与静态 ID 冲突，静态 ID 通常 < 2000）
Const IDM_SELECT_COLUMN_BASE As ULong = 50000

Const FB_Red As COLORREF = BGR(255, 0, 0)
Const FB_LightPinkRed As COLORREF = BGR(240, 210, 210)
Const FB_LightRed As COLORREF = BGR(250, 225, 225)
Const FB_GoldenYellow As COLORREF = BGR(255, 240, 128)
Const FB_AliceBlue As COLORREF = BGR(240, 248, 255)
Const FB_White As COLORREF = BGR(255, 255, 255)
Const FB_Black As COLORREF = BGR(0, 0, 0)
Const FB_LightGreen As COLORREF = BGR(50, 200, 50)
Const FB_Grey As COLORREF = BGR(212, 212, 212)
Const FB_BoheGreen As COLORREF = BGR(220, 240, 212)

Dim Shared CurrentInformationArray() As CURRENT_INFORMATION
Dim Shared pOldWindowProc As PVOID

' ============================================
' 获取指定文件/文件夹的图标索引
' ============================================
Function GetFileIconIndex(ByVal filePath As String, ByVal isDirectory As Boolean = False) As Long
    Dim sfi As SHFILEINFO
    Dim dwAttr As DWORD = 0
    
    ' 如果是文件夹，设置目录属性标志
    If isDirectory Then dwAttr = FILE_ATTRIBUTE_DIRECTORY
    
    ' 调用 SHGetFileInfo 获取图标索引
    SHGetFileInfo(filePath, dwAttr, @sfi, SizeOf(SHFILEINFO), _
        SHGFI_SYSICONINDEX Or SHGFI_SMALLICON Or SHGFI_USEFILEATTRIBUTES)
    
    ' 返回图标索引（在系统 ImageList 中的位置）
    Return sfi.iIcon
    
End Function

' 图标索引缓存包装函数：同扩展名仅查询一次
Function GetFileIconIndexCached(ByVal filePath As StringW, ByVal isDirectory As Boolean = False) As Long
    ' 文件夹不做缓存，直接调用原函数
    If isDirectory Then Return GetFileIconIndex(filePath, True)
    
    ' 提取扩展名（包含点，如 ".exe"）并转小写
    Dim dotPos As Integer = InStrRevW(filePath, ".")
    If dotPos = 0 Then Return GetFileIconIndex(filePath, False) ' 无扩展名直接查询
    Dim ext As StringW = LCaseW(MidW(filePath, dotPos))
    
    ' 静态缓存：多次调用自动复用，无需全局变量
    Static cacheExt() As StringW
    Static cacheIndex() As Long
    Static cacheCount As Integer = 0
    
    ' 查找缓存命中
    For i As Integer = 0 To cacheCount - 1
        If cacheExt(i) = ext Then
            Return cacheIndex(i)
        End If
    Next
    
    ' 未命中：调用原函数查询
    Dim idx As Long = GetFileIconIndex(filePath, False)
    
    ' 写入缓存（自动扩容）
    If cacheCount = 0 Then
        ReDim cacheExt(0 To 31)
        ReDim cacheIndex(0 To 31)
    ElseIf cacheCount > UBound(cacheExt) Then
        ReDim Preserve cacheExt(0 To cacheCount * 2 - 1)
        ReDim Preserve cacheIndex(0 To cacheCount * 2 - 1)
    End If
    cacheExt(cacheCount) = ext
    cacheIndex(cacheCount) = idx
    cacheCount += 1
    
    Return idx
End Function

Sub InitializeListView(ByVal FuncType As Interface, ListView As Class_ListView)
    ListView.DeleteAllColumn
    ListView.DeleteAllItems ' 此处是否有问题
    With ListView
        Select Case FuncType
            Case ForegroundProcess
                .AddColumn "进程名",, 400
                .AddColumn "内存占用",, 200
                .AddColumn "状态",, 200
            Case Process
                .AddColumn "PID",, 150
                .AddColumn "PPID",, 100
                .AddColumn "EPROCESS",, 270
                .AddColumn "名称",, 200
                .AddColumn "路径",, 500
                .AddColumn "会话-用户名",, 200
                .AddColumn "命令行参数",, 500
                .AddColumn "公司名",, 300
            Case Module
                .AddColumn "序号",, 50
                .AddColumn "名称",, 200
                .AddColumn "基址",, 230
                .AddColumn "大小",, 150
                .AddColumn "路径",, 500
                .AddColumn "公司名",, 300
            Case GlobalModule
                .AddColumn "进程ID",   , 80
                .AddColumn "进程名",   , 200
                .AddColumn "基址",     , 230
                .AddColumn "大小",     , 150
                .AddColumn "路径",     , 500
                .AddColumn "公司名",   , 300
            Case Thread
                '.AddColumn "序号",, 50
                .AddColumn "TID",, 100
                .AddColumn "优先级",, 120
                .AddColumn "状态",, 200
                .AddColumn "ETHREAD",, 270
                .AddColumn "线程入口",, 200
                .AddColumn "Teb",, 200
                '.AddColumn "大小",, 50
                .AddColumn "所属模块",, 1000
                .AddColumn "公司名",, 300
            Case Memory
                .AddColumn "地址",, 250
                .AddColumn "大小",, 100
                .AddColumn "属性",, 250
                .AddColumn "状态",, 200
                .AddColumn "类型",, 200
                .AddColumn "基址",, 250
                .AddColumn "对应模块",, 400
                '.AddColumn "公司名",, 200
            Case Windows
                .AddColumn "句柄",, 150
                .AddColumn "父窗口句柄",, 150
                .AddColumn "名称",, 300
                .AddColumn "当前WndProc",, 250
                .AddColumn "类名",, 300
                .AddColumn "TID",, 100
                .AddColumn "所属模块",, 400
                '.AddColumn "公司名",, 200
            Case Handles
                .AddColumn "类型",, 200
                .AddColumn "名称",, 350
                .AddColumn "值",, 100
                .AddColumn "访问权限",, 150
                .AddColumn "标志",, 200
                .AddColumn "对象地址",, 350
            Case Privilege
                .AddColumn "LUID",, 100
                .AddColumn "名称",, 400
                .AddColumn "状态",, 200
                .AddColumn "描述",, 400
                '.AddColumn "属性",, 100
            Case ProcessTimer
                .AddColumn "定时器ID",, 150
                .AddColumn "超时时间",, 200
                .AddColumn "回调地址",, 250
                .AddColumn "所属模块",, 250
                .AddColumn "所属窗口",, 150
                .AddColumn "所属TID",, 100
                .AddColumn "所属PID",, 100
            Case WebConnect
                .AddColumn "协议",, 80
                .AddColumn "PID",, 80
                .AddColumn "状态",, 200
                .AddColumn "本地地址",, 300
                .AddColumn "远程地址",, 300
                .AddColumn "路径",, 500
            Case Hook
                .AddColumn "Hook类型",, 200
                .AddColumn "地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "代理函数地址",, 300
                .AddColumn "所属模块",, 600
                .AddColumn "当前字节",, 300
                .AddColumn "原始字节",, 300
            Case MsgHook
                .AddColumn "hHook",, 150
                .AddColumn "类型",, 250
                .AddColumn "作用域",, 200
                .AddColumn "回调函数",, 300
                .AddColumn "所属模块",, 500
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
            Case EventHook
                .AddColumn "hHook",, 150
                .AddColumn "作用域",, 300
                .AddColumn "Flags",, 300
                .AddColumn "回调函数",, 250
                .AddColumn "所属模块",, 350
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
                .AddColumn "目标PID",, 100
                .AddColumn "目标TID",, 100
            Case Hotkey
                .AddColumn "hHotkey",, 250
                .AddColumn "热键id",, 200
                .AddColumn "热键组合",, 300
                .AddColumn "所属窗口",, 100
                .AddColumn "PID",, 100
                .AddColumn "TID",, 100
                .AddColumn "备注",, 300
                
            Case ThreadCallStack
                .AddColumn "栈帧序号",, 100
                .AddColumn "地址",, 250
                .AddColumn "名称",, 300
                .AddColumn "模块路径",, 300
                
            Case KernelModule
                .AddColumn "序号",, 100
                .AddColumn "名称",, 200
                .AddColumn "映像基址",, 300
                .AddColumn "映像大小",, 150
                .AddColumn "路径",, 500
                .AddColumn "驱动对象",, 230
            Case KernelThread
                .AddColumn "TID",, 100
                .AddColumn "优先级",, 120
                .AddColumn "状态",, 250
                .AddColumn "ETHREAD",, 270
                .AddColumn "线程入口",, 270
                .AddColumn "所属模块",, 500
            Case WorkItemThread
                .AddColumn "TID",, 100
                .AddColumn "ETHREAD",, 270
                .AddColumn "优先级",, 100
                .AddColumn "状态",, 250
                .AddColumn "入口地址",, 250
                .AddColumn "所属模块",, 500
            Case ObjectDirectory
                .AddColumn "名称",, 300
                .AddColumn "类型",, 200
                .AddColumn "符号链接路径",, 500
            Case SSDT
                .AddColumn "序号",, 100
                .AddColumn "索引",, 100
                .AddColumn "函数名",, 350
                .AddColumn "当前地址",, 300
                .AddColumn "所属模块",, 500
            Case ShadowSSDT
                .AddColumn "序号",, 150
                .AddColumn "Hex索引",, 100
                .AddColumn "函数名",, 350
                .AddColumn "当前地址",, 300
                .AddColumn "所属模块",, 500
            Case Callbacks
                .AddColumn "回调类型",, 250
                .AddColumn "回调函数地址",, 300
                .AddColumn "所属模块",, 500
                .AddColumn "回调函数上下文",, 300
                .AddColumn "备注",, 300
            Case MinifilterInstances
                .AddColumn "Filter",, 180
                .AddColumn "挂载卷路径",, 220
                .AddColumn "高度",, 100
                .AddColumn "实例名称",, 180
                .AddColumn "Frame",, 220
                .AddColumn "Features",, 100
                .AddColumn "Status",, 100
            Case WfpFilter
                .AddColumn "FilterId",, 200
                .AddColumn "CalloutId",, 100
                .AddColumn "名称",, 200
                .AddColumn "Action",, 100
                .AddColumn "Callout GUID",, 500
            Case WfpCallout
                .AddColumn "GUID",, 500
                .AddColumn "CalloutId",, 100
                .AddColumn "Classify",, 300
                .AddColumn "Notify",, 300
                .AddColumn "FlowDelete",, 300
                .AddColumn "名称",, 300
            Case File
                .AddColumn "名称",, 300
                .ADdColumn "类型",, 200
                .AddColumn "大小",, 100
            Case FileStream
                .AddColumn "名称",, 300
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
                .AddColumn "上下文",, 300
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
                .AddColumn "类型",, 180
                .AddColumn "ID",, 180
                .AddColumn "名称",, 180
                .AddColumn "CLSID",, 260
                .AddColumn "提供者路径",, 320
                .AddColumn "公司",, 120
                .AddColumn "文件描述",, 300
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
    InitColumnConfig FuncType, ListView
End Sub

Sub InitializeTreeView(TreeView As Class_TreeView)
    TreeView.DeleteAllItems
End Sub

Sub InitializeTreeList(ByVal FuncType As Interface, TreeList As Class_mCtrlTreeList)
    UninitTreeListData(TreeList)
    TreeList.DeleteItem MC_TLI_ROOT
    For i As Integer = TreeList.GetColumnCount - 1 To 0 Step -1
        TreeList.DeleteColumn i
    Next
    
    With TreeList
        Select Case FuncType
            Case GDT
                .AddColumn "CPU", 150
                .AddColumn "GDT Entry/Base", 250
                .AddColumn "Limit", 100
                .AddColumn "SEL", 100
                .AddColumn "SEL Base", 250
                .AddColumn "SEL Limit", 100
                .AddColumn "DPL", 100
                .AddColumn "Present", 100
                .AddColumn "Granularity", 100
                .AddColumn "Description", 300
            Case IDT
                .AddColumn "CPU", 150
                .AddColumn "ID", 100
                .AddColumn "函数名称", 250
                .AddColumn "当前函数地址", 250
                .AddColumn "SEL", 100
                .AddColumn "DPL", 100
                .AddColumn "所属模块", 500
            Case Minifilter
                .AddColumn "名称", 250
                .AddColumn "回调函数地址", 280
                .AddColumn "所属模块", 500
                .AddColumn "PFLT_FILTER", 280
                .AddColumn "Altitude", 150
            Case FilterDriver
                .AddColumn "驱动名称", 250
                .AddColumn "驱动对象地址", 250
                .AddColumn "路径", 500
                .AddColumn "设备名称", 250
                .AddColumn "设备对象地址", 250
            Case Ndis
                .AddColumn "名称", 400
                .AddColumn "类型", 100
                .AddColumn "标识信息", 500
                .AddColumn "驱动 / 协议映像", 400
                .AddColumn "地址", 200
            Case Etw
                .AddColumn "Name", 300
                .AddColumn "Type", 100
                .AddColumn "ID / GUID", 600
                .AddColumn "Meta", 300
        End Select
    End With
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

' 初始化列配置：从 ListView 读取列名和宽度，默认全部可见
' 调用时机：InitializeListView 各分支之后
Sub InitColumnConfig(ByVal FuncType As Interface, ListView As Class_ListView)
    Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(FuncType)
    
    ' 保存旧的可见性状态（防止重复初始化时丢失用户设置）
    Dim oldVisible() As Boolean
    Dim hasOld As Boolean = (cfg->Count > 0)
    If hasOld Then
        ReDim oldVisible(cfg->Count - 1)
        For i As Long = 0 To cfg->Count - 1
            oldVisible(i) = cfg->Visible(i)
        Next
    End If
    
    ' 释放旧数据
    If cfg->Count > 0 Then
        For i As Long = 0 To cfg->Count - 1
            If cfg->Names(i) <> 0 Then Deallocate cfg->Names(i)
        Next
        Erase cfg->Names
        Erase cfg->Widths
        Erase cfg->Visible
    End If
    cfg->Count = 0
    
    Dim hLV As HWND = ListView.hWnd
    Dim hdr As HWND = ListView_GetHeader(hLV)
    If hdr = NULL Then Exit Sub
    
    Dim colCount As Long = Header_GetItemCount(hdr)
    If colCount <= 0 Then Exit Sub
    
    cfg->Count = colCount
    ReDim cfg->Names(colCount - 1)
    ReDim cfg->Widths(colCount - 1)
    ReDim cfg->Visible(colCount - 1)
    
    Dim buffer As WString * 256
    Dim hdi As HDITEM
    
    For i As Long = 0 To colCount - 1
        ' 读取列标题文本和宽度
        buffer = ""
        hdi.mask = HDI_TEXT Or HDI_WIDTH
        hdi.pszText = @buffer
        hdi.cchTextMax = 256
        SendMessage hdr, HDM_GETITEM, i, Cast(LPARAM, @hdi)
        
        cfg->Names(i) = DupWStr(@buffer)
        cfg->Widths(i) = ListView_GetColumnWidth(hLV, i)
        
        ' 恢复可见性：若旧配置中有匹配列，保留用户选择
        If hasOld AndAlso i <= UBound(oldVisible) Then
            cfg->Visible(i) = oldVisible(i)
        Else
            cfg->Visible(i) = True
        End If
    Next
End Sub

' 设置列可见性：visible=True 恢复保存的宽度，visible=False 将列宽设为 0
Sub SetColumnVisible(ByVal FuncType As Interface, ByVal colIdx As Long, ByVal visible As Boolean, ByVal hLV As HWND)
    Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(FuncType)
    If cfg->Count <= 0 Or colIdx < 0 Or colIdx >= cfg->Count Then Exit Sub
    
    If visible Then
        ' 恢复列宽：若保存宽度为 0，使用默认 100
        Dim w As Long = cfg->Widths(colIdx)
        If w <= 0 Then w = 100
        ListView_SetColumnWidth(hLV, colIdx, w)
    Else
        ' 保存当前宽度，然后隐藏
        cfg->Widths(colIdx) = ListView_GetColumnWidth(hLV, colIdx)
        ListView_SetColumnWidth(hLV, colIdx, 0)
    End If
    cfg->Visible(colIdx) = visible
End Sub

' 获取列可见性
Function GetColumnVisible(ByVal FuncType As Interface, ByVal colIdx As Long) As Boolean
    Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(FuncType)
    If cfg->Count <= 0 Or colIdx < 0 Or colIdx >= cfg->Count Then Return False
    Return cfg->Visible(colIdx)
End Function

' 恢复列可见性：切换回该视图时调用（在 InitializeListView 之后立即调用）
' 将用户之前隐藏的列重新设为宽度 0
Sub RestoreColumnVisibility(ByVal FuncType As Interface, ByVal hLV As HWND)
    Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(FuncType)
    If cfg->Count <= 0 Then Exit Sub
    
    For i As Long = 0 To cfg->Count - 1
        If Not cfg->Visible(i) Then
            ListView_SetColumnWidth(hLV, i, 0)
        End If
    Next
End Sub

Sub Cache_AddRow(ByVal moduleType As Interface, ByVal row As LVRow Ptr)
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache->pListSnap = 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = cache->pListSnap

    ' 新行下标
    Dim idx As Long = snap->RowCount

    snap->Rows = ReAllocate(snap->Rows, (idx + 1) * SizeOf(LVRow Ptr))

    ' 扩容失败保护
    If snap->Rows = 0 Then Exit Sub

    ' 深拷贝新行（不变）
    Dim newRow As LVRow Ptr = CAllocate(1, SizeOf(LVRow))
    newRow->ColCount = row->ColCount
    newRow->Fore = row->Fore
    newRow->Back = row->Back
    newRow->iconIndex = row->iconIndex  ' 🔥 新增
    newRow->Cols = CAllocate(newRow->ColCount, SizeOf(Wstring Ptr))

    Dim i As Long
    For i = 0 To newRow->ColCount - 1
        If row->Cols[i] <> 0 Then
            newRow->Cols[i] = DupWStr(row->Cols[i])
        End If
    Next

    snap->Rows[idx] = newRow

    ' 行数+1
    snap->RowCount += 1
End Sub

Private Sub FreeLVRow(row As LVRow Ptr)
    'Print "  [FreeLVRow] 开始释放行: " & Hex(row)
    If row = 0 Then
        'Print "  [FreeLVRow] 行指针为空，直接退出"
        Exit Sub
    End If

    ' 打印关键信息
    'Print "  [FreeLVRow] ColCount = " & row->ColCount
    'Print "  [FreeLVRow] Cols指针 = " & Hex(row->Cols)

    ' 🔥 崩溃防护 + 调试：先判断 Cols 数组本身
    If row->Cols = 0 Then
        'Print "  [FreeLVRow] ⚠️ Cols 为空指针！跳过列释放，直接释放行"
        Deallocate row
        'Print "  [FreeLVRow] 行释放完成（跳过列）"
        Exit Sub
    End If

    Dim i As Long
    For i = 0 To row->ColCount - 1
        'Print "  [FreeLVRow] 释放列 " & i & ", 指针 = " & Hex(row->Cols[i])
        If row->Cols[i] <> 0 Then
            Deallocate row->Cols[i]
            'Print "  [FreeLVRow] 列 " & i & " 释放完成"
        End If
    Next

    'Print "  [FreeLVRow] 释放 Cols 数组"
    Deallocate row->Cols
    'Print "  [FreeLVRow] 释放行结构体"
    Deallocate row
    'Print "  [FreeLVRow] 行释放完成"
End Sub

Private Sub FreeListViewSnapshot(ByVal snap As ListViewSnapshot Ptr)
    'Print "[FreeListViewSnapshot] 开始释放快照: " & Hex(snap)
    If snap = 0 Then
        'Print "[FreeListViewSnapshot] 快照指针为空，直接退出"
        Exit Sub
    End If

    'Print "[FreeListViewSnapshot] RowCount = " & snap->RowCount
    
    Dim i As Long
    If snap->RowCount > 0 And snap->Rows <> 0 Then
        For i = 0 To snap->RowCount - 1
            If snap->Rows[i] <> 0 Then
                'Print "[FreeListViewSnapshot] 释放第 " & i & " 行"
                FreeLVRow snap->Rows[i]
                snap->Rows[i] = 0
            End If
        Next
        Deallocate snap->Rows  ' 👈 用 Deallocate 替代 Erase
    End If

    Deallocate snap
    'Print "[FreeListViewSnapshot] 快照释放完成"
End Sub

' ==============================================
' 释放 TreeView 快照所有内存（防止内存泄漏）
' ==============================================
Private Sub FreeTreeViewSnapshot(ByVal snap As TreeViewSnapshot Ptr)
    If snap = 0 Then Exit Sub

    Dim i As Long
    If snap->NodeCount > 0 And snap->Nodes <> 0 Then
        For i = 0 To snap->NodeCount - 1
            Dim node As TVNode Ptr = snap->Nodes[i]
            If node = 0 Then Continue For
            If node->Text <> 0 Then Deallocate node->Text
            Deallocate node
        Next
        Deallocate snap->Nodes  ' 👈 替代 Erase
    End If

    Deallocate snap
End Sub

' ==============================================
' 释放 TreeList 快照内存（避免内存泄漏）
' ==============================================
Private Sub FreeTreeListSnapshot(ByVal snap As TreeListSnapshot Ptr)
    'Print "Enter FreeTreeListSnapshot"
    If snap = 0 Then Exit Sub

    Dim i As Long, c As Long
    If snap->RowCount > 0 And snap->Rows <> 0 Then
        For i = 0 To snap->RowCount - 1
            Dim row As TLRow Ptr = snap->Rows[i]
            If row = 0 Then Continue For

            If row->Cols <> 0 Then
                For c = 0 To row->ColCount - 1
                    If row->Cols[c] <> 0 Then Deallocate row->Cols[c]
                Next
                Deallocate row->Cols
            End If
            Deallocate row
        Next
        Deallocate snap->Rows
    End If

    If snap->VisibleIndex <> 0 Then Deallocate snap->VisibleIndex

    Deallocate snap
    'Print "Exit FreeTreeListSnapshot"
End Sub

Private Sub InitAllModuleCache()
    For i As Long = 0 To MaxInterfaceCount - 1
        g_ViewCache(i).IsCached = False
        g_ViewCache(i).List_SelectedIndex = -1
        g_ViewCache(i).List_TopVisibleIndex = 0
        
        g_ViewCache(i).Tree_SelectedIndex = -1
        g_ViewCache(i).Tree_TopVisibleIndex = -1
        
        g_ViewCache(i).ItemCount = 0

        ' 释放ListView快照（不变）
        If g_ViewCache(i).pListSnap <> 0 Then
            FreeListViewSnapshot g_ViewCache(i).pListSnap
            g_ViewCache(i).pListSnap = 0
        End If
        ' 新增：释放可见索引映射
        If g_ViewCache(i).pVisibleIndex <> 0 Then
            Deallocate g_ViewCache(i).pVisibleIndex
            g_ViewCache(i).pVisibleIndex = 0
        End If
        ' 新增：释放筛选文本
        If g_ViewCache(i).FilterText <> 0 Then
            Deallocate g_ViewCache(i).FilterText
            g_ViewCache(i).FilterText = 0
        End If
        
        ' 释放TreeView快照（不变）
        If g_ViewCache(i).pTreeSnap <> 0 Then
            FreeTreeViewSnapshot g_ViewCache(i).pTreeSnap
            g_ViewCache(i).pTreeSnap = 0
        End If
        ' 🔥 新增：释放TreeList快照（专属函数）
        If g_ViewCache(i).pTreeListSnap <> 0 Then
            FreeTreeListSnapshot g_ViewCache(i).pTreeListSnap
            g_ViewCache(i).pTreeListSnap = 0
        End If
    Next
End Sub

Private Function CloneListViewSnapshot(ByVal src As ListViewSnapshot Ptr) As ListViewSnapshot Ptr
    If src = 0 Then Return 0
    If src->RowCount <= 0 Then Return 0

    Dim dst As ListViewSnapshot Ptr = Callocate(1, SizeOf(ListViewSnapshot))
    dst->RowCount = src->RowCount
    ' 🔥 修复：ReDim 改为 Callocate 二级指针
    dst->Rows = Callocate(dst->RowCount, SizeOf(LVRow Ptr))

    Dim i As Long, c As Long
    For i = 0 To src->RowCount - 1
        Dim srow As LVRow Ptr = src->Rows[i]
        If srow = 0 Then Continue For

        Dim drow As LVRow Ptr = Callocate(1, SizeOf(LVRow))
        drow->ColCount = srow->ColCount
        drow->Fore = srow->Fore
        drow->Back = srow->Back
        drow->iconIndex = srow->iconIndex  ' 🔥 新增：复制图标索引

        If drow->ColCount > 0 Then
            drow->Cols = Callocate(drow->ColCount, SizeOf(WString Ptr))
            For c = 0 To drow->ColCount - 1
                If srow->Cols[c] <> 0 Then
                    drow->Cols[c] = DupWStr(srow->Cols[c])
                End If
            Next
        End If

        dst->Rows[i] = drow
    Next

    Return dst
End Function

' ============================================================
' 直接从 ListView 控件读取当前显示状态（包含排序后的顺序）
' ============================================================
Private Function CloneListViewSnapshotFromListView(ByVal lvw As Class_ListView) As ListViewSnapshot Ptr
    Dim itemCount As Long = lvw.ItemCount
    If itemCount <= 0 Then Return 0

    ' 获取列数
    Dim colCount As Long = lvw.ColumnCount
    If colCount <= 0 Then colCount = 1

    Dim dst As ListViewSnapshot Ptr = Callocate(1, SizeOf(ListViewSnapshot))
    dst->RowCount = itemCount
    dst->Rows = Callocate(dst->RowCount, SizeOf(LVRow Ptr))

    Dim i As Long, c As Long
    For i = 0 To itemCount - 1
        ' 🔥 关键：按当前显示顺序读取，i 就是当前视觉顺序的索引
        Dim drow As LVRow Ptr = Callocate(1, SizeOf(LVRow))
        drow->ColCount = colCount
        dst->Rows[i] = drow

        ' 读取每列文本
        drow->Cols = Callocate(drow->ColCount, SizeOf(WString Ptr))
        For c = 0 To colCount - 1
            drow->Cols[c] = DupFromStringW(lvw.GetItemText(i, c))
        Next

        ' 读取颜色
        GetItemColor lvw, i, drow->Fore, drow->Back
        ' 读取图表索引
        drow->iconIndex = lvw.ItemImage(i)
    Next

    Return dst
End Function

' ==============================================
' TreeView 快照深拷贝（100%对齐 CloneListViewSnapshot）
' ==============================================
Private Function CloneTreeViewSnapshot(ByVal src As TreeViewSnapshot Ptr) As TreeViewSnapshot Ptr
    If src = 0 Then Return 0
    If src->NodeCount <= 0 Then Return 0

    Dim dst As TreeViewSnapshot Ptr = Callocate(1, SizeOf(TreeViewSnapshot))
    dst->NodeCount = src->NodeCount
    dst->Nodes = Callocate(dst->NodeCount, SizeOf(TVNode Ptr))  ' 👈 替代 ReDim

    Dim i As Long
    For i = 0 To src->NodeCount - 1
        Dim snode As TVNode Ptr = src->Nodes[i]
        If snode = 0 Then Continue For

        Dim dnode As TVNode Ptr = Callocate(1, SizeOf(TVNode))
        dnode->Text = DupWStr(snode->Text)
        dnode->Fore = snode->Fore
        dnode->Back = snode->Back
        dnode->Level = snode->Level
        dnode->ParentIndex = snode->ParentIndex

        dst->Nodes[i] = dnode
    Next

    Return dst
End Function

' ==============================================
' 深拷贝 TreeList 快照（和你的 CloneListViewSnapshot 逻辑完全一致）
' ==============================================
Private Function CloneTreeListSnapshot(ByVal src As TreeListSnapshot Ptr) As TreeListSnapshot Ptr
    If src = 0 Then Return 0
    If src->RowCount <= 0 Then Return 0

    Dim dst As TreeListSnapshot Ptr = Callocate(1, SizeOf(TreeListSnapshot))
    dst->RowCount = src->RowCount
    dst->Rows = Callocate(dst->RowCount, SizeOf(TLRow Ptr))  ' 👈 替代 ReDim

    Dim i As Long, c As Long
    For i = 0 To src->RowCount - 1
        Dim srow As TLRow Ptr = src->Rows[i]
        If srow = 0 Then Continue For

        Dim drow As TLRow Ptr = Callocate(1, SizeOf(TLRow))
        drow->ColCount = srow->ColCount
        'drow->Fore = srow->Fore
        'drow->Back = srow->Back
        'drow->UserData = srow->UserData
        'drow->iImage = srow->iImage
        'drow->iSelectedImage = srow->iSelectedImage
        'drow->iExpandedImage = srow->iExpandedImage
        drow->cChildren = srow->cChildren

        If drow->ColCount > 0 Then
            drow->Cols = Callocate(drow->ColCount, SizeOf(WString Ptr))
            For c = 0 To drow->ColCount - 1
                If srow->Cols[c] <> 0 Then
                    drow->Cols[c] = DupWStr(srow->Cols[c])
                End If
            Next
        End If

        dst->Rows[i] = drow
    Next

    Return dst
End Function

' 判断 hItem 是否展开
Function IsItemExpanded(tls As Class_mCtrlTreeList, hItem As MC_HTREELISTITEM) As Boolean
    ' 1. 先判断项句柄是否有效
    If hItem = 0 Then Return False
    
    Dim item As MC_TLITEMW
    ' 2. 清空结构体，避免脏数据
    memset(@item, 0, SizeOf(item))
    
    ' 3. 关键：开启状态获取掩码（必须）
    item.fMask = MC_TLIF_STATE
    ' 4. 指定要检测的状态位（展开状态）
    item.stateMask = MC_TLIS_EXPANDED
    
    ' 5. 用正确的控件句柄发送消息
    SendMessage tls.hWnd, MC_TLM_GETITEMW, Cast(wParam, hItem), Cast(lParam, @item)
    
    ' 6. 判断是否展开
    Return ((item.state And MC_TLIS_EXPANDED) <> 0)
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



Sub SaveCurrentListViewState(ByVal lvw As Class_ListView, ByVal moduleType As Interface)
    Dim ctx As ListViewContext Ptr = GetCtx(lvw)
    If ctx = 0 OrElse ctx->Snap = 0 OrElse ctx->Snap->RowCount = 0 Then Exit Sub

    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    
    ' 清理旧数据
    If cache->pListSnap <> 0 Then
        FreeListViewSnapshot cache->pListSnap
        cache->pListSnap = NULL
    End If
    If cache->pVisibleIndex <> 0 Then
        Deallocate cache->pVisibleIndex
        cache->pVisibleIndex = NULL
    End If
    If cache->FilterText <> 0 Then
        Deallocate cache->FilterText
        cache->FilterText = NULL
    End If

    ' ===================== 保存完整数据快照（始终从 ctx->Snap，保证数据完整）====================
    cache->pListSnap = CloneListViewSnapshot(ctx->Snap)
    If cache->pListSnap = NULL Then Exit Sub

    ' ===================== 保存排序+筛选后的可见索引映射 =====================
    ' 这保留了用户当前的视觉顺序，无论是否排序/筛选
    cache->VisibleCount = ctx->VisibleCount
    If ctx->VisibleCount > 0 Then
        cache->pVisibleIndex = Callocate(ctx->VisibleCount, SizeOf(Long))
        memcpy(cache->pVisibleIndex, @ctx->VisibleIndex(0), ctx->VisibleCount * SizeOf(Long))
    End If

    ' 保存排序状态
    cache->SortedColumn = ctx->SortedColumn
    cache->SortAscending = ctx->SortAscending

    ' 保存筛选条件（可选，用于恢复时提示用户）
    If ctx->Filter <> 0 AndAlso *ctx->Filter <> "" Then
        cache->FilterText = DupWStr(ctx->Filter)
    End If

    ' ===================== 保存列宽 =====================
    Dim colCount As Long = lvw.ColumnCount
    If colCount > 0 Then
        ReDim cache->ColumnWidths(0 To colCount - 1)
        cache->ColumnCount = colCount
        Dim c As Long
        For c = 0 To colCount - 1
            cache->ColumnWidths(c) = lvw.ColumnWidth(c)
        Next
    Else
        cache->ColumnCount = 0
        Erase cache->ColumnWidths
    End If

    cache->IsCached = True
    cache->List_SelectedIndex = lvw.SelectedItem
    cache->List_TopVisibleIndex = ListView_GetTopIndex(lvw.hWnd)
    cache->ItemCount = ctx->Snap->RowCount
    cache->List_SavedCountPerPage = lvw.CountPerPage
End Sub

Sub RestoreListViewFromCache(ByVal lvw As Class_ListView, ByVal moduleType As Interface)
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache = NULL OrElse cache->IsCached = False OrElse cache->pListSnap = NULL Then 
        'Print "  缓存无效"
        Exit Sub
    End If

    Dim snap As ListViewSnapshot Ptr = cache->pListSnap
    If snap->RowCount <= 0 OrElse snap->Rows = 0 Then Exit Sub

    InitListViewData lvw
    
    ' 恢复列宽
    Dim currentColCount As Long = lvw.ColumnCount
    If cache->ColumnCount > 0 AndAlso currentColCount = cache->ColumnCount Then
        Dim c As Long
        For c = 0 To cache->ColumnCount - 1
            lvw.ColumnWidth(c) = cache->ColumnWidths(c)
        Next
    End If
    
    ' ===================== 核心：按保存时的可见索引映射顺序恢复数据 =====================
    ' 如果存在排序/筛选映射，按映射顺序添加；否则按原始顺序
    Dim i As Long, c As Long
    If cache->pVisibleIndex <> 0 AndAlso cache->VisibleCount > 0 Then
        ' 有排序/筛选映射：按保存时的视觉顺序恢复
        For i = 0 To cache->VisibleCount - 1
            Dim srcIdx As Long = cache->pVisibleIndex[i]
            If srcIdx < 0 Or srcIdx >= snap->RowCount Then Continue For
            
            Dim row As LVRow Ptr = snap->Rows[srcIdx]
            If row = 0 Then Continue For
            
            Dim rowIdx As Long = AddItemEx(lvw, *row->Cols[0], row->Fore, row->Back)
            For c = 1 To row->ColCount - 1
                If row->Cols[c] <> 0 Then SetItemTextEx lvw, rowIdx, c, *row->Cols[c]
            Next
            SetItemIconEx lvw, rowIdx, row->iconIndex
        Next
        
        ' 恢复排序标记到 ctx（让用户知道当前是排序状态）
        Dim ctx As ListViewContext Ptr = GetCtx(lvw)
        If ctx <> 0 Then
            ctx->SortedColumn = cache->SortedColumn
            ctx->SortAscending = cache->SortAscending
            ' 可选：恢复筛选
            ' If cache->FilterText <> 0 Then SetListViewFilter(lvw, *cache->FilterText)
        End If
    Else
        ' 无排序/筛选：按原始顺序恢复
        For i = 0 To snap->RowCount - 1
            Dim row As LVRow Ptr = snap->Rows[i]
            If row = 0 Then Continue For
            Dim rowIdx As Long = AddItemEx(lvw, *row->Cols[0], row->Fore, row->Back)
            For c = 1 To row->ColCount - 1
                If row->Cols[c] <> 0 Then SetItemTextEx lvw, rowIdx, c, *row->Cols[c]
            Next
            SetItemIconEx lvw, rowIdx, row->iconIndex
        Next
    End If
    
    ' 3. 恢复筛选条件
    If cache->FilterText <> 0 AndAlso *cache->FilterText <> "" Then
        SetListViewFilter(lvw, *cache->FilterText)
    End If
    
    CommitListViewView lvw
    
    ' 5. 如果有排序，重新排序
    If cache->SortedColumn >= 0 Then
        Dim ctx As ListViewContext Ptr = GetCtx(lvw)
        ctx->SortedColumn = cache->SortedColumn
        ctx->SortAscending = cache->SortAscending
        SortListViewVisibleIndex ctx
        ' 排序后需要刷新控件显示
        InvalidateRect lvw.hWnd, NULL, True
    End If

    ' 恢复选中、滚动位置...
    If cache->List_SelectedIndex >= 0 And cache->List_SelectedIndex < lvw.ItemCount Then
        lvw.SelectedItem = cache->List_SelectedIndex
        If cache->List_SavedCountPerPage > 0 Then
            Dim targetTopIndex As Long = cache->List_TopVisibleIndex
            Dim maxIndex As Long = lvw.ItemCount - 1
            Dim scrollToIndex As Long = targetTopIndex + cache->List_SavedCountPerPage - 1
            If scrollToIndex > maxIndex Then scrollToIndex = maxIndex
            ListView_EnsureVisible(lvw.hWnd, scrollToIndex, True)
        End If
        lvw.SetFocus
    End If
End Sub

Sub SaveCurrentTreeViewState(ByVal tvw As Class_TreeView, ByVal moduleType As Interface)
    'Print "===== [DEBUG] SaveTreeView 模块: " & moduleType & " ====="
    If g_ViewCache(moduleType).pTreeSnap <> NULL Then
        FreeTreeViewSnapshot g_ViewCache(moduleType).pTreeSnap
        g_ViewCache(moduleType).pTreeSnap = NULL
    End If

    Dim totalCount As Long = tvw.GetCount
    'Print "  总项数: " & totalCount
    If totalCount <= 0 Then Exit Sub

    Dim pNewSnap As TreeViewSnapshot Ptr = Callocate(1, SizeOf(TreeViewSnapshot))
    pNewSnap->NodeCount = totalCount
    pNewSnap->Nodes = Callocate(pNewSnap->NodeCount, SizeOf(TVNode Ptr))

    Dim hItemMap As HTREEITEM Ptr = Callocate(totalCount, SizeOf(HTREEITEM))
    Dim index As Long = 0
    Dim hCur As HTREEITEM = tvw.GetRoot()

    While hCur <> 0 And index < totalCount
        Dim pNode As TVNode Ptr = Callocate(1, SizeOf(TVNode))
        pNode->Text = DupWStr(tvw.Text(hCur))
        pNode->Level = tvw.GetLevel(hCur)
        pNode->ParentIndex = -1

        hItemMap[index] = hCur
        pNewSnap->Nodes[index] = pNode
        index += 1

        Dim hChild As HTREEITEM = tvw.GetChild(hCur)
        If hChild <> 0 Then hCur = hChild : Continue While
        Dim hSibling As HTREEITEM = tvw.GetNextSiblin(hCur)
        If hSibling <> 0 Then hCur = hSibling : Continue While
        Dim hParent As HTREEITEM = tvw.GetParent(hCur)
        If hParent <> 0 Then hCur = tvw.GetNextSiblin(hParent) Else hCur = 0
    Wend

    For index = 0 To pNewSnap->NodeCount - 1
        hCur = hItemMap[index]
        Dim hParentNode As HTREEITEM = tvw.GetParent(hCur)
        If hParentNode <> 0 Then
            For i As Long = 0 To pNewSnap->NodeCount - 1
                If hItemMap[i] = hParentNode Then pNewSnap->Nodes[index]->ParentIndex = i : Exit For
            Next
        End If
    Next

    g_ViewCache(moduleType).pTreeSnap = pNewSnap
    g_ViewCache(moduleType).IsCached = True
    g_ViewCache(moduleType).ItemCount = totalCount

    ' 调试：保存选中/顶部索引
    g_ViewCache(moduleType).Tree_SelectedIndex = -1
    Dim hSel As HTREEITEM = tvw.Selection
    If hSel <> 0 Then
        For index = 0 To pNewSnap->NodeCount - 1
            If hItemMap[index] = hSel Then g_ViewCache(moduleType).Tree_SelectedIndex = index : Exit For
        Next
    End If

    g_ViewCache(moduleType).Tree_TopVisibleIndex = -1
    Dim hVisible As HTREEITEM = tvw.GetFirstVisible()
    If hVisible <> 0 Then
        For index = 0 To pNewSnap->NodeCount - 1
            If hItemMap[index] = hVisible Then g_ViewCache(moduleType).Tree_TopVisibleIndex = index : Exit For
        Next
    End If

    ' 🔥 关键调试输出
    /'Print "  保存选中索引: " & g_ViewCache(moduleType).Tree_SelectedIndex
    Print "  保存顶部可见索引: " & g_ViewCache(moduleType).Tree_TopVisibleIndex
    Print "============================================="'/
    
    Deallocate hItemMap
End Sub

' =============================================================================
Sub RestoreTreeViewFromCache(ByVal tvw As Class_TreeView, ByVal moduleType As Interface)
    'Print "===== [DEBUG] RestoreTreeView 模块: " & moduleType & " ====="
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache = NULL OrElse cache->IsCached = False OrElse cache->pTreeSnap = NULL Then 
        'Print "  缓存无效，退出"
        Exit Sub
    End If

    Dim snap As TreeViewSnapshot Ptr = cache->pTreeSnap
    'Print "  缓存总项数: " & snap->NodeCount
    'Print "  缓存选中: " & cache->Tree_SelectedIndex & " | 缓存顶部: " & cache->Tree_TopVisibleIndex
    If snap->NodeCount <= 0 OrElse snap->Nodes = 0 Then Exit Sub

    tvw.DeleteAllItems()
    Dim hNewNodes(snap->NodeCount - 1) As HTREEITEM
    Dim i As Long

    For i = 0 To snap->NodeCount - 1
        Dim node As TVNode Ptr = snap->Nodes[i]
        If node = 0 Then Continue For
        If node->ParentIndex = -1 Then
            hNewNodes(i) = tvw.InsertItem(TVI_ROOT, TVI_LAST, *node->Text)
        Else
            hNewNodes(i) = tvw.InsertItem(hNewNodes(node->ParentIndex), TVI_LAST, *node->Text)
        End If
    Next

    ' 🔥 修复：控件必须先渲染完成，再调用滚动API（核心坑！）
    'FF_DoEvents
    'tvw.Refresh

    ' 恢复选中
    If cache->Tree_SelectedIndex >= 0 And cache->Tree_SelectedIndex < snap->NodeCount Then
        tvw.Selection = hNewNodes(cache->Tree_SelectedIndex)
        'Print "  执行恢复选中: " & cache->Tree_SelectedIndex
    End If

    ' 恢复滚动条
    If cache->Tree_TopVisibleIndex >= 0 And cache->Tree_TopVisibleIndex < snap->NodeCount Then
        tvw.SelectSetFirstVisible(hNewNodes(cache->Tree_TopVisibleIndex))
        'Print "  执行恢复顶部: " & cache->Tree_TopVisibleIndex
    End If

    If cache->Tree_SelectedIndex >= 0 Then tvw.SetFocus()
    'Print "============================================="
End Sub

Sub SaveCurrentTreeListState(ByVal tls As Class_mCtrlTreeList, ByVal moduleType As Interface)
    Dim pCurrentCtx As TreeListContext Ptr = Cast(TreeListContext Ptr, _
        GetWindowLongPtr(tls.hWnd, GWLP_USERDATA))
    Dim pCurrentCache As ViewModuleCache Ptr = @g_ViewCache(moduleType)

    ' 已绑定的快照是唯一数据源；切换模块时只同步 UI 状态，不能重新
    ' 从筛选后的控件采集，否则会丢失隐藏节点和项颜色。
    If pCurrentCtx <> 0 AndAlso pCurrentCtx->Snap <> 0 AndAlso _
       pCurrentCache->pTreeListSnap = pCurrentCtx->Snap Then
        TreeListCaptureViewState(tls, pCurrentCtx)
        pCurrentCache->TreeList_SelectedIndex = pCurrentCtx->SavedSelectedIndex
        pCurrentCache->TreeList_TopVisibleIndex = pCurrentCtx->SavedTopVisibleIndex
        pCurrentCache->TreeList_SavedVisibleCount = pCurrentCtx->SavedVisibleCount
        Exit Sub
    End If

    CaptureTreeListData(tls, moduleType)
    Exit Sub

    'Print "=== [Save] Enter ==="
    
    If g_ViewCache(moduleType).pTreeListSnap <> 0 Then
        FreeTreeListSnapshot g_ViewCache(moduleType).pTreeListSnap
        g_ViewCache(moduleType).pTreeListSnap = NULL
    End If

    ' 统计所有节点真实总数（DFS 遍历）
    Dim totalCount As Long = 0
    Dim cur As MC_HTREELISTITEM = tls.GetRoot()
    While cur <> 0
        totalCount += 1
        Dim child As MC_HTREELISTITEM = tls.GetNextItem(cur, MC_TLGN_CHILD)
        If child <> 0 Then
            cur = child
            Continue While
        End If
        Dim sib As MC_HTREELISTITEM = tls.GetNextItem(cur, MC_TLGN_NEXT)
        If sib <> 0 Then
            cur = sib
            Continue While
        End If
        Do While cur <> 0
            sib = tls.GetNextItem(cur, MC_TLGN_NEXT)
            If sib <> 0 Then
                cur = sib
                Exit Do
            End If
            cur = tls.GetNextItem(cur, MC_TLGN_PARENT)
        Loop
    Wend
    'Print "[Save] 真实总节点数 = " & totalCount
    If totalCount <= 0 Then Exit Sub

    Dim colCount As Long = tls.GetColumnCount : If colCount <= 0 Then colCount = 1
    Dim pNewSnap As TreeListSnapshot Ptr = Callocate(1, SizeOf(TreeListSnapshot))
    pNewSnap->RowCount = totalCount
    pNewSnap->Rows = CAllocate(totalCount, SizeOf(TLRow Ptr))
    Dim hItemMap As MC_HTREELISTITEM Ptr = Callocate(totalCount, SizeOf(MC_HTREELISTITEM))

    ' 遍历保存数据（DFS + 深度计算）
    Dim index As Long = 0
    Dim hCur As MC_HTREELISTITEM = tls.GetRoot()
    Dim depth As Long = 0                     

    While hCur <> 0 And index < totalCount
        'Print "[Save] 保存索引 " & index & ", hCur = " & Hex(hCur)
        
        Dim pRow As TLRow Ptr = Callocate(1, SizeOf(TLRow))
        pRow->colCount = colCount
        pRow->Cols = Callocate(colCount, SizeOf(WString Ptr))

        ' 保存列文本
        Dim c As Long
        For c = 0 To colCount - 1
            Dim swText As StringW = tls.GetItemText(hCur, c)
            pRow->Cols[c] = DupWStr(StrPtrW(swText))
        Next

        ' 保存属性 + 深度
        pRow->cChildren = IIf(tls.GetChildCount(hCur) > 0, 1, 0)
        pRow->depth = depth
        pRow->Expanded = IsItemExpanded(tls, hCur)
        ' 【新增调试】打印所有节点的状态，重点看 hrdevmon
        Dim nodeText As StringW = tls.GetItemText(hCur, 0)
        'If pRow->Expanded Then Print "[DEBUG Save] 索引=" & index & " 文本=[" & nodeText & "] Expanded=" & pRow->Expanded & " 深度=" & depth

        hItemMap[index] = hCur
        pNewSnap->Rows[index] = pRow
        index += 1

        ' DFS 移动 + 深度更新
        Dim hChild As MC_HTREELISTITEM = tls.GetNextItem(hCur, MC_TLGN_CHILD)
        If hChild <> 0 Then
            depth += 1                     
            hCur = hChild
            Continue While
        End If

        Dim hNext As MC_HTREELISTITEM = tls.GetNextItem(hCur, MC_TLGN_NEXT)
        If hNext <> 0 Then
            hCur = hNext
            Continue While
        End If

        ' 回溯更新深度
        Do
            hCur = tls.GetNextItem(hCur, MC_TLGN_PARENT)
            If hCur = 0 Then
                depth = 0                 
                Exit Do
            End If
            depth -= 1                    
            hNext = tls.GetNextItem(hCur, MC_TLGN_NEXT)
            If hNext <> 0 Then
                hCur = hNext               
                Exit Do
            End If
        Loop
    Wend

    ' 保存缓存
    g_ViewCache(moduleType).pTreeListSnap = pNewSnap
    g_ViewCache(moduleType).IsCached = True
    g_ViewCache(moduleType).ItemCount = totalCount
    g_ViewCache(moduleType).TreeList_SavedVisibleCount = tls.GetVisibleCount

    ' ==============================================
    ' 🔥 核心修改：TreeList 独立保存选中/顶部索引
    ' ==============================================
    g_ViewCache(moduleType).TreeList_SelectedIndex = -1
    Dim hSel As MC_HTREELISTITEM = tls.Selection
    If hSel <> 0 Then
        For index = 0 To totalCount - 1
            If hItemMap[index] = hSel Then 
                g_ViewCache(moduleType).TreeList_SelectedIndex = index 
                Exit For
            End If
        Next
    End If

    g_ViewCache(moduleType).TreeList_TopVisibleIndex = -1
    ' 获取控件真实的第一个可见项（顶部）
    Dim hLastVisible As MC_HTREELISTITEM = tls.GetNextItem(NULL, MC_TLGN_FIRSTVISIBLE)
    If hLastVisible <> 0 Then
        For index = 0 To totalCount - 1
            If hItemMap[index] = hLastVisible Then 
                ' 直接保存顶部可见项的DFS索引
                g_ViewCache(moduleType).TreeList_TopVisibleIndex = index
                Exit For
            End If
        Next
    End If

    Deallocate hItemMap
    'Print "=== [Save] Exit ==="
End Sub
Sub PrintExpandedItem(tls As Class_mCtrlTreeList)
    Dim cur As MC_HTREELISTITEM = tls.GetRoot()
    While cur <> 0
        'If IsItemExpanded(tls, cur) Then Print "[Restore Info]" & tls.GetItemText(cur, 0) & "被展开!"
        Dim child As MC_HTREELISTITEM = tls.GetNextItem(cur, MC_TLGN_CHILD)
        If child <> 0 Then
            cur = child
            Continue While
        End If
        Dim sib As MC_HTREELISTITEM = tls.GetNextItem(cur, MC_TLGN_NEXT)
        If sib <> 0 Then
            cur = sib
            Continue While
        End If
        Do While cur <> 0
            sib = tls.GetNextItem(cur, MC_TLGN_NEXT)
            If sib <> 0 Then
                cur = sib
                Exit Do
            End If
            cur = tls.GetNextItem(cur, MC_TLGN_PARENT)
        Loop
    Wend
End Sub
Sub RestoreTreeListFromCache(ByVal tls As Class_mCtrlTreeList, ByVal moduleType As Interface)
    Dim pRestoreCache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If pRestoreCache = 0 OrElse pRestoreCache->IsCached = False OrElse _
       pRestoreCache->pTreeListSnap = 0 Then
        Exit Sub
    End If

    BindTreeListData(tls, pRestoreCache->pTreeListSnap)
    Dim pRestoreCtx As TreeListContext Ptr = Cast(TreeListContext Ptr, _
        GetWindowLongPtr(tls.hWnd, GWLP_USERDATA))
    If pRestoreCtx = 0 Then Exit Sub

    TreeListShowAllRows(pRestoreCtx)
    TreeListRebuild(tls, pRestoreCtx, False, _
        pRestoreCache->TreeList_SelectedIndex, _
        pRestoreCache->TreeList_TopVisibleIndex, _
        pRestoreCache->TreeList_SavedVisibleCount)
    Exit Sub

    'Print "=== [Restore] Enter ==="
    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache = 0 OrElse cache->IsCached = False OrElse cache->pTreeListSnap = 0 Then
        'Print "[Restore] 无有效缓存，退出"
        Exit Sub
    End If
    
    Dim snap As TreeListSnapshot Ptr = cache->pTreeListSnap
    'Print "[Restore] 缓存总节点数 = " & snap->RowCount
    'Print "[Restore] 选中索引: " & cache->TreeList_SelectedIndex & " | 顶部索引: " & cache->TreeList_TopVisibleIndex
    If snap->RowCount <= 0 OrElse snap->Rows = 0 Then Exit Sub
    
    ' 清空控件
    tls.DeleteItem MC_TLI_ROOT
    
    ' 堆分配句柄数组（严格对应DFS索引）
    Dim hNewItems As MC_HTREELISTITEM Ptr = Callocate(snap->RowCount, SizeOf(MC_HTREELISTITEM))

    ' 父节点栈
    Dim hParentStack(1023) As MC_HTREELISTITEM
    Dim top As Long = -1

    Dim i As Long, c As Long
    For i = 0 To snap->RowCount - 1
        Dim row As TLRow Ptr = snap->Rows[i]
        If row = 0 Or row->Cols = 0 Then
            hNewItems[i] = 0
            Continue For
        End If

        Dim curDepth As Long = row->Depth

        ' 调整父栈
        Do While top >= 0 AndAlso top >= curDepth
            top -= 1
        Loop

        Dim hParent As MC_HTREELISTITEM = MC_TLI_ROOT
        If top >= 0 Then hParent = hParentStack(top)

        Dim szText As WString * MAX_PATH = ""
        If row->Cols[0] <> 0 Then szText = *row->Cols[0]

        hNewItems[i] = tls.AddItem(hParent, szText)

        ' 设置列文本
        For c = 1 To row->ColCount - 1
            If row->Cols[c] <> 0 And hNewItems[i] <> 0 Then
                tls.SetItemText(hNewItems[i], c, *row->Cols[c])
            End If
        Next
        
        ' 压栈
        If row->cChildren > 0 Then
            top += 1
            If top <= 1023 Then hParentStack(top) = hNewItems[i]
        End If
    Next
    
    ' 恢复选中项（暂存，最后处理）
    Dim hSelItem As MC_HTREELISTITEM = 0
    If cache->TreeList_SelectedIndex >= 0 And cache->TreeList_SelectedIndex < snap->RowCount Then
        hSelItem = hNewItems[cache->TreeList_SelectedIndex]
    End If

    ' ==============================================
    ' 🔥 完全恢复你的原代码：统一设置展开/折叠（保留 TOGGLE 和调试打印）
    ' ==============================================
    'Print "[DEBUG Restore] 开始第二阶段：统一设置展开/折叠"
    For i = 0 To snap->RowCount - 1
        Dim row As TLRow Ptr = snap->Rows[i]
        If hNewItems[i] = 0 Or row = 0 Then Continue For
        
        ' 核心修复：严格按照缓存的状态，仅展开/折叠 当前节点
        If row->Expanded Then
            ' 🔥 恢复你的调试打印
            'If row->Cols[0] <> 0 Then Print "被展开节点:" & *row->Cols[0]
            ' 🔥 保留你的 TOGGLE 逻辑（仅展开一层）
            tls.Expand(hNewItems[i], MC_TLE_TOGGLE, 0)
        Else
            ' 折叠当前节点（你的原代码注释掉了，我也保持注释）
            'tls.Expand(hNewItems[i], MC_TLE_COLLAPSE, 0)
        End If
    Next
    PrintExpandedItem(tls)
    
    ' ==============================================
    ' 🔥 终极滚动修复：利用 EnsureVisible 置底特性，反向推算置顶
    ' ==============================================
    Dim hTargetTop As MC_HTREELISTITEM = 0
    If cache->TreeList_TopVisibleIndex >= 0 And cache->TreeList_TopVisibleIndex < snap->RowCount Then
        hTargetTop = hNewItems[cache->TreeList_TopVisibleIndex]
    End If

    If hTargetTop <> 0 Then
        Dim visibleRows As Long = cache->TreeList_SavedVisibleCount
        If visibleRows <= 0 Then visibleRows = tls.GetVisibleCount()
        
        ' 1. 先确保目标项可见
        tls.EnsureVisible(hTargetTop)
        
        ' 2. 🔥 核心逻辑：从目标项开始，向下数 (visibleRows - 1) 个“可见项”
        Dim hScrollItem As MC_HTREELISTITEM = hTargetTop
        Dim count As Long = 0
        
        ' 循环获取下一个可见项，直到数满一屏
        While count < visibleRows - 1 And hScrollItem <> 0
            Dim hNextVis As MC_HTREELISTITEM = tls.GetNextItem(hScrollItem, MC_TLGN_NEXTVISIBLE)
            If hNextVis = 0 Then Exit While
            hScrollItem = hNextVis
            count += 1
        Wend
        
        ' 3. 🔥 对最后这个项调用 EnsureVisible（利用它置底的特性，把目标顶到顶部）
        If hScrollItem <> 0 Then
            tls.EnsureVisible(hScrollItem)
            'Print "[DEBUG] 反向置顶成功：目标DFS索引=" & cache->TreeList_TopVisibleIndex
        End If
    End If
    
    ' 最后处理选中项（避免干扰滚动）
    If hSelItem <> 0 Then
        tls.Checked(hSelItem) = TRUE
        tls.SetFocus
    End If
    
    ' 释放内存
    Deallocate hNewItems
    'Print "=== [Restore] Exit ==="
End Sub

Sub Cache_DeleteRow(ByVal moduleType As Interface, ByVal index As Long)
    ' 【必加】首先校验模块索引合法性
    If moduleType < 0 Or moduleType >= MaxInterfaceCount Then
        OutputDebugString "Cache_DeleteRow: 非法 moduleType！"
        'Exit Sub
    End If

    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache = 0 Then Exit Sub
    If cache->pListSnap = 0 Then Exit Sub

    Dim snap As ListViewSnapshot Ptr = cache->pListSnap
    ' 【必加】索引边界校验
    If index < 0 Or index >= snap->RowCount Then Exit Sub

    ' 释放该行
    FreeLVRow snap->Rows[index]

    ' 前移数组
    Dim i As Long
    For i = index To snap->RowCount - 2
        snap->Rows[i] = snap->Rows[i + 1]
    Next
    snap->RowCount -= 1

    ' 调整数组大小
    If snap->RowCount > 0 Then
        snap->Rows = Reallocate(snap->Rows, snap->RowCount * SizeOf(LVRow Ptr))
    Else
        Deallocate snap->Rows
        snap->Rows = 0
    End If
End Sub

' 辅助函数：将 0x 前缀转换为 FreeBasic 认识的 &H 前缀
Private Function HexToVal(hexStr As WString Ptr) As UInteger
    If hexStr = NULL OrElse *hexStr = WStr("") Then Return 0
    
    Dim s As WString * 64
    Dim lenS As Long = Len(*hexStr)
    
    If lenS >= 2 AndAlso (Mid(*hexStr, 1, 2) = WStr("0x") OrElse Mid(*hexStr, 1, 2) = WStr("0X")) Then
        s = WStr("&H") & Mid(*hexStr, 3)
    Else
        s = *hexStr
    End If
    
    Return ValULng(s)
End Function

' 统一行比较函数：返回>0表示idx1应排在idx2后，<0表示排在前，=0相等
Private Function CompareRow( _
    ctx As ListViewContext Ptr, _
    idx1 As Long, _
    idx2 As Long, _
    sortCol As Long, _
    isNumber As Boolean _
) As Long
    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    Dim row1 As LVRow Ptr = snap->Rows[idx1]
    Dim row2 As LVRow Ptr = snap->Rows[idx2]
    
    ' 空行处理：空行统一排最后
    If row1 = NULL AndAlso row2 = NULL Then Return 0
    If row1 = NULL Then Return 1
    If row2 = NULL Then Return -1
    
    ' 列越界保护
    If sortCol >= row1->ColCount OrElse sortCol >= row2->ColCount Then Return 0
    
    Dim cell1 As WString Ptr = row1->Cols[sortCol]
    Dim cell2 As WString Ptr = row2->Cols[sortCol]
    
    ' 空单元格处理：空值排最后
    Dim empty1 As Boolean = (cell1 = NULL OrElse *cell1 = WStr(""))
    Dim empty2 As Boolean = (cell2 = NULL OrElse *cell2 = WStr(""))
    If empty1 AndAlso empty2 Then Return 0
    If empty1 Then Return 1
    If empty2 Then Return -1
    
    If isNumber Then
        ' 数值比较
        Dim val1 As Double, val2 As Double
        ' 判断是否为十六进制字符串（含 0x 前缀）
        If InStrW(*cell1, WStr("0x")) = 1 OrElse InStrW(*cell1, WStr("0X")) = 1 Then
            val1 = CDbl(HexToVal(*cell1))
        Else
            val1 = Val(*cell1)
        End If
        
        If InStrW(*cell2, WStr("0x")) = 1 OrElse InStrW(*cell2, WStr("0X")) = 1 Then
            val2 = CDbl(HexToVal(*cell2))
        Else
            val2 = Val(*cell2)
        End If
        If val1 > val2 Then Return 1
        If val1 < val2 Then Return -1
        Return 0
    Else
        ' 系统API字符串比较，性能远高于手写循环
        Return lstrcmpW(cell1, cell2)
    End If
End Function

' 插入排序：仅用于32行以内的小数据量
Private Sub InsertionSort( _
    arr() As Long, _
    ctx As ListViewContext Ptr, _
    sortCol As Long, _
    isNumber As Boolean, _
    isAsc As Boolean _
)
    Dim count As Long = UBound(arr) + 1
    If count <= 1 Then Exit Sub
    
    For i As Long = 1 To count - 1
        Dim key As Long = arr(i)
        Dim j As Long = i - 1
        
        While j >= 0
            Dim cmp As Long = CompareRow(ctx, arr(j), key, sortCol, isNumber)
            ' 升序：前值大于当前值则后移；降序反之
            If (isAsc AndAlso cmp > 0) OrElse (Not isAsc AndAlso cmp < 0) Then
                arr(j + 1) = arr(j)
                j -= 1
            Else
                Exit While
            End If
        Wend
        arr(j + 1) = key
    Next
End Sub

' 三数取中选基准：避免有序数据下快排退化到O(n²)
Private Function GetMedian( _
    arr() As Long, _
    nLeft As Long, _
    nMid As Long, _
    nRight As Long, _
    ctx As ListViewContext Ptr, _
    sortCol As Long, _
    isNumber As Boolean, _
    isAsc As Boolean _
) As Long
    Dim cmpLM As Long = CompareRow(ctx, arr(nLeft), arr(nMid), sortCol, isNumber)
    Dim cmpMR As Long = CompareRow(ctx, arr(nMid), arr(nRight), sortCol, isNumber)
    Dim cmpLR As Long = CompareRow(ctx, arr(nLeft), arr(nRight), sortCol, isNumber)
    
    If (cmpLM <= 0 AndAlso cmpMR >= 0) OrElse (cmpLM >= 0 AndAlso cmpMR <= 0) Then
        Return nMid
    ElseIf (cmpLR <= 0 AndAlso cmpMR <= 0) OrElse (cmpLR >= 0 AndAlso cmpMR >= 0) Then
        Return nRight
    Else
        Return nLeft
    End If
End Function

' 迭代版快速排序：用自定义栈替代递归，彻底避免栈溢出
Private Sub ListView_QuickSort_Iterative( _
    arr() As Long, _
    ctx As ListViewContext Ptr, _
    sortCol As Long, _
    isNumber As Boolean, _
    isAsc As Boolean _
)
    Dim nLeft As Long = 0
    Dim nRight As Long = UBound(arr)
    If nRight - nLeft + 1 <= 1 Then Exit Sub
    
    ' 显式栈：每个元素存(left, right)对
    ' 最大深度约 log2(n)，对于10万数据只需约17层
    Const MAX_STACK As Long = 64  ' 足够处理 2^64 规模的数据
    Dim stackL(0 To MAX_STACK - 1) As Long
    Dim stackR(0 To MAX_STACK - 1) As Long
    Dim sp As Long = 0  ' 栈指针
    
    ' 初始区间入栈
    stackL(sp) = nLeft
    stackR(sp) = nRight
    sp += 1
    
    While sp > 0
        sp -= 1
        Dim l As Long = stackL(sp)
        Dim r As Long = stackR(sp)
        
        ' 小区间直接用插入排序
        If r - l + 1 <= 16 Then
            For i As Long = l + 1 To r
                Dim key As Long = arr(i)
                Dim j As Long = i - 1
                While j >= l
                    Dim cmp As Long = CompareRow(ctx, arr(j), key, sortCol, isNumber)
                    If (isAsc AndAlso cmp > 0) OrElse (Not isAsc AndAlso cmp < 0) Then
                        arr(j + 1) = arr(j)
                        j -= 1
                    Else
                        Exit While
                    End If
                Wend
                arr(j + 1) = key
            Next
            Continue While
        End If
        
        ' 三数取中 + 分区（和之前一样）
        ' 如果某列有大量相同值（如状态列全是"运行中"），标准快排会退化。可改用三路划分：
        ' [l, lt) < pivot, [lt, gt] == pivot, (gt, r] > pivot
        Dim nMid As Long = l + (r - l) \ 2
        Dim pivotIdx As Long = GetMedian(arr(), l, nMid, r, ctx, sortCol, isNumber, isAsc)
        Swap arr(pivotIdx), arr(r)
        Dim pivotVal As Long = arr(r)
        
        Dim i As Long = l - 1
        For j As Long = l To r - 1
            Dim cmp As Long = CompareRow(ctx, arr(j), pivotVal, sortCol, isNumber)
            If (isAsc AndAlso cmp <= 0) OrElse (Not isAsc AndAlso cmp >= 0) Then
                i += 1
                Swap arr(i), arr(j)
            End If
        Next
        Swap arr(i + 1), arr(r)
        Dim p As Long = i + 1
        
        ' 优先将长区间入栈，短区间直接处理（减少栈深度）
        If p - 1 > l Then
            If r > p + 1 Then
                ' 两边都有元素，长的先入栈
                If (p - 1 - l) > (r - p - 1) Then
                    stackL(sp) = l : stackR(sp) = p - 1 : sp += 1
                    stackL(sp) = p + 1 : stackR(sp) = r : sp += 1
                Else
                    stackL(sp) = p + 1 : stackR(sp) = r : sp += 1
                    stackL(sp) = l : stackR(sp) = p - 1 : sp += 1
                End If
            Else
                stackL(sp) = l : stackR(sp) = p - 1 : sp += 1
            End If
        ElseIf r > p + 1 Then
            stackL(sp) = p + 1 : stackR(sp) = r : sp += 1
        End If
    Wend
End Sub

Private Function IsValidNumberW(pwStr As WString Ptr, ByRef isHex As Boolean = False) As Boolean
    If pwStr = NULL OrElse *pwStr = WStr("") Then Return False
    
    Dim p As WString Ptr = pwStr
    Dim hasDot As Boolean = False
    Dim hasDigit As Boolean = False
    isHex = False  ' 默认非十六进制
    
    ' 跳过开头空白
    While *p <> 0 AndAlso (*p = 32 OrElse *p = 9)
        p += 1
    Wend
    If *p = 0 Then Return False
    
    ' 处理正负号
    If *p = 43 OrElse *p = 45 Then
        p += 1
    End If
    
    ' === 新增：检测 0x / 0X 前缀 ===
    If *p = 48 Then  ' 字符 '0'
        Dim nextChar As UShort = *(p + 1)
        If nextChar = 120 OrElse nextChar = 88 Then  ' 'x' 或 'X'
            isHex = True
            p += 2  ' 跳过 "0x"
            ' 十六进制必须至少有一个有效字符
            If *p = 0 Then Return False
            ' 十六进制不允许小数点
            hasDot = True  ' 借用标记，防止后续进入小数点逻辑
        End If
    End If
    
    ' 逐字符校验
    While *p <> 0
        Dim c As UShort = *p
        
        If isHex Then
            ' === 十六进制模式：0-9, a-f, A-F ===
            If (c >= 48 AndAlso c <= 57) OrElse _      ' 0-9
               (c >= 65 AndAlso c <= 70) OrElse _       ' A-F
               (c >= 97 AndAlso c <= 102) Then           ' a-f
                hasDigit = True
            ElseIf c = 32 OrElse c = 9 Then
                Exit While  ' 尾部空白
            Else
                Return False
            End If
            
        Else
            ' === 十进制模式（原有逻辑）===
            If c = 46 Then 
                If hasDot Then Return False
                hasDot = True
            ElseIf c >= 48 AndAlso c <= 57 Then 
                hasDigit = True
            ElseIf c = 32 OrElse c = 9 Then 
                Exit While
            Else
                Return False
            End If
        End If
        
        p += 1
    Wend
    
    Return hasDigit
End Function

Private Sub CacheColumnTypes(ctx As ListViewContext Ptr)
    If ctx = NULL OrElse ctx->Snap = NULL OrElse ctx->Snap->RowCount = 0 Then Exit Sub
    
    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    Dim firstRow As LVRow Ptr = snap->Rows[0]
    If firstRow = NULL Then Exit Sub
    
    Dim colCount As Long = firstRow->ColCount
    ReDim ctx->ColumnIsNumber(0 To colCount - 1)
    
    ' 初始默认所有列都是数字列，发现非数字再标记为False
    For col As Long = 0 To colCount - 1
        ctx->ColumnIsNumber(col) = True
    Next
    
    ' 遍历所有行校验（超大数据量可改为前100行：i < Min(snap->RowCount, 100)）
    For i As Long = 0 To snap->RowCount - 1
        Dim row As LVRow Ptr = snap->Rows[i]
        If row = NULL Then Continue For
        
        For col As Long = 0 To colCount - 1
            ' 已经判定为文本列的，跳过校验提升性能
            If ctx->ColumnIsNumber(col) = False Then Continue For
            
            Dim cell As WString Ptr = row->Cols[col]
            ' 空单元格不影响列类型判断，直接跳过
            If cell = NULL OrElse *cell = WStr("") Then Continue For
            
            ' 只要有一个非空单元格不是数字，整列标记为文本列
            If Not IsValidNumberW(cell) Then
                ctx->ColumnIsNumber(col) = False
                ' 新增：打印第一个触发非数字判定的单元格内容
                'Print "列" & col & "第" & i & "行 非数字内容: [" & *cell & "]"
            End If
        Next
    Next
    ' 打印每一列的最终判定结果
    /'For col As Long = 0 To colCount - 1
        Print "列" & col & "类型:" & IIf(ctx->ColumnIsNumber(col), WStr("数字"), WStr("文本"))
    Next'/
End Sub

' 高性能自适应排序入口：对外唯一接口，自动选择最优算法
Sub SortListViewVisibleIndex(ctx As ListViewContext Ptr)
    If ctx = NULL OrElse ctx->VisibleCount <= 1 Then Exit Sub
    If ctx->SortedColumn < 0 Then Exit Sub
    
    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    Dim sortCol As Long = ctx->SortedColumn
    Dim isAsc As Boolean = ctx->SortAscending
    
    If UBound(ctx->ColumnIsNumber) < 0 Then
        CacheColumnTypes(ctx)
    End If
    If sortCol > UBound(ctx->ColumnIsNumber) Then Exit Sub
    Dim isNumber As Boolean = ctx->ColumnIsNumber(sortCol)
    
    ' 改用迭代版快排
    If ctx->VisibleCount <= 32 Then
        InsertionSort(ctx->VisibleIndex(), ctx, sortCol, isNumber, isAsc)
    Else
        ListView_QuickSort_Iterative(ctx->VisibleIndex(), ctx, sortCol, isNumber, isAsc)
    End If
End Sub

Sub InitListViewData(ListView As Class_ListView)
    Dim oldCtx As ListViewContext Ptr = GetCtx(ListView)
    
    ' ==============================================
    ' 🔥 核心修复：不销毁旧上下文，复用保留配置
    ' ==============================================
    If oldCtx = NULL Then
        ' 【首次初始化】：全新分配上下文
        Dim ctx As ListViewContext Ptr = New ListViewContext'CAllocate(1, SizeOf(ListViewContext))
        ctx->hLV = ListView.hWnd

        Static g_InstanceId As ULong = 0
        g_InstanceId += 1
        ctx->InstanceId = g_InstanceId
        ctx->IsValid = True

        ' 初始化快照
        ctx->Snap = CAllocate(1, SizeOf(ListViewSnapshot))
        ctx->Snap->RowCount = 0
        ctx->Snap->Rows = NULL

        ' 初始化可见项
        ReDim ctx->VisibleIndex(-1)
        ctx->VisibleCount = 0

        ' 初始化默认配置
        ctx->Filter = NULL
        ctx->SortedColumn = -1
        ctx->SortAscending = True
        ReDim ctx->ColumnIsNumber(-1)

        ' 绑定上下文到窗口
        SetWindowLongPtr(ListView.hWnd, GWLP_USERDATA, Cast(LONG_PTR, ctx))
    Else
        ' 【非首次初始化】：复用旧上下文，仅重置数据，保留所有配置（Filter/排序/列类型）
        oldCtx->IsValid = True ' 标记有效

        ' ==============================================
        ' 安全释放旧快照内存（避免内存泄漏）
        ' ==============================================
        If oldCtx->Snap <> NULL Then
            ' 释放所有行数据
            For i As Long = 0 To oldCtx->Snap->RowCount - 1
                Dim row As LVRow Ptr = oldCtx->Snap->Rows[i]
                If row <> NULL Then
                    ' 释放列文本内存
                    If row->Cols <> NULL Then Deallocate row->Cols
                    Deallocate row
                End If
            Next
            ' 释放行指针数组
            If oldCtx->Snap->Rows <> NULL Then Deallocate oldCtx->Snap->Rows
            ' 重置快照
            oldCtx->Snap->RowCount = 0
            oldCtx->Snap->Rows = NULL
        End If

        ' 重置可见项（保留筛选条件 Filter）
        ReDim oldCtx->VisibleIndex(-1)
        oldCtx->VisibleCount = 0
    End If

    ' 列表通用初始化（与配置无关）
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
    If ctx = NULL Or ctx->Snap = NULL Then Return -1
    
    Dim row As LVRow Ptr = Callocate(1, SizeOf(LVRow))
    If row = NULL Then Return -1
    
    row->ColCount = 1
    row->Fore = Fore
    row->Back = Back
    row->iconIndex = I_IMAGENONE
    row->Cols = CAllocate(1, SizeOf(WCHAR Ptr))
    If row->Cols = NULL Then
        Deallocate(row)
        Return -1
    End If
    
    row->Cols[0] = DupWStr(StrPtrW(Text0))
    If row->Cols[0] = NULL Then
        Deallocate(row->Cols)
        Deallocate(row)
        Return -1
    End If
    
    Dim idx As Long = ctx->Snap->RowCount
    ctx->Snap->RowCount += 1

    ' 🔥 修复：ReDim Preserve 改为手动重新分配指针数组
    Dim newRows As LVRow Ptr Ptr = Callocate(ctx->Snap->RowCount, SizeOf(LVRow Ptr))
    For i As Long = 0 To idx - 1
        newRows[i] = ctx->Snap->Rows[i]
    Next
    If ctx->Snap->Rows Then Deallocate ctx->Snap->Rows
    ctx->Snap->Rows = newRows
    ctx->Snap->Rows[idx] = row

    Return idx
End Function

Function AddItemColListEx CDecl ( _
    ListView As Class_ListView, _
    colCnt As Long, _
    Fore As COLORREF = FB_Black, _
    Back As COLORREF = FB_White, _
    ... _
) As Long

    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
        
    Dim row As LVRow Ptr = Callocate(1, SizeOf(LVRow))
    row->ColCount = colCnt
    row->Fore = Fore
    row->Back = Back
    row->iconIndex = I_IMAGENONE
    row->Cols = Callocate(colCnt, SizeOf(WCHAR Ptr))

    Dim ap As Any Ptr
    ap = va_first()

    For i As Long = 0 To colCnt - 1
        Dim src As WCHAR Ptr = va_arg(ap, WCHAR Ptr)
        ap = va_next(ap, WCHAR Ptr)
        ' 空指针/坏指针保护：至少能读 1 个 WCHAR，否则放空串，避免 lstrlenW 崩溃
        If src = NULL OrElse IsBadReadPtr(src, 2) Then
            row->Cols[i] = CAllocate(1, SizeOf(WCHAR))
        Else
            row->Cols[i] = DupWStr(src)
        End If
    Next

    Dim idx As Long = ctx->Snap->RowCount
    ctx->Snap->RowCount += 1

    ' 🔥 修复：ReDim Preserve 改为手动分配
    Dim newRows As LVRow Ptr Ptr = Callocate(ctx->Snap->RowCount, SizeOf(LVRow Ptr))
    For i As Long = 0 To idx - 1
        newRows[i] = ctx->Snap->Rows[i]
    Next
    If ctx->Snap->Rows Then Deallocate ctx->Snap->Rows
    ctx->Snap->Rows = newRows
    ctx->Snap->Rows[idx] = row
    
    'Cache_AddRow CurrentInformation.intType, row ' 这里需要防止因为缓存未初始化导致崩溃的问题

    Return idx
End Function

Sub SetItemTextEx(ListView As Class_ListView, Row As Long, Col As Long, Text As StringW)
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)
        
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

    If Row < 0 Or Row >= ctx->Snap->RowCount Then Exit Sub
    Dim r As LVRow Ptr = ctx->Snap->Rows[Row]
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

Sub SetItemIconEx(ListView As Class_ListView, Row As Long, iconIndex As Long)
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)
        
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

    If Row < 0 Or Row >= ctx->Snap->RowCount Then Exit Sub
    Dim r As LVRow Ptr = ctx->Snap->Rows[Row]
    If r = NULL Then Exit Sub
    
    ' 设置新文本
    r->iconIndex = iconIndex
End Sub

Sub SetListViewFilter(ListView As Class_ListView, s As StringW)
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)
        
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
    'Print "开始筛选"
    If ctx->Filter <> 0 Then
        Deallocate(ctx->Filter)
        ctx->Filter = 0
    End If

    If Len(s) > 0 Then
        ctx->Filter = DupWStr(StrPtrW(s))
    End If
End Sub

Sub SetItemColor(lvw As Class_ListView, Row As Long, Fore As COLORREF, Back As COLORREF)
    Dim ctx As ListViewContext Ptr = GetCtx(lvw)
    If ctx = 0 Then Exit Sub
    If ctx->Snap = 0 Then Exit Sub
    If Row < 0 Or Row >= ctx->Snap->RowCount Then Exit Sub
    Dim r As LVRow Ptr = ctx->Snap->Rows[Row]  ' 🔥 修复指针访问
    If r = 0 Then Exit Sub
    r->Fore = Fore
    r->Back = Back
End Sub

Sub GetItemColor(ListView As Class_ListView, Row As Long, ByRef Fore As COLORREF, ByRef Back As COLORREF)
    Dim r As LVRow Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))->Snap->Rows[Row]  ' 🔥 修复
    Fore = r->Fore
    Back = r->Back
End Sub

Function GetRawItemTextEx(ListView As Class_ListView, RealRow As Long, Col As Long) As String
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)

    If ctx = NULL Then Return ""
    If ctx->Snap = NULL Then Return ""

    If RealRow < 0 Or RealRow >= ctx->Snap->RowCount Then Return ""

    Dim r As LVRow Ptr = ctx->Snap->Rows[RealRow]
    If r = NULL Then Return ""

    If Col < 0 Or Col >= r->ColCount Then Return ""
    If r->Cols = NULL Then Return ""
    If r->Cols[Col] = NULL Then Return ""

    Return *r->Cols[Col]
End Function

Function GetViewItemTextEx(ListView As Class_ListView, ViewRow As Long, Col As Long) As String
    Dim ctx As ListViewContext Ptr = GetCtx(ListView)

    If ctx = NULL Then Return ""
    If ctx->Snap = NULL Then Return ""

    If ViewRow < 0 Or ViewRow >= ctx->VisibleCount Then Return ""

    If UBound(ctx->VisibleIndex) = -1 Then Return ""

    Dim RealRow As Long = ctx->VisibleIndex(ViewRow)

    Return GetRawItemTextEx(ListView, RealRow, Col)
End Function

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
        Dim r As LVRow Ptr = ctx->Snap->Rows[i]
        Dim hit As Boolean = (ctx->Filter = 0)

        If Not hit Then
            For c As Long = 0 To r->ColCount - 1
                If r->Cols[c] <> 0 AndAlso IsBadReadPtr(r->Cols[c], 2) = 0 Then
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
    
    Dim r As LVRow Ptr = ctx->Snap->Rows[real]
    If r = NULL Then Exit Sub
    
    ' 释放行内存
    For i As Long = 0 To r->ColCount - 1
        If r->Cols[i] <> NULL Then
            Deallocate(r->Cols[i])
        End If
    Next
    Deallocate r->Cols
    Deallocate r
    
    ' 前移快照行数组
    For i As Long = real To ctx->Snap->RowCount - 2
        ctx->Snap->Rows[i] = ctx->Snap->Rows[i + 1]
    Next
    ctx->Snap->RowCount -= 1
    
    ' 调整快照行数组大小
    If ctx->Snap->RowCount > 0 Then
        ctx->Snap->Rows = Reallocate(ctx->Snap->Rows, ctx->Snap->RowCount * SizeOf(LVRow Ptr))
    Else
        Deallocate ctx->Snap->Rows
        ctx->Snap->Rows = 0
    End If
    
    ' ========== 新增：同步更新可见索引数组 ==========
    ' 1. 移除当前可见行
    /'For i As Long = VisibleRow To ctx->VisibleCount - 2
        ctx->VisibleIndex(i) = ctx->VisibleIndex(i + 1)
    Next
    ctx->VisibleCount -= 1
    
    ' 2. 所有大于 real 的可见索引全部减 1（因为快照行前移了）
    For i As Long = 0 To ctx->VisibleCount - 1
        If ctx->VisibleIndex(i) > real Then
            ctx->VisibleIndex(i) -= 1
        End If
    Next'/
    
    Cache_DeleteRow(CurrentInformation.intType, real)
    
    If bCommit Then
        'SendMessage(ListView.hWnd, LVM_SETITEMCOUNT, ctx->VisibleCount, 0)
        CommitListViewView ListView
    End If
End Sub

Sub DeleteAllItemsEx(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))
    
    If ctx = NULL Or ctx->Snap = NULL Then Exit Sub
    
    ' 释放所有行数据
    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim r As LVRow Ptr = ctx->Snap->Rows[i]   ' 这里[]保留不动
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
            Deallocate r
        End If
    Next
    
    ' 🔥 修复：二级指针不能用 ReDim → 改为 Deallocate + 置空
    If ctx->Snap->Rows <> 0 Then
        Deallocate ctx->Snap->Rows
        ctx->Snap->Rows = 0
    End If
    ctx->Snap->RowCount = 0
    
    ' 🔥 修复：VisibleIndex 是数组 用()不动，只保留你原有逻辑
    ReDim ctx->VisibleIndex(-1)
    ctx->VisibleCount = 0
    
End Sub

Sub UninitListViewData(ListView As Class_ListView)
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(ListView.hWnd, GWLP_USERDATA))

    If ctx = 0 Then Exit Sub

    If ctx->Snap <> 0 Then
        For i As Long = 0 To ctx->Snap->RowCount - 1
            FreeLVRow(ctx->Snap->Rows[i])
        Next
        
        ctx->IsValid = False   ' 先标记失效！！！
        ctx->Snap = NULL       ' 先清空指针！！！
        Deallocate(ctx->Snap)
    End If

    If ctx->Filter <> 0 Then Deallocate(ctx->Filter)

    SetWindowLongPtr(ListView.hWnd, GWLP_USERDATA, 0)
    'Deallocate(ctx)
    Delete ctx
End Sub

' TreeList

Private Function TreeListGetContext(ByVal hTreeList As HWND) As TreeListContext Ptr
    Return Cast(TreeListContext Ptr, GetWindowLongPtr(hTreeList, GWLP_USERDATA))
End Function

Private Sub TreeListFreeItemMap(ByVal ctx As TreeListContext Ptr)
    If ctx = 0 OrElse ctx->ItemMap = 0 Then Exit Sub
    If ctx->ItemMap->Keys <> 0 Then Deallocate ctx->ItemMap->Keys
    If ctx->ItemMap->Values <> 0 Then Deallocate ctx->ItemMap->Values
    Deallocate ctx->ItemMap
    ctx->ItemMap = 0
End Sub

Private Function TreeListMapCapacity(ByVal count As Long) As Long
    Dim capacity As Long = 16
    While capacity < count * 2
        capacity *= 2
    Wend
    Return capacity
End Function

Private Sub TreeListMapPut(ByVal ctx As TreeListContext Ptr, _
    ByVal hItem As MC_HTREELISTITEM, ByVal rowIndex As Long)

    If ctx = 0 OrElse ctx->ItemMap = 0 OrElse hItem = 0 Then Exit Sub

    Dim key As ULONG_PTR = Cast(ULONG_PTR, hItem)
    Dim slot As Long = Cast(Long, (key Shr 4) And (ctx->ItemMap->Capacity - 1))
    While ctx->ItemMap->Keys[slot] <> 0 AndAlso ctx->ItemMap->Keys[slot] <> key
        slot = (slot + 1) And (ctx->ItemMap->Capacity - 1)
    Wend

    ctx->ItemMap->Keys[slot] = key
    ctx->ItemMap->Values[slot] = rowIndex
End Sub

Private Function TreeListMapFind(ByVal ctx As TreeListContext Ptr, _
    ByVal hItem As MC_HTREELISTITEM) As Long

    If ctx = 0 OrElse ctx->ItemMap = 0 OrElse hItem = 0 Then Return -1

    Dim key As ULONG_PTR = Cast(ULONG_PTR, hItem)
    Dim slot As Long = Cast(Long, (key Shr 4) And (ctx->ItemMap->Capacity - 1))
    Dim firstSlot As Long = slot
    While ctx->ItemMap->Keys[slot] <> 0
        If ctx->ItemMap->Keys[slot] = key Then Return ctx->ItemMap->Values[slot]
        slot = (slot + 1) And (ctx->ItemMap->Capacity - 1)
        If slot = firstSlot Then Exit While
    Wend

    Return -1
End Function

Private Sub TreeListRebuildItemMap(ByVal ctx As TreeListContext Ptr)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    TreeListFreeItemMap(ctx)

    Dim itemCount As Long = 0
    For i As Long = 0 To ctx->Snap->RowCount - 1
        If ctx->Snap->Rows[i] <> 0 AndAlso ctx->Snap->Rows[i]->hItem <> 0 Then
            itemCount += 1
        End If
    Next
    If itemCount = 0 Then Exit Sub

    ctx->ItemMap = Callocate(1, SizeOf(TreeListItemMap))
    If ctx->ItemMap = 0 Then Exit Sub

    ctx->ItemMap->Capacity = TreeListMapCapacity(itemCount)
    ctx->ItemMap->Keys = Callocate(ctx->ItemMap->Capacity, SizeOf(ULONG_PTR))
    ctx->ItemMap->Values = Callocate(ctx->ItemMap->Capacity, SizeOf(Long))
    If ctx->ItemMap->Keys = 0 OrElse ctx->ItemMap->Values = 0 Then
        TreeListFreeItemMap(ctx)
        Exit Sub
    End If

    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row <> 0 AndAlso row->hItem <> 0 Then TreeListMapPut(ctx, row->hItem, i)
    Next
End Sub

Private Function TreeListContainsIgnoreCase(ByVal source As WCHAR Ptr, _
    ByVal text As WCHAR Ptr) As Boolean

    If text = 0 OrElse *text = 0 Then Return True
    If source = 0 Then Return False

    Dim sourceLength As Long = lstrlenW(source)
    Dim textLength As Long = lstrlenW(text)
    If sourceLength < textLength Then Return False

    For i As Long = 0 To sourceLength - textLength
        Dim pCurrent As WCHAR Ptr = Cast(WCHAR Ptr, _
            Cast(ULONG_PTR, source) + i * SizeOf(WCHAR))
        If CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, _
            pCurrent, textLength, text, textLength) = 2 Then
            Return True
        End If
    Next

    Return False
End Function

Private Sub TreeListUpdateMatchesAndVisibility(ByVal ctx As TreeListContext Ptr)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    ctx->Snap->VisibleCount = 0
    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row = 0 Then Continue For

        row->Match = (ctx->Filter = 0)
        If Not row->Match Then
            For columnIndex As Long = 0 To row->ColCount - 1
                If row->Cols[columnIndex] <> 0 AndAlso _
                   TreeListContainsIgnoreCase(row->Cols[columnIndex], ctx->Filter) Then
                    row->Match = True
                    Exit For
                End If
            Next
        End If
        row->Visible = row->Match
    Next

    ' DFS 快照是父节点在子节点之前。倒序传播一次即可把可见状态传到祖先，
    ' 不需要旧实现的 N^2 子节点扫描。
    For i As Long = ctx->Snap->RowCount - 1 To 0 Step -1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row = 0 OrElse Not row->Visible Then Continue For
        If row->ParentIndex >= 0 AndAlso row->ParentIndex < ctx->Snap->RowCount Then
            Dim parentRow As TLRow Ptr = ctx->Snap->Rows[row->ParentIndex]
            If parentRow <> 0 Then parentRow->Visible = True
        End If
    Next

    For i As Long = 0 To ctx->Snap->RowCount - 1
        If ctx->Snap->Rows[i] <> 0 AndAlso ctx->Snap->Rows[i]->Visible Then
            ctx->Snap->VisibleCount += 1
        End If
    Next
End Sub

Sub TreeListShowAllRows(ByVal ctx As TreeListContext Ptr)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub
    ctx->Snap->VisibleCount = 0
    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row <> 0 Then
            row->Match = True
            row->Visible = True
            ctx->Snap->VisibleCount += 1
        End If
    Next
End Sub

Sub TreeListCaptureViewState(ByVal tl As Class_mCtrlTreeList, _
    ByVal ctx As TreeListContext Ptr)

    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    If Not ctx->IsFiltering Then
        For i As Long = 0 To ctx->Snap->RowCount - 1
            Dim row As TLRow Ptr = ctx->Snap->Rows[i]
            If row <> 0 AndAlso row->hItem <> 0 Then
                row->Expanded = IsItemExpanded(tl, row->hItem)
            End If
        Next

        ctx->SavedSelectedIndex = TreeListMapFind(ctx, tl.Selection)
        Dim hTop As MC_HTREELISTITEM = tl.GetNextItem(NULL, MC_TLGN_FIRSTVISIBLE)
        ctx->SavedTopVisibleIndex = TreeListMapFind(ctx, hTop)
        ctx->SavedVisibleCount = tl.GetVisibleCount()
    End If
End Sub

Private Sub TreeListRestorePosition(ByVal tl As Class_mCtrlTreeList, _
    ByVal ctx As TreeListContext Ptr, ByVal selectedIndex As Long, _
    ByVal topIndex As Long, ByVal visibleRows As Long)

    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    If topIndex >= 0 AndAlso topIndex < ctx->Snap->RowCount Then
        Dim topRow As TLRow Ptr = ctx->Snap->Rows[topIndex]
        If topRow <> 0 AndAlso topRow->hItem <> 0 Then
            Dim scrollItem As MC_HTREELISTITEM = topRow->hItem
            Dim remainingRows As Long = visibleRows - 1
            While remainingRows > 0 AndAlso scrollItem <> 0
                Dim nextVisible As MC_HTREELISTITEM = _
                    tl.GetNextItem(scrollItem, MC_TLGN_NEXTVISIBLE)
                If nextVisible = 0 Then Exit While
                scrollItem = nextVisible
                remainingRows -= 1
            Wend
            If scrollItem <> 0 Then tl.EnsureVisible(scrollItem)
        End If
    End If

    If selectedIndex >= 0 AndAlso selectedIndex < ctx->Snap->RowCount Then
        Dim selectedRow As TLRow Ptr = ctx->Snap->Rows[selectedIndex]
        If selectedRow <> 0 AndAlso selectedRow->hItem <> 0 Then
            tl.Selection = selectedRow->hItem
            tl.EnsureVisible(selectedRow->hItem)
        End If
    End If
End Sub

Sub TreeListRebuild(ByVal tl As Class_mCtrlTreeList, _
    ByVal ctx As TreeListContext Ptr, ByVal expandAll As Boolean, _
    ByVal selectedIndex As Long, ByVal topIndex As Long, ByVal visibleRows As Long)

    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    tl.ReDraw = False
    tl.DeleteItem(MC_TLI_ROOT)
    TreeListFreeItemMap(ctx)

    For i As Long = 0 To ctx->Snap->RowCount - 1
        If ctx->Snap->Rows[i] <> 0 Then ctx->Snap->Rows[i]->hItem = 0
    Next

    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row = 0 OrElse Not row->Visible Then Continue For

        Dim hParent As MC_HTREELISTITEM = MC_TLI_ROOT
        If row->ParentIndex >= 0 AndAlso row->ParentIndex < ctx->Snap->RowCount Then
            Dim parentRow As TLRow Ptr = ctx->Snap->Rows[row->ParentIndex]
            If parentRow = 0 OrElse parentRow->hItem = 0 Then Continue For
            hParent = parentRow->hItem
        End If

        Dim itemText As StringW = ""
        If row->Cols <> 0 AndAlso row->Cols[0] <> 0 Then itemText = *row->Cols[0]
        row->hItem = tl.AddItem(hParent, itemText, i)
        If row->hItem = 0 Then Continue For

        For columnIndex As Long = 1 To row->ColCount - 1
            If row->Cols[columnIndex] <> 0 Then
                tl.SetItemText(row->hItem, columnIndex, *row->Cols[columnIndex])
            End If
        Next
    Next

    For i As Long = 0 To ctx->Snap->RowCount - 1
        Dim row As TLRow Ptr = ctx->Snap->Rows[i]
        If row = 0 OrElse row->hItem = 0 Then Continue For
        If expandAll OrElse row->Expanded Then tl.Expand(row->hItem, MC_TLE_EXPAND, 0)
    Next

    TreeListRebuildItemMap(ctx)
    tl.ReDraw = True
    InvalidateRect(tl.hWnd, NULL, True)
    TreeListRestorePosition(tl, ctx, selectedIndex, topIndex, visibleRows)
End Sub

Private Function TreeListCreateSnapshot(ByVal tl As Class_mCtrlTreeList) As TreeListSnapshot Ptr
    Dim totalCount As Long = 0
    Dim currentItem As MC_HTREELISTITEM = tl.GetRoot()
    While currentItem <> 0
        totalCount += 1
        Dim childItem As MC_HTREELISTITEM = tl.GetNextItem(currentItem, MC_TLGN_CHILD)
        If childItem <> 0 Then
            currentItem = childItem
            Continue While
        End If
        Dim siblingItem As MC_HTREELISTITEM = tl.GetNextItem(currentItem, MC_TLGN_NEXT)
        If siblingItem <> 0 Then
            currentItem = siblingItem
            Continue While
        End If
        Do While currentItem <> 0
            siblingItem = tl.GetNextItem(currentItem, MC_TLGN_NEXT)
            If siblingItem <> 0 Then
                currentItem = siblingItem
                Exit Do
            End If
            currentItem = tl.GetNextItem(currentItem, MC_TLGN_PARENT)
        Loop
    Wend
    If totalCount = 0 Then Return 0

    Dim snap As TreeListSnapshot Ptr = Callocate(1, SizeOf(TreeListSnapshot))
    If snap = 0 Then Return 0
    snap->RowCount = totalCount
    snap->Rows = Callocate(totalCount, SizeOf(TLRow Ptr))
    If snap->Rows = 0 Then
        FreeTreeListSnapshot(snap)
        Return 0
    End If

    Dim parentStack As Long Ptr = Callocate(totalCount, SizeOf(Long))
    If parentStack = 0 Then
        FreeTreeListSnapshot(snap)
        Return 0
    End If

    Dim columnCount As Long = tl.GetColumnCount()
    If columnCount <= 0 Then columnCount = 1
    Dim index As Long = 0
    Dim depth As Long = 0
    currentItem = tl.GetRoot()

    While currentItem <> 0 AndAlso index < totalCount
        Dim row As TLRow Ptr = Callocate(1, SizeOf(TLRow))
        If row = 0 Then Exit While
        row->ColCount = columnCount
        row->Cols = Callocate(columnCount, SizeOf(WString Ptr))
        If row->Cols = 0 Then
            Deallocate row
            Exit While
        End If

        For columnIndex As Long = 0 To columnCount - 1
            Dim itemText As StringW = tl.GetItemText(currentItem, columnIndex)
            row->Cols[columnIndex] = DupWStr(StrPtrW(itemText))
        Next

        row->Depth = depth
        If depth > 0 Then
            row->ParentIndex = parentStack[depth - 1]
        Else
            row->ParentIndex = -1
        End If
        parentStack[depth] = index
        row->cChildren = IIf(tl.GetChildCount(currentItem) > 0, 1, 0)
        row->Expanded = IsItemExpanded(tl, currentItem)
        row->Match = True
        row->Visible = True
        row->hItem = currentItem
        snap->Rows[index] = row
        index += 1

        Dim childItem As MC_HTREELISTITEM = tl.GetNextItem(currentItem, MC_TLGN_CHILD)
        If childItem <> 0 Then
            depth += 1
            currentItem = childItem
            Continue While
        End If

        Dim siblingItem As MC_HTREELISTITEM = tl.GetNextItem(currentItem, MC_TLGN_NEXT)
        If siblingItem <> 0 Then
            currentItem = siblingItem
            Continue While
        End If

        Do
            currentItem = tl.GetNextItem(currentItem, MC_TLGN_PARENT)
            If currentItem = 0 Then
                depth = 0
                Exit Do
            End If
            depth -= 1
            siblingItem = tl.GetNextItem(currentItem, MC_TLGN_NEXT)
            If siblingItem <> 0 Then
                currentItem = siblingItem
                Exit Do
            End If
        Loop
    Wend

    Deallocate parentStack
    If index <> totalCount Then
        FreeTreeListSnapshot(snap)
        Return 0
    End If

    snap->VisibleCount = totalCount
    Return snap
End Function

Sub UninitTreeListData(ByVal tl As Class_mCtrlTreeList)
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    If ctx = 0 Then Exit Sub

    TreeListFreeItemMap(ctx)
    If ctx->Filter <> 0 Then Deallocate ctx->Filter
    SetWindowLongPtr(tl.hWnd, GWLP_USERDATA, 0)
    Delete ctx
End Sub

Sub BindTreeListData(ByVal tl As Class_mCtrlTreeList, ByVal snap As TreeListSnapshot Ptr)
    UninitTreeListData(tl)
    If snap = 0 Then Exit Sub

    Dim ctx As TreeListContext Ptr = New TreeListContext
    If ctx = 0 Then Exit Sub

    ctx->hTreeList = tl.hWnd
    ctx->Snap = snap
    ctx->IsValid = True
    ctx->SavedSelectedIndex = -1
    ctx->SavedTopVisibleIndex = -1
    ctx->SavedVisibleCount = 0
    SetWindowLongPtr(tl.hWnd, GWLP_USERDATA, Cast(LONG_PTR, ctx))
    TreeListRebuildItemMap(ctx)
End Sub

Sub CaptureTreeListData(ByVal tl As Class_mCtrlTreeList, ByVal moduleType As Interface)
    Dim snap As TreeListSnapshot Ptr = TreeListCreateSnapshot(tl)
    UninitTreeListData(tl)

    Dim cache As ViewModuleCache Ptr = @g_ViewCache(moduleType)
    If cache->pTreeListSnap <> 0 Then FreeTreeListSnapshot(cache->pTreeListSnap)
    cache->pTreeListSnap = snap
    cache->IsCached = IIf(snap <> 0, True, False)
    cache->ItemCount = IIf(snap <> 0, snap->RowCount, 0)
    cache->TreeList_SelectedIndex = -1
    cache->TreeList_TopVisibleIndex = -1
    cache->TreeList_SavedVisibleCount = tl.GetVisibleCount()
    If snap = 0 Then Exit Sub

    BindTreeListData(tl, snap)
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    TreeListCaptureViewState(tl, ctx)
    cache->TreeList_SelectedIndex = ctx->SavedSelectedIndex
    cache->TreeList_TopVisibleIndex = ctx->SavedTopVisibleIndex
    cache->TreeList_SavedVisibleCount = ctx->SavedVisibleCount
End Sub

Function GetTreeListVisibleItemCount(ByVal tl As Class_mCtrlTreeList) As Long
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    If ctx = 0 OrElse ctx->Snap = 0 Then Return 0
    Return ctx->Snap->VisibleCount
End Function

Function TreeListCustomDraw(ByVal lParam As LPARAM) As LRESULT
    Dim cd As MC_NMTLCUSTOMDRAW Ptr = Cast(MC_NMTLCUSTOMDRAW Ptr, lParam)
    If cd = 0 Then Return CDRF_DODEFAULT

    Dim ctx As TreeListContext Ptr = TreeListGetContext(cd->nmcd.hdr.hwndFrom)
    If ctx = 0 OrElse ctx->Snap = 0 Then Return CDRF_DODEFAULT

    Select Case cd->nmcd.dwDrawStage
        Case CDDS_PREPAINT
            Return CDRF_NOTIFYITEMDRAW
        Case CDDS_ITEMPREPAINT
            Return CDRF_NOTIFYSUBITEMDRAW
        Case CDDS_ITEMPREPAINT Or CDDS_SUBITEM
            Dim hItem As MC_HTREELISTITEM = Cast(MC_HTREELISTITEM, cd->nmcd.dwItemSpec)
            Dim rowIndex As Long = TreeListMapFind(ctx, hItem)
            If rowIndex < 0 Then rowIndex = Cast(Long, cd->nmcd.lItemlParam)
            If rowIndex < 0 OrElse rowIndex >= ctx->Snap->RowCount Then Return CDRF_DODEFAULT
            Dim row As TLRow Ptr = ctx->Snap->Rows[rowIndex]
            If row = 0 OrElse Not row->HasCustomColor Then Return CDRF_DODEFAULT
            cd->clrText = row->Fore
            cd->clrTextBk = row->Back
    End Select

    Return CDRF_DODEFAULT
End Function

Sub InitTreeListData( _
    tl As Class_mCtrlTreeList, _
    snap As TreeListSnapshot Ptr)
    BindTreeListData(tl, snap)
End Sub

Sub BuildTreeListItems( _
    tl As Class_mCtrlTreeList, _
    snap As TreeListSnapshot Ptr)
    BindTreeListData(tl, snap)
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    TreeListShowAllRows(ctx)
    TreeListRebuild(tl, ctx, False, -1, -1, 0)
End Sub

Sub CommitTreeListView( _
    tl As Class_mCtrlTreeList)
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    TreeListUpdateMatchesAndVisibility(ctx)
    TreeListRebuild(tl, ctx, ctx->Filter <> 0, -1, -1, 0)
End Sub

Sub UpdateTreeMatch( _
    ctx As TreeListContext Ptr)


    For i As Long=0 To _
        ctx->Snap->RowCount-1


        Dim row As TLRow Ptr

        row=ctx->Snap->Rows[i]


        row->Match=False



        If ctx->Filter=NULL Then

            row->Match=True


        Else


            For c As Long=0 To row->ColCount-1


                If row->Cols[c]<>NULL Then


                    If wcsstr( _
                        row->Cols[c], _
                        ctx->Filter)<>NULL Then


                        row->Match=True

                        Exit For

                    End If

                End If


            Next


        End If


    Next


End Sub

Function UpdateTreeVisibleNode( _
    ctx As TreeListContext Ptr, _
    index As Long) As Boolean



    Dim row As TLRow Ptr

    row=ctx->Snap->Rows[index]



    Dim visible As Boolean

    visible=row->Match



    For i As Long=0 To _
        ctx->Snap->RowCount-1



        If ctx->Snap->Rows[i]->ParentIndex=index Then


            If UpdateTreeVisibleNode(ctx,i) Then

                visible=True

            End If


        End If


    Next



    row->Visible=visible


    Return visible


End Function

Sub UpdateTreeVisible( _
    ctx As TreeListContext Ptr)


    For i As Long=0 To _
        ctx->Snap->RowCount-1


        If ctx->Snap->Rows[i]->ParentIndex=-1 Then

            UpdateTreeVisibleNode(ctx,i)

        End If


    Next


End Sub

Sub SetTreeListFilter( _
    tl As Class_mCtrlTreeList, _
    text As WString)
    Dim ctx As TreeListContext Ptr = TreeListGetContext(tl.hWnd)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    If Len(text) = 0 AndAlso ctx->Filter = 0 AndAlso (Not ctx->IsFiltering) Then Exit Sub

    Dim currentSelection As Long = TreeListMapFind(ctx, tl.Selection)
    If Len(text) > 0 AndAlso Not ctx->IsFiltering Then
        TreeListCaptureViewState(tl, ctx)
        ctx->IsFiltering = True
    End If

    If ctx->Filter <> 0 Then
        Deallocate ctx->Filter
        ctx->Filter = 0
    End If
    If Len(text) > 0 Then ctx->Filter = DupWStr(StrPtrW(text))

    If ctx->Filter = 0 Then
        ctx->IsFiltering = False
        TreeListShowAllRows(ctx)
        TreeListRebuild(tl, ctx, False, ctx->SavedSelectedIndex, _
            ctx->SavedTopVisibleIndex, ctx->SavedVisibleCount)
    Else
        TreeListUpdateMatchesAndVisibility(ctx)
        If currentSelection < 0 OrElse currentSelection >= ctx->Snap->RowCount Then
            currentSelection = -1
        ElseIf ctx->Snap->Rows[currentSelection] = 0 OrElse _
               Not ctx->Snap->Rows[currentSelection]->Visible Then
            currentSelection = -1
        End If
        TreeListRebuild(tl, ctx, True, currentSelection, _
            ctx->SavedTopVisibleIndex, ctx->SavedVisibleCount)
    End If
End Sub

Sub SetTreeListItemColor( _
    tl As Class_mCtrlTreeList, _
    hItem As MC_HTREELISTITEM, _
    fore As COLORREF, _
    back As COLORREF)


    Dim ctx As TreeListContext Ptr

    ctx=TreeListGetContext(tl.hWnd)
    If ctx = 0 OrElse ctx->Snap = 0 Then Exit Sub

    Dim rowIndex As Long = TreeListMapFind(ctx, hItem)
    If rowIndex < 0 OrElse rowIndex >= ctx->Snap->RowCount Then Exit Sub
    Dim row As TLRow Ptr = ctx->Snap->Rows[rowIndex]
    If row = 0 Then Exit Sub
    row->Fore = fore
    row->Back = back
    row->HasCustomColor = True
    InvalidateRect(tl.hWnd, NULL, True)
End Sub

Function TreeListGetDispInfo( _
    lParam As LPARAM) As LRESULT


    Dim p As MC_NMTLDISPINFOW Ptr

    p=Cast(MC_NMTLDISPINFOW Ptr,lParam)
    If p = 0 Then Return 0



    Dim ctx As TreeListContext Ptr

    ctx=Cast(TreeListContext Ptr, _
        GetWindowLongPtr( _
        p->hdr.hwndFrom, _
        GWLP_USERDATA))


    If ctx=NULL Then Return 0



    Dim row As TLRow Ptr

    Dim rowIndex As Long = TreeListMapFind(ctx, p->hItem)
    If rowIndex < 0 OrElse rowIndex >= ctx->Snap->RowCount Then Return 0
    row=ctx->Snap->Rows[rowIndex]



    If row=NULL Then Return 0



    '这里根据MC_TLITEMW定义填写

    p->item.pszText=row->Cols[0]



    Return 0


End Function

Function FindTLRow( _
    hItem As MC_HTREELISTITEM) As TLRow Ptr


    Dim ctx As TreeListContext Ptr = 0
    If ctx = 0 Then Return NULL


    For i As Long=0 To ctx->Snap->RowCount-1

        If ctx->Snap->Rows[i]->hItem=hItem Then

            Return ctx->Snap->Rows[i]

        End If

    Next


    Return NULL

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
    'Print WinErrorMsg(GetLastError) ' 这里GetLastError会是"句柄无效",但我们不用管它,反正已经拿到了pBuffer
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
        MyLog.PrintWin32Error "SetMenuText", "SetMenuItemInfo", "Menu=" & Menu.Text(iditem)
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
    'Print "[TreeList_SubItemHitTest]iColumnIndex:" & iColumnIndex
    iSubItem = iColumnIndex
End Sub

' 模拟TreeView指定节点的双击事件
Sub TreeView_SimulateDblClick(ByVal hTree As HWND, ByVal hItem As HTREEITEM)
    TreeView_SelectItem(hTree, hItem)
    TreeView_EnsureVisible(hTree, hItem)
    
    Dim rc As RECT
    TreeView_GetItemRect(hTree, hItem, @rc, True)
    
    Dim xPos As Long = (rc.Left + rc.Right) \ 2
    Dim yPos As Long = (rc.top + rc.bottom) \ 2
    
    ' 发送自定义消息，让消息循环统一处理
    SendMessage(GetParent(hTree), WM_TREEVIEW_DBLCLK, Cast(wParam, hItem), MAKELPARAM(xPos, yPos))
End Sub

' ===================== 递归查找节点 =====================

' 在指定父节点下递归查找匹配文本的第一个子节点
Private Function GetNodeByTextRecursive( _
    ByVal hTree As HWND, _
    ByVal hParent As HTREEITEM, _
    ByVal searchText As StringW, _
    ByVal bExactMatch As Boolean, _
    ByVal bCaseSensitive As Boolean _
) As HTREEITEM
    
    Dim hChild As HTREEITEM = TreeView_GetChild(hTree, hParent)
    If hChild = NULL Then Return NULL
    
    Dim buffer As WString * 512
    Dim tvi As TVITEMW
    
    Do While hChild <> NULL
        ' 获取节点文本
        ZeroMemory(@tvi, SizeOf(TVITEMW))
        tvi.mask = TVIF_TEXT
        tvi.hItem = hChild
        tvi.pszText = @buffer
        tvi.cchTextMax = 511
        
        If SendMessage(hTree, TVM_GETITEMW, 0, Cast(LPARAM, @tvi)) <> 0 Then
            ' 比较文本
            Dim isMatch As Boolean
            If bExactMatch Then
                If bCaseSensitive Then isMatch = (lstrcmpW(buffer, StrPtrW(searchText)) = 0) Else isMatch = (lstrcmpiW(buffer, StrPtrW(searchText)) = 0)
            Else
                If bCaseSensitive Then isMatch = (InStrW(buffer, searchText) > 0) Else isMatch = (InStrW(LCaseW(buffer), LCaseW(searchText)) > 0)
            End If
            
            If isMatch Then Return hChild
        End If
        
        ' 递归搜索子节点
        Dim hFound As HTREEITEM = GetNodeByTextRecursive(hTree, hChild, searchText, bExactMatch, bCaseSensitive)
        If hFound <> NULL Then Return hFound
        
        hChild = TreeView_GetNextSibling(hTree, hChild)
    Loop
    
    Return NULL
End Function

' ===================== 公开接口 =====================

' 在 TreeView 中按文本查找节点（从根开始）
' 参数:
'   MyTreeView    - VFB TreeView 控件对象
'   searchText    - 要查找的文本
'   bExactMatch   - True=完全匹配, False=包含匹配 (默认True)
'   bCaseSensitive- True=区分大小写, False=不区分 (默认False)
' 返回:
'   找到的节点句柄，未找到返回NULL
Function GetNodeByText( _
    ByRef MyTreeView As Class_TreeView, _
    ByVal searchText As StringW, _
    ByVal bExactMatch As Boolean = True, _
    ByVal bCaseSensitive As Boolean = False _
) As HTREEITEM
    
    Return GetNodeByTextRecursive(MyTreeView.hWnd, TVI_ROOT, searchText, bExactMatch, bCaseSensitive)
End Function

Function FindProcessInListView Overload(ListView As Class_ListView, ByVal PID As DWORD) As Boolean
    For i As Integer = 0 To ListView.ItemCount - 1
        If ValUInt(GetViewItemTextEx(ListView, i, 0)) = PID Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    FrmMain.lblNum.Caption = "正在刷新..."
    GetProcessList ListView, GetMenuCheckState(FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
    FrmMain.lblNum.Caption = "数量:" & WStr(ListView.ItemCount)
    For i As Integer = 0 To ListView.ItemCount - 1
        If ValUInt(GetViewItemTextEx(ListView, i, 0)) = PID Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    Return False
End Function

Function FindProcessInListView Overload(ListView As Class_ListView, ProcessPath As StringW) As Boolean
    For i As Integer = 0 To ListView.ItemCount - 1
        If UCaseW(GetViewItemTextEx(ListView, i, 4)) = UCaseW(ProcessPath) Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    FrmMain.lblNum.Caption = "正在刷新..."
    GetProcessList ListView, GetMenuCheckState(FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
    FrmMain.lblNum.Caption = "数量:" & WStr(ListView.ItemCount)
    For i As Integer = 0 To ListView.ItemCount - 1
        If UCaseW(GetViewItemTextEx(ListView, i, 4)) = UCaseW(ProcessPath) Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    Return False
End Function

Function FindDriverInListView(ListView As Class_ListView, DriverPath As StringW) As Boolean
    For i As Integer = 0 To ListView.ItemCount - 1
        If UCaseW(GetViewItemTextEx(ListView, i, 4)) = UCaseW(DriverPath) Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    FrmMain.lblNum.Caption = "正在刷新..."
    GetKernelModuleList ListView, GetMenuCheckState(FrmMain.mnuKernelModule, FrmMain_mnuKernelModule_mnuCheckHideDriver)
    FrmMain.lblNum.Caption = "数量:" & WStr(ListView.ItemCount)
    For i As Integer = 0 To ListView.ItemCount - 1
        If UCaseW(GetViewItemTextEx(ListView, i, 4)) = UCaseW(DriverPath) Then
            ListView.SelectedItem = i
            ListView_EnsureVisible(ListView.hWnd, i, False)
            Return True
        End If
    Next
    Return False
End Function
