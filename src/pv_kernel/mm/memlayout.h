#include <pvdef.h>

// Paravirtualized OS Kernel Paging Layout

/*
	Paravirtualized Cuda Virtual Memory Layout
  
	00000000`00000000	+---------------------------------------+
						| User-Mode Virtual Memory Space		|
	00007FFF`FFFFF000	+---------------------------------------+
						| User-Kernel Shared Page				|
	00007FFF`FFFFFFFF	+---------------------------------------+

	FFFF8000`00000000	+---------------------------------------+
						| Booting & First-Party Modules			|
	FFFFF680`00000000	+---------------------------------------+
						| Paging Structures						|
	FFFFF7FF`FFFFF000	+---------------------------------------+
						| User-Kernel Shared Page				|
	FFFFF800`00000000	+---------------------------------------+
						| System Memory Pool					|
	FFFFFA80`00000000	+---------------------------------------+
						| Swappable Memory Pool					|
	FFFFFFFF`80000000	+---------------------------------------+
						| Hardware Abstract Level				|
	FFFFFFFF`FFFFFFFF	+---------------------------------------+
*/

/*
	Paravirtualized Cuda Physical Memory Layout

	00000000`00000000	+---------------------------------------+
						| System Critical Range					|
						| 2 MiB Size							|
	00000000`00200000	+---------------------------------------+
						| Paging Structures						|
						| 126 MiB Size							|
	00000000`08000000	+---------------------------------------+
						| Basic Free Memory						|
						| 128 MiB Size							|
	00000000`10000000	+---------------------------------------+
						| Extended Free Memory					|
						| 256 MiB Size Per Extension			|
						+---------------------------------------+
*/

// Define Physical Memory Layout
#define PvSystemCriticalRangePhysicalBase		0x0
#define PvPagingStructuresPhysicalBase			0x200000
#define PvBasicFreeMemoryPhysicalBase			0x8000000
#define PvExtendedFreeMemoryPhysicalBase		0x10000000

// Define Virtual Memory Layout
#define PvCriticalRangeVirtualBase				0xFFFF800000000000
#define PvBootingModulesVirtualBase				0xFFFF800000200000
#define PvPagingStructuresVirtualBase			0xFFFFF68000000000
#define PvUserKernelSharedRegionVirtualBase		0xFFFFF7FFFFFFF000
#define PvSystemMemoryPoolVirtualBase			0xFFFFF80000000000
#define PvSwappableMemoryPoolVirtualBase		0xFFFFFA8000000000
#define PvHardwareAbstractLevelVirtualBase		0xFFFFFFFF00000000

#define MmPagingStructureAllocationBitmapSize	(PAGE_SIZE-0x40)

// Pool must be aligned on 16-byte boundary.
typedef struct _KVA_POOL_HEAD
{
	ULONG32 NextPoolHead;	// Offset to the next pool.
	ULONG32 PrevPoolHead;	// Offset to previous pool.
	struct
	{
		ULONG64 InUse:1;
		ULONG64 Last:1;
		ULONG64 Reserved:63;
	};
}KVA_POOL_HEAD,*PKVA_POOL_HEAD;

// Setup bitmaps to indicate physical page availability.
PVOID MmCriticalRangeAllocationBitmap=(PVOID)(PvBootingModulesVirtualBase-PAGE_SIZE);
PVOID MmPagingStructureAllocationBitmap=(PVOID)(PvBootingModulesVirtualBase-PAGE_SIZE+0x40);
PVOID MmFreeMemoryAllocationBitmap=(PVOID)(PvPagingStructuresVirtualBase-PAGE_HUGE_SIZE);

// PoC-Stage design: use incremental-only allocation.
ULONG64 volatile MmSystemPoolCurrentAllocation=PvSystemMemoryPoolVirtualBase;
ULONG64 volatile MmSystemPoolFreedAllocation=PvSystemMemoryPoolVirtualBase;
ULONG64 volatile MmSystemPoolAllocationLimit=PvSystemMemoryPoolVirtualBase+PAGE_SIZE;
ULONG64 volatile MmSwappablePoolCurrentAllocation=PvSwappableMemoryPoolVirtualBase;

ULONG32 volatile MmAllocationBitmapLock=0;