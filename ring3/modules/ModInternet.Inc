#include Once "win/winsock2.bi"

#define BUFFER_SIZE 1024

'Extern "C"
Declare Function inet_ntop Lib "ws2_32.dll " Alias "inet_ntop"(ByVal Family As INT_, ByVal pAddr As PVOID, ByVal pStringBuf As LPSTR, ByVal StringBufSize As UInteger) As LPCSTR
Declare Function IcmpCreateFile Lib "iphlpapi.dll" Alias "IcmpCreateFile"() As HANDLE
Declare Function IcmpSendEcho Lib "iphlpapi.dll" Alias "IcmpSendEcho"(ByVal IcmpHandle As HANDLE, ByVal DestinationAddress As IPAddr, ByVal RequestData As LPVOID, ByVal RequestSize As WORD, _
ByVal RequestOptions As PIP_OPTION_INFORMATION, ByVal ReplyBuffer As LPVOID, ByVal ReplySize As DWORD, ByVal Timeout As DWORD) As DWORD
Declare Sub IcmpCloseHandle Lib "iphlpapi.dll" Alias "IcmpCloseHandle"(ByVal IcmpHandle As HANDLE)
'End Extern

Dim Shared lvwIPScan As Class_ListView
Dim Shared lvwSniffer As Class_ListView
Dim Shared sock As Long
Dim Shared ip As IPHeader
Dim Shared tcp As TCPHeader
Dim Shared IsStartSniffer As Boolean
Dim Shared IPScanProgress As Integer

Private Function GetChecksum(bytData As Byte Ptr, ByVal length As Long) As UShort
    Dim sum As ULong
    Dim temp As ULong

    ' 按 16 位字进行求和
    For i As Integer = 0 To length - 1 Step 2
        If i + 1 < length Then
            temp = bytData[i] * 256 + bytData[i + 1]
        Else
            temp = bytData[i] * 256
        End If
        sum = sum + temp
    Next

    ' 处理进位
    sum = (sum Shr 16) + (sum And &HFFFF)
    sum = sum + (sum Shr 16)

    ' 取反得到校验和
    Return Cast(UShort, Not sum)
End Function

' 数值IP(你的ip ULONG) → MAC
Function GetMACByIP(ByVal ulIP As ULong) As String
    Dim dwSize As DWORD = 0
    Dim pTable As Any Ptr = NULL
    Dim pRow As MIB_IPNETROW Ptr
    Dim i As Integer
    Dim macStr As String = "未知MAC"  ' 默认兜底
    
    ' 1. 先获取ARP表所需内存大小
    If GetIpNetTable(NULL, @dwSize, 0) <> 0 And dwSize = 0 Then
        Return macStr
    End If
    
    ' 2. 分配内存
    pTable = Allocate(dwSize)
    If pTable = NULL Then Return macStr
    
    ' 3. 读取系统ARP表
    If GetIpNetTable(pTable, @dwSize, 0) = 0 Then
        ' 获取ARP表条目数量
        Dim dwCount As DWORD = *CPtr(DWORD Ptr, pTable)
        pRow = CPtr(MIB_IPNETROW Ptr, CPtr(Byte Ptr, pTable) + SizeOf(DWORD))
        
        ' 4. 遍历ARP表（关键：IP转换为网络字节序！）
        For i = 0 To dwCount - 1
            ' ⭐ 核心修复：htonl 转换字节序，匹配系统ARP表
            If pRow->dwAddr = htonl(ulIP) And pRow->dwPhysAddrLen = 6 Then
                ' 拼接标准MAC格式（00-11-22-33-44-55）
                macStr = UCase(Right("0" & Hex(pRow->bPhysAddr(0)), 2)) & "-" & _
                         UCase(Right("0" & Hex(pRow->bPhysAddr(1)), 2)) & "-" & _
                         UCase(Right("0" & Hex(pRow->bPhysAddr(2)), 2)) & "-" & _
                         UCase(Right("0" & Hex(pRow->bPhysAddr(3)), 2)) & "-" & _
                         UCase(Right("0" & Hex(pRow->bPhysAddr(4)), 2)) & "-" & _
                         UCase(Right("0" & Hex(pRow->bPhysAddr(5)), 2))
                Exit For
            End If
            pRow += 1
        Next
    End If
    
    ' 释放内存
    If pTable Then Deallocate(pTable)
    Return macStr
End Function

Function GetProtocolTxt(protocol As Integer) As String
    Select Case protocol
        Case IPPROTO_ICMP
            Return "ICMP"
        Case IPPROTO_TCP
            Return "TCP"
        Case IPPROTO_UDP
            Return "UDP"
        Case Else
            Return "UNKNOWN"
    End Select
End Function

' 定义线程数据结构
Type ThreadData
    IcmpHandle As HANDLE
    ip As ULong
End Type

' 使用gethostbyname将IP字符串转换为IPADDR
Function IPStringToIPADDR(ipStr As String) As UInteger
    'Dim library As Any Ptr = DyLibLoad("")
    Dim hostentPtr As hostent Ptr = gethostbyname(ipStr)
    If hostentPtr = NULL Then
        Print "无法解析IP地址," & WinErrorMsg(GetLastError)
        Return 0
    End If
    Dim ipAddr As UInteger
    CopyMemory(@IPAddr, hostentPtr->h_addr_list[0], 4)
    Return ipAddr
End Function

Sub NetworkListenerThread(ByVal userdata As Any Ptr)
    ' 堆分配缓冲区（避免栈溢出）
    Dim buffer As Byte Ptr = New Byte[BUFFER_SIZE]
    Dim ip As IPHeader
    Dim tcp As TCPHeader
    Do While True ' IsStartSniffer
        Dim running As Boolean = IsStartSniffer
        'LeaveCriticalSection(@cs)
        
        If Not running Then Exit Do
        
        ' 非阻塞检查套接字（解决recv阻塞无法退出问题）
        Dim fdSetR As fd_set
        FD_ZERO(@fdSetR)
        FD_SET_(sock, @fdSetR)
        
        Dim timeout As timeval
        timeout.tv_sec = 0
        timeout.tv_usec = 100000 ' 100ms超时，定时检查退出
        
        Dim retSelect As Long = select_(0, @fdSetR, NULL, NULL, @timeout)
        If retSelect <= 0 Then Continue Do
        
        ' 接收数据
        Dim ret As Long = recv(sock, buffer, BUFFER_SIZE, 0)
        If ret <= 0 Then Continue Do
        
        Print "ret > 0"
        
        ' ============== 安全校验：避免内存越界 ==============
        If ret < SizeOf(IPHeader) Then Continue Do
        memcpy @ip, buffer, SizeOf(IPHeader)
        
        ' 校验IP头长度
        Dim ipHeaderLen As Long = (ip.verIhl And &HF) * 4
        If ipHeaderLen < 20 Or ipHeaderLen > 60 Then Continue Do
        If ret < ipHeaderLen + SizeOf(TCPHeader) Then Continue Do
        
        ' 解析TCP头
        memcpy @tcp, buffer + ipHeaderLen, SizeOf(TCPHeader)
        lvwSniffer.AddItemColList,, GetProtocolTxt(ip.protocol), GetIPAddress(ip.saddr), GetIPAddress(ip.daddr), Str(ntohs(tcp.sourcePort)), Str(ntohs(tcp.destPort)), Str(ntohs(ip.totLen))
    Loop
End Sub

' 使用inet_ntop将IPADDR转换回IP字符串
Function IPADDRToIPString(ipAddr As UInteger) As String
    Dim ipStr As ZString * 16
    If inet_ntop(AF_INET, @IPAddr, @ipStr, 16) = 0 Then
        Print "转换失败," & WinErrorMsg(GetLastError)
        Return ""
    End If
    Return ipStr
End Function

' 线程函数
Sub PingThread(ByVal Param As Any Ptr)
    Dim tdata As ThreadData Ptr = CPtr(ThreadData Ptr, Param)
    Dim icmpReply As ICMP_ECHO_REPLY
    Dim buffer(1023) As Byte
    Dim st As DWORD = IcmpSendEcho(tdata->IcmpHandle, tdata->ip, @buffer(0), 32, NULL, @buffer(0), 1024, 1000)
    
    If st > 0 Then
        ' 主机在线
        lvwIPScan.AddItem IPADDRToIPString(tdata->ip)
        lvwIPScan.SetItemText lvwIPScan.ItemCount - 1, 1, GetHostNameByAddr(IPADDRToIPString(tdata->ip))
        'Print IPADDRToIPString(tdata->ip)
    Else
        ' 主机离线或拒绝响应
        Print "IP: " & IPADDRToIPString(tdata->ip) & " 离线或拒绝响应"
    End If
    'IPScanProgress += 1
    'FrmFireWall.lblStatus.Caption = WStr(IPScanProgress)
    ' 释放线程数据
    Deallocate tdata
End Sub

Sub GetAliveHostList(ByVal TheIP As String, ByVal startIP As ULong, ByVal endIP As ULong, ListView As Class_ListView, lblStatus As Class_Label)
    ' 初始化WinSock
    Dim wsaData As WSADATA
    If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
        Print "WSAStartup failed"
        Exit Sub
    End If
    Print TheIP & " " & startIP & " " & endIP
    ' 定义ICMP句柄
    Dim IcmpHandle As Any Ptr = IcmpCreateFile
    If IcmpHandle = 0 Then
        Print "无法创建ICMP句柄," & WinErrorMsg(GetLastError)
        Exit Sub
    End If
    
    ' 扫描指定网段
    Dim threads() As HANDLE
    ReDim threads(endIP - startIP) As HANDLE
    lvwIPScan = ListView
    
    For IP As ULong = startIP To endIP
        Dim tData As ThreadData Ptr = Allocate(SizeOf(ThreadData))
        tData->IcmpHandle = IcmpHandle
        tData->ip = IPStringToIPADDR(TheIP & "." & IP)
        threads(ip - startIP) = ThreadCreate(@PingThread, tData)
        FF_DoEvents
    Next

    ' 等待所有线程完成
    MsgWaitForMultipleObjects UBound(threads) + 1, @threads(0), True, 1000, QS_ALLINPUT
    
    For host As ULong = 0 To UBound(threads)
        TerminateThread threads(host), 0
        CloseHandle threads(host)
        'lblStatus.Caption = "进度:" & round((UBound(threads) + 1) / host * 100) & "%"
        FF_DoEvents
    Next

    ' 关闭ICMP句柄
    IcmpCloseHandle IcmpHandle
    
    '关闭WSA
    WSACleanup
End Sub

Function SetSnifferStatus(ListView As Class_ListView, ByVal IsStart As Boolean) As Boolean
    Static sock As SOCKET
    Static listenerThread As HANDLE
    If IsStart Then
        ' 初始化Winsock
        Dim wsaData As WSADATA
        If WSAStartup(MAKEWORD(2, 2), @wsaData) <> 0 Then
            Print "WSAStartup failed"
            Return False
        End If

        ' 创建原始套接字
        sock = socket_(AF_INET, SOCK_RAW, IPPROTO_IP)
        If sock = INVALID_SOCKET Then
            Print "Socket creation failed:" & WinErrorMsg(GetLastError)
            WSACleanup
            Return False
        End If
        
        ' 获取本机名
        'Dim LocalName(511) As Byte
        'gethostname @LocalName(0), 512
        '
        '' 获取本地IP地址
        'Dim pHost As hostent Ptr = gethostbyname(@LocalName(0))
        '
        '' 填充SOCKADDR_IN结构
        'Dim addr_in As sockaddr_in
        'memcpy @addr_in.sin_addr, pHost->h_addr_list[0], 4
        'addr_in.sin_family = AF_INET
        'addr_in.sin_port = htons(1004)
        '
        '' 绑定套接字到本地网卡地址
        'If bind(sock, Cast(SOCKADDR Ptr, @addr_in), Len(addr_in)) <> 0 Then
        '    Print "Bind failed:" & WinErrorMsg(GetLastError)
        '    closesocket sock
        '    WSACleanup
        '    Return False
        'End If
        
        Dim addr_in As sockaddr_in
        addr_in.sin_family = AF_INET
        addr_in.sin_addr.S_addr = htonl(INADDR_ANY)
        addr_in.sin_port = htons(0)
        If bind(sock, Cast(SOCKADDR Ptr, @addr_in), Len(addr_in)) <> 0 Then
            Print "Bind failed: " & WSAGetLastError
            closesocket(sock)
            sock = INVALID_SOCKET
            Return False
        End If

        ' 设置IP头操作选项
        /'Dim flag As Long = 1
        If setsockopt(sock, IPPROTO_IP, IP_HDRINCL, Cast(LPCSTR, @flag), SizeOf(Long)) <> 0 Then
            Print "setsockopt failed:" & WinErrorMsg(GetLastError)
            closesocket sock
            WSACleanup
            Return False
        End If'/

        ' 设置SOCK_RAW为SIO_RCVALL，以便接收所有的IP包
        /'Dim dwValue As Long = 1
        If ioctlsocket(sock, SIO_RCVALL, @dwValue) <> 0 Then
            Print "ioctlsocket failed:" & WinErrorMsg(WSAGetLastError) & WSAGetLastError
            closesocket sock
            WSACleanup
            Return False
        End If'/

        ' 创建线程监听网络数据包
        IsStartSniffer = True
        lvwSniffer = ListView
        listenerThread = ThreadCreate(@NetworkListenerThread)
        Return True
    Else
        IsStartSniffer = False
        ThreadWait listenerThread
        Threaddetach listenerThread
        ' 关闭套接字
        closesocket sock
        ' 清理Winsock
        WSACleanup
    End If
End Function