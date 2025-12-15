#include Once "win/winsvc.bi"
#include Once "win/winioctl.bi"
#include Once "win/winerror.bi"

Type PUNICODE_STRING As UNICODE_STRING Ptr
#define STATUS_IMAGE_ALREADY_LOADED &HC000010E

Function OpenDrv(ByVal szDrvLinkName As String) As HANDLE
    Dim hDrvHandle As HANDLE
    hDrvHandle = CreateFile(szDrvLinkName, GENERIC_READ Or GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
    If (hDrvHandle = INVALID_HANDLE_VALUE) Then Print WinErrorMsg(GetLastError) & "(CreateFile)"
    Return hDrvHandle
End Function

Function IoControl(ByVal hDrvHandle As HANDLE, ByVal dwIoControlCode As DWORD, ByVal lpInBuffer As LPVOID = NULL, ByVal nInBufferSize As DWORD = 0, ByVal lpOutBuffer As LPVOID = NULL, ByVal nOutBufferSize As DWORD = 0, ByVal lpBytesReturned As LPDWORD = NULL) As WINBOOL
    Dim lDrvRetSize As DWORD, ret As WINBOOL
    ret = DeviceIoControl(hDrvHandle, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, @lDrvRetSize, 0)
    If lpBytesReturned <> NULL Then *lpBytesReturned = lDrvRetSize
    Return ret
End Function

Sub CloseDrv(ByVal hDrvHandle As HANDLE)
    ZwClose hDrvHandle
End Sub

Function CTL_CODE_GEN(ByVal lngFunction As Long) As Long
    Return (FILE_DEVICE_UNKNOWN * (2 ^ 16)) Or (FILE_ANY_ACCESS * (2 ^ 14)) Or (lngFunction * (2 ^ 2)) Or METHOD_BUFFERED
End Function

/'// 删除注册表项及其所有子项
void DeleteRegistryKey(HKEY hKeyRoot, LPCWSTR subKey) {
    HKEY hKey;
    DWORD retCode;

    // 打开注册表项
    retCode = RegOpenKeyExW(hKeyRoot, subKey, 0, KEY_ALL_ACCESS, &hKey);
    if (retCode == ERROR_SUCCESS) {
        TCHAR path[2048];
        DWORD pathLen = 2048;
        FILETIME ftLastWriteTime;

        // 枚举并删除子项
        for (DWORD i = 0; ; i++) {
            pathLen = sizeof(path) / sizeof(TCHAR);
            retCode = RegEnumKeyExW(hKey, i, path, &pathLen, NULL, NULL, NULL, &ftLastWriteTime);
            if (retCode == ERROR_SUCCESS) {
                // 递归删除子项
                DeleteRegistryKey(hKey, path);
            } else if (retCode == ERROR_NO_MORE_ITEMS) {
                break;
            } else {
                printf("Error enumerating registry keys: %d\n", retCode);
                break;
            }
        }

        // 删除当前项
        retCode = RegDeleteKeyW(hKeyRoot, subKey);
        if (retCode != ERROR_SUCCESS) {
            printf("Failed to delete registry key: %d\n", retCode);
        }

        RegCloseKey(hKey);
    } else {
        printf("Failed to open registry key: %d\n", retCode);
    }
End Sub'/

Function LoadDriver(ByVal szDrvPath As String, ByVal IsMiniFilter As Boolean) As Boolean
    Function = True
    If Not IsAdmin Then
        If AfxMsg("需要管理员权限,是否重启进程以获取?", "提示", MB_YESNO) = IDYES Then
            RestartasAdmin True
        Else
            Return False
        End If
    End If
    Dim hNtdll As HMODULE = DyLibLoad("ntdll.dll")
    If (hNtdll = NULL) Then
        Print "DyLibLoad(NTDLL.DLL) Error: " & GetLastError
        Return False
    End If
    Dim hFltdll As HMODULE = DyLibLoad("FltLib.dll")
    If (hNtdll = NULL) Then
        Print "DyLibLoad(FltLib.dll) Error: " & GetLastError
        Return False
    End If

    Dim ZwLoadDriver As Function(ByVal As PUNICODE_STRING) As NTSTATUS = DyLibSymbol(hNtdll, "ZwLoadDriver")
    Dim FilterLoad As Function(ByVal As LPWSTR) As LRESULT = DyLibSymbol(hFltdll, "FilterLoad")
    'Dim RtlFreeUnicodeString As Sub(ByVal As PUNICODE_STRING) = DyLibSymbol(hNtdll, "RtlFreeUnicodeString")

    If (ZwLoadDriver = NULL) Then
        Print "DyLibSymbol Error: " & GetLastError
        DyLibFree hNtdll
        Return False
    End If
    
    Dim driverPath As StringW
    If LeftW(szDrvPath, 2) = "\\" OrElse LeftW(szDrvPath, 2) = "//" Then '如果是网络路径
        driverPath = "C:\Windows\System32\drivers\" & RightW(szDrvPath, LenW(szDrvPath) - InStrRevW(szDrvPath, "\"))
        Print driverPath
        CopyFileW szDrvPath, StrPtrW(driverPath), False
        szDrvPath = driverPath
    End If

    '构建注册表项路径
    Dim szDrvName As String = Mid(szDrvPath, InStrRev(szDrvPath, "\") + 1, InStrRev(szDrvPath, ".") - InStrRev(szDrvPath, "\") - 1)
    Dim szSubKey As Const WString * 260 = "SYSTEM\\CurrentControlSet\\Services\\" & szDrvName
    Dim szSubPath As Const WString * 260 = "\Registry\Machine\System\CurrentControlSet\Services\" & szDrvName

    '创建服务项
    Dim KeyHandle As HKEY
    Dim dwDisposition As DWORD
    Dim ret As Long = RegCreateKeyEx(HKEY_LOCAL_MACHINE, @szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, @KeyHandle, @dwDisposition)
    If (ret <> ERROR_SUCCESS) Then
        Print " Error: " & WinErrorMsg(ret) & "(" & ret & ")"
        DyLibFree hNtdll
        DyLibFree hFltdll
        Return False
    End If

    ' 设置服务项的键值
    Dim dwType As DWORD = 1 ' SERVICE_KERNEL_DRIVER
    Dim dwStart As DWORD = 3 ' SERVICE_DEMAND_START
    Dim dwErrorControl As DWORD = 1 ' SERVICE_ERROR_NORMAL
    Dim strGroup As WString * 30 = "FSFilter Activity Monitor"
    Dim DependOnService As WString * 10 = "FltMgr"

    RegSetValueEx KeyHandle, "Type", 0, REG_DWORD, Cast(Const PBYTE,@dwType), SizeOf(dwType)
    RegSetValueEx KeyHandle, "Start", 0, REG_DWORD, Cast(Const PBYTE,@dwStart), SizeOf(dwStart)
    RegSetValueEx KeyHandle, "ErrorControl", 0, REG_DWORD, Cast(Const PBYTE, @dwErrorControl), SizeOf(dwErrorControl)
    RegSetValueEx KeyHandle, "Group", 0, REG_SZ, Cast(Const PBYTE,@strGroup), SizeOf(strGroup)
    RegSetValueEx KeyHandle, "DependOnService", 0, REG_SZ, Cast(Const PBYTE,@DependOnService), SizeOf(DependOnService)

    Dim szImagePath As WString * 256 = "\??\" & szDrvPath
    'AfxMsg szImagePath & Len(szImagePath)
    RegSetValueEx KeyHandle, "ImagePath", 0, REG_SZ, Cast(Const PBYTE, @szImagePath), Len(szImagePath) * 2
    RegCloseKey KeyHandle
    '-------------------MiniFilter部分-------------------
    Dim szSubKey1 As Const WString * 260 = "SYSTEM\\CurrentControlSet\\Services\\" & szDrvName & "\\Instances"
    Dim InstanceName As Const WString * 20 = szDrvName & " Instance"
    ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, @szSubKey1, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, @KeyHandle, @dwDisposition)
    If (ret <> ERROR_SUCCESS) Then
        Print " Error: " & WinErrorMsg(ret) & "(" & ret & ")"
        DyLibFree hNtdll
        DyLibFree hFltdll
        Return False
    End If
    RegSetValueEx KeyHandle, "DefaultInstance", 0, REG_SZ, Cast(Const PBYTE, @InstanceName), SizeOf(InstanceName)
    RegCloseKey KeyHandle
    
    Dim szSubKey2 As Const WString * 260 = "SYSTEM\\CurrentControlSet\\Services\\" & szDrvName & "\\Instances\\" & InstanceName
    ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, @szSubKey2, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, @KeyHandle, @dwDisposition)
    If (ret <> ERROR_SUCCESS) Then
        Print " Error: " & WinErrorMsg(ret) & "(" & ret & ")"
        DyLibFree hNtdll
        DyLibFree hFltdll
        Return False
    End If
    '设置 Altitude
    Dim Altitude As WString * 20 = "370020"
    ret = RegSetValueEx(KeyHandle, "Altitude", 0, REG_SZ, Cast(Const PBYTE, @Altitude), SizeOf(Altitude))
    'If (ret <> ERROR_SUCCESS) Then Print "Failed to set Altitude: " & ret

    '设置 Flags
    Dim dwFlags As DWORD = 0
    ret = RegSetValueEx(KeyHandle, "Flags", 0, REG_DWORD, Cast(Const PBYTE, @dwFlags), SizeOf(dwFlags))
    'If (ret <> ERROR_SUCCESS) Then Print "Failed to set Flags: " & ret
    RegCloseKey KeyHandle
    
    ' 构建 UNICODE_STRING
    Dim uDriverServiceName As UNICODE_STRING
    RtlInitUnicodeString @uDriverServiceName, @szSubPath

    ' 加载驱动
    If (IsMiniFilter) Then
        Dim FilterName As WString * 20 = szDrvName
        Dim result As HRESULT = FilterLoad(@FilterName)
        If (result <> S_OK) Then
            Print "FilterLoad Error:" & Hex(result)
            DyLibFree hNtdll
            DyLibFree hFltdll
            Return False
        End If
    Else
        Dim st As ULong = ZwLoadDriver(@uDriverServiceName)
        If Not (NT_SUCCESS(st) Or st = STATUS_IMAGE_ALREADY_LOADED) Then
            Print "ZwLoadDriver Error: " & WinErrorMsg(st) & "(" & WStr(Hex(st)) & ")"
            Function = False
        End If
        'RtlFreeUnicodeString(@uDriverServiceName)'为什么不能去掉括号?
    End If
    DyLibFree hNtdll
    DyLibFree hFltdll
End Function

Public Function UnloadDriver(ByVal szDrvPath As String, ByVal IsMiniFilter As Boolean) As Boolean
    Dim szDrvName As String = Mid(szDrvPath, InStrRev(szDrvPath, "\") + 1, InStrRev(szDrvPath, ".") - InStrRev(szDrvPath, "\") - 1)
    Dim szSubKey1 As Const WString * 260 = "SYSTEM\\CurrentControlSet\\Services\\" & szDrvName
    Dim szSubKey2 As Const WString * 260 = "SYSTEM\\CurrentControlSet\\Services"
    Dim szSubPath As Const WString * 260 = "\Registry\Machine\System\CurrentControlSet\Services\\" & szDrvName
    Dim uDriverServiceName As UNICODE_STRING
    Dim hNtdll As HMODULE = DyLibLoad("ntdll.dll")
    Dim hFltdll As HMODULE = DyLibLoad("FltLib.dll")
    Dim ret As ULong
    If (hNtdll = NULL) Then
        Print "DyLibLoad(NTDLL.DLL) Error: " & GetLastError
        Return False
    End If
    If (hFltdll = NULL) Then
        Print "DyLibLoad(FltLib.dll) Error: " & GetLastError
        DyLibFree hNtdll
        Return False
    End If
    
    Dim FilterUnload As Function(ByVal As LPWSTR) As LRESULT = DyLibSymbol(hFltdll, "FilterUnload")
    
    Dim ZwUnloadDriver As Function(ByVal As PUNICODE_STRING) As NTSTATUS = DyLibSymbol(hNtdll, "ZwUnloadDriver")
    'Dim RtlFreeUnicodeString As Sub(ByVal As PUNICODE_STRING) = DyLibSymbol(hNtdll, "RtlFreeUnicodeString")
    RtlInitUnicodeString @uDriverServiceName, @szSubPath
    '卸载驱动
    If (IsMiniFilter) Then
        Dim FilterName As WString * 20 = szDrvName
        Dim result As HRESULT = FilterUnload(@FilterName)
        If (result <> S_OK) Then
            Print "FilterUnload Error:" & Hex(result)
            DyLibFree hNtdll
            DyLibFree hFltdll
            Return False
        End If
    Else
        ret = ZwUnloadDriver(@uDriverServiceName)
        If (ret <> 0) Then Print "ZwUnloadDriver Error: " & WinErrorMsg(ret) & "(" & WStr(Hex(ret)) & ")"
    End If
    '删除注册表信息
    RegDeleteKey HKEY_LOCAL_MACHINE, szSubKey1
    RegDeleteKey HKEY_LOCAL_MACHINE, szSubKey2
    'RtlFreeUnicodeString(@uDriverServiceName)
    DyLibFree hNtdll
    DyLibFree hFltdll
    Return (ret = 0)
End Function

