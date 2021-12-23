#pragma once

#include <ntddk.h>
#include <wdf.h>

#define WFPFLT_NT_DEVICE_NAME "\\Device\\wfpflt_example_device"
#define WFPFLT_DOS_DEVICE_NAME "\\DosDevices\\wfpflt_example_device"

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