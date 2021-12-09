#include <pvdef.h>
#include <pvbdk.h>

#pragma once

typedef struct _KSECTION
{
	PVOID ImageBase;
	ULONG ImageSize;
	HANDLE FileHandle;
}KSECTION,*PKSECTION;

typedef union _KPTE_SECTION
{
	struct
	{
		ULONG64 Read:1;
		ULONG64 Write:1;
		ULONG64 Execute:1;
		ULONG64 Swappable:1;
		ULONG64 Shared:1;
		ULONG64 Uncacheable:1;
		ULONG64 SwappedOut:1;
		ULONG64 Reserved:5;
		ULONG64 PageBase:52;
	};
	ULONG64 Value;
}KPTE_SECTION,*PKPTE_SECTION;

// VAD, acronym that stands for Virtual Address Descriptor, is
// used to describe a range of pages allocated for the process.
typedef struct _KVAD
{
	LIST_ENTRY ViewList;
	// AVL Balanced Tree
	struct _KVAD *Parent;
	struct _KVAD *LeftChild;
	struct _KVAD *RightChild;
	ULONG Height;
	// Virtual-Address Range.
	ULONG64 StartingPfn;
	ULONG64 EndingPfn;
	PKSECTION Section;
	KPTE_SECTION SectionPtes[1];
}KVAD,*PKVAD;