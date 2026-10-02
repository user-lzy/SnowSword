' ============================================================
' ClassifyDriverDispatchModel
'
' 单一 WDDM 分类出口：
'   DetectWddmDriver 判 WDDM → ModelKind = DispatchModelWddm
'   否则                   → GenericWdm
' ============================================================
Function ClassifyDriverDispatchModel( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT) As Boolean

    Context.ModelKind       = DispatchModelGenericWdm
    Context.FrameworkKind   = WdmFrameworkNone
    Context.ModelConfidence = 30

    ' 显式清零（含 String 成员，不能 memset）
    Context.ModelInfo.IsWddm         = False
    Context.ModelInfo.Confidence     = 0
    Context.ModelInfo.Reason         = ""
    Context.ModelInfo.FrameworkOwner = ""

    ' ---------------------------------------------------------
    ' 唯一 WDDM 分类路径
    ' ---------------------------------------------------------
    If DetectWddmDriver(Context, Context.ModelInfo) Then
        Context.ModelKind       = DispatchModelWddm
        Context.ModelConfidence = CULng(Context.ModelInfo.Confidence)

        MyLog.PrintInfo "ClassifyDriverDispatchModel", "", _
            "[CLASSIFY] Module=" & Context.ModuleName & _
            " Model=WDDM" & _
            " Confidence=" & Trim(Str(Context.ModelInfo.Confidence)) & _
            " Reason=" & Context.ModelInfo.Reason

        Return True
    End If

    MyLog.PrintInfo "ClassifyDriverDispatchModel", "", _
        "[CLASSIFY] Module=" & Context.ModuleName & _
        " Model=GenericWdm" & _
        " Confidence=" & Trim(Str(Context.ModelConfidence)) & _
        " WddmReason=" & Context.ModelInfo.Reason

    Return True
End Function

Sub MergeGenericDispatchResult( _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef GenericOutput As DISPATCH_RECOVERY_OUTPUT, _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT)

    RecoverOutput.FastIoDispatchRva = GenericOutput.FastIoDispatchRva

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        If GenericOutput.MajorFunction(i).Valid Then
            RecoverOutput.MajorFunction(i).Valid = True
            RecoverOutput.MajorFunction(i).ModuleBase = _
                GenericOutput.MajorFunction(i).ModuleBase
            RecoverOutput.MajorFunction(i).FunctionRva = _
                GenericOutput.MajorFunction(i).FunctionRva
            RecoverOutput.MajorFunction(i).State = DispatchSlotRecovered
            RecoverOutput.MajorFunction(i).OwnerKind = DispatchOwnerDriver
            RecoverOutput.MajorFunction(i).Source = DispatchSourceDirectStore
            RecoverOutput.MajorFunction(i).Confidence = _
                GenericOutput.Confidence

            RecoverOutput.HitCount += 1
        End If
    Next
End Sub

Sub FinalizeDispatchRecovery( _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT)

    Dim recoveredCount As ULong = 0
    Dim frameworkBaselineCount As ULong = 0
    Dim frameworkOwnedCount As ULong = 0
    Dim defaultCount As ULong = 0
    Dim unresolvedCount As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        Select Case RecoverOutput.MajorFunction(i).State

            Case DispatchSlotRecovered
                recoveredCount += 1

                If RecoverOutput.MajorFunction(i).Source = _
                   DispatchSourceFrameworkBaseline Then

                    frameworkBaselineCount += 1
                End If

            Case DispatchSlotFrameworkOwned
                frameworkOwnedCount += 1

            Case DispatchSlotDefault
                defaultCount += 1

            Case Else
                unresolvedCount += 1
        End Select
    Next

    MyLog.PrintInfo _
        "FinalizeDispatchRecovery", "", _
        "[FINALIZE]" & _
        " Model=" & Str(RecoverOutput.ModelKind) & _
        " FrameworkKind=" & _
            Str(RecoverOutput.FrameworkKind) & _
        " Recovered=" & Str(recoveredCount) & _
        " FrameworkBaseline=" & _
            Str(frameworkBaselineCount) & _
        " FrameworkOwned=" & _
            Str(frameworkOwnedCount) & _
        " Default=" & Str(defaultCount) & _
        " Unresolved=" & Str(unresolvedCount) & _
        " ModelConfidence=" & _
            Str(RecoverOutput.ModelConfidence)
End Sub

Function RecoverDriverDispatch( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT) As DISPATCH_RECOVERY_STATUS

    ' 1. 清零
    memset(@RecoverOutput, 0, SizeOf(RecoverOutput))

    RecoverOutput.ModelKind = DispatchModelUnknown
    RecoverOutput.FrameworkKind = WdmFrameworkNone
    RecoverOutput.ModelConfidence = 0

    ' 2. 永远先跑 Generic WDM backend
    Dim genericInput As DISPATCH_RECOVERY_INPUT
    Dim genericOutput As DISPATCH_RECOVERY_OUTPUT

    genericInput.pDump = Context.pDump
    genericInput.FileSize = Context.FileSize
    genericInput.ImageBase = Context.ImageBase
    genericInput.ModuleName = Context.ModuleName

    Dim genericStatus As DISPATCH_RECOVERY_STATUS = _
        RecoverDispatchFromDump(genericInput, genericOutput)

    If genericStatus = DispatchRecoveryParseFailed Then
        MyLog.PrintInfo "RecoverDriverDispatch", "", _
            "[ORCH] Generic WDM backend parse failed"
        Return DispatchRecoveryParseFailed
    End If

    ' 3. 合并 Generic 结果
    MergeGenericDispatchResult RecoverOutput, genericOutput, Context

    ' 4. 分类驱动模型
    ClassifyDriverDispatchModel Context

    RecoverOutput.ModelKind = Context.ModelKind
    RecoverOutput.FrameworkKind = Context.FrameworkKind
    RecoverOutput.ModelConfidence = Context.ModelConfidence

    ' 5. 按模型分流
    Select Case Context.ModelKind

        Case DispatchModelWddm
            '
            ' 第一层：ownership classification。
            '
            MergeWddmDispatchResult _
                RecoverOutput, _
                Context.ModelInfo, _
                Context

            '
            ' 第二层：独立恢复 dxgkrnl framework baseline。
            '
            Dim WddmBaseline As WDDM_DISPATCH_BASELINE_OUTPUT

            ClearWddmDispatchBaseline WddmBaseline

            If RecoverDxgkrnlDispatchBaseline( _
                Context, _
                RecoverOutput, _
                WddmBaseline) Then

                MergeWddmDispatchBaseline _
                    RecoverOutput, _
                    WddmBaseline
            Else
                MyLog.PrintInfo _
                    "RecoverDriverDispatch", "", _
                    "[WDDM-BASELINE] Baseline unavailable," & _
                    " retain FrameworkOwned semantics"
            End If

        Case DispatchModelFrameworkWdm
            ' HID 仍走 Generic scanner 的 ScanImportedDispatchCall 路径，
            ' 第一阶段不迁移。
            ' StorPort / NDIS / KS 未实现。

        Case DispatchModelKmdf
            ' Reserved for KMDF backend.
            MyLog.PrintInfo "RecoverDriverDispatch", "", _
                "[ORCH] KMDF backend reserved, fallback to generic result"

        Case Else
            ' Generic WDM only

    End Select

    ' 6. 补默认语义：当前值 == IopInvalidDeviceRequest 且尚未分类的槽位
    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        If RecoverOutput.MajorFunction(i).State = DispatchSlotUnresolved Then
            Dim curAddr As ULONG_PTR = Context.CurrentMajorFunction(i)

            If Context.IopInvalidDeviceRequest <> 0 AndAlso _
               curAddr = Context.IopInvalidDeviceRequest Then

                RecoverOutput.MajorFunction(i).State = DispatchSlotDefault
                RecoverOutput.MajorFunction(i).OwnerKind = DispatchOwnerKernelDefault
                RecoverOutput.MajorFunction(i).Source = DispatchSourceDefault
            End If
        End If
    Next

    ' 7. Finalize
    FinalizeDispatchRecovery RecoverOutput

    If RecoverOutput.HitCount > 0 Then
        RecoverOutput.Confidence = _
            IIf(RecoverOutput.HitCount >= 20, 100, RecoverOutput.HitCount * 5)
    Else
        RecoverOutput.Confidence = 0
    End If

    ' 8. 状态判定
    Dim totalResolved As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        If RecoverOutput.MajorFunction(i).State <> DispatchSlotUnresolved Then
            totalResolved += 1
        End If
    Next

    If totalResolved > 0 Then
        Return DispatchRecoveryRecovered
    Else
        Return DispatchRecoveryNoMatch
    End If
End Function

Sub BuildDriverDispatchRecoveryContext( _
    ByRef Context As DRIVER_DISPATCH_RECOVERY_CONTEXT, _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal imageBase As ULONG_PTR, _
    ByVal imageSize As ULong, _
    ByRef ModuleName As String, _
    ByRef CurrentDriverInfo As DRIVER_INFO, _
    ByVal IopInvalidDeviceRequestAddr As ULONG_PTR)

    ' 注意：Context 里有 String 成员，不能用 memset
    Context.pDump = pDump
    Context.FileSize = fileSize
    Context.ImageBase = imageBase
    Context.ImageSize = imageSize
    Context.ModuleName = ModuleName
    Context.IopInvalidDeviceRequest = IopInvalidDeviceRequestAddr

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        Context.CurrentMajorFunction(i) = _
            CurrentDriverInfo.MajorFunctionAddr(i)
    Next

    Context.ModelKind = DispatchModelUnknown
    Context.FrameworkKind = WdmFrameworkNone
    Context.ModelConfidence = 0

    ' 新增：显式初始化 ModelInfo（含 String，不能 memset）
    Context.ModelInfo.IsWddm         = False
    Context.ModelInfo.Confidence     = 0
    Context.ModelInfo.Reason         = ""
    Context.ModelInfo.FrameworkOwner = ""
End Sub

Function ApplyDispatchRecoveryResult( _
    ByRef RecoverOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT, _
    ByRef OriginalDriverInfo As DRIVER_INFO, _
    ByRef CurrentDriverInfo As DRIVER_INFO, _
    ByVal IopInvalidDeviceRequestAddr As ULONG_PTR) _
        As ORIGINAL_DRIVER_RECOVERY_STATUS

    Dim result As ORIGINAL_DRIVER_RECOVERY_STATUS = 0

    Dim recoveredCount As ULong = 0
    Dim frameworkBaselineCount As ULong = 0
    Dim frameworkOwnedCount As ULong = 0
    Dim unresolvedCount As ULong = 0

    For i As Integer = 0 To IRP_MJ_MAXIMUM_FUNCTION
        Dim ref As DISPATCH_FUNCTION_REF = _
            RecoverOutput.MajorFunction(i)

        Select Case ref.State

            Case DispatchSlotRecovered
                If ref.Valid AndAlso _
                   ref.ModuleBase <> 0 AndAlso _
                   ref.FunctionRva <> 0 Then

                    OriginalDriverInfo.MajorFunctionAddr(i) = _
                        ref.ModuleBase + ref.FunctionRva

                    recoveredCount += 1

                    If ref.Source = _
                       DispatchSourceFrameworkBaseline Then

                        frameworkBaselineCount += 1
                    End If
                Else
                    OriginalDriverInfo.MajorFunctionAddr(i) = 0
                    unresolvedCount += 1
                End If

            Case DispatchSlotFrameworkOwned
                '
                ' Ownership 已知，但没有可信 baseline。
                '
                OriginalDriverInfo.MajorFunctionAddr(i) = 0
                frameworkOwnedCount += 1

            Case DispatchSlotDefault
                OriginalDriverInfo.MajorFunctionAddr(i) = _
                    IopInvalidDeviceRequestAddr

            Case Else
                If IopInvalidDeviceRequestAddr <> 0 AndAlso _
                   CurrentDriverInfo.MajorFunctionAddr(i) = _
                       IopInvalidDeviceRequestAddr Then

                    OriginalDriverInfo.MajorFunctionAddr(i) = _
                        IopInvalidDeviceRequestAddr
                Else
                    OriginalDriverInfo.MajorFunctionAddr(i) = 0
                    unresolvedCount += 1
                End If
        End Select
    Next

    '
    ' 主状态。
    '
    If unresolvedCount > 0 Then
        If recoveredCount > 0 OrElse _
           frameworkOwnedCount > 0 Then

            result = OriginalRecoveryDispatchPartial
        Else
            result = OriginalRecoveryDispatchUnresolved
        End If

    ElseIf recoveredCount > 0 Then
        result = OriginalRecoveryDispatchRecovered

    ElseIf frameworkOwnedCount > 0 Then
        result = OriginalRecoveryDispatchFrameworkOwned

    Else
        result = OriginalRecoveryDispatchDefaultOnly
    End If

    '
    ' FrameworkOwned 附加状态。
    '
    If frameworkOwnedCount > 0 Then
        result Or= _
            OriginalRecoveryDispatchFrameworkOwned
    End If

    '
    ' 真正存在独立 framework baseline。
    '
    If frameworkBaselineCount > 0 Then
        result Or= _
            OriginalRecoveryDispatchFrameworkBaselineRecovered
    End If

    MyLog.PrintInfo _
        "ApplyDispatchRecoveryResult", "", _
        "[APPLY]" & _
        " Result=0x" & Hex(result) & _
        " Recovered=" & Trim(Str(recoveredCount)) & _
        " FrameworkBaseline=" & _
            Trim(Str(frameworkBaselineCount)) & _
        " FrameworkOwned=" & _
            Trim(Str(frameworkOwnedCount)) & _
        " Unresolved=" & _
            Trim(Str(unresolvedCount))

    Return result
End Function

Function GetOriginalDispatchFunctionAddr( _
    ByRef DriverPath As StringW, _
    ByRef CurrentDriverInfo As DRIVER_INFO, _
    ByRef OriginalDriverInfo As DRIVER_INFO) _
        As ORIGINAL_DRIVER_RECOVERY_STATUS

    Dim bytDump() As Byte
    Dim filePath As String = DriverPath
    Dim fileName As String = GetNameByPath(DriverPath)
    Dim imageSize As ULong
    Dim result As ORIGINAL_DRIVER_RECOVERY_STATUS = OriginalRecoveryFatal

    memset(@OriginalDriverInfo, 0, SizeOf(DRIVER_INFO))

    Dim imageBase As ULONG_PTR = _
        Cast(ULONG_PTR, GetKernelModuleBase(fileName, imageSize))

    If imageBase = 0 Then
        MyLog.PrintLog LOG_WARN, _
            "GetOriginalDispatchFunctionAddr",, _
            "无法获取驱动加载基址"
        Return OriginalRecoveryFatal
    End If

    On Error GoTo FileError

    Open filePath For Binary As #1
        ReDim bytDump(LOF(1) - 1)
        Get #1, , bytDump()
    Close #1

    On Error GoTo 0

    If UBound(bytDump) < 0 Then
        Return OriginalRecoveryFatal
    End If

    Dim fileSize As ULong = UBound(bytDump) + 1
    Dim pDump As UByte Ptr = Cast(UByte Ptr, @bytDump(0))

    ' 获取 IopInvalidDeviceRequest
    Dim IopInvalidDeviceRequestAddr As ULONG_PTR = 0

    SetLastError 0
    If IoControl( _
        hDrv, _
        IOCTL_GetIopInvalidDeviceRequest, _
        NULL, _
        0, _
        @IopInvalidDeviceRequestAddr, _
        SizeOf(IopInvalidDeviceRequestAddr)) = 0 Then

        MyLog.PrintWin32Error "GetOriginalDispatchFunctionAddr", _
            "IOCTL_GetIopInvalidDeviceRequest"
        Return OriginalRecoveryFatal
    End If

    ' ================================
    ' 统一派遣恢复抽象层
    ' ================================
    Dim recoveryContext As DRIVER_DISPATCH_RECOVERY_CONTEXT
    BuildDriverDispatchRecoveryContext _
        recoveryContext, _
        pDump, _
        fileSize, _
        imageBase, _
        imageSize, _
        fileName, _
        CurrentDriverInfo, _
        IopInvalidDeviceRequestAddr

    Dim recoveryOutput As DRIVER_DISPATCH_RECOVERY_OUTPUT
    Dim recoveryStatus As DISPATCH_RECOVERY_STATUS = _
        RecoverDriverDispatch(recoveryContext, recoveryOutput)

    If recoveryStatus = DispatchRecoveryParseFailed Then
        MyLog.PrintLog LOG_WARN, _
            "GetOriginalDispatchFunctionAddr",, _
            "驱动 PE/DriverEntry 解析失败"
        Return OriginalRecoveryFatal
    End If

    ' 应用到 OriginalDriverInfo
    result = ApplyDispatchRecoveryResult( _
        recoveryOutput, _
        OriginalDriverInfo, _
        CurrentDriverInfo, _
        IopInvalidDeviceRequestAddr)

    ' ================================
    ' FastIO —— 保持原状
    ' ================================
    If CurrentDriverInfo.FastIoDispatchAddr = NULL Then
        result Or= OriginalRecoveryFastIoNotPresent
    Else
        Dim fastIo As FAST_IO_DISPATCH_ORIGINAL

        If RecoverFastIoFromDump( _
            pDump, _
            fileSize, _
            imageBase, _
            recoveryOutput.FastIoDispatchRva, _
            CurrentDriverInfo.FastIoDispatchAddr, _
            @fastIo) Then

            For i As Integer = 0 To FAST_IO_MAX_COUNT - 1
                If fastIo.Functions(i) <> 0 Then
                    OriginalDriverInfo.FastIOFunctionAddr(i) = _
                        Cast(ULONG_PTR, fastIo.Functions(i))
                End If
            Next

            result Or= OriginalRecoveryFastIoRecovered
        Else
            result Or= OriginalRecoveryFastIoUnresolved

            MyLog.PrintLog LOG_WARN, _
                "GetOriginalDispatchFunctionAddr",, _
                "驱动存在 FastIoDispatch，但无法从 PE 恢复原始 FastIO"
        End If
    End If

    Return result

FileError:
    On Error GoTo 0

    MyLog.PrintLog LOG_WARN, _
        "GetOriginalDispatchFunctionAddr",, _
        "无法读取驱动 PE 文件"

    Return OriginalRecoveryFatal
End Function