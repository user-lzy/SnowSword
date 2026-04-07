#pragma once

#include "global.h"
#include "OtherFunctions.h"
#include "ntimage.h"

NTKERNELAPI PVOID RtlImageDirectoryEntryToData(
    _In_ PVOID Base,
    _In_ BOOLEAN MappedAsImage,
    _In_ USHORT DirectoryEntry,
    _Out_ PULONG Size
);

// 假设pUserUnicodeString是用户模式传来的UNICODE_STRING指针
NTSTATUS ValidateUserUnicodeString(PUNICODE_STRING pUserUnicodeString, PUNICODE_STRING pKernelUnicodeString) {
#define MAX_PATH 260
    NTSTATUS status = STATUS_SUCCESS;
    USHORT maxAllowedLength = MAX_PATH * sizeof(WCHAR); // 限制最大长度（按需调整）

    // 步骤1：验证用户传入的UNICODE_STRING结构体本身是否可访问
    if (pUserUnicodeString == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
    __try {
        // 探测结构体是否可读取（用户模式内存验证）
        ProbeForRead(pUserUnicodeString, sizeof(UNICODE_STRING), sizeof(ULONG));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
		//获取错误信息
		return GetExceptionCode(); // 返回异常代码
    }

    DbgPrint("1");

    // 步骤2：验证Length和MaximumLength的合法性
    if (pUserUnicodeString->Length > pUserUnicodeString->MaximumLength) {
        return STATUS_INVALID_PARAMETER; // Length不能超过MaximumLength
    }
    if (pUserUnicodeString->MaximumLength == 0) {
        return STATUS_INVALID_PARAMETER; // 缓冲区长度不能为0
    }
    if (pUserUnicodeString->Length % sizeof(WCHAR) != 0) {
        return STATUS_INVALID_PARAMETER; // Length必须是WCHAR的整数倍（每个WCHAR占2字节）
    }
    if (pUserUnicodeString->MaximumLength > maxAllowedLength) {
        return STATUS_NAME_TOO_LONG; // 限制最大长度，防止恶意输入过大缓冲区
    }

    // 步骤3：验证Buffer指针指向的用户内存是否可访问
    if (pUserUnicodeString->Buffer == NULL) {
        return STATUS_INVALID_PARAMETER; // Buffer不能为NULL
    }
    __try {
        // 探测Buffer指向的内存是否可读取（长度为MaximumLength）
        ProbeForRead(pUserUnicodeString->Buffer, pUserUnicodeString->MaximumLength, sizeof(WCHAR));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    // 步骤4：验证字符串内容（可选，根据业务需求）
    // 例如：检查是否以null结尾（部分场景需要，如文件路径）
    BOOLEAN isNullTerminated = FALSE;
    for (USHORT i = 0; i < pUserUnicodeString->Length; i += sizeof(WCHAR)) {
        if (pUserUnicodeString->Buffer[i / sizeof(WCHAR)] == L'\0') {
            isNullTerminated = TRUE;
            break;
        }
    }
    if (!isNullTerminated) {
        return STATUS_INVALID_PARAMETER; // 要求字符串必须以null结尾
    }

    // 步骤5：（可选）将用户空间字符串复制到内核空间（避免用户后续修改内存）
    pKernelUnicodeString->MaximumLength = pUserUnicodeString->MaximumLength;
    pKernelUnicodeString->Buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, pKernelUnicodeString->MaximumLength, 'tag');
    if (pKernelUnicodeString->Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(pKernelUnicodeString->Buffer, pUserUnicodeString->Buffer, pUserUnicodeString->Length);
    pKernelUnicodeString->Length = pUserUnicodeString->Length;

    return status;
}

PVOID GetPspCidTable() {
    PVOID PsLookupProcessByProcessIdAddr = (PVOID)PsLookupProcessByProcessId;
    PUCHAR StartSearchAddress = NULL;
    // fffff806`ca714a2e 488b0583126b00  mov     rax,qword ptr [nt!PspCidTable (fffff806`cadc5cb8)]
    // fffff803`16425cc4 e8a7010000      call    nt!PspReferenceCidTableEntry (fffff803`16425e70)
	UCHAR pSpecialCode1[1] = { 0xE8 };
    PVOID result = SearchSpecialCode(PsLookupProcessByProcessIdAddr, 0x100, pSpecialCode1, 1);
    if (result != NULL) {
        // 计算目标地址
        ULONG offset = *(PULONG)((PUCHAR)result + 1);
        StartSearchAddress = (PUCHAR)((PUCHAR)result + 5 + offset);
        DbgPrint("PspReferenceCidTableEntry: 0x%p \n", StartSearchAddress);
    }
    else {
        DbgPrint("PspReferenceCidTableEntry is NULL");
        StartSearchAddress = (PUCHAR)PsLookupProcessByProcessIdAddr;
    }

	// 指定特征码
	UCHAR pSpecialCode2[3] = { 0x48, 0x8B, 0x05 };
    result = SearchSpecialCode(StartSearchAddress, 0x100, pSpecialCode2, 3);
    if (NULL == result)
    {
        DbgPrint("PspCidTable is NULL");
        return NULL;
    }
    // 计算目标地址
    ULONG offset = *(PULONG)((PUCHAR)result + 3);
	return (PVOID)((PUCHAR)result + 7 + offset);
}

/* 解析一级表
// By: LyShark.com
BaseAddr：一级表的基地址
index1：第几个一级表
index2：第几个二级表
*/
static BOOLEAN parse_table_1(ULONG64 BaseAddr, INT index1, INT index2, BOOLEAN(*CallbackFunction)(HANDLEINFO*, PVOID), PVOID Context)
{
    // 遍历一级表（每个表项大小 16 ），表大小 4k，所以遍历 4096/16 = 1024 次
    INT i_id = 0;
    for (INT i = 0; i < 512; i++) // 256?
    {
        if (!MmIsAddressValid((PVOID64)(BaseAddr + (ULONG64)i * 16)))
        {
            DbgPrint("非法地址= %lld \n", BaseAddr + (ULONG64)i * 16);
            continue;
        }

        ULONG64 ul_recode = *(PULONG64)(BaseAddr + (ULONG64)i * 16);

        // 解密
        ULONG64 ul_decode = (LONG64)ul_recode >> 0x10;
        ul_decode &= 0xfffffffffffffff0;
        // 调用回调函数
		HANDLEINFO handleInfo = { 0 };
        i_id = i * 4 + 1024 * index1 + 512 * index2 * 1024;
		handleInfo.Handle = (HANDLE)i_id;
		handleInfo.Object = (PVOID)ul_decode;
		if (!CallbackFunction(&handleInfo, Context))
        {
			// 终止遍历
            return FALSE;
		}
    }
    return TRUE;
}

/* 解析二级表
// By: LyShark.com
BaseAddr：二级表基地址
index2：第几个二级表
*/
static BOOLEAN parse_table_2(ULONG64 BaseAddr, INT index2, BOOLEAN(*CallbackFunction)(HANDLEINFO*, PVOID), PVOID Context)
{
    // 遍历二级表（每个表项大小 8）,表大小 4k，所以遍历 4096/8 = 512 次
    ULONG64 ul_baseAddr_1 = 0;
    for (INT i = 0; i < 512; i++)
    {
        if (!MmIsAddressValid((PVOID64)(BaseAddr + (ULONG64)i * 8)))
        {
            DbgPrint("非法二级表指针（1）:%lld \n", BaseAddr + (ULONG64)i * 8);
            continue;
        }
        if (!MmIsAddressValid((PVOID64) * (PULONG64)(BaseAddr + (ULONG64)i * 8)))
        {
            DbgPrint("非法二级表指针（2）:%lld \n", BaseAddr + (ULONG64)i * 8);
            continue;
        }
        ul_baseAddr_1 = *(PULONG64)(BaseAddr + (ULONG64)i * 8);
        if (!parse_table_1(ul_baseAddr_1, i, index2, CallbackFunction, Context)) {
            // 终止遍历
			return FALSE;
        }
    }
    return TRUE;
}

/* 解析三级表
// By: LyShark.com
BaseAddr：三级表基地址
*/
static VOID parse_table_3(ULONG64 BaseAddr, BOOLEAN(*CallbackFunction)(HANDLEINFO*, PVOID), PVOID Context)
{
    // 遍历三级表（每个表项大小 8）,表大小 4k，所以遍历 4096/8 = 512 次
    ULONG64 ul_baseAddr_2 = 0;
    for (INT i = 0; i < 512; i++)
    {
        if (!MmIsAddressValid((PVOID64)(BaseAddr + (ULONG64)i * 8)))
        {
            continue;
        }
        if (!MmIsAddressValid((PVOID64) * (PULONG64)(BaseAddr + (ULONG64)i * 8)))
        {
            continue;
        }
        ul_baseAddr_2 = *(PULONG64)(BaseAddr + (ULONG64)i * 8);
        if (!parse_table_2(ul_baseAddr_2, i, CallbackFunction, Context)) {
			// 终止遍历
			return;
        }
    }
}

VOID MyExEnumHandleTable(
    PVOID TableBase,
    BOOLEAN (*CallbackFunction)(HANDLEINFO*, PVOID) ,
    PVOID Context
) {
    // 获取 _HANDLE_TABLE 的 TableCode
    ULONG64 ul_tableCode = *(PULONG64)(((ULONG64) * (PULONG64)TableBase) + 8);
    DbgPrint("ul_tableCode = %lld \n", ul_tableCode);

    // 取低 2位（二级制11 = 3）
    INT i_low2 = ul_tableCode & 3;
    DbgPrint("i_low2 = %X \n", i_low2);

    // 一级表
    if (i_low2 == 0)
    {
        // TableCode 低 2位抹零（二级制11 = 3）
        parse_table_1(ul_tableCode & (~3), 0, 0, CallbackFunction, Context);
    }
    // 二级表
    else if (i_low2 == 1)
    {
        // TableCode 低 2位抹零（二级制11 = 3）
        parse_table_2(ul_tableCode & (~3), 0, CallbackFunction, Context);
    }
    // 三级表
    else if (i_low2 == 2)
    {
        // TableCode 低 2位抹零（二级制11 = 3）
        parse_table_3(ul_tableCode & (~3), CallbackFunction, Context);
    }
    else
    {
        DbgPrint("提示: 错误,非法! ");
        return;
    }
}

/**
 * @brief 解析PE导出表，根据函数名查找函数地址（替代内部的MiFindExportedRoutineByName）
 * @param DllBase 模块基址
 * @param FunctionName ANSI格式的函数名
 * @return 函数地址（NULL表示未找到）
 */
PVOID FindExportedFunctionByName(
    _In_ PVOID DllBase,
    _In_ PCHAR FunctionName
)
{
    // 基础参数校验
    if (DllBase == NULL || FunctionName == NULL || *FunctionName == '\0')
    {
        return NULL;
    }

    ULONG ExportDirSize = 0;
    // 获取PE导出目录（公开接口）
    PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
        DllBase,
        TRUE,                  // 是PE镜像
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &ExportDirSize
    );
    

    // 无导出表则直接返回
    if (ExportDir == NULL || ExportDirSize == 0)
    {
        return NULL;
    }

    // 计算导出表各子表的实际地址
    PULONG  NameTable = (PULONG)((PUCHAR)DllBase + ExportDir->AddressOfNames);
    PUSHORT OrdinalTable = (PUSHORT)((PUCHAR)DllBase + ExportDir->AddressOfNameOrdinals);
    PULONG  FunctionTable = (PULONG)((PUCHAR)DllBase + ExportDir->AddressOfFunctions);

    // 子表地址无效则返回
    if (NameTable == NULL || OrdinalTable == NULL || FunctionTable == NULL)
    {
        return NULL;
    }

    // 二分查找（导出表名称表按字母序排序，效率最高）
    LONG   Low = 0;
    LONG   High = ExportDir->NumberOfNames - 1;
    LONG   Mid = 0;
    INT    Cmp = 0;

    while (Low <= High)
    {
        Mid = (Low + High) / 2; // 等价于位运算 (Low+High)>>1，更易读
        PCHAR CurrentFuncName = (PCHAR)DllBase + NameTable[Mid];

        Cmp = strcmp(FunctionName, CurrentFuncName);
        if (Cmp == 0)
        {
            // 找到匹配的函数名，退出循环
            break;
        }
        else if (Cmp < 0)
        {
            // 目标名称更小，查左半区
            High = Mid - 1;
        }
        else
        {
            // 目标名称更大，查右半区
            Low = Mid + 1;
        }
    }

    // 未找到匹配的函数名
    if (Low > High)
    {
        return NULL;
    }

    // 验证序号有效性
    USHORT Ordinal = OrdinalTable[Mid];
    if (Ordinal >= ExportDir->NumberOfFunctions)
    {
        return NULL;
    }

    // 计算最终的函数地址
    PVOID FuncAddr = (PVOID)((PUCHAR)DllBase + FunctionTable[Ordinal]);
    return FuncAddr;
}

/**
 * @brief 内核版GetProcAddress：根据模块名和函数名查找导出函数地址
 * @param ModuleName ANSI格式的模块名（如"ntoskrnl.exe"、"hal.dll"）
 * @param FunctionName ANSI格式的函数名（如"ZwCreateProcess"）
 * @return 函数地址（NULL表示未找到/参数错误）
 */
PVOID KernelGetProcAddress(
    _In_ PCHAR ModuleName,
    _In_ PCHAR FunctionName
)
{
    // 1. 基础参数校验
    if (ModuleName == NULL || *ModuleName == '\0' ||
        FunctionName == NULL || *FunctionName == '\0')
    {
        DbgPrint("KernelGetProcAddress: Invalid parameters\n");
        return NULL;
    }

    PVOID                  DllBase = NULL;
    PVOID                  FuncAddr = NULL;
    UNICODE_STRING         UnicodeModName = { 0 };
    ANSI_STRING            AnsiModName = { 0 };
    PLIST_ENTRY            NextEntry = NULL;
    PKLDR_DATA_TABLE_ENTRY  LdrEntry = NULL;
    NTSTATUS               Status = STATUS_SUCCESS;

    // 2. 初始化ANSI字符串并转换为UNICODE（模块列表用UNICODE匹配）
    RtlInitAnsiString(&AnsiModName, ModuleName);
    Status = RtlAnsiStringToUnicodeString(&UnicodeModName, &AnsiModName, TRUE);
    if (!NT_SUCCESS(Status))
    {
        DbgPrint("KernelGetProcAddress: RtlAnsiStringToUnicodeString failed (0x%08X)\n", Status);
        return NULL;
    }

    // 3. 同步：获取模块列表的共享锁（避免遍历过程中列表被修改）
    KeEnterCriticalRegion();
    //ExAcquireResourceSharedLite(&PsLoadedModuleResource, TRUE); // 会导致ATTEMPTED_WRITE_TO_READONLY_MEMORY蓝屏

    // 4. 遍历已加载模块列表，匹配目标模块
    NextEntry = PsLoadedModuleList->Flink;
    while (NextEntry != PsLoadedModuleList)
    {
        // 获取当前模块的LDR条目
        LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (LdrEntry == NULL)
        {
            NextEntry = NextEntry->Flink;
            continue;
        }

        // 匹配模块名（TRUE=不区分大小写，符合Windows规则）
        if (RtlEqualUnicodeString(&UnicodeModName, &LdrEntry->BaseDllName, TRUE))
        {
            DllBase = (PVOID)LdrEntry->DllBase;
            break;
        }

        NextEntry = NextEntry->Flink;
    }

    // 5. 释放同步资源
    //ExReleaseResourceLite(&PsLoadedModuleResource);
    KeLeaveCriticalRegion();

    // 6. 模块未找到的处理
    if (DllBase == NULL)
    {
        DbgPrint("KernelGetProcAddress: Module '%s' not found\n", ModuleName);
        RtlFreeUnicodeString(&UnicodeModName);
        return NULL;
    }

    // 7. 解析导出表，查找函数地址
    FuncAddr = FindExportedFunctionByName(DllBase, FunctionName);
    if (FuncAddr == NULL)
    {
        DbgPrint("KernelGetProcAddress: Function '%s' not found in '%s'\n", FunctionName, ModuleName);
    }

    // 8. 释放UNICODE字符串资源
    RtlFreeUnicodeString(&UnicodeModName);

    return FuncAddr;
}