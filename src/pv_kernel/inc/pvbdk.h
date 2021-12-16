// Miscellaneous basic definitions...
#include "pvdef.h"
#include <stdarg.h>

#pragma once

#define ConsoleInputPort	0
#define ConsoleOutputPort	1
#define ConsoleErrorPort	2
#define ConsoleDebugPort	3

#define PAGE_BASE(x)			(x&0xFFFFFFFFFFFFF000)
#define PAGE_OFFSET(x)			(x&0xFFF)

#define PAGE_SIZE				0x1000
#define PAGE_LARGE_SIZE			0x200000
#define PAGE_HUGE_SIZE			0x40000000

#define PAGE_SHIFT				12
#define PAGE_LARGE_SHIFT		21
#define PAGE_HUGE_SHIFT			30

#define KGDT_KERNEL_CODE64		0x10
#define KGDT_KERNEL_DATA64		0x18
#define KGDT_USER_TEB32			0x20
#define KGDT_USER_TEB64			0x30
#define KGDT_KERNEL_PROC64		0x30
#define KGDT_KERNEL_TSS64		0x40
#define KGDT_USER_CODE32		0x50
#define KGDT_USER_DATA			0x58
#define KGDT_USER_CODE64		0x60

#define PvUserKernelSharedRegionUserVirtualBase		0x7FFFFFFFF000

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY *Flink;
	struct _LIST_ENTRY *Blink;
}LIST_ENTRY,*PLIST_ENTRY;

typedef union _R128
{
	float f[4];
	double d[2];
}R128;

#pragma pack(1)
typedef struct _FXSAVE_AREA
{
	struct
	{
		USHORT Fcw;
		USHORT Fsw;
		BYTE Ftw;
		BYTE Reserved;
		USHORT Fop;
		ULONG64 Fip;
		ULONG64 Fdp;
		ULONG32 MxCsr;
		ULONG32 MxCsrMask;
	}Fpu;
	struct
	{
		ULONG64 Mm0;		// St0
		ULONG64 Reserved0;
		ULONG64 Mm1;		// St1
		ULONG64 Reserved1;
		ULONG64 Mm2;		// St2
		ULONG64 Reserved2;
		ULONG64 Mm3;		// St3
		ULONG64 Reserved3;
		ULONG64 Mm4;		// St4
		ULONG64 Reserved4;
		ULONG64 Mm5;		// St5
		ULONG64 Reserved5;
		ULONG64 Mm6;		// St6
		ULONG64 Reserved6;
		ULONG64 Mm7;		// St7
		ULONG64 Reserved7;
	}x87;
	R128 Xmm0;
	R128 Xmm1;
	R128 Xmm2;
	R128 Xmm3;
	R128 Xmm4;
	R128 Xmm5;
	R128 Xmm6;
	R128 Xmm7;
	R128 Xmm8;
	R128 Xmm9;
	R128 Xmm10;
	R128 Xmm11;
	R128 Xmm12;
	R128 Xmm13;
	R128 Xmm14;
	R128 Xmm15;
	ULONG64 Reserved[6];
}FXSAVE_AREA,*PFXSAVE_AREA;
#pragma pack()

typedef struct _CONTEXT
{
	ULONG64 P1Home;
	ULONG64 P2Home;
	ULONG64 P3Home;
	ULONG64 P4Home;
	ULONG64 P5Home;
	ULONG64 P6Home;
	ULONG32 ContextFlags;
	ULONG32 MxCsr;
	USHORT SegCs;
	USHORT SegDs;
	USHORT SegEs;
	USHORT SegFs;
	USHORT SegGs;
	USHORT SegSs;
	ULONG32 Eflags;
	ULONG64 Dr0;
	ULONG64 Dr1;
	ULONG64 Dr2;
	ULONG64 Dr3;
	ULONG64 Dr6;
	ULONG64 Dr7;
	ULONG64 Rax;
	ULONG64 Rcx;
	ULONG64 Rdx;
	ULONG64 Rbx;
	ULONG64 Rsp;
	ULONG64 Rbp;
	ULONG64 Rsi;
	ULONG64 Rdi;
	ULONG64 R8;
	ULONG64 R9;
	ULONG64 R10;
	ULONG64 R11;
	ULONG64 R12;
	ULONG64 R13;
	ULONG64 R14;
	ULONG64 R15;
	ULONG64 Rip;
	PVOID XsaveArea;
	FXSAVE_AREA FxState;
}CONTEXT,*PCONTEXT;

void RtlMoveMemory(IN PVOID Destination,IN PVOID Source,IN ULONG SizeInBytes);
void RtlCopyMemory(IN PVOID Destination,IN PVOID Source,IN ULONG SizeInBytes);
void RtlFillMemory(IN PVOID Destination,IN BYTE Filler,IN ULONG SizeInBytes);
void RtlZeroMemory(IN PVOID Destination,IN ULONG SizeInBytes);

void RtlInitializeListHead(OUT PLIST_ENTRY ListHead);
void RtlInsertToListHead(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry);
void RtlInsertToListTail(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry);
void RtlRemoveListEntry(IN OUT PLIST_ENTRY ListEntry);

void RtlSetBitmap(IN PVOID Bitmap,IN ULONG BitPosition);
void RtlResetBitmap(IN PVOID Bitmap,IN ULONG BitPosition);
ULONG RtlFindSetBit(IN PVOID Bitmap,IN ULONG Length);
ULONG RtlFindClearBit(IN PVOID Bitmap,IN ULONG Length);
ULONG RtlFindSetBitAndReset(IN PVOID Bitmap,IN ULONG Length);
ULONG RtlFindClearBitAndSet(IN PVOID Bitmap,IN ULONG Length);

ULONG RtlGetStringLengthA(IN PSTR String,IN ULONG Limit);
ULONG RtlCopyStringA(OUT PSTR Destination,IN PSTR Source,IN ULONG DestinationLimit);
ULONG RtlReverseStringA(IN OUT PSTR String,IN ULONG Limit);
ULONG RtlVPrintStringA(OUT PSTR Destination,IN ULONG Limit,IN PSTR Format,va_list ArgList);

void RtlCaptureContext(OUT PCONTEXT Context);
void RtlRestoreContext(IN PCONTEXT Context);

ULONG PvPrintStdOut(IN PSTR Format,...);