#include "win/winerror.bi"
#include "win/shlobj.bi"

Type WINTRUST_CATALOG_INFO_EX Field = 8
    cbStruct As DWORD
    dwCatalogVersion As DWORD
    pcwszCatalogFilePath As LPCWSTR
    pcwszMemberTag As LPCWSTR
    pcwszMemberFilePath As LPCWSTR
    hMemberFile As HANDLE
    pbCalculatedFileHash As UByte Ptr
    cbCalculatedFileHash As DWORD
    pcCatalogContext As Any Ptr
End Type

/'Type WINTRUST_FILE_INFO
    cbStruct As DWORD
    pcwszFilePath As LPCWSTR
    HFILE As HANDLE
    pgKnownSubject As GUID Ptr
End Type'/

/'Type WINTRUST_CATALOG_INFO
    cbStruct As DWORD
    dwCatalogVersion As DWORD
    pcwszCatalogFilePath As WString Ptr
    pcwszMemberTag As Wstring Ptr
    pcwszMemberFilePath As WString Ptr
    hMemberFile As HANDLE
    pbCalculatedFileHash As Byte Ptr
    cbCalculatedFileHash As DWORD
    pcCatalogContext As Any Ptr
End Type'/

'Type WINTRUST_DATA
'    cbStruct As DWORD
'    pPolicyCallbackData As LPVOID
'    pSIPClientData As LPVOID
'    dwUIChoice as DWORD
'    fdwRevocationChecks As DWORD
'    dwUnionChoice As DWORD
'    Union
'        pFile As WINTRUST_FILE_INFO Ptr
'        pCatalog As WINTRUST_CATALOG_INFO Ptr
'        'pBlob As WINTRUST_BLOB_INFO_ Ptr
'        'pSgnr As WINTRUST_SGNR_INFO_ Ptr
'        'pCert As WINTRUST_CERT_INFO_ Ptr
'    End Union
'    dwStateAction As DWORD
'    hWVTStateData As HANDLE
'    pwszURLReference As Wstring Ptr
'    dwProvFlags As DWORD
'    dwUIContext As DWORD
'End Type

Type CATALOG_INFO
    cbStruct As DWORD
    wszCatalogFile As Wstring * 260 'MAX_PATH
End Type

Declare Function ZwDeleteFile Lib "ntdll.dll" Alias "ZwDeleteFile"(ByVal ObjectAttributes As OBJECT_ATTRIBUTES Ptr) As NTSTATUS
'Declare Function WinVerifyTrust Lib "wintrust.dll" Alias "WinVerifyTrust"(ByVal hWnd As hWnd, ByVal pgActionID As GUID Ptr, ByVal pWVTData As LPVOID) As Long

Declare Function CryptCATAdminAcquireContext Lib "wintrust" _
    Alias "CryptCATAdminAcquireContext" _
    (ByVal hCatAdmin As Any Ptr, ByVal pgSubsystem As Byte Ptr, ByVal dwFlags As DWORD) As BOOL

Declare Function CryptCATAdminReleaseContext Lib "wintrust" _
    Alias "CryptCATAdminReleaseContext" _
    (ByVal hCatAdmin As Any Ptr, ByVal dwFlags As DWORD) As BOOL

Declare Function CryptCATAdminCalcHashFromFileHandle Lib "wintrust" _
    Alias "CryptCATAdminCalcHashFromFileHandle" _
    (ByVal hFile As HANDLE, ByVal pcbHash As DWORD Ptr, ByVal pbHash As Byte Ptr, ByVal dwFlags As DWORD) As BOOL

Declare Function CryptCATAdminEnumCatalogFromHash Lib "wintrust" _
    Alias "CryptCATAdminEnumCatalogFromHash" _
    (ByVal hCatAdmin As Any Ptr, ByVal pbHash As Byte Ptr, ByVal cbHash As DWORD, ByVal dwFlags As DWORD, _
    ByVal phPrevCatInfo As Any Ptr Ptr) As Any Ptr

Declare Function CryptCATAdminReleaseCatalogContext Lib "wintrust" _
    Alias "CryptCATAdminReleaseCatalogContext" _
    (ByVal hCatAdmin As Any Ptr, ByVal hCatInfo As Any Ptr, ByVal dwFlags As DWORD) As BOOL

Declare Function CryptCATCatalogInfoFromContext Lib "wintrust" _
    Alias "CryptCATCatalogInfoFromContext" _
    (ByVal hCatInfo As Any Ptr, ByVal psCatInfo As Any Ptr, ByVal dwFlags As DWORD) As BOOL

Const OBJ_CASE_INSENSITIVE = &H00000040

Const WTD_REVOKE_NONE = &H0
Const WTD_CHOICE_CATALOG = &H2
'Const WTD_STATEACTION_VERIFY = &H0000
Const WTD_STATEACTION_CLOSE = &H0002
Const WTD_UI_NONE = 2
Const WTD_CHOICE_FILE = 1
Const WTD_USE_DEFAULT_OSVER_CHECK = &H00000400
CONST WTD_REVOCATION_CHECK_NONE = &H00000010
CONST WTD_STATEACTION_IGNORE = &H00000000
CONST WTD_SAFER_FLAG = &H00000100


Public Sub GetDriveList(treFolder As Class_TreeView)
    treFolder.DeleteAllItems
    Dim i As Integer,CurrentNode As HTREEITEM,DiskName As StringW
    For i = Asc("A") To Asc("Z")
        If (GetDriveType(Chr(i) & ":\")) <> 1 Then
            DiskName = GetDiskLabel(Chr(i) & ":")
            If (DiskName = "") Then
                treFolder.AddItem NULL, "本地磁盘" & "(" & Chr(i) & ":" & ")"
            Else
                treFolder.AddItem NULL,DiskName & "(" & Chr(i) & ":" & ")"
            End If
        End If
    Next
End Sub

Private Function GetNode(ByVal szPath As LPWSTR,treFolder As Class_TreeView) As HTREEITEM
    Dim szFolder() As String

    Split Str(*szPath),"\",szFolder()
    Dim i As Integer,j As Integer,CurrentNode As HTREEITEM
    For i = 0 To UBound(szFolder)
        CurrentNode = treFolder.GetRoot
        'Do
            'If treFolder.Text(CurrentNode) = szFolder(i) Then
        'Loop While
    Next
    Return NULL
End Function

Public Sub GetFilePathByNodeW(Node As HTREEITEM, treFolder As Class_TreeView, ByRef FilePath As StringW)
    Dim CurrentNode As HTREEITEM = Node, PathLen As Integer = 256
    Dim i As Integer
    Do While True
        FilePath = treFolder.Text(CurrentNode) & "\" & FilePath
        If (treFolder.GetParent(CurrentNode) = NULL) Then Exit Do
        CurrentNode = treFolder.GetParent(CurrentNode)
    Loop
    Dim pos1 As Integer, pos2 As Integer
    pos1 = InStrRevW(FilePath, "(")
    pos2 = InStrRevW(FilePath, ")")
    FilePath = MidW(FilePath, pos1 + 1, pos2 - pos1 - 1) & RightW(FilePath, Len(FilePath) - pos2)
End Sub

'Private Function PrintFileSize(ByVal FileSize As QWORD) As LPWSTR
'    Dim rSize As Double,printSize As LPWSTR
'    rSize = round(FileSize/1024)
'    If rSize = 0 Then
'        *printSize = WStr(FileSize) & "B"
'    Else
'        rSize = round(rSize)
'        If rSize = 0 Then
'            '*printSize = WStr(r)
'        End If
'    End If
'End Function
'Replace
Public Function ReplaceW(ByVal MainString As LPWSTR,ByVal SearchString As LPWSTR,ByVal ReplaceString As LPWSTR) As LPWSTR
    Dim MidString As LPWSTR,i As Integer,SearchLen As Integer,MainLen As Integer,RetString As LPWSTR
    MainLen = Len(*MainString)
    SearchLen = Len(*SearchString)
    For i = 1 To MainLen
        *MidString = Mid(*MainString,i,SearchLen)
        If (MidString=SearchString) Then
            *RetString = Left(*MainString,i - 1) & *ReplaceString & Right(*MainString,MainLen - (i + SearchLen - 1))
        End If
    Next
    Return RetString
End Function

Public Sub GetFileList(CurrentNode As HTREEITEM, treFolder As Class_TreeView, lvwFile As Class_ListView, ByVal IsEnumFolder As Boolean, IsEnumFile As Boolean = True)
    Dim szPath As StringW = ""
    
    If IsEnumFolder Then DeleteChildItem treFolder, CurrentNode
    lvwFile.DeleteAllItems
    
    GetFilePathByNodeW CurrentNode, treFolder, szPath
    szPath = szPath & "*.*"
    Dim fData As WIN32_FIND_DATAW
    Dim hFind As HANDLE = FindFirstFileW(szPath,@fData)'不能使用@szPath,否则报"找不到文件"
    If hFind = INVALID_HANDLE_VALUE Then
        Print WinErrorMsg(GetLastError)
        Exit Sub
    End If
    Do
        If (fData.cFileName = "." OrElse fData.cFileName = "..") Then Continue Do
        If fData.dwFileAttributes And fbDirectory Then
            If IsEnumFolder Then treFolder.InsertItem CurrentNode,TVI_LAST,fData.cFileName
        ElseIf IsEnumFile Then
            lvwFile.AddItem fData.cFileName
            lvwFile.SetItemText lvwFile.ItemCount - 1, 1, WStr(fData.nFileSizeHigh * (2 ^ 32) + fData.nFileSizeLow)
        End If
    Loop While FindNextFileW(hFind,@fData) <> 0
    FindClose hFind
End Sub

Public Function GetNodeByFilePathW(sPath As StringW, treFolder As Class_TreeView, lvwFile As Class_ListView) As HTREEITEM
    ' 规范化路径：替换斜杠、去除末尾分隔符
    Dim sNormalizedPath As StringW = sPath
    If RightW(sNormalizedPath, 1) = "\" Then
        sNormalizedPath = LeftW(sNormalizedPath, LenW(sNormalizedPath) - 1)
    End If
    
    ' 将路径分割成部分
    Dim sParts() As StringW
    Dim iNumParts As Integer
    iNumParts = vbSplitW(sNormalizedPath, "\", sParts())
    
    ' 如果路径无效，直接返回 NULL
    If iNumParts < 1 Then Return NULL
    
    ' 第一步：找到匹配的根节点（驱动器）
    Dim hCurrentNode As HTREEITEM = treFolder.GetRoot()
    Dim hRootNode As HTREEITEM = NULL
    Dim sNodeText As StringW
    Dim pos1 As Integer, pos2 As Integer
    
    ' 遍历所有根节点
    While hCurrentNode <> NULL
        sNodeText = treFolder.Text(hCurrentNode)
        
        ' 从根节点文本中提取驱动器号（如从"本地磁盘 (C:)"中提取"C:"）
        pos1 = InStrRevW(sNodeText, "(")
        pos2 = InStrRevW(sNodeText, ")")
        If pos1 > 0 And pos2 > pos1 Then
            Dim sDriveLetter As StringW = MidW(sNodeText, pos1 + 1, pos2 - pos1 - 1)
            
            ' 比较提取的驱动器号与路径的第一部分
            If UCase(sDriveLetter) = UCase(sParts(0)) Then
                hRootNode = hCurrentNode
                'Print sDriveLetter
                If iNumParts > 1 Then
                    GetFileList hCurrentNode, treFolder, lvwFile, True, False
                    treFolder.ExpandEx hCurrentNode, TVE_EXPAND
                    'hCurrentNode = treFolder.GetChild(hCurrentNode)
                Else
                    Return hCurrentNode
                End If
                Exit While
            End If
        End If
        
        hCurrentNode = treFolder.GetNextSiblin(hCurrentNode)
    Wend
    
    If hRootNode = NULL Then Return NULL
    
    ' 第二步：从根节点开始，逐级匹配路径的剩余部分
    hCurrentNode = hRootNode
    Dim i As Integer
    Dim hChildNode As HTREEITEM
    Dim bFound As Boolean
    
    ' 从路径的第二部分开始遍历（第一部分是驱动器号，已经匹配过了）
    For i = 1 To iNumParts - 1
        'Print "sParts(" & i & "):" & sParts(i)
        If sParts(i) = "" Then Continue For
        
        bFound = False
        
        ' 获取当前节点的第一个子节点
        hChildNode = treFolder.GetChild(hCurrentNode)
        
        ' 如果当前节点没有子节点，且不是最后一级，则尝试展开
        /'If hChildNode = NULL And i < iNumParts - 1 Then
            GetFileList hCurrentNode, treFolder, lvwFile, True, False
            treFolder.ExpandEx hCurrentNode, TVE_EXPAND
            hChildNode = treFolder.GetChild(hCurrentNode)
        End If'/
        
        ' 遍历所有子节点
        While hChildNode <> NULL
            sNodeText = treFolder.Text(hChildNode)
            
            'Print "Comparing: " & sNodeText & " with " & sParts(i)
            
            ' 比较节点文本与当前路径部分（子目录节点通常没有卷标，直接比较）
            If UCase(sNodeText) = UCase(sParts(i)) Then
                bFound = True
                hCurrentNode = hChildNode
                
                ' 如果不是最后一部分，确保节点已展开
                If i < iNumParts - 1 Then
                    ' 如果节点未展开，则展开
                    If Not treFolder.Expand(hCurrentNode) Then
                        GetFileList hCurrentNode, treFolder, lvwFile, True, False
                        treFolder.ExpandEx hCurrentNode, TVE_EXPAND
                        Exit While
                    End If
                Else
                    Return hCurrentNode
                End If
                
                Exit While
            End If
            
            hChildNode = treFolder.GetNextSiblin(hChildNode)
        Wend
        
        If Not bFound Then
            Print "Not found: " & sParts(i)
            Return NULL
        End If
    Next
    
    Return hCurrentNode
End Function

Function MyDeleteFile(ByVal lpFilePath As LPCWSTR) As NTSTATUS
    Dim ObjectAttributes As OBJECT_ATTRIBUTES, unicodeFilePath As UNICODE_STRING, st As NTSTATUS
    RtlInitUnicodeString @unicodeFilePath, lpFilePath
    InitializeObjectAttributes @ObjectAttributes, @unicodeFilePath, OBJ_CASE_INSENSITIVE
    Return ZwDeleteFile(@ObjectAttributes)
End Function

Function LocateFilePathByExplorer(ByVal lpFilePath As LPCWSTR) As Boolean
    ' 打开文件资源管理器并选择文件
    'CoInitializeEx NULL, COINIT_MULTITHREADED
    Dim As ITEMIDLIST Ptr pidl = ILCreateFromPathW(lpFilePath)
    If pidl <> NULL Then
        SHOpenFolderAndSelectItems pidl, 0, NULL, 0
        ILFree pidl
        Return True
    Else
        Print "无法创建文件路径的 ITEMIDLIST。:" & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If 
End Function

' 辅助函数：解析错误码为文字描述
Function GetVerifyErrorString(ByVal errorCode As Long) As String
    #define TRUST_E_NOT_TIME_VALID &H800B0101
    Select Case errorCode
        Case ERROR_SUCCESS: Return "签名有效（无任何问题）"
        Case TRUST_E_NOSIGNATURE: Return "无数字签名"
        Case TRUST_E_BAD_DIGEST: Return "文件被篡改（哈希不匹配）"
        Case TRUST_E_CERT_SIGNATURE: Return "证书签名无效"
        Case TRUST_E_EXPLICIT_DISTRUST: Return "发布者被明确标记为不可信"
        Case TRUST_E_TIME_STAMP: Return "时间戳验证失败"
        Case TRUST_E_NOT_TIME_VALID: Return "证书不在有效期内"
        Case Else: Return "未知错误（代码：" & errorCode & "）"
    End Select
End Function

Function VerifyFileByCatalog(ByVal pwszFilePath As LPCWSTR) As BOOL
    Dim hCatAdmin As Any Ptr = NULL
    Dim hCatInfo As Any Ptr = NULL
    Dim hFile As HANDLE = INVALID_HANDLE_VALUE
    Dim fVerified As BOOL = FALSE
    Dim wvtGUID As GUID = Type < GUID > (&H00AAC56B, &HCD44, &H11D0, {&H8C, &HC2, &H00, &HC0, &H4F, &HC2, &H95, &HEE}) 
    
    Dim rgbHash(0 To 99) As Byte ' 预留100字节的哈希缓冲区
    Dim cbHash As DWORD = SizeOf(Byte) * 100
    
    Dim catalogInfo As CATALOG_INFO
    Dim pszMemberTag As WString Ptr = NULL
    
    ' 1. 获取目录管理员上下文
    If CryptCATAdminAcquireContext(@hCatAdmin, NULL, 0) = False Then
        Print "CryptCATAdminAcquireContext failed: " & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    ' 2. 打开文件并计算哈希
    hFile = CreateFileW(pwszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        Print "CreateFile failed: " & WinErrorMsg(GetLastError) & GetLastError
        Goto cleanup
    End If
    
    If CryptCATAdminCalcHashFromFileHandle(hFile, @cbHash, @rgbHash(0), 0) = False Then
        Print "CryptCATAdminCalcHashFromFileHandle failed: " & WinErrorMsg(GetLastError) & GetLastError
        Goto cleanup
    End If
    
    pszMemberTag = malloc(cbHash * 2 + 1)
    For dw As DWORD = 0 To cbHash - 1
        wsprintfW(@pszMemberTag[dw * 2], StrPtrW("%02X"), rgbHash(dw))
    Next
    
    CloseHandle(hFile)
    hFile = INVALID_HANDLE_VALUE
    
    ' 3. 根据哈希查找目录
    hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, @rgbHash(0), cbHash, 0, NULL)
    If hCatInfo = NULL Then
        Print "CryptCATAdminEnumCatalogFromHash failed. No catalog found for the file." & WinErrorMsg(GetLastError) & GetLastError
        Goto cleanup
    End If
    
    ' 4. 获取目录文件信息
    'memset @catalogInfo, 0, SizeOf(CATALOG_INFO)
    catalogInfo.cbStruct = SizeOf(catalogInfo)
    /'Print "WINTRUST_CATALOG_INFO size: " & SizeOf(WINTRUST_CATALOG_INFO)
    Print "CATALOG_INFO size: " & SizeOf(CATALOG_INFO)
    Print "cbHash: " & cbHash'/
    If CryptCATCatalogInfoFromContext(hCatInfo, @catalogInfo, 0) Then
        Print catalogInfo.wszCatalogFile
        ' 准备WINTRUST_CATALOG_INFO结构
        Dim wti As WINTRUST_CATALOG_INFO
        'memset @wti, 0, SizeOf(WINTRUST_CATALOG_INFO)
        wti.cbStruct = SizeOf(wti)
        'wti.dwCatalogVersion = 0 ' 使用默认值
        
        wti.pcwszMemberTag = pszMemberTag
        wti.pcwszMemberFilePath = pwszFilePath
        wti.pcwszCatalogFilePath = @catalogInfo.wszCatalogFile
        wti.pbCalculatedFileHash = @rgbHash(0)
        wti.cbCalculatedFileHash = cbHash
        'wti.hMemberFile = 0 ' INVALID_HANDLE_VALUE
        
        ' 准备WINTRUST_DATA结构
        Dim wtd As WINTRUST_DATA
        'memset @wtd, 0, SizeOf(WINTRUST_DATA)
        wtd.cbStruct = SizeOf(wtd)
        wtd.dwUIChoice = WTD_UI_NONE
        wtd.fdwRevocationChecks = WTD_REVOKE_NONE
        wtd.dwUnionChoice = WTD_CHOICE_CATALOG
        wtd.pCatalog = @wti
        wtd.dwStateAction = WTD_STATEACTION_IGNORE ' WTD_STATEACTION_VERIFY
        'wtd.dwProvFlags = WTD_REVOCATION_CHECK_NONE Or WTD_USE_DEFAULT_OSVER_CHECK
        wtd.dwProvFlags = WTD_REVOCATION_CHECK_NONE Or WTD_USE_DEFAULT_OSVER_CHECK
        
        ' 5. 调用WinVerifyTrust验证目录签名
        Dim Status As Long = WinVerifyTrust(NULL, @wvtGUID, @wtd)
        fVerified = (Status = ERROR_SUCCESS)
        Print wtd.fdwRevocationChecks & " " & wtd.dwStateAction & " " & wtd.dwProvFlags
        If Status = ERROR_SUCCESS Then Print "有效!" Else Print "WinVerifyTrust失败:" & Hex(Status)
        
        ' 关闭验证状态
        wtd.dwStateAction = WTD_STATEACTION_CLOSE
        WinVerifyTrust(NULL, @wvtGUID, @wtd)
    Else
        Print "CryptCATCatalogInfoFromContext failed: " & WinErrorMsg(GetLastError) & GetLastError
    End If
    
cleanup:
    ' 6. 清理资源
    If hCatInfo Then
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0)
    End If
    If hCatAdmin Then
        CryptCATAdminReleaseContext(hCatAdmin, 0)
    End If
    If hFile <> INVALID_HANDLE_VALUE Then
        CloseHandle(hFile)
    End If
    If pszMemberTag <> NULL Then
        free pszMemberTag
    End If
    
    Return fVerified
End Function

Function CheckFileTrust(ByVal lpFileName As LPCWSTR) As Integer
    ' 初始化返回值（默认失败）
    Dim bRet As Integer = FALSE
    
    ' 初始化所有结构体和句柄
    Dim wd As WINTRUST_DATA = Any
    Dim wfi As WINTRUST_FILE_INFO = Any
    Dim wci As WINTRUST_CATALOG_INFO = Any
    Dim ci As CATALOG_INFO = Any
    
    Dim hCatAdmin As Any Ptr = NULL
    Dim hFile As Any Ptr = NULL
    Dim hCatInfo As Any Ptr = NULL
    Dim pszMemberTag As WString Ptr = NULL ' 哈希的十六进制字符串（动态分配）
    
    Dim wvtGUID As GUID = Type < GUID > (&H00AAC56B, &HCD44, &H11D0, {&H8C, &HC2, &H00, &HC0, &H4F, &HC2, &H95, &HEE})
    
    Dim dwCnt As ULong = 100
    Dim byHash(0 To 99) As Byte ' 哈希缓冲区（100字节）
    
    Dim hr As Long
    
    ' ==========================================================================
    ' 步骤1：获取CAT目录管理员上下文
    ' ==========================================================================
    If CryptCATAdminAcquireContext(@hCatAdmin, NULL, 0) = 0 Then
        Print "CryptCATAdminAcquireContext failed:" & WinErrorMsg(GetLastError) & GetLastError
        Goto Cleanup ' 获取失败，直接清理资源
    End If
    
    ' ==========================================================================
    ' 步骤2：打开文件（只读模式，共享读）
    ' ==========================================================================
    hFile = CreateFileW(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        Print "CreateFileW failed:" & WinErrorMsg(GetLastError) & GetLastError
        Goto Cleanup ' 打开失败，清理资源
    End If
    
    ' ==========================================================================
    ' 步骤3：计算文件哈希（缓冲区大小100字节，与原C++一致）
    ' ==========================================================================
    ' 调用API计算哈希（忽略返回值，原C++未判断此处失败）
    CryptCATAdminCalcHashFromFileHandle(hFile, @dwCnt, @byHash(0), 0)
    CloseHandle(hFile) ' 计算完成后关闭文件句柄
    hFile = INVALID_HANDLE_VALUE ' 标记句柄已关闭
    
    ' ==========================================================================
    ' 步骤4：将哈希字节数组转换为十六进制宽字符串（pszMemberTag）
    ' ==========================================================================
    If dwCnt > 0 Then
        ' 分配字符串内存：每个字节占2个字符 + 1个终止符（\0）
        pszMemberTag = malloc((dwCnt * 2 + 1) * SizeOf(Wstring * 1))
        If pszMemberTag = NULL Then
            Print "Allocate failed"
            Goto Cleanup ' 内存分配失败，清理资源
        End If
        
        ' 逐个字节转换为十六进制（%02X格式）
        For dw As ULong = 0 To dwCnt - 1
            wsprintfW(@pszMemberTag[dw * 2], !"\%02X", byHash(dw))
        Next
        pszMemberTag[dwCnt * 2] = 0 ' 手动添加字符串终止符
        Print *pszMemberTag
    End If
    
    ' ==========================================================================
    ' 步骤5：根据哈希查找CAT目录
    ' ==========================================================================
    hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, @byHash(0), dwCnt, 0, NULL)
    
    ' ==========================================================================
    ' 步骤6：初始化验证参数（分两种情况：有CAT目录 / 无CAT目录）
    ' ==========================================================================
    If hCatInfo = NULL Then
        ' 情况1：无CAT目录 → 直接验证文件（WTD_CHOICE_FILE）
        With wfi
            .cbStruct = SizeOf(WINTRUST_FILE_INFO) ' 必须初始化结构体大小
            .pcwszFilePath = lpFileName            ' 待验证文件路径
            .hFile = NULL                          ' 不使用文件句柄
            .pgKnownSubject = NULL                 ' 无已知主题
        End With
        
        With wd
            .cbStruct = SizeOf(WINTRUST_DATA)      ' 必须初始化结构体大小
            .dwUnionChoice = WTD_CHOICE_FILE       ' 验证类型：文件
            .pFile = @wfi                          ' 关联文件验证结构体
            .dwUIChoice = WTD_UI_NONE              ' 不显示UI
            .fdwRevocationChecks = WTD_REVOKE_NONE ' 不检查吊销
            .dwStateAction = WTD_STATEACTION_VERIFY
            .dwProvFlags = WTD_SAFER_FLAG          ' 启用SAFER模式
            .hWVTStateData = NULL                  ' 无状态数据
            .pwszURLReference = NULL               ' 无URL参考
        End With
    Else
        ' 情况2：有CAT目录 → 通过CAT验证（WTD_CHOICE_CATALOG）
        ' 先获取CAT目录信息
        ci.cbStruct = SizeOf(CATALOG_INFO) ' 必须初始化结构体大小
        'Dim wszCatalogPath As WString * MAX_PATH ' 栈上分配260个宽字符的缓冲区
        'ci.wszCatalogFile = Wstring(MAX_PATH, 0)      ' 将缓冲区地址赋值给结构体指针
        If CryptCATCatalogInfoFromContext(hCatInfo, @ci, 0) Then
            Print ci.wszCatalogFile
            With wci
                .cbStruct = SizeOf(WINTRUST_CATALOG_INFO) ' 必须初始化结构体大小
                .pcwszCatalogFilePath = StrPtr(ci.wszCatalogFile) ' CAT文件路径
                .pcwszMemberFilePath = lpFileName         ' 待验证文件路径
                .pcwszMemberTag = pszMemberTag            ' 哈希十六进制字符串
                .hMemberFile = INVALID_HANDLE_VALUE                       ' 不使用文件句柄
            End With
            
            With wd
                .cbStruct = SizeOf(WINTRUST_DATA)       ' 必须初始化结构体大小
                .dwUnionChoice = WTD_CHOICE_CATALOG     ' 验证类型：目录
                .pCatalog = @wci                         ' 关联目录验证结构体
                .dwUIChoice = WTD_UI_NONE               ' 不显示UI
                .fdwRevocationChecks = WTD_REVOKE_NONE ' 原C++此处参数值（保持一致）
                .dwStateAction = WTD_STATEACTION_VERIFY
                .dwProvFlags = 0 ' WTD_REVOCATION_CHECK_NONE Or WTD_USE_DEFAULT_OSVER_CHECK
                .hWVTStateData = NULL                   ' 无状态数据
                .pwszURLReference = NULL                ' 无URL参考
            End With
        Else
            Print "CryptCATCatalogInfoFromContext failed: " & GetLastError
            Goto Cleanup '获取CAT信息失败，直接退出，避免后续崩溃
        End If
    End If
    ' ==========================================================================
    ' 步骤7：执行信任验证
    ' ==========================================================================
    For i As Integer = 0 To 1
        hr = WinVerifyTrust(NULL, @wvtGUID, @wd)
        If hr = S_OK Then Exit For
        Print lpFileName & " WinVerifyTrust failed:" & WinErrorMsg(GetLastError) & GetLastError
        bRet = (hr = S_OK) ' 验证成功的条件：HRESULT == S_OK
    Next

' ==========================================================================
' 步骤8：清理资源（避免内存泄漏，与原C++逻辑一致）
' ==========================================================================
Cleanup : 
    ' 释放CAT目录上下文
    If hCatInfo <> NULL Then
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0)
    End If
    ' 释放CAT管理员上下文
    If hCatAdmin <> NULL Then
        CryptCATAdminReleaseContext(hCatAdmin, 0)
    End If
    ' 关闭未关闭的文件句柄
    If hFile <> INVALID_HANDLE_VALUE Then
        CloseHandle(hFile)
    End If
    ' 释放哈希字符串内存（避免内存泄漏）
    If pszMemberTag <> NULL Then
        free pszMemberTag
    End If
    ' 返回验证结果
    Return bRet
End Function

' 改进后的签名验证函数：静默模式（不弹窗），并返回具体错误码
Function VerifyPeSignature(ByVal filePath As StringW, ByRef outError As Long) As Boolean
    Dim fileInfo As WINTRUST_FILE_INFO
    Dim wtd As WINTRUST_DATA
    Dim wvtGuid As GUID = Type<GUID>(&h00AAC56B, &hCD44, &h11d0, {&h8C, &hC2, &h00, &hC0, &h4F, &hC2, &h95, &hEE}) ' WINTRUST_ACTION_GENERIC_VERIFY_V2
    Dim result As Long
    
    ' 初始化文件信息
    With fileInfo
        .cbStruct = SizeOf(WINTRUST_FILE_INFO)
        .pcwszFilePath = filePath.WstrPtr
        .hFile = NULL
        .pgKnownSubject = NULL
    End With
    
    ' 初始化信任数据（关键：配置UI策略为静默模式）
    ZeroMemory(@wtd, SizeOf(WINTRUST_DATA))  ' 清空结构
    With wtd
        .cbStruct = SizeOf(WINTRUST_DATA)
        .pPolicyCallbackData = NULL
        .pSIPClientData = NULL
        .dwUIChoice = WTD_UI_NONE  ' 静默验证，不弹窗
        .fdwRevocationChecks = WTD_REVOKE_NONE  ' 不检查证书吊销（可选）
        .dwUnionChoice = WTD_CHOICE_FILE  ' 验证文件
        .pFile = @fileInfo  ' 关联文件信息
        .dwStateAction = 0
        .hWVTStateData = NULL
        .pwszURLReference = NULL
        .dwProvFlags = WTD_USE_DEFAULT_OSVER_CHECK' Or 
        .dwUIContext = 0
    End With
    
    ' 执行验证
    result = WinVerifyTrust(NULL, @wvtGuid, @wtd)
    outError = result ' 传出具体错误码
    ' 检查结果（仅ERROR_SUCCESS表示完全有效）
    Return (result = ERROR_SUCCESS)
End Function

'040904B0
Function GetFileCompany(FileName As StringW) As StringW
    Dim dwSize As DWORD
    Dim pBuffer As PVOID
    Dim dwDummy As DWORD
    Dim lpData As WString * MAX_PATH = ""
    Dim lpBuffer As PVOID
    Dim dwLen As DWORD
    Dim pTransTable As LANGANDCODEPAGE Ptr
    Dim nQuerySize As DWORD
    Dim langCharset As DWORD
    Dim subBlock As StringW
    Dim i As Integer  ' 声明循环变量

    ' 获取文件版本信息的大小
    dwSize = GetFileVersionInfoSizeW(FileName, @dwDummy)
    If dwSize = 0 Then
        Print "GetFileVersionInfoSizeW 失败, 错误代码: " & GetLastError
        Return ""
    End If

    ' 分配缓冲区
    pBuffer = Allocate(dwSize)
    If pBuffer = 0 Then
        Print "内存分配失败"
        Return ""
    End If

    If GetFileVersionInfoW(FileName, 0, dwSize, pBuffer) Then
        ' 获取翻译表（包含支持的语言和代码页）
        If VerQueryValueW(pBuffer, StrPtrW("\VarFileInfo\Translation"), @pTransTable, @nQuerySize) Then
            ' 遍历所有支持的语言/代码页组合（每个条目4字节）
            'For i = 0 To (nQuerySize \ 4) - 1
            ' 计算语言代码页组合（格式: wLanguage(2字节) + wCodePage(2字节)）
            langCharset = (pTransTable[0].wLanguage Shl 16) Or pTransTable[0].wCodePage
            ' 构建查询路径（使用当前语言代码页）
            subBlock = "\StringFileInfo\" & Hex(langCharset, 8) & "\CompanyName"
            
            ' 查询公司名称
            If VerQueryValueW(pBuffer, StrPtrW(subBlock), @lpBuffer, @dwLen) Then
                ' 确保获取到有效数据（宽字符串长度至少为1）
                If dwLen >= 2 Then
                    'lpData = Wstring(dwLen - 1, 0)  ' 预留字符串空间（减去终止符）
                    CopyMemory(StrPtr(lpData), lpBuffer, dwLen * 2)  ' 不复制终止符
                    Deallocate pBuffer
                    'Print FileName & " " & lpData
                    Return lpData
                End If
            End If
            'Next
            ' 遍历所有语言后仍未找到
            Print "未找到有效的CompanyName信息"
        Else
            Print "VerQueryValueW (Translation) 失败, 错误代码: " & GetLastError
        End If
    Else
        Print "GetFileVersionInfoW 失败, 错误代码: " & GetLastError
    End If

    ' 释放缓冲区
    Deallocate pBuffer
    Return ""
End Function

Function VerifyFileSign(ByVal pwszFilePath As LPCWSTR) As Bool
    Dim hCatAdmin As Any Ptr = NULL
    Dim hCatInfo As Any Ptr = NULL
    Dim hFile As HANDLE = INVALID_HANDLE_VALUE
    Dim fVerified As BOOL = FALSE
    Dim wvtGUID As GUID = Type < GUID > (&H00AAC56B, &HCD44, &H11D0, {&H8C, &HC2, &H00, &HC0, &H4F, &HC2, &H95, &HEE}) 
    
    Dim rgbHash(0 To 99) As Byte ' 预留100字节的哈希缓冲区
    Dim cbHash As DWORD = SizeOf(Byte) * 100
    
    Dim catalogInfo As CATALOG_INFO
    
    Dim pszMemberTag As WString Ptr = NULL
    
    Dim RevocationChecks As Long = WTD_REVOKE_NONE ' , WTD_REVOKE_WHOLECHAIN}
    'Dim StateAction(3) As Long = {WTD_STATEACTION_IGNORE, WTD_STATEACTION_VERIFY, WTD_STATEACTION_AUTO_CACHE, WTD_STATEACTION_AUTO_CACHE_FLUSH}
    Dim StateAction As Long = WTD_STATEACTION_IGNORE
    'Dim ProvFlags(9) As Long = {16, 32, 64, 128, 512, 1024, 2048, 4096, 8192}
    Dim ProvFlags(1) As Long = {16 + 1024, 16 + 1024} /'似乎存在某种预热机制,
    '每次改变dwRevocationChecks,StateAction或被验证文件路径都需要重新"预热","预热"时的参数可以与正式验证时的参数一致'/
    'Dim ProvFlags As Long = 16 + 1024
    Dim count As Integer = 0
    Dim buffer As String
    
    ' 1. 获取目录管理员上下文
    If CryptCATAdminAcquireContext(@hCatAdmin, NULL, 0) = False Then
        Print "CryptCATAdminAcquireContext failed: " & WinErrorMsg(GetLastError) & GetLastError
        Return False
    End If
    
    ' 2. 打开文件并计算哈希
    hFile = CreateFileW(pwszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        Print "CreateFile failed: " & WinErrorMsg(GetLastError) & GetLastError
        Goto cleanup
    End If
    
    If CryptCATAdminCalcHashFromFileHandle(hFile, @cbHash, @rgbHash(0), 0) = False Then
        Print "CryptCATAdminCalcHashFromFileHandle failed: " & WinErrorMsg(GetLastError) & GetLastError
        Goto cleanup
    End If
    
    ' 分配字节缓冲区，最后转换
    buffer = Space(cbHash * 2)  ' 分配ANSI字符串空间
    For dw As DWORD = 0 To cbHash - 1
        Mid(buffer, dw * 2 + 1, 2) = Hex(rgbHash(dw), 2)
    Next
    *pszMemberTag = WStr(buffer)  ' 一次性转换为宽字符串
    
    CloseHandle(hFile)
    hFile = INVALID_HANDLE_VALUE
    
    ' 3. 根据哈希查找目录
    hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, @rgbHash(0), cbHash, 0, NULL)
    If hCatInfo = NULL Then
        'Print "CryptCATAdminEnumCatalogFromHash failed. No catalog found for the file." & WinErrorMsg(GetLastError) & GetLastError
        'Goto cleanup
        Goto CheckFileSign
    End If
    
    ' 4. 获取目录文件信息
    catalogInfo.cbStruct = SizeOf(catalogInfo)
    If CryptCATCatalogInfoFromContext(hCatInfo, @catalogInfo, 0) Then
        'Print catalogInfo.wszCatalogFile
        ' 准备WINTRUST_CATALOG_INFO结构
        /'Dim wti As WINTRUST_CATALOG_INFO
        wti.cbStruct = SizeOf(wti)
        'wti.dwCatalogVersion = 0 ' 使用默认值
        
        wti.pcwszMemberTag = pszMemberTag
        wti.pcwszMemberFilePath = pwszFilePath
        wti.pcwszCatalogFilePath = @catalogInfo.wszCatalogFile
        wti.pbCalculatedFileHash = @rgbHash(0)
        wti.cbCalculatedFileHash = cbHash'/
        'wti.hMemberFile = 0 ' INVALID_HANDLE_VALUE
        Dim wti As WINTRUST_CATALOG_INFO = Type<WINTRUST_CATALOG_INFO>( _
            SizeOf(WINTRUST_CATALOG_INFO), _ ' cbStruct
            0, _                              ' dwCatalogVersion
            @catalogInfo.wszCatalogFile, _ ' pcwszCatalogFilePath
            pszMemberTag, _                   ' pcwszMemberTag
            pwszFilePath, _                   ' pcwszMemberFilePath
            NULL, _                           ' hMemberFile
            @rgbHash(0), _                    ' pbCalculatedFileHash
            cbHash, _                         ' cbCalculatedFileHash
            NULL _                            ' pcwszCatalogMemberTag
        )
        
        ' 准备WINTRUST_DATA结构
        Dim wtd As WINTRUST_DATA
        wtd.cbStruct = SizeOf(wtd)
        wtd.dwUIChoice = WTD_UI_NONE
        wtd.pCatalog = @wti
        wtd.dwUnionChoice = WTD_CHOICE_CATALOG
        wtd.fdwRevocationChecks = RevocationChecks
        wtd.dwStateAction = StateAction
        For a2 As Integer = 0 To 1
            wtd.dwProvFlags = ProvFlags(a2)
            
            ' 5. 调用WinVerifyTrust验证目录签名
            Dim Status As Long = WinVerifyTrust(NULL, @wvtGUID, @wtd)
            'Print "WinVerifyTrust:" & Hex(Status)
            fVerified = (Status = ERROR_SUCCESS)
            'Print RevocationChecks(a1) & " " & StateAction & " " & wtd.dwProvFlags
            'If Status = ERROR_SUCCESS Then Print "有效!" Else Print Hex(Status)
            ' 关闭验证状态
            wtd.dwStateAction = WTD_STATEACTION_CLOSE
            WinVerifyTrust(NULL, @wvtGUID, @wtd)
        Next
    Else
CheckFileSign:
        Dim wfi As WINTRUST_FILE_INFO
        With wfi
            .cbStruct = SizeOf(WINTRUST_FILE_INFO) ' 必须初始化结构体大小
            .pcwszFilePath = pwszFilePath            ' 待验证文件路径
            .hFile = NULL                          ' 不使用文件句柄
            .pgKnownSubject = NULL                 ' 无已知主题
        End With
        
        Dim wtd As WINTRUST_DATA
        With wtd
            .cbStruct = SizeOf(WINTRUST_DATA)      ' 必须初始化结构体大小
            .dwUnionChoice = WTD_CHOICE_FILE       ' 验证类型：文件
            .pFile = @wfi                          ' 关联文件验证结构体
            .dwUIChoice = WTD_UI_NONE              ' 不显示UI
            .fdwRevocationChecks = WTD_REVOKE_NONE ' 不检查吊销
            .dwStateAction = WTD_STATEACTION_VERIFY
            .dwProvFlags = WTD_SAFER_FLAG          ' 启用SAFER模式
            .hWVTStateData = NULL                  ' 无状态数据
            .pwszURLReference = NULL               ' 无URL参考
        End With
        ' 5. 调用WinVerifyTrust验证文件签名
        Dim Status As Long = WinVerifyTrust(NULL, @wvtGUID, @wtd)
        'Print "WinVerifyTrust:" & Hex(Status)
        fVerified = (Status = ERROR_SUCCESS)
        'Print RevocationChecks(a1) & " " & StateAction & " " & wtd.dwProvFlags
        'If Status = ERROR_SUCCESS Then Print "有效!" Else Print Hex(Status)
        ' 关闭验证状态
        wtd.dwStateAction = WTD_STATEACTION_CLOSE
        WinVerifyTrust(NULL, @wvtGUID, @wtd)
    End If
    
cleanup:
    ' 6. 清理资源
    If hCatInfo Then
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0)
    End If
    If hCatAdmin Then
        CryptCATAdminReleaseContext(hCatAdmin, 0)
    End If
    If hFile <> INVALID_HANDLE_VALUE Then
        CloseHandle(hFile)
    End If
    If pszMemberTag <> NULL Then
        Deallocate pszMemberTag
    End If
    
    Return fVerified
End Function
