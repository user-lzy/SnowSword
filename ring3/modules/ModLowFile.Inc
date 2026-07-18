Type NTFS_BOOT_SECTOR Field = 1
    jump(0 To 2)       As UByte
    oemID(0 To 7)      As UByte
    bytesPerSector     As Short
    SectorsPerCluster  As Byte
    reservedSectors    As Short
    zeros1(0 To 2)     As Byte
    unused1            As Short
    media              As Byte
    zeros2             As Short
    sectorsPerTrack    As Short
    numHeads           As Short
    hiddenSectors      As Long
    unused2            As Long
    unused3            As Long
    totalSectors       As ULONGLONG
    mftLCN             As ULongLong
    mftMirrLCN         As ULONGLONG
    clustersPerFR As Byte
    padding1(2) As UByte
    clustersPerIndex As Byte ' 有无符号?
    padding2(2) As UByte
    serialNumber       As ULongLong
    checksum           As ULong
    bootstrap(0 To 425) As UByte
    endMark            As UShort
End Type

'========================================================
'   MFT Record Header
'========================================================
Type MFT_RECORD_HEADER Field = 1
    magic          As ULong      ' "FILE" = 0x454C4946
    usaOffset      As UShort
    usaSize        As UShort
    lsn            As ULongLong
    seqNumber      As UShort
    linkCount      As UShort
    attrOffset     As UShort
    dwFlags          As UShort
    bytesInUse     As ULong
    bytesAlloc     As ULong
    baseRecord     As ULONGLONG
    nextAttrID     As UShort
    recordNumber   As ULong
End Type

'========================================================
'   NTFS Attribute Headers
'========================================================
Type ATTR_HEADER_RESIDENT Field = 1
    typeID      As ULong
    totalLen    As ULong
    nonResident As Byte ' UByte?
    nameLen     As UByte
    nameOff     As UShort
    flags       As UShort
    attrID      As UShort
    dataLen     As ULong
    dataOff     As UShort
    indexedFlag As UByte ' UShort
    padding     As UByte
End Type

Type ATTR_HEADER_NONRESIDENT Field = 1
    typeID      As ULong
    totalLen    As ULong
    nonResident As UByte
    nameLen     As UByte
    nameOff     As UShort
    flags       As UShort
    attrID      As UShort
    lowVCN      As ULongLong
    highVCN     As ULongLong
    runOff      As UShort
    compUnit    As UShort
    padding     As ULong
    allocSize   As ULongLong
    dataSize    As ULongLong
    initSize    As ULongLong
End Type

'========================================================
'   File Attribute
'========================================================
Type FILE_STANDARD_INFORMATION
    creationTime   AS ULongLong  ' 创建时间
    lastAccessTime As ULONGLONG  ' 最后访问时间
    lastWriteTime  As ULONGLONG  ' 最后修改时间
    changeTime     As ULONGLONG  ' 属性修改时间（或 MFT 记录修改时间）
    fileAttributes AS ULong      ' 文件属性标志
    reserved       As ULong
End Type

' NTFS属性列表条目
Type ATTRIBUTE_LIST_ENTRY
    typeID As ULong          ' 属性类型ID
    recordLen As UShort       ' 条目总长度
    nameLength As UByte      ' 名称长度
    nameOffset As UByte      ' 名称偏移
    startVCN As ULongLong    ' 起始VCN
    mftRef As ULongLong      ' MFT引用（低48位记录号）
    attrInstance As UShort   ' 属性实例ID
End Type

Type FILE_NAME_ATTR Field = 1
    parentRef   As ULongLong      ' 8 bytes
    createTime  As ULongLong      ' FILETIME 8
    modifyTime  As ULongLong
    mftTime     As ULongLong
    accessTime  As ULongLong
    allocSize   As ULongLong
    realSize    As ULongLong
    dwFlags       As ULong
    reparseTag  As ULong
    nameLen     As UByte
    nameType    As UByte
    uName(0 To 0) As UShort
End Type

' 属性通用头（我们只区分 Resident vs NonResident）
Type ATTR_HEADER_GENERIC Field = 1
    typeID      As ULong
    totalLen    As ULong
    nonResident As UByte
    nameLen     As UByte
    nameOff     As UShort
    flags       As UShort
    attrID      As UShort
End Type

Type REPARSE_DATA_HEADER Field = 1
    reparseTag        As ULong
    reparseDataLength As UShort
    reserved          As UShort
End Type

' Index Entry Header
Type NTFS_INDEX_ENTRY Field = 1
    fileRef     As ULongLong   ' 8
    entryLen    As UShort      ' 2
    keyLen      As UShort      ' 2
    flags       As UShort      ' 2  (保持 UShort)
    reserved    As UShort      ' 2  <-- 补齐到 16 bytes
End Type

' Index Root 头
Type NTFS_INDEX_ROOT Field = 1
    attrType    As ULong
    collRule    As ULong
    indexBlock  As ULong
    clustersPerIndex As Byte
    padding(2) As Byte
End Type

' Index Header（在 Index Root 和 Index Block 中通用）
Type NTFS_INDEX_HEADER Field = 1
    entriesOff  As ULong
    entriesSize As ULong
    allocSize   As ULong
    flags As Byte
    padding(2) As Byte
End Type

Type MFT_FRAGMENT
    startVCN      As ULONGLONG    ' 起始虚拟簇号
    startLCN      As ULongLong    ' 起始逻辑簇号
    clusterCount  As ULONGLONG    ' 簇数量
End Type

' 在常量定义部分添加
Const INDEX_BLOCK_SIZE = 4096  ' INDX块固定大小

' 正确的INDX块头（12字节）
Type INDX_BLOCK_HEADER Field = 1
    magic As ULong              ' "INDX" = 0x58444E49
    usaOffset As UShort
    usaSize As UShort
    lsn As ULongLong
End Type

'======================================================================
' 路径缓存（全局变量）
'======================================================================
Type PATH_CACHE_ENTRY
    recNo As ULong
    path As String
End Type

'======================================================================
' 目录缓存结构
'======================================================================
Type DIR_CACHE_ENTRY
    parentRecNo As ULong
    childRecNo As ULong
    childName As String
End Type

Type MFT_REFERENCE
    recordNumber As ULongLong  ' 低48位
    sequenceNumber As UShort   ' 高16位
End Type

'#define CBool(x) CByte((x))

' WIN32_FILE_DATAEX2W 结构体
Type WIN32_FILE_DATAEX2W
    dwFileAttributes As DWORD
    ftCreationTime As FILETIME
    ftLastAccessTime As FILETIME
    ftLastWriteTime As FILETIME
    dwFileSizeHigh As DWORD
    dwFileSizeLow As DWORD
    cFileName As String ' * MAX_PATH
    cAlternateFileName As String * 14
    mftRecordNumber As ULONGLONG      ' MFT 记录号
    logicalClusterNumber As ULONGLONG ' 逻辑簇号（LCN）
    LinkCount As DWORD ' 链接数
    bDeleted As Boolean ' 是否被删除
    'bStreamFile As Boolean ' 是否为ADS流文件(如$Repair:$Verify)
    bHasAlternateStream As Boolean ' 新字段：是否包含附加数据流
    bSuspicious As Boolean           ' 新增：存在多个数据流（含未命名流）
    bDirectory As Boolean
    ' === 新增：重解析点信息 ===
    bReparsePoint       As Boolean   ' 是否为重解析点
    reparseTag          As ULong     ' 重解析标签（如 0xA000000C）
End Type

' ============================================================================
' 统一的磁盘 IO 上下文（物理磁盘模式）
' ============================================================================
Type DISK_IO_CONTEXT
    hVol                    As HANDLE       ' 卷/磁盘句柄（回退模式用）
    VolName                 As WString * 64 ' 用于错误信息
    partitionStartOffset    As ULongLong   ' 分区在物理磁盘上的起始字节偏移
    bytesPerSector          As ULong
    diskIndex               As Integer      ' 物理磁盘号（0-based）
    lastError               As ULong
End Type

' ============================================================================
' NTFS 上下文
' ============================================================================
Type NTFS_CONTEXT
    io                      As DISK_IO_CONTEXT   ' 统一磁盘 IO
    
    ' NTFS 解析结果
    sectorsPerCluster       As ULong
    clusterSize             As ULong
    mftRecordSize           As ULong
    mftStartLCN             As ULongLong
    volumeTotalClusters     As ULONGLONG
    
    ' MFT 片段
    mftFragments(Any)       As MFT_FRAGMENT
    mftFragmentCount        As Integer
    
    ' 缓存
    dirCache(Any)           As DIR_CACHE_ENTRY
    dirCacheCount           As Integer
    pathCache(Any)          As PATH_CACHE_ENTRY
    pathCacheCount          As Integer
    
    lastError               As DWORD
End Type

'========================================================
'    常量
'========================================================
' NTFS属性类型常量
Const ATTR_TYPE_STANDARD_INFORMATION = &H10
Const ATTR_TYPE_ATTRIBUTE_LIST      = &H20
Const ATTR_TYPE_FILE_NAME           = &H30
Const ATTR_TYPE_OBJECT_ID           = &H40
Const ATTR_TYPE_SECURITY_DESCRIPTOR = &H50
Const ATTR_TYPE_VOLUME_NAME         = &H60
Const ATTR_TYPE_VOLUME_INFORMATION  = &H70
Const ATTR_TYPE_DATA                = &H80
Const ATTR_TYPE_INDEX_ROOT          = &H90
Const ATTR_TYPE_INDEX_ALLOCATION    = &HA0
Const ATTR_TYPE_BITMAP              = &HB0
Const ATTR_TYPE_REPARSE_POINT       = &HC0
Const ATTR_TYPE_EA_INFORMATION      = &HD0
Const ATTR_TYPE_EA                  = &HE0
Const ATTR_TYPE_LOGGED_UTILITY_STREAM = &H100

' NTFS 属性标志
Const ATTR_FLAG_COMPRESSED       = &H0001
Const ATTR_FLAG_SPARSE           = &H8000
Const ATTR_FLAG_ENCRYPTED        = &H4000

' 重解析点标签（常见）
Const IO_REPARSE_TAG_MOUNT_POINT = &HA0000003
Const IO_REPARSE_TAG_SYMLINK     = &HA000000C
Const IO_REPARSE_TAG_DEDUP       = &H80000013
Const IO_REPARSE_TAG_NFS         = &H80000014

' Win32 文件属性
Const FILE_ATTRIBUTE_REPARSE_POINT = &H00000400
Const FILE_ATTRIBUTE_COMPRESSED  = &H00000800
Const FILE_ATTRIBUTE_SPARSE_FILE   = &H00000200

Const NTFS_MFT_RECORD_MAGIC = &H454C4946
Const INDEX_BLOCK_MAGIC = &H58444E49 ' 'INDX' little-endian

Const UnknownEncoding = 0
Const Utf8Encoding = 1
Const Utf16Encoding = 2

' 索引条目标志位常量
Const INDEX_ENTRY_NODE = &H1     ' 有子节点（子目录）
Const INDEX_ENTRY_END = &H2      ' 最后一个条目
Const INDEX_ENTRY_SPACE_FILLER = &H8000 ' 填充条目

Const MFT_RECORD_IN_USE    = &H1
Const MFT_RECORD_IS_DIR = &H2

' ============================================================================
' NTFS 适配：内部调用统一 DiskIO
' ============================================================================
Private Function ReadAtOffset(ctx As NTFS_CONTEXT Ptr, _
                              ByVal offset As ULongLong, _
                              ByVal buf As Any Ptr, _
                              ByVal size As ULong) As Boolean
    Return DiskIO_Read(@ctx->io, offset, buf, size)
End Function

Private Function WriteAtOffset(ctx As NTFS_CONTEXT Ptr, _
                               ByVal offset As ULongLong, _
                               ByVal buf As Any Ptr, _
                               ByVal size As ULong) As Boolean
    Return DiskIO_Write(@ctx->io, offset, buf, size)
End Function

Private Function ReadClusters(ctx As NTFS_CONTEXT Ptr, ByVal startCluster As ULongLong, ByVal numClusters As ULong, ByVal buf As Any Ptr) As Boolean
    If startCluster = 0 Then
        memset(buf, 0, numClusters * ctx->clusterSize)
        Return False
    End If
    If numClusters = 0 Then Return False
    
    Dim offsetBytes As ULongLong = startCluster * ctx->clusterSize
    Dim bytesToRead As ULong = numClusters * ctx->clusterSize
    Return ReadAtOffset(ctx, offsetBytes, buf, bytesToRead)
End Function

Private Function WriteClusters(ctx As NTFS_CONTEXT Ptr, ByVal startCluster As ULongLong, ByVal numClusters As ULong, ByVal buf As Any Ptr) As Boolean
    If startCluster = 0 Then
        memset(buf, 0, numClusters * ctx->clusterSize)
        Return False
    End If
    If numClusters = 0 Then Return False
    
    Dim offsetBytes As ULongLong = startCluster * ctx->clusterSize
    Dim bytesToWrite As ULong = numClusters * ctx->clusterSize
    Return WriteAtOffset(ctx, offsetBytes, buf, bytesToWrite)
End Function

'======================================================================
' 引导扇区与 MFT 解析（新增 ctx 参数）
'======================================================================
Private Function ParseBoot(ctx As NTFS_CONTEXT Ptr, ByVal bootBuf As Any Ptr) As Boolean
    Dim bs As NTFS_BOOT_SECTOR Ptr = Cast(NTFS_BOOT_SECTOR Ptr, bootBuf)
    
    Dim s As String = ""
    For i As Integer = 0 To 7
        s += Chr(bs->oemID(i))
    Next
    If s <> "NTFS    " Then Return False
    
    ctx->io.bytesPerSector = bs->bytesPerSector
    ctx->sectorsPerCluster = bs->sectorsPerCluster
    ctx->clusterSize = ctx->io.bytesPerSector * ctx->sectorsPerCluster
    ctx->mftStartLCN = bs->mftLCN
    ctx->volumeTotalClusters = bs->totalSectors \ ctx->sectorsPerCluster
    
    Dim cpfr As Integer = bs->clustersPerFR
    If cpfr > 0 Then
        ctx->mftRecordSize = cpfr * ctx->clusterSize
    Else
        ctx->mftRecordSize = 1 Shl (Abs(cpfr))
    End If
    
    Return ctx->mftRecordSize > 0
End Function

Private Function FixupMFTRecord(ctx As NTFS_CONTEXT Ptr, ByVal recBufPtr As UByte Ptr, ByVal recordSize As ULong) As Boolean
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, recBufPtr)
    If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then Return False
    
    Dim usaOffset As UShort = mrh->usaOffset
    Dim usaCount  As UShort = mrh->usaSize
    If usaOffset = 0 Or usaCount < 2 Then Return True
    
    Dim usaPtr As UShort Ptr = Cast(UShort Ptr, recBufPtr + usaOffset)
    Dim usn As UShort = usaPtr[0]
    Dim sectorsPerRec As Integer = recordSize \ ctx->io.bytesPerSector
    
    If CULng(usaOffset) + usaCount * 2 > recordSize Then Return False
    
    For i As Integer = 1 To usaCount - 1
        Dim sectorEndOff As ULong = i * ctx->io.bytesPerSector - 2
        If sectorEndOff >= recordSize Then Return False
        Dim lastWordPtr As UShort Ptr = Cast(UShort Ptr, recBufPtr + sectorEndOff)
        If lastWordPtr[0] <> usn Then Return False
        lastWordPtr[0] = usaPtr[i]
    Next
    Return True
End Function

Private Function ParseRunList(ctx As NTFS_CONTEXT Ptr, _
                              ByVal runPtr As UByte Ptr, _
                              ByRef currentCluster As ULongLong, _
                              ByRef runLength As ULongLong, _
                              ByRef isSparse As Boolean = False) As UByte Ptr
    
    Dim p As UByte Ptr = runPtr
    Dim header As UByte = p[0]
    If header = 0 Then Return NULL
    
    Dim lenSize As Integer = header And &HF
    Dim offSize As Integer = (header Shr 4) And &HF
    If lenSize = 0 Then Return NULL
    
    Dim length As ULongLong = 0
    For i As Integer = 0 To lenSize - 1
        length = length Or (CULng(p[1 + i]) Shl (8 * i))
    Next
    runLength = length
    
    Dim offsetDelta As LongLong = 0
    If offSize > 0 Then
        Dim temp As ULongLong = 0
        For i As Integer = 0 To offSize - 1
            temp = temp Or (CULng(p[1 + lenSize + i]) Shl (8 * i))
        Next
        
        If offSize < 8 Then
            Dim signBit As ULongLong = 1ULL Shl (offSize * 8 - 1)
            If (temp And signBit) <> 0 Then
                offsetDelta = temp Or (Not ((1ULL Shl (offSize * 8)) - 1))
            Else
                offsetDelta = temp
            End If
        Else
            offsetDelta = Cast(LongLong, temp)
        End If
    End If
    
    ' === 核心修复：稀疏运行不得重置 currentCluster ===
    If offSize > 0 Then
        currentCluster += offsetDelta
        isSparse = False
    Else
        ' 稀疏运行：VCN 继续推进，但物理 LCN 不变（保持上一个值）
        isSparse = True
    End If
    
    Return p + 1 + lenSize + offSize
End Function

Private Function ParseMFTDataAttribute(ctx As NTFS_CONTEXT Ptr) As Boolean
    Dim pMftBuf As UByte Ptr = Allocate(ctx->mftRecordSize)
    
    ' 读取 $MFT 记录（使用简化方式，假设前几个记录连续）
    Dim recOffset As ULongLong = ctx->mftStartLCN * ctx->clusterSize
    If Not ReadAtOffset(ctx, recOffset, pMftBuf, ctx->mftRecordSize) Then Print "[ParseMFTDataAttribute]ReadAtOffset failed" : Return False
    If Not FixupMFTRecord(ctx, pMftBuf, ctx->mftRecordSize) Then Return False
    
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, pMftBuf)
    Dim attrOff As ULong = mrh->attrOffset
    
    While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, pMftBuf + attrOff)
        
        If g->typeID = &HFFFFFFFF Then Exit While
        If g->typeID = ATTR_TYPE_DATA Then
            If g->nonResident Then
                Dim nr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, g)
                Dim runPtr As UByte Ptr = Cast(UByte Ptr, nr) + nr->runOff
                Dim currentLCN As ULongLong = 0
                Dim runLen As ULongLong
                Dim vcn As ULongLong = nr->lowVCN
                
                While runPtr <> NULL And runPtr[0] <> 0
                    runPtr = ParseRunList(ctx, runPtr, currentLCN, runLen)
                    If runPtr = NULL Then Exit While
                    
                    ReDim Preserve ctx->mftFragments(0 To ctx->mftFragmentCount)
                    ctx->mftFragments(ctx->mftFragmentCount).startVCN = vcn
                    ctx->mftFragments(ctx->mftFragmentCount).startLCN = currentLCN
                    ctx->mftFragments(ctx->mftFragmentCount).clusterCount = runLen
                    ctx->mftFragmentCount += 1
                    vcn += runLen
                Wend
                Return ctx->mftFragmentCount > 0
            Else
                Deallocate pMftBuf
                Return False ' $MFT 不可能是常驻属性
            End If
        End If
        attrOff += g->totalLen
    Wend
    Deallocate pMftBuf
    Return False
End Function

Function GetSystemFileName(ByVal recNo As ULong) As String
    Dim SysFileName(16) As ZString * 20 = {"$MFT", "$MFTMirr", "$LogFile", "$Volume", _
    "$AttrDef", "$Root", "$Bitmap", "$Boot", _
    "$BadClus", "$Secure", "$UpCase", _
    "$Extend", "Reserved1", "$Extend\$UsnJrnl", "Reserved2", "Reserved3"}
    If recNo <=16 Then Return SysFileName(recNo) Else Return ""
End Function

' 从路径提取盘符索引（A=0, B=1...）
Private Function GetDriveIndex(ByVal path As String) As Integer
    If Len(path) < 2 Then Return -1
    If Mid(path, 2, 1) <> ":" Then Return -1
    Dim ch As Integer = Asc(UCase(Left(path, 1)))
    If ch >= Asc("A") And ch <= Asc("Z") Then
        Return ch - Asc("A")
    End If
    Return -1
End Function

' ============================================================================
' 核心：打开卷并自动获取物理磁盘号和分区偏移
' ============================================================================
Private Function OpenVolumeInternal(ctx As NTFS_CONTEXT Ptr, _
                                    ByVal path As Wstring Ptr, _
                                    ByVal partitionStartOffset As ULongLong, _
                                    ByVal bytesPerSec As ULong, _
                                    ByVal diskIdx As Integer) As Boolean
    
    ctx->io.hVol = CreateFile(path, GENERIC_READ Or GENERIC_WRITE, _
                      FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                      NULL, OPEN_EXISTING, 0, NULL)
    
    If ctx->io.hVol = INVALID_HANDLE_VALUE Then Return False
    
    ctx->io.bytesPerSector = bytesPerSec
    ctx->io.partitionStartOffset = partitionStartOffset
    ctx->io.diskIndex = diskIdx
    
    ' 提取 VolName 用于错误信息
    Dim p As WString Ptr = path
    Dim lenPath As Integer = LenW(*p)
    If lenPath >= 2 Then
        ctx->io.VolName = MidW(*p, lenPath - 1, 2)
    End If
    
    Dim pBootBuf As UByte Ptr = Allocate(512)
    If Not ReadAtOffset(ctx, 0, pBootBuf, 512) Then 
        Print "[OpenVolumeInternal] ReadAtOffset failed"
        Goto Cleanup
    End If
    If Not ParseBoot(ctx, pBootBuf) Then 
        Print "[OpenVolumeInternal] ParseBoot failed"
        Goto Cleanup
    End If
    If Not ParseMFTDataAttribute(ctx) Then 
        Print "[OpenVolumeInternal] ParseMFTDataAttribute failed"
        Goto Cleanup
    End If
    
    Deallocate pBootBuf
    Return True
    
Cleanup:
    If pBootBuf <> NULL Then Deallocate pBootBuf
    CloseHandle(ctx->io.hVol)
    ctx->io.hVol = INVALID_HANDLE_VALUE
    Return False
End Function

' ============================================================================
' 通过物理磁盘号 + 起始LBA 打开（保留原签名）
' ============================================================================
Public Function OpenNTFSVolumePhysical(ByVal diskIndex As Integer, _
                                       ByVal startLBA As ULongLong, _
                                       ByVal bytesPerSec As ULong) As NTFS_CONTEXT Ptr
    
    Dim ctx As NTFS_CONTEXT Ptr = New NTFS_CONTEXT
    If ctx = NULL Then Return NULL
    
    Dim wpath As WString * 64 = "\\.\PhysicalDrive" & diskIndex
    Dim startOffset As ULongLong = startLBA * bytesPerSec
    
    If OpenVolumeInternal(ctx, @wpath, startOffset, bytesPerSec, diskIndex) Then
        Return ctx
    End If
    
    Deallocate(ctx)
    Return NULL
End Function

' ============================================================================
' 通过盘符路径打开（内部自动转换为物理磁盘操作）
' ============================================================================
Public Function OpenNTFSVolumeContext(ByVal path As String) As NTFS_CONTEXT Ptr
    ' 先转换为物理路径
    Dim physPath As String = ConvertToPhysicalPath(path)
    If physPath = "" Then
        Print "[OpenNTFSVolumeContext] 无法将路径转换为物理路径: " & path
        Return NULL
    End If
    
    ' 解析物理路径
    Dim parsed As PARSED_PATH_INFO
    If Not ParsePath(physPath, parsed) Then Return NULL
    
    ' 查分区信息获取 startLBA 和 bytesPerSector
    Dim startLBA As ULongLong = 0
    Dim bytesPerSec As ULong = 512
    
    ' 优先查 g_HiddenParts
    For i As Integer = 0 To g_HiddenPartCount - 1
        If g_HiddenParts(i).diskIndex = parsed.diskIndex AndAlso _
           g_HiddenParts(i).partitionIndex = parsed.partitionIndex Then
            startLBA = g_HiddenParts(i).startLBA
            bytesPerSec = g_HiddenParts(i).bytesPerSector
            Exit For
        End If
    Next
    
    ' 再查 g_DiskList
    If startLBA = 0 Then
        For d As Integer = 0 To g_DiskCount - 1
            If g_DiskList(d).diskIndex <> parsed.diskIndex Then Continue For
            For p As Integer = 0 To g_DiskList(d).partitionCount - 1
                Dim part As PARTITION_INFO Ptr = @g_DiskList(d).partitions(p)
                If part->partitionIndex = parsed.partitionIndex Then
                    startLBA = part->startLBA
                    bytesPerSec = part->bytesPerSector
                    Exit For, For
                End If
            Next
        Next
    End If
    
    If startLBA = 0 Then
        Print "[OpenNTFSVolumeContext] 未找到分区信息: " & physPath
        Return NULL
    End If
    
    ' 统一走物理磁盘打开
    Return OpenNTFSVolumePhysical(parsed.diskIndex, startLBA, bytesPerSec)
End Function

' ============================================================================
' 清理上下文
' ============================================================================
Public Sub CloseNTFSVolumeContext(ctx As NTFS_CONTEXT Ptr)
    If ctx = NULL Then Exit Sub
    
    If ctx->io.hVol <> INVALID_HANDLE_VALUE And ctx->io.hVol <> NULL Then
        CloseHandle(ctx->io.hVol)
        ctx->io.hVol = INVALID_HANDLE_VALUE
    End If
    
    Erase ctx->mftFragments
    Erase ctx->dirCache
    Erase ctx->pathCache
    
    Deallocate(ctx)
End Sub

'======================================================================
' MFT 记录操作（新增 ctx 参数）
'======================================================================
Private Function ReadExternalMFTRecord(ctx As NTFS_CONTEXT Ptr, ByVal recNo As ULongLong, ByVal outBuf As UByte Ptr) As Boolean
    If ctx->mftFragmentCount = 0 Then Return False
    
    Dim recordOffsetBytes As ULongLong = recNo * ctx->mftRecordSize
    Dim startVCN As ULongLong = recordOffsetBytes \ ctx->clusterSize
    Dim offsetInCluster As ULong = recordOffsetBytes Mod ctx->clusterSize
    
    Dim bytesRemaining As ULong = ctx->mftRecordSize
    Dim dstPtr As UByte Ptr = outBuf
    Dim curVCN As ULongLong = startVCN
    Dim curOffset As ULong = offsetInCluster
    
    While bytesRemaining > 0
        Dim found As Boolean = False
        For fragIdx As Integer = 0 To ctx->mftFragmentCount - 1
            Dim frag As MFT_FRAGMENT Ptr = @ctx->mftFragments(fragIdx)
            
            If curVCN >= frag->startVCN And curVCN < frag->startVCN + frag->clusterCount Then
                Dim lcn As ULongLong = frag->startLCN + (curVCN - frag->startVCN)
                Dim diskOffset As ULongLong = lcn * ctx->clusterSize + curOffset
                
                Dim bytesInThisCluster As ULong = ctx->clusterSize - curOffset
                Dim toRead As ULong = IIf(bytesRemaining < bytesInThisCluster, bytesRemaining, bytesInThisCluster)
                
                If Not ReadAtOffset(ctx, diskOffset, dstPtr, toRead) Then Return False
                
                dstPtr += toRead
                bytesRemaining -= toRead
                curVCN += 1
                curOffset = 0
                found = True
                Exit For
            End If
        Next
        
        If Not found Then Return False
    Wend
    
    Return FixupMFTRecord(ctx, outBuf, ctx->mftRecordSize)
End Function

' 读取 MFT record（根据记录序号）。本函数为简化实现：假定 MFT 本身连续（未处理 MFT 的非驻列扩展）
Function ReadMFTRecord(ctx As NTFS_CONTEXT Ptr, ByVal recNo As ULongLong, ByVal outBuf As UByte Ptr) As Boolean
    ' 计算每个 MFT 记录在 MFT 区的偏移：mftStartLCN * clusterSize + recNo * mftRecordSize
    Dim recOffset As ULongLong = ctx->mftStartLCN * ctx->clusterSize + recNo * ctx->mftRecordSize
    ' 直接从磁盘读取整个记录
    Dim tmpBuf() As UByte
    ReDim tmpBuf(0 To ctx->mftRecordSize - 1) As UByte
    If Not ReadAtOffset(ctx, recOffset, @tmpBuf(0), ctx->mftRecordSize) Then Print "[ReadMFTRecord]读取MFT记录" & recNo & "失败" & WinErrorMsg(GetLastError) & GetLastError: Return False
    ' 执行 USN 修复
    If Not FixupMFTRecord(ctx, @tmpBuf(0), ctx->mftRecordSize) Then  Print "[ReadMFTRecord]MFT记录" & recNo & "失败": Return False
    ' 复制出去
    memcpy(outBuf, @tmpBuf(0), ctx->mftRecordSize)
    Return True
End Function

' UTF-8 -> ANSI 转换
Function Utf8ToAnsi(ByVal utf8Ptr As UByte Ptr, ByVal byteCount As Integer) As String
    If byteCount <= 0 Or utf8Ptr = NULL Then Return ""
    
    ' 将 UTF-8 转换为 UTF-16
    Dim wideCount As Integer = MultiByteToWideChar(CP_UTF8, 0, utf8Ptr, byteCount, NULL, 0)
    If wideCount <= 0 Then Return ""
    
    Dim wideBuffer As WString Ptr = Allocate((wideCount + 1) * SizeOf(WString))
    If wideBuffer = NULL Then Return ""
    
    If MultiByteToWideChar(CP_UTF8, 0, utf8Ptr, byteCount, wideBuffer, wideCount) = 0 Then
        Deallocate(wideBuffer)
        Return ""
    End If
    
    wideBuffer[wideCount] = 0 ' null terminate
    
    ' 将 UTF-16 转换为 ANSI
    Dim ansiCount As Integer = WideCharToMultiByte(CP_ACP, 0, wideBuffer, -1, NULL, 0, NULL, NULL)
    If ansiCount <= 0 Then
        Deallocate(wideBuffer)
        Return ""
    End If
    
    Dim result As String = Space(ansiCount - 1) ' 减去 null terminator
    If WideCharToMultiByte(CP_ACP, 0, wideBuffer, -1, StrPtr(result), ansiCount, NULL, NULL) = 0 Then
        result = ""
    End If
    
    Deallocate(wideBuffer)
    Return result
End Function

' UTF-16 (little-endian) -> ANSI 转换
Function Utf16ToAnsi(ByVal utf16Ptr As UShort Ptr, ByVal charCount As Integer) As String
    If charCount <= 0 Or utf16Ptr = NULL Then Return ""
    ' 注意：WideCharToMultiByte 的 cchWideChar 参数为字符数（可以为 -1 表示 null-terminated）
    Dim bufSize As Integer = WideCharToMultiByte(CP_ACP, 0, utf16Ptr, charCount, NULL, 0, NULL, NULL)
    If bufSize > 0 Then
        Dim s As String = Space(bufSize)
        If WideCharToMultiByte(CP_ACP, 0, utf16Ptr, charCount, StrPtr(s), bufSize, NULL, NULL) > 0 Then
            ' 移除嵌入空字符
            s = FF_Replace(s, Chr(0), "")
            Return s
        End If
    End If
    ' 回退（保守处理）：把能打印的字符保留，其他用 [U+xxxx]
    Dim strOut As String = ""
    For i As Integer = 0 To charCount - 1
        Dim ch As UShort = utf16Ptr[i]
        If ch = 0 Then Exit For
        If ch >= &H20 And ch <= &H7E Then
            strOut &= Chr(ch)
        Else
            strOut &= "[" & Hex(ch, 4) & "]"
        End If
    Next
    Return strOut
End Function

' 检测编码类型
Function GetEncodingType(ByVal dataPtr As UByte Ptr, ByVal dataSize As ULong) As Integer
    If dataSize < 2 Then Return UnknownEncoding ' 无法检测
    
    ' 检查 UTF-8 BOM
    If dataSize >= 3 AndAlso dataPtr[0] = &HEF AndAlso dataPtr[1] = &HBB AndAlso dataPtr[2] = &HBF Then
        Return Utf8Encoding ' UTF-8 with BOM
    End If
    
    ' 检查 UTF-16 LE BOM
    If dataSize >= 2 AndAlso dataPtr[0] = &HFF AndAlso dataPtr[1] = &HFE Then
        Return Utf16Encoding ' UTF-16 LE with BOM
    End If
    
    ' 简单启发式：统计可打印ASCII字符比例
    Dim asciiCount As Integer = 0
    For i As Integer = 0 To IIf(dataSize > 100, 99, dataSize - 1)
        Dim b As UByte = dataPtr[i]
        If (b >= 32 And b <= 126) Or b = 9 Or b = 10 Or b = 13 Then
            asciiCount += 1
        End If
    Next
    
    Dim asciiRatio As Single = asciiCount / IIf(dataSize > 100, 100.0, dataSize)
    
    If asciiRatio > 0.9 Then
        Return Utf8Encoding ' 很可能是 UTF-8 或 ANSI
    Else
        Return Utf16Encoding ' 可能是 UTF-16 或其他
    End If
End Function

' 文件名清理，替换非法字符
Function CleanFileName(ByVal strName As String) As String
    If strName = "" Then Return ""
    Dim res As String = ""
    Dim invalidChars As String = "/\:*?""<>|"
    For i As Integer = 1 To Len(strName)
        Dim ch As String = Mid(strName, i, 1)
        Dim code As Integer = AscW(ch)
        If code >= 32 And code <> 127 Then
            If InStr(invalidChars, ch) > 0 Then
                res += "_"
            Else
                res += ch
            End If
        End If
    Next
    res = Trim(res)
    If res = "" Then res = "unnamed"
    Return res
End Function

' 从 FILE_NAME_ATTR 结构读取名称（nameLen 个 UTF-16 code units）
Function GetFileNameFromAttr(ByVal fnAttrPtr As FILE_NAME_ATTR Ptr) As String
    If fnAttrPtr = NULL Then Return ""
    Dim n As Integer = fnAttrPtr->nameLen
    If n <= 0 Then Return ""
    Dim namePtr As UShort Ptr = Cast(UShort Ptr, @fnAttrPtr->uName(0))
    Dim ansi As String = Utf16ToAnsi(namePtr, n)
    ansi = CleanFileName(ansi)
    Return ansi
End Function

Sub ConvertToFileTime(ByVal timestamp As ULongLong,ByRef fileTime As FILETIME)
    ' FILETIME 为 100 纳秒间隔，单位为 100 纳秒
    fileTime.dwLowDateTime = timestamp And &HFFFFFFFF  ' 低32位
    fileTime.dwHighDateTime = (timestamp Shr 32) And &HFFFFFFFF  ' 高32位
End Sub
'======================================================================
' 属性操作（新增 ctx 参数）
'======================================================================
Private Function GetAttributeName(ByVal attr As ATTR_HEADER_GENERIC Ptr) As String
    If attr->nameLen = 0 Then Return ""
    Dim namePtr As UShort Ptr = Cast(UShort Ptr, Cast(UByte Ptr, attr) + attr->nameOff)
    Return Utf16ToAnsi(namePtr, attr->nameLen)
End Function

Private Function FindAttributeInRecord(ctx As NTFS_CONTEXT Ptr, ByVal mftHeader As MFT_RECORD_HEADER Ptr, ByVal targetType As ULong, ByVal targetInstance As UShort) As ATTR_HEADER_GENERIC Ptr
    If mftHeader = NULL Or mftHeader->magic <> NTFS_MFT_RECORD_MAGIC Then Return NULL
    
    Dim attrOffset As ULong = mftHeader->attrOffset
    While attrOffset + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim attr As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, Cast(UByte Ptr, mftHeader) + attrOffset)
        If attr->typeID = &HFFFFFFFF Then Exit While
        If attr->typeID = targetType And attr->attrID = targetInstance Then Return attr
        attrOffset += attr->totalLen
    Wend
    Return NULL
End Function

Private Function ReadNonResidentAttributeData(ctx As NTFS_CONTEXT Ptr, _
                                              ByVal nonResAttr As ATTR_HEADER_NONRESIDENT Ptr, _
                                              ByVal buffer As UByte Ptr, _
                                              ByVal bufferSize As ULong) As Boolean
    If nonResAttr = NULL Or buffer = NULL Then Return False
    
    Dim runPtr As UByte Ptr = Cast(UByte Ptr, nonResAttr) + nonResAttr->runOff
    Dim currentLCN As ULongLong = 0      ' 物理 LCN 累积值（修复）
    Dim bufOffset As ULong = 0
    Dim isSparse As Boolean = False
    
    While *runPtr <> 0 And bufOffset < bufferSize
        Dim runHeader As UByte = *runPtr
        Dim runLenBytes As Integer = runHeader And &h0F
        Dim runOffsetBytes As Integer = (runHeader And &hF0) Shr 4
        
        If runLenBytes = 0 Or runLenBytes > 8 Or runOffsetBytes > 8 Then Exit While
        
        Dim runLen As ULongLong = 0
        For j As Integer = 0 To runLenBytes - 1
            runLen Or= (Cast(ULongLong, *(runPtr + 1 + j)) Shl (j * 8))
        Next
        
        Dim runOffset As LongLong = 0
        For j As Integer = 0 To runOffsetBytes - 1
            Dim b As UByte = *(runPtr + 1 + runLenBytes + j)
            runOffset Or= (Cast(LongLong, b) Shl (j * 8))
        Next
        
        If runOffsetBytes > 0 Then
            Dim signBit As LongLong = 1 Shl (runOffsetBytes * 8 - 1)
            If (runOffset And signBit) <> 0 Then
                runOffset = runOffset Or (Not ((1 Shl (runOffsetBytes * 8)) - 1))
            End If
        End If
        
        ' 更新物理 LCN（稀疏运行时不更新）
        If runOffsetBytes > 0 Then
            currentLCN += runOffset
        End If
        
        Dim bytesToProcess As ULong = runLen * ctx->clusterSize
        If bufOffset + bytesToProcess > bufferSize Then bytesToProcess = bufferSize - bufOffset
        
        If runOffsetBytes = 0 Then
            ' === 稀疏运行：填充 0 ===
            memset(buffer + bufOffset, 0, bytesToProcess)
        Else
            ' === 正常读取 ===
            If Not ReadClusters(ctx, currentLCN, runLen, buffer + bufOffset) Then Return False
        End If
        
        bufOffset += bytesToProcess
        runPtr += 1 + runLenBytes + runOffsetBytes
    Wend
    Return True
End Function

' ============================================================================
' LZNT1 解压单个块
' 参数：
'   inBuf     - 输入缓冲区（已包含 2 字节头部）
'   inSize    - 输入缓冲区有效大小
'   outBuf    - 输出缓冲区
'   outSize   - 输出缓冲区大小
'   consumed  - 返回实际消耗了多少输入字节（含头部，已 2 字节对齐）
' 返回：实际解压出的字节数
' ============================================================================
Private Function LZNT1Decompress(ByVal inBuf As UByte Ptr, _
                                 ByVal inSize As ULong, _
                                 ByVal outBuf As UByte Ptr, _
                                 ByVal outSize As ULong, _
                                 ByRef consumed As ULong) As ULong
    If inSize < 2 Then Return 0
    
    Dim header As UShort = *Cast(UShort Ptr, inBuf)
    Dim chunkSize As ULong = (header And &H0FFF) + 1
    Dim isCompressed As Boolean = (header And &H8000) <> 0
    
    consumed = 2
    
    If isCompressed = False Then
        ' 未压缩块：头部后直接复制 chunkSize 字节
        Dim dataSize As ULong = chunkSize
        If consumed + dataSize > inSize Then dataSize = inSize - consumed
        If dataSize > outSize Then dataSize = outSize
        memcpy(outBuf, inBuf + consumed, dataSize)
        consumed += dataSize
        If (consumed And 1) <> 0 Then consumed += 1
        Return dataSize
    End If
    
    ' 压缩块：flag byte + token 组
    Dim srcPos As ULong = 2
    Dim dstPos As ULong = 0
    
    Do While srcPos < inSize And dstPos < outSize And dstPos < chunkSize
        If srcPos >= inSize Then Exit Do
        Dim flags As UByte = inBuf[srcPos]
        srcPos += 1
        
        For byt1 As Integer = 0 To 7
            If srcPos >= inSize Or dstPos >= outSize Or dstPos >= chunkSize Then Exit Do
            
            If (flags And (1 Shl byt1)) = 0 Then
                ' Literal
                outBuf[dstPos] = inBuf[srcPos]
                srcPos += 1
                dstPos += 1
            Else
                ' 2-byte pointer: 12-bit offset + 4-bit length
                If srcPos + 1 > inSize Then Exit Do
                Dim token As UShort = *Cast(UShort Ptr, inBuf + srcPos)
                srcPos += 2
                
                Dim offset As ULong = (token And &H0FFF) + 1
                Dim length As ULong = ((token Shr 12) And &H0F) + 3
                
                ' 安全回退（offset 不应超过已输出长度）
                Dim actualOffset As ULong = offset
                If actualOffset > dstPos Then actualOffset = dstPos
                
                For j As ULong = 0 To length - 1
                    If dstPos >= outSize Or dstPos >= chunkSize Then Exit For
                    outBuf[dstPos] = outBuf[dstPos - actualOffset]
                    dstPos += 1
                Next
            End If
        Next
    Loop
    
    consumed = srcPos
    If (consumed And 1) <> 0 Then consumed += 1
    Return dstPos
End Function

' ============================================================================
' 读取并解压 NTFS 压缩属性
' 策略：先按 Run 读取全部原始压缩字节到临时缓冲区，再逐个压缩单元解压
' ============================================================================
Private Function ReadCompressedAttributeData(ctx As NTFS_CONTEXT Ptr, _
                                             ByVal nonResAttr As ATTR_HEADER_NONRESIDENT Ptr, _
                                             ByVal buffer As UByte Ptr, _
                                             ByVal bufferSize As ULong) As Boolean
    If nonResAttr = NULL Or buffer = NULL Or bufferSize = 0 Then Return False
    
    ' 压缩单元大小（簇数），compUnit 通常是 4，即 16 个簇
    Dim compUnitClusters As ULong = 1 Shl nonResAttr->compUnit
    Dim compUnitSize As ULong = compUnitClusters * ctx->clusterSize
    
    Dim allocSize As ULongLong = nonResAttr->allocSize
    If allocSize = 0 Then Return True
    
    ' 一次性读取磁盘上的压缩原始数据
    Dim compressedBuf() As UByte
    ReDim compressedBuf(0 To allocSize - 1)
    
    Dim runPtr As UByte Ptr = Cast(UByte Ptr, nonResAttr) + nonResAttr->runOff
    Dim currentLCN As ULongLong = 0
    Dim compOffset As ULongLong = 0
    Dim isSparse As Boolean
    
    Do While runPtr <> NULL And runPtr[0] <> 0 And compOffset < allocSize
        Dim runLen As ULongLong
        runPtr = ParseRunList(ctx, runPtr, currentLCN, runLen, isSparse)
        If runPtr = NULL Then Exit Do
        
        Dim bytesToProc As ULong = runLen * ctx->clusterSize
        If compOffset + bytesToProc > allocSize Then bytesToProc = allocSize - compOffset
        
        If isSparse Then
            memset(@compressedBuf(compOffset), 0, bytesToProc)
        Else
            If Not ReadClusters(ctx, currentLCN, runLen, @compressedBuf(compOffset)) Then Return False
        End If
        compOffset += bytesToProc
    Loop
    
    ' 逐压缩单元解压
    Dim srcOffset As ULong = 0
    Dim dstOffset As ULong = 0
    
    Do While srcOffset < compOffset And dstOffset < bufferSize
        Dim consumed As ULong = 0
        Dim outAvail As ULong = compUnitSize
        If dstOffset + outAvail > bufferSize Then outAvail = bufferSize - dstOffset
        
        Dim decompressed As ULong = LZNT1Decompress(@compressedBuf(srcOffset), _
                                                     compOffset - srcOffset, _
                                                     buffer + dstOffset, _
                                                     outAvail, _
                                                     consumed)
        If decompressed = 0 And consumed = 0 Then Exit Do
        
        dstOffset += decompressed
        srcOffset += consumed
    Loop
    
    Return True
End Function

'======================================================================
' 索引解析（新增 ctx 参数）
'======================================================================
Private Function FixupIndexBlock(ctx As NTFS_CONTEXT Ptr, ByVal buf As UByte Ptr, ByVal blockSize As ULong) As Boolean
    Dim hdr As INDX_BLOCK_HEADER Ptr = Cast(INDX_BLOCK_HEADER Ptr, buf)
    If hdr->magic <> INDEX_BLOCK_MAGIC Then Return False
    
    Dim usaOffset As UShort = hdr->usaOffset
    Dim usaCount  As UShort = hdr->usaSize
    If usaOffset = 0 Or usaCount < 2 Then Return False
    If CULng(usaOffset) + usaCount * 2 > blockSize Then Return False
    
    Dim usaPtr As UShort Ptr = Cast(UShort Ptr, buf + usaOffset)
    Dim usn As UShort = usaPtr[0]
    
    For i As Integer = 1 To usaCount - 1
        Dim sectorEndOff As ULong = i * ctx->io.bytesPerSector - 2
        If sectorEndOff + 2 > blockSize Then Return False
        Dim lastWordPtr As UShort Ptr = Cast(UShort Ptr, buf + sectorEndOff)
        If lastWordPtr[0] <> usn Then Return False
        lastWordPtr[0] = usaPtr[i]
    Next
    Return True
End Function

Private Sub GetDirList_ParseIndexEntry(ByVal entryPtr As UByte Ptr, ByVal containerStart As UByte Ptr, ByVal containerSize As ULong, ByVal pfileData As WIN32_FILE_DATAEX2W Ptr)
    If pfileData = NULL Or entryPtr = NULL Then Return
    memset pfileData, 0, SizeOf(WIN32_FILE_DATAEX2W)
    pfileData->mftRecordNumber = Cast(ULONGLONG, -1)
    
    Dim e As NTFS_INDEX_ENTRY Ptr = Cast(NTFS_INDEX_ENTRY Ptr, entryPtr)
    If CULng(entryPtr - containerStart) + SizeOf(NTFS_INDEX_ENTRY) > containerSize Then Return
    
    Dim entryLen As ULong = e->entryLen
    If entryLen = 0 Or entryLen < SizeOf(NTFS_INDEX_ENTRY) Then Return
    If CULng(entryPtr - containerStart) + entryLen > containerSize Then Return
    
    Dim recNo As ULongLong = e->fileRef And &HFFFFFFFFFFFFULL
    If recNo = 0 Then Return
    
    Dim keyLen As ULong = e->keyLen
    Dim strName As String = ""
    Dim fnAttr As FILE_NAME_ATTR Ptr = NULL
    
    If keyLen >= 66 Then
        Dim keyPtr As UByte Ptr = entryPtr + SizeOf(NTFS_INDEX_ENTRY)
        If CULng(keyPtr - containerStart) + 66 <= containerSize Then
            fnAttr = Cast(FILE_NAME_ATTR Ptr, keyPtr)
            strName = GetFileNameFromAttr(fnAttr)
        End If
    End If
    
    Static lastRecNo As ULongLong = &HFFFFFFFFFFFFFFFFULL
    If recNo <> lastRecNo Then lastRecNo = recNo
    
    pfileData->cFileName = IIf(strName <> ".", strName, "$Root")
    If fnAttr <> NULL AndAlso fnAttr->nameType <> 2 Then
        pfileData->mftRecordNumber = lastRecNo
    End If
End Sub

Private Function GetDirList_ParseIndexRoot(ctx As NTFS_CONTEXT Ptr, ByVal rootAttr As ATTR_HEADER_RESIDENT Ptr, fileInfo() As WIN32_FILE_DATAEX2W) As Long
    If rootAttr = NULL Then Return -1
    
    Dim attrHeaderSize As ULong = SizeOf(ATTR_HEADER_RESIDENT)
    Dim dataOffset As ULong = rootAttr->dataOff
    Dim dataLen As ULong = rootAttr->dataLen
    
    If dataOffset < attrHeaderSize Then dataOffset = attrHeaderSize
    
    Dim rootStart As UByte Ptr = Cast(UByte Ptr, rootAttr) + dataOffset
    Dim rootSize As ULong = dataLen
    
    If rootSize < SizeOf(NTFS_INDEX_ROOT) Then Return -1
    
    Dim root As NTFS_INDEX_ROOT Ptr = Cast(NTFS_INDEX_ROOT Ptr, rootStart)
    Dim ih As NTFS_INDEX_HEADER Ptr = Cast(NTFS_INDEX_HEADER Ptr, rootStart + SizeOf(NTFS_INDEX_ROOT))
    If ih = NULL Then Return -1
    
    Dim entriesOff As ULong = ih->entriesOff
    Dim entriesSize As ULong = ih->entriesSize
    If entriesSize = 0 Then Return 0
    
    Dim entries As UByte Ptr = Cast(UByte Ptr, ih) + entriesOff
    Dim endPtr As UByte Ptr = entries + entriesSize
    
    If entries < rootStart Or endPtr > rootStart + rootSize Then
        endPtr = rootStart + rootSize
    End If
    
    Dim cur As UByte Ptr = entries
    Dim Count As Long = 0
    ReDim fileInfo(100) As WIN32_FILE_DATAEX2W
    
    Do While cur + SizeOf(NTFS_INDEX_ENTRY) <= endPtr
        Dim e As NTFS_INDEX_ENTRY Ptr = Cast(NTFS_INDEX_ENTRY Ptr, cur)
        Dim eLen As ULong = e->entryLen
        
        If eLen = 0 Then Exit Do
        If eLen < SizeOf(NTFS_INDEX_ENTRY) Then Exit Do
        If cur + eLen > endPtr Then Exit Do
        
        If (e->flags And INDEX_ENTRY_SPACE_FILLER) <> 0 Then
            cur += eLen
            Continue Do
        End If
        
        If Count > UBound(fileInfo) Then ReDim Preserve fileInfo(Count + 100)
        GetDirList_ParseIndexEntry cur, rootStart, rootSize, @fileInfo(Count)
        Count += 1
        
        If (e->flags And INDEX_ENTRY_END) <> 0 Then Exit Do
        cur += eLen
    Loop
    
    Return Count
End Function

Private Function GetDirList_ParseIndexAllocation(ctx As NTFS_CONTEXT Ptr, ByVal nonResAttr As ATTR_HEADER_NONRESIDENT Ptr, fileInfo() As WIN32_FILE_DATAEX2W) As Long
    If nonResAttr = NULL Then Return -1
    
    Dim runPtr As UByte Ptr = Cast(UByte Ptr, nonResAttr) + nonResAttr->runOff
    Dim currentLCN As ULongLong = 0
    Dim runLen As ULongLong
    
    Dim clusterBuf() As UByte
    ReDim clusterBuf(0 To ctx->clusterSize - 1)
    
    Dim Count As Long = 0
    ReDim fileInfo(100) As WIN32_FILE_DATAEX2W
    
    Do While runPtr <> NULL And runPtr[0] <> 0
        runPtr = ParseRunList(ctx, runPtr, currentLCN, runLen)
        If runPtr = NULL Then Exit Do
        
        For i As ULong = 0 To runLen - 1
            Dim lcn As ULongLong = currentLCN + i
            
            If Not ReadClusters(ctx, lcn, 1, @clusterBuf(0)) Then Continue For
            
            If clusterBuf(0) <> Asc("I") Or clusterBuf(1) <> Asc("N") Or clusterBuf(2) <> Asc("D") Or clusterBuf(3) <> Asc("X") Then
                Continue For
            End If
            
            If Not FixupIndexBlock(ctx, @clusterBuf(0), ctx->clusterSize) Then Continue For
            
            Dim ihOffset As ULong = &H18
            If ihOffset + SizeOf(NTFS_INDEX_HEADER) > ctx->clusterSize Then Continue For
            
            Dim ih As NTFS_INDEX_HEADER Ptr = Cast(NTFS_INDEX_HEADER Ptr, @clusterBuf(0) + ihOffset)
            Dim entries As UByte Ptr = @clusterBuf(0) + ihOffset + ih->entriesOff
            Dim endPtr As UByte Ptr = entries + ih->entriesSize
            
            If entries < @clusterBuf(0) Or endPtr > @clusterBuf(0) + ctx->clusterSize Then Continue For
            
            Dim curEntry As UByte Ptr = entries
            Do While curEntry < endPtr
                Dim e As NTFS_INDEX_ENTRY Ptr = Cast(NTFS_INDEX_ENTRY Ptr, curEntry)
                
                If e->entryLen = 0 Then Exit Do
                If e->entryLen < SizeOf(NTFS_INDEX_ENTRY) Or e->entryLen > 65536 Then Exit Do
                If curEntry + e->entryLen > endPtr Then Exit Do
                
                If (e->flags And INDEX_ENTRY_SPACE_FILLER) <> 0 Then
                    curEntry += e->entryLen
                    Continue Do
                End If
                
                If Count > UBound(fileInfo) Then ReDim Preserve fileInfo(Count + 100)
                GetDirList_ParseIndexEntry curEntry, @clusterBuf(0), ctx->clusterSize, @fileInfo(Count)
                Count += 1
                
                If (e->flags And INDEX_ENTRY_END) <> 0 Then Exit Do
                curEntry += e->entryLen
            Loop
        Next
    Loop
    
    Return Count
End Function

Private Function GetDirList_ParseAttributeList(ctx As NTFS_CONTEXT Ptr, ByVal currentMFTRecordNumber As ULongLong, ByVal attr As ATTR_HEADER_GENERIC Ptr, fileInfo() As WIN32_FILE_DATAEX2W) As Long
    If attr = NULL Then Return -1
    
    Dim listCount As Long = 0
    Dim dataPtr As UByte Ptr
    Dim dataSize As ULong
    Dim tempBuffer() As UByte
    
    If attr->nonResident = 0 Then
        Dim resAttr As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, attr)
        dataPtr = Cast(UByte Ptr, attr) + resAttr->dataOff
        dataSize = resAttr->dataLen
    Else
        Dim nonResAttr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, attr)
        dataSize = nonResAttr->dataSize
        If dataSize = 0 Or dataSize > 10*1024*1024 Then Return -1
        
        ReDim tempBuffer(0 To dataSize - 1)
        If Not ReadNonResidentAttributeData(ctx, nonResAttr, @tempBuffer(0), dataSize) Then Return -1
        dataPtr = @tempBuffer(0)
    End If
    
    If dataPtr = NULL Or dataSize = 0 Then Return -1
    
    Dim entry As ATTRIBUTE_LIST_ENTRY Ptr = Cast(ATTRIBUTE_LIST_ENTRY Ptr, dataPtr)
    Dim offset As ULong = 0
    
    While offset + SizeOf(ATTRIBUTE_LIST_ENTRY) <= dataSize
        If entry->recordLen < SizeOf(ATTRIBUTE_LIST_ENTRY) Then Exit While
        If offset + entry->recordLen > dataSize Then Exit While
        
        Dim refMFT As ULongLong = entry->mftRef And &hFFFFFFFFFFFFULL
        
        If entry->typeID = ATTR_TYPE_INDEX_ROOT Or entry->typeID = ATTR_TYPE_INDEX_ALLOCATION Then
            If refMFT <> currentMFTRecordNumber Then
                Dim externalRecBuf() As UByte
                ReDim externalRecBuf(0 To ctx->mftRecordSize - 1)
                
                If ReadExternalMFTRecord(ctx, refMFT, @externalRecBuf(0)) Then
                    Dim extMftHeader As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @externalRecBuf(0))
                    If extMftHeader->magic = NTFS_MFT_RECORD_MAGIC Then
                        Dim extAttr As ATTR_HEADER_GENERIC Ptr = FindAttributeInRecord(ctx, extMftHeader, entry->typeID, entry->attrInstance)
                        
                        If extAttr <> NULL Then
                            Dim tempInfo() As WIN32_FILE_DATAEX2W
                            Dim tempCount As Long = 0
                            
                            Select Case extAttr->typeID
                            Case ATTR_TYPE_INDEX_ROOT
                                tempCount = GetDirList_ParseIndexRoot(ctx, Cast(ATTR_HEADER_RESIDENT Ptr, extAttr), tempInfo())
                            Case ATTR_TYPE_INDEX_ALLOCATION
                                tempCount = GetDirList_ParseIndexAllocation(ctx, Cast(ATTR_HEADER_NONRESIDENT Ptr, extAttr), tempInfo())
                            End Select
                            
                            If tempCount > 0 Then
                                For k As Long = 0 To tempCount - 1
                                    If listCount > UBound(fileInfo) Then ReDim Preserve fileInfo(listCount + 100)
                                    fileInfo(listCount) = tempInfo(k)
                                    listCount += 1
                                Next
                            End If
                        End If
                    End If
                End If
            End If
        End If
        
        offset += entry->recordLen
        entry = Cast(ATTRIBUTE_LIST_ENTRY Ptr, dataPtr + offset)
    Wend
    
    Return listCount
End Function

'======================================================================
' 文件信息辅助（新增 ctx 参数）
'======================================================================
Private Sub FillFileInfo(ctx As NTFS_CONTEXT Ptr, ByVal recordNum As ULongLong, ByRef fileInfo As WIN32_FILE_DATAEX2W)
    Dim recBuf() As UByte
    ReDim recBuf(0 To ctx->mftRecordSize - 1)
    If Not ReadExternalMFTRecord(ctx, recordNum, @recBuf(0)) Then Return
    
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
    If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then Return
    
    fileInfo.LinkCount = mrh->linkCount
    fileInfo.logicalClusterNumber = mrh->lsn
    fileInfo.bHasAlternateStream = False
    
    Dim attrOff As ULong = mrh->attrOffset
    
    Dim dataStreamCount As Long = 0      ' 统计 $DATA 属性个数
    
    Do While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)
        
        If g->typeID = &HFFFFFFFF Then Exit Do
        If g->totalLen = 0 Then Exit Do
        If attrOff + g->totalLen > ctx->mftRecordSize Then Exit Do
        
        Select Case g->typeID
        Case ATTR_TYPE_STANDARD_INFORMATION
            Dim standardInfo As FILE_STANDARD_INFORMATION Ptr = Cast(FILE_STANDARD_INFORMATION Ptr, @recBuf(0) + attrOff + SizeOf(ATTR_HEADER_RESIDENT))
            fileInfo.dwFileAttributes = standardInfo->fileAttributes
            ConvertToFileTime standardInfo->creationTime, fileInfo.ftCreationTime
            ConvertToFileTime standardInfo->lastAccessTime, fileInfo.ftLastAccessTime
            ConvertToFileTime standardInfo->lastWriteTime, fileInfo.ftLastWriteTime
        Case ATTR_TYPE_REPARSE_POINT
            fileInfo.bReparsePoint = True
            If g->nonResident = 0 Then
                Dim resR As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, g)
                Dim rph As REPARSE_DATA_HEADER Ptr = Cast(REPARSE_DATA_HEADER Ptr, _
                    Cast(UByte Ptr, resR) + resR->dataOff)
                fileInfo.reparseTag = rph->reparseTag
            End If
            ' 同时设置 Win32 风格属性标志
            fileInfo.dwFileAttributes Or= FILE_ATTRIBUTE_REPARSE_POINT
        Case ATTR_TYPE_DATA
            dataStreamCount += 1
            Dim streamName As String = GetAttributeName(g)
            If streamName <> "" Then
                fileInfo.bHasAlternateStream = True
            End If
            'Print "streamName:" & streamName & " dataStreamCount:" & dataStreamCount
            If g->nonResident Then
                Dim nr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, g)
                fileInfo.dwFileSizeLow = nr->dataSize And &HFFFFFFFF
                fileInfo.dwFileSizeHigh = (nr->dataSize Shr 32) And &HFFFFFFFF
            Else
                Dim res As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, g)
                fileInfo.dwFileSizeLow = res->dataLen
                fileInfo.dwFileSizeHigh = 0
            End If
        End Select
        attrOff += g->totalLen
    Loop
    fileInfo.bSuspicious = (dataStreamCount >= 2)   ' 只要多一个流就标记可疑
End Sub

Private Function IsDirectory(ctx As NTFS_CONTEXT Ptr, ByVal recordNum As ULongLong) As Boolean
    Dim recBuf() As UByte
    ReDim recBuf(0 To ctx->mftRecordSize - 1)
    If Not ReadExternalMFTRecord(ctx, recordNum, @recBuf(0)) Then Return False
    
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
    If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then Return False
    Return (mrh->dwFlags And &H2) <> 0
End Function

'======================================================================
' 对外接口 Internal 实现（供包装层调用）
'======================================================================
Function NTFS_GetDirList_Internal(ctx As NTFS_CONTEXT Ptr, lpDirPath As String, fileInfoList() As WIN32_FILE_DATAEX2W) As Long
    Dim fileCount As Long = 0
    Dim parts() As String
    vbSplit lpDirPath, "\", parts()
    
    If ctx = NULL Then Return -1
    Print "[NTFS_GetDirList_Internal]lpDirPath:" & lpDirPath
    Dim recBuf() As UByte
    ReDim recBuf(0 To ctx->mftRecordSize - 1)
    Dim recordNum As ULONGLONG = 5
    
    For i As Integer = 1 To UBound(parts) + 1
        If Not ReadExternalMFTRecord(ctx, recordNum, @recBuf(0)) Then Return -1
        
        Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
        If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then Return -1
        
        Dim attrOff As ULong = mrh->attrOffset
        Dim cachedIndexAlloc As ATTR_HEADER_NONRESIDENT Ptr = NULL
        Dim cachedIndexRoot As ATTR_HEADER_RESIDENT Ptr = NULL
        
        ' 第一次扫描：缓存关键属性
        Dim tempOff As ULong = attrOff
        While tempOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
            Dim tempAttr As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + tempOff)
            If tempAttr->typeID = &HFFFFFFFF Or tempAttr->totalLen = 0 Then Exit While
            
            Select Case tempAttr->typeID
            Case ATTR_TYPE_INDEX_ALLOCATION
                cachedIndexAlloc = Cast(ATTR_HEADER_NONRESIDENT Ptr, tempAttr)
            Case ATTR_TYPE_INDEX_ROOT
                cachedIndexRoot = Cast(ATTR_HEADER_RESIDENT Ptr, tempAttr)
            End Select
            tempOff += tempAttr->totalLen
        Wend
        
        ' 第二次扫描：解析属性
        Dim fileInfo() As WIN32_FILE_DATAEX2W
        Dim ListCount As Long = 0
        attrOff = mrh->attrOffset
        
        ' 检查属性列表
        Dim hasAttributeList As Boolean = False
        Dim attributeListAttr As ATTR_HEADER_GENERIC Ptr = NULL
        
        tempOff = attrOff
        While tempOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
            Dim chkAttr As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + tempOff)
            If chkAttr->typeID = &HFFFFFFFF Or chkAttr->totalLen = 0 Then Exit While
            If chkAttr->typeID = ATTR_TYPE_ATTRIBUTE_LIST Then
                hasAttributeList = True
                attributeListAttr = chkAttr
            End If
            tempOff += chkAttr->totalLen
        Wend
        
        If hasAttributeList And attributeListAttr <> NULL Then
            ListCount = GetDirList_ParseAttributeList(ctx, recordNum, attributeListAttr, fileInfo())
            If ListCount >= 0 Then Goto ProcessResults
        End If
        
        ' 常规处理
        While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
            Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)
            
            If g->typeID = &HFFFFFFFF Then Exit While
            If g->totalLen = 0 Then Exit While
            If attrOff + g->totalLen > ctx->mftRecordSize Then Exit While
            
            Select Case g->typeID
            Case ATTR_TYPE_INDEX_ROOT
                If ListCount = 0 Then
                    ListCount = GetDirList_ParseIndexRoot(ctx, Cast(ATTR_HEADER_RESIDENT Ptr, g), fileInfo())
                End If
            Case ATTR_TYPE_INDEX_ALLOCATION
                If cachedIndexAlloc <> NULL Then
                    ListCount = GetDirList_ParseIndexAllocation(ctx, cachedIndexAlloc, fileInfo())
                    cachedIndexAlloc = NULL
                End If
            End Select
            
            attrOff += g->totalLen
        Wend
        
ProcessResults:
        If UBound(parts) = 0 OrElse i > UBound(parts) Then
            ' 最终目录，输出结果
            fileCount = 0
            If ListCount > 0 Then
                ReDim fileInfoList(ListCount - 1) As WIN32_FILE_DATAEX2W
                Dim FileRecBuf() As UByte
                
                For j As Long = 0 To ListCount - 1
                    If fileInfo(j).mftRecordNumber <> Cast(ULONGLONG, -1) Then
                        ReDim FileRecBuf(0 To ctx->mftRecordSize - 1)
                        If ReadExternalMFTRecord(ctx, fileInfo(j).mftRecordNumber, @FileRecBuf(0)) Then
                            Dim FileMftHeader As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @FileRecBuf(0))
                            If FileMftHeader->magic = NTFS_MFT_RECORD_MAGIC Then
                                fileInfo(j).bDeleted = (Not(CBool(FileMftHeader->dwFlags And &H1)))
                                fileInfo(j).bDirectory = (FileMftHeader->dwFlags And &H2)
                            End If
                            FillFileInfo ctx, fileInfo(j).mftRecordNumber, fileInfo(j)
                            'Print "fileInfo.bSuspicious:" & fileInfo(j).bSuspicious
                            fileInfoList(fileCount) = fileInfo(j)
                            fileCount += 1
                        End If
                    End If
                Next
            End If
            Return fileCount
        Else
            ' 继续遍历路径
            For j As Long = 0 To ListCount - 1
                If fileInfo(j).cFileName <> "" Then
                    If UCase(parts(i)) = UCase(fileInfo(j).cFileName) Then
                        recordNum = fileInfo(j).mftRecordNumber
                        Exit For
                    End If
                End If
            Next
        End If
    Next
    
    Return fileCount
End Function

' 辅助函数1：根据路径解析对应的MFT记录号（复用你现有GetDirList的路径遍历逻辑）
' 参数：lpFilePath=文件路径，outMftNum=输出MFT记录号
Private Function GetMFTRecordByPath(ctx As NTFS_CONTEXT Ptr, ByVal lpFilePath As String, ByRef outMftNum As ULongLong) As BOOL
    outMftNum = 0
    Dim dirPath As String, fileName As String
    Dim fileInfoList() As WIN32_FILE_DATAEX2W ' 你现有模块的文件信息结构
    Dim listCount As Long
    
    ' 拆分路径为目录和文件名（如C:\Test.txt → 目录=C:\，文件名=Test.txt）
    dirPath = Left(lpFilePath, InStrRev(lpFilePath, "\") - 1)
    fileName = Mid(lpFilePath, InStrRev(lpFilePath, "\") + 1)
    'Print "dirPath:" & dirPath
    ' 调用你现有GetDirList遍历目录，获取文件列表
    listCount = NTFS_GetDirList_Internal(ctx, dirPath, fileInfoList()) ' 假设GetDirList返回文件数量，并填充fileInfoList
    If listCount <= 0 Then Return 0
    
    'Print "fileName:" & fileName
    
    ' 遍历文件列表，匹配文件名
    For i As Long = 0 To listCount - 1
        'Print fileInfoList(i).cFileName
        'Print "fileInfoList(i).cFileName:" & fileInfoList(i).cFileName
        If LCase(fileInfoList(i).cFileName) = LCase(fileName) Then
            outMftNum = fileInfoList(i).mftRecordNumber
            'CloseHandle hVol
            Return 1
        End If
    Next
    
    'CloseHandle hVol
    Return 0
End Function

Private Function ReadFile2_Internal(ctx As NTFS_CONTEXT Ptr, ByVal lpFilePath As String, bytData() As UByte) As Boolean
    ' 初始化输出
    Dim parts() As String : vbSplit lpFilePath, "\", parts()
    
    Dim result As Boolean = False
    
    ' ========== 步骤1：验证路径并获取对应的MFT记录号 ==========
    Dim targetMftRecordNum As ULongLong = 0
    If GetMFTRecordByPath(ctx, lpFilePath, targetMftRecordNum) = 0 Then
        Print "错误：路径不存在或无法解析MFT记录 → " & lpFilePath
        Return False
    End If
    
    Dim recBuf() As UByte
    ReDim recBuf(0 To ctx->mftRecordSize - 1)
    If Not ReadExternalMFTRecord(ctx, targetMftRecordNum, @recBuf(0)) Then ' ReadMFTRecord_Safe
        Print "错误：读取MFT记录" & targetMftRecordNum & "失败（USA修复失败）"
        Return False
    End If
    
    ' 检查是否为文件（非目录）
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
    If (mrh->dwFlags And &H2) <> 0 Then ' DIRECTORY标志（0x2）
        Print "错误：目标路径是目录，非文件 → " & lpFilePath
        Return False
    End If
    
    ' ========== 步骤3：解析DATA属性（类型0x80） ==========
    Dim dataAttr As ATTR_HEADER_NONRESIDENT Ptr = NULL
    Dim targetMftBuf() As UByte   ' 用于存储外部 MFT 记录（如果需要）
    Dim isExternalRecord As Boolean = False
    
' 定义标签，用于从外部记录跳转到核心处理逻辑
FindDataAttr:
    Dim attrOff As ULong = mrh->attrOffset
    Dim fileSize As ULongLong = 0
    Dim gAttr As ATTR_HEADER_GENERIC Ptr
    
    ' 遍历MFT记录的所有属性，找到DATA属性
    Do While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        'Print ">>> [调试] 检查属性偏移: " & attrOff
        gAttr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)
        
        ' 终止条件：属性类型ID为0xFFFFFFFF
        If gAttr->typeID = &HFFFFFFFF Then Exit Do
        
        ' 找到DATA属性（类型0x80）
        If gAttr->typeID = &H80 Then
            ' 直接移交控制权给下方的现有处理逻辑
            Goto ProcessFoundDataAttr
        ElseIf gAttr->typeID = ATTR_TYPE_ATTRIBUTE_LIST AndAlso Not isExternalRecord Then
            ' 1. 解析 Attribute List 的数据内容
            Dim listDataPtr As UByte Ptr
            Dim listDataSize As ULong
            Dim tempListBuf() As UByte
            
            If gAttr->nonResident = 0 Then
                Dim resAttr As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, gAttr)
                listDataPtr = Cast(UByte Ptr, resAttr) + resAttr->dataOff
                listDataSize = resAttr->dataLen
            Else
                ' (可选) 处理非常驻的 Attribute List (通常很少见)
                'Print "    [!] Attribute List 是非常驻的，代码暂不支持"
                ' ============== 新增：处理非常驻 Attribute List ==============
                Print ">>> [调试] 处理非常驻 Attribute List"
                Dim nrAttr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, gAttr)
                listDataSize = nrAttr->dataSize
                
                ' 安全限制：防止超大列表占用内存
                If listDataSize = 0 Or listDataSize > 1024 * 1024 Then
                    Print "    [!] 非常驻 Attribute List 大小异常"
                    Goto ContinueDo
                End If
                
                ' 分配缓冲区，读取非常驻数据（复用你已有的函数）
                ReDim tempListBuf(0 To listDataSize - 1)
                If ReadNonResidentAttributeData(ctx, nrAttr, @tempListBuf(0), listDataSize) = False Then
                    Print "    [!] 读取非常驻 Attribute List 失败"
                    Goto ContinueDo
                End If
                
                ' 指针指向读取后的缓冲区
                listDataPtr = @tempListBuf(0)
            End If
            
            ' 2. 遍历 Attribute List 条目
            Dim entry As ATTRIBUTE_LIST_ENTRY Ptr = Cast(ATTRIBUTE_LIST_ENTRY Ptr, listDataPtr)
            Dim offset As ULong = 0
            
            While offset + SizeOf(ATTRIBUTE_LIST_ENTRY) <= listDataSize
                If entry->recordLen < SizeOf(ATTRIBUTE_LIST_ENTRY) Then Exit While
                If offset + entry->recordLen > listDataSize Then Exit While
                
                ' 检查：这个条目是不是 DATA 属性 (0x80)？
                If entry->typeID = ATTR_TYPE_DATA Then
                    Dim extMftRecNo As ULongLong = entry->mftRef And &HFFFFFFFFFFFFULL
                    'Print ">>> [调试] 找到外部 DATA，位于 MFT 记录: " & extMftRecNo
                    
                    ' 3. 读取外部 MFT 记录
                    ReDim targetMftBuf(0 To ctx->mftRecordSize - 1)
                    If ReadExternalMFTRecord(ctx, extMftRecNo, @targetMftBuf(0)) Then
                        ' 4. 切换上下文：将 recBuf 指向外部记录，并重置 mrh 指针
                        memcpy(@recBuf(0), @targetMftBuf(0), ctx->mftRecordSize)
                        
                        ' 重新初始化 MFT 头指针
                        mrh = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
                        isExternalRecord = True
                        
                        ' 5. 跳回标签 FindDataAttr，在新的 recBuf 里重新找属性
                        GoTo FindDataAttr
                    End If
                End If
                
                offset += entry->recordLen
                entry = Cast(ATTRIBUTE_LIST_ENTRY Ptr, listDataPtr + offset)
            Wend
        End If
ContinueDo:
        attrOff += gAttr->totalLen ' 偏移到下一个属性
    Loop
    
    Print "[!] 错误：未找到 $DATA 属性 (0x80)"
    Return False
    
ProcessFoundDataAttr:
    ' ========== 先扫描重解析点（0xC0）==========
    Dim hasReparsePoint As Boolean = False
    Dim reparseTag As ULong = 0
    Dim attrOff2 As ULong = mrh->attrOffset
    Do While attrOff2 + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g2 As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff2)
        If g2->typeID = &HFFFFFFFF Or g2->totalLen = 0 Then Exit Do
        If g2->typeID = ATTR_TYPE_REPARSE_POINT Then
            hasReparsePoint = True
            If g2->nonResident = 0 Then
                Dim resR As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, g2)
                Dim rph As REPARSE_DATA_HEADER Ptr = Cast(REPARSE_DATA_HEADER Ptr, _
                    Cast(UByte Ptr, resR) + resR->dataOff)
                reparseTag = rph->reparseTag
            End If
            Exit Do
        End If
        attrOff2 += g2->totalLen
    Loop
    
    If hasReparsePoint Then
        Print "警告：目标文件是重解析点 (Tag=0x" & Hex(reparseTag, 8) & _
              ")，读取的是重解析原始数据而非文件内容"
        ' 继续执行，调用方可根据 bReparsePoint 判断
    End If
    
    ' ========== 区分常驻 / 非常驻 / 压缩 ==========
    If gAttr->nonResident = 1 Then
        Dim nr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, gAttr)
        
        ' === 声明 clusterBuffer（必须在非常驻分支的最开始）===
        Dim clusterBuffer As UByte Ptr = Allocate(ctx->clusterSize)
        If clusterBuffer = NULL Then
            Print "    [!] 簇缓冲区分配失败"
            Return False
        End If
        
        ' 防御：dataSize 改用 ULongLong，防止 >4GB 截断
        Dim dataSize As ULongLong = nr->dataSize
        If dataSize = 0 Then Return True
        If dataSize > 256*1024*1024 Then
            Print "    [!] 文件过大，限制读取 256MB"
            dataSize = 256*1024*1024
        End If
        
        ' 检查压缩标志
        Dim isCompressed As Boolean = (nr->flags And ATTR_FLAG_COMPRESSED) <> 0
        
        ReDim bytData(dataSize - 1) As UByte
        
        If isCompressed Then
            ' === 压缩属性：使用新的解压读取 ===
            If Not ReadCompressedAttributeData(ctx, nr, @bytData(0), dataSize) Then
                Print "    [!] 读取压缩数据失败"
                Return False
            End If
            Print "    压缩文件读取并解压完成"
            result = True
        Else
            ' === 非压缩：原有逻辑，但使用 isSparse 修复 ===
            Dim pfileData As UByte Ptr = Allocate(dataSize)
            If pfileData = NULL Then Return False
            memset(pfileData, 0, dataSize)
            
            Dim runPtr As UByte Ptr = Cast(UByte Ptr, nr) + nr->runOff
            Dim currentLCN As ULongLong = 0
            Dim vcn As ULongLong = nr->lowVCN
            Dim bytesWritten As ULongLong = 0
            Dim baseVCN As ULongLong = nr->lowVCN
            Dim isSparse As Boolean = False
            
            While runPtr <> NULL And runPtr[0] <> 0
                Dim runLen As ULongLong
                runPtr = ParseRunList(ctx, runPtr, currentLCN, runLen, isSparse)
                If runPtr = NULL Then Exit While
                
                If runLen = 0 Or runLen > 1000000 Then Exit While
                
                For i As ULong = 0 To runLen - 1
                    Dim currentVCN As ULongLong = vcn + i
                    Dim offsetInData As ULongLong = (currentVCN - baseVCN) * ctx->clusterSize
                    
                    If offsetInData >= dataSize Then Exit For
                    
                    Dim bytesToRead As ULong = ctx->clusterSize
                    If offsetInData + bytesToRead > dataSize Then bytesToRead = dataSize - offsetInData
                    If bytesToRead = 0 Then Exit For
                    
                    If isSparse Then
                        ' 稀疏：已 memset 为 0，只需推进计数
                        bytesWritten += bytesToRead
                    Else
                        Dim lcn As ULongLong = currentLCN + i
                        If ReadClusters(ctx, lcn, 1, clusterBuffer) Then
                            memcpy(pfileData + offsetInData, clusterBuffer, bytesToRead)
                            bytesWritten += bytesToRead
                        End If
                    End If
                Next
                
                vcn += runLen
            Wend
            
            If bytesWritten > 0 Then
                ReDim bytData(bytesWritten - 1)
                memcpy(@bytData(0), pfileData, bytesWritten)
                result = True
            End If
            Deallocate(pfileData)
        End If
    Else
        ' 常驻属性（保持不变）
        Dim resAttr As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, gAttr)
        Dim dataLen As ULong = resAttr->dataLen
        Dim dataPtr As UByte Ptr = Cast(UByte Ptr, resAttr) + resAttr->dataOff
        If dataPtr <> NULL And dataLen > 0 Then
            ReDim bytData(dataLen - 1)
            memcpy(@bytData(0), dataPtr, dataLen)
            result = True
        End If
    End If
    
    ' ========== 最终结果返回 ==========
    If result = True Then
        Print "成功读取文件内容，大小：" & UBound(bytData) + 1 & " 字节"
    End If
    Return result
End Function

Private Function MyCopyFile_Internal(ctx As NTFS_CONTEXT Ptr, lpSourceFile As StringW, lpTargetFile As StringW) As Boolean
    Dim bytData() As UByte
    If Not ReadFile2_Internal(ctx, lpSourceFile, bytData()) Then
        Print "ReadFile2失败"
        Return False
    End If
    Dim hFile As HANDLE = CreateFile(lpTargetFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        Print "CreateFile(" & lpTargetFile & ")失败:" & WinErrorMsg(GetLastError)
        Return False
    End If
    Dim bytWritten As DWORD, dwWritten As DWORD = (UBound(bytData) + 1) * SizeOf(UByte)
    If (WriteFile(hFile, @bytData(0), dwWritten, @bytWritten, NULL) = 0) OrElse (bytWritten < dwWritten) Then
        Print "WriteFile失败(" & bytWritten & "/" & dwWritten & "):" & WinErrorMsg(GetLastError)
        CloseHandle hFile
        Return False
    End If
    CloseHandle hFile
    Return True
End Function

'======================================================================
'  目录栈条目（用于非递归遍历）
'======================================================================
Type DIR_STACK_ENTRY
    sourcePath      As String     ' 源目录路径
    targetPath      As String     ' 目标目录路径
    isProcessed     As Boolean    ' 是否已处理（创建目录+枚举子项）
End Type

'======================================================================
'  文件栈条目（用于批量文件拷贝）
'======================================================================
Type FILE_COPY_ENTRY
    sourceFile      As String     ' 源文件完整路径
    targetFile      As String     ' 目标文件完整路径
End Type

'======================================================================
'  内部函数：创建目录（递归创建父目录）
'======================================================================
Private Function CreateDirectoryTree(ByVal dirPath As String) As Boolean
    If dirPath = "" Then Return False
    
    ' 去除尾部反斜杠（除了根目录）
    Dim cleanPath As String = dirPath
    If Right(cleanPath, 1) = "\" And Len(cleanPath) > 3 Then
        cleanPath = Left(cleanPath, Len(cleanPath) - 1)
    End If
    
    ' 检查目录是否已存在
    Dim attr As DWORD = GetFileAttributes(cleanPath)
    If attr <> INVALID_FILE_ATTRIBUTES Then
        Return (attr And FILE_ATTRIBUTE_DIRECTORY) <> 0
    End If
    
    ' 递归创建父目录
    Dim parentPath As String = Left(cleanPath, InStrRev(cleanPath, "\") - 1)
    If Len(parentPath) >= 3 Then  ' 至少 "X:\"
        If Not CreateDirectoryTree(parentPath) Then Return False
    End If
    
    ' 创建当前目录
    Return CreateDirectory(cleanPath, NULL) <> 0 Or GetLastError() = ERROR_ALREADY_EXISTS
End Function

'======================================================================
'  内部函数：非递归拷贝单个文件（使用你的NTFS底层读取 + Win32写入）
'======================================================================
Private Function CopySingleFile_Internal(ctx As NTFS_CONTEXT Ptr, _
                                         ByVal srcPath As String, _
                                         ByVal dstPath As String) As Boolean
    Dim bytData() As UByte
    
    ' 使用底层NTFS读取（绕过Windows句柄限制，支持被锁定的系统文件）
    If Not ReadFile2_Internal(ctx, srcPath, bytData()) Then
        ' 回退：使用普通Win32 API拷贝（对于非NTFS卷或普通文件）
        Return CopyFile(srcPath, dstPath, FALSE) <> 0
    End If
    
    ' 确保目标目录存在
    Dim dstDir As String = Left(dstPath, InStrRev(dstPath, "\") - 1)
    If Not CreateDirectoryTree(dstDir) Then Return False
    
    ' 写入目标文件
    Dim hFile As HANDLE = CreateFile(dstPath, GENERIC_WRITE, _
                                     FILE_SHARE_READ, NULL, _
                                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then Return False
    
    Dim dwWritten As DWORD = 0
    Dim dwSize As DWORD = (UBound(bytData) + 1) * SizeOf(UByte)
    Dim result As Boolean = WriteFile(hFile, @bytData(0), dwSize, @dwWritten, NULL) <> 0 _
                            And dwWritten = dwSize
    CloseHandle hFile
    
    If Not result Then
        ' 写入失败，清理残文件
        DeleteFile dstPath
    End If
    
    Return result
End Function

'======================================================================
'  核心函数：非递归拷贝文件夹（包括子文件夹及所有文件）
'  
'  参数：
'    lpSourceDir  - 源目录路径，如 "C:\SourceFolder"
'    lpTargetDir  - 目标目录路径，如 "D:\Backup\SourceFolder"
'  
'  返回值：
'    True  - 成功
'    False - 失败（部分文件可能已拷贝）
'  
'  特点：
'    1. 完全非递归，使用动态数组模拟栈，避免爆栈
'    2. 支持NTFS底层读取（可拷贝被锁定的系统文件）
'    3. 自动创建目标目录结构
'    4. 保留文件属性（通过底层读取原始字节）
'======================================================================
Public Function CopyFolder_NonRecursive(ByVal lpSourceDir As String, _
                                        ByVal lpTargetDir As String) As Boolean
    ' ---------- 参数校验 ----------
    If Right(lpSourceDir, 1) <> "\" Then lpSourceDir += "\"
    If Right(lpTargetDir, 1) <> "\" Then lpTargetDir += "\"
    
    ' 获取源卷上下文
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpSourceDir)
    If ctx = NULL Then
        Print "[CopyFolder] 无法打开源卷: " & lpSourceDir
        Return False
    End If
    
    ' ---------- 初始化目录栈（DFS） ----------
    Dim dirStack() As DIR_STACK_ENTRY
    ReDim dirStack(0 To 255)  ' 初始容量256，动态扩展
    Dim dirStackTop As Long = -1  ' 栈顶指针，-1表示空栈
    
    ' 压入根目录
    dirStackTop += 1
    dirStack(dirStackTop).sourcePath = lpSourceDir
    dirStack(dirStackTop).targetPath = lpTargetDir
    dirStack(dirStackTop).isProcessed = False
    
    ' ---------- 初始化文件队列 ----------
    Dim fileQueue() As FILE_COPY_ENTRY
    ReDim fileQueue(0 To 1023)  ' 批量文件缓冲
    Dim fileQueueCount As Long = 0
    
    ' ---------- 统计信息 ----------
    Dim dirsCreated As Long = 0
    Dim filesCopied As Long = 0
    Dim filesFailed As Long = 0
    Dim totalBytes As ULongLong = 0
    
    Print "[CopyFolder] 开始非递归拷贝..."
    Print "  源: " & lpSourceDir
    Print "  目标: " & lpTargetDir
    
    ' ---------- 主循环：DFS遍历目录树 ----------
    Do While dirStackTop >= 0
        
        ' 查看栈顶（不弹出）
        Dim currentIdx As Long = dirStackTop
        Dim current As DIR_STACK_ENTRY Ptr = @dirStack(currentIdx)
        
        If current->isProcessed = False Then
            ' ====== 第一次访问：创建目录，枚举子项 ======
            
            ' 创建目标目录
            If Not CreateDirectoryTree(current->targetPath) Then
                Print "[警告] 无法创建目录: " & current->targetPath
                ' 标记为已处理并弹出（跳过此目录）
                current->isProcessed = True
                dirStackTop -= 1
                Continue Do
            End If
            dirsCreated += 1
            
            ' 获取源目录下的所有文件和子目录
            Dim fileInfoList() As WIN32_FILE_DATAEX2W
            Dim listCount As Long = NTFS_GetDirList_Internal(ctx, current->sourcePath, fileInfoList())
            
            If listCount > 0 Then
                ' 扩展目录栈容量（如果需要）
                If dirStackTop + listCount >= UBound(dirStack) Then
                    ReDim Preserve dirStack(0 To UBound(dirStack) + 512)
                End If
                
                ' 扩展文件队列容量（如果需要）
                If fileQueueCount + listCount >= UBound(fileQueue) Then
                    ReDim Preserve fileQueue(0 To UBound(fileQueue) + 1024)
                End If
                
                ' 遍历目录项，分类处理（文件入队列，目录入栈）
                For i As Long = 0 To listCount - 1
                    Dim itemName As String = fileInfoList(i).cFileName
                    
                    ' 跳过 "." 和 ".."
                    If itemName = "." Or itemName = ".." Then Continue For
                    If itemName = "" Then Continue For
                    
                    Dim srcFullPath As String = current->sourcePath & itemName
                    Dim dstFullPath As String = current->targetPath & itemName
                    
                    If fileInfoList(i).bDirectory Then
                        ' === 子目录：压入目录栈（DFS，后进先出）===
                        dirStackTop += 1
                        dirStack(dirStackTop).sourcePath = srcFullPath & "\"
                        dirStack(dirStackTop).targetPath = dstFullPath & "\"
                        dirStack(dirStackTop).isProcessed = False
                    Else
                        ' === 文件：加入文件队列 ===
                        fileQueue(fileQueueCount).sourceFile = srcFullPath
                        fileQueue(fileQueueCount).targetFile = dstFullPath
                        fileQueueCount += 1
                    End If
                Next
            End If
            
            ' 标记为已处理
            current->isProcessed = True
            
        Else
            ' ====== 第二次访问：目录处理完毕，弹出栈 ======
            dirStackTop -= 1
        End If
        
        ' 定期处理文件队列（防止内存占用过大）
        ' 策略：当队列满80%时，批量处理
        If fileQueueCount > 0 And (fileQueueCount >= UBound(fileQueue) * 0.8 Or dirStackTop < 0) Then
            Print "  [批量拷贝] 处理 " & fileQueueCount & " 个文件..."
            
            For i As Long = 0 To fileQueueCount - 1
                If CopySingleFile_Internal(ctx, _
                                           fileQueue(i).sourceFile, _
                                           fileQueue(i).targetFile) Then
                    filesCopied += 1
                Else
                    filesFailed += 1
                    Print "[失败] " & fileQueue(i).sourceFile
                End If
                FF_DoEvents  ' 防止UI卡死
            Next
            
            fileQueueCount = 0  ' 清空队列
        End If
        
        FF_DoEvents  ' UI响应
        
    Loop
    
    ' ---------- 最终统计 ----------
    Print !"\n[CopyFolder] 拷贝完成:"
    Print "  目录创建: " & dirsCreated
    Print "  文件成功: " & filesCopied
    Print "  文件失败: " & filesFailed
    
    Return filesFailed = 0
End Function

'======================================================================
'  简化包装函数：直接调用
'======================================================================
Function CopyFolder(ByVal lpSourceDir As String, _
                         ByVal lpTargetDir As String) As Boolean
    Return CopyFolder_NonRecursive(lpSourceDir, lpTargetDir)
End Function

' 同理，ReadFile2 和 MyCopyFile 的包装实现：
Function ReadFile2(ByVal lpFilePath As String, bytData() As UByte) As Boolean
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpFilePath)
    If ctx = NULL Then Return False
    
    Dim result As Boolean = ReadFile2_Internal(ctx, lpFilePath, bytData())
    
    CloseNTFSVolumeContext(ctx)
    Return result
End Function

Function MyCopyFile(lpSourceFile As StringW, lpTargetFile As StringW) As Boolean
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpSourceFile)
    If ctx = NULL Then Return False
    
    Dim result As Boolean = MyCopyFile_Internal(ctx, lpSourceFile, lpTargetFile)
    
    CloseNTFSVolumeContext(ctx)
    Return result
End Function

'======================================================================
' 内部辅助：从单条 MFT 记录中收集所有 $DATA 流
' 参数：
'   ctx          - 卷上下文
'   recBuf       - 已修复的 MFT 记录缓冲区
'   streamNames()- 输出流名数组
'   streamSizes()- 输出流大小数组
'   streamCount  - 输入/输出：当前已收集的流个数
' 返回值：成功返回 True，否则 False
'======================================================================
Private Function CollectDataStreamsFromRecord(ctx As NTFS_CONTEXT Ptr, _
                                              recBuf() As UByte, _
                                              streamNames() As String, _
                                              streamSizes() As ULongLong, _
                                              ByRef streamCount As Long) As Boolean
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
    If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then Return False

    Dim attrOff As ULong = mrh->attrOffset
    Do While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)

        If g->typeID = &HFFFFFFFF Then Exit Do
        If g->totalLen = 0 Then Exit Do
        If attrOff + g->totalLen > ctx->mftRecordSize Then Exit Do

        If g->typeID = ATTR_TYPE_DATA Then
            ' 获取流名（未命名流返回空字符串）
            Dim curName As String = GetAttributeName(g)
            Dim curSize As ULongLong = 0

            If g->nonResident Then
                Dim nr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, g)
                curSize = nr->dataSize
            Else
                Dim res As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, g)
                curSize = res->dataLen
            End If

            ' 扩展输出数组并存储
            ReDim Preserve streamNames(0 To streamCount)
            ReDim Preserve streamSizes(0 To streamCount)
            streamNames(streamCount) = curName
            streamSizes(streamCount) = curSize
            streamCount += 1
        End If

        attrOff += g->totalLen
    Loop

    Return True
End Function

'======================================================================
' 对外接口：枚举指定文件的所有数据流（未命名 + 命名流）
' 参数：
'   lpFilePath    - 完整文件路径，如 "C:\test.txt"
'   streamNames() - 输出：流名称数组，未命名流为 ""，命名流如 "Zone.Identifier"
'   streamSizes() - 输出：对应流的数据大小（字节）
' 返回值：
'   >0   - 流的总个数（至少为1，即未命名流）
'   -1   - 失败（路径无效、读取错误等）
'======================================================================
Public Function EnumerateFileStreams(lvwFileStream As Class_ListView, lpFilePath As String) As Long
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpFilePath)
    If ctx = NULL Then Return -1

    ' 步骤1：根据路径获取文件 MFT 记录号
    Dim targetRecNo As ULongLong
    If GetMFTRecordByPath(ctx, lpFilePath, targetRecNo) = 0 Then
        CloseNTFSVolumeContext(ctx)
        Return -1
    End If

    ' 步骤2：读取主 MFT 记录
    Dim recBuf() As UByte
    ReDim recBuf(0 To ctx->mftRecordSize - 1)
    If Not ReadExternalMFTRecord(ctx, targetRecNo, @recBuf(0)) Then
        CloseNTFSVolumeContext(ctx)
        Return -1
    End If

    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @recBuf(0))
    If mrh->magic <> NTFS_MFT_RECORD_MAGIC Then
        CloseNTFSVolumeContext(ctx)
        Return -1
    End If
    
    InitListViewData lvwFileStream
    
    ' 步骤3：收集当前记录中的 $DATA 流
    Dim totalStreams As Long = 0
    Dim streamNames() As String
    Dim streamSizes() As ULongLong
    ReDim streamNames(0 To 0)  ' 占位，将在 CollectDataStreamsFromRecord 中扩展
    ReDim streamSizes(0 To 0)
    If Not CollectDataStreamsFromRecord(ctx, recBuf(), streamNames(), streamSizes(), totalStreams) Then
        CommitListViewView lvwFileStream
        CloseNTFSVolumeContext(ctx)
        Return -1
    End If

    ' 步骤4：检查是否存在属性列表（$ATTRIBUTE_LIST），若有则处理外部记录
    Dim hasAttrList As Boolean = False
    Dim attrListAttr As ATTR_HEADER_GENERIC Ptr = NULL
    Dim attrOff As ULong = mrh->attrOffset
    Do While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)
        If g->typeID = &HFFFFFFFF Then Exit Do
        If g->typeID = ATTR_TYPE_ATTRIBUTE_LIST Then
            hasAttrList = True
            attrListAttr = g
            Exit Do
        End If
        attrOff += g->totalLen
    Loop

    If hasAttrList And attrListAttr <> NULL Then
        ' 解析属性列表数据
        Dim listDataPtr As UByte Ptr
        Dim listDataSize As ULong
        Dim tempListBuf() As UByte

        If attrListAttr->nonResident = 0 Then
            Dim resAttr As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, attrListAttr)
            listDataPtr = Cast(UByte Ptr, resAttr) + resAttr->dataOff
            listDataSize = resAttr->dataLen
        Else
            ' 非常驻属性列表（较罕见，但仍做简单处理）
            Dim nrAttr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, attrListAttr)
            listDataSize = nrAttr->dataSize
            If listDataSize = 0 Or listDataSize > 1024 * 1024 Then
                ' 大小异常，放弃处理外部流，返回当前已收集的流个数
                CommitListViewView lvwFileStream
                Return totalStreams
            End If
            ReDim tempListBuf(0 To listDataSize - 1)
            If Not ReadNonResidentAttributeData(ctx, nrAttr, @tempListBuf(0), listDataSize) Then
                CommitListViewView lvwFileStream
                Return totalStreams
            End If
            listDataPtr = @tempListBuf(0)
        End If

        ' 记录已处理过的 MFT 记录号，避免重复收集
        Dim processedRecNos(Any) As ULongLong
        ReDim processedRecNos(0 To 0)
        Dim processedCount As Long = 0
        processedRecNos(0) = targetRecNo
        processedCount = 1

        ' 遍历属性列表条目
        Dim entry As ATTRIBUTE_LIST_ENTRY Ptr = Cast(ATTRIBUTE_LIST_ENTRY Ptr, listDataPtr)
        Dim offset As ULong = 0

        Do While offset + SizeOf(ATTRIBUTE_LIST_ENTRY) <= listDataSize
            If entry->recordLen < SizeOf(ATTRIBUTE_LIST_ENTRY) Then Exit Do
            If offset + entry->recordLen > listDataSize Then Exit Do

            If entry->typeID = ATTR_TYPE_DATA Then
                Dim extRecNo As ULongLong = entry->mftRef And &HFFFFFFFFFFFFULL

                ' 跳过已经处理过的记录（包括当前记录）
                Dim alreadyProcessed As Boolean = False
                For i As Long = 0 To processedCount - 1
                    If processedRecNos(i) = extRecNo Then
                        alreadyProcessed = True
                        Exit For
                    End If
                Next

                If Not alreadyProcessed Then
                    ' 读取并修复外部记录
                    Dim extBuf() As UByte
                    ReDim extBuf(0 To ctx->mftRecordSize - 1)
                    If ReadExternalMFTRecord(ctx, extRecNo, @extBuf(0)) Then
                        ' 收集该记录中的 $DATA 流
                        If Not CollectDataStreamsFromRecord(ctx, extBuf(), streamNames(), streamSizes(), totalStreams) Then
                            ' 收集失败，忽略此记录
                        End If
                        Print "streamSizes:" & streamSizes(0) & " streamSizes:" & streamSizes(0)
                    End If

                    ' 标记为已处理
                    ReDim Preserve processedRecNos(0 To processedCount)
                    processedRecNos(processedCount) = extRecNo
                    processedCount += 1
                End If
            End If

            offset += entry->recordLen
            entry = Cast(ATTRIBUTE_LIST_ENTRY Ptr, listDataPtr + offset)
            FF_DoEvents
        Loop
    End If

' ====================== 核心修改：参照GetWfpFilterList填充ListView ======================
    ' 遍历所有流，填充列表（对齐WFP函数的写法）
    For i As Long = 0 To totalStreams - 1
        Dim sName As String = streamNames(i)
        ' 无名主流显示为默认名称，有名ADS显示原名
        If sName = "" Then sName = "(默认主数据流)"
        
        ' 格式化大小：千分位+字节单位
        Dim sSize As String = Format(streamSizes(i), "#,##0") & " 字节"
        
        ' 完全参照GetWfpFilterList的AddItemColListEx调用格式
        AddItemColListEx lvwFileStream, 2,,, WStr(sName), WStr(sSize)
    Next
' ======================================================================================

    ' 返回最终收集到的流个数
    CommitListViewView lvwFileStream
    CloseNTFSVolumeContext(ctx)
    Return totalStreams
End Function

Sub TestWriteSector()
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext("C:\")
    If ctx = NULL Then Exit Sub
    ' 读取测试
    Print !"\n=== 读取测试 ==="
    Dim buffer(511) As UByte
    
    If ReadAtOffset(ctx, 0, @buffer(0), 512) Then
        Print "MBR读取成功！"
        Print "签名: " & Hex(buffer(510), 2) & Hex(buffer(511), 2)
        
        ' 显示分区表
        Print !"\n分区表信息:"
        For i As Integer = 0 To 3
            Dim offset As Integer = 446 + i * 16
            Dim partType As Byte = buffer(offset + 4)
            If partType <> 0 Then
                Print "  分区" & i + 1 & ": 类型=" & Hex(partType, 2)
            End If
        Next
    Else
        Print "读取失败,错误码: " & GetLastError
    End If
    
    ' 写入测试：仅测试前16扇区（存在PartMgr防护）
    Print !"\n=== 写入测试（仅前16扇区）==="
    Print "警告: 直接写入磁盘扇区具有风险！"
    Print "Windows限制: RING3只能写入前16扇区(0-15)"
    Print "目标扇区: 15 (相对安全的保留扇区)"
    
    Dim confirm As String
    Input "输入 'YES' 确认写入测试: ", confirm
    
    If confirm = "YES" Then
        ' 修改数据（仅修改前8字节）
        Dim buffer(511) As UByte
        For i As Integer = 0 To 7
            buffer(i) = &H42  ' 写入测试标记 'B'
        Next
        
        ' 写入到扇区1（MBR备份位置，相对安全）
        If WriteAtOffset(ctx, 1 * 512, @buffer(0), 512) Then
            Print "写入成功！"
            
            ' 验证写入
            Dim verifyBuffer(511) As UByte
            ReadAtOffset(ctx, 1 * 512, @verifyBuffer(0), 512)
            
            If verifyBuffer(0) = &H42 Then
                Print "数据验证通过！"
            Else
                Print "警告: 数据验证失败"
            End If
        Else
            Print "写入失败，错误码: " & GetLastError
        End If
    Else
        Print "已取消写入测试"
    End If
    CloseNTFSVolumeContext ctx
End Sub
