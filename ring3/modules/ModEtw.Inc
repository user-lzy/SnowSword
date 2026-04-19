#include "win/guiddef.bi"
#include "win/combaseapi.bi"

' TDH.dll API 枚举ETW提供者
Type TDHSTATUS As ULong
const TDH_SUCCESS = 0
const ERROR_INSUFFICIENT_BUFFER = 122

Declare Function TdhEnumerateProviders Lib "tdh.dll" Alias "TdhEnumerateProviders"(ByVal pBuffer As PVOID, ByVal pBufferSize As PULONG) As TDHSTATUS

' ETW Provider 核心结构体
Type _PROVIDER_INFO
    ProviderGuid As GUID
    SchemaSource As ULong
    ProviderNameOffset As ULong
End Type

Type PROVIDER_ENUMERATION_INFO
    NumberOfProviders As ULong
    reserved As ULong
    ProviderInfo(0 To 0) As _PROVIDER_INFO
end type

'Type EVENT_TRACE_PROPERTIES_
'    Union
'        Wnode As WNODE_HEADER
'        BufferSize As ULONG
'    End Union
'    LoggerNameOffset As ULong
'    LogFileNameOffset As ULong
'    EnableFlags As ULong
'    LogFileMode As ULong
'    BuffersWritten As ULong
'    LogInstanceID As ULong
'    LoggerID As ULong
'    LogHeaderOffset As ULong
'    ProcessorTimeMask As ULong
'    BufferSizeKB As ULong
'    MinimumBuffers As ULong
'    MaximumBuffers As ULong
'    MaximumFileSize As ULong
'    FlushTimer As ULong
'    EnableFlags2 As ULong
'end type
'
'declare function QueryAllTraces lib "advapi32.dll" alias "QueryAllTracesW" ( _
'    byval pSessionList As PTR, _
'    ByVal SessionListSize As ULong, _
'    ByVal pSessionCount As PULONG _
') As ULong
'
'declare function ControlTrace lib "advapi32.dll" alias "ControlTraceW" ( _
'    byval TraceHandle As HANDLE, _
'    byval InstanceName As WSTRING PTR, _
'    byval Properties As EVENT_TRACE_PROPERTIES_ PTR, _
'    ByVal ControlCode As ULong _
') As ULong
'
'Const ERROR_SUCCESS = 0
'Const EVENT_CONTROL_CODE_GET_SESSION = 0

' ==============================================
' 辅助函数：WString 格式转换（适配 TreeView 宽字符）
' ==============================================
Private Function GUIDToWString(ByRef guid As GUID) As StringW
    Dim ws As WString * 40
    StringFromGUID2(@guid, @ws, 40)
    Return ws
End Function

' ==============================================
' 核心函数：枚举系统所有 ETW Provider (对齐你的代码风格)
' ==============================================
Sub GetETWProviderList(lvwEtwProvider As Class_ListView)
    Dim pBuffer As PVOID = NULL
    Dim bufSize As ULONG = 0
    Dim status As TDHSTATUS
    Dim pEnumInfo As PROVIDER_ENUMERATION_INFO Ptr
    Dim i As Integer, count As Integer
    Dim hRoot As HTREEITEM      ' 根节点句柄
    Dim hProvider As HTREEITEM  ' 子节点句柄
    
    ' ======================
    ' 1. 清空 TreeView 原有节点
    ' ======================
    InitListViewData lvwEtwProvider
    
    ' ======================
    ' 3. 获取 ETW Provider 缓冲区大小
    ' ======================
    status = TdhEnumerateProviders(NULL, @bufSize)
    If status <> ERROR_INSUFFICIENT_BUFFER Then
        Print "获取ETW Provider大小失败，错误码："; WHex(status)
        Exit Sub
    End If
    
    ' ======================
    ' 4. 分配内存并读取所有 Provider
    ' ======================
    pBuffer = Allocate(bufSize)
    If pBuffer = NULL Then
        Print "内存分配失败"
        Exit Sub
    End If
    
    pEnumInfo = pBuffer
    status = TdhEnumerateProviders(pBuffer, @bufSize)
    If status <> TDH_SUCCESS Then
        Print "枚举ETW Provider失败，错误码："; Hex(status)
        Deallocate(pBuffer)
        Exit Sub
    End If
    
    ' ======================
    ' 5. 遍历所有 Provider，添加为根节点子项
    ' ======================
    count = pEnumInfo->NumberOfProviders
    If count = 0 Then
        Print "未找到任何ETW Provider"
        Deallocate(pBuffer)
        Exit Sub
    End If
    
    For i = 0 To count - 1
        Dim pInfo As _PROVIDER_INFO Ptr = @pEnumInfo->ProviderInfo(i)
        Dim pwsProviderName As WString Ptr  ' 节点显示文本：Provider名称
        Dim wsGuid As StringW          ' 附加值：GUID字符串
        Dim lGuidParam As Long         ' 附加值（Long类型）
        
        ' 转换数据
        pwsProviderName = pBuffer + pInfo->ProviderNameOffset
        wsGuid = GUIDToWString(pInfo->ProviderGuid)
        
        ' ======================
        ' 关键：调用你的 AddItem 添加子节点
        ' 父项 = 根节点hRoot
        ' 文本 = Provider名称
        ' 附加值 = GUID字符串（可强转为Long存储）
        ' 图标/选中图标 = 0
        ' ======================
        AddItemColListEx lvwEtwProvider,2,,, WStr(wsGuid), *pwsProviderName
    Next
    
    ' ======================
    ' 6. 释放资源
    ' ======================
    Deallocate(pBuffer)
    
    Print "ETW Provider 枚举完成，总数："; count
    CommitListViewView lvwEtwProvider
End Sub

' ==============================================
' 核心函数：根据选中的ETW Provider，枚举关联的ETW Session
' 参数：lvwEtwSession - 你的TreeList控件
'       hSelected - TreeList选中的Provider节点句柄
' 功能：获取选中项GUID → 枚举关联Session → 添加为子节点
' ==============================================
/'Sub GetETWSessionList(lvwEtwSession As Class_mCtrlTreeList, hSelected As MC_HTREELISTITEM)
    Dim hSession As HTREEITEM
    Dim pBuffer As PVOID = NULL
    Dim bufSize As ULONG = 0
    Dim sessionCount As ULONG = 0
    Dim status As ULONG
    Dim i As Integer
    Dim pEnumSessions As TRACE_LOGFILE_HEADER PTR
    Dim pTraceProps As EVENT_TRACE_PROPERTIES_ PTR
    Dim wsProviderGuid As StringW  ' 选中项的Provider GUID
    Dim wsSessionName As WString * 256
    Dim loggerId As ULONG
    
    ' 安全校验：无选中项直接退出
    If hSelected = 0 Then
        Print "未选中任何ETW Provider节点"
        Exit Sub
    End If
    
    ' ======================
    ' 1. 获取选中项的Provider GUID（第0列）
    ' ======================
    wsProviderGuid = lvwEtwSession.GetItemText(hSelected, 0)
    If Len(wsProviderGuid) = 0 Then
        Print "无法获取选中项的Provider GUID"
        Exit Sub
    End If
    
    ' ======================
    ' 2. 清空选中节点下的旧子项
    ' ======================
    TreeList_DeleteAllChildItem lvwEtwSession, hSelected
    
    ' ======================
    ' 3. 获取所有Session的缓冲区大小
    ' ======================
    status = QueryAllTraces(NULL, 0, @sessionCount)
    bufSize = sessionCount * SizeOf(TRACE_LOGFILE_HEADER)
    pBuffer = Allocate(bufSize)
    If pBuffer = NULL Then
        Print "Session内存分配失败"
        Exit Sub
    End If
    
    ' ======================
    ' 4. 枚举系统所有ETW Session
    ' ======================
    status = QueryAllTraces(pBuffer, bufSize, @sessionCount)
    If status <> ERROR_SUCCESS Then
        Print "枚举Session失败，错误码："; WHex(status)
        Deallocate(pBuffer)
        Exit Sub
    End If
    
    pEnumSessions = pBuffer
    ' ======================
    ' 5. 遍历所有Session，匹配关联的Provider
    ' ======================
    For i = 0 To sessionCount - 1
        ' 获取Session LoggerID
        loggerId = pEnumSessions[i].Instance
        
        ' 分配查询Session属性的内存
        Dim propBuf As ZString * 1024
        pTraceProps = Cast(EVENT_TRACE_PROPERTIES_ PTR, @propBuf)
        pTraceProps->BufferSize = 1024
        
        ' 查询Session详细信息
        status = ControlTrace(NULL, Cast(Wstring Ptr, loggerId), pTraceProps, EVENT_CONTROL_CODE_GET_SESSION)
        If status <> ERROR_SUCCESS Then Continue For
        
        ' 获取Session名称
        wsSessionName = Cast(Wstring Ptr, Cast(ULong, pTraceProps) + pTraceProps->LoggerNameOffset)
        
        ' ======================
        ' 6. 添加为选中项的子节点
        ' 列0：LoggerID  列1：会话名称
        ' ======================
        hSession = lvwEtwSession.AddItem(hSelected, "0x" & WHex(loggerId))
        lvwEtwSession.SetItemText(hSession, 1, *wsSessionName)
    Next
    
    ' ======================
    ' 7. 释放资源
    ' ======================
    Deallocate(pBuffer)
    
    Print "ETW Session 枚举完成，Provider GUID："; wsProviderGuid
End Sub'/