#include Once "win/winsock2.bi"
#include Once "win/ws2tcpip.bi"
'Extern "C"
Declare Function inet_pton Lib "ws2_32.dll" Alias "inet_pton"(ByVal af As Integer, ByVal src As LPCSTR, ByVal dst As Any Ptr) As Integer
'End Extern

Type TCPHeader
    sourcePort As UShort
    destPort As UShort
    seqNumber As ULong
    ackNumber As ULong
    dataOffset As UByte
    reserved As UByte
    flags As UByte
    windowSize As UShort
    checksum As UShort
    urgentPointer As UShort
End Type

Type TCPFalseHeader
    sourceIP As ULong
    destIP As ULong
    reserved As UChar
    protocol As UChar
    tcpLength As UShort
End Type

Type IPHeader
    verIhl As UByte
    tos As UByte
    totLen As UShort
    id As UShort
    fragOff As UShort
    ttl As UByte
    protocol As UByte
    check As UShort
    saddr As ULong
    daddr As ULong
End Type

Type ICMPHeader
    TheType As UByte
    code As UByte
    checksum As UShort
    unused As UShort
    nextHopMtu As UShort
End Type

Type ARPHeader
    htype As UShort
    ptype As UShort
    hlen As UByte
    plen As UByte
    oper As UShort
    sha(5) As UByte
    spa(3) As UByte
    tha(5) As UByte
    tpa(3) As UByte
End Type

Type ECHO_REQUEST
    ttype As UByte
    code As UByte
    checksum As UShort
    id As UShort
    seq As UShort
End Type

Function GetHostNameByAddr(ByVal pstrIP As LPCSTR) As String 
    ' 初始化Winsock
    Dim wsaData As WSADATA
    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
        MyLog.PrintLog LOG_ERROR, "GetHostNameByAddr", "WSAStartup"
        Return ""
    End If

    Dim ipAddr As ULong = inet_addr(pstrIP)

    ' 创建sockaddr_in结构体
    'Dim addr As sockaddr_in
    'addr.sin_family = AF_INET
    'addr.sin_addr.s_addr = IPAddr
    'addr.sin_port = 0 ' 端口号可以设置为0

    ' 获取主机名
    /'Dim hostName As ZString * MAX_PATH
    Dim ret As Integer = getnameinfo(Cast(sockaddr Ptr, @addr), SizeOf(addr), @hostName, MAX_PATH, 0, 0, NI_NAMEREQD)
    If ret = 0 Then
        Function = hostName
        Print hostName
    Else
        Print "无法获取主机名"
    End If '/
    ' 获取主机名
    Dim hostentPtr As hostent Ptr = gethostbyaddr(Cast(ZString Ptr, @ipAddr), 4, AF_INET)
    If hostentPtr = 0 Then
        MyLog.PrintLog LOG_ERROR, "gethostbyaddr", "pstrIP=" & *pstrIP
    Else
        Function = *(hostentPtr->h_name)
    End If

    ' 清理Winsock
    WSACleanup
End Function

'Function GetHost(ip_str As Const ZString Ptr) As String
'    Dim addrinfo As SOCKADDR_IN
'    Dim addr_len As Integer = SizeOf(Integer)
'    Dim wsaData As WSADATA
'    Dim host_name As ZString Ptr = Allocate(NI_MAXHOST)
'    Dim serv_info As ZString Ptr = Allocate(NI_MAXSERV)
'    Dim result As Integer
'
'    ' 初始化 Winsock
'    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
'        MyLog.PrintLog LOG_ERROR, "GetHost", "WSAStartup"
'        Return ""
'    EndIf
'
'    ' 设置 sockaddr_in 结构体
'    addrinfo.sin_family = AF_INET
'    inet_pton(AF_INET, ip_str, Cast(LPVOID, @addrinfo.sin_addr))
'
'    ' 获取主机名和服务信息
'    'result = getnameinfo(Cast(LPSOCKADDR, @addrinfo), addr_len, CPtr(ZString Ptr, host_name), NI_MAXHOST, serv_info, NI_MAXSERV, NI_NAMEREQD)
'
'    ' 检查结果
'    If result <> 0 Then
'        MyP "Error in getnameinfo: " & WSAGetLastError
'        WSACleanup
'        Return ""
'    EndIf
'
'    ' 清理 Winsock
'    WSACleanup
'
'    ' 返回主机名
'    Return *host_name
'End Function

Public Function SendIcmpPacket(ByVal pIP As LPCSTR) As Boolean
    Dim ret As Long, wsa As WSAData
    ret = WSAStartup(MAKEWORD(2, 2), @wsa)
    Dim sock As Long = socket_(AF_INET, SOCK_RAW, IPPROTO_ICMP)
    Static seq As Integer, id As Integer : seq = 1 : id = 1
    Dim Icmp_Header As ICMPHeader = (8, 0, 0, seq, id)
    
    Dim Buf() As Byte: ReDim Buf(SizeOf(ICMPHeader) - 1)
    memcpy @Buf(0), @Icmp_Header, SizeOf(ICMPHeader)
    Icmp_Header.checksum = GetChecksum(@Buf(0), UBound(Buf) + 1)
    memcpy @Buf(0), @Icmp_Header, SizeOf(ICMPHeader)
    
    '解析目标IP地址
    Dim destAddr As SOCKADDR_IN = (AF_INET, 0)
    If (inet_pton(AF_INET, pIP, @destAddr.sin_addr) <= 0) Then
        MyLog.PrintLog LOG_ERROR, "SendIcmpPacket", "inet_pton", "Invalid IP address,pIP=" & *pIP
        closesocket sock
        WSACleanup
        Return False
    End If
    
    /'Dim fMode As Long = 1 ' 非阻塞模式
    If ioctlsocket(sock, FIONBIO, @fMode) <> 0 Then
        MyLog.PrintLog LOG_ERROR, "SendIcmpPacket", "ioctlsocket", "无法设置非阻塞模式,WSAGetLastError=" & WSAGetLastError
        Return False'/
    
    ret = sendto(sock, @Buf(0), SizeOf(ICMPHeader), 0, CPtr(Const SOCKADDR Ptr, @destAddr), Len(destAddr))
    If ret < 0 Then
        MyLog.PrintLog LOG_ERROR, "SendIcmpPacket", "sendto", "WSAGetLastError=" & WSAGetLastError & ",pIP=" & *pIP
    Else
        MyLog.PrintInfo "SendIcmpPacket",, "success!"
    End If
    SendIcmpPacket = (ret >= 0)
    seq += 1
    id += 1
    closesocket sock
    WSACleanup
End Function

