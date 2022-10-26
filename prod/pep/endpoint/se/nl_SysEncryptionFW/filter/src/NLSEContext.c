/*++

Module Name:

    NLSEContext.c

Abstract:
  Context management for system encryption in kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSEDef.h"

//
// linker commands
//

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, NLFSEDeleteAllContexts )
#pragma alloc_text( PAGE, NLSECheckDirEncryptionAttribute )

#endif  // ALLOC_PRAGMA

extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;


//Create (open if exists already) encryption ADS (alternative data stream)
//in order to store encryption information. Store ADS handle with per-stream 
//context for future updating. 
HANDLE
NLSEOpenandGetADSHandle(
			__in PFLT_FILTER filterHandle,
			__in PFLT_INSTANCE filterInstance,
			__in PUNICODE_STRING fileName,
			__in PFILE_BASIC_INFORMATION originalFileBasicInfo,
			__in NLFSE_PVOLUME_CONTEXT volCtx,
			__out BOOLEAN               *bChangeFileBasicInfo)
{
  UNICODE_STRING               streamName;
  OBJECT_ATTRIBUTES            objAttr;
  FILE_BASIC_INFORMATION       newFileBasicInfo;
  FILE_BASIC_INFORMATION       dummyFileBasicInfo;
  HANDLE                       handle;
  NTSTATUS                     status;
  IO_STATUS_BLOCK              ioStatusBlock;
  PVOID                        buffer;
  size_t                       bufferSize=sizeof(NLFSE_ENCRYPT_EXTENSION);
  ULONG                        bytesWritten;
  PFILE_OBJECT                 fileHandle;
  FILE_END_OF_FILE_INFORMATION stEndOfFile;
  LARGE_INTEGER                offset;
  int                          retryCnt=0;

  if(KeGetCurrentIrql() != PASSIVE_LEVEL)
    return NULL;

  ASSERT( KeGetCurrentIrql() != PASSIVE_LEVEL );

  //Initialization
  *bChangeFileBasicInfo=FALSE;
  RtlZeroMemory(&newFileBasicInfo,sizeof(newFileBasicInfo));
  newFileBasicInfo.FileAttributes=FILE_ATTRIBUTE_NORMAL;

  //compose ADS name
  RtlInitUnicodeString(&streamName, NULL);
  streamName.MaximumLength=NLFSE_NAME_LEN*sizeof(WCHAR);
  streamName.Length=0;
  streamName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_GETADSHANDLE_TAG);
  if(streamName.Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!OpenandGetADSHandle: failed to allocate %d memory.\n",
		streamName.MaximumLength);
    return NULL;
  }
  RtlZeroMemory(streamName.Buffer, streamName.MaximumLength);
  status=RtlAppendUnicodeStringToString(&streamName, fileName);
  if(status != STATUS_SUCCESS) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!OpenandGetADSHandle: failed to append 0x%x.\n",
		status);
    return NULL;
  }
  RtlAppendUnicodeToString(&streamName, NLFSE_ADS_SUFFIX);
  
  //Check if ADS exists 
  InitializeObjectAttributes(&objAttr,
			     &streamName,
			     OBJ_KERNEL_HANDLE,
			     NULL,
			     NULL);

  for(retryCnt=0; retryCnt < 10; retryCnt++) {
    //Open the file 
    status = FltCreateFile(filterHandle,
			   filterInstance,
			   &handle,
			   GENERIC_WRITE,
			   &objAttr,
			   &ioStatusBlock,
			   0,
			   FILE_ATTRIBUTE_NORMAL,
			   FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
			   FILE_OVERWRITE_IF,
			   FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
			   NULL,
			   0,
			   IO_IGNORE_SHARE_ACCESS_CHECK);
    if (!NT_SUCCESS(status)) {
      if(status == STATUS_ACCESS_DENIED) {
	if(NLSESetFileBasicInfo(filterHandle,
				filterInstance,
				volCtx,
				fileName,
				originalFileBasicInfo,
				&newFileBasicInfo)) {
	  *bChangeFileBasicInfo=TRUE;
	  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		      "NLSE!OpenandGetADSHandle: Re-try(%d) open ADS %wZ\n", 
		      retryCnt+1, &streamName);	  
	} else {
	  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		      "NLSE!OpenandGetADSHandle: can't change att of %wZ\n", 
		      fileName);
	}
      } else {
	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		    "NLSE!OpenandGetADSHandle: create %wZ failed 0x%x\n", 
		    &streamName, status);
	break;
      }
    } else {
      //Create success
      break;
    } 
  }

  if(!NT_SUCCESS(status)) {
    if(*bChangeFileBasicInfo == TRUE) {
      //We need to change the file attribute back
      if(!NLSESetFileBasicInfo(filterHandle,
			       filterInstance,
			       volCtx,
			       fileName,
			       &dummyFileBasicInfo,
			       originalFileBasicInfo)) {
	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		    "NLSE!OpenandGetADSHandle: Can't change attr back %wZ\n", 
		    fileName);	  
      }
    }
    ExFreePool(streamName.Buffer);
    return NULL;
  }

  ExFreePool(streamName.Buffer);
  return handle;
}/*NLSEOpenandGetADSHandle*/


VOID
NLFSEDeleteAllContexts (
    __in NLFSE_PVOLUME_CONTEXT volCtx
    )
/*++

Routine Description:

    This will free all existing contexts for the given device

Arguments:

    volCtx - The context of volumen to operate on

Return Value:

    None.

--*/
{
    PLIST_ENTRY link;
    PNLFSE_STREAM_CONTEXT pContext;
    PFSRTL_PER_STREAM_CONTEXT ctxCtrl;
    LIST_ENTRY localHead;
    ULONG deleteNowCount = 0;
    ULONG deleteDeferredCount = 0;
    ULONG deleteInCallbackCount = 0;

    PAGED_CODE();

    //INC_STATS(TotalContextDeleteAlls);

    InitializeListHead( &localHead );

    try {

        //
        //  Acquire list lock
        //

        NLFSEAcquireContextLockExclusive( volCtx );

        //
        //  Walk the list of contexts and release each one
        //

        while (!IsListEmpty( &volCtx->StreamCtxList )) {

            //
            //  Unlink from top of list
            //

            link = RemoveHeadList( &volCtx->StreamCtxList );
            pContext = CONTAINING_RECORD( link, NLFSE_STREAM_CONTEXT, 
					  VolumeContextLink );

            //
            //  Mark that we are unlinked from the list.  We need to do this
            //  because of the race condition between this routine and the
            //  deleteCallback from the FS.
            //

            ASSERT(FlagOn(pContext->Flags,CTXFL_InVolumeContextList));
            RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,
						 CTXFL_InVolumeContextList);

            //
            //  Try and remove ourselves from the File Systems context control
            //  structure.  Note that the file system could be trying to tear
            //  down their context control right now.  If they are then we
            //  will get a NULL back from this call.  This is OK because it
            //  just means that they are going to free the memory, not us.
            //  NOTE:  This will be safe because we are holding the ContextLock
            //         exclusively.  If this were happening then they would be
            //         blocked in the callback routine on this lock which
            //         means the file system has not freed the memory for
            //         this yet.
            //

            if (FlagOn(pContext->Flags,CTXFL_InStreamList)) {

                ctxCtrl = FsRtlRemovePerStreamContext( pContext->Stream,
                                                       volCtx,
                                                       NULL );

                //
                //  Always clear the flag wether we found it in the list or
                //  not.  We can have the flag set and not be in the list if
                //  after we acquired the context list lock we context swapped
                //  and the file system is right now in SpyDeleteContextCallback
                //  waiting on the list lock.
                //

                RtlInterlockedClearBitsDiscardReturn(&pContext->Flags,
						     CTXFL_InStreamList);

                //
                //  Handle wether we were still attached to the file or not.
                //

                if (NULL != ctxCtrl) {

                    ASSERT(pContext == CONTAINING_RECORD(ctxCtrl,
							 NLFSE_STREAM_CONTEXT,
							 ContextCtrl));

                    //
                    //  To save time we don't do the free now (with the lock
                    //  held).  We link into a local list and then free it
                    //  later (in this routine).  We can do this because it
                    //  is no longer on any list.
                    //

                    InsertHeadList( &localHead, &pContext->VolumeContextLink );

                } else {

                    //
                    //  The context is in the process of being freed by the file
                    //  system.  Don't do anything with it here, it will be
                    //  freed in the callback.
                    //

                    //INC_STATS(TotalContextsNotFoundInStreamList);
                    //INC_LOCAL_STATS(deleteInCallbackCount);
                }
            }
        }
    } finally {

        NLFSEReleaseContextLock( volCtx );
    }

    //
    //  We have removed everything from the list and released the list lock.
    //  Go through and figure out what entries we can free and then do it.
    //

    while (!IsListEmpty( &localHead )) {

        //
        //  Get next entry of the list and get our context back
        //

        link = RemoveHeadList( &localHead );
        pContext = CONTAINING_RECORD( link, NLFSE_STREAM_CONTEXT, 
				      VolumeContextLink );

        //
        //  Decrement the USE count and see if we can free it now
        //

        ASSERT(pContext->UseCount > 0);

        if (InterlockedDecrement( &pContext->UseCount ) <= 0) {

            //
            //  No one is using it, free it now
            //
	  if(pContext->encryptExt) {
		NLFSEFreeEncryptExtension(pContext->encryptExt);
		pContext->encryptExt=NULL;
	  }

	  if(NULL != pContext->FileName.Buffer)
	  {
		  ExFreePoolWithTag(pContext->FileName.Buffer, NLFSE_CONTEXT_TAG);
	  }

	  ExFreePoolWithTag(pContext, NLFSE_CONTEXT_TAG);

	  //INC_STATS(TotalContextNonDeferredFrees);
	  //INC_LOCAL_STATS(deleteNowCount);

        } else {

            //
            //  Someone still has a pointer to it, it will get deleted
            //  later when they release
            //

            //INC_LOCAL_STATS(deleteDeferredCount);
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
			"NLFSE!DeleteAllContexts:  DEFERRED    (%p) Fl=%02x Use=%d \"%wZ\"\n",
			pContext,
			pContext->Flags,
			pContext->UseCount,
			&pContext->FileName );
        }
    }

    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
		"NLFSE!DeleteAllContexts:   %3d deleted now, %3d deferred, %3d close contention  \"%wZ\"\n",
		deleteNowCount,
		deleteDeferredCount,
		deleteInCallbackCount,
		&volCtx->Name);
}

BOOLEAN NlfseInitPerStreamHandleContext(
	__in PUNICODE_STRING FileName,
	__in PNLFSE_ENCRYPT_EXTENSION EncryptExtension,
	__inout PNLFSE_STREAM_CONTEXT *StreamHandleContext
	)
{
	PNLFSE_STREAM_CONTEXT pStreamHandleContext = *StreamHandleContext;

	ASSERT(NULL != StreamHandleContext);
	ASSERT(NULL != EncryptExtension);

	ASSERT(NULL != pStreamHandleContext);

	//  Init the context structure
	RtlZeroMemory( pStreamHandleContext, sizeof(NLFSE_STREAM_CONTEXT) );
	pStreamHandleContext->UseCount = 1;
	KeInitializeSpinLock(&pStreamHandleContext->encryptExtLock);
	pStreamHandleContext->FileName.Length = 0;
	pStreamHandleContext->FileName.MaximumLength = FileName->Length + sizeof(WCHAR);
	pStreamHandleContext->bDelete=FALSE;
	ExInitializeFastMutex(&pStreamHandleContext->deleteFlagLock);

	// Allocate file name buffer
#pragma prefast(disable:6014, "Release this memory in different function cause incorrect preFast warning 6014 - memory Leak") 
	pStreamHandleContext->FileName.Buffer = ExAllocatePoolWithTag( 
			NonPagedPool, pStreamHandleContext->FileName.MaximumLength, NLFSE_CONTEXT_TAG );
#pragma prefast(enable:6014, "recover this warning")
	if(NULL==pStreamHandleContext->FileName.Buffer)
	{
		return FALSE;
	}
	else
	{
		RtlZeroMemory( pStreamHandleContext->FileName.Buffer, pStreamHandleContext->FileName.MaximumLength );
		//  set the file name
		RtlCopyUnicodeString( &pStreamHandleContext->FileName,  FileName);
		// associate encryptExt with per-steam context
		pStreamHandleContext->encryptExt = EncryptExtension;
			
		return TRUE;
	}
}

//typedef VOID
//  (*PFLT_CONTEXT_CLEANUP_CALLBACK) (
//    IN PFLT_CONTEXT  Context,
//    IN FLT_CONTEXT_TYPE  ContextType
//    );

//Since we allocated additional memory for Context
//we need free these additional memory.
VOID NlfseStreamHandleContextCleanupCallback(
	__in_opt PFLT_CONTEXT  Context,
    __in FLT_CONTEXT_TYPE  ContextType
	)
{
	PNLFSE_STREAM_CONTEXT pStreamHandleContext = Context;

	if(ContextType != FLT_STREAM_CONTEXT) return;
	if(Context == NULL) return;

	// ref count is maintained by FltXXXContext
	if(NULL != pStreamHandleContext->encryptExt)
	{
		NLFSEFreeEncryptExtension(pStreamHandleContext->encryptExt);
		pStreamHandleContext->encryptExt = NULL;
	}
	if(NULL != pStreamHandleContext->FileName.Buffer)
	{
		ExFreePoolWithTag(pStreamHandleContext->FileName.Buffer, NLFSE_CONTEXT_TAG);
		pStreamHandleContext->FileName.Buffer = NULL;
	}
}

NTSTATUS
NLFSEUpdateStreamHandleContextFileName(
                           __in_opt PCFLT_RELATED_OBJECTS FltObjects,
                           __in_opt PNLFSE_STREAM_CONTEXT StreamHandleContext,
                           __in_opt PFLT_FILE_NAME_INFORMATION FileInfo
                           )
{
    NTSTATUS status = STATUS_SUCCESS;
    USHORT   newFileNameLength = 0;
    PVOID    newFileNameBuffer = NULL;
    NLFSE_PVOLUME_CONTEXT pVolumeContext=NULL;
    UNICODE_STRING        newFileName;

    if(NULL == FltObjects
        || NULL == StreamHandleContext
        || NULL == FileInfo)
        return STATUS_INVALID_PARAMETER;

    // Initialize
    newFileName.Length = 0;
    newFileName.MaximumLength = 0;
    newFileName.Buffer = NULL;

    //Get our volume context 
    status = FltGetVolumeContext( FltObjects->Filter,
        FltObjects->Volume,
        &pVolumeContext);
    if (!NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		    "NLSE!UpdateContextFileName: get volume context err=%x\n",
		    status);
        return status;
    }

	CheckVolumeName(FltObjects->Volume,&pVolumeContext->Name);

    //compose encrypted file name as \\??\\C:\\path\\file-name
    NLFSEGetFileNonUNCNameByFileNameInfo(&newFileName, pVolumeContext, FileInfo);

    // Allocate buffer for new file name
    newFileNameLength = newFileName.Length;
    newFileNameBuffer=ExAllocatePoolWithTag(NonPagedPool, 
					    (newFileNameLength+sizeof(WCHAR)),
					    NLFSE_CONTEXT_TAG );
    if(NULL==newFileNameBuffer)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory( newFileNameBuffer, (newFileNameLength+sizeof(WCHAR)) );

    // Release old file name
    if(NULL != StreamHandleContext->FileName.Buffer)
    {
        ExFreePoolWithTag(StreamHandleContext->FileName.Buffer, NLFSE_CONTEXT_TAG);
        StreamHandleContext->FileName.Buffer = NULL;
        StreamHandleContext->FileName.Length = 0;
        StreamHandleContext->FileName.MaximumLength = 0;
    }

    // Set new file name information
    StreamHandleContext->FileName.Buffer = newFileNameBuffer;
    StreamHandleContext->FileName.Length = newFileNameLength;
    StreamHandleContext->FileName.MaximumLength = newFileNameLength+sizeof(WCHAR);
    RtlCopyUnicodeString( &StreamHandleContext->FileName,  &newFileName);

_exit:
    if(pVolumeContext) FltReleaseContext( pVolumeContext );
    if(newFileName.Buffer) ExFreePool(newFileName.Buffer);
    newFileName.Buffer = NULL;
    newFileName.Length = 0;
    newFileName.MaximumLength = 0;
    return status;
}
