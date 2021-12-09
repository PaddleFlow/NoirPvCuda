#include <pvdef.h>
#include <pvbdk.h>

void RtlInitializeListHead(OUT PLIST_ENTRY ListHead)
{
	ListHead->Flink=ListHead->Blink=ListHead;
}

void RtlInsertToListHead(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry)
{
	ListHead->Flink->Blink=ListEntry;
	ListEntry->Flink=ListHead->Flink;
	ListEntry->Blink=ListHead;
	ListHead->Flink=ListEntry;
}

void RtlInsertToListTail(OUT PLIST_ENTRY ListHead,IN PLIST_ENTRY ListEntry)
{
	ListHead->Blink->Flink=ListEntry;
	ListEntry->Blink=ListHead->Blink;
	ListEntry->Flink=ListHead;
	ListHead->Blink=ListEntry;
}

void RtlRemoveListEntry(IN OUT PLIST_ENTRY ListEntry)
{
	ListEntry->Flink->Blink=ListEntry->Blink;
	ListEntry->Blink->Flink=ListEntry->Flink;
	ListEntry->Flink=ListEntry->Blink=ListEntry;
}