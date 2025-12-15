'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCallbacks.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' Type CallbackInfo '已经搬到定义文件处
'     TheType As Wstring * 20 '已经搬到定义文件处
'     Func As ULONG64 '已经搬到定义文件处
'     TheContext As ULONG64 '已经搬到定义文件处
'     'DriverName As Wstring * 260 '已经搬到定义文件处
' End Type '已经搬到定义文件处

' Type ObCallbackInfo '已经搬到定义文件处
'     TheType As Wstring * 20 '已经搬到定义文件处
'     ObHandle As ULONG64 '已经搬到定义文件处
'     Func As ULONG64 '已经搬到定义文件处
'     'DriverName As Wstring * 260 '已经搬到定义文件处
' End Type '已经搬到定义文件处

Public Sub GetCallbackList(lvwCallbacks As Class_ListView)
    If (Not IsDriverLoaded) Then Exit Sub
    Dim Callbacks(63) As CallbackInfo
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    IoControl hDrv, CTL_CODE_GEN(&H850), NULL, 0, @Callbacks(0), 64 * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (Callbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(Callbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(Callbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 3, GetDriverPathByAddr(Callbacks(i).Func)'Callbacks(i).DriverName
        End If
    Next
    memset @Callbacks(0), 0, 64 * SizeOf(CallbackInfo)
    IoControl hDrv, CTL_CODE_GEN(&H851), NULL, 0, @Callbacks(0), 64 * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (Callbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(Callbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(Callbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 3, GetDriverPathByAddr(Callbacks(i).Func)
        End If
    Next
    memset @Callbacks(0), 0, 64 * SizeOf(CallbackInfo)
    IoControl hDrv, CTL_CODE_GEN(&H852), NULL, 0, @Callbacks(0), 64 * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (Callbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(Callbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(Callbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 3, GetDriverPathByAddr(Callbacks(i).Func)
        End If
    Next
    memset @Callbacks(0), 0, 64 * SizeOf(CallbackInfo)
    IoControl hDrv, CTL_CODE_GEN(&H853), NULL, 0, @Callbacks(0), 64 * SizeOf(CallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (Callbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(Callbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(Callbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 2, "0x" & Hex(Callbacks(i).TheContext)
            lvwCallbacks.SetItemText NewIndex, 3, GetDriverPathByAddr(Callbacks(i).Func)
        End If
    Next
    Dim ObCallbacks(63) As ObCallbackInfo
    memset @ObCallbacks(0), 0, 64 * SizeOf(ObCallbacks)
    IoControl hDrv, CTL_CODE_GEN(&H854), NULL, 0, @ObCallbacks(0), 64 * SizeOf(ObCallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (ObCallbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(ObCallbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(ObCallbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 2, "0x" & Hex(ObCallbacks(i).ObHandle)
        End If
    Next
    memset @ObCallbacks(0), 0, 64 * SizeOf(ObCallbacks)
    IoControl hDrv, CTL_CODE_GEN(&H855), NULL, 0, @ObCallbacks(0), 64 * SizeOf(ObCallbackInfo), @lpRet
    For i As Integer = 0 To 63
        If (ObCallbacks(i).Func <> 0) Then
            NewIndex = lvwCallbacks.AddItem(ObCallbacks(i).TheType)
            lvwCallbacks.SetItemText NewIndex, 1, "0x" & Hex(ObCallbacks(i).Func)
            lvwCallbacks.SetItemText NewIndex, 2, "0x" & Hex(ObCallbacks(i).ObHandle)
        End If
    Next
End Sub