#include "File.h"

PDRIVER_DISPATCH g_OriginalDiskWriteDispatch = NULL;
PDRIVER_DISPATCH g_OriginalDiskReadDispatch = NULL;
PDRIVER_DISPATCH g_OriginalDiskDeviceControlDispatch = NULL;
PDRIVER_DISPATCH g_OriginalStorPortScsiDispatch = NULL;
PDRIVER_DISPATCH g_OriginalNtfsDispatch = NULL;

NTSTATUS
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)

{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);
    *Irp->UserIosb = Irp->IoStatus;
    if (Irp->UserEvent)
        KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, 0);
    if (Irp->MdlAddress)
    {
        IoFreeMdl(Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }
    IoFreeIrp(Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
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
    //KEVENT kEvent;
    PIO_STACK_LOCATION IrpSp;
    //ACCESS_STATE AccessState;
    //AUX_ACCESS_DATA AuxData;
    //IO_SECURITY_CONTEXT SecurityContext;

    //初始化结构体
    KEVENT* pkEvent = NULL;
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
    pAuxData = (AUX_ACCESS_DATA*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(AUX_ACCESS_DATA), 'aaed');
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

    if (FilePath->Length < 6)
    {
        DbgPrint("FilePath->Length:%d", FilePath->Length);
        ntStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    //RtlInitUnicodeString(&UniDeviceNameString, L"//DosDevices//*://");
    //UniDeviceNameString.Buffer[12] = FilePath->Buffer[0];
    InitializeObjectAttributes(&ObjectAttributes, FilePath, OBJ_KERNEL_HANDLE, NULL, NULL);
    ntStatus = IoCreateFile(&hFile,
        GENERIC_READ | SYNCHRONIZE,
        &ObjectAttributes,
        IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        CreateFileTypeNone,
        NULL,
        IO_NO_PARAMETER_CHECKING);
    DbgPrint("IoCreateFile status:%d", ntStatus);
    if (!NT_SUCCESS(ntStatus)) goto Cleanup;

    ntStatus = ObReferenceObjectByHandle(hFile,
        FILE_READ_ACCESS, // ACCESS_MASK
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
    ObDereferenceObject(pFile);
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
        InterlockedIncrement(&pFile->DeviceObject->ReferenceCount);
        //if (_FileObject->Vpb) InterlockedIncrement(&_FileObject->Vpb->ReferenceCount);
        if (pFile->Vpb) InterlockedIncrement((volatile LONG*)&pFile->Vpb->ReferenceCount);
        //*FileObject = _FileObject;
        *FileObject = pFile;
    }

Cleanup:

    if (pkEvent != NULL) ExFreePoolWithTag(pkEvent, 'keve');
    if (pAccessState != NULL) ExFreePoolWithTag(pAccessState, 'aest');
    if (pAuxData != NULL) ExFreePoolWithTag(pAuxData, 'aaed');
    if (pSecurityContext != NULL) ExFreePoolWithTag(pSecurityContext, 'iosc');
    return ntStatus;
}

NTSTATUS
IrpClose(
    IN PFILE_OBJECT  FileObject)
{
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK  IoStatusBlock;
    PIRP Irp;
    KEVENT kEvent;
    PIO_STACK_LOCATION IrpSp;
    PDEVICE_OBJECT pBaseDeviceObject = FileObject->Vpb->DeviceObject;

    if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);
    Irp->UserEvent = &kEvent;
    Irp->UserIosb = &IoStatusBlock;
    Irp->RequestorMode = KernelMode;
    Irp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_CLEANUP;
    IrpSp->FileObject = FileObject;

    ntStatus = IoCallDriver(pBaseDeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);

    ntStatus = IoStatusBlock.Status;
    if (!NT_SUCCESS(ntStatus))
    {
        IoFreeIrp(Irp);
        return ntStatus;
    }

    KeClearEvent(&kEvent);
    IoReuseIrp(Irp, STATUS_SUCCESS);

    Irp->UserEvent = &kEvent;
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
        InterlockedDecrement((volatile LONG*)&FileObject->Vpb->ReferenceCount);
        FileObject->Flags |= FO_FILE_OPEN_CANCELLED;
    }

    ntStatus = IoCallDriver(pBaseDeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);

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
    IN PUNICODE_STRING  FileName  OPTIONAL)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    KEVENT kEvent;
    PIO_STACK_LOCATION IrpSp;

    if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);
    RtlZeroMemory(FileInformation, Length);

    Irp->UserEvent = &kEvent;
    Irp->UserIosb = IoStatusBlock;
    Irp->UserBuffer = FileInformation;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE)NULL;

    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    IrpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;
    IrpSp->FileObject = FileObject;
    IrpSp->Flags = SL_RESTART_SCAN;
    IrpSp->Parameters.QueryDirectory.Length = Length;
    IrpSp->Parameters.QueryDirectory.FileName = FileName;
    IrpSp->Parameters.QueryDirectory.FileInformationClass = FileInformationClass;

    IoSetCompletionRoutine(Irp, IoCompletionRoutine, 0, TRUE, TRUE, TRUE);
    ntStatus = IoCallDriver(FileObject->Vpb->DeviceObject, Irp);
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, 0);

    return IoStatusBlock->Status;
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
    KEVENT kEvent;
    PIO_STACK_LOCATION IrpSp;

    if (FileObject->Vpb == 0 || FileObject->Vpb->DeviceObject == NULL) return STATUS_UNSUCCESSFUL;

    Irp = IoAllocateIrp(FileObject->Vpb->DeviceObject->StackSize, FALSE);
    if (Irp == NULL) return STATUS_INSUFFICIENT_RESOURCES;

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

    RtlZeroMemory(FileInformation, Length);

    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->UserEvent = &kEvent;
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
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, 0);

    return IoStatusBlock->Status;
}

NTSTATUS
IrpSetInformationFile(
	IN HANDLE  FileHandle,
    IN PVOID  FileInformation,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
	IN ULONG  Length
)
{
    NTSTATUS ntStatus;
    PIRP Irp;
    KEVENT kEvent;
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

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

    Irp->AssociatedIrp.SystemBuffer = FileInformation;
    Irp->UserEvent = &kEvent;
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
    KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, 0);
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
    KEVENT kEvent;
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

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

    Irp->UserEvent = &kEvent;
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
    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, 0);

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
    KEVENT kEvent;
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

    KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

    Irp->UserEvent = &kEvent;
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

    if (ntStatus == STATUS_PENDING) KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, NULL);

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
    status = IrpSetInformationFile(hFile, &fileBaseInfo, FileBasicInformation, sizeof(fileBaseInfo));
    DbgPrint("IrpSetInformationFile[SetInformation] Error[0x%X]\n", status);
    if (!NT_SUCCESS(status)) goto Cleanup;

    // 发送IRP设置文件属性, 设置删除文件操作
    status = MyDeleteFile(hFile);
    DbgPrint("MyDeleteFile Error[0x%X]\n", status);

Cleanup:
	ZwClose(hFile);
    return status;
}

NTSTATUS FakeDiskWriteDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
#define STATUS_INVALID_VALUE 0xC000000D
    NTSTATUS status = STATUS_SUCCESS;
    IO_STACK_LOCATION* irpSp = IoGetCurrentIrpStackLocation(Irp);
    LARGE_INTEGER writeOffsetInBytes = irpSp->Parameters.Write.ByteOffset;
    ULONG writeLength = irpSp->Parameters.Write.Length;

    //DbgPrint("Enter the FakeDiskWriteDispatch,CurrentPID:%d", PsGetCurrentProcessId());

    // 保护MBR：MBR位于磁盘的起始扇区（0扇区）
    if (writeOffsetInBytes.QuadPart == 0 && writeLength >= 512)
    {
        // 拒绝写入MBR
        DbgPrint("已拒绝MBR写操作");

        // 获取写入的数据缓冲区
        PUCHAR buffer = Irp->AssociatedIrp.SystemBuffer; // 系统缓冲区
        if (!buffer) buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority); // 如果使用 MDL

        // 打印或处理写入的数据
        DbgPrint("Data to be written:\n");
        if (!buffer) {
            DbgPrint("Buffer is NULL.\n");
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
		}
        _try{
            for (ULONG i = 0; i < writeLength; i++) {
                DbgPrint("%02llX ", (ULONG_PTR)buffer[i]);
            }
        }
        _except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("Exception occurred while accessing buffer.\n");
		}
        DbgPrint("\n");
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    // 其他写操作正常处理
    return g_OriginalDiskWriteDispatch(DeviceObject, Irp);
}

NTSTATUS FakeDiskReadDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
#define STATUS_IO_ERROR 0xC0000036
    NTSTATUS status = STATUS_SUCCESS;
    IO_STACK_LOCATION* irpSp = IoGetCurrentIrpStackLocation(Irp);
    LARGE_INTEGER readOffsetInBytes = irpSp->Parameters.Read.ByteOffset;
    ULONG readLength = irpSp->Parameters.Read.Length;

    //DbgPrint("Enter the FakeDiskReadDispatch,CurrentPID:%d", PsGetCurrentProcessId());

    // 保护MBR：MBR位于磁盘的起始扇区（0扇区）
    if (readOffsetInBytes.QuadPart == 0 && readLength >= 512)
    {
        // 拒绝读取MBR
        DbgPrint("已拒绝MBR读操作并返回假结果!");

        // 获取读取的数据缓冲区
        PUCHAR buffer = Irp->AssociatedIrp.SystemBuffer; // 系统缓冲区
        if (!buffer) buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority); // 如果使用 MDL

        // 伪造数据
        PUCHAR fakeData = (PUCHAR)"Hello, I'm Faked MBR Data!";
        size_t fakeDataLength = strlen((const char*)fakeData) + 1; // 包括字符串的结束符

        // 将伪造数据写入缓冲区
        RtlCopyMemory(buffer, fakeData, fakeDataLength);

        // 设置 IRP 的状态和信息
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = fakeDataLength;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_VALUE;//发生I/O错误
    }

    // 其他写操作正常处理
    return g_OriginalDiskReadDispatch(DeviceObject, Irp);
}

NTSTATUS FakeDiskDeviceControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioctlCode = stack->Parameters.DeviceIoControl.IoControlCode;

    // 检查是否是格式化请求
    if (ioctlCode == IOCTL_DISK_FORMAT_TRACKS || ioctlCode == IOCTL_DISK_FORMAT_PARTITIONS) {
        // 拒绝格式化请求
        DbgPrint("已拦截格式化请求!");
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_ACCESS_DENIED;
    }

    // 调用原始的派遣函数
    return g_OriginalDiskDeviceControlDispatch(DeviceObject, Irp);
}

NTSTATUS HookDiskWriteDispatch()
{
    NTSTATUS status = STATUS_SUCCESS;
    PDRIVER_OBJECT diskDrvObj;
    UNICODE_STRING uniObjName;

    // 初始化驱动对象名称
    RtlInitUnicodeString(&uniObjName, L"\\Driver\\Disk");

    // 获取disk.sys驱动对象
    status = ObReferenceObjectByName(&uniObjName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &diskDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return status;
    }

    // 保存原始的IRP_MJ_WRITE函数指针
    g_OriginalDiskReadDispatch = diskDrvObj->MajorFunction[IRP_MJ_READ];
    g_OriginalDiskWriteDispatch = diskDrvObj->MajorFunction[IRP_MJ_WRITE];
    g_OriginalDiskDeviceControlDispatch = diskDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL];

    // 替换为自定义的IRP_MJ_WRITE函数
    diskDrvObj->MajorFunction[IRP_MJ_READ] = FakeDiskReadDispatch;
    diskDrvObj->MajorFunction[IRP_MJ_WRITE] = FakeDiskWriteDispatch;
    diskDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = FakeDiskDeviceControlDispatch;

    DbgPrint("IRP Hook Success!");

    // 设置挂钩状态
    //g_bHooked = TRUE;

    // 释放对驱动对象的引用
    ObDereferenceObject(diskDrvObj);

    return status;
}

NTSTATUS UnhookDiskWriteDispatch()
{
    NTSTATUS status = STATUS_SUCCESS;
    PDRIVER_OBJECT diskDrvObj;
    UNICODE_STRING uniObjName;

    // 初始化驱动对象名称
    RtlInitUnicodeString(&uniObjName, L"\\Driver\\Disk");

    // 获取disk.sys驱动对象
    status = ObReferenceObjectByName(&uniObjName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &diskDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return status;
    }

    // 检查是否已经挂钩
    //if (g_bHooked)
    //{
    // 恢复原始的IRP_MJ_WRITE函数指针
    diskDrvObj->MajorFunction[IRP_MJ_READ] = g_OriginalDiskReadDispatch;
    diskDrvObj->MajorFunction[IRP_MJ_WRITE] = g_OriginalDiskWriteDispatch;
    //g_bHooked = FALSE;
    //}

    DbgPrint("IRP Unhook Success!");

    // 释放对驱动对象的引用
    ObDereferenceObject(diskDrvObj);

    return status;
}

NTSTATUS FakeStorPortScsiDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    IO_STACK_LOCATION* irpSp = IoGetCurrentIrpStackLocation(Irp);
    SCSI_REQUEST_BLOCK* srb = (SCSI_REQUEST_BLOCK*)irpSp->Parameters.Scsi.Srb;

    // 检查是否为写入MBR的操作
    if (srb->Cdb[0] == SCSIOP_WRITE && srb->Cdb[2] == 0 && srb->Cdb[3] == 0)
    {
        // 拒绝写入MBR
        DbgPrint("已拒绝MBR写操作");
        Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_ACCESS_DENIED;
    }

    // 其他操作正常处理
    return g_OriginalStorPortScsiDispatch(DeviceObject, Irp);
}

NTSTATUS HookStorPortScsiDispatch()
{
    NTSTATUS status = STATUS_SUCCESS;
    PDRIVER_OBJECT storPortDrvObj;
    UNICODE_STRING uniObjName;
    //IoDeviceObjectType
    // 初始化驱动对象名称
    RtlInitUnicodeString(&uniObjName, L"\\Driver\\StorPort");//SCSI Miniport驱动

    // 获取disk.sys驱动对象
    status = ObReferenceObjectByName(&uniObjName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &storPortDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return status;
    }

    // 保存原始的IRP_MJ_SCSI函数指针
    g_OriginalStorPortScsiDispatch = storPortDrvObj->MajorFunction[IRP_MJ_SCSI];

    // 替换为自定义的IRP_MJ_SCSI函数
    storPortDrvObj->MajorFunction[IRP_MJ_SCSI] = FakeStorPortScsiDispatch;

    DbgPrint("IRP Hook Success!");

    // 设置挂钩状态
    //g_bHooked = TRUE;

    // 释放对驱动对象的引用
    ObDereferenceObject(storPortDrvObj);

    return status;
}

VOID UnhookStorPortScsiDispatch()
{
    NTSTATUS status = STATUS_SUCCESS;
    PDRIVER_OBJECT storPortDrvObj;
    UNICODE_STRING uniObjName;

    // 初始化驱动对象名称
    RtlInitUnicodeString(&uniObjName, L"\\Driver\\StorPort");//SCSI Miniport驱动

    // 获取disk.sys驱动对象
    status = ObReferenceObjectByName(&uniObjName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &storPortDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return;
    }
    //if (g_bHooked)
    //{
    storPortDrvObj->MajorFunction[IRP_MJ_SCSI] = g_OriginalStorPortScsiDispatch;
    //g_bHooked = FALSE;
    //}

    DbgPrint("IRP Unhook Success!");

    ObDereferenceObject(storPortDrvObj);
}

//通过FILE_OBJECT拿到文件名
BOOLEAN QueryFileObjectDosName(PFILE_OBJECT pFileObject, PWCHAR OutputBufferFreeByCaller)
{
    UNICODE_STRING volumeDosName = { 0 };
    //初始化输出路径
    OutputBufferFreeByCaller = ExAllocatePool2(POOL_FLAG_NON_PAGED, pFileObject->FileName.Length + 32, 'aaaa');
    RtlZeroMemory(OutputBufferFreeByCaller, pFileObject->FileName.Length + 32);
    //取得盘符DOS名
    RtlInitEmptyUnicodeString(&volumeDosName, NULL, 0);
    if (!NT_SUCCESS(IoVolumeDeviceToDosName(pFileObject->DeviceObject, &volumeDosName)) && volumeDosName.Buffer == NULL) return FALSE;
    memcpy(OutputBufferFreeByCaller, volumeDosName.Buffer, volumeDosName.Length);
    //连接文件名
    memcpy((PUCHAR)OutputBufferFreeByCaller + volumeDosName.Length, pFileObject->FileName.Buffer, pFileObject->FileName.Length);
    return TRUE;
}

NTSTATUS FakeNtfsCreateDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    // 获取当前堆栈位置
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    // 获取文件名
    WCHAR wFileName[260] = { 0 };
    QueryFileObjectDosName(stack->FileObject, wFileName);
    DbgPrint("Process %Iu try to create/open the file %ls", (ULONG_PTR)PsGetCurrentProcessId(), wFileName);

    /*
    if (NT_SUCCESS(status)) {
        // 将 OBJECT_NAME_INFORMATION 转换为 UNICODE_STRING
        fileName.Buffer = objectNameInfo.Name.Buffer;
        fileName.Length = objectNameInfo.Name.Length;
        fileName.MaximumLength = objectNameInfo.Name.MaximumLength;

        DbgPrint("Process %d try to create/open the file %wS", PsGetCurrentProcessId(), objectNameInfo.Name.Buffer);
        // 检查文件名是否受保护
        if (RtlEqualUnicodeString(&fileName, &ProtectedFilePath, TRUE)) {
            // 拒绝创建或打开受保护的文件
            Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_ACCESS_DENIED;
        }
    }*/
    return g_OriginalNtfsDispatch(DeviceObject, Irp);
}

NTSTATUS HookNtfsDispatch()
{
    UNICODE_STRING ntfsName;
    PDRIVER_OBJECT ntfsDrvObj = NULL;
    OBJECT_ATTRIBUTES objAttr;
    NTSTATUS status;

    RtlInitUnicodeString(&ntfsName, L"\\FileSystem\\Ntfs");
    InitializeObjectAttributes(&objAttr, &ntfsName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // 获取ntfs.sys驱动对象
    status = ObReferenceObjectByName(&ntfsName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &ntfsDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return STATUS_UNSUCCESSFUL;
    }

    // 保存原始派遣函数
    g_OriginalNtfsDispatch = ntfsDrvObj->MajorFunction[IRP_MJ_CREATE];

    // 替换为自定义的派遣函数
    ntfsDrvObj->MajorFunction[IRP_MJ_CREATE] = FakeNtfsCreateDispatch;

    ObDereferenceObject(ntfsDrvObj);

    return STATUS_SUCCESS;
}

VOID UnhookNtfsDispatch()
{
    UNICODE_STRING ntfsName;
    PDRIVER_OBJECT ntfsDrvObj = NULL;
    OBJECT_ATTRIBUTES objAttr;
    NTSTATUS status;

    RtlInitUnicodeString(&ntfsName, L"\\FileSystem\\Ntfs");
    InitializeObjectAttributes(&objAttr, &ntfsName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    // 获取ntfs.sys驱动对象
    status = ObReferenceObjectByName(&ntfsName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        &ntfsDrvObj);

    if (!NT_SUCCESS(status))
    {
        DbgPrint("ObReferenceObjectByName Failed!NTSTATUS = %X", status);
        return;
    }

    // 恢复原始派遣函数
    ntfsDrvObj->MajorFunction[IRP_MJ_CREATE] = g_OriginalNtfsDispatch;
    ObDereferenceObject(ntfsDrvObj);
}

VOID RtlGetEmptyUnicodeString(_Out_ PUNICODE_STRING str, _In_ USHORT length) {
    str->Length = length * sizeof(WCHAR);
    str->MaximumLength = (length + 1) * sizeof(WCHAR);
    str->Buffer = (PWSTR)ExAllocatePool2(POOL_FLAG_NON_PAGED, str->MaximumLength, 'MyTg');
    if (str->Buffer != NULL) {
        RtlZeroMemory(str->Buffer, str->Length);
    }
}

/*FLT_PREOP_CALLBACK_STATUS PreAntiDelete(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(CompletionContext);
    PAGED_CODE(); // PreCallback的IRQL应当<= APC_LEVEL

    FLT_PREOP_CALLBACK_STATUS Status = FLT_PREOP_SUCCESS_NO_CALLBACK; // 不再调用postoperation callback routine

    BOOLEAN IsDirectory; // 目录操作跳过，不管
    NTSTATUS status = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &IsDirectory);
    if (NT_SUCCESS(status)) {
        if (IsDirectory == TRUE) {
            return Status;
        }
    }

    if (Data->Iopb->MajorFunction == IRP_MJ_CREATE) {
        if (!FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DELETE_ON_CLOSE)) {
            return Status;
        }
    }

    if (Data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION) {
        switch (Data->Iopb->Parameters.SetFileInformation.FileInformationClass) {
        case FileRenameInformation:
        case FileRenameInformationEx:
        case FileDispositionInformation:
        case FileDispositionInformationEx:
        case FileRenameInformationBypassAccessCheck:
        case FileRenameInformationExBypassAccessCheck:
        case FileShortNameInformation:
            break;
        default:
            return Status;
        }
    }

    
    ZwCreateFile和ZwSetInformation函数都是运行在PASSIVE_LEVEL的，所以当前的线程上下文
    应当就是ZwCreateFile或ZwSetInformation函数的调用者进程对应的线程的上下文Context,
    所以这里可以判断调用者进程是否在白名单中(如果存在白名单进程的话)
    if (IoThreadToProcess(Data->Thread) == WhiteListProcess) {
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    

    DbgPrint("Enter the callback, Caller:%d,CurrentPID:%d", PsGetProcessId(IoThreadToProcess(Data->Thread)), PsGetCurrentProcessId());

    UNICODE_STRING ProtectedFileName = RTL_CONSTANT_STRING(L"D:\\example.txt");

    PFLT_FILE_NAME_INFORMATION FileNameInfo = NULL;
    if (FltObjects->FileObject != NULL) {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
        if (NT_SUCCESS(status)) {
            FltParseFileNameInformation(FileNameInfo);
            //if (RtlCompareUnicodeString(&FileNameInfo->Name, &Protected, TRUE) == 0) {
            if (RtlCompareUnicodeString(&FileNameInfo->Extension, &ProtectedFileName, TRUE) == 0) {
                DbgPrint("Protecting file success!");
                Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                Data->IoStatus.Information = 0;
                Status = FLT_PREOP_COMPLETE;
            }
            // Clean up file name information.
            FltReleaseFileNameInformation(FileNameInfo);
        }
    }

    return Status;
}*/

// 动态附加到所有卷
/*NTSTATUS FltAttachToAllVolumes(PFLT_FILTER FilterHandle) {
    FLT_VOLUME_LIST_ENTRY* volumeList = NULL;
    ULONG numVolumes = 0;

    // 获取系统中所有卷的列表
    NTSTATUS status = FltGetVolumeList(FilterHandle, &volumeList, &numVolumes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (ULONG i = 0; i < numVolumes; i++) {
        status = FltAttachVolume(FilterHandle, volumeList[i].Volume, NULL, NULL);
        if (!NT_SUCCESS(status)) {
            DbgPrint("Failed to attach to volume: %x\n", status);
        }
    }

    // 释放卷列表
    FltReleaseVolumeList(volumeList);
    return STATUS_SUCCESS;
}

// 动态从所有卷分离
NTSTATUS FltDetachFromAllVolumes(PFLT_FILTER FilterHandle) {
    FLT_VOLUME_LIST_ENTRY* volumeList = NULL;
    ULONG numVolumes = 0;

    // 获取系统中所有卷的列表
    NTSTATUS status = FltGetVolumeList(FilterHandle, &volumeList, &numVolumes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (ULONG i = 0; i < numVolumes; i++) {
        status = FltDetachVolume(FilterHandle, volumeList[i].Volume);
        if (!NT_SUCCESS(status)) {
            DbgPrint("Failed to detach from volume: %x\n", status);
        }
    }

    // 释放卷列表
    FltReleaseVolumeList(volumeList);
    return STATUS_SUCCESS;
}*/

/*NTSTATUS RegisterMiniFilter(PDRIVER_OBJECT pDriverObject)
{
    NTSTATUS status;
    status = FltRegisterFilter(pDriverObject, &FilterRegistration, &hFilter);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Failed to register hFilter: <0x%08x>.\n", status);
        return status;
    }

    status = FltStartFiltering(hFilter);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Failed to start hFilter: <0x%08x>.\n", status);
        FltUnregisterFilter(hFilter);
        hFilterUnregistered = TRUE;
    }
    return STATUS_SUCCESS;
}

VOID UnregisterMiniFilter()
{
    if (hFilter != NULL && !hFilterUnregistered) FltUnregisterFilter(hFilter);
}*/

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