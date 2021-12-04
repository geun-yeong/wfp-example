#include "wdfkm.h"
#include "wfpkm.h"
#include "error.h"



NTSTATUS InitDevice(
	_In_ PDRIVER_OBJECT driver_object,
	_In_ PUNICODE_STRING registry_path,
	_Outptr_ PDEVICE_OBJECT* device_object
)
{
	NTSTATUS status = STATUS_SUCCESS;

	WDF_OBJECT_ATTRIBUTES attributes = { 0, };
	WDF_DRIVER_CONFIG config         = { 0, };
	WDFDRIVER wdfdriver              = NULL;
	
	UNICODE_STRING device_name     = { 0, },
		           dos_device_name = { 0, };
	PWDFDEVICE_INIT device_init    = NULL;
	WDFDEVICE wdfdevice            = NULL;



	//
	// initialize an output
	//
	*device_object = NULL;



	//
	// create a wdf driver
	//
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.EvtCleanupCallback = NULL;
	attributes.EvtDestroyCallback = NULL;
	
	WDF_DRIVER_CONFIG_INIT(&config, NULL);
	config.EvtDriverUnload = WdfDriverUnload;
	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

	status = WdfDriverCreate(driver_object,
                             registry_path,
                             WDF_NO_OBJECT_ATTRIBUTES,
		                     &config,
                             &wdfdriver);
	IF_ERROR(WdfDriverCreate, EXIT_OF_INIT_DEVICE);



	//
	// create a wdf device
	//
	device_init = WdfControlDeviceInitAllocate(wdfdriver, &SDDL_DEVOBJ_KERNEL_ONLY);
	if (!device_init) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto EXIT_OF_INIT_DEVICE;
	}

	RtlInitUnicodeString(&device_name, TEXT(WDFKM_NT_DEVICE_NAME));
	status = WdfDeviceInitAssignName(device_init, &device_name);
	IF_ERROR(WdfDeviceInitAssignName, EXIT_OF_INIT_DEVICE);

	status = WdfDeviceCreate(&device_init, WDF_NO_OBJECT_ATTRIBUTES, &wdfdevice);
	IF_ERROR(WdfDeviceCreate, EXIT_OF_INIT_DEVICE);

	RtlInitUnicodeString(&dos_device_name, TEXT(WDFKM_DOS_DEVICE_NAME));
	status = IoCreateSymbolicLink(&dos_device_name, &device_name);
	if (status == STATUS_OBJECT_NAME_COLLISION) {
		IoDeleteSymbolicLink(&dos_device_name);
		status = IoCreateSymbolicLink(&dos_device_name, &device_name);
	}
	IF_ERROR(IoCreateSymbolicLink, EXIT_OF_INIT_DEVICE);



	//
	// done
	//
	*device_object = WdfDeviceWdmGetDeviceObject(wdfdevice);

EXIT_OF_INIT_DEVICE:

	if (!NT_SUCCESS(status)) {
		if(device_init) WdfDeviceInitFree(device_init);
	}

	return status;
}



NTSTATUS WdfDriverDeviceAdd(
	_In_ WDFDRIVER wdfdriver,
	_Inout_ PWDFDEVICE_INIT device_init
)
{
	UNREFERENCED_PARAMETER(wdfdriver);
	UNREFERENCED_PARAMETER(device_init);

	return STATUS_SUCCESS;
}



VOID WdfDriverUnload(
	_In_ WDFDRIVER wdfdriver
)
{
	UNREFERENCED_PARAMETER(wdfdriver);

	UNICODE_STRING dos_device_name = { 0, };

	FinWfp();

	RtlInitUnicodeString(&dos_device_name, TEXT(WDFKM_DOS_DEVICE_NAME));
	IoDeleteSymbolicLink(&dos_device_name);
}