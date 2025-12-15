Type SafeStringW_NTS
Private:
    m_pData As UShort Ptr
    m_nLength As ULong
    m_capacity As ULong  ' 新增：当前分配的字符容量
    m_refCount As UInteger Ptr
    
    Declare Sub CopyFromWString(wszStr As Wstring Ptr)
    Declare Sub MakeUnique()
    
Public: 
    Declare Sub AllocateString(ByVal nChars As ULong)
    
    Declare Constructor()
    Declare Constructor(wszStr As Wstring)
    Declare Constructor(szStr As String)
    Declare Constructor(ByRef other As SafeStringW_NTS)
    Declare Destructor()
    
    Declare Operator Let(wszStr As Wstring)
    Declare Operator Let(szStr As String)
    Declare Operator Let(ByRef other As SafeStringW_NTS)
    
    Declare Operator Cast() ByRef As Wstring  ' 返回指针而非引用
    Declare Function WStr() ByRef As Wstring  ' 返回指针
    Declare Function Str() As String
    
    Declare Function Length() As ULong
    Declare Function IsEmpty() As Boolean
    Declare Sub Reserve(ByVal nCapacity As ULong)
    Declare Sub Move(ByRef src As SafeStringW_NTS)  ' 转移资源，不复制
    Declare Sub Clear()
    
    Declare Operator [](ByVal nIndex As ULong) ByRef As UShort
    
    ' 公共接口（const属性，只读访问）
    Declare Function GetData() As UShort Ptr
    Declare Function GetLength() As ULong
    Declare Function GetRefCount() As UInteger Ptr
End Type

Extern "C"
    Declare Function memcpy(ByVal dest As Any Ptr, ByVal src As Any Ptr, ByVal n As ULong) As Any Ptr
End Extern

' ============================================
' 私有方法实现
' ============================================

Sub SafeStringW_NTS.AllocateString(ByVal nChars As ULong)
    ' 先分离共享引用
    If m_refCount AndAlso *m_refCount > 1 Then
        ( *m_refCount) -= 1
        If *m_refCount = 0 Then  ' 需要检查并释放
            Deallocate(m_pData)
            Delete m_refCount
        End If
        m_refCount = New UInteger(1)
        m_pData = 0 ' 强制分配新内存
        m_capacity = 0
    End If
    
    ' 仅在需要时扩大容量（类似 std::vector）
    If nChars > m_capacity Or m_pData = 0 Then
        Dim newCapacity As ULong = Iif(m_capacity = 0, nChars + 16, m_capacity * 2)
        If newCapacity < nChars Then newCapacity = nChars
        m_pData = Reallocate(m_pData, (newCapacity + 1) * SizeOf(UShort))
        m_capacity = newCapacity
    End If
    
    m_pData[nChars] = 0
    m_nLength = nChars
End Sub

' 新增：手动预留容量
Sub SafeStringW_NTS.Reserve(ByVal nCapacity As ULong)
    If nCapacity > m_capacity Then
        m_pData = Reallocate(m_pData, (nCapacity + 1) * SizeOf(UShort))
        m_capacity = nCapacity
    End If
End Sub

Sub SafeStringW_NTS.Move(ByRef src As SafeStringW_NTS)
    ' 释放自身资源
    If m_refCount AndAlso *m_refCount > 0 Then
        (*m_refCount) -= 1
        If *m_refCount = 0 Then
            Deallocate(m_pData)
            Delete m_refCount
        End If
    End If
    
    ' 直接接管源对象资源（无引用计数增减）
    m_pData = src.m_pData
    m_nLength = src.m_nLength
    m_capacity = src.m_capacity
    m_refCount = src.m_refCount
    
    ' 将源对象置于有效但未分配状态
    src.m_pData = CAllocate(1, SizeOf(UShort))
    src.m_pData[0] = 0
    src.m_nLength = 0
    src.m_capacity = 0
    src.m_refCount = New UInteger(1)
End Sub

Sub SafeStringW_NTS.CopyFromWString(wszStr As WString Ptr)
    If wszStr = 0 Then AllocateString(0) : Exit Sub
    
    Dim strLen As ULong = Len(*wszStr)
    AllocateString(strLen)
    
    If strLen > 0 AndAlso m_pData Then
        memcpy(m_pData, @(*wszStr)[0], (strLen + 1) * SizeOf(UShort))
    End If
End Sub

Sub SafeStringW_NTS.MakeUnique()
    If m_refCount = 0 OrElse *m_refCount <= 1 Then Exit Sub
    
    Dim oldData As UShort Ptr = m_pData
    Dim oldLen As ULong = m_nLength
    
    ' 创建新数据副本
    m_refCount = New UInteger(1)
    m_pData = 0  ' 防止 AllocateString 释放旧数据
    AllocateString(oldLen)
    If oldLen > 0 AndAlso oldData Then
        memcpy(m_pData, oldData, (oldLen + 1) * SizeOf(UShort))
    End If
End Sub

' ============================================
' 构造函数和析构函数
' ============================================

' 创建空字符串
Constructor SafeStringW_NTS()
    m_pData = CAllocate(1, SizeOf(UShort))  ' 始终分配1个字符
    m_pData[0] = 0
    m_nLength = 0
    m_capacity = 0
    m_refCount = New UInteger(1)
End Constructor

' 接收Wstring字符串的初始化
Constructor SafeStringW_NTS(wszStr As Wstring)
    Constructor()
    If Len(wszStr) > 0 Then
        Dim heapStr As WString Ptr = CAllocate((Len(wszStr) + 1), SizeOf(UShort))
        *heapStr = wszStr
        CopyFromWString(heapStr)
        Deallocate(heapStr)
    Else
        AllocateString(0)
    End If
End Constructor

Constructor SafeStringW_NTS(szStr As String)
    Constructor()
    If Len(szStr) > 0 Then
        Dim tmpStr As WString Ptr = CAllocate(Len(szStr) + 1, SizeOf(UShort))
        *tmpStr = szStr
        CopyFromWString(*tmpStr)
        Deallocate(tmpStr)
    End If
End Constructor

Constructor SafeStringW_NTS(ByRef other As SafeStringW_NTS)
    If other.m_refCount Then
        (*other.m_refCount) += 1
    Else
        other.m_refCount = New UInteger(1)
    End If
    
    m_pData = other.m_pData
    m_nLength = other.m_nLength
    m_refCount = other.m_refCount
    m_capacity = other.m_capacity
End Constructor

Destructor SafeStringW_NTS()
    If m_refCount Then
        (*m_refCount) -= 1
        If *m_refCount = 0 Then
            If m_pData Then Deallocate(m_pData)
            Delete m_refCount
            m_pData = 0
            m_refCount = 0
        End If
    End If
End Destructor

' ============================================
' 赋值操作符
' ============================================

Operator SafeStringW_NTS.Let(wszStr As Wstring)
    ' 释放旧数据（你的原代码不变）
    If m_refCount AndAlso *m_refCount > 0 Then
        (*m_refCount) -= 1
        If *m_refCount = 0 Then
            If m_pData Then Deallocate(m_pData)
            Delete m_refCount
        End If
    End If
    
    m_refCount = New UInteger(1)
    m_pData = 0
    
    ' 关键：先堆分配，再传指针
    If Len(wszStr) > 0 Then
        Dim heapStr As WString Ptr = CAllocate((Len(wszStr) + 1), SizeOf(UShort))
        *heapStr = wszStr
        CopyFromWString(heapStr)  ' 传堆指针
        Deallocate(heapStr)
    Else
        m_nLength = 0
    End If
End Operator

Operator SafeStringW_NTS.Let(szStr As String)
    If m_refCount AndAlso *m_refCount > 0 Then
        (*m_refCount) -= 1
        If *m_refCount = 0 Then
            If m_pData Then Deallocate(m_pData)
            Delete m_refCount
        End If
    End If
    
    m_refCount = New UInteger(1)
    m_pData = 0
    
    If Len(szStr) > 0 Then
        Dim tmpStr As WString Ptr = CAllocate(Len(szStr) + 1, SizeOf(UShort))
        *tmpStr = szStr
        CopyFromWString(*tmpStr)
        Deallocate(tmpStr)
    Else
        m_nLength = 0
    End If
End Operator

Operator SafeStringW_NTS.Let(ByRef other As SafeStringW_NTS)
    If @other = @This Then Exit Operator
    
    ' 释放旧数据
    If m_refCount AndAlso *m_refCount > 0 Then
        (*m_refCount) -= 1
        If *m_refCount = 0 Then
            If m_pData Then Deallocate(m_pData)
            Delete m_refCount
        End If
    End If
    
    ' 共享新数据
    If other.m_refCount AndAlso *other.m_refCount > 0 Then
        (*other.m_refCount) += 1
    Else
        other.m_refCount = New UInteger(1)
    End If
    
    m_pData = other.m_pData
    m_nLength = other.m_nLength
    m_refCount = other.m_refCount
    m_capacity = other.m_capacity
End Operator

' ============================================
' 公共访问接口
' ============================================

Function SafeStringW_NTS.GetData() As UShort Ptr
    Return m_pData
End Function

Function SafeStringW_NTS.GetLength() As ULong
    Return m_nLength
End Function

Function SafeStringW_NTS.GetRefCount() As UInteger Ptr
    Return m_refCount
End Function

' ============================================
' 其他公共方法
' ============================================

Function SafeStringW_NTS.Length() As ULong
    Return m_nLength
End Function

Function SafeStringW_NTS.IsEmpty() As Boolean
    Return (m_nLength = 0) OrElse (m_pData = 0) OrElse (m_pData[0] = 0)
End Function

/'Sub SafeStringW_NTS.Clear()
    If m_refCount AndAlso *m_refCount > 1 Then
        (*m_refCount) -= 1
        m_refCount = New UInteger(1)
    End If
    AllocateString(0)
    m_capacity = 0  ' 重置容量
End Sub '/

Sub SafeStringW_NTS.Clear()
    MakeUnique()   ' 必须先确保唯一
    m_nLength = 0
    If m_pData Then m_pData[0] = 0
End Sub

Operator SafeStringW_NTS.Cast() ByRef As Wstring
    Static emptyWStr As WString * 2 = ""  ' 关键：Static 保证生命周期
    If m_pData <> NULL Then
        Return *CPtr(Wstring Ptr, m_pData) ' 返回指针而非解引用
    Else
        Print "[SafeStringW_NTS.Cast]m_pData = NULL"
        Return emptyWStr
    End If
End Operator

Function SafeStringW_NTS.WStr() ByRef As WString
    ' 无需NULL检查，直接返回
    Return *CPtr(Wstring Ptr, m_pData)
End Function

Function SafeStringW_NTS.Str() As String
    If m_pData AndAlso m_nLength > 0 Then
        Return *CPtr(Wstring Ptr, m_pData)
    End If
    Return ""
End Function

Operator SafeStringW_NTS.[](ByVal nIndex As ULong) ByRef As UShort
    Static Zero As UShort = 0
    If m_pData AndAlso nIndex < m_nLength Then
        If m_refCount AndAlso *m_refCount > 1 Then
            MakeUnique()
        End If
        Return m_pData[nIndex]
    Else
        Return Zero
    End If
End Operator

' ============================================
' 全局运算符
' ============================================

Operator & (ByRef lhs As SafeStringW_NTS, ByRef rhs As SafeStringW_NTS) As SafeStringW_NTS
    Dim len1 As ULong = lhs.GetLength()
    Dim len2 As ULong = rhs.GetLength()
    Dim totalLen As ULong = len1 + len2
    
    If totalLen = 0 Then Return SafeStringW_NTS()
    
    Dim result As SafeStringW_NTS
    result.AllocateString(totalLen)
    
    If len1 > 0 AndAlso lhs.GetData() Then 
        memcpy(result.GetData(), lhs.GetData(), len1 * SizeOf(UShort))
    End If
    If len2 > 0 AndAlso rhs.GetData() Then
        memcpy(result.GetData() + len1, rhs.GetData(), (len2 + 1) * SizeOf(UShort))
    End If
    
    Return result
End Operator

Operator & (ByRef lhs As SafeStringW_NTS, rhs As Wstring) As SafeStringW_NTS
    Dim temp As SafeStringW_NTS = rhs
    Return lhs & temp
End Operator

Operator & (lhs As Wstring, ByRef rhs As SafeStringW_NTS) As SafeStringW_NTS
    Dim temp As SafeStringW_NTS = lhs
    Return temp & rhs
End Operator

' ============================================
' Val函数重载
' ============================================

Function Val Overload(ByRef s As SafeStringW_NTS) As Double
    Return .Val(s.WStr())
End Function

Function ValInt Overload(ByRef s As SafeStringW_NTS) As Long
    Return .ValInt(s.WStr())
End Function

Function ValLng Overload(ByRef s As SafeStringW_NTS) As LongInt
    Return .ValLng(s.WStr())
End Function

Function ValUInt Overload(ByRef s As SafeStringW_NTS) As ULong
    Return .ValUInt(s.WStr())
End Function

Function ValULng Overload(ByRef s As SafeStringW_NTS) As ULongInt
    Return .ValULng(s.WStr())
End Function
