Function GetByte(pDump As UByte Ptr, offset As ULong, fileSize As ULong) As Integer
    If pDump = 0 Or offset >= fileSize Then Return -1
    Return pDump[offset]
End Function

Function ReadRawRvaDword( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal rva As ULong, _
    ByRef Value As ULong) As Boolean

    Value = 0

    Dim foa As ULong = RvaToFoa(pDump, rva)
    If foa = 0 OrElse foa + 4 > fileSize Then Return False

    memcpy(@Value, pDump + foa, 4)
    Return True
End Function

Function ReadRawRvaQword( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal rva As ULong, _
    ByRef Value As ULONG_PTR) As Boolean

    Value = 0

    Dim foa As ULong = RvaToFoa(pDump, rva)
    If foa = 0 OrElse foa + SizeOf(ULONG_PTR) > fileSize Then Return False

    memcpy(@Value, pDump + foa, SizeOf(ULONG_PTR))
    Return True
End Function

Function ReadRawRvaWord( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal rva As ULong, _
    ByRef Value As UShort) As Boolean

    Value = 0

    Dim foa As ULong = RvaToFoa(pDump, rva)
    If foa = 0 OrElse foa + 2 > fileSize Then Return False

    memcpy(@Value, pDump + foa, 2)
    Return True
End Function

Function ReadAsciiStringByRva( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal rva As ULong, _
    ByRef Value As String) As Boolean

    Const MAX_PE_STRING_LENGTH = 1024

    Value = ""

    If pDump = NULL OrElse rva = 0 Then Return False

    Dim foa As ULong = RvaToFoa(pDump, rva)
    If foa = 0 OrElse foa >= fileSize Then Return False

    For i As ULong = 0 To MAX_PE_STRING_LENGTH - 1
        If foa + i >= fileSize Then Return False

        Dim ch As UByte = pDump[foa + i]

        If ch = 0 Then
            Return True
        End If

        Value &= Chr(ch)
    Next

    Return False
End Function

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

Function GetPeImageInfo( _
    ByVal pDump As UByte Ptr, _
    ByRef PreferredImageBase As ULONG_PTR, _
    ByRef SizeOfImage As ULong) As Boolean

    PreferredImageBase = 0
    SizeOfImage = 0

    If pDump = NULL Then Return False

    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)
    If nt64->Signature <> &H4550 Then Return False

    PreferredImageBase = Cast(ULONG_PTR, nt64->OptionalHeader.ImageBase)
    SizeOfImage = nt64->OptionalHeader.SizeOfImage

    Return PreferredImageBase <> 0 AndAlso SizeOfImage <> 0
End Function

Function GetImageEntryPointRVA(ByVal pDump As UByte Ptr) As ULong
    If pDump = 0 Then Return 0
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return 0

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)
    If nt64->Signature <> &H4550 Or nt64->FileHeader.Machine <> &H8664 Then Return 0

    Return nt64->OptionalHeader.AddressOfEntryPoint
End Function

Function GetRuntimeFunctionBounds( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal FunctionRva As ULong, _
    ByRef BeginRva As ULong, _
    ByRef EndRva As ULong) As Boolean

    Const IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX = 3
    Const RUNTIME_FUNCTION_SIZE = 12

    BeginRva = 0
    EndRva = 0

    If pDump = NULL OrElse fileSize = 0 OrElse FunctionRva = 0 Then Return False

    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False
    If dos->e_lfanew <= 0 Then Return False
    If CLngInt(dos->e_lfanew) + SizeOf(IMAGE_NT_HEADERS64) > fileSize Then Return False

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 OrElse _
       nt64->FileHeader.Machine <> &H8664 Then
        Return False
    End If

    Dim exceptionRva As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX).VirtualAddress

    Dim exceptionSize As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXCEPTION_INDEX).Size

    If exceptionRva = 0 OrElse exceptionSize < RUNTIME_FUNCTION_SIZE Then Return False

    Dim entryCount As ULong = exceptionSize \ RUNTIME_FUNCTION_SIZE

    For i As ULong = 0 To entryCount - 1
        Dim entryRva64 As LongInt = _
            CLngInt(exceptionRva) + CLngInt(i) * RUNTIME_FUNCTION_SIZE

        If entryRva64 <= 0 OrElse entryRva64 > &HFFFFFFFFLL Then Exit For

        Dim currentBegin As ULong = 0
        Dim currentEnd As ULong = 0

        If ReadRawRvaDword( _
            pDump, _
            fileSize, _
            CULng(entryRva64), _
            currentBegin) = False Then
            Continue For
        End If

        If ReadRawRvaDword( _
            pDump, _
            fileSize, _
            CULng(entryRva64) + 4, _
            currentEnd) = False Then
            Continue For
        End If

        If currentBegin = 0 OrElse currentEnd <= currentBegin Then Continue For

        If FunctionRva >= currentBegin AndAlso FunctionRva < currentEnd Then
            BeginRva = currentBegin
            EndRva = currentEnd
            Return True
        End If
    Next

    Return False
End Function

Function IsRvaExecutable(ByVal pDump As UByte Ptr, ByVal rva As ULong) As Boolean
    If pDump = NULL Then Return False
    Dim dos As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, pDump)
    If dos->e_magic <> &H5A4D Then Return False

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)
    If nt64->Signature <> &H4550 Then Return False

    Dim secHdr As IMAGE_SECTION_HEADER Ptr = Cast(IMAGE_SECTION_HEADER Ptr, Cast(ULONG_PTR, nt64) + SizeOf(IMAGE_NT_HEADERS64))

    For i As ULong = 0 To nt64->FileHeader.NumberOfSections - 1
        Dim sectionSize As ULong = secHdr->Misc.VirtualSize
        If sectionSize < secHdr->SizeOfRawData Then sectionSize = secHdr->SizeOfRawData
        If rva >= secHdr->VirtualAddress AndAlso rva < secHdr->VirtualAddress + sectionSize Then
            Return (secHdr->Characteristics And IMAGE_SCN_MEM_EXECUTE) <> 0
        End If
        secHdr += 1
    Next

    Return False
End Function

Function IsAddressInModule( _
    ByVal Address As ULONG_PTR, _
    ByRef ModuleName As String) As Boolean

    If Address = 0 OrElse Len(ModuleName) = 0 Then Return False

    Dim imageSize As ULong = 0
    Dim imageBase As ULONG_PTR = _
        Cast(ULONG_PTR, GetKernelModuleBase(ModuleName, imageSize))

    If imageBase = 0 OrElse imageSize = 0 Then Return False

    Return (Address >= imageBase) AndAlso _
           (Address < imageBase + imageSize)
End Function

Function LoadDispatchModuleContext( _
    ByRef ModuleName As String, _
    ByRef Context As DISPATCH_SCAN_CONTEXT, _
    FileBuffer() As Byte) As Boolean

    Context.pDump = NULL
    Context.FileSize = 0
    Context.ImageBase = 0
    Context.ModuleName = ""

    If Len(ModuleName) = 0 Then
        Return False
    End If

    Dim imageSize As ULong = 0

    Dim imageBase As ULONG_PTR = _
        Cast(ULONG_PTR, GetKernelModuleBase(ModuleName, imageSize))

    If imageBase = 0 Then
        MyLog.PrintInfo "LoadDispatchModuleContext", "", _
            "[REJECT] Module is not loaded Module=" & ModuleName

        Return False
    End If

    Dim systemRoot As String = Environ("SystemRoot")

    If Len(systemRoot) = 0 Then
        systemRoot = "C:\Windows"
    End If

    Dim modulePath As String = _
        systemRoot & "\System32\drivers\" & ModuleName

    If Dir(modulePath) = "" Then
        '
        ' ntoskrnl.exe、hal.dll 等不一定在 drivers。
        '
        modulePath = _
            systemRoot & "\System32\" & ModuleName
    End If

    If Dir(modulePath) = "" Then
        MyLog.PrintInfo "LoadDispatchModuleContext", "", _
            "[REJECT] Module file not found Module=" & ModuleName

        Return False
    End If

    Dim fileNo As Integer = FreeFile
    Dim fileOpened As Boolean = False

    On Error GoTo LoadError

    Open modulePath For Binary Access Read As #fileNo
    fileOpened = True

    Dim size64 As LongInt = LOF(fileNo)

    If size64 <= 0 OrElse size64 > &HFFFFFFFFLL Then
        Close #fileNo
        fileOpened = False
        On Error GoTo 0
        Return False
    End If

    Dim fileSize As ULong = CULng(size64)

    ReDim FileBuffer(0 To fileSize - 1)

    Get #fileNo, , FileBuffer()

    Close #fileNo
    fileOpened = False

    On Error GoTo 0

    If fileSize < SizeOf(IMAGE_DOS_HEADER) Then
        Erase FileBuffer
        Return False
    End If

    Dim pDump As UByte Ptr = _
        Cast(UByte Ptr, @FileBuffer(0))

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, pDump)

    If dos->e_magic <> &H5A4D Then
        Erase FileBuffer
        Return False
    End If

    If dos->e_lfanew <= 0 OrElse _
       CLngInt(dos->e_lfanew) + SizeOf(IMAGE_NT_HEADERS64) > fileSize Then

        Erase FileBuffer
        Return False
    End If

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 OrElse _
       nt64->FileHeader.Machine <> &H8664 Then

        Erase FileBuffer
        Return False
    End If

    Context.pDump = pDump
    Context.FileSize = fileSize
    Context.ImageBase = imageBase
    Context.ModuleName = ModuleName

    MyLog.PrintInfo "LoadDispatchModuleContext", "", _
        "[LOAD] Module=" & ModuleName & _
        " Path=" & modulePath & _
        " ImageBase=0x" & Hex(imageBase) & _
        " FileSize=0x" & Hex(fileSize)

    Return True

LoadError:
    If fileOpened Then
        Close #fileNo
    End If

    On Error GoTo 0

    Context.pDump = NULL
    Context.FileSize = 0
    Context.ImageBase = 0
    Context.ModuleName = ""

    Erase FileBuffer

    MyLog.PrintInfo "LoadDispatchModuleContext", "", _
        "[REJECT] Failed to load Module=" & ModuleName

    Return False
End Function

Function ResolveImportByIatRva( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByVal IatRva As ULong, _
    ByRef Target As PE_IMPORT_TARGET) As Boolean

    Const IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX = 1
    Const IMAGE_IMPORT_DESCRIPTOR_SIZE = 20
    Const IMAGE_THUNK_SIZE = 8
    Const IMAGE_ORDINAL_FLAG64_VALUE As ULONG_PTR = &H8000000000000000ULL

    Target.Valid = False
    Target.ModuleName = ""
    Target.FunctionName = ""
    Target.Ordinal = 0
    Target.ByOrdinal = False

    If pDump = NULL OrElse fileSize = 0 OrElse IatRva = 0 Then
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, pDump)

    If dos->e_magic <> &H5A4D Then
        Return False
    End If

    If dos->e_lfanew <= 0 Then
        Return False
    End If

    If CLngInt(dos->e_lfanew) + SizeOf(IMAGE_NT_HEADERS64) > fileSize Then
        Return False
    End If

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 OrElse _
       nt64->FileHeader.Machine <> &H8664 Then

        Return False
    End If

    Dim importRva As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX).VirtualAddress

    Dim importSize As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT_INDEX).Size

    If importRva = 0 OrElse importSize < IMAGE_IMPORT_DESCRIPTOR_SIZE Then
        Return False
    End If

    Dim importEnd64 As LongInt = _
        CLngInt(importRva) + CLngInt(importSize)

    If importEnd64 <= importRva OrElse importEnd64 > &HFFFFFFFFLL Then
        Return False
    End If

    Dim descriptorRva As ULong = importRva

    While CLngInt(descriptorRva) + IMAGE_IMPORT_DESCRIPTOR_SIZE <= importEnd64
        Dim originalFirstThunk As ULong = 0
        Dim timeDateStamp As ULong = 0
        Dim forwarderChain As ULong = 0
        Dim nameRva As ULong = 0
        Dim firstThunk As ULong = 0

        If ReadRawRvaDword(pDump, fileSize, descriptorRva + 0, originalFirstThunk) = False Then Exit While
        If ReadRawRvaDword(pDump, fileSize, descriptorRva + 4, timeDateStamp) = False Then Exit While
        If ReadRawRvaDword(pDump, fileSize, descriptorRva + 8, forwarderChain) = False Then Exit While
        If ReadRawRvaDword(pDump, fileSize, descriptorRva + 12, nameRva) = False Then Exit While
        If ReadRawRvaDword(pDump, fileSize, descriptorRva + 16, firstThunk) = False Then Exit While

        If originalFirstThunk = 0 AndAlso _
           timeDateStamp = 0 AndAlso _
           forwarderChain = 0 AndAlso _
           nameRva = 0 AndAlso _
           firstThunk = 0 Then

            Exit While
        End If

        If nameRva <> 0 AndAlso firstThunk <> 0 Then
            Dim moduleName As String

            If ReadAsciiStringByRva( _
                pDump, _
                fileSize, _
                nameRva, _
                moduleName) Then

                Dim lookupThunkRva As ULong

                If originalFirstThunk <> 0 Then
                    lookupThunkRva = originalFirstThunk
                Else
                    lookupThunkRva = firstThunk
                End If

                Dim thunkIndex As ULong = 0

                Do
                    Dim lookupRva64 As LongInt = _
                        CLngInt(lookupThunkRva) + _
                        CLngInt(thunkIndex) * IMAGE_THUNK_SIZE

                    Dim currentIatRva64 As LongInt = _
                        CLngInt(firstThunk) + _
                        CLngInt(thunkIndex) * IMAGE_THUNK_SIZE

                    If lookupRva64 <= 0 OrElse _
                       lookupRva64 > &HFFFFFFFFLL OrElse _
                       currentIatRva64 <= 0 OrElse _
                       currentIatRva64 > &HFFFFFFFFLL Then

                        Exit Do
                    End If

                    Dim thunkValue As ULONG_PTR = 0

                    If ReadRawRvaQword( _
                        pDump, _
                        fileSize, _
                        CULng(lookupRva64), _
                        thunkValue) = False Then

                        Exit Do
                    End If

                    If thunkValue = 0 Then
                        Exit Do
                    End If

                    If CULng(currentIatRva64) = IatRva Then
                        Target.ModuleName = moduleName

                        If (thunkValue And IMAGE_ORDINAL_FLAG64_VALUE) <> 0 Then
                            Target.ByOrdinal = True
                            Target.Ordinal = CULng(thunkValue And &HFFFFULL)
                            Target.FunctionName = ""
                        Else
                            If thunkValue > &HFFFFFFFFULL Then
                                Return False
                            End If

                            Dim importByNameRva As ULong = _
                                CULng(thunkValue)

                            '
                            ' IMAGE_IMPORT_BY_NAME:
                            '
                            ' +0 USHORT Hint
                            ' +2 CHAR Name[]
                            '
                            If ReadAsciiStringByRva( _
                                pDump, _
                                fileSize, _
                                importByNameRva + 2, _
                                Target.FunctionName) = False Then

                                Return False
                            End If

                            If Len(Target.FunctionName) = 0 Then
                                Return False
                            End If

                            Target.ByOrdinal = False
                            Target.Ordinal = 0
                        End If

                        Target.Valid = True

                        MyLog.PrintInfo "ResolveImportByIatRva", "", _
                            "[MATCH] IatRVA=0x" & Hex(IatRva) & _
                            " Module=" & Target.ModuleName & _
                            " Function=" & Target.FunctionName & _
                            " Ordinal=" & Str(Target.Ordinal) & _
                            " ByOrdinal=" & Str(Target.ByOrdinal)

                        Return True
                    End If

                    thunkIndex += 1

                    '
                    ' 防止损坏 PE 导致无限扫描。
                    '
                    If thunkIndex > &H10000 Then
                        Exit Do
                    End If
                Loop
            End If
        End If

        descriptorRva += IMAGE_IMPORT_DESCRIPTOR_SIZE
    Wend

    MyLog.PrintInfo "ResolveImportByIatRva", "", _
        "[REJECT] IAT slot not found RVA=0x" & Hex(IatRva)

    Return False
End Function

Function ResolveExportRva( _
    ByVal pDump As UByte Ptr, _
    ByVal fileSize As ULong, _
    ByRef FunctionName As String, _
    ByVal Ordinal As ULong, _
    ByVal ByOrdinal As Boolean, _
    ByRef FunctionRva As ULong) As Boolean

    Const IMAGE_DIRECTORY_ENTRY_EXPORT_INDEX = 0

    FunctionRva = 0

    If pDump = NULL OrElse fileSize = 0 Then
        Return False
    End If

    Dim dos As IMAGE_DOS_HEADER Ptr = _
        Cast(IMAGE_DOS_HEADER Ptr, pDump)

    If dos->e_magic <> &H5A4D Then
        Return False
    End If

    If dos->e_lfanew <= 0 Then
        Return False
    End If

    If CLngInt(dos->e_lfanew) + SizeOf(IMAGE_NT_HEADERS64) > fileSize Then
        Return False
    End If

    Dim nt64 As IMAGE_NT_HEADERS64 Ptr = _
        Cast(IMAGE_NT_HEADERS64 Ptr, pDump + dos->e_lfanew)

    If nt64->Signature <> &H4550 OrElse _
       nt64->FileHeader.Machine <> &H8664 Then

        Return False
    End If

    Dim exportRva As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT_INDEX).VirtualAddress

    Dim exportSize As ULong = _
        nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT_INDEX).Size

    If exportRva = 0 OrElse exportSize < 40 Then
        Return False
    End If

    '
    ' IMAGE_EXPORT_DIRECTORY:
    '
    ' +16 Base
    ' +20 NumberOfFunctions
    ' +24 NumberOfNames
    ' +28 AddressOfFunctions
    ' +32 AddressOfNames
    ' +36 AddressOfNameOrdinals
    '
    Dim ordinalBase As ULong = 0
    Dim numberOfFunctions As ULong = 0
    Dim numberOfNames As ULong = 0
    Dim addressOfFunctions As ULong = 0
    Dim addressOfNames As ULong = 0
    Dim addressOfNameOrdinals As ULong = 0

    If ReadRawRvaDword(pDump, fileSize, exportRva + 16, ordinalBase) = False Then Return False
    If ReadRawRvaDword(pDump, fileSize, exportRva + 20, numberOfFunctions) = False Then Return False
    If ReadRawRvaDword(pDump, fileSize, exportRva + 24, numberOfNames) = False Then Return False
    If ReadRawRvaDword(pDump, fileSize, exportRva + 28, addressOfFunctions) = False Then Return False
    If ReadRawRvaDword(pDump, fileSize, exportRva + 32, addressOfNames) = False Then Return False
    If ReadRawRvaDword(pDump, fileSize, exportRva + 36, addressOfNameOrdinals) = False Then Return False

    If numberOfFunctions = 0 OrElse addressOfFunctions = 0 Then
        Return False
    End If

    Dim functionIndex As ULong = &HFFFFFFFF

    If ByOrdinal Then
        If Ordinal < ordinalBase Then
            Return False
        End If

        Dim ordinalIndex64 As LongInt = _
            CLngInt(Ordinal) - CLngInt(ordinalBase)

        If ordinalIndex64 < 0 OrElse _
           ordinalIndex64 >= numberOfFunctions Then

            Return False
        End If

        functionIndex = CULng(ordinalIndex64)
    Else
        If Len(FunctionName) = 0 OrElse _
           numberOfNames = 0 OrElse _
           addressOfNames = 0 OrElse _
           addressOfNameOrdinals = 0 Then

            Return False
        End If

        For i As ULong = 0 To numberOfNames - 1
            Dim exportNameRva As ULong = 0

            If ReadRawRvaDword( _
                pDump, _
                fileSize, _
                addressOfNames + i * 4, _
                exportNameRva) = False Then

                Continue For
            End If

            Dim exportName As String

            If ReadAsciiStringByRva( _
                pDump, _
                fileSize, _
                exportNameRva, _
                exportName) = False Then

                Continue For
            End If

            If exportName = FunctionName Then
                Dim ordinalIndex As UShort = 0

                If ReadRawRvaWord( _
                    pDump, _
                    fileSize, _
                    addressOfNameOrdinals + i * 2, _
                    ordinalIndex) = False Then

                    Return False
                End If

                If ordinalIndex >= numberOfFunctions Then
                    Return False
                End If

                functionIndex = ordinalIndex
                Exit For
            End If
        Next

        If functionIndex = &HFFFFFFFF Then
            MyLog.PrintInfo "ResolveExportRva", "", _
                "[REJECT] Export name not found Name=" & FunctionName

            Return False
        End If
    End If

    Dim targetRva As ULong = 0

    If ReadRawRvaDword( _
        pDump, _
        fileSize, _
        addressOfFunctions + functionIndex * 4, _
        targetRva) = False Then

        Return False
    End If

    If targetRva = 0 Then
        Return False
    End If

    '
    ' 如果 AddressOfFunctions 指向 Export Directory 自己的范围，
    ' 这不是代码，而是 Forwarder 字符串。
    '
    Dim exportEnd64 As LongInt = _
        CLngInt(exportRva) + CLngInt(exportSize)

    If CLngInt(targetRva) >= exportRva AndAlso _
       CLngInt(targetRva) < exportEnd64 Then

        Dim forwarder As String

        ReadAsciiStringByRva _
            pDump, _
            fileSize, _
            targetRva, _
            forwarder

        MyLog.PrintInfo "ResolveExportRva", "", _
            "[REJECT] Forwarded export Function=" & FunctionName & _
            " Ordinal=" & Str(Ordinal) & _
            " Forwarder=" & forwarder

        Return False
    End If

    If RvaToFoa(pDump, targetRva) = 0 Then
        Return False
    End If

    FunctionRva = targetRva

    MyLog.PrintInfo "ResolveExportRva", "", _
        "[MATCH] Function=" & FunctionName & _
        " Ordinal=" & Str(Ordinal) & _
        " RVA=0x" & Hex(FunctionRva)

    Return True
End Function

Function GetRipRelativeOperandRva( _
    ByVal u As ud Ptr, _
    ByVal Opr As Const ud_operand Ptr) As ULong

    If u = NULL OrElse Opr = NULL Then Return 0
    If Opr->typ <> UD_OP_MEM Then Return 0
    If Opr->Base <> UD_R_RIP Then Return 0
    If Opr->index <> UD_NONE Then Return 0

    Dim disp As LongInt = UdMemoryDisplacement(Opr)

    Dim target64 As LongInt = _
        CLngInt(ud_insn_off(u)) + _
        CLngInt(ud_insn_len(u)) + _
        disp

    If target64 <= 0 OrElse target64 > &HFFFFFFFFLL Then Return 0

    Return CULng(target64)
End Function