#include "Symbol.h"
#include "ioctl.h"
#include "Module.h"

// 全局上下文
PDEVICE_CONTEXT g_SymbolCtx = NULL;
// ✅ 新增：驱动卸载标志，中断InternalQuerySymbol等待
BOOLEAN g_DriverUnloading = FALSE;

VOID UninitSymbolContext()
{
    if (!g_SymbolCtx)
        return;

    g_SymbolCtx->Unloading = TRUE;

    ExAcquireFastMutex(&g_SymbolCtx->Lock);

    if (g_SymbolCtx->WaitIrp)
    {
        PIRP irp = g_SymbolCtx->WaitIrp;
        g_SymbolCtx->WaitIrp = NULL;

        irp->IoStatus.Status = STATUS_CANCELLED;
        irp->IoStatus.Information = 0;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }

    PLIST_ENTRY e = g_SymbolCtx->JobList.Flink;
    while (e != &g_SymbolCtx->JobList)
    {
        PSYMBOL_JOB job = CONTAINING_RECORD(e, SYMBOL_JOB, Link);
        e = e->Flink;

        job->Response.Status = STATUS_CANCELLED;
        KeSetEvent(&job->DoneEvent, IO_NO_INCREMENT, FALSE);
    }

    ExReleaseFastMutex(&g_SymbolCtx->Lock);

    ExFreePool(g_SymbolCtx);
    g_SymbolCtx = NULL;

    DbgPrint("SymbolQuery: 卸载完成\n");
}

static VOID SymbolCancelRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    // 【修复】取消例程只完成IRP，不释放ctx！杜绝竞态双重释放
    // ctx由InternalQuerySymbol统一释放，保证唯一释放
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS DispatchClose_Symbol(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    if (g_SymbolCtx)
    {
        g_SymbolCtx->Unloading = TRUE;

        ExAcquireFastMutex(&g_SymbolCtx->Lock);

        //
        // 1. 如果 R3 正在挂起等待请求，必须把它取消掉
        //
        if (g_SymbolCtx->WaitIrp)
        {
            PIRP waitIrp = g_SymbolCtx->WaitIrp;
            g_SymbolCtx->WaitIrp = NULL;

            ExReleaseFastMutex(&g_SymbolCtx->Lock);

            KIRQL oldIrql;
            IoAcquireCancelSpinLock(&oldIrql);

            if (IoSetCancelRoutine(waitIrp, NULL))
            {
                IoReleaseCancelSpinLock(oldIrql);

                waitIrp->IoStatus.Status = STATUS_CANCELLED;
                waitIrp->IoStatus.Information = 0;
                IoCompleteRequest(waitIrp, IO_NO_INCREMENT);
            }
            else
            {
                IoReleaseCancelSpinLock(oldIrql);
            }
        }
        else
        {
            ExReleaseFastMutex(&g_SymbolCtx->Lock);
        }
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchIoControl_Symbol(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG info = 0;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioctl = irpSp->Parameters.DeviceIoControl.IoControlCode;

    switch (ioctl)
    {
    case IOCTL_WaitRequest:
    {
        if (!g_SymbolCtx)
        {
            Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_INVALID_DEVICE_STATE;
        }

        IoMarkIrpPending(Irp);

        ExAcquireFastMutex(&g_SymbolCtx->Lock);

        if (g_SymbolCtx->Unloading)
        {
            ExReleaseFastMutex(&g_SymbolCtx->Lock);

            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_CANCELLED;
        }

        //
        // ✅【关键修复】优先检查是否已有未派发的 job
        //
        PSYMBOL_JOB job = NULL;
        PLIST_ENTRY e = g_SymbolCtx->JobList.Flink;

        while (e != &g_SymbolCtx->JobList)
        {
            PSYMBOL_JOB cur = CONTAINING_RECORD(e, SYMBOL_JOB, Link);
            if (!cur->Delivered)
            {
                job = cur;
                break;
            }
            e = e->Flink;
        }

        if (job)
        {
            job->Delivered = TRUE;

            ExReleaseFastMutex(&g_SymbolCtx->Lock);

            SYMBOL_QUERY_PACKET* pkt =
                (SYMBOL_QUERY_PACKET*)Irp->AssociatedIrp.SystemBuffer;

            if (!pkt)
            {
                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_INVALID_PARAMETER;
            }

            pkt->RequestId = job->RequestId;
            RtlCopyMemory(&pkt->Request, &job->Request, sizeof(SYMBOL_QUERY_REQUEST));

            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = sizeof(SYMBOL_QUERY_PACKET);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            return STATUS_SUCCESS;
        }

        //
        // 没有 job，才挂起
        //
        if (g_SymbolCtx->WaitIrp != NULL)
        {
            ExReleaseFastMutex(&g_SymbolCtx->Lock);

            Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_DEVICE_BUSY;
        }

        g_SymbolCtx->WaitIrp = Irp;

        KIRQL oldIrql;
        IoAcquireCancelSpinLock(&oldIrql);

        if (Irp->Cancel)
        {
            g_SymbolCtx->WaitIrp = NULL;

            IoReleaseCancelSpinLock(oldIrql);
            ExReleaseFastMutex(&g_SymbolCtx->Lock);

            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return STATUS_CANCELLED;
        }

        IoSetCancelRoutine(Irp, SymbolCancelRoutine);

        IoReleaseCancelSpinLock(oldIrql);
        ExReleaseFastMutex(&g_SymbolCtx->Lock);

        return STATUS_PENDING;
    }

case IOCTL_SubmitResponse:
{
    if (!g_SymbolCtx ||
        irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SYMBOL_RESPONSE_PACKET))
    {
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_BUFFER_TOO_SMALL;
    }

    PSYMBOL_RESPONSE_PACKET pkt =
        (PSYMBOL_RESPONSE_PACKET)Irp->AssociatedIrp.SystemBuffer;

    ExAcquireFastMutex(&g_SymbolCtx->Lock);

    PSYMBOL_JOB job = NULL;
    PLIST_ENTRY e = g_SymbolCtx->JobList.Flink;

    while (e != &g_SymbolCtx->JobList)
    {
        PSYMBOL_JOB cur = CONTAINING_RECORD(e, SYMBOL_JOB, Link);
        if (cur->RequestId == pkt->RequestId)
        {
            job = cur;
            break;
        }
        e = e->Flink;
    }

    if (!job)
    {
        ExReleaseFastMutex(&g_SymbolCtx->Lock);

        Irp->IoStatus.Status = STATUS_NOT_FOUND;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_NOT_FOUND;
    }

    //
    // ✅【关键修复】锁内完成所有状态更新，避免UAF
    //
    if (!job->Completed)
    {
        RtlCopyMemory(&job->Response, &pkt->Response, sizeof(SYMBOL_QUERY_RESPONSE));
        job->Completed = TRUE;
        KeSetEvent(&job->DoneEvent, IO_NO_INCREMENT, FALSE);
    }

    ExReleaseFastMutex(&g_SymbolCtx->Lock);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS InternalQuerySymbol(
    PSYMBOL_QUERY_REQUEST Request,
    PSYMBOL_QUERY_RESPONSE Response)
{
    if (!g_SymbolCtx || !Request || !Response)
        return STATUS_INVALID_PARAMETER;

    PSYMBOL_JOB job = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(SYMBOL_JOB),
        'jbS');

    if (!job)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(job, sizeof(SYMBOL_JOB));
    KeInitializeEvent(&job->DoneEvent, SynchronizationEvent, FALSE);

    job->RequestId = InterlockedIncrement64((volatile LONG64*)&g_SymbolCtx->NextRequestId);
    RtlCopyMemory(&job->Request, Request, sizeof(SYMBOL_QUERY_REQUEST));

    ExAcquireFastMutex(&g_SymbolCtx->Lock);

    InsertTailList(&g_SymbolCtx->JobList, &job->Link);

    //
    // 如果有等待IRP，立即派发
    //
    if (g_SymbolCtx->WaitIrp)
    {
        PIRP waitIrp = g_SymbolCtx->WaitIrp;
        g_SymbolCtx->WaitIrp = NULL;

        job->Delivered = TRUE;

        ExReleaseFastMutex(&g_SymbolCtx->Lock);

        SYMBOL_QUERY_PACKET* pkt =
            (SYMBOL_QUERY_PACKET*)waitIrp->AssociatedIrp.SystemBuffer;

        if (pkt)
        {
            pkt->RequestId = job->RequestId;
            RtlCopyMemory(&pkt->Request, &job->Request, sizeof(SYMBOL_QUERY_REQUEST));

            waitIrp->IoStatus.Status = STATUS_SUCCESS;
            waitIrp->IoStatus.Information = sizeof(SYMBOL_QUERY_PACKET);
            IoCompleteRequest(waitIrp, IO_NO_INCREMENT);
        }
    }
    else
    {
        ExReleaseFastMutex(&g_SymbolCtx->Lock);
    }

    //
    // 等待完成
    //
    LARGE_INTEGER timeout;
    timeout.QuadPart = -10 * 1000 * 1000;

    NTSTATUS waitStatus = KeWaitForSingleObject(
        &job->DoneEvent,
        Executive,
        KernelMode,
        FALSE,
        &timeout
    );

    NTSTATUS finalStatus;

    ExAcquireFastMutex(&g_SymbolCtx->Lock);

    if (waitStatus == STATUS_TIMEOUT || !job->Completed)
    {
        //
        // ✅【关键修复】必须保证超时不会误用未填充数据
        //
        RemoveEntryList(&job->Link);
        finalStatus = STATUS_TIMEOUT;
    }
    else
    {
        RtlCopyMemory(Response, &job->Response, sizeof(SYMBOL_QUERY_RESPONSE));
        RemoveEntryList(&job->Link);
        finalStatus = job->Response.Status;
    }

    ExReleaseFastMutex(&g_SymbolCtx->Lock);

    ExFreePool(job);

    return finalStatus;
}

//VOID GetBaseNameFromPath(PCWSTR FullPath, PWCHAR BaseName, ULONG MaxLen)
//{
//    if (!FullPath || !BaseName || !MaxLen) return;
//
//    ULONG len = (ULONG)wcslen(FullPath);
//    ULONG i = len;
//
//    while (i > 0)
//    {
//        if (FullPath[i - 1] == L'\\' || FullPath[i - 1] == L'/')
//            break;
//        i--;
//    }
//
//    wcsncpy_s(BaseName, MaxLen, &FullPath[i], _TRUNCATE);
//}

// ==============================
// 导出 API
// ==============================
NTSTATUS KernelQueryAddressToSymbol(PCWSTR ModuleName, ULONG64 Address, PWCHAR SymbolName, ULONG NameLength)
{
    if (!ModuleName || !SymbolName || !NameLength || !g_SymbolCtx)
        return STATUS_INVALID_PARAMETER;

    SYMBOL_QUERY_REQUEST req = { 0 };
    SYMBOL_QUERY_RESPONSE resp = { 0 };

    req.Type = QueryAddressToSymbol;
    wcscpy_s(req.ModuleName, 256, ModuleName);

    UNICODE_STRING uModuleName = { 0 };
    RtlInitUnicodeString(&uModuleName, ModuleName);

    ULONG moduleSize = 0;
    PVOID ModuleBase = GetModuleBase(uModuleName, &moduleSize);

    DbgPrint("KernelQueryAddressToSymbol: ModuleBase=0x%p, ModuleSize=0x%lx\n", ModuleBase, moduleSize);

    if (!ModuleBase || moduleSize == 0)
        return STATUS_NOT_FOUND;

    ULONG64 base = (ULONG64)ModuleBase;
    if (Address < base || Address >= base + moduleSize)
    {
        DbgPrint("KernelQueryAddressToSymbol: 地址不在模块范围内\n");
        return STATUS_NOT_FOUND;
    }

    req.Data.AddrToSymbol.Address = Address - base;

    NTSTATUS status = InternalQuerySymbol(&req, &resp);
    if (NT_SUCCESS(status))
    {
        wcscpy_s(SymbolName, NameLength, resp.Result.SymbolName);
    }
    return status;
}

NTSTATUS KernelQuerySymbolAddress(PCWSTR ModuleName, PCWSTR SymbolName, PULONG64 Address)
{
    if (!ModuleName || !SymbolName || !Address || !g_SymbolCtx)
        return STATUS_INVALID_PARAMETER;

    SYMBOL_QUERY_REQUEST req = { 0 };
    SYMBOL_QUERY_RESPONSE resp = { 0 };

    req.Type = QuerySymbolToAddress;
    wcscpy_s(req.ModuleName, 256, ModuleName);
    wcscpy_s(req.Data.SymbolToAddr.SymbolName, 256, SymbolName);

    NTSTATUS status = InternalQuerySymbol(&req, &resp);
    if (NT_SUCCESS(status)) {
        UNICODE_STRING uModuleName = { 0 };
        RtlInitUnicodeString(&uModuleName, ModuleName);
        PVOID ModuleBase = GetModuleBase(uModuleName, NULL);
        DbgPrint("KernelQuerySymbolAddress: ModuleBase=0x%p, SymbolOffset=0x%llx\n", ModuleBase, resp.Result.Address);
        *Address = (ULONG64)ModuleBase + resp.Result.Address;
    }
    return status;
}

NTSTATUS GetNtSymbolAddress(PCWSTR SymbolName, PULONG64 Address)
{
    return KernelQuerySymbolAddress(L"ntoskrnl.exe", SymbolName, Address);
}

NTSTATUS KernelQueryStructOffset(PCWSTR ModuleName, PCWSTR StructName, PCWSTR MemberName, PLONG Offset)
{
    if (!ModuleName || !StructName || !MemberName || !Offset || !g_SymbolCtx)
        return STATUS_INVALID_PARAMETER;

    SYMBOL_QUERY_REQUEST req = { 0 };
    SYMBOL_QUERY_RESPONSE resp = { 0 };

    req.Type = QueryStructOffset;
    wcscpy_s(req.ModuleName, 256, ModuleName);
    wcscpy_s(req.Data.StructOffset.StructName, 64, StructName);
    wcscpy_s(req.Data.StructOffset.MemberName, 64, MemberName);

    NTSTATUS status = InternalQuerySymbol(&req, &resp);
    if (NT_SUCCESS(status))
        *Offset = resp.Result.Offset;

    return status;
}

// ==============================
// 初始化
// ==============================
NTSTATUS InitSymbolContext(PDEVICE_OBJECT deviceObj)
{
    g_SymbolCtx = ExAllocatePool2(
        POOL_FLAG_NON_PAGED,
        sizeof(DEVICE_CONTEXT),
        'SmbS'
    );

    if (!g_SymbolCtx)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(g_SymbolCtx, sizeof(DEVICE_CONTEXT));
    g_SymbolCtx->DeviceObject = deviceObj;

    ExInitializeFastMutex(&g_SymbolCtx->Lock);
    InitializeListHead(&g_SymbolCtx->JobList);
    g_SymbolCtx->WaitIrp = NULL;
    g_SymbolCtx->NextRequestId = 1;
    g_SymbolCtx->Unloading = FALSE;

    DbgPrint("SymbolQuery: 初始化成功\n");
    return STATUS_SUCCESS;
}