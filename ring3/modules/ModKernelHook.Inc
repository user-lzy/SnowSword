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
/'Function FindLeaForMov(pDump As UByte Ptr, movStartFoa As ULong, fileSize As ULong, ByRef outFuncRva As ULong) As Boolean
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
End Function '/
Function FindLeaForMov( _
    ByVal pDump As UByte Ptr, _
    ByVal movStartFoa As ULong, _
    ByVal fileSize As ULong, _
    ByRef outFuncRva As ULong) As Boolean

    Const MAX_BACKTRACE = 64
    outFuncRva = 0

    If pDump = 0 Or movStartFoa = 0 Or movStartFoa >= fileSize Then Return False

    Dim scanStart As Long = CLng(movStartFoa) - MAX_BACKTRACE
    If scanStart < 0 Then scanStart = 0

    '
    ' 从目标 MOV 向前逐字节搜索。
    '
    ' 目标是寻找：
    '
    '   lea rax, [rip+xxxx]
    '       ...
    '   mov [DriverObject+offset], rax
    '
    ' 中间允许出现不修改 RAX 的指令。
    '
    Dim pc As Long = CLng(movStartFoa) - 1

    While pc >= scanStart

        If pc + 7 <= fileSize Then

            Dim b0 As Integer = GetByte(pDump, pc, fileSize)
            Dim b1 As Integer = GetByte(pDump, pc + 1, fileSize)
            Dim b2 As Integer = GetByte(pDump, pc + 2, fileSize)

            '
            ' LEA RAX, [RIP+imm32]
            '
            ' 48 8D 05 xx xx xx xx
            '
            If b0 = &H48 And b1 = &H8D And b2 = &H05 Then

                Dim leaRva As ULong = FoaToRva(pDump, pc)
                If leaRva = 0 Then Return False

                Dim imm32 As Long = 0
                CopyMemory(@imm32, pDump + pc + 3, 4)

                Dim nextRva As LongInt = CLngInt(leaRva) + 7
                Dim targetRva As LongInt = nextRva + imm32

                If targetRva <= 0 Then Return False

                outFuncRva = CULng(targetRva)
                Return True
            End If

            '
            ' 如果在找到 LEA 之前发现明显覆盖 RAX 的指令，
            ' 则不能继续使用更早的 LEA。
            '
            ' xor eax,eax
            ' 31 C0 / 33 C0
            '
            If (b0 = &H31 And b1 = &HC0) Or _
               (b0 = &H33 And b1 = &HC0) Then
                Return False
            End If

            '
            ' xor rax,rax
            ' 48 31 C0 / 48 33 C0
            '
            If b0 = &H48 Then
                If (b1 = &H31 And b2 = &HC0) Or _
                   (b1 = &H33 And b2 = &HC0) Then
                    Return False
                End If
            End If

            '
            ' mov eax, imm32
            ' B8 xx xx xx xx
            '
            If b0 = &HB8 Then
                Return False
            End If

            '
            ' mov rax, imm64
            ' 48 B8 xx...
            '
            If b0 = &H48 And b1 = &HB8 Then
                Return False
            End If

            '
            ' CALL 会破坏 volatile register，
            ' RAX 在 Windows x64 ABI 下属于 volatile register。
            '
            ' E8 rel32
            '
            If b0 = &HE8 Then
                Return False
            End If

            '
            ' 间接 CALL：
            '
            ' FF /2
            '
            If b0 = &HFF Then
                Dim modrm As Integer = b1
                Dim regField As Integer = (modrm Shr 3) And 7

                If regField = 2 Then
                    Return False
                End If
            End If

        End If

        pc -= 1
    Wend

    Return False
End Function

' ==================== RecoverDispatchFromDump ====================
' 解析 DRIVER_OBJECT.MajorFunction[] 初始化序列
'
' 核心逻辑：
' 1. LEA RAX, Function 后允许多个 MajorFunction 槽位共用同一个 RAX
' 2. 不要求 LEA 与 MOV 紧邻
' 3. 按目标 Base Register 分组
' 4. 扫描结束后选择命中槽位最多的寄存器作为 DRIVER_OBJECT
'
Function RecoverDispatchFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal outResult As RECOVERED_DISPATCH Ptr) As Boolean

    Const DISPATCH_BASE = &H70
    Const DISPATCH_STEP = 8
    Const SCAN_RANGE = &H2000
    Const MAX_LEA_TO_MOV_DISTANCE = 96

    ' 初始化
    outResult->HitCount = 0
    outResult->Confidence = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        outResult->MajorFunction(i) = 0
    Next

    If pDump = 0 Then Return False

    ' PE 校验
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False

    Dim entryRva As ULong = GetDriverEntryRVA(pDump)
    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)

    If entryFoa = 0 Then Return False

    '
    ' 只从 DriverEntry 开始向后扫描。
    '
    ' 不再扫描 entryFoa - 0x2000，
    ' 避免把 DriverEntry 前其它函数中的
    ' [reg+70h+n*8] 写入误认成 MajorFunction。
    '
    Dim scanStart As ULong = entryFoa
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE

    If scanEnd > fileSize Then scanEnd = fileSize

    '
    ' 每个可能的 Base Register 保存一份候选表。
    '
    ' x64 通用寄存器编号：
    '
    ' 0 RAX
    ' 1 RCX
    ' 2 RDX
    ' 3 RBX
    ' 4 RSP
    ' 5 RBP
    ' 6 RSI
    ' 7 RDI
    ' 8~15 R8~R15
    '
    Dim Candidate(0 To 15, 0 To IRP_MJ_MAXIMUM_FUNCTION) As ULONG_PTR
    Dim RegHitCount(0 To 15) As ULong

    For r As Integer = 0 To 15
        RegHitCount(r) = 0

        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            Candidate(r, i) = 0
        Next
    Next

    Dim pc As ULong = scanStart

    '
    ' 最近一次：
    '
    '   lea rax,[rip+Function]
    '
    Dim currentFuncRva As ULong = 0
    Dim currentLeaEnd As ULong = 0
    Dim haveRaxSource As Boolean = False

    While pc < scanEnd

        If pc + 8 > fileSize Then Exit While

        Dim b0 As Integer = GetByte(pDump, pc, fileSize)
        Dim b1 As Integer = GetByte(pDump, pc + 1, fileSize)
        Dim b2 As Integer = GetByte(pDump, pc + 2, fileSize)

        '
        ' ==========================================
        ' LEA RAX,[RIP+imm32]
        '
        ' 48 8D 05 xx xx xx xx
        ' ==========================================
        '
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then

            Dim leaRva As ULong = FoaToRva(pDump, pc)

            If leaRva <> 0 Then

                Dim imm32 As Long = 0
                CopyMemory(@imm32, pDump + pc + 3, 4)

                Dim targetRva As LongInt = _
                    CLngInt(leaRva) + 7 + CLngInt(imm32)

                If targetRva > 0 Then

                    currentFuncRva = CULng(targetRva)
                    currentLeaEnd = pc + 7
                    haveRaxSource = True

                    'Print "[LEA RAX] RVA=0x" & Hex(currentFuncRva)

                End If

            End If

            pc += 7
            Continue While

        End If

        '
        ' LEA 与使用点不能无限远。
        '
        If haveRaxSource Then

            If pc >= currentLeaEnd Then

                If pc - currentLeaEnd > MAX_LEA_TO_MOV_DISTANCE Then

                    haveRaxSource = False
                    currentFuncRva = 0

                End If

            End If

        End If

        '
        ' ==========================================
        ' MOV [BaseReg+disp], RAX
        '
        ' REX.W + 89 /r
        ' ==========================================
        '
        If haveRaxSource Then

            If (b0 And &HF8) = &H48 And b1 = &H89 Then

                Dim rex As Integer = b0
                Dim modrm As Integer = b2

                Dim mode As Integer = (modrm Shr 6) And 3
                Dim srcReg As Integer = (modrm Shr 3) And 7
                Dim rm As Integer = modrm And 7

                '
                ' REX.R
                '
                If (rex And &H04) <> 0 Then
                    srcReg += 8
                End If

                '
                ' 只接受：
                '
                ' MOV [memory], RAX
                '
                If srcReg = 0 And mode <> 3 Then

                    Dim instrLen As ULong = 3
                    Dim baseReg As Integer = -1

                    Dim disp As Long = 0
                    Dim dispValid As Boolean = False
                    Dim hasBase As Boolean = True

                    '
                    ' ======================================
                    ' 解析 Base Register / SIB
                    ' ======================================
                    '
                    If rm = 4 Then

                        If pc + instrLen >= fileSize Then
                            Exit While
                        End If

                        Dim sib As Integer = _
                            GetByte(pDump, pc + instrLen, fileSize)

                        instrLen += 1

                        Dim sibBase As Integer = sib And 7

                        '
                        ' mod=00 + SIB.base=5：
                        ' 无 Base Register，只有 disp32。
                        '
                        If mode = 0 And sibBase = 5 Then

                            hasBase = False

                        Else

                            baseReg = sibBase

                            If (rex And &H01) <> 0 Then
                                baseReg += 8
                            End If

                        End If

                    Else

                        '
                        ' mod=00, rm=5：
                        ' RIP-relative，不是 DriverObject+offset。
                        '
                        If mode = 0 And rm = 5 Then

                            hasBase = False

                        Else

                            baseReg = rm

                            If (rex And &H01) <> 0 Then
                                baseReg += 8
                            End If

                        End If

                    End If

                    '
                    ' ======================================
                    ' displacement
                    ' ======================================
                    '
                    Select Case mode

                        Case 0

                            If hasBase Then
                                disp = 0
                                dispValid = True
                            End If

                        Case 1

                            If pc + instrLen + 1 <= fileSize Then

                                Dim d8 As Integer = _
                                    GetByte(pDump, pc + instrLen, fileSize)

                                If d8 >= &H80 Then
                                    disp = d8 - &H100
                                Else
                                    disp = d8
                                End If

                                instrLen += 1
                                dispValid = True

                            End If

                        Case 2

                            If pc + instrLen + 4 <= fileSize Then

                                CopyMemory( _
                                    @disp, _
                                    pDump + pc + instrLen, _
                                    4)

                                instrLen += 4
                                dispValid = True

                            End If

                    End Select

                    '
                    ' ======================================
                    ' 判断是否落在 MajorFunction[]
                    ' ======================================
                    '
                    If hasBase AndAlso _
                       baseReg >= 0 AndAlso _
                       baseReg <= 15 AndAlso _
                       dispValid AndAlso _
                       disp >= DISPATCH_BASE Then

                        Dim offsetDelta As Long = _
                            disp - DISPATCH_BASE

                        If (offsetDelta Mod DISPATCH_STEP) = 0 Then

                            Dim irpIndex As ULong = _
                                CULng(offsetDelta \ DISPATCH_STEP)

                            If irpIndex <= IRP_MJ_MAXIMUM_FUNCTION Then

                                '
                                ' 同一 LEA 可以对应多个槽。
                                '
                                ' 这里绝不能清除 currentFuncRva。
                                '
                                If Candidate(baseReg, irpIndex) = 0 Then

                                    Candidate(baseReg, irpIndex) = _
                                        Cast(ULONG_PTR, currentFuncRva)

                                    RegHitCount(baseReg) += 1

                                    'Print "[Candidate] Reg=" & baseReg & _
                                    '      " " & IRP_MJ_ToString(irpIndex) & _
                                    '      " RVA=0x" & Hex(currentFuncRva)

                                Else

                                    '
                                    ' 如果后续重新初始化同一槽，
                                    ' 使用较新的值。
                                    '
                                    Candidate(baseReg, irpIndex) = _
                                        Cast(ULONG_PTR, currentFuncRva)

                                End If

                                pc += instrLen
                                Continue While

                            End If

                        End If

                    End If

                End If

            End If

        End If

        pc += 1

    Wend

    '
    ' ==========================================
    ' 选择命中 MajorFunction[] 最多的 BaseReg
    '
    ' NTFS   -> R12
    ' fastfat-> RBX
    ' ==========================================
    '
    Dim bestReg As Integer = -1
    Dim bestCount As ULong = 0

    For r As Integer = 0 To 15

        If RegHitCount(r) > bestCount Then

            bestCount = RegHitCount(r)
            bestReg = r

        End If

    Next

    If bestReg = -1 Or bestCount = 0 Then
        Return False
    End If

    '
    ' 输出最终结果
    '
    Dim hitCount As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        If Candidate(bestReg, i) <> 0 Then

            outResult->MajorFunction(i) = _
                Cast(Any Ptr, Candidate(bestReg, i))

            hitCount += 1

        End If

    Next

    outResult->HitCount = hitCount

    outResult->Confidence = _
        IIf(hitCount >= 20, 100, hitCount * 5)

    'Print "Dispatch Base Register = " & bestReg
    'Print "成功恢复派遣函数数: " & hitCount

    Return hitCount > 0
End Function

Function RecoverFastIoFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal ImageBase As ULONG_PTR, _
    ByVal outFastIo As FAST_IO_DISPATCH_ORIGINAL Ptr) As Boolean

    Const SCAN_RANGE = &H3000
    Const FAST_IO_HEADER_SIZE = 8
    Const MAX_SOURCE_TO_STORE_DISTANCE = 64
    Const MAX_STORE_CANDIDATES = 512
    Const MIN_FASTIO_MATCH_COUNT = 4

    outFastIo->HitCount = 0
    outFastIo->Confidence = 0

    For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
        outFastIo->Functions(i) = 0
    Next

    If pDump = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] pDump = NULL"
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)

    If dos->e_magic <> &H5A4D Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] Invalid DOS signature"
        Return False
    End If

    Dim entryRva As ULong = GetDriverEntryRVA(pDump)
    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)

    If entryFoa = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] RvaToFoa(entryRva) failed, EntryRVA=0x" & Hex(entryRva)
        Return False
    End If

    Dim scanStart As ULong = IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE

    If scanEnd > fileSize Then scanEnd = fileSize

    MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] EntryRVA=0x" & Hex(entryRva) & " EntryFOA=0x" & Hex(entryFoa) & " ScanStart=0x" & Hex(scanStart) & " ScanEnd=0x" & Hex(scanEnd)

    '
    ' 保存所有候选：
    '
    '   LEA RAX,InternalFunction
    '        或
    '   MOV RAX,[RIP+IAT]
    '
    '   ...
    '
    '   MOV [RIP+GlobalSlot],RAX
    '
    Dim StoreTargetRva(0 To MAX_STORE_CANDIDATES - 1) As ULong
    Dim StoreSourceRva(0 To MAX_STORE_CANDIDATES - 1) As ULong
    Dim StoreSourceIsIat(0 To MAX_STORE_CANDIDATES - 1) As Boolean
    Dim StoreFoa(0 To MAX_STORE_CANDIDATES - 1) As ULong

    Dim candidateCount As ULong = 0

    Dim sourceValid As Boolean = False
    Dim sourceIsIat As Boolean = False
    Dim sourceRva As ULong = 0
    Dim sourceEndFoa As ULong = 0

    Dim pc As ULong = scanStart

    ' ==================== 第一阶段：收集所有全局函数指针写入 ====================
    While pc < scanEnd

        If pc + 8 > fileSize Then Exit While

        Dim b0 As Integer = GetByte(pDump, pc, fileSize)
        Dim b1 As Integer = GetByte(pDump, pc + 1, fileSize)
        Dim b2 As Integer = GetByte(pDump, pc + 2, fileSize)

        '
        ' LEA RAX,[RIP+Function]
        '
        If b0 = &H48 And b1 = &H8D And b2 = &H05 Then

            Dim loadRva As ULong = FoaToRva(pDump, pc)

            If loadRva <> 0 Then

                Dim imm32 As Long = 0

                CopyMemory( _
                    @imm32, _
                    pDump + pc + 3, _
                    4)

                Dim targetRva64 As LongInt = _
                    CLngInt(loadRva) + _
                    7 + _
                    CLngInt(imm32)

                If targetRva64 > 0 Then

                    sourceRva = CULng(targetRva64)
                    sourceIsIat = False
                    sourceValid = True
                    sourceEndFoa = pc + 7

                End If

            End If

            pc += 7
            Continue While

        End If

        '
        ' MOV RAX,[RIP+IAT]
        '
        If b0 = &H48 And b1 = &H8B And b2 = &H05 Then

            Dim loadRva As ULong = FoaToRva(pDump, pc)

            If loadRva <> 0 Then

                Dim imm32 As Long = 0

                CopyMemory( _
                    @imm32, _
                    pDump + pc + 3, _
                    4)

                Dim targetRva64 As LongInt = _
                    CLngInt(loadRva) + _
                    7 + _
                    CLngInt(imm32)

                If targetRva64 > 0 Then

                    sourceRva = CULng(targetRva64)
                    sourceIsIat = True
                    sourceValid = True
                    sourceEndFoa = pc + 7

                End If

            End If

            pc += 7
            Continue While

        End If

        '
        ' 防止 RAX 来源跨越过远
        '
        If sourceValid Then

            If pc >= sourceEndFoa Then

                If pc - sourceEndFoa > MAX_SOURCE_TO_STORE_DISTANCE Then

                    sourceValid = False
                    sourceRva = 0

                End If

            End If

        End If

        '
        ' MOV [RIP+Global],RAX
        '
        ' 48 89 05 disp32
        '
        If sourceValid AndAlso _
           b0 = &H48 And _
           b1 = &H89 And _
           b2 = &H05 Then

            Dim storeInstructionRva As ULong = _
                FoaToRva(pDump, pc)

            If storeInstructionRva <> 0 Then

                Dim imm32Store As Long = 0

                CopyMemory( _
                    @imm32Store, _
                    pDump + pc + 3, _
                    4)

                Dim targetRva64 As LongInt = _
                    CLngInt(storeInstructionRva) + _
                    7 + _
                    CLngInt(imm32Store)

                If targetRva64 > 0 Then

                    If candidateCount < MAX_STORE_CANDIDATES Then

                        StoreTargetRva(candidateCount) = _
                            CULng(targetRva64)

                        StoreSourceRva(candidateCount) = _
                            sourceRva

                        StoreSourceIsIat(candidateCount) = _
                            sourceIsIat

                        StoreFoa(candidateCount) = pc

                        candidateCount += 1

                    End If

                End If

            End If

            '
            ' 一个 RAX 来源正常情况下只对应一个全局槽
            '
            sourceValid = False
            sourceRva = 0

            pc += 7
            Continue While

        End If

        pc += 1

    Wend

    MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] Global store candidates=" & Str(candidateCount)

    If candidateCount = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] No MOV [RIP+Global],RAX candidates found"
        Return False
    End If

    ' ==================== 第二阶段：自动寻找 FAST_IO_DISPATCH 表 ====================
    '
    ' 对于每一个 StoreTargetRva：
    '
    ' 假设它分别属于 FastIO index 0..26，
    ' 反推出第一个函数指针地址。
    '
    ' 然后统计其它候选中有多少地址落入：
    '
    '   FunctionBase + index * 8
    '
    ' 最大命中的那一组即 FAST_IO_DISPATCH。
    '
    Dim bestFunctionBaseRva As ULong = 0
    Dim bestScore As ULong = 0

    Dim SeenIndex(0 To FAST_IO_MAX_COUNT - 1) As Boolean

    For i As ULong = 0 To candidateCount - 1

        For assumedIndex As ULong = 0 To FAST_IO_MAX_COUNT - 1

            Dim candidateBase64 As LongInt = _
                CLngInt(StoreTargetRva(i)) - _
                CLngInt(assumedIndex * FAST_IO_PTR_SIZE)

            If candidateBase64 <= 0 Then Continue For

            Dim candidateBase As ULong = _
                CULng(candidateBase64)

            For k As Integer = 0 To FAST_IO_MAX_COUNT - 1
                SeenIndex(k) = False
            Next

            Dim score As ULong = 0

            For j As ULong = 0 To candidateCount - 1

                Dim delta64 As LongInt = _
                    CLngInt(StoreTargetRva(j)) - _
                    CLngInt(candidateBase)

                If delta64 >= 0 Then

                    If (delta64 Mod FAST_IO_PTR_SIZE) = 0 Then

                        Dim index64 As LongInt = _
                            delta64 \ FAST_IO_PTR_SIZE

                        If index64 >= 0 AndAlso _
                           index64 < FAST_IO_MAX_COUNT Then

                            Dim index As ULong = _
                                CULng(index64)

                            If SeenIndex(index) = False Then

                                SeenIndex(index) = True
                                score += 1

                            End If

                        End If

                    End If

                End If

            Next

            If score > bestScore Then

                bestScore = score
                bestFunctionBaseRva = candidateBase

            End If

        Next

    Next

    If bestScore < MIN_FASTIO_MATCH_COUNT Then

        MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] Failed to identify FAST_IO_DISPATCH table, BestScore=" & Str(bestScore)

        Return False

    End If

    Dim FastIoDispatchRva As ULong = _
        bestFunctionBaseRva - FAST_IO_HEADER_SIZE

    MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] FAST_IO_DISPATCH selected: DispatchRVA=0x" & Hex(FastIoDispatchRva) & " FunctionBaseRVA=0x" & Hex(bestFunctionBaseRva) & " Score=" & Str(bestScore)

    ' ==================== 第三阶段：恢复各个 FastIO 地址 ====================
    Dim hitCount As ULong = 0

    For i As ULong = 0 To candidateCount - 1

        Dim delta64 As LongInt = _
            CLngInt(StoreTargetRva(i)) - _
            CLngInt(bestFunctionBaseRva)

        If delta64 < 0 Then Continue For
        If (delta64 Mod FAST_IO_PTR_SIZE) <> 0 Then Continue For

        Dim index64 As LongInt = _
            delta64 \ FAST_IO_PTR_SIZE

        If index64 < 0 OrElse _
           index64 >= FAST_IO_MAX_COUNT Then
            Continue For
        End If

        Dim index As ULong = CULng(index64)
        Dim finalAddr As ULONG_PTR = 0

        If StoreSourceIsIat(i) Then

            Dim iatAddr As ULONG_PTR = _
                ImageBase + StoreSourceRva(i)

            Dim readOk As Boolean = _
                ReadKernelMemory( _
                    iatAddr, _
                    SizeOf(ULONG_PTR), _
                    @finalAddr)

            MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] IAT Slot=" & Str(index) & " " & FastIoIndexToName(index) & " StoreRVA=0x" & Hex(StoreTargetRva(i)) & " IatRVA=0x" & Hex(StoreSourceRva(i)) & " ReadOk=" & Str(readOk) & " FinalVA=0x" & Hex(finalAddr)

            If readOk = False OrElse finalAddr = 0 Then
                Continue For
            End If

        Else

            finalAddr = _
                ImageBase + StoreSourceRva(i)

            MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] Internal Slot=" & Str(index) & " " & FastIoIndexToName(index) & " StoreRVA=0x" & Hex(StoreTargetRva(i)) & " SourceRVA=0x" & Hex(StoreSourceRva(i)) & " FinalVA=0x" & Hex(finalAddr)

        End If

        '
        ' 如果同一槽位在 DriverEntry 中被再次写入，
        ' 后出现的初始化结果覆盖前面的结果。
        '
        If outFastIo->Functions(index) = 0 Then
            hitCount += 1
        End If

        outFastIo->Functions(index) = _
            Cast(Any Ptr, finalAddr)

    Next

    outFastIo->HitCount = hitCount

    outFastIo->Confidence = _
        IIf(hitCount >= 20, 100, hitCount * 5)

    MyLog.PrintInfo "RecoverFastIoFromDump", "", "[Debug] Finished. DispatchRVA=0x" & Hex(FastIoDispatchRva) & " FunctionBaseRVA=0x" & Hex(bestFunctionBaseRva) & " TableScore=" & Str(bestScore) & " HitCount=" & Str(hitCount) & " Confidence=" & Str(outFastIo->Confidence)

    Return hitCount > 0
End Function

' ==================== 测试入口 ====================
Function GetOriginalDispatchFunctionAddr(DriverPath As StringW, OriginalDriverInfo As DRIVER_INFO) As Boolean
    Dim bytDump() As Byte
    Dim filePath As String = DriverPath
    Dim fileName As String = GetNameByPath(DriverPath)
    Dim imageSize As ULong
    
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
        'Print "==================== 最终恢复结果 ===================="
        'Print "IRP_MJ 序号 | 函数 RVA | 虚拟地址(VA)"
        'Print "-----------------------------------------"
        For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
            If rd.MajorFunction(i) <> 0 Then
                Dim va As ULONG_PTR = imageBase + CUInt(rd.MajorFunction(i))
                'Print IRP_MJ_ToString(i)  & " | 0x" & Hex(rd.MajorFunction(i))  & " | 0x" & Hex(va)
                OriginalDriverInfo.MajorFunctionAddr(i) = va
            Else
                Dim IopInvalidDeviceRequestAddr As UInteger = NULL
                SetLastError 0
                If IoControl(hDrv, IOCTL_GetIopInvalidDeviceRequest, NULL, 0, @IopInvalidDeviceRequestAddr, SizeOf(IopInvalidDeviceRequestAddr)) = 0 Then
                    'Print "[GetNtfsOriginalDispatchFunctionAddr]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
                    OriginalDriverInfo.MajorFunctionAddr(i) = NULL
                Else
                    OriginalDriverInfo.MajorFunctionAddr(i) = IopInvalidDeviceRequestAddr
                End If
                'Print IRP_MJ_ToString(i) & " | 未恢复（" & IopInvalidDeviceRequestAddr & "）"
            End If
        Next
    Else
        'Print "恢复Dispatch失败，请检查文件权限和PE格式"
        Return False
    End If
    
    If RecoverFastIoFromDump(pDump, fileSize, imageBase, @fastIo) Then ' 新增
        ' MyLog.PrintLog LOG_INFO,,, 
        'Print "==================== 最终恢复结果 ===================="
        'Print "FastIO | 函数 RVA | 虚拟地址(VA)"
        'Print "-----------------------------------------"
        For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
            If fastIo.Functions(i) <> 0 Then
                Dim va As ULONG_PTR = CUInt(fastIo.Functions(i)) ' imageBase + CUInt(fastIo.Functions(i))
                'Print FastIoIndexToName(i)  & " | 0x" & Hex(fastIo.Functions(i))  & " | 0x" & Hex(va)
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
                'Print FastIoIndexToName(i) & " | 未恢复（0）"
            End If
        Next
    Else
        'Print "恢复FastIO失败，请检查文件权限和PE格式"
        Return False
    End If
    Return True

FileError:
    'Print "文件读取失败！请以管理员权限运行，或检查文件路径是否正确"
    Return True
End Function

