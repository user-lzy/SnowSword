' 最终修复版：100%适配 ntfs!DriverEntry 反汇编特征
' 核心逻辑：先匹配 MOV 指令，再向前回溯对应 LEA 指令，彻底解决地址错配
' 修复：索引覆盖、无效偏移过滤、SIB字节解析、地址计算精度

Type RECOVERED_DISPATCH
    MajorFunction(0 To IRP_MJ_MAXIMUM_FUNCTION) As Any Ptr
    HitCount As ULong
    Confidence As ULong
End Type

' ==================== 地址转换核心函数 ====================
Function RvaToFoa(ByVal pFile As UByte Ptr, ByVal rva As ULong) As ULong
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pFile)
    If dos->e_magic <> &H5A4D Then Return 0
    
    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pFile + dos->e_lfanew)
    If nt64->Signature <> &H4550 Then Return 0
    
    Dim secHdr As IMAGE_SECTION_HEADER Ptr = Cast(IMAGE_SECTION_HEADER Ptr, _
        Cast(ULONG_PTR, nt64) + SizeOf(IMAGE_NT_HEADERS64))
    
    For i As ULong = 0 To nt64->FileHeader.NumberOfSections - 1
        If rva >= secHdr->VirtualAddress AndAlso rva < secHdr->VirtualAddress + secHdr->Misc.VirtualSize Then
            Return secHdr->PointerToRawData + (rva - secHdr->VirtualAddress)
        End If
        secHdr += 1
    Next
    Return 0
End Function

Function FoaToRva(ByVal pFile As UByte Ptr, ByVal foa As ULong) As ULong
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pFile)
    If dos->e_magic <> &H5A4D Then Return 0
    
    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pFile + dos->e_lfanew)
    If nt64->Signature <> &H4550 Then Return 0
    
    Dim secHdr As IMAGE_SECTION_HEADER Ptr = Cast(IMAGE_SECTION_HEADER Ptr, _
        Cast(ULONG_PTR, nt64) + SizeOf(IMAGE_NT_HEADERS64))
    
    For i As ULong = 0 To nt64->FileHeader.NumberOfSections - 1
        If foa >= secHdr->PointerToRawData AndAlso foa < secHdr->PointerToRawData + secHdr->SizeOfRawData Then
            Return secHdr->VirtualAddress + (foa - secHdr->PointerToRawData)
        End If
        secHdr += 1
    Next
    Return 0
End Function

Function GetDriverEntryRVA(ByVal pDump As UByte Ptr) As ULong
    If pDump = 0 Then Return 0
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return 0

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)
    If nt64->Signature <> &H4550 Or nt64->FileHeader.Machine <> &H8664 Then Return 0

    Return nt64->OptionalHeader.AddressOfEntryPoint
End Function

' ==================== 安全内存读取 ====================
Function GetByte(pDump As UByte Ptr, offset As ULong, fileSize As ULong) As Integer
    If pDump = 0 Or offset >= fileSize Then Return -1
    Return pDump[offset]
End Function

' ==================== 核心：从 MOV 指令向前回溯 LEA 指令 ====================
Function FindLeaForMov(pDump As UByte Ptr, movStartFoa As ULong, fileSize As ULong, ByRef outFuncRva As ULong) As Boolean
    Const MAX_BACKTRACE = 16 ' 最多向前回溯16字节（LEA和MOV紧挨着）
    outFuncRva = 0

    For i As Integer = 1 To MAX_BACKTRACE
        Dim leaFoa As Long = movStartFoa - i
        If leaFoa < 0 Or (leaFoa + 7) > fileSize Then Return False

        ' 精准匹配 LEA RAX, [RIP+imm32] 固定机器码 48 8D 05
        Dim b0 As Integer = GetByte(pDump, leaFoa, fileSize)
        Dim b1 As Integer = GetByte(pDump, leaFoa+1, fileSize)
        Dim b2 As Integer = GetByte(pDump, leaFoa+2, fileSize)
        
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then
            ' 计算目标函数 RVA
            Dim leaRva As ULong = FoaToRva(pDump, leaFoa)
            If leaRva = 0 Then Return False

            Dim nextRva As ULong = leaRva + 7 ' LEA 下一条指令的 RVA
            Dim imm32 As Long = 0
            CopyMemory(@imm32, pDump + leaFoa + 3, 4)
            outFuncRva = CULng(CLng(nextRva) + imm32)
            
            Return outFuncRva <> 0
        End If
    Next
    Return False
End Function

' ==================== 主恢复函数 ====================
Function RecoverDispatchFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal outResult As RECOVERED_DISPATCH Ptr) As Boolean

    Const DISPATCH_BASE = &H70
    Const DISPATCH_STEP = 8
    Const SCAN_RANGE = &H2000 ' 扫描 DriverEntry 前后 8KB

    ' 初始化结果
    outResult->HitCount = 0
    outResult->Confidence = 0
    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        outResult->MajorFunction(i) = 0
    Next

    ' PE 校验
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then
        Print "无效 PE 文件"
        Return False
    End If

    ' 获取 DriverEntry 地址
    Dim entryRva As ULong = GetDriverEntryRVA(pDump)
    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)
    If entryFoa = 0 Then
        Print "无法获取 DriverEntry 入口点"
        Return False
    End If

    Print "==================== PE 解析结果 ===================="
    Print "DriverEntry RVA: 0x"; Hex(entryRva)
    Print "DriverEntry FOA: 0x"; Hex(entryFoa)
    Print "文件大小: 0x"; Hex(fileSize)
    Print "扫描范围: 0x"; Hex(IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)); " ~ 0x"; Hex(entryFoa + SCAN_RANGE)
    Print

    ' 安全扫描范围
    Dim scanStart As ULong = IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE
    If scanEnd > fileSize Then scanEnd = fileSize

    Dim movTotal As ULong = 0
    Dim hitCount As ULong = 0

    ' 核心扫描：先找 MOV [reg+disp], RAX 指令
    Dim pc As ULong = scanStart
    While pc < scanEnd
        If pc + 8 > fileSize Then Exit While ' 最长指令不超过8字节

        ' 1. 匹配 REX.W 前缀（支持 0x48/0x49，对应 rax/r12 等寄存器）
        Dim rex As Integer = GetByte(pDump, pc, fileSize)
        If (rex And &H40) = 0 Or (rex And &H8) = 0 Then
            pc += 1
            Continue While
        End If

        ' 2. 匹配 MOV r/m64, r64 操作码 0x89
        Dim opc As Integer = GetByte(pDump, pc+1, fileSize)
        If opc <> &H89 Then
            pc += 1
            Continue While
        End If

        ' 3. 解析 ModRM 字节
        Dim modrm As Integer = GetByte(pDump, pc+2, fileSize)
        Dim srcReg As Integer = (modrm Shr 3) And 7
        Dim mode As Integer = (modrm Shr 6) And 3
        Dim rm As Integer = modrm And 7

        ' 源寄存器必须是 RAX（reg=0）
        If srcReg <> 0 Then
            pc += 1
            Continue While
        End If

        movTotal += 1
        Dim movStartFoa As ULong = pc
        Dim disp As Long = 0
        Dim instrLen As ULong = 3 ' REX + Opcode + ModRM
        Dim hasSib As Boolean = False

        ' 4. 解析 SIB 字节（rm=4 且 mode!=3 时必须有）
        If rm = 4 And mode <> 3 Then
            hasSib = True
            instrLen += 1
            If pc + instrLen > fileSize Then
                pc += 1
                Continue While
            End If
            ' SIB 字节无需解析，不影响偏移计算
        End If

        ' 5. 解析偏移量 disp（仅处理带偏移的模式）
        Dim dispValid As Boolean = False
        Select Case mode
            Case 1 ' [reg+disp8]
                If pc + instrLen + 1 > fileSize Then Exit Select
                Dim disp8 As Byte = CByte(GetByte(pDump, pc + instrLen, fileSize))
                disp = CShort(disp8) ' 符号扩展
                instrLen += 1
                dispValid = True
            Case 2 ' [reg+disp32]
                If pc + instrLen + 4 > fileSize Then Exit Select
                CopyMemory(@disp, pDump + pc + instrLen, 4)
                instrLen += 4
                dispValid = True
        End Select

        ' 6. 过滤无效偏移
        If dispValid = False Or disp < DISPATCH_BASE Then
            pc += instrLen
            Continue While
        End If

        ' 7. 计算 IRP_MJ 索引
        Dim offsetDelta As ULong = CULng(disp) - DISPATCH_BASE
        If offsetDelta Mod DISPATCH_STEP <> 0 Then
            pc += instrLen
            Continue While
        End If

        Dim irpIndex As ULong = offsetDelta \ DISPATCH_STEP
        If irpIndex > IRP_MJ_MAXIMUM_FUNCTION Then
            pc += instrLen
            Continue While
        End If

        ' 8. 向前回溯对应的 LEA 指令，获取函数地址
        Dim funcRva As ULong = 0
        If FindLeaForMov(pDump, movStartFoa, fileSize, funcRva) = False Then
            pc += instrLen
            Continue While
        End If

        ' 9. 赋值结果（直接覆盖，保证最新的正确地址）
        outResult->MajorFunction(irpIndex) = Cast(Any Ptr, Cast(ULONG_PTR, funcRva))
        hitCount += 1
        Print "[命中] IRP_MJ_"; irpIndex; " (0x"; Hex(disp); ") → RVA: 0x"; Hex(funcRva)

        pc += instrLen
    Wend

    ' 统计结果
    outResult->HitCount = hitCount
    outResult->Confidence = IIf(hitCount >= 20, 100, hitCount * 5)

    Print
    Print "==================== 扫描统计 ===================="
    Print "扫描到有效 MOV 指令数: "; movTotal
    Print "成功恢复派遣函数数: "; hitCount
    Print "置信度: "; outResult->Confidence; "%"
    Print

    Return hitCount > 0
End Function

' ==================== 测试入口 ====================
Public Sub Test()
    Dim bytDump() As Byte
    Dim filePath As String = "C:\Windows\System32\drivers\ntfs.sys"
    Dim imageBase As ULONG_PTR = &HFFFFF8035C6F0000 ' 从你的截图中获取的 ntfs.sys 加载基址

    ' 读取驱动文件
    On Error GoTo FileError
    Open filePath For Binary As #1
        ReDim bytDump(LOF(1)-1)
        Get #1, , bytDump()
    Close #1
    On Error GoTo 0

    Dim fileSize As ULong = UBound(bytDump) + 1
    Dim pDump As UByte Ptr = Cast(UByte Ptr, @bytDump(0))

    Dim rd As RECOVERED_DISPATCH
    If RecoverDispatchFromDump(pDump, fileSize, @rd) Then
        Print "==================== 最终恢复结果 ===================="
        Print "IRP_MJ 序号 | 函数 RVA | 虚拟地址(VA)"
        Print "-----------------------------------------"
        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            If rd.MajorFunction(i) <> 0 Then
                Dim va As ULONG_PTR = imageBase + CUInt(rd.MajorFunction(i))
                Print IRP_MJ_ToString(i); " | 0x"; Hex(rd.MajorFunction(i)); " | 0x"; Hex(va)
            Else
                Print "IRP_MJ_"; i; " | 未恢复（" & GetKernelProcAddress("ntoskrnl.exe", "IopInvalidDeviceRequest ") & "）"
            End If
        Next
    Else
        Print "恢复失败，请检查文件权限和PE格式"
    End If
    Exit Sub

FileError:
    Print "文件读取失败！请以管理员权限运行，或检查文件路径是否正确"
End Sub