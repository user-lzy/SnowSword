#include "Netstat.h"
#include "OtherFunctions.h"
#include <ntddk.h>
#include <ndis.h>

// ===================== 1. 宏定义 / 类型定义 =====================

// ✅ 100% 匹配你IDA反汇编的 NDIS_FILTER_BLOCK 结构体
typedef struct _NDIS_FILTER_BLOCK {
    USHORT    Signature;        // 0x00 固定签名 0x104
    USHORT    Size;             // 0x02 结构体大小
    ULONG     _Padding1;        // 0x04-0x07 对齐填充(x64必须)
    PVOID     Next;             // 0x08 链表指针(反汇编: mov [rsi+8], rdx)
    UCHAR     Reserved[0x60];   // 0x10~0x67 保留字段(核心修正)
    UNICODE_STRING FilterName;  // 0x68 过滤驱动名称(修正偏移)
    GUID      FilterGuid;       // 0x78 过滤驱动GUID
} NDIS_FILTER_BLOCK, * PNDIS_FILTER_BLOCK;

// ===================== 2. 内部辅助函数 =====================
// 从NdisFRegisterFilterDriver中提取 链表头 + 自旋锁（修正版：全RIP相对寻址）
static NTSTATUS
GetNdisFilterGlobalData(
    OUT PVOID* ppListHead,    // 输出：全局链表头 ndisFilterDriverList
    OUT PVOID* ppSpinLock     // 输出：自旋锁 ndisFilterDriverListLock
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PVOID pNdisFunc = NULL;
    PVOID pMatchList = NULL;
    PVOID pMatchLock = NULL;

    // 1. 获取函数基址
    pNdisFunc = KernelGetProcAddress("ndis.sys", "NdisFRegisterFilterDriver");
    if (!pNdisFunc || !ppListHead || !ppSpinLock)
    {
        return STATUS_INVALID_PARAMETER;
    }
    
    DbgPrint("[NDIS] NdisFRegisterFilterDriver = 0x%p\n", pNdisFunc);

    // ==================== 【精准定位】链表头：ndisFilterDriverList ====================
    // 对应指令：mov rdx, qword ptr [rip+offset]
    // 机器码：48 8B 15 ?? ?? ?? ?? （固定7字节）
    UCHAR sig_ListHead[] = { 0xe8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x15 };
    UCHAR mask_ListHead[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01 };

    pMatchList = SearchSpecialCodeWithMask(
        (PUCHAR)pNdisFunc + 0x500,
        0x500,
        sig_ListHead,
        mask_ListHead,
        sizeof(sig_ListHead)
    );

    if (!pMatchList)
    {
        DbgPrint("[NDIS] 错误：未找到链表头指令\n");
        return STATUS_NOT_FOUND;
    }

    // RIP 相对寻址计算（公式：指令地址 + 7 + 4字节偏移）
    LONG listOffset = *(PLONG)((PUCHAR)pMatchList + 3 + 5);
    *ppListHead = (PVOID)((PUCHAR)pMatchList + 7 + 5 + listOffset);

    // ==================== 【精准定位】自旋锁：ndisFilterDriverListLock ====================
    // 对应指令：lea rcx, [rip+offset]
    // 机器码：48 8D 0D ?? ?? ?? ?? （固定7字节）
    UCHAR sig_SpinLock[] = { 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00 };
    UCHAR mask_SpinLock[] = { 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };

    pMatchLock = SearchSpecialCodeWithMask(
        (PUCHAR)pNdisFunc + 0x500,
        0x500,
        sig_SpinLock,
        mask_SpinLock,
        sizeof(sig_SpinLock)
    );

    if (!pMatchLock)
    {
        DbgPrint("[NDIS] 错误：未找到自旋锁指令\n");
        return STATUS_NOT_FOUND;
    }

    // RIP 相对寻址计算（公式完全一致）
    LONG lockOffset = *(PLONG)((PUCHAR)pMatchLock + 3);
    *ppSpinLock = (PVOID)((PUCHAR)pMatchLock + 7 + lockOffset);

    // ==================== 输出结果（和你WinDbg显示完全一致） ====================
    DbgPrint("\n[NDIS] 定位成功！\n");
    DbgPrint("[NDIS] 自旋锁指令地址: 0x%p\n", pMatchLock);
    DbgPrint("[NDIS] 自旋锁(ndisFilterDriverListLock): 0x%p\n", *ppSpinLock);
    DbgPrint("[NDIS] 链表头指令地址: 0x%p\n", pMatchList);
    DbgPrint("[NDIS] 链表头(ndisFilterDriverList): 0x%p\n", *ppListHead);

    return STATUS_SUCCESS;
}

// ===================== 3. 核心枚举函数 =====================
NTSTATUS
EnumNdisFilterDrivers(
    OUT PNDIS_FILTER_INFO* ppFilterArray,
    OUT PULONG              pulFilterCount
)
{
    NTSTATUS    status = STATUS_SUCCESS;
    PVOID       pListHead = NULL;
    PVOID       pSpinLock = NULL;
    PKSPIN_LOCK pListSpinLock = NULL;
    PNDIS_FILTER_BLOCK pCurBlock = NULL;
    PNDIS_FILTER_INFO pArray = NULL;
    ULONG       count = 0;
    KIRQL       oldIrql = PASSIVE_LEVEL;

    if (!ppFilterArray || !pulFilterCount)
        return STATUS_INVALID_PARAMETER;

    *ppFilterArray = NULL;
    *pulFilterCount = 0;

    // 获取链表+锁
    status = GetNdisFilterGlobalData(&pListHead, &pSpinLock);
    if (!NT_SUCCESS(status)) return status;

    // 加锁
    pListSpinLock = (PKSPIN_LOCK)pSpinLock;
    KeAcquireSpinLock(pListSpinLock, &oldIrql);

    // 第一次遍历：统计有效节点
    pCurBlock = *(PNDIS_FILTER_BLOCK*)pListHead;
    while (pCurBlock)
    {
        // 仅统计：签名正确 + 字符串指针合法
        if (pCurBlock->Signature == 0x104 &&
            (PVOID)pCurBlock->FilterName.Buffer > (PVOID)0x1000) // 过滤非法指针
        {
            count++;
        }
        pCurBlock = (PNDIS_FILTER_BLOCK)pCurBlock->Next;
    }

    if (count == 0)
    {
        status = STATUS_NOT_FOUND;
        goto Exit;
    }

    // 分配内存
    pArray = (PNDIS_FILTER_INFO)ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        count * sizeof(NDIS_FILTER_INFO),
        'fDIs'
    );
    if (!pArray)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }
    RtlZeroMemory(pArray, count * sizeof(NDIS_FILTER_INFO));

    // 第二次遍历：赋值数据
    pCurBlock = *(PNDIS_FILTER_BLOCK*)pListHead;
    ULONG index = 0;
    while (pCurBlock && index < count)
    {
        if (pCurBlock->Signature == 0x104 && (PVOID)pCurBlock->FilterName.Buffer > (PVOID)0x1000)
        {
            // ✅ 初始化UNICODE_STRING（关键：防止内存错乱）
            RtlInitEmptyUnicodeString(&pArray[index].FilterName, NULL, 0);

            // ✅ 深拷贝字符串
            status = RtlDuplicateUnicodeString(
                RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                &pCurBlock->FilterName,
                &pArray[index].FilterName
            );
            
            // 拷贝GUID
            pArray[index].FilterGuid = pCurBlock->FilterGuid;
            pArray[index].pFilterBlock = pCurBlock;
            pArray[index].bValid = TRUE;
            index++;
        }
        pCurBlock = (PNDIS_FILTER_BLOCK)pCurBlock->Next;
    }

Exit:
    KeReleaseSpinLock(pListSpinLock, oldIrql);

    if (NT_SUCCESS(status))
    {
        *ppFilterArray = pArray;
        *pulFilterCount = count;
    }
    else if (pArray)
    {
        ExFreePool(pArray);
    }

    return status;
}

// ===================== 4. 内存释放 =====================
VOID FreeNdisFilterArray(PNDIS_FILTER_INFO pFilterArray) {
    if (pFilterArray) {
        ExFreePool(pFilterArray);
    }
}