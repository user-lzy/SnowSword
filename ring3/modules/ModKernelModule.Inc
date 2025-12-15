#include Once "win/psapi.bi"

#define IRP_MJ_CREATE                   &H00
#define IRP_MJ_CREATE_NAMED_PIPE        &H01
#define IRP_MJ_CLOSE                    &H02
#define IRP_MJ_READ                     &H03
#define IRP_MJ_WRITE                    &H04
#define IRP_MJ_QUERY_INFORMATION        &H05
#define IRP_MJ_SET_INFORMATION          &H06
#define IRP_MJ_QUERY_EA                 &H07
#define IRP_MJ_SET_EA                   &H08
#define IRP_MJ_FLUSH_BUFFERS            0x09
#define IRP_MJ_QUERY_VOLUME_INFORMATION &H0A
#define IRP_MJ_SET_VOLUME_INFORMATION   &H0B
#define IRP_MJ_DIRECTORY_CONTROL        &H0C
#define IRP_MJ_FILE_SYSTEM_CONTROL      &H0D
#define IRP_MJ_DEVICE_CONTROL           &H0E
#define IRP_MJ_INTERNAL_DEVICE_CONTROL  &H0F
#define IRP_MJ_SHUTDOWN                 &H10
#define IRP_MJ_LOCK_CONTROL             &H11
#define IRP_MJ_CLEANUP                  &H12
#define IRP_MJ_CREATE_MAILSLOT          &H13
#define IRP_MJ_QUERY_SECURITY           &H14
#define IRP_MJ_SET_SECURITY             &H15
#define IRP_MJ_POWER                    &H16
#define IRP_MJ_SYSTEM_CONTROL           &H17
#define IRP_MJ_DEVICE_CHANGE            &H18
#define IRP_MJ_QUERY_QUOTA              &H19
#define IRP_MJ_SET_QUOTA                &H1A
#define IRP_MJ_PNP                      &H1B
'#define IRP_MJ_PNP_POWER                IRP_MJ_PNP      // Obsolete....
#define IRP_MJ_MAXIMUM_FUNCTION         &H1B

Enum SYSTEM_INFORMATION_CLASS
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

Declare Function ZwQuerySystemInformation Lib "ntdll.dll" Alias "ZwQuerySystemInformation"(ByVal SystemInformationClass As SYSTEM_INFORMATION_CLASS,ByVal SystemInformation As PVOID,ByVal SystemInformationLength As ULong,ByVal ReturnLength As PULONG) As NTSTATUS
Declare Sub RtlInitUnicodeString Lib "ntdll.dll" Alias "RtlInitUnicodeString"(ByVal DestinationString As UNICODE_STRING Ptr,ByVal SourceString As PCWSTR)

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

Type DRIVER_INFO
    DriverObjectAddr As Integer
    AttachDevice As PVOID
    DriverInitAddr As Integer
    DriverStartIoAddr As Integer
    FastIoDispatchAddr As Integer
    DriverUnloadAddr As Integer
    MajorFunctionAddr(IRP_MJ_MAXIMUM_FUNCTION) As Integer
End Type

Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong)
    p->Length = SizeOf(OBJECT_ATTRIBUTES)
    p->RootDirectory = NULL
    p->Attributes = a
    p->ObjectName = n
    p->SecurityDescriptor = NULL
    p->SecurityQualityOfService = NULL
End Sub

Const STATUS_INFO_LENGTH_MISMATCH = Cast(NTSTATUS, &HC0000004)
Const STATUS_SUCCESS = Cast(NTSTATUS, &H00000000)

Function IRP_MJ_ToString(value As UInteger) As String
    Select Case value
        ' 标准IRP类型（0x00 - 0x1B）
        Case &H00
            Return "IRP_MJ_CREATE"
        Case &H01
            Return "IRP_MJ_CREATE_NAMED_PIPE"
        Case &H02
            Return "IRP_MJ_CLOSE"
        Case &H03
            Return "IRP_MJ_READ"
        Case &H04
            Return "IRP_MJ_WRITE"
        Case &H05
            Return "IRP_MJ_QUERY_INFORMATION"
        Case &H06
            Return "IRP_MJ_SET_INFORMATION"
        Case &H07
            Return "IRP_MJ_QUERY_EA"
        Case &H08
            Return "IRP_MJ_SET_EA"
        Case &H09
            Return "IRP_MJ_FLUSH_BUFFERS"
        Case &H0A
            Return "IRP_MJ_QUERY_VOLUME_INFORMATION"
        Case &H0B
            Return "IRP_MJ_SET_VOLUME_INFORMATION"
        Case &H0C
            Return "IRP_MJ_DIRECTORY_CONTROL"
        Case &H0D
            Return "IRP_MJ_FILE_SYSTEM_CONTROL"
        Case &H0E
            Return "IRP_MJ_DEVICE_CONTROL"
        Case &H0F
            Return "IRP_MJ_INTERNAL_DEVICE_CONTROL"
        Case &H10
            Return "IRP_MJ_SHUTDOWN"
        Case &H11
            Return "IRP_MJ_LOCK_CONTROL"
        Case &H12
            Return "IRP_MJ_CLEANUP"
        Case &H13
            Return "IRP_MJ_CREATE_MAILSLOT"
        Case &H14
            Return "IRP_MJ_QUERY_SECURITY"
        Case &H15
            Return "IRP_MJ_SET_SECURITY"
        Case &H16
            Return "IRP_MJ_POWER"
        Case &H17
            Return "IRP_MJ_SYSTEM_CONTROL"
        Case &H18
            Return "IRP_MJ_DEVICE_CHANGE"
        Case &H19
            Return "IRP_MJ_QUERY_QUOTA"
        Case &H1A
            Return "IRP_MJ_SET_QUOTA"
        Case &H1B
            Return "IRP_MJ_PNP"
        
        ' FltMgr扩展IRP（使用Cast(UCHAR, -n)转换负数值）
        Case Cast(UCHAR, -1)  ' 等价于 &HFF (255)
            Return "IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION"
        Case Cast(UCHAR, -2)  ' 等价于 &HFE (254)
            Return "IRP_MJ_RELEASE_FOR_SECTION_SYNCHRONIZATION"
        Case Cast(UCHAR, -3)  ' 等价于 &HFD (253)
            Return "IRP_MJ_ACQUIRE_FOR_MOD_WRITE"
        Case Cast(UCHAR, -4)  ' 等价于 &HFC (252)
            Return "IRP_MJ_RELEASE_FOR_MOD_WRITE"
        Case Cast(UCHAR, -5)  ' 等价于 &HFB (251)
            Return "IRP_MJ_ACQUIRE_FOR_CC_FLUSH"
        Case Cast(UCHAR, -6)  ' 等价于 &HFA (250)
            Return "IRP_MJ_RELEASE_FOR_CC_FLUSH"
        Case Cast(UCHAR, -7)  ' 等价于 &HF9 (249)
            Return "IRP_MJ_QUERY_OPEN"
        Case Cast(UCHAR, -13) ' 等价于 &HF3 (243)
            Return "IRP_MJ_FAST_IO_CHECK_IF_POSSIBLE"
        Case Cast(UCHAR, -14) ' 等价于 &HF2 (242)
            Return "IRP_MJ_NETWORK_QUERY_OPEN"
        Case Cast(UCHAR, -15) ' 等价于 &HF1 (241)
            Return "IRP_MJ_MDL_READ"
        Case Cast(UCHAR, -16) ' 等价于 &HF0 (240)
            Return "IRP_MJ_MDL_READ_COMPLETE"
        Case Cast(UCHAR, -17) ' 等价于 &HEF (239)
            Return "IRP_MJ_PREPARE_MDL_WRITE"
        Case Cast(UCHAR, -18) ' 等价于 &HEE (238)
            Return "IRP_MJ_MDL_WRITE_COMPLETE"
        Case Cast(UCHAR, -19) ' 等价于 &HED (237)
            Return "IRP_MJ_VOLUME_MOUNT"
        Case Cast(UCHAR, -20) ' 等价于 &HEC (236)
            Return "IRP_MJ_VOLUME_DISMOUNT"
        
        ' 未识别的IRP类型（显示十六进制值）
        Case Else
            Return "IRP_MJ_XXX (0x" & Hex(value, 2) & ")"
    End Select
End Function

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

Private Function GetDriverObjectAddr(ByVal DriverBaseAddr As PVOID) As Integer
    Dim pDriverObject As Integer, DriverBase As PVOID = DriverBaseAddr
    IoControl hDrv, IOCTL_GetDriverObjectByBaseAddress, @DriverBase, SizeOf(DriverBase), @pDriverObject, SizeOf(pDriverObject)
    Return pDriverObject
End Function

Public Sub GetDriverIOFunctionList(lvwIOFunction As Class_ListView, DriverObject As Integer)
    Print "0x" & WHex(DriverObject)
    Dim pDriverObject As Integer = DriverObject, DriverInfo As DRIVER_INFO
    If pDriverObject <> 0 Then
        IoControl hDrv, IOCTL_GetDriverInfo, @pDriverObject, SizeOf(pDriverObject), @DriverInfo, SizeOf(DriverInfo)
        lvwIOFunction.AddItemColList,, "DriverInit", "0x" & WHex(DriverInfo.DriverInitAddr), GetDriverPathByAddr(DriverInfo.DriverInitAddr)
        lvwIOFunction.AddItemColList,, "DriverStartIo", "0x" & WHex(DriverInfo.DriverStartIoAddr), GetDriverPathByAddr(DriverInfo.DriverStartIoAddr)
        lvwIOFunction.AddItemColList,, "FastIoDispatch", "0x" & WHex(DriverInfo.FastIoDispatchAddr), GetDriverPathByAddr(DriverInfo.FastIoDispatchAddr)
        lvwIOFunction.AddItemColList,, "DriverUnload", "0x" & WHex(DriverInfo.DriverUnloadAddr), GetDriverPathByAddr(DriverInfo.DriverUnloadAddr)
        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            If i = &H20 Then Continue For
            lvwIOFunction.AddItemColList ,, IRP_MJ_ToString(i), "0x" & WHex(DriverInfo.MajorFunctionAddr(i)), GetDriverPathByAddr(DriverInfo.MajorFunctionAddr(i))
        Next
    End If
End Sub

Public Sub GetKernelModuleList(lvwKernelModule As Class_ListView)
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        Print "Failed to allocate memory."
        Exit Sub
    End If
    lvwKernelModule.DeleteAllItems
    'First Call To Get the required buffer SIZE
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            Print "Failed to allocate memory."
            Exit Sub
        End If
    End If

    'Second call to get the actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        Print "NtQuerySystemInformation failed with st: 0x" & Hex(st)
        Deallocate buffer
        Exit Sub
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    'Print "Enumerated Drivers:"
    Print "Count:" & smi->Count
    For i As Integer = 0 To smi->Count - 1
        Dim DriverName As WString * 20, DriverPath As ZString * MAX_PATH = smi->Modules(i).ImageName
        lvwKernelModule.AddItem WStr(i)
        DriverPath = GetCorrectPath(DriverPath)
        'Print DriverPath
        DriverName = Right(DriverPath, Len(DriverPath) - InStrRev(DriverPath, "\"))
        'Print "Path:" & DriverPath
        lvwKernelModule.SetItemText i, 1, DriverName
        lvwKernelModule.SetItemText i, 2, "0x" & Hex(smi->Modules(i).dwBase)
        lvwKernelModule.SetItemText i, 3, "0x" & Hex(smi->Modules(i).dwSize)
        lvwKernelModule.SetItemText i, 4, DriverPath
        If IsDriverLoaded Then lvwKernelModule.SetItemText i, 5, "0x" & Hex(GetDriverObjectAddr(smi->Modules(i).dwBase))
        'lvwKernelModule.SetItemText i, 4, WStr(smi->Modules(i).LoadCount)
        'lvwKernelModule.SetItemText i, 5, WStr(smi->Modules(i).dwFlags)
    Next
    Deallocate buffer
End Sub

Public Function GetWString(ByVal SourceString As String) As LPWSTR
    Dim pStr As LPWSTR = Allocate(Len(SourceString) + 1)
    *pStr = StrToWStr(SourceString)
    Return pStr
End Function

Function GetDriverPathByAddr(ByVal Addr As Integer) As String
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        Print "Failed to allocate memory."
        Return "无法获取所在模块"
    End If

    'First Call To Get the required buffer SIZE
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            Print "Failed to allocate memory."
            Return "无法获取所在模块"
        End If
    End If

    'Second call to get the actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        Print "NtQuerySystemInformation failed with st: 0x" & Hex(st)
        Deallocate buffer
        Return "无法获取所在模块"
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    'Print "Enumerated Drivers:"
    For i As Integer = 0 To smi->Count - 1
        If (Cast(Integer, smi->Modules(i).dwBase) <= Addr And Cast(Integer, smi->Modules(i).dwBase + smi->Modules(i).dwSize) >= Addr) Then
            smi->Modules(i).ImageName = GetCorrectPath(smi->Modules(i).ImageName)
            Return smi->Modules(i).ImageName
        End If
    Next
    Deallocate buffer
    Return "未知模块"
End Function