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

Function AddUnlockFileMenu() As BOOLEAN
    Dim KeyHandle As HKEY
    Dim result As Long

    ' 创建注册表项
    Dim strName As WString * MAX_PATH
    strName = "*\shell\SnowSword"
    result = RegCreateKey(HKEY_CLASSES_ROOT, @strName, @KeyHandle)
    If result = ERROR_SUCCESS Then
        ' 设置默认值
        strName = "UnlockFile By SnowSword"
        result = RegSetValueExW(KeyHandle, vbNullString, 0, REG_SZ, CPtr(Const UByte Ptr, @strName), LenW(strName))
        ' 设置命令
        Dim strCommand As WString * MAX_PATH = App.Path & App.EXEName & !" -UnlockFile \"%1\""
        result = RegSetValueExW(KeyHandle, "command", 0, REG_SZ, CPtr(Const UByte Ptr, @strCommand), MAX_PATH)
        ' 关闭注册表项
        RegCloseKey KeyHandle
        Return True
    Else
        Return False
    End If
End Function