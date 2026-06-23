' ==============================================
' 必要声明
' ==============================================

#include "win/guiddef.bi"
#include "win/combaseapi.bi"

Enum TRACE_QUERY_INFO_CLASS
    TraceGuidQueryList = 0
    TraceGuidQueryInfo = 1
End Enum

Type WNODE_HEADER
    BufferSize         As ULONG
    ProviderId         As ULONG
    HistoricalContext  As ULONGLONG
    TimeStamp          As LARGE_INTEGER
    GUID               As GUID
    ClientContext      As ULONG
    Flags              As ULONG
End Type

Type EVENT_TRACE_PROPERTIES
    Wnode               As WNODE_HEADER
    BufferSize          As ULONG
    MinimumBuffers      As ULONG
    MaximumBuffers      As ULONG
    MaximumFileSize     As ULONG
    LogFileMode         As ULONG
    FlushTimer          As ULONG
    EnableFlags         As ULONG
    AgeLimit            As Long
    NumberOfBuffers     As ULONG
    FreeBuffers         As ULONG
    EventsLost          As ULONG
    BuffersWritten      As ULONG
    LogBuffersLost      As ULONG
    RealTimeBuffersLost As ULONG
    LoggerThreadId      As HANDLE
    LogFileNameOffset   As ULONG
    LoggerNameOffset    As ULONG
End Type

Declare Function QueryAllTracesW Lib "advapi32" Alias "QueryAllTracesW" ( _
    ByVal PropertyArray As EVENT_TRACE_PROPERTIES Ptr Ptr, _
    ByVal PropertyArrayCount As ULong, _
    ByRef LoggerCount As ULONG _
) As ULONG

Declare Function TdhEnumerateProviders Lib "tdh.dll" Alias "TdhEnumerateProviders" (ByVal pBuffer As Any Ptr, ByRef pBufferSize As ULong) As ULong

Declare Function EnumerateTraceGuidsEx Lib "advapi32" Alias "EnumerateTraceGuidsEx" ( _
    ByVal ClassType As Long, _
    ByVal InBuffer As Any Ptr, _
    ByVal InBufferSize As ULong, _
    ByVal OutBuffer As Any Ptr, _
    ByVal OutBufferSize As ULong, _
    ByRef ReturnLength As ULong _
) As Long

Type TRACE_GUID_INFO Field = 4
    InstanceCount As ULong
    Reserved      As ULong
End Type

Type TRACE_PROVIDER_INSTANCE_INFO Field = 4
    NextOffset    As ULong
    EnableCount   As ULong
    PID           As ULong
    dwFlags       As ULong
End Type

Type TRACE_ENABLE_INFO Field = 4
    IsEnabled       As ULong
    Level           As UByte
    Reserved1       As UByte
    LoggerId        As UShort
    EnableProperty  As ULong
    Reserved2       As ULong
    MatchAnyKeyword As ULongLong
    MatchAllKeyword As ULongLong
End Type

Type _PROVIDER_INFO
    ProviderGuid As GUID
    SchemaSource As ULong
    ProviderNameOffset As ULong
End Type

Type PROVIDER_ENUMERATION_INFO
    NumberOfProviders As ULong
    Reserved As ULong
    ProviderInfo(0 To 0) As _PROVIDER_INFO
End Type

Type PROVIDER_NAME_MAP
    GuidStr As Wstring * MAX_PATH
    Name As Wstring * MAX_PATH
End Type

Dim Shared pProviderMap As PROVIDER_NAME_MAP Ptr = NULL
Dim Shared ProviderCount As Long

Private Function GUIDToWString(ByRef guid As GUID) As StringW
    Dim ws As WString * 40
    StringFromGUID2(@guid, @ws, 40)
    Return ws
End Function

Private Function FindProviderName( _
    ByRef GuidStr As StringW, _
    ProviderMap() As PROVIDER_NAME_MAP, _
    ByVal Count As Long _
) As StringW

    Dim i As Long

    For i = 0 To Count - 1
        If ProviderMap(i).GuidStr = GuidStr Then
            Return ProviderMap(i).Name
        End If
    Next

    Return GuidStr

End Function

' ==============================================
' 修复后的主函数
' ==============================================
Function EnumETWSession(lvwEtw As Class_mCtrlTreeList) As ULong

    Const MAX_SESSIONS = 256

    Dim pProps(MAX_SESSIONS - 1) As EVENT_TRACE_PROPERTIES Ptr
    Dim hSession(MAX_SESSIONS - 1) As MC_HTREELISTITEM
    Dim loggerIdToIndex(65535) As Long

    Dim i As Long
    Dim count As ULong = 0

    For i = 0 To 65535
        loggerIdToIndex(i) = -1
    Next

    ' =====================================================
    ' Step 1: Query sessions
    ' =====================================================

    For i = 0 To MAX_SESSIONS - 1
        Dim cbSize As ULONG = SizeOf(EVENT_TRACE_PROPERTIES) + 1024
        pProps(i) = Allocate(cbSize)
        If pProps(i) = NULL Then Continue For

        ZeroMemory(pProps(i), cbSize)
        pProps(i)->Wnode.BufferSize = cbSize
        pProps(i)->LoggerNameOffset = SizeOf(EVENT_TRACE_PROPERTIES)
    Next

    Dim status As ULong
    status = QueryAllTracesW(@pProps(0), MAX_SESSIONS, count)

    If status <> ERROR_SUCCESS Then
        For i = 0 To MAX_SESSIONS - 1
            If pProps(i) <> NULL Then Deallocate(pProps(i))
        Next
        Return 0
    End If

    If count > MAX_SESSIONS Then count = MAX_SESSIONS

    ' =====================================================
    ' Build session tree and LoggerId -> index map
    ' =====================================================

    lvwEtw.DeleteItem MC_TLI_ROOT

    For i = 0 To count - 1
        Dim pName As WString Ptr
        Dim sid As UShort

        pName = Cast(WString Ptr, Cast(UByte Ptr, pProps(i)) + pProps(i)->LoggerNameOffset)
        sid = Cast(UShort, pProps(i)->Wnode.HistoricalContext)

        If sid <= 65535 Then
            loggerIdToIndex(sid) = i
        End If
        
        hSession(i) = lvwEtw.AddItem(MC_TLI_ROOT, *pName)
        lvwEtw.SetItemText hSession(i), 1, "Session"
        lvwEtw.SetItemText hSession(i), 2, "LoggerId=" & WStr(sid)
        lvwEtw.SetItemText hSession(i), 3, WStr(pProps(i)->EventsLost)
        
        If *pName = "NT Kernel Logger" Then

            Dim hItem As MC_HTREELISTITEM

            hItem = lvwEtw.AddItem( _
                        hSession(i), _
                        "Windows Kernel Trace")

            lvwEtw.SetItemText hItem,1,"Provider"

            lvwEtw.SetItemText hItem,2, _
                "{9E814AAD-3204-11D2-9A82-006008A86939}"

            lvwEtw.SetItemText hItem,3, _
                "EnableFlags=0x" & _
                Hex(pProps(i)->EnableFlags)
        End If
        FF_DoEvents
    Next
    
    Dim ProviderMap() As PROVIDER_NAME_MAP
    Dim ProviderCount As Long = 0

    Dim tdhSize As ULong = 0
    Dim tdhStatus As Long
    Dim pTdhBuf As Any Ptr = NULL

    Do
        tdhStatus = TdhEnumerateProviders(Cast(PROVIDER_ENUMERATION_INFO Ptr, pTdhBuf), tdhSize)

        If tdhStatus = ERROR_INSUFFICIENT_BUFFER Then
            If pTdhBuf <> NULL Then
                Deallocate(pTdhBuf)
                pTdhBuf = NULL
            End If

            pTdhBuf = Allocate(tdhSize)
            If pTdhBuf = NULL Then Exit Do

            ZeroMemory(pTdhBuf, tdhSize)
        Else
            Exit Do
        End If
    Loop While tdhStatus = ERROR_INSUFFICIENT_BUFFER

    If tdhStatus = ERROR_SUCCESS AndAlso pTdhBuf <> NULL Then

        Dim pEnum As PROVIDER_ENUMERATION_INFO Ptr
        pEnum = Cast(PROVIDER_ENUMERATION_INFO Ptr, pTdhBuf)

        If pEnum->NumberOfProviders > 0 Then
            ReDim ProviderMap(0 To pEnum->NumberOfProviders - 1) As PROVIDER_NAME_MAP

            Dim n As Long
            For n = 0 To pEnum->NumberOfProviders - 1
                Dim pProv As _PROVIDER_INFO Ptr
                pProv = @pEnum->ProviderInfo(n)
                
                Dim pProviderName As WString Ptr = Cast(WString Ptr, Cast(UByte Ptr, pTdhBuf) + pProv->ProviderNameOffset)
                ProviderMap(ProviderCount).GuidStr = GUIDToWString(pProv->ProviderGuid)
                ProviderMap(ProviderCount).Name = *pProviderName
                'If ProviderCount < 50 Then Print *pProviderName & " " & ProviderMap(ProviderCount).GuidStr
                ProviderCount += 1
            Next
        End If
    End If

    If pTdhBuf <> NULL Then Deallocate(pTdhBuf)

    ' =====================================================
    ' Step 2: Enumerate ALL runtime provider GUIDs
    '         (TraceGuidQueryList)
    ' =====================================================

    Dim pGuids As GUID Ptr = NULL
    Dim guidBufSize As ULong = 0
    Dim required As ULong = 0

    Do
        status = EnumerateTraceGuidsEx(TraceGuidQueryList, NULL, 0, pGuids, guidBufSize, required)

        If status <> ERROR_INSUFFICIENT_BUFFER Then Exit Do

        If pGuids <> NULL Then
            Deallocate(pGuids)
            pGuids = NULL
        End If

        guidBufSize = required
        If guidBufSize = 0 Then Exit Do

        pGuids = Cast(GUID Ptr, Allocate(guidBufSize))
        If pGuids = NULL Then Exit Do
        ZeroMemory(pGuids, guidBufSize)
    Loop

    If status <> ERROR_SUCCESS Then
        If pGuids <> NULL Then Deallocate(pGuids)
        For i = 0 To MAX_SESSIONS - 1
            If pProps(i) <> NULL Then Deallocate(pProps(i))
        Next
        Deallocate pProviderMap
        Return count
    End If

    Dim guidCount As ULong = guidBufSize \ SizeOf(GUID)

    ' =====================================================
    ' Step 3: For each provider GUID, query TraceGuidQueryInfo
    ' =====================================================

    For i = 0 To guidCount - 1

        Dim curGuid As GUID = pGuids[i]
        Dim curGuidStr As StringW = GUIDToWString(curGuid)

        Dim pInfoBuf As Any Ptr = NULL
        Dim infoBufSize As ULong = 0
        Dim infoReq As ULong = 0

        Do
            status = EnumerateTraceGuidsEx(TraceGuidQueryInfo, @curGuid, SizeOf(GUID), pInfoBuf, infoBufSize, infoReq)

            If status <> ERROR_INSUFFICIENT_BUFFER Then Exit Do

            If pInfoBuf <> NULL Then
                Deallocate(pInfoBuf)
                pInfoBuf = NULL
            End If

            infoBufSize = infoReq
            If infoBufSize = 0 Then Exit Do

            pInfoBuf = Allocate(infoBufSize)
            If pInfoBuf = NULL Then Exit Do
            ZeroMemory(pInfoBuf, infoBufSize)
        Loop

        If status <> ERROR_SUCCESS Then
            If pInfoBuf <> NULL Then Deallocate(pInfoBuf)
            Continue For
        End If

        Dim pGI As TRACE_GUID_INFO Ptr
        Dim pInstance As TRACE_PROVIDER_INSTANCE_INFO Ptr
        Dim j As ULong

        pGI = Cast(TRACE_GUID_INFO Ptr, pInfoBuf)
        pInstance = Cast(TRACE_PROVIDER_INSTANCE_INFO Ptr, Cast(UByte Ptr, pInfoBuf) + SizeOf(TRACE_GUID_INFO))

        For j = 0 To pGI->InstanceCount - 1

            Dim pEnable As TRACE_ENABLE_INFO Ptr
            Dim k As ULong

            If pInstance->EnableCount > 0 Then
                pEnable = Cast(TRACE_ENABLE_INFO Ptr, Cast(UByte Ptr, pInstance) + SizeOf(TRACE_PROVIDER_INSTANCE_INFO))

                For k = 0 To pInstance->EnableCount - 1
                    Dim sid As UShort = pEnable->LoggerId
                    'If curGuidStr = "{9E814AAD-3204-11D2-9A82-006008A86939}" Then Print "找到!"
                    If sid <= 65535 Then
                        Dim sIdx As Long = loggerIdToIndex(sid)

                        If sIdx <> -1 Then
                            Dim hItem As MC_HTREELISTITEM
                            Dim pProviderName As WString Ptr

                            ' 这里用 GUID 字符串当作节点标题，
                            ' 避免同 GUID 的不同 runtime instance 混淆
                            'pProviderName = @curGuidStr
                            
                            Dim displayName As StringW

                            displayName = FindProviderName(curGuidStr, ProviderMap(), ProviderCount)
                            hItem = lvwEtw.AddItem(hSession(sIdx), displayName)
                            lvwEtw.SetItemText hItem, 1, "Provider"
                            lvwEtw.SetItemText hItem, 2, curGuidStr
                            lvwEtw.SetItemText hItem, 3, _
                                "LoggerId=" & WStr(sid) & _
                                " Level=" & WStr(pEnable->Level) & _
                                " MatchAny=0x" & Hex(pEnable->MatchAnyKeyword) & _
                                " Flags=0x" & Hex(pInstance->dwFlags) & _
                                " PID=" & pInstance->PID
                                
                            If lvwEtw.GetItemText(hSession(sIdx), 0) = "Circular Kernel Context Logger" Then lvwEtw.SetItemText hItem, 0, "Circular Kernel Session Provider"
                        End If
                    End If

                    pEnable += 1
                    FF_DoEvents
                Next
            End If

            If pInstance->NextOffset = 0 Then Exit For
            pInstance = Cast(TRACE_PROVIDER_INSTANCE_INFO Ptr, Cast(UByte Ptr, pInstance) + pInstance->NextOffset)
        Next

        If pInfoBuf <> NULL Then Deallocate(pInfoBuf)
    Next

    If pGuids <> NULL Then Deallocate(pGuids)

    ' =====================================================
    ' Cleanup
    ' =====================================================

    For i = 0 To MAX_SESSIONS - 1
        If pProps(i) <> NULL Then Deallocate(pProps(i))
    Next
    
    Deallocate pProviderMap
    Return count
End Function



#include "win/objbase.bi"
#include "win/oleauto.bi"
#include "win/oaidl.bi"

Dim Shared TaskIndex As Integer = 0
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
    'dp.rgdispidNamedArgs = NULL
    'dp.cNamedArgs = 0
    ' === 修复：PROPERTYPUT/PUTREF 必须提供 DISPID_PROPERTYPUT ===
    Dim namedArg As DISPID = DISPID_PROPERTYPUT
    If wFlags = DISPATCH_PROPERTYPUT Or wFlags = DISPATCH_PROPERTYPUTREF Then
        dp.rgdispidNamedArgs = @namedArg
        dp.cNamedArgs = 1
    Else
        dp.rgdispidNamedArgs = NULL
        dp.cNamedArgs = 0
    End If

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
    
    ' 枚举当前文件夹任务
    Dim tasksVar As VARIANT : VariantInit(@tasksVar)
    Dim taskArg(0) As VARIANT : taskArg(0).vt = VT_I4 : taskArg(0).lVal = 1
    hr = DispatchInvokeMethod(folderDisp, "GetTasks", @taskArg(0), 1, tasksVar)
    
    If hr = 0 And tasksVar.vt = VT_DISPATCH Then
        Dim countVar As VARIANT : VariantInit(@countVar)
        hr = DispatchInvokeGet(tasksVar.pdispVal, "Count", countVar)
        
        If hr = 0 And (countVar.vt = VT_I4 Or countVar.vt = VT_I2) Then
            Dim taskCount As Long = countVar.lVal
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
                        Dim lastRunTime As WString * MAX_PATH = "无"
                        hr = DispatchInvokeGet(taskDisp, "LastRunTime", lastRunVar)
                        If hr = 0 And lastRunVar.vt = VT_DATE Then
                            lastRunTime = DateToText(lastRunVar)
                        End If
                        VariantClear(@lastRunVar)
                        
                        ' 5. 获取下次运行时间
                        Dim nextRunVar As VARIANT : VariantInit(@nextRunVar)
                        Dim nextRunTime As WString * MAX_PATH = "无"
                        hr = DispatchInvokeGet(taskDisp, "NextRunTime", nextRunVar)
                        If hr = 0 And nextRunVar.vt = VT_DATE Then
                            nextRunTime = DateToText(nextRunVar)
                        End If
                        VariantClear(@nextRunVar)
                        
                        ' 6. 获取上次运行结果
                        Dim lastResultVar As VARIANT : VariantInit(@lastResultVar)
                        Dim lastResult As WString * MAX_PATH = "未知"
                        hr = DispatchInvokeGet(taskDisp, "LastTaskResult", lastResultVar)
                        If hr = 0 And (lastResultVar.vt = VT_I4 Or lastResultVar.vt = VT_I2) Then
                            lastResult = "0x" & Hex(lastResultVar.lVal) & IIf(lastResultVar.lVal = 0, " (成功)", " (失败)")
                        End If
                        VariantClear(@lastResultVar)
                        
                        ' ======================
                        ' 获取 Definition（后续复用）
                        ' ======================
                        Dim defVar As VARIANT : VariantInit(@defVar)
                        Dim description As WString * MAX_PATH = ""
                        Dim execPath As WString * MAX_PATH = ""

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
                        
                        Dim taskPath As WString * MAX_PATH = ""
                        If currentPath <> "\" Then taskPath = currentPath & "\" & taskName Else taskPath = currentPath & taskName
                        execPath = LCase(execPath)
                        execPath = FF_Replace(execPath, "%systemroot%", Environ("SystemRoot"))
                        execPath = FF_Replace(execPath, "%windir%", Environ("windir"))
                        execPath = FF_Replace(execPath, "%localappdata%", Environ("localappdata"))
                        ' 打印任务完整信息
                        Dim ItemIndex As Long = AddItemColListEx(lvwTaskScheduler, 12,,, WStr(TaskIndex), taskPath, WStr(isEnabled), _
                        WStr(taskState), lastRunTime, nextRunTime, lastResult, execPath, description, WStr(""), WStr(""), WStr(""))
                        'VerifyFileSignAsync FrmMain.hWnd, FrmMain.ListView1.hWnd, "ListView", ItemIndex, strPath
                        /'Print "  索引:            "; index
                        Print "  任务名称："; taskName
                        Print "  启用状态："; isEnabled
                        Print "  当前状态："; taskState
                        Print "  上次运行："; lastRunTime
                        Print "  下次运行："; nextRunTime
                        Print "  运行结果："; lastResult
                        Print "--------------------------------------------------"'/
                        TaskIndex += 1
                        
                    Else
                        ' 调试打印：任务获取失败的错误码
                        Print "⚠️  第"; i; "个任务获取失败, hr=0x"; Hex(hr); ", vt="; taskVar.vt
                        'Print "--------------------------------------------------"
                    End If
                    VariantClear(@taskVar)
                    FF_DoEvents
                Next
                'Print
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
                FF_DoEvents
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
        TaskIndex = 0
        EnumTasksInFolder(rootFolderVar.pdispVal, "\", lvwTaskScheduler)
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
    Dim ret As Long = -1
    Dim vConn As VARIANT : VariantInit(@vConn)
    Dim folderVar As VARIANT : VariantInit(@folderVar)
    Dim taskVar As VARIANT : VariantInit(@taskVar)
    Dim result As VARIANT : VariantInit(@result)
    Dim fArg(0) As VARIANT
    Dim tArg(0) As VARIANT
    Dim runArg(0) As VARIANT

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr < 0 And hr <> &h80010106 Then Return -1  ' RPC_E_CHANGED_MODE 允许继续

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Or pService = NULL Then GoTo Cleanup

    hr = DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)
    If hr <> 0 Then GoTo Cleanup

    ' 获取根文件夹
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    hr = DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)
    If hr <> 0 Or folderVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 获取任务对象（GetTask 是方法，不是属性）
    tArg(0).vt = VT_BSTR : tArg(0).bstrVal = SysAllocString(WStr(taskPath))
    hr = DispatchInvokeMethod(folderVar.pdispVal, "GetTask", @tArg(0), 1, taskVar)
    SysFreeString(tArg(0).bstrVal)
    If hr <> 0 Or taskVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 运行任务：Run(0)
    runArg(0).vt = VT_I4 : runArg(0).lVal = 0
    hr = DispatchInvokeMethod(taskVar.pdispVal, "Run", @runArg(0), 1, result)
    ret = IIf(hr = 0, 0, -2)

Cleanup:
    VariantClear(@result)
    VariantClear(@taskVar)
    VariantClear(@folderVar)
    VariantClear(@vConn)
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
    Dim vConn As VARIANT : VariantInit(@vConn)
    Dim folderVar As VARIANT : VariantInit(@folderVar)
    Dim taskVar As VARIANT : VariantInit(@taskVar)
    Dim fArg(0) As VARIANT
    Dim tArg(0) As VARIANT
    Dim vEnable As VARIANT : VariantInit(@vEnable)
    Dim dummyResult As VARIANT : VariantInit(@dummyResult)

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr < 0 And hr <> &h80010106 Then Return -1

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Or pService = NULL Then GoTo Cleanup

    hr = DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)
    If hr <> 0 Then GoTo Cleanup

    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    hr = DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)
    If hr <> 0 Or folderVar.vt <> VT_DISPATCH Then GoTo Cleanup

    tArg(0).vt = VT_BSTR : tArg(0).bstrVal = SysAllocString(WStr(taskPath))
    hr = DispatchInvokeMethod(folderVar.pdispVal, "GetTask", @tArg(0), 1, taskVar)
    SysFreeString(tArg(0).bstrVal)
    If hr <> 0 Or taskVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 设置 Enabled 属性
    vEnable.vt = VT_BOOL
    vEnable.boolVal = IIf(enable, VARIANT_TRUE, VARIANT_FALSE)
    
    hr = DispatchInvokeGeneric(taskVar.pdispVal, "Enabled", @vEnable, 1, DISPATCH_PROPERTYPUT, dummyResult)
    ret = IIf(hr = 0, 0, -2)

Cleanup:
    VariantClear(@dummyResult)
    VariantClear(@vEnable)
    VariantClear(@taskVar)
    VariantClear(@folderVar)
    VariantClear(@vConn)
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
    Dim vConn As VARIANT : VariantInit(@vConn)
    Dim folderVar As VARIANT : VariantInit(@folderVar)
    Dim dummyResult As VARIANT : VariantInit(@dummyResult)
    Dim fArg(0) As VARIANT
    Dim delArg(0) As VARIANT

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr < 0 And hr <> &h80010106 Then Return -1

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Or pService = NULL Then GoTo Cleanup

    hr = DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)
    If hr <> 0 Then GoTo Cleanup

    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    hr = DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)
    If hr <> 0 Or folderVar.vt <> VT_DISPATCH Then GoTo Cleanup

    delArg(0).vt = VT_BSTR : delArg(0).bstrVal = SysAllocString(WStr(taskPath))
    hr = DispatchInvokeMethod(folderVar.pdispVal, "DeleteTask", @delArg(0), 1, dummyResult)
    SysFreeString(delArg(0).bstrVal)
    ret = IIf(hr = 0, 0, -2)

Cleanup:
    VariantClear(@dummyResult)
    VariantClear(@folderVar)
    VariantClear(@vConn)
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
    Dim vConn As VARIANT : VariantInit(@vConn)
    Dim folderVar As VARIANT : VariantInit(@folderVar)
    Dim defVar As VARIANT : VariantInit(@defVar)
    Dim regVar As VARIANT : VariantInit(@regVar)
    Dim actionsVar As VARIANT : VariantInit(@actionsVar)
    Dim actionVar As VARIANT : VariantInit(@actionVar)
    Dim triggersVar As VARIANT : VariantInit(@triggersVar)
    Dim triggerVar As VARIANT : VariantInit(@triggerVar)
    Dim fArg(0) As VARIANT
    Dim dummyResult As VARIANT : VariantInit(@dummyResult)
    Dim createArg(0) As VARIANT
    Dim vDesc As VARIANT : VariantInit(@vDesc)
    Dim pathVar As VARIANT : VariantInit(@pathVar)
    Dim triggerCreateArg(0) As VARIANT
    Dim regArg(4) As VARIANT
    Dim registerResult As VARIANT : VariantInit(@registerResult)

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)
    If hr < 0 And hr <> &h80010106 Then Return -1

    hr = CoCreateInstance(@CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, @IID_ITaskService, @pService)
    If hr <> 0 Or pService = NULL Then GoTo Cleanup

    hr = DispatchInvokeMethod(pService, "Connect", NULL, 0, vConn)
    If hr <> 0 Then GoTo Cleanup

    ' 获取根文件夹
    fArg(0).vt = VT_BSTR : fArg(0).bstrVal = SysAllocString(WStr("\"))
    hr = DispatchInvokeMethod(pService, "GetFolder", @fArg(0), 1, folderVar)
    SysFreeString(fArg(0).bstrVal)
    If hr <> 0 Or folderVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 创建任务定义
    hr = DispatchInvokeMethod(pService, "NewTask", NULL, 0, defVar)
    If hr <> 0 Or defVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' === 注册信息 ===
    hr = DispatchInvokeGet(defVar.pdispVal, "RegistrationInfo", regVar)
    If hr = 0 And regVar.vt = VT_DISPATCH Then
        vDesc.vt = VT_BSTR : vDesc.bstrVal = SysAllocString(WStr(desc))
        hr = DispatchInvokeGeneric(regVar.pdispVal, "Description", @vDesc, 1, DISPATCH_PROPERTYPUT, dummyResult)
        SysFreeString(vDesc.bstrVal)
        VariantClear(@dummyResult)
    End If

    ' === 执行动作 ===
    hr = DispatchInvokeGet(defVar.pdispVal, "Actions", actionsVar)
    If hr <> 0 Or actionsVar.vt <> VT_DISPATCH Then GoTo Cleanup

    createArg(0).vt = VT_I4 : createArg(0).lVal = 0  ' TASK_ACTION_EXEC
    hr = DispatchInvokeMethod(actionsVar.pdispVal, "Create", @createArg(0), 1, actionVar)
    If hr <> 0 Or actionVar.vt <> VT_DISPATCH Then GoTo Cleanup

    ' 设置执行路径
    pathVar.vt = VT_BSTR : pathVar.bstrVal = SysAllocString(WStr(CommandLine))
    hr = DispatchInvokeGeneric(actionVar.pdispVal, "Path", @pathVar, 1, DISPATCH_PROPERTYPUT, dummyResult)
    SysFreeString(pathVar.bstrVal)
    VariantClear(@dummyResult)
    If hr <> 0 Then GoTo Cleanup

    ' === 触发器（开机启动 TASK_TRIGGER_BOOT）===
    hr = DispatchInvokeGet(defVar.pdispVal, "Triggers", triggersVar)
    If hr = 0 And triggersVar.vt = VT_DISPATCH Then
        triggerCreateArg(0).vt = VT_I4 : triggerCreateArg(0).lVal = 8  ' TASK_TRIGGER_BOOT = 8
        hr = DispatchInvokeMethod(triggersVar.pdispVal, "Create", @triggerCreateArg(0), 1, triggerVar)
    End If

    ' === 注册任务 ===
    regArg(0).vt = VT_BSTR : regArg(0).bstrVal = SysAllocString(WStr(taskName))
    regArg(1).vt = VT_DISPATCH : regArg(1).pdispVal = defVar.pdispVal
    regArg(2).vt = VT_I4 : regArg(2).lVal = 6  ' TASK_CREATE_OR_UPDATE = 6
    regArg(3).vt = VT_BSTR : regArg(3).bstrVal = NULL
    regArg(4).vt = VT_BSTR : regArg(4).bstrVal = NULL

    hr = DispatchInvokeMethod(folderVar.pdispVal, "RegisterTaskDefinition", @regArg(0), 5, registerResult)
    
    SysFreeString(regArg(0).bstrVal)
    VariantClear(@registerResult)
    ret = IIf(hr = 0, 0, -2)

Cleanup:
    VariantClear(@triggerVar)
    VariantClear(@triggersVar)
    VariantClear(@actionVar)
    VariantClear(@actionsVar)
    VariantClear(@regVar)
    VariantClear(@defVar)
    VariantClear(@folderVar)
    VariantClear(@vConn)
    If pService Then pService->lpVtbl->Release(pService)
    CoUninitialize()
    Return ret
End Function