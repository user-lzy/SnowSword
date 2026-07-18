#VisualFreeBasic_Form#  Version=5.9.7
Locked=0

[Form]
Name=FrmLog
Help=False
ClassStyle=CS_VREDRAW,CS_HREDRAW,CS_DBLCLKS
ClassName=
WinStyle=WS_THICKFRAME,WS_CAPTION,WS_SYSMENU,WS_MINIMIZEBOX,WS_MAXIMIZEBOX,WS_CLIPSIBLINGS,WS_CLIPCHILDREN,WS_VISIBLE,WS_EX_WINDOWEDGE,WS_EX_CONTROLPARENT,WS_EX_LEFT,WS_EX_LTRREADING,WS_EX_RIGHTSCROLLBAR,WS_POPUP,WS_SIZEBOX
Style=3 - 常规窗口
Icon=
Caption=日志
StartPosition=1 - 屏幕中心
WindowState=0 - 正常
Enabled=True
Repeat=False
Left=0
Top=0
Width=616
Height=448
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

[RichEdit]
Name=txtLog
Help=
Index=-1
Style=3 - 凹边框
TextScrollBars=3 - 垂直和水平
Text=
Enabled=True
Visible=True
MaxLength=0
Font=微软雅黑,9,0
TextAlign=0 - 左对齐
Locked=True
HideSelection=False
Multiline=True
SaveSel=True
NoScroll=False
AutoHScroll=True
AutoVScroll=True
ContextMenu=True
LeftMargin=0
RightMargin=0
Left=0
Top=0
Width=600
Height=410
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

Dim Shared strLog As StringW = ""

' 可以改进为EM_STREAMIN
Sub SafeSetRichEditTextW(hRichEdit As HWND, text As StringW)

    Const CHUNK_CHARS = 32768

    Dim totalChars As Integer = LenW(text)
    If totalChars = 0 Then
        SendMessage(hRichEdit, WM_SETTEXT, 0, Cast(LPARAM, @""))
        Exit Sub
    End If

    ' RichEdit 取消文本限制
    SendMessage(hRichEdit, EM_EXLIMITTEXT, 0, -1)

    ' 清空内容
    SendMessage(hRichEdit, WM_SETTEXT, 0, Cast(LPARAM, @""))

    ' 禁止重绘
    SendMessage(hRichEdit, WM_SETREDRAW, FALSE, 0)

    ' 光标移动到末尾（仅一次）
    Dim cr As CHARRANGE
    cr.cpMin = -1
    cr.cpMax = -1
    SendMessage(hRichEdit, EM_EXSETSEL, 0, Cast(LPARAM, @cr))

    ' 只获取一次指针
    Dim pText As UShort Ptr = Cast(UShort Ptr, StrPtrW(text))

    ' 缓冲区放到循环外
    Dim wsz As WString * (CHUNK_CHARS + 1)

    Dim i As Integer

    For i = 0 To totalChars - 1 Step CHUNK_CHARS

        Dim chunkLen As Integer

        If i + CHUNK_CHARS <= totalChars Then
            chunkLen = CHUNK_CHARS
        Else
            chunkLen = totalChars - i
        End If

        memcpy(@wsz[0], pText + i, chunkLen * SizeOf(UShort))

        wsz[chunkLen] = 0

        SendMessage( _
            hRichEdit, _
            EM_REPLACESEL, _
            FALSE, _
            Cast(LPARAM, StrPtr(wsz)) _
        )

    Next

    SendMessage(hRichEdit, WM_SETREDRAW, TRUE, 0)

    InvalidateRect(hRichEdit, NULL, TRUE)
    UpdateWindow(hRichEdit)

End Sub

'[FrmLog]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmLog_Shown(hWndForm As hWnd, UserData As Integer)
    SafeSetRichEditTextW txtLog.hWnd, strLog
End Sub



