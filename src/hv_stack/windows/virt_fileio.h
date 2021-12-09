#include <Windows.h>

// Certain structures that supports asynchronously operating files.
typedef struct _PV_FILE_ASYNC_IO
{
	HANDLE FileHandle;
	OVERLAPPED OverlappedStatus;
	ULONG64 GVA;
	ULONG64 GuestCR3;
	PVOID OperationBuffer;
	LARGE_INTEGER FilePointer;
	ULONG RemainingSize;
	BOOLEAN IsWrite;
}PV_FILE_ASYNC_IO,*PPV_FILE_ASYNC_IO;

ULONG32 PvTranslateGvaToHva64(IN ULONG64 GuestCr3,IN ULONG64 Gva,IN ULONG64 Access,OUT PULONG64 Hva);
ULONG PvPrintConsoleA(IN PCSTR Format,...);

SRWLOCK AsyncFileIoLock=SRWLOCK_INIT;
PPV_FILE_ASYNC_IO AsyncFileIoHead=NULL;
PPV_FILE_ASYNC_IO AsyncFileIoTail=NULL;
HANDLE PvFileEvent=NULL;