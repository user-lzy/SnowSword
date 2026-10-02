#include Once "win/psapi.bi"

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

' ==============================================================================
' SYSTEM_INFORMATION_CLASS 完整枚举 (FreeBasic 64位)
' 区分: Q=NtQuerySystemInformation可读 / S=NtSetSystemInformation可写
' 仅收录你伪代码中出现、Windows原生公开/未公开有效Class
' ==============================================================================
/'Enum SYSTEM_INFORMATION_CLASS
    ' ========== 基础通用类 ==========
    SystemBasicInformation = 0          ' Q
    SystemProcessorInformation = 1     ' Q
    SystemPerformanceInformation = 2    ' Q
    SystemTimeOfDayInformation = 3      ' Q
    SystemProcessInformation = 5        ' Q
    SystemCallCounts = 6                ' Q(仅读,Set无实现)
    SystemModuleInformation = 11        ' Q
    SystemHandleInformation = 12       ' Q
    SystemObjectInformation = 14       ' Q
    SystemPageFileInformation = 18    ' Q

    ' ========== 内存管理类 ==========
    SystemWorkSetInformation = 21       ' Q+S
    SystemTimeAdjustmentInformation = 28 ' Q+S
    SystemCreateMirrorInformation = 30  ' S
    SystemMemoryListCommandInformation = 80 ' S (0x50 MmIssueMemoryListCommand)
    SystemMemoryScrubInformation = 127  ' S
    SystemCombinePhysicalPagesInfo = 130 ' S

    ' ========== DPC / 调度 / 上下文切换 ==========
    SystemDpcInformation = 24           ' Q+S
    SystemContextSwitchInformation = 36 ' Q(仅读,Set无实现)
    SystemPrioritySeparation = 39       ' Q+S

    ' ========== 驱动镜像加载卸载 ==========
    SystemLoadImageInformation = 26     ' S
    SystemUnloadImageInformation = 27   ' S
    SystemLoadAndCallImage = 38         ' S
    SystemConsoleDriverLoadInfo = 132   ' S condrv.sys 专用

    ' ========== 系统标志 / 调试全局配置 ==========
    SystemGlobalFlagInformation = 9     ' Q+S
    SystemGlobalFlag2Information = 207  ' Q+S

    ' ========== 崩溃转储 ==========
    SystemCrashDumpStateInformation = 34 ' Q+S

    ' ========== 注册表配额 ==========
    SystemRegistryQuotaInformation = 37 ' Q+S

    ' ========== ETW 性能跟踪 ==========
    SystemPerformanceTraceInformation = 31 ' Q+S

    ' ========== SuperFetch / 预取 ==========
    SystemSuperfetchInformation = 79    ' Q+S

    ' ========== 预取器(Prefetcher) ==========
    SystemPrefetcherInformation = 56     ' Q+S

    ' ========== 驱动验证器 Verifier ==========
    SystemAddVerifierEntry = 40         ' S
    SystemRemoveVerifierEntry = 41      ' S
    SystemVerifierInformation = 51       ' Q+S
    SystemVerifierInformationEx = 92     ' S

    ' ========== 内核对象引用跟踪 ==========
    SystemObjectRefTraceInformation = 86 ' Q+S

    ' ========== 时间滑移调试事件 ==========
    SystemTimeSlipEventInformation = 46  ' S

    ' ========== CPU 集 / 资源调度 ==========
    SystemCpuQuotaInformation = 113      ' Q+S
    SystemModifyAllowedCpuSets = 168     ' S
    SystemTagCpuSetsInfo = 176          ' S
    SystemIntSteerCpuSetAssign = 170     ' S

    ' ========== 时区配置 ==========
    SystemTimeZoneInformationClass = 93  ' Q+S

    ' ========== DPC 看门狗 ==========
    SystemDpcWatchdogConfig = 228       ' S

    ' ========== 闰秒开关 ==========
    SystemLeapSecondInfo = 206           ' Q+S

    ' ========== 调试内核断点配置 ==========
    SystemResourceTimeoutConfig = 236   ' S
    SystemContextUnwindBreakConfig = 237 ' S

    ' ========== VSL / VBS 虚拟化安全 ==========
    SystemVslRelaxQuotaInfo = 190       ' S
    SystemDumpEncryptionProvision = 194  ' S

    ' ========== IOMMU 设备控制 ==========
    SystemIommuBlockDevice = 233        ' S

    ' ========== Hypervisor 启动页配置 ==========
    SystemHvlBootPagesInfo = 235        ' S
End Enum'/

' 读写标记说明:
' Q = 仅 NtQuerySystemInformation 可读，NtSetSystemInformation 无对应逻辑
' S = 仅 NtSetSystemInformation 可写，Query无实现
' Q+S = 读写双向支持，Query读取参数/统计，Set修改配置

'Declare Function ZwQuerySystemInformation Lib "ntdll.dll" Alias "ZwQuerySystemInformation"(ByVal SystemInformationClass As SYSTEM_INFORMATION_CLASS,ByVal SystemInformation As PVOID,ByVal SystemInformationLength As ULong,ByVal ReturnLength As PULONG) As NTSTATUS
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
    DriverObjectAddr As UInteger
    AttachDevice As PVOID
    DriverInitAddr As UInteger
    DriverStartIoAddr As UInteger
    FastIoDispatchAddr As UInteger
    DriverUnloadAddr As UInteger
    MajorFunctionAddr(IRP_MJ_MAXIMUM_FUNCTION) As UInteger
    FastIOFunctionAddr(26) As UInteger
End Type

Type ENUM_DRIVER_DEVICES_INPUT
    DriverObject As ULongInt
End Type

Type DRIVER_DEVICE_LIST_HEADER
    Count As ULong
    TotalCount As ULong
End Type

Type DRIVER_DEVICE_INFO
    DeviceObject As ULongInt
    DriverObject As ULongInt
    NextDevice As ULongInt
    AttachedDevice As ULongInt
    LowerDevice As ULongInt
    TopDevice As ULongInt
    BaseDevice As ULongInt
    CurrentIrp As ULongInt
    Timer As ULongInt
    Vpb As ULongInt
    VpbDeviceObject As ULongInt
    VpbRealDevice As ULongInt
    DeviceExtension As ULongInt
    DeviceObjectExtension As ULongInt
    SecurityDescriptor As ULongInt

    ReferenceCount As Long
    DeviceType As ULong
    Characteristics As ULong
    dwFlags As ULong
    AlignmentRequirement As ULong
    ActiveThreadCount As ULong
    VpbFlags As ULong
    VpbReferenceCount As ULong
    VpbSerialNumber As ULong
    SectorSize As UShort
    StackSize As UByte
    Reserved0 As UByte

    DeviceName As WString * 260
End Type

#define KMDF_MAX_CALLBACKS 128

Type KMDF_CALLBACK_INFO Field = 8
    Id As ULong
    ContainerType As ULong
    Address As ULongInt
    ContainerObject As ULongInt
End Type

Type KMDF_CALLBACK_ENUM_REQUEST Field = 8
    DeviceObject As ULongInt
End Type

Type KMDF_CALLBACK_ENUM_RESULT Field = 8
    Count As ULong
    OffsetSource As ULong
    FxDevice As ULongInt
    FxDriver As ULongInt
    FxPkgPnp As ULongInt
    FxSelfManagedIoMachine As ULongInt
    Callbacks(0 To KMDF_MAX_CALLBACKS - 1) As KMDF_CALLBACK_INFO
End Type

#define KMDF_SYMBOL_LAYOUT_VERSION 4

#define KMDF_LAYOUT_SOURCE_NONE     0
#define KMDF_LAYOUT_SOURCE_SYMBOL   1
#define KMDF_LAYOUT_SOURCE_FALLBACK 2

Enum KMDF_CALLBACK_ID
    KmdfEvtDriverDeviceAdd = 1
    KmdfEvtDriverUnload

    KmdfEvtDeviceUsageNotification
    KmdfEvtDeviceUsageNotificationEx
    KmdfEvtDeviceRelationsQuery

    KmdfEvtDeviceD0Entry
    KmdfEvtDeviceD0EntryPostInterruptsEnabled
    KmdfEvtDeviceD0ExitPreInterruptsDisabled
    KmdfEvtDeviceD0Exit

    KmdfEvtDevicePrepareHardware
    KmdfEvtDeviceReleaseHardware
    KmdfEvtDeviceQueryStop
    KmdfEvtDeviceQueryRemove
    KmdfEvtDeviceSurpriseRemoval

    KmdfEvtDeviceSelfManagedIoCleanup
    KmdfEvtDeviceSelfManagedIoFlush
    KmdfEvtDeviceSelfManagedIoInit
    KmdfEvtDeviceSelfManagedIoSuspend
    KmdfEvtDeviceSelfManagedIoRestart
    
    KmdfEvtDeviceFileCreate
    KmdfEvtFileCleanup
    KmdfEvtFileClose
End Enum

Enum KMDF_CALLBACK_CONTAINER
    KmdfContainerFxDriver = 1
    KmdfContainerFxPkgPnp
    KmdfContainerFxSelfManagedIoMachine
    KmdfContainerFxFileObjectInfo
End Enum

Type KMDF_SYMBOL_OFFSETS Field = 4
    FxContextHeader_Object As Long
    FxContextHeader_Context As Long

    FxDevice_Driver As Long
    FxDevice_DeviceObject As Long
    MxDeviceObject_DeviceObject As Long
    FxDevice_PkgPnp As Long
    FxDevice_PkgGeneral As Long

    FxPkgGeneral_FileObjectInfoHeadList As Long

    FxFileObjectInfo_ListEntry As Long
    FxFileObjectInfo_EvtFileCreate As Long
    FxFileObjectInfo_EvtFileCleanup As Long
    FxFileObjectInfo_EvtFileClose As Long
    FxFileObjectInfo_ClassExtension As Long

    FxFileObjectFileCreate_Method As Long
    FxFileObjectFileCleanup_Method As Long
    FxFileObjectFileClose_Method As Long

    FxDriver_DriverDeviceAdd As Long
    FxDriver_DriverUnload As Long
    FxDriverDeviceAdd_Method As Long
    FxDriverUnload_Method As Long

    FxPkgPnp_SelfManagedIoMachine As Long

    FxPkgPnp_DeviceUsageNotification As Long
    FxPkgPnp_DeviceUsageNotificationEx As Long
    FxPkgPnp_DeviceRelationsQuery As Long
    FxPkgPnp_DeviceD0Entry As Long
    FxPkgPnp_DeviceD0EntryPostInterruptsEnabled As Long
    FxPkgPnp_DeviceD0ExitPreInterruptsDisabled As Long
    FxPkgPnp_DeviceD0Exit As Long
    FxPkgPnp_DevicePrepareHardware As Long
    FxPkgPnp_DeviceReleaseHardware As Long
    FxPkgPnp_DeviceQueryStop As Long
    FxPkgPnp_DeviceQueryRemove As Long
    FxPkgPnp_DeviceSurpriseRemoval As Long

    FxPnpDeviceUsageNotification_Method As Long
    FxPnpDeviceUsageNotificationEx_Method As Long
    FxPnpDeviceRelationsQuery_Method As Long
    FxPnpDeviceD0Entry_Method As Long
    FxPnpDeviceD0EntryPostInterruptsEnabled_Method As Long
    FxPnpDeviceD0ExitPreInterruptsDisabled_Method As Long
    FxPnpDeviceD0Exit_Method As Long
    FxPnpDevicePrepareHardware_Method As Long
    FxPnpDeviceReleaseHardware_Method As Long
    FxPnpDeviceQueryStop_Method As Long
    FxPnpDeviceQueryRemove_Method As Long
    FxPnpDeviceSurpriseRemoval_Method As Long

    FxSelfManagedIoMachine_DeviceSelfManagedIoCleanup As Long
    FxSelfManagedIoMachine_DeviceSelfManagedIoFlush As Long
    FxSelfManagedIoMachine_DeviceSelfManagedIoInit As Long
    FxSelfManagedIoMachine_DeviceSelfManagedIoSuspend As Long
    FxSelfManagedIoMachine_DeviceSelfManagedIoRestart As Long

    FxPnpDeviceSelfManagedIoCleanup_Method As Long
    FxPnpDeviceSelfManagedIoFlush_Method As Long
    FxPnpDeviceSelfManagedIoInit_Method As Long
    FxPnpDeviceSelfManagedIoSuspend_Method As Long
    FxPnpDeviceSelfManagedIoRestart_Method As Long

    FxDriver_DriverObject As Long
    MxDriverObject_DriverObject As Long
    FxDriver_Config As Long
    FxDriver_GetFxDriver_AddressLow As ULong
    FxDriver_GetFxDriver_AddressHigh As ULong
    FxDevice_Dispatch_AddressLow As ULong
    FxDevice_Dispatch_AddressHigh As ULong
    FxDevice_DispatchWithLock_AddressLow As ULong
    FxDevice_DispatchWithLock_AddressHigh As ULong
End Type

Type KMDF_SYMBOL_LAYOUT Field = 4
    Version As ULong
    Size As ULong
    Source As ULong
    Reserved As ULong
    DispatchPolicyKnown As ULong
    DispatchPolicy As ULong
    Offsets As KMDF_SYMBOL_OFFSETS
End Type

Enum KMDF_DISPATCH_POLICY
    KmdfDispatchPolicyUnknown = 0
    KmdfDispatchPolicyAllDispatch
    KmdfDispatchPolicyAllDispatchWithLock
    KmdfDispatchPolicyMixed
End Enum

Type WDF_MODULE_IDENTITY Field = 4
    PdbGuid As GUID
    PdbAge As ULong
    ImageSize As ULong
    TimeDateStamp As ULong
End Type

Type KMDF_DISPATCH_PROFILE Field = 4
    PdbGuid As GUID
    PdbAge As ULong
    Policy As ULong
End Type

Type KMDF_DRIVER_INFO_REQUEST Field = 4
    DriverObject As ULONG64
End Type

Type KMDF_DRIVER_INFO Field = 4
    IsKmdf As UByte
    OwnsDispatch As UByte
    DispatchPolicyKnown As UByte
    Reserved As UByte
    DriverInitFlags As ULong
    DispatchPolicy As ULong
    Reserved2 As ULong
    FxDriver As ULONG64
    Dispatch As ULONG64
    DispatchWithLock As ULONG64
End Type

Type GDT_ITEM
    Limit As ULong
    SEL_Base As ULONG64
    SEL_Limit As ULong
    Present As ULong
    Granularity As ULong
    DPL As ULong
    uType As ULong
    S As ULong
    'USHORT Segment
    'WCHAR Attributes[5]
End Type

Type GDT_INFO
    'DWORD NumOfItem
    Limit As UShort
    dwBase As ULONG64
    Item(10) As GDT_ITEM
End Type

Type IDT_ITEM
    HandlerFunc As ULONG64
    Selector As UShort
    Attributes As UShort
End Type

Type IDT_INFO
    IdtEntries(255) As IDT_ITEM
End Type

Enum MEMORY_SCAN_RESULT
    Normal = 0
    Shellcode
    HideDriver
    UnknownCode
End Enum

Type KernelModuleInfo
    DriverName As WString * 260
    BaseAddr As UInteger
    DriverPath As WString * 260
    result As MEMORY_SCAN_RESULT
End Type

' 定义线程参数结构
Type ScanThreadParam
    StartAddr As UInteger
    EndAddr As UInteger
    hMutex As Any Ptr
    ScannedPages As UInteger Ptr
    ThreadIndex As Integer
    bFinish As Boolean
End Type

Const NUM_THREADS As Integer = 8
Const PAGE_SIZE As DWORD = &H1000

' 全局扫描结果数组（线程共享）
Dim Shared g_ScanResults() As KernelModuleInfo
Dim Shared g_ScanResultCount As Integer = 0

Dim Shared g_KmdfSymbolResolveStarted As Boolean = False
Dim Shared g_KmdfSymbolLayoutReady As Boolean = False

Const STATUS_INFO_LENGTH_MISMATCH = Cast(NTSTATUS, &HC0000004)
Const STATUS_SUCCESS = Cast(NTSTATUS, &H00000000)

Sub InitializeObjectAttributes(ByVal p As OBJECT_ATTRIBUTES Ptr,ByVal n As UNICODE_STRING Ptr,ByVal a As ULong)
    p->Length = SizeOf(OBJECT_ATTRIBUTES)
    p->RootDirectory = NULL
    p->Attributes = a
    p->ObjectName = n
    p->SecurityDescriptor = NULL
    p->SecurityQualityOfService = NULL
End Sub

Private Function GetMemoryScanType(ByVal MemoryScanType As MEMORY_SCAN_RESULT) As String
    Select Case MemoryScanType
        Case Normal
            Return "Normal"
        Case ShellCode
            Return "ShellCode"
        Case HideDriver
            Return "HideDriver"
        Case UnknownCode
            Return "UnknownCode"
    End Select
End Function

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

Private Function GetCorrectPathW(strPath As StringW) As StringW
    strPath = YF_ReplaceW(strPath, "\??\", "")
    strPath = YF_ReplaceW(strPath, "\SystemRoot", Environ("SystemRoot"))
    Return strPath
End Function

Private Function GetDriverObjectAddr(ByVal DriverBaseAddr As PVOID) As UInteger
    Dim pDriverObject As UInteger, DriverBase As PVOID = DriverBaseAddr
    IoControl hDrv, IOCTL_GetDriverObjectByBaseAddress, @DriverBase, SizeOf(DriverBase), @pDriverObject, SizeOf(pDriverObject)
    Return pDriverObject
End Function

Public Sub GetDriverIOFunctionList(lvwIOFunction As Class_ListView, DriverPath As StringW, DriverObject As UInteger)
    Dim pDriverObject As UInteger = DriverObject
    Dim DriverInfo As DRIVER_INFO
    Dim OriginalDriverInfo As DRIVER_INFO

    InitListViewData lvwIOFunction

    If IoControl(hDrv, IOCTL_GetDriverInfo, @pDriverObject, SizeOf(pDriverObject), @DriverInfo, SizeOf(DriverInfo)) = 0 Then
        MyLog.PrintWin32Error "GetDriverIOFunctionList", "IOCTL_GetDriverInfo"
        Return
    End If

    Dim RecoveryStatus As ORIGINAL_DRIVER_RECOVERY_STATUS = _
        GetOriginalDispatchFunctionAddr(DriverPath, DriverInfo, OriginalDriverInfo)

    If RecoveryStatus = OriginalRecoveryFatal Then
        MyLog.PrintLog LOG_WARN, "GetDriverIOFunctionList",, "原始派遣函数恢复失败"
    ElseIf (RecoveryStatus And OriginalRecoveryDispatchDefaultOnly) <> 0 Then
        MyLog.PrintInfo "GetDriverIOFunctionList", "", "驱动 MajorFunction 全部保持系统默认值"
    ElseIf (RecoveryStatus And OriginalRecoveryDispatchUnresolved) <> 0 Then
        MyLog.PrintLog LOG_WARN, "GetDriverIOFunctionList",, "当前存在非默认 MajorFunction，但静态恢复器未能恢复"
    ElseIf (RecoveryStatus And OriginalRecoveryDispatchPartial) <> 0 Then
        MyLog.PrintLog LOG_WARN, "GetDriverIOFunctionList",, "MajorFunction 仅部分恢复"
    End If

    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverInit"), WStr("0x" & WHex(DriverInfo.DriverInitAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverInitAddr)), _
        WStr("-"), WStr("无")

    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverStartIo"), WStr("0x" & WHex(DriverInfo.DriverStartIoAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverStartIoAddr)), _
        WStr("-"), WStr("无")

    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverUnload"), WStr("0x" & WHex(DriverInfo.DriverUnloadAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverUnloadAddr)), _
        WStr("-"), WStr("无")

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        Dim ItemIndex As Long = AddItemColListEx( _
            lvwIOFunction, _
            5,,, _
            WStr(IRP_MJ_ToString(i)), _
            WStr("0x" & WHex(DriverInfo.MajorFunctionAddr(i))), _
            WStr(GetDriverPathByAddr(DriverInfo.MajorFunctionAddr(i))), _
            IIf(OriginalDriverInfo.MajorFunctionAddr(i) = NULL, WStr("-"), WStr("0x" & WHex(OriginalDriverInfo.MajorFunctionAddr(i)))), _
            IIf(OriginalDriverInfo.MajorFunctionAddr(i) = NULL, WStr("无"), WStr(GetDriverPathByAddr(OriginalDriverInfo.MajorFunctionAddr(i)))))

        If OriginalDriverInfo.MajorFunctionAddr(i) <> NULL AndAlso _
           DriverInfo.MajorFunctionAddr(i) <> OriginalDriverInfo.MajorFunctionAddr(i) Then

            SetItemColor lvwIOFunction, ItemIndex, FB_Red, FB_White
        End If

        FF_DoEvents
    Next

    AddItemColListEx lvwIOFunction, 5,,, WStr("FastIoDispatch"), WStr("0x" & WHex(DriverInfo.FastIoDispatchAddr)), WStr(GetDriverPathByAddr(DriverInfo.FastIoDispatchAddr)), _
        WStr("-"), WStr("无")

    If DriverInfo.FastIoDispatchAddr <> NULL Then
        For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
            Dim ItemIndex As Long = AddItemColListEx( _
                lvwIOFunction, _
                5,,, _
                WStr(FastIoIndexToName(i)), _
                WStr("0x" & WHex(DriverInfo.FastIOFunctionAddr(i))), _
                WStr(GetDriverPathByAddr(DriverInfo.FastIOFunctionAddr(i))), _
                IIf(OriginalDriverInfo.FastIOFunctionAddr(i) = NULL, WStr("-"), WStr("0x" & WHex(OriginalDriverInfo.FastIOFunctionAddr(i)))), _
                IIf(OriginalDriverInfo.FastIOFunctionAddr(i) = NULL, WStr("无"), WStr(GetDriverPathByAddr(OriginalDriverInfo.FastIOFunctionAddr(i)))))

            If OriginalDriverInfo.FastIOFunctionAddr(i) <> NULL AndAlso _
               DriverInfo.FastIOFunctionAddr(i) <> OriginalDriverInfo.FastIOFunctionAddr(i) Then

                SetItemColor lvwIOFunction, ItemIndex, FB_Red, FB_White
            End If

            FF_DoEvents
        Next
    End If

    CommitListViewView lvwIOFunction
End Sub

Private Function DeviceTypeToString(ByVal DeviceType As ULong) As String
    Select Case DeviceType
        Case &H02: Return "CD_ROM"
        Case &H03: Return "CD_ROM_FILE_SYSTEM"
        Case &H04: Return "CONTROLLER"
        Case &H07: Return "DISK"
        Case &H08: Return "DISK_FILE_SYSTEM"
        Case &H09: Return "FILE_SYSTEM"
        Case &H0B: Return "KEYBOARD"
        Case &H0C: Return "MAILSLOT"
        Case &H0F: Return "MOUSE"
        Case &H11: Return "NAMED_PIPE"
        Case &H12: Return "NETWORK"
        Case &H14: Return "NETWORK_FILE_SYSTEM"
        Case &H15: Return "NULL"
        Case &H16: Return "PARALLEL_PORT"
        Case &H17: Return "PHYSICAL_NETCARD"
        Case &H18: Return "PRINTER"
        Case &H1B: Return "SERIAL_PORT"
        Case &H1F: Return "TAPE"
        Case &H21: Return "TRANSPORT"
        Case &H22: Return "UNKNOWN"
        Case &H23: Return "VIDEO"
        Case &H24: Return "VIRTUAL_DISK"
        Case Else: Return "0x" & Hex(DeviceType)
    End Select
End Function

Public Sub GetDriverDeviceList(lvwDevice As Class_ListView, ByVal DriverObject As UInteger)
    Dim enum_Input As ENUM_DRIVER_DEVICES_INPUT
    Dim capacity As ULong = 32

    InitListViewData lvwDevice

    enum_Input.DriverObject = DriverObject
    If enum_Input.DriverObject = 0 Then
        CommitListViewView lvwDevice
        Exit Sub
    End If

    For attempt As Integer = 0 To 4
        Dim bufferSize As DWORD = SizeOf(DRIVER_DEVICE_LIST_HEADER) + capacity * SizeOf(DRIVER_DEVICE_INFO)
        Dim pBuffer As UByte Ptr = Callocate(bufferSize)
        If pBuffer = NULL Then
            MyLog.PrintLog LOG_ERROR, "GetDriverDeviceList", "Callocate failed. Size=" & Str(bufferSize)
            Exit Sub
        End If

        Dim bytesReturned As DWORD
        SetLastError 0

        If IoControl(hDrv, IOCTL_EnumDriverDevices, @enum_Input, SizeOf(enum_Input), pBuffer, bufferSize, @bytesReturned) = 0 Then
            MyLog.PrintWin32Error "GetDriverDeviceList", "IOCTL_EnumDriverDevices"
            Deallocate pBuffer
            Exit Sub
        End If

        Dim pHeader As DRIVER_DEVICE_LIST_HEADER Ptr = Cast(DRIVER_DEVICE_LIST_HEADER Ptr, pBuffer)

        If pHeader->TotalCount > capacity Then
            capacity = pHeader->TotalCount
            Deallocate pBuffer
            Continue For
        End If

        Dim pInfo As DRIVER_DEVICE_INFO Ptr = Cast(DRIVER_DEVICE_INFO Ptr, pBuffer + SizeOf(DRIVER_DEVICE_LIST_HEADER))
        If pHeader->Count = 0 Then
            Deallocate pBuffer
            CommitListViewView lvwDevice
            Exit Sub
        End If
        For i As ULong = 0 To pHeader->Count - 1
            Dim DeviceName As StringW = pInfo[i].DeviceName
            If LenW(DeviceName) = 0 Then DeviceName = "(未命名)"

            Dim ItemIndex As Long = AddItemColListEx(lvwDevice, 6,,, _
                WStr(DeviceName), _
                WStr("0x" & WHex(pInfo[i].DeviceObject)), _
                WStr(DeviceTypeToString(pInfo[i].DeviceType) & " (0x" & Hex(pInfo[i].DeviceType) & ")"), _
                WStr("0x" & WHex(pInfo[i].AttachedDevice)), _
                WStr("0x" & WHex(pInfo[i].LowerDevice)), _
                WStr("0x" & Hex(pInfo[i].dwFlags)))

            If pInfo[i].DriverObject <> enum_Input.DriverObject Then
                SetItemColor lvwDevice, ItemIndex, FB_Red, FB_White
            End If
        Next

        Deallocate pBuffer
        CommitListViewView lvwDevice
        Exit Sub
    Next

    MyLog.PrintLog LOG_ERROR, "GetDriverDeviceList", "设备数量持续变化，枚举重试次数已耗尽"
End Sub

Function KmdfLoadWdfSymbols(ByVal ModulePath As WString Ptr) As Boolean
    Dim pReq As SYMBOL_REQUEST Ptr

    pReq = SymbolActor_SubmitRequest( _
        REQ_LOAD_MODULE, _
        ModulePath)

    If pReq = 0 Then Return False

    If Not SymbolActor_WaitRequest(pReq) Then
        MyLog.PrintLog LOG_WARN, "KmdfLoadWdfSymbols",, "WDF 符号请求超时"
        SymbolActor_ReleaseRequest(pReq)
        Return False
    End If

    Dim success As Boolean = _
        (pReq->ResultStatus = STATUS_SUCCESS)

    SymbolActor_ReleaseRequest(pReq)

    Return success
End Function

Function KmdfResolveRequired( _
    ByVal ModulePath As WString Ptr, _
    ByVal TypeName As WString Ptr, _
    ByVal MemberName As WString Ptr, _
    ByRef Value As Long _
) As Boolean

    If Not KmdfQueryStructOffset(ModulePath, TypeName, MemberName, Value) Then
        MyLog.PrintLog LOG_ERROR, "BuildKmdfSymbolLayout",, _
            *TypeName & "." & *MemberName & " 解析失败"
        Return False
    End If

    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
        *TypeName & "." & *MemberName & "=0x" & Hex(Value)

    Return True
End Function

Function KmdfResolveOptional( _
    ByVal ModulePath As WString Ptr, _
    ByVal TypeName As WString Ptr, _
    ByVal MemberName As Wstring Ptr, _
    ByRef Value As Long _
) As Boolean

    Value = -1

    If Not KmdfQueryStructOffset(ModulePath, TypeName, MemberName, Value) Then
        MyLog.PrintLog LOG_WARN, "BuildKmdfSymbolLayout",, _
            *TypeName & "." & *MemberName & " 不可用"
        Value = -1
        Return False
    End If

    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
        *TypeName & "." & *MemberName & "=0x" & Hex(Value)

    Return True
End Function

Function KmdfResolveFunctionAddress( _
    ByVal ModulePath As WString Ptr, _
    ByVal SymbolName As WString Ptr, _
    ByRef Address As ULONG64 _
) As Boolean
    Dim pReq As SYMBOL_REQUEST Ptr
    Dim imageSize As ULong
    Dim moduleBase As ULONG64

    Address = 0
    pReq = SymbolActor_SubmitRequest(REQ_SYMBOL_TO_ADDR, ModulePath, SymbolName)
    If pReq = 0 Then Return False
    If Not SymbolActor_WaitRequest(pReq) Then
        MyLog.PrintLog LOG_WARN, "KmdfResolveFunctionAddress",, _
            *SymbolName & " 符号请求超时"
        SymbolActor_ReleaseRequest(pReq)
        Return False
    End If

    If pReq->ResultStatus <> STATUS_SUCCESS Then
        SymbolActor_ReleaseRequest(pReq)
        Return False
    End If

    moduleBase = Cast(ULONG64, GetKernelModuleBase(WStr("Wdf01000.sys"), imageSize))
    If moduleBase <> 0 Then Address = moduleBase + pReq->ResultAddress
    SymbolActor_ReleaseRequest(pReq)
    Return Address <> 0
End Function

Function KmdfFormatGuid(ByRef Value As GUID) As String
    Dim result As String = "{" & Hex(Value.Data1, 8) & "-" & Hex(Value.Data2, 4) & "-" & Hex(Value.Data3, 4) & "-"
    result &= Hex(Value.Data4(0), 2) & Hex(Value.Data4(1), 2) & "-"
    For i As Integer = 2 To 7
        result &= Hex(Value.Data4(i), 2)
    Next
    Return result & "}"
End Function

Function LookupKmdfDispatchPolicy(ByRef Identity As WDF_MODULE_IDENTITY, ByRef Policy As KMDF_DISPATCH_POLICY) As Boolean
    Dim profiles(0 To 0) As KMDF_DISPATCH_PROFILE
    Policy = KmdfDispatchPolicyUnknown

    ' Win10 22H2 target profile: PDB identity only; PE metadata is diagnostic.
    profiles(0).PdbGuid.Data1 = Cast(ULong, &HCCF30DF6ULL)
    profiles(0).PdbGuid.Data2 = &H86C8
    profiles(0).PdbGuid.Data3 = &H7755
    profiles(0).PdbGuid.Data4(0) = &HB0
    profiles(0).PdbGuid.Data4(1) = &H1C
    profiles(0).PdbGuid.Data4(2) = &H27
    profiles(0).PdbGuid.Data4(3) = &H76
    profiles(0).PdbGuid.Data4(4) = &H9D
    profiles(0).PdbGuid.Data4(5) = &H6D
    profiles(0).PdbGuid.Data4(6) = &H70
    profiles(0).PdbGuid.Data4(7) = &HFE
    profiles(0).PdbAge = 1
    profiles(0).Policy = KmdfDispatchPolicyAllDispatchWithLock

    For i As Integer = 0 To UBound(profiles)
        If Identity.PdbGuid.Data1 <> profiles(i).PdbGuid.Data1 OrElse _
           Identity.PdbGuid.Data2 <> profiles(i).PdbGuid.Data2 OrElse _
           Identity.PdbGuid.Data3 <> profiles(i).PdbGuid.Data3 OrElse _
           Identity.PdbAge <> profiles(i).PdbAge Then Continue For

        Dim guidMatches As Boolean = True
        For j As Integer = 0 To 7
            If Identity.PdbGuid.Data4(j) <> profiles(i).PdbGuid.Data4(j) Then
                guidMatches = False
                Exit For
            End If
        Next
        If guidMatches Then
            Policy = profiles(i).Policy
            Return True
        End If
    Next
    Return False
End Function

Function BuildKmdfSymbolLayout( _
    ByRef Layout As KMDF_SYMBOL_LAYOUT _
) As Boolean

    If SizeOf(KMDF_SYMBOL_OFFSETS) <> 256 OrElse _
       SizeOf(KMDF_SYMBOL_LAYOUT) <> 280 OrElse _
       SizeOf(KMDF_DRIVER_INFO) <> 40 OrElse _
       (Cast(ULongInt, @Layout.Offsets) - Cast(ULongInt, @Layout)) <> 24 Then
        MyLog.PrintLog LOG_ERROR, "BuildKmdfSymbolLayout",, "KMDF ABI size mismatch"
        Return False
    End If

    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
    "sizeof offsets=" & SizeOf(KMDF_SYMBOL_OFFSETS) & _
    " layout=" & SizeOf(KMDF_SYMBOL_LAYOUT)
    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
    "ABI PkgPnp=" & _
    (Cast(ULongInt, @Layout.Offsets.FxDevice_PkgPnp) - Cast(ULongInt, @Layout.Offsets)) & _
    " PkgGeneral=" & _
    (Cast(ULongInt, @Layout.Offsets.FxDevice_PkgGeneral) - Cast(ULongInt, @Layout.Offsets)) & _
    " DriverAdd=" & _
    (Cast(ULongInt, @Layout.Offsets.FxDriver_DriverDeviceAdd) - Cast(ULongInt, @Layout.Offsets))
    
    Dim ModuleName As WString * 64 = "Wdf01000.sys"
    Dim ModulePath As WString * MAX_PATH

    memset(@Layout, &HFF, SizeOf(Layout))

    Layout.Version = KMDF_SYMBOL_LAYOUT_VERSION
    Layout.SIZE = SizeOf(KMDF_SYMBOL_LAYOUT)
    Layout.Source = KMDF_LAYOUT_SOURCE_SYMBOL
    Layout.Reserved = 0
    Layout.DispatchPolicyKnown = 0
    Layout.DispatchPolicy = KmdfDispatchPolicyUnknown
    Layout.Offsets.FxDriver_GetFxDriver_AddressLow = 0
    Layout.Offsets.FxDriver_GetFxDriver_AddressHigh = 0
    Layout.Offsets.FxDevice_Dispatch_AddressLow = 0
    Layout.Offsets.FxDevice_Dispatch_AddressHigh = 0
    Layout.Offsets.FxDevice_DispatchWithLock_AddressLow = 0
    Layout.Offsets.FxDevice_DispatchWithLock_AddressHigh = 0

    If Not GetKernelModulePath(@ModuleName, @ModulePath) Then
        MyLog.PrintLog LOG_ERROR, "BuildKmdfSymbolLayout",, _
            "无法获取 Wdf01000.sys 路径"
        Return False
    End If

    ModulePath = YF_ReplaceW( _
        ModulePath, _
        "\SystemRoot", _
        WStr(Environ("SystemRoot")))

    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
        "Wdf01000=" & ModulePath

    If Not KmdfLoadWdfSymbols(@ModulePath) Then
        MyLog.PrintLog LOG_ERROR, "BuildKmdfSymbolLayout",, _
            "Wdf01000 符号加载失败"
        Return False
    End If

    Dim identity As WDF_MODULE_IDENTITY
    Dim dispatchPolicy As KMDF_DISPATCH_POLICY = KmdfDispatchPolicyUnknown
    If KmdfQueryModuleIdentity(@ModulePath, identity) Then
        MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
            "[KMDF] Wdf01000 identity: PdbGuid=" & KmdfFormatGuid(identity.PdbGuid) & _
            " Age=" & identity.PdbAge & _
            " ImageSize=0x" & Hex(identity.ImageSize) & _
            " TimeDateStamp=0x" & Hex(identity.TimeDateStamp)
        If LookupKmdfDispatchPolicy(identity, dispatchPolicy) Then
            Layout.DispatchPolicyKnown = 1
            Layout.DispatchPolicy = dispatchPolicy
            MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
                "[KMDF] dispatch profile matched: AllDispatchWithLock"
        Else
            MyLog.PrintLog LOG_WARN, "BuildKmdfSymbolLayout",, _
                "[KMDF] Wdf01000 dispatch profile unknown"
        End If
    Else
        MyLog.PrintLog LOG_WARN, "BuildKmdfSymbolLayout",, _
            "[KMDF] Wdf01000 dispatch profile unknown"
    End If
    
    ' Core traversal
    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxContextHeader"), StrPtrW("Object"), _
        Layout.Offsets.FxContextHeader_Object) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxContextHeader"), StrPtrW("Context"), _
        Layout.Offsets.FxContextHeader_Context) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxDevice"), StrPtrW("m_Driver"), _
        Layout.Offsets.FxDevice_Driver) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxDevice"), StrPtrW("m_DeviceObject"), _
        Layout.Offsets.FxDevice_DeviceObject) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("MxDeviceObject"), StrPtrW("m_DeviceObject"), _
        Layout.Offsets.MxDeviceObject_DeviceObject) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxDevice"), StrPtrW("m_PkgPnp"), _
        Layout.Offsets.FxDevice_PkgPnp) Then Return False
    
    ' Driver callback
    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxDriver"), StrPtrW("m_DriverDeviceAdd"), _
        Layout.Offsets.FxDriver_DriverDeviceAdd) Then Return False

    If Not KmdfResolveRequired(@ModulePath, StrPtrW("FxDriverDeviceAdd"), StrPtrW("Method"), _
        Layout.Offsets.FxDriverDeviceAdd_Method) Then Return False
    
    ' Optional
    KmdfResolveOptional @ModulePath, StrPtrW("FxDriver"), StrPtrW("m_DriverUnload"), _
        Layout.Offsets.FxDriver_DriverUnload

    KmdfResolveOptional @ModulePath, StrPtrW("FxDriver"), StrPtrW("m_DriverObject"), _
        Layout.Offsets.FxDriver_DriverObject

    KmdfResolveOptional @ModulePath, StrPtrW("MxDriverObject"), StrPtrW("m_DriverObject"), _
        Layout.Offsets.MxDriverObject_DriverObject

    KmdfResolveOptional @ModulePath, StrPtrW("FxDriver"), StrPtrW("m_Config"), _
        Layout.Offsets.FxDriver_Config

    Dim symbolAddress As ULONG64
    If KmdfResolveFunctionAddress(@ModulePath, StrPtrW("Wdf01000!FxDriver::GetFxDriver"), symbolAddress) Then
        Layout.Offsets.FxDriver_GetFxDriver_AddressLow = Cast(ULong, symbolAddress And &HFFFFFFFFULL)
        Layout.Offsets.FxDriver_GetFxDriver_AddressHigh = Cast(ULong, symbolAddress Shr 32)
    ElseIf KmdfResolveFunctionAddress(@ModulePath, StrPtrW("FxDriver::GetFxDriver"), symbolAddress) Then
        Layout.Offsets.FxDriver_GetFxDriver_AddressLow = Cast(ULong, symbolAddress And &HFFFFFFFFULL)
        Layout.Offsets.FxDriver_GetFxDriver_AddressHigh = Cast(ULong, symbolAddress Shr 32)
    End If

    If KmdfResolveFunctionAddress(@ModulePath, StrPtrW("Wdf01000!FxDevice::Dispatch"), symbolAddress) OrElse _
       KmdfResolveFunctionAddress(@ModulePath, StrPtrW("FxDevice::Dispatch"), symbolAddress) Then
        Layout.Offsets.FxDevice_Dispatch_AddressLow = Cast(ULong, symbolAddress And &HFFFFFFFFULL)
        Layout.Offsets.FxDevice_Dispatch_AddressHigh = Cast(ULong, symbolAddress Shr 32)
    End If

    If KmdfResolveFunctionAddress(@ModulePath, StrPtrW("Wdf01000!FxDevice::DispatchWithLock"), symbolAddress) OrElse _
       KmdfResolveFunctionAddress(@ModulePath, StrPtrW("FxDevice::DispatchWithLock"), symbolAddress) Then
        Layout.Offsets.FxDevice_DispatchWithLock_AddressLow = Cast(ULong, symbolAddress And &HFFFFFFFFULL)
        Layout.Offsets.FxDevice_DispatchWithLock_AddressHigh = Cast(ULong, symbolAddress Shr 32)
    End If

    KmdfResolveOptional @ModulePath, StrPtrW("FxDriverUnload"), StrPtrW("Method"), _
        Layout.Offsets.FxDriverUnload_Method
    
    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxDevice"), _
        StrPtrW("m_PkgGeneral"), _
        Layout.Offsets.FxDevice_PkgGeneral

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPkgGeneral"), _
        StrPtrW("m_FileObjectInfoHeadList"), _
        Layout.Offsets.FxPkgGeneral_FileObjectInfoHeadList

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectInfo"), _
        StrPtrW("ListEntry"), _
        Layout.Offsets.FxFileObjectInfo_ListEntry

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectInfo"), _
        StrPtrW("EvtFileCreate"), _
        Layout.Offsets.FxFileObjectInfo_EvtFileCreate

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectInfo"), _
        StrPtrW("EvtFileCleanup"), _
        Layout.Offsets.FxFileObjectInfo_EvtFileCleanup

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectInfo"), _
        StrPtrW("EvtFileClose"), _
        Layout.Offsets.FxFileObjectInfo_EvtFileClose

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectInfo"), _
        StrPtrW("ClassExtension"), _
        Layout.Offsets.FxFileObjectInfo_ClassExtension
    
    ' warpper
    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectFileCreate"), _
        StrPtrW("Method"), _
        Layout.Offsets.FxFileObjectFileCreate_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectFileCleanup"), _
        StrPtrW("Method"), _
        Layout.Offsets.FxFileObjectFileCleanup_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxFileObjectFileClose"), _
        StrPtrW("Method"), _
        Layout.Offsets.FxFileObjectFileClose_Method
    
    ' FxPkgPnp callback
    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceD0Entry"), _
        Layout.Offsets.FxPkgPnp_DeviceD0Entry

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceD0Entry"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceD0Entry_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceD0Exit"), _
        Layout.Offsets.FxPkgPnp_DeviceD0Exit

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceD0Exit"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceD0Exit_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DevicePrepareHardware"), _
        Layout.Offsets.FxPkgPnp_DevicePrepareHardware

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDevicePrepareHardware"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDevicePrepareHardware_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceReleaseHardware"), _
        Layout.Offsets.FxPkgPnp_DeviceReleaseHardware

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceReleaseHardware"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceReleaseHardware_Method
    
    ' ============================================================
    ' Usage / Relations
    ' ============================================================

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceUsageNotification"), _
        Layout.Offsets.FxPkgPnp_DeviceUsageNotification

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceUsageNotification"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceUsageNotification_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceUsageNotificationEx"), _
        Layout.Offsets.FxPkgPnp_DeviceUsageNotificationEx

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceUsageNotificationEx"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceUsageNotificationEx_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceRelationsQuery"), _
        Layout.Offsets.FxPkgPnp_DeviceRelationsQuery

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceRelationsQuery"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceRelationsQuery_Method
    
    ' D0 pre/post
    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPkgPnp"), _
        StrPtrW("m_DeviceD0EntryPostInterruptsEnabled"), _
        Layout.Offsets.FxPkgPnp_DeviceD0EntryPostInterruptsEnabled

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceD0EntryPostInterruptsEnabled"), _
        StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceD0EntryPostInterruptsEnabled_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPkgPnp"), _
        StrPtrW("m_DeviceD0ExitPreInterruptsDisabled"), _
        Layout.Offsets.FxPkgPnp_DeviceD0ExitPreInterruptsDisabled

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceD0ExitPreInterruptsDisabled"), _
        StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceD0ExitPreInterruptsDisabled_Method
    
    ' QueryStop / QueryRemove / SurpriseRemoval
    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceQueryStop"), _
        Layout.Offsets.FxPkgPnp_DeviceQueryStop

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceQueryStop"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceQueryStop_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceQueryRemove"), _
        Layout.Offsets.FxPkgPnp_DeviceQueryRemove

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceQueryRemove"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceQueryRemove_Method

    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_DeviceSurpriseRemoval"), _
        Layout.Offsets.FxPkgPnp_DeviceSurpriseRemoval

    KmdfResolveOptional @ModulePath, StrPtrW("FxPnpDeviceSurpriseRemoval"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSurpriseRemoval_Method
    
    ' SelfManagedIo
    KmdfResolveOptional @ModulePath, StrPtrW("FxPkgPnp"), StrPtrW("m_SelfManagedIoMachine"), _
        Layout.Offsets.FxPkgPnp_SelfManagedIoMachine
    
    KmdfResolveOptional @ModulePath, StrPtrW("FxSelfManagedIoMachine"), _
        StrPtrW("m_DeviceSelfManagedIoCleanup"), _
        Layout.Offsets.FxSelfManagedIoMachine_DeviceSelfManagedIoCleanup

    KmdfResolveOptional @ModulePath, StrPtrW("FxSelfManagedIoMachine"), _
        StrPtrW("m_DeviceSelfManagedIoFlush"), _
        Layout.Offsets.FxSelfManagedIoMachine_DeviceSelfManagedIoFlush

    KmdfResolveOptional @ModulePath, StrPtrW("FxSelfManagedIoMachine"), _
        StrPtrW("m_DeviceSelfManagedIoInit"), _
        Layout.Offsets.FxSelfManagedIoMachine_DeviceSelfManagedIoInit

    KmdfResolveOptional @ModulePath, StrPtrW("FxSelfManagedIoMachine"), _
        StrPtrW("m_DeviceSelfManagedIoSuspend"), _
        Layout.Offsets.FxSelfManagedIoMachine_DeviceSelfManagedIoSuspend

    KmdfResolveOptional @ModulePath, StrPtrW("FxSelfManagedIoMachine"), _
        StrPtrW("m_DeviceSelfManagedIoRestart"), _
        Layout.Offsets.FxSelfManagedIoMachine_DeviceSelfManagedIoRestart
    
    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceSelfManagedIoCleanup"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSelfManagedIoCleanup_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceSelfManagedIoFlush"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSelfManagedIoFlush_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceSelfManagedIoInit"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSelfManagedIoInit_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceSelfManagedIoSuspend"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSelfManagedIoSuspend_Method

    KmdfResolveOptional @ModulePath, _
        StrPtrW("FxPnpDeviceSelfManagedIoRestart"), StrPtrW("m_Method"), _
        Layout.Offsets.FxPnpDeviceSelfManagedIoRestart_Method
        
    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
        "KMDF Symbol Layout 构建完成"
    Dim resolved As Integer = 0
    Dim total As Integer = SizeOf(KMDF_SYMBOL_OFFSETS) \ SizeOf(Long)

    Dim p As Long Ptr = Cast(Long Ptr, @Layout.Offsets)

    For i As Integer = 0 To total - 1
        If p[i] >= 0 Then resolved += 1
    Next

    MyLog.PrintInfo "BuildKmdfSymbolLayout",, _
        "Offset resolved=" & resolved & "/" & total
    Return True
End Function

Function SendKmdfSymbolLayout( _
    ByRef Layout As KMDF_SYMBOL_LAYOUT _
) As Boolean

    Dim bytesRet As DWORD

    If SizeOf(KMDF_SYMBOL_OFFSETS) <> 256 OrElse _
       SizeOf(KMDF_SYMBOL_LAYOUT) <> 280 Then
        MyLog.PrintLog LOG_ERROR, "SendKmdfSymbolLayout",, "KMDF ABI size mismatch"
        Return False
    End If

    SetLastError 0

    If IoControl( _
        hDrv, _
        IOCTL_SetKmdfSymbolLayout, _
        @Layout, _
        SizeOf(Layout), _
        NULL, _
        0, _
        @bytesRet) = 0 Then

        MyLog.PrintWin32Error _
            "SendKmdfSymbolLayout", _
            "IOCTL_SetKmdfSymbolLayout"

        Return False
    End If

    MyLog.PrintInfo _
        "SendKmdfSymbolLayout",, _
        "KMDF Symbol Layout 已下发"

    Return True
End Function

Function KmdfSymbolLayoutWorker( _
    ByVal Parameter As Any Ptr _
) As DWORD

    Dim layout As KMDF_SYMBOL_LAYOUT

    If BuildKmdfSymbolLayout(layout) Then
        If SendKmdfSymbolLayout(layout) Then
            g_KmdfSymbolLayoutReady = True

            MyLog.PrintInfo _
                "KmdfSymbolLayoutWorker",, _
                "KMDF offset 已切换到符号模式"
        End If
    Else
        MyLog.PrintLog LOG_WARN, _
            "KmdfSymbolLayoutWorker",, _
            "符号解析失败，继续使用 fallback"
    End If

    Return 0
End Function

Sub EnsureKmdfSymbolLayoutAsync()
    If g_KmdfSymbolResolveStarted Then Exit Sub

    g_KmdfSymbolResolveStarted = True

    Dim hThread As HANDLE = CreateThread( _
        NULL, _
        0, _
        @KmdfSymbolLayoutWorker, _
        NULL, _
        0, _
        NULL)

    If hThread Then
        CloseHandle hThread
    Else
        g_KmdfSymbolResolveStarted = False
    End If
End Sub

Function QueryKmdfDriverInfo(ByVal DriverObject As ULONG64, ByRef Info As KMDF_DRIVER_INFO) As Boolean
    Dim request As KMDF_DRIVER_INFO_REQUEST
    Dim bytesRet As DWORD

    memset(@Info, 0, SizeOf(Info))
    If DriverObject < &HFFFF800000000000ULL Then Return False
    If hDrv = NULL OrElse hDrv = INVALID_HANDLE_VALUE Then Return False

    EnsureKmdfSymbolLayoutAsync
    If Not g_KmdfSymbolLayoutReady Then Return False

    request.DriverObject = DriverObject
    If IoControl(hDrv, IOCTL_QueryKmdfDriverInfo, @request, SizeOf(request), _
        @Info, SizeOf(Info), @bytesRet) = 0 Then
        MyLog.PrintWin32Error "QueryKmdfDriverInfo", "IOCTL_QueryKmdfDriverInfo"
        memset(@Info, 0, SizeOf(Info))
        Return False
    End If

    If bytesRet <> SizeOf(Info) OrElse Info.IsKmdf <> 1 OrElse _
       Info.DispatchPolicyKnown > 1 OrElse _
       (Info.DispatchPolicyKnown = 0 AndAlso Info.DispatchPolicy <> KmdfDispatchPolicyUnknown) OrElse _
       (Info.DispatchPolicyKnown = 1 AndAlso _
        Info.DispatchPolicy <> KmdfDispatchPolicyAllDispatch AndAlso _
        Info.DispatchPolicy <> KmdfDispatchPolicyAllDispatchWithLock AndAlso _
        Info.DispatchPolicy <> KmdfDispatchPolicyMixed) Then
        MyLog.PrintLog LOG_WARN, "QueryKmdfDriverInfo",, _
            "[KMDF] invalid driver info response, bytes=" & Str(bytesRet)
        memset(@Info, 0, SizeOf(Info))
        Return False
    End If

    Return True
End Function

Function KmdfCallbackIdToString(ByVal Id As KMDF_CALLBACK_ID) As String
    Select Case Id
        Case KmdfEvtDriverDeviceAdd
            Return "EvtDriverDeviceAdd"
        Case KmdfEvtDriverUnload
            Return "EvtDriverUnload"

        Case KmdfEvtDeviceUsageNotification
            Return "EvtDeviceUsageNotification"
        Case KmdfEvtDeviceUsageNotificationEx
            Return "EvtDeviceUsageNotificationEx"
        Case KmdfEvtDeviceRelationsQuery
            Return "EvtDeviceRelationsQuery"

        Case KmdfEvtDeviceD0Entry
            Return "EvtDeviceD0Entry"
        Case KmdfEvtDeviceD0EntryPostInterruptsEnabled
            Return "EvtDeviceD0EntryPostInterruptsEnabled"
        Case KmdfEvtDeviceD0ExitPreInterruptsDisabled
            Return "EvtDeviceD0ExitPreInterruptsDisabled"
        Case KmdfEvtDeviceD0Exit
            Return "EvtDeviceD0Exit"

        Case KmdfEvtDevicePrepareHardware
            Return "EvtDevicePrepareHardware"
        Case KmdfEvtDeviceReleaseHardware
            Return "EvtDeviceReleaseHardware"
        Case KmdfEvtDeviceQueryStop
            Return "EvtDeviceQueryStop"
        Case KmdfEvtDeviceQueryRemove
            Return "EvtDeviceQueryRemove"
        Case KmdfEvtDeviceSurpriseRemoval
            Return "EvtDeviceSurpriseRemoval"

        Case KmdfEvtDeviceSelfManagedIoCleanup
            Return "EvtDeviceSelfManagedIoCleanup"
        Case KmdfEvtDeviceSelfManagedIoFlush
            Return "EvtDeviceSelfManagedIoFlush"
        Case KmdfEvtDeviceSelfManagedIoInit
            Return "EvtDeviceSelfManagedIoInit"
        Case KmdfEvtDeviceSelfManagedIoSuspend
            Return "EvtDeviceSelfManagedIoSuspend"
        Case KmdfEvtDeviceSelfManagedIoRestart
            Return "EvtDeviceSelfManagedIoRestart"
            
        Case KmdfEvtDeviceFileCreate
            Return "EvtDeviceFileCreate"
        Case KmdfEvtFileCleanup
            Return "EvtFileCleanup"
        Case KmdfEvtFileClose
            Return "EvtFileClose"
    End Select

    Return "Unknown"
End Function

Function KmdfContainerToString(ByVal ContainerType As KMDF_CALLBACK_CONTAINER) As String
    Select Case ContainerType
        Case KmdfContainerFxDriver
            Return "FxDriver"
        Case KmdfContainerFxPkgPnp
            Return "FxPkgPnp"
        Case KmdfContainerFxSelfManagedIoMachine
            Return "FxSelfManagedIo"
        Case KmdfContainerFxFileObjectInfo
            Return "FxFileObjectInfo"
    End Select
    Return "Unknown"
End Function

Function GetKmdfCurrentCallbacks(lvwKmdf As Class_ListView, ByVal DeviceObject As ULongInt) As Boolean
    Dim req As KMDF_CALLBACK_ENUM_REQUEST
    Dim result As KMDF_CALLBACK_ENUM_RESULT
    Dim bytesRet As DWORD
    Dim NewIndex As Long
    
    EnsureKmdfSymbolLayoutAsync

    InitListViewData lvwKmdf

    req.DeviceObject = DeviceObject

    SetLastError 0
    If IoControl(hDrv, IOCTL_EnumKmdfCurrentCallbacks, @req, SizeOf(req), @result, SizeOf(result), @bytesRet) = 0 Then
        MyLog.PrintWin32Error "GetKmdfCurrentCallbacks", "IOCTL_EnumKmdfCurrentCallbacks", "DeviceObject=0x" & WHex(DeviceObject)
        CommitListViewView lvwKmdf
        Return False
    End If
    
    MyLog.PrintInfo "GetKmdfCurrentCallbacks",, _
    "bytesRet=" & bytesRet & _
    " Count=" & result.Count & _
    " Source=" & result.OffsetSource & _
    " FxDevice=0x" & Hex(result.FxDevice) & _
    " FxDriver=0x" & Hex(result.FxDriver) & _
    " FxPkgPnp=0x" & Hex(result.FxPkgPnp) & _
    " FxSelfManaged=0x" & Hex(result.FxSelfManagedIoMachine)

    If result.Count > KMDF_MAX_CALLBACKS Then
        MyLog.PrintLog LOG_ERROR, "GetKmdfCurrentCallbacks",, "返回数量异常:" & WStr(result.Count)
        CommitListViewView lvwKmdf
        Return False
    End If
    
    If result.Count = 0 Then
        MyLog.PrintLog LOG_ERROR, "GetKmdfCurrentCallbacks",, "无返回"
        CommitListViewView lvwKmdf
        Return False
    End If

    For i As ULong = 0 To result.Count - 1
        Dim CallbackName As String = KmdfCallbackIdToString(result.Callbacks(i).Id)
        Dim CallbackAddr As ULongInt = result.Callbacks(i).Address
        Dim ContainerObject As ULongInt = result.Callbacks(i).ContainerObject
        Dim DriverPath As StringW = ""

        If CallbackAddr <> 0 Then
            DriverPath = GetDriverPathByAddr(CallbackAddr)
        End If

        NewIndex = AddItemEx(lvwKmdf, CallbackName)
        SetItemTextEx lvwKmdf, NewIndex, 1, "0x" & WHex(CallbackAddr)
        SetItemTextEx lvwKmdf, NewIndex, 2, DriverPath
        SetItemTextEx lvwKmdf, NewIndex, 3, KmdfContainerToString(result.Callbacks(i).ContainerType)
        SetItemTextEx lvwKmdf, NewIndex, 4, "0x" & WHex(ContainerObject)
    Next

    If (result.OffsetSource And &H2) <> 0 Then
        MyLog.PrintLog LOG_WARN, "GetKmdfCurrentCallbacks",, "部分 KMDF Offset 使用硬编码回退"
    ElseIf (result.OffsetSource And &H1) <> 0 Then
        MyLog.PrintInfo "GetKmdfCurrentCallbacks",, "KMDF Offset 全部来自符号解析"
    End If
    
    CommitListViewView lvwKmdf

    Return True
End Function

' 线程工作函数 - 扫描内核内存，将结果写入共享数组
Sub KernelMemoryScanThread(ByVal userdata As Any Ptr)
    Dim p As ScanThreadParam Ptr = Cast(ScanThreadParam Ptr, userdata)
    Dim stMemory As MemoryStruct
    Dim result As MEMORY_SCAN_RESULT
    Dim localScanCount As UInteger = 0
    
    For Addr As UInteger = p->StartAddr To p->EndAddr Step PAGE_SIZE
        stMemory.Addr = Cast(PVOID, Addr)
        stMemory.dwSize = PAGE_SIZE
        
        If IoControl(hDrv, IOCTL_ScanKernelMemory, @stMemory, SizeOf(MemoryStruct), @result, SizeOf(MEMORY_SCAN_RESULT)) <> 0 Then
            If result <> Normal Then
                Dim DriverPath As StringW = GetDriverPathByAddr(Addr)
                If Addr = Cast(UInteger, &HFFFFF80267140000) Then DriverPath = "C:\Users\21607\Desktop\TestHideDriver1.sys"
                MutexLock(p->hMutex)
                
                ' 动态扩展数组
                Dim currentCount As Integer = g_ScanResultCount
                g_ScanResultCount += 1
                ReDim Preserve g_ScanResults(g_ScanResultCount - 1)
                
                g_ScanResults(currentCount).BaseAddr = Addr
                g_ScanResults(currentCount).result = result
                
                If result = HideDriver AndAlso LenW(DriverPath) > 0 Then
                    g_ScanResults(currentCount).DriverPath = DriverPath
                    Dim lastSlash As Integer = InStrRev(DriverPath, "\")
                    If lastSlash > 0 Then
                        g_ScanResults(currentCount).DriverName = Mid(DriverPath, lastSlash + 1)
                    Else
                        g_ScanResults(currentCount).DriverName = DriverPath
                    End If
                Else
                    g_ScanResults(currentCount).DriverName = GetMemoryScanType(result)
                    g_ScanResults(currentCount).DriverPath = ""
                End If
                
                MutexUnlock(p->hMutex)
            End If
        End If
        
        MutexLock(p->hMutex)
        *p->ScannedPages += 1
        MutexUnlock(p->hMutex)
        
        localScanCount += 1
        If (localScanCount Mod 20) = 0 Then
            FF_DoEvents
        End If
    Next
    
    MyLog.PrintInfo "KernelMemoryScanThread",, "扫描起始地址0x" & WHex(p->StartAddr) & "的线程退出"
    p->bFinish = True
End Sub

' 多线程扫描内核内存，填充结构体数组
Function GetKernelModuleList1(modules() As KernelModuleInfo) As Integer
    Dim threads(NUM_THREADS - 1) As Any Ptr
    Dim pParam As ScanThreadParam Ptr
    Dim hMutex As Any Ptr
    Dim totalPages As UInteger
    Dim scannedPages As UInteger = 0
    Dim i As Integer
    
    Dim ModuleSize As ULong
    Dim EndAddr As UInteger = &HFFFFF80280000000 ' Cast(UInteger, GetKernelModuleBase("ntoskrnl.exe", ModuleSize))
    Dim StartAddr As UInteger = &HFFFFF80260000000 ' EndAddr - &H70000000
    
    ' 初始化全局结果数组
    g_ScanResultCount = 0
    ReDim g_ScanResults(0)
    
    ' 计算总页数
    totalPages = ((EndAddr - StartAddr) \ PAGE_SIZE) + 1
    
    ' 创建互斥锁
    hMutex = MutexCreate()
    If hMutex = 0 Then
        MyLog.PrintWin32Error "GetKernelModuleList1", "MutexCreate"
        Return 0
    End If
    
    pParam = CAllocate(Len(ScanThreadParam) * NUM_THREADS)
    
    ' 创建并启动线程
    For i = 0 To NUM_THREADS - 1
        Dim addrRange As UInteger = (EndAddr - StartAddr) \ NUM_THREADS
        pParam[i].StartAddr = StartAddr + i * addrRange
        
        If i = NUM_THREADS - 1 Then
            pParam[i].EndAddr = EndAddr
        Else
            pParam[i].EndAddr = pParam[i].StartAddr + addrRange - 1
        End If
        
        ' 确保地址是PAGE_SIZE对齐
        pParam[i].StartAddr = (pParam[i].StartAddr \ PAGE_SIZE) * PAGE_SIZE
        pParam[i].EndAddr = ((pParam[i].EndAddr + PAGE_SIZE - 1) \ PAGE_SIZE) * PAGE_SIZE
        
        pParam[i].hMutex = hMutex
        pParam[i].scannedPages = @scannedPages
        pParam[i].bFinish = False
        
        Threads(i) = ThreadCreate(@KernelMemoryScanThread, @pParam[i])
        If threads(i) = 0 Then
            MyLog.PrintLog LOG_WARN, "ThreadCreate",, "线程 " & i & " 创建失败"
        End If
    Next
    
    ' 等待线程完成
    Do While True
        Dim activeThreadCount As Integer = NUM_THREADS
        For i As Integer = 0 To NUM_THREADS - 1
            If pParam[i].bFinish = True Then activeThreadCount -= 1
        Next
        If activeThreadCount = 0 Then Exit Do
        FF_DoEvents
    Loop
    
    ' 清理
    MutexDestroy(hMutex)
    Deallocate(pParam)
    
    ' 复制结果到调用者数组
    If g_ScanResultCount > 0 Then
        ReDim modules(g_ScanResultCount - 1)
        For i = 0 To g_ScanResultCount - 1
            modules(i) = g_ScanResults(i)
        Next
    End If
    
    Dim returnCount As Integer = g_ScanResultCount
    
    ' 清理全局数组
    Erase g_ScanResults
    g_ScanResultCount = 0
    
    Return returnCount
End Function

' 在官方模块数组中查找指定基地址（线性查找）
Private Function IsAddrInOfficialModules(ByVal baseAddr As UInteger, officialBases() As UInteger, officialCount As Integer) As Boolean
    For i As Integer = 0 To officialCount - 1
        If officialBases(i) = baseAddr Then
            Return True
        End If
    Next
    Return False
End Function

' 主函数：获取内核模块列表，可选检测隐藏模块
Public Sub GetKernelModuleList(lvwKernelModule As Class_ListView, ByVal bCheckHideKernelModule As Boolean = False)
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    
    If buffer = NULL Then
        MyLog.PrintLog LOG_ERROR, "GetKernelModuleList", "Allocate", "bufferSize=" & bufferSize
        Exit Sub
    End If
    
    InitListViewData lvwKernelModule
    'ListView_SetImageList(lvwKernelModule.hWnd, NULL, LVSIL_SMALL)
    
    ' 如果需要检测隐藏模块，先执行扫描
    Dim scanModules() As KernelModuleInfo
    Dim scanCount As Integer = 0
    
    If bCheckHideKernelModule Then
        If Not IsDriverLoaded Then
            If ShowMsgBox("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES Then
                If InitDriver Then ShowInfoBox "加载成功!" Else ShowErrorBox "加载失败!" : Return
            Else
                Return
            End If
        End If
        scanCount = GetKernelModuleList1(scanModules())
    End If
    
    ' 第一次调用获取缓冲区大小
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If st = STATUS_INFO_LENGTH_MISMATCH Then
        buffer = Reallocate(buffer, bufferSize)
        If buffer = NULL Then
            MyLog.PrintLog LOG_ERROR, "GetKernelModuleList", "Reallocate", "bufferSize=" & bufferSize
            Exit Sub
        End If
    End If
    
    ' 第二次调用获取实际数据
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If st < 0 Then
        MyLog.PrintNtError "GetKernelModuleList", "ZwQuerySystemInformation(SystemModuleInformation)", st
        Deallocate buffer
        Exit Sub
    End If
    
    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    Dim officialCount As Integer = smi->Count
    
    ' 提取官方模块基地址数组，用于后续对比
    Dim officialBases(0 To officialCount - 1) As UInteger
    For i As Integer = 0 To officialCount - 1
        officialBases(i) = Cast(UInteger, smi->Modules(i).dwBase)
    Next
    
    ' 先添加官方模块列表
    For i As Integer = 0 To officialCount - 1
        Dim DriverName As WString * 260
        Dim DriverPath As ZString * MAX_PATH = smi->Modules(i).ImageName
        Dim DriverObject As Integer
        
        DriverPath = GetCorrectPath(DriverPath)
        DriverName = Right(DriverPath, Len(DriverPath) - InStrRev(DriverPath, "\"))
        If IsDriverLoaded Then DriverObject = GetDriverObjectAddr(smi->Modules(i).dwBase)
        
        Dim ItemIndex As Long = AddItemColListEx(lvwKernelModule, 6,,, WStr(i), WStr(DriverName), _
        "0x" & WHex(smi->Modules(i).dwBase), "0x" & WHex(smi->Modules(i).dwSize), _
        WStr(DriverPath), "0x" & WHex(DriverObject))
        'Dim IconIndex As Long = GetFileIconIndexCached(DriverPath, False)
        'Print "IconIndex:" & IconIndex
        SetItemIconEx lvwKernelModule, ItemIndex, ICON_SYSTEM_COMPONENT
        'SetItemIconEx lvwKernelModule, ItemIndex, 188 ' ICON_SYSTEM_COMPONENT
        VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", ItemIndex, DriverPath
        FF_DoEvents
    Next
    
    ' 如果启用隐藏检测，添加扫描发现但不在官方列表中的模块
    If bCheckHideKernelModule Then
        Dim addedCount As Integer = 0
        For i As Integer = 0 To scanCount - 1
            Dim isHidden As Boolean = False
            
            Select Case scanModules(i).result
                Case HideDriver
                    ' HideDriver需要确认是否不在官方列表中
                    If Not IsAddrInOfficialModules(scanModules(i).BaseAddr, officialBases(), officialCount) Then
                        isHidden = True
                    End If
                Case ShellCode, UnknownCode
                    ' ShellCode和UnknownCode直接判定为可疑
                    isHidden = True
            End Select
            
            If isHidden Then
                Dim ItemIndex As Long = AddItemColListEx(lvwKernelModule, 6,,, _
                    WStr(officialCount + addedCount), _
                    WStr(scanModules(i).DriverName), _
                    "0x" & WHex(scanModules(i).BaseAddr), _
                    "0x" & WHex(PAGE_SIZE), _
                    WStr(scanModules(i).DriverPath), _
                    "0x0")
                
                SetItemColor lvwKernelModule, ItemIndex, FB_Red, FB_White
                addedCount += 1
            End If
        Next
    End If
    
    Deallocate buffer
    CommitListViewView lvwKernelModule
End Sub

Function GetKernelModuleBase(ByVal ModuleName As WString Ptr, ByRef ModuleSize As ULong) As PVOID
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then Return NULL
    
    ' 第一次获取缓冲区大小
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then Return NULL
    End If
    
    ' 第二次获取实际数据
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        Deallocate buffer
        Return NULL
    End If
    
    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    
    Dim targetStr As String = "\" & wStrToStr(ModuleName)  ' 前置反斜杠
    For i As Integer = 0 To smi->Count - 1
        If InStr(UCase(smi->Modules(i).ImageName), UCase(targetStr)) > 0 Then
            ModuleSize = smi->Modules(i).dwSize
            Dim baseAddr As PVOID = smi->Modules(i).dwBase
            Deallocate buffer
            Return baseAddr
        End If
    Next
    
    Deallocate buffer
    Return NULL
End Function

Function GetKernelModulePath(ByVal ModuleName As WString Ptr, ByVal OutPath As WString Ptr) As Boolean
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    
    If (buffer = NULL) Then
        MyLog.PrintLog LOG_ERROR, "GetKernelModulePath", "Allocate", "bufferSize=" & bufferSize
        Return False
    End If

    ' First call to get required buffer size
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            MyLog.PrintLog LOG_ERROR, "GetKernelModulePath", "Reallocate", "bufferSize=" & bufferSize
            Return False
        End If
    End If

    ' Second call to get actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        MyLog.PrintNtError "GetKernelModulePath", "ZwQuerySystemInformation(SystemModuleInformation)", st
        Deallocate buffer
        Return False
    End If

    ' 解析模块信息
    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)

    ' 构建前缀匹配字符串：\ + 文件名
    Dim target As StringW
    target = "\" & *ModuleName
    Dim found As Boolean = False

    For i As Integer = 0 To smi->Count - 1
        ' 前缀匹配，大小写不敏感
        If InStr(UCase(smi->Modules(i).ImageName), UCase(wStrToStr(target))) > 0 Then
            ' 找到模块，将 ANSI ImageName 转成 Unicode
            *OutPath = smi->Modules(i).ImageName
            found = True
            Exit For
        End If
    Next

    Deallocate buffer
    Return found
End Function

Function GetKernelProcAddress(ModuleName As StringW, FunctionName As StringW) As ULONG_PTR
    ' 1. 这里替换为你通过R3获取的 ntoskrnl.exe 内核基址
    Dim As ULONG_PTR ModuleBase = Cast(ULONG_PTR, GetKernelModuleBase(ModuleName.WstrPtr, NULL))
    If ModuleBase = NULL Then Return 0
    'Print "ModuleBase:0x" & WHex(ModuleBase)
    ' 2. 获取函数VA
    Return GetProcVA(ModuleName, FunctionName, ModuleBase)
End Function

Public Function GetWString(ByVal SourceString As String) As LPWSTR
    Dim pStr As LPWSTR = Allocate(Len(SourceString) + 1)
    *pStr = StrToWStr(SourceString)
    Return pStr
End Function

Function GetDriverPathByAddr(ByVal Addr As UInteger) As String
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        MyLog.PrintLog LOG_ERROR, "GetDriverPathByAddr", "Allocate", "bufferSize=" & bufferSize
        Return "-"
    End If

    'First Call To Get the required buffer SIZE
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            MyLog.PrintLog LOG_ERROR, "GetDriverPathByAddr", "Reallocate", "bufferSize=" & bufferSize
            Return "-"
        End If
    End If

    'Second call to get the actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        MyLog.PrintNtError "GetDriverPathByAddr", "ZwQuerySystemInformation(SystemModuleInformation)", st
        Deallocate buffer
        Return "-"
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    For i As Integer = 0 To smi->Count - 1
        If (Cast(UInteger, smi->Modules(i).dwBase) <= Addr And Cast(UInteger, smi->Modules(i).dwBase + smi->Modules(i).dwSize) >= Addr) Then
            Dim ImageName As String = ""
            ImageName = GetCorrectPath(smi->Modules(i).ImageName)
            Deallocate buffer
            Return ImageName
        End If
    Next
    Deallocate buffer
    Return "-"
End Function

Function GetGDT(lvwGDT As Class_mCtrlTreeList) As ULong
    Dim GDTArray() As GDT_INFO
    
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    SetLastError 0
    If IoControl(hDrv, IOCTL_GetGDT, NULL, 0, NULL, 0, @lpRet) = 0 Then
        MyLog.PrintWin32Error "GetGDT", "IOCTL_GetGDT", "第一次调用"
        Return 0
    End If
    
    ReDim GDTArray(lpRet / SizeOf(GDT_INFO) - 1) As GDT_INFO
    IoControl hDrv, IOCTL_GetGDT, NULL, 0, @GDTArray(0), (UBound(GDTArray) + 1) * SizeOf(GDT_INFO), @lpRet
    ' ====== CLI 旁路输出 ======
    If g_CuiMode Then
        Dim colNames(9) As StringW = { _
            "CPU", "GDT Entry/Base", "Limit", "SEL", "SEL Base", _
            "SEL Limit", "DPL", "Present", "Granularity", "Description" _
        }
        
        Dim maxRows As Long = (UBound(GDTArray) + 1) * (1 + 11)
        Dim rows(maxRows - 1) As CuiExportRow
        Dim rowIdx As Long = 0
        
        For i As Integer = 0 To UBound(GDTArray)
            ' --- 根行：CPU #N (indent=0) ---
            rows(rowIdx).IndentLevel = 0
            rows(rowIdx).IsGroup = True
            rows(rowIdx).GroupLabel = "CPU #" & i
            rows(rowIdx).ColCount = 10
            rows(rowIdx).Cols(0) = "CPU #" & i
            rows(rowIdx).Cols(1) = "0x" & WHex(GDTArray(i).dwBase)
            rows(rowIdx).Cols(2) = "0x" & WHex(GDTArray(i).Limit)
            ' Cols(3)~Cols(9) 自动为空字符串
            rowIdx += 1
            
            ' --- 子行：GDT 条目 (indent=1) ---
            For j As Integer = 0 To 10
                Dim item As GDT_ITEM Ptr = @GDTArray(i).Item(j)
                If item->Limit = 0 AndAlso item->SEL_Base = 0 Then Continue For
                
                Dim Description As StringW
                Dim SystemTypes(15) As StringW = { _
                    "Reserved", "16-bit TSS (Available)", "LDT", "16-bit TSS (Busy)", _
                    "16-bit Call Gate", "Task Gate", "16-bit Interrupt Gate", "16-bit Trap Gate", _
                    "Reserved", "32-bit TSS (Available)", "Reserved", "32-bit TSS (Busy)", _
                    "32-bit Call Gate", "Reserved", "32-bit Interrupt Gate", "32-bit Trap Gate" _
                }
                If item->S = 0 Then
                    Description = "System segment(" & SystemTypes(item->uType) & ")"
                Else
                    If item->uType And &H8 Then
                        Description = "Code Segment"
                    Else
                        Description = "Data Segment"
                    End If
                End If
                
                rows(rowIdx).IndentLevel = 1
                rows(rowIdx).IsGroup = False
                rows(rowIdx).ColCount = 10
                rows(rowIdx).Cols(0) = "Selector 0x" & WHex(j * 8)
                rows(rowIdx).Cols(1) = "0x" & WHex(GDTArray(i).dwBase + j * 8)
                rows(rowIdx).Cols(2) = "0x" & WHex(item->Limit)
                rows(rowIdx).Cols(3) = "0x" & WHex(j * 8)
                rows(rowIdx).Cols(4) = "0x" & WHex(item->SEL_Base)
                rows(rowIdx).Cols(5) = "0x" & WHex(item->SEL_Limit)
                rows(rowIdx).Cols(6) = WStr(item->DPL)
                rows(rowIdx).Cols(7) = WStr(item->Present)
                rows(rowIdx).Cols(8) = IIf(item->Granularity = 1, "Page", "Byte")
                rows(rowIdx).Cols(9) = Description
                rowIdx += 1
            Next
        Next
        
        If rowIdx > 0 Then
            ReDim Preserve rows(rowIdx - 1) As CuiExportRow
            CuiPrintTreeListData("GDT", rows(), colNames(), 10)
        Else
            Dim emptyRows() As CuiExportRow
            ReDim emptyRows(-1) As CuiExportRow
            CuiPrintTreeListData("GDT", emptyRows(), colNames(), 10)
        End If
        Return 0
    End If
    ' ==========================
    lvwGDT.DeleteItem MC_TLI_ROOT
    Dim hGroup As MC_HTREELISTITEM, hItem As MC_HTREELISTITEM
    For i As Integer = 0 To UBound(GDTArray)
        hGroup = lvwGDT.AddItem(MC_TLI_ROOT, "CPU #" & i)
        lvwGDT.SetItemText hGroup, 1, "0x" & WHex(GDTArray(i).dwBase)
        lvwGDT.SetItemText hGroup, 2, "0x" & WHex(GDTArray(i).Limit)
        For j As Integer = 0 To 10
            'Print "0x" & WHex(GDTArray(i).Item(j).pGDTDescriptor)
            hItem = lvwGDT.AddItem(hGroup, "")
            lvwGDT.SetItemText hItem, 1, "0x" & WHex(GDTArray(i).dwBase + j * 8)
            lvwGDT.SetItemText hItem, 2, "0x" & WHex(GDTArray(i).Item(j).Limit)
            lvwGDT.SetItemText hItem, 3, "0x" & WHex(j * 8)
            lvwGDT.SetItemText hItem, 4, "0x" & WHex(GDTArray(i).Item(j).SEL_Base)
            lvwGDT.SetItemText hItem, 5, "0x" & WHex(GDTArray(i).Item(j).SEL_Limit)
            lvwGDT.SetItemText hItem, 6, WStr(GDTArray(i).Item(j).DPL)
            lvwGDT.SetItemText hItem, 7, WStr(GDTArray(i).Item(j).Present)
            lvwGDT.SetItemText hItem, 8, IIf(GDTArray(i).Item(j).Granularity = 1, "Page", "Byte")
            
            Dim Description As StringW
            Dim SystemTypes(15) As StringW = {"Reserved", "16-bit TSS (Available)", "LDT", "16-bit TSS (Busy)", _
                        "16-bit Call Gate", "Task Gate", "16-bit Interrupt Gate", "16-bit Trap Gate", _
                        "Reserved", "32-bit TSS (Available)", "Reserved", "32-bit TSS (Busy)", _
                        "32-bit Call Gate", "Reserved", "32-bit Interrupt Gate", "32-bit Trap Gate"}
            If GDTArray(i).Item(j).S = 0 Then
                Description = "System segment(" & SystemTypes(GDTArray(i).Item(j).uType)  & ")"
            Else
                If GDTArray(i).Item(j).uType And &H8 Then
                    Description = "Code Segment"
                    /'DbgPrint("Attr: ")
                    DBGPRINT("%s", pGdtEntry[j].HighWord.Bits.Type & 0x4 ? "C" : "-")
                    DbgPrint("%s", pGdtEntry[j].HighWord.Bits.Type & 0x2 ? "R" : "-")
                    DBGPRINT("%s", pGdtEntry[j].HighWord.Bits.Type & 0x1 ? "A" : "-")
                    DBGPRINT("\n")'/
                Else
                    Description = "Data Segment"
                    /'DbgPrint("Attr: ")
                    DBGPRINT("%s", pGdtEntry[j].HighWord.Bits.Type & 0x4 ? "E" : "-")
                    DbgPrint("%s", pGdtEntry[j].HighWord.Bits.Type & 0x2 ? "W" : "-")
                    DbgPrint("%s", pGdtEntry[j].HighWord.Bits.Type & 0x1 ? "A" : "-")
                    DBGPRINT("\n")'/
                End If
            End If
            lvwGDT.SetItemText hItem, 9, Description
        Next
        FF_DoEvents
    Next
    Return 0
End Function

Function GetIDT(lvwIDT As Class_mCtrlTreeList) As ULong
    Dim idtHandlerNames(255) As String
    For i As Integer = 0 To 255
        idtHandlerNames(i) = "Unknown Function:" & i
    Next
    idtHandlerNames(0) = "Division Error #DE"
    idtHandlerNames(1) = "Debug #DB" 
    idtHandlerNames(2) = "NMI"
    idtHandlerNames(3) = "Breakpoint #BP"
    idtHandlerNames(4) = "Overflow #OF"
    idtHandlerNames(5) = "Bound Error #BR"
    idtHandlerNames(6) = "Invalid Opcode #UD"
    idtHandlerNames(7) = "Device Not Available #NM"
    idtHandlerNames(8) = "Double Fault #DF"
    idtHandlerNames(9) = "Segment Overrun"
    idtHandlerNames(10) = "Invalid TSS #TS"
    idtHandlerNames(11) = "Segment Not Present #NP"
    idtHandlerNames(12) = "Stack-Segment Fault #SS"
    idtHandlerNames(13) = "GP Fault #GP"
    idtHandlerNames(14) = "Page Fault #PF"
    idtHandlerNames(15) = "Reserved"
    idtHandlerNames(16) = "x87 Floating-Point Exception #MF"
    idtHandlerNames(17) = "Alignment Check #AC"
    idtHandlerNames(18) = "Machine Check #MC"
    idtHandlerNames(19) = "SIMD Floating-Point Exception #XM/#XF"
    idtHandlerNames(20) = "Virtualization Exception #VE"
    idtHandlerNames(21) = "Control Protection Exception #CP"
    idtHandlerNames(22) = "Reserved"
    idtHandlerNames(23) = "Reserved"
    idtHandlerNames(24) = "Reserved"
    idtHandlerNames(25) = "Reserved"
    idtHandlerNames(26) = "Reserved"
    idtHandlerNames(27) = "Reserved"
    idtHandlerNames(28) = "Hypervisor Injection Exception #HV"
    idtHandlerNames(29) = "VMM Communication Exception #VC"
    idtHandlerNames(30) = "Security Exception #SX"
    
    ' Windows特定的中断处理程序
    idtHandlerNames(31) = "KiApcInterrupt"
    idtHandlerNames(32) = "KiSwInterrupt"
    idtHandlerNames(44) = "KiRaiseAssertion"
    idtHandlerNames(45) = "KiDebugServiceTrap"
    idtHandlerNames(46) = "KiSystemService"
    idtHandlerNames(47) = "KiDpcInterrupt"
    
    Dim IDTArray() As IDT_INFO
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    SetLastError 0
    If IoControl(hDrv, IOCTL_GetIDT, NULL, 0, NULL, 0, @lpRet) = 0 Then
        MyLog.PrintWin32Error "GetIDT", "IOCTL_GetIDT", "第一次调用"
        Return 0
    End If
    
    ReDim IDTArray(lpRet / SizeOf(IDT_INFO) - 1) As IDT_INFO
    IoControl hDrv, IOCTL_GetIDT, NULL, 0, @IDTArray(0), (UBound(IDTArray) + 1) * SizeOf(IDT_INFO), @lpRet
    ' ====== CLI 旁路输出 ======
    If g_CuiMode Then
        Dim colNames(6) As StringW = { _
            "CPU", "ID", "函数名称", "当前函数地址", "SEL", "DPL", "所属模块" _
        }
        
        Dim maxRows As Long = (UBound(IDTArray) + 1) * (1 + 256)
        Dim rows(maxRows - 1) As CuiExportRow
        Dim rowIdx As Long = 0
        
        For i As Integer = 0 To UBound(IDTArray)
            ' --- 根行：CPU #N (indent=0) ---
            rows(rowIdx).IndentLevel = 0
            rows(rowIdx).IsGroup = True
            rows(rowIdx).GroupLabel = "CPU #" & i
            rows(rowIdx).ColCount = 7
            rows(rowIdx).Cols(0) = "CPU #" & i
            rowIdx += 1
            
            ' --- 子行：IDT 向量 (indent=1) ---
            For j As Integer = 0 To 255
                rows(rowIdx).IndentLevel = 1
                rows(rowIdx).IsGroup = False
                rows(rowIdx).ColCount = 7
                rows(rowIdx).Cols(0) = "Vector 0x" & WHex(j, 2)
                rows(rowIdx).Cols(1) = "0x" & WHex(j)
                rows(rowIdx).Cols(2) = idtHandlerNames(j)
                rows(rowIdx).Cols(3) = "0x" & WHex(IDTArray(i).IdtEntries(j).HandlerFunc)
                rows(rowIdx).Cols(4) = "0x" & WHex(IDTArray(i).IdtEntries(j).Selector)
                rows(rowIdx).Cols(5) = WStr(IDTArray(i).IdtEntries(j).Attributes)
                rows(rowIdx).Cols(6) = GetDriverPathByAddr(IDTArray(i).IdtEntries(j).HandlerFunc)
                rowIdx += 1
            Next
        Next
        
        If rowIdx > 0 Then
            ReDim Preserve rows(rowIdx - 1) As CuiExportRow
            CuiPrintTreeListData("IDT", rows(), colNames(), 7)
        Else
            Dim emptyRows() As CuiExportRow
            ReDim emptyRows(-1) As CuiExportRow
            CuiPrintTreeListData("IDT", emptyRows(), colNames(), 7)
        End If
        Return 0
    End If
    
    lvwIDT.DeleteItem MC_TLI_ROOT
    Dim hGroup As MC_HTREELISTITEM, hItem As MC_HTREELISTITEM
    For i As Integer = 0 To UBound(IDTArray)
        hGroup = lvwIDT.AddItem(MC_TLI_ROOT, "CPU #" & i)
        For j As Integer = 0 To 255
            hItem = lvwIDT.AddItem(hGroup, "")
            lvwIDT.SetItemText hItem, 1, "0x" & WHex(j)
            lvwIDT.SetItemText hItem, 2, WStr(idtHandlerNames(j))
            lvwIDT.SetItemText hItem, 3, "0x" & WHex(IDTArray(i).IdtEntries(j).HandlerFunc)
            lvwIDT.SetItemText hItem, 4, "0x" & WHex(IDTArray(i).IdtEntries(j).Selector)
            lvwIDT.SetItemText hItem, 5, WStr(IDTArray(i).IdtEntries(j).Attributes)
            lvwIDT.SetItemText hItem, 6, GetDriverPathByAddr(IDTArray(i).IdtEntries(j).HandlerFunc)
        Next
        FF_DoEvents
    Next
    Return 0
End Function

Function GetSSDT(lvwSSDT As Class_ListView) As ULong
    Dim Count As DWORD = 0
    
    InitListViewData lvwSSDT
    
    Dim ntBase As UInteger, ntSize As ULong
    ntBase = Cast(UInteger, GetKernelModuleBase(StrPtrW("ntoskrnl.exe"), ntSize))
    Dim ntEnd As UInteger = ntBase + ntSize
    For Index As DWORD = 0 To 1000
        Dim SSDTFuncAddr As ULONG64
        If IoControl(hDrv, IOCTL_GetSSDTFuncByIndex, @Index, SizeOf(DWORD), @SSDTFuncAddr, SizeOf(ULONG64)) = 0 Then
            'Print "[GetSSDT]获取索引为" & Index & "的SSDT函数时失败:" & WinErrorMsg(GetLastError) & GetLastError
            If GetLastError = ERROR_NO_MORE_ITEMS Then Exit For
        Else
            Dim ItemIndex As Long = AddItemColListEx(lvwSSDT, 5,,, WStr(Count), WHex(Index), WStr(FindFunctionNameBySyscallIndex("ntdll.dll", Index)), _
            WStr("0x" & WHex(SSDTFuncAddr)), WStr(GetDriverPathByAddr(SSDTFuncAddr)))
            SetItemIconEx lvwSSDT, ItemIndex, ICON_SYSTEM_COMPONENT
            If SSDTFuncAddr < ntBase OrElse SSDTFuncAddr > ntEnd Then ' 被hook
                SetItemColor lvwSSDT, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        End If
        FF_DoEvents
    Next
    CommitListViewView lvwSSDT
    Return Count
End Function

Function GetSSSDT(lvwSSSDT As Class_ListView) As ULong
    Dim Count As DWORD = 0, Index As DWORD = 0
    
    InitListViewData lvwSSSDT
    
    Dim win32kBase As UInteger, win32kSize As ULong
    win32kBase = Cast(UInteger, GetKernelModuleBase(StrPtrW("win32k.sys"), win32kSize))
    Dim win32kEnd As UInteger = win32kBase + win32kSize
    Do While True
        Dim SSSDTFuncAddr As ULONG64
        If IoControl(hDrv, IOCTL_GetSSSDTFuncByIndex, @Index, SizeOf(DWORD), @SSSDTFuncAddr, SizeOf(ULONG64)) = 0 Then
            Exit Do
        Else
            'Print FindFunctionNameBySyscallIndex("win32u.dll", Index)
            Dim ItemIndex As Long = AddItemColListEx(lvwSSSDT, 5,,, WStr(Count), WHex(Index + &H1000), WStr(FindFunctionNameBySyscallIndex("win32u.dll", Index + &H1000)), _
            "0x" & WHex(SSSDTFuncAddr), WStr(GetDriverPathByAddr(SSSDTFuncAddr)))
            SetItemIconEx lvwSSSDT, ItemIndex, ICON_SYSTEM_COMPONENT
            If SSSDTFuncAddr < win32kBase OrElse SSSDTFuncAddr > win32kEnd Then ' 被hook
                SetItemColor lvwSSSDT, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        End If
        Index += 1
        FF_DoEvents
    Loop
    CommitListViewView lvwSSSDT
    Return Count
End Function

Function GetHalDispatchTable(lvwHalDistapch As Class_ListView) As ULong
    Dim Count As DWORD = 0
    
    InitListViewData lvwHalDistapch
    
    Dim HalDispatchName(12) As WString * 50 = { _
    "Version", _
    "HalQuerySystemInformation", _
    "HalSetSystemInformation", _
    "HalQueryBusSlots", _
    "HalDeviceControl", _
    "HalExamineMBR", _
    "HalIoAssignDriveLetters", _
    "HalIoReadPartitionTable", _
    "HalIoSetPartitionInformation", _
    "HalIoWritePartitionTable", _
    "HalReferenceHandlerForBus", _
    "HalReferenceBusHandler", _
    "HalDereferenceBusHandler" _
    }
    
    Dim ntBase As UInteger, ntSize As ULong
    ntBase = Cast(UInteger, GetKernelModuleBase(StrPtrW("ntoskrnl.exe"), ntSize))
    Dim ntEnd As UInteger = ntBase + ntSize
    For Index As ULong = 1 To 12
        Dim HalDispatchAddr As PVOID = NULL
        If IoControl(hDrv, IOCTL_GetHalDispatchByIndex, @Index, SizeOf(DWORD), @HalDispatchAddr, SizeOf(PVOID)) <> 0 Then
            'Print HalDispatchName(Index) & ":" & "0x" & WHex(HalDispatchAddr)
            Dim ItemIndex As Long = AddItemColListEx(lvwHalDistapch, 4,,, WStr(Index), HalDispatchName(Index), "0x" & WHex(HalDispatchAddr), WStr(GetDriverPathByAddr(Cast(ULONG64, HalDispatchAddr))))
            SetItemIconEx lvwHalDistapch, ItemIndex, ICON_SYSTEM_COMPONENT
            If HalDispatchAddr <> NULL AndAlso (HalDispatchAddr < ntBase OrElse HalDispatchAddr > ntEnd) Then ' 被hook
                SetItemColor lvwHalDistapch, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        Else
            MyLog.PrintLog LOG_WARN, "GetHalDispatchTable", "IOCTL_GetHalDispatchByIndex", "Index=" & Index
        End If
        FF_DoEvents
    Next
    CommitListViewView lvwHalDistapch
    Return Count
End Function

Function GetHalPrivateDispatchTable(lvwHalPrivateDistapch As Class_ListView) As ULong
    Dim Count As DWORD = 0
    
    InitListViewData lvwHalPrivateDistapch
    
    Dim HalPrivateDispatchName(149) As WString * 50 = { _
    "Version", _
    "HalHandlerForBus", _
    "HalHandlerForConfigSpace", _
    "HalLocateHiberRanges", _
    "HalRegisterBusHandler", _
    "HalSetWakeEnable", _
    "HalSetWakeAlarm", _
    "HalPciTranslateBusAddress", _
    "HalPciAssignSlotResources", _
    "HalHaltSystem", _
    "HalFindBusAddressTranslation", _
    "HalResetDisplay", _
    "HalAllocateMapRegisters", _
    "KdSetupPciDeviceForDebugging", _
    "KdReleasePciDeviceForDebugging", _
    "KdGetAcpiTablePhase0", _
    "KdCheckPowerButton", _
    "HalVectorToIDTEntry", _
    "KdMapPhysicalMemory64", _
    "KdUnmapVirtualAddress", _
    "KdGetPciDataByOffset", _
    "KdSetPciDataByOffset", _
    "HalGetInterruptVectorOverride", _
    "HalGetVectorInputOverride", _
    "HalLoadMicrocode", _
    "HalUnloadMicrocode", _
    "HalPostMicrocodeUpdate", _
    "HalAllocateMessageTargetOverride", _
    "HalFreeMessageTargetOverride", _
    "HalDpReplaceBegin", _
    "HalDpReplaceTarget", _
    "HalDpReplaceControl", _
    "HalDpReplaceEnd", _
    "HalPrepareForBugcheck", _
    "HalQueryWakeTime", _
    "HalReportIdleStateUsage", _
    "HalTscSynchronization", _
    "HalWheaInitProcessorGenericSection", _
    "HalStopLegacyUsbInterrupts", _
    "HalReadWheaPhysicalMemory", _
    "HalWriteWheaPhysicalMemory", _
    "HalDpMaskLevelTriggeredInterrupts", _
    "HalDpUnmaskLevelTriggeredInterrupts", _
    "HalDpGetInterruptReplayState", _
    "HalDpReplayInterrupts", _
    "HalQueryIoPortAccessSupported", _
    "KdSetupIntegratedDeviceForDebugging", _
    "KdReleaseIntegratedDeviceForDebugging", _
    "HalGetEnlightenmentInformation", _
    "HalAllocateEarlyPages", _
    "HalMapEarlyPages", _
    "Dummy1", _
    "Dummy2", _
    "HalNotifyProcessorFreeze", _
    "HalPrepareProcessorForIdle", _
    "HalRegisterLogRoutine", _
    "HalResumeProcessorFromIdle", _
    "Dummy", _
    "HalVectorToIDTEntryEx", _
    "HalSecondaryInterruptQueryPrimaryInformation", _
    "HalMaskInterrupt", _
    "HalUnmaskInterrupt", _
    "HalIsInterruptTypeSecondary", _
    "HalAllocateGsivForSecondaryInterrupt", _
    "HalAddInterruptRemapping", _
    "HalRemoveInterruptRemapping", _
    "HalSaveAndDisableHvEnlightenment", _
    "HalRestoreHvEnlightenment", _
    "HalFlushIoBuffersExternalCache", _
    "HalFlushExternalCache", _
    "HalPciEarlyRestore", _
    "HalGetProcessorId", _
    "HalAllocatePmcCounterSet", _
    "HalCollectPmcCounters", _
    "HalFreePmcCounterSet", _
    "HalProcessorHalt", _
    "HalTimerQueryCycleCounter", _
    "Dummy3", _
    "HalPciMarkHiberPhase", _
    "HalQueryProcessorRestartEntryPoint", _
    "HalRequestInterrupt", _
    "HalEnumerateUnmaskedInterrupts", _
    "HalFlushAndInvalidatePageExternalCache", _
    "KdEnumerateDebuggingDevices", _
    "HalFlushIoRectangleExternalCache", _
    "HalPowerEarlyRestore", _
    "HalQueryCapsuleCapabilities", _
    "HalUpdateCapsule", _
    "HalPciMultiStageResumeCapable", _
    "HalDmaFreeCrashDumpRegisters", _
    "HalAcpiAoacCapable", _
    "HalInterruptSetDestination", _
    "HalGetClockConfiguration", _
    "HalClockTimerActivate", _
    "HalClockTimerInitialize", _
    "HalClockTimerStop", _
    "HalClockTimerArm", _
    "HalTimerOnlyClockInterruptPending", _
    "HalAcpiGetMultiNode", _
    "HalPowerSetRebootHandler", _
    "HalIommuRegisterDispatchTable", _
    "HalTimerWatchdogStart", _
    "HalTimerWatchdogResetCountdown", _
    "HalTimerWatchdogStop", _
    "HalTimerWatchdogGeneratedLastReset", _
    "HalTimerWatchdogTriggerSystemReset", _
    "HalInterruptVectorDataToGsiv", _
    "HalInterruptGetHighestPriorityInterrupt", _
    "HalProcessorOn", _
    "HalProcessorOff", _
    "HalProcessorFreeze", _
    "HalDmaLinkDeviceObjectByToken", _
    "HalDmaCheckAdapterToken", _
    "Dummy4", _
    "HalTimerConvertPerformanceCounterToAuxiliaryCounter", _
    "HalTimerConvertAuxiliaryCounterToPerformanceCounter", _
    "HalTimerQueryAuxiliaryCounterFrequency", _
    "HalConnectThermalInterrupt", _
    "HalIsEFIRuntimeActive", _
    "HalTimerQueryAndResetRtcErrors", _
    "HalAcpiLateRestore", _
    "KdWatchdogDelayExpiration", _
    "HalGetProcessorStats", _
    "HalTimerWatchdogQueryDueTime", _
    "HalConnectSyntheticInterrupt", _
    "HalPreprocessNmi", _
    "HalEnumerateEnvironmentVariablesWithFilter", _
    "HalCaptureLastBranchRecordStack", _
    "HalClearLastBranchRecordStack", _
    "HalConfigureLastBranchRecord", _
    "HalGetLastBranchInformation", _
    "HalResumeLastBranchRecord", _
    "HalStartLastBranchRecord", _
    "HalStopLastBranchRecord", _
    "HalIommuBlockDevice", _
    "HalIommuUnblockDevice", _
    "HalGetIommuInterface", _
    "HalRequestGenericErrorRecovery", _
    "HalTimerQueryHostPerformanceCounter", _
    "HalTopologyQueryProcessorRelationships", _
    "HalInitPlatformDebugTriggers", _
    "HalRunPlatformDebugTriggers", _
    "HalTimerGetReferencePage", _
    "HalGetHiddenProcessorPowerInterface", _
    "HalGetHiddenProcessorPackageId", _
    "HalGetHiddenPackageProcessorCount", _
    "HalGetHiddenProcessorApicIdByIndex", _
    "HalRegisterHiddenProcessorIdleState", _
    "HalIommuReportIommuFault", _
    "HalIommuDmaRemappingCapable" _
    }
    
    Dim ntBase As UInteger, ntSize As ULong
    ntBase = Cast(UInteger, GetKernelModuleBase(StrPtrW("ntoskrnl.exe"), ntSize))
    Dim ntEnd As UInteger = ntBase + ntSize
    For Index As ULong = 1 To 149
        Dim HalPrivateDispatchAddr As PVOID = NULL
        If IoControl(hDrv, IOCTL_GetHalPrivateDispatchByIndex, @Index, SizeOf(DWORD), @HalPrivateDispatchAddr, SizeOf(ULONG64)) <> 0 Then
            'Print HalPrivateDispatchName(Index) & ":" & "0x" & WHex(HalPrivateDispatchAddr)
            Dim ItemIndex As Long = AddItemColListEx(lvwHalPrivateDistapch, 4,,, WStr(Index), HalPrivateDispatchName(Index), "0x" & WHex(HalPrivateDispatchAddr), WStr(GetDriverPathByAddr(Cast(ULONG64, HalPrivateDispatchAddr))))
            SetItemIconEx lvwHalPrivateDistapch, ItemIndex, ICON_SYSTEM_COMPONENT
            If HalPrivateDispatchAddr <> NULL AndAlso (HalPrivateDispatchAddr < ntBase OrElse HalPrivateDispatchAddr > ntEnd) Then ' 被hook
                SetItemColor lvwHalPrivateDistapch, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        Else
            MyLog.PrintLog LOG_WARN, "GetHalPrivateDispatchTable", "IOCTL_GetHalDispatchByIndex", "Index=" & Index
        End If
        FF_DoEvents
    Next
    CommitListViewView lvwHalPrivateDistapch
    Return Count
End Function

Sub GetObjectRootList(treObjectDirectory As Class_TreeView)
    treObjectDirectory.DeleteAllItems
    ' 插入根节点 "\"
    treObjectDirectory.AddItem TVI_ROOT, "\"
End Sub

Public Sub GetObjectList(CurrentNode As HTREEITEM, treObjectDirectory As Class_TreeView, lvwObject As Class_ListView, ByVal IsEnumSubDir As Boolean, IsEnumObject As Boolean = True)
#define STATUS_MORE_ENTRIES         Cast(NTSTATUS, &h00000105)
#define DIRECTORY_QUERY             &h0001
#define DIRECTORY_TRAVERSE          &h0002
#define SYMBOLIC_LINK_QUERY         &h0001
Type OBJECT_DIRECTORY_INFORMATION
    Name As UNICODE_STRING
    TypeName As UNICODE_STRING
End Type
    Dim szPath As StringW = ""
    Dim hNtDll As HMODULE
    Dim pNtOpenDirectoryObject As Function(ByRef hDirectory As HANDLE, ByVal AccessMask As ACCESS_MASK, ByRef ObjectAttributes As OBJECT_ATTRIBUTES) As NTSTATUS
    Dim pNtQueryDirectoryObject As Function(ByVal hDirectory As HANDLE, ByVal Buffer As PVOID, ByVal Length As ULONG, ByVal ReturnSingleEntry As BOOLEAN, ByVal RestartScan As BOOLEAN, ByRef Context As ULONG, ByRef ReturnLength As ULONG) As NTSTATUS
    Dim pNtOpenSymbolicLinkObject As Function(ByRef hLinkHandle As HANDLE, ByVal DesiredAccess As ACCESS_MASK, ByRef ObjectAttributes As OBJECT_ATTRIBUTES) As NTSTATUS
    Dim pNtQuerySymbolicLinkObject As Function(ByVal hLinkHandle As HANDLE, ByRef LinkTarget As UNICODE_STRING, ByRef ReturnedLength As ULONG) As NTSTATUS
    Dim pRtlInitUnicodeString As Sub(ByRef DestinationString As UNICODE_STRING, ByVal SourceString As PCWSTR)
    Dim pNtClose As Function(ByVal hObject As HANDLE) As NTSTATUS
    
    ' 初始化
    If IsEnumSubDir Then DeleteChildItem treObjectDirectory, CurrentNode
    InitListViewData lvwObject
    
    GetPathByNodeW CurrentNode, treObjectDirectory, szPath
    ' 对象目录路径格式如 "\" 或 "\Devices\"，需要去掉末尾的\
    szPath = RightW(szPath, LenW(szPath) - 1)
    If RightW(szPath, 1) = "\" Then szPath = LeftW(szPath, LenW(szPath) - 1)
    If szPath = "" Then szPath = "\"  ' 根目录
    
    ' 加载ntdll
    hNtDll = GetModuleHandleW("ntdll.dll")
    If hNtDll = NULL Then
        MyLog.PrintLog LOG_ERROR, "GetModuleHandleW", "无法加载ntdll.dll"
        Exit Sub
    End If
    
    pNtOpenDirectoryObject = Cast(PVOID, GetProcAddress(hNtDll, "NtOpenDirectoryObject"))
    pNtQueryDirectoryObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtQueryDirectoryObject"))
    pNtOpenSymbolicLinkObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtOpenSymbolicLinkObject"))
    pNtQuerySymbolicLinkObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtQuerySymbolicLinkObject"))
    pRtlInitUnicodeString =  Cast(PVOID, GetProcAddress(hNtDll, "RtlInitUnicodeString"))
    pNtClose =  Cast(PVOID, GetProcAddress(hNtDll, "NtClose"))
    
    If pNtOpenDirectoryObject = NULL OrElse pNtQueryDirectoryObject = NULL OrElse pRtlInitUnicodeString = NULL Then
        MyLog.PrintLog LOG_ERROR, "GetObjectList", "GetProcAddress"
        Exit Sub
    End If
    
    Dim objName As UNICODE_STRING
    Dim oa As OBJECT_ATTRIBUTES
    Dim hDir As HANDLE = NULL
    Dim ntStatus As NTSTATUS
    
    pRtlInitUnicodeString(objName, szPath)
    InitializeObjectAttributes(@oa, @objName, 0)
    
    ntStatus = pNtOpenDirectoryObject(hDir, DIRECTORY_QUERY Or DIRECTORY_TRAVERSE, oa)
    If ntStatus <> STATUS_SUCCESS Then
        MyLog.PrintNtError "GetObjectList", "NtOpenDirectoryObject", ntStatus, "路径:" & szPath
        Exit Sub
    End If
    
    ' 分配缓冲区枚举对象目录
    #define BUFFER_SIZE 65536  ' 64KB
    Dim pBuffer As Byte Ptr = CAllocate(BUFFER_SIZE)
    If pBuffer = NULL Then
        pNtClose(hDir)
        Exit Sub
    End If
    
    Dim ctx As ULONG = 0
    Dim returnLen As ULONG = 0
    Dim restart As Boolean = True  ' 第一次调用需要RestartScan=TRUE
    Dim startIndex As ULong = 0
    
    ' 收集所有对象信息
    Dim objNames() As WString * MAX_PATH
    Dim objTypes() As WString * 100
    Dim objLinkTargets() As WString * MAX_PATH  ' SymbolicLink的目标路径
    Dim objIsDir() As Boolean        ' 是否是目录对象
    Dim totalObjs As Long = 0
    ReDim objNames(50) As Wstring * MAX_PATH
    ReDim objTypes(50) As Wstring * MAX_PATH
    ReDim objLinkTargets(50) As Wstring * MAX_PATH
    ReDim objIsDir(50) As Boolean
    
    Do
        NTSTATUS = pNtQueryDirectoryObject(hDir, pBuffer, BUFFER_SIZE, False, restart, ctx, returnLen)
        
        If NTSTATUS = STATUS_SUCCESS OrElse NTSTATUS = STATUS_MORE_ENTRIES Then
            Dim pInfo As OBJECT_DIRECTORY_INFORMATION Ptr = Cast(OBJECT_DIRECTORY_INFORMATION Ptr, pBuffer)
            Dim entries As ULONG = ctx - startIndex
            
            For i As ULONG = 0 To entries - 1
                ' 从UNICODE_STRING提取名称和类型
                Dim nameStr As WString * MAX_PATH = ""
                Dim typeStr As WString * MAX_PATH = ""
                
                If pInfo[i].Name.Length > 0 AndAlso pInfo[i].Name.Buffer <> NULL Then
                    nameStr = *Cast(Wstring Ptr, pInfo[i].Name.Buffer)
                End If
                If pInfo[i].TypeName.Length > 0 AndAlso pInfo[i].TypeName.Buffer <> NULL Then
                    typeStr = *Cast(Wstring Ptr, pInfo[i].TypeName.Buffer)
                End If
                ' 跳过空项（最后一个NULL结构）
                If nameStr = "" Then Continue For
                
                ' 扩展数组
                If totalObjs >= UBound(objNames) Then
                    ReDim Preserve objNames(UBound(objNames) + 50) As Wstring * MAX_PATH
                    ReDim Preserve objTypes(UBound(objTypes) + 50) As Wstring * 100
                    ReDim Preserve objLinkTargets(UBound(objLinkTargets) + 50) As Wstring * MAX_PATH
                    ReDim Preserve objIsDir(UBound(objIsDir) + 50) As Boolean
                End If
                
                objNames(totalObjs) = nameStr
                objTypes(totalObjs) = typeStr
                objIsDir(totalObjs) = (typeStr = "Directory")
                
                ' 如果是SymbolicLink，查询目标路径
                If typeStr = "SymbolicLink" AndAlso pNtOpenSymbolicLinkObject <> NULL AndAlso pNtQuerySymbolicLinkObject <> NULL Then
                    Dim linkName As UNICODE_STRING
                    Dim linkOa As OBJECT_ATTRIBUTES
                    Dim hLink As HANDLE = NULL
                    Dim targetStr As UNICODE_STRING
                    Dim retLen As ULONG = 0
                    
                    ' 构造完整路径: szPath & "\" & nameStr
                    Dim fullPath As WString * MAX_PATH = szPath
                    If RightW(fullPath, 1) <> "\" Then fullPath = fullPath & "\" 
                    fullPath = fullPath & nameStr
                    'Print "fullPath:" & fullPath
                    pRtlInitUnicodeString(linkName, @fullPath)
                    InitializeObjectAttributes(@linkOa, @linkName, 0)
                    
                    If pNtOpenSymbolicLinkObject(hLink, SYMBOLIC_LINK_QUERY, linkOa) = STATUS_SUCCESS Then
                        targetStr.Length = 0
                        targetStr.MaximumLength = 512
                        targetStr.Buffer = Allocate(512)
                        If targetStr.Buffer <> NULL Then
                            If pNtQuerySymbolicLinkObject(hLink, targetStr, retLen) = STATUS_SUCCESS Then
                                If targetStr.Buffer <> NULL AndAlso targetStr.Length > 0 Then
                                    lstrcpyW(objLinkTargets(totalObjs), targetStr.Buffer)
                                End If
                            End If
                            Deallocate(targetStr.Buffer)
                        End If
                        pNtClose(hLink)
                    End If
                End If
                
                totalObjs += 1
            Next
            
            startIndex = ctx
            restart = FALSE
        End If
        
        ' 没有更多条目或出错则退出
        If ntStatus <> STATUS_SUCCESS AndAlso ntStatus <> STATUS_MORE_ENTRIES Then
            Exit Do
        End If
        FF_DoEvents
    Loop
    
    pNtClose(hDir)
    Deallocate(pBuffer)
    
    ' 填充TreeView和ListView
    If IsEnumSubDir Then
        For j As Integer = 0 To totalObjs - 1
            If objIsDir(j) Then
                ' 目录对象插入TreeView，带占位子节点以便展开
                Dim hNewItem As HTREEITEM = treObjectDirectory.InsertItem(CurrentNode, TVI_LAST, objNames(j))
                ' 插入一个占位节点，使得父节点显示展开按钮(+号)
                'treObjectDirectory.InsertItem(hNewItem, TVI_LAST, "")
            End If
        Next
    End If
    
    If IsEnumObject Then
        For j As Integer = 0 To totalObjs - 1
            ' ========== 跳过对象目录，只在TreeView中显示 ==========
            If objIsDir(j) Then Continue For
            
            Dim displayType As StringW = objTypes(j)
            Dim displayTarget As StringW = ""
            
            ' SymbolicLink显示目标路径
            If objTypes(j) = "SymbolicLink" AndAlso objLinkTargets(j) <> "" Then
                displayTarget = objLinkTargets(j)
            End If
            
            ' 添加到ListView: 对象名 | 对象类型 | 链接目标(仅SymbolicLink)
            Dim ItemIndex As Long = AddItemColListEx(lvwObject, 3,,, WStr(objNames(j)), WStr(displayType), WStr(displayTarget))
            SetItemIconEx lvwObject, ItemIndex, ICON_SYSTEM_COMPONENT
            
            ' 根据对象类型设置图标或颜色
            /'Select Case objTypes(j)
                Case "Directory"
                    SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex("folder", False)
                Case "SymbolicLink"
                    SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex("link", False)
                Case "Device"
                    SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex("device", False)
                Case "Event", "Mutant", "Semaphore", "Timer"
                    SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex("sync", False)
                Case Else
                    SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex("object", False)
            End Select'/
        Next
    End If
    
    CommitListViewView lvwObject
    
    ' 清理
    Erase objNames
    Erase objTypes
    Erase objLinkTargets
    Erase objIsDir
End Sub
