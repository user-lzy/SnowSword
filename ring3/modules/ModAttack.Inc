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
        Print "WSAStartup failed"
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
        Print "无法获取主机名:" & WinErrorMsg(GetLastError) & GetLastError
    Else
        Function = *(hostentPtr->h_name)
    End If

    ' 清理Winsock
    WSACleanup
End Function

Function GetHost(ip_str As Const ZString Ptr) As String
    Dim addrinfo As SOCKADDR_IN
    Dim addr_len As Integer = SizeOf(Integer)
    Dim wsaData As WSADATA
    Dim host_name As ZString Ptr = Allocate(NI_MAXHOST)
    Dim serv_info As ZString Ptr = Allocate(NI_MAXSERV)
    Dim result As Integer

    ' 初始化 Winsock
    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
        Print "Failed to initialize Winsock"
        Return ""
    EndIf

    ' 设置 sockaddr_in 结构体
    addrinfo.sin_family = AF_INET
    inet_pton(AF_INET, ip_str, Cast(LPVOID, @addrinfo.sin_addr))

    ' 获取主机名和服务信息
    'result = getnameinfo(Cast(LPSOCKADDR, @addrinfo), addr_len, CPtr(ZString Ptr, host_name), NI_MAXHOST, serv_info, NI_MAXSERV, NI_NAMEREQD)

    ' 检查结果
    If result <> 0 Then
        Print "Error in getnameinfo: " & WSAGetLastError
        WSACleanup
        Return ""
    EndIf

    ' 清理 Winsock
    WSACleanup

    ' 返回主机名
    Return *host_name
End Function

Public Function SendIcmpPacket(ByVal pIP As LPCSTR) As BOOLEAN
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
        Print "Invalid IP address"
        closesocket sock
        WSACleanup
        Return False
    End If
    
    Dim fMode As Long = 1 ' 非阻塞模式
    If ioctlsocket(sock, FIONBIO, @fMode) <> 0 Then Print "无法设置非阻塞模式"
    
    ret = sendto(sock, @Buf(0), SizeOf(ICMPHeader), 0, CPtr(Const SOCKADDR Ptr, @destAddr), Len(destAddr))
    If ret < 0 Then
        Print "[SendIcmpPacket]sendto:" & WinErrorMsg(GetLastError) & GetLastError
    Else
        Print "[SendIcmpPacket]success!"
    End If
    SendIcmpPacket = (ret >= 0)
    seq += 1
    id += 1
    closesocket sock
    WSACleanup
End Function

Function CreateTCPChecksum(ipHdr As IPHeader Ptr, tcpHdr As TCPHeader Ptr, dataLen As Integer) As UShort
    Dim pseudoHdr(11) As Byte
    CopyMemory(@pseudoHdr(0), @ipHdr->saddr, 4)
    CopyMemory(@pseudoHdr(4), @ipHdr->daddr, 4)
    pseudoHdr(8) = 0
    pseudoHdr(9) = ipHdr->protocol
    CopyMemory(@pseudoHdr(10), @dataLen, 2)

    Dim buffer(11 + 20 + dataLen - 1) As Byte
    CopyMemory(@buffer(0), @pseudoHdr(0), 12)
    CopyMemory(@buffer(12), tcpHdr, 20 + dataLen)

    Return GetChecksum(@buffer(0), 12 + 20 + dataLen)
End Function

/'Sub SendTCPPacket()
    ' 初始化Winsock
    Dim wsaData As WSADATA
    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
        Print "WSAStartup failed"
        Exit Sub
    End If

    ' 创建原始套接字
    Dim sock As Long = socket_(AF_INET, SOCK_RAW, IPPROTO_TCP)
    If sock = -1 Then
        Print "Socket creation failed"
        WSACleanup
        Exit Sub
    End If

    ' 设置套接字选项
    Dim optVal As Long = 1
    If setsockopt(sock, IPPROTO_IP, IP_HDRINCL, Cast(Const ZString Ptr, @optVal), 4) <> 0 Then
        Print "setsockopt failed"
        closesocket(sock)
        WSACleanup
        Exit Sub
    End If
    
    ' 构造IP头部
    Dim ipHdr As IPHeader
    ipHdr.verIhl = &H45
    ipHdr.tos = 0
    ipHdr.totLen = htons(SizeOf(IPHeader) + SizeOf(TCPHeader))'htons(20 + 20) ' IP头部 + TCP头部
    ipHdr.id = htons(12345)
    ipHdr.fragOff = 0
    ipHdr.ttl = 64
    ipHdr.protocol = IPPROTO_TCP
    ipHdr.saddr = inet_addr("192.168.1.100")
    ipHdr.daddr = inet_addr("192.168.1.200")

    ' 构造TCP头部
    Dim tcpHdr As TCPHeader
    tcpHdr.sourcePort = htons(1234)
    tcpHdr.destPort = htons(80)
    tcpHdr.seqNumber = htonl(123456789)
    tcpHdr.ackNumber = 0
    tcpHdr.dataOffset = &H50
    tcpHdr.flags = &H02 ' SYN标志
    tcpHdr.windowSize = htons(512)
    tcpHdr.checksum = 0
    tcpHdr.urgentPointer = 0

    ' 计算TCP校验和
    tcpHdr.checksum = CreateTCPChecksum(@ipHdr, @tcpHdr, 0)

    ' 构造数据包
    Dim packet(39) As Byte
    CopyMemory(@packet(0), @ipHdr, 20)
    CopyMemory(@packet(20), @tcpHdr, 20)

    ' 发送数据包
    Dim destAddr As sockaddr_in
    destAddr.sin_family = AF_INET
    destAddr.sin_port = htons(80)
    destAddr.sin_addr.s_addr = inet_addr("192.168.1.200")

    Dim bytesSent As Long = sendto(sock, @packet(0), 40, 0, CPtr(sockaddr Ptr, @destAddr), Len(destAddr))
    If bytesSent < 0 Then
        Print "sendto failed: " & WinErrorMsg(GetLastError)
    Else
        Print "TCP packet sent successfully"
    End If

    ' 关闭套接字
    closesocket sock
    ' 清理Winsock
    WSACleanup
End Sub'/

