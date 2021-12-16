#include <pvdef.h>
#include <pvbdk.h>
#include <mm.h>
#include <page.h>
#include "memlayout.h"

BOOL MmTurnOffCopyOnWrite()
{
	ULONG64 Cr0=__readcr0();
	BOOL WP=_bittestandreset64(&Cr0,16);
	__writecr0(Cr0);
	return WP;
}

BOOL MmTurnOnCopyOnWrite()
{
	ULONG64 Cr0=__readcr0();
	BOOL WP=_bittestandset64(&Cr0,16);
	__writecr0(Cr0);
	return WP;
}

void static MmSetAllocationBitmap(IN ULONG64 PhysicalPage)
{
	if(PhysicalPage>=PvSystemCriticalRangePhysicalBase && PhysicalPage<PvBasicFreeMemoryPhysicalBase)
		RtlSetBitmap(MmCriticalRangeAllocationBitmap,(ULONG)(PhysicalPage>>PAGE_SHIFT));
	else
		RtlSetBitmap(MmFreeMemoryAllocationBitmap,(ULONG)((PhysicalPage-PvBasicFreeMemoryPhysicalBase)>>PAGE_SHIFT));
}

void static MmResetAllocationBitmap(IN ULONG64 PhysicalPage)
{
	if(PhysicalPage>=PvSystemCriticalRangePhysicalBase && PhysicalPage<PvBasicFreeMemoryPhysicalBase)
		RtlResetBitmap(MmCriticalRangeAllocationBitmap,(ULONG)(PhysicalPage>>PAGE_SHIFT));
	else
		RtlResetBitmap(MmFreeMemoryAllocationBitmap,(ULONG)((PhysicalPage-PvBasicFreeMemoryPhysicalBase)>>PAGE_SHIFT));
}

void static MmCombineFreedPool(IN ULONG64 PoolHeader)
{
	PKVA_POOL_HEAD PoolHead=(PKVA_POOL_HEAD)PoolHeader;
	PKVA_POOL_HEAD NextPool=(PKVA_POOL_HEAD)((ULONG64)PoolHead+PoolHead->NextPoolHead);
	while(NextPool->InUse==0)
	{
		PoolHead->NextPoolHead+=NextPool->NextPoolHead;
		NextPool=(PKVA_POOL_HEAD)((ULONG64)PoolHead+PoolHead->NextPoolHead);
	}
}

PVOID MmAllocateSystemPool(IN ULONG Length)
{
	ULONG PoolSize=POOL_ALIGNED_BASE(Length)+(Length&0xF?0x10:0)+sizeof(KVA_POOL_HEAD);
	PKVA_POOL_HEAD PoolHeader=(PKVA_POOL_HEAD)MmSystemPoolCurrentAllocation;
	PKVA_POOL_HEAD NextPoolHeader=(PKVA_POOL_HEAD)(MmSystemPoolCurrentAllocation+PoolSize);
	if(PoolHeader->Last)
	{
		if((ULONG64)PoolHeader+sizeof(KVA_POOL_HEAD)*2+Length>=MmSystemPoolAllocationLimit)
		{
			// Allocation is reaching limit...
			ULONG64 NumberOfBytes=(ULONG64)PoolHeader+sizeof(KVA_POOL_HEAD)*2+Length-MmSystemPoolAllocationLimit;
			ULONG64 NumberOfPages=(NumberOfBytes>>PAGE_SHIFT)+PAGE_OFFSET(NumberOfBytes)?1:0;
			KPAGE_RIGHTS PageRights;
			PageRights.Value=0;
			PageRights.Present=1;
			PageRights.Write=1;
			PageRights.Execute=1;
			PageRights.Caching=PAGE_CACHING_WB;
			// Allocate Pages for System Pool
			for(ULONG64 i=0;i<NumberOfPages;i++)
			{
				ULONG64 PhysicalPage=MmAllocatePhysicalPage();
				if(!PhysicalPage)return NULL;
				PVOID VirtualPage=(PVOID)MmSystemPoolAllocationLimit;
				MmMapVirtualPage(PvPagingStructuresPhysicalBase,(PVOID)MmSystemPoolAllocationLimit,PhysicalPage,PageRights);
				// Increment the limit if page is successfully allocated.
				MmSystemPoolAllocationLimit+=PAGE_SIZE;
			}
		}
		// Mark the current pool is not the last but the next is the last.
		PoolHeader->Last=FALSE;
		NextPoolHeader->Last=TRUE;
	}
	// Mark the pool is in use.
	PoolHeader->NextPoolHead=PoolSize;
	PoolHeader->InUse=1;
	PoolHeader->Reserved=0;
	// Initialize header for next pool.
	NextPoolHeader->PrevPoolHead=PoolSize;
	MmSystemPoolCurrentAllocation=(ULONG64)NextPoolHeader;
	return (PVOID)((ULONG64)PoolHeader+sizeof(KVA_POOL_HEAD));
}

void MmFreeSystemPool(IN PVOID PoolAddress)
{
	PKVA_POOL_HEAD CurPool=(PKVA_POOL_HEAD)((ULONG64)PoolAddress-sizeof(KVA_POOL_HEAD));
	// Set to not used.
	CurPool->InUse=0;
	// Change the freed allocation and combine.
	if((ULONG64)CurPool<MmSystemPoolFreedAllocation)
	{
		MmSystemPoolFreedAllocation=(ULONG64)CurPool;
		MmCombineFreedPool(MmSystemPoolFreedAllocation);
	}
}

ULONG64 MmGetPhysicalAddress(IN PVOID VirtualAddress)
{
	VIRTUAL_ADDRESS Va;
	PKPML4E Pml4eBase=(PKPML4E)PvPagingStructuresVirtualBase;
	Va.Value=(ULONG64)VirtualAddress;
	// Traverse the four levels of paging structure to retrieve the physical address.
	if(Pml4eBase[Va.Normal.Pml4eIndex].Present)
	{
		PKPDPTE PdpteBase=(PKPDPTE)((ULONG64)PvPagingStructuresVirtualBase+(Pml4eBase[Va.Normal.Pml4eIndex].PdpteBase<<PAGE_SHIFT)-PvPagingStructuresPhysicalBase);
		if(PdpteBase[Va.Normal.PdpteIndex].Normal.Present)
		{
			if(PdpteBase[Va.Normal.PdpteIndex].Huge.PageSize)
				return (PdpteBase[Va.Huge.PdpteIndex].Huge.PageBase<<PAGE_HUGE_SHIFT)+Va.Huge.PageOffset;
			else
			{
				PKPDE PdeBase=(PKPDE)((ULONG64)PvPagingStructuresVirtualBase+(PdpteBase[Va.Normal.PdpteIndex].Normal.PdeBase<<PAGE_SHIFT)-PvPagingStructuresPhysicalBase);
				if(PdeBase[Va.Normal.PdeIndex].Normal.Present)
				{
					if(PdeBase[Va.Normal.PdeIndex].Large.PageSize)
						return (PdeBase[Va.Large.PdeIndex].Large.PageBase<<PAGE_LARGE_SHIFT)+Va.Large.PageOffset;
					else
					{
						PKPTE PteBase=(PKPTE)((ULONG64)PvPagingStructuresVirtualBase+(PdeBase[Va.Normal.PdeIndex].Normal.PteBase<<PAGE_SHIFT)-PvPagingStructuresPhysicalBase);
						if(PteBase[Va.Normal.PteIndex].Present)return (PteBase[Va.Normal.PteIndex].PageBase<<PAGE_SHIFT)+Va.Normal.PageOffset;
					}
				}
			}
		}
	}
	return 0;
}

ULONG64 MmAllocatePhysicalPage()
{
	ULONG Index=RtlFindClearBitAndSet(MmFreeMemoryAllocationBitmap,PAGE_SIZE);
	if(Index==0xFFFFFFFF)return 0;
	return PvBasicFreeMemoryPhysicalBase+(Index<<PAGE_SHIFT);
}

void MmFreePhysicalPage(IN ULONG64 PhysicalPage)
{
	MmResetAllocationBitmap(PhysicalPage);
}

PVOID MmAllocatePageForPagingStructure()
{
	ULONG Index=RtlFindClearBitAndSet(MmPagingStructureAllocationBitmap,MmPagingStructureAllocationBitmapSize);
	if(Index==0xFFFFFFFF)return NULL;
	return (PVOID)(PvPagingStructuresVirtualBase+(Index<<PAGE_SHIFT));
}

void MmFreePageForPagingStructure(IN PVOID Page)
{
	ULONG64 BaseOffset=(ULONG64)Page-PvPagingStructuresVirtualBase;
	MmResetAllocationBitmap(PvPagingStructuresPhysicalBase+BaseOffset);
}

void MmMapVirtualPage(IN ULONG64 RootPage,IN PVOID VirtualPage,IN ULONG64 PhysicalPage,IN KPAGE_RIGHTS PageRights)
{
	PKPML4E Pml4eBase=(PKPML4E)(PvPagingStructuresVirtualBase+RootPage-PvPagingStructuresPhysicalBase);
	PKPDPTE PdpteBase;
	PKPDE PdeBase;
	PKPTE PteBase;
	VIRTUAL_ADDRESS Va;
	Va.Value=(ULONG64)VirtualPage;
	// If the PML4E is absent, there is no corresponding underlying paging structures either.
	if(Pml4eBase[Va.Normal.Pml4eIndex].Present==0)
	{
		// Allocate PDPTE Page and initialize to zeroes.
		PdpteBase=MmAllocatePageForPagingStructure();
		__stosq(PdpteBase,0,512);
		// Set the PDPTE Base in this PML4E.
		Pml4eBase[Va.Normal.Pml4eIndex].PdpteBase=((ULONG64)PdpteBase-PvPagingStructuresVirtualBase+PvPagingStructuresPhysicalBase)>>PAGE_SHIFT;
	}
	else
	{
		// Calculate the PDPTE Base.
		ULONG64 PdptePhysBase=Pml4eBase[Va.Normal.Pml4eIndex].PdpteBase<<PAGE_SHIFT;
		PdpteBase=(PKPDPTE)(PdptePhysBase-PvPagingStructuresPhysicalBase+PvPagingStructuresVirtualBase);
	}
	// Set up the PML4E.
	Pml4eBase[Va.Normal.Pml4eIndex].Present=1;
	Pml4eBase[Va.Normal.Pml4eIndex].Write=1;
	Pml4eBase[Va.Normal.Pml4eIndex].User=1;
	Pml4eBase[Va.Normal.Pml4eIndex].NoExecute=0;
	// If the PDPTE is absent, there is no corresponding underlying paging structures either.
	if(PdpteBase[Va.Normal.PdpteIndex].Normal.Present==0)
	{
		// Allocate PDE Page and initialize to zeroes.
		PdeBase=MmAllocatePageForPagingStructure();
		__stosq(PdeBase,0,512);
		// Set the PDE Base in this PDPTE.
		PdpteBase[Va.Normal.PdpteIndex].Normal.PdeBase=((ULONG64)PdeBase-PvPagingStructuresVirtualBase+PvPagingStructuresPhysicalBase)>>PAGE_SHIFT;
	}
	else
	{
		// Calculate the PDE Base.
		ULONG64 PdePhysBase=PdpteBase[Va.Normal.PdpteIndex].Normal.PdeBase<<PAGE_SHIFT;
		PdeBase=(PKPDE)(PdePhysBase-PvPagingStructuresPhysicalBase+PvPagingStructuresVirtualBase);
	}
	// Set up the PDPTE.
	if(PageRights.PageSize==2)
	{
		// Huge Page.
		PdpteBase[Va.Huge.PdpteIndex].Huge.Present=PageRights.Present;
		PdpteBase[Va.Huge.PdpteIndex].Huge.Write=PageRights.Write;
		PdpteBase[Va.Huge.PdpteIndex].Huge.User=PageRights.User;
		PdpteBase[Va.Huge.PdpteIndex].Huge.PageSize=TRUE;
		PdpteBase[Va.Huge.PdpteIndex].Huge.NoExecute=!PageRights.Execute;
		PdpteBase[Va.Huge.PdpteIndex].Huge.WriteThrough=_bittest(&PageRights.Value,4);
		PdpteBase[Va.Huge.PdpteIndex].Huge.CacheDisable=_bittest(&PageRights.Value,5);
		PdpteBase[Va.Huge.PdpteIndex].Huge.Pat=_bittest(&PageRights.Value,6);
		if(PageRights.ShadowStack)
		{
			PdpteBase[Va.Huge.PdpteIndex].Huge.Write=FALSE;
			PdpteBase[Va.Huge.PdpteIndex].Huge.Dirty=TRUE;
		}
		PdpteBase[Va.Huge.PdpteIndex].Huge.PageBase=PhysicalPage>>PAGE_HUGE_SHIFT;
		MmFreePageForPagingStructure(PdeBase);
	}
	else
	{
		// Large Page or Normal Page.
		PdpteBase[Va.Normal.PdpteIndex].Normal.Present=1;
		PdpteBase[Va.Normal.PdpteIndex].Normal.Write=1;
		PdpteBase[Va.Normal.PdpteIndex].Normal.User=1;
		PdpteBase[Va.Normal.PdpteIndex].Normal.NoExecute=0;
		// If the PDE is absent, there is no corresponding paging structures either.
		if(PdeBase[Va.Normal.PdeIndex].Normal.Present==0)
		{
			// Allocate PDE Page and initialize to zeroes.
			PteBase=MmAllocatePageForPagingStructure();
			__stosq(PteBase,0,512);
			// Set the PTE Base in this PDE.
			PdeBase[Va.Normal.PdeIndex].Normal.PteBase=((ULONG64)PteBase-PvPagingStructuresVirtualBase+PvPagingStructuresPhysicalBase)>>PAGE_SHIFT;
		}
		else
		{
			// Calculate the PTE Base.
			ULONG64 PtePhysBase=PdeBase[Va.Normal.PdeIndex].Normal.PteBase<<PAGE_SHIFT;
			PteBase=(PKPTE)(PtePhysBase-PvPagingStructuresPhysicalBase+PvPagingStructuresVirtualBase);
		}
		// Set up the PDE.
		if(PageRights.PageSize==1)
		{
			// Large Page.
			PdeBase[Va.Large.PdeIndex].Large.Present=PageRights.Present;
			PdeBase[Va.Large.PdeIndex].Large.Write=PageRights.Write;
			PdeBase[Va.Large.PdeIndex].Large.User=PageRights.User;
			PdeBase[Va.Large.PdeIndex].Large.PageSize=TRUE;
			PdeBase[Va.Large.PdeIndex].Large.NoExecute=!PageRights.Execute;
			PdeBase[Va.Large.PdeIndex].Large.WriteThrough=_bittest(&PageRights.Value,4);
			PdeBase[Va.Large.PdeIndex].Large.CacheDisable=_bittest(&PageRights.Value,5);
			PdeBase[Va.Large.PdeIndex].Large.Pat=_bittest(&PageRights.Value,6);
			if(PageRights.ShadowStack)
			{
				PdeBase[Va.Large.PdeIndex].Large.Write=FALSE;
				PdeBase[Va.Large.PdeIndex].Large.Dirty=TRUE;
			}
			PdeBase[Va.Large.PdeIndex].Large.PageBase=PhysicalPage>>PAGE_LARGE_SHIFT;
			MmFreePageForPagingStructure(PteBase);
		}
		else
		{
			// Normal Page.
			PdeBase[Va.Normal.PdeIndex].Normal.Present=1;
			PdeBase[Va.Normal.PdeIndex].Normal.Write=1;
			PdeBase[Va.Normal.PdeIndex].Normal.User=1;
			PdeBase[Va.Normal.PdeIndex].Normal.NoExecute=0;
			// Set up the PTE.
			PteBase[Va.Normal.PteIndex].Present=PageRights.Present;
			PteBase[Va.Normal.PteIndex].Write=PageRights.Write;
			PteBase[Va.Normal.PteIndex].User=PageRights.User;
			PteBase[Va.Normal.PteIndex].NoExecute=!PageRights.Execute;
			PteBase[Va.Normal.PteIndex].WriteThrough=_bittest(&PageRights.Value,4);
			PteBase[Va.Normal.PteIndex].CacheDisable=_bittest(&PageRights.Value,5);
			PteBase[Va.Normal.PteIndex].Pat=_bittest(&PageRights.Value,6);
			if(PageRights.ShadowStack)
			{
				PteBase[Va.Normal.PteIndex].Write=FALSE;
				PteBase[Va.Normal.PteIndex].Dirty=TRUE;
			}
			PteBase[Va.Normal.PteIndex].PageBase=PhysicalPage>>PAGE_SHIFT;
		}
	}
	// Invalidate the TLB for this entry.
	__invlpg(VirtualPage);
}

void MmInitializeUserKernelSharedRegion()
{
	KPAGE_RIGHTS PageRights;
	// Allocate a Physical Page for it.
	PvUserKernelSharedRegionPhysicalBase=MmAllocatePhysicalPage();
	// Map it to kernel space.
	PageRights.Value=0;
	PageRights.Present=1;
	PageRights.Write=1;
	PageRights.Caching=PAGE_CACHING_WB;
	PageRights.Global=1;
	MmMapVirtualPage(PvPagingStructuresPhysicalBase,(PVOID)PvUserKernelSharedRegionVirtualBase,PvUserKernelSharedRegionPhysicalBase,PageRights);
	// Initialize it.
}

void MmInitializeRootPageForProcess(IN PKPML4E RootPage)
{
	// Copy the PML4E of kernel pages...
	__movsq(&RootPage[256],(PVOID)(PvPagingStructuresVirtualBase+0x800),256);
}

void MmInitializeMemoryManager()
{
	// Initialize Bitmaps...
	MmSetAllocationBitmap(PvSystemCriticalRangePhysicalBase);						// CPU 0 Critical Structures...
	MmSetAllocationBitmap(PvPagingStructuresPhysicalBase-PAGE_SIZE);				// Allocation Bitmap Page itself...
	for(ULONG i=0;i<11;i++)
		MmSetAllocationBitmap(PvPagingStructuresPhysicalBase+(i<<PAGE_SHIFT));		// Initial Paging Structures...
	for(ULONG i=0;i<512;i++)														// Booting module and Stack...
		MmSetAllocationBitmap(PvBasicFreeMemoryPhysicalBase+(i<<PAGE_SHIFT));
	((PKVA_POOL_HEAD)MmSystemPoolCurrentAllocation)->NextPoolHead=0;
	((PKVA_POOL_HEAD)MmSystemPoolCurrentAllocation)->Last=TRUE;
}