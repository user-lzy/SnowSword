#pragma once

#include "DriverControl.h"
#include "global.h"

// 空的 DriverUnload 函数
VOID EmptyDriverUnload(PDRIVER_OBJECT DriverObject)
{
	// 这是一个空函数，不执行任何操作
	// 这样可以绕过驱动原有的清理逻辑
	UNREFERENCED_PARAMETER(DriverObject);
}

VOID UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
	//如果驱动对象不为空
	if (!pDriverObject) return;
	DbgPrint("DriverObject:0x%p", pDriverObject);
	//调用其DriverUnload
	if (pDriverObject->DriverUnload) {
		pDriverObject->DriverUnload(pDriverObject);
		ObReferenceObject(pDriverObject);
		ObMakeTemporaryObject(pDriverObject);
		ObDereferenceObject(pDriverObject);
	}
	else {
		pDriverObject->DriverUnload = EmptyDriverUnload;
		//ZwUn
	}
}