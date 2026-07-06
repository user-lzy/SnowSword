' ============================================================================
' 常量
' ============================================================================
Const FAT32_EOC_MIN     = &h0FFFFFF8
Const FAT32_EOC_MAX     = &h0FFFFFFF
Const FAT32_BAD_CLUSTER = &h0FFFFFF7
Const FAT32_FREE_CLUSTER = 0

Const ATTR_READ_ONLY    = &h01
Const ATTR_HIDDEN       = &h02
Const ATTR_SYSTEM       = &h04
Const ATTR_VOLUME_ID    = &h08
Const ATTR_DIRECTORY    = &h10
Const ATTR_ARCHIVE      = &h20
Const ATTR_LONG_NAME    = &h0F
Const ATTR_LONG_NAME_MASK = &h3F

' ============================================================================
' 结构定义
' ============================================================================
Type FAT32_BPB Field = 1
    jmpBoot(0 To 2)         As UByte
    oemName(0 To 7)         As UByte
    bytesPerSector          As UShort
    sectorsPerCluster       As UByte
    reservedSectorCount     As UShort
    numFATs                 As UByte
    rootEntryCount          As UShort
    totalSectors16          As UShort
    media                   As UByte
    fatSize16               As UShort
    sectorsPerTrack         As UShort
    numHeads                As UShort
    hiddenSectors           As ULong
    totalSectors32          As ULong
    fatSize32               As ULong
    extFlags                As UShort
    fsVersion               As UShort
    rootCluster             As ULong
    fsInfo                  As UShort
    backupBootSector        As UShort
    reserved2(0 To 11)      As UByte
    driveNum                As UByte
    reserved3               As UByte
    bootSig                 As UByte
    volumeID                As ULong
    volumeLabel(0 To 10)    As UByte
    fileSysType(0 To 7)     As UByte
End Type

Type FAT32_DIR_ENTRY_SFN Field = 1
    name(0 To 10)           As UByte
    attr                    As UByte
    ntRes                   As UByte
    crtTimeTenth            As UByte
    crtTime                 As UShort
    crtDate                 As UShort
    lstAccDate              As UShort
    fstClusHI               As UShort
    wrtTime                 As UShort
    wrtDate                 As UShort
    fstClusLO               As UShort
    fileSize                As ULong
End Type

Type FAT32_DIR_ENTRY_LFN Field = 1
    ord                     As UByte
    name1(0 To 9)           As UByte
    attr                    As UByte
    type                    As UByte
    checksum                As UByte
    name2(0 To 11)          As UByte
    fstClusLO               As UShort
    name3(0 To 3)           As UByte
End Type

Type FILE_ENTRY_COLLECTED
    sfn                     As FAT32_DIR_ENTRY_SFN
    lfnParts(0 To 19)       As FAT32_DIR_ENTRY_LFN   ' 20 个 LFN 条目，每个 32 字节
    numLFN                  As Integer
    hasSFN                  As Boolean
End Type

' ============================================================================
' FAT32 上下文
' ============================================================================
Type FAT32_CONTEXT
    io                      As DISK_IO_CONTEXT   ' 统一磁盘 IO
    
    ' FAT32 解析结果
    bytesPerSector          As ULong
    sectorsPerCluster       As UByte
    reservedSectors         As UShort
    numFATs                 As UByte
    totalSectors            As ULong
    fatSize                 As ULong
    rootCluster             As ULong
    dataStartSector         As ULong
    totalClusters           As ULong
    maxCluster              As ULong
    fatStartOffset          As ULongLong
    fatCache                As UByte Ptr
    fatCacheSize            As ULong
    fsInfoSector            As UShort
    backupBootSector        As UShort
End Type

' ============================================================================
' 全局管理
' ============================================================================
Dim Shared g_FAT32_VolCtx(0 To 25) As FAT32_CONTEXT Ptr
Dim Shared g_FAT32_Initialized As Boolean = False

Function IsEOC(cluster As ULong) As Boolean
    Return cluster >= FAT32_EOC_MIN And cluster <= FAT32_EOC_MAX
End Function

Function ClusterToOffset(ctx As FAT32_CONTEXT Ptr, cluster As ULong) As ULongLong
    Dim sector As ULong = ctx->dataStartSector + (cluster - 2) * ctx->sectorsPerCluster
    Return CULngInt(sector) * ctx->bytesPerSector
End Function

Function ReadCluster(ctx As FAT32_CONTEXT Ptr, cluster As ULong, buf As Any Ptr) As Boolean
    Dim clusterSize As ULong = CULng(ctx->sectorsPerCluster) * ctx->bytesPerSector
    Return DiskIO_Read(@ctx->io, ClusterToOffset(ctx, cluster), buf, clusterSize)
End Function

Function DOSDateTimeToFileTime_1(dosDate As UShort, dosTime As UShort) As FILETIME
    Dim st As SYSTEMTIME
    st.wYear = 1980 + (dosDate Shr 9)
    st.wMonth = (dosDate Shr 5) And &h0F
    st.wDay = dosDate And &h1F
    st.wHour = dosTime Shr 11
    st.wMinute = (dosTime Shr 5) And &h3F
    st.wSecond = (dosTime And &h1F) * 2
    st.wMilliseconds = 0
    Dim ft As FILETIME
    SystemTimeToFileTime(@st, @ft)
    Return ft
End Function

Function DOSDateTimeToFileTimeEx(dosDate As UShort, dosTime As UShort, tenths As UByte) As FILETIME
    Dim ft As FILETIME = DOSDateTimeToFileTime_1(dosDate, dosTime)
    If tenths > 0 Then
        Dim ll As ULONGLONG = *CPtr(ULONGLONG Ptr, @ft)
        ll += CULngInt(tenths) * 1000000
        *CPtr(ULONGLONG Ptr, @ft) = ll
    End If
    Return ft
End Function

Function LFN_Checksum(name83 As UByte Ptr) As UByte
    Dim sum As UByte = 0
    For i As Integer = 0 To 10
        sum = ((sum And 1) Shl 7) + (sum Shr 1) + name83[i]
    Next
    Return sum
End Function

' ============================================================================
' 短文件名格式化
' ============================================================================
Function SFN_ToString(sfn As FAT32_DIR_ENTRY_SFN Ptr) As String
    If sfn = NULL Then Return ""
    Dim s As String = ""
    Dim i As Integer
    For i = 0 To 7
        Dim c As UByte = sfn->name(i)
        If c = &h20 Then Exit For
        s += Chr(c)
    Next
    Dim ext As String = ""
    For i = 8 To 10
        Dim c As UByte = sfn->name(i)
        If c = &h20 Then Exit For
        ext += Chr(c)
    Next
    If ext <> "" Then s += "." + ext
    Return s
End Function

' ============================================================================
' 长文件名组装（返回 String，避免 VFB 固定长度字符串返回限制）
' ============================================================================
Function AssembleFileName(entry As FILE_ENTRY_COLLECTED Ptr) As String
    If entry->numLFN = 0 Then Return SFN_ToString(@entry->sfn)
    
    Dim result As WString * 260 = ""
    Dim i As Integer
    For i = entry->numLFN - 1 To 0 Step -1
        Dim lfn As FAT32_DIR_ENTRY_LFN Ptr = @entry->lfnParts(i)
        Dim chars(0 To 12) As UShort
        Dim j As Integer, k As Integer = 0
        
        For j = 0 To 4
            chars(k) = *CPtr(UShort Ptr, @lfn->name1(j * 2))
            k += 1
        Next
        For j = 0 To 5
            chars(k) = *CPtr(UShort Ptr, @lfn->name2(j * 2))
            k += 1
        Next
        For j = 0 To 1
            chars(k) = *CPtr(UShort Ptr, @lfn->name3(j * 2))
            k += 1
        Next
        
        For j = 0 To 12
            If chars(j) = 0 Or chars(j) = &hFFFF Then Exit For
            result += WChr(chars(j))
        Next
    Next
    Return result
End Function

' ============================================================================
' FAT 表操作
' ============================================================================
Function LoadFATCache(ctx As FAT32_CONTEXT Ptr) As Boolean
    Dim fatBytes As ULong = CULngInt(ctx->fatSize) * ctx->bytesPerSector
    If fatBytes <= 32 * 1024 * 1024 Then
        ctx->fatCache = Allocate(fatBytes)
        If ctx->fatCache = NULL Then Return False
        ctx->fatCacheSize = fatBytes
        If Not DiskIO_Read(@ctx->io, ctx->fatStartOffset, ctx->fatCache, fatBytes) Then
            Deallocate ctx->fatCache
            ctx->fatCache = NULL
            Return False
        End If
    End If
    Return True
End Function

Function GetFATEntry(ctx As FAT32_CONTEXT Ptr, cluster As ULong) As ULong
    If cluster < 2 Or cluster > ctx->maxCluster Then Return FAT32_EOC_MAX
    Dim entryOffset As ULong = cluster * 4
    If ctx->fatCache <> NULL Then
        Dim entry As ULong Ptr = CPtr(ULong Ptr, ctx->fatCache + entryOffset)
        Return *entry And &h0FFFFFFF
    Else
        Dim buf As ULong
        Dim fatOffset As ULongLong = ctx->fatStartOffset + entryOffset
        If Not DiskIO_Read(@ctx->io, fatOffset, @buf, 4) Then Return FAT32_EOC_MAX
        Return buf And &h0FFFFFFF
    End If
End Function

' ============================================================================
' BPB 解析
' ============================================================================
Function ParseBPB(ctx As FAT32_CONTEXT Ptr, buf As UByte Ptr) As Boolean
    Dim bpb As FAT32_BPB Ptr = CPtr(FAT32_BPB Ptr, buf)
    If buf[510] <> &h55 Or buf[511] <> &hAA Then Return False
    
    ctx->bytesPerSector = bpb->bytesPerSector
    ctx->sectorsPerCluster = bpb->sectorsPerCluster
    ctx->reservedSectors = bpb->reservedSectorCount
    ctx->numFATs = bpb->numFATs
    ctx->fatSize = bpb->fatSize32
    ctx->rootCluster = bpb->rootCluster
    ctx->fsInfoSector = bpb->fsInfo
    ctx->backupBootSector = bpb->backupBootSector
    
    If bpb->totalSectors16 <> 0 Then
        ctx->totalSectors = bpb->totalSectors16
    Else
        ctx->totalSectors = bpb->totalSectors32
    End If
    
    If ctx->bytesPerSector <> 512 And ctx->bytesPerSector <> 1024 And _
       ctx->bytesPerSector <> 2048 And ctx->bytesPerSector <> 4096 Then Return False
    If ctx->sectorsPerCluster = 0 Or (ctx->sectorsPerCluster And (ctx->sectorsPerCluster - 1)) <> 0 Then Return False
    If ctx->fatSize = 0 Then Return False
    
    ctx->dataStartSector = ctx->reservedSectors + (ctx->numFATs * ctx->fatSize)
    ctx->totalClusters = (ctx->totalSectors - ctx->dataStartSector) \ ctx->sectorsPerCluster
    ctx->maxCluster = ctx->totalClusters + 1
    ctx->fatStartOffset = CULngInt(ctx->reservedSectors) * ctx->bytesPerSector
    Return True
End Function 

' ============================================================================
' 目录条目读取（簇链遍历）
' ============================================================================
Function ReadDirectoryEntries(ctx As FAT32_CONTEXT Ptr, _
                              cluster As ULong, _
                              entries() As FILE_ENTRY_COLLECTED) As Integer
    Dim maxEntries As Integer = 1000
    ReDim entries(0 To maxEntries - 1)
    Dim count As Integer = 0
    
    Dim clusterSize As ULong = CULng(ctx->sectorsPerCluster) * ctx->bytesPerSector
    Dim clusterBuf As UByte Ptr = Allocate(clusterSize)
    If clusterBuf = NULL Then Return -1
    
    Dim currentCluster As ULong = cluster
    
    ' 临时 LFN 收集器（FAT32 中 LFN 在 SFN 之前出现）
    Dim tempLFN(0 To 19) As FAT32_DIR_ENTRY_LFN
    Dim tempLFNCount As Integer = 0
    
    Do While currentCluster >= 2 And currentCluster <= ctx->maxCluster
        If Not ReadCluster(ctx, currentCluster, clusterBuf) Then Exit Do
        
        Dim offset As ULong = 0
        Do While offset + 32 <= clusterSize
            Dim entryPtr As UByte Ptr = clusterBuf + offset
            Dim firstByte As UByte = entryPtr[0]
            
            If firstByte = 0 Then
                Deallocate clusterBuf
                Return count
            End If
            
            If firstByte = &hE5 Then
                ' 已删除条目：丢弃之前收集的 LFN（它们属于被删除的文件）
                tempLFNCount = 0
                offset += 32
                Continue Do
            End If
            
            Dim attr As UByte = entryPtr[11]
            
            If attr = ATTR_LONG_NAME Then
                ' 长文件名条目：按顺序收集
                If tempLFNCount < 20 Then
                    memcpy(@tempLFN(tempLFNCount), entryPtr, 32)
                    tempLFNCount += 1
                End If
            Else
                ' 短文件名条目、卷标、空条目等
                If attr = ATTR_VOLUME_ID Then
                    ' 卷标：不是文件，丢弃 LFN
                    tempLFNCount = 0
                    offset += 32
                    Continue Do
                End If
                
                ' 这是一个有效的文件/目录条目
                If count >= maxEntries Then
                    maxEntries += 1000
                    ReDim Preserve entries(0 To maxEntries - 1)
                End If
                
                memset(@entries(count), 0, SizeOf(FILE_ENTRY_COLLECTED))
                memcpy(@entries(count).sfn, entryPtr, 32)
                entries(count).hasSFN = True
                
                ' 如果有收集的 LFN，验证 checksum 后附加到这个 SFN
                If tempLFNCount > 0 Then
                    Dim calcChecksum As UByte = LFN_Checksum(@entries(count).sfn.name(0))
                    Dim valid As Boolean = True
                    Dim c As Integer
                    For c = 0 To tempLFNCount - 1
                        If tempLFN(c).checksum <> calcChecksum Then
                            valid = False
                            Exit For
                        End If
                    Next
                    
                    If valid Then
                        ' 保持目录中的原始顺序存入 lfnParts
                        ' lfnParts(0) = 目录中第一个 LFN 条目 = 最后一个片段
                        ' lfnParts(tempLFNCount-1) = 目录中最后一个 LFN 条目 = 第一个片段
                        ' AssembleFileName 从 numLFN-1 到 0 遍历，即先处理第一个片段
                        For c = 0 To tempLFNCount - 1
                            memcpy(@entries(count).lfnParts(c), @tempLFN(c), 32)
                        Next
                        entries(count).numLFN = tempLFNCount
                    End If
                End If
                
                count += 1
                tempLFNCount = 0  ' 重置收集器，准备下一个文件
            End If
            offset += 32
        Loop
        
        currentCluster = GetFATEntry(ctx, currentCluster)
        If IsEOC(currentCluster) Then Exit Do
    Loop
    
    Deallocate clusterBuf
    Return count
End Function

' ============================================================================
' 路径解析：逐级查找目录簇号
' ============================================================================
Function ResolvePathToCluster(ctx As FAT32_CONTEXT Ptr, path As String) As ULong
    path = Trim(path)
    If Left(path, 1) = "\" Then path = Mid(path, 2)
    If Right(path, 1) = "\" Then path = Left(path, Len(path) - 1)
    If path = "" Then Return ctx->rootCluster
    
    Dim parts() As String
    vbSplit path, "\", parts()
    
    Dim currentCluster As ULong = ctx->rootCluster
    Dim i As Integer
    For i = 0 To UBound(parts)
        Dim targetName As String = UCase(Trim(parts(i)))
        If targetName = "" Then Continue For
        
        Dim dirEntries() As FILE_ENTRY_COLLECTED
        Dim entryCount As Integer = ReadDirectoryEntries(ctx, currentCluster, dirEntries())
        If entryCount <= 0 Then Return 0
        
        Dim found As Boolean = False
        Dim j As Integer
        For j = 0 To entryCount - 1
            Dim entryName As String = UCase(AssembleFileName(@dirEntries(j)))
            If entryName = targetName Then
                If i < UBound(parts) Then
                    If (dirEntries(j).sfn.attr And ATTR_DIRECTORY) = 0 Then Return 0
                End If
                Dim clusHigh As ULong = dirEntries(j).sfn.fstClusHI
                Dim clusLow As ULong = dirEntries(j).sfn.fstClusLO
                currentCluster = (clusHigh Shl 16) Or clusLow
                found = True
                Exit For
            End If
        Next
        If Not found Then Return 0
    Next
    Return currentCluster
End Function

' ============================================================================
' 目录条目 → WIN32_FILE_DATAEX2W
' ============================================================================
Sub DirEntryToFileInfo(ctx As FAT32_CONTEXT Ptr, _
                       entry As FILE_ENTRY_COLLECTED Ptr, _
                       info As WIN32_FILE_DATAEX2W Ptr)
    memset(info, 0, SizeOf(WIN32_FILE_DATAEX2W))
    
    info->cFileName = AssembleFileName(entry)
    info->dwFileAttributes = entry->sfn.attr And &h3F
    info->bDirectory = (entry->sfn.attr And ATTR_DIRECTORY) <> 0
    
    info->ftCreationTime = DOSDateTimeToFileTimeEx(entry->sfn.crtDate, entry->sfn.crtTime, entry->sfn.crtTimeTenth)
    info->ftLastWriteTime = DOSDateTimeToFileTime_1(entry->sfn.wrtDate, entry->sfn.wrtTime)
    info->ftLastAccessTime = DOSDateTimeToFileTime_1(entry->sfn.lstAccDate, 0)
    
    info->dwFileSizeLow = entry->sfn.fileSize
    info->dwFileSizeHigh = 0
    info->logicalClusterNumber = (CULngInt(entry->sfn.fstClusHI) Shl 16) Or entry->sfn.fstClusLO
    
    ' NTFS 特有字段置空
    info->mftRecordNumber = 0
    info->LinkCount = 1
    info->bDeleted = False
    info->bHasAlternateStream = False
    info->bSuspicious = False
    info->bReparsePoint = False
    info->reparseTag = 0
    info->cAlternateFileName = SFN_ToString(@entry->sfn)
    
    Print "fileName:" & info->cFileName
End Sub

' ============================================================================
' 卷生命周期管理（与 NTFS 完全对称）
' ============================================================================
Sub CloseFAT32VolumeInternal(ctx As FAT32_CONTEXT Ptr)
    If ctx = NULL Then Exit Sub
    If ctx->fatCache <> NULL Then
        Deallocate ctx->fatCache
        ctx->fatCache = NULL
    End If
    If ctx->io.hVol <> INVALID_HANDLE_VALUE And ctx->io.hVol <> NULL Then
        CloseHandle(ctx->io.hVol)
        ctx->io.hVol = INVALID_HANDLE_VALUE
    End If
End Sub

Function OpenFAT32VolumeInternal(ctx As FAT32_CONTEXT Ptr, strDrive As String) As Boolean
    ctx->io.hVol = CreateFile("\\.\" & strDrive, GENERIC_READ Or GENERIC_WRITE, _
                          FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                          NULL, OPEN_EXISTING, 0, NULL)
    If ctx->io.hVol = INVALID_HANDLE_VALUE Then Return False
    
    ctx->io.VolName = strDrive
    ctx->io.partitionStartOffset = 0
    
    Dim bootBuf(0 To 511) As UByte
    If Not DiskIO_Read(@ctx->io, 0, @bootBuf(0), 512) Then Goto Cleanup
    If Not ParseBPB(ctx, @bootBuf(0)) Then Goto Cleanup
    If Not LoadFATCache(ctx) Then Goto Cleanup
    Return True
    
Cleanup:
    CloseHandle(ctx->io.hVol)
    ctx->io.hVol = INVALID_HANDLE_VALUE
    Return False
End Function

Function OpenFAT32VolumeInternalPhysical(ctx As FAT32_CONTEXT Ptr, _
                                         ByVal diskIndex As Integer, _
                                         ByVal startLBA As ULongLong, _
                                         ByVal bytesPerSec As ULong) As Boolean
    
    ctx->io.bytesPerSector = bytesPerSec
    ctx->io.partitionStartOffset = 0
    ctx->io.diskIndex = diskIndex
    
    ctx->io.VolName = "\\.\PhysicalDrive" & diskIndex
    
    Dim path As WString * 64 = "\\.\PhysicalDrive" & diskIndex
    ctx->io.hVol = CreateFile(@path, GENERIC_READ Or GENERIC_WRITE, _
                          FILE_SHARE_READ Or FILE_SHARE_WRITE, _
                          NULL, OPEN_EXISTING, 0, NULL)
    If ctx->io.hVol = INVALID_HANDLE_VALUE Then Return False
    
    Dim bootBuf(0 To 511) As UByte
    
    ' ===== 修复：使用 startLBA 计算正确的 Boot Sector 偏移 =====
    Dim partitionBaseOffset As ULongLong = startLBA * CULngInt(bytesPerSec)
    Print "[OpenFAT32VolumeInternalPhysical] partitionBaseOffset=" & partitionBaseOffset
    
    If Not DiskIO_Read(@ctx->io, partitionBaseOffset, @bootBuf(0), 512) Then 
        Print "[OpenFAT32VolumeInternalPhysical] DiskIO_Read failed at offset " & partitionBaseOffset
        Goto Cleanup
    End If
    
    If Not ParseBPB(ctx, @bootBuf(0)) Then 
        Print "[OpenFAT32VolumeInternalPhysical] ParseBPB failed"
        Goto Cleanup
    End If
    
    ' ===== 修复：调整所有基于扇区的偏移，加上分区起始偏移 =====
    ' 保存分区起始偏移到上下文中，供后续 ClusterToOffset 使用
    ' 由于 FAT32_CONTEXT 没有 startLBA 字段，我们需要修改 ClusterToOffset 或在这里调整
    
    ' 方案A：直接调整 fatStartOffset 和 dataStartSector 为绝对偏移
    ctx->fatStartOffset = ctx->fatStartOffset + partitionBaseOffset
    ctx->dataStartSector = ctx->dataStartSector + startLBA
    
    Print "[OpenFAT32VolumeInternalPhysical] Adjusted fatStartOffset=" & ctx->fatStartOffset
    Print "[OpenFAT32VolumeInternalPhysical] Adjusted dataStartSector=" & ctx->dataStartSector
    
    If Not LoadFATCache(ctx) Then 
        Print "[OpenFAT32VolumeInternalPhysical] LoadFATCache failed"
        Goto Cleanup
    End If
    
    Return True
    
Cleanup:
    If ctx->io.hVol <> INVALID_HANDLE_VALUE Then CloseHandle(ctx->io.hVol)
    ctx->io.hVol = INVALID_HANDLE_VALUE
    Return False
End Function

' 对外：物理磁盘 + LBA 打开（返回指针，调用方负责释放）
Function OpenFAT32VolumePhysical(ByVal diskIndex As Integer, _
                                        ByVal startLBA As ULongLong, _
                                        ByVal bytesPerSec As ULong) As FAT32_CONTEXT Ptr
    Dim ctx As FAT32_CONTEXT Ptr = Allocate(SizeOf(FAT32_CONTEXT))
    If ctx = NULL Then Return NULL
    memset(ctx, 0, SizeOf(FAT32_CONTEXT))
    
    If OpenFAT32VolumeInternalPhysical(ctx, diskIndex, startLBA, bytesPerSec) Then
        Return ctx
    End If
    Deallocate(ctx)
    Return NULL
End Function

' 盘符自动缓存管理（类似 OpenNTFSVolumeContext）
Function OpenFAT32VolumeContext(ByVal path As String) As FAT32_CONTEXT Ptr
    /'Dim idx As Integer = GetDriveIndex(path)
    If idx < 0 Then Return NULL
    
    If Not g_FAT32_Initialized Then
        memset(@g_FAT32_VolCtx(0), 0, SizeOf(g_FAT32_VolCtx))
        g_FAT32_Initialized = True
    End If
    
    If g_FAT32_VolCtx(idx) <> NULL Then Return g_FAT32_VolCtx(idx)'/
    
    Dim ctx As FAT32_CONTEXT Ptr = Allocate(SizeOf(FAT32_CONTEXT))
    If ctx = NULL Then Return NULL
    memset(ctx, 0, SizeOf(FAT32_CONTEXT))
    
    'Dim drive As String = Chr(Asc("A") + idx) & ":"
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
    Return OpenFAT32VolumePhysical(parsed.diskIndex, startLBA, bytesPerSec)
    
    'g_FAT32_VolCtx(idx) = ctx
    'Return ctx
End Function

Sub CloseFAT32VolumeContext(ctx As FAT32_CONTEXT Ptr)
    If ctx = NULL Then Exit Sub
    CloseFAT32VolumeInternal(ctx)
    Deallocate(ctx)
End Sub

/'Sub CleanupAllFAT32Volumes()
    For i As Integer = 0 To 25
        If g_FAT32_VolCtx(i) <> NULL Then
            CloseFAT32VolumeContext(Chr(Asc("A") + i) & ":")
        End If
    Next
End Sub'/

' ============================================================================
' 核心目录枚举（内部实现，路径格式与 NTFS 一致：X:\dir\subdir）
' ============================================================================
Function FAT32_GetDirList_Internal(ctx As FAT32_CONTEXT Ptr, _
                                           lpDirPath As String, _
                                           fileInfoList() As WIN32_FILE_DATAEX2W) As Long
    Dim parts() As String
    vbSplit lpDirPath, "\", parts()
    
    If ctx = NULL Then Return -1
    Print "[FAT32_GetDirList_Internal]lpDirPath:" & lpDirPath
    Dim currentCluster As ULong = ctx->rootCluster
    
    ' 逐级路径解析（跳过盘符 parts(0)）
    Dim i As Integer
    For i = 1 To UBound(parts)
        Dim partName As String = Trim(parts(i))
        If partName = "" Then Continue For
        
        Dim entries() As FILE_ENTRY_COLLECTED
        Dim entryCount As Integer = ReadDirectoryEntries(ctx, currentCluster, entries())
        If entryCount < 0 Then Return -1
        
        Dim found As Boolean = False
        Dim j As Integer
        For j = 0 To entryCount - 1
            Dim entryName As String = AssembleFileName(@entries(j))
            If UCase(entryName) = UCase(partName) Then
                If (entries(j).sfn.attr And ATTR_DIRECTORY) = 0 Then
                    If i < UBound(parts) Then Return -1
                End If
                Dim clusHigh As ULong = entries(j).sfn.fstClusHI
                Dim clusLow As ULong = entries(j).sfn.fstClusLO
                currentCluster = (clusHigh Shl 16) Or clusLow
                found = True
                Exit For
            End If
        Next
        If Not found Then Return -1
    Next
    
    ' 读取最终目录内容
    Dim entries() As FILE_ENTRY_COLLECTED
    Dim entryCount As Integer = ReadDirectoryEntries(ctx, currentCluster, entries())
    If entryCount < 0 Then Return -1
    
    Dim validCount As Integer = 0
    Dim j As Integer
    For j = 0 To entryCount - 1
        Dim fileName As String = AssembleFileName(@entries(j))
        If fileName = "." Or fileName = ".." Then Continue For
        validCount += 1
    Next
    
    If validCount > 0 Then
        ReDim fileInfoList(0 To validCount - 1)
        Dim idx As Integer = 0
        For j = 0 To entryCount - 1
            Dim fileName As String = AssembleFileName(@entries(j))
            If fileName = "." Or fileName = ".." Then Continue For
            DirEntryToFileInfo(ctx, @entries(j), @fileInfoList(idx))
            idx += 1
        Next
    End If
    
    Return validCount
End Function

' ============================================================================
' 辅助：写目录条目（处理驱动模式扇区对齐限制）
' ============================================================================
Function WriteDirEntryAt(ctx As FAT32_CONTEXT Ptr, _
                         dirCluster As ULong, _
                         entryOffset As ULong, _
                         entryBuf As UByte Ptr, _
                         entrySize As ULong) As Boolean
    
    Dim dirOffset As ULongLong = ClusterToOffset(ctx, dirCluster) + entryOffset
    Dim sectorSize As ULong = ctx->bytesPerSector
    
    ' 已对齐直接写
    If (dirOffset Mod sectorSize) = 0 And (entrySize Mod sectorSize) = 0 Then
        Dim result As Boolean = DiskIO_Write(@ctx->io, dirOffset, entryBuf, entrySize)
        If result AndAlso ctx->io.hVol <> INVALID_HANDLE_VALUE Then
            FlushFileBuffers(ctx->io.hVol)   ' ← 强制刷到磁盘
        End If
        Return result
    End If
    
    ' 非对齐：读-改-写整个扇区
    Dim sectorOffset As ULongLong = (dirOffset \ sectorSize) * sectorSize
    Dim withinSector As ULong = CULng(dirOffset - sectorOffset)
    Dim sectorBuf As UByte Ptr = Allocate(sectorSize)
    If sectorBuf = NULL Then Return False
    
    If Not DiskIO_Read(@ctx->io, sectorOffset, sectorBuf, sectorSize) Then
        Deallocate sectorBuf
        Return False
    End If
    
    memcpy(sectorBuf + withinSector, entryBuf, entrySize)
    
    Dim result As Boolean = DiskIO_Write(@ctx->io, sectorOffset, sectorBuf, sectorSize)
    
    ' ===== 关键：强制刷新 + 验证 =====
    If result AndAlso ctx->io.hVol <> INVALID_HANDLE_VALUE Then
        FlushFileBuffers(ctx->io.hVol)
        
        ' 重新读取整个扇区验证
        Dim verifyBuf As UByte Ptr = Allocate(sectorSize)
        If verifyBuf <> NULL Then
            If DiskIO_Read(@ctx->io, sectorOffset, verifyBuf, sectorSize) Then
                If verifyBuf[withinSector] <> entryBuf[0] Then
                    Print "[WriteDirEntryAt] 验证失败：写入未生效！"
                    result = False
                Else
                    Print "[WriteDirEntryAt] 验证通过：首字节=&h" & Hex(verifyBuf[withinSector], 2)
                End If
            End If
            Deallocate verifyBuf
        End If
    End If
    
    Deallocate sectorBuf
    Return result
End Function

' ============================================================================
' 辅助：设置 FAT 表项（分配/释放簇）
' ============================================================================
Function SetFATEntry(ctx As FAT32_CONTEXT Ptr, cluster As ULong, value As ULong) As Boolean
    If cluster < 2 Or cluster > ctx->maxCluster Then Return False
    
    Dim entryOffset As ULong = cluster * 4
    Dim fatOffset As ULongLong = ctx->fatStartOffset + entryOffset
    Dim newValue As ULong = value And &h0FFFFFFF
    
    If ctx->fatCache <> NULL Then
        Dim entry As ULong Ptr = CPtr(ULong Ptr, ctx->fatCache + entryOffset)
        *entry = (*entry And &hF0000000) Or newValue
        Return DiskIO_Write(@ctx->io, ctx->fatStartOffset, ctx->fatCache, ctx->fatCacheSize)
    Else
        Dim sectorSize As ULong = ctx->bytesPerSector
        Dim sectorOffset As ULongLong = (fatOffset \ sectorSize) * sectorSize
        Dim withinSector As ULong = CULng(fatOffset - sectorOffset)
        Dim sectorBuf As UByte Ptr = Allocate(sectorSize)
        If sectorBuf = NULL Then Return False
        
        If Not DiskIO_Read(@ctx->io, sectorOffset, sectorBuf, sectorSize) Then
            Deallocate sectorBuf
            Return False
        End If
        
        Dim entry As ULong Ptr = CPtr(ULong Ptr, sectorBuf + withinSector)
        *entry = (*entry And &hF0000000) Or newValue
        
        Dim result As Boolean = DiskIO_Write(@ctx->io, sectorOffset, sectorBuf, sectorSize)
        Deallocate sectorBuf
        Return result
    End If
End Function

' ============================================================================
' 辅助：释放整个簇链
' ============================================================================
Function FreeClusterChain(ctx As FAT32_CONTEXT Ptr, startCluster As ULong) As Boolean
    If startCluster < 2 Or startCluster > ctx->maxCluster Then Return False
    
    Dim currentCluster As ULong = startCluster
    Dim nextCluster As ULong
    Dim safety As ULong = 0
    
    Do
        nextCluster = GetFATEntry(ctx, currentCluster)
        If Not SetFATEntry(ctx, currentCluster, FAT32_FREE_CLUSTER) Then
            ' 尽量继续释放后续簇
        End If
        
        safety += 1
        If safety > ctx->totalClusters Then Exit Do  ' 防环
        
        If IsEOC(nextCluster) Then Exit Do
        If nextCluster < 2 Or nextCluster > ctx->maxCluster Then Exit Do
        currentCluster = nextCluster
    Loop
    
    Return True
End Function

' ============================================================================
' 内部删除实现（按父目录簇 + 文件名）
' ============================================================================
Function FAT32_DeleteFile_Internal(ctx As FAT32_CONTEXT Ptr, _
                                   parentCluster As ULong, _
                                   targetName As String) As Boolean
    
    targetName = UCase(Trim(targetName))
    If targetName = "" Or targetName = "." Or targetName = ".." Then Return False
    
    Dim clusterSize As ULong = CULng(ctx->sectorsPerCluster) * ctx->bytesPerSector
    Dim clusterBuf As UByte Ptr = Allocate(clusterSize)
    If clusterBuf = NULL Then Return False
    
    Dim currentCluster As ULong = parentCluster
    
    ' 跨簇收集 LFN（记录所在簇号和偏移，防止 LFN 与 SFN 跨簇）
    Dim lfnClusters(0 To 19) As ULong
    Dim lfnOffsets(0 To 19) As ULong
    Dim lfnEntries(0 To 19) As FAT32_DIR_ENTRY_LFN
    Dim lfnCount As Integer = 0
    
    Do While currentCluster >= 2 And currentCluster <= ctx->maxCluster
        If Not ReadCluster(ctx, currentCluster, clusterBuf) Then
            Print "ReadCluster失败"
            Deallocate clusterBuf
            Return False
        End If
        
        Dim offset As ULong = 0
        Do While offset + 32 <= clusterSize
            Dim entryPtr As UByte Ptr = clusterBuf + offset
            Dim firstByte As UByte = entryPtr[0]
            
            If firstByte = 0 Then
                Print "未找到"
                Deallocate clusterBuf
                Return False  ' 未找到
            End If
            
            If firstByte = &hE5 Then
                lfnCount = 0
                offset += 32
                Continue Do
            End If
            
            Dim attr As UByte = entryPtr[11]
            
            If attr = ATTR_LONG_NAME Then
                If lfnCount < 20 Then
                    lfnClusters(lfnCount) = currentCluster
                    lfnOffsets(lfnCount) = offset
                    memcpy(@lfnEntries(lfnCount), entryPtr, 32)
                    lfnCount += 1
                End If
            Else
                If attr = ATTR_VOLUME_ID Then
                    lfnCount = 0
                    offset += 32
                    Continue Do
                End If
                
                ' 组装 SFN
                Dim sfn As FAT32_DIR_ENTRY_SFN
                memcpy(@sfn, entryPtr, 32)
                
                Dim collected As FILE_ENTRY_COLLECTED
                memset(@collected, 0, SizeOf(FILE_ENTRY_COLLECTED))
                memcpy(@collected.sfn, @sfn, 32)
                collected.hasSFN = True
                
                If lfnCount > 0 Then
                    Dim calcChecksum As UByte = LFN_Checksum(@sfn.name(0))
                    Dim valid As Boolean = True
                    Dim c As Integer
                    For c = 0 To lfnCount - 1
                        If lfnEntries(c).checksum <> calcChecksum Then
                            Print "校验和不匹配"
                            valid = False
                            Exit For
                        End If
                    Next
                    If valid Then
                        For c = 0 To lfnCount - 1
                            memcpy(@collected.lfnParts(c), @lfnEntries(c), 32)
                        Next
                        collected.numLFN = lfnCount
                    End If
                End If
                
                Dim entryName As String = UCase(AssembleFileName(@collected))
                
                If entryName = targetName Then
                    Print "找到目标"
                    ' 找到目标，禁止删除目录
                    If (sfn.attr And ATTR_DIRECTORY) <> 0 Then
                        Print "禁止删除目录"
                        Deallocate clusterBuf
                        Return False
                    End If
                    
                    ' 1. 标记所有 LFN 条目为已删除
                    Print "LFN过程:"
                    Dim c As Integer
                    For c = 0 To lfnCount - 1
                        Dim delEntry(0 To 31) As UByte
                        memcpy(@delEntry(0), @lfnEntries(c), 32)
                        delEntry(0) = &hE5
                        If Not WriteDirEntryAt(ctx, lfnClusters(c), lfnOffsets(c), @delEntry(0), 32) Then
                            Print "修改LFN失败"
                            ' 继续尝试删除其余条目
                        End If
                    Next
                    
                    ' 2. 标记 SFN 条目为已删除
                    Print "SFN过程:"
                    Dim delSFN(0 To 31) As UByte
                    memcpy(@delSFN(0), @sfn, 32)
                    delSFN(0) = &hE5
                    If Not WriteDirEntryAt(ctx, currentCluster, offset, @delSFN(0), 32) Then
                        Print "修改SFN失败"
                        Deallocate clusterBuf
                        Return False
                    End If
                    
                    ' 标记 SFN 为删除后，立即验证
                    Dim sectorSize As ULong = ctx->bytesPerSector
                    Dim sectorOffset As ULongLong = (ClusterToOffset(ctx, currentCluster) + offset) \ sectorSize * sectorSize
                    Dim withinSector As ULong = CULng((ClusterToOffset(ctx, currentCluster) + offset) - sectorOffset)
                    Dim verifyBuf As UByte Ptr = Allocate(sectorSize)

                    If DiskIO_Read(@ctx->io, sectorOffset, verifyBuf, sectorSize) Then
                        Print "验证读取: 首字节=&h" & Hex(verifyBuf[withinSector], 2)
                        If verifyBuf[withinSector] <> &hE5 Then Print "错误：写入未生效！"
                    Else
                        Print "验证读取失败"
                    End If

                    Deallocate verifyBuf
                    
                    ' 3. 释放簇链（0字节文件 startCluster 可能为 0，跳过）
                    Dim startCluster As ULong = (CULng(sfn.fstClusHI) Shl 16) Or sfn.fstClusLO
                    If startCluster >= 2 And startCluster <= ctx->maxCluster Then
                        Print "释放簇链过程:"
                        FreeClusterChain(ctx, startCluster)
                    End If
                    
                    Deallocate clusterBuf
                    Return True
                End If
                
                lfnCount = 0
            End If
            
            offset += 32
        Loop
        
        currentCluster = GetFATEntry(ctx, currentCluster)
        If IsEOC(currentCluster) Then Exit Do
    Loop
    
    Deallocate clusterBuf
    Return False
End Function

' ============================================================================
' 辅助：从完整路径提取盘符和相对路径
' ============================================================================
Function ExtractDriveFromPath(fullPath As String, ByRef relPath As String) As String
    fullPath = Trim(fullPath)
    If Len(fullPath) < 2 Then Return ""
    
    ' 格式1: X:\dir\file.txt
    If Mid(fullPath, 2, 1) = ":" Then
        relPath = Mid(fullPath, 3)  ' 从 "\" 开始
        Return UCase(Left(fullPath, 2))
    End If
    
    ' 格式2: \\.\X: 或 \\.\PhysicalDriveN
    If Left(fullPath, 4) = "\\.\" Then
        Dim rest As String = Mid(fullPath, 5)
        If Len(rest) >= 2 And Mid(rest, 2, 1) = ":" Then
            relPath = "\"  ' 根目录
            Return UCase(Left(rest, 2))
        End If
    End If
    
    Return ""
End Function

' ============================================================================
' 对外接口：单路径参数删除文件
' ============================================================================
Function FAT32_DeleteFile2(ByVal fullPath As String) As Boolean
    fullPath = Trim(fullPath)
    If fullPath = "" Then Return False
    
    Dim relPath As String = ""
    Dim drive As String = ExtractDriveFromPath(fullPath, relPath)
    If drive = "" Then Return False
    
    If Left(relPath, 1) <> "\" Then relPath = "\" & relPath
    If Len(relPath) > 1 And Right(relPath, 1) = "\" Then
        relPath = Left(relPath, Len(relPath) - 1)
    End If
    If relPath = "" Or relPath = "\" Then Return False
    
    Dim ctx As FAT32_CONTEXT Ptr = OpenFAT32VolumeContext(fullPath)
    If ctx = NULL Then Print "ctx = NULL" : Return False
    
    Dim parentDir As String = ""
    Dim fileName As String = relPath
    Dim lastSlash As Integer = InStrRev(relPath, "\")
    If lastSlash > 1 Then
        parentDir = Mid(relPath, 2, lastSlash - 2)
        fileName = Mid(relPath, lastSlash + 1)
    ElseIf lastSlash = 1 Then
        parentDir = ""
        fileName = Mid(relPath, 2)
    End If
    
    Dim parentCluster As ULong
    If parentDir = "" Then
        parentCluster = ctx->rootCluster
    Else
        parentCluster = ResolvePathToCluster(ctx, parentDir)
    End If
    
    Dim result As Boolean = False
    If parentCluster <> 0 Then
        result = FAT32_DeleteFile_Internal(ctx, parentCluster, fileName)
    End If
    
    ' ===== 关键修复：强制刷新并关闭 =====
    If ctx->io.hVol <> INVALID_HANDLE_VALUE And ctx->io.hVol <> NULL Then
        FlushFileBuffers(ctx->io.hVol)   ' 强制把缓存刷到磁盘
    End If
    CloseFAT32VolumeContext(ctx)          ' 关闭句柄、释放缓存
    
    Return result
End Function

' ============================================================================
' 对外接口：物理磁盘方式删除文件
' ============================================================================
Function FAT32_DeleteFile2Physical(ByVal diskIndex As Integer, _
                                   ByVal startLBA As ULongLong, _
                                   ByVal bytesPerSec As ULong, _
                                   ByVal filePath As String) As Boolean
    filePath = Trim(filePath)
    If Left(filePath, 1) = "\" Then filePath = Mid(filePath, 2)
    If filePath = "" Then Return False
    
    Dim ctx As FAT32_CONTEXT Ptr = OpenFAT32VolumePhysical(diskIndex, startLBA, bytesPerSec)
    If ctx = NULL Then Return False
    
    Dim parentDir As String = ""
    Dim fileName As String = filePath
    Dim lastSlash As Integer = InStrRev(filePath, "\")
    If lastSlash > 0 Then
        parentDir = Left(filePath, lastSlash - 1)
        fileName = Mid(filePath, lastSlash + 1)
    End If
    
    Dim parentCluster As ULong
    If parentDir = "" Then
        parentCluster = ctx->rootCluster
    Else
        parentCluster = ResolvePathToCluster(ctx, parentDir)
    End If
    
    Dim result As Boolean = False
    If parentCluster <> 0 Then
        result = FAT32_DeleteFile_Internal(ctx, parentCluster, fileName)
    End If
    
    CloseFAT32VolumeInternal(ctx)
    Deallocate(ctx)
    Return result
End Function