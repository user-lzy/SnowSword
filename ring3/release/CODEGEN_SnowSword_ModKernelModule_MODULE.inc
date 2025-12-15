'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModKernelModule.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


' #include Once "win/psapi.bi" '已经搬到定义文件处

Private Enum SYSTEM_INFORMATION_CLASS
    SystemBasicInformation,               ' 0 Y N
    SystemProcessorInformation            ' 1 Y N
    SystemPerformanceInformation        ' 2 Y N
    SystemTimeOfDayInformation            ' 3 Y N
    SystemNotImplemented1                ' 4 Y N
    SystemProcessesAndThreadsInformation ' 5 Y N'----------------------
    SystemCallCounts                    ' 6 Y N
    SystemConfigurationInformation        ' 7 Y N
    SystemProcessorTimes                ' 8 Y N
    SystemGlobalFlag                    ' 9 Y Y
    SystemNotImplemented2                ' 10 Y N
    SystemModuleInformation            ' 11 Y N
    SystemLockInformation                ' 12 Y N
    SystemNotImplemented3                ' 13 Y N
    SystemNotImplemented4                ' 14 Y N
    SystemNotImplemented5                ' 15 Y N
    SystemHandleInformation            ' 16 Y N
    SystemObjectInformation            ' 17 Y N'----------------------
    SystemPagefileInformation            ' 18 Y N
    SystemInstructionEmulationCounts    ' 19 Y N
    SystemInvalidInfoClass1            ' 20
    SystemCacheInformation                ' 21 Y Y
    SystemPoolTagInformation            ' 22 Y N
    SystemProcessorStatistics            ' 23 Y N
    SystemDpcInformation                ' 24 Y Y'----------------------
    SystemNotImplemented6                ' 25 Y N
    SystemLoadImage                    ' 26 N Y'----------------------
    SystemUnloadImage                    ' 27 N Y'----------------------
    SystemTimeAdjustment                ' 28 Y Y
    SystemNotImplemented7                ' 29 Y N
    SystemNotImplemented8                ' 30 Y N
    SystemNotImplemented9                ' 31 Y N
    SystemCrashDumpInformation            ' 32 Y N
    SystemExceptionInformation            ' 33 Y N
    SystemCrashDumpStateInformation    ' 34 Y Y/N
    SystemKernelDebuggerInformation    ' 35 Y N'----------------------
    SystemContextSwitchInformation        ' 36 Y N
    SystemRegistryQuotaInformation        ' 37 Y Y
    SystemLoadAndCallImage                ' 38 N Y'----------------------
    SystemPrioritySeparation            ' 39 N Y
    SystemNotImplemented10                ' 40 Y N
    SystemNotImplemented11                ' 41 Y N
    SystemInvalidInfoClass2            ' 42
    SystemInvalidInfoClass3            ' 43
    SystemTimeZoneInformation            ' 44 Y N
    SystemLookasideInformation            ' 45 Y N
    SystemSetTimeSlipEvent                ' 46 N Y
    SystemCreateSession                ' 47 N Y'----------------------
    SystemDeleteSession                ' 48 N Y'----------------------
    SystemInvalidInfoClass4            ' 49
    SystemRangeStartInformation ' 50 Y N
    SystemVerifierInformation            ' 51 Y Y
    SystemAddVerifier                    ' 52 N Y
    SystemSessionProcessesInformation    ' 53 Y N
End Enum
    
Extern "C"  
    Declare Function ZwQuerySystemInformation Lib "ntdll.dll" (ByVal SystemInformationClass As SYSTEM_INFORMATION_CLASS,ByVal SystemInformation As PVOID,ByVal SystemInformationLength As ULong,ByVal ReturnLength As PULONG) As NTSTATUS
    Declare Sub RtlInitUnicodeString Lib "ntdll.dll"(ByVal DestinationString As UNICODE_STRING Ptr,ByVal SourceString As PCWSTR)
    'Declare Function ZwClose Lib "ntdll.dll" (ByVal TheHandle As HANDLE) As NTSTATUS
End Extern

Private Type SYSTEM_MODULE
    Reserved(1) As ULONG_PTR
    dwBase As PVOID
    dwSize As ULong
    dwFlags As ULong
    LoadOrderIndex As UShort
    InitOrderIndex As UShort
    LoadCount As UShort
    OffsetToFileName As UShort
    ImageName As ZString * 256
End Type

Private Type SYSTEM_MODULE_INFORMATION
    Count As ULong
    Modules(0) As SYSTEM_MODULE
End Type

Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong)
    p->Length = SizeOf(OBJECT_ATTRIBUTES)
    p->RootDirectory = NULL
    p->Attributes = a
    p->ObjectName = n
    p->SecurityDescriptor = NULL
    p->SecurityQualityOfService = NULL
End Sub

Private Const STATUS_INFO_LENGTH_MISMATCH = Cast(NTSTATUS, &HC0000004)
Private Const STATUS_SUCCESS = Cast(NTSTATUS, &H00000000)

Public Sub Replace(ByRef MainString As String,ByVal SearchString As String,ByVal ReplaceString As String)
    Dim MidString As String,i As Integer,SearchLen As Integer,MainLen As Integer
    MainLen = Len(MainString)
    SearchLen = Len(SearchString)
    For i = 1 To MainLen
        MidString = Mid(MainString,i,SearchLen)
        If (MidString = SearchString) Then
            MainString = Left(MainString,i - 1) & ReplaceString & Right(MainString,MainLen - (i + SearchLen - 1))
        End If
    Next
End Sub

Private Function GetCorrectPath(ByRef strPath As String) As String
    strPath = FF_Replace(strPath, "\??\", "")
    strPath = FF_Replace(strPath, "\SystemRoot", Environ("SystemRoot"))
    Return strPath
End Function

Public Sub GetKernelModuleList(lvwKernelModule As Class_ListView)
    Dim Status As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        PrintA "Failed to allocate memory."
        Exit Sub
    End If
    PrintA "1"
    'First Call To Get the required buffer SIZE
    Status = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (Status = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            PrintA "Failed to allocate memory."
            Exit Sub
        End If
    End If

    'Second call to get the actual data
    Status = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (Status < 0) Then
        AfxMsg "NtQuerySystemInformation failed with status: 0x" & Hex(Status)
        Deallocate buffer
        Exit Sub
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    'PrintA "Enumerated Drivers:"
    For i As Integer = 0 To smi->Count - 1
        Dim DriverName As WString * 20, DriverPath As ZString * MAX_PATH = smi->Modules(i).ImageName
        lvwKernelModule.AddItem WStr(i + 1)
        DriverPath = GetCorrectPath(DriverPath)
        'PrintA DriverPath
        DriverName = Right(DriverPath, Len(DriverPath) - InStrRev(DriverPath, "\"))
        'PrintA "Path:" & DriverPath
        lvwKernelModule.SetItemText i, 1, DriverName
        lvwKernelModule.SetItemText i, 2, "0x" & Hex(smi->Modules(i).dwBase)
        lvwKernelModule.SetItemText i, 3, "0x" & Hex(smi->Modules(i).dwSize)
        'lvwKernelModule.SetItemText i, 4, WStr(smi->Modules(i).LoadCount)
        'lvwKernelModule.SetItemText i, 5, WStr(smi->Modules(i).dwFlags)
        lvwKernelModule.SetItemText i, 4, DriverPath
    Next
    Deallocate buffer
End Sub

Public Function GetWString(ByVal SourceString As String) As LPWSTR
    Dim pStr As LPWSTR = Allocate(Len(SourceString) + 1)
    *pStr = StrToWStr(SourceString)
    Return pStr
End Function

Public Function GetDriverPathByAddr(ByVal Addr As ULONG64) As String
    Dim Status As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        PrintA "Failed to allocate memory."
        Return "无法获取所在模块"
    End If

    'First Call To Get the required buffer SIZE
    Status = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (Status = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            PrintA "Failed to allocate memory."
            Return "无法获取所在模块"
        End If
    End If

    'Second call to get the actual data
    Status = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (Status < 0) Then
        AfxMsg "NtQuerySystemInformation failed with status: 0x" & Hex(Status)
        Deallocate buffer
        Return "无法获取所在模块"
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    'PrintA "Enumerated Drivers:"
    For i As Integer = 0 To smi->Count - 1
        If (Cast(ULONG64, smi->Modules(i).dwBase) <= Addr And Cast(ULONG64, smi->Modules(i).dwBase + smi->Modules(i).dwSize) >= Addr) Then
            smi->Modules(i).ImageName = GetCorrectPath(smi->Modules(i).ImageName)
            Return smi->Modules(i).ImageName
        End If
    Next
    Deallocate buffer
    Return "未知模块"
End Function