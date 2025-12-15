'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModSelfProtect.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' Dim Shared pInlineHook As Any Ptr '已经搬到定义文件处
' Dim Shared DenyInjectDll As Function() As Long '已经搬到定义文件处
' Dim Shared AccessInjectDll As Function() As Long '已经搬到定义文件处
' Dim Shared ShieldMsgHook As Function(ByVal As BOOLEAN) As BOOLEAN '已经搬到定义文件处

Sub InitSelfProtect()
    pInlineHook = LoadLibraryEx(App.Path & "InlineHook.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    DenyInjectDll = Cast(Function() As Long, GetProcAddress(pInlineHook, "DenyInjectDll"))
    AccessInjectDll = Cast(Function() As Long, GetProcAddress(pInlineHook, "AccessInjectDll"))
    ShieldMsgHook = Cast(Function(ByVal As BOOLEAN) As BOOLEAN, GetProcAddress(pInlineHook, "ShieldMsgHook"))
    AfxMsg (DenyInjectDll=0)
    AfxMsg ShieldMsgHook(True)
End Sub

Sub UninitSelfProtect()
    AccessInjectDll()
    ShieldMsgHook(False)
    FreeLibrary pInlineHook
End Sub
