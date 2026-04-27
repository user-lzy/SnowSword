#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmOtherInfo
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
Width=692
Height=468
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

[TextBox]
Name=txtOtherInfo
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
Locked=True
HideSelection=True
Multiline=True
Uppercase=False
Lowercase=False
Number=False
AutoHScroll=True
AutoVScroll=True
Left=0
Top=0
Width=680
Height=430
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

'[FrmOtherInfo]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmOtherInfo_Shown(hWndForm As hWnd,UserData As Integer)
    CurrentInformation.hListView = Me.hWnd
    CurrentInformation.intType = UserData
    Me.hWndForm = hWndForm
    CurrentPID = CurrentInformation.ProcessId
    'CurrentTID = CurrentInformation.ThreadId
    CurrentProcessName = GetNameByPath(GetProcessPath(CurrentPID))
    'lblNum.Caption = "正在获取..."
    
    Select Case CurrentInformation.intType
        Case ProcessOtherInfo
            Me.Caption = "[" & CurrentProcessName & " " & CurrentPID & "]" & " 其他信息"
            GetProcessOtherInfo CurrentPID, txtOtherInfo
    End Select
End Sub


