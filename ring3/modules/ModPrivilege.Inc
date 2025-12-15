#include Once "win/winbase.bi"

' 常量定义
Const TOKEN_QUERY = &h0008

' 特权属性标志
Const SE_PRIVILEGE_ENABLED = &h00000002
Const SE_PRIVILEGE_ENABLED_BY_DEFAULT = &h00000001
Const SE_PRIVILEGE_USED_FOR_ACCESS = &h80000000

' 返回类型
Type PRIVILEGE_INFO
    lValue As DWORD          ' 特权LUID低32位
    sName As Wstring * 50          ' 特权名(如SeDebugPrivilege)
    sDisplay As Wstring * 50       ' 显示名(如调试程序)
    Attributes As DWORD      ' 状态属性
    sStatus As Wstring * 15        ' 状态文本
End Type

Function IsAdmin() As Boolean
/'    Dim hToken As HANDLE, tp As TOKEN_PRIVILEGES, dwRetLen As DWORD

    If OpenProcessToken(GetCurrentProcess, TOKEN_QUERY, @hToken) Then
        If GetTokenInformation(hToken, TokenStatistics, @tp, SizeOf(TOKEN_PRIVILEGES), @dwRetLen) Then IsAdmin = tp.PrivilegeCount
        CloseHandle hToken
    End If
    
    Print IsAdmin为什么会炸? '/
    Dim hToken As HANDLE
    Dim elevationType As DWORD
    Dim dwSize As DWORD

    ' 获取当前进程的访问令牌
    If OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, @hToken) Then
        ' 获取访问令牌的提升类型信息
        If GetTokenInformation(hToken, TokenElevationType, @elevationType, SizeOf(DWORD), @dwSize) Then
            ' 判断提升类型
            If elevationType = TokenElevationTypeFull Then
                CloseHandle hToken
                Return True
            End If
        End If
        CloseHandle hToken
    End If
    Return False
End Function

Sub RestartasAdmin(ByVal IsLoadDriver As BOOLEAN = False)
    If IsAdmin Then
        AfxMsg "当前进程已获得管理员(UAC)权限!", "提示"
        Exit Sub
    End If
    
    Dim sei As SHELLEXECUTEINFO, strExePath As WString * MAX_PATH = App.Path & App.EXEName
    ' 初始化 SHELLEXECUTEINFO 结构
    sei.cbSize = SizeOf(sei)
    sei.lpVerb = StrPtrW("runas") ' 请求管理员权限
    sei.lpFile = @strExePath
    If IsLoadDriver Then sei.lpParameters = StrPtrW("-LoadDriver")
    sei.nShow = SW_SHOW
    sei.fMask = SEE_MASK_NOCLOSEPROCESS Or SEE_MASK_FLAG_DDEWAIT Or SEE_MASK_FLAG_NO_UI

    ' 使用 ShellExecuteEx 启动进程
    If ShellExecuteEx(@sei) Then
        ' 等待新进程启动
        WaitForInputIdle sei.hProcess, INFINITE
        ' 关闭当前进程
        TerminateProcess GetCurrentProcess, 0
    Else
        Print "ShellExecuteEx失败:" & GetLastError
    End If
End Sub

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

' 获取进程特权信息
Sub GetProcessPrivileges(ByVal hProcess As HANDLE, ByRef langID As DWORD = 0, PrivilegeInfo() As PRIVILEGE_INFO)
    Dim hToken As HANDLE
    Dim bufferSize As DWORD
    Dim infoBuffer() As BYTE
    Dim tpTokens As TOKEN_PRIVILEGES
    Dim result() As PRIVILEGE_INFO
    Dim privInfo As PRIVILEGE_INFO
    Dim i As Integer
    Dim count As Integer
    
    ' 打开进程令牌
    If OpenProcessToken(hProcess, TOKEN_QUERY, @hToken) = 0 Then
        Print "OpenProcessToken" & WinErrorMsg(GetLastError) & GetLastError
        Exit Sub
    End If
    
    ' 获取所需缓冲区大小
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, @bufferSize)
    If bufferSize = 0 Then
        Print "GetTokenInformation1" & WinErrorMsg(GetLastError) & GetLastError
        CloseHandle(hToken)
        Exit Sub
    End If
    
    ' 分配缓冲区
    ReDim infoBuffer(0 To bufferSize - 1)
    
    ' 获取特权信息
    If GetTokenInformation(hToken, TokenPrivileges, @infoBuffer(0), bufferSize, @bufferSize) = 0 Then
        Print "GetTokenInformation2" & WinErrorMsg(GetLastError) & GetLastError
        CloseHandle(hToken)
        Exit Sub
    End If
    
    ' 复制数据到结构体
    memcpy @tpTokens, @infoBuffer(0), SizeOf(TOKEN_PRIVILEGES) - (ANYSIZE_ARRAY * SizeOf(LUID_AND_ATTRIBUTES))
    
    ' 遍历所有特权
    For i = 0 To tpTokens.PrivilegeCount - 1
        Dim nameBuffer As WString * 256
        Dim displayBuffer As WString * 256
        Dim nameLen As DWORD = 256
        Dim displayLen As DWORD = 256
        Dim TheLuid As LUID
        
        ' 获取LUID
        memcpy @TheLuid, @infoBuffer(SizeOf(DWORD) + i * SizeOf(LUID_AND_ATTRIBUTES)), SizeOf(LUID)
        
        ' 获取特权名
        If LookupPrivilegeNameW(NULL, @TheLuid, @nameBuffer, @nameLen) Then
            privInfo.sName = LeftW(nameBuffer, nameLen)
            privInfo.lValue = TheLuid.lowpart
            
            ' 获取显示名
            If LookupPrivilegeDisplayNameW(NULL, @nameBuffer, @displayBuffer, @displayLen, @langID) Then
                privInfo.sDisplay = LeftW(displayBuffer, displayLen)
            Else
                privInfo.sDisplay = privInfo.sName
            End If
            
            ' 获取属性
            memcpy @privInfo.Attributes, @infoBuffer(SizeOf(DWORD) + i * SizeOf(LUID_AND_ATTRIBUTES) + SizeOf(LUID)), SizeOf(DWORD)
            
            ' 解析状态
            If (privInfo.Attributes And SE_PRIVILEGE_ENABLED) Then
                privInfo.sStatus = "Enabled"
            ElseIf (privInfo.Attributes And SE_PRIVILEGE_ENABLED_BY_DEFAULT) Then
                privInfo.sStatus = "Default Enabled"
            Else
                privInfo.sStatus = "Disabled"
            End If
            
            ' 添加到数组
            ReDim Preserve PrivilegeInfo(count) As PRIVILEGE_INFO
            PrivilegeInfo(count) = privInfo
            count += 1
        End If
    Next
    
    CloseHandle(hToken)
End Sub

' 示例：枚举当前进程特权
Sub GetProcessPrivilegeList(ByVal dwPID As DWORD, lvwPrivilege As Class_ListView)
    Dim privileges() As PRIVILEGE_INFO
    Dim langID As DWORD
    Dim i As Integer
    Dim count As Integer
    
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwPID)
    If hProcess = NULL Then
        hProcess = SnowSword_OpenProcess(dwPID)
        If hProcess = NULL Then Exit Sub
    End If
    
    ' 获取当前进程特权
    GetProcessPrivileges hProcess, langID, privileges()
    
    If Ubound(privileges) = 0 Then
        Print "无法获取进程特权信息"
        Exit Sub
    End If
    
    For i = 0 To UBound(privileges)
        lvwPrivilege.AddItemColList,, WHex(privileges(i).lValue), privileges(i).sName, privileges(i).sStatus, privileges(i).sDisplay, "0x" & WHex(privileges(i).Attributes), 
    Next
    
    ' 释放内存
    Erase privileges
End Sub