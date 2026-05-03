#pragma once

#include "global.h"

// 查询类型枚举
typedef enum _QUERY_TYPE {
    QuerySymbolToAddress,    // 符号名 → 地址
    QueryAddressToSymbol,    // 地址 → 符号名
    QueryStructOffset        // 结构体 → 成员偏移
} QUERY_TYPE;

// 符号查询请求结构
typedef struct _SYMBOL_QUERY_REQUEST {
    WCHAR ModuleName[256];       // 新增：要查询的模块名称（如 win32kfull.sys）
    QUERY_TYPE Type;
    union {
        struct {
            WCHAR SymbolName[256];  // 输入: "nt!SwapContext"
        } SymbolToAddr;

        struct {
            ULONG64 Address;         // 输入: 内核地址
        } AddrToSymbol;

        struct {
            WCHAR StructName[64];    // 输入: "_EPROCESS"
            WCHAR MemberName[64];    // 输入: "ImageFileName"
        } StructOffset;
    } Data;
} SYMBOL_QUERY_REQUEST, * PSYMBOL_QUERY_REQUEST;

// 符号查询响应结构
typedef struct _SYMBOL_QUERY_RESPONSE {
    NTSTATUS Status;
    union {
        ULONG64 Address;             // 输出: 符号地址
        LONG    Offset;              // 输出: 结构体偏移
        WCHAR   SymbolName[256];     // 输出: 符号名
    } Result;
} SYMBOL_QUERY_RESPONSE, * PSYMBOL_QUERY_RESPONSE;

typedef struct _SYMBOL_QUERY_PACKET {
    ULONGLONG RequestId;
    SYMBOL_QUERY_REQUEST Request;
} SYMBOL_QUERY_PACKET, * PSYMBOL_QUERY_PACKET;

typedef struct _SYMBOL_RESPONSE_PACKET {
    ULONGLONG RequestId;
    SYMBOL_QUERY_RESPONSE Response;
} SYMBOL_RESPONSE_PACKET, * PSYMBOL_RESPONSE_PACKET;

typedef struct _SYMBOL_JOB {
    LIST_ENTRY Link;
    ULONGLONG RequestId;
    BOOLEAN Delivered;      // 是否已经发给 R3
    BOOLEAN Completed;      // 是否已经收到响应
    KEVENT DoneEvent;       // R0 等待这个 job 完成
    SYMBOL_QUERY_REQUEST Request;
    SYMBOL_QUERY_RESPONSE Response;
} SYMBOL_JOB, * PSYMBOL_JOB;

typedef struct _DEVICE_CONTEXT {
    PDEVICE_OBJECT DeviceObject;
    FAST_MUTEX Lock;

    LIST_ENTRY JobList;     // 所有未完成 job
    PIRP WaitIrp;           // R3 正在等待请求的 IRP（仅 1 个）
    ULONGLONG NextRequestId;
    BOOLEAN Unloading;
} DEVICE_CONTEXT, * PDEVICE_CONTEXT;

// 内核导出 API (供其他驱动调用)
NTSTATUS KernelQuerySymbolAddress(
    PCWSTR ModulePath,     // 新增：模块路径
    PCWSTR SymbolName,
    PULONG64 Address
);

NTSTATUS KernelQueryStructOffset(
    PCWSTR ModulePath,     // 新增：模块路径
    PCWSTR StructName,
    PCWSTR MemberName,
    PLONG Offset
);

NTSTATUS KernelQueryAddressToSymbol(
    PCWSTR ModulePath,     // 新增：模块路径
    ULONG64 Address,
    PWCHAR SymbolName,
    ULONG NameLength
);

NTSTATUS GetNtSymbolAddress(PCWSTR SymbolName, PULONG64 Address);
NTSTATUS GetNtStructOffset(PCWSTR StructName, PCWSTR MemberName, PLONG Offset);

NTSTATUS InitSymbolContext(PDEVICE_OBJECT deviceObj);
VOID UninitSymbolContext();
NTSTATUS DispatchClose_Symbol(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchIoControl_Symbol(PDEVICE_OBJECT DeviceObject, PIRP Irp);