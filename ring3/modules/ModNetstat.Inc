#include "win/iphlpapi.bi"

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

Public Sub GetWebConnectList(ByVal PID As DWORD, lvwInternet As Class_ListView, ByVal IsFilter As BOOLEAN)
    lvwInternet.DeleteAllItems
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
            lvwInternet.AddItemColList ,, "TCP", WStr(pTcpTable->table(i).dwOwningPid), IPStateToString(pTcpTable->table(i).dwState), GetIPAddress(pTcpTable->table(i).dwLocalAddr), GetIPAddress(pTcpTable->table(i).dwRemoteAddr), _
            WStr(htons(pTcpTable->table(i).dwLocalPort)), WStr(htons(pTcpTable->table(i).dwRemotePort)), GetProcessPath(pTcpTable->table(i).dwOwningPid)
        Else
            If pTcpTable->table(i).dwOwningPid <> PID Then Continue For
            lvwInternet.AddItemColList ,, "TCP", IPStateToString(pTcpTable->table(i).dwState), GetIPAddress(pTcpTable->table(i).dwLocalAddr), GetIPAddress(pTcpTable->table(i).dwRemoteAddr), _
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
            lvwInternet.AddItemColList ,, "TCP6", WStr(pTcp6Table->table(i).dwOwningPid), IPStateToString(pTcp6Table->table(i).dwState), GetIPv6Address(pTcp6Table->table(i).ucLocalAddr()), GetIPv6Address(pTcp6Table->table(i).ucRemoteAddr()), _
            WStr(htons(pTcp6Table->table(i).dwLocalPort)), WStr(htons(pTcp6Table->table(i).dwRemotePort)), GetProcessPath(pTcp6Table->table(i).dwOwningPid)
        Else
            /'/If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pTcp6Table->table(i).dwOwningPid <> PID Then Continue For
            lvwInternet.AddItemColList ,, "TCP6", IPStateToString(pTcp6Table->table(i).dwState), GetIPv6Address(pTcp6Table->table(i).ucLocalAddr()), GetIPv6Address(pTcp6Table->table(i).ucRemoteAddr()), _
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
            lvwInternet.AddItemColList ,, "UDP", WStr(pUdpTable->table(i).dwOwningPid),, GetIPAddress(pUdpTable->table(i).dwLocalAddr),, _
            WStr(htons(pUdpTable->table(i).dwLocalPort)),, GetProcessPath(pUdpTable->table(i).dwOwningPid)
        Else
            /'If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pUdpTable->table(i).dwOwningPid <> PID Then Continue For
            lvwInternet.AddItemColList ,, "UDP",, GetIPAddress(pUdpTable->table(i).dwLocalAddr),, _
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
            lvwInternet.AddItemColList ,, "UDP6", WStr(pUdp6Table->table(i).dwOwningPid),, GetIPv6Address(pUdp6Table->table(i).ucLocalAddr()),, _
            WStr(htons(pUdp6Table->table(i).dwLocalPort)),, GetProcessPath(pUdp6Table->table(i).dwOwningPid)
        Else
            /'If lvwInternet.ColumnText(1) = "PID" Then
                lvwInternet.DeleteColumn 8
                lvwInternet.DeleteColumn 1
            End If'/
            If pUdp6Table->table(i).dwOwningPid <> PID Then Continue For
            lvwInternet.AddItemColList ,, "UDP6",, GetIPv6Address(pUdp6Table->table(i).ucLocalAddr()),, _
            WStr(htons(pUdp6Table->table(i).dwLocalPort))
        End If
        FF_DoEvents
    Next
    Deallocate pUdp6Table
End Sub
