'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModData.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


/'Type HashEntry
    Key As Long
    Value As Long
    Next As HashEntry Ptr
End Type

Public Type HashTable
    bucketSize As Integer
    buckets() As HashEntry Ptr
    ' 构造函数
'     Declare Constructor(ByVal SIZE As Integer) '已经搬到定义文件处
    ' 析构函数
'     Declare Destructor '已经搬到定义文件处
    ' 方法
'     Declare Sub Add(Byval key As Long, Byval value As Long) '已经搬到定义文件处
'     Declare Function Contains(Byval key As Long) As Boolean '已经搬到定义文件处
'     Declare Function Get(Byval key As Long) As Long '已经搬到定义文件处
'     Declare Sub Remove(Byval key As Long) '已经搬到定义文件处
    ' 操作符
'     Declare Operator [](Byval key As Long) As Long '已经搬到定义文件处
End Type

Constructor HashTable(size As Integer)
    This.buckets = New HashEntry Ptr [size]
    For i As Integer = 0 To size - 1
        This.buckets[i] = NULL
    Next i
    This.bucketSize = size
End Constructor

Destructor HashTable()
    For i As Integer = 0 To This.bucketSize - 1
        Dim current As HashEntry Ptr = This.buckets[i]
        While current <> NULL
            Dim nextEntry As HashEntry Ptr = current->Next
            Delete current
            current = nextEntry
        Wend
    Next i
End Destructor

Sub HashTable.Add(key As Long, value As Long)
    Dim hashKey As Integer = key Mod This.bucketSize
    Dim current As HashEntry Ptr = This.buckets[hashKey]
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
        This.buckets[hashKey] = newNode
    Else
        previous->Next = newNode
    End If
End Sub

Function HashTable.Contains(key As Long) As Boolean
    Dim hashKey As Integer = key Mod This.bucketSize
    Dim current As HashEntry Ptr = This.buckets[hashKey]
    While current <> NULL
        If current->Key = key Then
            Return True
        End If
        current = current->Next
    Wend
    Return False
End Function

Function HashTable.Get(key As Long) As Long
    Dim hashKey As Integer = key Mod This.bucketSize
    Dim current As HashEntry Ptr = This.buckets[hashKey]
    While current <> NULL
        If current->Key = key Then
            Return current->Value
        End If
        current = current->Next
    Wend
    Return 0
End Function

Sub HashTable.Remove(key As Long)
    Dim hashKey As Integer = key Mod This.bucketSize
    Dim current As HashEntry Ptr = This.buckets[hashKey]
    Dim previous As HashEntry Ptr = NULL
    
    While current <> NULL
        If current->Key = key Then
            If previous = NULL Then
                This.buckets[hashKey] = current->Next
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
    Operator = This.Get(key)
End Operator'/