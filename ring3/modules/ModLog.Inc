'===============================================================================
' FreeBASIC 面向对象日志模块 - TextBox输出增强版
' 基于 Type 类实现，无全局变量，强封装，线程安全，灵活配置
' 默认配置：总开关开启 | TextBox输出 | 覆盖日志 | UTF-8 | 过滤级别≥WARN
'===============================================================================
#include Once "win/winbase.bi"
'-------------------------------------------------------------------------------
' 日志级别枚举（全局，方便外部调用）
'-------------------------------------------------------------------------------
Enum LogLevel
    LOG_DEBUG   = 0
    LOG_INFO    = 1
    LOG_WARN    = 2
    LOG_ERROR   = 3
    LOG_FATAL   = 4
End Enum

'-------------------------------------------------------------------------------
' 日志核心类（强封装，私有成员 + 公有接口）
'-------------------------------------------------------------------------------
Type Logger
    '==================== 私有成员变量（外部不可访问）====================
    Private:
        As Boolean      m_enable        ' 日志总开关
        As Boolean      m_toConsole     ' 输出到TextBox（兼容原变量名）
        As Boolean      m_toFile        ' 输出到文件
        As String       m_filePath      ' 日志保存目录
        As Boolean      m_append        ' 追加模式
        As Boolean      m_useUTF8       ' 编码：UTF-8/ANSI
        As LogLevel     m_filterLevel   ' 全局过滤级别
        As HANDLE       m_mutex         ' 线程互斥锁
        As Class_TextBox         m_TextBox      ' 【新增】TextBox

    '==================== 私有工具方法（内部调用）====================
    Declare Function GetTimeStamp() As String
    Declare Function GetDailyLogName() As String
    Declare Function LevelToStr(level As LogLevel) As String
    Declare Sub OutputCore(logText As String)

    '==================== 公有接口（外部可调用）====================
    Public:
        ' 构造函数：初始化默认配置
        Declare Constructor()
        ' 析构函数：自动释放线程锁资源
        Declare Destructor()

        ' 初始化线程安全锁
        Declare Sub InitMutex()
        ' 手动释放线程锁
        Declare Sub UninitMutex()

        ' 【新增】绑定TextBox句柄（核心：替换控制台输出）
        Declare Sub SetTextBox(TextBox As Class_TextBox)
        ' 设置输出目标：TextBox/文件
        Declare Sub SetOutput(console As Boolean, file As Boolean)
        ' 设置日志保存目录
        Declare Sub SetLogPath(path As String)
        ' 设置日志追加模式（修复关键字冲突：append→isAppend）
        Declare Sub SetAppend(isAppend As Boolean)
        ' 设置编码
        Declare Sub SetEncoding(useUTF8 As Boolean)
        ' 设置全局过滤级别
        Declare Sub SetFilterLevel(level As LogLevel)
        ' 日志总开关
        Declare Sub Toggle(enable As Boolean)

        ' 主日志打印方法
        Declare Sub PrintLog( _
            level As LogLevel, _
            msg As String, _
            module As String = "", _
            func As String = "", _
            apiName As String = "" _
        )
End Type

'-------------------------------------------------------------------------------
' 类方法实现
'-------------------------------------------------------------------------------
' 构造函数：默认配置初始化
Constructor Logger()
    m_enable      = TRUE
    m_toConsole   = TRUE   ' 代表启用TextBox输出
    m_toFile      = FALSE
    m_filePath    = CurDir
    m_append      = FALSE
    m_useUTF8     = TRUE
    m_filterLevel = LOG_WARN
    m_mutex       = NULL
    'm_hTextBox    = NULL   ' 初始化TextBox句柄为空
End Constructor

' 析构函数：自动清理线程锁
Destructor Logger()
    This.UninitMutex()
End Destructor

' 【新增】绑定TextBox窗口句柄
Sub Logger.SetTextBox(TextBox As Class_TextBox)
    m_TextBox = TextBox
End Sub

' 修复：标准时间戳，永不出错，兼容所有FB版本
Function Logger.GetTimeStamp() As String
    Dim As Double curr = Now  ' Now返回标准日期数值，完美支持Year/Month/Day
    Dim As String y = Right("0000" & Year(curr), 4)
    Dim As String m = Right("00" & Month(curr), 2)
    Dim As String d = Right("00" & Day(curr), 2)
    Dim As String h = Right("00" & Hour(curr), 2)
    Dim As String mi = Right("00" & Minute(curr), 2)
    Dim As String s = Right("00" & Second(curr), 2)
    Return y & "-" & m & "-" & d & " " & h & ":" & mi & ":" & s
End Function

Function Logger.GetDailyLogName() As String
    Dim As Double curr = Now
    Return Right("0000" & Year(curr), 4) & Right("00" & Month(curr), 2) & Right("00" & Day(curr), 2) & ".log"
End Function

' 日志级别转字符串
Function Logger.LevelToStr(level As LogLevel) As String
    Select Case level
        Case LOG_DEBUG : Return "DEBUG"
        Case LOG_INFO  : Return "INFO"
        Case LOG_WARN  : Return "WARN"
        Case LOG_ERROR : Return "ERROR"
        Case LOG_FATAL : Return "FATAL"
        Case Else      : Return "UNKNOWN"
    End Select
End Function

' 【核心抽象层】日志输出收口（控制台→TextBox替换）
Sub Logger.OutputCore(logText As String)
    ' 线程安全加锁
    If m_mutex <> NULL Then WaitForSingleObject(m_mutex, INFINITE)
    
    ' ===================== 核心修改：控制台Print → TextBox输出 =====================
    If m_toConsole Then
        ' 追加换行+日志文本到TextBox
        m_TextBox.Text = m_TextBox.Text & logText & vbCrLf
    End If
    ' ============================================================================
    
    ' 文件输出（修复Open语法：删除多余的#）
    If m_toFile Then
        Dim As String fullPath = m_filePath & "\" & This.GetDailyLogName()
        Dim As Integer fNum = FreeFile

        If m_useUTF8 Then
            If m_append Then
                Open fullPath For Append Encoding "utf8" As #fNum
            Else
                Open fullPath For Output Encoding "utf8" As #fNum
                Put #fNum, , Chr(&HEF) & Chr(&HBB) & Chr(&HBF) ' 在第一次打开时向头部写入BOM
            End If
        Else
            If m_append Then
                Open fullPath For Append As #fNum
            Else
                Open fullPath For Output As #fNum
            End If
        End If
        Print #fNum , logText
        Close #fNum
    End If
    
    ' 释放线程锁
    If m_mutex <> NULL Then ReleaseMutex(m_mutex)
End Sub

' 初始化线程互斥锁
Sub Logger.InitMutex()
    m_mutex = CreateMutex(NULL, FALSE, "FB_Logger_Mutex")
End Sub

' 释放线程互斥锁
Sub Logger.UninitMutex()
    If m_mutex <> NULL Then
        CloseHandle(m_mutex)
        m_mutex = NULL
    End If
End Sub

Sub Logger.SetOutput(console As Boolean, file As Boolean)
    m_toConsole = console
    m_toFile = file
End Sub

Sub Logger.SetLogPath(path As String)
    If Dir(path, 16) = "" Then MkDir path
    m_filePath = path
End Sub

' 修复关键字冲突：参数append改为isAppend
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

' 日志打印主方法
Sub Logger.PrintLog( _
    level As LogLevel, _
    msg As String, _
    module As String = "", _
    func As String = "", _
    apiName As String = "" _
)
    ' 校验开关和级别
    If Not m_enable Or level < m_filterLevel Then Exit Sub

    ' 拼接日志格式
    Dim As String logStr = "[" & This.GetTimeStamp() & "] [" & This.LevelToStr(level) & "]"
    If module <> "" Then logStr &= " [模块:" & module & "]"
    If func <> "" Then logStr &= " [函数:" & func & "]"
    If apiName <> "" Then logStr &= " [API:" & apiName & "]"
    logStr &= " " & msg

    ' 调用核心输出层
    This.OutputCore(logStr)
End Sub