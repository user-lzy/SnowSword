#include "DriverControl.h"
#include "EnumCallbacks.h"
#include "EnumDriverInfo.h"
#include "EnumTimer.h"
#include "File.h"
#include "GDT.h"
//#include "GUI.h"
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

//#include "global.h"
//#include "winioctl.h"
//#include "ntddisk.h"

/*#define DEBUG
#ifdef DEBUG
#define DBG_PRINTF(_fmt, ...) DbgPrint(_fmt, __VA_ARGS__)
#else
#define DBG_PRINTF(_fmt, ...) { NOTHING; }
#endif*/

//设备与设备之间通信
#define DEVICE_OBJECT_NAME  L"\\Device\\SnowSword"
//设备与Ring3之间通信
#define DEVICE_LINK_NAME    L"\\DosDevices\\SnowSword"

//DRIVER_INITIALIZE DriverEntry;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS DispatchRoutine(PDEVICE_OBJECT  pDeviceObject, PIRP pIrp);
NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT  pDeviceObject, PIRP pIrp);
//DRIVER_UNLOAD DriverUnload;
VOID DriverUnload(PDRIVER_OBJECT pDriverObject);

NTSTATUS status = STATUS_SUCCESS;
HANDLE hEvent = NULL;
PKEVENT pUserEvent = NULL;
PKEVENT pKernelEvent = NULL;
struct AdvancedOptions MyAdvancedOptions = { 0 };
struct MemoryStruct stMemory = { 0 };
PEPROCESS Notepad_EProcess = NULL;
PDEVICE_OBJECT g_TargetDeviceObject = NULL;

//extern PVOID HalDispatchTable;
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

	//将驱动挂靠到Disk.sys
	//获取Disk.sys驱动对象
	/*UNICODE_STRING targetDeviceName = {0};
	PDEVICE_OBJECT targetDeviceObject = NULL;
    RtlInitUnicodeString(&targetDeviceName, L"\\Device\\Harddisk0\\DR0");
	PFILE_OBJECT fileobject = NULL;
    status = IoGetDeviceObjectPointer(&targetDeviceName, FILE_READ_ACCESS | FILE_WRITE_ACCESS, &fileobject, &g_TargetDeviceObject);
    if (!NT_SUCCESS(status)) {
		DbgPrint("IoGetDeviceObjectPointer Failed!NTSTATUS = %X", status);
        IoDeleteDevice(pDeviceObject);
        return status;
    }
    status = IoAttachDeviceToDeviceStackSafe(pDeviceObject, g_TargetDeviceObject, &targetDeviceObject);
    if (!NT_SUCCESS(status)) {
		DbgPrint("IoAttachDeviceToDeviceStackSafe Failed!NTSTATUS = %X", status);
        IoDeleteDevice(pDeviceObject);
		ObDereferenceObject(g_TargetDeviceObject);
        g_TargetDeviceObject = NULL;
        return status;
    }*/
    
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

	/*PWCHAR ProcessPath = ExAllocatePool2(POOL_FLAG_NON_PAGED, 512 * sizeof(WCHAR), 'Path');
    if (GetProcessChangedPath((HANDLE)14520, &ProcessPath)) DbgPrint("ProcessPath:%ws", ProcessPath);
	else DbgPrint("GetProcessChangedPath Failed!");
	ExFreePoolWithTag(ProcessPath, 'Path');*/

    //TestXCBFunction();

    //EnumMiniFilter(NULL);

	//UNICODE_STRING uFileName = RTL_CONSTANT_STRING(L"\\??\\D:\\Programs\\VisualFreeBasic6.0\\Projects\\MyProjects\\SnowSword32\\release64\\新建 Microsoft Excel 工作表 (2).xlsx");
    //DbgPrint("IsFileReallyDeleted:%d", IsFileReallyDeleted(&uFileName));

    //DbgPrint("NtfsDecrementCleanupCounts : 0x%p", GetNtfsDecrementCleanupCounts());

    //EnumCreateProcessNotify();

    //打印IDT表信息
    //PrintIDT();

    KeServiceDescriptorTable = GetKeServiceDescriptorTable();
    KeServiceDescriptorTableShadow = GetKeServiceDescriptorTableShadow();

    //EnumFilterChains();

	//DbgPrint("PsGetNextProcessThread: 0x%p", FindPsGetNextProcessThread());

	//DbgPrint("PspCidTableAddress: 0x%p", GetPspCidTable());

    //EnumProcesses();
    /*SIZE_T bytRead = 0;
    ULONG data = 0;
    DbgPrint("Read TDI.sys:%X, bytRead:%d", VxkCopyMemory(&data, (PVOID)0xFFFFF80373BE0000, sizeof(ULONG), &bytRead), bytRead);
    DbgPrint("read data:%d", data);*/

	// 打印GDT表信息
	//EnumGDT();

	//打印SSDT和Shadow SSDT
    //GetKeServiceDescriptorTable();
	//GetKeServiceDescriptorTableShadow();

    //打印进程所有的定时器
    //EnumProcessTimers((HANDLE)11432, NULL);

	//枚举CreateProcess回调函数
	//EnumCreateProcessNotify();

    //枚举所有过滤驱动
	//EnumAllFilters(pDriverObject);

    //枚举MiniFilter
	//EnumMiniFilter();

	//GetObjectTypeNames();

    //保护MBR
    //HookDiskWriteDispatch();

    //保护文件
    ///HookNtfsDispatch();

    //注册MiniFilter
    //RegisterMiniFilter(pDriverObject);

    //强删文件
    //UNICODE_STRING uFileName = RTL_CONSTANT_STRING(L"\\??\\C:\\Users\\21607\\Desktop\\TestOpenFile - 副本.exe");
    //ForceDeleteFile(uFileName);
    //RtlInitEmptyUnicodeString
    //打印IDT
    //PrintIDT();
    //RtlUnicodeStringPrintf
	//枚举GDT
	//EnumGDT();

    //枚举Io定时器
    //EnumIoTimers();

    //枚举DPC定时器
    //EnumDpc();

    //FindPspExitThread();

    //DKOM
    //Notepad_EProcess = GetEProcess((HANDLE)30256);
	//InterlockedExchangePointer((PUCHAR volatile*)Notepad_EProcess + 0x440, (HANDLE)1);

    //EnumerateFilterDrivers();

    /*DbgPrint("驱动名字 = %wZ \n", pDriverObject->DriverName);
    DbgPrint("驱动起始地址 = %p | 大小 = %llx | 结束地址 %p \n", pDriverObject->DriverStart, pDriverObject->DriverSize, (ULONG64)pDriverObject->DriverStart + pDriverObject->DriverSize);

    DbgPrint("卸载地址 = %p\n", pDriverObject->DriverUnload);
    DbgPrint("IRP_MJ_READ地址 = %p\n", pDriverObject->MajorFunction[IRP_MJ_READ]);
    DbgPrint("IRP_MJ_WRITE地址 = %p\n", pDriverObject->MajorFunction[IRP_MJ_WRITE]);
    DbgPrint("IRP_MJ_CREATE地址 = %p\n", pDriverObject->MajorFunction[IRP_MJ_CREATE]);
    DbgPrint("IRP_MJ_CLOSE地址 = %p\n", pDriverObject->MajorFunction[IRP_MJ_CLOSE]);
    DbgPrint("IRP_MJ_DEVICE_CONTROL地址 = %p\n", pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]);

    // 输出完整的调用号
    for (auto i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DbgPrint("IRP_MJ调用号 = %d | 函数地址 = %p \r\n", i, pDriverObject->MajorFunction[i]);
    }*/

    return STATUS_SUCCESS;
}

//派遣历程
NTSTATUS DispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
    //PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    //判断请求是否为CreateFile发起
    /*if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_CREATE) {
		DeviceObject->Flags |= DO_DEVICE_INITIALIZING; //禁止ZwUnloadDriver卸载,但会使驱动无法被打开
	}
    else if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_CLOSE) {
        DeviceObject->Flags |= ~DO_DEVICE_INITIALIZING; //允许ZwUnloadDriver卸载
    }*/
    Irp->IoStatus.Information = 0;		//设置操作的字节数为0
    Irp->IoStatus.Status = STATUS_SUCCESS;	//返回成功
    IoCompleteRequest(Irp, IO_NO_INCREMENT);	//指示完成此IRP
    //DbgPrint("进入DispatchRoutine函数!");
    return STATUS_SUCCESS; //返回成功
    /*status = STATUS_SUCCESS;

    switch (irpStack->MajorFunction) {
    case IRP_MJ_READ: {
        LARGE_INTEGER byteOffset = irpStack->Parameters.Read.ByteOffset;
        ULONG length = irpStack->Parameters.Read.Length;

        if (byteOffset.QuadPart == 0 && length == 512) { // 判断是否是读取 MBR
			DbgPrint("已拒绝MBR读操作并返回假结果!");
            PUCHAR fakeMBR = ExAllocatePool(NonPagedPool, 512);
            if (fakeMBR) {
                RtlFillMemory(fakeMBR, 512, 0x00); // 填充伪造的 MBR 数据
                RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, fakeMBR, 512);
                ExFreePool(fakeMBR);
                Irp->IoStatus.Information = 512;
                Irp->IoStatus.Status = STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_SUCCESS;
            }
            else {
                Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        // 将 IRP 传递给下层驱动
        return IoCallDriver(g_TargetDeviceObject, Irp);
        break;
    }
    case IRP_MJ_WRITE: {
        LARGE_INTEGER byteOffset = irpStack->Parameters.Write.ByteOffset;
        ULONG length = irpStack->Parameters.Write.Length;

        if (byteOffset.QuadPart == 0 && length == 512) { // 判断是否是写入 MBR
			DbgPrint("已拒绝MBR写操作!");
            Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_ACCESS_DENIED;
        }
        // 将 IRP 传递给下层驱动
        return IoCallDriver(g_TargetDeviceObject, Irp);
        break;
    }
    default:
        // 完成IRP
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;    // bytes xfered
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
        //break;
    }*/
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

    /*if (IoControlCode == IOCTL_DISK_FORMAT_TRACKS || IoControlCode == IOCTL_DISK_FORMAT_PARTITIONS) {
		DbgPrint("已拦截格式化请求!");
        pIrp->IoStatus.Status = STATUS_ACCESS_DENIED;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_ACCESS_DENIED;
    }*/

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
        PDO_SOMETHING pSomeThing = (PDO_SOMETHING)pInputData;

        static PPROCESS_TIMER pProcessTimer = NULL;
        //static HANDLE PID = 0;
        static ULONG num1 = 0;
		DbgPrint("OutputDataLength:%d, sizeof(PROCESS_TIMER)*num1:%d", (DWORD)OutputDataLength, (DWORD)(sizeof(PROCESS_TIMER) * num1));

        if (OutputDataLength != 0 && OutputDataLength >= sizeof(PROCESS_TIMER) * num1)
        {
            DbgPrint("1");
            //memset(Callbacks, 0, sizeof(Callbacks));
            if (pProcessTimer != NULL) {
                Information = sizeof(PROCESS_TIMER) * num1;
                memcpy(pOutputData, pProcessTimer, Information);
                ExFreePoolWithTag(pProcessTimer, 'cbin');
                status = STATUS_SUCCESS;
            }
        }
        else
        {
            DbgPrint("2");
            pProcessTimer = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PROCESS_TIMER) * 512, 'cbin');
            if (pProcessTimer == NULL)
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            //检查pSomeThing的有效性
			if (pSomeThing == NULL || !MmIsAddressValid(pSomeThing))
			{
				DbgPrint("pSomeThing is NULL or invalid!");
				status = STATUS_INVALID_PARAMETER;
				break;
			}
            num1 = EnumProcessTimers(pSomeThing->ProcessId, pProcessTimer);
            Information = sizeof(PROCESS_TIMER) * num1;
            status = STATUS_SUCCESS;
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
		PDRIVER_OBJECT pDriverObject = *(PDRIVER_OBJECT*)pInputData;
        PATTACH_DEVICE_INFO pAttachDeviceInfo = (PATTACH_DEVICE_INFO)pOutputData;

        if (pOutputData != NULL && OutputDataLength >= sizeof(ATTACH_DEVICE_INFO))
        {
            status = EnumFilterChains(pDriverObject, pAttachDeviceInfo);
            Information = sizeof(ATTACH_DEVICE_INFO);
        }
        else
        {
			DbgPrint("sizeof(ATTACH_DEVICE_INFO)=%d", (DWORD)sizeof(ATTACH_DEVICE_INFO));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        break;

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
        __try {
            stMemory = *(PMemoryStruct)pInputData;
            //ProbeForRead((PVOID)stMemory.Addr, stMemory.Size, sizeof(ULONG));
            SIZE_T BytesTransferred = 0;
            status = DumpKernelModule((PVOID)stMemory.Addr, stMemory.pData, &BytesTransferred);
            Information = BytesTransferred;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }
        break;
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
