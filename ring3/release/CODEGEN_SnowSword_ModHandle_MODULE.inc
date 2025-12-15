'[FILE:D:\Programs\VisualFreeBasic6.0\Projects\MyProjects\SnowSword\modules\ModHandle.Inc
'-----------------------------------------------------------------------------
' 由 VisualFreeBasic 5.9.3 生成的源代码
' 生成时间：2025年03月03日 13时08分48秒
' 更多信息请访问 www.yfvb.com 
'-----------------------------------------------------------------------------


'#include windows.bi
'#include <winternl.h>
#include "crt/string.bi"
#include "win/winerror.bi"

' Type SYSTEM_HANDLE_TABLE_ENTRY_INFO '已经搬到定义文件处
'     UniqueProcessId As ULong '已经搬到定义文件处
'     ObjectTypeIndex As Byte '已经搬到定义文件处
'     HandleFlag As Byte '已经搬到定义文件处
'     HandleValue As UShort '已经搬到定义文件处
'     pObject As PVOID '已经搬到定义文件处
'     GrantedAccess As ACCESS_MASK '已经搬到定义文件处
' End Type '已经搬到定义文件处
' Type SYSTEM_HANDLE_INFORMATION '已经搬到定义文件处
'     NumberOfHandles As ULong '已经搬到定义文件处
'     Handles(0) As SYSTEM_HANDLE_TABLE_ENTRY_INFO '已经搬到定义文件处
' End Type '已经搬到定义文件处

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
' Type OBJECT_NAME_INFORMATION '已经搬到定义文件处
'     Name As UNICODE_STRING '已经搬到定义文件处
' End Type '已经搬到定义文件处

' 定义对象类型信息结构
' Type OBJECT_TYPE_INFORMATION '已经搬到定义文件处
'     TypeName As UNICODE_STRING '已经搬到定义文件处
'     ' 其他字段... '已经搬到定义文件处
' End Type'/ '已经搬到定义文件处

' Type HANDLE_INFO '已经搬到定义文件处
'     dwProcessId As DWORD '已经搬到定义文件处
'     dwHandle As DWORD '已经搬到定义文件处
'     pObject As ULONG64 '已经搬到定义文件处
'     strType As Wstring * 20 '已经搬到定义文件处
'     strName As Wstring * 260 '已经搬到定义文件处
' End Type '已经搬到定义文件处

/'' 定义线程参数结构
' Type THREAD_PARAM '已经搬到定义文件处
'     MyHandle As HANDLE '已经搬到定义文件处
'     MyEvent As HANDLE '已经搬到定义文件处
'     IsUnlockFile As BOOLEAN '已经搬到定义文件处
' End Type '已经搬到定义文件处

' 定义 NT 状态码
'#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

' 定义 NT 信息类
' Enum OBJECT_INFORMATION_CLASS '已经搬到定义文件处
'     ObjectBasicInformation '已经搬到定义文件处
'     ObjectNameInformation '已经搬到定义文件处
'     ObjectTypeInformation '已经搬到定义文件处
' End Enum '已经搬到定义文件处
    
' 全局变量
' Dim Shared gResults() As HANDLE_INFO '已经搬到定义文件处
' Dim Shared gMutexHandle As HANDLE '已经搬到定义文件处
' Dim Shared gResultCount As Long '已经搬到定义文件处

' 常量定义
' Const MUTEX_NAME As String = "Global\SnowSword_QueryHandle_Mutex" '已经搬到定义文件处

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
    Dim status As NTSTATUS
    
    Dim typeInfo As OBJECT_TYPE_INFORMATION Ptr = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(&H1000))
    Dim nameInfo As OBJECT_NAME_INFORMATION Ptr
    
    If (typeInfo = NULL) Then
        PrintA "Failed to allocate memory for type information"
        SetEvent MyEvent
        Return 1
    End If

    status = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, &H1000, @returnLength)
    If (Status = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate typeInfo
        typeInfo = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(returnLength))
        If (typeInfo = NULL) Then
            PrintA "Failed to allocate memory for type information"
            SetEvent MyEvent
            Return 1
        End If
        status = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, returnLength, @returnLength)
    Else
        PrintA "1Failed to query handle " & MyHandle & " type:" & status
    End If

    If (NT_SUCCESS(Status)) Then
        PrintA "Object Type:" & * (typeInfo->TypeName.Buffer)
        HandleInfo.strType = * (typeInfo->TypeName.Buffer)
        If ((HandleInfo.strType <> "File") And (param->IsUnlockFile)) Then Goto Exit1
    Else
        PrintA "2Failed to query handle " & MyHandle & " type:" & status
    End If
    
    Deallocate typeInfo

    nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(&H1000))
    If (nameInfo = NULL) Then
        PrintA "Failed to allocate memory for name information"
        SetEvent MyEvent
        Return 1
    End If

    status = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, &H1000, @returnLength)
    If (status = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate nameInfo
        nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(returnLength))
        If (nameInfo = NULL) Then
            PrintA "Failed to allocate memory for name information"
            SetEvent MyEvent
            Return 1
        End If
        status = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, returnLength, @returnLength)
    Else
        PrintA "1Failed to query handle " & MyHandle & " name:" & status
    End If

    If (NT_SUCCESS(Status)) Then
        Dim win32Path As WString * MAX_PATH
        If(NtPathToDosPathW(nameInfo->Name.Buffer, win32Path)) Then
            PrintA "Handle Name: "& win32Path
            HandleInfo.Name = win32Path
        Else
            PrintA "Failed to convert NT path to Win32 path.NT Path:" & *(nameInfo->Name.Buffer)
        End If
    Else
        PrintA "2Failed to query handle " & MyHandle & " name:" & status
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
    Dim status As NTSTATUS
    
    Dim typeInfo As OBJECT_TYPE_INFORMATION Ptr = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(&H1000))
    Dim nameInfo As OBJECT_NAME_INFORMATION Ptr
    
    If (typeInfo = NULL) Then
        PrintA "Failed to allocate memory for type information"
        SetEvent MyEvent
        Return 1
    End If

    status = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, &H1000, @returnLength)
    If (Status = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate typeInfo
        typeInfo = CPtr(OBJECT_TYPE_INFORMATION Ptr, Allocate(returnLength))
        If (typeInfo = NULL) Then
            PrintA "Failed to allocate memory for type information"
            SetEvent MyEvent
            Return 1
        End If
        status = ZwQueryObject(MyHandle, ObjectTypeInformation, typeInfo, returnLength, @returnLength)
    Else
        PrintA "1Failed to query handle " & MyHandle & " type:" & status
    End If

    If (NT_SUCCESS(Status)) Then
        PrintA "Object Type:" & * (typeInfo->TypeName.Buffer)
    Else
        PrintA "2Failed to query handle " & MyHandle & " type:" & status
    End If
    
    Deallocate typeinfo

    nameinfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(&H1000))
    If (nameInfo = NULL) Then
        PrintA "Failed to allocate memory for name information"
        SetEvent MyEvent
        Return 1
    End If

    status = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, &H1000, @returnLength)
    If (status = STATUS_INFO_LENGTH_MISMATCH) Then
        Deallocate nameInfo
        nameInfo = CPtr(OBJECT_NAME_INFORMATION Ptr, Allocate(returnLength))
        If (nameInfo = NULL) Then
            PrintA "Failed to allocate memory for name information"
            SetEvent MyEvent
            Return 1
        End If
        status = ZwQueryObject(MyHandle, ObjectNameInformation, nameInfo, returnLength, @returnLength)
    Else
        PrintA "1Failed to query handle " & MyHandle & " name:" & status
    End If

    If (NT_SUCCESS(Status)) Then
        Dim win32Path As WString * MAX_PATH
        If(NtPathToDosPathW(nameInfo->Name.Buffer, win32Path)) Then
            PrintA "Handle Name: "& win32Path
        Else
            PrintA "Failed to convert NT path to Win32 path.NT Path:" & *(nameInfo->Name.Buffer)
        End If
    Else
        PrintA "2Failed to query handle " & MyHandle & " name:" & status
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
            PrintA "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
            Continue For
        End If
        Dim status As NTSTATUS = ZwQueryInformationProcess(hProcess, ProcessHandleCount, @HandleCount, SizeOf(HandleCount), 0)
        If (status <> 0) Then
            PrintA "ZwQueryInformationProcess Failed!" & vbCrLf & WinErrorMsg(status)
            Continue For
        End If
        HandleValue = Cast(HANDLE, 4)
        Do While (HandleCount > 0)
            Dim hTargetHandle As HANDLE
            Dim hEvent As HANDLE, param As THREAD_PARAM, hThread As HANDLE, waitResult As DWORD
            If HandleValue > &HC0 Then Exit Do 'Goto ExitFor1
            If HandleValue = &HB4 Then
                PrintA "Entered!"
                'Exit Do
            End If
            '调用 DuplicateHandle 复制句柄
            PrintA "0x" & Hex(HandleValue)
            If (DuplicateHandle(hProcess, HandleValue, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS) = 0) Then'万一有正确的句柄无法被打开,那就死循环了[捂脸]
                PrintA "Duplicate Handle 0x" & Hex(HandleValue) & " failed: " & GetLastError
                Goto ExitFor1
            End If
            hEvent = CreateEvent(NULL, True, False, NULL)
            If (hEvent = NULL) Then
                PrintA "Failed to create event,code:" & GetLastError
                Exit Sub
            End If
            hEvents(UBound(hEvents)) = hEvent
            ReDim hEvents(UBound(hEvents) + 1) As HANDLE

            param.MyHandle = hTargetHandle
            param.MyEvent = hEvent
            param.IsUnlockFile = True
            'PrintA hTargetHandle
            hThread = CreateThread(NULL, 0, @QueryHandleInfoThread, @param, 0, NULL)
            If (hThread = NULL) Then
                PrintA "Failed to create thread,code:" & GetLastError
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
    Dim hProcess As HANDLE, status As NTSTATUS
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION Or PROCESS_DUP_HANDLE, False, PID)
    If (hProcess <= 0) Then
        PrintA "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
        Exit Sub
    End If
    Dim hTargetHandle As HANDLE
    Dim hEvent As HANDLE, param As THREAD_PARAM, hThread As HANDLE, waitResult As DWORD
    '调用 DuplicateHandle 复制句柄
    status = DuplicateHandle(hProcess, Cast(HANDLE, &HB8), GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS)
    If (Not NT_SUCCESS(status)) Then'万一有正确的句柄无法被打开,那就死循环了[捂脸]
        PrintA "Duplicate Handle failed: " & status
        Goto ExitFor1
    End If
    
    hEvent = CreateEvent(NULL, True, False, NULL)
    If (hEvent = NULL) Then
        PrintA "Failed to create event,code:" & GetLastError
        Exit Sub
    End If

    param.MyHandle = hTargetHandle
    param.MyEvent = hEvent
    param.IsUnlockFile = True
    hThread = CreateThread(NULL, 0, @QueryHandleInfoThread1, @param, 0, NULL)
    If (hThread = NULL) Then
        PrintA "Failed to create thread,code:" & GetLastError
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

Private Function GetHandleCount(ByVal hProcess As HANDLE, ByVal HandleCount As LPDWORD) As NTSTATUS
    Dim lpRet As DWORD
    Return ZwQueryInformationProcess(hProcess, ProcessHandleCount, HandleCount, SizeOf(DWORD), lpRet)
End Function

Private Sub CloseProcessHandle(ByVal ProcessId As DWORD, ByVal HandleValue As HANDLE)
    Dim hProcess As HANDLE = OpenProcess(PROCESS_DUP_HANDLE, False, ProcessId)
    If (hProcess <= 0) Then
        PrintA "OpenProcess Failed!" & " " & WinErrorMsg(GetLastError)
        Exit Sub
    End If
    Dim hTargetHandle As HANDLE, Status As WINBOOL
    '调用 DuplicateHandle 复制句柄
    Status = DuplicateHandle(hProcess, HandleValue, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS Or DUPLICATE_CLOSE_SOURCE)
    If (hTargetHandle = 0) Then
        PrintA "DuplicateHandle Failed!" & " " & WinErrorMsg(GetLastError)
        CloseHandle hProcess
        Exit Sub
    Else
        CloseHandle hTargetHandle
        CloseHandle hProcess
    End If
End Sub

Public Sub Unlock_File(ListView As Class_ListView)
    Dim MyHandleInfo As HANDLE_INFO, status As NTSTATUS, lpRet As DWORD
    Dim ProcessName() As String, PID() As DWORD, dwHandle As DWORD, j As Integer
    ListView.DeleteAllItems
    GetAllPID(ProcessName(), PID(), 0)
    For i As Integer = 0 To UBound(PID) - 1
        Dim ProcessPath As String, strProcess As String
        ProcessPath = GetProcessPath(PID(i + 1))
        strProcess = GetProcessNameByPath(ProcessPath)
        Dim HandleCount As DWORD, hProcess As HANDLE
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, False, PID(i + 1))
        If (hProcess <= 0) Then
            PrintA "OpenProcess " & PID(i + 1) & " Failed!" & " " & WinErrorMsg(GetLastError)
            Continue For
        End If
        status = GetProcessHandleCount(hProcess, @HandleCount)
        If (status = 0) Then
            PrintA "GetProcessHandleCount " & PID(i + 1) & " Failed!" & " " & status
            Continue For
        End If
        dwHandle = 4
        'PrintA "进程" & PID(i + 1) & "有" & HandleCount & "个句柄"
        Do While (HandleCount > 0)
            Dim MyHandleInfo As HANDLE_INFO
            Dim DosPath As WString * 260
            MyHandleInfo.dwHandle = dwHandle
            MyHandleInfo.dwProcessId = PID(i + 1)
            MyHandleInfo.strName = ""
            MyHandleInfo.strType = ""
            IoControl hDrv, CTL_CODE_GEN(&H860), @MyHandleInfo, SizeOf(MyHandleInfo), @MyHandleInfo, SizeOf(MyHandleInfo), @lpRet
            'PrintA "当前句柄:0x" & Hex(dwHandle)
            If (GetLastError <> ERROR_INVALID_HANDLE) Then
                'If (PID(i + 1) = 1568) Then PrintA WinErrorMsg(GetLastError) & "(" & GetLastError & ")"
                Goto ContinueDo
            End If
            HandleCount -= 1
            'PrintA "HandleCount:" & HandleCount
            'PrintA "当前句柄:0x" & Hex(dwHandle)
            If ((MyHandleInfo.strType <> "File") AndAlso (InStrW(MyHandleInfo.strName, "\Device\") <> 1)) Then Goto ContinueDo
            'PrintA "进程" & PID(i + 1) & "的句柄0x" & Hex(dwHandle) & "有效"
            'If MyHandleInfo.strType <> "" Then PrintA MyHandleInfo.strType
            ListView.AddItem RightW(ProcessName(i + 1), LenW(ProcessName(i + 1)) - InStrRevW(ProcessName(i + 1), "\"))
            ListView.SetItemText j, 1, WStr(PID(i + 1))
            If NtPathToDosPathW(@MyHandleInfo.strName, @DosPath) Then ListView.SetItemText j, 2, DosPath Else ListView.SetItemText j, 2, MyHandleInfo.strName
            ListView.SetItemText j, 3, ProcessName(i + 1)
            ListView.SetItemText j, 4, WStr(dwHandle)
            j += 1
            ContinueDo : 
            FF_DoEvents
            dwHandle += 4
            'If (dwHandle > 2700) Then Exit Do
        Loop
    Next
End Sub