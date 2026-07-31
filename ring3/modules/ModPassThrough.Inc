' =============================================================
' 统一存储设备Passthrough类库 - 最终版
' 支持ATA/SCSI/NVMe设备自动识别与操作
' =============================================================

#include "win/winioctl.bi"

' 常量定义
Const IOCTL_ATA_PASS_THROUGH = &H4D02C
Const IOCTL_SCSI_PASS_THROUGH_DIRECT = &H4D014
Const IOCTL_STORAGE_PROTOCOL_COMMAND = &H2D1400

' ATA标志位
Const ATA_FLAGS_DATA_IN = &H02
Const ATA_FLAGS_DATA_OUT = &H04
Const ATA_FLAGS_48BIT_COMMAND = &H80

' ATA常量
Const ATA_CMD_READ_SECTORS_EXT = &H24
Const ATA_CMD_WRITE_SECTORS_EXT = &H34

' NVME常量
Const STORAGE_PROTOCOL_STRUCTURE_VERSION = &H1
Const STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST = &H80000000

' SCSI数据方向
Const SCSI_IOCTL_DATA_IN = 1
Const SCSI_IOCTL_DATA_OUT = 0

' =============================================================
' 类定义：StorageDevice（面向对象封装）
' =============================================================
Type StorageDevice
    Private:
        m_path As String
        m_type As Integer
        sectorSize As ULong
        m_handle As HANDLE
        m_isLocked As Integer
        
        ' 私有方法
        Declare Function ATA_Read(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Function ATA_Write(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Sub ATA_Identify()
        Declare Function SCSI_Read(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Function SCSI_Write(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Sub SCSI_Inquiry()
        Declare Function NVMERead(sector As ULongInt, buffer As Any Ptr) As Long
        Declare Function NVMEWrite(sector As ULongInt, buffer As Any Ptr) As Long
        Declare Function ATA_SMART() As Integer
        Declare Function SCSI_Smart() As Integer
        
    Public:
        ' 构造函数与析构函数
        Declare Constructor(path As String)
        Declare Destructor()
        
        ' 基本操作
        Declare Function Open() As Boolean
        Declare Sub Close()
        Declare Function IsOpen() As Integer
        
        ' 属性获取
        Declare Function GetTypeName() As String
        Declare Function GetSectorSize() As ULong
        Declare Function GetType() As Integer
        Declare Function IsLocked() As Boolean
        
        ' 设备操作
        Declare Function ReadSector(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Function WriteSector(sector As ULongInt, buffer As UByte Ptr) As Integer
        Declare Function ReadAtOffset(offset As ULongInt, buf As Any Ptr, dwSize As ULong) As Long
        Declare Function WriteAtOffset(offset As ULongInt, buf As Any Ptr, dwSize As ULong) As Long
        Declare Sub QueryInfo()
        Declare Function QuerySMART() As Integer
        
        ' 设备锁定（写入前必须调用）
        Declare Function Lock() As Integer
        Declare Function Unlock() As Integer
        
        ' 静态工具函数
        Declare Function DetectType(drivePath As String) As Integer
End Type

' =============================================================
' 结构体定义
' =============================================================
Type ATA_PASS_THROUGH_EX
    Length              As UShort
    AtaFlags            As UShort
    PathId              As UByte
    TargetId            As UByte
    Lun                 As UByte
    ReservedAsUchar     As UByte
    DataTransferLength  As ULong
    TimeOutValue        As ULong
    ReservedAsUlong     As ULong
    DataBufferOffset    As ULONG_PTR
    PreviousTaskFile(0 To 7) As UByte
    CurrentTaskFile(0 To 7) As UByte
End Type

Type SCSI_PASS_THROUGH_DIRECT
    Length              As UShort
    ScsiStatus          As UByte
    PathId              As UByte
    TargetId            As UByte
    Lun                 As UByte
    CdbLength           As UByte
    SenseInfoLength     As UByte
    DataIn              As UByte
    DataTransferLength  As ULong
    TimeOutValue        As ULong
    DataBuffer          As ULONG_PTR
    SenseInfoOffset     As ULong
    Cdb(0 To 15)        As UByte
End Type

Type NVME_RW_COMMAND
    Opcode      As UByte
    Flags       As UByte
    CommandId   As UShort
    NSID        As ULong
    Reserved1   As ULongLong
    MPTR        As ULongLong
    PRP1        As ULongLong
    PRP2        As ULongLong
    SLBA        As ULongLong
    NLB         As UShort
    Control     As UShort
    Reserved2(0 To 5) As ULong
End Type

Type STORAGE_PROTOCOL_COMMAND
    Version                         As ULong
    Length                          As ULong
    ProtocolType                    As ULong   ' STORAGE_PROTOCOL_TYPE
    dwFlags                           As ULong
    ReturnStatus                    As ULong
    ErrorCode                       As ULong
    CommandLength                   As ULong
    ErrorInfoLength                 As ULong
    DataToDeviceTransferLength      As ULong
    DataFromDeviceTransferLength    As ULong
    TimeOutValue                    As ULong
    CommandSpecific                 As ULong
    Reserved0                       As ULong
    FixedProtocolReturnData         As ULong
    Reserved1(0 To 2)               As ULong
    CommandBufferOffset             As ULong
    ErrorInfoBufferOffset           As ULong
    DataToDeviceBufferOffset        As ULong
    DataFromDeviceBufferOffset      As ULong
End Type

Enum HD_TYPE
    HD_UNKNOWN = 0
    HD_ATA = 1
    HD_SCSI = 2
    HD_NVME = 3
End Enum

' =============================================================
' 类实现：StorageDevice
' =============================================================

' 构造函数
Constructor StorageDevice(path As String)
    This.m_path = path
    This.m_type = HD_UNKNOWN
    This.sectorSize = 512
    This.m_handle = INVALID_HANDLE_VALUE
    This.m_isLocked = 0
End Constructor

' 析构函数
Destructor StorageDevice()
    If This.m_isLocked Then This.Unlock()
    This.Close()
End Destructor

' 打开设备
Function StorageDevice.Open() As Boolean
    If This.m_handle <> INVALID_HANDLE_VALUE Then Return 1
    
    ' 自动检测类型
    This.m_type = DetectType(This.m_path)
    If This.m_type = HD_UNKNOWN Then Return False
    
    ' 打开句柄
    This.m_handle = CreateFile(This.m_path, _
        GENERIC_READ Or GENERIC_WRITE, _
        FILE_SHARE_READ Or FILE_SHARE_WRITE, _
        NULL, OPEN_EXISTING, 0, NULL)
    
    If This.m_handle = INVALID_HANDLE_VALUE Then
        This.m_type = HD_UNKNOWN
        Return False
    End If
    
    ' 获取扇区大小
    Dim geometry As DISK_GEOMETRY
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, _
                      NULL, 0, @geometry, SizeOf(geometry), _
                      @bytesReturned, NULL) Then
        This.sectorSize = geometry.BytesPerSector
    End If
    
    Return True
End Function

' 关闭设备
Sub StorageDevice.Close()
    If This.m_handle <> INVALID_HANDLE_VALUE Then
        CloseHandle(This.m_handle)
        This.m_handle = INVALID_HANDLE_VALUE
    End If
End Sub

' 是否已打开
Function StorageDevice.IsOpen() As Integer
    Return This.m_handle <> INVALID_HANDLE_VALUE
End Function

' 获取类型名称
Function StorageDevice.GetTypeName() As String
    Select Case This.m_type
    Case HD_ATA: Return "ATA/IDE/SATA"
    Case HD_SCSI: Return "SCSI/SAS/USB"
    Case HD_NVME: Return "NVMe SSD"
    Case Else: Return "未知"
    End Select
End Function

' 获取扇区大小
Function StorageDevice.GetSectorSize() As ULong
    Return This.sectorSize
End Function

' 获取类型
Function StorageDevice.GetType() As Integer
    Return This.m_type
End Function

' 获取锁定情况
Function StorageDevice.IsLocked() As Boolean
    Return This.m_isLocked
End Function

' 读取扇区（自动分发）
Function StorageDevice.ReadSector(sector As ULongInt, buffer As UByte Ptr) As Integer
    If Not This.IsOpen() Or buffer = NULL Then Return -1
    
    Select Case This.m_type
    Case HD_ATA: Return This.ATA_Read(sector, buffer)
    Case HD_SCSI, HD_NVME: Return This.SCSI_Read(sector, buffer)
    Case Else: Return -1
    End Select
End Function

' 写入扇区（自动分发）
Function StorageDevice.WriteSector(sector As ULongInt, buffer As UByte Ptr) As Integer
    If Not This.IsOpen() Or buffer = NULL Then Return -1
    
    If sector >= 16 Then
        MyLog.PrintLog LOG_ERROR, "StorageDevice.WriteSector",, "Windows阻止R3写入扇区" & sector & "(允许0-15)"
        Return -2
    End If
    
    ' 检查是否锁定
    If Not This.m_isLocked Then
        MyLog.PrintLog LOG_WARN, "StorageDevice.WriteSector",, "写入前未锁定设备,可能导致数据损坏"
    End If
    
    Select Case This.m_type
    Case HD_ATA: Return This.ATA_Write(sector, buffer)
    Case HD_SCSI, HD_NVME: Return This.SCSI_Write(sector, buffer)
    Case Else: Return -1
    End Select
End Function

Function StorageDevice.ReadAtOffset(offset As ULongInt, buf As Any Ptr, dwSize As ULong) As Long
    Dim total As ULong = 0
    Dim dst As UByte Ptr = buf

    While total < dwSize
        Dim sec As ULongInt = (offset + total) \ sectorSize
        Dim off As ULong = (offset + total) Mod sectorSize

        Dim tmp() As UByte
        ReDim tmp(sectorSize - 1)

        If ReadSector(sec, @tmp(0)) <= 0 Then Exit While

        Dim n As ULong = sectorSize - off
        If n > dwSize - total Then n = dwSize - total

        CopyMemory(dst, @tmp(off), n)
        dst += n
        total += n
    Wend

    Return total
End Function

Function StorageDevice.WriteAtOffset(offset As ULongInt, buf As Any Ptr, dwSize As ULong) As Long
    Dim total As ULong = 0
    Dim src As UByte Ptr = buf

    While total < dwSize
        Dim sec As ULongInt = (offset + total) \ sectorSize
        Dim off As ULong = (offset + total) Mod sectorSize

        Dim tmp() As UByte
        ReDim tmp(sectorSize - 1)

        If ReadSector(sec, @tmp(0)) <= 0 Then Exit While

        Dim n As ULong = sectorSize - off
        If n > dwSize - total Then n = dwSize - total

        CopyMemory(@tmp(off), src, n)
        If WriteSector(sec, @tmp(0)) <= 0 Then Exit While

        src += n
        total += n
    Wend

    Return total
End Function

' 查询信息（自动分发）
Sub StorageDevice.QueryInfo()
    If Not This.IsOpen() Then Exit Sub
    
    Select Case This.m_type
    Case HD_ATA: This.ATA_Identify()
    Case HD_SCSI, HD_NVME: This.SCSI_Inquiry()
    Case Else: MyLog.PrintLog LOG_ERROR, "StorageDevice.QueryInfo",, "不支持的设备类型"
    End Select
End Sub

' 查询S.M.A.R.T.（自动分发）
Function StorageDevice.QuerySMART() As Integer
    If Not This.IsOpen() Then Return 0
    
    Select Case This.m_type
    Case HD_ATA: Return This.ATA_SMART()
    Case HD_SCSI, HD_NVME: Return This.SCSI_Smart()
    Case Else: Return 0
    End Select
End Function

' 锁定设备
Function StorageDevice.Lock() As Integer
    If Not This.IsOpen() Then Return 0
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, FSCTL_LOCK_VOLUME, _
                      NULL, 0, NULL, 0, @bytesReturned, NULL) Then
        This.m_isLocked = 1
        Return 1
    End If
    
    Return 0
End Function

' 解锁设备
Function StorageDevice.Unlock() As Integer
    If Not This.IsOpen() Or Not This.m_isLocked Then Return 0
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, FSCTL_UNLOCK_VOLUME, _
                      NULL, 0, NULL, 0, @bytesReturned, NULL) Then
        This.m_isLocked = 0
        Return 1
    End If
    
    Return 0
End Function

' =============================================================
' 私有方法实现
' =============================================================

' ATA读取实现
Function StorageDevice.ATA_Read(sector As ULongInt, buffer As UByte Ptr) As Integer
    Dim packetSize As ULong = SizeOf(ATA_PASS_THROUGH_EX) + sectorSize
    Dim packet As UByte Ptr = Allocate(packetSize)
    If packet = NULL Then Return -1

    ZeroMemory(packet, packetSize)

    Dim apt As ATA_PASS_THROUGH_EX Ptr = Cast(ATA_PASS_THROUGH_EX Ptr, packet)
    Dim dataBuf As UByte Ptr = packet + SizeOf(ATA_PASS_THROUGH_EX)

    apt->Length = SizeOf(ATA_PASS_THROUGH_EX)
    apt->AtaFlags = ATA_FLAGS_DATA_IN Or ATA_FLAGS_48BIT_COMMAND
    apt->DataTransferLength = sectorSize
    apt->TimeOutValue = 10
    apt->DataBufferOffset = SizeOf(ATA_PASS_THROUGH_EX)

    ' ===== 48-bit LBA =====
    ' Sector Count = 1
    apt->CurrentTaskFile(0) = 1
    apt->CurrentTaskFile(1) = 0

    ' LBA low / mid / high (low bytes)
    apt->CurrentTaskFile(2) = sector And &HFF
    apt->CurrentTaskFile(3) = (sector Shr 8) And &HFF
    apt->CurrentTaskFile(4) = (sector Shr 16) And &HFF

    ' LBA high bytes
    apt->CurrentTaskFile(5) = (sector Shr 24) And &HFF
    apt->CurrentTaskFile(6) = (sector Shr 32) And &HFF
    apt->CurrentTaskFile(7) = (sector Shr 40) And &HFF

    ' Command
    apt->CurrentTaskFile(6) = ATA_CMD_READ_SECTORS_EXT

    Dim br As ULong
    ' IOCTL_ATA_PASS_THROUGH_EX
    Dim ok As WINBOOL = DeviceIoControl(This.m_handle, IOCTL_ATA_PASS_THROUGH, _
        packet, packetSize, packet, packetSize, @br, NULL)

    If ok Then
        CopyMemory(buffer, dataBuf, sectorSize)
        Deallocate(packet)
        Return sectorSize
    End If

    Deallocate(packet)
    Return -1
End Function

' ATA写入实现
Function StorageDevice.ATA_Write(sector As ULongInt, buffer As UByte Ptr) As Integer
    Dim packetSize As ULong = SizeOf(ATA_PASS_THROUGH_EX) + sectorSize
    Dim packet As UByte Ptr = Allocate(packetSize)
    If packet = NULL Then Return -1

    ZeroMemory(packet, packetSize)

    Dim apt As ATA_PASS_THROUGH_EX Ptr = Cast(ATA_PASS_THROUGH_EX Ptr, packet)
    Dim dataBuf As UByte Ptr = packet + SizeOf(ATA_PASS_THROUGH_EX)

    CopyMemory(dataBuf, buffer, sectorSize)

    apt->Length = SizeOf(ATA_PASS_THROUGH_EX)
    apt->AtaFlags = ATA_FLAGS_DATA_OUT Or ATA_FLAGS_48BIT_COMMAND
    apt->DataTransferLength = sectorSize
    apt->TimeOutValue = 10
    apt->DataBufferOffset = SizeOf(ATA_PASS_THROUGH_EX)

    ' Sector Count = 1
    apt->CurrentTaskFile(0) = 1
    apt->CurrentTaskFile(1) = 0

    ' LBA
    apt->CurrentTaskFile(2) = sector And &HFF
    apt->CurrentTaskFile(3) = (sector Shr 8) And &HFF
    apt->CurrentTaskFile(4) = (sector Shr 16) And &HFF
    apt->CurrentTaskFile(5) = (sector Shr 24) And &HFF
    apt->CurrentTaskFile(6) = (sector Shr 32) And &HFF
    apt->CurrentTaskFile(7) = (sector Shr 40) And &HFF

    apt->CurrentTaskFile(6) = ATA_CMD_WRITE_SECTORS_EXT

    Dim br As ULong
    Dim ok As WINBOOL = DeviceIoControl(This.m_handle, IOCTL_ATA_PASS_THROUGH, _
        packet, packetSize, packet, packetSize, @br, NULL)

    Deallocate(packet)
    If ok Then Return sectorSize Else Return -1
End Function

' ATA识别
Sub StorageDevice.ATA_Identify()
    Dim totalSize As ULong = SizeOf(ATA_PASS_THROUGH_EX) + 512
    Dim memBlock As UByte Ptr = Allocate(totalSize)
    If memBlock = NULL Then Exit Sub
    
    Clear(memBlock, 0, totalSize)
    
    Dim apt As ATA_PASS_THROUGH_EX Ptr = CPtr(ATA_PASS_THROUGH_EX Ptr, memBlock)
    Dim dataBuffer As UByte Ptr = memBlock + SizeOf(ATA_PASS_THROUGH_EX)
    
    apt->Length = SizeOf(ATA_PASS_THROUGH_EX)
    apt->AtaFlags = ATA_FLAGS_DATA_IN
    apt->DataTransferLength = 512
    apt->TimeOutValue = 10
    apt->DataBufferOffset = SizeOf(ATA_PASS_THROUGH_EX)
    apt->CurrentTaskFile(6) = &HEC  ' IDENTIFY DEVICE
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, IOCTL_ATA_PASS_THROUGH, _
                      memBlock, totalSize, memBlock, totalSize, _
                      @bytesReturned, NULL) Then
        Dim model(41) As Byte
        For i As Integer = 0 To 39 Step 2
            model(i) = dataBuffer[54 + i + 1]
            model(i + 1) = dataBuffer[54 + i]
        Next
        model(40) = 0
        MyLog.PrintInfo "StorageDevice.ATA_Identify",, "型号: " & *CPtr(ZString Ptr, @model(0))
    End If
    
    Deallocate(memBlock)
End Sub

' ATA S.M.A.R.T.
Function StorageDevice.ATA_SMART() As Integer
    ' SMART READ DATA (0xB0, 0xD0)
    Dim totalSize As ULong = SizeOf(ATA_PASS_THROUGH_EX) + 512
    Dim memBlock As UByte Ptr = Allocate(totalSize)
    If memBlock = NULL Then Return 0
    
    Clear(memBlock, 0, totalSize)
    
    Dim apt As ATA_PASS_THROUGH_EX Ptr = CPtr(ATA_PASS_THROUGH_EX Ptr, memBlock)
    Dim dataBuffer As UByte Ptr = memBlock + SizeOf(ATA_PASS_THROUGH_EX)
    
    apt->Length = SizeOf(ATA_PASS_THROUGH_EX)
    apt->AtaFlags = ATA_FLAGS_DATA_IN
    apt->DataTransferLength = 512
    apt->TimeOutValue = 10
    apt->DataBufferOffset = SizeOf(ATA_PASS_THROUGH_EX)
    apt->CurrentTaskFile(0) = &HD0
    apt->CurrentTaskFile(1) = 1
    apt->CurrentTaskFile(6) = &HB0
    
    Dim bytesReturned As ULong
    Dim success As Integer = DeviceIoControl(This.m_handle, IOCTL_ATA_PASS_THROUGH, _
                                            memBlock, totalSize, memBlock, totalSize, _
                                            @bytesReturned, NULL)
    
    Deallocate(memBlock)
    Return success
End Function

' SCSI读取实现
Function StorageDevice.SCSI_Read(sector As ULongInt, buffer As UByte Ptr) As Integer
    Dim sptd As SCSI_PASS_THROUGH_DIRECT
    Clear(@sptd, 0, SizeOf(SCSI_PASS_THROUGH_DIRECT))
    
    sptd.Length = SizeOf(SCSI_PASS_THROUGH_DIRECT)
    sptd.CdbLength = 10
    sptd.DataIn = SCSI_IOCTL_DATA_IN
    sptd.DataTransferLength = 512
    sptd.TimeOutValue = 10
    sptd.DataBuffer = Cast(ULONG_PTR, buffer)
    
    sptd.Cdb(0) = &H28       ' READ(10)
    sptd.Cdb(2) = (sector Shr 24) And &HFF
    sptd.Cdb(3) = (sector Shr 16) And &HFF
    sptd.Cdb(4) = (sector Shr 8) And &HFF
    sptd.Cdb(5) = sector And &HFF
    sptd.Cdb(8) = 1
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, _
                      @sptd, SizeOf(sptd), @sptd, SizeOf(sptd), _
                      @bytesReturned, NULL) Then
        Return 512
    Else
        MyLog.PrintWin32Error "StorageDevice.SCSI_Read", "IOCTL_SCSI_PASS_THROUGH_DIRECT", "sector=" & sector
        Return -1
    End If
End Function

' SCSI写入实现
Function StorageDevice.SCSI_Write(sector As ULongInt, buffer As UByte Ptr) As Integer
    Dim sptd As SCSI_PASS_THROUGH_DIRECT
    Clear(@sptd, 0, SizeOf(SCSI_PASS_THROUGH_DIRECT))
    
    sptd.Length = SizeOf(SCSI_PASS_THROUGH_DIRECT)
    sptd.CdbLength = 10
    sptd.DataIn = SCSI_IOCTL_DATA_OUT
    sptd.DataTransferLength = 512
    sptd.TimeOutValue = 10
    sptd.DataBuffer = Cast(ULONG_PTR, buffer)
    
    sptd.Cdb(0) = &H2A       ' WRITE(10)
    sptd.Cdb(2) = (sector Shr 24) And &HFF
    sptd.Cdb(3) = (sector Shr 16) And &HFF
    sptd.Cdb(4) = (sector Shr 8) And &HFF
    sptd.Cdb(5) = sector And &HFF
    sptd.Cdb(8) = 1
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, _
                      @sptd, SizeOf(sptd), @sptd, SizeOf(sptd), _
                      @bytesReturned, NULL) Then
        Return 512
    End If
    
    Return -1
End Function

' SCSI查询
Sub StorageDevice.SCSI_Inquiry()
    Dim sptd As SCSI_PASS_THROUGH_DIRECT
    Clear(@sptd, 0, SizeOf(SCSI_PASS_THROUGH_DIRECT))
    
    Dim buffer(255) As UByte
    
    sptd.Length = SizeOf(SCSI_PASS_THROUGH_DIRECT)
    sptd.CdbLength = 6
    sptd.DataIn = SCSI_IOCTL_DATA_IN
    sptd.DataTransferLength = 256
    sptd.TimeOutValue = 10
    sptd.DataBuffer = Cast(ULONG_PTR, @buffer(0))
    
    sptd.Cdb(0) = &H12       ' INQUIRY
    sptd.Cdb(4) = 255
    
    Dim bytesReturned As ULong
    If DeviceIoControl(This.m_handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, _
                      @sptd, SizeOf(sptd), @sptd, SizeOf(sptd), _
                      @bytesReturned, NULL) Then
        MyLog.PrintInfo "StorageDevice.SCSI_Inquiry",, "设备类型: " & buffer(0)
        MyLog.PrintInfo "StorageDevice.SCSI_Inquiry",, "厂商: " & *CPtr(ZString Ptr, @buffer(8))
        MyLog.PrintInfo "StorageDevice.SCSI_Inquiry",, "产品: " & *CPtr(ZString Ptr, @buffer(16))
    End If
End Sub

' SCSI S.M.A.R.T. (简化版)
Function StorageDevice.SCSI_Smart() As Integer
    ' SCSI没有统一的SMART,这里发送LOG SENSE
    Dim sptd As SCSI_PASS_THROUGH_DIRECT
    Clear(@sptd, 0, SizeOf(SCSI_PASS_THROUGH_DIRECT))
    
    Dim buffer(255) As UByte
    
    sptd.Length = SizeOf(SCSI_PASS_THROUGH_DIRECT)
    sptd.CdbLength = 10
    sptd.DataIn = SCSI_IOCTL_DATA_IN
    sptd.DataTransferLength = 256
    sptd.TimeOutValue = 10
    sptd.DataBuffer = Cast(ULONG_PTR, @buffer(0))
    
    sptd.Cdb(0) = &H4D       ' LOG SENSE
    sptd.Cdb(2) = &H2F       ' Informational Exceptions页
    
    Dim bytesReturned As ULong
    Return DeviceIoControl(This.m_handle, IOCTL_SCSI_PASS_THROUGH_DIRECT, _
                          @sptd, SizeOf(sptd), @sptd, SizeOf(sptd), _
                          @bytesReturned, NULL)
End Function

Function StorageDevice.NVMERead(sector As ULongInt, buffer As Any Ptr) As Long
    Dim cmd As NVME_RW_COMMAND
    ZeroMemory(@cmd, SizeOf(cmd))

    cmd.Opcode = &H02          ' NVME_OP_READ
    cmd.NSID = 1               ' 常见：Namespace 1
    cmd.SLBA = sector
    cmd.NLB = 0                ' 0 = 1 logical block

    Dim totalSize As ULong = _
        SizeOf(STORAGE_PROTOCOL_COMMAND) + _
        SizeOf(cmd) + _
        sectorSize

    Dim packet As UByte Ptr = Allocate(totalSize)
    If packet = NULL Then Return -1
    ZeroMemory(packet, totalSize)

    Dim stpc As STORAGE_PROTOCOL_COMMAND Ptr
    stpc = Cast(STORAGE_PROTOCOL_COMMAND Ptr, packet)

    stpc->Version = STORAGE_PROTOCOL_STRUCTURE_VERSION
    stpc->Length = SizeOf(STORAGE_PROTOCOL_COMMAND)
    stpc->ProtocolType = HD_NVME
    stpc->dwFlags = STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST
    stpc->CommandLength = SizeOf(cmd)
    stpc->DataFromDeviceTransferLength = sectorSize
    stpc->TimeOutValue = 10

    stpc->CommandBufferOffset = SizeOf(STORAGE_PROTOCOL_COMMAND)
    stpc->DataFromDeviceBufferOffset = _
        SizeOf(STORAGE_PROTOCOL_COMMAND) + SizeOf(cmd)

    CopyMemory(packet + stpc->CommandBufferOffset, @cmd, SizeOf(cmd))

    Dim br As ULong
    Dim ok As WINBOOL = DeviceIoControl( _
        This.m_handle, _
        IOCTL_STORAGE_PROTOCOL_COMMAND, _
        packet, totalSize, _
        packet, totalSize, _
        @br, _
        NULL)

    If ok Then
        CopyMemory(buffer, _
                   packet + stpc->DataFromDeviceBufferOffset, _
                   sectorSize)
        Deallocate(packet)
        Return sectorSize
    End If

    Deallocate(packet)
    Return -1
End Function

Function StorageDevice.NVMEWrite(sector As ULongInt, buffer As Any Ptr) As Long
    Dim cmd As NVME_RW_COMMAND
    ZeroMemory(@cmd, SizeOf(cmd))

    cmd.Opcode = &H01          ' NVME_OP_WRITE
    cmd.NSID = 1
    cmd.SLBA = sector
    cmd.NLB = 0               ' 1 block

    Dim totalSize As ULong = _
        SizeOf(STORAGE_PROTOCOL_COMMAND) + _
        SizeOf(cmd) + _
        sectorSize

    Dim packet As UByte Ptr = Allocate(totalSize)
    If packet = NULL Then Return -1
    ZeroMemory(packet, totalSize)

    Dim stpc As STORAGE_PROTOCOL_COMMAND Ptr
    stpc = Cast(STORAGE_PROTOCOL_COMMAND Ptr, packet)

    stpc->Version = STORAGE_PROTOCOL_STRUCTURE_VERSION
    stpc->Length = SizeOf(STORAGE_PROTOCOL_COMMAND)
    stpc->ProtocolType = HD_NVME
    stpc->dwFlags = STORAGE_PROTOCOL_COMMAND_FLAG_ADAPTER_REQUEST
    stpc->CommandLength = SizeOf(cmd)
    stpc->DataToDeviceTransferLength = sectorSize
    stpc->TimeOutValue = 10

    stpc->CommandBufferOffset = SizeOf(STORAGE_PROTOCOL_COMMAND)
    stpc->DataToDeviceBufferOffset = _
        SizeOf(STORAGE_PROTOCOL_COMMAND) + SizeOf(cmd)

    CopyMemory(packet + stpc->CommandBufferOffset, @cmd, SizeOf(cmd))
    CopyMemory(packet + stpc->DataToDeviceBufferOffset, buffer, sectorSize)

    Dim br As ULong
    Dim ok As WINBOOL = DeviceIoControl( _
        This.m_handle, _
        IOCTL_STORAGE_PROTOCOL_COMMAND, _
        packet, totalSize, _
        packet, totalSize, _
        @br, _
        NULL)

    Deallocate(packet)
    If ok Then Return sectorSize Else Return -1
End Function

' 静态检测方法
Function StorageDevice.DetectType(drivePath As String) As Integer
    Dim hDevice As HANDLE = CreateFile(drivePath, _
        GENERIC_READ Or GENERIC_WRITE, FILE_SHARE_READ Or FILE_SHARE_WRITE, _
        NULL, OPEN_EXISTING, 0, NULL)
    
    If hDevice = INVALID_HANDLE_VALUE Then Return HD_UNKNOWN
    
    ' 尝试ATA命令
    Dim ataCmd As ATA_PASS_THROUGH_EX
    Clear(@ataCmd, 0, SizeOf(ATA_PASS_THROUGH_EX))
    ataCmd.Length = SizeOf(ATA_PASS_THROUGH_EX)
    ataCmd.AtaFlags = ATA_FLAGS_DATA_IN
    ataCmd.DataTransferLength = 512
    ataCmd.TimeOutValue = 5
    
    Dim bytesReturned As ULong
    Dim testBuffer(1023) As UByte
    
    ' **关键修正：使用单块缓冲区**
    If DeviceIoControl(hDevice, IOCTL_ATA_PASS_THROUGH, _
                      @ataCmd, SizeOf(ATA_PASS_THROUGH_EX), _
                      @ataCmd, SizeOf(ATA_PASS_THROUGH_EX), _
                      @bytesReturned, NULL) Then
        CloseHandle(hDevice)
        Return HD_ATA
    Else
        MyLog.PrintWin32Error "StorageDevice.DetectType", "IOCTL_ATA_PASS_THROUGH", "drivePath=" & drivePath
    End If
    
    ' 尝试SCSI命令
    Dim scsiCmd As SCSI_PASS_THROUGH_DIRECT
    Clear(@scsiCmd, 0, SizeOf(SCSI_PASS_THROUGH_DIRECT))
    scsiCmd.Length = SizeOf(SCSI_PASS_THROUGH_DIRECT)
    scsiCmd.DataIn = SCSI_IOCTL_DATA_IN
    scsiCmd.DataTransferLength = 512
    scsiCmd.TimeOutValue = 5
    scsiCmd.DataBuffer = Cast(ULONG_PTR, @testBuffer(0))
    
    If DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH_DIRECT, _
                      @scsiCmd, SizeOf(SCSI_PASS_THROUGH_DIRECT), _
                      @scsiCmd, SizeOf(SCSI_PASS_THROUGH_DIRECT), _
                      @bytesReturned, NULL) Then
        CloseHandle(hDevice)
        Return HD_SCSI
    Else
        MyLog.PrintWin32Error "StorageDevice.DetectType", "IOCTL_SCSI_PASS_THROUGH_DIRECT", "drivePath=" & drivePath
    End If
    
    ' NVMe 判断
    Dim proto As STORAGE_PROPERTY_QUERY
    Dim desc As STORAGE_DEVICE_DESCRIPTOR
    proto.PropertyId = StorageDeviceProperty
    proto.QueryType = PropertyStandardQuery

    If DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, _
        @proto, SizeOf(proto), _
        @desc, SizeOf(desc), @bytesReturned, NULL) Then
        CloseHandle(hDevice)
        Return HD_NVME
    End If
    
    CloseHandle(hDevice)
    Return HD_UNKNOWN
End Function

' =============================================================
' 主程序演示
' =============================================================
'Sub TestPassThrough()
'    Print "存储设备Passthrough类库测试"
'    Print "============================"
'    
'    ' 创建设备对象
'    Dim devPath As String = "\\.\PhysicalDrive0"
'    Dim dev As StorageDevice = StorageDevice(devPath)
'    
'    Print "设备路径: " & devPath
'    
'    ' 打开设备
'    If Not dev.Open Then
'        Print "错误: 无法打开设备"
'        Print "可能原因: 1) 设备不存在 2) 被其他程序占用"
'        Exit Sub
'    End If
'    
'    ' 显示设备信息
'    Print !"\n=== 设备信息 ==="
'    Print "设备类型: " & dev.GetTypeName()
'    Print "扇区大小: " & dev.GetSectorSize() & " 字节"
'    
'    ' 查询详细信息
'    dev.QueryInfo
'    
'    ' 读取测试
'    Print !"\n=== 读取测试 ==="
'    Dim buffer(511) As UByte
'    Dim bytesRead As Integer = dev.ReadSector(0, @buffer(0))
'    
'    If bytesRead > 0 Then
'        Print "MBR读取成功！"
'        Print "签名: " & Hex(buffer(510), 2) & Hex(buffer(511), 2)
'        
'        ' 显示分区表
'        Print !"\n分区表信息:"
'        For i As Integer = 0 To 3
'            Dim offset As Integer = 446 + i * 16
'            Dim partType As Byte = buffer(offset + 4)
'            If partType <> 0 Then
'                Print "  分区" & i + 1 & ": 类型=" & Hex(partType, 2)
'            End If
'        Next
'    Else
'        Print "读取失败,bytesRead=" & bytesRead & "错误码: " & GetLastError
'    End If
'    
'    ' S.M.A.R.T.查询
'    Print !"\n=== S.M.A.R.T.状态 ==="
'    If dev.QuerySMART Then
'        Print "设备支持S.M.A.R.T.健康监测"
'    Else
'        Print "无法获取S.M.A.R.T.信息"
'    End If
'    
'    ' 写入测试：仅测试前16扇区（存在PartMgr防护）
'    Print !"\n=== 写入测试（仅前16扇区）==="
'    Print "警告: 直接写入磁盘扇区具有风险！"
'    Print "Windows限制: RING3只能写入前16扇区(0-15)"
'    Print "目标扇区: 15 (相对安全的保留扇区)"
'    
'    Dim confirm As String
'    Input "输入 'YES' 确认写入测试: ", confirm
'    
'    If confirm = "YES" Then
'        ' 先锁定设备
'        If Not dev.Lock Then
'            Print "错误: 无法锁定设备,可能被其他程序占用"
'        Else
'            Print "设备已锁定,正在写入..."
'            
'            ' 修改数据（仅修改前8字节）
'            For i As Integer = 0 To 7
'                buffer(i) = &H42  ' 写入测试标记 'B'
'            Next
'            
'            ' 写入到扇区1（MBR备份位置,相对安全）
'            Dim bytesWritten As Integer = dev.WriteSector(1, @buffer(0))
'            
'            If bytesWritten > 0 Then
'                Print "写入成功！写入 " & bytesWritten & " 字节到扇区1"
'                
'                ' 验证写入
'                Dim verifyBuffer(511) As UByte
'                dev.ReadSector(1, @verifyBuffer(0))
'                
'                If verifyBuffer(0) = &H42 Then
'                    Print "数据验证通过！"
'                Else
'                    Print "警告: 数据验证失败"
'                End If
'            Else
'                Print "写入失败,错误码: " & GetLastError
'            End If
'            
'            ' 解锁
'            dev.Unlock
'            Print "设备已解锁"
'        End If
'    Else
'        Print "已取消写入测试"
'    End If
'    
'    Print !"\n测试完成,按任意键退出..."
'End Sub