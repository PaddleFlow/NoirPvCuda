#include "pvdef.h"
#include "vad.h"

typedef struct _HANDLE_TABLE
{
	ULONG64 TableCode;
	ULONG NumberOfHandles;
}HANDLE_TABLE,*PHANDLE_TABLE;

typedef struct _KPROCESS
{
	LIST_ENTRY ActiveListEntry;
	PVOID RootPage;
	ULONG64 PagingBase;
	KVAD VadRoot;
	ULONG UniqueProcessId;
	PVOID SectionBase;
	ULONG64 DefaultStackLimit;
	LIST_ENTRY ThreadListHead;
}KPROCESS,*PKPROCESS;

typedef struct _KTHREAD
{
	LIST_ENTRY ThreadListEntry;
	CONTEXT ThreadContext;
	PVOID StackBase;
	PKPROCESS Process;
	ULONG64 StackLimit;
	ULONG UniqueThreadId;
}KTHREAD,*PKTHREAD;

KPROCESS PvIdleProcess={0};