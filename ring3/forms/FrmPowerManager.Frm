#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmPowerManager
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=电源管理
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=False
Left=0
Top=0
Width=300
Height=182
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

[CheckBox]
Name=Check3
Help=
Index=-1
Style=0 - 标准
Caption=重启
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=190
Top=100
Width=80
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[CheckBox]
Name=Check2
Help=
Index=-1
Style=0 - 标准
Caption=注销
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=90
Top=100
Width=90
Height=20
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
Caption=关机
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=10
Top=100
Width=80
Height=20
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
Caption=操作
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
Left=0
Top=70
Width=280
Height=70
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[ComboBox]
Name=Combo1
Help=
Index=-1
Style=0 - 标签和下拉框
Custom=
OwnDraw=0 - 系统绘制
LabelHeight=20
ItemHeight=18
HasString=False
Sorted=False
NoHeight=False
AutoHscroll=True
MaxLength=0
Uppercase=False
Lowercase=False
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,5
Font=微软雅黑,9,0
Left=10
Top=30
Width=260
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Frame]
Name=Frame1
Help=
Index=-1
Caption=类型
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
Left=0
Top=10
Width=280
Height=50
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False


[AllCode]
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。

Dim Shared FrmPowerManager_nWidth As Long, FrmPowerManager_nHeight As Long

Function FrmPowerManager_SubclassProc(ByVal hWnd As HWND, ByVal uMsg As UINT, ByVal wParam As WPARAM, ByVal lParam As LPARAM, ByVal uIdSubclass As UINT_PTR, ByVal dwRefData As DWORD_PTR) As LRESULT
    Select Case uMsg
        Case WM_SIZING
            Dim pRect As RECT Ptr = Cast(RECT Ptr, lParam)
            pRect->Right = pRect->Left + FrmPowerManager_nWidth
            pRect->bottom = pRect->top + FrmPowerManager_nHeight
            Return 1

        Case WM_NCDESTROY
            RemoveWindowSubclass hWnd, @FrmPowerManager_SubclassProc, uIdSubclass
    End Select

    Return DefSubclassProc(hWnd, uMsg, wParam, lParam)
End Function

'[FrmPowerManager]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmPowerManager_Shown(hWndForm As hWnd,UserData As Integer)
    Combo1.AddItem "普通"
    Combo1.AddItem "强制"
    Combo1.AddItem "驱动"
    Me.hWndForm = hWndForm
    FrmPowerManager_nWidth = Me.Width
    FrmPowerManager_nHeight = Me.Height
    SetWindowSubclass Me.hWnd, @FrmPowerManager_SubclassProc, 1, 0
End Sub

'[FrmPowerManager.Check1]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmPowerManager_Check1_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If AfxMsg("确认关机?", "警告", MB_YESNO) <> IDYES Then Exit Sub
    Select Case Combo1.ListIndex
        Case 0
            MyShutdown
        Case 1
            ForceShutdown
    End Select
End Sub

'[FrmPowerManager.Check3]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmPowerManager_Check3_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If AfxMsg("确认重启?", "警告", MB_YESNO) <> IDYES Then Exit Sub
    Select Case Combo1.ListIndex
        Case 0
            Reboot
        Case 1
            ForceReboot
    End Select
End Sub




