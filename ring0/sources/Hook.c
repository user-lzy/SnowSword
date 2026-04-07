#include "Hook.h"

// 原函数：判断指针是否在镜像范围内
static BOOLEAN
IsKernelCodePtr(PVOID Ptr, PVOID ImageBase, ULONG ImageSize)
{
    return (Ptr >= ImageBase &&
        Ptr < (PVOID)((PUCHAR)ImageBase + ImageSize));
}

// 修复SafeReadByte：支持从物理缓冲区读取（新增缓冲区参数）
BOOLEAN SafeReadByte(PUCHAR addr, PUCHAR out, PUCHAR PhysicalBuffer, PVOID DriverEntry)
{
    // 如果传入了物理缓冲区，优先从物理缓冲区读取（绕过EPT）
    if (PhysicalBuffer != NULL)
    {
        // 计算addr在物理缓冲区中的偏移
        ULONG_PTR offset = (ULONG_PTR)addr - (ULONG_PTR)DriverEntry;
        if (offset >= 0x800) // 物理缓冲区大小为0x800
        {
            DbgPrint("[SafeReadByte] Offset 0x%llX out of physical buffer range\n", offset);
            return FALSE;
        }
        *out = PhysicalBuffer[offset];
        return TRUE;
    }

    // 兼容原逻辑：虚拟地址读取（仅用于非虚拟化环境）
    __try {
        MM_COPY_ADDRESS ma = { 0 };
        ma.VirtualAddress = addr;
        SIZE_T bytesRead = 0;
        NTSTATUS status = MmCopyMemory(
            out,
            ma,
            sizeof(UCHAR),
            MM_COPY_MEMORY_VIRTUAL,
            &bytesRead
        );
        if (!NT_SUCCESS(status) || bytesRead != sizeof(UCHAR)) {
            DbgPrint("[SafeReadByte]0x%p, MmCopyMemory failed: %X\n", addr, status);
            return FALSE;
        }
        return TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint("[SafeReadByte] Exception reading 0x%p\n", addr);
        return FALSE;
    }
}

// 原函数：解码写入事件（适配物理缓冲区读取）
BOOLEAN
DecodeWriteEvent(
    PUCHAR Code,
    PUCHAR ImageBase,
    ULONG ImageSize,
    X64_REG DriverObjectReg,
    WRITE_EVENT* Event,
    ULONG* InstLen,
    PUCHAR PhysicalBuffer,
    PVOID DriverEntry
)
{
    RtlZeroMemory(Event, sizeof(*Event));
    *InstLen = 0;

    UCHAR b0, b1, b2;
    if (!SafeReadByte(Code + 0, &b0, PhysicalBuffer, DriverEntry) ||
        !SafeReadByte(Code + 1, &b1, PhysicalBuffer, DriverEntry) ||
        !SafeReadByte(Code + 2, &b2, PhysicalBuffer, DriverEntry))
    {
        return FALSE;
    }

    if (b0 == 0x48 && b1 == 0x8D && b2 == 0x05)
    {
        // 安全读取rip偏移（从物理缓冲区/虚拟地址）
        INT32 ripOff = 0;
        PUCHAR ripOffAddr = Code + 3;
        if (PhysicalBuffer != NULL)
        {
            ULONG_PTR offset = (ULONG_PTR)ripOffAddr - (ULONG_PTR)DriverEntry;
            if (offset + sizeof(INT32) > 0x800) return FALSE;
            ripOff = *(INT32*)(PhysicalBuffer + offset);
        }
        else
        {
            __try {
                ripOff = *(INT32*)ripOffAddr;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return FALSE;
            }
        }

        PUCHAR Target = Code + 7 + ripOff;
        if (!IsKernelCodePtr(Target, ImageBase, ImageSize))
            return FALSE;

        // 解析下一条指令：mov [reg+imm], rax
        PUCHAR Next = Code + 7;
        UCHAR nb0, nb1;
        if (!SafeReadByte(Next + 0, &nb0, PhysicalBuffer, DriverEntry) ||
            !SafeReadByte(Next + 1, &nb1, PhysicalBuffer, DriverEntry))
        {
            return FALSE;
        }

        if (nb0 == 0x48 && nb1 == 0x89) // 修正：原代码是 ||，应为 &&（mov rax到内存的正确指令前缀）
        {
            UCHAR modrm;
            if (!SafeReadByte(Next + 2, &modrm, PhysicalBuffer, DriverEntry)) return FALSE;
            UCHAR reg = (modrm >> 3) & 7;
            UCHAR rm = modrm & 7;

            if (reg == 0) // rax
            {
                X64_REG base = (X64_REG)(rm);
                ULONG offset = 0;
                PUCHAR offsetAddr = Next + 3;

                // 安全读取偏移值
                if (PhysicalBuffer != NULL)
                {
                    ULONG_PTR off = (ULONG_PTR)offsetAddr - (ULONG_PTR)DriverEntry;
                    if (off + sizeof(INT32) > 0x800) return FALSE;
                    offset = *(INT32*)(PhysicalBuffer + off);
                }
                else
                {
                    __try {
                        offset = *(INT32*)offsetAddr;
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                        return FALSE;
                    }
                }

                Event->BaseReg = base;
                Event->Offset = offset;
                Event->Target = Target;

                *InstLen = 7 + 7;
                return TRUE;
            }
        }
    }

    return FALSE;
}

// 核心函数：恢复Dispatch表（修复物理读取逻辑）
BOOLEAN
RecoverDispatchFromDriverEntry(
    PVOID DriverImageBase,
    ULONG DriverImageSize,
    PVOID DriverEntry,
    PRECOVERED_DISPATCH Out
)
{
    PARSE_STATE State = ST_INIT;
    X64_REG DriverObjectReg = REG_INVALID;
    PHYSICAL_ADDRESS pa = { 0 };
    PUCHAR pPhysicalBuffer = NULL;
    SIZE_T bytesRead = 0;

    // 初始化输出结构体
    RtlZeroMemory(Out, sizeof(*Out));

    // 1. 获取DriverEntry的物理地址（Hypervisor通常不会伪造物理地址）
    pa = MmGetPhysicalAddress(DriverEntry);
    if (pa.QuadPart == 0) {
        DbgPrint("[RecoverDispatchFromDriverEntry] MmGetPhysicalAddress returned 0 for DriverEntry: %p\n", DriverEntry);
        return FALSE;
    }

    // 2. 分配非分页缓冲区（大小0x800，足够覆盖DriverEntry附近指令）
    pPhysicalBuffer = (PUCHAR)ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_UNINITIALIZED,
        0x800, 'rspm'); // Tag为4个字符，倒序显示为'mpsr'
    if (!pPhysicalBuffer) {
        DbgPrint("[RecoverDispatchFromDriverEntry] ExAllocatePool2 failed\n");
        return FALSE;
    }

    // 3. 从物理层读取内存（绕过EPT/Guest页表，核心！）
    MM_COPY_ADDRESS mmAddr = { 0 };
    mmAddr.PhysicalAddress = pa;
    NTSTATUS status = MmCopyMemory(
        pPhysicalBuffer,
        mmAddr,
        0x800,
        MM_COPY_MEMORY_PHYSICAL, // 关键标志：直接读取物理内存
        &bytesRead
    );

    if (!NT_SUCCESS(status) || bytesRead != 0x800) {
        DbgPrint("[RecoverDispatchFromDriverEntry] MmCopyMemory failed: %X, bytesRead: %llu\n", status, bytesRead);
        ExFreePoolWithTag(pPhysicalBuffer, 'rspm');
        return FALSE;
    }
    DbgPrint("[ARK] Read %llu bytes from physical memory at 0x%llX (DriverEntry: %p)\n",
        bytesRead, pa.QuadPart, DriverEntry);

    // 4. 在物理缓冲区上解析指令（绕过EPT拦截）
    PUCHAR pc = (PUCHAR)DriverEntry;
    PUCHAR end = pc + 0x800;

    while (pc < end)
    {
        UCHAR b0, b1;
        // 从物理缓冲区读取指令字节
        if (!SafeReadByte(pc + 0, &b0, pPhysicalBuffer, DriverEntry) ||
            !SafeReadByte(pc + 1, &b1, pPhysicalBuffer, DriverEntry))
        {
            break;
        }

        // 识别 mov reg, rcx（DriverObject赋值指令）
        if (b0 == 0x48 && b1 == 0x89)
        {
            UCHAR modrm;
            if (!SafeReadByte(pc + 2, &modrm, pPhysicalBuffer, DriverEntry))
            {
                break;
            }
            // modrm格式：高2位Mod，中间3位Reg，低3位RM
            if (((modrm >> 3) & 0x07) == 1) // Reg字段=1 → RCX
            {
                DriverObjectReg = (X64_REG)(modrm & 0x07); // RM字段=目标寄存器
                State = ST_HAVE_DOBJECT;
                DbgPrint("[ARK] Found DriverObjectReg: %u (0x%X)\n", DriverObjectReg, modrm);
            }
        }

        // 解析写入Dispatch表的指令
        if (State >= ST_HAVE_DOBJECT)
        {
            WRITE_EVENT ev;
            ULONG len;

            if (DecodeWriteEvent(
                pc,
                (PUCHAR)DriverImageBase,
                DriverImageSize,
                DriverObjectReg,
                &ev,
                &len,
                pPhysicalBuffer,  // 传入物理缓冲区
                DriverEntry))     // 传入DriverEntry基址
            {
                if (ev.BaseReg == DriverObjectReg &&
                    ev.Offset >= 0x70 &&
                    ev.Offset < 0x70 + IRP_MJ_MAXIMUM_FUNCTION * 8)
                {
                    ULONG index = (ev.Offset - 0x70) / 8;
                    Out->MajorFunction[index] = ev.Target;
                    Out->HitCount++;
                    Out->Confidence = 90; // 命中则设置可信度

                    State = ST_PARSING;
                    pc += len;
                    continue;
                }
            }
        }

        pc++;
    }

    // 释放物理缓冲区
    ExFreePoolWithTag(pPhysicalBuffer, 'rspm');

    // 可信度评估：命中≥15个Dispatch函数则返回成功
    if (Out->HitCount >= 15)
    {
        DbgPrint("[ARK] Recovered %lu dispatch functions\n", Out->HitCount);
        return TRUE;
    }

    DbgPrint("[ARK] Only recovered %lu dispatch functions (need ≥15)\n", Out->HitCount);
    return FALSE;
}

// 测试函数
VOID TestNtfsRecover()
{
    PVOID ntfsBase = (PVOID)0xFFFFF8060E3F0000;
    ULONG ntfsSize = 0x2D7000;
    PVOID ntfsEntry = (PVOID)0xFFFFF8060E682010;

    RECOVERED_DISPATCH rd;
    RtlZeroMemory(&rd, sizeof(rd));

    if (RecoverDispatchFromDriverEntry(
        ntfsBase,
        ntfsSize,
        ntfsEntry,
        &rd))
    {
        DbgPrint("[ARK] NTFS dispatch recovered, hit=%lu\n", rd.HitCount);

        for (ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        {
            if (rd.MajorFunction[i])
            {
                DbgPrint("IRP_MJ_%u -> %p\n", i, rd.MajorFunction[i]);
            }
        }
    }
    else
    {
        DbgPrint("[ARK] NTFS recover failed\n");
    }
}
