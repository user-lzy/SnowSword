#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmFireWall
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=防火墙
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=False
Left=0
Top=0
Width=750
Height=500
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
Name=mnuSniffer
Help=
Index=-1
Menu=开始监听FrmFireWall_mnuSniffer_Sniffer0-10
Left=580
Top=190
Tag=

[PopupMenu]
Name=mnuIPScan
Help=
Index=-1
Menu=扫描存活主机FrmFireWall_mnuIPScan_mnuScanAliveHost0-10
Left=570
Top=40
Tag=

[TopMenu]
Name=TopMenu1
Help=
Menu=工具FrmFireWall_TopMenu1_mnuTool0-10{选项FrmFireWall_TopMenu1_mnuOptions0-10}
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
SingleSel=True
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
Check=True
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
Height=420
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

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
Left=0
Top=0
Width=200
Height=470
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Label]
Name=lblStatus
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
Left=630
Top=420
Width=100
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=Text1
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
Top=420
Width=420
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
LeftMargin=0
RightMargin=0
AcceptFiles=False


[AllCode]
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。

'[Form1]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmFireWall_Shown(hWndForm As hWnd,UserData As Integer)
    Dim treScan As HTREEITEM = treMain.AddItem(NULL, "扫描")
    treMain.AddItem NULL, "监听"
    'SendTCPPacket
    'GetHostNameByAddr StrPtr("10.11.23.81")
End Sub

'[Form1.treMain]事件 : 双击了鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmFireWall_treMain_NM_DBLCLK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    Dim SelectText As String = treMain.Text(treMain.Selection)
    Select Case SelectText
        Case "扫描"
            SetWindowZ ListView1.hWnd
            'ListView1.Width = 1080
            'ListView1.Height = 940
            'ListView1.Left = 400
            CurrentInformation.intType = IPScan
            InitializeListView ListView1, FrmMain.mCtrlTreeList1, FrmMain.TreeView
        Case "监听"
            SetWindowZ ListView1.hWnd
            'ListView1.Width = 1080
            'ListView1.Height = 940
            'ListView1.Left = 400
            CurrentInformation.intType = Sniffer
            InitializeListView ListView1, FrmMain.mCtrlTreeList1, FrmMain.TreeView
    End Select
    'AfxMsg GetMenuText(mnuSniffer, FrmFireWall_mnuSniffer_Sniffer)
    'SetMenuText mnuSniffer, FrmFireWall_mnuSniffer_Sniffer, "1"
   Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmFireWall.ListView1]事件 : 鼠标右键单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'xPos yPos   当前鼠标位置，相对于屏幕。就是屏幕坐标。
Sub FrmFireWall_ListView1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    Select Case CurrentInformation.intType
        Case IPScan
            PopupMenu hWndForm, mnuIPScan.HMENU
        Case Sniffer
            PopupMenu hWndForm, mnuSniffer.HMENU
    End Select
End Sub

'[FrmFireWall.mnuIPScan]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmFireWall_mnuIPScan_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmFireWall_mnuIPScan_mnuScanAliveHost ' 扫描存活主机
            Dim startIP As String, endIP As String
            Dim parts() As String
            Split AfxInputBox(,,, "提示", "请输入要扫描的IP范围(格式:192.168.1.0-255):", "192.168.1.0-255"), "-", parts()
            If UBound(parts) <> 2 Then
                AfxMsg "无效格式!"
                Exit Sub
            End If
            
            Dim ipBase As String = parts(1)
            Dim ipmax As String = parts(2)
            
            Dim ipParts() As String
            Split ipBase, ".", ipParts()
            If UBound(ipParts) <> 4 Then
                AfxMsg "无效格式!"
                Exit Sub
            End If
            
            ' 解析结束IP
            If (ValInt(ipParts(1)) < 0 Or ValInt(ipParts(1)) > 255) Or (ValInt(ipParts(2)) < 0 Or ValInt(ipParts(2)) > 255) Or (ValInt(ipParts(3)) < 0 Or ValInt(ipParts(3)) > 255) Or _
            (ValInt(ipParts(4)) < 0 Or ValInt(ipParts(4)) > 255) Or (ValInt(ipmax) < 0 Or ValInt(ipmax) > 255) Then
                AfxMsg "无效格式!"
                Exit Sub
            Else
                GetAliveHostList ipParts(1) & "." & ipParts(2) & "." & ipParts(3), ValULng(ipParts(4)), ValULng(ipmax), ListView1, lblStatus
            End If
   End Select
End Sub

'[FrmFireWall.mnuSniffer]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmFireWall_mnuSniffer_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmFireWall_mnuSniffer_Sniffer ' 监听
            If GetMenuCheckState(mnuSniffer, FrmFireWall_mnuSniffer_Sniffer) Then
                SetMenuCheckState mnuSniffer, FrmFireWall_mnuSniffer_Sniffer, False
                SetSnifferStatus ListView1, False
            Else
                SetMenuCheckState mnuSniffer, FrmFireWall_mnuSniffer_Sniffer, True
                SetSnifferStatus ListView1, True
            End If
   End Select
End Sub
'[FrmFireWall.TopMenu1]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmFireWall_TopMenu1_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmFireWall_TopMenu1_mnuOptions ' 选项

   End Select
End Sub

