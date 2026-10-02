#pragma once
#include <ntifs.h>

#define KMDF_MAX_CALLBACKS 128
#define KMDF_OFFSET_INVALID ((LONG)-1)

typedef enum _KMDF_CALLBACK_ID
{
    KmdfEvtDriverDeviceAdd = 1,
    KmdfEvtDriverUnload,

    KmdfEvtDeviceUsageNotification,
    KmdfEvtDeviceUsageNotificationEx,
    KmdfEvtDeviceRelationsQuery,

    KmdfEvtDeviceD0Entry,
    KmdfEvtDeviceD0EntryPostInterruptsEnabled,
    KmdfEvtDeviceD0ExitPreInterruptsDisabled,
    KmdfEvtDeviceD0Exit,

    KmdfEvtDevicePrepareHardware,
    KmdfEvtDeviceReleaseHardware,
    KmdfEvtDeviceQueryStop,
    KmdfEvtDeviceQueryRemove,
    KmdfEvtDeviceSurpriseRemoval,

    KmdfEvtDeviceSelfManagedIoCleanup,
    KmdfEvtDeviceSelfManagedIoFlush,
    KmdfEvtDeviceSelfManagedIoInit,
    KmdfEvtDeviceSelfManagedIoSuspend,
    KmdfEvtDeviceSelfManagedIoRestart,

    KmdfEvtDeviceFileCreate,
    KmdfEvtFileCleanup,
    KmdfEvtFileClose
} KMDF_CALLBACK_ID;

typedef enum _KMDF_CALLBACK_CONTAINER
{
    KmdfContainerFxDriver = 1,
    KmdfContainerFxPkgPnp,
    KmdfContainerFxSelfManagedIoMachine,
    KmdfContainerFxFileObjectInfo
} KMDF_CALLBACK_CONTAINER;

#define KMDF_OFFSET_SOURCE_SYMBOL   0x00000001
#define KMDF_OFFSET_SOURCE_FALLBACK 0x00000002

#define KMDF_SYMBOL_LAYOUT_VERSION 4

#define KMDF_LAYOUT_SOURCE_NONE     0
#define KMDF_LAYOUT_SOURCE_SYMBOL   1
#define KMDF_LAYOUT_SOURCE_FALLBACK 2

typedef struct _KMDF_SYMBOL_OFFSETS
{
    LONG FxContextHeader_Object;
    LONG FxContextHeader_Context;

    LONG FxDevice_Driver;
    LONG FxDevice_DeviceObject;
    LONG MxDeviceObject_DeviceObject;
    LONG FxDevice_PkgPnp;
    LONG FxDevice_PkgGeneral;

    LONG FxPkgGeneral_FileObjectInfoHeadList;

    LONG FxFileObjectInfo_ListEntry;
    LONG FxFileObjectInfo_EvtFileCreate;
    LONG FxFileObjectInfo_EvtFileCleanup;
    LONG FxFileObjectInfo_EvtFileClose;
    LONG FxFileObjectInfo_ClassExtension;

    LONG FxFileObjectFileCreate_Method;
    LONG FxFileObjectFileCleanup_Method;
    LONG FxFileObjectFileClose_Method;

    LONG FxDriver_DriverDeviceAdd;
    LONG FxDriver_DriverUnload;
    LONG FxDriverDeviceAdd_Method;
    LONG FxDriverUnload_Method;

    LONG FxPkgPnp_SelfManagedIoMachine;

    LONG FxPkgPnp_DeviceUsageNotification;
    LONG FxPkgPnp_DeviceUsageNotificationEx;
    LONG FxPkgPnp_DeviceRelationsQuery;
    LONG FxPkgPnp_DeviceD0Entry;
    LONG FxPkgPnp_DeviceD0EntryPostInterruptsEnabled;
    LONG FxPkgPnp_DeviceD0ExitPreInterruptsDisabled;
    LONG FxPkgPnp_DeviceD0Exit;
    LONG FxPkgPnp_DevicePrepareHardware;
    LONG FxPkgPnp_DeviceReleaseHardware;
    LONG FxPkgPnp_DeviceQueryStop;
    LONG FxPkgPnp_DeviceQueryRemove;
    LONG FxPkgPnp_DeviceSurpriseRemoval;

    LONG FxPnpDeviceUsageNotification_Method;
    LONG FxPnpDeviceUsageNotificationEx_Method;
    LONG FxPnpDeviceRelationsQuery_Method;
    LONG FxPnpDeviceD0Entry_Method;
    LONG FxPnpDeviceD0EntryPostInterruptsEnabled_Method;
    LONG FxPnpDeviceD0ExitPreInterruptsDisabled_Method;
    LONG FxPnpDeviceD0Exit_Method;
    LONG FxPnpDevicePrepareHardware_Method;
    LONG FxPnpDeviceReleaseHardware_Method;
    LONG FxPnpDeviceQueryStop_Method;
    LONG FxPnpDeviceQueryRemove_Method;
    LONG FxPnpDeviceSurpriseRemoval_Method;

    LONG FxSelfManagedIoMachine_DeviceSelfManagedIoCleanup;
    LONG FxSelfManagedIoMachine_DeviceSelfManagedIoFlush;
    LONG FxSelfManagedIoMachine_DeviceSelfManagedIoInit;
    LONG FxSelfManagedIoMachine_DeviceSelfManagedIoSuspend;
    LONG FxSelfManagedIoMachine_DeviceSelfManagedIoRestart;

    LONG FxPnpDeviceSelfManagedIoCleanup_Method;
    LONG FxPnpDeviceSelfManagedIoFlush_Method;
    LONG FxPnpDeviceSelfManagedIoInit_Method;
    LONG FxPnpDeviceSelfManagedIoSuspend_Method;
    LONG FxPnpDeviceSelfManagedIoRestart_Method;

    // Optional identity/dispatch offsets. Values come from Wdf01000 PDB.
    LONG FxDriver_DriverObject;
    LONG MxDriverObject_DriverObject;
    LONG FxDriver_Config;
    ULONG FxDriver_GetFxDriver_AddressLow;
    ULONG FxDriver_GetFxDriver_AddressHigh;
    ULONG FxDevice_Dispatch_AddressLow;
    ULONG FxDevice_Dispatch_AddressHigh;
    ULONG FxDevice_DispatchWithLock_AddressLow;
    ULONG FxDevice_DispatchWithLock_AddressHigh;
} KMDF_SYMBOL_OFFSETS, * PKMDF_SYMBOL_OFFSETS;

typedef struct _KMDF_SYMBOL_LAYOUT
{
    ULONG Version;
    ULONG Size;
    ULONG Source;
    ULONG Reserved;
    ULONG DispatchPolicyKnown;
    ULONG DispatchPolicy;

    KMDF_SYMBOL_OFFSETS Offsets;

} KMDF_SYMBOL_LAYOUT, * PKMDF_SYMBOL_LAYOUT;

C_ASSERT(sizeof(KMDF_SYMBOL_OFFSETS) == 256);
C_ASSERT(sizeof(KMDF_SYMBOL_LAYOUT) == 280);
C_ASSERT(FIELD_OFFSET(KMDF_SYMBOL_LAYOUT, Offsets) == 24);

typedef struct _KMDF_CALLBACK_INFO
{
    ULONG Id;
    ULONG ContainerType;
    ULONG64 Address;
    ULONG64 ContainerObject;
} KMDF_CALLBACK_INFO, * PKMDF_CALLBACK_INFO;

typedef struct _KMDF_CALLBACK_ENUM_REQUEST
{
    ULONG64 DeviceObject;
} KMDF_CALLBACK_ENUM_REQUEST, * PKMDF_CALLBACK_ENUM_REQUEST;

typedef struct _KMDF_CALLBACK_ENUM_RESULT
{
    ULONG Count;
    ULONG OffsetSource;
    ULONG64 FxDevice;
    ULONG64 FxDriver;
    ULONG64 FxPkgPnp;
    ULONG64 FxSelfManagedIoMachine;
    KMDF_CALLBACK_INFO Callbacks[KMDF_MAX_CALLBACKS];
} KMDF_CALLBACK_ENUM_RESULT, * PKMDF_CALLBACK_ENUM_RESULT;

typedef enum _KMDF_DISPATCH_POLICY
{
    KmdfDispatchPolicyUnknown = 0,
    KmdfDispatchPolicyAllDispatch,
    KmdfDispatchPolicyAllDispatchWithLock,
    KmdfDispatchPolicyMixed
} KMDF_DISPATCH_POLICY;

typedef struct _KMDF_DRIVER_INFO_REQUEST
{
    ULONG64 DriverObject;
} KMDF_DRIVER_INFO_REQUEST, * PKMDF_DRIVER_INFO_REQUEST;

typedef struct _KMDF_DRIVER_INFO
{
    BOOLEAN IsKmdf;
    BOOLEAN OwnsDispatch;
    BOOLEAN DispatchPolicyKnown;
    UCHAR Reserved;
    ULONG DriverInitFlags;
    ULONG DispatchPolicy;
    ULONG Reserved2;
    ULONG64 FxDriver;
    ULONG64 Dispatch;
    ULONG64 DispatchWithLock;
} KMDF_DRIVER_INFO, * PKMDF_DRIVER_INFO;

C_ASSERT(sizeof(KMDF_DRIVER_INFO) == 40);

NTSTATUS InitializeKmdfCallbackSupport();
NTSTATUS KmdfSetSymbolLayout(
    PKMDF_SYMBOL_LAYOUT Layout
);
NTSTATUS EnumKmdfCurrentCallbacks(
    PDEVICE_OBJECT DeviceObject,
    PKMDF_CALLBACK_ENUM_RESULT Result
);
BOOLEAN CanRecoverKmdfDispatch(const KMDF_DRIVER_INFO* Info);
NTSTATUS QueryKmdfDriverInfo(
    PDRIVER_OBJECT DriverObject,
    PKMDF_DRIVER_INFO Info
);
