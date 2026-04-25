Type NTFS_BOOT_SECTOR Field = 1
    jump(0 To 2)       As UByte
    oemID(0 To 7)      As UByte
    BytesPerSector     As Short
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
    bStreamFile As Boolean ' 是否为ADS流文件(如$Repair:$Verify)
    bDirectory As Boolean
End Type

'======================================================================
' 内部上下文结构（原全局变量全部移入此处）
'======================================================================
Type NTFS_CONTEXT
    ' 卷句柄与基础参数（原全局 hVol, bytesPerSector 等）
    hVol As HANDLE
    bytesPerSector As ULong
    sectorsPerCluster As ULong
    clusterSize As ULong
    mftRecordSize As ULong
    mftStartLCN As ULongLong
    volumeTotalClusters As ULONGLONG
    
    ' MFT 片段信息（原全局 mftFragments）
    mftFragments(Any) As MFT_FRAGMENT
    mftFragmentCount As Integer
    
    ' 缓存（原全局 dirCache/pathCache）
    dirCache(Any) As DIR_CACHE_ENTRY
    dirCacheCount As Integer
    pathCache(Any) As PATH_CACHE_ENTRY
    pathCacheCount As Integer
    
    ' 错误跟踪
    lastError As DWORD
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

' 内部全局管理器（26个盘符 A-Z）
Dim Shared g_VolCtx(0 To 25) As NTFS_CONTEXT Ptr
Dim Shared g_Initialized As Boolean = False

'======================================================================
' 底层 IO 函数（新增 ctx 参数）
'======================================================================
Private Function ReadAtOffset(ctx As NTFS_CONTEXT Ptr, ByVal offset As ULongLong, ByVal buf As Any Ptr, ByVal size As ULong) As Boolean
    Dim li As LARGE_INTEGER
    li.QuadPart = offset
    If SetFilePointerEx(ctx->hVol, li, NULL, FILE_BEGIN) = 0 Then
        ctx->lastError = GetLastError()
        Return False
    End If
    Dim bytesRead As DWORD
    If ReadFile(ctx->hVol, buf, size, @bytesRead, NULL) = 0 Then
        ctx->lastError = GetLastError()
        Return False
    End If
    Return (bytesRead = size)
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
    
    ctx->bytesPerSector = bs->BytesPerSector
    ctx->sectorsPerCluster = bs->SectorsPerCluster
    ctx->clusterSize = ctx->bytesPerSector * ctx->sectorsPerCluster
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
    Dim sectorsPerRec As Integer = recordSize \ ctx->bytesPerSector
    
    If CULng(usaOffset) + usaCount * 2 > recordSize Then Return False
    
    For i As Integer = 1 To usaCount - 1
        Dim sectorEndOff As ULong = i * ctx->bytesPerSector - 2
        If sectorEndOff >= recordSize Then Return False
        Dim lastWordPtr As UShort Ptr = Cast(UShort Ptr, recBufPtr + sectorEndOff)
        If lastWordPtr[0] <> usn Then Return False
        lastWordPtr[0] = usaPtr[i]
    Next
    Return True
End Function

Private Function ParseRunList(ctx As NTFS_CONTEXT Ptr, ByVal runPtr As UByte Ptr, ByRef currentCluster As ULongLong, ByRef runLength As ULongLong) As UByte Ptr
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
    
    If offSize > 0 Then
        currentCluster += offsetDelta
    Else
        currentCluster = 0
    End If
    
    Return p + 1 + lenSize + offSize
End Function

Private Function ParseMFTDataAttribute(ctx As NTFS_CONTEXT Ptr) As Boolean
    Dim mftBuf() As UByte
    ReDim mftBuf(0 To ctx->mftRecordSize - 1)
    
    ' 读取 $MFT 记录（使用简化方式，假设前几个记录连续）
    Dim recOffset As ULongLong = ctx->mftStartLCN * ctx->clusterSize
    If Not ReadAtOffset(ctx, recOffset, @mftBuf(0), ctx->mftRecordSize) Then Return False
    If Not FixupMFTRecord(ctx, @mftBuf(0), ctx->mftRecordSize) Then Return False
    
    Dim mrh As MFT_RECORD_HEADER Ptr = Cast(MFT_RECORD_HEADER Ptr, @mftBuf(0))
    Dim attrOff As ULong = mrh->attrOffset
    
    While attrOff + SizeOf(ATTR_HEADER_GENERIC) <= ctx->mftRecordSize
        Dim g As ATTR_HEADER_GENERIC Ptr = Cast(ATTR_HEADER_GENERIC Ptr, @mftBuf(0) + attrOff)
        
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
                Return False ' $MFT 不可能是常驻属性
            End If
        End If
        attrOff += g->totalLen
    Wend
    Return False
End Function

Function GetSystemFileName(ByVal recNo As ULong) As String
    Dim SysFileName(16) As ZString * 20 = {"$MFT", "$MFTMirr", "$LogFile", "$Volume", _
    "$AttrDef", "$Root", "$Bitmap", "$Boot", _
    "$BadClus", "$Secure", "$UpCase", _
    "$Extend", "Reserved1", "$Extend\$UsnJrnl", "Reserved2", "Reserved3"}
    If recNo <=16 Then Return SysFileName(recNo) Else Return ""
End Function

'======================================================================
' 卷生命周期管理（内部使用）
'======================================================================
Private Function OpenVolumeInternal(ctx As NTFS_CONTEXT Ptr, strDrive As String) As Boolean
    ctx->hVol = CreateFile("\\.\" & strDrive, GENERIC_READ Or GENERIC_WRITE, _
                      FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                      NULL, OPEN_EXISTING, 0, NULL)
    
    If ctx->hVol = INVALID_HANDLE_VALUE Then Return False
    
    Dim bootBuf(0 To 511) As UByte
    If Not ReadAtOffset(ctx, 0, @bootBuf(0), 512) Then Goto Cleanup
    If Not ParseBoot(ctx, @bootBuf(0)) Then Goto Cleanup
    If Not ParseMFTDataAttribute(ctx) Then Goto Cleanup
    
    Return True
    
Cleanup:
    CloseHandle(ctx->hVol)
    ctx->hVol = INVALID_HANDLE_VALUE
    Return False
End Function

Private Sub CloseVolumeInternal(ctx As NTFS_CONTEXT Ptr)
    If ctx->hVol <> INVALID_HANDLE_VALUE And ctx->hVol <> NULL Then
        CloseHandle(ctx->hVol)
        ctx->hVol = INVALID_HANDLE_VALUE
    End If
    Erase ctx->mftFragments
    Erase ctx->dirCache
    Erase ctx->pathCache
End Sub

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

' 获取或创建卷上下文（自动打开卷）
Private Function OpenNTFSVolumeContext(ByVal path As String) As NTFS_CONTEXT Ptr
    Dim idx As Integer = GetDriveIndex(path)
    If idx < 0 Then Return NULL
    
    ' 首次使用自动初始化
    If Not g_Initialized Then
        memset(@g_VolCtx(0), 0, SizeOf(g_VolCtx))
        g_Initialized = True
    End If
    
    ' 已存在且有效，直接返回
    If g_VolCtx(idx) <> NULL Then
        ' 可选：检查句柄是否仍然有效（卷是否被移除）
        Return g_VolCtx(idx)
    End If
    
    ' 创建新上下文
    Dim ctx As NTFS_CONTEXT Ptr = Allocate(SizeOf(NTFS_CONTEXT))
    If ctx = NULL Then Return NULL
    memset(ctx, 0, SizeOf(NTFS_CONTEXT))
    
    ' 打开卷并解析（修改后的 OpenVolume 接受 ctx 指针）
    Dim drive As String = Chr(Asc("A") + idx) & ":"
    If Not OpenVolumeInternal(ctx, drive) Then
        Deallocate(ctx)
        Return NULL
    End If
    
    g_VolCtx(idx) = ctx
    Return ctx
End Function

' 清理特定卷的上下文（供程序退出或手动重置使用）
Public Sub CloseNTFSVolumeContext(ByVal drive As String)
    Dim idx As Integer = GetDriveIndex(drive & "\")
    If idx < 0 Or g_VolCtx(idx) = NULL Then Exit Sub
    
    CloseVolumeInternal(g_VolCtx(idx))
    Deallocate(g_VolCtx(idx))
    g_VolCtx(idx) = NULL
End Sub

' 清理所有卷（程序退出时调用）
Public Sub CleanupAllNTFSVolumes()
    For i As Integer = 0 To 25
        If g_VolCtx(i) <> NULL Then
            CloseNTFSVolumeContext(Chr(Asc("A") + i) & ":")
        End If
    Next
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

Private Function ReadNonResidentAttributeData(ctx As NTFS_CONTEXT Ptr, ByVal nonResAttr As ATTR_HEADER_NONRESIDENT Ptr, ByVal buffer As UByte Ptr, ByVal bufferSize As ULong) As Boolean
    If nonResAttr = NULL Or buffer = NULL Then Return False
    
    Dim runPtr As UByte Ptr = Cast(UByte Ptr, nonResAttr) + nonResAttr->runOff
    Dim currentVCN As ULongLong = 0
    Dim bufOffset As ULong = 0
    
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
        
        Dim startLCN As ULongLong = currentVCN + runOffset
        Dim bytesToRead As ULong = runLen * ctx->clusterSize
        If bufOffset + bytesToRead > bufferSize Then bytesToRead = bufferSize - bufOffset
        
        If Not ReadClusters(ctx, startLCN, runLen, buffer + bufOffset) Then Return False
        
        bufOffset += bytesToRead
        currentVCN += runLen
        runPtr += 1 + runLenBytes + runOffsetBytes
    Wend
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
        Dim sectorEndOff As ULong = i * ctx->bytesPerSector - 2
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
    
    Dim attrOff As ULong = mrh->attrOffset
    
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
            
        Case ATTR_TYPE_DATA
            Dim streamName As String = GetAttributeName(g)
            If streamName <> "" Then
                fileInfo.bStreamFile = True
                fileInfo.cFileName = streamName
            Else
                fileInfo.bStreamFile = False
            End If
            
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
' 卷生命周期管理（内部使用）
'======================================================================
Private Function OpenVolume(ctx As NTFS_CONTEXT Ptr, strDrive As String) As Boolean
    ctx->hVol = CreateFile("\\.\" & strDrive, GENERIC_READ Or GENERIC_WRITE, _
                      FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                      NULL, OPEN_EXISTING, 0, NULL)
    
    If ctx->hVol = INVALID_HANDLE_VALUE Then Return False
    
    Dim bootBuf(0 To 511) As UByte
    If Not ReadAtOffset(ctx, 0, @bootBuf(0), 512) Then Goto Cleanup
    If Not ParseBoot(ctx, @bootBuf(0)) Then Goto Cleanup
    If Not ParseMFTDataAttribute(ctx) Then Goto Cleanup
    
    Return True
    
Cleanup:
    CloseHandle(ctx->hVol)
    ctx->hVol = INVALID_HANDLE_VALUE
    Return False
End Function

Private Sub CloseVolume(ctx As NTFS_CONTEXT Ptr)
    If ctx->hVol <> INVALID_HANDLE_VALUE And ctx->hVol <> NULL Then
        CloseHandle(ctx->hVol)
        ctx->hVol = INVALID_HANDLE_VALUE
    End If
    Erase ctx->mftFragments
    Erase ctx->dirCache
    Erase ctx->pathCache
End Sub

'======================================================================
' 对外接口 Internal 实现（供包装层调用）
'======================================================================
Private Function GetDirList_Internal(ctx As NTFS_CONTEXT Ptr, lpDirPath As String, fileInfoList() As WIN32_FILE_DATAEX2W) As Long
    Dim fileCount As Long = 0
    Dim parts() As String
    vbSplit lpDirPath, "\", parts()
    
    If ctx = NULL Then Return -1
    
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
    Print "dirPath:" & dirPath
    ' 调用你现有GetDirList遍历目录，获取文件列表
    listCount = GetDirList_Internal(ctx, dirPath, fileInfoList()) ' 假设GetDirList返回文件数量，并填充fileInfoList
    If listCount <= 0 Then Return 0
    
    Print "fileName:" & fileName
    
    ' 遍历文件列表，匹配文件名
    For i As Long = 0 To listCount - 1
        'Print fileInfoList(i).cFileName
        Print "fileInfoList(i).cFileName:" & fileInfoList(i).cFileName
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
    Print ">>> [调试] 卷基础参数:"
    Print "      ClusterSize: " & ctx->clusterSize
    Print "      MFT Start LCN: " & ctx->mftStartLCN
    Print "      MFT Record Size: " & ctx->mftRecordSize
    Print "成功解析路径，对应MFT记录号：" & targetMftRecordNum
    
    ' ========== 步骤2：读取MFT记录（带USA修复） ==========
    'If Not ParseVolume(parts(0)) Then Return -1
    'If Not ParseMFTDataAttribute Then Return -1
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
    
    Print "      Magic: " & Hex(mrh->magic, 8) & " (期望: 454C4946)"
    Print "      Flags: " & Hex(mrh->dwFlags, 4) & " (Bit0=InUse, Bit1=IsDir)"
    Print "      AttrOffset: " & mrh->attrOffset
    Print "      BytesInUse: " & mrh->bytesInUse
    
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
        Print ">>> [调试] 检查属性偏移: " & attrOff
        gAttr = Cast(ATTR_HEADER_GENERIC Ptr, @recBuf(0) + attrOff)
        Print "      TypeID: " & Hex(gAttr->typeID, 8) & " (80=DATA, 30=FILENAME)"
        Print "      TotalLen: " & gAttr->totalLen
        Print "      NonResident: " & gAttr->nonResident
        
        ' 终止条件：属性类型ID为0xFFFFFFFF
        If gAttr->typeID = &HFFFFFFFF Then Exit Do
        
        ' 找到DATA属性（类型0x80）
        If gAttr->typeID = &H80 Then
            Print ">>> [√] 找到 DATA 属性 (在" & IIf(isExternalRecord, "外部", "当前") & "记录中)"
            ' 直接移交控制权给下方的现有处理逻辑
            Goto ProcessFoundDataAttr
        ElseIf gAttr->typeID = ATTR_TYPE_ATTRIBUTE_LIST AndAlso Not isExternalRecord Then
            Print ">>> [调试] 发现 Attribute List，开始搜索外部 DATA..."
            
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
                    Print ">>> [调试] 找到外部 DATA，位于 MFT 记录: " & extMftRecNo
                    
                    ' 3. 读取外部 MFT 记录
                    ReDim targetMftBuf(0 To ctx->mftRecordSize - 1)
                    If ReadExternalMFTRecord(ctx, extMftRecNo, @targetMftBuf(0)) Then
                        ' 4. 切换上下文：将 recBuf 指向外部记录，并重置 mrh 指针
                        ' 注意：这里我们利用数组的内存连续性，直接操作 @recBuf(0) 是关键
                        ' FreeBASIC 动态数组内存是连续的，我们可以用 memcpy 覆盖它
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
    
    ' 如果走到这里，说明没找到 DATA 属性
    Print "[!] 错误：未找到 $DATA 属性 (0x80)"
    Return False
    
ProcessFoundDataAttr : 
    ' 区分常驻/非常驻属性（大部分文件是非常驻）
    Dim dataSize As ULong = 0
   If gAttr->nonResident = 1 Then
        Dim nr As ATTR_HEADER_NONRESIDENT Ptr = Cast(ATTR_HEADER_NONRESIDENT Ptr, gAttr)
        Print ">>> [调试] 非常驻 DATA 属性详情:"
        Print "      LowVCN: " & nr->lowVCN
        Print "      HighVCN: " & nr->highVCN
        Print "      DataSize: " & nr->dataSize
        Print "      AllocSize: " & nr->allocSize
        Print "      RunOff: " & nr->runOff
        dataSize = nr->dataSize
        Dim allocatedSize As ULongLong = nr->allocSize ' 已分配大小
        
        /'Print "    找到非驻留DATA属性："
        Print "      文件大小: " & fileSize & " 字节"
        Print "      分配大小: " & allocatedSize & " 字节"'/
        
        ' 防御性检查
        If dataSize = 0 Or dataSize > 256*1024*1024 Then
            Print "    [!] 非驻留数据大小异常: " & dataSize
            Goto ContinueDo
        End If
        
        Dim pfileData As UByte Ptr = Allocate(dataSize)
        If pfileData = NULL Then
            Print "    [!] 内存分配失败: " & dataSize & " 字节"
            Goto ContinueDo
        End If
        
        ' 初始化内存为零（处理稀疏运行）
        memset(pfileData, 0, dataSize)
        
        Dim runPtr As UByte Ptr = Cast(UByte Ptr, nr) + nr->runOff
        Dim currentLCN As ULongLong = 0
        Dim vcn As ULongLong = nr->lowVCN
        Dim bytesWritten As ULongLong = 0
        Dim clusterBuffer As UByte Ptr = Allocate(ctx->clusterSize)
        
        If clusterBuffer = NULL Then
            Print "    [!] 簇缓冲区分配失败"
            Deallocate(pfileData)
            Goto ContinueDo
        End If
        
        Print "    开始解析RunList..."
        Dim runIndex As Integer = 0
        Dim baseVCN As ULongLong = nr->lowVCN  ' 新增：保存该属性的起始 VCN

        While runPtr <> NULL And runPtr[0] <> 0
            Dim runLen As ULongLong
            runPtr = ParseRunList(ctx, runPtr, currentLCN, runLen)
            
            If runPtr = NULL Then Exit While
            
            Print "      Run " & runIndex & ": VCN=" & vcn & " LCN=" & currentLCN & " 长度=" & runLen & " 簇"
            
            If runLen = 0 Or runLen > 1000000 Then
                Print "    [!] 无效的run长度: " & runLen
                Exit While
            End If
            
            For i As ULong = 0 To runLen - 1
                Dim lcn As ULongLong = currentLCN + i
                
                ' ============== 核心修改开始 ==============
                ' 错误写法：Dim offsetInData As ULongLong = bytesWritten
                ' 正确写法：基于 (当前VCN - 起始VCN) 计算文件偏移
                Dim currentVCN As ULongLong = vcn + i
                Dim offsetInData As ULongLong = (currentVCN - baseVCN) * ctx->clusterSize
                ' ============== 核心修改结束 ==============
                
                ' 边界检查
                If offsetInData >= dataSize Then 
                    Print "      [!] 偏移超出数据大小: " & offsetInData & " >= " & dataSize
                    Exit For
                End If
                
                Dim bytesToRead As ULong = ctx->clusterSize
                If offsetInData + bytesToRead > dataSize Then
                    bytesToRead = dataSize - offsetInData
                End If
                
                If bytesToRead = 0 Then Exit For
                
                If lcn = 0 Then
                    ' 稀疏运行：保持为0（已初始化）
                    bytesWritten += bytesToRead
                Else
                    ' 正常运行：读取数据
                    If ReadClusters(ctx, lcn, 1, clusterBuffer) Then
                        memcpy(pfileData + offsetInData, clusterBuffer, bytesToRead)
                        bytesWritten += bytesToRead
                    Else
                        Print "      [!] 读取簇失败: LCN=" & lcn
                    End If
                End If
            Next
            
            vcn += runLen  ' 正确跟踪下一个 Run 的起始 VCN
            runIndex += 1
        Wend
        
        ' 显示结果
        Print "    读取完成: " & bytesWritten & " / " & dataSize & " 字节"
        
        If bytesWritten > 0 Then
            ' 显示前16字节（十六进制）
            /'Print "    前16字节(HEX): ";
            For i As Integer = 0 To IIf(dataSize > 15, 15, dataSize - 1)
                Print Hex(pfileData[i], 2) & " ";
            Next
            Print
            Print "编码类型:" & GetEncodingType(pfileData, dataSize)'/
            ' 显示为ASCII文本
            /'Print "    ASCII预览: ";
            Dim strData As String =  Utf8ToAnsi(pfileData, IIf(dataSize > 16, 16, dataSize))
            Print strData
            Print'/
            ReDim bytData(bytesWritten - 1) As UByte
            For i As Integer = 0 To bytesWritten - 1
                bytData(i) = pfileData[i]
            Next
        End If
        result = True
    Else
        ' 处理小文件（常驻属性）
        Dim resAttr As ATTR_HEADER_RESIDENT Ptr = Cast(ATTR_HEADER_RESIDENT Ptr, gAttr)
        fileSize = resAttr->dataLen
        ' 直接读取常驻数据
        'Dim dataPtr As UByte Ptr = @recBuf(0) + attrOff + SizeOf(ATTR_HEADER_RESIDENT) + resAttr->dataOff
        ' 修正：dataOff是从resident头开始的偏移，不需要加SizeOf
        Dim dataPtr As UByte Ptr = Cast(UByte Ptr, resAttr) + resAttr->dataOff
        
        If dataPtr <> NULL Then
            ' 显示前16字节（十六进制）
            /'Print "    前16字节(HEX): ";
            For i As Integer = 0 To IIf(fileSize > 15, 15, fileSize - 1)
                Print Hex(dataPtr[i], 2) & " ";
            Next
            Print
            Print "编码类型:" & GetEncodingType(dataPtr, fileSize)'/
            ' 显示为ASCII文本
            /'Print "    ASCII预览: ";
            Dim strData As String =  Utf8ToAnsi(dataPtr, IIf(fileSize > 16, 16, fileSize))
            Print strData
            Print'/
            ReDim bytData(fileSize - 1) As UByte
            For i As Integer = 0 To fileSize - 1
                bytData(i) = dataPtr[i]
            Next
        End If
            
        result = True
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
    Dim hFile As HANDLE = CreateFile(lpTargetFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        Print "CreateFile失败:" & WinErrorMsg(GetLastError)
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
' 对外接口（完全保持原签名）
'======================================================================
Function GetDirList(lpDirPath As String, fileInfoList() As WIN32_FILE_DATAEX2W) As Long
    ' 自动获取或创建对应卷的上下文
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpDirPath)
    If ctx = NULL Then 
        ' 卷打开失败，返回错误
        Return -1
    End If
    
    ' 调用内部实现，传入正确的上下文
    Return GetDirList_Internal(ctx, lpDirPath, fileInfoList())
End Function

' 同理，ReadFile2 和 MyCopyFile 的包装实现：
Function ReadFile2(ByVal lpFilePath As String, bytData() As UByte) As Boolean
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpFilePath)
    If ctx = NULL Then Return False
    Return ReadFile2_Internal(ctx, lpFilePath, bytData())
End Function

Function MyCopyFile(lpSourceFile As StringW, lpTargetFile As StringW) As Boolean
    ' 源文件和目标文件可能在不同卷，需要分别获取上下文
    ' 但 MyCopyFile 主要是读取源文件，写入目标文件是普通 Win32 API
    ' 因此只需要源文件的上下文
    Dim ctx As NTFS_CONTEXT Ptr = OpenNTFSVolumeContext(lpSourceFile)
    If ctx = NULL Then Return False
    Return MyCopyFile_Internal(ctx, lpSourceFile, lpTargetFile)
End Function
    