/*++

Module Name:
    NLSEDrmPathList.c

Abstract:
    Functions used by the kernel mode filter driver implementing DRM path
    list.

Environment:
    Kernel mode

--*/

#include "NLSEDef.h"
#include "NLSEUtility.h"
#include "NLSEDrmPathList.h"



//
// Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;


typedef struct _NONDRMPATH {

    LIST_ENTRY      Entry;
    UNICODE_STRING  Path;

} NONDRMPATH, *PNONDRMPATH;

typedef struct _NONDRMDATA {

    ULONG       Flags;
    ULONG       Count;
    LIST_ENTRY  Paths;
    ERESOURCE   Lock;

} NONDRMDATA, *PNONDRMDATA;

#define NONDRMDATA_FLAG_READY           0x00000001
#define NONDRMDATA_FLAG_INITIALIZING    0x00010000


NONDRMDATA  NonDrmData = {FALSE};


//
//  Declare Local Routine
//
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
PNONDRMPATH
AllocNonDrmPath (
                 __in PSTRING Path
                 );

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
FreeNonDrmPath (
                __in PNONDRMPATH NonDrmPath
                );

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
ReadConfigFile (
                __in PFLT_FILTER Filter,
                __in PFLT_INSTANCE Instance,
                __in PUNICODE_STRING FileName,
                __deref_out PVOID* Data,
                __out PULONG DataSize
                );
__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxIsSameUnicodeString(
                      __in PCUNICODE_STRING Str1,
                      __in PCUNICODE_STRING Str2,
                      __in BOOLEAN CaseInSensitive
                      );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxPrefixUnicodeString(
                      __in PCUNICODE_STRING Prefix,
                      __in PCUNICODE_STRING Source,
                      __in BOOLEAN CaseInSensitive
                      );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxPrefixString(
               __in const STRING* Prefix,
               __in const STRING* Source,
               __in BOOLEAN CaseInSensitive
               );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NxInitNonDrmData)
#pragma alloc_text(PAGE, NxCleanupNonDrmData)
#pragma alloc_text(PAGE, NxReadNonDrmList)
#pragma alloc_text(PAGE, NxIsNonDrmDirectory)
#pragma alloc_text(PAGE, NxIsNonDrmDirectoryEx)
#pragma alloc_text(PAGE, AllocNonDrmPath)
#pragma alloc_text(PAGE, FreeNonDrmPath)
#pragma alloc_text(PAGE, ReadConfigFile)
#pragma alloc_text(PAGE, NxIsSameUnicodeString)
#pragma alloc_text(PAGE, NxPrefixUnicodeString)
#pragma alloc_text(PAGE, NxPrefixString)
#endif


//
// Internal functions
//

/** DrmPathListCheckCB
 *
 *  \brief Compare the path with a wildcard path in the DRM path
 *         list.
 *
 *  \param pathToFind (in)      Path to find.
 *  \param dataToFind (in)      Data for the path to find.  (Unused.)
 *  \param pathInList (in)      Path of one path entry in the list.
 *  \param dataInList (in)      Data for the path entry in the list.  (Unused.)
 *  \param match (out)          TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
static NTSTATUS DrmPathListCheckCB(
    __in  PCUNICODE_STRING      pathToFind,
    __in  ULONG_PTR             dataToFind,
    __in  PCUNICODE_STRING      pathInList,
    __in  ULONG_PTR             dataInList,
    __out PBOOLEAN              match)
{
  if (RtlEqualUnicodeString(pathToFind, pathInList, TRUE))
  {
    *match = TRUE;
    return STATUS_SUCCESS;
  }
  else
  {
    return NLSEIsPathMatchingWildcardPath(pathToFind, pathInList, TRUE, match);
  }
} /* DrmPathListCheckCB */



//
// Exported functions
//

__checkReturn
NTSTATUS NLSEDrmPathListInit(VOID)
{
  return PathListInit(&nlfseGlobal.drmPathList, 0);
}

VOID NLSEDrmPathListShutdown(VOID)
{
  PathListShutdown(&nlfseGlobal.drmPathList);
}

NTSTATUS NLSEDrmPathListSet(__in   ULONG numPaths,
                            __in_z PCWCH paths)
{
  // Since we don't use data, we pass a dummy data value of 0.
  return PathListSetPaths(&nlfseGlobal.drmPathList, numPaths, paths, 0);
}

__checkReturn
NTSTATUS NLSEDrmPathListCheck(__in  PCUNICODE_STRING    path,
                              __out PBOOLEAN            result)
{
  UNICODE_STRING pathNoPrefix;

  // Skip the prefix, if any.
  pathNoPrefix = *path;
  if (pathNoPrefix.Length > 4 * sizeof(WCHAR) &&
      pathNoPrefix.Buffer[0] == L'\\' &&
      pathNoPrefix.Buffer[1] == L'?' &&
      pathNoPrefix.Buffer[2] == L'?' &&
      pathNoPrefix.Buffer[3] == L'\\')
  {
    pathNoPrefix.Buffer += 4;
    pathNoPrefix.Length -= 4 * sizeof(WCHAR);
    pathNoPrefix.MaximumLength -= 4 * sizeof(WCHAR);
  }

  // Since we don't use data, we pass a dummy data value of 0.
  return PathListFindPath(&nlfseGlobal.drmPathList, &pathNoPrefix, 0,
                          DrmPathListCheckCB, FALSE, result);
}


__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxInitNonDrmData (
                  )
{
    NTSTATUS    Status;

    PAGED_CODE();

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_READY))
        return STATUS_SUCCESS;

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING))
        return STATUS_DEVICE_BUSY;

    SetFlag(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING);

    NonDrmData.Count = 0;
    InitializeListHead(&NonDrmData.Paths);
    Status = ExInitializeResourceLite(&NonDrmData.Lock);
    if(NT_SUCCESS(Status))
    {
        SetFlag(NonDrmData.Flags, NONDRMDATA_FLAG_READY);
    }

    ClearFlag(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING);
    return Status;
}

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxCleanupNonDrmData (
                     )
{
    NTSTATUS Status;

    PAGED_CODE();

    if(!BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_READY))
        return STATUS_SUCCESS;

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING))
        return STATUS_DEVICE_BUSY;

    SetFlag(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING);

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&NonDrmData.Lock, TRUE);

    // Remove all the entries in the list
    while (!IsListEmpty(&NonDrmData.Paths))
    {
        PLIST_ENTRY Entry;
        PNONDRMPATH NonDrmPath;

        ASSERT(NonDrmData.Count > 0);

        Entry = RemoveHeadList(&NonDrmData.Paths);
        ASSERT(NULL != Entry);
        if(NULL == Entry)
            break;

        NonDrmData.Count --;

        NonDrmPath = CONTAINING_RECORD(Entry, NONDRMPATH, Entry);
        ASSERT(NULL != NonDrmPath);
        if(NULL == NonDrmPath)
            continue;

        FreeNonDrmPath(NonDrmPath);
    }


    ExReleaseResourceLite(&NonDrmData.Lock);
    KeLeaveCriticalRegion();

    // Delete the resource
    ExDeleteResourceLite(&NonDrmData.Lock);
    NonDrmData.Count = 0;

    ClearFlag(NonDrmData.Flags, NONDRMDATA_FLAG_READY);
    ClearFlag(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING);
    return STATUS_SUCCESS;
}

//
//  The config file must be in Windows's format or Unix format
//  which means the line ending must be "\0x0D\0x0A" or "\0x0A"
//
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxReadNonDrmList (
                  __in PFLT_FILTER Filter,
                  __in_opt PFLT_INSTANCE Instance,
                  __in PUNICODE_STRING FileName
                  )
{
    NTSTATUS            Status;
    LIST_ENTRY          NewList;
    PCHAR               Data;
    ULONG               DataSize;
    ULONG               Offset = 0;
    const STRING        NonDrmName = {10, 10, "NONDRMDIR="};

    static PFLT_INSTANCE LocalInstance = NULL;


    PAGED_CODE();

    
    if(!BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_READY))
        return STATUS_UNSUCCESSFUL;

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING))
        return STATUS_DEVICE_BUSY;


    if(NULL==LocalInstance && NULL!=Instance)
        LocalInstance = Instance;

    InitializeListHead(&NewList);

    Status = ReadConfigFile ( Filter,
                              LocalInstance,
                              FileName,
                              &Data,
                              &DataSize);
    if(!NT_SUCCESS(Status))
    {
        return Status;
    }



    // Get all valid NonDrmPath, and put them to new list
    while (Offset < DataSize)
    {
        ULONG   EndLineOffset = Offset;
        ULONG   LineLength    = 0;
        STRING  Line;
        PNONDRMPATH NonDrmPath;

        // try to find line ending
        while (EndLineOffset<(DataSize-1) && 0x0A!=Data[EndLineOffset])
            ++EndLineOffset;

        if(0x0A != Data[EndLineOffset])
        {
            // Read end of the data
            ASSERT(EndLineOffset == (DataSize-1));
            Line.Buffer = &Data[Offset];
            Line.Length = (USHORT)(EndLineOffset + 1 - Offset);
            Line.MaximumLength = Line.Length;

            Offset = (++EndLineOffset);
        }
        else
        {
            // Good, find line ending
            Line.Buffer = &Data[Offset];
            Line.Length = (USHORT)(EndLineOffset - Offset);                     // Originally assume line ending is "\0x0A"
            if( (EndLineOffset > Offset) && (0x0D == Data[EndLineOffset-1]) )   // Check if the line ending is "\0x0D\0x0A"
                Line.Length --;
            Line.MaximumLength = Line.Length;

            Offset = ++EndLineOffset;
        }

        // Is it a valid line
        if(Line.Length <= NonDrmName.Length)
            continue;

        // Not start with "NONDRMDIR="
        if(!NxPrefixString(&NonDrmName, &Line, TRUE))
            continue;

        // Remove header "NONDRMDIR="
        Line.Buffer += NonDrmName.Length;
        Line.Length -= NonDrmName.Length;
        Line.MaximumLength = Line.Length;
        RtlUpperString(&Line, &Line);

        NonDrmPath = AllocNonDrmPath(&Line);
        if(NULL != NonDrmPath)
        {
            InsertTailList(&NewList, &NonDrmPath->Entry);
        }            
    }

    //
    // Now we have a valid new list
    //
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&NonDrmData.Lock, TRUE);

    try {

        // Remove all the items in old list
        while (!IsListEmpty(&NonDrmData.Paths))
        {
            PLIST_ENTRY Entry;
            PNONDRMPATH NonDrmPath;

            ASSERT(NonDrmData.Count > 0);

            Entry = RemoveHeadList(&NonDrmData.Paths);
            ASSERT(NULL != Entry);
            if(NULL == Entry)
                break;

            NonDrmData.Count --;

            NonDrmPath = CONTAINING_RECORD(Entry, NONDRMPATH, Entry);
            ASSERT(NULL != NonDrmPath);
            if(NULL == NonDrmPath)
                continue;

            FreeNonDrmPath(NonDrmPath);
        }

        // Add data in new list to old list
        while (!IsListEmpty(&NewList))
        {
            PLIST_ENTRY Entry;
            PNONDRMPATH NonDrmPath;

            Entry = RemoveHeadList(&NewList);
            ASSERT(NULL != Entry);
            if(NULL == Entry)
                break;

            InsertTailList(&NonDrmData.Paths, Entry);
            NonDrmData.Count ++;
        }

    }
    finally {

        // Unlock
        ExReleaseResourceLite(&NonDrmData.Lock);
        KeLeaveCriticalRegion();
    }

    return Status;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NxIsNonDrmDirectoryEx (
                       __in WCHAR Drive,
                       __in PFLT_FILE_NAME_INFORMATION FileNameInfo,
                       __in PBOOLEAN IsNonDrm
                       )
{
    PLIST_ENTRY     Link;
    UNICODE_STRING  FileName;

    PAGED_CODE();

    *IsNonDrm = FALSE;
    
    if(!BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_READY))
        return STATUS_UNSUCCESSFUL;

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING))
        return STATUS_DEVICE_BUSY;

    //
    //  Following file must be NonDRM
    //
    if( 0 != FileNameInfo->Share.Length ||                      // Remote file/directory
        0 == FileNameInfo->Volume.Length ||                     // Not local
        0 != FileNameInfo->Stream.Length ||                     // ADS
        ( sizeof(WCHAR)==FileNameInfo->ParentDir.Length &&      // Root
          L'\\'==FileNameInfo->ParentDir.Buffer[0] &&
          0==FileNameInfo->FinalComponent.Length )
       )
    {
        *IsNonDrm = TRUE;
        return STATUS_SUCCESS;
    }

    FileName.Buffer = FileNameInfo->Name.Buffer + FileNameInfo->Volume.Length / sizeof(WCHAR);
    FileName.Length = FileNameInfo->Name.Length - FileNameInfo->Volume.Length;
    FileName.MaximumLength = FileName.Length;

    return NxIsNonDrmDirectory (Drive, &FileName, IsNonDrm);
}


__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxIsSameUnicodeString(
                      __in PCUNICODE_STRING Str1,
                      __in PCUNICODE_STRING Str2,
                      __in BOOLEAN CaseInSensitive
                      )
{
    USHORT  Size;
    USHORT  i;

    PAGED_CODE();

    if(Str1->Length != Str2->Length)
        return FALSE;

    Size = Str1->Length / sizeof(WCHAR);

    for(i=0; i<Size; i++)
    {
        if(CaseInSensitive)
        {
            if(RtlUpcaseUnicodeChar(Str1->Buffer[i]) != RtlUpcaseUnicodeChar(Str2->Buffer[i]))
                return FALSE;
        }
        else
        {
            if(Str1->Buffer[i] != Str2->Buffer[i])
                return FALSE;
        }
    }

    return TRUE;
}


__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxPrefixUnicodeString(
                      __in PCUNICODE_STRING Prefix,
                      __in PCUNICODE_STRING Source,
                      __in BOOLEAN CaseInSensitive
                      )
{
    USHORT  Size;
    USHORT  i;

    PAGED_CODE();

    if(Source->Length < Prefix->Length)
        return FALSE;

    Size = Prefix->Length / sizeof(WCHAR);

    for(i=0; i<Size; i++)
    {
        if(CaseInSensitive)
        {
            if(RtlUpcaseUnicodeChar(Prefix->Buffer[i]) != RtlUpcaseUnicodeChar(Source->Buffer[i]))
                return FALSE;
        }
        else
        {
            if(Prefix->Buffer[i] != Source->Buffer[i])
                return FALSE;
        }
    }

    return TRUE;
}



__checkReturn
__drv_maxIRQL(APC_LEVEL)
BOOLEAN
NxPrefixString(
               __in const STRING* Prefix,
               __in const STRING* Source,
               __in BOOLEAN CaseInSensitive
               )
{
    USHORT  Size;
    USHORT  i;

    PAGED_CODE();

    if(Source->Length < Prefix->Length)
        return FALSE;

    Size = Prefix->Length;

    for(i=0; i<Size; i++)
    {
        if(CaseInSensitive)
        {
            if(RtlUpperChar(Prefix->Buffer[i]) != RtlUpperChar(Source->Buffer[i]))
                return FALSE;
        }
        else
        {
            if(Prefix->Buffer[i] != Source->Buffer[i])
                return FALSE;
        }
    }

    return TRUE;
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NxIsNonDrmDirectory (
                     __in WCHAR Drive,
                     __in PCUNICODE_STRING FileName, // File path without volume information
                     __in PBOOLEAN IsNonDrm
                     )
{
    PLIST_ENTRY Link;

    PAGED_CODE();

    *IsNonDrm = FALSE;
    
    if(!BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_READY))
        return STATUS_UNSUCCESSFUL;

    if(BooleanFlagOn(NonDrmData.Flags, NONDRMDATA_FLAG_INITIALIZING))
        return STATUS_DEVICE_BUSY;

    Drive = RtlUpcaseUnicodeChar(Drive);
    
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(&NonDrmData.Lock, TRUE);

    // Search
    for (Link=NonDrmData.Paths.Flink; Link!=&NonDrmData.Paths; Link=Link->Flink)
    {
        PNONDRMPATH     NonDrmPath;
        UNICODE_STRING  NonDrmName;

        NonDrmPath = CONTAINING_RECORD(Link, NONDRMPATH, Entry);
        ASSERT(NULL != NonDrmPath);
        if(NULL == NonDrmPath)
            continue;

        if(Drive != NonDrmPath->Path.Buffer[0])
            continue;

        // Remove drive letter "C:"
        NonDrmName.Buffer = NonDrmPath->Path.Buffer + 2;
        NonDrmName.Length = NonDrmPath->Path.Length - 2*sizeof(WCHAR);
        NonDrmName.MaximumLength = NonDrmName.Length;

        if(FileName->Length == NonDrmName.Length)
        {
            if(NxIsSameUnicodeString(FileName, &NonDrmName, TRUE))
            {
                *IsNonDrm = TRUE;
            }
        }
        else if(FileName->Length > NonDrmName.Length)
        {
            if( (L'\\' == FileName->Buffer[NonDrmName.Length/sizeof(WCHAR)]) &&
                NxPrefixUnicodeString(&NonDrmName, FileName, TRUE)
               )
            {
                *IsNonDrm = TRUE;
            }
        }

        if(*IsNonDrm)
            break;
    }

    ExReleaseResourceLite(&NonDrmData.Lock);
    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}


__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
PNONDRMPATH
AllocNonDrmPath (
                 __in PSTRING Path
                 )
{
    NTSTATUS    Status;
    PNONDRMPATH NonDrmPath = NULL;

    PAGED_CODE();

    if(Path->Length == 0 || Path->Buffer == NULL)
        return NULL;

    NonDrmPath = (PNONDRMPATH)ExAllocatePoolWithTag(NonPagedPool, sizeof(NONDRMPATH), 'pnES');
    if(NULL == NonDrmPath)
        return NULL;

    RtlZeroMemory(NonDrmPath, sizeof(NONDRMPATH));
    InitializeListHead(&NonDrmPath->Entry);

    Status = RtlAnsiStringToUnicodeString(&NonDrmPath->Path, Path, TRUE);
    if(!NT_SUCCESS(Status))
    {
        ExFreePool(NonDrmPath);
        NonDrmPath = NULL;
        return NULL;
    }

    return NonDrmPath;
}


__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
FreeNonDrmPath (
                __in_opt PNONDRMPATH NonDrmPath
                )
{
    PAGED_CODE();

    if(NULL == NonDrmPath)
        return;

    if(NonDrmPath->Path.Length > 0 && NonDrmPath->Path.Buffer != NULL)
        RtlFreeUnicodeString(&NonDrmPath->Path);

    ExFreePool(NonDrmPath);
}

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
ReadConfigFile (
                __in PFLT_FILTER Filter,
                __in PFLT_INSTANCE Instance,
                __in PUNICODE_STRING FileName,
                __deref_out PVOID* Data,
                __out PULONG DataSize
                )
{
    NTSTATUS            Status;
    HANDLE              FileHandle;
    PFILE_OBJECT        FileObject;
    OBJECT_ATTRIBUTES   ObjectAttrs;
    IO_STATUS_BLOCK     Iosb;
    FILE_STANDARD_INFORMATION   StandardInfo;
    PVOID               Buffer;
    ULONG               BytesToRead = 0;
    ULONG               BytesRead   = 0;
    LARGE_INTEGER       LiZero;



    PAGED_CODE();


    LiZero.QuadPart = 0;
    *Data     = NULL;
    *DataSize = 0;

    InitializeObjectAttributes( &ObjectAttrs,
                                FileName,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    Status = FltCreateFile( Filter,
                            Instance,
                            &FileHandle,
                            GENERIC_READ,
                            &ObjectAttrs,
                            &Iosb,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                            FILE_OPEN,
                            FILE_NON_DIRECTORY_FILE,
                            NULL,
                            0,
                            IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
    {
        return Status;
    }

    Status = ObReferenceObjectByHandle( FileHandle,
                                        GENERIC_READ,
                                        *IoFileObjectType,
                                        KernelMode,
                                        &FileObject,
                                        NULL);
    if(!NT_SUCCESS(Status))
    {
        FltClose(FileHandle);
        return Status;
    }



    Status = FltQueryInformationFile( Instance,
                                      FileObject,
                                      &StandardInfo,
                                      sizeof(FILE_STANDARD_INFORMATION),
                                      FileStandardInformation,
                                      NULL);
    if(!NT_SUCCESS(Status))
    {
        ObDereferenceObject(FileObject);
        FltClose(FileHandle);
        return Status;
    }

    BytesToRead = StandardInfo.EndOfFile.LowPart;

    if(0 == BytesToRead)
    {
        // Empty file
        ObDereferenceObject(FileObject);
        FltClose(FileHandle);
        return STATUS_UNSUCCESSFUL;
    }

    Buffer = ExAllocatePoolWithTag(PagedPool, BytesToRead, 'pnES');
    if(NULL == Buffer)
    {
        ObDereferenceObject(FileObject);
        FltClose(FileHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(Buffer, BytesToRead);
    Status = FltReadFile( Instance,
                          FileObject,
                          &LiZero,
                          BytesToRead,
                          Buffer,
                          0,
                          &BytesRead,
                          NULL,
                          NULL);
    // At last, close file
    ObDereferenceObject(FileObject);
    FltClose(FileHandle);
    if(!NT_SUCCESS(Status))
    {
        ExFreePool(Buffer); Buffer = NULL;
        return Status;
    }

    *Data     = Buffer;
    *DataSize = BytesRead;
    return STATUS_SUCCESS;
}
