#pragma once

#include "Memory.h"

NTSTATUS ReadKernelMemory(PVOID pAddress, PVOID pBuffer, ULONG SizeOfRead)
{
    NTSTATUS status = STATUS_SUCCESS;
    PMDL mdl = NULL;
    PVOID addrMm = NULL;
    BOOLEAN bPagesLocked = FALSE;
    BOOLEAN bPagesMapped = FALSE;

    // 参数验证
    if (!pAddress || !pBuffer || SizeOfRead == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    // 1. 分配MDL
    mdl = IoAllocateMdl(pAddress, SizeOfRead, FALSE, FALSE, NULL);
    if (!mdl) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 2. 构建MDL（仅适用于非分页池）
    __try {
        //MmBuildMdlForNonPagedPool(mdl);

        //DbgPrint("mdl->StartVa: %llx, mdl->ByteCount: %d, mdl->MappedSystemVa: %llx, mdl->ByteOffset: %d\n",
        //    (ULONG_PTR)mdl->StartVa, mdl->ByteCount, (ULONG_PTR)mdl->MappedSystemVa, mdl->ByteOffset);

        // 3. 锁定页面
        //MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
        //bPagesLocked = TRUE;

        // 4. 映射到系统空间
        addrMm = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached,
            NULL, FALSE, NormalPagePriority);
        if (!addrMm) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
        bPagesMapped = TRUE;

        // 5. 复制内存
        memcpy(pBuffer, addrMm, SizeOfRead);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        DbgPrint("ReadKernelMemory exception: 0x%08X\n", status);
        goto Exit;
    }

Exit:
    // 6. 资源清理（按相反顺序）
    if (bPagesMapped) {
        MmUnmapLockedPages(addrMm, mdl);
    }

    if (bPagesLocked) {
        //MmUnlockPages(mdl);  // ✅ 关键：解锁你锁定的页
    }

    if (mdl) {
        IoFreeMdl(mdl);  // ✅ 现在这是安全的
    }

    return status;
}

NTSTATUS VxkCopyMemory(PVOID pDestination, PVOID pSourceAddress, SIZE_T SizeOfCopy, PSIZE_T pBytesTransferred)
{
    // 1. 验证源地址在内核空间
    if ((ULONG_PTR)pSourceAddress < (ULONG_PTR)MmSystemRangeStart) {
        DbgPrint("[VxkCopyMemory] 非法地址: %p", pSourceAddress);
        return STATUS_INVALID_ADDRESS;
    }

    // 2. 限制单次读取大小
    if (SizeOfCopy > 4 * 1024 * 1024) { // 4MB限制
        DbgPrint("[VxkCopyMemory] 大小超限: %lld", SizeOfCopy);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    // 3. 构建源地址结构体
    MM_COPY_ADDRESS Source = { 0 };
    Source.VirtualAddress = pSourceAddress; // 联合体赋值

    // 4. 执行安全拷贝
    NTSTATUS status = MmCopyMemory(
        pDestination,           // TargetAddress: 目标缓冲区
        Source,                 // SourceAddress: 源地址结构
        SizeOfCopy,             // NumberOfBytes: 要拷贝的字节数
        MM_COPY_MEMORY_VIRTUAL, // Flags: 虚拟地址模式
        pBytesTransferred       // NumberOfBytesTransferred: 实际拷贝字节数(输出)
    );

    // 5. 检查返回值
    if (!NT_SUCCESS(status)) {
        //DbgPrint("[VxkCopyMemory] MmCopyMemory失败: 0x%08X, 已拷贝: %lld",
        //    status, *pBytesTransferred);
        if (*pBytesTransferred != 0)
            return STATUS_SUCCESS;
        else {
            //DbgPrint("%lld", *pBytesTransferred);
            return status;
        }
    }

    // 6. 验证完整性（可选但推荐）
    if (*pBytesTransferred != SizeOfCopy) {
        DbgPrint("[VxkCopyMemory] 部分拷贝: 期望%lld, 实际%lld",
            SizeOfCopy, *pBytesTransferred);
        return STATUS_PARTIAL_COPY; // 或返回成功，视业务需求
    }

    return STATUS_SUCCESS;
}

//MmCopyMemory
NTSTATUS CopyMemory(HANDLE dwProcessId, PVOID pSourceAddress, PVOID pDestinationAddress, SIZE_T SizeOfCopy, PSIZE_T pBytesTransferred)
{
    if (dwProcessId == NULL) return VxkCopyMemory(pDestinationAddress, pSourceAddress, SizeOfCopy, pBytesTransferred);
    
    NTSTATUS status;
	PEPROCESS pEProcess = NULL;
	status = PsLookupProcessByProcessId(dwProcessId, &pEProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("PsLookupProcessByProcessId failed, status: 0x%X\n", status);
		return status;
	}
    
	status = MmCopyVirtualMemory(pEProcess, pSourceAddress, PsGetCurrentProcess(), pDestinationAddress, SizeOfCopy, KernelMode, pBytesTransferred);
    if (!status) DbgPrint("[CopyMemory]MmCopyVirtualMemory:status=%X", status);
    //KAPC_STATE ApcState = { 0 };
    //KeStackAttachProcess(pEProcess, &ApcState);
    //status = VxkCopyMemory(pDestinationAddress, pSourceAddress, SizeOfCopy) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
    //for (SIZE_T i = 0; i < min(SizeOfCopy, 16); i++) DbgPrint("Byte %llu: 0x%02X\n", i, ((PUCHAR)pDestinationAddress)[i]);
    //KeUnstackDetachProcess(&ApcState);
	ObDereferenceObject(pEProcess);
	return status;
}

NTSTATUS DisableCopyOnWrite(PVOID pMem, SIZE_T ulMemSize)
{
    ULONG64 ulPDEB = 0;
    ULONG64 ulPTEB = 0;
    BOOLEAN bPDEB = FALSE;
    BOOLEAN bPTEB = FALSE;
    ULONG64 ulPDE = 0;
    ULONG64 ulPTE = 0;
    BOOLEAN bPDE = FALSE;
    BOOLEAN bPTE = FALSE;
    BOOLEAN PAE = FALSE;
	//NTSTATUS status;

    PAE = ExIsProcessorFeaturePresent(PF_PAE_ENABLED);
    if (PAE == TRUE)
    {
        DbgPrint("PAE page mode.\n");
        // 按照PAE page mode尝试计算PDE和PTE,并查看虚拟地址是否在同一页面
        //要修改的地址起始处
        ulPDEB = ((((ULONG64)pMem) >> 18) & 0x3FF8) + 0xC0600000;
        ulPTEB = ((((ULONG64)pMem) >> 9) & 0x7FFFF8) + 0xC0000000;
        bPDEB = MmIsAddressValid((PVOID)ulPDEB);
        bPTEB = MmIsAddressValid((PVOID)ulPTEB);
        //要修改的地址后边界
        ulPDE = (((((ULONG64)pMem + ulMemSize)) >> 18) & 0x3FF8) + 0xC0600000;
        ulPTE = (((((ULONG64)pMem + ulMemSize)) >> 9) & 0x7FFFF8) + 0xC0000000;
        bPDE = MmIsAddressValid((PVOID)ulPDE);
        bPTE = MmIsAddressValid((PVOID)ulPTE);
        if ((bPDEB && bPTEB && bPTE))
        {
            DbgPrint("PDE(%d) : 0X%08llX -> 0X%08llX\n", bPDEB, ulPDEB, *(PULONG64)ulPDEB);
            DbgPrint("PTE(%d) : 0X%08llX -> 0X%08llX\n", bPTEB, ulPTEB, ulPTE);
        }
        else
            return STATUS_UNSUCCESSFUL;
    }
    else
    {
        DbgPrint("Non PAE page mode.\n");
        // 按照Non PAE page mode尝试计算PDE和PTE
        ulPDEB = ((((ULONG64)pMem) >> 20) & 0xFFC) + 0xC0300000;  //cr3寄存器起始地址
        ulPTEB = ((((ULONG64)pMem) >> 10) & 0x3FFFFC) + 0xC0000000;
        bPDEB = MmIsAddressValid((PVOID)ulPDEB);
        bPTEB = MmIsAddressValid((PVOID)ulPTEB);

        ulPDE = ((((ULONG64)pMem + ulMemSize) >> 20) & 0xFFC) + 0xC0300000;
        ulPTE = ((((ULONG64)pMem + ulMemSize) >> 10) & 0x3FFFFC) + 0xC0000000;
        bPDE = MmIsAddressValid((PVOID)ulPDE);
        bPTE = MmIsAddressValid((PVOID)ulPTE);


        if ((bPDEB && bPTEB && bPTE))
        {
            DbgPrint("PDE(%d) : 0X%08llX -> 0X%08llX\n", bPDEB, ulPDEB, *(PULONG64)ulPDEB);
            DbgPrint("PTE(%d) : 0X%08llX -> 0X%08llX\n", bPTEB, ulPTEB, *(PULONG64)ulPTEB);
        }
        else
            return STATUS_UNSUCCESSFUL;
    }

    // 禁用指定内存的Copy on write机制
    if (bPTE == bPTEB)//物理页面是否存在有效
    {
        *(PULONG64)ulPTEB |= 0x00000002; //修改PTE使指定页Copy on write机制失效
        DbgPrint("The copy-on-write attrib in address 0X%08llX  has been forbidden!\n", pMem);
    }
    else
    {
        *(PULONG64)ulPTEB |= 0x00000002;
        *(PULONG64)ulPTE |= 0x00000002;
        DbgPrint("The copy-on-write attrib has been forbidden!\n");
        
    }
    return STATUS_SUCCESS;
}

NTSTATUS EnableCopyOnWrite(PVOID pMem, SIZE_T ulMemSize)
{
    ULONG64 ulPDEB = 0;
    ULONG64 ulPTEB = 0;
    BOOLEAN bPDEB = FALSE;
    BOOLEAN bPTEB = FALSE;
    ULONG64 ulPDE = 0;
    ULONG64 ulPTE = 0;
    BOOLEAN bPDE = FALSE;
    BOOLEAN bPTE = FALSE;
    BOOLEAN PAE = FALSE;
	//NTSTATUS status;

    PAE = ExIsProcessorFeaturePresent(PF_PAE_ENABLED);
    if (PAE == TRUE)
    {
        DbgPrint("PAE page mode.\n");
        // 按照PAE page mode尝试计算PDE和PTE,并查看虚拟地址是否在同一页面
        //要修改的地址起始处
        ulPDEB = ((((ULONG64)pMem) >> 18) & 0x3FF8) + 0xC0600000;
        ulPTEB = ((((ULONG64)pMem) >> 9) & 0x7FFFF8) + 0xC0000000;
        bPDEB = MmIsAddressValid((PVOID)ulPDEB);
        bPTEB = MmIsAddressValid((PVOID)ulPTEB);
        //要修改的地址后边界
        ulPDE = (((((ULONG64)pMem + ulMemSize)) >> 18) & 0x3FF8) + 0xC0600000;
        ulPTE = (((((ULONG64)pMem + ulMemSize)) >> 9) & 0x7FFFF8) + 0xC0000000;
        bPDE = MmIsAddressValid((PVOID)ulPDE);
        bPTE = MmIsAddressValid((PVOID)ulPTE);
        if ((bPDEB && bPTEB && bPTE))
        {
            DbgPrint("PDE(%d) : 0X%08llX -> 0X%08X\n", bPDEB, ulPDEB, *(PULONG)ulPDEB);
            DbgPrint("PTE(%d) : 0X%08llX -> 0X%08llX\n", bPTEB, ulPTEB, ulPTE);
        }
        else
            return STATUS_UNSUCCESSFUL;
    }
    else
    {
        DbgPrint("Non PAE page mode.\n");
        // 按照Non PAE page mode尝试计算PDE和PTE
        ulPDEB = ((((ULONG64)pMem) >> 20) & 0xFFC) + 0xC0300000;  //cr3寄存器起始地址
        ulPTEB = ((((ULONG64)pMem) >> 10) & 0x3FFFFC) + 0xC0000000;
        bPDEB = MmIsAddressValid((PVOID)ulPDEB);
        bPTEB = MmIsAddressValid((PVOID)ulPTEB);

        ulPDE = ((((ULONG64)pMem + ulMemSize) >> 20) & 0xFFC) + 0xC0300000;
        ulPTE = ((((ULONG64)pMem + ulMemSize) >> 10) & 0x3FFFFC) + 0xC0000000;
        bPDE = MmIsAddressValid((PVOID)ulPDE);
        bPTE = MmIsAddressValid((PVOID)ulPTE);


        if ((bPDEB && bPTEB && bPTE))
        {
            DbgPrint("PDE(%d) : 0X%08llX -> 0X%08X\n", bPDEB, ulPDEB, *(PULONG)ulPDEB);
            DbgPrint("PTE(%d) : 0X%08llX -> 0X%08X\n", bPTEB, ulPTEB, *(PULONG)ulPTEB);
        }
        else
            return STATUS_UNSUCCESSFUL;
    }
    // 恢复指定内存的Copy on write机制
    if (bPTE == bPTEB)
    {
        *(PULONG)ulPTEB &= ~0x00000002;  //修改PTE恢复指定页的Copy on write机制
    }
    else
    {
        *(PULONG)ulPTEB &= ~0x00000002;
        *(PULONG)ulPTE &= ~0x00000002;
        
    }
    return STATUS_SUCCESS;
}

// 将物理地址映射到虚拟地址
PVOID MapPhysicalAddress(PHYSICAL_ADDRESS PhysicalAddress, SIZE_T Size)
{
    PHYSICAL_ADDRESS LowAddress, HighAddress, SkipBytes;
    LowAddress.QuadPart = PhysicalAddress.QuadPart;
    HighAddress.QuadPart = PhysicalAddress.QuadPart + Size;
    SkipBytes.QuadPart = 0;

    PVOID MappedVirtual = MmMapIoSpace(LowAddress, Size, MmNonCached);
    return MappedVirtual;
}

// 取消映射
VOID UnmapPhysicalAddress(PVOID BaseAddress, SIZE_T Size)
{
    MmUnmapIoSpace(BaseAddress, Size);
}

// 通用方法获取PTE地址
PHARDWARE_PTE GetPteAddressDynamic(PVOID VirtualAddress, PHARDWARE_PTE* OutMappedPage)
{
    ULONG64 VA = (ULONG64)VirtualAddress;
    PHYSICAL_ADDRESS Cr3Pa;
    PHARDWARE_PTE Pml4 = NULL, Pdp = NULL, Pd = NULL, Pt = NULL;
    PHARDWARE_PTE Result = NULL;

    // 1. 获取CR3（页表物理基址）
    Cr3Pa.QuadPart = __readcr3() & ~0xFFFULL;

    // 2. 映射PML4页
    Pml4 = (PHARDWARE_PTE)MapPhysicalAddress(Cr3Pa, PAGE_SIZE);
    if (!Pml4) return NULL;

    __try {
        ULONG Pml4Index = (VA >> 39) & 0x1FF;
        ULONG PdpIndex = (VA >> 30) & 0x1FF;
        ULONG PdIndex = (VA >> 21) & 0x1FF;
        ULONG PtIndex = (VA >> 12) & 0x1FF;

        // 3. 检查并映射PDP页
        if (!Pml4[Pml4Index].u.Bits.Valid) __leave;
        PHYSICAL_ADDRESS PdpPa;
        PdpPa.QuadPart = Pml4[Pml4Index].u.Bits.PageFrameNumber << 12;
        Pdp = (PHARDWARE_PTE)MapPhysicalAddress(PdpPa, PAGE_SIZE);
        if (!Pdp) __leave;

        // 4. 检查是否为1GB大页
        if (Pdp[PdpIndex].u.Bits.LargePage) __leave; // 大页情况特殊处理

        // 5. 检查并映射PD页
        if (!Pdp[PdpIndex].u.Bits.Valid) __leave;
        PHYSICAL_ADDRESS PdPa;
        PdPa.QuadPart = Pdp[PdpIndex].u.Bits.PageFrameNumber << 12;
        Pd = (PHARDWARE_PTE)MapPhysicalAddress(PdPa, PAGE_SIZE);
        if (!Pd) __leave;

        // 6. 检查是否为2MB大页
        if (Pd[PdIndex].u.Bits.LargePage) __leave; // 大页情况特殊处理

        // 7. 检查并返回PTE
        if (!Pd[PdIndex].u.Bits.Valid) __leave;
        PHYSICAL_ADDRESS PtPa;
        PtPa.QuadPart = Pd[PdIndex].u.Bits.PageFrameNumber << 12;

        // 映射PTE页，返回PTE指针
        Pt = (PHARDWARE_PTE)MapPhysicalAddress(PtPa, PAGE_SIZE);
        if (Pt) {
            Result = &Pt[PtIndex];
            *OutMappedPage = Pt; // 返回映射的页，用于后续释放
        }
    }
    __finally {
        // 清理中间映射
        if (Pml4) UnmapPhysicalAddress(Pml4, PAGE_SIZE);
        if (Pdp)  UnmapPhysicalAddress(Pdp, PAGE_SIZE);
        if (Pd)   UnmapPhysicalAddress(Pd, PAGE_SIZE);
        // 注意：Pt不释放，由调用者释放
    }

    return Result;
}

// 判断内核页面是否可执行（通用版）
BOOLEAN IsPageExecutable(PVOID KernelVirtualAddress)
{
    // 仅处理内核地址
    if ((ULONG64)KernelVirtualAddress < 0xFFFF800000000000ULL) {
        return FALSE;
    }

    // IRQL检查
    KIRQL OldIrql = KeGetCurrentIrql();
    if (OldIrql > DISPATCH_LEVEL) {
        return FALSE;
    }

    PHARDWARE_PTE Pte = NULL;
    PHARDWARE_PTE MappedPage = NULL;
    BOOLEAN IsExecutable = FALSE;

    __try {
        // 获取PTE（动态方法）
        Pte = GetPteAddressDynamic(KernelVirtualAddress, &MappedPage);
        if (!Pte || !Pte->u.Bits.Valid) {
            __leave;
        }

        // 检查NX位：NoExecute=0表示可执行
        IsExecutable = (Pte->u.Bits.NoExecute == 0);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        IsExecutable = FALSE;
    }

    // 清理：释放最后映射的PTE页
    if (MappedPage) {
        UnmapPhysicalAddress(MappedPage, PAGE_SIZE);
    }

    return IsExecutable;
}

BOOLEAN IsInModuleList(PVOID addr)
{
    PLIST_ENTRY head = PsLoadedModuleList;
    PLIST_ENTRY cur = head->Flink;

    while (cur != head) {
        PKLDR_DATA_TABLE_ENTRY e =
            CONTAINING_RECORD(cur, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        PUCHAR base = (PUCHAR)e->DllBase;
        PUCHAR end = base + e->SizeOfImage;

        if ((PUCHAR)addr >= base && (PUCHAR)addr < end)
            return TRUE;

        cur = cur->Flink;
    }
    return FALSE;
}

BOOLEAN LooksLikePE(PVOID addr)
{
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)addr;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;

    PIMAGE_NT_HEADERS64 nt =
        (PIMAGE_NT_HEADERS64)((PUCHAR)addr + dos->e_lfanew);

    return nt->Signature == IMAGE_NT_SIGNATURE;
}

BOOLEAN IsShellcode(PUCHAR b)
{
    // mov rax, <imm64>; jmp rax
    if (b[0] == 0x48 && b[1] == 0xB8 && b[10] == 0xFF && b[11] == 0xE0) return TRUE;

    // jmp [rip+imm32]
    if (b[0] == 0xFF && b[1] == 0x25) return TRUE;

    // syscall; ret
    if (b[0] == 0x0F && b[1] == 0x05 && b[2] == 0xC3) return TRUE;

    // CR0 / CR4 modify (dangerous)
    if ((b[0] == 0x0F && b[1] == 0x20) ||
        (b[0] == 0x0F && b[1] == 0x22)) return TRUE;

    // rare trap instructions
    if (b[0] == 0xF1 || b[0] == 0xCC || b[0] == 0x0F && b[1] == 0x0B) return TRUE;

    return FALSE;
}

// 安全地检查地址范围是否有效
BOOLEAN IsAddressRangeValid(PVOID addr, SIZE_T size)
{
    if (!addr) return FALSE;

    // 检查是否对齐
    if ((ULONG64)addr & (PAGE_SIZE - 1)) return FALSE;

    // 检查整个范围
    PUCHAR start = (PUCHAR)addr;
    PUCHAR end = start + size - 1;

    // 检查首尾是否有效
    if (!MmIsAddressValid(start) || !MmIsAddressValid(end))
        return FALSE;

    // 检查是否跨越大边界（避免性能问题）
    if ((ULONG64)start >> 39 != (ULONG64)end >> 39)
        return FALSE;

    return TRUE;
}

// 检查页面是否可执行
/*BOOLEAN IsExecutablePage(PVOID addr)
{
    PHYSICAL_ADDRESS physAddr;
    PVOID mappedAddr = NULL;
    BOOLEAN result = FALSE;

    // 获取物理地址
    physAddr = MmGetPhysicalAddress(addr);
    if (physAddr.QuadPart == 0)
        return FALSE;

    // 映射物理地址到系统空间
    mappedAddr = MmGetVirtualForPhysical(physAddr);
    if (!mappedAddr || !MmIsAddressValid(mappedAddr))
        return FALSE;

    // 读取PTE内容
    ULONG64 pte = *(PULONG64)mappedAddr;

    // 检查Present位 (bit 0)
    if (!(pte & 1))
        goto Cleanup;

    // 检查NX位 (bit 63)，NX=0表示可执行
    result = !((pte >> 63) & 1);

Cleanup:
    // 注意：MmGetVirtualForPhysical 返回的地址不需要显式unmap
    return result;
}*/

BOOLEAN FastIsExecutablePage(PVOID addr)
{
    // 计算PTE虚拟地址：9+9+9位页表索引 + PTE基址
    ULONG64 vaddr = (ULONG64)addr;
    ULONG64 pteIndex = ((vaddr >> 12) & 0x1FF) * 8;  // 页内偏移

    // 三级页表索引
    ULONG64 pml4Index = ((vaddr >> 39) & 0x1FF) * 512 * 512 * 512 * 8;
    ULONG64 pdptIndex = ((vaddr >> 30) & 0x1FF) * 512 * 512 * 8;
    ULONG64 pdIndex = ((vaddr >> 21) & 0x1FF) * 512 * 8;

    PULONG64 ptePtr = (PULONG64)(
        PTE_BASE_ADDR +
        pml4Index + pdptIndex + pdIndex + pteIndex
        );

    if (!MmIsAddressValid(ptePtr))
        return FALSE;

    ULONG64 pte = *ptePtr;

    // Present位必须为1
    if (!(pte & 1))
        return FALSE;

    // NX位(bit 63) = 0 表示可执行
    return !((pte >> 63) & 1);
}

MEMORY_SCAN_RESULT OptimizedScanKernelMemory(PVOID pMem, ULONG PageSize)
{
    // ============= 快速预检（无锁，无拷贝） ============
    if (!pMem || PageSize != PAGE_SIZE || ((ULONG64)pMem & (PAGE_SIZE - 1))) return Normal;

    // 地址范围快速过滤（只扫描内核代码常见区域）
    //ULONG64 addr = (ULONG64)pMem;
    //if (addr < 0xFFFFF80000000000ULL || addr >= 0xFFFFF90000000000ULL) return Normal;

    // ProbeForRead快速测试（比MmCopyMemory快10倍）
    __try {

        // ============= PTE检查（可执行性） ============
        //if (!IsPageExecutable(pMem)) return Normal;

        // ============= 使用缓存的快速模块检查 ============
        //BOOLEAN inModule = FastIsInModuleList(pMem);

        // ============= 安全拷贝数据 ============
        PUCHAR pageBuffer = (PUCHAR)pMem;  // 直接访问，已通过ProbeForRead

        // ============= 特征检查（PE标志优先） ============
        // 先检查MZ头（最快），再检查PE
        IMAGE_DOS_HEADER dosHeader = { 0 };
        IMAGE_NT_HEADERS64 ntHeader = { 0 };
        SIZE_T bytRead = 0;
        if (!NT_SUCCESS(VxkCopyMemory(&dosHeader, pageBuffer, sizeof(IMAGE_DOS_HEADER), &bytRead))) return Normal;
        DbgPrint("读取0x%p成功", pMem);
        if (dosHeader.e_magic == IMAGE_DOS_SIGNATURE) {
            if (!NT_SUCCESS(VxkCopyMemory(&ntHeader, pageBuffer + dosHeader.e_lfanew, sizeof(IMAGE_NT_HEADERS64), &bytRead))) return Normal;
            if (ntHeader.Signature == IMAGE_NT_SIGNATURE) {
                //if (!inModule) {
                DbgPrint("[UMH] Hidden Driver: %p\n", pMem);
                return HideDriver;
                //}
            }
        }

        // Shellcode检查（只在非模块区域）
        /*if (IsShellcode(pageBuffer)) {
            DbgPrint("[UMH] Shellcode: %p\n", pMem);
            return Shellcode;
        }*/
        // UnknownCode检查（非模块且非PE）
    }
    _except(EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("非法地址:0x%p", pMem);
        return Normal;
    }

    return Normal;
}

// 扫描非分页池
/*VOID ScanNonPagedPool()
{
    ULONG_PTR currentAddr = (ULONG_PTR)MmNonPagedPoolStart;
    ULONG_PTR endAddr = (ULONG_PTR)MmNonPagedPoolEnd;
    ULONG chunkSize = 16; // 64位系统块对齐

    while (currentAddr < endAddr)
    {
        PPOOL_HEADER poolHeader = (PPOOL_HEADER)currentAddr;

        // 跳过空闲块
        if (poolHeader->BlockSize == 0 || poolHeader->PoolType == 0)
        {
            currentAddr += chunkSize;
            continue;
        }

        // 计算实际块大小
        ULONG blockSize = poolHeader->BlockSize * chunkSize;

        // 检测条件：大小超过阈值 + 无PE结构 + 不在模块列表 + 可执行代码特征
        if (blockSize >= SUSPICIOUS_MIN_SIZE &&
            !IsValidPE((PVOID)((PUCHAR)currentAddr + sizeof(POOL_HEADER))) &&
            !IsAddressInLoadedModules((PVOID)currentAddr))
        {
            // 检查是否包含可执行代码特征（连续的0x48 0x89开头）
            PUCHAR codeStart = (PUCHAR)currentAddr + sizeof(POOL_HEADER);
            for (int i = 0; i < 100; i++)
            {
                if (*(PULONG64)(codeStart + i) == 0x8948EC8348) // mov rax, rsp; mov rsp, rbp
                {
                    DbgPrint("[PoolScanner] 发现可疑无模块驱动分配: 地址=%p, 大小=%lu, Tag=%c%c%c%c\n",
                        currentAddr, blockSize,
                        (poolHeader->PoolTag >> 0) & 0xFF,
                        (poolHeader->PoolTag >> 8) & 0xFF,
                        (poolHeader->PoolTag >> 16) & 0xFF,
                        (poolHeader->PoolTag >> 24) & 0xFF);
                    break;
                }
            }
        }

        currentAddr += blockSize;
    }
}*/

NTSTATUS ReadVirtualMemory(PVOID VirtualAddress, SIZE_T Size, PVOID Buffer)
{
    PMDL pMdl = NULL;
    PVOID pMapped = NULL;
    NTSTATUS status = STATUS_SUCCESS;

    // 创建MDL
    pMdl = IoAllocateMdl(VirtualAddress, (ULONG)Size, FALSE, FALSE, NULL);
    if (!pMdl) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    __try {
        // 建立MDL，但不要探测和锁定页面（对于代码段）
        MmBuildMdlForNonPagedPool(pMdl);

        // 映射内存
        pMapped = MmMapLockedPagesSpecifyCache(
            pMdl,
            KernelMode,
            MmNonCached,
            NULL,
            FALSE,
            NormalPagePriority
        );

        if (pMapped) {
            // 复制数据
            RtlCopyMemory(Buffer, pMapped, Size);
            MmUnmapLockedPages(pMapped, pMdl);
        }
        else {
            status = STATUS_UNSUCCESSFUL;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    IoFreeMdl(pMdl);
    return status;
}