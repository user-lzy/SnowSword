//#include "GUI.h"

/*__inline BOOLEAN CheckHotkeyValid(PUCHAR addr, UINT32 vk)
{
	if (MmIsAddressValid(addr)) {
		PHOT_KEY hk = (PHOT_KEY)addr;
		if ((hk->vk & 0x7f) == vk) {
			// Hotkey valid
			return TRUE;
		}
	}
	return FALSE;
}

NTSTATUS GetSectionRegion(PUCHAR base, CHAR* name, PUCHAR* start, ULONG* size)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PIMAGE_SECTION_HEADER section;
	PIMAGE_NT_HEADERS nt_hdr = PE_NT_HEADER(base);
	DWORD section_cnt = nt_hdr->FileHeader.NumberOfSections;
	if (!MmIsAddressValid(nt_hdr))
		return status;
	section = IMAGE_FIRST_SECTION(nt_hdr);
	for (DWORD i = 0; i < section_cnt; i++) {
		CHAR sn[IMAGE_SIZEOF_SHORT_NAME + 1];
		RtlCopyMemory(sn, section->Name, IMAGE_SIZEOF_SHORT_NAME);
		if (!_stricmp(sn, name)) {
			*start = base + section->VirtualAddress;
			*size = section->Misc.VirtualSize;
			status = STATUS_SUCCESS;
			break;
		}
		section++;
	}
	return status;
}

BOOLEAN SearchHotkeyTable(PUCHAR** htable)
{
	*htable = NULL;

	// catch the imagebase of win32k（Win7 win32k.sys，Win10 win32kfull.sys）
	PUCHAR win32k;
	ULONG win32ksize;
	UNICODE_STRING win32kbaseName = RTL_CONSTANT_STRING(L"win32kbase.sys");
	UNICODE_STRING win32kName = RTL_CONSTANT_STRING(L"win32k.sys");
	RTL_OSVERSIONINFOW info = { 0 };
	info.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
	RtlGetVersion(&info); 
	if (info.dwMajorVersion == 10) {
		win32k = (PUCHAR)GetModuleBase(win32kbaseName, &win32ksize);
	}
	else {
		win32k = (PUCHAR)GetModuleBase(win32kName, &win32ksize);
	}
	if (!win32k) return FALSE;
	
	DbgPrint("win32k:%p, win32ksize:%x\n", win32k, win32ksize);

	// catch the region of data segment (global HashTable)
	NTSTATUS status;
	PUCHAR start;
	ULONG size;
	status = GetSectionRegion(win32k, ".data", &start, &size);
	if (!NT_SUCCESS(status)) {
		return FALSE;
	}
	DbgPrint("win32k-data start:%p, size:%x\n", start, size);

	// now searching...
	PUCHAR* ptr = (PUCHAR*)start;
	for (int i = 0, j = 0; i < size / sizeof(ptr); i++) {
		if (j == 0x80) {
			DbgPrint("start:%p\n", &ptr[i]);
			// the first place
			i -= j;

			// validate the Hotkeys
			INT vks[] = { 5, 10 ,15, 20, 25, 30, 35, 40, 45 };
			for (int k = 0; k < sizeof(vks) / sizeof(vks[0]); k++) {
				if (!CheckHotkeyValid(ptr[i + vks[k]], vks[k])) {
					j = 0;
					break;
				}
			}
			// we catch it
			if (j != 0) {
				*htable = &ptr[i];
				//打印热键表地址
				DbgPrint("Hotkey Table:%p\n", &ptr[i]);
				break;
			}
			continue;
		}
		// kernel address filter
		if ((PVOID)ptr[i] > MmSystemRangeStart) {
			j++;
			continue;
		}
		j = 0;
	}

	return 1;
}

VOID DumpHotkeyNode(PHOT_KEY hk, PHOTKEY_ITEM items, ULONG* pos)
{
	UNREFERENCED_PARAMETER(hk);
	UNREFERENCED_PARAMETER(items);
	UNREFERENCED_PARAMETER(pos);
	/*if (MmIsAddressValid(hk->slist)) {
		DumpHotkeyNode(hk->slist, items, pos);
	}

	if (hk->id >= HOTKEY_PLACEHOLDER_ID && hk->id <= HOTKEY_PLACEHOLDER_ID + HOTKEY_MAX_VK) return;

	UCHAR* name = (UCHAR*)"";
	PETHREAD thread = hk->thdinfo->thread;
	PEPROCESS process = NULL;
	HANDLE pid = NULL;
	HANDLE tid = NULL;
	if (thread != NULL) {
		process = IoThreadToProcess(thread);
		pid = PsGetProcessId(process);
		tid = PsGetThreadId(thread);
		UCHAR* temp = PsGetProcessImageFileName(process);
		if (temp) name = temp;
	}

	HWND wnd = NULL;
	if (hk->wndinfo && MmIsAddressValid(hk->wndinfo))
		wnd = hk->wndinfo->wnd;
	DbgPrint("HK:0x%p NAME:%s PROCESS:%Iu THREAD:%Iu HWND:0x%p MOD:%d",
		hk, name, (ULONG_PTR)pid, (ULONG_PTR)tid, wnd, hk->modifiers1);

	items[*pos].hkobj = (ULONG64)hk;
	items[*pos].id = hk->id;
	items[*pos].tid = (UINT32)tid;
	items[*pos].pid = (UINT32)pid;
	items[*pos].mod1 = hk->modifiers1;
	items[*pos].mod2 = hk->modifiers2;
	items[*pos].vk = hk->vk;
	items[*pos].wnd = (UINT32)wnd;
	strncpy((char*)items[*pos].name, (char*)name, 63);

	*pos++;*/
/*}

VOID DumpHotkeyTable(PUCHAR* table, PHOTKEY_ITEM items, ULONG* pos)
{
	for (INT i = 0; i < 0x7f; i++)
	{
		PHOT_KEY hk = (PHOT_KEY)table[i];
		if (hk) {
			DumpHotkeyNode(hk, items, pos);
		}
	}
}

NTSTATUS GetHotkeyInfo()
{
	ULONG count = 0;
	PHOTKEY_ITEM items = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	//
	// Caller have to guarantee current Win32k Session
	//
	
	PEPROCESS csrss_proc = NULL;
	//HANDLE csrss_pid = ;// GetSessionProcessId();
	status = PsLookupProcessByProcessId(PsGetCurrentProcessId(), &csrss_proc);
	if (NT_SUCCESS(status)) {
		KAPC_STATE apc_state;
		KeStackAttachProcess(csrss_proc, &apc_state);
		status = STATUS_UNSUCCESSFUL;

		if (!HotkeyTable) SearchHotkeyTable(&HotkeyTable);

		DbgPrint("HotkeyTable:%p\n", HotkeyTable);

		if (HotkeyTable != NULL) {
			ULONG presize = sizeof(HOTKEY_ITEM) * 1024;
			items = (PHOTKEY_ITEM)ExAllocatePool2(POOL_FLAG_NON_PAGED, presize, 'phki');
            // 修改前：
            // DbgPrint("HotkeyTable:%llx\n", HotkeyTable);

            // 修改后：
            //DbgPrint("HotkeyTable:%p\n", HotkeyTable);
			if (items == NULL) return STATUS_INSUFFICIENT_RESOURCES;
			RtlZeroMemory(items, presize);
			if (items) {
				DumpHotkeyTable(HotkeyTable, items, &count);
				if (count) {
					//ULONG size = sizeof(HOTKEY_INFO) + (count - 1) * sizeof(HOTKEY_ITEM);
					//打印热键信息
					DbgPrint("Hotkey Count:%d\n", count);
					for (ULONG i = 0; i < count; i++) {
						DbgPrint("Hotkey %d: name:%s, pid:%d, tid:%d, hwnd:%x, mod1:%x, mod2:%x, vk:%x, id:%x, hkobj:%llx\n",
							i,
							items[i].name,
							items[i].pid,
							items[i].tid,
							items[i].wnd,
							items[i].mod1,
							items[i].mod2,
							items[i].vk,
							items[i].id,
							items[i].hkobj
						);
					}
					/*auto info = (PHOTKEY_INFO)outbuf;
					info->count = count;
					for (ULONG i = 0; i < count; i++) {
						info->items[i] = items[i];
					}
					irp->IoStatus.Information = size;
					status = STATUS_SUCCESS;*/
				/*}
				ExFreePoolWithTag(items, 'phki');
			}
		}
		KeUnstackDetachProcess(&apc_state);
		ObDereferenceObject(csrss_proc);
	}

	return status;
}*/