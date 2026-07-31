#inclib "Crypt32"
#include "win/winerror.bi"
#include "win/shlobj.bi"
#include "win/wincrypt.bi"
#include Once "win/strsafe.bi"

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

Type CATALOG_INFO
    cbStruct As DWORD
    wszCatalogFile As Wstring * 260 'MAX_PATH
End Type

Type AsyncTaskParam
    hWndForm As hWnd
    hWndControl As hWnd
    ctrlType    As Wstring * 20      ' 控件类型
    rowIndex    As Long          ' 行索引
    filePath As Wstring * 202 ' MAX_PATH ' 要验证签名的文件路径（你的耗时数据）
    'CompanyName As StringW ' 公司名称
End Type

Type COPY_PATH
    SourcePath As Wstring * MAX_PATH
    TargetPath As Wstring * MAX_PATH
    IsDirectory As Boolean
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

Const TRUST_E_BAD_SIGNATURE As Long = &H800B010F ' 签名无效/哈希不匹配
Const TRUST_E_EXPIRED As Long = &H800B0101       ' 证书过期/未生效
Const TRUST_E_CERT_SUSPENDED As Long = &H800B010C ' 证书被吊销

' 系统驱动/内核模块 → 用齿轮/系统组件图标
Const ICON_SYSTEM_COMPONENT = 188   ' DLL/系统文件图标

' ===============================
' 全局变量
' ===============================
Dim Shared TaskQueue() As AsyncTaskParam Ptr
Dim Shared QueueMutex As HANDLE
Dim Shared TaskEvent As HANDLE
Dim Shared Threads() As HANDLE
Const ThreadCount As Integer = 4

' 问题:EnumerateAllDisks() 会清空 g_DiskList 并重新枚举。
' 如果 CurrentNode 指向的是旧的磁盘节点，但 EnumerateAllDisks() 后磁盘索引可能变化（比如拔插U盘）。这会导致 targetDiskIndex 指向错误的磁盘。
' 如果要解决，需要更复杂的磁盘标识机制（如用磁盘签名或GUID）
Public Sub GetDriveList(CurrentNode As HTREEITEM, treFolder As Class_TreeView, ByVal IsPhysicalAnalyze As Boolean)
    
    ' ======================================================================
    ' 步骤 1：解析目标磁盘索引
    ' ======================================================================
    Dim targetDiskIndex As Integer = -1
    Dim bIsDiskNode As Boolean = False
    
    If CurrentNode <> NULL Then
        Dim nodeText As StringW = treFolder.Text(CurrentNode)
        Dim posDisk As Integer = InStrW(nodeText, "磁盘")
        If posDisk > 0 Then
            Dim j As Integer = posDisk + 2
            Dim diskNumStr As StringW = ""
            While j <= LenW(nodeText) And IsNumeric(MidW(nodeText, j, 1))
                diskNumStr &= MidW(nodeText, j, 1)
                j += 1
            Wend
            If diskNumStr <> "" Then
                targetDiskIndex = ValInt(diskNumStr)
                bIsDiskNode = True
            End If
        End If
        
        ' 非磁盘节点不处理
        If Not bIsDiskNode Then Return
        
        ' 删除该节点的子节点
        DeleteChildItem treFolder, CurrentNode
    Else
        ' 根级别，清空全部
        treFolder.DeleteAllItems
    End If
    
    ' ======================================================================
    ' 步骤 2：枚举磁盘数据
    ' ======================================================================
    EnumerateAllDisks()
    
    If g_CuiMode Then SafeStdOut("驱动器名称" & vbCrlfW)
    
    ' ======================================================================
    ' 步骤 3：生成树节点
    ' ======================================================================
    Dim d As Integer
    For d = 0 To g_DiskCount - 1
        With g_DiskList(d)
            ' 过滤：只处理目标磁盘
            If targetDiskIndex >= 0 And .diskIndex <> targetDiskIndex Then Continue For
            
            ' 构建磁盘节点文本
            Dim diskNodeText As String
            If Not IsPhysicalAnalyze Then
                diskNodeText = "磁盘" & .diskIndex
            Else
                diskNodeText = "磁盘" & .diskIndex & IIf(.isGPT, " [GPT]", " [MBR]")
            End If
            
            ' 获取或创建磁盘节点句柄
            Dim hDiskNode As HTREEITEM
            If CurrentNode <> NULL Then
                hDiskNode = CurrentNode
            Else
                If Not g_CuiMode Then
                    hDiskNode = treFolder.AddItem(NULL, diskNodeText)
                Else
                    SafeStdOut(diskNodeText & vbCrlfW)
                    hDiskNode = NULL
                End If
            End If
            
            ' 添加分区子节点
            Dim p As Integer
            For p = 0 To .partitionCount - 1
                ' 兼容模式跳过无盘符分区
                If Not IsPhysicalAnalyze And .partitions(p).drvLetter = "" Then Continue For
                
                Dim partText As String
                If .partitions(p).drvLetter <> "" Then
                    Dim volLabel As StringW = GetDiskLabel(.partitions(p).drvLetter & ":")
                    If volLabel = "" Then volLabel = "本地磁盘"
                    partText = volLabel & "(" & .partitions(p).drvLetter & ":)"
                Else
                    partText = .partitions(p).partitionName
                End If
                
                If g_CuiMode Then
                    SafeStdOut("  " & partText & vbCrlfW)
                ElseIf hDiskNode <> NULL Then
                    treFolder.AddItem(hDiskNode, partText)
                End If
            Next
        End With
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

Public Sub GetFileList(CurrentNode As HTREEITEM, treFolder As Class_TreeView, lvwFile As Class_ListView, ByVal IsEnumFolder As Boolean, IsEnumFile As Boolean = True, ByVal IsPhysicalAnalyze As Boolean)
    Dim szPath As StringW = "", IsBeAccess As Boolean = True
    If IsEnumFolder Then DeleteChildItem treFolder, CurrentNode
    InitListViewData lvwFile
    
    GetPathByNodeW CurrentNode, treFolder, szPath
    
    ' 方法一
    Dim fData() As WIN32_FIND_DATAW, i As ULong
    ReDim fData(10) As WIN32_FIND_DATAW
    Dim hFind As HANDLE = FindFirstFileW(szPath & "*.*",@fData(i))'不能使用@szPath,否则报"找不到文件"
    If hFind = INVALID_HANDLE_VALUE Then
        If IsPhysicalAnalyze Then
            IsBeAccess = False
        Else
            MyLog.PrintWin32Error "GetFileList", "FindFirstFileW", "szPath=" & szPath
            Exit Sub
        End If
    End If
    Do
        /'If (fData.cFileName = "." OrElse fData.cFileName = "..") Then Continue Do
        If fData.dwFileAttributes And fbDirectory Then
            If IsEnumFolder Then treFolder.InsertItem CurrentNode,TVI_LAST,fData.cFileName
        ElseIf IsEnumFile Then
            lvwFile.AddItem fData.cFileName
            lvwFile.SetItemText lvwFile.ItemCount - 1, 1, WStr(fData.nFileSizeHigh * (2 ^ 32) + fData.nFileSizeLow)
        End If '/
        i += 1
        If i >= UBound(fData) Then ReDim Preserve fData(UBound(fData) + 50) As WIN32_FIND_DATAW
        FF_DoEvents
    Loop While FindNextFileW(hFind,@fData(i)) <> 0
    FindClose hFind
    
    If Not IsPhysicalAnalyze Then
        For j As Integer = 0 To i - 1
            If (fData(j).cFileName = "." OrElse fData(j).cFileName = "..") Then Continue For
            If fData(j).dwFileAttributes And fbDirectory Then
                If IsEnumFolder Then treFolder.InsertItem CurrentNode,TVI_LAST,fData(j).cFileName
            ElseIf IsEnumFile Then
                Dim ItemIndex As Long = AddItemColListEx(lvwFile, 3,,, WStr(fData(j).cFileName), WStr(""), WStr(fData(j).nFileSizeHigh * (2 ^ 32) + fData(j).nFileSizeLow))
                SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex(fData(j).cFileName, False)
            End If
            'FF_DoEvents
        Next
    Else
        'InitListViewData lvwFile
        ' 方法二
        Dim fileInfo() As WIN32_FILE_DATAEX2W, listCount As Long
        listCount = GetDirList(szPath, fileInfo())
        If listCount < 0 Then Exit Sub
        For j As Integer = 0 To listCount - 1
            If fileInfo(j).bDirectory Then
                If IsEnumFolder Then treFolder.InsertItem CurrentNode,TVI_LAST,fileInfo(j).cFileName
            ElseIf IsEnumFile Then
                Dim ItemIndex As Long = AddItemColListEx(lvwFile, 3,,, WStr(fileInfo(j).cFileName), WStr(""), WStr(fileInfo(j).dwFileSizeHigh * (2 ^ 32) + fileInfo(j).dwFileSizeLow))
                If fileInfo(j).bSuspicious Then ' 是ADS流文件
                    SetItemTextEx lvwFile, ItemIndex, 1, "Stream"
                    SetItemColor lvwFile, ItemIndex, FB_Black, FB_GoldenYellow
                End If
                SetItemIconEx lvwFile, ItemIndex, GetFileIconIndex(fileInfo(j).cFileName, False)
                If IsBeAccess Then
                    For k As Integer = 0 To i - 1
                        If fData(k).cFileName = fileInfo(j).cFileName Then Continue For, For ' 找到则进入下一次循环
                    Next
                End If
                
                ' 找不到则标红
                Dim ForeColor As COLORREF, BackColor As COLORREF, pos1 As Integer, szPath1 As StringW
                pos1 = InStrW(szPath, ":")
                szPath1 = RightW(szPath, LenW(szPath) - pos1)
                'Print "fileInfo(j).cFileName:" & fileInfo(j).cFileName & " fileInfo(j).bSuspicious:" & fileInfo(j).bSuspicious
                If (fileInfo(j).mftRecordNumber >= 0 AndAlso fileInfo(j).mftRecordNumber <= 16) Then ' 是元文件(如$MFT,$Extend\$ObjId)
                    SetItemTextEx lvwFile, ItemIndex, 1, "Metafile"
                    SetItemColor lvwFile, ItemIndex, FB_LightPinkRed, FB_White
                Else ' 是隐藏文件!
                    SetItemTextEx lvwFile, ItemIndex, 1, "Hidden"
                    SetItemColor lvwFile, ItemIndex, FB_Red, FB_White
                End If
            End If
            FF_DoEvents
        Next
    End If
    CommitListViewView lvwFile
End Sub

' ===================== CLI 专属文件列表导出 =====================
' 参数：
'   szPath            - 绝对路径（如 "C:\Windows\"）
'   IsPhysicalAnalyze - 是否启用底层 MFT 分析（检测 Hidden/Stream/Metafile）
'   IsEnumFolder      - 是否输出目录
'   IsEnumFile        - 是否输出文件
Public Sub GetFileList_CLI(ByVal szPath As StringW, ByVal IsPhysicalAnalyze As Boolean = False, ByVal IsEnumFolder As Boolean = True, ByVal IsEnumFile As Boolean = True)
    Dim colNames(2) As StringW = {"名称", "类型", "大小"}
    
    ' 规范化路径
    If RightW(szPath, 1) <> "\" AndAlso RightW(szPath, 1) <> "/" Then
        szPath = szPath & "\"
    End If
    
    ' ====== 1. 普通 Win32 枚举 ======
    Dim fData() As WIN32_FIND_DATAW, i As ULong
    ReDim fData(10) As WIN32_FIND_DATAW
    Dim IsBeAccess As Boolean = True
    i = 0
    
    Dim hFind As HANDLE = FindFirstFileW(szPath & "*.*", @fData(i))
    If hFind = INVALID_HANDLE_VALUE Then
        If IsPhysicalAnalyze Then
            IsBeAccess = False
        Else
            MyLog.PrintWin32Error "GetFileList_CLI", "FindFirstFileW", "szPath=" & szPath
            Dim emptyRows() As CuiFlatRow
            ReDim emptyRows(-1) As CuiFlatRow
            CuiPrintFlatData("FILE", emptyRows(), colNames(), 3)
            Exit Sub
        End If
    Else
        Do
            i += 1
            If i >= UBound(fData) Then ReDim Preserve fData(UBound(fData) + 50) As WIN32_FIND_DATAW
        Loop While FindNextFileW(hFind, @fData(i)) <> 0
        FindClose hFind
    End If
    
    ' ====== 2. 物理分析模式：获取底层 MFT 数据 ======
    Dim fileInfo() As WIN32_FILE_DATAEX2W, listCount As Long
    If IsPhysicalAnalyze Then
        listCount = GetDirList(szPath, fileInfo())
        If listCount < 0 Then
            Dim emptyRows2() As CuiFlatRow
            ReDim emptyRows2(-1) As CuiFlatRow
            CuiPrintFlatData("FILE", emptyRows2(), colNames(), 3)
            Exit Sub
        End If
    End If
    
    ' ====== 3. 填充扁平行缓冲区 ======
    Dim maxRows As Long = IIf(IsPhysicalAnalyze, listCount, i)
    If maxRows <= 0 Then
        Dim emptyRows3() As CuiFlatRow
        ReDim emptyRows3(-1) As CuiFlatRow
        CuiPrintFlatData("FILE", emptyRows3(), colNames(), 3)
        Exit Sub
    End If
    
    Dim rows(maxRows - 1) As CuiFlatRow
    Dim rowIdx As Long = 0
    
    ' ---------- 普通模式 ----------
    If Not IsPhysicalAnalyze Then
        For j As Integer = 0 To i - 1
            If (fData(j).cFileName = "." OrElse fData(j).cFileName = "..") Then Continue For
            
            Dim isDir As Boolean = (fData(j).dwFileAttributes And fbDirectory) <> 0
            If isDir AndAlso Not IsEnumFolder Then Continue For
            If Not isDir AndAlso Not IsEnumFile Then Continue For
            
            rows(rowIdx).ColCount = 3
            rows(rowIdx).Cols(0) = fData(j).cFileName
            rows(rowIdx).Cols(1) = IIf(isDir, "Directory", "")
            rows(rowIdx).Cols(2) = IIf(isDir, WStr(""), WStr(fData(j).nFileSizeHigh * (2 ^ 32) + fData(j).nFileSizeLow))
            rowIdx += 1
        Next
        
    ' ---------- 物理分析模式 ----------
    Else
        For j As Integer = 0 To listCount - 1
            Dim isDir As Boolean = fileInfo(j).bDirectory
            If isDir AndAlso Not IsEnumFolder Then Continue For
            If Not isDir AndAlso Not IsEnumFile Then Continue For
            
            rows(rowIdx).ColCount = 3
            rows(rowIdx).Cols(0) = fileInfo(j).cFileName
            
            If isDir Then
                rows(rowIdx).Cols(1) = "Directory"
                rows(rowIdx).Cols(2) = ""
            Else
                Dim fileType As StringW = ""
                Dim fileSize As StringW = WStr(fileInfo(j).dwFileSizeHigh * (2 ^ 32) + fileInfo(j).dwFileSizeLow)
                
                If fileInfo(j).bSuspicious Then
                    fileType = "Stream"
                ElseIf IsBeAccess Then
                    Dim foundInNormal As Boolean = False
                    For k As Integer = 0 To i - 1
                        If fData(k).cFileName = fileInfo(j).cFileName Then
                            foundInNormal = True
                            Exit For
                        End If
                    Next
                    
                    If foundInNormal Then
                        fileType = ""
                    ElseIf fileInfo(j).mftRecordNumber >= 0 AndAlso fileInfo(j).mftRecordNumber <= 16 Then
                        fileType = "Metafile"
                    Else
                        fileType = "Hidden"
                    End If
                Else
                    If fileInfo(j).mftRecordNumber >= 0 AndAlso fileInfo(j).mftRecordNumber <= 16 Then
                        fileType = "Metafile"
                    Else
                        fileType = "Hidden"
                    End If
                End If
                
                rows(rowIdx).Cols(1) = fileType
                rows(rowIdx).Cols(2) = fileSize
            End If
            rowIdx += 1
        Next
    End If
    
    ' ====== 4. 输出 ======
    If rowIdx > 0 Then
        ReDim Preserve rows(rowIdx - 1) As CuiFlatRow
        CuiPrintFlatData("FILE", rows(), colNames(), 3)
    Else
        Dim emptyRows() As CuiFlatRow
        CuiPrintFlatData("FILE", emptyRows(), colNames(), 3)
    End If
End Sub

Sub GetPathByNodeW(Node As HTREEITEM, treFolder As Class_TreeView, ByRef FilePath As StringW)
    Dim CurrentNode As HTREEITEM = Node
    Dim ParentNode As HTREEITEM
    
    ' ========== 第一步：从叶子向上遍历，收集所有节点文本 ==========
    Dim nodeTexts() As StringW
    Dim nodeCount As Integer = 0
    
    Do
        ParentNode = treFolder.GetParent(CurrentNode)
        Dim NodeText As StringW = treFolder.Text(CurrentNode)
        
        ReDim Preserve nodeTexts(0 To nodeCount)
        nodeTexts(nodeCount) = NodeText
        nodeCount += 1
        
        CurrentNode = ParentNode
    Loop While CurrentNode <> NULL
    
    ' nodeTexts(0) = 叶子节点
    ' nodeTexts(nodeCount-1) = 根节点
    
    ' ========== 第二步：分析根节点，确定是否为物理模式 ==========
    Dim rootText As StringW = nodeTexts(nodeCount - 1)
    Dim bIsPhysicalMode As Boolean = False
    Dim diskNumStr As StringW = ""
    
    ' 检查根节点是否是磁盘节点（包含 "磁盘" 字样）
    Dim posDisk As Integer = InStrW(rootText, "磁盘")
    If posDisk > 0 Then
        bIsPhysicalMode = True
        ' 提取磁盘号
        Dim j As Integer = posDisk + 2
        While j <= LenW(rootText)
            Dim ch As StringW = MidW(rootText, j, 1)
            If IsNumeric(ch) Then
                diskNumStr &= ch
            Else
                Exit While
            End If
            j += 1
        Wend
    End If
    
    ' ========== 第三步：分析分区节点 ==========
    Dim partitionHasDriveLetter As Boolean = False
    Dim partitionDriveLetter As StringW = ""
    Dim partitionPhysicalIndex As StringW = ""
    
    ' 分区节点索引：物理模式是 nodeCount-2，非物理模式是 nodeCount-1（根节点自身）
    Dim partIdx As Integer
    If bIsPhysicalMode Then
        partIdx = nodeCount - 2
    Else
        partIdx = nodeCount - 1
    End If
    
    If partIdx >= 0 Then
        Dim partText As StringW = nodeTexts(partIdx)
        Dim pos1 As Integer = InStrRevW(partText, "(")
        Dim pos2 As Integer = InStrRevW(partText, ")")
        
        If pos1 > 0 And pos2 > pos1 Then
            Dim inner As StringW = MidW(partText, pos1 + 1, pos2 - pos1 - 1)
            If RightW(inner, 1) = ":" Then
                partitionHasDriveLetter = True
                partitionDriveLetter = inner
            ElseIf IsNumeric(inner) Then
                partitionPhysicalIndex = "Partition" & (ValInt(inner) + 1)
            End If
        End If
    End If
    
    ' ========== 第四步：构建路径 ==========
    Dim FullPath As StringW = ""
    
    If partitionHasDriveLetter Then
        ' ========== 有盘符路径 ==========
        FullPath = partitionDriveLetter
        
        ' 子目录起始索引：物理模式从 nodeCount-3，非物理模式从 nodeCount-2
        Dim startIdx As Integer
        If bIsPhysicalMode Then
            startIdx = nodeCount - 3
        Else
            startIdx = nodeCount - 2
        End If
        
        For i As Integer = startIdx To 0 Step -1
            If nodeTexts(i) <> "" Then
                FullPath = FullPath & "\" & nodeTexts(i)
            End If
        Next
        
    ElseIf bIsPhysicalMode And diskNumStr <> "" And partitionPhysicalIndex <> "" Then
        ' ========== 无盘符物理路径 ==========
        FullPath = "PhysicalDrive" & diskNumStr
        FullPath = FullPath & "\" & partitionPhysicalIndex
        
        For i As Integer = nodeCount - 3 To 0 Step -1
            If nodeTexts(i) <> "" Then
                FullPath = FullPath & "\" & nodeTexts(i)
            End If
        Next
        
    ElseIf bIsPhysicalMode And diskNumStr <> "" Then
        ' 只有磁盘根节点
        FullPath = "PhysicalDrive" & diskNumStr
        
    Else
        ' 异常情况，兜底
        For i As Integer = nodeCount - 1 To 0 Step -1
            If FullPath = "" Then
                FullPath = nodeTexts(i)
            Else
                FullPath = FullPath & "\" & nodeTexts(i)
            End If
        Next
    End If
    
    FilePath = FullPath & "\"
End Sub

' ============================================================================
' 根据路径获取节点（支持有盘符和无盘符分区）
' ============================================================================
Public Function GetNodeByFolderPathW(sPath As StringW, treFolder As Class_TreeView, lvwFile As Class_ListView) As HTREEITEM
    ' 规范化路径
    Dim sNormalizedPath As StringW = sPath
    sNormalizedPath = YF_ReplaceW(sNormalizedPath, "/", "\")
    If RightW(sNormalizedPath, 1) = "\" Then
        sNormalizedPath = LeftW(sNormalizedPath, LenW(sNormalizedPath) - 1)
    End If

    ' 分割路径
    Dim sParts() As StringW
    Dim iNumParts As Integer
    iNumParts = vbSplitW(sNormalizedPath, "\", sParts())

    If iNumParts < 1 Then Return NULL

    ' ==================================================================
    ' 第一步：判断路径类型
    ' ==================================================================
    Dim bIsPhysicalPath As Boolean = False
    Dim sTargetDisk As StringW = ""
    Dim sTargetPartition As StringW = ""
    Dim sTargetDrive As StringW = ""

    If LeftW(sParts(0), 14) = "PhysicalDrive" Then
        ' 无盘符物理路径: PhysicalDrive0\Partition1\...
        bIsPhysicalPath = True
        sTargetDisk = MidW(sParts(0), 15)
        If iNumParts >= 2 Then sTargetPartition = sParts(1)
    ElseIf RightW(sParts(0), 1) = ":" Then
        ' 有盘符路径: C:\Windows\... 或 C:
        bIsPhysicalPath = False
        sTargetDrive = sParts(0)
    Else
        Return NULL
    End If

    ' ==================================================================
    ' 第二步：匹配根节点（磁盘节点）
    ' ==================================================================
    Dim hCurrentNode As HTREEITEM = treFolder.GetRoot()
    Dim hRootNode As HTREEITEM = NULL
    Dim hPartitionNode As HTREEITEM = NULL
    Dim sNodeText As StringW

    While hCurrentNode <> NULL
        sNodeText = treFolder.Text(hCurrentNode)

        ' ========== 统一匹配磁盘根节点 "磁盘0 [GPT]" ==========
        Dim posDisk As Integer = InStrW(sNodeText, "磁盘")
        If posDisk > 0 Then
            Dim j As Integer = posDisk + 2
            Dim diskNum As StringW = ""
            While j <= LenW(sNodeText)
                Dim ch As StringW = MidW(sNodeText, j, 1)
                If IsNumeric(ch) Then
                    diskNum &= ch
                Else
                    Exit While
                End If
                j += 1
            Wend

            If bIsPhysicalPath Then
                ' 物理模式：直接匹配磁盘号
                If diskNum = sTargetDisk Then
                    hRootNode = hCurrentNode
                    Exit While
                End If
            Else
                ' 盘符模式：需要遍历该磁盘下的分区节点，找匹配的盘符
                Dim hTestPartition As HTREEITEM = treFolder.GetChild(hCurrentNode)
                Dim bDriveFound As Boolean = False
                While hTestPartition <> NULL
                    Dim sPartText As StringW = treFolder.Text(hTestPartition)
                    Dim pos1 As Integer = InStrRevW(sPartText, "(")
                    Dim pos2 As Integer = InStrRevW(sPartText, ")")
                    If pos1 > 0 And pos2 > pos1 Then
                        Dim sDriveInNode As StringW = MidW(sPartText, pos1 + 1, pos2 - pos1 - 1)
                        If UCaseW(sDriveInNode) = UCaseW(sTargetDrive) Then
                            hRootNode = hCurrentNode
                            hPartitionNode = hTestPartition
                            bDriveFound = True
                            Exit While
                        End If
                    End If
                    hTestPartition = treFolder.GetNextSiblin(hTestPartition)
                Wend
                If bDriveFound Then Exit While
            End If
        End If

        hCurrentNode = treFolder.GetNextSiblin(hCurrentNode)
    Wend

    If hRootNode = NULL Then Return NULL

    ' ==================================================================
    ' 第三步：物理模式下需要额外匹配分区节点
    ' ==================================================================
    If bIsPhysicalPath Then
        hPartitionNode = NULL
        hCurrentNode = treFolder.GetChild(hRootNode)

        While hCurrentNode <> NULL
            sNodeText = treFolder.Text(hCurrentNode)

            ' 无盘符：匹配 Partition1 → 找节点中包含 (0) 的
            If LeftW(sTargetPartition, 9) = "Partition" Then
                Dim targetPartIdx As Integer = ValInt(MidW(sTargetPartition, 10)) - 1
                Dim pos1 As Integer = InStrRevW(sNodeText, "(")
                Dim pos2 As Integer = InStrRevW(sNodeText, ")")
                If pos1 > 0 And pos2 > pos1 Then
                    Dim partIdxStr As StringW = MidW(sNodeText, pos1 + 1, pos2 - pos1 - 1)
                    If IsNumeric(partIdxStr) And ValInt(partIdxStr) = targetPartIdx Then
                        hPartitionNode = hCurrentNode
                        Exit While
                    End If
                End If
            End If

            hCurrentNode = treFolder.GetNextSiblin(hCurrentNode)
        Wend

        If hPartitionNode = NULL Then Return NULL

        ' 只有两级（PhysicalDrive0\Partition1），返回分区节点
        If iNumParts = 2 Then Return hPartitionNode
    Else
        ' 非物理模式下：盘符节点在磁盘下，已在上一步找到
        If iNumParts = 1 Then Return hPartitionNode
    End If

    ' ==================================================================
    ' 第四步：逐级匹配子目录
    ' ==================================================================
    hCurrentNode = hPartitionNode

    ' 确定子目录起始索引
    Dim startIdx As Integer
    If bIsPhysicalPath Then
        startIdx = 2  ' PhysicalDrive0\Partition1\子目录1\...
    Else
        startIdx = 1  ' C:\子目录1\...（有盘符时第二部分就是子目录）
    End If

    ' 先展开分区节点
    GetFileList hCurrentNode, treFolder, lvwFile, True, False, GetMenuCheckState(FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
    treFolder.ExpandEx hCurrentNode, TVE_EXPAND
    treFolder.EnsureVisible hCurrentNode

    Dim i As Integer
    Dim hChildNode As HTREEITEM
    Dim bFound As Boolean

    For i = startIdx To iNumParts - 1
        If sParts(i) = "" Then Continue For

        bFound = False
        hChildNode = treFolder.GetChild(hCurrentNode)

        While hChildNode <> NULL
            sNodeText = treFolder.Text(hChildNode)

            If UCaseW(sNodeText) = UCaseW(sParts(i)) Then
                bFound = True
                hCurrentNode = hChildNode

                If i < iNumParts - 1 Then
                    If Not treFolder.Expand(hCurrentNode) Then
                        GetFileList hCurrentNode, treFolder, lvwFile, True, False, GetMenuCheckState(FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
                        treFolder.ExpandEx hCurrentNode, TVE_EXPAND
                        treFolder.Selection = hCurrentNode
                    End If
                End If
                Exit While
            End If

            hChildNode = treFolder.GetNextSiblin(hChildNode)
        Wend

        If Not bFound Then
            MyLog.PrintInfo "GetNodeByFolderPathW",, "Not found: " & sParts(i)
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

Function ForceDeleteFile(ByVal lpFilePath As LPCWSTR) As Boolean
    Dim strFile As LPWSTR = Allocate(MAX_PATH * SizeOf(WString))
    If (strFile = NULL) Then
        MyLog.PrintLog LOG_ERROR, "ForceDeleteFile", "Allocate"
        Return False
    End If
    *strFile = "\??\" & *lpFilePath
    
    If IsDriverLoaded Then
        #define STATUS_CANNOT_DELETE Cast(NTSTATUS, &HC0000121)
        Dim status As NTSTATUS = MyDeleteFile(strFile)
        SetLastError 0
        If status = STATUS_CANNOT_DELETE Then ' 是正在运行的可执行文件
            If IoControl(hDrv, IOCTL_DeleteFileByIRP, strFile, MAX_PATH * SizeOf(Wstring)) = 0 Then
                MyLog.PrintWin32Error "ForceDeleteFile", "IOCTL_DeleteFileByIRP", "strFile=" & *strFile
                Deallocate strFile
                Return False
            End If
        Else ' 是被打开的文件
            If IoControl(hDrv, IOCTL_DeleteFileByXCB, strFile, MAX_PATH * SizeOf(Wstring)) = 0 Then
                MyLog.PrintWin32Error "ForceDeleteFile", "IOCTL_DeleteFileByXCB", "strFile=" & *strFile
                Deallocate strFile
                Return False
            End If
            If DeleteFile(strFile) = 0 Then
                MyLog.PrintWin32Error "ForceDeleteFile", "DeleteFile", "strFile=" & *strFile
                Deallocate strFile
                Return False
            End If
        End If
    End If
    Deallocate strFile
    Return True
End Function

Function DeleteDirectoryIterative(ByVal lpPath As StringW) As Boolean
    ' 使用栈模拟递归，避免深层嵌套导致栈溢出
    ' 栈元素：待处理的文件夹路径
    Dim stackPaths() As StringW
    ReDim stackPaths(0 To 15)  ' 预分配，动态扩容
    Dim stackTop As Long = -1  ' 栈顶指针，-1表示空
    
    ' 辅助函数：压栈
    #Macro PushStack(p)
        stackTop += 1
        If stackTop > UBound(stackPaths) Then
            ReDim Preserve stackPaths(0 To UBound(stackPaths) * 2 + 1)
        End If
        stackPaths(stackTop) = (p)
    #EndMacro
    
    ' 辅助函数：弹栈
    #Macro PopStack()
        stackTop -= 1
    #EndMacro
    
    ' 第一步：将初始路径压栈
    PushStack(lpPath)
    
    ' 栈不为空时循环处理
    While stackTop >= 0
        Dim wszCurrent As StringW = stackPaths(stackTop)
        PopStack()
        
        ' 枚举当前目录内容
        Dim wfd As WIN32_FIND_DATAW
        Dim hFind As HANDLE
        Dim wszSearch As StringW = wszCurrent & "\*"
        
        hFind = FindFirstFile(wszSearch, @wfd)
        If hFind = INVALID_HANDLE_VALUE Then
            ' 无法打开或为空，尝试直接删除文件夹
            RemoveDirectory(wszCurrent)
            Continue While
        End If
        
        ' 先收集所有子项，避免FindFirstFile/FindNextFile句柄冲突
        Dim subDirs() As StringW
        Dim subFiles() As StringW
        Dim dirCount As Long = 0
        Dim fileCount As Long = 0
        ReDim subDirs(0 To 7)
        ReDim subFiles(0 To 15)
        
        Do
            If wfd.cFileName <> "." AndAlso wfd.cFileName <> ".." Then
                Dim wszFull As StringW = wszCurrent & "\" & wfd.cFileName
                If (wfd.dwFileAttributes And FILE_ATTRIBUTE_DIRECTORY) <> 0 Then
                    ' 子文件夹
                    If dirCount > UBound(subDirs) Then
                        ReDim Preserve subDirs(0 To UBound(subDirs) * 2 + 1)
                    End If
                    subDirs(dirCount) = wszFull
                    dirCount += 1
                Else
                    ' 文件
                    If fileCount > UBound(subFiles) Then
                        ReDim Preserve subFiles(0 To UBound(subFiles) * 2 + 1)
                    End If
                    subFiles(fileCount) = wszFull
                    fileCount += 1
                End If
            End If
        Loop While FindNextFile(hFind, @wfd)
        
        FindClose hFind
        
        ' 先删除所有文件
        For i As Long = 0 To fileCount - 1
            DeleteFile(subFiles(i))
        Next
        
        ' 如果没有子文件夹，直接删除当前文件夹
        If dirCount = 0 Then
            RemoveDirectory(wszCurrent)
        Else
            ' 有子文件夹：先把自己重新压栈（等子文件夹处理完后再删）
            PushStack(wszCurrent)
            ' 再把所有子文件夹压栈（后进先出，保证深度优先）
            For i As Long = dirCount - 1 To 0 Step -1
                PushStack(subDirs(i))
            Next
        End If
    Wend
    
    Return True
End Function

Function LocateFilePath(ByVal lpFilePath As StringW) As Boolean
    If Not FileExists(Str(lpFilePath)) Then Return False
    Dim pos1 As Integer = InStrRevW(lpFilePath, "\")
    Dim FileName As StringW = UCaseW(RightW(lpFilePath, LenW(lpFilePath) - pos1))
    Dim FolderPath As StringW = UCaseW(LeftW(lpFilePath, pos1 - 1))
    
    If InStrW(lpFilePath, "PhysicalDrive") = 1 Then
        MyLog.PrintInfo "LocateFilePath",, lpFilePath & ":无盘符"
        SetMenuCheckState FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze, True
    End If
    SwitchToModule "文件"
    Dim FolderNode As HTREEITEM = GetNodeByFolderPathW(FolderPath, FrmMain.TreeView, FrmMain.ListView1)
    'SendMessage TreeView.hWnd, TVM_SELECTITEM, TVGN_CARET, Cast(lParam, FolderNode) ' 选中这项
    
    GetFileList FolderNode, FrmMain.TreeView, FrmMain.ListView1, False, True, GetMenuCheckState(FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze)
    
    For i As Integer = 0 To FrmMain.ListView1.ItemCount - 1
        If UCaseW(FrmMain.ListView1.GetItemText(i, 0)) = FileName Then
            FrmMain.ListView1.SelectedItem = i
            ListView_EnsureVisible(FrmMain.ListView1.hWnd, i, False)
            
            SetMenuCheckState FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze, False
            Return True
        End If
    Next
    
    SetMenuCheckState FrmMain.mnuFolder, FrmMain_mnuFolder_mnuEnablePhysicalAnalyze, False
    Return False
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
        MyLog.PrintWin32Error "ILCreateFromPathW", "lpFilePath=" & *lpFilePath
        Return False
    End If 
End Function

' 封装一个更易用的复制函数
Function FB_ShellCopyFile(sFrom As StringW, sTo As StringW, ByVal hOwner As HWND = NULL) As Long
    Dim FileOp As SHFILEOPSTRUCTW
    Dim wsFrom As WString * (MAX_PATH + 2) ' +2 是为了存放结尾的双null终止符
    Dim wsTo As WString * (MAX_PATH + 2)
    Dim ret As Long = 0

    ' 1. 初始化结构体
    ZeroMemory(@FileOp, SizeOf(SHFILEOPSTRUCTW))

    ' 2. 准备源和目标缓冲区。SHFileOperation 要求路径字符串以双null结尾
    wsFrom = sFrom & WChr(0)
    wsTo   = sTo   & WChr(0)

    ' 3. 填充结构体
    FileOp.hWnd   = hOwner                          ' 父窗口句柄，用于显示对话框
    FileOp.wFunc  = FO_COPY                         ' 操作类型：复制
    FileOp.pFrom  = @wsFrom                         ' 指向源文件/文件夹路径
    FileOp.pTo    = @wsTo                           ' 指向目标文件夹路径
    FileOp.fFlags = FOF_ALLOWUNDO Or FOF_SILENT' 操作标志：允许撤销 | 完全静默操作，不显示任何进度对话框
    ' 其他可用标志，可按需组合：
    ' FOF_NOCONFIRMMKDIR : 创建目录时不提示
    ' FOF_NOCONFIRMATION  : 对所有对话框都选择“是”。
    ' FOF_RENAMEONCOLLISION : 在发生冲突时自动重命名新文件。
    ' FOF_NOERRORUI       : 发生错误时不显示用户界面。

    ' 4. 执行文件操作
    ret = SHFileOperationW(@FileOp)

    ' 5. 检查用户是否取消了操作
    If FileOp.fAnyOperationsAborted <> 0 Then
        MyLog.PrintInfo "FB_ShellCopyFile", "SHFileOperationW", "操作被用户取消!"
        Return False
    End If
    If ret = 0 Then
        Return True
    Else
        MyLog.PrintNtError "FB_ShellCopyFile", "SHFileOperationW", ret
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

'040904B0
' ============================================
' 通用函数：获取文件版本信息中的任意字符串值
' ============================================
Function GetFileVersionString(FileName As StringW, ByRef szKey As WString) As StringW
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
    
    dwSize = GetFileVersionInfoSizeW(FileName, @dwDummy)
    If dwSize = 0 Then Return ""

    pBuffer = Allocate(dwSize)
    If pBuffer = 0 Then Return ""

    If GetFileVersionInfoW(FileName, 0, dwSize, pBuffer) Then
        If VerQueryValueW(pBuffer, StrPtrW("\VarFileInfo\Translation"), @pTransTable, @nQuerySize) Then
            langCharset = (pTransTable[0].wLanguage Shl 16) Or pTransTable[0].wCodePage
            subBlock = "\StringFileInfo\" & Hex(langCharset, 8) & "\" & szKey
            
            If VerQueryValueW(pBuffer, StrPtrW(subBlock), @lpBuffer, @dwLen) Then
                If dwLen >= 2 Then
                    CopyMemory(StrPtr(lpData), lpBuffer, dwLen * 2)
                    Deallocate pBuffer
                    Return lpData
                End If
            End If
            MyLog.PrintLog LOG_INFO, "GetFileVersionString", "VerQueryValueW", "未找到 " & szKey & ", Path=" & FileName
            Return ""
        Else
            MyLog.PrintWin32Error "GetFileVersionString", "VerQueryValueW", "Translation失败, Path=" & FileName
        End If
    Else
        MyLog.PrintWin32Error "GetFileVersionString", "GetFileVersionInfoW", "Path=" & FileName
    End If

    Deallocate pBuffer
    Return "-"
End Function

' ============================================
' 包装函数：保持原有调用方式不变
' ============================================
Function GetFileCompany(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "CompanyName")
End Function

Function GetFileDescription(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "FileDescription")
End Function

Function GetFileVersion(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "FileVersion")
End Function

Function GetProductName(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "ProductName")
End Function

Function GetProductVersion(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "ProductVersion")
End Function

Function GetOriginalFilename(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "OriginalFilename")
End Function

Function GetLegalCopyright(FileName As StringW) As StringW
    Return GetFileVersionString(FileName, "LegalCopyright")
End Function

Function GetFileCompany_Opt(ByVal FileName As StringW) As StringW
    ' 静态缓冲区全局复用，避免频繁堆分配释放
    Static verBuf As UByte Ptr = NULL
    Static verBufSize As DWORD = 0
    Dim dwSize As DWORD
    Dim dwDummy As DWORD
    Dim lpData As WString * MAX_PATH = ""
    Dim lpBuffer As PVOID
    Dim dwLen As DWORD
    Dim pTransTable As LANGANDCODEPAGE Ptr
    Dim nQuerySize As DWORD
    Dim langCharset As DWORD
    Dim subBlock As StringW

    ' 短路1：无盘符路径（System/Idle等内核进程）直接返回
    If InStrW(FileName, ":") = 0 Then
        Return "-"
    End If

    dwSize = GetFileVersionInfoSizeW(FileName, @dwDummy)
    If dwSize = 0 Then
        Return ""
    End If

    ' 复用静态缓冲区，不足则重新分配
    If verBufSize < dwSize Then
        If verBuf <> NULL Then Deallocate(verBuf)
        verBuf = Allocate(dwSize)
        verBufSize = dwSize
    End If

    If GetFileVersionInfoW(FileName, 0, dwSize, verBuf) = 0 Then
        MyLog.PrintWin32Error "GetFileCompany_Opt", "GetFileVersionInfoW", "Path=" & FileName
        Return "-"
    End If

    If VerQueryValueW(verBuf, StrPtrW("\VarFileInfo\Translation"), @pTransTable, @nQuerySize) = 0 Then
        MyLog.PrintWin32Error "GetFileCompany_Opt", "VerQueryValueW Translation", "Path=" & FileName
        Return ""
    End If

    ' 仅取第一条语言信息，绝大多数场景够用，省去循环
    langCharset = (pTransTable[0].wLanguage Shl 16) Or pTransTable[0].wCodePage
    subBlock = "\StringFileInfo\" & Hex(langCharset, 8) & "\CompanyName"

    If VerQueryValueW(verBuf, StrPtrW(subBlock), @lpBuffer, @dwLen) Then
        If dwLen >= 2 Then
            CopyMemory(StrPtr(lpData), lpBuffer, dwLen * 2)
            Return lpData
        End If
    End If

    MyLog.PrintLog LOG_INFO,,,"GetFileCompany_Opt 未找到CompanyName, Path=" & FileName
    Return ""
End Function

' 文件公司名缓存包装函数：同路径仅读取一次
Function GetFileCompanyCached(ByVal FileName As StringW) As StringW
    If FileName = "" Then Return "-"
    
    ' 静态缓存
    Static cachePath() As StringW
    Static cacheCompany() As StringW
    Static cacheCount As Integer = 0
    
    ' 查找缓存命中
    For i As Integer = 0 To cacheCount - 1
        If cachePath(i) = FileName Then
            Return cacheCompany(i)
        End If
    Next
    
    ' 未命中：调用原函数读取
    Dim result As StringW = GetFileCompany_Opt(FileName)
    
    ' 写入缓存（自动扩容）
    If cacheCount = 0 Then
        ReDim cachePath(0 To 63)
        ReDim cacheCompany(0 To 63)
    ElseIf cacheCount > UBound(cachePath) Then
        ReDim Preserve cachePath(0 To cacheCount * 2 - 1)
        ReDim Preserve cacheCompany(0 To cacheCount * 2 - 1)
    End If
    cachePath(cacheCount) = FileName
    cacheCompany(cacheCount) = result
    cacheCount += 1
    
    Return result
End Function

' 辅助函数：从指定文件提取签名证书并判断是否为微软
Private Function IsMicrosoftSignatureFromFile(ByVal filePath As LPCWSTR) As Boolean
    Dim hStore As Any Ptr = NULL
    Dim pMsg As Any Ptr = NULL
    Dim dwEncoding As DWORD = 0
    Dim dwContentType As DWORD = 0
    Dim dwFormatType As DWORD = 0
    Dim result As Boolean = False
    Dim dwError As DWORD
    
    'Print "===== 调试：检查微软签名 ====="
    'Print "文件路径：" & *filePath
    
    ' 开启内嵌签名验证
    If CryptQueryObject(CERT_QUERY_OBJECT_FILE, filePath, _
                        CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, _
                        CERT_QUERY_FORMAT_FLAG_BINARY, _
                        0, _
                        @dwEncoding, @dwContentType, @dwFormatType, _
                        @hStore, @pMsg, ByVal 0) Then
        'Print "√ CryptQueryObject 成功"
        
        If pMsg = NULL Then
            'Print "× pMsg 为 NULL，无法获取签名信息"
            Goto Cleanup
        End If
        
        Dim cbSize As DWORD = 0
        If CryptMsgGetParam(pMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, @cbSize) Then
            'Print "√ CryptMsgGetParam (获取大小) 成功，大小：" & cbSize
            Dim pbSignerInfo As Byte Ptr = Allocate(cbSize)
            If pbSignerInfo Then
                If CryptMsgGetParam(pMsg, CMSG_SIGNER_INFO_PARAM, 0, pbSignerInfo, @cbSize) Then
                    'Print "√ CryptMsgGetParam (获取数据) 成功"
                    Dim pSignerInfo As PCMSG_SIGNER_INFO = Cast(PCMSG_SIGNER_INFO, pbSignerInfo)
                    
                    ' 核心修正：声明 CERT_INFO 结构体并填充 Issuer 和 SerialNumber
                    Dim certInfo As CERT_INFO
                    certInfo.Issuer = pSignerInfo->Issuer
                    certInfo.SerialNumber = pSignerInfo->SerialNumber
                    
                    ' 传递 certInfo 的指针，而非 pSignerInfo
                    Dim certContext As PCCERT_CONTEXT = CertFindCertificateInStore( _
                        hStore, X509_ASN_ENCODING Or PKCS_7_ASN_ENCODING, _
                        0, CERT_FIND_SUBJECT_CERT, @certInfo, NULL)
                    
                    If certContext Then
                        'Print "√ 找到证书上下文"
                        Dim subject As WString * 1024
                        Dim nStr As DWORD = CertNameToStrW(certContext->dwCertEncodingType, @certContext->pCertInfo->subject, CERT_SIMPLE_NAME_STR, subject, 1024)
                        'Print "证书主题：" & WStr(subject)
                        
                        If InStrW(subject, "MICROSOFT") > 0 Or InStrW(subject, "Microsoft") > 0 Then
                            result = True
                            'Print "√ 匹配到微软签名!"
                        Else
                            'Print "× 未匹配到 MICROSOFT 关键词"
                        End If
                        CertFreeCertificateContext(certContext)
                    Else
                        'Print "× 未找到证书，错误码：" & GetLastError()
                    End If
                Else
                    'Print "× CryptMsgGetParam (数据) 失败，错误码：" & GetLastError()
                End If
                Deallocate pbSignerInfo
            End If
        Else
            'Print "× CryptMsgGetParam (大小) 失败，错误码：" & GetLastError()
        End If
    Else
        'Print "× CryptQueryObject 失败，错误码：" & GetLastError()
    End If
    
Cleanup:
    If pMsg Then CryptMsgClose(pMsg)
    If hStore Then CertCloseStore(hStore, 0)
    'Print "===== 最终结果：" & result & " ======" & Chr(10,13)
    Return result
End Function

Function VerifyFileSign(ByVal pwszFilePath As LPCWSTR, ByVal pIsMicrosoftSign As Boolean Ptr = NULL) As Long
    Dim hCatAdmin As Any Ptr = NULL
    Dim hCatInfo As Any Ptr = NULL
    Dim hFile As HANDLE = INVALID_HANDLE_VALUE
    Dim wvtGUID As GUID = Type < GUID > (&H00AAC56B, &HCD44, &H11D0, {&H8C, &HC2, &H00, &HC0, &H4F, &HC2, &H95, &HEE}) 
    
    Dim rgbHash(0 To 99) As Byte ' 预留100字节的哈希缓冲区
    Dim cbHash As DWORD = SizeOf(Byte) * 100
    
    Dim catalogInfo As CATALOG_INFO
    
    Dim szMemberTag As WString * MAX_PATH
    
    Dim RevocationChecks As Long = WTD_REVOKE_WHOLECHAIN ' , WTD_REVOKE_WHOLECHAIN}
    'Dim StateAction(3) As Long = {WTD_STATEACTION_IGNORE, WTD_STATEACTION_VERIFY, WTD_STATEACTION_AUTO_CACHE, WTD_STATEACTION_AUTO_CACHE_FLUSH}
    Dim StateAction As Long = WTD_STATEACTION_IGNORE
    'Dim ProvFlags(9) As Long = {16, 32, 64, 128, 512, 1024, 2048, 4096, 8192}
    Dim ProvFlags As Long = 16 + 1024 ' , 16 + 1024} /'似乎存在某种预热机制,
    '每次改变dwRevocationChecks,StateAction或被验证文件路径都需要重新"预热","预热"时的参数可以与正式验证时的参数一致'/
    'Dim ProvFlags As Long = 16 + 1024
    Dim count As Integer = 0
    Dim buffer As String
    Dim Status As Long
    
    ' 1. 获取目录管理员上下文
    If CryptCATAdminAcquireContext(@hCatAdmin, NULL, 0) = False Then
        MyLog.PrintWin32Error "VerifyFileSign", "CryptCATAdminAcquireContext"
        Return False
    End If
    
    ' 2. 打开文件并计算哈希
    hFile = CreateFileW(pwszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
    If hFile = INVALID_HANDLE_VALUE Then
        MyLog.PrintWin32Error "VerifyFileSign", "CreateFile", "pwszFilePath=" & *pwszFilePath
        Goto cleanup
    End If
    
    If CryptCATAdminCalcHashFromFileHandle(hFile, @cbHash, @rgbHash(0), 0) = False Then
        MyLog.PrintWin32Error "VerifyFileSign", "CryptCATAdminCalcHashFromFileHandle", "pwszFilePath=" & *pwszFilePath
        Goto cleanup
    End If
    
    ' 分配字节缓冲区，最后转换
    buffer = Space(cbHash * 2)  ' 分配ANSI字符串空间
    For dw As DWORD = 0 To cbHash - 1
        Mid(buffer, dw * 2 + 1, 2) = Hex(rgbHash(dw), 2)
    Next
    szMemberTag = WStr(buffer)  ' 一次性转换为宽字符串
    
    CloseHandle(hFile)
    hFile = INVALID_HANDLE_VALUE
    
    ' 3. 根据哈希查找目录
    hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, @rgbHash(0), cbHash, 0, NULL)
    If hCatInfo = NULL Then
        'MyLog.PrintInfo "VerifyFileSign", "CryptCATAdminEnumCatalogFromHash", "szMemberTag=" & szMemberTag
        Goto CheckFileSign
    End If
    
    ' 4. 获取目录文件信息
    catalogInfo.cbStruct = SizeOf(catalogInfo)
    If CryptCATCatalogInfoFromContext(hCatInfo, @catalogInfo, 0) Then
        Dim wti As WINTRUST_CATALOG_INFO = Type<WINTRUST_CATALOG_INFO>( _
            SizeOf(WINTRUST_CATALOG_INFO), _ ' cbStruct
            0, _                              ' dwCatalogVersion
            @catalogInfo.wszCatalogFile, _ ' pcwszCatalogFilePath
            @szMemberTag, _                   ' pcwszMemberTag
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
        wtd.dwProvFlags = ProvFlags
        
        ' 5. 调用WinVerifyTrust验证目录签名
        Dim Status As Long = WinVerifyTrust(NULL, @wvtGUID, @wtd)
        ' 关闭验证状态
        wtd.dwStateAction = WTD_STATEACTION_CLOSE
        WinVerifyTrust(NULL, @wvtGUID, @wtd)
        
        ' 成功时判断是否微软签名
        If Status = ERROR_SUCCESS Then
            If pIsMicrosoftSign Then *pIsMicrosoftSign = IsMicrosoftSignatureFromFile(catalogInfo.wszCatalogFile)
        End If
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
        Status = WinVerifyTrust(NULL, @wvtGUID, @wtd)
        ' 关闭验证状态
        wtd.dwStateAction = WTD_STATEACTION_CLOSE
        WinVerifyTrust(NULL, @wvtGUID, @wtd)
        
        ' 成功时判断是否微软签名
        If Status = ERROR_SUCCESS Then
            If pIsMicrosoftSign Then *pIsMicrosoftSign = IsMicrosoftSignatureFromFile(pwszFilePath)
        End If
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
    
    Return Status
End Function

' ===============================
' 从队列取任务（线程安全）
' ===============================
Function PopTask() As AsyncTaskParam Ptr
    Dim task As AsyncTaskParam Ptr = 0
    WaitForSingleObject QueueMutex, INFINITE
    
    If UBound(TaskQueue) >= 0 Then
        task = TaskQueue(0)
        Dim n As Integer = UBound(TaskQueue)
        If n > 0 Then
            For i As Integer = 0 To n - 1
                TaskQueue(i) = TaskQueue(i + 1)
            Next
            ReDim Preserve TaskQueue(n - 1)
        Else
            ReDim TaskQueue(-1)
        End If
    End If

    ReleaseMutex QueueMutex
    Return task
End Function

' ===============================
' 提交任务
' ===============================
Sub EnqueueTask(task As AsyncTaskParam Ptr)
    WaitForSingleObject QueueMutex, INFINITE
    Dim n As Integer = UBound(TaskQueue)
    ReDim Preserve TaskQueue(n + 1)
    TaskQueue(n + 1) = task
    ReleaseMutex QueueMutex

    ' 唤醒等待线程
    SetEvent TaskEvent
End Sub

' ===============================
' 工作线程
' ===============================
Sub WorkerThread(ByVal lpParam As LPVOID)
    Dim task As AsyncTaskParam Ptr

    Do While True
        ' 从队列取任务
        task = PopTask()
        If task = 0 Then
            ' 队列空时等待事件
            WaitForSingleObject(TaskEvent, INFINITE)
            ' 重置事件（自动重置或手动 ResetEvent，取决于 CreateEvent 参数）
            Continue Do
        End If

        ' 执行签名验证
        ' 初始化：默认前景色=黑色（所有状态统一）
        Dim ForeColor As COLORREF = FB_Black
        Dim BackColor As COLORREF = FB_White ' 默认正常色
        
        ' 执行签名验证
        If task->filePath <> "" AndAlso FileExists(Str(task->filePath)) <> 0 Then
            Dim verifyResult As Long, IsMicrosoftSign As Boolean
            'Print task->filePath
            verifyResult = VerifyFileSign(@task->filePath, @IsMicrosoftSign)
            ' 2. 根据验证结果 → 匹配你的5种状态 + 对应颜色
            Select Case verifyResult
                ' ==============================================
                ' 状态1：可信 → 白色（微软/知名厂商有效签名）
                ' ==============================================
                Case ERROR_SUCCESS
                    If IsMicrosoftSign Then
                        'MyLog.PrintLog LOG_INFO,,, task->filePath
                        BackColor = FB_White ' FB_LightGreen
                    Else
                        ' ==============================================
                        ' 状态2：正常 → 浅蓝（普通第三方有效签名）
                        ' ==============================================
                        BackColor = FB_AliceBlue
                    End If

                ' ==============================================
                ' 状态3：警告 → 金黄（无签名）
                ' ==============================================
                Case TRUST_E_NOSIGNATURE
                    BackColor = FB_GoldenYellow

                ' ==============================================
                ' 状态4：可疑 → 浅红（签名无效/过期/吊销）
                ' ==============================================
                Case TRUST_E_BAD_SIGNATURE, TRUST_E_EXPIRED, TRUST_E_CERT_SUSPENDED
                    BackColor = FB_LightRed

                ' ==============================================
                ' 状态5：危险 → 纯红（确认恶意/伪造签名）
                ' ==============================================
                Case Else
                    ' 其他所有异常 → 判定为危险
                    'Print "状态:0x" & WHex(verifyResult) & "(" & verifyResult & ")" & " TRUST_E_EXPIRED:0x" & WHex(TRUST_E_EXPIRED) & "(" & TRUST_E_EXPIRED & ")"
                    BackColor = FB_Grey
            End Select
            
            Dim msg As SIGNVERIFY_MSG Ptr = Callocate(SizeOf(SIGNVERIFY_MSG))
            If msg <> 0 Then
                msg->ctrlType = task->ctrlType
                msg->rowIndex = task->rowIndex
                msg->ForeColor = ForeColor
                msg->BackColor = BackColor
                
                Dim ctx As ListViewContext Ptr = Cast(ListViewContext Ptr, GetWindowLongPtr(task->hWndControl, GWLP_USERDATA))
                If ctx <> NULL Then
                    msg->hListView = task->hWndControl
                    msg->InstanceId = ctx->InstanceId
                    PostMessage task->hWndForm, WM_SignVerify, 0, Cast(lParam, msg)
                End If
            End If
        End If

        Deallocate task
    Loop
End Sub

' ===============================
' 初始化线程池
' ===============================
Sub InitThreadPool()
    QueueMutex = CreateMutex(NULL, FALSE, NULL)
    TaskEvent = CreateEvent(NULL, FALSE, FALSE, NULL) ' 自动重置事件

    ReDim Threads(ThreadCount - 1)
    For i As Integer = 0 To ThreadCount - 1
        Threads(i) = ThreadCreate(@WorkerThread, NULL, 0)
        Threaddetach Threads(i)
    Next
End Sub

' ===============================
' 启动异步验证
' ===============================
Sub VerifyFileSignAsync(hWndForm As HWND, hWndControl As HWND, ctrlType As StringW, rowIndex As Long, filePath As StringW)
    Dim task As AsyncTaskParam Ptr = Callocate(1, SizeOf(AsyncTaskParam))
    If task = 0 Then Exit Sub
    task->hWndForm = hWndForm
    task->hWndControl = hWndControl
    task->ctrlType = ctrlType
    task->rowIndex = rowIndex
    task->filePath = filePath
    EnqueueTask(task)
End Sub

Function ForceCopyFolder(SourcePath As StringW, TargetPath As StringW) As Boolean
    Dim pCopyPath As COPY_PATH Ptr = Allocate(SizeOf(COPY_PATH))
    StringCbCopyW @pCopyPath->SourcePath, MAX_PATH, SourcePath.WstrPtr
    StringCbCopyW @pCopyPath->TargetPath, MAX_PATH, TargetPath.WstrPtr
    pCopyPath->IsDirectory = True
    If IoControl(hDrv, IOCTL_CopyFolder, pCopyPath, SizeOf(COPY_PATH)) = 0 Then
        MyLog.PrintWin32Error "ForceCopyFolder", "IOCTL_CopyFolder", "SourcePath=" & SourcePath & ",TargetPath=" & TargetPath
        AfxMsg "强制复制失败!"
        Function = False
    Else
        AfxMsg "强制复制成功!"
        Function = True
    End If
    Deallocate pCopyPath
End Function