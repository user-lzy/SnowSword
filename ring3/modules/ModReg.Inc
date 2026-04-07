#include Once "win/winreg.bi"

Type REG_VALUE_ITEM
    ValueName As String
    ValueType As DWORD
    ValueData As String
End Type

Type REG_KEY_ITEM
    KeyPath As String          ' 完整路径，UI显示用（保留原始大小写）
    KeyPathUpper As String     ' 新增：标准化大写路径，用于内部比较
    SubKeys(Any) As String
    Values(Any) As REG_VALUE_ITEM
    IsSymLink As Boolean
    SymLinkTarget As String
    IsFromHKCU As Boolean      ' 新增：标记是否来自HKCU（用于HKCR合并优先级）
End Type

Declare Function RegDeleteTree Lib "Advapi32.dll" Alias "RegDeleteTreeW"(ByVal HKEY As HKEY, ByVal lpSubKey As LPCWSTR) As Long
Declare Function ConvertSidToStringSid Lib "advapi32.dll" Alias "ConvertSidToStringSidA" (ByVal SID As PSID, ByRef StringSid As LPSTR) As BOOL

Type QWORD As ULongInt

Const SOFTWARE_CLASSES_PATH = "\SOFTWARE\Classes"
Const SOFTWARE_CLASSES_LEN = Len(SOFTWARE_CLASSES_PATH)  ' 自动计算为16

Private Function GetHKEYFromStr(ByVal strHKEY As String) As HKEY
    Select Case strHKEY
        Case "HKEY_CLASSES_ROOT"
            Return HKEY_CLASSES_ROOT
        Case "HKEY_LOCAL_MACHINE"
            Return HKEY_LOCAL_MACHINE
        Case "HKEY_USERS"
            Return HKEY_USERS
        Case "HKEY_CURRENT_USER"
            Return HKEY_CURRENT_USER
        Case "HKEY_CURRENT_CONFIG"
            Return HKEY_CURRENT_CONFIG
    End Select
End Function

Private Function GetHKEYString(ByVal hKey As HKEY) As String
    Select Case hKey
        Case HKEY_CLASSES_ROOT:    Return "HKEY_CLASSES_ROOT"
        Case HKEY_LOCAL_MACHINE:     Return "HKEY_LOCAL_MACHINE"
        Case HKEY_USERS:             Return "HKEY_USERS"
        Case HKEY_CURRENT_USER:      Return "HKEY_CURRENT_USER"
        Case HKEY_CURRENT_CONFIG:    Return "HKEY_CURRENT_CONFIG"
        Case Else:                   Return "HKEY_UNKNOWN"
    End Select
End Function

Function GetRegPathByNodeW(Node As HTREEITEM, treKey As Class_TreeView, ByRef RegPath As StringW) As HKEY
    Dim CurrentNode As HTREEITEM = Node
    Do While True
        If (treKey.GetParent(CurrentNode) = NULL) Then
            Function = GetHKEYFromStr(treKey.Text(CurrentNode))
            Exit Do
        End If
        RegPath = treKey.Text(CurrentNode) & "\" & RegPath
        Print "treKey.Text(CurrentNode):" & treKey.Text(CurrentNode)
        Print "RegPath:" & RegPath
        CurrentNode = treKey.GetParent(CurrentNode)
    Loop
End Function

' 新增：获取完整路径字符串（用于显示/比较）
Function GetFullPathByNode(Node As HTREEITEM, treKey As Class_TreeView) As String
    Dim CurrentNode As HTREEITEM = Node
    Dim pathParts() As String
    Dim partCount As Integer = 0
    
    ' 收集所有节点（包括根节点）
    Do While CurrentNode <> NULL
        Dim parentNode As HTREEITEM = treKey.GetParent(CurrentNode)
        
        ReDim Preserve pathParts(partCount)
        pathParts(partCount) = treKey.Text(CurrentNode)
        partCount += 1
        
        CurrentNode = parentNode
    Loop
    
    ' 反向拼接（从根到叶子）
    Dim result As String = ""
    For i As Integer = partCount - 1 To 0 Step -1
        result &= pathParts(i)
        If i > 0 Then result &= "\"
    Next
    
    Return result
End Function

Public Sub GetRootList(treKey As Class_TreeView)
    treKey.AddItem TVI_ROOT, "HKEY_CLASSES_ROOT"
    treKey.AddItem TVI_ROOT, "HKEY_CURRENT_USER"
    treKey.AddItem TVI_ROOT, "HKEY_LOCAL_MACHINE"
    treKey.AddItem TVI_ROOT, "HKEY_USERS"
    treKey.AddItem TVI_ROOT, "HKEY_CURRENT_CONFIG"
End Sub

' ########## 注册表值类型转可读名称 ##########
Private Function GetRegTypeName(ByVal dataType As DWORD) As String
    Select Case dataType
        Case REG_NONE:                Return "REG_NONE"                ' 无类型
        Case REG_SZ:                  Return "REG_SZ"                  ' 字符串
        Case REG_EXPAND_SZ:           Return "REG_EXPAND_SZ"           ' 可扩展字符串（含%变量%）
        Case REG_BINARY:              Return "REG_BINARY"              ' 二进制数据
        Case REG_DWORD:               Return "REG_DWORD"               ' 32位无符号整数（小端序）
        Case REG_DWORD_BIG_ENDIAN:    Return "REG_DWORD_BIG_ENDIAN"    ' 32位无符号整数（大端序）
        Case REG_LINK:                Return "REG_LINK"                ' 符号链接
        Case REG_MULTI_SZ:            Return "REG_MULTI_SZ"            ' 多字符串（以两个空字符结尾）
        Case REG_RESOURCE_LIST:       Return "REG_RESOURCE_LIST"       ' 硬件资源列表
        Case REG_FULL_RESOURCE_DESCRIPTOR: Return "REG_FULL_RESOURCE_DESCRIPTOR" ' 硬件资源描述符
        Case REG_RESOURCE_REQUIREMENTS_LIST: Return "REG_RESOURCE_REQUIREMENTS_LIST" ' 硬件资源需求
        Case REG_QWORD:               Return "REG_QWORD"               ' 64位无符号整数
        Case Else:                    Return "未知类型 (" & Str(dataType) & ")" ' 兜底：显示原始数值
    End Select
End Function

Private Function ProcessValueData(dataType As DWORD, dataBuffer() As Byte, dataSize As DWORD) As String
    If dataSize = 0 Then Return "(零长度数据)"
    
    Select Case dataType
        ' ===== REG_SZ 和 REG_EXPAND_SZ 使用动态 WString =====
        Case REG_SZ, REG_EXPAND_SZ
            ' 分配 WString 缓冲区（宽字符数 = dataSize \ 2 + 1）
            Dim wsBuffer As WString Ptr
            Dim charCount As Integer = (dataSize + 1) \ 2  ' +1确保能容纳终止符
            wsBuffer = CAllocate(charCount, SizeOf(WString))
            
            If wsBuffer <> NULL Then
                ' 安全拷贝数据
                memcpy(wsBuffer, @dataBuffer(0), dataSize)
                
                ' 确保 null 终止（防止注册表数据损坏）
                wsBuffer[charCount - 1] = 0
                
                ' 转换为 FreeBasic StringW 并添加到 ListView
                Dim value As StringW = *wsBuffer
                ' 释放临时缓冲区
                Deallocate(wsBuffer)
                Return Value
            Else
                Return "(Error)"
            End If
            
        Case REG_BINARY
            Dim binaryData As String = ""
            For i As Integer = 0 To dataSize - 1
                binaryData &= Hex(dataBuffer(i), 2) & " "
            Next
            Return RTrim(binaryData)
            
        Case REG_DWORD
            Dim dwordData As ULong
            memcpy(@dwordData, @dataBuffer(0), Min(dataSize, SizeOf(DWORD)))
            Return Str(dwordData)
            
        Case REG_QWORD
            Dim qwordData As ULongInt
            memcpy(@qwordData, @dataBuffer(0), Min(dataSize, SizeOf(QWORD)))
            Return Str(qwordData)
            
        Case REG_MULTI_SZ
            Dim result As String = ""
            Dim p As WString Ptr = CPtr(WString Ptr, @dataBuffer(0))
            Dim totalSize As Integer = dataSize \ 2 ' 字符数
            
            Do While totalSize > 0 AndAlso p[0] <> 0
                result &= *p & ", "
                p += LenW(*p) + 1
                totalSize -= LenW(*p) + 1
            Loop
            
            If Len(result) > 0 Then result = Left(result, Len(result) - 2)
            Return result
            
        Case Else
            Return "(未知类型: " & Str(dataType) & ")"
    End Select
End Function

' ########## 新增：安全的DWORD值解析函数 ##########
Private Function ParseDWORDValue(valueData As String, valueType As DWORD) As DWORD
    Dim result As DWORD = 1  ' 默认值
    
    Select Case valueType
        Case REG_DWORD
            ' 如果是4字节二进制数据，直接转换
            If Len(valueData) = 4 Then
                ' 小端序转换
                result = CUInt(Asc(valueData, 1)) Or _
                        (CUInt(Asc(valueData, 2)) Shl 8) Or _
                        (CUInt(Asc(valueData, 3)) Shl 16) Or _
                        (CUInt(Asc(valueData, 4)) Shl 24)
            Else
                ' 字符串格式，安全转换
                Dim tempVal As LongInt = ValInt(valueData)
                If tempVal > 0 And tempVal <= 999 Then  ' ControlSet编号范围1-999
                    result = CUInt(tempVal)
                End If
            End If
            
        Case REG_DWORD_BIG_ENDIAN
            ' 大端序处理（罕见但可能）
            If Len(valueData) = 4 Then
                result = (CUInt(Asc(valueData, 1)) Shl 24) Or _
                        (CUInt(Asc(valueData, 2)) Shl 16) Or _
                        (CUInt(Asc(valueData, 3)) Shl 8) Or _
                        CUInt(Asc(valueData, 4))
            End If
            
        Case Else
            ' 尝试字符串解析
            Dim tempVal As LongInt = ValInt(valueData)
            If tempVal > 0 And tempVal <= 999 Then
                result = CUInt(tempVal)
            End If
    End Select
    
    ' 有效性检查：必须是1、2或3（Windows通常只有3个ControlSet）
    If result < 1 Or result > 3 Then
        result = 1  ' 回退到默认值1
    End If
    
    Return result
End Function

Function GetRegList1( _
    Node As HTREEITEM, _
    treKey As Class_TreeView, _
    ByRef outResult As REG_KEY_ITEM _
) As Boolean

    Dim result As REG_KEY_ITEM
    Dim KeyHandle As HKEY
    Dim lpSubKey As StringW
    Dim MainHKEY As HKEY = GetRegPathByNodeW(Node, treKey, lpSubKey)

    If lpSubKey = "" Then
        result.KeyPath = GetHKEYString(MainHKEY)
    Else
        result.KeyPath = GetHKEYString(MainHKEY) & "\" & lpSubKey
    End If
    'Print "result.KeyPath:" & result.KeyPath
    result.KeyPathUpper = PathToUpper(result.KeyPath) ' 同时初始化Upper版本
    'Print "result.KeyPathUpper:" & result.KeyPathUpper

    If RegOpenKeyEx(MainHKEY, lpSubKey, 0, KEY_READ, @KeyHandle) <> ERROR_SUCCESS Then Return False
    
    ' ---------- 子键 ----------
    Dim idx As DWORD = 0
    Dim keyName As WString * MAX_PATH
    Do
        If RegEnumKey(KeyHandle, idx, @keyName, MAX_PATH) <> ERROR_SUCCESS Then Exit Do
        ReDim Preserve result.SubKeys(idx)
        result.SubKeys(idx) = keyName
        idx += 1
    Loop

    ' ---------- 值 ----------
    idx = 0
    Do
        Dim valName As WString * MAX_PATH
        Dim valLen As DWORD = MAX_PATH
        Dim dataType As DWORD
        Dim dataSize As DWORD

        If RegEnumValue(KeyHandle, idx, @valName, @valLen, 0, @dataType, NULL, @dataSize) <> ERROR_SUCCESS Then Exit Do
        'Print "valName:" & valName
        Dim buf() As Byte
        If dataSize > 0 Then
            ReDim buf(dataSize - 1)
            RegQueryValueEx(KeyHandle, valName, 0, @dataType, @buf(0), @dataSize)
        End If

        ReDim Preserve result.Values(idx)
        result.Values(idx).ValueName = valName
        result.Values(idx).ValueType = dataType
        result.Values(idx).ValueData = ProcessValueData(dataType, buf(), dataSize)

        idx += 1
    Loop

    RegCloseKey(KeyHandle)
    outResult = result
    Return True
End Function

' ########## 辅助函数：路径标准化（核心修复4：大小写不敏感 + 移除末尾反斜杠） ##########
Function PathToUpper(path As String) As String
    Dim s As String = UCase(path)
    
    ' 步骤1：将 / 统一转为 \
    Do
        Dim pos1 As Integer = InStr(s, "/")
        If pos1 = 0 Then Exit Do
        Mid(s, pos1, 1) = "\"
    Loop
    
    ' 步骤2：循环移除末尾的所有反斜杠（核心修复）
    Do While Len(s) > 0 And Right(s, 1) = "\"
        s = Left(s, Len(s) - 1)
    Loop
    
    ' 步骤3：处理空路径（防御性编程）
    If s = "" Then s = "\"
    
    Return s
End Function

' ########## 辅助函数：大小写不敏感的键查找（核心修复4） ##########
Function FindKeyIndex(arr() As REG_KEY_ITEM, keyPath As String) As Integer
    Dim keyPathUpper As String = PathToUpper(keyPath)
    For i As Integer = LBound(arr) To UBound(arr)
        If arr(i).KeyPathUpper = keyPathUpper Then
            Return i
        End If
    Next
    Return -1
End Function

' ########## 辅助函数：大小写不敏感的值查找（核心修复4） ##########
Function FindValueName(arr() As REG_VALUE_ITEM, valueName As String) As Boolean
    If UBound(arr) < LBound(arr) Then Return False
    Dim valueNameUpper As String = UCase(valueName)
    For i As Integer = LBound(arr) To UBound(arr)
        If UCase(arr(i).ValueName) = valueNameUpper Then
            Return True
        End If
    Next
    Return False
End Function

' ########## 辅助函数：获取当前用户SID ##########
Function GetCurrentUserSid() As String
    Dim hToken As HANDLE = NULL
    Dim pTokenUser As PTOKEN_USER = NULL
    Dim dwReturnLen As DWORD = 0
    Dim szSid As LPSTR = NULL
    Dim sResult As String = ""

    If OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, hToken) = 0 Then GoTo Cleanup
    GetTokenInformation(hToken, TokenUser, NULL, 0, @dwReturnLen)
    If dwReturnLen = 0 Then GoTo Cleanup

    pTokenUser = CAllocate(dwReturnLen)
    If GetTokenInformation(hToken, TokenUser, pTokenUser, dwReturnLen, @dwReturnLen) = 0 Then Goto Cleanup
    If ConvertSidToStringSid(pTokenUser->User.SID, szSid) = 0 Then Goto Cleanup

    sResult = *szSid

Cleanup:
    If szSid <> NULL Then LocalFree(Cast(HLOCAL, szSid))
    If pTokenUser <> NULL Then Deallocate(pTokenUser)
    If hToken <> NULL Then CloseHandle(hToken)
    Return sResult
End Function

Private Function TreeViewFindChild(treKey As Class_TreeView, hParent As HTREEITEM, childText As String) As HTREEITEM
    If hParent = NULL Then Return NULL
    
    Dim hChild As HTREEITEM = treKey.GetChild(hParent)
    Dim targetTextUpper As String = UCase(childText)
    
    While hChild <> NULL
        ' 假设Class_TreeView有获取项目文本的方法（如果没有，请替换为你实际的GetItemText方法）
        Dim itemText As String = treKey.Text(hChild)
        If UCase(itemText) = targetTextUpper Then
            Return hChild
        End If
        hChild = treKey.GetNextSiblin(hChild) ' 注意：严格使用你提供的接口名（GetNextSiblin）
    Wend
    
    Return NULL
End Function

' ########## 辅助函数：生成分层TreeView节点（和regedit一致） ##########
Sub AddPathToTreeView(treKey As Class_TreeView, fullPath As String)
    If fullPath = "" Then Exit Sub
    Dim pathParts() As String
    Dim partCount As Integer = 0
    Dim tempPath As String = fullPath
    Dim hParent As HTREEITEM = TVI_ROOT
    Dim hCurrent As HTREEITEM = NULL

    ' 按反斜杠拆分路径（原有逻辑保留）
    Do
        Dim pos1 As Integer = InStr(tempPath, "\")
        If pos1 = 0 Then
            If tempPath <> "" Then
                ReDim Preserve pathParts(0 To partCount)
                pathParts(partCount) = tempPath
                partCount += 1
            End If
            Exit Do
        End If
        ReDim Preserve pathParts(0 To partCount)
        pathParts(partCount) = Left(tempPath, pos1 - 1)
        partCount += 1
        tempPath = Mid(tempPath, pos1 + 1)
    Loop

    ' 逐层创建节点（适配新接口）
    For i As Integer = 0 To partCount - 1
        Dim partName As String = pathParts(i)
        ' 用自定义函数查找子项
        hCurrent = TreeViewFindChild(treKey, hParent, partName)
        If hCurrent = NULL Then
            ' 修正AddItem参数顺序：(hParent, TheText, ...)
            hCurrent = treKey.AddItem(hParent, WStr(partName))
        End If
        hParent = hCurrent
    Next
End Sub

' ########## 辅助函数：深度复制键项（用于符号链接展开） ##########
Function DeepCopyKeyItem(src As REG_KEY_ITEM, newRootPath As String, oldRootPath As String) As REG_KEY_ITEM
    Dim dst As REG_KEY_ITEM
    Dim oldRootUpper As String = PathToUpper(oldRootPath)
    Dim srcPathUpper As String = src.KeyPathUpper
    
    ' 路径替换：将 oldRootPath 开头的路径替换为 newRootPath
    If Left(srcPathUpper, Len(oldRootUpper)) = oldRootUpper Then
        dst.KeyPath = newRootPath & Mid(src.KeyPath, Len(oldRootPath) + 1)
    Else
        dst.KeyPath = src.KeyPath
    End If
    
    dst.KeyPathUpper = PathToUpper(dst.KeyPath)
    dst.IsSymLink = src.IsSymLink
    dst.SymLinkTarget = src.SymLinkTarget
    dst.IsFromHKCU = src.IsFromHKCU
    
    ' 复制值数组
    If UBound(src.Values) >= LBound(src.Values) Then
        ReDim dst.Values(LBound(src.Values) To UBound(src.Values))
        For i As Integer = LBound(src.Values) To UBound(src.Values)
            dst.Values(i) = src.Values(i)
        Next
    End If
    
    ' 复制子键数组（可选，用于保留原始信息）
    If UBound(src.SubKeys) >= LBound(src.SubKeys) Then
        ReDim dst.SubKeys(LBound(src.SubKeys) To UBound(src.SubKeys))
        For i As Integer = LBound(src.SubKeys) To UBound(src.SubKeys)
            dst.SubKeys(i) = src.SubKeys(i)
        Next
    End If
    
    Return dst
End Function

Function IsPathCoveredByHKCU(pathUpper As String, hkcuPathSet() As String) As Boolean
    For i As Integer = LBound(hkcuPathSet) To UBound(hkcuPathSet)
        Dim coveredRoot As String = hkcuPathSet(i) & "\"
        If pathUpper = hkcuPathSet(i) Or Left(pathUpper, Len(coveredRoot)) = coveredRoot Then
            Return True
        End If
    Next
    Return False
End Function

' 在 TreeView 中查找完整路径对应的节点（如果不存在返回 NULL）
Function TreeViewFindPath(treKey As Class_TreeView, path As String) As HTREEITEM
    Dim parts() As String
    Dim partCount As Integer = 0
    
    ' 拆分路径
    Dim tempPath As String = path
    Do
        Dim pos1 As Integer = InStr(tempPath, "\")
        If pos1 = 0 Then
            If tempPath <> "" Then
                ReDim Preserve parts(partCount)
                parts(partCount) = tempPath
                partCount += 1
            End If
            Exit Do
        End If
        ReDim Preserve parts(partCount)
        parts(partCount) = Left(tempPath, pos1 - 1)
        partCount += 1
        tempPath = Mid(tempPath, pos1 + 1)
    Loop
    
    ' 逐层查找
    Dim hCurrent As HTREEITEM = TVI_ROOT
    For i As Integer = 0 To partCount - 1
        hCurrent = TreeViewFindChild(treKey, hCurrent, parts(i))
        If hCurrent = NULL Then Return NULL
    Next
    
    Return hCurrent
End Function

' 新增：对比API与Hive结果，检测隐藏项
Private Sub DetectHiddenItems(apiKey As REG_KEY_ITEM, hiveKey As REG_KEY_ITEM, hiddenSubKeys() As String, hiddenValues() As REG_VALUE_ITEM)
    Dim hiddenSubKeyCount As Integer = 0
    Dim hiddenValueCount As Integer = 0
    
    ' 检测隐藏子键：Hive存在、API不存在
    If UBound(hiveKey.SubKeys) >= 0 Then
        For i As Integer = 0 To UBound(hiveKey.SubKeys)
            Dim hiveSubKeyUpper As String = UCase(hiveKey.SubKeys(i))
            Dim found As Boolean = False
            If UBound(apiKey.SubKeys) >= 0 Then
                For j As Integer = 0 To UBound(apiKey.SubKeys)
                    If UCase(apiKey.SubKeys(j)) = hiveSubKeyUpper Then
                        found = True
                        Exit For
                    End If
                Next
            End If
            If Not found Then
                hiddenSubKeyCount += 1
                ReDim Preserve hiddenSubKeys(0 To hiddenSubKeyCount - 1)
                hiddenSubKeys(hiddenSubKeyCount - 1) = hiveKey.SubKeys(i)
                Print "[HIDDEN SUBKEY] " & hiveKey.KeyPath & "\" & hiveKey.SubKeys(i)
            End If
        Next
    End If
    
    ' 检测隐藏值：Hive存在、API不存在
    If UBound(hiveKey.Values) >= 0 Then
        For i As Integer = 0 To UBound(hiveKey.Values)
            Dim hiveValueUpper As String = UCase(hiveKey.Values(i).ValueName)
            Dim found As Boolean = False
            If UBound(apiKey.Values) >= 0 Then
                For j As Integer = 0 To UBound(apiKey.Values)
                    If UCase(apiKey.Values(j).ValueName) = hiveValueUpper Then
                        found = True
                        Exit For
                    End If
                Next
            End If
            If Not found Then
                hiddenValueCount += 1
                ReDim Preserve hiddenValues(0 To hiddenValueCount - 1)
                hiddenValues(hiddenValueCount - 1) = hiveKey.Values(i)
                Print "[HIDDEN VALUE] " & hiveKey.KeyPath & "\" & hiveKey.Values(i).ValueName
            End If
        Next
    End If
End Sub

Sub GetRegList(Node As HTREEITEM, treKey As Class_TreeView, lvwValue As Class_ListView, UseHive As Boolean)
    ' ===== Step1: 通用初始化 - 获取当前节点API枚举结果 =====
    Dim apiKeyItem As REG_KEY_ITEM
    Dim apiGetSuccess As Boolean = GetRegList1(Node, treKey, apiKeyItem)
    Dim currentFullPath As String
    Dim currentPathUpper As String
    Dim hiveInfo As ROOT_HIVE_INFO
    Dim hiveCacheItem As HIVE_CACHE_ITEM
    Dim relativePath As String = ""
    Dim hiveRootUpper As String = ""
    Dim targetNKOffset As ULong
    Dim hiveKeyItem As REG_KEY_ITEM
    Dim hiddenSubKeys() As String
    Dim hiddenValues() As REG_VALUE_ITEM
    Dim currentPathNorm As String
    
    ' 初始化值列表
    InitListViewData lvwValue
    
    ' ===== Step2: UseHive=False 原有逻辑保留，仅UI优化 =====
    If Not UseHive Then
        Print "=== DEBUG: UseHive=False ==="
        
        ' ✅ 仅清空当前节点的子节点，不全局清空TreeView
        If Node <> NULL Then
            Dim hChild As HTREEITEM = treKey.GetChild(Node)
            While hChild <> NULL
                Dim hNext As HTREEITEM = treKey.GetNextSiblin(hChild)
                treKey.DeleteItem hChild
                hChild = hNext
            Wend
            
            ' 仅添加当前节点的下一层子键
            If apiGetSuccess And UBound(apiKeyItem.SubKeys) >= LBound(apiKeyItem.SubKeys) Then
                For i As Integer = LBound(apiKeyItem.SubKeys) To UBound(apiKeyItem.SubKeys)
                    If TreeViewFindChild(treKey, Node, apiKeyItem.SubKeys(i)) = NULL Then
                        treKey.AddItem(Node, apiKeyItem.SubKeys(i))
                    End If
                Next
            End If
        End If
        
        ' 渲染值列表
        If apiGetSuccess And UBound(apiKeyItem.Values) >= LBound(apiKeyItem.Values) Then
            For j As Integer = LBound(apiKeyItem.Values) To UBound(apiKeyItem.Values)
                AddItemColListEx lvwValue, 3,,, WStr(apiKeyItem.Values(j).ValueName), WStr(GetRegTypeName(apiKeyItem.Values(j).ValueType)), WStr(apiKeyItem.Values(j).ValueData)
            Next
        End If
        
        Print "=== DEBUG: UseHive=False 结束 ==="
        CommitListViewView lvwValue
        Exit Sub
    End If
    
    ' ==================================================
    ' ===== Step3: UseHive=True 核心修复 - 隐藏项检测 =====
    ' ==================================================
    Print "=== DEBUG: UseHive=True - 隐藏项检测模式 ==="
    
    ' 基础校验
    If Node = NULL Or Not apiGetSuccess Then
        Print "DEBUG: 无法获取当前节点API信息，退出检测"
        Goto RenderFinalUI
    End If
    
    currentFullPath = apiKeyItem.KeyPath
    currentPathUpper = apiKeyItem.KeyPathUpper
    Print "DEBUG: 当前检测路径: " & currentFullPath
    
    ' 获取当前路径对应的hive信息
    If Not GetHiveInfoFromPath(currentFullPath, hiveInfo) Then
        Print "DEBUG: 不支持的路径，无对应hive文件"
        GoTo RenderFinalUI
    End If
    
    ' 按需加载hive
    If Not LoadHive(hiveInfo, hiveCacheItem) Then
        Print "DEBUG: hive加载失败，请检查管理员权限"
        GoTo RenderFinalUI
    End If
    
    ' ===== 修复：计算hive内相对路径 =====
    hiveRootUpper = PathToUpper(hiveInfo.RootName)
    currentPathNorm = currentPathUpper

    ' 步骤1：将当前路径的长键名替换为短键名，统一格式
    ' 例如：HKEY_LOCAL_MACHINE -> HKLM
    If Left(currentPathNorm, 18) = "HKEY_LOCAL_MACHINE" Then
        currentPathNorm = "HKLM" & Mid(currentPathNorm, 19)
    ElseIf Left(currentPathNorm, 17) = "HKEY_CURRENT_USER" Then
        currentPathNorm = "HKCU" & Mid(currentPathNorm, 18)
    ElseIf Left(currentPathNorm, 10) = "HKEY_USERS" Then
        currentPathNorm = "HKU" & Mid(currentPathNorm, 11)
    ElseIf Left(currentPathNorm, 17) = "HKEY_CLASSES_ROOT" Then
        currentPathNorm = "HKCR" & Mid(currentPathNorm, 18)
    ElseIf Left(currentPathNorm, 20) = "HKEY_CURRENT_CONFIG" Then
        currentPathNorm = "HKCC" & Mid(currentPathNorm, 21)
    End If

    Print "DEBUG: 标准化当前路径: " & currentPathNorm
    Print "DEBUG: Hive根路径(标准): " & hiveRootUpper

    ' 步骤2：安全截取相对路径
    If currentPathNorm = hiveRootUpper Then
        ' 路径完全匹配，相对路径为空（就是hive根节点）
        relativePath = ""
    ElseIf Left(currentPathNorm, Len(hiveRootUpper) + 1) = hiveRootUpper & "\" Then
        ' 当前路径是hive根路径的子键，截取后面的部分
        relativePath = Mid(currentPathNorm, Len(hiveRootUpper) + 2)
    Else
        ' 理论上不应该走到这里，除非GetHiveInfoFromPath逻辑有误
        Print "DEBUG: 警告 - 路径不匹配，无法计算相对路径"
    End If

    Print "DEBUG: 最终相对路径: " & relativePath
    ' ===== 修复结束 =====
    
    ' 查找目标NK节点
    targetNKOffset = FindNKByPath(hiveCacheItem.HiveData(), hiveCacheItem.RootNKFileOffset, relativePath)
    If targetNKOffset = 0 Then
        Print "DEBUG: 在hive中找不到目标节点"
        GoTo RenderFinalUI
    End If
    
    ' 获取hive中目标键的原始数据
    hiveKeyItem = GetKeyItemFromNK(hiveCacheItem.HiveData(), targetNKOffset, currentFullPath)
    Print "DEBUG: Hive子键数量: " & (UBound(hiveKeyItem.SubKeys) + 1)
    Print "DEBUG: Hive值数量: " & (UBound(hiveKeyItem.Values) + 1)
    
    ' 核心：检测隐藏项
    DetectHiddenItems apiKeyItem, hiveKeyItem, hiddenSubKeys(), hiddenValues()
    
    ' ==================================================
    ' ===== Step4: 修复后的UI渲染 - 仅操作当前节点 =====
    ' ==================================================
RenderFinalUI:
    ' 仅清空当前节点的子节点，不影响整棵树
    Dim hChild As HTREEITEM = treKey.GetChild(Node)
    While hChild <> NULL
        Dim hNext As HTREEITEM = treKey.GetNextSiblin(hChild)
        treKey.DeleteItem hChild
        hChild = hNext
    Wend
    
    ' 先添加正常子键，再添加带标记的隐藏子键
    If UBound(apiKeyItem.SubKeys) >= LBound(apiKeyItem.SubKeys) Then
        For i As Integer = LBound(apiKeyItem.SubKeys) To UBound(apiKeyItem.SubKeys)
            If TreeViewFindChild(treKey, Node, apiKeyItem.SubKeys(i)) = NULL Then
                treKey.AddItem(Node, apiKeyItem.SubKeys(i))
            End If
        Next
    End If
    If UBound(hiddenSubKeys) >= LBound(hiddenSubKeys) Then
        For i As Integer = LBound(hiddenSubKeys) To UBound(hiddenSubKeys)
            Dim displayName As String = hiddenSubKeys(i) & " [隐藏]"
            If TreeViewFindChild(treKey, Node, hiddenSubKeys(i)) = NULL Then
                treKey.AddItem(Node, displayName)
            End If
        Next
    End If
    
    ' 渲染值列表：正常值 + 带标记的隐藏值
    If UBound(apiKeyItem.Values) >= LBound(apiKeyItem.Values) Then
        For j As Integer = LBound(apiKeyItem.Values) To UBound(apiKeyItem.Values)
            AddItemColListEx lvwValue, 3,,, WStr(apiKeyItem.Values(j).ValueName), WStr(GetRegTypeName(apiKeyItem.Values(j).ValueType)), WStr(apiKeyItem.Values(j).ValueData)
        Next
    End If
    If UBound(hiddenValues) >= LBound(hiddenValues) Then
        For j As Integer = LBound(hiddenValues) To UBound(hiddenValues)
            Dim displayName As String = hiddenValues(j).ValueName & " [隐藏]"
            AddItemColListEx lvwValue, 3,,, WStr(displayName), WStr(GetRegTypeName(hiddenValues(j).ValueType)), "[隐藏数据 - 仅Hive中存在]"
        Next
    End If
    
    Print "=== DEBUG: UseHive=True 检测完成 ==="
    CommitListViewView lvwValue
End Sub

Function AddRButtonMenu() As BOOLEAN
    ' 右键菜单显示名称
    Dim menuName As Const WString * MAX_PATH = "使用SnowSword粉碎文件"

    Dim hKey As HKEY
    Dim hKey1 As hKey
    Dim hCmdKey As hKey
    Dim result As Long
    
    Dim szSubKey1 As Const WString * MAX_PATH = "*\shell\SnowSword"
    Function = True

    ' 为所有文件类型添加右键菜单
    result = RegCreateKeyEx(HKEY_CLASSES_ROOT, @szSubKey1, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hKey, NULL)
    ' 创建注册表项（HKEY_CLASSES_ROOT\*\shell\SnowSword\runas）
    result = RegCreateKeyEx(HKEY, StrPtrW("runas"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hKey1, NULL)
    If result = ERROR_SUCCESS Then
        ' 设置菜单显示名称
        RegSetValueExW hKey, NULL, 0, REG_SZ, CPtr(LPBYTE, @menuName), SizeOf(menuName)
        ' 创建command子项，指定程序路径
        result = RegCreateKeyEx(hKey, StrPtrW("command"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, @hCmdKey, NULL)
        If result = ERROR_SUCCESS Then
            ' 设置命令，%1表示选中的文件路径
            Dim strCommand As Const WString * MAX_PATH = !"PowerShell -Command \"Start-Process" & !" \'" & App.Path & App.EXEName _
                                                                                                & !"\' " & "-ArgumentList '-DeleteFile " & !"\"\"%1\"\"" & !"\' -Verb RunAs\""
            RegSetValueExW hCmdKey, NULL, 0, REG_SZ, CPtr(LPBYTE, @strCommand), SizeOf(strCommand)
            
            Print "右键菜单注册成功！"
        Else
            Print "创建command子项失败，错误代码：" & result
            Function = False
        End If
        
        Function = True
    Else
        Print "创建注册表项失败，错误代码：" & result
        Function = False
    End If
    RegCloseKey hCmdKey
    RegCloseKey hKey1
    RegCloseKey hKey
End Function

Function DeleteRButtonMenu() As BOOLEAN
    Dim result As Long

    ' 删除为所有文件类型添加的右键菜单
    result = RegDeleteTree(HKEY_CLASSES_ROOT, StrPtrW("*\shell\MyProgram"))

    If result = ERROR_SUCCESS Then
        Print "右键菜单已成功移除！"
        Return True
    ElseIf result = ERROR_FILE_NOT_FOUND Then
        Print "未找到已注册的右键菜单"
        Return False
    Else
        Print "移除右键菜单失败，错误代码：" & result
        Return False
    End If
End Function
