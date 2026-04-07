#include "windows.bi"

Enum PE_ARCH
    PE_ARCH_X86 = 1
    PE_ARCH_X64 = 2
End Enum

Type RemotePeImageEx
    hProc As HANDLE
    baseAddr As Any Ptr
    arch As PE_ARCH  ' 关键字段
End Type

Type EXPORT_VIEW
    NumberOfFunctions As DWORD
    NumberOfNames     As DWORD
    AddrFunctions     As DWORD
    AddrNames         As DWORD
    AddrOrdinals      As DWORD
End Type

Type NT_HEADERS_UNION
    nt32 As IMAGE_NT_HEADERS32
    nt64 As IMAGE_NT_HEADERS64
End Type

Type InlineHookInfo
    hookType As String
    proxyFunc  As ULongInt
End Type

Type OBJECT_INFORMATION
    ObjectName As Wstring * 50
    Procedure(7) As PVOID
End Type

'--消息钩子枚举--

'Const WH_MAX = 16

Type SHAREDINFO
    psi         As Any Ptr
    aheList     As Any Ptr
    HeEntrySize As ULong
End Type

Type SERVERINFO
    dwSRVIFlags As ULong
    cHandleEntries As ULong
    cbHandleTable As ULong
    apsi As Any Ptr        ' DESKTOPINFO**
End Type

Type DESKTOPINFO
    pvDesktopBase As Any Ptr
    pvDesktopLimit As Any Ptr
    aphkStart(WH_MAX - 1) As Any Ptr   ' tagHOOK*
End Type

Type TAGHOOK
    pNext   As Any Ptr
    pPrev   As Any Ptr
    pti     As Any Ptr
    iHook   As Long
    flags   As ULong
    offPfn  As ULong
    ihmod   As UShort
    _pad    As UShort
End Type

'----

Function ReadMem(ByVal hProc As HANDLE, addr As Any Ptr, buf As Any Ptr, sz As Integer) As Boolean
    Dim bytRead As SIZE_T
    If (ReadProcessMemory(hProc, addr, buf, sz, @bytRead) = 0) OrElse (bytRead < sz) Then
        Print "[ReadMem]" & "addr:" & WHex(addr) & " read:" & bytRead & "/" & sz & " " & WinErrorMsg(GetLastError) & GetLastError
        Return False
    Else
        Return True
    End If
End Function

Function WriteMem(ByRef img As RemotePeImageEx, addr As Any Ptr, buf As Any Ptr, sz As Integer) As Boolean
    Return WriteProcessMemory(img.hProc, addr, buf, sz, 0) <> 0
End Function

Function DetectRemoteModuleArch(hProc As HANDLE, baseAddr As Any Ptr) As PE_ARCH
    Dim dos As IMAGE_DOS_HEADER
    If ReadProcessMemory(hProc, baseAddr, @dos, SizeOf(dos), 0) = 0 Then
        Return 0
    End If

    Dim sig As DWORD
    ReadProcessMemory hProc, baseAddr + dos.e_lfanew, @sig, 4, 0
    If sig <> IMAGE_NT_SIGNATURE Then Return 0

    Dim magic As WORD
    ReadProcessMemory hProc, baseAddr + dos.e_lfanew + 4 + _
        SizeOf(IMAGE_FILE_HEADER), @magic, 2, 0

    If magic = &H10B Then
        Return PE_ARCH_X86   ' PE32
    ElseIf magic = &H20B Then
        Return PE_ARCH_X64   ' PE32+
    End If

    Return 0
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

Function GetRemoteNTEx(ByRef img As RemotePeImageEx, _
                       ByRef dos As IMAGE_DOS_HEADER, _
                       ByRef ntu As NT_HEADERS_UNION) As Boolean

    If Not ReadMem(img.hProc, img.baseAddr, @dos, SizeOf(dos)) Then Return False

    Select Case img.arch
    Case PE_ARCH_X86
        Return ReadMem(img.hProc, img.baseAddr + dos.e_lfanew, _
                          @ntu.nt32, SizeOf(IMAGE_NT_HEADERS32))
    Case PE_ARCH_X64
        Return ReadMem(img.hProc, img.baseAddr + dos.e_lfanew, _
                          @ntu.nt64, SizeOf(IMAGE_NT_HEADERS64))
    End Select

    Return False
End Function

Function ReadExportView(ByRef img As RemotePeImageEx, _
                        ByRef exportView As EXPORT_VIEW) As Boolean

    Dim dos As IMAGE_DOS_HEADER
    Dim ntu As NT_HEADERS_UNION
    If Not GetRemoteNTEx(img, dos, ntu) Then Return False

    Dim expRVA As DWORD
    If img.arch = PE_ARCH_X86 Then
        expRVA = ntu.nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    Else
        expRVA = ntu.nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    End If

    If expRVA = 0 Then Return False

    Dim raw(9) As DWORD
    If Not ReadMem(img.hProc, img.baseAddr + expRVA, @raw(0), 40) Then Return False

    exportView.NumberOfFunctions = raw(3)
    exportView.NumberOfNames     = raw(4)
    exportView.AddrFunctions     = raw(7)
    exportView.AddrNames         = raw(8)
    exportView.AddrOrdinals      = raw(9)

    ' 防御性校验
    If exportView.NumberOfFunctions > &H10000 Then Return False
    If exportView.NumberOfNames > &H10000 Then Return False

    Return True
End Function

Function RemoteRvaToVa(ByRef img As RemotePeImageEx, rva As DWORD) As Any Ptr
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

/'Function RemoteDumpExports(img As RemotePeImageEx, names() As String) As Integer

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
End Function'/

Function RemoteDumpExports(img As RemotePeImageEx, names() As String) As Integer

    Dim dos As IMAGE_DOS_HEADER
    If Not ReadMem(img.hProc, img.baseAddr, @dos, SizeOf(dos)) Then Return 0

    Dim sig As DWORD
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @sig, 4
    If sig <> &H4550 Then Return 0  ' PE\0\0

    Dim magic As WORD
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew + 24, @magic, 2

    Dim expRVA As DWORD

    If magic = &H10B Then
        ' PE32
        Dim nt32 As IMAGE_NT_HEADERS32
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt32, SizeOf(nt32)
        expRVA = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    ElseIf magic = &H20B Then
        ' PE64
        Dim nt64 As IMAGE_NT_HEADERS64
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt64, SizeOf(nt64)
        expRVA = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    Else
        Return 0
    End If

    If expRVA = 0 Then Return 0

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    If Not ReadMem(img.hProc, img.baseAddr + expRVA, @expDir, SizeOf(expDir)) Then Return 0

    If expDir.NumberOfNames = 0 Or expDir.NumberOfNames > &H10000 Then Return 0

    Dim nameRva(expDir.NumberOfNames - 1) As DWORD
    ReadMem img.hProc, img.baseAddr + expDir.AddressOfNames, _
            @nameRva(0), expDir.NumberOfNames * 4

    ReDim names(expDir.NumberOfNames - 1)

    For i As Integer = 0 To expDir.NumberOfNames - 1
        Dim buf(255) As UByte
        If ReadMem(img.hProc, img.baseAddr + nameRva(i), @buf(0), 255) Then
            names(i) = *Cast(ZString Ptr, @buf(0))
        End If
    Next

    Return expDir.NumberOfNames
End Function

Function GetModuleByVA32(img As RemotePeImageEx, ByVal addr As ULong, _
                         ByRef modBase As ULong, _
                         ByRef modSize As ULong, _
                         ByRef modName As String) As Boolean

    Dim mbi As MEMORY_BASIC_INFORMATION
    If VirtualQueryEx(img.hProc, Cast(LPCVOID, Cast(UInteger, addr)), @mbi, SizeOf(mbi)) = 0 Then Return False

    Dim path As ZString * MAX_PATH
    If GetModuleFileNameExA(img.hProc, mbi.AllocationBase, @path, MAX_PATH) = 0 Then
        modName = ""
    Else
        modName = path
    End If

    ' 读取 32 位 PE
    Dim dos As IMAGE_DOS_HEADER
    Dim nt32 As IMAGE_NT_HEADERS32

    If ReadMem(img.hProc, mbi.AllocationBase, @dos, SizeOf(dos)) = False Then Return False
    If ReadMem(img.hProc, mbi.AllocationBase + dos.e_lfanew, @nt32, SizeOf(nt32)) = False Then Return False

    modSize = nt32.OptionalHeader.SizeOfImage
    Return TRUE
End Function

Function GetModuleByVA64(img As RemotePeImageEx, ByVal addr As ULongInt, _
                       ByRef modBase As ULongInt, _
                       ByRef modSize As ULongInt, _
                       ByRef modName As String) As Boolean

    Dim mbi As MEMORY_BASIC_INFORMATION
    If VirtualQueryEx(img.hProc , Cast(LPCVOID, addr), @mbi, SizeOf(MEMORY_BASIC_INFORMATION)) = 0 Then Return False

    modBase = Cast(ULongInt, mbi.AllocationBase)

    ' 获取模块名
    Dim path As ZString * MAX_PATH
    Print img.hProc
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

    ReadMem img.hProc, mbi.AllocationBase, @dos, SizeOf(IMAGE_DOS_HEADER)
    ReadMem img.hProc, mbi.AllocationBase + dos.e_lfanew, @nt, SizeOf(IMAGE_NT_HEADERS)

    modSize = nt.OptionalHeader.SizeOfImage
    Return TRUE
End Function

Function ResolveFuncName(ByRef img As RemotePeImageEx, ByVal modBase As ULongInt, _
                        ByVal rva As ULongInt, _
                        ByRef bestName As String, _
                        ByRef bestOffset As DWORD) As Boolean

    bestName = ""
    bestOffset = rva  ' 默认偏移

    Dim dos As IMAGE_DOS_HEADER
    ReadMem img.hProc, Cast(Any Ptr, modBase), @dos, SizeOf(dos)

    Dim magic As WORD
    ReadMem img.hProc, Cast(Any Ptr, modBase + dos.e_lfanew + 24), @magic, 2 ' 可以用OffSetOf代替吗

    Dim expRVA As DWORD, dataDir As IMAGE_DATA_DIRECTORY

    If magic = &H10B Then
        Dim nt32 As IMAGE_NT_HEADERS32
        ReadMem img.hProc, Cast(Any Ptr, modBase + dos.e_lfanew), @nt32, SizeOf(nt32)
        expRVA = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
        dataDir = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)
    Else
        Dim nt64 As IMAGE_NT_HEADERS64
        ReadMem img.hProc, Cast(Any Ptr, modBase + dos.e_lfanew), @nt64, SizeOf(nt64)
        expRVA = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
        dataDir = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)
    End If

    If dataDir.VirtualAddress = 0 Then Return False

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    ReadMem img.hProc, Cast(Any Ptr, modBase + dataDir.VirtualAddress), @expDir, SizeOf(IMAGE_EXPORT_DIRECTORY)

    Dim funcRVAs(expDir.NumberOfFunctions-1) As DWORD
    ReadMem img.hProc, Cast(Any Ptr, modBase + expDir.AddressOfFunctions), @funcRVAs(0), expDir.NumberOfFunctions*4

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

    ReadMem img.hProc, Cast(Any Ptr, modBase + expDir.AddressOfNames), nameRVA, expDir.NumberOfNames*4
    ReadMem img.hProc, Cast(Any Ptr, modBase + expDir.AddressOfNameOrdinals), ordRVA, expDir.NumberOfNames*2

    ' 找到 bestIndex 对应的名称
    For i As Integer = 0 To expDir.NumberOfNames-1
        If ordRVA[i] = bestIndex Then
            Dim namePtr As ULongInt = modBase + nameRVA[i]
            Dim buf As ZString * 256
            ReadMem img.hProc, Cast(Any Ptr, namePtr), @buf, 256
            bestName = buf
            Exit For
        End If
    Next

    bestOffset = rva - funcRVAs(bestIndex)

    Deallocate(nameRVA)
    Deallocate(ordRVA)

    Return bestName <> ""
End Function

Function GetProxyDescription32(img As RemotePeImageEx, ByVal proxyVA As ULong) As String

    Dim modBase As ULong
    Dim modSize As ULong
    Dim modPath As String

    If GetModuleByVA32(img, proxyVA, modBase, modSize, modPath) = FALSE OrElse modPath = "" Then
        Return "0x" & Hex(proxyVA)
    End If

    Dim rva As ULong = proxyVA - modBase

    Dim funcName As String
    Dim offset As ULong

    If ResolveFuncName(img, modBase, rva, funcName, offset) = False Then
        Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & "!0x" & Hex(rva)
    End If

    Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & "!" & funcName & "+0x" & Hex(offset)
End Function

Function GetProxyDescription64(img As RemotePeImageEx, ByVal proxyVA As ULongInt) As String
    Dim modBase As ULongInt
    Dim modSize As ULongInt
    Dim modPath As String

    If GetModuleByVA64(img, proxyVA, modBase, modSize, modPath) = False OrElse modPath = "" Then Return "0x" & Hex(proxyVA)
    Print modPath
    Dim rva As ULongInt = proxyVA - modBase

    Dim strName As String
    Dim offset As ULong

    If ResolveFuncName(img, modBase, rva, strName, offset) = False Then Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) _ 
    & "!0x" & Hex(rva)

    Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & "!" & strName & "+0x" & Hex(offset)
End Function

Function GetProxyDescription(img As RemotePeImageEx, ByVal proxyVA As ULongInt) As String
    If img.arch = PE_ARCH_X86 Then
        Return GetProxyDescription32(img, Cast(ULong, proxyVA))
    Else
        Return GetProxyDescription64(img, proxyVA)
    End If
End Function

Sub GetTextSectionRange(ByRef img As RemotePeImageEx, _
                        ByRef textStart As ULongInt, _
                        ByRef textEnd As ULongInt)

    Dim dos As IMAGE_DOS_HEADER
    ReadMem img.hProc, img.baseAddr, @dos, SizeOf(dos)

    Dim secCount As WORD
    Dim secBase  As Any Ptr

    If img.arch = PE_ARCH_X86 Then
        Dim nt32 As IMAGE_NT_HEADERS32
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt32, SizeOf(nt32)
        secCount = nt32.FileHeader.NumberOfSections
        secBase = img.baseAddr + dos.e_lfanew + SizeOf(IMAGE_NT_HEADERS32)
    Else
        Dim nt64 As IMAGE_NT_HEADERS64
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt64, SizeOf(nt64)
        secCount = nt64.FileHeader.NumberOfSections
        secBase = img.baseAddr + dos.e_lfanew + SizeOf(IMAGE_NT_HEADERS64)
    End If

    Dim i As Integer
    For i = 0 To secCount - 1
        Dim sec As IMAGE_SECTION_HEADER
        ReadMem img.hProc, secBase + i * SizeOf(sec), @sec, SizeOf(sec)
        If sec.Name(0) = Asc(".") AndAlso sec.Name(1) = Asc("t") Then
            textStart = Cast(ULongInt, img.baseAddr + sec.VirtualAddress)
            textEnd   = textStart + sec.Misc.VirtualSize
            Exit Sub
        End If
    Next
End Sub

Function AnalyzeInlineHook(img As RemotePeImageEx, funcVA As Any Ptr,ByRef info As InlineHookInfo) As Boolean
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
        If ReadMem(img.hProc, Cast(Any Ptr, curInstrVA), @buf(0), 32) = False Then Return False

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
            Print "[jmp rel32] → " & GetProxyDescription(img, nextVA)

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

            If ReadMem(img.hProc, Cast(Any Ptr, ptrVA), @nextVA, 8) = False Then Return False

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

            If ReadMem(img.hProc, Cast(Any Ptr, ptrVA), @nextVA, 8) = False Then Return False

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

Function GetInlineHookList(ByRef img As RemotePeImageEx, funcVA As Any Ptr, diskEntry As UByte Ptr, ByVal FuncPath As StringW, lvwHook As Class_ListView) As ULong
    Dim textStart As ULongInt, textEnd As ULongInt
    GetTextSectionRange img, textStart, textEnd
    If funcVA < textStart Or funcVA >= textEnd Then
        ' 不是可执行代码，不能是 inline hook
        'Print "代码不属于.text段!"
        Return 0
    End If

    Dim cur(15) As UByte
    If ReadMem(img.hProc, funcVA, @cur(0), 16) = 0 Then Return 0

    Dim diff As Boolean = False
    For i As Integer = 0 To 15
        If cur(i) <> diskEntry[i] Then
            diff = True
            Exit For
        End If
    Next

    If diff Then
        Dim curCode As StringW, oldCode As StringW
        /'Print "[Inline Hook]"
        Print "当前字节:";'/
        For i As Integer = 0 To 31
            curCode &= WHex(cur(i), 2) & " "
            'Print WHex(cur(i), 2) & " ";
        Next
        /'Print ""
        Print "原始字节:";'/
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
        AddItemColListEx lvwHook, 7,,, WStr("InlineHook(" & info.hookType & ")"), "0x" & WHex(funcVA), WStr(FuncPath), "0x" & WHex(info.proxyFunc), WStr(ProxyDescription), WStr(curCode), WStr(oldCode)
        /'Dim old As DWORD
        VirtualProtectEx(img.hProc, funcVA, 16, PAGE_EXECUTE_READWRITE, @old)
        WriteMem(img, funcVA, diskEntry, 16)
        VirtualProtectEx(img.hProc, funcVA, 16, old, @old)'/
    End If
End Function

Function GetRemoteModuleBitness(ByRef img As RemotePeImageEx) As Integer
    Dim dos As IMAGE_DOS_HEADER
    If Not ReadMem(img.hProc, img.baseAddr, @dos, SizeOf(dos)) Then Return 0

    Dim magic As WORD
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew + 24, @magic, 2

    If magic = &H20B Then Return 64
    If magic = &H10B Then Return 32
    Return 0
End Function

Function ScanIAT64(ByRef img As RemotePeImageEx, diskImg As UByte Ptr, lvwHook As Class_ListView) As ULong
    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As NT_HEADERS_UNION' IMAGE_NT_HEADERS

    If GetRemoteNTEx(img, dos, nt) = False Then Return 0

    Dim impRVA As DWORD = nt.nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT).VirtualAddress
    If impRVA = 0 Then Return 0

    Dim impDes As IMAGE_IMPORT_DESCRIPTOR
    Dim impPtr As Any Ptr = img.baseAddr + impRVA

    Do
        ReadMem(img.hProc, impPtr, @impDes, SizeOf(IMAGE_IMPORT_DESCRIPTOR))
        If impDes.Name = 0 Then Exit Do

        Dim dllName As ZString * 256
        ReadMem(img.hProc, img.baseAddr + impDes.Name, @dllName, 256)

        Dim origThunk As IMAGE_THUNK_DATA
        Dim curThunk  As IMAGE_THUNK_DATA

        Dim pOrigThunk As Any Ptr = img.baseAddr + impDes.OriginalFirstThunk
        Dim pCurThunk As Any Ptr = img.baseAddr + impDes.FirstThunk

        Do
            ReadMem(img.hProc, pOrigThunk, @origThunk, SizeOf(IMAGE_THUNK_DATA))
            ReadMem(img.hProc, pCurThunk, @curThunk,  SizeOf(IMAGE_THUNK_DATA))

            If origThunk.u1.AddressOfData = 0 Then Exit Do

            If (origThunk.u1.Ordinal And &H80000000) = 0 Then
                Dim ibn As IMAGE_IMPORT_BY_NAME
                ReadMem(img.hProc, img.baseAddr + origThunk.u1.AddressOfData, @ibn, SizeOf(IMAGE_IMPORT_BY_NAME))

                Dim apiName As ZString * 256
                ReadMem(img.hProc, img.baseAddr + origThunk.u1.AddressOfData + 2, @apiName, 256)

                Dim rva As DWORD = GetDiskExportRVA(diskImg, apiName)
                If rva <> 0 Then
                    Dim realVA As ULongInt = Cast(ULongInt, img.baseAddr + rva)

                    If curThunk.u1.Function <> realVA Then
                        'Print "[IAT Hook]" & dllName & "!" & apiName
                        'Dim ModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, curThunk.u1.Function))
                        'Dim ModuleName As StringW = RightW(ModulePath, LenW(ModulePath) - InStrRevW(ModulePath, "\"))
                        'Print "代理函数:" & ModuleName & "!0x" & Hex(curThunk.u1.Function)
                        Dim ProxyDescription As StringW = GetProxyDescription(img, curThunk.u1.Function)
                        'Print "代理函数:" & GetProxyDescription(img, curThunk.u1.Function)
                        AddItemColListEx lvwHook, 7,,, WStr("IAT Hook"), "0x" & WHex(@curThunk.u1.Function), WStr(dllName & "!" & apiName), "0x" & WHex(curThunk.u1.Function), _
                        WStr(ProxyDescription), "0x" & WHex(curThunk.u1.Function), "0x" & WHex(realVA)
                        'WriteMem(img, tC, @realVA, 8)
                    End If
                End If
            End If

            pOrigThunk += SizeOf(IMAGE_THUNK_DATA)
            pCurThunk += SizeOf(IMAGE_THUNK_DATA)
        Loop

        impPtr += SizeOf(IMAGE_IMPORT_DESCRIPTOR)
    Loop
End Function

Function IsExecutableImageAddr(hProc As HANDLE, addr As ULongInt, _
                               ByRef allocBase As ULongInt) As Boolean
    Dim mbi As MEMORY_BASIC_INFORMATION
    If VirtualQueryEx(hProc, Cast(LPCVOID, addr), @mbi, SizeOf(mbi)) = 0 Then
        Return FALSE
    End If

    allocBase = Cast(ULongInt, mbi.AllocationBase)

    If mbi.State <> MEM_COMMIT Then Return FALSE
    If mbi.Type  <> MEM_IMAGE  Then Return FALSE

    Dim p As DWORD = mbi.Protect And &HFF
    If p <> PAGE_EXECUTE AndAlso _
       p <> PAGE_EXECUTE_READ AndAlso _
       p <> PAGE_EXECUTE_READWRITE AndAlso _
       p <> PAGE_EXECUTE_WRITECOPY Then
        Return FALSE
    End If

    Return TRUE
End Function

Function ScanIAT32(ByRef img As RemotePeImageEx, diskImg As UByte Ptr, lvwHook As Class_ListView) As ULong

    Dim dos As IMAGE_DOS_HEADER
    ReadMem img.hProc, img.baseAddr, @dos, SizeOf(dos)

    Dim nt As IMAGE_NT_HEADERS32
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt, SizeOf(nt)

    Dim impRVA As DWORD = nt.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT).VirtualAddress
    If impRVA = 0 Then Return 0

    Dim impDes As IMAGE_IMPORT_DESCRIPTOR
    Dim pImp As Any Ptr = img.baseAddr + impRVA

    Do
        ReadMem img.hProc, pImp, @impDes, SizeOf(impDes)
        If impDes.Name = 0 Then Exit Do

        Dim dllName As ZString * 256
        ReadMem img.hProc, img.baseAddr + impDes.Name, @dllName, 256

        Dim pOrg As Any Ptr = img.baseAddr + impDes.OriginalFirstThunk
        Dim pCur As Any Ptr = img.baseAddr + impDes.FirstThunk

        Dim ot As IMAGE_THUNK_DATA32, ct As IMAGE_THUNK_DATA32

        Do
            ReadMem img.hProc, pOrg, @ot, 4
            If ot.u1.AddressOfData = 0 Then Exit Do

            ReadMem img.hProc, pCur, @ct, 4

            If (ot.u1.Ordinal And &H80000000) = 0 Then
                Dim apiName As ZString * 256
                ReadMem img.hProc, img.baseAddr + ot.u1.AddressOfData + 2, @apiName, 256

                Dim rva As DWORD = GetDiskExportRVA(diskImg, apiName)
                If rva <> 0 Then
                    Dim realVA As ULongInt = Cast(ULongInt, img.baseAddr) + rva
                    Dim allocBase As ULongInt
                    If ct.u1.Function = realVA Then Goto ContinueDo
                    If Not IsExecutableImageAddr(img.hProc, ct.u1.Function, allocBase) Then Goto ContinueDo
                    
                    Dim proxyFunc As ULongInt = ct.u1.Function
                    Dim ProxyDescription As StringW = GetProxyDescription(img, proxyFunc)
                    If (InStrW(apiName, "api-ms") > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 属于Windows API Sets机制,无需关心
                    If (InStrW(ProxyDescription, apiName) <> 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 可能属于模块内部依赖/延迟加载/Wow64 API转发
                    
                    AddItemColListEx lvwHook, 7,,, WStr("IAT Hook"), _
                        "0x" & WHex(@ct.u1.Function), _
                        WStr(dllName & "!" & apiName), _
                        "0x" & WHex(ct.u1.Function), _
                        WStr(ProxyDescription), _
                        "0x" & WHex(ct.u1.Function), _
                        "0x" & WHex(realVA) 
                End If
            End If
ContinueDo:
            pOrg += 4 : pCur += 4
        Loop

        pImp += SizeOf(IMAGE_IMPORT_DESCRIPTOR)
        FF_DoEvents
    Loop

    Return 0
End Function

Function GetIATHookList(ByRef img As RemotePeImageEx, diskImg As UByte Ptr, lvwHook As Class_ListView) As ULong

    Dim bitness As Integer = GetRemoteModuleBitness(img)
    If bitness = 64 Then
        Return ScanIAT64(img, diskImg, lvwHook)
    ElseIf bitness = 32 Then
        Return ScanIAT32(img, diskImg, lvwHook)
    End If

    Return 0
End Function

Function GetEATHookList(ByRef img As RemotePeImageEx, diskImg As UByte Ptr, ModuleName As StringW, lvwHook As Class_ListView) As ULong
    Dim dos As IMAGE_DOS_HEADER
    ReadMem img.hProc, img.baseAddr, @dos, SizeOf(dos)

    Dim magic As WORD
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew + 24, @magic, 2

    Dim expRVA As DWORD

    If magic = &H10B Then
        Dim nt32 As IMAGE_NT_HEADERS32
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt32, SizeOf(nt32)
        expRVA = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    Else
        Dim nt64 As IMAGE_NT_HEADERS64
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt64, SizeOf(nt64)
        expRVA = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    End If

    If expRVA = 0 Then Return 0

    Dim expDir As IMAGE_EXPORT_DIRECTORY
    If Not ReadMem(img.hProc, img.baseAddr + expRVA, @expDir, SizeOf(IMAGE_EXPORT_DIRECTORY)) Then Return 0
    'Print "expDir.NumberOfFunctions:" & expDir.NumberOfFunctions
    
    Dim funRva(expDir.NumberOfFunctions - 1) As DWORD
    If Not ReadMem(img.hProc, img.baseAddr + expDir.AddressOfFunctions, @funRva(0), expDir.NumberOfFunctions*4) Then Return 0

    Dim nameRva(expDir.NumberOfNames - 1) As DWORD
    Dim ordRva(expDir.NumberOfNames - 1) As WORD

    ReadMem(img.hProc, img.baseAddr + expDir.AddressOfNames, @nameRva(0), expDir.NumberOfNames * 4)
    ReadMem(img.hProc, img.baseAddr + expDir.AddressOfNameOrdinals, @ordRva(0),  expDir.NumberOfNames * 2)

    For i As Integer = 0 To expDir.NumberOfNames - 1

        Dim apiName As ZString * 256
        ReadMem(img.hProc, img.baseAddr + nameRva(i), @apiName, 256)

        Dim idx As WORD = ordRva(i)
        Dim memRva As DWORD = funRva(idx)

        Dim rawRva As DWORD = GetDiskExportRVA(diskImg, apiName)
        If rawRva <> 0 And rawRva <> memRva Then
            'Print "[EAT Hook]" & apiName
            Dim proxyRVA As DWORD = memRva ' funRva(i)
            Dim proxyFunc As ULongInt = Cast(ULongInt, img.BaseAddr) + proxyRVA
            /'Dim ModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, proxyFunc))
            Dim ModuleName As StringW = RightW(ModulePath, LenW(ModulePath) - InStrRevW(ModulePath, "\"))
            Print "代理函数:" & ModuleName & "!0x" & Hex(proxyFunc) '/
            Dim ProxyDescription As StringW = GetProxyDescription(img, proxyFunc)
            'Print "代理函数:" & ProxyDescription
            If (InStrW(ModuleName, "api-ms") > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 属于Windows API Sets机制,无需关心
            'If (InStrW(ProxyDescription, ModuleName & "!" & LeftW(apiName, LenW(apiName) - 1)) > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 可能属于模块内部依赖/延迟加载
            If (InStrW(ProxyDescription, ModuleName & "!") = 1) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo ' 可能属于模块内部依赖/延迟加载/Wow64 API转发
            
            AddItemColListEx lvwHook, 7,,, WStr("EAT Hook"), "0x" & WHex(@funRva(idx)), WStr(ModuleName & "!" & apiName), "0x" & WHex(proxyFunc), _
            WStr(ProxyDescription), "0x" & WHex(proxyFunc), "0x" & WHex(rawRva)
            'WriteMem(img, img.baseAddr + expDir.AddressOfFunctions + idx*4, @rawRva, 4) ' 修复EAT Hook
ContinueDo:
        End If
        FF_DoEvents
    Next
End Function

Function GetModuleHookList(ByVal ProcessId As DWORD, ModuleName As StringW, ByVal ModuleHandle As Any Ptr, lvwHook As Class_ListView) As ULong
    Dim hProc As HANDLE = NULL
    If IsDriverLoaded Then
        hProc = SnowSword_OpenProcess(ProcessId)
    Else
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, ProcessId)
    End If
    If (hProc <= 0) Then
        Print "OpenProcess " & ProcessId & " Failed!"
        Return 0
    End If
    
    Dim img As RemotePeImageEx
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

    Print "修复完成: " & ModuleName

    Deallocate diskImg
    CloseHandle hProc
End Function

Sub GetProcessHookList(ByVal ProcessId As DWORD, lvwHook As Class_ListView)
    Dim hProc As HANDLE = NULL
    If IsDriverLoaded Then
        hProc = SnowSword_OpenProcess(ProcessId)
    Else
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, ProcessId)
    End If
    If (hProc <= 0) Then
        Print "OpenProcess " & ProcessId & " Failed!"
        Exit Sub
    End If
    
    InitListViewData lvwHook

    ' 获取远程模块基址
    Dim me32 As MODULEENTRY32
    me32.dwSize = SizeOf(MODULEENTRY32)
    Print "1"
    Dim hSnap As HANDLE = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE Or TH32CS_SNAPMODULE32, ProcessId)

    If Module32First(hSnap, @me32) <> 0 Then
        Do
            If me32.modBaseAddr = NULL Then Continue Do
            GetModuleHookList ProcessId, me32.szModule, me32.modBaseAddr, lvwHook
            FF_DoEvents
        Loop While Module32Next(hSnap, @me32)
    End If
    CloseHandle hSnap
    CommitListViewView lvwHook
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
    
    InitListViewData lvwObjectHook
    
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
            If ObjInfo.Procedure(i) <> NULL Then AddItemColListEx lvwObjectHook, 4,,, ProcedureNames(i), _
"0x" & WHex(Cast(UInteger, ObjInfo.Procedure(i))), ObjInfo.ObjectName, WStr(GetDriverPathByAddr(Cast(ULONG64, ObjInfo.Procedure(i))))
        Next
        ObjIndex += 1
        FF_DoEvents
    Loop
    CommitListViewView lvwObjectHook
    Return 0
End Function

'' 架构类型枚举
'Enum Architecture
'    ARCH_X86 = 32
'    ARCH_X64 = 64
'End Enum
'
'' Hook检测器基类（包含通用逻辑）
'Type HookAnalyzerBase
'    img As RemotePeImageEx
'    hProc As HANDLE
'    maxHops As Integer
'    visited() As ULongInt   ' 动态数组记录访问过的地址
'    
'    Declare Constructor(img As RemotePeImageEx)
'    Declare Function IsVAInCodeRange(va As ULongInt) As Boolean
'    Declare Function ReadRawBytes(va As ULongInt, buf As Any Ptr, size As Integer) As Boolean
'    Declare Function DetectLoop(va As ULongInt, hopIndex As Integer) As Boolean
'End Type
'
'Constructor HookAnalyzerBase(img As RemotePeImageEx)
'    This.img = img
'    This.hProc = img.hProc
'    This.maxHops = 8
'    ReDim This.visited(0 To This.maxHops-1)
'End Constructor
'
'Function HookAnalyzerBase.IsVAInCodeRange(va As ULongInt) As Boolean
'    ' 子类需要重写此方法
'    Return True
'End Function
'
'Function HookAnalyzerBase.ReadRawBytes(va As ULongInt, buf As Any Ptr, size As Integer) As Boolean
'    Return ReadMem(This.hProc, Cast(Any Ptr, va), buf, size)
'End Function
'
'Function HookAnalyzerBase.DetectLoop(va As ULongInt, hopIndex As Integer) As Boolean
'    For i As Integer = 0 To hopIndex-1
'        If This.visited(i) = va Then Return True
'    Next
'    This.visited(hopIndex) = va
'    Return False
'End Function
'
'' 32位专用分析器
'Type HookAnalyzer32 Extends HookAnalyzerBase
'    codeStart As ULongInt
'    codeEnd As ULongInt
'    
'    Declare Constructor(img As RemotePeImageEx)
'    Declare Function IsVAInCodeRange(va As ULongInt) As Boolean Override
'    Declare Function Analyze(funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'End Type
'
'Constructor HookAnalyzer32(img As RemotePeImageEx)
'    Base(img)
'    
'    ' 获取32位代码段范围
'    Dim codeSize As DWORD
'    If GetSizeOfCode32(img, @codeSize) Then
'        This.codeStart = img.pebImageBase + img.ntHeaders.OptionalHeader.BaseOfCode
'        This.codeEnd = This.codeStart + codeSize
'    End If
'End Constructor
'
'Function HookAnalyzer32.IsVAInCodeRange(va As ULongInt) As Boolean
'    Return (va >= This.codeStart AndAlso va < This.codeEnd)
'End Function
'
'' 64位专用分析器
'Type HookAnalyzer64 Extends HookAnalyzerBase
'    codeStart As ULongInt
'    codeEnd As ULongInt
'    
'    Declare Constructor(img As RemotePeImageEx)
'    Declare Function IsVAInCodeRange(va As ULongInt) As Boolean Override
'    Declare Function Analyze(funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'End Type
'
'Constructor HookAnalyzer64(img As RemotePeImageEx)
'    Base(img)
'    
'    ' 获取64位代码段范围
'    Dim codeSize As ULongInt
'    If GetSizeOfCode64(img, @codeSize) Then
'        This.codeStart = img.pebImageBase + img.ntHeaders.OptionalHeader.BaseOfCode
'        This.codeEnd = This.codeStart + codeSize
'    End If
'End Constructor
'
'Function HookAnalyzer64.IsVAInCodeRange(va As ULongInt) As Boolean
'    Return (va >= This.codeStart AndAlso va < This.codeEnd)
'End Function
'
'' 统一入口函数
'Function AnalyzeInlineHook(img As RemotePeImageEx, funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'    Dim arch As Architecture = DetectArchitecture(img)
'    
'    Select Case arch
'        Case ARCH_X86
'            Dim analyzer32 As HookAnalyzer32 = HookAnalyzer32(img)
'            Return analyzer32.Analyze(funcVA, info)
'            
'        Case ARCH_X64
'            Dim analyzer64 As HookAnalyzer64 = HookAnalyzer64(img)
'            Return analyzer64.Analyze(funcVA, info)
'            
'        Case Else
'            info.hookType = "unsupported_arch"
'            Return False
'    End Select
'End Function
'
'' 自动检测架构
'Function DetectArchitecture(img As RemotePeImageEx) As Architecture
'    ' 检查PE头魔数
'    If img.ntHeaders.OptionalHeader.Magic = &H10B Then
'        Return ARCH_X86
'    ElseIf img.ntHeaders.OptionalHeader.Magic = &H20B Then
'        Return ARCH_X64
'    End If
'    
'    ' 备用方案：通过进程句柄查询
'    Dim isWow64 As BOOL
'    If IsWow64Process(img.hProc, @isWow64) Then
'        Return IIf(isWow64, ARCH_X86, ARCH_X64)
'    End If
'    
'    Return 0
'End Function
'
'Function HookAnalyzer32.Analyze(funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'    info.hookType = ""
'    info.proxyFunc = 0
'    
'    Dim curVA As ULongInt = Cast(ULongInt, funcVA)
'    Dim buf(31) As UByte
'    
'    For hop As Integer = 0 To This.maxHops-1
'        ' 1. 循环检测
'        If This.DetectLoop(curVA, hop) Then
'            info.hookType = "LoopJump"
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        ' 2. 边界检查
'        If Not This.IsVAInCodeRange(curVA) Then
'            info.hookType = "OutOfCodeRange"
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        ' 3. 读取指令
'        If Not This.ReadRawBytes(curVA, @buf(0), 32) Then Return False
'        
'        ' 4. 模式匹配（32位专用）
'        Select Case As Const buf(0)
'        ' JMP rel8
'        Case &HEB
'            Dim disp As Byte = *Cast(Byte Ptr, @buf(1))
'            curVA = curVA + 2 + CLngInt(disp)
'            info.hookType = "jmp rel8"
'            Continue For
'            
'        ' JMP rel32
'        Case &HE9
'            Dim disp As Long = *Cast(Long Ptr, @buf(1))
'            curVA = curVA + 5 + CLngInt(disp)
'            info.hookType = "jmp rel32"
'            Continue For
'            
'        ' CALL rel32
'        Case &HE8
'            Dim disp As Long = *Cast(Long Ptr, @buf(1))
'            curVA = curVA + 5 + CLngInt(disp)
'            info.hookType = "call rel32"
'            Continue For
'            
'        ' JMP [mem32] : FF 25 xx xx xx xx
'        Case &HFF
'            If buf(1) = &H25 Then
'                Dim disp As Long = *Cast(Long Ptr, @buf(2))
'                Dim targetPtr As ULongInt = curVA + 6 + CLngInt(disp)
'                
'                ' 32位下读取4字节地址
'                Dim targetVA As ULongInt
'                If Not This.ReadRawBytes(targetPtr, @targetVA, 4) Then Return False
'                
'                curVA = targetVA
'                info.hookType = "jmp [mem32]"
'                Continue For
'            End If
'            
'            ' CALL [mem32] : FF 15 xx xx xx xx
'            If buf(1) = &H15 Then
'                Dim disp As Long = *Cast(Long Ptr, @buf(2))
'                Dim targetPtr As ULongInt = curVA + 6 + CLngInt(disp)
'                
'                Dim targetVA As ULongInt
'                If Not This.ReadRawBytes(targetPtr, @targetVA, 4) Then Return False
'                
'                curVA = targetVA
'                info.hookType = "call [mem32]"
'                Continue For
'            End If
'            
'        ' PUSH + RET
'        Case &H68  ' PUSH imm32
'            Dim imm As ULongInt = *Cast(ULongInt Ptr, @buf(1))
'            If buf(5) = &HC3 Then  ' RET
'                curVA = imm
'                info.hookType = "push+ret"
'                Continue For
'            End If
'            
'        End Select
'        
'        ' 5. 无匹配指令，跳转链终止
'        If info.hookType <> "" Then
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        info.hookType = "unknown"
'        Return False
'    Next
'    
'    info.hookType = "DeepChain"
'    info.proxyFunc = curVA
'    Return True
'End Function
'
'Function HookAnalyzer64.Analyze(funcVA As Any Ptr, ByRef info As InlineHookInfo) As Boolean
'    info.hookType = ""
'    info.proxyFunc = 0
'    
'    Dim curVA As ULongInt = Cast(ULongInt, funcVA)
'    Dim buf(31) As UByte
'    
'    For hop As Integer = 0 To This.maxHops-1
'        If This.DetectLoop(curVA, hop) Then
'            info.hookType = "LoopJump"
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        If Not This.IsVAInCodeRange(curVA) Then
'            info.hookType = "OutOfCodeRange"
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        If Not This.ReadRawBytes(curVA, @buf(0), 32) Then Return False
'        
'        ' 64位模式匹配（支持RIP相对寻址）
'        Select Case As Const buf(0)
'        ' REX.W + MOV RAX, imm64
'        Case &H48
'            If buf(1) = &HB8 Then
'                Dim imm64 As ULongInt = *Cast(ULongInt Ptr, @buf(2))
'                
'                ' 检查后续是否为 JMP RAX (FF E0)
'                If buf(10) = &HFF AndAlso buf(11) = &HE0 Then
'                    curVA = imm64
'                    info.hookType = "mov rax+jmp rax"
'                    Continue For
'                End If
'                
'                ' 检查 MOV RAX, imm64 + JMP [RAX]
'                If buf(10) = &HFF AndAlso buf(11) = &H20 Then
'                    Dim targetVA As ULongInt
'                    If Not This.ReadRawBytes(imm64, @targetVA, 8) Then Return False
'                    curVA = targetVA
'                    info.hookType = "mov rax+jmp [rax]"
'                    Continue For
'                End If
'            End If
'            
'        ' JMP rel32 (E9)
'        Case &HE9
'            Dim disp As Long = *Cast(Long Ptr, @buf(1))
'            curVA = curVA + 5 + CLngInt(disp)
'            info.hookType = "jmp rel32"
'            Continue For
'            
'        ' JMP [RIP+imm32] (FF 25)
'        Case &HFF
'            If buf(1) = &H25 Then
'                Dim disp As Long = *Cast(Long Ptr, @buf(2))
'                Dim targetPtr As ULongInt = curVA + 6 + CLngInt(disp)
'                
'                Dim targetVA As ULongInt
'                If Not This.ReadRawBytes(targetPtr, @targetVA, 8) Then Return False
'                
'                curVA = targetVA
'                info.hookType = "jmp [rip]"
'                Continue For
'            End If
'            
'            ' CALL [RIP+imm32] (FF 15)
'            If buf(1) = &H15 Then
'                Dim disp As Long = *Cast(Long Ptr, @buf(2))
'                Dim targetPtr As ULongInt = curVA + 6 + CLngInt(disp)
'                
'                Dim targetVA As ULongInt
'                If Not This.ReadRawBytes(targetPtr, @targetVA, 8) Then Return False
'                
'                curVA = targetVA
'                info.hookType = "call [rip]"
'                Continue For
'            End If
'            
'        ' ... 其他64位特殊模式
'            
'        End Select
'        
'        If info.hookType <> "" Then
'            info.proxyFunc = curVA
'            Return True
'        End If
'        
'        info.hookType = "unknown"
'        Return False
'    Next
'    
'    info.hookType = "DeepChain"
'    info.proxyFunc = curVA
'    Return True
'End Function



