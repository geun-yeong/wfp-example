#pragma once

#include <ntddk.h>



PSTR ConvertIpv4ToString(
	UINT32 ipv4, 
	PCHAR buffer, 
	size_t buffer_size
);

PSTR ConvertIpv6ToString(
	UINT8* ipv6, 
	PCHAR buffer, 
	size_t buffer_size
);