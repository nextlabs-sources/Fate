
#include <ntifs.h>
#include <fltKernel.h>
#include "nlsestruct.h"
#include "header.h"
#include "NLSEUtility.h"
#include "nldircache.h"


/*************************************************************************************
 *
 * Declare Local Routine
 *
*************************************************************************************/
typedef struct _SORTEDINFOLISTITEM
{
    LIST_ENTRY Entry;
    PVOID      Info;
} SORTEDINFOLISTITEM, *PSORTEDINFOLISTITEM;

#define GetNextEntryOffset(p)       (*((ULONG*)(p)))
#define GetNextInfoEntry(_p, _type) ( (0 == GetNextEntryOffset(_p)) ? NULL : ((_type*)((PUCHAR)(_p) + GetNextEntryOffset(_p))) )
#define GetNextVoidInfoEntry(p)     GetNextInfoEntry(p, VOID)

#define GetInfoFileName(Info, Type)         (((Type*)(Info))->FileName)
#define GetInfoFileNameLength(Info, Type)   (((Type*)(Info))->FileNameLength)


#define FixDirInfoSizeData(Info, Type)                                                                          \
    ((Type*)(Info))->AllocationSize.QuadPart -= NLSE_ENVELOPE_SIZE;                                             \
    ((Type*)(Info))->AllocationSize.QuadPart = ROUND_TO_SIZE(((Type*)(Info))->AllocationSize.QuadPart, 4096);   \
    ((Type*)(Info))->EndOfFile.QuadPart      -= NLSE_ENVELOPE_SIZE;


static
BOOLEAN
IsFileEncrypted(
                __in PFLT_FILTER Filter,
                __in PFLT_INSTANCE Instance,
                __in PCUNICODE_STRING ParentDir,
                __in PCUNICODE_STRING FileName
                );

static
VOID
FreeSortedList(
               __inout_opt PLIST_ENTRY ListHead
               );

static
VOID
InsertFileBothDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    );

static
ULONG
BuildSortedFileBothDirectoryInformationList(
    __in PFILE_BOTH_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    );

static
VOID
InsertFileDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    );

static
ULONG
BuildSortedFileDirectoryInformationList(
    __in PFILE_DIRECTORY_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    );

static
VOID
InsertFileFullDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    );

static
ULONG
BuildSortedFileFullDirectoryInformationList(
    __in PFILE_FULL_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    );

static
VOID
InsertFileIdBothDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    );

static
ULONG
BuildSortedFileIdBothDirectoryInformationList(
    __in PFILE_ID_BOTH_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    );

static
VOID
InsertFileIdFullDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    );

static
ULONG
BuildSortedFileIdFullDirectoryInformationList(
    __in PFILE_ID_FULL_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    );

/*************************************************************************************
 *
 * Define Global Routine
 *
*************************************************************************************/
VOID
CheckFileBothDirectoryInformation(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_BOTH_DIR_INFORMATION Info,
                                  __in BOOLEAN ProgramDir,
                                  __inout PNLDINFO DInfo
                                  )
{
    LIST_ENTRY  SortedListHead;
    ULONG       SortedListSize = 0;
    PLIST_ENTRY SortedLink     = NULL;
    PLIST_ENTRY FInfoLink      = NULL;
    BOOLEAN     CheckCache     = TRUE;
    KIRQL       OldIrql        = 0;

    InitializeListHead(&SortedListHead);
    SortedListSize = BuildSortedFileBothDirectoryInformationList(Info, &SortedListHead, ProgramDir);
    if(0 == SortedListSize)
        return;

    FInfoLink = DInfo->FInfoList.Flink;
    for(SortedLink=SortedListHead.Flink; (NULL!=SortedLink) && (SortedLink!=&SortedListHead); SortedLink=SortedLink->Flink)
    {
        PSORTEDINFOLISTITEM Item     = NULL;
        BOOLEAN             HitCache = FALSE;
        BOOLEAN             Encrypted= FALSE;
        PNLFINFO            FInfo    = NULL;
        UNICODE_STRING      FileName;
        Item = CONTAINING_RECORD(SortedLink, SORTEDINFOLISTITEM, Entry);
        
        FileName.Buffer = ((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileName;
        FileName.Length = (USHORT)((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength;
        FileName.MaximumLength = FileName.Length;

        while(FInfoLink!=NULL && FInfoLink != &(DInfo->FInfoList))
        {
            INT   CompareResult = 1;
            FInfo = NULL;
            FInfo = CONTAINING_RECORD(FInfoLink, NLFINFO, Entry);
            CompareResult = NLCompareName(FileName.Buffer, FileName.Length, FInfo->Name.Buffer, FInfo->Name.Length);
            if(CompareResult > 0)
            {
                // Input file name is smaller than current cache item
                // Move to next
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                continue;
            }
            else if(CompareResult == 0)
            {
                // Hit cache record
                HitCache   = TRUE;
                Encrypted  = FInfo->Enc;

                KdPrint(("    Hit File! %wZ (Encrypted: %d)\n", &FileName, Encrypted));
                if(Encrypted)
                {
                    FixDirInfoSizeData(Item->Info, FILE_BOTH_DIR_INFORMATION);
                }

                //
                // Point to next entry
                //
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                break;
            }
            else
            {
                // Input file name is bigger than current cache item
                // Don't need to search again
                break;
            }
        }

        if(!HitCache)
        {
            Encrypted = IsFileEncrypted(Filter, Instance, &DInfo->Path, &FileName);
            if(Encrypted)
            {
                FixDirInfoSizeData(Item->Info, FILE_BOTH_DIR_INFORMATION);
            }
            FInfo = NLCreateFInfo(((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileName, (USHORT)(((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength));
            if(FInfo)
            {
                FInfo->Enc = Encrypted;

                if(FInfoLink==NULL || FInfoLink == &(DInfo->FInfoList))
                {
                    ExInterlockedInsertTailList(&DInfo->FInfoList, &FInfo->Entry, &DInfo->FInfoLock);
                }
                else
                {
                    //
                    // FInfoLink point to entry which is greater than current item
                    // Insert current item before FInfoLink
                    //
                    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                    FInfo->Entry.Flink      = FInfoLink;
                    FInfo->Entry.Blink      = FInfoLink->Blink;
                    FInfoLink->Blink->Flink = &FInfo->Entry;
                    FInfoLink->Blink        = &FInfo->Entry;
                    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                }
                KdPrint(("    New File! %wZ (Encrypted: %d)\n", &FInfo->Name, FInfo->Enc));
            }
        }
    }

    FreeSortedList(&SortedListHead);
}

                                  
VOID
CheckFileDirectoryInformation(
                              __in PFLT_FILTER Filter,
                              __in PFLT_INSTANCE Instance,
                              __in PFILE_DIRECTORY_INFORMATION Info,
                              __in BOOLEAN ProgramDir,
                              __inout PNLDINFO DInfo
                              )
{
    LIST_ENTRY  SortedListHead;
    ULONG       SortedListSize = 0;
    PLIST_ENTRY SortedLink     = NULL;
    PLIST_ENTRY FInfoLink      = NULL;
    BOOLEAN     CheckCache     = TRUE;
    KIRQL       OldIrql        = 0;

    InitializeListHead(&SortedListHead);
    SortedListSize = BuildSortedFileDirectoryInformationList(Info, &SortedListHead, ProgramDir);
    if(0 == SortedListSize)
        return;

    FInfoLink = DInfo->FInfoList.Flink;
    for(SortedLink=SortedListHead.Flink; (NULL!=SortedLink) && (SortedLink!=&SortedListHead); SortedLink=SortedLink->Flink)
    {
        PSORTEDINFOLISTITEM Item     = NULL;
        BOOLEAN             HitCache = FALSE;
        BOOLEAN             Encrypted= FALSE;
        PNLFINFO            FInfo    = NULL;
        UNICODE_STRING      FileName;
        Item = CONTAINING_RECORD(SortedLink, SORTEDINFOLISTITEM, Entry);
        
        FileName.Buffer = ((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileName;
        FileName.Length = (USHORT)((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileNameLength;
        FileName.MaximumLength = FileName.Length;

        while(FInfoLink!=NULL && FInfoLink != &(DInfo->FInfoList))
        {
            INT   CompareResult = 1;
            FInfo = NULL;
            FInfo = CONTAINING_RECORD(FInfoLink, NLFINFO, Entry);
            CompareResult = NLCompareName(FileName.Buffer, FileName.Length, FInfo->Name.Buffer, FInfo->Name.Length);
            if(CompareResult > 0)
            {
                // Input file name is smaller than current cache item
                // Move to next
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                continue;
            }
            else if(CompareResult == 0)
            {
                // Hit cache record
                HitCache   = TRUE;
                Encrypted  = FInfo->Enc;

                KdPrint(("    Hit File! %wZ (Encrypted: %d)\n", &FileName, Encrypted));
                if(Encrypted)
                {
                    FixDirInfoSizeData(Item->Info, FILE_DIRECTORY_INFORMATION);
                }

                //
                // Point to next entry
                //
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                break;
            }
            else
            {
                // Input file name is bigger than current cache item
                // Don't need to search again
                break;
            }
        }

        if(!HitCache)
        {
            Encrypted = IsFileEncrypted(Filter, Instance, &DInfo->Path, &FileName);
            if(Encrypted)
            {
                FixDirInfoSizeData(Item->Info, FILE_DIRECTORY_INFORMATION);
            }
            FInfo = NLCreateFInfo(((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileName, (USHORT)(((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileNameLength));
            if(FInfo)
            {
                FInfo->Enc = Encrypted;

                if(FInfoLink==NULL || FInfoLink == &(DInfo->FInfoList))
                {
                    ExInterlockedInsertTailList(&DInfo->FInfoList, &FInfo->Entry, &DInfo->FInfoLock);
                }
                else
                {
                    //
                    // FInfoLink point to entry which is greater than current item
                    // Insert current item before FInfoLink
                    //
                    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                    FInfo->Entry.Flink      = FInfoLink;
                    FInfo->Entry.Blink      = FInfoLink->Blink;
                    FInfoLink->Blink->Flink = &FInfo->Entry;
                    FInfoLink->Blink        = &FInfo->Entry;
                    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                }
                KdPrint(("    New File! %wZ (Encrypted: %d)\n", &FInfo->Name, FInfo->Enc));
            }
        }
    }

    FreeSortedList(&SortedListHead);
}

                                  
VOID
CheckFileFullDirectoryInformation(
                                  __in PFLT_FILTER Filter,
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_FULL_DIR_INFORMATION Info,
                                  __in BOOLEAN ProgramDir,
                                  __inout PNLDINFO DInfo
                                  )
{
    LIST_ENTRY  SortedListHead;
    ULONG       SortedListSize = 0;
    PLIST_ENTRY SortedLink     = NULL;
    PLIST_ENTRY FInfoLink      = NULL;
    BOOLEAN     CheckCache     = TRUE;
    KIRQL       OldIrql        = 0;

    InitializeListHead(&SortedListHead);
    SortedListSize = BuildSortedFileFullDirectoryInformationList(Info, &SortedListHead, ProgramDir);
    if(0 == SortedListSize)
        return;

    FInfoLink = DInfo->FInfoList.Flink;
    for(SortedLink=SortedListHead.Flink; (NULL!=SortedLink) && (SortedLink!=&SortedListHead); SortedLink=SortedLink->Flink)
    {
        PSORTEDINFOLISTITEM Item     = NULL;
        BOOLEAN             HitCache = FALSE;
        BOOLEAN             Encrypted= FALSE;
        PNLFINFO            FInfo    = NULL;
        UNICODE_STRING      FileName;
        Item = CONTAINING_RECORD(SortedLink, SORTEDINFOLISTITEM, Entry);
        
        FileName.Buffer = ((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileName;
        FileName.Length = (USHORT)((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileNameLength;
        FileName.MaximumLength = FileName.Length;

        while(FInfoLink!=NULL && FInfoLink != &(DInfo->FInfoList))
        {
            INT   CompareResult = 1;
            FInfo = NULL;
            FInfo = CONTAINING_RECORD(FInfoLink, NLFINFO, Entry);
            CompareResult = NLCompareName(FileName.Buffer, FileName.Length, FInfo->Name.Buffer, FInfo->Name.Length);
            if(CompareResult > 0)
            {
                // Input file name is smaller than current cache item
                // Move to next
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                continue;
            }
            else if(CompareResult == 0)
            {
                // Hit cache record
                HitCache   = TRUE;
                Encrypted  = FInfo->Enc;

                KdPrint(("    Hit File! %wZ (Encrypted: %d)\n", &FileName, Encrypted));
                if(Encrypted)
                {
                    FixDirInfoSizeData(Item->Info, FILE_FULL_DIR_INFORMATION);
                }

                //
                // Point to next entry
                //
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                break;
            }
            else
            {
                // Input file name is bigger than current cache item
                // Don't need to search again
                break;
            }
        }

        if(!HitCache)
        {
            Encrypted = IsFileEncrypted(Filter, Instance, &DInfo->Path, &FileName);
            if(Encrypted)
            {
                FixDirInfoSizeData(Item->Info, FILE_FULL_DIR_INFORMATION);
            }
            FInfo = NLCreateFInfo(((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileName, (USHORT)(((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileNameLength));
            if(FInfo)
            {
                FInfo->Enc = Encrypted;

                if(FInfoLink==NULL || FInfoLink == &(DInfo->FInfoList))
                {
                    ExInterlockedInsertTailList(&DInfo->FInfoList, &FInfo->Entry, &DInfo->FInfoLock);
                }
                else
                {
                    //
                    // FInfoLink point to entry which is greater than current item
                    // Insert current item before FInfoLink
                    //
                    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                    FInfo->Entry.Flink      = FInfoLink;
                    FInfo->Entry.Blink      = FInfoLink->Blink;
                    FInfoLink->Blink->Flink = &FInfo->Entry;
                    FInfoLink->Blink        = &FInfo->Entry;
                    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                }
                KdPrint(("    New File! %wZ (Encrypted: %d)\n", &FInfo->Name, FInfo->Enc));
            }
        }
    }

    FreeSortedList(&SortedListHead);
}


VOID
CheckFileIdBothDirectoryInformation(
                                    __in PFLT_FILTER Filter,
                                    __in PFLT_INSTANCE Instance,
                                    __in PFILE_ID_BOTH_DIR_INFORMATION Info,
                                    __in BOOLEAN ProgramDir,
                                    __inout PNLDINFO DInfo
                                    )
{
    LIST_ENTRY  SortedListHead;
    ULONG       SortedListSize = 0;
    PLIST_ENTRY SortedLink     = NULL;
    PLIST_ENTRY FInfoLink      = NULL;
    BOOLEAN     CheckCache     = TRUE;
    KIRQL       OldIrql        = 0;

    InitializeListHead(&SortedListHead);
    SortedListSize = BuildSortedFileIdBothDirectoryInformationList(Info, &SortedListHead, ProgramDir);
    if(0 == SortedListSize)
        return;

    FInfoLink = DInfo->FInfoList.Flink;
    for(SortedLink=SortedListHead.Flink; (NULL!=SortedLink) && (SortedLink!=&SortedListHead); SortedLink=SortedLink->Flink)
    {
        PSORTEDINFOLISTITEM Item     = NULL;
        BOOLEAN             HitCache = FALSE;
        BOOLEAN             Encrypted= FALSE;
        PNLFINFO            FInfo    = NULL;
        UNICODE_STRING      FileName;
        Item = CONTAINING_RECORD(SortedLink, SORTEDINFOLISTITEM, Entry);
        
        FileName.Buffer = ((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileName;
        FileName.Length = (USHORT)((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength;
        FileName.MaximumLength = FileName.Length;

        while(FInfoLink!=NULL && FInfoLink != &(DInfo->FInfoList))
        {
            INT   CompareResult = 1;
            FInfo = NULL;
            FInfo = CONTAINING_RECORD(FInfoLink, NLFINFO, Entry);
            CompareResult = NLCompareName(FileName.Buffer, FileName.Length, FInfo->Name.Buffer, FInfo->Name.Length);
            if(CompareResult > 0)
            {
                // Input file name is smaller than current cache item
                // Move to next
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                continue;
            }
            else if(CompareResult == 0)
            {
                // Hit cache record
                HitCache   = TRUE;
                Encrypted  = FInfo->Enc;

                KdPrint(("    Hit File! %wZ (Encrypted: %d)\n", &FileName, Encrypted));
                if(Encrypted)
                {
                    FixDirInfoSizeData(Item->Info, FILE_ID_BOTH_DIR_INFORMATION);
                }

                //
                // Point to next entry
                //
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                break;
            }
            else
            {
                // Input file name is bigger than current cache item
                // Don't need to search again
                break;
            }
        }

        if(!HitCache)
        {
            Encrypted = IsFileEncrypted(Filter, Instance, &DInfo->Path, &FileName);
            if(Encrypted)
            {
                FixDirInfoSizeData(Item->Info, FILE_ID_BOTH_DIR_INFORMATION);
            }
            FInfo = NLCreateFInfo(((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileName, (USHORT)(((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength));
            if(FInfo)
            {
                FInfo->Enc = Encrypted;

                if(FInfoLink==NULL || FInfoLink == &(DInfo->FInfoList))
                {
                    ExInterlockedInsertTailList(&DInfo->FInfoList, &FInfo->Entry, &DInfo->FInfoLock);
                }
                else
                {
                    //
                    // FInfoLink point to entry which is greater than current item
                    // Insert current item before FInfoLink
                    //
                    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                    FInfo->Entry.Flink      = FInfoLink;
                    FInfo->Entry.Blink      = FInfoLink->Blink;
                    FInfoLink->Blink->Flink = &FInfo->Entry;
                    FInfoLink->Blink        = &FInfo->Entry;
                    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                }
                KdPrint(("    New File! %wZ (Encrypted: %d)\n", &FInfo->Name, FInfo->Enc));
            }
        }
    }

    FreeSortedList(&SortedListHead);
}


VOID
CheckFileIdFullDirectoryInformation(
                                    __in PFLT_FILTER Filter,
                                    __in PFLT_INSTANCE Instance,
                                    __in PFILE_ID_FULL_DIR_INFORMATION Info,
                                    __in BOOLEAN ProgramDir,
                                    __inout PNLDINFO DInfo
                                    )
{
    LIST_ENTRY  SortedListHead;
    ULONG       SortedListSize = 0;
    PLIST_ENTRY SortedLink     = NULL;
    PLIST_ENTRY FInfoLink      = NULL;
    BOOLEAN     CheckCache     = TRUE;
    KIRQL       OldIrql        = 0;

    InitializeListHead(&SortedListHead);
    SortedListSize = BuildSortedFileIdFullDirectoryInformationList(Info, &SortedListHead, ProgramDir);
    if(0 == SortedListSize)
        return;

    FInfoLink = DInfo->FInfoList.Flink;
    for(SortedLink=SortedListHead.Flink; (NULL!=SortedLink) && (SortedLink!=&SortedListHead); SortedLink=SortedLink->Flink)
    {
        PSORTEDINFOLISTITEM Item     = NULL;
        BOOLEAN             HitCache = FALSE;
        BOOLEAN             Encrypted= FALSE;
        PNLFINFO            FInfo    = NULL;
        UNICODE_STRING      FileName;
        Item = CONTAINING_RECORD(SortedLink, SORTEDINFOLISTITEM, Entry);
        
        FileName.Buffer = ((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileName;
        FileName.Length = (USHORT)((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileNameLength;
        FileName.MaximumLength = FileName.Length;

        while(FInfoLink!=NULL && FInfoLink != &(DInfo->FInfoList))
        {
            INT   CompareResult = 1;
            FInfo = NULL;
            FInfo = CONTAINING_RECORD(FInfoLink, NLFINFO, Entry);
            CompareResult = NLCompareName(FileName.Buffer, FileName.Length, FInfo->Name.Buffer, FInfo->Name.Length);
            if(CompareResult > 0)
            {
                // Input file name is smaller than current cache item
                // Move to next
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                continue;
            }
            else if(CompareResult == 0)
            {
                // Hit cache record
                HitCache   = TRUE;
                Encrypted  = FInfo->Enc;

                KdPrint(("    Hit File! %wZ (Encrypted: %d)\n", &FileName, Encrypted));
                if(Encrypted)
                {
                    FixDirInfoSizeData(Item->Info, FILE_ID_FULL_DIR_INFORMATION);
                }

                //
                // Point to next entry
                //
                KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                FInfoLink = FInfoLink->Flink;
                KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                break;
            }
            else
            {
                // Input file name is bigger than current cache item
                // Don't need to search again
                break;
            }
        }

        if(!HitCache)
        {
            Encrypted = IsFileEncrypted(Filter, Instance, &DInfo->Path, &FileName);
            if(Encrypted)
            {
                FixDirInfoSizeData(Item->Info, FILE_ID_FULL_DIR_INFORMATION);
            }
            FInfo = NLCreateFInfo(((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileName, (USHORT)(((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileNameLength));
            if(FInfo)
            {
                FInfo->Enc = Encrypted;

                if(FInfoLink==NULL || FInfoLink == &(DInfo->FInfoList))
                {
                    ExInterlockedInsertTailList(&DInfo->FInfoList, &FInfo->Entry, &DInfo->FInfoLock);
                }
                else
                {
                    //
                    // FInfoLink point to entry which is greater than current item
                    // Insert current item before FInfoLink
                    //
                    KeAcquireSpinLock(&DInfo->FInfoLock, &OldIrql);
                    FInfo->Entry.Flink      = FInfoLink;
                    FInfo->Entry.Blink      = FInfoLink->Blink;
                    FInfoLink->Blink->Flink = &FInfo->Entry;
                    FInfoLink->Blink        = &FInfo->Entry;
                    KeReleaseSpinLock(&DInfo->FInfoLock, OldIrql);
                }
                KdPrint(("    New File! %wZ (Encrypted: %d)\n", &FInfo->Name, FInfo->Enc));
            }
        }
    }

    FreeSortedList(&SortedListHead);
}


/*************************************************************************************
 *
 * Define Local Routine
 *
*************************************************************************************/
BOOLEAN
IsFileEncrypted(
                __in PFLT_FILTER Filter,
                __in PFLT_INSTANCE Instance,
                __in PCUNICODE_STRING ParentDir,
                __in PCUNICODE_STRING FileName
                )
{
    UNICODE_STRING FullName = {0, 0, NULL};
    BOOLEAN        Encrypted= FALSE;
    NTSTATUS       Status    = STATUS_SUCCESS;

    //
    // Get Full File Name
    //
    FullName.MaximumLength = ParentDir->Length + sizeof(WCHAR) + FileName->Length + sizeof(WCHAR);
    FullName.Buffer = ExAllocatePoolWithTag(NonPagedPool, FullName.MaximumLength, 'esln');
    if(NULL == FullName.Buffer)
        return FALSE;

    RtlZeroMemory(FullName.Buffer, FullName.MaximumLength);
    RtlCopyMemory(FullName.Buffer, ParentDir->Buffer, ParentDir->Length);         FullName.Length  = ParentDir->Length;
    RtlCopyMemory((PUCHAR)FullName.Buffer + FullName.Length, L"\\", sizeof(WCHAR)); FullName.Length += sizeof(WCHAR);
    RtlCopyMemory((PUCHAR)FullName.Buffer + FullName.Length, FileName->Buffer, FileName->Length);
    FullName.Length += FileName->Length;

    Status = NLSEIsEncryptedFile(Filter, Instance, &FullName, &Encrypted);
    ExFreePool(FullName.Buffer);
    return (NT_SUCCESS(Status) && Encrypted)?TRUE:FALSE;
}

VOID
FreeSortedList(
               __inout_opt PLIST_ENTRY ListHead
               )
{
    PLIST_ENTRY         Link = NULL;
    PSORTEDINFOLISTITEM Item = NULL;

    while(!IsListEmpty(ListHead))
    {
        Link = RemoveHeadList(ListHead); if(NULL==Link) break;
        Item = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
        if(NULL==Item) continue;
        ExFreePool(Item); Item = NULL;
    }
}


//
// For FileBothDirectoryInformation
//
VOID
InsertFileBothDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    )
{
    PLIST_ENTRY         Link      = NULL;
    PSORTEDINFOLISTITEM ItemPtr   = NULL;

    if(IsListEmpty(ListHead))
    {
        //
        // Empty list? insert it to tail
        //
        InsertTailList(ListHead, &(Item)->Entry); 
    } 
    else
    {
        //
        // Find: from last to first
        //
        for(Link=(ListHead)->Blink; (NULL!=Link) && (Link!=(ListHead)); Link=Link->Blink)
        {
            ItemPtr = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
            if(NULL==ItemPtr) continue;
            if(1 == NLCompareName( ((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileName,
                (USHORT)((PFILE_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength,
                ((PFILE_BOTH_DIR_INFORMATION)ItemPtr->Info)->FileName,
                (USHORT)((PFILE_BOTH_DIR_INFORMATION)ItemPtr->Info)->FileNameLength)
                ) 
            {
                //
                // Find first record which is smaller than new one
                //
                Link->Flink->Blink=&Item->Entry;
                Item->Entry.Flink = Link->Flink;
                Link->Flink       = &Item->Entry;
                Item->Entry.Blink = Link;
                return;
            } 
        }

        //
        // Input item is smallest, insert it to first
        //
        InsertHeadList(ListHead, &(Item)->Entry);
    }
}

ULONG
BuildSortedFileBothDirectoryInformationList(
    __in PFILE_BOTH_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    )
{
    ULONG   Count = 0;

    while(NULL != Info)
    {
        PSORTEDINFOLISTITEM Item = NULL;
        if(FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_SYSTEM)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_COMPRESSED)
            || (Info->EndOfFile.QuadPart < NLSE_ENVELOPE_SIZE)
            || (Info->FileNameLength/sizeof(WCHAR) > MAX_PATH)
            || (NULL==Info->FileName || 0==Info->FileNameLength)
            || IsIgnoredFiles(Info->FileName, Info->FileNameLength, IsProgramDir)
            )
        {
            Info = GetNextInfoEntry(Info, FILE_BOTH_DIR_INFORMATION);
            continue;
        }

#pragma prefast(disable:28197, "Possibly leaking memory preFast warning 28197 - memory Leak: it will released in \"FreeSortedList\"") 
        Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(SORTEDINFOLISTITEM), 'esln');
#pragma prefast(enable:28197, "Recover this warning")
        if(NULL == Item)
            break;
        InitializeListHead(&Item->Entry);
        Item->Info = Info;
        InsertFileBothDirectoryInformationToSortedList(ListHead, Item);
        ++Count;

        //
        // Move to next
        //
        Info = GetNextInfoEntry(Info, FILE_BOTH_DIR_INFORMATION);
    }

    return Count;
}

//
// For FileDirectoryInformation
//
VOID
InsertFileDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    )
{
    PLIST_ENTRY         Link      = NULL;
    PSORTEDINFOLISTITEM ItemPtr   = NULL;

    if(IsListEmpty(ListHead))
    {
        //
        // Empty list? insert it to tail
        //
        InsertTailList(ListHead, &(Item)->Entry); 
    } 
    else
    {
        //
        // Find: from last to first
        //
        for(Link=(ListHead)->Blink; (NULL!=Link) && (Link!=(ListHead)); Link=Link->Blink)
        {
            ItemPtr = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
            if(NULL==ItemPtr) continue;
            if(1 == NLCompareName( ((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileName,
                (USHORT)((PFILE_DIRECTORY_INFORMATION)Item->Info)->FileNameLength,
                ((PFILE_DIRECTORY_INFORMATION)ItemPtr->Info)->FileName,
                (USHORT)((PFILE_DIRECTORY_INFORMATION)ItemPtr->Info)->FileNameLength)
                ) 
            {
                //
                // Find first record which is smaller than new one
                //
                Link->Flink->Blink=&Item->Entry;
                Item->Entry.Flink = Link->Flink;
                Link->Flink       = &Item->Entry;
                Item->Entry.Blink = Link;
                return;
            } 
        }

        //
        // Input item is smallest, insert it to first
        //
        InsertHeadList(ListHead, &(Item)->Entry);
    }
}

ULONG
BuildSortedFileDirectoryInformationList(
    __in PFILE_DIRECTORY_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    )
{
    ULONG   Count = 0;

    while(NULL != Info)
    {
        PSORTEDINFOLISTITEM Item = NULL;
        if(FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_SYSTEM)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_COMPRESSED)
            || (Info->EndOfFile.QuadPart < NLSE_ENVELOPE_SIZE)
            || (Info->FileNameLength/sizeof(WCHAR) > MAX_PATH)
            || (NULL==Info->FileName || 0==Info->FileNameLength)
            || IsIgnoredFiles(Info->FileName, Info->FileNameLength, IsProgramDir)
            )
        {
            Info = GetNextInfoEntry(Info, FILE_DIRECTORY_INFORMATION);
            continue;
        }

#pragma prefast(disable:28197, "Possibly leaking memory preFast warning 28197 - memory Leak: it will released in \"FreeSortedList\"") 
        Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(SORTEDINFOLISTITEM), 'esln');
#pragma prefast(enable:28197, "Recover this warning")
        if(NULL == Item)
            break;
        InitializeListHead(&Item->Entry);
        Item->Info = Info;
        InsertFileDirectoryInformationToSortedList(ListHead, Item);
        ++Count;

        //
        // Move to next
        //
        Info = GetNextInfoEntry(Info, FILE_DIRECTORY_INFORMATION);
    }

    return Count;
}

//
// For FileFullDirectoryInformation
//
VOID
InsertFileFullDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    )
{
    PLIST_ENTRY         Link      = NULL;
    PSORTEDINFOLISTITEM ItemPtr   = NULL;

    if(IsListEmpty(ListHead))
    {
        //
        // Empty list? insert it to tail
        //
        InsertTailList(ListHead, &(Item)->Entry); 
    } 
    else
    {
        //
        // Find: from last to first
        //
        for(Link=(ListHead)->Blink; (NULL!=Link) && (Link!=(ListHead)); Link=Link->Blink)
        {
            ItemPtr = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
            if(NULL==ItemPtr) continue;
            if(1 == NLCompareName( ((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileName,
                (USHORT)((PFILE_FULL_DIR_INFORMATION)Item->Info)->FileNameLength,
                ((PFILE_FULL_DIR_INFORMATION)ItemPtr->Info)->FileName,
                (USHORT)((PFILE_FULL_DIR_INFORMATION)ItemPtr->Info)->FileNameLength)
                ) 
            {
                //
                // Find first record which is smaller than new one
                //
                Link->Flink->Blink=&Item->Entry;
                Item->Entry.Flink = Link->Flink;
                Link->Flink       = &Item->Entry;
                Item->Entry.Blink = Link;
                return;
            } 
        }

        //
        // Input item is smallest, insert it to first
        //
        InsertHeadList(ListHead, &(Item)->Entry);
    }
}

ULONG
BuildSortedFileFullDirectoryInformationList(
    __in PFILE_FULL_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    )
{
    ULONG   Count = 0;

    while(NULL != Info)
    {
        PSORTEDINFOLISTITEM Item = NULL;
        if(FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_SYSTEM)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_COMPRESSED)
            || (Info->EndOfFile.QuadPart < NLSE_ENVELOPE_SIZE)
            || (Info->FileNameLength/sizeof(WCHAR) > MAX_PATH)
            || (NULL==Info->FileName || 0==Info->FileNameLength)
            || IsIgnoredFiles(Info->FileName, Info->FileNameLength, IsProgramDir)
            )
        {
            Info = GetNextInfoEntry(Info, FILE_FULL_DIR_INFORMATION);
            continue;
        }

#pragma prefast(disable:28197, "Possibly leaking memory preFast warning 28197 - memory Leak: it will released in \"FreeSortedList\"") 
        Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(SORTEDINFOLISTITEM), 'esln');
#pragma prefast(enable:28197, "Recover this warning")
        if(NULL == Item)
            break;
        InitializeListHead(&Item->Entry);
        Item->Info = Info;
        InsertFileDirectoryInformationToSortedList(ListHead, Item);
        ++Count;

        //
        // Move to next
        //
        Info = GetNextInfoEntry(Info, FILE_FULL_DIR_INFORMATION);
    }

    return Count;
}

//
// For FileIdBothDirectoryInformation
//
VOID
InsertFileIdBothDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    )
{
    PLIST_ENTRY         Link      = NULL;
    PSORTEDINFOLISTITEM ItemPtr   = NULL;

    if(IsListEmpty(ListHead))
    {
        //
        // Empty list? insert it to tail
        //
        InsertTailList(ListHead, &(Item)->Entry); 
    } 
    else
    {
        //
        // Find: from last to first
        //
        for(Link=(ListHead)->Blink; (NULL!=Link) && (Link!=(ListHead)); Link=Link->Blink)
        {
            ItemPtr = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
            if(NULL==ItemPtr) continue;
            if(1 == NLCompareName( ((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileName,
                (USHORT)((PFILE_ID_BOTH_DIR_INFORMATION)Item->Info)->FileNameLength,
                ((PFILE_ID_BOTH_DIR_INFORMATION)ItemPtr->Info)->FileName,
                (USHORT)((PFILE_ID_BOTH_DIR_INFORMATION)ItemPtr->Info)->FileNameLength)
                ) 
            {
                //
                // Find first record which is smaller than new one
                //
                Link->Flink->Blink=&Item->Entry;
                Item->Entry.Flink = Link->Flink;
                Link->Flink       = &Item->Entry;
                Item->Entry.Blink = Link;
                return;
            } 
        }

        //
        // Input item is smallest, insert it to first
        //
        InsertHeadList(ListHead, &(Item)->Entry);
    }
}

ULONG
BuildSortedFileIdBothDirectoryInformationList(
    __in PFILE_ID_BOTH_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    )
{
    ULONG   Count = 0;

    while(NULL != Info)
    {
        PSORTEDINFOLISTITEM Item = NULL;
        if(FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_SYSTEM)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_COMPRESSED)
            || (Info->EndOfFile.QuadPart < NLSE_ENVELOPE_SIZE)
            || (Info->FileNameLength/sizeof(WCHAR) > MAX_PATH)
            || (NULL==Info->FileName || 0==Info->FileNameLength)
            || IsIgnoredFiles(Info->FileName, Info->FileNameLength, IsProgramDir)
            )
        {
            Info = GetNextInfoEntry(Info, FILE_ID_BOTH_DIR_INFORMATION);
            continue;
        }

#pragma prefast(disable:28197, "Possibly leaking memory preFast warning 28197 - memory Leak: it will released in \"FreeSortedList\"") 
        Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(SORTEDINFOLISTITEM), 'esln');
#pragma prefast(enable:28197, "Recover this warning")
        if(NULL == Item)
            break;
        InitializeListHead(&Item->Entry);
        Item->Info = Info;
        InsertFileDirectoryInformationToSortedList(ListHead, Item);
        ++Count;

        //
        // Move to next
        //
        Info = GetNextInfoEntry(Info, FILE_ID_BOTH_DIR_INFORMATION);
    }

    return Count;
}

//
// For FileIdFullDirectoryInformation
//
VOID
InsertFileIdFullDirectoryInformationToSortedList(
    __inout_opt PLIST_ENTRY ListHead,
    __inout PSORTEDINFOLISTITEM Item
    )
{
    PLIST_ENTRY         Link      = NULL;
    PSORTEDINFOLISTITEM ItemPtr   = NULL;

    if(IsListEmpty(ListHead))
    {
        //
        // Empty list? insert it to tail
        //
        InsertTailList(ListHead, &(Item)->Entry); 
    } 
    else
    {
        //
        // Find: from last to first
        //
        for(Link=(ListHead)->Blink; (NULL!=Link) && (Link!=(ListHead)); Link=Link->Blink)
        {
            ItemPtr = CONTAINING_RECORD(Link, SORTEDINFOLISTITEM, Entry);
            if(NULL==ItemPtr) continue;
            if(1 == NLCompareName( ((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileName,
                (USHORT)((PFILE_ID_FULL_DIR_INFORMATION)Item->Info)->FileNameLength,
                ((PFILE_ID_FULL_DIR_INFORMATION)ItemPtr->Info)->FileName,
                (USHORT)((PFILE_ID_FULL_DIR_INFORMATION)ItemPtr->Info)->FileNameLength)
                ) 
            {
                //
                // Find first record which is smaller than new one
                //
                Link->Flink->Blink=&Item->Entry;
                Item->Entry.Flink = Link->Flink;
                Link->Flink       = &Item->Entry;
                Item->Entry.Blink = Link;
                return;
            } 
        }

        //
        // Input item is smallest, insert it to first
        //
        InsertHeadList(ListHead, &(Item)->Entry);
    }
}

ULONG
BuildSortedFileIdFullDirectoryInformationList(
    __in PFILE_ID_FULL_DIR_INFORMATION Info,
    __inout_opt PLIST_ENTRY ListHead,
    __in BOOLEAN IsProgramDir
    )
{
    ULONG   Count = 0;

    while(NULL != Info)
    {
        PSORTEDINFOLISTITEM Item = NULL;
        if(FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_SYSTEM)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_ENCRYPTED)
            || FlagOn(Info->FileAttributes, FILE_ATTRIBUTE_COMPRESSED)
            || (Info->EndOfFile.QuadPart < NLSE_ENVELOPE_SIZE)
            || (Info->FileNameLength/sizeof(WCHAR) > MAX_PATH)
            || (NULL==Info->FileName || 0==Info->FileNameLength)
            || IsIgnoredFiles(Info->FileName, Info->FileNameLength, IsProgramDir)
            )
        {
            Info = GetNextInfoEntry(Info, FILE_ID_FULL_DIR_INFORMATION);
            continue;
        }

#pragma prefast(disable:28197, "Possibly leaking memory preFast warning 28197 - memory Leak: it will released in \"FreeSortedList\"") 
        Item = ExAllocatePoolWithTag(NonPagedPool, sizeof(SORTEDINFOLISTITEM), 'esln');
#pragma prefast(enable:28197, "Recover this warning")
        if(NULL == Item)
            break;
        InitializeListHead(&Item->Entry);
        Item->Info = Info;
        InsertFileDirectoryInformationToSortedList(ListHead, Item);
        ++Count;

        //
        // Move to next
        //
        Info = GetNextInfoEntry(Info, FILE_ID_FULL_DIR_INFORMATION);
    }

    return Count;
}