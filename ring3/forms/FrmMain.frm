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
Menu=刷新FrmMain_mnuCallbacks_mnuRefresh0-10-FrmMain_mnuCallbacks_Step10-10禁用回调FrmMain_mnuCallbacks_mnuControlCallback0-10移除回调FrmMain_mnuCallbacks_mnuRemoveCallback0-10-FrmMain_mnuCallbacks_mnuStep20-10查看/编辑内存FrmMain_mnuCallbacks_mnuEditMemory0-10复制FrmMain_mnuCallbacks_mnuCopy0-10{复制单格数据FrmMain_mnuCallbacks_mnuLittleCopy0-10}定位文件位置(资源浏览器)FrmMain_mnuCallbacks_mnuLocateFilePath0-10
Left=340
Top=220
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
Left=557
Top=440
Width=190
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
Menu=刷新FrmMain_mnuKernelModule_mnuRefresh0-10检测隐藏驱动FrmMain_mnuKernelModule_mnuCheckHideDriver0-10卸载驱动FrmMain_mnuKernelModule_mnuUnloadDriver0-10-FrmMain_mnuKernelModule_Step10-10查看IO派遣函数FrmMain_mnuKernelModule_mnuViewIOFunction0-10-FrmMain_mnuKernelModule_Step20-10dump到文件FrmMain_mnuKernelModule_mnuDumpToFile0-10查看/编辑内存FrmMain_mnuKernelModule_mnuEditMemory0-10复制FrmMain_mnuKernelModule_mnuCopy0-10{复制单格数据FrmMain_mnuKernelModule_mnuLittleCopy0-10}定位文件位置(资源浏览器)FrmMain_mnuKernelModule_mnuLocateFilePath0-10
Left=500
Top=220
Tag=

[PopupMenu]
Name=mnuService
Help=
Index=-1
Menu=刷新FrmMain_mnuService_mnuRefresh0-10-FrmMain_mnuService_Step10-10启动服务FrmMain_mnuService_mnuStartService0-10停止服务FrmMain_mnuService_mnuStopService0-10暂停服务FrmMain_mnuService_mnuPauseService0-10恢复服务FrmMain_mnuService_mnuContinueService0-10删除服务FrmMain_mnuService_mnuDeleteService0-10复制FrmMain_mnuService_mnuCopy0-10{复制单格数据FrmMain_mnuService_mnuLittleCopy0-10}定位文件位置FrmMain_mnuService_mnuLocateFilePath0-10
Left=420
Top=220
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
Width=350
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

[ImageList]
Name=ImageList1
Help=
Index=-1
Image=
SizeW=16
SizeH=16
DPI=True
BackColor=SYS,15
Left=580
Top=180
Tag=

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
Enabled=False
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
Enabled=False
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

[PopupMenu]
Name=mnuFilterDriver
Help=
Index=-1
Menu=刷新FrmMain_mnuFilterDriver_mnmuRefresh0-10-FrmMain_mnuFilterDriver_mnuStep10-10移除过滤设备FrmMain_mnuFilterDriver_mnuRemoveFilter0-10暴力摘除过滤设备FrmMain_mnuFilterDriver_mnuViolentRemoveFilter0-10
Left=260
Top=220
Tag=

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

[PopupMenu]
Name=mnuGDT
Help=
Index=-1
Menu=刷新FrmMain_mnuGDT_mnuRefresh0-10-FrmMain_mnuGDT_mnuStep10-10查看/编辑内存FrmMain_mnuGDT_FrmMain_mnuGDT_mnuEditMemory0-10
Left=220
Top=280
Tag=

[VEH]
Name=VEH1
Help=
Left=510
Top=170
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
NoTooltips=True
Enabled=True
Visible=False
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

[PopupMenu]
Name=mnuMain
Help=
Index=-1
Menu=显示主窗口FrmMain_mnuMain_mnuShowWindow0-10
Left=580
Top=220
Tag=

[PopupMenu]
Name=mnuRegKey
Help=
Index=-1
Menu=刷新FrmMain_mnuRegKey_mnuRefresh0-10-FrmMain_mnuRegKey_mnuStep10-10新建FrmMain_mnuRegKey_mnuCreateKey0-10删除FrmMain_mnuRegKey_mnuDeleteKey0-10重命名FrmMain_mnuRegKey_mnuRenameKey0-10-FrmMain_mnuRegKey_mnuStep20-10启用Hive分析FrmMain_mnuRegKey_mnuEnableHiveAnalysis0-10
Left=220
Top=220
Tag=

[PopupMenu]
Name=mnuListView
Help=
Index=-1
Menu=刷新FrmMain_mnuListView_mnuRefresh0-10-FrmMain_mnuListView_mnuStep10-10查看/编辑内存FrmMain_mnuListView_mnuEditMemory0-10
Left=660
Top=220
Tag=

[PopupMenu]
Name=mnuWinsockSPI
Help=
Index=-1
Menu=刷新FrmMain_mnuWinsockSPI_mnuRefresh0-10-FrmMain_mnuWinsockSPI_mnuStep10-10移除FrmMain_mnuWinsockSPI_mnuRemove0-10-FrmMain_mnuWinsockSPI_mnuStep20-10复制FrmMain_mnuWinsockSPI_mnuCopy0-10{复制整行数据FrmMain_mnuWinsockSPI_mnuCopyRowData0-10}
Left=700
Top=220
Tag=

[PopupMenu]
Name=mnuKernelThread
Help=
Index=-1
Menu=刷新FrmMain_mnuKernelThread_mnuRefresh0-10-FrmMain_mnuKernelThread_mnuStep10-10结束线程FrmMain_mnuKernelThread_mnuKillThread0-10挂起线程FrmMain_mnuKernelThread_mnuSuspendThread0-10恢复线程FrmMain_mnuKernelThread_mnuResumeThread0-10-FrmMain_mnuKernelThread_mnuStep20-10查看线程栈FrmMain_mnuKernelThread_mnuViewThreadStack0-10
Left=540
Top=220
Tag=

[PopupMenu]
Name=mnuColumn
Help=
Index=-1
Menu=
Left=620
Top=220
Tag=

[PopupMenu]
Name=mnuMinifilter
Help=
Index=-1
Menu=刷新FrmMain_mnuMinifilter_mnuRefresh0-10-FrmMain_mnuMinifilter_mnuStep10-10查看绑定实例FrmMain_mnuMinifilter_mnuViewInstance0-10移除过滤器FrmMain_mnuMinifilter_mnuRemoveFilter0-10暴力摘除过滤器FrmMain_mnuMinifilter_mnuViolentRemoveFilter0-10
Left=300
Top=220
Tag=

[PopupMenu]
Name=mnuIDT
Help=
Index=-1
Menu=刷新FrmMain_mnuIDT_mnuRefresh0-10-FrmMain_mnuIDT_mnuStep10-10查看/编辑内存FrmMain_mnuIDT_mnuEditMemory0-10
Left=260
Top=280
Tag=

[PopupMenu]
Name=mnuWfpCallout
Help=
Index=-1
Menu=刷新FrmMain_mnuWfpCallout_mnuRefresh0-10查看/编辑内存FrmMain_mnuWfpCallout_mnuEditMemory0-10
Left=340
Top=280
Tag=

[PopupMenu]
Name=mnuWfpFilter
Help=
Index=-1
Menu=刷新FrmMain_mnuWfpFilter_mnuRefresh0-10
Left=380
Top=280
Tag=

[PopupMenu]
Name=mnuFolder
Help=
Index=-1
Menu=刷新FrmMain_mnuFolder_mnuRefresh0-10-FrmMain_mnuFolder_mnuStep10-10新建FrmMain_mnuFolder_mnuCreateFolder0-10复制FrmMain_mnuFolder_mnuCopyFolder0-10强制复制到...FrmMain_mnuFolder_mnuForceCopyTo0-10删除FrmMain_mnuFolder_mnuDeleteFolder0-10强制删除FrmMain_mnuFolder_mnuForceDeleteFolder0-10-FrmMain_mnuFolder_mnuStep20-10开启物理磁盘分析FrmMain_mnuFolder_mnuEnablePhysicalAnalyze0-10
Left=300
Top=280
Tag=

[PopupMenu]
Name=mnuRegValue
Help=
Index=-1
Menu=刷新FrmMain_mnuRegValue_mnuRefresh0-10-FrmMain_mnuRegValue_mnuStep10-10新建FrmMain_mnuRegValue_mnuCreate0-10{新建字符串值FrmMain_mnuRegValue_mnuCreateString0-10}删除FrmMain_mnuRegValue_mnuDeleteValue0-10修改FrmMain_mnuRegValue_mnuModifyValue0-10
Left=420
Top=280
Tag=

[PopupMenu]
Name=mnuTaskScheduler
Help=
Index=-1
Menu=刷新FrmMain_mnuTaskScheduler_mnuRefresh0-10-FrmMain_mnuTaskScheduler_mnuStep10-10启用FrmMain_mnuTaskScheduler_mnuEnable0-10运行FrmMain_mnuTaskScheduler_mnuRun0-10删除FrmMain_mnuTaskScheduler_mnuDelete0-10
Left=460
Top=280
Tag=

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
Menu=刷新FrmMain_mnuFile_mnuRefresh0-10新建FrmMain_mnuFile_mnuCreateFile0-10复制到FrmMain_mnuFile_mnuCopyTo0-10强制复制到...FrmMain_mnuFile_mnuForceCopyTo0-10删除FrmMain_mnuFile_mnuDelete0-10强制删除FrmMain_mnuFile_mnuForceDelete0-10-FrmMain_mnuFile_mnuStep10-10查看文件流FrmMain_mnuFile_mnuViewFileStream0-10-FrmMain_mnuFile_mnuStep20-10复制文字FrmMain_mnuFile_mnuCopyStr0-10{复制单格数据FrmMain_mnuFile_mnuLittleCopy0-10}
Left=460
Top=220
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
Menu=文件FrmMain_TopMenu1_mnuFile0-10{查看文件占用FrmMain_TopMenu1_mnuUnlockFile0-10以管理员身份运行FrmMain_TopMenu1_mnuRunasAdmin0-10}网络FrmMain_TopMenu1_mnuWeb000{防火墙FrmMain_TopMenu1_mnuFireWall000}高级FrmMain_TopMenu1_mnuAdvanced0-10{显示日志FrmMain_TopMenu1_mnuViewLog0-10检测更新FrmMain_TopMenu1_mnuCheckUpdate0-10}
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
Menu=结束进程FrmMain_mnuProcess_mnuTerminateProcess0-10强制结束进程FrmMain_mnuProcess_mnuForceTerminateProcess0-10暴力结束进程FrmMain_mnuProcess_mnuViolentTerminateProcess0-10挂起进程FrmMain_mnuProcess_mnuSuspendProcess0-10恢复进程FrmMain_mnuProcess_mnuResumeProcess0-10保护进程FrmMain_mnuProcess_mnuProtectProcess0-10检测隐藏进程FrmMain_mnuProcess_mnuCheckHideProcess0-10校验数字签名FrmMain_mnuProcess_mnuCheckSign0-10校验所有数字签名FrmMain_mnuProcess_mnuCheckAllSign0-10刷新FrmMain_mnuProcess_mnuRefresh0-10-FrmMain_mnuProcess_mnuStep1000注入DLLFrmMain_mnuProcess_mnuInjectDll0-10-FrmMain_mnuProcess_mnuStep20-10查看模块FrmMain_mnuProcess_mnuViewModule0-10查看线程FrmMain_mnuProcess_mnuViewThread0-10查看内存FrmMain_mnuProcess_mnuViewMemory0-10查看...FrmMain_mnuProcess_mnuView0-10{查看窗口列表FrmMain_mnuProcess_mnuViewWindowList0-10查看句柄列表FrmMain_mnuProcess_mnuViewHandleList0-10查看网络连接列表FrmMain_mnuProcess_mnuViewWebConnect0-10查看特权FrmMain_mnuProcess_mnuViewPrivilege0-10查看HookFrmMain_mnuProcess_mnuViewHook0-10查看进程定时器FrmMain_mnuProcess_mnuViewProcessTimer0-10查看消息钩子FrmMain_mnuProcess_mnuViewMsgHook0-10查看事件钩子FrmMain_mnuProcess_mnuViewEventHook0-10查看热键FrmMain_mnuProcess_mnuViewHotkey0-10查看其他信息FrmMain_mnuProcess_mnuViewOtherInfo0-10}-FrmMain_mnuProcess_mnuStep30-10复制FrmMain_mnuProcess_mnuCopy0-10{复制单格FrmMain_mnuProcess_mnuLittleCopy0-10}定位文件位置(资源浏览器)FrmMain_mnuProcess_mnuLocateFilePath0-10
Left=380
Top=220
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
OwnerData=True
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
Visible=False
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

Enum UI_LAYOUT_MODE
    LAYOUT_LIST_ONLY          ' 仅右侧主视图
    LAYOUT_LIST_TREE          ' TreeView + 主视图
    LAYOUT_NONE               ' 设置页
End Enum

Enum UI_MAIN_VIEW
    VIEW_LISTVIEW
    VIEW_TREELIST             ' mCtrlTreeList1
End Enum

' ===================== 状态机定义 =====================
Enum TooltipState
    State_Idle           ' 空闲：鼠标不在控件上
    State_Hovering       ' 悬停中：鼠标在控件上，等待显示
    State_Showing        ' 显示中：Tooltip 已显示
End Enum

Type SIGNVERIFY_MSG
    ctrlType As Wstring * 20
    rowIndex As Long
    ForeColor As COLORREF
    BackColor As COLORREF
    
    hListView As hWnd        ' 新增：目标ListView句柄
    InstanceId As ULong      ' 新增：目标ListView唯一ID
End Type

' ===================== 全局变量 =====================
Dim Shared g_TooltipState As TooltipState = State_Idle  ' 当前状态
Dim Shared g_hTooltip As HWND                             ' 自定义 Tooltip 窗口句柄
Dim Shared g_sTooltipText As StringW
Dim Shared g_nHoverX As Long                              ' 悬停时的X坐标
Dim Shared g_nHoverY As Long                              ' 悬停时的Y坐标
Dim Shared g_hLastHoverItem As MC_HTREELISTITEM          ' 最后悬停的节点
Dim Shared g_hWndForm As hWnd                             ' 窗口句柄
Dim Shared g_hTreeControl As hWnd                         ' 控件句柄

Dim Shared gLayoutMode As UI_LAYOUT_MODE
Dim Shared gMainView   As UI_MAIN_VIEW

Const MAX_LEN = 200
Const MIN_WIDTH = 350
Const MIN_HEIGHT = 500
Const MAX_WIDTH = 350
Const MAX_HEIGHT = 500
Const GWL_WNDPROC = -4

#define WM_SignVerify (WM_USER + &H100)
#define WM_TREEVIEW_DBLCLK WM_USER + &H200

'Type WINDOWPROC As Function(ByVal As HWND, ByVal As UINT, ByVal As WPARAM, ByVal As LPARAM) As LRESULT

Dim Shared CurrentInformation As CURRENT_INFORMATION
'Dim Shared CurrentListViewItem(Any, Any) As ListViewItemInfo
Dim Shared CurrentPos As Point
Dim Shared CurrentNode As HTREEITEM
'Dim Shared hook As HHOOK
Dim Shared hDrv As HANDLE
Dim Shared IsHaveSelectItem As Boolean
Dim Shared IsDriverLoaded As BOOLEAN = False
Dim Shared PIDs() As DWORD
Dim Shared FilePathByUnlock As String
Dim Shared prevFrmMainProc As LONG_PTR = NULL

Dim Shared ListView_Height As Long
Dim Shared bFrmMainShowed As Boolean = False

' 在窗体 WM_CREATE 或 Load 事件中调用
Sub InitCustomTooltip(ByVal hWndParent As hWnd)
    g_hTooltip = CreateWindowEx( _
        WS_EX_TOPMOST, _
        TOOLTIPS_CLASS, _
        "", _
        WS_POPUP Or TTS_NOPREFIX Or TTS_ALWAYSTIP, _
        0, 0, 0, 0, _
        hWndParent, _
        NULL, _
        GetModuleHandle(NULL), _
        NULL _
    )

    If g_hTooltip = 0 Then Exit Sub

    Dim ti As TOOLINFOW
    ZeroMemory(@ti, SizeOf(TOOLINFOW))
    ti.cbSize = SizeOf(TOOLINFOW)
    ti.uFlags = TTF_IDISHWND Or TTF_TRACK Or TTF_ABSOLUTE
    ti.hWnd = hWndParent
    ti.uId = Cast(UINT_PTR, hWndParent)
    ti.lpszText = NULL ' StrPtr("")

    SendMessage g_hTooltip, TTM_ADDTOOL, 0, Cast(LPARAM, @ti)

    SendMessage g_hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 400
    SendMessage g_hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 5000
End Sub

' 仅对需要防闪烁的控件禁用重绘，且用 SetWindowPos 替代 Visible 属性
Sub BeginControlUpdate()
    ' 只禁用会频繁变化的控件，且始终可见的 treMain 不禁用
    SendMessage ListView1.hWnd, WM_SETREDRAW, False, 0
    SendMessage mCtrlTreeList1.hWnd, WM_SETREDRAW, False, 0
    SendMessage TreeView.hWnd, WM_SETREDRAW, False, 0
    ' txtFilePath 通常尺寸固定，不需要禁用
End Sub

Sub EndControlUpdate()
    ' 恢复重绘
    SendMessage ListView1.hWnd, WM_SETREDRAW, True, 0
    SendMessage mCtrlTreeList1.hWnd, WM_SETREDRAW, True, 0
    SendMessage TreeView.hWnd, WM_SETREDRAW, True, 0
    
    ' 只刷新当前可见的控件，避免不必要的绘制
    If ListView1.Visible Then
        InvalidateRect ListView1.hWnd, NULL, True
    End If
    If mCtrlTreeList1.Visible Then
        InvalidateRect mCtrlTreeList1.hWnd, NULL, True
    End If
    If TreeView.Visible Then
        InvalidateRect TreeView.hWnd, NULL, True
    End If
    
    ' 统一更新，减少闪烁
    UpdateWindow FrmMain.hWnd
End Sub

' 不要调用BeginLayoutUpdate和EndLayoutUpdate,否则会导致诡异现象:ListView被mCtrlTreeList覆盖
Sub UpdateLayout()
    #define FILEPATH_HEIGHT 40
    Dim rc As RECT
    GetClientRect FrmMain.hWnd, @rc

    Dim cw As Long = rc.right
    Dim ch As Long = rc.bottom

    BeginControlUpdate
    'InitializeListView ListView1, mCtrlTreeList1, TreeView
    ' ===== txtFilePath 可见性（唯一来源）=====
    Select Case gLayoutMode
    Case LAYOUT_LIST_TREE
        txtFilePath.Visible = True
        SetWindowZ txtFilePath.hWnd
    Case Else
        txtFilePath.Visible = False
    End Select
    TreeView.Visible = False
    ListView1.Visible = False
    mCtrlTreeList1.Visible = False

    ' ===== 左栏 =====
    treMain.Left = 0
    treMain.Top = 0
    treMain.Height = ch

    ' ===== 底部栏 =====
    TxtFilter2.Left = treMain.Width
    TxtFilter2.Width = cw - treMain.Width - lblNum.Width - 10
    TxtFilter2.Top = ch - TxtFilter2.Height
    lblNum.Left = TxtFilter2.Left + TxtFilter2.Width + 5
    lblNum.Top = TxtFilter2.Top

    ' ===== 顶部偏移 =====
    Dim topOffset As Long
    If gLayoutMode = LAYOUT_LIST_TREE Then
        txtFilePath.Left = treMain.Width
        txtFilePath.Top = 0
        txtFilePath.Width = cw - treMain.Width
        txtFilePath.Height = FILEPATH_HEIGHT   ' ★ 关键
        topOffset = txtFilePath.Height
    Else
        topOffset = 0
    End If

    Dim mainHeight As Long
    mainHeight = ch - TxtFilter2.Height - topOffset

    ' ===== 主体布局 =====
    Select Case gLayoutMode

    Case LAYOUT_LIST_TREE
        TreeView.Visible = True
        SetWindowZ TreeView.hWnd
        TreeView.Left = treMain.Width
        TreeView.Top = topOffset
        TreeView.Width = (cw - treMain.Width) \ 2
        TreeView.Height = mainHeight

        ListView1.Visible = True
        SetWindowZ ListView1.hWnd

        ListView1.Left = TreeView.Left + TreeView.Width
        ListView1.Top = topOffset
        ListView1.Width = cw - ListView1.Left
        ListView1.Height = mainHeight
        
        mCtrlTreeList1.Visible = False

    Case LAYOUT_LIST_ONLY
        TreeView.Visible = False
        ListView1.Visible = (gMainView = VIEW_LISTVIEW)
        mCtrlTreeList1.Visible = Not ListView1.Visible

        If ListView1.Visible Then
            ListView1.Left = treMain.Width
            ListView1.Top = topOffset
            ListView1.Width = cw - treMain.Width
            ListView1.Height = mainHeight
            SetWindowZ ListView1.hWnd
        Else
            mCtrlTreeList1.Left = treMain.Width
            mCtrlTreeList1.Top = topOffset
            mCtrlTreeList1.Width = cw - treMain.Width
            mCtrlTreeList1.Height = mainHeight
            SetWindowZ mCtrlTreeList1.hWnd
        End If

    Case LAYOUT_NONE
        TreeView.Visible = False
        ListView1.Visible = False
        mCtrlTreeList1.Visible = False
        
        FmeAdvance.Left = treMain.Width
        FmeAdvance.Top = topOffset
        FmeAdvance.Width = cw - treMain.Width
        FmeAdvance.Height = mainHeight

    End Select

    EndControlUpdate
End Sub

Function TreeView_GetClickedItem(hWndControl As HWND) As HTREEITEM
    Dim pt As Point
    Dim hti As TVHITTESTINFO
    Dim hClickedItem As HTREEITEM
    
    ' 获取当前鼠标位置 (屏幕坐标)
    GetCursorPos @pt
    
    ' 转换为 TreeView 客户区坐标
    ScreenToClient hWndControl, @pt
    
    ' 准备 HitTest 信息
    hti.pt = pt
    
    ' 发送 TVM_HITTEST 消息获取点击的节点
    Return TreeView_HitTest(hWndControl, @hti)
End Function

Private Sub DrawTreeView()
    Dim treKernel As HTREEITEM, treHardDisk As HTREEITEM, treOther As HTREEITEM, treKernelCallbacks As HTREEITEM
    treMain.AddItem NULL, "前台进程"
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
    treKernelCallbacks = treMain.AddItem(treKernel, "内核回调")
    treMain.AddItem treKernelCallbacks, "Callbacks"
    treMain.AddItem treKernelCallbacks, "KernelTimer"
    treMain.AddItem treKernelCallbacks, "WorkItemThread"
    treMain.AddItem treKernelCallbacks, "Minifilter"
    treMain.AddItem treKernelCallbacks, "过滤驱动"
    treMain.AddItem treKernelCallbacks, "WfpCallout"
    treMain.AddItem treKernelCallbacks, "WfpFilter"
    treMain.AddItem treKernelCallbacks, "Ndis"
    treMain.AddItem treKernel, "对象目录"
    treHardDisk = treMain.AddItem(NULL, "硬盘")
    treMain.AddItem treHardDisk, "文件"
    treMain.AddItem NULL, "注册表"
    treOther = treMain.AddItem(NULL, "其他")
    treMain.AddItem treOther, "服务"
    treMain.AddItem treOther, "启动项"
    treMain.AddItem treOther, "Etw"
    treMain.AddItem treOther, "Etw Provider"
    treMain.AddItem treOther, "Winsock SPI"
    treMain.AddItem treOther, "任务计划"
    Dim treAdvance As HTREEITEM = treMain.AddItem(NULL, "高级")
    treMain.AddItem treAdvance, "暴力检测"
    treMain.AddItem treAdvance, "设置"
End Sub

Sub FrmMain_Shown(hWndForm As hWnd, UserData As Integer)
    ' ========== 1. 解析参数 ==========
    'SetConsoleOutputCP(CP_UTF8)
    ProceedCommandLine Command(1)
    
    ' ========== 2. 如果是 CLI 模式，隐藏窗口并进入 REPL ==========
    If g_CuiMode Then
        ShowWindow(FrmMain.hWnd, SW_HIDE) ' 隐藏主窗口，但控件依然存在可当数据池
        
        ' 必须保留的核心初始化
        AdjustPrivilege GetCurrentProcessId, SE_DEBUG_NAME, True
        AdjustPrivilege GetCurrentProcessId, SE_LOAD_DRIVER_NAME, True
        AdjustPrivilege GetCurrentProcessId, SE_SHUTDOWN_NAME, True
        
        ' 初始化必要的控件底层结构
        ListView_SetExtendedListViewStyleEx(ListView1.hWnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
        InitNtUserFunction
        InitAllModuleCache
        InitThreadPool
        InitLog
        
        SymEngine_Init
        ' 启动 REPL 阻塞循环
        If g_InAgentMode Then RunAgentPipeRepl Else RunAgentRepl
        Exit Sub ' 保险，正常情况 End 在 REPL 内部执行
    End If
    'ChangeWindowMessageFilter(0x0049, MSGFLT_ADD)'允许拖放文件
    AdjustPrivilege GetCurrentProcessId, SE_DEBUG_NAME, True
    AdjustPrivilege GetCurrentProcessId, SE_LOAD_DRIVER_NAME, True
    AdjustPrivilege GetCurrentProcessId, SE_SHUTDOWN_NAME, True
    DrawTreeView
    FrmListView.Hide
    ListView_SetExtendedListViewStyleEx(ListView1.hWnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
    TreeView_SetExtendedStyle(TreeView.hWnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
    TreeView_SetExtendedStyle(mCtrlTreeList1.hWnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER)
    
    Dim sfi As SHFILEINFO
    Dim hSysImageList As HIMAGELIST = Cast(HIMAGELIST, SHGetFileInfo( _
        App.Path, _                          ' 任意存在的路径
        0, _                               ' 文件属性
        @sfi, _                            ' SHFILEINFO 结构
        SizeOf(SHFILEINFO), _              ' 结构大小
        SHGFI_SYSICONINDEX Or SHGFI_SMALLICON _  ' 【关键标志】
    ))
    ListView_SetImageList(ListView1.hWnd, hSysImageList, LVSIL_SMALL)
    
    gLayoutMode = LAYOUT_LIST_ONLY
    gMainView = VIEW_LISTVIEW
    UpdateLayout
    
    'CurrentInformation.hListView = Me.hWnd
    'CurrentInformation.intType = Process
    InitializeListView Process, ListView1
    CurrentInformation.intType = -1
    
    TxtFilter2.WindowsZ HWND_TOP
    TrayIco1.Create
    AddRButtonMenu
    ReDim Preserve CurrentInformationArray(10) As CURRENT_INFORMATION
    ' 移除根项线条样式
    SendMessage mCtrlTreeList1.hWnd, TVM_SETEXTENDEDSTYLE, 0, _
    SendMessage(mCtrlTreeList1.hWnd, TVM_GETEXTENDEDSTYLE, 0, 0) And (Not TVS_LINESATROOT)
    If Command(1) = "-LoadDriver" And IsAdmin Then
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!"
        End If
    /'ElseIf Command(1) = "-DeleteFile" Then
        AfxMsg Command(2)
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Exit Sub
            Dim ustrFilePath As WString * MAX_PATH = "\??\" & Command(2)
            #define STATUS_CANNOT_DELETE Cast(NTSTATUS, &HC0000121)
            Dim status As NTSTATUS = MyDeleteFile(@ustrFilePath)
            Print "MyDeleteFile:status=" & WHex(status)
            If status = STATUS_CANNOT_DELETE Then ' 是正在运行的可执行文件
                IoControl hDrv, IOCTL_DeleteFileByIRP, @ustrFilePath, MAX_PATH * SizeOf(Wstring)
            Else ' 是被打开的文件
                IoControl hDrv, IOCTL_DeleteFileByXCB, @ustrFilePath, MAX_PATH * SizeOf(Wstring)
                DeleteFile ustrFilePath
                Print "[DeleteFile]GetLastError:" & WinErrorMsg(GetLastError) & GetLastError
            End If
            AfxMsg "粉碎成功!"
        End If
    ElseIf Command(1) = "-UnlockFile" Then
        If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
            If LoadDriver(App.Path & "SnowSword.sys", False) Then
                hDrv = OpenDrv("\\.\\SnowSword", True)
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    SymbolService_StartWorkers
                    AfxMsg "加载成功!"
                    Check6.Value =True
                    IsDriverLoaded = True
                    FilePathByUnlock = Command(2)
                    Print "接收到文件路径:" & FilePathByUnlock
                    FrmListView.Show ,, UnlockTheFile
                Else
                    AfxMsg "加载失败!"
                End If
            Else
                AfxMsg "加载失败!"
            End If
        End If'/
    End If
    
    InitLog
    InitNtUserFunction
    InitAllModuleCache
    'MyLog.PrintLog GetSystemVersion
    'SetMenuText mnuProcess, FrmMain_mnuProcess_mnuTerminateProcess, "1"
    'If SymInit(GetCurrentProcess) = True Then QuerySymbol Cast(PULONG64, &HFFFFF8011B8F0000), NULL
    prevFrmMainProc = SetWindowLongPtr(FrmMain.hWnd, GWL_WNDPROC, Cast(LONG_PTR, @WNDPROC))
    InitCustomTooltip hWndForm
    'Dim bytData() As Byte
    'If Not ReadFile2("C:\WINDOWS\System32\config\SOFTWARE", bytData()) Then AfxMsg "读取失败!"
    
    
    InitThreadPool
    'CheckUpdate
    bFrmMainShowed = True
    
    SymEngine_Init
    
    'Print SizeOf(TRACE_PROVIDER_INSTANCE_INFO)
End Sub

'[Form1.ListView1]事件 : 鼠标右键单击
Sub FrmMain_ListView1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    ' ===================== 新增：判断是否点击【列标题】 =====================
    Dim hdrHWND As HWND = ListView_GetHeader(hWndControl) ' 获取列头控件句柄
    Dim clickOnHeader As Boolean = False
    Dim clickedColumn As Long = -1

    If hdrHWND <> NULL Then
        Dim hdrRect As RECT
        Dim hdrTest As HDHITTESTINFO
        ' 把屏幕鼠标坐标 转为 列头控件的客户区坐标
        hdrTest.pt.x = xPos
        hdrTest.pt.y = yPos
        ScreenToClient hdrHWND, @hdrTest.pt
        ' 测试点击位置是否在列头上
        SendMessage hdrHWND, HDM_HITTEST, 0, Cast(lParam, @hdrTest)
        ' 判断：有效点击列头 + 获取列索引
        If (hdrTest.flags And HHT_ONHEADER) <> 0 And hdrTest.iItem >= 0 Then
            clickOnHeader = True
            clickedColumn = hdrTest.iItem
        End If
    End If

    ' ============== 核心逻辑：点击列头 → 动态构建列选择菜单 ==============
    If clickOnHeader = True Then
        Dim intType As Interface = CurrentInformation.intType
        Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(intType)
        If cfg->Count > 0 Then
            ' 1) 清空旧菜单项
            DeleteAllMenu mnuColumn
            
            ' 2) 动态添加每列菜单项，使用 IDM_SELECT_COLUMN_BASE + 列索引 作为命令ID
            Dim i As Long
            For i = 0 To cfg->Count - 1
                Dim itemId As ULong = IDM_SELECT_COLUMN_BASE + i
                mnuColumn.AddMenu 0, *cfg->Names(i), itemId, "", -1, 0, 0, ""
                SetMenuCheckState mnuColumn, itemId, cfg->Visible(i)
            Next
            
            ' 3) 显示弹出菜单
            PopupMenu hWndForm, mnuColumn.HMENU
        End If
        Exit Sub
    End If
    ' ======================================================================
    ' ===================== 原有逻辑：点击列表内容，正常弹出菜单 =====================
    Dim lvinfo As LVHITTESTINFO
    lvinfo.pt.x = xPos
    lvinfo.pt.y = yPos
    ScreenToClient hWndControl, @lvinfo.pt
    ListView_SubItemHitTest(hWndControl, @lvinfo)
    If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
    (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
        LastClickedItem = lvinfo.iItem
        LastClickedSubItem = lvinfo.iSubItem
    End If
    
    Select Case CurrentInformation.intType
        Case Process
            PopupMenu hWndForm,mnuProcess.HMENU
        Case File
            PopupMenu hWndForm, mnuFile.HMENU
        Case KernelModule
            'Print "DriverObject:" & ListView1.GetItemText(LastClickedItem, 5)
            SetMenuStatus mnuKernelModule, FrmMain_mnuKernelModule_mnuViewIOFunction, (ListView1.GetItemText(LastClickedItem, 5) <> "0x0")
            PopupMenu hWndForm, mnuKernelModule.HMENU
        Case KernelThread, WorkItemThread
            PopupMenu hWndForm, mnuKernelThread.HMENU
        Case Callbacks
            PopupMenu hWndForm, mnuCallbacks.HMENU
        Case Registry
            PopupMenu hWndForm, mnuRegValue.HMENU
        Case Service
            PopupMenu hWndForm, mnuService.HMENU
        Case WinsockSPI
            PopupMenu hWndForm, mnuWinsockSPI.HMENU
        Case TaskScheduler
            PopupMenu hWndForm, mnuTaskScheduler.HMENU
        Case WfpCallout
        Case WfpFilter
        Case Else
            PopupMenu hWndForm, mnuListView.HMENU
    End Select
End Sub

' ==============================================
' 改写完成：TreeView双击切换模块（自动分类型保存/恢复缓存）
' ==============================================
Sub FrmMain_treMain_WM_LButtonDblclk(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    'Print "FrmMain_treMain_WM_LButtonDblclk"
    Dim treSelect As HTREEITEM = treMain.HitTest(xPos, yPos)
    If treMain.GetChild(treSelect) <> NULL Then Exit Sub ' 父节点直接滚，不碰任何逻辑
    
    Dim SelectText As String = treMain.Text(treSelect)
    txtFilePath.Text = ""
    'Print "进入FrmMain_treMain_WM_LButtonDblclk"
    ' ===================== 核心：切换前 → 保存【当前模块】状态（自动识别控件类型） =====================
    If CurrentInformation.intType >= 0 Then
        Select Case gLayoutMode
            Case LAYOUT_LIST_TREE
                ' 场景1：TreeView + ListView 双控件 → 保存两个控件
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
                SaveCurrentListViewState ListView1, CurrentInformation.intType
            Case LAYOUT_LIST_ONLY
                ' 场景2：仅单控件 → 判断是ListView还是TreeList
                If gMainView = VIEW_LISTVIEW Then
                    SaveCurrentListViewState ListView1, CurrentInformation.intType
                ElseIf gMainView = VIEW_TREELIST Then
                    SaveCurrentTreeListState mCtrlTreeList1, CurrentInformation.intType
                End If
        End Select
    End If
    ' ====================================================================================
    'Print "开始切换功能模块"
    ' ===================== 模块切换逻辑（完全保留你的原有代码，仅补全缓存恢复） =====================
    If SelectText = "前台进程" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = ForegroundProcess
        InitializeListView ForegroundProcess, ListView1
        lblNum.Caption = "正在获取..."
        
        ' ListView 恢复缓存
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetVisibleProcessList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "进程" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = Process
        InitializeListView Process, ListView1
        RestoreColumnVisibility Process, ListView1.hWnd   ' 恢复用户隐藏的列
        lblNum.Caption = "正在获取..."
        'AfxMsg "1"
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            'Dim As Double start, finish
            'start = Timer
            GetProcessList ListView1, GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            'finish = Timer
            'MyLog.PrintLog LOG_INFO,,, "函数执行耗时：" & finish - start & " 秒"
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "内核模块" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = KernelModule
        InitializeListView KernelModule, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetKernelModuleList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ElseIf SelectText = "内核线程" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = KernelThread
        InitializeListView KernelThread, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetKernelThreadList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "SSDT" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = SSDT
        InitializeListView SSDT, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetSSDT(ListView1)
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "Shadow SSDT" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = ShadowSSDT
        InitializeListView ShadowSSDT, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetSSSDT(ListView1)
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "GDT" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = GDT
        InitializeTreeList GDT, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        ' TreeList 恢复缓存
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            lblNum.Caption = "数量:" & WStr(GetGDT(mCtrlTreeList1))
        End If

    ElseIf SelectText = "IDT" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = IDT
        InitializeTreeList IDT, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            lblNum.Caption = "数量:" & WStr(GetIDT(mCtrlTreeList1))
        End If
        
    ElseIf SelectText = "Etw" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = Etw
        InitializeTreeList Etw, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        ' TreeList 恢复缓存
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            lblNum.Caption = "数量:" & WStr(EnumETWSession(mCtrlTreeList1))
        End If

    ElseIf SelectText = "HalDispatch" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = HalDispatch
        InitializeListView HalDispatch, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetHalDispatchTable(ListView1)
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "HalPrivateDispatch" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = HalPrivateDispatch
        InitializeListView HalPrivateDispatch, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetHalPrivateDispatchTable(ListView1)
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "Object Hook" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = ObjectHook
        InitializeListView ObjectHook, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetObjectInfo ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "Callbacks" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = Callbacks
        InitializeListView Callbacks, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetCallbackList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "KernelTimer" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = KernelTimer
        InitializeListView KernelTimer, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetKernelTimerList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "WorkItemThread" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = WorkItemThread
        InitializeListView WorkItemThread, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            GetWorkItemThreadList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "Minifilter" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = Minifilter
        InitializeTreeList Minifilter, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            lblNum.Caption = "数量:" & WStr(EnumMinifilter(mCtrlTreeList1))
        End If

    ElseIf SelectText = "过滤驱动" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = FilterDriver
        InitializeTreeList FilterDriver, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            lblNum.Caption = "数量:" & WStr(EnumAttachDevices(mCtrlTreeList1))
        End If
        
    ElseIf SelectText = "Ndis" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_TREELIST
        UpdateLayout
        
        CurrentInformation.intType = Ndis
        InitializeTreeList Ndis, mCtrlTreeList1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeListFromCache mCtrlTreeList1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            lblNum.Caption = "数量:" & WStr(EnumNdisMiniport(mCtrlTreeList1))
        End If

    ElseIf SelectText = "WfpCallout" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = WfpCallout
        InitializeListView WfpCallout, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            If GetSystemVersion <> "Windows 11 24H2" AndAlso GetSystemVersion <> "Windows 10 22H2" AndAlso AfxMsg("暂不支持的版本,是否执意继续?",, MB_YESNO) = IDNO Then Exit Sub
            GetWfpCalloutList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "WfpFilter" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = WfpFilter
        InitializeListView WfpFilter, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            If GetSystemVersion <> "Windows 11 24H2" AndAlso GetSystemVersion <> "Windows 10 22H2" AndAlso AfxMsg("暂不支持的版本,是否执意继续?",, MB_YESNO) = IDNO Then Exit Sub
            GetWfpFilterList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ' ===================== 特殊场景：TreeView + ListView 双控件（文件/注册表） =====================
    ElseIf SelectText = "文件" Then
        gLayoutMode = LAYOUT_LIST_TREE
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = File
        InitializeListView File, ListView1
        InitializeTreeView TreeView
        lblNum.Caption = "正在获取..."
        
        ' 双控件：恢复TreeView + ListView
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeViewFromCache TreeView, CurrentInformation.intType
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetDriveList TreeView
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "注册表" Then
        gLayoutMode = LAYOUT_LIST_TREE
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = Registry
        InitializeListView Registry, ListView1
        InitializeTreeView TreeView
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeViewFromCache TreeView, CurrentInformation.intType
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetRootList TreeView
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        
    ElseIf SelectText = "对象目录" Then
        gLayoutMode = LAYOUT_LIST_TREE
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = ObjectDirectory
        InitializeListView ObjectDirectory, ListView1
        InitializeTreeView TreeView
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreTreeViewFromCache TreeView, CurrentInformation.intType
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetObjectRootList TreeView
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
    ' ====================================================================================

    ElseIf SelectText = "服务" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = Service
        InitializeListView Service, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetServiceList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "Etw Provider" Then
        /'gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = EtwProvider
        InitializeListView EtwProvider, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetETWProviderList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)'/

    ElseIf SelectText = "Winsock SPI" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = WinsockSPI
        InitializeListView WinsockSPI, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetWinsockSPIList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "任务计划" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = TaskScheduler
        InitializeListView TaskScheduler, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            GetTaskSchedulerList ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "暴力检测" Then
        gLayoutMode = LAYOUT_LIST_ONLY
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        
        CurrentInformation.intType = ViolentCheck
        InitializeListView ViolentCheck, ListView1
        lblNum.Caption = "正在获取..."
        
        If g_ViewCache(CurrentInformation.intType).IsCached Then
            RestoreListViewFromCache ListView1, CurrentInformation.intType
        Else
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            ScanKernelMemoryMultiThread ListView1
        End If
        lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)

    ElseIf SelectText = "设置" Then
        gLayoutMode = LAYOUT_NONE
        gMainView = VIEW_LISTVIEW
        UpdateLayout
        Exit Sub
    End If
End Sub

'[FrmMain]事件 : 即将关闭窗口，返回非0可阻止关闭
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
Function FrmMain_WM_Close(hWndForm As hWnd) As LResult
    CloseDrv hDrv
    SymbolService_StopWorkers
    SymEngine_Cleanup
    UnloadDriver "SnowSword", False
    
    TrayIco1.Del
    UninitNtUserFunction
    SetWindowLongPtr FrmMain.hWnd, GWL_WNDPROC, prevFrmMainProc
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
Function FrmMain_TreeView_NM_DBLCLK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    Dim hClickedItem As HTREEITEM = TreeView_GetClickedItem(hWndControl)
    If TreeView.Expand(hClickedItem) Then
        TreeView.ExpandEx hClickedItem, TVE_COLLAPSE
        'DeleteChildItem TreeView, CurrentNode
    ElseIf TreeView.GetChild(hClickedItem) = NULL Then
        TreeView.ExpandEx hClickedItem, TVE_EXPAND
        Dim currentPath As StringW
        Select Case CurrentInformation.intType
            Case ObjectDirectory
                lblNum.Caption = "正在获取..."
                GetObjectList hClickedItem, TreeView, ListView1, True, True
                GetPathByNodeW hClickedItem, TreeView, currentPath
                lblNum.Caption = "对象数量:" & ListView1.ItemCount
                currentPath = RightW(currentPath, LenW(currentPath) - 1)
                If currentPath <> "" Then txtFilePath.Text = currentPath
            Case File
                lblNum.Caption = "正在获取..."
                GetFileList hClickedItem, TreeView, ListView1, True, True, GetMenuCheckState(mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
                GetPathByNodeW hClickedItem, TreeView, currentPath
                lblNum.Caption = "文件数量:" & ListView1.ItemCount
                If currentPath <> "" Then txtFilePath.Text = currentPath
                'TreeView.ExpandEx(CurrentNode, TVE_EXPAND)
            Case Registry
                lblNum.Caption = "正在获取..."
                GetRegList hClickedItem, TreeView, ListView1, GetMenuCheckState(mnuRegKey, FrmMain_mnuRegKey_mnuEnableHiveAnalysis)
                GetPathByNodeW hClickedItem, TreeView, currentPath
                If currentPath <> "" Then txtFilePath.Text = currentPath
                lblNum.Caption = "值数量:" & ListView1.ItemCount
                'Print "[NM_DBLCIK]" & currentPath & " " & txtFilePath.Text
        End Select
        'TreeView.ExpandEx CurrentNode, TVE_EXPAND
    End If
    Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.TreeView]事件 : 单击了鼠标右键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_RCLICK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    Dim hClickedItem As HTREEITEM = TreeView_GetClickedItem(hWndControl)
    Dim currentPath As StringW
    GetPathByNodeW hClickedItem, TreeView, currentPath
    If CurrentInformation.intType = ObjectDirectory Then currentPath = RightW(currentPath, LenW(currentPath) - 1)
    'If currentPath <> "" Then txtFilePath.Text = currentPath
    
    If hClickedItem <> 0 Then
        ' 找到节点了，主动选择它
        TreeView.Selection = hClickedItem
        'Print TreeView.Text(hClickedItem)
        ' 现在 TreeView.Selection 就是实际右键点击的节点
        CurrentNode = TreeView.Selection
        
        ' 只在点击到节点时才弹出菜单
        Select Case CurrentInformation.intType
            Case File
                PopupMenu hWndForm, mnuFolder.HMENU
            Case Registry
                PopupMenu hWndForm, mnuRegKey.HMENU
        End Select
    End If
   Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.Check3]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check3_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Not IsDriverLoaded Then
        If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
        Else
            Return
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
        /'If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword", True)
        If (hDrv = INVALID_HANDLE_VALUE) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        If (Not OpenSymbolDevice) OrElse (hSymbolDrv = INVALID_HANDLE_VALUE) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        SymbolService_StartWorkers
        AfxMsg "加载成功!"
        IsDriverLoaded = True '/
        If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!"
    Else
        /'CloseDrv hDrv
        SymbolService_StopWorkers
        UnloadDriver App.Path & "SnowSword.sys", False
        IsDriverLoaded = False '/
        UninitDriver
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
        If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
    Else
        Return
    End If
    Dim isStatus As BOOLEAN = Check5.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, IOCTL_DenyAccessRegistry, @isStatus, SizeOf(BOOLEAN)
End Sub

'[FrmMain.Check7]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check7_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Not IsDriverLoaded Then 
        If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
        Else
            Return
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
    If Not IsDriverLoaded Then 
        If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
        Else
            Return
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
            Dim CurrentInfo As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
            CurrentInfo->intType = UnlockTheFile
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_TopMenu1_mnuRunasAdmin ' 以管理员身份运行
            RestartasAdmin
        Case FrmMain_TopMenu1_mnuFireWall ' 防火墙
            FrmFireWall.Show
        Case FrmMain_TopMenu1_mnuViewLog ' 显示日志
            FrmLog.Show
        Case FrmMain_TopMenu1_mnuCheckUpdate ' 检测更新
            CheckUpdate
   End Select
End Sub

'[FrmMain.TxtFilter2]事件 : 文本已经被修改（修改前用 EN_UPDATE
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_TxtFilter2_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    If (CurrentInformation.intType = File) Or (CurrentInformation.intType = Registry) Then Exit Sub 
    lblNum.Caption = "正在筛选..."
    SetListViewFilter ListView1, TxtFilter2.Text
    CommitListViewView ListView1
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
        CurrentInformationArray(0).ThreadId = CurrentInformation.ThreadId
        
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
        CurrentInformation.ThreadId = CurrentInformationArray(0).ThreadId
        
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

' 获取ListView鼠标位置+命中测试
Function GetListViewHitText(ListView As Class_ListView) As String
    Dim pt As Point
    GetCursorPos(@pt)
    ScreenToClient(ListView.hWnd, @pt)
    
    Dim lvinfo As LVHITTESTINFO
    lvinfo.pt = pt
    ListView_SubItemHitTest(ListView.hWnd, @lvinfo)
    
    If (lvinfo.iItem >= 0) AndAlso (lvinfo.iSubItem >= 0) Then
        Return ListView.GetItemText(lvinfo.iItem, lvinfo.iSubItem)
    Else
        Return ""
    End If
End Function

'[FrmMain]事件 : 自定义消息（全部消息），在其它事件处理后才轮到本事件。
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'wMsg        消息值，0--&H400 由WIN系统定义，用户自定义是 WM_USER+** 定义。例 PostMessage( 窗口句柄 , Msg , wParam , lParam ) 
'wParam      主消息参数，什么作用，由发送消息者定义
'lParam      副消息参数，什么作用，由发送消息者定义
Function FrmMain_Custom(hWndForm As hWnd, wMsg As UInteger, wParam As wParam, lParam As lParam) As LResult
    Select Case wMsg
    Case WM_COMMAND
        Dim cmdId As ULong = LOWORD(wParam)
        Dim notifyCode As ULong = HIWORD(wParam)
        ' 检查是否是我们的动态菜单范围
        If cmdId >= IDM_SELECT_COLUMN_BASE And cmdId < IDM_SELECT_COLUMN_BASE + 100 Then
            ' 手动调用处理函数
            FrmMain_mnuColumn_WM_Command(hWndForm, cmdId)
        End If
    Case WM_NOTIFY
        Dim Nmhdr As NMHDR Ptr = Cast(NMHDR Ptr, lParam)
        If Nmhdr->hwndFrom = ListView1.hWnd Then ' 这里可以添加对多个控件的处理?
            Select Case Nmhdr->code
            Case LVN_GETDISPINFO
                If g_CuiMode Then Return 0
                Dim p As NMLVDISPINFO Ptr = Cast(NMLVDISPINFO Ptr, lParam)
                Dim ctx As ListViewContext Ptr = _
                    Cast(ListViewContext Ptr, GetWindowLongPtr(p->hdr.hwndFrom, GWLP_USERDATA))
                ' ========== 新增：全套安全校验（修复崩溃） ==========
                If ctx = NULL Then Return 0 ' 上下文为空
                If ctx->Snap = NULL Then Return 0 ' 快照为空
                If UBound(ctx->VisibleIndex) = -1 Then Return 0 ' ✅ 崩溃根源：缺失这行！
                If p->item.iItem < 0 Or p->item.iItem >= ctx->VisibleCount Then Return 0 ' 项索引越界
                If ctx->VisibleCount <= 0 Then Return 0 ' 无可见项
                ' ======================================================
                
                Dim real As Long = ctx->VisibleIndex(p->item.iItem)
                ' 新增：真实行索引校验
                If real < 0 Or real >= ctx->Snap->RowCount Then Return 0
                Dim r As LVRow Ptr = ctx->Snap->Rows[real]
                If r = NULL Then Return 0 ' 行数据为空
                
                ' ========== 新增：返回图标索引 ==========
                If (p->item.mask And LVIF_IMAGE) <> 0 Then
                    p->item.iImage = r->iconIndex
                End If
                
                If (p->item.mask And LVIF_TEXT) <> 0 Then
                    If p->item.iSubItem < r->ColCount Then
                        p->item.pszText = r->Cols[p->item.iSubItem]
                    Else
                        p->item.pszText = @WStr("")
                    End If
                    p->item.cchTextMax = lstrlenW(p->item.pszText) + 1
                End If
                Return 0

            Case NM_CUSTOMDRAW
                Dim cd As NMLVCUSTOMDRAW Ptr = Cast(NMLVCUSTOMDRAW Ptr, lParam)
                Dim ctx As ListViewContext Ptr = _
                Cast(ListViewContext Ptr, GetWindowLongPtr(cd->nmcd.hdr.hwndFrom, GWLP_USERDATA))
                
                ' ====================== 【崩溃修复】新增全套校验 ======================
                If ctx = NULL Then Return CDRF_DODEFAULT
                If ctx->Snap = NULL Then Return CDRF_DODEFAULT
                If UBound(ctx->VisibleIndex) = -1 Then Return CDRF_DODEFAULT
                If cd->nmcd.dwItemSpec < 0 Or cd->nmcd.dwItemSpec >= ctx->VisibleCount Then Return CDRF_DODEFAULT
                If ctx->VisibleCount <= 0 Then Return CDRF_DODEFAULT
                ' ====================================================================
                
                Select Case cd->nmcd.dwDrawStage

                Case CDDS_PREPAINT
                    ' 告诉系统：我要接管 Item 绘制
                    Return CDRF_NOTIFYITEMDRAW

                Case CDDS_ITEMPREPAINT
                    ' 告诉系统：我还要管子项
                    Return CDRF_NOTIFYSUBITEMDRAW

                Case CDDS_ITEMPREPAINT Or CDDS_SUBITEM
                    Dim real As Long = ctx->VisibleIndex(cd->nmcd.dwItemSpec)
                    
                    ' 基础校验
                    If real < 0 Or real >= ctx->Snap->RowCount Then Return CDRF_DODEFAULT
                    Dim r As LVRow Ptr = ctx->Snap->Rows[real]
                    If r = NULL Then Return CDRF_DODEFAULT
                    
                    ' ========== 关键修复：获取当前子项索引并校验 ==========
                    Dim iSubItem As Long = cd->iSubItem  ' 当前正在绘制的列索引
                    
                    ' 如果子项索引超出该行实际列数，使用默认绘制
                    If iSubItem < 0 Or iSubItem >= r->ColCount Then
                        Return CDRF_DODEFAULT
                    End If
                    
                    ' 如果该子项数据为空（NULL 或空字符串），使用默认绘制
                    ' 这样可以避免空列的颜色被错误设置
                    If r->Cols[iSubItem] = NULL Then
                        Return CDRF_DODEFAULT
                    End If
                    
                    ' 只有数据有效的列才应用自定义颜色
                    cd->clrText   = r->Fore
                    cd->clrTextBk = r->Back
                    Return CDRF_DODEFAULT

                End Select
            End Select
        ElseIf Nmhdr->hwndFrom = mCtrlTreeList1.hWnd Then
            If Nmhdr->code = TVN_KEYDOWN Then
                ' 判断 Ctrl + C
                Dim pKey As NMTVKEYDOWN Ptr = Cast(NMTVKEYDOWN Ptr, lParam)
                If pKey->wVKey = Asc("C") Then
                    If (GetKeyState(VK_CONTROL) And &H8000) <> 0 Then
                        CopyDataToClipboard mCtrlTreeList1.GetItemText(LastSelectItem, LastClickedSubItem)
                    End If
                End If
            End If
        End If
    Case WM_USER + TrayIco1.CallMsg
        Select Case lParam
        Case WM_LBUTTONDOWN
            'Print "左键单击托盘图标"
        Case WM_LBUTTONDBLCLK
            'Print "左键双击托盘图标"
            Me.WindowState = 0 ' 显示主窗口
            Me.Show
        Case WM_RBUTTONDOWN
            Dim pt As Point
            GetCursorPos @pt
            SetForegroundWindow hWndForm  ' 1. 设置前台窗口
            TrackPopupMenu mnuMain.HMENU, TPM_RIGHTBUTTON Or TPM_RETURNCMD, _
                          pt.x, pt.y, 0, hWndForm, NULL   ' 2. 显示菜单
            PostMessage hWndForm, WM_NULL, 0, 0  ' 3. 修复菜单焦点问题
            'PopupMenu hWndForm, mnuCallbacks.hMenu
            'DestroyMenu hMenu
            'SetFocus Me.hWnd ' 设置当前窗口的焦点
        End Select
    Case WM_SignVerify
        Dim pMsg As SIGNVERIFY_MSG Ptr = Cast(SIGNVERIFY_MSG Ptr, lParam)
        If pMsg = 0 Then Return False
        
        ' ====================== 核心UAF防护（保留异步，不取消消息） ======================
        ' 1. 校验ListView句柄有效性
        Dim ctx As ListViewContext Ptr = Cast(ListViewContext Ptr, GetWindowLongPtr(pMsg->hListView, GWLP_USERDATA))
        ' 2. 三重校验：上下文存在 + 有效标记 + 实例ID匹配
        If ctx = 0 Or ctx->IsValid = False Or ctx->InstanceId <> pMsg->InstanceId Then
            Deallocate pMsg  ' 直接释放无效消息，不访问任何指针
            Return 0
        End If
        ' ==================================================================================
    
        If pMsg->ctrlType = "ListView" Then
            ' 更新 ListView 的颜色
            'Print "开始更新 ListView 的颜色..."
            SetItemColor ListView1, pMsg->rowIndex, pMsg->ForeColor, pMsg->BackColor
            Deallocate pMsg
        End If
        
        ' 延迟刷新
        Static LastRefresh As ULong = 0
        If GetTickCount() - LastRefresh > 50 Then
            ListView1.Refresh
            LastRefresh = GetTickCount
        End If
    Case WM_TREEVIEW_DBLCLK
        Dim hItem As HTREEITEM = Cast(HTREEITEM, wParam)
        Dim xPos As Long = LOWORD(lParam)
        Dim yPos As Long = HIWORD(lParam)
        
        ' 直接调用你的处理函数
        FrmMain_treMain_WM_LButtonDblclk(hWndForm, treMain.hWnd, 0, xPos, yPos)
        Return 0
    End Select
    Function = FALSE ' 返回FALSE表示不阻止系统继续处理消息
End Function

' ===================== "转到进程"调用 =====================
Sub SwitchToModule(ModuleName As StringW)
    ' 触发切换/刷新（模拟双击或直接调用）
    TreeView_SimulateDblClick treMain.hWnd, GetNodeByText(treMain, ModuleName)
End Sub

'[FrmMain.mnuKernelModule]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuKernelModule_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuKernelModule_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetKernelModuleList ListView1
            'SetListViewData ListView1
            lblNum.Caption = "数量:" & ListView1.ItemCount
        Case FrmMain_mnuKernelModule_mnuCheckHideDriver ' 检测隐藏驱动
            Dim bState As Boolean = GetMenuCheckState(mnuKernelModule, FrmMain_mnuKernelModule_mnuCheckHideDriver)
            SetMenuCheckState mnuKernelModule, FrmMain_mnuKernelModule_mnuCheckHideDriver, Not bState
            If Not bState Then
                GetKernelModuleList ListView1
            Else
                GetKernelModuleList ListView1
            End If
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
            Dim CurrentInfo As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
            CurrentInfo->intType = IOFunction
            CurrentInfo->CurrentDriver.DriverObject = ValLng("&H" & RightW(ListView1.GetItemText(ListView1.SelectedItem, 5), LenW(ListView1.GetItemText(ListView1.SelectedItem, 5)) - 2))
            CurrentInfo->CurrentDriver.DriverName = ListView1.GetItemText(ListView1.SelectedItem, 1)
            CurrentInfo->CurrentDriver.DriverPath = ListView1.GetItemText(ListView1.SelectedItem, 4)
            FrmListView.Show,, Cast(Integer, CurrentInfo)
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
            stMemory.dwSize = ModuleSize
            stMemory.pData = @bytDump(0)
            
            If IoControl(hDrv, IOCTL_DumpKernelModule, @stMemory, SizeOf(MemoryStruct), NULL, 0, @lpRet) = 0 Then
                AfxMsg "dump失败!"
                'Print "[FrmMain_mnuKernelModule_mnuDumpToFile]IOCTL_ReadProcessMemory:" & WinErrorMsg(GetLastError) & GetLastError
                'Print "Information:" & lpRet
                Erase bytDump
                Exit Sub
            End If
            If lpRet = 0 Then
                Erase bytDump
                Exit Sub
            End If
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
        Case FrmMain_mnuKernelModule_mnuEditMemory ' 查看/编辑内存
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem
            'Print ListView.GetItemText(LastClickedItem, LastClickedSubItem)
            If LastClickedSubItem <> 2 AndAlso LastClickedSubItem <> 5 Then Exit Sub
            If LastClickedSubItem = 5 AndAlso (ValULng(FF_Replace(ListView1.GetItemText(LastClickedItem, LastClickedSubItem), "0x", "&H")) = 0) Then Exit Sub
            Dim Addr As ULONG64 = ValULng(FF_Replace(ListView1.GetItemText(LastClickedItem, LastClickedSubItem), "0x", "&H"))
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
        Case FrmMain_mnuKernelModule_mnuLittleCopy ' 复制单格数据
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
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
    If Not IsDriverLoaded Then 
        If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
            If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
        Else
            Return
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
    Dim SelectIndex As Long = ListView1.SelectedItem
    Select Case CurrentInformation.intType
        Case File
            Dim CurrentPath As StringW = ""
            GetPathByNodeW TreeView.Selection, TreeView, CurrentPath ' 获取当前文件路径
            CurrentPath = CurrentPath & ListView1.GetItemText(SelectIndex, 0)
            Print "CurrentPath:" & CurrentPath
            ShellExecute NULL, NULL, CurrentPath, NULL, NULL, 1 ' 以默认方式打开文件
        Case TaskScheduler
            Dim ProgramPath As StringW = UCaseW(ListView1.GetItemText(SelectIndex, 7))
            SwitchToModule("进程")
            For i As Integer = 0 To ListView1.ItemCount - 1
                If UCaseW(ListView1.GetItemText(i, 2)) = ProgramPath Then
                    ListView1.SelectedItem = i
                    ListView_EnsureVisible(ListView1.hWnd, i, False)
                    Return
                End If
            Next
            AfxMsg "未找到进程!"
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
        Select Case CurrentInformation.intType
            Case File
                If (GetFileAttributes(strPath) And fbDirectory) = fbDirectory Then ' 是目录,跳转过去
                    CurrentNode = GetNodeByFilePathW(strPath, TreeView, ListView1) ' 查找并获取对应Node
                    If CurrentNode <> NULL Then
                        SendMessage TreeView.hWnd, TVM_SELECTITEM, TVGN_CARET, Cast(lParam, CurrentNode) ' 选中这项
                    Else
                        'Print "[FrmMain_txtFilePath_WM_KeyUp]CurrentNode = NULL"
                    End If
                Else ' 是文件,直接打开
                    GetPathByNodeW CurrentNode, TreeView, CurrentPath ' 获取当前文件路径 
                    ShellExecute NULL, NULL, CurrentPath, NULL, NULL, 1 ' 以默认方式打开文件
                End If
            Case Registry
        End Select
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
   Dim gzmk As String, AllMode As String, pianyi As ULong, gzmkbb As String
   Dim ExceptionAddress As UInteger = Cast(UInteger, excp.ExceptionRecord->ExceptionAddress)
   Dim I As ULong
   
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
            AllMode   &= Hex(Mode.modBaseAddr, Len(ULong) * 2) & " " & CWSTRtoString(Mode.szExePath) & "  [" & GetVersionInfo(Mode.szExePath) & "][" & GetFileLength(Mode.szExePath) & "]" & vbCrLf
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
   
   Dim 堆栈列表 As String, zdcc As ULong, zdi As Long, sjStr As String, sjc As Long, zpi As Long, zk As Long = Len(ULong) * 2, jcm As String
   Dim sp       As ULong
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
               If zdcc > Cast(UInteger, gMode(bI).modBaseAddr) And zdcc < Cast(UInteger, gMode(bI).modBaseAddr) + Cast(UInteger, gMode(bI).modBaseSize) Then
                  if Len(sjStr) Then '纯数据
                     堆栈列表 &= sjStr & vbCrLf
                     sjStr    = ""
                  End if
                  堆栈列表 &= jcm & "+" & Hex(I - sp, 4) & "]=" & Hex(zdcc, zk) & "  " & CWSTRtoString(gMode(bI).szModule) & "+" & Hex(zdcc - Cast(UInteger, gMode(bI).modBaseAddr)) & vbCrLf
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
         局部列表 &= jcm & "+" & Hex(zdi *Len(ULong), 4) & "]=" & Hex(zdcc, zk) & " "
      ElseIf zdi < 0 Then
         局部列表 &= jcm & "-" & Hex(abs(zdi) *Len(ULong), 4) & "]=" & Hex(zdcc, zk) & " "
      End if
      if (zdi Mod 10) = 0 Then 局部列表 &= vbCrLf
      zdi -= 1
      
   Wend
   
   bug &= "堆栈列表(栈顶): Esp/Rsp ------------------------------"       & vbCrLf & 堆栈列表
   bug &= "局部列表(栈底): Ebp/Rbp ------------------------------------" & vbCrLf & 局部列表
   bug &= "模块列表:   ------------------------------------------"       & vbCrLf & AllMode
   
   bug &= "日志:" & vbCrLf & strLog
   
   If Len(文件名) Then SaveFileStr 文件名, bug
   
   Function = bug
   
End Function

'[FrmMain.VEH1]事件 : 向量化异常处理（程序崩溃后处理）
'整个软件，只需一个VEH即可，在主窗口放置控件，所有窗口、模块、多线程等发生崩溃，都会跑到这里执行。
Function FrmMain_VEH1_VectExcepHandler(ByRef excp As EXCEPTION_POINTERS) As Integer
    ' 这两种异常是 OutputDebugString 的产物，不是真正的错误
    ' DBG_PRINTEXCEPTION_C = 0x40010006
    ' DBG_PRINTEXCEPTION_W = 0x4001000A
    If excp.ExceptionRecord->ExceptionCode = &H40010006 OrElse _
       excp.ExceptionRecord->ExceptionCode = &H4001000A Then
        ' 直接返回 EXCEPTION_CONTINUE_SEARCH，系统会正常处理
        Return 0  ' 或者 Return 1 也行，这里返回 0 让系统自行处理即可
    End If
    'AfxMsg "程序即将崩溃..."
    保存软件崩溃日志 App.Path & "bug" & NowString(1) & ".txt", excp
    Return 0
End Function

'[FrmMain.mCtrlTreeList1]事件 : 移动鼠标（状态机核心）
Sub FrmMain_mCtrlTreeList1_WM_MouseMove(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Dim tme As TRACKMOUSEEVENT
    g_hWndForm = hWndForm
    g_hTreeControl = hWndControl
    
    tme.cbSize = SizeOf(TrackMouseEvent)
    tme.dwFlags = TME_HOVER Or TME_LEAVE
    tme.hwndTrack = hWndControl
    tme.dwHoverTime = HOVER_DEFAULT
    
    Select Case g_TooltipState
        Case State_Idle
            g_TooltipState = State_Hovering
            g_nHoverX = xPos
            g_nHoverY = yPos
            TrackMouseEvent @tme
        
        Case State_Hovering, State_Showing
            Dim nDeltaX As Long = Abs(xPos - g_nHoverX)
            Dim nDeltaY As Long = Abs(yPos - g_nHoverY)
            If nDeltaX > 4 Or nDeltaY > 4 Then
                ' ===================== 【核心修复1】统一用 TTM_TRACKACTIVATE 隐藏 =====================
                If g_TooltipState = State_Showing Then
                    Dim ti As TOOLINFOW
                    ZeroMemory(@ti, SizeOf(TOOLINFOW))
                    ti.cbSize = SizeOf(TOOLINFOW)
                    ti.uFlags = TTF_IDISHWND Or TTF_TRACK Or TTF_ABSOLUTE
                    ti.hWnd = g_hWndForm
                    ti.uId = Cast(UINT_PTR, g_hWndForm)
                    SendMessage g_hTooltip, TTM_TRACKACTIVATE, 0, Cast(LPARAM, @ti)
                End If
                
                ' ===================== 【核心修复2】重置状态后，重新调用 TrackMouseEvent =====================
                g_TooltipState = State_Hovering
                g_nHoverX = xPos
                g_nHoverY = yPos
                TrackMouseEvent @tme ' 关键！必须重新调用，让系统再次触发 Hover
            End If
    End Select
End Sub

'[FrmMain.mCtrlTreeList1]事件 : 悬停（定时器触发，显示 Tooltip）
Sub FrmMain_mCtrlTreeList1_WM_MouseHover(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    'Print "[调试] ✅ WM_MouseHover 已触发"
    'Print "[调试] 当前状态: " & g_TooltipState & " (1=Hovering,2=Showing)"
    If g_TooltipState <> State_Hovering Then Exit Sub
    
    Dim pp As MC_TLHITTESTINFO
    pp.pt.x = xPos
    pp.pt.y = yPos
    pp.flags = MC_TLHT_ONITEM
    SendMessage hWndControl, MC_TLM_HITTEST, 0, Cast(LPARAM, @pp)
    'Print "[调试] 悬停节点句柄: " & pp.hItem
    
    If pp.hItem = 0 Then
        g_TooltipState = State_Idle
        Exit Sub
    End If
    g_hLastHoverItem = pp.hItem
    
    Dim SelectItem As MC_HTREELISTITEM, iCurrentColumn As Long
    TreeList_SubItemHitTest mCtrlTreeList1, xPos, yPos, SelectItem, iCurrentColumn, False
    'Print "[FrmMain_mCtrlTreeList1_WM_MouseHover]xPos = " & xPos & "yPos = " & yPos & "iCurrentColumn:" & iCurrentColumn
    g_sTooltipText = mCtrlTreeList1.GetItemText(pp.hItem, iCurrentColumn)
    
    ' ===================== 【核心修复3】加 TTF_DI_SETITEM 标志 =====================
    Dim ti As TOOLINFOW
    ZeroMemory(@ti, SizeOf(TOOLINFOW))
    ti.cbSize = SizeOf(TOOLINFOW)
    ti.uFlags = TTF_IDISHWND Or TTF_TRACK Or TTF_ABSOLUTE Or TTF_DI_SETITEM ' 关键！
    ti.hWnd = g_hWndForm
    ti.uId = Cast(UINT_PTR, g_hWndForm)
    ti.lpszText = g_sTooltipText.WstrPtr

    SendMessage g_hTooltip, TTM_SETTOOLINFO, 0, Cast(LPARAM, @ti)

    Dim pt As POINT
    pt.x = xPos
    pt.y = yPos
    ClientToScreen g_hTreeControl, @pt

    SendMessage g_hTooltip, TTM_TRACKPOSITION, 0, MAKELONG(pt.x + 10, pt.y + 20)
    SendMessage g_hTooltip, TTM_TRACKACTIVATE, 1, Cast(LPARAM, @ti)

    'Print "[调试] ✅ Tooltip 已显示"
    g_TooltipState = State_Showing
End Sub

Sub FrmMain_mCtrlTreeList1_WM_MouseLeave(hWndForm As hWnd, hWndControl As hWnd)
    If g_TooltipState = State_Showing Then
        Dim ti As TOOLINFOW
        ZeroMemory(@ti, SizeOf(TOOLINFOW))
        ti.cbSize = SizeOf(TOOLINFOW)
        ti.uFlags = TTF_IDISHWND Or TTF_TRACK Or TTF_ABSOLUTE ' 完整字段
        ti.hWnd = g_hWndForm
        ti.uId = Cast(UINT_PTR, g_hWndForm)
        SendMessage g_hTooltip, TTM_TRACKACTIVATE, 0, Cast(LPARAM, @ti)
    End If
    g_TooltipState = State_Idle
    g_hLastHoverItem = 0
End Sub

Sub FrmMain_mCtrlTreeList1_WM_RButtonDown(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    If g_TooltipState = State_Showing Then
        Dim ti As TOOLINFOW
        ZeroMemory(@ti, SizeOf(TOOLINFOW))
        ti.cbSize = SizeOf(TOOLINFOW)
        ti.uFlags = TTF_IDISHWND Or TTF_TRACK Or TTF_ABSOLUTE ' 完整字段
        ti.hWnd = g_hWndForm
        ti.uId = Cast(UINT_PTR, g_hWndForm)
        SendMessage g_hTooltip, TTM_TRACKACTIVATE, 0, Cast(LPARAM, @ti)
    End If
    g_TooltipState = State_Idle
End Sub

'[FrmMain]事件 : 窗口已经改变了大小
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'fwSizeType = SIZE_MAXHIDE     SIZE_MAXIMIZED   SIZE_MAXSHOW    SIZE_MINIMIZED    SIZE_RESTORED  
''            其他窗口最大化   窗口已最大化     其他窗口恢复    窗口已最小化      窗口已调整大小
'nWidth nHeight  是客户区大小，不是全部窗口大小。
Sub FrmMain_WM_Size(hWndForm As hWnd, fwSizeType As Long, nWidth As Long, nHeight As Long)
    If Not bFrmMainShowed Then Exit Sub
    
    ' ==============================================
    ' 只有【初始化完成后】的 SIZE_RESTORED 才执行（用户手动调整）
    ' ==============================================
    If fwSizeType = SIZE_RESTORED Or fwSizeType = SIZE_MAXIMIZED Then
        'Print "[FrmMain_WM_Size]fwSizeType = " & fwSizeType & " nWidth:" & nWidth & " nHeight:" & nHeight
        'gLayoutMode = LAYOUT_LIST_ONLY
        'gMainView = VIEW_LISTVIEW
        UpdateLayout
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

'[FrmMain.TreeView]事件 : 单击了鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_CLICK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    'Print "FrmMain_TreeView_NM_CLICK"
    Dim hClickedItem As HTREEITEM = TreeView_GetClickedItem(hWndControl)
    Dim currentPath As StringW
    CurrentNode = hClickedItem
    GetPathByNodeW hClickedItem, TreeView, currentPath
    If CurrentInformation.intType = ObjectDirectory Then currentPath = RightW(currentPath, LenW(currentPath) - 1)
    If currentPath <> "" Then txtFilePath.Text = currentPath
    'Print "[NM_CLICK]" & currentPath & " " & txtFilePath.Text
    Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.mnuMain]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuMain_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_mnuMain_mnuShowWindow ' 显示主窗口
            Me.WindowState = 0 '正常
            Me.Show
   End Select
End Sub

'[FrmMain.ListView1]事件 : 键盘按下
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'pNKD.wVKey 虚拟键盘码。 
Sub FrmMain_ListView1_LVN_KeyDown(hWndForm As hWnd, hWndControl As hWnd, pNKD As LV_KEYDOWN)
    ' 判断 Ctrl + C
    If pNKD.wVKey = Asc("C") Then
        If (GetKeyState(VK_CONTROL) And &H8000) <> 0 Then
            'Print "按下Ctrl+C"
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
            End If
        End If
    End If
End Sub

'[FrmMain.ListView1]事件 : 按下鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_ListView1_WM_LButtonDown(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Dim lvinfo As LVHITTESTINFO
    lvinfo.pt.x = xPos
    lvinfo.pt.y = yPos
    ScreenToClient hWndControl, @CurrentPos
    ListView_SubItemHitTest(hWndControl, @lvinfo)
    If (lvinfo.iItem >= 0 AndAlso lvinfo.iItem <= ListView1.ItemCount - 1) AndAlso _
    (lvinfo.iSubItem >= 0 AndAlso lvinfo.iSubItem <= ListView1.ColumnCount - 1) Then
        LastClickedItem = lvinfo.iItem
        LastClickedSubItem = lvinfo.iSubItem
    End If
End Sub

'[FrmMain.ListView1]事件 : 一个列被点击了
'hWndForm    当前窗口的句柄
'hWndControl 当前控件的句柄
'pNMV.iSubItem  当前被点击列的索引
Sub FrmMain_ListView1_LVN_ColumnClick(hWndForm As hWnd, hWndControl As hWnd, pNMV As NM_LISTVIEW)
    ' 1. 获取ListView上下文（和你LVN_GETDISPINFO里的获取方式一致）
    Dim ctx As ListViewContext Ptr = _
        Cast(ListViewContext Ptr, GetWindowLongPtr(hWndControl, GWLP_USERDATA))
    
    ' 2. 安全校验（防止空指针崩溃）
    If ctx = NULL Then Exit Sub
    If ctx->Snap = NULL Then Exit Sub
    If ctx->VisibleCount <= 1 Then Exit Sub ' 不足1行无需排序
    If pNMV.iSubItem < 0 Then Exit Sub      ' 无效列索引
    
    ' 3. 更新排序状态
    ' 同列重复点击 → 切换升降序
    If ctx->SortedColumn = pNMV.iSubItem Then
        ctx->SortAscending = Not ctx->SortAscending
    Else
        ' 新列点击 → 默认升序
        ctx->SortedColumn = pNMV.iSubItem
        ctx->SortAscending = True
    End If
    
    ' 4. 执行排序（核心排序函数，复用之前的逻辑）
    SortListViewVisibleIndex(ctx)
    
    ' 5. 刷新虚拟ListView（必须手动刷新，否则界面不更新）
    ListView_RedrawItems(hWndControl, 0, ctx->VisibleCount - 1)
End Sub

'[FrmMain.mnuProcess]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码
'wID      菜单项命令ID
Sub FrmMain_mnuProcess_WM_Command(hWndForm As hWnd, wID As ULong)
    'Dim CurrentIndex As Integer = GetIndexByListViewHwnd(Me.hWnd)
    'CurrentInformation.ProcessId = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0))
    'Dim CurrentProcessName As StringW = ListView1.GetItemText(ListView1.SelectedItem, 2)
    Dim CurrentInfo As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
    CurrentInfo->ProcessId = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0))
    Select Case wID
        Case FrmMain_mnuProcess_mnuTerminateProcess ' 结束进程
            Dim dwProcessId As DWORD
            For i As Integer = ListView1.ItemCount - 1 To 0 Step -1
                If IsListViewItemSelected(ListView1, i) Then
                    dwProcessId = ValULng(ListView1.GetItemText(i, 0))
                    If Not KillProcess(dwProcessId) Then Continue For
                    DeleteItemEx ListView1, i, False
                End If
            Next
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
            CommitListViewView ListView1
        Case FrmMain_mnuProcess_mnuForceTerminateProcess '强制结束进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
                    AfxMsg "加载失败!"
                    Exit Sub
                End If
                hDrv = OpenDrv("\\.\\SnowSword", True)
                If (hDrv <> INVALID_HANDLE_VALUE) Then
                    SymbolService_StartWorkers
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
                    If IoControl(hDrv, IOCTL_KillProcess, @dwPID, SizeOf(HANDLE)) <> 0 Then
                        DeleteItemEx ListView1, i, False
                    Else
                        MyLog.PrintWin32Error "FrmMain_mnuProcess_mnuForceTerminateProcess", "IOCTL_KillProcess", "dwPID=" & dwPID
                    End If
                End If
            Next
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
            CommitListViewView ListView1
        Case FrmMain_mnuProcess_mnuViolentTerminateProcess '暴力结束进程
            Dim dwProcessId As HANDLE = Cast(HANDLE, ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0)))
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
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
            If Not IsDriverLoaded Then 
                If AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                    If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
                Else
                    Return
                End If
            End If
            Dim dwProcessId As DWORD = ValULng(ListView1.GetItemText(ListView1.SelectedItem, 0)), ret As Long, lpRet As DWORD
            IoControl hDrv, IOCTL_AddProtectedProcess, @dwProcessId, SizeOf(DWORD)
        Case FrmMain_mnuProcess_mnuCheckHideProcess '检测隐藏进程
            If (Not GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)) AndAlso (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Exit Sub
            End If
            SetMenuCheckState mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess, Not GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            lblNum.Caption = "正在刷新..."
            GetProcessList ListView1, GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            'SaveCurrentListViewState ListView1, CurrentInformation.intType
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuProcess_mnuCheckSign ' 校验数字签名
            For i As Integer = 0 To ListView1.ItemCount - 1
                If Not IsListViewItemSelected(ListView1, i) Then Continue For
                
                Dim dwPID As DWORD = ValUInt(ListView1.GetItemText(i, 0))
                Dim strPath As StringW = ListView1.GetItemText(i, 4)
                Dim strCompany As StringW = ListView1.GetItemText(i, 6)
                
                Dim ForeColor As COLORREF, BackColor As COLORREF
                If strPath = "System Idle Process" OrElse strPath = "System" OrElse strPath = "Registry" OrElse strPath = "Memory Compression" Then Continue For
                Dim bVerify As Boolean = VerifyFileSign(strPath)
                If (InStr(strPath, "\") <> 0) AndAlso _
                   (Not bVerify) Then
                    GetItemColor ListView1, i, ForeColor, BackColor
                    SetItemColor ListView1, i, ForeColor, FB_GoldenYellow
                    'Print dwPID & "可疑!"
                    'Print strCompany & " " & strPath & " " & bVerify
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
                
                ' 过滤条件：有实际路径、非空公司名、签名无效
                ' 排除无文件实体的系统进程（如 Memory Compression, Registry 等）
                If strPath = "System Idle Process" OrElse strPath = "System" OrElse strPath = "Registry" OrElse strPath = "Memory Compression" Then Continue For
                Dim bVerify As Boolean = VerifyFileSign(strPath)
                If (InStr(strPath, "\") <> 0) AndAlso _
                   (Not bVerify) Then
                    
                    GetItemColor ListView1, i, ForeColor, BackColor
                    SetItemColor ListView1, i, ForeColor, FB_GoldenYellow
                    MyLog.PrintLog LOG_WARN,,, dwPID & "可疑!"
                    'Print strCompany & " " & strPath & " 签名无效"
                End If
                FF_DoEvents
            Next
            ListView1.Refresh
        Case FrmMain_mnuProcess_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetProcessList ListView1, GetMenuCheckState(mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            'SaveCurrentListViewState ListView1, CurrentInformation.intType
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuProcess_mnuInjectDll ' 注入DLL
            Dim szFile As StringW = FF_OpenFileDialog(,,,,"DLL files (*.dll)|*.dll|" & "All Files (*.*)|*.*|")
            If szFile <> "" Then If RemoteInjectDll(CurrentInfo->ProcessId, szFile) Then AfxMsg "注入成功!" Else AfxMsg "注入失败!"
        Case FrmMain_mnuProcess_mnuViewModule ' 查看模块
            CurrentInfo->intType = Module
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewThread ' 查看线程
            CurrentInfo->intType = Thread
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewMemory ' 查看内存
            CurrentInfo->intType = Memory
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewWindowList ' 查看窗口列表
            CurrentInfo->intType = Windows
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewHandleList ' 查看句柄列表
            CurrentInfo->intType = Handles
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewWebConnect ' 查看网络连接列表
            CurrentInfo->intType = WebConnect
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewPrivilege ' 查看特权列表
            CurrentInfo->intType = Privilege
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewOtherInfo ' 查看其他信息
            CurrentInfo->intType = ProcessOtherInfo
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewHook ' 查看Hook
            CurrentInfo->CurrentModule.ModuleName = ""
            CurrentInfo->CurrentModule.ModuleHandle = NULL
            CurrentInfo->intType = Hook
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewProcessTimer ' 查看进程定时器列表
            CurrentInfo->intType = ProcessTimer
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewMsgHook ' 查看消息钩子
            CurrentInfo->intType = MsgHook
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewEventHook ' 查看事件钩子
            CurrentInfo->intType = EventHook
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuProcess_mnuViewHotkey ' 查看热键
            CurrentInfo->intType = Hotkey
            FrmListView.Show,, Cast(Integer, CurrentInfo)

        Case FrmMain_mnuProcess_mnuLittleCopy ' 复制单格数据
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
            End If
        Case FrmMain_mnuProcess_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 4)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
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
            'SetListViewData ListView1
            lblNum.Caption = "数量:" & ListView1.ItemCount
        Case FrmMain_mnuCallbacks_mnuControlCallback ' 禁用/启用回调
            
        Case FrmMain_mnuCallbacks_mnuRemoveCallback ' 移除回调
            Dim CallbackAddress As PVOID = Cast(PVOID, ValULng(FF_Replace(ListView1.GetItemText(SelectIndex, 1), "0x", "&H")))
            Dim CallbackType As StringW = ListView1.GetItemText(SelectIndex, 0)
            Dim TheContext As StringW = ListView1.GetItemText(SelectIndex, 3)
            Dim Other0 As StringW = ListView1.GetItemText(SelectIndex, 4)
            Other0 = RightW(Other0, LenW(Other0) - InStr(Other0, "="))
            Dim Callback_Info As CallbackInfo
            If (IsDriverLoaded AndAlso CallbackAddress <> NULL) Then
                Callback_Info.Func = Cast(UInteger, CallbackAddress)
                Callback_Info.TheType = CallbackType
                Callback_Info.TheContext = ValULng(FF_Replace(TheContext, "0x", "&H"))
                Callback_Info.Others(0) = ValULng(FF_Replace(Other0, "0x", "&H"))
                If IoControl(hDrv, IOCTL_DeleteCallback, @Callback_Info, SizeOf(Callback_Info)) <> 0 Then
                    AfxMsg "移除成功!"
                    DeleteItemEx ListView1, SelectIndex
                Else
                    AfxMsg "移除失败!"
                End If
            End If
        Case FrmMain_mnuCallbacks_mnuEditMemory ' 查看/编辑内存
            If LastClickedSubItem <> 1 AndAlso LastClickedSubItem <> 3 Then Exit Sub
            Dim Addr As ULONG64 = ValULng(FF_Replace(ListView1.GetItemText(LastClickedItem, LastClickedSubItem), "0x", "&H"))
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
        Case FrmMain_mnuCallbacks_mnuLittleCopy ' 复制单格数据
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
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
            'SetListViewData ListView1
            lblNum.Caption = "数量:" & ListView1.ItemCount
        Case FrmMain_mnuService_mnuStartService ' 启动服务
            If MyStartService(ListView1.GetItemText(ListView1.SelectedItem, 0)) Then AfxMsg "启动成功!" Else AfxMsg "启动失败!"
        Case FrmMain_mnuService_mnuStopService ' 停止服务
            If MyStopService(ListView1.GetItemText(ListView1.SelectedItem, 0)) Then AfxMsg "停止成功!" Else AfxMsg "停止失败!"
        Case FrmMain_mnuService_mnuPauseService ' 暂停服务
            If MyPauseService(ListView1.GetItemText(ListView1.SelectedItem, 0)) Then AfxMsg "暂停成功!" Else AfxMsg "暂停失败!"
        Case FrmMain_mnuService_mnuContinueService ' 恢复服务
            If MyContinueService(ListView1.GetItemText(ListView1.SelectedItem, 0)) Then AfxMsg "恢复成功!" Else AfxMsg "恢复失败!"
        Case FrmMain_mnuService_mnuDeleteService ' 删除服务
            'If MyDeleteService(ListView1.GetItemText(ListView1.SelectedItem, 0)) Then AfxMsg "删除成功!" Else AfxMsg "删除失败!"
        Case FrmMain_mnuService_mnuLittleCopy ' 复制单格数据
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
            End If
        Case FrmMain_mnuService_mnuLocateFilePath ' 定位文件位置(资源浏览器)
            Dim lpFilePath As StringW = ListView1.GetItemText(ListView1.SelectedItem, 2)
            If Not LocateFilePathByExplorer(lpFilePath) Then AfxMsg "定位失败!", "提示"
   End Select
End Sub

'[FrmMain.mCtrlTreeList1]事件 : 鼠标右键单击
'hWndForm    当前窗口的句柄
'hWndControl 当前控件的句柄
'xPos yPos   当前鼠标位置，相对于屏幕。就是屏幕坐标。
'[FrmMain.mCtrlTreeList1]事件 : 鼠标右键单击
Sub FrmMain_mCtrlTreeList1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    Dim SelectItem As MC_HTREELISTITEM, iCurrentColumn As Long
    TreeList_SubItemHitTest mCtrlTreeList1, xPos, yPos, SelectItem, iCurrentColumn, True
    mCtrlTreeList1.Selection = SelectItem
    'Print "[FrmMain_mCtrlTreeList1_WM_ContextMenu]" & "xPos = " & xPos & "yPos = " & yPos & "iCurrentColumn:" & iCurrentColumn
    LastSelectItem = SelectItem
    LastClickedSubItem = iCurrentColumn
    
    Select Case CurrentInformation.intType
        Case GDT
            PopupMenu hWndForm, mnuGDT.HMENU
        Case IDT
            PopupMenu hWndForm, mnuIDT.HMENU
        Case Minifilter
            ReleaseCapture ' 这里是否存在问题?
            PopupMenu hWndForm, mnuMinifilter.HMENU
        Case FilterDriver
            PopupMenu hWndForm, mnuFilterDriver.HMENU
    End Select
End Sub

'[FrmMain.mnuRegKey]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuRegKey_WM_Command(hWndForm As hWnd, wID As ULong)
    Dim CurrentPath As StringW = "", hHKEY As HKEY
    hHKEY = GetRegPathByNodeW(CurrentNode, TreeView, CurrentPath)
    Select Case wID
        Case FrmMain_mnuRegKey_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetRegList CurrentNode, TreeView, ListView1, GetMenuCheckState(mnuRegKey, FrmMain_mnuRegKey_mnuEnableHiveAnalysis)
            TreeView.ExpandEx CurrentNode, TVE_EXPAND
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuRegKey_mnuCreateKey ' 新建
            Dim NewKey As StringW = AfxInputBox(,,, "提示", "请输入新子键名称:", "新项 #1")
            Dim hkResult As HKEY
            'Print "新子键:" & CurrentPath & "\" & NewKey
            If RegCreateKey(hHKEY, CurrentPath & "\" & NewKey, @hkResult) = ERROR_SUCCESS Then
                AfxMsg "创建成功!"
                TreeView.InsertItem CurrentNode, TVI_SORT, NewKey
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
            Else
                MyLog.PrintWin32Error "", "RegCreateKey", CurrentPath & "\" & NewKey
                AfxMsg "创建失败!"
            End If
        Case FrmMain_mnuRegKey_mnuDeleteKey ' 删除
            Dim hkResult As HKEY
            If RegDeleteTree(hHKEY, CurrentPath) = ERROR_SUCCESS Then
                AfxMsg "删除成功!"
                TreeView.DeleteItem CurrentNode
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
            Else
                MyLog.PrintWin32Error "", "RegDeleteTree", CurrentPath
                AfxMsg "删除失败!"
            End If
        Case FrmMain_mnuRegKey_mnuRenameKey ' 重命名
            Dim NewKey As StringW = AfxInputBox(,,, "提示", "请输入子键新名称:", "新项 #1")
            'Print "子键新名称:" & CurrentPath & "\" & NewKey
            If RegRenameKey(hHKEY, CurrentPath, NewKey) = ERROR_SUCCESS Then
                AfxMsg "重命名成功!"
                TreeView_SetItemText(TreeView.hWnd, CurrentNode, StrPtrW(NewKey))
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
            Else
                MyLog.PrintWin32Error "", "RegRenameKey", CurrentPath & " " & NewKey
                AfxMsg "重命名失败!"
            End If
        Case FrmMain_mnuRegKey_mnuEnableHiveAnalysis ' 启用Hive分析
            SetMenuCheckState mnuRegKey, FrmMain_mnuRegKey_mnuEnableHiveAnalysis, Not GetMenuCheckState(mnuRegKey, FrmMain_mnuRegKey_mnuEnableHiveAnalysis)
    End Select
End Sub

'[FrmMain.mnuRegValue]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuRegValue_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuRegValue_mnuRefresh ' 刷新

        Case FrmMain_mnuRegValue_mnuCreate ' 新建

        Case FrmMain_mnuRegValue_mnuCreateString ' 新建字符串值

        Case FrmMain_mnuRegValue_mnuDeleteValue ' 删除

        Case FrmMain_mnuRegValue_mnuModifyValue ' 修改

    End Select
End Sub

'[FrmMain.mnuListView]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuListView_WM_Command(hWndForm As hWnd, wID As ULong)
   Static iSubItem As Long = 0
    Select Case wID
        Case FrmMain_mnuListView_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            /'If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
                If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Return
            Else
                Return
            End If'/
            Select Case CurrentInformation.intType
                'Case ForegroundProcess
                '    GetVisibleProcessList ListView1
                Case SSDT
                    iSubItem = 3
                    GetSSDT ListView1
                Case ShadowSSDT
                    iSubItem = 3
                    GetSSSDT ListView1
                Case HalDispatch
                    iSubItem = 2
                    GetHalDispatchTable ListView1
                Case HalPrivateDispatch
                    iSubItem = 2
                    GetHalPrivateDispatchTable ListView1
                Case ObjectHook
                    iSubItem = 1
                    GetObjectInfo ListView1
                /'Case WfpCallout
                    iSubItem = 2
                    'If GetSystemVersion <> "Windows 11 24H2" AndAlso GetSystemVersion <> "Windows 10 22H2" AndAlso AfxMsg("暂不支持的版本,是否执意继续?",, MB_YESNO) = IDNO Then Exit Sub
                    GetWfpCalloutList ListView1
                Case WfpFilter
                    GetWfpFilterList ListView1'/
            End Select
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuListView_mnuEditMemory ' 查看/编辑内存
            Dim Addr As ULONG64 = ValULng(FF_Replace(ListView1.GetItemText(LastClickedItem, iSubItem), "0x", "&H"))
            'Print "0x" & WHex(Addr)
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
   End Select
End Sub

'[FrmMain.mnuWinsockSPI]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuWinsockSPI_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuWinsockSPI_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetWinsockSPIList ListView1
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuWinsockSPI_mnuRemove ' 移除
            If RemoveWinsockSPI(ListView1.GetItemText(ListView1.SelectedItem, 4)) Then
                AfxMsg "移除成功!"
                DeleteItemEx ListView1, ListView1.SelectedItem
            Else
                AfxMsg "移除失败!"
            End If
        Case FrmMain_mnuWinsockSPI_mnuCopyRowData ' 复制整行数据

   End Select
End Sub

'[FrmMain.mnuKernelThread]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuKernelThread_WM_Command(hWndForm As hWnd, wID As ULong)
    Dim dwThreadId As DWORD = ValUInt(ListView1.GetItemText(ListView1.SelectedItem, 0))
    Select Case wID
        Case FrmMain_mnuKernelThread_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetKernelThreadList ListView1
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuKernelThread_mnuKillThread ' 结束线程
            If KillThread(dwThreadId) Then
                AfxMsg "结束线程成功!"
                DeleteItemEx ListView1, ListView1.SelectedItem
            Else
                AfxMsg "结束线程失败!"
            End If
        Case FrmMain_mnuKernelThread_mnuSuspendThread ' 挂起线程
            If MySuspendThread(dwThreadId) Then
                AfxMsg "挂起线程成功!"
                GetKernelThreadList ListView1
            Else
                AfxMsg "挂起线程失败!"
            End If
        Case FrmMain_mnuKernelThread_mnuResumeThread ' 恢复线程
            If MyResumeThread(dwThreadId) Then
                AfxMsg "恢复线程成功!"
                GetKernelThreadList ListView1
            Else
                AfxMsg "恢复线程失败!"
            End If
        Case FrmMain_mnuKernelThread_mnuViewThreadStack ' 查看线程栈
            Dim CurrentInfo_New As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
            CurrentInfo_New->ProcessId = 4
            CurrentInfo_New->ThreadId = ValUInt(ListView1.GetItemText(ListView1.SelectedItem, 0))
            CurrentInfo_New->intType = ThreadCallStack
            FrmListView.Show,, Cast(Integer, CurrentInfo_New)
    End Select
End Sub

'[FrmMain.mnuFilterDriver]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuFilterDriver_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuFilterDriver_mnmuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            lblNum.Caption = "数量:" & EnumAttachDevices(mCtrlTreeList1)
        Case FrmMain_mnuFilterDriver_mnuRemoveFilter ' 移除过滤设备
            If mCtrlTreeList1.GetChildCount(mCtrlTreeList1.Selection) = 0 Then
                'Print "无子项!"
            Else
                Dim pDeviceObject As ULONG64 = ValULng(FF_Replace(mCtrlTreeList1.GetItemText(LastSelectItem, 4), "0x", "&H"))
                'Print "pDeviceObject:0x" & WHex(pDeviceObject)
                If pDeviceObject <> 0 Then
                    IoControl hDrv, IOCTL_RemoveAttachedDevice, @pDeviceObject, SizeOf(pDeviceObject)
                    AfxMsg "摘除成功!"
                    mCtrlTreeList1.DeleteItem mCtrlTreeList1.Selection
                End If
            End If
        Case FrmMain_mnuFilterDriver_mnuViolentRemoveFilter ' 暴力摘除过滤设备

    End Select
End Sub

'[FrmMain.mnuMinifilter]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuMinifilter_WM_Command(hWndForm As hWnd,wID As ULong)
    Select Case wID
        Case FrmMain_mnuMinifilter_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            lblNum.Caption = "数量:" & EnumMinifilter(mCtrlTreeList1)
        Case FrmMain_mnuMinifilter_mnuViewInstance ' 查看绑定实例
            Dim FilterName As WString * MAX_PATH = mCtrlTreeList1.GetItemText(LastSelectItem, 0)
            Dim CurrentInfo As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
            CurrentInfo->intType = MinifilterInstances
            CurrentInfo->CurrentDriver.DriverName = FilterName
            'Print "FilterName:" & FilterName
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuMinifilter_mnuRemoveFilter ' 移除过滤器
            Dim FilterName As WString * MAX_PATH = mCtrlTreeList1.GetItemText(LastSelectItem, 0)
            'Print "FilterName:" & FilterName
            DetachAllFilterInstances @FilterName
            If UnloadFilter(@FilterName) Then
                AfxMsg "移除成功!"
                mCtrlTreeList1.DeleteItem LastSelectItem
            Else
                AfxMsg "移除失败!"
            End If
        Case FrmMain_mnuMinifilter_mnuViolentRemoveFilter ' 暴力摘除过滤器

    End Select

End Sub

'[FrmMain.mnuFile]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuFile_WM_Command(hWndForm As hWnd,wID As ULong)
    Dim CurrentPath As StringW
    GetPathByNodeW CurrentNode, TreeView, CurrentPath
    Select Case wID
        Case FrmMain_mnuFile_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetFileList CurrentNode, TreeView, ListView1, False, True, GetMenuCheckState(mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
            'TreeView.ExpandEx CurrentNode, TVE_EXPAND
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuFile_mnuCreateFile ' 新建
            Dim hFile As HANDLE
            Dim FileName As String = AfxInputBox(hWndForm,,, "提示", "请输入文件名:")
            hFile = CreateFileW(CurrentPath & FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
            If (hFile > 0) Then
                Dim nFileSize As LARGE_INTEGER
                GetFileSizeEx hFile, @nFileSize
                AddItemColListEx ListView1, 3,,, WStr(FileName), WStr(""), WStr(nFileSize.HighPart * (2 ^ 32) + nFileSize.LowPart)
                CommitListViewView ListView1
                'GetFileList CurrentNode, TreeView, ListView1
                AfxMsg "创建文件成功!"
                SaveCurrentListViewState ListView1, CurrentInformation.intType
                CloseHandle hFile
            End If
        Case FrmMain_mnuFile_mnuCopyTo ' 复制到
            'CopyFileToClipboard FilePath
            Dim SourcePath As StringW = CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            Dim TargetPath As StringW = AfxInputBox(,,,"提示", "目标路径")
            
            If RightW(SourcePath, 1) = "\" Then SourcePath = LeftW(SourcePath, LenW(SourcePath) - 1)
            If RightW(TargetPath, 1) <> "\" Then TargetPath = TargetPath & "\"
            TargetPath = TargetPath & GetNameByPath(SourcePath)
            'Print "SourcePath:" & SourcePath & " TargetPath:" & TargetPath
            If CopyFile(SourcePath, TargetPath, True) <> 0 Then
                AfxMsg "复制成功!"
            Else
                AfxMsg "复制失败!"
            End If
        Case FrmMain_mnuFile_mnuForceCopyTo ' 强制复制到...
            Dim SourcePath As StringW = CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            Dim TargetPath As StringW = AfxInputBox(,,, "目标路径")
            
            If RightW(SourcePath, 1) = "\" Then SourcePath = LeftW(SourcePath, LenW(SourcePath) - 1)
            If RightW(TargetPath, 1) <> "\" Then TargetPath = TargetPath & "\"
            TargetPath = TargetPath & GetNameByPath(SourcePath)
            'Print "SourcePath:" & SourcePath & " TargetPath:" & TargetPath
            If IsDriverLoaded Then
                If ForceCopyFolder("\??\" & SourcePath, "\??\" & TargetPath) Then AfxMsg "复制成功!" Else AfxMsg "复制失败!"
            Else
                If MyCopyFile(SourcePath, TargetPath) Then AfxMsg "复制成功!" Else AfxMsg "复制失败!"
            End If
        'Case FrmMain_mnuFile_mnuPaste ' 粘贴

        Case FrmMain_mnuFile_mnuDelete ' 删除
            Dim Path As StringW, ret As NTSTATUS
            Path = "\??\" & CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            ret = MyDeleteFile(Path)
            If NT_SUCCESS(ret) Then
                DeleteItemEx ListView1, ListView1.SelectedItem
                AfxMsg "删除文件(夹)成功!"
            Else
                AfxMsg "删除文件(夹)失败!Status=" & WHex(ret)
            End If
        Case FrmMain_mnuFile_mnuForceDelete ' 强制删除
            Dim strFile As LPWSTR = Allocate(MAX_PATH * SizeOf(WString))
            If (strFile = NULL) Then
                'Print "[FrmMain_mnuFile_mnuForceDelete]分配内存失败"
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
                    'Print "[DeleteFile]GetLastError:" & WinErrorMsg(GetLastError) & GetLastError
                End If
            End If
            Deallocate strFile
            DeleteItemEx ListView1, ListView1.SelectedItem
        Case FrmMain_mnuFile_mnuViewFileStream ' 查看文件流
            Dim CurrentInfo As CURRENT_INFORMATION Ptr = Allocate(SizeOf(CURRENT_INFORMATION))
            CurrentInfo->FilePath = CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            CurrentInfo->intType = FileStream
            FrmListView.Show,, Cast(Integer, CurrentInfo)
        Case FrmMain_mnuFile_mnuLittleCopy ' 复制单格数据
            'Print "最终索引："; LastClickedItem; "  "; LastClickedSubItem

            ' ==========================================
            ' 3. 复制到剪贴板
            ' ==========================================
            If (LastClickedItem >= 0 AndAlso LastClickedItem <= ListView1.ItemCount - 1) AndAlso _
               (LastClickedSubItem >= 0 AndAlso LastClickedSubItem <= ListView1.ColumnCount - 1) Then
                CopyDataToClipboard ListView1.GetItemText(LastClickedItem, LastClickedSubItem)
            End If
    End Select
End Sub

'[FrmMain.mnuFolder]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuFolder_WM_Command(hWndForm As hWnd, wID As ULong)
    Dim CurrentPath As StringW
    GetPathByNodeW CurrentNode, TreeView, CurrentPath
    Select Case wID
        Case FrmMain_mnuFolder_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetFileList CurrentNode, TreeView, ListView1, True, True, GetMenuCheckState(mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
            TreeView.ExpandEx CurrentNode, TVE_EXPAND
            lblNum.Caption = "数量:" & WStr(ListView1.ItemCount)
        Case FrmMain_mnuFolder_mnuCreateFolder ' 新建
            Dim FolderName As String = AfxInputBox(NULL,,,"提示","请输入文件夹名:")
            If (MkDir(CurrentPath & FolderName) = 0) Then
                TreeView.InsertItem CurrentNode, TVI_SORT, FolderName
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
                'GetFileList CurrentNode,TreeView,ListView1
                AfxMsg "创建文件夹成功!"
            Else
                'Print "新建文件夹:" & Err
                AfxMsg "创建文件夹失败!"
            End If
        Case FrmMain_mnuFolder_mnuCopyFolder ' 复制
            Dim SourcePath As StringW = CurrentPath
            Dim TargetPath As StringW = AfxInputBox(,,,"提示", "目标路径")
            
            If RightW(TargetPath, 1) <> "\" Then TargetPath = TargetPath & "\"
            TargetPath = TargetPath & GetNameByPath(SourcePath)
            
            If FB_ShellCopyFile(SourcePath, TargetPath, NULL) <> 0 Then
                AfxMsg "复制成功!"
            Else
                AfxMsg "复制失败!"
            End If
        Case FrmMain_mnuFolder_mnuForceCopyTo ' 强制复制到...
            Dim SourcePath As StringW = CurrentPath & ListView1.GetItemText(ListView1.SelectedItem, 0)
            Dim TargetPath As StringW = AfxInputBox(,,, "目标路径")
            
            If RightW(SourcePath, 1) = "\" Then SourcePath = LeftW(SourcePath, LenW(SourcePath) - 1)
            If RightW(TargetPath, 1) <> "\" Then TargetPath = TargetPath & "\"
            TargetPath = TargetPath & GetNameByPath(SourcePath)
            'Print "SourcePath:" & SourcePath & " TargetPath:" & TargetPath
            If IsDriverLoaded Then
                If ForceCopyFolder("\??\" & SourcePath, "\??\" & TargetPath) Then AfxMsg "复制成功!" Else AfxMsg "复制失败!"
            Else
                If MyCopyFile(SourcePath, TargetPath) Then AfxMsg "复制成功!" Else AfxMsg "复制失败!"
            End If
        Case FrmMain_mnuFolder_mnuDeleteFolder ' 删除
            If RmDir(CurrentPath) = 0 Then
                AfxMsg "删除文件夹成功!"
                TreeView.DeleteItem CurrentNode
                SaveCurrentTreeViewState TreeView, CurrentInformation.intType
            Else
                'Print "删除文件夹:" & Err
                AfxMsg "删除文件夹失败!"
            End If
        Case FrmMain_mnuFolder_mnuForceDeleteFolder ' 强制删除
            Dim strFolder As LPWSTR = Allocate(MAX_PATH * SizeOf(WString))
            If (strFolder = NULL) Then
                'Print "[FrmMain_mnuFolder_mnuForceDelete]分配内存失败"
                Exit Sub
            End If
            *strFolder = "\??\" & CurrentPath
            If IsDriverLoaded Then
                IoControl hDrv, IOCTL_DeleteFileByXCB, strFolder, MAX_PATH * SizeOf(Wstring)
                DeleteFile strFolder
                'Print "[DeleteFile]GetLastError:" & WinErrorMsg(GetLastError) & GetLastError
            End If
            Deallocate strFolder
            TreeView.DeleteItem CurrentNode
            SaveCurrentTreeViewState TreeView, CurrentInformation.intType
        Case FrmMain_mnuFolder_mnuEnablePhysicalAnalyze ' 是否物理磁盘分析
            SetMenuCheckState mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze, Not GetMenuCheckState(mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
   End Select
End Sub

'[FrmMain.mCtrlTreeList1]事件 : 按下鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_mCtrlTreeList1_WM_LButtonDown(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Dim SelectItem As MC_HTREELISTITEM, SelectColumn As Long
    TreeList_SubItemHitTest mCtrlTreeList1, xPos, yPos, SelectItem, SelectColumn, False
    If SelectItem <> NULL AndAlso (SelectColumn >= 0 AndAlso SelectColumn <= mCtrlTreeList1.GetColumnCount - 1) Then
        'Print "SelectItem:" & mCtrlTreeList1.GetItemText(SelectItem, 0) & " SelectColumn:" & SelectColumn
        LastSelectItem = SelectItem
        LastClickedSubItem = SelectColumn
    End If
End Sub

'[FrmMain.mnuColumn]事件 : 点击了动态列选择菜单项
'hWndForm 当前窗口的句柄
'wID      菜单项命令ID = IDM_SELECT_COLUMN_BASE + 列索引
Sub FrmMain_mnuColumn_WM_Command(hWndForm As hWnd, wID As ULong)
    'Print "wID:" & wID
    Dim colIdx As Long = CLng(wID) - CLng(IDM_SELECT_COLUMN_BASE)
    If colIdx < 0 Then Exit Sub
    
    Dim intType As Interface = CurrentInformation.intType
    Dim cfg As ColumnConfig Ptr = @g_ColumnConfig(intType)
    If cfg->Count = 0 Then Exit Sub
    If colIdx >= cfg->Count Then Exit Sub
    
    ' 切换可见性
    Dim curVisible As Boolean = GetColumnVisible(intType, colIdx)
    SetColumnVisible intType, colIdx, Not curVisible, ListView1.hWnd
End Sub

'[FrmMain.mnuWfpCallout]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuWfpCallout_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_mnuWfpCallout_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetWfpCalloutList ListView1
            lblNum.Caption = "数量:" & ListView1.ItemCount
        Case FrmMain_mnuWfpCallout_mnuEditMemory ' 查看/编辑内存
            Dim Addr As ULONG64 = ValULng(FF_Replace(ListView1.GetItemText(LastClickedItem, 2), "0x", "&H"))
            'Print "0x" & WHex(Addr)
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
   End Select
End Sub

'[FrmMain.mnuWfpFilter]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuWfpFilter_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_mnuWfpFilter_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            GetWfpFilterList ListView1
            lblNum.Caption = "数量:" & ListView1.ItemCount
   End Select
End Sub

'[FrmMain.mnuGDT]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuGDT_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_mnuGDT_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            lblNum.Caption = "数量:" & GetGDT(mCtrlTreeList1)
        Case FrmMain_mnuGDT_FrmMain_mnuGDT_mnuEditMemory ' 查看/编辑内存
            Dim Addr As ULONG64 = ValULng(FF_Replace(mCtrlTreeList1.GetItemText(LastSelectItem, 1), "0x", "&H"))
            'Print "0x" & WHex(Addr)
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
   End Select
End Sub

'[FrmMain.mnuIDT]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuIDT_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_mnuIDT_mnuRefresh ' 刷新
            lblNum.Caption = "正在刷新..."
            lblNum.Caption = "数量:" & GetGDT(mCtrlTreeList1)
        Case FrmMain_mnuIDT_mnuEditMemory ' 查看/编辑内存
            Dim Addr As ULONG64 = ValULng(FF_Replace(mCtrlTreeList1.GetItemText(LastSelectItem, 3), "0x", "&H"))
            'Print "0x" & WHex(Addr)
            Dim MemoryInfo As MemoryStruct Ptr = Allocate(SizeOf(MemoryStruct))
            MemoryInfo->Addr = Cast(PVOID, Addr)
            MemoryInfo->dwProcessId = 0
            FrmMemoryEditor.Show,, Cast(Integer, MemoryInfo)
   End Select
End Sub





