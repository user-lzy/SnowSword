#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmMemoryEditor
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=内存编辑器
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=True
Left=0
Top=0
Width=420
Height=370
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

[Label]
Name=Label1
Help=
Index=-1
Style=0 - 无边框
Caption=内存地址:
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
Left=10
Top=10
Width=60
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtAddr
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
Left=70
Top=10
Width=170
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

[TextBox]
Name=txtSize
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
Left=311
Top=10
Width=90
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
Name=Label2
Help=
Index=-1
Style=0 - 无边框
Caption=读取大小:
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
Left=250
Top=10
Width=80
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtData
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
Left=10
Top=50
Width=390
Height=200
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
LeftMargin=0
RightMargin=0
AcceptFiles=False

[Frame]
Name=Frame1
Help=
Index=-1
Caption=内存数据
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
Left=3
Top=30
Width=400
Height=230
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Button]
Name=CmdRead
Help=
Index=-1
Caption=读取数据
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=0
Top=266
Width=200
Height=40
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False

[Button]
Name=CmdWrite
Help=
Index=-1
Caption=写入数据
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=200
Top=266
Width=200
Height=40
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtPID
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
Left=30
Top=310
Width=80
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
Name=Label3
Help=
Index=-1
Style=0 - 无边框
Caption=PID:
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
Left=0
Top=310
Width=40
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Label]
Name=txtProcessName
Help=
Index=-1
Style=0 - 无边框
Caption=进程名:
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
Left=120
Top=310
Width=130
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

/'Type MemoryStruct
    dwProcessId As HANDLE
    Addr As ULONG64
    dwSize As size_t
    pData As PVOID
End Type '/

Dim Shared FrmMemoryEditor_PID As DWORD = 0

'[FrmMemoryEditor.CmdRead]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryEditor_CmdRead_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If FrmMemoryEditor_PID = 4 Then FrmMemoryEditor_PID = 0
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    Dim bytData() As UByte
    Dim dwSize As DWORD = ValUInt(FF_Replace(txtSize.Text, "0x", "&H"))
    ReDim bytData(dwSize - 1) As UByte
    Dim strData As String
    Dim stMemory As MemoryStruct
    stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, FrmMemoryEditor_PID))
    stMemory.Addr = Cast(PVOID, ValULng(FF_Replace(txtAddr.Text, "0x", "&H")))
    stMemory.dwSize = dwSize
    stMemory.pData = @bytData(0)
    SetLastError 0
    If IoControl(hDrv, IOCTL_ReadProcessMemory, @stMemory, SizeOf(MemoryStruct)) = 0 Then
        Print "[FrmMemoryEditor_CmdRead_BN_Clicked]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
    Else
        For i As Integer = 0 To UBound(bytData)
            strData = strData & Hex(bytData(i), 2) & " "
        Next
        txtData.Text = strData
    End If
End Sub

'[FrmMemoryEditor]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmMemoryEditor_Shown(hWndForm As hWnd,UserData As Integer)
    txtAddr.Text = "0x" & WHex(Cast(ULONG64, UserData))
    txtSize.Text = "0x200"
    FrmMemoryEditor.Caption = "内存编辑器"
End Sub

'[FrmMemoryEditor.txtPID]事件 : 文本已经被修改（修改前用 EN_UPDATE
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryEditor_txtPID_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    FrmMemoryEditor_PID = ValULng(txtPID.Text)
    txtProcessName.Caption = "进程名:" & GetNameByPath(GetProcessPath(FrmMemoryEditor_PID))
End Sub