#pragma once

#include <ntddk.h>

#define WFPKM_ITEM_POOL_TAG '10KW'



typedef struct _FILTER_ITEM {

	UINT32 fwps_callout_id;
	UINT32 fwpm_callout_id;
	UINT64 filter_id;

} FILTER_ITEM, *PFILTER_ITEM;

typedef struct _ITEM {

	LIST_ENTRY link;
	FILTER_ITEM data;

} ITEM, *PITEM;



VOID InitFilterList();

NTSTATUS AppendFilterItem(
	_In_ PFILTER_ITEM item
);

NTSTATUS TakeFilterItem(
	_Out_ PFILTER_ITEM item
);

NTSTATUS RemoveFilterItem(
	_In_ PFILTER_ITEM item
);

VOID ClearFilterList();

VOID FinFilterList();