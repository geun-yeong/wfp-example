#include "wfpkm.h"
#include "converter.h"
#include "fltlist.h"
#include "error.h"



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

	wfpkm_provider.serviceName             = (wchar_t*)L"wfpkm";
	wfpkm_provider.displayData.name        = (wchar_t*)L"wfpkm_example_provider";
	wfpkm_provider.displayData.description = (wchar_t*)L"The provider object for wfp-example";
	wfpkm_provider.providerKey             = WFPKM_PROVIDER_KEY;

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

		KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmCalloutDeleteById(%u)", item.fwpm_callout_id));
		status = FwpmCalloutDeleteById(kmfe_handle, item.fwpm_callout_id);
		IF_ERROR(FwpmCalloutDeleteById, CLEANUP_OF_FIN_WFP);

		KdPrint(("[wfpkm] " __FUNCTION__ " - FwpsCalloutUnregisterById(%u)", item.fwps_callout_id));
		status = FwpsCalloutUnregisterById(item.fwps_callout_id);
		IF_ERROR(FwpsCalloutUnregisterById, CLEANUP_OF_FIN_WFP);

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



NTSTATUS AddCalloutToLayer(
	_In_ const GUID* layer_key
)
{
	if (!kmfe_handle) return STATUS_APP_INIT_FAILURE;

	NTSTATUS status = STATUS_SUCCESS;

	FWPS_CALLOUT fwps_callout = { 0, };
	FWPM_CALLOUT fwpm_callout = { 0, };
	FWPM_FILTER  fwpm_filter  = { 0, };

	UINT32 fwps_callout_id = 0;
	UINT32 fwpm_callout_id = 0;
	UINT64 fwpm_filter_id  = 0;

	FILTER_ITEM wfpkm_filter_item = { 0, };



	//
	// create and register a callout object, add a filter object that has callout
	//
	status = FwpmTransactionBegin(kmfe_handle, 0);
	IF_ERROR(FwpmTransactionBegin, EXIT_OF_INIT_WFP);



	// register a callout object to system
	fwps_callout.classifyFn   = ClassifyFunctionRoutine;
	fwps_callout.notifyFn     = NotifyFunctionRoutine;
	fwps_callout.flowDeleteFn = FlowDeleteFunctionRoutine;
	do { status = ExUuidCreate(&fwps_callout.calloutKey); } while (status == STATUS_RETRY);

	status = FwpsCalloutRegister(wfpkm_device, &fwps_callout, &fwps_callout_id);
	IF_ERROR(FwpsCalloutRegister, CLEANUP_OF_ADD_CALLOUT_TO_LAYER);
	KdPrint(("[wfpkm] " __FUNCTION__ " - FwpsCalloutRegister success (callout id = %u)", fwps_callout_id));



	// add a callout object to filter engine
	fwpm_callout.calloutKey              = fwps_callout.calloutKey;
	fwpm_callout.displayData.name        = (wchar_t*)L"wfpkm_example_callout";
	fwpm_callout.displayData.description = (wchar_t*)L"The callout object for wfp-example";
	fwpm_callout.providerKey             = (GUID*)&WFPKM_PROVIDER_KEY;
	fwpm_callout.applicableLayer         = *layer_key;

	status = FwpmCalloutAdd(kmfe_handle, &fwpm_callout, NULL, &fwpm_callout_id);
	IF_ERROR(FwpmCalloutAdd, CLEANUP_OF_ADD_CALLOUT_TO_LAYER);
	KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmCalloutAdd success (callout id = %u)", fwpm_callout_id));
	


	// fwpm filter key was automatically created by FwpmFilterAdd
	fwpm_filter.displayData.name        = (wchar_t*)L"wfpkm_example_filter";
	fwpm_filter.displayData.description = (wchar_t*)L"The filter object for wfp-example";
	fwpm_filter.layerKey                = *layer_key;
	fwpm_filter.action.type             = FWP_ACTION_CALLOUT_UNKNOWN;
	fwpm_filter.action.calloutKey       = fwps_callout.calloutKey;

	status = FwpmFilterAdd(kmfe_handle, &fwpm_filter, NULL, &fwpm_filter_id);
	IF_ERROR(FwpmFilterAdd, CLEANUP_OF_ADD_CALLOUT_TO_LAYER);
	KdPrint(("[wfpkm] " __FUNCTION__ " - FwpmFilterAdd success (filter id = %llu)", fwpm_filter_id));



	//
	// store callout and filter id to list
	//
	wfpkm_filter_item.filter_id       = fwpm_filter_id;
	wfpkm_filter_item.fwpm_callout_id = fwpm_callout_id;
	wfpkm_filter_item.fwps_callout_id = fwps_callout_id;

	status = AppendFilterItem(&wfpkm_filter_item);
	IF_ERROR(AppendFilterItem, CLEANUP_OF_ADD_CALLOUT_TO_LAYER);
	KdPrint(("[wfpkm] " __FUNCTION__ " - AppendFilterItem success"));



	//
	// done 
	//
	status = FwpmTransactionCommit(kmfe_handle);

CLEANUP_OF_ADD_CALLOUT_TO_LAYER:

	if (!NT_SUCCESS(status)) FwpmTransactionAbort(kmfe_handle);

EXIT_OF_INIT_WFP:

	return status;
}



void NTAPI ClassifyFunctionRoutine(
	_In_        const FWPS_INCOMING_VALUES0* fixed_values,
	_In_        const FWPS_INCOMING_METADATA_VALUES0* meta_values,
	_Inout_opt_ void* layer_data,
	_In_opt_    const void* classify_context,
	_In_        const FWPS_FILTER3* filter,
	_In_        UINT64 flow_context,
	_Inout_     FWPS_CLASSIFY_OUT0* classify_out
)
{
	UNREFERENCED_PARAMETER(meta_values);
	UNREFERENCED_PARAMETER(layer_data);
	UNREFERENCED_PARAMETER(classify_context);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flow_context);
	UNREFERENCED_PARAMETER(classify_out);

	UINT32 pid = (meta_values->processId == 0) ? PtrToUint(PsGetCurrentProcessId()) : (UINT32)meta_values->processId;
	CHAR _local_ipstr[64], *local_ipstr = NULL;
	CHAR _remote_ipstr[64], *remote_ipstr = NULL;
	UINT16 local_port = 0, remote_port = 0;
	PCSTR direction = NULL;



	switch (fixed_values->layerId) {

		//
		// inbound ipv4 
		//
	case FWPS_LAYER_INBOUND_TRANSPORT_V4:
		local_ipstr = ConvertIpv4ToString(
			fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32,
			_local_ipstr,
			64
		);
		remote_ipstr = ConvertIpv4ToString(
			fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32,
			_remote_ipstr,
			64
		);
		local_port = fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
		remote_port = fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16;
		direction = "<-";
		
		break;



		//
		// inbound ipv6
		//
	case FWPS_LAYER_INBOUND_TRANSPORT_V6:
		local_ipstr = ConvertIpv6ToString(
			fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS].value.byteArray16->byteArray16,
			_local_ipstr,
			64
		);
		remote_ipstr = ConvertIpv6ToString(
			fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS].value.byteArray16->byteArray16,
			_remote_ipstr,
			64
		);
		local_port = fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_LOCAL_PORT].value.uint16;
		remote_port = fixed_values->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V6_IP_REMOTE_PORT].value.uint16;
		direction = "<-";

		break;



		//
		// outbound ipv4
		//
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
		local_ipstr = ConvertIpv4ToString(
			fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_ADDRESS].value.uint32,
			_local_ipstr,
			64
		);
		remote_ipstr = ConvertIpv4ToString(
			fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_ADDRESS].value.uint32,
			_remote_ipstr,
			64
		);
		local_port = fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
		remote_port = fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_REMOTE_PORT].value.uint16;
		direction = "->";

		break;



		//
		// outbound ipv6
		//
	case FWPS_LAYER_OUTBOUND_TRANSPORT_V6:
		local_ipstr = ConvertIpv6ToString(
			fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_ADDRESS].value.byteArray16->byteArray16,
			_local_ipstr,
			64
		);
		remote_ipstr = ConvertIpv6ToString(
			fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_ADDRESS].value.byteArray16->byteArray16,
			_remote_ipstr,
			64
		);
		local_port = fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_LOCAL_PORT].value.uint16;
		remote_port = fixed_values->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V6_IP_REMOTE_PORT].value.uint16;
		direction = "->";

		break;



		// 
		// else
		//
	default:
		break;
	}



	//
	// done
	//
	KdPrint((
		"[wfpkm] " __FUNCTION__ " [%-5u] %s:%u %s %s:%u",
		pid,
		local_ipstr, local_port,
		direction,
		remote_ipstr, remote_port
	));
}



NTSTATUS NTAPI NotifyFunctionRoutine(
	_In_    FWPS_CALLOUT_NOTIFY_TYPE notify_type,
	_In_    const GUID* filter_key,
	_Inout_ FWPS_FILTER3* filter
)
{
	UNREFERENCED_PARAMETER(filter_key);

	// filter was added
	if (notify_type == FWPS_CALLOUT_NOTIFY_ADD_FILTER) {
		KdPrint(("[wfpkm] " __FUNCTION__ " - create a filter (%llu)", filter->filterId));
	}
	// filter was deleted
	else if (notify_type == FWPS_CALLOUT_NOTIFY_DELETE_FILTER) {
		KdPrint(("[wfpkm] " __FUNCTION__ " - delete a filter (%llu)", filter->filterId));
	}

	return STATUS_SUCCESS;
}



void NTAPI FlowDeleteFunctionRoutine(
	_In_ UINT16 layer_id,
	_In_ UINT32 callout_id,
	_In_ UINT64 flow_context
)
{
	UNREFERENCED_PARAMETER(layer_id);
	UNREFERENCED_PARAMETER(callout_id);
	UNREFERENCED_PARAMETER(flow_context);

	return;
}