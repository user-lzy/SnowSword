' ===================== CUI Mode Module =====================
Dim Shared g_CuiMode As Boolean = False
Dim Shared g_OutputJson As Boolean = False
Dim Shared g_AgentPipeName As WString * 128
Dim Shared g_hAgentPipe As HANDLE = INVALID_HANDLE_VALUE
Dim Shared g_InAgentMode As Boolean = False
' 【新增】单次命令字符串（用于 -c "command string" 模式）
Dim Shared g_SingleCommand As WString * 1024
' 【新增】顶层参数错误 / 顶层帮助 / 版本
Dim Shared g_TopLevelError As WString * 256
Dim Shared g_ShowTopHelp As Boolean = False
Dim Shared g_ShowVersion As Boolean = False

' ===================== 管道捕获状态（| > >> findstr/sort/head）=====================
Dim Shared g_pipeCapture As Boolean = False     ' 正在捕获命令输出
Dim Shared g_pipeBuffer As StringW = ""          ' 捕获到的原始输出文本
Dim Shared g_forceText As Boolean = False        ' 强制文本(分隔符)输出，绕过 --format json
Dim Shared g_pipeCaptureHold As Boolean = False ' 交互提示直出标志（不进捕获缓冲）
Dim Shared g_cwd As StringW = WSpace(MAX_PATH)

#define vbCrlfW WChr(13, 10)
#define CUI_MAX_COLUMNS 32

Sub ParseArgv(ByVal argc As Long, argv As WString Ptr Ptr)
    ' 重置全局标记
    g_CuiMode = False
    g_OutputJson = False
    g_InAgentMode = False
    g_AgentPipeName = ""
    g_SingleCommand = ""
    g_TopLevelError = ""
    g_ShowTopHelp = False
    g_ShowVersion = False

    Dim i As Long = 1
    While i < argc
        Dim argRaw As WString * MAX_PATH = *argv[i]
        Dim arg As WString * MAX_PATH = LCaseW(argRaw)

        ' ---- 单 token 形式 --format=json ----
        If LeftW(arg, 8) = "--format=" Then
            If MidW(arg, 9) = "json" Then
                g_OutputJson = True
            Else
                g_TopLevelError = "Invalid --format value: " & argRaw
            End If
            i += 1
            Continue While
        End If

        Select Case arg
        Case "--cli"
            g_CuiMode = True
        Case "--format"
            If (i + 1) >= argc Then
                g_TopLevelError = "Option '--format' requires a value (json)."
            Else
                Dim fmtArg As WString * MAX_PATH = LCaseW(*argv[i+1])
                If fmtArg = "json" Then
                    g_OutputJson = True
                Else
                    g_TopLevelError = "Invalid --format value '" & *argv[i+1] & "' (expected 'json')."
                End If
                i += 1
            End If
        Case "--help", "-h", "/?"
            g_ShowTopHelp = True
        Case "--version", "-v"
            g_ShowVersion = True
        Case "--agent-pipe"
            If (i + 1) >= argc Then
                g_TopLevelError = "Option '--agent-pipe' requires a pipe name."
            Else
                g_InAgentMode = True
                g_AgentPipeName = *argv[i+1]
                g_CuiMode = True ' agent-pipe 本身属于CUI模式
                i += 1
            End If
        Case "-c", "--command"
            If (i + 1) >= argc Then
                g_TopLevelError = "Option '-c/--command' requires a command string."
            Else
                g_SingleCommand = *argv[i+1]
                If LenW(g_SingleCommand) = 0 Then
                    g_TopLevelError = "Empty command after '-c/--command'."
                End If
                g_CuiMode = True
                i += 1
            End If
        Case Else
            ' 未知/多余顶层参数：不再静默忽略
            If LeftW(arg, 1) = "-" OrElse LeftW(arg, 1) = "/" Then
                g_TopLevelError = "Unknown option: " & argRaw
            Else
                g_TopLevelError = "Unexpected argument: " & argRaw
            End If
        End Select
        i += 1
    Wend
End Sub

' ============================================
' 全局输出路由：Agent 管道模式 → PipeWrite，控制台模式 → WriteConsoleW
' ============================================
Private Function _PipeWriteRaw(ByVal pwText As WString Ptr) As Boolean
    If g_hAgentPipe = INVALID_HANDLE_VALUE OrElse pwText = NULL Then Return False
    Dim written As DWORD
    Dim cbNeeded As Integer = WideCharToMultiByte(CP_UTF8, 0, _
        pwText, lstrlenW(pwText), NULL, 0, NULL, NULL)
    If cbNeeded <= 0 Then Return True  ' 空内容视为成功
    Dim utf8Buf As ZString Ptr = Allocate(cbNeeded + 1)
    If utf8Buf = NULL Then Return False
    WideCharToMultiByte(CP_UTF8, 0, pwText, lstrlenW(pwText), _
        utf8Buf, cbNeeded, NULL, NULL)
    Dim bOk As Boolean = WriteFile(g_hAgentPipe, utf8Buf, cbNeeded, @written, NULL)
    If bOk = 0 Then
        Dim dwErr As DWORD = GetLastError()
        ' 109=ERROR_BROKEN_PIPE, 232=ERROR_NO_DATA, 233=ERROR_PIPE_NOT_CONNECTED
        If dwErr = 109 OrElse dwErr = 232 OrElse dwErr = 233 Then
            CloseHandle(g_hAgentPipe)
            g_hAgentPipe = INVALID_HANDLE_VALUE
        End If
    End If
    Deallocate(utf8Buf)
    Return bOk
End Function

' ---------- 重载1：接受 StringW ----------
Sub SafeStdOut Overload(ByVal szText As StringW)
    ' 【新增】管道捕获：追加到缓冲，不直接输出
    If g_pipeCapture AndAlso Not g_pipeCaptureHold Then
        g_pipeBuffer &= szText
        Exit Sub
    End If
    
    If g_InAgentMode AndAlso g_hAgentPipe <> INVALID_HANDLE_VALUE Then
        ' Agent 模式：走管道（已有 UTF-8 转换）
        If Not _PipeWriteRaw(StrPtrW(szText)) Then
            OutputDebugStringW(StrPtrW("PipeWrite failed, err=" & GetLastError()))
        End If
        Exit Sub
    End If
    
    Dim hOut As HANDLE = GetStdHandle(STD_OUTPUT_HANDLE)
    If hOut = INVALID_HANDLE_VALUE OrElse hOut = NULL Then
        OutputDebugStringW(StrPtrW(szText))
        Exit Sub
    End If
    
    ' ========== 关键修复：检测是否被重定向（管道/文件）==========
    Dim dwMode As DWORD
    Dim isConsole As Boolean = GetConsoleMode(hOut, @dwMode)
    
    If isConsole Then
        ' ===== 控制台模式：用 WriteConsoleW =====
        Dim written As DWORD
        WriteConsoleW(hOut, StrPtrW(szText), LenW(szText), @written, NULL)
    Else
        ' ===== 管道/重定向模式：转 UTF-8 后 WriteFile =====
        Dim cbNeeded As Integer = WideCharToMultiByte(CP_UTF8, 0, _
            StrPtrW(szText), LenW(szText), NULL, 0, NULL, NULL)
        If cbNeeded <= 0 Then Exit Sub
        
        Dim utf8Buf As ZString Ptr = Allocate(cbNeeded)
        If utf8Buf = NULL Then Exit Sub
        
        WideCharToMultiByte(CP_UTF8, 0, StrPtrW(szText), LenW(szText), _
            utf8Buf, cbNeeded, NULL, NULL)
        
        Dim written As DWORD
        WriteFile(hOut, utf8Buf, cbNeeded, @written, NULL)
        
        Deallocate(utf8Buf)
    End If
End Sub

' ---------- 重载2：接受 WString Ptr ----------
Sub SafeStdOut Overload(ByVal pwText As WString Ptr)
    If pwText = NULL Then Exit Sub
    
    ' 【新增】管道捕获：追加到缓冲，不直接输出
    If g_pipeCapture AndAlso Not g_pipeCaptureHold Then
        g_pipeBuffer &= *pwText
        Exit Sub
    End If
    
    If g_InAgentMode AndAlso g_hAgentPipe <> INVALID_HANDLE_VALUE Then
        If Not _PipeWriteRaw(pwText) Then
            OutputDebugStringW(StrPtrW("PipeWrite failed, err=" & GetLastError()))
        End If
        Exit Sub
    End If
    
    Dim hOut As HANDLE = GetStdHandle(STD_OUTPUT_HANDLE)
    If hOut = INVALID_HANDLE_VALUE OrElse hOut = NULL Then
        OutputDebugStringW(pwText)
        Exit Sub
    End If
    
    Dim dwMode As DWORD
    Dim isConsole As Boolean = GetConsoleMode(hOut, @dwMode)
    Dim textLen As Long = lstrlenW(pwText)
    
    If isConsole Then
        Dim written As DWORD
        WriteConsoleW(hOut, pwText, textLen, @written, NULL)
    Else
        Dim cbNeeded As Integer = WideCharToMultiByte(CP_UTF8, 0, _
            pwText, textLen, NULL, 0, NULL, NULL)
        If cbNeeded <= 0 Then Exit Sub
        
        Dim utf8Buf As ZString Ptr = Allocate(cbNeeded)
        If utf8Buf = NULL Then Exit Sub
        
        WideCharToMultiByte(CP_UTF8, 0, pwText, textLen, _
            utf8Buf, cbNeeded, NULL, NULL)
        
        Dim written As DWORD
        WriteFile(hOut, utf8Buf, cbNeeded, @written, NULL)
        
        Deallocate(utf8Buf)
    End If
End Sub

' ---------- 直出通道：绕过管道捕获，用于交互提示（如 Confirm? [y/N]:） ----------
Sub SafeStdOutDirect(ByVal szText As StringW)
    Dim savedHold As Boolean = g_pipeCaptureHold
    g_pipeCaptureHold = True
    SafeStdOut(szText)
    g_pipeCaptureHold = savedHold
End Sub

' ============================================
' 管道输入读取：UTF-8 字节流 → WString
' ============================================
Function PipeReadLine(ByVal outLine As WString Ptr, ByVal maxChars As Integer) As Boolean
    If g_hAgentPipe = INVALID_HANDLE_VALUE OrElse outLine = NULL OrElse maxChars <= 0 Then Return False
    
    Dim buf As ZString * INPUT_BUF_SIZE
    Dim totalRead As DWORD = 0
    Dim ch As Byte, br As DWORD
    
    Do
        Dim rfOk As Boolean = ReadFile(g_hAgentPipe, @ch, 1, @br, NULL)
        If rfOk = 0 OrElse br = 0 Then
            ' 客户端断开或管道错误（109, 232, 233）
            Return False
        End If
        If ch = 10 Then Exit Do          ' LF：行结束
        If ch <> 13 Then                 ' 忽略 CR
            buf[totalRead] = ch
            totalRead += 1
            If totalRead >= INPUT_BUF_SIZE - 2 Then Exit Do
        End If
    Loop
    
    buf[totalRead] = 0
    If totalRead = 0 Then
        ' 空行：返回空字符串（非失败）
        outLine[0] = 0
        Return True
    End If
    
    Dim cwNeeded As Integer = MultiByteToWideChar(CP_UTF8, 0, @buf, totalRead, NULL, 0)
    If cwNeeded <= 0 OrElse cwNeeded >= maxChars Then Return False
    
    MultiByteToWideChar(CP_UTF8, 0, @buf, totalRead, outLine, maxChars)
    outLine[cwNeeded] = 0
    Return True
End Function

' ===================== CLI 扁平数据导出器 =====================
' 适用场景：ListView、注册表值列表、进程列表等无层级数据
' 特征：无缩进、无分组层级、纯行列对齐

Type CuiFlatRow
    Cols(CUI_MAX_COLUMNS - 1) As StringW
    ColCount As Long
End Type

Sub CuiPrintFlatData( _
    ByVal typeName As StringW, _
    rows() As CuiFlatRow, _
    colNames() As StringW, _
    ByVal colCount As Long _
)
    
    SafeStdOut("===SNOWSWORD_CUI_START===" & vbCrlfW)
    SafeStdOut("MODULE:" & typeName & vbCrlfW)
    
    Dim rowCount As Long = UBound(rows) + 1
    If rowCount <= 0 OrElse colCount <= 0 Then
        SafeStdOut("ERROR:No data available" & vbCrlfW)
        SafeStdOut("ROWCOUNT:0" & vbCrlfW)
        SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
        Exit Sub
    End If
    
    SafeStdOut("ROWCOUNT:" & rowCount & vbCrlfW)
    
    ' ==================== JSON 模式 ====================
    If g_OutputJson AndAlso Not g_forceText Then
        SafeStdOut("DATA_START" & vbCrlfW)
        SafeStdOut("[" & vbCrlfW)
        
        For i As Long = 0 To rowCount - 1
            Dim r As CuiFlatRow = rows(i)
            SafeStdOut("  {")
            
            Dim actualCols As Long = IIf(r.ColCount < colCount, r.ColCount, colCount)
            For c As Long = 0 To actualCols - 1
                SafeStdOut("""" & CuiEscapeJson(colNames(c)) & """:""" & CuiEscapeJson(r.Cols(c)) & """")
                If c < actualCols - 1 Then SafeStdOut(",")
            Next
            
            For c As Long = actualCols To colCount - 1
                SafeStdOut(",""" & CuiEscapeJson(colNames(c)) & """:""""")
            Next
            
            SafeStdOut("}")
            If i < rowCount - 1 Then SafeStdOut("," & vbCrlfW)
        Next
        
        SafeStdOut(vbCrlfW & "]" & vbCrlfW)
        SafeStdOut("DATA_END" & vbCrlfW)
        
    ' ==================== Pipe 分隔符模式 ====================
    Else
        SafeStdOut("COLUMNS:")
        For c As Long = 0 To colCount - 1
            If c > 0 Then SafeStdOut("|")
            SafeStdOut(colNames(c))
        Next
        SafeStdOut(vbCrlfW)
        
        SafeStdOut("DATA_START" & vbCrlfW)
        
        For i As Long = 0 To rowCount - 1
            Dim r As CuiFlatRow = rows(i)
            Dim actualCols As Long = IIf(r.ColCount < colCount, r.ColCount, colCount)
            
            For c As Long = 0 To actualCols - 1
                If c > 0 Then SafeStdOut("|")
                SafeStdOut(r.Cols(c))
            Next
            
            For c As Long = actualCols To colCount - 1
                SafeStdOut("|")
            Next
            
            SafeStdOut(vbCrlfW)
        Next
        
        SafeStdOut("DATA_END" & vbCrlfW)
    End If
    
    SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
End Sub

' ===================== ListView 数据导出器 =====================
Sub CuiPrintListViewData()
    Dim ctx As ListViewContext Ptr = Cast(ListViewContext Ptr, GetWindowLongPtr(FrmMain.ListView1.hWnd, GWLP_USERDATA))
    
    SafeStdOut("===SNOWSWORD_CUI_START===" & vbCrlfW)
    SafeStdOut("MODULE:" & CurrentInformation.intType & vbCrlfW)
    
    If ctx = NULL OrElse ctx->Snap = NULL Then
        SafeStdOut("ERROR:No data available" & vbCrlfW)
        SafeStdOut("ROWCOUNT:0" & vbCrlfW)
        SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
        Exit Sub
    End If
    
    Dim snap As ListViewSnapshot Ptr = ctx->Snap
    Dim rowCount As Long = snap->RowCount
    SafeStdOut("ROWCOUNT:" & rowCount & vbCrlfW)
    
    ' JSON 模式
    If g_OutputJson AndAlso Not g_forceText Then
        SafeStdOut("DATA_START" & vbCrlfW)
        SafeStdOut("[" & vbCrlfW)
        For i As Long = 0 To rowCount - 1
            Dim r As LVRow Ptr = snap->Rows[i]
            If r = NULL Then Continue For
            SafeStdOut("  {")
            For c As Long = 0 To r->ColCount - 1
                Dim colName As StringW = FrmMain.ListView1.ColumnText(c)
                If c > 0 Then SafeStdOut(",")
                ' 【修复】NULL 指针保护，防止访问冲突崩溃
                If r->Cols[c] <> NULL Then
                    SafeStdOut("""" & CuiEscapeJson(colName) & """:""" &  CuiEscapeJson(*r->Cols[c]) & """")
                Else
                    SafeStdOut("""" &  CuiEscapeJson(colName) & """:""""")
                End If
            Next
            SafeStdOut("}" & vbCrlfW)
            If i < rowCount - 1 Then SafeStdOut(",")
        Next
        SafeStdOut("]" & vbCrlfW)
        SafeStdOut("DATA_END" & vbCrlfW)
        
    ' Pipe 分隔符模式 (默认)
    Else
        ' 输出列头
        SafeStdOut("COLUMNS:")
        For c As Integer = 0 To FrmMain.ListView1.ColumnCount - 1
            If c > 0 Then SafeStdOut("|")
            SafeStdOut(FrmMain.ListView1.ColumnText(c))
        Next
        SafeStdOut(vbCrlfW)
        
        SafeStdOut("DATA_START" & vbCrlfW)
        For i As Long = 0 To rowCount - 1
            Dim r As LVRow Ptr = snap->Rows[i]
            If r = NULL Then Continue For
            For c As Long = 0 To r->ColCount - 1
                If c > 0 Then SafeStdOut("|")
                ' 【修复】NULL 指针保护
                If r->Cols[c] <> NULL Then
                    SafeStdOut(r->Cols[c])
                Else
                    SafeStdOut("")
                End If
            Next
            SafeStdOut(vbCrlfW)
        Next
        SafeStdOut("DATA_END" & vbCrlfW)
    End If
    
    SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
    
    ' 输出完毕后，清空隐藏的ListView，为下一次查询腾出空间
    DeleteAllItemsEx FrmMain.ListView1
    ' 【修复】防止 DeleteAllItemsEx 内部已释放 snap 导致的双重释放/堆损坏
    'If ctx <> NULL Then ctx->Snap = NULL
End Sub

' ===================== CLI 统一导出引擎 v2 =====================
' 新增：子行缩进支持（Group + 缩进层级）

Type CuiExportRow
    Cols(CUI_MAX_COLUMNS - 1) As StringW
    ColCount As Long
    IndentLevel As Long      ' 0=根, 1=一级子节点, 2=二级子节点...
    IsGroup As Boolean       ' 是否为分组行（视觉上可区分）
    GroupLabel As StringW    ' 分组标识文本
End Type

' JSON 字符串转义
Function CuiEscapeJson(ByRef src As StringW) As StringW
    Dim result As StringW = ""
    For i As Long = 0 To Len(src) - 1
        Dim ch As Long = src[i]
        Select Case ch
        Case 34:  result &= "\"""
        Case 92:  result &= "\\"
        Case 8:   result &= "\b"
        Case 12:  result &= "\f"
        Case 10:  result &= "\n"
        Case 13:  result &= "\r"
        Case 9:   result &= "\t"
        Case Else
            If ch < 32 Then
                result &= "\u00" & Hex(ch, 2)
            Else
                result &= WChr(ch)
            End If
        End Select
    Next
    Return result
End Function

' 生成缩进前缀
Function CuiIndentPrefix(ByVal level As Long) As StringW
    If level <= 0 Then Return ""
    Dim prefix As StringW = ""
    For i As Long = 1 To level
        prefix &= "----"
    Next
    Return prefix
End Function

' 统一 TreeList 数据导出
Sub CuiPrintTreeListData( _
    ByVal typeName As StringW, _
    rows() As CuiExportRow, _
    colNames() As StringW, _
    ByVal colCount As Long _
)
    
    SafeStdOut("===SNOWSWORD_CUI_START===" & vbCrlfW)
    SafeStdOut("MODULE:" & typeName & vbCrlfW)
    
    Dim rowCount As Long = UBound(rows) + 1
    If rowCount <= 0 OrElse colCount <= 0 Then
        SafeStdOut("ERROR:No data available" & vbCrlfW)
        SafeStdOut("ROWCOUNT:0" & vbCrlfW)
        SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
        Exit Sub
    End If
    
    SafeStdOut("ROWCOUNT:" & rowCount & vbCrlfW)
    
    ' ==================== JSON 模式 ====================
    If g_OutputJson AndAlso Not g_forceText Then
        SafeStdOut("DATA_START" & vbCrlfW)
        SafeStdOut("[" & vbCrlfW)
        
        For i As Long = 0 To rowCount - 1
            Dim r As CuiExportRow = rows(i)
            SafeStdOut("  {")
            
            ' 注入缩进元数据
            SafeStdOut("""_indent"":" & r.IndentLevel & ",")
            SafeStdOut("""_isGroup"":" & IIf(r.IsGroup, "true", "false") & ",")
            
            Dim actualCols As Long = IIf(r.ColCount < colCount, r.ColCount, colCount)
            For c As Long = 0 To actualCols - 1
                Dim colName As StringW = colNames(c)
                Dim cellVal As StringW = r.Cols(c)
                
                ' 第一列附加缩进前缀（视觉辅助）
                If c = 0 AndAlso r.IndentLevel > 0 Then
                    cellVal = CuiIndentPrefix(r.IndentLevel) & cellVal
                End If
                
                SafeStdOut("""" & CuiEscapeJson(colName) & """:""" & CuiEscapeJson(cellVal) & """")
                If c < actualCols - 1 Then SafeStdOut(",")
            Next
            
            For c As Long = actualCols To colCount - 1
                SafeStdOut(",""" & CuiEscapeJson(colNames(c)) & """:""""")
            Next
            
            SafeStdOut("}")
            If i < rowCount - 1 Then SafeStdOut("," & vbCrlfW)
        Next
        
        SafeStdOut(vbCrlfW & "]" & vbCrlfW)
        SafeStdOut("DATA_END" & vbCrlfW)
        
    ' ==================== Pipe 分隔符模式 ====================
    Else
        ' 列头
        SafeStdOut("COLUMNS:")
        For c As Long = 0 To colCount - 1
            If c > 0 Then SafeStdOut("|")
            SafeStdOut(colNames(c))
        Next
        SafeStdOut(vbCrlfW)
        
        SafeStdOut("DATA_START" & vbCrlfW)
        
        For i As Long = 0 To rowCount - 1
            Dim r As CuiExportRow = rows(i)
            Dim actualCols As Long = IIf(r.ColCount < colCount, r.ColCount, colCount)
            
            For c As Long = 0 To actualCols - 1
                If c > 0 Then SafeStdOut("|")
                
                Dim cellVal As StringW = r.Cols(c)
                ' 第一列附加缩进前缀
                If c = 0 AndAlso r.IndentLevel > 0 Then
                    cellVal = CuiIndentPrefix(r.IndentLevel) & cellVal
                End If
                
                SafeStdOut(cellVal)
            Next
            
            ' 不足列补空
            For c As Long = actualCols To colCount - 1
                SafeStdOut("|")
            Next
            
            SafeStdOut(vbCrlfW)
        Next
        
        SafeStdOut("DATA_END" & vbCrlfW)
    End If
    
    SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
End Sub

' ========== 自定义 TrimW：从左右两侧删除 trimset 中出现的任意字符 ==========
Function MyTrimW(wszStr As String, trimset As String = " ") As StringW
    Dim As Integer i, j, n, m
    Dim As Boolean isTrimChar
    
    n = LenW(wszStr)
    m = LenW(trimset)
    If n = 0 Or m = 0 Then Return wszStr
    
    ' --- 左边界：找第一个不在 trimset 中的字符 ---
    Dim As Integer leftIdx = 0
    For i = 0 To n - 1
        isTrimChar = False
        For j = 0 To m - 1
            If wszStr[i] = trimset[j] Then
                isTrimChar = True
                Exit For
            End If
        Next
        If Not isTrimChar Then
            leftIdx = i
            Exit For
        End If
    Next
    
    ' 如果全是 trim 字符，返回空串
    If isTrimChar And i >= n Then Return ""
    
    ' --- 右边界：找最后一个不在 trimset 中的字符 ---
    Dim As Integer rightIdx = n - 1
    For i = n - 1 To leftIdx Step -1
        isTrimChar = False
        For j = 0 To m - 1
            If wszStr[i] = trimset[j] Then
                isTrimChar = True
                Exit For
            End If
        Next
        If Not isTrimChar Then
            rightIdx = i
            Exit For
        End If
    Next
    
    ' 提取并返回子串
    Return MidW(wszStr, leftIdx + 1, rightIdx - leftIdx + 1)
End Function

' ===================== 命令注册表与解析层 =====================
Const INPUT_BUF_SIZE    = 4096   ' 【修复P7】原1024，深层路径/长命令易截断
Const MAX_REG_ENTRIES   = 128

Type CmdHelpExtender
    header  As WString * 64      ' 区块标题，如 "Memory Write Specifics:"
    lines(7) As WString * 256    ' 最多8行详细说明
    lineCount As Integer
    example1 As WString * 128    ' 示例1
    example2 As WString * 128    ' 示例2
End Type

Enum ParamMode
    PM_NONE      = 0   ' 不支持/不需要
    PM_OPTIONAL  = 1   ' 支持，省略时查全部
    PM_REQUIRED  = 2   ' 必须提供，否则报错
End Enum

Type CmdEntry
    category      As WString * 64
    action        As WString * 32
    aliases       As WString * 128   ' 逗号分隔中文别名，如 "进程,程序"
    needsDriver   As Boolean
    pidMode       As ParamMode    ' ← 替换 needsPid + pidOptional
    tidMode       As ParamMode    ' ← 同理，tid/path/addr 统一
    pathMode      As ParamMode
    addrMode      As ParamMode
    isDestructive As Boolean         ' 【安全】是否需要确认
    helpText      As Wstring * 512
    hasExtHelp    As Boolean         ' ← 新增：是否有扩展帮助
    extHelp       As CmdHelpExtender ' ← 新增：扩展帮助内容
End Type

Type ParsedCmd
    category   As WString * 64
    action     As WString * 32
    pid        As DWORD
    tid        As DWORD
    path       As WString * MAX_PATH
    addr       As ULONG64
    SIZE As DWORD
    hexData    As Wstring * 1024   ' ← 新增：接收原始十六进制字符串
    bytdata       As Byte Ptr
    regName    As Wstring * 256
    regType    As WString * 32
    force      As Boolean
    dryRun     As Boolean
    showHelp   As Boolean            ' 【修复P13】内联 --help
    hasError   As Boolean
    errorMsg   As WString * 256
    unknownArg As WString * 64
    
    ' 【新增】显式参数提供标志，避免 PID=0/TID=0/Path="" 被误判为未提供
    hasPid     As Boolean
    hasTid     As Boolean
    hasPath    As Boolean
    hasAddr    As Boolean
End Type

Dim Shared g_registry(MAX_REG_ENTRIES) As CmdEntry
Dim Shared g_regCount As Integer = 0

' --------------- 注册表辅助 ---------------
Sub RegisterCmd(ByVal cat As StringW, ByVal act As StringW, ByVal ali As StringW, _
                ByVal drv As Boolean, _
                ByVal pidMd As ParamMode, ByVal tidMd As ParamMode, _
                ByVal pathMd As ParamMode, ByVal addrMd As ParamMode, _
                ByVal dest As Boolean, ByVal hlp As StringW)
    If g_regCount >= MAX_REG_ENTRIES Then Exit Sub
    With g_registry(g_regCount)
        .category = cat
        .action = act
        .aliases = ali
        .needsDriver = drv
        .pidMode = pidMd
        .tidMode = tidMd
        .pathMode = pathMd
        .addrMode = addrMd
        .isDestructive = dest
        .helpText = hlp
    End With
    g_regCount += 1
End Sub

Function FindCmd(ByVal cat As StringW, ByVal act As StringW) As CmdEntry Ptr
    Dim lcCat As StringW = LCaseW(cat)
    Dim lcAct As StringW = LCaseW(act)
    ' 直接匹配
    For i As Integer = 0 To g_regCount - 1
        If LCaseW(g_registry(i).category) = lcCat AndAlso LCaseW(g_registry(i).action) = lcAct Then
            Return @g_registry(i)
        End If
    Next
    ' 别名匹配（中文输入）
    For i As Integer = 0 To g_regCount - 1
        If InStrW(LCaseW(g_registry(i).aliases), lcCat) > 0 AndAlso LCaseW(g_registry(i).action) = lcAct Then
            Return @g_registry(i)
        End If
    Next
    Return NULL
End Function

' --------------- 十六进制/数值辅助 ---------------
Function ParseUInt64(ByVal s As StringW) As ULONG64
    Dim ls As StringW = LCaseW(TrimW(s))
    If LeftW(ls, 2) = "0x" Then
        Return ValULng("&H" & MidW(ls, 3))
    ElseIf LeftW(ls, 1) = "&" Then
        Return ValULng(ls)
    Else
        Return ValULng(ls)
    End If
End Function

' DWORD 数值解析：支持 0x / &H / 十进制
Function GetDWORDValue(ByVal s As StringW) As DWORD
    Dim ls As StringW = LCaseW(TrimW(s))
    If LeftW(ls, 2) = "0x" Then
        Return Cast(DWORD, ValULng("&H" & MidW(ls, 3)))
    ElseIf LeftW(ls, 1) = "&" Then
        Return Cast(DWORD, ValULng(ls))
    Else
        Return Cast(DWORD, ValULng(ls))
    End If
End Function

' --------------- 初始化注册表 ---------------
Sub InitCommandRegistry()
    ' ===== 一级查询（无需 PID）=====
    RegisterCmd "process",       "list", "进程,程序",       False, PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List all running processes"
    RegisterCmd "kernel-module", "list", "内核模块",         True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List loaded kernel modules"
    RegisterCmd "kernel-thread", "list", "内核线程",         True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List kernel threads"
    RegisterCmd "gdt",           "read", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "Read GDT entries"
    RegisterCmd "idt",           "read", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "Read IDT entries"
    RegisterCmd "ssdt",          "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List SSDT hooks"
    RegisterCmd "sssdt",          "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List ShadowSSDT hooks"
    RegisterCmd "callbacks",     "list", "回调",             True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List kernel callbacks"
    RegisterCmd "service", "list", "服务", False, PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List system services"
    RegisterCmd "etw",  "list", "ETW会话",        False, PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List ETW sessions"
    RegisterCmd "minifilter",    "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List minifilter drivers"
    RegisterCmd "filterdriver",  "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List filter drivers"
    RegisterCmd "hal-dispatch",  "read", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "Read HAL dispatch table"
    RegisterCmd "hal-private-dispatch", "read", "",          True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "Read HAL private dispatch"
    RegisterCmd "object-hook",   "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List object type hooks"
    RegisterCmd "kernel-timer",  "list", "内核定时器",       True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List kernel timers"
    RegisterCmd "work-item-thread","list","工作队列线程",    True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List work item threads"
    RegisterCmd "wfp-callout",   "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List WFP callouts"
    RegisterCmd "wfp-filter",    "list", "",                 True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List WFP filters"
    RegisterCmd "winsock-spi",   "list", "",                 False, PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List Winsock SPI entries"
    RegisterCmd "task-scheduler","list", "任务计划",         False, PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List scheduled tasks"
    RegisterCmd "unlock-file", "list", "显示文件占用情况",         True,  PM_NONE, PM_NONE, PM_NONE, PM_NONE, False, "List all handle of file"
    
    ' ===== 文件/注册表查询（可选 --path）=====
    RegisterCmd "file",     "list", "文件",   False, PM_NONE, PM_NONE, PM_OPTIONAL, PM_NONE, False, "List files (empty path = current drive root)"
    RegisterCmd "registry", "list", "注册表", False, PM_NONE, PM_NONE, PM_OPTIONAL, PM_NONE, False, "List registry (empty path = roots)"
    
    RegisterCmd "kernel-memory", "read", "内核内存", True, PM_NONE, PM_NONE, PM_NONE, PM_REQUIRED, False, "Read kernel memory (physical/virtual address)"
    With g_registry(g_regCount - 1)
        .hasExtHelp = True
        With .extHelp
            .header = "Kernel Memory Read Specifics:"
            .lines(0) = "--addr <Address>  Kernel virtual address (supports 0x prefix)"
            .lines(1) = "--size <Bytes>    Number of bytes to read (default: 256, max: 16MB)"
            .lines(2) = "Output format: hex dump with ASCII sidebar"
            .lineCount = 3
            .example1 = "kernel-memory read --addr 0xFFFFF80300000000 --size 64"
            .example2 = "kernel-memory read --addr 0x140000000"
        End With
    End With

    RegisterCmd "kernel-memory", "write", "内核内存", True, PM_NONE, PM_NONE, PM_NONE, PM_REQUIRED, True, "Write hex data to kernel memory"
    With g_registry(g_regCount - 1)
        .hasExtHelp = True
        With .extHelp
            .header = "Kernel Memory Write Specifics:"
            .lines(0) = "--addr <Address>  Kernel virtual address (supports 0x prefix)"
            .lines(1) = "--data <HexStr>   Hex data to write (REQUIRED unless using --size for zero-fill)"
            .lines(2) = "--size <Bytes>    Number of zero bytes to write (only if --data omitted)"
            .lines(3) = ""
            .lines(4) = "Data format rules:"
            .lines(5) = "- Compact: --data 48656C6C6F6F20776F726C64"
            .lines(6) = "- Spaced:  --data ""48 65 6C 6C 6F 20 77 6F 72 6C 64"""
            .lines(7) = "- Length must be even (2 hex chars = 1 byte)"
            .lines(8) = "- Max 16MB of data"
            .lineCount = 9
            .example1 = "kernel-memory write --addr 0xFFFFF80300000000 --data 9090"
            .example2 = "kernel-memory write --addr 0x140000000 --size 16"
        End With
    End With
    
    ' ===== 二级查询（需要 --pid）=====
    RegisterCmd "thread",       "list", "线程",     False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List threads of a process"
    RegisterCmd "module",       "list", "模块",     False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List modules of a process"
    RegisterCmd "windows",      "list", "窗口",     False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List windows of a process"
    RegisterCmd "handle", "list", "句柄", True, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List handles of a process (list all if --pid omitted)"
    RegisterCmd "memory", "list", "内存", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List process memory block"
    With g_registry(g_regCount - 1)
        .hasExtHelp = True
        With .extHelp
            .header = "Memory List Specifics:"
            .lines(0) = "Lists all memory blocks (regions) of the target process"
            .lines(1) = "Output includes: BaseAddress, Size, Protect, State, Type, AllocationBase, ModuleName(If it has)"
            .lineCount = 2
            .example1 = "memory list --pid 1234"
            .example2 = ""
        End With
    End With
    RegisterCmd "memory", "read", "内存", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_REQUIRED, False, "Read process memory (hex dump output)"
    ' 找到刚注册的条目，填充扩展帮助
    With g_registry(g_regCount - 1)
        .hasExtHelp = True
        With .extHelp
            .header = "Memory Read Specifics:"
            .lines(0) = "--size <Bytes>  Number of bytes to read (default: 256, max: 16MB)"
            .lines(1) = "Output format: hex dump with ASCII sidebar"
            .lineCount = 2
            .example1 = "memory read --pid 1234 --addr 0x7FFE0000 --size 64"
            .example2 = "memory read --pid 5678 --addr 0x1000"
        End With
    End With
    RegisterCmd "memory", "write", "内存", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_REQUIRED, True, "Write hex data to process memory. Data format: --data 48656C6C6F (no spaces) or --data ""48 65 6C 6C 6F"" (with spaces)"
    With g_registry(g_regCount - 1)
        .hasExtHelp = True
        With .extHelp
            .header = "Memory Write Specifics:"
            .lines(0) = "--data <HexStr>  Hex data to write (REQUIRED unless using --size for zero-fill)"
            .lines(1) = "--size <Bytes>   Number of zero bytes to write (only if --data omitted)"
            .lines(2) = ""
            .lines(3) = "Data format rules:"
            .lines(4) = "- Compact: --data 48656C6C6F6F20776F726C64"
            .lines(5) = "- Spaced:  --data ""48 65 6C 6C 6F 20 77 6F 72 6C 64"""
            .lines(6) = "- Length must be even (2 hex chars = 1 byte)"
            .lines(7) = "- Max 16MB of data"
            .lineCount = 8
            .example1 = "memory write --pid 1234 --addr 0x1000 --data 9090"
            .example2 = "memory write --pid 1234 --addr 0x2000 --size 16"
        End With
    End With
    RegisterCmd "processtimer", "list", "进程定时器", True, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List timers of a process (list all if --pid omitted)"
    RegisterCmd "webconnect", "list", "网络连接", False, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List network connections (list all if --pid omitted)"
    RegisterCmd "privilege",    "list", "特权",     False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List process privileges"
    RegisterCmd "hook",         "list", "钩子",     False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, False, "List process hooks"
    RegisterCmd "msghook", "list", "消息钩子", False, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List message hooks (list all if --pid omitted)(if failed, try again)"
    RegisterCmd "eventhook", "list", "事件钩子", False, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List event hooks (list all if --pid omitted)(if failed, try again)"
    RegisterCmd "hotkey", "list", "热键", False, PM_OPTIONAL, PM_NONE, PM_NONE, PM_NONE, False, _
            "List registered hotkeys (list all if --pid omitted)(if failed, try again)"
    
    ' ===== 三级查询（需要 --pid + --tid）=====
    RegisterCmd "thread", "callstack", "调用栈", False, PM_REQUIRED, PM_REQUIRED, PM_NONE, PM_NONE, False, "Show thread call stack"
    
    ' ===== 编辑/破坏性命令（安全机制） =====
    RegisterCmd "process",  "suspend", "", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, True, "Suspend a process"
    RegisterCmd "process",  "resume",  "", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, True, "Resume a suspended process"
    RegisterCmd "process",  "kill",    "", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, True, "Terminate a process"
    RegisterCmd "process",  "force-kill",    "", False, PM_REQUIRED, PM_NONE, PM_NONE, PM_NONE, True, "Force Terminate a process"
    RegisterCmd "thread",   "suspend", "", False, PM_REQUIRED, PM_REQUIRED, PM_NONE, PM_NONE, True, "Suspend a thread"
    RegisterCmd "thread",   "resume",  "", False, PM_REQUIRED, PM_REQUIRED, PM_NONE, PM_NONE, True, "Resume a thread"
    RegisterCmd "thread",   "kill",    "", False, PM_REQUIRED, PM_REQUIRED, PM_NONE, PM_NONE, True, "Terminate a thread"
    RegisterCmd "registry", "write",   "", False, PM_NONE, PM_NONE, PM_REQUIRED, PM_NONE, True, "Write registry value"
    RegisterCmd "registry", "delete",  "", False, PM_NONE, PM_NONE, PM_REQUIRED, PM_NONE, True, "Delete registry value"
    RegisterCmd "file", "delete", "", False, PM_NONE, PM_NONE, PM_REQUIRED, PM_NONE, True, "Delete a file"
    RegisterCmd "unlock-file", "unlock", "", True, PM_NONE, PM_NONE, PM_NONE, PM_NONE, True, "unlock handle of file"
End Sub

Function ParseCommandLine(parts() As StringW, ByVal numParts As Long, ByVal outResult As ParsedCmd Ptr) As Boolean

    ' 初始化
    outResult->pid = 0
    outResult->tid = 0
    outResult->addr = 0
    outResult->size = 0

    outResult->force = False
    outResult->dryRun = False
    outResult->showHelp = False

    outResult->hasError = False

    outResult->path = ""
    outResult->hexData = ""
    outResult->regName = ""
    outResult->regType = "REG_SZ"

    outResult->hasPid = False
    outResult->hasTid = False
    outResult->hasPath = False
    outResult->hasAddr = False

    If numParts < 2 Then
        outResult->hasError = True
        outResult->errorMsg = _
            "Usage: <category> <action> [options]. Example: 'process list' or 'thread list --pid 1234'"
        Return False
    End If

    outResult->category = parts(0)
    outResult->action = parts(1)

    Dim i As Long = 2

    Do While i < numParts

        Dim arg As StringW = LCaseW(parts(i))

        ' ========================================
        ' 必须是 --xxx / -x 参数
        ' ========================================
        If LeftW(arg, 2) <> "--" AndAlso _
           arg <> "-f" AndAlso _
           arg <> "-d" AndAlso _
           arg <> "-h" AndAlso _
           arg <> "/?" Then

            outResult->hasError = True
            outResult->unknownArg = parts(i)
            outResult->errorMsg = _
                "ERROR: Unknown argument '" & parts(i) & _
                "'. Use '<category> <action> --help' for usage."

            Return False
        End If


        ' ========================================
        ' 无值参数
        ' ========================================
        If arg = "--help" OrElse _
           arg = "-h" OrElse _
           arg = "/?" Then

            outResult->showHelp = True
            i += 1
            Continue Do
        End If


        If arg = "--force" OrElse arg = "-f" Then

            outResult->force = True
            i += 1
            Continue Do
        End If


        If arg = "--dry-run" OrElse arg = "-d" Then

            outResult->dryRun = True
            i += 1
            Continue Do
        End If


        ' ========================================
        ' 其余参数都需要 value
        ' ========================================
        If i + 1 >= numParts Then

            outResult->hasError = True
            outResult->errorMsg = _
                "ERROR: Missing value for '" & parts(i) & "'."

            Return False
        End If


        Dim value As StringW = parts(i + 1)


        ' ========================================
        ' --pid
        ' ========================================
        If arg = "--pid" Then

            outResult->pid = GetDWORDValue(value)
            outResult->hasPid = True

        ' ========================================
        ' --tid
        ' ========================================
        ElseIf arg = "--tid" Then

            outResult->tid = GetDWORDValue(value)
            outResult->hasTid = True

        ' ========================================
        ' --path
        ' ========================================
        ElseIf arg = "--path" Then

            outResult->path = value
            outResult->hasPath = True

        ' ========================================
        ' --addr
        ' ========================================
        ElseIf arg = "--addr" Then

            outResult->addr = ParseUInt64(value)
            outResult->hasAddr = True

        ' ========================================
        ' --size
        ' ========================================
        ElseIf arg = "--size" Then

            outResult->size = GetDWORDValue(value)

        ' ========================================
        ' --data
        ' ========================================
        ElseIf arg = "--data" Then

            outResult->hexData = value

        ' ========================================
        ' --name
        ' ========================================
        ElseIf arg = "--name" Then

            outResult->regName = value

        ' ========================================
        ' --type
        ' ========================================
        ElseIf arg = "--type" Then

            outResult->regType = value

        Else

            outResult->hasError = True
            outResult->unknownArg = parts(i)
            outResult->errorMsg = _
                "ERROR: Unknown argument '" & parts(i) & "'. Use '<category> <action> --help' for usage."

            Return False
        End If


        ' 参数 + value
        i += 2

    Loop

    Return True
End Function

Sub PrintGlobalHelp()
    SafeStdOut("SnowSword Agent Shell v2.0 (Subcommand Style)" & vbCrlfW)
    SafeStdOut("Usage: <category> <action> [options]" & vbCrlfW)
    SafeStdOut("       <category> <action> --help    for action-specific help" & vbCrlfW)
    SafeStdOut("       cmd [| findstr/sort/head [args]]... [> file] [>> file]   pipeline syntax" & vbCrlfW & vbCrlfW)
    
    SafeStdOut("[System]  (no --pid needed)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).pidMode = PM_NONE AndAlso g_registry(i).pathMode = PM_NONE Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Process] (require --pid)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).pidMode = PM_REQUIRED AndAlso g_registry(i).tidMode = PM_NONE Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Process] (optional --pid)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).pidMode = PM_OPTIONAL Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & "  (list all if --pid omitted)" & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Files/Registry] (optional --path)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).pathMode = PM_OPTIONAL Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & "  (empty path = roots/drives)" & vbCrlfW)
        End If
    Next

    SafeStdOut(vbCrlfW & "[Thread]  (require --pid --tid)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).tidMode = PM_REQUIRED Then
            SafeStdOut("  " & g_registry(i).category & " " & g_registry(i).action & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Edit/Destructive]  (require --force for kernel ops)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).isDestructive Then
            SafeStdOut("  " & g_registry(i).category & " " & g_registry(i).action & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "Pipeline & Redirection:" & vbCrlfW)
    SafeStdOut("  cmd | findstr [opts] <pattern>   keep matching data rows (/v invert, case-insensitive)" & vbCrlfW)
    SafeStdOut("  cmd | sort [/r]                  sort data rows (ascending, /r descending)" & vbCrlfW)
    SafeStdOut("  cmd | head [N]                   keep first N data rows (default 10)" & vbCrlfW)
    SafeStdOut("  cmd > file                       write output to file (overwrite, UTF-8)" & vbCrlfW)
    SafeStdOut("  cmd >> file                      append output to file (UTF-8)" & vbCrlfW)
    SafeStdOut("  Examples: process list | findstr svchost | sort | head 20" & vbCrlfW)
    SafeStdOut("            process list | findstr /v svchost > out.txt" & vbCrlfW & vbCrlfW)
    
    SafeStdOut("Global Options:" & vbCrlfW)
    SafeStdOut("  --force, -f     Skip confirmation for destructive operations" & vbCrlfW)
    SafeStdOut("  --dry-run, -d   Preview operation without executing" & vbCrlfW)
    SafeStdOut("  --help, -h      Show help (can be used inline)" & vbCrlfW)
End Sub

Sub PrintCategoryHelp(cat As StringW)
    Dim lcCat As StringW = LCaseW(cat)
    SafeStdOut("Category: " & cat & vbCrlfW)
    SafeStdOut("Available actions:" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If LCaseW(g_registry(i).category) = lcCat OrElse InStrW(LCaseW(g_registry(i).aliases), lcCat) > 0 Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).action
            If g_registry(i).aliases <> "" Then
                strLine = strLine & "    | " & g_registry(i).aliases
            End If
            If g_registry(i).isDestructive Then
                strLine = strLine & "  [DESTRUCTIVE]"
            End If
            strLine = strLine & "  - " & g_registry(i).helpText
            SafeStdOut(strLine & vbCrlfW)
        End If
    Next
End Sub

Sub PrintActionHelp(ByVal def As CmdEntry Ptr)
    SafeStdOut("Usage: " & def->category & " " & def->action & " [options]" & vbCrlfW)
    If def->aliases <> "" Then
        SafeStdOut("Aliases: " & def->aliases & vbCrlfW)
    End If
    SafeStdOut("Description: " & def->helpText & vbCrlfW & vbCrlfW)
    
    SafeStdOut("Parameters:" & vbCrlfW)
    Select Case def->pidMode
    Case PM_REQUIRED: SafeStdOut("  --pid <ProcessID>  (required)" & vbCrlfW)
    Case PM_OPTIONAL: SafeStdOut("  --pid <ProcessID>  (optional, omit to list all)" & vbCrlfW)
    End Select
    Select Case def->tidMode
    Case PM_REQUIRED: SafeStdOut("  --tid <ThreadID>   (required)" & vbCrlfW)
    Case PM_OPTIONAL: SafeStdOut("  --tid <ThreadID>   (optional)" & vbCrlfW)
    End Select
    Select Case def->pathMode
    Case PM_REQUIRED: SafeStdOut("  --path <Path>  (required)" & vbCrlfW)
    Case PM_OPTIONAL: SafeStdOut("  --path <Path>  (optional, omit to list all)" & vbCrlfW)
    End Select
    Select Case def->addrMode
    Case PM_REQUIRED: SafeStdOut("  --addr <Address>  (supports 0x prefix)   (required)" & vbCrlfW)
    Case PM_OPTIONAL: SafeStdOut("  --addr <Address>  (supports 0x prefix)   (optional)" & vbCrlfW)
    End Select
    
    SafeStdOut(vbCrlfW & "Optional:" & vbCrlfW)
    SafeStdOut("  --force         Skip confirmation" & vbCrlfW)
    SafeStdOut("  --dry-run       Preview only" & vbCrlfW)
    SafeStdOut("  --help          Show this help" & vbCrlfW)
    ' 【方案B核心】渲染扩展帮助（纯数据驱动，无硬编码命令名）
    If def->hasExtHelp Then
        SafeStdOut(vbCrlfW & def->extHelp.header & vbCrlfW)
        For i As Integer = 0 To def->extHelp.lineCount - 1
            If def->extHelp.lines(i) <> "" Then
                SafeStdOut("  " & def->extHelp.lines(i) & vbCrlfW)
            Else
                SafeStdOut(vbCrlfW)  ' 空行
            End If
        Next
        SafeStdOut(vbCrlfW & "Examples:" & vbCrlfW)
        SafeStdOut("  " & def->extHelp.example1 & vbCrlfW)
        If def->extHelp.example2 <> "" Then
            SafeStdOut("  " & def->extHelp.example2 & vbCrlfW)
        End If
    End If
End Sub

Function ConfirmDestructive(ByVal pc As ParsedCmd Ptr, ByVal def As CmdEntry Ptr) As Boolean
    If pc->dryRun Then
        If g_OutputJson Then
            ' JSON 格式输出（使用预览缓冲避免尾逗号）
            Dim previewJson As StringW = ""
            Dim needComma As Boolean = False
            If pc->pid <> 0 Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""pid"": " & pc->pid
                needComma = True
            End If
            If pc->tid <> 0 Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""tid"": " & pc->tid
                needComma = True
            End If
            If pc->path <> "" Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""path"": """ & CuiEscapeJson(pc->path) & """"
                needComma = True
            End If
            If pc->addr <> 0 Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""address"": ""0x" & Hex(pc->addr) & """"
                needComma = True
            End If
            If pc->size <> 0 Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""size"": " & pc->size
                needComma = True
            End If
            If LenW(pc->hexData) > 0 Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""data"": """ & CuiEscapeJson(pc->hexData) & """"
                needComma = True
            End If
            If pc->regName <> "" Then
                If needComma Then previewJson &= "," & vbCrlfW
                previewJson &= "    ""regName"": """ & CuiEscapeJson(pc->regName) & """"
                needComma = True
            End If

            SafeStdOut("{" & vbCrlfW)
            SafeStdOut("  ""dryRun"": true," & vbCrlfW)
            SafeStdOut("  ""category"": """ & CuiEscapeJson(pc->category) & """," & vbCrlfW)
            SafeStdOut("  ""action"": """ & CuiEscapeJson(pc->action) & """," & vbCrlfW)
            SafeStdOut("  ""preview"": {" & vbCrlfW)
            If LenW(previewJson) > 0 Then SafeStdOut(previewJson & vbCrlfW)
            SafeStdOut("  }" & vbCrlfW)
            SafeStdOut("}" & vbCrlfW)
        Else
            ' 原有文本输出
            SafeStdOut("[DRY-RUN] Operation preview:" & vbCrlfW)
            SafeStdOut("  Category : " & pc->category & vbCrlfW)
            SafeStdOut("  Action   : " & pc->action & vbCrlfW)
            If pc->pid <> 0    Then SafeStdOut("  PID      : " & pc->pid & vbCrlfW)
            If pc->tid <> 0    Then SafeStdOut("  TID      : " & pc->tid & vbCrlfW)
            If pc->path <> ""  Then SafeStdOut("  Path     : " & pc->path & vbCrlfW)
            If pc->addr <> 0   Then SafeStdOut("  Address  : 0x" & Hex(pc->addr) & vbCrlfW)
            If pc->size <> 0   Then SafeStdOut("  Size     : " & pc->size & vbCrlfW)
            If LenW(pc->hexData) > 0 Then SafeStdOut("  Data(hex): " & pc->hexData & vbCrlfW)
            If pc->regName <> "" Then SafeStdOut("  RegName  : " & pc->regName & vbCrlfW)
            SafeStdOut("No changes made (dry-run)." & vbCrlfW)
        End If
        Return True   ' ← 关键修改：返回 True 表示命令成功（但不执行实际操作）
    End If
    
    If def->isDestructive = False Then Return True
    
    ' 内核级破坏性操作：强制 --force，禁止交互（管道/控制台一致）
    If def->needsDriver AndAlso pc->force = False Then
        SafeStdOut("ERROR: Kernel destructive operations require --force. Interactive confirm is disabled for safety." & vbCrlfW)
        Return False
    End If
    
    If pc->force Then Return True  ' 用户已显式承担风险
    
    ' ===== 用户层交互确认 =====
    Dim msg As WString * 512
    msg = "WARNING: Destructive operation '" & pc->category & " " & pc->action & "'."
    If pc->pid <> 0 Then msg = msg & " PID=" & pc->pid
    If pc->tid <> 0 Then msg = msg & " TID=" & pc->tid
    msg = msg & vbCrlfW & "Confirm? [y/N]: "
    SafeStdOutDirect(msg)
    
    ' 【新增】确保提示符已完全送达客户端，再进入读取
    /'If g_InAgentMode AndAlso g_hAgentPipe <> INVALID_HANDLE_VALUE Then
        FlushFileBuffers(g_hAgentPipe)   ' ← 关键：强制刷新写缓冲区
    End If'/
    
    ' ---------- 管道模式：从命名管道读取 ----------
    If g_InAgentMode AndAlso g_hAgentPipe <> INVALID_HANDLE_VALUE Then
        Dim confirm As WString * 8 = ""
        If Not PipeReadLine(@confirm, 8) Then
            SafeStdOut("Cancelled (pipe disconnected)." & vbCrlfW)
            Return False
        End If
        
        If LCaseW(LeftW(confirm, 1)) = "y" Then Return True
        SafeStdOut("Cancelled." & vbCrlfW)
        Return False
    End If
    
    ' ---------- 控制台模式：从 STD_INPUT 读取 ----------
    Dim hIn As HANDLE = GetStdHandle(STD_INPUT_HANDLE)
    Dim confirmBuf As WString * 8
    Dim br As DWORD
    If ReadConsoleW(hIn, @confirmBuf, 7, @br, NULL) = 0 Then Return False
    confirmBuf[br] = 0
    If LCaseW(LeftW(confirmBuf, 1)) = "y" Then Return True
    
    SafeStdOut("Cancelled." & vbCrlfW)
    Return False
End Function

' --------------- 查询类执行函数示例 ---------------
Function Exec_Process_List(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = Process
    InitializeListView Process, FrmMain.ListView1
    If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW)
    GetProcessList FrmMain.ListView1, False
    CuiPrintListViewData()
    Return True
End Function

Function Exec_KernelModule_List(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = KernelModule
    InitializeListView KernelModule, FrmMain.ListView1
    If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW)
    GetKernelModuleList FrmMain.ListView1
    CuiPrintListViewData()
    Return True
End Function

Function Exec_Thread_List(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = Thread
    CurrentInformation.ProcessId = pc->pid
    InitializeListView Thread, FrmMain.ListView1
    GetThreadList pc->pid, FrmMain.ListView1
    CuiPrintListViewData()
    Return True
End Function

Function Exec_Memory_Read(ByVal pc As ParsedCmd Ptr) As Boolean
    Dim hProcess As HANDLE = NULL
    Dim hTarget As HANDLE = NULL
    Dim buf() As Byte
    Dim bytesRead As SIZE_T = 0
    Dim i As Integer
    Dim hexLine As WString * 80
    Dim hexPart As WString * 50
    Dim asciiPart As WString * 20
    Dim b As Byte
    
    ' ── 参数校验 ─────────────────────────────────────────
    If pc->hasPid = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --pid required for memory read"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    If pc->hasAddr = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --addr required for memory read"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    If pc->size = 0 Then
        pc->hasError = True
        pc->errorMsg = "Error: --size must be > 0"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    If pc->size > &H1000000 Then  ' 16MB 上限，防止恶意请求
        pc->hasError = True
        pc->errorMsg = "Error: --size exceeds 16MB limit"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 打开目标进程 ──────────────────────────────────────
    If IsDriverLoaded Then
        hTarget = SnowSword_OpenProcess(pc->pid)
        If hTarget = NULL Then
            pc->hasError = True
            pc->errorMsg = "Error: SnowSword_OpenProcess failed for PID=" & pc->pid
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        hProcess = hTarget
    Else
        hProcess = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, False, pc->pid)
        If hProcess = NULL Then
            pc->hasError = True
            pc->errorMsg = "Error: OpenProcess failed for PID=" & pc->pid & " (err=" & GetLastError() & "),try to load driver"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
    End If
    
    ' ── 分配缓冲区 ─────────────────────────────────────────
    ReDim buf(pc->size - 1)
    
    ' ── 执行读取 ───────────────────────────────────────────
    If ReadProcessMemory(hProcess, Cast(LPCVOID, pc->addr), @buf(0), pc->size, @bytesRead) = 0 Then
        pc->hasError = True
        pc->errorMsg = "Error: ReadProcessMemory failed at 0x" & Hex(pc->addr) & " (err=" & GetLastError() & "),try to load driver"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        If IsDriverLoaded = False Then CloseHandle(hProcess)
        Return False
    End If
    
    ' ── 输出结果 ───────────────────────────────────────────
    SafeStdOut("Read " & bytesRead & " bytes from PID=" & pc->pid & " at 0x" & Hex(pc->addr) & vbCrLfW)
    
    ' 十六进制 + ASCII 双栏输出
    For i = 0 To bytesRead - 1
        If (i Mod 16) = 0 Then
            If i > 0 Then
                SafeStdOut("  " & asciiPart & vbCrLfW)
            End If
            hexLine = Hex(pc->addr + i, 8) & "  "
            hexPart = ""
            asciiPart = ""
        End If
        
        b = buf(i)
        hexPart = hexPart & Hex(b, 2) & " "
        
        If b >= 32 And b <= 126 Then
            asciiPart = asciiPart & WChr(b)
        Else
            asciiPart = asciiPart & "."
        End If
    Next i
    
    ' 补齐最后一行
    If bytesRead > 0 Then
        Dim padCount As Integer = (16 - (bytesRead Mod 16)) Mod 16
        For i = 1 To padCount
            hexPart = hexPart & "   "
        Next i
        SafeStdOut(hexLine & hexPart & "  " & asciiPart & vbCrLfW)
    End If
    
    ' ── 清理 ───────────────────────────────────────────────
    If IsDriverLoaded = False Then
        CloseHandle(hProcess)
    End If
    
    Return True
End Function

Function Exec_Memory_Write(ByVal pc As ParsedCmd Ptr) As Boolean
    Dim hProcess As HANDLE = NULL
    Dim hTarget As HANDLE = NULL
    Dim bytesWritten As SIZE_T = 0
    Dim oldProtect As DWORD = 0
    Dim writeBuf() As Byte
    Dim i As Integer
    Dim dataLen As Integer
    
    ' ── 参数校验 ─────────────────────────────────────────
    If pc->hasPid = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --pid required for memory write"
        SafeStdOut(pc->errorMsg & vbCrlfW)
        Return False
    End If
    
    If pc->hasAddr = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --addr required for memory write"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 新增：如果提供了 hexData，解析为字节数组 ──
    If LenW(pc->hexData) > 0 Then
        Dim hexClean As StringW = MyTrimW(pc->hexData, " ")
        Dim hexLen As Integer = LenW(hexClean)
        
        ' 支持 "48 65 6C" 和 "48656C" 两种格式
        If InStrW(hexClean, " ") > 0 Then
            ' 空格分隔：先去掉所有空格
            Dim tmp As StringW = ""
            For i As Integer = 0 To hexLen - 1
                If hexClean[i] <> 32 Then tmp = tmp & WChr(hexClean[i])
            Next
            hexClean = tmp
            hexLen = LenW(hexClean)
        End If
        
        ' 长度必须是偶数
        If (hexLen Mod 2) <> 0 OrElse hexLen = 0 Then
            pc->hasError = True
            pc->errorMsg = "Error: --data must be valid hex string (even length)"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        
        Dim byteCount As Integer = hexLen \ 2
        If byteCount > &H1000000 Then
            pc->hasError = True
            pc->errorMsg = "Error: --data exceeds 16MB"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        
        pc->bytdata = Allocate(byteCount)
        If pc->bytdata = NULL Then
            pc->hasError = True
            pc->errorMsg = "Error: memory allocation failed"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        
        For i As Integer = 0 To byteCount - 1
            Dim hexByte As StringW = MidW(hexClean, i * 2 + 1, 2)
            pc->bytdata[i] = CByte(ValULng("&H" & hexByte))
        Next
        
        pc->size = byteCount   ' ← 从数据长度自动推导
    End If
    
    If pc->size = 0 And pc->bytdata = NULL Then
        pc->hasError = True
        pc->errorMsg = "Error: --size or --data required for memory write"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 准备写入数据 ───────────────────────────────────────
    ' 优先使用 bytdata（显式字节数据），否则用 size 写零
    If pc->bytdata <> NULL Then
        ' 计算 bytdata 长度（假设以 null 结尾或外部已知长度）
        ' 这里简化处理：如果 size > 0 且 bytdata 有效，使用 size 作为长度
        dataLen = pc->size
        If dataLen = 0 Then
            pc->hasError = True
            pc->errorMsg = "Error: bytdata provided but size=0"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        ReDim writeBuf(dataLen - 1)
        CopyMemory(@writeBuf(0), pc->bytdata, dataLen)
    Else
        ' 无显式数据，按 size 写零
        dataLen = pc->size
        If dataLen > &H1000000 Then
            pc->hasError = True
            pc->errorMsg = "Error: --size exceeds 16MB limit"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        ReDim writeBuf(dataLen - 1)
        ' 数组已自动初始化为 0
    End If
    
    ' ── Dry Run 模式 ───────────────────────────────────────
    If pc->dryRun Then
        SafeStdOut("[DRY RUN] Would write " & dataLen & " bytes to PID=" & pc->pid & " at 0x" & Hex(pc->addr) & vbCrLfW)
        Return True
    End If
    
    ' ── 打开目标进程 ──────────────────────────────────────
    If IsDriverLoaded Then
        hTarget = SnowSword_OpenProcess(pc->pid)
        If hTarget = NULL Then
            pc->hasError = True
            pc->errorMsg = "Error: SnowSword_OpenProcess failed for PID=" & pc->pid
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        hProcess = hTarget
    Else
        ' 写内存需要 VM_WRITE + VM_OPERATION（用于 VirtualProtectEx）
        hProcess = OpenProcess(PROCESS_VM_WRITE Or PROCESS_VM_OPERATION Or PROCESS_QUERY_INFORMATION, False, pc->pid)
        If hProcess = NULL Then
            pc->hasError = True
            pc->errorMsg = "Error: OpenProcess failed for PID=" & pc->pid & " (err=" & GetLastError() & "),try to load driver"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
    End If
    
    ' ── 修改内存保护属性（非驱动模式下可能需要）────────────
    If IsDriverLoaded = False Then
        If VirtualProtectEx(hProcess, Cast(LPVOID, pc->addr), dataLen, PAGE_EXECUTE_READWRITE, @oldProtect) = 0 Then
            ' 尝试只读/读写
            If VirtualProtectEx(hProcess, Cast(LPVOID, pc->addr), dataLen, PAGE_READWRITE, @oldProtect) = 0 Then
                SafeStdOut("Warning: VirtualProtectEx failed (err=" & GetLastError() & "), attempting write anyway..." & vbCrLfW)
                ' 某些情况下目标页已经是可写的，继续尝试
            End If
        End If
    End If
    
    ' ── 执行写入 ───────────────────────────────────────────
    If WriteProcessMemory(hProcess, Cast(LPVOID, pc->addr), @writeBuf(0), dataLen, @bytesWritten) = 0 Then
        pc->hasError = True
        pc->errorMsg = "Error: WriteProcessMemory failed at 0x" & Hex(pc->addr) & " (err=" & GetLastError() & "),try to load driver"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        
        ' 恢复保护属性
        If IsDriverLoaded = False And oldProtect <> 0 Then
            VirtualProtectEx(hProcess, Cast(LPVOID, pc->addr), dataLen, oldProtect, @oldProtect)
        End If
        If IsDriverLoaded = False Then CloseHandle(hProcess)
        Return False
    End If
    
    ' ── 恢复内存保护属性 ───────────────────────────────────
    If IsDriverLoaded = False And oldProtect <> 0 Then
        Dim tmp As DWORD
        VirtualProtectEx(hProcess, Cast(LPVOID, pc->addr), dataLen, oldProtect, @tmp)
    End If
    
    ' ── 输出结果 ───────────────────────────────────────────
    SafeStdOut("Wrote " & bytesWritten & "/" & dataLen & " bytes to PID=" & pc->pid & " at 0x" & Hex(pc->addr) & vbCrLfW)
    
    ' 打印写入内容的摘要（前 32 字节）
    If dataLen <= 32 Then
        Dim hexStr As WString * 100 = ""
        For i = 0 To dataLen - 1
            hexStr = hexStr & Hex(writeBuf(i), 2) & " "
        Next i
        SafeStdOut("Data: " & hexStr & vbCrLfW)
    Else
        Dim hexStr As WString * 100 = ""
        For i = 0 To 15
            hexStr = hexStr & Hex(writeBuf(i), 2) & " "
        Next i
        SafeStdOut("Data: " & hexStr & "... (" & dataLen & " bytes total)" & vbCrLfW)
    End If
    
    ' ── 清理 ───────────────────────────────────────────────
    If IsDriverLoaded = False Then
        CloseHandle(hProcess)
    End If
    
    Return True
End Function

' ===================== 内核内存读写（无需 PID，直接操作物理/内核地址）=====================

Function Exec_KernelMemory_Read(ByVal pc As ParsedCmd Ptr) As Boolean
    Dim buf() As Byte
    Dim bytesRead As SIZE_T = 0
    Dim i As Integer
    Dim hexLine As WString * 80
    Dim hexPart As WString * 50
    Dim asciiPart As WString * 20
    Dim b As Byte
    
    ' ── 参数校验 ─────────────────────────────────────────
    If pc->hasAddr = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --addr required for kernel memory read"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    If pc->size = 0 Then
        pc->size = 256   ' 默认读 256 字节
        SafeStdOut("Notice: --size not specified, defaulting to 256 bytes." & vbCrLfW)
    End If
    
    If pc->size > &H1000000 Then  ' 16MB 上限
        pc->hasError = True
        pc->errorMsg = "Error: --size exceeds 16MB limit"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 驱动检查 ─────────────────────────────────────────
    If Not IsDriverLoaded Then
        pc->hasError = True
        pc->errorMsg = "Error: Driver not loaded. Use 'load-driver' first."
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 分配缓冲区 ─────────────────────────────────────────
    ReDim buf(pc->size - 1)
    
    ' ── 执行内核读取 ───────────────────────────────────────
    If Not ReadKernelMemory(Cast(UINTEGER, pc->addr), pc->size, @buf(0)) Then
        pc->hasError = True
        pc->errorMsg = "Error: ReadKernelMemory failed at 0x" & Hex(pc->addr)
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    bytesRead = pc->size  ' 内核读取成功即完整读取
    
    ' ── 输出结果 ───────────────────────────────────────────
    SafeStdOut("Read " & bytesRead & " bytes from kernel address 0x" & Hex(pc->addr) & vbCrLfW)
    
    ' 十六进制 + ASCII 双栏输出
    For i = 0 To bytesRead - 1
        If (i Mod 16) = 0 Then
            If i > 0 Then
                SafeStdOut("  " & asciiPart & vbCrLfW)
            End If
            hexLine = Hex(pc->addr + i, 16) & "  "   ' 内核地址用16位宽
            hexPart = ""
            asciiPart = ""
        End If
        
        b = buf(i)
        hexPart = hexPart & Hex(b, 2) & " "
        
        If b >= 32 And b <= 126 Then
            asciiPart = asciiPart & WChr(b)
        Else
            asciiPart = asciiPart & "."
        End If
    Next i
    
    ' 补齐最后一行
    If bytesRead > 0 Then
        Dim padCount As Integer = (16 - (bytesRead Mod 16)) Mod 16
        For i = 1 To padCount
            hexPart = hexPart & "   "
        Next i
        SafeStdOut(hexLine & hexPart & "  " & asciiPart & vbCrLfW)
    End If
    
    Return True
End Function

Function Exec_KernelMemory_Write(ByVal pc As ParsedCmd Ptr) As Boolean
    Dim writeBuf() As Byte
    Dim i As Integer
    Dim dataLen As Integer
    
    ' ── 参数校验 ─────────────────────────────────────────
    If pc->hasAddr = False Then
        pc->hasError = True
        pc->errorMsg = "Error: --addr required for kernel memory write"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 驱动检查 ─────────────────────────────────────────
    If Not IsDriverLoaded Then
        pc->hasError = True
        pc->errorMsg = "Error: Driver not loaded. Use 'load-driver' first."
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 准备写入数据 ───────────────────────────────────────
    ' 优先使用 hexData 解析后的 bytdata
    If pc->bytdata <> NULL And pc->size > 0 Then
        dataLen = pc->size
        ReDim writeBuf(dataLen - 1)
        CopyMemory(@writeBuf(0), pc->bytdata, dataLen)
    ElseIf LenW(pc->hexData) > 0 Then
        ' 如果 bytdata 未设置但 hexData 有值，尝试解析（兜底）
        ' 这里简化：直接报错，要求上层先解析
        pc->hasError = True
        pc->errorMsg = "Error: hex data not parsed. Internal error."
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    ElseIf pc->size > 0 Then
        ' 无显式数据，按 size 写零
        dataLen = pc->size
        If dataLen > &H1000000 Then
            pc->hasError = True
            pc->errorMsg = "Error: --size exceeds 16MB limit"
            SafeStdOut(pc->errorMsg & vbCrLfW)
            Return False
        End If
        ReDim writeBuf(dataLen - 1)
        ' 数组已自动初始化为 0
    Else
        pc->hasError = True
        pc->errorMsg = "Error: --data <hex> required for write, or use --size N for zero-fill"
        SafeStdOut(pc->errorMsg & vbCrLfW)
        SafeStdOut("Usage: kernel-memory write --addr <ADDR> --data <HEX>" & vbCrLfW)
        SafeStdOut("       kernel-memory write --addr <ADDR> --size <N>  (writes N zero bytes)" & vbCrLfW)
        Return False
    End If
    
    ' ── Dry Run 模式 ───────────────────────────────────────
    If pc->dryRun Then
        SafeStdOut("[DRY RUN] Would write " & dataLen & " bytes to kernel address 0x" & Hex(pc->addr) & vbCrLfW)
        Return True
    End If
    
    ' ── 执行内核写入 ───────────────────────────────────────
    If Not WriteKernelMemory(Cast(UINTEGER, pc->addr), dataLen, @writeBuf(0)) Then
        pc->hasError = True
        pc->errorMsg = "Error: WriteKernelMemory failed at 0x" & Hex(pc->addr)
        SafeStdOut(pc->errorMsg & vbCrLfW)
        Return False
    End If
    
    ' ── 输出结果 ───────────────────────────────────────────
    SafeStdOut("Wrote " & dataLen & " bytes to kernel address 0x" & Hex(pc->addr) & vbCrLfW)
    
    ' 打印写入内容的摘要（前 32 字节）
    If dataLen <= 32 Then
        Dim hexStr As WString * 100 = ""
        For i = 0 To dataLen - 1
            hexStr = hexStr & Hex(writeBuf(i), 2) & " "
        Next i
        SafeStdOut("Data: " & hexStr & vbCrLfW)
    Else
        Dim hexStr As WString * 100 = ""
        For i = 0 To 15
            hexStr = hexStr & Hex(writeBuf(i), 2) & " "
        Next i
        SafeStdOut("Data: " & hexStr & "... (" & dataLen & " bytes total)" & vbCrLfW)
    End If
    
    Return True
End Function

Function Exec_Thread_CallStack(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = ThreadCallStack
    CurrentInformation.ProcessId = pc->pid : CurrentInformation.ThreadId = pc->tid
    InitializeListView ThreadCallStack, FrmMain.ListView1
    GetCallStack pc->pid, pc->tid, FrmMain.ListView1
    CuiPrintListViewData()
    Return True
End Function

Function Exec_File_List(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = File
    InitializeListView File, FrmMain.ListView1
    InitializeTreeView FrmMain.TreeView
    If pc->path = "" Then
        'GetDriveList FrmMain.TreeView, False
    Else
        GetFileList_CLI pc->path
    End If
    Return True
End Function

Function Exec_Registry_List(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = Registry
    InitializeListView Registry, FrmMain.ListView1
    InitializeTreeView FrmMain.TreeView
    If pc->path = "" Then
        GetRootList FrmMain.TreeView
    Else
        GetRegList_CLI pc->path
    End If
    Return True
End Function

' --------------- 编辑类执行函数（预留接口） ---------------
Function Exec_Process_Kill(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: terminate process " & pc->pid & vbCrlfW)
    If ForceKillProcess(pc->pid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Process_ForceKill(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: force terminate process " & pc->pid & vbCrlfW)
    If IoControl(hDrv, IOCTL_MemKillProcess, @pc->pid, SizeOf(DWORD)) <> 0 Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Process_Suspend(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: suspend process " & pc->pid & vbCrlfW)
    If SuspendProcess(pc->pid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Process_Resume(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: resume process " & pc->pid & vbCrlfW)
    If ResumeProcess(pc->pid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Thread_Kill(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: kill thread " & pc->tid & ",pid=" & pc->pid & vbCrlfW)
    If KillThread(pc->tid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Thread_Resume(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: resume thread " & pc->tid & ",pid=" & pc->pid & vbCrlfW)
    If MyResumeThread(pc->tid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Thread_Suspend(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: suspend thread " & pc->tid & ",pid=" & pc->pid & vbCrlfW)
    If MySuspendThread(pc->tid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Registry_Write(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: Write registry " & pc->path & " -> " & pc->regName & " = " & pc->hexData & " (" & pc->regType & ")" & vbCrlfW)
    ' TODO: 调用 RegSetValueEx 或驱动接口
    Return True
End Function

Function Exec_Registry_Delete(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: Delete registry value " & pc->path & " -> " & pc->regName & vbCrlfW)
    ' TODO: 调用 RegDeleteValue 或驱动接口
    Return True
End Function

Function Exec_Handle_Close(ByVal pc As ParsedCmd Ptr) As Boolean
    'Return CloseProcessHandle(pc->pid
End Function

' --------------- 统一分发器 ---------------
Function DispatchCommand(ByVal pc As ParsedCmd Ptr, ByVal def As CmdEntry Ptr) As Boolean
    Select Case def->pidMode
    Case PM_REQUIRED
        If pc->hasPid = False Then
            SafeStdOut("ERROR: --pid is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
            Return False
        End If
    Case PM_OPTIONAL
        ' 什么都不做，hasPid=False 就是查全部
    End Select
    Select Case def->tidMode
    Case PM_REQUIRED
        If pc->hasTid = False Then
            SafeStdOut("ERROR: --tid is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
            Return False
        End If
    End Select
    Select Case def->pathMode
    Case PM_REQUIRED
        If pc->hasPath = False Then
            SafeStdOut("ERROR: --path is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
            Return False
        End If
    End Select
    Select Case def->addrMode
    Case PM_REQUIRED
        If pc->hasAddr = False Then
            SafeStdOut("ERROR: --addr is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
            Return False
        End If
    End Select
    
    ' 驱动检查
    If def->needsDriver AndAlso Not IsDriverLoaded Then
        SafeStdOut("ERROR: Driver not loaded. Use 'load-driver' first." & vbCrlfW)
        Return False
    End If
    
    ' 安全确认
    If Not ConfirmDestructive(pc, def) Then Return False
    
    ' 如果是 dry-run，确认函数已经输出了预览，这里直接返回成功
    If pc->dryRun Then Return True
    
    ' 路由到具体实现（按 category + action 组合键）【修复P8】
    Dim key As WString * MAX_PATH = LCaseW(def->category) & "|" & LCaseW(def->action)
    Select Case key
        Case "process|list"       : Return Exec_Process_List(pc)
        Case "kernel-module|list" : Return Exec_KernelModule_List(pc)
        Case "kernel-thread|list"
            InitializeListView KernelThread, FrmMain.ListView1
            If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW)
            GetKernelThreadList FrmMain.ListView1
            CuiPrintListViewData()
            Return True
        Case "ssdt|list"          : InitializeListView SSDT, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetSSDT FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "sssdt|list"          : InitializeListView ShadowSSDT, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetSSSDT FrmMain.ListView1 : CuiPrintListViewData() : Return True
            
        Case "hal-dispatch|read"  : InitializeListView HalDispatch, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHalDispatchTable FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hal-private-dispatch|read": InitializeListView HalPrivateDispatch, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHalPrivateDispatchTable FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "object-hook|list"   : InitializeListView ObjectHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetObjectInfo FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "callbacks|list"     : InitializeListView Callbacks, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetCallbackList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "kernel-timer|list"  : InitializeListView KernelTimer, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetKernelTimerList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "work-item-thread|list": InitializeListView WorkItemThread, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWorkItemThreadList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "wfp-callout|list"   : InitializeListView WfpCallout, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWfpCalloutList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "wfp-filter|list"    : InitializeListView WfpFilter, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWfpFilterList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "service|list"       : InitializeListView Service, FrmMain.ListView1 : GetServiceList FrmMain.ListView1 : CuiPrintListViewData() : Return True
            
        Case "etw|list"  : InitializeTreeList Etw, FrmMain.mCtrlTreeList1 : EnumETWSession FrmMain.mCtrlTreeList1 : Return True
        Case "winsock-spi|list"   : InitializeListView WinsockSPI, FrmMain.ListView1 : GetWinsockSPIList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "task-scheduler|list": InitializeListView TaskScheduler, FrmMain.ListView1 : GetTaskSchedulerList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "unlock-file|list" : InitializeListView UnlockTheFile, FrmMain.ListView1 : Unlock_File FrmMain.ListView1 : CuiPrintListViewData() : Return True
        
        ' GDT / IDT / Minifilter / FilterDriver 等 TreeList 模块保持原样
        Case "gdt|read"           : InitializeTreeList GDT, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetGDT FrmMain.mCtrlTreeList1 : Return True
        Case "idt|read"           : InitializeTreeList IDT, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetIDT FrmMain.mCtrlTreeList1 : Return True
        Case "minifilter|list"    : InitializeTreeList Minifilter, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else EnumMinifilter FrmMain.mCtrlTreeList1 : Return True
        Case "filterdriver|list"  : InitializeTreeList FilterDriver, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else EnumAttachDevices FrmMain.mCtrlTreeList1 : Return True
            
        Case "kernel-memory|read"  : Return Exec_KernelMemory_Read(pc)
        Case "kernel-memory|write" : Return Exec_KernelMemory_Write(pc)
            
        ' 二级查询
        Case "thread|list"        : Return Exec_Thread_List(pc)
        Case "module|list"        : InitializeListView Module, FrmMain.ListView1 : GetModuleList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "windows|list"       : InitializeListView Windows, FrmMain.ListView1 : GetWindowList False, pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "handle|list"
            InitializeListView Handles, FrmMain.ListView1
            If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW)
            GetHandleList Not pc->hasPid, pc->pid, FrmMain.ListView1
            CuiPrintListViewData()
            Return True
        Case "memory|list"      : InitializeListView Memory, FrmMain.ListView1 : GetMemoryBlockList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "memory|read"        : Return Exec_Memory_Read(pc)
        Case "memory|write"        : Return Exec_Memory_Write(pc)
        Case "processtimer|list"  : InitializeListView ProcessTimer, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetProcessTimerList Not pc->hasPid, pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "webconnect|list"    : InitializeListView WebConnect, FrmMain.ListView1 : GetWebConnectList pc->pid, FrmMain.ListView1, Not pc->hasPid : CuiPrintListViewData() : Return True
        Case "privilege|list"     : InitializeListView Privilege, FrmMain.ListView1 : GetProcessPrivilegeList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hook|list"          : InitializeListView Hook, FrmMain.ListView1 : GetProcessHookList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "msghook|list"       : InitializeListView MsgHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetMsgHookList Not pc->hasPid, False, pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "eventhook|list"     : InitializeListView EventHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetEventHookList Not pc->hasPid, pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hotkey|list"        : InitializeListView Hotkey, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHotkeyList Not pc->hasPid, pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        
        ' 三级查询
        Case "thread|callstack"   : Return Exec_Thread_CallStack(pc)
        
        ' 文件/注册表查询
        Case "file|list"          : Return Exec_File_List(pc)
        Case "registry|list"      : Return Exec_Registry_List(pc)
        
        ' 编辑/破坏性命令
        Case "process|suspend"   : Return Exec_Process_Suspend(pc)
        Case "process|resume"     : Return Exec_Process_Resume(pc)
        Case "process|kill"            : Return Exec_Process_Kill(pc)
        Case "process|force-kill"  : Return Exec_Process_ForceKill(pc)
            
        Case "thread|suspend"     : Return Exec_Thread_Suspend(pc)
        Case "thread|resume"      : Return Exec_Thread_Resume(pc)
        Case "thread|kill"             : Return Exec_Thread_Kill(pc)

        Case "registry|write"        : Return Exec_Registry_Write(pc)
        Case "registry|delete"      : Return Exec_Registry_Delete(pc)
        Case "file|delete" : SafeStdOut("Executing: Delete file " & pc->path & vbCrlfW) : Return True
        
        Case "unlock-file|unlock" : Return 1
        
        Case Else
            SafeStdOut("ERROR: Command '" & key & "' registered but handler not implemented." & vbCrlfW)
            Return False
    End Select
End Function

' ===================== 管道 / 重定向 / 过滤器 =====================
' 语义（已确认）：
'   1. | 右侧只允许内置过滤器 findstr / sort / head
'   2. 过滤器只作用于 DATA_START ~ DATA_END 之间的数据行，框线原样保留；
'      无标记输出（help、内存转储等）整体透传
'   3. 出现 | > >> 时强制文本(分隔符)形式输出，绕过 --format json
'   4. > 覆盖写，>> 追加写，UTF-8 无 BOM

Enum CuiRedirectMode
    CR_NONE = 0
    CR_OVERWRITE = 1
    CR_APPEND = 2
End Enum

' ---------- 行拆分 / 合并 ----------
Function CuiSplitLines(ByRef text As StringW, lines() As StringW) As Long
    Dim L As Long = LenW(text)
    Dim n As Long = 0
    Dim startIdx As Long = 0
    Dim i As Long = 0
    While i < L
        Dim ch As Long = text[i]
        If ch = 13 Then
            If i + 1 < L AndAlso text[i + 1] = 10 Then
                n += 1
                If n = 1 Then Redim lines(0) Else Redim Preserve lines(n - 1)
                lines(n - 1) = MidW(text, startIdx + 1, i - startIdx)
                i += 2
                startIdx = i
            Else
                n += 1
                If n = 1 Then Redim lines(0) Else Redim Preserve lines(n - 1)
                lines(n - 1) = MidW(text, startIdx + 1, i - startIdx)
                i += 1
                startIdx = i
            End If
        ElseIf ch = 10 Then
            n += 1
            If n = 1 Then Redim lines(0) Else Redim Preserve lines(n - 1)
            lines(n - 1) = MidW(text, startIdx + 1, i - startIdx)
            i += 1
            startIdx = i
        Else
            i += 1
        End If
    Wend
    If startIdx < L Then
        n += 1
        If n = 1 Then Redim lines(0) Else Redim Preserve lines(n - 1)
        lines(n - 1) = MidW(text, startIdx + 1, L - startIdx)
    End If
    Return n
End Function

Sub CuiJoinLines(lines() As StringW, ByVal n As Long, ByRef outText As StringW)
    outText = ""
    For i As Long = 0 To n - 1
        outText &= lines(i) & vbCrlfW
    Next
End Sub

' ---------- 数据行区间检测 ----------
' 返回 True 并给出 [sIdx, eIdx) 数据行区间（不含 DATA_START / DATA_END 标记行）
Function CuiFindDataRegion(lines() As StringW, ByVal n As Long, ByRef sIdx As Long, ByRef eIdx As Long) As Boolean
    sIdx = -1 : eIdx = -1
    For i As Long = 0 To n - 1
        Dim t As StringW = LCaseW(MyTrimW(lines(i), " " & vbCrlfW))
        If t = "data_start" Then
            sIdx = i + 1
        ElseIf t = "data_end" Then
            eIdx = i
            Return True
        End If
    Next
    Return False
End Function

' 用新的数据行集合重建完整输出（框线原样保留）
Function CuiRebuildData(lines() As StringW, ByVal n As Long, ByVal sIdx As Long, ByVal eIdx As Long, kept() As StringW, ByVal keptN As Long) As StringW
    Dim outLines() As StringW
    Dim m As Long = 0
    For i As Long = 0 To sIdx - 1
        m += 1
        If m = 1 Then Redim outLines(0) Else Redim Preserve outLines(m - 1)
        outLines(m - 1) = lines(i)
    Next
    For i As Long = 0 To keptN - 1
        m += 1
        If m = 1 Then Redim outLines(0) Else Redim Preserve outLines(m - 1)
        outLines(m - 1) = kept(i)
    Next
    For i As Long = eIdx To n - 1
        m += 1
        If m = 1 Then Redim outLines(0) Else Redim Preserve outLines(m - 1)
        outLines(m - 1) = lines(i)
    Next
    Dim result As StringW = ""
    CuiJoinLines(outLines(), m, result)
    Return result
End Function

' ---------- findstr ----------
Function CuiFilterFindstr(ByRef text As StringW, ByRef pattern As StringW, ByVal invert As Boolean) As StringW
    Dim lines() As StringW
    Dim n As Long = CuiSplitLines(text, lines())
    Dim sIdx As Long = -1, eIdx As Long = -1
    If Not CuiFindDataRegion(lines(), n, sIdx, eIdx) Then Return text
    Dim lp As StringW = LCaseW(pattern)
    Dim kept() As StringW
    Dim k As Long = 0
    For i As Long = sIdx To eIdx - 1
        Dim match As Boolean = (InStrW(LCaseW(lines(i)), lp) > 0)
        If invert Then match = Not match
        If match Then
            k += 1
            If k = 1 Then Redim kept(0) Else Redim Preserve kept(k - 1)
            kept(k - 1) = lines(i)
        End If
    Next
    Return CuiRebuildData(lines(), n, sIdx, eIdx, kept(), k)
End Function

' ---------- sort ----------
Sub CuiSortLines(lines() As StringW, ByVal n As Long, ByVal descending As Boolean)
    For i As Long = 1 To n - 1
        Dim key As StringW = lines(i)
        Dim keyL As StringW = LCaseW(key)
        Dim j As Long = i - 1
        While j >= 0
            Dim needMove As Boolean
            If descending Then
                needMove = (LCaseW(lines(j)) < keyL)
            Else
                needMove = (LCaseW(lines(j)) > keyL)
            End If
            If Not needMove Then Exit While
            lines(j + 1) = lines(j)
            j -= 1
        Wend
        lines(j + 1) = key
    Next
End Sub

Function CuiFilterSort(ByRef text As StringW, ByVal descending As Boolean) As StringW
    Dim lines() As StringW
    Dim n As Long = CuiSplitLines(text, lines())
    Dim sIdx As Long = -1, eIdx As Long = -1
    If Not CuiFindDataRegion(lines(), n, sIdx, eIdx) Then Return text
    Dim dataN As Long = eIdx - sIdx
    Dim kept() As StringW
    If dataN > 0 Then
        Redim kept(dataN - 1)
        For i As Long = 0 To dataN - 1
            kept(i) = lines(sIdx + i)
        Next
        CuiSortLines(kept(), dataN, descending)
    End If
    Return CuiRebuildData(lines(), n, sIdx, eIdx, kept(), dataN)
End Function

' ---------- head ----------
Function CuiFilterHead(ByRef text As StringW, ByVal count As Long) As StringW
    If count <= 0 Then Return text
    Dim lines() As StringW
    Dim n As Long = CuiSplitLines(text, lines())
    Dim sIdx As Long = -1, eIdx As Long = -1
    If Not CuiFindDataRegion(lines(), n, sIdx, eIdx) Then Return text
    Dim dataN As Long = eIdx - sIdx
    Dim keptN As Long = dataN
    If keptN > count Then keptN = count
    Dim kept() As StringW
    Dim k As Long = 0
    For i As Long = sIdx To sIdx + keptN - 1
        k += 1
        If k = 1 Then Redim kept(0) Else Redim Preserve kept(k - 1)
        kept(k - 1) = lines(i)
    Next
    Return CuiRebuildData(lines(), n, sIdx, eIdx, kept(), k)
End Function

' ---------- 管道顶层分词 ----------
' 返回 True 并把各段写入 stages()，通过 stageCount 返回段数；
' 重定向模式 CR_NONE/CR_OVERWRITE/CR_APPEND 与路径由 redirectMode/redirectPath 返回。
Function CuiSplitPipeline(ByRef cmd As StringW, stages() As StringW, ByRef stageCount As Long, ByRef redirectMode As Long, ByRef redirectPath As StringW) As Boolean
    stageCount = 0
    redirectMode = CR_NONE
    redirectPath = ""
    
    Dim cur As StringW = ""
    Dim inQuote As Boolean = False
    Dim L As Long = LenW(cmd)
    Dim i As Long = 0
    
    While i < L
        Dim ch As Long = cmd[i]
        
        If ch = 34 Then
            inQuote = Not inQuote
            cur &= WChr(ch)
            i += 1
            Continue While
        End If
        
        If inQuote Then
            cur &= WChr(ch)
            i += 1
            Continue While
        End If
        
        If ch = 124 Then  ' |
            Dim st As StringW = MyTrimW(cur, " " & vbCrlfW)
            If LenW(st) > 0 Then
                If stageCount = 0 Then Redim stages(0) Else Redim Preserve stages(stageCount)
                stages(stageCount) = st
                stageCount += 1
            End If
            cur = ""
            i += 1
            Continue While
        End If
        
        If ch = 62 Then  ' >
            If redirectMode <> CR_NONE Then Return False
            Dim st2 As StringW = MyTrimW(cur, " " & vbCrlfW)
            If LenW(st2) > 0 Then
                If stageCount = 0 Then Redim stages(0) Else Redim Preserve stages(stageCount)
                stages(stageCount) = st2
                stageCount += 1
            End If
            If i + 1 < L AndAlso cmd[i + 1] = 62 Then
                redirectMode = CR_APPEND
                redirectPath = MyTrimW(MidW(cmd, i + 3), " " & vbCrlfW)
            Else
                redirectMode = CR_OVERWRITE
                redirectPath = MyTrimW(MidW(cmd, i + 2), " " & vbCrlfW)
            End If
            Exit While
        End If
        
        cur &= WChr(ch)
        i += 1
    Wend
    
    If redirectMode = CR_NONE Then
        Dim st3 As StringW = MyTrimW(cur, " " & vbCrlfW)
        If LenW(st3) > 0 Then
            If stageCount = 0 Then Redim stages(0) Else Redim Preserve stages(stageCount)
            stages(stageCount) = st3
            stageCount += 1
        End If
    End If
    
    ' 去除路径外层引号
    If LenW(redirectPath) >= 2 AndAlso redirectPath[0] = 34 AndAlso redirectPath[LenW(redirectPath) - 1] = 34 Then
        redirectPath = MidW(redirectPath, 2, LenW(redirectPath) - 2)
    End If
    
    ' 重定向必须位于末尾；路径中不能再出现 | 或 >
    If redirectMode <> CR_NONE Then
        If LenW(redirectPath) = 0 Then Return False
        If InStrW(redirectPath, "|") > 0 OrElse InStrW(redirectPath, ">") > 0 Then Return False
    End If
    
    Return True
End Function

' ---------- 重定向写文件（UTF-8 无 BOM） ----------
Function CuiWriteRedirectFile(ByRef path As StringW, ByRef text As StringW, ByVal bAppend As Boolean) As Boolean
    Dim hFile As HANDLE = INVALID_HANDLE_VALUE
    If bAppend Then
        hFile = CreateFileW(StrPtrW(path), GENERIC_WRITE, FILE_SHARE_READ Or FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    Else
        hFile = CreateFileW(StrPtrW(path), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    End If
    
    If hFile = INVALID_HANDLE_VALUE Then
        SafeStdOut("ERROR: Cannot open file '" & path & "' (err=" & GetLastError() & ")" & vbCrlfW)
        Return False
    End If
    
    If bAppend Then
        Dim liMove As LARGE_INTEGER
        liMove.QuadPart = 0
        SetFilePointerEx(hFile, liMove, NULL, FILE_END)
    End If
    
    Dim cbNeeded As Integer = WideCharToMultiByte(CP_UTF8, 0, StrPtrW(text), LenW(text), NULL, 0, NULL, NULL)
    If cbNeeded > 0 Then
        Dim utf8Buf As ZString Ptr = Allocate(cbNeeded)
        If utf8Buf <> NULL Then
            WideCharToMultiByte(CP_UTF8, 0, StrPtrW(text), LenW(text), utf8Buf, cbNeeded, NULL, NULL)
            Dim written As DWORD
            WriteFile(hFile, utf8Buf, cbNeeded, @written, NULL)
            Deallocate(utf8Buf)
        End If
    End If
    
    CloseHandle(hFile)
    Return True
End Function

Function CuiTokenize(ByRef text As StringW, tokens() As StringW) As Long
    Dim n As Long = 0
    Dim cur As StringW = ""
    Dim inQuote As Boolean = False

    Dim L As Long = LenW(text)
    Dim i As Long = 0

    While i < L
        Dim ch As Long = text[i]
        If ch = 34 Then
            ' 双引号只作为语法符号，不放入 token
            inQuote = Not inQuote
            i += 1
            Continue While
        End If
        
        If ch = 32 OrElse ch = 9 Then
            If inQuote Then
                cur &= WChr(ch)
            Else
                If LenW(cur) > 0 Then
                    If n = 0 Then
                        Redim tokens(0)
                    Else
                        Redim Preserve tokens(n)
                    End If

                    tokens(n) = cur
                    n += 1
                    cur = ""
                End If
            End If
        Else
            cur &= WChr(ch)
        End If
        i += 1
    Wend

    If LenW(cur) > 0 Then
        If n = 0 Then
            Redim tokens(0)
        Else
            Redim Preserve tokens(n)
        End If

        tokens(n) = cur
        n += 1
    End If
    Return n
End Function

' 新增辅助函数：把相对路径转为完整路径，使用全局工作目录 g_cwd
' 规则：
' 1. 以 X:\ 开头 或者 \\UNC路径 → 原样返回
' 2. 其余视为相对路径，拼接 g_cwd
Function MakeFullPathW(ByVal path As StringW) As StringW
    If LenW(path) = 0 Then Return path

    ' UNC \\ 开头
    If LeftW(path, 2) = "\\\" Then
        Return path
    End If

    ' 判断盘符格式，如 C:  C:\
    Dim firstCh As StringW = LeftW(path,1)
    If (firstCh >= "A" AndAlso firstCh <= "Z") OrElse (firstCh >= "a" AndAlso firstCh <= "z") Then
        If MidW(path,2,1) = ":" Then
            ' 已经带盘符，直接返回
            Return path
        End If
    End If

    ' 相对路径，拼接g_cwd
    Dim combined As StringW
    If RightW(g_cwd,1) = "\" Then
        combined = g_cwd & path
    Else
        combined = g_cwd & "\" & path
    End If
    Return combined
End Function

' ---------- 执行单个命令段（复用原有解析/分发逻辑） ----------
Function ExecuteCuiCommand(ByRef stageText As StringW) As Boolean
    Dim cmdParts() As StringW
    Dim numParts As Long = CuiTokenize(stageText, cmdParts())
    If numParts = 0 Then Return False
    
    Dim verb As StringW = LCaseW(cmdParts(0))
    
    ' ---------------- 全局命令 ----------------
    If verb = "load-driver" Then
        If InitDriver Then SafeStdOut("SUCCESS: Driver loaded." & vbCrlfW) Else SafeStdOut("ERROR: Driver load failed." & vbCrlfW)
        Return True
    ElseIf verb = "unload-driver" Then
        UninitDriver
        SafeStdOut("SUCCESS: Driver unloaded." & vbCrlfW)
        Return True
    ElseIf verb = "help" Then
        If numParts = 1 Then
            PrintGlobalHelp()
        ElseIf numParts >= 2 Then
            Dim tmp As ParsedCmd
            tmp.category = cmdParts(1)
            If numParts >= 3 Then tmp.action = cmdParts(2) Else tmp.action = ""
            Dim def As CmdEntry Ptr = FindCmd(tmp.category, tmp.action)
            If def <> NULL AndAlso numParts >= 3 Then
                PrintActionHelp(def)
            Else
                PrintCategoryHelp(cmdParts(1))
            End If
        End If
        Return True
    End If
    
    ' ---------------- 子命令解析 ----------------
    Dim pc As ParsedCmd
    If Not ParseCommandLine(cmdParts(), numParts, @pc) Then
        SafeStdOut(pc.errorMsg & vbCrlfW)
        Return False
    End If
    
    If pc.showHelp Then
        Dim def As CmdEntry Ptr = FindCmd(pc.category, pc.action)
        If def <> NULL Then
            PrintActionHelp(def)
        Else
            SafeStdOut("Unknown command. Type 'help' for available commands." & vbCrlfW)
        End If
        Return True
    End If
    
    Dim def2 As CmdEntry Ptr = FindCmd(pc.category, pc.action)
    If def2 = NULL Then
        SafeStdOut("ERROR: Unknown command '" & pc.category & " " & pc.action & "'. Type 'help' for available commands." & vbCrlfW)
        Return False
    End If
    
    Return DispatchCommand(@pc, def2)
End Function

' ---------- 共享命令行处理：管道解析 + 捕获 + 过滤 + 重定向 ----------
' 返回 False 表示退出 REPL（exit）
Function ProcessCuiLine(ByRef cmdLine As StringW) As Boolean
    Dim cmd As StringW = MyTrimW(cmdLine, vbCrlfW & " ")
    If LenW(cmd) = 0 Then Return True

    Dim firstParts() As StringW

    Dim firstN As Long = CuiTokenize(cmd, firstParts())
    If firstN > 0 AndAlso LCaseW(firstParts(0)) = "exit" Then Return False

    Dim stages() As StringW
    Dim numStages As Long = 0
    Dim redirMode As Long
    Dim redirPath As StringW

    If Not CuiSplitPipeline(cmd, stages(), numStages, redirMode, redirPath) Then
        SafeStdOut("ERROR: Invalid pipeline/redirection syntax. Redirect must be last and used only once." & vbCrlfW)
        Return True
    End If

    If numStages = 0 Then
        If redirMode <> CR_NONE Then SafeStdOut("ERROR: No command before redirect." & vbCrlfW)
        Return True
    End If

    ' 校验 | 右侧只能是内置过滤器
    For s As Long = 1 To numStages - 1
        Dim sp() As StringW

        Dim sn As Long = CuiTokenize(stages(s), sp())
        If sn > 0 Then
            Dim fv As StringW = LCaseW(sp(0))
            If fv <> "findstr" AndAlso fv <> "sort" AndAlso fv <> "head" Then
                SafeStdOut("ERROR: '|' only supports the built-in filters findstr/sort/head." & vbCrlfW)
                Return True
            End If
        End If
    Next

    ' 首段不能是过滤器
    Dim s0Parts() As StringW

    Dim s0N As Long = vbSplitW(stages(0), " ", s0Parts())
    If s0N > 0 Then
        Dim s0v As StringW = LCaseW(s0Parts(0))
        If s0v = "findstr" OrElse s0v = "sort" OrElse s0v = "head" Then
            SafeStdOut("ERROR: '" & s0v & "' requires piped input from a command (e.g. process list | " & s0v & " ...)." & vbCrlfW)
            Return True
        End If
    End If

    ' 纯命令（无管道/重定向）：保持原有直出行为
    If numStages = 1 AndAlso redirMode = CR_NONE Then
        ExecuteCuiCommand(stages(0))
        Return True
    End If

    ' 开始捕获
    g_pipeBuffer = ""

    g_pipeCapture = True
    g_forceText = True

    Dim execOk As Boolean = ExecuteCuiCommand(stages(0))

    g_pipeCapture = False
    g_forceText = False

    ' 命令失败：错误直出，跳过过滤/重定向

    If Not execOk Then
        SafeStdOutDirect(g_pipeBuffer)
        g_pipeBuffer = ""
        Return True
    End If
    
    ' 依次应用过滤器
    Dim result As StringW = g_pipeBuffer

    For s As Long = 1 To numStages - 1
        Dim fsp() As StringW

        Dim fn As Long = CuiTokenize(MyTrimW(stages(s), " " & vbCrlfW), fsp())
        If fn = 0 Then Continue For

        Dim fv As StringW = LCaseW(fsp(0))

        If fv = "findstr" Then
            Dim invert As Boolean = False
            Dim pat As StringW = ""
            For k As Long = 1 To fn - 1
                Dim a As StringW = LCaseW(fsp(k))
                If a = "/v" Then
                    invert = True
                ElseIf a = "/i" OrElse a = "/c" Then
                    ' 兼容接受
                Else
                    If LenW(pat) = 0 Then pat = fsp(k) Else pat = pat & " " & fsp(k)
                End If
            Next
            If LenW(pat) = 0 Then
                SafeStdOut("ERROR: findstr requires a pattern (findstr [options] <pattern>)." & vbCrlfW)
                g_pipeBuffer = ""
                Return True
            End If
            result = CuiFilterFindstr(result, pat, invert)
        ElseIf fv = "sort" Then
            Dim desc As Boolean = False
            For k As Long = 1 To fn - 1
                If LCaseW(fsp(k)) = "/r" Then desc = True
            Next
            result = CuiFilterSort(result, desc)

        ElseIf fv = "head" Then
            Dim cnt As Long = 10
            If fn >= 2 Then
                cnt = Val(fsp(1))
                If cnt <= 0 Then
                    SafeStdOut("ERROR: head requires a positive line count (head [N])." & vbCrlfW)
                    g_pipeBuffer = ""
                    Return True
                End If
            End If
            result = CuiFilterHead(result, cnt)
        End If
    Next

    ' ========== 新增：相对路径自动拼接当前工作目录 ==========
    If redirMode <> CR_NONE Then
        redirPath = MakeFullPathW(redirPath)
        CuiWriteRedirectFile(redirPath, result, (redirMode = CR_APPEND))
    Else
        If LenW(result) > 0 Then SafeStdOut(result)
    End If
    
    g_pipeBuffer = ""
    Return True
End Function

' ===================== 标准输入逐行读取（兼容控制台 / 重定向 stdin）=====================
Dim Shared g_stdinInit As Boolean = False
Dim Shared g_stdinIsConsole As Boolean = False
Dim Shared g_stdinPending As StringW = ""
Dim Shared g_stdinByteBuf(4095) As Byte

Function ReadLineFromStdin(ByRef outLine As StringW) As Boolean
    If Not g_stdinInit Then
        g_stdinInit = True
        Dim hInTmp As HANDLE = GetStdHandle(STD_INPUT_HANDLE)
        Dim dwMode As DWORD
        g_stdinIsConsole = GetConsoleMode(hInTmp, @dwMode)
    End If

    ' 先取缓存中已完整的一行
    Dim lfPos As Long = InStrW(g_stdinPending, WChr(10))
    If lfPos > 0 Then
        outLine = LeftW(g_stdinPending, lfPos - 1)
        g_stdinPending = MidW(g_stdinPending, lfPos + 1)
        Return True
    End If

    If g_stdinIsConsole Then
        ' 控制台：一次 ReadConsoleW 即一行（保留交互编辑）
        Dim hIn As HANDLE = GetStdHandle(STD_INPUT_HANDLE)
        Dim inputBuf As WString * INPUT_BUF_SIZE
        Dim rd As DWORD
        If ReadConsoleW(hIn, @inputBuf, INPUT_BUF_SIZE - 1, @rd, NULL) = 0 Then Return False
        If rd = 0 Then Return False
        inputBuf[rd] = 0
        outLine = LeftW(inputBuf, rd)
        Return True
    End If

    ' 重定向：按字节累积，遇 LF 或 EOF 返回一行
    Do
        Dim nRead As DWORD
        If ReadFile(GetStdHandle(STD_INPUT_HANDLE), @g_stdinByteBuf(0), 4096, @nRead, NULL) = 0 OrElse nRead = 0 Then
            ' EOF：剩余内容作为最后一行
            If LenW(g_stdinPending) > 0 Then
                outLine = g_stdinPending
                g_stdinPending = ""
                Return True
            End If
            Return False
        End If

        Dim cw As Integer = MultiByteToWideChar(CP_UTF8, 0, @g_stdinByteBuf(0), nRead, NULL, 0)
        If cw > 0 Then
            Dim ws As WString * 8192
            MultiByteToWideChar(CP_UTF8, 0, @g_stdinByteBuf(0), nRead, @ws, 8191)
            ws[cw] = 0
            g_stdinPending &= LeftW(ws, cw)
        End If

        lfPos = InStrW(g_stdinPending, WChr(10))
        If lfPos > 0 Then
            outLine = LeftW(g_stdinPending, lfPos - 1)
            g_stdinPending = MidW(g_stdinPending, lfPos + 1)
            Return True
        End If
    Loop
End Function

' ===================== 主 REPL 循环（重构版） =====================
Sub RunAgentRepl()
    InitCommandRegistry()  ' 【修复P9】启动时统一注册

    ' 仅在真实控制台时打印横幅/提示符；重定向 stdin 时保持输出干净
    Dim hInChk As HANDLE = GetStdHandle(STD_INPUT_HANDLE)
    Dim dwChk As DWORD
    Dim isConsoleMode As Boolean = GetConsoleMode(hInChk, @dwChk)
    If isConsoleMode Then
        SafeStdOut("SnowSword Agent Shell v2.0" & vbCrlfW)
        SafeStdOut("Format: <category> <action> [options] | findstr/sort/head [args] [> file] [>> file]" & vbCrlfW)
        SafeStdOut("Example: process list | findstr svchost | sort | head 20" & vbCrlfW)
        SafeStdOut("Type 'help' or '<category> <action> --help' for assistance." & vbCrlfW & vbCrlfW)
    End If

    Do
        If isConsoleMode Then SafeStdOut("ANTIROOTKIT> ")

        Dim cmd As StringW
        If Not ReadLineFromStdin(cmd) Then Exit Do
        cmd = MyTrimW(cmd, vbCrlfW & " ")
        If LenW(cmd) = 0 Then Continue Do

        ' 统一命令处理：全局命令 + 管道/重定向/过滤器
        If Not ProcessCuiLine(cmd) Then
            UninitDriver
            Exit Do
        End If
    Loop

    End
End Sub

' ===================== Agent 管道 REPL =====================
Sub RunAgentPipeRepl()
    Dim pipeName As WString * 128 = g_AgentPipeName
    If pipeName = "" Then
        SafeStdOut "ERROR: --agent-pipe requires a pipe name" & vbCrlfW
        Exit Sub
    End If
    
    Dim pipeFullName As WString * 256 = "\\.\pipe\" & pipeName
    
    Do
        ' 创建命名管道实例（阻塞模式，一次处理一个 Agent）
        g_hAgentPipe = CreateNamedPipeW(@pipeFullName, _
            PIPE_ACCESS_DUPLEX, _
            PIPE_TYPE_BYTE Or PIPE_READMODE_BYTE Or PIPE_WAIT, _
            PIPE_UNLIMITED_INSTANCES, _
            65536, 65536, 0, NULL)
            
        If g_hAgentPipe = INVALID_HANDLE_VALUE Then
            SafeStdOut "ERROR: Cannot create pipe " & pipeName & " (GLE=" & GetLastError() & ")" & vbCrlfW
            Exit Sub
        End If
        
        ' 等待 Agent 连接
        If ConnectNamedPipe(g_hAgentPipe, NULL) = 0 Then
            Dim gle As DWORD = GetLastError()
            If gle <> ERROR_PIPE_CONNECTED Then
                CloseHandle(g_hAgentPipe)
                g_hAgentPipe = INVALID_HANDLE_VALUE
                Sleep 100
                Continue Do
            End If
        End If
        
        ' === Agent 已连接 ===
        InitCommandRegistry()
        SafeStdOut("SnowSword Agent v2.0 (Pipe Mode)" & vbCrlfW)
        SafeStdOut("Pipe: " & pipeName & vbCrlfW)
        SafeStdOut("Pipeline: command [| findstr/sort/head ...] [> file] [>> file]" & vbCrlfW)
        
        Dim clientAlive As Boolean = True
        Do While clientAlive
            SafeStdOut("ANTIROOTKIT> ")
            
            Dim strLine As WString * INPUT_BUF_SIZE = ""
            If Not PipeReadLine(@strLine, INPUT_BUF_SIZE) Then
                clientAlive = False
                Continue Do
            End If
            
            Dim cmd As StringW = MyTrimW(strLine, vbCrlfW & " ")
            If LenW(cmd) = 0 Then Continue Do
            
            ' 统一命令处理：全局命令 + 管道/重定向/过滤器
            If Not ProcessCuiLine(cmd) Then
                clientAlive = False
                Continue Do
            End If
        Loop
        
        FlushFileBuffers(g_hAgentPipe)
        DisconnectNamedPipe(g_hAgentPipe)
        CloseHandle(g_hAgentPipe)
        g_hAgentPipe = INVALID_HANDLE_VALUE
        
    Loop
End Sub

' ===================== 单次命令执行 =====================
Function RunSingleCommand(cmdParts() As StringW, numParts As Long) As Integer
    If numParts <= 0 Then
        SafeStdOut("ERROR: Single-shot mode requires command" & vbCrlfW)
        Return 1
    End If
    
    ' 检测命令字符串中是否包含 --format 或 --format=xxx
    For i As Long = 0 To numParts - 1
        Dim part As StringW = LCaseW(cmdParts(i))
        If part = "--format" OrElse LeftW(part, 8) = "--format=" Then
            SafeStdOut("ERROR: '--format' must be specified outside of the -c command string." & vbCrlfW)
            SafeStdOut(!"Usage: snowsword -c <command> --format json" & vbCrlfW)
            Return 1
        End If
    Next

    Dim verb As StringW = LCaseW(cmdParts(0))

    ' ================= 全局命令 =================
    If verb = "help" Then
        If numParts = 1 Then
            PrintGlobalHelp()
            Return 0
        End If

        ' help <category> [action]
        Dim tmp As ParsedCmd
        tmp.category = cmdParts(1)

        If numParts >= 3 Then
            tmp.action = cmdParts(2)

            Dim defHelp As CmdEntry Ptr = FindCmd(tmp.category, tmp.action)

            If defHelp <> NULL Then
                PrintActionHelp(defHelp)
                Return 0
            End If
        End If

        ' help <category>
        PrintCategoryHelp(cmdParts(1))
        Return 0
    ElseIf verb = "load-driver" Then
        If InitDriver Then SafeStdOut("SUCCESS: Driver loaded." & vbCrlfW) Else SafeStdOut("ERROR: Driver load failed." & vbCrlfW)
        Return IIf(IsDriverLoaded, 0, 1)
    ElseIf verb = "unload-driver" Then
        UninitDriver
        SafeStdOut("SUCCESS: Driver unloaded." & vbCrlfW)
        Return 0
    End If
    
    ' 解析命令
    Dim pc As ParsedCmd
    If Not ParseCommandLine(cmdParts(), numParts, @pc) Then
        SafeStdOut(pc.errorMsg & vbCrlfW)
        Return 1
    End If
    
    ' 内联 --help
    If pc.showHelp Then
        Dim def As CmdEntry Ptr = FindCmd(pc.category, pc.action)
        If def <> NULL Then
            PrintActionHelp(def)
        Else
            SafeStdOut("Unknown command. Type 'help' for available commands." & vbCrlfW)
        End If
        Return 0
    End If
    
    ' 查找命令定义
    Dim def As CmdEntry Ptr = FindCmd(pc.category, pc.action)
    If def = NULL Then
        SafeStdOut("ERROR: Unknown command '" & pc.category & " " & pc.action & "'. Type 'help' for available commands." & vbCrlfW)
        Return 1
    End If
    
    ' 按需加载驱动
    Dim driverLoadedByUs As Boolean = False
    If def->needsDriver Then
        If Not IsDriverLoaded Then
            If Not InitDriver() Then
                SafeStdOut("ERROR: Driver load failed." & vbCrlfW)
                Return 1
            End If
            driverLoadedByUs = True
        End If
    End If
    
    ' 执行
    Dim result As Boolean = DispatchCommand(@pc, def)
    
    ' 按需卸载
    If driverLoadedByUs Then
        UninitDriver()
    End If
    
    Return IIf(result, 0, 1)
End Function

' ===================== 单次命令统一入口（-c / --command） =====================
' 1) 引号感知分词（CuiTokenize）
' 2) 含管道/重定向时走统一管线（ProcessCuiLine），并为首段内核命令预装驱动
' 3) 否则走 RunSingleCommand（保持精确退出码与 --format 校验）
Function RunSingleShot(ByRef cmdStr As StringW) As Integer
    Dim cmdParts() As StringW
    Dim cmdPartCount As Long = CuiTokenize(cmdStr, cmdParts())
    If cmdPartCount <= 0 Then
        SafeStdOut("ERROR: Single-shot mode requires command" & vbCrlfW)
        Return 1
    End If

    ' ---- 检测管道/重定向 ----
    Dim stages() As StringW
    Dim stageN As Long = 0
    Dim redirMode As Long = CR_NONE
    Dim redirPath As StringW
    If CuiSplitPipeline(cmdStr, stages(), stageN, redirMode, redirPath) AndAlso _
       CBool(stageN > 1 OrElse redirMode <> CR_NONE) Then

        ' 预装载驱动：首段命令 needsDriver 时自动 load（与 RunSingleCommand 对齐）
        Dim driverLoadedByUs As Boolean = False
        If stageN > 0 Then
            Dim sParts() As StringW
            Dim sn As Long = CuiTokenize(stages(0), sParts())
            If sn >= 2 Then
                Dim tmpPc As ParsedCmd
                If ParseCommandLine(sParts(), sn, @tmpPc) Then
                    Dim pdef As CmdEntry Ptr = FindCmd(tmpPc.category, tmpPc.action)
                    If CBool(pdef <> NULL) AndAlso pdef->needsDriver AndAlso Not IsDriverLoaded Then
                        If Not InitDriver() Then
                            SafeStdOut("ERROR: Driver load failed." & vbCrlfW)
                            Return 1
                        End If
                        driverLoadedByUs = True
                    End If
                End If
            End If
        End If

        ProcessCuiLine(cmdStr)

        If driverLoadedByUs Then UninitDriver()
        Return 0
    End If

    Return RunSingleCommand(cmdParts(), cmdPartCount)
End Function
