#include <Windows.h>
#include <dbghelp.h>

#define IMAGE_CODEVIEW_SIGNATURE		'SDSR'

typedef struct _IMAGE_CODEVIEW_DEBUG_DIRECTORY_ENTRY
{
	ULONG32 Signature;
	GUID Guid;
	ULONG32 Age;
	CHAR Path[1];
}IMAGE_CODEVIEW_DEBUG_DIRECTORY_ENTRY,*PIMAGE_CODEVIEW_DEBUG_DIRECTORY_ENTRY;

typedef struct _IMAGE_SYMBOL_LIST
{
	LIST_ENTRY List;
	IMAGEHLP_MODULE64 ModuleInformation;
	ULONG32 NumberOfSymbols;
	ULONG32 AccumulationIndex;
	PSYMBOL_INFO *SortedByName;
	PSYMBOL_INFO *SortedByAddr;
}IMAGE_SYMBOL_LIST,*PIMAGE_SYMBOL_LIST;

ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
PVOID MemAlloc(IN SIZE_T Length);
BOOL MemFree(IN PVOID Memory);

ULONG PvReadGuestMemory(IN ULONG64 GuestCr3,IN ULONG64 GuestAddress,OUT PVOID Buffer,IN ULONG Size,IN BOOLEAN VirtualAddress,IN ULONG64 Access);

void InitializeListHead(OUT PLIST_ENTRY ListHead);
void InsertHeadList(IN OUT PLIST_ENTRY ListHead,IN OUT PLIST_ENTRY Entry);
void InsertTailList(IN OUT PLIST_ENTRY ListHead,IN OUT PLIST_ENTRY Entry);
void RemoveListEntry(IN OUT PLIST_ENTRY Entry);

IMAGE_SYMBOL_LIST PvDebugImageListHead={0};
SRWLOCK PvDebugImageListLock=SRWLOCK_INIT;

extern PVOID PvCriticalRange;
extern PVOID PvPagingStructures;
extern PVOID PvBasicFreeMemory;