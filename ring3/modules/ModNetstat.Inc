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
    
    ' ==================== 列调整 ====================
    If IsFilter And lvwInternet.ColumnText(1) = "PID" Then
        lvwInternet.DeleteColumn 5   ' 删除"路径"列（索引变了）
        lvwInternet.DeleteColumn 1   ' 删除"PID"列
    ElseIf (Not IsFilter) And lvwInternet.ColumnText(1) <> "PID" Then
        lvwInternet.InsertColumn 1, "PID"
        lvwInternet.AddColumn "路径",, 500
    End If
    
    ' ==================== 预分配局部字符串缓冲区 ====================
    Dim wszProtocol     As WString * 8
    Dim wszPID          As WString * 16
    Dim wszState        As WString * 64
    Dim wszLocalAddr    As WString * 80    ' 合并后需要更大缓冲区
    Dim wszRemoteAddr   As WString * 80
    Dim wszPath         As WString * MAX_PATH
    
    ' ==================== 枚举TCP连接 ====================
    Dim lngSize As DWORD = 0, nRet As Long
    nRet = GetExtendedTcpTable(0, @lngSize, True, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
    Dim pTcpTable As PMIB_TCPTABLE_OWNER_PID = Allocate(lngSize)
    nRet = GetExtendedTcpTable(pTcpTable, @lngSize, True, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
    
    For i As Integer = 0 To pTcpTable->dwNumEntries - 1
        wszProtocol   = "TCP"
        wszState      = IPStateToString(pTcpTable->table(i).dwState)
        
        ' 合并 IP:Port
        Dim wszLIP As WString * 64 = GetIPAddress(pTcpTable->table(i).dwLocalAddr)
        Dim wszRIP As WString * 64 = GetIPAddress(pTcpTable->table(i).dwRemoteAddr)
        Dim wszLP  As WString * 16 = WStr(htons(pTcpTable->table(i).dwLocalPort))
        Dim wszRP  As WString * 16 = WStr(htons(pTcpTable->table(i).dwRemotePort))
        
        wszLocalAddr  = wszLIP & ":" & wszLP
        wszRemoteAddr = wszRIP & ":" & wszRP
        
        If Not IsFilter Then
            wszPID = WStr(pTcpTable->table(i).dwOwningPid)
            wszPath = GetProcessPath(pTcpTable->table(i).dwOwningPid)
            AddItemColListEx lvwInternet, 6,,, wszProtocol, wszPID, wszState, wszLocalAddr, wszRemoteAddr, wszPath
        Else
            If pTcpTable->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 4,,, wszProtocol, wszState, wszLocalAddr, wszRemoteAddr
        End If
        FF_DoEvents
    Next
    Deallocate pTcpTable
    
    ' ==================== 枚举TCP6连接 ====================
    lngSize = 0
    nRet = GetExtendedTcpTable(0, @lngSize, True, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0)
    Dim pTcp6Table As PMIB_TCP6TABLE_OWNER_PID = Allocate(lngSize)
    nRet = GetExtendedTcpTable(pTcp6Table, @lngSize, True, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0)
    
    For i As Integer = 0 To pTcp6Table->dwNumEntries - 1
        wszProtocol   = "TCP6"
        wszState      = IPStateToString(pTcp6Table->table(i).dwState)
        
        ' 合并 IP:Port
        Dim wszLIP6 As WString * 64 = GetIPv6Address(pTcp6Table->table(i).ucLocalAddr())
        Dim wszRIP6 As WString * 64 = GetIPv6Address(pTcp6Table->table(i).ucRemoteAddr())
        Dim wszLP6  As WString * 16 = WStr(htons(pTcp6Table->table(i).dwLocalPort))
        Dim wszRP6  As WString * 16 = WStr(htons(pTcp6Table->table(i).dwRemotePort))
        
        wszLocalAddr  = "[" & wszLIP6 & "]:" & wszLP6   ' IPv6 用方括号包裹
        wszRemoteAddr = "[" & wszRIP6 & "]:" & wszRP6
        
        If Not IsFilter Then
            wszPID = WStr(pTcp6Table->table(i).dwOwningPid)
            wszPath = GetProcessPath(pTcp6Table->table(i).dwOwningPid)
            AddItemColListEx lvwInternet, 6,,, wszProtocol, wszPID, wszState, wszLocalAddr, wszRemoteAddr, wszPath
        Else
            If pTcp6Table->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 4,,, wszProtocol, wszState, wszLocalAddr, wszRemoteAddr
        End If
        FF_DoEvents
    Next
    Deallocate pTcp6Table
    
    ' ==================== 枚举UDP连接 ====================
    lngSize = 0
    nRet = GetExtendedUdpTable(0, @lngSize, True, AF_INET, UDP_TABLE_OWNER_PID, 0)
    Dim pUdpTable As PMIB_UDPTABLE_OWNER_PID = Allocate(lngSize)
    nRet = GetExtendedUdpTable(pUdpTable, @lngSize, True, AF_INET, UDP_TABLE_OWNER_PID, 0)
    
    For i As Integer = 0 To pUdpTable->dwNumEntries - 1
        wszProtocol   = "UDP"
        wszState      = ""
        
        ' 合并 IP:Port
        Dim wszLIPU As WString * 64 = GetIPAddress(pUdpTable->table(i).dwLocalAddr)
        Dim wszLPU  As WString * 16 = WStr(htons(pUdpTable->table(i).dwLocalPort))
        
        wszLocalAddr  = wszLIPU & ":" & wszLPU
        wszRemoteAddr = ""   ' UDP无远程地址
        
        If Not IsFilter Then
            wszPID = WStr(pUdpTable->table(i).dwOwningPid)
            wszPath = GetProcessPath(pUdpTable->table(i).dwOwningPid)
            AddItemColListEx lvwInternet, 6,,, wszProtocol, wszPID, wszState, wszLocalAddr, wszRemoteAddr, wszPath
        Else
            If pUdpTable->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 4,,, wszProtocol, wszState, wszLocalAddr, wszRemoteAddr
        End If
        FF_DoEvents
    Next
    Deallocate pUdpTable
    
    ' ==================== 枚举UDP6连接 ====================
    lngSize = 0
    nRet = GetExtendedUdpTable(0, @lngSize, True, AF_INET6, UDP_TABLE_OWNER_PID, 0)
    Dim pUdp6Table As PMIB_UDP6TABLE_OWNER_PID = Allocate(lngSize)
    nRet = GetExtendedUdpTable(pUdp6Table, @lngSize, True, AF_INET6, UDP_TABLE_OWNER_PID, 0)
    
    For i As Integer = 0 To pUdp6Table->dwNumEntries - 1
        wszProtocol   = "UDP6"
        wszState      = ""
        
        ' 合并 IP:Port
        Dim wszLIPU6 As WString * 64 = GetIPv6Address(pUdp6Table->table(i).ucLocalAddr())
        Dim wszLPU6  As WString * 16 = WStr(htons(pUdp6Table->table(i).dwLocalPort))
        
        wszLocalAddr  = "[" & wszLIPU6 & "]:" & wszLPU6   ' IPv6 用方括号
        wszRemoteAddr = ""
        
        If Not IsFilter Then
            wszPID = WStr(pUdp6Table->table(i).dwOwningPid)
            wszPath = GetProcessPath(pUdp6Table->table(i).dwOwningPid)
            AddItemColListEx lvwInternet, 6,,, wszProtocol, wszPID, wszState, wszLocalAddr, wszRemoteAddr, wszPath
        Else
            If pUdp6Table->table(i).dwOwningPid <> PID Then Continue For
            AddItemColListEx lvwInternet, 4,,, wszProtocol, wszState, wszLocalAddr, wszRemoteAddr
        End If
        FF_DoEvents
    Next
    Deallocate pUdp6Table
    
    CommitListViewView lvwInternet
End Sub

' IP字符串 转 网络字节序DWORD
Function StringToIP(ByVal ipStr As String) As ULong
    Dim As String part(0 To 3)
    Dim As Long p, i, pos1
    Dim As ULong a, b, c, d

    ' 初始化
    For i = 0 To 3
        part(i) = ""
    Next

    p = 0
    pos1 = 1

    ' 按 . 分割 IP
    While pos1 <= Len(ipStr) And p < 4
        If Mid(ipStr, pos1, 1) = "." Then
            p += 1
            pos1 += 1
        Else
            part(p) += Mid(ipStr, pos1, 1)
            pos1 += 1
        End If
    Wend

    ' 转数字
    a = Val(part(0)) : b = Val(part(1))
    c = Val(part(2)) : d = Val(part(3))

    ' 范围检查
    If a > 255 Or b > 255 Or c > 255 Or d > 255 Then Return 0

    ' 关键：返回 网络字节序 (Big-Endian) DWORD
    ' 与 Windows inet_addr 完全一致
    Return (d Shl 24) Or (c Shl 16) Or (b Shl 8) Or a
End Function

Private Sub ParseAddrPort(ByVal AddrPort As String, ByRef IP As String, ByRef Port As Integer)
    Dim pos1 As Integer
    
    ' IPv6 格式: [::1]:80
    If Left(AddrPort, 1) = "[" Then
        pos1 = InStr(AddrPort, "]:")
        If pos1 > 0 Then
            IP = Mid(AddrPort, 2, pos - 2)           ' 提取 ::1
            Port = ValULng(Mid(AddrPort, pos1 + 2))   ' 提取 80
        Else
            IP = ""
            Port = 0
        End If
    Else
        ' IPv4 格式: 127.0.0.1:80
        pos1 = InStrRev(AddrPort, ":")
        If pos1 > 0 Then
            ip = Left(AddrPort, pos1 - 1)             ' 提取 127.0.0.1
            Port = ValULng(Mid(AddrPort, pos1 + 1))   ' 提取 80
        Else
            IP = AddrPort
            Port = 0
        End If
    End If
End Sub

' 关闭指定的TCP连接
' 参数：本地IP、本地端口、远程IP、远程端口
Public Function CloseTcpConnection(LocalIP As String, LocalPort As Integer, RemoteIP As String, RemotePort As Integer) As Boolean
    Dim tcpRow As MIB_TCPROW
    Dim ret As Long
    
    ' 填充连接信息
    tcpRow.dwLocalAddr = StringToIP(LocalIP)
    tcpRow.dwLocalPort = htons(LocalPort)
    tcpRow.dwRemoteAddr = StringToIP(RemoteIP)
    tcpRow.dwRemotePort = htons(RemotePort)
    
    ' 关键：把连接状态设为 DELETE_TCB = 强制关闭
    tcpRow.dwState = MIB_TCP_STATE_DELETE_TCB
    
    ' 调用API关闭
    ret = SetTcpEntry(@tcpRow)
    
    If ret = 0 Then
        MyLog.PrintLog LOG_INFO, "CloseTcpConnection", "SetTcpEntry", "连接关闭成功:" & LocalIP & ":" & LocalPort & " <-> " & RemoteIP & ":" & RemotePort
        Return True
    Else
        MyLog.PrintLog LOG_ERROR, "CloseTcpConnection", "SetTcpEntry", "关闭失败,错误:" & WinErrorMsg(ret) & "(" & ret & ")"
        Return False
    End If
End Function

' ==============================================
' 核心函数：枚举系统 Winsock SPI 提供者 (对齐ETW代码风格)
' ==============================================
' ============================================
' 统一的 SPI 信息结构（用于四种类型通用存储）
' ============================================
Type SPI_ENTRY
    spiType     As WString * 32   ' "Protocol"/"Protocol64"/"NameSpace"/"NameSpace64"
    index       As Integer
    szProtocol  As WString * 256  ' 协议/命名空间名称
    szDllPath   As WString * 1024 ' DLL路径
    szCLSID     As WString * 64   ' GUID字符串
    dwCatalogEntryId As DWORD      ' 目录条目ID（可选）
End Type

' ============================================
' 子函数：枚举 Protocol / Protocol64
' ============================================
Private Sub EnumProtocolSPI(lvwSPI As Class_ListView, ByRef spiType As WString, is64 As Boolean)
    Dim pBuffer As PVOID = NULL
    Dim bufSize As ULONG = 0
    Dim lProtocolsCount As Long = 0
    Dim pProtocolInfo As LPWSAPROTOCOL_INFOW
    Dim i As Integer
    Dim dwErr As DWORD
    
    ' 第一次调用：获取缓冲区大小
    If is64 Then
        lProtocolsCount = WSCEnumProtocols32(NULL, NULL, @bufSize, @dwErr)
    Else
        lProtocolsCount = WSCEnumProtocols(NULL, NULL, @bufSize, @dwErr)
    End If
    
    ' bufSize 会被设置为所需大小，即使返回 SOCKET_ERROR
    If bufSize = 0 Then
        ' 可能是32位系统上没有64位提供者，或确实没有协议
        Print spiType; ": 无数据或不可用 (错误:"; Hex(dwErr); ")"
        Exit Sub
    End If
    
    ' 分配内存
    pBuffer = Allocate(bufSize)
    If pBuffer = NULL Then
        Print spiType; ": 内存分配失败"
        Exit Sub
    End If
    
    pProtocolInfo = pBuffer
    
    ' 第二次调用：获取实际数据
    If is64 Then
        lProtocolsCount = WSCEnumProtocols32(NULL, pProtocolInfo, @bufSize, @dwErr)
    Else
        lProtocolsCount = WSCEnumProtocols(NULL, pProtocolInfo, @bufSize, @dwErr)
    End If
    
    If lProtocolsCount = SOCKET_ERROR Then
        Print spiType; ": 枚举失败，错误码："; Hex(dwErr)
        Deallocate(pBuffer)
        Exit Sub
    End If
    
    ' 遍历并添加到 ListView
    For i = 0 To lProtocolsCount - 1
        Dim pInfo As WSAPROTOCOL_INFOW Ptr = @pProtocolInfo[i]
        Dim szCLSID As WString * 64
        Dim szDllPath As WString * 1024
        Dim nPath As Long = 1024
        Dim nErrno As Long
        
        ' CLSID 转换
        StringFromGUID2(@pInfo->ProviderId, szCLSID, 64)
        
        ' 获取 DLL 路径
        If is64 Then
            If WSCGetProviderPath32(@pInfo->ProviderId, szDllPath, @nPath, @nErrno) = SOCKET_ERROR Then
                szDllPath = "获取失败(" & Hex(nErrno) & ")"
            End If
        Else
            If WSCGetProviderPath(@pInfo->ProviderId, szDllPath, @nPath, @nErrno) = SOCKET_ERROR Then
                szDllPath = "获取失败(" & Hex(nErrno) & ")"
            End If
        End If
        
        ' 替换环境变量
        szDllPath = FF_Replace(szDllPath, "%SystemRoot%", Environ("SystemRoot"))
        szDllPath = FF_Replace(szDllPath, "%SystemRoot%", Environ("SystemRoot"))  ' 有时大小写不同
        
        ' 添加到 ListView（6列：序号, 类型, 名称, DLL路径, 版本, GUID）
        AddItemColListEx lvwSPI, 7,,, _
            spiType, _                     ' 🔥 新增：类型列
            WStr(Format(i, "000000000000")), _       ' 序号
            pInfo->szProtocol, _ ' 协议名称
            szCLSID, _                         ' GUID
            szDllPath, _                    ' DLL路径
            WStr(GetFileCompany(szDllPath)), _ ' 公司名称
            WStr(GetFileDescription(szDllPath)) ' 文件描述
    Next
    
    Deallocate(pBuffer)
End Sub

' ============================================
' 获取 NameSpace 提供者的 信息（通过索引查注册表）
' ============================================
Function GetNameSpaceProviderInfoByIndex(index As Integer, is64 As Boolean, strInfo As StringW) As StringW
    Dim hKey As HKEY
    Dim szPath As WString * 1024
    Dim dwPathLen As DWORD = 1024 * 2
    Dim szRegPath As StringW
    Dim ret As LONG
    
    ' 构建注册表路径（子键是索引号，如 "000000000001"）
    Dim szIndex As StringW = RightW("000000000000" & WStr(index), 12)
    
    If is64 Then
        szRegPath = "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\NameSpace_Catalog5\Catalog_Entries64\" & szIndex
    Else
        szRegPath = "SYSTEM\CurrentControlSet\Services\WinSock2\Parameters\NameSpace_Catalog5\Catalog_Entries\" & szIndex
    End If
    
    ' 打开注册表键
    ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, StrPtrW(szRegPath), 0, KEY_QUERY_VALUE, @hKey)
    
    If ret <> ERROR_SUCCESS Then
        Print "regPath:" & "HKEY_LOCAL_MACHINE\" & szRegPath
        Return "注册表打开失败(" & WHex(ret) & ")"
    End If
    
    ' 读取 LibraryPath
    Dim dwType As DWORD = REG_SZ
    ret = RegQueryValueExW(hKey, StrPtrW(strInfo), NULL, @dwType, Cast(LPBYTE, @szPath), @dwPathLen)
    RegCloseKey(hKey)
    
    If ret = ERROR_SUCCESS AndAlso dwPathLen > 2 Then
        Print "szPath:" & szPath & " dwPathLen:" & dwPathLen
        szPath = FF_Replace(szPath, "%SystemRoot%", Environ("SystemRoot"))
        Return szPath
    Else
        Return "无法获取(" & WHex(ret) & ")"
    End If
End Function

' ============================================
' 子函数：枚举 NameSpace / NameSpace64
' ============================================
Private Sub EnumNameSpaceSPI(lvwSPI As Class_ListView, ByRef spiType As WString, is64 As Boolean)
    Dim pBuffer As PVOID = NULL
    Dim bufSize As ULONG = 0
    Dim lCount As LONG = 0
    Dim pNameSpaceInfo As LPWSANAMESPACE_INFOW
    Dim i As Integer
    
    ' ======================
    ' 第一次调用：获取缓冲区大小
    ' ======================
    ' 你的声明只有2个参数：bufSize指针 + buffer指针
    ' 传入 NULL buffer，让函数返回需要的缓冲区大小到 bufSize
    If is64 Then
        lCount = WSCEnumNameSpaceProviders32(@bufSize, NULL)
    Else
        lCount = WSCEnumNameSpaceProviders(@bufSize, NULL)
    End If
    
    ' 检查返回值：第一次调用通常返回 SOCKET_ERROR，但 bufSize 会被设置
    ' 如果 bufSize 仍为0，说明确实没有数据
    If bufSize = 0 Then
        Print spiType; ": 无数据或不可用"
        Exit Sub
    End If
    
    ' ======================
    ' 分配内存
    ' ======================
    pBuffer = Allocate(bufSize)
    If pBuffer = NULL Then
        Print spiType; ": 内存分配失败"
        Exit Sub
    End If
    
    pNameSpaceInfo = pBuffer
    
    ' ======================
    ' 第二次调用：获取实际数据
    ' ======================
    If is64 Then
        lCount = WSCEnumNameSpaceProviders32(@bufSize, pNameSpaceInfo)
    Else
        lCount = WSCEnumNameSpaceProviders(@bufSize, pNameSpaceInfo)
    End If
    
    ' 检查返回值
    If lCount = SOCKET_ERROR Then
        ' 没有 lpErrno 参数，直接用 GetLastError 或 WSAGetLastError
        Print spiType; ": 枚举失败，错误码："; Hex(GetLastError())
        Deallocate(pBuffer)
        Exit Sub
    End If
    
    ' ======================
    ' 遍历并添加到 ListView
    ' ======================
    For i = 0 To lCount - 1
        Dim pInfo As WSANAMESPACE_INFOW Ptr = @pNameSpaceInfo[i]
        Dim szCLSID As WString * 64
        Dim szDllPath As WString * 1024
        Dim nPath As Long = 1024
        Dim nErrno As Long
        
        ' CLSID 转换 (NSProviderId 就是 GUID)
        StringFromGUID2(@pInfo->NSProviderId, szCLSID, 64)
        
        ' ======================
        ' 获取 DLL 路径
        ' ======================
        szDllPath = GetNameSpaceProviderInfoByIndex(i + 1, is64, "LibraryPath")
        
        ' 替换环境变量
        szDllPath = FF_Replace(szDllPath, "%SystemRoot%", Environ("SystemRoot"))
        
        ' ======================
        ' 添加到 ListView
        ' ======================
        Dim szName As WString * 256 = *pInfo->lpszIdentifier
        
        AddItemColListEx lvwSPI, 7,,, _
            spiType, _ ' 类型列
            WStr(Format(i + 1, "000000000000")), _       ' 序号
            szName, _                      ' 命名空间名称
            szCLSID, _                      ' GUID
            szDllPath, _ ' DLL路径
            WStr(GetFileCompany(szDllPath)), _ ' 公司名称
            WStr(GetFileDescription(szDllPath)) ' 文件描述
    Next
    
    Deallocate(pBuffer)
End Sub

' ============================================
' 主函数：枚举所有 Winsock SPI（4种类型）
' ============================================
Sub GetWinsockSPIList(lvwSPI As Class_ListView)
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
    ' 3. 依次枚举四种类型
    ' ======================
    
    ' --- 3.1 Protocol (32位/默认) ---
    EnumProtocolSPI lvwSPI, "Protocol", False
    
    ' --- 3.2 Protocol64 ---
    EnumProtocolSPI lvwSPI, "Protocol64", True
    
    ' --- 3.3 NameSpace ---
    EnumNameSpaceSPI lvwSPI, "NameSpace", False
    
    ' --- 3.4 NameSpace64 ---
    EnumNameSpaceSPI lvwSPI, "NameSpace64", True
    
    ' ======================
    ' 4. 释放资源 & 刷新
    ' ======================
    WSACleanup()
    
    CommitListViewView lvwSPI
End Sub

' ============================================
' 统一移除函数：支持 Protocol / Protocol64 / NameSpace / NameSpace64
' ============================================
Function RemoveWinsockSPI(ByRef wszProviderId As WString, ByRef spiType As WString) As Boolean
    Dim ProviderId As GUID
    Dim lResult As Long
    Dim nErrno As Long
    
    ' GUID 字符串转结构体
    If CLSIDFromString(wszProviderId, @ProviderId) <> S_OK Then
        Print "无效的GUID: " & wszProviderId
        Return False
    End If
    
    ' 根据类型调用不同的卸载函数
    Select Case spiType
        Case "Protocol"
            lResult = WSCDeinstallProvider32(@ProviderId, @nErrno)
            
        Case "Protocol64"
            lResult = WSCDeinstallProvider(@ProviderId, @nErrno)
            
        Case "NameSpace"
            lResult = WSCUnInstallNameSpace32(@ProviderId)
            
        Case "NameSpace64"
            lResult = WSCUnInstallNameSpace(@ProviderId)
            
        Case Else
            Print "未知的SPI类型: " & spiType
            Return False
    End Select
    
    ' 错误处理
    If lResult = SOCKET_ERROR Then
        Print "移除 " & spiType & " [" & wszProviderId & "] 失败，错误码："; Hex(nErrno)
        Return False
    Else
        Print "移除 " & spiType & " [" & wszProviderId & "] 成功！"
        Return True
    End If
End Function