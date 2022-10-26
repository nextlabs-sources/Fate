#pragma once


__inline
	VOID
	InitializeListHead(
	IN PLIST_ENTRY ListHead
	)
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

__inline
	BOOL
	IsListEmpty(
	IN const LIST_ENTRY * ListHead
	)
{
	return (BOOLEAN)(ListHead->Flink == ListHead);
}

__inline
	BOOL
	RemoveEntryList(
	IN PLIST_ENTRY Entry
	)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Flink;

	Flink = Entry->Flink;
	Blink = Entry->Blink;
	Blink->Flink = Flink;
	Flink->Blink = Blink;
	return (Flink == Blink);
}

__inline
	PLIST_ENTRY
	RemoveHeadList(
	IN PLIST_ENTRY ListHead
	)
{
	PLIST_ENTRY Flink;
	PLIST_ENTRY Entry;

	Entry = ListHead->Flink;
	Flink = Entry->Flink;
	ListHead->Flink = Flink;
	Flink->Blink = ListHead;
	return Entry;
}

__inline
	PLIST_ENTRY
	RemoveTailList(
	IN PLIST_ENTRY ListHead
	)
{
	PLIST_ENTRY Blink;
	PLIST_ENTRY Entry;

	Entry = ListHead->Blink;
	Blink = Entry->Blink;
	ListHead->Blink = Blink;
	Blink->Flink = ListHead;
	return Entry;
}


__inline
	VOID
	InsertTailList(
	IN PLIST_ENTRY ListHead,
	IN PLIST_ENTRY Entry
	)
{
	PLIST_ENTRY Blink;

	Blink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = Blink;
	Blink->Flink = Entry;
	ListHead->Blink = Entry;
}

__inline
	VOID
	InsertHeadList(
	IN PLIST_ENTRY ListHead,
	IN PLIST_ENTRY Entry
	)
{
	PLIST_ENTRY Flink;

	Flink = ListHead->Flink;
	Entry->Flink = Flink;
	Entry->Blink = ListHead;
	Flink->Blink = Entry;
	ListHead->Flink = Entry;
}



