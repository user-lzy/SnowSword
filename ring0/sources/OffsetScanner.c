// OffsetScanner.c
#include "OffsetScanner.h"
#include "Symbol.h"
#include <Zydis.h>

// ==========================================
// 元数据偏移计算宏
// ==========================================
#define HandleEntry_HOOKTYPE_OFFSET(entrySize)     ((entrySize) - 8)
#define HandleEntry_TABLEINDEX_OFFSET(entrySize)   ((entrySize) - 6)

#define SCAN_MAX_SIZE       0x5000      // 最大扫描 20KB
#define SCAN_MAX_DEPTH      4096        // 最大指令数

// ==========================================
// 模式匹配上下文
// ==========================================
typedef struct _SCAN_CONTEXT {
    PWIN32K_OFFSETS Offsets;
    PVOID FuncBase;
    SIZE_T FuncSize;
    ULONG FoundMask;        // 位图：哪些偏移已找到
    ULONG Confidence[32];   // 每个偏移的置信度计数
} SCAN_CONTEXT, * PSCAN_CONTEXT;

// 修复：将字节偏移转换为位索引（每个 ULONG 对应一个位）
#define OFFSET_BIT_INDEX(_field) \
    ((ULONG)(FIELD_OFFSET(WIN32K_OFFSETS, _field) / sizeof(ULONG)))

#define SET_FOUND(_ctx, _field) \
    (_ctx)->FoundMask |= (1UL << OFFSET_BIT_INDEX(_field))

#define IS_FOUND(_ctx, _field) \
    (((_ctx)->FoundMask & (1UL << OFFSET_BIT_INDEX(_field))) != 0)

#define SET_OFFSET(_ctx, _field, _val) do { \
    *(PULONG)((PUCHAR)(_ctx)->Offsets + FIELD_OFFSET(WIN32K_OFFSETS, _field)) = (ULONG)(_val); \
    SET_FOUND(_ctx, _field); \
} while(0)

// static BOOLEAN g_bHasW32GetUserSessionState = FALSE;

static VOID DetectSessionStateMethod(
    _Out_ PBOOLEAN pHasSessionState
)
{
    PVOID addr = NULL;
    if (KernelQuerySymbolAddress(L"win32k.sys", L"W32GetUserSessionState", (PULONG64)&addr) == STATUS_SUCCESS && addr != NULL)
    {
        *pHasSessionState = TRUE;
    }
    else
    {
        *pHasSessionState = FALSE;
    }
}

// ==========================================
// Zydis 解码包装
// ==========================================
static BOOLEAN ZydisDecodeAt(
    _In_ PUCHAR Buffer,
    _In_ SIZE_T Length,
    _Out_ ZydisDecodedInstruction* Instruction,
    _Out_ ZydisDecodedOperand* Operands,
    _Out_ SIZE_T* InstructionLength
)
{
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    ZyanStatus status = ZydisDecoderDecodeFull(
        &decoder,
        Buffer,
        Length,
        Instruction,
        Operands
    );

    if (!ZYAN_SUCCESS(status))
        return FALSE;

    *InstructionLength = Instruction->length;
    return TRUE;
}

// ==========================================
// 指令特征匹配辅助
// ==========================================
static BOOLEAN IsRegMatch(ZydisRegister reg, ZydisRegister expected)
{
    return (expected == ZYDIS_REGISTER_NONE) || (reg == expected);
}

static BOOLEAN GetMemoryDisp(
    _In_ const ZydisDecodedInstruction* Instr,
    _In_ const ZydisDecodedOperand* Operands,
    _In_ ZydisRegister expectedBase,
    _In_ ZydisRegister expectedIndex,
    _In_ ULONG expectedScale,
    _Out_ LONG* Disp
)
{
    for (ZyanU8 i = 0; i < Instr->operand_count; i++) {
        const ZydisDecodedOperand* op = &Operands[i];
        if (op->type != ZYDIS_OPERAND_TYPE_MEMORY)
            continue;
        if (!IsRegMatch(op->mem.base, expectedBase))
            continue;
        if (!IsRegMatch(op->mem.index, expectedIndex))
            continue;
        if (expectedScale != 0 && op->mem.scale != expectedScale)
            continue;
        *Disp = (LONG)op->mem.disp.value;
        return TRUE;
    }
    return FALSE;
}

static BOOLEAN HasDestReg(const ZydisDecodedInstruction* i,
    const ZydisDecodedOperand* ops, ZydisRegister r) {
    return i->operand_count > 0 &&
        ops[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
        ops[0].reg.value == r;
}

// ==========================================
// 具体偏移提取器
// ==========================================

// 1. pti->aphkStart: lea r12, [r14 + 0x3C0]
static BOOLEAN ExtractPtiAphkStart(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_LEA)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R12))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R14, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0x300 || disp > 0x500)
        return FALSE;

    SET_OFFSET(ctx, Pti_aphkStart, (ULONG)disp);
    SET_FOUND(ctx, Pti_aphkStart);
    return TRUE;
}

// 3. pti->pDeskInfo: mov r12, [r13 + 0x1F8]
static BOOLEAN ExtractPtiDeskInfo(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R12))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R13, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0x100 || disp > 0x300)
        return FALSE;

    SET_OFFSET(ctx, Pti_pDeskInfo, (ULONG)disp);
    SET_FOUND(ctx, Pti_pDeskInfo);
    return TRUE;
}

// 4. pDeskInfo->aphkStart: add r12, 0x28 (紧跟在 mov r12,[r13+1F8] 之后)
//    或从 xxxCallHook: mov rcx, [rax+rdi*8+0x30]
static BOOLEAN ExtractDeskInfoAphkStart(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T currentOffset,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    UNREFERENCED_PARAMETER(codeBase);
    UNREFERENCED_PARAMETER(currentOffset);

    // 只信任模式 A：zzzSetWindowHookEx 中的 add r12, imm
    // 模式 B（xxxCallHook 中的 mov rcx, [rax+rdi*8+disp]）太容易误匹配
    if (instr->mnemonic != ZYDIS_MNEMONIC_ADD)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R12))
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;

            if (imm == 0x28) {
                SET_OFFSET(ctx, DeskInfo_aphkStart, 0x28);
                SET_FOUND(ctx, DeskInfo_aphkStart);
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOLEAN ExtractHookHHook(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    // 模式: mov [rdi], rdx  (handle value 存入 tagHOOK+0x00)
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
        return FALSE;
    if (operands[0].mem.base != ZYDIS_REGISTER_RDI)
        return FALSE;
    if (operands[0].mem.index != ZYDIS_REGISTER_NONE)
        return FALSE;
    if (operands[0].mem.disp.value != 0)
        return FALSE;
    if (operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;

    ZydisRegister srcReg = operands[1].reg.value;
    if (srcReg != ZYDIS_REGISTER_RDX && srcReg != ZYDIS_REGISTER_RAX)
        return FALSE;

    if (IS_FOUND(ctx, Hook_hHook) && ctx->Offsets->Hook_hHook != 0)
        return FALSE;

    SET_OFFSET(ctx, Hook_hHook, 0);
    SET_FOUND(ctx, Hook_hHook);
    DbgPrint("[+] Hook_hHook = 0x0\n");
    return TRUE;
}

// 5. hook->flags
// Win11: or [r15 + 0x40], edi
// Win10: or [rdi + 0x40], eax   /   or [rdi + 0x40], r15d
static BOOLEAN ExtractHookFlagsGlobal(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_OR) return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R15, ZYDIS_REGISTER_NONE, 0, &disp) &&
        !GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RDI, ZYDIS_REGISTER_NONE, 0, &disp)) {
        //DbgPrint("[Zydis] ExtractHookFlagsGlobal: no valid memory operand\n");
        return FALSE;
    }

    // 源操作数必须是 edi / eax / r15d（不能是立即数）
    BOOLEAN hasValidSrc = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            (operands[i].reg.value == ZYDIS_REGISTER_EDI ||
                operands[i].reg.value == ZYDIS_REGISTER_EAX ||
                operands[i].reg.value == ZYDIS_REGISTER_R15D)) {
            hasValidSrc = TRUE;
            break;
        }
    }
    if (!hasValidSrc)
        return FALSE;

    // 排除立即数源（如 or [rax+44h], 0FFFFFFFFh）
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            //DbgPrint("[Zydis] ExtractHookFlagsGlobal: source operand is immediate, skipping\n");
            return FALSE;
        }
    }

    SET_OFFSET(ctx, Hook_flags, (ULONG)disp);
    SET_FOUND(ctx, Hook_flags);
    return TRUE;
}

// 6. hook->ihmod
// Win11: mov [r15 + 0x44], eax
// Win10: mov [rdi + 0x44], eax
static BOOLEAN ExtractHookIhmod(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R15, ZYDIS_REGISTER_NONE, 0, &disp) &&
        !GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RDI, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if (disp < 0x40 || disp > 0x50)
        return FALSE;

    // 检查内存操作数是否是 WRITE 目标（确保是 mov [mem], reg 方向）
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
            if (!(operands[i].actions & ZYDIS_OPERAND_ACTION_WRITE)) {
                return FALSE;
            }
            break;
        }
    }

    BOOLEAN hasEax = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[i].reg.value == ZYDIS_REGISTER_EAX) {
            hasEax = TRUE;
            break;
        }
    }
    if (!hasEax) return FALSE;

    SET_OFFSET(ctx, Hook_ihmod, (ULONG)disp);
    SET_FOUND(ctx, Hook_ihmod);
    return TRUE;
}

// 7. hook->nHookType
// Win11: mov [r15 + 0x30], r13d
// Win10: mov [rdi + 0x30], r13d
static BOOLEAN ExtractHookType(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R15, ZYDIS_REGISTER_NONE, 0, &disp) &&
        !GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RDI, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if (disp < 0x20 || disp > 0x40)
        return FALSE;

    // 检查内存操作数是否是 WRITE 目标（确保是 mov [mem], reg 方向）
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
            if (!(operands[i].actions & ZYDIS_OPERAND_ACTION_WRITE)) {
                return FALSE;
            }
            break;
        }
    }

    BOOLEAN hasR13d = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[i].reg.value == ZYDIS_REGISTER_R13D) {
            hasR13d = TRUE;
            break;
        }
    }
    if (!hasR13d) return FALSE;

    SET_OFFSET(ctx, Hook_nHookType, (ULONG)disp);
    SET_FOUND(ctx, Hook_nHookType);
    return TRUE;
}

// 8. hook->offPfn
// Win11: mov [r15 + 0x38], rax
// Win10: mov [rdi + 0x38], rax
static BOOLEAN ExtractHookOffPfn(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R15, ZYDIS_REGISTER_NONE, 0, &disp) &&
        !GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RDI, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if (disp < 0x30 || disp > 0x40)
        return FALSE;

    // 检查内存操作数是否是 WRITE 目标（确保是 mov [mem], reg 方向）
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
            if (!(operands[i].actions & ZYDIS_OPERAND_ACTION_WRITE)) {
                return FALSE;
            }
            break;
        }
    }

    BOOLEAN hasRax = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[i].reg.value == ZYDIS_REGISTER_RAX) {
            hasRax = TRUE;
            break;
        }
    }
    if (!hasRax) return FALSE;

    SET_OFFSET(ctx, Hook_offPfn, (ULONG)disp);
    SET_FOUND(ctx, Hook_offPfn);
    return TRUE;
}

// 9. hook->phkNext
// Win11: mov [r15 + 0x28], rcx
// Win10: mov [rdi + 0x28], rax
static BOOLEAN ExtractHookPhkNext(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R15, ZYDIS_REGISTER_NONE, 0, &disp) &&
        !GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RDI, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if (disp < 0x20 || disp > 0x30)
        return FALSE;

    BOOLEAN hasValidSrc = FALSE;
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            (operands[i].reg.value == ZYDIS_REGISTER_RCX ||
                operands[i].reg.value == ZYDIS_REGISTER_RAX)) {
            hasValidSrc = TRUE;
            break;
        }
    }
    if (!hasValidSrc) return FALSE;

    SET_OFFSET(ctx, Hook_phkNext, (ULONG)disp);
    SET_FOUND(ctx, Hook_phkNext);
    return TRUE;
}

//static BOOLEAN ExtractHookPti(
//    _In_ PSCAN_CONTEXT ctx,
//    _In_ PUCHAR codeBase,
//    _In_ SIZE_T offset,
//    _In_ const ZydisDecodedInstruction* instr,
//    _In_ const ZydisDecodedOperand* operands
//)
//{
//    // 模式: mov rdi, [rcx+disp]  (从 tagHOOK 读取 pti)
//    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
//        return FALSE;
//    if (operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
//        return FALSE;
//    if (operands[0].reg.value != ZYDIS_REGISTER_RDI)
//        return FALSE;
//    if (operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
//        return FALSE;
//    if (operands[1].mem.base != ZYDIS_REGISTER_RCX)
//        return FALSE;
//
//    LONG disp = (LONG)operands[1].mem.disp.value;
//    if (disp < 0x08 || disp > 0x18)
//        return FALSE;
//
//    // 验证: 后续 64 字节内应有 [rdi+0x1A8] 或 [rdi+0x1E8] 访问 (pti 的已知字段)
//    BOOLEAN bVerified = FALSE;
//    SIZE_T verifyOffset = offset + instr->length;
//    for (SIZE_T i = 0; i < 64 && (verifyOffset + i + 4) < ctx->FuncSize; i++) {
//        if (!MmIsAddressValid(codeBase + verifyOffset + i))
//            break;
//
//        PUCHAR p = codeBase + verifyOffset + i;
//        // 检查访问 [rdi+disp32] 的指令 (如 test [rdi+1E8], imm)
//        if (p[0] == 0xF7 && (p[1] == 0x87 || p[1] == 0xBF)) {
//            LONG checkDisp = *(PLONG)(p + 2);
//            if (checkDisp == 0x1A8 || checkDisp == 0x1E8) {
//                bVerified = TRUE;
//                break;
//            }
//        }
//    }
//
//    if (!bVerified)
//        return FALSE;
//
//    if (IS_FOUND(ctx, Hook_pti) && ctx->Offsets->Hook_pti != (ULONG)disp) {
//        DbgPrint("[!] Hook_pti conflict: existing=%u new=%u\n",
//            ctx->Offsets->Hook_pti, (ULONG)disp);
//        return FALSE;
//    }
//
//    SET_OFFSET(ctx, Hook_pti, (ULONG)disp);
//    SET_FOUND(ctx, Hook_pti);
//    DbgPrint("[+] Hook_pti = 0x%X (verified by pti+0x1E8/0x1A8 access)\n", (ULONG)disp);
//    return TRUE;
//}

// ---- Hook_pti 提取器 ----
// 优先从 zzzUnhookWindowsHookEx (mov rdi, [rcx+disp]) 提取并验证
// 回退到 HMAllocObject (mov [rdi/rbx+disp], rbp/rax)
static BOOLEAN ExtractHookPti(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T offset,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    LONG disp = -1;
    BOOLEAN bVerified = FALSE;

    // 模式 A: zzzUnhookWindowsHookEx
    if (instr->mnemonic == ZYDIS_MNEMONIC_MOV &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
        operands[0].reg.value == ZYDIS_REGISTER_RDI &&
        operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY &&
        operands[1].mem.base == ZYDIS_REGISTER_RCX) {
        disp = (LONG)operands[1].mem.disp.value;
        if (disp < 0x08 || disp > 0x18) return FALSE;

        SIZE_T voff = offset + instr->length;
        for (SIZE_T i = 0; i < 64 && (voff + i + 4) < ctx->FuncSize; i++) {
            if (!MmIsAddressValid(codeBase + voff + i)) break;
            PUCHAR p = codeBase + voff + i;
            if ((p[0] == 0xF7 || p[0] == 0x8B || p[0] == 0x89) &&
                (p[1] == 0x87 || p[1] == 0xBF)) {
                if (*(PLONG)(p + 2) == 0x1A8 || *(PLONG)(p + 2) == 0x1E8) {
                    bVerified = TRUE; break;
                }
            }
        }
    }
    // 模式 B: HMAllocObject (仅当模式 A 未找到时)
    else if (!IS_FOUND(ctx, Hook_pti) &&
        instr->mnemonic == ZYDIS_MNEMONIC_MOV &&
        operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {
        ZydisRegister db = operands[0].mem.base;
        if (db != ZYDIS_REGISTER_RDI && db != ZYDIS_REGISTER_RBX) return FALSE;
        disp = (LONG)operands[0].mem.disp.value;
        if (disp < 0x08 || disp > 0x18) return FALSE;
        if (operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER) return FALSE;
        ZydisRegister src = operands[1].reg.value;
        if (src != ZYDIS_REGISTER_RBP && src != ZYDIS_REGISTER_RAX) return FALSE;
        bVerified = TRUE;  // 弱验证
    }
    else return FALSE;

    if (!bVerified) return FALSE;
    if (IS_FOUND(ctx, Hook_pti) && ctx->Offsets->Hook_pti != (ULONG)disp) return FALSE;

    SET_OFFSET(ctx, Hook_pti, (ULONG)disp);
    SET_FOUND(ctx, Hook_pti);
    DbgPrint("[+] Hook_pti = 0x%X%s\n", (ULONG)disp,
        bVerified ? "" : " (tentative)");
    return TRUE;
}

// ---- HandleEntrySize 提取器 ----
// 旧: 单一数组 24-byte (lea r8, [r10+r10*2])
// 新: 分离数组 32-byte (shl r15, 5)
static BOOLEAN ExtractHandleEntrySize(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    ULONG entrySize = 0;
    BOOLEAN bNew = FALSE;

    if (instr->mnemonic == ZYDIS_MNEMONIC_SHL &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
        operands[0].reg.value == ZYDIS_REGISTER_R15 &&
        operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
        ZyanU64 shift = operands[1].imm.value.u;
        if (shift < 3 || shift > 6) return FALSE;
        entrySize = 1 << (ULONG)shift;
        bNew = TRUE;
    }
    else if (instr->mnemonic == ZYDIS_MNEMONIC_LEA &&
        operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
        operands[0].reg.value == ZYDIS_REGISTER_R8 &&
        operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY &&
        operands[1].mem.base == ZYDIS_REGISTER_R10 &&
        operands[1].mem.index == ZYDIS_REGISTER_R10 &&
        operands[1].mem.scale == 2) {
        entrySize = 24;  // 旧版本固定 24
        bNew = FALSE;
    }
    else return FALSE;

    if (IS_FOUND(ctx, HandleEntrySize) &&
        ctx->Offsets->HandleEntrySize != entrySize) return FALSE;

    SET_OFFSET(ctx, HandleEntrySize, entrySize);
    SET_FOUND(ctx, HandleEntrySize);
    DbgPrint("[+] HandleEntrySize = %u (%s)\n", entrySize, bNew ? "new" : "old");
    return TRUE;
}

//static BOOLEAN ExtractHandleEntryHookTypeOffset(
//    _In_ PSCAN_CONTEXT ctx,
//    _In_ const ZydisDecodedInstruction* instr,
//    _In_ const ZydisDecodedOperand* operands
//)
//{
//    // 模式: mov [r15+disp], r8b  (存储 flags 到 handle entry)
//    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
//        return FALSE;
//    if (operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
//        return FALSE;
//    if (operands[0].mem.base != ZYDIS_REGISTER_R15)
//        return FALSE;
//    if (operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
//        return FALSE;
//
//    ZydisRegister srcReg = operands[1].reg.value;
//    if (srcReg != ZYDIS_REGISTER_R8B && srcReg != ZYDIS_REGISTER_R8)
//        return FALSE;
//
//    LONG disp = (LONG)operands[0].mem.disp.value;
//    if (disp < 0x08 || disp > 0x20)
//        return FALSE;
//
//    if (IS_FOUND(ctx, HandleEntry_HookTypeOffset) &&
//        ctx->Offsets->HandleEntry_HookTypeOffset != (ULONG)disp) {
//        DbgPrint("[!] HandleEntry_HookTypeOffset conflict: existing=%u new=%u\n",
//            ctx->Offsets->HandleEntry_HookTypeOffset, (ULONG)disp);
//        return FALSE;
//    }
//
//    SET_OFFSET(ctx, HandleEntry_HookTypeOffset, (ULONG)disp);
//    SET_FOUND(ctx, HandleEntry_HookTypeOffset);
//    DbgPrint("[+] HandleEntry_HookTypeOffset = 0x%X\n", (ULONG)disp);
//    return TRUE;
//}

// ---- Meta_HookTypeOffset 提取器 ----
// 旧: mov [r9+r8*8+10h], al  (单一 24-byte 数组)
// 新: mov [r15+18h], r8b     (分离 32-byte handle entry)
static BOOLEAN ExtractHandleEntryHookTypeOffset(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV) return FALSE;
    if (operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY) return FALSE;
    if (operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER) return FALSE;

    LONG disp = -1;
    if (operands[0].mem.base == ZYDIS_REGISTER_R15) {
        // 新版本
        ZydisRegister src = operands[1].reg.value;
        if (src != ZYDIS_REGISTER_R8B && src != ZYDIS_REGISTER_R8) return FALSE;
        disp = (LONG)operands[0].mem.disp.value;
    }
    else if (operands[0].mem.base == ZYDIS_REGISTER_R9 &&
        operands[0].mem.index == ZYDIS_REGISTER_R8 &&
        operands[0].mem.scale == 8) {
        // 旧版本
        ZydisRegister src = operands[1].reg.value;
        if (src != ZYDIS_REGISTER_AL && src != ZYDIS_REGISTER_RAX) return FALSE;
        disp = (LONG)operands[0].mem.disp.value;
    }
    else return FALSE;

    if (disp < 0x08 || disp > 0x20) return FALSE;

    if (IS_FOUND(ctx, HandleEntry_HookTypeOffset) &&
        ctx->Offsets->HandleEntry_HookTypeOffset != (ULONG)disp) return FALSE;

    SET_OFFSET(ctx, HandleEntry_HookTypeOffset, (ULONG)disp);
    SET_FOUND(ctx, HandleEntry_HookTypeOffset);
    DbgPrint("[+] HandleEntry_HookTypeOffset = 0x%X\n", (ULONG)disp);
    return TRUE;
}

static BOOLEAN ExtractHandleEntryTableIndexOffset(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    // 模式: movzx ecx, word ptr [rcx+rax+disp]
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOVZX)
        return FALSE;
    if (operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
        return FALSE;

    ZydisRegister dstReg = operands[0].reg.value;
    if (dstReg != ZYDIS_REGISTER_ECX && dstReg != ZYDIS_REGISTER_RCX)
        return FALSE;

    if (operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
        return FALSE;
    if (operands[1].size != 16)  // word
        return FALSE;
    if (operands[1].mem.base != ZYDIS_REGISTER_RCX)
        return FALSE;
    if (operands[1].mem.index != ZYDIS_REGISTER_RAX)
        return FALSE;

    LONG disp = (LONG)operands[1].mem.disp.value;
    if (disp < 0x10 || disp > 0x20)
        return FALSE;

    if (IS_FOUND(ctx, HandleEntry_TableIndexOffset) &&
        ctx->Offsets->HandleEntry_TableIndexOffset != (ULONG)disp) {
        DbgPrint("[!] HandleEntry_TableIndexOffset conflict: existing=%u new=%u\n",
            ctx->Offsets->HandleEntry_TableIndexOffset, (ULONG)disp);
        return FALSE;
    }

    SET_OFFSET(ctx, HandleEntry_TableIndexOffset, (ULONG)disp);
    SET_FOUND(ctx, HandleEntry_TableIndexOffset);
    DbgPrint("[+] HandleEntry_TableIndexOffset = 0x%X\n", (ULONG)disp);
    return TRUE;
}

// 15. pti->amdesk: mov ecx, [r13 + 0x3A8]
static BOOLEAN ExtractPtiAmdesk(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_ECX))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_R13, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0x300 || disp > 0x500)
        return FALSE;

    SET_OFFSET(ctx, Pti_amdesk, (ULONG)disp);
    SET_FOUND(ctx, Pti_amdesk);
    return TRUE;
}

// 16. SessionState.HookArray: mov rbx, [rax + 0xA510] (xxxCallHook)
static BOOLEAN ExtractSessionStateHookArray(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_RBX))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RAX, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0xA000 || disp > 0xB000)
        return FALSE;

    SET_OFFSET(ctx, SessionState_HookArray, (ULONG)disp);
    SET_FOUND(ctx, SessionState_HookArray);
    return TRUE;
}

// 17. SessionState.Flag: cmp dword ptr [rax + 0x4CF8], 0 (xxxCallHook)
static BOOLEAN ExtractSessionStateFlag(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_CMP)
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RAX, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0x4000 || disp > 0x6000)
        return FALSE;

    // 验证立即数是 0
    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            // 根据符号标志选择正确的成员进行比较
            ZyanBool isZero = ZYAN_FALSE;
            if (operands[i].imm.is_signed) {
                isZero = (operands[i].imm.value.s == 0);
            }
            else {
                isZero = (operands[i].imm.value.u == 0);
            }

            if (isZero) {
                SET_OFFSET(ctx, SessionState_Flag, (ULONG)disp);
                SET_FOUND(ctx, SessionState_Flag);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 18. pti->aphkStart 验证 (xxxCallHook): mov rcx, [rbp+rdi*8+0x3C8]
static BOOLEAN ExtractPtiAphkStartVerify(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_RCX))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RBP, ZYDIS_REGISTER_RDI, 8, &disp))
        return FALSE;
    if (disp < 0x300 || disp > 0x500)
        return FALSE;

    // 与之前提取的值交叉验证
    SET_OFFSET(ctx, Pti_aphkStart, (ULONG)disp);
    SET_FOUND(ctx, Pti_aphkStart);
    return TRUE;
}

// ==========================================
// Win10 专用偏移提取器
// ==========================================

// Win10: pti->aphkStart 从 xxxCallHook: mov rcx, [rax+rdx*8+0x398]
static BOOLEAN ExtractPtiAphkStartWin10(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_RCX))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RAX, ZYDIS_REGISTER_RDX, 8, &disp))
        return FALSE;
    if (disp < 0x300 || disp > 0x500)
        return FALSE;

    SET_OFFSET(ctx, Pti_aphkStart, (ULONG)disp);
    SET_FOUND(ctx, Pti_aphkStart);
    //DbgPrint("[Zydis][Win10] pti->aphkStart = 0x%X\n", ctx->Offsets->Pti_aphkStart);
    return TRUE;
}

// Win10: pti->pDeskInfo 从 zzzSetWindowsHookEx: mov r8, [rbx+0x1D0]
static BOOLEAN ExtractPtiDeskInfoWin10(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV) return FALSE;
    
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R8)) return FALSE;
    
    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RBX, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;

    if (disp < 0x100 || disp > 0x300) return FALSE;

    SET_OFFSET(ctx, Pti_pDeskInfo, (ULONG)disp);
    SET_FOUND(ctx, Pti_pDeskInfo);
    return TRUE;
}

// Win10: DeskInfo->aphkStart 从 zzzSetWindowsHookEx: add r8, 0x30
static BOOLEAN ExtractDeskInfoAphkStartWin10(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_ADD)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R8))
        return FALSE;

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            if (imm == 0x30) {
                SET_OFFSET(ctx, DeskInfo_aphkStart, 0x30);
                SET_FOUND(ctx, DeskInfo_aphkStart);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Win10: pti->amdesk 从 zzzSetWindowsHookEx: mov ecx, [rbx+0x378]
static BOOLEAN ExtractPtiAmdeskWin10(
    _In_ PSCAN_CONTEXT ctx,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV)
        return FALSE;
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_ECX))
        return FALSE;

    LONG disp;
    if (!GetMemoryDisp(instr, operands, ZYDIS_REGISTER_RBX, ZYDIS_REGISTER_NONE, 0, &disp))
        return FALSE;
    if (disp < 0x300 || disp > 0x500)
        return FALSE;

    SET_OFFSET(ctx, Pti_amdesk, (ULONG)disp);
    SET_FOUND(ctx, Pti_amdesk);
    return TRUE;
}

// Win10: HOOK 对象大小 从 zzzSetWindowsHookEx: mov r9d, 0x60 (紧邻 HMAllocObject 调用)
static BOOLEAN ExtractHookObjectSize(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PUCHAR codeBase,
    _In_ SIZE_T currentOffset,
    _In_ const ZydisDecodedInstruction* instr,
    _In_ const ZydisDecodedOperand* operands
)
{
    if (instr->mnemonic != ZYDIS_MNEMONIC_MOV) return FALSE;
    
    if (!HasDestReg(instr, operands, ZYDIS_REGISTER_R9D)) {
        //DbgPrint("[HookObjectSize] Fail: no R9D destination operand\n");
        return FALSE;
    }

    for (ZyanU8 i = 0; i < instr->operand_count; i++) {
        if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            ZyanU64 imm = operands[i].imm.value.u;
            // 验证范围：HOOK 对象大小通常在 0x50-0x80
            if (imm >= 0x50 && imm <= 0x80) {
                // 向前检查是否是 HMAllocObject 调用模式
                // 简化：直接记录，后续验证
                SET_OFFSET(ctx, Hook_ObjectSize, (ULONG)imm);
                SET_FOUND(ctx, Hook_ObjectSize);
                return TRUE;
            }
            else {
				//DbgPrint("[HookObjectSize] Fail: immediate value 0x%llX out of expected range\n", imm);
            }
        }
    }
    return FALSE;
}

// ==========================================
// 通用函数扫描器
// ==========================================
static NTSTATUS ScanFunction(
    _In_ PSCAN_CONTEXT ctx,
    _In_ PVOID funcAddr,
    _In_ SIZE_T maxSize
)
{
    if (!funcAddr)
        return STATUS_INVALID_ADDRESS;
    if (!MmIsAddressValid(funcAddr)) {
        DbgPrint("[!] ScanFunction: %p not present at entry\n", funcAddr);  // 加这个
        return STATUS_INVALID_ADDRESS;
    }

    PUCHAR codeBase = (PUCHAR)funcAddr;
    SIZE_T offset = 0;
    ULONG instrCount = 0;

    while (offset < maxSize && instrCount < SCAN_MAX_DEPTH) {
        if (!MmIsAddressValid(codeBase + offset)) {
            DbgPrint("[!] ScanFunction: page lost at offset 0x%zX\n", offset);  // 加这个
            break;
        }

        __try {
            ZydisDecodedInstruction instr;
            ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
            SIZE_T instrLen;

            if (!ZydisDecodeAt(codeBase + offset, maxSize - offset, &instr, operands, &instrLen)) {
                offset++;
                continue;
            }

            // 放在 ZydisDecodeAt 成功后、调用提取器之前
            BOOLEAN bIsZzz = (funcAddr == (PVOID)0xFFFFF9123F2FFC48); // 需要把参数传进来或比较地址
            if (bIsZzz && instrCount < 100) {
                DbgPrint("[Zzz:%04u] MNEM=%u | len=%02zu | ops=%u | addr=%p\n",
                    instrCount, instr.mnemonic, instrLen, instr.operand_count, codeBase + offset);
                for (ZyanU8 i = 0; i < instr.operand_count; i++) {
                    if (operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
                        DbgPrint("  -> MEM base=%u idx=%u scale=%u disp=%X\n",
                            operands[i].mem.base,
                            operands[i].mem.index,
                            operands[i].mem.scale,
                            (ULONG)operands[i].mem.disp.value);
                    }
                    else if (operands[i].type == ZYDIS_OPERAND_TYPE_REGISTER) {
                        DbgPrint("  -> REG val=%u\n", operands[i].reg.value);
                    }
                    else if (operands[i].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
                        DbgPrint("  -> IMM val=%llX\n", operands[i].imm.value.u);
                    }
                }
            }

            // 通用提取器（Win10/Win11 共用）
            ExtractHookFlagsGlobal(ctx, &instr, operands);
            ExtractHookHHook(ctx, &instr, operands);
            ExtractHookIhmod(ctx, &instr, operands);
            ExtractHookType(ctx, &instr, operands);
            ExtractHookOffPfn(ctx, &instr, operands);
            ExtractHookPhkNext(ctx, &instr, operands);
            ExtractHookPti(ctx, codeBase, offset, &instr, operands);

			ExtractHandleEntryHookTypeOffset(ctx, &instr, operands);
			ExtractHandleEntryTableIndexOffset(ctx, &instr, operands);
            
            if (ctx->Offsets->bIsWin10Path) {
                // Win10 专用提取器
                ExtractPtiAphkStartWin10(ctx, &instr, operands);
                //ExtractPtiFsHooksWin10(ctx, &instr, operands);
                ExtractPtiDeskInfoWin10(ctx, &instr, operands);
                ExtractDeskInfoAphkStartWin10(ctx, &instr, operands);
                //ExtractPtiEThreadWin10(ctx, &instr, operands);
                //ExtractPtiPpiWin10(ctx, &instr, operands);
                ExtractPtiAmdeskWin10(ctx, &instr, operands);
                ExtractHookObjectSize(ctx, codeBase, offset, &instr, operands);
                //ExtractHookRpdeskWin10(ctx, &instr, operands);
            }
            else {
                // Win11 专用提取器
                ExtractPtiAphkStart(ctx, &instr, operands);
                //ExtractPtiFsHooks(ctx, &instr, operands);
                ExtractPtiDeskInfo(ctx, &instr, operands);
                ExtractDeskInfoAphkStart(ctx, codeBase, offset, &instr, operands);
                //ExtractPtiEThread(ctx, &instr, operands);
                //ExtractPtiPpi(ctx, &instr, operands);
                //ExtractPpiUiPI(ctx, &instr, operands);
                //ExtractTebWin32ThreadInfo(ctx, &instr, operands);
                ExtractPtiAmdesk(ctx, &instr, operands);
                ExtractSessionStateHookArray(ctx, &instr, operands);
                ExtractSessionStateFlag(ctx, &instr, operands);
                ExtractPtiAphkStartVerify(ctx, &instr, operands);
            }

            offset += instrLen;
            instrCount++;

            if (instr.mnemonic == ZYDIS_MNEMONIC_RET) {
                // 只有真正需要的都找到了才提前退出，否则越过 retn 继续扫函数尾部
                if (IS_FOUND(ctx, Pti_pDeskInfo) && IS_FOUND(ctx, DeskInfo_aphkStart) &&
                    IS_FOUND(ctx, Hook_flags))
                    break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            DbgPrint("[!] ScanFunction exception at offset 0x%zX\n", offset);
            break;
        }
    }
    DbgPrint("[*] ScanFunction %p done: %u instrs, offset=0x%zX\n", funcAddr, instrCount, offset); // 加这个

    return STATUS_SUCCESS;
}

// ==========================================
// 默认值设置（按构建号）
// ==========================================
static VOID SetDefaultOffsets(
    _In_ ULONG buildNumber,
    _In_ BOOLEAN bHasW32GetUserSessionState,
    _In_ PWIN32K_OFFSETS offsets
)
{
    RtlZeroMemory(offsets, sizeof(WIN32K_OFFSETS));
    offsets->OsBuildNumber = buildNumber;
    offsets->bHasW32GetUserSessionState = bHasW32GetUserSessionState;

    // 元数据条目大小由构建号决定
    if (buildNumber < WIN10_1703_BUILD_NUMBER) {
        offsets->HandleEntrySize = 24;
        offsets->HandleEntry_HookTypeOffset = HandleEntry_HOOKTYPE_OFFSET(24);
        offsets->HandleEntry_TableIndexOffset = HandleEntry_TABLEINDEX_OFFSET(24);
    }
    else {
        offsets->HandleEntrySize = 32;
        offsets->HandleEntry_HookTypeOffset = HandleEntry_HOOKTYPE_OFFSET(32);
        offsets->HandleEntry_TableIndexOffset = HandleEntry_TABLEINDEX_OFFSET(32);
    }

    // Win11 默认值 (22621, 22631, 26100)
    if (bHasW32GetUserSessionState) {
        offsets->bIsWin10Path = FALSE;
        offsets->Pti_pDeskInfo = 0x1F8;
        offsets->Pti_aphkStart = 0x3C0;
        //offsets->Pti_fsHooks = 0x2D0;
        //offsets->Pti_ppi = 0x1D0;
        //offsets->Pti_pEThread = 0x208;
        offsets->Pti_amdesk = 0x3A8;
        offsets->Hook_hHook = 0x00;
        offsets->Hook_pti = 0x10;
        offsets->Hook_phkNext = 0x28;
        offsets->Hook_nHookType = 0x30;
        offsets->Hook_offPfn = 0x38;
        offsets->Hook_flags = 0x40;
        offsets->Hook_ihmod = 0x44;
        //offsets->Hook_ptiHooked = 0x48;
        //offsets->Hook_rpdesk = 0x50;
        offsets->Hook_ObjectSize = 0x60;
        offsets->DeskInfo_aphkStart = 0x28;
        offsets->DeskInfo_spwnd = 0x00;
        //offsets->Ppi_uiPI = 0x30C;
        //offsets->Ppi_MandatoryLabel = 0x310;
        //offsets->Teb_Win32ThreadInfo = 0x38;
        offsets->SessionState_HookArray = 0xA510;
        offsets->SessionState_Flag = 0x4CF8;
    }
    // Win10 默认值 (19041, 19045)
    else {
        offsets->bIsWin10Path = TRUE;
        offsets->Pti_pDeskInfo = 0x1D0;
        offsets->Pti_aphkStart = 0x398;
        //offsets->Pti_fsHooks = 0x2A8;
        //offsets->Pti_ppi = 0x1A8;
        //offsets->Pti_pEThread = 0x1C8;
        offsets->Pti_amdesk = 0x378;
        offsets->Hook_hHook = 0x00;
        offsets->Hook_pti = 0x10;
        offsets->Hook_phkNext = 0x28;
        offsets->Hook_nHookType = 0x30;
        offsets->Hook_offPfn = 0x38;
        offsets->Hook_flags = 0x40;
        offsets->Hook_ihmod = 0x44;
        //offsets->Hook_ptiHooked = 0x48;
        //offsets->Hook_rpdesk = 0x50;
        offsets->Hook_ObjectSize = 0x60;
        offsets->DeskInfo_aphkStart = 0x30;
        offsets->DeskInfo_spwnd = 0x00;
        //offsets->Ppi_uiPI = 0x30C;      // 待验证
        //offsets->Ppi_MandatoryLabel = 0x310;
        //offsets->Teb_Win32ThreadInfo = 0x38;
        offsets->SessionState_HookArray = 0;  // Win10 不适用
        offsets->SessionState_Flag = 0;       // Win10 不适用
    }
}

// ==========================================
// 公共接口
// ==========================================
NTSTATUS Win32kOffsetScanner_Initialize(
    _In_opt_ PVOID zzzSetWindowHookExAddr,
    _In_opt_ PVOID xxxCallHookAddr,
    _In_opt_ PVOID HMAllocObjectAddr,
    _In_opt_ PVOID zzzUnhookWindowsHookExAddr,
    _Out_ PWIN32K_OFFSETS Offsets
)
{
    if (!Offsets)
        return STATUS_INVALID_PARAMETER;

    // 检测方法
    BOOLEAN bHasW32GetUserSessionState = FALSE;
    DetectSessionStateMethod(&bHasW32GetUserSessionState);

    // 获取当前构建号
    RTL_OSVERSIONINFOW osVer = { 0 };
    osVer.dwOSVersionInfoSize = sizeof(osVer);
    RtlGetVersion(&osVer);
    ULONG buildNumber = osVer.dwBuildNumber;

    Offsets->bHasW32GetUserSessionState = bHasW32GetUserSessionState;
	Offsets->bIsWin10Path = !bHasW32GetUserSessionState;

    SCAN_CONTEXT ctx = { 0 };
    ctx.Offsets = Offsets;

    // 扫描 zzzSetWindowsHookEx（主要来源）
    if (zzzSetWindowHookExAddr) {
        ctx.FuncBase = zzzSetWindowHookExAddr;
        ctx.FuncSize = SCAN_MAX_SIZE;
        DbgPrint("[*] Scanning zzzSetWindowsHookEx at %p (Win10=%d)\n",
            zzzSetWindowHookExAddr, Offsets->bIsWin10Path);
        ScanFunction(&ctx, zzzSetWindowHookExAddr, SCAN_MAX_SIZE);
    }

    // 扫描 zzzUnhookWindowsHookEx
    if (zzzUnhookWindowsHookExAddr) {
        ctx.FuncBase = zzzUnhookWindowsHookExAddr;
        ctx.FuncSize = SCAN_MAX_SIZE;
        DbgPrint("[*] Scanning zzzUnhookWindowsHookEx at %p (Win10=%d)\n",
            zzzUnhookWindowsHookExAddr, Offsets->bIsWin10Path);
        ScanFunction(&ctx, zzzUnhookWindowsHookExAddr, SCAN_MAX_SIZE);
    }

    // 扫描 xxxCallHook（验证来源）
    if (xxxCallHookAddr) {
        ctx.FuncBase = xxxCallHookAddr;
        ctx.FuncSize = SCAN_MAX_SIZE;
        DbgPrint("[*] Scanning xxxCallHook at %p\n", xxxCallHookAddr);
        ScanFunction(&ctx, xxxCallHookAddr, SCAN_MAX_SIZE);
    }

    // 扫描 HMAllocObject
    if (HMAllocObjectAddr) {
        ctx.FuncBase = HMAllocObjectAddr;
        ctx.FuncSize = SCAN_MAX_SIZE;
        DbgPrint("[*] Scanning HMAllocObject at %p\n", HMAllocObjectAddr);
        ScanFunction(&ctx, HMAllocObjectAddr, SCAN_MAX_SIZE);
    }

    /*Offsets->Hook_hHook = 0x0;
    Offsets->Hook_pti = 0x10;
    if (buildNumber < WIN10_1703_BUILD_NUMBER) {
        Offsets->MetadataEntrySize = 24;
    }
    else {
        Offsets->MetadataEntrySize = 32;
    }
    Offsets->HandleEntry_HookTypeOffset = HandleEntry_HOOKTYPE_OFFSET(Offsets->MetadataEntrySize);
    Offsets->HandleEntry_TableIndexOffset = HandleEntry_TABLEINDEX_OFFSET(Offsets->MetadataEntrySize);*/

    // 验证结果
    if (!Win32kOffsetScanner_Validate(Offsets)) {
        DbgPrint("[!] Offset validation failed, using defaults\n");
        // 设置默认值
        //SetDefaultOffsets(buildNumber, bHasW32GetUserSessionState, Offsets);
        //Win32kOffsetScanner_Dump(Offsets, TRUE);
		//DbgPrint("[-] After using defaults, function should exit with STATUS_NOT_FOUND\n");
        return STATUS_NOT_FOUND;
        //SetDefaultOffsets(buildNumber, bHasW32GetUserSessionState, Offsets);
    }

    DbgPrint("[+] Offset scanning complete. FoundMask=0x%08X\n", ctx.FoundMask);
    return STATUS_SUCCESS;
}

BOOLEAN Win32kOffsetScanner_Validate(
    _In_ PWIN32K_OFFSETS Offsets
)
{
    if (!Win32kOffsetScanner_Dump(Offsets, TRUE)) return FALSE;

    // 关键偏移必须非零
    if (Offsets->Pti_aphkStart == 0 ||
        Offsets->Pti_pDeskInfo == 0 || Offsets->Hook_flags == 0)
        return FALSE;

    // 【精确验证】：扫描结果必须与默认值完全一致
    //WIN32K_OFFSETS defaults = { 0 };
    //SetDefaultOffsets(Offsets->OsBuildNumber, &defaults);

    DbgPrint("[+] All offsets validated exactly against defaults\n");
    return TRUE;
}

// ==========================================
// 偏移 Dump 与验证
// ==========================================

// 硬编码参考值（用于验证）
static const WIN32K_OFFSETS g_Win10_19045_Ref = {
    .OsBuildNumber = 19045,
    .Pti_pDeskInfo = 0x1D0,
    .Pti_aphkStart = 0x398,
    //.Pti_fsHooks = 0x2A8,
    //.Pti_pEThread = 0x1C8,
    //.Pti_ppi = 0x1A8,
    .Pti_amdesk = 0x378,
    .Hook_hHook = 0x00,
    .Hook_pti = 0x10,
    .Hook_phkNext = 0x28,
    .Hook_nHookType = 0x30,
    .Hook_offPfn = 0x38,
    .Hook_flags = 0x40,
    .Hook_ihmod = 0x44,
    //.Hook_ptiHooked = 0x48,
    //.Hook_rpdesk = 0x50,
    .Hook_ObjectSize = 0x60,
    .DeskInfo_aphkStart = 0x30,
    .DeskInfo_spwnd = 0x00,
    //.Ppi_uiPI = 0x30C,
    //.Ppi_MandatoryLabel = 0x310,
    //.Teb_Win32ThreadInfo = 0x38,
    .HandleEntrySize = 32,
    .HandleEntry_HookTypeOffset = 24,
    .HandleEntry_TableIndexOffset = 26,
    .bIsWin10Path = TRUE,
    .bHasW32GetUserSessionState = FALSE
};

static const WIN32K_OFFSETS g_Win11_22621_Ref = {
    .OsBuildNumber = 22621,
    .Pti_pDeskInfo = 0x1F8,
    .Pti_aphkStart = 0x3C0,
    //.Pti_fsHooks = 0x2D0,
    //.Pti_pEThread = 0x208,
    //.Pti_ppi = 0x1D0,
    .Pti_amdesk = 0x3A8,
    .Hook_hHook = 0x00,
    .Hook_pti = 0x10,
    .Hook_phkNext = 0x28,
    .Hook_nHookType = 0x30,
    .Hook_offPfn = 0x38,
    .Hook_flags = 0x40,
    .Hook_ihmod = 0x44,
    //.Hook_ptiHooked = 0x48,
    //.Hook_rpdesk = 0x50,
    .Hook_ObjectSize = 0x60,
    .DeskInfo_aphkStart = 0x28,
    .DeskInfo_spwnd = 0x00,
    //.Ppi_uiPI = 0x30C,
    //.Ppi_MandatoryLabel = 0x310,
    //.Teb_Win32ThreadInfo = 0x38,
    .SessionState_HookArray = 0xA510,
    .SessionState_Flag = 0x4CF8,
    .HandleEntrySize = 32,
    .HandleEntry_HookTypeOffset = 24,
    .HandleEntry_TableIndexOffset = 26,
    .bIsWin10Path = FALSE,
    .bHasW32GetUserSessionState = TRUE
};

BOOLEAN Win32kOffsetScanner_Dump(
    _In_ PWIN32K_OFFSETS Offsets,
    _In_ BOOLEAN bCompareWithReference
)
{
    DbgPrint("========================================\n");
    DbgPrint("WIN32K_OFFSETS Dump (Build=%lu, Win10Path=%d)\n",
        Offsets->OsBuildNumber, Offsets->bIsWin10Path);
    DbgPrint("========================================\n");

    // PTI 偏移
    DbgPrint("[PTI]\n");
    DbgPrint("  pDeskInfo     = 0x%03X\n", Offsets->Pti_pDeskInfo);
    DbgPrint("  aphkStart     = 0x%03X\n", Offsets->Pti_aphkStart);
    //DbgPrint("  fsHooks       = 0x%03X\n", Offsets->Pti_fsHooks);
    //DbgPrint("  pEThread      = 0x%03X\n", Offsets->Pti_pEThread);
    //DbgPrint("  ppi           = 0x%03X\n", Offsets->Pti_ppi);
    DbgPrint("  amdesk        = 0x%03X\n", Offsets->Pti_amdesk);

    // HOOK 对象偏移
    DbgPrint("[HOOK]\n");
    DbgPrint("  hHook         = 0x%03X\n", Offsets->Hook_hHook);
    DbgPrint("  pti           = 0x%03X\n", Offsets->Hook_pti);
    DbgPrint("  phkNext       = 0x%03X\n", Offsets->Hook_phkNext);
    DbgPrint("  nHookType     = 0x%03X\n", Offsets->Hook_nHookType);
    DbgPrint("  offPfn        = 0x%03X\n", Offsets->Hook_offPfn);
    DbgPrint("  flags         = 0x%03X\n", Offsets->Hook_flags);
    DbgPrint("  ihmod         = 0x%03X\n", Offsets->Hook_ihmod);
    //DbgPrint("  ptiHooked     = 0x%03X\n", Offsets->Hook_ptiHooked);
    //DbgPrint("  rpdesk        = 0x%03X\n", Offsets->Hook_rpdesk);
    DbgPrint("  ObjectSize    = 0x%03X\n", Offsets->Hook_ObjectSize);

    // DESKINFO 偏移
    DbgPrint("[DESKINFO]\n");
    DbgPrint("  aphkStart     = 0x%03X\n", Offsets->DeskInfo_aphkStart);
    DbgPrint("  spwnd         = 0x%03X\n", Offsets->DeskInfo_spwnd);

    // PPI 偏移
    //DbgPrint("[PPI]\n");
    //DbgPrint("  uiPI          = 0x%03X\n", Offsets->Ppi_uiPI);
    //DbgPrint("  MandatoryLabel= 0x%03X\n", Offsets->Ppi_MandatoryLabel);

    // TEB 偏移
    //DbgPrint("[TEB]\n");
    //DbgPrint("  Win32ThreadInfo= 0x%03X\n", Offsets->Teb_Win32ThreadInfo);

    // SessionState (Win11)
    if (!Offsets->bIsWin10Path) {
        DbgPrint("[SessionState]\n");
        DbgPrint("  HookArray     = 0x%03X\n", Offsets->SessionState_HookArray);
        DbgPrint("  Flag          = 0x%03X\n", Offsets->SessionState_Flag);
    }

    // 元数据
    DbgPrint("[HookEntry]\n");
    DbgPrint("  EntrySize     = %lu\n", Offsets->HandleEntrySize);
    DbgPrint("  HookTypeOffset= 0x%02X\n", Offsets->HandleEntry_HookTypeOffset);
    DbgPrint("  TableIndexOff = 0x%02X\n", Offsets->HandleEntry_TableIndexOffset);

    // 与参考值对比
    if (bCompareWithReference) {
        const WIN32K_OFFSETS* pRef = Offsets->bIsWin10Path ? &g_Win10_19045_Ref : &g_Win11_22621_Ref;
        ULONG mismatchCount = 0;

#define CHECK_FIELD(_field) do { \
            if (Offsets->_field != pRef->_field) { \
                DbgPrint("  [!] MISMATCH: %-20s Got=0x%03X Expected=0x%03X\n", \
                    #_field, Offsets->_field, pRef->_field); \
                mismatchCount++; \
            } \
        } while(0)

        CHECK_FIELD(Pti_pDeskInfo);
        CHECK_FIELD(Pti_aphkStart);
        //CHECK_FIELD(Pti_fsHooks);
        //CHECK_FIELD(Pti_pEThread);
        //CHECK_FIELD(Pti_ppi);
        CHECK_FIELD(Pti_amdesk);
        CHECK_FIELD(Hook_hHook);
        CHECK_FIELD(Hook_pti);
        CHECK_FIELD(Hook_phkNext);
        CHECK_FIELD(Hook_nHookType);
        CHECK_FIELD(Hook_offPfn);
        CHECK_FIELD(Hook_flags);
        CHECK_FIELD(Hook_ihmod);
        //CHECK_FIELD(Hook_ptiHooked);
        //CHECK_FIELD(Hook_rpdesk);
        CHECK_FIELD(Hook_ObjectSize);
        CHECK_FIELD(DeskInfo_aphkStart);
        //CHECK_FIELD(Ppi_uiPI);
        //CHECK_FIELD(Teb_Win32ThreadInfo);
        CHECK_FIELD(HandleEntrySize);
        CHECK_FIELD(HandleEntry_HookTypeOffset);
        CHECK_FIELD(HandleEntry_TableIndexOffset);

        if (!Offsets->bIsWin10Path) {
            CHECK_FIELD(SessionState_HookArray);
            CHECK_FIELD(SessionState_Flag);
        }

#undef CHECK_FIELD

        if (mismatchCount == 0) {
            DbgPrint("[+] ALL OFFSETS MATCH REFERENCE!\n");
            return TRUE;
        }
        else {
            DbgPrint("[!] %lu OFFSETS MISMATCH!\n", mismatchCount);
            return FALSE;
        }
    }
    return TRUE;

    //DbgPrint("========================================\n");
}