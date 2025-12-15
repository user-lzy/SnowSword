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

/'Private Type PROCESS_HANDLE_TABLE_ENTRY_INFO
    HandleValue As Long
    HandleCount As Long
    PointerCount As Long
    GrantedAccess As Long
    ObjectTypeIndex As Long
    HandleAttributes As Long
    Reserved As Long
End Type
Private Type PROCESS_HANDLE_SNAPSHOT_INFORMATION
    NumberOfHandles As Long
    Handles(0) As PROCESS_HANDLE_TABLE_ENTRY_INFO
End Type'/

' 定义对象基本信息结构
/'Type OBJECT_BASIC_INFORMATION
    Attributes As ULong
    GrantedAccess As ACCESS_MASK
    HandleCount As ULong
    PointerCount As ULong
    PagedPoolUsage As ULong
    NonPagedPoolUsage As ULong
    Reserved(2) As ULong
    NameInformationLength As ULong
    TypeInformationLength As ULong
    SecurityDescriptorLength As ULong
    CreationTime As LARGE_INTEGER
End Type

' 定义对象名称信息结构
Type OBJECT_NAME_INFORMATION
    Name As UNICODE_STRING
End Type'/

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

Private Function CloseProcessHandle(ByVal ProcessId As DWORD, ByVal HandleValue As HANDLE) As BOOLEAN
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

Private Function GetHandleType(ByVal ProcessId As DWORD, ByVal hHandle As HANDLE, ByVal strType As LPWSTR) As BOOLEAN
    Dim ReturnLength As Long
    Dim st As NTSTATUS
    Dim pObjInfo As OBJECT_TYPE_INFORMATION Ptr
    
    ' 获取 NtQueryObject 函数指针
    Dim hNtdll As Any Ptr = GetModuleHandle("ntdll.dll")
    Dim ZwQueryObject As Function(ByVal As HANDLE, ByVal As OBJECT_INFORMATION_CLASS, ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS = DyLibSymbol(hNtdll, "ZwQueryObject")
    
    Dim hProcess As HANDLE = SnowSword_OpenProcess(ProcessId)
    If hProcess = NULL Then
        *strType = ""
        Return False
    End If
    
    '复制句柄
    Dim hTargetHandle As HANDLE
    DuplicateHandle hProcess, hHandle, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS
    If hTargetHandle = NULL Then
        Print "0x" & Hex(hHandle) & "DuplicateHandle失败:" & WinErrorMsg(GetLastError) & GetLastError
        ZwClose hProcess
        *strType = ""
        Return False
    End If
    'ZwClose hTargetHandle
    'Return True
    ' 查询对象类型信息
    st = ZwQueryObject(hHandle, ObjectTypeInformation, NULL, 0, @ReturnLength)
    If st = STATUS_INFO_LENGTH_MISMATCH Then
        pObjInfo = Allocate(ReturnLength)
        st = ZwQueryObject(hHandle, ObjectTypeInformation, pObjInfo, ReturnLength, @ReturnLength)
        If NT_SUCCESS(st) Then
            ZwClose hProcess
            ZwClose hTargetHandle
            *strType = *(pObjInfo->TypeName.buffer)
            Return True
        Else
            Print "第二次ZwQueryObject失败,st:" & Hex(st)
            ZwClose hProcess
            ZwClose hTargetHandle
            *strType = ""
            Return True
        End If
    ElseIf st = Cast(NTSTATUS, &HC0000008) Then ' STATUS_INVALID_HANDLE
        ZwClose hProcess
        ZwClose hTargetHandle
        *strType = ""
        Return False
    Else
        Print "第一次ZwQueryObject失败,st:" & Hex(st)
        ZwClose hProcess
        ZwClose hTargetHandle
        *strType = ""
        Return True
    End If
End Function

Public Sub Unlock_File(ListView As Class_ListView)', FilePath As String = "")
    Dim st As NTSTATUS
    If (Not IsDriverLoaded) AndAlso (AfxMsg("驱动尚未加载,是否加载?",, MB_YESNO) = IDYES) Then
        If Not LoadDriver(App.Path & "SnowSword.sys", False) Then
            AfxMsg "加载失败!"
            Exit Sub
        End If
        hDrv = OpenDrv("\\.\\SnowSword")
        If (hDrv <> INVALID_HANDLE_VALUE) Then
            AfxMsg "加载成功!"
            FrmMain.Check6.Value =True
            IsDriverLoaded = True
        Else
            AfxMsg "加载失败!"
            Exit Sub
        End If
    End If
    
    ListView.DeleteAllItems
    
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
        If ((MyHandleInfo.strType <> "File") AndAlso (InStrW(MyHandleInfo.strName, "\Device\") <> 1)) Then Goto ContinueDo
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
        ListView.AddItem ProcessName
        ListView.SetItemText j, 1, WStr(pHandleEntry->Handles(i).UniqueProcessId)
        ListView.SetItemText j, 2, strHandleName
        ListView.SetItemText j, 3, ProcessPath
        ListView.SetItemText j, 4, "0x" & WHex(pHandleEntry->Handles(i).HandleValue)
        j += 1
        ContinueDo : 
        FF_DoEvents
    Next
End Sub

Private Function GetProcessIdOfProcessHandle(ByVal dwProcessId As DWORD, ByVal dwHandle As HANDLE, ByRef ProcessId As DWORD) As BOOLEAN
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

Private Function GetThreadIdOfThreadHandle(ByVal dwProcessId As DWORD, ByVal dwHandle As HANDLE, ByRef ThreadId As DWORD, ByRef ProcessId As DWORD) As BOOLEAN
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

Sub GetHandleList(ByVal ProcessId As DWORD, lvwHandle As Class_ListView)
    Dim st As NTSTATUS
    lvwHandle.DeleteAllItems
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
        lvwHandle.AddItem MyHandleInfo.strType
        lvwHandle.SetItemText j, 1, strHandleName
        lvwHandle.SetItemText j, 2, "0x" & WHex(pHandleEntry->Handles(i).HandleValue)
        lvwHandle.SetItemText j, 3, "0x" & WHex(pHandleEntry->Handles(i).GrantedAccess)
        lvwHandle.SetItemText j, 4, "0x" & WHex(pHandleEntry->Handles(i).pObject)
        j += 1
        ContinueDo : 
        FF_DoEvents
    Next
End Sub