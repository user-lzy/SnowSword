#pragma once

#include "IDT.h"

// 获取当前 IDT 表
ULONG GetIDT(PIDT_INFO Array) {
    // 获取 CPU 核心数
    int i_cpuNum = KeNumberProcessors;
    //DbgPrint("CPU核心数: %d \n", i_cpuNum);

    for (KAFFINITY i = 0; i < i_cpuNum; i++)
    {
        IDTR idtr = { 0 };
        PIDT_ENTRY idtEntries;
        //DbgPrint("CPU核心: %Iu \n", i);
        // 线程绑定特定 CPU
        KeSetSystemAffinityThread(i + 1);
        // 使用汇编指令 SIDT 获取 IDTR 
        __sidt(&idtr);
        // 取消绑定 CPU
        KeRevertToUserAffinityThread();

        idtEntries = (PIDT_ENTRY)idtr.Base;

        // 计算表项数量（每个表项 16 字节）
        ULONG entryCount = (idtr.Limit + 1) / sizeof(IDT_ENTRY);

        //DbgPrint("IDT Base: 0x%llX, Entry Count: %d\n", idtr.Base, entryCount);

        _try{
            // 枚举 IDT 表项
            for (ULONG j = 0; j < entryCount; j++) {
                PIDT_ENTRY entry = &idtEntries[j];
                ULONG64 offset = (ULONG64)entry->OffsetHigh << 32 |
                    (ULONG64)entry->OffsetMid << 16 |
                    entry->OffsetLow;
				Array[i].IdtEntries[j].HandlerFunc = (PVOID)offset;
				Array[i].IdtEntries[j].Selector = entry->Selector;
				Array[i].IdtEntries[j].Attributes = entry->Attributes;
                //DbgPrint("[INT %03d] Handler: 0x%llX, Selector: 0x%04X, Attr: 0x%04X, Name: %s\n",
                //    i, offset, entry->Selector, entry->Attributes, GetInterruptName(i));
            }
        }
        _except(EXCEPTION_EXECUTE_HANDLER)
        {
            //DbgPrint("EnumGDT except.status: %X", GetExceptionCode());
        }
    }
    return i_cpuNum;
}