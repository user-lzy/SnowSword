#pragma once

#include <ntifs.h>

// 声明自定义结构体
struct AdvancedOptions {
	BOOLEAN DenyCreateProcess;
	BOOLEAN DenyCreateThread;
	BOOLEAN DenyRemoteThread;
	BOOLEAN DenyLoadDriver;
	BOOLEAN DenyLoadDll;
	BOOLEAN DenyAccessRegistry;
	BOOLEAN IsProtectProcess;
	BOOLEAN IsProtectThread;
};
typedef struct AdvancedOptions* PAdvancedOptions;

typedef struct _HANDLEINFO {
	HANDLE Handle;
	PVOID Object;
} HANDLEINFO, * PHANDLEINFO;

// 声明全局变量
extern PKEVENT pKernelEvent;
extern struct AdvancedOptions MyAdvancedOptions;

typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned _int64 ULONG_PTR;
typedef void* PVOID;
typedef unsigned long DWORD;
#ifndef VOID
	typedef void VOID;
#endif

// 假设pUserUnicodeString是用户模式传来的UNICODE_STRING指针
NTSTATUS ValidateUserUnicodeString(PUNICODE_STRING pUserUnicodeString, PUNICODE_STRING pKernelUnicodeString);

PVOID GetPspCidTable();

VOID MyExEnumHandleTable(
	PVOID TableBase,
	BOOLEAN(*CallbackFunction)(HANDLEINFO*, PVOID),
	PVOID Context
);

