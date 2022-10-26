/*++

Module Name:

    NFSEUtility.c

Abstract:
  Utility Functions for system encryption in Kernel mode

Environment:

    Kernel mode

--*/
#include <FltKernel.h>
#include <Ntifs.h>
#include "NLSEStruct.h"
#include "NLSEUtility.h"

//Global variables
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;

//Local global variables
static UNICODE_STRING NtfsReservedNames[] = {
    { 8, 10, L"$Mft"},
    {16, 18, L"$MftMirr"},
    {16, 18, L"$LogFile"},
    {14, 16, L"$Volume"},
    {16, 18, L"$AttrDef"},
    {14, 16, L"$Bitmap"},
    {10, 12, L"$Boot"},
    {16, 18, L"$BadClus"},
    {14, 16, L"$Secure"},
    {14, 16, L"$UpCase"},
    {14, 16, L"$Extend"},
    { 0,  0, NULL}
};
static UNICODE_STRING ExtendDir = {18, 20, L"\\$Extend\\"};
static UNICODE_STRING NtfsExtendNames[] = {
    {14, 16, L"$Config"},
    {14, 16, L"$Delete"},
    {12, 14, L"$ObjId"},
    {12, 14, L"$Quota"},
    {14, 16, L"$Repair"},
    {22, 24, L"$Repair.log"},
    {16, 18, L"$Reparse"},
    {22, 24, L"$RmMetadata"},
    {10, 12, L"$Tops"},
    { 8, 10, L"$Txf"},
    {14, 16, L"$TxfLog"},
    { 0,  0, NULL}
};

#define GetNextFileStreamInformation(x) ((0!=x->NextEntryOffset)?(PFILE_STREAM_INFORMATION)((PUCHAR)x + x->NextEntryOffset):NULL)

//Copy the source string to the destination string
__checkReturn
NTSTATUS 
NLSEAllocateAndCopyUnicodeNameString(__out_opt PUNICODE_STRING  DestinationString,
				     __in PUNICODE_STRING  SourceString)
{
  //sanity checking
  if(DestinationString ==NULL) return STATUS_INVALID_PARAMETER;

  //initialization
  DestinationString->Buffer = NULL;
  DestinationString->Length = DestinationString->MaximumLength = 0;

  //If source string is empty, doing nothing 
  if(SourceString == NULL) return STATUS_SUCCESS;
  if(SourceString->Buffer == NULL) return STATUS_SUCCESS;
  if(SourceString->Length ==0) return STATUS_SUCCESS;

  //setup length and allocate buffer
  DestinationString->Length = SourceString->Length;
  if(DestinationString->Length >= NLFSE_NAME_LEN * sizeof(WCHAR)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
	      "NLSE!AllocateAndCopyUnicodeNameString: String(%d %wZ) longer than NLFSE_NAME_LEN(%d) defined for entry length in fileNameLookaside list. The string is to be tructed to %d long\n",
	       DestinationString->Length/sizeof(WCHAR), 
	      &SourceString, NLFSE_NAME_LEN, NLFSE_NAME_LEN-1);
    DestinationString->Length=(NLFSE_NAME_LEN-1)*sizeof(WCHAR);
  }
  DestinationString->MaximumLength = NLFSE_NAME_LEN*sizeof(WCHAR);
  DestinationString->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_ACOPYSTRING_TAG);

  //do copy if buffer allocated successfully
  if(DestinationString->Buffer)	{
    RtlZeroMemory(DestinationString->Buffer,DestinationString->MaximumLength);
    RtlCopyMemory(DestinationString->Buffer,
		  SourceString->Buffer,DestinationString->Length);	
    return STATUS_SUCCESS;
  } else {
    DestinationString->Length = DestinationString->MaximumLength = 0;
    return STATUS_INSUFFICIENT_RESOURCES;
  }
}/*AllocateAndCopyUnicodeNameString*/

//Free the buffer for the input name string
VOID
NLFSEFreeUnicodeNameString(__inout_opt PUNICODE_STRING  SourceString)
{
  if(SourceString == NULL) return;

  if(SourceString->Buffer == NULL) return;

  //ExFreeToNPagedLookasideList( &nlfseGlobal.fileNameLookaside,
  //			       SourceString->Buffer);
  ExFreePool(SourceString->Buffer);

}/*FreeUnicodeNameString*/

//Free an IRP entry
VOID
NLFSEFreeIRPEntry(__inout_opt NLFSE_PIRP_ENTRY irpEntry)
{
  if(!irpEntry) return;
  
  if(irpEntry->pEncryptExtension) {
    NLFSEFreeEncryptExtension(irpEntry->pEncryptExtension);
    irpEntry->pEncryptExtension=NULL;
  }

  if(irpEntry->fileParentDir.Buffer) {
    NLFSEFreeUnicodeNameString(&irpEntry->fileParentDir);
  }
  
  if(irpEntry->fileNameFinal.Buffer) {
    NLFSEFreeUnicodeNameString(&irpEntry->fileNameFinal);
  }

  if(irpEntry->fileName.Buffer) {
    NLFSEFreeUnicodeNameString(&irpEntry->fileName);
  }

  ExFreeToNPagedLookasideList( &nlfseGlobal.irpEntryLookaside,
			       irpEntry);
}/*NLFSEFreeIRPEntry*/

//Get the name (path and file-name-only) of the file based on 
//the callback data of an IO operation
BOOLEAN
NLFSEAllocateAndGetFileName(__inout PFLT_CALLBACK_DATA Data,
			    __in PCFLT_RELATED_OBJECTS FltObjects,
			    __out PUNICODE_STRING Name,
                __out PUNICODE_STRING parentDir,
                __out PUNICODE_STRING VolName)
{

  //  NOTE: By default, we use the query method
  //  FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP
  //  because MiniSpy would like to get the name as much as possible, but
  //  can cope if we can't retrieve a name.  For a debugging type filter,
  //  like Minispy, this is reasonable, but for most production filters
  //  who need names reliably, they should query the name at times when it
  //  is known to be safe and use the query method
  //  FLT_FILE_NAME_QUERY_DEFAULT.
  //
  //  In create, hard-link, 
  //  and rename operations, file name tunneling can cause 
  //  the final component in normalized file name information that a minifilter 
  //  driver retrieves in a preoperation callback routine to be invalidated. 
  //  If a minifilter driver retrieves 
  //  normalized file name information in a preoperation callback 
  //  (PFLT_PRE_OPERATION_CALLBACK) 
  //  routine by calling a routine such as FltGetFileNameInformation, 
  //  it must call FltGetTunneledName 
  //  from its postoperation callback (PFLT_POST_OPERATION_CALLBACK) 
  //  routine to retrieve the correct
  //  file name information for the file. 

  NTSTATUS status;
  BOOLEAN Ret = FALSE;
  PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

  if (FltObjects->FileObject != NULL) {
    status = FltGetFileNameInformation(Data,
				       FLT_FILE_NAME_NORMALIZED |
				       FLT_FILE_NAME_QUERY_DEFAULT,
				       &nameInfo );
    if (NT_SUCCESS( status )) {			
      status = FltParseFileNameInformation( nameInfo );
      if(NT_SUCCESS( status )) {
	if(nameInfo->Stream.Length) goto EXIT;	// ignore all stream
	if(nameInfo->Share.Length) goto EXIT;	// ignore all remote access
	if(!nameInfo->FinalComponent.Length) goto EXIT;
	if(!nameInfo->FinalComponent.Buffer) goto EXIT;
	if(STATUS_SUCCESS!=NLSEAllocateAndCopyUnicodeNameString(Name,
						&nameInfo->FinalComponent))
	  goto EXIT;
	if(nameInfo->ParentDir.Length != 0 &&
	   nameInfo->ParentDir.Buffer) {
	  NLSEAllocateAndCopyUnicodeNameString(parentDir, 
					       &nameInfo->ParentDir); 
	}
    if(nameInfo->Volume.Length !=0
        && NULL!=nameInfo->Volume.Buffer)
    {
        NLSEAllocateAndCopyUnicodeNameString(VolName, &nameInfo->Volume); 
    }
	Ret = TRUE;
      }			
    }
  }
EXIT:
  if(nameInfo) FltReleaseFileNameInformation( nameInfo );
  return Ret;
}

//Allocate an encrypt extension (a.k.a. encryption footer)
PNLFSE_ENCRYPT_EXTENSION
NLFSEAllocateEncryptExtension(__in ULONGLONG RealLength)
{
  PNLFSE_ENCRYPT_EXTENSION pExt;
  STRING                   tmpString;

  pExt = ExAllocatePoolWithTag(NonPagedPool, 
			       sizeof(NLFSE_ENCRYPT_EXTENSION),
			       NLFSE_BUFFER_TAG);
  if(pExt == NULL)
  {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
		"NLSE!AllocateEncryptExtension: can't allocate %d memory\n",
		sizeof(NLFSE_ENCRYPT_EXTENSION));
    return NULL;
  }
  RtlZeroMemory(pExt, sizeof(NLFSE_ENCRYPT_EXTENSION));  

  pExt->sh.stream_size=sizeof(NLFSE_ENCRYPT_EXTENSION);
  RtlInitString(&tmpString,	NLSE_STREAM_NAME);
  RtlCopyMemory(pExt->sh.stream_name, tmpString.Buffer,	tmpString.Length);  
  pExt->version_major=1;
  pExt->version_minor=0;
  pExt->fileRealLength=RealLength;
  return pExt;
}/*NLFSEAllocateEncryptExtension*/

//Free an encrypt extension (a.k.a. encryption footer)
void
NLFSEFreeEncryptExtension(__out PNLFSE_ENCRYPT_EXTENSION pExt)
{
  if(pExt == NULL) return;
  RtlSecureZeroMemory(pExt, sizeof(NLFSE_ENCRYPT_EXTENSION));
  ExFreePoolWithTag(pExt, NLFSE_BUFFER_TAG);
}

//compose file name as \??\C:\path\file-name
void NLFSEGetFileNonUNCName(__inout PUNICODE_STRING outFileName,
			    __in NLFSE_PVOLUME_CONTEXT volCtx,
			    __in_opt PUNICODE_STRING inFileNameFinal,
			    __in PUNICODE_STRING inParentDir)
{
	BOOLEAN AddPrefix = TRUE;

  outFileName->Length=0;
  outFileName->MaximumLength = volCtx->Name.Length; //driver name length
  if(inParentDir) {
    outFileName->MaximumLength += inParentDir->Length; //path length
  }
  if(inFileNameFinal) {
    outFileName->MaximumLength += inFileNameFinal->Length; //name length
  }
  outFileName->MaximumLength += 4*sizeof(WCHAR)+sizeof(WCHAR); // prefix length
  if(outFileName->MaximumLength > NLFSE_NAME_LEN*sizeof(WCHAR)) {
    if(inParentDir && inFileNameFinal) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "NLFSE!NLFSEGetFileNonUNCName: String(%d %wZ%wZ)longer than NLFSE_NAME_LEN(%d) defined for entry length in fileNameLookaside list. The string is to be tructed to %d long\n",
		 outFileName->MaximumLength/sizeof(WCHAR), 
		 inParentDir,
		 inFileNameFinal,
		 NLFSE_NAME_LEN, 
		  NLFSE_NAME_LEN-1);
    }
    outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  }
  //To be safe, set maximumlength as the length of lookaside buffer entry
  outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  outFileName->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,(NLFSE_NAME_LEN)*sizeof(WCHAR),NLSE_FILENONUNC_TAG);
  if(outFileName->Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"[NLFSE]!GetFileNonUNCName: failed to allocate %d bytes memory.\n",
		outFileName->MaximumLength);
    return;
  }

  RtlZeroMemory(outFileName->Buffer, outFileName->MaximumLength);
  // if the volCtx->Name is like L"\\?\Volume{ae3644a0-9638-11df-aec6-00155d63712f}",
  // do not add the L"\\??\\"
  if(volCtx->Name.Length>=4)
  {
	if(volCtx->Name.Buffer[0] == L'\\' && volCtx->Name.Buffer[1] == L'\\' &&
		volCtx->Name.Buffer[2] == L'?' && volCtx->Name.Buffer[3] == L'\\')
	{
		AddPrefix = FALSE;
	}
  }
  if(AddPrefix)
  {
	RtlAppendUnicodeToString(outFileName, L"\\??\\");
  }
  if(volCtx->Name.Buffer) {
    RtlAppendUnicodeStringToString(outFileName, &volCtx->Name);
  }
  if(inParentDir && inParentDir->Buffer) {
    RtlAppendUnicodeStringToString(outFileName, inParentDir);
  }
  if(inFileNameFinal && inFileNameFinal->Buffer) {
    RtlAppendUnicodeStringToString(outFileName, inFileNameFinal); 
  }
}/*NLFSEGetFileNonUNCName*/

//compose directory name without volume,e.g. folder\sub1\sub2
void NLSEComposeDirNameNoVolume(__inout PUNICODE_STRING outFileName,
				 __in_opt PUNICODE_STRING inFileNameFinal,
				__in PUNICODE_STRING inParentDir)
{
  outFileName->Length=0;
  outFileName->MaximumLength = 0;
  if(inParentDir) {
    outFileName->MaximumLength += inParentDir->Length; //path length
  }
  if(inFileNameFinal) {
    outFileName->MaximumLength += inFileNameFinal->Length; //name length
  }
  if(outFileName->MaximumLength > NLFSE_NAME_LEN*sizeof(WCHAR)) {
    if(inParentDir && inFileNameFinal) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "NLSE!ComposeDirNameNoVolume: String(%d %wZ%wZ)longer than NLFSE_NAME_LEN(%d) defined for entry length in fileNameLookaside list. The string is to be tructed to %d long\n",
		 outFileName->MaximumLength/sizeof(WCHAR), 
		 inParentDir,
		 inFileNameFinal,
		 NLFSE_NAME_LEN, 
		  NLFSE_NAME_LEN-1);
    }
    outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  }
  //To be safe, set maximumlength as the length of lookaside buffer entry
  outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  outFileName->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,(NLFSE_NAME_LEN)*sizeof(WCHAR),NLSE_COMPOSEDIRTOVOL_TAG);
  if(outFileName->Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"[NLSE]!ComposeDirNameNoVolume: failed to allocate %d bytes memory.\n",
		outFileName->MaximumLength);
    return;
  }

  RtlZeroMemory(outFileName->Buffer, outFileName->MaximumLength);
  if(inParentDir && inParentDir->Buffer) {
    RtlAppendUnicodeStringToString(outFileName, inParentDir);
  }
  if(inFileNameFinal && inFileNameFinal->Buffer) {
    RtlAppendUnicodeStringToString(outFileName, inFileNameFinal); 
  }
}/*NLSEComposeDirNameNoVolume*/

//NLFSE Allocate an IRP Entry
NLFSE_PIRP_ENTRY
NLFSEAllocateIRPEntry(__in PCFLT_RELATED_OBJECTS FltObjects,
			  __in NLFSE_PVOLUME_CONTEXT volCtx,
		      __in PUNICODE_STRING fileParentDir,
		      __in PUNICODE_STRING fileNameFinal)
{
  NLFSE_PIRP_ENTRY          irpEntry;

  irpEntry = ExAllocateFromNPagedLookasideList(&nlfseGlobal.irpEntryLookaside);
  if(irpEntry == NULL) 
    return NULL;

  //Initialize the entry
  RtlZeroMemory(irpEntry,sizeof(NLFSE_IRP_ENTRY));
  irpEntry->bExist=FALSE;


  //assign values to irp entry
  NLSEAllocateAndCopyUnicodeNameString(&irpEntry->fileNameFinal,
				       fileNameFinal);
  NLSEAllocateAndCopyUnicodeNameString(&irpEntry->fileParentDir,
				       fileParentDir);
  
  //Get FromFileName in Non-UNC
  CheckVolumeName(FltObjects->Volume,&volCtx->Name);

  //compose file name as \??\C:\path\file-name
  NLFSEGetFileNonUNCName(&irpEntry->fileName,
			   volCtx,
			   &irpEntry->fileNameFinal,
			   &irpEntry->fileParentDir);
 
  return irpEntry;
}

VOID
NLSEUpdateADSKeyId(__in PCFLT_RELATED_OBJECTS FltObjects,
                   __in ULONG ProcessId,
                   __in PUNICODE_STRING FileName,
                   __inout PNLFSE_ENCRYPT_EXTENSION  EncryptExt
                   )
{
    PNLFSE_ADS_WORKITEM     workItem = NULL;
    NTSTATUS                Status   = STATUS_SUCCESS;
    FLT_VOLUME_PROPERTIES   vp = {0};
    ULONG                   ulVolPropSize = 0;
    ULONG                   ulRet = 0;

    ExAcquireFastMutex(&nlfseGlobal.currentPCKeyLock);
    NLSEUpdateCurrentPCKey(ProcessId, TRUE);
    ExReleaseFastMutex(&nlfseGlobal.currentPCKeyLock);
    if(!nlfseGlobal.hasCurrentPCKey)
        return;

    if(sizeof(NLSE_KEY_ID) != RtlCompareMemory(&nlfseGlobal.currentPCKeyID, &EncryptExt->pcKeyID, sizeof(NLSE_KEY_ID)))
    {
        Status = FltGetVolumeProperties(FltObjects->Volume, &vp, sizeof(FLT_VOLUME_PROPERTIES), &ulVolPropSize);
        if(STATUS_SUCCESS==Status
            || STATUS_BUFFER_OVERFLOW==Status)
        {
            RtlCopyMemory(&EncryptExt->pcKeyID, &nlfseGlobal.currentPCKeyID, sizeof(NLSE_KEY_ID));
#pragma prefast(disable:28197, "Possibly leaking memory \"workItem\" 28197 - memory Leak") 
            // It will be released in work item handle routine
            workItem = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLFSE_ADS_WORKITEM), NLFSE_BUFFER_TAG);
#pragma prefast(enable:28197, "recover this warning")
            if(NULL != workItem)
            {
                RtlZeroMemory(workItem, sizeof(NLFSE_ADS_WORKITEM));
                FltObjectReference(FltObjects->Instance);
                workItem->fltInstance               = FltObjects->Instance;
                workItem->RetryCount                = 0;
                workItem->ProcessId                 = ProcessId;
                workItem->SectorSize                = vp.SectorSize;
                workItem->hostFilePath.MaximumLength= sizeof(WCHAR)*(2*MAX_PATH);
                workItem->hostFilePath.Length       = FileName->Length;
                workItem->hostFilePath.Buffer       = &(workItem->hostFilePathBuffer[0]);
                RtlCopyMemory(workItem->hostFilePathBuffer, FileName->Buffer, FileName->Length);
                RtlCopyMemory(&workItem->encryptExt, EncryptExt, sizeof(NLFSE_ENCRYPT_EXTENSION));
                NLSEQueueAdsWorkItem(workItem); workItem=NULL;
            }
        }
    }
}

//Check if the IO operated file exists or not
BOOLEAN
NLFSECheckFileExist(__in  PCFLT_RELATED_OBJECTS fltObjects,
		    __in  NLFSE_PVOLUME_CONTEXT volCtx,
            __in  NLFSE_PIRP_ENTRY      irpEntry,
            __in  BOOLEAN               OverWrite,
		    __out  PNLFSE_STREAM_CONTEXT *pContext,
		    __out BOOLEAN *bNotFile,
		    __out BOOLEAN *bWinEncrypted,
		    __out BOOLEAN *bWinCompressed)
{
    FILE_STANDARD_INFORMATION FileStdInfo;
    FILE_BASIC_INFORMATION    FileBasicInfo;
    PFLT_FILTER               filter=fltObjects->Filter;
    PFLT_INSTANCE             filterInstance=fltObjects->Instance;
    OBJECT_ATTRIBUTES         DummyObjectAttributes;
    IO_STATUS_BLOCK           DummyIoStatusBlock;
    HANDLE                    DummyFileHandle = NULL;
    PFILE_OBJECT              DummyFileObject = NULL;
    NTSTATUS                  DummyStatus = STATUS_UNSUCCESSFUL;
    NTSTATUS                  Status;
    BOOLEAN                   bFileExist=FALSE;

    //initialization
    *pContext=NULL;
    *bNotFile=FALSE;
    *bWinEncrypted = FALSE;
    *bWinCompressed= FALSE;
    RtlZeroMemory(&FileStdInfo,sizeof(FileStdInfo));
    RtlZeroMemory(&FileBasicInfo,sizeof(FileBasicInfo));

    //Prepare open file
    InitializeObjectAttributes(&DummyObjectAttributes,
        &irpEntry->fileName,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    //open the file
    DummyStatus = FltCreateFile(filter,
        filterInstance,
        &DummyFileHandle,
        GENERIC_READ,
        &DummyObjectAttributes,
        &DummyIoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(NT_SUCCESS(DummyStatus))
    {
        //file exists
        if(!OverWrite)
            bFileExist = TRUE;
        // Get file object from the handle
        DummyStatus = ObReferenceObjectByHandle(DummyFileHandle, //Handle
            0,               //DesiredAccess
            NULL,              //ObjectType
            KernelMode,        //AccessMode
            &DummyFileObject,  //File Handle
            NULL);
        if (NT_SUCCESS(DummyStatus))
        {
            // get file basic information
            DummyStatus = FltQueryInformationFile(filterInstance,
                DummyFileObject,
                &FileBasicInfo,
                sizeof(FileBasicInfo),
                FileBasicInformation,
                NULL);
            if(NT_SUCCESS(DummyStatus))
            {
                if(FileBasicInfo.FileAttributes&FILE_ATTRIBUTE_ENCRYPTED)
                    *bWinEncrypted = TRUE;
                if(FileBasicInfo.FileAttributes&FILE_ATTRIBUTE_COMPRESSED)
                    *bWinCompressed = TRUE;
            }

            //get fileinfo
            DummyStatus = FltQueryInformationFile(filterInstance,
                DummyFileObject,
                &FileStdInfo,
                sizeof(FileStdInfo),
                FileStandardInformation,
                NULL);
            if(NT_SUCCESS(DummyStatus))
            {
                // check if it's directory
                if(FileStdInfo.Directory)
                {
                    *bNotFile=TRUE; //a directory
                }
                else
                {
                    //a file; check if the file object will have per-stream attached
                    //after IRP_MJ_CREATE
                    //*pContext=NLFSEFindExistingContext(volCtx, DummyFileObject);
					DummyStatus = FltGetStreamContext( filterInstance,
                                        DummyFileObject,
                                        pContext );
                    if(!NT_SUCCESS(DummyStatus) || *pContext == NULL)
                    {
                        //For an existing file, check if the file has ADS stream;
                        //If yes, read it but not to acquire PC key for now;
                        if(!OverWrite)
                            irpEntry->pEncryptExtension=NLSEOpenAndReadADS(filter, filterInstance, &irpEntry->fileName, volCtx);
                    }
                    else
                    {
                        if(OverWrite)
                        {
                            // Remove per-stream context
							FltDeleteContext(*pContext);//try to delete it
							FltReleaseContext(*pContext);
                            *pContext = NULL;
                        }
                    }
                }
            }
            else
            {
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                    "NLFSE!CheckFileExist: query file failed, 0x%x\n",
                    DummyStatus);
            }
            ObDereferenceObject(DummyFileObject);
        }

        FltClose(DummyFileHandle);
    }

    //return
    return bFileExist;
}

//compose file name as \??\C:\path\file-name
void NLFSEGetFileNonUNCNameByFileNameInfo(__out PUNICODE_STRING outFileName,
					  __in NLFSE_PVOLUME_CONTEXT volCtx,
					  __in PFLT_FILE_NAME_INFORMATION fileNameInfo)
{
  BOOLEAN AddPrefix = TRUE;

  outFileName->Length=0;
  outFileName->MaximumLength = volCtx->Name.Length; //driver name length
  outFileName->MaximumLength += fileNameInfo->Name.Length; //file name length
  outFileName->MaximumLength += 4*sizeof(WCHAR)+sizeof(WCHAR); // prefix length
  
  if(outFileName->MaximumLength > NLFSE_NAME_LEN*sizeof(WCHAR)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLFSE!NLFSEGetFileNonUNCNameByFileNameInfo: String(%d %wZ%wZ)longer than NLFSE_NAME_LEN(%d) defined for entry length in fileNameLookaside list. The string is to be tructed to %d long\n",
		outFileName->MaximumLength/sizeof(WCHAR), 
		&fileNameInfo->ParentDir,
		&fileNameInfo->FinalComponent,
		NLFSE_NAME_LEN, NLFSE_NAME_LEN-1);
    outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  }
  //To be safe, set maximumlength as the length of lookaside buffer entry
  outFileName->MaximumLength=(NLFSE_NAME_LEN)*sizeof(WCHAR);
  outFileName->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,(NLFSE_NAME_LEN)*sizeof(WCHAR),NLSE_FILENONUNCNAMEINFO_TAG);

  if(outFileName->Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!GetFileNonUNCNameByFileNameInfo: failed to allocate %d bytes memory.\n",
		outFileName->MaximumLength);
    return;
  }

  // if the volCtx->Name is like L"\\?\Volume{ae3644a0-9638-11df-aec6-00155d63712f}",
  // do not add the L"\\??\\"
  if(volCtx->Name.Length >= 4)
  {
	if(volCtx->Name.Buffer[0] == L'\\' && volCtx->Name.Buffer[1] == L'\\' &&
		volCtx->Name.Buffer[2] == L'?' && volCtx->Name.Buffer[3] == L'\\')
	{
		AddPrefix = FALSE;
	}
  }

  RtlZeroMemory(outFileName->Buffer, outFileName->MaximumLength);
  if(AddPrefix)
  {
	RtlAppendUnicodeToString(outFileName, L"\\??\\");
  }
  RtlAppendUnicodeStringToString(outFileName, &volCtx->Name);
  RtlAppendUnicodeStringToString(outFileName, &fileNameInfo->ParentDir);
  RtlAppendUnicodeStringToString(outFileName, &fileNameInfo->FinalComponent); 
}

//Do synchronous read i/o
NTSTATUS
NLSESendSynchronousReadIO(__in PCFLT_RELATED_OBJECTS FltObjects,
			  __in LARGE_INTEGER readOffset,
			  __in PVOID         buffer,
			  __in LARGE_INTEGER bufferLen)
{  
  PFLT_CALLBACK_DATA  readData = NULL; 
  NTSTATUS status;

  status =FltAllocateCallbackData(FltObjects->Instance, 
				  FltObjects->FileObject,
				  &readData);
  if(!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLSE!SendSynchronousReadIO: allocate data err=0x%x\n",
		status);
    return status;
  }
  
  readData->RequestorMode = KernelMode;
  readData->Iopb->MajorFunction = IRP_MJ_READ;
  readData->Iopb->MinorFunction = IRP_MN_NORMAL;
  readData->Iopb->IrpFlags = IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO;
  readData->Iopb->Parameters.Read.ByteOffset.QuadPart = readOffset.QuadPart;
  readData->Iopb->Parameters.Read.Length = (ULONG)(bufferLen.QuadPart);
  readData->Iopb->Parameters.Read.MdlAddress = NULL;
  readData->Iopb->Parameters.Read.ReadBuffer = buffer;

  FltPerformSynchronousIo(readData);

  status=readData->IoStatus.Status;
  FltFreeCallbackData(readData);
  return status;
}

//Get the current time
LARGE_INTEGER 
GetCurrentTime()
{
  LARGE_INTEGER Ret;
  KeQuerySystemTime(&Ret);
  return Ret;
}

//Check if the access from remote client
//If yes, return TRUE
BOOLEAN NLSEIsAccessFromRemote(__in_opt PIO_SECURITY_CONTEXT secCtx)
{
  BOOLEAN bRet=FALSE;
  PACCESS_STATE pAS = NULL;
  PACCESS_TOKEN pT = NULL; 
  PTOKEN_GROUPS tokenGroupsPtr=NULL;
  NTSTATUS status;
  ULONG index=0;

  //Sanity checking on input
  if(secCtx == NULL) {
    return bRet;
  }

  pAS=secCtx->AccessState;
  if(pAS != NULL && 
     pAS->SubjectSecurityContext.ClientToken != NULL) {
    pT=SeQuerySubjectContextToken(&pAS->SubjectSecurityContext);
    status = SeQueryInformationToken(pT,
				     TokenGroups,
				     &tokenGroupsPtr);
    // Check if we've got token groups information.
    if (NT_SUCCESS(status)) {
      // Go through all SIDs to see if there's network pseudo group ID
      for (index = 0; index < tokenGroupsPtr->GroupCount; ++index) {
	if (RtlEqualSid(SeExports->SeNetworkSid, 
			tokenGroupsPtr->Groups[index].Sid)) {
	  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
		      "NLSE!IsAccessFromRemote: Yes\n");
	  bRet=TRUE;
	  break;
	}
      }
    }
    if (tokenGroupsPtr) {
      ExFreePool(tokenGroupsPtr);
    }
  }

  return bRet;
}

//Check if the FileObject is a directory
BOOLEAN
NLSEIsDirFileObj(__in PCFLT_RELATED_OBJECTS fltObjects)
{
  NTSTATUS                  Status = STATUS_UNSUCCESSFUL;
  FILE_STANDARD_INFORMATION FileStdInfo;
  BOOLEAN                   bDir=FALSE;

  //initialization
  RtlZeroMemory(&FileStdInfo,sizeof(FileStdInfo));

  //get fileinfo
  Status = FltQueryInformationFile(fltObjects->Instance,
				   fltObjects->FileObject,
				   &FileStdInfo,
				   sizeof(FileStdInfo),
				   FileStandardInformation,
				   NULL);
  if(NT_SUCCESS(Status))	{
    // check if it's directory
    if(FileStdInfo.Directory)	{
      bDir=TRUE; //a directory
    }
  }
  return bDir;
}

//Set Basic Information on input file "fileName" 
BOOLEAN
NLSESetFileBasicInfo(__in PFLT_FILTER filterHandle,
		     __in PFLT_INSTANCE filterInstance,
		     __in NLFSE_PVOLUME_CONTEXT volCtx,
		     __in PUNICODE_STRING fileName,
		     __out PFILE_BASIC_INFORMATION originalFileBasicInfo,
		     __in PFILE_BASIC_INFORMATION newFileBasicInfo)
{
  OBJECT_ATTRIBUTES            objAttr;
  HANDLE                       handle;
  PFILE_OBJECT                 fileHandle;
  IO_STATUS_BLOCK              ioStatusBlock;
  NTSTATUS                     status;
  FILE_BASIC_INFORMATION       fileBasicInfo;

  //Check if ADS exists 
  InitializeObjectAttributes(&objAttr,
			     fileName,
			     OBJ_KERNEL_HANDLE,
			     NULL,
			     NULL);

  //Open the file 
  status = FltCreateFile(filterHandle,
			 filterInstance,
			 &handle,
			 FILE_WRITE_ATTRIBUTES|SYNCHRONIZE,
			 &objAttr,
			 &ioStatusBlock,
			 0,
			 FILE_ATTRIBUTE_NORMAL,
			 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
			 FILE_OPEN,
			 FILE_SYNCHRONOUS_IO_NONALERT|FILE_COMPLETE_IF_OPLOCKED|FILE_NO_INTERMEDIATE_BUFFERING,
			 NULL,
			 0,
			 IO_IGNORE_SHARE_ACCESS_CHECK);
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLSE!SetFileBasicInfo: create %wZ failed 0x%x\n", 
		fileName, status);
    return FALSE;
  }
  
  // Get file object from the handle
  status = ObReferenceObjectByHandle(handle,       //Handle
				     0,            //DesiredAccess
				     NULL,         //ObjectType
				     KernelMode,   //AccessMode
				     &fileHandle,  //File Handle
				     NULL);

  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLSE!SetFileBasicInfo: reference obj failed 0x%x %wZ\n", 
		status, fileName);
    FltClose(handle);
    return FALSE;
  }

  //get original file basic information
  RtlZeroMemory(originalFileBasicInfo,sizeof(*originalFileBasicInfo));
  status = FltQueryInformationFile(filterInstance,
				   fileHandle,
				   originalFileBasicInfo,
				   sizeof(*originalFileBasicInfo),
				   FileBasicInformation,
				   NULL);
  if(!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLSE!SetFileBasicInfo: query basicInfo err=0x%x %wZ\n", 
		status, fileName);
    ObDereferenceObject(fileHandle);	
    FltClose(handle);
    return FALSE;
  }

  //Set file basic information
  RtlZeroMemory(&fileBasicInfo,sizeof(fileBasicInfo));
  fileBasicInfo.FileAttributes=newFileBasicInfo->FileAttributes;
  status=FltSetInformationFile(filterInstance,
			       fileHandle,
			       &fileBasicInfo,
			       sizeof(fileBasicInfo),
			       FileBasicInformation);
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		"NLSE!SetFileBasicInfo: set basicInfo err=0x%x %wZ\n", 
		status, fileName);
    ObDereferenceObject(fileHandle);	
    FltClose(handle);
    return FALSE;
  }
			       
  ObDereferenceObject(fileHandle);			       
  FltClose(handle);
  return TRUE;
}

BOOLEAN
IsNTFSReservedFile(
                   __in_opt PUNICODE_STRING ParentDir,
                   __in_opt PUNICODE_STRING FileName
                   )
{
    USHORT i = 0;
    if(NULL==ParentDir
        || NULL==FileName
        || 0==ParentDir->Length
        || NULL==ParentDir->Buffer
        || 0==FileName->Length
        || NULL==FileName->Buffer
        )
        return FALSE;

    if(2==ParentDir->Length && L'\\'==ParentDir->Buffer[0])
    {
        i = 0;
        while (NULL != NtfsReservedNames[i].Buffer)
        {
            if(0 == RtlCompareUnicodeString(FileName, &NtfsReservedNames[i++], TRUE))
                return TRUE;
        }        
    }
    else if(0 == RtlCompareUnicodeString(ParentDir, &ExtendDir, TRUE))
    {
        i = 0;
        while (NULL != NtfsExtendNames[i].Buffer)
        {
            if(0 == RtlCompareUnicodeString(FileName, &NtfsExtendNames[i++], TRUE))
                return TRUE;
        }
    }
    else
    {
        ; // Do nothing
    }

    return FALSE;
}

BOOLEAN
IsAdsStreamInfo(
                __in PFILE_STREAM_INFORMATION fsi
                )
{
    if(fsi->StreamNameLength != NLFSE_ADS_FULL_NAME_LENGTH_IN_BYTES)
        return FALSE;
    if(NLFSE_ADS_FULL_NAME_LENGTH_IN_BYTES != RtlCompareMemory(fsi->StreamName, NLFSE_ADS_FULL_NAME, NLFSE_ADS_FULL_NAME_LENGTH_IN_BYTES))
        return FALSE;
    return TRUE;
}

BOOLEAN
RemoveNLAdsFromQueryResult(
                           __in_opt PFILE_STREAM_INFORMATION fsi
                           )
{
    PFILE_STREAM_INFORMATION fsiNext = NULL;
    BOOLEAN                  Removed = FALSE;

    if(NULL == fsi)
        return FALSE;

    while (0 != fsi->NextEntryOffset)
    {
        // Don't need to check the first stream -- it is always the main file: "::DATA"
        fsiNext = GetNextFileStreamInformation(fsi);
        if(NULL == fsiNext) break; // This is the last record, break;

        // Next one is not a ADS stream, move to next
        // ADS stream name is ":nlse_stream:$DATA"
        if(!IsAdsStreamInfo(fsiNext))
        {
            fsi = fsiNext;
            continue;
        }

        // Next record is ADS record, remove it
        // If there is another record after ADS record, link it to current record
        // Otherwise, just remove ADS record
        fsi->NextEntryOffset = (0==fsiNext->NextEntryOffset)?0:(fsi->NextEntryOffset+fsiNext->NextEntryOffset);
        // Clean ADS record information
        fsiNext->NextEntryOffset      = 0;
        fsiNext->StreamAllocationSize.QuadPart = 0;
        fsiNext->StreamSize.QuadPart  = 0;
        fsiNext->StreamNameLength     = 0;
        fsiNext->StreamName[0]        = 0;
        // At most there is only one ADS for one file, no need to continue searching
        Removed = TRUE;
        break;
    }

    return Removed;
}

void CheckVolumeName(__in PFLT_VOLUME Volume,__inout_opt PUNICODE_STRING Name)
{
	UNICODE_STRING DosName;
	PDEVICE_OBJECT devObj = NULL;
	NTSTATUS status;
	PWCHAR OldBuffer = NULL;

	if(!Name) return;

	if(Name->Length >= 4 && 
		Name->Buffer[0] == L'\\' &&
		Name->Buffer[1] == L'\\' &&
		Name->Buffer[2] == L'?' &&
		Name->Buffer[3] == L'\\')
	{
		status = FltGetDiskDeviceObject( Volume, &devObj );
		if (NT_SUCCESS(status)) 
		{
			status = IoVolumeDeviceToDosName( devObj, &DosName );
			ObDereferenceObject( devObj );
			if(status == STATUS_SUCCESS)
			{
				OldBuffer = Name->Buffer;
				*Name = DosName;
				ExFreePool(OldBuffer);
			}
		}
	}
}

VOID
PfStart(
        __out PNLPERFORMANCE_COUNTER ppfc
        )
{
    ppfc->start = KeQueryPerformanceCounter(&ppfc->freq);
    ppfc->freq.QuadPart = ppfc->freq.QuadPart/1000000; // frequency per microseconds
}

/*
It gets time diff in microseconds
*/
VOID
PfEnd(
      __out PNLPERFORMANCE_COUNTER ppfc
      )
{
    ppfc->end = KeQueryPerformanceCounter(NULL);
    // Diff time is in microseconds.
    if(0 != ppfc->freq.QuadPart)
        ppfc->diff.QuadPart = (ppfc->end.QuadPart - ppfc->start.QuadPart)/ppfc->freq.QuadPart;
    else
        ppfc->diff.QuadPart = ppfc->end.QuadPart - ppfc->start.QuadPart;
}