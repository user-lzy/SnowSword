Private Type POINTAPI
    X As Long
    Y As Long
End Type
 
Private Type RECT
    Left        As Long
    Top         As Long
    Right       As Long
    Bottom      As Long
End Type
 
Private Type RECTW
    Left                As Long
    Top                 As Long
    Right               As Long
    Bottom              As Long
    Width               As Long
    Height              As Long
End Type
 
Private Type PAINTSTRUCT
    hDC                 As Long
    fErase              As Long
    rcPaint             As RECT
    fRestore            As Long
    fIncUpdate          As Long
    rgbReserved(32)     As Byte
End Type
 
Private Type HDITEM
    mask                As Long
    cxy                 As Long
    pszText             As String
    hbm                 As Long
    cchTextMax          As Long
    fmt                 As Long
    IntPtr              As Long
End Type
 
Private Type TRACKMOUSEEVENTTYPE
    cbSize      As Long
    dwFlags     As Long
    hwndTrack   As Long
    dwHoverTime As Long
End Type
 
Private Type WINDOWPOS
    hwnd                     As Long
    hWndInsertAfter          As Long
    X                        As Long
    Y                        As Long
    cX                       As Long
    cY                       As Long
    Flags                    As Long
End Type
 
Private Type NCCALCSIZE_PARAMS
    rgrc(0 To 2)             As RECT
    lppos                    As Long
End Type
 
Private Type DRAWITEMSTRUCT
    CtlType As Long
    CtlID As Long
    itemID As Long
    itemAction As Long
    itemState As Long
    hwndItem As Long
    hDC As Long
    rcItem As RECT
    itemData As Long
End Type
 
Private Enum DTSTYLE
    DT_LEFT = &H0
    DT_TOP = &H0
    DT_CENTER = &H1
    DT_RIGHT = &H2
    DT_VCENTER = &H4
    DT_BOTTOM = &H8
    DT_WORDBREAK = &H10
    DT_SINGLELINE = &H20
    DT_EXPANDTABS = &H40
    DT_TABSTOP = &H80
    DT_NOCLIP = &H100
    DT_EXTERNALLEADING = &H200
    DT_CALCRECT = &H400
    DT_NOPREFIX = &H800
    DT_INTERNAL = &H1000
    DT_EDITCONTROL = &H2000
    DT_PATH_ELLIPSIS = &H4000
    DT_FORE_ELLIPSIS = &H8000
    DT_END_ELLIPSIS = &H8000&
    DT_MODIFYSTRING = &H10000
    DT_RTLREADING = &H20000
    DT_WORD_ELLIPSIS = &H40000
End Enum
 
Private Const GWL_WNDPROC = (-4)
Private Const GWL_STYLE = (-16)
Private Const GWL_EXSTYLE = (-20)
 
Private Const WM_ACTIVATE                   As Long = &H6
Private Const WM_ACTIVATEAPP                As Long = &H1C
Private Const WM_ASKCBFORMATNAME            As Long = &H30C
Private Const WM_CANCELJOURNAL              As Long = &H4B
Private Const WM_CANCELMODE                 As Long = &H1F
Private Const WM_CHANGECBCHAIN              As Long = &H30D
Private Const WM_CHAR                       As Long = &H102
Private Const WM_CHARTOITEM                 As Long = &H2F
Private Const WM_CHILDACTIVATE              As Long = &H22
Private Const WM_CLEAR                      As Long = &H303
Private Const WM_CLOSE                      As Long = &H10
Private Const WM_COMMAND                    As Long = &H111
Private Const WM_COMMNOTIFY                 As Long = &H44
Private Const WM_COMPACTING                 As Long = &H41
Private Const WM_COMPAREITEM                As Long = &H39
Private Const WM_CONVERTREQUESTEX           As Long = &H108
Private Const WM_COPY                       As Long = &H301
Private Const WM_COPYDATA                   As Long = &H4A
Private Const WM_CREATE                     As Long = &H1
Private Const WM_CUT                        As Long = &H300
Private Const WM_DEADCHAR                   As Long = &H103
Private Const WM_DELETEITEM                 As Long = &H2D
Private Const WM_DESTROY                    As Long = &H2
Private Const WM_DESTROYCLIPBOARD           As Long = &H307
Private Const WM_DEVMODECHANGE              As Long = &H1B
Private Const WM_DRAWCLIPBOARD              As Long = &H308
Private Const WM_DRAWITEM                   As Long = &H2B
Private Const WM_DROPFILES                  As Long = &H233
Private Const WM_ENABLE                     As Long = &HA
Private Const WM_ENDSESSION                 As Long = &H16
Private Const WM_ENTERIDLE                  As Long = &H121
Private Const WM_ENTERMENULOOP              As Long = &H211
Private Const WM_ERASEBKGND                 As Long = &H14
Private Const WM_EXITMENULOOP               As Long = &H212
Private Const WM_FONTCHANGE                 As Long = &H1D
Private Const WM_GETDLGCODE                 As Long = &H87
Private Const WM_GETFONT                    As Long = &H31
Private Const WM_GETHOTKEY                  As Long = &H33
Private Const WM_GETMINMAXINFO              As Long = &H24
Private Const WM_GETTEXT                    As Long = &HD
Private Const WM_GETTEXTLENGTH              As Long = &HE
Private Const WM_HOTKEY                     As Long = &H312
Private Const WM_HSCROLL                    As Long = &H114
Private Const WM_HSCROLLCLIPBOARD           As Long = &H30E
Private Const WM_ICONERASEBKGND             As Long = &H27
Private Const WM_IME_CHAR                   As Long = &H286
Private Const WM_IME_COMPOSITION            As Long = &H10F
Private Const WM_IME_COMPOSITIONFULL        As Long = &H284
Private Const WM_IME_CONTROL                As Long = &H283
Private Const WM_IME_ENDCOMPOSITION         As Long = &H10E
Private Const WM_IME_KEYDOWN                As Long = &H290
Private Const WM_IME_KEYLAST                As Long = &H10F
Private Const WM_IME_KEYUP                  As Long = &H291
Private Const WM_IME_NOTIFY                 As Long = &H282
Private Const WM_IME_SELECT                 As Long = &H285
Private Const WM_IME_SETCONTEXT             As Long = &H281
Private Const WM_IME_STARTCOMPOSITION       As Long = &H10D
Private Const WM_INITDIALOG                 As Long = &H110
Private Const WM_INITMENU                   As Long = &H116
Private Const WM_INITMENUPOPUP              As Long = &H117
Private Const WM_KEYDOWN                    As Long = &H100
Private Const WM_KEYFIRST                   As Long = &H100
Private Const WM_KEYLAST                    As Long = &H108
Private Const WM_KEYUP                      As Long = &H101
Private Const WM_KILLFOCUS                  As Long = &H8
Private Const WM_LBUTTONDBLCLK              As Long = &H203
Private Const WM_LBUTTONDOWN                As Long = &H201
Private Const WM_LBUTTONUP                  As Long = &H202
Private Const WM_MBUTTONDBLCLK              As Long = &H209
Private Const WM_MBUTTONDOWN                As Long = &H207
Private Const WM_MBUTTONUP                  As Long = &H208
Private Const WM_MDIACTIVATE                As Long = &H222
Private Const WM_MDICASCADE                 As Long = &H227
Private Const WM_MDICREATE                  As Long = &H220
Private Const WM_MDIDESTROY                 As Long = &H221
Private Const WM_MDIGETACTIVE               As Long = &H229
Private Const WM_MDIICONARRANGE             As Long = &H228
Private Const WM_MDIMAXIMIZE                As Long = &H225
Private Const WM_MDINEXT                    As Long = &H224
Private Const WM_MDIREFRESHMENU             As Long = &H234
Private Const WM_MDIRESTORE                 As Long = &H223
Private Const WM_MDISETMENU                 As Long = &H230
Private Const WM_MDITILE                    As Long = &H226
Private Const WM_MEASUREITEM                As Long = &H2C
Private Const WM_MENUCHAR                   As Long = &H120
Private Const WM_MENUSELECT                 As Long = &H11F
Private Const WM_MOUSEACTIVATE              As Long = &H21
Private Const WM_MOUSEFIRST                 As Long = &H200
Private Const WM_MOUSELAST                  As Long = &H209
Private Const WM_MOUSELEAVE                 As Long = &H2A3
Private Const WM_MOUSEWHEEL                 As Long = &H20A
Private Const WM_MOUSEMOVE                  As Long = &H200
Private Const WM_MOVE                       As Long = &H3
Private Const WM_NEXTDLGCTL                 As Long = &H28
Private Const WM_NULL                       As Long = &H0
Private Const WM_OTHERWINDOWCREATED         As Long = &H42                      '  no longer suported
Private Const WM_OTHERWINDOWDESTROYED       As Long = &H43                      '  no longer suported
Private Const WM_PAINT                      As Long = &HF
Private Const WM_PAINTCLIPBOARD             As Long = &H309
Private Const WM_PAINTICON                  As Long = &H26
Private Const WM_PALETTECHANGED             As Long = &H311
Private Const WM_PALETTEISCHANGING          As Long = &H310
Private Const WM_PARENTNOTIFY               As Long = &H210
Private Const WM_PASTE                      As Long = &H302
Private Const WM_PENWINFIRST                As Long = &H380
Private Const WM_PENWINLAST                 As Long = &H38F
Private Const WM_POWER                      As Long = &H48
Private Const WM_USER                       As Long = &H400
Private Const WM_PSD_ENVSTAMPRECT           As Long = (WM_USER + 5)
Private Const WM_PSD_FULLPAGERECT           As Long = (WM_USER + 1)
Private Const WM_PSD_GREEKTEXTRECT          As Long = (WM_USER + 4)
Private Const WM_PSD_MARGINRECT             As Long = (WM_USER + 3)
Private Const WM_PSD_MINMARGINRECT          As Long = (WM_USER + 2)
Private Const WM_PSD_PAGESETUPDLG           As Long = (WM_USER)
Private Const WM_PSD_YAFULLPAGERECT         As Long = (WM_USER + 6)
Private Const WM_QUERYDRAGICON              As Long = &H37
Private Const WM_QUERYENDSESSION            As Long = &H11
Private Const WM_QUERYNEWPALETTE            As Long = &H30F
Private Const WM_QUERYOPEN                  As Long = &H13
Private Const WM_QUEUESYNC                  As Long = &H23
Private Const WM_QUIT                       As Long = &H12
Private Const WM_RBUTTONDBLCLK              As Long = &H206
Private Const WM_RBUTTONDOWN                As Long = &H204
Private Const WM_RBUTTONUP                  As Long = &H205
Private Const WM_RENDERALLFORMATS           As Long = &H306
Private Const WM_RENDERFORMAT               As Long = &H305
Private Const WM_SETCURSOR                  As Long = &H20
Private Const WM_SETFOCUS                   As Long = &H7
Private Const WM_SETFONT                    As Long = &H30
Private Const WM_SETHOTKEY                  As Long = &H32
Private Const WM_SETREDRAW                  As Long = &HB
Private Const WM_SETTEXT                    As Long = &HC
Private Const WM_SHOWWINDOW                 As Long = &H18
Private Const WM_SIZE                       As Long = &H5
Private Const WM_SIZECLIPBOARD              As Long = &H30B
Private Const WM_SPOOLERSTATUS              As Long = &H2A
Private Const WM_SYSCHAR                    As Long = &H106
Private Const WM_SYSCOLORCHANGE             As Long = &H15
Private Const WM_SYSCOMMAND                 As Long = &H112
Private Const WM_SYSDEADCHAR                As Long = &H107
Private Const WM_SYSKEYDOWN                 As Long = &H104
Private Const WM_SYSKEYUP                   As Long = &H105
Private Const WM_STYLECHANGED               As Long = &H7D
Private Const WM_TIMECHANGE                 As Long = &H1E
Private Const WM_TIMER                      As Long = &H113
Private Const WM_UNDO                       As Long = &H304
Private Const WM_VKEYTOITEM                 As Long = &H2E
Private Const WM_VSCROLL                    As Long = &H115
Private Const WM_VSCROLLCLIPBOARD           As Long = &H30A
Private Const WM_WINDOWPOSCHANGED           As Long = &H47
Private Const WM_WINDOWPOSCHANGING          As Long = &H46
Private Const WM_WININICHANGE               As Long = &H1A
Private Const WM_CHOOSEFONT_GETLOGFONT      As Long = (WM_USER + 1)
Private Const WM_CHOOSEFONT_SETFLAGS        As Long = (WM_USER + 102)
Private Const WM_CHOOSEFONT_SETLOGFONT      As Long = (WM_USER + 101)
Private Const WM_DDE_FIRST                  As Long = &H3E0
Private Const WM_DDE_ACK                    As Long = (WM_DDE_FIRST + 4)
Private Const WM_DDE_ADVISE                 As Long = (WM_DDE_FIRST + 2)
Private Const WM_DDE_DATA                   As Long = (WM_DDE_FIRST + 5)
Private Const WM_DDE_EXECUTE                As Long = (WM_DDE_FIRST + 8)
Private Const WM_DDE_INITIATE               As Long = (WM_DDE_FIRST)
Private Const WM_DDE_LAST                   As Long = (WM_DDE_FIRST + 8)
Private Const WM_DDE_POKE                   As Long = (WM_DDE_FIRST + 7)
Private Const WM_DDE_REQUEST                As Long = (WM_DDE_FIRST + 6)
Private Const WM_DDE_TERMINATE              As Long = (WM_DDE_FIRST + 1)
Private Const WM_DDE_UNADVISE               As Long = (WM_DDE_FIRST + 3)
 
Private Const WM_NCACTIVATE                 As Long = &H86
Private Const WM_NCCALCSIZE                 As Long = &H83
Private Const WM_NCCREATE                   As Long = &H81
Private Const WM_NCDESTROY                  As Long = &H82
Private Const WM_NCHITTEST                  As Long = &H84
Private Const WM_NCLBUTTONDBLCLK            As Long = &HA3
Private Const WM_NCLBUTTONDOWN              As Long = &HA1
Private Const WM_NCLBUTTONUP                As Long = &HA2
Private Const WM_NCMBUTTONDBLCLK            As Long = &HA9
Private Const WM_NCMBUTTONDOWN              As Long = &HA7
Private Const WM_NCMBUTTONUP                As Long = &HA8
Private Const WM_NCMOUSEMOVE                As Long = &HA0
Private Const WM_NCPAINT                    As Long = &H85
Private Const WM_NCRBUTTONDBLCLK            As Long = &HA6
Private Const WM_NCRBUTTONDOWN              As Long = &HA4
Private Const WM_NCRBUTTONUP                As Long = &HA5
Private Const WM_NCPOPUPMENU                As Long = &HAE
 
Private Const WM_CTLCOLOR                   As Long = &H19
Private Const WM_CTLCOLORBTN                As Long = &H135
Private Const WM_CTLCOLORDLG                As Long = &H136
Private Const WM_CTLCOLOREDIT               As Long = &H133
Private Const WM_CTLCOLORLISTBOX            As Long = &H134
Private Const WM_CTLCOLORMSGBOX             As Long = &H132
Private Const WM_CTLCOLORSCROLLBAR          As Long = &H137
Private Const WM_CTLCOLORSTATIC             As Long = &H138
 
Private Const HDM_FIRST                     As Long = &H1200
Private Const HDM_CLEARFILTER               As Long = (HDM_FIRST + 24)
Private Const HDM_CREATEDRAGIMAGE           As Long = (HDM_FIRST + 16)
Private Const HDM_DELETEITEM                As Long = (HDM_FIRST + 2)
Private Const HDM_EDITFILTER                As Long = (HDM_FIRST + 23)
Private Const HDM_GETBITMAPMARGIN           As Long = (HDM_FIRST + 21)
Private Const HDM_GETIMAGELIST              As Long = (HDM_FIRST + 9)
Private Const HDM_GETITEMA                  As Long = (HDM_FIRST + 3)
Private Const HDM_GETITEMCOUNT              As Long = (HDM_FIRST + 0)
Private Const HDM_GETITEMRECT               As Long = (HDM_FIRST + 7)
Private Const HDM_GETITEMW                  As Long = (HDM_FIRST + 11)
Private Const HDM_GETORDERARRAY             As Long = (HDM_FIRST + 17)
Private Const HDM_HITTEST                   As Long = (HDM_FIRST + 6)
Private Const HDM_INSERTITEMA               As Long = (HDM_FIRST + 1)
Private Const HDM_INSERTITEMW               As Long = (HDM_FIRST + 10)
Private Const HDM_LAYOUT                    As Long = (HDM_FIRST + 5)
Private Const HDM_ORDERTOINDEX              As Long = (HDM_FIRST + 15)
Private Const HDM_SETBITMAPMARGIN           As Long = (HDM_FIRST + 20)
Private Const HDM_SETFILTERCHANGETIMEOUT    As Long = (HDM_FIRST + 22)
Private Const HDM_SETHOTDIVIDER             As Long = (HDM_FIRST + 19)
Private Const HDM_SETIMAGELIST              As Long = (HDM_FIRST + 8)
Private Const HDM_SETITEMA                  As Long = (HDM_FIRST + 4)
Private Const HDM_SETITEMW                  As Long = (HDM_FIRST + 12)
Private Const HDM_SETORDERARRAY             As Long = (HDM_FIRST + 18)
Private Const HDM_SETUNICODEFORMAT          As Long = &H2005
Private Const HDM_GETUNICODEFORMAT          As Long = &H2006
 
Private Const HDI_BITMAP                    As Long = &H10
Private Const HDI_DI_SETITEM                As Long = &H40
Private Const HDI_FILTER                    As Long = &H100
Private Const HDI_FORMAT                    As Long = &H4
Private Const HDI_WIDTH                     As Long = &H1
Private Const HDI_HEIGHT                    As Long = HDI_WIDTH
Private Const HDI_HIDDEN                    As Long = (&H1)
Private Const HDI_IMAGE                     As Long = &H20
Private Const HDI_LPARAM                    As Long = &H8
Private Const HDI_ORDER                     As Long = &H80
Private Const HDI_TEXT                      As Long = &H2
 
Private Const MK_LBUTTON                    As Long = &H1
Private Const MK_MBUTTON                    As Long = &H10
Private Const MK_RBUTTON                    As Long = &H2
 
Private Const WS_BORDER                     As Long = &H800000
Private Const WS_VSCROLL                    As Long = &H200000
Private Const WS_HSCROLL                    As Long = &H100000
Private Const WS_EX_CLIENTEDGE              As Long = &H200&
 
Private Const SM_CXVSCROLL                  As Long = &H2
Private Const SM_CYVSCROLL                  As Long = &H14
Private Const SM_CXHSCROLL                  As Long = &H15
Private Const SM_CYHSCROLL                  As Long = &H3
Private Const SM_CXDLGFRAME                 As Long = &H7
Private Const SM_CYDLGFRAME                 As Long = &H8
Private Const SM_CXCHECKBOX                 As Long = &H47
Private Const SM_CYCHECKBOX                 As Long = &H48
 
Private Const CB_SHOWDROPDOWN               As Long = &H14F
Private Const CB_GETDROPPEDSTATE            As Long = &H157
 
Private Const PBM_GETPOS                    As Long = (WM_USER + 8)
Private Const PBM_SETBARCOLOR               As Long = (WM_USER + 9)
Private Const PBS_SMOOTH                    As Long = &H1
Private Const PBS_VERTICAL                  As Long = &H4
 
Private Const BM_GETCHECK                   As Long = &HF0
Private Const BM_SETCHECK                   As Long = &HF1
Private Const BM_GETSTATE                   As Long = &HF2
Private Const BM_SETSTYLE                   As Long = &HF4
 
Private Const BS_NULL                       As Long = 1
Private Const BS_3STATE                     As Long = &H5&
Private Const BS_AUTO3STATE                 As Long = &H6&
Private Const BS_AUTOCHECKBOX               As Long = &H3&
Private Const BS_AUTORADIOBUTTON            As Long = &H9&
Private Const BS_CHECKBOX                   As Long = &H2&
Private Const BS_DEFPUSHBUTTON              As Long = &H1&
Private Const BS_DIBPATTERN                 As Long = 5
Private Const BS_DIBPATTERN8X8              As Long = 8
Private Const BS_DIBPATTERNPT               As Long = 6
Private Const BS_GROUPBOX                   As Long = &H7&
Private Const BS_HATCHED                    As Long = 2
Private Const BS_HOLLOW                     As Long = BS_NULL
Private Const BS_INDEXED                    As Long = 4
Private Const BS_LEFTTEXT                   As Long = &H20&
Private Const BS_OWNERDRAW                  As Long = &HB&
Private Const BS_PATTERN                    As Long = 3
Private Const BS_PATTERN8X8                 As Long = 7
Private Const BS_PUSHBUTTON                 As Long = &H0&
Private Const BS_RADIOBUTTON                As Long = &H4&
Private Const BS_SOLID                      As Long = 0
Private Const BS_USERBUTTON                 As Long = &H8&
 
Private Const LBS_OWNERDRAWVARIABLE = &H20&
Private Const CBS_OWNERDRAWVARIABLE = &H20&
 
Private Const ODT_BUTTON = 4
Private Const ODT_COMBOBOX = 3
Private Const ODT_HEADER = 100
Private Const ODT_LISTBOX = 2
Private Const ODT_LISTVIEW = 102
Private Const ODT_MENU = 1
Private Const ODT_STATIC = 5
Private Const ODT_TAB = 101
 
Private Declare Function GetWindowLong Lib "user32" Alias "GetWindowLongA" (ByVal hwnd As Long, ByVal nIndex As Long) As Long
Private Declare Function SetWindowLong Lib "user32" Alias "SetWindowLongA" (ByVal hwnd As Long, ByVal nIndex As Long, ByVal dwNewLong As Long) As Long
Private Declare Function CallWindowProc Lib "user32" Alias "CallWindowProcA" (ByVal lpPrevWndFunc As Long, ByVal hwnd As Long, ByVal Msg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Private Declare Function DefWindowProc Lib "user32" Alias "DefWindowProcA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Private Declare Function GetProp Lib "user32" Alias "GetPropA" (ByVal hwnd As Long, ByVal lpString As String) As Long
Private Declare Function RemoveProp Lib "user32" Alias "RemovePropA" (ByVal hwnd As Long, ByVal lpString As String) As Long
Private Declare Function SetProp Lib "user32" Alias "SetPropA" (ByVal hwnd As Long, ByVal lpString As String, ByVal hData As Long) As Long
Private Declare Function SendMessage Lib "user32" Alias "SendMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, lParam As Any) As Long
Private Declare Function GetModuleHandleA Lib "kernel32" (ByVal lpModuleName As String) As Long
Private Declare Function LoadLibrary Lib "kernel32" Alias "LoadLibraryA" (ByVal lpLibFileName As String) As Long
Private Declare Function GetProcAddress Lib "kernel32" (ByVal hModule As Long, ByVal lpProcName As String) As Long
Private Declare Function FreeLibrary Lib "kernel32" (ByVal hLibModule As Long) As Long
Private Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (Destination As Any, Source As Any, ByVal Length As Long)
 
Private Declare Function CreateHalftonePalette Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function SelectPalette Lib "gdi32" (ByVal hDC As Long, ByVal hPalette As Long, ByVal bForceBackground As Long) As Long
Private Declare Function RealizePalette Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function CreateBitmap Lib "gdi32" (ByVal nWidth As Long, ByVal nHeight As Long, ByVal nPlanes As Long, ByVal nBitCount As Long, lpBits As Any) As Long
Private Declare Function OleTranslateColor Lib "oleaut32.dll" (ByVal lOleColor As Long, ByVal lHPalette As Long, lColorRef As Long) As Long
 
Private Declare Function GetClassName Lib "user32" Alias "GetClassNameA" (ByVal hwnd As Long, ByVal lpClassName As String, ByVal nMaxCount As Long) As Long
Private Declare Function GetWindowText Lib "user32" Alias "GetWindowTextA" (ByVal hwnd As Long, ByVal lpString As String, ByVal cch As Long) As Long
Private Declare Function GetWindowTextLength Lib "user32" Alias "GetWindowTextLengthA" (ByVal hwnd As Long) As Long
Private Declare Function GetWindowRect Lib "user32" (ByVal hwnd As Long, lpRect As RECT) As Long
Private Declare Function GetClientRect Lib "user32" (ByVal hwnd As Long, lpRect As RECT) As Long
Private Declare Function EnumChildWindows Lib "user32" (ByVal hWndParent As Long, ByVal lpEnumFunc As Long, ByVal lParam As Long) As Long
Private Declare Function IsWindowEnabled Lib "user32" (ByVal hwnd As Long) As Long
Private Declare Function IsWindowUnicode Lib "user32" (ByVal hwnd As Long) As Long
Private Declare Function BeginPaint Lib "user32" (ByVal hwnd As Long, lpPaint As PAINTSTRUCT) As Long
Private Declare Function EndPaint Lib "user32" (ByVal hwnd As Long, lpPaint As PAINTSTRUCT) As Long
Private Declare Function TrackMouseEvent Lib "user32.dll" (ByRef lpEventTrack As TRACKMOUSEEVENTTYPE) As Long ' Win98 or later
Private Declare Function TrackMouseEvent2 Lib "comctl32.dll" Alias "_TrackMouseEvent" (ByRef lpEventTrack As TRACKMOUSEEVENTTYPE) As Long ' Win95 w/ IE 3.0
Private Declare Function SetTimer Lib "user32" (ByVal hwnd As Long, ByVal nIDEvent As Long, ByVal uElapse As Long, ByVal lpTimerFunc As Long) As Long
Private Declare Function KillTimer Lib "user32" (ByVal hwnd As Long, ByVal nIDEvent As Long) As Long
Private Declare Function RedrawWindow Lib "user32" (ByVal hwnd As Long, lprcUpdate As Any, ByVal hrgnUpdate As Long, ByVal fuRedraw As Long) As Long
Private Declare Function GetParent Lib "user32" (ByVal hwnd As Long) As Long
Private Declare Function GetSystemMetrics Lib "user32" (ByVal nIndex As Long) As Long
Private Declare Function CreateWindowEx Lib "user32" Alias "CreateWindowExA" (ByVal dwExStyle As Long, ByVal lpClassName As String, ByVal lpWindowName As String, ByVal dwStyle As Long, ByVal X As Long, ByVal Y As Long, ByVal nWidth As Long, ByVal nHeight As Long, ByVal hWndParent As Long, ByVal hMenu As Long, ByVal hInstance As Long, lpParam As Any) As Long
 
Private Declare Function CreateDCAsNull Lib "gdi32" Alias "CreateDCA" (ByVal lpDriverName As String, lpDeviceName As Any, lpOutput As Any, lpInitData As Any) As Long
Private Declare Function CreateDC Lib "gdi32" Alias "CreateDCA" (ByVal lpDriverName As String, ByVal lpDeviceName As String, ByVal lpOutput As String, lpInitData As Any) As Long
Private Declare Function GetDC Lib "user32" (ByVal hwnd As Long) As Long
Private Declare Function GetWindowDC Lib "user32" (ByVal hwnd As Long) As Long
Private Declare Function CreateCompatibleDC Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function CreateCompatibleBitmap Lib "gdi32" (ByVal hDC As Long, ByVal nWidth As Long, ByVal nHeight As Long) As Long
Private Declare Function SelectObject Lib "gdi32" (ByVal hDC As Long, ByVal hObject As Long) As Long
Private Declare Function DeleteObject Lib "gdi32" (ByVal hObject As Long) As Long
Private Declare Function DeleteDC Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function ReleaseDC Lib "user32" (ByVal hwnd As Long, ByVal hDC As Long) As Long
Private Declare Function GetPixel Lib "gdi32" (ByVal hDC As Long, ByVal X As Long, ByVal Y As Long) As Long
Private Declare Function SetPixelV Lib "gdi32" (ByVal hDC As Long, ByVal X As Long, ByVal Y As Long, ByVal crColor As Long) As Long
Private Declare Function BitBlt Lib "gdi32" (ByVal hDestDC As Long, ByVal X As Long, ByVal Y As Long, ByVal nWidth As Long, ByVal nHeight As Long, ByVal hSrcDC As Long, ByVal xSrc As Long, ByVal ySrc As Long, ByVal dwRop As Long) As Long
Private Declare Function StretchBlt Lib "gdi32" (ByVal hDC As Long, ByVal X As Long, ByVal Y As Long, ByVal nWidth As Long, ByVal nHeight As Long, ByVal hSrcDC As Long, ByVal xSrc As Long, ByVal ySrc As Long, ByVal nSrcWidth As Long, ByVal nSrcHeight As Long, ByVal dwRop As Long) As Long
Private Declare Function AlphaBlend Lib "msimg32" (ByVal hDestDC As Long, ByVal X As Long, ByVal Y As Long, ByVal nWidth As Long, ByVal nHeight As Long, ByVal hSrcDC As Long, ByVal xSrc As Long, ByVal ySrc As Long, ByVal widthSrc As Long, ByVal heightSrc As Long, ByVal blendFunct As Long) As Boolean
Private Declare Function GetTextColor Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function SetTextColor Lib "gdi32" (ByVal hDC As Long, ByVal crColor As Long) As Long
Private Declare Function DrawText Lib "user32" Alias "DrawTextA" (ByVal hDC As Long, ByVal lpStr As String, ByVal nCount As Long, lpRect As RECT, ByVal wFormat As Long) As Long
Private Declare Function GetBkColor Lib "gdi32" (ByVal hDC As Long) As Long
Private Declare Function SetBkColor Lib "gdi32" (ByVal hDC As Long, ByVal crColor As Long) As Long
Private Declare Function SetBkMode Lib "gdi32" (ByVal hDC As Long, ByVal nBkMode As Long) As Long
Private Declare Function DrawFocusRect Lib "user32" (ByVal hDC As Long, lpRect As RECT) As Long
Private Declare Function GetFocus Lib "user32" () As Long
Private Declare Function CreateRoundRectRgn Lib "gdi32" (ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long, ByVal X3 As Long, ByVal Y3 As Long) As Long
Private Declare Function SetWindowRgn Lib "user32" (ByVal hwnd As Long, ByVal hRgn As Long, ByVal bRedraw As Boolean) As Long
Private Declare Function FrameRect Lib "user32" (ByVal hDC As Long, lpRect As RECT, ByVal hBrush As Long) As Long
Private Declare Function CreateSolidBrush Lib "gdi32" (ByVal crColor As Long) As Long
Private Declare Function FillRect Lib "user32" (ByVal hDC As Long, lpRect As RECT, ByVal hBrush As Long) As Long
Private Declare Function Rectangle Lib "gdi32" (ByVal hDC As Long, ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long) As Long
Private Declare Function GetCursorPos Lib "user32" (lpPoint As POINTAPI) As Long
Private Declare Function WindowFromPoint Lib "user32" (ByVal xPoint As Long, ByVal yPoint As Long) As Long
Private Declare Function FindWindowEx Lib "user32" Alias "FindWindowExA" (ByVal hWnd1 As Long, ByVal hWnd2 As Long, ByVal lpsz1 As String, ByVal lpsz2 As String) As Long
Private Declare Function InvalidateRect Lib "user32" (ByVal hwnd As Long, lpRect As RECT, ByVal bErase As Long) As Long
Private Declare Function RoundRect Lib "gdi32" (ByVal hDC As Long, ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long, ByVal X3 As Long, ByVal Y3 As Long) As Long
Private Declare Function CreatePen Lib "gdi32" (ByVal nPenStyle As Long, ByVal nWidth As Long, ByVal crColor As Long) As Long
 
Private m_Init              As Boolean                                          '     Ƿ  Ѿ   ʼ  
Private m_hBtnSrcDC         As Long
Private m_hCbbSrcDC         As Long
Private m_hCkbSrcDC         As Long
Private m_hOpbSrcDC         As Long
Private m_hHdbSrcDC         As Long
Private m_bTrackHandler32   As Boolean
Private m_SubclassCount     As Long                                             '       ໯ ĸ      Ա          д  ںͰ ť֮      ͷ   Դ

Public Function Attach(ByVal hwnd As Long) As Long
    If m_Init = False Then                                                      '   û г ʼ       ʼ  
        m_Init = True
        m_bTrackHandler32 = IsFunctionSupported("TrackMouseEvent", "User32")
        pInit
    End If
    Attach = pAttach(hwnd)
End Function

Public Function Detach(ByVal hwnd As Long) As Long
    Detach = pDetach(hwnd)
End Function

Private Function pAttach(ByVal hwnd As Long) As Long
    If hwnd = 0 Then Exit Function
    If GetProp(hwnd, "PROCADDR") Then Exit Function
    Dim sClassName  As String
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "#32770", "thunderformdc", "thunderrt6formdc", "form"
        EnumChildWindows(hwnd, AddressOf pEnumChildProc, ByVal 0&)
        
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        Dim i           As Long
        Dim m_hDC       As Long
        Dim m_mDC(3)    As Long
        Dim m_BMP(3)    As Long
        Dim m_wRect     As RECTW
        Dim m_dwStyle   As Long
        m_hDC = GetWindowDC(hwnd)
        pGetWindowRectW hwnd, m_wRect
        For i = 0 To 3
            m_mDC(i) = CreateCompatibleDC(m_hDC)
            m_BMP(i) = CreateCompatibleBitmap(m_hDC, m_wRect.Width, m_wRect.Height)
            DeleteObject SelectObject(m_mDC(i), m_BMP(i))
            SetProp hwnd, "HDC" & CStr(i), m_mDC(i)
            SetProp hwnd, "BMP" & CStr(i), m_BMP(i)
        Next
        pDrawMemDC(hwnd)
        ReleaseDC hwnd, m_hDC
        m_dwStyle = GetWindowLong(hwnd, GWL_STYLE)
        If (m_dwStyle And BS_CHECKBOX) Or (m_dwStyle And BS_RADIOBUTTON) Then
        Else
            SendMessage hwnd, BM_SETSTYLE, BS_OWNERDRAW, ByVal True
        End If
        SetProp hwnd, "OLDSTYLE", m_dwStyle                                     '   水ť ɵķ  , Ա   ȡ  Ƥ   ʱ  ָ ԭ   ķ  
        SetProp hwnd, "MOUSEFLAG", 0
        SetProp hwnd, "TIMERID", 0
        SetProp hwnd, "OLDSTATE", IIf(IsWindowEnabled(hwnd), 0, 3)
        SetProp hwnd, "ALPHALEVEL", 0
        SetWindowRgn hwnd, CreateRoundRectRgn(0, 0, m_wRect.Width + 1, m_wRect.Height + 1, 3, 3), True
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox", "thunderdrivelistbox", "thunderrt6drivelistbox", _
        "thundercheckbox", "thunderrt6checkbox", "thunderoptionbutton", "thunderrt6optionbutton"
        SetProp hwnd, "MOUSEFLAG", 0
        SetProp hwnd, "OLDSTATE", 0
        
        '=====================================================================================
    Case "progressbar20wndclass", "progressbarwndclass"
        'pGetWindowRectW(hWnd, m_wRect)
        'SetWindowRgn hWnd, CreateRoundRectRgn(0, 0, m_wRect.Width + 1, m_wRect.Height + 1, 3, 3), False
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        SetProp hwnd, "MOUSEFLAG", 0
        SetProp hwnd, "HDINDEX", -1
        SetProp hwnd, "HMINDEX", -1
        
        '=====================================================================================
    Case Else
        
    End Select
    m_SubclassCount = m_SubclassCount + 1
    SetProp hwnd, "PROCADDR", SetWindowLong(hwnd, GWL_WNDPROC, AddressOf WindowProc)
    SendMessage hwnd, WM_NCPAINT, 1&, 0&
    RedrawWindow hwnd, ByVal 0&, ByVal 0&, &H1 Or &H2
    pAttach = 1
End Function

Private Function pDetach(ByVal hwnd As Long) As Long
    If hwnd = 0 Then Exit Function
    Dim OrigProc As Long
    OrigProc = GetProp(hwnd, "PROCADDR")
    If OrigProc = 0 Then Exit Function
    Dim sClassName  As String
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "#32770", "thunderformdc", "thunderrt6formdc", "form"
        EnumChildWindows(hwnd, AddressOf pDeEnumChildProc, ByVal 0&)
        
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        Dim m_mDC(3)    As Long
        Dim m_BMP(3)    As Long
        Dim i As Long
        For i = 0 To 3
            m_mDC(i) = GetProp(hwnd, "HDC" & CStr(i))
            m_BMP(i) = GetProp(hwnd, "BMP" & CStr(i))
            DeleteObject m_mDC(i)
            DeleteDC m_BMP(i)
            RemoveProp hwnd, "HDC" & CStr(i)
            RemoveProp hwnd, "BMP" & CStr(i)
        Next
        pKillTimer(hwnd)
        SetWindowLong hwnd, -16, GetProp(hwnd, "OLDSTYLE")
        RemoveProp hwnd, "OLDSTYLE"
        RemoveProp hwnd, "MOUSEFLAG"
        RemoveProp hwnd, "TIMERID"
        RemoveProp hwnd, "OLDSTATE"
        RemoveProp hwnd, "ALPHALEVEL"
        SetWindowRgn hwnd, 0&, ByVal True
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox", "thunderdrivelistbox", "thunderrt6drivelistbox", _
        "thundercheckbox", "thunderrt6checkbox", "thunderoptionbutton", "thunderrt6optionbutton"
        RemoveProp hwnd, "MOUSEFLAG"
        RemoveProp hwnd, "OLDSTATE"
        
    Case "msvb_lib_header", "sysheader32"
        RemoveProp hwnd, "MOUSEFLAG"
        RemoveProp hwnd, "HDINDEX"
        RemoveProp hwnd, "HMINDEX"
        
        '=====================================================================================
    Case "progressbar20wndclass", "progressbarwndclass"
        'SetWindowRgn hWnd, 0&, ByVal True
        
        '=====================================================================================
    Case "datalistwndclass", "dblistwndclass"
        
        '=====================================================================================
    Case Else
        
    End Select
    RemoveProp hwnd, "PROCADDR"
    SetWindowLong(hwnd, GWL_WNDPROC, OrigProc)
    SendMessage hwnd, WM_NCPAINT, 1&, 0&
    RedrawWindow hwnd, ByVal 0&, ByVal 0&, &H1 Or &H2
    m_SubclassCount = m_SubclassCount - 1
    If m_SubclassCount <= 0 Then
        m_SubclassCount = 0
        DeleteDC m_hBtnSrcDC
        DeleteDC m_hCbbSrcDC
        DeleteDC m_hCkbSrcDC
        DeleteDC m_hOpbSrcDC
        DeleteDC m_hHdbSrcDC
        m_Init = False
    End If
    pDetach = 1
End Function

Private Sub pDrawComboBox(ByVal hwnd As Long, ByVal hDC As Long, State As Long, Optional ByVal Redraw As Boolean = False)
    Dim mOldState As Long
    Dim bDrop     As Long
    bDrop = SendMessage(hwnd, CB_GETDROPPEDSTATE, 0&, 0&)
    mOldState = GetProp(hwnd, "OLDSTATE")
    If bDrop Then State = 2
    If mOldState = State And Redraw = False Then Exit Sub
    If Not GetWindowLong(hwnd, GWL_STYLE) And &H2 Then Exit Sub
    SetProp(hwnd, "OLDSTATE", State)
    Dim m_BtSize    As Long
    Dim m_hDC       As Long
    Dim TmpDC       As Long
    Dim TmpBMP      As Long
    Dim m_wRect     As RECTW
    pGetWindowRectW(hwnd, m_wRect)
    m_BtSize = GetSystemMetrics(SM_CXVSCROLL) + 1
    TmpDC = pCreateDC(m_BtSize, m_wRect.Height - 2)
    Select Case State
    Case 0
        pFillRectL(TmpDC, 0, 0, m_BtSize, m_wRect.Height - 2, &HFFFFFF)
        
    Case 1
        GridBlt(TmpDC, 0, 0, m_BtSize, m_wRect.Height - 2, m_hCbbSrcDC, 0, 0, 4, 18, 2, 1, 1, 1)
        
    Case 2
        GridBlt(TmpDC, 0, 0, m_BtSize, m_wRect.Height - 2, m_hCbbSrcDC, 4, 0, 4, 18, 2, 1, 1, 1)
        
    End Select
    If IsWindowEnabled(hwnd) Then
        TransBlt(TmpDC, m_BtSize - 7 - (m_BtSize - 7) / 2, (m_wRect.Height - 6) / 2, 7, 4, m_hCbbSrcDC, 8, 0)
    Else
        TransBlt(TmpDC, m_BtSize - 7 - (m_BtSize - 7) / 2, (m_wRect.Height - 6) / 2, 7, 4, m_hCbbSrcDC, 8, 4)
    End If
    If hDC = 0 Then
        m_hDC = GetWindowDC(hwnd)
    Else
        m_hDC = hDC
    End If
    BitBlt m_hDC, m_wRect.Width - m_BtSize - 1, 1, m_BtSize, m_wRect.Height - 2, TmpDC, 0, 0, vbSrcCopy
    DeleteDC TmpDC
    DeleteObject TmpBMP
    If hDC = 0 Then ReleaseDC(hwnd, m_hDC)
End Sub

Private Function pDrawButton(ByVal hwnd As Long, ByVal hDC As Long) As Long
    Dim m_Style As Long
    Dim m_State As Long
    Dim m_OldSt As Long
    Dim m_SrcDC As Long
    Dim m_DstDC As Long
    Dim m_Level As Long
    Dim m_wRect As RECTW
    If IsWindowEnabled(hwnd) = 0 Then SetProp(hwnd, "OLDSTATE", 3)
    m_Style = GetProp(hwnd, "OLDSTYLE")
    If (m_Style And BS_CHECKBOX) Or (m_Style And BS_RADIOBUTTON) Then Exit Function
    pGetWindowRectW(hwnd, m_wRect)
    m_OldSt = GetProp(hwnd, "OLDSTATE")
    m_Level = GetProp(hwnd, "ALPHALEVEL")
    m_SrcDC = GetProp(hwnd, "HDC" & CStr(m_OldSt))
    m_DstDC = IIf(hDC = 0, GetWindowDC(hwnd), hDC)
    AlphaBlend m_DstDC, 0, 0, m_wRect.Width, m_wRect.Height, m_SrcDC, 0, 0, m_wRect.Width, m_wRect.Height, m_Level * &H10000
    If hDC = 0 Then ReleaseDC(hwnd, m_DstDC)
End Function

Private Function pDrawCheckBox(ByVal hwnd As Long, ByVal State As Long, Optional ByVal Redraw As Boolean = False) As Long
    Dim mOldState As Long
    mOldState = GetProp(hwnd, "OLDSTATE")
    If mOldState = State And Redraw = False Then Exit Function
    SetProp(hwnd, "OLDSTATE", State)
    Dim m_hDC       As Long
    Dim TmpDC       As Long
    Dim m_wRect     As RECTW
    Dim m_cX        As Long
    Dim m_cY        As Long
    Dim mValue      As Long
    m_cX = GetSystemMetrics(SM_CXCHECKBOX)
    m_cY = GetSystemMetrics(SM_CYCHECKBOX)
    pGetWindowRectW(hwnd, m_wRect)
    mValue = SendMessage(hwnd, BM_GETCHECK, 0&, 0&)
    TmpDC = pCreateDC(m_cX, m_cY)
    m_hDC = GetWindowDC(hwnd)
    pFillRectL(TmpDC, 0, 0, m_cX, m_cY, &HFFFFFF)
    If IsWindowEnabled(hwnd) Then
        If State = 2 Then
            pFrameRect(TmpDC, 0, 0, m_cX, m_cY, &HC48639)
        Else
            pFrameRect(TmpDC, 0, 0, m_cX, m_cY, &HD5A554)
        End If
        If State = 1 Then StretchBlt(TmpDC, 1, 1, m_cX - 2, m_cY - 2, m_hOpbSrcDC, 1, 17, 11, 5, vbSrcCopy)
        If State = 2 Then StretchBlt(TmpDC, 1, 1, m_cX - 2, m_cY - 2, m_hOpbSrcDC, 1, 30, 11, 5, vbSrcCopy)
        If mValue = 1 Then TransBlt(TmpDC, (m_cX - 9) / 2, (m_cY - 8) / 2, 9, 8, m_hCkbSrcDC, 0, 0)
        If mValue = 2 Then TransBlt(TmpDC, (m_cX - 7) / 2, (m_cY - 7) / 2, 7, 7, m_hCkbSrcDC, 1, 9)
    Else
        pFrameRect(TmpDC, 0, 0, m_cX, m_cY, &HE9CFA4)
        If mValue = 1 Then TransBlt(TmpDC, (m_cX - 9) / 2, (m_cY - 8) / 2, 9, 8, m_hCkbSrcDC, 9, 0)
        If mValue = 2 Then TransBlt(TmpDC, (m_cX - 7) / 2, (m_cY - 7) / 2, 7, 7, m_hCkbSrcDC, 10, 9)
    End If
    BitBlt m_hDC, 0, (m_wRect.Height - m_cY) / 2, m_cX, m_cY, TmpDC, 0, 0, vbSrcCopy
    ReleaseDC(hwnd, m_hDC)
    DeleteDC TmpDC
    pDrawCheckBox = 1
End Function

Private Function pDrawRadioBox(ByVal hwnd As Long, ByVal State As Long, Optional ByVal Redraw As Boolean = False) As Long
    Dim mOldState As Long
    mOldState = GetProp(hwnd, "OLDSTATE")
    If mOldState = State And Redraw = False Then Exit Function
    SetProp(hwnd, "OLDSTATE", State)
    Dim m_hDC       As Long
    Dim TmpDC       As Long
    Dim m_wRect     As RECTW
    Dim m_cX        As Long
    Dim m_cY        As Long
    Dim m_dX        As Long
    Dim m_dY        As Long
    Dim mValue      As Long
    pGetWindowRectW(hwnd, m_wRect)
    m_cX = GetSystemMetrics(SM_CXCHECKBOX)
    m_cY = GetSystemMetrics(SM_CYCHECKBOX)
    m_dX = 0
    m_dY = (m_wRect.Height - m_cY) / 2
    mValue = SendMessage(hwnd, BM_GETCHECK, 0&, 0&)
    m_hDC = GetWindowDC(hwnd)
    pFillRectL(TmpDC, 0, 0, m_cX, m_cY, &HFFFFFF)
    If IsWindowEnabled(hwnd) Then
        Select Case State
        Case 0
            GridBlt(m_hDC, m_dX, m_dY, m_cX, m_cY, m_hOpbSrcDC, 0, 0, 13, 13, 5, 5, 5, 5, RGB(255, 0, 255))
            
        Case 1
            GridBlt(m_hDC, m_dX, m_dY, m_cX, m_cY, m_hOpbSrcDC, 0, 13, 13, 13, 5, 5, 5, 5, RGB(255, 0, 255))
            
        Case 2
            GridBlt(m_hDC, m_dX, m_dY, m_cX, m_cY, m_hOpbSrcDC, 0, 26, 13, 13, 5, 5, 5, 5, RGB(255, 0, 255))
            
        End Select
        If mValue = 1 Then TransBlt(m_hDC, m_dX + (m_cX - 5) / 2, m_dY + (m_cY - 5) / 2, 5, 5, m_hCkbSrcDC, 2, 18)
    Else
        GridBlt(m_hDC, m_dX, m_dY, m_cX, m_cY, m_hOpbSrcDC, 0, 39, 13, 13, 5, 5, 5, 5, RGB(255, 0, 255))
        If mValue = 1 Then TransBlt(m_hDC, m_dX + (m_cX - 5) / 2, m_dY + (m_cY - 5) / 2, 5, 5, m_hCkbSrcDC, 11, 18)
    End If
    ReleaseDC(hwnd, m_hDC)
    DeleteDC TmpDC
    pDrawRadioBox = 1
End Function

Private Function pDrawHeader(ByVal hwnd As Long, ByVal hDC As Long, ByVal Index As Long, ByVal State As Long) As Long
    If Index < 0 Then Exit Function
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    Dim m_iRect     As RECT
    Dim m_iWidth    As Long
    Dim m_iHeight   As Long
    Dim TmpDC       As Long
    Dim m_iText     As String
    Dim m_iItem     As HDITEM
    pGetWindowRectW(hwnd, m_wRect)
    SendMessage(hwnd, HDM_GETITEMRECT, Index, m_iRect)
    m_iWidth = m_iRect.Right - m_iRect.Left
    m_iHeight = m_iRect.Bottom - m_iRect.Top
    TmpDC = pCreateDC(m_iWidth, m_wRect.Height)
    SelectObject TmpDC, SendMessage(hwnd, WM_GETFONT, 0, 0)
    SetBkMode TmpDC, 1
    Select Case State
    Case 1
        GridBlt(TmpDC, 0, 0, m_iWidth, m_wRect.Height, m_hHdbSrcDC, 3, 0, 3, 20, 1, 1, 1, 1)
        
    Case 2
        GridBlt(TmpDC, 0, 0, m_iWidth, m_wRect.Height, m_hHdbSrcDC, 6, 0, 3, 20, 1, 1, 1, 1)
        
    Case Else
        GridBlt(TmpDC, 0, 0, m_iWidth, m_wRect.Height, m_hHdbSrcDC, 0, 0, 3, 20, 1, 1, 1, 1)
        
    End Select
    
    With m_iItem
        .mask = HDI_TEXT
        .cchTextMax = 256
        .pszText = String$(255, 0)
    End With
    SendMessage(hwnd, HDM_GETITEMA, Index, m_iItem)
    m_iText = Replace$(m_iItem.pszText, Chr(0), vbNullString)
    If Len(m_iText) > 0 Then
        pDrawTextL(TmpDC, m_iText, 0, 0, m_iWidth, m_wRect.Height, DT_CENTER Or DT_VCENTER Or DT_SINGLELINE Or DT_END_ELLIPSIS)
    End If
    m_hDC = IIf(hDC = 0, GetWindowDC(hwnd), hDC)
    BitBlt m_hDC, m_iRect.Left, 0, m_iWidth, m_iHeight, TmpDC, 0, 0, vbSrcCopy
    If hDC = 0 Then ReleaseDC(hwnd, m_hDC)
    DeleteDC TmpDC
    pDrawHeader = 0
End Function

Private Function OnActivate(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    If lParam Then
        If GetParent(lParam) = hwnd Then
            pAttach lParam
        Else
            sClassName = LCase(pGetClassName(lParam))
            Select Case sClassName
                '=====================================================================================
            Case "#32770", "thunderformdc", "thunderrt6formdc", "form"
                pAttach lParam
                
            End Select
        End If
    End If
    OnActivate = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnPaint(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim TmpDC       As Long
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        Dim m_Style As Long
        m_Style = GetProp(hwnd, "OLDSTYLE")
        If (m_Style And BS_CHECKBOX) Or (m_Style And BS_RADIOBUTTON) Then
            OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Else
            Dim ps As PAINTSTRUCT
            pKillTimer(hwnd)
            SetProp(hwnd, "ALPHALEVEL", 255)
            BeginPaint(hwnd, ps)
            pDrawButton(hwnd, ps.hDC)
            EndPaint(hwnd, ps)
            OnPaint = False
        End If
        Exit Function
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox"
        Dim m_BtSize  As Long
        Dim mOldState As Long
        m_BtSize = GetSystemMetrics(SM_CXVSCROLL)
        mOldState = GetProp(hwnd, "OLDSTATE")
        pGetWindowRectW(hwnd, m_wRect)
        If GetWindowLong(hwnd, GWL_STYLE) And &H1 Then
            OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
            m_hDC = GetWindowDC(hwnd)
            If IsWindowEnabled(hwnd) Then
                pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
            Else
                pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
            End If
            pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HFFFFFF)
            pDrawComboBox(hwnd, m_hDC, mOldState, True)
            ReleaseDC(hwnd, m_hDC)
        Else
            OnPaint = False
            BeginPaint(hwnd, ps)
            pFillRectL(ps.hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HFFFFFF)
            If IsWindowEnabled(hwnd) Then
                pFrameRect(ps.hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
            Else
                pFrameRect(ps.hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
            End If
            pDrawComboBox(hwnd, ps.hDC, mOldState, True)
            EndPaint(hwnd, ps)
        End If
        Exit Function
        
    Case "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        m_BtSize = GetSystemMetrics(SM_CXVSCROLL)
        mOldState = GetProp(hwnd, "OLDSTATE")
        pGetWindowRectW(hwnd, m_wRect)
        m_hDC = GetWindowDC(hwnd)
        If IsWindowEnabled(hwnd) Then
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
        Else
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
        End If
        pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HFFFFFF)
        pDrawComboBox(hwnd, m_hDC, mOldState, True)
        ReleaseDC(hwnd, m_hDC)
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawCheckBox(hwnd, 0, True)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawRadioBox(hwnd, 0, True)
        Exit Function
        
        '=====================================================================================
    Case "datalistwndclass", "dblistwndclass"
        OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pGetWindowRectW(hwnd, m_wRect)
        m_hDC = GetWindowDC(hwnd)
        If IsWindowEnabled(hwnd) Then
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
        Else
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
        End If
        pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HFFFFFF)
        ReleaseDC(hwnd, m_hDC)
        Exit Function
        
    Case "combolbox"
        OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pGetWindowRectW(hwnd, m_wRect)
        m_hDC = GetWindowDC(hwnd)
        If IsWindowEnabled(hwnd) Then
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
        Else
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
        End If
        pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HFFFFFF)
        ReleaseDC(hwnd, m_hDC)
        Exit Function
        
        '=====================================================================================
    Case "progressbar20wndclass", "progressbarwndclass"
        OnPaint = False
        Dim dwStyle As Long
        Dim lPos    As Single
        Dim lW      As Long
        Dim lH      As Long
        Dim c_wRect As RECT
        dwStyle = GetWindowLong(hwnd, GWL_STYLE)
        GetClientRect(hwnd, c_wRect)
        lW = c_wRect.Right - c_wRect.Left
        lH = c_wRect.Bottom - c_wRect.Top
        TmpDC = pCreateDC(lW, lH)
        lPos = SendMessage(hwnd, PBM_GETPOS, 0, 0) / &H7530
        pFillRectL(TmpDC, 0, 0, lW, lH, &HFFFFFF)
        If lPos Then
            If dwStyle And PBS_VERTICAL Then
                pFillRectL(TmpDC, 0, lH - lH * lPos, lW, lH * lPos, &HF6D64C)
            Else
                pFillRectL(TmpDC, 0, 0, lW * lPos, lH, &HF6D64C)
            End If
        End If
        BeginPaint(hwnd, ps)
        BitBlt ps.hDC, 0, 0, lW, lH, TmpDC, 0, 0, vbSrcCopy
        EndPaint(hwnd, ps)
        DeleteDC TmpDC
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnPaint = False
        Dim m_iCount As Long
        pGetWindowRectW(hwnd, m_wRect)
        TmpDC = pCreateDC(m_wRect.Width, m_wRect.Height)
        StretchBlt TmpDC, 0, 0, m_wRect.Width, m_wRect.Height, m_hHdbSrcDC, 1, 0, 1, 20, vbSrcCopy
        m_iCount = SendMessage(hwnd, HDM_GETITEMCOUNT, 0&, 0&)
        If m_iCount > 0 Then
            Dim m_iIndex As Long
            Dim m_mIndex As Long
            Dim i As Long
            m_iIndex = GetProp(hwnd, "HDINDEX")
            m_mIndex = GetProp(hwnd, "HMINDEX")
            For i = 0 To m_iCount - 1
                Dim m_iText As String
                Dim m_iRect As RECT
                Dim m_iItem  As HDITEM
                With m_iItem
                    .mask = HDI_TEXT
                    .cchTextMax = 256
                    .pszText = String$(255, 0)
                End With
                SendMessage(hwnd, HDM_GETITEMRECT, i, m_iRect)
                SendMessage(hwnd, HDM_GETITEMA, i, m_iItem)
                m_iText = Replace$(m_iItem.pszText, Chr(0), vbNullString)
                If i = m_iIndex Then
                    GridBlt(TmpDC, m_iRect.Left, 0, m_iRect.Right - m_iRect.Left, m_wRect.Height, m_hHdbSrcDC, 6, 0, 3, 20, 1, 1, 1, 1)
                ElseIf i = m_mIndex Then
                    GridBlt(TmpDC, m_iRect.Left, 0, m_iRect.Right - m_iRect.Left, m_wRect.Height, m_hHdbSrcDC, 3, 0, 3, 20, 1, 1, 1, 1)
                Else
                    GridBlt(TmpDC, m_iRect.Left, 0, m_iRect.Right - m_iRect.Left, m_wRect.Height, m_hHdbSrcDC, 0, 0, 3, 20, 1, 1, 1, 1)
                End If
                If Len(m_iText) > 0 Then
                    SelectObject TmpDC, SendMessage(hwnd, WM_GETFONT, 0, 0)
                    SetBkMode TmpDC, 1
                    pDrawTextL(TmpDC, m_iText, m_iRect.Left, 0, m_iRect.Right - m_iRect.Left, m_iRect.Bottom, DT_CENTER Or DT_VCENTER Or DT_SINGLELINE Or DT_END_ELLIPSIS)
                End If
            Next
        End If
        BeginPaint(hwnd, ps)
        BitBlt ps.hDC, 0, 0, m_wRect.Width, m_wRect.Height, TmpDC, 0, 0, vbSrcCopy
        EndPaint(hwnd, ps)
        Exit Function
        
    Case Else
        If sClassName Like "apexgrid*" Then
            OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
            dwStyle = GetWindowLong(hwnd, GWL_STYLE)
            If dwStyle And &H800000 Then
                pGetWindowRectW(hwnd, m_wRect)
                m_hDC = GetWindowDC(hwnd)
                If IsWindowEnabled(hwnd) Then
                    pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
                Else
                    pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
                End If
                ReleaseDC(hwnd, m_hDC)
            End If
            Exit Function
        End If
        
    End Select
    OnPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnNcPaint(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    Dim TmpDC       As Long
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    Dim cX          As Long
    Dim cY          As Long
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "progressbar20wndclass", "progressbarwndclass"
        Dim dwStyle As Long
        OnNcPaint = False
        Dim lPos    As Single
        Dim lW      As Long
        Dim lH      As Long
        pGetBorderSize(hwnd, cX, cY)
        Dim c_wRect As RECT
        pGetWindowRectW(hwnd, m_wRect)
        GetClientRect(hwnd, c_wRect)
        pGetBorderSize(hwnd, cX, cY)
        lW = c_wRect.Right - c_wRect.Left
        lH = c_wRect.Bottom - c_wRect.Top
        TmpDC = pCreateDC(m_wRect.Width, m_wRect.Height)
        lPos = SendMessage(hwnd, PBM_GETPOS, 0, 0) / &H7530
        dwStyle = GetWindowLong(hwnd, GWL_STYLE)
        pFillRectL(TmpDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
        pFillRectL(TmpDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HFFFFFF)
        If lPos Then
            If dwStyle And PBS_VERTICAL Then
                If dwStyle And &H1 Then pFillRectL(TmpDC, cX, cY, lW, lH, &HF6D64C)
                pFillRectL(TmpDC, cX, cY, lW, lH - lH * lPos, &HFFFFFF)
            Else
                pFillRectL(TmpDC, cX, cY, lW * lPos, lH, &HF6D64C)
            End If
        End If
        m_hDC = GetWindowDC(hwnd)
        BitBlt m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, TmpDC, 0, 0, vbSrcCopy
        ReleaseDC(hwnd, m_hDC)
        DeleteDC TmpDC
        Exit Function
        '=====================================================================================
    Case "datalistwndclass", "dblistwndclass"
        
        '=====================================================================================
    Case Else
        OnNcPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetWindowLong(hwnd, GWL_EXSTYLE) And WS_EX_CLIENTEDGE Then
            Dim m_cDC       As Long
            Dim m_Width     As Long
            Dim m_Height    As Long
            Dim i           As Long
            pGetBorderSize(hwnd, cX, cY)
            pGetWindowRectW(hwnd, m_wRect)
            m_hDC = GetWindowDC(hwnd)
            m_cDC = GetDC(hwnd)
            If IsWindowEnabled(hwnd) Then
                pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
            Else
                pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HE9CFA4)
            End If
            If cX > 0 Then
                For i = 1 To cX
                    pFrameRect(m_hDC, i, i, m_wRect.Width - 1 - i, m_wRect.Height - 1 - i, GetBkColor(m_cDC))
                Next
            End If
            ReleaseDC hwnd, m_cDC
            ReleaseDC hwnd, m_hDC
        End If
        Exit Function
        
    End Select
    OnNcPaint = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnSize(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnSize = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim i As Long
        Dim m_mDC(3)    As Long
        Dim m_BMP(3)    As Long
        m_hDC = GetWindowDC(hwnd)
        pGetWindowRectW(hwnd, m_wRect)
        For i = 0 To 3
            m_mDC(i) = GetProp(hwnd, "HDC" & CStr(i))
            m_BMP(i) = CreateCompatibleBitmap(m_hDC, m_wRect.Width, m_wRect.Height)
            DeleteObject SelectObject(m_mDC(i), m_BMP(i))
        Next
        pDrawMemDC(hwnd)
        ReleaseDC hwnd, m_hDC
        SetWindowRgn hwnd, CreateRoundRectRgn(0, 0, m_wRect.Width + 1, m_wRect.Height + 1, 3, 3), True
        Exit Function
        
        '=====================================================================================
    Case "progressbar20wndclass", "progressbarwndclass"
        'OnSize = CallWindowProc(OrigProc, hWnd, uMsg, wParam, lParam)
        'pGetWindowRectW(hWnd, m_wRect)
        'SetWindowRgn hWnd, CreateRoundRectRgn(0, 0, m_wRect.Width + 1, m_wRect.Height + 1, 8, 8), True
        'Exit Function
        
        '=====================================================================================
    Case Else
        
    End Select
    OnSize = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnLButtonDBClick(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnLButtonDBClick = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pKillTimer(hwnd)
        SetProp(hwnd, "OLDSTATE", 2)
        SetProp(hwnd, "ALPHALEVEL", 255)
        pDrawButton(hwnd, 0)
        SetProp(hwnd, "ALPHALEVEL", 0)
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnLButtonDBClick = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim m_iCount As Long
        m_iCount = SendMessage(hwnd, HDM_GETITEMCOUNT, 0&, 0&)
        If m_iCount > 0 Then
            Dim iIndex  As Long
            Dim mX      As Long
            Dim i       As Long
            mX = LWORD(lParam)
            iIndex = -1
            For i = 0 To m_iCount - 1
                Dim m_iRect As RECT
                SendMessage(hwnd, HDM_GETITEMRECT, i, m_iRect)
                If mX >= m_iRect.Left And mX <= m_iRect.Right Then
                    iIndex = i
                    Exit For
                End If
            Next
            SetProp hwnd, "HDINDEX", iIndex
            If iIndex >= 0 Then pDrawHeader(hwnd, 0, iIndex, 2)
        End If
        Exit Function
        
    End Select
    OnLButtonDBClick = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnLButtonDown(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pKillTimer(hwnd)
        SetProp(hwnd, "OLDSTATE", 2)
        SetProp(hwnd, "ALPHALEVEL", 255)
        pDrawButton(hwnd, 0)
        SetProp(hwnd, "ALPHALEVEL", 0)
        Exit Function
        '=====================================================================================
    Case "#32770", "thunderformdc", "thunderrt6formdc", "form"
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim mX          As Long
        Dim mY          As Long
        Dim m_BtSize    As Long
        mX = LWORD(lParam)
        mY = HWORD(lParam)
        pGetWindowRectW(hwnd, m_wRect)
        m_BtSize = GetSystemMetrics(SM_CXVSCROLL)
        If GetWindowLong(hwnd, GWL_STYLE) And &H1 Then
            pDrawComboBox hwnd, 0, 2
        ElseIf mX >= m_wRect.Width - m_BtSize - 2 And mX <= m_wRect.Width - 2 And mY >= 2 And mY <= m_wRect.Height - 2 Then
            pDrawComboBox hwnd, 0, 2
        End If
        Exit Function
        
        '=====================================================================================
    Case "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawComboBox hwnd, 0, 2
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawCheckBox(hwnd, 2)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawRadioBox(hwnd, 2)
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim m_iCount As Long
        m_iCount = SendMessage(hwnd, HDM_GETITEMCOUNT, 0&, 0&)
        If m_iCount > 0 Then
            Dim iIndex  As Long
            Dim i       As Long
            mX = LWORD(lParam)
            iIndex = -1
            For i = 0 To m_iCount - 1
                Dim m_iRect As RECT
                SendMessage(hwnd, HDM_GETITEMRECT, i, m_iRect)
                If mX >= m_iRect.Left And mX <= m_iRect.Right Then
                    iIndex = i
                    Exit For
                End If
            Next
            SetProp hwnd, "HDINDEX", iIndex
            If iIndex >= 0 Then pDrawHeader(hwnd, 0, iIndex, 2)
        End If
        Exit Function
        
    End Select
    OnLButtonDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnLButtonUp(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        SetProp(hwnd, "MOUSEFLAG", 0)
        SetProp(hwnd, "OLDSTATE", 0)
        SetProp(hwnd, "ALPHALEVEL", 60)
        pSetTimer(hwnd)
        Exit Function
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim Pt As POINTAPI
        GetCursorPos(Pt)
        If WindowFromPoint(Pt.X, Pt.Y) = hwnd Then
            pDrawComboBox hwnd, 0, 1
        Else
            pDrawComboBox hwnd, 0, 0
        End If
        Exit Function
        
        '=====================================================================================
    Case "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        GetCursorPos(Pt)
        If WindowFromPoint(Pt.X, Pt.Y) = hwnd Then
            pDrawComboBox hwnd, 0, 1
        Else
            pDrawComboBox hwnd, 0, 0
        End If
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawCheckBox(hwnd, 0)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawRadioBox(hwnd, 0)
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim m_oIndex As Long
        m_oIndex = GetProp(hwnd, "HDINDEX")
        SetProp hwnd, "HDINDEX", -1
        If m_oIndex >= 0 Then
            GetCursorPos(Pt)
            If WindowFromPoint(Pt.X, Pt.Y) = hwnd Then
                pDrawHeader(hwnd, 0, m_oIndex, 0)
            Else
                pDrawHeader(hwnd, 0, m_oIndex, 1)
            End If
        End If
        Exit Function
        
    End Select
    OnLButtonUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnKeyDown(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnKeyDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam = 32 Then
            pKillTimer(hwnd)
            SetProp(hwnd, "OLDSTATE", 2)
            SetProp(hwnd, "ALPHALEVEL", 255)
            pDrawButton(hwnd, 0)
            SetProp(hwnd, "ALPHALEVEL", 0)
        End If
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnKeyDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam = 32 Then pDrawCheckBox(hwnd, 2)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnKeyDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam = 32 Then pDrawRadioBox(hwnd, 2)
        Exit Function
        
    End Select
    OnKeyDown = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnKeyUp(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        If wParam = 32 Then
            OnKeyUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
            SetProp(hwnd, "MOUSEFLAG", 0)
            SetProp(hwnd, "ALPHALEVEL", 0)
            SetProp(hwnd, "OLDSTATE", 0)
            pSetTimer(hwnd)
            Exit Function
        End If
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnKeyUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam = 32 Then pDrawCheckBox(hwnd, 0)
        Exit Function
        
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnKeyUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam = 32 Then pDrawRadioBox(hwnd, 0)
        Exit Function
        
    End Select
    OnKeyUp = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnMouseMove(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
            SetProp(hwnd, "OLDSTATE", 1)
            SetProp(hwnd, "ALPHALEVEL", 0)
            pSetTimer(hwnd)
        Else
            Dim Pt As POINTAPI
            GetCursorPos(Pt)
            If Not WindowFromPoint(Pt.X, Pt.Y) = hwnd Then
                pKillTimer(hwnd)
                SetProp(hwnd, "ALPHALEVEL", 255)
                pDrawButton(hwnd, 0)
            End If
        End If
        Exit Function
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim mX          As Long
        Dim mY          As Long
        Dim m_BtSize    As Long
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
        End If
        mX = LWORD(lParam)
        mY = HWORD(lParam)
        pGetWindowRectW(hwnd, m_wRect)
        m_BtSize = GetSystemMetrics(SM_CXVSCROLL)
        If GetWindowLong(hwnd, GWL_STYLE) And &H1 Then
            If wParam And MK_LBUTTON Then
                pDrawComboBox hwnd, 0, 2
            Else
                pDrawComboBox hwnd, 0, 1
            End If
        ElseIf mX >= m_wRect.Width - m_BtSize - 2 And mX <= m_wRect.Width - 2 And mY >= 2 And mY <= m_wRect.Height - 2 Then
            If wParam And MK_LBUTTON Then
                pDrawComboBox hwnd, 0, 2
            Else
                pDrawComboBox hwnd, 0, 1
            End If
        Else
            pDrawComboBox hwnd, 0, 0
        End If
        Exit Function
        
        '=====================================================================================
    Case "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
        End If
        If wParam And MK_LBUTTON Then
            pDrawComboBox hwnd, 0, 2
        Else
            pDrawComboBox hwnd, 0, 1
        End If
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
        End If
        If wParam And MK_LBUTTON Then
            pDrawCheckBox(hwnd, 2, True)
        Else
            pDrawCheckBox(hwnd, 1, True)
        End If
        Exit Function
        
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
        End If
        If wParam And MK_LBUTTON Then
            pDrawRadioBox(hwnd, 2, True)
        Else
            pDrawRadioBox(hwnd, 1, True)
        End If
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If GetProp(hwnd, "MOUSEFLAG") = 0 Then
            SetProp(hwnd, "MOUSEFLAG", 1)
            pTrackMouseTracking(hwnd)
        End If
        Dim m_iCount As Long
        m_iCount = SendMessage(hwnd, HDM_GETITEMCOUNT, 0&, 0&)
        Dim m_iIndex  As Long
        Dim m_oIndex  As Long
        Dim m_iRect   As RECT
        mX = LWORD(lParam)
        m_iIndex = -1
        m_oIndex = GetProp(hwnd, "HMINDEX")
        If m_iCount > 0 Then
            Dim i       As Long
            For i = 0 To m_iCount - 1
                SendMessage(hwnd, HDM_GETITEMRECT, i, m_iRect)
                If mX >= m_iRect.Left And mX <= m_iRect.Right Then
                    m_iIndex = i
                    Exit For
                End If
            Next
        End If
        Dim m_dIndex As Long
        m_dIndex = GetProp(hwnd, "HDINDEX")
        If Not m_oIndex = m_iIndex Then
            If m_oIndex >= 0 And Not m_oIndex = m_dIndex Then pDrawHeader(hwnd, 0, m_oIndex, 0)
            If wParam And MK_LBUTTON Then
                If m_dIndex = m_iIndex Then pDrawHeader(hwnd, 0, m_iIndex, 2)
            Else
                SetProp hwnd, "HMINDEX", m_iIndex
                pDrawHeader(hwnd, 0, m_iIndex, 1)
            End If
        End If
        Exit Function
        
    End Select
    OnMouseMove = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnMouseLeave(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        SetProp(hwnd, "MOUSEFLAG", 0)
        SetProp(hwnd, "OLDSTATE", 1)
        SetProp(hwnd, "ALPHALEVEL", 140)
        pDrawButton(hwnd, 0)
        SetProp(hwnd, "OLDSTATE", 0)
        SetProp(hwnd, "ALPHALEVEL", 0)
        pSetTimer(hwnd)
        Exit Function
        
        '=====================================================================================
    Case "#32770", "thunderformdc", "thunderrt6formdc", "form"
        
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox", "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        SetProp(hwnd, "MOUSEFLAG", 0)
        pDrawComboBox hwnd, 0, 0
        Exit Function
        
        '=====================================================================================
    Case "thundercheckbox", "thunderrt6checkbox"
        OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        SetProp(hwnd, "MOUSEFLAG", 0)
        pDrawCheckBox(hwnd, 0)
        Exit Function
        
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        SetProp(hwnd, "MOUSEFLAG", 0)
        pDrawRadioBox(hwnd, 0)
        Exit Function
        
        '=====================================================================================
    Case "msvb_lib_header", "sysheader32"
        OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Dim m_oIndex As Long
        m_oIndex = GetProp(hwnd, "HMINDEX")
        SetProp(hwnd, "MOUSEFLAG", 0)
        SetProp(hwnd, "HDINDEX", -1)
        SetProp(hwnd, "HMINDEX", -1)
        If m_oIndex >= 0 Then pDrawHeader(hwnd, 0, m_oIndex, 0)
        Exit Function
        
    End Select
    OnMouseLeave = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnTimer(OrigProc As Long, hwnd As Long, uMsg As Long, wParam As Long, lParam As Long) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        Dim m_Level As Long
        m_Level = GetProp(hwnd, "ALPHALEVEL")
        pDrawButton(hwnd, 0)
        m_Level = m_Level + 3                                                   '     +3   ٶȣ    Ըı Timer  ʱ   +    ֵ Ըı  ٶ 
        If m_Level > 255 Then
            pKillTimer(hwnd)
            SetProp(hwnd, "ALPHALEVEL", 0)
        Else
            SetProp(hwnd, "ALPHALEVEL", m_Level)
        End If
        
    End Select
    OnTimer = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnEnable(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnEnable = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        If wParam Then
            SetProp(hwnd, "OLDSTATE", 0)
        Else
            SetProp(hwnd, "OLDSTATE", 3)
        End If
        pDrawButton(hwnd, 0)
        Exit Function
        
    End Select
    OnEnable = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnSetFocus(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnSetFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawMemDC(hwnd)
        SetProp(hwnd, "ALPHALEVEL", 0)
        pSetTimer(hwnd)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnSetFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawRadioBox(hwnd, 0, True)
        Exit Function
        
    End Select
    OnSetFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnKillFocus(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercommandbutton", "thunderrt6commandbutton", "button"
        OnKillFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawMemDC(hwnd)
        SetProp(hwnd, "ALPHALEVEL", 0)
        pSetTimer(hwnd)
        Exit Function
        
        '=====================================================================================
    Case "thunderoptionbutton", "thunderrt6optionbutton"
        OnKillFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawRadioBox(hwnd, 0, True)
        Exit Function
        
    End Select
    OnKillFocus = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function OnWindowposChanging(OrigProc, hwnd, uMsg, wParam, lParam) As Long
    Dim sClassName  As String
    Dim m_hDC       As Long
    Dim m_wRect     As RECTW
    sClassName = LCase(pGetClassName(hwnd))
    Select Case sClassName
        '=====================================================================================
    Case "thundercombobox", "thunderrt6combobox", "combo", "combobox", "thunderdrivelistbox", "thunderrt6drivelistbox"
        OnWindowposChanging = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        pDrawComboBox hwnd, 0, 0
        Exit Function
        
    End Select
    OnWindowposChanging = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
End Function

Private Function WindowProc(ByVal hwnd As Long, ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
    Dim OrigProc    As Long
    If hwnd = 0 Then Exit Function
    OrigProc = GetProp(hwnd, "PROCADDR")
    If Not OrigProc = 0 Then
        If uMsg = WM_DESTROY Then
            pDetach(hwnd)
            WindowProc = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
        Else
            Select Case uMsg
            Case WM_ACTIVATE
                WindowProc = OnActivate(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_PAINT
                WindowProc = OnPaint(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_NCPAINT
                WindowProc = OnNcPaint(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_SIZE
                WindowProc = OnSize(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_LBUTTONDBLCLK
                WindowProc = OnLButtonDBClick(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_LBUTTONDOWN
                WindowProc = OnLButtonDown(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_LBUTTONUP
                WindowProc = OnLButtonUp(OrigProc, hwnd, uMsg, wParam, lParam)
                Dim hParent As Long
                hParent = GetParent(hwnd)
                If hParent = 0 Then hParent = hwnd
                EnumChildWindows(hParent, AddressOf pEnumRedrawProc, 0&)
                
            Case WM_KEYDOWN
                WindowProc = OnKeyDown(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_KEYUP
                WindowProc = OnKeyUp(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_MOUSEMOVE
                WindowProc = OnMouseMove(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_MOUSELEAVE
                WindowProc = OnMouseLeave(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_TIMER
                WindowProc = OnTimer(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_ENABLE
                WindowProc = OnEnable(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_SETFOCUS
                WindowProc = OnSetFocus(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_KILLFOCUS
                WindowProc = OnKillFocus(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_WINDOWPOSCHANGING
                WindowProc = OnWindowposChanging(OrigProc, hwnd, uMsg, wParam, lParam)
                
            Case WM_CTLCOLORLISTBOX
                WindowProc = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
                If lParam Then pAttach (lParam)
                
            Case WM_ENTERIDLE
                WindowProc = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
                If lParam Then pAttach lParam
                
            Case Else
                'Debug.Print uMsg & "|" & wParam & "|" & lParam & "|" & Now
                WindowProc = CallWindowProc(OrigProc, hwnd, uMsg, wParam, lParam)
                
            End Select
        End If
    Else
        WindowProc = DefWindowProc(hwnd, uMsg, wParam, lParam)
    End If
End Function

Private Function pEnumChildProc(ByVal hwnd As Long, ByVal lParam As Long) As Long
    pAttach(hwnd)
    pEnumChildProc = 1
End Function

Private Function pEnumRedrawProc(ByVal hwnd As Long, ByVal lParam As Long) As Long
    If GetProp(hwnd, "PROCADDR") Then
        Dim sClassName As String
        sClassName = LCase(pGetClassName(hwnd))
        Select Case sClassName
        Case "thundercheckbox", "thunderrt6checkbox"
            pDrawCheckBox(hwnd, 0, True)
            
        Case "thunderoptionbutton", "thunderrt6optionbutton"
            pDrawRadioBox(hwnd, 0, True)
            
        End Select
    End If
    pEnumRedrawProc = 1
End Function

Private Function pDeEnumChildProc(ByVal hwnd As Long, ByVal lParam As Long) As Long
    pDetach(hwnd)
    pDeEnumChildProc = 1
End Function

Private Function pGetClassName(ByVal hwnd As Long) As String
    On Error Resume Next
    Dim BuffStr     As String
    Dim BuffStrLen  As Long
    Dim Rtn         As Long
    BuffStr = String$(255, Chr(0))
    BuffStrLen = Len(BuffStr)
    Rtn = GetClassName(hwnd, ByVal BuffStr, BuffStrLen)
    If Not Rtn = 0 Then
        Dim iPos As Long
        iPos = InStr(1, BuffStr, Chr(0)) - 1
        If iPos < Len(BuffStr) Then
            pGetClassName = Left$(BuffStr, iPos)
        Else
            pGetClassName = BuffStr
        End If
    End If
End Function

Private Function pGetWindowText(ByVal hwnd As Long) As String
    Dim BuffStr     As String
    Dim BuffStrLen  As Long
    BuffStrLen = GetWindowTextLength(hwnd)
    BuffStr = String(BuffStrLen, Chr(0))
    GetWindowText(hwnd, ByVal BuffStr, BuffStrLen + 1)
    pGetWindowText = BuffStr
End Function

Private Function pGetText(ByVal hwnd As Long) As String
    Dim BuffStr As String, BuffStrLen As Long, Rtn As Long
    BuffStrLen = GetWindowTextLength(hwnd)
    BuffStr = String(BuffStrLen, Chr(0))
    Rtn = SendMessage(hwnd, WM_GETTEXT, BuffStrLen + 1, ByVal BuffStr)
    pGetText = BuffStr
End Function

Private Function pDrawText(ByVal hDC As Long, ByVal Text As String, ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long, ByVal lpFlag As DTSTYLE) As Long
    Dim TmpRect As RECT
    With TmpRect
        .Left = X1
        .Top = Y1
        .Right = X2
        .Bottom = Y2
    End With
    pDrawText = DrawText(hDC, Text, -1, TmpRect, lpFlag)
End Function

Private Function pDrawTextL(ByVal hDC As Long, ByVal Text As String, ByVal X As Long, ByVal Y As Long, ByVal Width As Long, ByVal Height As Long, ByVal lpFlag As DTSTYLE) As Long
    Dim TmpRect As RECT
    With TmpRect
        .Left = X
        .Top = Y
        .Right = X + Width
        .Bottom = Y + Height
    End With
    pDrawTextL = DrawText(hDC, Text, -1, TmpRect, lpFlag)
End Function

Private Function pFillRect(ByVal hDC As Long, ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long, ByVal Color As Long) As Long
    Dim TmpRect     As RECT
    Dim TmpBrush    As Long
    With TmpRect
        .Left = X1
        .Top = Y1
        .Right = X2
        .Bottom = Y2
    End With
    TmpBrush = CreateSolidBrush(Color)
    pFillRect = FillRect(hDC, TmpRect, TmpBrush)
    DeleteObject TmpBrush
End Function

Private Function pFillRectL(ByVal hDC As Long, ByVal X As Long, ByVal Y As Long, ByVal Width As Long, ByVal Height As Long, ByVal Color As Long) As Long
    Dim TmpRect     As RECT
    Dim TmpBrush    As Long
    With TmpRect
        .Left = X
        .Top = Y
        .Right = X + Width
        .Bottom = Y + Height
    End With
    TmpBrush = CreateSolidBrush(Color)
    pFillRectL = FillRect(hDC, TmpRect, TmpBrush)
    DeleteObject TmpBrush
End Function

Private Function pGetWindowRectW(ByVal hwnd As Long, lpRectW As RECTW) As Long
    Dim TmpRect As RECT
    Dim Rtn     As Long
    Rtn = GetWindowRect(hwnd, TmpRect)
    With lpRectW
        .Left = TmpRect.Left
        .Top = TmpRect.Top
        .Right = TmpRect.Right
        .Bottom = TmpRect.Bottom
        .Width = TmpRect.Right - TmpRect.Left
        .Height = TmpRect.Bottom - TmpRect.Top
    End With
    pGetWindowRectW = Rtn
End Function

Private Sub pSetRectW(ByVal X1 As Long, ByVal Y1 As Long, ByVal X2 As Long, ByVal Y2 As Long, lpRectW As RECTW)
    With lpRectW
        .Left = X1
        .Right = X2
        .Top = Y1
        .Bottom = Y2
        .Width = X2 - X1
        .Height = Y2 - Y1
    End With
End Sub

Private Function pDrawFocusRect(ByVal hDC As Long, ByVal X As Long, ByVal Y As Long, ByVal Width As Long, ByVal Height As Long) As Long
    Dim TmpRect As RECT
    With TmpRect
        .Left = X
        .Top = Y
        .Right = X + Width
        .Bottom = Y + Height
    End With
    pDrawFocusRect = DrawFocusRect(hDC, TmpRect)
End Function

Private Function pFrameRect(ByVal hDC As Long, ByVal X As Long, Y As Long, ByVal Width As Long, ByVal Height As Long, ByVal Color As Long) As Long
    Dim TmpRect     As RECT
    Dim m_hBrush    As Long
    With TmpRect
        .Left = X
        .Top = Y
        .Right = X + Width
        .Bottom = Y + Height
    End With
    m_hBrush = CreateSolidBrush(Color)
    pFrameRect = FrameRect(hDC, TmpRect, m_hBrush)
    DeleteObject m_hBrush
End Function

Private Function pDrawBorderLine(ByVal hwnd As Long, ByVal State As Long) As Long
    Dim m_wRect As RECTW
    Dim m_hDC   As Long
    If GetWindowLong(hwnd, GWL_EXSTYLE) And WS_EX_CLIENTEDGE Then
        pGetWindowRectW(hwnd, m_wRect)
        m_hDC = GetWindowDC(hwnd)
        If State = 0 Then
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HD5A554)
            pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HF4E7D3)
        Else
            pFrameRect(m_hDC, 0, 0, m_wRect.Width, m_wRect.Height, &HF4E7D3)
            pFrameRect(m_hDC, 1, 1, m_wRect.Width - 2, m_wRect.Height - 2, &HD5A554)
        End If
        ReleaseDC hwnd, m_hDC
        pDrawBorderLine = 1
    End If
End Function

Private Function pSetTimer(ByVal hwnd As Long) As Long
    Dim m_TimerID       As Long
    m_TimerID = GetProp(hwnd, "TIMERID")
    If m_TimerID Then Exit Function
    m_TimerID = SetTimer(hwnd, 1, 15, 0&)
    SetProp(hwnd, "TIMERID", m_TimerID)
    pSetTimer = m_TimerID
End Function

Private Function pKillTimer(ByVal hwnd As Long) As Long
    Dim m_TimerID       As Long
    m_TimerID = GetProp(hwnd, "TIMERID")
    If m_TimerID = 0 Then Exit Function
    pKillTimer = KillTimer(hwnd, m_TimerID)
    SetProp(hwnd, "TIMERID", 0)
End Function

Private Function pGetBorderSize(ByVal hwnd As Long, cX As Long, cY As Long) As Long
    Dim wRect   As RECT
    Dim cRect   As RECT
    Dim lW      As Long
    Dim lH      As Long
    Dim sX      As Long
    Dim sY      As Long
    Dim lStyle  As Long
    GetWindowRect(hwnd, wRect)
    GetClientRect(hwnd, cRect)
    sX = GetSystemMetrics(SM_CXVSCROLL)
    sY = GetSystemMetrics(SM_CYHSCROLL)
    lW = (wRect.Right - wRect.Left) - (cRect.Right - cRect.Left)
    lH = (wRect.Bottom - wRect.Top) - (cRect.Bottom - cRect.Top)
    lStyle = GetWindowLong(hwnd, GWL_STYLE)
    If lStyle And WS_VSCROLL Then lW = lW - sX
    If lStyle And WS_VSCROLL Then lH = lH - sY
    cX = CLng(lW / 2)
    cY = CLng(lH / 2)
    pGetBorderSize = 1
End Function

Private Function IsFunctionSupported(sFunction As String, sModule As String) As Boolean
    Dim hModule As Long
    hModule = GetModuleHandleA(sModule)
    
    If (hModule = 0) Then
        hModule = LoadLibrary(sModule)
    End If
    
    If (hModule) Then
        If (GetProcAddress(hModule, sFunction)) Then
            IsFunctionSupported = True
        End If
        FreeLibrary hModule
    End If
End Function

Private Sub pTrackMouseTracking(hwnd As Long)
    Dim lpEventTrack As TRACKMOUSEEVENTTYPE
    With lpEventTrack
        .cbSize = Len(lpEventTrack)
        .dwFlags = &H2
        .hwndTrack = hwnd
    End With
    
    If (m_bTrackHandler32) Then
        TrackMouseEvent lpEventTrack
    Else
        TrackMouseEvent2 lpEventTrack
    End If
End Sub

Private Sub pDrawMemDC(ByVal hwnd As Long)
    Dim m_wRect       As RECTW
    Dim m_wText       As String
    Dim i           As Long
    Dim m_hDC(3)    As Long
    pGetWindowRectW(hwnd, m_wRect)
    m_wText = pGetWindowText(hwnd)
    For i = 0 To 3
        m_hDC(i) = GetProp(hwnd, "HDC" & CStr(i))
        SelectObject m_hDC(i), SendMessage(hwnd, WM_GETFONT, 0&, 0&)
        SetBkMode m_hDC(i), 1
        GridBlt(m_hDC(i), 0, 0, m_wRect.Width, m_wRect.Height, m_hBtnSrcDC, 0, i * 21, 7, 21, 3, 3, 3, 3)
        SetTextColor m_hDC(i), IIf(i = 3, &H968A79, 0&)
        pDrawTextL m_hDC(i), m_wText, 2, 2, m_wRect.Width - 4, m_wRect.Height - 4, DT_CENTER Or DT_VCENTER Or DT_SINGLELINE Or DT_END_ELLIPSIS
        If GetFocus = hwnd Then pDrawFocusRect m_hDC(i), 2, 2, m_wRect.Width - 4, m_wRect.Height - 4
    Next
End Sub

Private Function pCreateDC(ByVal Width As Long, ByVal Height As Long) As Long
    Dim TmpDC   As Long
    Dim rDC     As Long
    Dim rBmp    As Long
    TmpDC = CreateDC("DISPLAY", "", "", ByVal 0&)
    If TmpDC Then
        rDC = CreateCompatibleDC(TmpDC)
        If rDC Then
            rBmp = CreateCompatibleBitmap(TmpDC, Width, Height)
            If rBmp Then
                DeleteObject SelectObject(rDC, rBmp)
                pCreateDC = rDC
                DeleteObject rBmp
            Else
                DeleteDC rDC
            End If
        End If
        DeleteDC TmpDC
    End If
End Function

Private Function pCreateDCFromHandle(Handle As Long) As Long
    If Handle = 0 Then Exit Function
    Dim TmpDC   As Long
    TmpDC = pCreateDC(1, 1)
    DeleteObject SelectObject(TmpDC, Handle)
    pCreateDCFromHandle = TmpDC
End Function

Private Function GridBlt(ByVal hDestDC As Long, ByVal dstX As Long, ByVal dstY As Long, ByVal dstWidth As Long, ByVal dstHeight As Long, _
    ByVal hSrcDC As Long, ByVal srcX As Long, ByVal srcY As Long, ByVal srcwidth As Long, ByVal srcheight As Long, _
    Optional ByVal gX1 As Long = 0, Optional ByVal gY1 As Long = 0, Optional ByVal gX2 As Long = 0, Optional ByVal gY2 As Long = 0, _
    Optional ByVal MaskColor As Variant) As Long
    If dstWidth = 0 Or dstHeight = 0 Or srcwidth = 0 Or srcheight = 0 Then Exit Function
    Dim TmpDC As Long
    TmpDC = pCreateDC(dstWidth, dstHeight)
    If gX1 <= 0 And gX2 <= 0 And gY1 <= 0 And gY2 <= 0 Then
        StretchBlt TmpDC, 0, 0, dstWidth, dstHeight, hSrcDC, srcX + gX1, srcY + gY1, srcwidth - gX2, srcheight - gY2, vbSrcCopy
    Else
        If gX1 > 0 And gY1 > 0 Then                                             '   Ͻ 
            BitBlt TmpDC, 0, 0, gX1, gY1, hSrcDC, srcX, srcY, vbSrcCopy
        End If
        If gX2 > 0 And gY1 > 0 Then                                             '   Ͻ 
            BitBlt TmpDC, dstWidth - gX2, 0, gX2, gY1, hSrcDC, srcX + srcwidth - gX2, srcY, vbSrcCopy
        End If
        If gX1 > 0 And gY2 > 0 Then                                             '   ½ 
            BitBlt TmpDC, 0, dstHeight - gY2, gX1, gY2, hSrcDC, srcX, srcY + srcheight - gY2, vbSrcCopy
        End If
        If gX2 > 0 And gY2 > 0 Then                                             '   ½ 
            BitBlt TmpDC, dstWidth - gX2, dstHeight - gY2, gX2, gY2, hSrcDC, srcX + srcwidth - gX2, srcY + srcheight - gY2, vbSrcCopy
        End If
        If gX1 > 0 Then                                                         '  ߿ 
            StretchBlt TmpDC, 0, gY1, gX1, dstHeight - gY1 - gY2, hSrcDC, srcX, srcY + gY1, gX1, srcheight - gY1 - gY2, vbSrcCopy
        End If
        If gX2 > 0 Then                                                         ' ұ߿ 
            StretchBlt TmpDC, dstWidth - gX2, gY1, gX2, dstHeight - gY1 - gY2, hSrcDC, srcX + srcwidth - gX2, srcY + gY1, gX2, srcheight - gY1 - gY2, vbSrcCopy
        End If
        If gY1 > 0 Then                                                         ' ϱ߿ 
            StretchBlt TmpDC, gX1, 0, dstWidth - gX1 - gX2, gY1, hSrcDC, srcX + gX1, srcY, srcwidth - gX1 - gX2, gY1, vbSrcCopy
        End If
        If gY2 > 0 Then                                                         ' ±߿ 
            StretchBlt TmpDC, gX1, dstHeight - gY2, dstWidth - gX1 - gX2, gY2, hSrcDC, srcX + gX1, srcY + srcheight - gY2, srcwidth - gX1 - gX2, gY2, vbSrcCopy
        End If
        ' м    չ    
        StretchBlt TmpDC, gX1, gY1, dstWidth - gX1 - gX2, dstHeight - gY1 - gY2, hSrcDC, srcX + gX1, srcY + gY1, srcwidth - gX1 - gX2, srcheight - gY1 - gY2, vbSrcCopy
    End If
    If IsMissing(MaskColor) Then
        GridBlt = BitBlt(hDestDC, dstX, dstY, dstWidth, dstHeight, TmpDC, 0, 0, vbSrcCopy)
    Else
        TransBlt(hDestDC, dstX, dstY, dstWidth, dstHeight, TmpDC, 0, 0, CLng(Val(MaskColor)))
        GridBlt = 1
    End If
    DeleteDC(TmpDC)
End Function

Private Sub TransBlt(ByVal hDestDC As Long, ByVal dstX As Long, ByVal dstY As Long, ByVal dstWidth As Long, ByVal dstHeight As Long, _
    ByVal hSrcDC As Long, Optional ByVal srcX As Long = 0, Optional ByVal srcY As Long = 0, Optional ByVal MaskColor As Long = vbMagenta, _
    Optional ByVal hpal As Long = 0)                                            'Optional ByVal MaskColor As OLE_COLOR = vbMagenta
    Dim hdcMask             As Long
    Dim hdcColor            As Long
    Dim hbmMask             As Long
    Dim hbmColor            As Long
    Dim hbmColorOld         As Long
    Dim hbmMaskOld          As Long
    Dim hpalOld             As Long
    Dim hTmpDC              As Long
    Dim hdcScnBuffer        As Long
    Dim hbmScnBuffer        As Long
    Dim hbmScnBufferOld     As Long
    Dim hPalBufferOld       As Long
    Dim lMaskColor          As Long
    Dim hpalHalftone        As Long
    
    hTmpDC = CreateDC("DISPLAY", "", "", ByVal 0&)
    If hpal = 0 Then
        hpalHalftone = CreateHalftonePalette(hTmpDC)
        hpal = hpalHalftone
    End If
    OleTranslateColor MaskColor, hpal, lMaskColor
    lMaskColor = lMaskColor And &HFFFFFF
    hbmScnBuffer = CreateCompatibleBitmap(hTmpDC, dstWidth, dstHeight)
    hdcScnBuffer = CreateCompatibleDC(hTmpDC)
    hbmScnBufferOld = SelectObject(hdcScnBuffer, hbmScnBuffer)
    hPalBufferOld = SelectPalette(hdcScnBuffer, hpal, True)
    RealizePalette hdcScnBuffer
    BitBlt hdcScnBuffer, 0, 0, dstWidth, dstHeight, hDestDC, dstX, dstY, vbSrcCopy
    hbmColor = CreateCompatibleBitmap(hTmpDC, dstWidth, dstHeight)
    hbmMask = CreateBitmap(dstWidth, dstHeight, 1, 1, ByVal 0&)
    hdcColor = CreateCompatibleDC(hTmpDC)
    hbmColorOld = SelectObject(hdcColor, hbmColor)
    hpalOld = SelectPalette(hdcColor, hpal, True)
    RealizePalette hdcColor
    SetBkColor(hdcColor, GetBkColor(hSrcDC))
    SetTextColor(hdcColor, GetTextColor(hSrcDC))
    BitBlt(hdcColor, 0, 0, dstWidth, dstHeight, hSrcDC, srcX, srcY, vbSrcCopy)
    hdcMask = CreateCompatibleDC(hTmpDC)
    hbmMaskOld = SelectObject(hdcMask, hbmMask)
    SetBkColor(hdcColor, lMaskColor)
    SetTextColor(hdcColor, vbWhite)
    BitBlt(hdcMask, 0, 0, dstWidth, dstHeight, hdcColor, 0, 0, vbSrcCopy)
    SetTextColor(hdcColor, vbBlack)
    SetBkColor(hdcColor, vbWhite)
    BitBlt(hdcColor, 0, 0, dstWidth, dstHeight, hdcMask, 0, 0, &H220326)
    BitBlt(hdcScnBuffer, 0, 0, dstWidth, dstHeight, hdcMask, 0, 0, vbSrcAnd)
    BitBlt(hdcScnBuffer, 0, 0, dstWidth, dstHeight, hdcColor, 0, 0, vbSrcPaint)
    BitBlt(hDestDC, dstX, dstY, dstWidth, dstHeight, hdcScnBuffer, 0, 0, vbSrcCopy)
    DeleteObject(SelectObject(hdcColor, hbmColorOld))
    SelectPalette(hdcColor, hpalOld, True)
    RealizePalette(hdcColor)
    DeleteDC(hdcColor)
    DeleteObject(SelectObject(hdcScnBuffer, hbmScnBufferOld))
    SelectPalette(hdcScnBuffer, hPalBufferOld, 0)
    RealizePalette(hdcScnBuffer)
    DeleteDC(hdcScnBuffer)
    DeleteObject(SelectObject(hdcMask, hbmMaskOld))
    DeleteDC(hdcMask)
    If hpalHalftone <> 0 Then
        DeleteObject(hpalHalftone)
    End If
End Sub

Private Function LWORD(Param As Long) As Long
    Dim intRet As Integer
    CopyMemory intRet, Param, 2
    LWORD = intRet
End Function

Private Function HWORD(Param As Long) As Long
    Dim intRet As Integer
    CopyMemory intRet, @Param + 2, 2
    HWORD = intRet
End Function

Private Sub pInit()
    m_hBtnSrcDC = pCreateDC(7, 84)
    m_hCbbSrcDC = pCreateDC(15, 18)
    m_hCkbSrcDC = pCreateDC(18, 24)
    m_hOpbSrcDC = pCreateDC(13, 52)
    m_hHdbSrcDC = pCreateDC(9, 20)
    
    SetPixelV m_hBtnSrcDC, 0, 0, 16376255
    SetPixelV m_hBtnSrcDC, 1, 0, 14922603
    SetPixelV m_hBtnSrcDC, 2, 0, 14194476
    SetPixelV m_hBtnSrcDC, 3, 0, 13995803
    SetPixelV m_hBtnSrcDC, 4, 0, 14194476
    SetPixelV m_hBtnSrcDC, 5, 0, 14922603
    SetPixelV m_hBtnSrcDC, 6, 0, 16376255
    SetPixelV m_hBtnSrcDC, 0, 1, 14856809
    SetPixelV m_hBtnSrcDC, 1, 1, 15188094
    SetPixelV m_hBtnSrcDC, 2, 1, 16511975
    SetPixelV m_hBtnSrcDC, 3, 1, 16777215
    SetPixelV m_hBtnSrcDC, 4, 1, 16511975
    SetPixelV m_hBtnSrcDC, 5, 1, 15188094
    SetPixelV m_hBtnSrcDC, 6, 1, 14856809
    SetPixelV m_hBtnSrcDC, 0, 2, 14128425
    SetPixelV m_hBtnSrcDC, 1, 2, 16578284
    SetPixelV m_hBtnSrcDC, 2, 2, 16777215
    SetPixelV m_hBtnSrcDC, 3, 2, 16777215
    SetPixelV m_hBtnSrcDC, 4, 2, 16777215
    SetPixelV m_hBtnSrcDC, 5, 2, 16578284
    SetPixelV m_hBtnSrcDC, 6, 2, 14128425
    SetPixelV m_hBtnSrcDC, 0, 3, 13995803
    SetPixelV m_hBtnSrcDC, 1, 3, 16644853
    SetPixelV m_hBtnSrcDC, 2, 3, 16578801
    SetPixelV m_hBtnSrcDC, 3, 3, 16578801
    SetPixelV m_hBtnSrcDC, 4, 3, 16578801
    SetPixelV m_hBtnSrcDC, 5, 3, 16644853
    SetPixelV m_hBtnSrcDC, 6, 3, 13995803
    SetPixelV m_hBtnSrcDC, 0, 4, 13995803
    SetPixelV m_hBtnSrcDC, 1, 4, 16579059
    SetPixelV m_hBtnSrcDC, 2, 4, 16512750
    SetPixelV m_hBtnSrcDC, 3, 4, 16512750
    SetPixelV m_hBtnSrcDC, 4, 4, 16512750
    SetPixelV m_hBtnSrcDC, 5, 4, 16579059
    SetPixelV m_hBtnSrcDC, 6, 4, 13995803
    SetPixelV m_hBtnSrcDC, 0, 5, 13995803
    SetPixelV m_hBtnSrcDC, 1, 5, 16578544
    SetPixelV m_hBtnSrcDC, 2, 5, 16512234
    SetPixelV m_hBtnSrcDC, 3, 5, 16512234
    SetPixelV m_hBtnSrcDC, 4, 5, 16512234
    SetPixelV m_hBtnSrcDC, 5, 5, 16578544
    SetPixelV m_hBtnSrcDC, 6, 5, 13995803
    SetPixelV m_hBtnSrcDC, 0, 6, 13995803
    SetPixelV m_hBtnSrcDC, 1, 6, 16578286
    SetPixelV m_hBtnSrcDC, 2, 6, 16511718
    SetPixelV m_hBtnSrcDC, 3, 6, 16511718
    SetPixelV m_hBtnSrcDC, 4, 6, 16511718
    SetPixelV m_hBtnSrcDC, 5, 6, 16578286
    SetPixelV m_hBtnSrcDC, 6, 6, 13995803
    SetPixelV m_hBtnSrcDC, 0, 7, 13995803
    SetPixelV m_hBtnSrcDC, 1, 7, 16578027
    SetPixelV m_hBtnSrcDC, 2, 7, 16445666
    SetPixelV m_hBtnSrcDC, 3, 7, 16445666
    SetPixelV m_hBtnSrcDC, 4, 7, 16445666
    SetPixelV m_hBtnSrcDC, 5, 7, 16578027
    SetPixelV m_hBtnSrcDC, 6, 7, 13995803
    SetPixelV m_hBtnSrcDC, 0, 8, 13995803
    SetPixelV m_hBtnSrcDC, 1, 8, 16577512
    SetPixelV m_hBtnSrcDC, 2, 8, 16445150
    SetPixelV m_hBtnSrcDC, 3, 8, 16445150
    SetPixelV m_hBtnSrcDC, 4, 8, 16445150
    SetPixelV m_hBtnSrcDC, 5, 8, 16577512
    SetPixelV m_hBtnSrcDC, 6, 8, 13995803
    SetPixelV m_hBtnSrcDC, 0, 9, 13995803
    SetPixelV m_hBtnSrcDC, 1, 9, 16511717
    SetPixelV m_hBtnSrcDC, 2, 9, 16379098
    SetPixelV m_hBtnSrcDC, 3, 9, 16379098
    SetPixelV m_hBtnSrcDC, 4, 9, 16379098
    SetPixelV m_hBtnSrcDC, 5, 9, 16511717
    SetPixelV m_hBtnSrcDC, 6, 9, 13995803
    SetPixelV m_hBtnSrcDC, 0, 10, 13995803
    SetPixelV m_hBtnSrcDC, 1, 10, 16511203
    SetPixelV m_hBtnSrcDC, 2, 10, 16378583
    SetPixelV m_hBtnSrcDC, 3, 10, 16378583
    SetPixelV m_hBtnSrcDC, 4, 10, 16378583
    SetPixelV m_hBtnSrcDC, 5, 10, 16511203
    SetPixelV m_hBtnSrcDC, 6, 10, 13995803
    SetPixelV m_hBtnSrcDC, 0, 11, 13995803
    SetPixelV m_hBtnSrcDC, 1, 11, 16313307
    SetPixelV m_hBtnSrcDC, 2, 11, 16114380
    SetPixelV m_hBtnSrcDC, 3, 11, 16114380
    SetPixelV m_hBtnSrcDC, 4, 11, 16114380
    SetPixelV m_hBtnSrcDC, 5, 11, 16313307
    SetPixelV m_hBtnSrcDC, 6, 11, 13995803
    SetPixelV m_hBtnSrcDC, 0, 12, 13995803
    SetPixelV m_hBtnSrcDC, 1, 12, 16247257
    SetPixelV m_hBtnSrcDC, 2, 12, 15982536
    SetPixelV m_hBtnSrcDC, 3, 12, 15982536
    SetPixelV m_hBtnSrcDC, 4, 12, 15982536
    SetPixelV m_hBtnSrcDC, 5, 12, 16247257
    SetPixelV m_hBtnSrcDC, 6, 12, 13995803
    SetPixelV m_hBtnSrcDC, 0, 13, 13995803
    SetPixelV m_hBtnSrcDC, 1, 13, 16115669
    SetPixelV m_hBtnSrcDC, 2, 13, 15850691
    SetPixelV m_hBtnSrcDC, 3, 13, 15850691
    SetPixelV m_hBtnSrcDC, 4, 13, 15850691
    SetPixelV m_hBtnSrcDC, 5, 13, 16115669
    SetPixelV m_hBtnSrcDC, 6, 13, 13995803
    SetPixelV m_hBtnSrcDC, 0, 14, 13995803
    SetPixelV m_hBtnSrcDC, 1, 14, 16049362
    SetPixelV m_hBtnSrcDC, 2, 14, 15718590
    SetPixelV m_hBtnSrcDC, 3, 14, 15718590
    SetPixelV m_hBtnSrcDC, 4, 14, 15718590
    SetPixelV m_hBtnSrcDC, 5, 14, 16049362
    SetPixelV m_hBtnSrcDC, 6, 14, 13995803
    SetPixelV m_hBtnSrcDC, 0, 15, 13995803
    SetPixelV m_hBtnSrcDC, 1, 15, 15917773
    SetPixelV m_hBtnSrcDC, 2, 15, 15586744
    SetPixelV m_hBtnSrcDC, 3, 15, 15586744
    SetPixelV m_hBtnSrcDC, 4, 15, 15586744
    SetPixelV m_hBtnSrcDC, 5, 15, 15917773
    SetPixelV m_hBtnSrcDC, 6, 15, 13995803
    SetPixelV m_hBtnSrcDC, 0, 16, 13995803
    SetPixelV m_hBtnSrcDC, 1, 16, 15851723
    SetPixelV m_hBtnSrcDC, 2, 16, 15454900
    SetPixelV m_hBtnSrcDC, 3, 16, 15454900
    SetPixelV m_hBtnSrcDC, 4, 16, 15454900
    SetPixelV m_hBtnSrcDC, 5, 16, 15851723
    SetPixelV m_hBtnSrcDC, 6, 16, 13995803
    SetPixelV m_hBtnSrcDC, 0, 17, 13995803
    SetPixelV m_hBtnSrcDC, 1, 17, 15785673
    SetPixelV m_hBtnSrcDC, 2, 17, 15388849
    SetPixelV m_hBtnSrcDC, 3, 17, 15388849
    SetPixelV m_hBtnSrcDC, 4, 17, 15388849
    SetPixelV m_hBtnSrcDC, 5, 17, 15785673
    SetPixelV m_hBtnSrcDC, 6, 17, 13995803
    SetPixelV m_hBtnSrcDC, 0, 18, 14128424
    SetPixelV m_hBtnSrcDC, 1, 18, 15652794
    SetPixelV m_hBtnSrcDC, 2, 18, 15521467
    SetPixelV m_hBtnSrcDC, 3, 18, 15323056
    SetPixelV m_hBtnSrcDC, 4, 18, 15521211
    SetPixelV m_hBtnSrcDC, 5, 18, 15652794
    SetPixelV m_hBtnSrcDC, 6, 18, 14128424
    SetPixelV m_hBtnSrcDC, 0, 19, 14856292
    SetPixelV m_hBtnSrcDC, 1, 19, 14791272
    SetPixelV m_hBtnSrcDC, 2, 19, 15653051
    SetPixelV m_hBtnSrcDC, 3, 19, 15851724
    SetPixelV m_hBtnSrcDC, 4, 19, 15653051
    SetPixelV m_hBtnSrcDC, 5, 19, 14791272
    SetPixelV m_hBtnSrcDC, 6, 19, 14790498
    SetPixelV m_hBtnSrcDC, 0, 20, 16508874
    SetPixelV m_hBtnSrcDC, 1, 20, 14790240
    SetPixelV m_hBtnSrcDC, 2, 20, 14194218
    SetPixelV m_hBtnSrcDC, 3, 20, 13995803
    SetPixelV m_hBtnSrcDC, 4, 20, 14194218
    SetPixelV m_hBtnSrcDC, 5, 20, 14790240
    SetPixelV m_hBtnSrcDC, 6, 20, 16508874
    SetPixelV m_hBtnSrcDC, 0, 21, 16376255
    SetPixelV m_hBtnSrcDC, 1, 21, 14922603
    SetPixelV m_hBtnSrcDC, 2, 21, 14194476
    SetPixelV m_hBtnSrcDC, 3, 21, 13995803
    SetPixelV m_hBtnSrcDC, 4, 21, 14194476
    SetPixelV m_hBtnSrcDC, 5, 21, 14922603
    SetPixelV m_hBtnSrcDC, 6, 21, 16376255
    SetPixelV m_hBtnSrcDC, 0, 22, 14856809
    SetPixelV m_hBtnSrcDC, 1, 22, 15115048
    SetPixelV m_hBtnSrcDC, 2, 22, 16300860
    SetPixelV m_hBtnSrcDC, 3, 22, 16499532
    SetPixelV m_hBtnSrcDC, 4, 22, 16300860
    SetPixelV m_hBtnSrcDC, 5, 22, 15115048
    SetPixelV m_hBtnSrcDC, 6, 22, 14856809
    SetPixelV m_hBtnSrcDC, 0, 23, 14128425
    SetPixelV m_hBtnSrcDC, 1, 23, 16366653
    SetPixelV m_hBtnSrcDC, 2, 23, 16702093
    SetPixelV m_hBtnSrcDC, 3, 23, 16772550
    SetPixelV m_hBtnSrcDC, 4, 23, 16702093
    SetPixelV m_hBtnSrcDC, 5, 23, 16366653
    SetPixelV m_hBtnSrcDC, 6, 23, 14128425
    SetPixelV m_hBtnSrcDC, 0, 24, 13995803
    SetPixelV m_hBtnSrcDC, 1, 24, 16499014
    SetPixelV m_hBtnSrcDC, 2, 24, 16638894
    SetPixelV m_hBtnSrcDC, 3, 24, 16576210
    SetPixelV m_hBtnSrcDC, 4, 24, 16704688
    SetPixelV m_hBtnSrcDC, 5, 24, 16499014
    SetPixelV m_hBtnSrcDC, 6, 24, 13995803
    SetPixelV m_hBtnSrcDC, 0, 25, 13995803
    SetPixelV m_hBtnSrcDC, 1, 25, 16499274
    SetPixelV m_hBtnSrcDC, 2, 25, 16639930
    SetPixelV m_hBtnSrcDC, 3, 25, 16577247
    SetPixelV m_hBtnSrcDC, 4, 25, 16639931
    SetPixelV m_hBtnSrcDC, 5, 25, 16499274
    SetPixelV m_hBtnSrcDC, 6, 25, 13995803
    SetPixelV m_hBtnSrcDC, 0, 26, 13995803
    SetPixelV m_hBtnSrcDC, 1, 26, 16499532
    SetPixelV m_hBtnSrcDC, 2, 26, 16640192
    SetPixelV m_hBtnSrcDC, 3, 26, 16577252
    SetPixelV m_hBtnSrcDC, 4, 26, 16640192
    SetPixelV m_hBtnSrcDC, 5, 26, 16499532
    SetPixelV m_hBtnSrcDC, 6, 26, 13995803
    SetPixelV m_hBtnSrcDC, 0, 27, 13995803
    SetPixelV m_hBtnSrcDC, 1, 27, 16499532
    SetPixelV m_hBtnSrcDC, 2, 27, 16639936
    SetPixelV m_hBtnSrcDC, 3, 27, 16577250
    SetPixelV m_hBtnSrcDC, 4, 27, 16639936
    SetPixelV m_hBtnSrcDC, 5, 27, 16499532
    SetPixelV m_hBtnSrcDC, 6, 27, 13995803
    SetPixelV m_hBtnSrcDC, 0, 28, 13995803
    SetPixelV m_hBtnSrcDC, 1, 28, 16499532
    SetPixelV m_hBtnSrcDC, 2, 28, 16574399
    SetPixelV m_hBtnSrcDC, 3, 28, 16511456
    SetPixelV m_hBtnSrcDC, 4, 28, 16574399
    SetPixelV m_hBtnSrcDC, 5, 28, 16499532
    SetPixelV m_hBtnSrcDC, 6, 28, 13995803
    SetPixelV m_hBtnSrcDC, 0, 29, 13995803
    SetPixelV m_hBtnSrcDC, 1, 29, 16499275
    SetPixelV m_hBtnSrcDC, 2, 29, 16574140
    SetPixelV m_hBtnSrcDC, 3, 29, 16511197
    SetPixelV m_hBtnSrcDC, 4, 29, 16574140
    SetPixelV m_hBtnSrcDC, 5, 29, 16499275
    SetPixelV m_hBtnSrcDC, 6, 29, 13995803
    SetPixelV m_hBtnSrcDC, 0, 30, 13995803
    SetPixelV m_hBtnSrcDC, 1, 30, 16499274
    SetPixelV m_hBtnSrcDC, 2, 30, 16573882
    SetPixelV m_hBtnSrcDC, 3, 30, 16510683
    SetPixelV m_hBtnSrcDC, 4, 30, 16573882
    SetPixelV m_hBtnSrcDC, 5, 30, 16499274
    SetPixelV m_hBtnSrcDC, 6, 30, 13995803
    SetPixelV m_hBtnSrcDC, 0, 31, 13995803
    SetPixelV m_hBtnSrcDC, 1, 31, 16499274
    SetPixelV m_hBtnSrcDC, 2, 31, 16573625
    SetPixelV m_hBtnSrcDC, 3, 31, 16510425
    SetPixelV m_hBtnSrcDC, 4, 31, 16573625
    SetPixelV m_hBtnSrcDC, 5, 31, 16499274
    SetPixelV m_hBtnSrcDC, 6, 31, 13995803
    SetPixelV m_hBtnSrcDC, 0, 32, 13995803
    SetPixelV m_hBtnSrcDC, 1, 32, 16432966
    SetPixelV m_hBtnSrcDC, 2, 32, 16375214
    SetPixelV m_hBtnSrcDC, 3, 32, 16245963
    SetPixelV m_hBtnSrcDC, 4, 32, 16375214
    SetPixelV m_hBtnSrcDC, 5, 32, 16432966
    SetPixelV m_hBtnSrcDC, 6, 32, 13995803
    SetPixelV m_hBtnSrcDC, 0, 33, 13995803
    SetPixelV m_hBtnSrcDC, 1, 33, 16432965
    SetPixelV m_hBtnSrcDC, 2, 33, 16243370
    SetPixelV m_hBtnSrcDC, 3, 33, 16114119
    SetPixelV m_hBtnSrcDC, 4, 33, 16243370
    SetPixelV m_hBtnSrcDC, 5, 33, 16432965
    SetPixelV m_hBtnSrcDC, 6, 33, 13995803
    SetPixelV m_hBtnSrcDC, 0, 34, 13995803
    SetPixelV m_hBtnSrcDC, 1, 34, 16367172
    SetPixelV m_hBtnSrcDC, 2, 34, 16177319
    SetPixelV m_hBtnSrcDC, 3, 34, 15982274
    SetPixelV m_hBtnSrcDC, 4, 34, 16177319
    SetPixelV m_hBtnSrcDC, 5, 34, 16367172
    SetPixelV m_hBtnSrcDC, 6, 34, 13995803
    SetPixelV m_hBtnSrcDC, 0, 35, 13995803
    SetPixelV m_hBtnSrcDC, 1, 35, 16301378
    SetPixelV m_hBtnSrcDC, 2, 35, 16045730
    SetPixelV m_hBtnSrcDC, 3, 35, 15915964
    SetPixelV m_hBtnSrcDC, 4, 35, 16045730
    SetPixelV m_hBtnSrcDC, 5, 35, 16301378
    SetPixelV m_hBtnSrcDC, 6, 35, 13995803
    SetPixelV m_hBtnSrcDC, 0, 36, 13995803
    SetPixelV m_hBtnSrcDC, 1, 36, 16301121
    SetPixelV m_hBtnSrcDC, 2, 36, 15979165
    SetPixelV m_hBtnSrcDC, 3, 36, 15849399
    SetPixelV m_hBtnSrcDC, 4, 36, 15979165
    SetPixelV m_hBtnSrcDC, 5, 36, 16301121
    SetPixelV m_hBtnSrcDC, 6, 36, 13995803
    SetPixelV m_hBtnSrcDC, 0, 37, 13995803
    SetPixelV m_hBtnSrcDC, 1, 37, 16300861
    SetPixelV m_hBtnSrcDC, 2, 37, 15978646
    SetPixelV m_hBtnSrcDC, 3, 37, 15717807
    SetPixelV m_hBtnSrcDC, 4, 37, 15978644
    SetPixelV m_hBtnSrcDC, 5, 37, 16300862
    SetPixelV m_hBtnSrcDC, 6, 37, 13995803
    SetPixelV m_hBtnSrcDC, 0, 38, 13995803
    SetPixelV m_hBtnSrcDC, 1, 38, 16300601
    SetPixelV m_hBtnSrcDC, 2, 38, 16109711
    SetPixelV m_hBtnSrcDC, 3, 38, 15914151
    SetPixelV m_hBtnSrcDC, 4, 38, 15977608
    SetPixelV m_hBtnSrcDC, 5, 38, 16300601
    SetPixelV m_hBtnSrcDC, 6, 38, 13995803
    SetPixelV m_hBtnSrcDC, 0, 39, 14128424
    SetPixelV m_hBtnSrcDC, 1, 39, 16168499
    SetPixelV m_hBtnSrcDC, 2, 39, 16238960
    SetPixelV m_hBtnSrcDC, 3, 39, 15979161
    SetPixelV m_hBtnSrcDC, 4, 39, 16239217
    SetPixelV m_hBtnSrcDC, 5, 39, 16168501
    SetPixelV m_hBtnSrcDC, 6, 39, 14128424
    SetPixelV m_hBtnSrcDC, 0, 40, 14856292
    SetPixelV m_hBtnSrcDC, 1, 40, 15048997
    SetPixelV m_hBtnSrcDC, 2, 40, 16168243
    SetPixelV m_hBtnSrcDC, 3, 40, 16300863
    SetPixelV m_hBtnSrcDC, 4, 40, 16168499
    SetPixelV m_hBtnSrcDC, 5, 40, 15048996
    SetPixelV m_hBtnSrcDC, 6, 40, 14790498
    SetPixelV m_hBtnSrcDC, 0, 41, 16508874
    SetPixelV m_hBtnSrcDC, 1, 41, 14790240
    SetPixelV m_hBtnSrcDC, 2, 41, 14194218
    SetPixelV m_hBtnSrcDC, 3, 41, 13995803
    SetPixelV m_hBtnSrcDC, 4, 41, 14194218
    SetPixelV m_hBtnSrcDC, 5, 41, 14790240
    SetPixelV m_hBtnSrcDC, 6, 41, 16508874
    SetPixelV m_hBtnSrcDC, 0, 42, 16376255
    SetPixelV m_hBtnSrcDC, 1, 42, 14922603
    SetPixelV m_hBtnSrcDC, 2, 42, 14194476
    SetPixelV m_hBtnSrcDC, 3, 42, 13995803
    SetPixelV m_hBtnSrcDC, 4, 42, 14194476
    SetPixelV m_hBtnSrcDC, 5, 42, 14922603
    SetPixelV m_hBtnSrcDC, 6, 42, 16376255
    SetPixelV m_hBtnSrcDC, 0, 43, 14856809
    SetPixelV m_hBtnSrcDC, 1, 43, 15114530
    SetPixelV m_hBtnSrcDC, 2, 43, 16299312
    SetPixelV m_hBtnSrcDC, 3, 43, 16497721
    SetPixelV m_hBtnSrcDC, 4, 43, 16233775
    SetPixelV m_hBtnSrcDC, 5, 43, 15114531
    SetPixelV m_hBtnSrcDC, 6, 43, 14856809
    SetPixelV m_hBtnSrcDC, 0, 44, 14128425
    SetPixelV m_hBtnSrcDC, 1, 44, 16299566
    SetPixelV m_hBtnSrcDC, 2, 44, 16304495
    SetPixelV m_hBtnSrcDC, 3, 44, 15977606
    SetPixelV m_hBtnSrcDC, 4, 44, 16304237
    SetPixelV m_hBtnSrcDC, 5, 44, 16233772
    SetPixelV m_hBtnSrcDC, 6, 44, 14128425
    SetPixelV m_hBtnSrcDC, 0, 45, 13995803
    SetPixelV m_hBtnSrcDC, 1, 45, 16365361
    SetPixelV m_hBtnSrcDC, 2, 45, 15911555
    SetPixelV m_hBtnSrcDC, 3, 45, 15979679
    SetPixelV m_hBtnSrcDC, 4, 45, 16043658
    SetPixelV m_hBtnSrcDC, 5, 45, 16365361
    SetPixelV m_hBtnSrcDC, 6, 45, 13995803
    SetPixelV m_hBtnSrcDC, 0, 46, 13995803
    SetPixelV m_hBtnSrcDC, 1, 46, 16365621
    SetPixelV m_hBtnSrcDC, 2, 46, 15912077
    SetPixelV m_hBtnSrcDC, 3, 46, 15980459
    SetPixelV m_hBtnSrcDC, 4, 46, 15912335
    SetPixelV m_hBtnSrcDC, 5, 46, 16365621
    SetPixelV m_hBtnSrcDC, 6, 46, 13995803
    SetPixelV m_hBtnSrcDC, 0, 47, 13995803
    SetPixelV m_hBtnSrcDC, 1, 47, 16365879
    SetPixelV m_hBtnSrcDC, 2, 47, 15847061
    SetPixelV m_hBtnSrcDC, 3, 47, 15980979
    SetPixelV m_hBtnSrcDC, 4, 47, 15912597
    SetPixelV m_hBtnSrcDC, 5, 47, 16365879
    SetPixelV m_hBtnSrcDC, 6, 47, 13995803
    SetPixelV m_hBtnSrcDC, 0, 48, 13995803
    SetPixelV m_hBtnSrcDC, 1, 48, 16365882
    SetPixelV m_hBtnSrcDC, 2, 48, 15913114
    SetPixelV m_hBtnSrcDC, 3, 48, 15981495
    SetPixelV m_hBtnSrcDC, 4, 48, 15913114
    SetPixelV m_hBtnSrcDC, 5, 48, 16365882
    SetPixelV m_hBtnSrcDC, 6, 48, 13995803
    SetPixelV m_hBtnSrcDC, 0, 49, 13995803
    SetPixelV m_hBtnSrcDC, 1, 49, 16366139
    SetPixelV m_hBtnSrcDC, 2, 49, 15979423
    SetPixelV m_hBtnSrcDC, 3, 49, 16113084
    SetPixelV m_hBtnSrcDC, 4, 49, 15979423
    SetPixelV m_hBtnSrcDC, 5, 49, 16366139
    SetPixelV m_hBtnSrcDC, 6, 49, 13995803
    SetPixelV m_hBtnSrcDC, 0, 50, 13995803
    SetPixelV m_hBtnSrcDC, 1, 50, 16431932
    SetPixelV m_hBtnSrcDC, 2, 50, 16111267
    SetPixelV m_hBtnSrcDC, 3, 50, 16179647
    SetPixelV m_hBtnSrcDC, 4, 50, 16111267
    SetPixelV m_hBtnSrcDC, 5, 50, 16431932
    SetPixelV m_hBtnSrcDC, 6, 50, 13995803
    SetPixelV m_hBtnSrcDC, 0, 51, 13995803
    SetPixelV m_hBtnSrcDC, 1, 51, 16432189
    SetPixelV m_hBtnSrcDC, 2, 51, 16177319
    SetPixelV m_hBtnSrcDC, 3, 51, 16310979
    SetPixelV m_hBtnSrcDC, 4, 51, 16177319
    SetPixelV m_hBtnSrcDC, 5, 51, 16432189
    SetPixelV m_hBtnSrcDC, 6, 51, 13995803
    SetPixelV m_hBtnSrcDC, 0, 52, 13995803
    SetPixelV m_hBtnSrcDC, 1, 52, 16497983
    SetPixelV m_hBtnSrcDC, 2, 52, 16308906
    SetPixelV m_hBtnSrcDC, 3, 52, 16377029
    SetPixelV m_hBtnSrcDC, 4, 52, 16308906
    SetPixelV m_hBtnSrcDC, 5, 52, 16497983
    SetPixelV m_hBtnSrcDC, 6, 52, 13995803
    SetPixelV m_hBtnSrcDC, 0, 53, 13995803
    SetPixelV m_hBtnSrcDC, 1, 53, 16564034
    SetPixelV m_hBtnSrcDC, 2, 53, 16507573
    SetPixelV m_hBtnSrcDC, 3, 53, 16509903
    SetPixelV m_hBtnSrcDC, 4, 53, 16507573
    SetPixelV m_hBtnSrcDC, 5, 53, 16564034
    SetPixelV m_hBtnSrcDC, 6, 53, 13995803
    SetPixelV m_hBtnSrcDC, 0, 54, 13995803
    SetPixelV m_hBtnSrcDC, 1, 54, 16564291
    SetPixelV m_hBtnSrcDC, 2, 54, 16507831
    SetPixelV m_hBtnSrcDC, 3, 54, 16510161
    SetPixelV m_hBtnSrcDC, 4, 54, 16507831
    SetPixelV m_hBtnSrcDC, 5, 54, 16564291
    SetPixelV m_hBtnSrcDC, 6, 54, 13995803
    SetPixelV m_hBtnSrcDC, 0, 55, 13995803
    SetPixelV m_hBtnSrcDC, 1, 55, 16564292
    SetPixelV m_hBtnSrcDC, 2, 55, 16573625
    SetPixelV m_hBtnSrcDC, 3, 55, 16575956
    SetPixelV m_hBtnSrcDC, 4, 55, 16573625
    SetPixelV m_hBtnSrcDC, 5, 55, 16564292
    SetPixelV m_hBtnSrcDC, 6, 55, 13995803
    SetPixelV m_hBtnSrcDC, 0, 56, 13995803
    SetPixelV m_hBtnSrcDC, 1, 56, 16564549
    SetPixelV m_hBtnSrcDC, 2, 56, 16573884
    SetPixelV m_hBtnSrcDC, 3, 56, 16576214
    SetPixelV m_hBtnSrcDC, 4, 56, 16573884
    SetPixelV m_hBtnSrcDC, 5, 56, 16564549
    SetPixelV m_hBtnSrcDC, 6, 56, 13995803
    SetPixelV m_hBtnSrcDC, 0, 57, 13995803
    SetPixelV m_hBtnSrcDC, 1, 57, 16564548
    SetPixelV m_hBtnSrcDC, 2, 57, 16574141
    SetPixelV m_hBtnSrcDC, 3, 57, 16576470
    SetPixelV m_hBtnSrcDC, 4, 57, 16574141
    SetPixelV m_hBtnSrcDC, 5, 57, 16564548
    SetPixelV m_hBtnSrcDC, 6, 57, 13995803
    SetPixelV m_hBtnSrcDC, 0, 58, 13995803
    SetPixelV m_hBtnSrcDC, 1, 58, 16564290
    SetPixelV m_hBtnSrcDC, 2, 58, 16574136
    SetPixelV m_hBtnSrcDC, 3, 58, 16576209
    SetPixelV m_hBtnSrcDC, 4, 58, 16574136
    SetPixelV m_hBtnSrcDC, 5, 58, 16564290
    SetPixelV m_hBtnSrcDC, 6, 58, 13995803
    SetPixelV m_hBtnSrcDC, 0, 59, 13995803
    SetPixelV m_hBtnSrcDC, 1, 59, 16564030
    SetPixelV m_hBtnSrcDC, 2, 59, 16704686
    SetPixelV m_hBtnSrcDC, 3, 59, 16640707
    SetPixelV m_hBtnSrcDC, 4, 59, 16638636
    SetPixelV m_hBtnSrcDC, 5, 59, 16564030
    SetPixelV m_hBtnSrcDC, 6, 59, 13995803
    SetPixelV m_hBtnSrcDC, 0, 60, 14128424
    SetPixelV m_hBtnSrcDC, 1, 60, 16431414
    SetPixelV m_hBtnSrcDC, 2, 60, 16701318
    SetPixelV m_hBtnSrcDC, 3, 60, 16639150
    SetPixelV m_hBtnSrcDC, 4, 60, 16701320
    SetPixelV m_hBtnSrcDC, 5, 60, 16431670
    SetPixelV m_hBtnSrcDC, 6, 60, 14128424
    SetPixelV m_hBtnSrcDC, 0, 61, 14856292
    SetPixelV m_hBtnSrcDC, 1, 61, 15115045
    SetPixelV m_hBtnSrcDC, 2, 61, 16366135
    SetPixelV m_hBtnSrcDC, 3, 61, 16630080
    SetPixelV m_hBtnSrcDC, 4, 61, 16366135
    SetPixelV m_hBtnSrcDC, 5, 61, 15115045
    SetPixelV m_hBtnSrcDC, 6, 61, 14790498
    SetPixelV m_hBtnSrcDC, 0, 62, 16508874
    SetPixelV m_hBtnSrcDC, 1, 62, 14790240
    SetPixelV m_hBtnSrcDC, 2, 62, 14194218
    SetPixelV m_hBtnSrcDC, 3, 62, 13995803
    SetPixelV m_hBtnSrcDC, 4, 62, 14194218
    SetPixelV m_hBtnSrcDC, 5, 62, 14790240
    SetPixelV m_hBtnSrcDC, 6, 62, 16508874
    SetPixelV m_hBtnSrcDC, 0, 63, 16513010
    SetPixelV m_hBtnSrcDC, 1, 63, 15717807
    SetPixelV m_hBtnSrcDC, 2, 63, 15386511
    SetPixelV m_hBtnSrcDC, 3, 63, 15254407
    SetPixelV m_hBtnSrcDC, 4, 63, 15386511
    SetPixelV m_hBtnSrcDC, 5, 63, 15717807
    SetPixelV m_hBtnSrcDC, 6, 63, 16513010
    SetPixelV m_hBtnSrcDC, 0, 64, 15717806
    SetPixelV m_hBtnSrcDC, 1, 64, 15850680
    SetPixelV m_hBtnSrcDC, 2, 64, 16512493
    SetPixelV m_hBtnSrcDC, 3, 64, 16645112
    SetPixelV m_hBtnSrcDC, 4, 64, 16512493
    SetPixelV m_hBtnSrcDC, 5, 64, 15850680
    SetPixelV m_hBtnSrcDC, 6, 64, 15717806
    SetPixelV m_hBtnSrcDC, 0, 65, 15320718
    SetPixelV m_hBtnSrcDC, 1, 65, 16513007
    SetPixelV m_hBtnSrcDC, 2, 65, 16645112
    SetPixelV m_hBtnSrcDC, 3, 65, 16645112
    SetPixelV m_hBtnSrcDC, 4, 65, 16645112
    SetPixelV m_hBtnSrcDC, 5, 65, 16513007
    SetPixelV m_hBtnSrcDC, 6, 65, 15320718
    SetPixelV m_hBtnSrcDC, 0, 66, 15254407
    SetPixelV m_hBtnSrcDC, 1, 66, 16578803
    SetPixelV m_hBtnSrcDC, 2, 66, 16513010
    SetPixelV m_hBtnSrcDC, 3, 66, 16513010
    SetPixelV m_hBtnSrcDC, 4, 66, 16513010
    SetPixelV m_hBtnSrcDC, 5, 66, 16578803
    SetPixelV m_hBtnSrcDC, 6, 66, 15254407
    SetPixelV m_hBtnSrcDC, 0, 67, 15254407
    SetPixelV m_hBtnSrcDC, 1, 67, 16513266
    SetPixelV m_hBtnSrcDC, 2, 67, 16513008
    SetPixelV m_hBtnSrcDC, 3, 67, 16513008
    SetPixelV m_hBtnSrcDC, 4, 67, 16513008
    SetPixelV m_hBtnSrcDC, 5, 67, 16513266
    SetPixelV m_hBtnSrcDC, 6, 67, 15254407
    SetPixelV m_hBtnSrcDC, 0, 68, 15254407
    SetPixelV m_hBtnSrcDC, 1, 68, 16513009
    SetPixelV m_hBtnSrcDC, 2, 68, 16512750
    SetPixelV m_hBtnSrcDC, 3, 68, 16512750
    SetPixelV m_hBtnSrcDC, 4, 68, 16512750
    SetPixelV m_hBtnSrcDC, 5, 68, 16513009
    SetPixelV m_hBtnSrcDC, 6, 68, 15254407
    SetPixelV m_hBtnSrcDC, 0, 69, 15254407
    SetPixelV m_hBtnSrcDC, 1, 69, 16513008
    SetPixelV m_hBtnSrcDC, 2, 69, 16512492
    SetPixelV m_hBtnSrcDC, 3, 69, 16512492
    SetPixelV m_hBtnSrcDC, 4, 69, 16512492
    SetPixelV m_hBtnSrcDC, 5, 69, 16513008
    SetPixelV m_hBtnSrcDC, 6, 69, 15254407
    SetPixelV m_hBtnSrcDC, 0, 70, 15254407
    SetPixelV m_hBtnSrcDC, 1, 70, 16512751
    SetPixelV m_hBtnSrcDC, 2, 70, 16512234
    SetPixelV m_hBtnSrcDC, 3, 70, 16512234
    SetPixelV m_hBtnSrcDC, 4, 70, 16512234
    SetPixelV m_hBtnSrcDC, 5, 70, 16512751
    SetPixelV m_hBtnSrcDC, 6, 70, 15254407
    SetPixelV m_hBtnSrcDC, 0, 71, 15254407
    SetPixelV m_hBtnSrcDC, 1, 71, 16512493
    SetPixelV m_hBtnSrcDC, 2, 71, 16511976
    SetPixelV m_hBtnSrcDC, 3, 71, 16511976
    SetPixelV m_hBtnSrcDC, 4, 71, 16511976
    SetPixelV m_hBtnSrcDC, 5, 71, 16512493
    SetPixelV m_hBtnSrcDC, 6, 71, 15254407
    SetPixelV m_hBtnSrcDC, 0, 72, 15254407
    SetPixelV m_hBtnSrcDC, 1, 72, 16512492
    SetPixelV m_hBtnSrcDC, 2, 72, 16446182
    SetPixelV m_hBtnSrcDC, 3, 72, 16446182
    SetPixelV m_hBtnSrcDC, 4, 72, 16446182
    SetPixelV m_hBtnSrcDC, 5, 72, 16512492
    SetPixelV m_hBtnSrcDC, 6, 72, 15254407
    SetPixelV m_hBtnSrcDC, 0, 73, 15254407
    SetPixelV m_hBtnSrcDC, 1, 73, 16512235
    SetPixelV m_hBtnSrcDC, 2, 73, 16445925
    SetPixelV m_hBtnSrcDC, 3, 73, 16445925
    SetPixelV m_hBtnSrcDC, 4, 73, 16445925
    SetPixelV m_hBtnSrcDC, 5, 73, 16512235
    SetPixelV m_hBtnSrcDC, 6, 73, 15254407
    SetPixelV m_hBtnSrcDC, 0, 74, 15254407
    SetPixelV m_hBtnSrcDC, 1, 74, 16445927
    SetPixelV m_hBtnSrcDC, 2, 74, 16313823
    SetPixelV m_hBtnSrcDC, 3, 74, 16313823
    SetPixelV m_hBtnSrcDC, 4, 74, 16313823
    SetPixelV m_hBtnSrcDC, 5, 74, 16445927
    SetPixelV m_hBtnSrcDC, 6, 74, 15254407
    SetPixelV m_hBtnSrcDC, 0, 75, 15254407
    SetPixelV m_hBtnSrcDC, 1, 75, 16380134
    SetPixelV m_hBtnSrcDC, 2, 75, 16247773
    SetPixelV m_hBtnSrcDC, 3, 75, 16247773
    SetPixelV m_hBtnSrcDC, 4, 75, 16247773
    SetPixelV m_hBtnSrcDC, 5, 75, 16380134
    SetPixelV m_hBtnSrcDC, 6, 75, 15254407
    SetPixelV m_hBtnSrcDC, 0, 76, 15254407
    SetPixelV m_hBtnSrcDC, 1, 76, 16314340
    SetPixelV m_hBtnSrcDC, 2, 76, 16181979
    SetPixelV m_hBtnSrcDC, 3, 76, 16181979
    SetPixelV m_hBtnSrcDC, 4, 76, 16181979
    SetPixelV m_hBtnSrcDC, 5, 76, 16314340
    SetPixelV m_hBtnSrcDC, 6, 76, 15254407
    SetPixelV m_hBtnSrcDC, 0, 77, 15254407
    SetPixelV m_hBtnSrcDC, 1, 77, 16313826
    SetPixelV m_hBtnSrcDC, 2, 77, 16115672
    SetPixelV m_hBtnSrcDC, 3, 77, 16115672
    SetPixelV m_hBtnSrcDC, 4, 77, 16115672
    SetPixelV m_hBtnSrcDC, 5, 77, 16313826
    SetPixelV m_hBtnSrcDC, 6, 77, 15254407
    SetPixelV m_hBtnSrcDC, 0, 78, 15254407
    SetPixelV m_hBtnSrcDC, 1, 78, 16248032
    SetPixelV m_hBtnSrcDC, 2, 78, 16049877
    SetPixelV m_hBtnSrcDC, 3, 78, 16049877
    SetPixelV m_hBtnSrcDC, 4, 78, 16049877
    SetPixelV m_hBtnSrcDC, 5, 78, 16248032
    SetPixelV m_hBtnSrcDC, 6, 78, 15254407
    SetPixelV m_hBtnSrcDC, 0, 79, 15254407
    SetPixelV m_hBtnSrcDC, 1, 79, 16182239
    SetPixelV m_hBtnSrcDC, 2, 79, 15983827
    SetPixelV m_hBtnSrcDC, 3, 79, 15983827
    SetPixelV m_hBtnSrcDC, 4, 79, 15983827
    SetPixelV m_hBtnSrcDC, 5, 79, 16182239
    SetPixelV m_hBtnSrcDC, 6, 79, 15254407
    SetPixelV m_hBtnSrcDC, 0, 80, 15254407
    SetPixelV m_hBtnSrcDC, 1, 80, 16181982
    SetPixelV m_hBtnSrcDC, 2, 80, 15983570
    SetPixelV m_hBtnSrcDC, 3, 80, 15983570
    SetPixelV m_hBtnSrcDC, 4, 80, 15983570
    SetPixelV m_hBtnSrcDC, 5, 80, 16181982
    SetPixelV m_hBtnSrcDC, 6, 80, 15254407
    SetPixelV m_hBtnSrcDC, 0, 81, 15320717
    SetPixelV m_hBtnSrcDC, 1, 81, 16115670
    SetPixelV m_hBtnSrcDC, 2, 81, 16049879
    SetPixelV m_hBtnSrcDC, 3, 81, 15918033
    SetPixelV m_hBtnSrcDC, 4, 81, 16049879
    SetPixelV m_hBtnSrcDC, 5, 81, 16115670
    SetPixelV m_hBtnSrcDC, 6, 81, 15320717
    SetPixelV m_hBtnSrcDC, 0, 82, 15717291
    SetPixelV m_hBtnSrcDC, 1, 82, 15652013
    SetPixelV m_hBtnSrcDC, 2, 82, 16115671
    SetPixelV m_hBtnSrcDC, 3, 82, 16182239
    SetPixelV m_hBtnSrcDC, 4, 82, 16115671
    SetPixelV m_hBtnSrcDC, 5, 82, 15652013
    SetPixelV m_hBtnSrcDC, 6, 82, 15651754
    SetPixelV m_hBtnSrcDC, 0, 83, 16512754
    SetPixelV m_hBtnSrcDC, 1, 83, 15651497
    SetPixelV m_hBtnSrcDC, 2, 83, 15386254
    SetPixelV m_hBtnSrcDC, 3, 83, 15254407
    SetPixelV m_hBtnSrcDC, 4, 83, 15386254
    SetPixelV m_hBtnSrcDC, 5, 83, 15651497
    SetPixelV m_hBtnSrcDC, 6, 83, 16512754
    
    SetPixelV m_hCbbSrcDC, 0, 0, 13738062
    SetPixelV m_hCbbSrcDC, 1, 0, 16775656
    SetPixelV m_hCbbSrcDC, 2, 0, 16775656
    SetPixelV m_hCbbSrcDC, 3, 0, 16775656
    SetPixelV m_hCbbSrcDC, 4, 0, 13738062
    SetPixelV m_hCbbSrcDC, 5, 0, 15983038
    SetPixelV m_hCbbSrcDC, 6, 0, 15983038
    SetPixelV m_hCbbSrcDC, 7, 0, 15983038
    SetPixelV m_hCbbSrcDC, 8, 0, 6247705
    SetPixelV m_hCbbSrcDC, 9, 0, 6247705
    SetPixelV m_hCbbSrcDC, 10, 0, 6247705
    SetPixelV m_hCbbSrcDC, 11, 0, 6247705
    SetPixelV m_hCbbSrcDC, 12, 0, 6247705
    SetPixelV m_hCbbSrcDC, 13, 0, 6247705
    SetPixelV m_hCbbSrcDC, 14, 0, 6247705
    SetPixelV m_hCbbSrcDC, 0, 1, 13738062
    SetPixelV m_hCbbSrcDC, 1, 1, 16709862
    SetPixelV m_hCbbSrcDC, 2, 1, 16641729
    SetPixelV m_hCbbSrcDC, 3, 1, 16709862
    SetPixelV m_hCbbSrcDC, 4, 1, 13738062
    SetPixelV m_hCbbSrcDC, 5, 1, 15983040
    SetPixelV m_hCbbSrcDC, 6, 1, 14857569
    SetPixelV m_hCbbSrcDC, 7, 1, 15983040
    SetPixelV m_hCbbSrcDC, 8, 1, 16711935
    SetPixelV m_hCbbSrcDC, 9, 1, 6247705
    SetPixelV m_hCbbSrcDC, 10, 1, 6247705
    SetPixelV m_hCbbSrcDC, 11, 1, 6247705
    SetPixelV m_hCbbSrcDC, 12, 1, 6247705
    SetPixelV m_hCbbSrcDC, 13, 1, 6247705
    SetPixelV m_hCbbSrcDC, 14, 1, 16711935
    SetPixelV m_hCbbSrcDC, 0, 2, 13738062
    SetPixelV m_hCbbSrcDC, 1, 2, 16709604
    SetPixelV m_hCbbSrcDC, 2, 2, 16575420
    SetPixelV m_hCbbSrcDC, 3, 2, 16709604
    SetPixelV m_hCbbSrcDC, 4, 2, 13738062
    SetPixelV m_hCbbSrcDC, 5, 2, 16049089
    SetPixelV m_hCbbSrcDC, 6, 2, 14923877
    SetPixelV m_hCbbSrcDC, 7, 2, 16049089
    SetPixelV m_hCbbSrcDC, 8, 2, 16711935
    SetPixelV m_hCbbSrcDC, 9, 2, 16711935
    SetPixelV m_hCbbSrcDC, 10, 2, 6247705
    SetPixelV m_hCbbSrcDC, 11, 2, 6247705
    SetPixelV m_hCbbSrcDC, 12, 2, 6247705
    SetPixelV m_hCbbSrcDC, 13, 2, 16711935
    SetPixelV m_hCbbSrcDC, 14, 2, 16711935
    SetPixelV m_hCbbSrcDC, 0, 3, 13738062
    SetPixelV m_hCbbSrcDC, 1, 3, 16643810
    SetPixelV m_hCbbSrcDC, 2, 3, 16443575
    SetPixelV m_hCbbSrcDC, 3, 3, 16643810
    SetPixelV m_hCbbSrcDC, 4, 3, 13738062
    SetPixelV m_hCbbSrcDC, 5, 3, 16114883
    SetPixelV m_hCbbSrcDC, 6, 3, 15055722
    SetPixelV m_hCbbSrcDC, 7, 3, 16114883
    SetPixelV m_hCbbSrcDC, 8, 3, 16711935
    SetPixelV m_hCbbSrcDC, 9, 3, 16711935
    SetPixelV m_hCbbSrcDC, 10, 3, 16711935
    SetPixelV m_hCbbSrcDC, 11, 3, 6247705
    SetPixelV m_hCbbSrcDC, 12, 3, 16711935
    SetPixelV m_hCbbSrcDC, 13, 3, 16711935
    SetPixelV m_hCbbSrcDC, 14, 3, 16711935
    SetPixelV m_hCbbSrcDC, 0, 4, 13738062
    SetPixelV m_hCbbSrcDC, 1, 4, 16643552
    SetPixelV m_hCbbSrcDC, 2, 4, 16377266
    SetPixelV m_hCbbSrcDC, 3, 4, 16643552
    SetPixelV m_hCbbSrcDC, 4, 4, 13738062
    SetPixelV m_hCbbSrcDC, 5, 4, 16115142
    SetPixelV m_hCbbSrcDC, 6, 4, 15122032
    SetPixelV m_hCbbSrcDC, 7, 4, 16115142
    SetPixelV m_hCbbSrcDC, 8, 4, 11314564
    SetPixelV m_hCbbSrcDC, 9, 4, 11314564
    SetPixelV m_hCbbSrcDC, 10, 4, 11314564
    SetPixelV m_hCbbSrcDC, 11, 4, 11314564
    SetPixelV m_hCbbSrcDC, 12, 4, 11314564
    SetPixelV m_hCbbSrcDC, 13, 4, 11314564
    SetPixelV m_hCbbSrcDC, 14, 4, 11314564
    SetPixelV m_hCbbSrcDC, 0, 5, 13738062
    SetPixelV m_hCbbSrcDC, 1, 5, 16577502
    SetPixelV m_hCbbSrcDC, 2, 5, 16245164
    SetPixelV m_hCbbSrcDC, 3, 5, 16577502
    SetPixelV m_hCbbSrcDC, 4, 5, 13738062
    SetPixelV m_hCbbSrcDC, 5, 5, 16181192
    SetPixelV m_hCbbSrcDC, 6, 5, 15254134
    SetPixelV m_hCbbSrcDC, 7, 5, 16181192
    SetPixelV m_hCbbSrcDC, 8, 5, 16711935
    SetPixelV m_hCbbSrcDC, 9, 5, 11314564
    SetPixelV m_hCbbSrcDC, 10, 5, 11314564
    SetPixelV m_hCbbSrcDC, 11, 5, 11314564
    SetPixelV m_hCbbSrcDC, 12, 5, 11314564
    SetPixelV m_hCbbSrcDC, 13, 5, 11314564
    SetPixelV m_hCbbSrcDC, 14, 5, 16711935
    SetPixelV m_hCbbSrcDC, 0, 6, 13738062
    SetPixelV m_hCbbSrcDC, 1, 6, 16511451
    SetPixelV m_hCbbSrcDC, 2, 6, 16113061
    SetPixelV m_hCbbSrcDC, 3, 6, 16511451
    SetPixelV m_hCbbSrcDC, 4, 6, 13738062
    SetPixelV m_hCbbSrcDC, 5, 6, 16246987
    SetPixelV m_hCbbSrcDC, 6, 6, 15385980
    SetPixelV m_hCbbSrcDC, 7, 6, 16246987
    SetPixelV m_hCbbSrcDC, 8, 6, 16711935
    SetPixelV m_hCbbSrcDC, 9, 6, 16711935
    SetPixelV m_hCbbSrcDC, 10, 6, 11314564
    SetPixelV m_hCbbSrcDC, 11, 6, 11314564
    SetPixelV m_hCbbSrcDC, 12, 6, 11314564
    SetPixelV m_hCbbSrcDC, 13, 6, 16711935
    SetPixelV m_hCbbSrcDC, 14, 6, 16711935
    SetPixelV m_hCbbSrcDC, 0, 7, 13738062
    SetPixelV m_hCbbSrcDC, 1, 7, 16445657
    SetPixelV m_hCbbSrcDC, 2, 7, 15980959
    SetPixelV m_hCbbSrcDC, 3, 7, 16445657
    SetPixelV m_hCbbSrcDC, 4, 7, 13738062
    SetPixelV m_hCbbSrcDC, 5, 7, 16247245
    SetPixelV m_hCbbSrcDC, 6, 7, 15518083
    SetPixelV m_hCbbSrcDC, 7, 7, 16247245
    SetPixelV m_hCbbSrcDC, 8, 7, 16711935
    SetPixelV m_hCbbSrcDC, 9, 7, 16711935
    SetPixelV m_hCbbSrcDC, 10, 7, 16711935
    SetPixelV m_hCbbSrcDC, 11, 7, 11314564
    SetPixelV m_hCbbSrcDC, 12, 7, 16711935
    SetPixelV m_hCbbSrcDC, 13, 7, 16711935
    SetPixelV m_hCbbSrcDC, 14, 7, 16711935
    SetPixelV m_hCbbSrcDC, 0, 8, 13738062
    SetPixelV m_hCbbSrcDC, 1, 8, 16379606
    SetPixelV m_hCbbSrcDC, 2, 8, 15848856
    SetPixelV m_hCbbSrcDC, 3, 8, 16379606
    SetPixelV m_hCbbSrcDC, 4, 8, 13738062
    SetPixelV m_hCbbSrcDC, 5, 8, 16313296
    SetPixelV m_hCbbSrcDC, 6, 8, 15650186
    SetPixelV m_hCbbSrcDC, 7, 8, 16313296
    SetPixelV m_hCbbSrcDC, 8, 8, 16711935
    SetPixelV m_hCbbSrcDC, 9, 8, 16711935
    SetPixelV m_hCbbSrcDC, 10, 8, 16711935
    SetPixelV m_hCbbSrcDC, 11, 8, 16711935
    SetPixelV m_hCbbSrcDC, 12, 8, 16711935
    SetPixelV m_hCbbSrcDC, 13, 8, 16711935
    SetPixelV m_hCbbSrcDC, 14, 8, 16711935
    SetPixelV m_hCbbSrcDC, 0, 9, 13738062
    SetPixelV m_hCbbSrcDC, 1, 9, 16379347
    SetPixelV m_hCbbSrcDC, 2, 9, 15716753
    SetPixelV m_hCbbSrcDC, 3, 9, 16379347
    SetPixelV m_hCbbSrcDC, 4, 9, 13738062
    SetPixelV m_hCbbSrcDC, 5, 9, 16379347
    SetPixelV m_hCbbSrcDC, 6, 9, 15716753
    SetPixelV m_hCbbSrcDC, 7, 9, 16379347
    SetPixelV m_hCbbSrcDC, 8, 9, 16711935
    SetPixelV m_hCbbSrcDC, 9, 9, 16711935
    SetPixelV m_hCbbSrcDC, 10, 9, 16711935
    SetPixelV m_hCbbSrcDC, 11, 9, 16711935
    SetPixelV m_hCbbSrcDC, 12, 9, 16711935
    SetPixelV m_hCbbSrcDC, 13, 9, 16711935
    SetPixelV m_hCbbSrcDC, 14, 9, 16711935
    SetPixelV m_hCbbSrcDC, 0, 10, 13738062
    SetPixelV m_hCbbSrcDC, 1, 10, 16313296
    SetPixelV m_hCbbSrcDC, 2, 10, 15650186
    SetPixelV m_hCbbSrcDC, 3, 10, 16313296
    SetPixelV m_hCbbSrcDC, 4, 10, 13738062
    SetPixelV m_hCbbSrcDC, 5, 10, 16379606
    SetPixelV m_hCbbSrcDC, 6, 10, 15848856
    SetPixelV m_hCbbSrcDC, 7, 10, 16379606
    SetPixelV m_hCbbSrcDC, 8, 10, 16711935
    SetPixelV m_hCbbSrcDC, 9, 10, 16711935
    SetPixelV m_hCbbSrcDC, 10, 10, 16711935
    SetPixelV m_hCbbSrcDC, 11, 10, 16711935
    SetPixelV m_hCbbSrcDC, 12, 10, 16711935
    SetPixelV m_hCbbSrcDC, 13, 10, 16711935
    SetPixelV m_hCbbSrcDC, 14, 10, 16711935
    SetPixelV m_hCbbSrcDC, 0, 11, 13738062
    SetPixelV m_hCbbSrcDC, 1, 11, 16247245
    SetPixelV m_hCbbSrcDC, 2, 11, 15518083
    SetPixelV m_hCbbSrcDC, 3, 11, 16247245
    SetPixelV m_hCbbSrcDC, 4, 11, 13738062
    SetPixelV m_hCbbSrcDC, 5, 11, 16445657
    SetPixelV m_hCbbSrcDC, 6, 11, 15980959
    SetPixelV m_hCbbSrcDC, 7, 11, 16445657
    SetPixelV m_hCbbSrcDC, 8, 11, 16711935
    SetPixelV m_hCbbSrcDC, 9, 11, 16711935
    SetPixelV m_hCbbSrcDC, 10, 11, 16711935
    SetPixelV m_hCbbSrcDC, 11, 11, 16711935
    SetPixelV m_hCbbSrcDC, 12, 11, 16711935
    SetPixelV m_hCbbSrcDC, 13, 11, 16711935
    SetPixelV m_hCbbSrcDC, 14, 11, 16711935
    SetPixelV m_hCbbSrcDC, 0, 12, 13738062
    SetPixelV m_hCbbSrcDC, 1, 12, 16246987
    SetPixelV m_hCbbSrcDC, 2, 12, 15385980
    SetPixelV m_hCbbSrcDC, 3, 12, 16246987
    SetPixelV m_hCbbSrcDC, 4, 12, 13738062
    SetPixelV m_hCbbSrcDC, 5, 12, 16511451
    SetPixelV m_hCbbSrcDC, 6, 12, 16113061
    SetPixelV m_hCbbSrcDC, 7, 12, 16511451
    SetPixelV m_hCbbSrcDC, 8, 12, 16711935
    SetPixelV m_hCbbSrcDC, 9, 12, 16711935
    SetPixelV m_hCbbSrcDC, 10, 12, 16711935
    SetPixelV m_hCbbSrcDC, 11, 12, 16711935
    SetPixelV m_hCbbSrcDC, 12, 12, 16711935
    SetPixelV m_hCbbSrcDC, 13, 12, 16711935
    SetPixelV m_hCbbSrcDC, 14, 12, 16711935
    SetPixelV m_hCbbSrcDC, 0, 13, 13738062
    SetPixelV m_hCbbSrcDC, 1, 13, 16181192
    SetPixelV m_hCbbSrcDC, 2, 13, 15254134
    SetPixelV m_hCbbSrcDC, 3, 13, 16181192
    SetPixelV m_hCbbSrcDC, 4, 13, 13738062
    SetPixelV m_hCbbSrcDC, 5, 13, 16577502
    SetPixelV m_hCbbSrcDC, 6, 13, 16245164
    SetPixelV m_hCbbSrcDC, 7, 13, 16577502
    SetPixelV m_hCbbSrcDC, 8, 13, 16711935
    SetPixelV m_hCbbSrcDC, 9, 13, 16711935
    SetPixelV m_hCbbSrcDC, 10, 13, 16711935
    SetPixelV m_hCbbSrcDC, 11, 13, 16711935
    SetPixelV m_hCbbSrcDC, 12, 13, 16711935
    SetPixelV m_hCbbSrcDC, 13, 13, 16711935
    SetPixelV m_hCbbSrcDC, 14, 13, 16711935
    SetPixelV m_hCbbSrcDC, 0, 14, 13738062
    SetPixelV m_hCbbSrcDC, 1, 14, 16115142
    SetPixelV m_hCbbSrcDC, 2, 14, 15122032
    SetPixelV m_hCbbSrcDC, 3, 14, 16115142
    SetPixelV m_hCbbSrcDC, 4, 14, 13738062
    SetPixelV m_hCbbSrcDC, 5, 14, 16643552
    SetPixelV m_hCbbSrcDC, 6, 14, 16377266
    SetPixelV m_hCbbSrcDC, 7, 14, 16643552
    SetPixelV m_hCbbSrcDC, 8, 14, 16711935
    SetPixelV m_hCbbSrcDC, 9, 14, 16711935
    SetPixelV m_hCbbSrcDC, 10, 14, 16711935
    SetPixelV m_hCbbSrcDC, 11, 14, 16711935
    SetPixelV m_hCbbSrcDC, 12, 14, 16711935
    SetPixelV m_hCbbSrcDC, 13, 14, 16711935
    SetPixelV m_hCbbSrcDC, 14, 14, 16711935
    SetPixelV m_hCbbSrcDC, 0, 15, 13738062
    SetPixelV m_hCbbSrcDC, 1, 15, 16114883
    SetPixelV m_hCbbSrcDC, 2, 15, 15055722
    SetPixelV m_hCbbSrcDC, 3, 15, 16114883
    SetPixelV m_hCbbSrcDC, 4, 15, 13738062
    SetPixelV m_hCbbSrcDC, 5, 15, 16643810
    SetPixelV m_hCbbSrcDC, 6, 15, 16443575
    SetPixelV m_hCbbSrcDC, 7, 15, 16643810
    SetPixelV m_hCbbSrcDC, 8, 15, 16711935
    SetPixelV m_hCbbSrcDC, 9, 15, 16711935
    SetPixelV m_hCbbSrcDC, 10, 15, 16711935
    SetPixelV m_hCbbSrcDC, 11, 15, 16711935
    SetPixelV m_hCbbSrcDC, 12, 15, 16711935
    SetPixelV m_hCbbSrcDC, 13, 15, 16711935
    SetPixelV m_hCbbSrcDC, 14, 15, 16711935
    SetPixelV m_hCbbSrcDC, 0, 16, 13738062
    SetPixelV m_hCbbSrcDC, 1, 16, 16049089
    SetPixelV m_hCbbSrcDC, 2, 16, 14923877
    SetPixelV m_hCbbSrcDC, 3, 16, 16049089
    SetPixelV m_hCbbSrcDC, 4, 16, 13738062
    SetPixelV m_hCbbSrcDC, 5, 16, 16709604
    SetPixelV m_hCbbSrcDC, 6, 16, 16575420
    SetPixelV m_hCbbSrcDC, 7, 16, 16709604
    SetPixelV m_hCbbSrcDC, 8, 16, 16711935
    SetPixelV m_hCbbSrcDC, 9, 16, 16711935
    SetPixelV m_hCbbSrcDC, 10, 16, 16711935
    SetPixelV m_hCbbSrcDC, 11, 16, 16711935
    SetPixelV m_hCbbSrcDC, 12, 16, 16711935
    SetPixelV m_hCbbSrcDC, 13, 16, 16711935
    SetPixelV m_hCbbSrcDC, 14, 16, 16711935
    SetPixelV m_hCbbSrcDC, 0, 17, 13738062
    SetPixelV m_hCbbSrcDC, 1, 17, 15983040
    SetPixelV m_hCbbSrcDC, 2, 17, 15983040
    SetPixelV m_hCbbSrcDC, 3, 17, 15983040
    SetPixelV m_hCbbSrcDC, 4, 17, 13738062
    SetPixelV m_hCbbSrcDC, 5, 17, 16709862
    SetPixelV m_hCbbSrcDC, 6, 17, 16709862
    SetPixelV m_hCbbSrcDC, 7, 17, 16709862
    SetPixelV m_hCbbSrcDC, 8, 17, 16711935
    SetPixelV m_hCbbSrcDC, 9, 17, 16711935
    SetPixelV m_hCbbSrcDC, 10, 17, 16711935
    SetPixelV m_hCbbSrcDC, 11, 17, 16711935
    SetPixelV m_hCbbSrcDC, 12, 17, 16711935
    SetPixelV m_hCbbSrcDC, 13, 17, 16711935
    SetPixelV m_hCbbSrcDC, 14, 17, 16711935
    
    SetPixelV m_hCkbSrcDC, 0, 0, 16711935
    SetPixelV m_hCkbSrcDC, 1, 0, 16711935
    SetPixelV m_hCkbSrcDC, 2, 0, 16711935
    SetPixelV m_hCkbSrcDC, 3, 0, 16711935
    SetPixelV m_hCkbSrcDC, 4, 0, 16711935
    SetPixelV m_hCkbSrcDC, 5, 0, 16711935
    SetPixelV m_hCkbSrcDC, 6, 0, 16711935
    SetPixelV m_hCkbSrcDC, 7, 0, 16711935
    SetPixelV m_hCkbSrcDC, 8, 0, 1947476
    SetPixelV m_hCkbSrcDC, 9, 0, 16711935
    SetPixelV m_hCkbSrcDC, 10, 0, 16711935
    SetPixelV m_hCkbSrcDC, 11, 0, 16711935
    SetPixelV m_hCkbSrcDC, 12, 0, 16711935
    SetPixelV m_hCkbSrcDC, 13, 0, 16711935
    SetPixelV m_hCkbSrcDC, 14, 0, 16711935
    SetPixelV m_hCkbSrcDC, 15, 0, 16711935
    SetPixelV m_hCkbSrcDC, 16, 0, 16711935
    SetPixelV m_hCkbSrcDC, 17, 0, 9231016
    SetPixelV m_hCkbSrcDC, 0, 1, 16711935
    SetPixelV m_hCkbSrcDC, 1, 1, 16711935
    SetPixelV m_hCkbSrcDC, 2, 1, 16711935
    SetPixelV m_hCkbSrcDC, 3, 1, 16711935
    SetPixelV m_hCkbSrcDC, 4, 1, 16711935
    SetPixelV m_hCkbSrcDC, 5, 1, 16711935
    SetPixelV m_hCkbSrcDC, 6, 1, 16711935
    SetPixelV m_hCkbSrcDC, 7, 1, 1815633
    SetPixelV m_hCkbSrcDC, 8, 1, 1815633
    SetPixelV m_hCkbSrcDC, 9, 1, 16711935
    SetPixelV m_hCkbSrcDC, 10, 1, 16711935
    SetPixelV m_hCkbSrcDC, 11, 1, 16711935
    SetPixelV m_hCkbSrcDC, 12, 1, 16711935
    SetPixelV m_hCkbSrcDC, 13, 1, 16711935
    SetPixelV m_hCkbSrcDC, 14, 1, 16711935
    SetPixelV m_hCkbSrcDC, 15, 1, 16711935
    SetPixelV m_hCkbSrcDC, 16, 1, 9165222
    SetPixelV m_hCkbSrcDC, 17, 1, 9165222
    SetPixelV m_hCkbSrcDC, 0, 2, 16711935
    SetPixelV m_hCkbSrcDC, 1, 2, 16711935
    SetPixelV m_hCkbSrcDC, 2, 2, 16711935
    SetPixelV m_hCkbSrcDC, 3, 2, 16711935
    SetPixelV m_hCkbSrcDC, 4, 2, 16711935
    SetPixelV m_hCkbSrcDC, 5, 2, 16711935
    SetPixelV m_hCkbSrcDC, 6, 2, 1552205
    SetPixelV m_hCkbSrcDC, 7, 2, 1552205
    SetPixelV m_hCkbSrcDC, 8, 2, 1552205
    SetPixelV m_hCkbSrcDC, 9, 2, 16711935
    SetPixelV m_hCkbSrcDC, 10, 2, 16711935
    SetPixelV m_hCkbSrcDC, 11, 2, 16711935
    SetPixelV m_hCkbSrcDC, 12, 2, 16711935
    SetPixelV m_hCkbSrcDC, 13, 2, 16711935
    SetPixelV m_hCkbSrcDC, 14, 2, 16711935
    SetPixelV m_hCkbSrcDC, 15, 2, 9033380
    SetPixelV m_hCkbSrcDC, 16, 2, 9033380
    SetPixelV m_hCkbSrcDC, 17, 2, 9033380
    SetPixelV m_hCkbSrcDC, 0, 3, 1288520
    SetPixelV m_hCkbSrcDC, 1, 3, 1288520
    SetPixelV m_hCkbSrcDC, 2, 3, 16711935
    SetPixelV m_hCkbSrcDC, 3, 3, 16711935
    SetPixelV m_hCkbSrcDC, 4, 3, 16711935
    SetPixelV m_hCkbSrcDC, 5, 3, 1288520
    SetPixelV m_hCkbSrcDC, 6, 3, 1288520
    SetPixelV m_hCkbSrcDC, 7, 3, 1288520
    SetPixelV m_hCkbSrcDC, 8, 3, 16711935
    SetPixelV m_hCkbSrcDC, 9, 3, 8901538
    SetPixelV m_hCkbSrcDC, 10, 3, 8901538
    SetPixelV m_hCkbSrcDC, 11, 3, 16711935
    SetPixelV m_hCkbSrcDC, 12, 3, 16711935
    SetPixelV m_hCkbSrcDC, 13, 3, 16711935
    SetPixelV m_hCkbSrcDC, 14, 3, 8901538
    SetPixelV m_hCkbSrcDC, 15, 3, 8901538
    SetPixelV m_hCkbSrcDC, 16, 3, 8901538
    SetPixelV m_hCkbSrcDC, 17, 3, 16711935
    SetPixelV m_hCkbSrcDC, 0, 4, 1024579
    SetPixelV m_hCkbSrcDC, 1, 4, 1024579
    SetPixelV m_hCkbSrcDC, 2, 4, 1024579
    SetPixelV m_hCkbSrcDC, 3, 4, 16711935
    SetPixelV m_hCkbSrcDC, 4, 4, 1024579
    SetPixelV m_hCkbSrcDC, 5, 4, 1024579
    SetPixelV m_hCkbSrcDC, 6, 4, 1024579
    SetPixelV m_hCkbSrcDC, 7, 4, 16711935
    SetPixelV m_hCkbSrcDC, 8, 4, 16711935
    SetPixelV m_hCkbSrcDC, 9, 4, 8769695
    SetPixelV m_hCkbSrcDC, 10, 4, 8769695
    SetPixelV m_hCkbSrcDC, 11, 4, 8769695
    SetPixelV m_hCkbSrcDC, 12, 4, 16711935
    SetPixelV m_hCkbSrcDC, 13, 4, 8769695
    SetPixelV m_hCkbSrcDC, 14, 4, 8769695
    SetPixelV m_hCkbSrcDC, 15, 4, 8769695
    SetPixelV m_hCkbSrcDC, 16, 4, 16711935
    SetPixelV m_hCkbSrcDC, 17, 4, 16711935
    SetPixelV m_hCkbSrcDC, 0, 5, 16711935
    SetPixelV m_hCkbSrcDC, 1, 5, 695358
    SetPixelV m_hCkbSrcDC, 2, 5, 695358
    SetPixelV m_hCkbSrcDC, 3, 5, 695358
    SetPixelV m_hCkbSrcDC, 4, 5, 695358
    SetPixelV m_hCkbSrcDC, 5, 5, 695358
    SetPixelV m_hCkbSrcDC, 6, 5, 16711935
    SetPixelV m_hCkbSrcDC, 7, 5, 16711935
    SetPixelV m_hCkbSrcDC, 8, 5, 16711935
    SetPixelV m_hCkbSrcDC, 9, 5, 16711935
    SetPixelV m_hCkbSrcDC, 10, 5, 8572317
    SetPixelV m_hCkbSrcDC, 11, 5, 8572317
    SetPixelV m_hCkbSrcDC, 12, 5, 8572317
    SetPixelV m_hCkbSrcDC, 13, 5, 8572317
    SetPixelV m_hCkbSrcDC, 14, 5, 8572317
    SetPixelV m_hCkbSrcDC, 15, 5, 16711935
    SetPixelV m_hCkbSrcDC, 16, 5, 16711935
    SetPixelV m_hCkbSrcDC, 17, 5, 16711935
    SetPixelV m_hCkbSrcDC, 0, 6, 16711935
    SetPixelV m_hCkbSrcDC, 1, 6, 16711935
    SetPixelV m_hCkbSrcDC, 2, 6, 431673
    SetPixelV m_hCkbSrcDC, 3, 6, 431673
    SetPixelV m_hCkbSrcDC, 4, 6, 431673
    SetPixelV m_hCkbSrcDC, 5, 6, 16711935
    SetPixelV m_hCkbSrcDC, 6, 6, 16711935
    SetPixelV m_hCkbSrcDC, 7, 6, 16711935
    SetPixelV m_hCkbSrcDC, 8, 6, 16711935
    SetPixelV m_hCkbSrcDC, 9, 6, 16711935
    SetPixelV m_hCkbSrcDC, 10, 6, 16711935
    SetPixelV m_hCkbSrcDC, 11, 6, 8440218
    SetPixelV m_hCkbSrcDC, 12, 6, 8440218
    SetPixelV m_hCkbSrcDC, 13, 6, 8440218
    SetPixelV m_hCkbSrcDC, 14, 6, 16711935
    SetPixelV m_hCkbSrcDC, 15, 6, 16711935
    SetPixelV m_hCkbSrcDC, 16, 6, 16711935
    SetPixelV m_hCkbSrcDC, 17, 6, 16711935
    SetPixelV m_hCkbSrcDC, 0, 7, 16711935
    SetPixelV m_hCkbSrcDC, 1, 7, 16711935
    SetPixelV m_hCkbSrcDC, 2, 7, 16711935
    SetPixelV m_hCkbSrcDC, 3, 7, 168245
    SetPixelV m_hCkbSrcDC, 4, 7, 16711935
    SetPixelV m_hCkbSrcDC, 5, 7, 16711935
    SetPixelV m_hCkbSrcDC, 6, 7, 16711935
    SetPixelV m_hCkbSrcDC, 7, 7, 16711935
    SetPixelV m_hCkbSrcDC, 8, 7, 16711935
    SetPixelV m_hCkbSrcDC, 9, 7, 16711935
    SetPixelV m_hCkbSrcDC, 10, 7, 16711935
    SetPixelV m_hCkbSrcDC, 11, 7, 16711935
    SetPixelV m_hCkbSrcDC, 12, 7, 8308632
    SetPixelV m_hCkbSrcDC, 13, 7, 16711935
    SetPixelV m_hCkbSrcDC, 14, 7, 16711935
    SetPixelV m_hCkbSrcDC, 15, 7, 16711935
    SetPixelV m_hCkbSrcDC, 16, 7, 16711935
    SetPixelV m_hCkbSrcDC, 17, 7, 16711935
    SetPixelV m_hCkbSrcDC, 0, 8, 16711935
    SetPixelV m_hCkbSrcDC, 1, 8, 16711935
    SetPixelV m_hCkbSrcDC, 2, 8, 16711935
    SetPixelV m_hCkbSrcDC, 3, 8, 16711935
    SetPixelV m_hCkbSrcDC, 4, 8, 16711935
    SetPixelV m_hCkbSrcDC, 5, 8, 16711935
    SetPixelV m_hCkbSrcDC, 6, 8, 16711935
    SetPixelV m_hCkbSrcDC, 7, 8, 16711935
    SetPixelV m_hCkbSrcDC, 8, 8, 16711935
    SetPixelV m_hCkbSrcDC, 9, 8, 16711935
    SetPixelV m_hCkbSrcDC, 10, 8, 16711935
    SetPixelV m_hCkbSrcDC, 11, 8, 16711935
    SetPixelV m_hCkbSrcDC, 12, 8, 16711935
    SetPixelV m_hCkbSrcDC, 13, 8, 16711935
    SetPixelV m_hCkbSrcDC, 14, 8, 16711935
    SetPixelV m_hCkbSrcDC, 15, 8, 16711935
    SetPixelV m_hCkbSrcDC, 16, 8, 16711935
    SetPixelV m_hCkbSrcDC, 17, 8, 16711935
    SetPixelV m_hCkbSrcDC, 0, 9, 16711935
    SetPixelV m_hCkbSrcDC, 1, 9, 1815633
    SetPixelV m_hCkbSrcDC, 2, 9, 1815633
    SetPixelV m_hCkbSrcDC, 3, 9, 1815633
    SetPixelV m_hCkbSrcDC, 4, 9, 1815633
    SetPixelV m_hCkbSrcDC, 5, 9, 1815633
    SetPixelV m_hCkbSrcDC, 6, 9, 1815633
    SetPixelV m_hCkbSrcDC, 7, 9, 1815633
    SetPixelV m_hCkbSrcDC, 8, 9, 16711935
    SetPixelV m_hCkbSrcDC, 9, 9, 16711935
    SetPixelV m_hCkbSrcDC, 10, 9, 9165222
    SetPixelV m_hCkbSrcDC, 11, 9, 9165222
    SetPixelV m_hCkbSrcDC, 12, 9, 9165222
    SetPixelV m_hCkbSrcDC, 13, 9, 9165222
    SetPixelV m_hCkbSrcDC, 14, 9, 9165222
    SetPixelV m_hCkbSrcDC, 15, 9, 9165222
    SetPixelV m_hCkbSrcDC, 16, 9, 9165222
    SetPixelV m_hCkbSrcDC, 17, 9, 16711935
    SetPixelV m_hCkbSrcDC, 0, 10, 16711935
    SetPixelV m_hCkbSrcDC, 1, 10, 1617998
    SetPixelV m_hCkbSrcDC, 2, 10, 1617998
    SetPixelV m_hCkbSrcDC, 3, 10, 1617998
    SetPixelV m_hCkbSrcDC, 4, 10, 1617998
    SetPixelV m_hCkbSrcDC, 5, 10, 1617998
    SetPixelV m_hCkbSrcDC, 6, 10, 1617998
    SetPixelV m_hCkbSrcDC, 7, 10, 1617998
    SetPixelV m_hCkbSrcDC, 8, 10, 16711935
    SetPixelV m_hCkbSrcDC, 9, 10, 16711935
    SetPixelV m_hCkbSrcDC, 10, 10, 9033380
    SetPixelV m_hCkbSrcDC, 11, 10, 9033380
    SetPixelV m_hCkbSrcDC, 12, 10, 9033380
    SetPixelV m_hCkbSrcDC, 13, 10, 9033380
    SetPixelV m_hCkbSrcDC, 14, 10, 9033380
    SetPixelV m_hCkbSrcDC, 15, 10, 9033380
    SetPixelV m_hCkbSrcDC, 16, 10, 9033380
    SetPixelV m_hCkbSrcDC, 17, 10, 16711935
    SetPixelV m_hCkbSrcDC, 0, 11, 16711935
    SetPixelV m_hCkbSrcDC, 1, 11, 1354569
    SetPixelV m_hCkbSrcDC, 2, 11, 1354569
    SetPixelV m_hCkbSrcDC, 3, 11, 1354569
    SetPixelV m_hCkbSrcDC, 4, 11, 1354569
    SetPixelV m_hCkbSrcDC, 5, 11, 1354569
    SetPixelV m_hCkbSrcDC, 6, 11, 1354569
    SetPixelV m_hCkbSrcDC, 7, 11, 1354569
    SetPixelV m_hCkbSrcDC, 8, 11, 16711935
    SetPixelV m_hCkbSrcDC, 9, 11, 16711935
    SetPixelV m_hCkbSrcDC, 10, 11, 8901795
    SetPixelV m_hCkbSrcDC, 11, 11, 8901795
    SetPixelV m_hCkbSrcDC, 12, 11, 8901795
    SetPixelV m_hCkbSrcDC, 13, 11, 8901795
    SetPixelV m_hCkbSrcDC, 14, 11, 8901795
    SetPixelV m_hCkbSrcDC, 15, 11, 8901795
    SetPixelV m_hCkbSrcDC, 16, 11, 8901795
    SetPixelV m_hCkbSrcDC, 17, 11, 16711935
    SetPixelV m_hCkbSrcDC, 0, 12, 16711935
    SetPixelV m_hCkbSrcDC, 1, 12, 1090629
    SetPixelV m_hCkbSrcDC, 2, 12, 1090629
    SetPixelV m_hCkbSrcDC, 3, 12, 1090629
    SetPixelV m_hCkbSrcDC, 4, 12, 1090629
    SetPixelV m_hCkbSrcDC, 5, 12, 1090629
    SetPixelV m_hCkbSrcDC, 6, 12, 1090629
    SetPixelV m_hCkbSrcDC, 7, 12, 1090629
    SetPixelV m_hCkbSrcDC, 8, 12, 16711935
    SetPixelV m_hCkbSrcDC, 9, 12, 16711935
    SetPixelV m_hCkbSrcDC, 10, 12, 8835488
    SetPixelV m_hCkbSrcDC, 11, 12, 8835488
    SetPixelV m_hCkbSrcDC, 12, 12, 8835488
    SetPixelV m_hCkbSrcDC, 13, 12, 8835488
    SetPixelV m_hCkbSrcDC, 14, 12, 8835488
    SetPixelV m_hCkbSrcDC, 15, 12, 8835488
    SetPixelV m_hCkbSrcDC, 16, 12, 8835488
    SetPixelV m_hCkbSrcDC, 17, 12, 16711935
    SetPixelV m_hCkbSrcDC, 0, 13, 16711935
    SetPixelV m_hCkbSrcDC, 1, 13, 826944
    SetPixelV m_hCkbSrcDC, 2, 13, 826944
    SetPixelV m_hCkbSrcDC, 3, 13, 826944
    SetPixelV m_hCkbSrcDC, 4, 13, 826944
    SetPixelV m_hCkbSrcDC, 5, 13, 826944
    SetPixelV m_hCkbSrcDC, 6, 13, 826944
    SetPixelV m_hCkbSrcDC, 7, 13, 826944
    SetPixelV m_hCkbSrcDC, 8, 13, 16711935
    SetPixelV m_hCkbSrcDC, 9, 13, 16711935
    SetPixelV m_hCkbSrcDC, 10, 13, 8638110
    SetPixelV m_hCkbSrcDC, 11, 13, 8638110
    SetPixelV m_hCkbSrcDC, 12, 13, 8638110
    SetPixelV m_hCkbSrcDC, 13, 13, 8638110
    SetPixelV m_hCkbSrcDC, 14, 13, 8638110
    SetPixelV m_hCkbSrcDC, 15, 13, 8638110
    SetPixelV m_hCkbSrcDC, 16, 13, 8638110
    SetPixelV m_hCkbSrcDC, 17, 13, 16711935
    SetPixelV m_hCkbSrcDC, 0, 14, 16711935
    SetPixelV m_hCkbSrcDC, 1, 14, 497723
    SetPixelV m_hCkbSrcDC, 2, 14, 497723
    SetPixelV m_hCkbSrcDC, 3, 14, 497723
    SetPixelV m_hCkbSrcDC, 4, 14, 497723
    SetPixelV m_hCkbSrcDC, 5, 14, 497723
    SetPixelV m_hCkbSrcDC, 6, 14, 497723
    SetPixelV m_hCkbSrcDC, 7, 14, 497723
    SetPixelV m_hCkbSrcDC, 8, 14, 16711935
    SetPixelV m_hCkbSrcDC, 9, 14, 16711935
    SetPixelV m_hCkbSrcDC, 10, 14, 8506011
    SetPixelV m_hCkbSrcDC, 11, 14, 8506011
    SetPixelV m_hCkbSrcDC, 12, 14, 8506011
    SetPixelV m_hCkbSrcDC, 13, 14, 8506011
    SetPixelV m_hCkbSrcDC, 14, 14, 8506011
    SetPixelV m_hCkbSrcDC, 15, 14, 8506011
    SetPixelV m_hCkbSrcDC, 16, 14, 8506011
    SetPixelV m_hCkbSrcDC, 17, 14, 16711935
    SetPixelV m_hCkbSrcDC, 0, 15, 16711935
    SetPixelV m_hCkbSrcDC, 1, 15, 431673
    SetPixelV m_hCkbSrcDC, 2, 15, 431673
    SetPixelV m_hCkbSrcDC, 3, 15, 431673
    SetPixelV m_hCkbSrcDC, 4, 15, 431673
    SetPixelV m_hCkbSrcDC, 5, 15, 431673
    SetPixelV m_hCkbSrcDC, 6, 15, 431673
    SetPixelV m_hCkbSrcDC, 7, 15, 431673
    SetPixelV m_hCkbSrcDC, 8, 15, 16711935
    SetPixelV m_hCkbSrcDC, 9, 15, 16711935
    SetPixelV m_hCkbSrcDC, 10, 15, 8440218
    SetPixelV m_hCkbSrcDC, 11, 15, 8440218
    SetPixelV m_hCkbSrcDC, 12, 15, 8440218
    SetPixelV m_hCkbSrcDC, 13, 15, 8440218
    SetPixelV m_hCkbSrcDC, 14, 15, 8440218
    SetPixelV m_hCkbSrcDC, 15, 15, 8440218
    SetPixelV m_hCkbSrcDC, 16, 15, 8440218
    SetPixelV m_hCkbSrcDC, 17, 15, 16711935
    SetPixelV m_hCkbSrcDC, 0, 16, 16711935
    SetPixelV m_hCkbSrcDC, 1, 16, 16711935
    SetPixelV m_hCkbSrcDC, 2, 16, 16711935
    SetPixelV m_hCkbSrcDC, 3, 16, 16711935
    SetPixelV m_hCkbSrcDC, 4, 16, 16711935
    SetPixelV m_hCkbSrcDC, 5, 16, 16711935
    SetPixelV m_hCkbSrcDC, 6, 16, 16711935
    SetPixelV m_hCkbSrcDC, 7, 16, 16711935
    SetPixelV m_hCkbSrcDC, 8, 16, 16711935
    SetPixelV m_hCkbSrcDC, 9, 16, 16711935
    SetPixelV m_hCkbSrcDC, 10, 16, 16711935
    SetPixelV m_hCkbSrcDC, 11, 16, 16711935
    SetPixelV m_hCkbSrcDC, 12, 16, 16711935
    SetPixelV m_hCkbSrcDC, 13, 16, 16711935
    SetPixelV m_hCkbSrcDC, 14, 16, 16711935
    SetPixelV m_hCkbSrcDC, 15, 16, 16711935
    SetPixelV m_hCkbSrcDC, 16, 16, 16711935
    SetPixelV m_hCkbSrcDC, 17, 16, 16711935
    SetPixelV m_hCkbSrcDC, 0, 17, 16711935
    SetPixelV m_hCkbSrcDC, 1, 17, 16711935
    SetPixelV m_hCkbSrcDC, 2, 17, 16711935
    SetPixelV m_hCkbSrcDC, 3, 17, 16711935
    SetPixelV m_hCkbSrcDC, 4, 17, 16711935
    SetPixelV m_hCkbSrcDC, 5, 17, 16711935
    SetPixelV m_hCkbSrcDC, 6, 17, 16711935
    SetPixelV m_hCkbSrcDC, 7, 17, 16711935
    SetPixelV m_hCkbSrcDC, 8, 17, 16711935
    SetPixelV m_hCkbSrcDC, 9, 17, 16711935
    SetPixelV m_hCkbSrcDC, 10, 17, 16711935
    SetPixelV m_hCkbSrcDC, 11, 17, 16711935
    SetPixelV m_hCkbSrcDC, 12, 17, 16711935
    SetPixelV m_hCkbSrcDC, 13, 17, 16711935
    SetPixelV m_hCkbSrcDC, 14, 17, 16711935
    SetPixelV m_hCkbSrcDC, 15, 17, 16711935
    SetPixelV m_hCkbSrcDC, 16, 17, 16711935
    SetPixelV m_hCkbSrcDC, 17, 17, 16711935
    SetPixelV m_hCkbSrcDC, 0, 18, 16711935
    SetPixelV m_hCkbSrcDC, 1, 18, 16711935
    SetPixelV m_hCkbSrcDC, 2, 18, 16711935
    SetPixelV m_hCkbSrcDC, 3, 18, 6343815
    SetPixelV m_hCkbSrcDC, 4, 18, 2669148
    SetPixelV m_hCkbSrcDC, 5, 18, 6343815
    SetPixelV m_hCkbSrcDC, 6, 18, 16711935
    SetPixelV m_hCkbSrcDC, 7, 18, 16711935
    SetPixelV m_hCkbSrcDC, 8, 18, 16711935
    SetPixelV m_hCkbSrcDC, 9, 18, 16711935
    SetPixelV m_hCkbSrcDC, 10, 18, 16711935
    SetPixelV m_hCkbSrcDC, 11, 18, 16711935
    SetPixelV m_hCkbSrcDC, 12, 18, 11527619
    SetPixelV m_hCkbSrcDC, 13, 18, 9690285
    SetPixelV m_hCkbSrcDC, 14, 18, 11527619
    SetPixelV m_hCkbSrcDC, 15, 18, 16711935
    SetPixelV m_hCkbSrcDC, 16, 18, 16711935
    SetPixelV m_hCkbSrcDC, 17, 18, 16711935
    SetPixelV m_hCkbSrcDC, 0, 19, 16711935
    SetPixelV m_hCkbSrcDC, 1, 19, 16711935
    SetPixelV m_hCkbSrcDC, 2, 19, 6211715
    SetPixelV m_hCkbSrcDC, 3, 19, 1683791
    SetPixelV m_hCkbSrcDC, 4, 19, 1683791
    SetPixelV m_hCkbSrcDC, 5, 19, 1683791
    SetPixelV m_hCkbSrcDC, 6, 19, 6211715
    SetPixelV m_hCkbSrcDC, 7, 19, 16711935
    SetPixelV m_hCkbSrcDC, 8, 19, 16711935
    SetPixelV m_hCkbSrcDC, 9, 19, 16711935
    SetPixelV m_hCkbSrcDC, 10, 19, 16711935
    SetPixelV m_hCkbSrcDC, 11, 19, 11461569
    SetPixelV m_hCkbSrcDC, 12, 19, 9230503
    SetPixelV m_hCkbSrcDC, 13, 19, 9230503
    SetPixelV m_hCkbSrcDC, 14, 19, 9230503
    SetPixelV m_hCkbSrcDC, 15, 19, 11461569
    SetPixelV m_hCkbSrcDC, 16, 19, 16711935
    SetPixelV m_hCkbSrcDC, 17, 19, 16711935
    SetPixelV m_hCkbSrcDC, 0, 20, 16711935
    SetPixelV m_hCkbSrcDC, 1, 20, 16711935
    SetPixelV m_hCkbSrcDC, 2, 20, 1944656
    SetPixelV m_hCkbSrcDC, 3, 20, 1222727
    SetPixelV m_hCkbSrcDC, 4, 20, 1222727
    SetPixelV m_hCkbSrcDC, 5, 20, 1222727
    SetPixelV m_hCkbSrcDC, 6, 20, 1944656
    SetPixelV m_hCkbSrcDC, 7, 20, 16711935
    SetPixelV m_hCkbSrcDC, 8, 20, 16711935
    SetPixelV m_hCkbSrcDC, 9, 20, 16711935
    SetPixelV m_hCkbSrcDC, 10, 20, 16711935
    SetPixelV m_hCkbSrcDC, 11, 20, 9360807
    SetPixelV m_hCkbSrcDC, 12, 20, 8967075
    SetPixelV m_hCkbSrcDC, 13, 20, 8967075
    SetPixelV m_hCkbSrcDC, 14, 20, 8967075
    SetPixelV m_hCkbSrcDC, 15, 20, 9360807
    SetPixelV m_hCkbSrcDC, 16, 20, 16711935
    SetPixelV m_hCkbSrcDC, 17, 20, 16711935
    SetPixelV m_hCkbSrcDC, 0, 21, 16711935
    SetPixelV m_hCkbSrcDC, 1, 21, 16711935
    SetPixelV m_hCkbSrcDC, 2, 21, 5552760
    SetPixelV m_hCkbSrcDC, 3, 21, 761151
    SetPixelV m_hCkbSrcDC, 4, 21, 761151
    SetPixelV m_hCkbSrcDC, 5, 21, 761151
    SetPixelV m_hCkbSrcDC, 6, 21, 5552760
    SetPixelV m_hCkbSrcDC, 7, 21, 16711935
    SetPixelV m_hCkbSrcDC, 8, 21, 16711935
    SetPixelV m_hCkbSrcDC, 9, 21, 16711935
    SetPixelV m_hCkbSrcDC, 10, 21, 16711935
    SetPixelV m_hCkbSrcDC, 11, 21, 11132091
    SetPixelV m_hCkbSrcDC, 12, 21, 8769183
    SetPixelV m_hCkbSrcDC, 13, 21, 8769183
    SetPixelV m_hCkbSrcDC, 14, 21, 8769183
    SetPixelV m_hCkbSrcDC, 15, 21, 11132091
    SetPixelV m_hCkbSrcDC, 16, 21, 16711935
    SetPixelV m_hCkbSrcDC, 17, 21, 16711935
    SetPixelV m_hCkbSrcDC, 0, 22, 16711935
    SetPixelV m_hCkbSrcDC, 1, 22, 16711935
    SetPixelV m_hCkbSrcDC, 2, 22, 16711935
    SetPixelV m_hCkbSrcDC, 3, 22, 5223539
    SetPixelV m_hCkbSrcDC, 4, 22, 1087808
    SetPixelV m_hCkbSrcDC, 5, 22, 5223539
    SetPixelV m_hCkbSrcDC, 6, 22, 16711935
    SetPixelV m_hCkbSrcDC, 7, 22, 16711935
    SetPixelV m_hCkbSrcDC, 8, 22, 16711935
    SetPixelV m_hCkbSrcDC, 9, 22, 16711935
    SetPixelV m_hCkbSrcDC, 10, 22, 16711935
    SetPixelV m_hCkbSrcDC, 11, 22, 16711935
    SetPixelV m_hCkbSrcDC, 12, 22, 11000249
    SetPixelV m_hCkbSrcDC, 13, 22, 8899743
    SetPixelV m_hCkbSrcDC, 14, 22, 11000249
    SetPixelV m_hCkbSrcDC, 15, 22, 16711935
    SetPixelV m_hCkbSrcDC, 16, 22, 16711935
    SetPixelV m_hCkbSrcDC, 17, 22, 16711935
    SetPixelV m_hCkbSrcDC, 0, 23, 16711935
    SetPixelV m_hCkbSrcDC, 1, 23, 16711935
    SetPixelV m_hCkbSrcDC, 2, 23, 16711935
    SetPixelV m_hCkbSrcDC, 3, 23, 16711935
    SetPixelV m_hCkbSrcDC, 4, 23, 16711935
    SetPixelV m_hCkbSrcDC, 5, 23, 16711935
    SetPixelV m_hCkbSrcDC, 6, 23, 16711935
    SetPixelV m_hCkbSrcDC, 7, 23, 16711935
    SetPixelV m_hCkbSrcDC, 8, 23, 16711935
    SetPixelV m_hCkbSrcDC, 9, 23, 16711935
    SetPixelV m_hCkbSrcDC, 10, 23, 16711935
    SetPixelV m_hCkbSrcDC, 11, 23, 16711935
    SetPixelV m_hCkbSrcDC, 12, 23, 16711935
    SetPixelV m_hCkbSrcDC, 13, 23, 16711935
    SetPixelV m_hCkbSrcDC, 14, 23, 16711935
    SetPixelV m_hCkbSrcDC, 15, 23, 16711935
    SetPixelV m_hCkbSrcDC, 16, 23, 16711935
    SetPixelV m_hCkbSrcDC, 17, 23, 16711935
    
    SetPixelV m_hOpbSrcDC, 0, 0, 16711935
    SetPixelV m_hOpbSrcDC, 1, 0, 16711935
    SetPixelV m_hOpbSrcDC, 2, 0, 16711935
    SetPixelV m_hOpbSrcDC, 3, 0, 15191996
    SetPixelV m_hOpbSrcDC, 4, 0, 14267285
    SetPixelV m_hOpbSrcDC, 5, 0, 13871238
    SetPixelV m_hOpbSrcDC, 6, 0, 13805187
    SetPixelV m_hOpbSrcDC, 7, 0, 13871238
    SetPixelV m_hOpbSrcDC, 8, 0, 14267285
    SetPixelV m_hOpbSrcDC, 9, 0, 15191996
    SetPixelV m_hOpbSrcDC, 10, 0, 16711935
    SetPixelV m_hOpbSrcDC, 11, 0, 16711935
    SetPixelV m_hOpbSrcDC, 12, 0, 16711935
    SetPixelV m_hOpbSrcDC, 0, 1, 16711935
    SetPixelV m_hOpbSrcDC, 1, 1, 16711935
    SetPixelV m_hOpbSrcDC, 2, 1, 14399645
    SetPixelV m_hOpbSrcDC, 3, 1, 14664361
    SetPixelV m_hOpbSrcDC, 4, 1, 15523793
    SetPixelV m_hOpbSrcDC, 5, 1, 15787997
    SetPixelV m_hOpbSrcDC, 6, 1, 15854047
    SetPixelV m_hOpbSrcDC, 7, 1, 15787997
    SetPixelV m_hOpbSrcDC, 8, 1, 15523793
    SetPixelV m_hOpbSrcDC, 9, 1, 14664361
    SetPixelV m_hOpbSrcDC, 10, 1, 14399645
    SetPixelV m_hOpbSrcDC, 11, 1, 16711935
    SetPixelV m_hOpbSrcDC, 12, 1, 16711935
    SetPixelV m_hOpbSrcDC, 0, 2, 16711935
    SetPixelV m_hOpbSrcDC, 1, 2, 14399645
    SetPixelV m_hOpbSrcDC, 2, 2, 15060665
    SetPixelV m_hOpbSrcDC, 3, 2, 15985891
    SetPixelV m_hOpbSrcDC, 4, 2, 16051685
    SetPixelV m_hOpbSrcDC, 5, 2, 16051943
    SetPixelV m_hOpbSrcDC, 6, 2, 16051943
    SetPixelV m_hOpbSrcDC, 7, 2, 16051943
    SetPixelV m_hOpbSrcDC, 8, 2, 16051685
    SetPixelV m_hOpbSrcDC, 9, 2, 15985891
    SetPixelV m_hOpbSrcDC, 10, 2, 15060665
    SetPixelV m_hOpbSrcDC, 11, 2, 14399645
    SetPixelV m_hOpbSrcDC, 12, 2, 16711935
    SetPixelV m_hOpbSrcDC, 0, 3, 15191996
    SetPixelV m_hOpbSrcDC, 1, 3, 14664361
    SetPixelV m_hOpbSrcDC, 2, 3, 16051685
    SetPixelV m_hOpbSrcDC, 3, 3, 16249581
    SetPixelV m_hOpbSrcDC, 4, 3, 16447475
    SetPixelV m_hOpbSrcDC, 5, 3, 16513527
    SetPixelV m_hOpbSrcDC, 6, 3, 16513527
    SetPixelV m_hOpbSrcDC, 7, 3, 16513527
    SetPixelV m_hOpbSrcDC, 8, 3, 16447475
    SetPixelV m_hOpbSrcDC, 9, 3, 16249581
    SetPixelV m_hOpbSrcDC, 10, 3, 16051685
    SetPixelV m_hOpbSrcDC, 11, 3, 14664361
    SetPixelV m_hOpbSrcDC, 12, 3, 15191996
    SetPixelV m_hOpbSrcDC, 0, 4, 14267285
    SetPixelV m_hOpbSrcDC, 1, 4, 15721174
    SetPixelV m_hOpbSrcDC, 2, 4, 16315631
    SetPixelV m_hOpbSrcDC, 3, 4, 16579577
    SetPixelV m_hOpbSrcDC, 4, 4, 16777215
    SetPixelV m_hOpbSrcDC, 5, 4, 16777215
    SetPixelV m_hOpbSrcDC, 6, 4, 16777215
    SetPixelV m_hOpbSrcDC, 7, 4, 16777215
    SetPixelV m_hOpbSrcDC, 8, 4, 16777215
    SetPixelV m_hOpbSrcDC, 9, 4, 16579577
    SetPixelV m_hOpbSrcDC, 10, 4, 16315631
    SetPixelV m_hOpbSrcDC, 11, 4, 15721174
    SetPixelV m_hOpbSrcDC, 12, 4, 14267285
    SetPixelV m_hOpbSrcDC, 0, 5, 13871238
    SetPixelV m_hOpbSrcDC, 1, 5, 16183531
    SetPixelV m_hOpbSrcDC, 2, 5, 16579577
    SetPixelV m_hOpbSrcDC, 3, 5, 16776958
    SetPixelV m_hOpbSrcDC, 4, 5, 16777215
    SetPixelV m_hOpbSrcDC, 5, 5, 16777215
    SetPixelV m_hOpbSrcDC, 6, 5, 16777215
    SetPixelV m_hOpbSrcDC, 7, 5, 16777215
    SetPixelV m_hOpbSrcDC, 8, 5, 16777215
    SetPixelV m_hOpbSrcDC, 9, 5, 16776958
    SetPixelV m_hOpbSrcDC, 10, 5, 16579577
    SetPixelV m_hOpbSrcDC, 11, 5, 16183531
    SetPixelV m_hOpbSrcDC, 12, 5, 13871238
    SetPixelV m_hOpbSrcDC, 0, 6, 13805187
    SetPixelV m_hOpbSrcDC, 1, 6, 16447475
    SetPixelV m_hOpbSrcDC, 2, 6, 16711421
    SetPixelV m_hOpbSrcDC, 3, 6, 16777215
    SetPixelV m_hOpbSrcDC, 4, 6, 16777215
    SetPixelV m_hOpbSrcDC, 5, 6, 16777215
    SetPixelV m_hOpbSrcDC, 6, 6, 16777215
    SetPixelV m_hOpbSrcDC, 7, 6, 16777215
    SetPixelV m_hOpbSrcDC, 8, 6, 16777215
    SetPixelV m_hOpbSrcDC, 9, 6, 16777215
    SetPixelV m_hOpbSrcDC, 10, 6, 16711421
    SetPixelV m_hOpbSrcDC, 11, 6, 16447475
    SetPixelV m_hOpbSrcDC, 12, 6, 13805187
    SetPixelV m_hOpbSrcDC, 0, 7, 13871238
    SetPixelV m_hOpbSrcDC, 1, 7, 16447476
    SetPixelV m_hOpbSrcDC, 2, 7, 16777215
    SetPixelV m_hOpbSrcDC, 3, 7, 16777215
    SetPixelV m_hOpbSrcDC, 4, 7, 16777215
    SetPixelV m_hOpbSrcDC, 5, 7, 16777215
    SetPixelV m_hOpbSrcDC, 6, 7, 16777215
    SetPixelV m_hOpbSrcDC, 7, 7, 16777215
    SetPixelV m_hOpbSrcDC, 8, 7, 16777215
    SetPixelV m_hOpbSrcDC, 9, 7, 16777215
    SetPixelV m_hOpbSrcDC, 10, 7, 16777215
    SetPixelV m_hOpbSrcDC, 11, 7, 16447476
    SetPixelV m_hOpbSrcDC, 12, 7, 13871238
    SetPixelV m_hOpbSrcDC, 0, 8, 14267285
    SetPixelV m_hOpbSrcDC, 1, 8, 16117222
    SetPixelV m_hOpbSrcDC, 2, 8, 16777215
    SetPixelV m_hOpbSrcDC, 3, 8, 16777215
    SetPixelV m_hOpbSrcDC, 4, 8, 16777215
    SetPixelV m_hOpbSrcDC, 5, 8, 16777215
    SetPixelV m_hOpbSrcDC, 6, 8, 16777215
    SetPixelV m_hOpbSrcDC, 7, 8, 16777215
    SetPixelV m_hOpbSrcDC, 8, 8, 16777215
    SetPixelV m_hOpbSrcDC, 9, 8, 16777215
    SetPixelV m_hOpbSrcDC, 10, 8, 16777215
    SetPixelV m_hOpbSrcDC, 11, 8, 16117222
    SetPixelV m_hOpbSrcDC, 12, 8, 14267285
    SetPixelV m_hOpbSrcDC, 0, 9, 15191996
    SetPixelV m_hOpbSrcDC, 1, 9, 14928051
    SetPixelV m_hOpbSrcDC, 2, 9, 16777215
    SetPixelV m_hOpbSrcDC, 3, 9, 16777215
    SetPixelV m_hOpbSrcDC, 4, 9, 16777215
    SetPixelV m_hOpbSrcDC, 5, 9, 16777215
    SetPixelV m_hOpbSrcDC, 6, 9, 16777215
    SetPixelV m_hOpbSrcDC, 7, 9, 16777215
    SetPixelV m_hOpbSrcDC, 8, 9, 16777215
    SetPixelV m_hOpbSrcDC, 9, 9, 16777215
    SetPixelV m_hOpbSrcDC, 10, 9, 16777215
    SetPixelV m_hOpbSrcDC, 11, 9, 14928051
    SetPixelV m_hOpbSrcDC, 12, 9, 15191996
    SetPixelV m_hOpbSrcDC, 0, 10, 16711935
    SetPixelV m_hOpbSrcDC, 1, 10, 14399645
    SetPixelV m_hOpbSrcDC, 2, 10, 15522250
    SetPixelV m_hOpbSrcDC, 3, 10, 16777215
    SetPixelV m_hOpbSrcDC, 4, 10, 16777215
    SetPixelV m_hOpbSrcDC, 5, 10, 16777215
    SetPixelV m_hOpbSrcDC, 6, 10, 16777215
    SetPixelV m_hOpbSrcDC, 7, 10, 16777215
    SetPixelV m_hOpbSrcDC, 8, 10, 16777215
    SetPixelV m_hOpbSrcDC, 9, 10, 16777215
    SetPixelV m_hOpbSrcDC, 10, 10, 15522250
    SetPixelV m_hOpbSrcDC, 11, 10, 14399645
    SetPixelV m_hOpbSrcDC, 12, 10, 16711935
    SetPixelV m_hOpbSrcDC, 0, 11, 16711935
    SetPixelV m_hOpbSrcDC, 1, 11, 16711935
    SetPixelV m_hOpbSrcDC, 2, 11, 14399645
    SetPixelV m_hOpbSrcDC, 3, 11, 15059638
    SetPixelV m_hOpbSrcDC, 4, 11, 16315117
    SetPixelV m_hOpbSrcDC, 5, 11, 16711164
    SetPixelV m_hOpbSrcDC, 6, 11, 16777215
    SetPixelV m_hOpbSrcDC, 7, 11, 16711164
    SetPixelV m_hOpbSrcDC, 8, 11, 16315117
    SetPixelV m_hOpbSrcDC, 9, 11, 15059638
    SetPixelV m_hOpbSrcDC, 10, 11, 14399645
    SetPixelV m_hOpbSrcDC, 11, 11, 16711935
    SetPixelV m_hOpbSrcDC, 12, 11, 16711935
    SetPixelV m_hOpbSrcDC, 0, 12, 16711935
    SetPixelV m_hOpbSrcDC, 1, 12, 16711935
    SetPixelV m_hOpbSrcDC, 2, 12, 16711935
    SetPixelV m_hOpbSrcDC, 3, 12, 15191996
    SetPixelV m_hOpbSrcDC, 4, 12, 14267285
    SetPixelV m_hOpbSrcDC, 5, 12, 13871238
    SetPixelV m_hOpbSrcDC, 6, 12, 13805187
    SetPixelV m_hOpbSrcDC, 7, 12, 13871238
    SetPixelV m_hOpbSrcDC, 8, 12, 14267285
    SetPixelV m_hOpbSrcDC, 9, 12, 15191996
    SetPixelV m_hOpbSrcDC, 10, 12, 16711935
    SetPixelV m_hOpbSrcDC, 11, 12, 16711935
    SetPixelV m_hOpbSrcDC, 12, 12, 16711935
    SetPixelV m_hOpbSrcDC, 0, 13, 16711935
    SetPixelV m_hOpbSrcDC, 1, 13, 16711935
    SetPixelV m_hOpbSrcDC, 2, 13, 16711935
    SetPixelV m_hOpbSrcDC, 3, 13, 15191996
    SetPixelV m_hOpbSrcDC, 4, 13, 14267285
    SetPixelV m_hOpbSrcDC, 5, 13, 13871238
    SetPixelV m_hOpbSrcDC, 6, 13, 13805187
    SetPixelV m_hOpbSrcDC, 7, 13, 13871238
    SetPixelV m_hOpbSrcDC, 8, 13, 14267285
    SetPixelV m_hOpbSrcDC, 9, 13, 15191996
    SetPixelV m_hOpbSrcDC, 10, 13, 16711935
    SetPixelV m_hOpbSrcDC, 11, 13, 16711935
    SetPixelV m_hOpbSrcDC, 12, 13, 16711935
    SetPixelV m_hOpbSrcDC, 0, 14, 16711935
    SetPixelV m_hOpbSrcDC, 1, 14, 16711935
    SetPixelV m_hOpbSrcDC, 2, 14, 14399645
    SetPixelV m_hOpbSrcDC, 3, 14, 14664361
    SetPixelV m_hOpbSrcDC, 4, 14, 16313027
    SetPixelV m_hOpbSrcDC, 5, 14, 16313027
    SetPixelV m_hOpbSrcDC, 6, 14, 16313027
    SetPixelV m_hOpbSrcDC, 7, 14, 16313284
    SetPixelV m_hOpbSrcDC, 8, 14, 16313285
    SetPixelV m_hOpbSrcDC, 9, 14, 14664361
    SetPixelV m_hOpbSrcDC, 10, 14, 14399645
    SetPixelV m_hOpbSrcDC, 11, 14, 16711935
    SetPixelV m_hOpbSrcDC, 12, 14, 16711935
    SetPixelV m_hOpbSrcDC, 0, 15, 16711935
    SetPixelV m_hOpbSrcDC, 1, 15, 14399645
    SetPixelV m_hOpbSrcDC, 2, 15, 15060665
    SetPixelV m_hOpbSrcDC, 3, 15, 16313027
    SetPixelV m_hOpbSrcDC, 4, 15, 16313027
    SetPixelV m_hOpbSrcDC, 5, 15, 16313027
    SetPixelV m_hOpbSrcDC, 6, 15, 16313284
    SetPixelV m_hOpbSrcDC, 7, 15, 16378821
    SetPixelV m_hOpbSrcDC, 8, 15, 16379080
    SetPixelV m_hOpbSrcDC, 9, 15, 16379338
    SetPixelV m_hOpbSrcDC, 10, 15, 15060665
    SetPixelV m_hOpbSrcDC, 11, 15, 14399645
    SetPixelV m_hOpbSrcDC, 12, 15, 16711935
    SetPixelV m_hOpbSrcDC, 0, 16, 15191996
    SetPixelV m_hOpbSrcDC, 1, 16, 14664361
    SetPixelV m_hOpbSrcDC, 2, 16, 16313027
    SetPixelV m_hOpbSrcDC, 3, 16, 16313027
    SetPixelV m_hOpbSrcDC, 4, 16, 16313027
    SetPixelV m_hOpbSrcDC, 5, 16, 16313028
    SetPixelV m_hOpbSrcDC, 6, 16, 16313286
    SetPixelV m_hOpbSrcDC, 7, 16, 16379080
    SetPixelV m_hOpbSrcDC, 8, 16, 16379339
    SetPixelV m_hOpbSrcDC, 9, 16, 16445133
    SetPixelV m_hOpbSrcDC, 10, 16, 16445391
    SetPixelV m_hOpbSrcDC, 11, 16, 14664361
    SetPixelV m_hOpbSrcDC, 12, 16, 15191996
    SetPixelV m_hOpbSrcDC, 0, 17, 14267285
    SetPixelV m_hOpbSrcDC, 1, 17, 16313027
    SetPixelV m_hOpbSrcDC, 2, 17, 16313027
    SetPixelV m_hOpbSrcDC, 3, 17, 16313028
    SetPixelV m_hOpbSrcDC, 4, 17, 16313028
    SetPixelV m_hOpbSrcDC, 5, 17, 16379078
    SetPixelV m_hOpbSrcDC, 6, 17, 16379081
    SetPixelV m_hOpbSrcDC, 7, 17, 16379339
    SetPixelV m_hOpbSrcDC, 8, 17, 16445133
    SetPixelV m_hOpbSrcDC, 9, 17, 16445392
    SetPixelV m_hOpbSrcDC, 10, 17, 16511187
    SetPixelV m_hOpbSrcDC, 11, 17, 16445910
    SetPixelV m_hOpbSrcDC, 12, 17, 14267285
    SetPixelV m_hOpbSrcDC, 0, 18, 13871238
    SetPixelV m_hOpbSrcDC, 1, 18, 16313027
    SetPixelV m_hOpbSrcDC, 2, 18, 16313027
    SetPixelV m_hOpbSrcDC, 3, 18, 16313285
    SetPixelV m_hOpbSrcDC, 4, 18, 16313543
    SetPixelV m_hOpbSrcDC, 5, 18, 16379337
    SetPixelV m_hOpbSrcDC, 6, 18, 16379339
    SetPixelV m_hOpbSrcDC, 7, 18, 16445134
    SetPixelV m_hOpbSrcDC, 8, 18, 16445648
    SetPixelV m_hOpbSrcDC, 9, 18, 16511187
    SetPixelV m_hOpbSrcDC, 10, 18, 16511446
    SetPixelV m_hOpbSrcDC, 11, 18, 16511705
    SetPixelV m_hOpbSrcDC, 12, 18, 13871238
    SetPixelV m_hOpbSrcDC, 0, 19, 13805187
    SetPixelV m_hOpbSrcDC, 1, 19, 16313028
    SetPixelV m_hOpbSrcDC, 2, 19, 16313285
    SetPixelV m_hOpbSrcDC, 3, 19, 16379079
    SetPixelV m_hOpbSrcDC, 4, 19, 16379337
    SetPixelV m_hOpbSrcDC, 5, 19, 16445132
    SetPixelV m_hOpbSrcDC, 6, 19, 16445135
    SetPixelV m_hOpbSrcDC, 7, 19, 16445648
    SetPixelV m_hOpbSrcDC, 8, 19, 16445908
    SetPixelV m_hOpbSrcDC, 9, 19, 16511446
    SetPixelV m_hOpbSrcDC, 10, 19, 16511705
    SetPixelV m_hOpbSrcDC, 11, 19, 16577755
    SetPixelV m_hOpbSrcDC, 12, 19, 13805187
    SetPixelV m_hOpbSrcDC, 0, 20, 13871238
    SetPixelV m_hOpbSrcDC, 1, 20, 16313285
    SetPixelV m_hOpbSrcDC, 2, 20, 16313287
    SetPixelV m_hOpbSrcDC, 3, 20, 16379081
    SetPixelV m_hOpbSrcDC, 4, 20, 16445132
    SetPixelV m_hOpbSrcDC, 5, 20, 16445390
    SetPixelV m_hOpbSrcDC, 6, 20, 16511185
    SetPixelV m_hOpbSrcDC, 7, 20, 16511188
    SetPixelV m_hOpbSrcDC, 8, 20, 16576983
    SetPixelV m_hOpbSrcDC, 9, 20, 16577497
    SetPixelV m_hOpbSrcDC, 10, 20, 16577500
    SetPixelV m_hOpbSrcDC, 11, 20, 16577758
    SetPixelV m_hOpbSrcDC, 12, 20, 13871238
    SetPixelV m_hOpbSrcDC, 0, 21, 14267285
    SetPixelV m_hOpbSrcDC, 1, 21, 16379079
    SetPixelV m_hOpbSrcDC, 2, 21, 16379338
    SetPixelV m_hOpbSrcDC, 3, 21, 16379596
    SetPixelV m_hOpbSrcDC, 4, 21, 16445391
    SetPixelV m_hOpbSrcDC, 5, 21, 16445394
    SetPixelV m_hOpbSrcDC, 6, 21, 16511444
    SetPixelV m_hOpbSrcDC, 7, 21, 16511704
    SetPixelV m_hOpbSrcDC, 8, 21, 16511962
    SetPixelV m_hOpbSrcDC, 9, 21, 16577756
    SetPixelV m_hOpbSrcDC, 10, 21, 16577758
    SetPixelV m_hOpbSrcDC, 11, 21, 16643552
    SetPixelV m_hOpbSrcDC, 12, 21, 14267285
    SetPixelV m_hOpbSrcDC, 0, 22, 15191996
    SetPixelV m_hOpbSrcDC, 1, 22, 14928051
    SetPixelV m_hOpbSrcDC, 2, 22, 16445133
    SetPixelV m_hOpbSrcDC, 3, 22, 16445392
    SetPixelV m_hOpbSrcDC, 4, 22, 16511186
    SetPixelV m_hOpbSrcDC, 5, 22, 16511445
    SetPixelV m_hOpbSrcDC, 6, 22, 16511704
    SetPixelV m_hOpbSrcDC, 7, 22, 16577499
    SetPixelV m_hOpbSrcDC, 8, 22, 16577756
    SetPixelV m_hOpbSrcDC, 9, 22, 16643551
    SetPixelV m_hOpbSrcDC, 10, 22, 16578016
    SetPixelV m_hOpbSrcDC, 11, 22, 14928051
    SetPixelV m_hOpbSrcDC, 12, 22, 15191996
    SetPixelV m_hOpbSrcDC, 0, 23, 16711935
    SetPixelV m_hOpbSrcDC, 1, 23, 14399645
    SetPixelV m_hOpbSrcDC, 2, 23, 15522250
    SetPixelV m_hOpbSrcDC, 3, 23, 16511187
    SetPixelV m_hOpbSrcDC, 4, 23, 16511446
    SetPixelV m_hOpbSrcDC, 5, 23, 16511704
    SetPixelV m_hOpbSrcDC, 6, 23, 16577755
    SetPixelV m_hOpbSrcDC, 7, 23, 16643293
    SetPixelV m_hOpbSrcDC, 8, 23, 16643551
    SetPixelV m_hOpbSrcDC, 9, 23, 16643553
    SetPixelV m_hOpbSrcDC, 10, 23, 15522250
    SetPixelV m_hOpbSrcDC, 11, 23, 14399645
    SetPixelV m_hOpbSrcDC, 12, 23, 16711935
    SetPixelV m_hOpbSrcDC, 0, 24, 16711935
    SetPixelV m_hOpbSrcDC, 1, 24, 16711935
    SetPixelV m_hOpbSrcDC, 2, 24, 14399645
    SetPixelV m_hOpbSrcDC, 3, 24, 15059638
    SetPixelV m_hOpbSrcDC, 4, 24, 16577497
    SetPixelV m_hOpbSrcDC, 5, 24, 16577499
    SetPixelV m_hOpbSrcDC, 6, 24, 16577757
    SetPixelV m_hOpbSrcDC, 7, 24, 16578015
    SetPixelV m_hOpbSrcDC, 8, 24, 16578017
    SetPixelV m_hOpbSrcDC, 9, 24, 15059638
    SetPixelV m_hOpbSrcDC, 10, 24, 14399645
    SetPixelV m_hOpbSrcDC, 11, 24, 16711935
    SetPixelV m_hOpbSrcDC, 12, 24, 16711935
    SetPixelV m_hOpbSrcDC, 0, 25, 16711935
    SetPixelV m_hOpbSrcDC, 1, 25, 16711935
    SetPixelV m_hOpbSrcDC, 2, 25, 16711935
    SetPixelV m_hOpbSrcDC, 3, 25, 15191996
    SetPixelV m_hOpbSrcDC, 4, 25, 14267285
    SetPixelV m_hOpbSrcDC, 5, 25, 13871238
    SetPixelV m_hOpbSrcDC, 6, 25, 13805187
    SetPixelV m_hOpbSrcDC, 7, 25, 13871238
    SetPixelV m_hOpbSrcDC, 8, 25, 14267285
    SetPixelV m_hOpbSrcDC, 9, 25, 15191996
    SetPixelV m_hOpbSrcDC, 10, 25, 16711935
    SetPixelV m_hOpbSrcDC, 11, 25, 16711935
    SetPixelV m_hOpbSrcDC, 12, 25, 16711935
    SetPixelV m_hOpbSrcDC, 0, 26, 16711935
    SetPixelV m_hOpbSrcDC, 1, 26, 16711935
    SetPixelV m_hOpbSrcDC, 2, 26, 16711935
    SetPixelV m_hOpbSrcDC, 3, 26, 14531748
    SetPixelV m_hOpbSrcDC, 4, 26, 13277558
    SetPixelV m_hOpbSrcDC, 5, 26, 12749671
    SetPixelV m_hOpbSrcDC, 6, 26, 12683620
    SetPixelV m_hOpbSrcDC, 7, 26, 12749671
    SetPixelV m_hOpbSrcDC, 8, 26, 13277558
    SetPixelV m_hOpbSrcDC, 9, 26, 14531748
    SetPixelV m_hOpbSrcDC, 10, 26, 16711935
    SetPixelV m_hOpbSrcDC, 11, 26, 16711935
    SetPixelV m_hOpbSrcDC, 12, 26, 16711935
    SetPixelV m_hOpbSrcDC, 0, 27, 16711935
    SetPixelV m_hOpbSrcDC, 1, 27, 16711935
    SetPixelV m_hOpbSrcDC, 2, 27, 13475711
    SetPixelV m_hOpbSrcDC, 3, 27, 13872268
    SetPixelV m_hOpbSrcDC, 4, 27, 16313542
    SetPixelV m_hOpbSrcDC, 5, 27, 16379338
    SetPixelV m_hOpbSrcDC, 6, 27, 16445389
    SetPixelV m_hOpbSrcDC, 7, 27, 16445393
    SetPixelV m_hOpbSrcDC, 8, 27, 16642516
    SetPixelV m_hOpbSrcDC, 9, 27, 14201753
    SetPixelV m_hOpbSrcDC, 10, 27, 13475711
    SetPixelV m_hOpbSrcDC, 11, 27, 16711935
    SetPixelV m_hOpbSrcDC, 12, 27, 16711935
    SetPixelV m_hOpbSrcDC, 0, 28, 16711935
    SetPixelV m_hOpbSrcDC, 1, 28, 13475711
    SetPixelV m_hOpbSrcDC, 2, 28, 14334880
    SetPixelV m_hOpbSrcDC, 3, 28, 16312508
    SetPixelV m_hOpbSrcDC, 4, 28, 16378562
    SetPixelV m_hOpbSrcDC, 5, 28, 16379078
    SetPixelV m_hOpbSrcDC, 6, 28, 16379338
    SetPixelV m_hOpbSrcDC, 7, 28, 16445134
    SetPixelV m_hOpbSrcDC, 8, 28, 16445393
    SetPixelV m_hOpbSrcDC, 9, 28, 16445908
    SetPixelV m_hOpbSrcDC, 10, 28, 15059637
    SetPixelV m_hOpbSrcDC, 11, 28, 13475711
    SetPixelV m_hOpbSrcDC, 12, 28, 16711935
    SetPixelV m_hOpbSrcDC, 0, 29, 14531748
    SetPixelV m_hOpbSrcDC, 1, 29, 13872268
    SetPixelV m_hOpbSrcDC, 2, 29, 16246453
    SetPixelV m_hOpbSrcDC, 3, 29, 16312250
    SetPixelV m_hOpbSrcDC, 4, 29, 16312510
    SetPixelV m_hOpbSrcDC, 5, 29, 16378562
    SetPixelV m_hOpbSrcDC, 6, 29, 16379078
    SetPixelV m_hOpbSrcDC, 7, 29, 16445130
    SetPixelV m_hOpbSrcDC, 8, 29, 16445390
    SetPixelV m_hOpbSrcDC, 9, 29, 16642515
    SetPixelV m_hOpbSrcDC, 10, 29, 16642517
    SetPixelV m_hOpbSrcDC, 11, 29, 14333340
    SetPixelV m_hOpbSrcDC, 12, 29, 14531748
    SetPixelV m_hOpbSrcDC, 0, 30, 13277558
    SetPixelV m_hOpbSrcDC, 1, 30, 16114352
    SetPixelV m_hOpbSrcDC, 2, 30, 16246195
    SetPixelV m_hOpbSrcDC, 3, 30, 16246456
    SetPixelV m_hOpbSrcDC, 4, 30, 16312250
    SetPixelV m_hOpbSrcDC, 5, 30, 16313022
    SetPixelV m_hOpbSrcDC, 6, 30, 16313539
    SetPixelV m_hOpbSrcDC, 7, 30, 16444615
    SetPixelV m_hOpbSrcDC, 8, 30, 16379596
    SetPixelV m_hOpbSrcDC, 9, 30, 16445390
    SetPixelV m_hOpbSrcDC, 10, 30, 16642515
    SetPixelV m_hOpbSrcDC, 11, 30, 16445909
    SetPixelV m_hOpbSrcDC, 12, 30, 13277558
    SetPixelV m_hOpbSrcDC, 0, 31, 12749671
    SetPixelV m_hOpbSrcDC, 1, 31, 16114350
    SetPixelV m_hOpbSrcDC, 2, 31, 16245424
    SetPixelV m_hOpbSrcDC, 3, 31, 16246195
    SetPixelV m_hOpbSrcDC, 4, 31, 16246456
    SetPixelV m_hOpbSrcDC, 5, 31, 16312251
    SetPixelV m_hOpbSrcDC, 6, 31, 16313022
    SetPixelV m_hOpbSrcDC, 7, 31, 16378563
    SetPixelV m_hOpbSrcDC, 8, 31, 16379336
    SetPixelV m_hOpbSrcDC, 9, 31, 16445133
    SetPixelV m_hOpbSrcDC, 10, 31, 16641999
    SetPixelV m_hOpbSrcDC, 11, 31, 16445907
    SetPixelV m_hOpbSrcDC, 12, 31, 12749671
    SetPixelV m_hOpbSrcDC, 0, 32, 12683620
    SetPixelV m_hOpbSrcDC, 1, 32, 16114093
    SetPixelV m_hOpbSrcDC, 2, 32, 16114350
    SetPixelV m_hOpbSrcDC, 3, 32, 16114353
    SetPixelV m_hOpbSrcDC, 4, 32, 16246196
    SetPixelV m_hOpbSrcDC, 5, 32, 16246456
    SetPixelV m_hOpbSrcDC, 6, 32, 16312252
    SetPixelV m_hOpbSrcDC, 7, 32, 16378559
    SetPixelV m_hOpbSrcDC, 8, 32, 16379075
    SetPixelV m_hOpbSrcDC, 9, 32, 16379336
    SetPixelV m_hOpbSrcDC, 10, 32, 16445389
    SetPixelV m_hOpbSrcDC, 11, 32, 16445391
    SetPixelV m_hOpbSrcDC, 12, 32, 12683620
    SetPixelV m_hOpbSrcDC, 0, 33, 12749671
    SetPixelV m_hOpbSrcDC, 1, 33, 16114093
    SetPixelV m_hOpbSrcDC, 2, 33, 16114093
    SetPixelV m_hOpbSrcDC, 3, 33, 16114094
    SetPixelV m_hOpbSrcDC, 4, 33, 16246193
    SetPixelV m_hOpbSrcDC, 5, 33, 16246452
    SetPixelV m_hOpbSrcDC, 6, 33, 16312249
    SetPixelV m_hOpbSrcDC, 7, 33, 16312507
    SetPixelV m_hOpbSrcDC, 8, 33, 16312513
    SetPixelV m_hOpbSrcDC, 9, 33, 16379077
    SetPixelV m_hOpbSrcDC, 10, 33, 16379336
    SetPixelV m_hOpbSrcDC, 11, 33, 16445133
    SetPixelV m_hOpbSrcDC, 12, 33, 12749671
    SetPixelV m_hOpbSrcDC, 0, 34, 13277558
    SetPixelV m_hOpbSrcDC, 1, 34, 16114093
    SetPixelV m_hOpbSrcDC, 2, 34, 16114093
    SetPixelV m_hOpbSrcDC, 3, 34, 16114093
    SetPixelV m_hOpbSrcDC, 4, 34, 16114094
    SetPixelV m_hOpbSrcDC, 5, 34, 16115122
    SetPixelV m_hOpbSrcDC, 6, 34, 16246452
    SetPixelV m_hOpbSrcDC, 7, 34, 16312249
    SetPixelV m_hOpbSrcDC, 8, 34, 16312508
    SetPixelV m_hOpbSrcDC, 9, 34, 16378561
    SetPixelV m_hOpbSrcDC, 10, 34, 16379078
    SetPixelV m_hOpbSrcDC, 11, 34, 16445130
    SetPixelV m_hOpbSrcDC, 12, 34, 13277558
    SetPixelV m_hOpbSrcDC, 0, 35, 14531748
    SetPixelV m_hOpbSrcDC, 1, 35, 13872268
    SetPixelV m_hOpbSrcDC, 2, 35, 16114093
    SetPixelV m_hOpbSrcDC, 3, 35, 16114093
    SetPixelV m_hOpbSrcDC, 4, 35, 16114094
    SetPixelV m_hOpbSrcDC, 5, 35, 16114352
    SetPixelV m_hOpbSrcDC, 6, 35, 16246194
    SetPixelV m_hOpbSrcDC, 7, 35, 16246196
    SetPixelV m_hOpbSrcDC, 8, 35, 16246713
    SetPixelV m_hOpbSrcDC, 9, 35, 16312510
    SetPixelV m_hOpbSrcDC, 10, 35, 16378562
    SetPixelV m_hOpbSrcDC, 11, 35, 14333340
    SetPixelV m_hOpbSrcDC, 12, 35, 14531748
    SetPixelV m_hOpbSrcDC, 0, 36, 16711935
    SetPixelV m_hOpbSrcDC, 1, 36, 13475711
    SetPixelV m_hOpbSrcDC, 2, 36, 14334880
    SetPixelV m_hOpbSrcDC, 3, 36, 16114093
    SetPixelV m_hOpbSrcDC, 4, 36, 16114093
    SetPixelV m_hOpbSrcDC, 5, 36, 16114093
    SetPixelV m_hOpbSrcDC, 6, 36, 16114352
    SetPixelV m_hOpbSrcDC, 7, 36, 16114354
    SetPixelV m_hOpbSrcDC, 8, 36, 16246453
    SetPixelV m_hOpbSrcDC, 9, 36, 16312250
    SetPixelV m_hOpbSrcDC, 10, 36, 15059637
    SetPixelV m_hOpbSrcDC, 11, 36, 13475711
    SetPixelV m_hOpbSrcDC, 12, 36, 16711935
    SetPixelV m_hOpbSrcDC, 0, 37, 16711935
    SetPixelV m_hOpbSrcDC, 1, 37, 16711935
    SetPixelV m_hOpbSrcDC, 2, 37, 13475711
    SetPixelV m_hOpbSrcDC, 3, 37, 13872268
    SetPixelV m_hOpbSrcDC, 4, 37, 16114093
    SetPixelV m_hOpbSrcDC, 5, 37, 16114093
    SetPixelV m_hOpbSrcDC, 6, 37, 16114094
    SetPixelV m_hOpbSrcDC, 7, 37, 16114352
    SetPixelV m_hOpbSrcDC, 8, 37, 16246194
    SetPixelV m_hOpbSrcDC, 9, 37, 14201753
    SetPixelV m_hOpbSrcDC, 10, 37, 13475711
    SetPixelV m_hOpbSrcDC, 11, 37, 16711935
    SetPixelV m_hOpbSrcDC, 12, 37, 16711935
    SetPixelV m_hOpbSrcDC, 0, 38, 16711935
    SetPixelV m_hOpbSrcDC, 1, 38, 16711935
    SetPixelV m_hOpbSrcDC, 2, 38, 16711935
    SetPixelV m_hOpbSrcDC, 3, 38, 14531748
    SetPixelV m_hOpbSrcDC, 4, 38, 13277558
    SetPixelV m_hOpbSrcDC, 5, 38, 12749671
    SetPixelV m_hOpbSrcDC, 6, 38, 12683620
    SetPixelV m_hOpbSrcDC, 7, 38, 12749671
    SetPixelV m_hOpbSrcDC, 8, 38, 13277558
    SetPixelV m_hOpbSrcDC, 9, 38, 14531748
    SetPixelV m_hOpbSrcDC, 10, 38, 16711935
    SetPixelV m_hOpbSrcDC, 11, 38, 16711935
    SetPixelV m_hOpbSrcDC, 12, 38, 16711935
    SetPixelV m_hOpbSrcDC, 0, 39, 16711935
    SetPixelV m_hOpbSrcDC, 1, 39, 16711935
    SetPixelV m_hOpbSrcDC, 2, 39, 16711935
    SetPixelV m_hOpbSrcDC, 3, 39, 15984605
    SetPixelV m_hOpbSrcDC, 4, 39, 15522250
    SetPixelV m_hOpbSrcDC, 5, 39, 15324098
    SetPixelV m_hOpbSrcDC, 6, 39, 15258305
    SetPixelV m_hOpbSrcDC, 7, 39, 15324098
    SetPixelV m_hOpbSrcDC, 8, 39, 15522250
    SetPixelV m_hOpbSrcDC, 9, 39, 15984605
    SetPixelV m_hOpbSrcDC, 10, 39, 16711935
    SetPixelV m_hOpbSrcDC, 11, 39, 16711935
    SetPixelV m_hOpbSrcDC, 12, 39, 16711935
    SetPixelV m_hOpbSrcDC, 0, 40, 16711935
    SetPixelV m_hOpbSrcDC, 1, 40, 16711935
    SetPixelV m_hOpbSrcDC, 2, 40, 15588302
    SetPixelV m_hOpbSrcDC, 3, 40, 15720660
    SetPixelV m_hOpbSrcDC, 4, 40, 16117736
    SetPixelV m_hOpbSrcDC, 5, 40, 16249838
    SetPixelV m_hOpbSrcDC, 6, 40, 16315631
    SetPixelV m_hOpbSrcDC, 7, 40, 16249838
    SetPixelV m_hOpbSrcDC, 8, 40, 16117736
    SetPixelV m_hOpbSrcDC, 9, 40, 15720660
    SetPixelV m_hOpbSrcDC, 10, 40, 15588302
    SetPixelV m_hOpbSrcDC, 11, 40, 16711935
    SetPixelV m_hOpbSrcDC, 12, 40, 16711935
    SetPixelV m_hOpbSrcDC, 0, 41, 16711935
    SetPixelV m_hOpbSrcDC, 1, 41, 15588302
    SetPixelV m_hOpbSrcDC, 2, 41, 15918812
    SetPixelV m_hOpbSrcDC, 3, 41, 16381425
    SetPixelV m_hOpbSrcDC, 4, 41, 16381682
    SetPixelV m_hOpbSrcDC, 5, 41, 16381683
    SetPixelV m_hOpbSrcDC, 6, 41, 16381683
    SetPixelV m_hOpbSrcDC, 7, 41, 16381683
    SetPixelV m_hOpbSrcDC, 8, 41, 16381682
    SetPixelV m_hOpbSrcDC, 9, 41, 16381425
    SetPixelV m_hOpbSrcDC, 10, 41, 15918812
    SetPixelV m_hOpbSrcDC, 11, 41, 15588302
    SetPixelV m_hOpbSrcDC, 12, 41, 16711935
    SetPixelV m_hOpbSrcDC, 0, 42, 15984605
    SetPixelV m_hOpbSrcDC, 1, 42, 15720660
    SetPixelV m_hOpbSrcDC, 2, 42, 16381682
    SetPixelV m_hOpbSrcDC, 3, 42, 16513270
    SetPixelV m_hOpbSrcDC, 4, 42, 16579577
    SetPixelV m_hOpbSrcDC, 5, 42, 16645371
    SetPixelV m_hOpbSrcDC, 6, 42, 16645371
    SetPixelV m_hOpbSrcDC, 7, 42, 16645371
    SetPixelV m_hOpbSrcDC, 8, 42, 16579577
    SetPixelV m_hOpbSrcDC, 9, 42, 16513270
    SetPixelV m_hOpbSrcDC, 10, 42, 16381682
    SetPixelV m_hOpbSrcDC, 11, 42, 15720660
    SetPixelV m_hOpbSrcDC, 12, 42, 15984605
    SetPixelV m_hOpbSrcDC, 0, 43, 15522250
    SetPixelV m_hOpbSrcDC, 1, 43, 16249066
    SetPixelV m_hOpbSrcDC, 2, 43, 16513527
    SetPixelV m_hOpbSrcDC, 3, 43, 16645628
    SetPixelV m_hOpbSrcDC, 4, 43, 16711422
    SetPixelV m_hOpbSrcDC, 5, 43, 16777215
    SetPixelV m_hOpbSrcDC, 6, 43, 16777215
    SetPixelV m_hOpbSrcDC, 7, 43, 16777215
    SetPixelV m_hOpbSrcDC, 8, 43, 16711422
    SetPixelV m_hOpbSrcDC, 9, 43, 16645628
    SetPixelV m_hOpbSrcDC, 10, 43, 16513527
    SetPixelV m_hOpbSrcDC, 11, 43, 16249066
    SetPixelV m_hOpbSrcDC, 12, 43, 15522250
    SetPixelV m_hOpbSrcDC, 0, 44, 15324098
    SetPixelV m_hOpbSrcDC, 1, 44, 16447477
    SetPixelV m_hOpbSrcDC, 2, 44, 16645628
    SetPixelV m_hOpbSrcDC, 3, 44, 16776958
    SetPixelV m_hOpbSrcDC, 4, 44, 16777215
    SetPixelV m_hOpbSrcDC, 5, 44, 16777215
    SetPixelV m_hOpbSrcDC, 6, 44, 16777215
    SetPixelV m_hOpbSrcDC, 7, 44, 16777215
    SetPixelV m_hOpbSrcDC, 8, 44, 16777215
    SetPixelV m_hOpbSrcDC, 9, 44, 16776958
    SetPixelV m_hOpbSrcDC, 10, 44, 16645628
    SetPixelV m_hOpbSrcDC, 11, 44, 16447477
    SetPixelV m_hOpbSrcDC, 12, 44, 15324098
    SetPixelV m_hOpbSrcDC, 0, 45, 15258305
    SetPixelV m_hOpbSrcDC, 1, 45, 16579577
    SetPixelV m_hOpbSrcDC, 2, 45, 16711422
    SetPixelV m_hOpbSrcDC, 3, 45, 16777215
    SetPixelV m_hOpbSrcDC, 4, 45, 16777215
    SetPixelV m_hOpbSrcDC, 5, 45, 16777215
    SetPixelV m_hOpbSrcDC, 6, 45, 16777215
    SetPixelV m_hOpbSrcDC, 7, 45, 16777215
    SetPixelV m_hOpbSrcDC, 8, 45, 16777215
    SetPixelV m_hOpbSrcDC, 9, 45, 16777215
    SetPixelV m_hOpbSrcDC, 10, 45, 16711422
    SetPixelV m_hOpbSrcDC, 11, 45, 16579577
    SetPixelV m_hOpbSrcDC, 12, 45, 15258305
    SetPixelV m_hOpbSrcDC, 0, 46, 15324098
    SetPixelV m_hOpbSrcDC, 1, 46, 16579577
    SetPixelV m_hOpbSrcDC, 2, 46, 16777215
    SetPixelV m_hOpbSrcDC, 3, 46, 16777215
    SetPixelV m_hOpbSrcDC, 4, 46, 16777215
    SetPixelV m_hOpbSrcDC, 5, 46, 16777215
    SetPixelV m_hOpbSrcDC, 6, 46, 16777215
    SetPixelV m_hOpbSrcDC, 7, 46, 16777215
    SetPixelV m_hOpbSrcDC, 8, 46, 16777215
    SetPixelV m_hOpbSrcDC, 9, 46, 16777215
    SetPixelV m_hOpbSrcDC, 10, 46, 16777215
    SetPixelV m_hOpbSrcDC, 11, 46, 16579577
    SetPixelV m_hOpbSrcDC, 12, 46, 15324098
    SetPixelV m_hOpbSrcDC, 0, 47, 15522250
    SetPixelV m_hOpbSrcDC, 1, 47, 16447218
    SetPixelV m_hOpbSrcDC, 2, 47, 16777215
    SetPixelV m_hOpbSrcDC, 3, 47, 16777215
    SetPixelV m_hOpbSrcDC, 4, 47, 16777215
    SetPixelV m_hOpbSrcDC, 5, 47, 16777215
    SetPixelV m_hOpbSrcDC, 6, 47, 16777215
    SetPixelV m_hOpbSrcDC, 7, 47, 16777215
    SetPixelV m_hOpbSrcDC, 8, 47, 16777215
    SetPixelV m_hOpbSrcDC, 9, 47, 16777215
    SetPixelV m_hOpbSrcDC, 10, 47, 16777215
    SetPixelV m_hOpbSrcDC, 11, 47, 16447218
    SetPixelV m_hOpbSrcDC, 12, 47, 15522250
    SetPixelV m_hOpbSrcDC, 0, 48, 15984605
    SetPixelV m_hOpbSrcDC, 1, 48, 15852505
    SetPixelV m_hOpbSrcDC, 2, 48, 16777215
    SetPixelV m_hOpbSrcDC, 3, 48, 16777215
    SetPixelV m_hOpbSrcDC, 4, 48, 16777215
    SetPixelV m_hOpbSrcDC, 5, 48, 16777215
    SetPixelV m_hOpbSrcDC, 6, 48, 16777215
    SetPixelV m_hOpbSrcDC, 7, 48, 16777215
    SetPixelV m_hOpbSrcDC, 8, 48, 16777215
    SetPixelV m_hOpbSrcDC, 9, 48, 16777215
    SetPixelV m_hOpbSrcDC, 10, 48, 16777215
    SetPixelV m_hOpbSrcDC, 11, 48, 15852505
    SetPixelV m_hOpbSrcDC, 12, 48, 15984605
    SetPixelV m_hOpbSrcDC, 0, 49, 16711935
    SetPixelV m_hOpbSrcDC, 1, 49, 15588302
    SetPixelV m_hOpbSrcDC, 2, 49, 16116964
    SetPixelV m_hOpbSrcDC, 3, 49, 16777215
    SetPixelV m_hOpbSrcDC, 4, 49, 16777215
    SetPixelV m_hOpbSrcDC, 5, 49, 16777215
    SetPixelV m_hOpbSrcDC, 6, 49, 16777215
    SetPixelV m_hOpbSrcDC, 7, 49, 16777215
    SetPixelV m_hOpbSrcDC, 8, 49, 16777215
    SetPixelV m_hOpbSrcDC, 9, 49, 16777215
    SetPixelV m_hOpbSrcDC, 10, 49, 16116964
    SetPixelV m_hOpbSrcDC, 11, 49, 15588302
    SetPixelV m_hOpbSrcDC, 12, 49, 16711935
    SetPixelV m_hOpbSrcDC, 0, 50, 16711935
    SetPixelV m_hOpbSrcDC, 1, 50, 16711935
    SetPixelV m_hOpbSrcDC, 2, 50, 15588302
    SetPixelV m_hOpbSrcDC, 3, 50, 15918298
    SetPixelV m_hOpbSrcDC, 4, 50, 16513270
    SetPixelV m_hOpbSrcDC, 5, 50, 16711421
    SetPixelV m_hOpbSrcDC, 6, 50, 16777215
    SetPixelV m_hOpbSrcDC, 7, 50, 16711421
    SetPixelV m_hOpbSrcDC, 8, 50, 16513270
    SetPixelV m_hOpbSrcDC, 9, 50, 15918298
    SetPixelV m_hOpbSrcDC, 10, 50, 15588302
    SetPixelV m_hOpbSrcDC, 11, 50, 16711935
    SetPixelV m_hOpbSrcDC, 12, 50, 16711935
    SetPixelV m_hOpbSrcDC, 0, 51, 16711935
    SetPixelV m_hOpbSrcDC, 1, 51, 16711935
    SetPixelV m_hOpbSrcDC, 2, 51, 16711935
    SetPixelV m_hOpbSrcDC, 3, 51, 15984605
    SetPixelV m_hOpbSrcDC, 4, 51, 15522250
    SetPixelV m_hOpbSrcDC, 5, 51, 15324098
    SetPixelV m_hOpbSrcDC, 6, 51, 15258305
    SetPixelV m_hOpbSrcDC, 7, 51, 15324098
    SetPixelV m_hOpbSrcDC, 8, 51, 15522250
    SetPixelV m_hOpbSrcDC, 9, 51, 15984605
    SetPixelV m_hOpbSrcDC, 10, 51, 16711935
    SetPixelV m_hOpbSrcDC, 11, 51, 16711935
    SetPixelV m_hOpbSrcDC, 12, 51, 16711935
    
    SetPixelV m_hHdbSrcDC, 0, 0, 16709098
    SetPixelV m_hHdbSrcDC, 1, 0, 16709098
    SetPixelV m_hHdbSrcDC, 2, 0, 16709098
    SetPixelV m_hHdbSrcDC, 3, 0, 16639677
    SetPixelV m_hHdbSrcDC, 4, 0, 16707283
    SetPixelV m_hHdbSrcDC, 5, 0, 16639677
    SetPixelV m_hHdbSrcDC, 6, 0, 16504727
    SetPixelV m_hHdbSrcDC, 7, 0, 16504727
    SetPixelV m_hHdbSrcDC, 8, 0, 16504727
    SetPixelV m_hHdbSrcDC, 0, 1, 16708841
    SetPixelV m_hHdbSrcDC, 1, 1, 16708841
    SetPixelV m_hHdbSrcDC, 2, 1, 16708841
    SetPixelV m_hHdbSrcDC, 3, 1, 16572071
    SetPixelV m_hHdbSrcDC, 4, 1, 16773078
    SetPixelV m_hHdbSrcDC, 5, 1, 16572071
    SetPixelV m_hHdbSrcDC, 6, 1, 16436862
    SetPixelV m_hHdbSrcDC, 7, 1, 16768159
    SetPixelV m_hHdbSrcDC, 8, 1, 16436862
    SetPixelV m_hHdbSrcDC, 0, 2, 16643305
    SetPixelV m_hHdbSrcDC, 1, 2, 16643048
    SetPixelV m_hHdbSrcDC, 2, 2, 16445408
    SetPixelV m_hHdbSrcDC, 3, 2, 16569487
    SetPixelV m_hHdbSrcDC, 4, 2, 16772819
    SetPixelV m_hHdbSrcDC, 5, 2, 16372107
    SetPixelV m_hHdbSrcDC, 6, 2, 16434534
    SetPixelV m_hHdbSrcDC, 7, 2, 16768416
    SetPixelV m_hHdbSrcDC, 8, 2, 16237412
    SetPixelV m_hHdbSrcDC, 0, 3, 16643049
    SetPixelV m_hHdbSrcDC, 1, 3, 16642790
    SetPixelV m_hHdbSrcDC, 2, 3, 16246998
    SetPixelV m_hHdbSrcDC, 3, 3, 16501623
    SetPixelV m_hHdbSrcDC, 4, 3, 16772560
    SetPixelV m_hHdbSrcDC, 5, 3, 16106865
    SetPixelV m_hHdbSrcDC, 6, 3, 16433758
    SetPixelV m_hHdbSrcDC, 7, 3, 16768674
    SetPixelV m_hHdbSrcDC, 8, 3, 16039259
    SetPixelV m_hHdbSrcDC, 0, 4, 16643563
    SetPixelV m_hHdbSrcDC, 1, 4, 16576996
    SetPixelV m_hHdbSrcDC, 2, 4, 15718844
    SetPixelV m_hHdbSrcDC, 3, 4, 16434533
    SetPixelV m_hHdbSrcDC, 4, 4, 16772300
    SetPixelV m_hHdbSrcDC, 5, 4, 15579227
    SetPixelV m_hHdbSrcDC, 6, 4, 16433758
    SetPixelV m_hHdbSrcDC, 7, 4, 16768932
    SetPixelV m_hHdbSrcDC, 8, 4, 15578454
    SetPixelV m_hHdbSrcDC, 0, 5, 16643564
    SetPixelV m_hHdbSrcDC, 1, 5, 16576738
    SetPixelV m_hHdbSrcDC, 2, 5, 15454638
    SetPixelV m_hHdbSrcDC, 3, 5, 16433758
    SetPixelV m_hHdbSrcDC, 4, 5, 16706504
    SetPixelV m_hHdbSrcDC, 5, 5, 15380820
    SetPixelV m_hHdbSrcDC, 6, 5, 16433758
    SetPixelV m_hHdbSrcDC, 7, 5, 16769190
    SetPixelV m_hHdbSrcDC, 8, 5, 15380820
    SetPixelV m_hHdbSrcDC, 0, 6, 16643821
    SetPixelV m_hHdbSrcDC, 1, 6, 16510944
    SetPixelV m_hHdbSrcDC, 2, 6, 15124383
    SetPixelV m_hHdbSrcDC, 3, 6, 16433758
    SetPixelV m_hHdbSrcDC, 4, 6, 16705988
    SetPixelV m_hHdbSrcDC, 5, 6, 15051857
    SetPixelV m_hHdbSrcDC, 6, 6, 16433758
    SetPixelV m_hHdbSrcDC, 7, 6, 16769704
    SetPixelV m_hHdbSrcDC, 8, 6, 15051857
    SetPixelV m_hHdbSrcDC, 0, 7, 16643823
    SetPixelV m_hHdbSrcDC, 1, 7, 16510431
    SetPixelV m_hHdbSrcDC, 2, 7, 14860180
    SetPixelV m_hHdbSrcDC, 3, 7, 16433758
    SetPixelV m_hHdbSrcDC, 4, 7, 16705729
    SetPixelV m_hHdbSrcDC, 5, 7, 14788687
    SetPixelV m_hHdbSrcDC, 6, 7, 16433758
    SetPixelV m_hHdbSrcDC, 7, 7, 16769963
    SetPixelV m_hHdbSrcDC, 8, 7, 14788687
    SetPixelV m_hHdbSrcDC, 0, 8, 16644081
    SetPixelV m_hHdbSrcDC, 1, 8, 16444637
    SetPixelV m_hHdbSrcDC, 2, 8, 14464386
    SetPixelV m_hHdbSrcDC, 3, 8, 16433758
    SetPixelV m_hHdbSrcDC, 4, 8, 16705469
    SetPixelV m_hHdbSrcDC, 5, 8, 14459724
    SetPixelV m_hHdbSrcDC, 6, 8, 16433758
    SetPixelV m_hHdbSrcDC, 7, 8, 16770477
    SetPixelV m_hHdbSrcDC, 8, 8, 14459724
    SetPixelV m_hHdbSrcDC, 0, 9, 16512493
    SetPixelV m_hHdbSrcDC, 1, 9, 16180691
    SetPixelV m_hHdbSrcDC, 2, 9, 14266490
    SetPixelV m_hHdbSrcDC, 3, 9, 16433758
    SetPixelV m_hHdbSrcDC, 4, 9, 16638383
    SetPixelV m_hHdbSrcDC, 5, 9, 14393931
    SetPixelV m_hHdbSrcDC, 6, 9, 16433758
    SetPixelV m_hHdbSrcDC, 7, 9, 16770992
    SetPixelV m_hHdbSrcDC, 8, 9, 14393931
    SetPixelV m_hHdbSrcDC, 0, 10, 16512237
    SetPixelV m_hHdbSrcDC, 1, 10, 16180691
    SetPixelV m_hHdbSrcDC, 2, 10, 14266490
    SetPixelV m_hHdbSrcDC, 3, 10, 16433758
    SetPixelV m_hHdbSrcDC, 4, 10, 16638126
    SetPixelV m_hHdbSrcDC, 5, 10, 14393931
    SetPixelV m_hHdbSrcDC, 6, 10, 16433758
    SetPixelV m_hHdbSrcDC, 7, 10, 16771251
    SetPixelV m_hHdbSrcDC, 8, 10, 14393931
    SetPixelV m_hHdbSrcDC, 0, 11, 16511979
    SetPixelV m_hHdbSrcDC, 1, 11, 16180691
    SetPixelV m_hHdbSrcDC, 2, 11, 14332541
    SetPixelV m_hHdbSrcDC, 3, 11, 16433758
    SetPixelV m_hHdbSrcDC, 4, 11, 16638382
    SetPixelV m_hHdbSrcDC, 5, 11, 14459724
    SetPixelV m_hHdbSrcDC, 6, 11, 16433758
    SetPixelV m_hHdbSrcDC, 7, 11, 16771766
    SetPixelV m_hHdbSrcDC, 8, 11, 14459724
    SetPixelV m_hHdbSrcDC, 0, 12, 16445669
    SetPixelV m_hHdbSrcDC, 1, 12, 16180691
    SetPixelV m_hHdbSrcDC, 2, 12, 14662284
    SetPixelV m_hHdbSrcDC, 3, 12, 16433758
    SetPixelV m_hHdbSrcDC, 4, 12, 16638383
    SetPixelV m_hHdbSrcDC, 5, 12, 14788686
    SetPixelV m_hHdbSrcDC, 6, 12, 16433758
    SetPixelV m_hHdbSrcDC, 7, 12, 16772281
    SetPixelV m_hHdbSrcDC, 8, 12, 14788686
    SetPixelV m_hHdbSrcDC, 0, 13, 16379618
    SetPixelV m_hHdbSrcDC, 1, 13, 16180691
    SetPixelV m_hHdbSrcDC, 2, 13, 14926231
    SetPixelV m_hHdbSrcDC, 3, 13, 16433758
    SetPixelV m_hHdbSrcDC, 4, 13, 16638383
    SetPixelV m_hHdbSrcDC, 5, 13, 15051857
    SetPixelV m_hHdbSrcDC, 6, 13, 16434533
    SetPixelV m_hHdbSrcDC, 7, 13, 16772540
    SetPixelV m_hHdbSrcDC, 8, 13, 15052117
    SetPixelV m_hHdbSrcDC, 0, 14, 16313566
    SetPixelV m_hHdbSrcDC, 1, 14, 16180691
    SetPixelV m_hHdbSrcDC, 2, 14, 15124643
    SetPixelV m_hHdbSrcDC, 3, 14, 16433758
    SetPixelV m_hHdbSrcDC, 4, 14, 16638383
    SetPixelV m_hHdbSrcDC, 5, 14, 15315284
    SetPixelV m_hHdbSrcDC, 6, 14, 16501359
    SetPixelV m_hHdbSrcDC, 7, 14, 16773054
    SetPixelV m_hHdbSrcDC, 8, 14, 15316575
    SetPixelV m_hHdbSrcDC, 0, 15, 16312794
    SetPixelV m_hHdbSrcDC, 1, 15, 16180691
    SetPixelV m_hHdbSrcDC, 2, 15, 15388591
    SetPixelV m_hHdbSrcDC, 3, 15, 16433758
    SetPixelV m_hHdbSrcDC, 4, 15, 16638383
    SetPixelV m_hHdbSrcDC, 5, 15, 15578454
    SetPixelV m_hHdbSrcDC, 6, 15, 16502908
    SetPixelV m_hHdbSrcDC, 7, 15, 16773829
    SetPixelV m_hHdbSrcDC, 8, 15, 15646573
    SetPixelV m_hHdbSrcDC, 0, 16, 16180949
    SetPixelV m_hHdbSrcDC, 1, 16, 16180691
    SetPixelV m_hHdbSrcDC, 2, 16, 15850692
    SetPixelV m_hHdbSrcDC, 3, 16, 16433758
    SetPixelV m_hHdbSrcDC, 4, 16, 16638383
    SetPixelV m_hHdbSrcDC, 5, 16, 16039258
    SetPixelV m_hHdbSrcDC, 6, 16, 16569995
    SetPixelV m_hHdbSrcDC, 7, 16, 16773832
    SetPixelV m_hHdbSrcDC, 8, 16, 16174979
    SetPixelV m_hHdbSrcDC, 0, 17, 16180691
    SetPixelV m_hHdbSrcDC, 1, 17, 16180691
    SetPixelV m_hHdbSrcDC, 2, 17, 15983052
    SetPixelV m_hHdbSrcDC, 3, 17, 16433758
    SetPixelV m_hHdbSrcDC, 4, 17, 16638124
    SetPixelV m_hHdbSrcDC, 5, 17, 16236636
    SetPixelV m_hHdbSrcDC, 6, 17, 16637337
    SetPixelV m_hHdbSrcDC, 7, 17, 16773832
    SetPixelV m_hHdbSrcDC, 8, 17, 16439700
    SetPixelV m_hHdbSrcDC, 0, 18, 16180691
    SetPixelV m_hHdbSrcDC, 1, 18, 16180691
    SetPixelV m_hHdbSrcDC, 2, 18, 16180691
    SetPixelV m_hHdbSrcDC, 3, 18, 16501875
    SetPixelV m_hHdbSrcDC, 4, 18, 16571816
    SetPixelV m_hHdbSrcDC, 5, 18, 16501875
    SetPixelV m_hHdbSrcDC, 6, 18, 16638888
    SetPixelV m_hHdbSrcDC, 7, 18, 16773832
    SetPixelV m_hHdbSrcDC, 8, 18, 16638888
    SetPixelV m_hHdbSrcDC, 0, 19, 15915712
    SetPixelV m_hHdbSrcDC, 1, 19, 15915712
    SetPixelV m_hHdbSrcDC, 2, 19, 15915712
    SetPixelV m_hHdbSrcDC, 3, 19, 15915712
    SetPixelV m_hHdbSrcDC, 4, 19, 15915712
    SetPixelV m_hHdbSrcDC, 5, 19, 15915712
    SetPixelV m_hHdbSrcDC, 6, 19, 15915712
    SetPixelV m_hHdbSrcDC, 7, 19, 15915712
    SetPixelV m_hHdbSrcDC, 8, 19, 15915712
End Sub

