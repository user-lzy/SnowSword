'控件类
#include Once "..\mCtrlPublic\treelist.bi"
Type Class_mCtrlTreeList Extends Class_Control
Protected : 
   m_ReDraw As Boolean  '修改列表时是否允许刷新
   Declare Sub CollapseAllChildItems(hNode As MC_HTREELISTITEM)
   Declare Sub ExpandAllChildItems(hNode As MC_HTREELISTITEM)
Public :    
    Declare Function InsertColumn(Col As Long ,zText As Wstring ,nWidth As Long  ,Alignment As Long =MC_TLFMT_LEFT,iImage As Long =0  ) As Long    '插入新列(从0开始)  返回新列的索引(从0开始)，否则返回-1失败。参数：插入位置，文本，像素宽度，对齐，图像列表控件索引 {4.MC_TLFMT_LEFT 文本向左对齐.MC_TLFMT_CENTER 文本居中.MC_TLFMT_RIGHT 文本右对齐}
    Declare Function AddColumn(zText As Wstring ,nWidth As Long  ,Alignment As Long =MC_TLFMT_LEFT,iImage As Long =0  ) As Long    '插入新列(从0开始)  返回新列的索引(从0开始)，否则返回-1失败。参数：插入位置，文本，像素宽度，对齐，图像列表控件索引 {3.MC_TLFMT_LEFT 文本向左对齐.MC_TLFMT_CENTER 文本居中.MC_TLFMT_RIGHT 文本右对齐}
    Declare Function SetColumn(Col As Long ,zText As Wstring ,nWidth As Long  ,Alignment As Long =MC_TLFMT_LEFT,iImage As Long =0 ) As Long    '设置列的属性(从0开始)  （BOOL）TRUE成功，FALSE否则。。参数：位置，文本，像素宽度，对齐，图像列表控件索引 {4.MC_TLFMT_LEFT 文本向左对齐.MC_TLFMT_CENTER 文本居中.MC_TLFMT_RIGHT 文本右对齐}
    Declare Function SetColumnWidth(Col As Long ,nWidth As Long ) As Long    '更改列的宽度(从0开始)  （BOOL）TRUE成功，FALSE否则。
    Declare Function GetColumnCount() As Long  '获取控件的表中的列数。
    Declare Function GetColumnText(Col As Long) As StringW    '获取列的属性 列(从0开始)
    Declare Function GetColumnWidth(Col As Long) As Long     '获取列的属性 列(从0开始)
    Declare Function GetColumnAlignment(Col As Long) As Long     '获取列的属性 列(从0开始)
    Declare Function GetColumnImaget(Col As Long) As Long     '获取列的属性 列(从0开始)
    Declare Function GetColumnOrder(Col As Long) As Long     '获取列的属性 列(从0开始)
    Declare Function DeleteColumn(Col As Long) As Long     '从控件中删除一列。 列(从0开始)（BOOL）TRUE成功，FALSE否则。
    Declare Function SetColumnOrderArray(iOrder() As Integer) As Long     '设置列的从左到右排序。排序用，列0是树状层次显示用，永远是0， 列(从0开始)（BOOL）TRUE成功，FALSE否则。
    Declare Function GetColumnOrderArray(iOrder() As Integer) As Long     '获取列的从左到右排序。排序用，列0是树状层次显示用，永远是0  列(从0开始) 返回数组个数，=0 为失败
    Declare Function InsertItem(hParent As MC_HTREELISTITEM,hInsertAfter As MC_HTREELISTITEM,zText As Wstring ,UserData As Integer =0,iImage As Long =0,iSelectedImage As Long =0,iExpandedImage As Long=0,cChildren As Long =0 ) As MC_HTREELISTITEM     '插入将新项。 返回新项目句柄，失败为 NULL 参数：父项，插入位置项，文本，用户数据，正常图像索引，选择图像，展开图像,是否父项 （父：MC_TLI_ROOT 根项，插入：第一个 MC_TLI_FIRST 或 最后一个 MC_TLI_LAST）  
    Declare Function AddItem(hParent As MC_HTREELISTITEM,zText As Wstring ,UserData As Integer =0,iImage As Long =0,iSelectedImage As Long=0 ,iExpandedImage As Long=0,cChildren As Long =0  ) As MC_HTREELISTITEM     '插入将新项。 返回新项目句柄，失败为 NULL 参数：父项，文本，用户数据，正常图像索引，选择图像，展开图像,是否父项 （父：MC_TLI_ROOT 根项）  
    
    Declare Function SetItem(hItem As MC_HTREELISTITEM,zText As Wstring ,UserData As Integer =0,iImage As Long =0,iSelectedImage As Long=0 ,iExpandedImage As Long=0,cChildren As Long =0  ) As Long      '设置项目。 （BOOL）TRUE成功，FALSE否则。 参数：项，文本，用户数据，正常图像索引，选择图像，展开图像,是否父项   
    Declare Function GetItem(hItem As MC_HTREELISTITEM) As MC_TLITEMW      '获取项目。
    Declare Function GetItemText(hItem As MC_HTREELISTITEM, iColumn As Long) As StringW '返回列表项的文本(索引从0开始)(支持宽字符的火星文)
    Declare Function SetItemText(hItem As MC_HTREELISTITEM, iColumn As Long, TheText As Wstring) As Long  '设置列表项的文本(索引从0开始)(支持宽字符的火星文)（BOOL）TRUE成功，FALSE否则
    Declare Function SetItemImage(hItem As MC_HTREELISTITEM, iImage As Long ,iSelectedImage As Long,iExpandedImage As Long) As Long  '设置列表项的正常图像索引，选择图像，展开图像（BOOL）TRUE成功，FALSE否则
    Declare Function SetItemChildren(hItem As MC_HTREELISTITEM, cChildren As Long) As Long  '设置 是否父项 （BOOL）TRUE成功，FALSE否则
    
    Declare Function GetChildCount(hItem As MC_HTREELISTITEM) As Long     '获取子级别项目数。用 MC_TLI_ROOT 获取根项目数
     
    Declare Function DeleteItem(hItem As MC_HTREELISTITEM) As Long   '删除项目 （BOOL）TRUE成功，FALSE否则。请注意，会删除所有子项，即删除整个子树。如果指定 MC_TLI_ROOT 所有项目都将被删除。
    Declare Function HitTest(x As Long ,y As Long ,flags As UInteger =MC_TLHT_ONITEM ) As MC_HTREELISTITEM   '返回指定位置的项句柄，没找到为 NULL 
    Declare Function Expand(hItem As MC_HTREELISTITEM,flags As ULong ,KillChild As Long =0) As Long  '展开子项 KillChild<>0删除其所有子项目 （BOOL）TRUE成功，FALSE否则。{2.MC_TLE_COLLAPSE 折叠子项.MC_TLE_EXPAND 展开子项.MC_TLE_TOGGLE 若展开则折，若折叠则展开}
    Declare Function GetNextItem(hItem As MC_HTREELISTITEM ,flags As ULong) As MC_HTREELISTITEM   '获取与指定项目的指定关系的项目。没找到为 NULL {2.MC_TLGN_ROOT 根项目.MC_TLGN_NEXT 下一个同级项目.MC_TLGN_PREVIOUS 上一个同级项目.MC_TLGN_PARENT 父项目.MC_TLGN_CHILD 第一个子项目.MC_TLGN_FIRSTVISIBLE 第一个可见项目.MC_TLGN_NEXTVISIBLE 下一个可见项.MC_TLGN_PREVIOUSVISIBLE 上一个可见项目.MC_TLGN_CARET 下一个选定项.MC_TLGN_LASTVISIBLE 最后可见项目}                   
    Declare Function GetVisibleCount() As Long  '获取当前客户区域的可以显示项目数量。
    Declare Function EnsureVisible(hItem As MC_HTREELISTITEM) As Long  '确保该项目可见。（BOOL）TRUE如果控件滚动了这些项目，但没有展开任何项目，FALSE否则。
    Declare Function GetSelectedCount() As Long  '返回当前所选项目的数量。
    Declare Function GetItemRect(hItem As MC_HTREELISTITEM,flags As ULong =MC_TLIR_BOUNDS  ) As RECT   '获取给定项目的矩形。{2.MC_TLIR_BOUNDS 整个项目.MC_TLIR_ICON 图标矩形.MC_TLIR_LABEL 标签矩形.MC_TLIR_SELECTBOUNDS 图标和标签}
    Declare Function GetSubItemRect(hItem As MC_HTREELISTITEM,iSubItem As Long ,flags As ULong =MC_TLIR_BOUNDS  ) As RECT   '获取给定子项(列表的第几列)的矩形。{2.MC_TLIR_BOUNDS 整个项目.MC_TLIR_LABEL 标签矩形}
    Declare Function GetToolTips() As .hWnd   '获取与控件关联的工具提示窗口。没有返回null 
    Declare Function GetRoot() As MC_HTREELISTITEM           '返回最顶层的第一个项目，成功则返回该项目的句柄，否则返回NULL。
        
    Declare Property ItemHeight() As Long      '返回/设置项目高度 以像素为单位
    Declare Property ItemHeight(v As Long)
    Declare Property Indent() As Long      '返回/设置项目缩进。 以像素为单位
    Declare Property Indent(v As Long)
    Declare Property ImageList() As HIMAGELIST      '返回/设置与控件关联 图像列表控件句柄
    Declare Property ImageList(v As HIMAGELIST)
   Declare Property ReDraw() As Boolean         '设置重绘标志，大量新增项目时关闭重绘，避免控件闪耀和提高速度。{=.True.False}
   Declare Property ReDraw(nValue As Boolean)    '修改项目时是否允许刷新，（注意：多开窗口中的控件，多窗口用同1个控件类，返回此值可能不正确）

   Declare Property Selection() As MC_HTREELISTITEM                 '返回/设置选择指定的项，并将视图滚动到项。
   Declare Property Selection(ByVal hItem As MC_HTREELISTITEM)
   Declare Property Checked(hItem As MC_HTREELISTITEM) As Boolean   '返回/设置项目“已选中”或“未选中”。{=.True.False}
   Declare Property Checked(hItem As MC_HTREELISTITEM, fCheck As Boolean)
   Declare Sub CollapseAllItems() '折叠所有项目列表。
   Declare Sub ExpandAllItems() '展开所有项目列表。   
   Declare Property ItemData(hItem As MC_HTREELISTITEM) As Integer                 '返回/设置指定项的 附加数据 lParam
   Declare Property ItemData(hItem As MC_HTREELISTITEM, newlParam As Integer)
     
End Type

'----------------------------------------------------------------------------------------------------------------------------------------------------------------

Function Class_mCtrlTreeList.InsertColumn(Col As Long ,zText As WString,nWidth As Long  ,Alignment As Long =MC_TLFMT_LEFT,iImage As Long =0 ) As Long 
   Dim pp As MC_TLCOLUMNW
    pp.fMask = MC_TLCF_FORMAT Or MC_TLCF_WIDTH Or MC_TLCF_TEXT Or MC_TLCF_IMAGE ' Or MC_TLCF_ORDER
    pp.fmt   = Alignment
    pp.cx    = nWidth
    pp.pszText =@zText
    pp.cchTextMax =Len(zText)
    pp.iImage = iImage 
'    pp.iOrder = iOrder 
   Function = SendMessage(hWndControl, MC_TLM_INSERTCOLUMNW, Col, Cast(lParam, @pp))
End Function
Function Class_mCtrlTreeList.AddColumn(zText As WString, nWidth As Long, Alignment As Long = MC_TLFMT_LEFT, iImage As Long = 0) As Long 
   Dim i As Long = This.GetColumnCount()
   Function = This.InsertColumn(i, zText, nWidth, Alignment, iImage)
End Function


Function Class_mCtrlTreeList.SetColumn(Col As Long, zText As WString, nWidth As Long, Alignment As Long = MC_TLFMT_LEFT, iImage As Long = 0) As Long
   Dim pp As MC_TLCOLUMNW
   pp.fMask      = MC_TLCF_FORMAT Or MC_TLCF_WIDTH Or MC_TLCF_TEXT Or MC_TLCF_IMAGE 'Or MC_TLCF_ORDER
   pp.fmt        = Alignment
   pp.cx         = nWidth
   pp.pszText    = @zText
   pp.cchTextMax = Len(zText)
   pp.iImage     = iImage
   '    pp.iOrder = iOrder
   Function = SendMessage(hWndControl, MC_TLM_SETCOLUMNW, Col, Cast(lParam, @pp))
End Function

Function Class_mCtrlTreeList.GetColumnText(Col As Long) As StringW
   Dim pp As MC_TLCOLUMNW
   pp.fMask = MC_TLCF_TEXT
   if SendMessage(hWndControl, MC_TLM_GETCOLUMNW, Col, Cast(lParam, @pp)) Then
      Return .Left( *pp.pszText, pp.cchTextMax)
   End if
End Function
Function Class_mCtrlTreeList.GetColumnCount() As Long 
   Dim hWndLVHdr As .hWnd = FindWindowEx(hWndControl, 0, "SysHeader32", NULL) 
   if hWndLVHdr Then 
      Function = SendMessage(hWndLVHdr, HDM_GETITEMCOUNT, 0, 0)
   end if 
End Function
Function Class_mCtrlTreeList.GetColumnWidth(Col As Long) As Long
    Function = SendMessage(hWndControl, MC_TLM_GETCOLUMNWIDTH, Col, 0) 
End Function

Function Class_mCtrlTreeList.GetColumnAlignment(Col As Long) As Long
   Dim pp As MC_TLCOLUMNW
    pp.fMask = MC_TLCF_FORMAT 
    if SendMessage(hWndControl, MC_TLM_GETCOLUMNW, Col, Cast(lParam, @pp)) Then 
        Return pp.fmt
    End if  
End Function

Function Class_mCtrlTreeList.GetColumnImaget(Col As Long) As Long
   Dim pp As MC_TLCOLUMNW
    pp.fMask = MC_TLCF_IMAGE 
    if SendMessage(hWndControl, MC_TLM_GETCOLUMNW, Col, Cast(lParam, @pp)) Then 
        Return pp.iImage
    End if  
End Function

Function Class_mCtrlTreeList.GetColumnOrder(Col As Long) As Long
   Dim pp As MC_TLCOLUMNW
    pp.fMask = MC_TLCF_ORDER 
    if SendMessage(hWndControl, MC_TLM_GETCOLUMNW, Col, Cast(lParam, @pp)) Then 
        Return pp.iOrder
    End if  
End Function

Function Class_mCtrlTreeList.DeleteColumn(Col As Long) As Long 
    Function = SendMessage(hWndControl, MC_TLM_DELETECOLUMN, Col, 0) 
End Function

Function Class_mCtrlTreeList.SetColumnOrderArray(iOrder() As Integer) As Long 
   Dim u As Long = UBound(iOrder)
   if u=-1 Then Return 0
    Function = SendMessage(hWndControl, MC_TLM_SETCOLUMNORDERARRAY, u*SizeOf(Integer), Cast(lParam, @iOrder(LBound(iOrder) ) ) ) 
End Function

Function Class_mCtrlTreeList.GetColumnOrderArray(iOrder() As Integer) As Long
   Dim u As Long = 1000
   ReDim iOrder (999)
   u = SendMessage(hWndControl, MC_TLM_GETCOLUMNORDERARRAY, u *SizeOf(Integer), Cast(lParam, @iOrder(0))) 
   if u = 0 Then Erase iOrder Else ReDim Preserve iOrder(u -1)
   Function = u  
End Function

Function Class_mCtrlTreeList.SetColumnWidth(Col As Long ,nWidth As Long ) As Long 
    Function = SendMessage(hWndControl, MC_TLM_SETCOLUMNWIDTH, Col, nWidth ) 
End Function

Function Class_mCtrlTreeList.InsertItem(hParent As MC_HTREELISTITEM, hInsertAfter As MC_HTREELISTITEM, zText As WString, UserData As Integer = 0, iImage As Long = 0, iSelectedImage As Long = 0, iExpandedImage As Long = 0, cChildren As Long = 0) As MC_HTREELISTITEM
   Dim pp As MC_TLINSERTSTRUCTW
   pp.hParent         = hParent
   pp.hInsertAfter    = hInsertAfter
   pp.item.fMask      = MC_TLIF_TEXT Or MC_TLIF_PARAM Or MC_TLIF_IMAGE Or MC_TLIF_SELECTEDIMAGE Or MC_TLIF_EXPANDEDIMAGE Or MC_TLIF_CHILDREN
   pp.item.pszText    = @zText
   pp.item.cchTextMax = Len(zText)
   '   pp.item.state = MC_TLIS_SELECTED   '选择
   pp.item.stateMask      = 0 '确定的state有效位
   pp.item.lParam         = UserData
   pp.item.iImage         = iImage
   pp.item.iSelectedImage = iSelectedImage
   pp.item.iExpandedImage = iExpandedImage
   if cChildren Then cChildren = 1
   pp.item.cChildren = cChildren
   
   Function = Cast(MC_HTREELISTITEM, SendMessage(hWndControl, MC_TLM_INSERTITEMW, 0, Cast(lParam, @pp)))
End Function
Function Class_mCtrlTreeList.AddItem(hParent As MC_HTREELISTITEM,zText As WString ,UserData As Integer =0,iImage As Long =0,iSelectedImage As Long=0 ,iExpandedImage As Long=0 ,cChildren As Long =0 ) As MC_HTREELISTITEM 
   Function = This.InsertItem(hParent,MC_TLI_LAST, zText,UserData,iImage,iSelectedImage,iExpandedImage,cChildren  )
End Function

Function Class_mCtrlTreeList.SetItem(hItem As MC_HTREELISTITEM, zText As WString, UserData As Integer = 0, iImage As Long = 0, iSelectedImage As Long = 0, iExpandedImage As Long = 0, cChildren As Long = 0) As Long
   Dim pp As MC_TLITEMW
   if hItem = 0 Then Return 0
   pp.fMask          = MC_TLIF_TEXT Or MC_TLIF_PARAM Or MC_TLIF_IMAGE Or MC_TLIF_SELECTEDIMAGE Or MC_TLIF_EXPANDEDIMAGE Or MC_TLIF_CHILDREN
   pp.pszText        = @zText
   pp.cchTextMax     = Len(zText)
   pp.stateMask      = 0 '确定的state有效位
   pp.lParam         = UserData
   pp.iImage         = iImage
   pp.iSelectedImage = iSelectedImage
   pp.iExpandedImage = iExpandedImage
   if cChildren Then cChildren = 1
   pp.cChildren = cChildren
   Function     = SendMessage(hWndControl, MC_TLM_SETITEMW, Cast(wParam, hItem), Cast(lParam, @pp))
End Function
Function Class_mCtrlTreeList.SetItemChildren(hItem As MC_HTREELISTITEM, cChildren As Long) As Long
   Dim pp As MC_TLITEMW
   if hItem =0 Then Return 0
   pp.fMask =  MC_TLIF_CHILDREN
   if cChildren Then cChildren =1
   pp.cChildren = cChildren
   Function = SendMessage(hWndControl, MC_TLM_SETITEMW,Cast(wParam,hItem) , Cast(lParam, @pp) ) 
End Function

Function Class_mCtrlTreeList.SetItemImage(hItem As MC_HTREELISTITEM ,iImage As Long ,iSelectedImage As Long ,iExpandedImage As Long) As Long
   Dim pp As MC_TLITEMW
   if hItem = 0 Then Return 0
   pp.fMask          = MC_TLIF_IMAGE Or MC_TLIF_SELECTEDIMAGE Or MC_TLIF_EXPANDEDIMAGE
   pp.iImage         = iImage
   pp.iSelectedImage = iSelectedImage
   pp.iExpandedImage = iExpandedImage
   Function     = SendMessage(hWndControl ,MC_TLM_SETITEMW ,Cast(wParam ,hItem) ,Cast(lParam ,@pp))
End Function
Function Class_mCtrlTreeList.GetItem(hItem As MC_HTREELISTITEM) As MC_TLITEMW
   Dim pp As MC_TLITEMW
   if hItem=0 Then Return pp
   Static tt As WString * 261
   memset(@tt ,0, 261*2)
   pp.fMask = MC_TLIF_TEXT Or MC_TLIF_PARAM Or MC_TLIF_IMAGE Or MC_TLIF_SELECTEDIMAGE Or MC_TLIF_EXPANDEDIMAGE 
   pp.pszText = @tt
   pp.cchTextMax = 260    
   If SendMessage(hWndControl, MC_TLM_GETITEMW,Cast(wParam,hItem), Cast(lParam, @pp) ) Then 
      Return pp
   end if 
End Function
Function Class_mCtrlTreeList.GetItemText(hItem As MC_HTREELISTITEM, iColumn As Long) As StringW
   if hItem = 0 Then Return ""
   if iColumn < 1 Then
      Dim pp As MC_TLITEMW
      Dim tt As WString * 261
      pp.fMask      = MC_TLIF_TEXT
      pp.pszText    = @tt
      pp.cchTextMax = 260
      if SendMessage(hWndControl, MC_TLM_GETITEMW, Cast(wParam, hItem), Cast(lParam, @pp)) Then
         Return tt
      End If
   Else
      Dim pp As MC_TLSUBITEMW
      Dim tt As WString * 261
      pp.fMask      = MC_TLSIF_TEXT
      pp.iSubItem   = iColumn
      pp.pszText    = @tt
      pp.cchTextMax = 260
      if SendMessage(hWndControl, MC_TLM_GETSUBITEMW, Cast(wParam, hItem), Cast(lParam, @pp)) Then
         Return tt
      end if
   End if
End Function
Function Class_mCtrlTreeList.SetItemText(hItem As MC_HTREELISTITEM, iColumn As Long, TheText As WString) As Long
   if hItem = 0 Then Return 0
   if iColumn < 1 Then
      Dim pp As MC_TLITEMW
      pp.fMask      = MC_TLIF_TEXT
      pp.pszText    = @TheText
      pp.cchTextMax = Len(TheText)
      Function      = SendMessage(hWndControl, MC_TLM_SETITEMW, Cast(wParam, hItem), Cast(lParam, @pp))
   Else
      Dim pp As MC_TLSUBITEMW
      pp.fMask      = MC_TLSIF_TEXT
      pp.iSubItem   = iColumn
      pp.pszText    = @TheText
      pp.cchTextMax = Len(TheText)
      Function      = SendMessage(hWndControl, MC_TLM_SETSUBITEMW, Cast(wParam, hItem), Cast(lParam, @pp))
      
   End if
End Function

Function Class_mCtrlTreeList.GetChildCount(hItem As MC_HTREELISTITEM) As Long 
    Dim hChild As MC_HTREELISTITEM
    Dim nCount As Long
    '检查窗口句柄是否有效
    If IsWindow(hWndControl) Then  
       hChild = This.GetNextItem( hItem, IIf(hItem=0,MC_TLGN_ROOT,MC_TLGN_CHILD) )
        While hChild <> 0
          nCount += 1
          hChild = This.GetNextItem( hChild, MC_TLGN_NEXT )
        Wend
        Return  nCount
    End If
End Function

Function Class_mCtrlTreeList.DeleteItem(hItem As MC_HTREELISTITEM) As Long

   Function = SendMessage(hWndControl, MC_TLM_DELETEITEM,0, Cast(lParam,hItem ) ) 
   
End Function

Property Class_mCtrlTreeList.ItemHeight(v As Long)

   SendMessage(hWndControl, MC_TLM_SETITEMHEIGHT,v, 0 ) 
   
End Property

Property Class_mCtrlTreeList.ItemHeight() As Long

   Property = SendMessage(hWndControl, MC_TLM_GETITEMHEIGHT,0, 0 ) 
   
End Property

Property Class_mCtrlTreeList.Indent(v As Long)

   SendMessage(hWndControl, MC_TLM_SETINDENT,v, 0 ) 
   
End Property

Property Class_mCtrlTreeList.Indent() As Long

   Property = SendMessage(hWndControl, MC_TLM_GETINDENT,0, 0 ) 
   
End Property

Property Class_mCtrlTreeList.ImageList() As HIMAGELIST 
   
  Property = Cast(HIMAGELIST, SendMessage(hWndControl, MC_TLM_GETIMAGELIST,0, 0 ) )
   
End Property

Property Class_mCtrlTreeList.ImageList(v As HIMAGELIST)

   SendMessage(hWndControl, MC_TLM_SETIMAGELIST,0, Cast(lParam, v) ) 
   
End Property



Function Class_mCtrlTreeList.HitTest(x As Long ,y As Long ,flags As UInteger =MC_TLHT_ONITEM ) As MC_HTREELISTITEM 
   Dim pp As MC_TLHITTESTINFO

   Function = Cast(MC_HTREELISTITEM, SendMessage(hWndControl, MC_TLM_HITTEST,0, Cast(lParam, @pp) ) )
End Function
Function Class_mCtrlTreeList.Expand(hItem As MC_HTREELISTITEM, flags As ULong, KillChild As Long = 0) As Long
   if KillChild Then flags Or=MC_TLE_COLLAPSERESET 
   Function = SendMessage(hWndControl, MC_TLM_EXPAND ,flags, Cast(lParam, hItem) )
End Function
Function Class_mCtrlTreeList.GetNextItem(hItem As MC_HTREELISTITEM, flags As ULong) As MC_HTREELISTITEM 
   Function =Cast(MC_HTREELISTITEM, SendMessage(hWndControl, MC_TLM_GETNEXTITEM  ,flags, Cast(lParam, hItem) ) )
End Function

Function Class_mCtrlTreeList.GetVisibleCount() As Long
   Function = SendMessage(hWndControl, MC_TLM_GETVISIBLECOUNT ,0, 0) 
End Function
Function Class_mCtrlTreeList.EnsureVisible(hItem As MC_HTREELISTITEM) As Long 
   if hItem =0 Then Return FALSE
   Function = SendMessage(hWndControl, MC_TLM_ENSUREVISIBLE ,0, Cast(lParam, hItem)) 
End Function

Function Class_mCtrlTreeList.GetSelectedCount() As Long 
   Function = SendMessage(hWndControl, MC_TLM_GETSELECTEDCOUNT ,0, 0) 
End Function

Function Class_mCtrlTreeList.GetItemRect(hItem As MC_HTREELISTITEM, flags As ULong = MC_TLIR_BOUNDS) As RECT
   Dim pp As RECT 
   if hItem=0 Then Return pp
   pp.left = flags
   if SendMessage(hWndControl, MC_TLM_GETITEMRECT, Cast(wParam, hItem), Cast(lParam, @pp)) =0 Then pp.left=0
   Return pp
End Function

Function Class_mCtrlTreeList.GetSubItemRect(hItem As MC_HTREELISTITEM,iSubItem As Long ,flags As ULong =MC_TLIR_BOUNDS  ) As RECT
   Dim pp As RECT 
   pp.left = flags
   pp.top = iSubItem
   if SendMessage(hWndControl, MC_TLM_GETSUBITEMRECT, Cast(wParam, hItem), Cast(lParam, @pp)) = 0 Then 
      pp.left = 0
      pp.top = 0
   End if 
   Return pp
End Function
Function Class_mCtrlTreeList.GetToolTips() As .HWND 

   Function =Cast(.HWND , SendMessage(hWndControl, MC_TLM_GETTOOLTIPS, 0, 0) )

End Function

Property Class_mCtrlTreeList.ReDraw()  As Boolean         '返回/设置重绘标志，大量新增项目时关闭重绘，避免控件闪耀和提高速度。{=.True.False}
    Property = m_ReDraw  '无需返回
End Property
Property Class_mCtrlTreeList.ReDraw(nValue As Boolean)
 m_ReDraw = nValue
 SendMessage(hWndControl, WM_SETREDRAW, nValue, 0)
End Property

Property Class_mCtrlTreeList.Selection() As MC_HTREELISTITEM                 '返回/设置选择指定的项，并将视图滚动到项。
   Property = This.GetNextItem( MC_TLI_ROOT,MC_TLGN_CARET )
End Property
Property Class_mCtrlTreeList.Selection(ByVal hItem As MC_HTREELISTITEM)
   This.Checked(hItem) = TRUE
   This.EnsureVisible(hItem)
End Property

Property Class_mCtrlTreeList.Checked(hItem As MC_HTREELISTITEM) As Boolean    
   Dim pp As MC_TLITEMW
   if hItem Then
      pp.fMask = MC_TLIF_STATE
      if SendMessage(hWndControl, MC_TLM_GETITEMW, Cast(wParam, hItem), Cast(lParam, @pp)) Then 
         Return  (pp.state And MC_TLIS_SELECTED)=MC_TLIS_SELECTED
      End if 
   End if
End Property
Property Class_mCtrlTreeList.Checked(hItem As MC_HTREELISTITEM, fCheck As Boolean)
   Dim pp As MC_TLITEMW
   if hItem Then
      pp.fMask = MC_TLIF_STATE
      pp.stateMask = MC_TLIS_SELECTED  '确定的state有效位
      pp.state = IIf(fCheck,MC_TLIS_SELECTED,0)
      SendMessage(hWndControl, MC_TLM_SETITEMW, Cast(wParam, hItem), Cast(lParam, @pp))
   End if
End Property

Sub Class_mCtrlTreeList.CollapseAllItems() '折叠所有项目列表。 
   if IsWindow(hWndControl) = 0 Then Return 
   DIM hNode AS MC_HTREELISTITEM = This.GetNextItem(MC_TLI_ROOT, MC_TLGN_ROOT)
   IF hNode THEN This.CollapseAllChildItems(hNode)   
End Sub

Sub Class_mCtrlTreeList.ExpandAllItems() '展开所有项目列表。
   if IsWindow(hWndControl) = 0 Then Return 
   DIM hNode AS MC_HTREELISTITEM = This.GetNextItem(MC_TLI_ROOT, MC_TLGN_ROOT)
   IF hNode THEN This.ExpandAllChildItems(hNode)    
End Sub

SUB Class_mCtrlTreeList.CollapseAllChildItems(BYVAL hNode AS MC_HTREELISTITEM)
   DIM hChildNode AS MC_HTREELISTITEM
   DO WHILE hNode
      This.Expand(hNode,MC_TLE_COLLAPSE)
      hChildNode = This.GetNextItem( hNode,MC_TLGN_CHILD)
      IF hChildNode THEN This.CollapseAllChildItems(hChildNode)
      hNode = This.GetNextItem( hNode,MC_TLGN_NEXT)
   LOOP
END SUB

SUB Class_mCtrlTreeList.ExpandAllChildItems (BYVAL hNode AS MC_HTREELISTITEM)
   DIM hChildNode AS MC_HTREELISTITEM
   DO WHILE hNode
      This.Expand(hNode,MC_TLE_EXPAND)
      hChildNode = This.GetNextItem( hNode,MC_TLGN_CHILD)
      IF hChildNode THEN This.ExpandAllChildItems(hChildNode)
      hNode = This.GetNextItem( hNode,MC_TLGN_NEXT)
   LOOP
END SUB

Function Class_mCtrlTreeList.GetRoot() As MC_HTREELISTITEM 
   Function = This.GetNextItem(MC_TLI_ROOT, MC_TLGN_ROOT)
End Function

Property Class_mCtrlTreeList.ItemData(hItem As MC_HTREELISTITEM) As Integer
   if hItem = 0 Then Return 0
   Dim pp As MC_TLITEMW
   pp.fMask =  MC_TLIF_PARAM 
   if SendMessage(hWndControl, MC_TLM_GETITEMW, Cast(wParam, hItem), Cast(lParam, @pp)) Then
      Return Cast(Integer ,pp.lParam )
   end if
End Property
Property Class_mCtrlTreeList.ItemData(hItem As MC_HTREELISTITEM, newlParam As Integer)
   Dim pp As MC_TLITEMW
   if hItem =0 Then Return 
   pp.fMask =  MC_TLIF_PARAM 
   pp.lParam = Cast(LPARAM,newlParam)
   SendMessage(hWndControl, MC_TLM_SETITEMW,Cast(wParam,hItem) , Cast(lParam, @pp) ) 
End Property









