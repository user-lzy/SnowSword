' -----------------------------------------------------------------------------
' 高权限CMD创建工具 - FreeBASIC完整实现
' 核心流程:版本检测 → 进程遍历采集令牌 → 令牌校验 → ACL调整 → 定制令牌 → 会话处理 → 创建CMD
' 适配系统:Windows XP/2003/Vista/7/8/10/11 (32/64位)
' -----------------------------------------------------------------------------
#Include "win/winbase.bi"
#Include "win/psapi.bi"
'#Include "win/tlhelp32.bi"
'#Include "win/wtsapi32.bi"
#include "win/accctrl.bi"
#include "win/aclapi.bi"
#include "win/userenv.bi"

Type TOKEN_PROCESS_TRUST_LEVEL
    SID As PSID ' 信任级别SID（如源令牌的S-1-19-512-8192）
End Type
Type PTOKEN_PROCESS_TRUST_LEVEL As TOKEN_PROCESS_TRUST_LEVEL Ptr

' NtCreateToken函数原型（ntdll.dll未公开API）
Declare Function NtCreateToken Lib "ntdll.dll" Alias "NtCreateToken"( _
    ByVal TokenHandle As PHANDLE, _
    ByVal DesiredAccess As ACCESS_MASK, _
    ByVal ObjectAttributes As OBJECT_ATTRIBUTES Ptr, _
    ByVal TokenType As TOKEN_TYPE, _
    ByVal AuthenticationId As PLUID, _
    ByVal ExpirationTime As PLARGE_INTEGER, _  ' ← 确认有这个参数
    ByVal User As PTOKEN_USER, _
    ByVal Groups As PTOKEN_GROUPS, _
    ByVal Privileges As PTOKEN_PRIVILEGES, _
    ByVal Owner As PTOKEN_OWNER, _
    ByVal PrimaryGroup As PTOKEN_PRIMARY_GROUP, _
    ByVal DefaultDacl As PTOKEN_DEFAULT_DACL, _
    ByVal Source As PTOKEN_SOURCE _
) As NTSTATUS

Declare Function NtCreateTokenEx Lib "ntdll.dll" ( _
    ByVal TokenHandle          As HANDLE Ptr, _              ' RCX:  输出令牌句柄
    ByVal DesiredAccess        As ACCESS_MASK, _             ' RDX:  访问权限掩码
    ByVal ObjectAttributes     As OBJECT_ATTRIBUTES Ptr, _   ' R8:   对象属性(可为NULL)
    ByVal TokenType            As TOKEN_TYPE, _              ' R9:   令牌类型(1=Primary,2=Impersonation)
    ByVal AuthenticationId     As LUID Ptr, _                ' 栈+0x28: 认证ID(LUID)
    ByVal ExpirationTime       As LARGE_INTEGER Ptr, _       ' 栈+0x30: 过期时间
    ByVal User                 As Any Ptr, _                 ' 栈+0x38: TOKEN_USER 结构(用户SID)
    ByVal Groups               As Any Ptr, _                 ' 栈+0x40: TOKEN_GROUPS 结构(组SID数组)
    ByVal Privileges           As Any Ptr, _                 ' 栈+0x48: TOKEN_PRIVILEGES 结构(特权数组)
    ByVal Owner                As Any Ptr, _                 ' 栈+0x50: TOKEN_OWNER 结构(默认所有者)
    ByVal PrimaryGroup         As Any Ptr, _                 ' 栈+0x58: TOKEN_PRIMARY_GROUP 结构(主组)
    ByVal DefaultDacl          As Any Ptr, _                 ' 栈+0x60: TOKEN_DEFAULT_DACL 结构(默认DACL)
    ByVal TokenSource          As Any Ptr, _                 ' 栈+0x68: TOKEN_SOURCE 结构(令牌源)
    ByVal SessionId            As UByte, _                   ' 栈+0x70: 会话ID(BOOLEAN)
    ByVal UserAttributes       As Any Ptr, _                 ' 栈+0x78: 用户安全属性(TOKEN_SECURITY_ATTRIBUTES_INFORMATION)
    ByVal DeviceAttributes     As Any Ptr, _                 ' 栈+0x80: 设备安全属性
    ByVal DeviceGroups         As Any Ptr, _                 ' 栈+0x88: 设备组(TOKEN_GROUPS)
    ByVal Capabilities         As Any Ptr, _                 ' 栈+0x90: 能力属性(AppContainer)
    ByVal Claims               As Any Ptr, _                 ' 栈+0x98: 声明属性
    ByVal RestrictedDeviceGroups As Any Ptr, _               ' 栈+0xA0: 受限设备组
    ByVal TokenFlags           As ULong _                    ' 栈+0xA8: 令牌标志(如TOKEN_WRITE_RESTRICTED)
) As NTSTATUS

Declare Function ConvertStringSidToSid Lib "advapi32.dll" Alias "ConvertStringSidToSidW"(ByVal lpStringSid As LPCWSTR, ByVal pPSID As PSID Ptr) As BOOL

' 令牌标志常量定义（winnt.h官方值,FreeBASIC手动补充）
Const TOKEN_HAS_TRAVERSE_PRIVILEGE As DWORD = &h00000001
Const TOKEN_ADMINISTRATOR As DWORD = &h00000002
Const TOKEN_ELEVATION As DWORD = &H00000008

' -----------------------------------------------------------------------------
' 辅助函数:系统版本检测（适配XP/Vista+）
' 返回值:True=Vista及以上,False=XP/2003
' -----------------------------------------------------------------------------
Function CheckWindowsVersion() As Boolean
    Dim As OSVERSIONINFOEXW osvi
    Dim As ULONGLONG dwlConditionMask = 0
    
    ' 初始化版本信息结构体
    osvi.dwOSVersionInfoSize = SizeOf(OSVERSIONINFOEXW)
    osvi.dwMajorVersion = 6
    osvi.dwMinorVersion = 0
    osvi.wServicePackMajor = 0
    osvi.wServicePackMinor = 0
    osvi.wProductType = VER_NT_WORKSTATION
    
    ' 设置版本检测掩码
    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL)
    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL)
    dwlConditionMask = VerSetConditionMask(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL)
    
    ' 验证系统版本
    Return VerifyVersionInfoW(@osvi, VER_MAJORVERSION Or VER_MINORVERSION Or VER_SERVICEPACKMAJOR, dwlConditionMask)
End Function

' 启用指定特权的函数
' 参数:privilegeName - 特权名称,如 SE_DEBUG_NAME、SE_BACKUP_NAME 等
' 返回:非零表示成功,0表示失败
Function EnablePrivilege(hToken As HANDLE, privilegeName As StringW) As Boolean
    Dim tp As TOKEN_PRIVILEGES
    Dim luid As LUID
    Dim result As Boolean = False
    
    ' 2. 查找特权的 LUID（本地唯一标识符）
    If LookupPrivilegeValue(NULL, privilegeName, @luid) = 0 Then
        MyLog.PrintWin32Error "EnablePrivilege", "LookupPrivilegeValue", "hToken=0x" & WHex(hToken) & ",privilegeName=" & privilegeName
        'CloseHandle(hToken)
        Return False
    End If
    
    ' 3. 设置要启用的特权
    tp.PrivilegeCount = 1
    tp.Privileges(0).luid = luid
    tp.Privileges(0).Attributes = SE_PRIVILEGE_ENABLED
    
    ' 4. 调整令牌特权
    If AdjustTokenPrivileges(hToken, False, @tp, SizeOf(tp), NULL, NULL) = 0 Then
        MyLog.PrintWin32Error "EnablePrivilege", "AdjustTokenPrivileges", "hToken=0x" & WHex(hToken) & ",privilegeName=" & privilegeName
    Else
        ' 检查是否成功启用（AdjustTokenPrivileges 可能返回成功但实际未启用）
        If GetLastError = ERROR_SUCCESS Then
            result = True
        Else
            MyLog.PrintLog LOG_ERROR, "EnablePrivilege", "AdjustTokenPrivileges", "hToken=0x" & WHex(hToken) & ",privilegeName=" & privilegeName
        End If
    End If
    
    Return result
End Function

' -----------------------------------------------------------------------------
' 辅助函数:校验令牌是否包含SeCreateTokenPrivilege
' 参数:hToken - 令牌句柄
' 返回值:True=包含该特权,False=不包含
' -----------------------------------------------------------------------------
Function CheckTokenPrivilege(hToken As HANDLE, lpPrivilegeName As StringW) As Boolean
    Dim As DWORD dwReturnLength = 0
    Dim As LUID luid
    
    ' 1. 获取 SeDebugPrivilege 的 LUID（这才是高权限进程通常有的特权）
    If LookupPrivilegeValueW(NULL, lpPrivilegeName, @luid) = 0 Then
        MyLog.PrintWin32Error "CheckTokenPrivilege", "LookupPrivilegeValueW", "hToken=0x" & WHex(hToken) & ",lpPrivilegeName=" & lpPrivilegeName
        Return False
    End If
    
    ' 2. 第一次调用:获取所需缓冲区大小（传 NULL 和 0）
    If GetTokenInformation(hToken, TokenPrivileges, NULL, 0, @dwReturnLength) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            MyLog.PrintWin32Error "CheckTokenPrivilege", "GetTokenInformation(TokenPrivileges)", "第一次调用,hToken=0x" & WHex(hToken) & ",lpPrivilegeName=" & lpPrivilegeName
            Return False
        End If
        
        ' 3. 分配正确大小的缓冲区
        Dim As PTOKEN_PRIVILEGES pTp = CPtr(PTOKEN_PRIVILEGES, LocalAlloc(LPTR, dwReturnLength))
        If pTp = NULL Then
            MyLog.PrintLog LOG_ERROR, "CheckTokenPrivilege", "LocalAlloc", "Length=" & dwReturnLength
            Return False
        End If
        
        ' 4. 第二次调用:获取实际特权数据
        If GetTokenInformation(hToken, TokenPrivileges, pTp, dwReturnLength, @dwReturnLength) = 0 Then
            Dim As DWORD dwError = GetLastError
            LocalFree(Cast(HLOCAL, pTp))
            MyLog.PrintWin32Error "CheckTokenPrivilege", "GetTokenInformation(TokenPrivileges)", "第二次调用,hToken=0x" & WHex(hToken) & ",lpPrivilegeName=" & lpPrivilegeName
            Return False
        End If
        
        ' 5. 遍历检查特权（这才是核心逻辑）
        For i As Integer = 0 To pTp->PrivilegeCount - 1
            If (pTp->Privileges(i).luid.LowPart = luid.LowPart) AndAlso _
               (pTp->Privileges(i).luid.HighPart = luid.HighPart) Then
                ' 检查是否启用
                If (pTp->Privileges(i).Attributes And SE_PRIVILEGE_ENABLED) Then
                    LocalFree(Cast(HLOCAL, pTp))
                    Return True ' ← 找到且已启用,立即返回
                End If
            End If
        Next
        
        LocalFree(Cast(HLOCAL, pTp))
        Return False  ' ← 遍历完没找到
    Else
        ' 第一次调用成功（理论上不可能发生）
        Return False
    End If
End Function

' -----------------------------------------------------------------------------
' 辅助函数:调整令牌ACL权限（确保当前进程可操作令牌）
' 参数:hProcess - 进程句柄,pResult - 输出操作结果
' 返回值:True=成功,False=失败
' -----------------------------------------------------------------------------
Function AdjustTokenACL(hProcess As HANDLE, ByRef pResult As Integer) As Boolean
    Dim As HANDLE hToken = NULL, hCurrentToken = NULL
    Dim As PSECURITY_DESCRIPTOR pSD = NULL
    Dim As PACL pDacl = NULL, pNewDacl = NULL
    Dim As PTOKEN_USER pTokenUser = NULL
    Dim As DWORD dwReturnLength = 0
    Dim As EXPLICIT_ACCESS_W ea
    Dim As DWORD dwError = 0
    Dim As Integer nResult = 0
    
    pResult = 0
    
    ' 打开进程令牌（TOKEN_ADJUST_DEFAULT | TOKEN_READ）
    If OpenProcessToken(hProcess, TOKEN_ADJUST_DEFAULT Or TOKEN_READ, @hToken) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "OpenProcessToken", "hProcess=0x" & WHex(hProcess)
        Goto Cleanup
    End If
    
    ' 获取令牌的DACL
    If GetSecurityInfo(hToken, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, _
        NULL, NULL, @pDacl, NULL, @pSD) <> ERROR_SUCCESS Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "GetSecurityInfo", "hProcess=0x" & WHex(hProcess)
        GoTo Cleanup
    End If
    
    ' 打开当前进程令牌（获取当前用户SID）
    If OpenProcessToken(GetCurrentProcess, TOKEN_READ, @hCurrentToken) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "OpenProcessToken", "hProcess=0x" & WHex(hProcess)
        GoTo Cleanup
    End If
    
    ' 获取当前用户SID长度
    If GetTokenInformation(hCurrentToken, TokenUser, NULL, 0, @dwReturnLength) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            dwError = GetLastError
            MyLog.PrintWin32Error "AdjustTokenACL", "GetTokenInformation(TokenUser)", "第一次调用,hProcess=0x" & WHex(hProcess)
            GoTo Cleanup
        End If
    End If
    
    ' 分配内存存储用户SID
    pTokenUser = CPtr(PTOKEN_USER, LocalAlloc(LPTR, dwReturnLength))
    If pTokenUser = NULL Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "LocalAlloc", "Length=" & dwReturnLength
        GoTo Cleanup
    End If
    
    ' 获取当前用户SID
    If GetTokenInformation(hCurrentToken, TokenUser, pTokenUser, dwReturnLength, @dwReturnLength) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "GetTokenInformation(TokenUser)", "第二次调用,hProcess=0x" & WHex(hProcess)
        GoTo Cleanup
    End If
    
    ' 初始化显式访问结构体
    ZeroMemory(@ea, SizeOf(EXPLICIT_ACCESS_W))
    ea.grfAccessPermissions = TOKEN_QUERY
    ea.grfAccessMode = GRANT_ACCESS
    ea.grfInheritance = NO_INHERITANCE
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID
    ea.Trustee.TrusteeType = TRUSTEE_IS_USER
    ea.Trustee.ptstrName = CPtr(LPWSTR, pTokenUser->User.Sid)
    
    ' 修改ACL
    If SetEntriesInAclW(1, @ea, pDacl, @pNewDacl) <> ERROR_SUCCESS Then
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "SetEntriesInAclW", "hProcess=0x" & WHex(hProcess)
        GoTo Cleanup
    End If
    
    ' 应用新ACL到令牌
    If SetSecurityInfo(hToken, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, _
        NULL, NULL, pNewDacl, NULL) = ERROR_SUCCESS Then
        nResult = 1
    Else
        dwError = GetLastError
        MyLog.PrintWin32Error "AdjustTokenACL", "SetSecurityInfo", "hProcess=0x" & WHex(hProcess)
    End If

Cleanup:
    ' 资源释放
    If hToken Then CloseHandle(hToken)
    If hCurrentToken Then CloseHandle(hCurrentToken)
    If pTokenUser Then LocalFree(Cast(HLOCAL, pTokenUser))
    If pNewDacl Then LocalFree(Cast(HLOCAL, pNewDacl))
    If pSD Then LocalFree(Cast(HLOCAL, pSD))
    
    pResult = nResult
    Return (nResult = 1)
End Function

Function CheckTokenFileAccess( _
    hToken As HANDLE, _
    FileName As StringW, _
    DesiredAccess As DWORD _
) As Boolean

    Dim As PSECURITY_DESCRIPTOR pSD = NULL
    Dim As PACL pDacl = NULL

    Dim As DWORD dwResult
    Dim As GENERIC_MAPPING gm

    Dim As PRIVILEGE_SET ps
    Dim As DWORD psSize = SizeOf(PRIVILEGE_SET)

    Dim As DWORD GrantedAccess
    Dim As BOOL AccessOK

    dwResult = GetNamedSecurityInfoW( _
        StrPtrW(FileName), _
        SE_FILE_OBJECT, _
        DACL_SECURITY_INFORMATION, _
        NULL,NULL, _
        @pDacl, _
        NULL, _
        @pSD)

    If dwResult <> ERROR_SUCCESS Then Return False

    gm.GenericRead = FILE_GENERIC_READ
    gm.GenericWrite = FILE_GENERIC_WRITE
    gm.GenericExecute = FILE_GENERIC_EXECUTE
    gm.GenericAll = FILE_ALL_ACCESS

    MapGenericMask(@DesiredAccess,@gm)

    If AccessCheck( _
        pSD, _
        hToken, _
        DesiredAccess, _
        @gm, _
        @ps, _
        @psSize, _
        @GrantedAccess, _
        @AccessOK) = 0 Then

        LocalFree(pSD)
        Return False
    End If

    LocalFree(pSD)
    Return AccessOK <> 0
End Function

' -----------------------------------------------------------------------------
' 核心函数:遍历系统进程,采集高权限令牌
' 参数:pTokenHandle - 输出采集到的令牌句柄
' 返回值:True=成功,False=失败
' -----------------------------------------------------------------------------
Function CollectHighPrivToken Overload(ByRef pTokenHandle As HANDLE, lpPrivilegeName As StringW) As Boolean
    Dim As HANDLE hSnapshot = NULL, hProcess = NULL, hToken = NULL, hDupToken = NULL
    Dim As PROCESSENTRY32W pe32
    Dim As WString * 520 szProcessPath = ""
    Dim As DWORD dwProcessId = 0, dwError = 0
    Dim As Boolean bIsVista = CheckWindowsVersion()
    
    pTokenHandle = NULL
    Function = False
    
    ' 创建进程快照
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    If hSnapshot = INVALID_HANDLE_VALUE Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CollectHighPrivToken(lpPrivilegeName)", "CreateToolhelp32Snapshot"
        Goto Cleanup
    End If
    
    ' 初始化进程遍历结构体
    pe32.dwSize = SizeOf(PROCESSENTRY32W)
    If Process32FirstW(hSnapshot, @pe32) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CollectHighPrivToken(lpPrivilegeName)", "Process32FirstW"
        GoTo Cleanup
    End If
    
    ' 遍历所有进程
    Do
        ' 释放上一轮的句柄
        If hProcess Then CloseHandle(hProcess) : hProcess = NULL
        If hToken Then CloseHandle(hToken) : hToken = NULL
        If hDupToken Then CloseHandle(hDupToken) : hDupToken = NULL
        
        dwProcessId = pe32.th32ProcessID
        
        ' 过滤无效进程（PID=0/4）
        If (dwProcessId = 0) Or (dwProcessId = 4) Then
            Continue Do
        End If
        
        ' 打开目标进程
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, dwProcessId)
        If hProcess = NULL Then
            Continue Do
        End If
        
        ' 读取进程路径
        'ZeroMemory(@szProcessPath, Len(szProcessPath))
        If GetModuleFileNameExW(hProcess, NULL, @szProcessPath, 520) = 0 Then
            Continue Do
        End If
        'Print dwProcessId & ":" & szProcessPath
        
        ' 尝试打开进程令牌（先TOKEN_QUERY,失败则TOKEN_READ）
        If OpenProcessToken(hProcess, TOKEN_IMPERSONATE Or TOKEN_DUPLICATE Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE, @hToken) = 0 Then
            MyLog.PrintWin32Error "CollectHighPrivToken(lpPrivilegeName)", "OpenProcessToken", "第一次失败,dwProcessId=" & dwProcessId
            If OpenProcessToken(hProcess, TOKEN_DUPLICATE Or TOKEN_READ, @hToken) = 0 Then
                MyLog.PrintWin32Error "CollectHighPrivToken(lpPrivilegeName)", "OpenProcessToken", "第二次失败,dwProcessId=" & dwProcessId
                Continue Do
            End If
        End If
        
        ' 校验令牌特权
        If Not CheckTokenPrivilege(hToken, lpPrivilegeName) Then Continue Do
        ' 调整令牌ACL权限
        'Dim As Integer nAclResult = 0
        /'If Not AdjustTokenACL(hProcess, nAclResult) Then
            Continue Do
        End If'/
        
        ' 复制令牌（TOKEN_ALL_ACCESS）
        If DuplicateTokenEx(hToken, READ_CONTROL Or TOKEN_ADJUST_PRIVILEGES Or TOKEN_ADJUST_DEFAULT _
            Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE Or TOKEN_IMPERSONATE Or TOKEN_DUPLICATE, NULL, _
            SecurityImpersonation, TokenPrimary, @hDupToken) <> 0 Then
            
            MyLog.PrintInfo "CollectHighPrivToken",, "找到高权限进程:PID=" & dwProcessId & " 路径=" & szProcessPath
            pTokenHandle = hDupToken
            hDupToken = NULL ' 避免Cleanup释放
            Function = True
            Goto SUCCESS
        Else
            MyLog.PrintWin32Error "CollectHighPrivToken", "DuplicateTokenEx", "dwProcessId=" & dwProcessId
        End If
        
    Loop While Process32NextW(hSnapshot, @pe32)

Success:
    CollectHighPrivToken = (pTokenHandle <> NULL)
    
Cleanup:
    ' 资源释放
    If hProcess Then CloseHandle(hProcess)
    If hToken Then CloseHandle(hToken)
    If hDupToken Then CloseHandle(hDupToken)
    If hSnapshot Then CloseHandle(hSnapshot)
End Function

' -----------------------------------------------------------------------------
' 核心函数:遍历系统进程,采集高权限令牌
' 参数:pTokenHandle - 输出采集到的令牌句柄
' 返回值:True=成功,False=失败
' -----------------------------------------------------------------------------
Function CollectHighPrivToken Overload(ByRef pTokenHandle As HANDLE, lpFilePath As StringW, ByVal DesiredAccess As ACCESS_MASK) As Boolean
    Dim As HANDLE hSnapshot = NULL, hProcess = NULL, hToken = NULL, hDupToken = NULL
    Dim As PROCESSENTRY32W pe32
    Dim As WString * 520 szProcessPath = ""
    Dim As DWORD dwProcessId = 0, dwError = 0
    Dim As Boolean bIsVista = CheckWindowsVersion()
    
    pTokenHandle = NULL
    Function = False
    
    ' 创建进程快照
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    If hSnapshot = INVALID_HANDLE_VALUE Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CollectHighPrivToken(lpFilePath, DesiredAccess)", "CreateToolhelp32Snapshot"
        GoTo Cleanup
    End If
    
    ' 初始化进程遍历结构体
    pe32.dwSize = SizeOf(PROCESSENTRY32W)
    If Process32FirstW(hSnapshot, @pe32) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CollectHighPrivToken(lpFilePath, DesiredAccess)", "Process32FirstW"
        GoTo Cleanup
    End If
    
    ' 遍历所有进程
    Do
        ' 释放上一轮的句柄
        If hProcess Then CloseHandle(hProcess) : hProcess = NULL
        If hToken Then CloseHandle(hToken) : hToken = NULL
        If hDupToken Then CloseHandle(hDupToken) : hDupToken = NULL
        
        dwProcessId = pe32.th32ProcessID
        
        ' 过滤无效进程（PID=0/4）
        If (dwProcessId = 0) Or (dwProcessId = 4) Then
            Continue Do
        End If
        
        ' 打开目标进程
        hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, dwProcessId)
        If hProcess = NULL Then Continue Do
        
        ' 尝试打开进程令牌（先TOKEN_QUERY,失败则TOKEN_READ）
        If OpenProcessToken(hProcess, TOKEN_IMPERSONATE Or TOKEN_DUPLICATE Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE, @hToken) = 0 Then
            MyLog.PrintWin32Error "CollectHighPrivToken(lpFilePath, DesiredAccess)", "OpenProcessToken", "第一次失败,dwProcessId=" & dwProcessId
            If OpenProcessToken(hProcess, TOKEN_DUPLICATE Or TOKEN_READ, @hToken) = 0 Then
                MyLog.PrintWin32Error "CollectHighPrivToken(lpFilePath, DesiredAccess)", "OpenProcessToken", "第二次失败,dwProcessId=" & dwProcessId
                Continue Do
            End If
        End If
        
        ' 校验令牌特权
        If Not CheckTokenFileAccess(hToken, lpFilePath, DesiredAccess) Then Continue Do
        
        If DuplicateTokenEx(hToken, READ_CONTROL Or TOKEN_ADJUST_PRIVILEGES Or TOKEN_ADJUST_DEFAULT _
            Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE Or TOKEN_IMPERSONATE Or TOKEN_DUPLICATE, NULL, _
            SecurityImpersonation, TokenPrimary, @hDupToken) <> 0 Then
            
            MyLog.PrintInfo "CollectHighPrivToken(lpFilePath, DesiredAccess)",, "找到高权限进程:PID=" & dwProcessId & " 路径=" & szProcessPath
            pTokenHandle = hDupToken
            hDupToken = NULL ' 避免Cleanup释放
            Function = True
            Goto SUCCESS
        Else
            MyLog.PrintWin32Error "CollectHighPrivToken(lpFilePath, DesiredAccess)", "DuplicateTokenEx", "dwProcessId=" & dwProcessId
        End If
        
    Loop While Process32NextW(hSnapshot, @pe32)

Success:
    CollectHighPrivToken = (pTokenHandle <> NULL)
    
Cleanup:
    ' 资源释放
    If hProcess Then CloseHandle(hProcess)
    If hToken Then CloseHandle(hToken)
    If hDupToken Then CloseHandle(hDupToken)
    If hSnapshot Then CloseHandle(hSnapshot)
End Function

Function InitTokenSource() As TOKEN_SOURCE
    Dim ts As TOKEN_SOURCE
    ts.SourceName = "ARKTOKN" & Chr(0)
    AllocateLocallyUniqueId(@ts.SourceIdentifier)
    Return ts
End Function

' 检查是否为 Integrity SID (S-1-16-*)
Function IsIntegritySid(ByVal pSid As PSID) As Boolean
    If pSid = NULL Or IsValidSid(pSid) = 0 Then Return False
    
    Dim pAuth As SID_IDENTIFIER_AUTHORITY Ptr = GetSidIdentifierAuthority(pSid)
    If pAuth = NULL Then Return False
    
    ' Integrity SID Authority 是 {0,0,0,0,0,16} (0x10)
    Return (pAuth->Value(5) = &h10)
End Function

' 创建修正后的 Groups 数组:移除现有 Integrity SID,添加新的 High Integrity SID
Function BuildCorrectedTokenGroups( _
    ByVal pOldGroups As PTOKEN_GROUPS, _
    ByVal pNewIntegritySid As PSID _
) As PTOKEN_GROUPS

    Dim As DWORD i, j
    Dim As DWORD oldCount = pOldGroups->GroupCount
    Dim As DWORD integrityCount = 0
    
    ' 统计现有的 Integrity SID 数量
    For i = 0 To oldCount - 1
        If IsIntegritySid(pOldGroups->Groups(i).Sid) Then
            integrityCount += 1
        End If
    Next
    
    ' 新大小 = 原大小 - 旧Integrity数量 + 1个新Integrity
    Dim As DWORD newCount = oldCount - integrityCount + 1
    Dim As DWORD newSize = SizeOf(DWORD) + newCount * SizeOf(SID_AND_ATTRIBUTES)
    
    Dim As PTOKEN_GROUPS pNew = LocalAlloc(LPTR, newSize)
    If pNew = NULL Then Return NULL
    
    ' 复制非 Integrity 的 Groups
    j = 0
    For i = 0 To oldCount - 1
        If Not IsIntegritySid(pOldGroups->Groups(i).Sid) Then
            pNew->Groups(j) = pOldGroups->Groups(i)
            j += 1
        End If
    Next
    
    ' 添加新的 High Integrity SID,Attributes 必须为 0x65 (SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_INTEGRITY | SE_GROUP_INTEGRITY_ENABLED)
    pNew->Groups(j).Sid = pNewIntegritySid
    pNew->Groups(j).Attributes = SE_GROUP_MANDATORY Or SE_GROUP_ENABLED Or SE_GROUP_INTEGRITY Or SE_GROUP_INTEGRITY_ENABLED
    pNew->GroupCount = newCount
    
    Return pNew
End Function

Function QueryTokenInfo( _
    ByVal hToken As HANDLE, _
    ByVal InfoClass As TOKEN_INFORMATION_CLASS, _
    ByRef ppOut As Any Ptr _
) As Boolean

    Dim As DWORD cb = 0
    GetTokenInformation(hToken, InfoClass, NULL, 0, @cb)

    If cb = 0 Then
        MyLog.PrintWin32Error "QueryTokenInfo", "GetTokenInformation", "第一次调用,hToken=0x" & WHex(hToken)
        Return False
    End If

    ppOut = LocalAlloc(LPTR, cb)
    If ppOut = NULL Then Return FALSE

    If GetTokenInformation(hToken, InfoClass, ppOut, cb, @cb) = False Then
        MyLog.PrintWin32Error "QueryTokenInfo", "GetTokenInformation", "第二次调用,hToken=0x" & WHex(hToken)
        LocalFree(ppOut)
        ppOut = NULL
        Return FALSE
    End If

    Return TRUE
End Function

' 查询令牌安全描述符
' 参数:
'   hToken       - 令牌句柄（需要相应的查询权限,如READ_CONTROL）
'   SecurityInfo - 要查询的安全信息类型（组合标志）
'   pdwSize      - [可选] 返回安全描述符的字节大小（可为NULL）
' 返回:
'   成功:指向SECURITY_DESCRIPTOR的指针（调用者必须用FreeTokenSecurityDescriptor释放）
'   失败:NULL,可通过GetLastError获取错误码
Function QueryTokenSecurityInfo( _
    ByVal hToken As HANDLE, _
    ByVal SecurityInfo As SECURITY_INFORMATION, _
    ByVal pdwSize As DWORD Ptr = NULL _
) As PSECURITY_DESCRIPTOR
    
    Dim dwLengthNeeded As DWORD = 0
    Dim pSD As PSECURITY_DESCRIPTOR = NULL
    
    ' 第一次调用:获取所需缓冲区大小
    If GetKernelObjectSecurity(hToken, SecurityInfo, NULL, 0, @dwLengthNeeded) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            MyLog.PrintWin32Error "QueryTokenSecurityInfo", "GetKernelObjectSecurity", "第一次调用,hToken=0x" & WHex(hToken)
            Return NULL
        End If
    End If
    
    If dwLengthNeeded = 0 Then
        MyLog.PrintLog LOG_ERROR, "QueryTokenSecurityInfo", "GetKernelObjectSecurity", "hToken=0x" & WHex(hToken) & ",dwLengthNeeded = 0"
        Return NULL
    End If
    
    ' 分配内存
    pSD = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLengthNeeded)
    If pSD = NULL Then
        MyLog.PrintLog LOG_ERROR, "QueryTokenSecurityInfo", "HeapAlloc", "Length=" & dwLengthNeeded
        Return NULL
    End If
    
    ' 第二次调用:获取实际数据
    If GetKernelObjectSecurity(hToken, SecurityInfo, pSD, dwLengthNeeded, @dwLengthNeeded) = 0 Then
        HeapFree(GetProcessHeap(), 0, pSD)
        MyLog.PrintWin32Error "QueryTokenSecurityInfo", "GetKernelObjectSecurity", "第二次调用,hToken=0x" & WHex(hToken)
        Return NULL
    End If
    
    ' 返回大小（如果调用者需要）
    If pdwSize <> NULL Then
        *pdwSize = dwLengthNeeded
    End If
    
    Return pSD
End Function

Function CreateIntegritySid(ByVal Rid As DWORD) As PSID
    Dim As SID_IDENTIFIER_AUTHORITY NtAuth = _
        Type({&H0, &H0, &H0, &H0, &H0, &H10})

    Dim As PSID pSid = NULL
    If AllocateAndInitializeSid( _
        @NtAuth, 1, Rid, _
        0,0,0,0,0,0,0, _
        @pSid _
    ) = FALSE Then
        Return NULL
    End If

    Return pSid
End Function

Function AppendIntegrityGroup( _
    ByVal pOld As PTOKEN_GROUPS, _
    ByVal pIntegritySid As PSID _
) As PTOKEN_GROUPS

    Dim As DWORD oldCount = pOld->GroupCount
    Dim As DWORD newCount = oldCount + 1

    Dim As DWORD oldSize = _
        SizeOf(DWORD) + oldCount * SizeOf(SID_AND_ATTRIBUTES)

    Dim As DWORD newSize = _
        SizeOf(DWORD) + newCount * SizeOf(SID_AND_ATTRIBUTES)

    Dim As PTOKEN_GROUPS pNew = LocalAlloc(LPTR, newSize)
    If pNew = NULL Then Return NULL

    memcpy(pNew, pOld, oldSize)

    pNew->GroupCount = newCount
    pNew->Groups(oldCount).Sid = pIntegritySid
    pNew->Groups(oldCount).Attributes = _
        SE_GROUP_INTEGRITY Or SE_GROUP_INTEGRITY_ENABLED

    Return pNew
End Function

' -----------------------------------------------------------------------------
' 核心函数:定制化创建高权限令牌（基于NtCreateToken）
' 参数:hOriginalToken - 原始令牌句柄
' 返回值:定制后的令牌句柄（NULL=失败）
' -----------------------------------------------------------------------------
Function CreateCustomToken(ByVal hSmssToken As HANDLE, ByVal hSourceToken As HANDLE) As HANDLE
    Dim hNewToken As HANDLE = NULL

    ' === Query required info ===
    Dim pUser      As PTOKEN_USER
    Dim pGroups    As PTOKEN_GROUPS
    Dim pPrivs     As PTOKEN_PRIVILEGES
    Dim pOwner     As PTOKEN_OWNER
    Dim pPrimary   As PTOKEN_PRIMARY_GROUP
    Dim pDacl      As PTOKEN_DEFAULT_DACL
    Dim pStats     As PTOKEN_STATISTICS

    If QueryTokenInfo(hSourceToken, TokenUser, pUser) = False Then MyLog.PrintWin32Error "CreateCustomToken", "QueryTokenInfo(TokenUser)" : Return NULL
    If QueryTokenInfo(hSourceToken, TokenGroups, pGroups) = False Then MyLog.PrintWin32Error "CreateCustomToken", "QueryTokenInfo(TokenGroups)" : Return NULL
    If QueryTokenInfo(hSourceToken, TokenPrivileges, pPrivs) = False Then MyLog.PrintWin32Error "CreateCustomToken", "QueryTokenInfo(TokenPrivileges)" : Return NULL
    QueryTokenInfo(hSourceToken, TokenOwner, pOwner)
    QueryTokenInfo(hSourceToken, TokenPrimaryGroup, pPrimary)
    QueryTokenInfo(hSourceToken, TokenDefaultDacl, pDacl)
    If QueryTokenInfo(hSourceToken, TokenStatistics, pStats) = False Then MyLog.PrintWin32Error "CreateCustomToken", "QueryTokenInfo(TokenStatistics)" : Return NULL
    'pStats->TokenFlags = TOKEN_HAS_TRAVERSE_PRIVILEGE Or TOKEN_ELEVATION Or TOKEN_ADMINISTRATOR

    ' === Integrity ===
    Dim pIntegritySid As PSID
    pIntegritySid = CreateIntegritySid(SECURITY_MANDATORY_SYSTEM_RID)
    If pIntegritySid = NULL Then Return NULL

    Dim pNewGroups As PTOKEN_GROUPS = BuildCorrectedTokenGroups(pGroups, pIntegritySid)
    'pNewGroups = pGroups   ' 暂时不要 AppendIntegrityGroup
    If pNewGroups = NULL Then Return NULL

    ' === Token Source ===
    Dim ts As TOKEN_SOURCE = InitTokenSource()
    
    ' 模拟原始令牌用户（提升NtCreateToken调用权限） 
    Dim hLsassToken As HANDLE = NULL
    If ImpersonateLoggedOnUser(hSmssToken) = 0 Then
        Dim As DWORD dwError = GetLastError
        MyLog.PrintWin32Error "CreateCustomToken", "ImpersonateLoggedOnUser", "hSmssToken=0x" & WHex(hSmssToken)
        'Goto Cleanup 
    End If
    
    Dim expTime As LARGE_INTEGER
    expTime.QuadPart = -1 ' 永不过期
    
    Dim oa As OBJECT_ATTRIBUTES
    Dim pSD As PSECURITY_DESCRIPTOR = NULL
    ' 安全信息类型:取完整的SD（所有者+主组+DACL）,这是OBJECT_ATTRIBUTES需要的最小完整集
    Const SD_INFO As SECURITY_INFORMATION = OWNER_SECURITY_INFORMATION Or GROUP_SECURITY_INFORMATION Or DACL_SECURITY_INFORMATION

    ' 调用QueryTokenSecurityInfo,获取令牌内核对象的合法SD
    pSD = QueryTokenSecurityInfo(hSourceToken, SD_INFO)
    If pSD = NULL Then
        MyLog.PrintLog LOG_ERROR, "CreateCustomToken", "QueryTokenSecurityInfo", "hSourceToken=0x" & WHex(hSourceToken)
        Return NULL
    End If

    ' 可选:校验SD合法性（双重保障,避免无效SD）
    If IsValidSecurityDescriptor(pSD) = 0 Then
        MyLog.PrintLog LOG_ERROR, "CreateCustomToken",, "获取的安全描述符结构无效"
        HeapFree(GetProcessHeap(), 0, pSD)
        Return NULL
    End If

    ' 初始化OBJECT_ATTRIBUTES（你的原有逻辑,完全不变）
    ZeroMemory(@oa, SizeOf(OBJECT_ATTRIBUTES))
    oa.Length = SizeOf(OBJECT_ATTRIBUTES)
    oa.SecurityDescriptor = pSD ' 赋值合法SD,彻底解决访问冲突

    ' === NtCreateToken ===
    Dim status As NTSTATUS = NtCreateToken(@hNewToken, TOKEN_ALL_ACCESS, @oa, TokenPrimary, @pStats->AuthenticationId, @expTime, _
    pUser, pNewGroups, pPrivs, pOwner, pPrimary, pDacl, _
    @ts _
    )
    
    If Not NT_SUCCESS(status) Then
        MyLog.PrintNtError "CreateCustomToken", "NtCreateToken失败", status
        Return NULL
    End If
    
    Dim tmlSize As DWORD = SizeOf(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)
    Dim pTml As PTOKEN_MANDATORY_LABEL = LocalAlloc(LPTR, tmlSize)
    pTml->Label.SID = pIntegritySid
    pTml->Label.Attributes = SE_GROUP_INTEGRITY
    SetTokenInformation(hNewToken, TokenIntegrityLevel, pTml, tmlSize)
        
    Dim tmp As TOKEN_MANDATORY_POLICY
    tmp.Policy = TOKEN_MANDATORY_POLICY_NO_WRITE_UP
    If SetTokenInformation(hNewToken, TokenMandatoryPolicy, @tmp, SizeOf(TOKEN_MANDATORY_POLICY)) = False Then
        MyLog.PrintWin32Error "CreateCustomToken", "SetTokenInformation(TokenMandatoryPolicy)"
        CloseHandle(hNewToken)
        Return NULL
    End If
    
    ' 还原到当前进程原始身份
    RevertToSelf
    
    Return hNewToken
End Function

' ============================================================
' 修改后的 CreateHighPrivCMD - 支持任意命令行执行
' ============================================================
Function CreateHighPrivProcess( _
    ByVal hCustomToken As HANDLE, _
    ByVal lpApplicationName As LPCWSTR = NULL, _
    ByVal lpCommandLine As LPWSTR = NULL, _      ' ← 新增:要执行的命令行（可为NULL则默认cmd）
    ByVal lpWorkDir As Wstring Ptr = NULL, _           ' ← 新增:工作目录（可为NULL则使用当前目录）
    ByVal dwCreationFlags As DWORD = 0 _               ' ← 新增:额外的创建标志
) As DWORD

    Dim As HANDLE hDupTokenForImpersonation = NULL
    Dim As HANDLE hDupTokenForProcess = NULL
    Dim As DWORD dwProcessId = 0, dwError = 0
    Dim As STARTUPINFOW si
    Dim As PROCESS_INFORMATION pi
    Dim As WString * MAX_PATH szDefaultWorkDir
    Dim As LPWSTR lpEnvBlock = NULL
    Dim As WString * 32768 szCmdLine ' ← 命令行缓冲区
    Dim As DWORD dwFlags = CREATE_NEW_CONSOLE Or CREATE_UNICODE_ENVIRONMENT
    
    ZeroMemory(@si, SizeOf(STARTUPINFOW))
    ZeroMemory(@pi, SizeOf(PROCESS_INFORMATION))
    si.cb = SizeOf(STARTUPINFOW)
    si.lpDesktop = StrPtrW("winsta0\default")
    
    ' ========================================================
    ' 1. 处理命令行参数（关键修改点）
    ' ========================================================
    If lpCommandLine = NULL OrElse *lpCommandLine = "" Then
        ' 默认启动 CMD
        szCmdLine = "C:\Windows\System32\cmd.exe"
        lpCommandLine = @szCmdLine
    Else
        ' 使用用户传入的命令
        ' 注意:如果命令包含空格且不是完整路径,需要处理
        'szCmdLine = *lpCommandLine
    End If
    
    ' ========================================================
    ' 2. 处理工作目录
    ' ========================================================
    Dim As WString Ptr pWorkDir = lpWorkDir
    If pWorkDir = NULL Then
        ' 使用当前程序所在目录作为默认
        If GetModuleFileNameW(NULL, @szDefaultWorkDir, MAX_PATH) <> 0 Then
            PathRemoveFileSpecW(@szDefaultWorkDir)
            pWorkDir = @szDefaultWorkDir
        End If
    End If
    
    ' ========================================================
    ' 3. 令牌复制（保持原有逻辑不变）
    ' ========================================================
    If DuplicateTokenEx(hCustomToken, TOKEN_ALL_ACCESS, NULL, _
        SecurityImpersonation, TokenImpersonation, @hDupTokenForImpersonation) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CreateHighPrivProcess", "DuplicateTokenEx(TokenImpersonation)", "hCustomToken=0x" & WHex(hCustomToken)
        GoTo Cleanup
    End If
    
    If DuplicateTokenEx(hCustomToken, TOKEN_ALL_ACCESS, NULL, _
        SecurityImpersonation, TokenPrimary, @hDupTokenForProcess) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CreateHighPrivProcess", "DuplicateTokenEx(TokenPrimary)", "hCustomToken=0x" & WHex(hCustomToken)
        GoTo Cleanup
    End If
    
    RevertToSelf
    MyLog.PrintInfo "CreateHighPrivProcess", "RevertToSelf", "线程身份已还原"
    
    ' ========================================================
    ' 4. 构建创建标志（合并用户传入的标志）
    ' ========================================================
    dwFlags Or= dwCreationFlags  ' 合并用户自定义标志（如 CREATE_SUSPENDED）
    
    ' ========================================================
    ' 5. 创建环境块（保持原有逻辑）
    ' ========================================================
    If CreateEnvironmentBlock(@lpEnvBlock, hDupTokenForProcess, True) = 0 Then
        dwError = GetLastError
        MyLog.PrintWin32Error "CreateHighPrivProcess", "CreateEnvironmentBlock"
        Goto Cleanup
    End If
    
    MyLog.PrintInfo "CreateHighPrivProcess",, "hCustomToken:0x" & WHex(hCustomToken) & " hDupTokenForProcess:0x" & WHex(hDupTokenForProcess)
    'AfxMsg "Pause"
    
    ' ========================================================
    ' 6. 创建进程（关键修改:使用 szCmdLine 而非硬编码）
    ' ========================================================
    ' 方案A: CreateProcessWithTokenW（需要 SeImpersonatePrivilege）
    If CreateProcessWithTokenW(hDupTokenForProcess, 0, lpApplicationName, lpCommandLine, dwFlags, lpEnvBlock, pWorkDir, @si, @pi ) <> 0 Then
        dwProcessId = pi.dwProcessId
        MyLog.PrintInfo "CreateHighPrivProcess", "CreateProcessWithTokenW", "高权限进程创建成功,PID=" & dwProcessId & " 命令行: " & szCmdLine
        
        CloseHandle(pi.hThread)
        CloseHandle(pi.hProcess)
    Else
        dwError = GetLastError
        MyLog.PrintWin32Error "CreateHighPrivProcess", "CreateProcessWithTokenW"
        
        ' ========================================================
        ' 7. 备用方案: CreateProcessAsUserW（保持原有逻辑）
        ' ========================================================
        If dwError = ERROR_PRIVILEGE_NOT_HELD Then
            MyLog.PrintInfo "CreateHighPrivProcess",, "尝试使用CreateProcessAsUserW..."
            
            If CreateProcessAsUserW(hDupTokenForProcess, NULL, @szCmdLine, NULL, NULL, False, dwFlags, lpEnvBlock, pWorkDir, @si, @pi) <> 0 Then
                dwProcessId = pi.dwProcessId
                MyLog.PrintInfo "CreateHighPrivProcess", "CreateProcessAsUserW", "备用方案成功,PID=" & dwProcessId & "命令行: " & szCmdLine
                CloseHandle(pi.hThread)
                CloseHandle(pi.hProcess)
            Else
                dwError = GetLastError
                MyLog.PrintWin32Error "CreateHighPrivProcess", "CreateProcessAsUserW也失败"
            End If
        End If
    End If

Cleanup:
    If lpEnvBlock Then DestroyEnvironmentBlock(lpEnvBlock)
    If hDupTokenForImpersonation Then CloseHandle(hDupTokenForImpersonation)
    If hDupTokenForProcess Then CloseHandle(hDupTokenForProcess)
    
    Return dwProcessId
End Function

' -----------------------------------------------------------------------------
' 主函数:串联所有流程
' -----------------------------------------------------------------------------
Function CreateSYSTEMPriviegeProcess( _
    ByVal lpApplicationName As Wstring Ptr = NULL, _
    ByVal lpCommandLine As Wstring Ptr = NULL, _      ' ← 新增:要执行的命令行（可为NULL则默认cmd）
    ByVal lpWorkDir As Wstring Ptr = NULL, _           ' ← 新增:工作目录（可为NULL则使用当前目录）
    ByVal dwCreationFlags As DWORD = 0 _ ' ← 新增:额外的创建标志
    ) As Boolean
    
    Function = True
    Dim As HANDLE hOriginalToken = NULL, hSmssToken = NULL, hCustomToken = NULL
    Dim As DWORD dwCmdPid = 0
    
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "=== 高权限进程创建工具 ==="
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "开始采集高权限令牌..."
    
    ' 步骤1:采集高权限原始令牌
    If Not CollectHighPrivToken(hSmssToken, "SeTcbPrivilege") Then
        MyLog.PrintLog LOG_ERROR, "CreateSYSTEMPriviegeProcess", "CollectHighPrivToken", "采集高权限令牌失败！"
        Function = False
        Goto Cleanup
    End If
    /'/If Not CollectProcessToken(hOriginalToken, "winlogon.exe") Then
        Print "采集高权限令牌失败！"
        Function = False
        GoTo Cleanup
    End If'/
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "高权限令牌采集成功！"
    If Not EnablePrivilege(hSmssToken, "SeCreateTokenPrivilege") Then Return NULL
    
    ' 步骤2:定制化创建新令牌
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "开始定制高权限令牌..."
    hCustomToken = CreateCustomToken(hSmssToken, hSmssToken) ' hOriginalToken
    If hCustomToken = NULL Then
        MyLog.PrintLog LOG_ERROR, "CreateSYSTEMPriviegeProcess", "CreateCustomToken", "定制高权限令牌失败！"
        Function = False
        GoTo Cleanup
    End If
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "高权限令牌定制成功！"
    
    ' 步骤3:创建高权限CMD
    MyLog.PrintInfo "CreateSYSTEMPriviegeProcess",, "开始创建高权限进程..."
    dwCmdPid = CreateHighPrivProcess(hCustomToken, lpApplicationName, lpCommandLine, lpWorkDir, dwCreationFlags)
    If dwCmdPid = 0 Then
        MyLog.PrintLog LOG_ERROR, "CreateSYSTEMPriviegeProcess", "CreateHighPrivProcess", "创建高权限进程失败！"
        Function = False
    End If

Cleanup:
    ' 资源释放
    If hOriginalToken Then CloseHandle(hOriginalToken)
    If hCustomToken Then CloseHandle(hCustomToken)
End Function