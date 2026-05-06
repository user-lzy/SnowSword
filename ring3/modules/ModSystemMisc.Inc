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
End Sub '/

#include "win/objbase.bi"
#include "win/oleauto.bi"
#include "win/oaidl.bi"

' 全局任务总数统计
Dim Shared totalTaskCount As Long = 0
Type Date As Double

' 任务状态枚举（对应微软官方TASK_STATE定义）
Enum TASK_STATE
    TASK_STATE_UNKNOWN = 0
    TASK_STATE_DISABLED = 1
    TASK_STATE_QUEUED = 2
    TASK_STATE_READY = 3
    TASK_STATE_RUNNING = 4
End Enum

' 适配你环境的正确GUID
Dim Shared CLSID_TaskScheduler As GUID = _
    Type<GUID>(&h0F87369F, &hA4E5, &h4CFC, {&hBD, &h3E, &h73, &hE6, &h15, &h45, &h72, &hDD})
Dim Shared IID_ITaskService As GUID = _
    Type<GUID>(&h2FABA4C7, &h4DA9, &h4013, {&h96, &h97, &h20, &hCC, &h3F, &hD4, &h0F, &h85})

' ==============================================
' 工具函数：COM日期转可读字符串
' ==============================================
Function VariantDateToString(ByVal dateVal As DATE) As String
    Dim st As SYSTEMTIME
    If VariantTimeToSystemTime(dateVal, @st) = 0 Then
        Return "无"
    End If
    ' 格式：年-月-日 时:分:秒
    Return Format(st.wYear, "0000") & "-" & Format(st.wMonth, "00") & "-" & Format(st.wDay, "00") & " " & _
           Format(st.wHour, "00") & ":" & Format(st.wMinute, "00") & ":" & Format(st.wSecond, "00")
End Function

' ==============================================
' 工具函数：任务状态枚举转文本
' ==============================================
Function TaskStateToString(ByVal state As Long) As String
    Select Case state
        Case TASK_STATE_DISABLED : Return "已禁用"
        Case TASK_STATE_QUEUED : Return "已排队"
        Case TASK_STATE_READY : Return "就绪"
        Case TASK_STATE_RUNNING : Return "运行中"
        Case Else : Return "未知"
    End Select
End Function

Function DateToText(ByVal v As VARIANT) As String
    If v.vt <> VT_DATE Then Return "无"
    If v.date = 0 Then Return "无"
    Return VariantDateToString(v.Date)
End Function

Function TaskResultText(ByVal hr As Long) As String
    Select Case hr
        Case &H0
            Return "0x0 (成功)"
        Case &H41301
            Return "0x41301 (运行中)"
        Case &H41306
            Return "0x41306 (无后续运行/状态码)"
        Case Else
            Return "0x" & Hex(hr)
    End Select
End Function

Function BSTRToString(ByVal b As BSTR) As String
    If b = NULL Then Return ""
    Return *Cptr(WString Ptr, b)
End Function

' ==============================================
' 通用Dispatch调用封装
' ==============================================
Function DispatchInvokeGeneric(pDisp As IDispatch Ptr, methodName As String, _
                               ByVal args As Any Ptr, ByVal numArgs As Long, _
                               ByVal wFlags As WORD, ByRef result As VARIANT) As Long
    Dim dispid As Long, hr As Long
    Dim bstrName As BSTR = SysAllocString(WStr(methodName))
    hr = pDisp->lpVtbl->GetIDsOfNames(pDisp, @IID_NULL, @bstrName, 1, LOCALE_USER_DEFAULT, @dispid)
    SysFreeString(bstrName)
    If hr <> 0 Then Return hr

    Dim dp As DISPPARAMS
    dp.rgvarg = IIf(numArgs > 0, args, NULL)
    dp.cArgs = numArgs
    dp.rgdispidNamedArgs = NULL
    dp.cNamedArgs = 0

    hr = pDisp->lpVtbl->Invoke(pDisp, dispid, @IID_NULL, LOCALE_USER_DEFAULT, wFlags, @dp, @result, NULL, NULL)
    Return hr
End Function

' 便捷封装
Function DispatchInvokeMethod(pDisp As IDispatch Ptr, methodName As String, ByVal args As Any Ptr, ByVal numArgs As Long, ByRef result As VARIANT) As Long
    Return DispatchInvokeGeneric(pDisp, methodName, args, numArgs, DISPATCH_METHOD, result)
End Function
Function DispatchInvokeGet(pDisp As IDispatch Ptr, propName As String, ByRef result As VARIANT) As Long
    Return DispatchInvokeGeneric(pDisp, propName, NULL, 0, DISPATCH_PROPERTYGET, result)
End Function
Function DispatchInvokeGetWithArgs(pDisp As IDispatch Ptr, propName As String, ByVal args As Any Ptr, ByVal numArgs As Long, ByRef result As VARIANT) As Long
    Return DispatchInvokeGeneric(pDisp, propName, args, numArgs, DISPATCH_PROPERTYGET, result)
End Function

' ==============================================
' 递归枚举任务
' ==============================================
Sub EnumTasksInFolder(folderDisp As IDispatch Ptr, currentPath As String, lvwTaskScheduler As Class_ListView)
    Dim hr As Long
    Static index As Integer = 0
    
    ' 枚举当前文件夹任务
    Dim tasksVar As VARIANT : VariantInit(@tasksVar)
    Dim taskArg(0) As VARIANT : taskArg(0).vt = VT_I4 : taskArg(0).lVal = 1
    hr = DispatchInvokeMethod(folderDisp, "GetTasks", @taskArg(0), 1, tasksVar)
    
    If hr = 0 And tasksVar.vt = VT_DISPATCH Then
        Dim countVar As VARIANT : VariantInit(@countVar)
        hr = DispatchInvokeGet(tasksVar.pdispVal, "Count", countVar)
        
        If hr = 0 And (countVar.vt = VT_I4 Or countVar.vt = VT_I2) Then
            Dim taskCount As Long = countVar.lVal
            totalTaskCount += taskCount
            If taskCount > 0 Then
                /'Print "=================================================="
                Print "【文件夹】"; currentPath
                Print "【任务数】"; taskCount
                Print "--------------------------------------------------"'/
                
                For i As Long = 1 To taskCount
                    Dim taskVar As VARIANT : VariantInit(@taskVar)
                    Dim idxArg(0) As VARIANT : idxArg(0).vt = VT_I4 : idxArg(0).lVal = i
                    
                    ' ==============================================
                    ' 核心修复：任务集合Item是属性，改用PropertyGet调用
                    ' ==============================================
                    hr = DispatchInvokeGetWithArgs(tasksVar.pdispVal, "Item", @idxArg(0), 1, taskVar)
                    
                    If hr = 0 And taskVar.vt = VT_DISPATCH Then
                        Dim taskDisp As IDispatch Ptr = taskVar.pdispVal
                        
                        ' 1. 获取任务名称
                        Dim nameVar As VARIANT : VariantInit(@nameVar)
                        Dim taskName As String = "未知名称"
                        hr = DispatchInvokeGet(taskDisp, "Name", nameVar)
                        If hr = 0 And nameVar.vt = VT_BSTR And nameVar.bstrVal <> NULL Then
                            taskName = BSTRToString(nameVar.bstrVal)
                        End If
                        VariantClear(@nameVar)
                        
                        ' 2. 获取启用状态
                        Dim enabledVar As VARIANT : VariantInit(@enabledVar)
                        Dim isEnabled As String = "未知"
                        hr = DispatchInvokeGet(taskDisp, "Enabled", enabledVar)
                        If hr = 0 And enabledVar.vt = VT_BOOL Then
                            isEnabled = IIf(enabledVar.boolVal <> 0, "是", "否")
                        End If
                        VariantClear(@enabledVar)
                        
                        ' 3. 获取任务状态
                        Dim stateVar As VARIANT : VariantInit(@stateVar)
                        Dim taskState As String = "未知"
                        hr = DispatchInvokeGet(taskDisp, "State", stateVar)
                        If hr = 0 And (stateVar.vt = VT_I4 Or stateVar.vt = VT_I2) Then
                            taskState = TaskStateToString(stateVar.lVal)
                        End If
                        VariantClear(@stateVar)
                        
                        ' 4. 获取上次运行时间
                        Dim lastRunVar As VARIANT : VariantInit(@lastRunVar)
                        Dim lastRunTime As String = "无"
                        hr = DispatchInvokeGet(taskDisp, "LastRunTime", lastRunVar)
                        If hr = 0 And lastRunVar.vt = VT_DATE Then
                            lastRunTime = DateToText(lastRunVar)
                        End If
                        VariantClear(@lastRunVar)
                        
                        ' 5. 获取下次运行时间
                        Dim nextRunVar As VARIANT : VariantInit(@nextRunVar)
                        Dim nextRunTime As String = "无"
                        hr = DispatchInvokeGet(taskDisp, "NextRunTime", nextRunVar)
                        If hr = 0 And nextRunVar.vt = VT_DATE Then
                            nextRunTime = DateToText(nextRunVar)
                        End If
                        VariantClear(@nextRunVar)
                        
                        ' 6. 获取上次运行结果
                        Dim lastResultVar As VARIANT : VariantInit(@lastResultVar)
                        Dim lastResult As String = "未知"
                        hr = DispatchInvokeGet(taskDisp, "LastTaskResult", lastResultVar)
                        If hr = 0 And (lastResultVar.vt = VT_I4 Or lastResultVar.vt = VT_I2) Then
                            lastResult = "0x" & Hex(lastResultVar.lVal) & IIf(lastResultVar.lVal = 0, " (成功)", " (失败)")
                        End If
                        VariantClear(@lastResultVar)
                        
                        ' ======================
                        ' 获取 Definition（后续复用）
                        ' ======================
                        Dim defVar As VARIANT : VariantInit(@defVar)
                        Dim description As String = ""
                        Dim execPath As String = ""

                        If DispatchInvokeGet(taskDisp, "Definition", defVar) = 0 And defVar.vt = VT_DISPATCH Then

                            ' ----------------------
                            ' 1. Description
                            ' ----------------------
                            Dim regVar As VARIANT : VariantInit(@regVar)
                            Dim descVar As VARIANT : VariantInit(@descVar)

                            If DispatchInvokeGet(defVar.pdispVal, "RegistrationInfo", regVar) = 0 And regVar.vt = VT_DISPATCH Then
                                If DispatchInvokeGet(regVar.pdispVal, "Description", descVar) = 0 Then
                                    If descVar.vt = VT_BSTR Then description = BSTRToString(descVar.bstrVal)
                                End If
                            End If

                            VariantClear(@descVar)
                            VariantClear(@regVar)

                            ' ----------------------
                            ' 2. ExecPath
                            ' ----------------------
                            Dim actionsVar As VARIANT : VariantInit(@actionsVar)

                            If DispatchInvokeGet(defVar.pdispVal, "Actions", actionsVar) = 0 And actionsVar.vt = VT_DISPATCH Then
                                
                                Dim actVar As VARIANT : VariantInit(@actVar)
                                Dim idxArg2(0) As VARIANT
                                idxArg2(0).vt = VT_I4 : idxArg2(0).lVal = 1

                                If DispatchInvokeGetWithArgs(actionsVar.pdispVal, "Item", @idxArg2(0), 1, actVar) = 0 And actVar.vt = VT_DISPATCH Then
                                    
                                    Dim pathVar As VARIANT : VariantInit(@pathVar)
                                    If DispatchInvokeGet(actVar.pdispVal, "Path", pathVar) = 0 Then
                                        If pathVar.vt = VT_BSTR Then execPath = BSTRToString(pathVar.bstrVal)
                                    End If
                                    VariantClear(@pathVar)

                                End If
                                VariantClear(@actVar)

                            End If
                            VariantClear(@actionsVar)

                        End If

                        ' ⚠️ 最后再释放 Definition
                        VariantClear(@defVar)
                        
                        Dim taskPath As String = ""
                        If currentPath <> "\" Then taskPath = currentPath & "\" & taskName Else taskPath = currentPath & taskName
                        execPath = LCase(execPath)
                        execPath = FF_Replace(execPath, "%systemroot%", Environ("SystemRoot"))
                        execPath = FF_Replace(execPath, "%windir%", Environ("windir"))
                        execPath = FF_Replace(execPath, "%localappdata%", Environ("localappdata"))
                        ' 打印任务完整信息
                        AddItemColListEx lvwTaskScheduler, 12,,, WStr(index), WStr(taskPath), WStr(isEnabled), WStr(taskState), WStr(lastRunTime), WStr(nextRunTime), WStr(lastResult), WStr(execPath), WStr(description), WStr(""), WStr(""), WStr("")
                        /'Print "  索引:            "; index
                        Print "  任务名称："; taskName
                        Print "  启用状态："; isEnabled
                        Print "  当前状态："; taskState
                        Print "  上次运行："; lastRunTime
                        Print "  下次运行："; nextRunTime
                        Print "  运行结果："; lastResult
                        Print "--------------------------------------------------"'/
                        index += 1
                        
                    Else
                        ' 调试打印：任务获取失败的错误码
                        Print "⚠️  第"; i; "个任务获取失败, hr=0x"; Hex(hr); ", vt="; taskVar.vt
                        Print "--------------------------------------------------"
                    End If
                    VariantClear(@taskVar)
                Next
                Print
            End If
        End If
        VariantClear(@countVar)
    End If
    VariantClear(@tasksVar)

    ' 递归枚举子文件夹
    Dim subFoldersVar As VARIANT : VariantInit(@subFoldersVar)
    Dim folderArg(0) As VARIANT : folderArg(0).vt = VT_I4 : folderArg(0).lVal = 1
    hr = DispatchInvokeMethod(folderDisp, "GetFolders", @folderArg(0), 1, subFoldersVar)
    
    If hr = 0 And subFoldersVar.vt = VT_DISPATCH Then
        Dim folderCountVar As VARIANT : VariantInit(@folderCountVar)
        hr = DispatchInvokeGet(subFoldersVar.pdispVal, "Count", folderCountVar)
        
        If hr = 0 And (folderCountVar.vt = VT_I4 Or folderCountVar.vt = VT_I2) Then
            Dim folderCount As Long = folderCountVar.lVal
            For i As Long = 1 To folderCount
                Dim subFolderVar As VARIANT : VariantInit(@subFolderVar)
                Dim idxArg(0) As VARIANT : idxArg(0).vt = VT_I4 : idxArg(0).lVal = i
                hr = DispatchInvokeGetWithArgs(subFoldersVar.pdispVal, "Item", @idxArg(0), 1, subFolderVar)
                
                If hr = 0 And subFolderVar.vt = VT_DISPATCH Then
                    Dim nameVar As VARIANT : VariantInit(@nameVar)
                    hr = DispatchInvokeGet(subFolderVar.pdispVal, "Name", nameVar)
                    If hr = 0 And nameVar.vt = VT_BSTR And nameVar.bstrVal <> NULL Then
                        Dim subFolderName As WString * 1024 = BSTRToString(nameVar.bstrVal)
                        ' lstrcpyW(@subFolderName, nameVar.bstrVal)
                        Dim fullPath As String = IIf(currentPath = "\", "\" & WStr(subFolderName), currentPath & "\" & WStr(subFolderName))
                        EnumTasksInFolder(subFolderVar.pdispVal, fullPath, lvwTaskScheduler)
                    End If
                    VariantClear(@nameVar)
                End If
                VariantClear(@subFolderVar)
            Next
        End If
        VariantClear(@folderCountVar)
    End If
    VariantClear(@subFoldersVar)
End Sub

' ==============================================
' 主函数
' ==============================================
Sub GetTaskSchedulerList(lvwTaskScheduler As Class_ListView)
    Dim pTaskService As IDispatch Ptr
    Dim hr As Long

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr <> 0 And hr <> 1 Then
        Print "COM初始化失败, hr=0x"; Hex(hr)
        Exit Sub
    End If

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pTaskService)
    If hr <> 0 Or pTaskService = NULL Then
        Print "创建TaskScheduler对象失败, hr=0x"; Hex(hr)
        CoUninitialize()
        Exit Sub
    End If

    Dim connectVar As VARIANT : VariantInit(@connectVar)
    hr = DispatchInvokeMethod(pTaskService, "Connect", NULL, 0, connectVar)
    If hr <> 0 Then
        Print "连接服务失败, hr=0x"; Hex(hr)
        pTaskService->lpVtbl->Release(pTaskService)
        CoUninitialize()
        Exit Sub
    End If
    VariantClear(@connectVar)

    Dim rootFolderVar As VARIANT : VariantInit(@rootFolderVar)
    Dim folderArg(0) As VARIANT : folderArg(0).vt = VT_BSTR : folderArg(0).bstrVal = SysAllocString(WStr("\"))
    hr = DispatchInvokeMethod(pTaskService, "GetFolder", @folderArg(0), 1, rootFolderVar)
    SysFreeString(folderArg(0).bstrVal)
    
    InitListViewData lvwTaskScheduler

    If hr = 0 And rootFolderVar.vt = VT_DISPATCH Then
        Print "===== 系统计划任务完整信息 ====="
        EnumTasksInFolder(rootFolderVar.pdispVal, "\", lvwTaskScheduler)
        Print "===== 枚举完成 ====="
        Print "系统总计计划任务数: "; totalTaskCount
    Else
        Print "无法获取根文件夹, hr=0x"; Hex(hr)
    End If
    
    CommitListViewView lvwTaskScheduler
    
    VariantClear(@rootFolderVar)

    pTaskService->lpVtbl->Release(pTaskService)
    CoUninitialize()
End Sub

' ==============================================
' 计划任务管理常用函数（对齐你的代码风格）
' 依赖：已有的COM封装、BSTRToString、Dispatch系列函数
' ==============================================

' --------------------------
' 1. 立即运行指定任务
' --------------------------
Function RunTask(taskPath As String) As Long
    Dim hr As Long, pService As IDispatch Ptr
    Dim result As VARIANT : VariantInit(@result)
    Dim ret As Long = -1
    Dim vConn As VARIANT
    Dim folderVar As VARIANT, fArg(0) As VARIANT
    Dim taskVar As VARIANT, tArg(0) As VARIANT

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr < 0 Then Return -1

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Or pService = NULL Then Goto Cleanup

    ' 连接服务
    DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)

    ' 获取任务文件夹
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)

    If folderVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 获取任务对象
    tArg(0).vt = VT_BSTR : tArg(0).bstrVal = SysAllocString(WStr(taskPath))
    hr = DispatchInvokeGetWithArgs(folderVar.pdispVal, "GetTask", @tArg(0), 1, taskVar)
    SysFreeString(tArg(0).bstrVal)

    If hr = 0 And taskVar.vt = VT_DISPATCH Then
        ' 运行：Run(0)
        Dim runArg(0) As VARIANT
        runArg(0).vt = VT_I4 : runArg(0).lVal = 0
        hr = DispatchInvokeMethod(taskVar.pdispVal, "Run", @runArg(0), 1, result)
        ret = IIf(hr = 0, 0, -2)
    End If

Cleanup:
    VariantClear(@taskVar)
    VariantClear(@folderVar)
    If pService Then pService->lpVtbl->Release(pService)
    CoUninitialize()
    Return ret
End Function

' --------------------------
' 2. 启用/禁用任务
' --------------------------
Function SetTaskEnabled(taskPath As String, ByVal enable As Boolean) As Long
    Dim hr As Long, pService As IDispatch Ptr
    Dim ret As Long = -1
    Dim vConn As VARIANT
    Dim folderVar As VARIANT, fArg(0) As VARIANT
    Dim taskVar As VARIANT, tArg(0) As VARIANT

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Then GoTo Cleanup

    DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)

    ' 获取文件夹
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)

    ' 获取任务
    tArg(0).vt = VT_BSTR : tArg(0).bstrVal = SysAllocString(WStr(taskPath))
    hr = DispatchInvokeGetWithArgs(folderVar.pdispVal, "GetTask", @tArg(0), 1, taskVar)
    SysFreeString(tArg(0).bstrVal)

    If hr = 0 And taskVar.vt = VT_DISPATCH Then
        ' 设置 Enabled 属性
        Dim vEnable As VARIANT
        vEnable.vt = VT_BOOL
        vEnable.boolVal = IIf(enable, -1, 0)
        hr = DispatchInvokeGeneric(taskVar.pdispVal, "Enabled", @vEnable, 1, DISPATCH_PROPERTYPUT, vEnable)
        ret = IIf(hr = 0, 0, -2)
    End If

Cleanup:
    If pService Then pService->lpVtbl->Release(pService)
    CoUninitialize()
    Return ret
End Function

' --------------------------
' 3. 删除任务
' --------------------------
Function DeleteTask(taskPath As String) As Long
    Dim hr As Long, pService As IDispatch Ptr
    Dim ret As Long = -1
    Dim vConn As VARIANT
    Dim folderVar As VARIANT, fArg(0) As VARIANT

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Then GoTo Cleanup

    DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)
    
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)

    If folderVar.vt = VT_DISPATCH Then
        Dim delArg(0) As VARIANT
        delArg(0).vt = VT_BSTR : delArg(0).bstrVal = SysAllocString(WStr(taskPath))
        hr = DispatchInvokeMethod(folderVar.pdispVal, "DeleteTask", @delArg(0), 1, folderVar)
        SysFreeString(delArg(0).bstrVal)
        ret = IIf(hr = 0, 0, -2)
    End If

Cleanup:
    If pService Then pService->lpVtbl->Release(pService)
    CoUninitialize()
    Return ret
End Function

' --------------------------
' 4. 创建简单执行程序任务
' --------------------------
Function CreateBasicTask(ByVal taskName As String, _
                         ByVal CommandLine As String, _
                         desc As String = "FB创建") As Long
    Dim hr As Long, pService As IDispatch Ptr
    Dim ret As Long = -1
    Dim vConn As VARIANT
    Dim folderVar As VARIANT, fArg(0) As VARIANT
    Dim defVar As VARIANT

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Then GoTo Cleanup
    
    DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)

    ' 根文件夹
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)

    ' 创建任务定义
    DispatchInvokeMethod(pService, "NewTask", NULL, 0, defVar)

    If defVar.vt = VT_DISPATCH Then
        ' 注册信息
        Dim regVar As VARIANT
        DispatchInvokeGet(defVar.pdispVal, "RegistrationInfo", regVar)

        Dim vDesc As VARIANT
        vDesc.vt = VT_BSTR : vDesc.bstrVal = SysAllocString(WStr(desc))
        DispatchInvokeGeneric(regVar.pdispVal, "Description", @vDesc, 1, DISPATCH_PROPERTYPUT, vDesc)
        SysFreeString(vDesc.bstrVal)

        ' 执行动作
        Dim actionsVar As VARIANT
        DispatchInvokeGet(defVar.pdispVal, "Actions", actionsVar)

        Dim actionVar As VARIANT
        Dim createArg(0) As VARIANT : createArg(0).vt = VT_I4 : createArg(0).lVal = 0
        DispatchInvokeMethod(actionsVar.pdispVal, "Create", @createArg(0), 1, actionVar)

        ' 设置路径
        Dim pathVar As VARIANT
        pathVar.vt = VT_BSTR : pathVar.bstrVal = SysAllocString(WStr(CommandLine))
        DispatchInvokeGeneric(actionVar.pdispVal, "Path", @pathVar, 1, DISPATCH_PROPERTYPUT, pathVar)
        SysFreeString(pathVar.bstrVal)

        ' 注册任务
        Dim regArg(4) As VARIANT
        regArg(0).vt = VT_BSTR : regArg(0).bstrVal = SysAllocString(WStr(taskName))
        regArg(1).vt = VT_DISPATCH : regArg(1).pdispVal = defVar.pdispVal
        regArg(2).vt = VT_I4 : regArg(2).lVal = 6
        regArg(3).vt = VT_BSTR : regArg(3).bstrVal = NULL
        regArg(4).vt = VT_BSTR : regArg(4).bstrVal = NULL
        hr = DispatchInvokeMethod(folderVar.pdispVal, "RegisterTaskDefinition", @regArg(0), 5, folderVar)

        SysFreeString(regArg(0).bstrVal)
        ret = IIf(hr = 0, 0, -2)
    End If

Cleanup:
    If pService Then pService->lpVtbl->Release(pService)
    CoUninitialize()
    Return ret
End Function