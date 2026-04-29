#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmMemoryChange
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=内存属性修改
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=True
Left=0
Top=0
Width=363
Height=368
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
Name=lblInfo
Help=
Index=-1
Style=0 - 无边框
Caption=普通属性:0x01 - PAGE_NOACCESS0x02 - PAGE_READONLY0x04 - PAGE_READWRITE0x08 - PAGE_WRITECOPY0x10 - PAGE_EXECUTE0x20 - PAGE_EXECUTE_READ0x40 - PAGE_EXECUTE_READWRITE0x80 - PAGE_EXECUTE_WRITECOPY特殊属性:0x100 - PAGE_GUARD0x200 - PAGE_NOCACHE0x400 - PAGE_WRITECOMBINE
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
Top=0
Width=350
Height=230
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtOld
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
Multiline=False
Uppercase=False
Lowercase=False
Number=False
AutoHScroll=True
AutoVScroll=False
Left=50
Top=240
Width=120
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
Name=lblOld
Help=
Index=-1
Style=0 - 无边框
Caption=旧属性:
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
Top=240
Width=50
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Label]
Name=lblNew
Help=
Index=-1
Style=0 - 无边框
Caption=新属性:
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
Left=180
Top=240
Width=60
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[TextBox]
Name=txtNew
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
Left=230
Top=240
Width=120
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
Name=CmdCancel
Help=
Index=-1
Caption=取消
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=200
Top=280
Width=140
Height=40
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False

[Button]
Name=CmdOK
Help=
Index=-1
Caption=确认
TextAlign=1 - 居中
Ico=
Enabled=True
Visible=True
Default=False
OwnDraw=False
MultiLine=False
Font=微软雅黑,9,0
Left=10
Top=280
Width=140
Height=40
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

Type MEMORY_CHANGE
    Addr As UInteger
    dwSize As ULong
    OldProtect As DWORD
End Type

'Dim MemoryInfo As MEMORY_CHANGE

'[FrmMemoryChange]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmMemoryChange_Shown(hWndForm As hWnd, UserData As Integer)
    Me.hWndForm = hWndForm
    Dim mc As MEMORY_CHANGE Ptr = Cast(MEMORY_CHANGE Ptr, UserData)
    'Print "Addr:0x" & WHex(mc->Addr) & " dwSize:0x" & WHex(mc->dwSize) & " OldProtect:0x" & mc->OldProtect
    txtOld.Text = "0x" & mc->OldProtect
    Me.UserData(0) = Cast(Integer, mc)
    'Deallocate mc
End Sub

'[FrmMemoryChange]事件 : 即将关闭窗口，返回非0可阻止关闭
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
Function FrmMemoryChange_WM_Close(hWndForm As hWnd) As LResult
    Dim mc As MEMORY_CHANGE Ptr = Cast(MEMORY_CHANGE Ptr, Me.UserData(0))
    Deallocate mc
    Function = FALSE ' 返回 TRUE 阻止关闭窗口。
End Function

'[FrmMemoryChange.CmdOK]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryChange_CmdOK_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    Me.hWndForm = hWndForm
    Dim mc As MEMORY_CHANGE Ptr = Cast(MEMORY_CHANGE Ptr, Me.UserData(0))
    'Print "Addr:0x" & WHex(mc->Addr) & " dwSize:0x" & WHex(mc->dwSize) & " OldProtect:0x" & mc->OldProtect
    Dim hProcess As HANDLE = NULL
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(CurrentPID)
    Else
        hProcess = OpenProcess(PROCESS_VM_OPERATION, False, CurrentPID)
    End If
    Dim OldProtect As DWORD, NewProtect As DWORD
    NewProtect = ValUInt(FF_Replace(txtNew.Text, "0x", "&H"))
    If VirtualProtectEx(hProcess, Cast(LPVOID, mc->Addr), mc->dwSize, NewProtect, @OldProtect) <> 0 Then
        AfxMsg "修改成功!"
    Else
        MyLog.PrintWin32Error "FrmMemoryChange_CmdOK_BN_Clicked", "VirtualProtectEx"
        AfxMsg "修改失败!"
    End If
    CloseForm hWndForm
End Sub




