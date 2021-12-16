#include <pvdef.h>
#include <pvbdk.h>

void RtlInitializeListHead(OUT PLIST_ENTRY ListHead)
{
	ListHead->Flink=ListHead->Blink=ListHead;
}

void RtlInsertToListHead(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry)
{
	// Initialize the insertee.
	ListEntry->Flink=ListHead->Flink;
	ListEntry->Blink=ListHead;
	// Set the insertee surroundings.
	ListHead->Flink->Blink=ListEntry;
	ListHead->Flink=ListEntry;
}

void RtlInsertToListTail(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry)
{
	// Initialize the insertee.
	ListEntry->Blink=ListHead->Blink;
	ListEntry->Flink=ListHead;
	// Set the insertee surroundings.
	ListHead->Blink->Flink=ListEntry;
	ListHead->Blink=ListEntry;
}

void RtlRemoveListEntry(IN OUT PLIST_ENTRY ListEntry)
{
	ListEntry->Flink->Blink=ListEntry->Blink;
	ListEntry->Blink->Flink=ListEntry->Flink;
	ListEntry->Flink=ListEntry->Blink=ListEntry;
}