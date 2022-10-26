#include "NLSEUtility.h"
#include "NLFileNameInfoCache.h"

static
VOID
NLFNInfoCacheItemDestroy(__in PNLFNINFO FNInfo)
{
  // Free node content.
  ExFreePoolWithTag(FNInfo->NormalizedFileName.Buffer,
                    NLSE_FILENAMEINFOCACHE_TAG);
  ExFreePoolWithTag(FNInfo->OpenedFileName.Buffer,
                    NLSE_FILENAMEINFOCACHE_TAG);

  // Free node.
  ExFreePoolWithTag(FNInfo, NLSE_FILENAMEINFOCACHE_TAG);
}

/* List lock must have already been acquired by caller. */
static
VOID
NLFNInfoCacheEnforceLimits(__inout PNLFNINFOLIST list)
{
  LARGE_INTEGER curTime;

  KeQuerySystemTime(&curTime);

  // Remove least recently added (not least recently used) nodes if cache size
  // is over size limit.
  // Remove all nodes whose lifetime are over time limit.
  while (list->Count > NL_FNINFO_CACHE_SIZE_LIMIT ||
         (!IsListEmpty(&list->ListHead) &&
          ((curTime.QuadPart -
            CONTAINING_RECORD(list->ListHead.Flink, NLFNINFO, Entry)
            ->AddedTime.QuadPart) / 10000)
          > NL_FNINFO_CACHE_TIME_LIMIT_MS))
  {
    PLIST_ENTRY FNInfoEntryOldest;
    PNLFNINFO FNInfoOldest;

    FNInfoEntryOldest = RemoveHeadList(&list->ListHead);
    list->Count--;
    FNInfoOldest = CONTAINING_RECORD(FNInfoEntryOldest, NLFNINFO, Entry);
    NLFNInfoCacheItemDestroy(FNInfoOldest);
  }
}

VOID
NLFNInfoCacheInit(__inout PNLFNINFOLIST list)
{
  InitializeListHead(&list->ListHead);
  list->Count = 0;
  ExInitializeFastMutex(&list->Lock);
}

VOID
NLFNInfoCacheDestroy(__inout PNLFNINFOLIST list)
{
  ExAcquireFastMutex(&list->Lock);

  while (list->Count > 0)
  {
    PLIST_ENTRY FNInfoEntry;
    PNLFNINFO FNInfo;

    // Remove one node.
    FNInfoEntry = RemoveHeadList(&list->ListHead);
    list->Count--;
    FNInfo = CONTAINING_RECORD(FNInfoEntry, NLFNINFO, Entry);
    NLFNInfoCacheItemDestroy(FNInfo);
  }

  ExReleaseFastMutex(&list->Lock);
}

NTSTATUS
NLFNInfoCacheAdd(__inout PNLFNINFOLIST list,
                 __in PUNICODE_STRING openedFileName,
                 __in PUNICODE_STRING fullFileName,
                 __in ULONG fileNameType)
{
  PNLFNINFO FNInfo = NULL;
  PWCHAR openedFileNameUpcaseBuf = NULL;
  PWCHAR fullFileNameBuf = NULL;

  // Allocate list node.
  FNInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof *FNInfo,
                                 NLSE_FILENAMEINFOCACHE_TAG);
  if (FNInfo == NULL)
  {
    goto outOfMem;
  }

  // Allocate buffers and copy filenames for node.
  openedFileNameUpcaseBuf = ExAllocatePoolWithTag
    (NonPagedPool, openedFileName->Length, NLSE_FILENAMEINFOCACHE_TAG);
  if (openedFileNameUpcaseBuf == NULL)
  {
    goto outOfMem;
  }
  RtlInitEmptyUnicodeString(&FNInfo->OpenedFileName, openedFileNameUpcaseBuf,
                            openedFileName->Length);
  NLUpcaseString(openedFileNameUpcaseBuf, openedFileName->Buffer,
                 openedFileName->Length);
  FNInfo->OpenedFileName.Length = openedFileName->Length;

  fullFileNameBuf = ExAllocatePoolWithTag
    (NonPagedPool, fullFileName->Length, NLSE_FILENAMEINFOCACHE_TAG);
  if (fullFileNameBuf == NULL)
  {
    goto outOfMem;
  }
  RtlInitEmptyUnicodeString(&FNInfo->NormalizedFileName, fullFileNameBuf,
                            fullFileName->Length);
  RtlCopyUnicodeString(&FNInfo->NormalizedFileName, fullFileName);

  // Store filename type in node.
  FNInfo->FileNameType = fileNameType;

  // Store time that file was added to cache.
  KeQuerySystemTime(&FNInfo->AddedTime);

  ExAcquireFastMutex(&list->Lock);

  // Add node to cache.
  InsertTailList(&list->ListHead, &FNInfo->Entry);
  list->Count++;

  // Remove over-limit nodes.
  NLFNInfoCacheEnforceLimits(list);

  ExReleaseFastMutex(&list->Lock);

  return STATUS_SUCCESS;

outOfMem:
  if (fullFileNameBuf != NULL)
  {
    ExFreePoolWithTag(fullFileNameBuf, NLSE_FILENAMEINFOCACHE_TAG);
  }
  if (openedFileNameUpcaseBuf != NULL)
  {
    ExFreePoolWithTag(openedFileNameUpcaseBuf, NLSE_FILENAMEINFOCACHE_TAG);
  }
  if (FNInfo != NULL)
  {
    ExFreePoolWithTag(FNInfo, NLSE_FILENAMEINFOCACHE_TAG);
  }

  return STATUS_INSUFFICIENT_RESOURCES;
}

NTSTATUS
NLFNInfoCacheFind(__in PNLFNINFOLIST list,
                  __in PUNICODE_STRING openedFileName,
                  __out PUNICODE_STRING fullFileName,
                  __out PULONG fileNameType)
{
  PLIST_ENTRY FNInfoEntry;
  PWCHAR openedFileNameUpcaseBuf = NULL;
  PWCHAR fullFileNameBuf = NULL;
  NTSTATUS status;

  ExAcquireFastMutex(&list->Lock);

  // Remove over-limit nodes.
  NLFNInfoCacheEnforceLimits(list);

  openedFileNameUpcaseBuf = ExAllocatePoolWithTag
    (NonPagedPool, openedFileName->Length, NLSE_FILENAMEINFOCACHE_TAG);
  if (openedFileNameUpcaseBuf == NULL)
  {
    goto outOfMem;
  }
  NLUpcaseString(openedFileNameUpcaseBuf, openedFileName->Buffer,
                 openedFileName->Length);

  // Loop through the nodes.  Start from the tail instead of the head, so
  // that we will check against the most recently added entries first.
  for (FNInfoEntry = list->ListHead.Blink;
       FNInfoEntry != &list->ListHead;
       FNInfoEntry = FNInfoEntry->Blink)
  {
    PNLFNINFO FNInfo;

    // See if node matches.
    FNInfo = CONTAINING_RECORD(FNInfoEntry, NLFNINFO, Entry);

    if (NLCompareNameCaseSensitive(openedFileNameUpcaseBuf,
                                   openedFileName->Length,
                                   FNInfo->OpenedFileName.Buffer,
                                   FNInfo->OpenedFileName.Length) == 0)
    {
      // Node matches.  Allocate buffers to return content.
      fullFileNameBuf = ExAllocatePoolWithTag
        (NonPagedPool, FNInfo->NormalizedFileName.Length,
         NLSE_FILENAMEINFOCACHE_TAG);
      if (fullFileNameBuf == NULL)
      {
        goto outOfMem;
      }

      RtlInitEmptyUnicodeString(fullFileName, fullFileNameBuf,
                                FNInfo->NormalizedFileName.Length);
      RtlCopyUnicodeString(fullFileName, &FNInfo->NormalizedFileName);

      *fileNameType = FNInfo->FileNameType;
      status = STATUS_SUCCESS;
      goto done;
    }
  }

  status = STATUS_NOT_FOUND;

done:
  ExFreePoolWithTag(openedFileNameUpcaseBuf, NLSE_FILENAMEINFOCACHE_TAG);

exit:
  ExReleaseFastMutex(&list->Lock);
  return status;

outOfMem:
  if (fullFileNameBuf != NULL)
  {
    ExFreePoolWithTag(fullFileNameBuf, NLSE_FILENAMEINFOCACHE_TAG);
  }
  if (openedFileNameUpcaseBuf != NULL)
  {
    ExFreePoolWithTag(openedFileNameUpcaseBuf, NLSE_FILENAMEINFOCACHE_TAG);
  }

  status = STATUS_INSUFFICIENT_RESOURCES;
  goto exit;
}
