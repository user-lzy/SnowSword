#include Once "win/winioctl.bi"

Const FS_TYPE_UNKNOWN = 0
Const FS_TYPE_NTFS    = 1
Const FS_TYPE_FAT32 = 2

' 分区表相关（放在常量定义区）
Const MBR_SIGNATURE = &HAA55
Const MBR_PARTITION_TYPE_NTFS = &H07
Const MBR_PARTITION_TYPE_EXTENDED = &H05
Const MBR_PARTITION_TYPE_EXTENDED_LBA = &H0F
Const MBR_PARTITION_TYPE_GPT_PROTECTIVE = &HEE

' GPT 相关
Const GPT_HEADER_LBA = 1

Type MBR_PARTITION_ENTRY Field = 1
    bootIndicator   As UByte
    startHead       As UByte
    startSector     As UByte
    startCylinder   As UByte
    partType        As UByte
    endHead         As UByte
    endSector       As UByte
    endCylinder     As UByte
    startLBA        As ULong
    totalSectors    As ULong
End Type

Type MBR_SECTOR Field = 1
    bootCode(0 To 445) As UByte
    partitions(0 To 3) As MBR_PARTITION_ENTRY
    signature       As UShort
End Type

Type GPT_HEADER Field = 1
    signature(0 To 7) As UByte
    revision        As ULong
    headerSize      As ULong
    headerCRC32     As ULong
    reserved        As ULong
    myLBA           As ULongLong
    alternateLBA    As ULongLong
    firstUsableLBA  As ULongLong
    lastUsableLBA   As ULongLong
    diskGUID(0 To 15) As UByte
    partEntryLBA    As ULongLong
    numPartEntries  As ULong
    partEntrySize   As ULong
    partArrayCRC32  As ULong
End Type

Type GPT_PARTITION_ENTRY Field = 1
    typeGUID(0 To 15)   As UByte
    uniqueGUID(0 To 15) As UByte
    firstLBA            As ULongLong
    lastLBA             As ULongLong
    attributes          As ULongLong
    name(0 To 35)       As UShort
End Type

' 无盘符分区信息（GetDriveList 展示用）
Type HIDDEN_PARTITION_INFO
    diskIndex       As Integer
    partitionIndex  As Integer     ' ← 新增：分区序号（1-based）
    startLBA        As ULongLong
    totalSectors    As ULongLong
    bytesPerSector  As ULong
    partitionName As String
    fsType  As Long   ' ← 新增
End Type

' 物理分区路径前缀常量
Const PHYSICAL_DRIVE_PREFIX = "\\.\PhysicalDrive"

' 路径解析结果（内部使用）
Type PARSED_PATH_INFO
    isPhysicalPath    As Boolean    ' 是否是物理路径格式
    diskIndex         As Integer    ' 物理磁盘号（物理路径有效）
    partitionIndex    As Integer    ' 分区序号（物理路径有效，1-based）
    driveLetter       As String * 2  ' 盘符（传统路径有效，如 "C"）
    relativePath      As String     ' 分区内的相对路径（不含前缀和盘符）
    startLBA          As ULongLong  ' 分区起始 LBA（解析后填充）
    bytesPerSector    As ULong      ' 扇区大小（解析后填充）
End Type

' 全局无盘符分区数组
Dim Shared g_HiddenParts(Any) As HIDDEN_PARTITION_INFO
Dim Shared g_HiddenPartCount As Integer = 0

' ============================================================================
' 分区信息（与文件系统无关）
' ============================================================================
Type PARTITION_INFO
    partitionIndex  As Integer      ' 分区序号（1-based）
    startLBA        As ULongLong
    totalSectors    As ULongLong
    bytesPerSector  As ULong
    fsType          As UByte        ' FS_TYPE_NTFS / FAT32 / UNKNOWN
    partitionName   As String       ' 显示名称
    drvLetter       As String       ' 盘符（无则为空）
    hasVolumeCtx    As Boolean      ' 是否已成功打开卷上下文
End Type

' ============================================================================
' 磁盘信息
' ============================================================================
Type DISK_INFO
    diskIndex       As Integer
    isGPT           As Boolean
    bytesPerSector  As ULong
    totalSectors    As ULongLong
    partitions(Any)    As PARTITION_INFO
    partitionCount  As Integer
End Type

' ============================================================================
' 全局磁盘列表
' ============================================================================
Dim Shared g_DiskList() As DISK_INFO
Dim Shared g_DiskCount As Integer = 0

Type SECTORIO_REQUEST
    DiskNumber    As ULONG      ' 物理磁盘号（0-based）
    DiskOffset    As LONGLONG   ' 物理磁盘字节偏移
    Length        As ULONG      ' 读写长度（512 对齐）
    DataOffset    As ULONG      ' 写操作专用：数据偏移
    Reserved(1)   As ULONG      ' 保留对齐
End Type

' ============================================================================
' 统一扇区读取（适用于 NTFS、FAT32 等任何文件系统）
' ============================================================================
Function DiskIO_Read(io As DISK_IO_CONTEXT Ptr, _
                     ByVal offset As ULongLong, _
                     ByVal buf As Any Ptr, _
                     ByVal size As ULong) As Boolean
    
    If buf = NULL Or size = 0 Then Print "参数非法" : Return False
    
    ' 物理偏移 = 分区相对偏移 + 分区在磁盘上的起始偏移
    Dim actualOffset As ULongLong = offset + io->partitionStartOffset
    If IsDriverLoaded Then
        ' 驱动模式
        If (size Mod io->bytesPerSector) <> 0 Then
            Print "未对齐:size=" & size & " bytesPerSector:" & io->bytesPerSector
            io->lastError = ERROR_INVALID_PARAMETER
            Return False
        End If

        Dim req As SECTORIO_REQUEST
        ZeroMemory(@req, SizeOf(req))
        
        req.DiskNumber = io->diskIndex
        req.DiskOffset = actualOffset
        req.Length = size

        Dim bytesReturned As DWORD
        Dim bSuccess As BOOL = IoControl(hDrv, IOCTL_SectorIo_Read, @req, SizeOf(req), buf, size, @bytesReturned)
        
        If bSuccess = 0 Then
            Print "[DiskIO_Read]IOCTL_SectorIo_Read failed, err=" & GetLastError
            io->lastError = GetLastError()
            Return False
        End If
        If bytesReturned <> size Then
            Print "[DiskIO_Read]IOCTL_SectorIo_Read failed, bytRead=" & bytesReturned
            io->lastError = ERROR_IO_INCOMPLETE
            Return False
        End If
        Return True
        
    Else
        ' 回退：直接文件句柄
        Dim li As LARGE_INTEGER
        li.QuadPart = actualOffset
        
        If SetFilePointerEx(io->hVol, li, NULL, FILE_BEGIN) = 0 Then
            io->lastError = GetLastError()
            Return False
        End If
        
        Dim bytesRead As DWORD
        If ReadFile(io->hVol, buf, size, @bytesRead, NULL) = 0 Then
            Print "[DiskIO_Read]ReadFile failed, err=" & GetLastError
            io->lastError = GetLastError()
            Return False
        End If
        Return (bytesRead = size)
    End If
End Function

' ============================================================================
' 统一扇区写入
' ============================================================================
Function DiskIO_Write(io As DISK_IO_CONTEXT Ptr, _
                      ByVal offset As ULongLong, _
                      ByVal buf As Any Ptr, _
                      ByVal size As ULong) As Boolean
    
    If buf = NULL Or size = 0 Then Return False
    
    Dim actualOffset As ULongLong = offset + io->partitionStartOffset
    
    If IsDriverLoaded Then
        If (size Mod io->bytesPerSector) <> 0 Then
            Print "[DiskIO_Write]未对齐"
            io->lastError = ERROR_INVALID_PARAMETER
            Return False
        End If

        Dim totalSize As ULong = SizeOf(SECTORIO_REQUEST) + size
        Dim sendBuf As Any Ptr = Allocate(totalSize)
        If sendBuf = NULL Then
            io->lastError = ERROR_NOT_ENOUGH_MEMORY
            Return False
        End If

        Dim req As SECTORIO_REQUEST Ptr = sendBuf
        ZeroMemory(sendBuf, totalSize)
        
        req->DiskNumber = io->diskIndex
        req->DiskOffset = actualOffset
        req->Length = size
        req->DataOffset = SizeOf(SECTORIO_REQUEST)

        memcpy(CPtr(Any Ptr, CPtr(ULONG_PTR, sendBuf) + req->DataOffset), buf, size)

        Dim bytesReturned As DWORD
        Dim bSuccess As BOOL = IoControl(hDrv, IOCTL_SectorIo_Write, _
            sendBuf, totalSize, NULL, 0, @bytesReturned)
        
        Dim lastErr As DWORD = GetLastError()
        Deallocate sendBuf
        
        If bSuccess = 0 Then
            Print "[DiskIO_Write]IOCTL_SectorIo_Write failed, err=" & GetLastError
            SetLastError(lastErr)
            io->lastError = lastErr
            Return False
        End If
        Return True
        
    Else
        Dim li As LARGE_INTEGER
        li.QuadPart = actualOffset
        
        If SetFilePointerEx(io->hVol, li, NULL, FILE_BEGIN) = 0 Then
            io->lastError = GetLastError()
            Return False
        End If
        
        Dim bytesWritten As DWORD
        If WriteFile(io->hVol, buf, size, @bytesWritten, NULL) = 0 Then
            io->lastError = GetLastError()
            Return False
        End If
        Return (bytesWritten = size)
    End If
End Function

' ============================================================================
' 统一初始化 DISK_IO_CONTEXT
' ============================================================================
Private Function InitDiskIOContext(io As DISK_IO_CONTEXT Ptr, _
                                    ByVal path As WString Ptr, _
                                    ByVal partitionStartOffset As ULongLong, _
                                    ByVal bytesPerSec As ULong, _
                                    ByVal diskIdx As Integer) As Boolean
    
    io->hVol = CreateFile(path, GENERIC_READ Or GENERIC_WRITE, _
                      FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                      NULL, OPEN_EXISTING, 0, NULL)
    
    If io->hVol = INVALID_HANDLE_VALUE Then Return False
    
    io->bytesPerSector = bytesPerSec
    io->partitionStartOffset = partitionStartOffset
    io->diskIndex = diskIdx
    
    ' 提取 VolName
    Dim p As WString Ptr = path
    Dim lenPath As Integer = LenW(*p)
    If lenPath >= 2 Then
        io->VolName = MidW(*p, lenPath - 1, 2)
    End If
    
    Return True
End Function

'======================================================================
' 通过物理磁盘号和分区起始偏移，查询 Windows 分配的盘符
' 返回：盘符字符（如 "C"），无盘符返回 ""
'======================================================================
Function QueryDriveLetter(ByVal diskIndex As Integer, _
                          ByVal startOffset As ULongLong) As String
    
    ' 分配足够大的缓冲区（支持最多 8 个 Extent）
    ' 大小 = 头部大小 + 7 个额外 Extent 的大小
    Const MAX_EXTENTS = 8
    Dim bufSize As Integer = SizeOf(VOLUME_DISK_EXTENTS) + _
                             (MAX_EXTENTS - 1) * SizeOf(DISK_EXTENT)
    
    ' 用字节数组模拟足够大的结构体
    Dim buf(0 To bufSize - 1) As UByte
    Dim pExtents As VOLUME_DISK_EXTENTS Ptr = Cast(VOLUME_DISK_EXTENTS Ptr, @buf(0))
    Dim bytesReturned As DWORD
    
    For ch As Integer = Asc("A") To Asc("Z")
        Dim volPath As WString * 16 = "\\.\" & Chr(ch) & ":"
        Dim hVol As HANDLE = CreateFile(@volPath, GENERIC_READ, _
            FILE_SHARE_READ Or FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
        If hVol = INVALID_HANDLE_VALUE Then Continue For
        
        If DeviceIoControl(hVol, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, _
            NULL, 0, pExtents, bufSize, @bytesReturned, NULL) Then
            
            ' 遍历实际返回的 Extent 数量
            For j As Integer = 0 To pExtents->NumberOfDiskExtents - 1
                ' 通过指针偏移访问 Extents[j]
                Dim pExtent As DISK_EXTENT Ptr = _
                    Cast(DISK_EXTENT Ptr, @pExtents->Extents(0)) + j
                
                If pExtent->DiskNumber = diskIndex And _
                   pExtent->StartingOffset.QuadPart = startOffset Then
                    CloseHandle hVol
                    Return Chr(ch)
                End If
            Next
        End If
        CloseHandle hVol
    Next
    Return ""
End Function

Function IsGPTDisk(ByVal hDisk As HANDLE, ByVal bytesPerSec As ULong) As Boolean
    Dim pGPTBuf As UByte Ptr = Allocate(512)
    Dim bytRead As DWORD
    
    ' 使用实际扇区大小跳到 LBA 1
    'Print "bytesPerSec:" & bytesPerSec
    Dim offset As LARGE_INTEGER
    offset.QuadPart = bytesPerSec
    
    SetLastError 0
    If SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN) = 0 Then Print "IsGPTDisk: SetFilePointerEx失败, err=" & GetLastError()
    
    SetLastError 0
    Dim readOK As Boolean = (ReadFile(hDisk, pGPTBuf, bytesPerSec, @bytRead, NULL) <> 0)
    
    If Not readOK Then
        Print "IsGPTDisk: ReadFile失败, err=" & GetLastError()
        Deallocate pGPTBuf
        Return False
    End If
    
    Dim As GPT_HEADER Ptr gpt = Cast(GPT_HEADER Ptr, pGPTBuf)
    
    ' 验证 "EFI PART"
    Dim As String sig
    
    ' 检查签名
    sig = String(8, 0)
    For i As Integer = 0 To 7
        sig[i] = gpt->Signature(i)
    Next
    
    If sig <> "EFI PART" Then
        Print "GPT签名不匹配: " & sig
        Deallocate pGPTBuf
        Return False
    End If
    
    Deallocate pGPTBuf
    Return True
End Function

Private Function DetectFileSystem(hDisk As HANDLE, startLBA As ULongLong, bytesPerSec As ULong) As UByte
    Dim pBuf As UByte Ptr = Allocate(512)
    Dim li As LARGE_INTEGER
    li.QuadPart = startLBA * bytesPerSec
    If SetFilePointerEx(hDisk, li, NULL, FILE_BEGIN) = 0 Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    Dim br As DWORD
    If ReadFile(hDisk, pBuf, 512, @br, NULL) = 0 Or br <> 512 Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    
    ' 检查签名
    If pBuf[510] <> &H55 Or pBuf[511] <> &HAA Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    
    ' 检查 NTFS OEM ID (偏移 0x03)
    Dim oemID As String = ""
    For i As Integer = 0 To 7
        oemID += Chr(pBuf[3 + i])
    Next
    If oemID = "NTFS    " Then
        Deallocate pBuf
        Return FS_TYPE_NTFS
    End If
    
    ' 检查 FAT32 BPB 特征
    ' FAT32: rootEntryCount=0, fatSize16=0, fatSize32!=0
    Dim rootEntryCount As UShort = *CPtr(UShort Ptr, @pBuf[&h11])
    Dim fatSize16 As UShort = *CPtr(UShort Ptr, @pBuf[&h16])
    Dim fatSize32 As ULong = *CPtr(ULong Ptr, @pBuf[&h24])
    
    If rootEntryCount = 0 And fatSize16 = 0 And fatSize32 <> 0 Then
        ' 进一步确认：检查 fileSysType 字段 (偏移 0x52)
        Dim fsType As String = ""
        For i As Integer = 0 To 7
            fsType += Chr(pBuf[&h52 + i])
        Next
        If InStr(fsType, "FAT32") > 0 Then
            Deallocate pBuf
            Return FS_TYPE_FAT32
        End If
    End If
    
    Deallocate pBuf
    Return FS_TYPE_UNKNOWN
End Function

' ============================================================================
' 检测指定 LBA 处的文件系统类型
' 返回: FS_TYPE_NTFS / FS_TYPE_FAT32 / FS_TYPE_UNKNOWN
' ============================================================================
Private Function DetectFileSystemAtLBA(ByVal hDisk As HANDLE, _
                                       ByVal startLBA As ULongLong, _
                                       ByVal bytesPerSec As ULong) As UByte
    
    Dim pBuf As UByte Ptr = Allocate(512)
    Dim li As LARGE_INTEGER
    li.QuadPart = startLBA * bytesPerSec
    
    If SetFilePointerEx(hDisk, li, NULL, FILE_BEGIN) = 0 Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    
    Dim br As DWORD
    If ReadFile(hDisk, pBuf, 512, @br, NULL) = 0 Or br <> 512 Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    
    ' 签名检查
    If pBuf[510] <> &H55 Or pBuf[511] <> &HAA Then
        Deallocate pBuf
        Return FS_TYPE_UNKNOWN
    End If
    
    ' ===== 检查 NTFS =====
    Dim oemID As String = ""
    For i As Integer = 0 To 7
        oemID += Chr(pBuf[3 + i])
    Next
    If oemID = "NTFS    " Then
        Deallocate pBuf
        Return FS_TYPE_NTFS
    End If
    
    ' ===== 检查 FAT32 =====
    ' FAT32 BPB 特征：rootEntryCount=0 (0x11-0x12), fatSize16=0 (0x16-0x17), fatSize32!=0 (0x24-0x27)
    Dim rootEntryCount As UShort = *CPtr(UShort Ptr, @pBuf[&h11])
    Dim fatSize16 As UShort = *CPtr(UShort Ptr, @pBuf[&h16])
    Dim fatSize32 As ULong = *CPtr(ULong Ptr, @pBuf[&h24])
    
    If rootEntryCount = 0 And fatSize16 = 0 And fatSize32 <> 0 Then
        ' 进一步验证 fileSysType 字段
        Dim fsTypeStr As String = ""
        For i As Integer = 0 To 7
            fsTypeStr += Chr(pBuf[&h52 + i])
        Next
        ' 可能是 "FAT32   " 或空（某些格式化工具不填）
        If InStr(fsTypeStr, "FAT32") > 0 Or fatSize32 > 0 Then
            Return FS_TYPE_FAT32
        End If
    End If
    
    ' ===== 检查 exFAT（可选）=====
    ' exFAT: 偏移 0x03 = "EXFAT   " (注意不是 BPB 位置)
    Dim exfatOem As String = ""
    For i As Integer = 0 To 6
        exfatOem += Chr(pBuf[3 + i])
    Next
    /'If exfatOem = "EXFAT  " Then
        Deallocate pBuf
        Return FS_TYPE_EXFAT
    End If'/
    
    Deallocate pBuf
    Return FS_TYPE_UNKNOWN
End Function

' ============================================================================
' 分区显示名称生成（统一格式：名称(分区索引)）
' ============================================================================

' MBR 分区类型 → 显示名称
Function GetMBRPartitionTypeName(ByVal partType As UByte) As String
    Select Case partType
    Case &h07:       Return "本地磁盘"
    Case &h0B, &h0C: Return "FAT32分区"
    Case &h0E:       Return "FAT16分区"
    Case &hEE:       Return "GPT Protective"
    Case &h27, &h17: Return "隐藏NTFS"
    Case &h82:       Return "Linux Swap"
    Case &h83:       Return "Linux"
    Case &h05, &h0F: Return "扩展分区"
    Case Else:       Return "未知分区"
    End Select
End Function

' GPT 常见 Type GUID → 显示名称（GUID 按小端字节序存储）
Function GetGPTPartitionTypeName(ByVal typeGUID As UByte Ptr) As String
    ' Microsoft Basic Data: EBD0A0A2-B9E5-4433-87C0-68B6B72699C7
    Dim basicData(15) As UByte = {&hA2,&hA0,&hD0,&hEB,&hE5,&hB9,&h33,&h44,&h87,&hC0,&h68,&hB6,&hB7,&h26,&h99,&hC7}
    ' EFI System: C12A7328-F81F-11D2-BA4B-00A0C93EC93B
    Dim efiSys(15) As UByte = {&h28,&h73,&h2A,&hC1,&h1F,&h8F,&hD2,&h11,&hBA,&h4B,&h00,&hA0,&hC9,&h3E,&hC9,&h3B}
    ' MSR: E3C9E316-0B5C-4DB8-817D-F92DF00215AE
    Dim msr(15) As UByte = {&h16,&hE3,&hC9,&hE3,&h0B,&h5C,&h4D,&hB8,&h81,&h7D,&hF9,&h2D,&hF0,&h02,&h15,&hAE}
    ' Recovery: DE94BBA4-06D1-4D40-A16A-BFD50179D6AC
    Dim recovery(15) As UByte = {&hA4,&hBB,&h94,&hDE,&h06,&hD1,&h40,&h4D,&hA1,&h6A,&hBF,&hD5,&h01,&h79,&hD6,&hAC}
    
    Dim i As Integer
    Dim match As Boolean = True
    
    ' 比较 Basic Data
    For i = 0 To 15
        If typeGUID[i] <> basicData(i) Then
            match = False 
            Exit For
        End If
    Next
    If match Then Return "本地磁盘"
    
    match = True
    For i = 0 To 15
        If typeGUID[i] <> efiSys(i) Then
            match = False
            Exit For
        End If
    Next
    If match Then Return "EFI System Partition"
    
    match = True
    For i = 0 To 15
        If typeGUID[i] <> msr(i) Then
            match = False
            Exit For
        End If
    Next
    If match Then Return "Microsoft Reserved Partition"
    
    match = True
    For i = 0 To 15
        If typeGUID[i] <> recovery(i) Then
            match = False
            Exit For
        End If
    Next
    If match Then Return "Windows Recovery"
    
    Return ""
End Function

' 文件系统类型 → 后备显示名称
Function GetFSFallbackName(ByVal fsType As UByte) As String
    If fsType = FS_TYPE_NTFS Then Return "NTFS分区"
    If fsType = FS_TYPE_FAT32 Then Return "FAT32分区"
    Return "未知分区"
End Function

' 统一生成无盘符分区显示名称
' 参数:
'   baseName    - GPT Partition Name 或 MBR 类型名（可能为空）
'   fsType      - 检测到的文件系统类型
'   partIndex   - 分区索引（1-based）
'   isGPT       - 是否为 GPT 分区
'   typeGUID    - GPT 类型 GUID（仅 GPT 有效，可为 NULL）
'   partType    - MBR 分区类型 ID（仅 MBR 有效）
' ============================================================================
Function BuildHiddenPartitionName(ByVal baseName As String, _
                                  ByVal fsType As UByte, _
                                  ByVal partIndex As Integer, _
                                  ByVal isGPT As Boolean, _
                                  ByVal typeGUID As UByte Ptr, _
                                  ByVal partType As UByte) As String
    
    Dim displayName As String = Trim(baseName)
    
    ' 如果 baseName 为空，尝试根据 GUID/类型推断
    If displayName = "" Or displayName = "未命名分区" Then
        If isGPT And typeGUID <> NULL Then
            displayName = GetGPTPartitionTypeName(typeGUID)
        Else
            displayName = GetMBRPartitionTypeName(partType)
        End If
    End If
    
    ' 如果还是空（GUID 未识别），用文件系统类型兜底
    If displayName = "" Then
        displayName = GetFSFallbackName(fsType)
    End If
    
    ' 统一格式：名称(分区索引)
    Return displayName & "(" & partIndex - 1 & ")"
End Function

'======================================================================
' 解析 MBR 分区表，填充 NTFS 分区到 g_VolCtx 或 g_HiddenParts
'======================================================================
Sub ParseMBRAndMapVolumes(ByVal diskIndex As Integer, _
                          ByVal hDisk As HANDLE, _
                          ByVal bytesPerSec As ULong, _
                          ByVal diskInfo As DISK_INFO Ptr)
    
    Dim mbrBuf(511) As UByte, bytRead As DWORD
    SetFilePointerEx(hDisk, Type<LARGE_INTEGER>(0,0), NULL, FILE_BEGIN)
    If ReadFile(hDisk, @mbrBuf(0), 512, @bytRead, NULL) = 0 Then Return
    
    Dim mbr As MBR_SECTOR Ptr = Cast(MBR_SECTOR Ptr, @mbrBuf(0))
    If mbr->signature <> MBR_SIGNATURE Then Return
    
    Dim partCount As Integer = 0
    
    For i As Integer = 0 To 3
        With mbr->partitions(i)
            If .partType = 0 Then Continue For
            
            Dim startLBA As ULongLong = CULngInt(.startLBA)
            Dim detectedFS As UByte = DetectFileSystemAtLBA(hDisk, startLBA, bytesPerSec)
            
            ' 只处理支持的文件系统
            If detectedFS <> FS_TYPE_NTFS And detectedFS <> FS_TYPE_FAT32 Then
                Continue For
            End If
            
            Dim drvLetter As String = QueryDriveLetter(diskIndex, startLBA * bytesPerSec)
            
            ' 不再打开卷上下文存入 g_VolCtx，只记录分区信息
            ' 盘符信息已保存在 drvLetter 中，需要时通过 OpenNTFSVolumeContext/OpenNTFSVolumePhysical 动态打开
            
            ' 填充分区信息到 diskInfo
            ReDim Preserve diskInfo->partitions(0 To partCount)
            With diskInfo->partitions(partCount)
                .partitionIndex = i + 1
                .startLBA = startLBA
                .totalSectors = mbr->partitions(i).totalSectors
                .bytesPerSector = bytesPerSec
                .fsType = detectedFS
                .drvLetter = drvLetter
                .hasVolumeCtx = False  ' 不再预打开
                .partitionName = BuildHiddenPartitionName("", detectedFS, i + 1, _
                    False, NULL, mbr->partitions(i).partType)
            End With
            partCount += 1
            
            ' 无盘符的放入隐藏列表
            If drvLetter = "" Then
                ReDim Preserve g_HiddenParts(0 To g_HiddenPartCount)
                With g_HiddenParts(g_HiddenPartCount)
                    .diskIndex = diskIndex
                    .partitionIndex = i + 1
                    .startLBA = startLBA
                    .totalSectors = diskInfo->partitions(partCount - 1).totalSectors
                    .bytesPerSector = bytesPerSec
                    .fsType = detectedFS
                    .partitionName = diskInfo->partitions(partCount - 1).partitionName
                End With
                g_HiddenPartCount += 1
            End If
        End With
    Next
    
    diskInfo->partitionCount = partCount
End Sub

'======================================================================
' 解析 GPT 分区表，填充 NTFS 分区到 g_VolCtx 或 g_HiddenParts
'======================================================================
Sub ParseGPTAndMapVolumes(ByVal diskIndex As Integer, _
                          ByVal hDisk As HANDLE, _
                          ByVal bytesPerSec As ULong, _
                          ByVal diskInfo As DISK_INFO Ptr)
    
    Dim bytRead As DWORD
    
    '------------------------------------------------------------------
    ' 步骤 1：读取 GPT 头（LBA 1）
    '------------------------------------------------------------------
    Dim pGptBuf As UByte Ptr
    pGptBuf = Allocate(512)
    Dim offset As LARGE_INTEGER
    offset.QuadPart = bytesPerSec  ' LBA 1
    
    SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN)
    If ReadFile(hDisk, pGptBuf, bytesPerSec, @bytRead, NULL) = 0 Then
        If g_CuiMode Then Print "PD" & diskIndex & " 读取 GPT 头失败"
        Deallocate pGptBuf
        Return
    End If
    
    Dim gpt As GPT_HEADER Ptr = Cast(GPT_HEADER Ptr, pGptBuf)
    
    ' 验证签名 "EFI PART"
    Dim gptSig As String = "EFI PART"
    For k As Integer = 0 To 7
        If gpt->signature(k) <> Asc(Mid(gptSig, k + 1, 1)) Then
            Print "PD" & diskIndex & " GPT 签名无效"
            Deallocate pGptBuf
            Return
        End If
    Next
    
    Print "PD" & diskIndex & " GPT 头验证通过"
    
    '------------------------------------------------------------------
    ' 步骤 2：读取分区条目数组
    '------------------------------------------------------------------
    Dim entrySize As ULong = gpt->partEntrySize      ' 通常为 128
    Dim numEntries As ULong = gpt->numPartEntries    ' 通常为 128
    Dim entryLBA As ULongLong = gpt->partEntryLBA    ' 通常为 LBA 2
    
    If entrySize = 0 Or numEntries = 0 Then
        Print "PD" & diskIndex & " GPT 分区条目数为0"
        Deallocate pGptBuf
        Return
    End If
    
    ' 计算需要读取的总字节数
    Dim arrayBytes As ULongLong = CULng(numEntries) * CULng(entrySize)
    
    ' 分配缓冲区（向上取整到扇区大小）
    Dim sectorsToRead As ULongLong = (arrayBytes + bytesPerSec - 1) \ bytesPerSec
    Dim bufSize As ULong = CULng(sectorsToRead * bytesPerSec)
    
    ' 用堆分配大缓冲区（避免栈溢出）
    Dim entryBuf As UByte Ptr = Allocate(bufSize)
    If entryBuf = NULL Then
        Print "PD" & diskIndex & " 内存分配失败"
        Deallocate pGptBuf
        Return
    End If
    
    offset.QuadPart = entryLBA * bytesPerSec
    SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN)
    
    Dim readOK As Boolean = (ReadFile(hDisk, entryBuf, bufSize, @bytRead, NULL) <> 0)
    
    If Not readOK Then
        Print "PD" & diskIndex & " 读取分区条目失败"
        Deallocate(entryBuf)
        Deallocate pGptBuf
        Return
    End If
    
    Print "PD" & diskIndex & " 分区条目数: " & numEntries
    
    '------------------------------------------------------------------
    ' 步骤 3：遍历分区条目
    '------------------------------------------------------------------
    Dim partCount As Integer = 0
    
    For i As Integer = 0 To numEntries - 1
        Dim pEntry As GPT_PARTITION_ENTRY Ptr = _
            Cast(GPT_PARTITION_ENTRY Ptr, entryBuf + i * entrySize)
        
        ' 检查空条目（不变）
        Dim isEmpty As Boolean = True
        For k As Integer = 0 To 15
            If pEntry->typeGUID(k) <> 0 Then isEmpty = False : Exit For
        Next
        If isEmpty Then Continue For
        
        Dim startLBA As ULongLong = pEntry->firstLBA
        Dim endLBA As ULongLong = pEntry->lastLBA
        If endLBA < startLBA Then Continue For
        
        Dim totalSectors As ULongLong = endLBA - startLBA + 1
        
        ' 提取 GPT Partition Name（原始名称）
        Dim rawPartName As String = ""
        For n As Integer = 0 To 35
            If pEntry->name(n) = 0 Then Exit For
            rawPartName &= WChr(pEntry->name(n))
        Next
        
        Dim detectedFS As UByte = DetectFileSystemAtLBA(hDisk, startLBA, bytesPerSec)
        
        ' 查询盘符（不变）
        Dim drvLetter As String = QueryDriveLetter(diskIndex, startLBA * CULng(bytesPerSec))
        
        ' 不再打开卷上下文存入 g_VolCtx
        Dim openSuccess As Boolean = False  ' 不再预打开
        
        ' 填充分区信息
        ReDim Preserve diskInfo->partitions(0 To partCount)
        With diskInfo->partitions(partCount)
            .partitionIndex = i + 1
            .startLBA = startLBA
            .totalSectors = totalSectors
            .bytesPerSector = bytesPerSec
            .fsType = detectedFS
            .drvLetter = drvLetter
            .hasVolumeCtx = False  ' 不再预打开
            .partitionName = BuildHiddenPartitionName(rawPartName, detectedFS, i + 1, _
                True, @pEntry->typeGUID(0), 0)
        End With
        partCount += 1
        
        ' 无盘符的放入隐藏列表
        If drvLetter = "" Then
            ReDim Preserve g_HiddenParts(0 To g_HiddenPartCount)
            With g_HiddenParts(g_HiddenPartCount)
                .diskIndex = diskIndex
                .partitionIndex = i + 1
                .startLBA = startLBA
                .totalSectors = totalSectors
                .bytesPerSector = bytesPerSec
                .fsType = detectedFS
                .partitionName = diskInfo->partitions(partCount - 1).partitionName
            End With
            g_HiddenPartCount += 1
        End If
        
    Next
    
    diskInfo->partitionCount = partCount
    
    Deallocate(entryBuf)
    Deallocate pGptBuf
    Print "PD" & diskIndex & " 有效GPT分区: " & partCount
End Sub

' ============================================================================
' 枚举所有物理磁盘，解析分区表，填充 g_DiskList
' 与 UI 完全解耦，只操作全局数据结构
' ============================================================================
Sub EnumerateAllDisks()
    Print "EnumerateAllDisks"
    ' 清空旧数据
    Erase g_DiskList
    g_DiskCount = 0
    Erase g_HiddenParts
    g_HiddenPartCount = 0
    
    Dim diskIdx As Integer
    For diskIdx = 0 To 63
        Dim hDisk As HANDLE = CreateFileW(StrPtrW("\\.\PhysicalDrive" & diskIdx), _
            GENERIC_READ, FILE_SHARE_READ Or FILE_SHARE_WRITE, _
            NULL, OPEN_EXISTING, 0, NULL)
        If hDisk = INVALID_HANDLE_VALUE Then Continue For
        
        ' 获取磁盘几何信息
        Dim dg As DISK_GEOMETRY, bytesReturned As DWORD
        Dim bytesPerSec As ULong = 512
        Dim totalSectors As ULongLong = 0
        
        If DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY, _
            NULL, 0, @dg, SizeOf(dg), @bytesReturned, NULL) Then
            bytesPerSec = dg.bytesPerSector
            If dg.bytesPerSector > 0 Then
                totalSectors = dg.Cylinders.QuadPart * dg.TracksPerCylinder _
                             * dg.SectorsPerTrack
            End If
        End If
        
        ' 检测 GPT
        Dim isGPT As Boolean = IsGPTDisk(hDisk, bytesPerSec)
        
        ' 读取扇区0判断格式
        Dim pBuf As UByte Ptr = Allocate(bytesPerSec)
        SetFilePointerEx(hDisk, Type<LARGE_INTEGER>(0,0), NULL, FILE_BEGIN)
        Dim bytRead As DWORD
        ReadFile(hDisk, pBuf, bytesPerSec, @bytRead, NULL)
        
        Dim hasValidFormat As Boolean = False
        
        ' 扩展 g_DiskList
        ReDim Preserve g_DiskList(0 To g_DiskCount)
        With g_DiskList(g_DiskCount)
            .diskIndex = diskIdx
            .isGPT = isGPT
            .bytesPerSector = bytesPerSec
            .totalSectors = totalSectors
            .partitionCount = 0
        End With
        
        If isGPT Then
            hasValidFormat = True
            ParseGPTAndMapVolumes(diskIdx, hDisk, bytesPerSec, _
                @g_DiskList(g_DiskCount))
        ElseIf pBuf[510] = &H55 And pBuf[511] = &HAA Then
            hasValidFormat = True
            ParseMBRAndMapVolumes(diskIdx, hDisk, bytesPerSec, _
                @g_DiskList(g_DiskCount))
        End If
        
        If hasValidFormat Then
            g_DiskCount += 1
        Else
            ' 无有效格式，回滚（不保留此磁盘）
            If g_DiskCount > 0 Then
                ReDim Preserve g_DiskList(0 To g_DiskCount - 1)
            Else
                Erase g_DiskList
            End If
        End If
        
        CloseHandle hDisk
        Deallocate pBuf
    Next
End Sub

'======================================================================
' 解析任意路径为内部统一格式
' 支持：
'   C:\Windows\System32\...          → 盘符路径
'   C:Windows\System32\...         → 盘符路径（无反斜杠，兼容）
'   \\.\PhysicalDrive0\Partition1\Windows\...  → 物理路径
'   \\.\PhysicalDrive0\Partition1\,PhysicalDrive0\Partition1\             → 物理路径，根目录
'======================================================================
Function ParsePath(ByVal lpPath As String, ByRef info As PARSED_PATH_INFO) As Boolean
    ZeroMemory(@info, SizeOf(info))
    
    If lpPath = "" Then Return False
    
    '========== 规范化路径分隔符 ==========
    ' 统一将 / 替换为 \，避免混合分隔符导致解析错误
    lpPath = YF_Replace(lpPath, "/", "\")
    
    '========== 检测完整物理路径格式: \\.\PhysicalDriveN\... ==========
    If Left(lpPath, Len(PHYSICAL_DRIVE_PREFIX)) = PHYSICAL_DRIVE_PREFIX Then
        info.isPhysicalPath = True
        
        ' 提取 \\.\PhysicalDriveN\PartitionM\...
        Dim pathWithoutPrefix As String = Mid(lpPath, Len(PHYSICAL_DRIVE_PREFIX) + 1)
        ' 此时 pathWithoutPrefix = "0\Partition1\..."
        
        ' 提取磁盘号
        Dim slashPos As Integer = InStr(pathWithoutPrefix, "\")
        If slashPos = 0 Then
            ' 只有磁盘号，没有分区信息，例如: \\.\PhysicalDrive0
            If Not IsNumeric(pathWithoutPrefix) Then Return False
            info.diskIndex = CInt(pathWithoutPrefix)
            info.partitionIndex = -1  ' 标记为未指定分区
            info.relativePath = ""
            Return True
        End If
        
        Dim diskStr As String = Left(pathWithoutPrefix, slashPos - 1)
        If Not IsNumeric(diskStr) Then Return False
        info.diskIndex = CInt(diskStr)
        
        ' 剩余：PartitionM\... 或 其他
        Dim afterDisk As String = Mid(pathWithoutPrefix, slashPos + 1)
        
        ' 必须以 "Partition" 开头（不区分大小写）
        If Left(LCase(afterDisk), 9) <> "partition" Then Return False
        
        Dim partStr As String = Mid(afterDisk, 10)
        slashPos = InStr(partStr, "\")
        
        If slashPos > 0 Then
            ' 有后续路径
            Dim partNumStr As String = Left(partStr, slashPos - 1)
            If Not IsNumeric(partNumStr) Then Return False
            info.partitionIndex = CInt(partNumStr)
            info.relativePath = Mid(partStr, slashPos + 1)
        Else
            ' 只有分区号，无后续路径（根目录）
            If Not IsNumeric(partStr) Then Return False
            info.partitionIndex = CInt(partStr)
            info.relativePath = ""
        End If
        
        Return True
    End If
    
    '========== 检测简化物理路径格式: PhysicalDriveN\PartitionM\... ==========
    ' 不带 \\.\ 前缀的形式
    Dim physPrefix As String = "PhysicalDrive"
    If Left(LCase(lpPath), Len(physPrefix)) = LCase(physPrefix) Then
        info.isPhysicalPath = True
        
        ' 提取 PhysicalDriveN\PartitionM\...
        Dim pathWithoutPrefix As String = Mid(lpPath, Len(physPrefix) + 1)
        ' 此时 pathWithoutPrefix = "0\Partition1\..."
        
        ' 提取磁盘号
        Dim slashPos As Integer = InStr(pathWithoutPrefix, "\")
        If slashPos = 0 Then
            ' 只有磁盘号
            If Not IsNumeric(pathWithoutPrefix) Then Return False
            info.diskIndex = CInt(pathWithoutPrefix)
            info.partitionIndex = -1
            info.relativePath = ""
            Return True
        End If
        
        Dim diskStr As String = Left(pathWithoutPrefix, slashPos - 1)
        If Not IsNumeric(diskStr) Then Return False
        info.diskIndex = CInt(diskStr)
        
        ' 剩余路径
        Dim afterDisk As String = Mid(pathWithoutPrefix, slashPos + 1)
        
        ' 检测 PartitionM 格式
        If Left(LCase(afterDisk), 9) = "partition" Then
            Dim partStr As String = Mid(afterDisk, 10)
            slashPos = InStr(partStr, "\")
            
            If slashPos > 0 Then
                Dim partNumStr As String = Left(partStr, slashPos - 1)
                If Not IsNumeric(partNumStr) Then Return False
                info.partitionIndex = CInt(partNumStr)
                info.relativePath = Mid(partStr, slashPos + 1)
            Else
                If Not IsNumeric(partStr) Then Return False
                info.partitionIndex = CInt(partStr)
                info.relativePath = ""
            End If
        Else
            ' 非 Partition 格式，可能是直接路径如 PhysicalDrive0\Windows\...
            ' 根据你的需求决定是否支持，这里暂不支持
            Return False
        End If
        
        Return True
    End If
    
    '========== 检测盘符路径格式 ==========
    ' 支持 "C:\"、"C:/"、"C:" 开头
    If Len(lpPath) >= 2 Then
        Dim firstChar As String = UCase(Left(lpPath, 1))
        Dim secondChar As String = Mid(lpPath, 2, 1)
        
        If secondChar = ":" Then
            If firstChar >= "A" And firstChar <= "Z" Then
                info.isPhysicalPath = False
                info.driveLetter = firstChar
                
                ' 提取相对路径
                If Len(lpPath) > 2 Then
                    ' 跳过 "C:" 或 "C:\"
                    If Mid(lpPath, 3, 1) = "\" Then
                        info.relativePath = Mid(lpPath, 4)
                    Else
                        info.relativePath = Mid(lpPath, 3)
                    End If
                Else
                    info.relativePath = ""
                End If
                
                Return True
            End If
        End If
    End If
    
    Return False
End Function

'======================================================================
' 对外接口：将盘符路径转换为物理路径格式
' 示例：C:\Windows\System32 → \\.\PhysicalDrive0\Partition1\Windows\System32
' 返回：转换后的路径，失败返回空字符串
'======================================================================
Function ConvertToPhysicalPath(ByVal lpPath As String) As String
    
    Dim parsed As PARSED_PATH_INFO
    If Not ParsePath(lpPath, parsed) Then Return ""
    
    If parsed.isPhysicalPath Then Return lpPath  ' 已经是物理路径
    
    ' 查 g_DiskList 找盘符对应的分区
    'Print "parsed.driveLetter:" & parsed.driveLetter
    For d As Integer = 0 To g_DiskCount - 1
        For p As Integer = 0 To g_DiskList(d).partitionCount - 1
            Dim part As PARTITION_INFO Ptr = @g_DiskList(d).partitions(p)
            'Print "part->drvLetter:" & part->drvLetter
            If UCase(Trim(part->drvLetter)) = UCase(Trim(parsed.driveLetter)) Then
                Print "[ConvertToPhysicalPath]找到"
                Dim result As String = PHYSICAL_DRIVE_PREFIX & g_DiskList(d).diskIndex & "\Partition" & part->partitionIndex
                'Print "result:" & result
                If parsed.relativePath <> "" Then
                    result &= "\" & parsed.relativePath
                End If
                Return result
            End If
        Next
    Next
    
    Return ""  ' 未找到
End Function

'======================================================================
' 对外接口：将物理路径转换为盘符路径（反向）
' 示例：\\.\PhysicalDrive0\Partition1\Windows → C:\Windows
' 返回：转换后的路径，无对应盘符返回空字符串
'====================================================================== 
Function ConvertToDriveLetterPath(ByVal lpPath As String) As String
    
    Dim parsed As PARSED_PATH_INFO
    If Not ParsePath(lpPath, parsed) Then Return ""
    
    If Not parsed.isPhysicalPath Then Return lpPath  ' 已经是盘符路径
    
    ' 查 g_DiskList 找匹配的物理分区
    For d As Integer = 0 To g_DiskCount - 1
        If g_DiskList(d).diskIndex <> parsed.diskIndex Then Continue For
        
        For p As Integer = 0 To g_DiskList(d).partitionCount - 1
            Dim part As PARTITION_INFO Ptr = @g_DiskList(d).partitions(p)
            If part->partitionIndex <> parsed.partitionIndex Then Continue For
            
            If part->drvLetter = "" Then Return ""  ' 无盘符
            
            If parsed.relativePath = "" Then
                Return part->drvLetter & ":\"
            Else
                Return part->drvLetter & ":\" & parsed.relativePath
            End If
        Next
    Next
    
    Return ""  ' 未找到
End Function

Function GetDirList(lpDirPath As String, fileInfoList() As WIN32_FILE_DATAEX2W) As Long
    Dim parsed As PARSED_PATH_INFO
    If Not ParsePath(lpDirPath, parsed) Then Print "ParsePath " & lpDirPath & " failed" : Return -1
    Print "lpDirPath:" & lpDirPath
    
    '========== 盘符路径：直接打开 ==========
    If Not parsed.isPhysicalPath Then
        Dim idx As Integer = Asc(UCase(parsed.driveLetter)) - Asc("A")
        If idx < 0 Or idx > 25 Then Print "idx非法" : Return -1
        
        ' 先尝试 NTFS
        Dim ctxNTFS As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpDirPath)
        If ctxNTFS <> NULL Then
            Print "ctxNTFS <> NULL"
            Dim result As Long = NTFS_GetDirList_Internal(ctxNTFS, lpDirPath, fileInfoList())
            CloseNTFSVolumeContext(ctxNTFS)  ' 立即关闭
            
            Return result
        End If
        Print "ctxNTFS = NULL"
        
        ' 再尝试 FAT32
        Dim ctxFAT32 As FAT32_CONTEXT Ptr = OpenFAT32VolumeContext(lpDirPath)
        If ctxFAT32 <> NULL Then
            Print "ctxFAT32 <> NULL"
            Dim result As Long = FAT32_GetDirList_Internal(ctxFAT32, lpDirPath, fileInfoList())
            CloseFAT32VolumeContext(ctxFAT32)  ' 立即关闭
            
            ' ===== 修复：清空全局缓存中的悬空指针 =====
            If g_FAT32_VolCtx(idx) = ctxFAT32 Then
                g_FAT32_VolCtx(idx) = NULL
            End If
            
            Return result
        End If
        
        Return -1
    End If
    
    '========== 物理路径：查 g_HiddenParts / g_DiskList ==========
    Print "parsed.diskIndex:" & parsed.diskIndex & " parsed.partitionIndex:" & parsed.partitionIndex
    
    ' 先查 g_HiddenParts
    For i As Integer = 0 To g_HiddenPartCount - 1
        If g_HiddenParts(i).diskIndex <> parsed.diskIndex Then Continue For
        If g_HiddenParts(i).partitionIndex <> parsed.partitionIndex Then Continue For
        
        Dim internalPath As String = "X:\" & parsed.relativePath
        
        Select Case g_HiddenParts(i).fsType
        Case FS_TYPE_NTFS
            Dim tempCtx As NTFS_CONTEXT Ptr = OpenNTFSVolumePhysical( _
                parsed.diskIndex, g_HiddenParts(i).startLBA, g_HiddenParts(i).bytesPerSector)
            If tempCtx = NULL Then Return -1
            
            Dim result As Long = NTFS_GetDirList_Internal(tempCtx, internalPath, fileInfoList())
            CloseNTFSVolumeContext(tempCtx)
            Return result
            
        Case FS_TYPE_FAT32
            Print "找到了,是FAT32"
            Dim tempCtx As FAT32_CONTEXT Ptr = OpenFAT32VolumePhysical( _
                parsed.diskIndex, g_HiddenParts(i).startLBA, g_HiddenParts(i).bytesPerSector)
            If tempCtx = NULL Then Return -1
            Print "打开成功"
            Dim result As Long = FAT32_GetDirList_Internal(tempCtx, internalPath, fileInfoList())
            CloseFAT32VolumeContext(tempCtx)
            Return result
            
        Case Else
            Return -1
        End Select
    Next
    
    ' 未在 g_HiddenParts 中找到，查 g_DiskList（有盘符的分区）
    For d As Integer = 0 To g_DiskCount - 1
        If g_DiskList(d).diskIndex <> parsed.diskIndex Then Continue For
        
        For p As Integer = 0 To g_DiskList(d).partitionCount - 1
            Dim part As PARTITION_INFO Ptr = @g_DiskList(d).partitions(p)
            If part->partitionIndex <> parsed.partitionIndex Then Continue For
            
            Dim internalPath As String = "X:\" & parsed.relativePath
            
            Select Case part->fsType
            Case FS_TYPE_NTFS
                Dim tempCtx As NTFS_CONTEXT Ptr = OpenNTFSVolumePhysical( _
                    parsed.diskIndex, part->startLBA, part->bytesPerSector)
                If tempCtx = NULL Then Return -1
                
                Dim result As Long = NTFS_GetDirList_Internal(tempCtx, internalPath, fileInfoList())
                CloseNTFSVolumeContext(tempCtx)
                Return result
                
            Case FS_TYPE_FAT32
                Dim tempCtx As FAT32_CONTEXT Ptr = OpenFAT32VolumePhysical( _
                    parsed.diskIndex, part->startLBA, part->bytesPerSector)
                If tempCtx = NULL Then Return -1
                
                Dim result As Long = FAT32_GetDirList_Internal(tempCtx, internalPath, fileInfoList())
                CloseFAT32VolumeContext(tempCtx)
                Return result
                
            Case Else
                Return -1
            End Select
        Next
    Next
    
    Return -1
End Function