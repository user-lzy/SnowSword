#include "DriverControl.h"
#include "EnumCallbacks.h"
#include "EnumDriverInfo.h"
#include "EnumTimer.h"
#include "File.h"
#include "GDT.h"
#include "Hook.h"
#include "IDT.h"
#include "ioctl.h"
#include "Memory.h"
#include "Module.h"
#include "ObjectInfo.h"
#include "OtherFunctions.h"
#include "Process.h"
#include "Registry.h"
#include "SSDT.h"
#include "Thread.h"
#include "Window.h"

//设备与设备之间通信
#define DEVICE_OBJECT_NAME  L"\\Device\\SnowSword"
//设备与Ring3之间通信
#define DEVICE_LINK_NAME    L"\\DosDevices\\SnowSword"

//DRIVER_INITIALIZE DriverEntry;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS DispatchRoutine(PDEVICE_OBJECT  pDeviceObject, PIRP pIrp);
NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT  pDeviceObject, PIRP pIrp);
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS status = STATUS_SUCCESS;
HANDLE hEvent = NULL;
PKEVENT pUserEvent = NULL;
PKEVENT pKernelEvent = NULL;
struct AdvancedOptions MyAdvancedOptions = { 0 };
struct MemoryStruct stMemory = { 0 };
PEPROCESS Notepad_EProcess = NULL;
PDEVICE_OBJECT g_TargetDeviceObject = NULL;

extern PVOID HalPrivateDispatchTable;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING RegistryPath) {

    UNREFERENCED_PARAMETER(RegistryPath);

    PDEVICE_OBJECT pDeviceObject = NULL;
    UNICODE_STRING DeviceObjectName = { 0 };
    UNICODE_STRING DeviceLinkName = { 0 };

    DbgPrint("Driver is loaded!");

    //创建设备对象名称
    RtlInitUnicodeString(&DeviceObjectName, DEVICE_OBJECT_NAME);
    //创建设备对象
    status = IoCreateDevice(pDriverObject, 0,
        &DeviceObjectName,
        FILE_DEVICE_UNKNOWN,
        0, FALSE,
        &pDeviceObject);
    if (!NT_SUCCESS(status)) return status;

    //创建设备连接名称
    RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
    //将设备连接名称与设备名称关联
    status = IoCreateSymbolicLink(&DeviceLinkName, &DeviceObjectName);

    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(pDeviceObject);
        return status;
    }
    
    //注册派遣函数
    pDriverObject->DriverUnload = DriverUnload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlDispatchRoutine;

    //绕过签名检查
    PKLDR_DATA_TABLE_ENTRY pLdrData = (PKLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
    pLdrData->Flags = pLdrData->Flags | 0x20;

    KeServiceDescriptorTable = GetKeServiceDescriptorTable();
    KeServiceDescriptorTableShadow = GetKeServiceDescriptorTableShadow();
    
    return STATUS_SUCCESS;
}

//派遣历程
NTSTATUS DispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Information = 0;		//设置操作的字节数为0
    Irp->IoStatus.Status = STATUS_SUCCESS;	//返回成功
    IoCompleteRequest(Irp, IO_NO_INCREMENT);	//指示完成此IRP
    
    return STATUS_SUCCESS; //返回成功
}

NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    //NTSTATUS status = 0;
    ULONG_PTR Information = 0;
    PVOID pInputData = NULL;
    ULONG InputDataLength = 0;
    PVOID pOutputData = NULL;
    ULONG OutputDataLength = 0;
    ULONG IoControlCode = 0;
    PIO_STACK_LOCATION IoStackLocation = IoGetCurrentIrpStackLocation(pIrp);  //Irp堆栈  
    HANDLE dwProcessId = NULL;
	HANDLE dwThreadId = NULL;
    static CallbackInfo Callbacks[64] = { 0 };
    static ObCallbackInfo ObCallbacks[64] = {0};
    PWSTR ustrFileName = NULL;
    PVOID NotifyRoutine = NULL;

    //DbgPrint("Enter the IoctlDispatchRoutine!");
    //DbgPrint("Current PID:%Iu", (ULONG_PTR)PsGetCurrentProcessId());

    IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
    pInputData = pIrp->AssociatedIrp.SystemBuffer;
    pOutputData = pIrp->AssociatedIrp.SystemBuffer;
    InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

    switch (IoControlCode)
    {
    case IOCTL_KillProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            DbgPrint("接收到PID:%p", dwProcessId);
            KillProcess(dwProcessId);
            if (!NT_SUCCESS(status))
            {
                DbgPrint("Kill Process %p Failed!\n", dwProcessId);
            }
        }
        if (pOutputData != NULL && OutputDataLength >= strlen("Hello-World") + 1)
        {
            memcpy(pOutputData, "Hello-World", strlen("Hello-World") + 1);
            status = STATUS_SUCCESS;
            Information = strlen("Hello-World") + 1;
        }
        else
        {
            status = STATUS_INSUFFICIENT_RESOURCES;   //内存不够
            Information = 0;
        }
        break;
    case IOCTL_MemKillProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            //DbgPrint("接收到PID:%p", dwProcessId);
            MemKillProcess(dwProcessId);
        }
        break;
    case IOCTL_ForceKillProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            //DbgPrint("接收到PID:%p", dwProcessId);
            ForceKillProcess(dwProcessId);
        }
        break;
    case IOCTL_ForceKillThread:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwThreadId = *(PHANDLE)pInputData;
            //DbgPrint("接收到TID:%p", dwThreadId);
            status = ForceKillThread(dwThreadId);
            if (!NT_SUCCESS(status))
                DbgPrint("ForceKillThread %lld Failed!status:%x\n", (ULONG_PTR)dwThreadId, status);
        }
        break;
    case IOCTL_GetEProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            PEPROCESS pEProcess = GetEProcess(dwProcessId);
            if (pEProcess == NULL)
                DbgPrint("GetEProcess Failed!");
            ULONG64 pEProcessAddr = (ULONG64)pEProcess;
            memcpy(pOutputData, &pEProcessAddr, sizeof(pEProcessAddr));
            Information = sizeof(pEProcessAddr);
        }
        break;
    case IOCTL_GetEThread:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwThreadId = *(PHANDLE)pInputData;
            PETHREAD pEThread = GetEThread(dwThreadId);
            if (pEThread == NULL)
                DbgPrint("GetEThread Failed!");
            ULONG64 pEThreadAddr = (ULONG64)pEThread;
            memcpy(pOutputData, &pEThreadAddr, sizeof(pEThreadAddr));
            Information = sizeof(pEThreadAddr);
        }
        break;
    case IOCTL_OpenProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            HANDLE hProcess = NULL;
            status = OpenProcess(dwProcessId, &hProcess);
            if (!NT_SUCCESS(status))
                DbgPrint("OpenProcess Failed!");
            memcpy(pOutputData, &hProcess, sizeof(HANDLE));
            Information = sizeof(HANDLE);
        }
        break;
    case IOCTL_OpenThread:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwThreadId = *(PHANDLE)pInputData;
            HANDLE hThread = NULL;
            status = OpenThread(dwThreadId, &hThread);
            if (!NT_SUCCESS(status))
                DbgPrint("OpenThread Failed!");
            memcpy(pOutputData, &hThread, sizeof(HANDLE));
            Information = sizeof(HANDLE);
        }
        break;
    case IOCTL_GetProcessPath:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;

            // 声明一个 LPWSTR 用于存储进程路径
            LPWSTR pProcessPath = NULL;
            DWORD dwProcessPathLength = 260;
            pProcessPath = (LPWSTR)ExAllocatePool2(POOL_FLAG_NON_PAGED, dwProcessPathLength, 'cbin');
            if (!pProcessPath)
            {
                DbgPrint("Failed to allocate memory for pProcessPath\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            // 调用 GetProcessImageName 获取进程路径
            status = GetProcessImageName(dwProcessId, pProcessPath);
            if (NT_SUCCESS(status))
            {
                //DbgPrint("GetProcessPath %lld Success!", (ULONG_PTR)dwProcessId);

                // 计算实际字符串长度（字节）
                size_t actualLength = (wcslen(pProcessPath) + 1) * sizeof(WCHAR);

                // 确保输出缓冲区足够大
                if (OutputDataLength >= actualLength)
                {
                    memcpy(pOutputData, pProcessPath, actualLength);
                    Information = actualLength;
                }
                else
                {
                    // 输出缓冲区不足
                    status = STATUS_BUFFER_OVERFLOW;
                    Information = actualLength;
                }
            }
            else
            {
                DbgPrint("GetProcessPath %Iu Failed! %X", (ULONG_PTR)dwProcessId, status);
            }

            // 释放分配的内存
            ExFreePoolWithTag(pProcessPath, 'cbin');
        }
        break;
    case IOCTL_SetProcessProtectStatus:
        if (pInputData != NULL && InputDataLength > 0)
        {
            MyAdvancedOptions.IsProtectProcess = *(PBOOLEAN)pInputData;
            SetProcessProtectionStatus(MyAdvancedOptions.IsProtectProcess);
        }
        break;
    case IOCTL_SetThreadProtectStatus:
        if (pInputData != NULL && InputDataLength > 0)
        {
            MyAdvancedOptions.IsProtectThread = *(PBOOLEAN)pInputData;
            SetThreadProtectionStatus(MyAdvancedOptions.IsProtectThread);
        }
        break;
    case IOCTL_AddProtectedProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            AddProcessToProtectedList(dwProcessId);
        }
        break;
    case IOCTL_RemoveProtectedProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            dwProcessId = *(PHANDLE)pInputData;
            RemoveProcessFromProtectedList(dwProcessId);
        }
        break;
    case IOCTL_ProtectThread:
        if (pInputData != NULL && InputDataLength > 0)
        {
            MyProcessId = *(PHANDLE)pInputData;
            SetThreadProtectionStatus(TRUE);
        }
        break;
    case IOCTL_SetEvent:
        hEvent = *(PHANDLE)pInputData;
        status = ObReferenceObjectByHandle(hEvent, EVENT_MODIFY_STATE, *ExEventObjectType, KernelMode, (PVOID*)&pUserEvent, NULL);
        ObDereferenceObject(pUserEvent);
        break;
    case IOCTL_GetEvent:
        WCHAR wEventName[] = L"\\BaseNamedObjects\\KernelEvent";
        UNICODE_STRING uEventName;

        RtlInitUnicodeString(&uEventName, wEventName);
        pKernelEvent = IoCreateNotificationEvent(&uEventName, &hEvent);
        break;

    case IOCTL_EnumProcesses:
        if (pInputData != NULL && InputDataLength >= sizeof(PHANDLE)) {
            PHANDLE Array = *(PHANDLE*)pInputData;
            ULONG NumOfProcess = EnumProcesses(Array);
			Information = NumOfProcess * sizeof(HANDLE);
        }
        break;
    case IOCTL_EnumThreads:
        if (pInputData != NULL && InputDataLength >= sizeof(PHANDLE)) {
            PHANDLE Array = *(PHANDLE*)pInputData;
            ULONG NumOfThread = EnumThreads(Array);
            Information = NumOfThread * sizeof(HANDLE);
        }
        break;
    case IOCTL_GetProcessOfThread:
        if (pInputData != NULL && InputDataLength >= sizeof(PHANDLE)) {
            dwThreadId = *(PHANDLE)pInputData;
            PETHREAD EThread = GetEThread(dwThreadId);
            if (EThread != NULL) {
                PEPROCESS EProcess = PsGetThreadProcess(EThread);
                dwProcessId = PsGetProcessId(EProcess);
                memcpy(pOutputData, &dwProcessId, sizeof(HANDLE));
                Information = sizeof(HANDLE);
                ObDereferenceObject(EThread);
			}
        }
        break;

    case IOCTL_DeleteFileByXCB:
		ustrFileName = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WCHAR) * 260, 'cbin');
		if (ustrFileName == NULL) {
			DbgPrint("Failed to allocate memory for ustrFileName\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
        //获取输入的文件路径
        if (pInputData != NULL && InputDataLength > 0) {
			//使用RtlStringCbCopyW获取输入的文件路径
			RtlStringCbCopyW(ustrFileName, sizeof(WCHAR) * 260, (PWSTR)pInputData);
            //RtlInitUnicodeString(&ustrFileName, L"\\??\\C:\\Users\\21607\\Desktop\\新建文件夹1\\TestMessageBox7.exe");
			DbgPrint("接收到文件路径:%ws", ustrFileName);
			UNICODE_STRING unicodeFileName;
			RtlInitUnicodeString(&unicodeFileName, ustrFileName);
            status = DeleteFileByXCBFunction(&unicodeFileName);
        }
        break;
    case IOCTL_DeleteFileByIRP:
        ustrFileName = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WCHAR) * 260, 'cbin');
        if (ustrFileName == NULL) {
            DbgPrint("Failed to allocate memory for ustrFileName\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        //获取输入的文件路径
        if (pInputData != NULL && InputDataLength > 0) {
            //使用RtlStringCbCopyW获取输入的文件路径
            RtlStringCbCopyW(ustrFileName, sizeof(WCHAR) * 260, (PWSTR)pInputData);
            //RtlInitUnicodeString(&ustrFileName, L"\\??\\C:\\Users\\21607\\Desktop\\新建文件夹1\\TestMessageBox7.exe");
            DbgPrint("接收到文件路径:%ws", ustrFileName);
            UNICODE_STRING unicodeFileName;
            RtlInitUnicodeString(&unicodeFileName, ustrFileName);
            status = ForceDeleteFile(unicodeFileName);
            DbgPrint("ForceDeleteFile result:0x%X", status);
        }
        break;
    case IOCTL_CopyFolder:
        //获取输入的文件路径
        if (pInputData != NULL && InputDataLength >= sizeof(COPY_PATH)) {
            _try{
                COPY_PATH stCopyPath = *(PCOPY_PATH)pInputData;
                PWCHAR sourcePath = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WCHAR) * 260, 'scph');
                PWCHAR destPath = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WCHAR) * 260, 'dcph');
                if (sourcePath == NULL || destPath == NULL) {
                    DbgPrint("Failed to allocate memory for sourcePath or destPath\n");
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                //使用RtlStringCbCopyW获取输入的文件路径
                RtlStringCbCopyW(sourcePath, sizeof(WCHAR) * 260, (PWSTR)stCopyPath.SourcePath);
                RtlStringCbCopyW(destPath, sizeof(WCHAR) * 260, (PWSTR)stCopyPath.TargetPath);
                //RtlInitUnicodeString(&ustrFileName, L"\\??\\C:\\Users\\21607\\Desktop\\新建文件夹1\\TestMessageBox7.exe");
                DbgPrint("接收到文件路径:%ws %ws", sourcePath, destPath);
                UNICODE_STRING uniSourcePath, uniDestPath;
                RtlInitUnicodeString(&uniSourcePath, sourcePath);
                RtlInitUnicodeString(&uniDestPath, destPath);
                if (stCopyPath.IsDirectory) status = ForceCopyFolder(&uniSourcePath, &uniDestPath);
                DbgPrint("ForceCopyFolder result:0x%X", status);
            }
            _except(EXCEPTION_EXECUTE_HANDLER){
                status = GetExceptionCode();
			}
        }
        break;

    case IOCTL_DenyCreateProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            //GetHotkeyInfo();
            //break;
            MyAdvancedOptions.DenyCreateProcess = *(PBOOLEAN)pInputData;
            SetProcessMonitorStatus(MyAdvancedOptions.DenyCreateProcess);
        }
        break;
    case IOCTL_DenyAccessRegistry:
        if (pInputData != NULL && InputDataLength > 0)
        {
            MyAdvancedOptions.DenyAccessRegistry = *(PBOOLEAN)pInputData;
            SetRegMonitorStatus(pDeviceObject->DriverObject, MyAdvancedOptions.DenyAccessRegistry);
        }
        break;
    case IOCTL_DenyLoadDriver:
        if (pInputData != NULL && InputDataLength > 0)
        {
            MyAdvancedOptions.DenyLoadDriver = *(PBOOLEAN)pInputData;
            SetImageMonitorStatus(MyAdvancedOptions.DenyLoadDriver);
        }
        break;
	case IOCTL_DenyRemoteThread:
		if (pInputData != NULL && InputDataLength > 0)
		{
			MyAdvancedOptions.DenyRemoteThread = *(PBOOLEAN)pInputData;
			SetThreadMonitorStatus(MyAdvancedOptions.DenyRemoteThread);
		}
		break;
    case IOCTL_ReadProcessMemory:
        __try {
            stMemory = *(PMemoryStruct)pInputData;
            //ProbeForRead((PVOID)stMemory.Addr, stMemory.Size, sizeof(ULONG));
            SIZE_T BytesTransferred = 0;
            status = CopyMemory(stMemory.dwProcessId, (PVOID)stMemory.Addr, stMemory.pData, stMemory.Size, &BytesTransferred);
			Information = BytesTransferred;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        DbgPrint("status=%X, Information=%lld", status, Information);
        break;
    case IOCTL_WriteProcessMemory:
        __try {
            stMemory = *(PMemoryStruct)pInputData;
            //ProbeForWrite((PVOID)stMemory.Addr, stMemory.Size, sizeof(ULONG));
            SIZE_T BytesTransferred = 0;
            status = CopyMemory(stMemory.dwProcessId, stMemory.pData, (PVOID)stMemory.Addr, stMemory.Size, &BytesTransferred);
            Information = BytesTransferred;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        break;
    case IOCTL_ScanKernelMemory:
        if (pInputData != NULL && InputDataLength > 0) {
            stMemory = *(PMemoryStruct)pInputData;
        }
        else
            {
            status = STATUS_INVALID_PARAMETER;
            break;
		}

        if (pOutputData != NULL && OutputDataLength >= sizeof(MEMORY_SCAN_RESULT))
        {
			*(PMEMORY_SCAN_RESULT)pOutputData = OptimizedScanKernelMemory((PVOID)stMemory.Addr, (ULONG)stMemory.Size);
            Information = sizeof(PMEMORY_SCAN_RESULT);
			status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
        break;

    case IOCTL_EnumCallbacks:
		static PCallbackInfo pCallbacks = NULL;
        static ULONG num = 0;

        if (pOutputData != NULL && OutputDataLength >= sizeof(CallbackInfo) * num)
        {
            //memset(Callbacks, 0, sizeof(Callbacks));
            Information = sizeof(CallbackInfo) * num;
            memcpy(pOutputData, pCallbacks, Information);
            if (pCallbacks != NULL) ExFreePoolWithTag(pCallbacks, 'cbin');
            //DbgPrint("GetHotkeyInfo status = %X", GetHotkeyInfo());
            status = STATUS_SUCCESS;
        }
        else
        {
            pCallbacks = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * 512, 'cbin');
            if (pCallbacks == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            num = EnumCallbacks(&pCallbacks);
            Information = sizeof(CallbackInfo) * num;
            status = STATUS_SUCCESS;
        }
        break;
    case IOCTL_EnumIoTimers:
		SYSTEM_TIMER SystemTimers[64] = { 0 };
		EnumIoTimers(SystemTimers);
        if (pOutputData != NULL && OutputDataLength >= sizeof(SystemTimers))
        {
            memcpy(pOutputData, SystemTimers, sizeof(SystemTimers));
            Information = sizeof(SystemTimers);
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_INFO_LENGTH_MISMATCH;
        }
        break;
    case IOCTL_EnumDpcTimers:
		SYSTEM_TIMER DpcTimers[64] = { 0 };
		EnumDpcTimers(DpcTimers);
		if (pOutputData != NULL && OutputDataLength >= sizeof(DpcTimers))
		{
			memcpy(pOutputData, DpcTimers, sizeof(DpcTimers));
			Information = sizeof(DpcTimers);
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_INFO_LENGTH_MISMATCH;
		}
		break;
    case IOCTL_GetDriverObjectByBaseAddress:
		if (pInputData != NULL && InputDataLength > 0)
		{
			PVOID BaseAddress = *(PVOID*)pInputData;
			PDRIVER_OBJECT pDriverObject = GetDriverObjectByBaseAddress(BaseAddress);
			memcpy(pOutputData, &pDriverObject, sizeof(PDRIVER_OBJECT));
			if (pDriverObject != NULL) ObDereferenceObject(pDriverObject);
			Information = sizeof(PDRIVER_OBJECT);
		}
		break;
    case IOCTL_GetDriverInfo:
        if (pInputData != NULL && InputDataLength > 0)
        {
            PDRIVER_OBJECT DriverBase = *(PDRIVER_OBJECT*)pInputData;
            PDRIVER_INFO pDriverInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(DRIVER_INFO), 'pdio');
            if (pDriverInfo == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            GetDriverInfo(DriverBase, pDriverInfo);
			//DbgPrint("GetDriverInfo Success!");
            memcpy(pOutputData, pDriverInfo, sizeof(DRIVER_INFO));
            Information = sizeof(DRIVER_INFO);
        }
        break;
    case IOCTL_DisableCallback:
        if (pInputData != NULL && InputDataLength > 0)
        {
			PVOID CallbackAddr = *(PVOID*)pInputData;
			UCHAR OldCode[2] = { 0 };
            ControlCallback(CallbackAddr, OldCode, FALSE);
			//将旧代码返回给用户
            if (OutputDataLength >= sizeof(OldCode))
            {
                memcpy(pOutputData, OldCode, sizeof(OldCode));
                Information = sizeof(OldCode);
                status = STATUS_SUCCESS;
            }
        }
        break;
    /*case IOCTL_EnableCallback:
        if (pInputData != NULL && InputDataLength > 0)
        {
            PVOID CallbackAddr = *(PVOID*)pInputData;
            UCHAR OldCode[2] = { 0 };
            ControlCallback(CallbackAddr, OldCode, TRUE);
        }
        break;*/
	case IOCTL_QueryObject:
		if (pInputData != NULL && InputDataLength > 0)
		{
			PHANDLE_INFO HandleInfo = (PHANDLE_INFO)pInputData;
			status = QueryObject(HandleInfo);
            if (OutputDataLength < sizeof(HANDLE_INFO))
            {
                status = STATUS_INFO_LENGTH_MISMATCH;
                //DbgPrint("OutputDataLength:%d, sizeof(HANDLE_INFO):%d", (long)OutputDataLength, (long)sizeof(HANDLE_INFO));
                break;
            }
			Information = sizeof(HANDLE_INFO);
            memcpy(pOutputData, HandleInfo, sizeof(HANDLE_INFO));
		}
		break;
    case IOCTL_SuspendProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            HANDLE PID = *(PHANDLE)pInputData;
            SuspendProcess(PID);
        }
        break;
    case IOCTL_ResumeProcess:
        if (pInputData != NULL && InputDataLength > 0)
        {
            HANDLE PID = *(PHANDLE)pInputData;
            ResumeProcess(PID);
        }
        break;
    case IOCTL_CloseHandle:
        if (pInputData != NULL && InputDataLength > 0)
        {
            PDO_SOMETHING DoSomeThing = (PDO_SOMETHING)pInputData;
            status = CloseHandle(DoSomeThing);
        }
        break;
    case IOCTL_UnloadDriver:
        if (pInputData != NULL && InputDataLength > 0)
        {
            PDRIVER_OBJECT *ppDriverObject = (PDRIVER_OBJECT*)pInputData;
            UnloadDriver(*ppDriverObject);
        }
        break;
	case IOCTL_GetProcessHandleCount:
		if (pInputData != NULL && InputDataLength > 0)
		{
			HANDLE ProcessId = *(PHANDLE)pInputData;
			ULONG HandleCount = 0;
			status = GetProcessHandleCount(ProcessId, &HandleCount);
			if (!NT_SUCCESS(status))
				DbgPrint("GetProcessHandleCount %Iu Failed!status:%x\n", (ULONG_PTR)ProcessId, status);
            //if (ProcessId == (HANDLE)4) DbgPrint("System Has %d Handles.", HandleCount);
            memcpy(pOutputData, &HandleCount, sizeof(HandleCount));
            Information = sizeof(HandleCount);
		}
		break;
    case IOCTL_EnumProcessTimers:
        static PWINDOW_TIMER pTimerArray = NULL;
        static ULONG ulTimerCount = 0;

        // 调试输出
        DbgPrint("[IOCTL_TIMER] 进入IOCTL，Input=%lu, Output=%lu, 缓存数量=%lu\n",
            InputDataLength, OutputDataLength, ulTimerCount);

        HANDLE TargetProcessId = NULL;
        // 读取PID
        if (pInputData != NULL && InputDataLength >= sizeof(HANDLE))
        {
            TargetProcessId = *(PHANDLE)pInputData;
            DbgPrint("[IOCTL_TIMER] 目标PID: %llu\n", (ULONG64)TargetProcessId);
        }
        else
        {
            DbgPrint("[IOCTL_TIMER] 输入参数错误\n");
            status = STATUS_INVALID_PARAMETER;
            Information = 0;
            break;
        }

        // ====================== 【修复】强制判断：只有输出缓冲区有效 才是第二次调用 ======================
        if (pOutputData != NULL && OutputDataLength > 0 && ulTimerCount > 0)
        {
            // 第二次调用：拷贝数据
            DbgPrint("[IOCTL_TIMER] 第二次调用：拷贝数据，数量=%lu\n", ulTimerCount);

            Information = sizeof(WINDOW_TIMER) * ulTimerCount;
            memcpy(pOutputData, pTimerArray, Information);

            // 释放内存+重置静态变量
            if (pTimerArray != NULL)
            {
                ExFreePoolWithTag(pTimerArray, 'meT');
                pTimerArray = NULL;
            }
            ulTimerCount = 0;
            status = STATUS_SUCCESS;
            DbgPrint("[IOCTL_TIMER] 拷贝完成，返回长度=%lu\n", Information);
        }
        else
        {
            //EnumProcessTimers1();
            //DbgPrint("分割线...");
            // ====================== 【修复】第一次调用：强制走枚举获取长度 ======================
            DbgPrint("[IOCTL_TIMER] 第一次调用：开始枚举定时器\n");

            // 清理旧内存/变量
            if (pTimerArray != NULL)
            {
                ExFreePoolWithTag(pTimerArray, 'meT');
                pTimerArray = NULL;
            }
            ulTimerCount = 0;

            // 分配缓存
            pTimerArray = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(WINDOW_TIMER) * 512, 'meT');
            if (!pTimerArray)
            {
                DbgPrint("[IOCTL_TIMER] 内存分配失败\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
                Information = 0;
                break;
            }

            // 枚举定时器
            NTSTATUS enumStatus = EnumProcessTimers(TargetProcessId, &pTimerArray, &ulTimerCount);
            DbgPrint("[IOCTL_TIMER] 枚举完成，数量=%lu, 状态=0x%X\n", ulTimerCount, enumStatus);

            // 失败处理
            if (!NT_SUCCESS(enumStatus) || ulTimerCount == 0)
            {
                if (pTimerArray)
                {
                    ExFreePoolWithTag(pTimerArray, 'meT');
                    pTimerArray = NULL;
                }
                ulTimerCount = 0;
            }

            // 返回长度（核心！第一次调用必须返回）
            Information = sizeof(WINDOW_TIMER) * ulTimerCount;
            status = STATUS_SUCCESS;
            DbgPrint("[IOCTL_TIMER] 第一次调用完成，返回总长度=%lu\n", Information);
        }
        break;
    case IOCTL_RemoveCreateProcessNotifyRoutine:
        if (pInputData != NULL && InputDataLength > 0) {
            NotifyRoutine = *(PVOID*)pInputData;
            status = RemoveCreateProcessNotifyRoutine(NotifyRoutine);
        }
        break;
    case IOCTL_RemoveCreateThreadNotifyRoutine:
        if (pInputData != NULL && InputDataLength > 0) {
            NotifyRoutine = *(PVOID*)pInputData;
            status = RemoveCreateThreadNotifyRoutine(NotifyRoutine);
        }
        break;
    case IOCTL_RemoveLoadImageNotifyRoutine:
        if (pInputData != NULL && InputDataLength > 0) {
            NotifyRoutine = *(PVOID*)pInputData;
            status = RemoveLoadImageNotifyRoutine(NotifyRoutine);
        }
        break;
    case IOCTL_UnregisterCmpCallback:
        if (pInputData != NULL && InputDataLength > 0) {
            LARGE_INTEGER nCookie = *(LARGE_INTEGER*)pInputData;
            status = UnregisterCmpCallback(nCookie);
        }
        break;
    case IOCTL_UnregisterObCallback:
        if (pInputData != NULL && InputDataLength > 0) {
            PVOID ObHandle = *(PVOID*)pInputData;
            UnregisterObCallback(ObHandle);
        }
        break;
    case IOCTL_EnumMiniFilter:
        static PMINIFILTER_OBJECT pMiniFilters = NULL;
        static ULONG num2 = 0;

        if (pOutputData != NULL && OutputDataLength >= sizeof(MINIFILTER_OBJECT) * num2)
        {
            //memset(Callbacks, 0, sizeof(Callbacks));
            Information = sizeof(MINIFILTER_OBJECT) * num2;
            memcpy(pOutputData, pMiniFilters, Information);
            if (pMiniFilters != NULL) ExFreePoolWithTag(pMiniFilters, 'mfin');
            //DbgPrint("GetHotkeyInfo status = %X", GetHotkeyInfo());
            status = STATUS_SUCCESS;
        }
        else
        {
            pMiniFilters = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(MINIFILTER_OBJECT) * 64, 'mfin');
            if (pMiniFilters == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            num2 = EnumMiniFilter(pMiniFilters);
            DbgPrint("num2:%d", num2);
            Information = sizeof(MINIFILTER_OBJECT) * num2;
            status = STATUS_SUCCESS;
        }
        break;
    case IOCTL_GetGDT:
        static PGDT_INFO pGdtInfo = NULL;
        static ULONG num3 = 0;

        if (pOutputData != NULL && OutputDataLength >= sizeof(GDT_INFO) * num3)
        {
            //memset(Callbacks, 0, sizeof(Callbacks));
            Information = sizeof(GDT_INFO) * num3;
            memcpy(pOutputData, pGdtInfo, Information);
            if (pGdtInfo != NULL) ExFreePoolWithTag(pGdtInfo, 'gdti');
            //DbgPrint("GetHotkeyInfo status = %X", GetHotkeyInfo());
            status = STATUS_SUCCESS;
        }
        else
        {
            pGdtInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(GDT_INFO) * 512, 'gdti');
            if (pGdtInfo == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            num3 = GetGDT(pGdtInfo);
            Information = sizeof(GDT_INFO) * num3;
            status = STATUS_SUCCESS;
        }
        break;
    case IOCTL_GetIDT:
        static PIDT_INFO pIdtInfo = NULL;
        static ULONG num4 = 0;

        if (pOutputData != NULL && OutputDataLength >= sizeof(IDT_INFO) * num4)
        {
            //memset(Callbacks, 0, sizeof(Callbacks));
            Information = sizeof(IDT_INFO) * num4;
            memcpy(pOutputData, pIdtInfo, Information);
            if (pIdtInfo != NULL) ExFreePoolWithTag(pIdtInfo, 'idti');
            //DbgPrint("GetHotkeyInfo status = %X", GetHotkeyInfo());
            status = STATUS_SUCCESS;
        }
        else
        {
            pIdtInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(IDT_INFO) * 512, 'idti');
            if (pIdtInfo == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            num4 = GetIDT(pIdtInfo);
            Information = sizeof(IDT_INFO) * num4;
            status = STATUS_SUCCESS;
        }
        break;
    case IOCTL_GetSSDTFuncByIndex:
        ULONG sysCallIndex1 = *(PULONG)pInputData;
        ULONG64 FuncAddr1 = (ULONG64)GetSSDTFuncAddrByIndex(sysCallIndex1);
        if (pOutputData != NULL && OutputDataLength >= sizeof(ULONG64)) {
            if (FuncAddr1 != 0) {
				DbgPrint("Index:%d | SSDTAddress:%llx", sysCallIndex1, FuncAddr1);
                memcpy(pOutputData, &FuncAddr1, sizeof(ULONG64));
				Information = sizeof(ULONG64);
				status = STATUS_SUCCESS;
            }
            else {
                Information = 0;
                status = STATUS_NO_MORE_ENTRIES;
            }
        }
        else {
            Information = 0;
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        break;
    case IOCTL_GetSSSDTFuncByIndex:
        ULONG sysCallIndex2 = *(PULONG)pInputData;
        ULONG64 FuncAddr2 = (ULONG64)GetSSSDTFuncAddrByIndex(sysCallIndex2);
        if (pOutputData != NULL && OutputDataLength >= sizeof(ULONG64)) {
            if (FuncAddr2 != 0) {
				DbgPrint("Index:%d | SSSDTAddress:%llx", sysCallIndex2, FuncAddr2);
                memcpy(pOutputData, &FuncAddr2, sizeof(ULONG64));
                Information = sizeof(ULONG64);
                status = STATUS_SUCCESS;
            }
            else {
                Information = 0;
                status = STATUS_NO_MORE_ENTRIES;
            }
        }
        else {
            Information = 0;
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        break;
    case IOCTL_EnumAttachDevices:
    {
        ULONG retLength = 0;
        DbgPrint("Enter IOCTL_EnumAttachDevices (global enumeration)\n");

        // 重新构建全局映射表（确保最新数据）
        status = BuildGlobalDeviceAttachmentMap();
        if (!NT_SUCCESS(status)) {
            DbgPrint("BuildGlobalDeviceAttachmentMap failed: 0x%X\n", status);
            break;
        }

        // 第一次调用：输出缓冲区为 NULL，返回所需大小
        if (OutputDataLength == 0) {
            ULONG requiredSize = 0;
            status = CalculateGlobalDataSize(&requiredSize);
            if (NT_SUCCESS(status)) {
                Information = requiredSize;
                DbgPrint("Global data size = %lu bytes\n", requiredSize);
            }
        }
        // 第二次调用：填充实际数据
        else {
            status = FillGlobalData(pOutputData, OutputDataLength, &retLength);
            if (!NT_SUCCESS(status)) {
                DbgPrint("FillGlobalData failed: 0x%X\n", status);
            }
            Information = retLength;
        }

        // 释放全局映射表（每次调用后释放，保证下次重新构建）
        FreeGlobalDeviceAttachmentMap();
        break;
    }

    case IOCTL_GetHalDispatchByIndex:
        if (pInputData != NULL && InputDataLength > 0)
        {
            ULONG index = *(PULONG)pInputData;
            if (pOutputData != NULL && OutputDataLength >= sizeof(PVOID)) {
                if (index <= 12) {
                    PVOID FuncAddr = ((PVOID*)HalDispatchTable)[index];
                    DbgPrint("Index:%d | HalDispatchAddress:%llx", index, (ULONG64)FuncAddr);
                    memcpy(pOutputData, &FuncAddr, sizeof(PVOID));
                    Information = sizeof(PVOID);
                    status = STATUS_SUCCESS;
                }
                else {
                    DbgPrint("Index:%d 不在正常范围内!", index);
                    Information = 0;
                    status = STATUS_NO_MORE_ENTRIES;
                }
            }
            else {
                Information = 0;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
		}
        break;
    case IOCTL_GetHalPrivateDispatchByIndex:
        if (pInputData != NULL && InputDataLength > 0)
        {
            ULONG index = *(PULONG)pInputData;
            if (pOutputData != NULL && OutputDataLength >= sizeof(PVOID)) {
                if (index <= 149) {
                    PVOID FuncAddr = ((PVOID*)HalPrivateDispatchTable)[index];
                    DbgPrint("Index:%d | HalPrivateDispatchAddress:%llx", index, (ULONG64)FuncAddr);
                    memcpy(pOutputData, &FuncAddr, sizeof(PVOID));
                    Information = sizeof(PVOID);
                    status = STATUS_SUCCESS;
                }
                else {
                    Information = 0;
                    status = STATUS_NO_MORE_ENTRIES;
                }
            }
            else {
                Information = 0;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        break;
    case IOCTL_InjectDllByApc:
        if (pInputData && InputDataLength >= sizeof(INJECT_INFO)) {
            INJECT_INFO InjectInfo = *(PINJECT_INFO)pInputData;
            status = InjectDllByApc(InjectInfo.ProcessId, InjectInfo.ThreadId, InjectInfo.DllPath);
            Information = 0;
            break;
        }
    case IOCTL_DumpKernelModule:
    {
        __try {
            // 验证输入缓冲区
            if (InputDataLength < sizeof(stMemory)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            // 拷贝输入结构
            memcpy(&stMemory, pInputData, sizeof(stMemory));

            // 验证用户输出缓冲区可写
            __try {
                ProbeForWrite(stMemory.pData, stMemory.Size, sizeof(UCHAR));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_ACCESS_VIOLATION;
                break;
            }

            // 分配内核缓冲区
            PVOID pKernelBuffer = ExAllocatePool2(POOL_FLAG_NON_PAGED,
                stMemory.Size,
                'pmdK');
            if (!pKernelBuffer) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            // 清零（确保无效页填充0）
            RtlZeroMemory(pKernelBuffer, stMemory.Size);

            SIZE_T BytesTransferred = 0;
            //DbgPrint("IRQL at entry: %d\n", KeGetCurrentIrql());
            // 调用内核DUMP函数（只传内核缓冲区）
            status = DumpKernelModule(
                (PVOID)stMemory.Addr,   // 目标内核模块基址
                pKernelBuffer,           // 内核输出缓冲区
				stMemory.Size,          // 输出缓冲区大小
                &BytesTransferred        // 实际传输字节数
            );

            // 成功后拷贝到用户空间
            if (NT_SUCCESS(status) && BytesTransferred > 0) {
                __try {
                    memcpy(stMemory.pData, pKernelBuffer, BytesTransferred);
                    Information = BytesTransferred;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    status = GetExceptionCode();
                    Information = 0;
                }
            }

            // 清理内核缓冲区
            ExFreePoolWithTag(pKernelBuffer, 'pmdK');
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        break;
    }
    case IOCTL_GetObjectInfo:
        __try {
            ULONG Index = *(PULONG)pInputData;
            OBJECT_INFORMATION ObjInfo = { 0 };
            status = GetObjectInfo(Index, &ObjInfo);
            if (pOutputData != NULL && OutputDataLength >= sizeof(OBJECT_INFORMATION)) {
                memcpy(pOutputData, &ObjInfo, sizeof(OBJECT_INFORMATION));
                Information = sizeof(OBJECT_INFORMATION);
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        break;
    // 注意:如果设置了错误码会导致设置的Information被忽略
    case IOCTL_EnumWfpCallouts:
    {
        // 静态缓存（保留你的需求）
        static PWFP_CALLOUT_INFO pCalloutCache = NULL;
        static ULONG calloutCount = 0;
        // 总数据大小
        ULONG totalSize = 0;

        // ===================== 核心修复1：初始化信息长度 =====================
        Information = 0;
        status = STATUS_SUCCESS;

        // ===================== 第一步：无缓存 → 重新枚举生成缓存 =====================
        if (pCalloutCache == NULL || calloutCount == 0)
        {
            // 释放旧缓存（防御性代码）
            if (pCalloutCache)
            {
                ExFreePoolWithTag(pCalloutCache, 'WfpE');
                pCalloutCache = NULL;
                calloutCount = 0;
            }

            // 枚举Callout（你的正常逻辑）
            status = EnumWfpCallouts(&pCalloutCache, &calloutCount);
            if (!NT_SUCCESS(status))
            {
                DbgPrint("EnumCallouts failed: %08X\n", status);
                // 失败后清空缓存
                pCalloutCache = NULL;
                calloutCount = 0;
                break;
            }
        }

        // 计算总大小
        totalSize = sizeof(WFP_CALLOUT_INFO) * calloutCount;
        Information = totalSize;
		DbgPrint("[IOCTL] 枚举到 %lu 个Callout，总数据大小: %lu bytes\n", calloutCount, totalSize);

        // ===================== 核心修复2：缓冲区为空/不足 → 返回标准错误码 =====================
        if (pOutputData == NULL || OutputDataLength < totalSize)
        {
            // ✅ 标准返回：告诉用户态需要多大缓冲区
            status = STATUS_SUCCESS;
            break;
        }

        // ===================== 核心修复3：缓冲区足够 → 拷贝数据 + 清空缓存 =====================
        if (pCalloutCache != NULL && totalSize > 0)
        {
            // 拷贝正确数据到用户态
            memcpy(pOutputData, pCalloutCache, totalSize);

            // 打印调试：确认内核缓存数据正确（可选，验证用）
            DbgPrint("[IOCTL] 拷贝数据成功，第一个Callout地址: 0x%llx\n",
                pCalloutCache[0].ClassifyFn);

            // 释放缓存（一次性使用，符合你的逻辑）
            ExFreePoolWithTag(pCalloutCache, 'WfpE');
            pCalloutCache = NULL;
            calloutCount = 0;
        }

        // ✅ 数据拷贝完成，返回成功
        status = STATUS_SUCCESS;
        break;
    }

    case IOCTL_EnumWfpFilters:
    {
        static PWFP_FILTER_INFO pFilterCache = NULL;
        static ULONG filterCount = 0;
        ULONG totalSize = 0;

        // 初始化：固定返回成功，长度清零
        Information = 0;
        status = STATUS_SUCCESS;

        DbgPrint("[IOCTL] 开始处理 WFP Filter 请求\n");

        // 缓存为空 → 重新枚举生成缓存
        if (pFilterCache == NULL)
        {
            // 防御性释放旧缓存
            if (pFilterCache)
            {
                ExFreePoolWithTag(pFilterCache, 'WfpE');
                pFilterCache = NULL;
                filterCount = 0;
            }

            // 枚举 Filter
            status = EnumWfpFilters(&pFilterCache, &filterCount);
            DbgPrint("[IOCTL] EnumWfpFilters 状态: %08X，枚举数量: %lu\n", status, filterCount);

            if (!NT_SUCCESS(status))
            {
                DbgPrint("EnumFilters failed: %08X\n", status);
                pFilterCache = NULL;
                filterCount = 0;
                break;
            }
        }

        // 计算总数据长度
        totalSize = sizeof(WFP_FILTER_INFO) * filterCount;
        Information = totalSize;
        DbgPrint("[IOCTL] Filter 总数据大小: %lu\n", totalSize);

        // 缓冲区为空 / 不足 → 仅返回所需长度，不拷贝数据（返回SUCCESS）
        if (pOutputData == NULL || OutputDataLength < totalSize)
        {
            DbgPrint("[IOCTL] Filter 缓冲区不足，返回长度: %lu\n", totalSize);
            break;
        }

        // 缓冲区足够 → 拷贝数据到用户态，并释放缓存
        if (pFilterCache != NULL && totalSize > 0)
        {
            memcpy(pOutputData, pFilterCache, totalSize);
            DbgPrint("[IOCTL] Filter 数据拷贝成功\n");

            // 释放缓存
            ExFreePoolWithTag(pFilterCache, 'WfpE');
            pFilterCache = NULL;
            filterCount = 0;
            DbgPrint("[IOCTL] Filter 缓存已释放\n");
        }

        status = STATUS_SUCCESS;
        break;
    }
    // ====================== 消息钩子枚举 IOCTL ======================
    case IOCTL_EnumMsgHook:
    {
        static PWIN32K_MSG_HOOK_INFO pMsgHookCache = NULL;
        static ULONG msgHookCount = 0;
        ULONG totalSize = 0;

        // 初始化：固定返回成功，长度清零
        Information = 0;
        status = STATUS_SUCCESS;

        DbgPrint("[IOCTL] 开始处理 MsgHook 请求\n");

        // 缓存为空 → 重新枚举生成缓存
        if (pMsgHookCache == NULL)
        {
            // 防御性释放旧缓存
            if (pMsgHookCache)
            {
                ExFreePoolWithTag(pMsgHookCache, 'MsHk');
                pMsgHookCache = NULL;
                msgHookCount = 0;
            }

            // 枚举消息钩子
            //status = EnumerateMsgHook(&pMsgHookCache, &msgHookCount);
            status = EnumerateMsgHook_New(&pMsgHookCache, &msgHookCount);
            DbgPrint("[IOCTL] EnumerateMsgHook 状态: %08X，枚举数量: %lu\n", status, msgHookCount);

            if (!NT_SUCCESS(status))
            {
                DbgPrint("EnumerateMsgHook failed: %08X\n", status);
                pMsgHookCache = NULL;
                msgHookCount = 0;
                break;
            }
        }

        // 计算总数据长度
        totalSize = sizeof(WIN32K_MSG_HOOK_INFO) * msgHookCount;
        Information = totalSize;
        DbgPrint("[IOCTL] MsgHook 总数据大小: %lu\n", totalSize);

        // 缓冲区为空 / 不足 → 仅返回所需长度，不拷贝数据（返回SUCCESS）
        if (pOutputData == NULL || OutputDataLength < totalSize)
        {
            DbgPrint("[IOCTL] MsgHook 缓冲区不足，返回长度: %lu\n", totalSize);
            break;
        }

        // 缓冲区足够 → 拷贝数据到用户态，并释放缓存
        if (pMsgHookCache != NULL && totalSize > 0)
        {
            memcpy(pOutputData, pMsgHookCache, totalSize);
            DbgPrint("[IOCTL] MsgHook 数据拷贝成功\n");

            // 释放缓存
            ExFreePoolWithTag(pMsgHookCache, 'MsHk');
            pMsgHookCache = NULL;
            msgHookCount = 0;
            DbgPrint("[IOCTL] MsgHook 缓存已释放\n");
        }

        status = STATUS_SUCCESS;
        break;
    }

    // ====================== 事件钩子枚举 IOCTL ======================
    case IOCTL_EnumEventHook:
    {
        static PWIN32K_EVENT_HOOK_INFO pEventHookCache = NULL;
        static ULONG eventHookCount = 0;
        ULONG totalSize = 0;

        // 初始化：固定返回成功，长度清零
        Information = 0;
        status = STATUS_SUCCESS;

        DbgPrint("[IOCTL] 开始处理 EventHook 请求\n");

        // 缓存为空 → 重新枚举生成缓存
        if (pEventHookCache == NULL)
        {
            // 防御性释放旧缓存
            if (pEventHookCache)
            {
                ExFreePoolWithTag(pEventHookCache, 'EvHk');
                pEventHookCache = NULL;
                eventHookCount = 0;
            }

            // 枚举事件钩子
            status = EnumerateEventHook_Win11(&pEventHookCache, &eventHookCount);
            //status = EnumerateEventHook(&pEventHookCache, &eventHookCount);
            DbgPrint("[IOCTL] EnumerateWinEventHook 状态: %08X，枚举数量: %lu\n", status, eventHookCount);

            if (!NT_SUCCESS(status))
            {
                DbgPrint("EnumerateWinEventHook failed: %08X\n", status);
                pEventHookCache = NULL;
                eventHookCount = 0;
                break;
            }
        }

        // 计算总数据长度
        totalSize = sizeof(WIN32K_EVENT_HOOK_INFO) * eventHookCount;
        Information = totalSize;
        DbgPrint("[IOCTL] EventHook 总数据大小: %lu\n", totalSize);

        // 缓冲区为空 / 不足 → 仅返回所需长度，不拷贝数据（返回SUCCESS）
        if (pOutputData == NULL || OutputDataLength < totalSize)
        {
            DbgPrint("[IOCTL] EventHook 缓冲区不足，返回长度: %lu\n", totalSize);
            break;
        }

        // 缓冲区足够 → 拷贝数据到用户态，并释放缓存
        if (pEventHookCache != NULL && totalSize > 0)
        {
            memcpy(pOutputData, pEventHookCache, totalSize);
            DbgPrint("[IOCTL] EventHook 数据拷贝成功\n");

            // 释放缓存
            ExFreePoolWithTag(pEventHookCache, EVENT_HOOK_POOL_TAG);
            pEventHookCache = NULL;
            eventHookCount = 0;
            DbgPrint("[IOCTL] EventHook 缓存已释放\n");
        }

        status = STATUS_SUCCESS;
        break;
    }
    // ====================== 热键枚举 IOCTL ======================
    case IOCTL_EnumHotkey:
    {
        // 静态缓存（和 EventHook 完全一致）
        static PWIN32K_HOTKEY_INFO pHotkeyCache = NULL;
        static ULONG hotkeyCount = 0;
        ULONG totalSize = 0;

        // 初始化：固定返回成功，长度清零
        Information = 0;
        status = STATUS_SUCCESS;

        DbgPrint("[IOCTL] 开始处理 Hotkey 请求\n");

        // 缓存为空 → 重新枚举生成缓存
        if (pHotkeyCache == NULL)
        {
            // 防御性释放旧缓存
            if (pHotkeyCache)
            {
                ExFreePoolWithTag(pHotkeyCache, HOTKEY_POOL_TAG);
                pHotkeyCache = NULL;
                hotkeyCount = 0;
            }

            // 调用热键枚举函数
            status = EnumHotkey(&pHotkeyCache, &hotkeyCount);
            DbgPrint("[IOCTL] EnumHotkey 状态: %08X，枚举数量: %lu\n", status, hotkeyCount);

            if (!NT_SUCCESS(status))
            {
                DbgPrint("EnumHotkey 失败: %08X\n", status);
                pHotkeyCache = NULL;
                hotkeyCount = 0;
                break;
            }
        }

        // 计算总数据长度
        totalSize = sizeof(WIN32K_HOTKEY_INFO) * hotkeyCount;
        Information = totalSize;
        DbgPrint("[IOCTL] Hotkey 总数据大小: %lu\n", totalSize);

        // 缓冲区为空 / 不足 → 仅返回所需长度，不拷贝数据
        if (pOutputData == NULL || OutputDataLength < totalSize)
        {
            DbgPrint("[IOCTL] Hotkey 缓冲区不足，返回长度: %lu\n", totalSize);
            break;
        }

        // 缓冲区足够 → 拷贝数据到用户态，并释放缓存
        if (pHotkeyCache != NULL && totalSize > 0)
        {
            memcpy(pOutputData, pHotkeyCache, totalSize);
            DbgPrint("[IOCTL] Hotkey 数据拷贝成功\n");

            // 释放缓存（和 EventHook 一致）
            ExFreePoolWithTag(pHotkeyCache, HOTKEY_POOL_TAG);
            pHotkeyCache = NULL;
            hotkeyCount = 0;
            DbgPrint("[IOCTL] Hotkey 缓存已释放\n");
        }

        status = STATUS_SUCCESS;
        break;
    }

    default: 
        break;
    }
    
    //Cleanup:
    pIrp->IoStatus.Status = status;             //Ring3 GetLastError();
	pIrp->IoStatus.Information = Information;   //Ring3 lpBytesReturned
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);  //将Irp返回给Io管理器
    return status;                            //Ring3 DeviceIoControl()返回值
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
    UNICODE_STRING  DeviceLinkName;
    PDEVICE_OBJECT  v1 = NULL;
    PDEVICE_OBJECT  pDeleteDeviceObject = NULL;

	if (MyAdvancedOptions.DenyCreateProcess) SetProcessMonitorStatus(FALSE);
	if (MyAdvancedOptions.DenyLoadDriver) SetImageMonitorStatus(FALSE);
	if (MyAdvancedOptions.DenyAccessRegistry) SetRegMonitorStatus(pDriverObject, FALSE);
	if (MyAdvancedOptions.DenyRemoteThread) SetThreadMonitorStatus(FALSE);
    if (MyAdvancedOptions.IsProtectProcess) SetProcessProtectionStatus(FALSE);
    if (MyAdvancedOptions.IsProtectThread) SetThreadProtectionStatus(FALSE);
    //UnhookDiskWriteDispatch();
    //UnhookNtfsDispatch();
    //UnregisterMiniFilter();
	//InterlockedExchangePointer((PUCHAR volatile*)Notepad_EProcess + 0x440, (HANDLE)30256);

    /*UNICODE_STRING targetDeviceName = {0};
    RtlInitUnicodeString(&targetDeviceName, L"\\Device\\Harddisk0\\DR0");
    PDEVICE_OBJECT targetDeviceObject = NULL;
    PFILE_OBJECT fileobject = NULL;
    status = IoGetDeviceObjectPointer(&targetDeviceName, FILE_READ_ACCESS | FILE_WRITE_ACCESS, &fileobject, &targetDeviceObject);
    if (!NT_SUCCESS(status)) {
        DbgPrint("IoGetDeviceObjectPointer Failed!NTSTATUS = %X", status);
    }
    else {
        IoDetachDevice(targetDeviceObject);
    }*/

    RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
    IoDeleteSymbolicLink(&DeviceLinkName);

    pDeleteDeviceObject = pDriverObject->DeviceObject;
    while (pDeleteDeviceObject != NULL)
    {
        v1 = pDeleteDeviceObject->NextDevice;
        IoDeleteDevice(pDeleteDeviceObject);
        pDeleteDeviceObject = v1;
    }
    DbgPrint("Driver is unloaded!");
    //FltUnregisterFilter(hFilter);
}
