#include Once "win/winreg.bi"

Declare Function RegDeleteTree Lib "Advapi32.dll" Alias "RegDeleteTreeW"(ByVal HKEY As HKEY, ByVal lpSubKey As LPCWSTR) As Long

Type QWORD As ULongInt

Private Function GetHKEYFromStr(ByVal strHKEY As String) As HKEY
    Select Case strHKEY
        Case "HKEY_CLASSES_ROOT"
            Return HKEY_CLASSES_ROOT
        Case "HKEY_LOCAL_MACHINE"
            Return HKEY_LOCAL_MACHINE
        Case "HKEY_USERS"
            Return HKEY_USERS
        Case "HKEY_CURRENT_USER"
            Return HKEY_CURRENT_USER
        Case "HKEY_CURRENT_CONFIG"
            Return HKEY_CURRENT_CONFIG
    End Select
End Function

Function GetRegPathByNodeW(Node As HTREEITEM, treKey As Class_TreeView, ByRef RegPath As StringW) As HKEY
    Dim CurrentNode As HTREEITEM = Node
    Do While True
        If (treKey.GetParent(CurrentNode) = NULL) Then
            Function = GetHKEYFromStr(treKey.Text(CurrentNode))
            Exit Do
        End If
        RegPath = treKey.Text(CurrentNode) & "\" & RegPath
        'Print treKey.Text(CurrentNode)
        CurrentNode = treKey.GetParent(CurrentNode)
    Loop
End Function

Public Sub GetRootList(treKey As Class_TreeView)
    treKey.AddItem TVI_ROOT, "HKEY_CLASSES_ROOT"
    treKey.AddItem TVI_ROOT, "HKEY_CURRENT_USER"
    treKey.AddItem TVI_ROOT, "HKEY_LOCAL_MACHINE"
    treKey.AddItem TVI_ROOT, "HKEY_USERS"
    treKey.AddItem TVI_ROOT, "HKEY_CURRENT_CONFIG"
End Sub

Public Sub GetRegList(Node As HTREEITEM, treKey As Class_TreeView, lvwValue As Class_ListView)
    Dim KeyHandle As HKEY
    Dim lpSubKey As StringW
    Dim MainHKEY As HKEY = GetRegPathByNodeW(Node, treKey, lpSubKey)
    lvwValue.DeleteAllItems
    
    If (RegOpenKeyEx(MainHKEY, lpSubKey, 0, KEY_READ, @KeyHandle) <> ERROR_SUCCESS) Then
        Print GetLastError & WinErrorMsg(GetLastError) & "(RegOpenKeyEx)"
        Exit Sub
    End If
    'AfxMsg *lpSubKey

    Dim szKeyName As WString * MAX_PATH
    Dim lStatus As Long
    Dim dwIndex As DWORD
    Do Until (lStatus = ERROR_NO_MORE_ITEMS)
        lStatus = RegEnumKey(KeyHandle, dwIndex, @szKeyName, MAX_PATH)
        If (lStatus = ERROR_SUCCESS) Then
            treKey.AddItem Node, szKeyName
        End If
        dwIndex += 1
    Loop
    
    dwIndex = 0
    Dim valueName As WString * MAX_PATH
    Dim valueNameLength As DWORD
    Dim dataType As DWORD
    Dim dataBuffer() As Byte
    Dim dataSize As DWORD
    
    ' 查询默认值
    dataSize = 256
    lStatus = RegQueryValueEx(KeyHandle, vbNullString, 0, @dataType, @dataBuffer(0), @dataSize)
    
    lvwValue.AddItem "(默认)"
    If lStatus = ERROR_SUCCESS Then
        Select Case dataType
            Case REG_SZ
                Dim szData As StringW
                memcpy szData, @dataBuffer(0), dataSize
                lvwValue.SetItemText 0, 1, "REG_SZ"
                lvwValue.SetItemText 0, 2, szData
            Case REG_EXPAND_SZ
                Dim szExpandData As StringW
                memcpy szExpandData, @dataBuffer(0), dataSize
                lvwValue.SetItemText 0, 1, "REG_EXPAND_SZ"
                lvwValue.SetItemText 0, 2, szExpandData
            Case REG_BINARY
                Dim binaryData As String = ""
                For i As Integer = 0 To dataSize - 1
                    binaryData = binaryData & Right("0" & Hex(dataBuffer(i)), 2) & " "
                Next
                lvwValue.SetItemText 0, 1, "REG_BINARY"
                lvwValue.SetItemText 0, 2, binaryData
            Case REG_DWORD
                Dim dwordData As Long = 0
                memcpy @dwordData, @dataBuffer(0), SizeOf(DWORD)
                lvwValue.SetItemText 0, 1, "REG_DWORD"
                lvwValue.SetItemText 0, 2, WStr(dwordData)
                Print dwordData
            Case REG_QWORD
                Dim qwordData As Long = 0
                memcpy @qwordData, @dataBuffer(0), SizeOf(QWORD)
                lvwValue.SetItemText 0, 1, "REG_QWORD"
                lvwValue.SetItemText 0, 2, WStr(qwordData)
            Case REG_MULTI_SZ'是否需要额外处理?
                Dim multiSzData As StringW
                memcpy multiSzData, @dataBuffer(0), dataSize
                lvwValue.SetItemText 0, 1, "REG_MULTI_SZ"
                lvwValue.SetItemText 0, 2, multiSzData
        End Select
    Else
        lvwValue.SetItemText 0, 1, "REG_SZ"
        lvwValue.SetItemText 0, 2, "(数据未设置)"
    End If
    
    Do
        valueName = String(255, vbNullChar)
        valueNameLength = 255
        ReDim dataBuffer(0 To 1023) As Byte
        dataSize = 1024
        ' 枚举注册表值
        lStatus = RegEnumValue(KeyHandle, dwIndex, @valueName, @valueNameLength, 0, @dataType, @dataBuffer(0), @dataSize)
        If lStatus = ERROR_SUCCESS Then
            valueName = Left(valueName, valueNameLength)
            lvwValue.AddItem valueName
            Select Case dataType
                Case REG_SZ
                    Dim szData As StringW
                    memcpy szData, @dataBuffer(0), dataSize
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_SZ"
                    lvwValue.SetItemText dwIndex + 1, 2, szData
                Case REG_EXPAND_SZ
                    Dim szExpandData As StringW
                    memcpy szExpandData, @dataBuffer(0), dataSize
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_EXPAND_SZ"
                    lvwValue.SetItemText dwIndex + 1, 2, szExpandData
                Case REG_BINARY
                    Dim binaryData As String = ""
                    For i As Integer = 0 To dataSize - 1
                        binaryData = binaryData & Right("0" & Hex(dataBuffer(i)), 2) & " "
                    Next
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_BINARY"
                    lvwValue.SetItemText dwIndex + 1, 2, binaryData
                Case REG_DWORD
                    Dim dwordData As Long = 0
                    memcpy @dwordData, @dataBuffer(0), SizeOf(DWORD)
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_DWORD"
                    lvwValue.SetItemText dwIndex + 1, 2, WStr(dwordData)
                Case REG_QWORD
                    Dim qwordData As Long = 0
                    memcpy @qwordData, @dataBuffer(0), SizeOf(QWORD)
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_QWORD"
                    lvwValue.SetItemText dwIndex + 1, 2, WStr(qwordData)
                Case REG_MULTI_SZ'是否需要额外处理?
                    Dim multiSzData As StringW
                    memcpy multiSzData, @dataBuffer(0), dataSize
                    lvwValue.SetItemText dwIndex + 1, 1, "REG_MULTI_SZ"
                    lvwValue.SetItemText dwIndex + 1, 2, multiSzData
            End Select
            dwIndex += 1
        ElseIf lStatus = ERROR_NO_MORE_ITEMS Then
            Exit Do
        Else
            Print "枚举注册表值时出错，错误代码：" & lStatus
            Exit Do
        End If
    Loop
    
    RegCloseKey KeyHandle
End Sub

Function AddRButtonMenu() As BOOLEAN
    ' 右键菜单显示名称
    Dim menuName As Const WString * MAX_PATH = "使用SnowSword粉碎文件"

    Dim hKey As HKEY
    Dim hKey1 As hKey
    Dim result As Long
    
    Dim szSubKey1 As Const WString * MAX_PATH = "*\shell\SnowSword"

    ' 为所有文件类型添加右键菜单
    result = RegCreateKeyEx(HKEY_CLASSES_ROOT, @szSubKey1, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hKey, NULL)
    ' 创建注册表项（HKEY_CLASSES_ROOT\*\shell\SnowSword\runas）
    result = RegCreateKeyEx(hKey, StrPtrW("runas"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hKey1, NULL)
    If result = ERROR_SUCCESS Then
        ' 设置菜单显示名称
        RegSetValueExW hKey, NULL, 0, REG_SZ, CPtr(LPBYTE, @menuName), SizeOf(menuName)
        ' 创建command子项，指定程序路径
        Dim hCmdKey As hKey
        result = RegCreateKeyEx(hKey, StrPtrW("command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hCmdKey, NULL)
        If result = ERROR_SUCCESS Then
            ' 设置命令，%1表示选中的文件路径
            Dim strCommand As Const WString * MAX_PATH = !"PowerShell -Command \"Start-Process" & !" \'" & App.Path & App.EXEName _
                                                                                                & !"\' " & "-ArgumentList '-DeleteFile " & !"\"\"%1\"\"" & !"\' -Verb RunAs\""
            RegSetValueExW hCmdKey, NULL, 0, REG_SZ, CPtr(LPBYTE, @strCommand), SizeOf(strCommand)
            RegCloseKey hCmdKey
            Print "右键菜单注册成功！"
        Else
            Print "创建command子项失败，错误代码：" & result
            Return False
        End If
        RegCloseKey hKey
        Return True
    Else
        Print "创建注册表项失败，错误代码：" & result
        Return False
    End If
End Function

Function DeleteRButtonMenu() As BOOLEAN
    Dim result As Long

    ' 删除为所有文件类型添加的右键菜单
    result = RegDeleteTree(HKEY_CLASSES_ROOT, StrPtrW("*\shell\MyProgram"))

    If result = ERROR_SUCCESS Then
        Print "右键菜单已成功移除！"
        Return True
    ElseIf result = ERROR_FILE_NOT_FOUND Then
        Print "未找到已注册的右键菜单"
        Return False
    Else
        Print "移除右键菜单失败，错误代码：" & result
        Return False
    End If
End Function
