// Paravirtualization CVM

#define ConsoleInputPort	0
#define ConsoleOutputPort	1
#define ConsoleErrorPort	2

#define NOIR_DUMMY_INTERRUPT_VECTOR			0x20

#define NOIR_HYPERCALL_CODE_SHUTDOWN		0x0
#define NOIR_HYPERCALL_MEMORY_EXTENSION		0x1
#define NOIR_HYPERCALL_FILE_CREATE			0x2
#define NOIR_HYPERCALL_FILE_CLOSE			0x3
#define NOIR_HYPERCALL_FILE_READ			0x4
#define NOIR_HYPERCALL_FILE_WRITE			0x5
#define NOIR_HYPERCALL_FILE_SET_POINTER		0x6
#define NOIR_HYPERCALL_FILE_REMOVE			0x7

#define KGDT_KERNEL_CODE64			0x10
#define KGDT_KERNEL_DATA64			0x18
#define KGDT_USER_TEB32				0x20
#define KGDT_USER_TEB64				0x30
#define KGDT_KERNEL_PROC64			0x30
#define KGDT_KERNEL_TSS64			0x40
#define KGDT_USER_CODE32			0x50
#define KGDT_USER_DATA				0x58
#define KGDT_USER_CODE64			0x60

#define PvDefaultMaxMemoryExtension	15		// Default limit of memory extensions. (Default: 3.75 GiB)

#define PvCriticalRangePages		512
#define PvPagingStructuresPages		32256
#define PvBasicFreeMemoryPages		32768
#define PvExtendedFreeMemoryPages	65536

#define PvCriticalRangeSize			PvCriticalRangePages*PAGE_SIZE
#define PvPagingStructuresSize		PvPagingStructuresPages*PAGE_SIZE
#define PvBasicFreeMemorySize		PvBasicFreeMemoryPages*PAGE_SIZE
#define PvExtendedFreeMemorySize	PvExtendedFreeMemoryPages*PAGE_SIZE

#define PvCriticalRangeBase			0x0
#define PvPagingStructuresBase		0x200000
#define PvBasicFreeMemoryBase		0x8000000
#define PvExtendedFreeMemoryBase	0x10000000

#define PvCriticalRangeGva			0xFFFF800000000000
#define PvBootingModuleGva			0xFFFF800000200000
#define PvPagingPoolGva				0xFFFFF68000000000
#define PvSystemMemoryPoolGva		0xFFFFF80000000000
#define PvSwappableMemoryPoolGva	0xFFFFFA8000000000
#define PvHalRegionGva				0xFFFFFFFF80000000

/*
	Paravirtualized Physical Memory Map:
	Base		Size	Description
	0x00000000	2MiB	Critical Range
	0x00200000	126MiB	Paging Structure
	0x08000000	128MiB	Basic Free Memory
	0x10000000	256MiB	Extended Free Memory #1
	0x20000000	256MiB	Extended Free Memory #2
	...
*/

/*
	Critical Range Layout:
	Each vCPU owns a 4KiB page of memory area in the Critical Range.
	The 4KiB page is divided as the following:
	Offset	Size	Description
	+0x000	0x400	Interrupt Descriptor Table Entries
	+0x400	0x80	Global Descriptor Table Entries
	+0x480	0x80	Task State Segment
	+0x500	0x300	Reserved for Future Design
	+0x800	0x800	Paravirtualized Kernel Processor Block
*/

ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
PVOID MemAlloc(IN SIZE_T Length);
BOOL MemFree(IN PVOID Memory);

BOOL PvReadGuestStdIn(OUT PVOID Buffer,IN ULONG Size);
BOOL PvWriteGuestStdOut(IN PVOID Buffer,IN ULONG Size);
BOOL PvWriteGuestStdErr(IN PVOID Buffer,IN ULONG Size);

void PvdLocateImageDebugDatabasePath(IN ULONG64 GuestCr3,IN ULONG64 ImageBaseGva,OUT PWSTR DatabasePath,IN ULONG Cch);

extern HANDLE StdIn;
extern HANDLE StdOut;

CHAR DummyBuffer[256]={0};
BYTE DummyPointer=0;
CVM_HANDLE PvCvm;
PVOID PvCriticalRange=NULL;
PVOID PvPagingStructures=NULL;
PVOID PvBasicFreeMemory=NULL;
PVOID* PvMemoryExtensionPointerTable=NULL;
ULONG PvAllocatedMemoryExtensions=0;
ULONG PvLimitOfMemoryExtensions=PvDefaultMaxMemoryExtension;
ULONG64 PvParavirtualizedKernelEntryPoint=PvBootingModuleGva;