Sub SetRecoveredDispatch( _
    ByRef OutputDispatch As DISPATCH_RECOVERY_OUTPUT, _
    ByVal IrpIndex As Integer, _
    ByVal ModuleBase As ULONG_PTR, _
    ByVal FunctionRva As ULong, _
    ByRef Source As String)

    If IrpIndex < 0 OrElse IrpIndex > IRP_MJ_MAXIMUM_FUNCTION Then Exit Sub
    If ModuleBase = 0 OrElse FunctionRva = 0 Then Exit Sub

    If OutputDispatch.MajorFunction(IrpIndex).Valid = False Then
        OutputDispatch.HitCount += 1
    End If

    OutputDispatch.MajorFunction(IrpIndex).Valid = True
    OutputDispatch.MajorFunction(IrpIndex).ModuleBase = ModuleBase
    OutputDispatch.MajorFunction(IrpIndex).FunctionRva = FunctionRva

    ' === 新增字段（不影响原有逻辑） ===
    OutputDispatch.MajorFunction(IrpIndex).State = DispatchSlotRecovered
    OutputDispatch.MajorFunction(IrpIndex).OwnerKind = DispatchOwnerDriver

    If Source = "REP_STOSQ" Then
        OutputDispatch.MajorFunction(IrpIndex).Source = _
            DispatchSourceRepStosq
    ElseIf Source = "HIDREGISTERMINIDRIVER" Then
        OutputDispatch.MajorFunction(IrpIndex).Source = _
            DispatchSourceImportedFramework
    Else
        OutputDispatch.MajorFunction(IrpIndex).Source = _
            DispatchSourceDirectStore
    End If

    OutputDispatch.MajorFunction(IrpIndex).Confidence = 0 ' OutputDispatch.Confidence

    MyLog.PrintInfo "SetRecoveredDispatch", "", _
        "[MATCH] " & IRP_MJ_ToString(IrpIndex) & _
        " ModuleBase=0x" & Hex(ModuleBase) & _
        " RVA=0x" & Hex(FunctionRva) & _
        " FinalVA=0x" & Hex(ModuleBase + FunctionRva) & _
        " Source=" & Source
End Sub

Function RecoverDispatchFromDump( _
    ByRef InputDispatch As DISPATCH_RECOVERY_INPUT, _
    ByRef OutputDispatch As DISPATCH_RECOVERY_OUTPUT) As DISPATCH_RECOVERY_STATUS

    memset(@OutputDispatch, 0, SizeOf(DISPATCH_RECOVERY_OUTPUT))

    If InputDispatch.pDump = NULL OrElse InputDispatch.FileSize = 0 Then
        Return DispatchRecoveryParseFailed
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, InputDispatch.pDump)

    If dos->e_magic <> &H5A4D Then
        Return DispatchRecoveryParseFailed
    End If

    Dim imageEntryRva As ULong = _
        GetImageEntryPointRVA(InputDispatch.pDump)

    If imageEntryRva = 0 Then
        Return DispatchRecoveryParseFailed
    End If

    Dim entryRva As ULong = _
        ResolveRealDriverEntryRVA( _
            InputDispatch.pDump, _
            InputDispatch.FileSize, _
            imageEntryRva)

    If entryRva = 0 Then
        Return DispatchRecoveryParseFailed
    End If

    Dim ScanContext As DISPATCH_SCAN_CONTEXT
    Dim ScanState As DISPATCH_SCAN_STATE

    memset(@ScanContext, 0, SizeOf(ScanContext))
    memset(@ScanState, 0, SizeOf(ScanState))

    ScanContext.pDump = InputDispatch.pDump
    ScanContext.FileSize = InputDispatch.FileSize
    ScanContext.ImageBase = InputDispatch.ImageBase
    ScanContext.ModuleName = InputDispatch.ModuleName

    ScanDispatchFunction ScanContext, _
        ScanState, _
        entryRva, _
        1, _
        0, _
        OutputDispatch

    If OutputDispatch.HitCount = 0 Then
        OutputDispatch.Confidence = 0

    MyLog.PrintInfo "RecoverDispatchFromDump", "", _
        "[Debug] No generic WDM MajorFunction initialization matched"

        Return DispatchRecoveryNoMatch
    End If

    OutputDispatch.Confidence = _
        IIf(OutputDispatch.HitCount >= 20, 100, OutputDispatch.HitCount * 5)

    MyLog.PrintInfo "RecoverDispatchFromDump", "", _
        "[Debug] Finished HitCount=" & Str(OutputDispatch.HitCount) & _
        " Confidence=" & Str(OutputDispatch.Confidence)

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        If OutputDispatch.MajorFunction(i).Valid Then
            OutputDispatch.MajorFunction(i).Confidence = _
                OutputDispatch.Confidence
        End If
    Next

    Return DispatchRecoveryRecovered
End Function

Sub ScanDispatchFunction( _
    ByRef ScanContext As DISPATCH_SCAN_CONTEXT, _
    ByRef State As DISPATCH_SCAN_STATE, _
    ByVal CurrentFunctionRva As ULong, _
    ByVal DriverObjectArgReg As Integer, _
    ByVal Depth As Integer, _
    ByRef OutputDispatch As DISPATCH_RECOVERY_OUTPUT)

    If ScanContext.pDump = NULL OrElse ScanContext.FileSize = 0 Then
        MyLog.PrintInfo "ScanDispatchFunction", "", _
            "[SCAN] Module=" & ScanContext.ModuleName & _
            " [REJECT] invalid dump context"
        Exit Sub
    End If

    If CurrentFunctionRva = 0 Then
        MyLog.PrintInfo "ScanDispatchFunction", "", _
            "[SCAN] Module=" & ScanContext.ModuleName & _
            " [REJECT] CurrentFunctionRva=0"
        Exit Sub
    End If

    If Depth > DISPATCH_MAX_CALL_DEPTH Then
        MyLog.PrintInfo "ScanDispatchFunction", "", _
            "[SCAN] Module=" & ScanContext.ModuleName & _
            " [REJECT] depth limit Depth=" & Str(Depth)
        Exit Sub
    End If

    If AddVisitedDispatchFunction(State, ScanContext.ImageBase, CurrentFunctionRva) = False Then
        MyLog.PrintInfo "ScanDispatchFunction", "", _
            "[SCAN] Module=" & ScanContext.ModuleName & _
            " [REJECT] visited/full FunctionRVA=0x" & Hex(CurrentFunctionRva)
        Exit Sub
    End If

    Dim funcFoa As ULong = RvaToFoa(ScanContext.pDump, CurrentFunctionRva)

    If funcFoa = 0 OrElse funcFoa >= ScanContext.FileSize Then
        MyLog.PrintInfo "ScanDispatchFunction", "", _
            "[SCAN] Module=" & ScanContext.ModuleName & _
            " [REJECT] invalid FOA FunctionRVA=0x" & Hex(CurrentFunctionRva) & _
            " FOA=0x" & Hex(funcFoa) & _
            " FileSize=0x" & Hex(ScanContext.FileSize)
        Exit Sub
    End If

    Dim scanSize As ULong = DISPATCH_FUNCTION_SCAN_RANGE
    If funcFoa + scanSize > ScanContext.FileSize Then
        scanSize = ScanContext.FileSize - funcFoa
    End If

    MyLog.PrintInfo "ScanDispatchFunction", "", _
        "[SCAN] Module=" & ScanContext.ModuleName & _
        " [SCAN] Depth=" & Str(Depth) & _
        " FunctionRVA=0x" & Hex(CurrentFunctionRva) & _
        " FOA=0x" & Hex(funcFoa) & _
        " ScanSize=0x" & Hex(scanSize)

    Dim u As ud
    ud_init(@u)
    ud_set_mode(@u, 64)
    ud_set_pc(@u, CurrentFunctionRva)
    ud_set_input_buffer(@u, ScanContext.pDump + funcFoa, scanSize)

    Dim RegState(0 To 15) As DISPATCH_REG_STATE

    For i As Integer = 0 To 15
        ClearDispatchReg RegState(i)
    Next

    ' Windows x64 ABI：第一个参数在 RCX。
    ' 只有递归进入被调函数时，才认为 RCX = DriverObject。
    If DriverObjectArgReg >= 0 AndAlso DriverObjectArgReg <= 15 Then
        RegState(DriverObjectArgReg).ValueType = DispatchRegDriverObject
    End If

    While ud_decode(@u) <> 0
        Dim mnemonic As ud_mnemonic_code = ud_insn_mnemonic(@u)
        Dim op0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
        Dim op1 As Const ud_operand Ptr = ud_insn_opr(@u, 1)

        Select Case mnemonic

            ' ==========================================
            ' MOV
            ' ==========================================
            Case UD_Imov

                ' MOV reg,reg
                If UdIs64BitGpr(op0) AndAlso UdIs64BitGpr(op1) Then
                    Dim dstReg As Integer = UdRegToGprIndex(op0->Base)
                    Dim srcReg As Integer = UdRegToGprIndex(op1->Base)

                    If dstReg >= 0 AndAlso srcReg >= 0 Then
                        CopyDispatchReg RegState(dstReg), RegState(srcReg)

                        If RegState(dstReg).ValueType <> DispatchRegUnknown OrElse _
                           RegState(srcReg).ValueType <> DispatchRegUnknown Then

                            MyLog.PrintInfo "ScanDispatchFunction", "", _
                               "[SCAN] Module=" & ScanContext.ModuleName & _
                                " [FLOW] MOV RVA=0x" & Hex(ud_insn_off(@u)) & _
                                " " & DispatchRegName(dstReg) & " <- " & DispatchRegName(srcReg) & _
                                " State=" & DispatchStateName(RegState(dstReg).ValueType) & _
                                " Value=0x" & Hex(RegState(dstReg).Value)
                        End If
                    End If

                    Continue While
                End If

                ' MOV [DriverObject + disp], FunctionReg
                If op0 <> NULL AndAlso op1 <> NULL AndAlso _
                   op0->typ = UD_OP_MEM AndAlso op1->typ = UD_OP_REG Then

                    Dim baseReg As Integer = UdRegToGprIndex(op0->Base)
                    Dim srcReg As Integer = UdRegToGprIndex(op1->Base)
                    Dim disp As LongInt = UdMemoryDisplacement(op0)

                    Dim baseOffset As LongInt = 0
                    Dim baseIsDriverObject As Boolean = False

                    If baseReg >= 0 Then
                        If RegState(baseReg).ValueType = DispatchRegDriverObject Then
                            baseIsDriverObject = True
                            baseOffset = 0
                        ElseIf RegState(baseReg).ValueType = DispatchRegDriverObjectAddress Then
                            baseIsDriverObject = True
                            baseOffset = CLngInt(RegState(baseReg).Value)
                        End If
                    End If

                    Dim totalDisp As LongInt = baseOffset + disp
                    
                    If baseIsDriverObject AndAlso srcReg >= 0 AndAlso op0->index = UD_NONE Then
                        If totalDisp = DRIVER_OBJECT_FAST_IO_DISPATCH AndAlso _
                           (RegState(srcReg).ValueType = DispatchRegImageRva OrElse _
                            RegState(srcReg).ValueType = DispatchRegFunctionRva) Then

                            OutputDispatch.FastIoDispatchRva = CULng(RegState(srcReg).Value)

                            MyLog.PrintInfo "ScanDispatchFunction", "", _
                                "[SCAN] Module=" & ScanContext.ModuleName & _
                                " [FASTIO-ANCHOR] RVA=0x" & Hex(OutputDispatch.FastIoDispatchRva) & _
                                " StoreRVA=0x" & Hex(ud_insn_off(@u)) & _
                                " Source=MOV"

                            Continue While
                        End If
                    End If

                    If baseIsDriverObject AndAlso _
                       srcReg >= 0 AndAlso _
                       op0->index = UD_NONE AndAlso _
                       RegState(srcReg).ValueType = DispatchRegFunctionRva Then

                        If totalDisp >= DISPATCH_BASE AndAlso _
                           totalDisp <= DISPATCH_BASE + IRP_MJ_MAXIMUM_FUNCTION * DISPATCH_STEP Then

                            Dim delta As LongInt = totalDisp - DISPATCH_BASE

                            If (delta Mod DISPATCH_STEP) = 0 Then
                                Dim irpIndex As LongInt = delta \ DISPATCH_STEP

                                If irpIndex >= 0 AndAlso irpIndex <= IRP_MJ_MAXIMUM_FUNCTION Then
                                    Dim callTarget As ULong = UdRelativeTargetRva(@u, op0)
                                    Dim RecoveredFunctionRva As ULong = CULng(RegState(srcReg).Value)
                                    Dim sourceName As String = "MOV"
                                    SetRecoveredDispatch OutputDispatch, _
                                        CInt(irpIndex), _
                                        ScanContext.ImageBase, _
                                        RecoveredFunctionRva, _
                                        sourceName
                                End If
                            End If
                        End If
                    End If

                    Continue While
                End If

                ' MOV reg,imm
                If op0 <> NULL AndAlso op1 <> NULL AndAlso _
                   op0->typ = UD_OP_REG AndAlso op1->typ = UD_OP_IMM Then

                    Dim dstReg As Integer = UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        ClearDispatchReg RegState(dstReg)

                        Dim immValue As ULONG_PTR = 0

                        Select Case op1->SIZE
                            Case 8
                                immValue = op1->lval.UByte
                            Case 16
                                immValue = op1->lval.UWORD
                            Case 32
                                immValue = op1->lval.udword
                            Case 64
                                immValue = op1->lval.uqword
                        End Select

                        RegState(dstReg).ValueType = DispatchRegImmediate
                        RegState(dstReg).Value = immValue

                        MyLog.PrintInfo "ScanDispatchFunction", "", _
                            "[SCAN] Module=" & ScanContext.ModuleName & _
                            " [FLOW] " & DispatchRegName(dstReg) & _
                            " = Immediate 0x" & Hex(immValue)
                    End If

                    Continue While
                End If

                ' 其他 MOV：目标寄存器状态失效
                If op0 <> NULL AndAlso op0->typ = UD_OP_REG Then
                    Dim dstReg As Integer = UdRegToGprIndex(op0->Base)
                    If dstReg >= 0 Then ClearDispatchReg RegState(dstReg)
                End If

            ' ==========================================
            ' LEA
            ' ==========================================
            Case UD_Ilea
                If op0 <> NULL AndAlso op0->typ = UD_OP_REG Then
                    Dim dstReg As Integer = UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        ClearDispatchReg RegState(dstReg)

                        ' LEA reg,[DriverObject + disp]
                        If op1 <> NULL AndAlso _
                           op1->typ = UD_OP_MEM AndAlso _
                           op1->index = UD_NONE Then

                            Dim baseReg As Integer = UdRegToGprIndex(op1->Base)

                            If baseReg >= 0 Then
                                Dim baseState As DISPATCH_REG_VALUE_TYPE = RegState(baseReg).ValueType
                                Dim objectDisp As LongInt = UdMemoryDisplacement(op1)

                                If baseState = DispatchRegDriverObject Then
                                    RegState(dstReg).ValueType = DispatchRegDriverObjectAddress
                                    RegState(dstReg).Value = Cast(ULONG_PTR, objectDisp)

                                    MyLog.PrintInfo "ScanDispatchFunction", "", _
                                        "[SCAN] Module=" & ScanContext.ModuleName & _
                                        " [FLOW] " & DispatchRegName(dstReg) & _
                                        " = DriverObjectAddress +0x" & Hex(objectDisp)

                                    Continue While
                                ElseIf baseState = DispatchRegDriverObjectAddress Then
                                    Dim totalDisp As LongInt = CLngInt(RegState(baseReg).Value) + objectDisp
                                    RegState(dstReg).ValueType = DispatchRegDriverObjectAddress
                                    RegState(dstReg).Value = Cast(ULONG_PTR, totalDisp)

                                    MyLog.PrintInfo "ScanDispatchFunction", "", _
                                        "[SCAN] Module=" & ScanContext.ModuleName & _
                                        " [FLOW] " & DispatchRegName(dstReg) & _
                                        " = DriverObjectAddress +0x" & Hex(totalDisp)

                                    Continue While
                                ElseIf baseState = DispatchRegImmediate Then
                                    Dim totalValue As ULONG_PTR = RegState(baseReg).Value + objectDisp

                                    If op0->SIZE = 32 Then
                                        totalValue And= &HFFFFFFFFULL
                                    ElseIf op0->SIZE <> 64 Then
                                        ClearDispatchReg RegState(dstReg)
                                        Continue While
                                    End If

                                    RegState(dstReg).ValueType = DispatchRegImmediate
                                    RegState(dstReg).Value = totalValue

                                    MyLog.PrintInfo "ScanDispatchFunction", "", _
                                        "[SCAN] Module=" & ScanContext.ModuleName & _
                                        " [FLOW] " & DispatchRegName(dstReg) & _
                                        " = Immediate 0x" & Hex(totalValue) & _
                                        " Source=LEA"

                                    Continue While
                                End If
                            End If
                        End If

                        ' LEA reg,[RIP+ImageRva]
                        If op0->SIZE = 64 AndAlso _
                           op1 <> NULL AndAlso _
                           op1->typ = UD_OP_MEM AndAlso _
                           op1->Base = UD_R_RIP AndAlso _
                           op1->index = UD_NONE Then

                            Dim disp As LongInt = UdMemoryDisplacement(op1)
                            Dim target64 As LongInt = _
                                CLngInt(ud_insn_off(@u)) + _
                                CLngInt(ud_insn_len(@u)) + _
                                disp

                            If target64 > 0 AndAlso target64 <= &HFFFFFFFFLL Then
                                Dim targetRva As ULong = CULng(target64)

                                If RvaToFoa(ScanContext.pDump, targetRva) <> 0 Then
                                    If IsRvaExecutable(ScanContext.pDump, targetRva) Then
                                        RegState(dstReg).ValueType = DispatchRegFunctionRva
                                        RegState(dstReg).Value = targetRva

                                        MyLog.PrintInfo "ScanDispatchFunction", "", _
                                            "[SCAN] Module=" & ScanContext.ModuleName & _
                                            " [FLOW] " & DispatchRegName(dstReg) & _
                                            " = CurrentFunctionRva 0x" & Hex(targetRva)
                                    Else
                                        RegState(dstReg).ValueType = DispatchRegImageRva
                                        RegState(dstReg).Value = targetRva

                                        MyLog.PrintInfo "ScanDispatchFunction", "", _
                                            "[SCAN] Module=" & ScanContext.ModuleName & _
                                            " [FLOW] " & DispatchRegName(dstReg) & _
                                            " = ImageRva 0x" & Hex(targetRva)
                                    End If
                                End If
                            End If
                        End If
                    End If
                End If

            ' ==========================================
            ' CALL
            ' ==========================================
            Case UD_Icall
                Dim passDriverObject As Boolean = _
                    (RegState(1).ValueType = DispatchRegDriverObject)

                If op0 <> NULL Then

                    ' ==========================
                    ' 1. 当前模块 direct call
                    ' ==========================
                    If op0->typ = UD_OP_JIMM Then
                        Dim callTarget As ULong = _
                            UdRelativeTargetRva(@u, op0)

                        If passDriverObject AndAlso _
                           CBool(callTarget <> 0) AndAlso _
                           CBool(callTarget <> CurrentFunctionRva) AndAlso _
                           CBool(Depth < DISPATCH_MAX_CALL_DEPTH) AndAlso _
                           IsRvaExecutable(ScanContext.pDump, callTarget) AndAlso _
                           RvaToFoa(ScanContext.pDump, callTarget) <> 0 Then

                            ScanDispatchFunction ScanContext, _
                                State, _
                                callTarget, _
                                1, _
                                Depth + 1, _
                                OutputDispatch
                        End If

                    ' ==========================
                    ' 2. IAT indirect call
                    ' ==========================
                    ElseIf op0->typ = UD_OP_MEM AndAlso _
                           op0->Base = UD_R_RIP AndAlso _
                           op0->index = UD_NONE Then

                        '
                        ' 不在这里用 RCX=DriverObject 筛 IAT 调用。
                        '
                        ' 原因：
                        '   HidRegisterMinidriver(RCX = PHID_MINIDRIVER_REGISTRATION)
                        '   等 framework API 会间接修改 DRIVER_OBJECT。
                        '
                        ' 普通跨模块函数是否可递归，
                        ' 在 ScanImportedDispatchCall 中再根据语义决定。
                        '
                        If Depth < DISPATCH_MAX_CALL_DEPTH Then
                            ScanImportedDispatchCall ScanContext, _
                                State, _
                                @u, _
                                op0, _
                                passDriverObject, _
                                Depth, _
                                OutputDispatch
                        End If
                    End If
                End If

                InvalidateVolatileDispatchRegs RegState()
                
            ' ==========================================
            ' JMP
            ' ==========================================
            /'Case UD_Ijmp
                Dim passDriverObject As Boolean = _
                    (RegState(1).ValueType = DispatchRegDriverObject)

                If passDriverObject AndAlso _
                   op0 <> NULL AndAlso _
                   op0->typ = UD_OP_JIMM Then

                    Dim jumpTarget As ULong = _
                        UdRelativeTargetRva(@u, op0)

                    If CBool(jumpTarget <> 0) AndAlso _
                       CBool(jumpTarget <> CurrentFunctionRva) AndAlso _
                       CBool(Depth < DISPATCH_MAX_CALL_DEPTH) AndAlso _
                       IsRvaExecutable(ScanContext.pDump, jumpTarget) Then

                        ScanDispatchFunction _
                            ScanContext, _
                            State, _
                            jumpTarget, _
                            1, _
                            Depth + 1, _
                            OutputDispatch
                    End If
                End If

                Exit While'/

            ' ==========================================
            ' REP STOSQ
            ' ==========================================
            Case UD_Istosq
                If RegState(7).ValueType = DispatchRegDriverObjectAddress AndAlso _
                   RegState(0).ValueType = DispatchRegFunctionRva AndAlso _
                   RegState(1).ValueType = DispatchRegImmediate Then

                    Dim startOffset As LongInt = CLngInt(RegState(7).Value)
                    Dim count As ULONG_PTR = RegState(1).Value
                    Dim RecoveredFunctionRva As ULONG_PTR = RegState(0).Value

                    If startOffset >= DISPATCH_BASE AndAlso _
                       ((startOffset - DISPATCH_BASE) Mod DISPATCH_STEP) = 0 Then

                        Dim startIndex As LongInt = _
                            (startOffset - DISPATCH_BASE) \ DISPATCH_STEP

                        If startIndex >= 0 AndAlso startIndex <= IRP_MJ_MAXIMUM_FUNCTION Then
                            Dim maxCount As ULONG_PTR = _
                                IRP_MJ_MAXIMUM_FUNCTION - startIndex + 1

                            If count <= maxCount Then
                                MyLog.PrintInfo "ScanDispatchFunction", "", _
                                    "[SCAN] Module=" & ScanContext.ModuleName & _
                                    " [REP-STOSQ] RVA=0x" & Hex(ud_insn_off(@u)) & _
                                    " StartIndex=" & Str(startIndex) & _
                                    " Count=" & Str(count) & _
                                    " FunctionRVA=0x" & Hex(RecoveredFunctionRva)

                                For n As Integer = 0 To CInt(count) - 1
                                    Dim sourceName As String = "REP_STOSQ"
                                    SetRecoveredDispatch OutputDispatch, _
                                        CInt(startIndex) + n, _
                                        ScanContext.ImageBase, _
                                        CULng(RecoveredFunctionRva), _
                                        sourceName
                                Next
                            End If
                        End If
                    End If
                End If

                ClearDispatchReg RegState(1)
                ClearDispatchReg RegState(7)

            ' ==========================================
            ' RET
            ' ==========================================
            Case UD_Iret, UD_Iretf
                MyLog.PrintInfo "ScanDispatchFunction", "", _
                    "[SCAN] Module=" & ScanContext.ModuleName & _
                    " [RET] RVA=0x" & Hex(ud_insn_off(@u)) & _
                    " Depth=" & Str(Depth)

                Exit While

            ' ==========================================
            ' 其他已知会覆盖 Op0 的指令
            ' ==========================================
            Case Else
                InvalidateUnknownInstructionDestination @u, RegState()

        End Select
    Wend
End Sub

Sub ScanImportedDispatchCall( _
    ByRef CallerContext As DISPATCH_SCAN_CONTEXT, _
    ByRef State As DISPATCH_SCAN_STATE, _
    ByVal u As ud Ptr, _
    ByVal CallOperand As Const ud_operand Ptr, _
    ByVal PassDriverObject As Boolean, _
    ByVal Depth As Integer, _
    ByRef OutputDispatch As DISPATCH_RECOVERY_OUTPUT)

    Dim iatRva As ULong = GetRipRelativeOperandRva(u, CallOperand)
    If iatRva = 0 Then Exit Sub

    Dim ImportTarget As PE_IMPORT_TARGET

    If ResolveImportByIatRva( _
        CallerContext.pDump, _
        CallerContext.FileSize, _
        iatRva, _
        ImportTarget) = False Then
        Exit Sub
    End If

    '
    ' 鏅€氬鍏ュ嚱鏁板鏋滄病鏈?DriverObject 鍙傛暟锛?
    ' 涓嶉渶瑕佷负浜嗗畠鍔犺浇澶栭儴妯″潡銆?
    '
    ' Framework API 渚嬪锛?
    ' HidRegisterMinidriver 鐨?RCX 鏄?PHID_MINIDRIVER_REGISTRATION锛?
    ' 浣嗗畠浼氶棿鎺ユ敼鍐欏叾涓?DriverObject 鐨勬淳閬ｈ〃銆?
    '
    Dim isHidRegister As Boolean = False

    If ImportTarget.ByOrdinal = False Then
        isHidRegister = _
            (LCase(ImportTarget.FunctionName) = "hidregisterminidriver") AndAlso _
            (LCase(ImportTarget.ModuleName) = "hidclass.sys")
    End If

    If PassDriverObject = False AndAlso isHidRegister = False Then
        MyLog.PrintInfo "ScanImportedDispatchCall", "", _
            "[SKIP] Imported call does not receive DriverObject" & _
            " Caller=" & CallerContext.ModuleName & _
            " Function=" & ImportTarget.FunctionName & _
            " IatRVA=0x" & Hex(iatRva)

        Exit Sub
    End If

    Dim ExternalDump() As Byte
    Dim ExternalContext As DISPATCH_SCAN_CONTEXT

    If LoadDispatchModuleContext( _
        ImportTarget.ModuleName, _
        ExternalContext, _
        ExternalDump()) = False Then
        Exit Sub
    End If

    Dim targetRva As ULong = 0

    If ResolveExportRva( _
        ExternalContext.pDump, _
        ExternalContext.FileSize, _
        ImportTarget.FunctionName, _
        ImportTarget.Ordinal, _
        ImportTarget.ByOrdinal, _
        targetRva) = False Then
        Exit Sub
    End If

    If IsRvaExecutable( _
        ExternalContext.pDump, _
        targetRva) = False Then

        MyLog.PrintInfo "ScanImportedDispatchCall", "", _
            "[REJECT] Export is not executable Module=" & _
            ExternalContext.ModuleName & _
            " Function=" & ImportTarget.FunctionName & _
            " RVA=0x" & Hex(targetRva)

        Exit Sub
    End If

    MyLog.PrintInfo "ScanImportedDispatchCall", "", _
        "[IMPORT-CALL] Caller=" & CallerContext.ModuleName & _
        " CallRVA=0x" & Hex(ud_insn_off(u)) & _
        " IatRVA=0x" & Hex(iatRva) & _
        " TargetModule=" & ExternalContext.ModuleName & _
        " Function=" & ImportTarget.FunctionName & _
        " TargetRVA=0x" & Hex(targetRva) & _
        " PassDriverObject=" & Str(PassDriverObject)

    '
    ' ============================================================
    ' HID minidriver registration
    ' ============================================================
    '
    If isHidRegister Then
        MyLog.PrintInfo "ScanImportedDispatchCall", "", _
            "[HID] HidRegisterMinidriver semantic scan Module=" & _
            ExternalContext.ModuleName & _
            " RVA=0x" & Hex(targetRva)

        If RecoverHidRegisterMinidriverDispatch( _
            ExternalContext, _
            targetRva, _
            OutputDispatch) = False Then

            MyLog.PrintInfo "ScanImportedDispatchCall", "", _
                "[HID] HidRegisterMinidriver semantic scan produced no dispatch overrides"
        End If

        Exit Sub
    End If

    '
    ' ============================================================
    ' 鏅€氳法妯″潡 DriverObject 鍒濆鍖?
    ' ============================================================
    '
    If PassDriverObject = False Then Exit Sub
    If Depth >= DISPATCH_MAX_CALL_DEPTH Then Exit Sub

    ScanDispatchFunction ExternalContext, _
        State, _
        targetRva, _
        1, _
        Depth + 1, _
        OutputDispatch
End Sub