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
    Dim buffer(BUFFER_SIZE - 1) As Byte
    Do While True ' IsStartSniffer
        'Print "1"
        Dim ret As Long = recv(sock, @buffer(0), BUFFER_SIZE, 0)
        If ret > 0 Then
            memcpy @ip, @buffer(0), SizeOf(IPHeader)
            memcpy @tcp, @buffer(ip.verIhl * 4), SizeOf(TCPHeader)
            lvwSniffer.AddItemColList,, GetProtocolTxt(ip.protocol), GetIPAddress(ip.saddr), GetIPAddress(ip.daddr), Str(ntohs(tcp.sourcePort)), Str(ntohs(tcp.destPort)), Str(ntohs(ip.totLen))
            Print "2"
        End If
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

Function SetSnifferStatus(ListView As Class_ListView, ByVal IsStart As BOOLEAN) As BOOLEAN
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
        Dim LocalName(511) As Byte
        gethostname @LocalName(0), 512

        ' 获取本地IP地址
        Dim pHost As hostent Ptr = gethostbyname(@LocalName(0))

        ' 填充SOCKADDR_IN结构
        Dim addr_in As sockaddr_in
        memcpy @addr_in.sin_addr, pHost->h_addr_list[0], 4
        addr_in.sin_family = AF_INET
        addr_in.sin_port = htons(1004)

        ' 绑定套接字到本地网卡地址
        If bind(sock, Cast(SOCKADDR Ptr, @addr_in), Len(addr_in)) <> 0 Then
            Print "Bind failed:" & WinErrorMsg(GetLastError)
            closesocket sock
            WSACleanup
            Return False
        End If

        ' 设置IP头操作选项
        Dim flag As Long = 1
        If setsockopt(sock, IPPROTO_IP, IP_HDRINCL, Cast(LPCSTR, @flag), SizeOf(Long)) <> 0 Then
            Print "setsockopt failed:" & WinErrorMsg(GetLastError)
            closesocket sock
            WSACleanup
            Return False
        End If

        ' 设置SOCK_RAW为SIO_RCVALL，以便接收所有的IP包
        Dim dwValue As Long = 1
        If ioctlsocket(sock, SIO_RCVALL, @dwValue) <> 0 Then
            Print "ioctlsocket failed:" & WinErrorMsg(GetLastError)
            closesocket sock
            WSACleanup
            Return False
        End If

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