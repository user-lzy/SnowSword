#include "windows.bi"

Type RemotePeImage
    hProc As HANDLE
    baseAddr  As Any Ptr
End Type

Type InlineHookInfo
    hookType As String
    proxyFunc  As ULongInt
End Type

Type OBJECT_INFORMATION
    ObjectName As Wstring * 50
    Procedure(7) As PVOID
End Type

Function ReadMem(ByRef img As RemotePeImage, addr As Any Ptr, buf As Any Ptr, sz As Integer) As Boolean
    Return ReadProcessMemory(img.hProc, addr, buf, sz, 0) <> 0
End Function

Function WriteMem(ByRef img As RemotePeImage, addr As Any Ptr, buf As Any Ptr, sz As Integer) As Boolean
    Return WriteProcessMemory(img.hProc, addr, buf, sz, 0) <> 0
End Function

Function LoadDiskImage(path As String, ByRef sizeOut As UInteger) As UByte Ptr
    Dim h As HANDLE = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)
    If h = INVALID_HANDLE_VALUE Then Return NULL

    sizeOut = GetFileSize(h, NULL)
    Dim buf As UByte Ptr = Allocate(sizeOut)
    Dim br As DWORD
    ReadFile(h, buf, sizeOut, @br, NULL)
    CloseHandle(h)
    Return buf
End Function

Function GetRemoteNT(ByRef img As RemotePeImage, ByRef dos As IMAGE_DOS_HEADER, ByRef nt As IMAGE_NT_HEADERS) As Boolean

    If ReadMem(img, img.baseAddr, @dos, SizeOf(dos)) = 0 Then Return False
    If ReadMem(img, img.baseAddr + dos.e_lfanew, @nt, SizeOf(nt)) = 0 Then Return False

    Return TRUE
End Function

Function RemoteRvaToVa(ByRef img As RemotePeImage, rva As DWORD) As Any Ptr
    Return img.baseAddr + rva
End Function

Function GetDiskExportRVA(diskImg As UByte Ptr, apiName As String) As DWORD
    If diskImg = NULL Then Return 0
    
    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, diskImg)
    If dos->e_magic <> IMAGE_DOS_SIGNATURE Then Return 0
    
    Dim nt As IMAGE_NT_HEADERS Ptr = CPtr(IMAGE_NT_HEADERS Ptr, diskImg + dos->e_lfanew)
    If nt->Signature <> IMAGE_NT_SIGNATURE Then Return 0
    
    Dim expRVA As DWORD = nt->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    If expRVA = 0 Then Return 0
    
    ' 将RVA转换为文件偏移
    Dim expDirRaw As UByte Ptr = RvaToRaw(diskImg, expRVA)
    If expDirRaw = NULL Then Return 0
    
    Dim expDir As IMAGE_EXPORT_DIRECTORY Ptr = CPtr(IMAGE_EXPORT_DIRECTORY Ptr, expDirRaw)
    
    ' 安全地读取数组指针
    Dim nameRvaRaw As UByte Ptr = RvaToRaw(diskImg, expDir->AddressOfNames)
    Dim ordRvaRaw As UByte Ptr = RvaToRaw(diskImg, expDir->AddressOfNameOrdinals)
    Dim funRvaRaw As UByte Ptr = RvaToRaw(diskImg, expDir->AddressOfFunctions)
    
    If nameRvaRaw = NULL Or ordRvaRaw = NULL Or funRvaRaw = NULL Then Return 0
    
    Dim nameRva As DWORD Ptr = CPtr(DWORD Ptr, nameRvaRaw)
    Dim ordRva As WORD Ptr = CPtr(WORD Ptr, ordRvaRaw)
    Dim funRva As DWORD Ptr = CPtr(DWORD Ptr, funRvaRaw)
    
    For i As Integer = 0 To expDir->NumberOfNames - 1
        ' 安全边界检查
        If i >= expDir->NumberOfNames Then Exit For
        
        ' 将函数名RVA转换为文件偏移
        Dim nameRaw As UByte Ptr = RvaToRaw(diskImg, nameRva[i])
        If nameRaw = NULL Then Continue For
        
        Dim pName As ZString Ptr = CPtr(ZString Ptr, nameRaw)
        
        ' 安全比较字符串
        If *pName = apiName Then
            Dim idx As WORD = ordRva[i]
            If idx < expDir->NumberOfFunctions Then
                Return funRva[idx]
            End If
        End If
    Next
    
    Return 0
End Function

/'Function GetDiskExportRVA(diskImg As UByte Ptr, apiName As String) As DWORD

    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, diskImg)
    Dim nt  As IMAGE_NT_HEADERS Ptr = CPtr(IMAGE_NT_HEADERS Ptr, diskImg + dos->e_lfanew)

    Dim expRVA As DWORD = nt->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    If expRVA = 0 Then Return 0

    Dim expDir As IMAGE_EXPORT_DIRECTORY Ptr = CPtr(IMAGE_EXPORT_DIRECTORY Ptr, diskImg + expRVA)

    Dim nameRva As DWORD Ptr = CPtr(DWORD Ptr, diskImg + expDir->AddressOfNames)
    If nameRva = NULL Then Print "nameRva = NULL":Return 0
    Dim ordRva As WORD Ptr = CPtr(WORD Ptr, diskImg + expDir->AddressOfNameOrdinals)
    If ordRva = NULL Then Print "ordRva = NULL":Return 0
    Dim funRva As DWORD Ptr = CPtr(DWORD Ptr, diskImg + expDir->AddressOfFunctions)
    If funRva = NULL Then Print "funRva = NULL":Return 0

    For i As Integer = 0 To expDir->NumberOfNames - 1
        Dim pName As ZString Ptr = CPtr(ZString Ptr, diskImg + nameRva[i])
        If *pName = apiName Then
            Return funRva[ ordRva[i] ]
        End If
    Next

    Return 0
End Function'/

Function RvaToRaw(buf As UByte Ptr, rva As DWORD) As UByte Ptr
    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, buf)
    Dim nt As IMAGE_NT_HEADERS Ptr = CPtr(IMAGE_NT_HEADERS Ptr, buf + dos->e_lfanew)

    Dim sec As IMAGE_SECTION_HEADER Ptr = _
        Cast(IMAGE_SECTION_HEADER Ptr, buf + dos->e_lfanew + _
             SizeOf(IMAGE_NT_HEADERS))

    For i As Integer = 0 To nt->FileHeader.NumberOfSections - 1
        With sec[i]
            If rva >= .VirtualAddress And rva < .VirtualAddress + .Misc.VirtualSize Then
                Return buf + .PointerToRawData + (rva - .VirtualAddress)
            End If
        End With
    Next

    Return NULL
End Function

Function RemoteDumpExports(img As RemotePeImage, names() As String) As Integer

    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As IMAGE_NT_HEADERS

    If GetRemoteNT(img, dos, nt) = FALSE Then Return 0

    Dim expRVA As DWORD = nt.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    If expRVA = 0 Then Return 0

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    ReadMem(img, img.baseAddr + expRVA, @expDir, SizeOf(IMAGE_EXPORT_DIRECTORY))

    Dim nameRva(expDir.NumberOfNames - 1) As DWORD
    ReadMem(img, img.baseAddr + expDir.AddressOfNames, @nameRva(0), expDir.NumberOfNames*4)

    ReDim names(expDir.NumberOfNames-1)

    For i As Integer = 0 To expDir.NumberOfNames-1
        Dim namePtr As ZString * 256
        ReadMem(img, img.baseAddr + nameRva(i), @namePtr, 256)
        names(i) = namePtr
    Next

    Return expDir.NumberOfNames
End Function

Function GetModuleOfAddress(hProc As HANDLE, addr As Any Ptr) As String
    Dim sz As ZString * MAX_PATH
    GetMappedFileNameA(hProc, addr, @sz, MAX_PATH)
    Return sz
End Function

Function GetModuleByVA(img As RemotePeImage, ByVal addr As ULongInt, _
                       ByRef modBase As ULongInt, _
                       ByRef modSize As ULongInt, _
                       ByRef modName As String) As Boolean

    Dim mbi As MEMORY_BASIC_INFORMATION
    If VirtualQueryEx(img.hProc , Cast(LPCVOID, addr), @mbi, SizeOf(MEMORY_BASIC_INFORMATION)) = 0 Then Return False

    modBase = Cast(ULongInt, mbi.AllocationBase)

    ' 获取模块名
    Dim path As ZString * MAX_PATH
    'Print img.hProc
    If GetModuleFileNameExA(img.hProc, mbi.AllocationBase, @path, MAX_PATH) = 0 Then
    'If GetMappedFileNameA(img.hProc, mbi.AllocationBase, @path, MAX_PATH) = 0 Then
        modName = ""
        Print "[GetMappedFileNameA]" & WinErrorMsg(GetLastError) & GetLastError
    Else
        modName = path
    End If

    ' PE 大小
    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As IMAGE_NT_HEADERS

    ReadMem img, mbi.AllocationBase, @dos, SizeOf(IMAGE_DOS_HEADER)
    ReadMem img, mbi.AllocationBase + dos.e_lfanew, @nt, SizeOf(IMAGE_NT_HEADERS)

    modSize = nt.OptionalHeader.SizeOfImage
    Return TRUE
End Function

Function ResolveFuncName(img As RemotePeImage, ByVal modBase As ULongInt, _
                        ByVal rva As ULongInt, _
                        ByRef bestName As String, _
                        ByRef bestOffset As ULongInt) As Boolean

    bestName = ""
    bestOffset = rva  ' 默认偏移

    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As IMAGE_NT_HEADERS

    'ReadProcessMemory(hProc, Cast(LPCVOID, modBase), @dos, SizeOf(dos), 0)
    ReadMem img, Cast(Any Ptr, modBase), @dos, SizeOf(IMAGE_DOS_HEADER)
    'ReadProcessMemory(hProc, Cast(LPCVOID, modBase + dos.e_lfanew), _
    '@nt, SizeOf(nt), 0)
    ReadMem img, Cast(Any Ptr, modBase + dos.e_lfanew), @nt, SizeOf(IMAGE_NT_HEADERS)

    Dim dataDir As IMAGE_DATA_DIRECTORY = nt.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)
    If dataDir.VirtualAddress = 0 Then Return False

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    ReadMem img, Cast(Any Ptr, modBase + dataDir.VirtualAddress), @expDir, SizeOf(IMAGE_EXPORT_DIRECTORY)

    Dim funcRVAs(expDir.NumberOfFunctions-1) As DWORD
    ReadMem img, Cast(Any Ptr, modBase + expDir.AddressOfFunctions), @funcRVAs(0), expDir.NumberOfFunctions*4

    ' 遍历所有函数，找 offset<=rva 中最大的
    Dim bestIndex As Integer = -1
    For i As Integer = 0 To expDir.NumberOfFunctions-1
        If funcRVAs(i) <= rva Then
            If bestIndex = -1 OrElse funcRVAs(i) > funcRVAs(bestIndex) Then
                bestIndex = i
            End If
        End If
    Next

    If bestIndex = -1 Then Return FALSE

    Dim nameRVA As DWORD Ptr
    Dim ordRVA  As WORD Ptr

    nameRVA = Allocate(expDir.NumberOfNames*4)
    ordRVA  = Allocate(expDir.NumberOfNames*2)

    ReadMem img, Cast(Any Ptr, modBase + expDir.AddressOfNames), nameRVA, expDir.NumberOfNames*4
    ReadMem img, Cast(Any Ptr, modBase + expDir.AddressOfNameOrdinals), ordRVA, expDir.NumberOfNames*2

    ' 找到 bestIndex 对应的名称
    For i As Integer = 0 To expDir.NumberOfNames-1
        If ordRVA[i] = bestIndex Then
            Dim namePtr As ULongInt = modBase + nameRVA[i]
            Dim buf As ZString * 256
            ReadMem img, Cast(Any Ptr, namePtr), @buf, 256
            bestName = buf
            Exit For
        End If
    Next

    bestOffset = rva - funcRVAs(bestIndex)

    Deallocate(nameRVA)
    Deallocate(ordRVA)

    Return bestName <> ""
End Function

Function GetProxyDescription(img As RemotePeImage, ByVal proxyVA As ULongInt) As String
    Dim modBase As ULongInt
    Dim modSize As ULongInt
    Dim modPath As String

    If GetModuleByVA(img, proxyVA, modBase, modSize, modPath) = False OrElse modPath = "" Then Return "0x" & Hex(proxyVA)
    'Print modPath
    Dim rva As ULongInt = proxyVA - modBase

    Dim strName As String
    Dim offset As ULongInt

    If ResolveFuncName(img, modBase, rva, strName, offset) = False Then Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) _ 
    & "!0x" & Hex(rva)

    Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & "!" & strName & "+0x" & Hex(offset)
End Function

Function GetTextSectionRange(img As RemotePeImage, ByRef startVA As ULongInt, ByRef endVA As ULongInt) As Boolean
    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As IMAGE_NT_HEADERS

    If GetRemoteNT(img, dos, nt) = FALSE Then Return FALSE

    Dim sec As IMAGE_SECTION_HEADER
    Dim secAddr As Any Ptr = img.baseAddr + dos.e_lfanew + SizeOf(IMAGE_NT_HEADERS)

    For i As Integer = 0 To nt.FileHeader.NumberOfSections - 1
        ReadMem(img, secAddr + i * SizeOf(IMAGE_SECTION_HEADER), @sec, SizeOf(sec))
        If Left(*Cast(ZString Ptr, @sec.Name(0)), 5) = ".text" Then
            startVA = Cast(ULongInt, img.baseAddr) + sec.VirtualAddress
            endVA   = startVA + sec.Misc.VirtualSize
            Return TRUE
        End If
    Next
    Return FALSE
End Function

Function AnalyzeInlineHook(img As RemotePeImage, funcVA As Any Ptr,ByRef info As InlineHookInfo) As Boolean
    info.hookType = ""
    info.proxyFunc = 0

    Dim buf(31) As UByte         ' 反汇编最多 32 字节
    Dim curInstrVA As ULongInt   ' 指令真实地址（用于 RIP）
    Dim nextVA As ULongInt       ' 跳转后地址

    curInstrVA = Cast(ULongInt, funcVA) ' 初始化为原 API 入口
    Print "curInstrVA:0x" & Hex(curInstrVA)

    Dim hopCount As Integer = 0
    Const MAX_HOPS = 8

    Do
        hopCount += 1
        If hopCount > MAX_HOPS Then
            info.hookType = "LoopJump"
            info.proxyFunc = curInstrVA
            Return True
        End If

        ' 读取当前指令
        If ReadMem(img, Cast(Any Ptr, curInstrVA), @buf(0), 32) = FALSE Then Return False

        '============================================================
        ' JMP rel32  : E9 xx xx xx xx
        '============================================================
        If buf(0) = &HE9 Then
            ' disp32 必须读取为 Long（有符号）
            Dim disp As Long = *Cast(Long Ptr, @buf(1))
            Print "disp32:" & disp & " " & WHex(disp)
            Dim disp64 As LongInt = disp
            Print "disp64:" & disp64 & " " & WHex(disp64)
            Dim nextVA1 As LongInt = CLngInt(curInstrVA) + 5 + disp64
            Print "nextVA1:" & WHex(nextVA1)
            Print "nextVA2:" & WHex(CLngInt(curInstrVA) + 5 + disp)
            Print "nextVA3:" & WHex(curInstrVA + 5 + disp64)
            Print "nextVA4:" & WHex(curInstrVA + 5 + disp)
            nextVA = CULngInt(nextVA1)
            For i As Integer = 0 To 15
                Print Hex(buf(i), 2) & " ";
            Next
            Print ""
            info.hookType = "jmp rel32"
            info.proxyFunc = nextVA
            Print "[jmp rel32] → "; GetProxyDescription(img, nextVA)

            curInstrVA = nextVA
            Continue Do
        End If

        '============================================================
        ' CALL rel32 : E8 xx xx xx xx
        '============================================================
        If buf(0) = &HE8 Then
            Dim disp As Long = *Cast(Long Ptr, @buf(1))
            nextVA = curInstrVA + 5 + CLngInt(disp)

            info.hookType = "call rel32"
            info.proxyFunc = nextVA

            curInstrVA = nextVA
            Continue Do
        End If

        '============================================================
        ' JMP [RIP+imm32] : FF 25 xx xx xx xx
        '============================================================
        If buf(0) = &HFF AndAlso buf(1) = &H25 Then
            Dim disp As Long = *Cast(Long Ptr, @buf(2))
            Dim ptrVA As ULongInt = curInstrVA + 6 + CLngInt(disp)

            If ReadMem(img, Cast(Any Ptr, ptrVA), @nextVA, 8) = FALSE Then Return False

            info.hookType = "jmp [rip]"
            info.proxyFunc = nextVA

            curInstrVA = nextVA
            Continue Do
        End If

        '============================================================
        ' CALL [RIP+imm32] : FF 15 xx xx xx xx
        '============================================================
        If buf(0) = &HFF AndAlso buf(1) = &H15 Then
            Dim disp As Long = *Cast(Long Ptr, @buf(2))
            Dim ptrVA As ULongInt = curInstrVA + 6 + CLngInt(disp)

            If ReadMem(img, Cast(Any Ptr, ptrVA), @nextVA, 8) = FALSE Then Return False

            info.hookType = "call [rip]"
            info.proxyFunc = nextVA

            curInstrVA = nextVA
            Continue Do
        End If

        '============================================================
        ' MOV RAX, imm64 ; JMP RAX  
        ' 48 B8 xx..xx FF E0
        '============================================================
        If buf(0) = &H48 AndAlso buf(1) = &HB8 Then
            Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))

            If buf(10)=&HFF AndAlso buf(11)=&HE0 Then
                info.hookType = "mov rax+jmp rax"
                info.proxyFunc = imm64

                curInstrVA = imm64
                Continue Do
            End If
        End If

        '============================================================
        ' FF E0 : jmp rax
        '============================================================
        If buf(0)=&HFF AndAlso buf(1)=&HE0 Then
            info.hookType = "jmp rax"
            info.proxyFunc = 0   ' 无法判断 RAX
            Return True
        End If

        '============================================================
        ' 无更多跳转 → 跳转链终止
        '============================================================
        If info.hookType <> "" Then
            info.proxyFunc = curInstrVA
            Return True
        End If

        info.hookType = "unknown"
        Return False

    Loop

    Return False
End Function

Function GetInlineHookList(ByRef img As RemotePeImage, funcVA As Any Ptr, diskEntry As UByte Ptr, ByVal FuncPath As StringW, lvwHook As Class_ListView) As ULong
    Dim textStart As ULongInt, textEnd As ULongInt
    If GetTextSectionRange(img, textStart, textEnd) Then
        If funcVA < textStart Or funcVA >= textEnd Then
            ' 不是可执行代码，不能是 inline hook
            'Print "代码不属于.text段!"
            Return 0
        End If
    End If

    Dim cur(15) As UByte
    If ReadMem(img, funcVA, @cur(0), 16) = 0 Then Return 0

    Dim diff As Boolean = False
    For i As Integer = 0 To 15
        If cur(i) <> diskEntry[i] Then
            diff = True
            Exit For
        End If
    Next

    If diff Then
        Dim curCode As StringW, oldCode As StringW
        'Print "[Inline Hook]"
        'Print "当前字节:";
        For i As Integer = 0 To 31
            curCode &= WHex(cur(i), 2) & " "
            'Print WHex(cur(i), 2) & " ";
        Next
        'Print ""
        'Print "原始字节:";
        For i As Integer = 0 To 31
            oldCode &= WHex(diskEntry[i], 2) & " "
            'Print WHex(diskEntry[i], 2) & " ";
        Next
        'Print ""
        Dim info As InlineHookInfo
        AnalyzeInlineHook img, funcVA, info
        'Print "inlinehook类型:" & info.hookType
        Dim ProxyDescription As StringW
        If info.proxyFunc <> NULL Then
            'Dim ModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, info.proxyFunc))
            'Dim ModuleName As StringW = RightW(ModulePath, LenW(ModulePath) - InStrRevW(ModulePath, "\"))
            'Dim FuncPos As String = GetFuncNameFromVA
            'Print "代理函数:" & GetProxyDescription(img, info.proxyFunc)
            ProxyDescription = GetProxyDescription(img, info.proxyFunc)
        Else
            'Print "代理函数:unknown"
            ProxyDescription = "unknown"
        End If
        lvwHook.AddItemColList,, "InlineHook(" & info.hookType & ")", "0x" & WHex(funcVA), FuncPath, "0x" & WHex(info.proxyFunc), ProxyDescription, curCode, oldCode
        /'Dim old As DWORD
        VirtualProtectEx(img.hProc, funcVA, 16, PAGE_EXECUTE_READWRITE, @old)
        WriteMem(img, funcVA, diskEntry, 16)
        VirtualProtectEx(img.hProc, funcVA, 16, old, @old)'/
    End If
End Function

Function GetIATHookList(ByRef img As RemotePeImage, diskImg As UByte Ptr, lvwHook As Class_ListView) As ULong
    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As IMAGE_NT_HEADERS

    If GetRemoteNT(img, dos, nt) = False Then Return 0

    Dim impRVA As DWORD = nt.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT).VirtualAddress
    If impRVA = 0 Then Return 0

    Dim impDes As IMAGE_IMPORT_DESCRIPTOR
    Dim impPtr As Any Ptr = img.baseAddr + impRVA

    Do
        ReadMem(img, impPtr, @impDes, SizeOf(IMAGE_IMPORT_DESCRIPTOR))
        If impDes.Name = 0 Then Exit Do

        Dim dllName As ZString * 256
        ReadMem(img, img.baseAddr + impDes.Name, @dllName, 256)

        Dim origThunk As IMAGE_THUNK_DATA
        Dim curThunk  As IMAGE_THUNK_DATA

        Dim pOrigThunk As Any Ptr = img.baseAddr + impDes.OriginalFirstThunk
        Dim pCurThunk As Any Ptr = img.baseAddr + impDes.FirstThunk

        Do
            ReadMem(img, pOrigThunk, @origThunk, SizeOf(IMAGE_THUNK_DATA))
            ReadMem(img, pCurThunk, @curThunk,  SizeOf(IMAGE_THUNK_DATA))

            If origThunk.u1.AddressOfData = 0 Then Exit Do

            If (origThunk.u1.Ordinal And &H80000000) = 0 Then
                Dim ibn As IMAGE_IMPORT_BY_NAME
                ReadMem(img, img.baseAddr + origThunk.u1.AddressOfData, @ibn, SizeOf(IMAGE_IMPORT_BY_NAME))

                Dim apiName As ZString * 256
                ReadMem(img, img.baseAddr + origThunk.u1.AddressOfData + 2, @apiName, 256)

                Dim rva As DWORD = GetDiskExportRVA(diskImg, apiName)
                If rva <> 0 Then
                    Dim realVA As ULongInt = Cast(ULongInt, img.baseAddr + rva)

                    If curThunk.u1.Function <> realVA Then
                        'Print "[IAT Hook]" & dllName & "!" & apiName
                        'Dim ModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, curThunk.u1.Function))
                        'Dim ModuleName As StringW = RightW(ModulePath, LenW(ModulePath) - InStrRevW(ModulePath, "\"))
                        'Print "代理函数:" & ModuleName & "!0x" & Hex(curThunk.u1.Function)
                        Dim ProxyDescription As StringW = GetProxyDescription(img, curThunk.u1.Function)
                        If (InStrW(dllName, "api-ms") > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 属于Windows API Sets机制,无需关心
                        
                        If (InStrW(ProxyDescription, dllName & "!" & LeftW(apiName, LenW(apiName) - 1)) > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then 
                            Goto ContinueDo ' 可能属于模块内部依赖/延迟加载,无需关心
                        End If
                        
                        'Print "代理函数:" & GetProxyDescription(img, curThunk.u1.Function)
                        lvwHook.AddItemColList,, "IAT Hook", "0x" & WHex(@curThunk.u1.Function), dllName & "!" & apiName, "0x" & WHex(curThunk.u1.Function), _
                        ProxyDescription, "0x" & WHex(curThunk.u1.Function), "0x" & WHex(realVA)
                        'WriteMem(img, tC, @realVA, 8)
                    End If
                End If
            End If
ContinueDo:
            pOrigThunk += SizeOf(IMAGE_THUNK_DATA)
            pCurThunk += SizeOf(IMAGE_THUNK_DATA)
            FF_DoEvents
        Loop

        impPtr += SizeOf(IMAGE_IMPORT_DESCRIPTOR)
    Loop
End Function

Function GetEATHookList(ByRef img As RemotePeImage, diskImg As UByte Ptr, ModuleName As StringW, lvwHook As Class_ListView) As ULong
    Dim dos As IMAGE_DOS_HEADER
    Dim nt As IMAGE_NT_HEADERS

    If GetRemoteNT(img, dos, nt) = False Then Return 0

    Dim expRVA As DWORD = nt.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    If expRVA = 0 Then Return 0

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    ReadMem(img, img.baseAddr + expRVA, @expDir, SizeOf(IMAGE_EXPORT_DIRECTORY))

    Dim funRva(expDir.NumberOfFunctions-1) As DWORD
    ReadMem(img, img.baseAddr + expDir.AddressOfFunctions, @funRva(0), expDir.NumberOfFunctions*4)

    Dim nameRva(expDir.NumberOfNames - 1) As DWORD
    Dim ordRva(expDir.NumberOfNames - 1) As WORD

    ReadMem(img, img.baseAddr + expDir.AddressOfNames, @nameRva(0), expDir.NumberOfNames*4)
    ReadMem(img, img.baseAddr + expDir.AddressOfNameOrdinals, @ordRva(0),  expDir.NumberOfNames*2)

    For i As Integer = 0 To expDir.NumberOfNames-1

        Dim nm As ZString * 256
        ReadMem(img, img.baseAddr + nameRva(i), @nm, 256)

        Dim idx As WORD = ordRva(i)
        Dim memRva As DWORD = funRva(idx)

        Dim rawRva As DWORD = GetDiskExportRVA(diskImg, nm)
        If rawRva <> 0 And rawRva <> memRva Then
            Print "[EAT Hook]" & nm
            Dim proxyRVA As DWORD = memRva ' funRva(i)
            Dim proxyFunc As ULongInt = Cast(ULongInt, img.BaseAddr) + proxyRVA
            /'Dim ModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, proxyFunc))
            Dim ModuleName As StringW = RightW(ModulePath, LenW(ModulePath) - InStrRevW(ModulePath, "\"))
            Print "代理函数:" & ModuleName & "!0x" & Hex(proxyFunc) '/
            Print "代理函数:" & GetProxyDescription(img, proxyFunc)
            lvwHook.AddItemColList,, "EAT Hook", "0x" & WHex(@funRva(idx)), ModuleName & "!" & nm, "0x" & WHex(proxyFunc), _
            GetProxyDescription(img, proxyFunc), "0x" & WHex(proxyFunc), "0x" & WHex(rawRva)
            'WriteMem(img, img.baseAddr + expDir.AddressOfFunctions + idx*4, @rawRva, 4) ' 修复EAT Hook
        End If
        FF_DoEvents
    Next
End Function

Function GetModuleHookList(ByVal pid As DWORD, ModuleName As StringW, ByVal ModuleHandle As Any Ptr, lvwHook As Class_ListView) As ULong
    Dim hProc As HANDLE = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, FALSE, pid)
    If hProc = NULL Then
        Print "OpenProcess failed."
        If IsDriverLoaded Then hProc = SnowSword_OpenProcess(pid)
        If hProc = NULL Then Return 0
    End If
    
    Dim img As RemotePeImage
    img.hProc = hProc
    img.BaseAddr  = ModuleHandle

    ' 加载磁盘 DLL
    Dim fullPath As ZString * MAX_PATH
    GetModuleFileNameExA(hProc, ModuleHandle, @fullPath, MAX_PATH)

    Dim sz As UInteger
    Dim diskImg As UByte Ptr = LoadDiskImage(fullPath, sz)
    If diskImg = NULL Then Return 0

    ' 导出表
    Dim names() As String
    RemoteDumpExports img, names()

    ' EAT
    GetEATHookList img, diskImg, ModuleName, lvwHook

    ' Inline
    For i As Integer = 0 To UBound(names)
        Dim rva As DWORD = GetDiskExportRVA(diskImg, names(i))
        If rva <> 0 Then
            Dim funcVA As Any Ptr = img.BaseAddr + rva
            Dim diskEntry As UByte Ptr = RvaToRaw(diskImg, rva)
            If diskEntry <> NULL Then
                GetInlineHookList img, funcVA, diskEntry, ModuleName & "!" & names(i), lvwHook
                FF_DoEvents
            End If
        End If
    Next

    ' IAT
    GetIATHookList img, diskImg, lvwHook

    'Print "修复完成: " & ModuleName

    Deallocate diskImg
    CloseHandle hProc
End Function

Sub GetProcessHookList(ByVal pid As DWORD, lvwHook As Class_ListView)
    Dim hProc As HANDLE = OpenProcess(PROCESS_VM_READ Or PROCESS_QUERY_INFORMATION, FALSE, pid)
    If hProc = NULL Then
        Print "OpenProcess failed."
        If IsDriverLoaded Then hProc = SnowSword_OpenProcess(pid)
        If hProc = NULL Then Exit Sub
    End If

    ' 获取远程模块基址
    Dim me32 As MODULEENTRY32
    me32.dwSize = SizeOf(MODULEENTRY32)
    Print "1"
    Dim hSnap As HANDLE = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)

    If Module32First(hSnap, @me32) <> 0 Then
        Do
            If me32.modBaseAddr = NULL Then Continue Do
            GetModuleHookList pid, me32.szModule, me32.modBaseAddr, lvwHook
            FF_DoEvents
        Loop While Module32Next(hSnap, @me32)
    End If
    CloseHandle hSnap
End Sub

Function GetObjectInfo(lvwObjectHook As Class_ListView) As ULong
    Dim ProcedureNames(7) As WString * 50 = { _
    "DumpProcedure", _
    "OpenProcedure", _
    "CloseProcedure", _
    "DeleteProcedure", _
    "ParseProcedure", _
    "SecurityProcedure", _
    "QueryNameProcedure", _
    "OkayToCloseProcedure" _
    }
    If Not IsDriverLoaded Then Return 0
    Print "1"
    Dim ObjInfo As OBJECT_INFORMATION
    Dim ObjIndex As ULong = 2
    Do While True
        SetLastError 0
        If IoControl(hDrv, IOCTL_GetObjectInfo, @ObjIndex, SizeOf(ULong), @ObjInfo, SizeOf(OBJECT_INFORMATION)) = 0 Then
            If GetLastError = ERROR_NO_MORE_ITEMS Then Exit Do
            
            Print "[GetObjectInfo]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
            ObjIndex += 1
            Continue Do
        End If
        For i As Integer = 0 To 7
            If ObjInfo.Procedure(i) <> NULL Then lvwObjectHook.AddItemColList,, ProcedureNames(i), _
"0x" & WHex(Cast(UInteger, ObjInfo.Procedure(i))), ObjInfo.ObjectName, GetDriverPathByAddr(Cast(ULONG64, ObjInfo.Procedure(i)))
        Next
        ObjIndex += 1
        FF_DoEvents
    Loop
    Return 0
End Function