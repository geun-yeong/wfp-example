#include <ntddk.h>

#include "wfpdev.h"
#include "wfpflt.h"
#include "../wfpkm/error.h"

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT driver_object,
	_In_ PUNICODE_STRING registry_path
)
{
	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_OBJECT device_object = NULL;

	const GUID* layer_keys[] = { // blocked layers
		&FWPM_LAYER_INBOUND_TRANSPORT_V4,
		&FWPM_LAYER_OUTBOUND_TRANSPORT_V4
	};

	//
	// initialize a device object
	//
	status = InitDevice(
		driver_object,
		registry_path,
		&device_object
	);
	IF_ERROR(InitDevice, EXIT_OF_DRIVER_ENTRY);
	KdPrint(("[wfpkm] " __FUNCTION__ " - InitDevice	success"));

	//
	// initialize wfp configurations
	//
	status = InitWfp(device_object);
	IF_ERROR(InitWfp, EXIT_OF_DRIVER_ENTRY);
	KdPrint(("[wfpkm] " __FUNCTION__ " - InitWfp success"));

	//
	// block all communication from/to 8.8.8.8 in transport layer
	//
	FWP_V4_ADDR_AND_MASK blocked = { 0x08'08'08'08 /*8.8.8.8*/, 
		                             0xFF'FF'FF'FF /*255.255.255.255*/};
	for (size_t i = 0; i < ARRAYSIZE(layer_keys); i++) {
		status = AddFilterIpv4(layer_keys[i], &blocked);
		IF_ERROR(AddFilterIpv4, EXIT_OF_DRIVER_ENTRY);
	}

EXIT_OF_DRIVER_ENTRY:

	if (!NT_SUCCESS(status)) {
		FinWfp();
	}

	return status;
}