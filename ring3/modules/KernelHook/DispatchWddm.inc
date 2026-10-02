Private Sub ClearWddmBaselineCandidate( _
    ByRef Candidate As WDDM_BASELINE_CANDIDATE)

    Candidate.Valid = False

    Candidate.WriterFunctionRva = 0
    Candidate.BaseReg = -1

    Candidate.DriverObjectTracked = False

    Candidate.SequentialMajorFunction = 0
    Candidate.FirstMajorFunctionOffset = -1

    Candidate.HitCount = 0

    Candidate.MatchCount = 0
    Candidate.ExtraCount = 0
    Candidate.MissingCount = 0

    Candidate.Score = 0
    Candidate.ExactCount = 0
    Candidate.ConflictCount = 0
    Candidate.HookedCount = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        Candidate.MajorFunction(i).Valid = False
        Candidate.MajorFunction(i).ModuleBase = 0
        Candidate.MajorFunction(i).FunctionRva = 0

        Candidate.MajorFunction(i).State = _
            DispatchSlotUnresolved

        Candidate.MajorFunction(i).OwnerKind = _
            DispatchOwnerUnknown

        Candidate.MajorFunction(i).Source = _
            DispatchSourceUnknown

        Candidate.MajorFunction(i).Confidence = 0

    Next

End Sub

Private Function WddmMajorOffsetToIndex( _
    ByVal Offset As LongInt, _
    ByRef IrpIndex As Integer) As Boolean

    IrpIndex = -1

    If Offset < DISPATCH_BASE Then
        Return False
    End If

    Dim maxOffset As LongInt = _
        DISPATCH_BASE + _
        CLngInt(IRP_MJ_MAXIMUM_FUNCTION) * _
        DISPATCH_STEP

    If Offset > maxOffset Then
        Return False
    End If

    Dim delta As LongInt = _
        Offset - DISPATCH_BASE

    If (delta Mod DISPATCH_STEP) <> 0 Then
        Return False
    End If

    Dim index64 As LongInt = _
        delta \ DISPATCH_STEP

    If index64 < 0 OrElse _
       index64 > IRP_MJ_MAXIMUM_FUNCTION Then

        Return False
    End If

    IrpIndex = CInt(index64)

    Return True
End Function

Sub ClearWddmDispatchBaseline( _
    ByRef Baseline As WDDM_DISPATCH_BASELINE_OUTPUT)

    Baseline.HitCount = 0
    Baseline.Confidence = 0
    Baseline.DxgkrnlBase = 0
    Baseline.WriterFunctionRva = 0
    Baseline.CandidateBaseReg = -1
    Baseline.HasUniformDefault = False
    Baseline.DefaultRva = 0
    Baseline.DefaultSlotCount = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        Baseline.MajorFunction(i).Valid = False
        Baseline.MajorFunction(i).ModuleBase = 0
        Baseline.MajorFunction(i).FunctionRva = 0
        Baseline.MajorFunction(i).State = _
            DispatchSlotUnresolved
        Baseline.MajorFunction(i).OwnerKind = _
            DispatchOwnerUnknown
        Baseline.MajorFunction(i).Source = _
            DispatchSourceUnknown
        Baseline.MajorFunction(i).Confidence = 0
    Next
End Sub

Private Function SameWddmBaselineTargets( _
    ByRef A As WDDM_BASELINE_CANDIDATE, _
    ByRef B As WDDM_BASELINE_CANDIDATE) As Boolean

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        If A.MajorFunction(i).Valid <> _
           B.MajorFunction(i).Valid Then
            Return False
        End If

        If A.MajorFunction(i).Valid Then
            If A.MajorFunction(i).FunctionRva <> _
               B.MajorFunction(i).FunctionRva Then

                Return False
            End If
        End If
    Next

    Return True
End Function

Private Function ScoreWddmBaselineTieBreaker( _
    ByRef Candidate As WDDM_BASELINE_CANDIDATE) As Long

    Dim score As Long = 0

    '1. DriverObject寄存器匹配
    If Candidate.BaseReg = UD_R_RCX OrElse _
       Candidate.DriverObjectTracked Then

        score += 10000
    End If


    '2. MajorFunction连续写入
    If Candidate.SequentialMajorFunction >= 28 Then
        score += 5000
    Else
        score += Candidate.SequentialMajorFunction * 100
    End If


    '3. 第一个slot是否为 MajorFunction[0]
    If Candidate.FirstMajorFunctionOffset = &H70 Then
        score += 2000
    End If


    '4. 写入数量
    score += Candidate.MatchCount * 10


    Return score

End Function

Private Sub ConsiderWddmBaselineCandidate( _
    ByRef Candidate As WDDM_BASELINE_CANDIDATE, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByVal DxgRuntimeBase As ULONG_PTR, _
    ByRef BestCandidate As WDDM_BASELINE_CANDIDATE, _
    ByRef Ambiguous As Boolean)

    If ScoreWddmBaselineCandidate( _
        Candidate, _
        RecoverOutput, _
        Context, _
        DxgRuntimeBase) = False Then

        Exit Sub
    End If

    Candidate.Valid = True

    If BestCandidate.Valid = False Then
        BestCandidate = Candidate
        Ambiguous = False
        Exit Sub
    End If

    '
    ' 注意:Candidate.Score 已包含 tie-breaker,
    ' 这里不再二次相加。
    '
    Dim CandidateTotalScore As Long = Candidate.Score
    Dim BestTotalScore     As Long = BestCandidate.Score

    If CandidateTotalScore > BestTotalScore Then
        BestCandidate = Candidate
        Ambiguous = False
        Exit Sub
    End If

    If CandidateTotalScore < BestTotalScore Then
        Exit Sub
    End If

    '
    ' 同分时优先 extra 更少。
    '
    If Candidate.ExtraCount < BestCandidate.ExtraCount Then
        BestCandidate = Candidate
        Ambiguous = False
        Exit Sub
    End If

    If Candidate.ExtraCount > BestCandidate.ExtraCount Then
        Exit Sub
    End If

    '
    ' 再优先 missing 更少。
    '
    If Candidate.MissingCount < BestCandidate.MissingCount Then
        BestCandidate = Candidate
        Ambiguous = False
        Exit Sub
    End If

    If Candidate.MissingCount > BestCandidate.MissingCount Then
        Exit Sub
    End If

    '
    ' 同 score / same shape,但 target 集不同,则不能武断选择。
    '
    If SameWddmBaselineTargets( _
        Candidate, _
        BestCandidate) = False Then

        Ambiguous = True

        MyLog.PrintInfo _
            "ConsiderWddmBaselineCandidate", "", _
            "[WDDM-BASELINE] Ambiguous equal-score candidates" & _
            " A=0x" & Hex(BestCandidate.WriterFunctionRva) & _
            " B=0x" & Hex(Candidate.WriterFunctionRva) & _
            " Score=" & Trim(Str(Candidate.Score))
    End If
End Sub

' ============================================================
' ApplyWddmUniformDefault
'
' 对 candidate 未覆盖、但 current ∈ dxgkrnl 的槽位取 RVA 众数:
'   众数出现 >= 5 次且 RVA 可执行
'     → RVA == 众数的槽位恢复为 FrameworkDefault (Confidence 85)
'     → RVA != 众数的槽位保持 FrameworkOwned (疑似 dxgkrnl 内 hook)
'   current 不在 dxgkrnl 的槽位 一律不覆盖。
'
' 安全性:填入值 == 该槽位 current,不改变地址,
' 仅将 "UNKNOWN" 升级为 "框架合法"。
' ============================================================
Private Sub ApplyWddmUniformDefault( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef DxgContext As DISPATCH_SCAN_CONTEXT, _
    ByRef Candidate As WDDM_BASELINE_CANDIDATE, _
    ByRef Baseline As WDDM_DISPATCH_BASELINE_OUTPUT)

    Const WDDM_UNIFORM_DEFAULT_MIN_SLOTS As ULong = 5

    Dim histRva(0 To IRP_MJ_MAXIMUM_FUNCTION) As ULong
    Dim histCnt(0 To IRP_MJ_MAXIMUM_FUNCTION) As ULong
    Dim histN As Integer = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        If Candidate.MajorFunction(i).Valid Then Continue For

        Dim curAddr As ULONG_PTR = Context.CurrentMajorFunction(i)

        If curAddr = 0 Then Continue For
        If curAddr = Context.IopInvalidDeviceRequest Then Continue For
        If IsAddressInModule(curAddr, "dxgkrnl.sys") = False Then Continue For

        Dim curRva As ULong = CULng(curAddr - DxgContext.ImageBase)

        Dim found As Boolean = False

        For j As Integer = 0 To histN - 1
            If histRva(j) = curRva Then
                histCnt(j) += 1
                found = True
                Exit For
            End If
        Next

        If found = False Then
            If histN <= IRP_MJ_MAXIMUM_FUNCTION Then
                histRva(histN) = curRva
                histCnt(histN) = 1
                histN += 1
            End If
        End If
    Next

    If histN = 0 Then Exit Sub

    Dim bestIdx As Integer = -1
    Dim bestCnt As ULong = 0

    For j As Integer = 0 To histN - 1
        If histCnt(j) > bestCnt Then
            bestCnt = histCnt(j)
            bestIdx = j
        End If
    Next

    If bestIdx < 0 Then Exit Sub
    If bestCnt < WDDM_UNIFORM_DEFAULT_MIN_SLOTS Then Exit Sub

    Dim defaultRva As ULong = histRva(bestIdx)

    If IsRvaExecutable(DxgContext.pDump, defaultRva) = False Then
        MyLog.PrintInfo "ApplyWddmUniformDefault", "", _
            "[WDDM-BASELINE] Uniform default rejected" & _
            " RVA=0x" & Hex(defaultRva) & _
            " Count=" & Trim(Str(bestCnt)) & _
            " Reason=NotExecutable"
        Exit Sub
    End If

    Dim filledCount As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        If Candidate.MajorFunction(i).Valid Then Continue For

        Dim curAddr As ULONG_PTR = Context.CurrentMajorFunction(i)

        If curAddr = 0 Then Continue For
        If curAddr = Context.IopInvalidDeviceRequest Then Continue For
        If IsAddressInModule(curAddr, "dxgkrnl.sys") = False Then Continue For

        If CULng(curAddr - DxgContext.ImageBase) <> defaultRva Then

            '
            ' current 在 dxgkrnl 内但偏离众数:
            ' 保留 FrameworkOwned,凸显为可疑槽位。
            '
            MyLog.PrintInfo "ApplyWddmUniformDefault", "", _
                "[WDDM-BASELINE] NonDefault slot retained" & _
                " Slot=" & IRP_MJ_ToString(i) & _
                " RVA=0x" & Hex(CULng(curAddr - DxgContext.ImageBase))
            Continue For
        End If

        Baseline.MajorFunction(i).Valid = True
        Baseline.MajorFunction(i).ModuleBase = DxgContext.ImageBase
        Baseline.MajorFunction(i).FunctionRva = defaultRva
        Baseline.MajorFunction(i).State = DispatchSlotRecovered
        Baseline.MajorFunction(i).OwnerKind = DispatchOwnerFramework
        Baseline.MajorFunction(i).Source = DispatchSourceFrameworkDefault
        Baseline.MajorFunction(i).Confidence = 85

        filledCount += 1

        MyLog.PrintInfo "ApplyWddmUniformDefault", "", _
            "[WDDM-BASELINE] " & IRP_MJ_ToString(i) & _
            " UniformDefault RVA=0x" & Hex(defaultRva) & _
            " VA=0x" & Hex(DxgContext.ImageBase + defaultRva)
    Next

    If filledCount > 0 Then
        Baseline.HasUniformDefault = True
        Baseline.DefaultRva = defaultRva
        Baseline.DefaultSlotCount = filledCount

        MyLog.PrintInfo "ApplyWddmUniformDefault", "", _
            "[WDDM-BASELINE] Uniform default accepted" & _
            " RVA=0x" & Hex(defaultRva) & _
            " Distinct=" & Trim(Str(histN)) & _
            " Filled=" & Trim(Str(filledCount))
    End If
End Sub

Private Function IsKnownKmdodVersion( _
    ByVal Value As ULong) As Boolean

    Select Case Value
        Case &H1052, &H1053, _
             &H2005, _
             &H300E, _
             &H4002, &H4003, _
             &H5023, _
             &H6003, _
             &H6010, &H6011, _
             &H700A, _
             &H8001, _
             &H9006, _
             &HA00B, _
             &HB004, _
             &HC004, _
             &HD001, _
             &HE003, _
             &HF003, _
             &H10004
            Return True
    End Select

    If Value >= &H11007 AndAlso _
       Value < &H100000 Then
        Return True
    End If

    Return False
End Function

' ============================================================
' DetectWddmDriver
'
' 输入：
'   Context  — 扫描上下文（含 pDump / FileSize / ModuleName 等）
' 输出：
'   Info     — DRIVER_MODEL_INFO，IsWddm / Confidence / Reason /
'              FrameworkOwner
'
' 证据来源（一次汇总，不再二次分类）：
'   Evidence 1   ScanWddmReachableFunction → 0x23003F / 0x230043
'   Evidence 2   ScanWddmReachableFunction → 间接注册路径
'   Evidence 3   ScanCallbackTableReferences → 辅助证据
'   Evidence 4   ScanForDxgkrnlImport      → 加分项（非否决项）
'
' 评分：
'   +30  reachable helper 中发现 0x23003F / 0x230043
'   +30  registration constant 附近存在 image-local callback
'        或间接调用路径
'   +20  callback table 结构合理
'   +20  direct dxgkrnl import
'
' 硬约束（不满足直接拒绝，避免阈值修补导致误判）：
'   必须 HasRegistrationConstant 或 ExplicitDxgkRegistration
'   —— 仅凭 callback table 不判 WDDM。
'
' 判定阈值：Confidence >= 70
'
' 注意：不做 DxgkDdi* 解析，只做模块级识别。
' ============================================================
Function DetectWddmDriver( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef Info As DRIVER_MODEL_INFO) As Boolean

    Info.IsWddm         = False
    Info.Confidence     = 0
    Info.Reason         = ""
    Info.FrameworkOwner = ""

    If Context.pDump = NULL OrElse Context.FileSize = 0 Then Return False

    Dim entryRva As ULong = GetImageEntryPointRVA(Context.pDump)
    If entryRva = 0 Then Return False

    ' ---------------------------------------------------------
    ' Evidence 收集
    ' ---------------------------------------------------------
    Dim Evidence As WDDM_DETECTION_EVIDENCE
    Evidence.Has23003F                 = False
    Evidence.Has230043                 = False
    Evidence.HasLargeSelfCallbackTable = False
    Evidence.CallbackRun               = 0
    Evidence.CallbackTableRva          = 0
    Evidence.HasIndirectRegistrationPath = False
    Evidence.RegistrationHelperRva     = 0
    Evidence.Confidence                = 0

    Evidence.HasKmdodInitialization = False
    Evidence.KmdodVersion = 0
    Evidence.KmdodHelperRva = 0
    Evidence.KmdodCallbackCount = 0
    
    ' --- Evidence 1 + 2：reachable helper 扫描 ---
    Dim reachState As DISPATCH_SCAN_STATE
    memset(@reachState, 0, SizeOf(reachState))

    ScanWddmReachableFunction _
        Context, reachState, entryRva, 0, Evidence

    Dim hasRegistrationConstant As Boolean = _
        Evidence.Has23003F OrElse Evidence.Has230043

    ' --- Evidence 3：callback table（辅助） ---
    Dim cbHits     As ULong = 0
    Dim cbTableRva As ULong = 0

    Dim hasCallbackTable As Boolean = _
        ScanCallbackTableReferences(Context, cbHits, cbTableRva)

    Evidence.CallbackRun               = cbHits
    Evidence.CallbackTableRva          = cbTableRva
    Evidence.HasLargeSelfCallbackTable = hasCallbackTable

    ' --- Evidence 4：dxgkrnl 导入（加分） ---
    Dim hasDxgkrnl As Boolean = ScanForDxgkrnlImport(Context)

    ' ---------------------------------------------------------
    ' 硬约束：不能仅凭 callback table 判 WDDM
    ' ---------------------------------------------------------
    Dim hasExplicitDxgkReg As Boolean = hasDxgkrnl
    
    Dim hasRegistrationEvidence As Boolean = _
        hasRegistrationConstant OrElse _
        hasExplicitDxgkReg OrElse _
        Evidence.HasKmdodInitialization

    If hasRegistrationEvidence = False Then
        Info.Confidence = 0
        Info.Reason = "NoRegistrationEvidence"

        MyLog.PrintInfo "DetectWddmDriver", "", _
            "[WDDM] Rejected: no registration evidence" & _
            " CallbackTable=" & Trim(Str(cbHits))

        Return False
    End If

    ' ---------------------------------------------------------
    ' 评分
    ' ---------------------------------------------------------
    Dim confidence  As ULong = 0
    Dim reasonParts As String = ""

    ' 1) registration constant（必给，硬约束已确认至少一项）
    If hasRegistrationConstant Then
        confidence += 30

        If Evidence.Has23003F Then
            reasonParts &= IIf(Len(reasonParts) > 0, "+", "") & _
                "RegValue 0x23003F"
        End If
        If Evidence.Has230043 Then
            reasonParts &= IIf(Len(reasonParts) > 0, "+", "") & _
                "RegValue 0x230043"
        End If

        MyLog.PrintInfo "DetectWddmDriver", "", _
            "[WDDM-EVIDENCE] RegistrationConstant=true" & _
            " has23003F=" & Str(Evidence.Has23003F) & _
            " has230043=" & Str(Evidence.Has230043) & _
            " HelperRVA=0x" & Hex(Evidence.RegistrationHelperRva)
    End If

    ' 2) 间接注册路径
    If Evidence.HasIndirectRegistrationPath Then
        confidence += 30
        reasonParts &= IIf(Len(reasonParts) > 0, "+", "") & _
            "IndirectRegistrationPath"

        MyLog.PrintInfo "DetectWddmDriver", "", _
            "[WDDM-EVIDENCE] IndirectRegistrationPath=true"
    End If

    If Evidence.HasKmdodInitialization Then
        confidence += 70

        reasonParts &= _
            IIf(Len(reasonParts) > 0, "+", "") & _
            "KMDOD(0x" & _
            Hex(Evidence.KmdodVersion) & _
            "," & _
            Trim(Str(Evidence.KmdodCallbackCount)) & _
            " callbacks)"

        MyLog.PrintInfo "DetectWddmDriver", "", _
            "[WDDM-EVIDENCE] KMDOD initialization=true" & _
            " Version=0x" & Hex(Evidence.KmdodVersion) & _
            " HelperRVA=0x" & Hex(Evidence.KmdodHelperRva) & _
            " Callbacks=" & _
                Trim(Str(Evidence.KmdodCallbackCount))
    End If

    ' 3) callback table（辅助，不单独判 WDDM）
    If hasCallbackTable Then
        confidence += 20
        reasonParts &= IIf(Len(reasonParts) > 0, "+", "") & _
            "CallbackTable(" & Trim(Str(cbHits)) & ")"

        MyLog.PrintInfo "DetectWddmDriver", "", _
            "[WDDM] CallbackTable run=" & Trim(Str(cbHits)) & _
            " TableStartRVA=0x" & Hex(cbTableRva)
    End If

    ' 4) dxgkrnl import（加分）
    If hasDxgkrnl Then
        confidence += 20
        reasonParts &= IIf(Len(reasonParts) > 0, "+", "") & "dxgkrnl"
    End If

    If confidence > 100 Then confidence = 100

    Info.Confidence = CInt(confidence)
    Info.Reason     = reasonParts

    ' 判定阈值 70
    If confidence >= 70 Then
        Info.IsWddm         = True
        Info.FrameworkOwner = "dxgkrnl.sys"
    End If

    MyLog.PrintInfo "DetectWddmDriver", "", _
        "[WDDM] Detected=" & Str(Info.IsWddm) & _
        " Confidence=" & Trim(Str(Info.Confidence)) & _
        " RegistrationConstant=" & Str(hasRegistrationConstant) & _
        " IndirectRegistrationPath=" & _
            Str(Evidence.HasIndirectRegistrationPath) & _
        " CallbackRun=" & Trim(Str(cbHits)) & _
        " DxgkrnlImport=" & Str(hasDxgkrnl) & _
        " Reason=" & Info.Reason

    Return Info.IsWddm
End Function

Sub ScanWddmReachableFunction( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef State As DISPATCH_SCAN_STATE, _
    ByVal FunctionRva As ULong, _
    ByVal Depth As Integer, _
    ByRef Evidence As WDDM_DETECTION_EVIDENCE)

    Const WDDM_REACH_MAX_DEPTH As Integer = 3
    Const WDDM_REACH_FALLBACK_SCAN_SIZE As ULong = &H800

    If Context.pDump = NULL OrElse Context.FileSize = 0 Then Exit Sub
    If FunctionRva = 0 Then Exit Sub
    If Depth > WDDM_REACH_MAX_DEPTH Then Exit Sub

    If AddVisitedDispatchFunction( _
        State, Context.ImageBase, FunctionRva) = False Then
        Exit Sub
    End If

    Dim funcFoa As ULong = RvaToFoa(Context.pDump, FunctionRva)
    If funcFoa = 0 OrElse funcFoa >= Context.FileSize Then Exit Sub

    '
    ' 优先使用 PDATA 中的真实函数边界。
    ' 不能像旧版本一样从每个函数起点无条件扫 0x2000，
    ' 否则会跨进后续函数并产生大量伪 call edge。
    '
    Dim scanSize As ULong = WDDM_REACH_FALLBACK_SCAN_SIZE
    Dim funcBegin As ULong = 0
    Dim funcEnd As ULong = 0

    If GetRuntimeFunctionBounds( _
        Context.pDump, _
        Context.FileSize, _
        FunctionRva, _
        funcBegin, _
        funcEnd) Then

        If funcEnd > FunctionRva Then
            scanSize = funcEnd - FunctionRva
        End If
    End If

    If scanSize = 0 Then Exit Sub

    If funcFoa + scanSize > Context.FileSize Then
        scanSize = Context.FileSize - funcFoa
    End If

    MyLog.PrintInfo "ScanWddmReachableFunction", "", _
        "[WDDM-SCAN] Depth=" & Str(Depth) & _
        " FunctionRVA=0x" & Hex(FunctionRva) & _
        " ScanSize=0x" & Hex(scanSize)

    Dim u As ud
    ud_init(@u)
    ud_set_mode(@u, 64)
    ud_set_pc(@u, FunctionRva)
    ud_set_input_buffer(@u, Context.pDump + funcFoa, scanSize)

    '
    ' WDDM detection 不需要完整通用数据流，
    ' 这里只追踪 DriverObject 是否仍然存在于某个 GPR 中。
    '
    Dim RegState(0 To 15) As DISPATCH_REG_STATE

    For i As Integer = 0 To 15
        ClearDispatchReg RegState(i)
    Next

    '
    ' 入口约定：
    ' 只有当调用者确认 RCX == DriverObject 时才会递归进入本函数。
    ' 根 DriverEntry 同样满足这一约定。
    '
    RegState(1).ValueType = DispatchRegDriverObject

    Dim funcLocalCallbackCount As Integer = 0
    Dim funcIndirectCalls As Integer = 0
    Dim funcHasConst As Boolean = False
    Dim funcConstValue As ULong = 0
    Dim funcHasKmdodVersion As Boolean = False
    Dim funcKmdodVersion As ULong = 0

    While ud_decode(@u) <> 0
        Dim mnemonic As ud_mnemonic_code = ud_insn_mnemonic(@u)
        Dim op0 As Const ud_operand Ptr = ud_insn_opr(@u, 0)
        Dim op1 As Const ud_operand Ptr = ud_insn_opr(@u, 1)

        Select Case mnemonic

            Case UD_Imov
                '
                ' 先处理 DriverObject 寄存器传播。
                '
                If op0 <> NULL AndAlso op0->typ = UD_OP_REG Then
                    Dim dstReg As Integer = UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        If UdIs64BitGpr(op1) Then
                            Dim srcReg As Integer = _
                                UdRegToGprIndex(op1->Base)

                            If srcReg >= 0 Then
                                CopyDispatchReg _
                                    RegState(dstReg), _
                                    RegState(srcReg)
                            Else
                                ClearDispatchReg RegState(dstReg)
                            End If
                        Else
                            ClearDispatchReg RegState(dstReg)
                        End If
                    End If
                End If

                '
                ' 再独立处理 WDDM registration constant。
                '
                If op0 <> NULL AndAlso op1 <> NULL Then
                    If op0->typ = UD_OP_REG AndAlso _
                       op1->typ = UD_OP_IMM Then

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

                        Dim low32 As ULong = _
                            CULng(immValue And &HFFFFFFFFULL)

                        Dim dstReg As Integer = _
                            UdRegToGprIndex(op0->Base)

                        If low32 = &H23003F Then
                            Evidence.Has23003F = True
                            Evidence.RegistrationHelperRva = FunctionRva
                            funcHasConst = True
                            funcConstValue = &H23003F

                            MyLog.PrintInfo _
                                "ScanWddmReachableFunction", "", _
                                "[WDDM-EVIDENCE]" & _
                                " HelperRVA=0x" & Hex(FunctionRva) & _
                                " RegValue=0x23003F" & _
                                " Reg=" & DispatchRegName(dstReg) & _
                                " InsnRVA=0x" & _
                                    Hex(ud_insn_off(@u))

                        ElseIf low32 = &H230043 Then
                            Evidence.Has230043 = True
                            Evidence.RegistrationHelperRva = FunctionRva
                            funcHasConst = True
                            funcConstValue = &H230043

                            MyLog.PrintInfo _
                                "ScanWddmReachableFunction", "", _
                                "[WDDM-EVIDENCE]" & _
                                " HelperRVA=0x" & Hex(FunctionRva) & _
                                " RegValue=0x230043" & _
                                " Reg=" & DispatchRegName(dstReg) & _
                                " InsnRVA=0x" & _
                                    Hex(ud_insn_off(@u))
                        End If
                    End If
                End If
                
                '
                ' KMDOD_INITIALIZATION_DATA.Version 通常是：
                '
                '   mov [rsp/rbp + local.Version], imm32
                '
                If op0 <> NULL AndAlso op1 <> NULL Then
                    If op0->typ = UD_OP_MEM AndAlso _
                       op1->typ = UD_OP_IMM Then

                        If op0->Base = UD_R_RSP OrElse _
                           op0->Base = UD_R_RBP Then

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

                            Dim low32 As ULong = _
                                CULng(immValue And &HFFFFFFFFULL)

                            If IsKnownKmdodVersion(low32) Then
                                funcHasKmdodVersion = True
                                funcKmdodVersion = low32

                                MyLog.PrintInfo _
                                    "ScanWddmReachableFunction", "", _
                                    "[WDDM-EVIDENCE] KMDOD version candidate" & _
                                    " HelperRVA=0x" & Hex(FunctionRva) & _
                                    " Version=0x" & Hex(low32) & _
                                    " InsnRVA=0x" & Hex(ud_insn_off(@u))
                            End If
                        End If
                    End If
                End If

            Case UD_Ilea
                If op0 <> NULL AndAlso op0->typ = UD_OP_REG Then
                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        '
                        ' LEA 一般产生地址，不再视为原 DriverObject。
                        '
                        ClearDispatchReg RegState(dstReg)
                    End If
                End If

                If op0 <> NULL AndAlso op1 <> NULL Then
                    If op0->typ = UD_OP_REG AndAlso _
                       op1->typ = UD_OP_MEM AndAlso _
                       op1->Base = UD_R_RIP AndAlso _
                       op1->index = UD_NONE Then

                        Dim tgtRva As ULong = _
                            GetRipRelativeOperandRva(@u, op1)

                        If tgtRva <> 0 AndAlso _
                           IsRvaExecutable( _
                               Context.pDump, tgtRva) Then

                            funcLocalCallbackCount += 1

                            Dim dstReg As Integer = _
                                UdRegToGprIndex(op0->Base)

                            MyLog.PrintInfo _
                                "ScanWddmReachableFunction", "", _
                                "[WDDM-EVIDENCE] ImageLocalFnPtr" & _
                                " HelperRVA=0x" & Hex(FunctionRva) & _
                                " Reg=" & DispatchRegName(dstReg) & _
                                " TargetRVA=0x" & Hex(tgtRva)
                        End If
                    End If
                End If

            Case UD_Icall
                '
                ' 关键修复：
                '
                ' 只有 RCX 当前仍然是 DriverObject，
                ' 才沿 direct call 递归。
                '
                Dim passDriverObject As Boolean = _
                    (RegState(1).ValueType = _
                        DispatchRegDriverObject)

                If op0 <> NULL Then
                    If op0->typ = UD_OP_JIMM Then
                        Dim callTarget As ULong = _
                            UdRelativeTargetRva(@u, op0)

                        If passDriverObject AndAlso _
                           CBool(callTarget <> 0) AndAlso _
                           CBool(callTarget <> FunctionRva) AndAlso _
                           CBool(Depth < WDDM_REACH_MAX_DEPTH) AndAlso _
                           CBool(RvaToFoa( _
                               Context.pDump, _
                               callTarget) <> 0) AndAlso _
                           IsRvaExecutable( _
                               Context.pDump, _
                               callTarget) Then

                            MyLog.PrintInfo _
                                "ScanWddmReachableFunction", "", _
                                "[WDDM-CALL]" & _
                                " Caller=0x" & Hex(FunctionRva) & _
                                " Target=0x" & Hex(callTarget) & _
                                " DriverObject=true"

                            ScanWddmReachableFunction _
                                Context, _
                                State, _
                                callTarget, _
                                Depth + 1, _
                                Evidence
                        End If

                    Else
                        '
                        ' 对间接调用本身只收证据。
                        '
                        funcIndirectCalls += 1
                    End If
                End If

                '
                ' Windows x64 volatile registers 在调用后失效。
                '
                InvalidateVolatileDispatchRegs RegState()

            Case UD_Iret, UD_Iretf
                '
                ' 已经限制到真实函数边界，
                ' 遇到 RET 就可以结束。
                '
                Exit While

        End Select
    Wend

    If funcHasConst Then
        If funcLocalCallbackCount > 0 OrElse _
           funcIndirectCalls > 0 Then

            Evidence.HasIndirectRegistrationPath = True

            MyLog.PrintInfo _
                "ScanWddmReachableFunction", "", _
                "[WDDM-EVIDENCE]" & _
                " IndirectRegistrationPath" & _
                " HelperRVA=0x" & Hex(FunctionRva) & _
                " Const=0x" & Hex(funcConstValue) & _
                " LocalLeas=" & _
                    Trim(Str(funcLocalCallbackCount)) & _
                " IndirectCalls=" & _
                    Trim(Str(funcIndirectCalls))
        End If
    End If
    
    If funcHasKmdodVersion AndAlso _
       funcLocalCallbackCount >= 2 Then

        Evidence.HasKmdodInitialization = True
        Evidence.KmdodVersion = funcKmdodVersion
        Evidence.KmdodHelperRva = FunctionRva
        Evidence.KmdodCallbackCount = funcLocalCallbackCount

        MyLog.PrintInfo _
            "ScanWddmReachableFunction", "", _
            "[WDDM-EVIDENCE] KMDOD initialization" & _
            " HelperRVA=0x" & Hex(FunctionRva) & _
            " Version=0x" & Hex(funcKmdodVersion) & _
            " LocalCallbacks=" & _
                Trim(Str(funcLocalCallbackCount))
    End If
End Sub

' ============================================================
' 扫描已初始化数据段，寻找连续的可执行函数 RVA。
' 判定条件：连续 ≥ 3 个 entry 属于当前映像可执行节。
' 返回最长连续 run 的长度和起始 RVA。
' ============================================================
Function ScanCallbackTableReferences( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef CallbackHits As ULong, _
    ByRef TableStartRva As ULong) As Boolean

    Const IMAGE_SCN_MEM_EXECUTE_FLAG       As ULong = &H20000000
    Const IMAGE_SCN_CNT_INITIALIZED_DATA_L As ULong = &H00000040

    CallbackHits = 0
    TableStartRva = 0

    If Context.pDump = NULL OrElse Context.FileSize = 0 Then Return False

    Dim preferredBase As ULONG_PTR = 0
    Dim sizeOfImage   As ULong = 0

    If GetPeImageInfo( _
        Context.pDump, _
        preferredBase, _
        sizeOfImage) = False Then
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, Context.pDump)

    If dos->e_magic <> &H5A4D Then Return False

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, Context.pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 Then Return False

    Dim secHdr As IMAGE_SECTION_HEADER Ptr = _
        Cast(IMAGE_SECTION_HEADER Ptr, _
             Cast(ULONG_PTR, nt64) + SizeOf(IMAGE_NT_HEADERS64))

    Dim bestRun   As ULong = 0
    Dim bestStart As ULong = 0

    For secIdx As ULong = 0 To nt64->FileHeader.NumberOfSections - 1
        Dim ch As ULong = secHdr->Characteristics

        ' 只看已初始化数据段，排除 .text
        If (ch And IMAGE_SCN_MEM_EXECUTE_FLAG) = 0 AndAlso _
           (ch And IMAGE_SCN_CNT_INITIALIZED_DATA_L) <> 0 Then

            Dim rawStart As ULong = secHdr->PointerToRawData
            Dim rawSize  As ULong = secHdr->SizeOfRawData

            If rawSize >= 8 AndAlso _
               rawStart + rawSize <= Context.FileSize Then

                Dim pSec As UByte Ptr = Context.pDump + rawStart
                Dim numEntries As ULong = rawSize \ 8

                Dim currentRun      As ULong = 0
                Dim currentRunStart As ULong = 0

                For i As ULong = 0 To numEntries - 1
                    Dim nVal As ULONG_PTR = 0
                    memcpy(@nVal, pSec + i * 8, 8)

                    Dim isExec As Boolean = False

                    ' 判定：VA 落在 preferred image 且 RVA 可执行
                    If nVal >= preferredBase AndAlso _
                       nVal < preferredBase + sizeOfImage Then

                        Dim candidateRva As ULong = _
                            CULng(nVal - preferredBase)

                        If IsRvaExecutable( _
                            Context.pDump, _
                            candidateRva) Then
                            isExec = True
                        End If
                    End If

                    If isExec Then
                        If currentRun = 0 Then
                            currentRunStart = _
                                secHdr->VirtualAddress + i * 8
                        End If
                        currentRun += 1
                    Else
                        If currentRun > bestRun Then
                            bestRun = currentRun
                            bestStart = currentRunStart
                        End If
                        currentRun = 0
                    End If
                Next i

                If currentRun > bestRun Then
                    bestRun   = currentRun
                    bestStart = currentRunStart
                End If
            End If
        End If

        secHdr += 1
    Next secIdx

    CallbackHits  = bestRun
    TableStartRva = bestStart

    Return bestRun >= 3
End Function

' ============================================================
' 判断导入表是否引用 dxgkrnl.sys
' 只读 Import Directory，不解析 IAT 槽位，不做 DxgkDdi 解析。
' ============================================================
Function ScanForDxgkrnlImport( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT) As Boolean

    Const IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX = 1
    Const IMAGE_IMPORT_DESCRIPTOR_SIZE = 20

    If Context.pDump = NULL OrElse Context.FileSize = 0 Then Return False

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, Context.pDump)

    If dos->e_magic <> &H5A4D Then Return False
    If dos->e_lfanew <= 0 Then Return False
    If CLngInt(dos->e_lfanew) + SizeOf(IMAGE_NT_HEADERS64) > Context.FileSize Then
        Return False
    End If

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, Context.pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 Then Return False

    Dim importRva As ULong = _
        nt64->OptionalHeader.DataDirectory( _
            IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX).VirtualAddress

    Dim importSize As ULong = _
        nt64->OptionalHeader.DataDirectory( _
            IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX).Size

    If importRva = 0 OrElse importSize < IMAGE_IMPORT_DESCRIPTOR_SIZE Then
        Return False
    End If

    Dim importEnd64 As LongInt = _
        CLngInt(importRva) + CLngInt(importSize)

    Dim descriptorRva As ULong = importRva

    While CLngInt(descriptorRva) + _
          IMAGE_IMPORT_DESCRIPTOR_SIZE <= importEnd64

        Dim nameRva As ULong = 0

        If ReadRawRvaDword( _
            Context.pDump, _
            Context.FileSize, _
            descriptorRva + 12, _
            nameRva) = False Then
            Exit While
        End If

        If nameRva = 0 Then Exit While

        Dim moduleName As String

        If ReadAsciiStringByRva( _
            Context.pDump, _
            Context.FileSize, _
            nameRva, _
            moduleName) Then

            If LCase(moduleName) = "dxgkrnl.sys" Then
                Return True
            End If
        End If

        descriptorRva += IMAGE_IMPORT_DESCRIPTOR_SIZE
    Wend

    Return False
End Function

' ============================================================
' RecoverDxgkrnlDispatchBaseline
'
' 恢复 dxgkrnl 框架安装的 MajorFunction 基线:
'
'   1. 结构扫描 dxgkrnl.sys 全部 RuntimeFunction,
'      按 target-vs-dump 精确一致评分,选出本 DriverObject
'      的真实 dispatch writer;
'   2. exact slot → FrameworkBaseline
'      (Confidence 由 exact/conFLICT 动态决定);
'   3. 未覆盖槽位若 current 呈均匀默认分布
'      → FrameworkDefault (Confidence 85)。
'
' 前置条件(由 RecoverDriverDispatch 保证):
'   Context.ModelKind = DispatchModelWddm 且已执行
'   MergeWddmDispatchResult (FrameworkOwned 标记)。
' ============================================================
Function RecoverDxgkrnlDispatchBaseline( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Baseline As WDDM_DISPATCH_BASELINE_OUTPUT) _
        As Boolean

    ClearWddmDispatchBaseline Baseline

    If Context.ModelKind <> _
       DispatchModelWddm Then
        Return False
    End If

    If Context.ModelInfo.IsWddm = False Then
        Return False
    End If

    Dim moduleName As String = _
        "dxgkrnl.sys"

    Dim DxgContext As DISPATCH_SCAN_CONTEXT
    Dim DxgFile() As Byte

    DxgContext.pDump = NULL
    DxgContext.FileSize = 0
    DxgContext.ImageBase = 0
    DxgContext.ModuleName = ""

    If LoadDispatchModuleContext( _
        moduleName, _
        DxgContext, _
        DxgFile()) = False Then

        MyLog.PrintInfo _
            "RecoverDxgkrnlDispatchBaseline", "", _
            "[WDDM-BASELINE] Failed to load dxgkrnl.sys"

        Return False
    End If

    Dim Candidate As WDDM_BASELINE_CANDIDATE
    ClearWddmBaselineCandidate Candidate

    If ScanDxgkrnlRuntimeFunctionsForBaseline( _
        DxgContext, _
        RecoverOutput, _
        Context, _
        Candidate) = False Then

        MyLog.PrintInfo _
            "RecoverDxgkrnlDispatchBaseline", "", _
            "[WDDM-BASELINE]" & _
            " Structural scan found no candidate"

        Erase DxgFile

        Return False
    End If

    '
    ' 第二道保险:
    '
    ' exact >= 4 是强验证(评分硬门槛已保证),
    ' 这里只再要求候选至少覆盖 4 个槽位,
    ' 防止极端巧合下的单点误匹配。
    '
    If Candidate.HitCount < 4 Then

        MyLog.PrintInfo _
            "RecoverDxgkrnlDispatchBaseline", "", _
            "[WDDM-BASELINE] Candidate rejected" & _
            " Hits=" & _
                Trim(Str(Candidate.HitCount))

        Erase DxgFile

        Return False
    End If

    Baseline.DxgkrnlBase = _
        DxgContext.ImageBase

    Baseline.WriterFunctionRva = _
        Candidate.WriterFunctionRva

    Baseline.CandidateBaseReg = _
        Candidate.BaseReg

    Baseline.DriverObjectTracked = _
        Candidate.DriverObjectTracked

    Baseline.SequentialMajorFunction = _
        Candidate.SequentialMajorFunction

    Baseline.HitCount = _
        Candidate.HitCount

    '
    ' Confidence 由 dump 实证强度决定:
    '
    '   无冲突 + exact >= 8  → 95
    '   无冲突 + exact >= 4  → 90
    '   其它                 → 80
    '
    If Candidate.ConflictCount = 0 AndAlso _
       Candidate.ExactCount >= 8 Then

        Baseline.Confidence = 95

    ElseIf Candidate.ConflictCount = 0 AndAlso _
           Candidate.ExactCount >= 4 Then

        Baseline.Confidence = 90

    Else
        Baseline.Confidence = 80
    End If

    '
    ' ---- 第一层:exact slot 填充 ----
    '
    For i As Integer = 0 To _
        IRP_MJ_MAXIMUM_FUNCTION

        If Candidate.MajorFunction(i).Valid = False Then
            Continue For
        End If

        Dim targetRva As ULong = _
            Candidate.MajorFunction(i).FunctionRva

        '
        ' 最后一次确认:
        ' baseline 必须指向 dxgkrnl 可执行代码。
        '
        If targetRva = 0 OrElse _
           IsRvaExecutable( _
               DxgContext.pDump, _
               targetRva) = False Then
            Continue For
        End If

        Baseline.MajorFunction(i).Valid = True

        Baseline.MajorFunction(i).ModuleBase = _
            DxgContext.ImageBase

        Baseline.MajorFunction(i).FunctionRva = _
            targetRva

        Baseline.MajorFunction(i).State = _
            DispatchSlotRecovered

        Baseline.MajorFunction(i).OwnerKind = _
            DispatchOwnerFramework

        Baseline.MajorFunction(i).Source = _
            DispatchSourceFrameworkBaseline

        Baseline.MajorFunction(i).Confidence = _
            Baseline.Confidence

        MyLog.PrintInfo _
            "RecoverDxgkrnlDispatchBaseline", "", _
            "[WDDM-BASELINE] " & _
            IRP_MJ_ToString(i) & _
            " RVA=0x" & Hex(targetRva) & _
            " VA=0x" & _
                Hex( _
                    DxgContext.ImageBase + _
                    targetRva)
    Next

    '
    ' ---- 第二层:uniform default 填充 ----
    '
    ' candidate 未覆盖、但 current ∈ dxgkrnl 且呈均匀分布
    ' 的槽位,恢复为 FrameworkDefault。
    '
    ApplyWddmUniformDefault _
        Context, _
        DxgContext, _
        Candidate, _
        Baseline

    MyLog.PrintInfo _
        "RecoverDxgkrnlDispatchBaseline", "", _
        "[WDDM-BASELINE] Structural baseline accepted" & _
        " WriterRVA=0x" & _
            Hex(Baseline.WriterFunctionRva) & _
        " Base=" & _
            DispatchRegName( _
                Baseline.CandidateBaseReg) & _
        " Hits=" & _
            Trim(Str(Baseline.HitCount)) & _
        " Exact=" & _
            Trim(Str(Candidate.ExactCount)) & _
        " Confidence=" & _
            Trim(Str(Baseline.Confidence)) & _
        IIf(Baseline.HasUniformDefault, _
            " UniformDefault=0x" & Hex(Baseline.DefaultRva) & _
            "(" & Trim(Str(Baseline.DefaultSlotCount)) & ")", _
            "")

    Erase DxgFile

    Return True
End Function

Function ScanDxgkrnlRuntimeFunctionsForBaseline( _
    ByRef DxgContext As DISPATCH_SCAN_CONTEXT, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _      ' ← 新增
    ByRef BestCandidate As WDDM_BASELINE_CANDIDATE) As Boolean

    Const IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX = 3
    Const RUNTIME_FUNCTION_ENTRY_SIZE = 12

    ClearWddmBaselineCandidate BestCandidate

    If DxgContext.pDump = NULL OrElse _
       DxgContext.FileSize = 0 Then
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, _
            DxgContext.pDump)

    If dos->e_magic <> &H5A4D Then
        Return False
    End If

    If dos->e_lfanew <= 0 Then
        Return False
    End If

    If CLngInt(dos->e_lfanew) + _
       SizeOf(IMAGE_NT_HEADERS64) > _
       DxgContext.FileSize Then
        Return False
    End If

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, _
            DxgContext.pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 Then
        Return False
    End If

    Dim exceptionRva As ULong = _
        nt64->OptionalHeader.DataDirectory( _
            IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX _
        ).VirtualAddress

    Dim exceptionSize As ULong = _
        nt64->OptionalHeader.DataDirectory( _
            IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX _
        ).Size

    If exceptionRva = 0 OrElse _
       exceptionSize < RUNTIME_FUNCTION_ENTRY_SIZE Then

        MyLog.PrintInfo _
            "ScanDxgkrnlRuntimeFunctionsForBaseline", "", _
            "[WDDM-BASELINE] No exception directory"

        Return False
    End If

    Dim entryCount As ULong = _
        exceptionSize \ _
        RUNTIME_FUNCTION_ENTRY_SIZE

    MyLog.PrintInfo _
        "ScanDxgkrnlRuntimeFunctionsForBaseline", "", _
        "[WDDM-BASELINE] RuntimeFunctionCount=" & _
        Trim(Str(entryCount))
    
    Dim ambiguous As Boolean = False
    
    For i As ULong = 0 To entryCount - 1

        Dim entryRva As ULong = _
            exceptionRva + _
            i * RUNTIME_FUNCTION_ENTRY_SIZE

        Dim beginRva As ULong = 0
        Dim endRva As ULong = 0

        If ReadRawRvaDword( _
            DxgContext.pDump, _
            DxgContext.FileSize, _
            entryRva, _
            beginRva) = False Then

            Continue For
        End If

        If ReadRawRvaDword( _
            DxgContext.pDump, _
            DxgContext.FileSize, _
            entryRva + 4, _
            endRva) = False Then

            Continue For
        End If

        If beginRva = 0 OrElse _
           endRva <= beginRva Then
            Continue For
        End If

        If IsRvaExecutable( _
            DxgContext.pDump, _
            beginRva) = False Then

            Continue For
        End If
        
        ScanDxgkrnlBaselineFunction _
            DxgContext, _
            beginRva, _
            endRva, _
            RecoverOutput, _
            Context, _                                         ' ← 新增
            BestCandidate, _
            ambiguous
    Next

    If BestCandidate.Valid = False Then
        MyLog.PrintInfo _
            "ScanDxgkrnlRuntimeFunctionsForBaseline", "", _
            "[WDDM-BASELINE] No structural candidate"

        Return False
    End If

    If ambiguous Then
        MyLog.PrintInfo _
            "ScanDxgkrnlRuntimeFunctionsForBaseline", "", _
            "[WDDM-BASELINE] Best candidate ambiguous," & _
            " reject baseline"

        Return False
    End If

    MyLog.PrintInfo _
        "ScanDxgkrnlRuntimeFunctionsForBaseline", "", _
        "[WDDM-BASELINE] Best structural candidate" & _
        " Writer=0x" & _
            Hex(BestCandidate.WriterFunctionRva) & _
        " Base=" & _
            DispatchRegName(BestCandidate.BaseReg) & _
        " Hits=" & Trim(Str(BestCandidate.HitCount)) & _
        " Match=" & Trim(Str(BestCandidate.MatchCount)) & _
        " Missing=" & Trim(Str(BestCandidate.MissingCount)) & _
        " Extra=" & Trim(Str(BestCandidate.ExtraCount)) & _
        " Score=" & Trim(Str(BestCandidate.Score))

    Return True
End Function

Private Sub TrackDriverObjectRegister( _
    RegState() As DISPATCH_REG_STATE, _
    CandidateByReg() As WDDM_BASELINE_CANDIDATE)

    For i As Integer = 0 To 15

        If RegState(i).ValueType = _
           DispatchRegDriverObject Then

            CandidateByReg(i).DriverObjectTracked = True

        End If

    Next

End Sub

Function ScanDxgkrnlBaselineFunction( _
    ByRef DxgContext As DISPATCH_SCAN_CONTEXT, _
    ByVal FunctionRva As ULong, _
    ByVal FunctionEndRva As ULong, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _      ' ← 新增
    ByRef BestCandidate As WDDM_BASELINE_CANDIDATE, _
    ByRef Ambiguous As Boolean) As Boolean

    Const WDDM_BASELINE_MIN_HITS = 6
    Const WDDM_BASELINE_MAX_FUNCTION_SIZE = &H4000

    If DxgContext.pDump = NULL OrElse _
       DxgContext.FileSize = 0 Then
        Return False
    End If

    If FunctionRva = 0 OrElse _
       FunctionEndRva <= FunctionRva Then
        Return False
    End If

    Dim functionSize As ULong = _
        FunctionEndRva - FunctionRva

    If functionSize = 0 OrElse _
       functionSize > WDDM_BASELINE_MAX_FUNCTION_SIZE Then
        Return False
    End If

    Dim functionFoa As ULong = _
        RvaToFoa( _
            DxgContext.pDump, _
            FunctionRva)

    If functionFoa = 0 OrElse _
       functionFoa >= DxgContext.FileSize Then
        Return False
    End If

    If functionFoa + functionSize > _
       DxgContext.FileSize Then

        functionSize = _
            DxgContext.FileSize - functionFoa
    End If

    Dim RegState(0 To 15) As DISPATCH_REG_STATE

    For i As Integer = 0 To 15
        ClearDispatchReg RegState(i)
    Next

    '
    ' 每个可能的 base register 单独统计。
    '
    Dim CandidateByReg(0 To 15) As WDDM_BASELINE_CANDIDATE

    For i As Integer = 0 To 15
        ClearWddmBaselineCandidate CandidateByReg(i)

        CandidateByReg(i).WriterFunctionRva = FunctionRva
        CandidateByReg(i).BaseReg = i
        CandidateByReg(i).DriverObjectTracked = False
        CandidateByReg(i).FirstMajorFunctionOffset = -1
    Next

    Dim u As ud

    ud_init(@u)
    ud_set_mode(@u, 64)
    ud_set_pc(@u, FunctionRva)

    ud_set_input_buffer _
        @u, _
        DxgContext.pDump + functionFoa, _
        functionSize

    While ud_decode(@u) <> 0

        Dim mnemonic As ud_mnemonic_code = _
            ud_insn_mnemonic(@u)

        Dim op0 As Const ud_operand Ptr = _
            ud_insn_opr(@u, 0)

        Dim op1 As Const ud_operand Ptr = _
            ud_insn_opr(@u, 1)

        Select Case mnemonic

            Case UD_Ilea
                '
                ' LEA reg, [RIP + function]
                '
                If op0 <> NULL AndAlso _
                   op1 <> NULL AndAlso _
                   op0->typ = UD_OP_REG AndAlso _
                   op1->typ = UD_OP_MEM AndAlso _
                   op1->Base = UD_R_RIP AndAlso _
                   op1->index = UD_NONE Then

                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        ClearDispatchReg RegState(dstReg)

                        Dim targetRva As ULong = _
                            GetRipRelativeOperandRva( _
                                @u, _
                                op1)

                        If targetRva <> 0 AndAlso _
                           IsRvaExecutable( _
                               DxgContext.pDump, _
                               targetRva) Then

                            RegState(dstReg).ValueType = _
                                DispatchRegFunctionRva

                            RegState(dstReg).Value = _
                                targetRva
                        End If
                    End If
                ElseIf op0 <> NULL AndAlso _
                       op0->typ = UD_OP_REG Then

                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        ClearDispatchReg RegState(dstReg)
                    End If
                End If

            Case UD_Imov
                '
                ' MOV reg, reg
                '
                If UdIs64BitGpr(op0) AndAlso _
                   UdIs64BitGpr(op1) Then

                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    Dim srcReg As Integer = _
                        UdRegToGprIndex(op1->Base)

                    If dstReg >= 0 AndAlso _
                       srcReg >= 0 Then

                        CopyDispatchReg _
                            RegState(dstReg), _
                            RegState(srcReg)
                    End If
                
                    If srcReg = UD_R_RCX Then
                        CandidateByReg(dstReg).DriverObjectTracked = True
                    End If

                    Continue While
                End If

                '
                ' MOV [base + MajorFunctionOffset], FunctionReg
                '
                If op0 <> NULL AndAlso _
                   op1 <> NULL AndAlso _
                   op0->typ = UD_OP_MEM AndAlso _
                   op1->typ = UD_OP_REG AndAlso _
                   op0->index = UD_NONE Then

                    Dim baseReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    Dim srcReg As Integer = _
                        UdRegToGprIndex(op1->Base)

                    If baseReg >= 0 AndAlso _
                       srcReg >= 0 Then

                        Dim disp As LongInt = _
                            UdMemoryDisplacement(op0)

                        Dim irpIndex As Integer = -1

                        If WddmMajorOffsetToIndex( _
                            disp, _
                            irpIndex) Then

                            If RegState(srcReg).ValueType = _
                               DispatchRegFunctionRva Then

                                Dim targetRva As ULong = _
                                    CULng( _
                                        RegState(srcReg).Value)

                                If targetRva <> 0 AndAlso _
                                   IsRvaExecutable( _
                                       DxgContext.pDump, _
                                       targetRva) Then

                                    Dim pRef As DISPATCH_FUNCTION_REF Ptr = _
                                        @CandidateByReg(baseReg).MajorFunction(irpIndex)

                                    If pRef->Valid = False Then
                                        CandidateByReg(baseReg).HitCount += 1
                                    End If

                                    pRef->Valid = True
                                    pRef->ModuleBase = _
                                        DxgContext.ImageBase
                                    pRef->FunctionRva = _
                                        targetRva
                                    pRef->State = _
                                        DispatchSlotRecovered
                                    pRef->OwnerKind = _
                                        DispatchOwnerFramework
                                    pRef->Source = _
                                        DispatchSourceFrameworkBaseline
                                    pRef->Confidence = 0

                                    MyLog.PrintInfo _
                                        "ScanDxgkrnlBaselineFunction", "", _
                                        "[WDDM-BASELINE-CANDIDATE]" & _
                                        " Writer=0x" & _
                                            Hex(FunctionRva) & _
                                        " Base=" & _
                                            DispatchRegName(baseReg) & _
                                        " Slot=" & _
                                            IRP_MJ_ToString(irpIndex) & _
                                        " TargetRVA=0x" & _
                                            Hex(targetRva) & _
                                        " StoreRVA=0x" & _
                                            Hex(ud_insn_off(@u))
                                End If
                            End If
                        End If
                    End If

                    Continue While
                End If

                '
                ' MOV 写寄存器但不是 reg <- reg，
                ' 则旧 FunctionRva 状态失效。
                '
                If op0 <> NULL AndAlso _
                   op0->typ = UD_OP_REG Then

                    Dim dstReg As Integer = _
                        UdRegToGprIndex(op0->Base)

                    If dstReg >= 0 Then
                        ClearDispatchReg RegState(dstReg)
                    End If
                End If

            Case UD_Icall
                '
                ' volatile GPR 在 call 后不可继续相信。
                '
                InvalidateVolatileDispatchRegs RegState()

            Case Else
                InvalidateUnknownInstructionDestination _
                    @u, _
                    RegState()
        End Select
    Wend

    Dim foundCandidate As Boolean = False

    For i As Integer = 0 To 15

        If CandidateByReg(i).HitCount = 0 Then
            Continue For
        End If

        CandidateByReg(i).Valid = True
        
        MyLog.PrintInfo _
            "ScanDxgkrnlBaselineFunction", "", _
            "[DEBUG] BeforeConsider Writer=0x" & _
            Hex(CandidateByReg(i).WriterFunctionRva) & _
            " Hit=" & Str(CandidateByReg(i).HitCount)
            
        ConsiderWddmBaselineCandidate _
            CandidateByReg(i), _
            RecoverOutput, _
            Context, _                                         ' ← 新增
            DxgContext.ImageBase, _                            ' ← 新增(运行时基址)
            BestCandidate, _
            Ambiguous

        foundCandidate = True
    Next

    Return foundCandidate
End Function

' ============================================================
' ScoreWddmBaselineCandidate
'
' 主评分 = target 与 dump 当前值的一致性:
'   exact    — candidate 写入的 RVA == dxgkrnl 运行时该槽位 RVA
'   conflict — current ∈ dxgkrnl 但 ≠ candidate RVA
'              (说明该 writer 服务于别的 DriverObject)
'   hooked   — current 在 dxgkrnl 之外
'              (candidate 值即 hook 前原始地址,不罚分)
'
' 硬门槛:
'   exact >= 4        (dump 实证,不靠结构巧合)
'   conflict <= 1     (真 writer 不应与 dump 冲突)
'
' slot-mask 统计(match/missing/extra)降级为辅助信息 + 轻微加分。
' ============================================================
Function ScoreWddmBaselineCandidate( _
    ByRef Candidate As WDDM_BASELINE_CANDIDATE, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByVal DxgRuntimeBase As ULONG_PTR) As Boolean

    Dim exactCount    As ULong = 0
    Dim conflictCount As ULong = 0
    Dim hookedCount   As ULong = 0
    Dim validCount    As ULong = 0

    '
    ' slot-mask 统计(仅辅助)
    '
    Dim expectedCount As ULong = 0
    Dim matchCount    As ULong = 0
    Dim extraCount    As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        Dim frameworkOwned As Boolean = _
            (RecoverOutput.MajorFunction(i).State = _
                DispatchSlotFrameworkOwned)

        If frameworkOwned Then expectedCount += 1

        If Candidate.MajorFunction(i).Valid = False Then Continue For

        validCount += 1

        If frameworkOwned Then
            matchCount += 1
        Else
            extraCount += 1
        End If

        '
        ' target-vs-dump 一致性
        '
        Dim curAddr As ULONG_PTR = Context.CurrentMajorFunction(i)

        If curAddr = 0 Then Continue For
        If curAddr = Context.IopInvalidDeviceRequest Then Continue For

        If IsAddressInModule(curAddr, "dxgkrnl.sys") Then

            If curAddr - DxgRuntimeBase = _
               Candidate.MajorFunction(i).FunctionRva Then

                exactCount += 1
            Else
                conflictCount += 1
            End If
        Else
            '
            ' current 在 dxgkrnl 之外:可能是被 hook 的槽位,
            ' candidate 的值恰是 hook 前的原始地址。
            ' 不罚分,单独计数供日志/UI 使用。
            '
            hookedCount += 1
        End If
    Next

    Dim missingCount As ULong = expectedCount - matchCount

    Candidate.MatchCount   = matchCount
    Candidate.ExtraCount   = extraCount
    Candidate.missingCount = missingCount

    '
    ' ---- 硬门槛(带日志,不再静默拒绝) ----
    '
    If expectedCount < 6 Then
        MyLog.PrintInfo "ScoreWddmBaselineCandidate", "", _
            "[WDDM-BASELINE-SCORE] REJECT(TooFewFrameworkOwned)" & _
            " Writer=0x" & Hex(Candidate.WriterFunctionRva) & _
            " Base=" & DispatchRegName(Candidate.BaseReg) & _
            " Expected=" & Trim(Str(expectedCount))
        Return False
    End If

    If exactCount < 4 Then
        MyLog.PrintInfo "ScoreWddmBaselineCandidate", "", _
            "[WDDM-BASELINE-SCORE] REJECT(NoExactMatch)" & _
            " Writer=0x" & Hex(Candidate.WriterFunctionRva) & _
            " Base=" & DispatchRegName(Candidate.BaseReg) & _
            " Valid=" & Trim(Str(validCount)) & _
            " Exact=" & Trim(Str(exactCount)) & _
            " Conflict=" & Trim(Str(conflictCount)) & _
            " Hooked=" & Trim(Str(hookedCount))
        Return False
    End If

    If conflictCount > 1 Then
        MyLog.PrintInfo "ScoreWddmBaselineCandidate", "", _
            "[WDDM-BASELINE-SCORE] REJECT(ConflictsDump)" & _
            " Writer=0x" & Hex(Candidate.WriterFunctionRva) & _
            " Base=" & DispatchRegName(Candidate.BaseReg) & _
            " Exact=" & Trim(Str(exactCount)) & _
            " Conflict=" & Trim(Str(conflictCount))
        Return False
    End If

    '
    ' ---- 评分:exact 为主,coverage 为辅 ----
    '
    Candidate.Score = _
        CLngInt(exactCount) * 1000 _
        - CLngInt(conflictCount) * 5000 _
        + CLngInt(validCount) * 10 _
        + CLngInt(matchCount) * 10

    '
    ' 完全覆盖所有 framework-owned 槽位:强 bonus。
    '
    If missingCount = 0 AndAlso extraCount = 0 Then
        Candidate.Score += 10000
    End If
    
    Candidate.exactCount    = exactCount
    Candidate.ConflictCount = conflictCount
    Candidate.hookedCount   = hookedCount

    MyLog.PrintInfo "ScoreWddmBaselineCandidate", "", _
        "[WDDM-BASELINE-SCORE]" & _
        " Writer=0x" & Hex(Candidate.WriterFunctionRva) & _
        " Base=" & DispatchRegName(Candidate.BaseReg) & _
        " Hits=" & Trim(Str(Candidate.HitCount)) & _
        " Exact=" & Trim(Str(exactCount)) & _
        " Conflict=" & Trim(Str(conflictCount)) & _
        " Hooked=" & Trim(Str(hookedCount)) & _
        " Match=" & Trim(Str(matchCount)) & _
        " Missing=" & Trim(Str(missingCount)) & _
        " Extra=" & Trim(Str(extraCount)) & _
        " Score=" & Trim(Str(Candidate.Score))

    Return True
End Function


Sub MergeWddmDispatchBaseline( _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef Baseline As WDDM_DISPATCH_BASELINE_OUTPUT)

    If Baseline.HitCount = 0 OrElse _
       Baseline.DxgkrnlBase = 0 Then
        Exit Sub
    End If

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        '
        ' Generic direct recovery 优先级最高。
        '
        If RecoverOutput.MajorFunction(i).State = _
           DispatchSlotRecovered Then

            If RecoverOutput.MajorFunction(i).Source <> _
               DispatchSourceFrameworkBaseline Then
                Continue For
            End If
        End If

        '
        ' Baseline 只允许填 FrameworkOwned。
        '
        If RecoverOutput.MajorFunction(i).State <> _
           DispatchSlotFrameworkOwned Then
            Continue For
        End If

        If Baseline.MajorFunction(i).Valid = False Then
            Continue For
        End If

        '
        ' baseline 地址自身必须属于 dxgkrnl.sys。
        '
        Dim baselineVa As ULONG_PTR = _
            Baseline.MajorFunction(i).ModuleBase + _
            Baseline.MajorFunction(i).FunctionRva

        If IsAddressInModule( _
            baselineVa, _
            "dxgkrnl.sys") = False Then

            MyLog.PrintInfo _
                "MergeWddmDispatchBaseline", "", _
                "[WDDM-BASELINE] Reject outside dxgkrnl" & _
                " Slot=" & IRP_MJ_ToString(i) & _
                " VA=0x" & Hex(baselineVa)

            Continue For
        End If

        RecoverOutput.MajorFunction(i).Valid = True

        RecoverOutput.MajorFunction(i).ModuleBase = _
            Baseline.MajorFunction(i).ModuleBase

        RecoverOutput.MajorFunction(i).FunctionRva = _
            Baseline.MajorFunction(i).FunctionRva

        RecoverOutput.MajorFunction(i).State = _
            DispatchSlotRecovered

        RecoverOutput.MajorFunction(i).OwnerKind = _
            DispatchOwnerFramework

        RecoverOutput.MajorFunction(i).Source = _
            Baseline.MajorFunction(i).Source

        RecoverOutput.MajorFunction(i).Confidence = _
            Baseline.MajorFunction(i).Confidence

        RecoverOutput.HitCount += 1

        MyLog.PrintInfo _
            "MergeWddmDispatchBaseline", "", _
            "[MERGE] " & IRP_MJ_ToString(i) & _
            " FrameworkBaseline fills" & _
            " RVA=0x" & _
                Hex(Baseline.MajorFunction(i).FunctionRva) & _
            " VA=0x" & Hex(baselineVa)
    Next
End Sub

' ============================================================
' 将 WDDM 判定结果合并进 DISPATCH_RECOVERY_OUTPUT：
'
' 对于当前地址属于 dxgkrnl.sys 的 MajorFunction：
'   - State        = FrameworkOwned
'   - OwnerKind    = DispatchOwnerFramework
'   - Source       = FrameworkClassification
'   - Valid        = False / ModuleBase = 0 / FunctionRva = 0
'     → 绝不伪造 OriginalFunction
'
' 对于其它槽位：不覆盖 Generic 结果。
' ============================================================
Sub MergeWddmDispatchResult( _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef ModelInfo As DRIVER_MODEL_INFO, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT)

    If ModelInfo.IsWddm = False Then Exit Sub

    RecoverOutput.ModelKind       = DispatchModelWddm
    RecoverOutput.ModelConfidence = ModelInfo.Confidence

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION

        ' 1. Generic 已恢复的槽位不覆盖
        If RecoverOutput.MajorFunction(i).State = _
           DispatchSlotRecovered Then
            Continue For
        End If

        Dim curAddr As ULONG_PTR = Context.CurrentMajorFunction(i)
        If curAddr = 0 Then Continue For

        ' 2. 默认 IopInvalidDeviceRequest 保留 default 语义
        If Context.IopInvalidDeviceRequest <> 0 AndAlso _
           curAddr = Context.IopInvalidDeviceRequest Then
            Continue For
        End If

        ' 3. 只对 dxgkrnl.sys 的 current 标记 FrameworkOwned
        If IsAddressInModule(curAddr, "dxgkrnl.sys") Then

            ' 绝不把 current 当 original
            RecoverOutput.MajorFunction(i).Valid       = False
            RecoverOutput.MajorFunction(i).ModuleBase  = 0
            RecoverOutput.MajorFunction(i).FunctionRva = 0
            RecoverOutput.MajorFunction(i).State       = _
                DispatchSlotFrameworkOwned
            RecoverOutput.MajorFunction(i).OwnerKind   = _
                DispatchOwnerFramework
            RecoverOutput.MajorFunction(i).Source      = _
                DispatchSourceFrameworkClassification
            RecoverOutput.MajorFunction(i).Confidence  = _
                ModelInfo.Confidence

            MyLog.PrintInfo "MergeWddmDispatchResult", "", _
                "[WDDM] MajorFunction[" & IRP_MJ_ToString(i) & "]" & _
                " Current=0x" & Hex(curAddr) & _
                " Owner=dxgkrnl.sys" & _
                " State=FrameworkOwned" & _
                " Original=UNKNOWN"
        End If
    Next
End Sub