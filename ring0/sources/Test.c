// Zydis 静态编译配置（必须在包含 Zydis.h 之前）
#define ZYAN_NO_LIBC
#include "Test.h"
#include "Zydis.h"

// ============================================================
// 类型定义
// ============================================================

// 基础字节级特征码（传统 AOB Scan）
typedef struct _BYTE_SIGNATURE {
    PUCHAR  Pattern;      // 字节模式，如 "\x48\x89\x5C\x24\x00\x57"
    PCHAR   Mask;         // 掩码，如 "xxxx?x" (x=匹配, ?=忽略/通配)
    SIZE_T  Length;
    PCHAR   Name;         // 特征名，用于日志
} BYTE_SIGNATURE, * PBYTE_SIGNATURE;

// 指令级特征：匹配特定指令语义（抗代码变形/寄存器重命名）
typedef struct _INSTRUCTION_SIG {
    ZydisMnemonic       Mnemonic;           // 指令助记符
    ZydisOperandType    Op1Type;            // 操作数1类型约束
    ZydisOperandType    Op2Type;            // 操作数2类型约束
    ZyanBool            CheckOp1Size;       // 是否检查操作数1大小
    ZyanU16             Op1Size;            // 期望的操作数1大小（位）
    ZyanBool            CheckOp2Size;
    ZyanU16             Op2Size;
} INSTRUCTION_SIG, * PINSTRUCTION_SIG;

// 指令链特征：匹配多指令序列（如函数序言、特定控制流模式）
typedef struct _INSTRUCTION_CHAIN {
    PINSTRUCTION_SIG    Signatures;
    SIZE_T              Count;
    PCHAR               Name;
} INSTRUCTION_CHAIN, * PINSTRUCTION_CHAIN;

// 匹配结果
typedef struct _MATCH_RESULT {
    PUCHAR              Address;            // 匹配到的地址
    SIZE_T              MatchLength;        // 匹配的字节长度
    PCHAR               SignatureName;      // 匹配的特征名
    ZyanBool            IsInstructionMatch;   // 是否是指令级匹配
} MATCH_RESULT, * PMATCH_RESULT;

// ============================================================
// 全局解码器（DriverEntry 中初始化一次）
// ============================================================
ZydisDecoder g_ZydisDecoder;
ZyanBool g_ZydisInitialized = ZYAN_FALSE;

// ============================================================
// 初始化 Zydis 解码器
// ============================================================
NTSTATUS ZydisMatchEngineInit(VOID)
{
    ZyanStatus status;

    // 64位系统使用 LONG_64 模式
    status = ZydisDecoderInit(&g_ZydisDecoder,
        ZYDIS_MACHINE_MODE_LONG_64,
        ZYDIS_STACK_WIDTH_64);

    if (ZYAN_SUCCESS(status)) {
        g_ZydisInitialized = ZYAN_TRUE;
        DbgPrint("[Zydis] 解码器初始化成功\n");
        return STATUS_SUCCESS;
    }

    DbgPrint("[Zydis] 解码器初始化失败: 0x%X\n", status);
    return STATUS_UNSUCCESSFUL;
}

// ============================================================
// 1. 基础字节级特征码扫描（传统 AOB）
// ============================================================
PUCHAR SigScanBytes(
    _In_ PUCHAR Start,
    _In_ SIZE_T Size,
    _In_ PBYTE_SIGNATURE Sig)
{
    if (!Sig || !Sig->Pattern || !Sig->Mask || Sig->Length == 0)
        return NULL;

    if (Size < Sig->Length)
        return NULL;

    for (SIZE_T i = 0; i <= Size - Sig->Length; i++) {
        BOOLEAN match = TRUE;
        for (SIZE_T j = 0; j < Sig->Length; j++) {
            if (Sig->Mask[j] == 'x' && Start[i + j] != Sig->Pattern[j]) {
                match = FALSE;
                break;
            }
        }
        if (match) {
            DbgPrint("[Zydis] 字节特征命中: %s @ %p\n", Sig->Name, &Start[i]);
            return &Start[i];
        }
    }
    return NULL;
}

// ============================================================
// 2. 单条指令语义匹配（抗代码变形）
// ============================================================
ZyanBool MatchSingleInstruction(
    _In_ PUCHAR Code,
    _In_ SIZE_T CodeLen,
    _In_ PINSTRUCTION_SIG Sig,
    _Out_opt_ ZydisDecodedInstruction* OutInst,
    _Out_opt_ ZyanU8* OutInstLen)
{
    ZydisDecodedInstruction inst;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    ZyanStatus status;

    if (!g_ZydisInitialized)
        return ZYAN_FALSE;

    // 使用 DecodeFull 获取操作数信息
    status = ZydisDecoderDecodeFull(&g_ZydisDecoder, Code, CodeLen, &inst, operands);
    if (!ZYAN_SUCCESS(status))
        return ZYAN_FALSE;

    // 匹配助记符
    if (inst.mnemonic != Sig->Mnemonic)
        return ZYAN_FALSE;

    // 匹配操作数1类型
    if (Sig->Op1Type != ZYDIS_OPERAND_TYPE_UNUSED) {
        if (inst.operand_count < 1)
            return ZYAN_FALSE;
        if (operands[0].type != Sig->Op1Type)
            return ZYAN_FALSE;
        if (Sig->CheckOp1Size && operands[0].size != Sig->Op1Size)
            return ZYAN_FALSE;
    }

    // 匹配操作数2类型
    if (Sig->Op2Type != ZYDIS_OPERAND_TYPE_UNUSED) {
        if (inst.operand_count < 2)
            return ZYAN_FALSE;
        if (operands[1].type != Sig->Op2Type)
            return ZYAN_FALSE;
        if (Sig->CheckOp2Size && operands[1].size != Sig->Op2Size)
            return ZYAN_FALSE;
    }

    if (OutInst) *OutInst = inst;
    if (OutInstLen) *OutInstLen = inst.length;
    return ZYAN_TRUE;
}

// ============================================================
// 3. 指令链扫描（跨多条指令的语义匹配）
// ============================================================
PUCHAR SigScanInstructionChain(
    _In_ PUCHAR Start,
    _In_ SIZE_T Size,
    _In_ PINSTRUCTION_CHAIN Chain)
{
    SIZE_T offset = 0;
    SIZE_T chainIdx = 0;
    PUCHAR chainStart = NULL;

    if (!Chain || Chain->Count == 0 || !g_ZydisInitialized)
        return NULL;

    while (offset < Size) {
        ZydisDecodedInstruction inst;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        ZyanStatus status = ZydisDecoderDecodeFull(
            &g_ZydisDecoder,
            Start + offset,
            Size - offset,
            &inst,
            operands);

        if (!ZYAN_SUCCESS(status)) {
            // 解码失败，逐字节偏移重试（处理指令边界对齐问题）
            offset++;
            chainIdx = 0;
            chainStart = NULL;
            continue;
        }

        PINSTRUCTION_SIG sig = &Chain->Signatures[chainIdx];

        // 检查当前指令是否匹配链中的第 chainIdx 个特征
        ZyanBool typeMatch = ZYAN_TRUE;

        // 匹配助记符
        if (inst.mnemonic != sig->Mnemonic) {
            typeMatch = ZYAN_FALSE;
        }
        else {
            // 匹配操作数1类型
            if (sig->Op1Type != ZYDIS_OPERAND_TYPE_UNUSED) {
                if (inst.operand_count < 1 || operands[0].type != sig->Op1Type)
                    typeMatch = ZYAN_FALSE;
                else if (sig->CheckOp1Size && operands[0].size != sig->Op1Size)
                    typeMatch = ZYAN_FALSE;
            }
            // 匹配操作数2类型
            if (typeMatch && sig->Op2Type != ZYDIS_OPERAND_TYPE_UNUSED) {
                if (inst.operand_count < 2 || operands[1].type != sig->Op2Type)
                    typeMatch = ZYAN_FALSE;
                else if (sig->CheckOp2Size && operands[1].size != sig->Op2Size)
                    typeMatch = ZYAN_FALSE;
            }
        }

        if (typeMatch) {
            if (chainIdx == 0)
                chainStart = Start + offset;

            chainIdx++;
            if (chainIdx >= Chain->Count) {
                DbgPrint("[Zydis] 指令链命中: %s @ %p\n",
                    Chain->Name, chainStart);
                return chainStart;
            }
        }
        else {
            // 关键修正：回退逻辑
            // 如果当前指令不匹配链中的当前位置，检查它是否匹配链的第一个元素
            // 这样可以处理重叠模式
            if (chainIdx > 0) {
                // 先回退到开头，检查当前指令是否匹配第一个特征
                chainIdx = 0;
                chainStart = NULL;

                // 重新检查当前指令是否匹配第一个特征
                PINSTRUCTION_SIG firstSig = &Chain->Signatures[0];
                ZyanBool firstMatch = ZYAN_TRUE;

                if (inst.mnemonic != firstSig->Mnemonic) {
                    firstMatch = ZYAN_FALSE;
                }
                else {
                    if (firstSig->Op1Type != ZYDIS_OPERAND_TYPE_UNUSED) {
                        if (inst.operand_count < 1 || operands[0].type != firstSig->Op1Type)
                            firstMatch = ZYAN_FALSE;
                        else if (firstSig->CheckOp1Size && operands[0].size != firstSig->Op1Size)
                            firstMatch = ZYAN_FALSE;
                    }
                    if (firstMatch && firstSig->Op2Type != ZYDIS_OPERAND_TYPE_UNUSED) {
                        if (inst.operand_count < 2 || operands[1].type != firstSig->Op2Type)
                            firstMatch = ZYAN_FALSE;
                        else if (firstSig->CheckOp2Size && operands[1].size != firstSig->Op2Size)
                            firstMatch = ZYAN_FALSE;
                    }
                }

                if (firstMatch) {
                    chainStart = Start + offset;
                    chainIdx = 1;
                }
            }
        }

        offset += inst.length;
    }
    return NULL;
}

// ============================================================
// 4. 进阶：指令上下文分析（命中点后反汇编周围代码）
// ============================================================
VOID AnalyzeHitContext(
    _In_ PUCHAR HitAddress,
    _In_ SIZE_T ContextSize,      // 前后分析的字节数
    _In_ PUCHAR RegionStart,
    _In_ SIZE_T RegionSize)
{
    // 修正：使用指针运算确保边界安全
    PUCHAR start;
    SIZE_T size;
    SIZE_T offset = 0;

    // 计算安全的起始地址
    if ((ULONG_PTR)HitAddress >= (ULONG_PTR)RegionStart + ContextSize) {
        start = HitAddress - ContextSize;
    }
    else {
        start = RegionStart;
    }

    // 计算安全的大小
    size = ContextSize * 2;
    if ((ULONG_PTR)start + size > (ULONG_PTR)RegionStart + RegionSize) {
        size = (RegionStart + RegionSize) - start;
    }

    DbgPrint("[Zydis] ===== 命中点上下文分析 =====\n");
    DbgPrint("[Zydis] 命中地址: %p\n", HitAddress);

    while (offset < size && (start + offset) < (RegionStart + RegionSize)) {
        ZydisDecodedInstruction inst;
        ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
        ZyanStatus status = ZydisDecoderDecodeFull(
            &g_ZydisDecoder,
            start + offset,
            size - offset,
            &inst,
            operands);

        if (!ZYAN_SUCCESS(status)) {
            DbgPrint("[Zydis]   %p: 解码失败 (字节: 0x%02X)\n",
                start + offset, *(PUCHAR)(start + offset));
            offset++;
            continue;
        }

        PUCHAR addr = start + offset;
        if (addr == HitAddress)
            DbgPrint("[Zydis] >>> 命中点 <<<\n");

        // 分析特定指令语义
        if (inst.mnemonic == ZYDIS_MNEMONIC_CALL) {
            DbgPrint("[Zydis]   %p: CALL 指令 (目标分析...)\n", addr);
            // 可进一步解析调用目标地址
        }
        else if (inst.mnemonic == ZYDIS_MNEMONIC_JMP) {
            DbgPrint("[Zydis]   %p: JMP 指令\n", addr);
        }
        else if (inst.mnemonic == ZYDIS_MNEMONIC_MOV &&
            inst.operand_count_visible >= 2 &&
            operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER &&
            operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE) {
            DbgPrint("[Zydis]   %p: MOV reg, imm (常量加载)\n", addr);
        }

        offset += inst.length;
    }
    DbgPrint("[Zydis] ===== 分析结束 =====\n");
}

// ============================================================
// 5. 综合扫描函数：同时执行字节扫描和指令级扫描
// ============================================================
NTSTATUS ComprehensiveSigScan(
    _In_ PUCHAR Start,
    _In_ SIZE_T Size,
    _In_ PBYTE_SIGNATURE ByteSigs,
    _In_ SIZE_T ByteSigCount,
    _In_ PINSTRUCTION_CHAIN InstChains,
    _In_ SIZE_T InstChainCount,
    _Out_writes_opt_(MaxResults) PMATCH_RESULT Results,
    _In_ SIZE_T MaxResults,
    _Out_ PSIZE_T ResultCount)
{
    SIZE_T found = 0;

    if (!g_ZydisInitialized)
        return STATUS_UNSUCCESSFUL;

    // --- 阶段1: 字节级特征扫描 ---
    for (SIZE_T i = 0; i < ByteSigCount && found < MaxResults; i++) {
        PUCHAR hit = SigScanBytes(Start, Size, &ByteSigs[i]);
        if (hit) {
            Results[found].Address = hit;
            Results[found].MatchLength = ByteSigs[i].Length;
            Results[found].SignatureName = ByteSigs[i].Name;
            Results[found].IsInstructionMatch = ZYAN_FALSE;
            found++;

            // 进阶：对命中点进行指令上下文分析
            AnalyzeHitContext(hit, 0x40, Start, Size);
        }
    }

    // --- 阶段2: 指令链特征扫描 ---
    for (SIZE_T i = 0; i < InstChainCount && found < MaxResults; i++) {
        PUCHAR hit = SigScanInstructionChain(Start, Size, &InstChains[i]);
        if (hit) {
            Results[found].Address = hit;
            Results[found].MatchLength = 0; // 指令链长度不固定
            Results[found].SignatureName = InstChains[i].Name;
            Results[found].IsInstructionMatch = ZYAN_TRUE;
            found++;
        }
    }

    *ResultCount = found;
    return found > 0 ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

// ============================================================
// 使用示例
// ============================================================
VOID ExampleUsage(PDRIVER_OBJECT DriverObj)
{
    // 假设要扫描驱动自身的代码段
    PUCHAR codeBase = (PUCHAR)DriverObj->DriverStart;
    SIZE_T codeSize = DriverObj->DriverSize;

    // --- 示例1: 传统字节特征（匹配 push rbp; mov rbp, rsp）
    BYTE_SIGNATURE byteSigs[] = {
        {
            .Pattern = (PUCHAR)"\x55\x48\x8B\xEC",  // push rbp; mov rbp, rsp
            .Mask = "xxxx",
            .Length = 4,
            .Name = "x64函数序言"
        },
        {
            .Pattern = (PUCHAR)"\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57",
            .Mask = "xxxx?xxxx?x",
            .Length = 11,
            .Name = "保存寄存器+对齐"
        }
    };

    // --- 示例2: 指令级特征（抗变形）
    // 匹配 "mov reg64, [mem64]" 这种模式，不管具体寄存器是什么
    /*INSTRUCTION_SIG movRegMem = {
        .Mnemonic = ZYDIS_MNEMONIC_MOV,
        .Op1Type = ZYDIS_OPERAND_TYPE_REGISTER,
        .Op2Type = ZYDIS_OPERAND_TYPE_MEMORY,
        .CheckOp1Size = ZYAN_TRUE,
        .Op1Size = 64
    };*/

    // --- 示例3: 指令链特征（函数序言后的首个调用模式）
    // 修正：CALL 的操作数类型改为 UNUSED，使其更通用
    INSTRUCTION_SIG prologueChainSigs[] = {
        { ZYDIS_MNEMONIC_PUSH, ZYDIS_OPERAND_TYPE_REGISTER, ZYDIS_OPERAND_TYPE_UNUSED },
        { ZYDIS_MNEMONIC_PUSH, ZYDIS_OPERAND_TYPE_REGISTER, ZYDIS_OPERAND_TYPE_UNUSED },
        { ZYDIS_MNEMONIC_SUB,  ZYDIS_OPERAND_TYPE_REGISTER, ZYDIS_OPERAND_TYPE_IMMEDIATE },
        { ZYDIS_MNEMONIC_CALL, ZYDIS_OPERAND_TYPE_UNUSED,   ZYDIS_OPERAND_TYPE_UNUSED }
    };
    INSTRUCTION_CHAIN prologueChain = {
        .Signatures = prologueChainSigs,
        .Count = 4,
        .Name = "函数序言+调用模式"
    };

    // --- 执行扫描 ---
    MATCH_RESULT results[10];
    SIZE_T resultCount = 0;

    NTSTATUS status = ComprehensiveSigScan(
        codeBase, codeSize,
        byteSigs, 2,
        &prologueChain, 1,
        results, 10, &resultCount);

    if (NT_SUCCESS(status)) {
        DbgPrint("[Zydis] 找到 %zu 个匹配\n", resultCount);
        for (SIZE_T i = 0; i < resultCount; i++) {
            DbgPrint("  [%zu] %s @ %p (指令级: %s)\n",
                i,
                results[i].SignatureName,
                results[i].Address,
                results[i].IsInstructionMatch ? "是" : "否");
        }
    }
}