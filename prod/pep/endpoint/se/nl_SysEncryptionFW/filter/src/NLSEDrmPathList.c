/*++

Module Name:
    NLSEDrmPathList.c

Abstract:
    Functions used by the kernel mode filter driver implementing DRM path
    list.

Environment:
    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEDef.h"
#include "NLSEDrmPathList.h"



//
// Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;

DECLARE_CONST_UNICODE_STRING(wildcardStr, L"*");



//
// Internal functions
//

/** getNextPathComponent
 *
 *  \brief Get the next component in the path, starting at the passed index.
 *
 *  \param path (in)        Path from which to extract the next component
 *  \param index (inout)    Pointer to index.  On entry, it contains the index
 *                          at which the next compoment is to be eextracted.
 *  \param component (out)  String to store the next component, or empty string
 *                          if the next component is empty (e.g.
 *                          C:\Dir1\Dir2\\Dir4).  Caller needs to free the
 *                          memory allocated for this string.
 *
 *  \return TRUE on success (including the case where the next component is
 *                          empty), otherwise FALSE.
 */
static BOOLEAN getNextPathComponent(__in PCUNICODE_STRING path,
                                    __inout USHORT *index,
                                    __out PUNICODE_STRING component)
{
  USHORT i;

  if (*index >= path->Length / sizeof(WCHAR))
  {
    // Invalid parameter error.
    return FALSE;
  }

  for (i = *index; i < path->Length / sizeof(WCHAR); i++)
  {
    if (path->Buffer[i] == L'\\')
    {
      break;
    }
  }

  // We have reached either a '\' or the end of the string.  Return the
  // (maybe empty) component.

  if (i == *index)
  {
    // Component is empty.  Return empty component.
    RtlInitUnicodeString(component, NULL);
    return TRUE;
  }

  // Return non-empty component.
  component->Buffer = ExAllocatePoolWithTag(PagedPool,
                                            (i - *index) * sizeof(WCHAR),
                                            NLSE_DRMPATHNAME_TAG);
  if (component->Buffer == NULL)
  {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                "NLSE!getNextPathComponent: failed to allocate component\n");
    return FALSE;
  }
  component->Length = (i - *index) * sizeof(WCHAR);
  component->MaximumLength = component->Length;
  RtlCopyMemory(component->Buffer, &path->Buffer[*index],
                (i - *index) * sizeof(WCHAR));
  *index = i;

  return TRUE;
}

/** isPathMatchingWildcardPath
 *
 *  \brief Check is the path matches a path containing wildcards.
 *
 *  \param path (in)            The path to check.
 *  \param wildcardPath (in)    The wildcard of which to check against.
 *  \param caseInSensitive (in) TRUE if ignore case.
 *
 *  \return TRUE if match, FALSE if not match or error.
 */
static BOOLEAN isPathMatchingWildcardPath(__in PCUNICODE_STRING path,
                                          __in PCUNICODE_STRING wildcardPath,
                                          __in BOOLEAN caseInSensitive)
{
  USHORT index1 = 0, index2 = 0;

  while (1)
  {
    BOOLEAN endReached1, endReached2;
    UNICODE_STRING component1, component2;
    BOOLEAN componentMatch;

    endReached1 = (index1 >= path->Length / sizeof(WCHAR));
    endReached2 = (index2 >= wildcardPath->Length / sizeof(WCHAR));

    if (endReached1 && endReached2)
    {
      // Both paths have reached the end.  The paths match.
      return TRUE;
    }
    else if (endReached1 || endReached2)
    {
      // One, but not both, path has reached the end.  The paths don't match.
      return FALSE;
    }

    // Neither paths have reached the end.  See if the components between the
    // two paths match.

    if (!getNextPathComponent(path, &index1, &component1))
    {
      // Error.
      return FALSE;
    }

    if (!getNextPathComponent(wildcardPath, &index2, &component2))
    {
      // Error.
      if (component1.Buffer != NULL)
      {
        ExFreePoolWithTag(component1.Buffer, NLSE_DRMPATHNAME_TAG);
      }
      return FALSE;
    }

    componentMatch =
      (RtlEqualUnicodeString(&component2, &wildcardStr, caseInSensitive) ||
       RtlEqualUnicodeString(&component2, &component1, caseInSensitive));

    // Free the components if they are not empty.
    if (component2.Buffer != NULL)
    {
      ExFreePoolWithTag(component2.Buffer, NLSE_DRMPATHNAME_TAG);
    }
    if (component1.Buffer != NULL)
    {
      ExFreePoolWithTag(component1.Buffer, NLSE_DRMPATHNAME_TAG);
    }

    if (!componentMatch)
    {
      // This component doesn't match.  The paths don't match.
      return FALSE;
    }

    // Skip the component separator (either a '\' or the wchar after the last
    // wchar in the paths).
    index1++;
    index2++;
  }
}

/** drmPathListPurgePaths
 *
 *  \brief  Purge (free) the existing paths in the DRM path list.  The caller
 *          needs to have already acquired the list lock.
 */
static void drmPathListPurgePaths(void)
{
  while (!IsListEmpty(&nlfseGlobal.drmPathList))
  {
    PLIST_ENTRY entry;
    PNLFSE_DRM_PATH_ENTRY pDrmPath;

    entry = RemoveHeadList(&nlfseGlobal.drmPathList);
    pDrmPath = CONTAINING_RECORD(entry, NLFSE_DRM_PATH_ENTRY, listEntry);
    ExFreePoolWithTag(pDrmPath->path.Buffer, NLSE_DRMPATHNAME_TAG);
    ExFreePoolWithTag(pDrmPath, NLSE_DRMPATHENTRY_TAG);
  }
}

/** drmPathListAddPaths
 *
 *  \brief  Add the passed paths to the DRM path list.  The caller needs to
 *          have already acquired the list lock.
 *
 *  \param numPaths (in)    # of paths to add.
 *  \param paths (in)       Blob of paths separated by null-terminators.
 *
 *  \return STATUS_SUCCESS on success.
 */
static NTSTATUS drmPathListAddPaths(__in int numPaths,
                                    __in_z const WCHAR *paths)
{
  int i;
  const WCHAR* Path = paths;

  for (i = 0; i < numPaths; i++)
  {
    PNLFSE_DRM_PATH_ENTRY pDrmPath;
    USHORT len;

    // Allocate path entry.
    pDrmPath = ExAllocatePoolWithTag(PagedPool, sizeof *pDrmPath,
                                     NLSE_DRMPATHENTRY_TAG);
    if (pDrmPath == NULL)
    {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                  "NLSE!drmPathListAddPaths: failed to allocate entry\n");
      return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Find the length of the passed string.
    len = 0;
    len = (USHORT)wcslen(Path);

    // Allocate path name.
    pDrmPath->path.Length = len * sizeof(WCHAR);
    pDrmPath->path.MaximumLength = (len + 1) * sizeof(WCHAR);
    pDrmPath->path.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                  (len + 1) * sizeof(WCHAR),
                                                  NLSE_DRMPATHNAME_TAG);
    if (pDrmPath->path.Buffer == NULL)
    {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
                  "NLSE!drmPathListAddPaths: failed to allocate path name\n");
      ExFreePoolWithTag(pDrmPath, NLSE_DRMPATHENTRY_TAG);
      return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Store path name into path entry.
    RtlCopyMemory(pDrmPath->path.Buffer, Path, (len + 1) * sizeof(WCHAR));
    RtlUpcaseUnicodeString(&pDrmPath->path, &pDrmPath->path, FALSE);

    // Make sure the path doesn't end with "\\"
    if(pDrmPath->path.Buffer[pDrmPath->path.Length/sizeof(WCHAR) - 1] == L'\\')
        pDrmPath->path.Length -= sizeof(WCHAR);

    // Add path entry to list.
    InsertTailList(&nlfseGlobal.drmPathList, &pDrmPath->listEntry);

    // Move to next
    Path += (len + 1);
  }

  return STATUS_SUCCESS;
}



//
// Exported functions
//

/** NLSEDrmPathListInit
 *
 *  \brief  Initialize the DRM path list.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmPathListInit(void)
{
  InitializeListHead(&nlfseGlobal.drmPathList);
  ExInitializeResourceLite(&nlfseGlobal.drmPathListLock);

  return STATUS_SUCCESS;
}

/** NLSEDrmPathListShutdown
 *
 *  \brief  Shutdown the DRM path list.
 */
void NLSEDrmPathListShutdown(void)
{
  ExDeleteResourceLite(&nlfseGlobal.drmPathListLock);
  drmPathListPurgePaths();
}

/** NLSEDrmPathlistSet
 *
 *  \brief  Replace the stored DRM path list with the passed one.
 *
 *  \param numPaths (in)    # of paths.
 *  \param paths (in)       Blob of paths separated by null-terminators.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmPathListSet(__in int numPaths,
                            __in_z const WCHAR *paths)
{
  NTSTATUS status;

  NLFSEAcquireResourceExclusive(&nlfseGlobal.drmPathListLock, TRUE);
  drmPathListPurgePaths();
  status = drmPathListAddPaths(numPaths, paths);
  NLFSEReleaseResource(&nlfseGlobal.drmPathListLock);

  return status;
}

/** NLSEDrmPathListCheckPath
 *
 *  \brief   See if the passed path matches any path in the list.
 *
 *  \param path (in)        Path to check.
 *  \param result (out)     TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmPathListCheckPath(__in PCUNICODE_STRING path,
                                  __out BOOLEAN *result)
{
  UNICODE_STRING pathNoPrefix;
  PLIST_ENTRY entry;
  PNLFSE_DRM_PATH_ENTRY pDrmPath;

  *result = FALSE;

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

  NLFSEAcquireResourceShared(&nlfseGlobal.drmPathListLock, TRUE);

  // Get head of list, if any.
  entry = nlfseGlobal.drmPathList.Flink;

  // Stop if end of list is reached.
  while (entry != &nlfseGlobal.drmPathList)
  {
    pDrmPath = CONTAINING_RECORD(entry, NLFSE_DRM_PATH_ENTRY, listEntry);

    if (RtlEqualUnicodeString(&pathNoPrefix, &pDrmPath->path, TRUE) ||
        isPathMatchingWildcardPath(&pathNoPrefix, &pDrmPath->path, TRUE))
    {
      *result = TRUE;
      break;
    }

    // Get next list entry.
    entry = entry->Flink;
  }

  NLFSEReleaseResource(&nlfseGlobal.drmPathListLock);

  return STATUS_SUCCESS;
}


__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEInDRMDirectory (
                    __in WCHAR DriveLetter,
                    __in PUNICODE_STRING Path,
                    __out PBOOLEAN IsDRM
                    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PLIST_ENTRY     Entry;


    *IsDRM = FALSE;

    if(Path->Length == 0)
        return STATUS_INVALID_PARAMETER;

    // Upcase drive letetr
    DriveLetter = RtlUpcaseUnicodeChar(DriveLetter);

    // Get shared lock
    NLFSEAcquireResourceShared(&nlfseGlobal.drmPathListLock, TRUE);

    // Get head of list, if any.
    Entry = nlfseGlobal.drmPathList.Flink;

    // Stop if end of list is reached.
    while (Entry != &nlfseGlobal.drmPathList)
    {
        PNLFSE_DRM_PATH_ENTRY   DrmPathEntry;
        UNICODE_STRING          DrmPath;

        DrmPathEntry = CONTAINING_RECORD(Entry, NLFSE_DRM_PATH_ENTRY, listEntry);

        if( DrmPathEntry->path.Length < 6 ||                // The min length is 3*sizeof(WCHAR), L"C:\"
            DriveLetter != DrmPathEntry->path.Buffer[0] ||  // Different drive
            L':' != DrmPathEntry->path.Buffer[1]            // 2nd character is not L':'
            )
        {
            Entry = Entry->Flink;
            continue;
        }
        
        DrmPath.Length = DrmPathEntry->path.Length - sizeof(WCHAR)*2;
        DrmPath.Buffer = DrmPathEntry->path.Buffer + 2;
        DrmPath.MaximumLength = DrmPath.Length;

        if(Path->Length > DrmPath.Length)
        {
            *IsDRM = ((L'\\' == Path->Buffer[DrmPath.Length/sizeof(WCHAR)]) &&
                       RtlPrefixUnicodeString(&DrmPath, Path, TRUE)) ? TRUE : FALSE;
        }

        // It is DRM directory
        if(*IsDRM)
        {
            break;
        }

        // Get next list entry.
        Entry = Entry->Flink;
    }

    // Free shared lock
    NLFSEReleaseResource(&nlfseGlobal.drmPathListLock);

    return STATUS_SUCCESS;
}
