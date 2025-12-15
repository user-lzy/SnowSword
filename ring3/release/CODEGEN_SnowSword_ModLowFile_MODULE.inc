'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModLowFile.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/winbase.bi" '已经搬到定义文件处

Public Sub GetMBR()
    Dim hDisk As HANDLE = CreateFile("\\.\PHYSICALDRIVE0", GENERIC_READ, FILE_SHARE_READ Or FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
    If (hDisk = NULL) Then AfxMsg WinErrorMsg(GetLastError) & "(" & GetLastError & ")":Exit Sub
    '读取MBR数据, 前512字节
    Dim bytMBRData(511) As UByte, dwRet As DWORD
    If (ReadFile(hDisk, @bytMBRData(0), 512, @dwRet, NULL) = 0) Then AfxMsg WinErrorMsg(GetLastError) & "(" & GetLastError & ")":Exit Sub
    AfxMsg bytMBRData(0) & bytMBRData(1)
    CloseHandle hDisk
End Sub