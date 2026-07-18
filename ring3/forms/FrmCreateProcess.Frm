#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmCreateProcess
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=运行
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=False
Left=0
Top=0
Width=413
Height=241
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
Name=txtCommandLine
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
Left=10
Top=20
Width=340
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

[Button]
Name=cmdBrowser
Help=
Index=-1
Caption=...
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=360
Top=20
Width=30
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False

[Frame]
Name=Frame1
Help=
Index=-1
Caption=命令行
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
Top=0
Width=400
Height=60
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[CheckBox]
Name=Check1
Help=
Index=-1
Style=0 - 标准
Caption=以管理员权限运行
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
Width=120
Height=40
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
Caption=以SYSTEM权限运行
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=140
Top=100
Width=180
Height=40
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
Caption=高级选项
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
Width=400
Height=90
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Button]
Name=cmdOK
Help=
Index=-1
Caption=确定
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=True
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=60
Top=170
Width=270
Height=30
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False


[AllCode]
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。

Dim Shared FrmCreateProcess_nWidth As Long, FrmCreateProcess_nHeight As Long

Function FrmCreateProcess_SubclassProc(ByVal hWnd As HWND, ByVal uMsg As UINT, ByVal wParam As WPARAM, ByVal lParam As LPARAM, ByVal uIdSubclass As UINT_PTR, ByVal dwRefData As DWORD_PTR) As LRESULT
    Select Case uMsg
        Case WM_SIZING
            Dim pRect As RECT Ptr = Cast(RECT Ptr, lParam)
            pRect->Right = pRect->Left + FrmCreateProcess_nWidth
            pRect->bottom = pRect->top + FrmCreateProcess_nHeight
            Return 1

        Case WM_NCDESTROY
            RemoveWindowSubclass hWnd, @FrmCreateProcess_SubclassProc, uIdSubclass
    End Select

    Return DefSubclassProc(hWnd, uMsg, wParam, lParam)
End Function

'[FrmCreateProcess.cmdOK]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmCreateProcess_cmdOK_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    Dim strCommandLine As StringW = txtCommandLine.Text
    Dim strProgram As StringW = strCommandLine   ' 程序名
    Dim strParameters As StringW = ""
    Dim pos1 As Integer = 0
    Dim result As HINSTANCE
    
    ' 分离程序名和参数
    pos1 = InStrW(strCommandLine, " ")
    If pos1 >= 1 Then
        strProgram = LeftW(strCommandLine, pos1 - 1)      ' "notepad"
        strParameters = RightW(strCommandLine, LenW(strCommandLine) - pos1)  ' "F:\1.txt"
    End If
    'Print strProgram & " " & strParameters
    If Check2.Value Then
        If CreateSYSTEMPriviegeProcess(NULL, IIf(strCommandLine <> "", StrPtrW(strCommandLine), NULL)) Then
            AfxMsg "创建成功!"
        Else
            AfxMsg "创建失败!"
        End If
    ElseIf Check1.Value Then
        result = ShellExecute(NULL, "runas", strProgram, IIf(strParameters <> "", StrPtrW(strParameters), NULL), NULL, SW_SHOWNORMAL)
        ' 返回值 > 32 表示成功
        If result > Cast(HINSTANCE, 32) Then
            AfxMsg "创建成功!"
        Else
            AfxMsg "创建失败!"
            MyLog.PrintLog LOG_WARN, "FrmCreateProcess_cmdOK_BN_Clicked", "ShellExecute", "result=" & result
        End If
    Else
        result = ShellExecute(NULL, "open", strProgram, IIf(strParameters <> "", StrPtrW(strParameters), NULL), NULL, SW_SHOWNORMAL)
        ' 返回值 > 32 表示成功
        If result > Cast(HINSTANCE, 32) Then
            AfxMsg "创建成功!"
        Else
            AfxMsg "创建失败!"
            MyLog.PrintLog LOG_WARN, "FrmCreateProcess_cmdOK_BN_Clicked", "ShellExecute", "result=" & result
        End If
    End If
End Sub

'[FrmCreateProcess.Check2]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmCreateProcess_Check2_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Check1.Value Then Check1.Value = 0
End Sub

'[FrmCreateProcess.Check1]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmCreateProcess_Check1_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If Check2.Value Then Check2.Value = 0
End Sub

'[FrmCreateProcess]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmCreateProcess_Shown(hWndForm As hWnd, UserData As Integer)
    Me.hWndForm = hWndForm
    FrmCreateProcess_nWidth = Me.Width
    FrmCreateProcess_nHeight = Me.Height
    SetWindowSubclass Me.hWnd, @FrmCreateProcess_SubclassProc, 1, 0
End Sub





