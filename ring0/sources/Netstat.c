#include "Netstat.h"
#include "OtherFunctions.h"
#include "Symbol.h"
#include "ntstrsafe.h"
#include <ntddk.h>
#include <ndis.h>

/* ---------------------------------------------------------------------------
   辅助：RIP-relative 转绝对地址
   --------------------------------------------------------------------------- */
static ULONG64 RipToAbsolute(_In_ PUCHAR Inst, _In_ ULONG InstLen, _In_ ULONG RelOffset)
{
    LONG Rel32 = *(PLONG)(Inst + RelOffset);
    return (ULONG64)(Inst + InstLen + Rel32);
}

/* ---------------------------------------------------------------------------
   辅助：获取 ndis.sys 模块基址和大小（必须！防止越界扫描）
   --------------------------------------------------------------------------- */
NTSTATUS NTAPI NtQuerySystemInformation(
    _In_      ULONG SystemInformationClass,
    _Out_opt_ PVOID SystemInformation,
    _In_      ULONG SystemInformationLength,
    _Out_opt_ PULONG ReturnLength
);

static NTSTATUS GetNdisModuleInfo(_Out_ PVOID* Base, _Out_ ULONG* Size)
{
    NTSTATUS Status;
    ULONG Need = 0;
    PVOID Buf = NULL;
#define SystemModuleInformation 0xB

    Status = NtQuerySystemInformation(SystemModuleInformation, NULL, 0, &Need);
    if (Status != STATUS_INFO_LENGTH_MISMATCH)
        return Status;

    Buf = ExAllocatePool2(POOL_FLAG_NON_PAGED, Need, 'mNds');
    if (!Buf) return STATUS_INSUFFICIENT_RESOURCES;

    Status = NtQuerySystemInformation(SystemModuleInformation, Buf, Need, &Need);
    if (NT_SUCCESS(Status)) {
        PRTL_PROCESS_MODULES Mods = (PRTL_PROCESS_MODULES)Buf;
        for (ULONG i = 0; i < Mods->NumberOfModules; i++) {
            PRTL_PROCESS_MODULE_INFORMATION M = &Mods->Modules[i];
            PCHAR Name = (PCHAR)M->FullPathName + M->ModuleNameOffset;
            if (_stricmp(Name, "ndis.sys") == 0) {
                *Base = M->ImageBase;
                *Size = M->ImageSize;
                break;
            }
        }
    }
    ExFreePoolWithTag(Buf, 'mNds');
    return (*Base) ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

/* ---------------------------------------------------------------------------
   辅助：在已确保合法的范围内调用你的搜索函数
   --------------------------------------------------------------------------- */
static PVOID SafeSearch(_In_ PVOID ModBase, _In_ ULONG ModSize,
    _In_ PVOID StartAddr, _In_ ULONG DesiredLen,
    _In_ PUCHAR Sig, _In_ PUCHAR Mask, _In_ ULONG SigLen)
{
    if (!StartAddr) return NULL;

    /* 裁剪起始地址 */
    if ((PUCHAR)StartAddr < (PUCHAR)ModBase)
        StartAddr = ModBase;
    if ((PUCHAR)StartAddr >= (PUCHAR)ModBase + ModSize)
        return NULL;

    /* 裁剪长度，确保不超出模块边界 */
    ULONG MaxLen = (ULONG)((PUCHAR)ModBase + ModSize - (PUCHAR)StartAddr);
    if (DesiredLen > MaxLen) DesiredLen = MaxLen;
    if (DesiredLen == 0 || DesiredLen < SigLen)
        return NULL;

    return SearchSpecialCodeWithMask(StartAddr, DesiredLen, Sig, Mask, SigLen);
}

/* ---------------------------------------------------------------------------
   主函数
   --------------------------------------------------------------------------- */
NTSTATUS
NTAPI
GetNdisListAndLockAddresses(
    _Out_ PULONG64 ProtocolListLock,
    _Out_ PULONG64 ProtocolList,
    _Out_ PULONG64 MiniportListLock,
    _Out_ PULONG64 MiniportList
)
{
    NTSTATUS Status;
    PVOID NdisBase = NULL;
    ULONG NdisSize = 0;
    PUCHAR CommonLea = NULL;   // ndisRegisterProtocolDriverCommon 内的 lea rdx,[rcx+328h]
    PUCHAR Match = NULL;

    /* 1. 优先符号解析 ---------------------------------------------------- */
    Status = KernelQuerySymbolAddress(L"ndis.sys", L"ndisProtocolListLock", ProtocolListLock);
    if (Status == STATUS_SUCCESS)
        Status = KernelQuerySymbolAddress(L"ndis.sys", L"ndisProtocolList", ProtocolList);
    if (Status == STATUS_SUCCESS)
        Status = KernelQuerySymbolAddress(L"ndis.sys", L"ndisMiniportListLock", MiniportListLock);
    if (Status == STATUS_SUCCESS)
        Status = KernelQuerySymbolAddress(L"ndis.sys", L"ndisMiniportList", MiniportList);
    if (Status == STATUS_SUCCESS)
        return STATUS_SUCCESS;

    /* 2. 获取模块边界 ---------------------------------------------------- */
    Status = GetNdisModuleInfo(&NdisBase, &NdisSize);
    if (!NT_SUCCESS(Status)) return Status;

    /* 3. 定位未导出的 ndisRegisterProtocolDriverCommon
       稳定特征：lea rdx,[rcx+328h] (48 8D 91 28 03 00 00)
       这是该函数内用于调用 ndisQueryDriverImageName 的标志性指令，
       跨版本稳定，且在 ndis.sys 中几乎唯一。                         */
    {
        UCHAR LeaSig[] = { 0x48, 0x8D, 0x91, 0x28, 0x03, 0x00, 0x00 };
        UCHAR LeaMask[] = { 1, 1, 1, 1, 1, 1, 1 };

        CommonLea = (PUCHAR)SafeSearch(NdisBase, NdisSize, NdisBase, NdisSize,
            LeaSig, LeaMask, sizeof(LeaSig));
    }
    if (!CommonLea) return STATUS_NOT_FOUND;

    /* 4. 搜索 Protocol List / Lock ---------------------------------------
       Win10 & Win11 骨架完全一致（差异在更后面，不影响本特征码）       */
    {
        UCHAR ProtoSig[] = {
            0x48, 0x8D, 0x0D, 0,0,0,0,    // +0x00 lea rcx,[ndisProtocolListLock]
            0x4C, 0x8B, 0x15, 0,0,0,0,    // +0x07 mov r10,[imp_KeAcquireSpinLockRaiseToDpc]
            0xE8, 0,0,0,0,                // +0x0E call nt!KeAcquireSpinLockRaiseToDpc
            0x48, 0x8B, 0x15, 0,0,0,0,    // +0x13 mov rdx,[ndisProtocolList]
            0x48, 0x8D, 0x0D, 0,0,0,0,    // +0x1A lea rcx,[ndisProtocolListLock]
            0x48, 0x89, 0x53, 0x10        // +0x21 mov [rbx+10h],rdx
        };
        UCHAR ProtoMask[] = {
            1,1,1,0,0,0,0,
            1,1,1,0,0,0,0,
            1,0,0,0,0,
            1,1,1,0,0,0,0,
            1,1,1,0,0,0,0,
            1,1,1,1
        };

        Match = (PUCHAR)SafeSearch(NdisBase, NdisSize, CommonLea, 0x200,
            ProtoSig, ProtoMask, sizeof(ProtoSig));
    }
    if (!Match) return STATUS_NOT_FOUND;

    if (ProtocolListLock) *ProtocolListLock = RipToAbsolute(Match + 0x00, 7, 3);
    if (ProtocolList) *ProtocolList = RipToAbsolute(Match + 0x13, 7, 3);

    /* 5. 搜索 Miniport List / Lock -------------------------------------- */
    {
        /* --- 先尝试 Win11 主路径风格（带 mov [rsp+30h],rdi）--- */
        UCHAR Mini11Sig[] = {
            0x48, 0x8D, 0x0D, 0,0,0,0,    // +0x00 lea rcx,[ndisMiniportListLock]
            0x48, 0x89, 0x7C, 0x24, 0x30, // +0x07 mov [rsp+30h],rdi
            0x4C, 0x8B, 0x15, 0,0,0,0,    // +0x0C mov r10,[imp]
            0xE8, 0,0,0,0,                // +0x13 call
            0x48, 0x8B, 0x3D, 0,0,0,0     // +0x18 mov rdi,[ndisMiniportList]
        };
        UCHAR Mini11Mask[] = {
            1,1,1,0,0,0,0,
            1,1,1,1,1,
            1,1,1,0,0,0,0,
            1,0,0,0,0,
            1,1,1,0,0,0,0
        };

        Match = (PUCHAR)SafeSearch(NdisBase, NdisSize, CommonLea, 0x300,
            Mini11Sig, Mini11Mask, sizeof(Mini11Sig));

        if (Match) {
            if (MiniportListLock) *MiniportListLock = RipToAbsolute(Match + 0x00, 7, 3);
            if (MiniportList) *MiniportList = RipToAbsolute(Match + 0x18, 7, 3);
        }
        else {
            /* --- Win10：Miniport 逻辑在 jl 远跳分支中 ---
               先在 Common 函数主路径末尾搜索 jl 远跳指令。
               Win11 此处是 jge，不会误匹配。                         */
            UCHAR JlSig[] = {
                0x83, 0x7B, 0x40, 0x00,       // cmp dword ptr [rbx+40h],0
                0x0F, 0x8C, 0,0,0,0           // jl rel32
            };
            UCHAR JlMask[] = {
                1, 1, 1, 1,
                1, 1, 0, 0, 0, 0
            };

            PUCHAR JlMatch = (PUCHAR)SafeSearch(NdisBase, NdisSize, CommonLea, 0x100,
                JlSig, JlMask, sizeof(JlSig));
            if (!JlMatch) return STATUS_NOT_FOUND;

            /* JlMatch 指向 83 7B 40 00；0F 8C 在 +4；rel32 在 +6。
               此处读取的内存已在 SearchSpecialCodeWithMask 匹配时被证明可读。 */
            LONG Rel32 = *(PLONG)(JlMatch + 6);
            PUCHAR FarTarget = JlMatch + 10 + Rel32;   // 4 + 6 = 10

            if (FarTarget < (PUCHAR)NdisBase ||
                FarTarget >= (PUCHAR)NdisBase + NdisSize)
                return STATUS_NOT_FOUND;

            /* 在远跳目标处搜索 Win10 Miniport 特征（无 mov [rsp+30h],rdi） */
            UCHAR Mini10Sig[] = {
                0x48, 0x8D, 0x0D, 0,0,0,0,    // +0x00 lea rcx,[ndisMiniportListLock]
                0x4C, 0x8B, 0x15, 0,0,0,0,    // +0x07 mov r10,[imp]
                0xE8, 0,0,0,0,                // +0x0E call
                0x48, 0x8B, 0x3D, 0,0,0,0     // +0x13 mov rdi,[ndisMiniportList]
            };
            UCHAR Mini10Mask[] = {
                1,1,1,0,0,0,0,
                1,1,1,0,0,0,0,
                1,0,0,0,0,
                1,1,1,0,0,0,0
            };

            Match = (PUCHAR)SafeSearch(NdisBase, NdisSize, FarTarget, 0x100,
                Mini10Sig, Mini10Mask, sizeof(Mini10Sig));
            if (!Match) return STATUS_NOT_FOUND;

            if (MiniportListLock) *MiniportListLock = RipToAbsolute(Match + 0x00, 7, 3);
            if (MiniportList) *MiniportList = RipToAbsolute(Match + 0x13, 7, 3);
        }
    }

    return STATUS_SUCCESS;
}

// ============================================================================
// NDIS Miniport 枚举器 - 以 Miniport 为根遍历 Filter 栈与协议绑定
// 支持 Win10 22H2 / Win11 23H2+
// ============================================================================

// ----------------------------------------------------------------------------
// 结构体偏移缓存（初始化后只读，无需锁）
// ----------------------------------------------------------------------------
typedef struct _NDIS_STRUCT_OFFSETS {
    BOOLEAN     Initialized;

    // _NDIS_MINIPORT_BLOCK
    LONG        Off_NextGlobalMiniport;     // +0xf08
    LONG        Off_OpenQueue;              // +0x038
    LONG        Off_HighestFilter;          // +0x800
    LONG        Off_LowestFilter;           // +0x7f8
    //LONG        Off_MiniportName;           // +0xee0
    LONG        Off_BaseName;               // +0xed0
    LONG        Off_DeviceObject;           // +0xef0
	LONG        Off_pAdapterInstanceName;   // +0xf10

    // _NDIS_FILTER_BLOCK
    LONG        Off_Flt_NextFilter;         // +0x008
    LONG        Off_Flt_FilterDriver;       // +0x010
    LONG        Off_Flt_Miniport;           // +0x020
    LONG        Off_Flt_FilterInstanceName; // +0x028
    LONG        Off_Flt_FilterFriendlyName; // +0x030
    LONG        Off_Flt_NextGlobalFilter;   // +0x068
    LONG        Off_Flt_LowerFilter;        // +0x070
    LONG        Off_Flt_HigherFilter;       // +0x078

    // _NDIS_OPEN_BLOCK
    LONG        Off_Open_MiniportHandle;    // +0x010
    LONG        Off_Open_ProtocolHandle;    // +0x018
    LONG        Off_Open_MiniportNextOpen;  // +0x188
    LONG        Off_Open_ProtocolNextOpen;  // +0x190

    // _NDIS_PROTOCOL_BLOCK
    LONG        Off_Prot_Name;              // +0x048
    LONG        Off_Prot_ImageName;         // +0x328

    // _NDIS_FILTER_DRIVER_BLOCK
    LONG        Off_FltDrv_ImageName;       // Win10:0x160, Win11:0x158

} NDIS_STRUCT_OFFSETS, * PNDIS_STRUCT_OFFSETS;

static NDIS_STRUCT_OFFSETS g_NdisOffsets = { 0 };

// ----------------------------------------------------------------------------
// 版本辅助
// ----------------------------------------------------------------------------
static BOOLEAN
IsWindows11OrLater(
    VOID
)
{
    RTL_OSVERSIONINFOW ver = { 0 };
    ver.dwOSVersionInfoSize = sizeof(ver);
    RtlGetVersion(&ver);
    return (ver.dwMajorVersion > 10) ||
        (ver.dwMajorVersion == 10 && ver.dwBuildNumber >= 22000);
}

// ----------------------------------------------------------------------------
// 安全读取 UnicodeString 到用户缓冲区（带 SEH）
// ----------------------------------------------------------------------------
static NTSTATUS
SafeCopyUnicodeString(
    _In_opt_ PUNICODE_STRING pSrc,
    _Out_writes_(MaxChars) PWCHAR Dst,
    _In_ ULONG MaxChars
)
{
    RtlZeroMemory(Dst, MaxChars * sizeof(WCHAR));
    if (!pSrc || !pSrc->Buffer || !MmIsAddressValid(pSrc->Buffer) || pSrc->Length == 0 || !Dst || !MmIsAddressValid(Dst))
        return STATUS_SUCCESS;

    __try {
        ULONG copyLen = min(pSrc->Length, (MaxChars - 1) * sizeof(WCHAR));
        RtlCopyMemory(Dst, pSrc->Buffer, copyLen);
        Dst[copyLen / sizeof(WCHAR)] = L'\0';
        return STATUS_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_ACCESS_VIOLATION;
    }
}

// ----------------------------------------------------------------------------
// 读取 wistd::unique_ptr<Rtl::KString>
// KString 在 Win10/Win11 中以 UNICODE_STRING 为首个成员
// ----------------------------------------------------------------------------
static NTSTATUS
SafeCopyKString(
    _In_ PVOID pUniquePtrStorage,
    _Out_writes_(MaxChars) PWCHAR Dst,
    _In_ ULONG MaxChars
)
{
    RtlZeroMemory(Dst, MaxChars * sizeof(WCHAR));
    if (!pUniquePtrStorage)
        return STATUS_INVALID_PARAMETER;

    __try {
        PVOID pKString = *(PVOID*)pUniquePtrStorage;
        if (!pKString)
            return STATUS_SUCCESS;

        PUNICODE_STRING pUni = (PUNICODE_STRING)pKString;
        return SafeCopyUnicodeString(pUni, Dst, MaxChars);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_ACCESS_VIOLATION;
    }
}

// ----------------------------------------------------------------------------
// 偏移初始化：优先符号查询，失败按版本回退硬编码
// ----------------------------------------------------------------------------
static NTSTATUS
NdisEnumInitializeOffsets(
    VOID
)
{
    NTSTATUS status;
    BOOLEAN isWin11 = IsWindows11OrLater();
    PCWSTR mod = L"ndis.sys";

    if (g_NdisOffsets.Initialized)
        return STATUS_SUCCESS;

    DbgPrint(DBG_PREFIX "Initializing offsets (Win11=%d)...\n", isWin11);

#define QUERY_OR_FALLBACK(_symW, _memberW, _field, _win11off, _win10off) \
    status = KernelQueryStructOffset(mod, _symW, _memberW, &g_NdisOffsets._field); \
    if (status != STATUS_SUCCESS) { \
        g_NdisOffsets._field = isWin11 ? (_win11off) : (_win10off); \
        DbgPrint(DBG_PREFIX "  Fallback %ws.%ws = 0x%X\n", _symW, _memberW, g_NdisOffsets._field); \
    } else { \
        DbgPrint(DBG_PREFIX "  Symbol  %ws.%ws = 0x%X\n", _symW, _memberW, g_NdisOffsets._field); \
    }

    // _NDIS_MINIPORT_BLOCK
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"NextGlobalMiniport", Off_NextGlobalMiniport, 0x0f08, 0x0f08);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"OpenQueue", Off_OpenQueue, 0x0038, 0x0038);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"HighestFilter", Off_HighestFilter, 0x0800, 0x0800);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"LowestFilter", Off_LowestFilter, 0x07f8, 0x07f8);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"BaseName", Off_BaseName, 0x0ed0, 0x0ed0);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"DeviceObject", Off_DeviceObject, 0x0ef0, 0x0ef0);
    QUERY_OR_FALLBACK(L"_NDIS_MINIPORT_BLOCK", L"pAdapterInstanceName", Off_pAdapterInstanceName, 0x0f10, 0x0f10);

    // _NDIS_FILTER_BLOCK
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"NextFilter", Off_Flt_NextFilter, 0x0008, 0x0008);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"FilterDriver", Off_Flt_FilterDriver, 0x0010, 0x0010);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"Miniport", Off_Flt_Miniport, 0x0020, 0x0020);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"FilterInstanceName", Off_Flt_FilterInstanceName, 0x0028, 0x0028);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"FilterFriendlyName", Off_Flt_FilterFriendlyName, 0x0030, 0x0030);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"NextGlobalFilter", Off_Flt_NextGlobalFilter, 0x0068, 0x0068);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"LowerFilter", Off_Flt_LowerFilter, 0x0070, 0x0070);
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_BLOCK", L"HigherFilter", Off_Flt_HigherFilter, 0x0078, 0x0078);

    // _NDIS_OPEN_BLOCK
    QUERY_OR_FALLBACK(L"_NDIS_OPEN_BLOCK", L"MiniportHandle", Off_Open_MiniportHandle, 0x0010, 0x0010);
    QUERY_OR_FALLBACK(L"_NDIS_OPEN_BLOCK", L"ProtocolHandle", Off_Open_ProtocolHandle, 0x0018, 0x0018);
    QUERY_OR_FALLBACK(L"_NDIS_OPEN_BLOCK", L"MiniportNextOpen", Off_Open_MiniportNextOpen, 0x0188, 0x0188);
    QUERY_OR_FALLBACK(L"_NDIS_OPEN_BLOCK", L"ProtocolNextOpen", Off_Open_ProtocolNextOpen, 0x0190, 0x0190);

    // _NDIS_PROTOCOL_BLOCK
    QUERY_OR_FALLBACK(L"_NDIS_PROTOCOL_BLOCK", L"Name", Off_Prot_Name, 0x0048, 0x0048);
    QUERY_OR_FALLBACK(L"_NDIS_PROTOCOL_BLOCK", L"ImageName", Off_Prot_ImageName, 0x0328, 0x0328);

    // _NDIS_FILTER_DRIVER_BLOCK
    QUERY_OR_FALLBACK(L"_NDIS_FILTER_DRIVER_BLOCK", L"ImageName", Off_FltDrv_ImageName, 0x0158, 0x0160);

#undef QUERY_OR_FALLBACK

    g_NdisOffsets.Initialized = TRUE;
    return STATUS_SUCCESS;
}

// ----------------------------------------------------------------------------
// 核心遍历函数：以 Miniport 为根，枚举 Filter 栈与协议绑定
// ----------------------------------------------------------------------------
// 参数：
//   Entries      - 调用者提供的缓冲区（结构体数组）
//   EntrySize    - 单个条目大小（用于版本兼容校验）
//   MaxEntries   - 缓冲区可容纳的条目数
//   NeededEntries- 输出：实际需要条目数（即使返回 BUFFER_TOO_SMALL）
//   ActualEntries- 输出：实际填充的条目数
// ----------------------------------------------------------------------------
NTSTATUS
NTAPI
NdisEnumMiniports(
    _Out_writes_bytes_opt_(EntrySize* MaxEntries) PNDIS_MINIPORT_ENUM_ENTRY Entries,
    _In_ ULONG EntrySize,
    _In_ ULONG MaxEntries,
    _Out_ PULONG NeededEntries,
    _Out_ PULONG ActualEntries
)
{
    NTSTATUS status;
    ULONG64 ProtocolListLock = 0, ProtocolList = 0, MiniportListLock = 0, MiniportList = 0;
    PKSPIN_LOCK pSpinLock = NULL;
    PVOID pListHead = NULL;
    PVOID pMiniport = NULL;
    KIRQL oldIrql = 0;
    ULONG count = 0;
    PNDIS_STRUCT_OFFSETS ofs = &g_NdisOffsets;

    *NeededEntries = 0;
    *ActualEntries = 0;

    if (EntrySize < sizeof(NDIS_MINIPORT_ENUM_ENTRY)) {
        DbgPrint(DBG_PREFIX "EntrySize too small: %u < %u\n",
            EntrySize, (ULONG)sizeof(NDIS_MINIPORT_ENUM_ENTRY));
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    //DbgPrint(DBG_PREFIX "1");
    status = NdisEnumInitializeOffsets();
    if (!NT_SUCCESS(status))
        return status;
    //DbgPrint(DBG_PREFIX "2");
    status = GetNdisListAndLockAddresses(&ProtocolListLock, &ProtocolList, &MiniportListLock, &MiniportList);
    if (!NT_SUCCESS(status)) {
        DbgPrint(DBG_PREFIX "Failed to get list/lock addresses: 0x%X\n", status);
        return status;
    }

    pSpinLock = (PKSPIN_LOCK)MiniportListLock;
    pListHead = *(PVOID*)MiniportList;
    //DbgPrint(DBG_PREFIX "开始遍历...");
    // ========== 加锁区域：仅做纯内存访问，禁止 DbgPrint ==========
    KeAcquireSpinLock(pSpinLock, &oldIrql);

    __try {
        pMiniport = pListHead;

        while (pMiniport != NULL) {
            PNDIS_MINIPORT_ENUM_ENTRY entry = NULL;
            PVOID pFilter = NULL;
            PVOID pOpen = NULL;

            if (count < MaxEntries && Entries != NULL) {
                entry = (PNDIS_MINIPORT_ENUM_ENTRY)((PUCHAR)Entries + count * EntrySize);
                RtlZeroMemory(entry, EntrySize);
                entry->MiniportBlock = (ULONG64)pMiniport;
            }

            // ---------- Miniport 基本信息 ----------
            PVOID pDevObj = *(PVOID*)((PUCHAR)pMiniport + ofs->Off_DeviceObject);
            PUNICODE_STRING pAdapterInstanceName = *(PUNICODE_STRING*)((PUCHAR)pMiniport + ofs->Off_pAdapterInstanceName);
            PUNICODE_STRING pBaseName = (PUNICODE_STRING)((PUCHAR)pMiniport + ofs->Off_BaseName);

            if (entry) {
                //entry->DeviceObject = (ULONG64)pDevObj;
                if (pDevObj && MmIsAddressValid(pDevObj))
					SafeCopyUnicodeString(&((PKLDR_DATA_TABLE_ENTRY)((PDEVICE_OBJECT)pDevObj)->DriverObject->DriverSection)->FullDllName, entry->DriverPath, 260);
                SafeCopyUnicodeString(pAdapterInstanceName, entry->pAdapterInstanceName, NDIS_ENUM_MAX_STRING_LEN);
                SafeCopyUnicodeString(pBaseName, entry->BaseName, NDIS_ENUM_MAX_STRING_LEN);
            }

            // ---------- Filter 栈遍历 ----------
            pFilter = *(PVOID*)((PUCHAR)pMiniport + ofs->Off_HighestFilter);
            
            while (pFilter && MmIsAddressValid(pFilter)) {
                PVOID pLower = NULL;
                PVOID pDriver = NULL;
                PNDIS_ENUM_FILTER_ENTRY fEntry = NULL;
                ULONG fIdx = 0;

                if (entry) {
                    fIdx = entry->FilterCount;
                    if (fIdx < NDIS_ENUM_MAX_FILTERS_PER_MINIPORT) {
                        fEntry = &entry->Filters[fIdx];
                        fEntry->FilterBlock = (ULONG64)pFilter;
                    }
                }

                //PVOID pBackMiniport = *(PVOID*)((PUCHAR)pFilter + ofs->Off_Flt_Miniport);

                if (fEntry) {
                    SafeCopyKString(
                        (PUCHAR)pFilter + ofs->Off_Flt_FilterInstanceName,
                        fEntry->InstanceName,
                        NDIS_ENUM_MAX_STRING_LEN
                    );

                    PUNICODE_STRING* ppFriendly = (PUNICODE_STRING*)((PUCHAR)pFilter + ofs->Off_Flt_FilterFriendlyName);
                    SafeCopyUnicodeString(*ppFriendly, fEntry->FriendlyName, NDIS_ENUM_MAX_STRING_LEN);

                    pDriver = *(PVOID*)((PUCHAR)pFilter + ofs->Off_Flt_FilterDriver);
                    if (pDriver) {
                        fEntry->DriverObject = (ULONG64) * (PVOID*)((PUCHAR)pDriver + 0x010);
                        PUNICODE_STRING pDrvName = (PUNICODE_STRING)((PUCHAR)pDriver + ofs->Off_FltDrv_ImageName);
                        SafeCopyUnicodeString(pDrvName, fEntry->DriverImageName, NDIS_ENUM_MAX_STRING_LEN);
                    }
                }

                if (entry && fEntry) {
                    entry->FilterCount++;
                }

                pLower = *(PVOID*)((PUCHAR)pFilter + ofs->Off_Flt_LowerFilter);
                pFilter = pLower;
            }

            // ---------- Open / Protocol 遍历 ----------
			pOpen = *(PVOID*)((PUCHAR)pMiniport + ofs->Off_OpenQueue);
            
            while (pOpen && MmIsAddressValid(pOpen)) {
                PVOID pNextOpen = NULL;
                PVOID pProtocol = NULL;
                PNDIS_ENUM_OPEN_ENTRY oEntry = NULL;
                ULONG oIdx = 0;

                if (entry) {
                    oIdx = entry->OpenCount;
                    if (oIdx < NDIS_ENUM_MAX_OPENS_PER_MINIPORT) {
                        oEntry = &entry->Opens[oIdx];
                        oEntry->OpenBlock = (ULONG64)pOpen;
                    }
                }

                //PVOID pBackMiniport = *(PVOID*)((PUCHAR)pOpen + ofs->Off_Open_MiniportHandle);

                pProtocol = *(PVOID*)((PUCHAR)pOpen + ofs->Off_Open_ProtocolHandle);
                if (oEntry) {
                    oEntry->ProtocolBlock = (ULONG64)pProtocol;
                }

                if (pProtocol && oEntry) {
                    PUNICODE_STRING pProtName = (PUNICODE_STRING)((PUCHAR)pProtocol + ofs->Off_Prot_Name);
                    PUNICODE_STRING pProtImg = (PUNICODE_STRING)((PUCHAR)pProtocol + ofs->Off_Prot_ImageName);
                    SafeCopyUnicodeString(pProtName, oEntry->ProtocolName, NDIS_ENUM_MAX_STRING_LEN);
                    SafeCopyUnicodeString(pProtImg, oEntry->ProtocolImageName, NDIS_ENUM_MAX_STRING_LEN);
                }

                if (entry && oEntry) {
                    entry->OpenCount++;
                }

                pNextOpen = *(PVOID*)((PUCHAR)pOpen + ofs->Off_Open_MiniportNextOpen);
                pOpen = pNextOpen;
            }

            // ---------- 下一个全局 Miniport ----------
            count++;
            pMiniport = *(PVOID*)((PUCHAR)pMiniport + ofs->Off_NextGlobalMiniport);
        }
    }
    __finally {
        KeReleaseSpinLock(pSpinLock, oldIrql);
    }
    // ========== 解锁完成 ==========

    *NeededEntries = count;
    *ActualEntries = (Entries != NULL) ? min(count, MaxEntries) : 0;
    //DbgPrint(DBG_PREFIX "开始输出Miniport...");
    // ========== 解锁后：统一调试输出（此时 IRQL 已恢复正常） ==========
    if (Entries != NULL) {
        for (ULONG i = 0; i < *ActualEntries; i++) {
            PNDIS_MINIPORT_ENUM_ENTRY entry = (PNDIS_MINIPORT_ENUM_ENTRY)((PUCHAR)Entries + i * EntrySize);

            DbgPrint(DBG_PREFIX "=== Miniport [%u] at 0x%p ===\n", i, (PVOID)entry->MiniportBlock);
            DbgPrint(DBG_PREFIX "  pAdapterInstanceName: %ws, BaseName: %ws, DriverPath: %ws\n",
                entry->pAdapterInstanceName[0] ? entry->pAdapterInstanceName : L"(null)",
                entry->BaseName[0] ? entry->BaseName : L"(null)",
                entry->DriverPath);

            for (ULONG f = 0; f < entry->FilterCount; f++) {
                PNDIS_ENUM_FILTER_ENTRY fEntry = &entry->Filters[f];
                DbgPrint(DBG_PREFIX "  Filter [%u] at 0x%p\n", f, (PVOID)fEntry->FilterBlock);
                DbgPrint(DBG_PREFIX "    InstanceName: %ws, FriendlyName: %ws, Driver: %ws\n",
                    fEntry->InstanceName[0] ? fEntry->InstanceName : L"?",
                    fEntry->FriendlyName[0] ? fEntry->FriendlyName : L"?",
                    fEntry->DriverImageName[0] ? fEntry->DriverImageName : L"?");
            }

            for (ULONG o = 0; o < entry->OpenCount; o++) {
                PNDIS_ENUM_OPEN_ENTRY oEntry = &entry->Opens[o];
                DbgPrint(DBG_PREFIX "  Open [%u] at 0x%p\n", o, (PVOID)oEntry->OpenBlock);
                DbgPrint(DBG_PREFIX "    Protocol: %ws (Image: %ws) at 0x%p\n",
                    oEntry->ProtocolName[0] ? oEntry->ProtocolName : L"(null)",
                    oEntry->ProtocolImageName[0] ? oEntry->ProtocolImageName : L"(null)",
                    (PVOID)oEntry->ProtocolBlock);
            }
        }
    }

    DbgPrint(DBG_PREFIX "Enumeration complete. Total=%u, Filled=%u, BufferMax=%u\n",
        count, *ActualEntries, MaxEntries);

    if (count > MaxEntries) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    return STATUS_SUCCESS;
}

// ----------------------------------------------------------------------------
// 便捷封装：自动分配内存并枚举（调用者需 ExFreePoolWithTag）
// ----------------------------------------------------------------------------
NTSTATUS
NTAPI
NdisEnumMiniportsAlloc(
    _Out_ PNDIS_MINIPORT_ENUM_ENTRY* ppEntries,
    _Out_ PULONG EntryCount
)
{
    NTSTATUS status;
    ULONG needed = 0;
    ULONG actual = 0;
    PNDIS_MINIPORT_ENUM_ENTRY buffer = NULL;

    *ppEntries = NULL;
    *EntryCount = 0;

    // 第一次调用：获取数量
    status = NdisEnumMiniports(NULL, sizeof(NDIS_MINIPORT_ENUM_ENTRY), 0, &needed, &actual);
    if (status != STATUS_BUFFER_TOO_SMALL && needed == 0) {
        return status;
    }

    if (needed == 0)
        return STATUS_SUCCESS;

    buffer = (PNDIS_MINIPORT_ENUM_ENTRY)ExAllocatePool2(
        POOL_FLAG_NON_PAGED | POOL_FLAG_UNINITIALIZED,
        needed * sizeof(NDIS_MINIPORT_ENUM_ENTRY),
        NDIS_ENUM_TAG
    );
    if (!buffer)
        return STATUS_INSUFFICIENT_RESOURCES;

    status = NdisEnumMiniports(buffer, sizeof(NDIS_MINIPORT_ENUM_ENTRY), needed, &needed, &actual);
    if (!NT_SUCCESS(status)) {
        ExFreePoolWithTag(buffer, NDIS_ENUM_TAG);
        return status;
    }

    *ppEntries = buffer;
    *EntryCount = actual;
    return STATUS_SUCCESS;
}