#pragma once

#include <ntddk.h>
#include <fwpmk.h>
#include <fwpsk.h>

DEFINE_GUID(
	WFPKM_PROVIDER_KEY, 
	0x11111111, 
	0x2222, 
	0x3333, 
	0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44
);



//
// wfp configuration functions
//
NTSTATUS InitWfp(
    _In_ PDEVICE_OBJECT device_object
);

NTSTATUS FinWfp();

NTSTATUS AddCalloutToLayer(
	_In_ const GUID *layer_key
);



//
// callout routines
//
void NTAPI ClassifyFunctionRoutine(
    _In_        const FWPS_INCOMING_VALUES0* fixed_values,
    _In_        const FWPS_INCOMING_METADATA_VALUES0* meta_values,
    _Inout_opt_ void* layer_data,
    _In_opt_    const void* classify_context,
    _In_        const FWPS_FILTER3* filter,
    _In_        UINT64 flow_context,
    _Inout_     FWPS_CLASSIFY_OUT0* classify_out
);

NTSTATUS NTAPI NotifyFunctionRoutine(
    _In_    FWPS_CALLOUT_NOTIFY_TYPE notify_type,
    _In_    const GUID* filter_key,
    _Inout_ FWPS_FILTER3* filter
);

void NTAPI FlowDeleteFunctionRoutine(
    _In_ UINT16 layer_id,
    _In_ UINT32 callout_id,
    _In_ UINT64 flow_context
);