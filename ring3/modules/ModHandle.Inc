'#include windows.bi
'#include <winternl.h>
#include "crt/string.bi"
#include "win/winerror.bi"

Type SYSTEM_HANDLE_TABLE_ENTRY_INFO
    UniqueProcessId As ULong
    ObjectTypeIndex As Byte
    HandleFlag As Byte
    HandleValue As UShort
    pObject As PVOID
    GrantedAccess As ACCESS_MASK
End Type

Type SYSTEM_HANDLE_INFORMATION
    NumberOfHandles As ULong
    Handles(0) As SYSTEM_HANDLE_TABLE_ENTRY_INFO
End Type

' 定义对象类型信息结构
Type OBJECT_TYPE_INFORMATION
    TypeName As UNICODE_STRING
    ObjectCount As Long
    HandleCount As Long
    Reserved1 As Long
    Reserved2 As Long
    Reserved3 As Long
    Reserved4 As Long
    Reserved5 As Long
    Reserved6 As Long
    MaintainTypeList As UShort
End Type

Type HANDLE_INFO
    dwProcessId As HANDLE
    dwHandle As HANDLE
    pObject As PVOID
    strType As Wstring * 50
    strName As Wstring * 260
End Type

' SystemObjectInformation (17) 返回的对象类型信息结构
Type SYSTEM_OBJECTTYPE_INFORMATION Field = 8
    NextEntryOffset            As ULong          ' 下一项的字节偏移，为0表示最后一项
    NumberOfObjects            As ULong          ' 该类型的内核对象实例总数
    NumberOfHandles            As ULong          ' 该类型的全局打开句柄总数
    TypeIndex                  As ULong          ' 对象类型全局索引
    InvalidAttributes          As ULong          ' 无效属性掩码
    GenericMapping             As GENERIC_MAPPING ' 通用权限到具体权限的映射表
    ValidAccessMask            As ULong          ' 该类型支持的有效访问权限掩码
    SecurityRequired           As UByte          ' 创建对象是否必须附带安全描述符（BOOLEAN，1字节）
    MaintainHandleCount        As UByte          ' 是否维护句柄引用计数（BOOLEAN，1字节）
    Reserved(1)                As UByte          ' 对齐保留字段（2字节）
    PoolType                   As ULong          ' 使用的内存池类型（POOL_TYPE）
    DefaultPagedPoolCharge     As ULong          ' 默认分页池内存配额
    DefaultNonPagedPoolCharge  As ULong          ' 默认非分页池内存配额
    NameBuffer(0)              As WChar          ' 变长对象类型名称（宽字符柔性数组）
End Type

/'' 定义线程参数结构
Type THREAD_PARAM
    MyHandle As HANDLE
    MyEvent As HANDLE
    IsUnlockFile As BOOLEAN
End Type'/

' 定义 NT 状态码
'#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

' 定义 NT 信息类
Enum OBJECT_INFORMATION_CLASS
    ObjectBasicInformation
    ObjectNameInformation
    ObjectTypeInformation
End Enum
    
' 全局变量
/'Dim Shared gResults() As HANDLE_INFO
Dim Shared gMutexHandle As HANDLE
Dim Shared gResultCount As Long

' 常量定义
Const MUTEX_NAME As String = "Global\SnowSword_QueryHandle_Mutex"

Sub InitializeGlobals()
    ' 初始化结果数组
    ReDim gResults(0) As HANDLE_INFO
    gResultCount = 0

    ' 创建互斥体
    gMutexHandle = CreateMutex(0, False, MUTEX_NAME)
    If gMutexHandle = 0 Then
        AfxMsg "Failed to create mutex: " & GetLastError
        Exit Sub
    End If
End Sub

' 线程函数
Function QueryHandleInfoThread(ByVal lpParam As LPVOID) As DWORD
    Dim param As THREAD_PARAM Ptr = CPtr(THREAD_PARAM Ptr, lpParam)
    Dim MyHandle As HANDLE = param->MyHandle
    Dim MyEvent As HANDLE = param->MyEvent
    Dim HandleInfo As HANDLE_INFO
    HandleInfo.Value = MyHandle

    ' 获取 NtQueryObject 函数指针
    Dim hNtdll As Any Ptr = DyLibLoad("ntdll")
    Dim ZwQueryObject As Function(ByVal As HANDLE, ByVal As OBJECT_INFORMATION_CLASS, ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS = DyLibSymbol(hNtdll, "ZwQueryObject")

    Dim returnLength As ULong
    Dim st As NTSTATUS
    
    Dim typeInfo As OBJECT_TYPE_INFORMATION Ptr = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(&H1000))
    Dim nameInfo As OBJECT_NAME_INFORMATION Ptr
    
    If (typeInfo = NULL) Then
        Print "Failed to allocate memory for type information"
        SetEvent MyEvent
        Return 1
    End If

    st = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, &H1000, @returnLength)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate typeInfo
        typeInfo = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(returnLength))
        If (typeInfo = NULL) Then
            Print "Failed to allocate memory for type information"
            SetEvent MyEvent
            Return 1
        End If
        st = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, returnLength, @returnLength)
    Else
        Print "1Failed to query handle " & MyHandle & " type:" & st
    End If

    If (NT_SUCCESS(st)) Then
        Print "Object Type:" & * (typeInfo->TypeName.Buffer)
        HandleInfo.strType = * (typeInfo->TypeName.Buffer)
        If ((HandleInfo.strType <> "File") And (param->IsUnlockFile)) Then Goto Exit1
    Else
        Print "2Failed to query handle " & MyHandle & " type:" & st
    End If
    
    Deallocate typeInfo

    nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(&H1000))
    If (nameInfo = NULL) Then
        Print "Failed to allocate memory for name information"
        SetEvent MyEvent
        Return 1
    End If

    st = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, &H1000, @returnLength)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate nameInfo
        nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(returnLength))
        If (nameInfo = NULL) Then
            Print "Failed to allocate memory for name information"
            SetEvent MyEvent
            Return 1
        End If
        st = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, returnLength, @returnLength)
    Else
        Print "1Failed to query handle " & MyHandle & " name:" & st
    End If

    If (NT_SUCCESS(st)) Then
        Dim win32Path As WString * MAX_PATH
        If(NtPathToDosPathW(nameInfo->Name.Buffer, win32Path)) Then
            Print "Handle Name: "& win32Path
            HandleInfo.Name = win32Path
        Else
            Print "Failed to convert NT path to Win32 path.NT Path:" & *(nameInfo->Name.Buffer)
        End If
    Else
        Print "2Failed to query handle " & MyHandle & " name:" & st
    End If

    ' 将结果存储到全局数组中
    WaitForSingleObject gMutexHandle, INFINITE
    ReDim Preserve gResults(gResultCount) As HANDLE_INFO
    gResults(gResultCount) = HandleInfo
    gResultCount = gResultCount + 1
    ReleaseMutex gMutexHandle

    Exit1:
    Deallocate typeInfo
    Deallocate nameInfo
    SetEvent MyEvent
End Function

Function QueryHandleInfoThread1(ByVal lpParam As LPVOID) As DWORD
    Dim param As THREAD_PARAM Ptr = CPtr(THREAD_PARAM Ptr, lpParam)
    Dim MyHandle As HANDLE = param->MyHandle
    Dim MyEvent As HANDLE = param->MyEvent

    ' 获取 NtQueryObject 函数指针
    Dim hNtdll As Any Ptr = DyLibLoad("ntdll")
    Dim ZwQueryObject As Function(ByVal As HANDLE, ByVal As OBJECT_INFORMATION_CLASS, ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS = DyLibSymbol(hNtdll, "ZwQueryObject")

    Dim returnLength As ULong
    Dim st As NTSTATUS
    
    Dim typeInfo As OBJECT_TYPE_INFORMATION Ptr = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(&H1000))
    Dim nameInfo As OBJECT_NAME_INFORMATION Ptr
    
    If (typeInfo = NULL) Then
        Print "Failed to allocate memory for type information"
        SetEvent MyEvent
        Return 1
    End If

    st = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, &H1000, @returnLength)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate typeInfo
        typeInfo = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(returnLength))
        If (typeInfo = NULL) Then
            Print "Failed to allocate memory for type information"
            SetEvent MyEvent
            Return 1
        End If
        st = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, returnLength, @returnLength)
    Else
        Print "1Failed to query handle " & MyHandle & " type:" & st
    End If

    If (NT_SUCCESS(st)) Then
        Print "Object Type:" & * (typeInfo->TypeName.Buffer)
    Else
        Print "2Failed to query handle " & MyHandle & " type:" & st
    End If
    
    Deallocate typeinfo

    nameinfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(&H1000))
    If (nameInfo = NULL) Then
        Print "Failed to allocate memory for name information"
        SetEvent MyEvent
        Return 1
    End If

    st = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, &H1000, @returnLength)
    If (st = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate nameInfo
        nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(returnLength))
        If (nameInfo = NULL) Then
            Print "Failed to allocate memory for name information"
            SetEvent MyEvent
            Return 1
        End If
        st = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, returnLength, @returnLength)
    Else
        Print "1Failed to query handle " & MyHandle & " name:" & st
    End If

    If (NT_SUCCESS(st)) Then
        Dim win32Path As WString * MAX_PATH
        If(NtPathToDosPathW(nameInfo->Name.Buffer, win32Path)) Then
            Print "Handle Name: "& win32Path
        Else
            Print "Failed to convert NT path to Win32 path.NT Path:" & *(nameInfo->Name.Buffer)
        End If
    Else
        Print "2Failed to query handle " & MyHandle & " name:" & st
    End If
    
    Exit1:
    Deallocate typeInfo
    Deallocate nameInfo
    SetEvent MyEvent
End Function

Sub UnlockOccupyFile(ByVal wstrFilePath As LPWSTR)
    InitializeGlobals
    Dim hEvents() As HANDLE
    ReDim hEvents(0) As HANDLE
    Dim hThreads() As HANDLE
    ReDim hThreads(0) As HANDLE
    Dim PIDs() As DWORD, ProcessName() As String
    'GetAllPID ProcessName(), PIDs()
    Dim HandleValue As HANDLE
    ReDim PIDs(0)
    PIDs(0) = 19964
    For i As Integer = 0 To UBound(PIDs)
        Dim HandleCount As Long
        Dim hProcess As HANDLE
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_DUP_HANDLE, False, PIDs(i))
        If (hProcess <= 0) Then
            Print "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
            Continue For
        End If
        Dim st As NTSTATUS = ZwQueryInformationProcess(hProcess, ProcessHandleCount, @HandleCount, SizeOf(HandleCount), 0)
        If (st <> 0) Then
            Print "ZwQueryInformationProcess Failed!" & vbCrLf & WinErrorMsg(st)
            Continue For
        End If
        HandleValue = Cast(HANDLE, 4)
        Do While (HandleCount > 0)
            Dim hTargetHandle As HANDLE
            Dim hEvent As HANDLE, param As THREAD_PARAM, hThread As HANDLE, waitResult As DWORD
            If HandleValue > &HC0 Then Exit Do 'Goto ExitFor1
            If HandleValue = &HB4 Then
                Print "Entered!"
                'Exit Do
            End If
            '调用 DuplicateHandle 复制句柄
            Print "0x" & Hex(HandleValue)
            If (DuplicateHandle(hProcess, HandleValue, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0) Then'万一有正确的句柄无法被打开,那就死循环了[捂脸]
                Print "Duplicate Handle 0x" & Hex(HandleValue) & " failed: " & GetLastError
                Goto ExitFor1
            End If
            hEvent = CreateEvent(NULL, True, False, NULL)
            If (hEvent = NULL) Then
                Print "Failed to create event,code:" & GetLastError
                Exit Sub
            End If
            hEvents(UBound(hEvents)) = hEvent
            ReDim hEvents(UBound(hEvents) + 1) As HANDLE

            param.MyHandle = hTargetHandle
            param.MyEvent = hEvent
            param.IsUnlockFile = True
            Print hTargetHandle
            hThread = CreateThread(NULL, 0, @QueryHandleInfoThread, @param, 0, NULL)
            If (hThread = NULL) Then
                Print "Failed to create thread,code:" & GetLastError
                CloseHandle hEvent
                Exit Sub
            End If
            
            hThreads(UBound(hEvents)) = hThread
            ReDim hThreads(UBound(hThreads) + 1) As HANDLE
            
            waitResult = WaitForMultipleObjects(UBound(hEvents) + 1, @hEvents(0), True, 5000) ' 5秒超时
            If (waitResult = WAIT_TIMEOUT) Then
                AfxMsg "Query timed out"
                For i As Integer = 0 To UBound(hThreads)
                    TerminateThread(hThreads(i), 1) ' 强制终止线程
                Next
            ElseIf (waitResult >= WAIT_OBJECT_0) And (waitResult <= WAIT_OBJECT_0 + UBound(hThreads)) Then
                AfxMsg "Query completed successfully"
            Else
                'AfxMsg "Wait failed"
                For i As Integer = 0 To UBound(hThreads)
                    TerminateThread(hThreads(i), 1) ' 强制终止线程
                Next
            End If
            
            For i As Integer = 0 To UBound(hThreads)
                CloseHandle hThreads(i)
                CloseHandle hEvents(i)
            Next
            HandleCount -= 1
            CloseHandle hTargetHandle
            ExitFor1:
            'CloseHandle hProcess
            HandleValue += 4
        Loop
        ReDim hEvents(0) As HANDLE
        ReDim hThreads(0) As HANDLE
        FF_DoEvents
    Next

End Sub

Sub UnlockOccupyFile1(ByVal wstrFilePath As LPWSTR)
    Dim PID As DWORD = 27968
    Dim hProcess As HANDLE, st As NTSTATUS
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_DUP_HANDLE, False, PID)
    If (hProcess <= 0) Then
        Print "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
        Exit Sub
    End If
    Dim hTargetHandle As HANDLE
    Dim hEvent As HANDLE, param As THREAD_PARAM, hThread As HANDLE, waitResult As DWORD
    '调用 DuplicateHandle 复制句柄
    st = DuplicateHandle(hProcess, Cast(HANDLE, &HB8), GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS)
    If (Not NT_SUCCESS(st)) Then'万一有正确的句柄无法被打开,那就死循环了[捂脸]
        Print "Duplicate Handle failed: " & st
        Goto ExitFor1
    End If
    
    hEvent = CreateEvent(NULL, True, False, NULL)
    If (hEvent = NULL) Then
        Print "Failed to create event,code:" & GetLastError
        Exit Sub
    End If

    param.MyHandle = hTargetHandle
    param.MyEvent = hEvent
    param.IsUnlockFile = True
    hThread = CreateThread(NULL, 0, @QueryHandleInfoThread1, @param, 0, NULL)
    If (hThread = NULL) Then
        Print "Failed to create thread,code:" & GetLastError
        CloseHandle hEvent
        Exit Sub
    End If
    
    waitResult = WaitForSingleObject(hEvent, 5000) ' 5秒超时
    If (waitResult = WAIT_TIMEOUT) Then
        AfxMsg "Query timed out"
        TerminateThread(hThread, 1) ' 强制终止线程
    ElseIf (waitResult >= WAIT_OBJECT_0) Then
        AfxMsg "Query completed successfully"
    Else
        AfxMsg "Wait failed"
    End If
    
    CloseHandle hThread
    CloseHandle hEvent
    CloseHandle hTargetHandle
    ExitFor1:
    CloseHandle hProcess

End Sub'/

'DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE

Private Function GetHandleCount(ByVal ProcessId As DWORD, ByVal pHandleCount As LPDWORD) As NTSTATUS
    Dim lpRet As DWORD, hProcess As HANDLE, PID As DWORD = ProcessId
    If IsDriverLoaded Then
        'Return IoControl(hDrv, IOCTL_GetProcessHandleCount, pProcessId, SizeOf(DWORD), pHandleCount, SizeOf(DWORD))
        IoControl hDrv, IOCTL_OpenProcess, @PID, SizeOf(DWORD), @hProcess, SizeOf(HANDLE)
    Else
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, PID)
        'Return ZwQueryInformationProcess(*phProcess, ProcessHandleCount, pHandleCount, SizeOf(DWORD), lpRet)
    End If
    Function = ZwQueryInformationProcess(hProcess, ProcessHandleCount, pHandleCount, SizeOf(DWORD), lpRet)
    ZwClose hProcess
End Function

Private Function CloseProcessHandle(ByVal ProcessId As DWORD, ByVal HandleValue As HANDLE) As Boolean
    Dim hProcess As HANDLE = OpenProcess(PROCESS_DUP_HANDLE, False, ProcessId)
    If (hProcess <= 0) Then
        Print "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
        Return False
    End If
    Dim hTargetHandle As HANDLE, st As WINBOOL
    '调用 DuplicateHandle 复制句柄
    st = DuplicateHandle(hProcess, HandleValue, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS Or DUPLICATE_CLOSE_SOURCE)
    If (hTargetHandle = 0) Then
        Print "DuplicateHandle Failed!" & " " & WinErrorMsg(GetLastError)
        CloseHandle hProcess
        Return False
    Else
        CloseHandle hTargetHandle
        CloseHandle hProcess
        Return True
    End If
End Function

Private Function GetHandleType(ByVal ProcessId As DWORD, ByVal hHandle As HANDLE, ByVal strType As LPWSTR) As Boolean
    Dim ReturnLength As Long
    Dim st As NTSTATUS
    Dim pObjInfo As OBJECT_TYPE_INFORMATION Ptr
    Dim hNtdll As Any Ptr = GetModuleHandle("ntdll.dll")
    Dim ZwQueryObject As Function(ByVal As HANDLE, ByVal As OBJECT_INFORMATION_CLASS, ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS = DyLibSymbol(hNtdll, "ZwQueryObject")

    ' 打开目标进程
    Dim hProcess As HANDLE = NULL
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(ProcessId)
    Else
        hProcess = OpenProcess(PROCESS_DUP_HANDLE, False, ProcessId)
    End If
    If hProcess = NULL Then
        strType[0] = 0
        Return False
    End If

    ' 将句柄复制到本进程上下文
    Dim hTargetHandle As HANDLE
    If DuplicateHandle(hProcess, hHandle, GetCurrentProcess(), @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0 Then
        Print "0x" & Hex(hHandle) & " DuplicateHandle失败: " & WinErrorMsg(GetLastError) & " (" & GetLastError & ")"
        ZwClose hProcess
        strType[0] = 0
        Return False
    End If

    ' 第一次查询：获取所需缓冲区大小
    st = ZwQueryObject(hTargetHandle, ObjectTypeInformation, NULL, 0, @ReturnLength)
    If st = STATUS_INFO_LENGTH_MISMATCH Then
        pObjInfo = Allocate(ReturnLength)
        If pObjInfo = NULL Then
            ZwClose hProcess
            ZwClose hTargetHandle
            strType[0] = 0
            Return False
        End If

        ' 第二次查询：获取类型信息
        st = ZwQueryObject(hTargetHandle, ObjectTypeInformation, pObjInfo, ReturnLength, @ReturnLength)
        If NT_SUCCESS(st) Then
            ' 安全复制类型名称到输出缓冲区
            Dim nameLength As ULong = pObjInfo->TypeName.Length
            Dim copyChars As ULong = nameLength \ 2
            ' 限制最大复制字符数，防止溢出（假设缓冲区至少能容纳 256 个宽字符）
            If copyChars > 255 Then copyChars = 255
            If copyChars > 0 Then
                memcpy(strType, pObjInfo->TypeName.Buffer, copyChars * 2)
            End If
            strType[copyChars] = 0   ' 添加终止空字符

            Deallocate(pObjInfo)
            ZwClose hProcess
            ZwClose hTargetHandle
            Return True
        Else
            Print "第二次 ZwQueryObject 失败, st: 0x" & Hex(st)
            Deallocate(pObjInfo)
        End If
    ElseIf st = &HC0000008 Then   ' STATUS_INVALID_HANDLE
        Print "句柄无效 (STATUS_INVALID_HANDLE)"
    Else
        Print "第一次 ZwQueryObject 失败, st: 0x" & Hex(st)
    End If

    ' 统一清理与失败返回
    ZwClose hProcess
    ZwClose hTargetHandle
    strType[0] = 0
    Return False
End Function

' 获取系统"File"对象的类型索引，仅需调用一次；失败返回0，退化为全量查询
Private Function GetFileObjectTypeIndex() As ULong
    Dim bufNeeded As ULong = &H20000 ' 初始缓冲区大小
    Dim buf As PVOID = Allocate(bufNeeded)
    Dim st As NTSTATUS
    Dim fileTypeIndex As ULong = 0

    ' 扩容循环：第四个参数输出需要的总长度
    Do
        If buf = 0 Then Exit Do ' 分配失败直接退出
        st = ZwQuerySystemInformation(SystemObjectInformation, buf, bufNeeded, @bufNeeded)
        If st <> STATUS_INFO_LENGTH_MISMATCH Then Exit Do
        ' 重新分配更大缓冲区
        Dim newBuf As PVOID = Reallocate(buf, bufNeeded)
        If newBuf = 0 Then Exit Do
        buf = newBuf
    Loop

    ' 查询成功，遍历所有对象类型
    If NT_SUCCESS(st) AndAlso buf <> 0 Then
        Dim pCur As SYSTEM_OBJECTTYPE_INFORMATION Ptr = buf
        Do
            ' 对比名称 L"File"
            If lstrcmpW(@pCur->NameBuffer(0), @WStr("File")) = 0 Then
                fileTypeIndex = pCur->TypeIndex
                Exit Do ' 找到File类型直接跳出遍历
            End If
            ' 无下一项则结束
            If pCur->NextEntryOffset = 0 Then Exit Do
            ' 偏移跳转下一条
            pCur = CPtr(SYSTEM_OBJECTTYPE_INFORMATION Ptr, CPtr(UByte Ptr, pCur) + pCur->NextEntryOffset)
        Loop
    End If

    ' 释放缓冲区
    If buf <> 0 Then Deallocate(buf)
    Return fileTypeIndex
End Function

' 同PID进程路径缓存，同一进程仅查询一次
Function GetProcessPathCached(ByVal dwPID As DWORD) As StringW
    Static cachePid() As DWORD
    Static cachePath() As StringW
    Static cacheCnt As Integer = 0
    
    ' 命中缓存直接返回
    For i As Integer = 0 To cacheCnt - 1
        If cachePid(i) = dwPID Then
            Return cachePath(i)
        End If
    Next
    
    ' 未命中查询并写入缓存
    Dim path As StringW = GetProcessPath(dwPID)
    
    If cacheCnt = 0 Then
        ReDim cachePid(0 To 127)
        ReDim cachePath(0 To 127)
    ElseIf cacheCnt > UBound(cachePid) Then
        ReDim Preserve cachePid(0 To cacheCnt * 2 - 1)
        ReDim Preserve cachePath(0 To cacheCnt * 2 - 1)
    End If
    cachePid(cacheCnt) = dwPID
    cachePath(cacheCnt) = path
    cacheCnt += 1
    
    Return path
End Function

Function NtPathToDosPathWFast(ByVal ntPath As LPWSTR, ByVal outDosPath As LPWSTR) As Boolean
#define MAX_CACHE 16
    Static cacheNtPrefix() As WString * MAX_PATH
    Static cacheDosPrefix() As WString * MAX_PATH
    Static cacheCnt As Integer = 0

    ' 空指针防护
    If ntPath = 0 OrElse outDosPath = 0 Then Return False

    Dim ntPathLen As Long = lstrlenW(ntPath)
    If ntPathLen = 0 Then Return False

    ' ========== 缓存命中逻辑 ==========
    For i As Integer = 0 To cacheCnt - 1
        Dim prefixLen As Integer = LenW(cacheNtPrefix(i))
        ' 长度不足直接跳过
        If ntPathLen < prefixLen Then Continue For
        
        ' 大小写不敏感前缀匹配
        Dim cmpResult As Long = CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, _
            ntPath, prefixLen, _
            @cacheNtPrefix(i), prefixLen)
        
        If cmpResult = 2 Then
            ' 关键：前缀包含末尾反斜杠，剩余路径从反斜杠后开始，避免双反斜杠
            Dim remainPtr As WChar Ptr = ntPath + prefixLen
            ' 先拷贝盘符前缀
            lstrcpyW(outDosPath, @cacheDosPrefix(i))
            ' 再追加剩余路径
            lstrcpyW(outDosPath + 3, remainPtr)
            Return True
        End If
    Next

    ' ========== 未命中，调用原函数 ==========
    If NtPathToDosPathW(ntPath, outDosPath) Then
        ' 正确转换：指针转FB宽字符串，必须用WStr()，绝对不能用*解引用
        Dim ntStr As WString * MAX_PATH = "", dosStr As WString * MAX_PATH = ""
        lstrcpyW(@ntStr, ntPath)
        lstrcpyW(@dosStr, outDosPath)

        ' 提取NT卷前缀（包含末尾反斜杠，如 \Device\HarddiskVolume8\）
        Dim pos1 As Integer = InStrW(2, ntStr, "\")  ' 第二个反斜杠位置
        If pos1 = 0 Then Return True ' 非法NT路径，不缓存
        
        Dim pos2 As Integer = InStrW(pos1 + 1, ntStr, "\") ' 第三个反斜杠位置
        Dim ntPrefix As WString * MAX_PATH = ""
        
        If pos2 > 0 Then
            ' 普通路径：包含第三个反斜杠作为前缀结尾，避免Volume1匹配Volume10
            ntPrefix = LeftW(ntStr, pos2)
        Else
            ' 卷根路径：末尾补反斜杠，统一前缀格式
            ntPrefix = ntStr & "\"
        End If

        ' 仅缓存标准盘符格式（X:\），过滤UNC、网络路径
        If LenW(dosStr) >= 3 AndAlso MidW(dosStr, 2, 1) = ":" AndAlso MidW(dosStr, 3, 1) = "\" Then
            Dim dosPrefix As StringW = LeftW(dosStr, 3)

            ' 首次初始化缓存数组
            If cacheCnt = 0 Then
                ReDim cacheNtPrefix(0 To MAX_CACHE - 1) As Wstring * MAX_PATH
                ReDim cacheDosPrefix(0 To MAX_CACHE - 1) As Wstring * MAX_PATH
            End If

            ' 写入缓存前先去重，避免重复条目
            Dim exist As Boolean = False
            For j As Integer = 0 To cacheCnt - 1
                If LenW(cacheNtPrefix(j)) = LenW(ntPrefix) Then
                    Dim c As Long = CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, _
                        @cacheNtPrefix(j), LenW(ntPrefix), _
                        @ntPrefix, LenW(ntPrefix))
                    If c = 2 Then exist = True : Exit For
                End If
            Next

            ' 未达上限且不重复则写入
            If (Not exist) AndAlso CBool(cacheCnt < MAX_CACHE) Then
                cacheNtPrefix(cacheCnt) = ntPrefix
                cacheDosPrefix(cacheCnt) = dosPrefix
                cacheCnt += 1
            End If
        End If

        Return True
    End If

    Return False
End Function

Public Sub Unlock_File(ListView As Class_ListView)', FilePath As String = "")
    Dim st As NTSTATUS
    ' 全局总计时
    'Dim tTotalStart As Double = Timer()
    
    ' 分段计时变量
    Dim tBufAllocStart As Double, tBufAllocEnd As Double
    Dim tLoopStart As Double, tLoopEnd As Double
    Dim tIoCtlTotal As Double = 0.0
    Dim tPathConvTotal As Double = 0.0
    Dim tGetProcPathTotal As Double = 0.0
    Dim tListViewAddTotal As Double = 0.0
    Dim tDoEventsTotal As Double = 0.0
    Dim tTemp As Double
    
    InitListViewData ListView
    
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    
    ' ========== 阶段1：动态扩容缓冲区 + ZwQuerySystemInformation 循环 ==========
    'tBufAllocStart = Timer()
    Do
        bufferSize *= 2
        buffer = Reallocate(buffer, bufferSize)
        st = ZwQuerySystemInformation(SystemHandleInformation, buffer, bufferSize, NULL)
        Print bufferSize & " " & Hex(st)
    Loop While st = STATUS_INFO_LENGTH_MISMATCH
    'tBufAllocEnd = Timer()
    
    Dim fileTypeIndex As UByte = GetFileObjectTypeIndex
    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    Dim totalHandleCount As ULong = pHandleEntry->NumberOfHandles
    Dim matchFileCount As Integer = 0
    Dim lastDoEvents As Double = Timer()
    
    Dim strHandleName As WString * MAX_PATH, j As Integer
    j = 0
    
    ' ========== 阶段2：遍历全部系统句柄循环 ==========
    tLoopStart = Timer()
    For i As Integer = 0 To pHandleEntry->NumberOfHandles - 1
        Dim MyHandleInfo As HANDLE_INFO
        Dim DosPath As WString * 260, ProcessName As StringW, ProcessPath As StringW
        
        ' 【最外层快速过滤：非文件句柄直接跳过，不执行任何 heavy 操作】
        If fileTypeIndex <> 0 AndAlso pHandleEntry->Handles(i).ObjectTypeIndex <> fileTypeIndex Then
            Goto ContinueDo
        End If
        
        MyHandleInfo.dwHandle = Cast(HANDLE, Cast(UInteger, pHandleEntry->Handles(i).HandleValue))
        MyHandleInfo.dwProcessId = Cast(HANDLE, Cast(Integer, pHandleEntry->Handles(i).UniqueProcessId))
        MyHandleInfo.strName = ""
        MyHandleInfo.strType = ""
        SetLastError 0
        
        ' 1. 驱动IOCTL查询对象信息计时
        'tTemp = Timer()
        IoControl hDrv, IOCTL_QueryObject, @MyHandleInfo, SizeOf(HANDLE_INFO), @MyHandleInfo, SizeOf(HANDLE_INFO)
        'tIoCtlTotal += Timer() - tTemp
        
        If ((MyHandleInfo.strType <> "File") OrElse (InStrW(MyHandleInfo.strName, "\Device\HarddiskVolume") = 0)) Then Goto ContinueDo
        
        ' 2. NT路径转DOS路径计时
        'tTemp = Timer()
        If NtPathToDosPathWFast(@MyHandleInfo.strName, @DosPath) Then
            strHandleName = DosPath
        Else
            strHandleName = MyHandleInfo.strName
        End If
        'tPathConvTotal += Timer() - tTemp
        
        ' 3. 获取进程路径计时
        'tTemp = Timer()
        ProcessPath = GetProcessPathCached(pHandleEntry->Handles(i).UniqueProcessId)
        'tGetProcPathTotal += Timer() - tTemp
        
        If InStrW(ProcessPath, ":") <= 0 Then
            ProcessName = ProcessPath
        Else
            ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
        End If
        
        ' 4. 虚拟列表行填充计时
        'tTemp = Timer()
        AddItemColListEx ListView, 5,,, WStr(ProcessName), WStr(pHandleEntry->Handles(i).UniqueProcessId), strHandleName, WStr(ProcessPath), _
        "0x" & WHex(pHandleEntry->Handles(i).HandleValue)
        'tListViewAddTotal += Timer() - tTemp
        
        matchFileCount += 1
        j += 1
        
        ' 5. FF_DoEvents计时
        tTemp = Timer()
        If Timer() - lastDoEvents > 0.05 Then ' 50ms 执行一次消息循环
            FF_DoEvents
            lastDoEvents = Timer()
        End If
        'tDoEventsTotal += Timer() - tTemp
        
ContinueDo : 
    Next
    'tLoopEnd = Timer()
    
    ' ========== 阶段3：统一提交ListView ==========
    'Dim tCommitStart As Double = Timer()
    CommitListViewView ListView
    'Dim tCommitEnd As Double = Timer()
    
    ' 释放缓冲区
    If buffer <> NULL Then Deallocate(buffer)
    
    ' ========== 输出全量耗时日志 ==========
    'Dim totalAllCost As Double = Timer() - tTotalStart
    'Dim loopTotalCost As Double = tLoopEnd - tLoopStart
    
    /'MyLog.PrintLog LOG_INFO,,,"==== Unlock_File 耗时统计 ===="
    MyLog.PrintLog LOG_INFO,,,"总系统句柄数量：" & totalHandleCount & "，匹配磁盘文件句柄数：" & matchFileCount
    MyLog.PrintLog LOG_INFO,,,"阶段1 缓冲区扩容+ZwQuerySystemInformation 耗时：" & tBufAllocEnd - tBufAllocStart & " 秒"
    MyLog.PrintLog LOG_INFO,,,"阶段2 全句柄遍历循环总耗时：" & loopTotalCost & " 秒"
    MyLog.PrintLog LOG_INFO,,,"  - IoControl查询对象总耗时：" & tIoCtlTotal & " 秒，单句柄平均：" & tIoCtlTotal / totalHandleCount
    MyLog.PrintLog LOG_INFO,,,"  - NtPathToDosPathW转换总耗时：" & tPathConvTotal & " 秒，匹配条目平均：" & IIf(matchFileCount>0, tPathConvTotal/matchFileCount, 0)
    MyLog.PrintLog LOG_INFO,,,"  - GetProcessPath获取进程路径总耗时：" & tGetProcPathTotal & " 秒，匹配条目平均：" & IIf(matchFileCount>0, tGetProcPathTotal/matchFileCount, 0)
    MyLog.PrintLog LOG_INFO,,,"  - AddItemColListEx列表填充总耗时：" & tListViewAddTotal & " 秒，匹配条目平均：" & IIf(matchFileCount>0, tListViewAddTotal/matchFileCount, 0)
    MyLog.PrintLog LOG_INFO,,,"  - FF_DoEvents消息循环总耗时：" & tDoEventsTotal & " 秒，单句柄平均：" & tDoEventsTotal / totalHandleCount
    MyLog.PrintLog LOG_INFO,,,"阶段3 CommitListViewView统一提交耗时：" & tCommitEnd - tCommitStart & " 秒"
    MyLog.PrintLog LOG_INFO,,,"Unlock_File 完整总耗时：" & totalAllCost & " 秒"'/
End Sub

Public Sub Unlock_File1(ListView As Class_ListView)', FilePath As String = "")
    Dim st As NTSTATUS
    
    InitListViewData ListView
    
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    Do
        bufferSize *= 2
        'ReDim buffer(bufferSize - 1) As Byte
        buffer = Reallocate(buffer, bufferSize)
        st = ZwQuerySystemInformation(SystemHandleInformation, buffer, bufferSize, NULL)
        Print bufferSize & " " & Hex(st)
    Loop While st = STATUS_INFO_LENGTH_MISMATCH
    
    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    '下面的代码虽然能拿到handleNum但后续数据全为0
    'memcpy @HandleNum, @buffer(0), 4
    'HandleEntry = Allocate(bufferSize - SizeOf(ULong))
    'ReDim HandleEntry.Handles(HandleNum - 1) As SYSTEM_HANDLE_TABLE_ENTRY_INFO
    'memcpy @HandleEntry(0), @buffer(4), 10 * 16
    
    Dim strHandleName As WString * MAX_PATH, j As Integer
    For i As Integer = 0 To pHandleEntry->NumberOfHandles - 1
        Dim MyHandleInfo As HANDLE_INFO
        Dim DosPath As WString * 260, ProcessName As StringW, ProcessPath As StringW
        MyHandleInfo.dwHandle = Cast(HANDLE, Cast(UInteger, pHandleEntry->Handles(i).HandleValue))
        MyHandleInfo.dwProcessId = Cast(HANDLE, Cast(Integer, pHandleEntry->Handles(i).UniqueProcessId))
        MyHandleInfo.strName = ""
        MyHandleInfo.strType = ""
        SetLastError 0
        IoControl hDrv, IOCTL_QueryObject, @MyHandleInfo, SizeOf(HANDLE_INFO), @MyHandleInfo, SizeOf(HANDLE_INFO)
        'If GetLastError = ERROR_INVALID_HANDLE Then Goto ContinueDo
        'Print PID(i + 1) & "有" & HandleCount & "个句柄" & " 0x" & Hex(dwHandle)
        If ((MyHandleInfo.strType <> "File") OrElse (InStrW(MyHandleInfo.strName, "\Device\HarddiskVolume") = 0)) Then Goto ContinueDo
        If NtPathToDosPathW(@MyHandleInfo.strName, @DosPath) Then
            strHandleName = DosPath
        Else
            strHandleName = MyHandleInfo.strName
        End If
        ProcessPath = GetProcessPath(pHandleEntry->Handles(i).UniqueProcessId)
        If InStrW(ProcessPath, ":") <= 0 Then
            ProcessName = ProcessPath
        Else
            ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
        End If
        'If FilePath <> "" AndAlso InStr(strHandleName, FilePath) <= 0 Then Goto ContinueDo
        'If bInited Then ' 直接AddItemColList
        AddItemColListEx ListView, 5,,, WStr(ProcessName), WStr(pHandleEntry->Handles(i).UniqueProcessId), strHandleName, WStr(ProcessPath), _
        "0x" & WHex(pHandleEntry->Handles(i).HandleValue)
        j += 1
        ContinueDo : 
        FF_DoEvents
    Next
    CommitListViewView ListView
End Sub

Private Function GetProcessIdOfProcessHandle(ByVal dwProcessId As DWORD, ByVal dwHandle As HANDLE, ByRef ProcessId As DWORD) As Boolean
    Dim hProcess As HANDLE, hTargetHandle As HANDLE, st As NTSTATUS
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(dwProcessId)
    Else
        hProcess = OpenProcess(PROCESS_DUP_HANDLE, False, dwProcessId)
    End If
    If hProcess = NULL Then Return False
    If DuplicateHandle(hProcess, dwHandle, GetCurrentProcess, @hTargetHandle, PROCESS_DUP_HANDLE, False, DUPLICATE_SAME_ACCESS) = 0 Then
        Print "[GetProcessId]DuplicateHandle:" & WinErrorMsg(GetLastError) & GetLastError
        CloseHandle hProcess
        Return False
    End If
    Dim pbi As PROCESS_BASIC_INFORMATION, ret As DWORD
    st = ZwQueryInformationProcess(hTargetHandle, BasicInformation, @pbi, SizeOf(pbi), ret)
    If Not NT_SUCCESS(st) Then
        Print "[GetProcessId]ZwQueryInformationProcess:" & Hex(st)
        CloseHandle hProcess
        CloseHandle hTargetHandle
        Return False
    End If
    ProcessId = Cast(DWORD, Cast(UInteger, pbi.UniqueProcessId))
    CloseHandle hProcess
    CloseHandle hTargetHandle
    Return True
End Function

Private Function GetThreadIdOfThreadHandle(ByVal dwProcessId As DWORD, ByVal dwHandle As HANDLE, ByRef ThreadId As DWORD, ByRef ProcessId As DWORD) As Boolean
    Dim hProcess As HANDLE, hTargetHandle As HANDLE, st As NTSTATUS
    If IsDriverLoaded Then
        hProcess = SnowSword_OpenProcess(CLngInt(dwProcessId))
    Else
        hProcess = OpenProcess(PROCESS_DUP_HANDLE, False, dwProcessId)
    End If
    If hProcess = NULL Then Return False
    If DuplicateHandle(hProcess, dwHandle, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0 Then
        Print "[GetThreadId]DuplicateHandle:" & WinErrorMsg(GetLastError) & GetLastError
        CloseHandle hProcess
        Return False
    End If
    Dim tbi As THREAD_BASIC_INFORMATION, ret As DWORD
    st = ZwQueryInformationThread(hTargetHandle, ThreadBasicInformation, @tbi, SizeOf(tbi), ret)
    If Not NT_SUCCESS(st) Then
        Print "[GetThreadId]ZwQueryInformationThread:" & Hex(st)
        CloseHandle hProcess
        CloseHandle hTargetHandle
        Return False
    End If
    ProcessId = Cast(DWORD, Cast(UInteger, tbi.ClientId.UniqueProcess))
    ThreadId = Cast(DWORD, Cast(UInteger, tbi.ClientId.UniqueThread))
    CloseHandle hProcess
    CloseHandle hTargetHandle
    Return True
End Function

Function HandleFlagsToString(ByVal dwHandleFlags As DWORD) As StringW
    Dim As StringW strResult  ' 宽字符串存储结果
    
    ' 检查 可继承 标志
    If (dwHandleFlags And HANDLE_FLAG_INHERIT) <> 0 Then
        strResult = "INHERIT"
    End If
    
    ' 检查 禁止关闭 标志，使用 | 分隔
    If (dwHandleFlags And HANDLE_FLAG_PROTECT_FROM_CLOSE) <> 0 Then
        If strResult <> "" Then 
            strResult += "|"  ' 用竖线分隔多个标志
        End If
        strResult += "PROTECT"
    End If
    
    Function = strResult
End Function

Sub GetHandleList(ByVal ProcessId As DWORD, lvwHandle As Class_ListView)
    Dim st As NTSTATUS
    
    InitListViewData lvwHandle
    
    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)
    Do
        bufferSize *= 2
        'ReDim buffer(bufferSize - 1) As Byte
        buffer = Reallocate(buffer, bufferSize)
        st = ZwQuerySystemInformation(SystemHandleInformation, buffer, bufferSize, NULL)
        Print bufferSize & " " & Hex(st)
    Loop While st = STATUS_INFO_LENGTH_MISMATCH
    
    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    '下面的代码虽然能拿到handleNum但后续数据全为0
    'memcpy @HandleNum, @buffer(0), 4
    'HandleEntry = Allocate(bufferSize - SizeOf(ULong))
    'ReDim HandleEntry.Handles(HandleNum - 1) As SYSTEM_HANDLE_TABLE_ENTRY_INFO
    'memcpy @HandleEntry(0), @buffer(4), 10 * 16
    
    Dim strHandleName As WString * MAX_PATH, j As Integer
    For i As Integer = 0 To pHandleEntry->NumberOfHandles - 1
        If pHandleEntry->Handles(i).UniqueProcessId <> ProcessId Then Continue For
        Dim MyHandleInfo As HANDLE_INFO
        Dim DosPath As WString * 260
        If IsDriverLoaded Then
            MyHandleInfo.dwHandle = Cast(HANDLE, Cast(UInteger, pHandleEntry->Handles(i).HandleValue))
            MyHandleInfo.dwProcessId = Cast(HANDLE, Cast(Integer, pHandleEntry->Handles(i).UniqueProcessId))
            MyHandleInfo.strName = ""
            MyHandleInfo.strType = ""
            SetLastError 0
            IoControl hDrv, IOCTL_QueryObject, @MyHandleInfo, SizeOf(HANDLE_INFO), @MyHandleInfo, SizeOf(HANDLE_INFO)
            'If GetLastError = ERROR_INVALID_HANDLE Then Goto ContinueDo
            'Print PID(i + 1) & "有" & HandleCount & "个句柄" & " 0x" & Hex(dwHandle)
            If NtPathToDosPathW(@MyHandleInfo.strName, @DosPath) Then
                strHandleName = DosPath
            Else
                strHandleName = MyHandleInfo.strName
            End If
        Else
            If Not GetHandleType(ProcessId, Cast(HANDLE, Cast(ULONG_PTR, pHandleEntry->Handles(i).HandleValue)), @MyHandleInfo.strType) Then MyLog.PrintLog LOG_WARN, "GetHandleList", "GetHandleType", "HANDLE=0x" & WHex(pHandleEntry->Handles(i).HandleValue)
        End If
        Dim ProcessName As StringW, ProcessPath As StringW
        Dim dwProcessId As DWORD, dwThreadId As DWORD
        If MyHandleInfo.strType = "Process" Then
            If GetProcessIdOfProcessHandle(pHandleEntry->Handles(i).UniqueProcessId, Cast(HANDLE, CLngInt(pHandleEntry->Handles(i).HandleValue)), dwProcessId) Then
                ProcessPath = GetProcessPath(dwProcessId)
                If InStrW(ProcessPath, ":") <= 0 Then
                    ProcessName = ProcessPath
                Else
                    ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                End If
                strHandleName = ProcessName & "(" & dwProcessId & ")"
            End If
        ElseIf MyHandleInfo.strType = "Thread" Then
            If GetThreadIdOfThreadHandle(pHandleEntry->Handles(i).UniqueProcessId, Cast(HANDLE, CLngInt(pHandleEntry->Handles(i).HandleValue)), dwThreadId, dwProcessId) Then
                ProcessPath = GetProcessPath(dwProcessId)
                If InStrW(ProcessPath, ":") <= 0 Then
                    ProcessName = ProcessPath
                Else
                    ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                End If
                strHandleName = ProcessName & "(" & dwProcessId & ")" & dwThreadId
            End If
        End If
        'If FilePath <> "" AndAlso InStr(strHandleName, FilePath) <= 0 Then Goto ContinueDo
        AddItemColListEx lvwHandle, 6,,, MyHandleInfo.strType, strHandleName, "0x" & WHex(pHandleEntry->Handles(i).HandleValue), _
        "0x" & WHex(pHandleEntry->Handles(i).GrantedAccess), WStr(HandleFlagsToString(pHandleEntry->Handles(i).HandleFlag)), "0x" & WHex(pHandleEntry->Handles(i).pObject)
        j += 1
        ContinueDo : 
        FF_DoEvents
    Next
    CommitListViewView lvwHandle
End Sub