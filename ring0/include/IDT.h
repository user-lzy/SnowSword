#pragma once

#include "global.h"

#pragma pack(push, 1)
typedef struct _IDTR {
    USHORT Limit;     // IDT 表大小（字节数）
    ULONG64 Base;     // IDT 基地址（64 位）
} IDTR, * PIDTR;

typedef struct _IDT_ENTRY {
    USHORT OffsetLow;   // 中断处理函数地址低 16 位 
    USHORT Selector;    // 段选择子 
    USHORT Attributes;  // 属性（门类型、DPL 等）
    USHORT OffsetMid;   // 中断处理函数地址中间 16 位 
    ULONG OffsetHigh;   // 中断处理函数地址高 32 位 
    ULONG Reserved;     // 保留字段 
} IDT_ENTRY, * PIDT_ENTRY;
#pragma pack(pop)

typedef struct _IDT_ITEM {
    PVOID HandlerFunc;
    USHORT Selector;
    USHORT Attributes;
}IDT_ITEM, * PIDT_ITEM;

typedef struct _IDT_INFO {
	IDT_ITEM IdtEntries[256];
}IDT_INFO, * PIDT_INFO;

// 获取当前 IDT 表的地址
ULONG GetIDT(PIDT_INFO Array);