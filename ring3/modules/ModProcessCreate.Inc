' -----------------------------------------------------------------------------
' 高权限CMD创建工具 - FreeBASIC完整实现
' 核心流程：版本检测 → 进程遍历采集令牌 → 令牌校验 → ACL调整 → 定制令牌 → 会话处理 → 创建CMD
' 适配系统：Windows XP/2003/Vista/7/8/10/11 (32/64位)
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

' 令牌标志常量定义（winnt.h官方值，FreeBASIC手动补充）
Const TOKEN_HAS_TRAVERSE_PRIVILEGE As DWORD = &h00000001
Const TOKEN_ADMINISTRATOR As DWORD = &h00000002
Const TOKEN_ELEVATION As DWORD = &H00000008

' -----------------------------------------------------------------------------
' 辅助函数：系统版本检测（适配XP/Vista+）
' 返回值：True=Vista及以上，False=XP/2003
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
' 参数：privilegeName - 特权名称，如 SE_DEBUG_NAME、SE_BACKUP_NAME 等
' 返回：非零表示成功，0表示失败
Function EnablePrivilege(hToken As HANDLE, privilegeName As StringW) As Boolean
    Dim tp As TOKEN_PRIVILEGES
    Dim luid As LUID
    Dim result As Boolean = False
    
    ' 2. 查找特权的 LUID（本地唯一标识符）
    If LookupPrivilegeValue(NULL, privilegeName, @luid) = 0 Then
        Print "LookupPrivilegeValue 失败，错误码: "; GetLastError()
        'CloseHandle(hToken)
        Return False
    End If
    
    ' 3. 设置要启用的特权
    tp.PrivilegeCount = 1
    tp.Privileges(0).luid = luid
    tp.Privileges(0).Attributes = SE_PRIVILEGE_ENABLED
    
    ' 4. 调整令牌特权
    If AdjustTokenPrivileges(hToken, False, @tp, SizeOf(tp), NULL, NULL) = 0 Then
        Print "AdjustTokenPrivileges 失败，错误码: "; GetLastError()
    Else
        ' 检查是否成功启用（AdjustTokenPrivileges 可能返回成功但实际未启用）
        If GetLastError = ERROR_SUCCESS Then
            result = True
        Else
            Print "特权未完全启用，错误码: "; GetLastError()
        End If
    End If
    
    ' 5. 清理
    'CloseHandle(hToken)
    Return result
End Function

' -----------------------------------------------------------------------------
' 辅助函数：校验令牌是否包含SeCreateTokenPrivilege
' 参数：hToken - 令牌句柄
' 返回值：True=包含该特权，False=不包含
' -----------------------------------------------------------------------------
Function CheckTokenPrivilege(hToken As HANDLE, lpPrivilegeName As StringW) As Boolean
    Dim As DWORD dwReturnLength = 0
    Dim As LUID luid
    
    ' 1. 获取 SeDebugPrivilege 的 LUID（这才是高权限进程通常有的特权）
    If LookupPrivilegeValueW(NULL, lpPrivilegeName, @luid) = 0 Then
        Print "LookupPrivilegeValueW 失败：" & GetLastError
        Return False
    End If
    
    ' 2. 第一次调用：获取所需缓冲区大小（传 NULL 和 0）
    If GetTokenInformation(hToken, TokenPrivileges, NULL, 0, @dwReturnLength) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            Print "GetTokenInformation(获取大小)失败：" & GetLastError
            Return False
        End If
        
        ' 3. 分配正确大小的缓冲区
        Dim As PTOKEN_PRIVILEGES pTp = CPtr(PTOKEN_PRIVILEGES, LocalAlloc(LPTR, dwReturnLength))
        If pTp = NULL Then
            Print "LocalAlloc 失败：" & GetLastError
            Return False
        End If
        
        ' 4. 第二次调用：获取实际特权数据
        If GetTokenInformation(hToken, TokenPrivileges, pTp, dwReturnLength, @dwReturnLength) = 0 Then
            Dim As DWORD dwError = GetLastError
            LocalFree(Cast(HLOCAL, pTp))
            Print "GetTokenInformation(获取数据)失败：" & WinErrorMsg(dwError) & dwError
            Return False
        End If
        
        ' 5. 遍历检查特权（这才是核心逻辑）
        For i As Integer = 0 To pTp->PrivilegeCount - 1
            If (pTp->Privileges(i).luid.LowPart = luid.LowPart) AndAlso _
               (pTp->Privileges(i).luid.HighPart = luid.HighPart) Then
                ' 检查是否启用
                If (pTp->Privileges(i).Attributes And SE_PRIVILEGE_ENABLED) Then
                    LocalFree(Cast(HLOCAL, pTp))
                    Return True ' ← 找到且已启用，立即返回
                'Else
                '    If EnablePrivilege(
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
' 辅助函数：调整令牌ACL权限（确保当前进程可操作令牌）
' 参数：hProcess - 进程句柄，pResult - 输出操作结果
' 返回值：True=成功，False=失败
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
        Print "OpenProcessToken失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 获取令牌的DACL
    If GetSecurityInfo(hToken, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, _
        NULL, NULL, @pDacl, NULL, @pSD) <> ERROR_SUCCESS Then
        dwError = GetLastError
        Print "GetSecurityInfo失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 打开当前进程令牌（获取当前用户SID）
    If OpenProcessToken(GetCurrentProcess, TOKEN_READ, @hCurrentToken) = 0 Then
        dwError = GetLastError
        Print "OpenProcessToken(Current)失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 获取当前用户SID长度
    If GetTokenInformation(hCurrentToken, TokenUser, NULL, 0, @dwReturnLength) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            dwError = GetLastError
            Print "GetTokenInformation(TokenUser)失败：" & dwError
            GoTo Cleanup
        End If
    End If
    
    ' 分配内存存储用户SID
    pTokenUser = CPtr(PTOKEN_USER, LocalAlloc(LPTR, dwReturnLength))
    If pTokenUser = NULL Then
        dwError = GetLastError
        Print "LocalAlloc失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 获取当前用户SID
    If GetTokenInformation(hCurrentToken, TokenUser, pTokenUser, dwReturnLength, @dwReturnLength) = 0 Then
        dwError = GetLastError
        Print "GetTokenInformation(TokenUser)失败：" & dwError
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
        Print "SetEntriesInAclW失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 应用新ACL到令牌
    If SetSecurityInfo(hToken, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, _
        NULL, NULL, pNewDacl, NULL) = ERROR_SUCCESS Then
        nResult = 1
    Else
        dwError = GetLastError
        Print "SetSecurityInfo失败：" & WinErrorMsg(dwError) & dwError
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

' -----------------------------------------------------------------------------
' 核心函数：遍历系统进程，采集高权限令牌
' 参数：pTokenHandle - 输出采集到的令牌句柄
' 返回值：True=成功，False=失败
' -----------------------------------------------------------------------------
Function CollectHighPrivToken(ByRef pTokenHandle As HANDLE, lpPrivilegeName As StringW) As Boolean
    Dim As HANDLE hSnapshot = NULL, hProcess = NULL, hToken = NULL, hDupToken = NULL
    Dim As PROCESSENTRY32W pe32
    Dim As WString * 520 szProcessPath = ""
    Dim As DWORD dwProcessId = 0, dwAccess = 0, dwError = 0
    Dim As Boolean bIsVista = CheckWindowsVersion()
    
    pTokenHandle = NULL
    Function = False
    
    ' 创建进程快照
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    If hSnapshot = INVALID_HANDLE_VALUE Then
        dwError = GetLastError
        Print "CreateToolhelp32Snapshot失败：" & dwError
        GoTo Cleanup
    End If
    
    ' 初始化进程遍历结构体
    pe32.dwSize = SizeOf(PROCESSENTRY32W)
    If Process32FirstW(hSnapshot, @pe32) = 0 Then
        dwError = GetLastError
        Print "Process32FirstW失败：" & dwError
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
        
        ' 确定进程访问权限（Vista+用4096，XP用1040）
        'dwAccess = IIf(bIsVista, 4096, 1040)
        dwAccess = PROCESS_QUERY_LIMITED_INFORMATION ' PROCESS_ALL_ACCESS' PROCESS_QUERY_INFORMATION
        
        ' 打开目标进程
        hProcess = OpenProcess(dwAccess, False, dwProcessId)
        If hProcess = NULL Then
            Continue Do
        End If
        
        ' 读取进程路径
        'ZeroMemory(@szProcessPath, Len(szProcessPath))
        If GetModuleFileNameExW(hProcess, NULL, @szProcessPath, 520) = 0 Then
            Continue Do
        End If
        'Print dwProcessId & ":" & szProcessPath
        
        ' 尝试打开进程令牌（先TOKEN_QUERY，失败则TOKEN_READ）
        If OpenProcessToken(hProcess, TOKEN_IMPERSONATE Or TOKEN_DUPLICATE Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE, @hToken) = 0 Then
            Print "OpenProcessToken1(" & dwProcessId & ")失败:" & GetLastError
            If OpenProcessToken(hProcess, TOKEN_DUPLICATE Or TOKEN_READ, @hToken) = 0 Then
                Print "OpenProcessToken2(" & dwProcessId & ")失败:" & GetLastError
                Continue Do
            End If
        End If
        'Print "OpenProcessToken(" & dwProcessId & ") Success"
        ' 校验令牌特权
        If Not CheckTokenPrivilege(hToken, lpPrivilegeName) Then
            Continue Do
        End If
        Print "PID:" & dwProcessId
        ' 调整令牌ACL权限
        Dim As Integer nAclResult = 0
        /'If Not AdjustTokenACL(hProcess, nAclResult) Then
            Continue Do
        End If'/
        
        ' 复制令牌（TOKEN_ALL_ACCESS）
        'If DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, _
        '    SecurityImpersonation, TokenPrimary, @hDupToken) <> 0 Then
        If DuplicateTokenEx(hToken, READ_CONTROL Or TOKEN_ADJUST_PRIVILEGES Or TOKEN_ADJUST_DEFAULT _
            Or TOKEN_QUERY Or TOKEN_QUERY_SOURCE Or TOKEN_IMPERSONATE Or TOKEN_DUPLICATE, NULL, _
            SecurityImpersonation, TokenPrimary, @hDupToken) <> 0 Then
            
            Print "找到高权限进程：PID=" & dwProcessId & " 路径=" & szProcessPath
            pTokenHandle = hDupToken
            hDupToken = NULL ' 避免Cleanup释放
            Function = True
            Goto SUCCESS
        Else
            Print "[CollectHighPrivToken]DuplicateTokenEx(" & dwProcessId & "):" & GetLastError
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

' 创建修正后的 Groups 数组：移除现有 Integrity SID，添加新的 High Integrity SID
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
    
    ' 添加新的 High Integrity SID，Attributes 必须为 0x65 (SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_INTEGRITY | SE_GROUP_INTEGRITY_ENABLED)
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
        Print "[QueryTokenInfo]GetTokenInformation1:" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If

    ppOut = LocalAlloc(LPTR, cb)
    If ppOut = NULL Then Return FALSE

    If GetTokenInformation(hToken, InfoClass, ppOut, cb, @cb) = False Then
        Print "[QueryTokenInfo]GetTokenInformation2:" & WinErrorMsg(GetLastError) & GetLastError
        LocalFree(ppOut)
        ppOut = NULL
        Return FALSE
    End If

    Return TRUE
End Function

' 查询令牌安全描述符
' 参数：
'   hToken       - 令牌句柄（需要相应的查询权限，如READ_CONTROL）
'   SecurityInfo - 要查询的安全信息类型（组合标志）
'   pdwSize      - [可选] 返回安全描述符的字节大小（可为NULL）
' 返回：
'   成功：指向SECURITY_DESCRIPTOR的指针（调用者必须用FreeTokenSecurityDescriptor释放）
'   失败：NULL，可通过GetLastError获取错误码
Function QueryTokenSecurityInfo( _
    ByVal hToken As HANDLE, _
    ByVal SecurityInfo As SECURITY_INFORMATION, _
    ByVal pdwSize As DWORD Ptr = NULL _
) As PSECURITY_DESCRIPTOR
    
    Dim dwLengthNeeded As DWORD = 0
    Dim pSD As PSECURITY_DESCRIPTOR = NULL
    
    ' 第一次调用：获取所需缓冲区大小
    If GetKernelObjectSecurity(hToken, SecurityInfo, NULL, 0, @dwLengthNeeded) = 0 Then
        If GetLastError <> ERROR_INSUFFICIENT_BUFFER Then
            ' 其他错误（如访问拒绝）
            Print "[QueryTokenSecurityInfo]GetKernelObjectSecurity1:" & WinErrorMsg(GetLastError) & GetLastError
            Return NULL
        End If
    End If
    
    If dwLengthNeeded = 0 Then
        'SetLastError(ERROR_INVALID_DATA)
        Print "[QueryTokenSecurityInfo]GetKernelObjectSecurity1:dwLengthNeeded = 0"
        Return NULL
    End If
    
    ' 分配内存
    pSD = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLengthNeeded)
    If pSD = NULL Then
        'SetLastError(ERROR_NOT_ENOUGH_MEMORY)
        Print "[QueryTokenSecurityInfo]HeapAlloc:分配失败"
        Return NULL
    End If
    
    ' 第二次调用：获取实际数据
    If GetKernelObjectSecurity(hToken, SecurityInfo, pSD, dwLengthNeeded, @dwLengthNeeded) = 0 Then
        HeapFree(GetProcessHeap(), 0, pSD)
        Print "[QueryTokenSecurityInfo]GetKernelObjectSecurity2:" & WinErrorMsg(GetLastError) & GetLastError
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
' 核心函数：定制化创建高权限令牌（基于NtCreateToken）
' 参数：hOriginalToken - 原始令牌句柄
' 返回值：定制后的令牌句柄（NULL=失败）
' -----------------------------------------------------------------------------
Function CreateCustomToken(ByVal hSourceToken As HANDLE) As HANDLE

    Dim hNewToken As HANDLE = NULL

    ' === Query required info ===
    Dim pUser      As PTOKEN_USER
    Dim pGroups    As PTOKEN_GROUPS
    Dim pPrivs     As PTOKEN_PRIVILEGES
    Dim pOwner     As PTOKEN_OWNER
    Dim pPrimary   As PTOKEN_PRIMARY_GROUP
    Dim pDacl      As PTOKEN_DEFAULT_DACL
    Dim pStats     As PTOKEN_STATISTICS

    If QueryTokenInfo(hSourceToken, TokenUser, pUser) = False Then Print "Query TokenUser failed": Return NULL
    If QueryTokenInfo(hSourceToken, TokenGroups, pGroups) = False Then Print "Query TokenGroups failed": Return NULL
    If QueryTokenInfo(hSourceToken, TokenPrivileges, pPrivs) = False Then Print "Query TokenPrivileges failed": Return NULL
    QueryTokenInfo(hSourceToken, TokenOwner, pOwner)
    QueryTokenInfo(hSourceToken, TokenPrimaryGroup, pPrimary)
    QueryTokenInfo(hSourceToken, TokenDefaultDacl, pDacl)
    If QueryTokenInfo(hSourceToken, TokenStatistics, pStats) = False Then Print "Query TokenStatistics failed": Return NULL
    'pStats->TokenFlags = TOKEN_HAS_TRAVERSE_PRIVILEGE Or TOKEN_ELEVATION Or TOKEN_ADMINISTRATOR

    ' === Integrity ===
    Dim pIntegritySid As PSID
    pIntegritySid = CreateIntegritySid(SECURITY_MANDATORY_SYSTEM_RID)
    If pIntegritySid = NULL Then Return NULL

    Dim pNewGroups As PTOKEN_GROUPS = BuildCorrectedTokenGroups(pGroups, pIntegritySid)
    'pNewGroups = pGroups   ' 暂时不要 AppendIntegrityGroup
    If pNewGroups = NULL Then Return NULL
    For i As Integer = 0 To pNewGroups->GroupCount - 1
        Print "Group[" & i & "]: Sid = " & pNewGroups->Groups(i).SID & " Attributes = " & pNewGroups->Groups(i).Attributes
    Next

    ' === Token Source ===
    Dim ts As TOKEN_SOURCE = InitTokenSource()
    
    ' 模拟原始令牌用户（提升NtCreateToken调用权限） 
    Dim hLsassToken As HANDLE = NULL
    'If Not CollectHighPrivToken(hLsassToken, "SeCreateTokenPrivilege") Then Return NULL
    If ImpersonateLoggedOnUser(hSourceToken) = 0 Then
        Dim As DWORD dwError = GetLastError
        Print "ImpersonateLoggedOnUser失败：" & dwError
        'Goto Cleanup 
    End If
    
    Dim expTime As LARGE_INTEGER
    expTime.QuadPart = -1 ' 永不过期
    
    Dim oa As OBJECT_ATTRIBUTES
    Dim pSD As PSECURITY_DESCRIPTOR = NULL
    ' 安全信息类型：取完整的SD（所有者+主组+DACL），这是OBJECT_ATTRIBUTES需要的最小完整集
    Const SD_INFO As SECURITY_INFORMATION = OWNER_SECURITY_INFORMATION Or GROUP_SECURITY_INFORMATION Or DACL_SECURITY_INFORMATION

    ' 调用你编写的QueryTokenSecurityInfo，获取令牌内核对象的合法SD
    pSD = QueryTokenSecurityInfo(hSourceToken, SD_INFO)
    If pSD = NULL Then
        Print "获取令牌安全描述符失败，错误码：" & GetLastError()
        Return NULL
    End If

    ' 可选：校验SD合法性（双重保障，避免无效SD）
    If IsValidSecurityDescriptor(pSD) = 0 Then
        Print "获取的安全描述符结构无效"
        HeapFree(GetProcessHeap(), 0, pSD) ' 注意释放方式（HeapAlloc对应HeapFree）
        Return NULL
    End If

    ' 初始化OBJECT_ATTRIBUTES（你的原有逻辑，完全不变）
    ZeroMemory(@oa, SizeOf(OBJECT_ATTRIBUTES))
    oa.Length = SizeOf(OBJECT_ATTRIBUTES)
    oa.SecurityDescriptor = pSD ' 赋值合法SD，彻底解决访问冲突

    ' === NtCreateToken ===
    Dim status As NTSTATUS = NtCreateToken( _
        @hNewToken, _
        TOKEN_ALL_ACCESS, _
        @oa, _
        TokenPrimary, _
        @pStats->AuthenticationId, _
        @expTime, _
        pUser, _
        pNewGroups, _
        pPrivs, _
        pOwner, _
        pPrimary, _
        pDacl, _
        @ts _
    )
    
    If Not NT_SUCCESS(status) Then
        Print "NtCreateToken失败:" & WHex(status)
        Return NULL
    End If
    
    Dim tmlSize As DWORD = SizeOf(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid)
    Dim pTml As PTOKEN_MANDATORY_LABEL = LocalAlloc(LPTR, tmlSize)
    pTml->Label.SID = pIntegritySid
    pTml->Label.Attributes = SE_GROUP_INTEGRITY
    SetTokenInformation( _
        hNewToken, _
        TokenIntegrityLevel, _
        pTml, _
        tmlSize _
        )
        
    Dim tmp As TOKEN_MANDATORY_POLICY
    tmp.Policy = TOKEN_MANDATORY_POLICY_NO_WRITE_UP
    If SetTokenInformation( _
        hNewToken, _
        TokenMandatoryPolicy, _
        @tmp, _
        SizeOf(TOKEN_MANDATORY_POLICY) _
    ) = FALSE Then
        Print "SetTokenInformation(TokenMandatoryPolicy) failed:", GetLastError()
        CloseHandle(hNewToken)
        Return NULL
    End If
    
    /'#define TokenProcessTrustLevel 41
    ' === 新增：补全Process Token TrustLevelSid（核心，解决0xC0000142）===
    Dim pTrustLevel As PTOKEN_PROCESS_TRUST_LEVEL ' 信任级别结构体指针
    ' 1. 从源SMSS令牌获取合法的TrustLevelSid（TokenProcessTrustLevel对应枚举值35）
    If QueryTokenInfo(hSourceToken, TokenProcessTrustLevel, pTrustLevel) Then
        ' 2. 将TrustLevelSid设置到新令牌
        If SetTokenInformation(hNewToken, TokenProcessTrustLevel, pTrustLevel, SizeOf(TOKEN_PROCESS_TRUST_LEVEL)) = 0 Then
            Print "设置ProcessTrustLevel失败，错误码：" & GetLastError()
        Else
            Print "Process Token TrustLevelSid设置成功（继承源SMSS令牌）"
        End If
        ' 3. 释放内存（LocalAlloc对应LocalFree，复用QueryTokenInfo的释放规则）
        LocalFree(Cast(HLOCAL, pTrustLevel))
    Else
        Print "获取源令牌ProcessTrustLevel失败，错误码：" & GetLastError()
    End If'/
    ' === TrustLevelSid补全结束 ===
    
    ' === 新增：修改令牌会话ID为1（活动控制台会话），开始 ===
    Dim As DWORD dwTargetSessionId = 1 ' 控制台用户的活动会话ID，固定为1（可通过WTSGetActiveConsoleSessionId动态获取）
    ' 可选：动态获取活动控制台会话ID，适配多会话场景
    ' dwTargetSessionId = WTSGetActiveConsoleSessionId()
    ' If dwTargetSessionId = DWORD(-1) Then dwTargetSessionId = 1

    If SetTokenInformation(hNewToken, TokenSessionId, @dwTargetSessionId, SizeOf(DWORD)) = 0 Then
        Dim As DWORD dwError = GetLastError()
        Print "修改令牌会话ID为" & dwTargetSessionId & "失败：" & WinErrorMsg(dwError) & dwError
        CloseHandle(hNewToken)
        Return NULL
    End If
    Print "令牌会话ID成功修改为：" & dwTargetSessionId
    
    /'Dim As TOKEN_ELEVATION_TYPE ElevationType
    ElevationType = TokenElevationTypeFull  ' 设置为完全提升的管理员权限
    Print "TokenElevationTypeFull = " & TokenElevationTypeFull  ' 必须输出 1
    Print "SizeOf(TOKEN_ELEVATION_TYPE) = " & SizeOf(TOKEN_ELEVATION_TYPE)  ' 必须输出 4（INT 4字节）
    If SetTokenInformation(hNewToken, TokenElevationType, @ElevationType, SizeOf(TOKEN_ELEVATION_TYPE)) = 0 Then
        Print "SetTokenInformation(TokenElevationType) failed:", GetLastError()
        CloseHandle(hNewToken)
        Return NULL
    End If'/
    
    ' 初始化 TokenElevation 结构体
    Dim As TOKEN_ELEVATION te
    te.TokenIsElevated = True  ' 标记为已提升

    ' 调用 SetTokenInformation 设置提升状态
    /'If SetTokenInformation( _
        hNewToken, _               ' 你的定制令牌句柄
        TokenElevation, _          ' 信息类：TokenElevation
        @te, _                     ' 结构体指针
        SizeOf(te) _  ' 结构体大小
    ) = 0 Then
        Print "SetTokenInformation(TokenElevation) 失败：" & GetLastError()
        CloseHandle(hNewToken)
        Return NULL
    End If '/
    If Not QueryTokenInfo(hSourceToken, TokenElevation, @te) Then Return NULL
    Print "Source Elevation:" & te.TokenIsElevated
    If Not QueryTokenInfo(hNewToken, TokenElevation, @te) Then Return NULL
    Print "New Elevation:" & te.TokenIsElevated
    
    Dim As DWORD cb = 0
    Dim As TOKEN_ELEVATION_TYPE ElevationType
    GetTokenInformation(hSourceToken, TokenElevationType, NULL, 0, @cb)
    If GetTokenInformation(hSourceToken, TokenElevationType, @ElevationType, cb, @cb) = False Then Return NULL
    Print "Source ElevationType:" & ElevationType
    GetTokenInformation(hNewToken, TokenElevationType, NULL, 0, @cb)
    If GetTokenInformation(hNewToken, TokenElevationType, @ElevationType, cb, @cb) = False Then Return NULL
    Print "New ElevationType:" & ElevationType
    Print "hSourceToken:0x" & WHex(hSourceToken)
    Print "hNewToken:0x" & WHex(hNewToken)
    'AfxMsg "Pause"
    ' 新增：还原到当前进程原始身份（关键，模拟后必须还原）
    RevertToSelf
    
    Return hNewToken
End Function

' ============================================================
' 修改后的 CreateHighPrivCMD - 支持任意命令行执行
' ============================================================
Function CreateHighPrivProcess( _
    ByVal hCustomToken As HANDLE, _
    ByVal lpApplicationName As LPCWSTR = NULL, _
    ByVal lpCommandLine As LPWSTR = NULL, _      ' ← 新增：要执行的命令行（可为NULL则默认cmd）
    ByVal lpWorkDir As Wstring Ptr = NULL, _           ' ← 新增：工作目录（可为NULL则使用当前目录）
    ByVal dwCreationFlags As DWORD = 0 _               ' ← 新增：额外的创建标志
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
        ' 注意：如果命令包含空格且不是完整路径，需要处理
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
        Print "DuplicateTokenEx(模拟令牌)失败：" & WinErrorMsg(dwError) & dwError
        GoTo Cleanup
    End If
    
    If DuplicateTokenEx(hCustomToken, TOKEN_ALL_ACCESS, NULL, _
        SecurityImpersonation, TokenPrimary, @hDupTokenForProcess) = 0 Then
        dwError = GetLastError
        Print "DuplicateTokenEx(主令牌)失败：" & WinErrorMsg(dwError) & dwError
        GoTo Cleanup
    End If
    
    RevertToSelf
    Print "线程身份已还原"
    
    ' ========================================================
    ' 4. 构建创建标志（合并用户传入的标志）
    ' ========================================================
    dwFlags Or= dwCreationFlags  ' 合并用户自定义标志（如 CREATE_SUSPENDED）
    
    ' ========================================================
    ' 5. 创建环境块（保持原有逻辑）
    ' ========================================================
    If CreateEnvironmentBlock(@lpEnvBlock, hDupTokenForProcess, True) = 0 Then
        dwError = GetLastError
        Print "CreateEnvironmentBlock失败," & WinErrorMsg(dwError) & dwError
        Goto Cleanup
    End If
    
    Print "hCustomToken:0x" & WHex(hCustomToken) & " hDupTokenForProcess:0x" & WHex(hDupTokenForProcess)
    AfxMsg "Pause"
    
    ' ========================================================
    ' 6. 创建进程（关键修改：使用 szCmdLine 而非硬编码）
    ' ========================================================
    ' 方案A: CreateProcessWithTokenW（需要 SeImpersonatePrivilege）
    If CreateProcessWithTokenW( _
        hDupTokenForProcess, _
        LOGON_WITH_PROFILE, _
        lpApplicationName, _                                    ' ← 程序路径为NULL，从命令行解析
        lpCommandLine, _                              ' ← 使用动态命令行
        dwFlags, _
        lpEnvBlock, _
        pWorkDir, _                                ' ← 使用动态工作目录
        @si, _
        @pi _
    ) <> 0 Then
        
        dwProcessId = pi.dwProcessId
        Print "高权限进程创建成功，PID=" & dwProcessId & " 命令: " & szCmdLine
        
        CloseHandle(pi.hThread)
        CloseHandle(pi.hProcess)
        
    Else
        dwError = GetLastError
        Print "CreateProcessWithTokenW失败：" & WinErrorMsg(dwError) & dwError
        
        ' ========================================================
        ' 7. 备用方案: CreateProcessAsUserW（保持原有逻辑）
        ' ========================================================
        If dwError = ERROR_PRIVILEGE_NOT_HELD Then
            Print "尝试使用CreateProcessAsUserW..."
            
            If CreateProcessAsUserW( _
                hDupTokenForProcess, _
                NULL, _                            ' ← 程序路径为NULL，从命令行解析
                @szCmdLine, _                      ' ← 使用动态命令行
                NULL, NULL, FALSE, _
                dwFlags, _
                lpEnvBlock, _
                pWorkDir, _                        ' ← 使用动态工作目录
                @si, _
                @pi _
            ) <> 0 Then
                
                dwProcessId = pi.dwProcessId
                Print "备用方案成功，PID=" & dwProcessId & " 命令: " & szCmdLine
                CloseHandle(pi.hThread)
                CloseHandle(pi.hProcess)
                
            Else
                dwError = GetLastError
                Print "CreateProcessAsUserW也失败：" & dwError
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
' 主函数：串联所有流程
' -----------------------------------------------------------------------------
Function CreateSYSTEMPriviegeProcess( _
    ByVal lpApplicationName As Wstring Ptr = NULL, _
    ByVal lpCommandLine As Wstring Ptr = NULL, _      ' ← 新增：要执行的命令行（可为NULL则默认cmd）
    ByVal lpWorkDir As Wstring Ptr = NULL, _           ' ← 新增：工作目录（可为NULL则使用当前目录）
    ByVal dwCreationFlags As DWORD = 0 _ ' ← 新增：额外的创建标志
    ) As Boolean
    
    Function = True
    Dim As HANDLE hOriginalToken = NULL, hCustomToken = NULL
    Dim As DWORD dwCmdPid = 0
    
    Print "=== 高权限CMD创建工具 ==="
    Print "开始采集高权限令牌..."
    
    ' 步骤1：采集高权限原始令牌
    If Not CollectHighPrivToken(hOriginalToken, "SeTcbPrivilege") Then
        Print "采集高权限令牌失败！"
        Function = False
        GoTo Cleanup
    End If
    Print "高权限令牌采集成功！"
    If Not EnablePrivilege(hOriginalToken, "SeCreateTokenPrivilege") Then Return NULL
    
    ' 步骤2：定制化创建新令牌
    Print "开始定制高权限令牌..."
    hCustomToken = CreateCustomToken(hOriginalToken) ' hOriginalToken
    If hCustomToken = NULL Then
        Print "定制高权限令牌失败！"
        Function = False
        GoTo Cleanup
    End If
    Print "高权限令牌定制成功！"
    
    ' 步骤3：创建高权限CMD
    Print "开始创建高权限CMD..."
    dwCmdPid = CreateHighPrivProcess(hCustomToken, lpApplicationName, lpCommandLine, lpWorkDir, dwCreationFlags)
    If dwCmdPid = 0 Then
        Print "创建高权限CMD失败！"
        Function = False
    End If

Cleanup:
    ' 资源释放
    If hOriginalToken Then CloseHandle(hOriginalToken)
    If hCustomToken Then CloseHandle(hCustomToken)
End Function