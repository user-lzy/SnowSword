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
' Hex 格式常量（和你的生成逻辑严格对应）
Const HEX_LINE_LENGTH As Integer = 87
Const HEX_AREA_START   As Integer = 20
Const HEX_AREA_END As Integer = 67

#define MemoryInfo_Index 0
#define OldTxtDataProc_Index 1

' 辅助函数：判断指定位置（0-based）是否在Hex列区域内
Function IsPosInHexArea(ByVal hWnd As HWND, ByVal nPos As Integer) As Boolean
    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
    If nPos < 0 Or nPos >= txtLen Then Return False
    
    Dim lineOff As Integer = nPos Mod HEX_LINE_LENGTH
    Return (lineOff >= HEX_AREA_START AndAlso lineOff <= HEX_AREA_END)
End Function

' 安全获取编辑框文本（强制ANSI，避免编码不匹配）
Function GetEditText(ByVal hWnd As HWND) As String
    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
    If txtLen <= 0 Then Return ""
    
    Dim buf As ZString Ptr = Allocate(txtLen + 1)
    If buf = 0 Then Return ""
    *buf = ""
    
    SendMessageA(hWnd, WM_GETTEXT, txtLen + 1, Cast(LPARAM, buf))
    Dim res As String = *buf
    Deallocate(buf)
    Return res
End Function

' Hex区域内自动跳过空格
Function SkipSpaceInHexArea(ByVal hWnd As HWND, ByVal curPos As Integer, ByVal isBackward As Boolean) As Integer
    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
    If curPos < 0 Or curPos >= txtLen Then Return curPos
    
    Dim txt As String = GetEditText(hWnd)
    Do
        ' 边界校验
        If isBackward AndAlso curPos < 0 Then Exit Do
        If Not isBackward AndAlso curPos >= txtLen Then Exit Do
        
        Dim ch As String = Mid(txt, curPos + 1, 1)
        If ch <> " " Then Exit Do
        
        If isBackward Then
            curPos -= 1
        Else
            curPos += 1
        End If
    Loop
    
    ' 最终边界兜底
    If curPos < 0 Then curPos = 0
    If curPos > txtLen Then curPos = txtLen
    Return curPos
End Function

' 判断选中范围是否都在Hex编辑区
Function IsSelectionInHexArea(ByVal hWnd As HWND, ByVal start As Integer, ByVal endPos As Integer) As Boolean
    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
    If start < 0 Or endPos > txtLen Then Return False
    
    For i As Integer = start To endPos - 1
        If Not IsPosInHexArea(hWnd, i) Then Return False
    Next
    Return True
End Function

' 更新自定义块光标的位置
Sub UpdateCaretPos(ByVal hWnd As HWND, ByVal charPos As Integer)
    ' 正确调用 EM_POSFROMCHAR：wParam传字符索引，lParam传0
    Dim posResult As LRESULT = SendMessageA(hWnd, EM_POSFROMCHAR, Cast(WPARAM, charPos), 0)
    ' 从返回值提取坐标：低16位x，高16位y
    Dim x As Integer = LOWORD(posResult)
    Dim y As Integer = HIWORD(posResult)
    ' 设置光标位置
    SetCaretPos(x, y)
End Sub

' 统一获取选中范围（32位无溢出）
Sub GetSel(ByVal hWnd As HWND, ByRef start As Integer, ByRef endPos As Integer)
    Dim s As UInteger, e As UInteger
    SendMessageA(hWnd, EM_GETSEL, Cast(WPARAM, @s), Cast(LPARAM, @e))
    start = s
    endPos = e
End Sub

Function TxtDataWndProc(ByVal hWnd As HWND, ByVal uMsg As UINT, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT
    Me.hWndForm = GetParent(hWnd)
    Dim OldTxtDataProc As WNDPROC = Cast(WNDPROC, Me.UserData(OldTxtDataProc_Index))
    
    ' ========== 反汇编视图：系统原生只读，不做任何自定义拦截 ==========
    If optDisasm.Value Then
        ' 所有消息全部走原生窗口过程，只读特性由 EM_SETREADONLY 系统样式保证
        ' 原生完整支持：Ctrl+C复制、Ctrl+A全选、方向键导航、右键菜单复制/全选等所有只读操作
        ' 同时天然禁止所有修改操作：键盘输入、退格删除、粘贴、剪切、右键修改等
        Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
    End If
    
    ' ========== Hex视图：编辑控制 ==========
    Select Case uMsg
        ' ==============================================
        ' 字符输入：完全接管
        ' ==============================================
        Case WM_CHAR, WM_IME_CHAR
            ' 控制字符单独处理
            If wParam < 32 Then
                Select Case wParam
                    Case VK_BACK, VK_DELETE
                        Dim selStart As Integer, selEnd As Integer
                        GetSel(hWnd, selStart, selEnd)
                        
                        ' 区域合法性校验
                        If Not IsSelectionInHexArea(hWnd, selStart, selEnd) Then
                            Return 0
                        End If
                        
                        Dim fullText As String = GetEditText(hWnd)
                        
                        If selStart <> selEnd Then
                            ' 有选中：全部替换为0
                            Mid(fullText, selStart + 1, selEnd - selStart) = String(selEnd - selStart, Asc("0"))
                            SendMessageA(hWnd, WM_SETTEXT, 0, Cast(LPARAM, StrPtr(fullText)))
                            SendMessageA(hWnd, EM_SETSEL, selStart, selStart)
                            UpdateCaretPos(hWnd, selStart)
                        Else
                            ' 无选中：计算目标位置
                            Dim targetPos As Integer = IIf(wParam = VK_BACK, selStart - 1, selStart)
                            
                            ' 边界校验
                            If targetPos < 0 OrElse targetPos >= Len(fullText) Then
                                Return 0
                            End If
                            
                            ' 退格时向左跳过空格
                            If wParam = VK_BACK Then
                                targetPos = SkipSpaceInHexArea(hWnd, targetPos, True)
                            End If
                            
                            ' 二次校验目标位置
                            If Not IsPosInHexArea(hWnd, targetPos) Then
                                Return 0
                            End If
                            
                            Mid(fullText, targetPos + 1, 1) = "0"
                            SendMessageA(hWnd, WM_SETTEXT, 0, Cast(LPARAM, StrPtr(fullText)))
                            SendMessageA(hWnd, EM_SETSEL, targetPos, targetPos)
                            UpdateCaretPos(hWnd, targetPos)
                        End If
                        Return 0
                    
                    Case VK_RETURN, VK_TAB
                        ' 禁止换行制表，破坏格式
                        Return 0
                    
                    Case Else
                        ' 其他控制字符（如Ctrl+组合）放行给原生
                        Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
                End Select
            End If
            
            ' --- 可打印字符：Hex校验 + 覆盖替换 ---
            Dim inputCh As String = UCase(Chr(wParam))
            If Not ((inputCh >= "0" AndAlso inputCh <= "9") OrElse (inputCh >= "A" AndAlso inputCh <= "F")) Then
                Return 0
            End If
            
            Dim selStart As Integer, selEnd As Integer
            GetSel(hWnd, selStart, selEnd)
            
            ' 区域校验
            If Not IsSelectionInHexArea(hWnd, selStart, selEnd) Then
                Return 0
            End If
            
            Dim fullText As String = GetEditText(hWnd)
            Dim txtLen As Integer = Len(fullText)
            Dim editPos As Integer = selStart
            
            If selStart <> selEnd Then
                ' 有选中：覆盖整个范围
                Dim fillLen As Integer = selEnd - selStart
                Mid(fullText, selStart + 1, fillLen) = String(fillLen, Asc(inputCh))
                editPos = selEnd
            Else
                ' 无选中：覆盖当前字符
                If editPos >= txtLen Then Return 0
                Mid(fullText, editPos + 1, 1) = inputCh
                editPos += 1
            End If
            
            ' 写回文本
            SendMessageA(hWnd, WM_SETTEXT, 0, Cast(LPARAM, StrPtr(fullText)))
            
            ' 光标后移并跳过空格
            If editPos < txtLen Then
                editPos = SkipSpaceInHexArea(hWnd, editPos, False)
            End If
            editPos = IIf(editPos > txtLen, txtLen, editPos)
            
            ' 最终确保光标在Hex区域内
            If Not IsPosInHexArea(hWnd, editPos) Then
                ' 跳到下一行Hex开头
                Dim curLine As Integer = editPos \ HEX_LINE_LENGTH
                editPos = curLine * HEX_LINE_LENGTH + HEX_AREA_START
                ' 兜底校验
                If editPos >= txtLen Then
                    editPos = txtLen
                End If
            End If
            
            SendMessageA(hWnd, EM_SETSEL, editPos, editPos)
            UpdateCaretPos(hWnd, editPos)
            
            Return 0
        
        ' ==============================================
        ' 方向键：校准光标，跳过空格，限制在Hex区域
        ' ==============================================
        Case WM_KEYDOWN
            Select Case wParam
                Case VK_LEFT, VK_RIGHT
                    ' 先让原生移动一步
                    Dim res As LRESULT = CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
                    
                    Dim curPos As Integer, tmp As Integer
                    GetSel(hWnd, curPos, tmp)
                    
                    Dim isBack As Boolean = (wParam = VK_LEFT)
                    curPos = SkipSpaceInHexArea(hWnd, curPos, isBack)
                    
                    ' 边界限制：确保在Hex区域内
                    If Not IsPosInHexArea(hWnd, curPos) Then
                        If isBack Then
                            ' 向左超出：跳到上一行Hex末尾
                            Dim curLine As Integer = curPos \ HEX_LINE_LENGTH
                            If curLine > 0 Then
                                curPos = (curLine - 1) * HEX_LINE_LENGTH + HEX_AREA_END
                            Else
                                curPos = HEX_AREA_START ' 第一行开头
                            End If
                        Else
                            ' 向右超出：跳到下一行Hex开头
                            Dim curLine As Integer = curPos \ HEX_LINE_LENGTH
                            Dim maxLine As Integer = (SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)) \ HEX_LINE_LENGTH
                            If curLine < maxLine Then
                                curPos = (curLine + 1) * HEX_LINE_LENGTH + HEX_AREA_START
                            Else
                                curPos = curLine * HEX_LINE_LENGTH + HEX_AREA_END ' 最后一行末尾
                            End If
                        End If
                    End If
                    
                    ' 最终位置兜底
                    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
                    If curPos < 0 Then curPos = 0
                    If curPos > txtLen Then curPos = txtLen
                    
                    SendMessageA(hWnd, EM_SETSEL, curPos, curPos)
                    UpdateCaretPos(hWnd, curPos)
                    Return res
                
                Case VK_UP, VK_DOWN
                    Dim curPos As Integer, tmp As Integer
                    GetSel(hWnd, curPos, tmp)
                    
                    Dim txtLen As Integer = SendMessageA(hWnd, WM_GETTEXTLENGTH, 0, 0)
                    Dim isUp As Boolean = (wParam = VK_UP)
                    
                    ' 计算当前行列
                    Dim curLine As Integer = curPos \ HEX_LINE_LENGTH
                    Dim lineOff As Integer = curPos Mod HEX_LINE_LENGTH
                    
                    ' 目标行边界
                    Dim targetLine As Integer = IIf(isUp, curLine - 1, curLine + 1)
                    Dim maxLine As Integer = txtLen \ HEX_LINE_LENGTH
                    If targetLine < 0 Or targetLine > maxLine Then
                        Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
                    End If
                    
                    ' 计算目标位置并校准
                    Dim targetPos As Integer = targetLine * HEX_LINE_LENGTH + lineOff
                    targetPos = SkipSpaceInHexArea(hWnd, targetPos, isUp)
                    
                    ' 确保在Hex区域内
                    If Not IsPosInHexArea(hWnd, targetPos) Then
                        targetPos = targetLine * HEX_LINE_LENGTH + HEX_AREA_START
                    End If
                    
                    ' 兜底
                    If targetPos < 0 Then targetPos = 0
                    If targetPos > txtLen Then targetPos = txtLen
                    
                    SendMessageA(hWnd, EM_SETSEL, targetPos, targetPos)
                    UpdateCaretPos(hWnd, targetPos)
                    Return 0
            End Select
            
            ' 其他按键走原生
            Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
        
        ' ==============================================
        ' 粘贴：过滤 + 覆盖
        ' ==============================================
        Case WM_PASTE
            Dim selStart As Integer, selEnd As Integer
            GetSel(hWnd, selStart, selEnd)
            
            If Not IsSelectionInHexArea(hWnd, selStart, selEnd) Then
                Return 0
            End If
            
            ' 获取剪贴板内容
            If OpenClipboard(hWnd) = 0 Then Return 0
            Dim hClip As HANDLE = GetClipboardData(CF_TEXT)
            If hClip = 0 Then
                CloseClipboard()
                Return 0
            End If
            
            Dim pClipText As ZString Ptr = GlobalLock(hClip)
            Dim clipStr As String = UCase(*pClipText)
            GlobalUnlock(hClip)
            CloseClipboard()
            
            ' 过滤仅保留Hex字符
            Dim cleanStr As String
            For i As Integer = 1 To Len(clipStr)
                Dim ch As String = Mid(clipStr, i, 1)
                If (ch >= "0" AndAlso ch <= "9") OrElse (ch >= "A" AndAlso ch <= "F") Then
                    cleanStr &= ch
                End If
            Next
            If Len(cleanStr) = 0 Then Return 0
            
            ' 覆盖替换，不超出文本长度
            Dim fullText As String = GetEditText(hWnd)
            Dim txtLen As Integer = Len(fullText)
            Dim pasteLen As Integer = Len(cleanStr)
            If selStart + pasteLen > txtLen Then
                pasteLen = txtLen - selStart
                cleanStr = Left(cleanStr, pasteLen)
            End If
            
            Mid(fullText, selStart + 1, pasteLen) = cleanStr
            SendMessageA(hWnd, WM_SETTEXT, 0, Cast(LPARAM, StrPtr(fullText)))
            
            ' 设置光标
            Dim endPos As Integer = selStart + pasteLen
            endPos = SkipSpaceInHexArea(hWnd, endPos, False)
            If endPos > txtLen Then endPos = txtLen
            
            SendMessageA(hWnd, EM_SETSEL, endPos, endPos)
            UpdateCaretPos(hWnd, endPos)
            
            Return 0
        
        ' ==============================================
        ' 焦点：自定义块光标
        ' ==============================================
        Case WM_SETFOCUS
            Dim res As LRESULT = CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
            
            ' 计算字符尺寸
            Dim hdc As HDC = GetDC(hWnd)
            Dim hFont As HFONT = Cast(HFONT, SendMessageA(hWnd, WM_GETFONT, 0, 0))
            Dim oldFont As HFONT = SelectObject(hdc, hFont)
            
            Dim charSize As SIZE
            'Dim szChar As ZString * 2 = 
            GetTextExtentPoint32(hdc, "0", 1, @charSize)
            
            SelectObject(hdc, oldFont)
            ReleaseDC(hWnd, hdc)
            
            ' 创建实心块光标
            CreateCaret(hWnd, Cast(HBITMAP, 0), charSize.cx, charSize.cy)
            ShowCaret(hWnd)
            
            ' 初始化光标位置
            Dim curPos As Integer, tmp As Integer
            GetSel(hWnd, curPos, tmp)
            UpdateCaretPos(hWnd, curPos)
            
            Return res
        
        Case WM_KILLFOCUS
            DestroyCaret()
            Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
        
        ' ==============================================
        ' 窗口销毁：恢复原窗口过程
        ' ==============================================
        Case WM_NCDESTROY
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, Cast(LONG_PTR, OldTxtDataProc))
            Me.UserData(OldTxtDataProc_Index) = 0
            Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
    End Select
    
    ' 其余消息全部走原生
    Return CallWindowProc(OldTxtDataProc, hWnd, uMsg, wParam, lParam)
End Function

' ==============================================
' 辅助函数：从Hex视图文本中解析出字节数组
' ==============================================
Function ParseBytesFromHexText(ByVal hexText As String, outBytes() As UByte) As Boolean
    Dim nPos As Integer = 1
    Dim allHex As String
    
    ' 逐行提取Hex部分
    While nPos <= Len(hexText)
        Dim crPos As Integer = InStr(nPos, hexText, !"\r\n")
        Dim strLine As String
        If crPos = 0 Then
            strLine = Mid(hexText, nPos)
            nPos = Len(hexText) + 1
        Else
            strLine = Mid(hexText, nPos, crPos - nPos)
            nPos = crPos + 2
        End If
        
        ' 跳过非地址行
        If Len(strLine) < 20 OrElse Left(strLine, 2) <> "0x" Then
            Continue While
        End If
        
        ' 提取Hex列（第21位开始，共48字符，1-based索引）
        Dim hexPart As String = Mid(strLine, 21, 48)
        ' 过滤空格，拼接所有Hex字符
        For i As Integer = 1 To Len(hexPart)
            Dim c As String = Mid(hexPart, i, 1)
            If c <> " " Then
                allHex += c
            End If
        Next
    Wend
    
    ' 合法性校验
    If Len(allHex) = 0 Then
        AfxMsg "未解析到有效Hex数据！"
        Return False
    End If
    If Len(allHex) Mod 2 <> 0 Then
        AfxMsg "Hex数据长度不完整（奇数个字符），无法解析！"
        Return False
    End If
    
    ' 字符合法性校验
    For i As Integer = 1 To Len(allHex)
        Dim c As String = UCase(Mid(allHex, i, 1))
        If Not ((c >= "0" And c <= "9") Or (c >= "A" And c <= "F")) Then
            AfxMsg "包含非法Hex字符：" & c & "（位置：" & i & "）"
            Return False
        End If
    Next
    
    ' 转换为字节数组
    Dim byteCount As Integer = Len(allHex) \ 2
    ReDim outBytes(byteCount - 1) As UByte
    For i As Integer = 0 To byteCount - 1
        Dim hexByte As String = Mid(allHex, i * 2 + 1, 2)
        outBytes(i) = ValUInt("&H" & hexByte)
    Next
    
    Return True
End Function

' ==============================================
' 终极版：仅使用指定Hex函数 + 修复所有问题
' ==============================================
Sub UpdateTextBoxFromBytes( _
    TextBox As Class_TextBox, _
    bytes() As UByte, _
    ByVal disasm As Boolean = False, _
    ByVal baseAddr As UInteger = 0 _
)
    If UBound(bytes) < 0 Then
        TextBox.Text = "无数据"
        ' 无数据时反汇编视图仍设为只读
        SendMessage TextBox.hWnd, EM_SETREADONLY, disasm, 0
        Exit Sub
    End If
    
    Dim As String outputStr
    Dim As Integer arrLen = UBound(bytes) - LBound(bytes) + 1
    
    ' ============================
    ' Hex 视图（修复：两位标准格式+完美对齐）
    ' ============================
    If disasm = False Then
        Dim As Integer i = 0
        While i < arrLen
            ' 地址：0x + 16位标准十六进制
            Dim addr As String = "0x" & Hex(baseAddr + i, 16)
            outputStr += addr & "  "
            
            ' Hex数据：固定2位十六进制，16字节一行，不足补空格
            Dim hexLine As String
            For j As Integer = 0 To 15
                If i + j < arrLen Then
                    ' 修复：直接传字节值，而非取元素地址
                    hexLine += Hex(bytes(i+j), 2) & " "
                Else
                    hexLine += "   "
                End If
            Next
            outputStr += Left(hexLine, 48) & " "
            
            ' ASCII列：可打印字符直接显示，其余用.代替
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
            
            ' 统一使用指定Hex函数格式化地址
            Dim addrStr As String = "0x" & Hex(offset, 16)
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
    ' 视图切换时自动设置只读：反汇编视图只读，Hex视图可编辑
    SendMessage TextBox.hWnd, EM_SETREADONLY, disasm, 0
End Sub

'[FrmMemoryEditor.CmdRead]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码。
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMemoryEditor_CmdRead_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    Me.hWndForm = hWndForm
    Dim MemoryInfo As MemoryStruct Ptr = Cast(MemoryStruct Ptr, Me.UserData(MemoryInfo_Index))
    If MemoryInfo->dwProcessId = 4 Then MemoryInfo->dwProcessId = 0
    
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
        stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, MemoryInfo->dwProcessId))
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
    Dim hProcess As HANDLE = OpenProcess(PROCESS_VM_READ, FALSE, Cast(DWORD, Cast(ULONG_PTR, MemoryInfo->dwProcessId)))
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
        hDrv = OpenDrv("\\.\\SnowSword", True)
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
    stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, MemoryInfo->dwProcessId))
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
End Sub

'[FrmMemoryEditor.CmdWrite]事件 : 单击
Sub FrmMemoryEditor_CmdWrite_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    Me.hWndForm = hWndForm
    Dim MemoryInfo As MemoryStruct Ptr = Cast(MemoryStruct Ptr, Me.UserData(MemoryInfo_Index))
    If MemoryInfo->dwProcessId = 4 Then MemoryInfo->dwProcessId = 0
    
    ' 反汇编视图禁止直接写入
    If optDisasm.Value = True Then
        AfxMsg "反汇编视图无法直接修改，请切换到Hex视图编辑字节后再写入。"
        Exit Sub
    End If
    
    ' 解析Hex文本为字节数组
    Dim bytData() As UByte
    If Not ParseBytesFromHexText(txtData.Text, bytData()) Then
        Exit Sub
    End If
    
    Dim dwSize As DWORD = UBound(bytData) - LBound(bytData) + 1
    Dim writeAddr As UInteger = ValULng(FF_Replace(txtAddr.Text, "0x", "&H"))
    
    ' ==============================================
    ' 逻辑分支1：驱动已加载 → 直接使用驱动写入
    ' ==============================================
    If IsDriverLoaded = True Then
        Dim stMemory As MemoryStruct
        stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, MemoryInfo->dwProcessId))
        stMemory.Addr = Cast(PVOID, writeAddr)
        stMemory.dwSize = dwSize
        stMemory.pData = @bytData(0)
        SetLastError 0
        
        If IoControl(hDrv, IOCTL_WriteProcessMemory, @stMemory, SizeOf(MemoryStruct)) = 0 Then
            Print "[驱动写入]IoControl失败：" & WinErrorMsg(GetLastError) & GetLastError
            AfxMsg "驱动写入内存失败！"
        Else
            AfxMsg "内存写入成功！"
            ' 写入成功后刷新显示，同步ASCII与反汇编
            UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, writeAddr
        End If
        Exit Sub
    End If
    
    ' ==============================================
    ' 逻辑分支2：驱动未加载 → 优先使用系统API WriteProcessMemory
    ' ==============================================
    Dim hProcess As HANDLE = OpenProcess(PROCESS_VM_WRITE Or PROCESS_VM_OPERATION, FALSE, Cast(DWORD, Cast(ULONG_PTR, MemoryInfo->dwProcessId)))
    If hProcess = INVALID_HANDLE_VALUE Then
        Print "[API写入]OpenProcess失败：" & WinErrorMsg(GetLastError)
    Else
        Dim bytesWritten As SIZE_T = 0
        If WriteProcessMemory(hProcess, Cast(LPVOID, writeAddr), @bytData(0), dwSize, @bytesWritten) And bytesWritten = dwSize Then
            AfxMsg "内存写入成功！"
            ' 写入成功后刷新显示
            UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, writeAddr
            CloseHandle(hProcess)
            Exit Sub
        Else
            Print "[API写入]WriteProcessMemory失败：" & WinErrorMsg(GetLastError)
        End If
        CloseHandle(hProcess)
    End If
    
    ' ==============================================
    ' API写入失败 → 弹窗询问是否使用驱动写入
    ' ==============================================
    If AfxMsg("系统API写入内存失败，是否尝试加载驱动写入？",, MB_YESNO) = IDNO Then
        Exit Sub
    End If
    
    ' ==============================================
    ' 驱动加载 + 驱动写入
    ' ==============================================
    If Not IsDriverLoaded Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "驱动加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword", True)
        If hDrv = INVALID_HANDLE_VALUE Then
            AfxMsg "驱动打开失败!"
            Exit Sub
        End If
        AfxMsg "驱动加载成功!"
        FrmMain.Check6.Value = True
        IsDriverLoaded = True
    End If
    
    ' 驱动写入执行
    Dim stMemory As MemoryStruct
    stMemory.dwProcessId = Cast(HANDLE, Cast(ULONG64, MemoryInfo->dwProcessId))
    stMemory.Addr = Cast(PVOID, writeAddr)
    stMemory.dwSize = dwSize
    stMemory.pData = @bytData(0)
    SetLastError 0
    
    If IoControl(hDrv, IOCTL_WriteProcessMemory, @stMemory, SizeOf(MemoryStruct)) = 0 Then
        Print "[驱动写入]IoControl失败：" & WinErrorMsg(GetLastError) & GetLastError
        AfxMsg "驱动写入内存失败！"
    Else
        AfxMsg "内存写入成功！"
        ' 同步更新显示
        UpdateTextBoxFromBytes txtData, bytData(), optDisasm.Value, writeAddr
    End If
End Sub

'[FrmMemoryEditor]事件 : 窗口完全显示后。
Sub FrmMemoryEditor_Shown(hWndForm As hWnd, UserData As Integer)
    Me.hWndForm = hWndForm
    Me.UserData(MemoryInfo_Index) = UserData
    Dim MemoryInfo As MemoryStruct Ptr = Cast(MemoryStruct Ptr, UserData)
    
    txtPID.Text = WStr(MemoryInfo->dwProcessId)
    txtAddr.Text = "0x" & WHex(Cast(ULONG64, MemoryInfo->Addr))
    txtSize.Text = "0x200"
    FrmMemoryEditor.Caption = "内存编辑器"
    txtProcessName.Caption = "进程名:" & GetNameByPath(GetProcessPath(Cast(DWORD, Cast(ULONG_PTR, MemoryInfo->dwProcessId))))
    
    Me.UserData(OldTxtDataProc_Index) = Cast(Integer, SetWindowLongPtr(txtData.hWnd, GWLP_WNDPROC, Cast(LONG_PTR, @TxtDataWndProc)))
End Sub

'[FrmMemoryEditor.txtPID]事件 : 文本已经被修改
Sub FrmMemoryEditor_txtPID_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    Me.hWndForm = hWndForm
    Dim MemoryInfo As MemoryStruct Ptr = Cast(MemoryStruct Ptr, Me.UserData(MemoryInfo_Index))
    Dim newPid As DWORD = Cast(DWORD, ValULng(txtPID.Text))
    
    MemoryInfo->dwProcessId = Cast(HANDLE, Cast(ULONG64, newPid))
    
    txtProcessName.Caption = "进程名:" & GetNameByPath(GetProcessPath(newPid))
End Sub