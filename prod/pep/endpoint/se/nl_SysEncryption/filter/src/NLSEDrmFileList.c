/*++

Module Name:
    NLSEDrmFileList.c

Abstract:
    Functions used by the kernel mode filter driver implementing DRM file
    lists.

Environment:
    Kernel mode

--*/

#include "NLSEUtility.h"
#include "pathList.h"
#include "NLSEDrmFileList.h"



//
// Global variables
//

extern NLFSE_GLOBAL_DATA nlfseGlobal;



//
// Internal functions
//

/** DrmFileOneShotListCheckCB
 *
 *  \brief Compare the path and PID to find with a path and a PID in the
 *         one-shot DRM file list.  It is a match if both match.  Wildcards in
 *         paths in list are honored (i.e. wildcard paths can match normal
 *         paths.)
 *
 *  \param pathToFind (in)      Path to find.
 *  \param dataToFind (in)      Data (PID) for the path to find.
 *  \param pathInList (in)      Path of one path entry in the list.
 *  \param dataInList (in)      Data (PID) for the path entry in the list.
 *  \param match (out)          TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
static NTSTATUS DrmFileOneShotListCheckCB(
    __in  PCUNICODE_STRING      pathToFind,
    __in  ULONG_PTR             dataToFind,
    __in  PCUNICODE_STRING      pathInList,
    __in  ULONG_PTR             dataInList,
    __out PBOOLEAN              match)
{
  // It is a match if both the paths and the PIDs for the path are the same.
  // (The data for the paths are the PIDs for the paths.)

  if ((ULONG) dataToFind != (ULONG) dataInList)
  {
    *match = FALSE;
    return STATUS_SUCCESS;
  }

  if (RtlEqualUnicodeString(pathToFind, pathInList, TRUE))
  {
    *match = TRUE;
    return STATUS_SUCCESS;
  }
  else
  {
    return NLSEIsPathMatchingWildcardPath(pathToFind, pathInList, TRUE, match);
  }
} /* DrmFileOneShotListCheckCB */

/** DrmFileOneShotListRemoveCB
 *
 *  \brief Compare the path and PID to find with a path and a PID in the
 *         one-shot DRM file list.  It is a match if both match.  Wildcards in
 *         paths in list are not honored (i.e. wildcard paths cannnot match
 *         normal paths.)
 *
 *  \param pathToFind (in)      Path to find.
 *  \param dataToFind (in)      Data (PID) for the path to find.
 *  \param pathInList (in)      Path of one path entry in the list.
 *  \param dataInList (in)      Data (PID) for the path entry in the list.
 *  \param match (out)          TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
static NTSTATUS DrmFileOneShotListRemoveCB(
    __in  PCUNICODE_STRING      pathToFind,
    __in  ULONG_PTR             dataToFind,
    __in  PCUNICODE_STRING      pathInList,
    __in  ULONG_PTR             dataInList,
    __out PBOOLEAN              match)
{
  // It is a match if both the paths and the PIDs for the paths are the same.
  // (The data for the paths are the PIDs for the paths.)
  *match = ((ULONG) dataToFind == (ULONG) dataInList &&
            RtlEqualUnicodeString(pathToFind, pathInList, TRUE));
  return STATUS_SUCCESS;
} /* DrmFileOneShotListRemoveCB */

/** DrmFileOneShotListRemoveAllCB
 *
 *  \brief Compare the PID to find with a PID in the one-shot DRM file list.
 *
 *  \param pathToFind (in)      Path to find.  (Unused.)
 *  \param dataToFind (in)      Data (PID) for the path to find.
 *  \param pathInList (in)      Path of one path entry in the list.  (Unused.)
 *  \param dataInList (in)      Data (PID) for the path entry in the list.
 *  \param match (out)          TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
static NTSTATUS DrmFileOneShotListRemoveAllCB(
    __in  PCUNICODE_STRING      pathToFind,
    __in  ULONG_PTR             dataToFind,
    __in  PCUNICODE_STRING      pathInList,
    __in  ULONG_PTR             dataInList,
    __out PBOOLEAN              match)
{
  // It is a match if the PIDs for the paths are the same.  (The data for the
  // paths are the PIDs for the paths.)
  *match = ((ULONG) dataToFind == (ULONG) dataInList);
  return STATUS_SUCCESS;
} /* DrmFileOneShotListRemoveAllCB */



//
// Exported functions
//

__checkReturn
NTSTATUS NLSEDrmFileOneShotListInit(__in ULONG maxCount)
{
  return PathListInit(&nlfseGlobal.drmFileOneShotList, maxCount);
} /* NLSEDrmFileOneShotListInit */

VOID NLSEDrmFileOneShotListShutdown(VOID)
{
  PathListShutdown(&nlfseGlobal.drmFileOneShotList);
} /* NLSEDrmFileOneShotListShutdown */

NTSTATUS NLSEDrmFileOneShotListAdd(__in_z PCWCH path,
                                   __in   ULONG pid)
{
  // Return error if list is full.
  return PathListAddPath(&nlfseGlobal.drmFileOneShotList, path, pid, FALSE);
} /* NLSEDrmFileOneShotListAdd */

__checkReturn
NTSTATUS NLSEDrmFileOneShotListCheck(__in  PCUNICODE_STRING     path,
                                     __in  ULONG                pid,
                                     __out PBOOLEAN             result)
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

  // Try to find the path in the list.  Remove the entry if found.
  return PathListFindPath(&nlfseGlobal.drmFileOneShotList, &pathNoPrefix, pid,
                          DrmFileOneShotListCheckCB, TRUE, result);
} /* NLSEDrmFileOneShotListCheck */

NTSTATUS NLSEDrmFileOneShotListRemove(__in_z PCWCH path,
                                      __in   ULONG pid)
{
  UNICODE_STRING pathStr;
  BOOLEAN result;
  NTSTATUS status;

  RtlInitUnicodeString(&pathStr, path);
  status = PathListFindPath(&nlfseGlobal.drmFileOneShotList, &pathStr, pid,
                            DrmFileOneShotListRemoveCB, TRUE, &result);

  if (NT_SUCCESS(status) && !result)
  {
    return STATUS_NOT_FOUND;
  }
  else
  {
    return status;
  }
} /* NLSEDrmFileOneShotListRemove */

NTSTATUS NLSEDrmFileOneShotListRemoveAll(__in ULONG pid)
{
  // Searching for any matching entries in the list.  For each round, Start
  // the searching at the list head.
  //
  // This is O(n^2) speed which is slow.  However, in real world scenario:
  // - this function is not called often,
  // - the list is usually very short.
  // So O(n^2) is still okay.
  BOOLEAN foundAtLeastOne = FALSE;

  for (;;)
  {
    BOOLEAN result;
    NTSTATUS status;

    status = PathListFindPath(&nlfseGlobal.drmFileOneShotList, NULL, pid,
                              DrmFileOneShotListRemoveAllCB, TRUE, &result);
    if (!NT_SUCCESS(status))
    {
      return status;
    }

    if (!result)
    {
      return foundAtLeastOne ? STATUS_SUCCESS : STATUS_NOT_FOUND;
    }

    foundAtLeastOne = TRUE;
  }
} /* NLSEDrmFileOneShotListRemoveAll */
