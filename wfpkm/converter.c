#include "converter.h"
#include <ntstrsafe.h>



PSTR ConvertIpv4ToString(
	UINT32 ipv4,
	PCHAR buffer,
	size_t buffer_size
)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = RtlStringCbPrintfA(
		buffer,
		buffer_size,
		"%u.%u.%u.%u",
		(ipv4 >> 24) & 0xFF, (ipv4 >> 16) & 0xFF, (ipv4 >> 8) & 0xFF, ipv4 & 0xFF
	);

	return (NT_SUCCESS(status)) ? buffer : NULL;
}



PSTR ConvertIpv6ToString(
	UINT8* ipv6,
	PCHAR buffer,
	size_t buffer_size
)
{
	NTSTATUS status = STATUS_SUCCESS;

	status = RtlStringCbPrintfA(
		buffer,
		buffer_size,
		"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
		ipv6[0], ipv6[1], ipv6[2],  ipv6[3],  ipv6[4],  ipv6[5],  ipv6[6],  ipv6[7],
		ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15]
	);

	return (NT_SUCCESS(status)) ? buffer : NULL;
}