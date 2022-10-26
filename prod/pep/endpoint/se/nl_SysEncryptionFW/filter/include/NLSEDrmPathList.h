/*++

Module Name:
    NLSEDrmPathList.h

Abstract:
    This is the header file defining the functions used by 
    the kernel mode filter driver implementing DRM path list.

Environment:
    Kernel mode

--*/
#ifndef __NLSE_DRM_PATH_LIST_H__
#define __NLSE_DRM_PATH_LIST_H__

#include <ntddk.h>

/** NLSEDrmPathListInit
 *
 *  \brief  Initialize the DRM path list.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmPathListInit(void);

/** NLSEDrmPathListShutdown
 *
 *  \brief  Shutdown the DRM path list.
 */
void NLSEDrmPathListShutdown(void);

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
                            __in_z const WCHAR *paths);

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
                                  __out BOOLEAN *result);



__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NLSEInDRMDirectory (
                    __in WCHAR DriveLetter,
                    __in PUNICODE_STRING Path,
                    __out PBOOLEAN IsDRM
                    );

#endif
