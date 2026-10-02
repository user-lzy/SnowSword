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

Private Function ValidateFastIoDispatchSize( _
    ByVal structureSize As ULong, _
    ByRef slotCount As ULong) As Boolean

    Const FAST_IO_HEADER_SIZE = 8

    slotCount = 0

    ' 至少包含 Size 字段和一个函数指针
    If structureSize < (FAST_IO_HEADER_SIZE + FAST_IO_PTR_SIZE) Then
        Return False
    End If

    ' 防止非法非对齐尺寸
    If ((structureSize - FAST_IO_HEADER_SIZE) Mod FAST_IO_PTR_SIZE) <> 0 Then
        Return False
    End If

    Dim declaredCount As ULong = _
        (structureSize - FAST_IO_HEADER_SIZE) \ FAST_IO_PTR_SIZE

    If declaredCount = 0 Then
        Return False
    End If

    ' 目标结构可以大于 SnowSword 已知结构
    ' 只解析已知的 FAST_IO_MAX_COUNT 项
    If declaredCount > FAST_IO_MAX_COUNT Then
        slotCount = FAST_IO_MAX_COUNT
    Else
        slotCount = declaredCount
    End If

    Return slotCount > 0

End Function

Private Function ValidateFastIoDispatchTable( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal dispatchRva As ULong, _
    ByRef slotCount As ULong) As Boolean

    slotCount = 0

    If pDump = NULL OrElse _
       fileSize = 0 OrElse _
       dispatchRva = 0 Then

        Return False
    End If

    Dim structureSize As ULong = 0

    If ReadRawRvaDword( _
        pDump, _
        fileSize, _
        dispatchRva, _
        structureSize) = False Then

        Return False
    End If

    Return ValidateFastIoDispatchSize( _
        structureSize, _
        slotCount)
End Function

Private Function ValidateFastIoDispatchRuntime( _
    ByVal dispatchAddress As ULONG_PTR, _
    ByRef slotCount As ULong) As Boolean

    slotCount = 0

    If dispatchAddress = 0 Then
        Return False
    End If

    Dim structureSize As ULong = 0

    If ReadKernelMemory( _
        dispatchAddress, _
        SizeOf(structureSize), _
        @structureSize) = False Then

        MyLog.PrintInfo "ValidateFastIoDispatchRuntime", "", _
            "[REJECT] Failed to read runtime FAST_IO_DISPATCH header VA=0x" & _
            Hex(dispatchAddress)

        Return False
    End If

    Dim valid As Boolean = _
        ValidateFastIoDispatchSize( _
            structureSize, _
            slotCount)

    MyLog.PrintInfo "ValidateFastIoDispatchRuntime", "", _
        "[RUNTIME] VA=0x" & _
        Hex(dispatchAddress) & _
        " Size=0x" & _
        Hex(structureSize) & _
        " SlotCount=" & _
        Str(slotCount) & _
        " Valid=" & _
        Str(valid)

    Return valid
End Function

Private Function ScanFastIoDynamicInitialization( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal ImageBase As ULONG_PTR, _
    ByVal functionBaseRva As ULong, _
    ByVal slotCount As ULong, _
    ByVal entryRva As ULong, _
    ByVal outFastIo As FAST_IO_DISPATCH_ORIGINAL Ptr, _
    ByRef hitCount As ULong) As ULong

    Const SCAN_RANGE = &H3000

    Dim entryFoa As ULong = RvaToFoa(pDump, entryRva)

    If entryFoa = 0 Then
        Return 0
    End If

    Dim scanStart As ULong = IIf(entryFoa > SCAN_RANGE, entryFoa - SCAN_RANGE, 0)
    Dim scanEnd As ULong = entryFoa + SCAN_RANGE

    If scanEnd > fileSize Then
        scanEnd = fileSize
    End If

    Dim u As ud

    ud_init(@u)
    ud_set_mode(@u,64)
    ud_set_pc(@u,entryRva - (entryFoa - scanStart))
    ud_set_input_buffer(@u,pDump + scanStart,scanEnd - scanStart)

    Dim RaxState As FASTIO_SOURCE_STATE

    RaxState.Valid=False
    RaxState.IsIat=False
    RaxState.SourceRva=0

    Dim candidateCount As ULong = 0


    While ud_decode(@u) <> 0

        Dim mnemonic As ud_mnemonic_code = ud_insn_mnemonic(@u)

        Dim op0 As Const ud_operand Ptr = ud_insn_opr(@u,0)
        Dim op1 As Const ud_operand Ptr = ud_insn_opr(@u,1)


        Select Case mnemonic


        Case UD_Ilea

            If op0<>NULL AndAlso op1<>NULL Then

                If op0->typ=UD_OP_REG AndAlso _
                   UdRegToGprIndex(op0->base)=0 AndAlso _
                   op1->typ=UD_OP_MEM AndAlso _
                   op1->Base=UD_R_RIP Then


                    Dim targetRva As ULong = _
                        GetRipRelativeOperandRva(@u,op1)

                    If targetRva<>0 AndAlso _
                       IsRvaExecutable(pDump,targetRva) Then

                        RaxState.Valid=True
                        RaxState.IsIat=False
                        RaxState.SourceRva=targetRva

                    Else

                        RaxState.Valid=False
                        RaxState.SourceRva=0

                    End If

                End If

            End If



        Case UD_Imov


            If op0<>NULL AndAlso op1<>NULL Then


                ' mov rax,[rip+xxx]
                If op0->typ=UD_OP_REG AndAlso _
                   UdRegToGprIndex(op0->base)=0 AndAlso _
                   op1->typ=UD_OP_MEM AndAlso _
                   op1->Base=UD_R_RIP Then


                    Dim targetRva As ULong = _
                        GetRipRelativeOperandRva(@u,op1)


                    If targetRva<>0 Then

                        RaxState.Valid=True
                        RaxState.IsIat=True
                        RaxState.SourceRva=targetRva

                    Else

                        RaxState.Valid=False
                        RaxState.SourceRva=0

                    End If


                ' mov [rip+xxx],rax
                ElseIf op0->typ=UD_OP_MEM AndAlso _
                       op0->Base=UD_R_RIP AndAlso _
                       op1->typ=UD_OP_REG AndAlso _
                       UdRegToGprIndex(op1->base)=0 Then


                    If RaxState.Valid Then


                        Dim targetRva As ULong = _
                            GetRipRelativeOperandRva(@u,op0)


                        If targetRva<>0 Then


                            Dim delta64 As LongInt = _
                                CLngInt(targetRva)- _
                                CLngInt(functionBaseRva)


                            If delta64>=0 AndAlso _
                               (delta64 Mod FAST_IO_PTR_SIZE)=0 Then


                                Dim index64 As LongInt = _
                                    delta64 \ FAST_IO_PTR_SIZE


                                If index64>=0 AndAlso _
                                   index64<slotCount AndAlso _
                                   index64<FAST_IO_MAX_COUNT Then


                                    Dim finalAddr As ULONG_PTR=0


                                    If RaxState.IsIat Then


                                        If ReadKernelMemory( _
                                            ImageBase+RaxState.SourceRva, _
                                            SizeOf(ULONG_PTR), _
                                            @finalAddr)=False Then

                                            finalAddr=0

                                        End If


                                    Else


                                        If IsRvaExecutable( _
                                            pDump, _
                                            RaxState.SourceRva) Then

                                            finalAddr=ImageBase+RaxState.SourceRva

                                        End If


                                    End If



                                    If finalAddr<>0 Then


                                        If outFastIo->Functions(index64)=0 Then
                                            hitCount+=1
                                        End If


                                        outFastIo->Functions(index64)= _
                                            Cast(Any Ptr,finalAddr)

                                        candidateCount+=1


                                    End If


                                End If


                            End If


                        End If


                    End If


                    RaxState.Valid=False
                    RaxState.SourceRva=0


                Else

                    ' 不认识的 mov，避免污染 RAX 状态
                    If op0->typ=UD_OP_REG AndAlso _
                       UdRegToGprIndex(op0->base)=0 Then

                        RaxState.Valid=False
                        RaxState.SourceRva=0

                    End If

                End If


            End If



        Case UD_Icall

            RaxState.Valid=False
            RaxState.SourceRva=0



        Case Else


            If UdMnemonicWritesOp0(mnemonic) Then

                If op0<>NULL AndAlso _
                   op0->typ=UD_OP_REG Then


                    If UdRegToGprIndex(op0->base)=0 Then

                        RaxState.Valid=False
                        RaxState.SourceRva=0

                    End If

                End If

            End If


        End Select


    Wend


    Return candidateCount

End Function

Function RecoverFastIoFromDump( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal ImageBase As ULONG_PTR, _
    ByVal FastIoDispatchRvaHint As ULong, _
    ByVal CurrentFastIoDispatchAddr As ULONG_PTR, _
    ByVal outFastIo As FAST_IO_DISPATCH_ORIGINAL Ptr) As Boolean

    Const SCAN_RANGE = &H3000
    Const FAST_IO_HEADER_SIZE = 8
    Const MAX_SOURCE_TO_STORE_DISTANCE = 64
    Const MAX_STORE_CANDIDATES = 512

    If outFastIo = NULL Then Return False

    outFastIo->HitCount = 0
    outFastIo->Confidence = 0

    For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
        outFastIo->Functions(i) = 0
    Next

    If pDump = NULL OrElse fileSize = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] Invalid dump input"
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, pDump)

    If dos->e_magic <> &H5A4D Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] Invalid DOS signature"
        Return False
    End If

    Dim preferredImageBase As ULONG_PTR = 0
    Dim sizeOfImage As ULong = 0

    If GetPeImageInfo( _
        pDump, _
        preferredImageBase, _
        sizeOfImage) = False Then

        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] GetPeImageInfo failed"

        Return False
    End If

    '
    ' ============================================================
    ' 第一阶段：
    '
    ' 确定 DRIVER_OBJECT->FastIoDispatch 对应 RVA。
    '
    ' static anchor 是恢复“原始值”的优先证据。
    '
    ' raw PE header 无效并不代表 anchor 错误：
    ' FAST_IO_DISPATCH 可能位于 .bss，并在 DriverEntry 中动态初始化。
    '
    ' 这种情况下，如果 runtime 指针和 static anchor 完全一致，
    ' 则允许使用 runtime memory 中的 header 为 static anchor 背书。
    ' ============================================================
    '

    Dim FastIoDispatchRva As ULong = 0
    Dim slotCount As ULong = 0

    Dim staticHeaderValid As Boolean = False
    Dim staticSlotCount As ULong = 0

    If FastIoDispatchRvaHint <> 0 Then
        staticHeaderValid = _
            ValidateFastIoDispatchTable( _
                pDump, _
                fileSize, _
                FastIoDispatchRvaHint, _
                staticSlotCount)

        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] Static FastIo anchor RVA=0x" & _
            Hex(FastIoDispatchRvaHint) & _
            " RawHeaderValid=" & _
            Str(staticHeaderValid) & _
            " SlotCount=" & _
            Str(staticSlotCount)
    End If

    '
    ' 当前运行时 FastIoDispatch。
    '
    Dim runtimeRva As ULong = 0
    Dim runtimeHeaderValid As Boolean = False
    Dim runtimeSlotCount As ULong = 0

    If CurrentFastIoDispatchAddr <> 0 AndAlso _
       CurrentFastIoDispatchAddr >= ImageBase AndAlso _
       CurrentFastIoDispatchAddr < ImageBase + sizeOfImage Then

        runtimeRva = _
            CULng(CurrentFastIoDispatchAddr - ImageBase)

        runtimeHeaderValid = _
            ValidateFastIoDispatchRuntime( _
                CurrentFastIoDispatchAddr, _
                runtimeSlotCount)

        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] Runtime FastIoDispatch RVA=0x" & _
            Hex(runtimeRva) & _
            " HeaderValid=" & _
            Str(runtimeHeaderValid) & _
            " SlotCount=" & _
            Str(runtimeSlotCount)

    ElseIf CurrentFastIoDispatchAddr <> 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[WARN] Runtime FastIoDispatch is outside current module VA=0x" & _
            Hex(CurrentFastIoDispatchAddr)
    End If

    '
    ' ============================================================
    ' Anchor 选择。
    ' ============================================================
    '

    If FastIoDispatchRvaHint <> 0 Then

        '
        ' 情况 1：
        ' raw PE 中已经是完整静态 FAST_IO_DISPATCH。
        '
        If staticHeaderValid Then
            FastIoDispatchRva = FastIoDispatchRvaHint
            slotCount = staticSlotCount

            If runtimeRva <> 0 Then
                If runtimeRva = FastIoDispatchRvaHint Then
                    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                        "[Debug] Static/runtime FastIoDispatch match RVA=0x" & _
                        Hex(FastIoDispatchRvaHint)
                Else
                    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                        "[WARN] Static/runtime FastIoDispatch mismatch Static=0x" & _
                        Hex(FastIoDispatchRvaHint) & _
                        " Runtime=0x" & _
                        Hex(runtimeRva)
                End If
            End If

        '
        ' 情况 2：
        ' raw header 无效，但 runtime 指向完全相同的 static anchor。
        '
        ' 典型情况：.bss FAST_IO_DISPATCH 在 DriverEntry 中动态构造。
        '
        ElseIf CBool(runtimeRva <> 0) AndAlso _
               CBool(runtimeRva = FastIoDispatchRvaHint) AndAlso _
               runtimeHeaderValid Then

            FastIoDispatchRva = FastIoDispatchRvaHint
            slotCount = runtimeSlotCount

            MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                "[Debug] Static anchor validated by runtime memory RVA=0x" & _
                Hex(FastIoDispatchRva) & _
                " SlotCount=" & _
                Str(slotCount)

        Else
            MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                "[WARN] Static FastIo anchor could not be validated RVA=0x" & _
                Hex(FastIoDispatchRvaHint) & _
                " RuntimeRVA=0x" & _
                Hex(runtimeRva) & _
                " RuntimeValid=" & _
                Str(runtimeHeaderValid)
        End If

    '
    ' 情况 3：
    ' 没有静态 anchor，才允许单独使用 runtime fallback。
    '
    ElseIf runtimeRva <> 0 AndAlso runtimeHeaderValid Then

        FastIoDispatchRva = runtimeRva
        slotCount = runtimeSlotCount

        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] Use runtime-only FastIoDispatch fallback RVA=0x" & _
            Hex(FastIoDispatchRva) & _
            " SlotCount=" & _
            Str(slotCount)
    End If

    If FastIoDispatchRva = 0 OrElse slotCount = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] FastIoDispatch RVA unresolved"

        Return False
    End If

    If slotCount > FAST_IO_MAX_COUNT Then
        slotCount = FAST_IO_MAX_COUNT
    End If

    Dim functionBaseRva As ULong = _
        FastIoDispatchRva + FAST_IO_HEADER_SIZE

    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
        "[Debug] FAST_IO_DISPATCH confirmed DispatchRVA=0x" & _
        Hex(FastIoDispatchRva) & _
        " FunctionBaseRVA=0x" & _
        Hex(functionBaseRva) & _
        " SlotCount=" & _
        Str(slotCount)

    '
    ' ============================================================
    ' 第二阶段：
    '
    ' 尝试读取 raw PE 中已经静态初始化的 Functions[]。
    '
    ' 如果表位于 .bss，则这里读不到有效静态函数指针也没关系，
    ' 后面的动态初始化扫描负责恢复。
    ' ============================================================
    '

    Dim hitCount As ULong = 0

    For index As ULong = 0 To slotCount - 1
        Dim slotRva As ULong = _
            functionBaseRva + index * FAST_IO_PTR_SIZE

        Dim rawPointer As ULONG_PTR = 0

        If ReadRawRvaQword( _
            pDump, _
            fileSize, _
            slotRva, _
            rawPointer) = False Then

            MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                "[STATIC-READ-FAIL] Slot=" & _
                Str(index) & _
                " " & _
                FastIoIndexToName(index) & _
                " SlotRVA=0x" & _
                Hex(slotRva)

            Continue For
        End If

        If rawPointer = 0 Then
            Continue For
        End If

        If rawPointer >= preferredImageBase AndAlso _
           rawPointer < preferredImageBase + sizeOfImage Then

            Dim targetRva As ULong = _
                CULng(rawPointer - preferredImageBase)

            If IsRvaExecutable(pDump, targetRva) = False Then
                MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                    "[REJECT] Static Slot=" & _
                    Str(index) & _
                    " " & _
                    FastIoIndexToName(index) & _
                    " TargetRVA=0x" & _
                    Hex(targetRva) & _
                    " not executable"

                Continue For
            End If

            Dim finalAddr As ULONG_PTR = _
                ImageBase + targetRva

            outFastIo->Functions(index) = _
                Cast(Any Ptr, finalAddr)

            hitCount += 1

            MyLog.PrintInfo "RecoverFastIoFromDump", "", _
                "[STATIC] Slot=" & _
                Str(index) & _
                " " & _
                FastIoIndexToName(index) & _
                " SlotRVA=0x" & _
                Hex(slotRva) & _
                " TargetRVA=0x" & _
                Hex(targetRva) & _
                " FinalVA=0x" & _
                Hex(finalAddr)

            Continue For
        End If

        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[STATIC-SKIP] Slot=" & _
            Str(index) & _
            " " & _
            FastIoIndexToName(index) & _
            " RawVA=0x" & _
            Hex(rawPointer) & _
            " outside preferred image"
    Next

    '
    ' ============================================================
    ' 第三阶段：
    '
    ' 扫描 DriverEntry 附近的动态初始化：
    '
    '   LEA RAX,[RIP+InternalFunction]
    '       或
    '   MOV RAX,[RIP+IAT]
    '
    '   ...
    '
    '   MOV [RIP+GlobalSlot],RAX
    '
    ' ============================================================
    '

    Dim entryRva As ULong = _
        GetImageEntryPointRVA(pDump)

    entryRva = _
        ResolveRealDriverEntryRVA( _
            pDump, _
            fileSize, _
            entryRva)

    Dim entryFoa As ULong = _
        RvaToFoa(pDump, entryRva)

    If entryFoa = 0 Then
        MyLog.PrintInfo "RecoverFastIoFromDump", "", _
            "[Debug] DriverEntry FOA unavailable, skip dynamic FastIO scan"

        outFastIo->HitCount = hitCount

        If FastIoDispatchRvaHint <> 0 Then
            If runtimeRva = FastIoDispatchRvaHint AndAlso _
               runtimeHeaderValid Then

                outFastIo->Confidence = _
                    IIf(hitCount = 0, _
                        60, _
                        80 + IIf(hitCount >= 10, 20, hitCount * 2))
            Else
                outFastIo->Confidence = _
                    IIf(hitCount = 0, _
                        40, _
                        60 + IIf(hitCount >= 10, 20, hitCount * 2))
            End If
        Else
            outFastIo->Confidence = _
                IIf(hitCount >= 20, 100, hitCount * 5)
        End If

        If outFastIo->Confidence > 100 Then
            outFastIo->Confidence = 100
        End If

        Return hitCount > 0
    End If

    Dim scanStart As ULong = _
        IIf(entryFoa > SCAN_RANGE, _
            entryFoa - SCAN_RANGE, _
            0)

    Dim scanEnd As ULong = _
        entryFoa + SCAN_RANGE

    If scanEnd > fileSize Then
        scanEnd = fileSize
    End If

    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
        "[Debug] Dynamic scan EntryRVA=0x" & _
        Hex(entryRva) & _
        " EntryFOA=0x" & _
        Hex(entryFoa) & _
        " ScanStart=0x" & _
        Hex(scanStart) & _
        " ScanEnd=0x" & _
        Hex(scanEnd)

    Dim dynamicCandidates As ULong = _
    ScanFastIoDynamicInitialization( _
        pDump, _
        fileSize, _
        ImageBase, _
        functionBaseRva, _
        slotCount, _
        entryRva, _
        outFastIo, _
        hitCount)

    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
        "[Debug] Dynamic FastIO candidates=" & _
        Str(dynamicCandidates)

    outFastIo->HitCount = hitCount

    If FastIoDispatchRvaHint <> 0 Then
        If CBool(runtimeRva <> 0) AndAlso _
           CBool(runtimeRva = FastIoDispatchRvaHint) AndAlso _
           runtimeHeaderValid Then

            If hitCount = 0 Then
                outFastIo->Confidence = 60
            Else
                outFastIo->Confidence = _
                    80 + IIf(hitCount >= 10, _
                        20, _
                        hitCount * 2)
            End If
        Else
            If hitCount = 0 Then
                outFastIo->Confidence = 40
            Else
                outFastIo->Confidence = _
                    60 + IIf(hitCount >= 10, _
                        20, _
                        hitCount * 2)
            End If
        End If
    Else
        outFastIo->Confidence = _
            IIf(hitCount >= 20, _
                100, _
                hitCount * 5)
    End If

    If outFastIo->Confidence > 100 Then
        outFastIo->Confidence = 100
    End If

    MyLog.PrintInfo "RecoverFastIoFromDump", "", _
        "[Debug] Finished DispatchRVA=0x" & _
        Hex(FastIoDispatchRva) & _
        " FunctionBaseRVA=0x" & _
        Hex(functionBaseRva) & _
        " StaticAnchorRVA=0x" & _
        Hex(FastIoDispatchRvaHint) & _
        " RuntimeRVA=0x" & _
        Hex(runtimeRva) & _
        " RawHeaderValid=" & _
        Str(staticHeaderValid) & _
        " RuntimeHeaderValid=" & _
        Str(runtimeHeaderValid) & _
        " HitCount=" & _
        Str(hitCount) & _
        " Confidence=" & _
        Str(outFastIo->Confidence)

    Return hitCount > 0
End Function