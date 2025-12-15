#pragma once
#include "ntifs.h"
#include "fltKernel.h"
#include "storport.h"
#include "OtherFunctions.h"

typedef struct _AUX_ACCESS_DATA {
    PPRIVILEGE_SET PrivilegesUsed;
    GENERIC_MAPPING GenericMapping;
    ACCESS_MASK AccessesToAudit;
    ULONG Reserve;                           
} AUX_ACCESS_DATA, * PAUX_ACCESS_DATA;

NTKERNELAPI
NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING  ObjectName,
    IN ACCESS_MASK  DesiredAccess,
    OUT PFILE_OBJECT* FileObject,
    OUT PDEVICE_OBJECT* DeviceObject
);

NTKERNELAPI
NTSTATUS
ObCreateObject(
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID* Object
);

NTKERNELAPI
NTSTATUS
SeCreateAccessState(
    IN PACCESS_STATE AccessState,
    IN PAUX_ACCESS_DATA AuxData,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping OPTIONAL
);

NTSTATUS
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
);

NTKERNELAPI NTSTATUS ObReferenceObjectByName(
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PVOID* Object
);

static UNICODE_STRING Protected = RTL_CONSTANT_STRING(L"123");
//PWCHAR ProtectedFilePath = L"\\??\\D:\\example.txt";

extern POBJECT_TYPE *IoDriverObjectType;

extern PDRIVER_DISPATCH g_OriginalDiskWriteDispatch;
extern PDRIVER_DISPATCH g_OriginalDiskReadDispatch;
extern PDRIVER_DISPATCH g_OriginalDiskDeviceControlDispatch;
extern PDRIVER_DISPATCH g_OriginalStorPortScsiDispatch;
extern PDRIVER_DISPATCH g_OriginalNtfsDispatch;
//BOOLEAN g_bHooked = FALSE;

#define IOCTL_DISK_FORMAT_TRACKS 0x7000C
#define IOCTL_DISK_FORMAT_PARTITIONS 0x7001C

//
// IrpCreateFile
//
// This routine is used as NtCreateFile but first and third parameter.
//
// Inputs:
// DesiredAccess - Specifies an ACCESS_MASK value that determines
//                     the requested access to the object.
// FilePath - Path of file to create,as L"C://Windows"(Unicode).
// AllocationSize - Pointer to a LARGE_INTEGER that contains the initial allocation
//                     size, in bytes, for a file that is created or overwritten.
// FileAttributes - Specifies one or more FILE_ATTRIBUTE_XXX flags, which represent
//                     the file attributes to set if you are creating or overwriting a file.
// ShareAccess - Type of share access.
// CreateDisposition - Specifies the action to perform if the file does or does not exist.
// CreateOptions - Specifies the options to apply when creating or opening the file.
// EaBuffer - For device and intermediate drivers, this parameter must be a NULL pointer.
// EaLength - For device and intermediate drivers, this parameter must be zero.
//
// Ouputs:
// FileObject - Pointer to a PFILE_OBJECT variable that
//                 receives a PFILE_OBJECT to the file.
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtCreateFile,but return FILE_OBJECT not HANDLE.
//

NTSTATUS
IrpCreateFile(
    OUT PFILE_OBJECT* FileObject,
    IN ACCESS_MASK  DesiredAccess,
    IN PUNICODE_STRING  FilePath,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PLARGE_INTEGER  AllocationSize OPTIONAL,
    IN ULONG  FileAttributes,
    IN ULONG  ShareAccess,
    IN ULONG  CreateDisposition,
    IN ULONG  CreateOptions,
    IN PVOID  EaBuffer OPTIONAL,
    IN ULONG  EaLength
);

//
// IrpClose
//
// This routine is used as ObDereferenceObject.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT variable that will close
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to ObDereferenceObject
//

NTSTATUS
IrpClose(
    IN PFILE_OBJECT  FileObject
);

//
// IrpQueryDirectoryFile
//
// This routine is used as NtQueryDirectoryFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass.
// FileInformationClass - Type of information to be returned about files in the directory.
// FileName - Pointer to a caller-allocated Unicode string containing the name of a file
//            (or multiple files, if wildcards are used) within the directory specified by FileHandle.
//            This parameter is optional and can be NULL.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
// FileInformation - Pointer to a buffer that receives the desired
//                          information about the file.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtQueryDirectoryFile, but no ApcRoutine.
//

NTSTATUS
IrpQueryDirectoryFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
    IN PUNICODE_STRING  FileName  OPTIONAL
);

//
// IrpQueryInformationFile
//
// This routine is used as NtQueryInformationFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass.
// FileInformationClass - Type of information to be returned about files in the directory.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
// FileInformation - Pointer to a buffer that receives the desired
//                          information about the file.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtQueryInformationFile.
//

NTSTATUS
IrpQueryInformationFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass
);

//
// IrpSetInformationFile
//
// This routine is used as NtSetInformationFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// FileInformation - Pointer to a buffer that contains the information to set for the file. 
// Length - Size, in bytes, of the buffer pointed to by FileInformation. The caller
//            should set this parameter according to the given FileInformationClass.
// FileInformationClass - Type of information to be returned about files in the directory.
// ReplaceIfExists - Set to TRUE to specify that if a file with the same name already exists,
//                     it should be replaced with the given file. Set to FALSE if the rename
//                     operation should fail if a file with the given name already exists.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtSetInformationFile.
//

/*NTSTATUS
IrpSetInformationFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  FileInformation,
    IN ULONG  Length,
    IN FILE_INFORMATION_CLASS  FileInformationClass,
    IN BOOLEAN  ReplaceIfExists
);*/

//
// IrpReadFile
//
// This routine is used as NtReadFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Buffer - Pointer to a caller-allocated buffer that receives the data read from the file.
// Length - The size, in bytes, of the buffer pointed to by Buffer.
// ByteOffset - Pointer to a variable that specifies the starting byte offset
//                 in the file where the read operation will begin.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                completion status and information about the requested read operation.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtReadFile, but no ApcRoutine.
//

NTSTATUS
IrpReadFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    OUT PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL
);

//
// IrpReadFile
//
// This routine is used as NtReadFile.
//
// Inputs:
// FileObject - Pointer to a PFILE_OBJECT.
// Buffer - Pointer to a caller-allocated buffer that contains the data to write to the file.
// Length - The size, in bytes, of the buffer pointed to by Buffer.
// ByteOffset - Pointer to a variable that specifies the starting byte offset
//                 in the file for beginning the write operation.
//
// Ouputs:
// IoStatusBlock - Pointer to an IO_STATUS_BLOCK structure that receives the final
//                 completion status and information about the requested read operation.
//
// Returns:
// The IRP send status.
//
// Notes:
// This is equivalent to NtReadFile, but no ApcRoutine.
//

NTSTATUS
IrpWriteFile(
    IN PFILE_OBJECT  FileObject,
    OUT PIO_STATUS_BLOCK  IoStatusBlock,
    IN PVOID  Buffer,
    IN ULONG  Length,
    IN PLARGE_INTEGER  ByteOffset  OPTIONAL
);

NTSTATUS
IoCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);

UNICODE_STRING RtlGetUnicodeString(LPWSTR wStr);

NTSTATUS MyCreateFile(PUNICODE_STRING ustrFileName, PHANDLE pFileHandle);

NTSTATUS MyDeleteFile(HANDLE FileHandle);

// 强制删除文件
NTSTATUS ForceDeleteFile(UNICODE_STRING ustrFileName);

NTSTATUS FakeDiskWriteDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS FakeDiskReadDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS FakeDiskDeviceControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS HookDiskWriteDispatch();

NTSTATUS UnhookDiskWriteDispatch();

NTSTATUS FakeStorPortScsiDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

NTSTATUS HookStorPortScsiDispatch();

VOID UnhookStorPortScsiDispatch();

//通过FILE_OBJECT拿到文件名
BOOLEAN QueryFileObjectDosName(PFILE_OBJECT pFileObject, PWCHAR OutputBufferFreeByCaller);

NTSTATUS FakeNtfsCreateDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS HookNtfsDispatch();

VOID UnhookNtfsDispatch();

VOID RtlGetEmptyUnicodeString(_Out_ PUNICODE_STRING str, _In_ USHORT length);

FLT_PREOP_CALLBACK_STATUS PreAntiDelete(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext);

NTSTATUS RegisterMiniFilter(PDRIVER_OBJECT pDriverObject);

VOID UnregisterMiniFilter();

NTSTATUS GetNtfsCommonCleanup(PVOID* pNtfsCommonCleanup);

PVOID GetNtfsDecrementCleanupCounts();

NTSTATUS DeleteFileByXCBFunction(PUNICODE_STRING ustrFileName);

BOOLEAN IsFileReallyDeleted(PUNICODE_STRING FileName);