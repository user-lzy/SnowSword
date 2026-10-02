Private Function RecoverHidRegisterMinidriverDispatch( _
    ByRef HidContext As DISPATCH_SCAN_CONTEXT, _
    ByVal FunctionRva As ULong, _
    ByRef OutputDispatch As DISPATCH_RECOVERY_OUTPUT) As Boolean

    If HidContext.pDump = NULL OrElse _
       HidContext.FileSize = 0 Then

        Return False
    End If

    Dim funcBegin As ULong = 0
    Dim funcEnd As ULong = 0

    Dim scanSize As ULong = &H1000

    If GetRuntimeFunctionBounds( _
        HidContext.pDump, _
        HidContext.FileSize, _
        FunctionRva, _
        funcBegin, _
        funcEnd) Then
        If funcEnd > FunctionRva Then
            scanSize = funcEnd - FunctionRva
        End If
    End If

    Dim funcFoa As ULong = _
        RvaToFoa(HidContext.pDump, FunctionRva)

    If funcFoa = 0 OrElse _
       funcFoa >= HidContext.FileSize Then
        Return False
    End If

    If funcFoa + scanSize > HidContext.FileSize Then
        scanSize = HidContext.FileSize - funcFoa
    End If

    Dim HandlerRva(0 To 15) As ULong
    Dim HandlerValid(0 To 15) As Boolean

    For i As Integer = 0 To 15
        HandlerRva(i)=0
        HandlerValid(i)=False
    Next

    Dim u As ud

    ud_init(@u)
    ud_set_mode(@u,64)
    ud_set_pc(@u,FunctionRva)

    ud_set_input_buffer( _
        @u, _
        HidContext.pDump + funcFoa, _
        scanSize)

    While ud_decode(@u) <> 0
        Dim mnemonic As ud_mnemonic_code = _
            ud_insn_mnemonic(@u)

        Dim op0 As Const ud_operand Ptr = _
            ud_insn_opr(@u,0)

        Dim op1 As Const ud_operand Ptr = _
            ud_insn_opr(@u,1)


        Select Case mnemonic
        '
        ' =====================================================
        ' lea reg,[rip+handler]
        '
        ' 保存每一个寄存器里的 handler
        '
        ' =====================================================
        '
        Case UD_Ilea
            If op0 <> NULL AndAlso _
               op1 <> NULL Then
                If op0->typ = UD_OP_REG AndAlso _
                   op1->typ = UD_OP_MEM AndAlso _
                   op1->Base = UD_R_RIP Then
                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 AndAlso dstReg <= 15 Then

                        Dim targetRva As ULong = _
                            GetRipRelativeOperandRva( _
                                @u, _
                                op1)

                        If targetRva <> 0 AndAlso _
                           IsRvaExecutable( _
                               HidContext.pDump, _
                               targetRva) Then

                            HandlerRva(dstReg)=targetRva
                            HandlerValid(dstReg)=True

                            MyLog.PrintInfo _
                                "RecoverHidRegisterMinidriverDispatch", _
                                "", _
                                "[HID] LEA handler " & _
                                DispatchRegName(dstReg) & _
                                " RVA=0x" & Hex(targetRva)
                        End If
                    End If
                End If
            End If

        '
        ' =====================================================
        ' MOV [DriverObject+offset],reg
        '
        ' =====================================================
        '
        Case UD_Imov
            If op0 <> NULL AndAlso _
               op1 <> NULL Then

                If op0->typ = UD_OP_MEM AndAlso _
                   op1->typ = UD_OP_REG Then

                    Dim srcReg As Integer = _
                        UdRegToGprIndex(op1->Base)

                    If CBool(srcReg >= 0) AndAlso _
                       CBool(srcReg <= 15) AndAlso _
                       HandlerValid(srcReg) Then

                        Dim offset As LongInt = _
                            op0->offset


                        '
                        ' DRIVER_OBJECT MajorFunction
                        '
                        ' HID 内部最终仍然写 DriverObject
                        '
                        ' 0x70 开始
                        '
                        '
                        If offset >= &H70 AndAlso _
                           offset <= _
                           &H70 + _
                           IRP_MJ_MAXIMUM_FUNCTION * 8 Then

                            Dim irpIndex As Integer = _
                                CInt((offset - &H70) \ 8)

                            Dim Source As String = _
                                "HidRegisterMinidriver"

                            SetRecoveredDispatch _
                                OutputDispatch, _
                                irpIndex, _
                                HidContext.ImageBase, _
                                HandlerRva(srcReg), _
                                Source

                            MyLog.PrintInfo _
                                "RecoverHidRegisterMinidriverDispatch", _
                                "", _
                                "[HID] Dispatch[" & _
                                Str(irpIndex) & _
                                "] RVA=0x" & _
                                Hex(HandlerRva(srcReg))

                            HandlerValid(srcReg)=False
                            HandlerRva(srcReg)=0
                        End If
                    End If
                End If
            End If

        '
        ' =====================================================
        ' CALL
        '
        ' volatile registers invalid
        '
        ' =====================================================
        '
        Case UD_Icall
            For i As Integer = 0 To 15

                Select Case i
                    Case 0,1,2,8,9,10,11
                        HandlerValid(i)=False
                        HandlerRva(i)=0
                End Select
            Next

        Case Else
            '
            ' 普通写寄存器
            ' 覆盖旧 handler
            '
            If UdMnemonicWritesOp0(mnemonic) Then
                If op0 <> NULL Then
                    If op0->typ = UD_OP_REG Then
                        Dim dstReg As Integer = _
                            UdRegToGprIndex(op0->Base)

                        If dstReg >=0 AndAlso _
                           dstReg <=15 Then
                            HandlerValid(dstReg)=False
                            HandlerRva(dstReg)=0
                        End If
                    End If
                End If
            End If
        End Select

    Wend

    If OutputDispatch.HitCount > 0 Then
        Return True
    End If

    Return False

End Function