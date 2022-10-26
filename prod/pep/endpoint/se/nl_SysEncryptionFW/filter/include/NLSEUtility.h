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

//Get the current time
LARGE_INTEGER GetCurrentTime();

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

//Check if the access from remote client; If yes, return TRUE
BOOLEAN NLSEIsAccessFromRemote(__in_opt PIO_SECURITY_CONTEXT secCtx);

//Do synchronous read i/o
NTSTATUS
NLSESendSynchronousReadIO(__in PCFLT_RELATED_OBJECTS FltObjects,
			  __in LARGE_INTEGER readOffset,
			  __in PVOID         buffer,
			  __in LARGE_INTEGER bufferLen);

//Get the size of file
NTSTATUS  
NLSEGetFileSize(__in PFLT_FILTER  filterHandle,
		__in PFLT_INSTANCE filterInstance,
		__in NLFSE_PVOLUME_CONTEXT volCtx,
		__in PNLFSE_STREAM_CONTEXT pCtx,
		__out FILE_END_OF_FILE_INFORMATION *stEndOfFile);

//compose file name as \??\C:\path\file-name
void NLFSEGetFileNonUNCName(__inout PUNICODE_STRING outFileName,
			    __in NLFSE_PVOLUME_CONTEXT volCtx,
			    __in_opt PUNICODE_STRING inFileNameFinal,
			    __in PUNICODE_STRING inParentDir);
//compose file name as \??\C:\path\file-name
void 
NLFSEGetFileNonUNCNameByFileNameInfo(__out PUNICODE_STRING outFileName,
				     __in NLFSE_PVOLUME_CONTEXT volCtx,
				     __in PFLT_FILE_NAME_INFORMATION fnInfo);

//compose directory name without volume,e.g. folder\sub1\sub2
void NLSEComposeDirNameNoVolume(__inout PUNICODE_STRING outFileName,
				 __in_opt PUNICODE_STRING inFileNameFinal,
				__in PUNICODE_STRING inParentDir);

//Copy the source string to the destination string
__checkReturn
NTSTATUS 
NLSEAllocateAndCopyUnicodeNameString(__out_opt PUNICODE_STRING  DestinationString,
				     __in PUNICODE_STRING  SourceString);

//Free the buffer for the input name string
VOID
NLFSEFreeUnicodeNameString(__inout_opt PUNICODE_STRING  SourceString);

//Get the name (path and file-name-only) of the file based on 
//the callback data of an IO operation
BOOLEAN
NLFSEAllocateAndGetFileName(__inout PFLT_CALLBACK_DATA Data,
			    __in PCFLT_RELATED_OBJECTS FltObjects,
			    __out PUNICODE_STRING Name,
			    __out PUNICODE_STRING ParentDir,
                __out PUNICODE_STRING VolName);

//Corresponding to the function NLFSEAllocateAndGetFileName
VOID 
NLFSEDeallocateFileName( PUNICODE_STRING Name,
			 PUNICODE_STRING parentDir);
NLFSE_PIRP_ENTRY
NLFSEAllocateIRPEntry(__in PCFLT_RELATED_OBJECTS FltObjects,
			  __in NLFSE_PVOLUME_CONTEXT volCtx,
		      __in PUNICODE_STRING fileParentDir,
		      __in PUNICODE_STRING fileNameFinal);

//Free an IRP entry
VOID
NLFSEFreeIRPEntry(__inout_opt NLFSE_PIRP_ENTRY irpEntry);

NTSTATUS
NLFSEPrintIRPEntry(LONG Index, const WCHAR *);

//Policy helper functions
//Set DRM attribute to a file or directory
VOID NLSESetDRMAttribute(WCHAR *name, BOOLEAN bFile);
//Unset DRM attribute to a file or directory
VOID NLSEUnsetDRMAttribute(WCHAR *name, BOOLEAN bFile);
//For a new file, we check if the file is pre-set to be encrypted
//For existing file, we check if the file has ADS.
//If the file should be encrypted, set irpEntry->NeedEncrypt to TRUE
FLT_PREOP_CALLBACK_STATUS 
NLFSECheckFileEncryption(__in PCFLT_RELATED_OBJECTS FltObjects,
			 __inout_opt NLFSE_PIRP_ENTRY irpEntry,
			 __inout PFLT_CALLBACK_DATA Data);

VOID 
NLSEHandlePendedIO(__in NLSE_PENDING_IO_QUEUE_PCONTEXT queueCtx,
		   __in PFLT_CALLBACK_DATA Data);

//Check if the target of renamed file should be encrypted; if yes, encrypt 
//the target file
void NLFSECheckAndEncryptRenamedFile(__in PCFLT_RELATED_OBJECTS fltObjects,
				     __in PFLT_CALLBACK_DATA Data,
				     __in PFLT_FILE_NAME_INFORMATION target,
				     __in PFLT_FILE_NAME_INFORMATION src,
				     __inout BOOLEAN *bReleaseSource);
//Remove a policy on the input file if the policy exists
VOID NLSERemoveFilePolicy(PUNICODE_STRING fileName);

//free all existing per-stream contexts for the given device
VOID
NLFSEDeleteAllContexts (
    __in NLFSE_PVOLUME_CONTEXT volCtx
    );


//PFLT_CONTEXT_CLEANUP_CALLBACK NlfseStreamHandleContextCleanupCallback;
VOID
NlfseStreamHandleContextCleanupCallback(
	__in_opt PFLT_CONTEXT  Context,
	__in FLT_CONTEXT_TYPE  ContextType
	);

BOOLEAN
NlfseInitPerStreamHandleContext(
	__in PUNICODE_STRING FileName,
	__in PNLFSE_ENCRYPT_EXTENSION EncryptExtension,
	__inout PNLFSE_STREAM_CONTEXT *StreamHandleContext
	);


// Chaneg this context related file name
NTSTATUS
NLFSEUpdateStreamHandleContextFileName(
                           __in_opt PCFLT_RELATED_OBJECTS FltObjects,
                           __in_opt PNLFSE_STREAM_CONTEXT pCtx,
                           __in_opt PFLT_FILE_NAME_INFORMATION fileInfo
                           );


//Check if the IO operated file exists or not
BOOLEAN
NLFSECheckFileExist(__in PCFLT_RELATED_OBJECTS  FltObjects,
		    __in  NLFSE_PVOLUME_CONTEXT volCtx,
		    __in  NLFSE_PIRP_ENTRY      irpEntry,
            __in  BOOLEAN               OverWrite,
		    __out  PNLFSE_STREAM_CONTEXT *pContext,
		    __out BOOLEAN               *bNotFile,
		    __out BOOLEAN               *bWinEncrypted,
		    __out BOOLEAN               *bWinCompressed);

//Check if the FileObject is a directory
BOOLEAN
NLSEIsDirFileObj(__in PCFLT_RELATED_OBJECTS fltObjects);

//File Encryption Extension helper functions
//Allocate an encrypt extension (a.k.a. encryption footer)
PNLFSE_ENCRYPT_EXTENSION
NLFSEAllocateEncryptExtension(__in ULONGLONG realLength);

//Free an encrypt extension (a.k.a. encryption footer)
void
NLFSEFreeEncryptExtension(__out PNLFSE_ENCRYPT_EXTENSION pExt);


//Check if the file has encryption alternative data stream.
//If yes, create an encrypt extension and return true. If not, return false.
/*BOOLEAN 
NLFSECheckEncrypteADSExist(__in PFLT_FILTER filterHandle,
			   __in PFLT_INSTANCE filterInstance,
			   __in PUNICODE_STRING fileName,
			   __in NLFSE_PVOLUME_CONTEXT volCtx,
			   __in ULONG                 pid,
			   __deref_out PNLFSE_ENCRYPT_EXTENSION *pEncryptExt,
			   __out BOOLEAN *bGetPCKeyFailed);*/

//Open encryption alternative data stream if it exists.
//If ADS exists, this function return the pointer to a
//encryption extension buffer.
PNLFSE_ENCRYPT_EXTENSION  
NLSEOpenAndReadADS(__in PFLT_FILTER filter,
		   __in PFLT_INSTANCE instance,
		   __in PUNICODE_STRING fileName,
		   __in NLFSE_PVOLUME_CONTEXT volCtx);

// Write Encrypt Extension to ADS file
NTSTATUS
EncryptExtensionToAdsFile(__in PFLT_INSTANCE             Instance,
                          __in PFILE_OBJECT              FileObject,
                          __in PNLFSE_ENCRYPT_EXTENSION  dupExt,
                          __in ULONG                     pid,
                          __in ULONG                     sectorSize);

//Create (open if exists already) encryption ADS (alternative data stream)
//in order to store encryption information. 
void
NLSEGenEncryptionADS(__in PFLT_FILTER filterHandle,
		     __in PFLT_INSTANCE filterInstance,
		     __in PUNICODE_STRING fileName,
		     __in NLFSE_PVOLUME_CONTEXT volCtx,
		     __in_opt PNLFSE_ENCRYPT_EXTENSION pEncryptExt,
		     __in ULONG pid);

//Delete (if exists) encryption ADS (alternative data stream)
void
NLSEDeleteEncryptionADS(__in PFLT_FILTER filterHandle,
			__in PFLT_INSTANCE filterInstance,
			__in PUNICODE_STRING fileName,
			__in NLFSE_PVOLUME_CONTEXT volCtx);

// Create ADS for a new file at PostCreate
NTSTATUS
NLSECreateAdsAtPostCreate(
                          __in PFLT_FILTER Filter,
                          __in_opt PFLT_INSTANCE Instance,
                          __in_opt PUNICODE_STRING FileName,
                          __in NLFSE_PVOLUME_CONTEXT  volCtx,
                          __in_opt PNLFSE_ENCRYPT_EXTENSION Extension
                          );

//Set Basic Information on input file "fileName" 
BOOLEAN
NLSESetFileBasicInfo(__in PFLT_FILTER filterHandle,
		     __in PFLT_INSTANCE filterInstance,
		     __in NLFSE_PVOLUME_CONTEXT volCtx,
		     __in PUNICODE_STRING fileName,
		     __out PFILE_BASIC_INFORMATION originalFileBasicInfo,
		     __in PFILE_BASIC_INFORMATION newFileBasicInfo);

//Checking if a directory has encryption attribute.
//The attribute is in the form of NTFS alternative data stream.
//If yes, return true; 
BOOLEAN NLSECheckDirEncryptionAttribute(__in PCFLT_RELATED_OBJECTS FltObjects,
					__in PUNICODE_STRING dirName,
					__in BOOLEAN         bHasBackSlash);

/** NLSEIsDirectoryEncrypted
 *
 *  \brief Determine if a given directory is encrypted.
 *
 *  \param FltObjcets (in) Filter objects.
 *  \param in_path    (in) Directory to check.
 *  \param out_result (in) Result - TRUE/FALSE if encrypted
 */
__checkReturn
NTSTATUS NLSEIsDirectoryEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result );

/** NLSEIsDirectoryEncrypted
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
			      __in BOOLEAN* out_result );

//Create (open if exists already) encryption ADS (alternative data stream)
//in order to store encryption information. Return ADS handle.
//context for future updating. 
HANDLE
NLSEOpenandGetADSHandle(__in PFLT_FILTER filterHandle,
			__in PFLT_INSTANCE filterInstance,
			__in PUNICODE_STRING fileName,
			__in PFILE_BASIC_INFORMATION originalFileBasicInfo,
			__in NLFSE_PVOLUME_CONTEXT volCtx,
			__out BOOLEAN               *bChangeFileBasicInfo);
//end: File Encryption Extension helper functions

//Generate random data encryption key
VOID NLSEGenDataEncryptionKey(
				__inout PNLFSE_ENCRYPT_EXTENSION pEncryptExt);

//encrypt encryption indicator
BOOLEAN NLSEEncryptIndicator(__inout PNLFSE_ENCRYPT_EXTENSION pExt,
			     __in    char                     *key,
			     __in    NLSE_KEY_ID              *keyID,
			     __in    char                     *keyRingName);

//decrypt encryption indicator
BOOLEAN NLSEDecryptIndicator(__inout PNLFSE_ENCRYPT_EXTENSION pExt,
			     __in    char                     *key,
			     __in    size_t                   keyLen);

//Do CBC cipher block decryption
BOOLEAN NLSEDecryptionAtPostRead(__in PFLT_CALLBACK_DATA Data, 
				 __in PCFLT_RELATED_OBJECTS FltObjects, 
				 __in_opt NLFSE_PPRE_2_POST_CONTEXT p2pCtx,
				 __inout PUCHAR outBuf, 
				 __in PUCHAR inBuf);
//Do CBC cipher block encryption
//return false if the operation failed
BOOLEAN NLSEEncryptionAtPreWrite(__in PFLT_CALLBACK_DATA Data, 
				 __in PCFLT_RELATED_OBJECTS FltObjects,   
				 __in_opt NLFSE_PPRE_2_POST_CONTEXT p2pCtx,
				 __in ULONG writeLen,
				 __inout PUCHAR outBuf, 
				 __in PUCHAR inBuf);

//Encrypt a file by reading and re-writing it; 
//and fill in the encryption extension buffer
NTSTATUS NLSEEncryptFile(__in  PFLT_FILTER                  Filter,
			 __in  PFLT_INSTANCE                Instance,
			 __in  NLFSE_PVOLUME_CONTEXT        volCtx,
			 __in  PUNICODE_STRING              fileName,
			 __out PNLFSE_ENCRYPT_EXTENSION     pEncryptExt);

//Do decryption when the buffer is safe to use
FLT_POSTOP_CALLBACK_STATUS
NLSEPostReadDecryptionWhenSafe (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

//Calculate padding in a worker thread after write OP
BOOLEAN 
NLSECalculatePadding(__inout NLFSE_PPRE_2_POST_CONTEXT p2pCtx,
		     __in PFILE_END_OF_FILE_INFORMATION endOfFile);

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

VOID
NLSEUpdateADSKeyId(__in PCFLT_RELATED_OBJECTS FltObjects,
                   __in ULONG ProcessId,
                   __in PUNICODE_STRING FileName,
                   __inout PNLFSE_ENCRYPT_EXTENSION  EncryptExt
                   );

VOID 
NLSEQueueAdsWorkItem(
                     __inout PNLFSE_ADS_WORKITEM workItem
                     );

BOOLEAN NLSEGetPCKeyByID(__in    char        *keyRingName,
			 __in    NLSE_KEY_ID *keyID,
			 __in    ULONG       pid,
			 __inout char        *key);
//end - key management

//User-Kernel Communication helper functions
VOID NLSEClientDisconnect(__in PVOID ConnectionCookie );

NTSTATUS NLSEClientConnect(PFLT_PORT ClientPort,
			   PVOID ServerPortCookie,
			   PVOID ConnectionContext,
			   ULONG SizeOfContext,
			   PVOID *ConnectionCookie );

NTSTATUS NLSEClientMessage( PVOID ConnectionCookie,
			    PVOID InputBuffer,
			    ULONG InputBufferSize,
			    PVOID OutputBuffer,
			    ULONG OutputBufferSize,
			    PULONG ReturnOutputBufferLength );

BOOLEAN
IsNTFSReservedFile(
                   __in_opt PUNICODE_STRING ParentDir,
                   __in_opt PUNICODE_STRING FileName
                   );

BOOLEAN
IsAdsStreamInfo(
                __in PFILE_STREAM_INFORMATION fsi
                );

BOOLEAN
RemoveNLAdsFromQueryResult(
                           __in_opt PFILE_STREAM_INFORMATION fsi
                           );

void CheckVolumeName(__in PFLT_VOLUME Volume,__inout_opt PUNICODE_STRING Name);

#endif
