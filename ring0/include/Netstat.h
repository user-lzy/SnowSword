#pragma once

#include "global.h"
// NDIS过滤驱动 信息结构体（对外输出，存储枚举结果）
typedef struct _NDIS_FILTER_INFO {
    UNICODE_STRING  FilterName;       // 过滤驱动名称
    GUID            FilterGuid;       // 过滤驱动唯一GUID
    PVOID           pFilterBlock;     // NDIS内部过滤控制块地址
    PDRIVER_OBJECT  pDriverObject;    // 驱动对象
    ULONG           FilterVersion;    // 驱动版本
    BOOLEAN         bValid;           // 节点有效性标记
} NDIS_FILTER_INFO, * PNDIS_FILTER_INFO;

NTSTATUS
EnumNdisFilterDrivers(
    OUT PNDIS_FILTER_INFO* ppFilterArray,
    OUT PULONG              pulFilterCount
);
VOID
FreeNdisFilterArray(PNDIS_FILTER_INFO pFilterArray);
