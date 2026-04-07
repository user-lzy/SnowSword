#include "File.h"

// ============================================================================
// 池标签定义
// ============================================================================
#define POOL_TAG_PATH        'htpC'  // CtpH - Copy Path
#define POOL_TAG_BUFFER      'fubC'  // Cubf - Copy Buffer
#define POOL_TAG_WORKITEM    'kwoC'  // Cowk - Copy Work Item

// ============================================================================
// 工作队列结构（用于迭代替代递归）
// ============================================================================
typedef struct _COPY_WORK_ITEM {
    LIST_ENTRY ListEntry;
    UNICODE_STRING SourcePath;  // 深拷贝的源路径
    UNICODE_STRING DestPath;    // 深拷贝的目标路径
    BOOLEAN IsDirectory;
} COPY_WORK_ITEM, * PCOPY_WORK_ITEM;

// ============================================================================
// 辅助函数：IRQL检查
// ============================================================================
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS CheckIrqlPassiveLevel(VOID) {
    if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
        DbgPrint("[ERROR] IRQL is %d, must be PASSIVE_LEVEL!\n", KeGetCurrentIrql());
        return STATUS_INVALID_DEVICE_STATE;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)

{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);
    //*Irp->UserIosb = Irp->IoStatus;
    if (Irp->UserEvent) {
        KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);
        //ExFreePoolWithTag(Irp->UserEvent, 'kevn');
    }
    /*if (Irp->MdlAddress)
    {
        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }*/
    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
    //return STATUS_SUCCESS;
}

NTSTATUS
IrpCreateFile(
    OUT PFILE_OBJECT* FileObject,
    IN ACCESS_MASK  DesiredAccess,
    IN PUNICODE_STRING  FilePath,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize  OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer  OPTIONAL,
    IN ULONG  EaLength)
{
	UNREFERENCED_PARAMETER(AllocationSize);
    NTSTATUS ntStatus;
    HANDLE hFile;
    PFILE_OBJECT pFile;//, _FileObject;
    //UNICODE_STRING UniDeviceNameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PDEVICE_OBJECT DeviceObject, RealDevice;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;
    //ACCESS_STATE AccessState;
    //AUX_ACCESS_DATA AuxData;
    //IO_SECURITY_CONTEXT SecurityContext;
    if (!FileObject) return STATUS_UNSUCCESSFUL;

    //初始化结构体
    //KEVENT* pkEvent = NULL;
    ACCESS_STATE* pAccessState = NULL;
    AUX_ACCESS_DATA * pAuxData = NULL;
    IO_SECURITY_CONTEXT* pSecurityContext = NULL;

    //为结构体分配空间
    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    pAccessState = (ACCESS_STATE*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(ACCESS_STATE), 'aest');
    if (!pAccessState) {
        DbgPrint("Failed to allocate memory for AccessState\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    pAuxData = (AUX_ACCESS_DATA*)ExAllocatePool2(POOL_FLAG_NON_PAGED, 0xE0, 'aaed');
    if (!pAuxData) {
        DbgPrint("Failed to allocate memory for AUX_ACCESS_DATA\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    pSecurityContext = (IO_SECURITY_CONTEXT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(IO_SECURITY_CONTEXT), 'iosc');
    if (!pSecurityContext) {
        DbgPrint("Failed to allocate memory for IO_SECURITY_CONTEXT\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    // 初始化对象
    RtlZeroMemory(pkEvent, sizeof(KEVENT));
    RtlZeroMemory(pAccessState, sizeof(ACCESS_STATE));
    RtlZeroMemory(pAuxData, sizeof(AUX_ACCESS_DATA));
    RtlZeroMemory(pSecurityContext, sizeof(IO_SECURITY_CONTEXT));

    //RtlInitUnicodeString(&UniDeviceNameString, L"//DosDevices//*://");
    //UniDeviceNameString.Buffer[12] = FilePath->Buffer[0];
    InitializeObjectAttributes(&ObjectAttributes, FilePath, OBJ_KERNEL_HANDLE, NULL, NULL);
    ntStatus = IoCreateFile(
        &hFile,
        DesiredAccess | SYNCHRONIZE,  // 保留同步权限，补充传入的访问权限
        &ObjectAttributes,
        IoStatusBlock,
        AllocationSize,               // 使用传入的AllocationSize
        FileAttributes,               // 使用传入的FileAttributes（目录/文件）
        ShareAccess,                  // 使用传入的ShareAccess
        CreateDisposition,            // 使用传入的CreateDisposition（创建/打开）
        CreateOptions,                // 使用传入的CreateOptions
        EaBuffer,                     // 使用传入的EaBuffer
        EaLength,                     // 使用传入的EaLength
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING);
    DbgPrint("IoCreateFile status:%d", ntStatus);
    if (!NT_SUCCESS(ntStatus)) goto Cleanup;

    ntStatus = ObReferenceObjectByHandle(hFile,
        FILE_ALL_ACCESS, // ACCESS_MASK
        *IoFileObjectType,
        KernelMode,
        &pFile,
        0);
    NtClose(hFile);
    DbgPrint("ObReferenceObjectByHandle status:%d", ntStatus);
    if (!NT_SUCCESS(ntStatus)) goto Cleanup;

    PDEVICE_OBJECT fsdDevice = IoGetRelatedDeviceObject(pFile);//获得与文件对象相关联的设备对象

    DeviceObject = pFile->Vpb->DeviceObject;
    RealDevice = pFile->Vpb->RealDevice;
    //ObDereferenceObject(pFile);
    InitializeObjectAttributes(&ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE, 0, NULL);
    /*ntStatus = ObCreateObject(KernelMode,
        *IoFileObjectType,
        &ObjectAttributes,
        KernelMode,
        NULL,
        sizeof(FILE_OBJECT),
        0,
        0,
        &_FileObject);
    DbgPrint("ObCreateObject status:%d", ntStatus);
    if (!NT_SUCCESS(ntStatus)) goto Cleanup;*/

    Irp = IoAllocateIrp(fsdDevice->StackSize, FALSE);
    if (Irp == NULL)
    {
        //ObDereferenceObject(_FileObject);
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);
    /*RtlZeroMemory(_FileObject, sizeof(FILE_OBJECT));
    _FileObject->Type = IO_TYPE_FILE;
    _FileObject->Size = sizeof(FILE_OBJECT);
    _FileObject->DeviceObject = RealDevice;
    _FileObject->Flags = FO_SYNCHRONOUS_IO;
    RtlInitUnicodeString(&_FileObject->FileName, &FilePath->Buffer[2]);
    KeInitializeEvent(&_FileObject->Lock, SynchronizationEvent, FALSE);
    KeInitializeEvent(&_FileObject->Event, NotificationEvent, FALSE);*/
    RtlZeroMemory(pAuxData, sizeof(AUX_ACCESS_DATA));
    ntStatus = SeCreateAccessState(pAccessState,
        pAuxData,
        DesiredAccess,
        IoGetFileObjectGenericMapping());
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("SeCreateAccessState status:%d", ntStatus);
        IoFreeIrp(Irp);
        //ObDereferenceObject(_FileObject);
        goto Cleanup;
    }

    pSecurityContext->SecurityQos = NULL;
    pSecurityContext->AccessState = pAccessState;
    pSecurityContext->DesiredAccess = DesiredAccess;
    pSecurityContext->FullCreateOptions = 0;

    Irp->MdlAddress = NULL;
    Irp->AssociatedIrp.SystemBuffer = EaBuffer;
    Irp->Flags = IRP_CREATE_OPERATION | IRP_SYNCHRONOUS_API;
    Irp->RequestorMode = KernelMode;
    Irp->UserIosb = IoStatusBlock;
    Irp->UserEvent = pkEvent;
    Irp->PendingReturned = FALSE;
    Irp->Cancel = FALSE;
    Irp->CancelRoutine = NULL;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
    Irp->Tail.Overlay.OriginalFileObject = pFile;// _FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_CREATE;
    IrpSp->DeviceObject = fsdDevice;// DeviceObject;
    IrpSp->FileObject = pFile;// _FileObject;
    IrpSp->Parameters.Create.SecurityContext = pSecurityContext;
    IrpSp->Parameters.Create.Options = (CreateDisposition << 24) | CreateOptions;
    IrpSp->Parameters.Create.FileAttributes = (USHORT)FileAttributes;
    IrpSp->Parameters.Create.ShareAccess = (USHORT)ShareAccess;
    IrpSp->Parameters.Create.EaLength = EaLength;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    ntStatus = IoCallDriver(fsdDevice, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, 0);

    ntStatus = IoStatusBlock->Status;
    if (!NT_SUCCESS(ntStatus))
    {
        //_FileObject->DeviceObject = NULL;
        //ObDereferenceObject(_FileObject);
    }
    else
    {
        //InterlockedIncrement(&_FileObject->DeviceObject->ReferenceCount);
        //InterlockedIncrement(&pFile->DeviceObject->ReferenceCount);
        //if (_FileObject->Vpb) InterlockedIncrement(&_FileObject->Vpb->ReferenceCount);
        //if (pFile->Vpb) InterlockedIncrement((volatile LONG*)&pFile->Vpb->ReferenceCount);
        //*FileObject = _FileObject;
        *FileObject = pFile;
    }

Cleanup:

	if (pkEvent != NULL) ExFreePoolWithTag(pkEvent, 'kevn');
    if (pAccessState != NULL) ExFreePoolWithTag(pAccessState, 'aest');
    if (pAuxData != NULL) ExFreePoolWithTag(pAuxData, 'aaed');
    if (pSecurityContext != NULL) ExFreePoolWithTag(pSecurityContext, 'iosc');
    return ntStatus;
}

NTSTATUS
IrpCloseFile(
    IN PFILE_OBJECT  FileObject)
{
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK  IoStatusBlock;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT pBaseDeviceObject = FileObject->Vpb->DeviceObject;

    if (!FileObject || FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);
    Irp->UserEvent = pkEvent;
    Irp->UserIosb = &IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_CLEANUP;
    IrpSp->FileObject = FileObject;

    ntStatus = IoCallDriver(pBaseDeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, FALSE, NULL);

    ntStatus = IoStatusBlock.Status;
    if (!NT_SUCCESS(ntStatus))
    {
        IoFreeIrp(Irp);
        return ntStatus;
    }

    KeClearEvent(pkEvent);
    IoReuseIrp(Irp, STATUS_SUCCESS);

    Irp->UserEvent = pkEvent;
    Irp->UserIosb = &IoStatusBlock;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->AssociatedIrp.SystemBuffer = (PVOID)NULL;
    Irp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_CLOSE;
    IrpSp->FileObject = FileObject;

    if (FileObject->Vpb && !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN))
    {
        //InterlockedDecrement((volatile LONG*)&FileObject->Vpb->ReferenceCount);
        //FileObject->Flags |= FO_FILE_OPEN_CANCELLED;
    }

    ntStatus = IoCallDriver(pBaseDeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, FALSE, NULL);

    IoFreeIrp(Irp);

    ntStatus = IoStatusBlock.Status;
    return ntStatus;
}

NTSTATUS
IrpQueryDirectoryFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
    IN PUNICODE_STRING  FileName  OPTIONAL,
    IN BOOLEAN bRestartScan  // 改为布尔值：TRUE=从头扫，FALSE=继续扫
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIRP Irp = NULL;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp = NULL;

    // 严格入参校验
    if (!FileObject || !IoStatusBlock || !FileInformation || Length == 0) {
        return STATUS_INVALID_PARAMETER;
    }
    if (!FileObject->Vpb || !FileObject->Vpb->DeviceObject) {
        return STATUS_UNSUCCESSFUL;
    }

    // 分配IRP
    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (!Irp) {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    // 分配事件
    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    // 初始化事件和缓冲区
    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);
    RtlZeroMemory(FileInformation, Length);

    // 填充IRP
    Irp->UserEvent = pkEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    Irp->Flags = IRP_SYNCHRONOUS_API;

    // 填充IRP栈
    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    IrpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;
    IrpSp->FileObject = FileObject;
    IrpSp->Flags = bRestartScan ? SL_RESTART_SCAN : 0;
    IrpSp->Parameters.QueryDirectory.Length = Length;
    IrpSp->Parameters.QueryDirectory.FileName = FileName;
    IrpSp->Parameters.QueryDirectory.FileInformationClass = FileInformationClass;
    
    // 调用驱动并等待完成
    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    ntStatus = IoCallDriver(FileObject->Vpb->DeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) {
        KeWaitForSingleObject(pkEvent, Executive, KernelMode, FALSE, NULL);
    }

    // 更新最终状态
    ntStatus = IoStatusBlock->Status;

Cleanup:
    // 释放资源（避免泄漏）
    if (pkEvent) ExFreePoolWithTag(pkEvent, 'kevn');
    if (Irp && !NT_SUCCESS(ntStatus)) {  // 仅失败时释放IRP（成功则由驱动处理）
        IoFreeIrp(Irp);
    }
    return ntStatus;
}

NTSTATUS
IrpQueryInformationFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;

    if (!FileObject || FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);

    RtlZeroMemory(FileInformation, Length);

    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->UserEvent = pkEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    IrpSp->DeviceObject = FileObject->Vpb->DeviceObject;
    IrpSp->FileObject = FileObject;
    IrpSp->Parameters.QueryFile.Length = Length;
    IrpSp->Parameters.QueryFile.FileInformationClass = FileInformationClass;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    ntStatus = IoCallDriver(FileObject->Vpb->DeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, 0);

    return IoStatusBlock->Status;
}

NTSTATUS
IrpSetInformationFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  FileInformation,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
	IN ULONG  Length
)
{
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;
	PDEVICE_OBJECT pDeviceObject;

	if (!FileObject) return STATUS_INVALID_PARAMETER;

    // 获取与指定文件对象相关联的设备对象   
    pDeviceObject = IoGetRelatedDeviceObject(FileObject);

    Irp = IoAllocateIrp(pDeviceObject->StackSize, TRUE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);

    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->UserEvent = pkEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    IrpSp->DeviceObject = pDeviceObject;
    IrpSp->FileObject = FileObject;
    IrpSp->Parameters.SetFile.Length = Length;
    IrpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;
    IrpSp->Parameters.SetFile.FileObject = FileObject;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    IoCallDriver(pDeviceObject, Irp);
    KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, 0);
    return IoStatusBlock->Status;
}

NTSTATUS
MySetInformationFile(
    IN HANDLE  FileHandle,
    IN PVOID  FileInformation,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
    IN ULONG  Length
)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PFILE_OBJECT  pFileObject;
    IO_STATUS_BLOCK  IoStatusBlock;
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT pDeviceObject;

    // 获取文件对象   
    ntStatus = ObReferenceObjectByHandle(FileHandle, DELETE,
        *IoFileObjectType, KernelMode, &pFileObject, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ObReferenceObjectByHandle error!%X", ntStatus);
        return FALSE;
    }

    if (pFileObject == NULL) return STATUS_INVALID_PARAMETER;

    // 获取与指定文件对象相关联的设备对象   
    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

    Irp = IoAllocateIrp(pDeviceObject->StackSize, TRUE);
    if (Irp == NULL) {
        ObDereferenceObject(pFileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);

    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->UserEvent = pkEvent;
    Irp->UserIosb = &IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = pFileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    IrpSp->DeviceObject = pDeviceObject;
    IrpSp->FileObject = pFileObject;
    IrpSp->Parameters.SetFile.Length = Length;
    IrpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;
    IrpSp->Parameters.SetFile.FileObject = pFileObject;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    IoCallDriver(pDeviceObject, Irp);
    KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, 0);
    ObDereferenceObject(pFileObject);
    return IoStatusBlock.Status;
}

NTSTATUS
IrpReadFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;

    if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    if (ByteOffset == NULL)
    {
        if (!(FileObject->Flags & FO_SYNCHRONOUS_IO)) return STATUS_INVALID_PARAMETER;

        ByteOffset = &FileObject->CurrentByteOffset;
    }

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(Buffer, Length);

    if (FileObject->DeviceObject->Flags & DO_BUFFERED_IO)
    {
        Irp->AssociatedIrp.SystemBuffer = Buffer;
    }
    else if (FileObject->DeviceObject->Flags & DO_DIRECT_IO)
    {
        Irp->MdlAddress = IoAllocateMdl(Buffer, Length, 0, 0, 0);
        if (Irp->MdlAddress == NULL)
        {
            IoFreeIrp(Irp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        MmBuildMdlForNonPagedPool(Irp->MdlAddress);
    }
    else
    {
        Irp->UserBuffer = Buffer;
    }

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);

    Irp->UserEvent = pkEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Flags = IRP_READ_OPERATION;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_READ;
    IrpSp->MinorFunction = IRP_MN_NORMAL;
    IrpSp->DeviceObject = FileObject->Vpb->DeviceObject;
    IrpSp->FileObject = FileObject;
    IrpSp->Parameters.Read.Length = Length;
    IrpSp->Parameters.Read.ByteOffset = *ByteOffset;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);

    ntStatus = IoCallDriver(FileObject->Vpb->DeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, 0);

    return IoStatusBlock->Status;
}

NTSTATUS
IrpWriteFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    PKEVENT pkEvent = NULL;
    PIO_STACK_LOCATION IrpSp;

    if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    if (ByteOffset == NULL)
    {
        if (!(FileObject->Flags & FO_SYNCHRONOUS_IO)) return STATUS_INVALID_PARAMETER;

        ByteOffset = &FileObject->CurrentByteOffset;
    }

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    if (FileObject->DeviceObject->Flags & DO_BUFFERED_IO)
    {
        Irp->AssociatedIrp.SystemBuffer = Buffer;
    }
    else
    {
        Irp->MdlAddress = IoAllocateMdl(Buffer, Length, 0, 0, 0);
        if (Irp->MdlAddress == NULL)
        {
            IoFreeIrp(Irp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        MmBuildMdlForNonPagedPool(Irp->MdlAddress);
    }

    pkEvent = (KEVENT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KEVENT), 'kevn');
    if (!pkEvent) {
        DbgPrint("Failed to allocate memory for KEVENT\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(pkEvent, SynchronizationEvent, FALSE);

    Irp->UserEvent = pkEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Flags = IRP_WRITE_OPERATION;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_WRITE;
    IrpSp->MinorFunction = IRP_MN_NORMAL;
    IrpSp->DeviceObject = FileObject->Vpb->DeviceObject;
    IrpSp->FileObject = FileObject;
    IrpSp->Parameters.Write.Length = Length;
    IrpSp->Parameters.Write.ByteOffset = *ByteOffset;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);
    ntStatus = IoCallDriver(FileObject->Vpb->DeviceObject, Irp);

    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(pkEvent, Executive, KernelMode, TRUE, NULL);

    return IoStatusBlock->Status;
}

UNICODE_STRING RtlGetUnicodeString(LPWSTR wStr)
{
    UNICODE_STRING uStr;
    //计算字符串的长度（不包括结尾的 NULL）
    uStr.Length = (USHORT)(wcslen(wStr) * sizeof(WCHAR));
    //计算字符串的最大长度（包括结尾的 NULL）
    uStr.MaximumLength = (USHORT)(uStr.Length + sizeof(WCHAR));
    uStr.Buffer = wStr;
    return uStr;
}

NTSTATUS MyCreateFile(PUNICODE_STRING ustrFileName, PHANDLE pFileHandle)
{
    NTSTATUS            ntStatus;
    OBJECT_ATTRIBUTES   objectAttributes;
    IO_STATUS_BLOCK     ioStatus;

    // 确保IRQL在PASSIVE_LEVEL上   
	if (KeGetCurrentIrql() > PASSIVE_LEVEL) return STATUS_INVALID_LEVEL;

    //初始化对象属性   
    InitializeObjectAttributes(&objectAttributes, ustrFileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    // 打开文件   
    ntStatus = IoCreateFile(pFileHandle, FILE_READ_ATTRIBUTES, &objectAttributes, &ioStatus,
        0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE, FILE_OPEN, 0, NULL, 0, CreateFileTypeNone, NULL, IO_NO_PARAMETER_CHECKING);
    if (!NT_SUCCESS(ntStatus)) {
        DbgPrint("IoCreateFile error:%X", ntStatus);
		return ntStatus;
    }
	// 返回成功状态
	return STATUS_SUCCESS;
}

NTSTATUS MyDeleteFile(HANDLE FileHandle)
{
    NTSTATUS          ntStatus = STATUS_SUCCESS;
    PFILE_OBJECT      fileObject;
    PDEVICE_OBJECT    DeviceObject;
    PIRP              Irp;
    KEVENT            SycEvent;
    FILE_DISPOSITION_INFORMATION    FileInformation;
    IO_STATUS_BLOCK                 ioStatus;
    PIO_STACK_LOCATION              irpSp;
    PSECTION_OBJECT_POINTERS        pSectionObjectPointer;

    // 获取文件对象   
    ntStatus = ObReferenceObjectByHandle(FileHandle, DELETE,
        *IoFileObjectType, KernelMode, (PVOID*)&fileObject, NULL);
    if (!NT_SUCCESS(ntStatus))
    {
        DbgPrint("ObReferenceObjectByHandle error!%X", ntStatus);
        return FALSE;
    }

    // 获取与指定文件对象相关联的设备对象   
    DeviceObject = IoGetRelatedDeviceObject(fileObject);

    // 创建IRP   
    Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (Irp == NULL)
    {
        ObDereferenceObject(fileObject);
        DbgPrint("FD_DeleteFile IoAllocateIrp error");
        return FALSE;
    }

    // 初始化同步事件对象   
    KeInitializeEvent(&SycEvent, SynchronizationEvent, FALSE);

    FileInformation.DeleteFile = TRUE;

    // 初始化IRP   
    Irp->AssociatedIrp.SystemBuffer = &FileInformation;
    Irp->UserEvent = &SycEvent;
    Irp->UserIosb = &ioStatus;
    Irp->Tail.Overlay.OriginalFileObject = fileObject;
    Irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
    Irp->RequestorMode = KernelMode;

    // 设置IRP堆栈   
    irpSp = IoGetNextIrpStackLocation(Irp);
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->DeviceObject = DeviceObject;
    irpSp->FileObject = fileObject;
    irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
    irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
    irpSp->Parameters.SetFile.FileObject = fileObject;

    // 设置完成例程   
    IoSetCompletionRoutine(Irp, IoCompletionRoutine, NULL, TRUE, TRUE, TRUE);

    // 如果没有这3行，就无法删除正在运行的文件   
    pSectionObjectPointer = fileObject->SectionObjectPointer;
    pSectionObjectPointer->ImageSectionObject = NULL;
    pSectionObjectPointer->DataSectionObject = NULL;
    
    /*fileObject->SharedRead = TRUE;
    fileObject->SharedWrite = TRUE;
    fileObject->SharedDelete = TRUE;
    fileObject->ReadAccess = 0;
    fileObject->WriteAccess = 0;
    fileObject->DeleteAccess = 0;
    fileObject->DeletePending = FALSE;*/

    // 派发IRP   
    IoCallDriver(DeviceObject, Irp);
    
    // 等待IRP完成   
    KeWaitForSingleObject(&SycEvent, Executive, KernelMode, TRUE, NULL);

    // 递减引用计数   
    ObDereferenceObject(fileObject);

	return ioStatus.Status;
}

// 强制删除文件
NTSTATUS ForceDeleteFile(UNICODE_STRING ustrFileName)
{
    NTSTATUS status = STATUS_SUCCESS;
    FILE_BASIC_INFORMATION fileBaseInfo = { 0 };
	HANDLE hFile = NULL;
    // 发送IRP打开文件
    status = MyCreateFile(&ustrFileName, &hFile);//, GENERIC_READ | GENERIC_WRITE, ,
        //&iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        //FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    DbgPrint("MyCreateFile Error[0x%X]\n", status);
    if (!NT_SUCCESS(status)) goto Cleanup;

    // 发送IRP设置文件属性, 去掉只读属性, 修改为 FILE_ATTRIBUTE_NORMAL
    RtlZeroMemory(&fileBaseInfo, sizeof(fileBaseInfo));
    fileBaseInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
    status = MySetInformationFile(hFile, &fileBaseInfo, FileBasicInformation, sizeof(fileBaseInfo));
    DbgPrint("IrpSetInformationFile[SetInformation] Error[0x%X]\n", status);
    if (!NT_SUCCESS(status)) goto Cleanup;

    // 发送IRP设置文件属性, 设置删除文件操作
    status = MyDeleteFile(hFile);
    DbgPrint("MyDeleteFile Error[0x%X]\n", status);

Cleanup:
	ZwClose(hFile);
    return status;
}

VOID RtlGetEmptyUnicodeString(_Out_ PUNICODE_STRING str, _In_ USHORT length) {
    str->Length = length * sizeof(WCHAR);
    str->MaximumLength = (length + 1) * sizeof(WCHAR);
    str->Buffer = (PWSTR)ExAllocatePool2(POOL_FLAG_NON_PAGED, str->MaximumLength, 'MyTg');
    if (str->Buffer != NULL) {
        RtlZeroMemory(str->Buffer, str->Length);
    }
}

NTSTATUS GetNtfsFsdCleanup(PVOID* pNtfsFsdCleanup) {
    UNICODE_STRING driverPath;
    PDRIVER_OBJECT ntfsDriverObject = NULL;
    NTSTATUS status;

    // 初始化NTFS驱动对象的路径（\\FileSystem\\Ntfs）
    RtlInitUnicodeString(&driverPath, L"\\FileSystem\\Ntfs");

    // 获取驱动对象指针
    status = ObReferenceObjectByName(&driverPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
		(PVOID*)&ntfsDriverObject);

    if (!NT_SUCCESS(status) || !ntfsDriverObject) return status;

    // 从MajorFunction数组获取IRP_MJ_CLEANUP对应的派遣函数
    *pNtfsFsdCleanup = (PVOID)ntfsDriverObject->MajorFunction[IRP_MJ_CLEANUP];

    // 关闭句柄（若不需要保留）
	ObDereferenceObject(ntfsDriverObject);
    return STATUS_SUCCESS;
}

PVOID GetNtfsDecrementCleanupCounts() {
    PVOID NtfsFsdCleanupAddr = NULL;
    NTSTATUS status = GetNtfsFsdCleanup(&NtfsFsdCleanupAddr);
    if (!NT_SUCCESS(status)) {
        DbgPrint("GetNtfsFsdCleanup failed.status = %X", status);
        return NULL;
    }

    /*
    fffff805`54766bc8 7607            jbe     Ntfs!NtfsFsdCleanup+0x1f1 (fffff805`54766bd1)  Branch

    Ntfs!NtfsFsdCleanup+0x1ea:
    fffff805`54766bca e8e1010000      call    Ntfs!NtfsCommonCleanup (fffff805`54766db0)
    */
	PVOID NtfsCommonCleanupAddr = NULL;
	UCHAR pattern1[] = { 0x76, 0x07, 0xE8 };
	PVOID result = SearchSpecialCode((PUCHAR)NtfsFsdCleanupAddr, 0x300, pattern1, sizeof(pattern1));
    if (result == NULL) {
        DbgPrint("Get NtfsCommonCleanupAddr failed.");
        return NULL;
	}
	DbgPrint("result = 0x%p", result);
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	NtfsCommonCleanupAddr = (PVOID)((PUCHAR)result + 7 + offset);
	DbgPrint("NtfsCommonCleanupAddr = 0x%p", NtfsCommonCleanupAddr);

    /*
    fffff805`5476c107 488bcf          mov     rcx,rdi
    fffff805`5476c10a e8218ce6ff      call    Ntfs!NtfsDecrementCleanupCounts (fffff805`545d4d30)
    */
	UCHAR pattern2[] = {0x48, 0x8B, 0xCF, 0xE8};
    result = SearchSpecialCode((PUCHAR)NtfsCommonCleanupAddr + 0x5300, 0x100, pattern2, sizeof(pattern2));
    if (result == NULL) {
        DbgPrint("Get NtfsDecrementCleanupCountsAddr failed.");
        return NULL;
    }
	DbgPrint("result = 0x%p", result);
    // 计算实际函数地址
	LONG offset1 = *(PLONG)((PUCHAR)result + 4);
	DbgPrint("offset = 0x%X", offset);
    PVOID NtfsDecrementCleanupCountsAddr = NULL;
    NtfsDecrementCleanupCountsAddr = (PVOID)((PUCHAR)result + 8 + offset1);
	return NtfsDecrementCleanupCountsAddr;
}

NTSTATUS DeleteFileByXCBFunction(PUNICODE_STRING ustrFileName)
{
    typedef VOID(__fastcall *pNtfsDecrementCleanupCounts)(__int64 a1, __int64* a2, __int64* a3, int a4, int a5);
    pNtfsDecrementCleanupCounts NtfsDecrementCleanupCounts = (pNtfsDecrementCleanupCounts)GetNtfsDecrementCleanupCounts();

    if (NtfsDecrementCleanupCounts == NULL) {
        DbgPrint("Get NtfsDecrementCleanupCounts failed.");
        return STATUS_UNSUCCESSFUL;
	}
	DbgPrint("NtfsDecrementCleanupCounts = 0x%p", NtfsDecrementCleanupCounts);

    IO_STATUS_BLOCK ioStatus;
    HANDLE handle;

    OBJECT_ATTRIBUTES objAttributes = { 0 };
    InitializeObjectAttributes(&objAttributes, ustrFileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);


    NTSTATUS ntStatus = ZwCreateFile(
        &handle,
        SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
        &objAttributes,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);
    if (NT_SUCCESS(ntStatus))
    {
        PFILE_OBJECT FileObject;

        ntStatus = ObReferenceObjectByHandle(handle,
            DELETE,
            *IoFileObjectType,
            KernelMode,
            &FileObject,
            NULL);

        if (!NT_SUCCESS(ntStatus))
        {
			DbgPrint("[DeleteFileByXCBFunction]ObReferenceObjectByHandle failed. ntStatus = %X", ntStatus);
            ZwClose(handle);
            return ntStatus;
        }

        if (MmIsAddressValid((PVOID)NtfsDecrementCleanupCounts))
        {

            PVOID pSCB = FileObject->FsContext;
            //PVOID pCCB = FileObject->FsContext2;

            NtfsDecrementCleanupCounts(0, pSCB, 0, FileObject->Flags & 8, 0);

        }
        else
        {
            ntStatus = STATUS_INVALID_PARAMETER;
        }

        ObDereferenceObject(FileObject);
        ZwClose(handle);
    }
    else {
        DbgPrint("[DeleteFileByXCBFunction]ZwCreateFile failed. ntStatus = %X", ntStatus);
		return ntStatus;
    }

    DbgPrint("[DeleteFileByXCBFunction]ntStatus %x\n", ntStatus);
    return ntStatus;
}

// 检查文件是否真正被删除
BOOLEAN IsFileReallyDeleted(PUNICODE_STRING FileName) {
    NTSTATUS status;
    HANDLE hFile;
    IO_STATUS_BLOCK iosb;
    FILE_STANDARD_INFORMATION fsInfo = { 0 };
    OBJECT_ATTRIBUTES oa;

    // 初始化对象属性（以查询为目的打开文件）
    InitializeObjectAttributes(&oa, FileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    // 尝试以只读、共享所有权限打开文件（不实际打开数据，仅查询元数据）
    status = ZwOpenFile(&hFile,
        GENERIC_READ,
        &oa,
        &iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_FOR_BACKUP_INTENT | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status)) {
        DbgPrint("[IsFileReallyDeleted]ZwOpenFile:status = %X", status);
        // 状态码为STATUS_OBJECT_NAME_NOT_FOUND：文件已真正删除（元数据不存在）
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) return TRUE;
        // 其他错误（如权限不足）：无法判断，返回FALSE
        return FALSE;
    }

    // 查询文件标准信息
    status = ZwQueryInformationFile(hFile,
        &iosb,
        &fsInfo,
        sizeof(fsInfo),
        FileStandardInformation);
    ZwClose(hFile);

    DbgPrint("[IsFileReallyDeleted]ZwQueryInformationFile:status = %X", status);
    if (!NT_SUCCESS(status)) return FALSE;

    // 若DeletePending为TRUE：文件处于延迟删除状态（未真正删除）
    // 若NumberOfLinks为0且DeletePending为FALSE：理论上不可能，除非删除逻辑异常
    return (fsInfo.DeletePending == FALSE && fsInfo.NumberOfLinks == 0) ? TRUE : FALSE;
}

// ============================================================================
// 函数：ConcatPath - 安全拼接路径
// ============================================================================
NTSTATUS ConcatPath(
    IN PUNICODE_STRING BasePath,
    IN PCWSTR SubPath,
    IN ULONG SubPathLen,  // 子路径长度（字节，无NULL终止符）
    OUT PUNICODE_STRING ResultPath
)
{
    USHORT totalLen = 0;
    BOOLEAN needsSlash = FALSE;
    PWCHAR pDest = NULL;

    // 强化入参校验：过滤空路径/空文件名
    if (!BasePath || !SubPath || !ResultPath) {
        DbgPrint("[ConcatPath] Null parameter\n");
        return STATUS_INVALID_PARAMETER;
    }
    if (SubPathLen == 0 || (USHORT)SubPathLen > MAXUSHORT - BasePath->Length) {
        DbgPrint("[ConcatPath] Invalid SubPathLen: %lu\n", SubPathLen);
        return STATUS_INVALID_PARAMETER;
    }
    if (BasePath->Length > MAXUSHORT - SubPathLen - sizeof(WCHAR)) {
        DbgPrint("[ConcatPath] BasePath too long: %hu\n", BasePath->Length);
        return STATUS_INVALID_PARAMETER;
    }

    // 初始化输出
    RtlZeroMemory(ResultPath, sizeof(UNICODE_STRING));

    // 判断是否需要添加反斜杠
    if (BasePath->Length > 0) {
        ULONG baseLastIdx = (BasePath->Length / sizeof(WCHAR)) - 1;
        // 处理路径末尾的空格/非法字符
        if (BasePath->Buffer[baseLastIdx] != L'\\' && BasePath->Buffer[baseLastIdx] != L' ') {
            needsSlash = TRUE;
        }
    }

    // 计算总长度（源路径 + 反斜杠 + 子路径）
    totalLen = BasePath->Length + (needsSlash ? sizeof(WCHAR) : 0) + (USHORT)SubPathLen;

    // 分配缓冲区（包含NULL终止符）
    ResultPath->Buffer = (PWCH)ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        totalLen + sizeof(WCHAR),  // +2字节用于NULL终止符
        POOL_TAG_PATH
    );
    if (!ResultPath->Buffer) {
        DbgPrint("[ConcatPath] Allocate buffer failed\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 初始化缓冲区
    ResultPath->Length = 0;
    ResultPath->MaximumLength = totalLen + sizeof(WCHAR);
    RtlZeroMemory(ResultPath->Buffer, ResultPath->MaximumLength);
    pDest = ResultPath->Buffer;

    // 复制基础路径（过滤末尾空格）
    if (BasePath->Length > 0) {
        USHORT validBaseLen = BasePath->Length;
        // 移除基础路径末尾的空格
        while (validBaseLen > 0 && BasePath->Buffer[(validBaseLen / sizeof(WCHAR)) - 1] == L' ') {
            validBaseLen -= sizeof(WCHAR);
        }
        if (validBaseLen > 0) {
            RtlCopyMemory(pDest, BasePath->Buffer, validBaseLen);
            pDest += validBaseLen / sizeof(WCHAR);
            ResultPath->Length += validBaseLen;
        }
    }

    // 添加反斜杠
    if (needsSlash) {
        *pDest++ = L'\\';
        ResultPath->Length += sizeof(WCHAR);
    }

    // 复制子路径（过滤空/非法字符）
    if (SubPathLen > 0) {
        RtlCopyMemory(pDest, SubPath, SubPathLen);
        pDest += SubPathLen / sizeof(WCHAR);
        ResultPath->Length += (USHORT)SubPathLen;
    }

    // 添加NULL终止符
    *pDest = L'\0';
    
    DbgPrint("[ConcatPath] Success: %wZ\n", ResultPath);
    return STATUS_SUCCESS;
}

// ============================================================================
// 函数：CreateDirectory - 安全创建目录
// ============================================================================
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS CreateDirectory(IN PUNICODE_STRING DirPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT dirObj = NULL;
    IO_STATUS_BLOCK iosb = { 0 };

    status = CheckIrqlPassiveLevel();
    if (!NT_SUCCESS(status)) return status;

    // 尝试创建
    status = IrpCreateFile(&dirObj, GENERIC_READ | GENERIC_WRITE, DirPath, &iosb, NULL,
        FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_CREATE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    // 如果已存在，尝试打开
    if (status == STATUS_OBJECT_NAME_COLLISION) {
        status = IrpCreateFile(&dirObj, GENERIC_READ, DirPath, &iosb, NULL,
            FILE_ATTRIBUTE_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN, FILE_DIRECTORY_FILE, NULL, 0);

        // ✅ 必须关闭！
        if (NT_SUCCESS(status) && dirObj) {
            IrpCloseFile(dirObj);
            return STATUS_SUCCESS; // 目录已存在，视为成功
        }
    }

    if (NT_SUCCESS(status) && dirObj) {
        IrpCloseFile(dirObj);
    }

    return status;
}

// ============================================================================
// 函数：ForceCopyFile - 安全复制文件（池分配 + IRQL检查）
// ============================================================================
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS ForceCopyFile(IN PUNICODE_STRING SrcPath, IN PUNICODE_STRING DstPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT srcFile = NULL, dstFile = NULL;
    IO_STATUS_BLOCK srcIosb = { 0 }, dstIosb = { 0 };
    PUCHAR buffer = NULL;
    LARGE_INTEGER offset = { 0 };
    ULONG bytesRead = 0;
    ULONG bytesWritten = 0;
    FILE_BASIC_INFORMATION basicInfo = { 0 };

    // IRQL检查
    status = CheckIrqlPassiveLevel();
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // 分配复制缓冲区（非分页池，避免栈溢出）
    const ULONG BUFFER_SIZE = 64 * 1024;  // 64KB块大小
    buffer = (PUCHAR)ExAllocatePool2(POOL_FLAG_NON_PAGED, BUFFER_SIZE, POOL_TAG_BUFFER);
    if (!buffer) {
        DbgPrint("[ForceCopyFile] Failed to allocate buffer\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __try {
        // 打开源文件
        status = IrpCreateFile(
            &srcFile,
            GENERIC_READ,
            SrcPath,
            &srcIosb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
        );
        if (!NT_SUCCESS(status)) {
            DbgPrint("[ForceCopyFile] Failed to open source: 0x%X, Path: %wZ\n", status, SrcPath);
            __leave;
        }

        // 创建目标文件
        status = IrpCreateFile(
            &dstFile,
            GENERIC_WRITE,
            DstPath,
            &dstIosb,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            0,
            FILE_CREATE,
            FILE_SYNCHRONOUS_IO_NONALERT | FILE_OVERWRITE_IF,
            NULL,
            0
        );
        if (!NT_SUCCESS(status)) {
            DbgPrint("[ForceCopyFile] Failed to create dest: 0x%X, Path: %wZ\n", status, DstPath);
            __leave;
        }

        // 流式复制
        do {
            RtlZeroMemory(buffer, BUFFER_SIZE);

            // 读取
            status = IrpReadFile(srcFile, &srcIosb, buffer, BUFFER_SIZE, &offset);
            if (!NT_SUCCESS(status) && status != STATUS_END_OF_FILE) {
                DbgPrint("[ForceCopyFile] Read failed: 0x%X\n", status);
                __leave;
            }

            bytesRead = (ULONG)srcIosb.Information;
            if (bytesRead == 0) break;  // EOF

            // 写入
            status = IrpWriteFile(dstFile, &dstIosb, buffer, bytesRead, &offset);
            if (!NT_SUCCESS(status)) {
                DbgPrint("[ForceCopyFile] Write failed: 0x%X\n", status);
                __leave;
            }

            bytesWritten = (ULONG)dstIosb.Information;
            if (bytesRead != bytesWritten) {
                DbgPrint("[ForceCopyFile] Byte mismatch: read=%d, write=%d\n", bytesRead, bytesWritten);
                status = STATUS_IO_DEVICE_ERROR;
                __leave;
            }

            offset.QuadPart += bytesRead;
        } while (NT_SUCCESS(status));

        // 复制文件属性
        status = IrpQueryInformationFile(srcFile, &srcIosb, &basicInfo, sizeof(basicInfo), FileBasicInformation);
        if (NT_SUCCESS(status)) {
            IrpSetInformationFile(dstFile, &dstIosb, &basicInfo, FileBasicInformation, sizeof(basicInfo));
        }
        DbgPrint("[ForceCopyFile] Create file: %wZ", DstPath);
        status = STATUS_SUCCESS;  // 忽略EOF错误

    }
    __finally {
        // 清理资源（无论成功或失败）
        if (srcFile) {
            IrpCloseFile(srcFile);
        }
        if (dstFile) {
            IrpCloseFile(dstFile);
        }
        if (buffer) {
            ExFreePoolWithTag(buffer, POOL_TAG_BUFFER);
        }
    }

    return status;
}

// ============================================================================
// 函数：ProcessDirectory - 处理单个目录并填充工作队列（无递归）
// ============================================================================
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS ProcessDirectory(
    IN PUNICODE_STRING SrcPath,
    IN PUNICODE_STRING DstPath,
    IN OUT PLIST_ENTRY WorkQueue
) {
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_OBJECT srcDir = NULL;
    IO_STATUS_BLOCK iosb = { 0 };
    PUCHAR dirBuffer = NULL;
    ULONG dirBufferSize = 4096;  // 增大缓冲区避免溢出
    BOOLEAN bRestartScan = TRUE; // 首次扫描：从头开始

    // IRQL检查
    status = CheckIrqlPassiveLevel();
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // 过滤源路径末尾空格（关键：修复日志中源路径的空格问题）
    UNICODE_STRING cleanSrcPath = { 0 };
    status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE, SrcPath, &cleanSrcPath);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[ProcessDirectory] Duplicate src path failed: 0x%X\n", status);
        return status;
    }
    // 移除末尾空格
    while (cleanSrcPath.Length > 0 && cleanSrcPath.Buffer[(cleanSrcPath.Length / sizeof(WCHAR)) - 1] == L' ') {
        cleanSrcPath.Length -= sizeof(WCHAR);
    }
    cleanSrcPath.Buffer[cleanSrcPath.Length / sizeof(WCHAR)] = L'\0';

    // 分配目录查询缓冲区
    dirBuffer = (PUCHAR)ExAllocatePool2(POOL_FLAG_NON_PAGED, dirBufferSize, POOL_TAG_BUFFER);
    if (!dirBuffer) {
        DbgPrint("[ProcessDirectory] Failed to allocate dirBuffer\n");
        RtlFreeUnicodeString(&cleanSrcPath);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __try {
        // 打开源目录（使用清理后的路径）
        status = IrpCreateFile(
            &srcDir,
            GENERIC_READ,
            &cleanSrcPath,  // 修复：使用无末尾空格的路径
            &iosb,
            NULL,
            FILE_ATTRIBUTE_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0
        );
        if (!NT_SUCCESS(status)) {
            DbgPrint("[ProcessDirectory] Failed to open dir: 0x%X, Path: %wZ\n", status, &cleanSrcPath);
            __leave;
        }

        // 枚举目录项：直到无更多文件
        while (TRUE) {
            RtlZeroMemory(dirBuffer, dirBufferSize);

            // 查询目录（核心：首次bRestartScan=TRUE，后续=FALSE）
            status = IrpQueryDirectoryFile(
                srcDir,
                &iosb,
                dirBuffer,
                dirBufferSize,
                FileDirectoryInformation,
                NULL,
                bRestartScan
            );

            // 无更多文件，退出循环
            if (status == STATUS_NO_MORE_FILES) {
                status = STATUS_SUCCESS;
                break;
            }
            // 其他错误，退出
            if (!NT_SUCCESS(status)) {
                DbgPrint("[ProcessDirectory] QueryDirectory failed: 0x%X\n", status);
                __leave;
            }

            // 首次扫描后，后续不再重启
            bRestartScan = FALSE;

            // 遍历当前批次的目录项（防越界）
            PFILE_DIRECTORY_INFORMATION dirInfo = (PFILE_DIRECTORY_INFORMATION)dirBuffer;
            PUCHAR bufferEnd = dirBuffer + dirBufferSize; // 缓冲区边界
            while (TRUE) {
                // 防越界：指针超出缓冲区则退出
                if ((PUCHAR)dirInfo >= bufferEnd || (PUCHAR)dirInfo + sizeof(FILE_DIRECTORY_INFORMATION) > bufferEnd) {
                    DbgPrint("[ProcessDirectory] DirInfo out of buffer\n");
                    break;
                }

                // 过滤空文件名（核心修复）
                if (dirInfo->FileNameLength == 0 || dirInfo->FileName == NULL) {
                    DbgPrint("[ProcessDirectory] Skip empty filename\n");
                    if (dirInfo->NextEntryOffset == 0) break;
                    dirInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)dirInfo + dirInfo->NextEntryOffset);
                    continue;
                }

                // 跳过 . 和 ..
                if ((dirInfo->FileNameLength == sizeof(WCHAR) && dirInfo->FileName[0] == L'.') ||
                    (dirInfo->FileNameLength == 2 * sizeof(WCHAR) && dirInfo->FileName[0] == L'.' && dirInfo->FileName[1] == L'.')) {
                    if (dirInfo->NextEntryOffset == 0) break;
                    dirInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)dirInfo + dirInfo->NextEntryOffset);
                    continue;
                }

                // 打印调试信息（定位问题）
                DbgPrint("[ProcessDirectory] FileName: %.*ws, Length: %hu\n",
                    dirInfo->FileNameLength / sizeof(WCHAR), dirInfo->FileName, dirInfo->FileNameLength);

                // 拼接路径（使用清理后的源路径）
                UNICODE_STRING currentSrcPath = { 0 };
                UNICODE_STRING currentDstPath = { 0 };

                status = ConcatPath(&cleanSrcPath, dirInfo->FileName, dirInfo->FileNameLength, &currentSrcPath);
                if (!NT_SUCCESS(status)) {
                    DbgPrint("[ProcessDirectory] Concat source path failed: 0x%X, FileName: %.*ws\n",
                        status, dirInfo->FileNameLength / sizeof(WCHAR), dirInfo->FileName);
                    __leave;
                }

                status = ConcatPath(DstPath, dirInfo->FileName, dirInfo->FileNameLength, &currentDstPath);
                if (!NT_SUCCESS(status)) {
                    ExFreePoolWithTag(currentSrcPath.Buffer, POOL_TAG_PATH);
                    DbgPrint("[ProcessDirectory] Concat dest path failed: 0x%X, FileName: %.*ws\n",
                        status, dirInfo->FileNameLength / sizeof(WCHAR), dirInfo->FileName);
                    __leave;
                }

                // 创建工作项
                PCOPY_WORK_ITEM workItem = (PCOPY_WORK_ITEM)ExAllocatePool2(
                    POOL_FLAG_NON_PAGED,
                    sizeof(COPY_WORK_ITEM),
                    POOL_TAG_WORKITEM
                );
                if (!workItem) {
                    ExFreePoolWithTag(currentSrcPath.Buffer, POOL_TAG_PATH);
                    ExFreePoolWithTag(currentDstPath.Buffer, POOL_TAG_PATH);
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }

                // 初始化工作项
                InitializeListHead(&workItem->ListEntry);
                workItem->SourcePath = currentSrcPath;
                workItem->DestPath = currentDstPath;
                workItem->IsDirectory = (dirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;

                InsertTailList(WorkQueue, &workItem->ListEntry);

                // 下一个目录项（防越界）
                if (dirInfo->NextEntryOffset == 0) break;
                PUCHAR nextDirInfo = (PUCHAR)dirInfo + dirInfo->NextEntryOffset;
                if (nextDirInfo >= bufferEnd) {
                    DbgPrint("[ProcessDirectory] Next entry out of buffer\n");
                    break;
                }
                dirInfo = (PFILE_DIRECTORY_INFORMATION)nextDirInfo;
            }
        }

    }
    __finally {
        // 清理资源
        if (srcDir) IrpCloseFile(srcDir);
        if (dirBuffer) ExFreePoolWithTag(dirBuffer, POOL_TAG_BUFFER);
        RtlFreeUnicodeString(&cleanSrcPath); // 释放清理后的路径
    }

    return status;
}

// ============================================================================
// 函数：ForceCopyFolder - 迭代实现（无递归）
// ============================================================================
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS ForceCopyFolder(IN PUNICODE_STRING SrcFolder, IN PUNICODE_STRING DstFolder)
{
    NTSTATUS status = STATUS_SUCCESS;
    LIST_ENTRY workQueue;
    PCOPY_WORK_ITEM initialItem = NULL;

    // IRQL检查
    status = CheckIrqlPassiveLevel();
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // 初始化队列
    InitializeListHead(&workQueue);

    __try {
        // 创建初始工作项（源目录）
        initialItem = (PCOPY_WORK_ITEM)ExAllocatePool2(
            POOL_FLAG_NON_PAGED,
            sizeof(COPY_WORK_ITEM),
            POOL_TAG_WORKITEM
        );
        if (!initialItem) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        // 深拷贝路径（必须独立分配）
        status = RtlDuplicateUnicodeString(
            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
            SrcFolder,
            &initialItem->SourcePath
        );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        status = RtlDuplicateUnicodeString(
            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
            DstFolder,
            &initialItem->DestPath
        );
        if (!NT_SUCCESS(status)) {
            __leave;
        }

        initialItem->IsDirectory = TRUE;
        InsertTailList(&workQueue, &initialItem->ListEntry);
        initialItem = NULL;  // 所有权转移给队列

        // 处理队列（迭代而非递归）
        while (!IsListEmpty(&workQueue)) {
            PLIST_ENTRY listEntry = RemoveHeadList(&workQueue);
            PCOPY_WORK_ITEM workItem = CONTAINING_RECORD(listEntry, COPY_WORK_ITEM, ListEntry);

            __try {
                if (workItem->IsDirectory) {
                    // 创建目标目录
                    status = CreateDirectory(&workItem->DestPath);
                    if (!NT_SUCCESS(status)) {
                        DbgPrint("[ForceCopyFolder] CreateDirectory failed: 0x%X, Path: %wZ\n",
                            status, &workItem->DestPath);
                        __leave;
                    }
					DbgPrint("[ForceCopyFolder] Created directory: %wZ\n", &workItem->DestPath);

                    // 枚举并添加子项到队列
                    status = ProcessDirectory(
                        &workItem->SourcePath,
                        &workItem->DestPath,
                        &workQueue
                    );
                }
                else {
                    // 复制文件
                    status = ForceCopyFile(&workItem->SourcePath, &workItem->DestPath);
                }
            }
            __finally {
                // 释放当前工作项（无论成功失败）
                RtlFreeUnicodeString(&workItem->SourcePath);
                RtlFreeUnicodeString(&workItem->DestPath);
                ExFreePoolWithTag(workItem, POOL_TAG_WORKITEM);
            }

            if (!NT_SUCCESS(status)) {
                break;  // 退出主循环
            }
        }

    }
    __finally {
        // 清理残留队列（出错时）
        while (!IsListEmpty(&workQueue)) {
            PLIST_ENTRY listEntry = RemoveHeadList(&workQueue);
            PCOPY_WORK_ITEM workItem = CONTAINING_RECORD(listEntry, COPY_WORK_ITEM, ListEntry);

            RtlFreeUnicodeString(&workItem->SourcePath);
            RtlFreeUnicodeString(&workItem->DestPath);
            ExFreePoolWithTag(workItem, POOL_TAG_WORKITEM);
        }

        // 清理初始项（如果未加入队列）
        if (initialItem) {
            if (initialItem->SourcePath.Buffer) {
                RtlFreeUnicodeString(&initialItem->SourcePath);
            }
            ExFreePoolWithTag(initialItem, POOL_TAG_WORKITEM);
        }
    }

    return status;
}