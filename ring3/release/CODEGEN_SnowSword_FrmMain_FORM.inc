'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|1000]
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分49秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------

'------------------------------------------------------------------------------
'VisualFreeBasic 自动生成的 FrmMain 函数
'------------------------------------------------------------------------------
'{FORM_WS_EX_NOACTIVATE3}  '为无焦点窗口创建定义
Function FrmMain_Class_Form.Show(hWndParent As .hWnd = HWND_DESKTOP, Modal As Boolean = False, UserData As Integer = 0) As .hWnd  '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
   '注：hWnd 是类属性，与API的 hWnd 冲突，因此使用 .hWnd 表示API，就是前面加个小数点。
   If This.Repeat = False Then     '是不是不允许多开窗口
      If IsWindow(This.hWnd) Then 
         This.FlashWindow
         Return This.hWnd
      End if 
   End if 
'{FORM_WS_EX_NOACTIVATE}  '为无焦点窗口创建多线程代码
   Dim pWindow AS CWindow Ptr = New CWindow(WStr("SnowSword_FrmMain"))
   '创建一个窗口
  Dim NewhWnd As .hWnd = pWindow->Create(hWndParent, WStr("SnowSword"), Cast(WNDPROC, @FrmMain_FORMPROCEDURE), 0, 0, 750, 500, WS_THICKFRAME Or WS_CAPTION Or WS_SYSMENU Or WS_MINIMIZEBOX Or WS_MAXIMIZEBOX Or WS_CLIPSIBLINGS Or WS_CLIPCHILDREN Or WS_POPUP Or WS_SIZEBOX, WS_EX_WINDOWEDGE Or WS_EX_LEFT Or WS_EX_LTRREADING Or WS_EX_RIGHTSCROLLBAR Or WS_EX_CONTROLPARENT)
'{FORM_WS_EX_NOACTIVATE2}  '为无焦点窗口返回窗口句柄
   hWnd = NewhWnd
   If IsWindow(NewhWnd) = 0 Then
      Delete pWindow
      Return 0
   End if
   Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
   vfb_Set_Control_Ptr(NewhWnd,fp)
   DIm rcParent AS RECT  '为控件布局属性，初始化数据用
   .GetClientRect(NewhWnd, @rcParent)
   rcParent.Right = AfxUnscaleX(rcParent.Right)  ' 为自动响应DPI，全部调整为 100%DPI 时的数值
   rcParent.Bottom = AfxUnscaleX(rcParent.Bottom) 
   fp->hWndParent = hWndParent
   fp->nName = "FrmMain"
   
   '给窗口设置其它参数
   pWindow->ClassStyle = CS_VREDRAW Or CS_HREDRAW Or CS_DBLCLKS
   '图标
   fp->BigIcon = LoadImage(app.hInstance, "AAAAA_APPICON", IMAGE_ICON, 48, 48, LR_SHARED)
   fp->SmallIcon = LoadImage(app.hInstance, "AAAAA_APPICON", IMAGE_ICON, 32, 32, LR_SHARED)
   pWindow->BigIcon = fp->BigIcon     '需要在销毁窗口时销毁句柄
   pWindow->SmallIcon = fp->SmallIcon
   '菜单
   '启动位置
   pWindow->Center
   hWnd = NewhWnd '在特殊情况下，多开窗口后发生 hWnd 的值被前一个窗口替换的情况  
   '鼠标穿透 透明度 透明颜色
   
   
   This.TransColor = GetCodeColorGDI(&H197F7F7F)
   fp->TransColor = &H197F7F7F
   This.MousePointer = 0
   This.BackColor = GetCodeColorGDI(&H0F7F7F7F)
   fp->BackColor = &H0F7F7F7F
   
   
         
'[Create main window]  创建主窗口，在此前一行插入。
    Dim hWndControl As .hWnd '控件句柄
    
   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.TopMenu1.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 127
      fp->nText = WStr("")
      This.TopMenu1.IDC =127
      fp->nName = "TopMenu1"
      This.TopMenu1.hMenu = CreateMenu 
      This.TopMenu1.AddMenu(0,"小工具",FrmMain_TopMenu1_mnuTools,"",-1,0,0,"")
      This.TopMenu1.AddMenu(FrmMain_TopMenu1_mnuTools,"查看被占用文件",FrmMain_TopMenu1_mnuUnlockFile,"",-1,0,0,"")
      SetMenu hWnd, This.TopMenu1.hMenu 
   End IF
   .GetClientRect(hWnd, @rcParent) '加窗口菜单后引起客户区大小改变，必须重新获取
   rcParent.Right = AfxUnscaleX(rcParent.Right) 
   rcParent.Bottom = AfxUnscaleX(rcParent.Bottom)

'[Create control top]  顶部创建控件的代码，（主要为了顶部菜单，必须在所有控件之前创建）在此前一行插入。
        
   hWndControl = pWindow->AddControl("TEXTBOX", hWnd, 102, WStr(""), 200, 427, 540, 30,WS_CHILD Or WS_CLIPSIBLINGS Or ES_LEFT Or WS_VISIBLE Or ES_AUTOHSCROLL Or WS_TABSTOP Or ES_WANTRETURN ,WS_EX_CLIENTEDGE , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 102
      fp->nText = WStr("")
      This.TxtFilter2.hWnd = hWndControl 
      This.TxtFilter2.IDC =102
      fp->nName = "TxtFilter2"
      This.TxtFilter2.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.TxtFilter2.BackColor =GetCodeColorGDI(&H057F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H057F7F7F
      This.TxtFilter2.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 200
      fp->nTop = 427
      fp->nWidth = 540
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 103, WStr("自我保护(应用层)"), 500, 50, 100, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 103
      fp->nText = WStr("自我保护(应用层)")
      This.Check9.hWnd = hWndControl 
      This.Check9.IDC =103
      fp->nName = "Check9"
      This.Check9.Caption = WStr("自我保护(应用层)")
      This.Check9.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check9.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check9.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 500
      fp->nTop = 50
      fp->nWidth = 100
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 104, WStr("自我保护(内核层)"), 220, 160, 100, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 104
      fp->nText = WStr("自我保护(内核层)")
      This.Check8.hWnd = hWndControl 
      This.Check8.IDC =104
      fp->nName = "Check8"
      This.Check8.Caption = WStr("自我保护(内核层)")
      This.Check8.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check8.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check8.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 220
      fp->nTop = 160
      fp->nWidth = 100
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 105, WStr("禁止加载驱动"), 560, 120, 100, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 105
      fp->nText = WStr("禁止加载驱动")
      This.Check7.hWnd = hWndControl 
      This.Check7.IDC =105
      fp->nName = "Check7"
      This.Check7.Caption = WStr("禁止加载驱动")
      This.Check7.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check7.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check7.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 560
      fp->nTop = 120
      fp->nWidth = 100
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 106, WStr("加载驱动"), 400, 50, 90, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 106
      fp->nText = WStr("加载驱动")
      This.Check6.hWnd = hWndControl 
      This.Check6.IDC =106
      fp->nName = "Check6"
      This.Check6.Caption = WStr("加载驱动")
      This.Check6.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check6.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check6.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 400
      fp->nTop = 50
      fp->nWidth = 90
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 107, WStr("禁止操作注册表"), 440, 120, 110, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 107
      fp->nText = WStr("禁止操作注册表")
      This.Check5.hWnd = hWndControl 
      This.Check5.IDC =107
      fp->nName = "Check5"
      This.Check5.Caption = WStr("禁止操作注册表")
      This.Check5.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check5.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check5.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 440
      fp->nTop = 120
      fp->nWidth = 110
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 108, WStr("禁止线程创建"), 330, 120, 100, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 108
      fp->nText = WStr("禁止线程创建")
      This.Check4.hWnd = hWndControl 
      This.Check4.IDC =108
      fp->nName = "Check4"
      This.Check4.Caption = WStr("禁止线程创建")
      This.Check4.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check4.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check4.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 330
      fp->nTop = 120
      fp->nWidth = 100
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 109, WStr("禁止进程创建"), 220, 120, 100, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 109
      fp->nText = WStr("禁止进程创建")
      This.Check3.hWnd = hWndControl 
      This.Check3.IDC =109
      fp->nName = "Check3"
      This.Check3.Caption = WStr("禁止进程创建")
      This.Check3.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check3.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check3.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 220
      fp->nTop = 120
      fp->nWidth = 100
      fp->nHeight = 30
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.Frame3.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 110
      fp->nText = WStr("Ring0设置")
      This.Frame3.IDC =110
      fp->nName = "Frame3"
      This.Frame3.Caption = WStr("Ring0设置")
      fp->Style = (fp->Style And &H00FFFFFF) Or (Cast(UInteger,0) Shl 24)
      fp->Style = (fp->Style And &HFF00FFFF) Or (Cast(UInteger,5) Shl 16)
      fp->Style = (fp->Style And &HFFFF0FFF) Or (Cast(UInteger,1) Shl 12)
      fp->CtlData(0) = &H107F7F7F
      This.Frame3.ForeColorGDIplue =GetCodeColorGDIplue(&H087F7F7F,&HFF000000)
      fp->ForeColor =&H087F7F7F
      This.Frame3.BackColorGDIplue =GetCodeColorGDIplue(&H197F7F7F,&HFFE0E0E0)
      fp->BackColor =&H197F7F7F
      fp->Style Or= &H1 
      fp->Style Or= &H2 
      This.Frame3.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 210
      fp->nTop = 100
      fp->nWidth = 520
      fp->nHeight = 110
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 111, WStr("屏蔽消息钩子"), 300, 50, 90, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 111
      fp->nText = WStr("屏蔽消息钩子")
      This.Check2.hWnd = hWndControl 
      This.Check2.IDC =111
      fp->nName = "Check2"
      This.Check2.Caption = WStr("屏蔽消息钩子")
      This.Check2.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check2.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check2.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 300
      fp->nTop = 50
      fp->nWidth = 90
      fp->nHeight = 30
   End IF

   hWndControl = pWindow->AddControl("CHECKBOX", hWnd, 112, WStr("禁止注销"), 220, 50, 70, 30,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or BS_AUTOCHECKBOX Or BS_TEXT Or BS_NOTIFY Or BS_LEFT Or BS_VCENTER Or BS_MULTILINE Or WS_VISIBLE Or WS_TABSTOP ,0 , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 112
      fp->nText = WStr("禁止注销")
      This.Check1.hWnd = hWndControl 
      This.Check1.IDC =112
      fp->nName = "Check1"
      This.Check1.Caption = WStr("禁止注销")
      This.Check1.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.Check1.BackColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H197F7F7F
      This.Check1.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 220
      fp->nTop = 50
      fp->nWidth = 70
      fp->nHeight = 30
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.Frame2.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 113
      fp->nText = WStr("Ring3设置")
      This.Frame2.IDC =113
      fp->nName = "Frame2"
      This.Frame2.Caption = WStr("Ring3设置")
      fp->Style = (fp->Style And &H00FFFFFF) Or (Cast(UInteger,0) Shl 24)
      fp->Style = (fp->Style And &HFF00FFFF) Or (Cast(UInteger,5) Shl 16)
      fp->Style = (fp->Style And &HFFFF0FFF) Or (Cast(UInteger,1) Shl 12)
      fp->CtlData(0) = &H107F7F7F
      This.Frame2.ForeColorGDIplue =GetCodeColorGDIplue(&H087F7F7F,&HFF000000)
      fp->ForeColor =&H087F7F7F
      This.Frame2.BackColorGDIplue =GetCodeColorGDIplue(&H197F7F7F,&HFFE0E0E0)
      fp->BackColor =&H197F7F7F
      fp->Style Or= &H1 
      fp->Style Or= &H2 
      This.Frame2.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 210
      fp->nTop = 30
      fp->nWidth = 520
      fp->nHeight = 60
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.Line1.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 114
      fp->nText = WStr("")
      This.Line1.IDC =114
      fp->nName = "Line1"
      fp->Style = (fp->Style And &H0FFFFFFF) Or (Cast(UInteger,0) Shl 28)
      fp->Style = (fp->Style And &HF0FFFFFF) Or (Cast(UInteger,2) Shl 24)
      fp->Style = (fp->Style And &HFF0FFFFF) Or (Cast(UInteger,0) Shl 20)
      fp->Style = (fp->Style And &HFFF0FFFF) Or (Cast(UInteger,0) Shl 16)
      fp->Style = (fp->Style And &HFFFF0FFF) Or (Cast(UInteger,0) Shl 12)
      fp->Style = (fp->Style And &HFFFFF0FF) Or (Cast(UInteger,0) Shl 8)
      fp->ForeColor = &H017F7F7F
      fp->Style Or= &H1 
      fp->Style Or= &H2 
      fp->nLeft = 260
      fp->nTop = 20
      fp->nWidth = 470
      fp->nHeight = 20
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.Label1.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 115
      fp->nText = WStr("系统相关")
      This.Label1.IDC =115
      fp->nName = "Label1"
      fp->Style = (fp->Style And &H00FFFFFF) Or (Cast(UInteger,0) Shl 24)
      This.Label1.Caption = WStr("系统相关")
      fp->Style Or= &H1 
      fp->Style Or= &H2 
      This.Label1.ForeColorGDIplue =GetCodeColorGDIplue(&H087F7F7F,&HFF000000)
      fp->ForeColor =&H087F7F7F
      This.Label1.BackColorGDIplue =GetCodeColorGDIplue(&H197F7F7F,&HFFE0E0E0)
      fp->BackColor =&H197F7F7F
      This.Label1.Font = "Microsoft YaHei,9,0"
      This.Label1.FontWidth =0
      This.Label1.FontAngle =0
      fp->Style = (fp->Style And &HFF00FFFF) Or (Cast(UInteger,0) Shl 16)
      fp->Style Or= &H4 
      fp->Style And= Not &H8 
      fp->nLeft = 210
      fp->nTop = 10
      fp->nWidth = 50
      fp->nHeight = 20
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.FmeAdvance.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 116
      fp->nText = WStr("")
      This.FmeAdvance.IDC =116
      fp->nName = "FmeAdvance"
      fp->Style = (fp->Style And &H00FFFFFF) Or (Cast(UInteger,0) Shl 24)
      fp->Style = (fp->Style And &HFF00FFFF) Or (Cast(UInteger,1) Shl 16)
      fp->Style = (fp->Style And &HFFFF0FFF) Or (Cast(UInteger,1) Shl 12)
      fp->CtlData(0) = &H107F7F7F
      This.FmeAdvance.ForeColorGDIplue =GetCodeColorGDIplue(&H087F7F7F,&HFF000000)
      fp->ForeColor =&H087F7F7F
      This.FmeAdvance.BackColorGDIplue =GetCodeColorGDIplue(&H197F7F7F,&HFFE0E0E0)
      fp->BackColor =&H197F7F7F
      fp->Style Or= &H1 
      fp->Style Or= &H2 
      This.FmeAdvance.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 200
      fp->nTop = 0
      fp->nWidth = 540
      fp->nHeight = 460
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.mnuFile.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 123
      fp->nText = WStr("")
      This.mnuFile.IDC =123
      fp->nName = "mnuFile"
      This.mnuFile.hMenu = CreatePopupMenu 
      This.mnuFile.AddMenu(0,"新建",FrmMain_mnuFile_mnuCreate,"",-1,0,0,"")
      This.mnuFile.AddMenu(FrmMain_mnuFile_mnuCreate,"新建文件",FrmMain_mnuFile_mnuCreateFile,"",-1,0,0,"")
      This.mnuFile.AddMenu(FrmMain_mnuFile_mnuCreate,"新建文件夹",FrmMain_mnuFile_mnuCreateFolder,"",-1,0,0,"")
      This.mnuFile.AddMenu(0,"复制",FrmMain_mnuFile_mnuCopy,"",-1,0,0,"")
      This.mnuFile.AddMenu(0,"粘贴",FrmMain_mnuFile_mnuPaste,"",-1,0,0,"")
      This.mnuFile.AddMenu(0,"删除",FrmMain_mnuFile_mnuDelete,"",-1,0,0,"")
      fp->nLeft = 670
      fp->nTop = 40
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.TmrProtect.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 124
      fp->nText = WStr("")
      This.TmrProtect.IDC =124
      fp->nName = "TmrProtect"
      This.TmrProtect.Interval = 1
      fp->Style And= Not &H1 
      This.TmrProtect.Enabled = False
      fp->nLeft = 620
      fp->nTop = 40
   End IF

   hWndControl = pWindow->AddControl("TREEVIEW", hWnd, 128, WStr(""), -1, -2, 200, 463,WS_CHILD Or WS_CLIPSIBLINGS Or TVS_HASBUTTONS Or TVS_HASLINES Or TVS_LINESATROOT Or WS_VISIBLE Or WS_TABSTOP ,WS_EX_CLIENTEDGE , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 128
      fp->nText = WStr("")
      This.treMain.hWnd = hWndControl 
      This.treMain.IDC =128
      fp->nName = "treMain"
      SetWindowTheme(hWndControl,"Explorer", NULL)
      This.treMain.LineColor =GetCodeColorGDI(&H197F7F7F,CLR_DEFAULT)
      fp->CtlData(0) =&H197F7F7F
      This.treMain.ForeColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H197F7F7F
      This.treMain.BackColor =GetCodeColorGDI(&H057F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H057F7F7F
      This.treMain.Font = "Microsoft YaHei,9,0"
      fp->nLeft = -1
      fp->nTop = -2
      fp->nWidth = 200
      fp->nHeight = 463
   End IF

   fp->VrControls = new FormControlsPro_TYPE '创建虚拟控件链表
   fp = fp->VrControls
   If fp Then 
      This.mnuProcess.hWndForm = hWnd 
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 141
      fp->nText = WStr("")
      This.mnuProcess.IDC =141
      fp->nName = "mnuProcess"
      This.mnuProcess.hMenu = CreatePopupMenu 
      This.mnuProcess.AddMenu(0,"结束进程",FrmMain_mnuProcess_mnuTerminateProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"强制结束进程",FrmMain_mnuProcess_mnuForceTerminateProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"暴力结束进程",FrmMain_mnuProcess_mnuViolentTerminateProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"挂起进程",FrmMain_mnuProcess_mnuSuspendProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"恢复进程",FrmMain_mnuProcess_mnuResumeProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"保护进程",FrmMain_mnuProcess_mnuProtectProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"检测隐藏进程",FrmMain_mnuProcess_mnuCheckHideProcess,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"刷新",FrmMain_mnuProcess_mnuRefresh,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"-",FrmMain_mnuProcess_mnuStep1,"",0,0,0,"")
      This.mnuProcess.AddMenu(0,"查看模块",FrmMain_mnuProcess_mnuViewModule,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"查看线程",FrmMain_mnuProcess_mnuViewThread,"",-1,0,0,"")
      This.mnuProcess.AddMenu(0,"查看内存",FrmMain_mnuProcess_mnuViewMemory,"",-1,0,0,"")
      fp->nLeft = 660
      fp->nTop = 0
   End IF

   hWndControl = pWindow->AddControl("LISTVIEW", hWnd, 142, WStr(""), 200, 0, 540, 430,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or LVS_SHAREIMAGELISTS Or LVS_REPORT Or LVS_SHOWSELALWAYS Or WS_VISIBLE Or WS_TABSTOP ,WS_EX_CLIENTEDGE , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 142
      fp->nText = WStr("")
      This.ListView1.hWnd = hWndControl 
      This.ListView1.IDC =142
      fp->nName = "ListView1"
      SNDMSG(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT)
      SNDMSG(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES)
      This.ListView1.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.ListView1.BackColor =GetCodeColorGDI(&H057F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H057F7F7F
      This.ListView1.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 200
      fp->nTop = 0
      fp->nWidth = 540
      fp->nHeight = 430
   End IF

   hWndControl = pWindow->AddControl("TREEVIEW", hWnd, 143, WStr(""), 200, 0, 270, 430,WS_CHILD Or WS_CLIPSIBLINGS Or TVS_HASBUTTONS Or TVS_HASLINES Or TVS_LINESATROOT Or TVS_EDITLABELS Or TVS_SHOWSELALWAYS Or WS_VISIBLE Or WS_TABSTOP ,WS_EX_CLIENTEDGE , , Cast(Any Ptr, @FrmMain_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 143
      fp->nText = WStr("")
      This.TreeView.hWnd = hWndControl 
      This.TreeView.IDC =143
      fp->nName = "TreeView"
      SetWindowTheme(hWndControl,"Explorer", NULL)
      This.TreeView.LineColor =GetCodeColorGDI(&H197F7F7F,CLR_DEFAULT)
      fp->CtlData(0) =&H197F7F7F
      This.TreeView.ForeColor =GetCodeColorGDI(&H197F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H197F7F7F
      This.TreeView.BackColor =GetCodeColorGDI(&H057F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H057F7F7F
      This.TreeView.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 200
      fp->nTop = 0
      fp->nWidth = 270
      fp->nHeight = 430
   End IF

'[Create control]  创建控件的代码，在此前一行插入。

      FrmMain.Frame3.Enabled = FrmMain.Frame3.Enabled

      FrmMain.Frame3.Visible = FrmMain.Frame3.Visible

      FrmMain.Frame2.Enabled = FrmMain.Frame2.Enabled

      FrmMain.Frame2.Visible = FrmMain.Frame2.Visible

      FrmMain.FmeAdvance.Enabled = FrmMain.FmeAdvance.Enabled

      FrmMain.FmeAdvance.Visible = FrmMain.FmeAdvance.Visible

'[Create control end]  创建全部控件后需要处理的代码，在此前一行插入。


   PostMessage(NewhWnd, &H1000, &H1000,CAst(lPArAm,UserData)) '触发显示事件
   
   hWndControl = NewhWnd '设置焦点后会发生消息循环的情况，要是同时多开同一个窗口，就发生 hWnd 被赋值为下一个窗口句柄的情况。
   This.TxtFilter2.SetFocus
'[SET_FOCUS] '将焦点设置为选项卡顺序中的最低控件 
   Function = FLY_DoMessagePump(pWindow,Modal, hWndControl, hWndParent, SW_SHOWNORMAL, False) ' IsMDIForm 如果这是MDI窗口，请设置标志
End Function

Function FrmMain_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult 'FrmMain 窗口回调过程
   Dim tLResult As LResult
   Select Case wMsg
      Case WM_SIZE
         If wParam <> SIZE_MINIMIZED Then
            '[CONTROL_WM_SIZE] 状态栏调整大小
            FLY_VFB_Layout_hWndForm(hWndForm) '控件布局调整大小
            '[CONTROL_WM_SIZE_END] 状态栏调整大小
         End If
      Case WM_PAINT '处理虚拟控件
         Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(hWndForm)
         if fp Then
            Dim WinCc As Long = GetCodeColorGDI(fp->BackColor)
            If WinCc = -1 Then WinCc = GetSysColor(COLOR_BTNFACE)
            Dim gg As yGDI = yGDI(hWndForm, WinCc, True)
            
'[FORM_PAINT_START]  主窗口开始画画事件，在此前一行插入。
            '[DRAWINGVIRTUALCONTROLS] '描绘虚拟控件，在此下一行插入（这样就最底层先画）。
            FrmMain.FmeAdvance.hWndForm = hWndForm 
            FrmMain.FmeAdvance.Drawing(gg,hWndForm,WinCc)  
            FrmMain.Label1.hWndForm = hWndForm 
            FrmMain.Label1.Drawing(gg,hWndForm,WinCc)  
            FrmMain.Line1.hWndForm = hWndForm 
            FrmMain.Line1.Drawing(gg,hWndForm,WinCc)  
            FrmMain.Frame2.hWndForm = hWndForm 
            FrmMain.Frame2.Drawing(gg,hWndForm,WinCc)  
            FrmMain.Frame3.hWndForm = hWndForm 
            FrmMain.Frame3.Drawing(gg,hWndForm,WinCc)  
            
'[FORM_PAINT_END] 主窗口开始画画事件，在此前一行插入。
         end if
   End Select
   '以下CASE从表单上的控件以及用户自己处理的任何这些消息中调用通知事件。
   Select Case wMsg
      Case &H1000
         if wParam = &H1000 then
            Dim rc as Rect '由于控件创建时间比尺寸消息晚，需要重新触发一次
            GetClientRect(hWndForm, @rc)
            SendMessage(hWndForm, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.Right - rc.Left, rc.Bottom - rc.Top))
             FRMMAIN_SHOWN(hWndForm ,Cast(Integer,lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|27]
'[FORM_SHOWN]  主窗口第一次显示事件，在此前一行插入。
            InvalidateRect(hWndForm, Null, True)
            UpdateWindow(hWndForm)
            '[Create window shadow]  创建窗口阴影，在此前一行插入。
            Return 0
         End If
      Case WM_COMMAND
         
'[FORM_WM_COMMAND]  主窗口命令处理事件，在此前一行插入。
                  Dim As Long IDC =LoWord(wParam) ,CODE = HiWord(wParam)  '控件处理初始化代码
                   If IDC = 102 And CODE = EN_CHANGE Then  ' TxtFilter2
              FRMMAIN_TXTFILTER2_EN_CHANGE(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|460]
           End If
          If IDC = 104 And CODE = BN_CLICKED Then  ' Check8
              FRMMAIN_CHECK8_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|425]
           End If
          If IDC = 105 And CODE = BN_CLICKED Then  ' Check7
              FRMMAIN_CHECK7_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|405]
           End If
          If IDC = 106 And CODE = BN_CLICKED Then  ' Check6
              FRMMAIN_CHECK6_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|358]
           End If
          If IDC = 107 And CODE = BN_CLICKED Then  ' Check5
              FRMMAIN_CHECK5_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|392]
           End If
          If IDC = 109 And CODE = BN_CLICKED Then  ' Check3
              FRMMAIN_CHECK3_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|328]
           End If
          If IDC = 111 And CODE = BN_CLICKED Then  ' Check2
              FRMMAIN_CHECK2_BN_CLICKED(hWndForm, Cast(HWND, lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|418]
           End If
'[CONTROL_WM_COMMAND]  控件处理事件，在此前一行插入。
      Case WM_HSCROLL
         
'[FORM_WM_HSCROLL]  主窗口和一些特殊控件要的事件，在此前一行插入。
      Case WM_VSCROLL
         
'[FORM_WM_VSCROLL]  主窗口和一些特殊控件要的事件，在此前一行插入。
         
         Case WM_CLOSE
         tLResult = FRMMAIN_WM_CLOSE(hWndForm )
           If tLResult Then Return tLResult  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|222]















'[Other events in main window]  主窗口其它事件，需要加 Case ，在此前一行插入。 注意每个消息值不可重复，不然后面的无法执行
               Case WM_DRAWITEM
         Dim lpdis As DRAWITEMSTRUCT Ptr = Cast(Any Ptr, lParam)
                  If Cast(Long, wParam) = 142  Then ' ListView1
           tLResult =  FRMMAIN_LISTVIEW1_OWNERDRAW(hWndForm,lpdis->hwndItem,*lpdis)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|471]
           If tLResult Then Return tLResult
         End If

'[FORM_WM_DRAWITEM]
      Case WM_TIMER
         If wParam = 357897 then '虚拟控件要刷新，使用防抖动方式刷新，避免虚拟控件多时，同时设置控件属性，造成大量刷新变卡顿。
            killTimer hWndForm, 357897
            AfxRedrawWindow hWndForm
         end if
                   If wParam = 124 Then
             FrmMain.TmrProtect.hWndForm = hWndForm 
             KillTimer(hWndForm, 124)
             FRMMAIN_TMRPROTECT_WM_TIMER(hWndForm, Cast(Long, wParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|205]
             FrmMain.TmrProtect.hWndForm = hWndForm 
             If FrmMain.TmrProtect.Enabled = True Then  SetTimer(hWndForm, 124, FrmMain.TmrProtect.Interval(), NULL) 
          End if 

'[FORM_WM_TIMER]  时间控件事件
      Case WM_NOTIFY
         Dim FLY_pNotify As NMHDR Ptr = Cast(NMHDR Ptr, lParam)
                  If (FLY_pNotify->idFrom = 142) And (FLY_pNotify->Code = LVN_COLUMNCLICK) Then
             FrmMain.ListView1.hWndForm = hWndForm
             FrmMain.ListView1.ColumnToSort(Cast(Any Ptr, lParam))
         End If
         If (FLY_pNotify->idFrom = 143) And (FLY_pNotify->Code = NM_DBLCLK) Then
              tLResult =  FRMMAIN_TREEVIEW_NM_DBLCLK(hWndForm, FLY_pNotify->hwndFrom)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|247]
            If tLResult Then Return tLResult
         End If
         If (FLY_pNotify->idFrom = 143) And (FLY_pNotify->Code = NM_RCLICK) Then
              tLResult =  FRMMAIN_TREEVIEW_NM_RCLICK(hWndForm, FLY_pNotify->hwndFrom)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|316]
            If tLResult Then Return tLResult
         End If
'[CONTROLS_NOTIFY]  控件事件，标准控件的事件都是按照这个传递
         
'[FORM_WM_NOTIFY]   主窗口的事件
   End Select
   Select Case wMsg
      Case WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK, WM_MOUSEMOVE, WM_CONTEXTMENU, WM_MOUSEWHEEL, WM_Timer
         Dim As Long xPos = GET_X_LPARAM(lParam), yPos = GET_Y_LPARAM(lParam)
         '[VIRTUAL_CONTROL_EVENTS] 处理虚拟控件事件，在此前一行插入。
         
   End Select
   
   
   
   If wMsg = WM_COMMAND Then 
      Dim wID As ULong = LoWord(wParam)
      If wID >= 117 And wID <= 123 Then
         FrmMain.mnuFile.hWndForm = hWndForm 
         FRMMAIN_MNUFILE_WM_COMMAND(hWndForm, wID)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|263]
      End If 
   End If 

   If wMsg = WM_COMMAND Then 
      Dim wID As ULong = LoWord(wParam)
      If wID >= 125 And wID <= 127 Then
         FrmMain.TopMenu1.hWndForm = hWndForm 
         FRMMAIN_TOPMENU1_WM_COMMAND(hWndForm, wID)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|446]
      End If 
   End If 

   If wMsg = WM_COMMAND Then 
      Dim wID As ULong = LoWord(wParam)
      If wID >= 129 And wID <= 141 Then
         FrmMain.mnuProcess.hWndForm = hWndForm 
         FRMMAIN_MNUPROCESS_WM_COMMAND(hWndForm, wID)  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|75]
      End If 
   End If 

'[CALL_FORM_CUSTOM] 处理任何自定义消息。
   
   '以下CASE处理VisualFreeBasic内部要求
   Select Case wMsg
      Case WM_GETMINMAXINFO
         '不要为MDI子窗体处理此消息，因为它会干扰子窗体的最大化。
         If (GetWindowLongPtr(hWndForm, GWL_EXSTYLE) And WS_EX_MDICHILD) <> WS_EX_MDICHILD Then
            DefWindowProcW(hWndForm, wMsg, wParam, lParam)
            Dim FLY_pMinMaxInfo As MINMAXINFO Ptr = Cast(MINMAXINFO Ptr, lParam)
            
            
            
            
            Return 0
         End If
      Case WM_SYSCOMMAND
         If (wParam And &HFFF0) = SC_CLOSE Then
            SendMessage hWndForm, WM_CLOSE, wParam, lParam
            Exit Function
         End If
      Case WM_SETFOCUS
         '将焦点设置回正确的子控件。
         Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(hWndForm)
         If fp then
            If IsWindow(fp->CtrlFocus) Then '预防控件已经关闭或没控件
               If GetAncestor(fp->CtrlFocus, GA_ROOT) = hWndForm Then '预防控件被设置其它主窗口了
                  SetFocus fp->CtrlFocus
               End If
            End If
         End If
      Case WM_SYSCOLORCHANGE, WM_THEMECHANGED '当对系统颜色设置进行更改, 在主题更改事件之后广播到每个窗口。
         '为窗体上的控件重新创建任何背景画笔。
         '将此消息转发给任何通用控件，因为它们不会自动接收此消息。
         Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(hWndForm)
         If fp then
            Dim cc as Long = GetCodeColorGDI(fp->BackColor)
            if cc = -1        then cc = GetSysColor(COLOR_BTNFACE)
            If fp->hBackBrush Then DeleteBrush(fp->hBackBrush)
            fp->hBackBrush = CreateSolidBrush(cc)
            dim zhWnd as HWND = GetWindow(hWndForm, GW_CHILD)
            While zhWnd
               dim zfp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(zhWnd)
               if zfp Then
                  Dim zcc as Long = GetCodeColorGDI(zfp->BackColor)
                  if zcc = -1        then zcc = cc
                  If zfp->hBackBrush Then DeleteBrush(zfp->hBackBrush)
                  zfp->hBackBrush = CreateSolidBrush(zcc)
               End if
               zhWnd = GetWindow(zhWnd, GW_HWNDNEXT)
            Wend
         End If
         
      Case WM_SETCURSOR
         dim a as long = LOWORD(lParam)
         if a = 1 then '位于客户区
            Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(cast(hwnd, wParam))
            if fp Then
               if fp->nCursor then
                  SetCursor(fp->nCursor)
                  Return True
               end if
            end if
         end if
      Case WM_PAINT
         Return True '采用自绘，需要立即返回
      Case WM_ERASEBKGND ' 采用自绘，不需要系统画背景
         Return True '防止擦除背景，不加这个会闪的。 与mCrtl控件冲突，加了后，mCrtl控件有透明的就不会画了。去掉这行，对窗口也无影响。
      Case WM_MOUSEMOVE
         
         
'[CALL_FORM_WM_MOUSEMOVE]
      Case WM_Close
         '处理模式窗口，发现所有者窗口被禁用后，恢复即可 ------
         '以前在消息循环中处理，关闭窗口后，由于此时所有者窗口还被禁用中，系统就把前台窗口交给下一个窗口
         '虽然在消息循环后面重新设置前台窗口，但会发生快速切换前台窗口的闪，虽然很快很难察觉，特殊情况下还会发生前台失败、闪耀明显感觉等。
         '在关闭窗口前恢复被禁用的所有者窗口，其它无需任何处理就完美解决。
         '本来在消息循环里判断 WM_Close ，然后没想到消息循环里并没有这个消息，只能用在这里才可以。
         Dim hWndParent As hWnd = GetParent(hWndForm)
         If hWndParent Then
            While (GetWindowLongPtr(hWndParent, GWL_STYLE) And WS_CHILD) <> 0 '确定活动控件的顶层窗口
               hWndParent = GetParent(hWndParent)
               If IsWindow(hWndParent) = 0 Then Exit While
               If (GetWindowLongPtr(hWndParent, GWL_EXSTYLE) And WS_EX_MDICHILD) <> 0 Then Exit While
            Wend
            If IsWindow(hWndParent) Then EnableWindow(hWndParent, True)
         End If
      Case WM_DESTROY
         
       FrmMain.mnuFile.hWndForm = hWndForm 
       FrmMain.mnuFile.UnLoadControl() 

       FrmMain.TopMenu1.hWndForm = hWndForm 
       FrmMain.TopMenu1.UnLoadControl() 

       FrmMain.mnuProcess.hWndForm = hWndForm 
       FrmMain.mnuProcess.UnLoadControl() 

'[CALL_FORM_WM_DESTROY]  窗口卸载事件
         
                  KillTimer hWndForm, 124
'[DELETE_USER_TIMERS] 删除任何用户定义的Timer控件
         
         FrmMain.hWnd = 0
         Dim pWindow AS CWindow Ptr = AfxCWindowPtr(hWndForm)
         If pWindow Then Delete pWindow
         Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(hWndForm)
         While fp '清理虚拟控件
            if fp->BigIcon    then DestroyIcon fp->BigIcon
            if fp->SmallIcon  then DestroyIcon fp->SmallIcon
            if fp->hBackBrush then DeleteBrush(fp->hBackBrush)
            If fp->ToolWnd    Then DestroyWindow fp->ToolWnd
            if fp->nCursor    then DestroyCursor fp->nCursor
            Dim sfp As FormControlsPro_TYPE ptr = fp->VrControls
            Delete fp
            fp = sfp
         Wend
         vfb_Remove_Control_Ptr(hWndForm)
      Case WM_CTLCOLOREDIT, WM_CTLCOLORLISTBOX, WM_CTLCOLORSTATIC, WM_CTLCOLORBTN
         '从子控件收到的消息即将显示。在此处设置颜色属性。lParam是Control的句柄。 wParam是hDC。
         Dim fp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(Cast(HWND, lParam))
         If fp then
            Dim cc as Long = GetCodeColorGDI(fp->ForeColor)
            if cc <> -1 then SetTextColor(Cast(hDC, wParam), cc)
            cc = GetCodeColorGDI(fp->BackColor)
            if cc = -1 Then
               Dim wfp As FormControlsPro_TYPE ptr = vfb_Get_Control_Ptr(hWndForm)
               if wfp = 0 then exit select
               cc = GetCodeColorGDI(wfp->BackColor)
            End If
            if cc = -1 then cc = GetSysColor(COLOR_BTNFACE)
            SetBkColor(Cast(hDC, wParam), cc)
            If fp->hBackBrush Then DeleteBrush(fp->hBackBrush)
            fp->hBackBrush = CreateSolidBrush(cc)
            '            SetBkMode Cast(hDC, wParam), TRANSPARENT
            Return Cast(LRESULT, fp->hBackBrush)
         End If
   End Select
   Function = DefWindowProcW(hWndForm, wMsg, wParam, lParam)
End Function

Function FrmMain_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT  ' FrmMain 控件回调
   'FrmMain 窗口上每个控件的所有消息都在此函数中处理。
   Dim fp As FormControlsPro_TYPE ptr 
   Dim tLResult As LResult
   Dim IDC As Long = GetDlgCtrlId(hWndControl)

'[CONTROL_LEAVEHOVER]  '启用鼠标出窗口检查

'[CALL_CONTROL_CUSTOM]  处理任何自定义消息。

   '以下CASE在处理用户定义事件之前处理内部VisualFreeBasic要求（这些事件在此之后的单独CASE中处理）。
   Select Case wMsg
      Case 0  '需要这一行，万一没控件事件，语句也能成立

         
      Case WM_LBUTTONDBLCLK ''' 
          If IDC = 142 Then  ' ListView1
              FRMMAIN_LISTVIEW1_WM_LBUTTONDBLCLK(GetParent(hWndControl), hWndControl, Cast(Long, wParam), GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|345]
          End If

          If IDC = 128 Then  ' treMain
              FRMMAIN_TREMAIN_WM_LBUTTONDBLCLK(GetParent(hWndControl), hWndControl, Cast(Long, wParam), GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|154]
          End If

      Case WM_CONTEXTMENU ''' 
          If IDC = 142 Then  ' ListView1
              FRMMAIN_LISTVIEW1_WM_CONTEXTMENU(GetParent(hWndControl), hWndControl, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam))  '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|62]
          End If

'[CONTROL_CASE_OTHER]  '控件事件。         

   End Select
   
    Select Case wMsg
       Case WM_DESTROY  '// 需要：删除控件子类
           fp = vfb_Get_Control_Ptr(hWndControl)
           Select Case IDC 
               Case 0  '需要这一行，万一没控件事件，语句也能成立
'[CALL_CONTROL_DESTROY] 销毁。  
           End Select

          SetWindowLongPtrW hWndControl ,GWLP_WNDPROC ,CAST(LONG_PTR ,RemovePropW(hWndControl ,"OLDWNDPROC"))
            vfb_Remove_Control_Ptr(hWndControl)           
            If fp Then
               if fp->hBackBrush then DeleteBrush(fp->hBackBrush)
               If fp->ToolWnd Then DestroyWindow fp->ToolWnd
               if fp->nCursor then DestroyCursor fp->nCursor
               Delete fp
            End If
   
       Case WM_SETFOCUS
         ' 如果这是一个TextBox，我们检查是否需要突出显示文本。
'         If fp Then
'            If ff->SelText Then 
'               SendMessage hWndControl, EM_SETSEL, 0, -1
'            Else
'               SendMessage hWndControl, EM_SETSEL, -1, 0
'            End If   
'         End If
         ' 将焦点控件存储在父窗体中如果此窗体是TabControl子窗体，则需要将CtrlFocus存储在此子窗体的父窗体中。
         Dim FLY_tempLong As Long = GetWindowLongPtr(GetParent(hWndControl), GWL_STYLE)
         If (FLY_tempLong And WS_CHILD) = WS_CHILD Then
            ' 必须是TabControl子对话框
            fp = vfb_Get_Control_Ptr(GetParent(GetParent(hWndControl)))
         Else   
            fp = vfb_Get_Control_Ptr(GetParent(hWndControl))
         End If   
         If fp Then fp->CtrlFocus = hWndControl          
                
   End Select
   '此控件是子类，因此我们必须将所有未处理的消息发送到原始窗口过程。
   FUNCTION = CallWindowProcW(GetPropW(hWndControl, "OLDWNDPROC"), hWndControl, wMsg, wParam, lParam)
   
End Function
'这是标准的工程模版，你也可做自己的模版。
'写好工程，复制全部文件到VFB软件文件夹里【template】里即可，子文件夹名为 VFB新建工程里显示的名称
'快去打造属于你自己的工程模版吧。
' #include Once "win/winuser.bi" '已经搬到定义文件处
' Const MAX_LEN = 200 '已经搬到定义文件处
' Dim Shared hook As HHOOK '已经搬到定义文件处
' Dim Shared CurrentType As Integer = -1'None '已经搬到定义文件处
' Dim Shared hDrv As HANDLE '已经搬到定义文件处
' Dim Shared IsHaveSelectItem As BOOLEAN '已经搬到定义文件处
' Dim Shared IsDriverLoaded As BOOLEAN = False '已经搬到定义文件处
' Dim Shared PIDs() As DWORD '已经搬到定义文件处

' Dim Shared CurrentPID As DWORD '已经搬到定义文件处
' Dim Shared CurrentProcessName As String '已经搬到定义文件处

Private Sub DrawTreeView()
    Dim treKernel As HTREEITEM, treHardDisk As HTREEITEM
    FrmMain.treMain.AddItem NULL, "前台窗口"
    FrmMain.treMain.AddItem NULL, "进程"
    treKernel = FrmMain.treMain.AddItem(NULL, "内核")
    FrmMain.treMain.AddItem treKernel, "内核模块"
    FrmMain.treMain.AddItem treKernel, "系统回调"
    treHardDisk = FrmMain.treMain.AddItem(NULL, "硬盘")
    FrmMain.treMain.AddItem treHardDisk, "文件"
    FrmMain.treMain.AddItem NULL, "高级设置"
End Sub

Sub FrmMain_Shown(hWndForm As hWnd, UserData As Integer)
    'Dim sa As Any Ptr
   'ChangeWindowMessageFilter(0x0049, MSGFLT_ADD)'允许拖放文件 
    AdjustPrivilege
    AdjustPrivilege SE_LOAD_DRIVER_NAME
    DrawTreeView
    FrmListView.Hide
    'SetWindowZ Picture1.hWnd
    FrmMain.TreeView.Visible = False
    FrmMain.FmeAdvance.Visible = False
    SetWindowZ FrmMain.ListView1.hWnd
    InitializeListView FrmMain.ListView1, Process
    GetProcessList FrmMain.ListView1, False
    RefreshListViewItem FrmMain.ListView1
    'ForceKillProcess 32728
    'AfxMsg Process_GetPath(972)
    'Dim wstrFilePath As WString * 20 = "D:\example.txt"
    'ReDim PIDs(0) As DWORD
    'UnlockOccupyFile @wstrFilePath
    'main
    'ForceKillProcess 36032
    'AfxMsg FrmMain_TopMenu1_mnuCreateProcess
    'FrmMain_TopMenu1_mnuCreateProcess = 1
    'sa = @WndProc
    'AfxMsg TypeOf(sa)
    'AfxMsg WinErrorMsg(DeleteFileW("D:\SnowSword.txt"))
    'GetMBR
    'PrintProcessModules
    'InitSelfProtect
End Sub

'[Form1.ListView1]事件 : 鼠标右键单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'xPos yPos   当前鼠标位置，相对于屏幕。就是屏幕坐标。
Sub FrmMain_ListView1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long)
    Select Case CurrentType
        Case Process
            PopupMenu hWndForm,FrmMain.mnuProcess.HMENU
        Case File
            PopupMenu hWndForm, FrmMain.mnuFile.HMENU
    End Select
End Sub

'[FrmMain.mnuProcess]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码
'wID      菜单项命令ID
Sub FrmMain_mnuProcess_WM_Command(hWndForm As hWnd, wID As ULong)
    CurrentPID = ValULng(FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0))
    CurrentProcessName = FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 2)
    Select Case wID
        Case FrmMain_mnuProcess_mnuTerminateProcess ' 结束进程
            Dim dwProcessId As DWORD
            For i As Integer = FrmMain.ListView1.ItemCount To 1 Step -1
                If IsListViewItemSelected(FrmMain.ListView1, i) Then
                    dwProcessId = ValULng(FrmMain.ListView1.GetItemText(i, 0))
                    If Not KillProcess(dwProcessId) Then
                        AfxMsg "结束进程" & dwProcessId & "失败!"
                        Continue For
                    End If
                    FrmMain.ListView1.DeleteItem i
                End If
            Next
            RefreshListViewItem FrmMain.ListView1
        Case FrmMain_mnuProcess_mnuForceTerminateProcess '强制结束进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            For i As Integer = FrmMain.ListView1.ItemCount To 1 Step -1
                If IsListViewItemSelected(FrmMain.ListView1, i) Then
                    dwProcessId = ValULng(FrmMain.ListView1.GetItemText(i, 0))
                    IoControl hDrv, CTL_CODE_GEN(&H800), @dwProcessId, 260, @ret, 100, @lpRet
                    FrmMain.ListView1.DeleteItem i
                End If
            Next
            RefreshListViewItem FrmMain.ListView1
        Case FrmMain_mnuProcess_mnuViolentTerminateProcess '暴力结束进程
            Dim dwProcessId As DWORD = ValULng(FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0))
            Dim ret As String * 100, lpRet As DWORD
            If AfxMsg("是否使用内存清零杀进程?", "提示", MB_YESNO) = IDYES Then
                IoControl hDrv, CTL_CODE_GEN(&H803), @dwProcessId, 260, @ret, 100, @lpRet
            Else
                IoControl hDrv, CTL_CODE_GEN(&H808), @dwProcessId, 260, @ret, 100, @lpRet
            End If
        Case FrmMain_mnuProcess_mnuSuspendProcess ' 挂起进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            For i As Integer = FrmMain.ListView1.ItemCount To 1 Step -1
                If IsListViewItemSelected(FrmMain.ListView1, i) Then
                    dwProcessId = ValULng(FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0))
                    If Not SuspendProcess(dwProcessId) Then AfxMsg "挂起进程" & dwProcessId & "失败!" 
                End If
            Next
        Case FrmMain_mnuProcess_mnuResumeProcess ' 恢复进程
            Dim ret As String * 100, lpRet As DWORD, dwProcessId As DWORD
            For i As Integer = FrmMain.ListView1.ItemCount To 1 Step -1
                If IsListViewItemSelected(FrmMain.ListView1, i) Then
                    dwProcessId = ValULng(FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0))
                    If Not ResumeProcess(dwProcessId) Then AfxMsg "恢复进程" & dwProcessId & "失败!" 
                End If
            Next
        Case FrmMain_mnuProcess_mnuProtectProcess
            If (Not IsDriverLoaded) Then
                AfxMsg "使用此功能需要先加载驱动!"
                Exit Sub
            End If
            Dim dwProcessId As DWORD = ValULng(FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0)), ret As Long, lpRet As DWORD
            IoControl hDrv, CTL_CODE_GEN(&H805), @dwProcessId, SizeOf(DWORD), @ret, 100, @lpRet
        Case FrmMain_mnuProcess_mnuCheckHideProcess '检测隐藏进程
            SetMenuCheckState FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess, Not GetMenuCheckState(FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
        Case FrmMain_mnuProcess_mnuRefresh ' 刷新
            GetProcessList FrmMain.ListView1, GetMenuCheckState(FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
            RefreshListViewItem FrmMain.ListView1
        Case FrmMain_mnuProcess_mnuViewModule ' 查看模块
            FrmListView.Show,, Module
        Case FrmMain_mnuProcess_mnuViewThread ' 查看线程
            FrmListView.Show,, Thread
        Case FrmMain_mnuProcess_mnuViewMemory ' 查看内存
            FrmListView.Show,, Memory
    End Select
End Sub
'SetWIndowsHookEx
'[FrmMain.treMain]事件 : 双击鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_treMain_WM_LButtonDblclk(hWndForm As hWnd,hWndControl As hWnd,MouseFlags As Long,xPos As Long,yPos As Long)
    Dim SelectText As String = FrmMain.treMain.Text(FrmMain.treMain.HitTest(xPos, yPos))
    If SelectText = "前台窗口" Then
        FrmMain.ListView1.Left = 400'200
        FrmMain.ListView1.Width = 1080 '540
        FrmMain.ListView1.Visible = True
        SetWindowZ FrmMain.ListView1.hWnd
        InitializeListView FrmMain.ListView1,ForegroundWindow
        GetVisibleWindowList FrmMain.ListView1
    ElseIf SelectText = "进程" Then
        FrmMain.ListView1.Left = 400'200
        FrmMain.ListView1.Width = 1080 '540
        FrmMain.ListView1.Visible = True
        SetWindowZ FrmMain.ListView1.hWnd
        InitializeListView FrmMain.ListView1,Process
        GetProcessList FrmMain.ListView1, GetMenuCheckState(FrmMain.mnuProcess, FrmMain_mnuProcess_mnuCheckHideProcess)
    ElseIf SelectText = "内核模块" Then
        FrmMain.ListView1.Left = 400'200
        FrmMain.ListView1.Width = 1080 '540
        FrmMain.ListView1.Visible = True
        SetWindowZ FrmMain.ListView1.hWnd
        InitializeListView FrmMain.ListView1,KernelModule
        GetKernelModuleList FrmMain.ListView1
    ElseIf SelectText = "系统回调" Then
        FrmMain.ListView1.Left = 400'200
        FrmMain.ListView1.Width = 1080 '540
        FrmMain.ListView1.Visible = True
        SetWindowZ FrmMain.ListView1.hWnd
        InitializeListView FrmMain.ListView1,Callbacks
        GetCallbackList FrmMain.ListView1
    ElseIf SelectText = "文件" Then
        FrmMain.ListView1.Left =  940
        FrmMain.ListView1.Width = 540
        FrmMain.ListView1.Visible = True
        FrmMain.TreeView.Visible = True
        SetWindowZ FrmMain.ListView1.hWnd
        SetWindowZ FrmMain.TreeView.hWnd
        InitializeListView FrmMain.ListView1,File
        GetDriveList FrmMain.TreeView
    ElseIf SelectText = "高级设置" Then
        FrmMain.ListView1.Visible = False
        FrmMain.TreeView.Visible = False
        Exit Sub
    End If
    RefreshListViewItem FrmMain.ListView1
End Sub

'[FrmMain.TmrProtect]事件 : 定时器
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''         本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wTimerID  定时器ID，区分不同定时器的编号
Sub FrmMain_TmrProtect_WM_Timer(hWndForm As hWnd,wTimerID As Long)'WMI
    Dim PID() As DWORD,ProcessName() As String,i As Integer,Path As String,ret As String
    GetAllPID(ProcessName(),PID(),0)
    For i = 0 To UBound(PID) - 1
        Path = Process_GetPath(PID(i))
        'If PID(i) = 24768 Then AfxMsg Path & " " & Right(Path,Len(Path) - InStrRev(Path,"\"))
        If InStr(Path,"\") <> 0 Then
            If LCase(Right(Path,Len(Path) - InStrRev(Path,"\"))) = "taskmgr.exe" Then
                ret = RemoteDLLinjection(PID(i),"D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release64\InineHook.dll")
                If ret <> "" Then AfxMsg ret
            End If
        End If
    Next
End Sub

'[FrmMain]事件 : 即将关闭窗口，返回非0可阻止关闭
'hWndForm  当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
Function FrmMain_WM_Close(hWndForm As hWnd) As LResult
    'If (hook <> 0) Then UnhookWindowsHookEx hook
    If (IsDriverLoaded) Then
        CloseDrv hDrv
        UnloadDriver "C:\Users\21607\source\repos\SnowSword\x64\Debug\SnowSword.sys", False
    End If
    'UninitSelfProtect
    Function = FALSE ' 返回 TRUE 阻止关闭窗口。
End Function

Sub DeleteChildItem(MyTreeView As Class_TreeView, Node As HTREEITEM)
    Dim CurrentItem As HTREEITEM = MyTreeView.GetChild(Node)
    Dim NextItem As HTREEITEM
    'If (CurrentItem = NULL) Then Exit Sub
    Do While CurrentItem <> NULL
        NextItem = MyTreeView.GetNextSiblin(CurrentItem)
        MyTreeView.DeleteItem CurrentItem
        CurrentItem = NextItem
        FF_DoEvents
    Loop
End Sub

'[FrmMain.TreeView]事件 : 双击了鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_DBLCLK(hWndForm As hWnd,hWndControl As hWnd) As LResult
    Dim CurrentNode As HTREEITEM = FrmMain.TreeView.Selection
    If (FrmMain.TreeView.GetChild(CurrentNode) <> NULL) Then
        DeleteChildItem FrmMain.TreeView, CurrentNode
    Else
        FrmMain.ListView1.DeleteAllItems
        GetFileList CurrentNode, FrmMain.TreeView, FrmMain.ListView1
        'TreeView.ExpandEx(CurrentNode, TVE_EXPAND)
    End If
    Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.mnuFile]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_mnuFile_WM_Command(hWndForm As hWnd,wID As ULong)
    Dim CurrentNode As HTREEITEM = FrmMain.TreeView.Selection
    Dim CurrentPath As StringW
    GetFilePathByNodeW CurrentNode, FrmMain.TreeView, CurrentPath
    'AfxMsg *CurrentPath
    Select Case wID
        Case FrmMain_mnuFile_mnuCreateFile ' 新建文件
            Dim hFile As HANDLE
            Dim FileName As String = AfxInputBox(hWndForm,,, "提示", "请输入文件名:")
            hFile = CreateFileW(CurrentPath & FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL)
            If (hFile > 0) Then
                FrmMain.TreeView.InsertItem CurrentNode,TVI_SORT,FileName
                GetFileList CurrentNode,FrmMain.TreeView,FrmMain.ListView1
                AfxMsg "创建文件成功!"
                CloseHandle hFile
            End If
        Case FrmMain_mnuFile_mnuCreateFolder ' 新建文件夹
            Dim FolderName As String = AfxInputBox(NULL,,,"提示","请输入文件夹名:")
            If (MkDir(CurrentPath & FolderName) = 0) Then
                FrmMain.TreeView.InsertItem CurrentNode,TVI_SORT,FolderName
                GetFileList CurrentNode,FrmMain.TreeView,FrmMain.ListView1
                AfxMsg "创建文件夹成功!"
            Else
                AfxMsg Err
            End If
        Case FrmMain_mnuFile_mnuCopy ' 复制

        Case FrmMain_mnuFile_mnuPaste ' 粘贴

        Case FrmMain_mnuFile_mnuDelete ' 删除
            Dim Path As StringW, ret As WINBOOL,strSub As LPWSTR,strReplace As LPWSTR
            Path = CurrentPath & FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0)
            '*strSub = WStr("\")
            '*strReplace = WStr("\\")
            AfxMsg Path
            AfxMsg CurrentPath & FrmMain.ListView1.GetItemText(FrmMain.ListView1.SelectedItem, 0)
            'AfxMsg *ReplaceW(Path,strSub,strReplace)
            ret = DeleteFileW(Path)'ReplaceW(Path,strSub,strReplace))
            If (ret<0) Then
                Dim ParentNode As HTREEITEM = FrmMain.TreeView.GetParent(CurrentNode)
                FrmMain.TreeView.DeleteItem CurrentNode
                FrmMain.ListView1.DeleteAllItems
                GetFileList ParentNode,FrmMain.TreeView,FrmMain.ListView1
                AfxMsg "删除文件(夹)成功!"
            Else
                AfxMsg WinErrorMsg(GetLastError)' & WinErrorMsg(ret)
            End If
    End Select
End Sub

'[FrmMain.TreeView]事件 : 单击了鼠标右键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Function FrmMain_TreeView_NM_RCLICK(hWndForm As hWnd, hWndControl As hWnd) As LResult
    'Dim a As CRegExp
    Select Case CurrentType
        Case File
            PopupMenu hWndForm,FrmMain.mnuFile.HMENU
    End Select
   Function = False '返回 TRUE 非零以防止默认处理，返回 False 零以允许默认处理。
End Function

'[FrmMain.Check3]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check3_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) Then
        AfxMsg "使用此功能需要先加载驱动!"
        FrmMain.Check3.Value = False
        Exit Sub
    End If
    Dim isStatus As BOOLEAN = FrmMain.Check3.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, CTL_CODE_GEN(&H830), @isStatus, SizeOf(BOOLEAN), @ret, 100, @lpRet
End Sub

'[FrmMain.ListView1]事件 : 双击鼠标左键
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'MouseFlags  MK_CONTROL   MK_LBUTTON     MK_MBUTTON     MK_RBUTTON    MK_SHIFT     MK_XBUTTON1       MK_XBUTTON2 
''           CTRL键按下   鼠标左键按下   鼠标中键按下   鼠标右键按下  SHIFT键按下  第一个X按钮按下   第二个X按钮按下
'检查什么键按下用  If (MouseFlags And MK_CONTROL)<>0 Then CTRL键按下 
'xPos yPos   当前鼠标位置，相对于控件。就是在控件里的坐标。
Sub FrmMain_ListView1_WM_LButtonDblclk(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long)
    Select Case CurrentType
        Case Process
            If (Not IsDriverLoaded) Then
                AfxMsg "请先加载驱动!"
                Exit Sub
            End If
    End Select
End Sub

'[FrmMain.Check6]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check6_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (FrmMain.Check6.Value) Then
        LoadDriver "C:\Users\21607\source\repos\SnowSword\x64\Debug\SnowSword.sys", False
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            IsDriverLoaded = True
            'u1 ValULng(AfxInputBox(,,, "提示", "请输入PID:")), ValULng(AfxInputBox(,,, "提示", "请输入句柄:"))
            'TopMenu1.Check FrmMain_TopMenu1_mnuLoadDriver, True
            'Dim dwProcessId As DWORD = ValULng(AfxInputBox(,,, "提示", "请输入要结束进程的PID:"))
            'Dim strFilePath As WString * 260 = "\??\C:\Users\21607\Desktop\1234.txt"
            'Dim ret As String * 100, lpRet As DWORD
            'AfxMsg "DeviceIoControl return:" & IoControl(hDrv, CTL_CODE_GEN(&H820), @strFilePath, 260, @ret, 100, @lpRet)
            'AfxMsg "Result:" & ret
            'AfxMsg "Len of the result:" & lpRet
        Else
            AfxMsg "加载失败!"
        End If
    Else
        CloseDrv hDrv
        UnloadDriver "C:\Users\21607\source\repos\SnowSword\x64\Debug\SnowSword.sys", False
        IsDriverLoaded = False
        AfxMsg "卸载成功!"
        FrmMain.Check3.Value = False
        FrmMain.Check4.Value = False
        FrmMain.Check5.Value = False
        FrmMain.Check8.Value = False
        'TopMenu1.Check FrmMain_TopMenu1_mnuLoadDriver, False
    End If
End Sub

'[FrmMain.Check5]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check5_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) Then
        AfxMsg "使用此功能需要先加载驱动!"
        FrmMain.Check5.Value = False
        Exit Sub
    End If
    Dim isStatus As BOOLEAN = FrmMain.Check5.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, CTL_CODE_GEN(&H831), @isStatus, SizeOf(BOOLEAN), @ret, 100, @lpRet
End Sub

'[FrmMain.Check7]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check7_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) Then
        AfxMsg "使用此功能需要先加载驱动!"
        FrmMain.Check7.Value = False
        Exit Sub
    End If
    Dim isStatus As BOOLEAN = FrmMain.Check7.Value, ret As Long, lpRet As DWORD
    IoControl hDrv, CTL_CODE_GEN(&H832), @isStatus, SizeOf(BOOLEAN), @ret, 100, @lpRet
End Sub

'[FrmMain.Check2]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check2_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    ShieldMsgHook(FrmMain.Check2.Value)
End Sub

'[FrmMain.Check8]事件 : 单击
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_Check8_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd)
    If (Not IsDriverLoaded) Then
        AfxMsg "使用此功能需要先加载驱动!"
        FrmMain.Check7.Value = False
        Exit Sub
    End If
    Dim dwProcessId As DWORD = GetCurrentProcessId, ret As Long, lpRet As DWORD
    If (FrmMain.Check8.Value) Then
        IoControl hDrv, CTL_CODE_GEN(&H805), @dwProcessId, SizeOf(DWORD), @ret, 100, @lpRet
        IoControl hDrv, CTL_CODE_GEN(&H807), @dwProcessId, SizeOf(DWORD), @ret, 100, @lpRet
    Else
        IoControl hDrv, CTL_CODE_GEN(&H806), @dwProcessId, SizeOf(DWORD), @ret, 100, @lpRet
        dwProcessId = 0
        IoControl hDrv, CTL_CODE_GEN(&H805), @dwProcessId, SizeOf(DWORD), @ret, 100, @lpRet
    End If
End Sub

'[FrmMain.TopMenu1]事件 : 点击了菜单项
'hWndForm 当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
''           本控件为功能控件，就是无窗口，无显示，只有功能。如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 
'wID      菜单项命令ID
Sub FrmMain_TopMenu1_WM_Command(hWndForm As hWnd,wID As ULong)
   Select Case wID
        Case FrmMain_TopMenu1_mnuUnlockFile ' 解锁文件
            If (Not IsDriverLoaded) Then
                AfxMsg "使用此功能需要先加载驱动!"
                Exit Sub
            End If
            FrmListView.Show,, UnlockTheFile
   End Select
End Sub

'[FrmMain.TxtFilter2]事件 : 文本已经被修改（修改前用 EN_UPDATE
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
Sub FrmMain_TxtFilter2_EN_Change(hWndForm As hWnd, hWndControl As hWnd)
    FilterListViewItem FrmMain.ListView1, FrmMain.TxtFilter2.Text
End Sub
'[FrmMain.ListView1]事件 : 自绘控件（需要设计时选择自绘属性）
'hWndForm    当前窗口的句柄(WIN系统用来识别窗口的一个编号，如果多开本窗口，必须 Me.hWndForm = hWndForm 后才可以执行后续操作本窗口的代码)
'hWndControl 当前控件的句柄(也是窗口句柄，如果多开本窗口，必须 Me.控件名.hWndForm = hWndForm 后才可以执行后续操作本控件的代码 )
'lpdis.hDC         设备上下文的句柄 
'lpdis.rcItem      一个矩形，要绘制的控件的边界。
'lpdis.itemAction  绘图操作: ODA_DRAWENTIRE 需要绘制整个控件, ODA_FOCUS 失去或获得焦点, ODA_SELECT 选择状态已更改
''                 例： if (lpdis.itemAction And ODA_DRAWENTIRE)<>0  Then  
'lpdis.itemState   视觉状态  详细说明，选中它 DRAWITEMSTRUCT 然后按 F1 键盘  
Function FrmMain_ListView1_OwnerDraw(hWndForm As hWnd, hWndControl As hWnd,lpdis As DRAWITEMSTRUCT) As LResult


   Function = FALSE ' 如果处理了此事件，则应返回 TRUE 。
End Function

