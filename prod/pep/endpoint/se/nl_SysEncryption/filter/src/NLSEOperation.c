/*++

Module Name:

NLSEOperation.c

Abstract:
IPR operation for system encryption in kernel mode

Environment:

Kernel mode

--*/
#include <fltKernel.h>
#include "header.h"
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "nlperf.h"
#include "dircontrol.h"
#include "nlprocess.h"
#include "NLSEDrmFileList.h"
#include "NLSEDrmPathList.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;
extern ULONG nlseLoggingFlags;
extern ULONG g_SystemProcessId;

extern
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
const CHAR*
NLGetCurrentProcessName(
                        );

/**********************************************************************
    Declare Local Routines
***********************************************************************/
static
FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetInfoWhenSafe (__inout PFLT_CALLBACK_DATA Data,
                                    __in PCFLT_RELATED_OBJECTS FltObjects,
                                    __in PVOID CompletionContext,
                                    __in FLT_POST_OPERATION_FLAGS Flags);

static
VOID
NLSEHandleDirectoryQuery(
                         __in PFLT_FILTER Filter,
                         __in PFLT_INSTANCE Instance,
                         __in PCUNICODE_STRING DirectoryName,
                         __in BOOLEAN ProgramDir,
                         __in FILE_INFORMATION_CLASS FileInfoClass,
                         __in PVOID DirectoryBuffer,
                         __in PFILE_OBJECT DirectoryObject,
                         __in PNLDINFOLIST DList
                         );

static
VOID
NLSEAdjustFileSizeInPostQueryInformation(
    __inout PFLT_CALLBACK_DATA Data
    );

static
FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreWriteBuffered(
                                __inout PFLT_CALLBACK_DATA Data,
                                __in PCFLT_RELATED_OBJECTS FltObjects,
                                __in PNLFSE_STREAM_CONTEXT StmContext
                                );

static
FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostWriteWhenSafe (
                               __inout PFLT_CALLBACK_DATA Data,
                               __in PCFLT_RELATED_OBJECTS FltObjects,
                               __in PVOID CompletionContext,
                               __in FLT_POST_OPERATION_FLAGS Flags
                               );

static
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSECalculateAesPadWhenFileGrow(
                                __in PFLT_INSTANCE Instance,
                                __in PFILE_OBJECT FileObject,
                                __in ULONG SectorSize,
                                __in_bcount(16) PUCHAR AesKey,
                                __in const LARGE_INTEGER* FileSizePreSet,
                                __in const LARGE_INTEGER* FileSizePostSet,
                                __inout PULONG AesPadLength,
                                __inout_bcount_full(16) PUCHAR AesPadData
                                );

static
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSECalculateAesPadWhenFileShrink(
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_OBJECT FileObject,
                                  __in ULONG SectorSize,
                                  __in const LARGE_INTEGER* FileSizePreSet,
                                  __in const LARGE_INTEGER* FileSizePostSet,
                                  __inout PULONG AesPadLength,
                                  __inout_bcount_full(16) PUCHAR AesPadData
                                  );

static
__checkReturn
NTSTATUS
NLSEIsPathEncryptedEx(
                      __in PCFLT_RELATED_OBJECTS FltObjects,
                      __in PCUNICODE_STRING FullPath,
                      __in PCUNICODE_STRING VolumeName,
                      __in PCUNICODE_STRING VolumeDosName,
                      __in BOOLEAN* Encrypted
                      );

static
__checkReturn
BOOLEAN
IsUncheckDiretory(
                  __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                  __in PCUNICODE_STRING FileName
                  );

static
__drv_maxIRQL(APC_LEVEL)
VOID
NLSEFlushStreamContext(
                       __in PFLT_INSTANCE Instance,
                       __in PFILE_OBJECT FileObject,
                       __in BOOLEAN Paging,
                       __inout PNLFSE_STREAM_CONTEXT StmContext
                       );

static
__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
NLSEDirectoryControlRoutine(
                            __in PFLT_GENERIC_WORKITEM FltWorkItem,
                            __in PVOID FltObject,
                            __in_opt PVOID WorkItemContext
                            );

static
__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
NLSERenameWorkRoutine(
                      __in PFLT_GENERIC_WORKITEM FltWorkItem,
                      __in PVOID FltObject,
                      __in_opt PVOID WorkItemContext
                      );

static
FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostReadWhenSafe (
                                 __inout PFLT_CALLBACK_DATA Data,
                                 __in PCFLT_RELATED_OBJECTS FltObjects,
                                 __in PVOID CompletionContext,
                                 __in FLT_POST_OPERATION_FLAGS Flags
                                 );


/*************************************************************************
  MiniFilter callback routines.
*************************************************************************/
/* This struct is only used in Pre/Post Create */
typedef struct _NLSE_CREATE_CONTEXT
{
    UNICODE_STRING  FullFileName;
    BOOLEAN         Existing;
    BOOLEAN         Directory;
    ULONG           FileAttrs;
    LARGE_INTEGER   FileSize;
    ULONG           SectorSize;
} NLSE_CREATE_CONTEXT, *PNLSE_CREATE_CONTEXT;

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreCreate (__inout PFLT_CALLBACK_DATA Data,
                          __in PCFLT_RELATED_OBJECTS FltObjects,
                          __deref_out_opt PVOID *CompletionContext)
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    UNICODE_STRING              FullFileName= {0};
    ULONG                       FileNameType= 0;
    PNLSE_CREATE_CONTEXT        Context     = NULL;
    NLFSE_PVOLUME_CONTEXT       VolContext  = NULL;
    PIO_SECURITY_CONTEXT        SecurityCtx = NULL;
    ULONG                       CreateOptions       = 0;
    ULONG                       CreateDisposition   = 0;

    BOOLEAN                     FromSystem  = FALSE;
    BOOLEAN                     Trusted     = FALSE;
    BOOLEAN                     Existing    = FALSE;
    BOOLEAN                     Directory   = FALSE;
    BOOLEAN                     Encrypted   = FALSE;
    ULONG                       FileAttrs   = 0;
    LARGE_INTEGER               FileSize    = {0};

    ULONG                       CallerProcessFlag= 0; // 0: unknown, 1:untrusted, 2: trusted
    ULONG                       CurrentProcessId = 0;
    ULONG                       CallerProcessId  = 0;
    const CHAR*                 CallerName       = NULL;

	ANSI_STRING					Ntrestoreapp1, temp ;


#ifdef NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    // Is the request from SYSTEM
    FromSystem = (g_SystemProcessId == FltGetRequestorProcessId(Data))?TRUE:FALSE;
    
    CreateOptions       = Data->Iopb->Parameters.Create.Options & 0x00FFFFFF;
    CreateDisposition   = (Data->Iopb->Parameters.Create.Options & 0xFF000000) >> 24;
    SecurityCtx         = Data->Iopb->Parameters.Create.SecurityContext;





    // Init global data    
    /* Determine if the log file must be set. */
    if( nlseLoggingFlags != 0 && nlfseGlobal.log_file_set == FALSE )
    {
        ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
        if( nlfseGlobal.log_file_set == FALSE )
        {
            UNICODE_STRING log_file;
            nlfseGlobal.log_file_set = TRUE;
            RtlInitUnicodeString(&log_file,L"\\??\\C:\\Program Files\\NextLabs\\System Encryption\\diags\\logs\\se.klog");
            NL_KLOG_SetFile(&nlseKLog,&log_file,1024*1024); /* 1MB log size  */
        }
        ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
    }

	//Init Crypto Functions
	if(nlfseGlobal.CryptoInit == FALSE)
	{

		nlfseGlobal.CryptoInit = TRUE;

		NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "[NLSE]: DriverEntry: loading nl_crypto\n");
		if( nl_crypto_initialize(&nlfseGlobal.crypto_ctx,0x0) != NL_CRYPTO_ERROR_SUCCESS )
		{
		  nlfseGlobal.CryptoInitSuccess = FALSE;
		  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "[NLSE]: DriverEntry: nl_crypto_initialize failed\n");
		}

		if( nlfseGlobal.crypto_ctx.rand == NULL ||
		nlfseGlobal.crypto_ctx.encrypt == NULL ||
		nlfseGlobal.crypto_ctx.decrypt == NULL )
		{
		  nlfseGlobal.CryptoInitSuccess = FALSE;
		  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
			  "[NLSE]: DriverEntry: nl_crypto_initialize failed (callback NULL)\n");
		
		}
		
	}
    if(NULL==FltObjects->Filter || NULL==FltObjects->Volume)
        goto _exit;
    
    CurrentProcessId = (ULONG)PsGetCurrentProcessId();
    CallerProcessId  = FltGetRequestorProcessId(Data);
    CallerName       = NLGetCurrentProcessName();
    if(NULL == CallerName)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "[NLSE]: Fail to get current process name in PreCreate (caller is %d)\n", CallerProcessId);
    }
    if(CurrentProcessId != CallerProcessId)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "[NLSE]: current process is not caller (current=%d, caller=%d)\n", CurrentProcessId, CallerProcessId);
    }

    Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolContext);
    if(!NT_SUCCESS(Status) || NULL==VolContext)
    {
        VolContext = NULL;
        goto _exit;
    }

    // Get destination name
    Status = NLSEParseFileName(VolContext, Data, &FullFileName, &FileNameType);
    if(!NT_SUCCESS(Status) || NULL==FullFileName.Buffer) goto _exit;

    // Don't handle it if
    //   a. Fail to parse file name
    //   b. It is reserved NTFS file name
    //   c. It is a network share file name
    //   d. It is a stream file name
    //   e. It is a system file
    if(NLSE_INVALID_FILENAME==FileNameType) goto _exit;
    if(FlagOn(FileNameType, NLSE_RESERVED_FILENAME)) goto _exit;
    if(FlagOn(FileNameType, NLSE_NETSHARE_FILENAME)) goto _exit;
    if(FlagOn(FileNameType, NLSE_STREAM_FILENAME)) goto _exit;
    if(FlagOn(FileNameType, NLSE_SYSTEM_FILENAME)) goto _exit;
    if(FlagOn(FileNameType, NLSE_IGNORED_FILENAME)) goto _exit;
    if(FlagOn(FileNameType, NLSE_NONDRM_FILENAME)) goto _exit;

    // Don't handle it if cannot get file attributes
    Status = NLSEGetFileAttributes(FltObjects->Filter, FltObjects->Instance, (USHORT)VolContext->SectorSize, &FullFileName, &Existing, &Directory, &Encrypted, &FileAttrs, &FileSize);
    if(STATUS_SHARING_VIOLATION == Status)
    {
        // We cannot open the file because of OpLocked
        // According to DDK Help:
        //   If a filter or minifilter cannot honor the FILE_COMPLETE_IF_OPLOCKED flag,
        //   it must complete the IRP_MJ_CREATE request with STATUS_SHARING_VIOLATION.
        Data->IoStatus.Status   = STATUS_SHARING_VIOLATION;
        FltStatus               = FLT_PREOP_COMPLETE;
        goto _exit;
    }
    if(!NT_SUCCESS(Status))
    {
        goto _exit;
    }


    // Check
    if(Existing)
    {
        // Don's handle directory, EFS and compressed file
        if(Directory) goto _exit;
        if(FlagOn(FileAttrs, FILE_ATTRIBUTE_ENCRYPTED) || FlagOn(FileAttrs, FILE_ATTRIBUTE_COMPRESSED)) goto _exit;

        // If this file is not a encrypted file, don't handle it
        if(!Encrypted) goto _exit;

        // Is it trusted process?
        CallerProcessFlag = NLGetProcessInfo((HANDLE)CallerProcessId);
        if(2 != CallerProcessFlag)
        {
            Trusted = NT_SUCCESS(NLSEUpdateCurrentPCKey(FltGetRequestorProcessId(Data), FALSE))?TRUE:FALSE;
            if(Trusted)
            {
                NLUpdateProcessInfo((HANDLE)CallerProcessId, 2);
                KdPrint(( "\nnlse: process (%d) %s become trusted\n\n", CallerProcessId, ((NULL!=CallerName)?CallerName:"(Unknown)") ));
            }
        }
        else
        {
            Trusted = TRUE;
            KdPrint(( "\nnlse: process (%d) %s is trusted according to cache\n\n", CallerProcessId, ((NULL!=CallerName)?CallerName:"(Unknown)")));
        }

        // In following cases, opening encrypted file is not allowed
        //   a. Policy Controller is not up OR caller is not a trusted process.
        //   - AND -
        //   b. Caller is trying to read, write or execute the data.
        if ((SecurityCtx->DesiredAccess &
             (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL |
              FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA | FILE_EXECUTE))
            != 0)
        {

			if(!nlfseGlobal.bEnable)
			{
				//check if it process requesting access is NT restore application sdclt.exe and svchost.exe 
				if ( NULL != CallerName)
				{
					RtlInitAnsiString(&Ntrestoreapp1, "sdclt.exe");
					RtlInitAnsiString(&temp,CallerName);
					if(RtlCompareString(&Ntrestoreapp1 ,&temp,TRUE) == 0 )
						goto _exit;
				}

				Data->IoStatus.Status   = STATUS_ACCESS_DENIED;
				FltStatus               = FLT_PREOP_COMPLETE;
				goto _exit;
			} 
			if( !Trusted)
			{
				Data->IoStatus.Status   = STATUS_ACCESS_DENIED;
				FltStatus               = FLT_PREOP_COMPLETE;
				goto _exit;
			}
		}
        // Deny access from remote
        if(NLSEIsAccessFromRemote(SecurityCtx))
        {
            Data->IoStatus.Status   = STATUS_ACCESS_DENIED;
            FltStatus               = FLT_PREOP_COMPLETE;
            goto _exit;
        }
    }
    else if (CreateDisposition == FILE_SUPERSEDE ||
             CreateDisposition == FILE_CREATE ||
             CreateDisposition == FILE_OPEN_IF ||
             CreateDisposition == FILE_OVERWRITE_IF)
    {
        // The file doesn't exist, and the Create op is actually trying to
        // create the non-existing file.  So check for all of the things
        // below.
        // (Otherwise, if the Create op is not trying to create the file
        // (e.g. opening for reading attribute), there is no point of checking
        // for all of the following as the Create op will fail anyway.)

        UNICODE_STRING DosFileName;
        BOOLEAN isDrmFileOneShot = FALSE;

        // Don't handle directory, EFS and compressed file.
        if(FlagOn(CreateOptions, FILE_DIRECTORY_FILE)) goto _exit;
        if(FlagOn(FileAttrs, FILE_ATTRIBUTE_ENCRYPTED) || FlagOn(FileAttrs, FILE_ATTRIBUTE_COMPRESSED)) goto _exit;

        // For .nxl file, since we can't check if the file is a wrapped file
        // by checking the NLF_WRAPPED flag in the header before the file is
        // created, we'll have to assume that it will be a wrapped file.  So
        // don't handle it.
        if(FlagOn(FileNameType, NLSE_NEXTLABS_FILENAME)) goto _exit;

        // If parent directory is not DRM directory, don't handle it
        Status = NLSEIsPathEncryptedEx(FltObjects, &FullFileName, &VolContext->Name, &VolContext->DosName, &Encrypted);
        if(!NT_SUCCESS(Status))
        {
            // If this is not a DRM directory, don't handle it
            Encrypted = FALSE;
        }

        {
          Status = NLSEBuildDosPath(&FullFileName, &VolContext->Name,
                                    &VolContext->DosName, &DosFileName);
          if(NT_SUCCESS(Status))
          {
            Status = NLSEDrmFileOneShotListCheck(&DosFileName, CallerProcessId,
                                                 &isDrmFileOneShot);
            if(!NT_SUCCESS(Status))
            {
              isDrmFileOneShot = FALSE;
            }

            ExFreePool(DosFileName.Buffer);
            DosFileName.Buffer = NULL;
          }
        }

        if(!Encrypted && !isDrmFileOneShot)
            goto _exit;

        // Is it trusted process?
        CallerProcessFlag = NLGetProcessInfo((HANDLE)CallerProcessId);
        if(2 != CallerProcessFlag)
        {
            Trusted = NT_SUCCESS(NLSEUpdateCurrentPCKey(FltGetRequestorProcessId(Data), FALSE))?TRUE:FALSE;
            if(Trusted)
            {
                NLUpdateProcessInfo((HANDLE)CallerProcessId, 2);
                KdPrint(( "\nnlse: process (%d) %s become trusted\n\n", CallerProcessId, ((NULL!=CallerName)?CallerName:"(Unknown)") ));
            }
        }
        else
        {
            Trusted = TRUE;
            KdPrint(( "\nnlse: process (%d) %s is trusted according to cache\n\n", CallerProcessId, ((NULL!=CallerName)?CallerName:"(Unknown)")));
        }

        // In following cases, create new file under DRM directory is not allowed
        //   a. Policy Controller is not up, aloww user create new file, but the file is not encrypted
        if(!nlfseGlobal.bEnable)
        {
            // Don't need to set up any context, let application write raw data to file
            // And the file won't be encrypted
            goto _exit;
        }
        //   b. Caller is not a trusted process
        if(!Trusted)
        {
            Data->IoStatus.Status   = STATUS_ACCESS_DENIED;
            FltStatus               = FLT_PREOP_COMPLETE;
            goto _exit;
        }

        // Deny access from remote
        if(NLSEIsAccessFromRemote(SecurityCtx))
        {
            Data->IoStatus.Status   = STATUS_ACCESS_DENIED;
            FltStatus               = FLT_PREOP_COMPLETE;
            goto _exit;
        }

        FileSize.QuadPart   = 0;
        FileAttrs           = Data->Iopb->Parameters.Create.FileAttributes;
    }

    // No EFS/Compress flags is allowed for encrypted file
    if( (Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
        || (Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_COMPRESSED)
        )
    {
        Data->Iopb->Parameters.Create.FileAttributes &= (~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED));
        FltSetCallbackDataDirty(Data);
    }

	// Checking if the initialization of Crypto Modules was ok, in the context of an encrypted file
	if( nlfseGlobal.CryptoInitSuccess == FALSE ||  nlfseGlobal.CryptoInit == FALSE)
	{
		Data->IoStatus.Status = STATUS_CRYPTO_SYSTEM_INVALID;
		FltStatus = FLT_PREOP_COMPLETE;
		goto _exit;	
	}

    KdPrint(("\nnlse: %s open %wZ\n\n", ((NULL!=CallerName)?CallerName:"(Unknown)"), &FullFileName));

    Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_CREATE_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == Context) goto _exit;
    RtlZeroMemory(Context, sizeof(NLSE_CREATE_CONTEXT));

    RtlCopyMemory(&Context->FullFileName, &FullFileName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&FullFileName, sizeof(UNICODE_STRING));
    Context->Existing           = Existing;
    Context->Directory          = Directory;
    Context->FileAttrs          = FileAttrs;
    Context->FileSize.QuadPart  = FileSize.QuadPart;
    Context->SectorSize         = VolContext->SectorSize;
    *CompletionContext          = Context; Context = NULL;
    FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

_exit:

#ifdef	NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("pre-create elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

    if(NULL != VolContext) FltReleaseContext(VolContext); VolContext = NULL;
    if(NULL != FullFileName.Buffer) ExFreePool(FullFileName.Buffer); FullFileName.Buffer=NULL;
    return FltStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostCreate (__inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __in PVOID CompletionContext,
                           __in FLT_POST_OPERATION_FLAGS Flags)
{
    PNLSE_CREATE_CONTEXT    Context     = CompletionContext;
    PNLFSE_STREAM_CONTEXT   StmContext  = NULL;
    NTSTATUS                Status      = STATUS_SUCCESS;
    LARGE_INTEGER           Offset      = {0};
    BOOLEAN                 PcKeyChanged= FALSE;

#ifdef	NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    if(NULL == Context) goto _exit;    
    if(!NT_SUCCESS(Data->IoStatus.Status)) goto _exit;

    
    switch(Data->IoStatus.Information)
    {
    case FILE_OPENED:       // An existing file is opened
        // a. Try to find an existing stream context
        if((!Context->Directory) && Context->FileSize.QuadPart < NLSE_ENVELOPE_SIZE)
            break;

        Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext);
        if(STATUS_NOT_FOUND==Status || NULL==StmContext)
        {
            // b. If there is no existing stream context, read header from file and setup stream context
            Status = NLSEBuildStreamContext(FltObjects->Filter, &Context->FullFileName, &Context->FileSize, Context->FileAttrs, Context->SectorSize, &StmContext);
            if(!NT_SUCCESS(Status)) goto _exit;

            // Good, we have the context now, read file header from file
            Status = NLSEReadEncryptSection2(FltObjects->Instance, FltObjects->FileObject, &StmContext->encryptExt);
            if(!NT_SUCCESS(Status)) goto _exit;

            if(StmContext->FileSize.QuadPart != StmContext->encryptExt->fileRealLength)
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[NLSE::OpenFile] Inconsistant file size.\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Name:        %wZ\n", &StmContext->FileName);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Current:     %I64d\n", StmContext->FileSize.QuadPart);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Pad Current: %d\n", (16 - StmContext->FileSize.QuadPart%16)%16);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Encrypt:     %I64d\n", StmContext->encryptExt->fileRealLength);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Pad Encrypt: %d\n", (16 - StmContext->encryptExt->fileRealLength%16)%16);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"                 Pad Real:    %d\n", StmContext->encryptExt->paddingLen);
            }

            // Try to decrypt AesKey
            Status = NLSEDecryptAesKey2(FltGetRequestorProcessId(Data), StmContext->encryptExt, &PcKeyChanged);
            if(!NT_SUCCESS(Status)) goto _exit;

            // Key Rotation:
            //     If PC key changed, we need to update it to current key
			if(StmContext->encryptExt->pcKeyID.timestamp < nlfseGlobal.currentPCKeyID.timestamp && PcKeyChanged)
			{
				//if()
				 //{
					NLSEUpdateEncryptSection2(FltObjects->Instance,
						FltObjects->FileObject,
						FALSE,
						NLE_KEY_RING_LOCAL,
						(PUCHAR)&nlfseGlobal.currentPCKeyID,
						nlfseGlobal.currentPCKey,
						StmContext->encryptExt);
				 //}
			}

            // Clean cache
            NLSEPurgeFileCache(FltObjects->FileObject, FALSE, NULL, NLSE_ENVELOPE_SIZE);

            // Set context
            Status = FltSetStreamContext(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_REPLACE_IF_EXISTS, StmContext, NULL);
            if(NT_SUCCESS(Status))
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Insert stream context when file is opened\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Name:   %wZ\n", &StmContext->FileName);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size:   %d\n", StmContext->FileSize.QuadPart);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Sector Size: %d\n", StmContext->SectorSize);
            }
        }
		else
			{
            // Check  if the key changed            
            if(sizeof(NLSE_KEY_ID) != RtlCompareMemory(&StmContext->encryptExt->pcKeyID, (PUCHAR)&nlfseGlobal.currentPCKeyID, sizeof(NLSE_KEY_ID)) && StmContext->encryptExt->pcKeyID.timestamp < nlfseGlobal.currentPCKeyID.timestamp)
            {
                // if it changed, update extension and update the header section

                NLSEUpdateEncryptSection2(FltObjects->Instance,
                    FltObjects->FileObject,
                    FALSE,
                    NLE_KEY_RING_LOCAL,
                    (PUCHAR)&nlfseGlobal.currentPCKeyID,
                    nlfseGlobal.currentPCKey,
                    StmContext->encryptExt);
            }
        }
#ifdef NLSE_DEBUG_PERFORMANCE
		KdPrint(("File Name:   %wZ\n", &StmContext->FileName));
#endif
        break;

    case FILE_OVERWRITTEN:  // An existing file is overwritten
    case FILE_SUPERSEDED:   // An existing file is superseded
        // Since the file is empty now
        // We need to treat it as a new file
        // And add a new stream context
        // If the stream context already exists, we should replace it
        Context->FileSize.QuadPart   = 0;

    case FILE_CREATED:      // A new file is created
        // It is a new file
        // a. Allocate/Fill Stream Context
        // b. Set up stream context
        // c. Write the header to file
        Status = NLSEBuildStreamContext(FltObjects->Filter, &Context->FullFileName, &Context->FileSize, Context->FileAttrs, Context->SectorSize, &StmContext);
        if(!NT_SUCCESS(Status)) goto _exit;

        // Generic NextLabs envelope Header (With sections)
        Status = NLSEGenerateEmptyHeader2(FltObjects->Instance,
            FltObjects->FileObject,
            NULL,
            (PUCHAR)&nlfseGlobal.currentPCKeyID,
            nlfseGlobal.currentPCKey,
            &StmContext->encryptExt);
        if(!NT_SUCCESS(Status)) goto _exit;
        // Since we have already write file header, we need to purge cache
        NLSEPurgeFileCache(FltObjects->FileObject, FALSE, NULL, NLSE_ENVELOPE_SIZE);
        StmContext->FileSize.QuadPart = 0;

        // Set context
        Status = FltSetStreamContext(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_REPLACE_IF_EXISTS, StmContext, NULL);
        if(NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Insert stream context when file is created/overwritten\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Name:   %wZ\n", &StmContext->FileName);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size:   %d\n", StmContext->FileSize.QuadPart);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Sector Size: %d\n", StmContext->SectorSize);
        }
        break; 

    default:
        goto _exit;
    }

_exit:
    if (NULL!=Context)
    {
	 	if (NULL!=Context->FullFileName.Buffer) ExFreePool(Context->FullFileName.Buffer);
        ExFreePool(Context);
    }
    if(NULL!=StmContext) FltReleaseContext(StmContext);
    
#ifdef NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("post-create elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif
    
    return FLT_POSTOP_FINISHED_PROCESSING;
}

typedef struct _NLSE_READ_CONTEXT
{
    PNLFSE_STREAM_CONTEXT   StmContext;
    PUCHAR                  SeReadBuffer;
    PMDL                    SeReadMdl;
    LARGE_INTEGER           SeReadOffset;
    ULONG                   SeReadLength;
    PUCHAR                  OriginalReadBuffer;
    PMDL                    OriginalReadMdl;
    LARGE_INTEGER           OriginalReadOffset;
    ULONG                   OriginalReadLength;
} NLSE_READ_CONTEXT, *PNLSE_READ_CONTEXT;

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreRead(
                       __inout PFLT_CALLBACK_DATA Data,
                       __in PCFLT_RELATED_OBJECTS FltObjects,
                       __deref_out_opt PVOID *CompletionContext
                       )
/*++
Routine Description:
This routine demonstrates how to swap buffers for the READ operation.
Note that it handles all errors by simply not doing the buffer swap.

Arguments:
Data - Pointer to the filter callbackData that is passed to us.
FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
opaque handles to this filter, instance, its associated volume and
file object.
CompletionContext - Receives the context that will be passed to the
post-operation callback.

Return Value:
FLT_PREOP_SUCCESS_WITH_CALLBACK - we want a postOpeation callback
FLT_PREOP_SUCCESS_NO_CALLBACK - we don't want a postOperation callback
--*/
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;
    PNLSE_READ_CONTEXT          Context     = NULL;

#ifdef NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    // Sanity check
    if(0 == Data->Iopb->Parameters.Read.Length)
        goto _exit;

    // Try to get stream context
    Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext);
    if(!NT_SUCCESS(Status) || StmContext == NULL)
        goto _exit;

    // Fast I/O is not allowed for a encrypted file
    if(FLT_IS_FASTIO_OPERATION(Data))
    {
        Data->IoStatus.Information = 0;
        FltStatus = FLT_PREOP_DISALLOW_FASTIO;
        goto _exit;
    }

    // We still need to handle Buffered I/O
    if(!NLSE_IS_NONCACHE_OR_PAGING_IO(Data))
    {
        *CompletionContext = StmContext; StmContext = NULL;
        FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
        goto _exit;
    }


    // Okay, it is an encrypted file, allocate pre to post context
    Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_READ_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == Context)
    {
        FltStatus                   = FLT_PREOP_COMPLETE;
        Data->IoStatus.Status       = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information  = 0;
        goto _exit;
    }
    RtlZeroMemory(Context, sizeof(NLSE_READ_CONTEXT));

    // Initialize this context
    Context->StmContext                 = StmContext; StmContext = NULL;
    Context->OriginalReadOffset.QuadPart= Data->Iopb->Parameters.Read.ByteOffset.QuadPart;                      // Modify the offset, add envelope header size
    Context->OriginalReadLength         = Data->Iopb->Parameters.Read.Length; //ROUND_TO_SIZE(Data->Iopb->Parameters.Read.Length, Context->StmContext->SectorSize);
    Context->SeReadOffset.QuadPart      = Context->OriginalReadOffset.QuadPart + NLSE_ENVELOPE_SIZE;            // Since we only handle sector size is 512, so we don't need to change it
    Context->SeReadLength               = ROUND_TO_SIZE(Context->OriginalReadLength, Context->StmContext->SectorSize);
    Context->SeReadLength               = ROUND_TO_SIZE(Context->SeReadLength, nlfseGlobal.cbcBlockSize);       // Actually, since it is a non-cached I/O, it must aligned with sector size (512)
                                                                                                                // It must also be aligned with nlfseGlobal.cbcBlockSize (16)
    Context->SeReadBuffer               = ExAllocatePoolWithTag(NonPagedPool, Context->SeReadLength, NLFSE_BUFFER_TAG);
    if(NULL == Context->SeReadBuffer)
    {
        FltStatus                   = FLT_PREOP_COMPLETE;
        Data->IoStatus.Status       = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information  = 0;
        goto _exit;
    }
    RtlZeroMemory(Context->SeReadBuffer, Context->SeReadLength);

    //  We only need to build a MDL for IRP operations.  We don't need to
    //  do this for a FASTIO operation since the FASTIO interface has no
    //  parameter for passing the MDL to the file system.
    if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION))
    {
        //  Allocate a MDL for the new allocated memory.  If we fail
        //  the MDL allocation then we won't swap buffer for this operation
        Context->SeReadMdl = IoAllocateMdl( Context->SeReadBuffer, Context->SeReadLength, FALSE, FALSE, NULL );
        if (Context->SeReadMdl == NULL)
        {
            FltStatus                   = FLT_PREOP_COMPLETE;
            Data->IoStatus.Status       = STATUS_INSUFFICIENT_RESOURCES;
            Data->IoStatus.Information  = 0;
            goto _exit;
        }

        //  setup the MDL for the non-paged pool we just allocated
        MmBuildMdlForNonPagedPool(Context->SeReadMdl);
    }

    // Done, swap buffer
    Data->Iopb->Parameters.Read.ReadBuffer          = Context->SeReadBuffer;
    Data->Iopb->Parameters.Read.MdlAddress          = Context->SeReadMdl;
    Data->Iopb->Parameters.Read.ByteOffset.QuadPart = Context->SeReadOffset.QuadPart;
    Data->Iopb->Parameters.Read.Length              = Context->SeReadLength;
    FltSetCallbackDataDirty(Data);

    // use post callback
    *CompletionContext = Context; Context = NULL;
    FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    if(Context)
    {
        if(NULL != Context->StmContext)   FltReleaseContext(Context->StmContext); Context->StmContext   = NULL;
        if(NULL != Context->SeReadBuffer) ExFreePool(Context->SeReadBuffer);      Context->SeReadBuffer = NULL;
        if(NULL != Context->SeReadMdl)    IoFreeMdl(Context->SeReadMdl);          Context->SeReadMdl    = NULL;
        ExFreePool(Context);
    }
    
#ifdef NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart/1000 )
		KdPrint(("pre-read elasped time = %I64d milliseconds\n", pfc.diff.QuadPart/1000 ));
#endif
    
    return FltStatus;
} /*-- NLFSEOpCallbackPreRead --*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostRead(
                        __inout PFLT_CALLBACK_DATA Data,
                        __in PCFLT_RELATED_OBJECTS FltObjects,
                        __in_opt PVOID CompletionContext,
                        __in FLT_POST_OPERATION_FLAGS Flags
                        )
/*++
Routine Description:
    This routine does postRead buffer swap handling

Arguments:
Data       - Pointer to the filter callbackData that is passed to us.
FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
             opaque handles to this filter, instance, its associated volume and
             file object.
CompletionContext - The completion context set in the pre-operation routine.
Flags      - Denotes whether the completion is successful or is being drained.

Return Value:
FLT_POSTOP_FINISHED_PROCESSING
FLT_POSTOP_MORE_PROCESSING_REQUIRED
 --*/
{    
    FLT_POSTOP_CALLBACK_STATUS  FltStatus   = FLT_POSTOP_FINISHED_PROCESSING;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLSE_READ_CONTEXT          Context     = NULL;
    ULONG                       BytesRead   = (ULONG)Data->IoStatus.Information;
    ULONG                       BytesDecrypt= ROUND_TO_SIZE(BytesRead, nlfseGlobal.cbcBlockSize);
    KIRQL                       IrqlOld;
    ULONG                       AesPadLength= 0;
    UCHAR                       AesPadData[16] = {0};
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;

    ULONG                       PadOffsetInSeBuffer = 0;
    ULONG                       PadLengthInSeBuffer = 0;
    
#ifdef NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    //  This system won't draining an operation with swapped buffers, verify
    //  the draining flag is not set.
    ASSERT(!FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING));

    if(NULL == CompletionContext)
        return FLT_POSTOP_FINISHED_PROCESSING;
    
    // We still need to handle Buffered I/O
    if(!NLSE_IS_NONCACHE_OR_PAGING_IO(Data))
    {
        // Fail to read? just return
        if(BytesRead <= 0) goto _exit;

        // Check file size
        StmContext = (PNLFSE_STREAM_CONTEXT)CompletionContext;
        if(StmContext->encryptExt->fileRealLength <= (UINT64)Data->Iopb->Parameters.Read.ByteOffset.QuadPart)
        {
            Data->IoStatus.Status      = STATUS_END_OF_FILE;
            Data->IoStatus.Information = 0;
        }
        else if( ((UINT64)Data->Iopb->Parameters.Read.ByteOffset.QuadPart+BytesRead) > StmContext->encryptExt->fileRealLength )
        {
            Data->IoStatus.Information = (ULONG)(StmContext->encryptExt->fileRealLength - Data->Iopb->Parameters.Read.ByteOffset.QuadPart);
        }
        else
        {
            ; // Do nothing
        }

        goto _exit;
    }

    // Non-cache or Paging I/O
    StmContext  = NULL;
    Context     = (PNLSE_READ_CONTEXT)CompletionContext;

    // Fail to read? just return
    if(BytesRead <= 0)
        goto _exit;

    try
    {
        //  We need to copy the read data back into the users buffer.  Note
        //  that the parameters passed in are for the users original buffers
        //  not our swapped buffers.
        if (Data->Iopb->Parameters.Read.MdlAddress != NULL)
        {
            //  There is a MDL defined for the original buffer, get a
            //  system address for it so we can copy the data back to it.
            //  We must do this because we don't know what thread context
            //  we are in.
            Context->OriginalReadBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Read.MdlAddress, NormalPagePriority );
            if (Context->OriginalReadBuffer == NULL)
            {
                Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Data->IoStatus.Information = 0;
                leave;
            }
        }
        else if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) || FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION))
        {
            //  If this is a system buffer, just use the given address because it is valid in all thread contexts.
            //  If this is a FASTIO operation, we can just use the buffer (inside a try/except) since we know we are in
            //     the correct thread context (you can't pend FASTIO's).
            Context->OriginalReadBuffer = Data->Iopb->Parameters.Read.ReadBuffer;
        }
        else
        {
            //  They don't have a MDL and this is not a system buffer or a fastio so this is probably some arbitrary user
            //  buffer.  We can not do the processing at DPC level so try and get to a safe IRQL so we can do the processing.
            if (FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, NLFSEOpCallbackPostReadWhenSafe, &FltStatus))
            {
                Context = NULL; // Don't release the context
            }
            else
            {
                //  We are in a state where we can not get to a safe IRQL and we do not have a MDL.  There is nothing we can do to safely
                //  copy the data back to the users buffer, fail the operation and return.  This shouldn't ever happen because in those
                //  situations where it is not safe to post, we should have a MDL.
                Data->IoStatus.Status       = STATUS_UNSUCCESSFUL;
                Data->IoStatus.Information  = 0;
            }
            leave;
        }

        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE::Read] Decrypt data\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Offset in raw Data:       %d\n", Context->OriginalReadOffset.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Original Bytes to Read:   %d\n", Context->OriginalReadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    SE read Offset:           %d\n", Context->SeReadOffset.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    SE Bytes to Read:         %d\n", Context->SeReadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Real Bytes Read:          %d\n", BytesRead);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding needed:           %d\n", (BytesDecrypt-BytesRead));


        // If we get original buffer, decrypt data we read in SeBuffer, and copy it to original buffer
        // Do we need to use padding?
        if(Context->StmContext->encryptExt->fileRealLength <= (UINT64)Context->OriginalReadOffset.QuadPart)
        {
            // The new data is written buffered, so it has not been on disk
            // Current tagging/data is still the old one
            goto _exit;
        }
        else
        {
            PadOffsetInSeBuffer = (ULONG)(Context->StmContext->encryptExt->fileRealLength - Context->OriginalReadOffset.QuadPart);
            PadLengthInSeBuffer = Context->StmContext->encryptExt->paddingLen;

            if(PadOffsetInSeBuffer > BytesDecrypt)
            {
                if(BytesRead != BytesDecrypt)
                {
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    * Padding Offset is not correct.\n");
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"        Current Offset is  %I64d\n", Context->StmContext->encryptExt->fileRealLength);
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"        Expected Offset is %I64d\n", (Context->OriginalReadOffset.QuadPart+BytesRead));
                    goto _exit;
                }
            }
            else
            {
                BytesDecrypt        = ROUND_TO_SIZE(PadOffsetInSeBuffer, nlfseGlobal.cbcBlockSize);
                PadLengthInSeBuffer = BytesDecrypt - PadOffsetInSeBuffer;
                KeAcquireSpinLock(&Context->StmContext->encryptExtLock, &IrqlOld);
                RtlCopyMemory( Context->SeReadBuffer+PadOffsetInSeBuffer, Context->StmContext->encryptExt->paddingData, PadLengthInSeBuffer);
                KeReleaseSpinLock(&Context->StmContext->encryptExtLock, IrqlOld);
            }
        }

        if(!decrypt_buffer(Context->StmContext->encryptExt->key,
            NLE_KEY_LENGTH_IN_BYTES,
            Context->OriginalReadOffset.QuadPart,    // We should use the real offset in RAW data to decrypt the file
            Context->SeReadBuffer,
            BytesDecrypt))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    * Fail to decrypt data\n");
            Data->IoStatus.Status       = STATUS_UNSUCCESSFUL;
            Data->IoStatus.Information  = 0;
            leave;
        }
        
        // Great! we decrypt the buffer, copy the data we needed
        Data->IoStatus.Information  = min(Context->OriginalReadLength, BytesRead);
        RtlCopyMemory(Context->OriginalReadBuffer, Context->SeReadBuffer, Data->IoStatus.Information);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        //  The copy failed, return an error, failing the operation.
        Data->IoStatus.Status       = GetExceptionCode();
        Data->IoStatus.Information  = 0;
    }

_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    if(Context)
    {
        if(NULL != Context->StmContext)   FltReleaseContext(Context->StmContext); Context->StmContext   = NULL;
        if(NULL != Context->SeReadBuffer) ExFreePool(Context->SeReadBuffer);      Context->SeReadBuffer = NULL;
        ExFreePool(Context); Context = NULL;
    }

#ifdef NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("post-read elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif
    return FltStatus;
}/*--NLFSEOpCallbackPostRead--*/


FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostReadWhenSafe (
                                 __inout PFLT_CALLBACK_DATA Data,
                                 __in PCFLT_RELATED_OBJECTS FltObjects,
                                 __in PVOID CompletionContext,
                                 __in FLT_POST_OPERATION_FLAGS Flags
                                 )
{
    FLT_POSTOP_CALLBACK_STATUS  FltStatus   = FLT_POSTOP_FINISHED_PROCESSING;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLSE_READ_CONTEXT          Context     = (PNLSE_READ_CONTEXT)CompletionContext;
    ULONG                       BytesRead   = (ULONG)Data->IoStatus.Information;
    ULONG                       BytesDecrypt= ROUND_TO_SIZE(BytesRead, nlfseGlobal.cbcBlockSize);
    KIRQL                       IrqlOld;

    ULONG                       PadOffsetInSeBuffer = 0;
    ULONG                       PadLengthInSeBuffer = 0;

    try
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE::Read] Decrypt data\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Offset in raw Data:       %d\n", Context->OriginalReadOffset.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Original Bytes to Read:   %d\n", Context->OriginalReadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    SE Read Offset:           %d\n", Context->SeReadOffset.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    SE Bytes to Read:         %d\n", Context->SeReadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Real Bytes Read:          %d\n", BytesRead);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding needed:           %d\n", (BytesDecrypt-BytesRead));

        // Lock user buffer
        Status = FltLockUserBuffer(Data);
        if (!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    Fail to lock user data:   0x%08X\n", Status);
            Data->IoStatus.Status       = Status;
            Data->IoStatus.Information  = 0;
            leave;
        }

        // Get original buffer
        Context->OriginalReadBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Read.MdlAddress, NormalPagePriority );
        if (Context->OriginalReadBuffer == NULL)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    Fail to get original buffer\n");
            Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Data->IoStatus.Information = 0;
            leave;
        }        

        // If we get original buffer, decrypt data we read in SeBuffer, and copy it to original buffer
        // Do we need to use padding?
        if(Context->StmContext->encryptExt->fileRealLength <= (UINT64)Context->OriginalReadOffset.QuadPart)
        {
            // The new data is written buffered, so it has not been on disk
            // Current tagging/data is still the old one
            leave;
        }
        else
        {
            PadOffsetInSeBuffer = (ULONG)(Context->StmContext->encryptExt->fileRealLength - Context->OriginalReadOffset.QuadPart);
            PadLengthInSeBuffer = Context->StmContext->encryptExt->paddingLen;

            if(PadOffsetInSeBuffer > BytesDecrypt)
            {
                if(BytesRead != BytesDecrypt)
                {
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    * Padding Offset is not correct.\n");
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"        Current Offset is  %I64d\n", Context->StmContext->encryptExt->fileRealLength);
                    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"        Expected Offset is %I64d\n", (Context->OriginalReadOffset.QuadPart+BytesRead));
                    leave;
                }
            }
            else
            {
                BytesDecrypt        = ROUND_TO_SIZE(PadOffsetInSeBuffer, nlfseGlobal.cbcBlockSize);
                PadLengthInSeBuffer = BytesDecrypt - PadOffsetInSeBuffer;
                KeAcquireSpinLock(&Context->StmContext->encryptExtLock, &IrqlOld);
                RtlCopyMemory( Context->SeReadBuffer+PadOffsetInSeBuffer, Context->StmContext->encryptExt->paddingData, PadLengthInSeBuffer);
                KeReleaseSpinLock(&Context->StmContext->encryptExtLock, IrqlOld);
            }
        }

        if(!decrypt_buffer(Context->StmContext->encryptExt->key,
            NLE_KEY_LENGTH_IN_BYTES,
            Context->OriginalReadOffset.QuadPart,    // We should use the real offset in RAW data to decrypt the file
            Context->SeReadBuffer,
            BytesDecrypt))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"    * Fail to decrypt data\n");
            Data->IoStatus.Status       = STATUS_UNSUCCESSFUL;
            Data->IoStatus.Information  = 0;
            leave;
        }
        
        // Great! we decrypt the buffer, copy the data we needed
        Data->IoStatus.Information  = min(Context->OriginalReadLength, BytesRead);
        RtlCopyMemory(Context->OriginalReadBuffer, Context->SeReadBuffer, Data->IoStatus.Information);
    }
    finally
    {
        if(Context)
        {
            if(NULL != Context->StmContext)   FltReleaseContext(Context->StmContext); Context->StmContext   = NULL;
            if(NULL != Context->SeReadBuffer) ExFreePool(Context->SeReadBuffer);      Context->SeReadBuffer = NULL;
            ExFreePool(Context); Context = NULL;
        }
    }

    return FltStatus;
}


typedef struct _NLSE_WRITE_CONTEXT
{
    PNLFSE_STREAM_CONTEXT   StmContext;
    LARGE_INTEGER           FileSizePreWrite;
    PUCHAR                  SeWriteBuffer;
    PMDL                    SeWriteMdl;
    LARGE_INTEGER           SeWriteOffset;
    ULONG                   SeWriteLength;
    PUCHAR                  OriginalWriteBuffer;
    PMDL                    OriginalWriteMdl;
    LARGE_INTEGER           OriginalWriteOffset;
    ULONG                   OriginalWriteLength;
} NLSE_WRITE_CONTEXT, *PNLSE_WRITE_CONTEXT;

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreWrite(
                        __inout PFLT_CALLBACK_DATA Data,
                        __in PCFLT_RELATED_OBJECTS FltObjects,
                        __deref_out_opt PVOID *CompletionContext
                        )
/*++
Routine Description:
This routine demonstrates how to swap buffers for the WRITE operation.
Note that it handles all errors by simply not doing the buffer swap.

Arguments:
Data - Pointer to the filter callbackData that is passed to us.
FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
opaque handles to this filter, instance, its associated volume and
file object.
CompletionContext - Receives the context that will be passed to the
post-operation callback.

Return Value:
FLT_PREOP_SUCCESS_WITH_CALLBACK - we want a postOpeation callback
FLT_PREOP_SUCCESS_NO_CALLBACK - we don't want a postOperation callback
FLT_PREOP_COMPLETE -
--*/
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;
    PNLSE_WRITE_CONTEXT         Context     = NULL;
    LARGE_INTEGER               FileSizePreWrite = {0};


    // No stream context?
	Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext );
    if(!NT_SUCCESS(Status) || NULL == StmContext)
        goto _exit;


    // Fast I/O is not allowed for a encrypted file
    if(FLT_IS_FASTIO_OPERATION(Data))
    {
        Data->IoStatus.Information = 0;
        FltStatus = FLT_PREOP_DISALLOW_FASTIO;
        goto _exit;
    }
    
    // For buffered I/O, we have to calculate file size
    // If it is necessary, we also need to reset its size
    if(!NLSE_IS_NONCACHE_OR_PAGING_IO(Data))
    {
        FltStatus = NLFSEOpCallbackPreWriteBuffered(Data, FltObjects, StmContext);
        goto _exit;
    } 

    // Set pre-write file size
    FileSizePreWrite.QuadPart = StmContext->FileSize.QuadPart + NLSE_ENVELOPE_SIZE;

    // This is Non-cached or Paging I/O, we need to encrypt the data    
    Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_WRITE_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == Context)
    {
        FltStatus                   = FLT_PREOP_COMPLETE;
        Data->IoStatus.Status       = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information  = 0;
        goto _exit;
    }
    RtlZeroMemory(Context, sizeof(NLSE_READ_CONTEXT));

    // Initialize this context
    Context->StmContext                     = StmContext; StmContext = NULL;
    Context->FileSizePreWrite.QuadPart      = FileSizePreWrite.QuadPart;
    Context->OriginalWriteOffset.QuadPart   = Data->Iopb->Parameters.Write.ByteOffset.QuadPart + NLSE_ENVELOPE_SIZE; // Modify the offset, add envelope header size
    Context->OriginalWriteLength            = Data->Iopb->Parameters.Write.Length;
    // Since we only handle sector size is 512, so we don't need to change it
    Context->SeWriteOffset.QuadPart         = Context->OriginalWriteOffset.QuadPart;
    // Need to be alisgned with sector size (512), and it must also be aligned with nlfseGlobal.cbcBlockSize (16)
    Context->SeWriteLength                  = ROUND_TO_SIZE(Context->OriginalWriteLength, Context->StmContext->SectorSize);
    Context->SeWriteBuffer                  = ExAllocatePoolWithTag(NonPagedPool, Context->SeWriteLength, NLFSE_BUFFER_TAG);
    if(NULL == Context->SeWriteBuffer)
    {
        FltStatus                   = FLT_PREOP_COMPLETE;
        Data->IoStatus.Status       = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information  = 0;
        goto _exit;
    }
    RtlZeroMemory(Context->SeWriteBuffer, Context->SeWriteLength);

    // for IRP operation, we need to build a MDL    
    if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION))
    {
        //  Allocate a MDL for the new allocated memory.  If we fail
        //  the MDL allocation then we won't swap buffer for this operation
        Context->SeWriteMdl = IoAllocateMdl(Context->SeWriteBuffer, Context->SeWriteLength, FALSE, FALSE, NULL);
        if(NULL == Context->SeWriteMdl)
        {
            Data->IoStatus.Status      = STATUS_INSUFFICIENT_RESOURCES;
            Data->IoStatus.Information = 0;
            FltStatus = FLT_PREOP_COMPLETE;
            goto _exit;
        }
        
        //  setup the MDL for the non-paged pool we just allocated
        MmBuildMdlForNonPagedPool(Context->SeWriteMdl);
    }

    // Get input write buffer
    if (Data->Iopb->Parameters.Write.MdlAddress != NULL)
        Context->OriginalWriteBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Write.MdlAddress, NormalPagePriority );
    else  //  There was no MDL defined, use the given buffer address.          
        Context->OriginalWriteBuffer = Data->Iopb->Parameters.Write.WriteBuffer;
    if(NULL == Context->OriginalWriteBuffer)
    {
        Data->IoStatus.Status      = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information = 0;
        FltStatus = FLT_PREOP_COMPLETE;
        goto _exit;
    }

    // Good, try to read data from file
    try
    {
        RtlCopyMemory(Context->SeWriteBuffer, Context->OriginalWriteBuffer, Context->OriginalWriteLength);
        encrypt_buffer(Context->StmContext->encryptExt->key,
            NLE_KEY_LENGTH_IN_BYTES,
            (Context->OriginalWriteOffset.QuadPart - NLSE_ENVELOPE_SIZE),   // Use the offset in RAW data
            Context->SeWriteBuffer,
            Context->SeWriteLength);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Data->IoStatus.Status      = GetExceptionCode();
        Data->IoStatus.Information = 0;
        FltStatus                  = FLT_PREOP_COMPLETE;
        goto _exit;
    }

    // Okay, we get the new buffer, pass it to post
    Data->Iopb->Parameters.Write.WriteBuffer        = Context->SeWriteBuffer;
    Data->Iopb->Parameters.Write.MdlAddress         = Context->SeWriteMdl;
    Data->Iopb->Parameters.Write.ByteOffset.QuadPart= Context->SeWriteOffset.QuadPart;
    Data->Iopb->Parameters.Write.Length             = Context->SeWriteLength;
    FltSetCallbackDataDirty(Data);
    
    // use post callback
    *CompletionContext = Context; Context = NULL;
    FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    if(Context)
    {
        if(NULL != Context->StmContext)    FltReleaseContext(Context->StmContext);  Context->StmContext    = NULL;
        if(NULL != Context->SeWriteBuffer) ExFreePool(Context->SeWriteBuffer);      Context->SeWriteBuffer = NULL;
        if(NULL != Context->SeWriteMdl)    IoFreeMdl(Context->SeWriteMdl);          Context->SeWriteMdl    = NULL;
        ExFreePool(Context);
    }
    return FltStatus;
} /*--NLFSEOpCallbackPreWrite--*/


FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostWrite(
                         __inout PFLT_CALLBACK_DATA Data,
                         __in PCFLT_RELATED_OBJECTS FltObjects,
                         __in PVOID CompletionContext,
                         __in FLT_POST_OPERATION_FLAGS Flags
                         )
/*++
Routine Description:
Arguments:
Return Value:
--*/
{
    FLT_POSTOP_CALLBACK_STATUS  FltStatus   = FLT_POSTOP_FINISHED_PROCESSING;
    PNLSE_WRITE_CONTEXT         Context     = (PNLSE_WRITE_CONTEXT)CompletionContext;
    ULONG                       BytesWritten= (ULONG)Data->IoStatus.Information;

    UNREFERENCED_PARAMETER( Flags );

    // Write fail or Zero byte is written, don't handle it
    if(!NT_SUCCESS(Data->IoStatus.Status) || 0==BytesWritten)
        goto _exit;

    // It is a cached I/O, we have to purge cache
    if(!NLSE_IS_NONCACHE_OR_PAGING_IO(Data))
    {
        if(KeGetCurrentIrql() <= APC_LEVEL)
        {
            NLSEPurgeFileCache(FltObjects->FileObject, TRUE, NULL, 0);
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[NLSE] Fail to clean cache in post write. Current IRQL is %d\n", KeGetCurrentIrql());
        }
        goto _exit;
    }

    // Non-cached or Paging WRITE succeed, re-calculate the padding in Safe Routien
    // The context cannot be NULL
    if(NULL == Context)
        goto _exit;

    FltStatus = NLFSEOpCallbackPostWriteWhenSafe(Data, FltObjects, CompletionContext, Flags);

_exit:
    if(Context)
    {
        if(NULL != Context->StmContext)    FltReleaseContext(Context->StmContext); Context->StmContext   = NULL;
        if(NULL != Context->SeWriteBuffer) ExFreePool(Context->SeWriteBuffer);     Context->SeWriteBuffer= NULL;
        ExFreePool(Context); Context = NULL;
    }
    return FltStatus;
}/*--NLFSEOpCallbackPostWrite--*/

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreWriteBuffered(
                                __inout PFLT_CALLBACK_DATA Data,
                                __in PCFLT_RELATED_OBJECTS FltObjects,
                                __in_opt PNLFSE_STREAM_CONTEXT StmContext
                                )
{
    FLT_PREOP_CALLBACK_STATUS FltStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                  Status    = STATUS_SUCCESS;
    LARGE_INTEGER             FileSizePreWrite   = {0};
    LARGE_INTEGER             FileSizePostWrite  = {0};

    if(NULL == StmContext)
        goto _exit;

    // If new file size is bigger than old file size
    // 1. Reset file size
    // 2. Purge cache
    FileSizePostWrite.QuadPart = Data->Iopb->Parameters.Write.ByteOffset.QuadPart + NLSE_ENVELOPE_SIZE + Data->Iopb->Parameters.Write.Length;
    Status = NLSEGetFileSizeSync(FltObjects->Instance, FltObjects->FileObject, &FileSizePreWrite);
    if(NT_SUCCESS(Status) && (FileSizePostWrite.QuadPart > FileSizePreWrite.QuadPart))
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Prepare to write file (BUFFERED)\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       ToWrite Offset: %d\n", Data->Iopb->Parameters.Write.ByteOffset.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       ToWrite Length: %d\n", Data->Iopb->Parameters.Write.Length);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Current File Length: %d\n", FileSizePreWrite.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Reset File Length:   %d\n", FileSizePostWrite.QuadPart);
        // File size is not big enough, we have to extend it
        Status = NLSESetFileSizeSync(FltObjects->Instance, FltObjects->FileObject, &FileSizePostWrite);
        if(!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"       Fail to reset file size. (0x%08x)\n", Status);
        }
#ifdef _DEBUG
        Status = NLSEGetFileSizeSync(FltObjects->Instance, FltObjects->FileObject, &FileSizePostWrite);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       FileSize Reset:      %d\n", FileSizePostWrite.QuadPart);
#endif

        // After reset file size, the content and padding is changed, we need to update it
        // a. read original data

        // We need to clean cache at post buffered write
        FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

_exit:
    return FltStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostWriteWhenSafe (
                                  __inout PFLT_CALLBACK_DATA Data,
                                  __in PCFLT_RELATED_OBJECTS FltObjects,
                                  __in PVOID CompletionContext,
                                  __in FLT_POST_OPERATION_FLAGS Flags
                                  )
{
    NTSTATUS                    Status    = STATUS_SUCCESS;
    FLT_POSTOP_CALLBACK_STATUS  FltStatus = FLT_POSTOP_FINISHED_PROCESSING;
    PNLSE_WRITE_CONTEXT         Context   = (PNLSE_WRITE_CONTEXT)CompletionContext;
    ULONG                       BytesWritten        = (ULONG)Data->IoStatus.Information;    
    ULONG                       BytesDecrypt        = ROUND_TO_SIZE(BytesWritten, nlfseGlobal.cbcBlockSize);
    LARGE_INTEGER               FileSize            = {0};
    LARGE_INTEGER               LastBlock           = {0};
    UCHAR                       NewAesPadData[16]   = {0};
    ULONG                       NewAesPadLength     = 0;
    KIRQL                       OldIRQL;

    ASSERT(NLSE_IS_NONCACHE_OR_PAGING_IO(Data));

    // Get file size (RAW data size), exclude the envelope size
    if((Context->SeWriteOffset.QuadPart + BytesWritten) > Context->FileSizePreWrite.QuadPart)
        FileSize.QuadPart = Context->SeWriteOffset.QuadPart + BytesWritten - NLSE_ENVELOPE_SIZE;
    else
        FileSize.QuadPart = Context->FileSizePreWrite.QuadPart - NLSE_ENVELOPE_SIZE;

    // Get last block's offset (exclude the envelope size)
    if(0 == (Context->FileSizePreWrite.QuadPart-NLSE_ENVELOPE_SIZE))
        LastBlock.QuadPart = 0;
    else
        LastBlock.QuadPart = ROUND_TO_SIZE((Context->FileSizePreWrite.QuadPart-NLSE_ENVELOPE_SIZE), nlfseGlobal.cryptoBlockSize) - nlfseGlobal.cryptoBlockSize;

    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE::Write]\n");
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    WriteOffset:      %d\n", (Context->SeWriteOffset.QuadPart - NLSE_ENVELOPE_SIZE));
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    BytesToWrite:     %d\n", Context->SeWriteLength);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    BytesWritten:     %d\n", BytesWritten);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    FileSize:         %d\n", FileSize.QuadPart);

    // If the last crypto block is changed, we need to re-calculate the padding
    if((Context->SeWriteOffset.QuadPart - NLSE_ENVELOPE_SIZE + BytesWritten) > LastBlock.QuadPart)
    {
        NewAesPadLength = BytesDecrypt - BytesWritten;
        if(NewAesPadLength>0) RtlCopyMemory(NewAesPadData, Context->SeWriteBuffer+BytesWritten, NewAesPadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    BytesWritten:     %d\n", BytesWritten);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding Length:   %d\n", NewAesPadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding Data:     0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
            NewAesPadData[0], NewAesPadData[1], NewAesPadData[2], NewAesPadData[3],
            NewAesPadData[4], NewAesPadData[5], NewAesPadData[6], NewAesPadData[7]);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"                      0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
            NewAesPadData[8],  NewAesPadData[9],  NewAesPadData[10], NewAesPadData[11],
            NewAesPadData[12], NewAesPadData[13], NewAesPadData[14], NewAesPadData[15]);

        // Update encrypt extension        
        KeAcquireSpinLock(&Context->StmContext->encryptExtLock, &OldIRQL);
        Context->StmContext->FileSize.QuadPart          = FileSize.QuadPart;
        Context->StmContext->encryptExt->fileRealLength = FileSize.QuadPart;
        Context->StmContext->encryptExt->paddingLen     = NewAesPadLength;
        RtlCopyMemory(Context->StmContext->encryptExt->paddingData, NewAesPadData, 16);
        KeReleaseSpinLock(&Context->StmContext->encryptExtLock, OldIRQL);

        // Set the dirty flag
        Context->StmContext->encryptExtDirty = TRUE;

        // Try to flush stream context to disk if IRQL is <= APC_LEVEL
        if(KeGetCurrentIrql() <= APC_LEVEL)
        {
            NLSEFlushStreamContext(FltObjects->Instance, FltObjects->FileObject, NLSE_IS_PAGING_IO(Data), Context->StmContext);
        }
    }

    return FltStatus;
}


typedef struct _NLSE_DIRCONTROL_CONTEXT
{
    PFLT_CALLBACK_DATA  Data;
    PFLT_FILTER         Filter;
    PFLT_INSTANCE       Instance;
    PFILE_OBJECT        FileObject;
    UNICODE_STRING      Name;
    BOOLEAN             ProgramDir;
    PNLDINFOLIST        DList;
} NLSE_DIRCONTROL_CONTEXT, *PNLSE_DIRCONTROL_CONTEXT;


FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreDirectoryCtrl (
                                 __inout PFLT_CALLBACK_DATA Data,
                                 __in PCFLT_RELATED_OBJECTS FltObjects,
                                 __deref_out PVOID *CompletionContext
                                 )
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PFLT_FILE_NAME_INFORMATION  pfni        = NULL;
    BOOLEAN                     Root        = FALSE;
    PNLSE_DIRCONTROL_CONTEXT    Context     = NULL;
    BOOLEAN                     FromSystem  = FALSE;
    PNLDINFOLIST                DList       = NULL;
    NLFSE_PVOLUME_CONTEXT       VolCtx      = NULL;
    ULONG                       DirFlags    = 0;
    WCHAR                       DriveLetter = 0;
    BOOLEAN                     IsNonDRM    = FALSE;

    // If it is not enabled, don't perform the callback
    if(!nlfseGlobal.bEnable)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolCtx);
    if(!NT_SUCCESS(Status) || NULL==VolCtx)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    DriveLetter = VolCtx->DosName.Buffer[0];
    DList = &VolCtx->DInfoList;
    FltReleaseContext(VolCtx); VolCtx = NULL;


    // Is the request from SYSTEM
    FromSystem = (g_SystemProcessId == FltGetRequestorProcessId(Data))?TRUE:FALSE;

    Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &pfni);
    if(!NT_SUCCESS(Status) || NULL==pfni)
        goto _exit;

    Status = FltParseFileNameInformation(pfni);
    if(!NT_SUCCESS(Status))
        goto _exit;

    Status = NxIsNonDrmDirectoryEx(DriveLetter, pfni, &IsNonDRM);
    if(NT_SUCCESS(Status) && IsNonDRM)
    {
        // This directory is (or is under) a NON-DRM directory
        // Then we don't need to check it
        goto _exit;
    }

    // Don't Handle NetWorkShare
    if(pfni->Share.Length > 0) goto _exit;
    // Don't Handle Stream -- it should be directory
    if(pfni->Stream.Length > 0) goto _exit;

    // Is NT reserved file?
    if(NLSEIsReservedFileName(&pfni->ParentDir, &pfni->FinalComponent))
        goto _exit;    

    // Get dir flags
    DirFlags = NLCheckDiretory(&pfni->ParentDir, &pfni->FinalComponent);
    if(NL_IGNORE_DIRECTORY & DirFlags)
        goto _exit;

    // Root directory
    if(pfni->ParentDir.Length==2 &&  pfni->ParentDir.Buffer[0]==L'\\' && 0==pfni->FinalComponent.Length)
        Root = TRUE;


    // Allocate p2p context
    Context = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_DIRCONTROL_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == Context)
        goto _exit;

    // Build directory name
    RtlZeroMemory(Context, sizeof(NLSE_DIRCONTROL_CONTEXT));
    Context->Name.Length = pfni->Name.Length;
    Context->Name.MaximumLength = Context->Name.Length + sizeof(WCHAR);
    Context->Name.Buffer = ExAllocatePoolWithTag(NonPagedPool, Context->Name.MaximumLength, NLFSE_BUFFER_TAG);
    if(NULL == Context->Name.Buffer)
        goto _exit;
    RtlZeroMemory(Context->Name.Buffer, Context->Name.MaximumLength);
    RtlCopyMemory(Context->Name.Buffer, pfni->Name.Buffer, pfni->Name.Length);


    // We need to check it at Post directory Control
    Context->Data       = Data;
    Context->Instance   = FltObjects->Instance;
    Context->Filter     = FltObjects->Filter;
    Context->FileObject = FltObjects->FileObject;
    Context->DList      = DList;
    Context->ProgramDir = FlagOn(DirFlags, NL_PROGRAM_DIRECTORY)?TRUE:FALSE;
    *CompletionContext  = Context; Context = NULL;
    FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

_exit:
    if(NULL != Context)
    {
        if(Context->Name.Buffer) ExFreePool(Context->Name.Buffer); Context->Name.Buffer=NULL;
        ExFreePool(Context); Context=NULL;
    }
    if(NULL!=pfni) FltReleaseFileNameInformation(pfni); pfni = NULL;
    return FltStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostDirectoryCtrl (
                                  __inout PFLT_CALLBACK_DATA Data,
                                  __in PCFLT_RELATED_OBJECTS FltObjects,
                                  __in PVOID CompletionContext,
                                  __in FLT_POST_OPERATION_FLAGS Flags
                                  )
{
    NTSTATUS                    Status          = STATUS_SUCCESS;
    FLT_POSTOP_CALLBACK_STATUS  FltStatus       = FLT_POSTOP_FINISHED_PROCESSING;
    FILE_INFORMATION_CLASS      FileInfoClass   = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
    PVOID                       DirectoryBuffer = NULL;
    PNLSE_DIRCONTROL_CONTEXT    Context         = (PNLSE_DIRCONTROL_CONTEXT)CompletionContext;
    PFLT_GENERIC_WORKITEM       WorkItem        = NULL;

    // Sanity check
    if(NULL==Context || NULL==Context->Data || 0==Context->Name.Length) goto _exit;
    if(!NT_SUCCESS(Data->IoStatus.Status) || (Data->IoStatus.Information == 0)) goto _exit;

    //  Queue a work item 
    WorkItem = FltAllocateGenericWorkItem();
    if(NULL == WorkItem)
        goto _exit;

    // Good, we need to queue a work item and pending this IRP
    Status = FltQueueGenericWorkItem(WorkItem, (PVOID)FltObjects->Filter, NLSEDirectoryControlRoutine, CriticalWorkQueue, (PVOID)Context);
    if(NT_SUCCESS(Status))
    {
        FltStatus = FLT_POSTOP_MORE_PROCESSING_REQUIRED;
        WorkItem  = NULL; // Don't free work item
        Context   = NULL; // Don't free context
    }

_exit:
    if(NULL != Context)
    {
        if(Context->Name.Buffer) ExFreePool(Context->Name.Buffer); Context->Name.Buffer=NULL;
        ExFreePool(Context); Context=NULL;
    }
    if(NULL!=WorkItem) FltFreeGenericWorkItem(WorkItem); WorkItem = NULL;
    return FltStatus;
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreCleanup (
                           __inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __deref_out PVOID *CompletionContext
                           )
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;
    PNLSE_WRITE_CONTEXT         Context     = NULL;
    LARGE_INTEGER               FileSizePreWrite = {0};

#ifdef NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc;
	PfStart(
		&pfc
		);
#endif

    // No stream context?
	Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext );
    if(!NT_SUCCESS(Status) || NULL == StmContext)
        goto _exit;

    // Is the context dirty? try to flush it to disk
    NLSEFlushStreamContext(FltObjects->Instance, FltObjects->FileObject, NLSE_IS_PAGING_IO(Data), StmContext);

_exit:

#ifdef NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc
		);
	if (pfc.diff.QuadPart / 1000)
		KdPrint(("pre-cleanup elasped time = %I64d milliseconds\n", pfc.diff.QuadPart / 1000));
#endif

    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    return FltStatus;
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreFlushBuffers (
                                __inout PFLT_CALLBACK_DATA Data,
                                __in PCFLT_RELATED_OBJECTS FltObjects,
                                __deref_out PVOID *CompletionContext
                                )
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                    Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;
    PNLSE_WRITE_CONTEXT         Context     = NULL;
    LARGE_INTEGER               FileSizePreWrite = {0};

    // No stream context?
	Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext );
    if(!NT_SUCCESS(Status) || NULL == StmContext)
        goto _exit;

    // Is the context dirty? try to flush it to disk
    NLSEFlushStreamContext(FltObjects->Instance, FltObjects->FileObject, NLSE_IS_PAGING_IO(Data), StmContext);

_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    return FltStatus;
}

static
BOOLEAN
NLSEStrStartWith(
                 __in PUNICODE_STRING Source,
                 __in PUNICODE_STRING SubStr,
                 __in BOOLEAN  CaseInSensitive
                 )
{
    UNICODE_STRING  SourceHeader = {0, 0, NULL};

    if(Source->Length < SubStr->Length)
        return FALSE;
    SourceHeader.Length        = SubStr->Length;
    SourceHeader.MaximumLength = SubStr->Length;
    SourceHeader.Buffer        = Source->Buffer;
    if(0 == RtlCompareUnicodeString(&SourceHeader, SubStr, CaseInSensitive))
        return TRUE;

    return FALSE;
}

static
BOOLEAN
NLSEStrStartWithCch(
                    __in PUNICODE_STRING Source,
                    __in_z const WCHAR* SubCch,
                    __in BOOLEAN  CaseInSensitive
                    )
{
    UNICODE_STRING  SubStr       = {0, 0, NULL};

    SubStr.Buffer = (WCHAR*)SubCch;
    SubStr.Length = wcslen(SubCch)*sizeof(WCHAR);
    SubStr.MaximumLength = SubStr.Length + sizeof(WCHAR);
    return NLSEStrStartWith(Source, &SubStr, CaseInSensitive);
}

static
BOOLEAN
NLSEStrEndWith(
               __in PUNICODE_STRING Source,
               __in PUNICODE_STRING SubStr,
               __in BOOLEAN  CaseInSensitive
               )
{
    UNICODE_STRING  SourceTail = {0, 0, NULL};

    if(Source->Length < SubStr->Length)
        return FALSE;
    SourceTail.Length        = SubStr->Length;
    SourceTail.MaximumLength = SubStr->Length;
    SourceTail.Buffer        = Source->Buffer + (Source->Length - SubStr->Length)/sizeof(WCHAR);
    if(0 == RtlCompareUnicodeString(&SourceTail, SubStr, CaseInSensitive))
        return TRUE;

    return FALSE;
}

static
BOOLEAN
NLSEStrEndWithCch(
                  __in PUNICODE_STRING Source,
                  __in_z const WCHAR* SubCch,
                  __in BOOLEAN  CaseInSensitive
                  )
{
    UNICODE_STRING  SubStr       = {0, 0, NULL};

    SubStr.Buffer = (WCHAR*)SubCch;
    SubStr.Length = wcslen(SubCch)*sizeof(WCHAR);
    SubStr.MaximumLength = SubStr.Length + sizeof(WCHAR);
    return NLSEStrEndWith(Source, &SubStr, CaseInSensitive);
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreNwQueryOpen (
                               __inout PFLT_CALLBACK_DATA Data,
                               __in PCFLT_RELATED_OBJECTS FltObjects,
                               __deref_out PVOID *CompletionContext
                               )
{
    // Disable NetworkQueryOpen
    Data->IoStatus.Information = 0;
    return FLT_PREOP_DISALLOW_FASTIO;
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreQueryInfo (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             )
{
    NTSTATUS                    Status      = STATUS_SUCCESS;
    FLT_PREOP_CALLBACK_STATUS   FltStatus   = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PNLFSE_STREAM_CONTEXT       StmContext  = NULL;
    BOOLEAN                     Encrypted   = FALSE;
    
		Status = FltGetStreamContext( FltObjects->Instance, FltObjects->FileObject, &StmContext );
		if(!NT_SUCCESS(Status) || NULL == StmContext)
			goto _exit; // Fail to get stream context?

    FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
_exit:
    if(StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    return FltStatus;
} /*--NLFSEOpCallbackPreQueryInfo--*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostQueryInfo (
                              __inout PFLT_CALLBACK_DATA Data,
                              __in PCFLT_RELATED_OBJECTS FltObjects,
                              __in PVOID CompletionContext,
                              __in FLT_POST_OPERATION_FLAGS Flags
                              )
{
    // Sanity check
    ASSERT(Data->Iopb->MajorFunction == IRP_MJ_QUERY_INFORMATION);

    // Adjust file size: we need to substract extra header size
    NLSEAdjustFileSizeInPostQueryInformation(Data);

    return FLT_POSTOP_FINISHED_PROCESSING;
} /*--NLFSEOpCallbackPostQueryInfo--*/


typedef struct _NLSE_SETINFO_CONTEXT
{
    PFLT_CALLBACK_DATA      Data;
    PFLT_FILTER             Filter;
    PFLT_INSTANCE           Instance;
    PFILE_OBJECT            FileObject;
    PNLFSE_STREAM_CONTEXT   StmContext;
    LARGE_INTEGER           FileSizePreSet;
    LARGE_INTEGER           FileSizePostSet;
    ULONG                   SectorSize;
    ULONG                   AesPadLength;
    UCHAR                   AesPadData[16];
    UNICODE_STRING          TargetName;
} NLSE_SETINFO_CONTEXT, *PNLSE_SETINFO_CONTEXT;

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreSetInfo (__inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __deref_out_opt PVOID *CompletionContext
                           )
/*++
Return Value:
Identifies how processing should continue for this operation.
If this routine returns FLT_PREOP_COMPLETE, it must set the callback data 
structure's IoStatus.Status field to the final NTSTATUS value for the 
I/O operation. 
This NTSTATUS value cannot be STATUS_PENDING. For a cleanup or close 
operation, it must be a success NTSTATUS value other than STATUS_PENDING 
because cleanup and close operations cannot fail. 
--*/
{
    FLT_PREOP_CALLBACK_STATUS   FltStatus       = FLT_PREOP_SUCCESS_NO_CALLBACK; 
    NTSTATUS                    Status          = STATUS_SUCCESS;
    NLFSE_PVOLUME_CONTEXT       VolContext      = NULL;
    PNLFSE_STREAM_CONTEXT       StmContext      = NULL;
    PNLSE_SETINFO_CONTEXT       Context         = NULL;
    PFLT_FILE_NAME_INFORMATION  fni             = NULL;
    BOOLEAN                     Trusted         = FALSE;
    BOOLEAN                     DRMDirectory    = FALSE;

    PFILE_ALLOCATION_INFORMATION        fai = NULL;
    PFILE_END_OF_FILE_INFORMATION       feof= NULL;
    PFILE_POSITION_INFORMATION          fpi = NULL;
    PFILE_VALID_DATA_LENGTH_INFORMATION fvli= NULL;
    PFILE_RENAME_INFORMATION            fri = NULL;

    // If it is not enabled, don't perform the callback
    if(!nlfseGlobal.bEnable)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;    

    Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolContext);
    if(!NT_SUCCESS(Status) || NULL==VolContext)
    {
        VolContext = NULL;
        goto _exit;
    }

    // Try to find stream context
	Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext);
    if(!NT_SUCCESS(Status)) StmContext = NULL;

    // If it is not rename action, the stream context must not be NULL
    if(NULL == StmContext && FileRenameInformation != Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
        goto _exit;

    // Allocate pre to post context, and Initialize it
    Context = (PNLSE_SETINFO_CONTEXT)ExAllocatePoolWithTag(NonPagedPool, sizeof(NLSE_SETINFO_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == Context)
    {
        Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        FltStatus = FLT_PREOP_COMPLETE;
        goto _exit;
    }
    RtlZeroMemory(Context, sizeof(NLSE_SETINFO_CONTEXT));
    Context->StmContext     = StmContext; StmContext = NULL;
    Context->TargetName.Length = 0;
    Context->TargetName.Buffer = NULL;
    Context->SectorSize = VolContext->SectorSize;
    Context->Data       = Data;
    Context->Filter     = FltObjects->Filter;
    Context->Instance   = FltObjects->Instance;
    Context->FileObject = FltObjects->FileObject;

    // Is this a RENAME request?
    if(FileRenameInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // This is a special case, we need to handle it seperately
        fri = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
        Status=FltGetDestinationFileNameInformation(FltObjects->Instance,
			    FltObjects->FileObject,
			    fri->RootDirectory,
			    fri->FileName,
			    fri->FileNameLength,
			    FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT,
			    &fni);
        if(!NT_SUCCESS(Status))
            goto _exit;

        // Is it encrypted directory?
        Status = NLSEIsPathEncryptedEx(FltObjects, &fni->Name, &VolContext->Name, &VolContext->DosName, &DRMDirectory);
        if(!NT_SUCCESS(Status)) DRMDirectory=FALSE;

        // Move Nonencrypted file to non-DRM directory
        // Don't handle it
        if(NULL==Context->StmContext && !DRMDirectory)
            goto _exit;

        // Is it trusted process?
        Trusted = NT_SUCCESS(NLSEUpdateCurrentPCKey(FltGetRequestorProcessId(Data), FALSE))?TRUE:FALSE;

        if(!Trusted)
        {
            // Untrusted process cannot rename encrypted file or move any file to DRM directory
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            FltStatus = FLT_PREOP_COMPLETE;
        }
        else
        {
            // Generate target file name
            Context->TargetName.Buffer = ExAllocatePoolWithTag(NonPagedPool, fni->Name.Length+sizeof(WCHAR), NLFSE_BUFFER_TAG);
            if(NULL == Context->TargetName.Buffer)
            {
                // Fail to allocate buffer, don't allow this reanme action
                Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                FltStatus = FLT_PREOP_COMPLETE;
                goto _exit;
            }

            // We need to check it at Post set information
            Context->TargetName.Length = fni->Name.Length;
            Context->TargetName.MaximumLength = fni->Name.Length+sizeof(WCHAR);
            RtlCopyUnicodeString(&Context->TargetName, &fni->Name);
            *CompletionContext = Context; Context = NULL;
            FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        goto _exit;
    }

    Status = NLSEGetFileSizeSync(FltObjects->Instance, FltObjects->FileObject, &Context->FileSizePreSet);
    if(!NT_SUCCESS(Status)) Context->FileSizePreSet.QuadPart = Context->StmContext->FileSize.QuadPart + NLSE_ENVELOPE_SIZE;
    Context->FileSizePostSet.QuadPart = Context->FileSizePreSet.QuadPart;
    // Initialize Context Padding with current padding informaiton in Stream Context
    Context->AesPadLength   = Context->StmContext->encryptExt->paddingLen;
    RtlCopyMemory(Context->AesPadData, Context->StmContext->encryptExt->paddingData, 16);

    // Set EOF?
    if(FileEndOfFileInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // Is the context dirty? try to flush it to disk
        NLSEFlushStreamContext(FltObjects->Instance, FltObjects->FileObject, NLSE_IS_PAGING_IO(Data), Context->StmContext);

        feof = (PFILE_END_OF_FILE_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
        Context->FileSizePostSet.QuadPart  = feof->EndOfFile.QuadPart;

        // Don't handle PAGING I/O
        // Because the file size has been changed in Buffered Write
        if(!(Data->Iopb->IrpFlags & (IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO)))
        {
            feof->EndOfFile.QuadPart += NLSE_ENVELOPE_SIZE;
            Context->FileSizePostSet.QuadPart  = feof->EndOfFile.QuadPart;
            FltSetCallbackDataDirty( Data );

            if(Context->FileSizePreSet.QuadPart != Context->FileSizePostSet.QuadPart)
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Set EOF\n");
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Name:         %wZ\n", &Context->StmContext->FileName);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Is NonCache:        %d\n", FlagOn(Data->Iopb->IrpFlags, IRP_NOCACHE)?1:0);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Is IRP:             %d\n", FLT_IS_IRP_OPERATION(Data)?1:0);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       FileSize PreSet:    %d\n", Context->FileSizePreSet.QuadPart);
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       FileSize PostSet:   %d\n", Context->FileSizePostSet.QuadPart);
            }
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Set EOF (Paging I/O)\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Name:         %wZ\n", &Context->StmContext->FileName);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Is NonCache:        %d\n", FlagOn(Data->Iopb->IrpFlags, IRP_NOCACHE)?1:0);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Is IRP:             %d\n", FLT_IS_IRP_OPERATION(Data)?1:0);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       FileSize PreSet:    %d\n", Context->FileSizePreSet.QuadPart);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       FileSize PostSet:   %d\n", Context->FileSizePostSet.QuadPart);
            if(Context->FileSizePreSet.QuadPart != Context->FileSizePostSet.QuadPart)
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       NOT Equal:          TRUE\n");

            // Don't handle paging I/O
            goto _exit;
        }

        // File size is not changed?
        if(Context->FileSizePreSet.QuadPart==Context->FileSizePostSet.QuadPart)
            goto _exit;

        // File shrink?
        if(Context->FileSizePostSet.QuadPart < Context->FileSizePreSet.QuadPart)
        {
            Status = NLSECalculateAesPadWhenFileShrink(FltObjects->Instance, FltObjects->FileObject, Context->StmContext->SectorSize, &Context->FileSizePreSet, &Context->FileSizePostSet, &Context->AesPadLength, Context->AesPadData);
            if(!NT_SUCCESS(Status))
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"       * Fail to get Aes Padding (0x%08x)\n", Status);
                goto _exit;
            }
        }
        else
        {
            ; // File growth, we need to encrypt the new data (All Zero Data) on disk at post set information
              // And also calculate padding, update header
        }

        // We need to check it at Post set information
        *CompletionContext = Context; Context = NULL;
        FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }
    else if(FileAllocationInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // Reset allocate size
        fai = (PFILE_ALLOCATION_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
        fai->AllocationSize.QuadPart += NLSE_ENVELOPE_SIZE;
        fai->AllocationSize.QuadPart = ROUND_TO_SIZE(fai->AllocationSize.QuadPart, 4096); // Round to page size
        FltSetCallbackDataDirty( Data );

        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Set Allocate\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Allocate PreSet:    %d\n", fai->AllocationSize.QuadPart-NLSE_ENVELOPE_SIZE);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Allocate PostSet:   %d\n", fai->AllocationSize.QuadPart);
    }
    else if(FileDispositionInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // Delete File
        // We need to change stream context at Post set information
        *CompletionContext = Context; Context = NULL;
        FltStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }
    else if(FilePositionInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        ; // We don't need to change current offset
    }
    else if(FileValidDataLengthInformation == Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // Reset valid data length
        fvli = (PFILE_VALID_DATA_LENGTH_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
        fvli->ValidDataLength.QuadPart += NLSE_ENVELOPE_SIZE;
        FltSetCallbackDataDirty( Data );

        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Set Valid Length\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Valid PreSet:    %d\n", fvli->ValidDataLength.QuadPart-NLSE_ENVELOPE_SIZE);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Valid PostSet:   %d\n", fvli->ValidDataLength.QuadPart);
    }
    else
    {
        ; // Nothing needs to be changed
    }

_exit:
    if(fni) FltReleaseFileNameInformation(fni); fni = NULL;
    if(VolContext) FltReleaseContext(VolContext); VolContext=NULL;
    if(NULL != StmContext) FltReleaseContext(StmContext); StmContext = NULL;
    if(NULL != Context)
    {
        if(Context->TargetName.Buffer) ExFreePool(Context->TargetName.Buffer); Context->TargetName.Buffer=NULL; Context->TargetName.Length=0;
        if(Context->StmContext) FltReleaseContext(Context->StmContext); Context->StmContext = NULL;
        ExFreePool(Context); Context=NULL;
    }
    return FltStatus;
} /*--NLFSEOpCallbackPreSetInfo--*/


FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetInfo (__inout PFLT_CALLBACK_DATA Data,
                            __in PCFLT_RELATED_OBJECTS FltObjects,
                            __in PVOID CompletionContext,
                            __in FLT_POST_OPERATION_FLAGS Flags)
{
    FLT_POSTOP_CALLBACK_STATUS  FltStatus           = FLT_POSTOP_FINISHED_PROCESSING;
    NTSTATUS                    Status              = STATUS_SUCCESS;
    PNLSE_SETINFO_CONTEXT       Context             = (PNLSE_SETINFO_CONTEXT)CompletionContext;
    FILE_INFORMATION_CLASS      InformationClass    = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
    PFLT_GENERIC_WORKITEM       WorkItem            = NULL;

    if(!NT_SUCCESS(Data->IoStatus.Status))
        goto _exit;

    ASSERT((FileEndOfFileInformation==InformationClass) || (FileRenameInformation==InformationClass) || (FileDispositionInformation==InformationClass));

    // For Disposition, we just need to set the delete flag
    if(FileDispositionInformation == InformationClass)
    {
        Context->StmContext->bDelete = TRUE;
        goto _exit;
    }
    
    // For Rename
    if(FileRenameInformation==InformationClass)
    {
        if(NULL != Context->StmContext)
        {
            // Source file has already been encrypted, we just need to change file name
            if(Context->StmContext->FileName.Buffer) ExFreePool(Context->StmContext->FileName.Buffer);
            Context->StmContext->FileName.Buffer = Context->TargetName.Buffer;
            Context->StmContext->FileName.Length = Context->TargetName.Length;
            Context->StmContext->FileName.MaximumLength = Context->TargetName.MaximumLength;
            Context->TargetName.Buffer=NULL; Context->TargetName.Length=0; Context->TargetName.MaximumLength=0;
        }
        else
        {
            // Source is not a encrypted file, but destination is in DRM directory

            // No need to encrypt the file if it's wrapped.
            BOOLEAN Wrapped;

            NLSEIsEncryptedOrWrappedFile2(FltObjects->Instance, FltObjects->FileObject, NULL, &Wrapped);
            if(Wrapped)
            {
                goto _exit;
            }

            // We need to post this action to system worker thread, and encrypt the file
            //  Queue a work item 
            WorkItem = FltAllocateGenericWorkItem();
            if(NULL == WorkItem)
                goto _exit;

            // Good, we need to queue a work item and pending this IRP
            Status = FltQueueGenericWorkItem(WorkItem, (PVOID)FltObjects->Filter, NLSERenameWorkRoutine, CriticalWorkQueue, (PVOID)Context);
            if(NT_SUCCESS(Status))
            {
                FltStatus = FLT_POSTOP_MORE_PROCESSING_REQUIRED;
                WorkItem  = NULL; // Don't free work item
                Context   = NULL; // Don't free context
            }
        }

        // Done
        goto _exit;
    }

    // For set file size, we need to do it in safe routine
    if(FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, NLFSEOpCallbackPostSetInfoWhenSafe, &FltStatus))
    {
        Context = NULL;
    }
    else
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[nlSE::PostSetInfo] Fail to issue a safe routine\n");
    }

_exit:
    if(NULL!=WorkItem) FltFreeGenericWorkItem(WorkItem); WorkItem = NULL;
    if(NULL != Context)
    {
        if(Context->TargetName.Buffer) ExFreePool(Context->TargetName.Buffer); Context->TargetName.Buffer=NULL; Context->TargetName.Length=0;
        if(Context->StmContext) FltReleaseContext(Context->StmContext); Context->StmContext = NULL;
        ExFreePool(Context); Context=NULL;
    }
    return FltStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetInfoWhenSafe (__inout PFLT_CALLBACK_DATA Data,
                                    __in PCFLT_RELATED_OBJECTS FltObjects,
                                    __in PVOID CompletionContext,
                                    __in FLT_POST_OPERATION_FLAGS Flags)
{
    FLT_POSTOP_CALLBACK_STATUS  FltStatus           = FLT_POSTOP_FINISHED_PROCESSING;
    NTSTATUS                    Status              = STATUS_SUCCESS;
    PNLSE_SETINFO_CONTEXT       Context             = (PNLSE_SETINFO_CONTEXT)CompletionContext;
    FILE_INFORMATION_CLASS      InformationClass    = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
    KIRQL                       OldIRQL;

    ASSERT((FileEndOfFileInformation==InformationClass) || (FileRenameInformation==InformationClass));

    // For Set FileSize
    if(FileEndOfFileInformation==InformationClass)
    {
        // Shrink
        if(Context->FileSizePreSet.QuadPart > Context->FileSizePostSet.QuadPart)
        {
            ; // the padding has already been got in pre set information
        }
        else if(Context->FileSizePreSet.QuadPart < Context->FileSizePostSet.QuadPart)
        {
            //  Growth
            Status = NLSECalculateAesPadWhenFileGrow(
                FltObjects->Instance,
                FltObjects->FileObject,
                Context->StmContext->SectorSize,
                Context->StmContext->encryptExt->key,
                &Context->FileSizePreSet,
                &Context->FileSizePostSet,
                &Context->AesPadLength,
                Context->AesPadData);
        }

        // We have get new padding information
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding Length:   %d\n", Context->AesPadLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"    Padding Data:     0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
            Context->AesPadData[0], Context->AesPadData[1], Context->AesPadData[2], Context->AesPadData[3],
            Context->AesPadData[4], Context->AesPadData[5], Context->AesPadData[6], Context->AesPadData[7]);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"                      0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
            Context->AesPadData[8],  Context->AesPadData[9],  Context->AesPadData[10], Context->AesPadData[11],
            Context->AesPadData[12], Context->AesPadData[13], Context->AesPadData[14], Context->AesPadData[15]);

        // Update encrypt extension        
        KeAcquireSpinLock(&Context->StmContext->encryptExtLock, &OldIRQL);
        Context->StmContext->FileSize.QuadPart          = Context->FileSizePostSet.QuadPart - NLSE_ENVELOPE_SIZE;
        Context->StmContext->encryptExt->fileRealLength = Context->FileSizePostSet.QuadPart - NLSE_ENVELOPE_SIZE;
        Context->StmContext->encryptExt->paddingLen     = Context->AesPadLength;
        RtlCopyMemory(Context->StmContext->encryptExt->paddingData, Context->AesPadData, 16);
        KeReleaseSpinLock(&Context->StmContext->encryptExtLock, OldIRQL);

        // Write the encryption extension to file
        Context->StmContext->encryptExtDirty = TRUE;
        NLSEFlushStreamContext(FltObjects->Instance, FltObjects->FileObject, NLSE_IS_PAGING_IO(Data), Context->StmContext);
    }

    if(NULL != Context)
    {
        if(Context->TargetName.Buffer) ExFreePool(Context->TargetName.Buffer); Context->TargetName.Buffer=NULL; Context->TargetName.Length=0;
        if(Context->StmContext) FltReleaseContext(Context->StmContext); Context->StmContext = NULL;
        ExFreePool(Context); Context=NULL;
    }
    return FltStatus;
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreFsControl (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             )
{
    FLT_PREOP_CALLBACK_STATUS  FltStatus    = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PFLT_FILE_NAME_INFORMATION NameInfo     = NULL;
    NTSTATUS                   Status       = STATUS_SUCCESS;
    NLFSE_PVOLUME_CONTEXT      VolContext   = NULL;
    PNLFSE_STREAM_CONTEXT      StmContext   = NULL;
    BOOLEAN                    Directory    = FALSE;
    BOOLEAN                    Encrypted    = FALSE;
    UNICODE_STRING             DosName      = {0};

    UNREFERENCED_PARAMETER(CompletionContext);

    // If it is not enabled, don't perform the callback
    if(!nlfseGlobal.bEnable)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    if(IRP_MN_USER_FS_REQUEST != Data->Iopb->MinorFunction)
        goto _exit;

    // Only handle EFS encrypt and Windows compression
    if(FSCTL_SET_ENCRYPTION != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode
        && FSCTL_ENCRYPTION_FSCTL_IO != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode
        && FSCTL_SET_COMPRESSION != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode)
        goto _exit;

    // a. Get our volume context.
    Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &VolContext);
    if (!NT_SUCCESS(Status))
        goto _exit;

    Status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StmContext);
    if (NT_SUCCESS(Status) && NULL!=StmContext)
    {
        // Try to encrypt/compress a NextLabs Encrypted file?
        // return FAKE success
        Data->IoStatus.Status       = STATUS_SUCCESS;
        Data->IoStatus.Information  = 0;
        FltStatus                   = FLT_PREOP_COMPLETE;
        goto _exit;
    }

    // If it is not a directory, don't need to handle it
    Status = NLSEIsDirectory(FltObjects->Instance, FltObjects->FileObject, &Directory);
    if (!NT_SUCCESS(Status) || !Directory)
        goto _exit;


    // We need to check if this is DRM directory
    Status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &NameInfo);
    if (!NT_SUCCESS(Status) || NULL==NameInfo)
        goto _exit;
    Status = FltParseFileNameInformation(NameInfo);
    if(!NT_SUCCESS(Status))
        goto _exit;


    // ignore all streams
    if(NameInfo->Stream.Length) goto _exit;
    // ignore all remote access
    if(NameInfo->Share.Length)  goto _exit;

    // Create Dos Name
    DosName.Length          = VolContext->DosName.Length + NameInfo->ParentDir.Length + NameInfo->FinalComponent.Length;
    DosName.MaximumLength   = DosName.Length + sizeof(WCHAR);
    DosName.Buffer          = ExAllocatePoolWithTag(NonPagedPool, DosName.MaximumLength, NLFSE_BUFFER_TAG);
    if(NULL == DosName.Buffer) goto _exit;

    RtlCopyUnicodeString(&DosName, &VolContext->DosName);
    RtlAppendUnicodeStringToString(&DosName, &NameInfo->ParentDir);
    RtlAppendUnicodeStringToString(&DosName, &NameInfo->FinalComponent);
    Status = NLSEIsPathEncrypted(FltObjects, &DosName, &Encrypted);
    // If fail to get directory or the directory is not DRM directory, don't handle it
    if(!NT_SUCCESS(Status) || Encrypted) goto _exit;

    // Otherwise, we cannot set EFS/Compress flag on a DRM directory
    Data->IoStatus.Status       = STATUS_SUCCESS;
    Data->IoStatus.Information  = 0;
    FltStatus                   = FLT_PREOP_COMPLETE;

_exit:
    if(NULL!=DosName.Buffer) ExFreePool(DosName.Buffer);
    if(NameInfo)            FltReleaseFileNameInformation(NameInfo); NameInfo=NULL;
    if(NULL != VolContext)  FltReleaseContext(VolContext); VolContext = NULL;
    if(NULL != StmContext)  FltReleaseContext(StmContext); StmContext = NULL;
    return FltStatus;
}/*--NLFSEOpCallbackPreFsControl--*/

//////////////////////////////////////////////////////////////////


/******************************************************************************************************************
  Define Local Routines
*******************************************************************************************************************/
VOID
NLSEHandleDirectoryQuery(
                         __in PFLT_FILTER Filter,
                         __in PFLT_INSTANCE Instance,
                         __in PCUNICODE_STRING DirectoryName,
                         __in BOOLEAN ProgramDir,
                         __in FILE_INFORMATION_CLASS FileInfoClass,
                         __in PVOID DirectoryBuffer,
                         __in PFILE_OBJECT DirectoryObject,
                         __in PNLDINFOLIST DList
                         )
{
    PNLDINFO DInfo   = NULL;
    BOOLEAN  NewDInfo= FALSE;
    KIRQL    OldIrql = 0;
    NLPERFCOUNTER Counter;

    if(FileBothDirectoryInformation != FileInfoClass
        && FileDirectoryInformation != FileInfoClass
        && FileFullDirectoryInformation != FileInfoClass
        && FileIdBothDirectoryInformation != FileInfoClass
        && FileIdFullDirectoryInformation != FileInfoClass
        )
        return;
    //
    // Get DInfo
    //
    DInfo   = NLFindInDList(DList, DirectoryName);

    // Allocate new DInfo
    if(NULL == DInfo)
    {
        DInfo = NLCreateDInfo(DirectoryName);
        if(NULL == DInfo)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[NLSE] NLSEHandleDirectoryQuery: Fail to allocate pool for new dir info\n");
            return;
        }
        NLRefDInfo(DInfo);
        NewDInfo= TRUE;
        KdPrint(("\nDirInfo Cache: no record, build new one - %wZ\n", &DInfo->Path));
    }
    else
    {
        KdPrint(("\nDirInfo Cache: hit record - %wZ\n", &DInfo->Path));
    }
    
    NLPerfStart(&Counter);
    switch(FileInfoClass)
    {
    case FileBothDirectoryInformation:
        CheckFileBothDirectoryInformation(Filter, Instance, (PFILE_BOTH_DIR_INFORMATION)DirectoryBuffer, ProgramDir, DInfo);
        break;
    case FileDirectoryInformation:
        CheckFileDirectoryInformation(Filter, Instance, (PFILE_DIRECTORY_INFORMATION)DirectoryBuffer, ProgramDir, DInfo);
        break;
    case FileFullDirectoryInformation:
        CheckFileFullDirectoryInformation(Filter, Instance, (PFILE_FULL_DIR_INFORMATION)DirectoryBuffer, ProgramDir, DInfo);
        break;
    case FileIdBothDirectoryInformation:
        CheckFileIdBothDirectoryInformation(Filter, Instance, (PFILE_ID_BOTH_DIR_INFORMATION)DirectoryBuffer, ProgramDir, DInfo);
        break;
    case FileIdFullDirectoryInformation:
        CheckFileIdFullDirectoryInformation(Filter, Instance, (PFILE_ID_FULL_DIR_INFORMATION)DirectoryBuffer, ProgramDir, DInfo);
        break;
    case FileNamesInformation:
    case FileObjectIdInformation:
    case FileReparsePointInformation:
    default:
        break;
    }
    NLPerfStop(&Counter);
    KdPrint(("    Time used: %d micros second\n", Counter.elapse.LowPart));

    //
    // Link new directory info
    //
    if(NewDInfo)
    {
        if(DList->Count >= NL_MAX_DIRINFO_LIST_SIZE)
        {
            NLRemoveAllExpiredRecords(DList);
            if(DList->Count >= NL_MAX_DIRINFO_LIST_SIZE)
                NLRemoveFirstRecord(DList);
        }
        KeAcquireSpinLock(&DList->Lock, &OldIrql);
        InsertTailList(&DList->ListHead, &DInfo->Entry);
        DList->Count++;
        KeReleaseSpinLock(&DList->Lock, OldIrql);
    }

    //
    // Update record time, and dereference use count
    //
    KeQuerySystemTime(&DInfo->Time);
    NLDerefDInfo(DInfo);
}

VOID
NLSEAdjustFileSizeInPostQueryInformation(
    __inout PFLT_CALLBACK_DATA Data
    )
{
    PFILE_STANDARD_INFORMATION          fsi  = NULL;
    PFILE_ALL_INFORMATION               fai  = NULL;
    PFILE_POSITION_INFORMATION          fpi  = NULL;
    PFILE_ALLOCATION_INFORMATION        fali = NULL;
    PFILE_NETWORK_OPEN_INFORMATION      fnoi = NULL;
    PFILE_VALID_DATA_LENGTH_INFORMATION fvdl = NULL;

    // If query return nothing, just return
    if(0 == Data->IoStatus.Information)
        return;

    if(FilePositionInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass
        && Data->IoStatus.Information>=sizeof(FILE_POSITION_INFORMATION))
    {
        fpi = (PFILE_POSITION_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer;
        if(fpi->CurrentByteOffset.QuadPart >= NLSE_ENVELOPE_SIZE)
            fpi->CurrentByteOffset.QuadPart -= NLSE_ENVELOPE_SIZE;
    }
    else if(FileAllocationInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass
        && Data->IoStatus.Information>=sizeof(FILE_ALLOCATION_INFORMATION))
    {
        fali = (PFILE_ALLOCATION_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer; 
        if(fali->AllocationSize.QuadPart >= NLSE_ENVELOPE_SIZE)
        {
            fali->AllocationSize.QuadPart -= NLSE_ENVELOPE_SIZE;
            fali->AllocationSize.QuadPart = ROUND_TO_SIZE(fali->AllocationSize.QuadPart, 4096); // Round to page size
        }
    }
    else if(FileValidDataLengthInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass
        && Data->IoStatus.Information>=sizeof(FILE_VALID_DATA_LENGTH_INFORMATION))
    {
        fvdl = (PFILE_VALID_DATA_LENGTH_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer; 
        if(fvdl->ValidDataLength.QuadPart >= NLSE_ENVELOPE_SIZE)
            fvdl->ValidDataLength.QuadPart -= NLSE_ENVELOPE_SIZE;
    }
    else if(FileNetworkOpenInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass
        && Data->IoStatus.Information>=sizeof(FILE_NETWORK_OPEN_INFORMATION))
    {
        fnoi = (PFILE_NETWORK_OPEN_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer;
        if(fnoi->EndOfFile.QuadPart >= NLSE_ENVELOPE_SIZE)
            fnoi->EndOfFile.QuadPart -= NLSE_ENVELOPE_SIZE;
        if(fnoi->AllocationSize.QuadPart >= NLSE_ENVELOPE_SIZE)
        {
            fnoi->AllocationSize.QuadPart -= NLSE_ENVELOPE_SIZE;
            fnoi->AllocationSize.QuadPart = ROUND_TO_SIZE(fnoi->AllocationSize.QuadPart, 4096); // Round to page size
        }
    }
    else if(FileStandardInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass
        && Data->IoStatus.Information>=sizeof(FILE_STANDARD_INFORMATION))
    {
        fsi = (PFILE_STANDARD_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer;
        if(fsi->EndOfFile.QuadPart >= NLSE_ENVELOPE_SIZE)
            fsi->EndOfFile.QuadPart -= NLSE_ENVELOPE_SIZE;
        if(fsi->AllocationSize.QuadPart >= NLSE_ENVELOPE_SIZE)
        {
            fsi->AllocationSize.QuadPart -= NLSE_ENVELOPE_SIZE;
            fsi->AllocationSize.QuadPart = ROUND_TO_SIZE(fsi->AllocationSize.QuadPart, 4096); // Round to page size
        }
    }
    else if(FileAllInformation == Data->Iopb->Parameters.QueryFileInformation.FileInformationClass)
    {
        fai = (PFILE_ALL_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer;
        if (Data->IoStatus.Information >= (sizeof(FILE_BASIC_INFORMATION) + sizeof(FILE_STANDARD_INFORMATION)))
        {
            // Standard information is valid
            if(fai->StandardInformation.EndOfFile.QuadPart >= NLSE_ENVELOPE_SIZE)
                fai->StandardInformation.EndOfFile.QuadPart -= NLSE_ENVELOPE_SIZE;
            if(fai->StandardInformation.AllocationSize.QuadPart >= NLSE_ENVELOPE_SIZE)
            {
                fai->StandardInformation.AllocationSize.QuadPart -= NLSE_ENVELOPE_SIZE;
                fai->StandardInformation.AllocationSize.QuadPart = ROUND_TO_SIZE(fai->StandardInformation.AllocationSize.QuadPart, 4096); // Round to page size
            }

            if(Data->IoStatus.Information >= (sizeof(FILE_BASIC_INFORMATION)+sizeof(FILE_STANDARD_INFORMATION)+sizeof(FILE_INTERNAL_INFORMATION)+sizeof(FILE_EA_INFORMATION)+sizeof(FILE_ACCESS_INFORMATION)+sizeof(FILE_POSITION_INFORMATION)))
            {
                // Position information is valid
                if(fai->PositionInformation.CurrentByteOffset.QuadPart >= NLSE_ENVELOPE_SIZE)
                    fai->PositionInformation.CurrentByteOffset.QuadPart -= NLSE_ENVELOPE_SIZE;
            }
        }
    }
    else
    {
        ; // Do nothing
    }
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSECalculateAesPadWhenFileGrow(
                                __in PFLT_INSTANCE Instance,
                                __in PFILE_OBJECT FileObject,
                                __in ULONG SectorSize,
                                __in_bcount(16) PUCHAR AesKey,
                                __in const LARGE_INTEGER* FileSizePreSet,
                                __in const LARGE_INTEGER* FileSizePostSet,
                                __inout PULONG AesPadLength,
                                __inout_bcount_full(16) PUCHAR AesPadData
                                )
{
    NTSTATUS        Status              = STATUS_SUCCESS;
    LARGE_INTEGER   RawOffset           = {0};
    LARGE_INTEGER   BlockOffset         = {0};
    ULONG           BlockSize           = 0;
    PUCHAR          BlockBuffer         = NULL;
    ULONG           DecryptSize         = 0;
    ULONG           AesPadOffset        = 0;
    ULONG           NeededAesPadLength  = 0;
    static const ULONG MaxEmptyBufferSize  = 16384;


    // Allocate buffer
    BlockBuffer = ExAllocatePoolWithTag(NonPagedPool, MaxEmptyBufferSize, NLFSE_BUFFER_TAG);
    if(NULL == BlockBuffer) return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(BlockBuffer, MaxEmptyBufferSize);


    // Since the crypto size is 512 which is <= the sector size
    // So the block size should be aligned with the sector size
    if(0 != (FileSizePreSet->QuadPart%nlfseGlobal.cryptoBlockSize))
    {
        // Since the crypto size is 512 which is <= the sector size
        // So the block size should be aligned with the sector size
        BlockOffset.QuadPart = ROUND_TO_SIZE(FileSizePreSet->QuadPart, SectorSize) - SectorSize;
        BlockSize            = SectorSize;
        DecryptSize          = (ULONG)(ROUND_TO_SIZE((FileSizePreSet->QuadPart - BlockOffset.QuadPart), nlfseGlobal.cbcBlockSize));
        AesPadOffset         = (ULONG)(FileSizePreSet->QuadPart - BlockOffset.QuadPart);
        NeededAesPadLength   = DecryptSize - AesPadOffset;

        ASSERT(0 == (BlockOffset.QuadPart%nlfseGlobal.cryptoBlockSize));
        ASSERT(*AesPadLength == NeededAesPadLength);

        // a. read last block (SectorSize), and decrypt it
        Status = NLSEReadFileSync(Instance, FileObject, TRUE, FALSE, &BlockOffset, &BlockSize, BlockBuffer);
        if(0 == BlockSize)
            goto _exit;

        // b. if we need padding, append old padding
        if(0 != NeededAesPadLength)
            RtlCopyMemory(BlockBuffer+AesPadOffset, AesPadData, NeededAesPadLength);

        // c. decrypt it
        RawOffset.QuadPart = BlockOffset.QuadPart - NLSE_ENVELOPE_SIZE;
        decrypt_buffer(AesKey, 16, RawOffset.QuadPart, BlockBuffer, DecryptSize);
        RtlZeroMemory((BlockBuffer+AesPadOffset), (SectorSize-AesPadOffset)); // The data after padding offset should be ZERO

        // d. re-encrypt it
        encrypt_buffer(AesKey, 16, RawOffset.QuadPart, BlockBuffer, SectorSize);

        // e. write it to disk
        Status = NLSEWriteFileSync(Instance, FileObject, TRUE, FALSE, &BlockOffset, &BlockSize, BlockBuffer);
        if((BlockOffset.QuadPart+BlockSize) >= FileSizePostSet->QuadPart)
        {
            // we have written all the useful data to disk, calculate the padding
            RtlZeroMemory(AesPadData, 16);
            AesPadOffset = (ULONG)(FileSizePostSet->QuadPart - BlockOffset.QuadPart);
            *AesPadLength = ROUND_TO_SIZE(AesPadOffset, nlfseGlobal.cbcBlockSize) - AesPadOffset;
            if(0 != *AesPadLength) RtlCopyMemory(AesPadData, (BlockBuffer+AesPadOffset), *AesPadLength);

            if((BlockOffset.QuadPart+BlockSize) > FileSizePostSet->QuadPart)
            {
                // We write more data and exceed file size, so we need to reset file size
                NLSESetFileSizeSync(Instance, FileObject, FileSizePostSet);
            }

            // Since we already reach the end, don't need to write any more
            goto _exit;
        }
    }

    BlockOffset.QuadPart = ROUND_TO_SIZE(FileSizePreSet->QuadPart, SectorSize);
    while(BlockOffset.QuadPart < FileSizePostSet->QuadPart)
    {
        // There is empty area, we need to fil them
        RtlZeroMemory(BlockBuffer, MaxEmptyBufferSize);
        RawOffset.QuadPart = BlockOffset.QuadPart - NLSE_ENVELOPE_SIZE;
        encrypt_buffer(AesKey, 16, RawOffset.QuadPart, BlockBuffer, MaxEmptyBufferSize);
        
        // Each time we write 16K bytes
        BlockSize = MaxEmptyBufferSize;
        Status = NLSEWriteFileSync(Instance, FileObject, TRUE, FALSE, &BlockOffset, &BlockSize, BlockBuffer);
        if((BlockOffset.QuadPart+BlockSize) >= FileSizePostSet->QuadPart)
        {
            // we have written all the useful data to disk, calculate the padding
            RtlZeroMemory(AesPadData, 16);
            AesPadOffset = (ULONG)(FileSizePostSet->QuadPart - BlockOffset.QuadPart);
            *AesPadLength = ROUND_TO_SIZE(AesPadOffset, nlfseGlobal.cbcBlockSize) - AesPadOffset;
            if(0 != *AesPadLength) RtlCopyMemory(AesPadData, (BlockBuffer+AesPadOffset), *AesPadLength);

            if((BlockOffset.QuadPart+BlockSize) > FileSizePostSet->QuadPart)
            {
                // We write more data and exceed file size, so we need to reset file size
                NLSESetFileSizeSync(Instance, FileObject, FileSizePostSet);
            }

            // Since we already reach the end, don't need to write any more
            goto _exit;
        }

        BlockOffset.QuadPart += BlockSize;  // Move to next
        ASSERT( 0 == (BlockOffset.QuadPart%SectorSize) );
    }

_exit:
    if(NULL != BlockBuffer) ExFreePool(BlockBuffer); BlockBuffer=NULL;
    return Status;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSECalculateAesPadWhenFileShrink(
                                  __in PFLT_INSTANCE Instance,
                                  __in PFILE_OBJECT FileObject,
                                  __in ULONG SectorSize,
                                  __in const LARGE_INTEGER* FileSizePreSet,
                                  __in const LARGE_INTEGER* FileSizePostSet,
                                  __inout PULONG AesPadLength,
                                  __inout_bcount_full(16) PUCHAR AesPadData
                                  )
{
    NTSTATUS        Status          = STATUS_SUCCESS;
    LARGE_INTEGER   LastBlockOffset = {0};
    ULONG           LastBlockSize   = 0;
    ULONG           LastBlockValid  = 0;
    PUCHAR          BlockBuffer     = NULL;
    ULONG           NewAesPadLength = 0;

    // Calculate new padding length
    NewAesPadLength   = (ULONG)(ROUND_TO_SIZE(FileSizePostSet->QuadPart, nlfseGlobal.cbcBlockSize) - FileSizePostSet->QuadPart);

    // If the file shrink, we need to get new padding information at PreSetInformation
    // Last Crypto Block
    LastBlockOffset.QuadPart= (FileSizePostSet->QuadPart/nlfseGlobal.cryptoBlockSize) * nlfseGlobal.cryptoBlockSize;
    LastBlockSize           = ROUND_TO_SIZE(nlfseGlobal.cryptoBlockSize, SectorSize);
    LastBlockValid          = (ULONG)(FileSizePostSet->QuadPart - LastBlockOffset.QuadPart);
    LastBlockValid          = ROUND_TO_SIZE(LastBlockValid, nlfseGlobal.cbcBlockSize);

    // Okay, we need to re-calculate the padding
    if(0 == NewAesPadLength)
    {
        *AesPadLength = 0;
        RtlZeroMemory(AesPadData, 16); // New padding is Zero, don't need to modify
    }
    else
    {
        // New padding is not Zero, we need to read data out, and update padding
        BlockBuffer = ExAllocatePoolWithTag(NonPagedPool, LastBlockSize, NLFSE_BUFFER_TAG);
        if(NULL == BlockBuffer)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"       * Fail to allocate buffer to read data from disk\n");
            goto _exit;
        }

        Status = NLSEReadFileSync(Instance, FileObject, TRUE, FALSE, &LastBlockOffset, &LastBlockSize, BlockBuffer);
        if(!NT_SUCCESS(Status) || 0==LastBlockSize)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"       * Fail to read data from disk (0x%08x)\n", Status);
            goto _exit;
        }

        if(LastBlockSize < (LastBlockValid - nlfseGlobal.cbcBlockSize))
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"       * Fail to read data from disk (read: %d, expected: %d)\n", LastBlockSize, LastBlockValid);
            goto _exit;
        }
        else if(LastBlockSize < LastBlockValid)
        {
            // We need padding
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_INFO,"       Data is read, but we still need to use old padding\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_INFO,"           Padding Needed: %d\n", (LastBlockValid-LastBlockSize));
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_INFO,"           Old AesPadLen:  %d\n", *AesPadLength);
            if((LastBlockValid-LastBlockSize) != *AesPadLength)
            {
                NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"             * Padding length is not correct\n");
            }
            RtlCopyMemory((BlockBuffer + LastBlockValid - *AesPadLength), AesPadData, *AesPadLength);
        }

        // Good, we get valid padding
        *AesPadLength   = NewAesPadLength;
        RtlCopyMemory(AesPadData, (BlockBuffer + LastBlockValid - NewAesPadLength), NewAesPadLength);
    }

_exit:
    if(BlockBuffer) ExFreePool(BlockBuffer); BlockBuffer = NULL;
    return Status;
}

__checkReturn
NTSTATUS
NLSEIsPathEncryptedEx(
                      __in PCFLT_RELATED_OBJECTS FltObjects,
                      __in PCUNICODE_STRING FullPath,
                      __in PCUNICODE_STRING VolumeName,
                      __in PCUNICODE_STRING VolumeDosName,
                      __out BOOLEAN* Encrypted
                      )
{
    NTSTATUS            Status      = STATUS_SUCCESS;
    UNICODE_STRING      DosFileName = {0};

    *Encrypted = FALSE;

    // Get DOS file name
    Status = NLSEBuildDosPath(FullPath, VolumeName, VolumeDosName,
                              &DosFileName);
    if(!NT_SUCCESS(Status))
    {
        goto _exit;
    }

    // Check this path
    Status = NLSEIsPathEncrypted(FltObjects, &DosFileName, Encrypted);

_exit:
    if(NULL != DosFileName.Buffer) ExFreePool(DosFileName.Buffer); DosFileName.Buffer = NULL;
    return Status;
}

static const UNICODE_STRING DirRoot     = RTL_CONSTANT_STRING(L"\\");
static const UNICODE_STRING DirBoot     = RTL_CONSTANT_STRING(L"\\Boot\\");
static const UNICODE_STRING DirWindows  = RTL_CONSTANT_STRING(L"\\Windows\\");
static const UNICODE_STRING DirProgFile = RTL_CONSTANT_STRING(L"\\Program Files\\");
static const UNICODE_STRING DirProgData = RTL_CONSTANT_STRING(L"\\ProgramData\\");
static const UNICODE_STRING DirSystem   = RTL_CONSTANT_STRING(L"\\Windows\\System\\");
static const UNICODE_STRING DirSystem32 = RTL_CONSTANT_STRING(L"\\Windows\\System32\\");
static const UNICODE_STRING DirSVI      = RTL_CONSTANT_STRING(L"\\System Volume Information\\");
static const UNICODE_STRING DirNextLabs = RTL_CONSTANT_STRING(L"\\Program Files\\NextLabs\\");
static const UNICODE_STRING DirVMVare   = RTL_CONSTANT_STRING(L"\\ProgramData\\VMware\\");

static const UNICODE_STRING NameBoot    = RTL_CONSTANT_STRING(L"Boot");
static const UNICODE_STRING NameWindows = RTL_CONSTANT_STRING(L"Windows");
static const UNICODE_STRING NameSVI     = RTL_CONSTANT_STRING(L"System Volume Information");
static const UNICODE_STRING NameNextLabs= RTL_CONSTANT_STRING(L"NextLabs");
static const UNICODE_STRING NameVMVare  = RTL_CONSTANT_STRING(L"VMware");
__checkReturn
BOOLEAN
IsUncheckDiretory(
                  __in PCUNICODE_STRING ParentDir, /*Without Volume*/
                  __in PCUNICODE_STRING FileName
                  )
{
    BOOLEAN UnderRoot = FALSE;


    if(0 == RtlCompareUnicodeString(ParentDir, &DirRoot, FALSE))
    {
        if(0 == RtlCompareUnicodeString(FileName, &NameBoot, TRUE)) return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &NameWindows, TRUE)) return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &NameSVI, TRUE)) return TRUE;
    }
    else if(0 == RtlCompareUnicodeString(ParentDir, &DirProgFile, TRUE))
    {
        if(0 == RtlCompareUnicodeString(FileName, &NameNextLabs, TRUE)) return TRUE;
        if(0 == RtlCompareUnicodeString(FileName, &NameVMVare, TRUE)) return TRUE;
    }
    else if(0 == RtlCompareUnicodeString(ParentDir, &DirProgData, TRUE))
    {
        if(0 == RtlCompareUnicodeString(FileName, &NameVMVare, TRUE)) return TRUE;
    }
    else if(0 == RtlCompareUnicodeString(ParentDir, &DirWindows, TRUE)
        || 0 == RtlCompareUnicodeString(ParentDir, &DirSystem, TRUE)
        || 0 == RtlCompareUnicodeString(ParentDir, &DirSystem32, TRUE)
        || 0 == RtlCompareUnicodeString(ParentDir, &DirSVI, TRUE)
        || 0 == RtlCompareUnicodeString(ParentDir, &DirNextLabs, TRUE)
        || 0 == RtlCompareUnicodeString(ParentDir, &DirVMVare, TRUE)
        )
    {
        return TRUE;
    }
    else
    {
        return FALSE; //
    }

    return FALSE;
}

__drv_maxIRQL(APC_LEVEL)
VOID
NLSEFlushStreamContext(
                       __in PFLT_INSTANCE Instance,
                       __in PFILE_OBJECT FileObject,
                       __in BOOLEAN Paging,
                       __inout PNLFSE_STREAM_CONTEXT StmContext
                       )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    if(!StmContext->encryptExtDirty)
        return;

    if(NULL==StmContext->encryptExt)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[NLSE::FlushStreamContext] NULL Encrypt Extension\n");
        return;
    }

    Status = NLSEUpdateEncryptSection2(Instance, FileObject, Paging, NULL, (PUCHAR)&nlfseGlobal.currentPCKeyID, nlfseGlobal.currentPCKey, StmContext->encryptExt);
    if(NT_SUCCESS(Status))
    {
        StmContext->encryptExtDirty = FALSE;

        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE::FlushStreamContext]\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Name:         %wZ\n", &StmContext->FileName);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size 1:  %I64d\n", StmContext->FileSize.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size 2:  %I64d\n", StmContext->encryptExt->fileRealLength);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Pad Expected: %d\n", (16 - StmContext->encryptExt->fileRealLength%16)%16);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Pad Real:     %d\n", StmContext->encryptExt->paddingLen);

        return;
    }

    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"\n");
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,"[NLSE::FlushStreamContext] Fail to update encrypt section (0x%08x)\n", Status);
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
NLSEDirectoryControlRoutine(
                            __in PFLT_GENERIC_WORKITEM FltWorkItem,
                            __in PVOID FltObject,
                            __in_opt PVOID WorkItemContext
                            )
{
    NTSTATUS                    Status          = STATUS_SUCCESS;
    FILE_INFORMATION_CLASS      FileInfoClass   = 0;
    PVOID                       DirectoryBuffer = NULL;
    PNLSE_DIRCONTROL_CONTEXT    Context         = (PNLSE_DIRCONTROL_CONTEXT)WorkItemContext;
    PFLT_CALLBACK_DATA          Data            = NULL;

    // Sanity check
    if(NULL==FltWorkItem) return;
    if(NULL==Context) return;

    
    // well, we have to get the memory and modify it
    __try
    {
        Data = Context->Data;
        if(NULL==Data)
            __leave;
        
        // Get file info class
        FileInfoClass   = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;

        if(NULL != Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress)
        {
            // We need to get buffer from MDL
            DirectoryBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress, NormalPagePriority);
            if(NULL == DirectoryBuffer)
            {
                // Fail to get valid buffer
                Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Data->IoStatus.Information = 0;
                __leave;
            }
        }
        else if(FlagOn(Data->Flags,FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) || FlagOn(Data->Flags,FLTFL_CALLBACK_DATA_FAST_IO_OPERATION))
        {
            // We can use user buffer directly
            DirectoryBuffer = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
        }
        else
        {
            // Well, we have to lock buffer to get a valid MDL
            Status = FltLockUserBuffer(Data);
            if (!NT_SUCCESS(Status) || NULL==Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress) 
            {
                Data->IoStatus.Status = Status;
                Data->IoStatus.Information = 0;
                __leave;
            }

            DirectoryBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.DirectoryControl.QueryDirectory.MdlAddress, NormalPagePriority);
            if(NULL == DirectoryBuffer)
            {
                // Fail to get valid buffer
                Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Data->IoStatus.Information = 0;
                __leave;
            }
        }

        // Okay, check if we really get a valid buffer
        if(NULL == DirectoryBuffer)
        {
            // Fail to get valid buffer
            Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Data->IoStatus.Information = 0;
            __leave;
        }

        // If we get a valid buffer, then change its content
        NLSEHandleDirectoryQuery(Context->Filter, Context->Instance, &Context->Name, Context->ProgramDir, FileInfoClass, DirectoryBuffer, Context->FileObject, Context->DList);
    }
    __finally
    {
        FltFreeGenericWorkItem(FltWorkItem);
        if(NULL != Context)
        {
            if(Context->Name.Buffer) ExFreePool(Context->Name.Buffer); Context->Name.Buffer=NULL;
            ExFreePool(Context); Context=NULL;
        }
        FltCompletePendedPostOperation(Data);
    }
}

__checkReturn
__drv_maxIRQL(APC_LEVEL)
NTSTATUS
NLSEEncryptFileEx(
                  __in PFLT_FILTER Filter,
                  __in PFLT_INSTANCE Instance,
                  __in PCUNICODE_STRING FullPath,
                  __in ULONG FileAttributes,
                  __in ULONG SectorSize
                  )
{
    NTSTATUS                Status      = STATUS_SUCCESS;
    PNLFSE_STREAM_CONTEXT   StmContext  = NULL;
    PFILE_OBJECT            FileObject  = NULL;
    HANDLE                  FileHandle  = NULL;
    PUCHAR                  HeaderBuffer= NULL;
    PUCHAR                  WriteBuffer = NULL;
    PUCHAR                  FetchBuffer = NULL;
    PNLSE_ENVELOPE          Envelope    = NULL;
    PNLSE_ENCRYPT_SECTION   eSec        = NULL;
    PNLSE_SECTION_HEADER    tagSec      = NULL;
    LARGE_INTEGER           FileSize    = {0};
    const ULONG             RWLength    = NLSE_ENVELOPE_SIZE;
    OBJECT_ATTRIBUTES       ObjAttr;
    IO_STATUS_BLOCK         IoStatusBlock;
    STRING                  tmpString   = {0};
    
    LARGE_INTEGER   WriteOffset= {0};
    ULONG           BytesFetch = 0;
    ULONG           BytesValid = 0;
    ULONG           BytesWritten = 0;


    // Allocate Header Buffer
    HeaderBuffer = ExAllocatePoolWithTag(PagedPool, RWLength, NLFSE_BUFFER_TAG);
    if(NULL == HeaderBuffer)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to allocate header buffer. File: %wZ\n", FullPath);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(HeaderBuffer, RWLength);
    Envelope= (PNLSE_ENVELOPE)HeaderBuffer;
    eSec    = (PNLSE_ENCRYPT_SECTION)(HeaderBuffer + 2*NLSE_DEFAULT_SECTOR_SIZE);
    tagSec  = (PNLSE_SECTION_HEADER)(HeaderBuffer + 3*NLSE_DEFAULT_SECTOR_SIZE);
    NLSEInitNextLabsHeader(Envelope);
    NLSEInitTagsSectionHeader(tagSec);
    NLSEInitEncryptSectionHeader(eSec, NULL, (PUCHAR)&nlfseGlobal.currentPCKeyID);

    
    // Allocate memory for stream context
    Status = FltAllocateContext(Filter, FLT_STREAM_CONTEXT, sizeof(NLFSE_STREAM_CONTEXT), NonPagedPool, &StmContext);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to allocate stream context (0x%08X). File: %wZ\n", Status, FullPath);
        goto _exit;
    }
    RtlZeroMemory(StmContext, sizeof(NLFSE_STREAM_CONTEXT));
#pragma prefast(disable:6014, "Release this memory in different function cause incorrect preFast warning 6014 - memory Leak")
    StmContext->encryptExt = ExAllocatePoolWithTag(NonPagedPool, sizeof(NLFSE_ENCRYPT_EXTENSION), NLFSE_BUFFER_TAG);
#pragma prefast(enable:6014, "Recover this warning")
    if(NULL==StmContext->encryptExt)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to allocate encrypt extension for stream context. File: %wZ\n", FullPath);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(StmContext->encryptExt, sizeof(NLFSE_ENCRYPT_EXTENSION));

    // Allocate Write/Fetch Buffer
    WriteBuffer = ExAllocatePoolWithTag(PagedPool, RWLength, NLFSE_BUFFER_TAG);
    if(NULL == WriteBuffer)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to allocate write buffer. File: %wZ\n", FullPath);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(WriteBuffer, RWLength);
    FetchBuffer = ExAllocatePoolWithTag(PagedPool, RWLength, NLFSE_BUFFER_TAG);
    if(NULL == FetchBuffer)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to allocate fetch buffer. File: %wZ\n", FullPath);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(FetchBuffer, RWLength);

    // Open file
    InitializeObjectAttributes(&ObjAttr, (PUNICODE_STRING)FullPath, OBJ_KERNEL_HANDLE, NULL, NULL);
    Status = FltCreateFile(Filter,
        Instance,
        &FileHandle,
        GENERIC_WRITE|GENERIC_READ|SYNCHRONIZE,
        &ObjAttr,
        &IoStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_COMPLETE_IF_OPLOCKED|FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(STATUS_OPLOCK_BREAK_IN_PROGRESS == Status)
    {
        // File is locked?        
        // According to DDK Help (See help page for IRP_MJ_CREATE):
        //   If a filter or minifilter cannot honor the FILE_COMPLETE_IF_OPLOCKED flag,
        //   it must complete the IRP_MJ_CREATE request with STATUS_SHARING_VIOLATION.
        FltClose(FileHandle); FileHandle = NULL;
        Status  = STATUS_SHARING_VIOLATION;
        goto _exit;
    }
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to open file (%0x08X). File: %wZ\n", Status, FullPath);
        goto _exit;
    }
    
    Status = ObReferenceObjectByHandle(FileHandle, 0, NULL, KernelMode, &FileObject, NULL);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to reference file object (%0x08X). File: %wZ\n", Status, FullPath);
        goto _exit;
    }

    // Get file size
    Status = NLSEGetFileSizeSync(Instance, FileObject, &FileSize);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to get file size (%0x08X). File: %wZ\n", Status, FullPath);
        goto _exit;
    }

    // Initialize Stream Context
    StmContext->encryptExtDirty     = FALSE;
	StmContext->bDelete             = FALSE;
	StmContext->UseCount            = 1;
    StmContext->FileAttrs           = FileAttributes;
    StmContext->SectorSize          = SectorSize;
    StmContext->FileSize.QuadPart   = FileSize.QuadPart;
	KeInitializeSpinLock(&StmContext->encryptExtLock);
	ExInitializeFastMutex(&StmContext->deleteFlagLock);
#pragma prefast(disable:6014, "Release this memory in different function cause incorrect preFast warning 6014 - memory Leak")
    StmContext->FileName.Buffer = ExAllocatePoolWithTag(NonPagedPool, FullPath->MaximumLength, NLFSE_BUFFER_TAG);
#pragma prefast(enable:6014, "Recover this warning")
    if(NULL == StmContext->FileName.Buffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(StmContext->FileName.Buffer, FullPath->MaximumLength);
    RtlCopyMemory(StmContext->FileName.Buffer, FullPath->Buffer, FullPath->Length);    
    StmContext->FileName.Length  = FullPath->Length;

    // Init Encrypt Extension
    RtlInitString(&tmpString, NLE_STREAM_NAME);
    RtlCopyMemory(StmContext->encryptExt->sh.stream_name, tmpString.Buffer, tmpString.Length);  
    StmContext->encryptExt->sh.stream_size   = sizeof(NLFSE_ENCRYPT_EXTENSION);
    StmContext->encryptExt->version_major    = NLE_FILE_VERSION_MAJOR;
    StmContext->encryptExt->version_minor    = NLE_FILE_VERSION_MINOR;
    StmContext->encryptExt->fileRealLength   = FileSize.QuadPart;
    StmContext->encryptExt->flags            = 0;
    RtlCopyMemory(StmContext->encryptExt->pcKeyRingName, eSec->PcKeyRingName, 16);
    RtlCopyMemory(&StmContext->encryptExt->pcKeyID, eSec->PcKeyId, 36);
    RtlCopyMemory(StmContext->encryptExt->key, eSec->AesKey, 16);
    // Encrypt AesKey before write it to disk
    encrypt_buffer(nlfseGlobal.currentPCKey, 16, 0, eSec->AesKey, 16);

    // Initialize Offset
    WriteOffset.QuadPart = 0;
    RtlCopyMemory(WriteBuffer, HeaderBuffer, RWLength);
    BytesValid = RWLength;
    do
    {
        // Fectch data that will be covered
        BytesFetch = 0;
        RtlZeroMemory(FetchBuffer, RWLength);
        Status = FltReadFile(Instance, FileObject, &WriteOffset, RWLength, FetchBuffer, FLTFL_IO_OPERATION_NON_CACHED, &BytesFetch, NULL, NULL);
        if(NT_SUCCESS(Status))
        {
            encrypt_buffer(StmContext->encryptExt->key, 16, WriteOffset.QuadPart, FetchBuffer, RWLength);
        }

        // Write data to file
        Status = FltWriteFile(Instance, FileObject, &WriteOffset, RWLength, WriteBuffer, FLTFL_IO_OPERATION_NON_CACHED, &BytesWritten, NULL, NULL);
        if(!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSEEncryptFileEx! Fail to write encrypted data to file (%0x08X). File: %wZ\n", Status, FullPath);
            goto _exit;
        }
        // Increase the offset
        WriteOffset.QuadPart += BytesWritten;
        if(WriteOffset.QuadPart >= (FileSize.QuadPart+NLSE_ENVELOPE_SIZE))
        {
            // Nice
            if(0 ==(BytesValid%16))
            {
                StmContext->encryptExt->paddingLen = 0;
                RtlZeroMemory(StmContext->encryptExt->paddingData, 16);
            }
            else
            {
                StmContext->encryptExt->paddingLen = 16 - BytesValid%16;
                RtlZeroMemory(StmContext->encryptExt->paddingData, 16);
                RtlCopyMemory(StmContext->encryptExt->paddingData, WriteBuffer+BytesValid, StmContext->encryptExt->paddingLen);
            }

            // Flush stream context
            StmContext->encryptExtDirty = TRUE;
            NLSEFlushStreamContext(Instance, FileObject, FALSE, StmContext);
            Status = STATUS_SUCCESS;

            break;
        }

        // Encrypt fect data and copy it to write buffer
        RtlCopyMemory(WriteBuffer, FetchBuffer, RWLength);
        BytesValid = BytesFetch;

    }while(WriteOffset.QuadPart < (FileSize.QuadPart+NLSE_ENVELOPE_SIZE));

    // Make sure file size is correct
    FileSize.QuadPart += NLSE_ENVELOPE_SIZE;
    NLSESetFileSizeSync(Instance, FileObject, &FileSize);
    NLSEPurgeFileCache(FileObject, TRUE, NULL, 0);

    // Set up stream context
    Status = FltSetStreamContext(Instance, FileObject, FLT_SET_CONTEXT_REPLACE_IF_EXISTS, StmContext, NULL);
    if(NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Insert stream context when file is renamed\n");
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Name:   %wZ\n", &StmContext->FileName);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size:   %d\n", StmContext->FileSize.QuadPart);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Sector Size: %d\n", StmContext->SectorSize);
    }

_exit:
    if(HeaderBuffer) ExFreePool(HeaderBuffer);
    if(FetchBuffer) ExFreePool(FetchBuffer);
    if(WriteBuffer) ExFreePool(WriteBuffer);
    if(StmContext) FltReleaseContext(StmContext);
    if(NULL != FileObject) ObDereferenceObject(FileObject);
    if(NULL != FileHandle) FltClose(FileHandle);
    return Status;
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
NLSERenameWorkRoutine(
                      __in PFLT_GENERIC_WORKITEM FltWorkItem,
                      __in PVOID FltObject,
                      __in_opt PVOID WorkItemContext
                      )
{
    NTSTATUS                Status      = STATUS_SUCCESS;
    PNLSE_SETINFO_CONTEXT   Context     = (PNLSE_SETINFO_CONTEXT)WorkItemContext;
    PFLT_CALLBACK_DATA      Data        = NULL;
    ULONG                   Attributes  = 0;
    BOOLEAN                 ReadOnlyRemoved = FALSE;

    // Sanity check
    if(NULL==FltWorkItem) return;
    if(NULL==Context) return;

    ASSERT(NULL==Context->StmContext);

    Data = Context->Data;

    // Get file attributes
    Status = NLSEGetFileAttributesSync(Context->Instance, Context->FileObject, &Attributes);
    if(!NT_SUCCESS(Status))
    {
        // Fail to get file attributes?
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSERenameWorkRoutine! Fail to get file attributes. File: %wZ\n", &Context->TargetName);
        goto _exit;
    }

    // If it is read only, we need to remove readonly flag
    if(FlagOn(Attributes, FILE_ATTRIBUTE_READONLY))
    {
        Attributes &= (~FILE_ATTRIBUTE_READONLY);
        Status = NLSESetFileAttributesSync(Context->Instance, Context->FileObject, Attributes);
        if(!NT_SUCCESS(Status))
        {
            // Fail to set file attributes?
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSERenameWorkRoutine! Fail to remove readonly flag. File: %wZ\n", &Context->TargetName);
            goto _exit;
        }
        ReadOnlyRemoved = TRUE;
        Attributes |= FILE_ATTRIBUTE_READONLY;
    }

    // Encrypt file
    Status = NLSEEncryptFileEx(Context->Filter, Context->Instance, &Context->TargetName, Attributes, Context->SectorSize);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSERenameWorkRoutine! Fail to encrypt file %wZ\n", &Context->TargetName);
    }


_exit:
    if(ReadOnlyRemoved) NLSESetFileAttributesSync(Context->Instance, Context->FileObject, Attributes);
    FltFreeGenericWorkItem(FltWorkItem);
    if(NULL != Context)
    {
        if(Context->StmContext) FltReleaseContext(Context->StmContext);
        if(Context->TargetName.Buffer) ExFreePool(Context->TargetName.Buffer); Context->TargetName.Buffer=NULL; Context->TargetName.Length=0;
        ExFreePool(Context); Context=NULL;
    }
    FltCompletePendedPostOperation(Data);
}
