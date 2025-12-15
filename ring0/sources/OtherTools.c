#pragma once

#include "OtherTools.h"

VOID ForceShutdown()
{
	typedef void(__fastcall* FCSD)(void);
	/*
		mov ax, 2001h
		mov dx, 1004h
		out dx, ax
		ret
	*/
	FCSD fcsd = NULL;
	UCHAR shellcode[12] = "\x66\xB8\x01\x20\x66\xBA\x04\x10\x66\xEF\xC3";
	fcsd = (FCSD)ExAllocatePool2(POOL_FLAG_NON_PAGED, 11, 'fscd');
	if (!fcsd) {
		DbgPrint("Memory allocation failed for shellcode");
		return;
	}
	memcpy((PVOID)fcsd, shellcode, 11);
	fcsd();
}

VOID ForceReboot()
{
	typedef void(__fastcall* FCRB)(void);
	/*
	mov al, 0FEh
	out 64h, al
	ret
	*/
	FCRB fcrb = NULL;
	UCHAR shellcode[6] = "\xB0\xFE\xE6\x64\xC3";
	fcrb = (FCRB)ExAllocatePool2(POOL_FLAG_NON_PAGED, 5, 'fcrb');
	if (!fcrb)
	{
		DbgPrint("Memory allocation failed for shellcode");
		return;
	}
	memcpy((PVOID)fcrb, shellcode, 5);
	fcrb();
}