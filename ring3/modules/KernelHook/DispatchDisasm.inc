Function DispatchRegName(ByVal RegIndex As Integer) As String
    Dim Names(0 To 15) As String = { _
        "RAX", "RCX", "RDX", "RBX", _
        "RSP", "RBP", "RSI", "RDI", _
        "R8", "R9", "R10", "R11", _
        "R12", "R13", "R14", "R15" _
    }

    If RegIndex < 0 OrElse RegIndex > 15 Then Return "?"
    Return Names(RegIndex)
End Function

Function DispatchStateName(ByVal ValueType As DISPATCH_REG_VALUE_TYPE) As String
    Select Case ValueType
        Case DispatchRegDriverObject: Return "DriverObject"
        Case DispatchRegFunctionRva: Return "FunctionRva"
        Case DispatchRegImageRva: Return "ImageRva"
        Case DispatchRegImmediate: Return "Immediate"
        Case DispatchRegDriverObjectAddress: Return "DriverObjectAddress"
    End Select
    Return "Unknown"
End Function

Function UdRegToGprIndex(ByVal RegType As ud_type) As Integer
    Select Case RegType
        Case UD_R_RAX, UD_R_EAX, UD_R_AX, UD_R_AL, UD_R_AH: Return 0
        Case UD_R_RCX, UD_R_ECX, UD_R_CX, UD_R_CL, UD_R_CH: Return 1
        Case UD_R_RDX, UD_R_EDX, UD_R_DX, UD_R_DL, UD_R_DH: Return 2
        Case UD_R_RBX, UD_R_EBX, UD_R_BX, UD_R_BL, UD_R_BH: Return 3
        Case UD_R_RSP, UD_R_ESP, UD_R_SP, UD_R_SPL: Return 4
        Case UD_R_RBP, UD_R_EBP, UD_R_BP, UD_R_BPL: Return 5
        Case UD_R_RSI, UD_R_ESI, UD_R_SI, UD_R_SIL: Return 6
        Case UD_R_RDI, UD_R_EDI, UD_R_DI, UD_R_DIL: Return 7
        Case UD_R_R8, UD_R_R8D, UD_R_R8W, UD_R_R8B: Return 8
        Case UD_R_R9, UD_R_R9D, UD_R_R9W, UD_R_R9B: Return 9
        Case UD_R_R10, UD_R_R10D, UD_R_R10W, UD_R_R10B: Return 10
        Case UD_R_R11, UD_R_R11D, UD_R_R11W, UD_R_R11B: Return 11
        Case UD_R_R12, UD_R_R12D, UD_R_R12W, UD_R_R12B: Return 12
        Case UD_R_R13, UD_R_R13D, UD_R_R13W, UD_R_R13B: Return 13
        Case UD_R_R14, UD_R_R14D, UD_R_R14W, UD_R_R14B: Return 14
        Case UD_R_R15, UD_R_R15D, UD_R_R15W, UD_R_R15B: Return 15
    End Select
    Return -1
End Function

Function UdIs64BitGpr(ByVal Opr As Const ud_operand Ptr) As Boolean
    If Opr = NULL OrElse Opr->typ <> UD_OP_REG OrElse Opr->SIZE <> 64 Then Return False
    Return UdRegToGprIndex(Opr->Base) >= 0
End Function

Function UdSignedValue(ByVal Value As ud_lval, ByVal Bits As Integer) As LongInt
    Select Case Bits
        Case 8
            Dim v8 As Byte
            memcpy(@v8, @Value.UByte, 1)
            Return CLngInt(v8)

        Case 16
            Dim v16 As Short
            memcpy(@v16, @Value.UWORD, 2)
            Return CLngInt(v16)

        Case 32
            Dim v32 As Integer
            memcpy(@v32, @Value.udword, 4)
            Return CLngInt(v32)

        Case 64
            Return Value.sqword
    End Select

    Return 0
End Function

Function UdMemoryDisplacement(ByVal Opr As Const ud_operand Ptr) As LongInt
    If Opr = NULL OrElse Opr->typ <> UD_OP_MEM Then Return 0

    '
    ' x64 RIP-relative memory operand 使用 signed disp32。
    '
    If Opr->Base = UD_R_RIP Then
        Return CLngInt(Opr->lval.sdword)
    End If

    Select Case Opr->offset
        Case 0
            Return 0

        Case 8
            Return CLngInt(Opr->lval.sbyte)

        Case 16
            Return CLngInt(Opr->lval.sword)

        Case 32
            Return CLngInt(Opr->lval.sdword)

        Case 64
            Return Opr->lval.sqword
    End Select

    Return 0
End Function

Function UdRelativeTargetRva( _
    ByVal u As ud Ptr, _
    ByVal Opr As Const ud_operand Ptr) As ULong

    If u = NULL OrElse Opr = NULL OrElse Opr->typ <> UD_OP_JIMM Then
        Return 0
    End If

    Dim rel As LongInt

    Select Case Opr->SIZE
        Case 8
            rel = CLngInt(Opr->lval.sbyte)

        Case 16
            rel = CLngInt(Opr->lval.sword)

        Case 32
            rel = CLngInt(Opr->lval.sdword)

        Case 64
            rel = Opr->lval.sqword

        Case Else
            Return 0
    End Select

    Dim target As LongInt = _
        CLngInt(ud_insn_off(u)) + _
        CLngInt(ud_insn_len(u)) + _
        rel

    If target <= 0 OrElse target > &HFFFFFFFFLL Then Return 0

    Return CULng(target)
End Function

Function UdMnemonicWritesOp0(ByVal Mnemonic As ud_mnemonic_code) As Boolean
    Select Case Mnemonic
        Case UD_Iadd, UD_Iadc, UD_Isub, UD_Isbb, UD_Iand, UD_Ior, UD_Ixor, _
             UD_Iimul, UD_Iinc, UD_Idec, UD_Ineg, UD_Inot, UD_Ishl, UD_Ishr, _
             UD_Isar, UD_Irol, UD_Iror, UD_Ircl, UD_Ircr, UD_Ipop, UD_Imovsx, _
             UD_Imovsxd, UD_Imovzx, UD_Ibswap, UD_Ixchg, UD_Ixadd
            Return True
    End Select
    Return False
End Function

Sub ClearDispatchReg(ByRef RegState As DISPATCH_REG_STATE)
    RegState.ValueType = DispatchRegUnknown
    RegState.Value = 0
End Sub

Sub CopyDispatchReg(ByRef Dst As DISPATCH_REG_STATE, ByRef Src As DISPATCH_REG_STATE)
    Dst.ValueType = Src.ValueType
    Dst.Value = Src.Value
End Sub

Sub InvalidateVolatileDispatchRegs(RegState() As DISPATCH_REG_STATE)
    ClearDispatchReg RegState(0)
    ClearDispatchReg RegState(1)
    ClearDispatchReg RegState(2)
    ClearDispatchReg RegState(8)
    ClearDispatchReg RegState(9)
    ClearDispatchReg RegState(10)
    ClearDispatchReg RegState(11)
End Sub

Sub InvalidateUnknownInstructionDestination(ByVal u As ud Ptr, RegState() As DISPATCH_REG_STATE)
    If u = NULL Then Return

    Dim mnemonic As ud_mnemonic_code = ud_insn_mnemonic(u)
    If Not UdMnemonicWritesOp0(mnemonic) Then Return

    Dim op0 As Const ud_operand Ptr = ud_insn_opr(u, 0)
    If op0 = NULL OrElse op0->typ <> UD_OP_REG Then Return

    Dim regIndex As Integer = UdRegToGprIndex(op0->Base)
    If regIndex >= 0 Then ClearDispatchReg RegState(regIndex)
End Sub

Function DispatchFunctionVisited( _
    ByRef State As DISPATCH_SCAN_STATE, _
    ByVal ModuleBase As ULONG_PTR, _
    ByVal FunctionRva As ULong) As Boolean

    If State.VisitedCount = 0 Then Return False

    For i As ULong = 0 To State.VisitedCount - 1
        If State.Visited(i).ModuleBase = ModuleBase AndAlso _
           State.Visited(i).FunctionRva = FunctionRva Then
            Return True
        End If
    Next

    Return False
End Function

Function AddVisitedDispatchFunction( _
    ByRef State As DISPATCH_SCAN_STATE, _
    ByVal ModuleBase As ULONG_PTR, _
    ByVal FunctionRva As ULong) As Boolean

    If DispatchFunctionVisited(State, ModuleBase, FunctionRva) Then Return False
    If State.VisitedCount >= DISPATCH_MAX_VISITED Then Return False

    State.Visited(State.VisitedCount).ModuleBase = ModuleBase
    State.Visited(State.VisitedCount).FunctionRva = FunctionRva
    State.VisitedCount += 1

    Return True
End Function

Function IsTailForwardCall( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal callFoa As ULong) As Boolean

    Const MAX_TAIL_DISTANCE = &H30

    If pDump = NULL OrElse callFoa >= fileSize Then Return False

    Dim callRva As ULong = FoaToRva(pDump, callFoa)
    If callRva = 0 Then Return False

    Dim available As ULong = MAX_TAIL_DISTANCE

    If callFoa + available > fileSize Then
        available = fileSize - callFoa
    End If

    Dim u As ud
    ud_init(@u)
    ud_set_mode(@u, 64)
    ud_set_pc(@u, callRva)
    ud_set_input_buffer(@u, pDump + callFoa, available)

    '
    ' 第一条应该是候选 CALL 本身。
    '
    If ud_decode(@u) = 0 Then Return False
    If ud_insn_mnemonic(@u) <> UD_Icall Then Return False

    While ud_decode(@u) <> 0
        Select Case ud_insn_mnemonic(@u)
            Case UD_Iret, UD_Iretf
                Return True

            Case UD_Icall
                '
                ' 后面又发生 CALL，
                ' 就不是简单 tail forwarding。
                '
                Return False
        End Select
    Wend

    Return False
End Function

Function FindForwardedDriverEntryRVA( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal funcRva As ULong) As ULong

    Const FUNCTION_SCAN_SIZE = &H600
    Const MAX_CANDIDATES = 16

    Dim funcFoa As ULong = RvaToFoa(pDump, funcRva)
    If funcFoa = 0 OrElse funcFoa >= fileSize Then Return 0

    Dim scanSize As ULong = FUNCTION_SCAN_SIZE
    If funcFoa + scanSize > fileSize Then scanSize = fileSize - funcFoa

    Dim u As ud
    ud_init(@u)
    ud_set_mode(@u, 64)
    ud_set_pc(@u, funcRva)
    ud_set_input_buffer(@u, pDump + funcFoa, scanSize)

    Dim regValue(0 To 15) As Integer
    regValue(1) = ENTRY_REG_DRIVER_OBJECT
    regValue(2) = ENTRY_REG_REGISTRY_PATH

    Dim CandidateTarget(0 To MAX_CANDIDATES - 1) As ULong
    Dim CandidateCallRva(0 To MAX_CANDIDATES - 1) As ULong
    Dim CandidateCount As Integer = 0

    While ud_decode(@u) <> 0
        Dim mnemonic As ud_mnemonic_code = ud_insn_mnemonic(@u)
        Dim op0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
        Dim op1 As Const ud_operand Ptr = ud_insn_opr(@u, 1)

        Select Case mnemonic
            Case UD_Imov
                If UdIs64BitGpr(op0) Then
                    Dim dst As Integer = UdRegToGprIndex(op0->Base)

                    If UdIs64BitGpr(op1) Then
                        Dim src As Integer = UdRegToGprIndex(op1->Base)
                        regValue(dst) = regValue(src)
                    Else
                        regValue(dst) = ENTRY_REG_UNKNOWN
                    End If
                End If

            Case UD_Ilea
                If op0 <> NULL AndAlso op0->typ = UD_OP_REG Then
                    Dim dst As Integer = UdRegToGprIndex(op0->Base)
                    If dst >= 0 Then regValue(dst) = ENTRY_REG_UNKNOWN
                End If
                
            Case UD_Ijmp
                If op0 <> NULL AndAlso op0->typ = UD_OP_JIMM Then
                    Dim jumpTarget As ULong = _
                        UdRelativeTargetRva(@u, op0)

                    If CBool(jumpTarget <> 0) AndAlso _
                       CBool(jumpTarget <> funcRva) AndAlso _
                       CBool(regValue(1) = ENTRY_REG_DRIVER_OBJECT) AndAlso _
                       CBool(regValue(2) = ENTRY_REG_REGISTRY_PATH) AndAlso _
                       IsRvaExecutable(pDump, jumpTarget) Then

                        Return jumpTarget
                    End If
                End If

            Case UD_Icall
                Dim callTarget As ULong = UdRelativeTargetRva(@u, op0)
                Dim callRva As ULong = CULng(ud_insn_off(@u))

                If callTarget <> 0 AndAlso _
                   callTarget <> funcRva AndAlso _
                   RvaToFoa(pDump, callTarget) <> 0 AndAlso _
                   regValue(1) = ENTRY_REG_DRIVER_OBJECT AndAlso _
                   regValue(2) = ENTRY_REG_REGISTRY_PATH Then

                    If CandidateCount < MAX_CANDIDATES Then
                        CandidateTarget(CandidateCount) = callTarget
                        CandidateCallRva(CandidateCount) = callRva

                        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
                            "[Debug] Candidate#" & Str(CandidateCount) & _
                            " CallRVA=0x" & Hex(callRva) & _
                            " TargetRVA=0x" & Hex(callTarget)

                        CandidateCount += 1
                    End If
                End If

                regValue(0) = ENTRY_REG_UNKNOWN
                regValue(1) = ENTRY_REG_UNKNOWN
                regValue(2) = ENTRY_REG_UNKNOWN
                regValue(8) = ENTRY_REG_UNKNOWN
                regValue(9) = ENTRY_REG_UNKNOWN
                regValue(10) = ENTRY_REG_UNKNOWN
                regValue(11) = ENTRY_REG_UNKNOWN

            Case UD_Iret, UD_Iretf
                Exit While

            Case Else
                If UdMnemonicWritesOp0(mnemonic) AndAlso _
                   op0 <> NULL AndAlso _
                   op0->typ = UD_OP_REG Then

                    Dim dst As Integer = UdRegToGprIndex(op0->Base)
                    If dst >= 0 Then regValue(dst) = ENTRY_REG_UNKNOWN
                End If
        End Select
    Wend

    If CandidateCount = 0 Then
        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
            "[Debug] No forwarding candidates"
        Return 0
    End If

    '
    ' 1. 优先选择真正的 tail-forward：
    '    call candidate
    '    ...
    '    ret
    '
    ' 从后往前找，因为真正 DriverEntry 通常是 wrapper 尾部调用。
    '
    For i As Integer = CandidateCount - 1 To 0 Step -1
        Dim callFoa As ULong = RvaToFoa(pDump, CandidateCallRva(i))

        If callFoa <> 0 AndAlso _
           IsTailForwardCall(pDump, fileSize, callFoa) Then

            MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
                "[Debug] Accept tail forwarding Candidate#" & Str(i) & _
                " CallRVA=0x" & Hex(CandidateCallRva(i)) & _
                " TargetRVA=0x" & Hex(CandidateTarget(i))

            Return CandidateTarget(i)
        End If
    Next

    '
    ' 2. 所有候选指向同一个目标。
    '
    Dim sameTarget As Boolean = True

    For i As Integer = 1 To CandidateCount - 1
        If CandidateTarget(i) <> CandidateTarget(0) Then
            sameTarget = False
            Exit For
        End If
    Next

    If CandidateCount >= 2 AndAlso sameTarget Then
        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
            "[Debug] Accept common forwarding target Count=" & Str(CandidateCount) & _
            " RVA=0x" & Hex(CandidateTarget(0))

        Return CandidateTarget(0)
    End If

    '
    ' 3. 多个不同 target，但不存在明确 tail-forward。
    '
    ' 不再像旧版那样一见不同就立即失败。
    ' 最后一个候选通常最接近 wrapper 返回点，
    ' 但为了避免制造假阳性，这里暂时仍返回 0。
    '
    If CandidateCount = 1 Then
        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
            "[Debug] Single candidate rejected: function is not a forwarding wrapper" & _
            " CallRVA=0x" & Hex(CandidateCallRva(0)) & _
            " TargetRVA=0x" & Hex(CandidateTarget(0))
    Else
        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
            "[Debug] Ambiguous forwarding candidates Count=" & Str(CandidateCount)
    End If

    For i As Integer = 0 To CandidateCount - 1
        MyLog.PrintInfo "FindForwardedDriverEntryRVA", "", _
            "[Debug] Candidate#" & Str(i) & _
            " CallRVA=0x" & Hex(CandidateCallRva(i)) & _
            " TargetRVA=0x" & Hex(CandidateTarget(i))
    Next

    Return 0
End Function

Function ResolveRealDriverEntryRVA( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal entryRva As ULong) As ULong

    Const MAX_ENTRY_DEPTH = 4

    If pDump = 0 OrElse entryRva = 0 Then
        MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] Invalid input EntryRVA=0x" & Hex(entryRva)
        Return entryRva
    End If

    MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] Begin ImageEntryRVA=0x" & Hex(entryRva)

    Dim visited(0 To MAX_ENTRY_DEPTH) As ULong
    Dim currentRva As ULong = entryRva

    For depth As Integer = 0 To MAX_ENTRY_DEPTH - 1
        visited(depth) = currentRva

        MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", _
            "[Debug] Depth=" & Str(depth) & _
            " CurrentRVA=0x" & Hex(currentRva)

        Dim nextRva As ULong = FindForwardedDriverEntryRVA(pDump, fileSize, currentRva)

        MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", _
            "[Debug] Depth=" & Str(depth) & _
            " CurrentRVA=0x" & Hex(currentRva) & _
            " NextRVA=0x" & Hex(nextRva)

        If nextRva = 0 Then
            MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] No forwarding target, stop at RVA=0x" & Hex(currentRva)
            Exit For
        End If

        If nextRva = currentRva Then
            MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] Self target detected, stop"
            Exit For
        End If

        For i As Integer = 0 To depth
            If visited(i) = nextRva Then
                MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] Loop detected RVA=0x" & Hex(nextRva)
                Return currentRva
            End If
        Next

        currentRva = nextRva
    Next

    MyLog.PrintInfo "ResolveRealDriverEntryRVA", "", "[Debug] Final RealDriverEntryRVA=0x" & Hex(currentRva)

    Return currentRva
End Function