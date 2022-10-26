/*++

Module Name:
    pathList.h

Abstract:
    This is the header file defining the functions used by 
    the kernel mode filter driver implementing path list.

Environment:
    Kernel mode

--*/
#ifndef __PATH_LIST_H__
#define __PATH_LIST_H__

#include <ntddk.h>



typedef struct _NL_PATH_ENTRY
{
  LIST_ENTRY            Entry;
  UNICODE_STRING        Path;
  ULONG_PTR             Data;
} NL_PATH_ENTRY, *PNL_PATH_ENTRY;

typedef struct _NL_PATH_LIST
{
  LIST_ENTRY            ListHead;
  ERESOURCE             Lock;
  ULONG                 MaxCount;       // max. # of entries, or 0 if no limit.
  ULONG                 Count;
} NL_PATH_LIST, *PNL_PATH_LIST;



/** path_list_find_path_cb_t
 *  \brief Callback function for PathListFindPath()
 *
 *  \param pathToFind (in)      Path to find in the list
 *  \param dataToFind (in)      Data for path to find in the list
 *  \param pathInList (in)      Path in path list
 *  \param dataInList (in)      Data for path in path list
 *  \param match (out)          TRUE if a match, which stops searching
 *                              FALSE if not match, which searching continues.
 *
 *  \return STATUS_SUCCESS on success.
 */
typedef
__checkReturn
NTSTATUS (*path_list_find_path_cb_t) (
    __in  PCUNICODE_STRING      pathToFind,
    __in  ULONG_PTR             dataToFind,
    __in  PCUNICODE_STRING      pathInList,
    __in  ULONG_PTR             dataInList,
    __out PBOOLEAN              match);

/** PathListInit
 *
 *  \brief Initialize the path list.
 *
 *  \param list (inout)         Path list.
 *  \param maxCount (in)        Max. # of entries in the list, or 0 if no
 *                              limit.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS PathListInit(__inout PNL_PATH_LIST     list,
                      __in    ULONG             maxCount);

/** PathListShutdown
 *
 *  \brief Shutdown the path list.
 *
 *  \param list (inout)         Path list.
 */
VOID PathListShutdown(__inout PNL_PATH_LIST list);

/** PathListSetPaths
 *
 *  \brief Replace the stored paths with the passed ones.
 *
 *  \param list (inout)         Path list.
 *  \param numPaths (in)        # of paths.
 *  \param paths (in)           Blob of paths separated by null-terminators.
 *  \param data (in)            Data associated with the paths.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS PathListSetPaths(__inout PNL_PATH_LIST list,
                          __in    ULONG         numPaths,
                          __in_z  PCWCH         paths,
                          __in    ULONG_PTR     data);

/** PathListPurgePaths
 *
 *  \brief Purge the existing paths in the path list.
 *
 *  \param list (inout)         Path list.
 */
VOID PathListPurgePaths(__inout PNL_PATH_LIST list);

/** PathListAddPath
 *
 *  \brief Add the passed path to the path list.
 *
 *  \param list (inout)         Path list.
 *  \param path (in)            Path to add.
 *  \param data (in)            Data associated with the path.
 *  \param replaceOldestIfFull (in) TRUE if replace the oldest entry with
 *                              the passed path when the list is full.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS PathListAddPath(__inout PNL_PATH_LIST  list, 
                         __in_z  PCWCH          path,
                         __in    ULONG_PTR      data,
                         __in    BOOLEAN        replaceOldestIfFull);

/** PathListFindPath
 *  \brief Find the path in the path list.
 *
 *  \param list (inout)         Path list.
 *  \param path (in)            Path to find.
 *  \param data (in)            Data associated with path to find.
 *  \param cb (in)              Callback function to compare paths and data.
 *  \param removeIfFound (in)   TRUE if the matching entry in the list is to
 *                              be removed if the path is found.
 *  \param found (out)          TRUE if found.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS PathListFindPath(__inout  PNL_PATH_LIST                list,
                          __in     PCUNICODE_STRING             path,
                          __in     ULONG_PTR                    data,
                          __in     path_list_find_path_cb_t     cb,
                          __in     BOOLEAN                      removeIfFound,
                          __out    PBOOLEAN                     found);



#endif /* __PATH_LIST_H__ */
