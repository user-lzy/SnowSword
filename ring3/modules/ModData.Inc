/'Type HashEntry
    Key As Long
    Value As Long
    Next As HashEntry Ptr
End Type

Public Type HashTable
    bucketSize As Integer
    buckets() As HashEntry Ptr
    ' 构造函数
    Declare Constructor(ByVal SIZE As Integer)
    ' 析构函数
    Declare Destructor
    ' 方法
    Declare Sub Add(Byval key As Long, Byval value As Long)
    Declare Function Contains(ByVal key As Long) As BOOLEAN
    Declare Function Get(Byval key As Long) As Long
    Declare Sub Remove(Byval key As Long)
    ' 操作符
    Declare Operator [](Byval key As Long) As Long
End Type

Constructor HashTable(size As Integer)
    buckets = New HashEntry Ptr [size]
    For i As Integer = 0 To size - 1
        buckets[i] = NULL
    Next i
    bucketSize = size
End Constructor

Destructor HashTable()
    For i As Integer = 0 To bucketSize - 1
        Dim current As HashEntry Ptr = buckets[i]
        While current <> NULL
            Dim nextEntry As HashEntry Ptr = current->Next
            Delete current
            current = nextEntry
        Wend
    Next i
End Destructor

Sub HashTable.Add(key As Long, value As Long)
    Dim hashKey As Integer = key Mod bucketSize
    Dim current As HashEntry Ptr = buckets[hashKey]
    Dim previous As HashEntry Ptr = NULL
    
    While current <> NULL
        If current->Key = key Then
            current->Value = value ' 更新值
            Exit Sub
        End If
        previous = current
        current = current->Next
    Wend
    
    Dim newNode As HashEntry Ptr = New HashEntry
    newNode->Key = key
    newNode->Value = value
    newNode->Next = NULL
    
    If previous = NULL Then
        buckets[hashKey] = newNode
    Else
        previous->Next = newNode
    End If
End Sub

Function HashTable.Contains(key As Long) As Boolean
    Dim hashKey As Integer = key Mod bucketSize
    Dim current As HashEntry Ptr = buckets[hashKey]
    While current <> NULL
        If current->Key = key Then
            Return True
        End If
        current = current->Next
    Wend
    Return False
End Function

Function HashTable.Get(key As Long) As Long
    Dim hashKey As Integer = key Mod bucketSize
    Dim current As HashEntry Ptr = buckets[hashKey]
    While current <> NULL
        If current->Key = key Then
            Return current->Value
        End If
        current = current->Next
    Wend
    Return 0
End Function

Sub HashTable.Remove(key As Long)
    Dim hashKey As Integer = key Mod bucketSize
    Dim current As HashEntry Ptr = buckets[hashKey]
    Dim previous As HashEntry Ptr = NULL
    
    While current <> NULL
        If current->Key = key Then
            If previous = NULL Then
                buckets[hashKey] = current->Next
            Else
                previous->Next = current->Next
            End If
            Delete current
            Exit Sub
        End If
        previous = current
        current = current->Next
    Wend
End Sub

Operator HashTable.[](key As Long) As Long
    Operator = Get(key)
End Operator'/