#pragma once

#include <ntddk.h>
#include <wdf.h>

#define WDFKM_NT_DEVICE_NAME "\\Device\\wdfkm_example_device"
#define WDFKM_DOS_DEVICE_NAME "\\DosDevices\\wdfkm_example_device"

NTSTATUS InitDevice(
	_In_ PDRIVER_OBJECT driver_object,
	_In_ PUNICODE_STRING registry_path,
	_Outptr_ PDEVICE_OBJECT* device_object
);

NTSTATUS WdfDriverDeviceAdd(
	_In_ WDFDRIVER wdfdriver,
	_Inout_ PWDFDEVICE_INIT device_init
);

VOID WdfDriverUnload(
	_In_ WDFDRIVER wdfdriver
);