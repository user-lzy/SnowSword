'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


#define UNICODE
#include Once "Afx/CGdiPlus/CGdiPlus.inc"
#include Once "windows.bi"    'WIN系统基础库，主要是WIN系统常用的API，不使用API可以不包含它。
#include Once "win/shlobj.bi" 'WIN系统对象库，shell32.dll的头文件，主要涉及shell及通用对话框等。
#lang "FB" '表示为标准FB格式

Type StringW '宽字符变长字符串类型
   WStrPtr As Wstring Ptr '字符内存指针
   WstrLen As ULong       '字符个数（不是字节，字节=个数*2）
   Declare Constructor '声明  Dim a As StringW
   Declare Destructor  '销毁时要释放内存
   Declare Constructor(wszStr As Wstring) '声明+赋值  Dim a As StringW ="xx"
   Declare Operator Cast() ByRef As Wstring '返回W字符，默认用途（可惜FB内置字符处理函数先转换A字符处理）
   Declare Operator Cast() As Wstring Ptr '返回W指针，主要兼容系统API调用，因为API用的是指针
   Declare Function WStr() ByRef As Wstring '返回给一些不支持Cast返回宽字符的，强制输出宽字符（主要是FB内置字符处理函数）
   Declare Operator Let(wszStr As Wstring) '赋值
   Declare Operator Let(wszStr As StringW) '同类赋值，没这个会发生直接复制成员值，而不是拷贝字符
   Declare Operator [](nIndex As ULong) ByRef As UShort '直接使用指针方式获取或修改字符编码。
   bug As Integer = 1460339748 '解决FB编译器BUG用，当作为函数返回时，当函数里没执行返回代码，将会返回一个没初始化的类，即 WStrPtr 内容是随机的，并不是0
End Type
Constructor StringW
   WStrPtr = CAllocate(2) '初始化时，分配
End Constructor
Destructor StringW
   If WStrPtr Then Deallocate WStrPtr '销毁类时释放内存
   bug = 0 '避免内存残留数字，造成新创建的类刚好碰撞上
End Destructor
Constructor StringW(wszStr As Wstring)
   This = wszStr '变量声明时赋值
End Constructor
Operator StringW.Cast() ByRef As Wstring
   If WStrPtr = 0 Or bug <> 1460339748 Then WStrPtr = CAllocate(2)  '解决FB编译器的BUG，假如没初始化类而内存刚好是1460339748，那么就是天意，软件崩溃吧。
   Return *WStrPtr
End Operator
Operator StringW.Cast() As Wstring Ptr
   Return WStrPtr '
End Operator
Operator StringW.Let(wszStr As Wstring) '变量赋值时的默认函数
   WstrLen = Len(wszStr) '字符个数
   If WStrPtr Then Deallocate WStrPtr '释放内存
   WStrPtr = Allocate(WstrLen * 2 + 2) '申请内存，内存不清零（速度快）
    *WStrPtr = wszStr
End Operator
Operator StringW.Let(wszStr As StringW) '变量赋值时的默认函数
   If wszStr.WstrPtr Then This = *wszStr.WstrPtr
End Operator
Function StringW.WStr() ByRef As WString
   If WStrPtr = 0 Or bug <> 1460339748 Then WStrPtr = CAllocate(2)  '解决FB编译器的BUG，假如没初始化类而内存刚好是1460339748，那么就是天意，软件崩溃吧。
   Return *WStrPtr
End Function
Operator StringW.[](nIndex As ULong) ByRef As UShort
   Static Zero As UShort = 0
   If nIndex > WstrLen - 1 Then Return Zero
   Operator = *Cast(UShort Ptr, WstrPtr + nIndex)
End Operator
Operator & (lhs As StringW, rhs As StringW) As StringW '必须 重载一下 & 才可以  a & b & c 的3个以上连续拼接
   Return lhs.WStr & rhs.WStr
End Operator
Operator & (lhs As StringW, rhs As Wstring) As StringW '和其它类型拼贴，主要是兼容 CWSTR 混合拼贴字符，毕竟很多 AFX函数返回是 CWSTR ，不用此就编译出错误
   Return lhs.WStr & rhs
End Operator
Operator & (lhs As Wstring, rhs As StringW) As StringW '和其它类型拼贴，主要是兼容 CWSTR 混合拼贴字符，毕竟很多 AFX函数返回是 CWSTR ，不用此就编译出错误
   Return lhs & rhs.WStr
End Operator
Operator Len(lhs As StringW) As Integer '重载一下才能正常使用
   Return lhs.WstrLen
End Operator
Function Val Overload(lhs As StringW) As Double '重载一下才能正常使用
   Return .Val(lhs.WStr)
End Function
Function ValInt Overload(lhs As StringW) As Long '重载一下才能正常使用
   Return .ValInt(lhs.WStr)
End Function
Function ValLng Overload(lhs As StringW) As LongInt '重载一下才能正常使用
   Return .ValLng(lhs.WStr)
End Function
Function ValUInt Overload(lhs As StringW) As ULong '重载一下才能正常使用
   Return .ValUInt(lhs.WStr)
End Function
Function ValULng Overload(lhs As StringW) As ULongInt '重载一下才能正常使用
   Return .ValULng(lhs.WStr)
End Function


Type FormControlsPro_TYPE '保存控件的私有属性，每个控件都不同，全部在此，各取所需。
   hWndParent     As hWnd     '父窗口句柄
   nName          As String   '名称\1\用来代码中识别对象的名称\Command\
   Index          As Long     '控件数组索引，小于零表示非控件数组
   IDC            As Long     '
   CtrlFocus      As hWnd     ' 当前焦点在什么控件
   BigIcon        As HICON    '窗口大图标，主要用来主窗口
   SmallIcon      As HICON    '窗口小图标
   nText          As StringW  '
   ControlType    As Long     '控件类型，为了画控件  >=100 为虚拟控件  100=LABEL   TEXT=1 Button=2
   nData          As String   '编写控件使用，数据存放，无特定，控件根据自己需要存放任意数据。
   CtlData(99)    As Integer  '为每个控件提供 100 个数据储存(编写控件使用，控件根据自己需要存放任意数据)。
   UserData(99)   As Integer  '为每个控件提供 100 个数据储存(用户使用)。
   Style          As UInteger '样式，每个控件都有自己的定义
   TransPer       As Long     '透明度\0\窗口透明度，百分比(0--100)，0%不透明 100%全透明\0\
   TransColor     As Long = &H197F7F7F '透明颜色\3\透明颜色，需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
   MousePointer   As Long '指针\2\鼠标在窗口上的形状\0 - 默认,1 - 后台运行,2 - 标准箭头,3 - 十字光标,4 - 箭头和问号,5 - 文本工字光标,6 - 不可用禁止圈,7 - 移动,8 - 双箭头↙↗,9 - 双箭头↑↓,10 - 双箭头向↖↘,11 - 双箭头←→,12 - 垂直箭头,13 - 沙漏,14 - 手型
   ForeColor      As Long = &H197F7F7F '文字色 需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
   BackColor      As Long = &H197F7F7F '背景色 需要用 GetCodeColorGDI 或 GetCodeColorGDIplue 转为 GDI 和 GDI+ 颜色值
   hBackBrush     As HBRUSH  '背景刷子，返回给窗口和控件用
   nFont          As String  '字体\4\用于此对象的文本字体。\微软雅黑,9,0\
   Tag            As StringW ' 附加 \ 1 \ 私有自定义文本与控件关联。 \ \
   ToolTip        As StringW ' 提示 \ 1 \ 一个提示，当鼠标光标悬停在控件时显示它。 \ \
   ToolTipBalloon As Long    ' 气球样式 \ 2 \ 一个气球样式显示工具提示。 \ False \ True, False
   ToolWnd        As hWnd    '提示窗口句柄，用来销毁
   nCursor        As HCURSOR '鼠标指针句柄
   nLeft          As Long    '返回/设置相对于父窗口的 X 响应DPI数值，为 100%DPI数值（像素）
   nTop           As Long    '返回/设置相对于父窗口的 Y 响应DPI数值，为 100%DPI数值（像素）
   nWidth         As Long    '返回/设置控件宽度 响应DPI数值，为 100%DPI数值（像素）
   nHeight        As Long    '返回/设置控件高度 响应DPI数值，为 100%DPI数值（像素））
   anchor         AS LONG    '控件布局 自动调整方式
   nRight         As Long    '控件布局 用，相对于窗口右边距离 响应DPI数值，为 100%DPI数值（像素））
   nBottom        As Long    '控件布局 用，相对于窗口底边距离 响应DPI数值，为 100%DPI数值（像素））
   centerX        AS LONG    '控件布局 用 中点  响应DPI数值，为 100%DPI数值（像素）
   centerY        AS LONG    '控件布局 用
   VrControls     As FormControlsPro_TYPE Ptr '如果是主窗口带 虚拟控件 ，就是下一个虚拟控件链表指针，直到为 0 表示没了
End Type

'Type vfb_Control_Ptr_Save_type
'   hW As hWnd   '窗口句柄，对应窗口用
'   vp As FormControlsPro_TYPE Ptr '为窗口保存的指针
'End Type

'Dim Shared vfb_Control_Ptr_Save() As vfb_Control_Ptr_Save_type '控件数据指针保存处
Function vfb_Get_Control_Ptr(hWndControl As hWnd) As FormControlsPro_TYPE Ptr '获取控件数据指针
   Return Cast(FormControlsPro_TYPE Ptr, GetPropW(hWndControl, "vfb_Control_Ptr"))
   'If vfb_Control_Ptr_Save = 0 Then Return 0
   'Dim tsave As FormControlsPro_TYPE Ptr = vfb_Control_Ptr_Save
   'Dim ii    As Long
   'Do
   '   tsave->hWndParent =
   '      If tsave[ii] = hWndControl Then Return tsave[ii + 1]
   '   Next
   '   if tsave[201] = 0 Then Return 0  '没有下一个指针组
   '   tsave = tsave[201]
   'Loop
End Function
Sub vfb_Set_Control_Ptr(hWndControl As hWnd, pp As FormControlsPro_TYPE Ptr) ' 设置控件数据指针
   SetPropW hWndControl, "vfb_Control_Ptr", Cast(Any Ptr, pp)
   '参考 API GetProp SetProp RemoveProp
   'Dim u As Long = UBound(vfb_Control_Ptr_Save)
   'If u = -1 Then ReDim vfb_Control_Ptr_Save(99)
   'Dim tsave As FormControlsPro_TYPE Ptr = vfb_Control_Ptr_Save
   'Dim i     As Long
   ''先寻找是不是重复窗口，有重复就更新
   'u = UBound(vfb_Control_Ptr_Save)
   'For i = 0 To u
   '   If vfb_Control_Ptr_Save(i).hW = hWndControl Then
   '      vfb_Control_Ptr_Save(i).vp = pp
   '      Return
   '   End If
   'Next
   ''寻找空位置来保存窗口数据指针
   'For i = 0 To u
   '   If vfb_Control_Ptr_Save(i).hW = 0 Then
   '      vfb_Control_Ptr_Save(i).hW = hWndControl
   '      vfb_Control_Ptr_Save(i).vp = pp
   '      Return
   '   End If
   'Next
   '
   'Do
   '   For ii = 0 To 198 Step 2
   '      if tsave[ii] = 0 Then
   '         tsave[ii] = hWndControl
   '         tsave[ii + 1] = pp
   '         Return
   '      End if
   '   Next
   '   If tsave[201] = 0 Then '没有下一组，就创建一个
   '      tsave[200] = tsave
   '      tsave[201] = CAllocate(SizeOf(Integer) * 202)
   '   End if
   '   tsave = tsave[201]
   'Loop
End Sub
Sub vfb_Remove_Control_Ptr(hWndControl As Any Ptr) '删除数据指针
   RemoveProp hWndControl, "vfb_Control_Ptr"
   'If vfb_Control_Ptr_Save = 0 Then Return
   'Dim tsave As Any Ptr Ptr = vfb_Control_Ptr_Save
   'Dim ii    As Long
   'Do
   '   For ii = 0 To 198 Step 2
   '      if tsave[ii] = hWndControl Then '清除数据，为别的控件可以保存数据
   '         tsave[ii] = 0
   '         tsave[ii + 1] = 0
   '         Return
   '      End If
   '   Next
   '   if tsave[201] = 0 Then Return '没有下一个指针组
   '   tsave = tsave[201]
   'Loop
End Sub


'[START_APPSTART]
'************ 应用程序起始模块 ************
' 这里是打开软件后最初被执行代码的地方，此时软件内部还未初始化。（注：一般情况EXE包含DLL的，DLL先于EXE执行DLL自己的起始代码）
' 不管是EXE还是DLL，都从这里开始执行，然后到【程序入口函数】执行，整个软件结束或DLL被卸载就执行【程序出口】过程。(这里的EXE、DLL表示自己程序)
' 一般情况在这里写 DLL 声明、自定义声明、常量和#Include的包含文件。由于很多初始化代码未执行，这里不建议写用户代码。

#define UNICODE                 '表示WIN的API默认使用 W系列，宽字符处理，如果删除（使用ASCII字符）会造成控件和API使用的代码编写方式，影响深远。
#lang "FB"                      '表示为标准FB格式
#include Once "windows.bi"      'WIN系统基础库，主要是WIN系统常用的API，不使用API可以不包含它。
#include Once "win/shlobj.bi"   'WIN系统对象库，shell32.dll的头文件，主要涉及shell及通用对话框等。
#include Once "afx/CWindow.inc" 'WinFBX 库，是WIN系统增强基础库，使用窗口和控件必须要用到它。
#include Once "vbcompat.bi"     '一些兼容VB的语句和常数，不包含就会无法使用它们了。
#include Once "fbthread.bi"     'VisualFreeBasic线程语句支持库，要用线程语句，就必须包含。

'以上 包含文件 会影响最终编译生成文件的大小，而不会影响运行效率，对于文件大小比较敏感的，可以根据需要调整。

'当你打开这个模块的同时，证明你已经对 VisualFreeBasic 有所了解，即将进入高手阶段。
'此时你需要了解VFB的工作过程。
'VFB 代码执行流程：
'系统加载EXE -->【起始模块】-->【入口函数】-->【启动窗口】-->关闭启动窗口后-->【出口函数】--> 进程退出 
'
'为了简化编写代码，VisualFreeBasic 对代码进行加工，但不包括【起始模块】，就是说【起始模块】不处理任何东西直接去编译的。
'因此本模块不要写用户代码，一般写定义和引用。
'
'代码进行加工过程如下：
'1，将窗口设计，转换为WIn系统SDK模式的纯代码，包括事件和消息流程处理等等非常复杂的操作。
'2，把中文代码替换成英文代码后编译，让FreeBasic支持中文代码（前提：VFB选项中选择了这个功能，默认选择的）
'3，提取函数名，类、全局变量等的定义设置放到单独文件中引用。让你写代码无需考虑函数先后顺序问题，以及自己写定义。
'4，自动检查代码调用了【源码库】和【我的代码库】，把相关代码全部放入单独文件中引用。让我们可以直接使用源码库，而无需自己去复制源码。
'5，当你工程属性选择多国语言，将支持多国语言函数。多国语言用法见相关例题。
'6，当工程中包含其它模块时，自动帮你引用。
'加工过程产生临时文件，在工程属性里设置是否自动删除。

'[END_APPSTART]



' 程序员可以通过共享APP变量访问的公共信息。
Type APP_TYPE
   Comments        As Wstring * 100 ' 注释
   CompanyName     As Wstring * 100 ' 公司名
   EXEName         As Wstring * 100 ' 程序的EXE名称
   FileDescription As Wstring * 100 ' 文件描述
   HINSTANCE       As HINSTANCE ' 程序的实例句柄
   Path            As Wstring * 260 ' EXE的当前路径
   ProductName     As Wstring * 100 ' 产品名称
   LegalCopyright  As Wstring * 100 ' 版权所有
   LegalTrademarks As Wstring * 100 ' 商标
   ProductMajor    As Long    ' 产品主要编号
   ProductMinor    As Long    ' 产品次要编号
   ProductRevision As Long    ' 产品修订号
   ProductBuild    As Long    ' 产品内部编号
   FileMajor       As Long    ' 文件主要编号
   FileMinor       As Long    ' 文件次要编号
   FileRevision    As Long    ' 文件修订号
   FileBuild       As Long    ' 文件内部编号
   ReturnValue     As Integer ' 返回的用户值
End Type


Dim Shared App As APP_TYPE
Sub Setting_up_Application_Common_Information()
   '设置共享应用程序变量的值
   #if __FB_OUT_EXE__
      App.hInstance = GetModuleHandle(null)
   #else
      Dim mbi as MEMORY_BASIC_INFORMATION
      VirtualQuery(@Setting_up_Application_Common_Information, @mbi, SizeOf(mbi))
      App.hInstance = mbi.AllocationBase
   #endif
   Dim zTemp As WString * MAX_PATH
   Dim x     As Long
   App.CompanyName     = WStr("")
   App.FileDescription = WStr("")
   App.ProductName     = WStr("")
   App.LegalCopyright  = WStr("")
   App.LegalTrademarks = WStr("")
   App.Comments        = WStr("")
   
   App.ProductMajor    = 1
   App.ProductMinor    = 0
   App.ProductRevision = 0
   App.ProductBuild    = 516
   
   App.FileMajor    = 1
   App.FileMinor    = 0
   App.FileRevision = 0
   App.FileBuild    = 944
   
   'App.hInstance 在WinMain / LibMain中设置
   
   '检索程序完整路径和 EXE/DLL 名称
   GetModuleFileNameW App.hInstance, zTemp, MAX_PATH
   x = InStrRev(zTemp, Any ":/\")
   If x Then
      App.Path    = Left(zTemp, x)
      App.EXEname = Mid(zTemp, x + 1)
   Else
      App.Path    = ""
      App.EXEname = zTemp
   End If
End Sub
Setting_up_Application_Common_Information
' 声明/等同 项目中的所有函数，表单和控件
#Include Once "CODEGEN_SnowSword_DECLARES.inc"
#Include Once "CODEGEN_SnowSword_UTILITY.inc"
#Include Once "CODEGEN_SnowSword_ModProcess_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModPrivileges_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModModule_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModThread_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModKernelModule_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModStringFunc_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModFile_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModSelfProtect_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModLoadDriver_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModReg_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModWindow_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModLowFile_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModTools_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModCallbacks_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModHandle_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModLog_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModListView_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModCheckApiHook_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModMemory_MODULE.inc"
#Include Once "CODEGEN_SnowSword_ModData_MODULE.inc"
#Include Once "CODEGEN_SnowSword_FrmMain_FORM.inc"
#Include Once "CODEGEN_SnowSword_FrmModule_FORM.inc"
#Include Once "CODEGEN_SnowSword_FrmThread_FORM.inc"
#Include Once "CODEGEN_SnowSword_FrmListView_FORM.inc"


'[START_WINMAIN]
Function FF_WINMAIN(ByVal hInstance As HINSTANCE) As Long '程序入口函数
   'hInstance EXE或DLL的模块句柄，就是在内存中的地址，EXE 通常固定为 &H400000  DLL 一般不固定 
   '编译为 LIB静态库时，这里是无任何用处 
   ' -------------------------------------------------------------------------------------------
   '  DLL 例题 ********  函数无需返回值
   '  DLL被加载到内存时，不要执行太耗时间的代码，若需要耗时就用多线程。
   '        AfxMsg "DLL被加载到内存时"
   ' -------------------------------------------------------------------------------------------
   '  EXE 例题 ********   
   '        AfxMsg "EXE刚启动"
   ' 如果这个函数返回TRUE（非零），将会结束该软件。如果没有启动窗口，那么此函数过后，也会终止软件。
   ' 您可以在此函数做程序初始化。
   ' -------------------------------------------------------------------------------------------
   ' (这里的EXE、DLL表示自己程序，无法获取其它EXE、DLL入口和出口)

   Function = False   
End Function

Sub FF_WINEND(ByVal hInstance As HINSTANCE) '程序出口，程序终止后的最后代码。
   'hInstance EXE或DLL的模块句柄，就是在内存中的地址，EXE 通常固定为 &H400000  DLL 一般不固定 
   '编译为 LIB静态库时，这里是无任何用处 
   ' -------------------------------------------------------------------------------------------
   '  DLL 例题 ********    
   '    卸载DLL，DLL被卸载，需要快速完成，不能用进程锁。
   '    AfxMsg "DLL被卸载时" 
   ' -------------------------------------------------------------------------------------------
   '  EXE 例题 ********   
   '   程序即将结束，这里是最后要执行的代码，（：无法停止被退出的命运。
   '      AfxMsg "EXE退出"
   ' -------------------------------------------------------------------------------------------
   ' (这里的EXE、DLL表示自己程序，无法获取其它EXE、DLL入口和出口)

End Sub



'[END_WINMAIN]


'[START_PUMPHOOK]
Function FF_PUMPHOOK( uMsg As Msg ) As Long '消息钩子
   '所有窗口消息都经过这里，你可以在这里拦截消息。

   ' 如果这个函数返回 FALSE（零），那么 VisualFreeBasic 消息泵将继续进行。
   ' 返回 TRUE（非零）将绕过消息泵（屏蔽消息），就是吃掉这消息不给窗口和控件处理。
   ' 

   Function = False    '如果你需要屏蔽消息，返回 TRUE 。

End Function



'[END_PUMPHOOK]


Function FLY_Win_Main(ByVal hInstance As HINSTANCE) As Long
   
      Dim gdipToken As ULONG_PTR
   Dim gdipsi As GdiplusStartupInput
   gdipsi.GdiplusVersion = 1
   GdiplusStartup( @gdipToken, @gdipsi, Null )
   
   ' 调用 FLY_WinMain()函数。 如果该函数返回True，则停止执行该程序。
   If FF_WINMAIN(hInstance) Then Return True
   ' 创建启动窗体。
   #if __FB_OUT_EXE__
      FrmMain.Show 0, TRUE
      GdiplusShutdown( gdipToken )
   #endif
   Function = 0
End Function
Public Sub WinMainsexit() Destructor
   FF_WINEND(App.hInstance)
End Sub
FLY_Win_Main(App.hInstance)




