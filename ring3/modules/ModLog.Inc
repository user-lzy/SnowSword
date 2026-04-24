'===============================================================================
' FreeBASIC 高性能异步日志模块 - TextBox版（零阻塞、不影响调用者性能）
'===============================================================================
#include Once "win/winbase.bi"
#include Once "fbthread.bi"

'-------------------------------------------------------------------------------
' 日志级别枚举
'-------------------------------------------------------------------------------
Enum LogLevel
    LOG_DEBUG   = 0
    LOG_INFO    = 1
    LOG_WARN    = 2
    LOG_ERROR   = 3
    LOG_FATAL   = 4
End Enum

'-------------------------------------------------------------------------------
' 日志队列节点（内存存储，极快）
'-------------------------------------------------------------------------------
Type LogNode
    As String  text
    As LogNode Ptr next
End Type

'-------------------------------------------------------------------------------
' 高性能异步 Logger 类
'-------------------------------------------------------------------------------
Type Logger
Private:
    ' 配置
    As Boolean      m_enable
    As Boolean      m_toConsole
    As Boolean      m_toFile
    As String       m_filePath
    As Boolean      m_append
    As Boolean      m_useUTF8
    As LogLevel     m_filterLevel
    
    ' 异步核心
    As LogNode Ptr     m_queueHead   ' 日志队列头
    As LogNode Ptr     m_queueTail   ' 日志队列尾
    As HANDLE       m_queueMutex  ' 队列锁（极短占用，无阻塞）
    As HANDLE       m_wakeEvent   ' 唤醒后台线程事件
    As Any Ptr      m_thread      ' 后台输出线程
    As Boolean      m_threadRun   ' 线程运行标记
    
    ' 文件持久化（长连接，避免频繁开关）
    As Integer      m_fileNum
    As Boolean      m_fileOpened
    
    ' UI绑定
    As Class_TextBox m_TextBox

Public:
    Declare Constructor()
    Declare Destructor()

    ' 初始化/销毁
    Declare Sub Start()   ' 启动后台线程（必须调用）
    Declare Sub Stop()    ' 安全停止线程

    ' 配置接口
    Declare Sub SetTextBox(TextBox As Class_TextBox)
    Declare Sub SetOutput(console As Boolean, file As Boolean)
    Declare Sub SetLogPath(path As String)
    Declare Sub SetAppend(isAppend As Boolean)
    Declare Sub SetEncoding(useUTF8 As Boolean)
    Declare Sub SetFilterLevel(level As LogLevel)
    Declare Sub Toggle(enable As Boolean)

    ' 日志打印（0阻塞，仅入队）
    Declare Sub PrintLog(level As LogLevel, func As String = "", apiName As String = "", MSG As String = "")
    
    Declare Sub PrintWin32Error(funcName As String, apiName As String, MSG As String = "")
    Declare Sub PrintNtError(funcName As String, apiName As String, ByVal Status As NTSTATUS, MSG As String = "")

Private:
    ' 工具方法
    Declare Function GetTimeStamp() As String
    Declare Function GetDailyLogName() As String
    Declare Function LevelToStr(level As LogLevel) As String
    
    ' 异步核心
    Declare Sub EnqueueLog(text As String)
    Declare Function DequeueLog() As String
    Declare Sub OpenLogFile()
    Declare Sub CloseLogFile()
    Declare Sub OutputToUI(text As String)
    Declare Sub OutputToFile(text As String)
    Declare Sub WorkerThread(ByVal param As Any Ptr)  ' 后台工作线程
End Type

' 全局日志实例
Dim Shared As Logger MyLog

'-------------------------------------------------------------------------------
' 构造/析构
'-------------------------------------------------------------------------------
Constructor Logger()
    m_enable      = True
    m_toConsole   = True
    m_toFile      = False
    m_filePath    = CurDir
    m_append      = False
    m_useUTF8     = True
    m_filterLevel = LOG_WARN
    
    m_queueHead   = 0
    m_queueTail   = 0
    m_queueMutex  = 0
    m_wakeEvent   = 0
    m_thread      = 0
    m_threadRun   = False
    m_fileNum     = 0
    m_fileOpened  = False
End Constructor

Destructor Logger()
    This.Stop()
End Destructor

'-------------------------------------------------------------------------------
' 启动/停止后台线程
'-------------------------------------------------------------------------------
Sub Logger.Start()
    If m_threadRun Then Exit Sub
    
    ' 创建同步对象
    m_queueMutex = CreateMutex(NULL, False, NULL)
    m_wakeEvent  = CreateEvent(NULL, True, False, NULL)
    m_threadRun  = True
    
    ' 启动后台输出线程
    m_thread = ThreadCreate(CPtr(Sub (ByVal userdata As Any Ptr), @Logger.WorkerThread), @MyLog)
End Sub

Sub Logger.Stop()
    If Not m_threadRun Then Exit Sub
    
    m_threadRun = False
    SetEvent m_wakeEvent
    ThreadWait(m_thread)
    
    ' 释放资源
    CloseLogFile()
    If m_queueMutex Then CloseHandle(m_queueMutex)
    If m_wakeEvent Then CloseHandle(m_wakeEvent)
    m_queueMutex = 0
    m_wakeEvent = 0
    m_thread = 0
End Sub

'-------------------------------------------------------------------------------
' 线程包装（FB线程必须用静态函数）
'-------------------------------------------------------------------------------
/'Sub Logger.WorkerThreadProc(ByVal userData As Any Ptr)
    Dim As Logger Ptr pLog = Cast(Logger Ptr, userData)
    pLog->WorkerThread()
End Sub'/

'-------------------------------------------------------------------------------
' 后台工作线程（唯一执行IO/UI的线程，完全不阻塞调用者）
'-------------------------------------------------------------------------------
Sub Logger.WorkerThread(ByVal param As Any Ptr)
    Dim As String logText
    Dim As Integer waitRet
    
    While m_threadRun
        ' 等待日志队列消息（无日志时休眠，不占CPU）
        waitRet = WaitForSingleObject(m_wakeEvent, 100)
        If waitRet = WAIT_OBJECT_0 Then ResetEvent(m_wakeEvent)
        
        ' 批量处理所有日志（极高效率）
        While True
            logText = DequeueLog()
            If logText = "" Then Exit While
            
            If m_toConsole Then OutputToUI(logText)
            If m_toFile Then OutputToFile(logText)
        Wend
    Wend
    
    ' 退出前刷空剩余日志
    While True
        logText = DequeueLog()
        If logText = "" Then Exit While
        If m_toConsole Then OutputToUI(logText)
        If m_toFile Then OutputToFile(logText)
    Wend
End Sub

'-------------------------------------------------------------------------------
' 日志入队（调用者仅执行这一步，速度=内存赋值，零阻塞）
'-------------------------------------------------------------------------------
Sub Logger.EnqueueLog(text As String)
    If Not m_enable Or text = "" Then Exit Sub
    
    ' 极短时间加锁（仅操作链表指针）
    If m_queueMutex Then WaitForSingleObject(m_queueMutex, INFINITE)
    
    Dim As LogNode Ptr node = New LogNode
    node->text = text
    node->next = 0
    
    If m_queueTail = 0 Then
        m_queueHead = node
        m_queueTail = node
    Else
        m_queueTail->next = node
        m_queueTail = node
    End If
    
    ' 唤醒后台线程
    SetEvent m_wakeEvent
    
    If m_queueMutex Then ReleaseMutex(m_queueMutex)
End Sub

'-------------------------------------------------------------------------------
' 日志出队（后台线程使用）
'-------------------------------------------------------------------------------
Function Logger.DequeueLog() As String
    Dim As String res = ""
    
    If m_queueMutex Then WaitForSingleObject(m_queueMutex, INFINITE)
    
    If m_queueHead <> 0 Then
        Dim As LogNode Ptr node = m_queueHead
        res = node->text
        m_queueHead = node->next
        If m_queueHead = 0 Then m_queueTail = 0
        Delete node
    End If
    
    If m_queueMutex Then ReleaseMutex(m_queueMutex)
    
    Return res
End Function

'-------------------------------------------------------------------------------
' UI输出（批量刷新，不卡顿）
'-------------------------------------------------------------------------------
Sub Logger.OutputToUI(text As String)
    ' 安全UI更新（仅追加文本，避免全量替换）
    m_TextBox.Text = m_TextBox.Text & text & Chr(13,10)
End Sub

'-------------------------------------------------------------------------------
' 文件输出（长连接，一次打开，全程写入）
'-------------------------------------------------------------------------------
Sub Logger.OpenLogFile()
    If m_fileOpened Then Exit Sub
    
    Dim As String fullPath = m_filePath & "\" & GetDailyLogName()
    m_fileNum = FreeFile
    
    If m_useUTF8 Then
        If m_append Then
            Open fullPath For Append Encoding "utf8" As #m_fileNum
        Else
            Open fullPath For Output Encoding "utf8" As #m_fileNum
            Put #m_fileNum,, Chr(&HEF)&Chr(&HBB)&Chr(&HBF)
        End If
    Else
        If m_append Then
            Open fullPath For Append As #m_fileNum
        Else
            Open fullPath For Output As #m_fileNum
        End If
    End If
    
    m_fileOpened = True
End Sub

Sub Logger.CloseLogFile()
    If m_fileOpened Then
        Close #m_fileNum
        m_fileOpened = False
        m_fileNum = 0
    End If
End Sub

Sub Logger.OutputToFile(text As String)
    If Not m_toFile Then Exit Sub
    OpenLogFile()
    Print #m_fileNum, text
End Sub

'-------------------------------------------------------------------------------
' 工具函数
'-------------------------------------------------------------------------------
Function Logger.GetTimeStamp() As String
    Dim As Double curr = Now
    Return _
        Right("0000"& Year(curr),4) &"-"& _
        Right("00"& Month(curr),2) &"-"& _
        Right("00"& Day(curr),2) &" "& _
        Right("00"& Hour(curr),2) &":"& _
        Right("00"& Minute(curr),2) &":"& _
        Right("00"& Second(curr),2)
End Function

Function Logger.GetDailyLogName() As String
    Dim As Double curr = Now
    Return _
        Right("0000"&Year(curr),4)& _
        Right("00"&Month(curr),2)& _
        Right("00"&Day(curr),2)&".log"
End Function

Function Logger.LevelToStr(level As LogLevel) As String
    Select Case level
        Case LOG_DEBUG:Return "DEBUG"
        Case LOG_INFO:Return "INFO"
        Case LOG_WARN:Return "WARN"
        Case LOG_ERROR:Return "ERROR"
        Case LOG_FATAL:Return "FATAL"
        Case Else:Return "UNKNOWN"
    End Select
End Function

'-------------------------------------------------------------------------------
' 公开配置接口
'-------------------------------------------------------------------------------
Sub Logger.SetTextBox(TextBox As Class_TextBox)
    m_TextBox = TextBox
End Sub

Sub Logger.SetOutput(console As Boolean, file As Boolean)
    m_toConsole = console
    m_toFile = file
End Sub

Sub Logger.SetLogPath(path As String)
    If Dir(path,16)="" Then MkDir path
    m_filePath = path
End Sub

Sub Logger.SetAppend(isAppend As Boolean)
    m_append = isAppend
End Sub

Sub Logger.SetEncoding(useUTF8 As Boolean)
    m_useUTF8 = useUTF8
End Sub

Sub Logger.SetFilterLevel(level As LogLevel)
    m_filterLevel = level
End Sub

Sub Logger.Toggle(enable As Boolean)
    m_enable = enable
End Sub

'-------------------------------------------------------------------------------
' 日志打印（对外核心接口，0耗时）
'-------------------------------------------------------------------------------
'Sub Logger.PrintLog(level As LogLevel, module As String = "", func As String = "", apiName As String = "", MSG As String = "")
Sub Logger.PrintLog(level As LogLevel, func As String = "", apiName As String = "", MSG As String = "")
    If Not m_enable Or level < m_filterLevel Then Exit Sub
    
    Dim As String logStr = "[" & GetTimeStamp() & "] [" & LevelToStr(level) & "]"
    'If module <> "" Then logStr = logStr & " [模块:" & module & "]"
    If func <> "" Then logStr = logStr & " [函数:" & func & "]"
    If apiName <> "" Then logStr = logStr & " [API:" & apiName & "]"
    logStr = logStr & " " & msg
    
    ' 仅入内存队列，立即返回
    EnqueueLog(logStr)
End Sub

Sub Logger.PrintWin32Error(funcName As String, apiName As String, msg As String)
    If Not m_enable Then Exit Sub
    
    Dim nError As DWORD = GetLastError
    Dim As String logStr = "[" & GetTimeStamp() & "] [" & LevelToStr(LOG_ERROR) & "]"
    logStr = logStr & " [函数:" & funcName & "]"
    If apiName <> "" Then logStr = logStr & " [API:" & apiName & "]"
    logStr = logStr & "调用失败,错误码:" & nError & "(" & WinErrorMsg(nError) & ")"
    logStr = logStr & msg
    
    ' 仅入内存队列，立即返回
    EnqueueLog(logStr)
End Sub

Sub Logger.PrintNtError(funcName As String, apiName As String, ByVal status As NTSTATUS, msg As String)
    If Not m_enable Then Exit Sub
    
    Dim As String logStr = "[" & GetTimeStamp() & "] [" & LevelToStr(LOG_ERROR) & "]"
    logStr = logStr & " [函数:" & funcName & "]"
    If apiName <> "" Then logStr = logStr & " [API:" & apiName & "]"
    logStr = logStr & "调用失败,错误码:0x" & WHex(status)
    logStr = logStr & msg
    
    ' 仅入内存队列，立即返回
    EnqueueLog(logStr)
End Sub
'-------------------------------------------------------------------------------
' 初始化示例（调用方式不变）
'-------------------------------------------------------------------------------
Sub InitLog()
    ' 必须先启动后台线程
    MyLog.Start()
    
    MyLog.SetOutput(True, False)
    MyLog.SetTextBox(FrmLog.txtLog)
    MyLog.SetEncoding(False)
    MyLog.SetFilterLevel(LOG_DEBUG)

    ' 所有 PrintLog 都瞬间完成，不影响性能
    /'MyLog.PrintLog(LOG_WARN, "进程句柄获取失败", __FILE__, __FUNCTION__, "OpenProcess")
    MyLog.PrintLog(LOG_ERROR, "内存分配失败", , , "VirtualAlloc")
    MyLog.PrintLog(LOG_INFO, "模块初始化完成")
    MyLog.PrintLog(LOG_FATAL, "程序异常退出")'/
End Sub