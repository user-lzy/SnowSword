'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModReg.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/winreg.bi" '已经搬到定义文件处
Public Sub GetRegList(treKey As Class_TreeView)
Dim KeyHandle As HKEY
Dim lpSubKey As LPWSTR
*lpSubKey = WStr("Software\Microsoft\Windows\CurrentVersion\Run")
If (RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, @KeyHandle) <> ERROR_SUCCESS) Then
    AfxMsg GetLastError & WinErrorMsg(GetLastError) & "(RegOpenKeyEx)"
    Exit Sub
End If

Dim szValueName As WString * MAX_PATH
Dim lStatus As Long
Dim dwIndex As DWORD
Do Until (lStatus = ERROR_NO_MORE_ITEMS)
    lStatus = RegEnumKey(KeyHandle, dwIndex, @szValueName, MAX_PATH)
    If (lStatus = ERROR_SUCCESS) Then AfxMsg szValueName
    dwIndex += 1
Loop

RegCloseKey KeyHandle
End Sub

Public Sub Test1()
Dim KeyHandle As HKEY
Dim disposition As DWORD
Dim szSubKey As Const WString * 20 = "Software\\aaa"
Dim result As Long = RegCreateKeyEx(HKEY_CURRENT_USER,@szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,@KeyHandle,@disposition)
If (result = ERROR_SUCCESS) Then
    AfxMsg "Key created or opened successfully. Disposition: " & disposition
    RegCloseKey KeyHandle
Else
    AfxMsg "Failed to create or open key. Error code: " & result & "(" & WinErrorMsg(result) & ")"
End If
End Sub