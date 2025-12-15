#include Once "win/imagehlp.bi"
#include Once "win/winbase.bi"
'Extern "C"
Declare Function RtlGetVersion Lib "ntdll.dll" Alias "RtlGetVersion"(ByVal pVersion As PRTL_OSVERSIONINFOEXW) As NTSTATUS
'End Extern

Type SYMBOL_CONTEXT
    FuncAddr As ULONG64 Ptr
    pFuncName As LPWSTR
End Type

' 初始化函数 - 获取Dll基址和导出表
Private Function GetDllExports(ByVal DllName As String, ByRef exportDir As IMAGE_EXPORT_DIRECTORY Ptr) As PVOID
    ' 获取Dll模块句柄
    Dim dllBase As PVOID
    dllBase = LoadLibrary(DllName)
    'dllBase = GetModuleHandle(DllName)
    If dllBase = NULL Then
        Print "无法获取ntdll.dll模块句柄"
        Return NULL
    End If
    
    ' 获取DOS头
    Dim dosHeader As IMAGE_DOS_HEADER Ptr = Cast(IMAGE_DOS_HEADER Ptr, dllBase)
    If dosHeader->e_magic <> &H5A4D Then ' "MZ"
        Print "无效的PE文件"
        Return NULL
    End If
    
    ' 获取NT头
    Dim ntHeaders As IMAGE_NT_HEADERS64 Ptr = Cast(IMAGE_NT_HEADERS64 Ptr, Cast(Byte Ptr, dllBase) + dosHeader->e_lfanew)
    If ntHeaders->Signature <> &H4550 Then ' "PE"
        Print "无效的PE签名"
        Return NULL
    End If
    
    ' 获取导出表目录
    Dim exportDirRVA As Long = ntHeaders->OptionalHeader.DataDirectory(0).VirtualAddress
    If exportDirRVA = 0 Then
        Print "没有导出表"
        Return NULL
    End If
    
    ' 计算导出表实际地址
    exportDir = Cast(IMAGE_EXPORT_DIRECTORY Ptr, Cast(Byte Ptr, dllBase) + exportDirRVA)
    
    Return dllBase
End Function

' 检查函数代码是否匹配系统调用模式
Private Function IsSyscallFunction(ByVal functionAddr As PVOID, ByRef syscallIndex As DWORD) As Boolean
    ' 系统调用模式:
    ' mov r10, rcx    ; 4C 8B D1
    ' mov eax, XX XX XX XX ; B8 XX XX XX XX
    
    Dim code As UByte Ptr = CPtr(UByte Ptr, functionAddr)
    
    ' 检查前3个字节是否为 "mov r10, rcx" (4C 8B D1)
    If code[0] = &H4C And code[1] = &H8B And code[2] = &HD1 Then
        ' 检查第4个字节是否为 "mov eax, imm32" (B8)
        If code[3] = &HB8 Then
            ' 提取系统调用索引 (接下来的4个字节，小端序)
            syscallIndex = *CPtr(DWORD Ptr, @code[4])
            Return True
        End If
    End If
    
    Return False
End Function

' 根据系统调用索引查找函数名称
Function FindFunctionNameBySyscallIndex(ByVal DllName As String, ByVal targetIndex As DWORD) As String
    Dim dllBase As PVOID, exportDir As IMAGE_EXPORT_DIRECTORY Ptr
    dllBase = GetDllExports(DllName, exportDir)
    If dllBase = NULL OrElse exportDir = NULL Then Return "" 
    
    ' 获取导出表的各种数组
    Dim functions As DWORD Ptr = Cast(DWORD Ptr, Cast(Byte Ptr, dllBase) + exportDir->AddressOfFunctions)
    Dim names As DWORD Ptr = Cast(DWORD Ptr, Cast(Byte Ptr, dllBase) + exportDir->AddressOfNames)
    Dim ordinals As UShort Ptr = Cast(UShort Ptr, Cast(Byte Ptr, dllBase) + exportDir->AddressOfNameOrdinals)
    
    ' 遍历所有导出函数
    For i As Integer = 0 To exportDir->NumberOfNames - 1
        ' 获取函数名称
        Dim namePtr As ZString Ptr = Cast(ZString Ptr, Cast(Byte Ptr, dllBase) + names[i])
        Dim functionName As String = *namePtr
        
        ' 获取函数地址
        Dim functionRVA As DWORD = functions[ordinals[i]]
        Dim functionAddr As PVOID = Cast(Byte Ptr, dllBase) + functionRVA
        ' 检查是否为系统调用函数
        Dim syscallIndex As DWORD = 0
        Dim code As UByte Ptr = CPtr(UByte Ptr, functionAddr)
        ' 如果找到匹配的系统调用索引
        If (code[0] = &H4C) AndAlso (code[1] = &H8B) AndAlso (code[2] = &HD1) _
        AndAlso (code[3] = &HB8) AndAlso (*CPtr(DWORD Ptr, @code[4]) = targetIndex) Then Return functionName
    Next
    
    Return ""
End Function

'获取系统版本号
Function GetSystemVersion() As String
    Dim number As ULong = 0
    Dim osvi As RTL_OSVERSIONINFOEXW
    osvi.dwOSVersionInfoSize = SizeOf(OSVERSIONINFOW)
    If NT_SUCCESS(RtlGetVersion(@osvi)) Then
        Select Case osvi.dwMajorVersion
            Case 10
                ' Windows 10 或 Windows 11
                If osvi.dwBuildNumber >= 22000 Then
                    ' Windows 11
                    Select Case osvi.dwBuildNumber
                        Case 22000
                            Return "Windows 11 21H2"
                        Case 22621
                            Return "Windows 11 22H2"
                        Case 22631
                            Return "Windows 11 23H2"
                        Case 26100
                            Return "Windows 11 24H2"
                        Case Else
                            Return "Windows 11 (版本号:" & osvi.dwBuildNumber & ")"
                    End Select
                Else
                    ' Windows 10
                    Select Case osvi.dwBuildNumber
                        Case 10240
                            Return "Windows 10 1507"
                        Case 14393
                            Return "Windows 10 1607"
                        Case 15063
                            Return "Windows 10 1703"
                        Case 16299
                            Return "Windows 10 1709"
                        Case 17134
                            Return "Windows 10 1803"
                        Case 17763
                            Return "Windows 10 1809"
                        Case 18362
                            Return "Windows 10 1903"
                        Case 18363
                            Return "Windows 10 1909"
                        Case 19041
                            Return "Windows 10 2004"
                        Case 19042
                            Return "Windows 10 20H2"
                        Case 19043
                            Return "Windows 10 21H1"
                        Case 19044
                            Return "Windows 10 21H2"
                        Case 19045
                            Return "Windows 10 22H2"
                        Case Else
                            Return "Windows 10 (版本号:" & osvi.dwBuildNumber & ")"
                    End Select
                End If
            Case 6
                Select Case osvi.dwMinorVersion
                    Case 3
                        Return "Windows 8.1"
                    Case 2
                        Return "Windows 8"
                    Case 1
                        Return "Windows 7"
                End Select
            Case 5
                Select Case osvi.dwMinorVersion
                    Case 1
                        Return "Windows XP"
                    Case 2
                        Return "Windows Server 2003"
                End Select
        End Select
    Else
        Return "无法获取系统版本信息"
    End If
End Function

Function EnumSymCallBack(ByVal pSymInfo As PSYMBOL_INFO, ByVal SymbolSize As DWORD, ByVal UserContext As PVOID) As BOOL
    Dim pSymbolContext As SYMBOL_CONTEXT Ptr = CPtr(SYMBOL_CONTEXT Ptr, UserContext)
    /'If (* (pSymbolContext->FuncAddr) <> NULL) AndAlso (pSymInfo->ADDRESS = * (pSymbolContext->FuncAddr)) Then
        * (pSymbolContext->pFuncName) = pSymInfo->Name
        Print "0x" & WHex(pSymInfo->ADDRESS) & "->" & pSymInfo->Name
        Return False
    ElseIf (pSymbolContext->pFuncName <> NULL) AndAlso (pSymInfo->Name = * (pSymbolContext->pFuncName)) Then
        * (pSymbolContext->FuncAddr) = pSymInfo->ADDRESS
        Print * (pSymbolContext->pFuncName) & "->" & "0x" & WHex(pSymInfo->ADDRESS)
        Return False
    Else
        Return True
    End If '/
    Print pSymInfo->Name & "->" & "0x" & WHex(pSymInfo->ADDRESS)
    Return True
End Function

' 加载内核模块符号
Function QuerySymbol(ByVal Addr As ULONG64 Ptr, pName As LPWSTR) As Boolean
    ' 常见的内核模块名称
    Dim kernelModules(0) As WString * 50 = {"drivers\ntfs.sys"} ' {"drivers\ntfs.sys"} ' {"ntoskrnl.exe"}
    
    Dim i As Integer
    For i = 0 To UBound(kernelModules)
        Dim ModuleBase As ULONG64, ModuleSize As DWORD
        ModuleBase = Cast(ULONG64, GetKernelModuleBase(@kernelModules(i), ModuleSize))
        Print "ModuleBase:" & WHex(ModuleBase)
        Print "ModulePath:" & "C:\Windows\System32\" & kernelModules(i)
        Dim hFile As HANDLE = CreateFile("C:\Windows\System32\" & kernelModules(i), GENERIC_READ, FILE_SHARE_READ Or FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL)
        If hFile = NULL Then Print "[QuerySymbol]CreateFile:" & "C:\Windows\System32\" & kernelModules(i) & " " & WinErrorMsg(GetLastError) & GetLastError
        Dim dwFileSize As DWORD = GetFileSize(hFile, NULL)
        CloseHandle hFile
        'Print kernelModules(i) & ":0x" & WHex(ModuleBase)
        'Print kernelModules(i) & " 0x" & WHex(ModuleBase) & " " & "0x" & WHex(ModuleSize)
        'Dim SymLoadModule64(byval hProcess as HANDLE, byval hFile as HANDLE, byval ImageName as PSTR, byval ModuleName as PSTR, byval BaseOfDll as DWORD64, byval SizeOfDll as DWORD) as DWORD64 = DyLibSymbol(GetModuleHandle("DbgHelp.dll"), "SymLoadModuleExW")
        Dim dwBase As ULONG64 = SymLoadModule64(GetCurrentProcess, NULL, "C:\Windows\System32\" & kernelModules(i), NULL, ModuleBase, ModuleSize)
        Print "dwBase:" & WHex(dwBase)
        Print  WinErrorMsg(GetLastError) & GetLastError
        Dim SymInfo As SYMBOL_CONTEXT
        SymInfo.FuncAddr = Addr
        SymInfo.pFuncName = pName
        Print SymEnumSymbols(GetCurrentProcess, dwBase, NULL, @EnumSymCallBack, @SymInfo) & " " & WinErrorMsg(GetLastError) & GetLastError
        SymUnloadModule64 GetCurrentProcess, dwBase
        If (*(SymInfo.pFuncName) <> "") AndAlso (*Addr <> NULL) Then Return True
    Next
    Print "无法加载任何内核模块符号:" & WinErrorMsg(GetLastError) & GetLastError
    Return False
End Function

Function SymInit(ByVal hProcess As HANDLE) As Boolean
    Dim symbolPath As WString * 50
    
    '' 直接使用在线符号服务器，不进行本地缓存
    symbolPath = "SRV*" & App.Path & "symbols*" & "https://msdl.microsoft.com/download/symbols" ' "http://msdl.blackint3.com:88/download/symbols"
    
    SetEnvironmentVariable("_NT_SYMBOL_PATH", @symbolPath)
    Print "使用在线符号服务器（无缓存）"
    
    Dim options As DWORD = SymSetOptions(SYMOPT_DEFERRED_LOADS Or SYMOPT_UNDNAME Or SYMOPT_LOAD_LINES)
    
    ' 初始化符号处理
    If SymInitialize(hProcess, NULL, False) = False Then
        Print "SymInitialize 失败: " & GetLastError
        Return False
    End If
    Return True
End Function

Sub CopyDataToClipboard(ByVal strData As String)
    Dim hMemory As HGLOBAL
    Dim pMemory As LPVOID
    
    ' 1. 打开剪贴板
    If OpenClipboard(GetForegroundWindow) Then
        ' 2. 清空剪贴板
        EmptyClipboard
        ' 3. 分配全局内存 (长度需加1用于存储空终止符)
        hMemory = GlobalAlloc(GMEM_MOVEABLE Or GMEM_ZEROINIT, Len(strData) + 1)
        If hMemory Then
            ' 4. 锁定内存，获取指针
            pMemory = GlobalLock(hMemory)
            If pMemory Then
                ' 5. 将文本数据复制到内存块
                *CPtr(LPSTR, pMemory) = strData
                ' 6. 解锁内存
                GlobalUnlock hMemory
                ' 7. 将数据设置到剪贴板，系统将管理 hMemory
                SetClipboardData CF_TEXT, hMemory
            Else
                Print "[CopyDataToClipboard]GlobalLock:" & WinErrorMsg(GetLastError) & GetLastError
            End If
        Else
            Print "[CopyDataToClipboard]GlobalAlloc:" & WinErrorMsg(GetLastError) & GetLastError
        End If
        ' 8. 关闭剪贴板
        CloseClipboard
    Else
        Print "[CopyDataToClipboard]OpenClipboard:" & WinErrorMsg(GetLastError) & GetLastError
    End If
End Sub