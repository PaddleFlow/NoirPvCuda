#include "pvdef.h"
#include "pvbdk.h"

#pragma once

#define POOL_ALIGNMENT_SIZE		0x10
#define POOL_ALIGNED_BASE(p)	(p&0xfffffffffffffff0)

#define PAGE_CACHING_WB			0
#define PAGE_CACHING_WT			1
#define PAGE_CACHING_UCM		2
#define PAGE_CACHING_UC			3

typedef union _KPAGE_RIGHTS
{
	struct
	{
		ULONG32 Present:1;
		ULONG32 Write:1;
		ULONG32 Execute:1;
		ULONG32 User:1;
		ULONG32 Caching:3;
		ULONG32 ShadowStack:1;
		ULONG32 PageSize:2;
		ULONG32 Global:1;
		ULONG32 PageInfo:3;
		ULONG32 Reserved:18;
	};
	ULONG32 Value;
}KPAGE_RIGHTS,*PKPAGE_RIGHTS;

ULONG64 MmAllocatePhysicalPage();
void MmFreePhysicalPage(IN ULONG64 PhysicalPage);
PVOID MmAllocatePageForPagingStructure();
void MmFreePageForPagingStructure(IN PVOID Page);
PVOID MmAllocateSystemPool(IN ULONG Length);
void MmFreeSystemPool(IN PVOID PoolAddress);
void MmMapVirtualPage(IN ULONG64 RootPage,IN PVOID VirtualPage,IN ULONG64 PhysicalPage,IN KPAGE_RIGHTS PageRights);
ULONG64 MmGetPhysicalAddress(IN PVOID VirtualAddress);
void MmInitializeRootPageForProcess(IN PVOID RootPage);
BOOL MmTurnOffCopyOnWrite();
BOOL MmTurnOnCopyOnWrite();