#include "win/aclapi.bi"
Dim Shared pInlineHook As Any Ptr
Dim Shared pSnowSword As Any Ptr
Dim Shared DenyInjectDll As Function() As Long
Dim Shared AccessInjectDll As Function() As Long
Dim Shared ShieldMsgHook As Function(ByVal As Boolean) As Boolean

Dim Shared nextHook As Any Ptr = NULL

' 获取完整的 DLL 路径（模拟 Windows 搜索机制）
Function GetRealDllPath(ByVal dllName As String) As String
    Dim As String result
    Dim As UInteger bufferSize = MAX_PATH
    Dim As WString * MAX_PATH buffer
    
    ' 规范化文件名
    If InStr(dllName, ".") = 0 Then dllName &= ".dll"
    
    ' 如果已为完整路径，直接返回
    If InStr(dllName, "\") > 0 OrElse InStr(dllName, "/") > 0 Then
        GetFullPathName(dllName, bufferSize, @buffer, NULL)
        result = buffer
        If FileExists(result) Then
            'Deallocate(buffer)
            Return result
        End If
    End If
    
    ' 1. 搜索应用程序所在目录
    bufferSize = GetModuleFileName(NULL, @buffer, MAX_PATH)
    If bufferSize > 0 Then
        Dim As String strExePath = buffer
        Dim As Integer lastSlash = InStrRev(strExePath, "\")
        If lastSlash > 0 Then
            strExePath = Left(strExePath, lastSlash)
            result = strExePath & dllName
            If FileExists(result) Then
                'Deallocate(buffer)
                Return result
            End If
        End If
    End If
    
    ' 2. 搜索当前目录
    If GetCurrentDirectory(MAX_PATH, @buffer) > 0 Then
        'Print "buffer1:" & buffer
        result = buffer & "\" & dllName
        If FileExists(result) Then
            'Deallocate(buffer)
            Return result
        End If
    End If
    
    ' 3. 搜索系统目录
    If GetSystemDirectory(@buffer, MAX_PATH) > 0 Then
        'Print "buffer2:" & buffer
        result = buffer & "\" & dllName
        If FileExists(result) Then
            'Deallocate(buffer)
            Return result
        End If
    End If
    
    ' 4. 搜索 Windows 目录
    If GetWindowsDirectory(@buffer, MAX_PATH) > 0 Then
        'Print "buffer3:" & buffer
        result = buffer & "\" & dllName
        If FileExists(result) Then
            'Deallocate(buffer)
            Return result
        End If
    End If
    
    ' 5. 搜索 PATH 环境变量中的目录
    Dim As ULong pathLen = GetEnvironmentVariable("PATH", NULL, 0)
    If pathLen > 0 Then
        Dim As WString Ptr pathBuffer = Allocate((pathLen + 1) * 2)
        If GetEnvironmentVariable("PATH", pathBuffer, pathLen) > 0 Then
            'Print "pathBuffer:" & *pathBuffer
            Dim As String pathStr = *pathBuffer
            Dim As Integer startPos = 1
            Dim As Integer endPos
            Dim As String pathEntry
            
            Do While startPos <= Len(pathStr)
                endPos = InStr(startPos, pathStr, ";")
                If endPos = 0 Then endPos = Len(pathStr) + 1
                
                pathEntry = Mid(pathStr, startPos, endPos - startPos)
                pathEntry = Trim(pathEntry)
                
                If Len(pathEntry) > 0 Then
                    If Right(pathEntry, 1) = "\" Then result = pathEntry & dllName Else result = pathEntry & "\" & dllName
                    If FileExists(result) Then
                        Deallocate(pathBuffer)
                        'Deallocate(buffer)
                        Return result
                    End If
                End If
                
                startPos = endPos + 1
            Loop
        End If
        Deallocate(pathBuffer)
    End If
    
    'Deallocate(buffer)
    Return ""  ' 未找到文件
End Function

Function FakeLoadLibraryExW(ByVal lpLibFileName As LPCWSTR, ByVal hFile As HANDLE, ByVal dwFlags As DWORD) As HMODULE
    'Print "Enter FakeLoadLibraryExW, dwFlags=" & dwFlags
    Const nLen As Long = 100
    Dim fullPath As WString * MAX_PATH, filePath As WString * MAX_PATH
    
    Dim OldLoadLibraryExW As Function(ByVal As LPCWSTR, ByVal As HANDLE, ByVal As DWORD) As HMODULE = nextHook
    
    If (dwFlags And LOAD_LIBRARY_AS_DATAFILE <> 0) OrElse (dwFlags And LOAD_LIBRARY_AS_DATAFILE <> 0) Then Return OldLoadLibraryExW(lpLibFileName, hFile, dwFlags) ' 不拦截
    If lpLibFileName <> NULL Then
        filePath = *lpLibFileName
        If InStr( *lpLibFileName, "\") = 0 Then
            filePath = GetRealDllPath(*lpLibFileName)
            'Print fullPath
        End If
        Print "FilePath:" & filePath
        If InStr(UCase(filePath), UCase(Environ("SystemRoot"))) > 0 Then Return OldLoadLibraryExW(lpLibFileName, hFile, dwFlags)
        If AfxMsg("检测到dll:" & filePath & "正在加载,是否拦截?",, MB_YESNO) = IDYES Then
            Return NULL
        Else
            Return OldLoadLibraryExW(lpLibFileName, hFile, dwFlags)
        End If
    Else
        Return OldLoadLibraryExW(lpLibFileName, hFile, dwFlags)
    End If
End Function

Sub InitSelfProtect()
    /'pInlineHook = LoadLibraryEx(App.Path & "InlineHook.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    DenyInjectDll = Cast(Function() As Long, GetProcAddress(pInlineHook, "DenyInjectDll"))
    AccessInjectDll = Cast(Function() As Long, GetProcAddress(pInlineHook, "AccessInjectDll"))
    ShieldMsgHook = Cast(Function(ByVal As BOOLEAN) As BOOLEAN, GetProcAddress(pInlineHook, "ShieldMsgHook"))
    AfxMsg (DenyInjectDll=0)
    AfxMsg ShieldMsgHook(True) '/
    /'pSnowSword = LoadLibraryEx(App.Path & "SnowSword.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
    Print App.Path & "SnowSword.dll"
    If pSnowSword = NULL Then AfxMsg "Dll加载失败!"
    Dim HookLoadLibraryExW As Sub(ByVal As PVOID) = DyLibSymbol(pSnowSword, "HookLoadLibraryExW")
    Print "HookLoadLibraryExW:0x" & WHex(HookLoadLibraryExW)'/
    nextHook = Hook_API("kernelbase.dll", "LoadLibraryExW", CPtr(Any Ptr, @FakeLoadLibraryExW))
    If nextHook = NULL Then AfxMsg "Hook LoadLibraryExW失败"
End Sub

Sub UninitSelfProtect()
    /'AccessInjectDll()
    ShieldMsgHook(False)
    FreeLibrary pInlineHook '/
    /'Dim UnHookLoadLibraryExW As Sub() = DyLibSymbol(pSnowSword, "UnHookLoadLibraryExW")
    If UnHookLoadLibraryExW <> NULL Then UnHookLoadLibraryExW() Else AfxMsg "UnHookLoadLibraryExW = NULL"'/
    UnHook_API nextHook
End Sub

Sub Cleaner(TkHandle As HANDLE, ProcACL As PACL, UserTk As PTOKEN_USER)
    If TkHandle <> 0 Then
        CloseHandle(TkHandle)
    END IF
    If ProcACL <> 0 Then
        free(ProcACL)
    End If
    If UserTk <> 0 Then
        free(UserTk)
    End If
End Sub

Function SetSecurityControls() As Boolean
    Dim TkHandle As HANDLE = 0
    Dim UserTk As PTOKEN_USER = 0
    Dim ProcACL As PACL = 0
    Dim CbBuffer As DWORD = 0
    Dim CbACL As DWORD = 0

    If OpenProcessToken(GetCurrentProcess, TOKEN_QUERY, @TkHandle) = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "OpenProcessToken:" & WinErrorMsg(GetLastError)
        Return False
    End If

    GetTokenInformation TkHandle, TokenUser, 0, 0, @CbBuffer
    UserTk = CAllocate(CbBuffer)
    If UserTk = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "分配内存失败"
        Return False
    End If

    If GetTokenInformation(TkHandle, TokenUser, UserTk, CbBuffer, @CbBuffer) = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "GetTokenInformation:" & WinErrorMsg(GetLastError)
        Return False
    End If

    If IsValidSid(UserTk->User.SID) = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "用户SID无效"
        Return False
    End If

    CbACL = SizeOf(ACL) + SizeOf(ACCESS_DENIED_ACE) + GetLengthSid(UserTk->User.SID)
    ProcACL = CAllocate(CbACL)
    If ProcACL = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "分配内存失败"
        Return False
    End If

    If InitializeAcl(ProcACL, CbACL, ACL_REVISION) = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "InitializeAcl:" & WinErrorMsg(GetLastError)
        Return False
    End If

    If AddAccessDeniedAce(ProcACL, ACL_REVISION, PROCESS_ALL_ACCESS, UserTk->User.SID) = 0 Then
        Cleaner TkHandle, ProcACL, UserTk
        Print "AddAccessDeniedAce:" & WinErrorMsg(GetLastError)
        Return False
    End If

    If SetSecurityInfo(GetCurrentProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, ProcACL, 0) = ERROR_SUCCESS Then
        Print "Successfully set security controls."
    Else
        Print "SetSecurityInfo:" & WinErrorMsg(GetLastError)
    End If

    Cleaner TkHandle, 0, UserTk
    Deallocate ProcACL
    Return True
End Function
