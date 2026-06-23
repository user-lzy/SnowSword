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

' 定义线程参数结构
Type ScanThreadParam
    lvwSuspectKernelMemory As Class_ListView
    StartAddr As UInteger
    EndAddr As UInteger
    hMutex As Any Ptr      ' 互斥锁用于同步访问共享资源
    ScannedPages As UInteger Ptr  ' 共享进度计数器
    ThreadIndex As Integer  ' 线程索引
    SuspectCount As Integer Ptr ' 发现的可疑内存计数
    bFinish As Boolean ' 结束标志
End Type

Const NUM_THREADS As Integer = 8 ' 可根据CPU核心数调整
Const PAGE_SIZE As DWORD = &H1000 ' (4KB大小)

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

' 线程工作函数
Sub KernelMemoryScanThread(ByVal userdata As Any Ptr)
    Dim p As ScanThreadParam Ptr = Cast(ScanThreadParam Ptr, userdata)
    Dim stMemory As MemoryStruct
    Dim result As MEMORY_SCAN_RESULT
    Dim localScanCount As UInteger = 0
    
    ' 遍历分配的内存范围
    For Addr As UInteger = p->StartAddr To p->EndAddr Step PAGE_SIZE
        stMemory.Addr = Cast(PVOID, Addr)
        stMemory.dwSize = PAGE_SIZE
        
        ' 调用驱动扫描内存
        If IoControl(hDrv, IOCTL_ScanKernelMemory, @stMemory, SizeOf(MemoryStruct), @result, SizeOf(MEMORY_SCAN_RESULT)) <> 0 Then
            If result <> Normal Then
                ' 发现可疑内存，需要同步添加到列表
                ' 增加可疑计数
                Dim SuspectIndex As Integer = 0
                MutexLock(p->hMutex)
                SuspectIndex = *p->SuspectCount
                'Print SuspectIndex & " 0x" & WHex(Addr)
                Dim DriverPath As StringW = GetDriverPathByAddr(Addr)
                Dim ItemIndex As Long = AddItemColListEx(p->lvwSuspectKernelMemory, 4,,, WStr(SuspectIndex), WStr(GetMemoryScanType(result)), "0x" & WHex(Addr), WStr(DriverPath))
                If result <> HideDriver OrElse LenW(DriverPath) = 0 Then SetItemColor p->lvwSuspectKernelMemory, ItemIndex, FB_Red, FB_White
                *p->SuspectCount += 1
                MutexUnlock(p->hMutex)
            End If
        End If
        
        ' 增加扫描页数计数器
        MutexLock(p->hMutex)
        *p->ScannedPages += 1
        MutexUnlock(p->hMutex)
        
        localScanCount += 1
        
        ' 定期让出CPU时间给其他线程和UI
        If (localScanCount Mod 20) = 0 Then
            FF_DoEvents
        End If
    Next
    Print "扫描起始地址0x" & WHex(p->StartAddr) & "的线程退出"
    p->bFinish = True
End Sub

' 主函数中的多线程扫描实现
Sub ScanKernelMemoryMultiThread(lvwSuspectKernelMemory As Class_ListView)
    Dim threads(NUM_THREADS - 1) As Any Ptr
    ' 循环内为每个线程单独分配堆内存（核心修复）
    Dim pParam As ScanThreadParam Ptr
    Dim hMutex As Any Ptr
    Dim totalPages As UInteger
    Dim scannedPages As UInteger = 0
    Dim suspectCount As Integer = 0
    Dim i As Integer
    
    Dim ModuleSize As ULong
    Dim EndAddr As UInteger = Cast(UInteger, GetKernelModuleBase("ntoskrnl.exe", ModuleSize)) '&HFFFFF90000000000ULL
    Dim StartAddr As UInteger = EndAddr - &H70000000 ' &HFFFFF80300000000ULL
    
    Dim result As MEMORY_SCAN_RESULT
    If AfxMsg("是否扫描?",, MB_YESNO) = IDNO Then Exit Sub
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If InitDriver Then AfxMsg "加载成功!" Else AfxMsg "加载失败!" : Exit Sub
    End If
    
    InitListViewData lvwSuspectKernelMemory
    
    ' 计算总页数
    totalPages = ((EndAddr - StartAddr) \ PAGE_SIZE) + 1
    
    ' 创建互斥锁
    hMutex = MutexCreate()
    If hMutex = 0 Then
        AfxMsg "无法创建互斥锁"
        Exit Sub
    End If
    
    ' 设置进度标签初始值
    FrmMain.lblNum.Caption = "进度: 0%"
    FF_DoEvents
    
    pParam = CAllocate(Len(ScanThreadParam) * NUM_THREADS)  ' 堆内存，生命周期跨线程，不会失效
    
    ' 创建并启动线程
    For i = 0 To NUM_THREADS - 1
        ' 计算每个线程的地址范围（确保地址对齐）
        Dim addrRange As UInteger = (EndAddr - StartAddr) \ NUM_THREADS
        pParam[i].StartAddr = StartAddr + i * addrRange
        Print "为线程" & i & "分配扫描地址0x" & Hex(pParam[i].StartAddr) & "的任务"
        ' 最后一个线程处理剩余所有范围
        If i = NUM_THREADS - 1 Then
            pParam[i].EndAddr = EndAddr
        Else
            pParam[i].EndAddr = pParam[i].StartAddr + addrRange - 1
        End If
        
        pParam[i].lvwSuspectKernelMemory = lvwSuspectKernelMemory
        
        ' 确保地址是PAGE_SIZE对齐
        pParam[i].StartAddr = (pParam[i].StartAddr \ PAGE_SIZE) * PAGE_SIZE
        pParam[i].EndAddr = ((pParam[i].EndAddr + PAGE_SIZE - 1) \ PAGE_SIZE) * PAGE_SIZE
        
        ' 设置同步参数
        pParam[i].hMutex = hMutex
        pParam[i].scannedPages = @scannedPages
        pParam[i].suspectCount = @suspectCount
        pParam[i].bFinish = False
        
        ' 创建线程
        Threads(i) = ThreadCreate(@KernelMemoryScanThread, @pParam[i])
        If threads(i) = 0 Then
            AfxMsg "线程 " & i & " 创建失败"
        End If
    Next
    
    ' 等待线程完成并更新进度
    Dim lastProgress As Integer = -1
    
    Print "正在等待所有线程结束..."

    Do While True
        ' 检查线程状态
        Dim activeThreadCount As Integer = NUM_THREADS
        'Print "activeThreadCount:" & activeThreadCount
        For i As Integer = 0 To NUM_THREADS - 1
            If pParam[i].bFinish = True Then activeThreadCount -= 1
        Next
        If activeThreadCount = 0 Then
            Print "线程已全部退出!"
            Exit Do
        End If
        
        ' 获取并更新进度
        'MutexLock(hMutex)
        Dim currentProgress As Integer = (scannedPages * 100) \ totalPages
        'MutexUnlock(hMutex)
        
        ' 只在进度变化时更新UI（减少闪烁）
        If currentProgress <> lastProgress Then
            lastProgress = currentProgress
            FrmMain.lblNum.Caption = "进度:" & WStr(currentProgress) & "%"
            'FrmMain.lblNum.Refresh  ' 强制立即刷新
        End If
        
        FF_DoEvents
        'Sleep(1)  ' 避免CPU占用过高
    Loop
    
    ' 清理
    MutexDestroy(hMutex)
    Deallocate(pParam)
    
    ' 扫描完成
    AfxMsg "扫描完成，发现 " & suspectCount & " 处可疑内存"
    CommitListViewView lvwSuspectKernelMemory
End Sub

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

Private Function GetDriverObjectAddr(ByVal DriverBaseAddr As PVOID) As Integer
    Dim pDriverObject As Integer, DriverBase As PVOID = DriverBaseAddr
    IoControl hDrv, IOCTL_GetDriverObjectByBaseAddress, @DriverBase, SizeOf(DriverBase), @pDriverObject, SizeOf(pDriverObject)
    Return pDriverObject
End Function
' Dim bCheck As Boolean = (LCaseW(CurrentDriver_Info.DriverName) = "ntfs.sys") OrElse (LCaseW(CurrentDriver_Info.DriverName) = "exfat.sys") OrElse (LCaseW(CurrentDriver_Info.DriverName) = "fastfat.sys")
Public Sub GetDriverIOFunctionList(lvwIOFunction As Class_ListView, DriverPath As StringW, DriverObject As Integer)
    Dim pDriverObject As Integer = DriverObject
    Dim DriverInfo As DRIVER_INFO
    Dim OriginalDriverInfo As DRIVER_INFO
    InitListViewData lvwIOFunction
    
    SetLastError 0
    If IoControl(hDrv, IOCTL_GetDriverInfo, @pDriverObject, SizeOf(pDriverObject), @DriverInfo, SizeOf(DriverInfo)) = 0 Then Print "IOCTL_GetDriverInfo失败," & WinErrorMsg(GetLastError)
    GetOriginalDispatchFunctionAddr(DriverPath, OriginalDriverInfo)
    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverInit"), WStr("0x" & WHex(DriverInfo.DriverInitAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverInitAddr)), _
    WStr("-"), WStr("无")
    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverStartIo"), WStr("0x" & WHex(DriverInfo.DriverStartIoAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverStartIoAddr)), _
    WStr("-"), WStr("无")
    AddItemColListEx lvwIOFunction, 5,,, WStr("DriverUnload"), WStr("0x" & WHex(DriverInfo.DriverUnloadAddr)), WStr(GetDriverPathByAddr(DriverInfo.DriverUnloadAddr)), _
    WStr("-"), WStr("无")
    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        'If i = &H20 Then Continue For ' 这里有什么用?
        Print "0x" & WHex(DriverInfo.MajorFunctionAddr(i))
        Dim ItemIndex As Long = AddItemColListEx(lvwIOFunction, 5,,, WStr(IRP_MJ_ToString(i)), WStr("0x" & WHex(DriverInfo.MajorFunctionAddr(i))), WStr(GetDriverPathByAddr(DriverInfo.MajorFunctionAddr(i))), _
        IIf(OriginalDriverInfo.MajorFunctionAddr(i) = NULL, WStr("-"), "0x" & WHex(OriginalDriverInfo.MajorFunctionAddr(i))), _
        IIf(OriginalDriverInfo.MajorFunctionAddr(i) = NULL, WStr("无"), WStr(GetDriverPathByAddr(OriginalDriverInfo.MajorFunctionAddr(i)))))
        If (OriginalDriverInfo.MajorFunctionAddr(i) <> NULL) AndAlso (DriverInfo.MajorFunctionAddr(i) <> OriginalDriverInfo.MajorFunctionAddr(i)) Then
            SetItemColor lvwIOFunction, ItemIndex, FB_Red, FB_White
        End If
        FF_DoEvents
    Next
    
    AddItemColListEx lvwIOFunction, 5,,, WStr("FastIoDispatch"), WStr("0x" & WHex(DriverInfo.FastIoDispatchAddr)), WStr(GetDriverPathByAddr(DriverInfo.FastIoDispatchAddr)), _
    WStr("-"), WStr("无")
    If DriverInfo.FastIoDispatchAddr <> NULL Then
        For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
            Dim ItemIndex As Long = AddItemColListEx(lvwIOFunction, 5,,, WStr(FastIoIndexToName(i)), WStr("0x" & WHex(DriverInfo.FastIOFunctionAddr(i))), WStr(GetDriverPathByAddr(DriverInfo.FastIOFunctionAddr(i))), _
            IIf(OriginalDriverInfo.FastIOFunctionAddr(i) = NULL, WStr("-"), "0x" & WHex(OriginalDriverInfo.FastIOFunctionAddr(i))), _
            IIf(OriginalDriverInfo.FastIOFunctionAddr(i) = NULL, WStr("无"), WStr(GetDriverPathByAddr(OriginalDriverInfo.FastIOFunctionAddr(i)))))
            If (OriginalDriverInfo.FastIOFunctionAddr(i) <> NULL) AndAlso (DriverInfo.FastIOFunctionAddr(i) <> OriginalDriverInfo.FastIOFunctionAddr(i)) Then
                SetItemColor lvwIOFunction, ItemIndex, FB_Red, FB_White
            End If
            FF_DoEvents
        Next
    End If
        
    CommitListViewView lvwIOFunction
End Sub

Public Sub GetKernelModuleList(lvwKernelModule As Class_ListView)
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        Print "Failed to allocate memory."
        Exit Sub
    End If
    
    InitListViewData lvwKernelModule
    
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
    For i As Integer = 0 To smi->Count - 1
        Dim DriverName As WString * 20, DriverPath As ZString * MAX_PATH = smi->Modules(i).ImageName
        Dim DriverObject As Integer
        
        DriverPath = GetCorrectPath(DriverPath)
        DriverName = Right(DriverPath, Len(DriverPath) - InStrRev(DriverPath, "\"))
        If IsDriverLoaded Then DriverObject = GetDriverObjectAddr(smi->Modules(i).dwBase)
        
        Dim ItemIndex As Long = AddItemColListEx(lvwKernelModule, 6,,, WStr(i), WStr(DriverName), "0x" & WHex(smi->Modules(i).dwBase), "0x" & WHex(smi->Modules(i).dwSize), _
        WStr(DriverPath), "0x" & WHex(DriverObject))
        VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", ItemIndex, DriverPath
        FF_DoEvents
    Next
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

Public Function GetKernelModulePath(ByVal ModuleName As WString Ptr, ByVal OutPath As WString Ptr) As Boolean
    Dim st As NTSTATUS
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    If (buffer = NULL) Then
        Print "Failed to allocate memory."
        Return False
    End If

    ' First call to get required buffer size
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            Print "Failed to allocate memory."
            Return False
        End If
    End If

    ' Second call to get actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        Print "NtQuerySystemInformation failed with st: 0x" & Hex(st)
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
        ' 只打印调试信息（可选）
        ' Print "Module "; i; ": "; smi->Modules(i).ImageName

        ' 前缀匹配，大小写不敏感
        If InStr(UCase(smi->Modules(i).ImageName), UCase(wStrToStr(target))) > 0 Then
            ' 找到模块，将 ANSI ImageName 转成 Unicode
            Print "smi->Modules(i).ImageName:" & smi->Modules(i).ImageName
            *OutPath = smi->Modules(i).ImageName
            Print "OutPath:" & *OutPath
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
    Print "ModuleBase:0x" & WHex(ModuleBase)
    ' 2. 获取函数VA
    Return GetProcVA(ModuleName, FunctionName, ModuleBase)
End Function

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
        'Path = "无法获取所在模块"
    End If

    'First Call To Get the required buffer SIZE
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, @bufferSize)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        buffer = Reallocate(buffer, bufferSize)
        If (buffer = NULL) Then
            Print "Failed to allocate memory."
            'Path = "无法获取所在模块"
        End If
    End If

    'Second call to get the actual data
    st = ZwQuerySystemInformation(SystemModuleInformation, buffer, bufferSize, NULL)
    If (st < 0) Then
        Print "NtQuerySystemInformation failed with st: 0x" & Hex(st)
        Deallocate buffer
        'Path = "无法获取所在模块"
    End If

    Dim smi As SYSTEM_MODULE_INFORMATION Ptr = CPtr(SYSTEM_MODULE_INFORMATION Ptr, buffer)
    'Print "Enumerated Drivers:"
    For i As Integer = 0 To smi->Count - 1
        If (Cast(Integer, smi->Modules(i).dwBase) <= Addr And Cast(Integer, smi->Modules(i).dwBase + smi->Modules(i).dwSize) >= Addr) Then
            Dim ImageName As String = ""
            ImageName = GetCorrectPath(smi->Modules(i).ImageName)
            Deallocate buffer
            Return ImageName
        End If
    Next
    Deallocate buffer
    Return "未知模块"
End Function

Function GetGDT(lvwGDT As Class_mCtrlTreeList) As ULong
    Dim GDTArray() As GDT_INFO
    
    Dim ret As Long, lpRet As DWORD, NewIndex As Long
    /'If GetSystemVersion <> "Windows 11 23H2" Then
        AfxMsg "很抱歉,该功能暂时仅在Win11 23H2上可用!", "提示"
        Exit Sub
    End If '/
    SetLastError 0
    IoControl hDrv, IOCTL_GetGDT, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetGDT]第一次IOCTL失败:" & WinErrorMsg(GetLastError) & GetLastError
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
    'CommitListViewView lvwGDT
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
    IoControl hDrv, IOCTL_GetIDT, NULL, 0, NULL, 0, @lpRet
    If GetLastError <> 0 Then
        Print "[GetIDT]第一次IOCTL失败:" & WinErrorMsg(GetLastError) & GetLastError
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
    'CommitListViewView lvwIDT
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
            If HalDispatchAddr <> NULL AndAlso (HalDispatchAddr < ntBase OrElse HalDispatchAddr > ntEnd) Then ' 被hook
                SetItemColor lvwHalDistapch, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        Else
            Print "[GetHalDispatchTable]IoControl" & Index & ":" & WinErrorMsg(GetLastError) & GetLastError
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
            If HalPrivateDispatchAddr <> NULL AndAlso (HalPrivateDispatchAddr < ntBase OrElse HalPrivateDispatchAddr > ntEnd) Then ' 被hook
                SetItemColor lvwHalPrivateDistapch, ItemIndex, FB_Red, FB_White
            End If
            Count += 1
        Else
            Print "[GetHalPrivateDispatchTable]IoControl" & Index & ":" & WinErrorMsg(GetLastError) & GetLastError
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
    'Print "szPath:" & szPath
    szPath = RightW(szPath, LenW(szPath) - 1)
    If RightW(szPath, 1) = "\" Then szPath = LeftW(szPath, LenW(szPath) - 1)
    'Print "szPath:" & szPath
    If szPath = "" Then szPath = "\"  ' 根目录
    
    ' 加载ntdll
    hNtDll = GetModuleHandleW("ntdll.dll")
    If hNtDll = NULL Then
        Print "[GetObjectList]无法加载ntdll.dll"
        Exit Sub
    End If
    
    pNtOpenDirectoryObject = Cast(PVOID, GetProcAddress(hNtDll, "NtOpenDirectoryObject"))
    pNtQueryDirectoryObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtQueryDirectoryObject"))
    pNtOpenSymbolicLinkObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtOpenSymbolicLinkObject"))
    pNtQuerySymbolicLinkObject =  Cast(PVOID, GetProcAddress(hNtDll, "NtQuerySymbolicLinkObject"))
    pRtlInitUnicodeString =  Cast(PVOID, GetProcAddress(hNtDll, "RtlInitUnicodeString"))
    pNtClose =  Cast(PVOID, GetProcAddress(hNtDll, "NtClose"))
    
    If pNtOpenDirectoryObject = NULL OrElse pNtQueryDirectoryObject = NULL OrElse pRtlInitUnicodeString = NULL Then
        Print "[GetObjectList]无法获取必要的Native API函数地址"
        Exit Sub
    End If
    
    Dim objName As UNICODE_STRING
    Dim oa As OBJECT_ATTRIBUTES
    Dim hDir As HANDLE = NULL
    Dim ntStatus As NTSTATUS
    
    pRtlInitUnicodeString(objName, szPath)
    InitializeObjectAttributes(@oa, @objName, 0)
    
    NTSTATUS = pNtOpenDirectoryObject(hDir, DIRECTORY_QUERY Or DIRECTORY_TRAVERSE, oa)
    If ntStatus <> STATUS_SUCCESS Then
        Print "[GetObjectList]NtOpenDirectoryObject失败，路径:" & szPath & " 状态码:0x" & Hex(ntStatus)
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
    Dim objNames() As StringW
    Dim objTypes() As StringW
    Dim objLinkTargets() As StringW  ' SymbolicLink的目标路径
    Dim objIsDir() As Boolean        ' 是否是目录对象
    Dim totalObjs As Long = 0
    ReDim objNames(50) As StringW
    ReDim objTypes(50) As StringW
    ReDim objLinkTargets(50) As StringW
    ReDim objIsDir(50) As Boolean
    
    Do
        NTSTATUS = pNtQueryDirectoryObject(hDir, pBuffer, BUFFER_SIZE, False, restart, ctx, returnLen)
        
        If NTSTATUS = STATUS_SUCCESS OrElse NTSTATUS = STATUS_MORE_ENTRIES Then
            Dim pInfo As OBJECT_DIRECTORY_INFORMATION Ptr = Cast(OBJECT_DIRECTORY_INFORMATION Ptr, pBuffer)
            Dim entries As ULONG = ctx - startIndex
            
            For i As ULONG = 0 To entries - 1
                ' 从UNICODE_STRING提取名称和类型
                Dim nameStr As StringW = ""
                Dim typeStr As StringW = ""
                
                If pInfo[i].Name.Length > 0 AndAlso pInfo[i].Name.Buffer <> NULL Then
                    nameStr = *Cast(WString Ptr, pInfo[i].Name.Buffer)
                End If
                
                If pInfo[i].TypeName.Length > 0 AndAlso pInfo[i].TypeName.Buffer <> NULL Then
                    typeStr = *Cast(WString Ptr, pInfo[i].TypeName.Buffer)
                End If
                
                ' 跳过空项（最后一个NULL结构）
                If nameStr = "" Then Continue For
                
                ' 扩展数组
                If totalObjs >= UBound(objNames) Then
                    ReDim Preserve objNames(UBound(objNames) + 50) As StringW
                    ReDim Preserve objTypes(UBound(objTypes) + 50) As StringW
                    ReDim Preserve objLinkTargets(UBound(objLinkTargets) + 50) As StringW
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
                    Dim fullPath As StringW = szPath
                    If RightW(fullPath, 1) <> "\" Then fullPath = fullPath & "\" 
                    fullPath = fullPath & nameStr
                    
                    pRtlInitUnicodeString(linkName, fullPath)
                    InitializeObjectAttributes(@linkOa, @linkName, 0)
                    
                    If pNtOpenSymbolicLinkObject(hLink, SYMBOLIC_LINK_QUERY, linkOa) = STATUS_SUCCESS Then
                        targetStr.Length = 0
                        targetStr.MaximumLength = 512
                        targetStr.Buffer = Allocate(512)
                        If targetStr.Buffer <> NULL Then
                            If pNtQuerySymbolicLinkObject(hLink, targetStr, retLen) = STATUS_SUCCESS Then
                                If targetStr.Buffer <> NULL AndAlso targetStr.Length > 0 Then
                                    objLinkTargets(totalObjs) = *Cast(WString Ptr, targetStr.Buffer)
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