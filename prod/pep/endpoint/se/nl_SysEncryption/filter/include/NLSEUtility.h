/*++

Module Name:

    NLSEUtility.h

Abstract:
    This is the header file defining the utility functions used by 
    the kernel mode filter driver implementing NLSE

Environment:

    Kernel mode

--*/
#ifndef __NLSE_UTILITY_H__
#define __NLSE_UTILITY_H__



#include "NLSEStruct.h"



//Misc functions

//Get the current time
LARGE_INTEGER GetCurrentTime();

INT
NLCompareName(
              __in PCWCH Name1,
              __in USHORT Length1,
              __in PCWCH Name2,
              __in USHORT Length2
              );

INT
NLCompareNameCaseSensitive(
              __in PCWCH Name1,
              __in USHORT Length1,
              __in PCWCH Name2,
              __in USHORT Length2
              );

VOID
NLUpcaseString(
               __out PWCH OutString,
               __in PCWCH InString,
               __in USHORT Length
               );

//Check if the access from remote client; If yes, return TRUE
BOOLEAN NLSEIsAccessFromRemote(__in_opt PIO_SECURITY_CONTEXT secCtx);

//end: Misc functions



//Performance measurement helper functions

#ifdef NLSE_DEBUG_PERFORMANCE

typedef struct _NLPERFORMANCE_COUNTER
{
    LARGE_INTEGER   start;
    LARGE_INTEGER   end;
    LARGE_INTEGER   diff;
    LARGE_INTEGER   freq;
}NLPERFORMANCE_COUNTER, *PNLPERFORMANCE_COUNTER;

VOID
PfStart(
        __out PNLPERFORMANCE_COUNTER ppfc
        );

/*
It gets time diff in microseconds
*/
VOID
PfEnd(
      __out PNLPERFORMANCE_COUNTER ppfc
      );

#endif

//end: Performance measurement helper functions



//Pathname helper functions

/** NLSEBuildDosPath
 *
 *  \brief Constructs the DOS path from a full path.
 *
 *  \param FullPath (in)        Full path to convert (e.g.
 *                              "\Device\HarddiskVolume2\temp\foo.txt")
 *  \param VolumeName (in)      Volume name (e.g. "\Device\HarddiskVolume2")
 *  \param VolumeDosName (in)   DOS name of Volume (e.g. "C:")
 *  \param DosPath (out)        DOS path (e.g. "C:\temp\foo.txt")
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEBuildDosPath(
    __in  PCUNICODE_STRING FullPath,
    __in  PCUNICODE_STRING VolumeName,
    __in  PCUNICODE_STRING VolumeDosName,
    __out PUNICODE_STRING DosPath);

/** NLSEIsPathMatchingWildcardPath
 *
 *  \brief Check if the path matches a path containing wildcards.
 *         Supported wildcard formats are:
 *         - "*" (the path component is a "*")
 *         - "*abc" (the path component starts with a "*")
 *         Thus "C:\*Dir\barDir\*\*lo.txt" is supported and matches
 *         "C:\fooDir\barDir\bazDir\hello.txt".
 *
 *  \param path (in)            The path to check.
 *  \param wildcardPath (in)    The wildcard of which to check against.
 *  \param caseInSensitive (in) TRUE if ignore case.
 *  \param match (out)          TRUE if matches.
 *
 *  \return STATUS_SUCCESS on success.
 */
__checkReturn
NTSTATUS NLSEIsPathMatchingWildcardPath(
    __in  PCUNICODE_STRING      path,
    __in  PCUNICODE_STRING      wildcardPath,
    __in  BOOLEAN               caseInSensitive,
    __out PBOOLEAN              result);

//end: Pathname helper functions



//File Encryption Extension helper functions

//Free an encrypt extension (a.k.a. encryption footer)
void
NLFSEFreeEncryptExtension(__out_opt PNLFSE_ENCRYPT_EXTENSION pExt);

/** NLSEIsPathEncrypted
 *
 *  \brief Determine if a given file path is in the path of an encrypted directory.
 *
 *  \param FltObjcets (in) Filter objects.
 *  \param in_path    (in) File path to check.
 *  \param out_result (in) Result - TRUE/FALSE if encrypted
 */
__checkReturn
NTSTATUS NLSEIsPathEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
			      __in PUNICODE_STRING in_path ,
			      __out BOOLEAN* out_result );

//end: File Encryption Extension helper functions



//key management

/*
Return Value:
STATUS_SUCCESS - Succeed
STATUS_INSUFFICIENT_RESOURCES - FAIL: Fail to allocate buffer for request
STATUS_PORT_DISCONNECTED - FAIL: The port is not opened
STATUS_GENERIC_COMMAND_FAILED - FAIL: Policy Controller return error
STATUS_INVALID_BUFFER_SIZE - FAIL: Policy Controller return wrong buffer size
*/
NTSTATUS
NLSEUpdateCurrentPCKey(
                       __in ULONG pid,          // Requester process Id
                       __in BOOLEAN UseCache    // Use cache or not
                       );

BOOLEAN NLSEGetPCKeyByID(__in    char        *keyRingName,
			 __in    NLSE_KEY_ID *keyID,
			 __in    ULONG       pid,
			 __inout char        *key);

//end: key management



//User-Kernel Communication helper functions

VOID NLSEClientDisconnect(__in PVOID ConnectionCookie );

NTSTATUS NLSEClientConnect(__in PFLT_PORT ClientPort,
			   __in PVOID ServerPortCookie,
			   __in PVOID ConnectionContext,
			   __in ULONG SizeOfContext,
			   __out PVOID *ConnectionCookie );

NTSTATUS NLSEClientMessage(__in PVOID ConnectionCookie,
			    __in_opt PVOID InputBuffer,
			    __in ULONG InputBufferSize,
			    __out PVOID OutputBuffer,
			    __in ULONG OutputBufferSize,
			    __in PULONG ReturnOutputBufferLength );

//end: User-Kernel Communication helper functions



#endif
