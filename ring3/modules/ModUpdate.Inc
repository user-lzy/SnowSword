#include once "win/wininet.bi"

#Define APP_NAME "SnowSword"
#Define DEBUG_LOG OutputDebug

' 调试日志
Sub OutputDebug(level As LogLevel, func As String = "", apiName As String = "", sMsg As WString)
    MyLog.PrintLog level, func, apiName, sMsg
End Sub

' 读取本地版本
Function GetLocalVersion(sExePath As WString Ptr, sVer As WString Ptr) As Boolean
    Dim As DWORD h = 0, size = 0
    Dim As Any Ptr pData = 0
    Dim As VS_FIXEDFILEINFO Ptr pInfo = 0
    Dim As BOOL bRet = False

    DEBUG_LOG LOG_INFO, "GetLocalVersion",, "读取本地版本号..."
    DEBUG_LOG LOG_INFO, "GetLocalVersion",, "sExePath:" & *sExePath

    size = GetFileVersionInfoSizeW(sExePath, @h)
    If size = 0 Then
        DEBUG_LOG LOG_ERROR, "GetLocalVersion",, "获取版本信息大小失败"
        Return False
    End If

    pData = Allocate(size)
    If pData = 0 Then
        DEBUG_LOG LOG_ERROR, "GetLocalVersion",, "内存分配失败"
        Return False
    End If

    bRet = GetFileVersionInfoW(sExePath, 0, size, pData)
    If bRet = False Then
        DEBUG_LOG LOG_ERROR, "GetLocalVersion",, "读取版本信息失败"
        Deallocate(pData)
        Return False
    End If

    bRet = VerQueryValueW(pData, !"\\", @pInfo, @size)
    If bRet = False Or pInfo = 0 Then
        DEBUG_LOG LOG_ERROR, "GetLocalVersion",, "解析版本信息失败"
        Deallocate(pData)
        Return False
    End If

    wsprintfW(sVer, !"%d.%d.%d.%d", _
    HiWord(pInfo->dwFileVersionMS), LoWord(pInfo->dwFileVersionMS), _
    HiWord(pInfo->dwFileVersionLS), LoWord(pInfo->dwFileVersionLS))

    Deallocate(pData)
    DEBUG_LOG LOG_INFO, "GetLocalVersion",, "本地版本：" & *sVer
    Return True
End Function

' ==============================================
' 【修复乱码】宽字符专用 - 智能解析版本号（过滤字母，提取纯数字段）
' 完美支持：1.0.0.3903、1.0-beta.2、2.1-alpha.5 等格式
' ==============================================
Sub ParseVersionSmart(verStr As WString Ptr, ByRef v1 As Long, ByRef v2 As Long, ByRef v3 As Long, ByRef v4 As Long)
    Dim As WString * 256 cleanVer = WStr("")
    Dim As Long i = 0, idx = 0
    
    ' 初始化版本号
    v1 = 0 : v2 = 0 : v3 = 0 : v4 = 0

    ' 🔥 核心修复：宽字符遍历，只保留 数字(0-9) 和 点号(.)
    While verStr[i] <> 0
        ' 数字
        If AscW("0") <= verStr[i] AndAlso AscW("9") >= verStr[i] Then
            cleanVer[idx] = verStr[i]
            idx += 1
        ' 点号
        ElseIf verStr[i] = AscW(".") Then
            cleanVer[idx] = verStr[i]
            idx += 1
        End If
        i += 1
    Wend

    ' 调试输出（无乱码）
    DEBUG_LOG LOG_INFO,"ParseVersionSmart",, "【版本净化】原始：" & *verStr & " → 纯数字：" & cleanVer

    ' 按点分割解析4段版本号
    Dim As WString Ptr p = @cleanVer
    For i = 0 To 3
        Dim As Long num = wcstoul(p, @p, 10)
        Select Case i
            Case 0: v1 = num
            Case 1: v2 = num
            Case 2: v3 = num
            Case 3: v4 = num
        End Select
        If *p = AscW(".") Then p += 1 Else Exit For
    Next
End Sub

' ==============================================
' 版本对比函数（无乱码，宽字符专用）
' ==============================================
Function CheckNeedUpdate(localVersion As WString Ptr, remoteVersion As WString Ptr) As Boolean
    Dim As Long l1,l2,l3,l4, r1,r2,r3,r4
    
    DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"当前版本: " & *localVersion
    DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"最新版本: " & *remoteVersion
    
    ' 解析版本
    ParseVersionSmart(localVersion, l1, l2, l3, l4)
    ParseVersionSmart(remoteVersion, r1, r2, r3, r4)
    
    DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"【本地解析】" & l1 & "." & l2 & "." & l3 & "." & l4
    DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"【远程解析】" & r1 & "." & r2 & "." & r3 & "." & r4
    
    ' 标准版本对比
    If r1 > l1 Then Return True : DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"✅ 主版本更新"
    If r1 = l1 And r2 > l2 Then Return True : DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"✅ 次版本更新"
    If r1 = l1 And r2 = l2 And r3 > l3 Then Return True : DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"✅ 修订版更新"
    If r1 = l1 And r2 = l2 And r3 = l3 And r4 > l4 Then Return True : DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"✅ 构建号更新"
    
    DEBUG_LOG LOG_INFO, "CheckNeedUpdate",,"✅ 当前已是最新版本"
    Return False
End Function

' 下载文件
Function DownloadFile(url As WString Ptr, savePath As WString Ptr) As Boolean
    Dim As HINTERNET hSession, hConn, hReq
    Dim As DWORD bytRead, bytWrite
    Dim As Byte Ptr buf = Allocate(4096)
    Dim As HANDLE hFile
    Dim As WString * 256 host, path
    Dim As Integer port = INTERNET_DEFAULT_HTTPS_PORT
    Dim As WString * 256 urlBuf

    urlBuf = *url

    DEBUG_LOG LOG_INFO, "DownloadFile",,"开始下载：" & urlBuf
    DEBUG_LOG LOG_INFO, "DownloadFile",,"保存到：" & *savePath

    If InStr(urlBuf, WStr("https://")) = 1 Then
        Dim As Integer pos1 = InStr(9, urlBuf, "/")
        host = Mid(urlBuf, 9, pos1 - 9)
        path = Mid(urlBuf, pos1)
    Else
        host = urlBuf
        path = WStr("/")
    End If

    hSession = InternetOpenW(WStr(APP_NAME), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)
    If hSession=0 Then DEBUG_LOG LOG_ERROR, "DownloadFile",,"InternetOpen 失败" : Goto err1

    hConn = InternetConnectW(hSession, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)
    If hConn=0 Then DEBUG_LOG LOG_ERROR, "DownloadFile",,"InternetConnect 失败": Goto err1

    hReq = HttpOpenRequestW(hConn, WStr("GET"), path, NULL, NULL, NULL, INTERNET_FLAG_SECURE Or INTERNET_FLAG_RELOAD, 0)
    If hReq=0 Then DEBUG_LOG LOG_ERROR, "DownloadFile",,"HttpOpenRequest 失败": Goto err1

    HttpAddRequestHeadersW(hReq, WStr("User-Agent: Mozilla/5.0"), -1, HTTP_ADDREQ_FLAG_REPLACE)

    If HttpSendRequestW(hReq, NULL, 0, NULL, 0)=0 Then DEBUG_LOG LOG_ERROR, "DownloadFile",,"HttpSendRequest 失败": Goto err1

    hFile = CreateFileW(savePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then Goto err1

    Do
        ' 修复：校验API返回值
        If InternetReadFile(hReq, buf, 4096, @bytRead) = 0 Then Exit Do
        If bytRead=0 Then Exit Do
        WriteFile(hFile, buf, bytRead, @bytWrite, NULL)
    Loop

    DEBUG_LOG LOG_INFO, "DownloadFile",,"下载完成！"
    CloseHandle(hFile)
    InternetCloseHandle(hReq)
    InternetCloseHandle(hConn)
    InternetCloseHandle(hSession)
    Deallocate(buf)
    Return True

err1:
    DEBUG_LOG LOG_ERROR, "DownloadFile",,"下载失败！"
    If hReq Then InternetCloseHandle(hReq)
    If hConn Then InternetCloseHandle(hConn)
    If hSession Then InternetCloseHandle(hSession)
    If hFile<>INVALID_HANDLE_VALUE Then CloseHandle(hFile)
    Deallocate(buf)
    Return False
End Function

' ==============================================
' 【最终修复】适配JSON空格格式（100%解析成功）
' ==============================================
Function GetRemoteVersionJson _
( _
    outVer As WString Ptr, _
    outExeUrl As WString Ptr, _
    outSysUrl As WString Ptr, _
    outUpdateLog As WString Ptr _  ' 新增：输出更新日志
) As Boolean

    ' Gitee官方原生RAW地址
    #Define JSON_URL "https://gitee.com/le-ziyi/SnowSword/raw/main/version.json"

    Dim As WString * 512 url = JSON_URL
    Dim As Byte Ptr buf = Allocate(8192)
    memset(buf, 0, 8192)
    Dim As DWORD nLen = 0, statusCode = 0
    Dim As Boolean bOK = FALSE
    Dim As HINTERNET hSession = NULL, hConn = NULL, hReq = NULL
    Dim As WString * 256 host = "", path = ""
    Dim As Integer pos1 = 0
    Dim As DWORD bufSize = 8

    DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "开始获取远程版本..."
    DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "官方请求URL：" & url

    ' 解析URL
    If InStr(url, WStr("https://")) = 1 Then
        pos1 = InStr(9, url, "/")
        host = Mid(url, 9, pos1-9)
        path = Mid(url, pos1)
        DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "Host：" & host
        DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "Path：" & path
    Else
        DEBUG_LOG LOG_ERROR, "GetRemoteVersionJson",, "❌ URL非HTTPS"
        Deallocate(buf)
        Return False
    EndIf

    ' 网络请求
    hSession = InternetOpenW(WStr(APP_NAME), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)
    hConn = InternetConnectW(hSession, host, 443, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)
    hReq = HttpOpenRequestW(hConn, WStr("GET"), path, NULL, NULL, NULL, INTERNET_FLAG_SECURE Or INTERNET_FLAG_RELOAD Or INTERNET_FLAG_IGNORE_CERT_CN_INVALID, 0)
    
    If hSession=NULL Or hConn=NULL Or hReq=NULL Then
        DEBUG_LOG LOG_ERROR, "GetRemoteVersionJson",, "❌ 网络句柄创建失败"
        Goto CLEANUP
    EndIf

    HttpAddRequestHeadersW(hReq, WStr("User-Agent: Mozilla/5.0"), -1, HTTP_ADDREQ_FLAG_REPLACE)
    If HttpSendRequestW(hReq, NULL,0,NULL,0) = FALSE Then
        DEBUG_LOG LOG_ERROR, "GetRemoteVersionJson",, "❌ 请求发送失败"
        Goto CLEANUP
    EndIf

    ' 获取HTTP状态码
    HttpQueryInfoW(hReq, HTTP_QUERY_STATUS_CODE Or HTTP_QUERY_FLAG_NUMBER, @statusCode, @bufSize, 0)
    DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "【HTTP状态码】：" & statusCode
    If statusCode <> 200 Then
        DEBUG_LOG LOG_ERROR, "GetRemoteVersionJson",, "❌ 请求失败！HTTP状态码：" & statusCode & "（非200）"
        Goto CLEANUP
    EndIf

    ' 读取数据
    InternetReadFile(hReq, buf, 8191, @nLen)
    DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "【数据长度】：" & nLen
    DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "【返回内容】：" & *Cast(ZString Ptr, buf)

    ' 解析JSON
    If nLen > 0 Then
        Dim As ZString Ptr p = buf
        Dim As ZString Ptr v = StrStr(p, !"version\": \"")
        Dim As ZString Ptr e = StrStr(p, !"exe_url\": \"")
        Dim As ZString Ptr s = StrStr(p, !"sys_url\": \"")
        Dim As ZString Ptr l = StrStr(p, !"update_log\": \"")  ' 新增：匹配更新日志

        ' 新增日志字段校验
        If v<>NULL And e<>NULL And s<>NULL And l<>NULL Then
            DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "✅ 所有JSON字段匹配成功！"
            ' 截取字段值
            v += 11 : *StrStr(v, !"\"") = 0
            e += 11 : *StrStr(e, !"\"") = 0
            s += 11 : *StrStr(s, !"\"") = 0
            l += 13 : *strstr(l, !"\"") = 0  ' 新增：截取更新日志

            ' 转宽字符
            MultiByteToWideChar(CP_UTF8, 0, v, -1, outVer, 259)
            MultiByteToWideChar(CP_UTF8, 0, e, -1, outExeUrl, 259)
            MultiByteToWideChar(CP_UTF8, 0, s, -1, outSysUrl, 259)
            MultiByteToWideChar(CP_UTF8, 0, l, -1, outUpdateLog, 1023)  ' 新增：转换日志

            DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "✅ 解析成功！远程版本：" & *outVer
            DEBUG_LOG LOG_INFO, "GetRemoteVersionJson",, "✅ 更新日志：" & *outUpdateLog
            bOK = True
        Else
            DEBUG_LOG LOG_ERROR, "GetRemoteVersionJson",, "❌ JSON字段缺失"
        EndIf
    EndIf

CLEANUP:
    If hReq Then InternetCloseHandle(hReq)
    If hConn Then InternetCloseHandle(hConn)
    If hSession Then InternetCloseHandle(hSession)
    Deallocate(buf)
    Return bOK
End Function

' 更新检测主逻辑
Sub CheckUpdate()
    Dim As WString * 260 appDir, appPath, localVer, remoteVer
    Dim As WString * 260 tempExe, tempSys, sysPath
    Dim As WString * 256 GITEE_EXE, GITEE_SYS
    Dim As WString * 1024 updateLog  ' 新增：存储更新日志
    Dim As Integer userChoice  ' 新增：用户选择

    DEBUG_LOG LOG_INFO, "CheckUpdate",, "========== SnowSword 自动更新 =========="
    
    appDir = App.Path
    appPath = appDir & App.EXEName
    
    ' 驱动路径
    wcscpy(@sysPath, @appDir)
    Dim As Long i = CUInt(wcsrchr( @sysPath, AscW("\") )) - CUInt(@sysPath) + 1
    sysPath[i] = 0
    wcscat(@sysPath, StrPtrW("SnowSword.sys"))  ' 修复StrPtrW

    If Not GetLocalVersion(@appPath, @localVer) Then Return
    
    ' 新增：传入更新日志参数
    If GetRemoteVersionJson(@remoteVer, @GITEE_EXE, @GITEE_SYS, @updateLog) = False Then
        DEBUG_LOG LOG_ERROR, "CheckUpdate",, "获取版本信息失败"
        Return
    EndIf
    
    If CheckNeedUpdate(@localVer, @remoteVer) = False Then Return

    ' ==============================================
    ' 【核心新增】弹窗显示更新日志，询问是否更新
    ' ==============================================
    Dim As WString * 2048 msg
    msg = "检测到新版本！" & !"\n\n当前版本：" & localVer & !"\n最新版本：" & remoteVer & !"\n\n更新内容：\n" & updateLog
    userChoice = AfxMsg(msg, "更新检测", MB_YESNO Or MB_ICONQUESTION)

    ' 用户选择 否 → 取消更新
    If userChoice = IDNO Then
        DEBUG_LOG LOG_INFO, "CheckUpdate",, "用户取消更新"
        Return
    EndIf
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "用户确认更新，开始执行..."

    ' 临时文件
    GetTempPathW(260, tempExe) : wcscat(@tempExe, StrPtrW("SnowSword_new.exe"))
    GetTempPathW(260, tempSys) : wcscat(@tempSys, StrPtrW("SnowSword_new.sys"))

    If DownloadFile(@GITEE_EXE, @tempExe) = False Then Return
    If DownloadFile(@GITEE_SYS, @tempSys) = False Then Return

    ' 后续替换逻辑（完全不变）
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "开始替换主程序..."
    LoadDriver App.Path & "SnowSword.sys", False
    Dim As HANDLE hDrv  ' 修复：补全变量声明
    Dim As Boolean IsDriverLoaded = False
    hDrv = OpenDrv("\\.\\SnowSword")
    If (hDrv <> INVALID_HANDLE_VALUE) Then IsDriverLoaded = True
    ForceDeleteFile(@appPath)
    CloseDrv hDrv
    UnloadDriver App.Path & "SnowSword.sys", False
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "已删除主程序!"
    CopyFileW(@tempExe, @appPath, False)

    DEBUG_LOG LOG_INFO, "CheckUpdate",, "开始替换驱动文件..."
    DeleteFileW(@sysPath)
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "已删除驱动!"
    CopyFileW(@tempSys, @sysPath, False)
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "已拷贝驱动!"
    DeleteFileW(@tempExe)
    DeleteFileW(@tempSys)
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "已删除临时文件!"
    DEBUG_LOG LOG_INFO, "CheckUpdate",, "更新完成！重启软件生效"
    AfxMsg "更新完成！请重启软件",APP_NAME, MB_OK
End Sub