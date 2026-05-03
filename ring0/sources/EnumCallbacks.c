#pragma once
#include "EnumCallbacks.h"
#include "Symbol.h"

typedef struct _FLT_FILTER_OFFSETS {
	LONG Operations;
	LONG Name;
	LONG Altitude;
	LONG DriverObject;
	LONG FilterUnload;
	LONG InstanceSetup;
	LONG InstanceQueryTeardown;
	LONG InstanceTeardownStart;
	LONG InstanceTeardownComplete;
	LONG PreVolumeMount;
	LONG PostVolumeMount;
	LONG GenerateFileName;
	LONG NormalizeNameComponent;
	LONG NormalizeNameComponentEx;
	LONG NormalizeContextCleanup;
	BOOLEAN Valid;
} FLT_FILTER_OFFSETS, * PFLT_FILTER_OFFSETS;

//=============================================================================
// 版本适配配置
//=============================================================================
typedef struct _WFP_VERSION_CONFIG {
	ULONG MaxCalloutIdOffset;   // WFP_GLOBAL 中 MaxCalloutId 的偏移
	ULONG CalloutTableOffset;    // WFP_GLOBAL 中 CalloutTable 的偏移
	ULONG CalloutEntrySize;      // 单个 CALLOUT_ENTRY 的大小
} WFP_VERSION_CONFIG, * PWFP_VERSION_CONFIG;

// 预定义 Win10/Win11 配置
WFP_VERSION_CONFIG g_Win10Config = {
	.MaxCalloutIdOffset = 0x190,
	.CalloutTableOffset = 0x198,
	.CalloutEntrySize = 0x50
};

WFP_VERSION_CONFIG g_Win11Config = {
	.MaxCalloutIdOffset = 0x198,
	.CalloutTableOffset = 0x1A0,
	.CalloutEntrySize = 0x60
};

//=============================================================================
// 1. 内核 CALLOUT_ENTRY 结构 (大小 0x50，从逆向分析得来)
//=============================================================================
typedef struct _CALLOUT_ENTRY {
	UINT32  char0;              // 0x00
	UINT32  IsUsed;             // 0x04
	UINT32  field_8;            // 0x08
	UINT32  field_C;            // 0x0C
	PVOID   classifyFn;         // 0x10
	PVOID   notifyFn;           // 0x18
	PVOID   flowDeleteFn;       // 0x20
	PVOID   classifyFnFast;     // 0x28
	UINT32  flags;              // 0x30
	UINT32  field_34;           // 0x34
	UINT32  field_38;           // 0x38
	UINT32  field_3C;           // 0x3C
	PVOID   deviceObject;       // 0x40
	UINT8   byte48;             // 0x48
	UINT8   byte49;             // 0x49
	UINT16  word4A;             // 0x4A
	UINT32  dword4C;            // 0x4C
} CALLOUT_ENTRY, * PCALLOUT_ENTRY;

// 内核全局结构（只取所需字段，偏移硬编码，适用于 Windows 10 19041+）
typedef struct _WFP_GLOBAL {
	// ... 大量字段 ...
	UINT32  MaxCalloutId;       // 偏移 0x190
	PVOID   CalloutTable;       // 偏移 0x198
	// ...
} WFP_GLOBAL, * PWFP_GLOBAL;

//=============================================================================
// 2. 映射表结构
//=============================================================================
#define MAX_CALLOUT_MAP 2048

// 内核映射：calloutId -> 回调
typedef struct _KERNEL_CALLOUT_MAP {
	GUID    CalloutGuid;        // ✅ 你需要的 Callout GUID
	UINT32  CalloutId;          // Callout ID
	PVOID   ClassifyFn;         // 分类回调
	PVOID   NotifyFn;           // 通知回调
	PVOID   FlowDeleteFn;       // 流删除回调
	WCHAR   Name[256];          // ✅ 你需要的 Callout 名称
} KERNEL_CALLOUT_MAP, *PKERNEL_CALLOUT_MAP;

// 完整映射：GUID -> (calloutId, 回调)
typedef struct _FULL_CALLOUT_MAP {
	GUID    CalloutGuid;
	ULONG   CalloutId;
	PVOID   ClassifyFn;
	PVOID   NotifyFn;
	PVOID   FlowDeleteFn;
} FULL_CALLOUT_MAP;

KERNEL_CALLOUT_MAP g_KernelMap[MAX_CALLOUT_MAP];
ULONG g_KernelMapCount = 0;

FULL_CALLOUT_MAP g_FullMap[MAX_CALLOUT_MAP];
ULONG g_FullMapCount = 0;

NTKERNELAPI NTSTATUS ObReferenceObjectByName(
	IN PUNICODE_STRING ObjectName,
	IN ULONG Attributes,
	IN PACCESS_STATE PassedAccessState OPTIONAL,
	IN ACCESS_MASK DesiredAccess OPTIONAL,
	IN POBJECT_TYPE ObjectType,
	IN KPROCESSOR_MODE AccessMode,
	IN OUT PVOID ParseContext OPTIONAL,
	OUT PVOID* Object
);

CALLBACK_LOOKUP_TABLE g_CallbackTable = { 0 };

// ============================================================================
// 版本检测函数（精准区分Win10/Win11）
// ============================================================================
BOOLEAN IsWindows11OrGreater() {
	RTL_OSVERSIONINFOW osVersion = { sizeof(RTL_OSVERSIONINFOW) };
	NTSTATUS status = RtlGetVersion(&osVersion);
	if (!NT_SUCCESS(status)) {
		return FALSE;
	}
	// Win11 内部版本号从 22000 开始，Win10 最高为 19045
	return (osVersion.dwMajorVersion == 10 && osVersion.dwBuildNumber >= 22000)
		|| (osVersion.dwMajorVersion > 10);
}

PVOID FindPspCreateProcessNotifyRoutine()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PspCreateProcessNotifyRoutine", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PspCreateProcessNotifyRoutine=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING PsSetCreateProcessNotifyRoutineName = RTL_CONSTANT_STRING(L"PsSetCreateProcessNotifyRoutine");
	PVOID PsSetCreateProcessNotifyRoutineAddr = MmGetSystemRoutineAddress(&PsSetCreateProcessNotifyRoutineName);
	if (NULL == PsSetCreateProcessNotifyRoutineAddr)
	{
		DbgPrint("PsSetCreateProcessNotifyRoutineAddr is NULL");
		return NULL;
	}
	DbgPrint("PsSetCreateProcessNotifyRoutineAddr: %p", PsSetCreateProcessNotifyRoutineAddr);
	//4c8d2d
	// ---------------------------------------------------
	// LyShark 开始定位特征

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)PsSetCreateProcessNotifyRoutineAddr;

	// 设置搜索长度
	ULONG size = 0x20;

	// 指定特征码
	UCHAR pSpecialCode[256] = { 0 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 1;

	pSpecialCode[0] = 0xe8;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode1(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("PspSetCreateProcessNotifyRoutineAddr is NULL");
		// 打印前30字节
		for (int i = 0; i < 30 && ((PUCHAR)PsSetCreateProcessNotifyRoutineAddr)[i] != 0xc3;i++)
			DbgPrint("%02X", ((PUCHAR)PsSetCreateProcessNotifyRoutineAddr)[i]);
		return NULL;
	}
	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 1);
	PVOID PspSetCreateProcessNotifyRoutineAddr = (PVOID)((PUCHAR)result + 5 + offset);

	DbgPrint("PspSetCreateProcessNotifyRoutineAddr首地址: 0x%p \n", PspSetCreateProcessNotifyRoutineAddr);
	/////////////////////////////////////////////////
	// 设置起始位置
	StartSearchAddress = (PUCHAR)PspSetCreateProcessNotifyRoutineAddr;

	// 设置搜索长度
	size = 0xc4;

	// 指定特征码长度
	ulSpecialCodeLength = 3;

	pSpecialCode[0] = 0x4c;
	pSpecialCode[1] = 0x8d;
	pSpecialCode[2] = 0x2d;

	// 开始搜索,找到后返回首地址
	result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("PspCreateProcessNotifyRoutineAddr is NULL");
		return NULL;
	}
	// 计算目标地址
	offset = *(PULONG)((PUCHAR)result + 3);
	PVOID PspCreateProcessNotifyRoutineAddr = (PVOID)((PUCHAR)result + 7 + offset);

	//DbgPrint("PspCreateProcessNotifyRoutineAddr首地址: 0x%p \n", PspCreateProcessNotifyRoutineAddr);
	return PspCreateProcessNotifyRoutineAddr;
}

PVOID FindPspCreateThreadNotifyRoutine()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PspCreateThreadNotifyRoutine", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PspCreateThreadNotifyRoutine=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING PsSetCreateThreadNotifyRoutineName = RTL_CONSTANT_STRING(L"PsSetCreateThreadNotifyRoutine");
	PVOID PsSetCreateThreadNotifyRoutineAddr = MmGetSystemRoutineAddress(&PsSetCreateThreadNotifyRoutineName);
	if (NULL == PsSetCreateThreadNotifyRoutineAddr)
	{
		DbgPrint("PsSetCreateThreadNotifyRoutineAddr is NULL");
		return NULL;
	}
	DbgPrint("PsSetCreateThreadNotifyRoutineAddr: %p", PsSetCreateThreadNotifyRoutineAddr);
	//4c8d2d
	// ---------------------------------------------------
	// LyShark 开始定位特征

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)PsSetCreateThreadNotifyRoutineAddr;

	// 设置搜索长度
	ULONG size = 0x10;

	// 指定特征码
	UCHAR pSpecialCode[256] = { 0 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 1;

	pSpecialCode[0] = 0xe8;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("PspSetCreateThreadNotifyRoutineAddr is NULL");
		// 打印前30字节
		for (int i = 0; i < 30 && ((PUCHAR)PsSetCreateThreadNotifyRoutineAddr)[i] != 0xc3; i++)
			DbgPrint("%02X", ((PUCHAR)PsSetCreateThreadNotifyRoutineAddr)[i]);
		return NULL;
	}
	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 1);
	PVOID PspSetCreateThreadNotifyRoutineAddr = (PVOID)((PUCHAR)result + 5 + offset);

	DbgPrint("PspSetCreateThreadNotifyRoutineAddr首地址: 0x%p \n", PspSetCreateThreadNotifyRoutineAddr);
	/////////////////////////////////////////////////
	// 设置起始位置
	StartSearchAddress = (PUCHAR)PspSetCreateThreadNotifyRoutineAddr;

	// 设置搜索长度
	size = 0xc4;

	// 指定特征码长度
	ulSpecialCodeLength = 3;

	pSpecialCode[0] = 0x48;
	pSpecialCode[1] = 0x8d;
	pSpecialCode[2] = 0x0d;

	// 开始搜索,找到后返回首地址
	result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("PspCreateThreadNotifyRoutineAddr is NULL");
		return NULL;
	}
	// 计算目标地址
	offset = *(PULONG)((PUCHAR)result + 3);
	PVOID PspCreateThreadNotifyRoutineAddr = (PVOID)((PUCHAR)result + 7 + offset);

	//DbgPrint("PspCreateThreadNotifyRoutineAddr首地址: 0x%p \n", PspCreateThreadNotifyRoutineAddr);
	return PspCreateThreadNotifyRoutineAddr;
}

PVOID FindPspLoadImageNotifyRoutine()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PspLoadImageNotifyRoutine", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PspLoadImageNotifyRoutine=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING PsSetLoadImageNotifyRoutineExName = RTL_CONSTANT_STRING(L"PsSetLoadImageNotifyRoutineEx");
	PVOID PsSetLoadImageNotifyRoutineExAddr = MmGetSystemRoutineAddress(&PsSetLoadImageNotifyRoutineExName);
	if (NULL == PsSetLoadImageNotifyRoutineExAddr)
	{
		DbgPrint("PsSetLoadImageNotifyRoutineExAddr is NULL");
		return NULL;
	}
	DbgPrint("PsSetLoadImageNotifyRoutineExAddr: %p", PsSetLoadImageNotifyRoutineExAddr);
	//4c8d2d
	// ---------------------------------------------------
	// LyShark 开始定位特征

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)PsSetLoadImageNotifyRoutineExAddr;

	// 设置搜索长度
	ULONG size = 0x5f;

	// 指定特征码
	UCHAR pSpecialCode[256] = { 0 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	pSpecialCode[0] = 0x48;
	pSpecialCode[1] = 0x8d;
	pSpecialCode[2] = 0x0d;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("PspLoadImageNotifyRoutineAddr is NULL");
		// 打印前30字节
		for (int i = 0; i < 30 && ((PUCHAR)PsSetLoadImageNotifyRoutineExAddr)[i] != 0xc3; i++)
			DbgPrint("%02X", ((PUCHAR)PsSetLoadImageNotifyRoutineExAddr)[i]);
		return NULL;
	}
	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID PspLoadImageNotifyRoutineAddr = (PVOID)((PUCHAR)result + 7 + offset);

	DbgPrint("PspLoadImageNotifyRoutine: 0x%p \n", PspLoadImageNotifyRoutineAddr);
	return PspLoadImageNotifyRoutineAddr;
}

PVOID FindCallbackListHead()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"CallbackListHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: CmCallbackListHead=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING CmUnRegisterCallbackName = RTL_CONSTANT_STRING(L"CmUnRegisterCallback");
	PVOID CmUnRegisterCallbackAddr = MmGetSystemRoutineAddress(&CmUnRegisterCallbackName);
	if (NULL == CmUnRegisterCallbackAddr)
	{
		DbgPrint("CmUnRegisterCallbackAddr is NULL");
		return NULL;
	}
	DbgPrint("CmUnRegisterCallbackAddr: %p", CmUnRegisterCallbackAddr);
	//4c8d2d
	// ---------------------------------------------------
	// LyShark 开始定位特征

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)CmUnRegisterCallbackAddr;

	// 设置搜索长度
	ULONG size = 0x100;// 0x5f;

	// 指定特征码
	UCHAR pSpecialCode[256] = { 0 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	pSpecialCode[0] = 0x48;
	pSpecialCode[1] = 0x8d;
	pSpecialCode[2] = 0x0d;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode1(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	if (NULL == result)
	{
		DbgPrint("CmCallbackListHead is NULL");
		// 打印完整函数
		//for (int i = 0; i < 0x600 && ((PUCHAR)CmUnRegisterCallbackAddr)[i] != 0xc3; i++)
		//	DbgPrint("0x%Xi:%02X", i, ((PUCHAR)CmUnRegisterCallbackAddr)[i]);
		return NULL;
	}
	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID CmCallbackListHeadAddr = (PVOID)((PUCHAR)result + 7 + offset);

	DbgPrint("CmCallbackListHead: 0x%p \n", CmCallbackListHeadAddr);
	return CmCallbackListHeadAddr;
}

NTSTATUS ControlCallback(PVOID pCallbackFunc, PUCHAR OldCode, BOOLEAN Status)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;
	ULONG ulShellcodeLength = 2;
	UCHAR pShellcode[2] = { 0xC0, 0xC3 };

	// 分配 MDL
	pMdl = IoAllocateMdl(pCallbackFunc, ulShellcodeLength, FALSE, FALSE, NULL);
	if (!pMdl)
	{
		DbgPrint("IoAllocateMdl failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 构建 MDL
	MmBuildMdlForNonPagedPool(pMdl);

	// 映射内存
	pVoid = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
	if (!pVoid)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPagesSpecifyCache failed");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 初始化并获取自旋锁
	KSPIN_LOCK mySpinLock;
	KeInitializeSpinLock(&mySpinLock);
	KIRQL oldIrql;
	KeAcquireSpinLock(&mySpinLock, &oldIrql);

	// 检查 MDL 大小是否足够
	if (MmGetMdlByteCount(pMdl) < ulShellcodeLength) {
		DbgPrint("memcpy skipped: MmGetMdlByteCount(pMdl) < ulShellcodeLength");
		status = STATUS_BUFFER_TOO_SMALL;
		goto Cleanup;
	}

	__try {
		if (Status) {
			memcpy(OldCode, pVoid, ulShellcodeLength);
			memcpy(pVoid, pShellcode, ulShellcodeLength);
                
        }
		else
		{
			// 恢复原始代码
			memcpy(pVoid, OldCode, ulShellcodeLength);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("ControlCallback except.status: %X", GetExceptionCode());
		status = GetExceptionCode();
	}

	Cleanup:

	// 释放自旋锁
	KeReleaseSpinLock(&mySpinLock, oldIrql);

	// 取消映射并释放 MDL
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return status;
}

static BOOLEAN ResolveFltFilterOffsets(PFLT_FILTER_OFFSETS Offsets)
{
	NTSTATUS status;
	RtlZeroMemory(Offsets, sizeof(*Offsets));

#define QUERY(member, field) \
    status = KernelQueryStructOffset(L"fltmgr.sys", L"_FLT_FILTER", member, &Offsets->field); \
    if (!NT_SUCCESS(status)) goto fail;

	QUERY(L"Operations", Operations);
	QUERY(L"Name", Name);
	QUERY(L"DefaultAltitude", Altitude);
	QUERY(L"DriverObject", DriverObject);
	QUERY(L"FilterUnload", FilterUnload);
	QUERY(L"InstanceSetup", InstanceSetup);
	QUERY(L"InstanceQueryTeardown", InstanceQueryTeardown);
	QUERY(L"InstanceTeardownStart", InstanceTeardownStart);
	QUERY(L"InstanceTeardownComplete", InstanceTeardownComplete);
	QUERY(L"PreVolumeMount", PreVolumeMount);
	QUERY(L"PostVolumeMount", PostVolumeMount);
	QUERY(L"GenerateFileName", GenerateFileName);
	QUERY(L"NormalizeNameComponent", NormalizeNameComponent);
	QUERY(L"NormalizeNameComponentEx", NormalizeNameComponentEx);
	QUERY(L"NormalizeContextCleanup", NormalizeContextCleanup);

#undef QUERY

	Offsets->Valid = TRUE;
	return TRUE;

fail:
	DbgPrint("Symbol offset resolve failed, fallback to hardcode\n");
	return FALSE;
}

static VOID FallbackFltOffsets(PFLT_FILTER_OFFSETS Offsets)
{
	RTL_OSVERSIONINFOEXW OSVersion = { 0 };
	OSVersion.dwOSVersionInfoSize = sizeof(OSVersion);
	RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

	if (OSVersion.dwMajorVersion >= 10 && OSVersion.dwBuildNumber >= 26100) {

		Offsets->Operations = 0x1B0;
		Offsets->Name = 0x40;
		Offsets->Altitude = 0x50;
		Offsets->DriverObject = 0x68;
		Offsets->FilterUnload = 0x108;
		Offsets->InstanceSetup = 0x110;
		Offsets->InstanceQueryTeardown = 0x118;
		Offsets->InstanceTeardownStart = 0x120;
		Offsets->InstanceTeardownComplete = 0x128;
		Offsets->PreVolumeMount = 0x170;
		Offsets->PostVolumeMount = 0x178;
		Offsets->GenerateFileName = 0x180;
		Offsets->NormalizeNameComponent = 0x188;
		Offsets->NormalizeNameComponentEx = 0x190;
		Offsets->NormalizeContextCleanup = 0x198;

	}
	else {

		Offsets->Operations = 0x1A8;
		Offsets->Name = 0x38;
		Offsets->Altitude = 0x48;
		Offsets->DriverObject = 0x60;
		Offsets->FilterUnload = 0x100;
		Offsets->InstanceSetup = 0x108;
		Offsets->InstanceQueryTeardown = 0x110;
		Offsets->InstanceTeardownStart = 0x118;
		Offsets->InstanceTeardownComplete = 0x120;
		Offsets->PreVolumeMount = 0x168;
		Offsets->PostVolumeMount = 0x170;
		Offsets->GenerateFileName = 0x178;
		Offsets->NormalizeNameComponent = 0x180;
		Offsets->NormalizeNameComponentEx = 0x188;
		Offsets->NormalizeContextCleanup = 0x190;
	}

	Offsets->Valid = TRUE;
}

ULONG EnumMiniFilter(PMINIFILTER_OBJECT* Array, PULONG InOutCount)
{
	static FLT_FILTER_OFFSETS gOffsets;
	static BOOLEAN gOffsetsInit = FALSE;

	static PMINIFILTER_OBJECT gCache = NULL;
	static ULONG gCacheCount = 0;
	static BOOLEAN gValid = FALSE;

	ULONG maxNum = 64;
	ULONG ulFilterListSize = 0;
	PFLT_FILTER* ppFilterList = NULL;
	ULONG i = 0;
	NTSTATUS status;

	PFLT_OPERATION_REGISTRATION op = NULL;

	if (!InOutCount)
		return 0;

	// =========================
	// ✔ 使用缓存（核心）
	// =========================
	if (gValid && gCache)
	{
		*Array = gCache;
		*InOutCount = gCacheCount;
		return gCacheCount;
	}

	// =========================
	// 获取 filter 数量
	// =========================
	FltEnumerateFilters(NULL, 0, &ulFilterListSize);

	ppFilterList = (PFLT_FILTER*)ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		ulFilterListSize * sizeof(PFLT_FILTER),
		'cbin'
	);

	if (!ppFilterList)
		return 0;

	status = FltEnumerateFilters(ppFilterList, ulFilterListSize, &ulFilterListSize);
	if (!NT_SUCCESS(status))
	{
		ExFreePoolWithTag(ppFilterList, 'cbin');
		return 0;
	}

	// =========================
	// 初始化 offsets（你已有逻辑，这里略）
	// =========================
	FLT_FILTER_OFFSETS Offsets;
	if (!gOffsetsInit)
	{
		if (!ResolveFltFilterOffsets(&gOffsets))
			FallbackFltOffsets(&gOffsets);

		gOffsetsInit = TRUE;
	}
	Offsets = gOffsets;

	// =========================
	// 分配 cache（一次性）
	// =========================
	PMINIFILTER_OBJECT cache =
		ExAllocatePool2(POOL_FLAG_NON_PAGED,
			sizeof(MINIFILTER_OBJECT) * ulFilterListSize,
			'mfin');

	if (!cache)
	{
		ExFreePoolWithTag(ppFilterList, 'cbin');
		return 0;
	}

	// =========================
	// 枚举
	// =========================
	for (i = 0; i < ulFilterListSize; i++)
	{
		__try
		{
			PUCHAR base = (PUCHAR)ppFilterList[i];

			cache[i].hFilter = ppFilterList[i];

			RtlStringCbCopyUnicodeString(
				cache[i].Name,
				sizeof(cache[i].Name),
				(PUNICODE_STRING)(base + Offsets.Name)
			);

			RtlStringCbCopyUnicodeString(
				cache[i].Altitude,
				sizeof(cache[i].Altitude),
				(PUNICODE_STRING)(base + Offsets.Altitude)
			);

			PDRIVER_OBJECT drv =
				*(PDRIVER_OBJECT*)(base + Offsets.DriverObject);

			if (drv && drv->DriverSection)
			{
				PLDR_DATA_TABLE_ENTRY ldr =
					(PLDR_DATA_TABLE_ENTRY)drv->DriverSection;

				if (ldr->FullDllName.Buffer)
				{
					RtlStringCbCopyW(
						cache[i].Path,
						sizeof(cache[i].Path),
						ldr->FullDllName.Buffer
					);
				}
			}

			// callbacks
			cache[i].FilterFunc[0] = *(PVOID*)(base + Offsets.FilterUnload);
			cache[i].FilterFunc[1] = *(PVOID*)(base + Offsets.InstanceSetup);
			cache[i].FilterFunc[2] = *(PVOID*)(base + Offsets.InstanceQueryTeardown);
			cache[i].FilterFunc[3] = *(PVOID*)(base + Offsets.InstanceTeardownStart);
			cache[i].FilterFunc[4] = *(PVOID*)(base + Offsets.InstanceTeardownComplete);
			cache[i].FilterFunc[5] = *(PVOID*)(base + Offsets.PreVolumeMount);
			cache[i].FilterFunc[6] = *(PVOID*)(base + Offsets.PostVolumeMount);
			cache[i].FilterFunc[7] = *(PVOID*)(base + Offsets.GenerateFileName);
			cache[i].FilterFunc[8] = *(PVOID*)(base + Offsets.NormalizeNameComponent);
			cache[i].FilterFunc[9] = *(PVOID*)(base + Offsets.NormalizeNameComponentEx);
			cache[i].FilterFunc[10] = *(PVOID*)(base + Offsets.NormalizeContextCleanup);

			op = *(PFLT_OPERATION_REGISTRATION*)(base + Offsets.Operations);

			cache[i].bMajorFunction = TRUE;

			ULONG safe = 0;

			while (op && safe < 0x80)
			{
				UCHAR mj = op->MajorFunction;

				if (mj == IRP_MJ_OPERATION_END)
					break;

				if (mj < IRP_MJ_MAXIMUM_FUNCTION)
				{
					cache[i].MajorFunction[mj].PreOperation =
						(PVOID)op->PreOperation;

					cache[i].MajorFunction[mj].PostOperation =
						(PVOID)op->PostOperation;
				}

				op = (PFLT_OPERATION_REGISTRATION)
					((PUCHAR)op + sizeof(FLT_OPERATION_REGISTRATION));

				safe++;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			cache[i].bMajorFunction = FALSE;
		}

		FltObjectDereference(ppFilterList[i]);
	}

	ExFreePoolWithTag(ppFilterList, 'cbin');

	// =========================
	// 写入 cache
	// =========================
	gCache = cache;
	gCacheCount = i;
	gValid = TRUE;

	*Array = gCache;
	*InOutCount = gCacheCount;

	return gCacheCount;
}

PVOID FindKeBugCheckCallbackListHead()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"KeBugCheckCallbackListHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: KeBugCheckCallbackListHead=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING KeRegisterBugCheckCallbackName = RTL_CONSTANT_STRING(L"KeRegisterBugCheckCallback");
	PVOID KeRegisterBugCheckCallbackAddr = MmGetSystemRoutineAddress(&KeRegisterBugCheckCallbackName);
	if (NULL == KeRegisterBugCheckCallbackAddr)
	{
		DbgPrint("KeRegisterBugCheckCallbackAddr is NULL");
		return NULL;
	}
	
	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)KeRegisterBugCheckCallbackAddr;

	// 设置搜索长度
	ULONG size = 0x70;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x4c,0x8d,0x05 };
	
	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);
	
	if (NULL == result)
	{
		DbgPrint("KeBugCheckCallbackListHead is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID KeBugCheckCallbackListHead = (PVOID)((PUCHAR)result + 7 + offset);

	return KeBugCheckCallbackListHead;
}

PVOID FindKeBugCheckReasonCallbackListHead()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"KeBugCheckReasonCallbackListHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: KeBugCheckReasonCallbackListHead=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING KeRegisterBugCheckReasonCallbackName = RTL_CONSTANT_STRING(L"KeRegisterBugCheckReasonCallback");
	PVOID KeRegisterBugCheckReasonCallbackAddr = MmGetSystemRoutineAddress(&KeRegisterBugCheckReasonCallbackName);
	if (NULL == KeRegisterBugCheckReasonCallbackAddr)
	{
		DbgPrint("KeRegisterBugCheckReasonCallbackAddr is NULL");
		return NULL;
	}

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)KeRegisterBugCheckReasonCallbackAddr;

	// 设置搜索长度
	ULONG size = 0x100;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x48,0x8d,0x3d };
	if (!IsWindows11OrGreater()) pSpecialCode[2] = 0x0d;

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("KeBugCheckReasonCallbackListHead is NULL");
		// 打印前0x100字节
		//for (int i = 0; i < 0x100 && ((PUCHAR)KeRegisterBugCheckReasonCallbackAddr)[i] != 0xc3; i++)
		//	DbgPrint("%X:%02X", i, ((PUCHAR)KeRegisterBugCheckReasonCallbackAddr)[i]);
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID KeBugCheckReasonCallbackListHead = (PVOID)((PUCHAR)result + 7 + offset);

	return KeBugCheckReasonCallbackListHead;
}

PVOID FindPspLegoNotifyRoutine()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PspLegoNotifyRoutine", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PspLegoNotifyRoutine=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING PsSetLegoNotifyRoutineName = RTL_CONSTANT_STRING(L"PsSetLegoNotifyRoutine");
	PVOID PsSetLegoNotifyRoutineAddr = MmGetSystemRoutineAddress(&PsSetLegoNotifyRoutineName);
	if (NULL == PsSetLegoNotifyRoutineAddr)
	{
		DbgPrint("PsSetLegoNotifyRoutineAddr is NULL");
		return NULL;
	}

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)PsSetLegoNotifyRoutineAddr;

	// 设置搜索长度
	ULONG size = 0x10;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x48,0x89,0x0d };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("PspLegoNotifyRoutine is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID PspLegoNotifyRoutine = (PVOID)((PUCHAR)result + 7 + offset);

	return PspLegoNotifyRoutine;
}

VOID FindIopNotifyShutdownQueueHead(PVOID* pIopNotifyShutdownQueueHead, PVOID* pIopNotifyLastChanceShutdownQueueHead)
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"IopNotifyShutdownQueueHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: IopNotifyShutdownQueueHead=%p\n", (PVOID)addr);
		*pIopNotifyShutdownQueueHead = (PVOID)addr;

		status = GetNtSymbolAddress(L"IopNotifyLastChanceShutdownQueueHead", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: IopNotifyLastChanceShutdownQueueHead=%p\n", (PVOID)addr);
			*pIopNotifyLastChanceShutdownQueueHead = (PVOID)addr;
			return;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING IoRegisterShutdownNotificationName = RTL_CONSTANT_STRING(L"IoRegisterShutdownNotification");
	PVOID IoRegisterShutdownNotificationAddr = MmGetSystemRoutineAddress(&IoRegisterShutdownNotificationName);
	if (NULL == IoRegisterShutdownNotificationAddr)
	{
		DbgPrint("IoRegisterShutdownNotificationAddr is NULL");
		return;
	}
	UNICODE_STRING IoRegisterLastChanceShutdownNotificationName = RTL_CONSTANT_STRING(L"IoRegisterLastChanceShutdownNotification");
	PVOID IoRegisterLastChanceShutdownNotificationAddr = MmGetSystemRoutineAddress(&IoRegisterLastChanceShutdownNotificationName);
	if (NULL == IoRegisterLastChanceShutdownNotificationAddr)
	{
		DbgPrint("IoRegisterLastChanceShutdownNotificationAddr is NULL");
		return;
	}

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)IoRegisterShutdownNotificationAddr;

	// 设置搜索长度
	ULONG size = 0x60;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x48,0x8d,0x0d };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode1(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("IopNotifyShutdownQueueHead is NULL");
		// 打印前30字节
		//for (int i = 0; i < 0x100 && ((PUCHAR)IoRegisterShutdownNotificationAddr)[i] != 0xc3; i++)
		//	DbgPrint("%X:%02X", i, ((PUCHAR)IoRegisterShutdownNotificationAddr)[i]);
		return;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	*pIopNotifyShutdownQueueHead = (PVOID)((PUCHAR)result + 7 + offset);

	StartSearchAddress = (PUCHAR)IoRegisterLastChanceShutdownNotificationAddr;
	result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("IopNotifyLastChanceShutdownQueueHead is NULL");
		return;
	}

	// 计算目标地址
	offset = *(PULONG)((PUCHAR)result + 3);
	*pIopNotifyLastChanceShutdownQueueHead = (PVOID)((PUCHAR)result + 7 + offset);
}

// 同时获取 普通版 + Ex版 登录会话终止回调链表头
NTSTATUS FindLogonSessionTerminatedHeads(
	OUT PVOID* NormalHead,
	OUT PVOID* ExHead
)
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"SeFileSystemNotifyRoutinesHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: SeFileSystemNotifyRoutinesHead=%p\n", (PVOID)addr);
		*NormalHead = (PVOID)addr;

		status = GetNtSymbolAddress(L"SeFileSystemNotifyRoutinesExHead", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: SeFileSystemNotifyRoutinesExHead=%p\n", (PVOID)addr);
			*ExHead = (PVOID)addr;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	PUCHAR pFunc = NULL;
	PUCHAR pSearch = NULL;
	ULONG offset = 0;

	// 初始化输出
	*NormalHead = NULL;
	*ExHead = NULL;

	// 1. 获取 SeRegisterLogonSessionTerminatedRoutine
	UNICODE_STRING FuncName = RTL_CONSTANT_STRING(L"SeRegisterLogonSessionTerminatedRoutine");
	pFunc = (PUCHAR)MmGetSystemRoutineAddress(&FuncName);
	if (!pFunc) return STATUS_NOT_FOUND;

	// --------------------------
	// 搜索 普通链表头: SeFileSystemNotifyRoutinesHead
	// --------------------------
	pSearch = (PUCHAR)SearchSpecialCode(pFunc, 0x90, (UCHAR*)"\x48\x8B\x05", 3);
	if (pSearch)
	{
		offset = *(PULONG)(pSearch + 3);
		*NormalHead = (PVOID)(pSearch + 7 + offset);
	}

	// --------------------------
	// 搜索 Ex 链表头: SeFileSystemNotifyRoutinesExHead
	// 特征码：mov rax, cs:SeFileSystemNotifyRoutinesExHead → 48 8B 05 ????
	// --------------------------
	UNICODE_STRING ExFuncName = RTL_CONSTANT_STRING(L"SeRegisterLogonSessionTerminatedRoutineEx");
	pFunc = (PUCHAR)MmGetSystemRoutineAddress(&ExFuncName);
	if (pFunc)
	{
		pSearch = (PUCHAR)SearchSpecialCode(pFunc, 0x90, (UCHAR*)"\x48\x8B\x05", 3);
		if (pSearch)
		{
			offset = *(PULONG)(pSearch + 3);
			*ExHead = (PVOID)(pSearch + 7 + offset);
		}
	}

	return (*NormalHead || *ExHead) ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

// ============================================================================
// 1. 查找设备类通知信息（Win11原逻辑100%保留，新增Win10兼容分支）
// ============================================================================
NTSTATUS FindDeviceClassNotifyInfo(PDEVICE_CLASS_NOTIFY_INFO pInfo) {
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PnpDeviceClassNotifyList", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PnpDeviceClassNotifyList=%p\n", (PVOID)addr);
		pInfo->Buckets = (PVOID)addr;

		status = GetNtSymbolAddress(L"PnpDeviceClassNotifyLock", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: PnpDeviceClassNotifyLock=%p\n", (PVOID)addr);
			pInfo->Lock = (PVOID)addr;
			pInfo->NumBuckets = 13;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	RtlZeroMemory(pInfo, sizeof(DEVICE_CLASS_NOTIFY_INFO));
	PVOID IoRegisterPlugPlayNotificationAddr = KernelGetProcAddress("ntoskrnl.exe", "IoRegisterPlugPlayNotification");
	if (!IoRegisterPlugPlayNotificationAddr) return STATUS_DLL_NOT_FOUND;

	BOOLEAN isWin11 = IsWindows11OrGreater();
	PVOID pLockIns = NULL;

	// -------------------------------------------------------------------------
	// 【Win11 分支】完全复用你原有的硬编码偏移和特征码，无任何修改
	// -------------------------------------------------------------------------
	if (isWin11) {
		UCHAR patternLock[] = { 0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8 };
		UCHAR maskLock[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01 };
		// 完全保留你原有的 +0x180 偏移
		pLockIns = SearchSpecialCodeWithMask(
			(PUCHAR)IoRegisterPlugPlayNotificationAddr + 0x180,
			0x100,
			patternLock,
			maskLock,
			sizeof(patternLock)
		);
	}
	// -------------------------------------------------------------------------
	// 【Win10 分支】匹配Win10专属特征码（KeAcquireGuardedMutex）
	// -------------------------------------------------------------------------
	else {
		// Win10 专属特征：lea rcx, [PnpDeviceClassNotifyLock] + call KeAcquireGuardedMutex
		UCHAR patternLock[] = { 0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8 };
		UCHAR maskLock[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01 };
		// Win10 特征在函数前半段，从函数基址搜索
		pLockIns = SearchSpecialCodeWithMask(
			(PUCHAR)IoRegisterPlugPlayNotificationAddr + 0x150,
			0x100,
			patternLock,
			maskLock,
			sizeof(patternLock)
		);
	}

	if (!pLockIns) return STATUS_NOT_FOUND;
	pInfo->Lock = (PVOID)CALCULATE_RIP_TARGET(pLockIns, 7);

	// 搜索桶数组基址（Win10/Win11通用，在锁指令附近搜索）
	UCHAR patternList[] = { 0x48, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC };
	UCHAR maskList[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
	PVOID pListIns = SearchSpecialCodeWithMask((PUCHAR)pLockIns, 0x100, patternList, maskList, sizeof(patternList));

	if (!pListIns) return STATUS_NOT_FOUND;
	pInfo->Buckets = (PLIST_ENTRY)CALCULATE_RIP_TARGET(pListIns, 7);
	pInfo->NumBuckets = 13; // 哈希桶数量Win10/Win11均固定为13
	return STATUS_SUCCESS;
}

// ============================================================================
// 2. 查找硬件配置文件通知信息（Win11原逻辑100%保留，新增Win10兼容分支）
// ============================================================================
NTSTATUS FindHwProfileNotifyInfo(PHWPROFILE_NOTIFY_INFO pInfo) {
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"PnpProfileNotifyList", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: PnpProfileNotifyList=%p\n", (PVOID)addr);
		pInfo->ListHead = (PVOID)addr;

		status = GetNtSymbolAddress(L"PnpHwProfileNotifyLock", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: PnpHwProfileNotifyLock=%p\n", (PVOID)addr);
			pInfo->Lock = (PVOID)addr;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	RtlZeroMemory(pInfo, sizeof(HWPROFILE_NOTIFY_INFO));
	PVOID IoRegisterPlugPlayNotificationAddr = KernelGetProcAddress("ntoskrnl.exe", "IoRegisterPlugPlayNotification");
	if (!IoRegisterPlugPlayNotificationAddr) return STATUS_DLL_NOT_FOUND;

	BOOLEAN isWin11 = IsWindows11OrGreater();
	PVOID pLockIns = NULL;

	// -------------------------------------------------------------------------
	// 【Win11 分支】完全复用你原有的硬编码偏移和特征码，无任何修改
	// -------------------------------------------------------------------------
	if (isWin11) {
		UCHAR patternLock[] = { 0x4C, 0x8D, 0x2D, 0xCC, 0xCC, 0xCC, 0xCC };
		UCHAR maskLock[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
		// 完全保留你原有的 +0x1a0000 超大偏移
		pLockIns = SearchSpecialCodeWithMask(
			(PUCHAR)IoRegisterPlugPlayNotificationAddr + 0x1a0000,
			0x10000,
			patternLock,
			maskLock,
			sizeof(patternLock)
		);
	}
	// -------------------------------------------------------------------------
	// 【Win10 分支】匹配Win10专属特征码
	// -------------------------------------------------------------------------
	else {
		// Win10 硬件配置文件锁特征：lea rcx, [PnpHwProfileNotifyLock] + call KeAcquireGuardedMutex
		UCHAR patternLock[] = { 0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8 };
		UCHAR maskLock[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01 };
		pLockIns = SearchSpecialCodeWithMask(
			(PUCHAR)IoRegisterPlugPlayNotificationAddr + 0x300,
			0x100,
			patternLock,
			maskLock,
			sizeof(patternLock)
		);
		// 兜底：Win10 也可能使用 lea r13 指令
		/*if (!pLockIns) {
			UCHAR patternLockAlt[] = { 0x4C, 0x8D, 0x2D, 0xCC, 0xCC, 0xCC, 0xCC };
			UCHAR maskLockAlt[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
			pLockIns = SearchSpecialCodeWithMask(
				(PUCHAR)IoRegisterPlugPlayNotificationAddr,
				0x800,
				patternLockAlt,
				maskLockAlt,
				sizeof(patternLockAlt)
			);
		}*/
	}

	if (!pLockIns) return STATUS_NOT_FOUND;
	pInfo->Lock = (PVOID)CALCULATE_RIP_TARGET(pLockIns, 7);

	// 搜索链表头（Win10/Win11通用）
	UCHAR patternList1[] = { 0x48, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC };
	UCHAR maskList[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
	PVOID pListIns = SearchSpecialCodeWithMask((PUCHAR)pLockIns, 0x100, patternList1, maskList, sizeof(patternList1));

	if (pListIns) {
		pInfo->ListHead = (PLIST_ENTRY)CALCULATE_RIP_TARGET(pListIns, 7);
	}
	else {
		DbgPrint("未找到链表头特征指令,尝试打印pLockIns开始的0x100字节:\n");
		for (int i = 0; i < 0x100 && ((PUCHAR)pLockIns)[i] != 0xc3; i++)
			DbgPrint("0x%X:%02X ", i, ((PUCHAR)pLockIns)[i]);
	}
	return STATUS_SUCCESS;
}

PVOID FindIopFsNotifyChangeQueueHead()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"IopFsNotifyChangeQueueHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: IopFsNotifyChangeQueueHead=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING IoRegisterFsRegistrationChangeName = RTL_CONSTANT_STRING(L"IoRegisterFsRegistrationChange");
	PVOID IoRegisterFsRegistrationChangeAddr = MmGetSystemRoutineAddress(&IoRegisterFsRegistrationChangeName);
	if (NULL == IoRegisterFsRegistrationChangeAddr)
	{
		DbgPrint("IoRegisterFsRegistrationChangeAddr is NULL");
		return NULL;
	}

	/*
	fffff801`a630f517 e814000000      call    nt!IoRegisterFsRegistrationChangeMountAware (fffff801`a630f530)
	*/

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0xe8 };

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(IoRegisterFsRegistrationChangeAddr, 0x10, pSpecialCode, 1);

	if (NULL == result)
	{
		DbgPrint("IoRegisterFsRegistrationChangeMountAware is NULL");
		// 打印前0x10字节
		//UCHAR bytRead[0x10] = { 0 };
		//DbgPrint("2Current IRQL:%d", KeGetCurrentIrql());
		/*for (int i = 0; i < 0x10 && ((PUCHAR)IoRegisterFsRegistrationChangeAddr)[i] != 0xc3; i++) {
			DbgPrint("0x%llX:%02X", (ULONG_PTR)IoRegisterFsRegistrationChangeAddr + i, ((PUCHAR)IoRegisterFsRegistrationChangeAddr)[i]);
			bytRead[i] = ((PUCHAR)IoRegisterFsRegistrationChangeAddr)[i];
		}*/
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 1);
	//offset = (ULONG64)(0xFFFFFFFF00000000ULL | (offset & 0xFFFFFFFFULL)); //为什么?
	//DbgPrint("result:0x%p,offset:0x%llx", result, offset);
	PVOID StartSearchAddress = (PVOID)((PUCHAR)result + 5 + offset);
	DbgPrint("IoRegisterFsRegistrationChangeMountAware: 0x%p", StartSearchAddress);

	// 指定特征码
	pSpecialCode[0] = 0x4c;
	pSpecialCode[1] = 0x8d;
	pSpecialCode[2] = 0x3d;

	// 开始搜索,找到后返回首地址
	result = SearchSpecialCode(StartSearchAddress, 0x100, pSpecialCode, 3);

	if (NULL == result)
	{
		DbgPrint("IopFsNotifyChangeQueueHead is NULL");
		// 打印前0x100字节
		//for (int i = 0; i < 0x100 && ((PUCHAR)IoRegisterFsRegistrationChangeAddr)[i] != 0xc3; i++)
		//	DbgPrint("%llX:%02X", i, ((PUCHAR)IoRegisterFsRegistrationChangeAddr)[i]);
		return NULL;
	}

	// 计算目标地址
	offset = *(PULONG)((PUCHAR)result + 3);
	return (PVOID)((PUCHAR)result + 7 + offset);
}

BOOLEAN FindExpCallbackList(PVOID* pListHead, PEX_PUSH_LOCK* pListLock)
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"ExpCallbackListHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: ExpCallbackListHead=%p\n", (PVOID)addr);
		pListHead = (PVOID)addr;

		status = GetNtSymbolAddress(L"ExpCallbackListLock", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: ExpCallbackListLock=%p\n", (PVOID)addr);
			*pListLock = (PVOID)addr;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UCHAR pSpecialCode[3] = { 0x48,0x89,0x0d };
	if (!IsWindows11OrGreater()) pSpecialCode[2] = 0x05;

	PVOID result = SearchSpecialCode((PUCHAR)ExCreateCallback, 0x200, pSpecialCode, 3);
	if (NULL == result)
	{
		DbgPrint("ExpCallbackListHead is NULL");
		return FALSE;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	DbgPrint("ExpCallbackListHead:0x%p", (PVOID)((PUCHAR)result + 7 + offset - 8));
	*pListHead = (PVOID)((PUCHAR)result + 7 + offset - 8);

	result = SearchSpecialCode((PUCHAR)result, 0x100, (PUCHAR)"\xe8", 1);
	if (NULL == result)
	{
		DbgPrint("ExpUnlockCallbackListExclusive is NULL");
		return FALSE;
	}

	// 计算目标地址
	LONG offset1 = *(PLONG)((PUCHAR)result + 1);
	PVOID ExpUnlockCallbackListExclusive = (PVOID)((PUCHAR)result + 5 + offset1);

	DbgPrint("ExpUnlockCallbackListExclusive:0x%p", ExpUnlockCallbackListExclusive);
	if (ExpUnlockCallbackListExclusive == NULL) return FALSE;

	// 在 ExpUnlockCallbackListExclusive 附近搜索锁地址
	if (!IsWindows11OrGreater())
	{
		pSpecialCode[0] = 0x48;
		pSpecialCode[1] = 0x8d;
		pSpecialCode[2] = 0x0d;
	}
	else {
		pSpecialCode[0] = 0x0f;
		pSpecialCode[1] = 0x0d;
		pSpecialCode[2] = 0x0d;
	}
	result = SearchSpecialCode((PUCHAR)ExpUnlockCallbackListExclusive, 0x50, pSpecialCode, 3);
	if (NULL == result)
	{
		DbgPrint("ExpCallbackListLock is NULL");
		return FALSE;
	}

	// 计算目标地址
	offset = *(PULONG)((PUCHAR)result + 3);
	result = (PVOID)((PUCHAR)result + 7 + offset);
	DbgPrint("ExpCallbackListLock:0x%p", result);
	if (result == NULL) return FALSE;

	*pListLock = (PEX_PUSH_LOCK)result;
	return TRUE;
}

// ==============================
// 【核心实现】查找 NMI 回调链表 + 自旋锁
// 严格匹配你提供的反汇编指令
// ==============================
NTSTATUS FindKiNmiCallbackList(
	OUT PVOID* KiNmiCallbackListHead,
	OUT PKSPIN_LOCK* KiNmiCallbackListLock
)
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"KiNmiCallbackListHead", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: KiNmiCallbackListHead=%p\n", (PVOID)addr);
		*KiNmiCallbackListHead = (PVOID)addr;

		status = GetNtSymbolAddress(L"KiNmiCallbackListLock", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: KiNmiCallbackListLock=%p\n", (PVOID)addr);
			*KiNmiCallbackListLock = (PVOID)addr;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING FuncName = RTL_CONSTANT_STRING(L"KeRegisterNmiCallback");
	PUCHAR pFunc = MmGetSystemRoutineAddress(&FuncName);

	if (!pFunc || !KiNmiCallbackListHead || !KiNmiCallbackListLock)
	{
		DbgPrint("[-] KeRegisterNmiCallback 未找到\n");
		return STATUS_NOT_FOUND;
	}

	PUCHAR pCode = NULL;
	LONG offset = 0;

	// ==============================================
	// 1. 搜索 KiNmiCallbackListLock (自旋锁)
	// 反汇编特征: lea rcx, KiNmiCallbackListLock → 48 8D 0D
	// ==============================================
	UCHAR sigLock[] = { 0x48, 0x8D, 0x0D };
	pCode = SearchSpecialCode(pFunc, 0x80, sigLock, sizeof(sigLock));
	if (!pCode) return STATUS_NOT_FOUND;

	offset = *(PLONG)(pCode + 3);
	*KiNmiCallbackListLock = (PKSPIN_LOCK)(pCode + 7 + offset);

	// ==============================================
	// 2. 搜索 KiNmiCallbackListHead (链表头)
	// 反汇编特征: mov rdx, cs:KiNmiCallbackListHead → 48 8B 15
	// ==============================================
	UCHAR sigList[] = { 0x48, 0x8B, 0x15 };
	pCode = SearchSpecialCode(pFunc, 0x80, sigList, sizeof(sigList));
	if (!pCode) return STATUS_NOT_FOUND;

	offset = *(PLONG)(pCode + 3);
	*KiNmiCallbackListHead = (PVOID)(pCode + 7 + offset);

	DbgPrint("[+] NMI链表: %p | NMI锁: %p\n", *KiNmiCallbackListHead, *KiNmiCallbackListLock);
	return STATUS_SUCCESS;
}

// ==============================
// 【修复】定位 DbgPrint 回调链表 + 自旋锁
// ==============================
NTSTATUS FindDbgPrintCallbackInfo(OUT PDBG_PRINT_INFO pInfo)
{
	if (!pInfo) return STATUS_INVALID_PARAMETER;

	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"RtlpDebugPrintCallbackList", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: RtlpDebugPrintCallbackList=%p\n", (PVOID)addr);
		pInfo->CallbackList = (PVOID)addr;

		status = GetNtSymbolAddress(L"RtlpDebugPrintCallbackLock", &addr);
		if (status == STATUS_SUCCESS && addr)
		{
			DbgPrint("Symbol: RtlpDebugPrintCallbackLock=%p\n", (PVOID)addr);
			pInfo->CallbackLock = (PVOID)addr;
			return STATUS_SUCCESS;
		}

	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	// 1. 获取导出函数 DbgSetDebugPrintCallback
	UNICODE_STRING funcName = RTL_CONSTANT_STRING(L"DbgSetDebugPrintCallback");
	PUCHAR pDbgSet = MmGetSystemRoutineAddress(&funcName);
	if (!pDbgSet) return STATUS_NOT_FOUND;

	// 2. 定位 DbgpInsertDebugPrintCallback（call 指令）
	PUCHAR pCall = SearchSpecialCode(pDbgSet, 0x50, (UCHAR*)"\xE8", 1);
	if (!pCall) return STATUS_NOT_FOUND;

	LONG offset = *(PLONG)(pCall + 1);
	PUCHAR pInsertFunc = pCall + 5 + offset;

	// 3. 【关键】第一次搜索：RtlpDebugPrintCallbackLock（自旋锁）
	UCHAR sigLea[] = { 0x48, 0x8D, 0x0D }; // lea rcx, [xxx]
	PUCHAR pLockLea = SearchSpecialCode(pInsertFunc, 0x100, sigLea, sizeof(sigLea));
	if (!pLockLea) return STATUS_NOT_FOUND;

	offset = *(PLONG)(pLockLea + 3);
	pInfo->CallbackLock = (PEX_SPIN_LOCK)(pLockLea + 7 + offset);

	// 4. 【关键】跳过锁，第二次搜索：RtlpDebugPrintCallbackList（链表头）
	PUCHAR pListLea = SearchSpecialCode(pLockLea + 8, 0x100, sigLea, sizeof(sigLea));
	if (!pListLea) return STATUS_NOT_FOUND;

	offset = *(PLONG)(pListLea + 3);
	pInfo->CallbackList = (PLIST_ENTRY)(pListLea + 7 + offset);

	DbgPrint("[+] DbgPrint 链表: %p | 自旋锁: %p\n", pInfo->CallbackList, pInfo->CallbackLock);
	return STATUS_SUCCESS;
}

// 搜索 KiBoundsCallback 全局指针
PVOID FindKiBoundsCallback()
{
	ULONG64 addr = 0;
	NTSTATUS status = GetNtSymbolAddress(L"KiBoundsCallback", &addr);
	if (status == STATUS_SUCCESS && addr)
	{
		DbgPrint("Symbol: KiBoundsCallback=%p\n", (PVOID)addr);
		return (PVOID)addr;
	}
	DbgPrint("Symbol failed, fallback to pattern scan\n");
	// -------------------------------------------
	UNICODE_STRING FuncName = RTL_CONSTANT_STRING(L"KeRegisterBoundCallback");
	PUCHAR pFunc = (PUCHAR)MmGetSystemRoutineAddress(&FuncName);
	if (!pFunc)
		return NULL;

	// 特征码：lea rcx, KiBoundsCallback → 48 8D 0D
	UCHAR sig[] = { 0x48, 0x8D, 0x0D };
	PUCHAR pPos = SearchSpecialCode(pFunc, 0x60, sig, sizeof(sig));
	if (!pPos)
		return NULL;

	LONG offset = *(PLONG)(pPos + 3);
	addr = (ULONG64)(pPos + 7 + offset);

	DbgPrint("[+] KiBoundsCallback = %llx\n", addr);
	return (PVOID)addr;
}

// 4字节Tag转可读字符串（如 0x6C6C6143 → "llaC"）
VOID TagToString(ULONG Tag, PSTR Buffer, ULONG BufferSize)
{
	if (!Buffer || BufferSize < 5) return;
	Buffer[0] = (CHAR)(Tag & 0xFF);
	Buffer[1] = (CHAR)((Tag >> 8) & 0xFF);
	Buffer[2] = (CHAR)((Tag >> 16) & 0xFF);
	Buffer[3] = (CHAR)((Tag >> 24) & 0xFF);
	Buffer[4] = '\0';

	// 过滤不可打印字符
	for (INT i = 0; i < 4; i++) {
		if (!isprint(Buffer[i])) {
			Buffer[0] = '\0';
			break;
		}
	}
}

// 生成规范化匿名Callback名称
VOID GenerateAnonymousCallbackName(
	PCALLBACK_OBJECT pCbObj,
	PWSTR OutName,
	ULONG NameSize
)
{
	CHAR TagStr[5] = { 0 };
	TagToString(pCbObj->Tag, TagStr, sizeof(TagStr));

	// 规则1：有有效Tag → 用Tag
	//if (TagStr[0] != '\0') {
	//	RtlStringCbPrintfW(OutName, NameSize, L"\\Callback\\Anonymous_Tag(%hs)", TagStr);
	//}
	// 规则2：无Tag → 用8位短地址（安全简洁）
	//else {
		ULONG64 ShortAddr = (ULONG64)pCbObj & 0xFFFFFFFF;
		RtlStringCbPrintfW(OutName, NameSize, L"\\Callback\\Unnamed_Callback_%08llX", ShortAddr);
	//}
}

NTSTATUS InitializeCallbackTable() {
	NTSTATUS status;
	HANDLE hCallbackDir = NULL;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING callbackDirPath = RTL_CONSTANT_STRING(L"\\Callback");

	// CRITICAL: IRQL检查
	if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
		return STATUS_INVALID_LEVEL;
	}

	// 初始化自旋锁
	KeInitializeSpinLock(&g_CallbackTable.Lock);

	// 打开Callback目录
	InitializeObjectAttributes(&oa, &callbackDirPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
	status = ZwOpenDirectoryObject(&hCallbackDir, DIRECTORY_QUERY, &oa);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to open \\Callback: 0x%X", status);
		return status;
	}

	// 首次分配（16个条目）
	g_CallbackTable.Capacity = 16;
	g_CallbackTable.Entries = (PCALLBACK_ENTRY)ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(CALLBACK_ENTRY) * g_CallbackTable.Capacity,
		'cTbl'
	);

	if (!g_CallbackTable.Entries) {
		ZwClose(hCallbackDir);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	// 枚举并填充表
	PDIRECTORY_BASIC_INFORMATION pBuffer = NULL;
	ULONG ulLength = 0x800;
	ULONG ulContext = 0;
	ULONG ulRet = 0;

	do {
		if (pBuffer) ExFreePoolWithTag(pBuffer, 'pbuf');
		ulLength *= 2;
		pBuffer = (PDIRECTORY_BASIC_INFORMATION)ExAllocatePool2(POOL_FLAG_NON_PAGED, ulLength, 'pbuf');
		if (!pBuffer) break;

		status = ZwQueryDirectoryObject(hCallbackDir, pBuffer, ulLength, FALSE, TRUE, &ulContext, &ulRet);
	} while (status == STATUS_MORE_ENTRIES);

	if (status == STATUS_SUCCESS) {
		PDIRECTORY_BASIC_INFORMATION pCurrent = pBuffer;
		POBJECT_TYPE* ExCallbackObjectType = FindExCallbackObjectType();

		if (ExCallbackObjectType) {
			while (pCurrent->ObjectName.Length != 0) {
				UNICODE_STRING objectPath;
				WCHAR pathBuffer[256];
				RtlInitEmptyUnicodeString(&objectPath, pathBuffer, sizeof(pathBuffer));
				RtlAppendUnicodeToString(&objectPath, L"\\Callback\\");
				RtlAppendUnicodeStringToString(&objectPath, &pCurrent->ObjectName);

				PCALLBACK_OBJECT callbackObj;
				status = ObReferenceObjectByName(&objectPath, OBJ_CASE_INSENSITIVE, NULL, 0,
					*ExCallbackObjectType, KernelMode, NULL, (PVOID*)&callbackObj);

				if (NT_SUCCESS(status)) {
					// 动态扩容
					if (g_CallbackTable.Count >= g_CallbackTable.Capacity) {
						ULONG newCapacity = g_CallbackTable.Capacity * 2;
						PCALLBACK_ENTRY newEntries = (PCALLBACK_ENTRY)ExAllocatePool2(
							POOL_FLAG_NON_PAGED,
							sizeof(CALLBACK_ENTRY) * newCapacity,
							'cTbl'
						);
						if (newEntries) {
							RtlCopyMemory(newEntries, g_CallbackTable.Entries,
								sizeof(CALLBACK_ENTRY) * g_CallbackTable.Count);
							ExFreePoolWithTag(g_CallbackTable.Entries, 'cTbl');
							g_CallbackTable.Entries = newEntries;
							g_CallbackTable.Capacity = newCapacity;
						}
					}

					// 添加条目（在自旋锁保护下）
					if (g_CallbackTable.Count < g_CallbackTable.Capacity) {
						KIRQL oldIrql;
						KeAcquireSpinLock(&g_CallbackTable.Lock, &oldIrql);

						PCALLBACK_ENTRY entry = &g_CallbackTable.Entries[g_CallbackTable.Count++];
						entry->CallbackObject = callbackObj; // 保留引用！
						RtlStringCbCopyUnicodeString(entry->Name, sizeof(entry->Name), &objectPath);

						//DbgPrint("Table[%d]: %p -> %ws", g_CallbackTable.Count - 1, entry->CallbackObject, entry->Name);

						KeReleaseSpinLock(&g_CallbackTable.Lock, oldIrql);
					}

					// 注意：这里不ObDereferenceObject，保留引用防止对象释放
				}

				pCurrent++;
			}
		}
	}

	if (pBuffer) ExFreePoolWithTag(pBuffer, 'pbuf');
	ZwClose(hCallbackDir);

	g_CallbackTable.Initialized = TRUE;
	return STATUS_SUCCESS;
}

VOID CleanupCallbackTable() {
	if (!g_CallbackTable.Entries) return;

	// 释放所有保留的对象引用
	for (ULONG i = 0; i < g_CallbackTable.Count; i++) {
		if (g_CallbackTable.Entries[i].CallbackObject) {
			ObDereferenceObject(g_CallbackTable.Entries[i].CallbackObject);
		}
	}

	ExFreePoolWithTag(g_CallbackTable.Entries, 'cTbl');
	RtlZeroMemory(&g_CallbackTable, sizeof(g_CallbackTable));
}

NTSTATUS QueryCallbackNameByPointer_Fast(PVOID pObject, PWCHAR OutName, ULONG NameLength) {
	if (!g_CallbackTable.Initialized || !g_CallbackTable.Entries) {
		return STATUS_NOT_FOUND;
	}

	// 线性搜索（无需哈希表，简单直接）
	for (ULONG i = 0; i < g_CallbackTable.Count; i++) {
		if (g_CallbackTable.Entries[i].CallbackObject == pObject) {
			RtlStringCbCopyW(OutName, NameLength, g_CallbackTable.Entries[i].Name);
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

ULONG EnumCallbacks(PCallbackInfo* pArray)
{
	PCallbackInfo Array = *pArray;  // 解引用
	ULONG max_num = 512;
	ULONG k = 0;
	PCallbackInfo newArray = NULL;

#define MAX_TEMP 512
	PVOID* tempFunc = NULL;
	PVOID* tempCtx = NULL;
	PCALLBACK_OBJECT* tempCbObj = NULL;
	PCALLBACK_REGISTRATION* tempRegObj = NULL;

	PVOID* tempFunc3 = NULL;
	PVOID* tempCtx3 = NULL;

	PVOID* tempFunc2 = NULL;
	PVOID* tempCtx2 = NULL;

	PVOID* tempFunc1 = NULL;

	// 一次性分配堆内存，彻底解决栈溢出
	tempFunc = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * MAX_TEMP, 'stk1');
	tempCtx = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * MAX_TEMP, 'stk2');
	tempCbObj = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PCALLBACK_OBJECT) * MAX_TEMP, 'stk3');
	tempRegObj = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PCALLBACK_REGISTRATION) * MAX_TEMP, 'stkR');

	tempFunc3 = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * MAX_TEMP, 'stk4');
	tempCtx3 = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * MAX_TEMP, 'stk5');

	tempFunc2 = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * 64, 'stk6');
	tempCtx2 = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * 64, 'stk7');

	tempFunc1 = ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(PVOID) * 16, 'stk8');

	// 任意分配失败则直接禁用缓存，不崩溃
	if (!tempFunc || !tempCtx || !tempCbObj || !tempRegObj || !tempFunc3 || !tempCtx3 || !tempFunc2 || !tempCtx2 || !tempFunc1)
	{
		if (tempFunc) ExFreePool(tempFunc);
		if (tempCtx) ExFreePool(tempCtx);
		if (tempCbObj) ExFreePool(tempCbObj);
		if (tempRegObj) ExFreePool(tempRegObj);
		if (tempFunc3) ExFreePool(tempFunc3);
		if (tempCtx3) ExFreePool(tempCtx3);
		if (tempFunc2) ExFreePool(tempFunc2);
		if (tempCtx2) ExFreePool(tempCtx2);
		if (tempFunc1) ExFreePool(tempFunc1);

		tempFunc = tempCtx = tempFunc3 = tempCtx3 = tempFunc2 = tempCtx2 = tempFunc1 = NULL;
		tempCbObj = NULL;
		tempRegObj = NULL;
	}

	int tempCount3 = 0;
	int tempCnt2 = 0;
	int tempCnt = 0;

	POBJECT_TYPE pProcessType = *PsProcessType;
	POBJECT_TYPE pThreadType = *PsThreadType;
	//POBJECT_TYPE pDeskTopType = *ExDesktopObjectType;
	ULONG64 CallbackListOffset = 0;

	// 判断操作系统版本
	if (GetNtStructOffset(L"_OBJECT_TYPE", L"CallbackList", &CallbackListOffset) == STATUS_SUCCESS) {
		DbgPrint("Found CallbackList offset from PDB: 0x%llx\n", CallbackListOffset);
	}
	else {
		RTL_OSVERSIONINFOEXW OSVersion = { 0 };
		OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
		RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

		if (OSVersion.dwBuildNumber >= 9200)
			CallbackListOffset = 0xC8;
		else if (OSVersion.dwBuildNumber >= 7600)
			CallbackListOffset = 0xC0;
	}

	//if (!pProcessType) goto 

	POB_CALLBACK pProcessCallbackEntry = NULL;
	POB_CALLBACK pThreadCallbackEntry = NULL;
	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pThreadListHead = NULL;
	PLIST_ENTRY pCurrentEntry = NULL;

	// 1. 获取链表头（注意：这里获取的是 LIST_ENTRY 指针，不是第一个元素）
	pProcessListHead = (PLIST_ENTRY)((UCHAR*)pProcessType + CallbackListOffset);
	pThreadListHead = (PLIST_ENTRY)((UCHAR*)pThreadType + CallbackListOffset);
	
	__try {
		// =========================================================================
		// 枚举 Process 回调
		// =========================================================================
		// 从链表头的 Flink 开始遍历
		pCurrentEntry = pProcessListHead->Flink;

		while (pCurrentEntry != pProcessListHead) // 回到链表头则结束
		{
			// 【关键修复】使用 CONTAINING_RECORD 获取真正的 OB_CALLBACK 结构
			pProcessCallbackEntry = CONTAINING_RECORD(pCurrentEntry, OB_CALLBACK, ListEntry);

			// 【关键修复】有效性检查：确保 PreCall 或 PostCall 有一个不为空，且 ObTypeAddr 有效
			// 以此跳过链表头或未初始化的垃圾项
			if (MmIsAddressValid(pProcessCallbackEntry) &&
				pProcessCallbackEntry->ObjectType != NULL &&
				(pProcessCallbackEntry->PreOperation != NULL || pProcessCallbackEntry->PostOperation != NULL))
			{
				// 检查数组空间
				if (k + 1 > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					max_num += 100;
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * max_num, 'cbin');
					if (!Array) break; // 分配失败直接退出
					*pArray = Array;
				}
				//POB_OPERATION_REGISTRATION a;
				// 填充数据
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"ObProcessCallback(Pre)");
				RtlStringCbCopyW(Array[k + 1].Type, sizeof(Array[k + 1].Type), L"ObProcessCallback(Post)");
				//Array[k].Context = ((POB_REGISTRATION_BLOCK)pProcessCallbackEntry->RegistrationHandle)->Callbacks;
				Array[k].Others[0] = (ULONG64)pProcessCallbackEntry->RegistrationHandle;
				Array[k].Func = pProcessCallbackEntry->PreOperation;
				//Array[k + 1].Context = pProcessCallbackEntry->RegistrationContext;
				Array[k + 1].Others[0] = (ULONG64)pProcessCallbackEntry->RegistrationHandle;
				Array[k + 1].Func = pProcessCallbackEntry->PostOperation;

				DbgPrint("[Process] PreCall:0x%p, PostCall:0x%p\n", pProcessCallbackEntry->PreOperation, pProcessCallbackEntry->PostOperation);
				k += 2;
			}

			// 移动到下一个 LIST_ENTRY
			pCurrentEntry = pCurrentEntry->Flink;
		}

		// =========================================================================
		// 枚举 Thread 回调 (逻辑同上)
		// =========================================================================
		pCurrentEntry = pThreadListHead->Flink;

		while (pCurrentEntry != pThreadListHead)
		{
			pThreadCallbackEntry = CONTAINING_RECORD(pCurrentEntry, OB_CALLBACK, ListEntry);

			if (MmIsAddressValid(pThreadCallbackEntry) &&
				pThreadCallbackEntry->ObjectType != NULL &&
				(pThreadCallbackEntry->PreOperation != NULL || pThreadCallbackEntry->PostOperation != NULL))
			{
				if (k + 1 > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					max_num += 100;
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * max_num, 'cbin');
					if (!Array) break;
					*pArray = Array;
				}

				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"ObThreadCallback(Pre)");
				RtlStringCbCopyW(Array[k + 1].Type, sizeof(Array[k + 1].Type), L"ObThreadCallback(Post)");
				//Array[k].Context = pThreadCallbackEntry->RegistrationContext;
				Array[k].Others[0] = (ULONG64)pThreadCallbackEntry->RegistrationHandle;
				Array[k].Func = pThreadCallbackEntry->PreOperation;
				//Array[k + 1].Context = pThreadCallbackEntry->RegistrationContext;
				Array[k + 1].Others[0] = (ULONG64)pThreadCallbackEntry->RegistrationHandle;
				Array[k + 1].Func = pThreadCallbackEntry->PostOperation;
				
				DbgPrint("[Thread] PreCall:0x%p, PostCall:0x%p\n", pThreadCallbackEntry->PreOperation, pThreadCallbackEntry->PostOperation);
				k += 2;
			}

			pCurrentEntry = pCurrentEntry->Flink;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("[!] Exception: 0x%X\n", GetExceptionCode());
	}
	// 【结构体版本】枚举 CreateProcess 回调
	ULONG64 MagicPtr = 0;
	PEX_CALLBACK_ROUTINE_BLOCK pCallbackBlock = NULL;
	WCHAR szType[64] = { 0 };
	PVOID pCallbackContext = NULL;

	ULONG64 PspCreateProcessNotifyRoutine = (ULONG64)FindPspCreateProcessNotifyRoutine();
	if (!PspCreateProcessNotifyRoutine) goto exit1;
	
	__try
	{
		for (int i = 0; i < 64; i++)
		{
			// 1. 获取数组项指针
			MagicPtr = PspCreateProcessNotifyRoutine + i * 8;
			pCallbackBlock = *(PVOID*)MagicPtr;

			// 2. 清除低4位对齐位（Windows 回调固定格式）
			pCallbackBlock = (PEX_CALLBACK_ROUTINE_BLOCK)((ULONG64)pCallbackBlock & ~0xF);

			// 3. 地址合法性校验
			if (!pCallbackBlock || !MmIsAddressValid((PVOID)pCallbackBlock))
				continue;

			// 4. 结构体直接读取【函数地址】和【类型标志】
			if (!pCallbackBlock->Function || !MmIsAddressValid(pCallbackBlock->Function))
				continue;

			// ====================== 结构体直接判断类型 ======================
			switch (pCallbackBlock->Flags)
			{
			case 0:
				RtlStringCbCopyW(szType, sizeof(szType), L"CreateProcess");
				break;
			case 2:
				RtlStringCbCopyW(szType, sizeof(szType), L"CreateProcessEx");
				break;
			case 6:
				RtlStringCbCopyW(szType, sizeof(szType), L"CreateProcessEx2");
				pCallbackContext = pCallbackBlock->Context; // ✅ 读取Ex2上下文
				break;
			default:
				RtlStringCbCopyW(szType, sizeof(szType), L"Unknown");
				break;
			}

			// 5. 存入数组（和你原来逻辑完全一致）
			if (k >= max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;
			}

			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), szType);
			Array[k].Func = pCallbackBlock->Function;  // 结构体直接取值
			Array[k].Context = pCallbackContext; // Ex2上下文（其他类型为NULL）
			k++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Callback Enum Exception: 0x%X", GetExceptionCode());
	}
exit1:
	// ====================== 最终可用版（无错误，回调正常枚举） ======================
	MagicPtr = 0;
	pCallbackBlock = NULL;
	memset(szType, 0, sizeof(szType));

	ULONG64 PspCreateThreadNotifyRoutine = (ULONG64)FindPspCreateThreadNotifyRoutine();
	if (!PspCreateThreadNotifyRoutine) goto exit2;

	__try
	{
		for (int i = 0; i < 64; i++)
		{
			// 1. 原始正确逻辑
			MagicPtr = PspCreateThreadNotifyRoutine + i * 8;
			pCallbackBlock = *(PVOID*)MagicPtr;

			// 2. 清除低4位锁标记 + 对齐（内核固定规则，正确）
			pCallbackBlock = (PEX_CALLBACK_ROUTINE_BLOCK)((ULONG64)pCallbackBlock & ~0xF);

			// 3. 合法性校验
			if (!pCallbackBlock || !MmIsAddressValid((PVOID)pCallbackBlock))
				continue;
			if (!pCallbackBlock->Function || !MmIsAddressValid(pCallbackBlock->Function))
				continue;

			// ====================== 最终修正：放弃无法实现的类型区分 ======================
			// 内核不存储类型标记，统一显示即可（这是最真实、最稳定的结果）
			RtlStringCbCopyW(szType, sizeof(szType), L"CreateThread");

			// 4. 存储逻辑（完全不变）
			if (k >= max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;
			}

			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), szType);
			Array[k].Func = pCallbackBlock->Function;
			k++;

			// 正常调试打印
			DbgPrint("枚举成功 [%d] 回调地址: 0x%llX", i, (ULONG_PTR)pCallbackBlock->Function);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Thread Callback Enum Exception: 0x%X", GetExceptionCode());
	}
exit2:
	// 【结构体版本】枚举 LoadImageNotify 镜像加载回调
	MagicPtr = 0;
	pCallbackBlock = NULL;
	memset(szType, 0, sizeof(szType));

	ULONG64 PspLoadImageNotifyRoutine = (ULONG64)FindPspLoadImageNotifyRoutine();
	if (!PspLoadImageNotifyRoutine) goto exit3;

	__try
	{
		for (int i = 0; i < 64; i++)
		{
			// 1. 获取数组项指针
			MagicPtr = PspLoadImageNotifyRoutine + i * 8;
			pCallbackBlock = *(PVOID*)MagicPtr;

			// 2. 清除低4位对齐掩码（Windows 回调固定规则）
			pCallbackBlock = (PEX_CALLBACK_ROUTINE_BLOCK)((ULONG64)pCallbackBlock & ~0xF);

			// 3. 地址合法性校验
			if (!pCallbackBlock || !MmIsAddressValid((PVOID)pCallbackBlock))
				continue;

			// 4. 读取回调函数地址
			if (!pCallbackBlock->Function || !MmIsAddressValid(pCallbackBlock->Function))
				continue;

			// ====================== LoadImage 类型判断 ======================
			// LoadImage 只有一种：Flags = 0
			switch (pCallbackBlock->Flags)
			{
			case 0:
				RtlStringCbCopyW(szType, sizeof(szType), L"LoadImage");
				break;
			default:
				RtlStringCbCopyW(szType, sizeof(szType), L"LoadImageUnknown");
				break;
			}

			// 5. 存入数组（和你原有逻辑完全一致）
			if (k >= max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;
			}

			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), szType);
			Array[k].Func = pCallbackBlock->Function;
			k++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("LoadImage Callback Enum Exception: 0x%X", GetExceptionCode());
	}
exit3:
	//枚举CmpCallback
	// 遍历链表结构
	PCM_NOTIFY_ENTRY pNotifyEntry = NULL;

	PVOID pCallbackListHead = FindCallbackListHead();
	// 获取回调函数链表头地址
	if (NULL == pCallbackListHead) goto exit4;

	// 开始遍历双向链表
	pNotifyEntry = (PCM_NOTIFY_ENTRY)pCallbackListHead;
	do
	{
		// 判断pNotifyEntry地址是否有效
		if (FALSE == MmIsAddressValid(pNotifyEntry)) break;
		// 判断回调函数地址是否有效
		if (MmIsAddressValid(pNotifyEntry->Function))
		{
			//DbgPrint("回调函数地址: 0x%p | 回调函数Cookie: 0x%I64X \n", pNotifyEntry->Function, pNotifyEntry->Cookie.QuadPart);
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"CmpCallback");
			Array[k].Func = (PVOID)pNotifyEntry->Function;
			Array[k].Context = pNotifyEntry->Context;
			Array[k].Others[0] = pNotifyEntry->Cookie.QuadPart;
			//GetDriverNameByAddr((PVOID)pNotifyEntry->Function, Array[i].DriverName);
			k++;
		}
		// 获取下一链表
		pNotifyEntry = (PCM_NOTIFY_ENTRY)pNotifyEntry->ListEntryHead.Flink;

	} while (pCallbackListHead != (PVOID)pNotifyEntry);
exit4:
	//枚举LegoNotify
	PVOID PspLegoNotifyRoutine = FindPspLegoNotifyRoutine();
	if (!PspLegoNotifyRoutine) {
		DbgPrint("Failed to find PspLegoNotifyRoutine!\n");
		goto exit7;
	}
	if (k >= max_num)  // 修正边界
	{
		ULONG newSize = max_num + 100;
		newArray = (PCallbackInfo)ExAllocatePool2(
			POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

		if (!newArray) goto exit7;  // 必须检查

		if (Array && k > 0) {
			RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
		}

		ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
		Array = newArray;
		max_num = newSize;  // 直接赋值，避免 += 导致不一致
		*pArray = Array;
	}
	RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"Lego");
	Array[k].Func = PspLegoNotifyRoutine;
	k++;
exit7:
	//枚举ShutdownNotify
	PVOID IopNotifyShutdownQueueHead = NULL;
	PVOID IopNotifyLastChanceShutdownQueueHead = NULL;
	FindIopNotifyShutdownQueueHead(&IopNotifyShutdownQueueHead, &IopNotifyLastChanceShutdownQueueHead);
	if (!IopNotifyShutdownQueueHead) {
		DbgPrint("Failed to find IopNotifyShutdownQueueHead!\n");
		goto exit8;
	}
	PLIST_ENTRY pShutdownHead = (PLIST_ENTRY)IopNotifyShutdownQueueHead;
	PLIST_ENTRY pShutdownEntry = pShutdownHead;
	PSHUTDOWN_PACKET pShutdownPacket = NULL;

	__try
	{
		do {
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			pShutdownEntry = pShutdownEntry->Flink;
			pShutdownPacket = CONTAINING_RECORD(pShutdownEntry, SHUTDOWN_PACKET, ListEntry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"Shutdown(IRP_MJ_SHUTDOWN)");
			Array[k].Func = (PVOID)pShutdownPacket->DeviceObject->DriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
			k++;
		} while (pShutdownEntry->Flink != pShutdownHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit8:
	//枚举LastChanceShutdownNotify
	if (!IopNotifyLastChanceShutdownQueueHead) {
		DbgPrint("Failed to find IopNotifyLastChanceShutdownQueueHead!\n");
		goto exit9;
	}
	PLIST_ENTRY pLastChanceHead = (PLIST_ENTRY)IopNotifyLastChanceShutdownQueueHead;
	PLIST_ENTRY pLastChanceEntry = pLastChanceHead;
	PSHUTDOWN_PACKET pLastChanceShutdownPacket = NULL;
	__try
	{
		do {
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			pLastChanceEntry = pLastChanceEntry->Flink;
			pLastChanceShutdownPacket = CONTAINING_RECORD(pLastChanceEntry, SHUTDOWN_PACKET, ListEntry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"LastChanceShutdown(IRP_MJ_SHUTDOWN)");
			Array[k].Func = (PVOID)pLastChanceShutdownPacket->DeviceObject->DriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
			k++;
		} while (pLastChanceEntry->Flink != pLastChanceHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit9:
	// ==============================
	// 遍历：LogonSessionTerminated (普通 + EX 双版本)
	// ==============================
	PVOID NormalHead = NULL, ExHead = NULL;
	FindLogonSessionTerminatedHeads(&NormalHead, &ExHead);

	// ------------------------------
	// 1. 遍历普通版
	// ------------------------------
	if (NormalHead)
	{
		PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION pEntry = *(PVOID*)NormalHead;

		__try
		{
			while (pEntry && MmIsAddressValid(pEntry))
			{
				if (k >= max_num)  // 修正边界
				{
					ULONG newSize = max_num + 100;
					newArray = (PCallbackInfo)ExAllocatePool2(
						POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

					if (!newArray) break;  // 必须检查

					if (Array && k > 0) {
						RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
					}

					ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
					Array = newArray;
					max_num = newSize;  // 直接赋值，避免 += 导致不一致
					*pArray = Array;
				}

				// 明确标注类型，R3 能看懂
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"LogonSessionTerminated(Normal)");
				Array[k].Func = pEntry->CallbackRoutine;
				Array[k].Context = NULL;
				k++;

				pEntry = pEntry->Next;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}

	// ------------------------------
	// 2. 遍历 Ex 版
	// ------------------------------
	if (ExHead)
	{
		PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION_EX pEntry = *(PVOID*)ExHead;

		__try
		{
			while (pEntry && MmIsAddressValid(pEntry))
			{
				if (k >= max_num)  // 修正边界
				{
					ULONG newSize = max_num + 100;
					newArray = (PCallbackInfo)ExAllocatePool2(
						POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

					if (!newArray) break;  // 必须检查

					if (Array && k > 0) {
						RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
					}

					ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
					Array = newArray;
					max_num = newSize;  // 直接赋值，避免 += 导致不一致
					*pArray = Array;
				}

				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"LogonSessionTerminatedEx");
				Array[k].Func = pEntry->CallbackRoutine;
				Array[k].Context = pEntry->CallbackContext; // Ex版带上下文
				k++;

				pEntry = pEntry->Next;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}
	//枚举FsNotifyChangeRoutinue
	PLIST_ENTRY pIopFsNotifyChangeQueueHead = (PLIST_ENTRY)FindIopFsNotifyChangeQueueHead();
	if (!pIopFsNotifyChangeQueueHead) {
		DbgPrint("Failed to find IopFsNotifyChangeQueueHead!\n");
		goto exit11;
	}
	
	PLIST_ENTRY pIopFsNotifyChangeEntry = pIopFsNotifyChangeQueueHead;

	__try
	{
		do {
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			pIopFsNotifyChangeEntry = pIopFsNotifyChangeEntry->Flink;
			PNOTIFICATION_PACKET pFsNotificationPacket = CONTAINING_RECORD(pIopFsNotifyChangeEntry, NOTIFICATION_PACKET, ListEntry);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"FsNotifyChange");
			//DbgPrint("FsNotifyChange: 0x%llx", (ULONG64)pFsNotificationPacket->NotificationRoutine);
			Array[k].Func = pFsNotificationPacket->NotificationRoutine;
			k++;
		} while (pIopFsNotifyChangeEntry->Flink != pIopFsNotifyChangeQueueHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit11:
	// ===================== 【关键】遍历前一次性预分配内存（max_num + 128 个元素） =====================
	// 先扩容：一次性分配足够内存，遍历中不再操作堆
	newArray = (PCallbackInfo)ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		sizeof(CallbackInfo) * (max_num + 128),  // 按要求多分配128个
		'cbin'
	);
	if (!newArray) {
		DbgPrint("[-] 预分配内存失败");
		goto exit12;
	}
	// 拷贝原有数据
	if (Array != NULL && k > 0) {
		RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);
		ExFreePoolWithTag(Array, 'cbin');
	}
	Array = newArray;
	*pArray = Array;  // ✅ 更新调用者指针
	max_num += 128;  // 更新最大容量
	// ===================== 预分配完成，开始遍历 =====================

	BOOLEAN isWin11 = IsWindows11OrGreater();
	
	// 1. 枚举设备类通知
	DEVICE_CLASS_NOTIFY_INFO devInfo = { 0 };
	if (NT_SUCCESS(FindDeviceClassNotifyInfo(&devInfo))) {
		DbgPrint("[+] Found DeviceClass notify info, Buckets:0x%p, Lock:0x%p", devInfo.Buckets, devInfo.Lock);
		//goto exit115;
		if (devInfo.Lock == NULL || devInfo.Buckets == NULL) {
			DbgPrint("[-] DeviceClass 指针无效");
			goto exit12;
		}

		// 【核心修正】分版本调用正确的锁获取函数
		if (isWin11) {
			ExAcquireFastMutex(devInfo.Lock);
		}
		else {
			KeAcquireGuardedMutex((PKGUARDED_MUTEX)devInfo.Lock);
		}
		__try {
			for (ULONG i = 0; i < devInfo.NumBuckets; i++) {
				PLIST_ENTRY pBucket = &devInfo.Buckets[i];
				if (pBucket->Flink == pBucket) continue;

				for (PLIST_ENTRY pEntry = pBucket->Flink; pEntry != pBucket; pEntry = pEntry->Flink) {
					if (pEntry == NULL) break;

					PPNP_NOTIFY_ENTRY pNotify = CONTAINING_RECORD(pEntry, PNP_NOTIFY_ENTRY, ListEntry);
					// 仅做校验+赋值，无任何堆操作
					if (pNotify != NULL && pNotify->Callback != NULL && k < max_num) {
						RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"PnpDeviceClass");
						Array[k].Func = pNotify->Callback;
						Array[k].Context = pNotify->Context;
						DbgPrint("[+] DeviceClass PNP回调: %p | 上下文: %p\n", pNotify->Callback, pNotify->Context);
						k++;
					}
				}
			}
		}
		__finally {
			ExReleaseFastMutex(devInfo.Lock); // KeReleaseGuardedMutex
		}
	}
	else {
		DbgPrint("[-] Failed to find DeviceClass notify info\n");
	}
exit115:
	// 2. 枚举硬件配置文件通知（安全版，无锁异常）
	HWPROFILE_NOTIFY_INFO hwInfo = { 0 };
	if (NT_SUCCESS(FindHwProfileNotifyInfo(&hwInfo))) {
		DbgPrint("[+] Found HwProfile notify info, ListHead:0x%p, Lock:0x%p", hwInfo.ListHead, hwInfo.Lock);
		//goto exit12;
		// 空链表直接跳过，不遍历
		if (hwInfo.ListHead == NULL || hwInfo.ListHead->Flink == hwInfo.ListHead) {
			DbgPrint("[*] HwProfile 链表为空");
		}
		else {
			// 【核心修正】分版本调用正确的锁获取函数
			if (isWin11) {
				ExAcquireFastMutex(hwInfo.Lock);
			}
			else {
				KeAcquireGuardedMutex((PKGUARDED_MUTEX)hwInfo.Lock);
			}
			__try {
				for (PLIST_ENTRY pEntry = hwInfo.ListHead->Flink; pEntry != hwInfo.ListHead; pEntry = pEntry->Flink) {
					if (pEntry == NULL) break;

					PPNP_NOTIFY_ENTRY pNotify = CONTAINING_RECORD(pEntry, PNP_NOTIFY_ENTRY, ListEntry);
					if (pNotify != NULL && pNotify->Callback != NULL && k < max_num) {
						RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"PnpHwProfile");
						Array[k].Func = pNotify->Callback;
						Array[k].Context = pNotify->Context;
						DbgPrint("[+] HwProfile PNP回调: %p | 上下文: %p\n", pNotify->Callback, pNotify->Context);
						k++;
					}
				}
			}
			__finally {
				ExReleaseFastMutex(hwInfo.Lock); // KeReleaseGuardedMutex
			}
		}
	}
	else {
		DbgPrint("[-] Failed to find HwProfile notify info\n");
	}
exit12:
	//枚举BugCheckCallbacks
	//KeRegisterBugCheckCallback->KeBugCheckCallbackListHead
	PVOID KeBugCheckCallbackListHead = FindKeBugCheckCallbackListHead();
	if (!KeBugCheckCallbackListHead) {
		DbgPrint("Failed to find KeBugCheckCallbackListHead!\n");
		goto exit5;
	}

	PLIST_ENTRY pListEntry = (PLIST_ENTRY)KeBugCheckCallbackListHead;
	PLIST_ENTRY pHead = pListEntry;

	if (IsListEmpty(pListEntry)) {
		DbgPrint("KeBugCheckCallbackListHead is empty!\n");
		goto exit5;
	}

	PKBUGCHECK_CALLBACK_RECORD pCallbackRecord = NULL;

	__try
	{
		do {
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			pListEntry = pListEntry->Flink;
			pCallbackRecord = CONTAINING_RECORD(pListEntry, KBUGCHECK_CALLBACK_RECORD, Entry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Context: 0x%p.", pCallbackRecord->CallbackRoutine, pCallbackRecord->Component, pCallbackRecord->Buffer);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"BugCheck");
			Array[k].Func = (PVOID)pCallbackRecord->CallbackRoutine;
			Array[k].Context = pCallbackRecord->Component;
			Array[k].Others[0] = (ULONG64)pCallbackRecord;
			k++;
		} while (pListEntry->Flink != pHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit5:
	//枚举BugCheckReasonCallbacks
	//KeRegisterBugCheckReasonCallback->KeBugCheckReasonCallbackListHead
	PVOID KeBugCheckReasonCallbackListHead = FindKeBugCheckReasonCallbackListHead();
	if (!KeBugCheckReasonCallbackListHead) {
		DbgPrint("Failed to find KeBugCheckReasonCallbackListHead!\n");
		goto exit6;
	}

	pListEntry = (PLIST_ENTRY)KeBugCheckReasonCallbackListHead;
	pHead = pListEntry;

	if (IsListEmpty(pListEntry)) {
		DbgPrint("KeBugCheckReasonCallbackListHead is empty!\n");
		goto exit6;
	}
	// 注意:枚举此回调需要加锁,此处暂不加锁,可能存在遍历异常的风险
	PKBUGCHECK_REASON_CALLBACK_RECORD pReasonCallbackRecord = NULL;

	__try
	{
		do {
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}
			pListEntry = pListEntry->Flink;
			pReasonCallbackRecord = CONTAINING_RECORD(pListEntry, KBUGCHECK_REASON_CALLBACK_RECORD, Entry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"BugCheckReason");
			Array[k].Func = (PVOID)pReasonCallbackRecord->CallbackRoutine;
			Array[k].Context = (PVOID)pReasonCallbackRecord->Component;
			Array[k].Others[0] = (ULONG64)pReasonCallbackRecord;
			k++;
		} while (pListEntry->Flink != pHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit6:
	// ==============================
	// 你的原版逻辑 100% 保留
	// 仅修复：自旋锁安全、移除锁内内存分配
	// ==============================
	PLIST_ENTRY listEntry;
	PCALLBACK_OBJECT callbackObject;
	PLIST_ENTRY ExpCallbackListHead = NULL;
	PEX_PUSH_LOCK ExpCallbackListLock = NULL;

	if ((!FindExpCallbackList(&ExpCallbackListHead, &ExpCallbackListLock))||!MmIsAddressValid(ExpCallbackListHead) || !MmIsAddressValid(ExpCallbackListLock)) {
		DbgPrint("Failed to find ExpCallbackListHead or ExpCallbackListLock!\n");
		goto exit13;
	}

	// 临时缓存：锁内仅存指针（唯一新增的安全修复）
	/*PVOID tempFunc3[MAX_TEMP] = {0};
	PVOID tempCtx3[MAX_TEMP] = { 0 };
	PCALLBACK_OBJECT tempCbObj[MAX_TEMP] = { 0 };*/
	tempCount3 = 0;

	DbgPrint("=== Starting Ex Callback Enumeration ===\n");
	if (!ExpCallbackListHead || !ExpCallbackListLock) goto exit13;

	InitializeCallbackTable();

	// 进入 guarded region（完全保留）
	KeEnterGuardedRegion();
	ExAcquirePushLockShared(ExpCallbackListLock);

	// 检查链表是否为空（完全保留）
	if (IsListEmpty(ExpCallbackListHead)) {
		DbgPrint("No Ex callbacks found\n");
		goto cleanup;
	}

	DbgPrint("ExpCallbackListHead:0x%p", ExpCallbackListHead);

	_try{
		// ======================
		// 你的原版枚举逻辑 完全不动
		// do-while / ListEntry2 / 遍历顺序 100% 保留
		// ======================
		listEntry = ExpCallbackListHead->Flink;
		PLIST_ENTRY currentlistEntry = ExpCallbackListHead->Flink;
		//OB_CALLBACK_REGISTRATION* registration = NULL;
		do {
			//DbgPrint("listEntry:0x%p", listEntry);

			// 你的原版结构：ListEntry2 完全保留
			callbackObject = CONTAINING_RECORD(listEntry, CALLBACK_OBJECT, ListEntry2);
			if (!callbackObject || !MmIsAddressValid(callbackObject)) {
				listEntry = listEntry->Flink;
				continue;
			}

			KIRQL oldIrql = PASSIVE_LEVEL;
			_try {
				// 原版加锁方式 保留
				oldIrql = KeAcquireSpinLockRaiseToDpc(&callbackObject->SpinLock);

				PLIST_ENTRY listEntry1 = &callbackObject->ListEntry;
				if (!IsListEmpty(listEntry1)) {
					for (PLIST_ENTRY iter = listEntry1->Flink; iter != listEntry1; iter = iter->Flink) {

						PCALLBACK_REGISTRATION registration = CONTAINING_RECORD(iter, CALLBACK_REGISTRATION, ListEntry);
						if (!registration || !MmIsAddressValid(registration)) {
							DbgPrint("[Error] Invalid registration object. Breaking loop.\n");
							break;
						}

						// ======================
						// 核心修复：锁内只存指针，不分配内存/不拼字符串
						// 你的遍历逻辑完全不动！
						// ======================
						if (tempCount3 < MAX_TEMP && registration->CallbackFunction && MmIsAddressValid((PVOID)registration->CallbackFunction)) {
							tempFunc3[tempCount3] = (PVOID)registration->CallbackFunction;
							tempCtx3[tempCount3] = registration->CallbackContext;
							tempCbObj[tempCount3] = callbackObject;
							tempRegObj[tempCount3] = registration;
							tempCount3++;
						}
					}
				}

				// 正常释放锁
				KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
			}
				// 你的要求：禁用 finally，只保留 except
				_except(EXCEPTION_EXECUTE_HANDLER) {
				// 异常时手动安全释放锁（无 finally）
				if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
					KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
				}
				DbgPrint("Exception in processing, skipping entry 0x%p\n", listEntry);
			}

			listEntry = listEntry->Flink;
		} while (listEntry != currentlistEntry);
	}
	__finally {
		// 无论是否异常，100%执行释放
		ExReleasePushLockShared(ExpCallbackListLock);
		KeLeaveGuardedRegion();
	}

	// ======================
	// 解锁后 → 统一处理数据（内存分配/命名/拷贝）
	// 完全不影响你的枚举逻辑
	// ======================
	for (int i = 0; i < tempCount3; i++) {
		if (k >= max_num)  // 修正边界
		{
			ULONG newSize = max_num + 100;
			newArray = (PCallbackInfo)ExAllocatePool2(
				POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

			if (!newArray) break;  // 必须检查

			if (Array && k > 0) {
				RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
			}

			ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
			Array = newArray;
			max_num = newSize;  // 直接赋值，避免 += 导致不一致
			*pArray = Array;
		}

		// 命名逻辑 完全保留
		if (!NT_SUCCESS(QueryCallbackNameByPointer_Fast(tempCbObj[i], Array[k].Type, sizeof(Array[k].Type))))
			GenerateAnonymousCallbackName(tempCbObj[i], Array[k].Type, sizeof(Array[k].Type));

		Array[k].Func = tempFunc3[i];
		Array[k].Context = tempCtx3[i];
		Array[k].Others[0] = (ULONG64)tempCbObj[i]; // 额外存储回调对象指针，方便调试
		k++;

		// 原版日志 保留
		DbgPrint("  Type:%ws ObjectType:0x%p Function:0x%p Context:0x%p\n",
			Array[k - 1].Type,
			tempCbObj[i],
			tempFunc3[i],
			tempCtx3[i]);
	}
cleanup:

	CleanupCallbackTable();

exit13:
	// ==============================
	// 枚举 NMI 回调（最终修复版）
	// ==============================
	PVOID KiNmiCallbackListHead = NULL;
	PKSPIN_LOCK KiNmiCallbackListLock = NULL;
	NTSTATUS status = FindKiNmiCallbackList(&KiNmiCallbackListHead, &KiNmiCallbackListLock);

	if (NT_SUCCESS(status) && KiNmiCallbackListHead && KiNmiCallbackListLock)
	{
		tempCnt2 = 0;
		KIRQL  oldIrql;

		// ==============================================
		// 【锁配对正确】获取锁（保留你的原代码）
		// ==============================================
		oldIrql = KeAcquireSpinLockRaiseToDpc(KiNmiCallbackListLock);

		__try
		{
			PKI_NMI_CALLBACK_ENTRY pEntry = *(PKI_NMI_CALLBACK_ENTRY*)KiNmiCallbackListHead;

			// 【修复】增加临时指针非空判断，防止空指针异常
			while (pEntry && MmIsAddressValid(pEntry) && tempFunc2 && tempCtx2 && tempCnt2 < 63)
			{
				tempFunc2[tempCnt2] = (PVOID)pEntry->Callback;
				tempCtx2[tempCnt2] = pEntry->Context;
				tempCnt2++;
				pEntry = pEntry->Next;
			}
		}
		__finally
		{
			// ==============================================
			// 【核心修复】正确释放锁 + 自动恢复 IRQL！！！
			// ==============================================
			KeReleaseSpinLock(KiNmiCallbackListLock, oldIrql);
		}

		// 下方逻辑完全不动
		for (int i = 0; i < tempCnt2; i++)
		{
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}

			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"KeRegisterNmiCallback");
			Array[k].Func = tempFunc2[i];
			Array[k].Context = tempCtx2[i];
			k++;
		}
	}
	// ==============================
	// 枚举 DbgPrint 回调（反汇编对齐·无蓝屏·必抓DebugView）
	// ==============================
	DBG_PRINT_INFO dbgInfo = { 0 };
	status = FindDbgPrintCallbackInfo(&dbgInfo);

	if (NT_SUCCESS(status) && dbgInfo.CallbackList && dbgInfo.CallbackLock && tempFunc1)
	{
		tempCnt = 0;
		KIRQL oldIrql = 0;

		// 你的IRQL逻辑（完全不动）
		KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

		__try
		{
			ExAcquireSpinLockExclusiveAtDpcLevel(dbgInfo.CallbackLock);

			// 遍历链表（你的循环逻辑完全不动）
			for (PLIST_ENTRY pEntry = dbgInfo.CallbackList->Flink;
				pEntry != dbgInfo.CallbackList && tempCnt < 15 && MmIsAddressValid(pEntry);
				pEntry = pEntry->Flink)
			{
				// 🔥 【反汇编精准计算】唯一正确的指针计算（无结构体、无越界）
				PVOID pAllocateBase = (PVOID)((PUCHAR)pEntry - 0x18);  // 分配基址 = 链表节点 - 0x18
				PDEBUG_PRINT_CALLBACK pCallback = *(PDEBUG_PRINT_CALLBACK*)((PUCHAR)pAllocateBase + 0x10); // 回调 = 基址 + 0x10

				if (pCallback && MmIsAddressValid((PVOID)pCallback))
				{
					tempFunc1[tempCnt++] = (PVOID)pCallback;
				}
			}
		}
		__finally
		{
			// 你的锁/IRQL释放（完全不动，最安全）
			ExReleaseSpinLockExclusiveFromDpcLevel(dbgInfo.CallbackLock);
			KeLowerIrql(oldIrql);
		}

		// 下方拷贝逻辑 一字不改
		for (int i = 0; i < tempCnt; i++)
		{
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) break;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}

			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"DbgPrintCallback");
			Array[k].Func = tempFunc1[i];
			Array[k].Context = NULL;
			k++;
		}
	}
	
	// ==============================
	// 枚举 KeRegisterBoundCallback（安全最终版）
	// ==============================
	PVOID KiBoundsCallback = FindKiBoundsCallback();

	if (KiBoundsCallback)
	{
		// 【先安全读取指针】（极短操作，无需锁）
		PVOID callbackFunc = NULL;
		__try
		{
			callbackFunc = *(PVOID*)KiBoundsCallback;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrint("[!] KiBoundsCallback 读取异常\n");
			callbackFunc = NULL;
		}

		// 【解锁后/安全区】再分配内存、填充数据
		if (callbackFunc != NULL)
		{
			if (k >= max_num)  // 修正边界
			{
				ULONG newSize = max_num + 100;
				newArray = (PCallbackInfo)ExAllocatePool2(
					POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * newSize, 'cbin');

				if (!newArray) goto exit_bound;  // 必须检查

				if (Array && k > 0) {
					RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);  // ✅ 拷贝旧数据
				}

				ExFreePoolWithTag(Array, 'cbin');  // 释放旧内存
				Array = newArray;
				max_num = newSize;  // 直接赋值，避免 += 导致不一致
				*pArray = Array;
			}

			// 填充数据
			RtlStringCbCopyW(
				Array[k].Type,
				sizeof(Array[k].Type),
				L"KeRegisterBoundCallback"
			);
			Array[k].Func = callbackFunc;
			Array[k].Context = NULL;
			k++;
		}
	}
exit_bound:
	return k;
}

NTSTATUS RemoveCreateProcessNotifyRoutine(PVOID CreateProcessNotifyRoutine) {
	return PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotifyRoutine, TRUE);
}

NTSTATUS RemoveCreateThreadNotifyRoutine(PVOID CreateThreadNotifyRoutine) {
	NTSTATUS status = PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)CreateThreadNotifyRoutine);
	return status;
}

NTSTATUS UnregisterCmpCallback(LARGE_INTEGER Cookie) {
	return CmUnRegisterCallback(Cookie);
}

NTSTATUS RemoveLoadImageNotifyRoutine(PVOID LoadImageNotifyRoutine) {
	return PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)LoadImageNotifyRoutine);
}

VOID UnregisterObCallback(PVOID ObHandle) {
	ObUnRegisterCallbacks(ObHandle);
}

// 运行时检测 Windows 版本并返回配置
PWFP_VERSION_CONFIG GetWfpVersionConfig()
{
	RTL_OSVERSIONINFOW osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	if (!NT_SUCCESS(RtlGetVersion(&osvi))) {
		DbgPrint("[-] RtlGetVersion failed, default to Win10\n");
		return &g_Win10Config;
	}

	// Win11 Build >= 22000
	if (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 22000) {
		DbgPrint("[+] Detected Win11 (Build %lu)\n", osvi.dwBuildNumber);
		return &g_Win11Config;
	}

	DbgPrint("[+] Detected Win10 (Build %lu)\n", osvi.dwBuildNumber);
	return &g_Win10Config;
}

//=============================================================================
// 4. 根据 calloutId 从内核映射中查找回调
//=============================================================================
KERNEL_CALLOUT_MAP* LookupKernelCallout(ULONG calloutId)
{
	for (ULONG i = 0; i < g_KernelMapCount; i++) {
		if (g_KernelMap[i].CalloutId == calloutId)
			return &g_KernelMap[i];
	}
	return NULL;
}

//=============================================================================
// 5. 枚举 FWPM_CALLOUT，建立 GUID -> calloutId 映射，并与内核映射合并
//=============================================================================
NTSTATUS BuildFullCalloutMap(HANDLE engineHandle)
{
	HANDLE enumHandle = NULL;
	FWPM_CALLOUT0** ppCallouts = NULL;
	UINT32 calloutCount = 0;
	NTSTATUS status;

	// 创建枚举句柄
	status = FwpmCalloutCreateEnumHandle0(engineHandle, NULL, &enumHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutCreateEnumHandle0 failed: %08X\n", status);
		return status;
	}

	// 枚举所有 FWPM_CALLOUT 对象
	status = FwpmCalloutEnum0(engineHandle, enumHandle, 0xFFFF, &ppCallouts, &calloutCount);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutEnum0 failed: %08X\n", status);
		FwpmCalloutDestroyEnumHandle0(engineHandle, enumHandle);
		return status;
	}

	DbgPrint("[+] FwpmCalloutEnum0: %u callouts\n", calloutCount);

	g_FullMapCount = 0;
	for (UINT32 i = 0; i < calloutCount && g_FullMapCount < MAX_CALLOUT_MAP; i++) {
		FWPM_CALLOUT0* c = ppCallouts[i];

		// 记录 GUID 和 calloutId
		g_FullMap[g_FullMapCount].CalloutGuid = c->calloutKey;
		g_FullMap[g_FullMapCount].CalloutId = c->calloutId;

		// 从内核映射中查找回调地址
		KERNEL_CALLOUT_MAP* k = LookupKernelCallout(c->calloutId);
		if (k) {
			g_FullMap[g_FullMapCount].ClassifyFn = k->ClassifyFn;
			g_FullMap[g_FullMapCount].NotifyFn = k->NotifyFn;
			g_FullMap[g_FullMapCount].FlowDeleteFn = k->FlowDeleteFn;
		}
		else {
			g_FullMap[g_FullMapCount].ClassifyFn = NULL;
			g_FullMap[g_FullMapCount].NotifyFn = NULL;
			g_FullMap[g_FullMapCount].FlowDeleteFn = NULL;
		}

		DbgPrint("[FWPM] Callout: %08lX-%04hX-%04hX... Id=%lu Classify=%p\n",
			c->calloutKey.Data1, c->calloutKey.Data2, c->calloutKey.Data3,
			c->calloutId, g_FullMap[g_FullMapCount].ClassifyFn);

		g_FullMapCount++;
	}

	// 清理
	FwpmFreeMemory0((VOID**)&ppCallouts);
	FwpmCalloutDestroyEnumHandle0(engineHandle, enumHandle);

	DbgPrint("[+] BuildFullCalloutMap: %lu entries\n", g_FullMapCount);
	return STATUS_SUCCESS;
}

//=============================================================================
// 6. 通过 GUID 查找完整映射
//=============================================================================
FULL_CALLOUT_MAP* LookupFullCallout(const GUID* pGuid)
{
	for (ULONG i = 0; i < g_FullMapCount; i++) {
		if (IsEqualGUID(&g_FullMap[i].CalloutGuid, pGuid))
			return &g_FullMap[i];
	}
	return NULL;
}

//=============================================================================
// 3. 辅助函数：从内核 CalloutTable 枚举，填充 g_KernelMap
//=============================================================================
NTSTATUS BuildKernelCalloutMap()
{
	// 1. 获取版本适配配置
	PWFP_VERSION_CONFIG pConfig = GetWfpVersionConfig();

	// 2. 获取 netio.sys 导出的 gWfpGlobal
	typedef PWFP_GLOBAL(NTAPI* FE_GET_WFP_GLOBAL_PTR)(VOID);
	FE_GET_WFP_GLOBAL_PTR pFn = (FE_GET_WFP_GLOBAL_PTR)
		KernelGetProcAddress("netio.sys", "FeGetWfpGlobalPtr");
	if (!pFn) {
		DbgPrint("[-] FeGetWfpGlobalPtr not found\n");
		return STATUS_NOT_FOUND;
	}

	PWFP_GLOBAL pWfpGlobal = pFn();
	if (!pWfpGlobal) return STATUS_INVALID_ADDRESS;

	// 3. 使用配置偏移读取全局信息
	UINT32 maxId = *(PULONG)((PUCHAR)pWfpGlobal + pConfig->MaxCalloutIdOffset);
	PUCHAR pCalloutTableBase = *(PUCHAR*)((PUCHAR)pWfpGlobal + pConfig->CalloutTableOffset);

	if (maxId == 0 || !pCalloutTableBase || maxId > 0x20000)
		return STATUS_INVALID_PARAMETER;

	// 4. 遍历 CalloutTable（使用字节指针 + 配置步长）
	g_KernelMapCount = 0;
	for (ULONG i = 0; i < maxId && g_KernelMapCount < MAX_CALLOUT_MAP; i++) {
		PUCHAR pEntryRaw = pCalloutTableBase + i * pConfig->CalloutEntrySize;

		// 检查 IsUsed (偏移 0x04，Win10/11 一致)
		if (*(PUINT32)(pEntryRaw + 0x04) != 1) continue;

		// 读取核心回调（偏移 Win10/11 完全一致）
		PVOID classifyFn = *(PVOID*)(pEntryRaw + 0x10);
		PVOID notifyFn = *(PVOID*)(pEntryRaw + 0x18);
		PVOID flowDeleteFn = *(PVOID*)(pEntryRaw + 0x20);

		// 填充映射表
		g_KernelMap[g_KernelMapCount].CalloutId = i;
		g_KernelMap[g_KernelMapCount].ClassifyFn = classifyFn;
		g_KernelMap[g_KernelMapCount].NotifyFn = notifyFn;
		g_KernelMap[g_KernelMapCount].FlowDeleteFn = flowDeleteFn;
		g_KernelMapCount++;

		DbgPrint("[Kernel] Id=%05lu Classify=%p Notify=%p\n", i, classifyFn, notifyFn);
	}

	DbgPrint("[+] BuildKernelCalloutMap: %lu entries\n", g_KernelMapCount);
	return STATUS_SUCCESS;
}

NTSTATUS EnumWfpCallouts(
	__out PWFP_CALLOUT_INFO* ppCalloutArray,
	__out ULONG* pCount
)
{
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
	NTSTATUS status;
	HANDLE engineHandle = NULL;
	PWFP_CALLOUT_INFO pArray = NULL;
	ULONG calloutCount = 0;
	ULONG i;

	// 1. 从内核 CalloutTable 获取回调地址
	status = BuildKernelCalloutMap();
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] BuildKernelCalloutMap failed: %08X\n", status);
		return status;
	}

	// 2. 打开 WFP 引擎，使用动态会话标志
	FWPM_SESSION0 session = { 0 };
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;  // 允许引擎动态分配资源
	status = FwpmEngineOpen0(NULL, RPC_C_AUTHN_DEFAULT, NULL, &session, &engineHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmEngineOpen0 failed: %08X\n", status);
		return status;
	}

	// 3. 枚举 FWPM_CALLOUT
	HANDLE enumHandle = NULL;
	FWPM_CALLOUT0** ppCallouts = NULL;
	UINT32 numCallouts = 0;

	status = FwpmCalloutCreateEnumHandle0(engineHandle, NULL, &enumHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutCreateEnumHandle0 failed: %08X\n", status);
		goto Cleanup;
	}

	status = FwpmCalloutEnum0(engineHandle, enumHandle, 0xFFFF, &ppCallouts, &numCallouts);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutEnum0 failed: %08X\n", status);
		goto Cleanup;
	}

	// 4. 统计需要保留的 Callout（有内核回调）
	calloutCount = 0;
	for (i = 0; i < numCallouts; i++) {
		KERNEL_CALLOUT_MAP* k = LookupKernelCallout(ppCallouts[i]->calloutId);
		if (k) calloutCount++;
	}

	DbgPrint("calloutCount:%d", calloutCount);
	if (calloutCount > 0) {
		pArray = (PWFP_CALLOUT_INFO)ExAllocatePool2(
			POOL_FLAG_NON_PAGED,
			calloutCount * sizeof(WFP_CALLOUT_INFO),
			'WfpE');
		if (!pArray) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			goto Cleanup;
		}
		RtlZeroMemory(pArray, calloutCount * sizeof(WFP_CALLOUT_INFO));
	}

	// 5. 填充数组
	ULONG idx = 0;
	for (i = 0; i < numCallouts; i++) {
		FWPM_CALLOUT0* c = ppCallouts[i];
		KERNEL_CALLOUT_MAP* k = LookupKernelCallout(c->calloutId);
		if (!k) continue;

		WFP_CALLOUT_INFO* entry = &pArray[idx++];
		entry->CalloutGuid = c->calloutKey;
		entry->CalloutId = c->calloutId;
		entry->ClassifyFn = k->ClassifyFn;
		entry->NotifyFn = k->NotifyFn;
		entry->FlowDeleteFn = k->FlowDeleteFn;

		if (c->displayData.name) {
			wcsncpy_s(entry->Name, ARRAY_SIZE(entry->Name),
				c->displayData.name, _TRUNCATE);
		}
		else {
			entry->Name[0] = L'\0';
		}
	}

	*ppCalloutArray = pArray;
	*pCount = calloutCount;
	status = STATUS_SUCCESS;

Cleanup:
	if (ppCallouts) FwpmFreeMemory0((VOID**)&ppCallouts);
	if (enumHandle) FwpmCalloutDestroyEnumHandle0(engineHandle, enumHandle);
	if (engineHandle) FwpmEngineClose0(engineHandle);
	return status;
}

NTSTATUS EnumWfpFilters(
	__out PWFP_FILTER_INFO* ppFilterArray,
	__out ULONG* pCount
)
{
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define MAX_TEMP_MAP 2048

	NTSTATUS status;
	// 🔥 核心修复：所有指针/句柄 初始化为 NULL（解决 C4703 错误）
	HANDLE engineHandle = NULL;
	HANDLE enumHandle = NULL;
	FWPM_CALLOUT0** ppCallouts = NULL;
	HANDLE filterEnumHandle = NULL;
	FWPM_FILTER0** ppFilters = NULL;
	PWFP_FILTER_INFO pArray = NULL;
	ULONG i;

	// 堆分配临时映射表指针（初始化为NULL）
	typedef struct _TEMP_MAP {
		GUID Guid;
		UINT32 CalloutId;
	} TEMP_MAP, * PTEMP_MAP;
	PTEMP_MAP tempMap = NULL;
	UINT32 tempMapCount = 0;
	UINT32 numCallouts = 0;
	UINT32 filterCount = 0;

	// 初始化输出参数
	*ppFilterArray = NULL;
	*pCount = 0;

	// 正确打开 WFP 引擎
	FWPM_SESSION0 session = { 0 };
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;
	status = FwpmEngineOpen0(NULL, RPC_C_AUTHN_DEFAULT, NULL, &session, &engineHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmEngineOpen0 failed: %08X\n", status);
		goto Cleanup;
	}

	// 枚举 FWPM_CALLOUT
	status = FwpmCalloutCreateEnumHandle0(engineHandle, NULL, &enumHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutCreateEnumHandle0 failed: %08X\n", status);
		goto Cleanup;
	}

	status = FwpmCalloutEnum0(engineHandle, enumHandle, 0xFFFF, &ppCallouts, &numCallouts);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmCalloutEnum0 failed: %08X\n", status);
		goto Cleanup;
	}

	// 堆分配临时映射表（修复栈溢出）
	tempMap = (PTEMP_MAP)ExAllocatePool2(
		POOL_FLAG_NON_PAGED,
		MAX_TEMP_MAP * sizeof(TEMP_MAP),
		'WfpT'
	);
	if (!tempMap) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Cleanup;
	}

	// 填充映射表
	tempMapCount = 0;
	for (UINT32 j = 0; j < numCallouts && tempMapCount < MAX_TEMP_MAP; j++) {
		tempMap[tempMapCount].Guid = ppCallouts[j]->calloutKey;
		tempMap[tempMapCount].CalloutId = ppCallouts[j]->calloutId;
		tempMapCount++;
	}

	// 枚举 Filter
	status = FwpmFilterCreateEnumHandle0(engineHandle, NULL, &filterEnumHandle);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmFilterCreateEnumHandle0 failed: %08X\n", status);
		goto Cleanup;
	}

	status = FwpmFilterEnum0(engineHandle, filterEnumHandle, (UINT32)0xFFFF, &ppFilters, &filterCount);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[-] FwpmFilterEnum0 failed: %08X\n", status);
		goto Cleanup;
	}

	// 统计有效 Filter
	ULONG validCount = 0;
	for (i = 0; i < filterCount; i++) {
		FWPM_FILTER0* f = ppFilters[i];
		if (f->action.type == FWP_ACTION_CALLOUT_TERMINATING ||
			f->action.type == FWP_ACTION_CALLOUT_UNKNOWN ||
			f->action.type == FWP_ACTION_CALLOUT_INSPECTION) {
			validCount++;
		}
	}

	// 分配结果内存
	if (validCount > 0) {
		pArray = (PWFP_FILTER_INFO)ExAllocatePool2(
			POOL_FLAG_NON_PAGED,
			validCount * sizeof(WFP_FILTER_INFO),
			'WfpE');
		if (!pArray) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			goto Cleanup;
		}
		RtlZeroMemory(pArray, validCount * sizeof(WFP_FILTER_INFO));
	}

	// 填充 Filter 数据
	ULONG idx = 0;
	for (i = 0; i < filterCount && validCount > 0; i++) {
		FWPM_FILTER0* f = ppFilters[i];
		if (f->action.type != FWP_ACTION_CALLOUT_TERMINATING &&
			f->action.type != FWP_ACTION_CALLOUT_UNKNOWN &&
			f->action.type != FWP_ACTION_CALLOUT_INSPECTION) {
			continue;
		}

		WFP_FILTER_INFO* entry = &pArray[idx++];
		entry->FilterKey = f->filterKey;
		entry->FilterId = f->filterId;
		entry->ActionType = f->action.type;
		entry->CalloutGuid = f->action.calloutKey;
		entry->CalloutId = 0;

		// 匹配 CalloutId
		for (UINT32 j = 0; j < tempMapCount; j++) {
			if (IsEqualGUID(&tempMap[j].Guid, &entry->CalloutGuid)) {
				entry->CalloutId = tempMap[j].CalloutId;
				break;
			}
		}

		// 复制名称
		if (f->displayData.name) {
			wcsncpy_s(entry->Name, ARRAY_SIZE(entry->Name), f->displayData.name, _TRUNCATE);
		}
		else {
			entry->Name[0] = L'\0';
		}
	}

	// 输出结果
	*ppFilterArray = pArray;
	*pCount = validCount;
	status = STATUS_SUCCESS;

Cleanup:
	// 安全释放所有资源（全部初始化为NULL，无野指针）
	if (tempMap) ExFreePoolWithTag(tempMap, 'WfpT');
	if (ppFilters) FwpmFreeMemory0((VOID**)&ppFilters);
	if (filterEnumHandle) FwpmFilterDestroyEnumHandle0(engineHandle, filterEnumHandle);
	if (ppCallouts) FwpmFreeMemory0((VOID**)&ppCallouts);
	if (enumHandle) FwpmCalloutDestroyEnumHandle0(engineHandle, enumHandle);
	if (engineHandle) FwpmEngineClose0(engineHandle);

	// 失败时清理内存
	if (!NT_SUCCESS(status) && pArray) {
		ExFreePoolWithTag(pArray, 'WfpE');
		*ppFilterArray = NULL;
		*pCount = 0;
	}

	return status;
}

// ==============================
// 统一删除回调函数（增强版）
// 功能：根据 CALLBACK_INFO 自动识别类型并删除对应内核回调
// 支持：CreateThread/CreateThreadEx/CreateProcess/LoadImage
//       通用Ex回调 (\Callback\xxx) / 注册表回调 / 对象回调 / BugCheck回调
// ==============================
#define _countof(_Array) (sizeof(_Array) / sizeof((_Array)[0]))
NTSTATUS DeleteCallback(PCallbackInfo pCallbackInfo)
{
	// 1. 入参合法性校验
	if (!pCallbackInfo) {
		DbgPrint("[DeleteCallback] 错误：入参指针为空\n");
		return STATUS_INVALID_PARAMETER;
	}
	if (!pCallbackInfo->Func) {
		DbgPrint("[DeleteCallback] 错误：回调函数地址为空\n");
		return STATUS_INVALID_PARAMETER;
	}

	// 2. IRQL 校验（删除回调必须在 PASSIVE_LEVEL）
	if (KeGetCurrentIrql() != PASSIVE_LEVEL) {
		DbgPrint("[DeleteCallback] 错误：当前IRQL不允许删除回调\n");
		return STATUS_INVALID_LEVEL;
	}

	NTSTATUS status = STATUS_NOT_SUPPORTED;
	DbgPrint("[DeleteCallback] 尝试删除回调：%ws | 地址：0x%p\n",
		pCallbackInfo->Type, pCallbackInfo->Func);

	// 3. 严格按照指定顺序判断回调类型
	// =========================================================================
	// 1. 进程回调 (ObProcessCallback)
	// =========================================================================
	if (_wcsnicmp(pCallbackInfo->Type, L"ObProcessCallback", _countof(L"ObProcessCallback") - 1) == 0)
	{
		if (pCallbackInfo->Others[0])
		{
			ObUnRegisterCallbacks((PVOID)pCallbackInfo->Others[0]);
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_INVALID_HANDLE;
		}
	}
	// =========================================================================
	// 3. 线程回调 (ObThreadCallback)
	// =========================================================================
	else if (_wcsnicmp(pCallbackInfo->Type, L"ObThreadCallback", _countof(L"ObThreadCallback") - 1) == 0)
	{
		if (pCallbackInfo->Others[0])
		{
			ObUnRegisterCallbacks((PVOID)pCallbackInfo->Others[0]);
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_INVALID_HANDLE;
		}
	}
	// =========================================================================
	// 5. 创建进程 (CreateProcess)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"CreateProcess") == 0)
	{
		status = PsSetCreateProcessNotifyRoutine(
			(PCREATE_PROCESS_NOTIFY_ROUTINE)pCallbackInfo->Func, TRUE);
	}
	// =========================================================================
	// 6. 创建进程Ex (CreateProcessEx)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"CreateProcessEx") == 0)
	{
		status = PsSetCreateProcessNotifyRoutineEx(
			(PCREATE_PROCESS_NOTIFY_ROUTINE_EX)pCallbackInfo->Func, TRUE);
	}
	// =========================================================================
	// 7. 创建进程Ex2 (CreateProcessEx2)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"CreateProcessEx2") == 0)
	{
		status = PsSetCreateProcessNotifyRoutineEx2(
			PsCreateProcessNotifySubsystems, (PVOID)&pCallbackInfo->Func, TRUE);
	}
	// =========================================================================
	// 8. 创建线程 (CreateThread)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"CreateThread") == 0)
	{
		status = PsRemoveCreateThreadNotifyRoutine(
			(PCREATE_THREAD_NOTIFY_ROUTINE)pCallbackInfo->Func);
	}
	// =========================================================================
	// 9. 加载镜像 (LoadImage)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"LoadImage") == 0)
	{
		status = PsRemoveLoadImageNotifyRoutine(
			(PLOAD_IMAGE_NOTIFY_ROUTINE)pCallbackInfo->Func);
	}
	// =========================================================================
	// 10. 未知加载镜像 (LoadImageUnknown) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"LoadImageUnknown") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：LoadImageUnknown\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 11. 注册表回调 (CmpCallback) → 修复Bug：使用Others[0]存储Cookie
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"CmpCallback") == 0)
	{
		if (pCallbackInfo->Others[0] != 0)
		{
			LARGE_INTEGER cookie;
			cookie.QuadPart = (LONGLONG)pCallbackInfo->Others[0];
			status = CmUnRegisterCallback(cookie);
		}
		else
		{
			status = STATUS_INVALID_HANDLE;
		}
	}
	// =========================================================================
	// 12. Lego回调 (Lego) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"Lego") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：Lego回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 13. 普通关机通知 (Shutdown(IRP_MJ_SHUTDOWN)) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"Shutdown(IRP_MJ_SHUTDOWN)") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：关机通知回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 14. 最终机会关机通知 (LastChanceShutdown(IRP_MJ_SHUTDOWN)) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"LastChanceShutdown(IRP_MJ_SHUTDOWN)") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：最终机会关机通知回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 15. 登录会话终止 (LogonSessionTerminated(Normal)) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"LogonSessionTerminated(Normal)") == 0)
	{
		status = SeUnregisterLogonSessionTerminatedRoutine((PSE_LOGON_SESSION_TERMINATED_ROUTINE)pCallbackInfo->Func);
	}
	// =========================================================================
	// 16. 登录会话终止Ex (LogonSessionTerminatedEx) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"LogonSessionTerminatedEx") == 0)
	{
		status = SeUnregisterLogonSessionTerminatedRoutineEx((PSE_LOGON_SESSION_TERMINATED_ROUTINE_EX)pCallbackInfo->Func, pCallbackInfo->Context);
	}
	// =========================================================================
	// 17. 文件系统变更通知 (FsNotifyChange) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"FsNotifyChange") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：文件系统变更通知\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 18. PNP设备类通知 (PnpDeviceClass) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"PnpDeviceClass") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：PNP设备类通知\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 19. PNP硬件配置文件通知 (PnpHwProfile) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"PnpHwProfile") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：PNP硬件配置文件通知\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 20. 系统崩溃回调 (BugCheck)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"BugCheck") == 0)
	{
		BOOLEAN ret = KeDeregisterBugCheckCallback((PKBUGCHECK_CALLBACK_RECORD)pCallbackInfo->Others[0]);
		status = ret ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}
	// =========================================================================
	// 21. 系统崩溃原因回调 (BugCheckReason)
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"BugCheckReason") == 0)
	{
		BOOLEAN ret = KeDeregisterBugCheckReasonCallback((PKBUGCHECK_REASON_CALLBACK_RECORD)pCallbackInfo->Others[0]);
		status = ret ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}
	// =========================================================================
	// 22. NMI不可屏蔽中断回调 (KeRegisterNmiCallback) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"KeRegisterNmiCallback") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：NMI中断回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 23. DbgPrint输出回调 (DbgPrintCallback) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"DbgPrintCallback") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：DbgPrint回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 24. 边界异常回调 (KeRegisterBoundCallback) → 不支持删除
	// =========================================================================
	else if (_wcsicmp(pCallbackInfo->Type, L"KeRegisterBoundCallback") == 0)
	{
		DbgPrint("[DeleteCallback] 不支持删除：边界检查回调\n");
		status = STATUS_NOT_SUPPORTED;
	}
	// =========================================================================
	// 通用Ex回调 (\Callback\xxx)
	// =========================================================================
	else if (_wcsnicmp(pCallbackInfo->Type, L"\\Callback\\", 10) == 0)
	{
		if (pCallbackInfo->Others[0])
		{
			ExUnregisterCallback((PCALLBACK_OBJECT)pCallbackInfo->Others[0]);
			status = STATUS_SUCCESS;
		}
		else
		{
			status = STATUS_INVALID_HANDLE;
		}
	}
	// =========================================================================
	// 未知类型
	// =========================================================================
	else
	{
		DbgPrint("[DeleteCallback] 不支持的回调类型：%ws\n", pCallbackInfo->Type);
	}

	// 4. 打印执行结果
	if (NT_SUCCESS(status)) {
		DbgPrint("[DeleteCallback] 删除成功！%ws | 0x%p\n",
			pCallbackInfo->Type, pCallbackInfo->Func);
	}
	else {
		DbgPrint("[DeleteCallback] 删除失败！状态码：0x%X\n", status);
	}

	return status;
}