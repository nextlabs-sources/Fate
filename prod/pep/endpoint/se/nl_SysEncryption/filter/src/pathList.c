/*++

Module Name:
    pathList.c

Abstract:
    Functions used by the kernel mode filter driver implementing path
    list.

Environment:
    Kernel mode

--*/

#include "NLSEDef.h"
#include "NLSEStruct.h"
#include "pathList.h"



//
// Global variables
//

extern NL_KLOG nlseKLog;



//
// Internal functions
//

/** PathListAllocPathEntry
 *
 *  \brief Allocate one path entry to store the path and data.  The path entry
 *         is suitable for adding to a path list (although it is not already
 *         added by default.)
 *
 *  \param path (in)            Path for the entry.
 *  \param data (in)            Data assosicated with the path for the entry.
 *
 *  \return Pointer to path entry on success, or NULL
 *          on error.
 */
__checkReturn
__drv_allocatesMem(mem)
static PNL_PATH_ENTRY PathListAllocPathEntry(__in_z PCWCH       path,
                                             __in   ULONG_PTR   data)
{
  PNL_PATH_ENTRY pathEntry;
  USHORT len;

  // Allocate path entry.
  pathEntry = ExAllocatePoolWithTag(PagedPool, sizeof *pathEntry,
                                    NLSE_PATHENTRY_TAG);
  if (pathEntry == NULL)
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
                "NLSE!" __FUNCTION__ ": failed to allocate entry\n");
    goto exit;
  }

  // Find the length of the passed string.
  len = (USHORT)wcslen(path);

  // Allocate path name.
  pathEntry->Path.Length = len * sizeof(WCHAR);
  pathEntry->Path.MaximumLength = (len + 1) * sizeof(WCHAR);
  pathEntry->Path.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                 (len + 1) * sizeof(WCHAR),
                                                 NLSE_PATHNAME_TAG);
  if (pathEntry->Path.Buffer == NULL)
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
                "NLSE!" __FUNCTION__ ": failed to allocate path name\n");
    ExFreePoolWithTag(pathEntry, NLSE_PATHENTRY_TAG);
    pathEntry = NULL;
    goto exit;
  }

  // Store path name into path entry.
  RtlCopyMemory(pathEntry->Path.Buffer, path, (len + 1) * sizeof(WCHAR));

  // Store data
  pathEntry->Data = data;

exit:
  return pathEntry;
} /* PathListAllocPathEntry */

/** PathListFreePathEntry
 *
 *  \brief Free one path entry that is not in any path list.
 *
 *  \param pathEntry (in)       Path entry to free.
 */
static VOID PathListFreePathEntry(__in __drv_freesMem(mem)
                                  __in PNL_PATH_ENTRY pathEntry)
{
  ExFreePoolWithTag(pathEntry->Path.Buffer, NLSE_PATHNAME_TAG);
  ExFreePoolWithTag(pathEntry, NLSE_PATHENTRY_TAG);
} /* PathListFreePathEntry */



//
// Exported functions
//

__checkReturn
NTSTATUS PathListInit(__inout PNL_PATH_LIST     list,
                      __in    ULONG             maxCount)
{
  InitializeListHead(&list->ListHead);
  ExInitializeResourceLite(&list->Lock);
  list->MaxCount = maxCount;
  list->Count = 0;
  return STATUS_SUCCESS;
} /* PathListInit */

VOID PathListShutdown(__inout PNL_PATH_LIST list)
{
  PathListPurgePaths(list);
  ExDeleteResourceLite(&list->Lock);
} /* PathListShutdown */

NTSTATUS PathListSetPaths(__inout PNL_PATH_LIST list,
                          __in    ULONG         numPaths,
                          __in_z  PCWCH         paths,
                          __in    ULONG_PTR     data)
{
  PCWCH p;
  NTSTATUS status = STATUS_SUCCESS;
  ULONG i;

  // Even though PathListPurgePaths() and PathListAddPath() below lock and
  // unlock the list, we still need to lock it here so that the whole
  // "SetPaths" operation will be atomic.
  NLFSEAcquireResourceExclusive(&list->Lock, TRUE);

  PathListPurgePaths(list);

  p = paths;

  for (i = 0; i < numPaths; i++)
  {
    status = PathListAddPath(list, p, data, FALSE);
    if (!NT_SUCCESS(status))
    {
      break;
    }

    p += wcslen(p) + 1;
  }

  NLFSEReleaseResource(&list->Lock);

  return status;
} /* PathListSetPaths */

VOID PathListPurgePaths(__inout PNL_PATH_LIST list)
{
  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
              "NLSE!" __FUNCTION__ ": list %#p.\n",
              list);

  NLFSEAcquireResourceExclusive(&list->Lock, TRUE);

  while (list->Count > 0)
  {
    PLIST_ENTRY listEntry;
    PNL_PATH_ENTRY pathEntry;

    listEntry = RemoveHeadList(&list->ListHead);
    PathListFreePathEntry(CONTAINING_RECORD(listEntry, NL_PATH_ENTRY, Entry));
    list->Count--;
  }

  NLFSEReleaseResource(&list->Lock);
} /* PathListPurgePaths */

NTSTATUS PathListAddPath(__inout PNL_PATH_LIST  list, 
                         __in_z  PCWCH          path,
                         __in    ULONG_PTR      data,
                         __in    BOOLEAN        replaceOldestIfFull)
{
  PNL_PATH_ENTRY pathEntry;
  NTSTATUS status = STATUS_SUCCESS;

  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
              "NLSE!" __FUNCTION__ ": list %#p.  Adding (%ws, %#IX).\n",
              list, path, data);

  NLFSEAcquireResourceExclusive(&list->Lock, TRUE);

  // Make sure we can allocate a path entry first, before we check the list
  // count and possibly remove the oldest entry.
  pathEntry = PathListAllocPathEntry(path, data);

  if (pathEntry == NULL)
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto exit;
  }

  if (list->MaxCount != 0 && list->Count == list->MaxCount)
  {
    if (replaceOldestIfFull)
    {
      // Remove the oldest entry.
      PLIST_ENTRY listEntry = RemoveHeadList(&list->ListHead);
      PNL_PATH_ENTRY oldestPathEntry = CONTAINING_RECORD(listEntry,
                                                         NL_PATH_ENTRY, Entry);

      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_WARNING,
                  "NLSE!" __FUNCTION__ ": removing oldest path %wZ in order to add path %ws\n",
                  &oldestPathEntry->Path, path);
      PathListFreePathEntry(oldestPathEntry);
      list->Count--;
    }
    else
    {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
                  "NLSE!" __FUNCTION__ ": failed to add path %ws because list is full\n",
                  path);
      status = STATUS_INSUFFICIENT_RESOURCES;
      PathListFreePathEntry(pathEntry);
      goto exit;
    }
  }

  // Add path entry to list.
  InsertTailList(&list->ListHead, &pathEntry->Entry);
  list->Count++;

exit:
  NLFSEReleaseResource(&list->Lock);
  return status;
} /* PathListAddPath */

__checkReturn
NTSTATUS PathListFindPath(__inout  PNL_PATH_LIST                list,
                          __in     PCUNICODE_STRING             path,
                          __in     ULONG_PTR                    data,
                          __in     path_list_find_path_cb_t     cb,
                          __in     BOOLEAN                      removeIfFound,
                          __out    PBOOLEAN                     found)
{
  PLIST_ENTRY listEntry;
  PNL_PATH_ENTRY pathEntry;
  NTSTATUS status = STATUS_SUCCESS;

  *found = FALSE;

  if (removeIfFound)
  {
    NLFSEAcquireResourceExclusive(&list->Lock, TRUE);
  }
  else
  {
    NLFSEAcquireResourceShared(&list->Lock, TRUE);
  }

  // Walk through the list
  for (listEntry = list->ListHead.Flink;
       listEntry != &list->ListHead;
       listEntry = listEntry->Flink)
  {
    BOOLEAN match;

    pathEntry = CONTAINING_RECORD(listEntry, NL_PATH_ENTRY, Entry);

    status = (*cb)(path, data, &pathEntry->Path, pathEntry->Data, &match);
    if (!NT_SUCCESS(status))
    {
      break;
    }

    if (match)
    {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
                  "NLSE!" __FUNCTION__ ": list %#p.  (%wZ, %#IX) matches list entry (%wZ, %#IX).\n",
                  list, path, data, &pathEntry->Path, pathEntry->Data);

      *found = TRUE;

      if (removeIfFound)
      {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
                    "NLSE!" __FUNCTION__ ": list %#p.  Removing (%wZ, %#IX) from list.\n",
                    list, &pathEntry->Path, pathEntry->Data);

        RemoveEntryList(listEntry);
        PathListFreePathEntry(pathEntry);
        list->Count--;
      }

      break;
    }
  }

  NLFSEReleaseResource(&list->Lock);
  return status;
} /* PathListFindPath */
