#include <pvdef.h>

ULONG RtlFindSetBit(IN PVOID Bitmap,IN ULONG Length)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	for(ULONG i=0;i<(Length>>3);i++)
	{
		ULONG j;
		if(_BitScanForward64(&j,Bmp[i]))return (i<<6)+j;
	}
	return 0xFFFFFFFF;
}

ULONG RtlFindClearBit(IN PVOID Bitmap,IN ULONG Length)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	for(ULONG i=0;i<(Length>>3);i++)
	{
		ULONG j;
		if(_BitScanForward64(&j,~Bmp[i]))return (i<<6)+j;
	}
	return 0xFFFFFFFF;
}

ULONG RtlFindSetBitAndReset(IN PVOID Bitmap,IN ULONG Length)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	for(ULONG i=0;i<(Length>>3);i++)
	{
		ULONG j;
		if(_BitScanForward64(&j,Bmp[i]))
		{
			ULONG BitPos=(i<<6)+j;
			_bittestandreset64(&Bmp[i],j);
			return BitPos;
		}
	}
	return 0xFFFFFFFF;
}

ULONG RtlFindClearBitAndSet(IN PVOID Bitmap,IN ULONG Length)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	for(ULONG i=0;i<(Length>>3);i++)
	{
		ULONG j;
		if(_BitScanForward64(&j,~Bmp[i]))
		{
			ULONG BitPos=(i<<6)+j;
			_bittestandset64(&Bmp[i],j);
			return BitPos;
		}
	}
	return 0xFFFFFFFF;
}

void RtlSetBitmap(IN PVOID Bitmap,IN ULONG BitPosition)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	ULONG i=BitPosition>>6;
	ULONG j=BitPosition&0x3f;
	_bittestandset64(&Bmp[i],j);
}

void RtlResetBitmap(IN PVOID Bitmap,IN ULONG BitPosition)
{
	PULONG64 Bmp=(PULONG64)Bitmap;
	ULONG i=BitPosition>>6;
	ULONG j=BitPosition&0x3f;
	_bittestandreset64(&Bmp[i],j);
}