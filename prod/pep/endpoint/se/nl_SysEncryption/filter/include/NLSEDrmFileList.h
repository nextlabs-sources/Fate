/*++

Module Name:
    NLSEDrmFileList.h

Abstract:
    This is the header file defining the functions used by 
    the kernel mode filter driver implementing DRM file lists.

Environment:
    Kernel mode

--*/

#ifndef __NLSE_DRM_FILE_LIST_H__
#define __NLSE_DRM_FILE_LIST_H__



#include <ntddk.h>



/** NLSEDrmFileOneShotListInit
 *
 *  \brief Initialize the one-shot DRM file list.
 *  \param maxCount (in)    Max. count of list
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEDrmFileOneShotListInit(__in ULONG maxCount);

/** NLSEDrmFileOneShotListShutdown
 *
 *  \brief Shutdown the one-shot DRM file list.
 */
VOID NLSEDrmFileOneShotListShutdown(VOID);

/** NLSEDrmFileOneShotListAdd
 *
 *  \brief Add the passed path to the one-shot DRM file list.
 *
 *  \param path (in)        Path to add.
 *  \param pid (in)         PID of process which this file applies.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmFileOneShotListAdd(__in_z PCWCH path,
                                   __in   ULONG pid);

/** NLSEDrmFileOneShotListCheck
 *
 *  \brief See if the passed path matches any path in the one-shot DRM file
 *         list.
 *
 *  \param path (in)        Path to check.
 *  \param pid (in)         PID of process for the file to check.
 *  \param result (out)     TRUE if matches (which the entry is removed).
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEDrmFileOneShotListCheck(__in  PCUNICODE_STRING     path,
                                     __in  ULONG                pid,
                                     __out PBOOLEAN             result);

/** NLSEDrmFileOneShotListRemove
 *
 *  \brief Remove the passed path from the one-shot DRM file list.
 *
 *  \param path (in)        Path to remove.
 *  \param pid (in)         PID of process which this file applies.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmFileOneShotListRemove(__in_z PCWCH path,
                                      __in   ULONG pid);

/** NLSEDrmFileOneShotListRemoveAll
 *
 *  \brief Remove all paths from the one-shot DRM file list.
 *
 *  \param pid (in)         PID of process which the files apply.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmFileOneShotListRemoveAll(__in ULONG pid);



#endif /* __NLSE_DRM_FILE_LIST_H__ */
