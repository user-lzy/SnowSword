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
Width=627
Height=425
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
Left=520
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
Left=430
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
TextScrollBars=3 - 垂直和水平
Text=
Enabled=True
Visible=True
MaxLength=0
ForeColor=SYS,8
BackColor=SYS,5
Font=Consolas,9
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
Width=600
Height=220
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
Width=610
Height=240
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
Top=330
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
Left=410
Top=330
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
Top=370
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
Top=370
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
Left=480
Top=370
Width=130
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
ToolTip=
ToolTipBalloon=False

[Option]
Name=optHex
Help=
Index=-1
Style=0 - 标准
Caption=Hex
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=0 - 未选择
Multiline=True
GroupName=OptionGroup1
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=10
Top=300
Width=120
Height=20
Layout=0 - 不锚定
MousePointer=0 - 默认
Tag=
Tab=True
ToolTip=
ToolTipBalloon=False
AcceptFiles=False

[Option]
Name=optDisasm
Help=
Index=-1
Style=0 - 标准
Caption=Disasm
TextAlign=3 - 中左对齐
Alignment=0 - 文本在左边
Value=1 - 选择
Multiline=True
GroupName=OptionGroup1
Enabled=True
Visible=True
ForeColor=SYS,8
BackColor=SYS,25
Font=微软雅黑,9,0
Left=430
Top=300
Width=170
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
Caption=显示方式
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
Top=280
Width=610
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

Dim Shared FrmMemoryEditor_PID As DWORD = 0

' ==============================================
' 终极版：仅使用你指定的Hex函数 + 修复所有问题
' ==============================================
Sub UpdateTextBoxFromBytes( _
    ByVal TextBox As Class_TextBox, _
    bytes() As UByte, _
    ByVal disasm As Boolean = False, _
    ByVal baseAddr As UInteger = 0 _
)
    If UBound(bytes) < 0 Then
        TextBox.Text = "无数据"
        Exit Sub
    End If
    
    Dim As String outputStr
    Dim As Integer arrLen = UBound(bytes) - LBound(bytes) + 1
    
    ' ============================
    ' Hex 视图（100%修复：两位标准格式+完美对齐）
    ' ============================
    If disasm = False Then
        Dim As Integer i = 0
        While i < arrLen
            ' 地址：0x + 16位标准十六进制（用你指定的Hex函数）
            Dim addr As String = "0x" & Hex(baseAddr + i, 16)
            outputStr += addr & "  "
            
            ' Hex数据：固定2位十六进制，16字节一行
            Dim hexLine As String
            For j As Integer = 0 To 15
                If i + j < arrLen Then
                    ' ✅ 核心：用你指定的Hex函数，转2位十六进制
                    hexLine += Hex(@bytes(i+j), 2) & " "
                Else
                    hexLine += "   "
                End If
            Next
            outputStr += Left(hexLine, 48) & " "
            
            ' ASCII列
            Dim asciiLine As String
            For j As Integer = 0 To 15
                If i + j < arrLen Then
                    Dim c As UByte = bytes(i + j)
                    asciiLine += Iif(c >= 32 And c <= 126, Chr(c), ".")
                End If
            Next
            outputStr += asciiLine & !"\r\n"
            i += 16
        Wend
    
    ' ============================
    ' 反汇编视图（无粘连+完美对齐）
    ' ============================
    Else
        Dim u As ud
        ud_init(@u)
        ud_set_mode(@u, 64)
        ud_set_syntax(@u, UD_SYN_INTEL)
        ud_set_pc(@u, baseAddr)
        ud_set_input_buffer(@u, @bytes(LBound(bytes)), arrLen)
        
        Const HEX_COL_WIDTH = 20

        While ud_disassemble(@u)
            Dim As Const ZString Ptr asmText = ud_insn_asm(@u)
            Dim As Const ZString Ptr hexText = ud_insn_hex(@u)
            Dim As UInt64 offset = ud_insn_off(@u)
            
            ' 统一用你指定的Hex函数
            Dim addrStr As String = "0x" & Hex(@offset, 16)
            Dim hexStr As String = *hexText
            Dim fixedHex As String = hexStr
            
            If Len(fixedHex) < HEX_COL_WIDTH Then
                fixedHex &= Space(HEX_COL_WIDTH - Len(fixedHex))
            Else
                fixedHex = Left(fixedHex, HEX_COL_WIDTH)
            End If
            
            outputStr &= addrStr & " " & fixedHex & *asmText & !"\r\n"
        Wend
    End If
    
    TextBox.Text = outputStr
End Sub

'[FrmMemoryEditor.CmdRead]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryEditor_CmdRead_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If FrmMemoryEditor_PID = 4 Then FrmMemoryEditor_PID = 0
    
    ' 读取参数初始化（共用逻辑）
    Dim bytData() As UByte
    Dim dwSize As DWORD = ValUInt(FF_Replace(txtSize.Text, "0x", "&H"))
    If dwSize = 0 Then
        AfxMsg "读取大小不能为0！"
        Exit Sub
    End If
    ReDim bytData(dwSize - 1) As UByte
    Dim readAddr As UInteger = ValULng(FF_Replace(txtAddr.Text, "0x", "&H"))
    
    ' ==============================================
    ' 逻辑分支1：驱动已加载 → 直接使用驱动读取
    ' ==============================================
    If IsDriverLoaded = True Then
        Dim stMemory As MemoryStruct
        stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, FrmMemoryEditor_PID))
        stMemory.Addr = Cast(PVOID, readAddr)
        stMemory.dwSize = dwSize
        stMemory.pData = @bytData(0)
        SetLastError 0
        
        If IoControl(hDrv, IOCTL_ReadProcessMemory, @stMemory, SizeOf(MemoryStruct)) = 0 Then
            Print "[驱动读取]IoControl失败：" & WinErrorMsg(GetLastError) & GetLastError
            AfxMsg "驱动读取内存失败！"
        Else
            UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, readAddr
        End If
        Exit Sub
    End If
    
    ' ==============================================
    ' 逻辑分支2：驱动未加载 → 优先使用系统API ReadProcessMemory
    ' ==============================================
    Dim hProcess As HANDLE = OpenProcess(PROCESS_VM_READ, FALSE, FrmMemoryEditor_PID)
    If hProcess = INVALID_HANDLE_VALUE Then
        Print "[API读取]OpenProcess失败：" & WinErrorMsg(GetLastError)
    Else
        Dim bytesRead As SIZE_T = 0
        ' 调用系统原生ReadProcessMemory
        If ReadProcessMemory(hProcess, Cast(LPCVOID, readAddr), @bytData(0), dwSize, @bytesRead) And bytesRead = dwSize Then
            ' API读取成功 → 直接显示
            UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, readAddr
            CloseHandle(hProcess)
            Exit Sub
        Else
            Print "[API读取]ReadProcessMemory失败：" & WinErrorMsg(GetLastError)
        End If
        CloseHandle(hProcess)
    End If
    
    ' ==============================================
    ' API读取失败 → 弹窗询问是否使用驱动读取
    ' ==============================================
    If AfxMsg("系统API读取内存失败，是否尝试加载驱动读取？",, MB_YESNO) = IDNO Then
        Exit Sub
    End If
    
    ' ==============================================
    ' 驱动加载 + 驱动读取（原有逻辑）
    ' ==============================================
    If Not IsDriverLoaded Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "驱动加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If hDrv = INVALID_HANDLE_VALUE Then
            AfxMsg "驱动打开失败!"
            Exit Sub
        End If
        AfxMsg "驱动加载成功!"
        FrmMain.Check6.Value = True
        IsDriverLoaded = True
    End If
    
    ' 驱动读取执行
    Dim stMemory As MemoryStruct
    stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, FrmMemoryEditor_PID))
    stMemory.Addr = Cast(PVOID, readAddr)
    stMemory.dwSize = dwSize
    stMemory.pData = @bytData(0)
    SetLastError 0
    
    If IoControl(hDrv, IOCTL_ReadProcessMemory, @stMemory, SizeOf(MemoryStruct)) = 0 Then
        Print "[驱动读取]IoControl失败：" & WinErrorMsg(GetLastError) & GetLastError
        AfxMsg "驱动读取内存失败！"
    Else
        Print "optDisasm.Value:" & optDisasm.Value
        UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, readAddr
    End If
End Sub

'[FrmMemoryEditor]事件 : 窗口完全显示后。
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'UserData  来自显示窗口最后1个参数，例： Form2.Show(父窗口句柄,模式,UserData)
Sub FrmMemoryEditor_Shown(hWndForm As hWnd, UserData As Integer)
    FrmMemoryEditor_PID = CurrentPID
    txtPID.Text = WStr(FrmMemoryEditor_PID)
    txtAddr.Text = "0x" & WHex(Cast(ULONG64, UserData))
    txtSize.Text = "0x200"
    FrmMemoryEditor.Caption = "内存编辑器"
    txtProcessName.Caption = "进程名:" & GetNameByPath(GetProcessPath(FrmMemoryEditor_PID))
End Sub

'[FrmMemoryEditor.txtPID]事件 : 文本已经被修改（修改前用 EN_UPDATE
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryEditor_txtPID_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    FrmMemoryEditor_PID = ValULng(txtPID.Text)
    txtProcessName.Caption = "进程名:" & GetNameByPath(GetProcessPath(FrmMemoryEditor_PID))
End Sub


