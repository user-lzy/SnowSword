Enum SHUTDOWN_ACTION
    ShutdownNoReboot '关闭系统
    ShutdownReboot '重启
    ShutdownPowerOff '关闭系统并关闭电源
    ShutdownForceReboot '强制重启
End Enum

Private Enum SystemPowerState
    PowerSystemUnspecified
    PowerSystemWorking
    PowerSystemSleeping1
    PowerSystemSleeping2
    PowerSystemSleeping3
    PowerSystemHibernate
    PowerSystemShutdown
    PowerSystemMaximum
End Enum
    
Private Enum PowerAction
    PowerActionNone
    PowerActionReserved
    PowerActionSleep
    PowerActionHibernate
    PowerActionShutdown
    PowerActionShutdownReset
    PowerActionShutdownOff
    PowerActionWarmEject
End Enum

Declare Function ZwShutdownSystem Lib "ntdll.dll" Alias "ZwShutdownSystem"(ByVal SystemAction As SHUTDOWN_ACTION) As NTSTATUS
Declare Function ZwInitiatePowerAction Lib "ntdll.dll" Alias "ZwInitiatePowerAction"(ByVal SystemAction As PowerAction, ByVal MinSystemState As SystemPowerState, ByVal FLAGS As Long, ByVal bFlag As BOOLEAN) As Long
Declare Function ZwSetSystemPowerState Lib "ntdll.dll" Alias "ZwSetSystemPowerState"(ByVal SystemAction As PowerAction, ByVal MinSystemState As SystemPowerState, ByVal FLAGS As Long) As Long

Sub MyShutdown()
    Dim ret As BOOLEAN = NT_SUCCESS(ZwInitiatePowerAction(PowerActionShutdown, PowerSystemShutdown, 0, True)) Or _
    NT_SUCCESS(ZwSetSystemPowerState(PowerActionShutdown, PowerSystemShutdown, 0)) Or _
    NT_SUCCESS(ZwShutdownSystem(ShutdownNoReboot))
    If ret Then AfxMsg "普通关机成功!", "提示" Else AfxMsg "普通关机失败!", "提示"
End Sub

Sub ForceShutdown()
    Dim ret As BOOLEAN = NT_SUCCESS(ZwInitiatePowerAction(PowerActionShutdownOff, PowerSystemShutdown, 0, True)) Or _
    NT_SUCCESS(ZwSetSystemPowerState(PowerActionShutdownOff, PowerSystemShutdown, 0)) Or _
    NT_SUCCESS(ZwShutdownSystem(ShutdownPowerOff))
    If ret Then AfxMsg "强制关机成功!", "提示" Else AfxMsg "强制关机失败!", "提示"
End Sub

Sub Reboot()
    Dim ret As BOOLEAN = NT_SUCCESS(ZwInitiatePowerAction(PowerActionShutdownReset, PowerSystemShutdown, 0, True)) Or _
    NT_SUCCESS(ZwSetSystemPowerState(PowerActionShutdownReset, PowerSystemShutdown, 0)) Or _
    NT_SUCCESS(ZwShutdownSystem(ShutdownReboot))
    If ret Then AfxMsg "普通重启成功!", "提示" Else AfxMsg "普通重启失败!", "提示"
End Sub

Sub ForceReboot()
    Dim st As NTSTATUS = ZwShutdownSystem(ShutdownForceReboot)
    If NT_SUCCESS(st) Then
        AfxMsg "强制重启成功!", "提示"
    Else
        AfxMsg "强制重启失败!", "提示"
        Print "[ForceReboot]ZwShutdownSystem" & Hex(st)
    End If
End Sub

Function AddUnlockFileMenu() As Boolean
    Dim hKey As HKEY
    Dim hCmdKey As HKEY
    Dim ret As Long

    ' 创建 *\shell\SnowSword
    Dim keyPath As WString * MAX_PATH
    keyPath = "*\shell\SnowSword"
    ret = RegCreateKeyW(HKEY_CLASSES_ROOT, @keyPath, @hKey)
    If ret <> ERROR_SUCCESS Then Return False

    ' 默认显示名
    Dim menuText As WString * 260
    menuText = "UnlockFile By SnowSword"
    RegSetValueExW(hKey, NULL, 0, REG_SZ, _
                   CPtr(Const BYTE Ptr, @menuText), _
                   (Len(menuText) + 1) * 2)

    ' ★★★ 核心：声明需要管理员权限 ★★★
    RegSetValueExW(hKey, "HasLUAShield", 0, REG_SZ, _
                   NULL, 0)

    ' 创建 command 子键
    ret = RegCreateKeyW(hKey, "command", @hCmdKey)
    If ret <> ERROR_SUCCESS Then
        RegCloseKey hKey
        Return False
    End If

    ' 命令行
    Dim cmd As WString * 520
    cmd = """" & App.Path & "\" & App.EXEName & """" & _
          " -UnlockFile ""%1"""

    RegSetValueExW(hCmdKey, NULL, 0, REG_SZ, _
                   CPtr(Const BYTE Ptr, @cmd), _
                   (Len(cmd) + 1) * 2)

    RegCloseKey hCmdKey
    RegCloseKey hKey

    Return True
End Function