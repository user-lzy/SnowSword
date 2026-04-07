' 恢复的派遣函数表结构
Type RECOVERED_DISPATCH
    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As Any Ptr
    HitCount As ULong
    Confidence As ULong
End Type

' 指令解析时用到的寄存器枚举
Private Enum X64_REG
    REG_INVALID = -1
    REG_RAX = 0
    REG_RCX = 1
    REG_RDX = 2
    REG_RBX = 3
    REG_RSP = 4
    REG_RBP = 5
    REG_RSI = 6
    REG_RDI = 7
End Enum

' 写入事件记录
Private Type WRITE_EVENT
    BaseReg As X64_REG
    Offset As ULong
    Target As Any Ptr
End Type

Function GetDriverEntryRVA(ByVal pDump As UByte Ptr) As ULong
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return 0   ' 不是有效的 DOS 头

    Dim ntHeaders As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)
    If ntHeaders->Signature <> &H4550 Then Return 0   ' 不是有效的 NT 头

    ' 检查是否为 64 位 PE
    If ntHeaders->FileHeader.Machine <> &H8664 Then Return 0   ' 不是 AMD64 映像

    ' 返回入口点 RVA
    Return ntHeaders->OptionalHeader.AddressOfEntryPoint
End Function

' ==================== 内存Dump专用辅助函数 ====================
' 直接读取内存字节（无任何转换）
Function GetByte(pDump As UByte Ptr, offset As ULong) As Integer
    If pDump = 0 Then Return -1
    Return pDump[offset]
End Function

' 打印指定范围的所有字节（调试核心）
Sub PrintAllBytes(pDump As UByte Ptr, startOff As ULong, endOff As ULong, ImageSize As ULong)
    ' 强制修正范围：保证start ≤ end ≤ ImageSize
    Dim realStart As ULong = IIf(startOff < 1, 1, startOff)
    Dim realEnd As ULong = IIf(endOff > ImageSize, ImageSize - 1, endOff)
    
    If realStart >= realEnd Then
        Print "⚠️  扫描范围无效！realStart(" & realStart & ") ≥ realEnd(" & realEnd & ")"
        Print "   镜像大小: " & ImageSize & "，请检查DriverEntryRVA是否正确"
        Return
    End If

    Print "📝 逐字节扫描结果（有效RVA范围: 0x" & Hex(realStart) & " ~ 0x" & Hex(realEnd) & "）:"
    For i As ULong = realStart To realEnd
        Dim b As Integer = GetByte(pDump, i)
        If b < 0 Then Exit For
        ' 每16字节换行，便于查看指令字节
        If (i - realStart) Mod 16 = 0 Then Print vbTab & "0x" & Hex(i, 8) & ": ";
        Print Hex(b, 2) & " ";
        If (i - realStart) Mod 16 = 15 Then Print ""
    Next
    Print vbCrLf
End Sub

' 回溯找LEA RAX, [RIP+imm32]
Function FindLEA(pDump As UByte Ptr, movOff As ULong, ImageSize As ULong) As ULongInt
    For i As Integer = 0 To 10
        Dim leaOff As ULong = movOff - i
        If leaOff < 1 Or leaOff >= ImageSize Then Return 0
        
        Dim b0 As Integer = GetByte(pDump, leaOff)
        Dim b1 As Integer = GetByte(pDump, leaOff + 1)
        Dim b2 As Integer = GetByte(pDump, leaOff + 2)
        
        ' 匹配 LEA RAX, [RIP+imm32] → 48 8D 05
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then
            Dim ripOff As Long = *Cast(Long Ptr, pDump + leaOff + 3)
            Dim targetOff As ULongInt = (leaOff + 7) + ripOff
            Print "  ← LEA指令: RVA=0x" & Hex(leaOff) & " → 函数RVA=0x" & Hex(targetOff)
            Return targetOff
        End If
    Next
    Return 0
End Function

' ==================== 主函数（移除硬编码+动态扫描） ====================
Function RecoverDispatchFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal ImageSize As ULong, _
    ByVal ImageBase As ULongInt, _
    ByVal DriverEntryVA As ULongInt, _
    ByVal outResult As RECOVERED_DISPATCH Ptr) As Boolean

    Const DISPATCH_BASE As ULong = &H70   ' DriverObject->MajorFunction 起始偏移
    Const DISPATCH_STEP As ULong = 8      ' 每个函数指针8字节
    Const SCAN_EXTEND As ULong = &H2000   ' DriverEntry前后扩展扫描范围

    ' 初始化输出
    outResult->HitCount = 0
    outResult->Confidence = 0
    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        outResult->MajorFunction(i) = 0
    Next

    ' 校验内存镜像
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then
        Print "❌ 无效的内存Dump（不是PE文件）"
        Return False
    End If

    ' 核心：VA转RVA（动态计算，无硬编码！）
    Dim DriverEntryRVA As ULong = DriverEntryVA - ImageBase
    Print "✅ 内存Dump模式（VA转RVA）"
    Print "   ImageBase: 0x" & Hex(ImageBase)
    Print "   DriverEntry VA: 0x" & Hex(DriverEntryVA) & " → RVA: 0x" & Hex(DriverEntryRVA)
    
    ' 动态计算扫描范围：DriverEntryRVA ± SCAN_EXTEND（保证范围有效）
    Dim SCAN_START As ULong = IIf(DriverEntryRVA > SCAN_EXTEND, DriverEntryRVA - SCAN_EXTEND, 1)
    Dim SCAN_END As ULong = DriverEntryRVA + SCAN_EXTEND
    If SCAN_END > ImageSize Then SCAN_END = ImageSize
    Print "   动态扫描范围 RVA: 0x" & Hex(SCAN_START) & " ~ 0x" & Hex(SCAN_END)
    Print "========================================"

    ' 第一步：打印扫描范围内的所有字节（调试必看）
    PrintAllBytes(pDump, SCAN_START, SCAN_END, ImageSize)

    ' 第二步：扫描MOV [RSI+offset], RAX指令
    Dim pcRVA As ULong = SCAN_START
    While pcRVA < SCAN_END
        Dim b0 As Integer = GetByte(pDump, pcRVA)
        If b0 < 0 Then Exit While

        ' 匹配MOV指令开头：48 89（MOV r/m64, RAX）
        If b0 = &H48 Then
            Dim b1 As Integer = GetByte(pDump, pcRVA + 1)
            If b1 = &H89 Then
                Dim b2 As Integer = GetByte(pDump, pcRVA + 2)
                Dim modrm As Integer = b2
                
                ' 解析MODRM：reg=RAX(0)，rm=RSI(6)
                Dim reg As Integer = (modrm Shr 3) And 7
                Dim rm As Integer = modrm And 7
                Dim mode As Integer = (modrm Shr 6) And 3

                ' 仅处理RAX→RSI的情况（DriverObject存在RSI中）
                If reg = 0 And rm = 6 Then
                    Print "📌 找到目标指令 MOV [RSI+offset], RAX"
                    Print "   RVA: 0x" & Hex(pcRVA) & "  字节: " & Hex(b0,2) & " " & Hex(b1,2) & " " & Hex(b2,2)

                    ' 读取偏移值（8位/32位）
                    Dim offsetVal As ULong = 0
                    Select Case mode
                        Case 1: ' 8位无符号偏移
                            offsetVal = GetByte(pDump, pcRVA + 3)
                            Print "   偏移: 0x" & Hex(offsetVal,2) & " (8位)"
                        Case 2: ' 32位偏移
                            offsetVal = *Cast(ULong Ptr, pDump + pcRVA + 3)
                            Print "   偏移: 0x" & Hex(offsetVal,8) & " (32位)"
                        Case Else:
                            Print "   ❌ 无偏移，跳过"
                            pcRVA += 1
                            Continue While
                    End Select

                    ' 校验MajorFunction偏移（70h开始，8字节步长）
                    If offsetVal >= DISPATCH_BASE Then
                        Dim index As ULong = (offsetVal - DISPATCH_BASE) \ DISPATCH_STEP
                        If index <= IRP_MJ_MAXIMUM_FUNCTION Then
                            ' 回溯找对应的LEA指令
                            Dim funcRVA As ULongInt = FindLEA(pDump, pcRVA, ImageSize)
                            If funcRVA > 0 Then
                                outResult->MajorFunction(index) = Cast(PVOID, funcRVA)
                                outResult->HitCount += 1
                                outResult->Confidence = 100
                                Print "   ✅ MajorFunction[" & index & "] = 0x" & Hex(funcRVA)
                                Print "   📊 累计命中: " & outResult->HitCount
                            Else
                                Print "   ❌ 未找到对应的LEA指令"
                            End If
                        Else
                            Print "   ❌ 索引超出范围: " & index & " (最大27)"
                        End If
                    Else
                        Print "   ❌ 偏移小于0x70: 0x" & Hex(offsetVal)
                    End If

                    ' 跳过当前指令（避免重复扫描）
                    pcRVA += IIf(mode=1, 4, IIf(mode=2, 7, 3))
                    Continue While
                End If
            End If
        End If

        pcRVA += 1
    Wend

    ' 结果汇总
    Print "========================================"
    If outResult->HitCount >= 15 Then
        Print "🎉 恢复成功！总命中数: " & outResult->HitCount
        ' 打印最终结果
        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            If outResult->MajorFunction(i) <> 0 Then
                Print "   IRP_MJ_" & i & " → 0x" & Hex(ImageBase + outResult->MajorFunction(i))
            End If
        Next
        Return True
    Else
        Print "❌ 恢复失败，仅命中: " & outResult->HitCount
        Return False
    End If
End Function