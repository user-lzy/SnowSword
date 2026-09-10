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
Type OBJECT_TYPE_INFORMATION Field = 8

    TypeName                    As UNICODE_STRING

    TotalNumberOfObjects        As ULong
    TotalNumberOfHandles        As ULong
    TotalPagedPoolUsage         As ULong
    TotalNonPagedPoolUsage      As ULong
    TotalNamePoolUsage          As ULong
    TotalHandleTableUsage       As ULong

    HighWaterNumberOfObjects    As ULong
    HighWaterNumberOfHandles    As ULong
    HighWaterPagedPoolUsage     As ULong
    HighWaterNonPagedPoolUsage  As ULong
    HighWaterNamePoolUsage      As ULong
    HighWaterHandleTableUsage   As ULong

    InvalidAttributes           As ULong

    GenericMapping              As GENERIC_MAPPING

    ValidAccessMask             As ULong

    SecurityRequired            As UByte
    MaintainHandleCount         As UByte

    TypeIndex                   As UByte
    ReservedByte                As UByte

    PoolType                    As ULong

    DefaultPagedPoolCharge      As ULong
    DefaultNonPagedPoolCharge   As ULong

End Type

Type HANDLE_INFO
    dwProcessId As HANDLE
    dwHandle As HANDLE
    pObject As PVOID
    strType As Wstring * 50
    strName As Wstring * 260
End Type

Type FILE_HANDLE_INFO
    dwProcessId As HANDLE
    dwHandle As HANDLE
    strName As Wstring * 260
End Type

Type HANDLE_QUERY_ENTRY Field = 8
    dwProcessId As HANDLE
    dwHandle As HANDLE

    Status As NTSTATUS

    pObject As PVOID
    
    GrantedAccess As ULong
    HandleFlag As UByte

    strType As Wstring * 64
    strName As Wstring * MAX_PATH
End Type

Type FILE_HANDLE_QUERY_ENTRY
    dwProcessId As HANDLE
    dwHandle As HANDLE
    Status As Long
    strName As Wstring * 260
End Type

' SystemObjectInformation (17) 返回的对象类型信息结构
' Windows x64
Type SYSTEM_OBJECTTYPE_INFORMATION Field = 8
    NextEntryOffset            As ULong          ' +0x00
    TypeIndex                  As ULong          ' +0x04
    NumberOfObjects            As ULong          ' +0x08
    SecurityRequired           As UByte          ' +0x0C
    MaintainHandleCount        As UByte          ' +0x0D
    Reserved                   As UShort         ' +0x0E
    PoolType                   As ULong          ' +0x10
    GenericMapping             As GENERIC_MAPPING ' +0x14
    ValidAccessMask            As ULong          ' +0x24
    DefaultPagedPoolCharge     As ULong          ' +0x28
    DefaultNonPagedPoolCharge  As ULong          ' +0x2C
    TypeName                   As UNICODE_STRING ' +0x30
End Type

' 定义 NT 信息类
Enum OBJECT_INFORMATION_CLASS
    ObjectBasicInformation
    ObjectNameInformation
    ObjectTypeInformation
End Enum

' ===================== Handle Performance Profile =====================

Dim Shared g_HandleProfileEnabled As Boolean = True

Dim Shared g_HandleTotalCount As ULong
Dim Shared g_HandleTypeMatched As ULong
Dim Shared g_HandleIoctlCalled As ULong
Dim Shared g_HandleIoctlSuccess As ULong
Dim Shared g_HandleIoctlEntryCount As ULong
Dim Shared g_HandleIoctlBatchCount As ULong

Dim Shared g_HandleBatchIoctlCalled As ULong
Dim Shared g_HandleBatchEntriesSubmitted As ULong
Dim Shared g_HandleBatchIoctlSuccess As ULong

Dim Shared g_HandleFileMatched As ULong

Dim Shared g_HandlePathConvertCalled As ULong
Dim Shared g_HandlePathConvertSuccess As ULong
Dim Shared g_HandlePathCacheHit As ULong
Dim Shared g_HandlePathCacheMiss As ULong

Dim Shared g_HandleProcessPathCalled As ULong
Dim Shared g_HandleProcessPathCacheHit As ULong
Dim Shared g_HandleProcessPathCacheMiss As ULong
Dim Shared g_HandleUniquePid As ULong

Dim Shared g_HandleListAddCount As ULong
Dim Shared g_HandleDoEventsCount As ULong

Dim Shared g_HandleIoctlTime As Double
Dim Shared g_HandleBatchIoctlTime As Double
Dim Shared g_HandlePathConvertTime As Double
Dim Shared g_HandleProcessPathTime As Double
Dim Shared g_HandleListAddTime As Double
Dim Shared g_HandleDoEventsTime As Double
Dim Shared g_HandleQuerySystemTime As Double
Dim Shared g_HandleCommitTime As Double
Dim Shared g_HandleTotalTime As Double

Private Sub ResetHandleProfile()

    g_HandleTotalCount = 0
    g_HandleTypeMatched = 0
    g_HandleIoctlCalled = 0
    g_HandleIoctlSuccess = 0
    g_HandleFileMatched = 0

    g_HandlePathConvertCalled = 0
    g_HandlePathConvertSuccess = 0
    g_HandlePathCacheHit = 0
    g_HandlePathCacheMiss = 0

    g_HandleProcessPathCalled = 0
    g_HandleProcessPathCacheHit = 0
    g_HandleProcessPathCacheMiss = 0
    g_HandleUniquePid = 0

    g_HandleListAddCount = 0
    g_HandleDoEventsCount = 0

    g_HandleIoctlTime = 0
    g_HandlePathConvertTime = 0
    g_HandleProcessPathTime = 0
    g_HandleListAddTime = 0
    g_HandleDoEventsTime = 0
    g_HandleQuerySystemTime = 0
    g_HandleCommitTime = 0
    g_HandleTotalTime = 0

End Sub

Private Sub PrintHandleProfile(ByVal FunctionName As String)

    If Not g_HandleProfileEnabled Then Return

    MyLog.PrintLog LOG_INFO,,,"========== " & FunctionName & " Performance Profile =========="

    MyLog.PrintLog LOG_INFO,,,"Total Handles              : " & g_HandleTotalCount
    MyLog.PrintLog LOG_INFO,,,"ObjectType Matched         : " & g_HandleTypeMatched

    If g_HandleTotalCount > 0 Then
        MyLog.PrintLog LOG_INFO,,,"Type Match Ratio           : " & _
            Format(g_HandleTypeMatched * 100.0 / g_HandleTotalCount, "0.00") & "%"
    End If

    MyLog.PrintLog LOG_INFO,,, "IOCTL Batch Calls        : " & _
        Str(g_HandleBatchIoctlCalled) & vbCrLf

    MyLog.PrintLog LOG_INFO,,, "IOCTL Entries Submitted  : " & _
        Str(g_HandleBatchEntriesSubmitted) & vbCrLf
    MyLog.PrintLog LOG_INFO,,, "IOCTL Entries Success  : " & _
        Str(g_HandleBatchIoctlSuccess) & vbCrLf
    MyLog.PrintLog LOG_INFO,,,"IOCTL QueryObject Time     : " & _
        Format(g_HandleIoctlTime, "0.000000") & " s"
    MyLog.PrintLog LOG_INFO,,,"IOCTL Batch IOCTL Time     : " & _
        Format(g_HandleBatchIoctlTime, "0.000000") & " s"
        
        If g_HandleIoctlCalled > 0 Then
        MyLog.PrintLog LOG_INFO,,,"IOCTL Batch Average              : " & _
            Format(g_HandleIoctlTime * 1000000.0 / g_HandleIoctlCalled, "0.00") & " us"
    End If

    MyLog.PrintLog LOG_INFO,,,"File Matched               : " & g_HandleFileMatched

    MyLog.PrintLog LOG_INFO,,,"Path Convert Calls         : " & g_HandlePathConvertCalled
    MyLog.PrintLog LOG_INFO,,,"Path Convert Success       : " & g_HandlePathConvertSuccess
    MyLog.PrintLog LOG_INFO,,,"Path Cache Hit             : " & g_HandlePathCacheHit
    MyLog.PrintLog LOG_INFO,,,"Path Cache Miss            : " & g_HandlePathCacheMiss
    MyLog.PrintLog LOG_INFO,,,"Path Convert Time           : " & _
        Format(g_HandlePathConvertTime, "0.000000") & " s"

    If g_HandlePathConvertCalled > 0 Then
        MyLog.PrintLog LOG_INFO,,,"Path Convert Average       : " & _
            Format(g_HandlePathConvertTime * 1000000.0 / g_HandlePathConvertCalled, "0.00") & " us"
    End If

    MyLog.PrintLog LOG_INFO,,,"ProcessPath Calls          : " & g_HandleProcessPathCalled
    MyLog.PrintLog LOG_INFO,,,"ProcessPath Cache Hit       : " & g_HandleProcessPathCacheHit
    MyLog.PrintLog LOG_INFO,,,"ProcessPath Cache Miss      : " & g_HandleProcessPathCacheMiss
    MyLog.PrintLog LOG_INFO,,,"ProcessPath Time            : " & _
        Format(g_HandleProcessPathTime, "0.000000") & " s"

    If g_HandleProcessPathCalled > 0 Then
        MyLog.PrintLog LOG_INFO,,,"ProcessPath Average        : " & _
            Format(g_HandleProcessPathTime * 1000000.0 / g_HandleProcessPathCalled, "0.00") & " us"
    End If

    MyLog.PrintLog LOG_INFO,,,"ListView Add Count         : " & g_HandleListAddCount
    MyLog.PrintLog LOG_INFO,,,"ListView Add Time           : " & _
        Format(g_HandleListAddTime, "0.000000") & " s"

    If g_HandleListAddCount > 0 Then
        MyLog.PrintLog LOG_INFO,,,"ListView Add Average       : " & _
            Format(g_HandleListAddTime * 1000000.0 / g_HandleListAddCount, "0.00") & " us"
    End If

    MyLog.PrintLog LOG_INFO,,,"FF_DoEvents Count          : " & g_HandleDoEventsCount
    MyLog.PrintLog LOG_INFO,,,"FF_DoEvents Time            : " & _
        Format(g_HandleDoEventsTime, "0.000000") & " s"

    MyLog.PrintLog LOG_INFO,,,"ZwQuerySystemInformation    : " & _
        Format(g_HandleQuerySystemTime, "0.000000") & " s"

    MyLog.PrintLog LOG_INFO,,,"CommitListView              : " & _
        Format(g_HandleCommitTime, "0.000000") & " s"

    MyLog.PrintLog LOG_INFO,,,"TOTAL                       : " & _
        Format(g_HandleTotalTime, "0.000000") & " s"

    MyLog.PrintLog LOG_INFO,,,"===================================================="

End Sub

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
        MyLog.PrintWin32Error "CloseProcessHandle", "OpenProcess", "ProcessId=" & ProcessId & " Handle=0x" & WHex(HandleValue)
        Return False
    End If
    Dim hTargetHandle As HANDLE, st As WINBOOL
    '调用 DuplicateHandle 复制句柄
    st = DuplicateHandle(hProcess, HandleValue, GetCurrentProcess, @hTargetHandle, 0, False, DUPLICATE_SAME_ACCESS Or DUPLICATE_CLOSE_SOURCE)
    If (hTargetHandle = 0) Then
        MyLog.PrintWin32Error "CloseProcessHandle", "DuplicateHandle", "ProcessId=" & ProcessId & " Handle=0x" & WHex(HandleValue)
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
        MyLog.PrintWin32Error "GetHandleType", "DuplicateHandle", "0x" & Hex(hHandle)
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
            MyLog.PrintNtError "GetHandleType", "ZwQueryObject", st, "第二次调用"
            Deallocate(pObjInfo)
        End If
    Else
        MyLog.PrintNtError "GetHandleType", "ZwQueryObject", st, "第一次调用"
    End If

    ' 统一清理与失败返回
    ZwClose hProcess
    ZwClose hTargetHandle
    strType[0] = 0
    Return False
End Function

' 获取系统"File"对象的类型索引，仅需调用一次；失败返回0，退化为全量查询
' SystemObjectInformation = 0x11
'Private Function GetFileObjectTypeIndex() As ULong
'
'    Dim bufferSize As ULong = &H10000
'    Dim returnLength As ULong = 0
'    Dim buffer As PVOID = NULL
'    Dim st As NTSTATUS
'    Dim fileTypeIndex As ULong = 0
'
'    ' =========================================================
'    ' 动态查询 SystemObjectInformation
'    ' 注意：
'    ' 第一次查询可能只返回 STATUS_INFO_LENGTH_MISMATCH
'    ' + ReturnLength = 0x40，0x40只是最小记录尺寸，
'    ' 并不是整个 SystemObjectInformation 缓冲区大小。
'    ' =========================================================
'
'    Do
'
'        If buffer <> NULL Then
'            Deallocate(buffer)
'            buffer = NULL
'        End If
'
'        buffer = Allocate(bufferSize)
'
'        If buffer = NULL Then
'            MyLog.PrintLog LOG_ERROR, "GetFileObjectTypeIndex", _
'                "Allocate", "bufferSize=" & bufferSize
'            Return 0
'        End If
'
'        returnLength = 0
'
'        st = ZwQuerySystemInformation( _
'            SystemObjectInformation, _
'            buffer, _
'            bufferSize, _
'            @returnLength)
'
'        If st = STATUS_INFO_LENGTH_MISMATCH Then
'
'            ' 优先使用内核返回的长度
'            If returnLength > bufferSize Then
'                bufferSize = returnLength + &H1000
'            Else
'                bufferSize *= 2
'            End If
'
'            Continue Do
'        End If
'
'        Exit Do
'
'    Loop
'
'    MyLog.PrintLog LOG_INFO, "GetFileObjectTypeIndex", _
'        "ZwQuerySystemInformation", _
'        "status=0x" & Hex(st) & _
'        " bufferSize=" & bufferSize & _
'        " returnLength=" & returnLength
'
'    If Not NT_SUCCESS(st) Then
'
'        MyLog.PrintLog LOG_WARN, "GetFileObjectTypeIndex", _
'            "ZwQuerySystemInformation", _
'            "failed status=0x" & Hex(st) & _
'            " bufferSize=" & bufferSize & _
'            " returnLength=" & returnLength
'
'        If buffer <> NULL Then
'            Deallocate(buffer)
'        End If
'
'        Return 0
'    End If
'
'    ' =========================================================
'    ' 遍历 SYSTEM_OBJECTTYPE_INFORMATION
'    ' =========================================================
'
'    Dim pCur As SYSTEM_OBJECTTYPE_INFORMATION Ptr = _
'        CPtr(SYSTEM_OBJECTTYPE_INFORMATION Ptr, buffer)
'
'    Do
'
'        ' 防止异常数据导致无限循环
'        If pCur = NULL Then Exit Do
'
'        ' TypeName 是 UNICODE_STRING
'        If pCur->TypeName.Buffer <> NULL AndAlso _
'           pCur->TypeName.Length > 0 Then
'
'            Dim typeName As WString * 64 = ""
'
'            Dim copyChars As ULong = pCur->TypeName.Length \ 2
'
'            If copyChars > 63 Then
'                copyChars = 63
'            End If
'
'            If copyChars > 0 Then
'                memcpy( _
'                    @typeName, _
'                    pCur->TypeName.Buffer, _
'                    copyChars * SizeOf(WChar))
'            End If
'
'            typeName[copyChars] = 0
'
'            If lstrcmpW(@typeName, @WStr("File")) = 0 Then
'
'                fileTypeIndex = pCur->TypeIndex
'
'                MyLog.PrintLog LOG_INFO, _
'                    "GetFileObjectTypeIndex", _
'                    "Found", _
'                    "File TypeIndex=" & fileTypeIndex
'
'                Exit Do
'            End If
'
'        End If
'
'        ' 最后一项
'        If pCur->NextEntryOffset = 0 Then
'            Exit Do
'        End If
'
'        ' 安全检查
'        If pCur->NextEntryOffset < SizeOf(SYSTEM_OBJECTTYPE_INFORMATION) Then
'            MyLog.PrintLog LOG_WARN, _
'                "GetFileObjectTypeIndex", _
'                "Invalid NextEntryOffset", _
'                "offset=" & pCur->NextEntryOffset
'            Exit Do
'        End If
'
'        pCur = CPtr( _
'            SYSTEM_OBJECTTYPE_INFORMATION Ptr, _
'            CPtr(UByte Ptr, pCur) + pCur->NextEntryOffset)
'
'    Loop
'
'    ' =========================================================
'    ' 释放缓冲区
'    ' =========================================================
'
'    If buffer <> NULL Then
'        Deallocate(buffer)
'    End If
'
'    Return fileTypeIndex
'
'End Function

Private Function GetFileObjectTypeIndex() As ULong

    Dim hFile As HANDLE = INVALID_HANDLE_VALUE
    Dim tempPath As WString * 260
    Dim tempFile As WString * 260

    Dim buffer As PVOID = NULL
    Dim returnLength As ULong = 0
    Dim bufferSize As ULong = &H400

    Dim st As NTSTATUS
    Dim typeIndex As ULong = 0

    ' =========================================================
    ' 1. 获取临时目录
    ' =========================================================

    If GetTempPathW(260, @tempPath) = 0 Then

        MyLog.PrintLog LOG_ERROR, _
            "GetFileObjectTypeIndex", _
            "GetTempPathW", _
            "failed"

        Return 0
    End If


    ' =========================================================
    ' 2. 创建临时文件名
    ' =========================================================

    If GetTempFileNameW( _
        @tempPath, _
        @WStr("SST"), _
        0, _
        @tempFile) = 0 Then

        MyLog.PrintLog LOG_ERROR, _
            "GetFileObjectTypeIndex", _
            "GetTempFileNameW", _
            "failed"

        Return 0
    End If


    ' =========================================================
    ' 3. 打开文件
    ' =========================================================

    hFile = CreateFileW( _
        @tempFile, _
        GENERIC_READ Or GENERIC_WRITE, _
        FILE_SHARE_READ Or FILE_SHARE_WRITE Or FILE_SHARE_DELETE, _
        NULL, _
        OPEN_EXISTING, _
        FILE_ATTRIBUTE_NORMAL Or FILE_FLAG_DELETE_ON_CLOSE, _
        NULL)

    If hFile = INVALID_HANDLE_VALUE Then

        MyLog.PrintLog LOG_ERROR, _
            "GetFileObjectTypeIndex", _
            "CreateFileW", _
            "failed error=" & GetLastError()

        DeleteFileW(@tempFile)

        Return 0
    End If


    ' =========================================================
    ' 4. 分配 ZwQueryObject 输出缓冲区
    ' =========================================================

    buffer = Allocate(bufferSize)

    If buffer = NULL Then

        MyLog.PrintLog LOG_ERROR, _
            "GetFileObjectTypeIndex", _
            "Allocate", _
            "bufferSize=" & bufferSize

        CloseHandle(hFile)
        DeleteFileW(@tempFile)

        Return 0
    End If


    ' =========================================================
    ' 5. 查询 ObjectTypeInformation
    ' =========================================================

    returnLength = 0
    Dim hNtdll As Any Ptr = GetModuleHandle("ntdll.dll")
    Dim ZwQueryObject As Function(ByVal As HANDLE, ByVal As OBJECT_INFORMATION_CLASS, ByVal As PVOID, ByVal As ULong, ByVal As PULONG) As NTSTATUS = DyLibSymbol(hNtdll, "ZwQueryObject")

    st = ZwQueryObject( _
        hFile, _
        ObjectTypeInformation, _
        buffer, _
        bufferSize, _
        @returnLength)


    MyLog.PrintLog LOG_INFO, _
        "GetFileObjectTypeIndex", _
        "ZwQueryObject", _
        "status=0x" & Hex(st) & _
        " returnLength=" & returnLength


    If Not NT_SUCCESS(st) Then

        MyLog.PrintLog LOG_WARN, _
            "GetFileObjectTypeIndex", _
            "ZwQueryObject", _
            "failed status=0x" & Hex(st) & _
            " returnLength=" & returnLength

        Deallocate(buffer)
        CloseHandle(hFile)
        DeleteFileW(@tempFile)
        

        Return 0
    End If


    ' =========================================================
    ' 6. 读取 TypeIndex
    '
    ' Win8.1+ 内部 OBJECT_TYPE_INFORMATION：
    '
    ' SecurityRequired    +0x58
    ' MaintainHandleCount +0x59
    ' TypeIndex           +0x5A
    ' =========================================================

    Dim pTypeInfo As OBJECT_TYPE_INFORMATION Ptr = _
        CPtr(OBJECT_TYPE_INFORMATION Ptr, buffer)

    typeIndex = pTypeInfo->TypeIndex


    ' =========================================================
    ' 7. 输出 TypeName
    ' =========================================================

    If pTypeInfo->TypeName.Buffer <> NULL AndAlso _
       pTypeInfo->TypeName.Length > 0 Then

        Dim typeName As WString * 64 = ""

        Dim chars As ULong = pTypeInfo->TypeName.Length \ 2

        If chars > 63 Then
            chars = 63
        End If

        If chars > 0 Then

            memcpy( _
                @typeName, _
                pTypeInfo->TypeName.Buffer, _
                chars * SizeOf(WChar))

            typeName[chars] = 0

        End If

        MyLog.PrintLog LOG_INFO, _
            "GetFileObjectTypeIndex", _
            "ObjectType", _
            "Name=" & typeName & _
            " TypeIndex=" & typeIndex

    Else

        MyLog.PrintLog LOG_INFO, _
            "GetFileObjectTypeIndex", _
            "ObjectType", _
            "TypeIndex=" & typeIndex

    End If


    ' =========================================================
    ' 8. 清理
    ' =========================================================

    Deallocate(buffer)

    CloseHandle(hFile)

    ' FILE_FLAG_DELETE_ON_CLOSE 通常已经删除。
    ' 这里再调用一次也无妨。
    DeleteFileW(@tempFile)


    Return typeIndex

End Function

' 同PID进程路径缓存，同一进程仅查询一次
Function GetProcessPathCached(ByVal dwPID As DWORD) As StringW

    Static cachePid() As DWORD
    Static cachePath() As StringW
    Static cacheCnt As Integer = 0

    g_HandleProcessPathCalled += 1

    ' 命中缓存直接返回
    For i As Integer = 0 To cacheCnt - 1
        If cachePid(i) = dwPID Then
            g_HandleProcessPathCacheHit += 1
            Return cachePath(i)
        End If
    Next

    g_HandleProcessPathCacheMiss += 1

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

    g_HandleUniquePid = cacheCnt

    Return path

End Function

Function NtPathToDosPathWFast(ByVal ntPath As LPWSTR, ByVal outDosPath As LPWSTR) As Boolean

#define MAX_CACHE 16

    Static cacheNtPrefix() As WString * MAX_PATH
    Static cacheDosPrefix() As WString * MAX_PATH
    Static cacheCnt As Integer = 0

    If ntPath = 0 OrElse outDosPath = 0 Then Return False

    Dim ntPathLen As Long = lstrlenW(ntPath)
    If ntPathLen = 0 Then Return False

    g_HandlePathConvertCalled += 1

    ' ========== 缓存命中逻辑 ==========
    For i As Integer = 0 To cacheCnt - 1

        Dim prefixLen As Integer = LenW(cacheNtPrefix(i))

        If ntPathLen < prefixLen Then Continue For

        Dim cmpResult As Long = CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, _
            ntPath, prefixLen, _
            @cacheNtPrefix(i), prefixLen)

        If cmpResult = 2 Then

            g_HandlePathCacheHit += 1

            Dim remainPtr As WChar Ptr = ntPath + prefixLen

            lstrcpyW(outDosPath, @cacheDosPrefix(i))
            lstrcpyW(outDosPath + 3, remainPtr)

            g_HandlePathConvertSuccess += 1

            Return True

        End If

    Next

    g_HandlePathCacheMiss += 1

    ' ========== 未命中，调用原函数 ==========
    If NtPathToDosPathW(ntPath, outDosPath) Then

        g_HandlePathConvertSuccess += 1

        Dim ntStr As WString * MAX_PATH = ""
        Dim dosStr As WString * MAX_PATH = ""

        lstrcpyW(@ntStr, ntPath)
        lstrcpyW(@dosStr, outDosPath)

        Dim pos1 As Integer = InStrW(2, ntStr, "\")

        If pos1 = 0 Then Return True

        Dim pos2 As Integer = InStrW(pos1 + 1, ntStr, "\")

        Dim ntPrefix As WString * MAX_PATH = ""

        If pos2 > 0 Then
            ntPrefix = LeftW(ntStr, pos2)
        Else
            ntPrefix = ntStr & "\"
        End If

        If LenW(dosStr) >= 3 AndAlso _
           MidW(dosStr, 2, 1) = ":" AndAlso _
           MidW(dosStr, 3, 1) = "\" Then

            Dim dosPrefix As StringW = LeftW(dosStr, 3)

            If cacheCnt = 0 Then
                ReDim cacheNtPrefix(0 To MAX_CACHE - 1) As Wstring * MAX_PATH
                ReDim cacheDosPrefix(0 To MAX_CACHE - 1) As Wstring * MAX_PATH
            End If

            Dim exist As Boolean = False

            For j As Integer = 0 To cacheCnt - 1

                If LenW(cacheNtPrefix(j)) = LenW(ntPrefix) Then

                    Dim c As Long = CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, _
                        @cacheNtPrefix(j), LenW(ntPrefix), _
                        @ntPrefix, LenW(ntPrefix))

                    If c = 2 Then
                        exist = True
                        Exit For
                    End If

                End If

            Next

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

Const BATCH_SIZE As Long = 512
Public Sub Unlock_File(ListView As Class_ListView)', FilePath As String = "")
    Dim st As NTSTATUS

    ResetHandleProfile()

    Dim tTotalStart As Double = Timer()
    Dim tTemp As Double
    Dim tQueryStart As Double
    Dim tCommitStart As Double
    Dim tStart As Double

    InitListViewData ListView

    Dim bufferSize As ULong = &H1000
    Dim buffer As PVOID = Allocate(bufferSize)

    ' ========== 阶段1：动态扩容缓冲区 + ZwQuerySystemInformation 循环 ==========
    tQueryStart = Timer()

    Do
        bufferSize *= 2
        buffer = Reallocate(buffer, bufferSize)

        st = ZwQuerySystemInformation( _
            SystemHandleInformation, _
            buffer, _
            bufferSize, _
            NULL)

    Loop While st = STATUS_INFO_LENGTH_MISMATCH

    g_HandleQuerySystemTime = Timer() - tQueryStart

    If buffer = NULL Then Return

    If Not NT_SUCCESS(st) Then
        Deallocate(buffer)
        Return
    End If

    ' 注意：保持完整 ULong 类型
    Dim fileTypeIndex As ULong = GetFileObjectTypeIndex()

    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = _
        CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)

    Dim totalHandleCount As ULong = _
        pHandleEntry->NumberOfHandles

    g_HandleTotalCount = totalHandleCount

    Dim lastDoEvents As Double = Timer()

    Dim strHandleName As WString * MAX_PATH
    Dim j As Integer = 0

    Dim DosPath As WString * 260
    Dim ProcessName As StringW
    Dim ProcessPath As StringW

    ' =========================================================
    ' Batch IOCTL 缓冲区
    ' =========================================================

    Const BATCH_SIZE As Long = 512

    Dim batchSize As ULong = _
        SizeOf(ULong) + _
        BATCH_SIZE * SizeOf(FILE_HANDLE_QUERY_ENTRY)

    Dim batchBuffer As UByte Ptr = _
        CPtr(UByte Ptr, Allocate(batchSize))

    If batchBuffer = NULL Then
        Deallocate(buffer)
        Return
    End If

    Dim pBatchCount As ULong Ptr = _
        CPtr(ULong Ptr, batchBuffer)

    Dim pBatch As FILE_HANDLE_QUERY_ENTRY Ptr = _
        CPtr(FILE_HANDLE_QUERY_ENTRY Ptr, _
            batchBuffer + SizeOf(ULong))

    Dim batchCount As Long = 0

    ' ========== 阶段2：遍历全部系统句柄循环 ==========

    For i As ULong = 0 To pHandleEntry->NumberOfHandles - 1

        ' -----------------------------------------------------
        ' 最外层快速过滤
        ' -----------------------------------------------------

        If fileTypeIndex <> 0 AndAlso _
           pHandleEntry->Handles(i).ObjectTypeIndex <> fileTypeIndex Then
            Goto ContinueDo
        End If

        g_HandleTypeMatched += 1

        ' -----------------------------------------------------
        ' 加入 Batch
        ' -----------------------------------------------------

        pBatch[batchCount].dwHandle = _
            Cast(HANDLE, _
                Cast(UInteger, _
                    pHandleEntry->Handles(i).HandleValue))

        pBatch[batchCount].dwProcessId = _
            Cast(HANDLE, _
                Cast(Integer, _
                    pHandleEntry->Handles(i).UniqueProcessId))

        pBatch[batchCount].Status = STATUS_UNSUCCESSFUL
        pBatch[batchCount].strName = ""

        batchCount += 1

        ' -----------------------------------------------------
        ' Batch 满了，执行一次 IOCTL
        ' -----------------------------------------------------

        If batchCount >= BATCH_SIZE Then

            *pBatchCount = batchCount

            g_HandleIoctlCalled += 1

            tStart = Timer()

            Dim ioctlRet As WINBOOL = IoControl( _
                hDrv, _
                IOCTL_QueryFileObjects, _
                batchBuffer, _
                batchSize, _
                batchBuffer, _
                batchSize)

            g_HandleIoctlTime += Timer() - tStart

            If ioctlRet <> 0 Then
                g_HandleIoctlSuccess += 1
            End If

            ' -------------------------------------------------
            ' 处理 Batch 结果
            ' -------------------------------------------------

            For k As Long = 0 To batchCount - 1

                If Not NT_SUCCESS(pBatch[k].Status) Then
                    Continue For
                End If

                If InStrW( _
                    pBatch[k].strName, _
                    "\Device\HarddiskVolume") = 0 Then
                    Continue For
                End If

                g_HandleFileMatched += 1

                ' =============================================
                ' NT Path → DOS Path
                ' =============================================

                tStart = Timer()

                If NtPathToDosPathWFast( _
                    @pBatch[k].strName, _
                    @DosPath) Then

                    strHandleName = DosPath

                Else

                    strHandleName = pBatch[k].strName

                End If

                g_HandlePathConvertTime += _
                    Timer() - tStart

                ' =============================================
                ' ProcessPath
                ' =============================================

                tStart = Timer()

                ProcessPath = _
                    GetProcessPathCached( _
                        Cast(ULong, Cast(ULONG_PTR, pBatch[k].dwProcessId)))

                g_HandleProcessPathTime += _
                    Timer() - tStart

                If InStrW(ProcessPath, ":") <= 0 Then

                    ProcessName = ProcessPath

                Else

                    ProcessName = RightW( _
                        ProcessPath, _
                        LenW(ProcessPath) - _
                        InStrRevW(ProcessPath, "\"))

                End If

                ' =============================================
                ' ListView
                ' =============================================

                tStart = Timer()

                AddItemColListEx ListView, 5,,, _
                    WStr(ProcessName), _
                    WStr(pBatch[k].dwProcessId), _
                    strHandleName, _
                    WStr(ProcessPath), _
                    "0x" & WHex( _
                        Cast(UShort, _
                            Cast(ULONG_PTR, pBatch[k].dwHandle)))

                g_HandleListAddTime += _
                    Timer() - tStart

                g_HandleListAddCount += 1

                j += 1

            Next

            ' -------------------------------------------------
            ' 清空 Batch
            ' -------------------------------------------------

            batchCount = 0

            ' -------------------------------------------------
            ' UI 消息处理
            ' -------------------------------------------------

            If Timer() - lastDoEvents > 0.05 Then

                tStart = Timer()

                FF_DoEvents

                g_HandleDoEventsTime += _
                    Timer() - tStart

                g_HandleDoEventsCount += 1

                lastDoEvents = Timer()

            End If

        End If

ContinueDo:

    Next

    ' =========================================================
    ' 处理最后不足 BATCH_SIZE 的部分
    ' =========================================================

    If batchCount > 0 Then

        *pBatchCount = batchCount
        g_HandleIoctlBatchCount += 1
        g_HandleIoctlEntryCount += batchCount
        g_HandleIoctlCalled += 1

        tStart = Timer()

        Dim ioctlRet As WINBOOL = IoControl( _
            hDrv, _
            IOCTL_QueryFileObjects, _
            batchBuffer, _
            batchSize, _
            batchBuffer, _
            batchSize)

        g_HandleIoctlTime += Timer() - tStart

        If ioctlRet <> 0 Then
            g_HandleIoctlSuccess += 1
        End If

        ' -----------------------------------------------------
        ' 处理最后 Batch
        ' -----------------------------------------------------

        For k As Long = 0 To batchCount - 1
            If Not NT_SUCCESS(pBatch[k].Status) Then
                Continue For
            End If

            If InStrW( _
                pBatch[k].strName, _
                "\Device\HarddiskVolume") = 0 Then
                Continue For
            End If

            g_HandleFileMatched += 1

            ' =============================================
            ' NT Path → DOS Path
            ' =============================================

            tStart = Timer()

            If NtPathToDosPathWFast( _
                @pBatch[k].strName, _
                @DosPath) Then

                strHandleName = DosPath

            Else

                strHandleName = pBatch[k].strName

            End If

            g_HandlePathConvertTime += _
                Timer() - tStart

            ' =============================================
            ' ProcessPath
            ' =============================================

            tStart = Timer()

            ProcessPath = _
                GetProcessPathCached( _
                    Cast(ULong, Cast(ULONG_PTR, pBatch[k].dwProcessId)))

            g_HandleProcessPathTime += _
                Timer() - tStart

            If InStrW(ProcessPath, ":") <= 0 Then

                ProcessName = ProcessPath

            Else

                ProcessName = RightW( _
                    ProcessPath, _
                    LenW(ProcessPath) - _
                    InStrRevW(ProcessPath, "\"))

            End If

            ' =============================================
            ' ListView
            ' =============================================

            tStart = Timer()

            AddItemColListEx ListView, 5,,, _
                WStr(ProcessName), _
                WStr(pBatch[k].dwProcessId), _
                strHandleName, _
                WStr(ProcessPath), _
                "0x" & WHex( _
                    Cast(UShort, _
                        Cast(ULONG_PTR, pBatch[k].dwHandle)))

            g_HandleListAddTime += _
                Timer() - tStart

            g_HandleListAddCount += 1

            j += 1

        Next

    End If

    ' ========== 释放 Batch 缓冲区 ==========

    If batchBuffer <> NULL Then
        Deallocate(batchBuffer)
    End If

    ' ========== 阶段3：统一提交ListView ==========

    tCommitStart = Timer()

    CommitListViewView ListView

    g_HandleCommitTime = _
        Timer() - tCommitStart

    ' ========== 释放系统句柄缓冲区 ==========

    If buffer <> NULL Then
        Deallocate(buffer)
    End If

    ' ========== 完整耗时 ==========

    g_HandleTotalTime = _
        Timer() - tTotalStart

    ' ========== 输出 Profiling ==========

    PrintHandleProfile "Unlock_File"

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
        'Print bufferSize & " " & Hex(st)
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
        MyLog.PrintWin32Error "GetProcessId", "DuplicateHandle", "dwProcessId=" & dwProcessId & ",dwHandle=0x" & dwHandle
        CloseHandle hProcess
        Return False
    End If
    Dim pbi As PROCESS_BASIC_INFORMATION, ret As DWORD
    st = ZwQueryInformationProcess(hTargetHandle, BasicInformation, @pbi, SizeOf(pbi), ret)
    If Not NT_SUCCESS(st) Then
        MyLog.PrintNtError "GetProcessId", "ZwQueryInformationProcess(ProcessBasicInformation)", st, "dwProcessId=" & dwProcessId & ",dwHandle=0x" & dwHandle
        CloseHandle hProcess
        CloseHandle hTargetHandle
        Return False
    End If
    ProcessId = Cast(DWORD, Cast(UInteger, pbi.UniqueProcessId))
    CloseHandle hProcess
    CloseHandle hTargetHandle
    Return True
End Function

Private Function GetProcessIdOfOwnProcessHandle(ByVal dwHandle As HANDLE, ByRef ProcessId As DWORD) As Boolean
    Dim st As NTSTATUS
    Dim pbi As PROCESS_BASIC_INFORMATION, ret As DWORD
    st = ZwQueryInformationProcess(dwHandle, BasicInformation, @pbi, SizeOf(pbi), ret)
    If Not NT_SUCCESS(st) Then
        MyLog.PrintNtError "GetProcessIdOfOwnProcessHandle", "ZwQueryInformationProcess(ProcessBasicInformation)", st, "ProcessId=" & ProcessId & ",dwHandle=0x" & dwHandle
        Return False
    End If
    ProcessId = Cast(DWORD, Cast(UInteger, pbi.UniqueProcessId))
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
        MyLog.PrintWin32Error "GetThreadIdOfThreadHandle", "DuplicateHandle", "dwProcessId=" & dwProcessId & ",dwHandle=0x" & dwHandle
        CloseHandle hProcess
        Return False
    End If
    Dim tbi As THREAD_BASIC_INFORMATION, ret As DWORD
    st = ZwQueryInformationThread(hTargetHandle, ThreadBasicInformation, @tbi, SizeOf(tbi), ret)
    If Not NT_SUCCESS(st) Then
        MyLog.PrintNtError "GetThreadIdOfThreadHandle", "ZwQueryInformationThread(ThreadBasicInformation)", st, "dwProcessId=" & dwProcessId & ",dwHandle=0x" & dwHandle
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

Sub GetHandleList(ByVal bGlobal As Boolean, ByVal ProcessId As DWORD, lvwHandle As Class_ListView)
    Dim st As NTSTATUS
    ResetHandleProfile()

    Dim tTotalStart As Double = Timer()
    Dim tStart As Double
    Dim tQueryStart As Double
    Dim tCommitStart As Double

    InitListViewData lvwHandle
    
    If Not bGlobal Then
        If lvwHandle.ColumnText(0) = "PID" Then
            lvwHandle.DeleteColumn 0
        End If
    Else
        lvwHandle.InsertColumn 0, "PID",, 150
    End If

    Dim bufferSize As ULong = &H10000
    Dim returnLength As ULong = 0
    Dim buffer As PVOID = Allocate(bufferSize)

    tQueryStart = Timer()
    Do
        returnLength = 0
        st = ZwQuerySystemInformation( _
                SystemHandleInformation, _
                buffer, _
                bufferSize, _
                @returnLength)
        If st = STATUS_INFO_LENGTH_MISMATCH Then
            If returnLength > bufferSize Then
                bufferSize = returnLength + &H10000
            Else
                bufferSize *= 2
            End If
            buffer = Reallocate(buffer, bufferSize)
        Else
            Exit Do
        End If
    Loop
    g_HandleQuerySystemTime = Timer() - tQueryStart

    If buffer = NULL Then Return
    If Not NT_SUCCESS(st) Then
        Deallocate(buffer)
        Return
    End If

    Dim pHandleEntry As SYSTEM_HANDLE_INFORMATION Ptr = CPtr(SYSTEM_HANDLE_INFORMATION Ptr, buffer)
    g_HandleTotalCount = pHandleEntry->NumberOfHandles

    Dim strHandleName As WString * MAX_PATH
    Dim DosPath As WString * 260
    Dim ProcessName As StringW
    Dim ProcessPath As StringW
    Dim dwProcessId As DWORD
    Dim dwThreadId As DWORD

    Dim lastDoEvents As Double = Timer()
    Dim j As Integer = 0

    ' ========== Batch IOCTL 缓冲区 ==========
    Const BATCH_SIZE As Long = 512
    Dim batchTotalSize As ULong = SizeOf(ULong) + BATCH_SIZE * SizeOf(HANDLE_QUERY_ENTRY)
    Dim batchBuffer As UByte Ptr = CPtr(UByte Ptr, Allocate(batchTotalSize))
    If batchBuffer = NULL Then
        Deallocate(buffer)
        Return
    End If

    Dim pBatchCount As ULong Ptr = CPtr(ULong Ptr, batchBuffer)
    Dim pBatchEntry As HANDLE_QUERY_ENTRY Ptr = CPtr(HANDLE_QUERY_ENTRY Ptr, batchBuffer + SizeOf(ULong))
    Dim batchCount As Long = 0
    Dim doEventsCounter As Long = 0

    ' ========== 遍历系统句柄 ==========
    For i As ULong = 0 To pHandleEntry->NumberOfHandles - 1
        ' 过滤：只保留目标进程句柄
        If (Not bGlobal) AndAlso pHandleEntry->Handles(i).UniqueProcessId <> ProcessId Then
            Continue For
        End If

        ' 仅驱动加载时使用批量IOCTL
        If IsDriverLoaded Then
            ' 填充Batch条目
            pBatchEntry[batchCount].dwProcessId = Cast(HANDLE, Cast(Integer, pHandleEntry->Handles(i).UniqueProcessId))
            pBatchEntry[batchCount].dwHandle = Cast(HANDLE, Cast(UInteger, pHandleEntry->Handles(i).HandleValue))
            pBatchEntry[batchCount].GrantedAccess = pHandleEntry->Handles(i).GrantedAccess
            pBatchEntry[batchCount].HandleFlag = pHandleEntry->Handles(i).HandleFlag
            pBatchEntry[batchCount].Status = STATUS_UNSUCCESSFUL
            pBatchEntry[batchCount].pObject = NULL
            pBatchEntry[batchCount].strType = ""
            pBatchEntry[batchCount].strName = ""

            batchCount += 1

            ' Batch满，执行批量IOCTL
            If batchCount >= BATCH_SIZE Then
                *pBatchCount = batchCount
                g_HandleBatchIoctlCalled += 1
                g_HandleBatchEntriesSubmitted += batchCount
                tStart = Timer()
                Dim ioctlRet As WINBOOL = IoControl( _
                    hDrv, _
                    IOCTL_QueryObjects, _
                    batchBuffer, _
                    batchTotalSize, _
                    batchBuffer, _
                    batchTotalSize)
                g_HandleBatchIoctlTime += Timer() - tStart
                If ioctlRet <> 0 Then
                    g_HandleBatchIoctlSuccess += 1
                End If

                ' 遍历Batch结果，填充ListView快照
                For k As Long = 0 To batchCount - 1
                    Dim pEntry As HANDLE_QUERY_ENTRY Ptr = @pBatchEntry[k]
                    If Not NT_SUCCESS(pEntry->Status) Then
                        Continue For
                    End If

                    strHandleName = ""

                    ' Process / Thread 特殊处理
                    If pEntry->strType = "Process" Then
                        tStart = Timer()
                        If GetProcessIdOfProcessHandle( _
                            Cast(DWORD, Cast(ULONG_PTR, pEntry->dwProcessId)), _
                            pEntry->dwHandle, _
                            dwProcessId) Then
                            ProcessPath = GetProcessPathCached(dwProcessId)
                            If InStrW(ProcessPath, ":") <= 0 Then
                                ProcessName = ProcessPath
                            Else
                                ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                            End If
                            strHandleName = ProcessName & "(" & dwProcessId & ")"
                        End If
                        g_HandleProcessPathTime += Timer() - tStart
                    ElseIf pEntry->strType = "Thread" Then
                        tStart = Timer()
                        If GetThreadIdOfThreadHandle( _
                            Cast(DWORD, Cast(ULONG_PTR, pEntry->dwProcessId)), _
                            pEntry->dwHandle, _
                            dwThreadId, _
                            dwProcessId) Then
                            ProcessPath = GetProcessPathCached(dwProcessId)
                            If InStrW(ProcessPath, ":") <= 0 Then
                                ProcessName = ProcessPath
                            Else
                                ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                            End If
                            strHandleName = ProcessName & "(" & dwProcessId & ")" & dwThreadId
                        End If
                        g_HandleProcessPathTime += Timer() - tStart
                    ElseIf LenW(pEntry->strName) <> 0 Then
                        tStart = Timer()
                        If NtPathToDosPathWFast(@pEntry->strName, @DosPath) Then
                            strHandleName = DosPath
                        Else
                            strHandleName = pEntry->strName
                        End If
                        g_HandlePathConvertTime += Timer() - tStart
                    End If

                    ' AddItemColListEx 写入快照
                    tStart = Timer()
                    If Not bGlobal Then
                        AddItemColListEx lvwHandle, 6,,, _
                            pEntry->strType, _
                            strHandleName, _
                            "0x" & WHex(pEntry->dwHandle), _
                            "0x" & WHex(pEntry->GrantedAccess), _
                            WStr(HandleFlagsToString(pEntry->HandleFlag)), _
                            "0x" & WHex(pEntry->pObject)
                    Else
                        AddItemColListEx lvwHandle, 7,,, _
                            WStr(pEntry->dwProcessId), _
                            pEntry->strType, _
                            strHandleName, _
                            "0x" & WHex(pEntry->dwHandle), _
                            "0x" & WHex(pEntry->GrantedAccess), _
                            WStr(HandleFlagsToString(pEntry->HandleFlag)), _
                            "0x" & WHex(pEntry->pObject)
                    End If
                    g_HandleListAddTime += Timer() - tStart
                    g_HandleListAddCount += 1
                    j += 1
                Next

                batchCount = 0

                ' UI消息泵
                doEventsCounter += 1

                If doEventsCounter >= 2000 Then
                    tStart = Timer()
                    FF_DoEvents
                    g_HandleDoEventsTime += Timer() - tStart
                    g_HandleDoEventsCount += 1
                    doEventsCounter = 0
                End If
            End If
        Else
            ' ===== 驱动未加载，保留原有单条 GetHandleType 逻辑 =====
            Dim MyHandleInfo As HANDLE_INFO
            strHandleName = ""
            tStart = Timer()
            If GetHandleType( _
                ProcessId, _
                Cast(HANDLE, Cast(ULONG_PTR, pHandleEntry->Handles(i).HandleValue)), _
                @MyHandleInfo.strType) Then
                g_HandleIoctlSuccess += 1
            Else
                MyLog.PrintLog LOG_WARN, _
                    "GetHandleList", _
                    "GetHandleType", _
                    "HANDLE=0x" & WHex(pHandleEntry->Handles(i).HandleValue)
            End If
            g_HandleIoctlTime += Timer() - tStart

            ' Process / Thread
            If MyHandleInfo.strType = "Process" Then
                tStart = Timer()
                If GetProcessIdOfProcessHandle( _
                    pHandleEntry->Handles(i).UniqueProcessId, _
                    Cast(HANDLE, CLngInt(pHandleEntry->Handles(i).HandleValue)), _
                    dwProcessId) Then
                    ProcessPath = GetProcessPathCached(dwProcessId)
                    If InStrW(ProcessPath, ":") <= 0 Then
                        ProcessName = ProcessPath
                    Else
                        ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                    End If
                    strHandleName = ProcessName & "(" & dwProcessId & ")"
                End If
                g_HandleProcessPathTime += Timer() - tStart
            ElseIf MyHandleInfo.strType = "Thread" Then
                tStart = Timer()
                If GetThreadIdOfThreadHandle( _
                    pHandleEntry->Handles(i).UniqueProcessId, _
                    Cast(HANDLE, CLngInt(pHandleEntry->Handles(i).HandleValue)), _
                    dwThreadId, _
                    dwProcessId) Then
                    ProcessPath = GetProcessPathCached(dwProcessId)
                    If InStrW(ProcessPath, ":") <= 0 Then
                        ProcessName = ProcessPath
                    Else
                        ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                    End If
                    strHandleName = ProcessName & "(" & dwProcessId & ")" & dwThreadId
                End If
                g_HandleProcessPathTime += Timer() - tStart
            ElseIf LenW(MyHandleInfo.strName) <> 0 Then
                tStart = Timer()
                If NtPathToDosPathWFast(@MyHandleInfo.strName, @DosPath) Then
                    strHandleName = DosPath
                Else
                    strHandleName = MyHandleInfo.strName
                End If
                g_HandlePathConvertTime += Timer() - tStart
            End If

            tStart = Timer()
            If Not bGlobal Then
                AddItemColListEx lvwHandle, 6,,, _
                    MyHandleInfo.strType, _
                    strHandleName, _
                    "0x" & WHex(pHandleEntry->Handles(i).HandleValue), _
                    "0x" & WHex(pHandleEntry->Handles(i).GrantedAccess), _
                    WStr(HandleFlagsToString(pHandleEntry->Handles(i).HandleFlag)), _
                    "0x" & WHex(pHandleEntry->Handles(i).pObject)
            Else
                AddItemColListEx lvwHandle, 7,,, _
                    WStr(MyHandleInfo.dwProcessId), _
                    MyHandleInfo.strType, _
                    strHandleName, _
                    "0x" & WHex(pHandleEntry->Handles(i).HandleValue), _
                    "0x" & WHex(pHandleEntry->Handles(i).GrantedAccess), _
                    WStr(HandleFlagsToString(pHandleEntry->Handles(i).HandleFlag)), _
                    "0x" & WHex(pHandleEntry->Handles(i).pObject)
            End If
            g_HandleListAddTime += Timer() - tStart
            g_HandleListAddCount += 1
            j += 1

            ' 时间节流 DoEvents
            If Timer() - lastDoEvents > 0.05 Then
                tStart = Timer()
                FF_DoEvents
                g_HandleDoEventsTime += Timer() - tStart
                g_HandleDoEventsCount += 1
                lastDoEvents = Timer()
            End If
        End If
    Next

    ' ========== 处理剩余不足 BATCH_SIZE 的尾批（驱动加载分支） ==========
    If IsDriverLoaded AndAlso batchCount > 0 Then
        *pBatchCount = batchCount
        g_HandleIoctlCalled += 1
        tStart = Timer()
        Dim ioctlRet As WINBOOL = IoControl( _
            hDrv, _
            IOCTL_QueryObjects, _
            batchBuffer, _
            batchTotalSize, _
            batchBuffer, _
            batchTotalSize)
        g_HandleIoctlTime += Timer() - tStart
        If ioctlRet <> 0 Then
            g_HandleIoctlSuccess += 1
        End If

        For k As Long = 0 To batchCount - 1
            Dim pEntry As HANDLE_QUERY_ENTRY Ptr = @pBatchEntry[k]
            If Not NT_SUCCESS(pEntry->Status) Then
                Continue For
            End If
            strHandleName = ""

            If pEntry->strType = "Process" Then
                tStart = Timer()
                If GetProcessIdOfProcessHandle( _
                    Cast(DWORD, Cast(ULONG_PTR, pEntry->dwProcessId)), _
                    pEntry->dwHandle, _
                    dwProcessId) Then
                    ProcessPath = GetProcessPathCached(dwProcessId)
                    If InStrW(ProcessPath, ":") <= 0 Then
                        ProcessName = ProcessPath
                    Else
                        ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                    End If
                    strHandleName = ProcessName & "(" & dwProcessId & ")"
                End If
                g_HandleProcessPathTime += Timer() - tStart
            ElseIf pEntry->strType = "Thread" Then
                tStart = Timer()
                If GetThreadIdOfThreadHandle( _
                    Cast(DWORD, Cast(ULONG_PTR, pEntry->dwProcessId)), _
                    pEntry->dwHandle, _
                    dwThreadId, _
                    dwProcessId) Then
                    ProcessPath = GetProcessPathCached(dwProcessId)
                    If InStrW(ProcessPath, ":") <= 0 Then
                        ProcessName = ProcessPath
                    Else
                        ProcessName = RightW(ProcessPath, LenW(ProcessPath) - InStrRevW(ProcessPath, "\"))
                    End If
                    strHandleName = ProcessName & "(" & dwProcessId & ")" & dwThreadId
                End If
                g_HandleProcessPathTime += Timer() - tStart
            ElseIf LenW(pEntry->strName) <> 0 Then
                tStart = Timer()
                If NtPathToDosPathWFast(@pEntry->strName, @DosPath) Then
                    strHandleName = DosPath
                Else
                    strHandleName = pEntry->strName
                End If
                g_HandlePathConvertTime += Timer() - tStart
            End If

            tStart = Timer()
            If Not bGlobal Then
                AddItemColListEx lvwHandle, 6,,, _
                    pEntry->strType, _
                    strHandleName, _
                    "0x" & WHex(pEntry->dwHandle), _
                    "0x" & WHex(pEntry->GrantedAccess), _
                    WStr(HandleFlagsToString(pEntry->HandleFlag)), _
                    "0x" & WHex(pEntry->pObject)
            Else
                AddItemColListEx lvwHandle, 7,,, _
                    WStr(pEntry->dwProcessId), _
                    pEntry->strType, _
                    strHandleName, _
                    "0x" & WHex(pEntry->dwHandle), _
                    "0x" & WHex(pEntry->GrantedAccess), _
                    WStr(HandleFlagsToString(pEntry->HandleFlag)), _
                    "0x" & WHex(pEntry->pObject)
            End If
            g_HandleListAddTime += Timer() - tStart
            g_HandleListAddCount += 1
            j += 1
        Next
    End If

    ' ========== 释放Batch缓冲区 ==========
    If batchBuffer <> NULL Then
        Deallocate(batchBuffer)
    End If

    ' ========== Commit ==========
    tCommitStart = Timer()
    CommitListViewView lvwHandle
    g_HandleCommitTime = Timer() - tCommitStart

    If buffer <> NULL Then Deallocate(buffer)
    g_HandleTotalTime = Timer() - tTotalStart

    PrintHandleProfile "GetHandleList"
End Sub