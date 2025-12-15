'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModPrivileges.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/winbase.bi" '已经搬到定义文件处

Public Function AdjustPrivilege(ByVal Privilege As String = "SeDebugPrivilege") As WINBOOL
    Dim ret As WINBOOL
    Dim Token As TOKEN_PRIVILEGES
    Dim hProc As HANDLE, hToken As HANDLE
    
    hProc = GetCurrentProcess
    OpenProcessToken hProc, TOKEN_ADJUST_PRIVILEGES Or TOKEN_QUERY, @hToken
    Token.PrivilegeCount = 1
    Token.Privileges(0).Attributes = SE_PRIVILEGE_ENABLED
    ret = LookupPrivilegeValue("", Privilege, @Token.Privileges(0).LUID)
    ret = AdjustTokenPrivileges(hToken, False, @Token, ByVal 0&, ByVal 0&, ByVal 0& )
    If ret = 1 Then CloseHandle hToken
    Return ret
End Function
