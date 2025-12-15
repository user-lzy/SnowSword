#pragma once

#include "Registry.h"
LARGE_INTEGER Cookie = { 0 };

// 获取注册表完整路径
BOOLEAN GetFullPath(PUNICODE_STRING pRegistryPath, PVOID pRegistryObject)
{
    // 判断数据地址是否有效
    if ((FALSE == MmIsAddressValid(pRegistryObject)) ||
        (NULL == pRegistryObject))
    {
        return FALSE;
    }
    // 申请内存
    ULONG ulSize = 512;
    PVOID lpObjectNameInfo = ExAllocatePool2(POOL_FLAG_NON_PAGED, ulSize, 'lpon');
    if (NULL == lpObjectNameInfo)
    {
        return FALSE;
    }
    // 获取注册表路径
    ULONG ulRetLen = 0;
    NTSTATUS status = ObQueryNameString(pRegistryObject, (POBJECT_NAME_INFORMATION)lpObjectNameInfo, ulSize, &ulRetLen);
    if (!NT_SUCCESS(status))
    {
        ExFreePool(lpObjectNameInfo);
        return FALSE;
    }
    // 复制
    RtlCopyUnicodeString(pRegistryPath, (PUNICODE_STRING)lpObjectNameInfo);
    // 释放内存
    ExFreePoolWithTag(lpObjectNameInfo, 'lpon');
    return TRUE;
}

/*NTSTATUS GetProcessImageFileName(PEPROCESS Process, PUNICODE_STRING ProcessImageFileName)
{
    NTSTATUS status;
    ULONG returnedLength;
    PVOID buffer;
    ULONG bufferSize = 512; // 初始缓冲区大小

    //buffer = ExAllocatePoolWithTag(NonPagedPool, bufferSize, 'proc');
    if (buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //status = ZwQueryInformationProcess(ZwCurrentProcess(), ProcessImageFileName, buffer, bufferSize, &returnedLength);
    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        ExFreePool(buffer);
        bufferSize = returnedLength;
        //buffer = ExAllocatePoolWithTag(NonPagedPool, bufferSize, 'proc');
        if (buffer == NULL)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //status = ZwQueryInformationProcess(ZwCurrentProcess(), ProcessImageFileName, buffer, bufferSize, &returnedLength);
    }

    if (NT_SUCCESS(status))
    {
        RtlInitUnicodeString(ProcessImageFileName, (PCWSTR)buffer);
    }
    else
    {
        ExFreePool(buffer);
    }

    return status;
}*/

NTSTATUS RegMonitorCallback(PVOID CallbackContext, PVOID Argument1, PVOID Argument2)
{
	UNREFERENCED_PARAMETER(CallbackContext);
    REG_NOTIFY_CLASS RegNotifyClass = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;

    // 获取当前进程名称
    /*PEPROCESS currentProcess = PsGetCurrentProcess();
    UNICODE_STRING currentProcessName;
    NTSTATUS status = GetProcessImageFileName(currentProcess, &processImageFileName);
    if (!NT_SUCCESS(status)) return STATUS_SUCCESS;

    // 检查是否为受保护的进程
    if (RtlEqualUnicodeString(&unicodeProcessName, &ProtectedProcessName, TRUE))
    {
        //RtlFreeUnicodeString(&unicodeProcessName);
        return STATUS_ACCESS_DENIED;
    }

    //RtlFreeUnicodeString(&unicodeProcessName);*/

    // 初始化受保护的注册表路径
	UNICODE_STRING ProtectedKeyPath;
	UNICODE_STRING ProtectedValueName;
    RtlInitUnicodeString(&ProtectedKeyPath, L"\\Registry\\Machine\\Software\\MyProtectedKey");
    RtlInitUnicodeString(&ProtectedValueName, L"MyProtectedValue");

    switch (RegNotifyClass)
    {
    case RegNtPreSetValueKey:
    {
        PREG_SET_VALUE_KEY_INFORMATION preSetValueInfo = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
        UNICODE_STRING ustrRegistryPath = { 0 };
        if (GetFullPath(&ustrRegistryPath, preSetValueInfo->Object))
        {
            /*DbgPrint("设置注册表值: %wZ, ValueName: %wZ, Type: %lu, DataSize: %lu",
                &ustrRegistryPath,
                preSetValueInfo->ValueName,
                preSetValueInfo->Type,
                preSetValueInfo->DataSize);*/
        }
        break;
    }
    case RegNtPreDeleteKey:
    {
        PREG_DELETE_KEY_INFORMATION preDeleteKeyInfo = (PREG_DELETE_KEY_INFORMATION)Argument2;
        UNICODE_STRING ustrRegistryPath = { 0 };
        if (GetFullPath(&ustrRegistryPath, preDeleteKeyInfo->Object))
        {
            DbgPrint("尝试删除注册表键: %wZ", &ustrRegistryPath);
            if (RtlEqualUnicodeString(&ustrRegistryPath, &ProtectedKeyPath, TRUE)|| MyAdvancedOptions.DenyAccessRegistry)
            {
                DbgPrint("已禁止");
                return STATUS_REGISTRY_CORRUPT;//注册表文件已损坏
            }
        }
        break;
    }
    case RegNtPreCreateKey:
    {
        PREG_CREATE_KEY_INFORMATION preCreateKeyInfo = (PREG_CREATE_KEY_INFORMATION)Argument2;
        DbgPrint("创建注册表键: %wZ", preCreateKeyInfo->CompleteName);
        break;
    }
    case RegNtPreDeleteValueKey:
    {
        PREG_DELETE_VALUE_KEY_INFORMATION preDeleteValueInfo = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
        UNICODE_STRING ustrRegistryPath = { 0 };
        if (GetFullPath(&ustrRegistryPath, preDeleteValueInfo->Object))
        {
            DbgPrint("尝试删除注册表值: %wZ, ValueName: %wZ", &ustrRegistryPath, preDeleteValueInfo->ValueName);
            if (RtlEqualUnicodeString(&ustrRegistryPath, &ProtectedKeyPath, TRUE) &&
                RtlEqualUnicodeString(preDeleteValueInfo->ValueName, &ProtectedValueName, TRUE))
            {
                DbgPrint("已禁止");
                return STATUS_REGISTRY_HIVE_UNLOADED;//注册表配置单元已卸载
                //STATUS_REGISTRY_NO_SECRETS (0xC00002E5) 注册表中没有秘密
            }
        }
        break;
    }
    case RegNtPreQueryValueKey:
    {
        PREG_QUERY_VALUE_KEY_INFORMATION preQueryValueInfo = (PREG_QUERY_VALUE_KEY_INFORMATION)Argument2;
        UNICODE_STRING ustrRegistryPath = { 0 };
        if (GetFullPath(&ustrRegistryPath, preQueryValueInfo->Object))
        {
            /*DbgPrint("查询注册表值: %wZ, ValueName: %wZ",
                &ustrRegistryPath,
                preQueryValueInfo->ValueName);*/
        }
        break;
    }
    case RegNtPreQueryKey:
    {
        PREG_QUERY_KEY_INFORMATION preQueryKeyInfo = (PREG_QUERY_KEY_INFORMATION)Argument2;
        UNICODE_STRING ustrRegistryPath = { 0 };
        if (GetFullPath(&ustrRegistryPath, preQueryKeyInfo->Object))
        {
            /*DbgPrint("查询注册表键: %wZ",
                &ustrRegistryPath);*/
        }
        break;
    }
    // 处理其他注册表操作
    default:
        break;
    }
    return STATUS_SUCCESS;
}

NTSTATUS SetRegMonitorStatus(PDRIVER_OBJECT DriverObject, BOOLEAN flag)
{
    if (flag)
    {
        Cookie.QuadPart = 123456;

        UNICODE_STRING Altitude = { 0 };
        RtlInitUnicodeString(&Altitude, L"321999");

        return CmRegisterCallbackEx(RegMonitorCallback, &Altitude, DriverObject, NULL, &Cookie, NULL);   //启用注册表回调
	}
    else
    {
        return CmUnRegisterCallback(Cookie);   //关闭注册表回调
    }
}