#include "KmdfCallbacks.h"
#include "Symbol.h"
#include "Memory.h"
#include <wdf.h>

typedef struct _KMDF_CALLBACK_DESCRIPTOR
{
    ULONG Id;
    ULONG ContainerType;
    LONG WrapperOffset;
    LONG MethodOffset;
} KMDF_CALLBACK_DESCRIPTOR, * PKMDF_CALLBACK_DESCRIPTOR;

static KMDF_SYMBOL_LAYOUT g_KmdfLayout;
static FAST_MUTEX g_KmdfLayoutLock;
static BOOLEAN g_KmdfLayoutInitialized = FALSE;

static BOOLEAN KmdfIsKernelPointer(PVOID Address)
{
#ifdef _WIN64
    return Address && (ULONG_PTR)Address >= 0xFFFF800000000000ULL;
#else
    return Address != NULL;
#endif
}

static VOID KmdfSetFallbackOffsets(PKMDF_SYMBOL_OFFSETS O)
{
    RtlFillMemory(O, sizeof(*O), 0xFF);

    O->FxContextHeader_Object = 0x000;
    O->FxContextHeader_Context = 0x030;

    O->FxDevice_Driver = 0x088;
    O->FxDevice_DeviceObject = 0x090;
    O->MxDeviceObject_DeviceObject = 0x000;
    O->FxDevice_PkgPnp = 0x288;

    O->FxDriver_DriverDeviceAdd = 0x0A8;
    O->FxDriver_DriverUnload = 0x168;
    O->FxDriverDeviceAdd_Method = 0x008;
    O->FxDriverUnload_Method = 0x000;

    O->FxPkgPnp_SelfManagedIoMachine = 0x388;

    O->FxPkgPnp_DeviceUsageNotification = 0x528;
    O->FxPkgPnp_DeviceUsageNotificationEx = 0x530;
    O->FxPkgPnp_DeviceRelationsQuery = 0x538;
    O->FxPkgPnp_DeviceD0Entry = 0x540;
    O->FxPkgPnp_DeviceD0EntryPostInterruptsEnabled = 0x570;
    O->FxPkgPnp_DeviceD0ExitPreInterruptsDisabled = 0x578;
    O->FxPkgPnp_DeviceD0Exit = 0x580;
    O->FxPkgPnp_DevicePrepareHardware = 0x600;
    O->FxPkgPnp_DeviceReleaseHardware = 0x638;
    O->FxPkgPnp_DeviceQueryStop = 0x668;
    O->FxPkgPnp_DeviceQueryRemove = 0x670;
    O->FxPkgPnp_DeviceSurpriseRemoval = 0x678;

    O->FxPnpDeviceUsageNotification_Method = 0x000;
    O->FxPnpDeviceUsageNotificationEx_Method = 0x000;
    O->FxPnpDeviceRelationsQuery_Method = 0x000;
    O->FxPnpDeviceD0Entry_Method = 0x018;
    O->FxPnpDeviceD0EntryPostInterruptsEnabled_Method = 0x000;
    O->FxPnpDeviceD0ExitPreInterruptsDisabled_Method = 0x000;
    O->FxPnpDeviceD0Exit_Method = 0x018;
    O->FxPnpDevicePrepareHardware_Method = 0x018;
    O->FxPnpDeviceReleaseHardware_Method = 0x018;
    O->FxPnpDeviceQueryStop_Method = 0x000;
    O->FxPnpDeviceQueryRemove_Method = 0x000;
    O->FxPnpDeviceSurpriseRemoval_Method = 0x018;

    /*
     * FxSelfManagedIoMachine 内部成员暂不硬编码。
     * 正常情况下由 Wdf01000.pdb 解析。
     * 未验证版本绝不能猜 offset。
     */
}

static VOID KmdfBuildFallbackLayout(PKMDF_SYMBOL_LAYOUT Layout)
{
    RtlZeroMemory(Layout, sizeof(*Layout));
    RtlFillMemory(&Layout->Offsets, sizeof(Layout->Offsets), 0xFF);

    Layout->Version = KMDF_SYMBOL_LAYOUT_VERSION;
    Layout->Size = sizeof(*Layout);
    Layout->Source = KMDF_LAYOUT_SOURCE_FALLBACK;
    Layout->DispatchPolicyKnown = FALSE;
    Layout->DispatchPolicy = KmdfDispatchPolicyUnknown;

    Layout->Offsets.FxContextHeader_Object = 0x000;
    Layout->Offsets.FxContextHeader_Context = 0x030;

    Layout->Offsets.FxDevice_Driver = 0x088;
    Layout->Offsets.FxDevice_DeviceObject = 0x090;
    Layout->Offsets.MxDeviceObject_DeviceObject = 0x000;
    Layout->Offsets.FxDevice_PkgPnp = 0x288;

    Layout->Offsets.FxDriver_DriverDeviceAdd = 0x0A8;
    Layout->Offsets.FxDriver_DriverUnload = 0x168;
    Layout->Offsets.FxDriverDeviceAdd_Method = 0x008;
    Layout->Offsets.FxDriverUnload_Method = 0x000;

    Layout->Offsets.FxPkgPnp_DeviceD0Entry = 0x540;
    Layout->Offsets.FxPnpDeviceD0Entry_Method = 0x018;

    Layout->Offsets.FxPkgPnp_DeviceD0Exit = 0x580;
    Layout->Offsets.FxPnpDeviceD0Exit_Method = 0x018;

    Layout->Offsets.FxPkgPnp_DevicePrepareHardware = 0x600;
    Layout->Offsets.FxPnpDevicePrepareHardware_Method = 0x018;

    Layout->Offsets.FxPkgPnp_DeviceReleaseHardware = 0x638;
    Layout->Offsets.FxPnpDeviceReleaseHardware_Method = 0x018;
}

NTSTATUS InitializeKmdfCallbackSupport()
{
    ExInitializeFastMutex(&g_KmdfLayoutLock);

    KmdfBuildFallbackLayout(&g_KmdfLayout);

    g_KmdfLayoutInitialized = TRUE;

    return STATUS_SUCCESS;
}

static NTSTATUS KmdfGetFxDevice(
    PDEVICE_OBJECT DeviceObject,
    const KMDF_SYMBOL_OFFSETS* Offsets,
    PVOID* FxDevice
)
{
    if (!DeviceObject || !Offsets || !FxDevice)
        return STATUS_INVALID_PARAMETER;

    *FxDevice = NULL;

    __try
    {
        PVOID deviceExtension = DeviceObject->DeviceExtension;

        if (!deviceExtension)
            return STATUS_NOT_FOUND;

        PUCHAR header =
            (PUCHAR)deviceExtension -
            Offsets->FxContextHeader_Context;

        PVOID object =
            *(PVOID*)(header +
                Offsets->FxContextHeader_Object);

        if (!KmdfIsKernelPointer(object))
            return STATUS_NOT_FOUND;

        PDEVICE_OBJECT backDeviceObject =
            *(PDEVICE_OBJECT*)((PUCHAR)object +
                Offsets->FxDevice_DeviceObject +
                Offsets->MxDeviceObject_DeviceObject);

        if (backDeviceObject != DeviceObject)
            return STATUS_NOT_FOUND;

        *FxDevice = object;
        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return GetExceptionCode();
    }
}

static BOOLEAN KmdfReadCallback(
    PVOID Container,
    LONG WrapperOffset,
    LONG MethodOffset,
    PVOID* Address
)
{
    if (!Container || !Address ||
        WrapperOffset < 0 ||
        MethodOffset < 0)
        return FALSE;

    *Address = NULL;

    __try
    {
        PVOID callback =
            *(PVOID*)((PUCHAR)Container +
                WrapperOffset +
                MethodOffset);

        if (callback && !KmdfIsKernelPointer(callback))
            return FALSE;

        *Address = callback;
        return TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
}

//static VOID KmdfAddCallback(
//    PKMDF_CALLBACK_ENUM_RESULT Result,
//    ULONG Id,
//    ULONG ContainerType,
//    PVOID Container,
//    LONG WrapperOffset,
//    LONG MethodOffset
//)
//{
//    PVOID callback = NULL;
//
//    if (!Result ||
//        Result->Count >= KMDF_MAX_CALLBACKS ||
//        !Container)
//        return;
//
//    if (!KmdfReadCallback(
//        Container,
//        WrapperOffset,
//        MethodOffset,
//        &callback))
//        return;
//
//    /*
//     * NULL = 驱动没有注册该 Evt。
//     * Current Callback 枚举只返回实际存在的 callback。
//     */
//    if (!callback)
//        return;
//
//    PKMDF_CALLBACK_INFO info =
//        &Result->Callbacks[Result->Count++];
//
//    info->Id = Id;
//    info->ContainerType = ContainerType;
//    info->Address = (ULONG64)callback;
//    info->ContainerObject = (ULONG64)Container;
//}

static VOID KmdfAddCallback(
    PKMDF_CALLBACK_ENUM_RESULT Result,
    ULONG Id,
    ULONG ContainerType,
    PVOID Container,
    LONG WrapperOffset,
    LONG MethodOffset
)
{
    PVOID callback = NULL;

    if (!Result)
    {
        DbgPrint("[KMDF] Add id=%lu: Result NULL\n", Id);
        return;
    }

    if (Result->Count >= KMDF_MAX_CALLBACKS)
    {
        DbgPrint("[KMDF] Add id=%lu: MAX callbacks\n", Id);
        return;
    }

    if (!Container)
    {
        DbgPrint("[KMDF] Add id=%lu: Container NULL\n", Id);
        return;
    }

    if (WrapperOffset < 0 || MethodOffset < 0)
    {
        DbgPrint(
            "[KMDF] Add id=%lu: invalid offsets wrapper=%X method=%X\n",
            Id,
            WrapperOffset,
            MethodOffset);
        return;
    }

    if (!KmdfReadCallback(
        Container,
        WrapperOffset,
        MethodOffset,
        &callback))
    {
        DbgPrint(
            "[KMDF] Add id=%lu: read failed container=%p wrapper=%X method=%X\n",
            Id,
            Container,
            WrapperOffset,
            MethodOffset);
        return;
    }

    if (!callback)
    {
        DbgPrint(
            "[KMDF] Add id=%lu: callback NULL container=%p wrapper=%X method=%X\n",
            Id,
            Container,
            WrapperOffset,
            MethodOffset);
        return;
    }

    DbgPrint(
        "[KMDF] Add id=%lu OK callback=%p container=%p wrapper=%X method=%X\n",
        Id,
        callback,
        Container,
        WrapperOffset,
        MethodOffset);

    PKMDF_CALLBACK_INFO info =
        &Result->Callbacks[Result->Count++];

    info->Id = Id;
    info->ContainerType = ContainerType;
    info->Address = (ULONG64)callback;
    info->ContainerObject = (ULONG64)Container;
}

static BOOLEAN KmdfGetLayoutSnapshot(
    PKMDF_SYMBOL_LAYOUT Layout
)
{
    if (!Layout || !g_KmdfLayoutInitialized)
        return FALSE;

    ExAcquireFastMutex(&g_KmdfLayoutLock);

    RtlCopyMemory(
        Layout,
        &g_KmdfLayout,
        sizeof(*Layout));

    ExReleaseFastMutex(&g_KmdfLayoutLock);

    return TRUE;
}

static BOOLEAN KmdfIsValidOffset(
    LONG Offset,
    ULONG Maximum
)
{
    if (Offset < 0)
        return FALSE;

    return (ULONG)Offset <= Maximum;
}

static BOOLEAN KmdfValidateSymbolLayout(
    PKMDF_SYMBOL_LAYOUT Layout
)
{
    PKMDF_SYMBOL_OFFSETS o;

    if (!Layout)
        return FALSE;

    if (Layout->Version != KMDF_SYMBOL_LAYOUT_VERSION ||
        Layout->Size != sizeof(KMDF_SYMBOL_LAYOUT) ||
        Layout->Source != KMDF_LAYOUT_SOURCE_SYMBOL)
        return FALSE;

    if (Layout->DispatchPolicyKnown > 1)
        return FALSE;
    if (!Layout->DispatchPolicyKnown &&
        Layout->DispatchPolicy != KmdfDispatchPolicyUnknown)
        return FALSE;
    if (Layout->DispatchPolicyKnown &&
        Layout->DispatchPolicy != KmdfDispatchPolicyAllDispatch &&
        Layout->DispatchPolicy != KmdfDispatchPolicyAllDispatchWithLock &&
        Layout->DispatchPolicy != KmdfDispatchPolicyMixed)
        return FALSE;

    o = &Layout->Offsets;

    if (!KmdfIsValidOffset(o->FxContextHeader_Object, 0x100) ||
        !KmdfIsValidOffset(o->FxContextHeader_Context, 0x100))
        return FALSE;

    if (!KmdfIsValidOffset(o->FxDevice_Driver, 0x1000) ||
        !KmdfIsValidOffset(o->FxDevice_DeviceObject, 0x1000) ||
        !KmdfIsValidOffset(o->MxDeviceObject_DeviceObject, 0x100) ||
        !KmdfIsValidOffset(o->FxDevice_PkgPnp, 0x1000))
        return FALSE;

    if (!KmdfIsValidOffset(o->FxDriver_DriverDeviceAdd, 0x1000) ||
        !KmdfIsValidOffset(o->FxDriverDeviceAdd_Method, 0x100))
        return FALSE;

    // Optional identity offsets must be supplied as a complete nested pair.
    if ((o->FxDriver_DriverObject >= 0) !=
        (o->MxDriverObject_DriverObject >= 0))
        return FALSE;
    if (o->FxDriver_DriverObject >= 0 &&
        (!KmdfIsValidOffset(o->FxDriver_DriverObject, 0x1000) ||
         !KmdfIsValidOffset(o->MxDriverObject_DriverObject, 0x1000)))
        return FALSE;
    if (o->FxDriver_Config >= 0 &&
        !KmdfIsValidOffset(o->FxDriver_Config, 0x1000))
        return FALSE;

    return TRUE;
}

static PVOID KmdfJoinAddress(ULONG Low, ULONG High)
{
    return (PVOID)(((ULONG64)High << 32) | Low);
}

static NTSTATUS KmdfReadExact(PVOID Address, PVOID Buffer, SIZE_T Size)
{
    SIZE_T transferred = 0;
    NTSTATUS status;

    if (!KmdfIsKernelPointer(Address) || !Buffer || !Size || Size > MAXULONG)
        return STATUS_INVALID_ADDRESS;
    status = VxkCopyMemory(Buffer, Address, Size, &transferred);
    if (!NT_SUCCESS(status) || transferred != Size)
        return NT_SUCCESS(status) ? STATUS_PARTIAL_COPY : status;
    return STATUS_SUCCESS;
}

NTSTATUS QueryKmdfDriverInfo(PDRIVER_OBJECT DriverObject, PKMDF_DRIVER_INFO Info)
{
    NTSTATUS status;
    KMDF_SYMBOL_LAYOUT layout;
    PKMDF_SYMBOL_OFFSETS o;
    PVOID getFxDriver, extension, fxDriver = NULL, storedDriverObject = NULL;
    WDF_DRIVER_CONFIG config;
    ULONG minimumConfigSize = FIELD_OFFSET(WDF_DRIVER_CONFIG, DriverInitFlags) + sizeof(ULONG);
    ULONG64 dispatchAddress, dispatchWithLockAddress;

    if (!DriverObject || !Info)
        return STATUS_INVALID_PARAMETER;
    RtlZeroMemory(Info, sizeof(*Info));
    Info->DispatchPolicy = KmdfDispatchPolicyUnknown;
    DbgPrint("[KMDF] Query driver=%p\n", DriverObject);
    if (!KmdfIsKernelPointer(DriverObject))
        return STATUS_INVALID_ADDRESS;
    if (!KmdfGetLayoutSnapshot(&layout))
        return STATUS_DEVICE_NOT_READY;

    o = &layout.Offsets;
    if (o->FxDriver_DriverObject < 0 || o->MxDriverObject_DriverObject < 0 ||
        o->FxDriver_Config < 0)
    {
        DbgPrint("[KMDF] identity offsets unavailable\n");
        return STATUS_NOT_SUPPORTED;
    }
    getFxDriver = KmdfJoinAddress(o->FxDriver_GetFxDriver_AddressLow,
        o->FxDriver_GetFxDriver_AddressHigh);
    if (!KmdfIsKernelPointer(getFxDriver))
    {
        DbgPrint("[KMDF] FxDriver::GetFxDriver symbol unavailable\n");
        return STATUS_NOT_SUPPORTED;
    }

    __try
    {
        extension = IoGetDriverObjectExtension(DriverObject, getFxDriver);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
        DbgPrint("[KMDF] DriverObjectExtension lookup exception=0x%08X\n", status);
        return status;
    }
    if (!extension)
    {
        DbgPrint("[KMDF] DriverObjectExtension not found\n");
        return STATUS_NOT_FOUND;
    }
    // The WDF client extension payload is an FxDriver*; copy it safely once.
    status = KmdfReadExact(extension, &fxDriver, sizeof(fxDriver));
    if (!NT_SUCCESS(status) || !KmdfIsKernelPointer(fxDriver))
    {
        DbgPrint("[KMDF] invalid FxDriver extension payload status=0x%08X\n", status);
        return NT_SUCCESS(status) ? STATUS_NOT_FOUND : status;
    }
    Info->FxDriver = (ULONG64)(ULONG_PTR)fxDriver;
    DbgPrint("[KMDF] FxDriver=%p extension=%p\n", fxDriver, extension);

    status = KmdfReadExact((PUCHAR)fxDriver + o->FxDriver_DriverObject +
        o->MxDriverObject_DriverObject, &storedDriverObject, sizeof(storedDriverObject));
    if (!NT_SUCCESS(status) || storedDriverObject != DriverObject)
    {
        DbgPrint("[KMDF] FxDriver back-reference mismatch status=0x%08X value=%p expected=%p\n",
            status, storedDriverObject, DriverObject);
        return NT_SUCCESS(status) ? STATUS_OBJECT_TYPE_MISMATCH : status;
    }

    RtlZeroMemory(&config, sizeof(config));
    status = KmdfReadExact((PUCHAR)fxDriver + o->FxDriver_Config,
        &config, minimumConfigSize);
    if (!NT_SUCCESS(status))
        return status;
    if (config.Size < minimumConfigSize || config.Size > 0x1000)
    {
        DbgPrint("[KMDF] invalid WDF_DRIVER_CONFIG size=%lu minimum=%lu\n",
            config.Size, minimumConfigSize);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    Info->IsKmdf = TRUE;
    Info->DriverInitFlags = config.DriverInitFlags;
    Info->OwnsDispatch =
        (config.DriverInitFlags & WdfDriverInitNoDispatchOverride) ? FALSE : TRUE;
    dispatchAddress = ((ULONG64)o->FxDevice_Dispatch_AddressHigh << 32) |
        o->FxDevice_Dispatch_AddressLow;
    dispatchWithLockAddress = ((ULONG64)o->FxDevice_DispatchWithLock_AddressHigh << 32) |
        o->FxDevice_DispatchWithLock_AddressLow;
    Info->Dispatch = KmdfIsKernelPointer((PVOID)(ULONG_PTR)dispatchAddress)
        ? dispatchAddress : 0;
    Info->DispatchWithLock = KmdfIsKernelPointer((PVOID)(ULONG_PTR)dispatchWithLockAddress)
        ? dispatchWithLockAddress : 0;
    Info->DispatchPolicyKnown = layout.DispatchPolicyKnown ? TRUE : FALSE;
    Info->DispatchPolicy = Info->DispatchPolicyKnown
        ? layout.DispatchPolicy : KmdfDispatchPolicyUnknown;

    DbgPrint("[KMDF] DriverInitFlags=0x%08X NonPnp=%u NoDispatchOverride=%u\n",
        Info->DriverInitFlags,
        !!(Info->DriverInitFlags & WdfDriverInitNonPnpDriver),
        !!(Info->DriverInitFlags & WdfDriverInitNoDispatchOverride));
    DbgPrint("[KMDF] Dispatch=%p DispatchWithLock=%p OwnsDispatch=%u Policy=%u\n",
        (PVOID)(ULONG_PTR)dispatchAddress, (PVOID)(ULONG_PTR)dispatchWithLockAddress,
        Info->OwnsDispatch, Info->DispatchPolicy);
    if (Info->OwnsDispatch && Info->DispatchPolicy == KmdfDispatchPolicyUnknown)
        DbgPrint("[KMDF] dispatch policy unknown, recovery skipped\n");
    DbgPrint("[KMDF] CanRecoverKmdfDispatch=%u\n", CanRecoverKmdfDispatch(Info));
    return STATUS_SUCCESS;
}

BOOLEAN CanRecoverKmdfDispatch(const KMDF_DRIVER_INFO* Info)
{
    if (!Info || !Info->IsKmdf || !Info->OwnsDispatch ||
        !Info->DispatchPolicyKnown)
        return FALSE;

    return Info->DispatchPolicy == KmdfDispatchPolicyAllDispatch ||
        Info->DispatchPolicy == KmdfDispatchPolicyAllDispatchWithLock;
}

NTSTATUS KmdfSetSymbolLayout(
    PKMDF_SYMBOL_LAYOUT Layout
)
{
    if (!KmdfValidateSymbolLayout(Layout))
        return STATUS_INVALID_PARAMETER;

    DbgPrint(
        "[KMDF] SetLayout Ver=%lu Size=%lu Source=%lu PolicyKnown=%lu Policy=%lu\n",
        Layout->Version,
        Layout->Size,
        Layout->Source,
        Layout->DispatchPolicyKnown,
        Layout->DispatchPolicy);

    DbgPrint(
        "[KMDF] Core: Context=%X Object=%X Driver=%X DevObj=%X PkgPnp=%X PkgGeneral=%X\n",
        Layout->Offsets.FxContextHeader_Context,
        Layout->Offsets.FxContextHeader_Object,
        Layout->Offsets.FxDevice_Driver,
        Layout->Offsets.FxDevice_DeviceObject,
        Layout->Offsets.FxDevice_PkgPnp,
        Layout->Offsets.FxDevice_PkgGeneral);

    DbgPrint(
        "[KMDF] DriverAdd: wrapper=%X method=%X\n",
        Layout->Offsets.FxDriver_DriverDeviceAdd,
        Layout->Offsets.FxDriverDeviceAdd_Method);
    DbgPrint("[KMDF] DriverInfo offsets FxDriver.m_DriverObject=%X MxDriverObject.m_DriverObject=%X m_Config=%X\n",
        Layout->Offsets.FxDriver_DriverObject,
        Layout->Offsets.MxDriverObject_DriverObject,
        Layout->Offsets.FxDriver_Config);
    DbgPrint("[KMDF] Symbols GetFxDriver=%p Dispatch=%p DispatchWithLock=%p\n",
        KmdfJoinAddress(Layout->Offsets.FxDriver_GetFxDriver_AddressLow,
            Layout->Offsets.FxDriver_GetFxDriver_AddressHigh),
        KmdfJoinAddress(Layout->Offsets.FxDevice_Dispatch_AddressLow,
            Layout->Offsets.FxDevice_Dispatch_AddressHigh),
        KmdfJoinAddress(Layout->Offsets.FxDevice_DispatchWithLock_AddressLow,
            Layout->Offsets.FxDevice_DispatchWithLock_AddressHigh));


    DbgPrint(
        "[KMDF] D0Entry: wrapper=%X method=%X\n",
        Layout->Offsets.FxPkgPnp_DeviceD0Entry,
        Layout->Offsets.FxPnpDeviceD0Entry_Method);

    DbgPrint(
        "[KMDF] File: Head=%X ListEntry=%X Create=%X/%X Cleanup=%X/%X Close=%X/%X ClassExt=%X\n",
        Layout->Offsets.FxPkgGeneral_FileObjectInfoHeadList,
        Layout->Offsets.FxFileObjectInfo_ListEntry,
        Layout->Offsets.FxFileObjectInfo_EvtFileCreate,
        Layout->Offsets.FxFileObjectFileCreate_Method,
        Layout->Offsets.FxFileObjectInfo_EvtFileCleanup,
        Layout->Offsets.FxFileObjectFileCleanup_Method,
        Layout->Offsets.FxFileObjectInfo_EvtFileClose,
        Layout->Offsets.FxFileObjectFileClose_Method,
        Layout->Offsets.FxFileObjectInfo_ClassExtension);

    ExAcquireFastMutex(&g_KmdfLayoutLock);

    RtlCopyMemory(
        &g_KmdfLayout,
        Layout,
        sizeof(g_KmdfLayout));

    g_KmdfLayoutInitialized = TRUE;

    ExReleaseFastMutex(&g_KmdfLayoutLock);

    return STATUS_SUCCESS;
}

#define KMDF_MAX_FILE_OBJECT_INFO 64

static VOID KmdfEnumFileObjectCallbacks(
    PKMDF_CALLBACK_ENUM_RESULT Result,
    PVOID PkgGeneral,
    const KMDF_SYMBOL_OFFSETS* Offsets
)
{
    if (!Result || !PkgGeneral || !Offsets)
        return;

    if (Offsets->FxPkgGeneral_FileObjectInfoHeadList < 0 ||
        Offsets->FxFileObjectInfo_ListEntry < 0)
        return;

    __try
    {
        PLIST_ENTRY head =
            (PLIST_ENTRY)(
                (PUCHAR)PkgGeneral +
                Offsets->FxPkgGeneral_FileObjectInfoHeadList);

        PLIST_ENTRY entry = head->Flink;

        ULONG count = 0;

        while (entry != head &&
            entry != NULL &&
            count < KMDF_MAX_FILE_OBJECT_INFO)
        {
            if (!KmdfIsKernelPointer(entry))
                break;

            //
            // 先保存 next，避免后续解析期间重复解引用当前 entry
            //
            PLIST_ENTRY next = entry->Flink;

            PVOID fileObjectInfo =
                (PVOID)(
                    (PUCHAR)entry -
                    Offsets->FxFileObjectInfo_ListEntry);

            if (!KmdfIsKernelPointer(fileObjectInfo))
                break;

            //
            // 当前阶段只枚举 client FileObject callbacks，
            // 暂时跳过 Class Extension 的 FxFileObjectInfo。
            //
            BOOLEAN isClassExtension = FALSE;

            if (Offsets->FxFileObjectInfo_ClassExtension >= 0)
            {
                isClassExtension =
                    *(PUCHAR)(
                        (PUCHAR)fileObjectInfo +
                        Offsets->FxFileObjectInfo_ClassExtension)
                    ? TRUE : FALSE;
            }

            if (!isClassExtension)
            {
                KmdfAddCallback(
                    Result,
                    KmdfEvtDeviceFileCreate,
                    KmdfContainerFxFileObjectInfo,
                    fileObjectInfo,
                    Offsets->FxFileObjectInfo_EvtFileCreate,
                    Offsets->FxFileObjectFileCreate_Method);

                KmdfAddCallback(
                    Result,
                    KmdfEvtFileCleanup,
                    KmdfContainerFxFileObjectInfo,
                    fileObjectInfo,
                    Offsets->FxFileObjectInfo_EvtFileCleanup,
                    Offsets->FxFileObjectFileCleanup_Method);

                KmdfAddCallback(
                    Result,
                    KmdfEvtFileClose,
                    KmdfContainerFxFileObjectInfo,
                    fileObjectInfo,
                    Offsets->FxFileObjectInfo_EvtFileClose,
                    Offsets->FxFileObjectFileClose_Method);
            }

            entry = next;
            count++;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        //
        // Current callback 枚举不应因为一个损坏/释放中的内部链表
        // 直接拖垮整个 IOCTL。
        //
        return;
    }
}

NTSTATUS EnumKmdfCurrentCallbacks(
    PDEVICE_OBJECT DeviceObject,
    PKMDF_CALLBACK_ENUM_RESULT Result
)
{
    NTSTATUS status;
    PVOID fxDevice = NULL;
    PVOID fxDriver = NULL;
    PVOID pkgPnp = NULL;
    PVOID pkgGeneral = NULL;
    PVOID selfManagedIo = NULL;

    DbgPrint(
        "[KMDF] ABI sizeof offsets=%Iu layout=%Iu PkgPnp=0x%08lX PkgGeneral=0x%08lX DriverAdd=0x%08lX\n",
        sizeof(KMDF_SYMBOL_OFFSETS),
        sizeof(KMDF_SYMBOL_LAYOUT),
        (ULONG)FIELD_OFFSET(KMDF_SYMBOL_OFFSETS, FxDevice_PkgPnp),
        (ULONG)FIELD_OFFSET(KMDF_SYMBOL_OFFSETS, FxDevice_PkgGeneral),
        (ULONG)FIELD_OFFSET(KMDF_SYMBOL_OFFSETS, FxDriver_DriverDeviceAdd));

    if (!DeviceObject || !Result)
        return STATUS_INVALID_PARAMETER;

    RtlZeroMemory(Result, sizeof(*Result));

    KMDF_SYMBOL_LAYOUT layout;

    if (!KmdfGetLayoutSnapshot(&layout))
        return STATUS_DEVICE_NOT_READY;

    DbgPrint(
        "[KMDF] Runtime offsets PkgPnp=0x%X PkgGeneral=0x%X DriverAdd=0x%X\n",
        layout.Offsets.FxDevice_PkgPnp,
        layout.Offsets.FxDevice_PkgGeneral,
        layout.Offsets.FxDriver_DriverDeviceAdd);

    PKMDF_SYMBOL_OFFSETS offsets = &layout.Offsets;

    status = KmdfGetFxDevice(DeviceObject, offsets, &fxDevice);

    if (!NT_SUCCESS(status)) {
        DbgPrint("[KMDF] GetFxDevice failed: status=0x%08X\n", status);
        return status;
    }

    DbgPrint(
        "[KMDF] Enum source=%lu FxDevice=%p\n",
        layout.Source,
        fxDevice);

    __try
    {
        fxDriver =
            *(PVOID*)((PUCHAR)fxDevice +
                offsets->FxDevice_Driver);

        if (!KmdfIsKernelPointer(fxDriver)) {
            DbgPrint("[KMDF] Invalid FxDriver pointer: %p\n", fxDriver);
            return STATUS_NOT_FOUND;
        }

        if (offsets->FxDevice_PkgPnp >= 0)
        {
            pkgPnp =
                *(PVOID*)((PUCHAR)fxDevice +
                    offsets->FxDevice_PkgPnp);

            if (pkgPnp &&
                !KmdfIsKernelPointer(pkgPnp)) {
                DbgPrint("[KMDF] Invalid PkgPnp pointer: %p\n", pkgPnp);
                pkgPnp = NULL;
            }
        }

        if (pkgPnp &&
            offsets->FxPkgPnp_SelfManagedIoMachine >= 0)
        {
            selfManagedIo =
                *(PVOID*)((PUCHAR)pkgPnp +
                    offsets->FxPkgPnp_SelfManagedIoMachine);

            if (selfManagedIo &&
                !KmdfIsKernelPointer(selfManagedIo))
                selfManagedIo = NULL;
        }

        if (offsets->FxDevice_PkgGeneral >= 0)
        {
            pkgGeneral =
                *(PVOID*)((PUCHAR)fxDevice +
                    offsets->FxDevice_PkgGeneral);

            if (pkgGeneral &&
                !KmdfIsKernelPointer(pkgGeneral)) {
                DbgPrint("[KMDF-FILE] Invalid PkgGeneral pointer: %p\n", pkgGeneral);
                pkgGeneral = NULL;
            }

            if (pkgGeneral)
            {
                DbgPrint(
                    "[KMDF-FILE] PkgGeneral=%p HeadOff=%X\n",
                    pkgGeneral,
                    offsets->FxPkgGeneral_FileObjectInfoHeadList);

                PLIST_ENTRY head =
                    (PLIST_ENTRY)((PUCHAR)pkgGeneral +
                        offsets->FxPkgGeneral_FileObjectInfoHeadList);

                DbgPrint(
                    "[KMDF-FILE] Head=%p Flink=%p Blink=%p\n",
                    head,
                    head->Flink,
                    head->Blink);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DbgPrint("[KMDF] Exception reading containers: code=0x%08X\n", GetExceptionCode());
        return GetExceptionCode();
    }

    DbgPrint(
        "[KMDF] Containers: FxDriver=%p FxPkgPnp=%p SelfManaged=%p FxPkgGeneral=%p\n",
        fxDriver,
        pkgPnp,
        selfManagedIo,
        pkgGeneral);

    Result->FxDevice = (ULONG64)fxDevice;
    Result->FxDriver = (ULONG64)fxDriver;
    Result->FxPkgPnp = (ULONG64)pkgPnp;
    Result->FxSelfManagedIoMachine = (ULONG64)selfManagedIo;
    Result->OffsetSource = layout.Source;

#define ADD_DRIVER(id, wrapper, method) \
    KmdfAddCallback(Result, id, KmdfContainerFxDriver, fxDriver, \
        offsets->wrapper, offsets->method)

#define ADD_PNP(id, wrapper, method) \
    KmdfAddCallback(Result, id, KmdfContainerFxPkgPnp, pkgPnp, \
        offsets->wrapper, offsets->method)

#define ADD_SMIO(id, wrapper, method) \
    KmdfAddCallback(Result, id, KmdfContainerFxSelfManagedIoMachine, selfManagedIo, \
        offsets->wrapper, offsets->method)

    ADD_DRIVER(KmdfEvtDriverDeviceAdd,
        FxDriver_DriverDeviceAdd,
        FxDriverDeviceAdd_Method);

    ADD_DRIVER(KmdfEvtDriverUnload,
        FxDriver_DriverUnload,
        FxDriverUnload_Method);

    if (pkgPnp)
    {
        ADD_PNP(KmdfEvtDeviceUsageNotification,
            FxPkgPnp_DeviceUsageNotification,
            FxPnpDeviceUsageNotification_Method);

        ADD_PNP(KmdfEvtDeviceUsageNotificationEx,
            FxPkgPnp_DeviceUsageNotificationEx,
            FxPnpDeviceUsageNotificationEx_Method);

        ADD_PNP(KmdfEvtDeviceRelationsQuery,
            FxPkgPnp_DeviceRelationsQuery,
            FxPnpDeviceRelationsQuery_Method);

        ADD_PNP(KmdfEvtDeviceD0Entry,
            FxPkgPnp_DeviceD0Entry,
            FxPnpDeviceD0Entry_Method);

        ADD_PNP(KmdfEvtDeviceD0EntryPostInterruptsEnabled,
            FxPkgPnp_DeviceD0EntryPostInterruptsEnabled,
            FxPnpDeviceD0EntryPostInterruptsEnabled_Method);

        ADD_PNP(KmdfEvtDeviceD0ExitPreInterruptsDisabled,
            FxPkgPnp_DeviceD0ExitPreInterruptsDisabled,
            FxPnpDeviceD0ExitPreInterruptsDisabled_Method);

        ADD_PNP(KmdfEvtDeviceD0Exit,
            FxPkgPnp_DeviceD0Exit,
            FxPnpDeviceD0Exit_Method);

        ADD_PNP(KmdfEvtDevicePrepareHardware,
            FxPkgPnp_DevicePrepareHardware,
            FxPnpDevicePrepareHardware_Method);

        ADD_PNP(KmdfEvtDeviceReleaseHardware,
            FxPkgPnp_DeviceReleaseHardware,
            FxPnpDeviceReleaseHardware_Method);

        ADD_PNP(KmdfEvtDeviceQueryStop,
            FxPkgPnp_DeviceQueryStop,
            FxPnpDeviceQueryStop_Method);

        ADD_PNP(KmdfEvtDeviceQueryRemove,
            FxPkgPnp_DeviceQueryRemove,
            FxPnpDeviceQueryRemove_Method);

        ADD_PNP(KmdfEvtDeviceSurpriseRemoval,
            FxPkgPnp_DeviceSurpriseRemoval,
            FxPnpDeviceSurpriseRemoval_Method);
    }

    if (selfManagedIo)
    {
        ADD_SMIO(KmdfEvtDeviceSelfManagedIoCleanup,
            FxSelfManagedIoMachine_DeviceSelfManagedIoCleanup,
            FxPnpDeviceSelfManagedIoCleanup_Method);

        ADD_SMIO(KmdfEvtDeviceSelfManagedIoFlush,
            FxSelfManagedIoMachine_DeviceSelfManagedIoFlush,
            FxPnpDeviceSelfManagedIoFlush_Method);

        ADD_SMIO(KmdfEvtDeviceSelfManagedIoInit,
            FxSelfManagedIoMachine_DeviceSelfManagedIoInit,
            FxPnpDeviceSelfManagedIoInit_Method);

        ADD_SMIO(KmdfEvtDeviceSelfManagedIoSuspend,
            FxSelfManagedIoMachine_DeviceSelfManagedIoSuspend,
            FxPnpDeviceSelfManagedIoSuspend_Method);

        ADD_SMIO(KmdfEvtDeviceSelfManagedIoRestart,
            FxSelfManagedIoMachine_DeviceSelfManagedIoRestart,
            FxPnpDeviceSelfManagedIoRestart_Method);
    }

    if (pkgGeneral)
    {
        KmdfEnumFileObjectCallbacks(
            Result,
            pkgGeneral,
            offsets);
    }

#undef ADD_DRIVER
#undef ADD_PNP
#undef ADD_SMIO

    return STATUS_SUCCESS;
}
