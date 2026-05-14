#include Once "win/winbase.bi"

Declare Function RtlGetVersion Lib "ntdll.dll" Alias "RtlGetVersion"(ByVal pVersion As PRTL_OSVERSIONINFOEXW) As NTSTATUS

Declare Function SHCreateDataObject Lib "shell32.dll" Alias "SHCreateDataObject" ( _
    ByVal pidlFolder As Any Ptr, _
    ByVal cidl As UInteger, _
    ByVal apidl As Any Ptr, _
    ByVal pdtInner As Any Ptr, _
    ByVal riid As GUID Ptr, _
    ByRef ppv As Any Ptr _
    ) As Long

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
    
    Return "NtQuerySystemTime"
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

' 复制文件到剪贴板（等同于资源管理器复制文件）
Sub CopyFileToClipboard(sFilePath As StringW)
#define CF_PREFERREDDROPHEFFECT &H0105
    Dim hDrop As HGLOBAL
    Dim pDrop As DROPFILES Ptr
    Dim pData As LPVOID
    Dim bufSize As DWORD
    
    ' 打开剪贴板
    If OpenClipboard(GetForegroundWindow) = 0 Then
        Print "[CopyFileToClipboard]OpenClipboard 失败"
        Exit Sub
    End If
    EmptyClipboard

    ' ===================== 修复1：使用 UNICODE 宽字符（系统原生格式） =====================
    ' 路径必须以 双宽字符结束符 结尾
    Dim cbData As DWORD = LenW(sFilePath) * 2 + 2
    bufSize = SizeOf(DROPFILES) + cbData

    ' 分配内存
    hDrop = GlobalAlloc(GMEM_MOVEABLE Or GMEM_ZEROINIT, bufSize)
    If hDrop = NULL Then
        CloseClipboard
        Print "[CopyFileToClipboard]GlobalAlloc 失败"
        Exit Sub
    End If

    pDrop = GlobalLock(hDrop)
    If pDrop = NULL Then
        GlobalFree hDrop
        CloseClipboard
        Print "[CopyFileToClipboard]GlobalLock 失败"
        Exit Sub
    End If

    ' 填充 DROPFILES（UNICODE 模式）
    pDrop->pFiles = SizeOf(DROPFILES)
    pDrop->fWide = 1  ' 关键：切换为 Unicode，兼容现代系统

    ' 复制宽字符路径
    pData = Cast(LPVOID, pDrop) + SizeOf(DROPFILES)
    memcpy pData, StrPtrW(sFilePath), LenW(sFilePath) * 2

    GlobalUnlock hDrop
    SetClipboardData 15, hDrop  ' CF_HDROP = 15

    ' ===================== 修复2：添加系统必需的「复制操作标记」（核心！） =====================
    ' CF_PREFERREDDROPHEFFECT = 0x0105，DROPEFFECT_COPY = 1
    /'Dim hEffect As HGLOBAL = GlobalAlloc(GMEM_MOVEABLE Or GMEM_ZEROINIT, 4)
    If hEffect Then
        Dim pEffect As Long Ptr = GlobalLock(hEffect)
        *pEffect = 1 ' 声明：这是【复制操作】
        GlobalUnlock hEffect
        SetClipboardData CF_PREFERREDDROPHEFFECT, hEffect ' 写入拖放效果格式
    End If'/
    ' ========================================================================================

    CloseClipboard
End Sub

Sub CopyFileToClipboardFull(sFilePath As StringW)
    Dim pidlFile As Any Ptr
    Dim pidlFolder As Any Ptr
    Dim apidl(0) As Any Ptr
    Dim pDataObject As LPDATAOBJECT
    Dim IID_IDataObject As GUID = Type <GUID>(&H0000010E, &H0000, &H0000, {&HC0, &H00, &H00, &H00, &H00, &H00, &H00, &H46})
    Dim parentPath As StringW
    Dim hr As Long

    ' 初始化 COM
    hr = OleInitialize(NULL)
    If hr <> 0 And hr <> &H1 Then ' S_OK 或 S_FALSE
        Print "CoInitialize FAILED: "; hr
        Exit Sub
    End If

    ' 获取文件完整 PIDL
    pidlFile = ILCreateFromPathW(StrPtrW(sFilePath))
    If pidlFile = NULL Then
        Print "ILCreateFromPathW FAILED, "; GetLastError()
        CoUninitialize
        Exit Sub
    End If

    ' 获取父目录路径
    parentPath = LeftW(sFilePath, InStrRevW(sFilePath, "\") - 1)
    pidlFolder = ILCreateFromPathW(StrPtrW(parentPath))
    If pidlFolder = NULL Then
        Print "ILCreateFromPathW parent FAILED, "; GetLastError()
        ILFree pidlFile
        CoUninitialize
        Exit Sub
    End If

    ' 获取子 PIDL（文件名相对父目录）
    apidl(0) = ILFindLastID(pidlFile)
    If apidl(0) = NULL Then
        Print "ILFindLastID FAILED"
        ILFree pidlFile
        ILFree pidlFolder
        CoUninitialize
        Exit Sub
    End If

    ' 创建 IDataObject
    hr = SHCreateDataObject(pidlFolder, 1, Varptr(apidl(0)), NULL, @IID_IDataObject, pDataObject)
    If hr <> 0 Or pDataObject = NULL Then
        Print "SHCreateDataObject FAILED, "; hr
        ILFree pidlFile
        ILFree pidlFolder
        CoUninitialize
        Exit Sub
    End If

    ' 设置到剪贴板
    hr = OleSetClipboard(pDataObject)
    If hr <> 0 Then
        Print "OleSetClipboard FAILED, "; hr
    End If

    ' 释放 PIDL
    ILFree pidlFile
    ILFree pidlFolder

    ' 释放 IDataObject（剪贴板拥有引用）
    If pDataObject <> NULL Then
        'ReleaseCOMObject(pDataObject)
    End If

    ' 解除 COM
    OleUninitialize
End Sub