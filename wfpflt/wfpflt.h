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



//
// function that add filter object that can block network events
//
NTSTATUS AddFilterIpv4(
    _In_ const GUID* layer_key,
    _In_ FWP_V4_ADDR_AND_MASK *ipv4
);