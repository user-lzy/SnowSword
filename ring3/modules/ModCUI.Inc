' ===================== CUI Mode Module =====================
Dim Shared g_CuiMode As Boolean = False
Dim Shared g_OutputJson As Boolean = False
#define vbCrlfW WChr(13, 10)
#define CUI_MAX_COLUMNS 32

' 检测 CLI 模式触发
Sub ProceedCommandLine(CommandLine As String)
    If InStr(UCase(CommandLine), "--CLI") > 0 Then g_CuiMode = True
    If InStr(UCase(CommandLine), "--FORMAT JSON") > 0 Then g_OutputJson = True
    
    For i As Integer = 1 To 10
        If Command(i) = "" Then Exit For
        If UCase(Command(i)) = "--CLI" Then g_CuiMode = True
        If UCase(Command(i)) = "--FORMAT" AndAlso UCase(Command(i+1)) = "JSON" Then g_OutputJson = True
    Next
End Sub

' ============================================
' 原有版本完全不变，常量字符串继续走这里
' ============================================
Sub SafeStdOut Overload(ByVal szText As StringW)
    Dim hOut As HANDLE = GetStdHandle(STD_OUTPUT_HANDLE)
    If hOut = INVALID_HANDLE_VALUE Then Exit Sub
    Dim written As DWORD
    WriteConsoleW(hOut, StrPtrW(szText), LenW(szText), @written, NULL)
End Sub

' ============================================
' 新增：直接接受 WString Ptr，无需任何转换
' ============================================
Sub SafeStdOut Overload(ByVal pwText As WString Ptr)
    If pwText = NULL Then Exit Sub
    Dim hOut As HANDLE = GetStdHandle(STD_OUTPUT_HANDLE)
    If hOut = INVALID_HANDLE_VALUE Then Exit Sub
    Dim written As DWORD
    WriteConsoleW(hOut, pwText, lstrlenW(pwText), @written, NULL)
End Sub

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
    If g_OutputJson Then
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
    If g_OutputJson Then
        SafeStdOut("DATA_START" & vbCrlfW)
        SafeStdOut("[" & vbCrlfW)
        For i As Long = 0 To rowCount - 1
            Dim r As LVRow Ptr = snap->Rows[i]
            If r = NULL Then Continue For
            SafeStdOut("  {")
            For c As Long = 0 To r->ColCount - 1
                'Dim colName As StringW = W2S(FrmMain.ListView1.ColumnText(c)) ' 需确保取到列名
                Dim colName As StringW = FrmMain.ListView1.ColumnText(c)
                'SafeStdOut("""" & colName & """:""" & W2S(r->Cols[c]) & """")
                SafeStdOut("""" & colName & """:""" & r->Cols[c] & """")
                If c < r->ColCount - 1 Then SafeStdOut(",")
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
                'SafeStdOut(W2S(r->Cols[c]))
                SafeStdOut(r->Cols[c])
            Next
            SafeStdOut(vbCrlfW)
        Next
        SafeStdOut("DATA_END" & vbCrlfW)
    End If
    
    SafeStdOut("===SNOWSWORD_CUI_END===" & vbCrlfW)
    
    ' 输出完毕后，清空隐藏的ListView，为下一次查询腾出空间
    ' 注意：不能 DestroyWindow，只能清空数据
    DeleteAllItemsEx FrmMain.ListView1
    If ctx->snap Then Deallocate(ctx->snap) : ctx->snap = NULL
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
    If g_OutputJson Then
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

Type CmdEntry
    category      As WString * 64
    action        As WString * 32
    aliases       As WString * 128   ' 逗号分隔中文别名，如 "进程,程序"
    needsDriver   As Boolean
    needsPid      As Boolean
    needsTid      As Boolean
    needsPath     As Boolean
    needsAddr     As Boolean         ' 内存操作专用
    isDestructive As Boolean         ' 【安全】是否需要确认
    helpText      As WString * 512
End Type

Type ParsedCmd
    category   As WString * 64
    action     As WString * 32
    pid        As DWORD
    tid        As DWORD
    path       As WString * MAX_PATH
    addr       As ULONG64
    size       As DWORD
    data       As WString * 256
    regName    As WString * 256
    regType    As WString * 32
    force      As Boolean
    dryRun     As Boolean
    showHelp   As Boolean            ' 【修复P13】内联 --help
    hasError   As Boolean
    errorMsg   As WString * 256
    unknownArg As WString * 64
End Type

Dim Shared g_registry(MAX_REG_ENTRIES) As CmdEntry
Dim Shared g_regCount As Integer = 0

' --------------- 注册表辅助 ---------------
Sub RegisterCmd(ByVal cat As StringW, ByVal act As StringW, ByVal ali As StringW, _
                ByVal drv As Boolean, ByVal pid As Boolean, ByVal tid As Boolean, _
                ByVal pth As Boolean, ByVal adr As Boolean, ByVal dest As Boolean, _
                ByVal hlp As StringW)
    If g_regCount >= MAX_REG_ENTRIES Then Exit Sub
    With g_registry(g_regCount)
        .category = cat
        .action = act
        .aliases = ali
        .needsDriver = drv
        .needsPid = pid
        .needsTid = tid
        .needsPath = pth
        .needsAddr = adr
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

' --------------- 初始化注册表 ---------------
Sub InitCommandRegistry()
    ' ===== 一级查询（无需 PID）=====
    RegisterCmd "process",       "list", "进程,程序",       False, False, False, False, False, False, "List all running processes"
    RegisterCmd "kernel-module", "list", "内核模块",         True,  False, False, False, False, False, "List loaded kernel modules"
    RegisterCmd "kernel-thread", "list", "内核线程",         True,  False, False, False, False, False, "List kernel threads"
    RegisterCmd "gdt",           "read", "",                 True,  False, False, False, False, False, "Read GDT entries"
    RegisterCmd "idt",           "read", "",                 True,  False, False, False, False, False, "Read IDT entries"
    RegisterCmd "ssdt",          "list", "",                 True,  False, False, False, False, False, "List SSDT hooks"
    RegisterCmd "sssdt",          "list", "",                 True,  False, False, False, False, False, "List ShadowSSDT hooks"
    RegisterCmd "callbacks",     "list", "回调",             True,  False, False, False, False, False, "List kernel callbacks"
    RegisterCmd "service",       "list", "服务",             False, False, False, False, False, False, "List system services"
    RegisterCmd "etw-provider",  "list", "ETW提供者",        False, False, False, False, False, False, "List ETW providers"
    RegisterCmd "minifilter",    "list", "",                 True,  False, False, False, False, False, "List minifilter drivers"
    RegisterCmd "filterdriver",  "list", "",                 True,  False, False, False, False, False, "List filter drivers"
    RegisterCmd "hal-dispatch",  "read", "",                 True,  False, False, False, False, False, "Read HAL dispatch table"
    RegisterCmd "hal-private-dispatch", "read", "",          True,  False, False, False, False, False, "Read HAL private dispatch"
    RegisterCmd "object-hook",   "list", "",                 True,  False, False, False, False, False, "List object type hooks"
    RegisterCmd "kernel-timer",  "list", "内核定时器",       True,  False, False, False, False, False, "List kernel timers"
    RegisterCmd "work-item-thread","list","工作队列线程",    True,  False, False, False, False, False, "List work item threads"
    RegisterCmd "wfp-callout",   "list", "",                 True,  False, False, False, False, False, "List WFP callouts"
    RegisterCmd "wfp-filter",    "list", "",                 True,  False, False, False, False, False, "List WFP filters"
    RegisterCmd "winsock-spi",   "list", "",                 False, False, False, False, False, False, "List Winsock SPI entries"
    RegisterCmd "task-scheduler","list", "任务计划",         False, False, False, False, False, False, "List scheduled tasks"
    RegisterCmd "violent-check", "scan", "暴力检测",         True,  False, False, False, False, False, "Perform kernel memory scan"
    
    ' ===== 文件/注册表查询（需要 --path）=====
    RegisterCmd "file",     "list", "文件",   False, False, False, True, False, False, "List files (empty path = drives)"
    RegisterCmd "registry", "read", "注册表", False, False, False, True, False, False, "Read registry (empty path = roots)"
    
    ' ===== 二级查询（需要 --pid）=====
    RegisterCmd "thread",       "list", "线程",     False, True, False, False, False, False, "List threads of a process"
    RegisterCmd "module",       "list", "模块",     False, True, False, False, False, False, "List modules of a process"
    RegisterCmd "windows",      "list", "窗口",     False, True, False, False, False, False, "List windows of a process"
    RegisterCmd "handles",      "list", "句柄",     True,  True, False, False, False, False, "List handles of a process"
    RegisterCmd "memory",       "read", "内存",     False, True, False, False, True,  False, "Read process memory block"
    RegisterCmd "processtimer", "list", "进程定时器", True, True, False, False, False, False, "List timers of a process"
    RegisterCmd "webconnect",   "list", "网络连接", False, True, False, False, False, False, "List network connections"
    RegisterCmd "privilege",    "list", "特权",     False, True, False, False, False, False, "List process privileges"
    RegisterCmd "hook",         "list", "钩子",     False, True, False, False, False, False, "List process hooks"
    RegisterCmd "msghook",      "list", "消息钩子", False, True, False, False, False, False, "List message hooks"
    RegisterCmd "eventhook",    "list", "事件钩子", False, True, False, False, False, False, "List event hooks"
    RegisterCmd "hotkey",       "list", "热键",     False, True, False, False, False, False, "List registered hotkeys"
    
    ' ===== 三级查询（需要 --pid + --tid）=====
    RegisterCmd "thread", "callstack", "调用栈", False, True, True, False, False, False, "Show thread call stack"
    
    ' ===== 编辑/破坏性命令（安全机制） =====
    RegisterCmd "process",  "suspend", "", False, True,  False, False, False, True, "Suspend a process"
    RegisterCmd "process",  "resume",  "", False, True,  False, False, False, True, "Resume a suspended process"
    RegisterCmd "process",  "kill",    "", False, True,  False, False, False, True, "Terminate a process"
    RegisterCmd "process",  "force-kill",    "", False, True,  False, False, False, True, "Force Terminate a process"
    RegisterCmd "thread",   "suspend", "", False, True,  True,  False, False, True, "Suspend a thread"
    RegisterCmd "thread",   "resume",  "", False, True,  True,  False, False, True, "Resume a thread"
    RegisterCmd "thread",   "kill",    "", False, True,  True,  False, False, True, "Terminate a thread"
    RegisterCmd "memory",   "write",   "", False, True,  False, False, True,  True, "Write to process memory"
    RegisterCmd "registry", "write",   "", False, False, False, True,  False, True, "Write registry value"
    RegisterCmd "registry", "delete",  "", False, False, False, True,  False, True, "Delete registry value"
    RegisterCmd "file",     "delete",  "", False, False, False, True,  False, True, "Delete a file"
End Sub

Function ParseCommandLine(parts() As StringW, ByVal numParts As Long, ByVal outResult As ParsedCmd Ptr) As Boolean
    outResult->pid = 0 : outResult->tid = 0 : outResult->addr = 0 : outResult->SIZE = 0
    outResult->force = False : outResult->dryRun = False : outResult->showHelp = False
    outResult->hasError = False : outResult->path = "" : outResult->Data = ""
    outResult->regName = "" : outResult->regType = "REG_SZ"
    
    If numParts < 2 Then
        outResult->hasError = True
        outResult->errorMsg = "Usage: <category> <action> [options]. Example: 'process list' or 'thread list --pid 1234'"
        Return False
    End If
    
    outResult->category = parts(0)
    outResult->action = parts(1)
    
    Dim i As Integer = 2
    Do While i < numParts
        Dim arg As StringW = LCaseW(parts(i))
        
        ' 【修复P13】任意位置的内联 --help
        If arg = "--help" OrElse arg = "-h" OrElse arg = "/?" Then
            outResult->showHelp = True
            i += 1 : Continue Do
        End If
        
        If arg = "--pid" AndAlso (i + 1 < numParts) Then
            outResult->pid = ValULng(parts(i + 1)) : i += 2
        ElseIf arg = "--tid" AndAlso (i + 1 < numParts) Then
            outResult->tid = ValULng(parts(i + 1)) : i += 2
        ElseIf arg = "--path" AndAlso (i + 1 < numParts) Then
            outResult->path = parts(i + 1) : i += 2
        ElseIf arg = "--addr" AndAlso (i + 1 < numParts) Then
            outResult->addr = ParseUInt64(parts(i + 1)) : i += 2
        ElseIf arg = "--size" AndAlso (i + 1 < numParts) Then
            outResult->size = ValULng(parts(i + 1)) : i += 2
        ElseIf arg = "--data" AndAlso (i + 1 < numParts) Then
            outResult->data = parts(i + 1) : i += 2
        ElseIf arg = "--name" AndAlso (i + 1 < numParts) Then
            outResult->regName = parts(i + 1) : i += 2
        ElseIf arg = "--type" AndAlso (i + 1 < numParts) Then
            outResult->regType = parts(i + 1) : i += 2
        ElseIf arg = "--force" OrElse arg = "-f" Then
            outResult->force = True : i += 1
        ElseIf arg = "--dry-run" OrElse arg = "-d" Then
            outResult->dryRun = True : i += 1
        ElseIf LeftW(arg, 1) = "-" Then
            ' 【修复P6/P10】未知参数立即报错，不再静默忽略
            outResult->hasError = True
            outResult->unknownArg = parts(i)
            outResult->errorMsg = "ERROR: Unknown argument '" & parts(i) & "'. Use '<category> <action> --help' for usage."
            Return False
        Else
            ' 【修复P6】风格B不允许裸位置参数
            outResult->hasError = True
            outResult->errorMsg = "ERROR: Unexpected positional argument '" & parts(i) & "'. All arguments must use --key value format."
            Return False
        End If
    Loop
    Return True
End Function

Sub PrintGlobalHelp()
    SafeStdOut("SnowSword Agent Shell v2.0 (Subcommand Style)" & vbCrlfW)
    SafeStdOut("Usage: <category> <action> [options]" & vbCrlfW)
    SafeStdOut("       <category> <action> --help    for action-specific help" & vbCrlfW & vbCrlfW)
    
    SafeStdOut("[System]  (no --pid needed)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).needsPid = False AndAlso g_registry(i).needsPath = False Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Process] (require --pid)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).needsPid AndAlso g_registry(i).needsTid = False Then
            Dim strLine As WString * 512
            strLine = "  " & g_registry(i).category & " " & g_registry(i).action
            If g_registry(i).aliases <> "" Then strLine = strLine & "  |  " & g_registry(i).aliases
            SafeStdOut(strLine & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Thread]  (require --pid --tid)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).needsTid Then
            SafeStdOut("  " & g_registry(i).category & " " & g_registry(i).action & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "[Edit/Destructive]  (require --force for kernel ops)" & vbCrlfW)
    For i As Integer = 0 To g_regCount - 1
        If g_registry(i).isDestructive Then
            SafeStdOut("  " & g_registry(i).category & " " & g_registry(i).action & vbCrlfW)
        End If
    Next
    
    SafeStdOut(vbCrlfW & "Global Options:" & vbCrlfW)
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
    
    SafeStdOut("Required:" & vbCrlfW)
    If def->needsPid  Then SafeStdOut("  --pid <ProcessID>" & vbCrlfW)
    If def->needsTid  Then SafeStdOut("  --tid <ThreadID>" & vbCrlfW)
    If def->needsPath Then SafeStdOut("  --path <Path>" & vbCrlfW)
    If def->needsAddr Then SafeStdOut("  --addr <Address>  (supports 0x prefix)" & vbCrlfW)
    SafeStdOut(vbCrlfW & "Optional:" & vbCrlfW)
    SafeStdOut("  --force         Skip confirmation" & vbCrlfW)
    SafeStdOut("  --dry-run       Preview only" & vbCrlfW)
    SafeStdOut("  --help          Show this help" & vbCrlfW)
End Sub

Function ConfirmDestructive(ByVal pc As ParsedCmd Ptr, ByVal def As CmdEntry Ptr) As Boolean
    If pc->dryRun Then
        SafeStdOut("[DRY-RUN] Operation preview:" & vbCrlfW)
        SafeStdOut("  Category : " & pc->category & vbCrlfW)
        SafeStdOut("  Action   : " & pc->action & vbCrlfW)
        If pc->pid <> 0    Then SafeStdOut("  PID      : " & pc->pid & vbCrlfW)
        If pc->tid <> 0    Then SafeStdOut("  TID      : " & pc->tid & vbCrlfW)
        If pc->path <> ""  Then SafeStdOut("  Path     : " & pc->path & vbCrlfW)
        If pc->addr <> 0   Then SafeStdOut("  Address  : 0x" & Hex(pc->addr) & vbCrlfW)
        If pc->size <> 0   Then SafeStdOut("  Size     : " & pc->size & vbCrlfW)
        If pc->data <> ""  Then SafeStdOut("  Data     : " & pc->data & vbCrlfW)
        If pc->regName <> "" Then SafeStdOut("  RegName  : " & pc->regName & vbCrlfW)
        SafeStdOut("No changes made (dry-run)." & vbCrlfW)
        Return False  ' dry-run 不进入真实执行
    End If
    
    If def->isDestructive = False Then Return True
    
    ' 内核级破坏性操作：强制 --force，禁止交互
    If def->needsDriver AndAlso pc->force = False Then
        SafeStdOut("ERROR: Kernel destructive operations require --force. Interactive confirm is disabled for safety." & vbCrlfW)
        Return False
    End If
    
    If pc->force Then Return True  ' 用户已显式承担风险
    
    ' 用户层交互确认
    Dim msg As WString * 512
    msg = "WARNING: Destructive operation '" & pc->category & " " & pc->action & "'."
    If pc->pid <> 0 Then msg = msg & " PID=" & pc->pid
    If pc->tid <> 0 Then msg = msg & " TID=" & pc->tid
    msg = msg & vbCrlfW & "Confirm? [y/N]: "
    SafeStdOut(msg)
    
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
    CurrentInformation.intType = Thread : CurrentInformation.ProcessId = pc->pid
    InitializeListView Thread, FrmMain.ListView1
    GetThreadList pc->pid, FrmMain.ListView1
    CuiPrintListViewData()
    Return True
End Function

Function Exec_Memory_Read(ByVal pc As ParsedCmd Ptr) As Boolean
    CurrentInformation.intType = Memory : CurrentInformation.ProcessId = pc->pid
    InitializeListView Memory, FrmMain.ListView1
    GetMemoryBlockList pc->pid, FrmMain.ListView1
    CuiPrintListViewData()
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
        GetDriveList FrmMain.TreeView
    Else
        GetFileList_CLI pc->path
    End If
    Return True
End Function

Function Exec_Registry_Read(ByVal pc As ParsedCmd Ptr) As Boolean
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
    If MyResumeThread(pc->pid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Thread_Suspend(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: suspend thread " & pc->tid & ",pid=" & pc->pid & vbCrlfW)
    If MySuspendThread(pc->pid) Then
        SafeStdOut("Success!" & vbCrlfW)
        Return True
    Else
        SafeStdOut("Failed!" & vbCrlfW)
        Return False
    End If
End Function

Function Exec_Memory_Write(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: Write memory PID=" & pc->pid & " Addr=0x" & Hex(pc->addr) & " Size=" & pc->size & vbCrlfW)
    ' TODO: 调用 WriteProcessMemory 或驱动接口
    Return True
End Function

Function Exec_Registry_Write(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: Write registry " & pc->path & " -> " & pc->regName & " = " & pc->data & " (" & pc->regType & ")" & vbCrlfW)
    ' TODO: 调用 RegSetValueEx 或驱动接口
    Return True
End Function

Function Exec_Registry_Delete(ByVal pc As ParsedCmd Ptr) As Boolean
    SafeStdOut("Executing: Delete registry value " & pc->path & " -> " & pc->regName & vbCrlfW)
    ' TODO: 调用 RegDeleteValue 或驱动接口
    Return True
End Function

' --------------- 统一分发器 ---------------
Function DispatchCommand(ByVal pc As ParsedCmd Ptr, ByVal def As CmdEntry Ptr) As Boolean
    ' 必填参数校验
    If def->needsPid Then ' AndAlso pc->pid = 0
        SafeStdOut("ERROR: --pid is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
        Return False
    End If
    If def->needsTid AndAlso pc->tid = 0 Then
        SafeStdOut("ERROR: --tid is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
        Return False
    End If
    If def->needsPath AndAlso pc->path = "" Then
        SafeStdOut("ERROR: --path is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
        Return False
    End If
    If def->needsAddr AndAlso pc->addr = 0 Then
        SafeStdOut("ERROR: --addr is required for '" & def->category & " " & def->action & "'." & vbCrlfW)
        Return False
    End If
    
    ' 驱动检查
    If def->needsDriver AndAlso Not IsDriverLoaded Then
        SafeStdOut("ERROR: Driver not loaded. Use 'load-driver' first." & vbCrlfW)
        Return False
    End If
    
    ' 安全确认
    If Not ConfirmDestructive(pc, def) Then Return False
    
    ' 路由到具体实现（按 category + action 组合键）【修复P8】
    Dim key As WString * MAX_PATH = LCaseW(def->category) & "|" & LCaseW(def->action)
    Select Case key
        Case "process|list"       : Return Exec_Process_List(pc)
        Case "kernel-module|list" : Return Exec_KernelModule_List(pc)
        Case "kernel-thread|list" : CurrentInformation.intType = KernelThread : InitializeListView KernelThread, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW) : GetKernelThreadList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "ssdt|list"          : CurrentInformation.intType = SSDT : InitializeListView SSDT, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetSSDT FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "sssdt|list"          : CurrentInformation.intType = ShadowSSDT : InitializeListView ShadowSSDT, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetSSSDT FrmMain.ListView1 : CuiPrintListViewData() : Return True
            
        Case "hal-dispatch|read"  : CurrentInformation.intType = HalDispatch : InitializeListView HalDispatch, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHalDispatchTable FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hal-private-dispatch|read": CurrentInformation.intType = HalPrivateDispatch : InitializeListView HalPrivateDispatch, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHalPrivateDispatchTable FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "object-hook|list"   : CurrentInformation.intType = ObjectHook : InitializeListView ObjectHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetObjectInfo FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "callbacks|list"     : CurrentInformation.intType = Callbacks : InitializeListView Callbacks, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetCallbackList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "kernel-timer|list"  : CurrentInformation.intType = KernelTimer : InitializeListView KernelTimer, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetKernelTimerList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "work-item-thread|list": CurrentInformation.intType = WorkItemThread : InitializeListView WorkItemThread, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWorkItemThreadList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "wfp-callout|list"   : CurrentInformation.intType = WfpCallout : InitializeListView WfpCallout, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWfpCalloutList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "wfp-filter|list"    : CurrentInformation.intType = WfpFilter : InitializeListView WfpFilter, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetWfpFilterList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "service|list"       : CurrentInformation.intType = Service : InitializeListView Service, FrmMain.ListView1 : GetServiceList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        
        Case "etw-provider|list"  : CurrentInformation.intType = EtwProvider : InitializeListView EtwProvider, FrmMain.ListView1 : GetETWProviderList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "winsock-spi|list"   : CurrentInformation.intType = WinsockSPI : InitializeListView WinsockSPI, FrmMain.ListView1 : GetWinsockSPIList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "task-scheduler|list": CurrentInformation.intType = TaskScheduler : InitializeListView TaskScheduler, FrmMain.ListView1 : GetTaskSchedulerList FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "violent-check|scan" : CurrentInformation.intType = ViolentCheck : InitializeListView ViolentCheck, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else ScanKernelMemoryMultiThread FrmMain.ListView1 : CuiPrintListViewData() : Return True
        
        ' GDT / IDT / Minifilter / FilterDriver 等 TreeList 模块保持原样
        Case "gdt|read"           : CurrentInformation.intType = GDT : InitializeTreeList GDT, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetGDT FrmMain.mCtrlTreeList1 : Return True
        Case "idt|read"           : CurrentInformation.intType = IDT : InitializeTreeList IDT, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetIDT FrmMain.mCtrlTreeList1 : Return True
        Case "minifilter|list"    : CurrentInformation.intType = Minifilter : InitializeTreeList Minifilter, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else EnumMinifilter FrmMain.mCtrlTreeList1 : Return True
        Case "filterdriver|list"  : CurrentInformation.intType = FilterDriver : InitializeTreeList FilterDriver, FrmMain.mCtrlTreeList1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else EnumAttachDevices FrmMain.mCtrlTreeList1 : Return True

        ' 二级查询
        Case "thread|list"        : Return Exec_Thread_List(pc)
        Case "module|list"        : CurrentInformation.intType = Module : CurrentInformation.ProcessId = pc->pid : InitializeListView Module, FrmMain.ListView1 : GetModuleList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "windows|list"       : CurrentInformation.intType = Windows : CurrentInformation.ProcessId = pc->pid : InitializeListView Windows, FrmMain.ListView1 : GetWindowList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "handles|list"       : CurrentInformation.intType = Handles : CurrentInformation.ProcessId = pc->pid : InitializeListView Handles, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("notice: driver hasn't been loaded." & vbCrlfW) : GetHandleList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "memory|read"        : Return Exec_Memory_Read(pc)
        Case "processtimer|list"  : CurrentInformation.intType = ProcessTimer : CurrentInformation.ProcessId = pc->pid : InitializeListView ProcessTimer, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetProcessTimerList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "webconnect|list"    : CurrentInformation.intType = WebConnect : CurrentInformation.ProcessId = pc->pid : InitializeListView WebConnect, FrmMain.ListView1 : GetWebConnectList pc->pid, FrmMain.ListView1, True : CuiPrintListViewData() : Return True
        Case "privilege|list"     : CurrentInformation.intType = Privilege : CurrentInformation.ProcessId = pc->pid : InitializeListView Privilege, FrmMain.ListView1 : GetProcessPrivilegeList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hook|list"          : CurrentInformation.intType = Hook : CurrentInformation.ProcessId = pc->pid : InitializeListView Hook, FrmMain.ListView1 : GetProcessHookList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "msghook|list"       : CurrentInformation.intType = MsgHook : CurrentInformation.ProcessId = pc->pid : InitializeListView MsgHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetMsgHookList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "eventhook|list"     : CurrentInformation.intType = EventHook : CurrentInformation.ProcessId = pc->pid : InitializeListView EventHook, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetEventHookList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        Case "hotkey|list"        : CurrentInformation.intType = Hotkey : CurrentInformation.ProcessId = pc->pid : InitializeListView Hotkey, FrmMain.ListView1 : If Not IsDriverLoaded Then SafeStdOut("warning: driver hasn't been loaded." & vbCrlfW) : Return False Else GetHotkeyList pc->pid, FrmMain.ListView1 : CuiPrintListViewData() : Return True
        
        ' 三级查询
        Case "thread|callstack"   : Return Exec_Thread_CallStack(pc)
        
        ' 文件/注册表查询
        Case "file|list"          : Return Exec_File_List(pc)
        Case "registry|read"      : Return Exec_Registry_Read(pc)
        
        ' 编辑/破坏性命令
        Case "process|suspend"   : Return Exec_Process_Suspend(pc)
        Case "process|resume"     : Return Exec_Process_Resume(pc)
        Case "process|kill"            : Return Exec_Process_Kill(pc)
        Case "process|force-kill"  : Return Exec_Process_ForceKill(pc)
            
        Case "thread|suspend"     : Return Exec_Thread_Suspend(pc)
        Case "thread|resume"      : Return Exec_Thread_Resume(pc)
        Case "thread|kill"             : Return Exec_Thread_Kill(pc)

        Case "memory|write"       : Return Exec_Memory_Write(pc)
        Case "registry|write"        : Return Exec_Registry_Write(pc)
        Case "registry|delete"      : Return Exec_Registry_Delete(pc)
        Case "file|delete"             : SafeStdOut("Executing: Delete file " & pc->path & vbCrlfW) : Return True
        
        Case Else
            SafeStdOut("ERROR: Command '" & key & "' registered but handler not implemented." & vbCrlfW)
            Return False
    End Select
End Function

' ===================== 主 REPL 循环（重构版） =====================
Sub RunAgentRepl()
    Dim hIn As HANDLE = GetStdHandle(STD_INPUT_HANDLE)
    Dim inputBuf As WString * INPUT_BUF_SIZE  ' 【修复P7】4096字符
    Dim bytesRead As DWORD
    
    InitCommandRegistry()  ' 【修复P9】启动时统一注册
    
    SafeStdOut("SnowSword Agent Shell v2.0" & vbCrlfW)
    SafeStdOut("Format: <category> <action> [options]" & vbCrlfW)
    SafeStdOut("Example: process list | thread list --pid 1234 | process kill --pid 1234 --force" & vbCrlfW)
    SafeStdOut("Type 'help' or '<category> <action> --help' for assistance." & vbCrlfW & vbCrlfW)
    
    Do
        SafeStdOut("ANTIROOTKIT> ")
        
        If ReadConsoleW(hIn, @inputBuf, INPUT_BUF_SIZE - 1, @bytesRead, NULL) = 0 Then Exit Do
        If bytesRead = 0 Then Exit Do
        
        inputBuf[bytesRead] = 0
        Dim cmd As StringW = MyTrimW(inputBuf, vbCrlfW & " ")
        If LenW(cmd) = 0 Then Continue Do
        
        Dim cmdParts() As StringW
        Dim numParts As Long = vbSplitW(cmd, " ", cmdParts())
        If numParts = 0 Then Continue Do
        
        Dim verb As StringW = LCaseW(cmdParts(0))
        
        ' ---------------- 全局命令 ----------------
        If verb = "exit" Then
            UninitDriver
            Exit Do
            
        ElseIf verb = "load-driver" Then
            If InitDriver Then SafeStdOut("SUCCESS: Driver loaded." & vbCrlfW) Else SafeStdOut("ERROR: Driver load failed." & vbCrlfW)
            Continue Do
            
        ElseIf verb = "help" Then
            If numParts = 1 Then
                PrintGlobalHelp()
            ElseIf numParts >= 2 Then
                ' help <category> [action]
                Dim tmp As ParsedCmd
                tmp.category = cmdParts(1)
                If numParts >= 3 Then tmp.action = cmdParts(2) Else tmp.action = ""
                Dim def As CmdEntry Ptr = FindCmd(tmp.category, tmp.action)
                If def <> NULL AndAlso numParts >= 3 Then
                    PrintActionHelp(def)
                ElseIf def <> NULL AndAlso numParts = 2 Then
                    ' 如果只给 category，打印该类别下所有 action
                    PrintCategoryHelp(cmdParts(1))
                Else
                    PrintCategoryHelp(cmdParts(1))
                End If
            End If
            Continue Do
        End If
        
        ' ---------------- 子命令解析 ----------------
        Dim pc As ParsedCmd
        If Not ParseCommandLine(cmdParts(), numParts, @pc) Then
            SafeStdOut(pc.errorMsg & vbCrlfW)
            Continue Do
        End If
        
        ' 【修复P13】内联 --help：process list --help
        If pc.showHelp Then
            Dim def As CmdEntry Ptr = FindCmd(pc.category, pc.action)
            If def <> NULL Then
                PrintActionHelp(def)
            Else
                SafeStdOut("Unknown command. Type 'help' for available commands." & vbCrlfW)
            End If
            Continue Do
        End If
        
        ' 查找命令定义
        Dim def As CmdEntry Ptr = FindCmd(pc.category, pc.action)
        If def = NULL Then
            SafeStdOut("ERROR: Unknown command '" & pc.category & " " & pc.action & "'. Type 'help' for available commands." & vbCrlfW)
            Continue Do
        End If
        
        ' 执行
        DispatchCommand(@pc, def)
        
    Loop
    
    End
End Sub
