

#include <ntifs.h>
#include "NLSEUtility.h"
#include "nldircache.h"

//*****************************************************************************
// Declare Local Routines
//*****************************************************************************
VOID
NLFreeDInfo(
            __in PNLDINFO  DInfo
            );

BOOLEAN
RecordExpired(
              __in PNLDINFO  DInfo,
              __out PULONG Elapse
              );




//*****************************************************************************
// Define Global Routines
//*****************************************************************************
VOID
NLInitDList(
            __inout PNLDINFOLIST DList
            )
{
    KeInitializeSpinLock(&DList->Lock);
    InitializeListHead(&DList->ListHead);
    DList->Count = 0;
}

VOID
NLFreeDList(
            __inout PNLDINFOLIST DList
            )
{
    PLIST_ENTRY DInfoEntry = NULL;
    PNLDINFO    DInfo      = NULL;

    DInfoEntry = ExInterlockedRemoveHeadList(&DList->ListHead, &DList->Lock);
    while(DInfoEntry)
    {
        if(DList->Count>0) --DList->Count;
        DInfo = CONTAINING_RECORD(DInfoEntry, NLDINFO, Entry);
        if(NULL != DInfo)
            NLFreeDInfo(DInfo);

        DInfoEntry = ExInterlockedRemoveHeadList(&DList->ListHead, &DList->Lock);
    }
}

PNLDINFO
NLFindInDList(
              __in PNLDINFOLIST DList,
              __in PCUNICODE_STRING Path
              )
{
    KIRQL       OldIrql = 0;
    PLIST_ENTRY Link    = NULL;
    PNLDINFO    DInfoPtr= NULL;
    BOOLEAN     Found   = FALSE;
    NTSTATUS    Status  = STATUS_SUCCESS;
    
    UNICODE_STRING  UpperPath = {0, 0, NULL};
    UpperPath.MaximumLength = Path->MaximumLength;
    UpperPath.Length = 0;
    UpperPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, Path->MaximumLength, 'esln');
    if(NULL == UpperPath.Buffer)
        return NULL;
    RtlZeroMemory(UpperPath.Buffer, UpperPath.MaximumLength);
    RtlUpcaseUnicodeString(&UpperPath, Path, FALSE);

    //
    // Remove expired record
    //
    NLRemoveFirstExpiredRecord(DList);


    //
    // Find in list
    //
    KeAcquireSpinLock(&DList->Lock, &OldIrql);
    if(!IsListEmpty(&DList->ListHead))
    {
        for(Link=DList->ListHead.Flink; Link!=(PLIST_ENTRY)&DList->ListHead.Flink; Link=Link->Flink)
        {
            if(NULL==Link) break;

            DInfoPtr = (PNLDINFO)CONTAINING_RECORD(Link, NLDINFO, Entry);
            if(NULL!=DInfoPtr)
            {
                if(0==DInfoPtr->Path.Length || NULL==DInfoPtr->Path.Buffer)
                {
                    KdPrint(("\n!! Wrong Path Name !!\n\n"));
                }
                else
                {
                    if(UpperPath.Length==DInfoPtr->Path.Length && UpperPath.Length==RtlCompareMemory(UpperPath.Buffer, DInfoPtr->Path.Buffer, UpperPath.Length))
                    {
                        Found = TRUE;                        
                        break;
                    }
                }
            }
        }
    }
    KeReleaseSpinLock(&DList->Lock, OldIrql);

    if(UpperPath.Buffer) ExFreePool(UpperPath.Buffer);
    if(Found && NULL!=DInfoPtr)
    {
        KeQuerySystemTime(&DInfoPtr->Time);        
        NLRefDInfo(DInfoPtr);
    }
    return Found?DInfoPtr:NULL;
}

PNLFINFO
NLFindInDInfo(
              __in PNLDINFO DInfo,
              __in_z WCHAR* FileName,
              __in USHORT FileNameLength
              )
{
    KIRQL       OldIrql = 0;
    PLIST_ENTRY Link    = NULL;
    PNLFINFO    FInfo   = NULL;
    BOOLEAN     Found   = FALSE;
    NTSTATUS    Status  = STATUS_SUCCESS;

    UNICODE_STRING  Name= {FileNameLength, FileNameLength, FileName};
    UNICODE_STRING  UpperName = {0, 0, NULL};

    UpperName.MaximumLength = FileNameLength + sizeof(WCHAR);
    UpperName.Length = 0;
    UpperName.Buffer = ExAllocatePoolWithTag(NonPagedPool, UpperName.MaximumLength, 'esln');
    if(NULL == UpperName.Buffer)
        return NULL;
    RtlZeroMemory(UpperName.Buffer, UpperName.MaximumLength);
    RtlUpcaseUnicodeString(&UpperName, &Name, FALSE);


    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
    if(!IsListEmpty(&DInfo->FInfoList))
    {
        for(Link=DInfo->FInfoList.Flink; Link!=(PLIST_ENTRY)&DInfo->FInfoList.Flink; Link=Link->Flink)
        {
            if(NULL==Link) break;

            FInfo = (PNLFINFO)CONTAINING_RECORD(Link, NLFINFO, Entry);
            if(NULL!=FInfo)
            {
                if(0==FInfo->Name.Length || NULL==FInfo->Name.Buffer)
                {
                    KdPrint(("    Wrong File Name\n"));
                }
                else
                {
                    if(UpperName.Length==FInfo->Name.Length && UpperName.Length==RtlCompareMemory(UpperName.Buffer, FInfo->Name.Buffer, UpperName.Length))
                    {
                        Found = TRUE;
                        break;
                    }
                }
            }
        }
    }
    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);

    if(UpperName.Buffer) ExFreePool(UpperName.Buffer);
    return Found?FInfo:NULL;
}

PNLDINFO
NLCreateDInfo(
              __in PCUNICODE_STRING Path
              )
{
    PNLDINFO DInfo = NULL;

    DInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLDINFO), 'esln');
    if(NULL == DInfo)
        return NULL;
    RtlZeroMemory(DInfo, sizeof(NLDINFO));

    //
    // Init directory name
    //
    DInfo->Path.MaximumLength = Path->MaximumLength;
    DInfo->Path.Buffer        = ExAllocatePoolWithTag(NonPagedPool, Path->MaximumLength, 'esln');
    if(NULL == DInfo->Path.Buffer)
    {
        ExFreePool(DInfo);
        return NULL;
    }
    RtlZeroMemory(DInfo->Path.Buffer, Path->MaximumLength);
    //DInfo->Path.Length = Path->Length;
    //RtlCopyMemory(DInfo->Path.Buffer, Path->Buffer, Path->MaximumLength);
    RtlUpcaseUnicodeString(&DInfo->Path, Path, FALSE);

    //
    // Init other variabels
    //
    ExInitializeFastMutex(&DInfo->ReferenceLock);
    KeInitializeSpinLock(&DInfo->FInfoLock);
    InitializeListHead(&DInfo->Entry);
    InitializeListHead(&DInfo->FInfoList);
    KeQuerySystemTime(&DInfo->Time);

    return DInfo;
}

PNLFINFO
NLCreateFInfo(
              __in_z WCHAR* FileName,
              __in USHORT FileNameLength
              )
{
    PNLFINFO FInfo = NULL;
    UNICODE_STRING UnicodeFileName = {FileNameLength, FileNameLength, FileName};

    FInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLFINFO), 'esln');
    if(NULL == FInfo)
        return NULL;

    RtlZeroMemory(FInfo, sizeof(NLFINFO));
    InitializeListHead(&FInfo->Entry);
    FInfo->Name.MaximumLength = FileNameLength + sizeof(WCHAR);
    FInfo->Name.Buffer = ExAllocatePoolWithTag(NonPagedPool, FInfo->Name.MaximumLength, 'esln');
    if(NULL == FInfo)
    {
        ExFreePool(FInfo);
        return NULL;
    }
    RtlZeroMemory(FInfo->Name.Buffer, FInfo->Name.MaximumLength);
    //FInfo->Name.Length = FileNameLength;
    //RtlCopyMemory(FInfo->Name.Buffer, FileName, FInfo->Name.Length);
    RtlUpcaseUnicodeString(&FInfo->Name, &UnicodeFileName, FALSE);
    FInfo->Enc = FALSE;
    return FInfo;
}

VOID
NLFreeDInfo(
            __inout PNLDINFO DInfo
            )
{
    PLIST_ENTRY FInfoEntry = NULL;
    PNLFINFO    FInfo      = NULL;

    if(DInfo->ReferenceCount)
        return;
    

    FInfoEntry = ExInterlockedRemoveHeadList(&DInfo->FInfoList, &DInfo->FInfoLock);
    while(FInfoEntry)
    {
        FInfo = CONTAINING_RECORD(FInfoEntry, NLFINFO, Entry);
        if(NULL != FInfo)
        {
            if(FInfo->Name.Buffer) ExFreePool(FInfo->Name.Buffer);
            ExFreePool(FInfo);
            FInfo = NULL;
        }

        FInfoEntry = ExInterlockedRemoveHeadList(&DInfo->FInfoList, &DInfo->FInfoLock);
    }

	if (DInfo->Path.Buffer != NULL)
	{
		ExFreePool(DInfo->Path.Buffer);
	}
	
    ExFreePool(DInfo);
}

VOID
NLRefDInfo(
           __inout PNLDINFO DInfo
           )
{
    ExAcquireFastMutex(&DInfo->ReferenceLock);
    DInfo->ReferenceCount++;
    ExReleaseFastMutex(&DInfo->ReferenceLock);
}

VOID
NLDerefDInfo(
             __inout PNLDINFO DInfo
             )
{
    ExAcquireFastMutex(&DInfo->ReferenceLock);
    DInfo->ReferenceCount--;
    ExReleaseFastMutex(&DInfo->ReferenceLock);
}

BOOLEAN
NLRemoveFirstExpiredRecord(
                           __inout PNLDINFOLIST DList
                           )
{
    KIRQL       OldIrql = 0;
    PLIST_ENTRY Link    = NULL;
    PNLDINFO    DInfoPtr= NULL;
    BOOLEAN     Removed = FALSE;
    ULONG       Elapse  = 0;

    KeAcquireSpinLock(&DList->Lock, &OldIrql);
    for(Link=DList->ListHead.Flink; Link!=(PLIST_ENTRY)&DList->ListHead.Flink; Link=Link->Flink)
    {
        if(NULL==Link) break;
        DInfoPtr = (PNLDINFO)CONTAINING_RECORD(Link, NLDINFO, Entry);
        if(NULL!=DInfoPtr && RecordExpired(DInfoPtr, &Elapse) && 0==DInfoPtr->ReferenceCount)
        {
            RemoveEntryList(Link);
            Removed = TRUE;
            if(DList->Count>0) --DList->Count;
            break;
        }
    }
    KeReleaseSpinLock(&DList->Lock, OldIrql);

    if(Removed && NULL!=DInfoPtr)
    {
        KdPrint(("DirInfo Cache! Record expired (%d) and will be removed - %wZ\n", Elapse, &DInfoPtr->Path));
        NLFreeDInfo(DInfoPtr);
    }

    return Removed;
}

VOID
NLRemoveAllExpiredRecords(
                          __inout PNLDINFOLIST DList
                          )
{
    while(NLRemoveFirstExpiredRecord(DList))
    {
        ; // Do nothing
    }
}


VOID
NLRemoveFirstRecord(
                    __inout PNLDINFOLIST DList
                    )
{
    PLIST_ENTRY Link = NULL;
    PNLDINFO    DInfo= NULL;

    Link = ExInterlockedRemoveHeadList(&DList->ListHead, &DList->Lock);
    if(NULL != Link)
    {
        if(DList->Count>0) --DList->Count;
        DInfo = (PNLDINFO)CONTAINING_RECORD(Link, NLDINFO, Entry);
        if(NULL!=DInfo)
        {
            KdPrint(("DirInfo Cache! Directory List is full, remove first record - %wZ\n", &DInfo->Path));
            NLFreeDInfo(DInfo);
        }
    }
}

BOOLEAN
RecordExpired(
              __in PNLDINFO  DInfo,
              __out PULONG Elapse
              )
{
    LARGE_INTEGER   NewTime;

    KeQuerySystemTime(&NewTime);
    NewTime.QuadPart = (NewTime.QuadPart - DInfo->Time.QuadPart)/10000000; // seconds elapsed
    if(NULL!=Elapse) *Elapse = NewTime.LowPart;
    if(NewTime.QuadPart > 60)
        return TRUE;

    return FALSE;
}

VOID
NLInsertFInfo(
              __inout PNLDINFO DInfo,
              __inout PNLFINFO FInfo
              )
{
    PLIST_ENTRY Link    = NULL;
    PNLFINFO    FInfoPtr= NULL;
    BOOLEAN     Inserted= FALSE;
    //
    // Insert to empty list?
    //
    if(IsListEmpty(&DInfo->FInfoList))
    {
        InsertTailList(&DInfo->FInfoList, &FInfo->Entry);
        return;
    }

    //
    // Find a proper position
    //    
    for(Link=DInfo->FInfoList.Flink; (NULL!=Link) && (Link!=(PLIST_ENTRY)&DInfo->FInfoList.Flink); Link=Link->Flink)
    {
        FInfoPtr = CONTAINING_RECORD(Link, NLFINFO, Entry);
        if(NULL==FInfoPtr)
            continue;

        if(-1 == NLCompareName(FInfo->Name.Buffer, FInfo->Name.Length, FInfoPtr->Name.Buffer, FInfoPtr->Name.Length))
        {
            //
            // Insert this item
            //
            Link->Blink->Flink = &FInfo->Entry;
            FInfo->Entry.Blink = Link->Blink;
            Link->Blink        = &FInfo->Entry;
            FInfo->Entry.Flink = Link;
            return;
        }
    }
    
    //
    // Cannot find any item bigger than this, put it at tail
    //
    InsertTailList(&DInfo->FInfoList, &FInfo->Entry);
}