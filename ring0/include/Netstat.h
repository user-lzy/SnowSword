#pragma once

#include "global.h"

// ----------------------------------------------------------------------------
// 调试前缀
// ----------------------------------------------------------------------------
#define NDIS_ENUM_TAG   'mNdE'
#define DBG_PREFIX      "[NdisEnum] "

// ----------------------------------------------------------------------------
// 枚举结果结构体（每个 Miniport 一个条目）
// ----------------------------------------------------------------------------
#define NDIS_ENUM_MAX_FILTERS_PER_MINIPORT  8
#define NDIS_ENUM_MAX_OPENS_PER_MINIPORT    16
#define NDIS_ENUM_MAX_STRING_LEN            128

#pragma pack(push, 1)
typedef struct _NDIS_ENUM_FILTER_ENTRY {
    ULONG64     FilterBlock;
    ULONG64     DriverObject;
    WCHAR       InstanceName[NDIS_ENUM_MAX_STRING_LEN];
    WCHAR       FriendlyName[NDIS_ENUM_MAX_STRING_LEN];
    WCHAR       DriverImageName[NDIS_ENUM_MAX_STRING_LEN];
} NDIS_ENUM_FILTER_ENTRY, * PNDIS_ENUM_FILTER_ENTRY;

typedef struct _NDIS_ENUM_OPEN_ENTRY {
    ULONG64     OpenBlock;
    ULONG64     ProtocolBlock;
    WCHAR       ProtocolName[NDIS_ENUM_MAX_STRING_LEN];
    WCHAR       ProtocolImageName[NDIS_ENUM_MAX_STRING_LEN];
} NDIS_ENUM_OPEN_ENTRY, * PNDIS_ENUM_OPEN_ENTRY;

typedef struct _NDIS_MINIPORT_ENUM_ENTRY {
    ULONG64     MiniportBlock;
    //ULONG64     DeviceObject;
	WCHAR       DriverPath[260];
    WCHAR       pAdapterInstanceName[NDIS_ENUM_MAX_STRING_LEN];
    WCHAR       BaseName[NDIS_ENUM_MAX_STRING_LEN];

    ULONG       FilterCount;
    NDIS_ENUM_FILTER_ENTRY Filters[NDIS_ENUM_MAX_FILTERS_PER_MINIPORT];

    ULONG       OpenCount;
    NDIS_ENUM_OPEN_ENTRY   Opens[NDIS_ENUM_MAX_OPENS_PER_MINIPORT];
} NDIS_MINIPORT_ENUM_ENTRY, * PNDIS_MINIPORT_ENUM_ENTRY;
#pragma pack(pop)

NTSTATUS
NTAPI
NdisEnumMiniports(
    _Out_writes_bytes_opt_(EntrySize* MaxEntries) PNDIS_MINIPORT_ENUM_ENTRY Entries,
    _In_ ULONG EntrySize,
    _In_ ULONG MaxEntries,
    _Out_ PULONG NeededEntries,
    _Out_ PULONG ActualEntries
);

NTSTATUS
NTAPI
NdisEnumMiniportsAlloc(
    _Out_ PNDIS_MINIPORT_ENUM_ENTRY* ppEntries,
    _Out_ PULONG EntryCount
);

NTSTATUS
NTAPI
GetNdisListAndLockAddresses(
    _Out_ PULONG64 ProtocolListLock,
    _Out_ PULONG64 ProtocolList,
    _Out_ PULONG64 MiniportListLock,
    _Out_ PULONG64 MiniportList
);
