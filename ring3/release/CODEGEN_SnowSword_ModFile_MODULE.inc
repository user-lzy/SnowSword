'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModFile.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


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

Public Sub GetFileList(CurrentNode As HTREEITEM,treFolder As Class_TreeView,lvwFile As Class_ListView)
    Dim szPath As StringW = ""
    GetFilePathByNodeW CurrentNode, treFolder, szPath
    szPath = szPath & "*.*"
    Dim fData As WIN32_FIND_DATAW
    Dim hFind As HANDLE = FindFirstFileW(szPath,@fData)'不能使用@szPath,否则报"找不到文件"
    If hFind = INVALID_HANDLE_VALUE Then
        PrintA WinErrorMsg(GetLastError)
        Exit Sub
    End If
    Do
        If fData.dwFileAttributes And fbDirectory Then
            treFolder.InsertItem CurrentNode,TVI_LAST,fData.cFileName
        Else
            lvwFile.AddItem fData.cFileName
            lvwFile.SetItemText lvwFile.ItemCount - 1,1,WStr(fData.nFileSizeHigh*(2^32) + fData.nFileSizeLow)
        End If
    Loop While FindNextFileW(hFind,@fData) <> 0
    FindClose hFind
End Sub
