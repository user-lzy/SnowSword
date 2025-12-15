'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmThread.Frm|391]
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分49秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------

'------------------------------------------------------------------------------
'VisualFreeBasic 自动生成的 FrmThread 函数
'------------------------------------------------------------------------------
'{FORM_WS_EX_NOACTIVATE3}  '为无焦点窗口创建定义
Function FrmThread_Class_Form.Show(hWndParent As .hWnd = HWND_DESKTOP, Modal As Boolean = False, UserData As Integer = 0) As .hWnd  '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
   '注：hWnd 是类属性，与API的 hWnd 冲突，因此使用 .hWnd 表示API，就是前面加个小数点。
   If This.Repeat = False Then     '是不是不允许多开窗口
      If IsWindow(This.hWnd) Then 
         This.FlashWindow
         Return This.hWnd
      End if 
   End if 
'{FORM_WS_EX_NOACTIVATE}  '为无焦点窗口创建多线程代码
   Dim pWindow AS CWindow Ptr = New CWindow(WStr("SnowSword_FrmThread"))
   '创建一个窗口
  Dim NewhWnd As .hWnd = pWindow->Create(hWndParent, WStr(""), Cast(WNDPROC, @FrmThread_FORMPROCEDURE), 0, 0, 420, 300, WS_THICKFRAME Or WS_CAPTION Or WS_SYSMENU Or WS_MINIMIZEBOX Or WS_MAXIMIZEBOX Or WS_CLIPSIBLINGS Or WS_CLIPCHILDREN Or WS_POPUP Or WS_SIZEBOX, WS_EX_WINDOWEDGE Or WS_EX_LEFT Or WS_EX_LTRREADING Or WS_EX_RIGHTSCROLLBAR Or WS_EX_CONTROLPARENT)
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
   fp->nName = "FrmThread"
   
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
    
'[Create control top]  顶部创建控件的代码，（主要为了顶部菜单，必须在所有控件之前创建）在此前一行插入。
        
   hWndControl = pWindow->AddControl("LISTVIEW", hWnd, 102, WStr(""), 0, 0, 410, 260,WS_CHILD Or WS_CLIPCHILDREN Or WS_CLIPSIBLINGS Or LVS_SHAREIMAGELISTS Or LVS_REPORT Or LVS_SINGLESEL Or LVS_SHOWSELALWAYS Or WS_VISIBLE Or WS_TABSTOP ,WS_EX_CLIENTEDGE , , Cast(Any Ptr, @FrmThread_CODEPROCEDURE))
   If hWndControl Then 
      Dim fp As FormControlsPro_TYPE ptr = new FormControlsPro_TYPE
      vfb_Set_Control_Ptr(hWndControl,fp)
      fp->hWndParent = hWnd
      fp->Index = -1
      fp->IDC = 102
      fp->nText = WStr("")
      This.lvwThread.hWnd = hWndControl 
      This.lvwThread.IDC =102
      fp->nName = "lvwThread"
      This.lvwThread.InsertColumn 0, "线程ID",0, AfxScaleX(50)
      This.lvwThread.InsertColumn 1, "线程优先级",0, AfxScaleX(70)
      This.lvwThread.InsertColumn 2, "线程状态",0, AfxScaleX(100)
      This.lvwThread.InsertColumn 3, "线程入口",0, AfxScaleX(110)
      This.lvwThread.InsertColumn 4, "线程大小",0, AfxScaleX(70)
      This.lvwThread.InsertColumn 5, "线程所属模块",0, AfxScaleX(250)
      SNDMSG(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FLATSB, LVS_EX_FLATSB)
      SNDMSG(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT)
      SNDMSG(hWndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES)
      This.lvwThread.ForeColor =GetCodeColorGDI(&H087F7F7F,GetSysColor(COLOR_BTNTEXT))
      fp->ForeColor =&H087F7F7F
      This.lvwThread.BackColor =GetCodeColorGDI(&H057F7F7F,GetSysColor(COLOR_BTNFACE))
      fp->BackColor =&H057F7F7F
      This.lvwThread.Font = "Microsoft YaHei,9,0"
      fp->nLeft = 0
      fp->nTop = 0
      fp->nWidth = 410
      fp->nHeight = 260
   End IF

'[Create control]  创建控件的代码，在此前一行插入。

'[Create control end]  创建全部控件后需要处理的代码，在此前一行插入。


   PostMessage(NewhWnd, &H1000, &H1000,CAst(lPArAm,UserData)) '触发显示事件
   
   hWndControl = NewhWnd '设置焦点后会发生消息循环的情况，要是同时多开同一个窗口，就发生 hWnd 被赋值为下一个窗口句柄的情况。
   This.lvwThread.SetFocus
'[SET_FOCUS] '将焦点设置为选项卡顺序中的最低控件 
   Function = FLY_DoMessagePump(pWindow,Modal, hWndControl, hWndParent, SW_SHOWNORMAL, False) ' IsMDIForm 如果这是MDI窗口，请设置标志
End Function

Function FrmThread_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult 'FrmThread 窗口回调过程
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
            
'[FORM_SHOWN]  主窗口第一次显示事件，在此前一行插入。
            InvalidateRect(hWndForm, Null, True)
            UpdateWindow(hWndForm)
            '[Create window shadow]  创建窗口阴影，在此前一行插入。
            Return 0
         End If
      Case WM_COMMAND
         
'[FORM_WM_COMMAND]  主窗口命令处理事件，在此前一行插入。
         '{CONTROL_WM_COMMAND}  '控件处理初始化代码
         '[CONTROL_WM_COMMAND]  控件处理事件，在此前一行插入。
      Case WM_HSCROLL
         
'[FORM_WM_HSCROLL]  主窗口和一些特殊控件要的事件，在此前一行插入。
      Case WM_VSCROLL
         
'[FORM_WM_VSCROLL]  主窗口和一些特殊控件要的事件，在此前一行插入。
         
         















'[Other events in main window]  主窗口其它事件，需要加 Case ，在此前一行插入。 注意每个消息值不可重复，不然后面的无法执行
         '{FORM_WM_DRAWITEM}
         '[FORM_WM_DRAWITEM]
      Case WM_TIMER
         If wParam = 357897 then '虚拟控件要刷新，使用防抖动方式刷新，避免虚拟控件多时，同时设置控件属性，造成大量刷新变卡顿。
            killTimer hWndForm, 357897
            AfxRedrawWindow hWndForm
         end if
         '[FORM_WM_TIMER]  时间控件事件
      Case WM_NOTIFY
         Dim FLY_pNotify As NMHDR Ptr = Cast(NMHDR Ptr, lParam)
                  If (FLY_pNotify->idFrom = 102) And (FLY_pNotify->Code = LVN_COLUMNCLICK) Then
             FrmThread.lvwThread.hWndForm = hWndForm
             FrmThread.lvwThread.ColumnToSort(Cast(Any Ptr, lParam))
         End If
'[CONTROLS_NOTIFY]  控件事件，标准控件的事件都是按照这个传递
         
'[FORM_WM_NOTIFY]   主窗口的事件
   End Select
   Select Case wMsg
      Case WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK, WM_MOUSEMOVE, WM_CONTEXTMENU, WM_MOUSEWHEEL, WM_Timer
         Dim As Long xPos = GET_X_LPARAM(lParam), yPos = GET_Y_LPARAM(lParam)
         '[VIRTUAL_CONTROL_EVENTS] 处理虚拟控件事件，在此前一行插入。
         
   End Select
   
   
   
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
         
'[CALL_FORM_WM_DESTROY]  窗口卸载事件
         
         '[DELETE_USER_TIMERS] 删除任何用户定义的Timer控件
         
         FrmThread.hWnd = 0
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

Function FrmThread_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT  ' FrmThread 控件回调
   'FrmThread 窗口上每个控件的所有消息都在此函数中处理。
   Dim fp As FormControlsPro_TYPE ptr 
   Dim tLResult As LResult
   Dim IDC As Long = GetDlgCtrlId(hWndControl)

'[CONTROL_LEAVEHOVER]  '启用鼠标出窗口检查

'[CALL_CONTROL_CUSTOM]  处理任何自定义消息。

   '以下CASE在处理用户定义事件之前处理内部VisualFreeBasic要求（这些事件在此之后的单独CASE中处理）。
   Select Case wMsg
      Case 0  '需要这一行，万一没控件事件，语句也能成立

         
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





