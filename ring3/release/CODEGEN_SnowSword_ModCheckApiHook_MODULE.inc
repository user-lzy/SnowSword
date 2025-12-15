'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModCheckApiHook.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


/'Public Function CheckApiHook(ByVal szDllPath As String, ByVal szFuncName As String, OldCode As String, NewCode As String) As Boolean // szDllPath为DLL的完整路径 !
    1.CH_E88EB7E58F96E69687E4BBB6E695B0E68DAE_
    Dim hFile As Long: hFile = CreateFile(szDllPath, Win32Api.GENERIC_READ, FILE_SHARE_READ, 0, _
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)
    If (HFILE = INVALID_HANDLE_VALUE) Then Exit Function
    Open szDllPath For Binary As #1
    Dim ntdllAddr As HMODULE = LoadLibraryA(Right(szDllPath, Len(szDllPath) - InStrRev(szDllPath, "\")))
    Dim apiAddr As ULONG64 = GetProcAddress(ntdllAddr, szFuncName)
    If apiAddr = 0 Then
        Close #1
        Exit Function
    End If
    
    //CH_E68A8A_NtdllCH_E8AFBBE585A5E8BF9BE58685E5AD98_
    Dim dwFileSize As ULONG64 = FileLen(szDllPath)
    Dim pBuff() As Byte: ReDim pBuff(dwFileSize - 1)
    PrintA Get(#1,, pBuff())
    

    'Debug.Print Err.Number & Err.Description'去掉注释就炸
    
    2.CH_E88EB7E58F96E587BDE695B0E59CA8E69687E4BBB6E4B8ADE79A84E4BD8DE7BDAE_
    Dim pDosHeader As IMAGE_DOS_HEADER: memcpy @pDosHeader, @pBuff(0), Len(pDosHeader)
    Dim pNtHeader As IMAGE_NT_HEADERS: memcpy @pNtHeader, @pBuff(Len(pDosHeader) + pDosHeader.e_lfanew), Len(pNtHeader)
    Dim pSectionHeader As IMAGE_SECTION_HEADER
    Dim Deviation As Long
    //CH_E587BDE695B0E59CB0E59D80_ - DLLCH_E59CB0E59D80_ = RVA
    Dim Rva As ULONG64 = apiAddr - Cast(ULONG64, ntdllAddr)
    Dim i As Long
    // CH_E58588E9809AE8BF87_RVACH_E689BEE588B0E68980E59CA8E58CBAE6AEB5_,CH_E5868DE794A8_RVA-CH_E5BD93E5898DE58CBAE6AEB5_RVA+CH_E5BD93E5898DE58CBAE6AEB5E69687E4BBB6E5818FE7A7BB_ = F0A
    // CH_E69C80E5908EE794A8_F0ACH_E689BEE588B0E79BB8E5AFB9E79A84_opcode,CH_E5A48DE588B6E588B0E696B0E58685E5AD98E689A7E8A18CE5868DE8B7B3E8BDAC_
    For i = 0 To pNtHeader.FileHeader.NumberOfSections
        memcpy @pSectionHeader, @pBuff(Len(pNtHeader) + Len(pSectionHeader) * i), Len(pSectionHeader)
        If (Rva > pSectionHeader.VirtualAddress) And (Rva < pSectionHeader.VirtualAddress + pSectionHeader.SizeOfRawData) Then
            //CH_E9809AE8BF87_RVACH_E689BEE588B0E68980E59CA8E58CBAE6AEB5_,CH_E5868DE794A8_RVA-CH_E5BD93E5898DE58CBAE6AEB5_RVA+CH_E69687E4BBB6E5818FE7A7BB_ = F0A
            Deviation = Rva - pSectionHeader.VirtualAddress + pSectionHeader.PointerToRawData
            Exit For
        End If
    Next
    If (Deviation = 0) Then
        Erase pBuff
        Close #1
        Exit Function
    End If
    
    3.CH_E88EB7E58F96E58E9FE69CBAE599A8E7A081_
    // CH_E69C80E5908EE794A8E6AEB5E58685E5818FE7A7BBE689BEE588B0E79BB8E5AFB9E79A84_opcode
    Dim OldByte As Byte, NewByte As Byte
    CheckApiHook = True
    For i As Integer = 0 To &H10
        memcpy @OldByte, @pBuff(Deviation), 1
        memcpy @NewByte, apiAddr, 1
        If OldByte <> NewByte Then PrintA Hex(OldByte) & " " & Hex(NewByte)
        OldCode = OldCode + " " & Format(Hex(OldByte), "00")
        NewCode = NewCode + " " & Format(Hex(NewByte), "00")
    Next
    Erase pBuff
    Close #1
    
End Function'/