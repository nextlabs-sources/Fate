/*++

Module Name:

  NLSEPolicy.c

Abstract:
  Encryption Policy for system encryption in kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"

//
//  Magic file name
//
const UNICODE_STRING nlfseTestFile[10]={RTL_CONSTANT_STRING(L"nlfse_test.txt"),
				       RTL_CONSTANT_STRING(L"nlfse_test.doc"),
				       RTL_CONSTANT_STRING(L"nlfse_test.ppt"),
				       RTL_CONSTANT_STRING(L"nlfse_test.xls"),
				       RTL_CONSTANT_STRING(L"nlfse_test.pdf"),
				       RTL_CONSTANT_STRING(L"nlfse_test.rtf"),
				       RTL_CONSTANT_STRING(L"nlfse_test.docx"),
				       RTL_CONSTANT_STRING(L"nlfse_test.pptx"),
				       RTL_CONSTANT_STRING(L"nlfse_test.xlsx"),
				       RTL_CONSTANT_STRING(L"nlfse_test.html")
};

extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)
//#define NLSE_DEBUG              1


//Worker thread to encrypt a file and create its ADS
static VOID NLSEEncryptFileWorker(__in PFLT_GENERIC_WORKITEM workItem ,
				  __in PFLT_FILTER filter ,
				  __in PVOID context )
{
  PFLT_FILE_NAME_INFORMATION       fileInfo=NULL;
  NLFSE_PVOLUME_CONTEXT            volCtx = NULL;
  PFLT_VOLUME                      volume=NULL;
  UNICODE_STRING                   fileToEncrypt;
  NTSTATUS                         status;
  NLSE_ENCRYPT_FILE_QUEUE_PCONTEXT pQCtx=NULL;
  NLFSE_ENCRYPT_EXTENSION          dupExt;
  KIRQL                            irql;
  PFLT_CALLBACK_DATA               Data = NULL;


  pQCtx=(NLSE_ENCRYPT_FILE_QUEUE_PCONTEXT)context;
  if(pQCtx==NULL) {
    FltFreeGenericWorkItem(workItem);
    InterlockedDecrement(&nlfseGlobal.encryptWorkQueueSize); 
    return;
  }
  fileInfo=pQCtx->file;
  if(fileInfo == NULL || pQCtx->filterInstance == NULL) {
    goto EncryptFileWorkerCleanup;
  }

  Data = pQCtx->data;

  //Get volume 
  status = FltGetVolumeFromInstance(pQCtx->filterInstance,
				    &volume);
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
	       "NLSE!EncryptFileWorker: get volume, err=%x\n",
	       status );
    volume=NULL;
    goto EncryptFileWorkerCleanup;
  }  
  //Get our volume context.
  status = FltGetVolumeContext( filter,
				volume,
				&volCtx );
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"NLSE!EncryptFileWorker: get volume context, err=%x\n",
		status );
    volCtx=NULL;
    goto EncryptFileWorkerCleanup;
  }

  if(pQCtx->fileStreamCtx->encryptExt == NULL) {
    goto EncryptFileWorkerCleanup;
  }

  //compose encrypted file name as \\??\\C:\\path\\file-name
  NLFSEGetFileNonUNCNameByFileNameInfo(&fileToEncrypt, volCtx, fileInfo);
  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
	      "NLSE!EncryptFileWorker:  encrypted=%wZ\n", 
	      &fileToEncrypt);
    

  //Encrpty the file; store ADS
  //make a copy of encryption extension
  RtlZeroMemory(&dupExt, sizeof(dupExt));
  KeAcquireSpinLock(&pQCtx->fileStreamCtx->encryptExtLock,
		    &irql);    
  RtlCopyMemory(&dupExt, 
		pQCtx->fileStreamCtx->encryptExt, 
		sizeof(NLFSE_ENCRYPT_EXTENSION));
  KeReleaseSpinLock(&pQCtx->fileStreamCtx->encryptExtLock,
		    irql); 
  //encrypt file
  status=NLSEEncryptFile(filter,
			 pQCtx->filterInstance,
			 volCtx, 
			 &fileToEncrypt, 
			 &dupExt);
  if(!NT_SUCCESS(status)) {
    //Free temporary file name buffer
    NLFSEFreeUnicodeNameString(&fileToEncrypt);
    goto EncryptFileWorkerCleanup;
  }

  //Check if the file is deleted
  ExAcquireFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
  if(pQCtx->fileStreamCtx->bDelete == TRUE) {
    //file is being deleted or has been deleted; 
    ExReleaseFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"NLSE!EncryptFileWorker: File is pending for delete %wZ\n", 
		&pQCtx->fileStreamCtx->FileName);	  
    //Free temporary file name buffer
    NLFSEFreeUnicodeNameString(&fileToEncrypt);
    goto EncryptFileWorkerCleanup;
  }
  ExReleaseFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
     
  //update encryption extension in memory
  KeAcquireSpinLock(&pQCtx->fileStreamCtx->encryptExtLock,
		    &irql);    
  RtlCopyMemory(pQCtx->fileStreamCtx->encryptExt, 
		&dupExt,
		sizeof(NLFSE_ENCRYPT_EXTENSION));
  KeReleaseSpinLock(&pQCtx->fileStreamCtx->encryptExtLock,
		    irql);
    
  //Create encryption ADS
  NLSEGenEncryptionADS(filter,
		       pQCtx->filterInstance,
		       &fileToEncrypt,
		       volCtx,
		       &dupExt,
		       pQCtx->pid);
  
  
  //Check again if the file is deleted
  ExAcquireFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
  if(pQCtx->fileStreamCtx->bDelete == TRUE) {
    //file is being deleted or has been deleted; 
    ExReleaseFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"NLSE!EncryptFileWorker: delete ADS of deleted file %wZ\n", 
		&pQCtx->fileStreamCtx->FileName);
    NLSEDeleteEncryptionADS(filter,
			    pQCtx->filterInstance,
			    &fileToEncrypt,
			    volCtx);
  } else {
    ExReleaseFastMutex(&pQCtx->fileStreamCtx->deleteFlagLock);
  }

  //Free temporary file name buffer
  NLFSEFreeUnicodeNameString(&fileToEncrypt);
  
 EncryptFileWorkerCleanup:
  if(volume) {
    FltObjectDereference(volume);
  }
  if(volCtx) {
    FltReleaseContext(volCtx);
  }
  if(fileInfo) {
    FltReleaseFileNameInformation(fileInfo);
  }
  if(pQCtx->filterInstance) {
    FltObjectDereference(pQCtx->filterInstance);
  }
  if(pQCtx->fileStreamCtx) {
    FltReleaseContext( pQCtx->fileStreamCtx);
  }
  ExFreeToNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList,
			       pQCtx );
  FltFreeGenericWorkItem(workItem);
  InterlockedDecrement(&nlfseGlobal.encryptWorkQueueSize);

  FltCompletePendedPostOperation(Data);
}/*NLSEEncryptFileWorker*/

//Check a new file should be encrypted or not
//If yes, return true
BOOLEAN NLSECheckNewFileEncryption(__in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PUNICODE_STRING parentDir,
				   __in PUNICODE_STRING fileNameFinal,
				   __in PUNICODE_STRING fileNameFull,
				   __in ULONG pid)
{  
  int i;
  BOOLEAN hasCurrentPCKey = FALSE;

  //Check its parent directory has NLSE attribute
  if(!NLSECheckDirEncryptionAttribute(FltObjects,parentDir, TRUE))
  {
    return FALSE;
  }

  //If we haven't gotten a valid PC key, try to update it to see if it's
  //available.  If we have already gotten one earlier, try to update it to see
  //if there is a new one or if the current one has been deleted.
  ExAcquireFastMutex(&nlfseGlobal.currentPCKeyLock);
  NLSEUpdateCurrentPCKey(pid, nlfseGlobal.hasCurrentPCKey?TRUE:FALSE);
  hasCurrentPCKey = nlfseGlobal.hasCurrentPCKey;
  ExReleaseFastMutex(&nlfseGlobal.currentPCKeyLock);

  //If we still don't have a valid PC key, we can't encrypt the new file.
  if (hasCurrentPCKey) {
    return TRUE;
  }

#ifdef NLSE_DEBUG
  //for spike test
  for( i=0; i < 10 ;++i) {
    if (RtlCompareUnicodeString(fileNameFinal,
				&nlfseTestFile[i],
				TRUE ) == 0) {
      /*NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
	("NLFSE!CheckPolicy: Found the matching file %wZ. \n", 
	irpEntry->FromFileName));*/
      return TRUE;
    }
  }
#endif
  return FALSE;
}
				      
//Check if the target of renamed file should be encrypted; if yes, encrypt it
void NLFSECheckAndEncryptRenamedFile(__in PCFLT_RELATED_OBJECTS fltObjects,
				     __in PFLT_CALLBACK_DATA Data,
				     __in PFLT_FILE_NAME_INFORMATION target,
				     __in PFLT_FILE_NAME_INFORMATION src,
				     __inout BOOLEAN *bReleaseResource)
{
	BOOLEAN                          bNeedEncrypt=FALSE;
	NTSTATUS                         status;
	NLSE_ENCRYPT_FILE_QUEUE_PCONTEXT pQCtx=NULL;
	PFLT_GENERIC_WORKITEM            wi=NULL;
	ULONG                            pid;

	*bReleaseResource=TRUE;

	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		  "NLSE!CheckAndEncryptRenamed: source=%wZ \n\t target=%wZ--\n", 
		  &src->Name, &target->Name);

	pid = FltGetRequestorProcessId(Data); 
	if(!NLSECheckNewFileEncryption(fltObjects,
				&target->ParentDir,
				&target->FinalComponent,
				&target->Name,
				pid))
	{
	  return;
	}
  
    //encrypt the target file; queue this task to a worker thread
    status = FltObjectReference(fltObjects->Instance);
    if(!NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		  "NLSE!CheckAndEncryptRename: reference instance err=0x%x\n",
		  status );
      goto CheckAndEncryptRenamedCleanup;
    }

    //allocate work queue context 
    pQCtx=ExAllocateFromNPagedLookasideList(&nlfseGlobal.fileEncryptCtxList);
    if (pQCtx == NULL) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		  "NLSE!CheckAndEncryptRename: Failed to allocate context\n");
      FltObjectDereference(fltObjects->Instance);
      goto CheckAndEncryptRenamedCleanup;
    }
    RtlZeroMemory(pQCtx, sizeof( NLSE_ENCRYPT_FILE_QUEUE_CONTEXT));
    pQCtx->filterInstance=fltObjects->Instance;
    pQCtx->file=target;
    pQCtx->pid=pid;
    pQCtx->data = Data;

    //attach per stream context to the fileobject
    //status=NLSEPostRenameContextSetup(fltObjects, target, Data, &pQCtx->fileStreamCtx);
	{
		PNLFSE_STREAM_CONTEXT pStreamHandleContext=NULL;
		NLFSE_PVOLUME_CONTEXT pVolumeContext = NULL;
		PNLFSE_ENCRYPT_EXTENSION pEncryptExt=NULL;
		UNICODE_STRING           encryptFileName;

		status = FltGetStreamContext( fltObjects->Instance,
						fltObjects->FileObject,
                        &pStreamHandleContext );

		if (!NT_SUCCESS(status))
		{
			//try to Create a new  stream context
			status = FltGetVolumeContext( fltObjects->Filter, fltObjects->Volume, &pVolumeContext);			
			if (NT_SUCCESS( status ))
			{
				//compose encrypted file name as \\??\\C:\\path\\file-name
				NLFSEGetFileNonUNCNameByFileNameInfo(&encryptFileName, pVolumeContext, target);
				
				//create encryption extension
				pEncryptExt=NLFSEAllocateEncryptExtension(0);      
				
				//Get data encryption key
				if(NULL!=pEncryptExt)
				{
					//generate key
					NLSEGenDataEncryptionKey(pEncryptExt);

					status = FltAllocateContext( fltObjects->Filter,
                                 FLT_STREAM_CONTEXT,
                                 sizeof(NLFSE_STREAM_CONTEXT),
                                 NonPagedPool,
                                 &pStreamHandleContext );
					if (NT_SUCCESS(status))
					{
						if(NlfseInitPerStreamHandleContext(
							&encryptFileName,
							pEncryptExt,
							&pStreamHandleContext))
						{
							//NLFSELinkContext( volCtx, FltObjects->FileObject, irpEntry, &pStreamContext ))
							pStreamHandleContext->Stream = FsRtlGetPerStreamContextPointer(fltObjects->FileObject);
							// set the context
							(VOID) FltSetStreamContext( fltObjects->Instance,
													  fltObjects->FileObject,
													  FLT_SET_CONTEXT_REPLACE_IF_EXISTS,
													  pStreamHandleContext,
													  NULL );

						}
						else
						{
							// failed to init the context
							NLFSEFreeEncryptExtension(pEncryptExt);
							FltReleaseContext(pStreamHandleContext);//delete the new created
							pStreamHandleContext = NULL;
						}
					}
					else
					{
						// failed to allocate the context
						NLFSEFreeEncryptExtension(pEncryptExt);
						pStreamHandleContext = NULL;
					}
					
				}
				NLFSEFreeUnicodeNameString(&encryptFileName);
				FltReleaseContext( pVolumeContext );
			}
		}

		pQCtx->fileStreamCtx = pStreamHandleContext;//remember to release it cause both get and set increase the ref-count
	}
    
	if(NULL == pQCtx->fileStreamCtx)
	{
		NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		  "NLSE!CheckAndEncryptRename: setup stream context err0x%x\n",
		  status);
		FltObjectDereference(pQCtx->filterInstance);
		ExFreeToNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList, pQCtx );
		goto CheckAndEncryptRenamedCleanup;
    }

    //Queue the encryption work
    wi = FltAllocateGenericWorkItem();
    if( wi != NULL )
	{
		LONG qsize = InterlockedIncrement(&nlfseGlobal.encryptWorkQueueSize);
		if( qsize < NLFSE_MAX_WORK_QUEUE_SIZE )
		{
			status = FltQueueGenericWorkItem(wi,
					 (PVOID)fltObjects->Filter,
					 NLSEEncryptFileWorker,
					 CriticalWorkQueue,
					 (PVOID)pQCtx);
			if(!NT_SUCCESS(status))
			{
				FltReleaseContext( pQCtx->fileStreamCtx );
				FltObjectDereference(pQCtx->filterInstance);
				ExFreeToNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList, pQCtx );
				FltFreeGenericWorkItem(wi);
				NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
				  "NLSE!CheckAndEncryptRename: Fail to queue item 0x%x\n",
				  status);
				InterlockedDecrement(&nlfseGlobal.encryptWorkQueueSize);
				goto CheckAndEncryptRenamedCleanup;
			}
		}
		else
		{
			InterlockedDecrement(&nlfseGlobal.encryptWorkQueueSize);
			FltReleaseContext( pQCtx->fileStreamCtx );
			FltObjectDereference(pQCtx->filterInstance);
			ExFreeToNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList, pQCtx );
			FltFreeGenericWorkItem(wi);
			NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
					"NLSE!CheckAndEncryptRename: exceed queue sizen\n");
			goto CheckAndEncryptRenamedCleanup;
		}
	}
	else
	{
		FltReleaseContext( pQCtx->fileStreamCtx );
		FltObjectDereference(pQCtx->filterInstance);
		ExFreeToNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList, pQCtx );
		NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
			("NLSE!CheckAndEncryptRename: fail to allocate work item\n"));
		goto CheckAndEncryptRenamedCleanup;
    }
    
    //Not release resource in order to encrypt the target file in 
    //a worker thread
    *bReleaseResource=FALSE;
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"NLSE!CheckAndEncryptRenamed: Queue encrypt file %wZ \n", 
		&target->Name);

CheckAndEncryptRenamedCleanup:
  return;
}/*NLFSECheckAndEncryptRenamedFile*/

//For a new file, we check if the file is pre-set to be encrypted
//For existing file, we check if we can decrypt the data encryption key.
//If the file should be encrypted, set irpEntry->NeedEncrypt to TRUE
FLT_PREOP_CALLBACK_STATUS 
NLFSECheckFileEncryption(__in PCFLT_RELATED_OBJECTS FltObjects,
			 __inout_opt NLFSE_PIRP_ENTRY irpEntry,
			 __inout PFLT_CALLBACK_DATA Data)
{
  char                      pcKey[NLSE_KEY_LENGTH_IN_BYTES]={0};
  FLT_PREOP_CALLBACK_STATUS returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
  PNLFSE_ENCRYPT_EXTENSION  pEncryptExt=NULL;
  ULONG                     pid;
  BOOLEAN                   bGetPCKeyFailed=FALSE;
	ULONG					RemoveFlag;

  //Sanity checking
  if(irpEntry == NULL) {
    return returnStatus;
  } else if(irpEntry->bExist && irpEntry->pEncryptExtension == NULL) {
    //For an existing file, it doesn't have NLSE ADS; do nothing
    return returnStatus;
  }

  pid = FltGetRequestorProcessId(Data); 
  if(irpEntry->bExist) {
    //an existing file; 
    //Acquire PC key
    pEncryptExt=irpEntry->pEncryptExtension;
    if(NLSEGetPCKeyByID(pEncryptExt->pcKeyRingName, 
			&pEncryptExt->pcKeyID, 
			pid, 
			pcKey)) {
      //decrypt the memory storing data encryption key
      NLSEDecryptIndicator(pEncryptExt, pcKey, NLSE_KEY_LENGTH_IN_BYTES); 
      RtlSecureZeroMemory(pcKey, NLSE_KEY_LENGTH_IN_BYTES);

      // Here we check if the key is the latest one
      // If it is not, then we need to update ADS with the latest key
      NLSEUpdateADSKeyId(FltObjects, pid, &(irpEntry->fileName), pEncryptExt);

      //File has ADS; ADS information is copied into irpEntry
      irpEntry->NeedEncrypt=TRUE;
      returnStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    } else {
      //Fail to get key; 
      Data->IoStatus.Status=STATUS_ACCESS_DENIED;
      returnStatus=FLT_PREOP_COMPLETE;
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		  "NLSE!CheckFileEncryption: get PC key failed (%wZ)\n", 
		  &irpEntry->fileName);
    }
  } else {
    //A new file, check if this file should be encrypted.
    if(NLSECheckNewFileEncryption(FltObjects,
				  &irpEntry->fileParentDir,
				  &irpEntry->fileNameFinal,
				  &irpEntry->fileName,
				  pid)) {
      //NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
      //	  "NLSE!CheckFileEncryption: encrypt new file %wZ \n",
      //	  &irpEntry->fileName);      
      irpEntry->NeedEncrypt=TRUE;

	  // if has EFS attribute, remove it

      // If it is a new create file with EFS/Compress flag, we don't need to handle it
      if(Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED
	 || Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_COMPRESSED)
      {
	RemoveFlag = FILE_ATTRIBUTE_ENCRYPTED;
	Data->Iopb->Parameters.Create.FileAttributes &= (~RemoveFlag);
	RemoveFlag = FILE_ATTRIBUTE_COMPRESSED;
	Data->Iopb->Parameters.Create.FileAttributes &= (~RemoveFlag);

	FltSetCallbackDataDirty(Data);
      }

      returnStatus=FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }
  }

  return returnStatus;
}/*NLFSECheckFileEncryption*/

VOID 
NLSEHandlePendedIO(__in NLSE_PENDING_IO_QUEUE_PCONTEXT queueCtx,
		   __in PFLT_CALLBACK_DATA Data) 
{
  NLFSE_PIRP_ENTRY irpEntry;
  NLFSE_PPRE_2_POST_CONTEXT p2pCtx;
  FLT_PREOP_CALLBACK_STATUS status;
  FLT_POSTOP_CALLBACK_STATUS postStatus;

  switch(Data->Iopb->MajorFunction)
    {
    case IRP_MJ_CREATE:
      irpEntry=queueCtx->irpEntry;
      status=NLFSECheckFileEncryption(queueCtx->FltObjects,irpEntry, Data);
      if (status == FLT_PREOP_SUCCESS_WITH_CALLBACK) {
	// Complete the I/O with callback
	FltCompletePendedPreOperation( Data,
				       FLT_PREOP_SUCCESS_WITH_CALLBACK,
				       irpEntry );      
      } else if(status == FLT_PREOP_SUCCESS_NO_CALLBACK) {
	// Complete the I/O without callback
	// Don't need this IRP entry any more.
	NLFSEFreeIRPEntry(irpEntry);
	FltCompletePendedPreOperation( Data,
				       FLT_PREOP_SUCCESS_NO_CALLBACK,
				       NULL );
      } else {
	//Operation denied; discard the irp entry
	NLFSEFreeIRPEntry(irpEntry);
	FltCompletePendedPreOperation( Data,
				       status,
				       NULL );	  
      }
      break;
    default:
      break;
    }
}
