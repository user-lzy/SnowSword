'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


Function gFLY_GetFontHandles(mFont As String) As HFONT '由字体字符，返回字体句柄
   Dim As Long i,u=UBound(gFLY_FontNames),aa =-1
   If u > -1 Then
      For i = 0 To u
         If gFLY_FontNames(i) = mFont Then
            Return gFLY_FontHandles(i)
         End If
      Next
   End If
   aa = u + 1
   ReDim Preserve gFLY_FontNames(aa), gFLY_FontHandles(aa)
   gFLY_FontNames(aa) = mFont
   Dim pvv(5) As String, ff as Long, zz as String = mFont
   for i = 0 to 5
      ff = instr(zz, ",")
      if ff = 0 then
         pvv(i) = Trim(zz)
         exit for
      end if
      pvv(i) = Trim(.left(zz, ff -1))
      zz = Mid(zz, ff + 1)
   next
   if valint(pvv(1)) = 0 then pvv(1) = "9"
   If Len(pvv(0)) = 0 Then pvv(0) = "SimSun" '宋体
   gFLY_FontHandles(aa) = AfxCreateFont(pvv(0), ValInt(pvv(1)), -1, IIf(ValInt(pvv(2)) = 0, FW_NORMAL, FW_BOLD), ValInt(pvv(3)), ValInt(pvv(4)), ValInt(pvv(5)))

   If gFLY_FontHandles(aa) = 0 Then  '创建失败
      aa -= 1
      ReDim Preserve gFLY_FontNames(aa), gFLY_FontHandles(aa)
      Return 0
   End If
   Function = gFLY_FontHandles(aa)
End Function
Function GetCodeColorGDI(coColor As Long, defaultColor As Long = -1) As Long  '把控件特殊颜色值，转换为 GDI 色  ,返回-1 为不使用或默认
   '格式：&H01020304  02.03.04为7F时，01值0到30是系统色（25为不使用）其它时候是A通道，GDI+需要 02 04 里值对调
  If (&H00FFFFFF And coColor) = &H7F7F7F Then
      Dim f As Long = Cast(UInteger, (&HFF000000 And coColor)) Shr 24
      If f = 25 Then Return defaultColor   '不使用或默认值 
      If f < 31 Then 
          Return GetSysColor(f)  
      End If  
  End If
  Function = (&H00FFFFFF And coColor) '去掉 A 通道
End Function
Function GetCodeColorGDIplue(coColor As Long, defaultColor As Long = 0) As Long  '把控件特殊颜色值，转换为 GDI+ 色  ,返回0 为不使用或默认
 '格式：&H01020304  02.03.04为7F时，01值0到30是系统色（25为不使用）其它时候是A通道，GDI+需要 02 04 里值对调  
  Dim tColor As Long = coColor 
  If (&H00FFFFFF And coColor) = &H7F7F7F Then
      Dim f As Long = Cast(UInteger, (&HFF000000 And coColor)) Shr 24
      If f = 25 Then Return defaultColor  ' 不使用或默认值 
      If f < 31 Then 
          tColor = GetSysColor(f) Or &HFF000000 '增加 A通道，不透明，不然是全透明  
      End If  
  End If 
  '因为保存的是GDI 的颜色，GDI+ 需要调换
  Dim As UInteger c1 =(&H00FF0000 And tColor),c2 = (&H000000FF And tColor) ,c3 =(&HFF00FF00 And tColor)
  c1 Shr= 16
  c2 Shl= 16 
  Function = c1 Or c2 Or c3  
End Function
Function FLY_ResourceToIcon(ByVal ResImg As String) As HICON '资源获取图标句柄
   Dim nIcon As HICON
   Dim ffi   As Long = InStr(ResImg ,"|")
   if ffi > 0 Then ResImg = Mid(ResImg ,ffi + 1)
   if Left(ResImg ,7) = "BITMAP_" Then
      Dim nBmp As HBITMAP = LoadImageA(app.hInstance ,ResImg ,IMAGE_BITMAP ,0 ,0 ,LR_DEFAULTCOLOR)
      Dim po   As ICONINFO
      po.fIcon    = TRUE
      po.hbmColor = nBmp
      po.hbmMask  = nBmp
      nIcon       = CreateIconIndirect(@po)
      DeleteObject nBmp
   Elseif Left(ResImg ,5) = "ICON_" Or ResImg = "AAAAA_APPICON" Then
      nIcon = LoadImageA(app.hInstance ,ResImg ,IMAGE_ICON ,0 ,0 ,LR_DEFAULTCOLOR) '从资源里加载图标
   Else 
      nIcon = AfxGdipIconFromRes(App.hInstance ,ResImg)
   End if
   Function = nIcon
End Function
' =====================================================================================
' 根据Jose Roca的代码
' 为窗口的整个客户区域创建标准工具提示。
' 参数:
' - hwnd = 窗口句柄
' - strTooltipText = 工具提示文本
' - bBalloon = 气球提示 (TRUE or FALSE)
' 返回值:
'   工具提示控件的句柄
' =====================================================================================
Function FF_AddTooltip(hWndForm As hWnd, strTooltipText As Wstring, bBalloon As Long, X As Long = 0, Y As Long = 0, W As Long = 0, H As Long = 0) As hWnd
   
   If hWndForm = 0 Then Exit Function
   
   Dim hwndTT AS HWND
   Dim dwStyle As Long
   
   dwStyle = WS_POPUP OR TTS_NOPREFIX OR TTS_ALWAYSTIP
   IF bBalloon THEN dwStyle = dwStyle OR TTS_BALLOON
   hwndTT = CreateWindowExW(WS_EX_TOPMOST, "tooltips_class32", "", dwStyle, 0, 0, 0, 0, 0, Cast(HMENU, NULL), 0, ByVal Cast(LPVOID, NULL))
   
   If hwndTT = 0 Then Exit Function
   'SetWindowPos(hwndTT, null, 100, 100, 0, 0,  SWP_NOSIZE Or SWP_NOZORDER Or SWP_NOACTIVATE)
   
   Dim tti As TTTOOLINFOW
   tti.cbSize = SizeOf(tti)
   tti.uFlags = TTF_SUBCLASS
   tti.hWnd = hWndForm
   tti.hinst = GetModuleHandle(ByVal NULL)
   
   GetClientRect(hWndForm, Varptr(tti.Rect))
   tti.Rect.Left = x
   tti.rect.top = y
   if w > 0 then tti.rect.Right = x + w
   if h > 0 then tti.rect.bottom = y + h
   '// 字符串的长度不能超过80个字符，包括终止的空值
   tti.uId = 0
   Dim ff As Long = InStr(strTooltipText, WChr(13, 10) )
   if ff Then
      tti.lpszText =Cast(LPWSTR, Cast(UInteger, @strTooltipText) + ff * 2 + 2) 
      SendMessageW hwndTT, TTM_ADDTOOLW, 0, Cast(LPARAM, Varptr(tti))
      Dim nw As WString * 100 = Left(strTooltipText,ff-1)
      SendMessageW hwndTT, TTM_SETTITLEW, 0, Cast(LPARAM, @nw)
   Else
      tti.lpszText = @strTooltipText
      SendMessageW hwndTT, TTM_ADDTOOLW, 0, Cast(lParam, Varptr(tti))
   End if

   
   Function = hwndTT
   
End Function
Function FLY_DoMessagePump(pWindow AS CWindow Ptr, ByVal ShowModalFlag As Long, ByVal hWndForm As HWND, ByVal hWndParent As HWND, ByVal nFormShowState As Long, ByVal IsMDIForm As Long) As HWND
   '如果这是一个MDI子窗体，那么它不能显示为模态。
   If (GetWindowLongPtr(hWndForm, GWL_EXSTYLE) And WS_EX_MDICHILD) = WS_EX_MDICHILD Then ShowModalFlag = False
   If (GetWindowLongPtr(hWndForm, GWL_EXSTYLE) And WS_EX_NOACTIVATE) = WS_EX_NOACTIVATE And nFormShowState = SW_SHOWNORMAL Then nFormShowState = SW_SHOWNOACTIVATE
   If ShowModalFlag = True Then '模式窗口，进入消息循环处理
      '确定活动控件的顶层窗口
      While (GetWindowLongPtr(hWndParent, GWL_STYLE) And WS_CHILD) <> 0
         hWndParent = GetParent(hWndParent)
         If IsWindow(hWndParent) = 0 Then Exit While
         If (GetWindowLongPtr(hWndParent, GWL_EXSTYLE) And WS_EX_MDICHILD) <> 0 Then Exit While
      Wend
      
      '为父窗体禁用鼠标和键盘输入
      If IsWindow(hWndParent) Then EnableWindow(hWndParent, False)
      ShowWindow(hWndForm, nFormShowState)
      UpdateWindow(hWndForm)
      '主消息循环：
      Dim uMsg        As MSG
      Dim zTempString As zString * MAX_PATH
      Dim hWndP       As HWND = pWindow->hWindow
      Dim HACCEL      As HACCEL
      '获取系统版本号
      Dim vi As OSVERSIONINFO, bil As Long
      vi.dwOsVersionInfoSize = SizeOf(OSVERSIONINFO)
      GetVersionEx @vi
      If vi.dwPlatformId = VER_PLATFORM_WIN32_NT Then
         bil = vi.dwMajorVersion * 10 + vi.dwMinorVersion
      End If
      Dim moua As Point, mouh As hWnd
      
      '消息循环开始
      Do While GetMessage(@uMsg, NULL, 0, 0)
         If FF_PUMPHOOK(uMsg) = 0 Then
            HACCEL = pWindow->AccelHandle
            '处理菜单命令的快捷键
            If (hAccel = 0) OrElse (TranslateAcceleratorW(hWndP, hAccel, @uMsg)) = 0 Then '
               If IsMDIForm = True Then
                  If TranslateMDISysAccel(hWndP, @uMsg) <> 0 Then Continue Do
               End If
               If bil < 62 AndAlso uMsg.message = WM_MOUSEWHEEL Then
                  'WIn7 鼠标滚轮只在当前焦点窗口，WIn10系统是鼠标在什么窗口上滚轮在什么窗口
                  GetCursorPos(@moua)
                  mouh = WindowFromPoint(moua)
                  If mouh <> uMsg.hWnd Then
                     PostMessage(mouh, WM_MOUSEWHEEL, uMsg.wParam, uMsg.lParam)
                     Continue Do
                  End If
               End If
               If IsDialogMessageW(hWndP, @uMsg) = 0 Then
                  TranslateMessage @uMsg
                  DispatchMessage @uMsg
               End If
            End If
         End If
         If IsWindow(hWndForm) = False Then Exit Do ' 如果窗口被销毁，则退出模态消息循环（重要）。
      Loop
      Function        = Cast(hWnd, Cast(LONG_PTR, App.ReturnValue))
      App.ReturnValue = 0
   Else
      ShowWindow hWndForm, nFormShowState
      Function = hWndForm
   End If
End Function

Sub FLY_VFB_Layout_hWndForm(hWndForm As HWND) '处理控件布局
   DIM rcParent AS RECT ,rcChild AS RECT
   DIM x        AS LONG ,y       AS LONG ,xWidth AS LONG ,yHeight AS LONG
   GetClientRect(hWndForm ,@rcParent)
   rcParent.Right  = AfxUnscaleX(rcParent.Right)  ' 为自动响应DPI，全部调整为 100%DPI 时的数值
   rcParent.Bottom = AfxUnscaleY(rcParent.Bottom)
   '真实控件
   Dim zWnd  As HWND = GetWindow(hWndForm ,GW_CHILD)
   Dim nHDWP As HDWP = BeginDeferWindowPos(1) '同时更新控件位置
   While zWnd
      Dim fp As FormControlsPro_TYPE Ptr = vfb_Get_Control_Ptr(zWnd)
      If fp <> 0 AndAlso fp->anchor > 0 Then
         GetWindowRect(zWnd ,@rcChild)
         FLY_VFB_Layout_Handle(fp ,rcParent.Right ,rcParent.Bottom ,AfxUnscaleX(rcChild.Right - rcChild.Left) ,AfxUnscaleY(rcChild.Bottom - rcChild.Top) ,x ,y ,xWidth ,yHeight)
         fp->nLeft   = x
         fp->nTop    = y
         fp->nWidth  = xWidth
         fp->nHeight = yHeight
         nHDWP       = DeferWindowPos(nHDWP ,zWnd ,0 ,AfxscaleX(x) ,AfxscaleY(y) ,AfxscaleX(xWidth) ,AfxscaleY(yHeight) ,SWP_NOZORDER Or SWP_NOACTIVATE)
      End If
      zWnd = GetWindow(zWnd ,GW_HWNDNEXT)
   Wend
   
   '虚拟控件
   Dim fp As FormControlsPro_TYPE Ptr = vfb_Get_Control_Ptr(hWndForm)
   While fp    '
      if fp->anchor > 0 Then
         if fp->CtrlFocus Then '非 CW创建的控件，因为不 fp 指针不内置入窗口，真实控件查不到
            GetWindowRect(fp->CtrlFocus ,@rcChild)
            FLY_VFB_Layout_Handle(fp ,rcParent.Right ,rcParent.Bottom ,AfxUnscaleX(rcChild.Right - rcChild.Left) ,AfxUnscaleY(rcChild.Bottom - rcChild.Top) ,x ,y ,xWidth ,yHeight)
            nHDWP = DeferWindowPos(nHDWP ,fp->CtrlFocus ,0 ,AfxscaleX(x) ,AfxscaleY(y) ,AfxscaleX(xWidth) ,AfxscaleY(yHeight) ,SWP_NOZORDER Or SWP_NOACTIVATE)
         Else
            FLY_VFB_Layout_Handle(fp ,rcParent.Right ,rcParent.Bottom ,fp->nWidth ,fp->nHeight ,x ,y ,xWidth ,yHeight)
         End if
         fp->nLeft   = x
         fp->nTop    = y
         fp->nWidth  = xWidth
         fp->nHeight = yHeight
      End if
      Dim sfp As FormControlsPro_TYPE Ptr = fp->VrControls
      fp = sfp
   Wend
   EndDeferWindowPos nHDWP
End Sub

Sub FLY_VFB_Layout_Handle(fp As FormControlsPro_TYPE ptr, pWidth AS LONG, pHeight AS LONG, nWidth AS LONG, nHeight AS LONG, ByRef x AS LONG, ByRef y AS LONG, ByRef xWidth AS LONG, ByRef yHeight AS LONG)
  '处理控件布局 
   x = 0 : y = 0 : xWidth = 0 : yHeight = 0
   SELECT CASE fp->anchor
      CASE 0 'AFX_ANCHOR_NONE
         x = fp->nLeft
         y = fp->nTop
         xWidth = MAX(pWidth - fp->nLeft - fp->nRight, 0)
         yHeight = nHeight
      CASE 1 'AFX_ANCHOR_WIDTH
         x = fp->nLeft
         y = fp->nTop
         xWidth = MAX(pWidth - fp->nLeft - fp->nRight, 0)
         yHeight = nHeight
      CASE 2 'AFX_ANCHOR_RIGHT
         x = pWidth - nWidth - fp->nRight
         y = fp->nTop
         xWidth = nWidth
         yHeight = nHeight
      CASE 3 'AFX_ANCHOR_CENTER_HORZ
         x = (pWidth \ 2) + fp->centerX
         y = fp->nTop
         xWidth = nWidth
         yHeight = nHeight
      CASE 4 'AFX_ANCHOR_HEIGHT
         x = fp->nLeft
         y = fp->nTop
         xWidth = nWidth
         yHeight = MAX(pHeight - fp->nTop - fp->nBottom, 0)
      CASE 5 'AFX_ANCHOR_HEIGHT_WIDTH
         x = fp->nLeft
         y = fp->nTop
         xWidth = MAX(pWidth - fp->nLeft - fp->nRight, 0)
         yHeight = MAX(pHeight - fp->nTop - fp->nBottom, 0)
      CASE 6 'AFX_ANCHOR_HEIGHT_RIGHT
         x = pWidth - nWidth - fp->nRight
         y = fp->nTop
         xWidth = nWidth
         yHeight = MAX(pHeight - fp->nTop - fp->nBottom, 0)
      CASE 7 'AFX_ANCHOR_BOTTOM
         x = fp->nLeft
         y = pHeight - fp->nBottom - nHeight
         xWidth = nWidth
         yHeight = nHeight
      CASE 8 'AFX_ANCHOR_BOTTOM_WIDTH
         x = fp->nLeft
         y = pHeight - fp->nBottom - nHeight
         xWidth = MAX(pWidth - fp->nLeft - fp->nRight, 0)
         yHeight = nHeight
      CASE 9 'AFX_ANCHOR_BOTTOM_RIGHT
         x = pWidth - nWidth - fp->nRight
         y = pHeight - fp->nBottom - nHeight
         xWidth = nWidth
         yHeight = nHeight
      CASE 10 'AFX_ANCHOR_CENTER_HORZ_BOTTOM
         x = (pWidth \ 2) + fp->centerX
         y = pHeight - fp->nBottom - nHeight
         xWidth = nWidth
         yHeight = nHeight
      CASE 11 'AFX_ANCHOR_CENTER_VERT
         x = fp->nLeft
         y = (pHeight - nHeight) \ 2
         xWidth = nWidth
         yHeight = nHeight
      CASE 12 'AFX_ANCHOR_CENTER_VERT_RIGHT
         x = pWidth - nWidth - fp->nRight
         y = (pHeight - nHeight) \ 2
         xWidth = nWidth
         yHeight = nHeight
      CASE 13 'AFX_ANCHOR_CENTER
         x = (pWidth \ 2) + fp->centerX
         y = (pHeight \ 2) + fp->centerY
         xWidth = nWidth
         yHeight = nHeight
   END SELECT
End Sub





'[SourceDB:0-171292]
'发送内容到【调试输出窗口显示】
' #include Once "afx/CWindow.inc" 'WinFBX 库，是WIN系统增强基础库，使用窗口和控件必须要用到它。 '已经搬到定义文件处
Sub PrintA(p1 As CWSTR = wChr(0), p2 As CWSTR = wChr(0), p3 As CWSTR = wChr(0), p4 As CWSTR = wChr(0), p5 As CWSTR = wChr(0), p6 As CWSTR = wChr(0), p7 As CWSTR = wChr(0), p8 As CWSTR = wChr(0), p9 As CWSTR = wChr(0), p10 As CWSTR = wChr(0))
   Static vfbWnd As hWnd
   Dim ff As Long
   If IsWindow(vfbWnd) = 0 Then vfbWnd = FindWindowA("yfPrintForm", Null)
   If IsWindow(vfbWnd) = 0 Then
      vfbWnd = FindWindowA("VisualFreeBasic5x", Null)
      ff = 1
   End if
   If IsWindow(vfbWnd) Then
      Dim nn As CWSTR
      if *p1 Then
         nn = p1
         if *p2 Then
            nn &= " | " & p2
            If *p3 Then
               nn &= " | " & p3
               if *p4 Then
                  nn &= " | " & p4
                  if *p5 Then
                     nn &= " | " & p5
                     if *p6 Then
                        nn &= " | " & p6
                        If *p7 Then
                           nn &= " | " & p7
                           if *p8 Then
                              nn &= " | " & p8
                              if *p9 Then
                                 nn &= " | " & p9
                                 If *p10 Then
                                    nn &= " | " & p10
                                 End if
                              End if
                           End if
                        End if
                     End if
                  End if
               End if
            End if
         End if
      End if
      if Len(nn) = 0 Then nn = " "
      Dim Param as COPYDATASTRUCT  'dwData|cbData|lpData
      Param.cbData =Len(nn) *2+2 
      Param.lpData = nn.vptr 
      SendMessageW(vfbWnd, WM_COPYDATA, &H502, Cast(lParam, VarPtr(Param)))
      if ff Then vfbWnd = 0
   End if
End Sub

'[SourceDB:0-171273]
'WinErrorMsg
'WinErrorMsg(GetLastError) '显示API错误
'显示指定Windows错误代码的文本
'显示为中文
Function WinErrorMsg (ByVal dError as DWord)  as String

    Dim pBuffer   as ZString Ptr 
    Dim ncbBuffer as DWord

    ncbBuffer = FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER Or _
                               FORMAT_MESSAGE_FROM_SYSTEM Or _
                               FORMAT_MESSAGE_IGNORE_INSERTS, _
                               Null, _
                               dError, _
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), _
                               Cast(any ptr, @pBuffer), _
                               0, _
                               ByVal Null)

    If pBuffer>0 And ncbBuffer>0 Then
        Function = *pBuffer
        LocalFree pBuffer
    End If

End Function
'[SourceDB:0-197434]
Function RightW(wszStr As StringW, n As Integer ) As StringW  '返回宽字符串的最右边的子串
   Return Right(wszStr.WStr,n)
End Function
'[SourceDB:0-197430]
Function LenW(wszStr As StringW) As Integer  '返回宽字符长度（Unicode编码）一个中文算一个字符，一个英文算一个字符
   Return wszStr.WstrLen 
End Function
'[SourceDB:0-197424]
Function InStrRevW(wszStr As StringW, substring As StringW,start As Integer =-1) As Integer '查找字符串中子字符串或字符的最后一次出现
   Return InStrRev(wszStr.WStr,substring.WStr,start)
End Function 
'[SourceDB:0-171296]
'获取所有进程软件名和 ID
' GetAllPID(Sof() As String,PID() As Dword,Optional ByVal c As Long
' c =0  ,Sof() 带文件夹的全部文件名 其他，只是进程名
'数组从1开始，0表示没有，
'返回是数量
' #Include  Once "win\tlhelp32.bi" '已经搬到定义文件处
Function GetAllPID(Sof() as String, PID() as DWord, ByVal c as Integer = 0) as Integer
   Dim as Integer p,i,u
   Dim Proc as PROCESSENTRY32A, hSnap as HANDLE
   ReDim Sof(0), PID(0)
   hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, Null)
   If hSnap = 0 Then Exit Function
   Proc.dwSize = SizeOf(PROCESSENTRY32A)
   p = Process32FirstA(hSnap, @Proc)
   u = 100
   ReDim Sof(u), PID(u)
   While p
      'If Proc.th32ProcessID >10 Then
      I = I + 1
      If i > u Then
         u = u + 100
         ReDim Preserve Sof(u), PID(u)
      End If
      PID(I) = Proc.th32ProcessID
      If c = 0 Then
         Sof(I) = AfxGetPathFromProcessId(pId(I))
         If Len(Sof(I)) = 0 Then Sof(I) = Process_GetPath(pId(I))
         If Len(Sof(I)) = 0 Then Sof(I) = Proc.szExeFile
      Else
         Sof(I) = Proc.szExeFile
      End If
      
      'End If
      p = Process32NextA(hSnap, @Proc)
   Wend
   CloseHandle hSnap
   ReDim Preserve Sof(i), PID(i)
   Function = i
End Function
'[SourceDB:0-197425]
Function InStrW Overload(wszStr As StringW, substring As StringW) As Integer '查找宽字符串中第一个出现的子字符串或字符
   Return InStr(wszStr.WStr,substring.WStr)
End Function 
Function InStrW Overload(start As Integer, wszStr As StringW, substring As StringW) As Integer '查找宽字符串中第一个出现的子字符串或字符
   Return InStr(start, wszStr.WStr, substring.WStr)
End Function
'[SourceDB:0-171244]
'在对话框中显示消息(兼容VB6)，等待用户单击按钮，并返回一个 Integer 告诉用户单击哪一个按钮。（选项太多，建议用【工具】菜单里【对话框编辑器】自动产生代码）
'按钮 MB_OK MB_OKCANCEL MB_YESNOCANCEL MB_YESNO MB_RETRYCANCEL MB_ABORTRETRYIGNORE MB_CANCELTRYCONTINUE
'图标 MB_ICONERROR MB_ICONQUESTION MB_ICONWARNING MB_ICONINFORMATION
'默认按钮 MB_DEFBUTTON1 MB_DEFBUTTON2 MB_DEFBUTTON3 MB_DEFBUTTON4
'模态对话框 MB_APPLMODAL MB_SYSTEMMODAL MB_TASKMODAL  
'帮助 MB_HELP
'返回 IDOK IDCANCEL IDYES IDNO IDCANCEL IDRETRY IDCANCEL IDABORT IDRETRY IDIGNORE IDTRYAGAIN IDCONTINUE
Function MsgBox Overload(CH_E68F90E7A4BAE69687E69CAC_ As WString , CH_E98089E9A1B9_ As ULong = MB_OK, CH_E6A087E9A298E69687E69CAC_ As WString = App.ProductName , CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND = AfxGetHwndFromPID(GetCurrentProcessId)) As Integer
   'VB兼容模式
'   PrintA "VB兼容模式"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_, CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload(CH_E68F90E7A4BAE69687E69CAC_ As String , CH_E98089E9A1B9_ As ULong = MB_OK, CH_E6A087E9A298E69687E69CAC_ As String = App.ProductName , CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND = AfxGetHwndFromPID(GetCurrentProcessId)) As Integer
   'VB兼容模式
'   PrintA "VB兼容模式"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_, CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As WString, CH_E6A087E9A298E69687E69CAC_ As WString = App.ProductName , CH_E98089E9A1B9_ As ULong = MB_OK) As Integer
   'API 模式
'   PrintA "API 模式"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_, CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E6A087E9A298E69687E69CAC_ As String = App.ProductName , CH_E98089E9A1B9_ As ULong = MB_OK) As Integer
   'API 模式
'   PrintA "API 模式"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_, CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As WString, CH_E98089E9A1B9_ As ULong ) As Integer
   'API 模式 ，标题默认
   Dim CH_E6A087E9A298E69687E69CAC_ As WString * 200 = App.ProductName 
'   PrintA "API 模式 ，标题默认"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_,CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E98089E9A1B9_ As ULong ) As Integer
   'API 模式 ，标题默认
   Dim CH_E6A087E9A298E69687E69CAC_ As WString * 200 = App.ProductName 
'   PrintA "API 模式 ，标题默认"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_,CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function
Function MsgBox Overload( CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E6A087E9A298E69687E69CAC_ As String,ByVal CH_E98089E9A1B9_ As Integer=0,ByVal  CH_E788B6E7AA97E58FA3E58FA5E69F84_ As Hwnd = 0 ) As Integer
   'WIN9库 模式 ，标题默认
   '   PrintA "WIN9库 模式"
   Function = MessageBox(CH_E788B6E7AA97E58FA3E58FA5E69F84_, CH_E68F90E7A4BAE69687E69CAC_,CH_E6A087E9A298E69687E69CAC_, CH_E98089E9A1B9_)
End Function

'[SourceDB:0-171107]
'  FF_REPLACE
'  指定在字符串内，使用其他字符串替换一个字符串的所有匹配项。
'  Replaces all occurrences of MatchPattern in MainString with ReplaceWith.  
'  The replacement can cause MainString to grow or condense in size. 
'  This function is case-sensitive. 
'  When a match is found, the scan for the next match begins at the position 
'  immediately following the prior match.
'  Function FF_Replace( ByRef sMainString   as String, _
'                       ByRef sMatchPattern as String, _
'                       ByRef sReplaceWith  as String _
'                       ) as String
''
''  FF_REPLACE
''  Within a specified string, replace all occurrences of one string with another string.
''
''  Replaces all occurrences of MatchPattern in MainString with ReplaceWith.  
''  The replacement can cause MainString to grow or condense in size. 
''  This function is case-sensitive. 
''  When a match is found, the scan for the next match begins at the position 
''  immediately following the prior match.
''
Function FF_Replace( ByRef sMainString   as String, _
                     ByRef sMatchPattern as String, _
                     ByRef sReplaceWith  as String _
                     ) as String

    Dim i as Integer 
    Dim s as String 
   
    s = sMainString
    i = 1
    Do
        i = InStr(i, s, sMatchPattern)
        If i > 0 Then 
           s = Left(s, i - 1) & sReplaceWith & Mid(s, i + Len(sMatchPattern))
           i += Len(sReplaceWith)
        End If   
    Loop Until i = 0
   
    Return s

End Function


''
''  FF_REPLACEANY
''  Within a specified string, replace all occurrences of any of the individual characters
''  specified in the MatchPattern string.
''
''  This function is case-sensitive.      
''  sReplaceWidth must be a single character. This function does not replace words therefore
''  MainString will be the same size - it will not shrink or grow.
''
'Function FF_ReplaceAny( ByRef sMainString   as String, _
'                        ByRef sMatchPattern as String, _
'                        ByRef sReplaceWith  as String _
'                        ) as String
'    Dim nLenMain    as Integer = Len(sMainString)
'    Dim nLenMatch   as Integer = Len(sMatchPattern)
'    Dim nLenReplace as Integer = Len(sReplaceWith)
'    Dim s as String   
'    Dim y as Integer
'    Dim i as Integer 
'    ' Note: sReplaceWith must be one character in size because this function
'    '       replaces individual characters rather than words.
'    If nLenReplace <> 1 Then Return sMainString
'    If nLenMain  = 0 Then Return sMainString
'    If nLenMatch = 0 Then Return sMainString
'    s = sMainString   ' make a default return copy of the string
'    ' Get each character in the sMatchPattern string and look for it in sMainString
'    ' Using string pointer indexing for speed.
'    For y = 0 To nLenMatch - 1
'       For i = 0 To nLenMain - 1
'          If sMatchPattern[y] = sMainString[i] Then
'             s[i] = sReplaceWith[0]
'          End If
'       Next
'    Next
'    Function =  s
'End Function


'[SourceDB:0-171142]
'普通字符 换成 宽字符 
'返回包含宽字符格式的普通字符串，使用还是要转换的
Function StrToWStr(sStr As String, CodePage As UINT = CP_ACP) As String 'CodePage=语言代码页 简体中文=936 CP_ACP是默认系统语言，不同语言系统用CP_ACP转换会发生乱码
   Dim As UInteger ZStrLen, WStrLen
   Dim ss      As String, ff     As Long
   Dim ZStrPtr as LPCCH, WStrMem as LPWSTR
   ZStrLen = Len(sStr)
   If ZStrLen > 0 Then
      ZStrPtr = StrPtr(sStr)
      WStrLen = ZStrLen * 2
      ss      = String(WStrLen, 0)
      WStrMem = Cast(LPWSTR, StrPtr(ss))
      WStrLen = MultiByteToWideChar(CodePage, 0, ZStrPtr, ZStrLen, WStrMem, WStrLen)
      'WStrLen 是宽字符个数，不是字节数
      Function = Left(ss, WStrLen * 2)
   End If
End Function
'[SourceDB:0-171194]
'返回指定磁盘的卷标文本。
'  #include Once "Afx/CFileSys.inc" '已经搬到定义文件处
Function GetDiskLabel(nDrive As WString ) As StringW
   Dim pFileSys As CFileSys
   Return pFileSys.GetVolumeName(nDrive)
End Function
'[SourceDB:0-171131]
'返回一个下标从零开始的一维数组，它包含指定数目的子字符串。
'返回 数组(0) 是数量，实际内容从1开始
'返回值是数量
Function Split(TEXT as String, DELIMIT as String, RET() as String) as Integer
   '' 请注意，此函数将数组的内存分配给第一个指针，以便释放此指针可以释放完整的数组。
   Dim as Integer DMAX=0
   Dim RES() as ZString Ptr
   Dim as Integer I1 , I2, ini, fini
   Dim as ZString Ptr p , p1 , p2, p3
   Dim as Integer LDelimit = Len(DELIMIT), LT= Len(TEXT)
   Dim as Integer Posi()
   
   If LT = 0 Or LDelimit > LT Then
      ReDim RET(1)
      RET(0) = "0"
      DMAX = 0
      Return DMAX
      Exit Function
   EndIf
   '计数分隔符
   p = StrPtr(TEXT)
   p1 = p
   If LDelimit > 0 Then ' au moins 1 caract鑢e en d閘imiter
      Do While *p
         I2 = 0
         If *p = DELIMIT[0] Then
            p3 = p
            If LDelimit > 1 Then    'plus d'1 caract鑢e en d閘imiter
               For I1 = 1 To LDelimit -1
                  I2 = 0
                  p += 1
                  If *p <> DELIMIT[I1] Then Exit For
                  I2 = 1
               Next
            Else
               I2 = 1
            EndIf
            If I2 = 1 Then
               If p = StrPtr(TEXT) + LT -1 Then fini = 1  ' fini avec le d閘imiter
               
               DMAX += 1
               ReDim Preserve Posi(0 To DMAX -1)
               Posi(DMAX -1) = p3 - p1 + 1
            EndIf
         EndIf
         p += 1
      Loop
      
      If DMAX = 0 And ini = 0 Then  ' aucun d閘imiter trouv?
         DMAX = 1
         ReDim RET(2)
         RET(0) = "1"
         RET(1) = TEXT       ' copy the full text
         
         Return DMAX
         Exit Function
      ElseIf DMAX = 0 And ini = 1 Then  ' debut avec d閘imiter trouv?
         DMAX = 1
         ReDim RET(2)
         RET(0) = "1"
         RET(1) = Mid(TEXT, LDelimit + 1)
         
         Return DMAX
         Exit Function
      EndIf
      'dimention the array and assign memory to first element
      If fini = 0 Then DMAX += 1
      ReDim RET(0 To DMAX)
      ReDim RES(0 To DMAX -1)
      'msgbox  Str(DMAX)
      RES(0) = Allocate(Len(TEXT) + 1)
       *RES(0) = TEXT       ' copy the full text
      
      'step through the string, setting pointers for each element and null terminating
      p = RES(0)
      'msgbox  "DMAX = " & *RES(0) ,"p = "  & Str(p)
      For I1 = 0 To DMAX -2
         p2 = p + Posi(I1) -1
         'msgbox  "I1 = " & Str(I1) & "  Posi(I1)  " & Str(Posi(I1)) &  "  (ini * LDelimit) " & Str(ini * LDelimit)  ,"p2 = "  & Str(p2)
          *p2 = 0       ' null terminate each element
         RES(I1 + 1) = p2 + LDelimit       ' set pointer to next element
         RET(I1 + 1) = *RES(I1)
         'msgbox   RET(I1)
      Next
      If fini = 1 Then
         p2 = p + Posi(DMAX -1) -1
         'p2= p - LDelimit + 1
          *p2 = 0
      EndIf
      RET(DMAX) = *RES(DMAX -1)
      RET(0) = Str(DMAX)
      'msgbox   RET(DMAX-1)
      Deallocate RES(0)
   Else
      ReDim RET(2)
      RET(0) = "1"
      RET(1) = TEXT       ' copy the full texte
      DMAX = 1
      
   EndIf
   Return DMAX
End Function
'[SourceDB:0-197431]
Function MidW Overload(wszStr As StringW, start As Integer, n As Integer ) As StringW '返回宽字符串的子宽字符串
   Return Mid(wszStr.WStr,start,n)
End Function 
Function MidW Overload(wszStr As StringW, start As Integer) As StringW '返回宽字符串的子宽字符串
   Return Mid(wszStr.WStr,start)
End Function 
'[SourceDB:0-171234]
'doevents
'   挂起的控件或窗体的消息的过程。如果你正在执行一紧/下一步的或做循环和需要允许您控制/表单响应用户输入，请调用此函数。
Sub FF_DoEvents()

   Dim uMsg As Msg
   
   Do While PeekMessage(Varptr(uMsg), 0, 0, 0, PM_REMOVE) 
      If IsDialogMessage(GetActiveWindow, Varptr(uMsg)) = 0 Then
         TranslateMessage Varptr(uMsg)
         DispatchMessage Varptr(uMsg)
      End If   
   Loop

End Sub


'[SourceDB:0-171249]
'设置窗口Z位置
'SetWindowZ(窗口句柄,标识=HWND_TOP ) 
'HWND_BOTTOM     ：值为1，将窗口置于Z序的底部。如果参数hWnd标识了一个顶层窗口，则窗口失去顶级位置，并且被置在其他窗口的底部。
'HWND_NOTOPMOST  ：值为-2，将窗口置于所有非顶层窗口之上（即在所有顶层窗口之后）。如果窗口已经是非顶层窗口则该标志不起作用。
'HWND_TOP        ：值为0，将窗口置于Z序的顶部。
'HWND_TOPMOST    ：值为-1，将窗口置于所有非顶层窗口之上。即使窗口未被激活窗口也将保持顶级位置。
Sub SetWindowZ(ByVal hWndControl as HWnd,ByVal hWndlnsertAfter as HWND =HWND_TOP ) '设置窗口Z位置
    '检查窗口句柄是否有效
    If IsWindow(hWndControl) Then  
        SetWindowPos( hWndControl,hWndlnsertAfter  ,0,0,0,0, SWP_NOSIZE Or SWP_NOMOVE Or SWP_SHOWWINDOW)
    End If
End Sub
'[SourceDB:0-170843]
'在鼠标位置弹出菜单
Sub PopupMenu(hWndForm as HWnd, yMenu as HMENU) '在鼠标位置弹出菜单
  Dim  P as  Point
  GetCursorPos @p
  TrackPopupMenu yMenu, 0, p.x, p.y, 0, hWndForm, Null '比方说弹出菜单
End Sub
'[SourceDB:0-171312]
' 获取进程文件路径
'Function Process_GetPath(进程ID) as String
' #Include Once "Inc/NtDef.bi" '已经搬到定义文件处
' 获取进程文件路径
Function Process_GetPath(pid As Integer) As StringW
   Dim aa As Integer,rr As StringW 
   Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, pid)
   If hProcess Then
      Dim Is64 As Integer
      If GetSys_Bits = 64 Then IsWow64Process(hProcess, Cast(Any Ptr, @Is64)) Else Is64 = 1
      If Len(aa) = 8 Or Len(aa) = 4 And Is64 <> 0 Then '自己是64 或 自己是32位目标也是32位的
         Dim FilePath As Wstring * MAX_PATH
         GetModuleFileNameExW(hProcess, NULL, @FilePath, MAX_PATH)
         rr = FilePath
      Else
         Dim NtHdr As HANDLE = GetModuleHandle("ntdll")
         If NtHdr Then
            Dim NtWow64QueryInformationProcess64 as Function(ProcessHandle as HANDLE, ProcessInformationClass as UINT, ProcessInformation as PVOID, ProcessInformationLength as ULong, ReturnLength as PULONG) as Integer
            Dim NtWow64ReadVirtualMemory64 as Function(ProcessHandle as HANDLE, BaseAddress as ULongInt, Buffer as PVOID, NumberOfBytesToRead as UINT64, NumberOfBytesReaded as PUINT64) as Integer
            NtWow64QueryInformationProcess64 = Cast(Any Ptr, GetProcAddress(NtHdr, "NtWow64QueryInformationProcess64"))
            NtWow64ReadVirtualMemory64 = Cast(Any Ptr, GetProcAddress(NtHdr, "NtWow64ReadVirtualMemory64"))
            Dim pbi as PROCESS_BASIC_INFORMATION_64
            Dim peb as PROCESS_ENVIRONMENT_BLOCK_64
            Dim pps as PROCESS_PARAMETERS_64
            Dim sCmdLine as WString * MAX_PATH
            If NT_SUCCESS(NtWow64QueryInformationProcess64(hProcess, ProcessBasicInformation, @pbi, SizeOf(pbi), 0)) Then
               If NT_SUCCESS(NtWow64ReadVirtualMemory64(hProcess, pbi.PebBaseAddress, @peb, SizeOf(peb), 0)) Then
                  If NT_SUCCESS(NtWow64ReadVirtualMemory64(hProcess, peb.ProcessParameters, @pps, SizeOf(pps), Null)) Then
                     'sCmdLine=String(pps.ImagePathName.MaximumLength,0)
                     If NT_SUCCESS(NtWow64ReadVirtualMemory64(hProcess, pps.ImagePathName.Buffer, @sCmdLine, pps.ImagePathName.MaximumLength, Null)) Then
                        rr = sCmdLine '  wStrToStr(Cast(WString Ptr, StrPtr(sCmdLine)), pps.ImagePathName.MaximumLength/2)
                     EndIf
                  EndIf
               EndIf
            EndIf
         EndIf
         
      EndIf
      CloseHandle(hProcess)
   EndIf
   Return rr 
End Function
'[SourceDB:0-171315]
'远程DLL注入
'32位只用与32,64位只用与64位
' #Include Once "win/mmsystem.bi" '已经搬到定义文件处
Function RemoteDLLinjection(ByVal jID As DWord, nDLL As WString ) As String  '远程DLL注入
   Dim as Integer  lResult,i
   Dim As UInteger  Tt
   Dim lpThreadAttributes as SECURITY_ATTRIBUTES
   Dim hProcess as HANDLE, tmp_Addr as LPVOID, pfnStartAddr as FARPROC
   Dim DohThread as HANDLE
   hProcess = OpenProcess(&H1F0FFF, False, jID)
   'Print "hProcess=" & hProcess
   If hProcess <> 0 Then
      tmp_Addr = VirtualAllocEx(hProcess, Null, len(nDLL)*2+10, MEM_COMMIT Or MEM_RESERVE, &H4) '申请内存空间
'      Printa "tmp_Addr=" & hex(tmp_Addr)
      If tmp_Addr <> 0 Then
         If WriteProcessMemory(hProcess, tmp_Addr,@nDLL, Len(nDLL)*2, 0) > 0 Then '写入DLL文件（带文件夹路径）
            pfnStartAddr = GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryW") '获取加载DLL函数入口
            'Print "pfnStartAddr=" & pfnStartAddr
            If pfnStartAddr <> 0 Then
               'For i=1 To 20
               DohThread = CreateRemoteThread(hProcess, @lpThreadAttributes, Null, Cast(LPTHREAD_START_ROUTINE, pfnStartAddr), tmp_Addr, 0, 0) '运行远程线程
               'If DohThread<>0 Then Exit For
               'DoEventsEX 100
               'Next
               'Print "DohThread=" & DohThread
               If DohThread <> 0 Then
                  'tt=timeGetTime
                  'DoEventsEX 1000
                  WaitForSingleObject DohThread, 5000
                  
                  CloseHandle DohThread '关闭线程句柄
                  
               Else
                  Function = "运行远程线程时失败，无法远程注入DLL"
               End If
            Else
               Function = "获取加载DLL函数入口失败，无法远程注入DLL"
            End If
         Else
            Function = "写入远程内存失败，无法远程注入DLL"
         End If
         VirtualFreeEx(hProcess, tmp_Addr, Null, &H8000&)  '清理内存
      Else
         Function = "申请内存空间入口失败，无法远程注入DLL"
      End If
      CloseHandle hProcess '关闭
   End If
   
End Function
'[SourceDB:0-171072]
'画线 DrawLine(ByVal hDc As Long , ByVal X1 As Long ,ByVal Y1 As Long ,ByVal X2 As Long ,ByVal Y2 As Long ,_
'             ByVal nPenStyle As Long, ByVal nWidth As Long, ByVal crColor As Long)
'需要支持高DPI，请代码中加 AfxScaleX(X), AfxScaleY(Y)  
'nPenStyle ------ Long，指定画笔样式，可以是下述常数之一
'PS_SOLID        画笔画出的是实线
'PS_DASH         画笔画出的是虚线（nWidth必须不大于1）
'PS_DOT          画笔画出的是点线（nWidth必须不大于1）
'PS_DASHDOT      画笔画出的是点划线（nWidth必须不大于1）
'PS_DASHDOTDOT   画笔画出的是点-点-划线（nWidth必须不大于1）
'PS_NULL         画笔不能画图
'PS_INSIDEFRAME  由椭圆、矩形、圆角矩形、饼图以及弦等生成的封闭对象框时，画线宽度向内扩展。如指定的准确RGB颜色不存在，就进行抖动处理
'颜色用 BGR(0, 0, 255) 创建  不能用 RGB
Sub DrawLine(ByVal hDC as hDC, ByVal X1 as Integer, ByVal Y1 as Integer, ByVal X2 as Integer, ByVal Y2 as Integer, _
      ByVal nPenStyle as Integer, ByVal nWidth as Integer, ByVal crColor as Long) '画线
   '线条类型 nPenStyle
   '实线 = 0
   '虚线 = 1
   '点线 = 2
   '点划线 = 3
   '双点划线 = 4
   '无线 = 5
   '内收实线 = 6
   Dim hOldPen as HGDIOBJ, hPen as HGDIOBJ, lpPoint as Point
   hPen = CreatePen(nPenStyle, nWidth, crColor) '创建新画笔
   hOldPen = SelectObject(hDC, hPen)            '画笔存入DC
   MoveToEx hDC, X1, Y1, @lpPoint             '设置起点
   LineTo hDC, X2, Y2                      '画到终点
   SelectObject hDC, hOldPen            '销毁
   DeleteObject hPen
   
End Sub
'[SourceDB:0-171071]
'画框 DrawFrame(DC,X1,Y1,X2,Y2,填充色彩,线条类型,线条色彩,线条宽)
'需要支持高DPI，请代码中加 AfxScaleX(X), AfxScaleY(Y)  
'nPenStyle ------ Long，指定画笔样式，可以是下述常数之一
'PS_SOLID        画笔画出的是实线
'PS_DASH         画笔画出的是虚线（nWidth必须不大于1）
'PS_DOT          画笔画出的是点线（nWidth必须不大于1）
'PS_DASHDOT      画笔画出的是点划线（nWidth必须不大于1）
'PS_DASHDOTDOT   画笔画出的是点-点-划线（nWidth必须不大于1）
'PS_NULL         画笔不能画图
'PS_INSIDEFRAME  由椭圆、矩形、圆角矩形、饼图以及弦等生成的封闭对象框时，画线宽度向内扩展。如指定的准确RGB颜色不存在，就进行抖动处理
'颜色用 BGR(0, 0, 255) 创建  不能用 RGB
Sub DrawFrame(DC as hDC, X1 as Long, Y1 as Long, X2 as Long, Y2 as Long, C as Long, L as Long = PS_NULL, cL as Long = 0, wL as Long = 1)
   '画框                                                                   填充色彩  线条类型     线条色彩
   
   Dim hBrush as HBRUSH, hPen as HPEN
   Dim hOldBrush as HGDIOBJ, hOldPen as HGDIOBJ
   
   If c <> -1 Then
      hBrush = CreateSolidBrush(c)
   Else
      hBrush = GetStockObject(HOLLOW_BRUSH) '创建空画笔，才能画出空心来
   End If
   hOldBrush = SelectObject(DC, hBrush)
   
   hPen = CreatePen(L, wl, cl)
   
   hOldPen = SelectObject(DC, hPen)
   
   Rectangle DC, X1, Y1, X2, Y2
   
   SelectObject DC, hOldPen
   
   DeleteObject hPen
   
   SelectObject DC, hOldBrush
   DeleteObject hBrush
   
   
End Sub
'[SourceDB:0-171069]
'圆角矩形 
' DrawCircleFrame(DC,X,Y,W,H,x圆角,y圆角,框线粗细,线条类型 ,边框色,填充色)
'需要支持高DPI，请代码中加 AfxScaleX(X), AfxScaleY(Y)  
'nPenStyle ------ Long，指定画笔样式，可以是下述常数之一
'PS_SOLID        画笔画出的是实线
'PS_DASH         画笔画出的是虚线（nWidth必须不大于1）
'PS_DOT          画笔画出的是点线（nWidth必须不大于1）
'PS_DASHDOT      画笔画出的是点划线（nWidth必须不大于1）
'PS_DASHDOTDOT   画笔画出的是点-点-划线（nWidth必须不大于1）
'PS_NULL         画笔不能画图
'PS_INSIDEFRAME  由椭圆、矩形、圆角矩形、饼图以及弦等生成的封闭对象框时，画线宽度向内扩展。如指定的准确RGB颜色不存在，就进行抖动处理
'颜色用 BGR(0, 0, 255) 创建  不能用 RGB
Sub DrawCircleFrame(ByVal sDC as hDC, ByVal X as Long, ByVal Y as Long, ByVal hWidth as Long, ByVal hHeight as Long, ByVal xCircle as Long, ByVal yCircle as Long, _
      ByVal LineWidth as Long, ByVal LineType as Long , ByVal bColor as Long, ByVal tColor as Long)'圆角矩形
   
   '(ByVal sDC As Long, ByVal X As Long, ByVal Y As Long, ByVal 宽度 As Long, ByVal 高度 As Long, X圆角 As Long, Y圆角 As Long, _
   'ByVal 框线粗细 As Long, ByVal 线条类型 As 线条类型_m, ByVal 边框色 As Long, ByVal 填充色 As Long) '圆角矩形
   '以支持高DPI，以DPI100%设置坐标就可以了
   
   Dim hBrush as HBRUSH, hPen as HPEN
   Dim hOldBrush as HGDIOBJ, hOldPen as HGDIOBJ
   
   If tColor <> -1 Then
      hBrush = CreateSolidBrush(tColor)
   Else
      hBrush = GetStockObject(HOLLOW_BRUSH) '创建空画笔，才能画出空心来
   End If
   hOldBrush = SelectObject(sDC, hBrush)
   
   hPen = CreatePen(LineType, LineWidth, bColor)
   
   hOldPen = SelectObject(sDC, hPen)
   
   RoundRect sDC, X, Y, X + hWidth, Y + hHeight, xCircle, yCircle
   
   SelectObject sDC, hOldPen
   
   DeleteObject hPen
   
   SelectObject sDC, hOldBrush
   DeleteObject hBrush
   
   '参数表
   '参数 类型及说明
   'sDC Long，用于绘图的设备场景
   'X1,Y1 Long，对矩形左上角位置进行说明的X，Y坐标
   'X2,Y2 Long，对矩形右下角位置进行说明的X，Y坐标
   'X3 Long，用于生成圆角效果的一个椭圆的宽度。取值范围从零（表示不加圆角），一直到矩形的宽度（全圆）
   'Y3 Long，用于生成圆角效果的一个椭圆的高度。取值范围从零（表示不加圆角），一直到矩形的高度（全圆）
   
   
End Sub
'[SourceDB:0-171070]
'描绘一个椭圆 
'DrawEllipse(Dc,X,Y,宽度 ,高度,线条类型,框线粗细,边框色,填充色)
'需要支持高DPI，请代码中加 AfxScaleX(X), AfxScaleY(Y)  
'nPenStyle ------ Long，指定画笔样式，可以是下述常数之一
'PS_SOLID        画笔画出的是实线
'PS_DASH         画笔画出的是虚线（nWidth必须不大于1）
'PS_DOT          画笔画出的是点线（nWidth必须不大于1）
'PS_DASHDOT      画笔画出的是点划线（nWidth必须不大于1）
'PS_DASHDOTDOT   画笔画出的是点-点-划线（nWidth必须不大于1）
'PS_NULL         画笔不能画图
'PS_INSIDEFRAME  由椭圆、矩形、圆角矩形、饼图以及弦等生成的封闭对象框时，画线宽度向内扩展。如指定的准确RGB颜色不存在，就进行抖动处理
'颜色用 BGR(0, 0, 255) 创建  不能用 RGB
Sub DrawEllipse(ByVal hDC as hDC, ByVal X as Integer, ByVal Y as Integer, ByVal hWidth as Integer, ByVal hHeight as Integer, _
      ByVal LineType as Integer ,ByVal LineWidth as Integer, ByVal bColor as  Integer , ByVal tColor as  Integer )'描绘一个椭圆
   
   '(ByVal hDc As Long, ByVal X As Long, ByVal Y As Long, ByVal 宽度 As Long, ByVal 高度 As Long, _
   'ByVal 框线粗细 As Long, ByVal 线条类型 As 线条类型_m, ByVal 边框色 As Long, ByVal 填充色 As Long) '圆角矩形
   
   
   Dim hBrush as HGDIOBJ, hPen as HGDIOBJ
   Dim hOldBrush as HGDIOBJ, hOldPen as HGDIOBJ
   
   If tColor <> -1 Then
      hBrush = CreateSolidBrush(tColor)
   Else
      hBrush = GetStockObject(HOLLOW_BRUSH)
   End If
   hOldBrush = SelectObject(hDC, hBrush)
   
   
   'SetBkMode hdc,TRANSPARENT
   '设置这个后，画上的字是透明的
   
   hPen = CreatePen(LineType, LineWidth, bColor)
   
   hOldPen = SelectObject(hDC, hPen)
   
   Ellipse hDC, X, Y, X + hWidth, Y + hHeight
   
   SelectObject hDC, hOldPen
   
   DeleteObject hPen
   
   SelectObject hDC, hOldBrush
   DeleteObject hBrush
   
   '参数表
   
   'X1，Y1 Long，约束矩形采用逻辑坐标的左上角位置
   'X2，Y2 Long，约束矩形采用逻辑坐标的右下角位置
   
   
End Sub

'[SourceDB:0-171078]
'获取文本显示宽度
Function GetTextWidth(ByVal DC as hDC, nText as String) as Long '获取文本显示宽度
   Dim s as SIZE
   GetTextExtentPoint32A DC, StrPtr(nText), Len(nText), @s
   Function = s.cx
End Function
'[SourceDB:0-171076]
'获取文本显示高度
Function GetTextHeight(ByVal DC as hDC, nText as String) as Long '获取文本显示高度
   Dim s as SIZE
   GetTextExtentPoint32A DC, StrPtr(nText), Len(nText), @s
   Function = s.cy
End Function
'[SourceDB:0-171237]
'刷新窗口
Sub FF_Redraw(HWND_F as HWnd )
   if IsWindow(HWND_F) Then
   InvalidateRect (HWND_F , Null, True)
   UpdateWindow (HWND_F)
   end if 
End Sub
'[SourceDB:0-171264]
'获取系统位数
'Function GetSys_Bits() as Integer '获取系统位数
Function GetSys_Bits() as Integer '获取系统位数
   Dim Global_Sys_Info as SYSTEM_INFO
   GetNativeSystemInfo(@Global_Sys_Info)
   Select Case Global_Sys_Info.wProcessorArchitecture
      Case PROCESSOR_ARCHITECTURE_AMD64, PROCESSOR_ARCHITECTURE_IA64
         Return 64
      Case Else
         Return 32
   End Select
End Function