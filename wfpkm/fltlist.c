#include "fltlist.h"



LIST_ENTRY head;
KSPIN_LOCK lock;



VOID InitFilterList()
{
	InitializeListHead(&head);
	KeInitializeSpinLock(&lock);
}



NTSTATUS AppendFilterItem(
	_In_ PFILTER_ITEM item
)
{
	NTSTATUS status = STATUS_SUCCESS;

	PITEM list_item = ExAllocatePoolWithTag(NonPagedPool, sizeof(ITEM), WFPKM_ITEM_POOL_TAG);
	if (list_item) {
		list_item->data.filter_id       = item->filter_id;
		list_item->data.fwpm_callout_id = item->fwpm_callout_id;
		list_item->data.fwps_callout_id = item->fwps_callout_id;

		ExInterlockedInsertTailList(&head, &list_item->link, &lock);

		status = STATUS_SUCCESS;
	}
	else {
		status = STATUS_NO_MEMORY;
	}

	return status;
}



NTSTATUS TakeFilterItem(
	_Out_ PFILTER_ITEM item
)
{
	NTSTATUS status = STATUS_SUCCESS;

	KIRQL irql;
	KeAcquireSpinLock(&lock, &irql);
	{
		if (IsListEmpty(&head) || !head.Flink) {
			status = STATUS_NO_DATA_DETECTED;
		}
		else {
			PITEM first_item = CONTAINING_RECORD(head.Flink, ITEM, link);
			*item = first_item->data;
		}
	}
	KeReleaseSpinLock(&lock, irql);

	return status;
}



NTSTATUS RemoveFilterItem(
	_In_ PFILTER_ITEM item
)
{
	NTSTATUS status = STATUS_NOT_FOUND;

	KIRQL irql;
	KeAcquireSpinLock(&lock, &irql);
	{
		PLIST_ENTRY current_link = head.Flink;
		while (current_link != &head && current_link) {

			PITEM current_item = CONTAINING_RECORD(current_link, ITEM, link);

			if (current_item->data.filter_id       == item->filter_id
			||	current_item->data.fwpm_callout_id == item->fwpm_callout_id
			||	current_item->data.fwps_callout_id == item->fwps_callout_id
			)
			{
				RemoveEntryList(current_link);
				ExFreePoolWithTag(current_item, WFPKM_ITEM_POOL_TAG);
				status = STATUS_SUCCESS;
				break;
			}

			current_link = current_item->link.Flink;
		}
	}
	KeReleaseSpinLock(&lock, irql);

	return status;
}



VOID ClearFilterList()
{
	KIRQL irql;
	KeAcquireSpinLock(&lock, &irql);
	{
		PLIST_ENTRY current_link = head.Flink, next_link = NULL;
		while (current_link != &head && current_link) {

			PITEM current_item = CONTAINING_RECORD(current_link, ITEM, link);
			
			next_link = current_link->Flink;
			RemoveEntryList(current_link);
			current_link = next_link;

			ExFreePoolWithTag(current_item, WFPKM_ITEM_POOL_TAG);
		}
	}
	KeReleaseSpinLock(&lock, irql);
}



VOID FinFilterList()
{
	if (!IsListEmpty(&head)) {
		ClearFilterList();
	}

	RtlZeroMemory(&head, sizeof(head));
	RtlZeroMemory(&lock, sizeof(lock));
}