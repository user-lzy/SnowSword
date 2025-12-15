#include "SSDT.h"

PKSERVICE_TABLE_DESCRIPTOR_TABLE KeServiceDescriptorTable = NULL;
PKSERVICE_TABLE_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow = NULL;

static PVOID GetKiSystemServiceRepeat()
{
	//获取ZwSetEvent地址
	UNICODE_STRING ZwSetEventName = RTL_CONSTANT_STRING(L"ZwSetEvent");
	PVOID ZwSetEventAddr = MmGetSystemRoutineAddress(&ZwSetEventName);
	if (ZwSetEventAddr == NULL) {
		DbgPrint("获取ZwSetEvent地址失败");
		return NULL;
	}

	PVOID result = NULL;
	ULONG offset = 0;
	//获取KiServiceInternal地址
	/*
	fffff802`af2a0fd9 e9e2770100      jmp     nt!KiServiceInternal (fffff802`af2b87c0)
	*/
	UCHAR ulSpecialCode1[] = {0xe9};
	result = SearchSpecialCode(ZwSetEventAddr, 0x30, ulSpecialCode1, 1);
	if (result == NULL) {
		DbgPrint("获取KiServiceInternal地址失败");
		return NULL;
	}
	offset = *(PULONG)((PUCHAR)result + 1);
	PVOID KiServiceInternalAddr = (PVOID)((PUCHAR)result + 5 + offset);
	DbgPrint("KiServiceInternalAddr:0x%p", KiServiceInternalAddr);

	//获取KiSystemServiceStart地址
	/*
	fffff802`af2b8822 4c8d1d97030000  lea     r11,[nt!KiSystemServiceStart (fffff802`af2b8bc0)]
	*/
	UCHAR ulSpecialCode2[] = { 0x4c, 0x8d, 0x1d };
	result = SearchSpecialCode(KiServiceInternalAddr, 0x100, ulSpecialCode2, 3);
	if (result == NULL) {
		DbgPrint("获取KiSystemServiceStart地址失败");
		return NULL;
	}
	offset = *(PULONG)((PUCHAR)result + 3);
	PVOID KiSystemServiceStartAddr = (PVOID)((PUCHAR)result + 7 + offset);
	DbgPrint("KiSystemServiceStartAddr:0x%p", KiSystemServiceStartAddr);

	//获取KiSystemServiceRepeat地址
	/*
	fffff802`b2ab94f9 4889a390000000  mov     qword ptr [rbx+90h],rsp
	fffff802`b2ab9500 0f84cef6ffff    je      nt!KiSystemServiceRepeat (fffff802`b2ab8bd4)
	*/
	UCHAR ulSpecialCode4[] = { 0x48, 0x89, 0xa3, 0x90, 0x00, 0x00, 0x00, 0x0f, 0x84 };
	result = SearchSpecialCode(KiSystemServiceStartAddr, 0x1000, ulSpecialCode4, 9);
	if (result == NULL) {
		DbgPrint("获取KiSystemServiceRepeat地址失败");
		return NULL;
	}
	LONG offset2 = *(PLONG)((PUCHAR)result + 9);
	PVOID KiSystemServiceRepeatAddr = (PVOID)((PUCHAR)result + 13 + offset2);
	DbgPrint("KiSystemServiceRepeatAddr:0x%p", KiSystemServiceRepeatAddr);
	return KiSystemServiceRepeatAddr;
}

PVOID GetKeServiceDescriptorTable()
{
	/*
	fffff802`b2ab8bd4 4c8d15e58cb400  lea     r10,[nt!KeServiceDescriptorTable (fffff802`b36018c0)]
	*/
	PVOID KiSystemServiceRepeatAddr = GetKiSystemServiceRepeat();
	if (KiSystemServiceRepeatAddr == NULL)
	{
		DbgPrint("KiSystemServiceRepeatAddr is NULL");
		return NULL;
	}
	UCHAR ulSpecialCode[] = { 0x4c, 0x8d, 0x15 };
	PVOID result = SearchSpecialCode(KiSystemServiceRepeatAddr, 0x20, ulSpecialCode, 3);
	if (result == NULL)
	{
		DbgPrint("获取KeServiceDescriptorTable失败");
		return NULL;
	}
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID KeServiceDescriptorTableAddr = (PVOID)((PUCHAR)result + 7 + offset);
	DbgPrint("KeServiceDescriptorTableAddr: 0x%p \n", KeServiceDescriptorTableAddr);
	return KeServiceDescriptorTableAddr;
}

PVOID GetKeServiceDescriptorTableShadow()
{
	/*
	fffff802`b2ab8bdb 4c8d1d9ed69000  lea     r11,[nt!KeServiceDescriptorTableShadow (fffff802`b33c6280)]	
	*/
	PVOID KiSystemServiceRepeatAddr = GetKiSystemServiceRepeat();
	if (KiSystemServiceRepeatAddr == NULL)
	{
		DbgPrint("KiSystemServiceRepeatAddr is NULL");
		return NULL;
	}
	UCHAR ulSpecialCode[] = { 0x4c, 0x8d, 0x1d };
	PVOID result = SearchSpecialCode(KiSystemServiceRepeatAddr, 0x20, ulSpecialCode, 3);
	if (result == NULL)
	{
		DbgPrint("获取KeServiceDescriptorTableShadow失败");
		return NULL;
	}
	ULONG offset = *(PULONG)((PUCHAR)result + 3);
	PVOID KeServiceDescriptorTableShadowAddr = (PVOID)((PUCHAR)result + 7 + offset);
	DbgPrint("KeServiceDescriptorTableShadowAddr: 0x%p \n", KeServiceDescriptorTableShadowAddr);
	return KeServiceDescriptorTableShadowAddr;
}

// 根据系统调用号获取函数地址
PVOID GetSSDTFuncAddrByIndex(ULONG sysCallNumber) {
	if (sysCallNumber >= KeServiceDescriptorTable->NativeApiTable.NumberOfServices) {
		DbgPrint("[GetSSDTFuncAddrByIndex]系统调用号%d超出范围", sysCallNumber);
		return NULL;
	}
	ULONG entry = KeServiceDescriptorTable->NativeApiTable.ServiceTableBase[sysCallNumber];
	LONG_PTR offset = (LONG_PTR)(entry >> 4); // 必须符号扩展
	PVOID NtFuncAddr = (PVOID)((ULONG_PTR)KeServiceDescriptorTable->NativeApiTable.ServiceTableBase + offset);
	return NtFuncAddr;
}

// 根据系统调用号获取函数地址
PVOID GetSSSDTFuncAddrByIndex(ULONG sysCallNumber) {
	if (sysCallNumber >= KeServiceDescriptorTableShadow->Win32kApiTable.NumberOfServices) {
		DbgPrint("[GetSSSDTFuncAddrByIndex]系统调用号%d超出范围", sysCallNumber);
		return NULL;
	}
	ULONG entry = KeServiceDescriptorTableShadow->Win32kApiTable.ServiceTableBase[sysCallNumber];
	LONG offset;
	if (entry & 0x80000000) {
		// 负偏移 - 正确的符号扩展
		offset = (LONG)((entry >> 4) | 0xF0000000);  // 扩展高4位
	}
	else {
		// 正偏移
		offset = (LONG)(entry >> 4);
	}
	return (PVOID)((ULONG_PTR)KeServiceDescriptorTableShadow->Win32kApiTable.ServiceTableBase + offset);
}

PVOID GetSSDTFuncAddr(PWCHAR SSDTFuncName) {
	if (!KeServiceDescriptorTable) return NULL; // 错误处理
	//获取ZwXXX地址,将函数名中的"Nt"替换为"Zw"
	// 安全地构建Zw函数名
	WCHAR ZwFuncName[256] = { 0 };

	// 检查是否以"Nt"开头且有足够长度
	if (SSDTFuncName[0] == L'N' && SSDTFuncName[1] == L't' && SSDTFuncName[2] != L'\0') {
		// 安全地构建"Zw" + 剩余部分
		ZwFuncName[0] = L'Z';
		ZwFuncName[1] = L'w';

		// 复制剩余字符，确保不溢出缓冲区
		SIZE_T i = 2;
		while (SSDTFuncName[i] != L'\0' && i < (sizeof(ZwFuncName) / sizeof(WCHAR) - 1)) {
			ZwFuncName[i] = SSDTFuncName[i];
			i++;
		}
		ZwFuncName[i] = L'\0';
	}
	else {
		// 如果不是有效的Nt函数名，直接返回nullptr
		DbgPrint("无效的Nt函数名: %ws", SSDTFuncName);
		return NULL;
	}
	// 关键修改：用RtlInitUnicodeString初始化动态字符串（支持非常量PWCHAR）
	UNICODE_STRING ZwFuncUnicodeString;
	RtlInitUnicodeString(&ZwFuncUnicodeString, ZwFuncName); // 显式初始化，支持动态字符串
	PVOID ZwFuncAddr = MmGetSystemRoutineAddress(&ZwFuncUnicodeString);
	if (ZwFuncAddr == NULL) {
		DbgPrint("获取%ws地址失败", ZwFuncName);
		return NULL;
	}
	/*
	nt!ZwLoadDriver:
	fffff805`d32a2fc0 488bc4          mov     rax,rsp
	fffff805`d32a2fc3 fa              cli
	fffff805`d32a2fc4 4883ec10        sub     rsp,10h
	fffff805`d32a2fc8 50              push    rax
	fffff805`d32a2fc9 9c              pushfq
	fffff805`d32a2fca 6a10            push    10h
	fffff805`d32a2fcc 488d059d410000  lea     rax,[nt!KiServiceLinkage (fffff805`d32a7170)]
	fffff805`d32a2fd3 50              push    rax
	fffff805`d32a2fd4 b80e010000      mov     eax,10Eh
	fffff805`d32a2fd9 e9e2570100      jmp     nt!KiServiceInternal (fffff805`d32b87c0)  Branch
	*/
	//获取系统调用号
	UCHAR ulSpecialCode[] = { 0xb8 };
	PVOID result = SearchSpecialCode(ZwFuncAddr, 0x20, ulSpecialCode, sizeof(ulSpecialCode));
	if (result == NULL) {
		DbgPrint("获取系统调用号失败");
		return NULL;
	}
	ULONG sysCallNumber = *(PULONG)((PUCHAR)result + 1);
	DbgPrint("SysCallNumber: %lu \n", sysCallNumber);
	// 根据系统调用号获取函数地址
	if (sysCallNumber >= KeServiceDescriptorTable->NativeApiTable.NumberOfServices) {
		DbgPrint("系统调用号超出范围");
		return NULL;
	}
	ULONG entry = KeServiceDescriptorTable->NativeApiTable.ServiceTableBase[sysCallNumber];
	LONG_PTR offset = (LONG_PTR)(entry >> 4); // 必须符号扩展
	PVOID NtFuncAddr = (PVOID)((ULONG_PTR)KeServiceDescriptorTable->NativeApiTable.ServiceTableBase + offset);
	return NtFuncAddr;
}