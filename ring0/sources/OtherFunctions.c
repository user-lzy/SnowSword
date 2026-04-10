#pragma once

#include "OtherFunctions.h"
#include "Memory.h"

BOOLEAN MDLReadMemory(PVOID pBaseAddress, PVOID pReadData, SIZE_T readDataSize)
{
    /*PMDL pMdl = NULL;
    PVOID pNewAddress = NULL;
    // 创建 MDL
    pMdl = MmCreateMdl(NULL, pBaseAddress, readDataSize);
    if (NULL == pMdl) return FALSE;
    
    // 更新 MDL 对物理内存的描述
    MmBuildMdlForNonPagedPool(pMdl);
    // 映射到虚拟内存中
    pNewAddress = MmMapLockedPages(pMdl, KernelMode);
    if (NULL == pNewAddress)
    {
        IoFreeMdl(pMdl);
        return FALSE;
    }
    // 写入数据
    RtlCopyMemory(pReadData, pNewAddress, readDataSize);
    // 释放
    MmUnmapLockedPages(pNewAddress, pMdl);
    IoFreeMdl(pMdl);
    return TRUE;*/
    // 1. 验证
    if (!pBaseAddress || !pReadData || readDataSize == 0) {
        return FALSE;
    }

    if (pBaseAddress < MmSystemRangeStart) {
        return FALSE;
    }

    // 2. 直接读取（SEH保护）
    __try {
        //ProbeForRead(pBaseAddress, readDataSize, 1);
        RtlCopyMemory(pReadData, pBaseAddress, readDataSize);
        return TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("DirectReadKernelMemory failed at %p\n", pBaseAddress);
        return FALSE;
    }
}

BOOLEAN PhysicalReadMemory(PVOID pBaseAddress, PVOID pReadData, SIZE_T readDataSize)
{
    PHYSICAL_ADDRESS physAddr;
    PVOID pMappedIoSpace = NULL;

    // 获取物理地址
    physAddr = MmGetPhysicalAddress(pBaseAddress);
    if (!physAddr.QuadPart) {
        //DbgPrint("无法获取0x%p的物理地址", pBaseAddress);
        return FALSE;
    }

    // 映射物理内存
    pMappedIoSpace = MmMapIoSpace(physAddr, readDataSize, MmNonCached);
    if (!pMappedIoSpace) {
        DbgPrint("无法映射0x%p的物理地址", pBaseAddress);
        return FALSE;
    }

    __try {
        RtlCopyMemory(pReadData, pMappedIoSpace, readDataSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        MmUnmapIoSpace(pMappedIoSpace, readDataSize);
        return FALSE;
    }

    MmUnmapIoSpace(pMappedIoSpace, readDataSize);
    return TRUE;
}

// By: LyShark.com
PVOID SearchSpecialCode(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength)
{
    PVOID pDestAddr = NULL;
    PUCHAR pBeginAddr = (PUCHAR)pSearchBeginAddr;
    PUCHAR pEndAddr = pBeginAddr + ulSearchLength - ulSpecialCodeLength;
    PUCHAR i = NULL;
    ULONG j = 0;

    for (i = pBeginAddr; i <= pEndAddr; i++)
    {
        // 遍历特征码
        for (j = 0; j < ulSpecialCodeLength; j++)
        {
            // 判断地址是否有效
            /*if (FALSE == MmIsAddressValid((PVOID)(i + j)))
            {
                DbgPrint("[SearchSpecialCode] 地址无效: %p", i + j);
                break;
            }*/
            // 匹配特征码
            UCHAR bytCode = 0;
            //SIZE_T bytRead = 0;
            //KIRQL oldirql = KeRaiseIrqlToDpcLevel();
            if (!PhysicalReadMemory(i + j, &bytCode, sizeof(UCHAR))) {
                //KeLowerIrql(oldirql);
                break;
            }
            /*for (int k = 0; k < 2; k++) {
                __try {
                    //ProbeForRead(pBaseAddress, readDataSize, 1);
                    if (!PhysicalReadMemory(i + j, &bytCode, sizeof(UCHAR))) continue;
                    break;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    DbgPrint("DirectReadKernelMemory failed at %p\n", i + j);
                }
            }*/
            //KeLowerIrql(oldirql);
            //NTSTATUS status = MmCopyVirtualMemory(PsGetCurrentProcess(), i + j, PsGetCurrentProcess(), &bytCode, sizeof(UCHAR), UserMode, &bytRead);
            /*if (!NT_SUCCESS(status)) {
                DbgPrint("MmCopyVirtualMemory:%X", status);
                break;
            }*/
            //DbgPrint("Addr:0x%p", i + j);
            //DbgPrint("1Current IRQL:%d", KeGetCurrentIrql());
            if (bytCode != pSpecialCode[j])
            //if (bytCode != pSpecialCode[j])
            {
                //DbgPrint("[SearchSpecialCode] 在 %p 不匹配: 0x%02X != 0x%02X",
                //    i + j, bytCode, pSpecialCode[0]);
                break;
            }
            //if (bytCode != pSpecialCode[j]) break;
        }

        // 匹配成功
        if (j >= ulSpecialCodeLength)
        {
            pDestAddr = (PVOID)i;
            break;
        }
    }
    return pDestAddr;
}

PVOID SearchSpecialCode1(PVOID pSearchBeginAddr, ULONG ulSearchLength, PUCHAR pSpecialCode, ULONG ulSpecialCodeLength)
{
    PVOID pDestAddr = NULL;
    PUCHAR pBeginAddr = (PUCHAR)pSearchBeginAddr;
    PUCHAR pEndAddr = pBeginAddr + ulSearchLength - ulSpecialCodeLength;
    PUCHAR i = NULL;
    ULONG j = 0;

    //DbgPrint("[SearchSpecialCode] 开始搜索:");
    //DbgPrint("起始: %p, 结束: %p, 长度: %lu", pBeginAddr, pEndAddr, ulSearchLength);

    for (i = pBeginAddr; i <= pEndAddr; i++)
    {
        // 每16字节打印一次进度
        if (((i - pBeginAddr) % 16) == 0)
        {
            //DbgPrint("[SearchSpecialCode] 检查地址: %p (偏移: 0x%X)", i, (ULONG)(i - pBeginAddr));
        }

        // 遍历特征码
        for (j = 0; j < ulSpecialCodeLength; j++)
        {
            // 判断地址是否有效
            if (FALSE == MmIsAddressValid((PVOID)(i + j)))
            {
                //DbgPrint("[SearchSpecialCode] 地址无效: %p", i + j);
                //break;
            }
            // 匹配特征码
            if (*(PUCHAR)(i + j) != pSpecialCode[j])
            {
                // 可以在这里输出不匹配的字节
                if (j == 0)  // 如果是第一个字节不匹配
                {
                    //DbgPrint("[SearchSpecialCode] 在 %p 不匹配: 0x%02X != 0x%02X",
                    //    i, *(PUCHAR)i, pSpecialCode[0]);
                }
                break;
            }

        }

        // 匹配成功
        if (j >= ulSpecialCodeLength)
        {
            DbgPrint("[SearchSpecialCode] 找到匹配! 地址: %p", i);
            pDestAddr = (PVOID)i;
            break;
        }
    }

    return pDestAddr;
}

PVOID SearchSpecialCodeWithMask(
    PVOID  pSearchBeginAddr,
    ULONG  ulSearchLength,
    PUCHAR pSpecialCode,
    PUCHAR pMask,
    ULONG  ulSpecialCodeLength
)
{
    if (!pSearchBeginAddr || !pSpecialCode || !pMask || ulSpecialCodeLength == 0)
        return NULL;

    PUCHAR pStart = (PUCHAR)pSearchBeginAddr;
    PUCHAR pEnd = pStart + ulSearchLength - ulSpecialCodeLength;

    for (PUCHAR pCurr = pStart; pCurr <= pEnd; pCurr++)
    {
        BOOLEAN bMatch = TRUE;

        for (ULONG j = 0; j < ulSpecialCodeLength; j++)
        {
            UCHAR currByte;

            __try
            {
                currByte = *(pCurr + j);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                bMatch = FALSE;
                break;   // 这里只能跳出内层 j 循环
            }

            if (pMask[j] == 1 && currByte != pSpecialCode[j])
            {
                bMatch = FALSE;
                break;
            }
        }

        if (bMatch)
            return pCurr;
    }

    return NULL;
}

PVOID FindObpInfoMaskToOffset()
{
    UNICODE_STRING ObQueryNameInfoName = RTL_CONSTANT_STRING(L"ObQueryNameInfo");
    PVOID ObQueryNameInfoAddr = MmGetSystemRoutineAddress(&ObQueryNameInfoName);
    if (NULL == ObQueryNameInfoAddr)
    {
        DbgPrint("ObQueryNameInfoAddr is NULL");
        return NULL;
    }
    //DbgPrint("ObQueryNameInfoAddr: %p", ObQueryNameInfoAddr);
    //488d15
    // ---------------------------------------------------
    // LyShark 开始定位特征

    // 设置起始位置
    PUCHAR StartSearchAddress = (PUCHAR)ObQueryNameInfoAddr;

    // 设置搜索长度
    ULONG size = 0x20;

    // 指定特征码
    UCHAR pSpecialCode[256] = { 0 };

    // 指定特征码长度
    ULONG ulSpecialCodeLength = 3;

    pSpecialCode[0] = 0x48;
    pSpecialCode[1] = 0x8d;
    pSpecialCode[2] = 0x15;

    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
    if (NULL == result)
    {
        DbgPrint("ObpInfoMaskToOffsetAddr is NULL");
        return NULL;
    }
    // 计算目标地址
    ULONG offset = *(PULONG)((PUCHAR)result + 3);
    PVOID ObpInfoMaskToOffsetAddr = (PVOID)((PUCHAR)result + 7 + offset);

    //DbgPrint("ObpInfoMaskToOffset: 0x%p \n", ObpInfoMaskToOffsetAddr);
    return ObpInfoMaskToOffsetAddr;
}

PVOID FindPsGetNextProcessThread()
{
    UNICODE_STRING PsSuspendProcessName = RTL_CONSTANT_STRING(L"PsSuspendProcess");
    PVOID PsSuspendProcessAddr = MmGetSystemRoutineAddress(&PsSuspendProcessName);
    if (NULL == PsSuspendProcessAddr)
    {
        DbgPrint("PsSuspendProcessAddr is NULL");
        return NULL;
    }
    DbgPrint("PsSuspendProcessAddr: %p", PsSuspendProcessAddr);

    /*
    fffff803`537b1f32 488bce          mov     rcx,rsi
    fffff803`537b1f35 e856d2cfff      call    nt!PsGetNextProcessThread (fffff803`534af190)
    */

    UCHAR pSpecialCode[] = { 0x48,0x8b,0xce,0xe8 };

    // 开始搜索,找到后返回首地址
    PVOID result = SearchSpecialCode(PsSuspendProcessAddr, 0x100, pSpecialCode, 4);
    if (NULL == result)
    {
        DbgPrint("搜索失败!");
        return NULL;
    }
    // 计算目标地址
    LONG offset = *(PLONG)((PUCHAR)result + 4);
    DbgPrint("offset:%X", offset);
    PVOID PsGetNextProcessThreadAddr = (PVOID)((PUCHAR)result + 8 + offset);

    //DbgPrint("PsGetNextProcessThread: 0x%p \n", PsGetNextProcessThreadAddr);
    return PsGetNextProcessThreadAddr;
}
