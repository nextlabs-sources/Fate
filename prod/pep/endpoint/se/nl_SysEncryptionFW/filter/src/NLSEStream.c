/*++

Module Name:

  NLSEStream.c

Abstract:
  Encryption Stream (e.g. ADS) for system encryption in kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSEDrmpathList.h"
#include "FileOpHelp.h"
//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;

//Check if stream name is NLSE 
static BOOLEAN IsNextLabsEncryptionFile(__out_opt unsigned char *n)
{
  BOOLEAN bRet=FALSE;
  STRING  tmpString;
  
  if(n == NULL)
    return bRet;

  RtlInitString(&tmpString,
		NLSE_STREAM_NAME);
  if(RtlCompareMemory(n, tmpString.Buffer, tmpString.Length)==tmpString.Length)
    bRet=TRUE;

  return bRet;		      
}

//Check if cookie is the NL cookie
static BOOLEAN IsNextLabsFile(__out_opt unsigned char *cookie)
{
  BOOLEAN bRet=FALSE;
  
  if(cookie == NULL)
    return bRet;

  if(cookie[0] == 'N' && cookie[1] == 'e' && 
     cookie[2] == 'x' && cookie[3] == 't' && 
     cookie[4] == 'L' && cookie[5] == 'a' && 
     cookie[6] == 'b' && cookie[7] == 's')
    bRet=TRUE;

  return bRet;		      
}

//Initialize NextLabs File Type structure
static VOID InitializeNLFileType(__out_opt NextLabsFile_TYPE *t)
{
  RtlZeroMemory(t, sizeof(NextLabsFile_TYPE));
  t->version_major=0x1;
  t->version_minor=0x0;
  t->stream_count=1;
  t->header_size=sizeof(*t);
  t->cookie[0]='N';
  t->cookie[1]='e';
  t->cookie[2]='x';
  t->cookie[3]='t';
  t->cookie[4]='L';
  t->cookie[5]='a';
  t->cookie[6]='b';
  t->cookie[7]='s';
}

NTSTATUS
EncryptExtensionToAdsFile(__in PFLT_INSTANCE             Instance,
                          __in PFILE_OBJECT              FileObject,
                          __in PNLFSE_ENCRYPT_EXTENSION  dupExt,
                          __in ULONG                     pid,
                          __in ULONG                     sectorSize)
{
    NTSTATUS                     status = STATUS_SUCCESS;
    char                         *buffer;
    size_t                       bufferSize;
    ULONG                        bytesWritten;
    FILE_END_OF_FILE_INFORMATION stEndOfFile;
    LARGE_INTEGER                offset;
    FLT_IO_OPERATION_FLAGS       writeFlags;
    NextLabsFile_TYPE            nlFileType;

    //Initialize NextLabsFile_TYPE
    InitializeNLFileType(&nlFileType);

    //Update current PC encryption keys stored in nlfseGlobal
    ExAcquireFastMutex(&nlfseGlobal.currentPCKeyLock);
    NLSEUpdateCurrentPCKey(pid, TRUE);
    ExReleaseFastMutex(&nlfseGlobal.currentPCKeyLock);

    //encrypt data encryption key using nlfseGlobal.currentPCKey
    ExAcquireFastMutex(&nlfseGlobal.currentPCKeyLock);
    NLSEEncryptIndicator(dupExt, nlfseGlobal.currentPCKey, &(nlfseGlobal.currentPCKeyID), NLSE_KEY_RING_LOCAL);
    ExReleaseFastMutex(&nlfseGlobal.currentPCKeyLock);

    //allocate write buffer 
    bufferSize=sizeof(NextLabsFile_TYPE);
    bufferSize+=sizeof(NLFSE_ENCRYPT_EXTENSION);
    bufferSize=ROUND_TO_SIZE(bufferSize, sectorSize);
    buffer = FltAllocatePoolAlignedWithTag( Instance, NonPagedPool, bufferSize, NLFSE_BUFFER_TAG);
    if (NULL == buffer)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!WriteADS: can't allocate %d bytes of memory\n", bufferSize);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(buffer, bufferSize);
    RtlCopyMemory(buffer, &nlFileType, sizeof(NextLabsFile_TYPE));
    RtlCopyMemory(buffer+sizeof(NextLabsFile_TYPE), dupExt, sizeof(NLFSE_ENCRYPT_EXTENSION));

    //write data 
    writeFlags=FLTFL_IO_OPERATION_NON_CACHED;
    writeFlags|=FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET;
    offset.QuadPart=0;
    status = FltWriteFile(Instance, FileObject, &offset, bufferSize, buffer, writeFlags, &bytesWritten, NULL, NULL);   
    if (!NT_SUCCESS(status)
        ||  bytesWritten < (sizeof(NextLabsFile_TYPE)+sizeof(NLFSE_ENCRYPT_EXTENSION)))
    {
        //write failed; exit
        if(NT_SUCCESS(status))
            status = STATUS_UNSUCCESSFUL;
        goto _exit;
    }

    //set file size to the size to encryption extension size
    stEndOfFile.EndOfFile.QuadPart = sizeof(NLFSE_ENCRYPT_EXTENSION);
    stEndOfFile.EndOfFile.QuadPart += sizeof(NextLabsFile_TYPE);
    status = FltSetInformationFile(Instance,
        FileObject,
        &stEndOfFile, 
        sizeof(FILE_END_OF_FILE_INFORMATION), 
        FileEndOfFileInformation);

_exit:
    if(NULL!=buffer) FltFreePoolAlignedWithTag(Instance, buffer, NLFSE_BUFFER_TAG ); 
    return status;
}

//Check if the file has encryption alternative data stream.
//If yes, create an encrypt extension and return true. If not, return false.
static BOOLEAN 
NLFSECheckEncrypteADSExist(__in PFLT_FILTER filter,
			   __in PFLT_INSTANCE instance,
			   __in PUNICODE_STRING fileName,
			   __in NLFSE_PVOLUME_CONTEXT volCtx,
			   __deref_out PNLFSE_ENCRYPT_EXTENSION *pEncryptExt)
{
  PNLFSE_ENCRYPT_EXTENSION     pExt=NULL;
  BOOLEAN                      bExtExist=FALSE;

  if(pEncryptExt == NULL) 
    return bExtExist;

  //Open and Read ADS content
  pExt=NLSEOpenAndReadADS(filter,
			  instance,
			  fileName,
			  volCtx);
  if(pExt == NULL) {
    return bExtExist;
  }

  bExtExist=TRUE;

  //clean up
  *pEncryptExt=pExt;
  return bExtExist;
}/*NLFSECheckEncryptADSExist*/

//Checking if a directory or any directory above has encryption attribute.
//The attribute is in the form of NTFS alternative data stream.
//If yes, return true; 
BOOLEAN NLSECheckDirEncryptionAttribute(__in PCFLT_RELATED_OBJECTS FltObjects,
					__in PUNICODE_STRING dirName,
					__in BOOLEAN         bHasBackSlash)
{
  NTSTATUS                 status;
  NLFSE_PVOLUME_CONTEXT    volCtx=NULL;
  BOOLEAN                  bADSExist=FALSE;
  PNLFSE_ENCRYPT_EXTENSION pEncryptExt=NULL;
  UNICODE_STRING           dNameOnly;     
  UNICODE_STRING           dNonUNCName;   
  BOOLEAN                  bDummy;
  BOOLEAN                  bReturn=FALSE;
  SHORT                    index=0;
  BOOLEAN                  bRoot=FALSE;

  //Get our volume context 
  status = FltGetVolumeContext( FltObjects->Filter,
				FltObjects->Volume,
				&volCtx);
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"NLSE!CheckDirEncryptionAtt: get volume context err=%x\n",
		status );
    return FALSE;
  }

  //compose directory name without '\' at the end
  RtlInitUnicodeString(&dNameOnly, NULL);
  dNameOnly.MaximumLength=dirName->Length;
  if(bHasBackSlash) {
    dNameOnly.MaximumLength-=sizeof(WCHAR);
  }
  if(dNameOnly.MaximumLength == 0) {
    FltReleaseContext( volCtx );
    return FALSE;
  }
  if(dNameOnly.MaximumLength > NLFSE_NAME_LEN*sizeof(WCHAR)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		"NLSE!CheckDirEncryptionAtt: The dir name %wZ is too long\n",
		dirName);
    FltReleaseContext( volCtx );
    return FALSE;
  }
  dNameOnly.Length=0;
  dNameOnly.Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,4096,NLSE_CHECKDIRENCATTR_TAG);

  if(dNameOnly.Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
	      ("NLSE!CheckDirEncryptionAtt: failed to allocate buffer\n"));
    FltReleaseContext( volCtx );
    return FALSE;
  }
  RtlZeroMemory(dNameOnly.Buffer, dNameOnly.MaximumLength);
  RtlCopyUnicodeString(&dNameOnly, dirName);

  //compose directory non-UNC name
  CheckVolumeName(FltObjects->Volume,&volCtx->Name);
  RtlInitUnicodeString(&dNonUNCName, NULL);
  NLFSEGetFileNonUNCName(&dNonUNCName, 
			 volCtx,
			 NULL, 
			 &dNameOnly);

  //Check if the directory or any directory above has NLSE ADS
  index=((SHORT)(dNonUNCName.Length/2))-1;
  bRoot=FALSE;
  while (1) {
    BOOLEAN out_result;

    /*NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,
		"NLSE!CheckDirEncryptionAtt: check index=%d %wZ, %wZ\n",
		index, &dNameOnly, &dNonUNCName);*/
    if(NLFSECheckEncrypteADSExist(FltObjects->Filter,
				   FltObjects->Instance,
				   &dNonUNCName,
				   volCtx, 
				   &pEncryptExt)) {
      //One directory along the path has NLSE ADS; stop scanning
      /*NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
		  "NLSE!CheckDirEncryptionAtt: DRM dir=%wZ  drm-parent=%wZ\n", 
		  &dNameOnly, &dNonUNCName);*/
      bReturn=TRUE;
      break;
    } else if (NT_SUCCESS(NLSEDrmPathListCheckPath(&dNonUNCName, &out_result))
               && out_result) {
      //One directory along the path matches the DRM path list; stop scanning
      bReturn=TRUE;
      break;
    } else {
      for(; index >= 0; --index) {
	if(dNonUNCName.Buffer[index] == L'\\') {
	  //find "\"
	  //get the parent directory path
	  if((index-1) >= 0 && dNonUNCName.Buffer[index-1] == L':') {
	    //find ":\";reach the root
	    bRoot=TRUE;
	  }
	  dNonUNCName.Buffer[index]=L'\0';
	  dNonUNCName.Length=(USHORT)(index*2);
	  break;
	}
      }
      if(bRoot || !(index > 0)) {
	//Trace to the root; stop scanning
	break;
      } 
    }
  }

  //clean up
  ExFreePool(dNameOnly.Buffer);

  FltReleaseContext( volCtx );
  NLFSEFreeUnicodeNameString(&dNonUNCName);
  if(bReturn && pEncryptExt) NLFSEFreeEncryptExtension(pEncryptExt);

  //return
  return bReturn;  
}/*NLSECheckDirEncryptionAttribute*/

__checkReturn
NTSTATUS NLSEIsDirectoryEncryptedByAds( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PCUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result )
{
  UNICODE_STRING               streamName;
  OBJECT_ATTRIBUTES            objAttr;
  HANDLE                       handle;
  NTSTATUS                     status;
  IO_STATUS_BLOCK              ioStatusBlock;
  LONGLONG                     bufferSize=sizeof(NLFSE_ENCRYPT_EXTENSION);
  FLT_IO_OPERATION_FLAGS       readFlags;

  ASSERT( in_path != NULL );
  ASSERT( out_result != NULL );

  //compose ADS name
  RtlInitUnicodeString(&streamName,NULL);

  /* Allocate space for the given input path with space for the stream append  */
  streamName.MaximumLength = in_path->MaximumLength + sizeof(NLFSE_ADS_SUFFIX);
  streamName.Length = 0;
  streamName.Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,
						   streamName.MaximumLength + sizeof(NLFSE_ADS_SUFFIX),
						   NLSE_OPENREADADS_TAG);
  if( streamName.Buffer == NULL )
  {
    return STATUS_INSUFFICIENT_RESOURCES;
  }

  RtlZeroMemory(streamName.Buffer,streamName.MaximumLength);
  RtlAppendUnicodeStringToString(&streamName,in_path);
  RtlAppendUnicodeToString(&streamName,NLFSE_ADS_SUFFIX);

  //Check if ADS exists 
  InitializeObjectAttributes(&objAttr,&streamName,OBJ_KERNEL_HANDLE,NULL,NULL);

  //Open the file 
  status = FltCreateFile(FltObjects->Filter,FltObjects->Instance,&handle,GENERIC_READ,
			 &objAttr,&ioStatusBlock,0,
			 FILE_ATTRIBUTE_NORMAL,
			 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			 FILE_OPEN,
			 FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
			 NULL,0,
			 IO_IGNORE_SHARE_ACCESS_CHECK);
  ExFreePool(streamName.Buffer);
  *out_result = FALSE;
  if( NT_SUCCESS(status) )
  {
    *out_result = TRUE;
    FltClose(handle);
  }
  return STATUS_SUCCESS;
}/* NLSEIsDirectoryEncryptedByAds */

NTSTATUS NLSEIsDirectoryEncryptedByDrmPathList( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PCUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result )
{
  return NLSEDrmPathListCheckPath(in_path, out_result);
}/* NLSEIsDirectoryEncryptedByDrmPathList */

__checkReturn
NTSTATUS NLSEIsDirectoryEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
				   __in PUNICODE_STRING in_path ,
				   __in BOOLEAN* out_result )
{
  NTSTATUS status;

  status = NLSEIsDirectoryEncryptedByAds(FltObjects, in_path, out_result);
  if (!NT_SUCCESS(status) || *out_result)
  {
    return status;
  }

  return NLSEIsDirectoryEncryptedByDrmPathList(FltObjects, in_path, out_result);
}/* NLSEIsDirectoryEncrypted */

__checkReturn
NTSTATUS NLSEIsPathEncrypted( __in PCFLT_RELATED_OBJECTS FltObjects,
			      __in PUNICODE_STRING in_path ,
			      __in BOOLEAN* out_result )
{
  UNICODE_STRING path;
  PWCHAR p = NULL;
  LONG i = 0;
  NTSTATUS status = STATUS_SUCCESS;

  ASSERT( in_path != NULL );
  ASSERT( out_result != NULL );

  RtlCopyMemory(&path,in_path,sizeof(path));
  path.Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,in_path->MaximumLength,NLSE_CHECKDIRENCATTR_TAG);
  if( path.Buffer == NULL )
  {
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  RtlCopyMemory(path.Buffer,in_path->Buffer,in_path->Length);

  *out_result = FALSE; /* Default is not an encrypted path */
  if (path.Length == 0)
  {
    goto _exit;
  }
  for( ; ; )
  {
    BOOLEAN curr_enc = FALSE;
    /* Traverse the string for a slash and truncate including the slash.  The UNICODE_STRING
     * must also be updated after truncation.
     */

    p = path.Buffer + (path.Length / sizeof(WCHAR)) - 1;
    while( p != path.Buffer )
    {
      if( *p == L'\\' )
      {
	*p = (WCHAR)NULL;
	break;
      }
      p--;
    }/* while */
    path.Length = (p - path.Buffer) * sizeof(WCHAR);

    /* Are we are root? */
    if( p == path.Buffer || path.Length <= sizeof(L"\\??\\c:"))
    {
      break;
    }

    /* If the current directory is encrypted, then stop the search.  Otherwise walk to
     * the root to determine if the current path is encrypted.
     */
    status = NLSEIsDirectoryEncrypted(FltObjects,&path,&curr_enc);
    if( !NT_SUCCESS(status) )
    {
      break;
    }
    if( curr_enc == TRUE )
    {
      *out_result = TRUE;
      break;
    }

  }/* for */

_exit:
  ExFreePool(path.Buffer);

  return status;
}/* NLSEIsPathEncrypted */

//Create (open if exists already) encryption ADS (alternative data stream)
//in order to store encryption information. 
void
NLSEGenEncryptionADS(__in PFLT_FILTER filterHandle,
		     __in PFLT_INSTANCE filterInstance,
		     __in PUNICODE_STRING fileName,
		     __in NLFSE_PVOLUME_CONTEXT volCtx,
		     __in_opt PNLFSE_ENCRYPT_EXTENSION pEncryptExt,
		     __in ULONG                    pid)
{
  UNICODE_STRING               streamName;
  OBJECT_ATTRIBUTES            objAttr;
  HANDLE                       handle;
  NTSTATUS                     status;
  IO_STATUS_BLOCK              ioStatusBlock;
  char                         *buffer;
  size_t                       bufferSize=sizeof(NLFSE_ENCRYPT_EXTENSION);
  ULONG                        bytesWritten;
  PFILE_OBJECT                 fileHandle;
  FILE_END_OF_FILE_INFORMATION stEndOfFile;
  LARGE_INTEGER                offset;
  FLT_IO_OPERATION_FLAGS       writeFlags;
  ULONG                        createOpts;
	FILE_BASIC_INFORMATION      fbi             = {0};
	BOOLEAN						AttrModified = FALSE;

  if(KeGetCurrentIrql() != PASSIVE_LEVEL)
    return;

  if(pEncryptExt == NULL)
    return;

  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  //compose ADS name
  RtlInitUnicodeString(&streamName, NULL);
  streamName.MaximumLength=NLFSE_NAME_LEN*sizeof(WCHAR);
  streamName.Length=0;
  streamName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_GETENCADS_TAG);
  if(streamName.Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!GenEncryptionADS: allocate %d bytes memory failed.\n",
		streamName.MaximumLength );
    return;
  }
  RtlZeroMemory(streamName.Buffer, streamName.MaximumLength);
  status=RtlAppendUnicodeStringToString(&streamName, fileName);
  if(status != STATUS_SUCCESS) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!GenEncryptionADSHandle: failed to append 0x%x.\n",
		status );
    ExFreePool(streamName.Buffer);
    return;
  }
  RtlAppendUnicodeToString(&streamName, NLFSE_ADS_SUFFIX);
  
	status = FOH_GET_FILE_BASICINFO_BY_NAME(filterHandle,filterInstance,fileName,&fbi);
    if(NT_SUCCESS(status))
    {
		if(fbi.FileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			status = FOH_REMOVE_READONLY_ATTRIBUTES(filterHandle,filterInstance,fileName);
			if(NT_SUCCESS(status))
			{
				AttrModified = TRUE;
			}			
		}
    }
	
  //Create ADS
  InitializeObjectAttributes(&objAttr,
			     &streamName,
			     OBJ_KERNEL_HANDLE,
			     NULL,
			     NULL);

  //Open the ADS
  createOpts=FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT;
  createOpts|=FILE_NO_INTERMEDIATE_BUFFERING;
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
			 createOpts,
			 NULL,
			 0,
			 IO_IGNORE_SHARE_ACCESS_CHECK);
  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!GenEncryptionADS: create %wZ failed 0x%x\n", 
		&streamName, status);
    ExFreePool(streamName.Buffer);

	if(AttrModified)
	{
		FOH_SET_FILE_BASICINFO_BY_NAME(filterHandle, filterInstance, fileName, &fbi);
	}
    return;
  }
  
  // Get file object from the handle
  status = ObReferenceObjectByHandle(handle,       //Handle
				     0,            //DesiredAccess
				     NULL,         //ObjectType
				     KernelMode,   //AccessMode
				     &fileHandle,  //File Handle
				     NULL);

  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!GenEncryptionADS: reference obj failed 0x%x %wZ\n", 
		status, &streamName);
    ExFreePool(streamName.Buffer);
    FltClose(handle);
	if(AttrModified)
	{
		FOH_SET_FILE_BASICINFO_BY_NAME(filterHandle, filterInstance, fileName, &fbi);
	}
    return;
  }

  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
	      "NLSE!GenEncryptionADS: %wZ size=%ld\n", 
	      &streamName, (int)(pEncryptExt->fileRealLength));

  EncryptExtensionToAdsFile(filterInstance, fileHandle, pEncryptExt, pid, volCtx->SectorSize);

  //clean up
  ObDereferenceObject(fileHandle);
  FltClose(handle);
  ExFreePool(streamName.Buffer);

  if(AttrModified)
  {
	  FOH_SET_FILE_BASICINFO_BY_NAME(filterHandle, filterInstance, fileName, &fbi);
  }

  return;
}/*NLSEGenEncryptionADS*/

//Delete (if exists) encryption ADS (alternative data stream)
void
NLSEDeleteEncryptionADS(__in PFLT_FILTER filterHandle,
			__in PFLT_INSTANCE filterInstance,
			__in PUNICODE_STRING fileName,
			__in NLFSE_PVOLUME_CONTEXT volCtx)
{
  UNICODE_STRING               streamName;
  OBJECT_ATTRIBUTES            objAttr;
  HANDLE                       handle;
  NTSTATUS                     status;
  IO_STATUS_BLOCK              ioStatusBlock;
  PFILE_OBJECT                 fileHandle;
  ULONG                        createOpts;

  if(KeGetCurrentIrql() != PASSIVE_LEVEL)
    return;

  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  //compose ADS name
  RtlInitUnicodeString(&streamName, NULL);
  streamName.MaximumLength=NLFSE_NAME_LEN*sizeof(WCHAR);
  streamName.Length=0;
  streamName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_DELETEENCADS_TAG);
  if(streamName.Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!DeleteEncryptionADS: allocate %d bytes failed.\n",
		streamName.MaximumLength );
    return;
  }
  RtlZeroMemory(streamName.Buffer, streamName.MaximumLength);
  status=RtlAppendUnicodeStringToString(&streamName, fileName);
  if(status != STATUS_SUCCESS) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]!DeleteEncryptionADSHandle: failed to append 0x%x.\n",
		status );
    ExFreePool(streamName.Buffer);
    return;
  }
  RtlAppendUnicodeToString(&streamName, NLFSE_ADS_SUFFIX);
  
  //Create ADS
  InitializeObjectAttributes(&objAttr,
			     &streamName,
			     OBJ_KERNEL_HANDLE,
			     NULL,
			     NULL);

  //Open the ADS
  createOpts=FILE_COMPLETE_IF_OPLOCKED|FILE_SYNCHRONOUS_IO_NONALERT;
  createOpts|=FILE_NO_INTERMEDIATE_BUFFERING;
  createOpts|=FILE_DELETE_ON_CLOSE;
  status = FltCreateFile(filterHandle,
			 filterInstance,
			 &handle,
			 DELETE|SYNCHRONIZE,
			 &objAttr,
			 &ioStatusBlock,
			 0,
			 FILE_ATTRIBUTE_NORMAL,
			 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
			 FILE_OPEN,
			 createOpts,
			 NULL,
			 0,
			 IO_IGNORE_SHARE_ACCESS_CHECK);
  if (!NT_SUCCESS(status)) {
    if(status != STATUS_FLT_DELETING_OBJECT &&
       status != STATUS_OBJECT_NAME_NOT_FOUND) {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		  "NLSE!DeleteEncryptionADS: create %wZ failed 0x%x\n", 
		  &streamName, status);
    }
    ExFreePool(streamName.Buffer);
    return;
  }
  
  // Get file object from the handle
  status = ObReferenceObjectByHandle(handle,       //Handle
				     0,            //DesiredAccess
				     NULL,         //ObjectType
				     KernelMode,   //AccessMode
				     &fileHandle,  //File Handle
				     NULL);

  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!DeleteEncryptionADS: reference obj failed 0x%x %wZ\n", 
		status, &streamName);
    ExFreePool(streamName.Buffer);
    FltClose(handle);
    return;
  }

  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, 
	      "NLSE!DeleteEncryptionADS: %wZ\n", 
	      &streamName);

  //clean up
  ObDereferenceObject(fileHandle);
  FltClose(handle);
  ExFreePool(streamName.Buffer);
  return;
}/*NLSEDeleteEncryptionADS*/

// This function can only be called in PostCreate for a new created file
NTSTATUS
NLSECreateAdsAtPostCreate(
                          __in PFLT_FILTER Filter,
                          __in_opt PFLT_INSTANCE Instance,
                          __in_opt PUNICODE_STRING FileName,
                          __in NLFSE_PVOLUME_CONTEXT  volCtx,
                          __in_opt PNLFSE_ENCRYPT_EXTENSION Extension
                          )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    UNICODE_STRING      StreamName;
    HANDLE              StreamHandle = NULL;
    PFILE_OBJECT        StreamObject = NULL;
    OBJECT_ATTRIBUTES   ObjAttr;
    IO_STATUS_BLOCK     IoStatusBlock;
    NLFSE_ENCRYPT_EXTENSION          dupExt;
    BOOLEAN                     AttrModified    = FALSE;
    FILE_BASIC_INFORMATION      fbi             = {0};

    if(NULL==Instance
        || NULL==Extension
        || NULL==FileName
        || 0==FileName->Length)
        return STATUS_INVALID_PARAMETER;

    // If not PASSIVE LEVEL, return (MSDN: POST_CREATE must run at PASSIVE_LEVEL)
    if(PASSIVE_LEVEL != KeGetCurrentIrql())
        return STATUS_INVALID_LEVEL;

    ASSERT( PASSIVE_LEVEL == PASSIVE_LEVEL );

    // Compose ADS name
    RtlInitUnicodeString(&StreamName, NULL);
    StreamName.MaximumLength=NLFSE_NAME_LEN*sizeof(WCHAR);
    StreamName.Length=0;
    StreamName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_CREATEADSPOST_TAG);
    if(StreamName.Buffer == NULL)
        return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(StreamName.Buffer, StreamName.MaximumLength);
    RtlAppendUnicodeStringToString(&StreamName, FileName);
    RtlAppendUnicodeToString(&StreamName, NLFSE_ADS_SUFFIX);

    Status = FOH_GET_FILE_BASICINFO_BY_NAME(Filter, Instance, FileName, &fbi);
    if(NT_SUCCESS(Status))
    {
		if(fbi.FileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			Status = FOH_REMOVE_READONLY_ATTRIBUTES(Filter,Instance,FileName);
			if(NT_SUCCESS(Status))
			{
				AttrModified = TRUE;
			}			
		}
    }	

    //Check if ADS exists 
    InitializeObjectAttributes(&ObjAttr, &StreamName, OBJ_KERNEL_HANDLE, NULL, NULL);

    // Try to create a new Stream (And make sure no existing ADS)
    Status = FltCreateFile(Filter,
        Instance,
        &StreamHandle,
        GENERIC_WRITE,
        &ObjAttr,
        &IoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_CREATE, // Only create if there is no ADS
        FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if (!NT_SUCCESS(Status))
        goto _exit;

    Status = ObReferenceObjectByHandle(StreamHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &StreamObject,//File Handle
        NULL);
    if (!NT_SUCCESS(Status))
        goto _exit;

    // Write ADS
    RtlZeroMemory(&dupExt, sizeof(dupExt));
    RtlCopyMemory(&dupExt, Extension, sizeof(NLFSE_ENCRYPT_EXTENSION));
    EncryptExtensionToAdsFile(Instance,
        StreamObject,
        &dupExt,
        (ULONG)PsGetCurrentProcessId(),
        volCtx->SectorSize);

_exit:
	if(AttrModified)
	{
		FOH_SET_FILE_BASICINFO_BY_NAME(Filter, Instance, FileName, &fbi);
	}
    if(StreamObject) ObDereferenceObject(StreamObject); StreamObject=NULL;
    if(StreamHandle) FltClose(StreamHandle); StreamHandle=NULL;
    if(NULL!=StreamName.Buffer) ExFreePool(StreamName.Buffer);
    return Status;
}

//Open encryption alternative data stream if it exists.
//If ADS exists, this function return the pointer to a
//encryption extension buffer.
PNLFSE_ENCRYPT_EXTENSION  
NLSEOpenAndReadADS(__in PFLT_FILTER filter,
		   __in PFLT_INSTANCE instance,
		   __in PUNICODE_STRING fileName,
		   __in NLFSE_PVOLUME_CONTEXT volCtx)
{
  NextLabsFile_TYPE            nlfType; 
  UNICODE_STRING               streamName;
  OBJECT_ATTRIBUTES            objAttr;
  HANDLE                       handle;
  NTSTATUS                     status;
  IO_STATUS_BLOCK              ioStatusBlock;
  char                         *buffer;
  LONGLONG                     bufferSize=sizeof(NLFSE_ENCRYPT_EXTENSION);
  ULONG                        bytesRead;
  LARGE_INTEGER                readOffset;
  PFILE_OBJECT                 fileHandle;
  PNLFSE_ENCRYPT_EXTENSION     pExt=NULL;
  FILE_STANDARD_INFORMATION    stFileInfo;
  FLT_IO_OPERATION_FLAGS       readFlags;

  //compose ADS name
  RtlInitUnicodeString(&streamName, NULL);
  streamName.MaximumLength=NLFSE_NAME_LEN*sizeof(WCHAR);
  streamName.Length=0;
  streamName.Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool,NLFSE_NAME_LEN*sizeof(WCHAR),NLSE_OPENREADADS_TAG);
  if(streamName.Buffer == NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		"NLSE!NLSEOpenAndReadADS: can't allocate %d bytes memory\n",
		streamName.MaximumLength );
    return NULL;
  }
  RtlZeroMemory(streamName.Buffer, streamName.MaximumLength);
  RtlAppendUnicodeStringToString(&streamName, fileName);
  RtlAppendUnicodeToString(&streamName, NLFSE_ADS_SUFFIX);
  
  //Check if ADS exists 
  InitializeObjectAttributes(&objAttr,
			     &streamName,
			     OBJ_KERNEL_HANDLE,
			     NULL,
			     NULL);

  //Open the file 
  status = FltCreateFile(filter,
			 instance,
			 &handle,
			 GENERIC_READ,
			 &objAttr,
			 &ioStatusBlock,
			 0,
			 FILE_ATTRIBUTE_NORMAL,
			 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
			 FILE_OPEN,
			 FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
			 NULL,
			 0,
			 IO_IGNORE_SHARE_ACCESS_CHECK);
  if (!NT_SUCCESS(status)) {
    ExFreePool(streamName.Buffer);
    return NULL;
  }
  
  // Get file object from the handle
  status = ObReferenceObjectByHandle(handle,       //Handle
				     0,            //DesiredAccess
				     NULL,         //ObjectType
				     KernelMode,   //AccessMode
				     &fileHandle,  //File Handle
				     NULL);

  if (!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!OpenAndReadADS: reference obj failed 0x%x %wZ\n", 
		status, &streamName);
    ExFreePool(streamName.Buffer);
    FltClose(handle);
    return NULL;
  }

  //get ADS length
  status = FltQueryInformationFile(instance,
				   fileHandle,
				   &stFileInfo, 
				   sizeof(FILE_STANDARD_INFORMATION), 
				   FileStandardInformation, 
				   NULL);
  if(!NT_SUCCESS(status)) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!OpenAndReadADS: query file info: 0x%x %wZ\n", 
		status, &streamName);
    ObDereferenceObject(fileHandle);
    FltClose(handle);
    
    ExFreePool(streamName.Buffer);
    return NULL;
  }  

  //sanity checking on stream size
  if(stFileInfo.EndOfFile.QuadPart < 
     (sizeof(NextLabsFile_TYPE)+sizeof(NLFSE_ENCRYPT_EXTENSION))) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!OpenAndReadADS: small ADS size %ld %wZ\n", 
		(int)(stFileInfo.EndOfFile.QuadPart), &streamName);
    ObDereferenceObject(fileHandle);
    FltClose(handle);

    ExFreePool(streamName.Buffer);
    return NULL;
  }

  //allocate read buffer
  bufferSize=stFileInfo.EndOfFile.QuadPart; 
  bufferSize=ROUND_TO_SIZE(bufferSize, volCtx->SectorSize);
  buffer = FltAllocatePoolAlignedWithTag( instance,
					  NonPagedPool,
					  (size_t)bufferSize, 
					  NLFSE_BUFFER_TAG);
  if (NULL == buffer) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!OpenAndReadADS: can't allocate %ld memory\n", 
	       (int)bufferSize);
    ObDereferenceObject(fileHandle);
    FltClose(handle);
    ExFreePool(streamName.Buffer);
    return NULL;
  }
  RtlZeroMemory(buffer, (size_t)bufferSize);

  //read data 
  readOffset.QuadPart=0;
  readFlags=FLTFL_IO_OPERATION_NON_CACHED;
  //readFlags|=FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET;
  status = FltReadFile( instance,
			fileHandle,
			&readOffset,
			(size_t)bufferSize,
			buffer,
			readFlags, 
			&bytesRead, 
			NULL, 
			NULL );
  if (!NT_SUCCESS(status) || 
      bytesRead < (sizeof(NextLabsFile_TYPE)+sizeof(NLFSE_ENCRYPT_EXTENSION))){
    //read failed; exit
    FltFreePoolAlignedWithTag( instance,
			       buffer,
			       NLFSE_BUFFER_TAG );    
    ObDereferenceObject(fileHandle);
    FltClose(handle);
    ExFreePool(streamName.Buffer);
    return NULL;
  }
  
  //allocate memory for encryption extension
  pExt =NLFSEAllocateEncryptExtension(0);
  if(pExt == NULL) {
    FltFreePoolAlignedWithTag( instance,
			       buffer,
			       NLFSE_BUFFER_TAG );    
    ObDereferenceObject(fileHandle);
    FltClose(handle);
    ExFreePool(streamName.Buffer);
    return NULL;    
  }

  //assign file type and encryption extension
  RtlCopyMemory(&nlfType,
		buffer,
		sizeof(NextLabsFile_TYPE));
  RtlCopyMemory(pExt, 
		buffer+sizeof(NextLabsFile_TYPE),
		sizeof(NLFSE_ENCRYPT_EXTENSION));

  //Check if our encryption ADS
  if(!(IsNextLabsFile(nlfType.cookie) &&
       IsNextLabsEncryptionFile(pExt->sh.stream_name))) {
    //Not our encrypt extension_ID
    NLFSEFreeEncryptExtension(pExt);
    FltFreePoolAlignedWithTag( instance,
			       buffer,
			       NLFSE_BUFFER_TAG );    
    ObDereferenceObject(fileHandle);
    FltClose(handle);
    ExFreePool(streamName.Buffer);
    return NULL;
  } 

  //clean up
  FltFreePoolAlignedWithTag( instance,
			     buffer,
			     NLFSE_BUFFER_TAG );    
  ObDereferenceObject(fileHandle);
  FltClose(handle);
  ExFreePool(streamName.Buffer);
  return pExt;
}/*NLSEOpenAndReadADS*/

