#include <pvdef.h>

// Basic memory operation codes are subject to be rewritten in Assembly for best performance.

void RtlMoveMemory(IN PVOID Destination,IN PVOID Source,IN ULONG SizeInBytes)
{
	PBYTE dest=(PBYTE)Destination;
	PBYTE src=(PBYTE)Source;
	for(ULONG i=0;i<SizeInBytes;i++)dest[SizeInBytes-i-1]=src[SizeInBytes-i-1];
}

void RtlCopyMemory(IN PVOID Destination,IN PVOID Source,IN ULONG SizeInBytes)
{
	__movsb(Destination,Source,SizeInBytes);
}

void RtlFillMemory(IN PVOID Destination,IN BYTE Filler,IN ULONG SizeInBytes)
{
	__stosb(Destination,Filler,SizeInBytes);
}