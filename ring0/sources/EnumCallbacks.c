#pragma once
#include "EnumCallbacks.h"

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

PVOID FindPspCreateProcessNotifyRoutine()
{
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

PVOID FindCmCallbackListHead()
{
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

ULONG EnumMiniFilter(PMINIFILTER_OBJECT Array)
{
	ULONG maxNum = 64;
	ULONG ulFilterListSize = 0;
	PFLT_FILTER* ppFilterList = NULL;
	ULONG i = 0;
	NTSTATUS status = STATUS_SUCCESS;
	LONG lOperationsOffset = 0;
	PFLT_OPERATION_REGISTRATION pFltOperationRegistration = NULL;

	// 获取 Minifilter 过滤器Filter 的数量
	FltEnumerateFilters(NULL, 0, &ulFilterListSize);

	// 申请内存
	ppFilterList = (PFLT_FILTER*)ExAllocatePool2(POOL_FLAG_NON_PAGED, ulFilterListSize * sizeof(PFLT_FILTER), 'cbin');
	if (NULL == ppFilterList)
	{
		DbgPrint("ExAllocatePool2 failed");
		return 0;
	}

	// 获取 Minifilter 中所有过滤器Filter 的信息
	status = FltEnumerateFilters(ppFilterList, ulFilterListSize, &ulFilterListSize);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("FltEnumerateFilters failed, status: 0x%X", status);
		ExFreePoolWithTag(ppFilterList, 'cbin');
		return 0;
	}

	DbgPrint("过滤器数量: %d \n", ulFilterListSize);

	// 获取 PFLT_FILTER 中 Operations 偏移
	RTL_OSVERSIONINFOEXW OSVersion = { 0 };
	OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);
	if (OSVersion.dwMajorVersion >= 10 && OSVersion.dwBuildNumber >= 26100) {
		// Windows 11及以上，使用对应偏移
		lOperationsOffset = 0x1B0;
		DbgPrint("win11");
	}
	else {
		// 旧版本偏移
		//lOperationsOffset = 0x1A8;
		DbgPrint("暂不支持的系统版本!");
		return 0;
	}

	if (!Array) {
		DbgPrint("无效Array!");
		return 0;
	}

	// 开始遍历 Minifilter
	for (i = 0; i < ulFilterListSize; i++)
	{
		// 获取 PFLT_FILTER 中 Operations 成员地址
		pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)(*(PVOID*)((PUCHAR)ppFilterList[i] + lOperationsOffset));

		__try
		{
			if (i > maxNum) { // 重新分配
				DbgPrint("过滤器数量过多，重新分配内存");
				PMINIFILTER_OBJECT pNewArray = (PMINIFILTER_OBJECT)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(MINIFILTER_OBJECT) * (maxNum + 64), 'mfin');
				if (pNewArray) {
					memcpy(pNewArray, Array, sizeof(MINIFILTER_OBJECT) * maxNum);
					ExFreePoolWithTag(Array, 'mfin');
					Array = pNewArray;
					maxNum += 64;
				}
				else {
					DbgPrint("重新分配内存失败");
					break;
				}
			}
			Array[i].hFilter = ppFilterList[i];
			/*
			lkd> dt fltmgr!_FLT_FILTER
			   +0x000 Base             : _FLT_OBJECT
			   +0x038 Frame            : Ptr64 _FLTP_FRAME
			   +0x040 Name             : _UNICODE_STRING
			   +0x050 DefaultAltitude  : _UNICODE_STRING
			   +0x060 Flags            : _FLT_FILTER_FLAGS
			   +0x068 DriverObject     : Ptr64 _DRIVER_OBJECT
			   +0x070 InstanceList     : _FLT_RESOURCE_LIST_HEAD
			   +0x0f0 VerifierExtension : Ptr64 _FLT_VERIFIER_EXTENSION
			   +0x0f8 VerifiedFiltersLink : _LIST_ENTRY
			   +0x108 FilterUnload     : Ptr64     long
			   +0x110 InstanceSetup    : Ptr64     long
			   +0x118 InstanceQueryTeardown : Ptr64     long
			   +0x120 InstanceTeardownStart : Ptr64     void
			   +0x128 InstanceTeardownComplete : Ptr64     void
			   +0x130 SupportedContextsListHead : Ptr64 _ALLOCATE_CONTEXT_HEADER
			   +0x138 SupportedContexts : [7] Ptr64 _ALLOCATE_CONTEXT_HEADER
			   +0x170 PreVolumeMount   : Ptr64     _FLT_PREOP_CALLBACK_STATUS
			   +0x178 PostVolumeMount  : Ptr64     _FLT_POSTOP_CALLBACK_STATUS
			   +0x180 GenerateFileName : Ptr64     long
			   +0x188 NormalizeNameComponent : Ptr64     long
			   +0x190 NormalizeNameComponentEx : Ptr64     long
			   +0x198 NormalizeContextCleanup : Ptr64     void
			*/
			//Array[i].Name = *(PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x40);
			//Array[i].Altitude = *(PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x50);
			PDRIVER_OBJECT pDriverObject = *(PDRIVER_OBJECT*)((PUCHAR)ppFilterList[i] + 0x68);
			RtlStringCbCopyUnicodeString(Array[i].Name, sizeof(Array[i].Name), (PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x40));
			// 获取驱动路径
			PLDR_DATA_TABLE_ENTRY pLdr = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
			RtlStringCbCopyW(Array[i].Path, sizeof(Array[i].Path), pLdr->FullDllName.Buffer);
			RtlStringCbCopyUnicodeString(Array[i].Altitude, sizeof(Array[i].Altitude), (PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x50));
			Array[i].hFilter = ppFilterList[i];
			
			DbgPrint("DriverName:%wZ, hFilter:0x%p, Altitude:%wS", (PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x40), Array[i].hFilter, Array[i].Altitude);
			//RtlStringCbCopyW(Array[i].Name, sizeof(Array[i].Name), ((PUNICODE_STRING)((PUCHAR)ppFilterList[i] + 0x40))->Buffer);
			Array[i].FilterFunc[0] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x108);
			Array[i].FilterFunc[1] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x110);
			Array[i].FilterFunc[2] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x118);
			Array[i].FilterFunc[3] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x120);
			Array[i].FilterFunc[4] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x128);
			Array[i].FilterFunc[5] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x170);
			Array[i].FilterFunc[6] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x178);
			Array[i].FilterFunc[7] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x180);
			Array[i].FilterFunc[8] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x188);
			Array[i].FilterFunc[9] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x190);
			Array[i].FilterFunc[10] = *(PVOID*)((PUCHAR)ppFilterList[i] + 0x198);
			Array[i].bMajorFunction = TRUE;
			// 同一过滤器下的回调信息
			while (IRP_MJ_OPERATION_END != pFltOperationRegistration->MajorFunction)
			{
				DbgPrint("Filter: %p | IRP: %d | PreFunc: 0x%p | PostFunc=0x%p \n", ppFilterList[i], pFltOperationRegistration->MajorFunction,
						pFltOperationRegistration->PreOperation, pFltOperationRegistration->PostOperation);
				 Array[i].MajorFunction[pFltOperationRegistration->MajorFunction].PreOperation = (PVOID)pFltOperationRegistration->PreOperation;
				 Array[i].MajorFunction[pFltOperationRegistration->MajorFunction].PostOperation = (PVOID)pFltOperationRegistration->PostOperation;

				// 获取下一个消息回调信息
				pFltOperationRegistration = (PFLT_OPERATION_REGISTRATION)((PUCHAR)pFltOperationRegistration + sizeof(FLT_OPERATION_REGISTRATION));
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			DbgPrint("Exception1: 0x%X", GetExceptionCode());
			Array[i].bMajorFunction = FALSE;
		}
		// 释放hFilter引用
		FltObjectDereference(ppFilterList[i]);
	}
	// 释放内存
	ExFreePoolWithTag(ppFilterList, 'cbin');
	ppFilterList = NULL;
	return (i + 1);
}

PVOID FindKeBugCheckCallbackListHead()
{
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

PVOID FindLogonSessionTerminatedRoutinueHead()
{
	UNICODE_STRING SeRegisterLogonSessionTerminatedRoutineName = RTL_CONSTANT_STRING(L"SeRegisterLogonSessionTerminatedRoutine");
	PVOID SeRegisterLogonSessionTerminatedRoutineAddr = MmGetSystemRoutineAddress(&SeRegisterLogonSessionTerminatedRoutineName);
	if (NULL == SeRegisterLogonSessionTerminatedRoutineAddr)
	{
		DbgPrint("SeRegisterLogonSessionTerminatedRoutineAddr is NULL");
		return NULL;
	}

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)SeRegisterLogonSessionTerminatedRoutineAddr;

	// 设置搜索长度
	ULONG size = 0x90;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x48,0x8b,0x05 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("LogonSessionTerminatedRoutinueHead is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	return (PVOID)((PUCHAR)result + 7 + offset);
}

PVOID FindPnpDeviceClassNotifyList()
{
	UNICODE_STRING IoRegisterPlugPlayNotificationName = RTL_CONSTANT_STRING(L"IoRegisterPlugPlayNotification");
	PVOID IoRegisterPlugPlayNotificationAddr = MmGetSystemRoutineAddress(&IoRegisterPlugPlayNotificationName);
	if (NULL == IoRegisterPlugPlayNotificationAddr)
	{
		DbgPrint("IoRegisterPlugPlayNotificationAddr is NULL");
		return NULL;
	}

	// 设置起始位置
	PUCHAR StartSearchAddress = (PUCHAR)IoRegisterPlugPlayNotificationAddr + 0x200;

	// 设置搜索长度
	ULONG size = 0x300;

	// 指定特征码
	UCHAR pSpecialCode[3] = { 0x48,0x8d,0x15 };

	// 指定特征码长度
	ULONG ulSpecialCodeLength = 3;

	// 开始搜索,找到后返回首地址
	PVOID result = SearchSpecialCode(StartSearchAddress, size, pSpecialCode, ulSpecialCodeLength);

	if (NULL == result)
	{
		DbgPrint("PnpDeviceClassNotifyList is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	DbgPrint("result:0x%p,offset:0x%x", result, offset);
	return (PVOID)((PUCHAR)result + 7 + offset);
}

PVOID FindIopFsNotifyChangeQueueHead()
{
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

PVOID FindExpCallbackListHead()
{
	UCHAR pSpecialCode[3] = { 0x48,0x89,0x0d };
	PVOID result = SearchSpecialCode((PUCHAR)ExCreateCallback, 0x100, pSpecialCode, 3);
	if (NULL == result)
	{
		DbgPrint("ExpCallbackListHead is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	DbgPrint("ExpCallbackListHead:0x%p", (PVOID)((PUCHAR)result + 7 + offset));
	return (PVOID)((PUCHAR)result + 7 + offset);
}

PEX_PUSH_LOCK FindExpCallbackListLock()
{
	/*
	fffff807`9b072326 488bce          mov     rcx,rsi
	fffff807`9b072329 e88607a7ff      call    nt!ExpUnlockCallbackListExclusive (fffff807`9aae2ab4)
	*/
	UCHAR pSpecialCode[4] = { 0x48,0x8b,0xce,0xe8 };
	PVOID result = SearchSpecialCode((PUCHAR)ExCreateCallback, 0x150, pSpecialCode, 4);
	if (NULL == result)
	{
		DbgPrint("ExpUnlockCallbackListExclusive is NULL");
		return NULL;
	}
	// 计算目标地址
	LONG offset = *(PLONG)((PUCHAR)result + 4);
	PVOID ExpUnlockCallbackListExclusive = (PVOID)((PUCHAR)result + 8 + offset);
	DbgPrint("ExpUnlockCallbackListExclusive:0x%p", ExpUnlockCallbackListExclusive);

	/*
	fffff807`9aae2abd 0f0d0dfc77a100  prefetchw [nt!ExpCallbackListLock (fffff807`9b4fa2c0)]
	*/
	pSpecialCode[0] = 0x0f;
	pSpecialCode[1] = 0x0d;
	pSpecialCode[2] = 0x0d;
	result = SearchSpecialCode((PUCHAR)ExpUnlockCallbackListExclusive, 0x50, pSpecialCode, 3);
	if (NULL == result)
	{
		DbgPrint("ExpCallbackListLock is NULL");
		return NULL;
	}

	// 计算目标地址
	ULONG offset2 = *(PULONG)((PUCHAR)result + 3);
	DbgPrint("ExpCallbackListLock:0x%p", (PEX_PUSH_LOCK)((PUCHAR)result + 7 + offset2));
	return (PEX_PUSH_LOCK)((PUCHAR)result + 7 + offset2);
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

						DbgPrint("Table[%d]: %p -> %ws", g_CallbackTable.Count - 1, entry->CallbackObject, entry->Name);

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
	// IoRegisterPriorityCallback,CcCoalescingCallBack,IoRegisterBootDriverCallback,
	// IoRegisterBootDriverReinitialization,IoRegisterDriverReinitialization,KdRegisterPowerHandler
	// KdRegisterPowerHandler,KeRegisterBoundCallback,KeRegisterNmiCallback,
	// KeRegisterProcessorChangeCallback,SeRegisterImageVerificationCallback,
	// DbgSetDebugPrintCallback,
	PCallbackInfo Array = *pArray;  // 解引用
	ULONG max_num = 512;
	ULONG k = 0;
	//枚举ObProcess(Thread)Callback
	POBJECT_TYPE pProcessType = *PsProcessType;
	POBJECT_TYPE pThreadType = *PsThreadType;
	//POBJECT_TYPE pDeskTopType = *ExDesktopObjectType;
	ULONG64 CallbackListOffset = 0;

	// 判断操作系统版本
	RTL_OSVERSIONINFOEXW OSVersion = { 0 };
	OSVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	RtlGetVersion((PRTL_OSVERSIONINFOW)&OSVersion);

	if (OSVersion.dwBuildNumber >= 9200)
		CallbackListOffset = 0xC8;
	else if (OSVersion.dwBuildNumber >= 7600)
		CallbackListOffset = 0xC0;

	//if (!pProcessType) goto 

	POB_CALLBACK pProcessCallbackEntry = *(POB_CALLBACK*)((UCHAR*)pProcessType + CallbackListOffset);
	POB_CALLBACK pThreadCallbackEntry = *(POB_CALLBACK*)((UCHAR*)pThreadType + CallbackListOffset);
	POB_CALLBACK pProcessHead = pProcessCallbackEntry;
	POB_CALLBACK pThreadHead = pThreadCallbackEntry;
	
	//DbgPrint("CallbackList Addr:0x%p", &CallbackList);
	//枚举Process
	__try {
		do
		{
			if (pProcessCallbackEntry && MmIsAddressValid(pProcessCallbackEntry) && pProcessCallbackEntry->ObTypeAddr != NULL)
			{
				if (k > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
					max_num += 100;
					*pArray = Array;  // ✅ 更新调用者指针
				}
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"ObProcessCallback(Pre)");
				RtlStringCbCopyW(Array[k+1].Type, sizeof(Array[k+1].Type), L"ObProcessCallback(Post)");
				Array[k].Context = pProcessCallbackEntry->ObHandle;
				Array[k].Func = pProcessCallbackEntry->PreCall;
				Array[k+1].Context = pProcessCallbackEntry->ObHandle;
				Array[k+1].Func = pProcessCallbackEntry->PostCall;
				DbgPrint("PreCall:0x%p,PostCall:0x%p", pProcessCallbackEntry->PreCall, pProcessCallbackEntry->PostCall);
				k+=2;
			}
			pProcessCallbackEntry = (POB_CALLBACK)(pProcessCallbackEntry->ListEntry.Flink);
		} while (pProcessHead != pProcessCallbackEntry);

		//枚举Thread
		do
		{
			if (pThreadCallbackEntry && MmIsAddressValid(pThreadCallbackEntry) && pThreadCallbackEntry->ObTypeAddr != NULL)
			{
				if (k > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
					max_num += 100;
					*pArray = Array;  // ✅ 更新调用者指针
				}
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"ObThreadCallback(Pre)");
				RtlStringCbCopyW(Array[k + 1].Type, sizeof(Array[k + 1].Type), L"ObThreadCallback(Post)");
				Array[k].Context = pThreadCallbackEntry->ObHandle;
				Array[k].Func = pThreadCallbackEntry->PreCall;
				Array[k + 1].Context = pThreadCallbackEntry->ObHandle;
				Array[k + 1].Func = pThreadCallbackEntry->PostCall;
				DbgPrint("PreCall:0x%p,PostCall:0x%p", pThreadCallbackEntry->PreCall, pThreadCallbackEntry->PostCall);
				k += 2;
			}
			pThreadCallbackEntry = (POB_CALLBACK)(pThreadCallbackEntry->ListEntry.Flink);
		} while (pThreadHead != pThreadCallbackEntry);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
	//枚举CreateProcessNotify
	ULONG64	NotifyAddr = 0, MagicPtr = 0;
	ULONG64	PspCreateProcessNotifyRoutine = (ULONG64)FindPspCreateProcessNotifyRoutine();
	//DbgPrint("PspCreateProcessNotifyRoutine: 0x%llx", PspCreateProcessNotifyRoutine);
	if (!PspCreateProcessNotifyRoutine) goto exit1;
	__try {
		for (int i = 0; i < 64; i++)
		{
			MagicPtr = PspCreateProcessNotifyRoutine + i * 8;
			NotifyAddr = *(PULONG64)(MagicPtr);
			if (MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
			{
				if (k > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
					max_num += 100;
					*pArray = Array;  // ✅ 更新调用者指针
				}
				NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);
				//DbgPrint("[CreateProcess]0x%llx", NotifyAddr);
				//RtlInitUnicodeString(&Array[i].Type, L"CreateProcess");
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[i].Type), L"CreateProcess");
				Array[k].Func = (PVOID)NotifyAddr;
				k++;
				//GetDriverNameByAddr((PVOID)NotifyAddr, Array[i].DriverName);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit1:
	//枚举CreateThreadNotify
	//NotifyAddr = 0;
	//MagicPtr = 0;
	ULONG64	PspCreateThreadNotifyRoutine = (ULONG64)FindPspCreateThreadNotifyRoutine();
	//DbgPrint("PspCreateThreadNotifyRoutine: 0x%llx", PspCreateThreadNotifyRoutine);
	if (!PspCreateThreadNotifyRoutine) goto exit2;
	__try {
		for (int i = 0; i < 64; i++)
		{
			MagicPtr = PspCreateThreadNotifyRoutine + i * 8;
			NotifyAddr = *(PULONG64)(MagicPtr);
			if (MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
			{
				if (k > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
					max_num += 100;
					*pArray = Array;  // ✅ 更新调用者指针
				}
				NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);
				//DbgPrint("[CreateThread]0x%llx", NotifyAddr);
				//RtlInitUnicodeString(&Array[i].Type, L"CreateThread");
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[i].Type), L"CreateThread");
				Array[k].Func = (PVOID)NotifyAddr;
				k++;
				//GetDriverNameByAddr((PVOID)NotifyAddr, Array[i].DriverName);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit2:
	//枚举LoadImageNotify
	//ULONG64	NotifyAddr = 0, MagicPtr = 0;
	ULONG64	PspLoadImageNotifyRoutine = (ULONG64)FindPspLoadImageNotifyRoutine();
	//DbgPrint("PspLoadImageNotifyRoutine: 0x%llx", PspLoadImageNotifyRoutine);
	if (!PspLoadImageNotifyRoutine) goto exit3;
	__try {
		for (int i = 0; i < 64; i++)
		{
			MagicPtr = PspLoadImageNotifyRoutine + i * 8;
			NotifyAddr = *(PULONG64)(MagicPtr);
			if (MmIsAddressValid((PVOID)NotifyAddr) && NotifyAddr != 0)
			{
				if (k > max_num)
				{
					ExFreePoolWithTag(Array, 'cbin');
					Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
					max_num += 100;
					*pArray = Array;  // ✅ 更新调用者指针
				}
				NotifyAddr = *(PULONG64)(NotifyAddr & 0xfffffffffffffff8);
				//DbgPrint("[LoadImage]0x%llx", NotifyAddr);
				//RtlInitUnicodeString(&Array[i].Type, L"CreateThread");
				RtlStringCbCopyW(Array[k].Type, sizeof(Array[i].Type), L"LoadImage");
				Array[k].Func = (PVOID)NotifyAddr;
				k++;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit3:
	//枚举CmpCallback
	// 遍历链表结构
	PCM_NOTIFY_ENTRY pNotifyEntry = NULL;

	PVOID pCallbackListHead = FindCmCallbackListHead();
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
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"CmpCallback");
			Array[k].Func = (PVOID)pNotifyEntry->Function;
			Array[k].Context = (PVOID)pNotifyEntry->Cookie.QuadPart;
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
	if (k > max_num)
	{
		ExFreePoolWithTag(Array, 'cbin');
		Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
		max_num += 100;
		*pArray = Array;  // ✅ 更新调用者指针
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
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			pShutdownEntry = pShutdownEntry->Flink;
			pShutdownPacket = CONTAINING_RECORD(pShutdownEntry, SHUTDOWN_PACKET, ListEntry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"Shutdown");
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
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			pLastChanceEntry = pLastChanceEntry->Flink;
			pLastChanceShutdownPacket = CONTAINING_RECORD(pLastChanceEntry, SHUTDOWN_PACKET, ListEntry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"LastChanceShutdown");
			Array[k].Func = (PVOID)pLastChanceShutdownPacket->DeviceObject->DriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
			k++;
		} while (pLastChanceEntry->Flink != pLastChanceHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit9:
	//枚举LogonSessionTerminatedRoutinue
	PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION pLogonSessionTerminatedRoutinueHead = (PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION)FindLogonSessionTerminatedRoutinueHead();
	if (!pLogonSessionTerminatedRoutinueHead) {
		DbgPrint("Failed to find LogonSessionTerminatedRoutinueHead!\n");
		goto exit10;
	}
	DbgPrint("pLogonSessionTerminatedRoutinueHead:0x%p", pLogonSessionTerminatedRoutinueHead);
	PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION pLogonSessionTerminatedRoutinueEntry = pLogonSessionTerminatedRoutinueHead;
	__try
	{
		do {
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"LogonSessionTerminated");
			Array[k].Func = (PVOID)pLogonSessionTerminatedRoutinueEntry->CallbackRoutine;
			pLogonSessionTerminatedRoutinueEntry = pLogonSessionTerminatedRoutinueEntry->Next;
			k++;
		} while (pLogonSessionTerminatedRoutinueEntry != pLogonSessionTerminatedRoutinueHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit10:
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
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
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
	//枚举IopPlugPlayCallback
	PLIST_ENTRY pIopPlugPlayCallbackListHead = (PLIST_ENTRY)FindPnpDeviceClassNotifyList();
	if (!pIopPlugPlayCallbackListHead) {
		DbgPrint("Failed to find FindPnpDeviceClassNotifyList!\n");
		goto exit12;
	}
	PLIST_ENTRY pIopPlugPlayCallbackEntry = pIopPlugPlayCallbackListHead;
	__try
	{
		do {
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			pIopPlugPlayCallbackEntry = pIopPlugPlayCallbackEntry->Flink;
			PSETUP_NOTIFY_DATA pPlugPlayNotificationData = CONTAINING_RECORD(pIopPlugPlayCallbackEntry, SETUP_NOTIFY_DATA, ListEntry);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"PlugPlay");
			Array[k].Func = (PVOID)pPlugPlayNotificationData->Callback;
			Array[k].Context = (PVOID)pPlugPlayNotificationData->Context;
			k++;
		} while (pIopPlugPlayCallbackEntry->Flink != pIopPlugPlayCallbackListHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
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
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			pListEntry = pListEntry->Flink;
			pCallbackRecord = CONTAINING_RECORD(pListEntry, KBUGCHECK_CALLBACK_RECORD, Entry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Context: 0x%p.", pCallbackRecord->CallbackRoutine, pCallbackRecord->Component, pCallbackRecord->Buffer);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"BugCheck");
			Array[k].Func = (PVOID)pCallbackRecord->CallbackRoutine;
			Array[k].Context = pCallbackRecord->Component;
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

	PKBUGCHECK_REASON_CALLBACK_RECORD pReasonCallbackRecord = NULL;

	__try
	{
		do {
			if (k > max_num)
			{
				ExFreePoolWithTag(Array, 'cbin');
				Array = (PCallbackInfo)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(CallbackInfo) * (max_num + 100), 'cbin');
				max_num += 100;
				*pArray = Array;  // ✅ 更新调用者指针
			}
			pListEntry = pListEntry->Flink;
			pReasonCallbackRecord = CONTAINING_RECORD(pListEntry, KBUGCHECK_REASON_CALLBACK_RECORD, Entry);
			//DbgPrint("CallbackRoutine: 0x%p | Component: %s | Reason: %d.", pReasonCallbackRecord->CallbackRoutine, pReasonCallbackRecord->Component, pReasonCallbackRecord->Reason);
			RtlStringCbCopyW(Array[k].Type, sizeof(Array[k].Type), L"BugCheckReason");
			Array[k].Func = (PVOID)pReasonCallbackRecord->CallbackRoutine;
			Array[k].Context = (PVOID)pReasonCallbackRecord->Component;
			k++;
		} while (pListEntry->Flink != pHead);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("Exception: 0x%X", GetExceptionCode());
	}
exit6:
	PLIST_ENTRY listEntry;
	PCALLBACK_OBJECT callbackObject;
	PLIST_ENTRY ExpCallbackListHead = (PLIST_ENTRY)FindExpCallbackListHead();
	PEX_PUSH_LOCK ExpCallbackListLock = (PEX_PUSH_LOCK)FindExpCallbackListLock();
	//NTSTATUS status = STATUS_SUCCESS;

	DbgPrint("=== Starting Ex Callback Enumeration ===\n");
	if (!ExpCallbackListHead || !ExpCallbackListLock) goto exit13;

	InitializeCallbackTable();

	// 进入 guarded region（重要！）
	KeEnterGuardedRegion();

	// Windows 11 23H2 使用高版本加锁方式
	//ExAcquirePushLockSharedEx(g_ExpCallbackListLock, 0);
	ExAcquirePushLockShared(ExpCallbackListLock);
	/*else {
		// 低版本兼容（Windows 10之前）
		ExAcquirePushLockShared(g_ExpCallbackListLock);
	}*/

	// 检查链表是否为空
	if (IsListEmpty(ExpCallbackListHead)) {
		DbgPrint("No Ex callbacks found\n");
		//status = STATUS_NOT_FOUND;
		goto cleanup;
	}
	//ULONG count = 0;
	DbgPrint("ExpCallbackListHead:0x%p", ExpCallbackListHead);
	//DbgPrint("ExCallbackObjectType:0x%p", FindExCallbackObjectType());
	//DbgPrint("PsProcessType:0x%p", *PsProcessType);
	_try{
		listEntry = ExpCallbackListHead->Flink;
		PLIST_ENTRY currentlistEntry = ExpCallbackListHead->Flink;
		//while (listEntry != ExpCallbackListHead) {
		do {
			DbgPrint("listEntry:0x%p", listEntry);
			//count++;
			/*if (listEntry == currentlistEntry) {
				DbgPrint("Too many callback objects, possible list corruption. Stopping enumeration.\n");
				break;
			}*/
			callbackObject = CONTAINING_RECORD(listEntry, CALLBACK_OBJECT, ListEntry2);
			if (!callbackObject || !MmIsAddressValid(callbackObject)) {
				listEntry = listEntry->Flink;
				continue;
			}

			// [核心修复] 使用 __try/__finally 保护自旋锁
			KIRQL oldIrql = PASSIVE_LEVEL;  // 初始化为安全值
			//PWCHAR wszObjectName = NULL;
			_try {
				// 提升IRQL并保护链表操作
				oldIrql = KeAcquireSpinLockRaiseToDpc(&callbackObject->SpinLock);

				PLIST_ENTRY listEntry1 = &callbackObject->ListEntry;
				if (!IsListEmpty(listEntry1)) {
					for (PLIST_ENTRY iter = listEntry1->Flink; iter != listEntry1; iter = iter->Flink) {

						// 在移动到下一个节点前，先检查下一个节点的有效性
						/*if (!nextEntry || !MmIsAddressValid(nextEntry)) {
							DbgPrint("[Error] Invalid next pointer in list. Breaking loop.\n");
							break; // 链表可能损坏，安全退出内层循环
						}*/

						PCALLBACK_REGISTRATION registration = CONTAINING_RECORD(iter, CALLBACK_REGISTRATION, ListEntry);
						if (!registration || !MmIsAddressValid(registration)) {
							DbgPrint("[Error] Invalid registration object. Breaking loop.\n");
							//KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
							break;
						}

						// 所有检查通过，再安全移动指针
						//listEntry1 = nextEntry;

						// [内存管理] 修复容量检查逻辑
						if (k >= max_num) {  // 使用 >= 避免越界
							PCallbackInfo newArray = ExAllocatePool2(
								POOL_FLAG_NON_PAGED,
								sizeof(CallbackInfo) * (max_num + 100),
								'cbin'
							);
							if (!newArray) {
								DbgPrint("Failed to realloc buffer, stopping enumeration\n");
								KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
								goto cleanup;
							}
							RtlCopyMemory(newArray, Array, sizeof(CallbackInfo) * k);
							ExFreePoolWithTag(Array, 'cbin');
							Array = newArray;
							*pArray = Array;  // 更新调用者指针
							max_num += 100;
						}

						if (!NT_SUCCESS(QueryCallbackNameByPointer_Fast(callbackObject, Array[k].Type, sizeof(Array[k].Type))))
							RtlStringCbCopyNW(Array[k].Type, sizeof(Array[k].Type), L"\\Callback\\Unknown", wcslen(L"\\Callback\\Unknown") * 2);

						Array[k].Func = (PVOID)registration->CallbackFunction;
						Array[k].Context = registration->CallbackContext;
						k++;

						DbgPrint("  Type:%ws ObjectType:0x%p Function:0x%p Context:0x%p\n",
							Array[k - 1].Type,
							callbackObject,
							registration->CallbackFunction,
							registration->CallbackContext);

					}
				}
				KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
			}
			/*_finally{
				// [关键] 无论是否异常，必定恢复IRQL
				KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
			}*/
			_except(EXCEPTION_EXECUTE_HANDLER) {
				if (KeGetCurrentIrql() != PASSIVE_LEVEL) KeReleaseSpinLock(&callbackObject->SpinLock, oldIrql);
				DbgPrint("Exception in processing, skipping entry 0x%p\n", listEntry);
			}

			listEntry = listEntry->Flink;
		} while (listEntry != currentlistEntry);
	}
	_except(EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("Exception in enumeration: 0x%X\n", GetExceptionCode());
	}

cleanup:
	// 释放锁
	//if (g_ExReleasePushLockSharedEx) {
	ExReleasePushLockShared(ExpCallbackListLock);
	//}
	//else {
	//	ExReleasePushLockShared(g_ExpCallbackListLock);
	//}

	// 离开 guarded region
	KeLeaveGuardedRegion();
	CleanupCallbackTable();
exit13:
	return k;
}

NTSTATUS RemoveCreateProcessNotifyRoutine(PVOID CreateProcessNotifyRoutine) {
	return PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotifyRoutine, TRUE);
}

NTSTATUS RemoveCreateThreadNotifyRoutine(PVOID CreateThreadNotifyRoutine) {
	return PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)CreateThreadNotifyRoutine);
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