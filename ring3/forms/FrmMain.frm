#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmMain
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_CAPTION,WS_CLIPCHILDREN,WS_CLIPSIBLINGS,WS_MAXIMIZEBOX,WS_MINIMIZEBOX,WS_SYSMENU,WS_VISIBLE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_EX_WINDOWEDGE,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=SnowSword.ico
Caption=SnowSword
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=False
Left=0
Top=0
Width=754
Height=516
TopMost=False
Child=False
MdiChild=False
TitleBar=True
SizeBox=True
SysMenu=True
MaximizeBox=True
MinimizeBox=True
Help=False
Hscroll=False
Vscroll=False
MinWidth=0
MinHeight=0
MaxWidth=0
MaxHeight=0
NoActivate=False
MousePass=False
TransPer=0
TransColor=SYS,25
Shadow=0 - 无阴影
BackColor=SYS,15
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[PopupMenu]
Name=mnuCallbacks
Help=
Index=-1
Menu=刷新FrmMain_mnuCallbacks_mnuRefresh0-10-FrmMain_mnuCallbacks_Step10-10禁用回调FrmMain_mnuCallbacks_mnuControlCallback0-10移除回调FrmMain_mnuCallbacks_mnuRemoveCallback0-10-FrmMain_mnuCallbacks_mnuStep20-10复制FrmMain_mnuCallbacks_mnuCopy0-10{复制单格数据FrmMain_mnuCallbacks_mnuLittleCopyCtrl+C0-10}定位文件位置(资源浏览器)FrmMain_mnuCallbacks_mnuLocateFilePath0-10
Left=400
Top=280
Tag=

[Button]
Name=Command1
Help=
Index=-1
Caption=电源管理
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=330
Top=160
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False

[Label]
Name=lblNum
Help=
Index=-1
Style=0 - 无边框
Caption=
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
FontWidth=0
FontAngle=0
TextAlign=0 - 左对齐
Prefix=True
Ellipsis=False
Left=640
Top=440
Width=100
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[PopupMenu]
Name=mnuKernelModule
Help=
Index=-1
Menu=刷新FrmMain_mnuKernelModule_mnuRefresh0-10卸载驱动FrmMain_mnuKernelModule_mnuUnloadDriver0-10-FrmMain_mnuKernelModule_Step10-10查看IO派遣函数FrmMain_mnuKernelModule_mnuViewIOFunction0-10-FrmMain_mnuKernelModule_Step20-10dump到文件FrmMain_mnuKernelModule_mnuDumpToFile0-10复制FrmMain_mnuKernelModule_mnuCopy0-10{复制单格数据FrmMain_mnuKernelModule_mnuLittleCopyCtrl+C0-10}定位文件位置(资源浏览器)FrmMain_mnuKernelModule_mnuLocateFilePath0-10
Left=670
Top=180
Tag=

[PopupMenu]
Name=mnuService
Help=
Index=-1
Menu=刷新FrmMain_mnuService_mnuRefreshF50-10-FrmMain_mnuService_Step10-10复制FrmMain_mnuService_mnuCopy0-10{复制单格数据FrmMain_mnuService_mnuLittleCopyCtrl+C0-10}定位文件位置FrmMain_mnuService_mnuLocateFilePath0-10
Left=550
Top=180
Tag=

[TrayIco]
Name=TrayIco1
Help=
Icon=SnowSword.ico
CallMsg=400
Tips=SnowSword - lzy
Left=430
Top=180
Tag=

[TextBox]
Name=TxtFilter2
Help=
Index=-1
Style=3 - 凹边框
TextScrollBars=0 - 无滚动条
Text=
Enabled=True
Visible=True
MaxLength=0
ForeColor=SYS,8
BackColor=SYS,5
Font=微软雅黑,9,0
TextAlign=0 - 左对齐
PasswordChar=
Locked=False
HideSelection=True
Multiline=False
Uppercase=False
Lowercase=False
Number=False
AutoHScroll=True
AutoVScroll=False
Left=200
Top=440
Width=440
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
LeftMargin=0
RightMargin=0
AcceptFiles=False

[CheckBox]
Name=Check9
Help=
Index=-1
Style=0 - 标准
Caption=自我保护(应用层)
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=500
Top=50
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check8
Help=
Index=-1
Style=0 - 标准
Caption=自我保护(内核层)
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=220
Top=160
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check7
Help=
Index=-1
Style=0 - 标准
Caption=禁止加载驱动
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=560
Top=120
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check6
Help=
Index=-1
Style=0 - 标准
Caption=加载驱动
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=400
Top=50
Width=90
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check5
Help=
Index=-1
Style=0 - 标准
Caption=禁止操作注册表
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=440
Top=120
Width=110
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check4
Help=
Index=-1
Style=0 - 标准
Caption=禁止线程创建
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=330
Top=120
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check3
Help=
Index=-1
Style=0 - 标准
Caption=禁止进程创建
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=220
Top=120
Width=100
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Frame]
Name=Frame3
Help=
Index=-1
Caption=Ring0设置
Frame=True
TextAlign=0 - 左对齐
Fillet=5
BorderWidth=1
BorderColor=SYS,16
ForeColor=SYS,8
BackColor=SYS,25
Enabled=True
Visible=True
Font=微软雅黑,9,0
Left=210
Top=100
Width=520
Height=110
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[CheckBox]
Name=Check2
Help=
Index=-1
Style=0 - 标准
Caption=屏蔽消息钩子
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=300
Top=50
Width=90
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check1
Help=
Index=-1
Style=0 - 标准
Caption=禁止注销
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=220
Top=50
Width=70
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check10
Help=
Index=-1
Style=0 - 标准
Caption=窗口置顶
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=610
Top=50
Width=90
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Frame]
Name=Frame2
Help=
Index=-1
Caption=Ring3设置
Frame=True
TextAlign=0 - 左对齐
Fillet=5
BorderWidth=1
BorderColor=SYS,16
ForeColor=SYS,8
BackColor=SYS,25
Enabled=True
Visible=True
Font=微软雅黑,9,0
Left=210
Top=30
Width=520
Height=60
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Line]
Name=Line1
Help=
Index=-1
Style=0 - 顶部
BorderWidth=2
ArrowStartW=0 - 无箭头
ArrowStartH=0 - 无箭头
ArrowEndW=0 - 无箭头
ArrowEndH=0 - 无箭头
BorderColor=SYS,1
Enabled=True
Visible=True
Left=260
Top=20
Width=470
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Label]
Name=Label1
Help=
Index=-1
Style=0 - 无边框
Caption=系统相关
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
FontWidth=0
FontAngle=0
TextAlign=0 - 左对齐
Prefix=True
Ellipsis=False
Left=210
Top=10
Width=50
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtFilePath
Help=
Index=-1
Style=3 - 凹边框
TextScrollBars=0 - 无滚动条
Text=
Enabled=True
Visible=True
MaxLength=0
ForeColor=SYS,8
BackColor=SYS,5
Font=微软雅黑,9,0
TextAlign=0 - 左对齐
PasswordChar=
Locked=False
HideSelection=True
Multiline=False
Uppercase=False
Lowercase=False
Number=False
AutoHScroll=True
AutoVScroll=False
Left=200
Top=0
Width=540
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
LeftMargin=0
RightMargin=0
AcceptFiles=False

[VEH]
Name=VEH1
Help=
Left=240
Top=280
Tag=

[mCtrlTreeList]
Name=mCtrlTreeList1
Help=
Index=-1
Style=1 - 细边框
WindowTheme=True
ImageList=无图像列表控件
HasButton=True
HasLines=True
LinesRoot=True
GridLines=True
ShowSel=True
FullSel=True
NoHeight=False
DBuffer=True
ColHeader=True
HeaderDrop=False
SingleExpand=False
MultiSelect=False
NoTooltips=False
Enabled=True
Visible=True
Left=560
Top=330
Width=90
Height=70
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Frame]
Name=FmeAdvance
Help=
Index=-1
Caption=
Frame=True
TextAlign=0 - 左对齐
Fillet=1
BorderWidth=1
BorderColor=SYS,16
ForeColor=SYS,8
BackColor=SYS,25
Enabled=True
Visible=True
Font=微软雅黑,9,0
Left=200
Top=0
Width=540
Height=460
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[PopupMenu]
Name=mnuFile
Help=
Index=-1
Menu=刷新FrmMain_mnuFile_mnuRefreshF50-10新建FrmMain_mnuFile_mnuCreate0-10{新建文件FrmMain_mnuFile_mnuCreateFile0-10新建文件夹FrmMain_mnuFile_mnuCreateFolder0-10}复制FrmMain_mnuFile_mnuCopy0-10粘贴FrmMain_mnuFile_mnuPaste0-10删除FrmMain_mnuFile_mnuDelete0-10强制删除FrmMain_mnuFile_mnuForceDelete0-10-FrmMain_mnuFile_mnuStep10-10复制文字FrmMain_mnuFile_mnuCopyStr0-10{复制单格数据FrmMain_mnuFile_mnuLittleCopyCtrl+C0-10}
Left=680
Top=90
Tag=

[Timer]
Name=TmrProtect
Help=
Index=-1
Interval=1
Enabled=False
Left=610
Top=60
Tag=

[TopMenu]
Name=TopMenu1
Help=
Menu=文件FrmMain_TopMenu1_mnuFile0-10{查看文件占用FrmMain_TopMenu1_mnuUnlockFile0-10以管理员身份运行FrmMain_TopMenu1_mnuRunasAdmin0-10}网络FrmMain_TopMenu1_mnuWeb0-10{防火墙FrmMain_TopMenu1_mnuFireWall0-10}
Tag=

[TreeView]
Name=treMain
Help=
Index=-1
BStyle=3 - 凹边框
Theme=True
ImageList=无图像列表控件
HasButtons=True
HasLines=True
LinesRoot=True
EditLabels=False
DisableDragDrop=False
ShowSelAlways=False
RTLreading=False
NoToolTips=False
Check=False
TrackSelect=False
SingleExpand=False
InfoTip=False
FullRowSelect=False
NoScroll=False
NoNevenHeight=False
NoHScroll=False
Enabled=True
Visible=True
LineColor=SYS,25
ForeColor=SYS,25
BackColor=SYS,5
Font=微软雅黑,9,0
Left=-1
Top=-2
Width=200
Height=463
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[PopupMenu]
Name=mnuProcess
Help=
Index=-1
Menu=结束进程FrmMain_mnuProcess_mnuTerminateProcess0-10强制结束进程FrmMain_mnuProcess_mnuForceTerminateProcess0-10暴力结束进程FrmMain_mnuProcess_mnuViolentTerminateProcess0-10挂起进程FrmMain_mnuProcess_mnuSuspendProcess0-10恢复进程FrmMain_mnuProcess_mnuResumeProcess0-10保护进程FrmMain_mnuProcess_mnuProtectProcess0-10检测隐藏进程FrmMain_mnuProcess_mnuCheckHideProcess0-10校验数字签名FrmMain_mnuProcess_mnuCheckSign0-10校验所有数字签名FrmMain_mnuProcess_mnuCheckAllSign0-10刷新FrmMain_mnuProcess_mnuRefresh0-10-FrmMain_mnuProcess_mnuStep1000注入DLLFrmMain_mnuProcess_mnuInjectDll0-10-FrmMain_mnuProcess_mnuStep20-10查看模块FrmMain_mnuProcess_mnuViewModule0-10查看线程FrmMain_mnuProcess_mnuViewThread0-10查看内存FrmMain_mnuProcess_mnuViewMemory0-10查看窗口列表FrmMain_mnuProcess_mnuViewWindowList0-10查看窗口定时器列表FrmMain_mnuProcess_mnuViewWindowTimer0-10查看句柄列表FrmMain_mnuProcess_mnuViewHandleList0-10查看网络连接列表FrmMain_mnuProcess_mnuViewWebConnect0-10查看特权FrmMain_mnuProcess_mnuViewPrivilege0-10查看HookFrmMain_mnuProcess_mnuViewHook0-10-FrmMain_mnuProcess_mnuStep30-10复制FrmMain_mnuProcess_mnuCopy0-10{复制单格FrmMain_mnuProcess_mnuLittleCopyCtrl+C0-10}定位文件位置(资源浏览器)FrmMain_mnuProcess_mnuLocateFilePath0-10
Left=510
Top=250
Tag=

[ListView]
Name=ListView1
Help=
Index=-1
Style=1 - 报表视图
BStyle=3 - 凹边框
Theme=False
ImageNormalS=无图像列表控件
ImageSmallS=无图像列表控件
SingleSel=False
ShowSel=True
Sort=0 - 不排序
SortColumn=True
NoScroll=False
AutoArrange=False
EditLabels=False
OwnerData=False
AlignTop=False
AlignLeft=False
OwnDraw=False
ColumnHeader=True
ColumnEditor=
NoSortHeader=False
BorderSelect=False
Check=False
FlatScrollBars=True
FullRowSelect=True
GridLines=True
HeaderDragDrop=False
InfoTip=False
LabelTip=True
MultiWorkAreas=False
OneClickActivate=False
Regional=False
SimpleSelect=False
SubItemImages=False
TrackSelect=False
TwoClickActivate=False
UnderlineCold=False
UnderlineHot=False
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,5
TextBkColor=SYS,5
Font=微软雅黑,9,0
Left=200
Top=0
Width=540
Height=440
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[TreeView]
Name=TreeView
Help=
Index=-1
BStyle=3 - 凹边框
Theme=True
ImageList=无图像列表控件
HasButtons=True
HasLines=True
LinesRoot=True
EditLabels=True
DisableDragDrop=False
ShowSelAlways=True
RTLreading=False
NoToolTips=False
Check=False
TrackSelect=False
SingleExpand=False
InfoTip=False
FullRowSelect=False
NoScroll=False
NoNevenHeight=False
NoHScroll=False
Enabled=True
Visible=True
LineColor=SYS,25
ForeColor=SYS,25
BackColor=SYS,5
Font=微软雅黑,9,0
Left=200
Top=0
Width=270
Height=440
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False


[AllCode]
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。
#include Once "win/winuser.bi"
#include Once "Afx/CDicObj.inc"
#include "Afx/CVar.inc"

Type MemoryStruct
    dwProcessId As HANDLE
    Addr As PVOID
    dwSize As size_t
    pData As PVOID
End Type

Const MAX_LEN = 200
Const MIN_WIDTH = 350
Const MIN_HEIGHT = 500
Const MAX_WIDTH = 350
Const MAX_HEIGHT = 500
Const GWL_WNDPROC = -4

'Type WINDOWPROC As Function(ByVal As HWND, ByVal As UINT, ByVal As WPARAM, ByVal As LPARAM) As LRESULT

Dim Shared CurrentInformation As CURRENT_INFORMATION
Dim Shared CurrentListViewItem(Any, Any) As ListViewItemInfo
Dim Shared CurrentPos As Point
'Dim Shared hook As HHOOK
Dim Shared hDrv As HANDLE
Dim Shared IsHaveSelectItem As Boolean
Dim Shared IsDriverLoaded As BOOLEAN = False
Dim Shared PIDs() As DWORD
Dim Shared FilePathByUnlock As String
Dim Shared prevFrmMainProc As LONG_PTR = NULL

Private Sub DrawTreeView()
    Dim treKernel As HTREEITEM, treHardDisk As HTREEITEM, treOther As HTREEITEM, treKernelCallbacks As HTREEITEM
    treMain.AddItem NULL, "前台窗口"
    treMain.AddItem NULL, "进程"
    treKernel = treMain.AddItem(NULL, "内核")
    treMain.AddItem treKernel, "内核模块"
    treMain.AddItem treKernel, "内核线程"
    Dim treTable As HTREEITEM = treMain.AddItem(treKernel, "内核表项")
    treMain.AddItem treTable, "SSDT"
    treMain.AddItem treTable, "Shadow SSDT"
    treMain.AddItem treTable, "GDT"
    treMain.AddItem treTable, "IDT"
    Dim treHal As HTREEITEM = treMain.AddItem(treTable, "Hal")
    treMain.AddItem treHal, "HalDispatch"
    treMain.AddItem treHal, "HalPrivateDispatch"
    Dim treKernelHook As HTREEITEM = treMain.AddItem(treKernel, "内核Hook")
    treMain.AddItem treKernelHook, "Object Hook"
    Dim treGUI As HTREEITEM = treMain.AddItem(treKernel, "GUI相关")
    treMain.AddItem treGUI, "热键"
    treKernelCallbacks = treMain.AddItem(treKernel, "内核回调")
    treMain.AddItem treKernelCallbacks, "Callbacks"
    treMain.AddItem treKernelCallbacks, "KernelTimer"
    treMain.AddItem treKernelCallbacks, "Minifilter"
    treMain.AddItem treKernelCallbacks, "过滤驱动"
    treHardDisk = treMain.AddItem(NULL, "硬盘")
    treMain.AddItem treHardDisk, "文件"
    treMain.AddItem NULL, "注册表"
    treOther = treMain.AddItem(NULL, "启动管理")
    treMain.AddItem treOther, "服务"
    treMain.AddItem treOther, "启动项"
    Dim treAdvance As HTREEITEM = treMain.AddItem(NULL, "高级")
    treMain.AddItem treAdvance, "暴力检测"
    treMain.AddItem treAdvance, "设置"
End Sub

Sub FrmMain_Shown(hWndForm As hWnd, UserData As Integer)
    'ChangeWindowMessageFilter(0x0049, MSGFLT_ADD)'允许拖放文件
    'CurrentListViewItem.BackColor = BGR(255, 255, 255)
    'CurrentListViewItem.ForeColor = BGR(0, 0, 0)
    'AfxMsg ForceKillProcess(GetCurrentProcessId)
    'AfxMsg SizeOf(ATTACH_DEVICE_INFO)
    AdjustPrivilege
    AdjustPrivilege SE_LOAD_DRIVER_NAME
    AdjustPrivilege SE_SHUTDOWN_NAME
    DrawTreeView
    FrmListView.Hide
    'SetWindowZ Picture1.hWnd
    TreeView.Visible = False
    SetWindowZ ListView1.hWnd
    'FmeAdvance.Visible
    CurrentInformation.hListView = Me.hWnd
    CurrentInformation.intType = Process
    InitializeListView ListView1, mCtrlTreeList1, TreeView
    TxtFilter2.WindowsZ HWND_TOP
    TrayIco1.Create
    AddRButtonMenu
    ReDim Preserve CurrentInformationArray(10) As CURRENT_INFORMATION
    ' 移除根项线条样式
    SendMessage mCtrlTreeList1.hWnd, TVM_SETEXTENDEDSTYLE, 0, _
    SendMessage(mCtrlTreeList1.hWnd, TVM_GETEXTENDEDSTYLE, 0, 0) And (Not TVS_LINESATROOT)
    If Command(1) = "-LoadDriver" And IsAdmin Then
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If Not LoadDriver(App.Path & "SnowSword.sys", False) Then AfxMsg "加载失败!"
            hDrv = OpenDrv("\\.\\SnowSword")
            If (hDrv <> INVALID_HANDLE_VALUE) Then
                AfxMsg "加载成功!"
                Check6.Value =True
                IsDriverLoaded = True
            Else
                AfxMsg "加载失败!"
            End If
        End If
    ElseIf Command(1) = "-DeleteFile" Then
        AfxMsg Command(2)
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If Not LoadDriver(App.Path & "SnowSword.sys", False) Then AfxMsg "加载失败!"
            hDrv = OpenDrv("\\.\\SnowSword")
            If (hDrv <> INVALID_HANDLE_VALUE) Then
                AfxMsg "加载成功!"
                Check6.Value =True
                IsDriverLoaded = True
                Dim ustrFilePath As StringW = "\??\" & Command(2)
                'IoControl hDrv, IOCTL_DeleteFile, ustrFilePath.WstrPtr, ustrFilePath.WstrLen * 2
                If GetLastError = 0 Then
                    AfxMsg "粉碎成功!"
                Else
                    AfxMsg "粉碎失败!"
                    Print "[IOCTL_DeleteFile]" & WinErrorMsg(GetLastError) & GetLastError
                End If
            Else
                AfxMsg "加载失败!"
            End If
        End If
    /'ElseIf Command(1) = "-UnlockFile" Then
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If Not LoadDriver(App.Path & "SnowSword.sys", False) Then AfxMsg "加载失败!"
            hDrv = OpenDrv("\\.\\SnowSword")
            If (hDrv <> INVALID_HANDLE_VALUE) Then
                AfxMsg "加载成功!"
                Check6.Value =True
                IsDriverLoaded = True
                FilePathByUnlock = Command(2)
                FrmListView.Show ,, UnlockTheFile
            Else
                AfxMsg "加载失败!"
            End If
        End If'/
    End If
    'GetProcessList ListView1, False
    'lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    'SetListViewData ListView1
    InitNtUserFunction
    'If SymInit(GetCurrentProcess) = True Then QuerySymbol Cast(PULONG64, &HFFFFF8011B8F0000), NULL
    'AfxMsg CheckFileTrust("D:\Dialog.dll") ' VerifyFileByCatalog
    'AfxMsg CheckFileTrust("C:\Windows\System32\notepad.exe")
    'AfxMsg SizeOf(WINTRUST_DATA)
    'AfxMsg CheckFileTrust("C:\Windows\System32\winlogon.exe")
    'Print VerifyFileByCatalog(StrPtrW("C:\Windows\System32\win32k.sys"))
    'Print VerifyFileByCatalog(StrPtrW("C:\Windows\System32\win32k.sys"))
    'Print VerifyFileByCatalog2(StrPtrW("C:\Windows\System32\win32k.sys"))
    'Print VerifyFileByCatalog2(StrPtrW("C:\Windows\explorer.exe"))
    'Dim err1 As Long
    'Print VerifyFileByCatalog2("C:\Windows\explorer.exe")
    prevFrmMainProc = SetWindowLongPtr(FrmMain.hWnd, GWL_WNDPROC, Cast(LONG_PTR, @WndProc))
End Sub

'[Form1.ListView1]事件 : 鼠标右键单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'xPos yPos   当前鼠标位置，相对于屏幕。就是屏幕坐标。
Sub FrmMain_ListView1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    CurrentPos.x = xPos
    CurrentPos.y = yPos
    ScreenToClient hWndControl, @CurrentPos
    Select Case CurrentInformation.intType
        Case Process
            PopupMenu hWndForm,mnuProcess.HMENU
        Case File
            PopupMenu hWndForm, mnuFile.HMENU
        Case KernelModule
            PopupMenu hWndForm, mnuKernelModule.HMENU
        Case Callbacks
            PopupMenu hWndForm, mnuCallbacks.HMENU
        Case Service
            PopupMenu hWndForm, mnuService.HMENU
    End Select
End Sub

'[FrmMain.mnuProcess]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码
'wID      菜单项命令ID
Sub FrmMain_mnuProcess_WM_Command(hWndForm As hWnd, wID As ULong)
    'Dim CurrentIndex As Integer = GetIndexByListViewHwnd(Me.hWnd)
    CurrentInformation.ProcessId = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0))
    Dim CurrentProcessName As StringW = ListView1.GetItemText(ListView1.SelectedItem, 2)
    Select Case wID
        Case FrmMain_mnuProcess_mnuTerminateProcess ' 结束进程
            Dim dwProcessId As DWORD
            For i As Integer = ListView1.ItemCount - 1 To 0 Step -1
                If IsListViewItemSelected(ListView1, i) Then
                    dwProcessId = ValULng(ListView1.GetItemText(i, 0))
                    Dim IsCritical As Boolean
                    If (IsProcessCritical(dwProcessId, @IsCritical)) AndAlso IsCritical Then
                        If AfxMsg("进程" & dwProcessId & "处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDYES Then
                            SetCriticalProcess dwProcessId, False
                        Else
                            Exit Sub
                        End If
                    End If
                    If Not KillProcess(dwProcessId) Then
                        AfxMsg "结束进程" & dwProcessId & "失败!"
                        Continue For
                    End If
                    DeleteItemEx ListView1, i
                End If
            Next
            'RefreshListViewItem ListView1
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuProcess_mnuForceTerminateProcess '强制结束进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
                hDrv = OpenDrv("\\.\\SnowSword")
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    AfxMsg "加载成功!"
                    Check6.Value =True
                    IsDriverLoaded = True
                Else
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
            End If
            For i As Integer = ListView1.ItemCount - 1 To 0 Step -1
                If IsListViewItemSelected(ListView1, i) Then
                    Dim dwPID As HANDLE = Cast(HANDLE, ValLng(ListView1.GetItemText(i, 0)))
                    Dim IsCritical As Boolean
                    If (IsProcessCritical(dwProcessId, @IsCritical)) AndAlso IsCritical Then
                        If AfxMsg("进程" & dwProcessId & "处于Critical状态,强行结束可能蓝屏,是否结束?", "提示", MB_YESNO) = IDYES Then
                            SetCriticalProcess dwProcessId, False
                        Else
                            Exit Sub
                        End If
                    End If
                    IoControl hDrv, IOCTL_KillProcess, @dwPID, SizeOf(HANDLE)
                    DeleteItemEx ListView1, i
                End If
            Next
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
            'RefreshListViewItem ListView1
        Case FrmMain_mnuProcess_mnuViolentTerminateProcess '暴力结束进程
            Dim dwProcessId As HANDLE = Cast(HANDLE, ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0)))
            /'Dim ret As String * 100, lpRet As DWORD
            Dim hProcess As HANDLE
            IoControl hDrv, IOCTL_OpenProcess, @dwProcessId, SizeOf(DWORD), @hProcess, SizeOf(HANDLE)
            TerminateProcess hProcess, 0
            ZwClose hProcess
            ListView1.DeleteItem ListView1.SelectedItem
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
            Exit Sub'/
            If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
                hDrv = OpenDrv("\\.\\SnowSword")
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    AfxMsg "加载成功!"
                    Check6.Value =True
                    IsDriverLoaded = True
                Else
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
            End If
            IoControl hDrv, IOCTL_MemKillProcess, @dwProcessId, SizeOf(DWORD) ' 容易导致线程在KeSynchronizeExecution+0x4891处死锁,杀不死
            'IoControl hDrv, IOCTL_ForceKillProcess, @dwProcessId, SizeOf(HANDLE)
        Case FrmMain_mnuProcess_mnuSuspendProcess ' 挂起进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            For i As Integer = ListView1.ItemCount - 1 To 0 Step -1
                If IsListViewItemSelected(ListView1, i) Then
                    dwProcessId = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0))
                    If Not SuspendProcess(dwProcessId) Then AfxMsg "挂起进程" & dwProcessId & "失败!" 
                End If
            Next
        Case FrmMain_mnuProcess_mnuResumeProcess ' 恢复进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            For i As Integer = ListView1.ItemCount - 1 To 0 Step -1
                If IsListViewItemSelected(ListView1, i) Then
                    dwProcessId = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0))
                    If Not ResumeProcess(dwProcessId) Then AfxMsg "恢复进程" & dwProcessId & "失败!" 
                End If
            Next
        Case FrmMain_mnuProcess_mnuProtectProcess
            If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
                hDrv = OpenDrv("\\.\\SnowSword")
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    AfxMsg "加载成功!"
                    FrmMain.Check6.Value =True
                    IsDriverLoaded = True
                Else
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
            End If
            Dim dwProcessId As DWORD = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0)), ret As Long, lpRet As DWORD
            IoControl hDrv, IOCTL_AddProtectedProcess, @dwProcessId, SizeOf(DWORD)
        Case FrmMain_mnuProcess_mnuCheckHideProcess '检测隐藏进程
            If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
                hDrv = OpenDrv("\\.\\SnowSword")
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    AfxMsg "加载成功!"
                    FrmMain.Check6.Value =True
                    IsDriverLoaded = True
                Else
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
            End If
            SetMenuCheckState mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess, Not GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
        Case FrmMain_mnuProcess_mnuCheckSign ' 校验数字签名
            For i As Integer = 0 To ListView1.ItemCount - 1
                If Not IsListViewItemSelected(ListView1, i) Then Continue For
                
                Dim dwPID As DWORD = ValUInt(ListView1.GetItemText(i, 0))
                Dim strPath As StringW = ListView1.GetItemText(i, 4)
                Dim strCompany As StringW = ListView1.GetItemText(i, 6)
                
                Dim ForeColor As COLORREF, BackColor As COLORREF
                
                If (strCompany <> "Microsoft Corporation") AndAlso (dwPID <> 0) OrElse _
                (InStr(strPath, "\") <> 0 AndAlso (strCompany <> "") AndAlso (Not VerifyFileSign(strPath))) Then
                    GetItemColor ListView1, i, ForeColor, BackColor
                    SetItemColor ListView1, i, ForeColor, FB_GoldenYellow
                    Print dwPID & "可疑!"
                    'Print strCompany & " " & strPath & " " & VerifyFileSign(strPath)
                End If
                FF_DoEvents
            Next
            ListView1.Refresh
        Case FrmMain_mnuProcess_mnuCheckAllSign ' 校验所有数字签名
            For i As Integer = 0 To ListView1.ItemCount - 1
                Dim dwPID As DWORD = ValUInt(ListView1.GetItemText(i, 0))
                Dim strPath As StringW = ListView1.GetItemText(i, 4)
                Dim strCompany As StringW = ListView1.GetItemText(i, 6)
                
                Dim ForeColor As COLORREF, BackColor As COLORREF
                
                If (strCompany <> "Microsoft Corporation") AndAlso (dwPID <> 0) OrElse _
                (InStr(strPath, "\") <> 0 AndAlso (strCompany <> "") AndAlso (Not VerifyFileSign(strPath))) Then
                    GetItemColor ListView1, i, ForeColor, BackColor
                    SetItemColor ListView1, i, ForeColor, FB_GoldenYellow
                    Print dwPID & "可疑!"
                    'Print strCompany & " " & strPath & " " & VerifyFileSign(strPath)
                End If
                FF_DoEvents
            Next
            ListView1.Refresh
        Case FrmMain_mnuProcess_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetProcessList ListView1, GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            SetListViewData ListView1
            lblNum.Caption = "数量:" & WStr(FilterAllItem(ListView1))
        Case FrmMain_mnuProcess_mnuInjectDll ' 注入DLL
            Dim szFile As StringW = FF_OpenFileDialog(,,,,"DLL files (*.dll)|*.dll|" & "All Files (*.*)|*.*|")
            If szFile <> "" Then If RemoteInjectDll(CurrentInformation.ProcessId, szFile) Then AfxMsg "注入成功!" Else AfxMsg "注入失败!"
        Case FrmMain_mnuProcess_mnuViewModule ' 查看模块
            FrmListView.Show,, Module
        Case FrmMain_mnuProcess_mnuViewThread ' 查看线程
            FrmListView.Show,, Thread
        Case FrmMain_mnuProcess_mnuViewMemory ' 查看内存
            FrmListView.Show ,, Memory
        Case FrmMain_mnuProcess_mnuViewWindowList ' 查看窗口列表
            FrmListView.Show,, Windows
        Case FrmMain_mnuProcess_mnuViewWindowTimer ' 查看窗口定时器列表
            FrmListView.Show,, WindowTimer
        Case FrmMain_mnuProcess_mnuViewHandleList ' 查看句柄列表
            FrmListView.Show,, Handles
        Case FrmMain_mnuProcess_mnuViewWebConnect ' 查看网络连接列表
            FrmListView.Show,, WebConnect
        Case FrmMain_mnuProcess_mnuViewPrivilege ' 查看特权列表
            FrmListView.Show,, Privilege
        Case FrmMain_mnuProcess_mnuViewHook ' 查看Hook
            CurrentInformation.CurrentModule.ModuleName = ""
            CurrentInformation.CurrentModule.ModuleHandle = NULL
            FrmListView.Show,, Hook
        Case FrmMain_mnuProcess_mnuLittleCopy ' 复制单格数据
            Dim lvinfo As LVHITTESTINFO
            lvinfo.pt.x = CurrentPos.x
            lvinfo.pt.y = CurrentPos.y
            ListView_SubItemHitTest(ListView1.hWnd, @lvinfo)
            Print lvinfo.iItem & " " & lvinfo.iSubItem
            If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
            (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
            End If
        Case FrmMain_mnuProcess_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 4)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
    End Select
End Sub

'[FrmMain.treMain]事件 : 双击鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_treMain_WM_LButtonDblclk(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Dim CurrentIndex As Integer = GetIndexByListViewHwnd(Me.hWnd)
    Dim SelectText As String = treMain.Text(treMain.HitTest(xPos, yPos))
    If SelectText <> "" Then
        If SelectText = "内核" OrElse SelectText = "内核表项" OrElse SelectText = "内核回调" OrElse _
        SelectText = "硬盘" OrElse SelectText = "其他" OrElse SelectText = "GUI相关" OrElse _
        SelectText = "高级" OrElse SelectText = "Hal" OrElse SelectText = "内核Hook" Then Exit Sub
        ListView1.Top = 0
        TreeView.Top = 0
        ListView1.Height = CLng(AfxScaleX(440))
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540)) ' 1080
        ListView1.Visible = True
        TreeView.Height = CLng(AfxScaleX(440))
        txtFilePath.Visible = False
    End If
    If SelectText = "前台窗口" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = ForegroundWindow
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetVisibleWindowList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "进程" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = Process
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetProcessList ListView1, GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "内核模块" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = KernelModule
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetKernelModuleList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "内核线程" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = KernelThread
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetKernelThreadList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "SSDT" Then
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540))
        ListView1.Visible = True
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = SSDT
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetSSDT(ListView1))
    ElseIf SelectText = "Shadow SSDT" Then
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540))
        ListView1.Visible = True
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = ShadowSSDT
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetSSSDT(ListView1))
    ElseIf SelectText = "GDT" Then
        mCtrlTreeList1.Top = 0
        mCtrlTreeList1.Height = CLng(AfxScaleX(440))
        mCtrlTreeList1.Left = treMain.Width
        mCtrlTreeList1.Width = CLng(AfxScaleX(540))
        mCtrlTreeList1.Visible = True
        SetWindowZ mCtrlTreeList1.hWnd
        CurrentInformation.intType = GDT
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetGDT(mCtrlTreeList1))
    ElseIf SelectText = "IDT" Then
        mCtrlTreeList1.Top = 0
        mCtrlTreeList1.Height = CLng(AfxScaleX(440))
        mCtrlTreeList1.Left = treMain.Width
        mCtrlTreeList1.Width = CLng(AfxScaleX(540))
        mCtrlTreeList1.Visible = True
        SetWindowZ mCtrlTreeList1.hWnd
        CurrentInformation.intType = IDT
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetIDT(mCtrlTreeList1))
    ElseIf SelectText = "HalDispatch" Then
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540))
        ListView1.Visible = True
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = HalDispatch
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetHalDispatchTable(ListView1))
    ElseIf SelectText = "HalPrivateDispatch" Then
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540))
        ListView1.Visible = True
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = HalPrivateDispatch
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(GetHalPrivateDispatchTable(ListView1))
    ElseIf SelectText = "Object Hook" Then
        ListView1.Left = treMain.Width
        ListView1.Width = CLng(AfxScaleX(540))
        ListView1.Visible = True
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = ObjectHook
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetObjectInfo ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "Callbacks" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = Callbacks
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetCallbackList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "KernelTimer" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = KernelTimer
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetKernelTimerList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "Minifilter" Then
        mCtrlTreeList1.Top = 0
        mCtrlTreeList1.Height = CLng(AfxScaleX(440))
        mCtrlTreeList1.Left = treMain.Width
        mCtrlTreeList1.Width = CLng(AfxScaleX(540))
        mCtrlTreeList1.Visible = True
        SetWindowZ mCtrlTreeList1.hWnd
        CurrentInformation.intType = Minifilter
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(EnumMinifilter(mCtrlTreeList1))
    ElseIf SelectText = "过滤驱动" Then
        mCtrlTreeList1.Top = 0
        mCtrlTreeList1.Height = CLng(AfxScaleX(440))
        mCtrlTreeList1.Left = treMain.Width
        mCtrlTreeList1.Width = CLng(AfxScaleX(540))
        mCtrlTreeList1.Visible = True
        SetWindowZ mCtrlTreeList1.hWnd
        CurrentInformation.intType = FilterDriver
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        lblNum.Caption = "数量:" & WStr(EnumAttachDevices(mCtrlTreeList1))
    ElseIf SelectText = "文件" Then
        ListView1.Left = treMain.Width + TreeView.Width
        ListView1.Height = CLng(AfxScaleX(440 - 20))
        ListView1.Width = CLng(AfxScaleX(270))
        ListView1.Top = CLng(AfxScaleX(20))
        txtFilePath.Visible = True
        txtFilePath.Width = CLng(AfxScaleX(540))
        TreeView.Height = CLng(AfxScaleX(440 - 20))
        TreeView.Top = CLng(AfxScaleX(20))
        TreeView.Visible = True
        SetWindowZ ListView1.hWnd
        SetWindowZ TreeView.hWnd
        SetWindowZ txtFilePath.hWnd
        txtFilePath.Visible = True
        CurrentInformation.intType = File
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetDriveList TreeView
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "注册表" Then
        ListView1.Left = treMain.Width + TreeView.Width
        ListView1.Height = CLng(AfxScaleX(440 - 20))
        ListView1.Width = CLng(AfxScaleX(270))
        ListView1.Top = CLng(AfxScaleX(20))
        txtFilePath.Visible = True
        txtFilePath.Width = CLng(AfxScaleX(540))
        TreeView.Height = CLng(AfxScaleX(440 - 20))
        TreeView.Top = CLng(AfxScaleX(20))
        TreeView.Visible = True
        SetWindowZ ListView1.hWnd
        SetWindowZ TreeView.hWnd
        SetWindowZ txtFilePath.hWnd
        CurrentInformation.intType = Registry
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetRootList TreeView
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "服务" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = Service
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        GetServiceList ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "暴力检测" Then
        SetWindowZ ListView1.hWnd
        CurrentInformation.intType = ViolentCheck
        InitializeListView ListView1, mCtrlTreeList1, TreeView
        lblNum.Caption = "正在获取..."
        ScanKernelMemoryMultiThread ListView1
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "设置" Then
        ListView1.Visible = False
        mCtrlTreeList1.Visible = False
        TreeView.Visible = False
        txtFilePath.Visible = False
        Exit Sub
    End If
    SetListViewData ListView1
End Sub

'[FrmMain]事件 : 即将关闭窗口，返回非0可阻止关闭
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
Function FrmMain_WM_Close(hWndForm As hWnd) As LResult
    'If (hook <> 0) Then UnhookWindowsHookEx hook
    'SetWindowLongPtr hWndForm, GWLP_WNDPROC, Cast(LONG_PTR, pOldWindowProc)
    If IsDriverLoaded Then
        CloseDrv hDrv
        UnloadDriver App.Path & "SnowSword.sys", False
    End If
    TrayIco1.Del
    UninitNtUserFunction
    SetWindowLongPtr FrmMain.hWnd, GWL_WNDPROC, prevFrmMainProc
    'UninitSelfProtect
    Function = FALSE ' 返回 TRUE 阻止关闭窗口。
    End
End Function

Sub DeleteChildItem(MyTreeView As Class_TreeView, Node As HTREEITEM)
    Dim CurrentItem As HTREEITEM = MyTreeView.GetChild(Node)
    Dim NextItem As HTREEITEM
    'If (CurrentItem = NULL) Then Exit Sub
    Do While CurrentItem <> NULL
        NextItem = MyTreeView.GetNextSiblin(CurrentItem)
        MyTreeView.DeleteItem CurrentItem
        CurrentItem = NextItem
        FF_DoEvents
    Loop
End Sub

'[FrmMain.TreeView]事件 : 双击了鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_DBLCLK(hWndForm As hWnd,hWndControl As hWnd) As LResult
    Dim CurrentNode As HTREEITEM = TreeView.Selection
    If TreeView.Expand(CurrentNode) Then
        TreeView.ExpandEx CurrentNode, TVE_COLLAPSE
        'DeleteChildItem TreeView, CurrentNode
    ElseIf TreeView.GetChild(CurrentNode) = NULL Then
        ListView1.DeleteAllItems
        TreeView.ExpandEx CurrentNode, TVE_EXPAND
        Select Case CurrentInformation.intType
            Case File
                GetFileList CurrentNode, TreeView, ListView1, True
                GetFilePathByNodeW CurrentNode, TreeView, txtFilePath.Text
                'TreeView.ExpandEx(CurrentNode, TVE_EXPAND)
            Case Registry
                GetRegList CurrentNode, TreeView, ListView1
        End Select
        'TreeView.ExpandEx CurrentNode, TVE_EXPAND
    End If
    Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.mnuFile]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuFile_WM_Command(hWndForm As hWnd,wID As ULong)
    Dim CurrentNode As HTREEITEM = TreeView.Selection
    Dim CurrentPath As StringW
    GetFilePathByNodeW CurrentNode, TreeView, CurrentPath
    Select Case wID
        Case FrmMain_mnuFile_mnuRefresh ' 刷新
            GetFileList CurrentNode, TreeView, ListView1, (GetFocus = TreeView.hWnd)
        Case FrmMain_mnuFile_mnuCreateFile ' 新建文件
            Dim hFile As HANDLE
            Dim FileName As String = AfxInputBox(hWndForm,,, "提示", "请输入文件名:")
            hFile = CreateFileW(CurrentPath & FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL)
            If (hFile > 0) Then
                Dim nFileSize As LARGE_INTEGER
                ListView1.AddItem FileName
                GetFileSizeEx hFile, @nFileSize
                ListView1.SetItemText ListView1.ItemCount - 1, 1, WStr(nFileSize.HighPart*(2^32) + nFileSize.LowPart)
                'GetFileList CurrentNode,TreeView,ListView1
                AfxMsg "创建文件成功!"
                CloseHandle hFile
            End If
        Case FrmMain_mnuFile_mnuCreateFolder ' 新建文件夹
            Dim FolderName As String = AfxInputBox(NULL,,,"提示","请输入文件夹名:")
            If (MkDir(CurrentPath & FolderName) = 0) Then
                TreeView.InsertItem CurrentNode,TVI_SORT,FolderName
                'GetFileList CurrentNode,TreeView,ListView1
                AfxMsg "创建文件夹成功!"
            Else
                Print "新建文件夹:" & Err
                AfxMsg "创建文件夹失败!"
            End If
        Case FrmMain_mnuFile_mnuCopy ' 复制

        Case FrmMain_mnuFile_mnuPaste ' 粘贴

        Case FrmMain_mnuFile_mnuDelete ' 删除
            Dim Path As StringW, ret As NTSTATUS
            Path = "\??\" & CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            ret = MyDeleteFile(Path)
            If NT_SUCCESS(ret) Then
                If CurrentNode = NULL Then
                    TreeView.DeleteItem CurrentNode
                    ListView1.DeleteAllItems
                Else
                    ListView1.DeleteItem ListView1.SelectedItem
                End If
                AfxMsg "删除文件(夹)成功!"
            Else
                AfxMsg "删除文件(夹)失败!"
            End If
        Case FrmMain_mnuFile_mnuForceDelete ' 强制删除
            Dim strFile As LPWSTR = Allocate(MAX_PATH * SizeOf(WString))
            If (strFile = NULL) Then
                Print "[FrmMain_mnuFile_ForceDelete]分配内存失败"
                Exit Sub
            End If
            *strFile = "\??\" & CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            'AfxMsg *strFile
            If IsDriverLoaded Then
                #define STATUS_CANNOT_DELETE Cast(NTSTATUS, &HC0000121)
                Dim status As NTSTATUS = MyDeleteFile(strFile)
                If status = STATUS_CANNOT_DELETE Then ' 是正在运行的可执行文件
                    IoControl hDrv, IOCTL_DeleteFileByIRP, strFile, MAX_PATH * SizeOf(Wstring)
                Else ' 是被打开的文件
                    IoControl hDrv, IOCTL_DeleteFileByXCB, strFile, MAX_PATH * SizeOf(Wstring)
                    DeleteFile strFile
                    Print "[DeleteFile]GetLastError:" & WinErrorMsg(GetLastError) & GetLastError
                End If
            End If
            Deallocate strFile
        Case FrmMain_mnuFile_mnuLittleCopy ' 复制单格数据
            Dim lvinfo As LVHITTESTINFO
            lvinfo.pt.x = CurrentPos.x
            lvinfo.pt.y = CurrentPos.y
            ListView_SubItemHitTest(ListView1.hWnd, @lvinfo)
            If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
            (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
            End If
    End Select
End Sub

'[FrmMain.TreeView]事件 : 单击了鼠标右键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_RCLICK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    'Dim a As CRegExp
    Select Case CurrentInformation.intType
        Case File
            PopupMenu hWndForm,mnuFile.HMENU
    End Select
   Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.Check3]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check3_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Check3.Value = False
            Exit Sub
        End If
    End If
    Dim isStatus As BOOLEAN = Check3.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, IOCTL_DenyCreateProcess, @isStatus, SizeOf(BOOLEAN)
End Sub

'[FrmMain.Check6]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check6_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Check6.Value) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
        End If
    Else
        CloseDrv hDrv
        UnloadDriver App.Path & "SnowSword.sys", False
        IsDriverLoaded = False
        AfxMsg "卸载成功!"
        Check3.Value = False
        Check4.Value = False
        Check5.Value = False
        Check8.Value = False
        'TopMenu1.Check FrmMain_TopMenu1_mnuLoadDriver, False
    End If
End Sub

'[FrmMain.Check5]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check5_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Check5.Value = False
            Exit Sub
        End If
    End If
    Dim isStatus As BOOLEAN = Check5.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, IOCTL_DenyAccessRegistry, @isStatus, SizeOf(BOOLEAN)
End Sub

'[FrmMain.Check7]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check7_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Check7.Value = False
            Exit Sub
        End If
    End If
    Dim isStatus As BOOLEAN = Check7.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, IOCTL_DenyLoadDriver, @isStatus, SizeOf(BOOLEAN)
End Sub

'[FrmMain.Check2]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check2_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    ShieldMsgHook(Check2.Value)
End Sub

'[FrmMain.Check8]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check8_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Check8.Value = False
            Exit Sub
        End If
    End If
    Dim dwProcessId As DWORD = GetCurrentProcessId, ret As Long, bStatus As BOOLEAN = Check8.Value
    If (Check8.Value) Then
        IoControl hDrv, IOCTL_SetProcessProtectStatus, @bStatus, SizeOf(Boolean)
        IoControl hDrv, IOCTL_AddProtectedProcess, @dwProcessId, SizeOf(DWORD)
        IoControl hDrv, IOCTL_SetThreadProtectStatus, @bStatus, SizeOf(BOOLEAN)
    Else
        IoControl hDrv, IOCTL_SetProcessProtectStatus, @bStatus, SizeOf(Boolean)
        IoControl hDrv, IOCTL_SetThreadProtectStatus, @bStatus, SizeOf(BOOLEAN)
    End If
End Sub

'[FrmMain.TopMenu1]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_TopMenu1_WM_Command(hWndForm As hWnd, wID As ULong)
   Select Case wID
        Case FrmMain_TopMenu1_mnuUnlockFile ' 解锁文件
            FrmListView.Show,, UnlockTheFile
            'GetThreadList1
        Case FrmMain_TopMenu1_mnuRunasAdmin ' 以管理员身份运行
            
            'FrmMemoryEditor.Show
            'Exit Sub
            RestartasAdmin
            /'Dim MyHandleInfo As HANDLE_INFO
            Dim DosPath As WString * 260
            MyHandleInfo.dwHandle = Cast(HANDLE, &H884)
            MyHandleInfo.dwProcessId = Cast(HANDLE, 35876)
            MyHandleInfo.strName = ""
            MyHandleInfo.strType = ""
            IoControl hDrv, IOCTL_QueryObject, @MyHandleInfo, SizeOf(MyHandleInfo), @MyHandleInfo, SizeOf(MyHandleInfo) '/
            
        Case FrmMain_TopMenu1_mnuFireWall ' 防火墙
            'FrmFireWall.Show
   End Select
End Sub

'[FrmMain.TxtFilter2]事件 : 文本已经被修改（修改前用 EN_UPDATE
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_TxtFilter2_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    If (CurrentInformation.intType = File) Or (CurrentInformation.intType = Registry) Then Exit Sub 
    lblNum.Caption = "正在筛选..."
    SetFilter ListView1, TxtFilter2.Text
    FilterAllItem ListView1
    lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
End Sub

'[FrmMain]事件 : 活动状态（被激活或失去焦点）
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'fActive     =0 失去焦点 <>0 得到焦点
Sub FrmMain_WM_NcActivate(hWndForm As hWnd, fActive As Long)
   Dim CurrentIndex As Integer = GetIndexByListViewHwnd(Me.hWnd)
   If fActive = 0 Then
        If CurrentIndex = -1 Then
            ReDim Preserve CurrentInformationArray(1) As CURRENT_INFORMATION
            CurrentInformationArray(0).hListView = Me.hWnd
        End If
        CurrentInformationArray(0).intType = CurrentInformation.intType
        CurrentInformationArray(0).ProcessId = CurrentInformation.ProcessId
        
        CurrentInformationArray(0).CurrentModule.ModuleName = CurrentInformation.CurrentModule.ModuleName
        CurrentInformationArray(0).CurrentModule.ModuleHandle = CurrentInformation.CurrentModule.ModuleHandle
        
        CurrentInformationArray(0).CurrentDriver.DriverBase = CurrentInformation.CurrentDriver.DriverBase
        CurrentInformationArray(0).CurrentDriver.DriverSize = CurrentInformation.CurrentDriver.DriverSize
        CurrentInformationArray(0).CurrentDriver.DriverObject = CurrentInformation.CurrentDriver.DriverObject
        CurrentInformationArray(0).CurrentDriver.DriverName = CurrentInformation.CurrentDriver.DriverName
        CurrentInformationArray(0).CurrentDriver.DriverPath = CurrentInformation.CurrentDriver.DriverPath
        'Print "[FrmMain]失去焦点," & FrmMain.hWnd & "->" & CurrentInformation.intType
    Else
        If CurrentIndex = -1 Then Exit Sub
        ' 错误做法
        'memcpy @CurrentInformation, @CurrentInformationArray(0), SizeOf(CurrentInformation)
        ' 正确做法：逐个字段赋值
        CurrentInformation.intType = CurrentInformationArray(0).intType
        CurrentInformation.ProcessId = CurrentInformationArray(0).ProcessId
        
        CurrentInformation.CurrentModule.ModuleName = CurrentInformationArray(0).CurrentModule.ModuleName
        CurrentInformation.CurrentModule.ModuleHandle = CurrentInformationArray(0).CurrentModule.ModuleHandle
        
        CurrentInformation.CurrentDriver.DriverBase = CurrentInformationArray(0).CurrentDriver.DriverBase
        CurrentInformation.CurrentDriver.DriverSize = CurrentInformationArray(0).CurrentDriver.DriverSize
        CurrentInformation.CurrentDriver.DriverObject = CurrentInformationArray(0).CurrentDriver.DriverObject
        ' ... 所有字段逐一复制
        CurrentInformation.CurrentDriver.DriverName = CurrentInformationArray(0).CurrentDriver.DriverName ' StringW会自动处理内存
        CurrentInformation.CurrentDriver.DriverPath = CurrentInformationArray(0).CurrentDriver.DriverPath
        'Print "[FrmMain]得到焦点后," & FrmMain.hWnd & "->" & CurrentInformation.intType'初始化时,ListView1.hWnd为0,可能因为初始化未完成
    End If
End Sub

'[FrmMain.Check9]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check9_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Check9.Value Then 
        InitSelfProtect
        'SetSecurityControls
    Else
        UninitSelfProtect
    End If
End Sub

'[FrmMain]事件 : 自定义消息（全部消息），在其它事件处理后才轮到本事件。
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'wMsg        消息值，0--&H400 由WIN系统定义，用户自定义是 WM_USER+** 定义。例 PostMessage( 窗口句柄 , Msg , wParam , lParam ) 
'wParam      主消息参数，什么作用，由发送消息者定义
'lParam      副消息参数，什么作用，由发送消息者定义
Function FrmMain_Custom(hWndForm As hWnd, wMsg As UInteger, wParam As wParam, lParam As lParam) As LResult
    Select Case wMsg
        Case WM_NOTIFY
            If CurrentInformation.intType <> Process Then Return CDRF_DODEFAULT
            'Print "Type" & CurrentInformation.intType
            ' 解析 NMHDR 结构体（lParam 指向它）
            Dim As LPNMHDR pNMHDR = Cast(LPNMHDR, lParam)
            ' 判断：是否来自目标 ListView，且是 NM_CUSTOMDRAW 通知
            If pNMHDR->hwndFrom = FrmMain.ListView1.hWnd AndAlso pNMHDR->code = NM_CUSTOMDRAW Then
                Dim As LPNMLVCUSTOMDRAW pLVCD = Cast(LPNMLVCUSTOMDRAW, lParam)
                Dim As LRESULT result = CDRF_DODEFAULT
                Select Case pLVCD->nmcd.dwDrawStage
                    Case CDDS_PREPAINT
                        result = CDRF_NOTIFYITEMDRAW ' 通知项绘制
                    Case CDDS_ITEMPREPAINT
                        Dim Node As ListViewDataNode Ptr = FindListViewHwnd(ListView1)
                        /'If pLVCD->nmcd.dwItemSpec Mod 2 = 0 Then
                            pLVCD->clrTextBk = FB_GoldenYellow
                            pLVCD->clrText = FB_Black
                        End If'/
                        If Node <> NULL Then
                            Dim rowIndex As Integer = pLVCD->nmcd.dwItemSpec
                            ' 检查数组边界
                            'If rowIndex = 1 Then Print Node->Filter_ItemColor(rowIndex).ForeColor & " " & Node->Filter_ItemColor(rowIndex).BackColor
                            If rowIndex >= 0 And rowIndex <= UBound(Node->Filter_ItemColor) Then
                                pLVCD->clrTextBk = Node->Filter_ItemColor(rowIndex).BackColor
                                pLVCD->clrText = Node->Filter_ItemColor(rowIndex).ForeColor
                            Else
                                ' 超出边界使用默认颜色
                                pLVCD->clrTextBk = FB_White
                                pLVCD->clrText = FB_Black
                            End If
                        Else
                            pLVCD->clrTextBk = FB_White
                            pLVCD->clrText = FB_Black
                        End If
                        result = CDRF_DODEFAULT
                End Select
                Return result
            End If
        Case WM_USER + TrayIco1.CallMsg
            Select Case lParam
                Case WM_LBUTTONDOWN
                    ' 左键单击托盘图标
                    ' 在此处添加左键单击的处理逻辑
                    'AfxMsg "左键单击托盘图标"
                Case WM_LBUTTONDBLCLK
                    ' 左键双击托盘图标
                    ' 在此处添加左键双击的处理逻辑
                    Me.WindowState = 0 ' 显示主窗口
                    Me.Show
                Case WM_RBUTTONDOWN
                    ' 右键单击托盘图标
                    ' 获取鼠标位置并显示右键菜单
                    /'Dim P As Point
                    GetCursorPos @P
                    Dim hMenu As HMENU
                    hMenu = CreatePopupMenu()
                    AppendMenu hMenu, MF_STRING, 1001, "菜单项1"
                    AppendMenu hMenu, MF_STRING, 1002, "菜单项2"
                    'SetForegroundWindow Me.hWnd '确保主窗口为前台窗口
                    TrackPopupMenu hMenu, TPM_LEFTALIGN, P.x, P.y, 0, Me.hWnd, 0'/
                    'PostMessage Me.hWnd, WM_NULL, 0, 0 '发送空消息激活消息循环
                    PopupMenu hWndForm, mnuCallbacks.HMENU
                    'DestroyMenu hMenu
                    'SetFocus Me.hWnd ' 设置当前窗口的焦点
            End Select
    End Select
    Function = FALSE ' 返回FALSE表示不阻止系统继续处理消息
End Function

'[FrmMain.mnuKernelModule]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuKernelModule_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuKernelModule_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetKernelModuleList ListView1
            SetListViewData ListView1
            lblNum.Caption = "数量:" & FilterAllItem(ListView1)
        Case FrmMain_mnuKernelModule_mnuUnloadDriver ' 卸载驱动
            If Not (UnloadDriver(ListView1.GetItemText(ListView1.SelectedItem, 4), False)) Then
                If IsDriverLoaded Then
                    Dim pDriverObject As PVOID = Cast(PVOID, Cast(Integer, ValLng(FF_Replace(ListView1.GetItemText(ListView1.SelectedItem, 5), "0x", "&H"))))
                    IoControl hDrv, IOCTL_UnloadDriver, @pDriverObject, SizeOf(pDriverObject)
                End If
            Else
                AfxMsg "卸载成功!"
            End If
        Case FrmMain_mnuKernelModule_mnuViewIOFunction ' 查看IO派遣函数
            CurrentInformation.CurrentDriver.DriverObject = ValLng("&H" & RightW(ListView1.GetItemText(ListView1.SelectedItem, 5), LenW(ListView1.GetItemText(ListView1.SelectedItem, 5)) - 2))
            CurrentInformation.CurrentDriver.DriverName = ListView1.GetItemText(ListView1.SelectedItem, 1)
            FrmListView.Show,, IOFunction
        Case FrmMain_mnuKernelModule_mnuDumpToFile ' dump到文件
            If Not IsDriverLoaded Then
                AfxMsg "请先加载驱动!"
                Exit Sub
            End If
            Dim bytDump() As UByte
            Dim ModuleBase As PVOID = Cast(PVOID, ValULng(FF_Replace(ListView1.GetItemText(ListView1.SelectedItem, 2), "0x", "&H")))
            Dim ModuleSize As size_t = ValUInt(FF_Replace(ListView1.GetItemText(ListView1.SelectedItem, 3), "0x", "&H"))
            ReDim bytDump(ModuleSize - 1) As UByte
            
            Dim lpRet As DWORD = 0
            Dim stMemory As MemoryStruct
            stMemory.dwProcessId = NULL
            stMemory.Addr = ModuleBase
            stMemory.dwSize = 0 ' ModuleSize
            stMemory.pData = @bytDump(0)
            
            If IoControl(hDrv, IOCTL_DumpKernelModule, @stMemory, SizeOf(MemoryStruct), NULL, 0, @lpRet) = 0 Then
                AfxMsg "dump失败!"
                Print "[FrmMain_mnuKernelModule_mnuDumpToFile]IOCTL_ReadProcessMemory:" & WinErrorMsg(GetLastError) & GetLastError
                Print "Information:" & lpRet
                Erase bytDump
                Exit Sub
            End If
            If lpRet = 0 Then
                Erase bytDump
                Exit Sub
            End If
            Print "Information:0x" & Hex(lpRet)
            ReDim Preserve bytDump(lpRet - 1) As UByte
            Dim lpSavePath As StringW = FF_OpenFileDialog(,,,, "Driver files (*.sys)|*.sys|" & "All Files (*.*)|*.*|")
            If lpSavePath = "" Then
                AfxMsg "请输入有效路径!"
                Erase bytDump
                Exit Sub
            End If
            Open lpSavePath For Output As #1
            Put #1,, bytDump()
            Close #1
        Case FrmMain_mnuKernelModule_mnuLittleCopy ' 复制单格数据
            Dim lvinfo As LVHITTESTINFO
            lvinfo.pt.x = CurrentPos.x
            lvinfo.pt.y = CurrentPos.y
            ListView_SubItemHitTest(ListView1.hWnd, @lvinfo)
            If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
            (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
            End If
        Case FrmMain_mnuKernelModule_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 4)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
   End Select
End Sub

'[FrmMain.Command1]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Command1_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    FrmPowerManager.Show
End Sub

'[FrmMain.Check4]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check4_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Check4.Value = False
            Exit Sub
        End If
    End If
    Dim bStatus As BOOLEAN = Check4.Value
    IoControl hDrv, IOCTL_DenyRemoteThread, @bStatus, SizeOf(BOOLEAN)
End Sub

'[FrmMain.ListView1]事件 : 双击鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_ListView1_WM_LButtonDblclk(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Dim CurrentNode As HTREEITEM = TreeView.Selection
    Dim CurrentPath As StringW
    Select Case CurrentInformation.intType
        Case File
            GetFilePathByNodeW CurrentNode, TreeView, CurrentPath ' 获取当前文件路径
            ShellExecute NULL, NULL, CurrentPath, NULL, NULL, 1 ' 以默认方式打开文件
    End Select
End Sub

'[FrmMain.mnuCallbacks]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuCallbacks_WM_Command(hWndForm As hWnd, wID As ULong)
   Dim SelectIndex As Integer = ListView1.SelectedItem 
    Select Case wID
        Case FrmMain_mnuCallbacks_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetCallbackList ListView1
            SetListViewData ListView1
            lblNum.Caption = "数量:" & FilterAllItem(ListView1)
        Case FrmMain_mnuCallbacks_mnuControlCallback ' 禁用/启用回调
            
        Case FrmMain_mnuCallbacks_mnuRemoveCallback ' 移除回调
            Dim CallbackAddress As PVOID = Cast(PVOID, ValULng(FF_Replace(ListView1.GetItemText(SelectIndex, 1), "0x", "&H")))
            Dim ObHandle As PVOID, Cookie As LARGE_INTEGER
            If (IsDriverLoaded AndAlso CallbackAddress <> NULL) Then
                Dim CallbackType As StringW = ListView1.GetItemText(SelectIndex, 0)
                If InStr(CallbackType, "Ob") > 0 Then
                    Dim ObHandle As PVOID = Cast(PVOID, CallbackArray(SelectIndex).TheContext)
                    IoControl hDrv, IOCTL_UnregisterObCallback, @ObHandle
                ElseIf CallbackType = "CreateProcess" Then
                    IoControl hDrv, IOCTL_RemoveCreateProcessNotifyRoutine, @CallbackAddress
                ElseIf CallbackType = "CreateThread" Then
                    IoControl hDrv, IOCTL_RemoveCreateThreadNotifyRoutine, @CallbackAddress
                ElseIf CallbackType = "LoadImage" Then
                    IoControl hDrv, IOCTL_RemoveLoadImageNotifyRoutine, @CallbackAddress
                ElseIf CallbackType = "CmpCallback" Then
                    Cookie.QuadPart = CallbackArray(SelectIndex).TheContext
                    IoControl hDrv, IOCTL_UnregisterCmpCallback, @Cookie
                End If
            End If
        Case FrmMain_mnuCallbacks_mnuLittleCopy ' 复制单格数据
            Dim lvinfo As LVHITTESTINFO
            lvinfo.pt.x = CurrentPos.x
            lvinfo.pt.y = CurrentPos.y
            ListView_SubItemHitTest(ListView1.hWnd, @lvinfo)
            If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
            (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
            End If
        Case FrmMain_mnuCallbacks_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 2)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
   End Select
End Sub

'[FrmMain.mnuService]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuService_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuService_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetServiceList ListView1
            SetListViewData ListView1
            lblNum.Caption = "数量:" & FilterAllItem(ListView1)
        Case FrmMain_mnuService_mnuLittleCopy ' 复制单格数据
            Dim lvinfo As LVHITTESTINFO
            lvinfo.pt.x = CurrentPos.x
            lvinfo.pt.y = CurrentPos.y
            ListView_SubItemHitTest(ListView1.hWnd, @lvinfo)
            If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
            (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
            End If
        Case FrmMain_mnuService_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 2)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
   End Select
End Sub

'[FrmMain.txtFilePath]事件 : 释放某按键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'nVirtKey 虚拟键代码。详细看微软的 https://docs.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes  
'lKeyData 键盘数据：0-15重复计数、16-23扫描代码、24-24扩展键标志、25-28保留；不使用、29-29上下文代码、30-30先前的键状态标志、31-31过渡状态标志  
''        重复计数 = LoWord(lKeyData)                 : 当前消息的重复计数。该值是由于用户按住键而自动重复击键的次数。如果按键保持足够长的时间，则会发送多条消息。但是，重复计数不是累积的。
''        扫描代码 = (lKeyData And &hFF0000) Shr 16   : 该值取决于OEM。
''        扩展标志 = (lKeyData And &h1000000) Shr 24  : 指示该键是否为扩展键，例如出现在增强型101键或102键键盘上的右侧ALT和CTRL键。如果是扩展键，则值为1；否则为0
''        上下文   = (lKeyData And &h20000000) Shr 29 : 对于WM_KEYUP消息，该值始终为0 。
''        先前状态 = (lKeyData And &h40000000) Shr 30 : 对于WM_KEYUP消息，该值始终为1 。
''        过渡状态 = (lKeyData And &h80000000) Shr 31 : 对于WM_KEYUP消息，该值始终为1 。 
Function FrmMain_txtFilePath_WM_KeyUp(hWndForm As hWnd, hWndControl As hWnd, nVirtKey As Long, lKeyData As Long) As Long
    Dim CurrentNode As HTREEITEM = NULL
    Dim CurrentPath As StringW
    If (nVirtKey = 13) Then ' 按下Enter键
        Dim strPath As StringW = txtFilePath.Text
        If (GetFileAttributes(strPath) And fbDirectory) = fbDirectory Then ' 是目录,跳转过去
            CurrentNode = GetNodeByFilePathW(strPath, TreeView, ListView1) ' 查找并获取对应Node
            If CurrentNode <> NULL Then
                SendMessage TreeView.hWnd, TVM_SELECTITEM, TVGN_CARET, Cast(lParam, CurrentNode) ' 选中这项
            Else
                Print "[FrmMain_txtFilePath_WM_KeyUp]CurrentNode = NULL"
            End If
        Else ' 是文件,直接打开
            GetFilePathByNodeW CurrentNode, TreeView, CurrentPath ' 获取当前文件路径 
            ShellExecute NULL, NULL, CurrentPath, NULL, NULL, 1 ' 以默认方式打开文件
        End If
    End If
    Function = FALSE ' 若想屏蔽这个按键到控件中就返回 TRUE （注意：屏蔽字符用WM_CHAR事件）    
End Function
'[FrmMain.Check10]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check10_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Check10.Value Then
        SetWindowZ hWndForm, HWND_TOPMOST
    Else
        SetWindowZ hWndForm, HWND_NOTOPMOST
    End If
End Sub

Function 保存软件崩溃日志(文件名 As String, excp As EXCEPTION_POINTERS) As String '返回日志内容
   Dim vi As OSVERSIONINFO
   vi.dwOsVersionInfoSize = SizeOf(OSVERSIONINFO)
   GetVersionEx @vi
   Dim bug As String = " OS 版本:      " & vi.dwMajorVersion & "." & vi.dwMinorVersion & "." & vi.dwBuildNumber & vbCrLf
   bug &= "区域设置 ID:   " & GetUserDefaultLangID & vbCrLf
   bug &= "应用程序名:    " & App.EXEName          & vbCrLf
   bug &= "应用程序版本:  " & App.ProductMajor     & "." & App.ProductMinor & "." & App.ProductRevision & "." & App.ProductBuild & vbCrLf
   Dim gzmk As String, AllMode As String, pianyi As UInteger, gzmkbb As String
   Dim ExceptionAddress As UInteger = Cast(UInteger, excp.ExceptionRecord->ExceptionAddress)
   Dim I As UInteger
   
   Dim bI      As Long, mSnapshot As HANDLE
   Dim Mode    As MODULEENTRY32
   Dim gMode() As MODULEENTRY32
   '----------------查找进程的执行程序的路径-----------------------
   '通过模块快照，获得进程的模块快照句柄
   mSnapshot = CreateToolhelp32Snapshot(&H8, GetCurrentProcessId()) 'Const TH32CS_SNAPmodule = &H8
   If mSnapshot > 0 Then
      Mode.dwSize = SizeOf(Mode) '初始化结构mo的大小
      '用该进程第1个模块的szExePath字段，作为进程的程序路径
      If Module32First(mSnapshot, @Mode) Then
         Do
            ReDim Preserve gMode(bi)
            gMode(bi) = Mode
            bi        = bi + 1
            AllMode   &= Hex(Mode.modBaseAddr, Len(UInteger) * 2) & " " & CWSTRtoString(Mode.szExePath) & "  [" & GetVersionInfo(Mode.szExePath) & "][" & GetFileLength(Mode.szExePath) & "]" & vbCrLf
            If ExceptionAddress > Cast(UInteger, Mode.modBaseAddr) And ExceptionAddress < Cast(UInteger, Mode.modBaseAddr) + Cast(UInteger, Mode.modBaseSize) Then
               gzmk   = CWSTRtoString(Mode.szModule)
               gzmkbb = GetVersionInfo(Mode.szExePath)
               pianyi = ExceptionAddress - Cast(UInteger, Mode.modBaseAddr)
            End if
         Loop Until Module32Next(mSnapshot, @Mode) = 0
      End If
      CloseHandle(mSnapshot) '关闭模块快照句柄
   End If
   
   '32位 寄存器 excp.ContextRecord->Eax Ebx Ecx Edx Ebp Esi Edi Eip
   '62位 寄存器 excp.ContextRecord->Rax Rbx Rcx Rdx Rbp Rsi Rdi Rip Rsp R8 R9 R10 R11 R12 R13 R14 R15
   
   bug &= "故障模块名称:  " & gzmk   & vbCrLf
   bug &= "故障模块版本:  " & gzmkbb & vbCrLf
   
   Dim ErrStr As String
   Select Case excp.ExceptionRecord->ExceptionCode '发生异常的原因。这是由硬件异常生成的代码，或在RaiseException函数中为软件生成的异常指定的代码 。
      Case EXCEPTION_ACCESS_VIOLATION
         ErrStr = "线程试图读取或写入对其没有适当访问权限的虚拟地址。"
      Case EXCEPTION_ARRAY_BOUNDS_EXCEEDED
         ErrStr = "线程尝试访问超出范围的数组元素，并且基础硬件支持范围检查。"
      Case EXCEPTION_BREAKPOINT
         ErrStr = "遇到断点。"
      Case EXCEPTION_DATATYPE_MISALIGNMENT
         ErrStr = "线程试图读取或写入在不提供对齐方式的硬件上未对齐的数据。例如，必须在2字节边界上对齐16位值；4字节边界上的32位值，依此类推。"
      Case EXCEPTION_FLT_DENORMAL_OPERAND
         ErrStr = "浮点运算中的操作数之一是非正规的。非标准值是一个太小而无法表示为标准浮点值的值。"
      Case EXCEPTION_FLT_DIVIDE_BY_ZERO
         ErrStr = "线程试图将浮点值除以零的浮点除数。"
      Case EXCEPTION_FLT_INEXACT_RESULT
         ErrStr = "浮点运算的结果不能完全表示为小数。"
      Case EXCEPTION_FLT_INVALID_OPERATION
         ErrStr = "此异常表示此列表中未包含的任何浮点异常。"
      Case EXCEPTION_FLT_OVERFLOW
         ErrStr = "浮点运算的指数大于相应类型所允许的大小。"
      Case EXCEPTION_FLT_STACK_CHECK
         ErrStr = "由于浮点运算，堆栈上溢或下溢。"
      Case EXCEPTION_FLT_UNDERFLOW
         ErrStr = "浮点运算的指数小于相应类型所允许的大小。"
      Case EXCEPTION_ILLEGAL_INSTRUCTION
         ErrStr = "线程试图执行无效指令。"
      Case EXCEPTION_IN_PAGE_ERROR
         ErrStr = "该线程试图访问一个不存在的页面，系统无法加载该页面。例如，如果通过网络运行程序时网络连接丢失，可能会发生此异常。"
      Case EXCEPTION_INT_DIVIDE_BY_ZERO
         ErrStr = "线程尝试将整数值除以零的整数除数。"
      Case EXCEPTION_INT_OVERFLOW
         ErrStr = "整数运算的结果导致对结果的最高有效位进行进位。"
      Case EXCEPTION_INVALID_DISPOSITION
         ErrStr = "异常处理程序将无效处置返回给异常调度程序。使用诸如C之类的高级语言的程序员应该永远不会遇到此异常。"
      Case EXCEPTION_NONCONTINUABLE_EXCEPTION
         ErrStr = "发生不可连续的异常后，线程尝试继续执行。"
      Case EXCEPTION_PRIV_INSTRUCTION
         ErrStr = "线程试图执行一条指令，该指令在当前机器模式下是不允许的。"
      Case EXCEPTION_SINGLE_STEP
         ErrStr = "跟踪陷阱或其他单指令机制表明已执行了一条指令。"
      Case EXCEPTION_STACK_OVERFLOW
         ErrStr = "线程耗尽了其堆栈。"
      Case Else
         ErrStr = "未知"
   End Select
   bug &= "异常代码:      " & Hex(excp.ExceptionRecord->ExceptionCode) & "  " & ErrStr & vbCrLf
   bug &= "异常偏移:      " & Hex(pianyi, 8) & vbCrLf
   
   #IfDef __FB_64BIT__
      bug &= "寄存器值:      Rax=" & Hex(excp.ContextRecord->Rax)
      If excp.ContextRecord->Rax > &H10000 Then bug &= " [Rax]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rax))
      bug &= " Rbx=" & Hex(excp.ContextRecord->Rbx)
      If excp.ContextRecord->Rbx > &H10000 Then bug &= " [Rbx]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rbx))
      bug &= " Rcx=" & Hex(excp.ContextRecord->Rcx)
      If excp.ContextRecord->Rcx > &H10000 Then bug &= " [Rcx]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rcx))
      bug &= " Rdx=" & Hex(excp.ContextRecord->Rdx)
      If excp.ContextRecord->Rdx > &H10000 Then bug &= " [Rdx]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rdx))
      bug &= " Rbp=" & Hex(excp.ContextRecord->Rbp)
      If excp.ContextRecord->Rbp > &H10000 Then bug &= " [Rbp]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rbp))
      bug &= " Rsp=" & Hex(excp.ContextRecord->Rsp)
      If excp.ContextRecord->Rsp > &H10000 Then bug &= " [Rsp]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rsp))
      bug &= " Rsi=" & Hex(excp.ContextRecord->Rsi)
      If excp.ContextRecord->Rsi > &H10000 Then bug &= " [Rsi]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rsi))
      bug &= " Rdi=" & Hex(excp.ContextRecord->Rdi)
      If excp.ContextRecord->Rdi > &H10000 Then bug &= " [Rdi]=" & Hex(MEM_Read_ULongInt(GetCurrentProcessId, excp.ContextRecord->Rdi))
      bug &= " Rip=" & Hex(excp.ContextRecord->Rip) & vbCrLf
   #else
      bug &= "寄存器值:      Eax=" & Hex(excp.ContextRecord->Eax)
      If excp.ContextRecord->Eax > &H10000 Then bug &= " [Eax]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Eax))
      bug &= " Ebx=" & Hex(excp.ContextRecord->Ebx)
      If excp.ContextRecord->Ebx > &H10000 Then bug &= " [Ebx]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Ebx))
      bug &= " Ecx=" & Hex(excp.ContextRecord->Ecx)
      If excp.ContextRecord->Ecx > &H10000 Then bug &= " [Ecx]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Ecx))
      bug &= " Edx=" & Hex(excp.ContextRecord->Edx)
      If excp.ContextRecord->Edx > &H10000 Then bug &= " [Edx]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Edx))
      bug &= " Ebp=" & Hex(excp.ContextRecord->Ebp)
      If excp.ContextRecord->Ebp > &H10000 Then bug &= " [Ebp]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Ebp))
      bug &= " Esp=" & Hex(excp.ContextRecord->Esp)
      If excp.ContextRecord->Esp > &H10000 Then bug &= " [Esp]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Esp))
      bug &= " Esi=" & Hex(excp.ContextRecord->Esi)
      If excp.ContextRecord->Esi > &H10000 Then bug &= " [Esi]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Esi))
      bug &= " Edi=" & Hex(excp.ContextRecord->Edi)
      If excp.ContextRecord->Edi > &H10000 Then bug &= " [Edi]=" & Hex(MEM_Read_ULong(GetCurrentProcessId, excp.ContextRecord->Edi))
      bug &= " Eip=" & Hex(excp.ContextRecord->Eip) & vbCrLf
   #endif
   
   Dim 堆栈列表 As String, zdcc As UInteger, zdi As Long, sjStr As String, sjc As Long, zpi As Long, zk As Long = Len(UInteger) * 2, jcm As String
   Dim sp       As UInteger
   #IfDef __FB_64BIT__
      jcm = "[Rsp"
      I   = excp.ContextRecord->Rsp
      sp  = i
      While zdi < 30 And zpi < 100
         zdcc = MEM_Read_ULongInt(GetCurrentProcessId, I)
         i    += 8
   #Else
      jcm = "[Esp"
      I   = excp.ContextRecord->Esp
      sp  = i
      While zdi < 30 And zpi < 100
         zdcc = MEM_Read_ULong(GetCurrentProcessId, I)
         i    += 4
   #endif
      sjc = 0 '是不是查到模块
      zpi += 1
      if zdcc Then
         if zdcc > &H400000 Then
            For bI = 0 To UBound(gMode)
               if zdcc > Cast(UInteger, gMode(bI).modBaseAddr) And zdcc < Cast(UInteger, gMode(bI).modBaseAddr) + Cast(UInteger, gMode(bI).modBaseSize) Then
                  if Len(sjStr) Then '纯数据
                     堆栈列表 &= sjStr & vbCrLf
                     sjStr    = ""
                  End if
                  堆栈列表 &= jcm & "+" & Hex(i - sp, 4) & "]=" & Hex(zdcc, zk) & "  " & CWSTRtoString(gMode(bI).szModule) & "+" & Hex(zdcc - Cast(UInteger, gMode(bI).modBaseAddr)) & vbCrLf
                  sjc      = 1
                  zdi      += 1
                  Exit For
               End if
            Next
         End if
      End if
      if sjc = 0 Then
         If Len(sjStr) Then
            sjStr &= "," & Hex(zdcc, zk)
         Else
            sjStr = jcm & "+" & Hex(i - sp, 4) & "]=" & Hex(zdcc, zk)
         End if
      End if
   Wend
   if Len(sjStr) Then 堆栈列表 &= sjStr & vbCrLf
   
   Dim 局部列表 As String
   zdi = 9
   #IfDef __FB_64BIT__
      jcm = "[Rbp"
      I   = excp.ContextRecord->Rbp + 72
      While zdi > -121
         zdcc = MEM_Read_ULongInt(GetCurrentProcessId, i)
         i    -= 8
   #Else
      jcm = "[Ebp"
      I   = excp.ContextRecord->Ebp + 36
      While zdi > -121
         zdcc = MEM_Read_ULong(GetCurrentProcessId, i)
         i    -= 4
   #endif
      if zdi >= 0 then
         局部列表 &= jcm & "+" & Hex(zdi *Len(UInteger), 4) & "]=" & Hex(zdcc, zk) & " "
      ElseIf zdi < 0 Then
         局部列表 &= jcm & "-" & Hex(abs(zdi) *Len(UInteger), 4) & "]=" & Hex(zdcc, zk) & " "
      End if
      if (zdi Mod 10) = 0 Then 局部列表 &= vbCrLf
      zdi -= 1
      
   Wend
   
   bug &= "堆栈列表(栈顶): Esp/Rsp ------------------------------"       & vbCrLf & 堆栈列表
   bug &= "局部列表(栈底): Ebp/Rbp ------------------------------------" & vbCrLf & 局部列表
   bug &= "模块列表:   ------------------------------------------"       & vbCrLf & AllMode

   
   If Len(文件名) Then SaveFileStr 文件名, bug
   
   Function = bug
   
End Function

'[FrmMain.VEH1]事件 : 向量化异常处理（程序崩溃后处理）
'整个软件，只需一个VEH即可，在主窗口放置控件，所有窗口、模块、多线程等发生崩溃，都会跑到这里执行。
Function FrmMain_VEH1_VectExcepHandler(ByRef excp As EXCEPTION_POINTERS)As Integer
    保存软件崩溃日志 App.Path & "bug" & NowString(1) & ".txt", excp
    Return 1
End Function

'[FrmMain.mCtrlTreeList1]事件 : 鼠标右键单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'xPos yPos   当前鼠标位置，相对于屏幕。就是屏幕坐标。
Sub FrmMain_mCtrlTreeList1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    CurrentPos.x = xPos
    CurrentPos.y = yPos
    ScreenToClient hWndControl, @CurrentPos
End Sub

'[FrmMain]事件 : 窗口已经改变了大小
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'fwSizeType = SIZE_MAXHIDE     SIZE_MAXIMIZED   SIZE_MAXSHOW    SIZE_MINIMIZED    SIZE_RESTORED  
''            其他窗口最大化   窗口已最大化     其他窗口恢复    窗口已最小化      窗口已调整大小
'nWidth nHeight  是客户区大小，不是全部窗口大小。
Sub FrmMain_WM_Size(hWndForm As hWnd, fwSizeType As Long, nWidth As Long, nHeight As Long)
    If fwSizeType = SIZE_RESTORED Then
        ListView1.Width = nWidth - treMain.Width
        
    End If 
End Sub

' 窗口过程：处理消息
Function WndProc(ByVal hWnd As HWND, ByVal uMsg As UINT, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT
    Select Case uMsg
        Case WM_SIZING
            ' lParam是RECT结构体指针（拟调整的窗口矩形）
            Dim As RECT Ptr pRect = Cast(RECT Ptr, lParam)
            Dim newWidth As Integer = pRect->right - pRect->left ' 计算拟调整的宽度
            Dim newHeight As Integer = pRect->bottom - pRect->top  ' 计算拟调整的高度
            'Print "WM_SIZE"
            ' 根据拉伸方向（wParam）调整宽度
            Select Case wParam
                ' 1. 仅调整左边界（影响宽度）
                Case WMSZ_LEFT
                    ' 限制宽度在[MIN_WIDTH, MAX_WIDTH]
                    If newWidth < MIN_WIDTH Then
                        pRect->left = pRect->right - MIN_WIDTH  ' 强制左边界右移，保证最小宽度
                    'ElseIf newWidth > MAX_WIDTH Then
                    '    pRect->left = pRect->right - MAX_WIDTH  ' 强制左边界左移，保证最大宽度
                    End If
                
                ' 2. 仅调整右边界（影响宽度）
                Case WMSZ_RIGHT
                    If newWidth < MIN_WIDTH Then
                        pRect->right = pRect->left + MIN_WIDTH  ' 强制右边界左移，保证最小宽度
                    'ElseIf newWidth > MAX_WIDTH Then
                    '    pRect->right = pRect->left + MAX_WIDTH  ' 强制右边界右移，保证最大宽度
                    End If
                
                ' 3. 仅调整上边界（影响高度）
                Case WMSZ_TOP
                    If newHeight < MIN_HEIGHT Then
                        pRect->top = pRect->bottom - MIN_HEIGHT  ' 强制上边界上移，保证最小高度
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->top = pRect->bottom - MAX_HEIGHT  ' 强制上边界下移，保证最大高度
                    End If
                
                ' 4. 仅调整下边界（影响高度）
                Case WMSZ_BOTTOM
                    If newHeight < MIN_HEIGHT Then
                        pRect->bottom = pRect->top + MIN_HEIGHT  ' 强制下边界下移，保证最小高度
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->bottom = pRect->top + MAX_HEIGHT  ' 强制下边界上移，保证最大高度
                    End If
                
                ' 5. 调整左上角（同时影响宽度和高度）
                Case WMSZ_TOPLEFT
                    ' 先限制宽度
                    If newWidth < MIN_WIDTH Then
                        pRect->left = pRect->right - MIN_WIDTH
                    'ElseIf newWidth > MAX_WIDTH Then
                    '    pRect->left = pRect->right - MAX_WIDTH
                    End If
                    ' 再限制高度（需重新计算宽高，因为left可能已修改）
                    newHeight = pRect->bottom - pRect->top  ' 重新计算高度
                    If newHeight < MIN_HEIGHT Then
                        pRect->top = pRect->bottom - MIN_HEIGHT
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->top = pRect->bottom - MAX_HEIGHT
                    End If
                
                ' 6. 调整右上角（同时影响宽度和高度）
                Case WMSZ_TOPRIGHT
                    ' 先限制宽度
                    If newWidth < MIN_WIDTH Then
                        pRect->right = pRect->left + MIN_WIDTH
                    ElseIf newWidth > MAX_WIDTH Then
                        pRect->right = pRect->left + MAX_WIDTH
                    End If
                    ' 再限制高度
                    newHeight = pRect->bottom - pRect->top
                    If newHeight < MIN_HEIGHT Then
                        pRect->top = pRect->bottom - MIN_HEIGHT
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->top = pRect->bottom - MAX_HEIGHT
                    End If
                
                ' 7. 调整左下角（同时影响宽度和高度）
                Case WMSZ_BOTTOMLEFT
                    ' 先限制宽度
                    If newWidth < MIN_WIDTH Then
                        pRect->left = pRect->right - MIN_WIDTH
                    'ElseIf newWidth > MAX_WIDTH Then
                    '    pRect->left = pRect->right - MAX_WIDTH
                    End If
                    ' 再限制高度
                    newHeight = pRect->bottom - pRect->top
                    If newHeight < MIN_HEIGHT Then
                        pRect->bottom = pRect->top + MIN_HEIGHT
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->bottom = pRect->top + MAX_HEIGHT
                    End If
                
                ' 8. 调整右下角（同时影响宽度和高度）
                Case WMSZ_BOTTOMRIGHT
                    ' 先限制宽度
                    If newWidth < MIN_WIDTH Then
                        pRect->right = pRect->left + MIN_WIDTH
                    'ElseIf newWidth > MAX_WIDTH Then
                    '    pRect->right = pRect->left + MAX_WIDTH
                    End If
                    ' 再限制高度
                    newHeight = pRect->bottom - pRect->top
                    If newHeight < MIN_HEIGHT Then
                        pRect->bottom = pRect->top + MIN_HEIGHT
                    'ElseIf newHeight > MAX_HEIGHT Then
                    '    pRect->bottom = pRect->top + MAX_HEIGHT
                    End If
            End Select
            
            Return 1  ' 通知系统使用修改后的矩形
    End Select
    Return CallWindowProc(Cast(WndProc, prevFrmMainProc), hWnd, uMsg, wParam, lParam)
End Function