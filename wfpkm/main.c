#include <ntddk.h>

#include "wdfkm.h"
#include "wfpkm.h"
#include "error.h"



NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT driver_object,
	_In_ PUNICODE_STRING registry_path
)
{
	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_OBJECT device_object;

	const GUID* layer_keys[] = { // monitored layers
		&FWPM_LAYER_INBOUND_TRANSPORT_V4,
		&FWPM_LAYER_INBOUND_TRANSPORT_V6,
		&FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
		&FWPM_LAYER_OUTBOUND_TRANSPORT_V6
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

	for (size_t i = 0; i < ARRAYSIZE(layer_keys); i++) {
		status = AddCalloutToLayer(layer_keys[i]);
		IF_ERROR(AddCalloutToLayer, EXIT_OF_DRIVER_ENTRY);
	}

EXIT_OF_DRIVER_ENTRY:

	if (!NT_SUCCESS(status)) {
		FinWfp();
	}

	return status;
}