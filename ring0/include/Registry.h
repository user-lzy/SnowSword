#pragma once

#include "global.h"

extern LARGE_INTEGER Cookie;

#define STATUS_REGISTRY_HIVE_UNLOADED (0xC0000423)

// 获取注册表完整路径
BOOLEAN GetFullPath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject);

NTSTATUS RegMonitorCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2);

NTSTATUS SetRegMonitorStatus(PDRIVER_OBJECT DriverObject, BOOLEAN flag);