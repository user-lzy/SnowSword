#pragma once

#include "HookSysenter.h"

/*ULONGLONG GetKiFastCallEntry()
{
	ULONGLONG KiFastCallEntry = 0;
	WRMSR(MSR_SYSENTER_EIP, (ULONGLONG)(ULONG)KiFastCallEntry);
	return KiFastCallEntry;
}*/