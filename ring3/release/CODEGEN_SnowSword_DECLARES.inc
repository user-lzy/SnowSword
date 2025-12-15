'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------

' 这里是全局定义源码，所有type、全局变量、函数定义 都在此处。
#include Once "Afx/CGdiPlus/CGdiPlus.inc"
#include Once "afx\CWindow.inc"
'Type FormControlsPro_TYPE '保存控件的私有属性，每个控件都不同，全部在此，各取所需。
'   hWndParent     As hWnd       '父窗口句柄
'   nName          As String     '名称\1\用来代码中识别对象的名称\Command\
'   Index          As Long       '控件数组索引，小于零表示非控件数组
'   IDC            As Long       '
'   CtrlFocus      As HWND       ' 当前焦点在什么控件
'   BigIcon        As HICON      '窗口大图标，主要用来主窗口
'   SmallIcon      As HICON      '窗口小图标
'   nText          as CWSTR      '
'   ControlType    as Long       '控件类型，为了画控件  >=100 为虚拟控件  100=LABEL   TEXT=1 Button=2
'   nData          As String     '编写控件使用，数据存放，无特定，控件根据自己需要存放任意数据。
'   CtlData(99)    As Integer    '为每个控件提供 100 个数据储存(编写控件使用，控件根据自己需要存放任意数据)。
'   UserData(99)   As Integer    '为每个控件提供 100 个数据储存(用户使用)。
'   Style          as UInteger   '样式，每个控件都有自己的定义
'   TransPer       As Long       '透明度\0\窗口透明度，百分比(0--100)，0%不透明 100%全透明\0\
'   TransColor     As Long = &H197F7F7F  '透明颜色\3\透明颜色，需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
'   MousePointer   As Long               '指针\2\鼠标在窗口上的形状\0 - 默认,1 - 后台运行,2 - 标准箭头,3 - 十字光标,4 - 箭头和问号,5 - 文本工字光标,6 - 不可用禁止圈,7 - 移动,8 - 双箭头↙↗,9 - 双箭头↑↓,10 - 双箭头向↖↘,11 - 双箭头←→,12 - 垂直箭头,13 - 沙漏,14 - 手型
'   ForeColor      As Long = &H197F7F7F  '文字色 需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
'   BackColor      As Long = &H197F7F7F  '背景色 需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
'   hBackBrush     As HBRUSH             '背景刷子，返回给窗口和控件用
'   nFont          As String                   '字体\4\用于此对象的文本字体。\微软雅黑,9,0\
'   Tag            As CWSTR                    ' 附加 \ 1 \ 私有自定义文本与控件关联。 \ \
'   ToolTip        As CWSTR                    ' 提示 \ 1 \ 一个提示，当鼠标光标悬停在控件时显示它。 \ \
'   ToolTipBalloon As Long                     ' 气球样式 \ 2 \ 一个气球样式显示工具提示。 \ False \ True, False
'   ToolWnd        As HWND                     '提示窗口句柄，用来销毁
'   nCursor        As HCURSOR                  '鼠标指针句柄
'   nLeft          As Long                     '返回/设置相对于父窗口的 X 响应DPI数值，为 100%DPI数值（像素）
'   nTop           As Long                     '返回/设置相对于父窗口的 Y 响应DPI数值，为 100%DPI数值（像素）
'   nWidth         As Long                     '返回/设置控件宽度 响应DPI数值，为 100%DPI数值（像素）
'   nHeight        As Long                     '返回/设置控件高度 响应DPI数值，为 100%DPI数值（像素））
'   anchor         AS LONG                     '控件布局 自动调整方式
'   nRight         As Long                     '控件布局 用，相对于窗口右边距离 响应DPI数值，为 100%DPI数值（像素））
'   nBottom        As Long                     '控件布局 用，相对于窗口底边距离 响应DPI数值，为 100%DPI数值（像素））
'   centerX        AS LONG                     '控件布局 用 中点  响应DPI数值，为 100%DPI数值（像素）
'   centerY        AS LONG                     '控件布局 用
'   VrControls     As FormControlsPro_TYPE ptr '如果是主窗口带 虚拟控件 ，就是下一个虚拟控件链表指针，直到为 0 表示没了
'End Type




Declare Function gFLY_GetFontHandles(mFont As String) As HFONT
Declare Function GetCodeColorGDI(coColor As Long,defaultColor As Long =-1) As Long
Declare Function GetCodeColorGDIplue(coColor As Long,defaultColor As Long =0) As Long
Declare Function FF_AddTooltip(hWndForm AS HWND, strTooltipText AS wString, bBalloon AS Long, X as Long = 0, Y As Long = 0, W As Long = 0, H As Long = 0) As HWND
Declare Sub FLY_VFB_Layout_hWndForm(hWndForm As HWND) 
Declare Sub FLY_VFB_Layout_Handle(fp As FormControlsPro_TYPE ptr, pWidth AS LONG, pHeight AS LONG, nWidth AS LONG, nHeight AS LONG, ByRef x AS LONG, ByRef y AS LONG, ByRef xWidth AS LONG, ByRef yHeight AS LONG)
#include Once "win/tlhelp32.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|0]
#include Once "win/psapi.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|1]
#include Once "win/winnt.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|2]
#include Once "Afx/CWmiDisp.inc" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|3]
#include Once "win/winbase.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModPrivileges.Inc|0]
#include Once "win/winnt.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|0]
#ifdef PrintLog
#endif
#ifdef PrintLastError
#endif
#include Once "win/psapi.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModKernelModule.Inc|0]
#include Once "win/winsvc.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|0]
#include Once "win/winioctl.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|1]
#include Once "win/winerror.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|2]
#include Once "win/winreg.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModReg.Inc|0]
#include Once "win/winuser.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModWindow.Inc|0]
#include Once "win/winbase.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLowFile.Inc|0]
#include Once "win/winuser.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|3]
#include Once "afx/CWindow.inc" 'WinFBX 库，是WIN系统增强基础库，使用窗口和控件必须要用到它。 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|330]
#Include  Once "win\tlhelp32.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|424]
#include Once "Afx/CFileSys.inc" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|614]
#Include Once "Inc/NtDef.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|769]
#Include Once "win/mmsystem.bi" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|812]

Declare Sub PrintA(p1 As CWSTR = wChr(0), p2 As CWSTR = wChr(0), p3 As CWSTR = wChr(0), p4 As CWSTR = wChr(0), p5 As CWSTR = wChr(0), p6 As CWSTR = wChr(0), p7 As CWSTR = wChr(0), p8 As CWSTR = wChr(0), p9 As CWSTR = wChr(0), p10 As CWSTR = wChr(0)) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|331]
Declare Function WinErrorMsg (ByVal dError as DWord)  as String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|385]
Declare Function RightW(wszStr As StringW, n As Integer ) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|407]
Declare Function LenW(wszStr As StringW) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|411]
Declare Function InStrRevW(wszStr As StringW, substring As StringW,start As Integer =-1) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|415]
Declare Function GetAllPID(Sof() as String, PID() as DWord, ByVal c as Integer = 0) as Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|425]
Declare Function InStrW Overload(wszStr As StringW, substring As StringW) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|459]
Declare Function InStrW Overload(start As Integer, wszStr As StringW, substring As StringW) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|462]
Declare Function MsgBox Overload(CH_E68F90E7A4BAE69687E69CAC_ As WString , CH_E98089E9A1B9_ As ULong = MB_OK, CH_E6A087E9A298E69687E69CAC_ As WString = App.ProductName , CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND = AfxGetHwndFromPID(GetCurrentProcessId)) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|473]
Declare Function MsgBox Overload(CH_E68F90E7A4BAE69687E69CAC_ As String , CH_E98089E9A1B9_ As ULong = MB_OK, CH_E6A087E9A298E69687E69CAC_ As String = App.ProductName , CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND = AfxGetHwndFromPID(GetCurrentProcessId)) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|478]
Declare Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As WString, CH_E6A087E9A298E69687E69CAC_ As WString = App.ProductName , CH_E98089E9A1B9_ As ULong = MB_OK) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|483]
Declare Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E6A087E9A298E69687E69CAC_ As String = App.ProductName , CH_E98089E9A1B9_ As ULong = MB_OK) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|488]
Declare Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As WString, CH_E98089E9A1B9_ As ULong ) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|493]
Declare Function MsgBox Overload(CH_E788B6E7AA97E58FA3E58FA5E69F84_ As HWND, CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E98089E9A1B9_ As ULong ) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|499]
Declare Function MsgBox Overload( CH_E68F90E7A4BAE69687E69CAC_ As String, CH_E6A087E9A298E69687E69CAC_ As String,ByVal CH_E98089E9A1B9_ As Integer=0,ByVal  CH_E788B6E7AA97E58FA3E58FA5E69F84_ As Hwnd = 0 ) As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|505]
Declare Function FF_Replace( ByRef sMainString   as String, _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|533]
ByRef sMatchPattern as String, _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|534]
ByRef sReplaceWith  as String _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|535]
) as String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|536]
Declare Function StrToWStr(sStr As String, CodePage As UINT = CP_ACP) As String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|597]
Declare Function GetDiskLabel(nDrive As WString ) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|615]
Declare Function Split(TEXT as String, DELIMIT as String, RET() as String) as Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|623]
Declare Function MidW Overload(wszStr As StringW, start As Integer, n As Integer ) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|723]
Declare Function MidW Overload(wszStr As StringW, start As Integer) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|726]
Declare Sub FF_DoEvents() '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|732]
Declare Sub SetWindowZ(ByVal hWndControl as HWnd,ByVal hWndlnsertAfter as HWND =HWND_TOP ) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|753]
Declare Sub PopupMenu(hWndForm as HWnd, yMenu as HMENU) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|761]
Declare Function Process_GetPath(pid As Integer) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|771]
Declare Function RemoteDLLinjection(ByVal jID As DWord, nDLL As WString ) As String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|813]
Declare Sub DrawLine(ByVal hDC as hDC, ByVal X1 as Integer, ByVal Y1 as Integer, ByVal X2 as Integer, ByVal Y2 as Integer, _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|872]
ByVal nPenStyle as Integer, ByVal nWidth as Integer, ByVal crColor as Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|873]
Declare Sub DrawFrame(DC as hDC, X1 as Long, Y1 as Long, X2 as Long, Y2 as Long, C as Long, L as Long = PS_NULL, cL as Long = 0, wL as Long = 1) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|903]
Declare Sub DrawCircleFrame(ByVal sDC as hDC, ByVal X as Long, ByVal Y as Long, ByVal hWidth as Long, ByVal hHeight as Long, ByVal xCircle as Long, ByVal yCircle as Long, _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|944]
ByVal LineWidth as Long, ByVal LineType as Long , ByVal bColor as Long, ByVal tColor as Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|945]
Declare Sub DrawEllipse(ByVal hDC as hDC, ByVal X as Integer, ByVal Y as Integer, ByVal hWidth as Integer, ByVal hHeight as Integer, _ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|997]
ByVal LineType as Integer ,ByVal LineWidth as Integer, ByVal bColor as  Integer , ByVal tColor as  Integer ) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|998]
Declare Function GetTextWidth(ByVal DC as hDC, nText as String) as Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|1041]
Declare Function GetTextHeight(ByVal DC as hDC, nText as String) as Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|1048]
Declare Sub FF_Redraw(HWND_F as HWnd ) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|1055]
Declare Function GetSys_Bits() as Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\release\CODEGEN_SnowSword_UTILITY.inc|1064]

#include Once "yGDI.inc"
#include Once "Form\ClsForm.inc"
#include Once "ClsControl.inc"
#include Once "ClsVirtualCl.inc"

#include Once "TextBox\ClsTextBox.inc"

#include Once "CheckBox\ClsCheckBox.inc"

#include Once "Frame\ClsFrame.inc"

#include Once "Line\ClsLine.inc"

#include Once "Label\ClsLabel.inc"

#include Once "PopupMenu\ClsPopupMenu.inc"

#include Once "Timer\ClsTimer.inc"

#include Once "TopMenu\ClsTopMenu.inc"

#include Once "TreeView\ClsTreeView.inc"

#include Once "ListView\ClsListView.inc"


Type UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|23]
Length As UShort '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|24]
MaximumLength As UShort '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|25]
Buffer As PWSTR '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|26]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|27]
Type PLIST_ENTRY As LIST_ENTRY Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|38]
Type PEB64_LDR_DATA '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|40]
Reserved1(7) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|41]
Reserved2(2) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|42]
InMemoryOrderModuleList As LIST_ENTRY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|43]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|44]
Type PPEB64_LDR_DATA As PEB64_LDR_DATA Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|45]
Type LDR_DATA_TABLE_ENTRY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|47]
Reserved1(1) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|48]
InMemoryOrderLinks As LIST_ENTRY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|49]
Reserved2(1) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|50]
DllBase As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|51]
Reserved3(1) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|52]
FullDllName As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|53]
Reserved4(7) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|54]
Reserved5(2) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|55]
Union DUMMYUNIONNAME '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|56]
CheckSum As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|57]
Reserved6 As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|58]
End Union '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|59]
TimeDateStamp As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|60]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|61]
Type PLDR_DATA_TABLE_ENTRY As LDR_DATA_TABLE_ENTRY Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|62]
Type RTL_USER_PROCESS_PARAMETERS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|64]
Reserved1(15) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|65]
Reserved2(9) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|66]
ImagePathName As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|67]
CommandLine As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|68]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|69]
Type PRTL_USER_PROCESS_PARAMETERS As RTL_USER_PROCESS_PARAMETERS Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|70]
Type PEB64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|72]
Reserved1(1) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|73]
BeingDebugged As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|74]
Reserved2 As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|75]
Reserved3(1) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|76]
Ldr As PPEB64_LDR_DATA '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|77]
ProcessParameters As PRTL_USER_PROCESS_PARAMETERS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|78]
Reserved4(2) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|79]
AtlThunkSListPtr As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|80]
Reserved5 As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|81]
Reserved6 As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|82]
Reserved7 As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|83]
Reserved8 As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|84]
AtlThunkSListPtr32 As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|85]
Reserved9(44) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|86]
Reserved10(95) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|87]
PostProcessInitRoutine As Any Ptr'PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|88]
Reserved11(127) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|89]
Reserved12 As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|90]
SessionId As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|91]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|92]
Type PPEB64 As PEB64 Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|93]
Type OBJECT_ATTRIBUTES '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|95]
Length As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|96]
RootDirectory As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|97]
ObjectName As UNICODE_STRING Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|98]
Attributes As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|99]
SecurityDescriptor As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|100]
SecurityQualityOfService As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|101]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|102]
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|103]
Type CLIENT_ID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|105]
UniqueProcess As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|106]
UniqueThread As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|107]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|108]
Type PCLIENT_ID As CLIENT_ID Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|109]
Type POBJECT_ATTRIBUTES As OBJECT_ATTRIBUTES Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|110]
Type SYSTEM_THREAD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|112]
KernelTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|113]
UserTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|114]
CreateTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|115]
WaitTime As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|116]
StartAddress As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|117]
ClientID As CLIENT_ID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|118]
Priority As Long 'KPRIORITY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|119]
BasePriority As Long ' KPRIORITY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|120]
ContextSwitchCount As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|121]
ThreadState As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|122]
WaitReason As Long ' KWAIT_REASON '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|123]
#ifdef _WIN64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|124]
Reserved As ULong'Add '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|125]
#else '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|126]
#endif '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|127]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|128]
Type VM_COUNTERS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|130]
PeakVirtualSize As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|131]
VirtualSize As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|132]
PageFaultCount As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|133]
PeakWorkingSetSize As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|134]
WorkingSetSize As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|135]
QuotaPeakPagedPoolUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|136]
QuotaPagedPoolUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|137]
QuotaPeakNonPagedPoolUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|138]
QuotaNonPagedPoolUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|139]
PagefileUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|140]
PeakPagefileUsage As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|141]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|142]
Type SYSTEM_PROCESS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|144]
NextEntryDelta As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|145]
ThreadCount As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|146]
Reserved(0 To 5) As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|147]
CreateTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|148]
UserTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|149]
KernelTime As LARGE_INTEGER '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|150]
ProcessName As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|151]
BasePriority As Integer ' KPRIORITY '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|152]
ProcessId As UInteger ' HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|153]
InheritedFromProcessId As UInteger ' HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|154]
HandleCount As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|155]
SessionId As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|156]
PageDirectoryBase As ULONG_PTR '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|157]
VmCounters As VM_COUNTERS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|158]
PrivatePageCount As size_t '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|159]
IoCounters As IO_COUNTERS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|160]
Threads(0) As SYSTEM_THREAD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|161]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|162]
#define SYSTEM_CALL_OPENPROCESS &H7A '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|221]
#define SYSTEM_CALL_TERMINATEPROCESS &H2C '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|222]
Enum MEMORY_INFORMATION_CLASS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|2]
MemoryBasicInformation '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|3]
MemoryWorkingSetList '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|4]
MemorySectionName '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|5]
End Enum '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|6]
Type PMEMORY_BASIC_INFORMATION As MEMORY_BASIC_INFORMATION Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|8]
Type PVOID As Any Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|9]
#define PrintLog(nt_Status, Reason) MyPrintLog(nt_Status, Reason, __FUNCTION__, __LINE__) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|53]
#define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModModule.Inc|54]
Type TEB '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|9]
Reserved1(11) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|10]
ProcessEnvironmentBlock As PPEB64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|11]
Reserved2(398)As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|12]
Reserved3(1951) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|13]
TlsSlots(63) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|14]
Reserved4(7) As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|15]
Reserved5(25) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|16]
ReservedForOle As PVOID  ' Windows 2000 only '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|17]
Reserved6(3) As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|18]
TlsExpansionSlots As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|19]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|20]
Type PTEB As TEB Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|21]
#define STILL_ALIVE 259 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|61]
#ifdef PrintLog
#endif
#ifdef PrintLastError
#define PrintLastError MyPrintA "Error In Function [" & __FUNCTION__ & "] At Line " & __LINE__ & ":" & WinErrorMsg(GetLastError) & "(" & Str(GetLastError) & ")" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModThread.Inc|81]
#endif
Type PUNICODE_STRING As UNICODE_STRING Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|4]
Enum SHUTDOWN_ACTION '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|0]
ShutdownNoReboot '关闭系统 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|1]
ShutdownReboot '重启 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|2]
ShutdownPowerOff '关闭系统并关闭电源 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|3]
ShutdownForceReboot '强制重启 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|4]
End Enum '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModTools.Inc|5]
Type CallbackInfo '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|0]
TheType As Wstring * 20 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|1]
Func As ULONG64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|2]
TheContext As ULONG64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|3]
'DriverName As Wstring * 260 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|4]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|5]
Type ObCallbackInfo '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|7]
TheType As Wstring * 20 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|8]
ObHandle As ULONG64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|9]
Func As ULONG64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|10]
'DriverName As Wstring * 260 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|11]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc|12]
Type SYSTEM_HANDLE_TABLE_ENTRY_INFO '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|5]
UniqueProcessId As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|6]
ObjectTypeIndex As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|7]
HandleFlag As Byte '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|8]
HandleValue As UShort '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|9]
pObject As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|10]
GrantedAccess As ACCESS_MASK '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|11]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|12]
Type SYSTEM_HANDLE_INFORMATION '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|13]
NumberOfHandles As ULong '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|14]
Handles(0) As SYSTEM_HANDLE_TABLE_ENTRY_INFO '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|15]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|16]
Type OBJECT_NAME_INFORMATION '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|34]
Name As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|35]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|36]
Type OBJECT_TYPE_INFORMATION '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|39]
TypeName As UNICODE_STRING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|40]
' 其他字段... '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|41]
End Type'/ '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|42]
Type HANDLE_INFO '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|44]
dwProcessId As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|45]
dwHandle As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|46]
pObject As ULONG64 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|47]
strType As Wstring * 20 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|48]
strName As Wstring * 260 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|49]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|50]
Type THREAD_PARAM '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|53]
MyHandle As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|54]
MyEvent As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|55]
IsUnlockFile As BOOLEAN '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|56]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|57]
Enum OBJECT_INFORMATION_CLASS '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|63]
ObjectBasicInformation '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|64]
ObjectNameInformation '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|65]
ObjectTypeInformation '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|66]
End Enum '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|67]
#define LVM_SETITEMHEIGHT &H101B '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc|0]
Type FrmMain_Class_Form Extends Class_Form
    TxtFilter2 As Class_TextBox
    Check9 As Class_CheckBox
    Check8 As Class_CheckBox
    Check7 As Class_CheckBox
    Check6 As Class_CheckBox
    Check5 As Class_CheckBox
    Check4 As Class_CheckBox
    Check3 As Class_CheckBox
    Frame3 As Class_Frame
    Check2 As Class_CheckBox
    Check1 As Class_CheckBox
    Frame2 As Class_Frame
    Line1 As Class_Line
    Label1 As Class_Label
    FmeAdvance As Class_Frame
    mnuFile As Class_PopupMenu
    TmrProtect As Class_Timer
    TopMenu1 As Class_TopMenu
    treMain As Class_TreeView
    mnuProcess As Class_PopupMenu
    ListView1 As Class_ListView
    TreeView As Class_TreeView
   Declare Function Show(hWndParent As .hWnd =HWND_DESKTOP,Modal As Boolean = False ,UserData As Integer = 0) As .hWnd '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
End Type
Type FrmModule_Class_Form Extends Class_Form
    lvwModule As Class_ListView
   Declare Function Show(hWndParent As .hWnd =HWND_DESKTOP,Modal As Boolean = False ,UserData As Integer = 0) As .hWnd '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
End Type
Type FrmThread_Class_Form Extends Class_Form
    lvwThread As Class_ListView
   Declare Function Show(hWndParent As .hWnd =HWND_DESKTOP,Modal As Boolean = False ,UserData As Integer = 0) As .hWnd '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
End Type
Type FrmListView_Class_Form Extends Class_Form
    TxtFilter1 As Class_TextBox
    mnuUnlock_FIle As Class_PopupMenu
    mnuModule As Class_PopupMenu
    mnuThread As Class_PopupMenu
    ListView As Class_ListView
   Declare Function Show(hWndParent As .hWnd =HWND_DESKTOP,Modal As Boolean = False ,UserData As Integer = 0) As .hWnd '加载窗口并且显示, 模态显示用True {2.True.False} UserData 是传递给创建事件的参数，不保存数值
End Type
Type DO_SOMETHING '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|3]
ProcessId As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|4]
TheContext As PVOID '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|5]
End Type '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|6]
Dim Shared gFLY_FontNames() As String ,gFLY_FontHandles() As HFONT
Dim Shared hThread As Any Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|7]
CONST OBJ_KERNEL_HANDLE = &h00000200 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|220]
#ifdef PrintLog
#endif
#ifdef PrintLastError
#endif
Dim Shared pInlineHook As Any Ptr '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|0]
Dim Shared DenyInjectDll As Function() As Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|1]
Dim Shared AccessInjectDll As Function() As Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|2]
Dim Shared ShieldMsgHook As Function(ByVal As BOOLEAN) As BOOLEAN '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|3]
Dim Shared gResults() As HANDLE_INFO '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|70]
Dim Shared gMutexHandle As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|71]
Dim Shared gResultCount As Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|72]
Const MUTEX_NAME As String = "Global\SnowSword_QueryHandle_Mutex" '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|75]
Dim Shared ListViewData() As String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc|1]
Dim Shared FrmMain As FrmMain_Class_Form

#define FrmMain_mnuFile_mnuCreate 117
#define FrmMain_mnuFile_mnuCreateFile 118
#define FrmMain_mnuFile_mnuCreateFolder 119
#define FrmMain_mnuFile_mnuCopy 120
#define FrmMain_mnuFile_mnuPaste 121
#define FrmMain_mnuFile_mnuDelete 122
#define FrmMain_TopMenu1_mnuTools 125
#define FrmMain_TopMenu1_mnuUnlockFile 126
#define FrmMain_mnuProcess_mnuTerminateProcess 129
#define FrmMain_mnuProcess_mnuForceTerminateProcess 130
#define FrmMain_mnuProcess_mnuViolentTerminateProcess 131
#define FrmMain_mnuProcess_mnuSuspendProcess 132
#define FrmMain_mnuProcess_mnuResumeProcess 133
#define FrmMain_mnuProcess_mnuProtectProcess 134
#define FrmMain_mnuProcess_mnuCheckHideProcess 135
#define FrmMain_mnuProcess_mnuRefresh 136
#define FrmMain_mnuProcess_mnuStep1 137
#define FrmMain_mnuProcess_mnuViewModule 138
#define FrmMain_mnuProcess_mnuViewThread 139
#define FrmMain_mnuProcess_mnuViewMemory 140

Const MAX_LEN = 200 '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|4]
Dim Shared hook As HHOOK '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|5]
Dim Shared CurrentType As Integer = -1'None '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|6]
Dim Shared hDrv As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|7]
Dim Shared IsHaveSelectItem As BOOLEAN '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|8]
Dim Shared IsDriverLoaded As BOOLEAN = False '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|9]
Dim Shared PIDs() As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|10]
Dim Shared CurrentPID As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|12]
Dim Shared CurrentProcessName As String '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|13]
Dim Shared FrmModule As FrmModule_Class_Form


Dim Shared FrmThread As FrmThread_Class_Form


Dim Shared FrmListView As FrmListView_Class_Form

#define FrmListView_mnuUnlock_FIle_mnuRefresh 103
#define FrmListView_mnuUnlock_FIle_mnuCloseHandle 104
#define FrmListView_mnuUnlock_FIle_mnuForceCloseHandle 105
#define FrmListView_mnuUnlock_FIle_mnuTerminateProcess 106
#define mnuRefreshModule 108
#define mnuUnloadModule 109
#define mnuRefreshThread 111
#define mnuTerminateThread 112
#define mnuSuspendThread 113
#define mnuResumeThread 114

Dim Shared uData As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|8]
Dim Shared OriginalType As Integer '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|9]
Declare Function FF_PUMPHOOK( uMsg As Msg ) As Long '[FILE:#FF_PumpHook#|0]
Declare Function FF_WINMAIN(ByVal hInstance As HINSTANCE) As Long '[FILE:#FF_WinMain#|0]
Declare Sub FF_WINEND(ByVal hInstance As HINSTANCE) '[FILE:#FF_WinMain#|18]
Extern "C"
End Extern
Declare Function ForceKillProcess(ByVal dwProcessId As DWORD) As BOOLEAN '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|259]
Declare Function GetProcessNameByPath(ByVal ProcessPath As StringW) As StringW '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|339]
Declare Sub ThreadProc(ByVal UserData As Any Ptr) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModProcess.Inc|618]
Extern "C"
End Extern
Extern "C"
End Extern
#ifdef PrintLog
#endif
#ifdef PrintLastError
#endif
Extern "C"
End Extern
Declare Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModKernelModule.Inc|82]
Declare Sub InitSelfProtect() '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|5]
Declare Sub UninitSelfProtect() '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc|14]
Declare Function OpenDrv(ByVal szDrvLinkName As String) As HANDLE '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|6]
Declare Function IoControl(ByVal hDrvHandle As HANDLE, ByVal dwIoControlCode As DWORD, ByVal lpInBuffer As LPVOID, ByVal nInBufferSize As DWORD, ByVal lpOutBuffer As LPVOID, ByVal nOutBufferSize As DWORD, ByVal lpBytesReturned As LPDWORD = NULL) As WINBOOL '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|13]
Declare Sub CloseDrv(ByVal hDrvHandle As HANDLE) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|20]
Declare Function CTL_CODE_GEN(ByVal lngFunction As Long) As Long '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLoadDriver.Inc|24]
Declare Sub InitializeGlobals() '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|77]
Declare Function QueryHandleInfoThread(ByVal lpParam As LPVOID) As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|91]
Declare Function QueryHandleInfoThread1(ByVal lpParam As LPVOID) As DWORD '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|184]
Declare Sub UnlockOccupyFile(ByVal wstrFilePath As LPWSTR) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|265]
Declare Sub UnlockOccupyFile1(ByVal wstrFilePath As LPWSTR) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc|358]
Declare Sub SetListViewItemBackColor(ListView As Class_ListView, ByVal ItemIndex As Integer, ByVal ItemColor As COLORREF) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc|45]
Declare Function GetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long) As BOOLEAN '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc|50]
Declare Sub SetMenuCheckState(Menu As Class_Menu, ByVal iditem As Long, ByVal state As BOOLEAN) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModListView.Inc|55]
Declare Function FrmMain_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|-385]
Declare Function FrmMain_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|-74]
Declare Sub FrmMain_Shown(hWndForm As hWnd, UserData As Integer) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|27]
Declare Sub FrmMain_ListView1_WM_ContextMenu(hWndForm As hWnd, hWndControl As hWnd, xPos As Long, yPos As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|62]
Declare Sub FrmMain_mnuProcess_WM_Command(hWndForm As hWnd, wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|75]
Declare Sub FrmMain_treMain_WM_LButtonDblclk(hWndForm As hWnd,hWndControl As hWnd,MouseFlags As Long,xPos As Long,yPos As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|154]
Declare Sub FrmMain_TmrProtect_WM_Timer(hWndForm As hWnd,wTimerID As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|205]
Declare Function FrmMain_WM_Close(hWndForm As hWnd) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|222]
Declare Sub DeleteChildItem(MyTreeView As Class_TreeView, Node As HTREEITEM) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|232]
Declare Function FrmMain_TreeView_NM_DBLCLK(hWndForm As hWnd,hWndControl As hWnd) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|247]
Declare Sub FrmMain_mnuFile_WM_Command(hWndForm As hWnd,wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|263]
Declare Function FrmMain_TreeView_NM_RCLICK(hWndForm As hWnd, hWndControl As hWnd) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|316]
Declare Sub FrmMain_Check3_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|328]
Declare Sub FrmMain_ListView1_WM_LButtonDblclk(hWndForm As hWnd, hWndControl As hWnd, MouseFlags As Long, xPos As Long, yPos As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|345]
Declare Sub FrmMain_Check6_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|358]
Declare Sub FrmMain_Check5_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|392]
Declare Sub FrmMain_Check7_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|405]
Declare Sub FrmMain_Check2_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|418]
Declare Sub FrmMain_Check8_BN_Clicked(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|425]
Declare Sub FrmMain_TopMenu1_WM_Command(hWndForm As hWnd,wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|446]
Declare Sub FrmMain_TxtFilter2_EN_Change(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|460]
Declare Function FrmMain_ListView1_OwnerDraw(hWndForm As hWnd, hWndControl As hWnd,lpdis As DRAWITEMSTRUCT) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmMain.frm|471]
Declare Function FrmModule_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmModule.Frm|-278]
Declare Function FrmModule_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmModule.Frm|-60]
Declare Function FrmThread_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmThread.Frm|-282]
Declare Function FrmThread_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmThread.Frm|-60]
Declare Function FrmListView_FORMPROCEDURE(ByVal hWndForm As hWnd, ByVal wMsg As UInteger, ByVal wParam As wParam, ByVal lParam As lParam) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|-326]
Declare Function FrmListView_CODEPROCEDURE(ByVal hWndControl As HWND, ByVal wMsg As uInteger, ByVal wParam As WPARAM, ByVal lParam As LPARAM) As LRESULT '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|-65]
Declare Sub FrmListView_Shown(hWndForm As hWnd,UserData As Integer) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|13]
Declare Sub FrmListView_ListView_WM_ContextMenu(hWndForm As hWnd,hWndControl As hWnd,xPos As Long,yPos As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|39]
Declare Sub FrmListView_mnuThread_WM_Command(hWndForm As hWnd,wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|60]
Declare Sub FrmListView_mnuModule_WM_Command(hWndForm As hWnd,wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|94]
Declare Function FrmListView_WM_Close(hWndForm As hWnd) As LResult '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|108]
Declare Sub FrmListView_WM_NcActivate(hWndForm As hWnd, fActive As Long) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|117]
Declare Sub FrmListView_mnuUnlock_FIle_WM_Command(hWndForm As hWnd,wID As ULong) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|132]
Declare Sub FrmListView_TxtFilter1_EN_Change(hWndForm As hWnd, hWndControl As hWnd) '[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\forms\FrmListView.Frm|160]
