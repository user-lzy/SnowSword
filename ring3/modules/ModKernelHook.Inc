' 最终修复版：100%适配 ntfs!DriverEntry 反汇编特征
' 核心逻辑：先匹配 MOV 指令，再向前回溯对应 LEA 指令，彻底解决地址错配
' 修复：索引覆盖、无效偏移过滤、SIB字节解析、地址计算精度

' ==================== FastIO 结果结构体 ====================
' ==================== 核心配置 ====================
' Windows 官方 FAST_IO_DISPATCH 总函数数量(x64)
#define FAST_IO_MAX_COUNT 27
' 每个函数指针大小(x64)
Const FAST_IO_PTR_SIZE = 8

' ==================== FastIO 连续存储结构体 ====================
' 内存布局：纯连续指针数组，与内核 FAST_IO_DISPATCH 完全一致
Type FAST_IO_DISPATCH_ORIGINAL
    Functions(0 To FAST_IO_MAX_COUNT - 1) As Any Ptr  ' 按初始化顺序连续存储
    HitCount      As ULong
    Confidence    As ULong
End Type

' ==================== FastIO 索引 → 名称映射表（唯一需要的语义表） ====================
Function FastIoIndexToName(ByVal index As ULong) As String
    ' 顺序 = 你提供的 FAST_IO_DISPATCH 结构体顺序（无任何自定义/错位）
    Dim NameTable(0 To FAST_IO_MAX_COUNT - 1) As String = { _
        "FastIoCheckIfPossible", _        ' 0
        "FastIoRead", _                   ' 1
        "FastIoWrite", _                  ' 2
        "FastIoQueryBasicInfo", _         ' 3
        "FastIoQueryStandardInfo", _      ' 4  修正：原名错写为 FastIoQueryStdInfo
        "FastIoLock", _                   ' 5
        "FastIoUnlockSingle", _           ' 6
        "FastIoUnlockAll", _              ' 7
        "FastIoUnlockAllByKey", _         ' 8
        "FastIoDeviceControl", _          ' 9  补全：官方缺失的核心函数
        "AcquireFileForNtCreateSection", _' 10
        "ReleaseFileForNtCreateSection", _ ' 11
        "FastIoDetachDevice", _           ' 12
        "FastIoQueryNetworkOpenInfo", _   ' 13
        "AcquireForModWrite", _           ' 14 修正：官方标准名称
        "MdlRead", _                      ' 15
        "MdlReadComplete", _              ' 16
        "PrepareMdlWrite", _              ' 17
        "MdlWriteComplete", _             ' 18
        "FastIoReadCompressed", _         ' 19
        "FastIoWriteCompressed", _        ' 20
        "MdlReadCompleteCompressed", _    ' 21
        "MdlWriteCompleteCompressed", _   ' 22
        "FastIoQueryOpen", _              ' 23
        "ReleaseForModWrite", _           ' 24 修正：官方标准名称
        "AcquireForCcFlush", _            ' 25
        "ReleaseForCcFlush" _             ' 26
    }
    
    If index > FAST_IO_MAX_COUNT - 1 Then Return "Unknown"
    Return NameTable(index)
End Function

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
    Const MAX_BACKTRACE = 64     ' ← 从 16 改为 64，覆盖批量 MOV 序列
    outFuncRva = 0

    For i As Integer = 1 To MAX_BACKTRACE
        Dim leaFoa As Long = movStartFoa - i
        If leaFoa < 0 Or (leaFoa + 7) > fileSize Then Return False

        Dim b0 As Integer = GetByte(pDump, leaFoa, fileSize)
        Dim b1 As Integer = GetByte(pDump, leaFoa+1, fileSize)
        Dim b2 As Integer = GetByte(pDump, leaFoa+2, fileSize)
        
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then
            Dim leaRva As ULong = FoaToRva(pDump, leaFoa)
            If leaRva = 0 Then Return False

            Dim nextRva As ULong = leaRva + 7
            Dim imm32 As Long = 0
            CopyMemory(@imm32, pDump + leaFoa + 3, 4)
            outFuncRva = CULng(CLng(nextRva) + imm32)
            
            Return outFuncRva <> 0
        End If
    Next
    Return False
End Function

' ==================== 状态机版 RecoverDispatchFromDump ====================
' 解析 DRIVER_OBJECT.MajorFunction[] 的初始化序列
' 匹配模式：LEA RAX, [func] 后紧跟一至多条 MOV [r12+idx*8+0x70], RAX

Function RecoverDispatchFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal outResult As RECOVERED_DISPATCH Ptr) As Boolean

    Const DISPATCH_BASE = &H70
    Const DISPATCH_STEP = 8
    Const SCAN_RANGE = &H2000

    ' 初始化
    outResult->HitCount = 0
    outResult->Confidence = 0
    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        outResult->MajorFunction(i) = 0
    Next

    ' PE 校验
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False

    Dim entryRva As ULong = GetDriverEntryRVA(pDump)
    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)
    If entryFoa = 0 Then Return False

    Print "DriverEntry RVA: 0x" & Hex(entryRva)
    Print "DriverEntry FOA: 0x" & Hex(entryFoa)

    Dim scanStart As ULong = IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE
    If scanEnd > fileSize Then scanEnd = fileSize

    Dim pc As ULong = scanStart
    Dim currentFuncRva As ULong = 0    ' 当前未决的函数 RVA（来自上一条 LEA）
    Dim inProlog As Boolean = False    ' 是否处于 LEA 后的匹配状态
    Dim hitCount As ULong = 0

    While pc < scanEnd
        If pc + 8 > fileSize Then Exit While

        ' 读取指令首字节
        Dim b0 As Integer = GetByte(pDump, pc, fileSize)
        Dim b1 As Integer = GetByte(pDump, pc+1, fileSize)
        Dim b2 As Integer = GetByte(pDump, pc+2, fileSize)

        ' ─── 优先检测 LEA RAX, [RIP+imm32] (48 8D 05) ───
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then
            ' 计算该 LEA 指向的函数 RVA
            Dim leaRva As ULong = FoaToRva(pDump, pc)
            If leaRva <> 0 Then
                Dim nextRva As ULong = leaRva + 7
                Dim imm32 As Long = 0
                CopyMemory(@imm32, pDump + pc + 3, 4)
                currentFuncRva = CULng(CLng(nextRva) + imm32)
                inProlog = True ' 进入待命状态
                Print "[LEA] 函数 RVA: 0x" & Hex(currentFuncRva)
            End If
            pc += 7 ' 跳过 LEA 指令（固定 7 字节）
            Continue While
        End If

        ' ─── 若处于待命状态，尝试匹配 MOV [r12+disp], RAX ───
        If inProlog Then
            ' 匹配 REX.W (0x48/0x49) + MOV r/m64, r64 (0x89)
            If (b0 = &H48 Or b0 = &H49) And b1 = &H89 Then
                Dim modrm As Integer = b2
                Dim srcReg As Integer = (modrm Shr 3) And 7
                Dim mode As Integer = (modrm Shr 6) And 3
                Dim rm As Integer = modrm And 7
                
                If srcReg = 0 Then ' 源是 RAX
                    Dim instrLen As ULong = 3 ' REX + Opcode + ModRM
                    Dim disp As Long = 0
                    Dim dispValid As Boolean = False

                    ' 解析 SIB
                    If rm = 4 And mode <> 3 Then instrLen += 1

                    ' 解析 disp
                    Select Case mode
                        Case 1 ' disp8
                            If pc + instrLen + 1 <= fileSize Then
                                disp = CByte(GetByte(pDump, pc + instrLen, fileSize))
                                instrLen += 1
                                dispValid = True
                            End If
                        Case 2 ' disp32
                            If pc + instrLen + 4 <= fileSize Then
                                CopyMemory(@disp, pDump + pc + instrLen, 4)
                                instrLen += 4
                                dispValid = True
                            End If
                    End Select

                    If dispValid AndAlso disp >= DISPATCH_BASE Then
                        Dim offsetDelta As ULong = CULng(disp) - DISPATCH_BASE
                        If (offsetDelta Mod DISPATCH_STEP) = 0 Then
                            Dim irpIndex As ULong = offsetDelta \ DISPATCH_STEP
                            If irpIndex <= IRP_MJ_MAXIMUM_FUNCTION Then
                                ' 有效的派遣槽位，记录
                                outResult->MajorFunction(irpIndex) = Cast(Any Ptr, Cast(ULONG_PTR, currentFuncRva))
                                hitCount += 1
                                Print "[命中] IRP_MJ_" & irpIndex & " (0x" & Hex(disp) & ") → RVA: 0x" & Hex(currentFuncRva)
                                pc += instrLen
                                Continue While
                            End If
                        End If
                    End If
                End If
            End If
            ' 若是其他指令，则退出待命状态（当前 LEA 的序列结束）
            inProlog = False
            currentFuncRva = 0
        End If

        pc += 1 ' 逐字节推进（仅在非匹配指令时）
    Wend

    outResult->HitCount = hitCount
    outResult->Confidence = IIf(hitCount >= 20, 100, hitCount * 5)

    Print "成功恢复派遣函数数: " & hitCount
    Return hitCount > 0
End Function

Function RecoverFastIoFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal ImageBase As ULONG_PTR, _
    ByVal outFastIo As FAST_IO_DISPATCH_ORIGINAL Ptr) As Boolean

    Const SCAN_RANGE = &H2000
    Const FAST_IO_DISPATCH_OFFSET_DEVICE = &H50
    Const FAST_IO_HEADER_SIZE = 8

    outFastIo->HitCount = 0
    outFastIo->Confidence = 0
    For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
        outFastIo->Functions(i) = 0
    Next

    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False

    Dim entryRva As ULong = GetDriverEntryRVA(pDump)
    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)
    If entryFoa = 0 Then Return False

    Print "==================== RecoverFastIoFromDump 开始 ===================="

    Dim scanStart As ULong = IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE
    If scanEnd > fileSize Then scanEnd = fileSize

    Dim pc As ULong = scanStart
    Dim FastIoDispatchBaseRva As ULong = 0
    Dim foundBaseAddr As Boolean = False

    ' 第一阶段：定位 FastIoDispatch 全局变量（Size 字段地址）
    While pc < scanEnd AndAlso foundBaseAddr = False
        If pc + 15 > fileSize Then Exit While

        Dim rex As Integer = GetByte(pDump, pc, fileSize)
        Dim opc As Integer = GetByte(pDump, pc+1, fileSize)

        If (rex And &H40) <> 0 AndAlso opc = &H89 Then
            Dim modrm As Integer = GetByte(pDump, pc+2, fileSize)
            Dim mode As Integer = (modrm Shr 6) And 3
            Dim rm As Integer = modrm And 7
            Dim srcReg As Integer = (modrm Shr 3) And 7

            If srcReg = 0 Then
                Dim instrLen As ULong = 3
                Dim disp As Long = 0
                Dim dispValid As Boolean = False

                If rm = 4 And mode <> 3 Then instrLen += 1

                Select Case mode
                    Case 1
                        If pc + instrLen + 1 > fileSize Then Exit Select
                        disp = CByte(GetByte(pDump, pc + instrLen, fileSize))
                        instrLen += 1
                        dispValid = True
                    Case 2
                        If pc + instrLen + 4 > fileSize Then Exit Select
                        CopyMemory(@disp, pDump + pc + instrLen, 4)
                        instrLen += 4
                        dispValid = True
                End Select

                If dispValid AndAlso disp = FAST_IO_DISPATCH_OFFSET_DEVICE Then
                    Dim backtrackPc As Long = pc - 10
                    If backtrackPc < 0 Then backtrackPc = 0

                    While backtrackPc < pc
                        Dim b0 As Integer = GetByte(pDump, backtrackPc, fileSize)
                        Dim b1 As Integer = GetByte(pDump, backtrackPc+1, fileSize)
                        Dim b2 As Integer = GetByte(pDump, backtrackPc+2, fileSize)

                        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then
                            Dim leaRva As ULong = FoaToRva(pDump, backtrackPc)
                            If leaRva <> 0 Then
                                Dim nextRva As ULong = leaRva + 7
                                Dim imm32 As Long = 0
                                CopyMemory(@imm32, pDump + backtrackPc + 3, 4)
                                FastIoDispatchBaseRva = CULng(CLng(nextRva) + imm32)
                                foundBaseAddr = True
                                pc = pc + instrLen
                                Print "[基址] FastIoDispatch Size 字段 RVA: 0x" & Hex(FastIoDispatchBaseRva)
                                Exit While
                            End If
                        End If
                        backtrackPc += 1
                    Wend
                End If
            End If
        End If

        If foundBaseAddr = False Then pc += 1
    Wend

    If foundBaseAddr = False Then
        Print "[错误] 无法定位 FastIoDispatch 基址"
        Return False
    End If

    FastIoDispatchBaseRva += FAST_IO_HEADER_SIZE
    Print "[修正] 第一个函数指针基址 RVA: 0x" & Hex(FastIoDispatchBaseRva)

    ' 第二阶段：扫描所有 FastIo 槽位，自动解析 IAT
    Dim hitCount As ULong = 0
    Dim scanEndPhase2 As ULong = pc + &H500
    If scanEndPhase2 > fileSize Then scanEndPhase2 = fileSize

    While pc < scanEndPhase2 AndAlso hitCount < FAST_IO_MAX_COUNT
        If pc + 15 > fileSize Then Exit While

        Dim b0 As Integer = GetByte(pDump, pc, fileSize)
        Dim b1 As Integer = GetByte(pDump, pc+1, fileSize)
        Dim b2 As Integer = GetByte(pDump, pc+2, fileSize)

        Dim isLea As Boolean = (b0 = &H48 And b1 = &H8D And b2 = &H05)
        Dim isMovLoad As Boolean = (b0 = &H48 And b1 = &H8B And b2 = &H05)

        If isLea Or isMovLoad Then
            Dim loadFoa As ULong = pc
            Dim loadRva As ULong = FoaToRva(pDump, loadFoa)
            If loadRva = 0 Then
                pc += 1
                Continue While
            End If

            Dim nextRvaLoad As ULong = loadRva + 7
            Dim imm32Load As Long = 0
            CopyMemory(@imm32Load, pDump + loadFoa + 3, 4)
            Dim srcRva As ULong = CULng(CLng(nextRvaLoad) + imm32Load)

            Dim movPc As ULong = pc + 7
            If movPc + 7 > fileSize Then
                pc += 1
                Continue While
            End If

            Dim rexMov As Integer = GetByte(pDump, movPc, fileSize)
            Dim opcMov As Integer = GetByte(pDump, movPc+1, fileSize)
            Dim modrmMov As Integer = GetByte(pDump, movPc+2, fileSize)

            If rexMov = &H48 And opcMov = &H89 And modrmMov = &H05 Then
                Dim movRva As ULong = FoaToRva(pDump, movPc)
                Dim nextRvaMov As ULong = movRva + 7
                Dim imm32Mov As Long = 0
                CopyMemory(@imm32Mov, pDump + movPc + 3, 4)
                Dim targetMemRva As ULong = CULng(CLng(nextRvaMov) + imm32Mov)

                Dim delta As Long = CLng(targetMemRva) - CLng(FastIoDispatchBaseRva)

                If delta >= 0 And (delta Mod 8) = 0 Then
                    Dim index As ULong = CULng(delta) \ 8
                    If index < FAST_IO_MAX_COUNT Then
                        ' 计算最终地址
                        Dim finalAddr As ULONG_PTR = 0
                        If isMovLoad Then
                            ' 导入函数：从 IAT 解引用
                            Dim iatAddr As ULONG_PTR = ImageBase + srcRva
                            If (Not ReadKernelMemory(iatAddr, 8, @finalAddr)) OrElse finalAddr = 0 Then
                                Print "[警告] IAT 条目为空，使用 RVA"
                                finalAddr = ImageBase + srcRva
                            End If
                            Print "[导入函数] 索引 " & index & " " & FastIoIndexToName(index) & _
                                  " -> VA: 0x" & Hex(finalAddr)
                        Else
                            ' 内部函数：直接计算 VA
                            finalAddr = ImageBase + srcRva
                            Print "[内部函数] 索引 " & index & " " & FastIoIndexToName(index) & _
                                  " -> VA: 0x" & Hex(finalAddr)
                        End If

                        outFastIo->Functions(index) = Cast(Any Ptr, finalAddr)
                        hitCount += 1
                        pc = movPc + 7
                        Continue While
                    End If
                End If
            End If
        End If

        pc += 1
    Wend

    outFastIo->HitCount = hitCount
    outFastIo->Confidence = IIf(hitCount >= 20, 100, hitCount * 5)

    Print "==================== FastIO 扫描统计 ===================="
    Print "成功恢复 FastIO 函数数: " & hitCount
    Print "置信度: " & outFastIo->Confidence & "%"
    Print

    Return hitCount > 0
End Function

' ==================== 测试入口 ====================
Function GetOriginalDispatchFunctionAddr(DriverPath As StringW, OriginalDriverInfo As DRIVER_INFO) As Boolean
    Dim bytDump() As Byte
    Dim filePath As String = DriverPath
    Dim fileName As String = GetNameByPath(filePath)
    Dim imageSize As ULong
    Print "fileName:" & fileName
    Dim imageBase As ULONG_PTR = Cast(ULONG_PTR, GetKernelModuleBase(fileName, imageSize)) ' &HFFFFF8035C6F0000 ' 从你的截图中获取的 ntfs.sys 加载基址

    ' 读取驱动文件
    On Error GoTo FileError
    Open filePath For Binary As #1
        ReDim bytDump(LOF(1)-1)
        Get #1, , bytDump()
    Close #1
    On Error GoTo 0

    Dim fileSize As ULong = UBound(bytDump) + 1
    Dim pDump As UByte Ptr = Cast(UByte Ptr, @bytDump(0))
    'Print "DriverPath:" & DriverPath
    Dim rd As RECOVERED_DISPATCH
    Dim fastIo As FAST_IO_DISPATCH_ORIGINAL ' 新增
    If RecoverDispatchFromDump(pDump, fileSize, @rd) Then
        Print "==================== 最终恢复结果 ===================="
        Print "IRP_MJ 序号 | 函数 RVA | 虚拟地址(VA)"
        Print "-----------------------------------------"
        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            If rd.MajorFunction(i) <> 0 Then
                Dim va As ULONG_PTR = imageBase + CUInt(rd.MajorFunction(i))
                Print IRP_MJ_ToString(i)  & " | 0x" & Hex(rd.MajorFunction(i))  & " | 0x" & Hex(va)
                OriginalDriverInfo.MajorFunctionAddr(i) = va
            Else
                Dim IopInvalidDeviceRequestAddr As UInteger = NULL
                SetLastError 0
                If IoControl(hDrv, IOCTL_GetIopInvalidDeviceRequest, NULL, 0, @IopInvalidDeviceRequestAddr, SizeOf(IopInvalidDeviceRequestAddr)) = 0 Then
                    Print "[GetNtfsOriginalDispatchFunctionAddr]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
                    OriginalDriverInfo.MajorFunctionAddr(i) = NULL
                Else
                    OriginalDriverInfo.MajorFunctionAddr(i) = IopInvalidDeviceRequestAddr
                End If
                Print IRP_MJ_ToString(i) & " | 未恢复（" & IopInvalidDeviceRequestAddr & "）"
            End If
        Next
    Else
        Print "恢复Dispatch失败，请检查文件权限和PE格式"
        Return False
    End If
    
    If RecoverFastIoFromDump(pDump, fileSize, imageBase, @fastIo) Then ' 新增
        ' MyLog.PrintLog LOG_INFO,,, 
        Print "==================== 最终恢复结果 ===================="
        Print "FastIO | 函数 RVA | 虚拟地址(VA)"
        Print "-----------------------------------------"
        For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
            If fastIo.Functions(i) <> 0 Then
                Dim va As ULONG_PTR = CUInt(fastIo.Functions(i)) ' imageBase + CUInt(fastIo.Functions(i))
                Print FastIoIndexToName(i)  & " | 0x" & Hex(fastIo.Functions(i))  & " | 0x" & Hex(va)
                OriginalDriverInfo.FastIOFunctionAddr(i) = va
            Else
                /'Dim IopInvalidDeviceRequestAddr As UInteger = NULL
                SetLastError 0
                If IoControl(hDrv, IOCTL_GetIopInvalidDeviceRequest, NULL, 0, @IopInvalidDeviceRequestAddr, SizeOf(IopInvalidDeviceRequestAddr)) = 0 Then
                    Print "[GetNtfsOriginalDispatchFunctionAddr]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
                    OriginalDriverInfo.MajorFunctionAddr(i) = NULL
                Else
                    OriginalDriverInfo.MajorFunctionAddr(i) = IopInvalidDeviceRequestAddr
                End If'/
                Print FastIoIndexToName(i) & " | 未恢复（0）"
            End If
        Next
    Else
        Print "恢复FastIO失败，请检查文件权限和PE格式"
        Return False
    End If
    Return True

FileError:
    Print "文件读取失败！请以管理员权限运行，或检查文件路径是否正确"
    Return True
End Function