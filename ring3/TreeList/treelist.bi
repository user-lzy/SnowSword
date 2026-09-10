


#include Once "_common.bi"

Extern "C"     
    Declare Function mcTreeList_Initialize MCTRL_API  Lib "mCtrl" Alias "mcTreeList_Initialize"() As  BOOL
    Declare Sub mcTreeList_Terminate MCTRL_API  Lib "mCtrl" Alias "mcTreeList_Terminate"()
     
End Extern
 mcTreeList_Initialize

#define MC_TLS_HASBUTTONS           &H0001
#define MC_TLS_HASLINES             &H0002
#define MC_TLS_LINESATROOT          &H0004
#define MC_TLS_GRIDLINES            &H0008
#define MC_TLS_SHOWSELALWAYS        &H0010
#define MC_TLS_FULLROWSELECT        &H0020
#define MC_TLS_NONEVENHEIGHT        &H0040
#define MC_TLS_DOUBLEBUFFER         &H0080
#define MC_TLS_NOCOLUMNHEADER       &H0100
#define MC_TLS_HEADERDRAGDROP       &H0200
#define MC_TLS_SINGLEEXPAND         &H0400
#define MC_TLS_MULTISELECT          &H0800
#define MC_TLS_NOTOOLTIPS           &H1000

#if 0 
#define MC_TLS_CHECKBOXES           &H2000
#define MC_TLS_EDITLABELS           &H4000
#define MC_TLS_EDITSUBLABELS        &H8000
#endif

#define MC_TLCF_FORMAT          (1 Shl 0)
#define MC_TLCF_WIDTH           (1 Shl 1)
#define MC_TLCF_TEXT            (1 Shl 2)
#define MC_TLCF_IMAGE           (1 Shl 3)
#define MC_TLCF_ORDER           (1 Shl 4)

#define MC_TLFMT_LEFT             &H0
#define MC_TLFMT_RIGHT            &H1
#define MC_TLFMT_CENTER           &H2
#define MC_TLFMT_JUSTIFYMASK      &H3

#define MC_TLI_ROOT  (Cast(MC_HTREELISTITEM,Cast(ULONG_PTR, -&H10000)))

#define MC_TLI_FIRST (Cast(MC_HTREELISTITEM,Cast(ULONG_PTR, -&Hffff))) 

#define MC_TLI_LAST  (Cast(MC_HTREELISTITEM,Cast(ULONG_PTR, -&Hfffe))) 

#define MC_TLIF_STATE                (1 Shl 0)
#define MC_TLIF_TEXT                 (1 Shl 1)
#define MC_TLIF_PARAM                (1 Shl 2)
#define MC_TLIF_LPARAM               (1 Shl 2)
#define MC_TLIF_IMAGE                (1 Shl 3)
#define MC_TLIF_SELECTEDIMAGE        (1 Shl 4)
#define MC_TLIF_EXPANDEDIMAGE        (1 Shl 5)
#define MC_TLIF_CHILDREN             (1 Shl 6)

#define MC_TLIS_SELECTED             (1 Shl 1)
#define MC_TLIS_EXPANDED             (1 Shl 5)

#define MC_TLSIF_TEXT                (1 Shl 1)

#define MC_TLHT_NOWHERE              (1 Shl 0)
#define MC_TLHT_ONITEMICON           (1 Shl 1)
#define MC_TLHT_ONITEMSTATEICON      (1 Shl 2)
#define MC_TLHT_ONITEMLABEL          (1 Shl 3)
#define MC_TLHT_ONITEM               (MC_TLHT_ONITEMICON or MC_TLHT_ONITEMSTATEICON or MC_TLHT_ONITEMLABEL)
#define MC_TLHT_ONITEMINDENT         (1 Shl 4)
#define MC_TLHT_ONITEMBUTTON         (1 Shl 5)
#define MC_TLHT_ONITEMRIGHT          (1 Shl 6)
#define MC_TLHT_ONITEMLEFT           (1 Shl 7)
#define MC_TLHT_ABOVE                (1 Shl 8)
#define MC_TLHT_BELOW                (1 Shl 9)
#define MC_TLHT_TORIGHT              (1 Shl 10)
#define MC_TLHT_TOLEFT               (1 Shl 11)

#define MC_TLE_COLLAPSE              &H1
#define MC_TLE_EXPAND                &H2
#define MC_TLE_TOGGLE                &H3
#define MC_TLE_COLLAPSERESET         &H8000

#define MC_TLGN_ROOT                 &H0
#define MC_TLGN_NEXT                 &H1
#define MC_TLGN_PREVIOUS             &H2
#define MC_TLGN_PARENT               &H3
#define MC_TLGN_CHILD                &H4
#define MC_TLGN_FIRSTVISIBLE         &H5
#define MC_TLGN_NEXTVISIBLE          &H6
#define MC_TLGN_PREVIOUSVISIBLE      &H7
#define MC_TLGN_CARET                &H9
#define MC_TLGN_LASTVISIBLE          &Ha
#define MC_TLIR_BOUNDS               0
#define MC_TLIR_ICON                 1
#define MC_TLIR_LABEL                2
#define MC_TLIR_SELECTBOUNDS         3

Type MC_TLCOLUMNW
    fMask      As ULong            ' ← 改为 ULong（32位无符号）
    fmt        As Long             ' int 通常用 Long（32位）
    cx         As Long
    pszText    As WString Ptr
    cchTextMax As Long
    iImage     As Long
    iOrder     As Long
End Type

Type MC_TLCOLUMNA
    fMask      As ULong           
    fmt        As Long
    cx         As Long
    pszText    As ZString Ptr
    cchTextMax As Long
    iImage     As Long
    iOrder     As Long
End Type

Type MC_HTREELISTITEM As Any Ptr 

' 宽字符版本 (Unicode)
Type MC_TLITEMW
    fMask          As ULong        ' UINT → 32位无符号
    state          As ULong        ' UINT
    stateMask      As ULong        ' UINT
    pszText        As WString Ptr  ' WCHAR*
    cchTextMax     As Long         ' int → 32位有符号
    lParam         As LPARAM      ' LPARAM → 指针大小的有符号整数
    iImage         As Long         ' int
    iSelectedImage As Long         ' int
    iExpandedImage As Long         ' int
    cChildren      As Long         ' int
End Type

' ANSI 字符版本
Type MC_TLITEMA
    fMask          As ULong        ' UINT
    state          As ULong        ' UINT
    stateMask      As ULong        ' UINT
    pszText        As ZString Ptr  ' char*
    cchTextMax     As Long         ' int
    lParam         As LongInt      ' LPARAM
    iImage         As Long         ' int
    iSelectedImage As Long         ' int
    iExpandedImage As Long         ' int
    cChildren      As Long         ' int
End Type

 ' 宽字符子项结构
Type MC_TLSUBITEMW
    fMask      As ULong          ' UINT → 32位无符号
    iSubItem   As Long           ' int → 32位有符号
    pszText    As WString Ptr    ' WCHAR*
    cchTextMax As Long           ' int
End Type

' ANSI 子项结构
Type MC_TLSUBITEMA
    fMask      As ULong          ' UINT
    iSubItem   As Long           ' int
    pszText    As ZString Ptr    ' char*
    cchTextMax As Long           ' int
 End Type
 
type MC_TLINSERTSTRUCTW
     hParent As MC_HTREELISTITEM
     hInsertAfter As MC_HTREELISTITEM
     item as MC_TLITEMW
End type 
 
type MC_TLINSERTSTRUCTA
     hParent As MC_HTREELISTITEM
     hInsertAfter As MC_HTREELISTITEM
     item as MC_TLITEMA
 End type 
  
Type MC_TLHITTESTINFO
    pt       As POINT     ' Windows 的 POINT 结构（x, y 为 Long）
    flags    As ULong     ' UINT → 32位无符号
    hItem    As MC_HTREELISTITEM  ' 项句柄
    iSubItem As Long      ' int → 32位有符号
 End Type
 
Type MC_NMTREELIST 
     hdr As NMHDR
     action As ULong
     hItemOld As MC_HTREELISTITEM
     lParamOld As LPARAM
     hItemNew As MC_HTREELISTITEM
     lParamNew As LPARAM
End type 


Type MC_NMTLCUSTOMDRAW
    nmcd        As MC_NMCUSTOMDRAW
    iLevel      As Long          ' int
    iSubItem    As Long          ' int
    clrText     As COLORREF      ' ← 直接使用 COLORREF！
    clrTextBk   As COLORREF      ' ← 同上
 End Type
 
Type MC_NMTLDISPINFOW
     hdr As NMHDR
     hItem As MC_HTREELISTITEM
     item As MC_TLITEMW
End Type 
 
Type MC_NMTLDISPINFOA
     hdr As NMHDR
     hItem As MC_HTREELISTITEM
     item As MC_TLITEMA
End Type 
   
Type MC_NMTLSUBDISPINFOW
     hdr As NMHDR
     hItem As MC_HTREELISTITEM
     lItemParam As LPARAM
     subitem As MC_TLSUBITEMW
End Type 
  
Type MC_NMTLSUBDISPINFOA
     hdr As NMHDR
     hItem As MC_HTREELISTITEM
     lItemParam As LPARAM
     subitem As MC_TLSUBITEMA
End Type 

#define MC_TLM_INSERTCOLUMNW         (MC_TLM_FIRST + 0)

#define MC_TLM_INSERTCOLUMNA         (MC_TLM_FIRST + 1)

#define MC_TLM_SETCOLUMNW            (MC_TLM_FIRST + 2)

#define MC_TLM_SETCOLUMNA            (MC_TLM_FIRST + 3)

#define MC_TLM_GETCOLUMNW            (MC_TLM_FIRST + 4)

#define MC_TLM_GETCOLUMNA            (MC_TLM_FIRST + 5)

#define MC_TLM_DELETECOLUMN          (MC_TLM_FIRST + 6)

#define MC_TLM_SETCOLUMNORDERARRAY   (MC_TLM_FIRST + 7)

#define MC_TLM_GETCOLUMNORDERARRAY   (MC_TLM_FIRST + 8)

#define MC_TLM_SETCOLUMNWIDTH        (MC_TLM_FIRST + 9)

#define MC_TLM_GETCOLUMNWIDTH        (MC_TLM_FIRST + 10)

#define MC_TLM_INSERTITEMW           (MC_TLM_FIRST + 11)

#define MC_TLM_INSERTITEMA           (MC_TLM_FIRST + 12)

#define MC_TLM_SETITEMW              (MC_TLM_FIRST + 13)

#define MC_TLM_SETITEMA              (MC_TLM_FIRST + 14)

#define MC_TLM_GETITEMW              (MC_TLM_FIRST + 15)

#define MC_TLM_GETITEMA              (MC_TLM_FIRST + 16)

#define MC_TLM_DELETEITEM            (MC_TLM_FIRST + 17)

#define MC_TLM_SETITEMHEIGHT         (MC_TLM_FIRST + 18)

#define MC_TLM_GETITEMHEIGHT         (MC_TLM_FIRST + 19)

#define MC_TLM_SETSUBITEMW           (MC_TLM_FIRST + 20)

#define MC_TLM_SETSUBITEMA           (MC_TLM_FIRST + 21)

#define MC_TLM_GETSUBITEMW           (MC_TLM_FIRST + 22)

#define MC_TLM_GETSUBITEMA           (MC_TLM_FIRST + 23)

#define MC_TLM_SETINDENT             (MC_TLM_FIRST + 24)

#define MC_TLM_GETINDENT             (MC_TLM_FIRST + 25)

#define MC_TLM_HITTEST               (MC_TLM_FIRST + 26)

#define MC_TLM_EXPAND                (MC_TLM_FIRST + 27)

#define MC_TLM_GETNEXTITEM           (MC_TLM_FIRST + 28)

#define MC_TLM_GETVISIBLECOUNT       (MC_TLM_FIRST + 29)

#define MC_TLM_ENSUREVISIBLE         (MC_TLM_FIRST + 30)

#define MC_TLM_SETIMAGELIST          (MC_TLM_FIRST + 31)

#define MC_TLM_GETIMAGELIST          (MC_TLM_FIRST + 32)

#define MC_TLM_GETSELECTEDCOUNT     (MC_TLM_FIRST + 33)

#define MC_TLM_GETITEMRECT          (MC_TLM_FIRST + 34)

#define MC_TLM_GETSUBITEMRECT       (MC_TLM_FIRST + 35)

#define MC_TLM_SETTOOLTIPS          (MC_TLM_FIRST + 36)

#define MC_TLM_GETTOOLTIPS          (MC_TLM_FIRST + 37)


#define MC_TLN_DELETEITEM            (MC_TLN_FIRST + 0)
#define MC_TLN_SELCHANGING           (MC_TLN_FIRST + 1)

#define MC_TLN_SELCHANGED            (MC_TLN_FIRST + 2)
#define MC_TLN_EXPANDING             (MC_TLN_FIRST + 3)
#define MC_TLN_EXPANDED              (MC_TLN_FIRST + 4)

#if 0
#define MC_TLN_SETDISPINFOW          (MC_TLN_FIRST + 5)
#define MC_TLN_SETDISPINFOA          (MC_TLN_FIRST + 6)
#endif

#define MC_TLN_GETDISPINFOW          (MC_TLN_FIRST + 7)

#define MC_TLN_GETDISPINFOA          (MC_TLN_FIRST + 8)

#if 0
#define MC_TLN_SETSUBDISPINFOW       (MC_TLN_FIRST + 9)
#define MC_TLN_SETSUBDISPINFOA       (MC_TLN_FIRST + 10)
#endif

 #define MC_TLN_GETSUBDISPINFOW       (MC_TLN_FIRST + 11)

#define MC_TLN_GETSUBDISPINFOA       (MC_TLN_FIRST + 12)

#define MC_WC_TREELIST           MCTRL_NAME_AW(MC_WC_TREELIST)
#define MC_TLCOLUMN              MCTRL_NAME_AW(MC_TLCOLUMN)
#define MC_TLITEM                MCTRL_NAME_AW(MC_TLITEM)
#define MC_TLSUBITEM             MCTRL_NAME_AW(MC_TLSUBITEM)
#define MC_TLINSERTSTRUCT        MCTRL_NAME_AW(MC_TLINSERTSTRUCT)
#define MC_NMTLDISPINFO          MCTRL_NAME_AW(MC_NMTLDISPINFO)
#define MC_NMTLSUBDISPINFO       MCTRL_NAME_AW(MC_NMTLSUBDISPINFO)
#define MC_TLM_SETCOLUMN         MCTRL_NAME_AW(MC_TLM_SETCOLUMN)
#define MC_TLM_INSERTCOLUMN      MCTRL_NAME_AW(MC_TLM_INSERTCOLUMN)
#define MC_TLM_SETCOLUMN         MCTRL_NAME_AW(MC_TLM_SETCOLUMN)
#define MC_TLM_GETCOLUMN         MCTRL_NAME_AW(MC_TLM_GETCOLUMN)
#define MC_TLM_INSERTITEM        MCTRL_NAME_AW(MC_TLM_INSERTITEM)
#define MC_TLM_SETITEM           MCTRL_NAME_AW(MC_TLM_SETITEM)
#define MC_TLM_GETITEM           MCTRL_NAME_AW(MC_TLM_GETITEM)
#define MC_TLM_SETSUBITEM        MCTRL_NAME_AW(MC_TLM_SETSUBITEM)
#define MC_TLM_GETSUBITEM        MCTRL_NAME_AW(MC_TLM_GETSUBITEM)
#define MC_TLN_SETDISPINFO       MCTRL_NAME_AW(MC_TLN_SETDISPINFO)
#define MC_TLN_GETDISPINFO       MCTRL_NAME_AW(MC_TLN_GETDISPINFO)
#define MC_TLN_SETSUBDISPINFO    MCTRL_NAME_AW(MC_TLN_SETSUBDISPINFO)
#define MC_TLN_GETSUBDISPINFO    MCTRL_NAME_AW(MC_TLN_GETSUBDISPINFO)

