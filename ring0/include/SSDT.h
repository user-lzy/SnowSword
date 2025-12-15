#pragma once

#include "OtherFunctions.h"

typedef struct KSERVICE_TABLE_DESCRIPTOR {
    PULONG     ServiceTableBase;       // 服务函数地址表基址（核心）
    PULONG     ServiceCounterTableBase;// 服务调用计数表（通常为NULL）
    ULONG      NumberOfServices;       // 服务总数
    PUCHAR     ParamTableBase;         // 服务参数长度表
} KSERVICE_TABLE_DESCRIPTOR, * PKSERVICE_TABLE_DESCRIPTOR;

typedef struct _KSERVICE_TABLE_DESCRIPTOR_TABLE {
    KSERVICE_TABLE_DESCRIPTOR NativeApiTable;
    KSERVICE_TABLE_DESCRIPTOR Win32kApiTable;
    KSERVICE_TABLE_DESCRIPTOR NotusedTable1;
    KSERVICE_TABLE_DESCRIPTOR NotusedTable2;
} KSERVICE_TABLE_DESCRIPTOR_TABLE, * PKSERVICE_TABLE_DESCRIPTOR_TABLE;

extern PKSERVICE_TABLE_DESCRIPTOR_TABLE KeServiceDescriptorTable;
extern PKSERVICE_TABLE_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow;

static PVOID GetKiSystemServiceRepeat();

PVOID GetKeServiceDescriptorTable();

PVOID GetKeServiceDescriptorTableShadow();

PVOID GetSSDTFuncAddrByIndex(ULONG sysCallNumber);
PVOID GetSSSDTFuncAddrByIndex(ULONG sysCallNumber);