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

#define FAST_IO_MAX_COUNT 27
Const FAST_IO_PTR_SIZE = 8

Type FASTIO_SOURCE_STATE
    Valid As Boolean
    IsIat As Boolean
    SourceRva As ULong
End Type

Type FAST_IO_DISPATCH_ORIGINAL
    Functions(0 To FAST_IO_MAX_COUNT - 1) As Any Ptr  ' 按初始化顺序连续存储
    HitCount      As ULong
    Confidence    As ULong
End Type

Enum DISPATCH_RECOVERY_STATUS
    DispatchRecoveryParseFailed = 0
    DispatchRecoveryRecovered = 1
    DispatchRecoveryNoMatch = 2
End Enum

' ==================== 派遣恢复抽象 ?类型 ====================
Enum DISPATCH_MODEL_KIND
    DispatchModelUnknown = 0
    DispatchModelGenericWdm
    DispatchModelFrameworkWdm
    DispatchModelWddm
    DispatchModelKmdf
End Enum

Enum WDM_FRAMEWORK_KIND
    WdmFrameworkNone = 0
    WdmFrameworkHid
    WdmFrameworkStorPort
    WdmFrameworkScsiPort
    WdmFrameworkNdis
    WdmFrameworkKs
    WdmFrameworkOther
End Enum

Enum DISPATCH_OWNER_KIND
    DispatchOwnerUnknown = 0
    DispatchOwnerDriver
    DispatchOwnerFramework
    DispatchOwnerKernelDefault
End Enum

Enum DISPATCH_SLOT_STATE
    DispatchSlotUnresolved = 0
    DispatchSlotRecovered
    DispatchSlotFrameworkOwned
    DispatchSlotDefault
End Enum

Enum DISPATCH_RECOVERY_SOURCE
    DispatchSourceUnknown = 0
    DispatchSourceDirectStore
    DispatchSourceRepStosq
    DispatchSourceImportedFramework
    DispatchSourceFrameworkClassification
    DispatchSourceFrameworkBaseline
    DispatchSourceDefault
    DispatchSourceFrameworkDefault
End Enum

Type DISPATCH_RECOVERY_INPUT
    pDump As UByte Ptr
    FileSize As ULong
    ImageBase As ULONG_PTR
    ModuleName As String
End Type

Type DISPATCH_FUNCTION_REF
    Valid As Boolean
    ModuleBase As ULONG_PTR
    FunctionRva As ULong

    State As DISPATCH_SLOT_STATE
    OwnerKind As DISPATCH_OWNER_KIND
    Source As DISPATCH_RECOVERY_SOURCE
    Confidence As ULong
End Type

Type DISPATCH_RECOVERY_OUTPUT
    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As DISPATCH_FUNCTION_REF
    FastIoDispatchRva As ULong
    HitCount As ULong
    Confidence As ULong
End Type

' ==================== 驱动模型信息（WDDM 等） ====================
Type DRIVER_MODEL_INFO
    IsWddm         As Boolean
    Confidence     As Integer
    Reason         As String
    FrameworkOwner As String
End Type

Type DRIVER_DISPATCH_RECOVERY_CONTEXT
    pDump As UByte Ptr
    FileSize As ULong
    ImageBase As ULONG_PTR
    ImageSize As ULong
    ModuleName As String

    CurrentMajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As ULONG_PTR
    IopInvalidDeviceRequest As ULONG_PTR

    ModelKind As DISPATCH_MODEL_KIND
    FrameworkKind As WDM_FRAMEWORK_KIND
    ModelConfidence As ULong

    ' 新增：WDDM 检测结 ?
    ModelInfo As DRIVER_MODEL_INFO
End Type

Type DRIVER_DISPATCH_RECOVERY_OUTPUT
    ModelKind As DISPATCH_MODEL_KIND
    FrameworkKind As WDM_FRAMEWORK_KIND
    ModelConfidence As ULong

    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As DISPATCH_FUNCTION_REF

    FastIoDispatchRva As ULong

    HitCount As ULong
    Confidence As ULong
End Type

Type WDDM_RECOVERY_OUTPUT
    Detected As Boolean
    RegistrationFunctionRva As ULong
    InitializationDataRva As ULong
    Confidence As ULong
End Type

Type WDDM_DISPATCH_BASELINE_OUTPUT

    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As DISPATCH_FUNCTION_REF

    HitCount As ULong
    Confidence As ULong


    DxgkrnlBase As ULONG_PTR


    WriterFunctionRva As ULong
    CandidateBaseReg As Integer


    '
    ' baseline可信来源
    '
    DriverObjectTracked As Boolean

    SequentialMajorFunction As ULong

    HasUniformDefault As Boolean   ' ← 新增
    DefaultRva As ULong            ' ← 新增
    DefaultSlotCount As ULong      ' ← 新增
End Type

Type WDDM_BASELINE_CANDIDATE
    Valid As Boolean

    '
    ' 写入派遣表的函数 RVA
    '
    WriterFunctionRva As ULong

    '
    ' 认为是 DRIVER_OBJECT 的基址寄存器
    '
    BaseReg As Integer


    '
    ' DriverObject 追踪证据
    '
    DriverObjectTracked As Boolean


    '
    ' 连续写入 MajorFunction 数量
    '
    SequentialMajorFunction As ULong


    '
    ' 第一次发现的 MajorFunction 偏移
    '
    FirstMajorFunctionOffset As LongInt


    '
    ' 实际发现写入数量
    '
    HitCount As ULong


    '
    ' 与 Generic Recovery FrameworkOwned 的匹配情况
    '
    MatchCount As ULong
    ExtraCount As ULong
    MissingCount As ULong


    '
    ' 综合评分
    '
    Score As LongInt


    '
    ' 恢复出的槽位
    '
    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As DISPATCH_FUNCTION_REF

    ExactCount As ULong       ' ← 新增:与 dump 精确一致
    ConflictCount As ULong    ' ← 新增:与 dump 冲突
    HookedCount As ULong      ' ← 新增:current 在 dxgkrnl 之外
End Type

Const DRIVER_OBJECT_FAST_IO_DISPATCH = &H50
Const DRIVER_OBJECT_DRIVER_UNLOAD = &H68
Const DISPATCH_BASE = &H70
Const DISPATCH_STEP = 8

Enum DISPATCH_REG_VALUE_TYPE
    DispatchRegUnknown = 0
    DispatchRegDriverObject
    DispatchRegFunctionRva
    DispatchRegImageRva
    DispatchRegImmediate
    DispatchRegDriverObjectAddress
End Enum

Type DISPATCH_REG_STATE
    ValueType As DISPATCH_REG_VALUE_TYPE
    Value As ULONG_PTR
End Type

Const ENTRY_REG_UNKNOWN = 0
Const ENTRY_REG_DRIVER_OBJECT = 1
Const ENTRY_REG_REGISTRY_PATH = 2

Enum ORIGINAL_DRIVER_RECOVERY_STATUS
    OriginalRecoveryFatal = 0
    OriginalRecoveryDispatchRecovered = &H1
    OriginalRecoveryDispatchDefaultOnly = &H2
    OriginalRecoveryDispatchPartial = &H4
    OriginalRecoveryDispatchUnresolved = &H8
    OriginalRecoveryFastIoRecovered = &H10
    OriginalRecoveryFastIoNotPresent = &H20
    OriginalRecoveryFastIoUnresolved = &H40
    OriginalRecoveryDispatchFrameworkOwned = &H80
    ' 预留，第一阶段不实 ?
    OriginalRecoveryDispatchFrameworkBaselineRecovered = &H100
End Enum

' ==================== 安全内存读取 ====================
Type PE_IMPORT_TARGET
    Valid As Boolean
    ModuleName As String
    FunctionName As String
    Ordinal As ULong
    ByOrdinal As Boolean
End Type

Type WDDM_DETECTION_EVIDENCE
    Has23003F As Boolean
    Has230043 As Boolean

    HasLargeSelfCallbackTable As Boolean
    CallbackRun As ULong
    CallbackTableRva As ULong

    HasIndirectRegistrationPath As Boolean

    RegistrationHelperRva As ULong
    Confidence As ULong

    HasKmdodInitialization As Boolean
    KmdodVersion As ULong
    KmdodHelperRva As ULong
    KmdodCallbackCount As ULong
End Type

Const DISPATCH_MAX_CALL_DEPTH = 3
Const DISPATCH_FUNCTION_SCAN_RANGE = &H3000
Const DISPATCH_MAX_VISITED = 128

Type DISPATCH_SCAN_CONTEXT
    pDump As UByte Ptr
    FileSize As ULong
    ImageBase As ULONG_PTR
    ModuleName As String
End Type

Type DISPATCH_VISITED_FUNCTION
    ModuleBase As ULONG_PTR
    FunctionRva As ULong
End Type

Type DISPATCH_SCAN_STATE
    Visited(0 To 128 - 1) As DISPATCH_VISITED_FUNCTION ' DISPATCH_MAX_VISITED
    VisitedCount As ULong
End Type

Type FASTIO_RAX_STATE
    Valid As Boolean
    IsIat As Boolean
    SourceRva As ULong
End Type