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

Type OBJECT_INFORMATION
    ObjectName As Wstring * 50
    Procedure(7) As PVOID
End Type

Function ReadMem(ByVal hProc As HANDLE, addr As Any Ptr, buf As Any Ptr, sz As Integer) As Boolean
    Dim bytRead As SIZE_T
    If (ReadProcessMemory(hProc, addr, buf, sz, @bytRead) = 0) OrElse (bytRead < sz) Then
        MyLog.PrintWin32Error "ReadMem", "ReadProcessMemory", "addr:0x" & WHex(addr) & " read:" & bytRead & "/" & sz
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

' 新增：判断 RVA 是否落在代码节
Function IsRvaInCodeSection(buf As UByte Ptr, rva As DWORD) As Boolean
    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, buf)
    Dim fh As IMAGE_FILE_HEADER Ptr = CPtr(IMAGE_FILE_HEADER Ptr, buf + dos->e_lfanew + 4)

    Dim ntHeadersSize As Integer
    If fh->Machine = IMAGE_FILE_MACHINE_AMD64 Then
        ntHeadersSize = SizeOf(IMAGE_NT_HEADERS64)
    Else
        ntHeadersSize = SizeOf(IMAGE_NT_HEADERS32)
    End If

    Dim sec As IMAGE_SECTION_HEADER Ptr = _
        Cast(IMAGE_SECTION_HEADER Ptr, buf + dos->e_lfanew + ntHeadersSize)

    For i As Integer = 0 To fh->NumberOfSections - 1
        With sec[i]
            If rva >= .VirtualAddress AndAlso rva < .VirtualAddress + .Misc.VirtualSize Then
                Dim secName As String = *CPtr(ZString Ptr, @.Name(0))
                ' 只认代码节；数据节中的导出视为 forwarder
                If Left(secName, 5) = ".text" OrElse Left(secName, 4) = "CODE" Then
                    Return True
                Else
                    Return False
                End If
            End If
        End With
    Next
    Return False
End Function

Function GetDiskExportRVA(diskImg As UByte Ptr, apiName As String) As DWORD
    If diskImg = NULL Then Return 0
    
    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, diskImg)
    If dos->e_magic <> IMAGE_DOS_SIGNATURE Then Return 0
    
    ' 先读取NT头签名和Magic，判断PE类型
    Dim sig As DWORD
    sig = *CPtr(DWORD Ptr, diskImg + dos->e_lfanew)
    If sig <> IMAGE_NT_SIGNATURE Then Return 0
    
    Dim magic As WORD
    magic = *CPtr(WORD Ptr, diskImg + dos->e_lfanew + 24)
    
    Dim expRVA As DWORD = 0
    
    If magic = &H10B Then
        ' PE32 - 使用32位结构
        Dim nt32 As IMAGE_NT_HEADERS32 Ptr = CPtr(IMAGE_NT_HEADERS32 Ptr, diskImg + dos->e_lfanew)
        expRVA = nt32->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    ElseIf magic = &H20B Then
        ' PE64 - 使用64位结构
        Dim nt64 As IMAGE_NT_HEADERS64 Ptr = CPtr(IMAGE_NT_HEADERS64 Ptr, diskImg + dos->e_lfanew)
        expRVA = nt64->OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
    Else
        Return 0
    End If
    
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
                Dim rva As DWORD = funRva[idx]
                ' 过滤掉 forwarder（数据段导出）
                If IsRvaInCodeSection(diskImg, rva) = FALSE Then
                    Return 0
                End If
                Return rva
            End If
        End If
    Next
    
    Return 0
End Function

Function RvaToRaw(buf As UByte Ptr, rva As DWORD) As UByte Ptr
    Dim dos As IMAGE_DOS_HEADER Ptr = CPtr(IMAGE_DOS_HEADER Ptr, buf)
    If dos->e_magic <> IMAGE_DOS_SIGNATURE Then Return NULL

    Dim ntSig As DWORD Ptr = CPtr(DWORD Ptr, buf + dos->e_lfanew)
    If *ntSig <> IMAGE_NT_SIGNATURE Then Return NULL

    ' FileHeader 固定位于 Signature 之后 4 字节处，Machine 字段可安全读取
    Dim fh As IMAGE_FILE_HEADER Ptr = CPtr(IMAGE_FILE_HEADER Ptr, buf + dos->e_lfanew + 4)

    Dim ntHeadersSize As Integer
    If fh->Machine = IMAGE_FILE_MACHINE_AMD64 Then
        ntHeadersSize = SizeOf(IMAGE_NT_HEADERS64)
    Else
        ntHeadersSize = SizeOf(IMAGE_NT_HEADERS32)
    End If

    Dim sec As IMAGE_SECTION_HEADER Ptr = _
        Cast(IMAGE_SECTION_HEADER Ptr, buf + dos->e_lfanew + ntHeadersSize)

    For i As Integer = 0 To fh->NumberOfSections - 1
        With sec[i]
            If rva >= .VirtualAddress AndAlso rva < .VirtualAddress + .Misc.VirtualSize Then
                Return buf + .PointerToRawData + (rva - .VirtualAddress)
            End If
        End With
    Next

    Return NULL
End Function

' 检测指定 RVA 是否为导出转发（Forwarder Export）
Function IsForwarderExport(diskImg As UByte Ptr, rva As DWORD, imgSize As ULongInt) As Boolean
    If rva = 0 OrElse rva >= imgSize Then Return False
    
    ' 核心修复：原始文件缓冲区必须通过 RvaToRaw 转换
    ' 内存对齐映像中 RvaToRaw 同样有效（节表仍然保留）
    Dim pStr As UByte Ptr = RvaToRaw(diskImg, rva)
    If pStr = NULL Then
        ' 极少数情况：RVA 落在头部（如 DOS Stub）不在任何节内
        pStr = diskImg + rva
    End If
    
    ' 读取以 null 结尾的 ASCII 字符串（最长 255 字节）
    Dim fwdStr As String
    For i As Integer = 0 To 254
        Dim c As UByte = pStr[i]
        If c = 0 Then Exit For
        fwdStr += Chr(c)
    Next
    
    ' 调试时可临时打开确认
    'Print "[IsForwarderExport] fwdStr:" & fwdStr
    
    ' 转发字符串格式: DLLNAME.FunctionName 或 DLLNAME.#Ordinal
    Dim dotPos As Integer = InStr(fwdStr, ".")
    If dotPos < 2 OrElse dotPos >= Len(fwdStr) Then Return False
    
    ' 验证点号前的 DLL 名是否合法（字母、数字、-、_）
    For k As Integer = 0 To dotPos - 2
        Dim c As Integer = Asc(Mid(fwdStr, k + 1, 1))
        If Not ((c >= 65 And c <= 90) Or (c >= 97 And c <= 122) Or _
                (c >= 48 And c <= 57) Or c = 45 Or c = 95) Then
            Return False
        End If
    Next
    
    Return True
End Function

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
                         ByRef modSize As ULong) As StringW

    modBase = 0
    modSize = 0

    If addr = 0 Then Return ""

    Dim mbi As MEMORY_BASIC_INFORMATION
    If VirtualQueryEx(img.hProc, Cast(LPCVOID, Cast(UInteger, addr)), @mbi, SizeOf(mbi)) = 0 Then
        MyLog.PrintWin32Error "GetModuleByVA32", "VirtualQueryEx", "addr=" & WHex(addr)
        Return ""
    End If

    ' 必须是映像区域
    If mbi.Type <> MEM_IMAGE Then
        MyLog.PrintWin32Error "GetModuleByVA32", "VirtualQueryEx", "mbi.Type <> MEM_IMAGE"
        Return ""
    End If

    If mbi.AllocationBase = NULL Then Return ""

    modBase = Cast(ULong, Cast(ULONG_PTR, mbi.AllocationBase))

    ' 获取模块名
    Dim path As WString * MAX_PATH
    If GetModuleFileNameExW(img.hProc, mbi.AllocationBase, @path, MAX_PATH) = 0 Then
        MyLog.PrintWin32Error "GetModuleByVA32", "GetModuleFileNameExW", _
            "mbi.AllocationBase=" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    ' 读取 32 位 PE
    Dim dos As IMAGE_DOS_HEADER
    If Not ReadMem(img.hProc, Cast(Any Ptr, Cast(ULONG_PTR, modBase)), @dos, SizeOf(dos)) Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "1,addr=0x" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    If dos.e_magic <> IMAGE_DOS_SIGNATURE Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "Invalid DOS signature,addr=0x" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    If dos.e_lfanew < SizeOf(IMAGE_DOS_HEADER) OrElse dos.e_lfanew > &H100000 Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "Invalid e_lfanew=0x" & Hex(dos.e_lfanew)
        modBase = 0
        Return ""
    End If

    Dim nt32 As IMAGE_NT_HEADERS32
    Dim ntAddr As ULong
    ntAddr = modBase + Cast(ULong, dos.e_lfanew)

    If ReadMem(img.hProc, Cast(Any Ptr, Cast(ULONG_PTR, ntAddr)), @nt32, SizeOf(nt32)) = False Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "2,addr=0x" & WHex(ntAddr)
        modBase = 0
        Return ""
    End If

    If nt32.Signature <> IMAGE_NT_SIGNATURE Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "Invalid PE signature,addr=0x" & WHex(ntAddr)
        modBase = 0
        Return ""
    End If

    If nt32.OptionalHeader.Magic <> IMAGE_NT_OPTIONAL_HDR32_MAGIC Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA32", "ReadMem", _
            "Invalid PE32 magic=0x" & Hex(nt32.OptionalHeader.Magic)
        modBase = 0
        Return ""
    End If

    modSize = nt32.OptionalHeader.SizeOfImage

    If modSize = 0 Then
        modBase = 0
        Return ""
    End If

    ' 确认目标地址确实属于该 Image
    If addr < modBase Then
        modBase = 0
        modSize = 0
        Return ""
    End If

    If addr - modBase >= modSize Then
        modBase = 0
        modSize = 0
        Return ""
    End If

    Return path
End Function

Function GetModuleByVA64(img As RemotePeImageEx, ByVal addr As ULongInt, _
                         ByRef modBase As ULongInt, _
                         ByRef modSize As ULongInt) As StringW

    modBase = 0
    modSize = 0

    If addr = 0 Then Return ""

    Dim mbi As MEMORY_BASIC_INFORMATION

    If VirtualQueryEx(img.hProc, Cast(LPCVOID, addr), @mbi, SizeOf(MEMORY_BASIC_INFORMATION)) = 0 Then
        MyLog.PrintWin32Error "GetModuleByVA64", "VirtualQueryEx", _
            "addr=" & WHex(addr)
        Return ""
    End If

    ' 必须是 MEM_IMAGE
    If mbi.Type <> MEM_IMAGE Then
        MyLog.PrintWin32Error "GetModuleByVA64", "VirtualQueryEx", "mbi.Type <> MEM_IMAGE"
        Return ""
    End If

    If mbi.AllocationBase = NULL Then Return ""

    modBase = Cast(ULongInt, mbi.AllocationBase)

    ' 获取模块名
    Dim path As WString * MAX_PATH

    If GetModuleFileNameExW(img.hProc, mbi.AllocationBase, @path, MAX_PATH) = 0 Then
        MyLog.PrintWin32Error "GetModuleByVA64", "GetModuleFileNameExW", _
            "mbi.AllocationBase=" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    ' ------------------------------------------------------------
    ' 读取 DOS Header
    ' ------------------------------------------------------------

    Dim dos As IMAGE_DOS_HEADER

    If Not ReadMem(img.hProc, Cast(Any Ptr, modBase), @dos, SizeOf(IMAGE_DOS_HEADER)) Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "1,addr=0x" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    If dos.e_magic <> IMAGE_DOS_SIGNATURE Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "Invalid DOS signature,addr=0x" & WHex(modBase)
        modBase = 0
        Return ""
    End If

    ' 防止恶意/损坏 PE 导致越界地址
    If dos.e_lfanew < SizeOf(IMAGE_DOS_HEADER) OrElse _
       dos.e_lfanew > &H100000 Then

        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "Invalid e_lfanew=0x" & Hex(dos.e_lfanew)
        modBase = 0
        Return ""
    End If

    ' ------------------------------------------------------------
    ' 读取 NT Header
    ' ------------------------------------------------------------

    Dim nt As IMAGE_NT_HEADERS64
    Dim ntAddr As ULongInt

    ntAddr = modBase + Cast(ULongInt, dos.e_lfanew)

    If Not ReadMem(img.hProc, Cast(Any Ptr, ntAddr), @nt, SizeOf(IMAGE_NT_HEADERS64)) Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "2,addr=0x" & WHex(ntAddr)
        modBase = 0
        Return ""
    End If

    If nt.Signature <> IMAGE_NT_SIGNATURE Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "Invalid PE signature,addr=0x" & WHex(ntAddr)
        modBase = 0
        Return ""
    End If

    If nt.OptionalHeader.Magic <> IMAGE_NT_OPTIONAL_HDR64_MAGIC Then
        MyLog.PrintLog LOG_WARN, "GetModuleByVA64", "ReadMem", _
            "Invalid PE32+ magic=0x" & Hex(nt.OptionalHeader.Magic)
        modBase = 0
        Return ""
    End If

    modSize = nt.OptionalHeader.SizeOfImage

    If modSize = 0 Then
        modBase = 0
        Return ""
    End If

    ' ------------------------------------------------------------
    ' 确认目标地址属于当前 PE Image
    ' ------------------------------------------------------------

    If addr < modBase Then
        modBase = 0
        modSize = 0
        Return ""
    End If

    If addr - modBase >= modSize Then
        modBase = 0
        modSize = 0
        Return ""
    End If

    Return path
End Function

Function ResolveFuncName(ByRef img As RemotePeImageEx, ByVal modBase As ULongInt, _
                         ByVal rva As ULongInt, _
                         ByRef bestName As String, _
                         ByRef bestOffset As DWORD) As Boolean

    bestName = ""
    bestOffset = Cast(DWORD, rva)

    If modBase = 0 Then Return False

    If rva > &HFFFFFFFFULL Then Return False

    Dim dos As IMAGE_DOS_HEADER

    If Not ReadMem(img.hProc, Cast(Any Ptr, modBase), @dos, SizeOf(dos)) Then
        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "1,addr=0x" & WHex(modBase)
        Return False
    End If

    If dos.e_magic <> IMAGE_DOS_SIGNATURE Then Return False

    If dos.e_lfanew < SizeOf(IMAGE_DOS_HEADER) OrElse _
       dos.e_lfanew > &H100000 Then Return False

    Dim magic As WORD
    Dim ntAddr As ULongInt

    ntAddr = modBase + Cast(ULongInt, dos.e_lfanew)

    If Not ReadMem(img.hProc, Cast(Any Ptr, ntAddr + 24), @magic, 2) Then
        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "2,addr=0x" & WHex(ntAddr + 24)
        Return False
    End If

    Dim expRVA As DWORD
    Dim dataDir As IMAGE_DATA_DIRECTORY

    If magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC Then

        Dim nt32 As IMAGE_NT_HEADERS32

        If Not ReadMem(img.hProc, Cast(Any Ptr, ntAddr), @nt32, SizeOf(nt32)) Then
            MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
                "3,addr=0x" & WHex(ntAddr)
            Return False
        End If

        If nt32.Signature <> IMAGE_NT_SIGNATURE Then Return False

        expRVA = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
        dataDir = nt32.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)

    ElseIf magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC Then

        Dim nt64 As IMAGE_NT_HEADERS64

        If Not ReadMem(img.hProc, Cast(Any Ptr, ntAddr), @nt64, SizeOf(nt64)) Then
            MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
                "4,addr=0x" & WHex(ntAddr)
            Return False
        End If

        If nt64.Signature <> IMAGE_NT_SIGNATURE Then Return False

        expRVA = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT).VirtualAddress
        dataDir = nt64.OptionalHeader.DataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT)

    Else
        Return False
    End If

    If dataDir.VirtualAddress = 0 Then Return False
    If dataDir.SIZE < SizeOf(IMAGE_EXPORT_DIRECTORY) Then Return False

    If expRVA = 0 Then Return False

    Dim expDir As IMAGE_EXPORT_DIRECTORY

    If Not ReadMem( _
        img.hProc, _
        Cast(Any Ptr, modBase + Cast(ULongInt, expRVA)), _
        @expDir, _
        SizeOf(IMAGE_EXPORT_DIRECTORY)) Then

        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "5,addr=0x" & WHex(modBase + expRVA)
        Return False
    End If

    ' 防止损坏 PE 导致巨量 Allocate
    If expDir.NumberOfFunctions = 0 Then Return False
    If expDir.NumberOfFunctions > &H100000 Then Return False

    If expDir.NumberOfNames > &H100000 Then Return False

    If expDir.AddressOfFunctions = 0 Then Return False

    If expDir.NumberOfNames > 0 Then
        If expDir.AddressOfNames = 0 OrElse _
           expDir.AddressOfNameOrdinals = 0 Then Return False
    End If

    ' RVA 必须是 32 位范围
    If rva > &HFFFFFFFFULL Then Return False

    ' ------------------------------------------------------------
    ' 读取函数 RVA 表
    ' ------------------------------------------------------------

    Dim funcSize As ULongInt
    funcSize = Cast(ULongInt, expDir.NumberOfFunctions) * 4ULL

    If funcSize > &H400000ULL Then Return False

    Dim funcRVAs As DWORD Ptr

    funcRVAs = Allocate(Cast(ULong, funcSize))

    If funcRVAs = NULL Then Return False

    If Not ReadMem( _
        img.hProc, _
        Cast(Any Ptr, modBase + expDir.AddressOfFunctions), _
        funcRVAs, _
        Cast(ULONG, funcSize)) Then

        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "6,addr=0x" & WHex(modBase + expDir.AddressOfFunctions)

        Deallocate(funcRVAs)
        Return False
    End If

    ' ------------------------------------------------------------
    ' 找到 RVA <= 目标 RVA 中最大的函数
    ' ------------------------------------------------------------

    Dim bestIndex As Integer = -1

    For i As Integer = 0 To expDir.NumberOfFunctions - 1

        If funcRVAs[i] <> 0 Then

            If funcRVAs[i] <= Cast(DWORD, rva) Then

                If bestIndex = -1 OrElse _
                   funcRVAs[i] > funcRVAs[bestIndex] Then

                    bestIndex = i
                End If

            End If

        End If

    Next

    If bestIndex = -1 Then
        Deallocate(funcRVAs)
        Return False
    End If

    ' ------------------------------------------------------------
    ' 没有名称表
    ' ------------------------------------------------------------

    If expDir.NumberOfNames = 0 Then

        bestOffset = Cast(DWORD, rva) - funcRVAs[bestIndex]

        Deallocate(funcRVAs)

        Return False
    End If

    ' ------------------------------------------------------------
    ' 读取名称 RVA / Ordinal 表
    ' ------------------------------------------------------------

    Dim nameSize As ULongInt
    Dim ordSize As ULongInt

    nameSize = Cast(ULongInt, expDir.NumberOfNames) * 4ULL
    ordSize = Cast(ULongInt, expDir.NumberOfNames) * 2ULL

    If nameSize > &H400000ULL OrElse _
       ordSize > &H200000ULL Then

        Deallocate(funcRVAs)
        Return False
    End If

    Dim nameRVA As DWORD Ptr
    Dim ordRVA As WORD Ptr

    nameRVA = Allocate(Cast(ULong, nameSize))
    ordRVA = Allocate(Cast(ULong, ordSize))

    If nameRVA = NULL OrElse ordRVA = NULL Then

        If nameRVA <> NULL Then Deallocate(nameRVA)
        If ordRVA <> NULL Then Deallocate(ordRVA)

        Deallocate(funcRVAs)

        Return False
    End If

    If Not ReadMem( _
        img.hProc, _
        Cast(Any Ptr, modBase + expDir.AddressOfNames), _
        nameRVA, _
        Cast(ULONG, nameSize)) Then

        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "7,addr=0x" & WHex(modBase + expDir.AddressOfNames)

        Deallocate(nameRVA)
        Deallocate(ordRVA)
        Deallocate(funcRVAs)

        Return False
    End If

    If Not ReadMem( _
        img.hProc, _
        Cast(Any Ptr, modBase + expDir.AddressOfNameOrdinals), _
        ordRVA, _
        Cast(ULONG, ordSize)) Then

        MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
            "8,addr=0x" & WHex(modBase + expDir.AddressOfNameOrdinals)

        Deallocate(nameRVA)
        Deallocate(ordRVA)
        Deallocate(funcRVAs)

        Return False
    End If

    ' ------------------------------------------------------------
    ' 找到 bestIndex 对应名称
    ' ------------------------------------------------------------

    For i As Integer = 0 To expDir.NumberOfNames - 1

        If ordRVA[i] = bestIndex Then

            Dim namePtr As ULongInt

            namePtr = modBase + Cast(ULongInt, nameRVA[i])

            If nameRVA[i] = 0 Then Exit For

            Dim buf As ZString * 256

            ' 只读 255 字节，保留 buf(255) 作为强制终止符，防止远程无 \0 字符串越界
            If Not ReadMem( _
                img.hProc, _
                Cast(Any Ptr, namePtr), _
                @buf, _
                255) Then

                MyLog.PrintLog LOG_WARN, "ResolveFuncName", "ReadMem", _
                    "9,addr=0x" & WHex(namePtr)

                Exit For
            End If
            buf[255] = 0

            bestName = buf
            Exit For

        End If

    Next

    bestOffset = Cast(DWORD, rva) - funcRVAs[bestIndex]

    Deallocate(nameRVA)
    Deallocate(ordRVA)
    Deallocate(funcRVAs)

    Return bestName <> ""
End Function

Function GetProxyDescription32(img As RemotePeImageEx, ByVal proxyVA As ULong) As String

    If proxyVA = 0 Then Return "0x0"
    If proxyVA < &H10000 Then Return "0x" & Hex(proxyVA)

    Dim modBase As ULong
    Dim modSize As ULong
    Dim modPath As String

    modPath = GetModuleByVA32(img, proxyVA, modBase, modSize)

    If modPath = "" Then
        MyLog.PrintLog LOG_WARN, "GetProxyDescription32", _
            "GetModuleByVA32", "失败"
        Return "0x" & Hex(proxyVA)
    End If

    If proxyVA < modBase Then
        Return "0x" & Hex(proxyVA)
    End If

    Dim rva As ULong
    rva = proxyVA - modBase

    If rva >= modSize Then
        Return "0x" & Hex(proxyVA)
    End If

    Dim funcName As String
    Dim offset As DWORD

    If ResolveFuncName(img, modBase, rva, funcName, offset) = False Then
        Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & _
            "+0x" & Hex(rva)
    End If

    Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & _
        "!" & funcName & "+0x" & Hex(offset)
End Function

Function GetProxyDescription64(img As RemotePeImageEx, ByVal proxyVA As ULongInt) As String

    If proxyVA = 0 Then Return "0x0"
    If proxyVA < &H10000ULL Then Return "0x" & Hex(proxyVA)

    Dim modBase As ULongInt
    Dim modSize As ULongInt
    Dim modPath As String

    modPath = GetModuleByVA64(img, proxyVA, modBase, modSize)

    If modPath = "" Then
        MyLog.PrintLog LOG_WARN, "GetProxyDescription64", _
            "GetModuleByVA64", "失败"
        Return "0x" & Hex(proxyVA)
    End If

    If proxyVA < modBase Then
        Return "0x" & Hex(proxyVA)
    End If

    Dim rva As ULongInt
    rva = proxyVA - modBase

    If rva >= modSize Then
        Return "0x" & Hex(proxyVA)
    End If

    Dim strName As String
    Dim offset As DWORD

    If ResolveFuncName(img, modBase, rva, strName, offset) = False Then
        Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & _
            "+0x" & Hex(rva)
    End If

    Return Right(modPath, Len(modPath) - InStrRev(modPath, "\")) & _
        "!" & strName & "+0x" & Hex(offset)
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

Function GetRemoteModuleBitness(ByRef img As RemotePeImageEx) As Integer
    Dim dos As IMAGE_DOS_HEADER
    If Not ReadMem(img.hProc, img.baseAddr, @dos, SizeOf(dos)) Then Return 0

    Dim magic As WORD
    ReadMem img.hProc, img.baseAddr + dos.e_lfanew + 24, @magic, 2

    If magic = &H20B Then Return 64
    If magic = &H10B Then Return 32
    Return 0
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

Function ScanIAT64(ByRef img As RemotePeImageEx, diskImg As UByte Ptr, lvwHook As Class_ListView) As ULong
    Dim dos As IMAGE_DOS_HEADER
    Dim nt  As NT_HEADERS_UNION

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
                Dim apiName As ZString * 256
                ReadMem(img.hProc, img.baseAddr + origThunk.u1.AddressOfData + 2, @apiName, 256)

                Dim rva As DWORD = GetDiskExportRVA(diskImg, apiName)
                If rva = 0 Then Goto ContinueDo  ' 磁盘镜像中无此导出，跳过

                Dim realVA As ULongInt = Cast(ULongInt, img.baseAddr + rva)
                Dim allocBase As ULongInt

                ' === 基础过滤 ===
                If curThunk.u1.Function = realVA Then Goto ContinueDo
                If Not IsExecutableImageAddr(img.hProc, curThunk.u1.Function, allocBase) Then Goto ContinueDo

                Dim proxyFunc As ULongInt = curThunk.u1.Function
                Dim ProxyDescription As StringW = GetProxyDescription(img, proxyFunc)

                ' === API Set 过滤 ===
                If (InStr(dllName, "api-ms") > 0) AndAlso (InStr(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

                ' === 同模块转发过滤（核心修复）===
                Dim importBaseName As StringW = dllName
                Dim pos1 As Integer = InStrRevW(importBaseName, "\")
                If pos1 > 0 Then importBaseName = MidW(importBaseName, pos1 + 1)
                
                Dim proxyBaseName As StringW = ProxyDescription
                pos1 = InStrW(proxyBaseName, "!")
                If pos1 > 0 Then proxyBaseName = LeftW(proxyBaseName, pos1 - 1)
                
                ' 不区分大小写比较模块名
                If LCaseW(importBaseName) = LCaseW(proxyBaseName) Then
                    Goto ContinueDo
                End If

                ' === 系统 DLL 间导出验证 ===
                ' 如果代理模块能解析出同名导出函数，且地址匹配，则是正常转发
                Dim proxyModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, proxyFunc))
                If proxyModulePath <> "" Then
                    Dim DiskSize As UInteger = 0
                    Dim proxyDiskImg As UByte Ptr = LoadDiskImage(proxyModulePath, DiskSize)
                    If proxyDiskImg <> NULL Then
                        Dim proxyExportRVA As DWORD = GetDiskExportRVA(proxyDiskImg, apiName)
                        If proxyExportRVA <> 0 Then
                            Dim proxyBase As ULongInt = Cast(ULongInt, GetModuleBaseByPath(img.hProc, proxyModulePath))
                            If proxyBase <> 0 Then
                                Dim expectedProxyVA As ULongInt = proxyBase + proxyExportRVA
                                If proxyFunc = expectedProxyVA Then
                                    'FreeDiskImage(proxyDiskImg)
                                    Goto ContinueDo
                                End If
                            End If
                        End If
                        'FreeDiskImage(proxyDiskImg)
                    End If
                End If

                ' === 延迟加载/Wow64 API 转发过滤 ===
                If (InStr(ProxyDescription, apiName) <> 0) AndAlso (InStr(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

                ' 通过所有过滤，确认为 Hook
                AddItemColListEx lvwHook, 7,,, WStr("IAT Hook"), _
                    "0x" & WHex(@curThunk.u1.Function), _
                    WStr(dllName & "!" & apiName), _
                    "0x" & WHex(curThunk.u1.Function), _
                    WStr(ProxyDescription), _
                    "0x" & WHex(curThunk.u1.Function), _
                    "0x" & WHex(realVA)
            End If
ContinueDo:
            pOrigThunk += SizeOf(IMAGE_THUNK_DATA)
            pCurThunk += SizeOf(IMAGE_THUNK_DATA)
        Loop
        impPtr += SizeOf(IMAGE_IMPORT_DESCRIPTOR)
        FF_DoEvents
    Loop

    Return 0
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
                If rva = 0 Then Goto ContinueDo  ' 磁盘镜像中无此导出，跳过

                Dim realVA As ULongInt = Cast(ULongInt, img.baseAddr) + rva
                Dim allocBase As ULongInt

                ' === 基础过滤（与 64 位一致）===
                If ct.u1.Function = realVA Then Goto ContinueDo
                If Not IsExecutableImageAddr(img.hProc, ct.u1.Function, allocBase) Then Goto ContinueDo

                Dim proxyFunc As ULongInt = ct.u1.Function
                Dim ProxyDescription As StringW = GetProxyDescription(img, proxyFunc)

                ' === API Set 过滤 ===
                If (InStr(dllName, "api-ms") > 0) AndAlso (InStr(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

                ' === 同模块转发过滤（核心）===
                Dim importBaseName As StringW = dllName
                Dim pos1 As Integer = InStrRevW(importBaseName, "\")
                If pos1 > 0 Then importBaseName = MidW(importBaseName, pos1 + 1)

                Dim proxyBaseName As StringW = ProxyDescription
                pos1 = InStrW(proxyBaseName, "!")
                If pos1 > 0 Then proxyBaseName = LeftW(proxyBaseName, pos1 - 1)

                ' 不区分大小写比较模块名
                If LCaseW(importBaseName) = LCaseW(proxyBaseName) Then
                    Goto ContinueDo
                End If

                ' === Wow64 特殊过滤：32位 ntdll → 64位 ntdll 转发 ===
                ' 32位进程中，ntdll 的某些导出指向高地址（Wow64 转换层）
                If LCase(importBaseName) = "ntdll.dll" AndAlso proxyFunc > &H7FFFFFFF Then
                    ' 检查是否是已知的 Wow64 系统调用入口
                    'If IsWow64SystemServiceStub(img.hProc, proxyFunc) Then
                        Goto ContinueDo
                    'End If
                End If

                ' === 系统 DLL 间导出验证（与 64 位一致）===
                Dim proxyModulePath As StringW = GetModuleOfAddress(img.hProc, Cast(Any Ptr, proxyFunc))
                If proxyModulePath <> "" Then
                    Dim DiskSize As UInteger = 0
                    Dim proxyDiskImg As UByte Ptr = LoadDiskImage(proxyModulePath, DiskSize)
                    If proxyDiskImg <> NULL Then
                        Dim proxyExportRVA As DWORD = GetDiskExportRVA(proxyDiskImg, apiName)
                        If proxyExportRVA <> 0 Then
                            Dim proxyBase As ULongInt = Cast(ULongInt, GetModuleBaseByPath(img.hProc, proxyModulePath))
                            If proxyBase <> 0 Then
                                Dim expectedProxyVA As ULongInt = proxyBase + proxyExportRVA
                                If proxyFunc = expectedProxyVA Then
                                    'FreeDiskImage(proxyDiskImg)
                                    Goto ContinueDo
                                End If
                            End If
                        End If
                        'FreeDiskImage(proxyDiskImg)
                    End If
                End If

                ' === 延迟加载/Wow64 API 转发过滤 ===
                If (InStr(ProxyDescription, apiName) <> 0) AndAlso (InStr(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

                ' 通过所有过滤，确认为 Hook
                AddItemColListEx lvwHook, 7,,, WStr("IAT Hook"), _
                    "0x" & WHex(@ct.u1.Function), _
                    WStr(dllName & "!" & apiName), _
                    "0x" & WHex(ct.u1.Function), _
                    WStr(ProxyDescription), _
                    "0x" & WHex(ct.u1.Function), _
                    "0x" & WHex(realVA)
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

    Dim funRva(expDir.NumberOfFunctions - 1) As DWORD
    If Not ReadMem(img.hProc, img.baseAddr + expDir.AddressOfFunctions, @funRva(0), expDir.NumberOfFunctions*4) Then Return 0

    Dim nameRva(expDir.NumberOfNames - 1) As DWORD
    Dim ordRva(expDir.NumberOfNames - 1) As WORD

    ReadMem(img.hProc, img.baseAddr + expDir.AddressOfNames, @nameRva(0), expDir.NumberOfNames * 4)
    ReadMem(img.hProc, img.baseAddr + expDir.AddressOfNameOrdinals, @ordRva(0),  expDir.NumberOfNames * 2)

    ' 获取当前模块的基址和大小，用于范围检查
    Dim modBase As ULongInt = Cast(ULongInt, img.baseAddr)
    Dim modSize As ULongInt = 0
    ' 从 PE 头获取镜像大小
    If magic = &H10B Then
        Dim nt32 As IMAGE_NT_HEADERS32
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt32, SizeOf(nt32)
        modSize = nt32.OptionalHeader.SizeOfImage
    Else
        Dim nt64 As IMAGE_NT_HEADERS64
        ReadMem img.hProc, img.baseAddr + dos.e_lfanew, @nt64, SizeOf(nt64)
        modSize = nt64.OptionalHeader.SizeOfImage
    End If

    For i As Integer = 0 To expDir.NumberOfNames - 1

        Dim apiName As ZString * 256
        Dim idx As WORD, memRva As DWORD, rawRva As DWORD
        Dim proxyFunc As ULongInt = 0
        Dim ProxyDescription As StringW = ""
        Dim proxyModulePath As StringW = ""
        Dim proxyBaseName As StringW = ""
        Dim pos1 As Integer = 0
        Dim sysDlls(0 To 9) As String = { _
            "ntdll.dll", "kernel32.dll", "kernelbase.dll", _
            "user32.dll", "gdi32.dll", "advapi32.dll", _
            "combase.dll", "rpcrt4.dll", "ole32.dll", "oleaut32.dll"}
        Dim isSysDll As Boolean = False
        
        ReadMem(img.hProc, img.baseAddr + nameRva(i), @apiName, 256)

        idx = ordRva(i)
        memRva = funRva(idx)

        rawRva = GetDiskExportRVA(diskImg, apiName)
        If rawRva = 0 Then Goto ContinueDo  ' 磁盘镜像中无此导出，跳过
        If rawRva = memRva Then Goto ContinueDo ' 完全一致，不是 Hook
        
        ' === 核心修复：过滤合法的导出转发（Forwarder Export）===
        If IsForwarderExport(diskImg, rawRva, modSize) Then Goto ContinueDo

        proxyFunc = modBase + memRva

        ' === 获取代理函数所在模块 ===
        ProxyDescription = GetProxyDescription(img, proxyFunc)
        proxyModulePath = GetModuleOfAddress(img.hProc, Cast(Any Ptr, proxyFunc))
        proxyBaseName = proxyModulePath
        pos1 = InStrRevW(proxyBaseName, "\")
        If pos1 > 0 Then proxyBaseName = MidW(proxyBaseName, pos1 + 1)

        ' === 过滤 1: API Set ===
        If (InStrW(ModuleName, "api-ms") > 0) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

        ' === 过滤 2: 同模块内部重定向（核心修复）===
        /'If LCaseW(proxyBaseName) = LCaseW(ModuleName) Then
            ' 代理函数仍在当前模块内，是正常导出转发/Wow64转换
            Goto ContinueDo
        End If'/

        ' === 过滤 3: 代理地址在当前模块合法范围内 ===
        If proxyFunc >= modBase AndAlso proxyFunc < modBase + modSize Then
            ' 地址落在当前模块内，但不是 GetModuleOfAddress 识别的模块名
            ' 可能是模块内部的子区域（如 .text 中的不同段）
            Goto ContinueDo
        End If

        ' === 过滤 4: 延迟加载/Wow64 API 转发 ===
        'If (InStrW(ProxyDescription, ModuleName & "!") = 1) AndAlso (InStrW(ProxyDescription, "0x0") > 0) Then Goto ContinueDo

        ' === 过滤 5: 系统 DLL 的 Wow64 存根 ===
        /'For j As Integer = 0 To 9
            If LCase(ModuleName) = sysDlls(j) Then isSysDll = True : Exit For
        Next

        If isSysDll AndAlso proxyFunc < modBase + modSize Then
            ' 系统 DLL 的导出指向自身内部，正常
            Goto ContinueDo
        End If'/

        ' 通过所有过滤，确认为 Hook
        AddItemColListEx lvwHook, 7,,, WStr("EAT Hook"), _
            "0x" & WHex(@funRva(idx)), _
            WStr(ModuleName & "!" & apiName), _
            "0x" & WHex(proxyFunc), _
            WStr(ProxyDescription), _
            "0x" & WHex(proxyFunc), _
            "0x" & WHex(rawRva)

ContinueDo:
        FF_DoEvents
    Next

    Return 0
End Function

Function GetModuleHookList(ByVal ProcessId As DWORD, ModuleName As StringW, ByVal ModuleHandle As Any Ptr, lvwHook As Class_ListView) As ULong
    Dim hProc As HANDLE = NULL
    If IsDriverLoaded Then
        hProc = SnowSword_OpenProcess(ProcessId)
        If (hProc <= 0) Then
            MyLog.PrintWin32Error "GetModuleHookList", "SnowSword_OpenProcess", "ProcessId=" & ProcessId
            Return 0
        End If
    Else
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, ProcessId)
        If (hProc <= 0) Then
            MyLog.PrintWin32Error "GetModuleHookList", "OpenProcess", "ProcessId=" & ProcessId
            Return 0
        End If
    End If

    Dim img As RemotePeImageEx
    img.hProc = hProc
    img.BaseAddr = ModuleHandle
    
    img.arch = DetectRemoteModuleArch(img.hProc, img.baseAddr)

    ' 加载磁盘 DLL
    Dim fullPath As ZString * MAX_PATH = GetModuleOfAddress(hProc, ModuleHandle)

    Dim sz As UInteger
    Dim diskImg As UByte Ptr = LoadDiskImage(fullPath, sz)
    If diskImg = NULL Then CloseHandle hProc : Return 0

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

Sub GetProcessHookList(ByVal ProcessId As DWORD, lvwHook As Class_ListView)
    Dim hProc As HANDLE = NULL
    If IsDriverLoaded Then
        hProc = SnowSword_OpenProcess(ProcessId)
        If (hProc <= 0) Then
            MyLog.PrintWin32Error "GetModuleHookList", "SnowSword_OpenProcess", "ProcessId=" & ProcessId
            Return
        End If
    Else
        hProc = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, ProcessId)
        If (hProc <= 0) Then
            MyLog.PrintWin32Error "GetModuleHookList", "OpenProcess", "ProcessId=" & ProcessId
            Return
        End If
    End If
    
    InitListViewData lvwHook

    ' 获取远程模块基址
    Dim me32 As MODULEENTRY32
    me32.dwSize = SizeOf(MODULEENTRY32)
    Dim hSnap As HANDLE = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE Or TH32CS_SNAPMODULE32, ProcessId)

    If Module32First(hSnap, @me32) <> 0 Then
        Do
            'Print me32.szModule & " 0x" & WHex(me32.modBaseAddr)
            If me32.modBaseAddr = NULL Then Continue Do
            GetModuleHookList ProcessId, me32.szModule, me32.modBaseAddr, lvwHook
            FF_DoEvents
        Loop While Module32Next(hSnap, @me32)
    End If
    CloseHandle hSnap
    CloseHandle hProc
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
            
            'Print "[GetObjectInfo]IoControl:" & WinErrorMsg(GetLastError) & GetLastError
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