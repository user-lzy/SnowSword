#include "win/iphlpapi.bi"
#include once "win/ws2spi.bi"

Const TCP_TABLE_BASIC_LISTENER As Long = 0
Const TCP_TABLE_BASIC_CONNECTIONS As Long = 1
Const TCP_TABLE_BASIC_ALL As Long = 2
Const TCP_TABLE_OWNER_PID_LISTENER As Long = 3
Const TCP_TABLE_OWNER_PID_CONNECTIONS As Long = 4
Const TCP_TABLE_OWNER_PID_ALL As Long = 5
Const TCP_TABLE_OWNER_MODULE_LISTENER As Long = 6
Const TCP_TABLE_OWNER_MODULE_CONNECTIONS As Long = 7
Const TCP_TABLE_OWNER_MODULE_ALL As Long = 8
Const UDP_TABLE_BASIC As Long = 0
Const UDP_TABLE_OWNER_PID As Long = 1
Const UDP_TABLE_OWNER_MODULE As Long = 2

/'Private Function GetIPAddress(dwAddr As Long) As String
    Dim arrIpParts(3) As Byte
    memcpy @arrIpParts(0), @dwAddr, 4
    Return Str(arrIpParts(0)) & "." & _
    Str(arrIpParts(1)) & "." & _
    Str(arrIpParts(2)) & "." & _
    Str(arrIpParts(3))
End Function '/

Function GetIPv6Address(addr() As UCHAR) As String
    Dim ipv6 As String
    For i As Integer = 0 To 15 Step 2
        Dim part As String
        part = Hex(addr(i)) & Hex(addr(i + 1))
        ipv6 &= part & ":"
    Next
    ipv6 = Left(ipv6, Len(ipv6) - 1) ' 去掉最后一个冒号
    Return ipv6
End Function

Function IPStateToString(state As DWORD) As String
    Select Case state
        Case MIB_TCP_STATE_CLOSED
            Return "CLOSED"
        Case MIB_TCP_STATE_LISTEN
            Return "LISTENING"
        Case MIB_TCP_STATE_SYN_SENT
            Return "SYN_SENT"
        Case MIB_TCP_STATE_SYN_RCVD
            Return "SYN_RCVD"
        Case MIB_TCP_STATE_ESTAB
            Return "ESTABLISHED"
        Case MIB_TCP_STATE_FIN_WAIT1
            Return "FIN_WAIT1"
        Case MIB_TCP_STATE_FIN_WAIT2
            Return "FIN_WAIT2"
        Case MIB_TCP_STATE_CLOSE_WAIT
            Return "CLOSE_WAIT"
        Case MIB_TCP_STATE_CLOSING
            Return "CLOSING"
        Case MIB_TCP_STATE_LAST_ACK
            Return "LAST_ACK"
        Case MIB_TCP_STATE_TIME_WAIT
            Return "TIME_WAIT"
        Case MIB_TCP_STATE_DELETE_TCB
            Return "DELETE_TCB"
        Case Else
            Return "UNKNOWN"
    End Select
End Function

Function GetIPAddress(dwAddr As DWORD) As String
    '将DWORD类型的dwAddr转换为IN_ADDR类型
    Dim inAddr As IN_ADDR
    memcpy @inAddr, @dwAddr, SizeOf(DWORD)
    Dim strIP As ZString Ptr = Allocate(20)
    strIP = inet_ntoa(inAddr)
    Return *strIP
End Function

Public Sub GetWebConnectList(ByVal PID As DWORD, lvwInternet As Class_ListView, ByVal IsFilter As Boolean)
    InitListViewData lvwInternet
    
    'Print lvwInternet.ColumnText(2)
    If IsFilter And lvwInternet.ColumnText(1) = "PID" Then
        lvwInternet.DeleteColumn 7
        lvwInternet.DeleteColumn 1
    ElseIf (Not IsFilter) And lvwInternet.ColumnText(1) <> "PID" Then
        lvwInternet.InsertColumn 1, "PID"
        lvwInternet.AddColumn "路径",, 500
    End If
    '枚举TCP连接
    Dim lngSize As DWORD = 0, nRet As Long, pTcpTable As PMIB_TCPTABLE_OWNER_PID'MODULE
    nRet = GetExtendedTcpTable(0, @lngSize, True, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) 'Requires Windows Vista or Windows XP SP2.
    pTcpTable = Allocate(lngSize)
    nRet = GetExtendedTcpTable(pTcpTable, @lngSize, True, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
    For i As Integer = 0 To pTcpTable->dwNumEntries - 1
        'If TcpTable.dwRemoteAddr <> 0 Or GetPort(TcpTable.dwRemotePort) <> 0 Or GetPort(TcpTable.dwLocalPort) <> 0 Then
        If Not IsFilter Then
            AddItemColListEx lvwInternet, 8,,, "TCP", WStr(pTcpTable->table(i).dwOwningPid), IPStateToString(pTcpTable->table(i).dwState), GetIPAddress(pTcpTable->table(i).dwLocalAddr), GetIPAddress(pTcpTable->table(i).dwRemoteAddr), _
            WStr(htons(pTcpTable->table(i).dwLocalPort)), WStr(htons(pTcpTable->table(i).dwRemotePort)), WStr(GetProcessPath(pTcpTable->table(i).dwOwningPid))
        Else
            If pTcpTable->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 6,,, "TCP", IPStateToString(pTcpTable->table(i).dwState), GetIPAddress(pTcpTable->table(i).dwLocalAddr), GetIPAddress(pTcpTable->table(i).dwRemoteAddr), _
            WStr(htons(pTcpTable->table(i).dwLocalPort)), WStr(htons(pTcpTable->table(i).dwRemotePort))
        End If
        FF_DoEvents
    Next
    Deallocate pTcpTable
    '枚举TCP6连接
    Dim pTcp6Table As PMIB_TCP6TABLE_OWNER_PID'MODULE
    nRet = GetExtendedTcpTable(0, @lngSize, True, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) 'Requires Windows Vista or Windows XP SP2.
    pTcp6Table = Allocate(lngSize)
    nRet = GetExtendedTcpTable(pTcp6Table, @lngSize, True, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0)
    For i As Integer = 0 To pTcp6Table->dwNumEntries - 1
        'If TcpTable.dwRemoteAddr <> 0 Or GetPort(TcpTable.dwRemotePort) <> 0 Or GetPort(TcpTable.dwLocalPort) <> 0 Then
        If Not IsFilter Then
            /'If lvwInternet.ColumnText(1) <> "PID" Then
                lvwInternet.InsertColumn 1, "PID"
                lvwInternet.AddColumn "路径"
            End If'/
            AddItemColListEx lvwInternet, 8,,, "TCP6", WStr(pTcp6Table->table(i).dwOwningPid), IPStateToString(pTcp6Table->table(i).dwState), GetIPv6Address(pTcp6Table->table(i).ucLocalAddr()), GetIPv6Address(pTcp6Table->table(i).ucRemoteAddr()), _
            WStr(htons(pTcp6Table->table(i).dwLocalPort)), WStr(htons(pTcp6Table->table(i).dwRemotePort)), WStr(GetProcessPath(pTcp6Table->table(i).dwOwningPid))
        Else
            /'/If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pTcp6Table->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 6,,, "TCP6", IPStateToString(pTcp6Table->table(i).dwState), GetIPv6Address(pTcp6Table->table(i).ucLocalAddr()), GetIPv6Address(pTcp6Table->table(i).ucRemoteAddr()), _
            WStr(htons(pTcp6Table->table(i).dwLocalPort)), WStr(htons(pTcp6Table->table(i).dwRemotePort))
        End If
        FF_DoEvents
    Next
    Deallocate pTcp6Table
    '枚举UDP连接
    Dim pUdpTable As PMIB_UDPTABLE_OWNER_PID'MODULE
    nRet = GetExtendedUdpTable(0, @lngSize, True, AF_INET, UDP_TABLE_OWNER_PID, 0) 'Requires Windows Vista or Windows XP SP2.'UDP_TABLE_OWNER_PID_ALL
    pUdpTable = Allocate(lngSize)
    nRet = GetExtendedUdpTable(pUdpTable, @lngSize, True, AF_INET, UDP_TABLE_OWNER_PID, 0)
    For i As Integer = 0 To pUdpTable->dwNumEntries - 1
        'If UdpTable.dwRemoteAddr <> 0 Or GetPort(UdpTable.dwRemotePort) <> 0 Or GetPort(UdpTable.dwLocalPort) <> 0 Then
        If Not IsFilter Then
            /'If lvwInternet.ColumnText(1) <> "PID" Then
                lvwInternet.InsertColumn 1, "PID"
                lvwInternet.AddColumn "路径"
            End If'/
            AddItemColListEx lvwInternet, 8,,, "UDP", WStr(pUdpTable->table(i).dwOwningPid), WStr(""), GetIPAddress(pUdpTable->table(i).dwLocalAddr), WStr(""), _
            WStr(htons(pUdpTable->table(i).dwLocalPort)), WStr(""), WStr(GetProcessPath(pUdpTable->table(i).dwOwningPid))
        Else
            /'If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pUdpTable->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 5,,, "UDP", WStr(""), GetIPAddress(pUdpTable->table(i).dwLocalAddr), WStr(""), _
            WStr(htons(pUdpTable->table(i).dwLocalPort))
        End If
        FF_DoEvents
    Next
    Deallocate pUdpTable
    '枚举UDP6连接
    Dim pUdp6Table As PMIB_UDP6TABLE_OWNER_PID'MODULE
    nRet = GetExtendedUdpTable(0, @lngSize, True, AF_INET6, UDP_TABLE_OWNER_PID, 0) 'Requires Windows Vista or Windows XP SP2.
    pUdp6Table = Allocate(lngSize)
    nRet = GetExtendedUdpTable(pUdp6Table, @lngSize, True, AF_INET6, UDP_TABLE_OWNER_PID, 0)
    For i As Integer = 0 To pUdp6Table->dwNumEntries - 1
        'If Udp6Table.dwRemoteAddr <> 0 Or GetPort(Udp6Table.dwRemotePort) <> 0 Or GetPort(Udp6Table.dwLocalPort) <> 0 Then
        If Not IsFilter Then
            /'If lvwInternet.ColumnText(1) <> "PID" Then
                lvwInternet.InsertColumn 1, "PID"
                lvwInternet.AddColumn "路径"
            End If'/
            AddItemColListEx lvwInternet, 8,,, "UDP6", WStr(pUdp6Table->table(i).dwOwningPid), WStr(""), GetIPv6Address(pUdp6Table->table(i).ucLocalAddr()), WStr(""), _
            WStr(htons(pUdp6Table->table(i).dwLocalPort)), WStr(""), WStr(GetProcessPath(pUdp6Table->table(i).dwOwningPid))
        Else
            /'If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pUdp6Table->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 5,,, "UDP6", WStr(""), GetIPv6Address(pUdp6Table->table(i).ucLocalAddr()), WStr(""), _
            WStr(htons(pUdp6Table->table(i).dwLocalPort))
        End If
        FF_DoEvents
    Next
    Deallocate pUdp6Table
    CommitListViewView lvwInternet
End Sub

' ==============================================
' 核心函数：枚举系统 Winsock SPI 提供者 (对齐ETW代码风格)
' ==============================================
Sub GetWinsockSPIList(lvwSPI As Class_ListView)
    Dim pBuffer As PVOID = NULL
    Dim bufSize As ULONG = 0
    Dim lProtocolsCount As LONG = 0
    Dim pProtocolInfo As LPWSAPROTOCOL_INFOW  ' 用你的自定义指针
    Dim i As Integer, count As Integer
    Dim wsaData As WSADATA
    
    ' ======================
    ' 1. 清空 ListView
    ' ======================
    InitListViewData lvwSPI
    
    ' ======================
    ' 2. 初始化Winsock
    ' ======================
    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
        Print "Winsock初始化失败"
        Exit Sub
    End If
    
    ' ======================
    ' 3. 获取缓冲区大小
    ' ======================
    WSCEnumProtocols(NULL, NULL, @bufSize, NULL)
    If bufSize = 0 Then
        Print "获取缓冲区大小失败"
        WSACleanup()
        Exit Sub
    End If
    
    ' ======================
    ' 4. 分配内存
    ' ======================
    pBuffer = Allocate(bufSize)
    If pBuffer = NULL Then
        Print "内存分配失败"
        WSACleanup()
        Exit Sub
    End If
    
    pProtocolInfo = pBuffer
    
    ' ======================
    ' 🔥 修复2：删除强制转型！直接传你的自定义结构体指针
    ' ======================
    lProtocolsCount = WSCEnumProtocols(NULL, pProtocolInfo, @bufSize, NULL)
    If lProtocolsCount = SOCKET_ERROR Then
        Print "枚举SPI失败，错误码："; Hex(GetLastError())
        Deallocate(pBuffer)
        WSACleanup()
        Exit Sub
    End If
    
    ' ======================
    ' 5. 遍历数据
    ' ======================
    count = lProtocolsCount
    If count = 0 Then
        Print "未找到任何Winsock SPI提供者"
        Deallocate(pBuffer)
        WSACleanup()
        Exit Sub
    End If
    
    For i = 0 To count - 1
        Dim pInfo As WSAPROTOCOL_INFOW Ptr = @pProtocolInfo[i]
        Dim szCLSID As WString * 64
        Dim szDllPath As WString * 1024  ' 缓冲区
        Dim nPath As Long = 1024
        Dim nErrno As Long
        
        ' CLSID 转换 (ProviderId 就是你要的 GUID)
        StringFromGUID2(@pInfo->ProviderId, szCLSID, 64)
        
        ' 🔥 正确获取 DLL 路径 🔥
        If WSCGetProviderPath(@pInfo->ProviderId, szDllPath, @nPath, @nErrno) = SOCKET_ERROR Then
            szDllPath = "获取失败"
        End If
        szDllPath = FF_Replace(szDllPath, "%SystemRoot%", Environ("SystemRoot"))
        ' 添加列表
        AddItemColListEx lvwSPI, 5,,, WStr(i + 1), _
            pInfo->szProtocol, _
            szDllPath, _  ' 这里传正确获取的路径
            WStr("未知"), _
            szCLSID
    Next
    
    ' ======================
    ' 6. 释放资源
    ' ======================
    Deallocate(pBuffer)
    WSACleanup()
    
    Print "Winsock SPI 枚举完成，总数："; count
    CommitListViewView lvwSPI
End Sub