#include Once "win/winbase.bi"

Public Sub GetMBR()
    Dim hDisk As HANDLE = CreateFile("\\.\PHYSICALDRIVE0", GENERIC_READ, FILE_SHARE_READ Or FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
    If (hDisk = NULL) Then AfxMsg WinErrorMsg(GetLastError) & "(" & GetLastError & ")":Exit Sub
    '读取MBR数据, 前512字节
    Dim bytMBRData(511) As UByte, dwRet As DWORD
    If (ReadFile(hDisk, @bytMBRData(0), 512, @dwRet, NULL) = 0) Then AfxMsg WinErrorMsg(GetLastError) & "(" & GetLastError & ")":Exit Sub
    AfxMsg bytMBRData(0) & bytMBRData(1)
    CloseHandle hDisk
End Sub