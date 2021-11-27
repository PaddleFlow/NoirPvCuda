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

ULONG PvPrintConsoleA(IN PCSTR Format,...);
ULONG PvPrintConsoleW(IN PCWSTR Format,...);
PVOID MemAlloc(IN SIZE_T Length);
BOOL MemFree(IN PVOID Memory);

ULONG PvReadGuestMemory(IN ULONG64 GuestCr3,IN ULONG64 GuestAddress,OUT PVOID Buffer,IN ULONG Size,IN BOOLEAN VirtualAddress,IN ULONG64 Access);

extern PVOID PvCriticalRange;
extern PVOID PvPagingStructures;
extern PVOID PvBasicFreeMemory;