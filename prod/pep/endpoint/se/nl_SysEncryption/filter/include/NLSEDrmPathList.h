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
 *  \brief Initialize the DRM path list.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEDrmPathListInit(VOID);

/** NLSEDrmPathListShutdown
 *
 *  \brief Shutdown the DRM path list.
 */
VOID NLSEDrmPathListShutdown(VOID);

/** NLSEDrmPathListSet
 *
 *  \brief Replace the stored DRM path list with the passed one.
 *
 *  \param numPaths (in)    # of paths.
 *  \param paths (in)       Blob of paths separated by null-terminators.
 *
 *  \return STATUS_SUCCESS on success.
 */
NTSTATUS NLSEDrmPathListSet(__in   ULONG numPaths,
                            __in_z PCWCH paths);

/** NLSEDrmPathListCheck
 *
 *  \brief See if the passed path matches any path in the DRM path list.
 *
 *  \param path (in)        Path to check.
 *  \param result (out)     TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEDrmPathListCheck(__in  PCUNICODE_STRING    path,
                              __out PBOOLEAN            result);


__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxInitNonDrmData (
                  );

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxCleanupNonDrmData (
                     );

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
NTSTATUS
NxReadNonDrmList (
                     __in PFLT_FILTER Filter,
                     __in_opt PFLT_INSTANCE Instance,
                     __in PUNICODE_STRING FileName
                  );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NxIsNonDrmDirectory (
                     __in WCHAR Drive,
                     __in PCUNICODE_STRING FileName, // File path without volume information
                     __in PBOOLEAN IsNonDrm
                     );

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NxIsNonDrmDirectoryEx (
                       __in WCHAR Drive,
                       __in PFLT_FILE_NAME_INFORMATION FileNameInfo,
                       __in PBOOLEAN IsNonDrm
                       );

#endif /* __NLSE_DRM_PATH_LIST_H__ */
