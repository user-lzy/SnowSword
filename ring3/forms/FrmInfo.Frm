#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmInfo
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=True
Left=0
Top=0
Width=602
Height=433
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

[ListView]
Name=ListView
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
Check=False
FlatScrollBars=False
FullRowSelect=True
GridLines=True
HeaderDragDrop=False
InfoTip=False
LabelTip=False
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
Left=0
Top=0
Width=590
Height=380
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[TextBox]
Name=TxtFilter1
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
Left=0
Top=380
Width=480
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
Left=481
Top=380
Width=110
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False


[AllCode]
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。

'[FrmInfo]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmInfo_Shown(hWndForm As hWnd,UserData As Integer)
    CurrentInformation.hListView = Me.hWnd
    CurrentInformation.intType = UserData
    Me.hWndForm = hWndForm
    CurrentPID = CurrentInformation.ProcessId
    'CurrentTID = CurrentInformation.ThreadId
    CurrentProcessName = GetNameByPath(GetProcessPath(CurrentPID))
    lblNum.Caption = "正在获取..."
    InitializeListView ListView, FrmMain.mCtrlTreeList1, FrmMain.TreeView '注意,别莫名其妙炸了
    
    Select Case CurrentInformation.intType
        Case ThreadCallStack
            'GetCallStack CurrentPID, CurrentTID, ListView
            'Me.Caption = "[" & CurrentProcessName & " " & CurrentPID & " TID:" & CurrentTID & "]" & " 调用栈"
    End Select
End Sub


