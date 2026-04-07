Type REGF_HEADER 
    Signature      As ULong
    Seq1           As ULong
    Seq2           As ULong
    LastWriteTime  As LongInt
    Major          As ULong
    Minor          As ULong
    Type           As ULong
    Format         As ULong
    RootCellOffset As ULong
    HiveBinsSize   As ULong
End Type

Type HBIN_HEADER Field = 1
    Sig(3)      As UByte   ' "hbin"
    Offset      As ULong
    Size        As ULong
    Reserved1   As ULong
    Reserved2   As ULong
    TimeStamp   As LongInt
    Spare       As ULong
End Type

Type CELL_HEADER
    SIZE As Long
End Type

Type NK_FULL Field = 1
    Sig(1)                  As UByte
    Flags                   As UShort
    LastWriteTime           As ULongInt
    AccessBits              As ULong
    ParentOffset            As ULong
    SubKeyCount             As ULong
    VolatileSubKeyCount     As ULong
    SubKeyListOff           As ULong
    VolatileSubKeyListOff   As ULong
    ValueCount              As ULong
    ValueListOff            As ULong
    SecurityOff             As ULong
    ClassOff                As ULong
    LargestSubKeyNameLen    As ULong
    LargestSubKeyClassLen   As ULong
    LargestValueNameLen     As ULong
    LargestValueDataLen     As ULong
    WorkVar                 As ULong
    NameLen                 As UShort
    ClassNameLen            As UShort
End Type

Type VK_FULL
    Sig(1)      As UByte    ' "vk"
    NameLen     As UShort
    DataLen     As ULong
    DataOffset  As ULong
    DataType    As ULong
    Flags       As UShort
    Spare       As UShort
End Type

' 新增：Hive缓存结构，避免重复加载hive文件
Type HIVE_CACHE_ITEM
    HiveRootPath As String    ' 根路径，如 "HKLM\SOFTWARE"
    HiveData(Any) As UByte       ' hive原始二进制数据
    RootNKFileOffset As ULong ' 根NK节点的文件偏移
    IsValid As Boolean         ' 缓存是否有效
End Type

' 新增：全局hive缓存
Dim Shared g_HiveCache() As HIVE_CACHE_ITEM
Dim Shared g_HiveCacheCount As Integer = 0

' 新增：根键与hive文件映射结构
Type ROOT_HIVE_INFO
    RootName As String         ' 根键名称，如 "HKLM\SOFTWARE"
    HiveSubPath As String      ' hive内的根路径
    HiveFilePath As String     ' hive文件本地路径
    IsUserHive As Boolean      ' 是否为用户配置hive
End Type

Const HBIN_BASE As UInteger = &H1000
Const HBIN_HEADER_SIZE As ULong = &H20

Function CellPtrFromLogicOffset(hiveData() As UByte, logicOffset As ULong) As UByte Ptr
    Dim fileOffset As ULong = HBIN_BASE + logicOffset
    If fileOffset >= UBound(hiveData) Then
        Print "ERROR: offset out of hive size!"
        Return 0
    End If
    Return @hiveData(fileOffset)
End Function

Function CellDataPtrFromFileOffset(hiveData() As UByte, fileOffset As ULong) As UByte Ptr
    If fileOffset >= UBound(hiveData) + 1 Then
        Print "ERROR: CellDataPtrFromFileOffset - offset out of range: "; Hex(fileOffset)
        Return 0
    End If
    Return @hiveData(fileOffset + 4)
End Function

Function ScanForRootNK(hiveData() As UByte) As ULong
    Dim pos1 As ULong = &h1000
    While pos1 + SizeOf(HBIN_HEADER) < UBound(hiveData) + 1
        Dim h As HBIN_HEADER Ptr = Cast(HBIN_HEADER Ptr, @hiveData(pos1))
        If h->Sig(0) <> Asc("h") Or h->Sig(1) <> Asc("b") _
           Or h->Sig(2) <> Asc("i") Or h->Sig(3) <> Asc("n") Then
            Exit While
        End If
        Dim dataSize As ULong = h->Size - SizeOf(HBIN_HEADER)
        Dim cellPos As ULong = pos1 + SizeOf(HBIN_HEADER)
        Dim remain As ULong = dataSize
        While remain > 4
            Dim size As Long = *Cast(Long Ptr, @hiveData(cellPos))
            If size < 0 Then
                Dim sig0 As UByte = hiveData(cellPos + 4)
                Dim sig1 As UByte = hiveData(cellPos + 5)
                If sig0 = Asc("n") And sig1 = Asc("k") Then
                    Return cellPos
                End If
            End If
            If Abs(size) = 0 Then Exit While
            cellPos += Abs(size)
            remain -= Abs(size)
        Wend
        pos1 += h->size
    Wend
    Return 0
End Function

Function StringFromBytes(bytPtr As UByte Ptr, nameLen As UShort) As String
    Dim s As String = ""
    Dim charCount As Integer = nameLen
    For i As Integer = 0 To charCount - 1
        Dim lowByte As UByte = bytPtr[i*2]
        Dim highByte As UByte = bytPtr[i*2 + 1]
        Dim ch As UShort = lowByte + (highByte * 256)
        If ch >= 32 And ch <= 126 Then
            s &= Chr(ch)
        ElseIf ch = 0 Then
            Exit For
        ElseIf ch < 32 Then
            s = s & "[CTRL-" & Hex(ch, 2) & "]"
        Else
            s = s & "[U+" & Hex(ch, 4) & "]"
        End If
    Next
    Return s
End Function

Function GetNkName(hiveData() As UByte, nkFileOffset As ULong) As String
    Dim nkPtr As UByte Ptr = @hiveData(nkFileOffset + 4)
    If nkPtr[0] <> Asc("n") Or nkPtr[1] <> Asc("k") Then
        Return ""
    End If
    Dim nk As NK_FULL Ptr = Cast(NK_FULL Ptr, nkPtr)
    Dim nameLen As UShort = nk->NameLen
    If nameLen = 0 Then Return ""
    Dim namePtr As UByte Ptr = nkPtr + SizeOf(NK_FULL)
    If (nk->Flags And &H20) <> 0 Then
        Dim s As String = Space(nameLen)
        For i As Integer = 0 To nameLen - 1
            s[i] = namePtr[i]
        Next
        Return s
    Else
        Return StringFromBytes(namePtr, nameLen)
    End If
End Function

Function GetVkName(hiveData() As UByte, vk As VK_FULL Ptr) As String
    If vk->NameLen = 0 Then
        Return "(Default)"
    End If
    Dim namePtr As UByte Ptr = Cast(UByte Ptr, vk) + SizeOf(VK_FULL)
    Dim isAnsi As Boolean = (vk->Flags And &h1) <> 0
    If isAnsi Then
        Dim s As String = ""
        For i As Integer = 0 To vk->NameLen - 1
            s &= Chr(namePtr[i])
        Next
        Return s
    Else
        Dim wcharCount As Integer = vk->NameLen \ 2
        Dim s As String = ""
        For i As Integer = 0 To wcharCount - 1
            Dim ch As UShort = *Cast(UShort Ptr, namePtr + i * 2)
            If ch = 0 Then Exit For
            s &= WChr(ch)
        Next
        Return s
    End If
End Function
' 新增：从子键列表同时获取名称+逻辑偏移（核心支撑逐级查找）
Sub GetSubKeyNamesAndOffsets(hiveData() As UByte, listLogicOffset As ULong, names() As String, offsets() As ULong)
    Dim listp As UByte Ptr = CellPtrFromLogicOffset(hiveData(), listLogicOffset)
    If listp = 0 Then
        Erase names
        Erase offsets
        Exit Sub
    End If
    Dim listData As UByte Ptr = listp + 4
    Dim sig As UShort = *Cast(UShort Ptr, listData)
    Dim cnt As UShort = *Cast(UShort Ptr, listData + 2)
    
    Select Case sig
    Case &H6972  ' "ri" 嵌套列表
        Dim allNames() As String
        Dim allOffsets() As ULong
        Dim total As Integer = 0
        For i As Integer = 0 To cnt - 1
            Dim subListOff As ULong = *Cast(ULong Ptr, listData + 4 + i * 4)
            Dim subNames() As String
            Dim subOffsets() As ULong
            GetSubKeyNamesAndOffsets hiveData(), subListOff, subNames(), subOffsets()
            For j As Integer = 0 To UBound(subNames)
                total += 1
                If total = 1 Then
                    ReDim allNames(0 To 0)
                    ReDim allOffsets(0 To 0)
                    allNames(0) = subNames(j)
                    allOffsets(0) = subOffsets(j)
                Else
                    ReDim Preserve allNames(0 To total - 1)
                    ReDim Preserve allOffsets(0 To total - 1)
                    allNames(total - 1) = subNames(j)
                    allOffsets(total - 1) = subOffsets(j)
                End If
            Next
        Next
        If total > 0 Then
            ReDim names(0 To total - 1)
            ReDim offsets(0 To total - 1)
            For i As Integer = 0 To total - 1
                names(i) = allNames(i)
                offsets(i) = allOffsets(i)
            Next
        Else
            Erase names
            Erase offsets
        End If
    Case &H666C, &H686C  ' "lf"/"lh" 直接列表
        ReDim names(0 To cnt - 1)
        ReDim offsets(0 To cnt - 1)
        For i As Integer = 0 To cnt - 1
            offsets(i) = *Cast(ULong Ptr, listData + 4 + i * 8)
            names(i) = GetNkName(hiveData(), HBIN_BASE + offsets(i))
        Next
    Case Else
        Erase names
        Erase offsets
    End Select
End Sub

' 兼容原有函数，不破坏现有逻辑
Sub GetSubKeyNamesFromList(hiveData() As UByte, listLogicOffset As ULong, names() As String)
    Dim dummyOffsets() As ULong
    GetSubKeyNamesAndOffsets hiveData(), listLogicOffset, names(), dummyOffsets()
End Sub

' 新增：根据相对路径，从hive根逐级查找目标NK节点
Private Function FindNKByPath(hiveData() As UByte, rootNKFileOffset As ULong, relativePath As String) As ULong
    Dim currentNKOffset As ULong = rootNKFileOffset
    Dim pathParts() As String
    Dim partCount As Integer = 0
    
    ' 拆分相对路径
    Dim tempPath As String = relativePath
    If tempPath = "" Then Return currentNKOffset
    
    Do
        Dim pos1 As Integer = InStr(tempPath, "\")
        If pos1 = 0 Then
            If tempPath <> "" Then
                ReDim Preserve pathParts(0 To partCount)
                pathParts(partCount) = tempPath
                partCount += 1
            End If
            Exit Do
        End If
        ReDim Preserve pathParts(0 To partCount)
        pathParts(partCount) = Left(tempPath, pos1 - 1)
        partCount += 1
        tempPath = Mid(tempPath, pos1 + 1)
    Loop
    
    If partCount = 0 Then Return currentNKOffset
    
    ' 逐级查找子键
    For i As Integer = 0 To partCount - 1
        Dim targetName As String = UCase(pathParts(i))
        Dim nkp As UByte Ptr = CellDataPtrFromFileOffset(hiveData(), currentNKOffset)
        If nkp = 0 Then Return 0
        Dim nk As NK_FULL Ptr = Cast(NK_FULL Ptr, nkp)
        
        ' 验证NK签名
        If nk->Sig(0) <> Asc("n") Or nk->Sig(1) <> Asc("k") Then Return 0
        If nk->SubKeyCount = 0 Or nk->SubKeyListOff = &hFFFFFFFF Then Return 0
        
        ' 查找目标子键
        Dim subNames() As String
        Dim subOffsets() As ULong
        GetSubKeyNamesAndOffsets hiveData(), nk->SubKeyListOff, subNames(), subOffsets()
        
        Dim foundIndex As Integer = -1
        For j As Integer = 0 To UBound(subNames)
            If UCase(subNames(j)) = targetName Then
                foundIndex = j
                Exit For
            End If
        Next
        
        If foundIndex = -1 Then Return 0
        currentNKOffset = HBIN_BASE + subOffsets(foundIndex)
    Next
    
    Return currentNKOffset
End Function

' 新增：从NK节点获取当前键的完整信息（仅当前键，不递归）
Private Function GetKeyItemFromNK(hiveData() As UByte, nkFileOffset As ULong, fullKeyPath As String) As REG_KEY_ITEM
    Dim result As REG_KEY_ITEM
    Dim nkp As UByte Ptr = CellDataPtrFromFileOffset(hiveData(), nkFileOffset)
    If nkp = 0 Then Return result
    
    Dim nk As NK_FULL Ptr = Cast(NK_FULL Ptr, nkp)
    If nk->Sig(0) <> Asc("n") Or nk->Sig(1) <> Asc("k") Then Return result
    
    ' 填充基础信息
    result.KeyPath = fullKeyPath
    result.KeyPathUpper = PathToUpper(fullKeyPath)
    result.IsSymLink = (nk->Flags And &H10) <> 0
    
    ' 枚举值
    If nk->ValueCount > 0 And nk->ValueListOff <> &hFFFFFFFF Then
        Dim listp As UByte Ptr = CellPtrFromLogicOffset(hiveData(), nk->ValueListOff)
        If listp <> 0 Then
            Dim vkBase As UByte Ptr = listp + 4
            Dim valCount As Integer = 0
            ReDim result.Values(0 To nk->ValueCount - 1)
            For i As Integer = 0 To nk->ValueCount - 1
                Dim vkLogicOffset As Long = *Cast(Long Ptr, vkBase + i * 4)
                If vkLogicOffset < 0 Then Continue For
                Dim vkCellPtr As UByte Ptr = CellPtrFromLogicOffset(hiveData(), vkLogicOffset)
                If vkCellPtr = 0 Then Continue For
                Dim vk As VK_FULL Ptr = Cast(VK_FULL Ptr, vkCellPtr + 4)
                If *Cast(UShort Ptr, @vk->Sig(0)) <> &h6B76 Then Continue For
                result.Values(valCount).ValueName = GetVkName(hiveData(), vk)
                result.Values(valCount).ValueType = vk->DataType
                valCount += 1
            Next
            If valCount > 0 Then
                ReDim Preserve result.Values(0 To valCount - 1)
            Else
                Erase result.Values
            End If
        End If
    End If
    
    ' 枚举直接子键
    If nk->SubKeyCount > 0 And nk->SubKeyListOff <> &hFFFFFFFF Then
        Dim subNames() As String
        GetSubKeyNamesFromList hiveData(), nk->SubKeyListOff, subNames()
        If UBound(subNames) >= 0 Then
            ReDim result.SubKeys(UBound(subNames))
            For i As Integer = 0 To UBound(subNames)
                result.SubKeys(i) = subNames(i)
            Next
        End If
    End If
    
    Return result
End Function

Sub EnumHiveKey(hiveData() As UByte, nkFileOffset As ULong, parentPath As String, _
                hiveArr() As REG_KEY_ITEM, ByRef keyCount As Integer, _
                ByVal isRoot As Boolean = FALSE, ByVal isFromUserHive As Boolean = FALSE)

    Dim nkp As UByte Ptr = CellDataPtrFromFileOffset(hiveData(), nkFileOffset)
    If nkp = 0 Then Exit Sub

    Dim nk As NK_FULL Ptr = Cast(NK_FULL Ptr, nkp)
    Dim keyItem As REG_KEY_ITEM
    Dim keyName As String = GetNkName(hiveData(), nkFileOffset)

    ' 构建当前键的完整路径
    If isRoot Then
        keyItem.KeyPath = parentPath
    Else
        If parentPath = "" Then
            keyItem.KeyPath = keyName
        Else
            keyItem.KeyPath = parentPath & "\" & keyName
        End If
    End If
    
    ' 若 PathToUpper 函数已定义则使用，否则暂时注释
    ' keyItem.KeyPathUpper = PathToUpper(keyItem.KeyPath)
    keyItem.KeyPathUpper = UCase(keyItem.KeyPath)   ' 简单替代
    keyItem.IsFromHKCU = isFromUserHive

    ' ---- 枚举当前键的值 ----
    If nk->ValueCount > 0 And nk->ValueListOff <> &hFFFFFFFF Then
        Dim listp As UByte Ptr = CellPtrFromLogicOffset(hiveData(), nk->ValueListOff)
        If listp <> 0 Then
            Dim vkBase As UByte Ptr = listp + 4
            Dim valCount As Integer = 0
            ReDim keyItem.Values(0 To nk->ValueCount - 1)

            For i As Integer = 0 To nk->ValueCount - 1
                Dim vkLogicOffset As Long = *Cast(Long Ptr, vkBase + i * 4)
                If vkLogicOffset < 0 Then Continue For

                Dim vkCellPtr As UByte Ptr = CellPtrFromLogicOffset(hiveData(), vkLogicOffset)
                If vkCellPtr = 0 Then Continue For

                Dim vk As VK_FULL Ptr = Cast(VK_FULL Ptr, vkCellPtr + 4)
                If *Cast(UShort Ptr, @vk->Sig(0)) <> &h6B76 Then Continue For

                keyItem.Values(valCount).ValueName = GetVkName(hiveData(), vk)
                keyItem.Values(valCount).ValueType = vk->DataType
                valCount += 1
            Next

            If valCount > 0 Then
                ReDim Preserve keyItem.Values(0 To valCount - 1)
            Else
                Erase keyItem.Values
            End If
        End If
    End If

    ' ---- 收集直接子键名称 ----
    If nk->SubKeyCount > 0 And nk->SubKeyListOff <> &hFFFFFFFF Then
        Dim subNames() As String
        GetSubKeyNamesFromList hiveData(), nk->SubKeyListOff, subNames()
        If UBound(subNames) >= 0 Then
            ReDim keyItem.SubKeys(UBound(subNames))
            For i As Integer = 0 To UBound(subNames)
                keyItem.SubKeys(i) = subNames(i)
            Next
        End If
    End If

    ' ---- 符号链接标记 ----
    If (nk->Flags And &H10) <> 0 Then
        keyItem.IsSymLink = True
        ' 可扩展解析目标路径（例如读取 "(Default)" 值）
    End If

    ' ---- 将当前键加入结果数组 ----
    keyCount += 1
    If UBound(hiveArr) < keyCount - 1 Then
        If keyCount = 1 Then
            ReDim hiveArr(0 To 0)
        Else
            ReDim Preserve hiveArr(0 To keyCount - 1)
        End If
    End If
    hiveArr(keyCount - 1) = keyItem
End Sub

' 新增：根据注册表路径获取对应hive信息
Private Function GetHiveInfoFromPath(fullPath As String, ByRef outHiveInfo As ROOT_HIVE_INFO) As Boolean
    Dim pathUpper As String = PathToUpper(fullPath)
    Dim currentUserSid As String = GetCurrentUserSid()
    Dim systemRoot As String = Environ("SystemRoot") & "\System32\config\"
    Dim userProfile As String = "C:\Users\" & Environ("USERNAME") & "\"
    
    outHiveInfo = Type<ROOT_HIVE_INFO>()
    
    ' HKLM 系列根键
    If Left(pathUpper, 18) = "HKEY_LOCAL_MACHINE" Or Left(pathUpper, 4) = "HKLM" Then
        Dim subPath As String = Mid(pathUpper, InStr(pathUpper, "\") + 1)
        Dim firstSubKey As String = Left(subPath, InStr(subPath, "\") - 1)
        If firstSubKey = "" Then firstSubKey = subPath
        
        Select Case firstSubKey
            Case "SAM"
                outHiveInfo.RootName = "HKLM\SAM"
                outHiveInfo.HiveFilePath = systemRoot & "SAM"
                Return True
            Case "SECURITY"
                outHiveInfo.RootName = "HKLM\SECURITY"
                outHiveInfo.HiveFilePath = systemRoot & "SECURITY"
                Return True
            Case "SOFTWARE"
                outHiveInfo.RootName = "HKLM\SOFTWARE"
                outHiveInfo.HiveFilePath = systemRoot & "SOFTWARE"
                Return True
            Case "SYSTEM"
                outHiveInfo.RootName = "HKLM\SYSTEM"
                outHiveInfo.HiveFilePath = systemRoot & "SYSTEM"
                Return True
            Case "HARDWARE"
                Return False ' 动态生成，无对应hive文件
        End Select
    ' HKCU 根键
    ElseIf Left(pathUpper, 17) = "HKEY_CURRENT_USER" Or Left(pathUpper, 4) = "HKCU" Then
        If currentUserSid = "" Then Return False
        outHiveInfo.RootName = "HKU\" & currentUserSid
        outHiveInfo.HiveFilePath = userProfile & "NTUSER.DAT"
        outHiveInfo.IsUserHive = TRUE
        Return True
    ' HKU 根键
    ElseIf Left(pathUpper, 10) = "HKEY_USERS" Or Left(pathUpper, 3) = "HKU" Then
        Dim subPath As String = Mid(pathUpper, InStr(pathUpper, "\") + 1)
        Dim firstSubKey As String = Left(subPath, InStr(subPath, "\") - 1)
        If firstSubKey = "" Then firstSubKey = subPath
        
        If firstSubKey = ".DEFAULT" Then
            outHiveInfo.RootName = "HKU\.DEFAULT"
            outHiveInfo.HiveFilePath = systemRoot & "DEFAULT"
            Return True
        ElseIf firstSubKey = currentUserSid Then
            outHiveInfo.RootName = "HKU\" & currentUserSid
            outHiveInfo.HiveFilePath = userProfile & "NTUSER.DAT"
            outHiveInfo.IsUserHive = TRUE
            Return True
        End If
    ' HKCR 根键
    ElseIf Left(pathUpper, 17) = "HKEY_CLASSES_ROOT" Or Left(pathUpper, 4) = "HKCR" Then
        If currentUserSid <> "" Then
            outHiveInfo.RootName = "HKU\" & currentUserSid & "\SOFTWARE\Classes"
            outHiveInfo.HiveFilePath = userProfile & "NTUSER.DAT"
            outHiveInfo.IsUserHive = TRUE
            Return True
        Else
            outHiveInfo.RootName = "HKLM\SOFTWARE\Classes"
            outHiveInfo.HiveFilePath = systemRoot & "SOFTWARE"
            Return True
        End If
    ' HKCC 根键
    ElseIf Left(pathUpper, 20) = "HKEY_CURRENT_CONFIG" Or Left(pathUpper, 4) = "HKCC" Then
        outHiveInfo.RootName = "HKLM\SYSTEM"
        outHiveInfo.HiveFilePath = systemRoot & "SYSTEM"
        Return True
    End If
    
    Return False
End Function

' 新增：按需加载hive，优先从缓存获取
Function LoadHive(ByRef hiveInfo As ROOT_HIVE_INFO, ByRef outCacheItem As HIVE_CACHE_ITEM) As Boolean
    ' 优先查询缓存
    Dim hiveRootUpper As String = PathToUpper(hiveInfo.RootName)
    For i As Integer = 0 To g_HiveCacheCount - 1
        If PathToUpper(g_HiveCache(i).HiveRootPath) = hiveRootUpper Then
            If g_HiveCache(i).IsValid Then
                outCacheItem = g_HiveCache(i)
                Return True
            End If
        End If
    Next
    
    ' 缓存未命中，加载hive文件
    Dim hiveData() As UByte
    If Not ReadFile2(hiveInfo.HiveFilePath, hiveData()) Then
        Print "加载hive失败: " & hiveInfo.HiveFilePath
        Return False
    End If
    
    ' 验证REGF签名
    If *Cast(ULong Ptr, @hiveData(0)) <> &H66676572 Then
        Print "无效的REGF文件: " & hiveInfo.HiveFilePath
        Return False
    End If
    
    ' 查找根NK节点
    Dim rootNK As ULong = ScanForRootNK(hiveData())
    If rootNK = 0 Then
        Print "找不到根NK节点: " & hiveInfo.HiveFilePath
        Return False
    End If
    
    ' 存入缓存
    g_HiveCacheCount += 1
    ReDim Preserve g_HiveCache(0 To g_HiveCacheCount - 1)
    g_HiveCache(g_HiveCacheCount - 1).HiveRootPath = hiveInfo.RootName
    ' 为目标数组分配内存（必须！）
    Dim dataSize As Integer = UBound(hiveData) + 1
    ReDim (g_HiveCache(g_HiveCacheCount - 1).hiveData)(dataSize - 1)
    
    ' 深拷贝数据
    memcpy @g_HiveCache(g_HiveCacheCount - 1).hiveData(0), @hiveData(0), dataSize * SizeOf(UByte)
    g_HiveCache(g_HiveCacheCount - 1).RootNKFileOffset = rootNK
    g_HiveCache(g_HiveCacheCount - 1).IsValid = True
    
    outCacheItem = g_HiveCache(g_HiveCacheCount - 1)
    Return True
End Function