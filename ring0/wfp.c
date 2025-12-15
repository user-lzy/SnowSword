#include "wfp.h"

// 枚举WFP过滤器的函数
/*NTSTATUS EnumWfpFilters()
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE engineHandle = NULL;
    FWPM_FILTER_ENUM_TEMPLATE0 enumTemplate = { 0 };
    UINT32 numFilters = 0;
    FWPM_FILTER0** filters = NULL;  // 修正为二级指针
    UINT32 i = 0;

    // 步骤1：打开WFP内核模式会话
    status = FwpmEngineOpen(
        NULL,
        NULL,  // 内核模式下使用NULL而不是FWPM_SESSION_FLAG_KERNEL
        NULL,
        NULL,
        &engineHandle
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrint("FwpmEngineOpen failed: 0x%X\n", status);
        return status;
    }

    // 步骤2：配置枚举模板
    // 内核模式下使用不同的方式指定层
    RtlZeroMemory(&enumTemplate, sizeof(enumTemplate));
    //enumTemplate.layerKey = GUID_NULL;  // 使用GUID_NULL表示所有层

    // 步骤3：枚举WFP过滤器
    status = FwpmFilterEnum0(
        engineHandle,
        &enumTemplate,
        0,
        (FWPM_FILTER0***)&filters,  // 类型转换
        &numFilters
    );
    if (!NT_SUCCESS(status))
    {
        DbgPrint("FwpmFilterEnum0 failed: 0x%X\n", status);
        FwpmEngineClose(engineHandle);
        return status;
    }

    // 步骤4：遍历并打印过滤器信息
    DbgPrint("枚举到 %d 个WFP过滤器：\n", numFilters);
    for (i = 0; i < numFilters; i++)
    {
        FWPM_FILTER0* pFilter = filters[i];  // 直接访问数组元素
        if (pFilter == NULL) continue;

        DbgPrint("过滤器 %d：\n", i);
        DbgPrint("  ID: 0x%llX\n", pFilter->filterId);
        DbgPrint("  层Key: %!GUID!\n", &pFilter->layerKey);  // 使用GUID打印
        DbgPrint("  操作: %d\n", pFilter->action.type);
        DbgPrint("  权重: %d\n", pFilter->weight.uint32);
        DbgPrint("------------------------\n");
    }

    // 步骤5：清理资源
    if (filters != NULL)
    {
        FwpmFreeMemory0((void**)&filters);
    }
    FwpmEngineClose(engineHandle);

    return STATUS_SUCCESS;
}*/