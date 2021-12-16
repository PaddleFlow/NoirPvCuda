#include "pvdef.h"
#include "vad.h"

#define USER_STACK_SIZE		0x100000
#define USER_STACK_PAGES	0x100

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
	LIST_ENTRY ThreadListHead;
}KPROCESS,*PKPROCESS;

typedef struct _KTHREAD
{
	LIST_ENTRY ThreadListEntry;
	CONTEXT Context;
	PVOID StackBase;
	PKPROCESS Process;
	ULONG UniqueThreadId;
}KTHREAD,*PKTHREAD;

#if defined(_PSMGR)
KPROCESS PvIdleProcess={0};
extern ULONG64 PvUserKernelSharedRegionPhysicalBase;
#endif