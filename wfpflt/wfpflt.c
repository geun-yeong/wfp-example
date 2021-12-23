#include "wfpflt.h"
#include "../wfpkm/fltlist.h"
#include "../wfpkm/error.h"



static HANDLE kmfe_handle; // kernel mode filter engine handle
static PDEVICE_OBJECT wfpkm_device;



NTSTATUS InitWfp(
	_In_ PDEVICE_OBJECT device_object
)
{
	if (!device_object) return STATUS_INVALID_PARAMETER;

	NTSTATUS status = STATUS_SUCCESS;

	FWPM_PROVIDER wfpkm_provider = { 0, };



	//
	// open a kernel mode filter engine
	//
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, NULL, &kmfe_handle);
	IF_ERROR(FwpmEngineOpen, EXIT_OF_INIT_WFP);



	//
	// add a provider of this module to kmfe
	//
	status = FwpmTransactionBegin(kmfe_handle, 0);
	IF_ERROR(FwpmTransactionBegin, CLEANUP_OF_INIT_WFP);

	wfpkm_provider.serviceName = (wchar_t*)L"wfpkm";
	wfpkm_provider.displayData.name = (wchar_t*)L"wfpkm_example_provider";
	wfpkm_provider.displayData.description = (wchar_t*)L"The provider object for wfp-example";
	wfpkm_provider.providerKey = WFPKM_PROVIDER_KEY;

	status = FwpmProviderAdd(kmfe_handle, &wfpkm_provider, NULL);
	IF_ERROR(FwpmProviderAdd, CLEANUP_OF_INIT_WFP);
	KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmProviderAdd success"));



	//
	// create a list to store callout and filter id
	//
	InitFilterList();



	//
	// done
	//
	status = FwpmTransactionCommit(kmfe_handle);
	wfpkm_device = device_object;

CLEANUP_OF_INIT_WFP:

	if (!NT_SUCCESS(status)) FwpmTransactionAbort(kmfe_handle);

EXIT_OF_INIT_WFP:

	return status;
}



NTSTATUS FinWfp()
{
	if (!kmfe_handle) return STATUS_APP_INIT_FAILURE;

	NTSTATUS status = STATUS_SUCCESS;
	FILTER_ITEM item = { 0, };



	//
	// get callout and filter id from list and delete/unregister them
	//
	status = FwpmTransactionBegin(kmfe_handle, 0);
	IF_ERROR(FwpmTransactionBegin, EXIT_OF_FIN_WFP);

	while (NT_SUCCESS(TakeFilterItem(&item))) {
		KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmFilterDeleteById(%llu)", item.filter_id));
		status = FwpmFilterDeleteById(kmfe_handle, item.filter_id);
		IF_ERROR(FwpmFilterDeleteById, CLEANUP_OF_FIN_WFP);

		KdPrint(("[wfpkm] " __FUNCTION__ " - RemoveFilterItem"));
		status = RemoveFilterItem(&item);
		IF_ERROR(RemoveFilterItem, CLEANUP_OF_FIN_WFP);
	}

	//
	// done 
	//
	status = FwpmTransactionCommit(kmfe_handle);

	FwpmProviderDeleteByKey(kmfe_handle, &WFPKM_PROVIDER_KEY);
	FwpmEngineClose(kmfe_handle);

CLEANUP_OF_FIN_WFP:

	if (!NT_SUCCESS(status)) FwpmTransactionAbort(kmfe_handle);

EXIT_OF_FIN_WFP:

	return status;
}



NTSTATUS AddFilterIpv4(
	_In_ const GUID* layer_key,
	_In_ FWP_V4_ADDR_AND_MASK* ipv4
)
{
	if (!kmfe_handle) return STATUS_APP_INIT_FAILURE;

	NTSTATUS status = STATUS_SUCCESS;

	FWPM_FILTER_CONDITION fwpm_condition = { 0, };
	FWPM_FILTER  fwpm_filter = { 0, };
	UINT64 fwpm_filter_id = 0;

	FILTER_ITEM wfpkm_filter_item = { 0, };



	//
	// add a filter object that block network events has ipv4 as remote address
	//
	status = FwpmTransactionBegin(kmfe_handle, 0);
	IF_ERROR(FwpmTransactionBegin, EXIT_OF_ADD_FILTER_IPV4);

	fwpm_condition.fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
	fwpm_condition.matchType = FWP_MATCH_EQUAL;
	fwpm_condition.conditionValue.type = FWP_V4_ADDR_MASK;
	fwpm_condition.conditionValue.v4AddrMask = ipv4;

	// fwpm filter key was automatically created by FwpmFilterAdd
	fwpm_filter.displayData.name = (wchar_t*)L"wfpflt_example_filter";
	fwpm_filter.displayData.description = (wchar_t*)L"The filter object for wfp-filter-example";
	fwpm_filter.layerKey = *layer_key;
	fwpm_filter.weight.type = FWP_EMPTY;
	fwpm_filter.action.type = FWP_ACTION_BLOCK;
	fwpm_filter.filterCondition = &fwpm_condition;
	fwpm_filter.numFilterConditions = 1;

	status = FwpmFilterAdd(kmfe_handle, &fwpm_filter, NULL, &fwpm_filter_id);
	IF_ERROR(FwpmFilterAdd, CLEANUP_OF_ADD_FILTER_IPV4);
	KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmFilterAdd success (filter id = %llu)", fwpm_filter_id));



	//
	// store callout and filter id to list
	//
	wfpkm_filter_item.filter_id = fwpm_filter_id;

	status = AppendFilterItem(&wfpkm_filter_item);
	IF_ERROR(AppendFilterItem, CLEANUP_OF_ADD_FILTER_IPV4);
	KdPrint(("[wfpkm] " __FUNCTION__ " - AppendFilterItem success"));



	//
	// done 
	//
	status = FwpmTransactionCommit(kmfe_handle);

CLEANUP_OF_ADD_FILTER_IPV4:

	if (!NT_SUCCESS(status)) FwpmTransactionAbort(kmfe_handle);

EXIT_OF_ADD_FILTER_IPV4:

	return status;
}