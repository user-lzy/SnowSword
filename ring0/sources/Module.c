#pragma once
#include "Module.h"
#include "Thread.h"
#include "Process.h"
#include "SSDT.h"

// 拒绝加载驱动
NTSTATUS DenyLoadDriver(PVOID pImageBase)
{
	NTSTATUS status = STATUS_SUCCESS;
	PMDL pMdl = NULL;
	PVOID pVoid = NULL;
	ULONG ulShellcodeLength = 16;
	UCHAR pShellcode[16] = { 0xB8, 0x22, 0x00, 0x00, 0xC0, 0xC3, 0x90, 0x90,
							0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)pDosHeader + pDosHeader->e_lfanew);
	//找到DriverEntry的基址
	PVOID pDriverEntry = (PVOID)((PUCHAR)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);

	pMdl = IoAllocateMdl(pDriverEntry, ulShellcodeLength, FALSE, FALSE, NULL);
	MmBuildMdlForNonPagedPool(pMdl);
	//MmProbeAndLockPages
	pVoid = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
	if (NULL == pVoid)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPagesSpecifyCache failed");
		return FALSE;
	}
	RtlCopyMemory(pVoid, pShellcode, ulShellcodeLength);
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return status;
}

// 拒绝加载 DLL 模块
BOOLEAN DenyLoadDll(PVOID pLoadImageBase)
{
	// DLL拒绝加载, 不能类似驱动那样直接在入口点返回拒绝加载信息. 这样达不到卸载DLL的效果.
	// 将文件头 前0x200 字节数据置零

	ULONG ulDataSize = 0x200;
	// 创建 MDL 方式修改内存
	PMDL pMdl = IoAllocateMdl(pLoadImageBase, ulDataSize, FALSE, FALSE, NULL);
	if (NULL == pMdl) return FALSE;
	MmBuildMdlForNonPagedPool(pMdl);
	PVOID pVoid = MmMapLockedPagesSpecifyCache(pMdl, KernelMode, MmCached, NULL, FALSE, NormalPagePriority);
	if (NULL == pVoid)
	{
		IoFreeMdl(pMdl);
		DbgPrint("MmMapLockedPagesSpecifyCache failed");
		return FALSE;
	}
	// 置零
	RtlZeroMemory(pVoid, ulDataSize);
	// 释放 MDL
	MmUnmapLockedPages(pVoid, pMdl);
	IoFreeMdl(pMdl);

	return TRUE;
}

void SetLoadImageNotifyRoutine(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,
	_In_ PIMAGE_INFO ImageInfo
)
{
	//DbgPrint("进程%d正在加载模块%wZ", ProcessId, FullImageName);

	// 拒绝加载指定模块
	//if (NULL != wcsstr(FullImageName->Buffer, L"DriverTest.sys") ||
	//	NULL != wcsstr(FullImageName->Buffer, L"Test.dll"))
	//Driver
	if (0 == ProcessId)
	{
		if (MyAdvancedOptions.DenyLoadDriver)
		{
			DbgPrint("进程%lld尝试加载驱动%wZ,已拒绝", (ULONG64)PsGetCurrentProcessId(), FullImageName);
			DenyLoadDriver(ImageInfo->ImageBase);
		}
	}
	//Dll
	else
	{
		//DbgPrint("Deny Load DLL\n");
		//DenyLoadDll(ImageInfo->ImageBase);
	}
}

NTSTATUS SetImageMonitorStatus(BOOLEAN flag)
{
	if (flag)
		return PsSetLoadImageNotifyRoutine(SetLoadImageNotifyRoutine);
	else
		return PsRemoveLoadImageNotifyRoutine(SetLoadImageNotifyRoutine);
}

PVOID GetModuleBase(UNICODE_STRING ModuleName, PULONG pModuleSize)
{
	PLIST_ENTRY ModuleListHead = PsLoadedModuleList;
	KeEnterCriticalRegion();
	PLIST_ENTRY CurrentEntry = ModuleListHead->Flink;

	while (CurrentEntry != ModuleListHead)
	{
		PKLDR_DATA_TABLE_ENTRY ModuleEntry = CONTAINING_RECORD(CurrentEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		UNICODE_STRING CurrentModuleName = ModuleEntry->BaseDllName;

		if (RtlCompareUnicodeString(&ModuleName, &CurrentModuleName, TRUE) == 0)
		{
			KeLeaveCriticalRegion();
			*pModuleSize = ModuleEntry->SizeOfImage;
			return (PVOID)ModuleEntry->DllBase;
		}

		CurrentEntry = CurrentEntry->Flink;
	}
	KeLeaveCriticalRegion();
	*pModuleSize = 0;
	return NULL;
}

PVOID GetProcAddress(PCHAR FunctionName, PVOID ModuleBase)
{
	if (!ModuleBase) return NULL;
	DbgPrint("ModuleBase:0x%p", ModuleBase);
	if (!MmIsAddressValid(ModuleBase) || FunctionName == NULL) return NULL;

	__try {
		PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ModuleBase;
		PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)ModuleBase + DosHeader->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)ModuleBase + NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		ULONG* NameTable = (ULONG*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfNames);
		SHORT* OrdinalTable = (SHORT*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfNameOrdinals);
		ULONG* AddressTable = (ULONG*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfFunctions);

		for (ULONG i = 0; i < ExportDirectory->NumberOfNames; i++)
		{
			PCHAR ExportName = (PCHAR)((PUCHAR)ModuleBase + NameTable[i]);
			if (_stricmp(ExportName, FunctionName) == 0)
			{
				SHORT Ordinal = OrdinalTable[i];
				return (PVOID)((PUCHAR)ModuleBase + AddressTable[Ordinal]);
			}
		}

		return NULL;
	}
	__except (1) {
		return NULL;
	}
}

PVOID GetR3ProcAddress(HANDLE ProcessId, PCHAR FunctionName, PVOID ModuleBase)
{
	if (!ModuleBase) return NULL;
	//DbgPrint("ModuleBase:0x%p", ModuleBase);
	if (!MmIsAddressValid(ModuleBase) || FunctionName == NULL) return NULL;

	HANDLE ProcessHandle = NULL;
	if (!NT_SUCCESS(OpenProcess(ProcessId, &ProcessHandle))) return NULL;

	PIMAGE_DOS_HEADER DosHeader = NULL;
	//NtReadVirtualMemory(ProcessHandle, ModuleBase, DosHeader, )
	PIMAGE_NT_HEADERS NtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)ModuleBase + DosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PUCHAR)ModuleBase + NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	ULONG* NameTable = (ULONG*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfNames);
	SHORT* OrdinalTable = (SHORT*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfNameOrdinals);
	ULONG* AddressTable = (ULONG*)((PUCHAR)ModuleBase + ExportDirectory->AddressOfFunctions);

	for (ULONG i = 0; i < ExportDirectory->NumberOfNames; i++)
	{
		PCHAR ExportName = (PCHAR)((PUCHAR)ModuleBase + NameTable[i]);
		if (_stricmp(ExportName, FunctionName) == 0)
		{
			SHORT Ordinal = OrdinalTable[i];
			return (PVOID)((PUCHAR)ModuleBase + AddressTable[Ordinal]);
		}
	}

	return NULL;
}

void EnumerateFilterDrivers() {
	PLIST_ENTRY moduleList = PsLoadedModuleList;
	PLIST_ENTRY currentList = moduleList->Flink;
	PDRIVER_OBJECT driverObject;
	PDEVICE_OBJECT* deviceObjectList = NULL;
	ULONG deviceObjectListSize = 0;
	ULONG actualNumberOfDeviceObjects = 0;
	NTSTATUS status;

	KeEnterCriticalRegion();

	__try {
		while (currentList != moduleList)
		{
			PKLDR_DATA_TABLE_ENTRY moduleEntry = CONTAINING_RECORD(currentList, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			driverObject = (PDRIVER_OBJECT)moduleEntry->DllBase;

			// 第一次调用IoEnumerateDeviceObjectList，获取设备对象列表的大小
			status = IoEnumerateDeviceObjectList(driverObject, NULL, 0, &actualNumberOfDeviceObjects);
			if (status == STATUS_BUFFER_TOO_SMALL) {
				deviceObjectListSize = actualNumberOfDeviceObjects * sizeof(PDEVICE_OBJECT);
				deviceObjectList = (PDEVICE_OBJECT*)ExAllocatePool2(POOL_FLAG_NON_PAGED, deviceObjectListSize, 'dOeD');
				if (deviceObjectList) {
					// 第二次调用IoEnumerateDeviceObjectList，获取设备对象列表
					status = IoEnumerateDeviceObjectList(driverObject, deviceObjectList, deviceObjectListSize, &actualNumberOfDeviceObjects);
					if (NT_SUCCESS(status)) {
						for (ULONG i = 0; i < actualNumberOfDeviceObjects; i++) {
							if (i * sizeof(PDEVICE_OBJECT) < deviceObjectListSize) {
								PDEVICE_OBJECT deviceObject = deviceObjectList[i];
								DbgPrint("Device Name: %wZ, Driver Name: %wZ\n", deviceObject->DriverObject->DriverName, ((PKLDR_DATA_TABLE_ENTRY)deviceObject->DriverObject->DriverSection)->BaseDllName);
							}
						}
					}
					ExFreePoolWithTag(deviceObjectList, 'dOeD');
				}
			}
			currentList = currentList->Flink;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("Exception occurred while enumerating filter drivers.code: %x\n", GetExceptionCode());
	}

	KeLeaveCriticalRegion();
}

VOID KernelApcRoutine(
	PKAPC Apc,
	PKNORMAL_ROUTINE* NormalRoutine,
	PVOID* NormalContext,
	PVOID* SystemArgument1,
	PVOID* SystemArgument2
)
{
	UNREFERENCED_PARAMETER(NormalRoutine);
	UNREFERENCED_PARAMETER(NormalContext);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);

	ExFreePoolWithTag(Apc, 'apcX');
}

NTSTATUS InjectDllByApc(
	HANDLE ProcessId,
	HANDLE ThreadId,
	PCWSTR DllPath
)
{
	UNREFERENCED_PARAMETER(DllPath);
	NTSTATUS status;
	PEPROCESS Process = NULL;
	PETHREAD Thread = NULL;

	status = PsLookupProcessByProcessId(ProcessId, &Process);
	if (!NT_SUCCESS(status)) return status;

	status = PsLookupThreadByThreadId(ThreadId, &Thread);
	if (!NT_SUCCESS(status)) {
		ObDereferenceObject(Process);
		return status;
	}

	HANDLE hProcess = NULL;
	status = ObOpenObjectByPointer(Process, OBJ_KERNEL_HANDLE,
		NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &hProcess);
	if (!NT_SUCCESS(status)) goto _exit;

	//---------------------------------------------------------
	// 在远程进程分配 DLL 路径缓冲区
	//---------------------------------------------------------
	SIZE_T RegionSize = 0x1000;
	PWCHAR RemotePath = NULL;

	status = ZwAllocateVirtualMemory(
		hProcess,
		&RemotePath,
		0,
		&RegionSize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE
	);

	if (!NT_SUCCESS(status)) goto _exit;

	// 写入 DLL 路径
	//SIZE_T bytes = (wcslen(DllPath) + 1) * sizeof(WCHAR);
	//memcpy(ProcessId, (PVOID)DllPath, RemotePath, bytes);

	//---------------------------------------------------------
	// 取得 LoadLibraryW 地址（用户态）
	//---------------------------------------------------------
	//ULONG64 pLoadLibraryW = GetProcAddress(
	//	ProcessId, L"kernel32.dll", "LoadLibraryW"
	//);

	//if (!pLoadLibraryW) {
	//	status = STATUS_NOT_FOUND;
	//	goto _exit;
	//}

	//---------------------------------------------------------
	// 构造 APC
	//---------------------------------------------------------
	KAPC* pApc = (KAPC*)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(KAPC), 'apcX');

	/*KeInitializeApc(
		pApc,
		Thread,
		OriginalApcEnvironment,
		(PKKERNEL_ROUTINE)KernelApcRoutine,     // 内核 APC
		NULL,
		(PKNORMAL_ROUTINE)pLoadLibraryW,        // 用户 APC (LoadLibraryW)
		UserMode,
		RemotePath                               // NormalContext 参数 = DLL路径
	);*/

	BOOLEAN inserted = KeInsertQueueApc(
		pApc,
		RemotePath,   // SystemArgument1
		NULL,         // SystemArgument2
		IO_NO_INCREMENT
	);

	if (!inserted)
		status = STATUS_UNSUCCESSFUL;
	else
		status = STATUS_SUCCESS;

_exit:
	if (Thread) ObDereferenceObject(Thread);
	if (Process) ObDereferenceObject(Process);
	if (hProcess) ZwClose(hProcess);

	return status;
}

NTSTATUS DumpKernelModule(
	PVOID ModuleBase,
	PVOID pOutputBuffer,
	PSIZE_T pBytesWritten
)
{
	// 1. 读取PE头
	IMAGE_DOS_HEADER dosHeader = { 0 };
	MM_COPY_ADDRESS Source = { 0 };
	Source.VirtualAddress = ModuleBase;
	SIZE_T bytesRead = 0;

	MmCopyMemory(&dosHeader, Source, sizeof(dosHeader),
		MM_COPY_MEMORY_VIRTUAL, &bytesRead);

	// 2. 读取NT头获取完整大小
	IMAGE_NT_HEADERS64 ntHeaders = { 0 };
	Source.VirtualAddress = (PVOID)((ULONG_PTR)ModuleBase + dosHeader.e_lfanew);
	MmCopyMemory(&ntHeaders, Source, sizeof(ntHeaders),
		MM_COPY_MEMORY_VIRTUAL, &bytesRead);

	SIZE_T imageSize = ntHeaders.OptionalHeader.SizeOfImage;

	// 3. 关键：按页DUMP，跳过无效页但保留对齐
	for (SIZE_T offset = 0; offset < imageSize; offset += PAGE_SIZE) {
		PVOID currentPage = (PVOID)((ULONG_PTR)ModuleBase + offset);
		PVOID destPage = (PVOID)((PUCHAR)pOutputBuffer + offset);

		// 检查页有效
		//if (MmIsAddressValid(currentPage)) {
		_try {
			MmCopyMemory(destPage,
				(MM_COPY_ADDRESS) {
				.VirtualAddress = currentPage
			},
				PAGE_SIZE,
				MM_COPY_MEMORY_VIRTUAL,
				&bytesRead);
			*pBytesWritten += bytesRead;
		}
		//else {
		_except(EXCEPTION_EXECUTE_HANDLER) {
			// 关键：用0填充无效页以保持PE结构对齐
			RtlZeroMemory(destPage, PAGE_SIZE);
			DbgPrint("[DumpKernelModule] 填充无效页: %p", currentPage);
		}
	}

	return STATUS_SUCCESS;
}