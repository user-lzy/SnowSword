'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModMemory.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


Private Function GetProtectString(ByVal Protect As Long) As String
    Select Case Protect
        Case PAGE_READONLY
            GetProtectString = "PAGE_READONLY"
        Case PAGE_READWRITE
            GetProtectString = "PAGE_READWRITE"
        Case PAGE_EXECUTE_READ
            GetProtectString = "PAGE_EXECUTE_READ"
        Case PAGE_EXECUTE_READWRITE
            GetProtectString = "PAGE_EXECUTE_READWRITE"
        Case Else
            GetProtectString = "UNKNOWN"
    End Select
End Function

Private Function GetStateString(ByVal State As Long) As String
    Select Case State
        Case MEM_COMMIT
            GetStateString = "MEM_COMMIT"
        Case MEM_RESERVE
            GetStateString = "MEM_RESERVE"
        Case MEM_FREE
            GetStateString = "MEM_FREE"
        Case Else
            GetStateString = "UNKNOWN"
    End Select
End Function

Private Function GetTypeString(ByVal MemoryType As Long) As String
    Select Case MemoryType
        Case MEM_PRIVATE
            GetTypeString = "MEM_PRIVATE"
        Case MEM_MAPPED
            GetTypeString = "MEM_MAPPED"
        Case MEM_IMAGE
            GetTypeString = "MEM_IMAGE"
        Case Else
            GetTypeString = "UNKNOWN"
    End Select
End Function

Private Sub GetMemoryBlockList(ByVal dwProcessId As DWORD, lvwMemory As Class_ListView)
    Dim mbi As MEMORY_BASIC_INFORMATION
    Dim Address As PVOID
    Dim AddressEnd As Long
    Dim MemorySize As Long
    Dim MemoryProtect As Long
    Dim MemoryType As Long
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION, False, dwProcessId)
    If (hProcess <= 0) Then
        PrintA "OpenProcess " & dwProcessId & "Failed!"
        Exit Sub
    End If
    
    Do While VirtualQueryEx(hProcess, Address, @mbi, Len(mbi)) <> 0
        ' 检查是否为 0x1000 大小的块
        If mbi.RegionSize = &H1000 Then
            ' 跳过 0x1000 大小的块
            Address = Address + mbi.RegionSize
            Continue Do
        End If

        ' 检查是否需要合并属性相同的块
        If MemoryProtect = mbi.Protect And MemoryType = mbi.Type Then
            ' 合并块
            MemorySize = MemorySize + mbi.RegionSize
        Else
            ' 添加新的块到 ListView
            If MemorySize > 0 Then
                With lvwMemory
                    .AddItem "0x" & WHex(Address)
                    .SetItemText .ItemCount - 1, 1, "0x" & WHex(MemorySize)
                    .SetItemText .ItemCount - 1, 2, GetProtectString(MemoryProtect)
                    .SetItemText .ItemCount - 1, 3, GetStateString(mbi.State)
                    .SetItemText .ItemCount - 1, 4, GetTypeString(MemoryType)
                    .SetItemText .ItemCount - 1, 5, "0x" & WHex(mbi.AllocationBase)
                End With
            End If

            ' 更新当前块信息
            Address = mbi.BaseAddress
            MemorySize = mbi.RegionSize
            MemoryProtect = mbi.Protect
            MemoryType = mbi.Type
        End If

        ' 更新地址
        Address = Address + mbi.RegionSize
    Loop

    ' 添加最后一个块
    If MemorySize > 0 Then
        With lvwMemory
            .AddItem "0x" & WHex(Address)
            .SetItemText .ItemCount - 1, 1, "0x" & WHex(MemorySize)
            .SetItemText .ItemCount - 1, 2, GetProtectString(MemoryProtect)
            .SetItemText .ItemCount - 1, 3, GetStateString(mbi.State)
            .SetItemText .ItemCount - 1, 4, GetTypeString(MemoryType)
            .SetItemText .ItemCount - 1, 5, "0x" & WHex(mbi.AllocationBase)
        End With
    End If
End Sub