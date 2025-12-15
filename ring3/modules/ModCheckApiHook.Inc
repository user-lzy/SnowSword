/'Public Function CheckApiHook(ByVal szDllPath As String, ByVal szFuncName As String, OldCode As String, NewCode As String) As Boolean // szDllPath为DLL的完整路径 !
    1.获取文件数据
    Dim hFile As Long: hFile = CreateFile(szDllPath, Win32Api.GENERIC_READ, FILE_SHARE_READ, 0, _
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)
    If (HFILE = INVALID_HANDLE_VALUE) Then Exit Function
    Open szDllPath For Binary As #1
    Dim ntdllAddr As HMODULE = LoadLibraryA(Right(szDllPath, Len(szDllPath) - InStrRev(szDllPath, "\")))
    Dim apiAddr As Integer = GetProcAddress(ntdllAddr, szFuncName)
    If apiAddr = 0 Then
        Close #1
        Exit Function
    End If
    
    //把Ntdll读入进内存
    Dim dwFileSize As Integer = FileLen(szDllPath)
    Dim pBuff() As Byte: ReDim pBuff(dwFileSize - 1)
    Print Get(#1,, pBuff())
    

    'Debug.Print Err.Number & Err.Description'去掉注释就炸
    
    2.获取函数在文件中的位置
    Dim pDosHeader As IMAGE_DOS_HEADER: memcpy @pDosHeader, @pBuff(0), Len(pDosHeader)
    Dim pNtHeader As IMAGE_NT_HEADERS: memcpy @pNtHeader, @pBuff(Len(pDosHeader) + pDosHeader.e_lfanew), Len(pNtHeader)
    Dim pSectionHeader As IMAGE_SECTION_HEADER
    Dim Deviation As Long
    //函数地址 - DLL地址 = RVA
    Dim Rva As Integer = apiAddr - Cast(Integer, ntdllAddr)
    Dim i As Long
    // 先通过RVA找到所在区段,再用RVA-当前区段RVA+当前区段文件偏移 = F0A
    // 最后用F0A找到相对的opcode,复制到新内存执行再跳转
    For i = 0 To pNtHeader.FileHeader.NumberOfSections
        memcpy @pSectionHeader, @pBuff(Len(pNtHeader) + Len(pSectionHeader) * i), Len(pSectionHeader)
        If (Rva > pSectionHeader.VirtualAddress) And (Rva < pSectionHeader.VirtualAddress + pSectionHeader.SizeOfRawData) Then
            //通过RVA找到所在区段,再用RVA-当前区段RVA+文件偏移 = F0A
            Deviation = Rva - pSectionHeader.VirtualAddress + pSectionHeader.PointerToRawData
            Exit For
        End If
    Next
    If (Deviation = 0) Then
        Erase pBuff
        Close #1
        Exit Function
    End If
    
    3.获取原机器码
    // 最后用段内偏移找到相对的opcode
    Dim OldByte As Byte, NewByte As Byte
    CheckApiHook = True
    For i As Integer = 0 To &H10
        memcpy @OldByte, @pBuff(Deviation), 1
        memcpy @NewByte, apiAddr, 1
        If OldByte <> NewByte Then Print Hex(OldByte) & " " & Hex(NewByte)
        OldCode = OldCode + " " & Format(Hex(OldByte), "00")
        NewCode = NewCode + " " & Format(Hex(NewByte), "00")
    Next
    Erase pBuff
    Close #1
    
End Function'/