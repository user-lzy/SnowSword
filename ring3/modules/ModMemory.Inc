Enum MEMORY_SCAN_RESULT
    Normal = 0
    Shellcode
    HideDriver
    UnknownCode
End Enum

' 定义线程参数结构
Type ScanThreadParam
    lvwSuspectKernelMemory As Class_ListView
    StartAddr As UInteger
    EndAddr As UInteger
    hMutex As Any Ptr      ' 互斥锁用于同步访问共享资源
    ScannedPages As UInteger Ptr  ' 共享进度计数器
    ThreadIndex As Integer  ' 线程索引
    SuspectCount As Integer Ptr ' 发现的可疑内存计数
    bFinish As Boolean ' 结束标志
End Type

Const NUM_THREADS As Integer = 8 ' 可根据CPU核心数调整
Const PAGE_SIZE As DWORD = &H1000 ' (4KB大小)

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

' 增强的GetProtectString函数，处理更多情况
Private Function GetProtectString(ByVal Protect As Long) As String
    ' 首先检查是否为0
    If Protect = 0 Then
        GetProtectString = "PAGE_NOACCESS"
        Exit Function
    End If
    
    Select Case Protect
        Case PAGE_NOACCESS
            GetProtectString = "PAGE_NOACCESS"
        Case PAGE_READONLY
            GetProtectString = "PAGE_READONLY"
        Case PAGE_READWRITE
            GetProtectString = "PAGE_READWRITE"
        Case PAGE_EXECUTE_READ
            GetProtectString = "PAGE_EXECUTE_READ"
        Case PAGE_EXECUTE_READWRITE
            GetProtectString = "PAGE_EXECUTE_READWRITE"
        Case PAGE_WRITECOPY
            GetProtectString = "PAGE_WRITECOPY"
        Case PAGE_EXECUTE
            GetProtectString = "PAGE_EXECUTE"
        Case PAGE_EXECUTE_WRITECOPY
            GetProtectString = "PAGE_EXECUTE_WRITECOPY"
        ' 处理组合标志
        Case PAGE_READONLY Or PAGE_GUARD
            GetProtectString = "PAGE_READONLY | PAGE_GUARD"
        Case PAGE_READWRITE Or PAGE_GUARD
            GetProtectString = "PAGE_READWRITE | PAGE_GUARD"
        Case PAGE_EXECUTE_READ Or PAGE_GUARD
            GetProtectString = "PAGE_EXECUTE_READ | PAGE_GUARD"
        Case PAGE_EXECUTE_READWRITE Or PAGE_GUARD
            GetProtectString = "PAGE_EXECUTE_READWRITE | PAGE_GUARD"
        Case PAGE_WRITECOPY Or PAGE_GUARD
            GetProtectString = "PAGE_WRITECOPY | PAGE_GUARD"
        Case PAGE_EXECUTE Or PAGE_GUARD
            GetProtectString = "PAGE_EXECUTE | PAGE_GUARD"
        Case PAGE_EXECUTE_WRITECOPY Or PAGE_GUARD
            GetProtectString = "PAGE_EXECUTE_WRITECOPY | PAGE_GUARD"
        Case Else
            ' 尝试分解保护标志
            Dim result As String
            result = ""
            
            If (Protect And PAGE_NOACCESS) = PAGE_NOACCESS Then
                result = result & "PAGE_NOACCESS | "
            End If
            If (Protect And PAGE_READONLY) = PAGE_READONLY Then
                result = result & "PAGE_READONLY | "
            End If
            If (Protect And PAGE_READWRITE) = PAGE_READWRITE Then
                result = result & "PAGE_READWRITE | "
            End If
            If (Protect And PAGE_EXECUTE_READ) = PAGE_EXECUTE_READ Then
                result = result & "PAGE_EXECUTE_READ | "
            End If
            If (Protect And PAGE_EXECUTE_READWRITE) = PAGE_EXECUTE_READWRITE Then
                result = result & "PAGE_EXECUTE_READWRITE | "
            End If
            If (Protect And PAGE_WRITECOPY) = PAGE_WRITECOPY Then
                result = result & "PAGE_WRITECOPY | "
            End If
            If (Protect And PAGE_EXECUTE) = PAGE_EXECUTE Then
                result = result & "PAGE_EXECUTE | "
            End If
            If (Protect And PAGE_EXECUTE_WRITECOPY) = PAGE_EXECUTE_WRITECOPY Then
                result = result & "PAGE_EXECUTE_WRITECOPY | "
            End If
            If (Protect And PAGE_GUARD) = PAGE_GUARD Then
                result = result & "PAGE_GUARD | "
            End If
            If (Protect And PAGE_NOCACHE) = PAGE_NOCACHE Then
                result = result & "PAGE_NOCACHE | "
            End If
            If (Protect And PAGE_WRITECOMBINE) = PAGE_WRITECOMBINE Then
                result = result & "PAGE_WRITECOMBINE | "
            End If
            
            If Len(result) > 0 Then
                result = Left(result, Len(result) - 3) ' 移除最后的 " | "
                GetProtectString = result
            Else
                GetProtectString = "UNKNOWN (0x" & Hex(Protect) & ")"
            End If
    End Select
End Function

' 辅助函数：直接添加内存块
Private Sub AddMemoryBlockDirectly(ByRef lvwMemory As Class_ListView, ByVal dwProcessId As DWORD, ByRef mbi As MEMORY_BASIC_INFORMATION)
    Dim actualProtect As Long = mbi.Protect
    If actualProtect = 0 And mbi.State = MEM_COMMIT Then
        actualProtect = mbi.AllocationProtect
    End If
    
    With lvwMemory
        .AddItem "0x" & WHex(mbi.BaseAddress)
        .SetItemText .ItemCount - 1, 1, "0x" & WHex(mbi.RegionSize)
        .SetItemText .ItemCount - 1, 2, GetProtectString(actualProtect)
        .SetItemText .ItemCount - 1, 3, GetStateString(mbi.State)
        .SetItemText .ItemCount - 1, 4, GetTypeString(mbi.Type)
        .SetItemText .ItemCount - 1, 5, "0x" & WHex(mbi.AllocationBase)
        
        If mbi.Type = MEM_IMAGE Then
            Dim szModuleName As WString * MAX_PATH
            GetModuleByAddress dwProcessId, mbi.BaseAddress, @szModuleName
            .SetItemText .ItemCount - 1, 6, szModuleName
        Else
            .SetItemText .ItemCount - 1, 6, ""
        End If
    End With
End Sub

' 如果需要合并功能，使用这个版本（更安全的合并逻辑）
Private Sub GetMemoryBlockList(ByVal dwProcessId As DWORD, lvwMemory As Class_ListView)
    Dim mbi As MEMORY_BASIC_INFORMATION
    Dim QueryAddress As PVOID
    Dim hProcess As HANDLE = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_VM_READ, False, dwProcessId)
    
    If (hProcess <= 0) Then
        Print "OpenProcess " & dwProcessId & " Failed!"
        If IsDriverLoaded Then hProcess = SnowSword_OpenProcess(dwProcessId)
        If hProcess <= 0 Then Exit Sub
    End If
    
    ' 使用集合来存储内存块信息，避免在遍历时合并
    Dim memoryBlocks() As MEMORY_BASIC_INFORMATION
    Dim blockCount As Long = 0
    
    ' 第一步：收集所有内存块
    QueryAddress = 0
    Do While VirtualQueryEx(hProcess, QueryAddress, @mbi, Len(mbi)) <> 0
        ' 只处理有效的基地址
        If mbi.BaseAddress <> 0 Then
            ' 扩展数组
            blockCount = blockCount + 1
            Redim Preserve memoryBlocks(blockCount - 1)
            memoryBlocks(blockCount - 1) = mbi
        End If
        
        QueryAddress = mbi.BaseAddress + mbi.RegionSize
        If QueryAddress <= mbi.BaseAddress Then
            Exit Do
        End If
    Loop
    
    ' 第二步：合并相邻且属性相同的块
    If blockCount > 0 Then
        Dim i As Long
        Dim currentBlock As MEMORY_BASIC_INFORMATION
        currentBlock = memoryBlocks(0)
        
        For i = 1 To blockCount - 1
            Dim nextBlock As MEMORY_BASIC_INFORMATION
            nextBlock = memoryBlocks(i)
            
            ' 检查是否可以合并：相邻且属性相同
            If (currentBlock.BaseAddress + currentBlock.RegionSize = nextBlock.BaseAddress) And _
               (currentBlock.Protect = nextBlock.Protect) And _
               (currentBlock.State = nextBlock.State) And _
               (currentBlock.Type = nextBlock.Type) Then
                ' 合并块
                currentBlock.RegionSize = currentBlock.RegionSize + nextBlock.RegionSize
            Else
                ' 添加当前合并块到列表
                AddMemoryBlockDirectly lvwMemory, dwProcessId, currentBlock
                ' 开始新的合并块
                currentBlock = nextBlock
            End If
        Next i
        
        ' 添加最后一个块
        AddMemoryBlockDirectly lvwMemory, dwProcessId, currentBlock
    End If
    
    CloseHandle hProcess
End Sub

Private Function GetMemoryScanType(ByVal MemoryScanType As MEMORY_SCAN_RESULT) As String
    Select Case MemoryScanType
        Case Normal
            Return "Normal"
        Case ShellCode
            Return "ShellCode"
        Case HideDriver
            Return "HideDriver"
        Case UnknownCode
            Return "UnknownCode"
    End Select
End Function

/'Function ScanSuspectKernelMemory(lvwSuspectKernelMemory As Class_ListView) As DWORD
    Dim StartAddr As UInteger = &HFFFFF80000000000ULL
    Dim EndAddr As UInteger = &HFFFFF90000000000ULL
    Dim result As MEMORY_SCAN_RESULT
    
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Return 0
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            FrmMain.Check3.Value = False
            Return 0
        End If
    End If
    
    Dim stMemory As MemoryStruct
    For Addr As UInteger = StartAddr To EndAddr Step PAGE_SIZE
        stMemory.Addr = Cast(PVOID, Addr)
        stMemory.dwSize = PAGE_SIZE
        If IoControl(hDrv, IOCTL_ScanKernelMemory, @stMemory, SizeOf(MemoryStruct), @result, SizeOf(MEMORY_SCAN_RESULT)) <> 0 Then
            If result <> Normal Then lvwSuspectKernelMemory.AddItemColList,, GetMemoryScanType(result), "0x" & WHex(Addr),, GetDriverPathByAddr(Addr)
        End If
        FF_DoEvents
        FrmMain.lblNum.Caption =  "进度:" & WStr(round((Addr - StartAddr) / (EndAddr - StartAddr) * 100)) & "%"
    Next
    
    Return 0
End Function'/

' 线程工作函数
Sub KernelMemoryScanThread(ByVal userdata As Any Ptr)
    Dim p As ScanThreadParam Ptr = Cast(ScanThreadParam Ptr, userdata)
    Dim stMemory As MemoryStruct
    Dim result As MEMORY_SCAN_RESULT
    Dim localScanCount As UInteger = 0
    
    ' 遍历分配的内存范围
    For Addr As UInteger = p->StartAddr To p->EndAddr Step PAGE_SIZE
        stMemory.Addr = Cast(PVOID, Addr)
        stMemory.dwSize = PAGE_SIZE
        
        ' 调用驱动扫描内存
        If IoControl(hDrv, IOCTL_ScanKernelMemory, @stMemory, SizeOf(MemoryStruct), @result, SizeOf(MEMORY_SCAN_RESULT)) <> 0 Then
            If result <> Normal Then
                ' 发现可疑内存，需要同步添加到列表
                ' 增加可疑计数
                Dim SuspectIndex As Integer = 0
                MutexLock(p->hMutex)
                SuspectIndex = *p->SuspectCount
                'Print SuspectIndex & " 0x" & WHex(Addr)
                p->lvwSuspectKernelMemory.AddItemColList,, WStr(SuspectIndex), GetMemoryScanType(result), "0x" & WHex(Addr), GetDriverPathByAddr(Addr)
                *p->SuspectCount += 1
                MutexUnlock(p->hMutex)
            End If
        End If
        
        ' 增加扫描页数计数器
        MutexLock(p->hMutex)
        *p->ScannedPages += 1
        MutexUnlock(p->hMutex)
        
        localScanCount += 1
        
        ' 定期让出CPU时间给其他线程和UI
        If (localScanCount Mod 20) = 0 Then
            FF_DoEvents
        End If
    Next
    Print "扫描起始地址0x" & p->StartAddr & "的线程退出"
    p->bFinish = True
End Sub

' 主函数中的多线程扫描实现
Sub ScanKernelMemoryMultiThread(lvwSuspectKernelMemory As Class_ListView)
    Dim threads(NUM_THREADS - 1) As Any Ptr
    Dim params(NUM_THREADS - 1) As ScanThreadParam
    Dim hMutex As Any Ptr
    Dim totalPages As UInteger
    Dim scannedPages As UInteger = 0
    Dim suspectCount As Integer = 0
    Dim i As Integer
    
    Dim StartAddr As UInteger = &HFFFFF80300000000ULL
    Dim EndAddr As UInteger = &HFFFFF90000000000ULL ' &HFFFFF80387840000
    
    Dim result As MEMORY_SCAN_RESULT
    If AfxMsg("是否扫描?",, MB_YESNO) = IDNO Then Exit Sub
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Return
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value = True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            FrmMain.Check3.Value = False
            Return
        End If
    End If
    
    ' 计算总页数
    totalPages = ((EndAddr - StartAddr) \ PAGE_SIZE) + 1
    
    ' 创建互斥锁
    hMutex = MutexCreate()
    If hMutex = 0 Then
        MsgBox("无法创建互斥锁")
        Exit Sub
    End If
    
    ' 设置进度标签初始值
    FrmMain.lblNum.Caption = "进度: 0%"
    FF_DoEvents
    
    ' 创建并启动线程
    For i = 0 To NUM_THREADS - 1
        ' 计算每个线程的地址范围（确保地址对齐）
        Dim addrRange As UInteger = (EndAddr - StartAddr) \ NUM_THREADS
        params(i).StartAddr = StartAddr + i * addrRange
        Print "为线程" & i & "分配扫描地址0x" & Hex(params(i).StartAddr) & "的任务"
        ' 最后一个线程处理剩余所有范围
        If i = NUM_THREADS - 1 Then
            params(i).EndAddr = EndAddr
        Else
            params(i).EndAddr = params(i).StartAddr + addrRange - 1
        End If
        
        params(i).lvwSuspectKernelMemory = lvwSuspectKernelMemory
        
        ' 确保地址是PAGE_SIZE对齐
        params(i).StartAddr = (params(i).StartAddr \ PAGE_SIZE) * PAGE_SIZE
        params(i).EndAddr = ((params(i).EndAddr + PAGE_SIZE - 1) \ PAGE_SIZE) * PAGE_SIZE
        
        ' 设置同步参数
        params(i).hMutex = hMutex
        params(i).scannedPages = @scannedPages
        params(i).suspectCount = @suspectCount
        params(i).bFinish = False
        
        ' 创建线程
        threads(i) = ThreadCreate(@KernelMemoryScanThread, @params(i))
        If threads(i) = 0 Then
            AfxMsg "线程 " & i & " 创建失败"
        End If
    Next
    
    ' 等待线程完成并更新进度
    Dim lastProgress As Integer = -1
    
    Print "正在等待所有线程结束..."

    Do While True
        ' 检查线程状态
        Dim activeThreadCount As Integer = NUM_THREADS
        'Print "activeThreadCount:" & activeThreadCount
        For i As Integer = 0 To NUM_THREADS - 1
            If params(i).bFinish = True Then activeThreadCount -= 1
        Next
        If activeThreadCount = 0 Then
            Print "线程已全部退出!"
            Exit Do
        End If
        
        ' 获取并更新进度
        'MutexLock(hMutex)
        Dim currentProgress As Integer = (scannedPages * 100) \ totalPages
        'MutexUnlock(hMutex)
        
        ' 只在进度变化时更新UI（减少闪烁）
        If currentProgress <> lastProgress Then
            lastProgress = currentProgress
            FrmMain.lblNum.Caption = "进度:" & WStr(currentProgress) & "%"
            FrmMain.lblNum.Refresh  ' 强制立即刷新
        End If
        
        FF_DoEvents
        'Sleep(1)  ' 避免CPU占用过高
    Loop
    
    ' 清理
    MutexDestroy(hMutex)
    
    ' 扫描完成
    AfxMsg "扫描完成，发现 " & suspectCount & " 处可疑内存"
End Sub
