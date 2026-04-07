Function BinarySearch_DWord(arr() As DWORD, target As DWORD) As Integer
    Dim As Integer low, high, mid1
    
    low = LBound(arr)
    high = UBound(arr)
    
    While low <= high
        mid1 = (low + high) \ 2  ' 计算中间位置
        
        If arr(mid1) = target Then
            Return mid1  ' 找到目标，返回索引
        ElseIf arr(mid1) < target Then
            low = mid1 + 1  ' 在右半部分搜索
        Else
            high = mid1 - 1  ' 在左半部分搜索
        End If
    Wend
    
    Return -1  ' 未找到返回-1
End Function

' 二分查找函数（返回插入位置版本）
' 如果找到返回索引，如果未找到返回应插入的位置（负值）
Function BinarySearch_DWord_InsertPos(arr() As DWORD, target As DWORD) As Integer
    Dim As Integer low, high, mid1
    
    low = LBound(arr)
    high = UBound(arr)
    
    While low <= high
        mid1 = (low + high) \ 2
        
        If arr(mid1) = target Then
            Return mid1  ' 找到返回索引
        ElseIf arr(mid1) < target Then
            low = mid1 + 1
        Else
            high = mid1 - 1
        End If
    Wend
    
    ' 未找到，返回应插入位置的负值（如返回-3表示应插入到索引3处）
    Return -low
End Function

' 分区函数（使用三数取中法选择枢轴）
Private Function Partition(arr() As DWORD, low As Integer, high As Integer) As Integer
    Dim As Integer i, j, pivot
    
    ' 三数取中法：选择左、中、右三个位置的中间值作为枢轴
    Dim midNum As Integer = (low + high) \ 2
    
    ' 将三个值排序
    If arr(low) > arr(midNum) Then Swap arr(low), arr(midNum)
    If arr(midNum) > arr(high) Then Swap arr(midNum), arr(high)
    If arr(low) > arr(midNum) Then Swap arr(low), arr(midNum)
    
    ' 选择中间值作为枢轴，并交换到末尾
    Swap arr(midNum), arr(high)
    pivot = arr(high)
    
    i = low - 1
    
    For j = low To high - 1
        If arr(j) <= pivot Then
            i += 1
            Swap arr(i), arr(j)
        End If
    Next j
    
    Swap arr(i + 1), arr(high)
    Return i + 1
End Function

' 快速排序主函数
Private Sub QuickSort(arr() As DWORD, low As Integer, high As Integer)
    ' 小规模数组使用插入排序更高效
    Const INSERTION_THRESHOLD As Integer = 10
    
    If low < high Then
        ' 如果数组规模较小，使用插入排序
        If high - low + 1 <= INSERTION_THRESHOLD Then
            ' 插入排序
            Dim As Integer i, j, key
            For i = low + 1 To high
                key = arr(i)
                j = i - 1
                While j >= low AndAlso arr(j) > key
                    arr(j + 1) = arr(j)
                    j -= 1
                Wend
                arr(j + 1) = key
            Next i
        Else
            ' 快速排序
            Dim pi As Integer = Partition(arr(), low, high)
            QuickSort arr(), low, pi - 1
            QuickSort arr(), pi + 1, high
        End If
    End If
End Sub

' 包装函数（方便调用）
Sub QuickSortWrapper(arr() As DWORD)
    If UBound(arr) > LBound(arr) Then
        QuickSort arr(), LBound(arr), UBound(arr)
    End If
End Sub

Function IsDWORDInArray(ByVal Element As DWORD, Array() As DWORD) As Boolean
    Dim i As Integer
    For i = LBound(Array) To UBound(Array)
        If (Array(i) = Element) Then Return True
    Next
    Return False
End Function

'' 改进版：将DWORD数据插入到有序数组中，拒绝重复元素，避免频繁ReDim
Sub InsertDwordToSortedArray(arr() As DWORD, ByVal newValue As DWORD)
    Dim As Integer i, insertPos, actualSize, ub, capacity
    
    ' 检查数组是否未初始化
    If UBound(arr) = -1 Then
        ReDim arr(0) As DWORD
        arr(0) = newValue
        Exit Sub
    End If
    
    ub = UBound(arr)
    
    ' 计算实际使用的大小（非零元素数量）
    actualSize = 0
    For i = LBound(arr) To ub
        If arr(i) <> 0 Then
            actualSize = actualSize + 1
        Else
            Exit For  ' 遇到第一个空位，停止计数
        End If
    Next i
    
    ' 检查重复元素（仅在已使用的部分检查）
    For i = 0 To actualSize - 1
        If arr(i) = newValue Then
            ' 发现重复元素，直接返回，不插入
            Print "重复元素 "; newValue; " 不插入"
            Exit Sub
        End If
    Next i
    
    ' 查找插入位置（保持升序排序，只在已使用部分查找）
    insertPos = -1
    For i = 0 To actualSize - 1
        If arr(i) >= newValue Then
            insertPos = i
            Exit For
        End If
    Next i
    
    ' 如果新值大于所有现有值，则插入到末尾
    If insertPos = -1 Then
        insertPos = actualSize
    End If
    
    ' 检查是否需要扩展数组
    If actualSize > ub Then
        ' 数组已满，需要扩展
        ' 避免频繁ReDim的策略：按一定策略增加容量
        If ub < 50 Then
            capacity = ub + 10  ' 小数组，增加10个位置
        ElseIf ub < 500 Then
            capacity = ub + 50  ' 中等数组，增加50个位置
        Else
            capacity = ub * 3 \ 2  ' 大数组，增加50%容量
        End If
        
        ReDim Preserve arr(capacity) As DWORD
    End If
    
    ' 如果插入位置在数组中间，需要移动元素
    If insertPos < actualSize Then
        ' 从最后一个实际元素开始，向后移动，直到插入位置
        For i = actualSize To insertPos + 1 Step -1
            arr(i) = arr(i - 1)
        Next i
    End If
    
    ' 插入新值
    arr(insertPos) = newValue
End Sub