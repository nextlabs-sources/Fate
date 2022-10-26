/*++

Module Name:

NLSEOperation.c

Abstract:
IPR operation for system encryption in kernel mode

Environment:

Kernel mode

--*/
#include <ntifs.h>
#include <ntdef.h>
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "FileOpHelp.h"
#include "nl_klog.h"
#include "NLSEDrmPathList.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern const UNICODE_STRING nlfseTestFile[];
extern NL_KLOG nlseKLog;
extern ULONG nlseLoggingFlags;

NTSTATUS NLSEPreCreateAndPostWorkItem( PFLT_CALLBACK_DATA );

static const UNICODE_STRING nlseShadowPrefix=RTL_CONSTANT_STRING(L"\\Device\\HarddiskVolumeShadowCopy");
static const UNICODE_STRING NextLabsDir86   =RTL_CONSTANT_STRING(L"\\Program Files (x86)\\NextLabs\\");
static const UNICODE_STRING ProgramsDir86   =RTL_CONSTANT_STRING(L"\\Program Files (x86)\\");
static const UNICODE_STRING NextLabsDir     =RTL_CONSTANT_STRING(L"\\Program Files\\NextLabs\\");
static const UNICODE_STRING ProgramsDir     =RTL_CONSTANT_STRING(L"\\Program Files\\");
static const UNICODE_STRING WindowsDir      =RTL_CONSTANT_STRING(L"\\Windows\\");

static
__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
BOOLEAN
NLSEIsNxlFile(
              __in PCUNICODE_STRING FileName
              );
/*************************************************************************
Local helper routines.
*************************************************************************/
static VOID 
CheckWinEFSOrCompressed(__in PCFLT_RELATED_OBJECTS FltObjects,
                        __out BOOLEAN *bEFS,
                        __out BOOLEAN *bCompressed)
{
    FILE_BASIC_INFORMATION fileBasicInfo;
    NTSTATUS               status=STATUS_SUCCESS;

    *bEFS=FALSE;
    *bCompressed=FALSE;
    status = FltQueryInformationFile(FltObjects->Instance,
        FltObjects->FileObject,
        &fileBasicInfo,
        sizeof(fileBasicInfo),
        FileBasicInformation,
        NULL);
    if(NT_SUCCESS(status))
    {
        if(fileBasicInfo.FileAttributes&FILE_ATTRIBUTE_ENCRYPTED)
            *bEFS=TRUE;
        if(fileBasicInfo.FileAttributes&FILE_ATTRIBUTE_COMPRESSED) 
            *bCompressed=TRUE;
    }
}/*--CheckWinEFSOrCompressed--*/

static BOOLEAN IsShadowVolume(__in PCFLT_RELATED_OBJECTS FltObjects,
                              __in NLFSE_PVOLUME_CONTEXT volCtx)
{
    return RtlPrefixUnicodeString(&nlseShadowPrefix, &volCtx->Name, TRUE);
}

static FLT_PREOP_CALLBACK_STATUS 
PendingIOForPolicyEvaluation (__in NLFSE_PIRP_ENTRY irpEntry,
                              __inout PFLT_CALLBACK_DATA Data,
                              __in PCFLT_RELATED_OBJECTS FltObjects)
{
    NLSE_PENDING_IO_QUEUE_PCONTEXT queueCtx = NULL;
    NLFSE_PINSTANCE_CONTEXT instCtx = NULL;
    FLT_PREOP_CALLBACK_STATUS retStatus=FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS status;

    // Allocate a context for each I/O to be inserted into the queue.
    queueCtx=ExAllocateFromNPagedLookasideList(&nlfseGlobal.queueContextLookaside);
    if (queueCtx == NULL)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, ("NLFSE: Failed to allocate from NPagedLookasideList\n"));
        goto PendingIOCleanup;
    }

    RtlZeroMemory(queueCtx, sizeof(NLSE_PENDING_IO_QUEUE_CONTEXT));
    queueCtx->irpEntry=irpEntry;
    queueCtx->FltObjects=FltObjects;

    //Get the instance context.
    status = FltGetInstanceContext( FltObjects->Instance, &instCtx );
    if (!NT_SUCCESS( status ))
    {
        ASSERT( !"Instance context is missing" );
        goto PendingIOCleanup;
    }

    //  Set the queue context
    Data->QueueContext[0] = (PVOID) queueCtx;
    Data->QueueContext[1] = NULL;

    //  Create a new work item with the data
    status = NLSEPreCreateAndPostWorkItem( Data );

    if (status == STATUS_SUCCESS)
    {
        retStatus = FLT_PREOP_PENDING;
    }
    else
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE:Failed to insert into pre-create cbdq (Status = 0x%x)\n", status);
    }

PendingIOCleanup:
    //  Clean up
    if (queueCtx && retStatus != FLT_PREOP_PENDING)
        ExFreeToNPagedLookasideList( &nlfseGlobal.queueContextLookaside,queueCtx );
    if (instCtx)
        FltReleaseContext( instCtx );
    return retStatus;
}



/** NLSEADSWork 
*  
*  Routine to handle deferred update encryption ADS work item. 
*/
static
BOOLEAN
NLSEADSWork(
            __inout PNLFSE_ADS_WORKITEM workItem
            )
{
    NTSTATUS                    Status          = STATUS_SUCCESS;
    PFLT_FILTER                 Filter          = NULL;
    HANDLE                      FileHandle      = NULL;
    PFILE_OBJECT                FileObject      = NULL;
    OBJECT_ATTRIBUTES           oa              = {0};
    IO_STATUS_BLOCK             iosb            = {0};
    BOOLEAN                     DeletePending   = FALSE;
    BOOLEAN                     AttrModified    = FALSE;
    FILE_BASIC_INFORMATION      fbi             = {0};
    UNICODE_STRING              adsFile         = {0};
    FILE_STANDARD_INFORMATION   fsi             = {0};
    BOOLEAN                     bRet            = FALSE;

    ASSERT( workItem != NULL);

    workItem->RetryCount++;

    // Sanity Checking
    if(NULL == workItem->fltInstance)
        goto _exit;
    if(0 == workItem->hostFilePath.Length)
        goto _exit;

    Status = FltGetFilterFromInstance(workItem->fltInstance, &Filter);
    if (!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: get filter, err=0x%x\n", Status);
        Filter=NULL;
        goto _exit;
    }

    Status = FOH_IS_DELETE_PENDING(Filter, workItem->fltInstance, &workItem->hostFilePath, &DeletePending);
    if(!NT_SUCCESS(Status) || DeletePending)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: we don't need to handle a delete pending file\n");
        bRet = TRUE;
        goto _exit;
    }

    Status = FOH_GET_FILE_BASICINFO_BY_NAME(Filter, workItem->fltInstance, &workItem->hostFilePath, &fbi);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: fail to get file attributes\n");
        goto _exit;
    }

    Status = FOH_GET_FILE_STANDARDINFO_BY_NAME(Filter, workItem->fltInstance, &workItem->hostFilePath, &fsi);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: fail to get file standard information\n");
        goto _exit;
    }
    workItem->encryptExt.fileRealLength = fsi.EndOfFile.QuadPart;

    // Remove read-only attribute so that we can write ADS
    if(fbi.FileAttributes & FILE_ATTRIBUTE_READONLY)
    {
        Status = FOH_REMOVE_READONLY_ATTRIBUTES(Filter, workItem->fltInstance, &workItem->hostFilePath);
        if(!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: fail to remove read-only attribute\n");
            goto _exit;
        }
        AttrModified = TRUE;
    }

    // Compose Stream Name
    adsFile.Length          = 0;
    adsFile.MaximumLength   = sizeof(WCHAR)*(2*MAX_PATH);
    adsFile.Buffer          = ExAllocatePoolWithTag(NonPagedPool, adsFile.MaximumLength, NLFSE_BUFFER_TAG);
    if(NULL==adsFile.Buffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork (0x%08x): fail to allocate memory to hold stream name for file %wZ\n", Status, &workItem->hostFilePath);
        goto _exit;
    }
    RtlCopyUnicodeString(&adsFile, &workItem->hostFilePath);
    Status = RtlAppendUnicodeToString(&adsFile, NLFSE_ADS_SUFFIX);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork (0x%08x): fail to compose stream name for file %wZ\n", Status, &workItem->hostFilePath);
        goto _exit;
    }

    // Open stream file
    InitializeObjectAttributes(&oa, &adsFile, OBJ_KERNEL_HANDLE, NULL, NULL);
    Status = FltCreateFile(Filter,
        workItem->fltInstance,
        &FileHandle,
        GENERIC_WRITE,
        &oa,
        &iosb,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 
        FILE_OPEN,
        FILE_COMPLETE_IF_OPLOCKED|FILE_OPEN_REPARSE_POINT|FILE_NO_INTERMEDIATE_BUFFERING,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if(!NT_SUCCESS(Status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!ADSWork: fail to open ADS file\n");
        goto _exit;
    }

    // Reference File Object
    Status = ObReferenceObjectByHandle(FileHandle,       //Handle
        0,            //DesiredAccess
        NULL,         //ObjectType
        KernelMode,   //AccessMode
        &FileObject,  //File Handle
        NULL);
    if(!NT_SUCCESS(Status))
        goto _exit;

    // Write Encrypt Extension to ADS File
    Status = EncryptExtensionToAdsFile(workItem->fltInstance, FileObject, &workItem->encryptExt, workItem->ProcessId, workItem->SectorSize);
    if(NT_SUCCESS(Status))
        bRet = TRUE;
    else
        bRet = FALSE;

_exit:
    // Close stream file
    if(FileObject) ObDereferenceObject(FileObject); FileObject=NULL;
    if(FileHandle) FltClose(FileHandle); FileHandle=NULL;
    // Recover original file attributes
    if(AttrModified)   FOH_SET_FILE_BASICINFO_BY_NAME(Filter, workItem->fltInstance, &workItem->hostFilePath, &fbi);
    if(adsFile.Buffer) ExFreePoolWithTag(adsFile.Buffer, NLFSE_BUFFER_TAG); adsFile.Buffer=0;
    // Free work item
    if(Filter)     FltObjectDereference(Filter); Filter=NULL;

    if(workItem->RetryCount >= 3)
        bRet = TRUE;    // The max retry number is 3

    // Free work item
    if(bRet)
    {
        if(workItem->fltInstance) FltObjectDereference(workItem->fltInstance); workItem->fltInstance=NULL;
        ExFreePoolWithTag(workItem, NLFSE_BUFFER_TAG); workItem=NULL;
    }
    return bRet;
}/* NLSEADSWork */

/** NLSEADSWorkerStart
*  
*  Worker thread to sequentially update AES padding and ADS 
*/
VOID
NLSEAdsWorkerStart(__in PVOID pCtx)
{
    PNLFSE_ADS_WORKITEM workItem=NULL;
    KIRQL               OldIrql; //IRQL of ads worker queue 

    while(TRUE)
    {
        workItem = NULL;
        KeAcquireSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, &OldIrql);
        if (!IsListEmpty(&nlfseGlobal.adsWorkQueue))
        {
            workItem=(PNLFSE_ADS_WORKITEM)RemoveHeadList(&nlfseGlobal.adsWorkQueue);
        }
        KeReleaseSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, OldIrql);

        // Do we have something to do?  If not, we go to sleep and wait.
        if (!workItem)
        {
            // Wait to we're awoken
            KeWaitForSingleObject(&nlfseGlobal.adsWorkerThreadEvent,
                Executive,
                KernelMode,
                FALSE,
                0);
            continue;
        }

        //Execute the work item
        if(!NLSEADSWork(workItem))
        {
            KeAcquireSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, &OldIrql);
            InsertHeadList(&nlfseGlobal.adsWorkQueue, &workItem->adsWorkerThreadList);
            KeReleaseSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, OldIrql);
        }

    }
}

VOID 
NLSEQueueAdsWorkItem(__inout PNLFSE_ADS_WORKITEM workItem) 
{
    KIRQL                     irql; //IRQL of ads worker queue 

    // Add the work item to the list of things to do.
    KeAcquireSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, &irql);
    InsertTailList(&nlfseGlobal.adsWorkQueue, &workItem->adsWorkerThreadList);
    KeReleaseSpinLock(&nlfseGlobal.adsWorkQueueSpinLock, irql);

    //Wake the woker thread
    KeSetEvent(&nlfseGlobal.adsWorkerThreadEvent, 0, FALSE);
} 

static
PVOID 
NLSEPreSetFileDisposition(__in PFLT_CALLBACK_DATA Data,
                          __in PCFLT_RELATED_OBJECTS FltObjects) 
{
    NTSTATUS                     status;
    PNLFSE_STREAM_CONTEXT        pContext=NULL;
    NLFSE_PPRE_2_POST_CONTEXT    p2pCtx=NULL;
    NLFSE_PVOLUME_CONTEXT        volCtx = NULL;

    //  Get our volume context.
    status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx );
    if (!NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreSetFileDisposition: getting volume context, err=%x\n", status);
        return NULL;
    } 

    //pContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);  
	status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pContext );
    if(!NT_SUCCESS(status) || pContext == NULL)
    {
        FltReleaseContext( volCtx );
        return NULL;
    } 

    p2pCtx = ExAllocateFromNPagedLookasideList(&nlfseGlobal.pre2PostContextList);
    if (p2pCtx == NULL)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreSetFileDisposition:  %wZ can't allocate pre2Post ctx\n", &volCtx->Name );
        FltReleaseContext( volCtx );
        FltReleaseContext(pContext);
        return NULL;
    }
    RtlZeroMemory(p2pCtx, sizeof( NLFSE_PRE_2_POST_CONTEXT));

    p2pCtx->streamCtx = pContext;
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "NLSE!PreSetFileDisposition:  %wZ\n", &pContext->FileName );
    return p2pCtx;  
}

//Get the source file of rename operation; 
//check if the file is already encrypted
static PVOID 
NLSEPreFileRename(__in PFLT_CALLBACK_DATA Data,
                  __in PCFLT_RELATED_OBJECTS FltObjects) 
{
    PNLFSE_STREAM_CONTEXT        pContext=NULL;
    NLFSE_PVOLUME_CONTEXT        volCtx = NULL;
    NTSTATUS                     status;
    PFLT_FILE_NAME_INFORMATION   sourceFileNameInfo=NULL;
    NLFSE_PPRE_2_POST_CONTEXT    p2pCtx=NULL;
    FILE_BASIC_INFORMATION       fbi = {0};

    //get source file name
    if (FltObjects->FileObject == NULL) 
        return NULL;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT, &sourceFileNameInfo );
    if (status == STATUS_SUCCESS)
    {
        status = FltParseFileNameInformation( sourceFileNameInfo);
        if(status != STATUS_SUCCESS)
        {
            //failed to parse the file name; do nothing
            FltReleaseFileNameInformation(sourceFileNameInfo);
            return NULL;
        } 
    }
    else
    {
        //failed to get name information; do nothing
        return NULL;
    }

    if(NLSEIsNxlFile(&sourceFileNameInfo->Name))
    {
        // Don't Handle NXL file
        FltReleaseFileNameInformation(sourceFileNameInfo);
        return NULL;
    }

    status = FOH_GET_FILE_BASICINFO_BY_NAME(FltObjects->Filter, FltObjects->Instance, &sourceFileNameInfo->Name, &fbi);
    if(NT_SUCCESS(status)
        && ((fbi.FileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            || (fbi.FileAttributes&FILE_ATTRIBUTE_COMPRESSED)
            || (fbi.FileAttributes&FILE_ATTRIBUTE_ENCRYPTED))
        )
    {
        // Don't care about directory
        FltReleaseFileNameInformation(sourceFileNameInfo);
        return NULL;
    }

    //Get our volume context.
    status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx );
    if (!NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreFileRename: getting volume context, err=%x\n", status);
        FltReleaseFileNameInformation(sourceFileNameInfo);
        return NULL;
    } 

    p2pCtx = ExAllocateFromNPagedLookasideList(&nlfseGlobal.pre2PostContextList);
    if (p2pCtx == NULL)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreFileRename:  %wZ can't allocate pre2Post ctx\n", &volCtx->Name );
        FltReleaseContext( volCtx );
        FltReleaseFileNameInformation(sourceFileNameInfo);
        return NULL;
    }
    RtlZeroMemory(p2pCtx, sizeof( NLFSE_PRE_2_POST_CONTEXT));

    //pContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);  
	status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pContext );
    FltReleaseContext( volCtx );   
    if(NT_SUCCESS(status) && pContext != NULL)
    {
        p2pCtx->bSourceEncrypted=TRUE;
    }
    else
    {
        p2pCtx->bSourceEncrypted=FALSE;
    }

    p2pCtx->streamCtx = pContext;
    p2pCtx->fileNameInfo=sourceFileNameInfo;
    return p2pCtx;
}/*NLSEPreFileRename*/

static
NTSTATUS
NLSEGetEncryptBlock(
                    __in PFLT_INSTANCE Instance,
                    __in PFILE_OBJECT FileObject,
                    __in BOOLEAN PagingIo,
                    __in PLARGE_INTEGER BlockOffset,
                    __in ULONG ByteToRead,
                    __in PUCHAR Padding,
                    __in ULONG PaddingLength,
                    __out_opt PUCHAR BlockData,
                    __out_opt PULONG BlockDataValidLength
                    )
{
    NTSTATUS    Status              = STATUS_SUCCESS;
    ULONG       BytesRead           = 0;
    FLT_IO_OPERATION_FLAGS ReadFlags = PagingIo?(FLTFL_IO_OPERATION_NON_CACHED|FLTFL_IO_OPERATION_PAGING):(FLTFL_IO_OPERATION_NON_CACHED);

    if(NULL==BlockData || NULL==BlockDataValidLength)
        return STATUS_INVALID_PARAMETER;

    ASSERT(0 == ((ByteToRead+PaddingLength)%nlfseGlobal.cbcBlockSize));
    ASSERT(nlfseGlobal.cryptoBlockSize >= (ByteToRead+PaddingLength));

    // DO NOT pass the FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET flag in the
    // call to FltReadFile below!  Passing the flag was causing data corruption
    // even though it is legal to pass the flag.  This is a Microsoft bug in
    // both WinXP and Win7.  See Bug 11198.  (Details to be added later.)
    Status = FltReadFile(Instance,
        FileObject,
        BlockOffset,
        nlfseGlobal.cryptoBlockSize,   // Read one block
        BlockData,
        ReadFlags,
        BlockDataValidLength, 
        NULL, 
        NULL );
    if(!NT_SUCCESS(Status))
        return Status;

    BytesRead = *BlockDataValidLength;
    if(BytesRead < ByteToRead)
        return STATUS_UNEXPECTED_IO_ERROR;

    if(BytesRead > ByteToRead)
    {
        *BlockDataValidLength = ByteToRead;
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_INFO, "NLSE!NLSEGetEncryptBlock:  Read more data (0x%X, 0x%X)\n", ByteToRead, BytesRead );
    }

    if(PaddingLength > 0)
    {
        RtlCopyMemory( (BlockData+ByteToRead), Padding, PaddingLength );
        *BlockDataValidLength += PaddingLength;
    }

    return Status;
}

static
NTSTATUS
NLSEGetPlainBlock(
                  __in PFLT_INSTANCE Instance,
                  __in PFILE_OBJECT FileObject,
                  __in BOOLEAN PagingIo,
                  __in_opt const LARGE_INTEGER* BlockOffset,
                  __in PUCHAR Padding,
                  __in ULONG PaddingLength,
                  __in_opt char* Key,
                  __out_opt PUCHAR BlockData,
                  __out_opt PULONG BlockDataValidLength
                  )
{
    NTSTATUS    Status              = STATUS_SUCCESS;
    ULONG       DecryptLength       = 0;
    ULONG       BytesRead           = 0;
    LARGE_INTEGER ReadOffset        = {0};
    ULONG ReadFlags= IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO;

    if(NULL==Key || NULL==BlockData || NULL==BlockOffset)
        return STATUS_INVALID_PARAMETER;

    ReadOffset.QuadPart = BlockOffset->QuadPart;

    Status = FOH_SYNC_READ(Instance,
        FileObject,
        &ReadOffset,
        nlfseGlobal.cryptoBlockSize,   // Read one block
        BlockData,
        ReadFlags,
        &BytesRead);
    if(!NT_SUCCESS(Status))
        return Status;

    if(0 != (BytesRead%nlfseGlobal.cbcBlockSize))
    {
        // data is not aligned with CBC Block Size (16 bytes), we need padding
        if(NULL==Padding || (nlfseGlobal.cbcBlockSize-(BytesRead%nlfseGlobal.cbcBlockSize)) != PaddingLength)
            return STATUS_INVALID_PARAMETER;

        RtlCopyMemory( (BlockData+BytesRead), Padding, PaddingLength );
        DecryptLength = BytesRead + PaddingLength;
    }

    // Decrypt data
    if(!decrypt_buffer(Key, NLSE_KEY_LENGTH_IN_BYTES, BlockOffset->QuadPart, BlockData, nlfseGlobal.cryptoBlockSize))
        return STATUS_UNSUCCESSFUL;

    if(BytesRead < nlfseGlobal.cryptoBlockSize)
    {
        // Set invalid data to ZERO
        RtlZeroMemory( (BlockData+BytesRead), (nlfseGlobal.cryptoBlockSize-BytesRead) );
    }

    if(BlockDataValidLength) *BlockDataValidLength = BytesRead;
    return STATUS_SUCCESS;
}

static
BOOLEAN
CalculatePaddingInEmptyBlock(
                             __in char* Key,
                             __in ULONGLONG BlockOffset,
                             __in ULONG PaddingOffset,
                             __out PUCHAR Padding,
                             __out PULONG PaddingLength
                             )
{
    BOOLEAN bRet    = FALSE;
    PUCHAR  pbBlock = NULL;

    pbBlock = ExAllocatePoolWithTag(NonPagedPool, nlfseGlobal.cryptoBlockSize, NLFSE_BUFFER_TAG);
    if(NULL == pbBlock)
        return FALSE;

    RtlZeroMemory(pbBlock, nlfseGlobal.cryptoBlockSize);
    encrypt_buffer(Key, NLSE_KEY_LENGTH_IN_BYTES, BlockOffset, pbBlock, nlfseGlobal.cryptoBlockSize);

    *PaddingLength = (nlfseGlobal.cbcBlockSize - (PaddingOffset%nlfseGlobal.cbcBlockSize))%nlfseGlobal.cbcBlockSize;
    RtlCopyMemory(Padding, pbBlock+PaddingOffset, *PaddingLength);
    ExFreePool(pbBlock);
    return TRUE;
}

//Check if the file is encrypted. If yes, return the pointer to 
//pre-post context; otherwise, return NULL
static PVOID 
NLFSEPreSetEndOfFile(__in PFLT_CALLBACK_DATA Data,
                     __in PCFLT_RELATED_OBJECTS FltObjects
                     )
{
    NTSTATUS                    Status  = STATUS_SUCCESS;
    BOOLEAN                     NeedCB  = FALSE;
    NLFSE_PVOLUME_CONTEXT       volCtx  = NULL;
    PPRETOPOST_CONTEXT          p2pCtx  = NULL;
    ULONG                       PaddingOffset = 0;
    ULONG                       ulWritten = 0;
    ULONGLONG                   BlockStartOffset = 0;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_ctx = {0};
    NLPERFORMANCE_COUNTER   pfc_getsize = {0};
    NLPERFORMANCE_COUNTER   pfc_calcpad = {0};
    PfStart(&pfc_total);
#endif

    p2pCtx = ExAllocatePoolWithTag(NonPagedPool, sizeof(PRETOPOST_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == p2pCtx)
        goto _exit;
    RtlZeroMemory(p2pCtx, sizeof(PRETOPOST_CONTEXT));

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_ctx);
#endif

    p2pCtx->ProcessId = FltGetRequestorProcessId(Data);
    p2pCtx->MajorFunction = IRP_MJ_SET_INFORMATION;

    //  Get our volume context.
    Status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx );
    if (!NT_SUCCESS(Status))
        goto _exit;
    p2pCtx->SectorSize = volCtx->SectorSize;

    //p2pCtx->psCtx = NLFSEFindExistingContext(volCtx, FltObjects->FileObject);
	Status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
										&p2pCtx->psCtx );
    if(!NT_SUCCESS(Status) || NULL ==  p2pCtx->psCtx)
        goto _exit;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_ctx);
    logPF = TRUE;
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_getsize);
#endif
    //get current file size
    Status = FOH_GET_FILE_SIZE_BY_HANDLE(FltObjects->Instance, FltObjects->FileObject, &(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet));
    if(!NT_SUCCESS(Status))
        goto _exit;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_getsize);
#endif

    //get new file size
    p2pCtx->Parameters.SetInformationFile.FileInformationClass = FileEndOfFileInformation;
    p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart = ((FILE_END_OF_FILE_INFORMATION *)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->EndOfFile.QuadPart;
    if(0 == p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart)
    {
        NeedCB = TRUE;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock = NULL;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = 0;
        RtlZeroMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, 16);
        goto _exit;
    }

    // File size is not changed, ignore this request
    if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart == p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart)
        goto _exit;


#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_calcpad);
#endif

    NeedCB = TRUE;
    if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart > p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart)
    {   // ** File size growing **//
        if(0 == (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart%nlfseGlobal.cryptoBlockSize))
        {   // Original file size is aligned with cryptoBlockSize
            // In this case, we don't need to update old file's last block, only need to fill empty file context
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart;
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = (ULONG)(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart - p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart);
            if(0 == (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength%nlfseGlobal.cryptoBlockSize))
            {
                // New file size is also aligned with 4K
                // Padding is ZERO
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = 0;
                RtlZeroMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, 16);
            }
            else
            {
                // We need to calculate padding
                BlockStartOffset = (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize;
                PaddingOffset = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength%nlfseGlobal.cryptoBlockSize;
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = (nlfseGlobal.cbcBlockSize - PaddingOffset%nlfseGlobal.cbcBlockSize)%nlfseGlobal.cbcBlockSize;
                CalculatePaddingInEmptyBlock(p2pCtx->psCtx->encryptExt->key, BlockStartOffset, PaddingOffset, p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength);
            }
        }
        else
        {
            // Original file size is not aligned with 4K
            // In this case, we need to re-encrypt last 4K block, and then
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart = (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize;
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock = ExAllocatePoolWithTag(NonPagedPool, nlfseGlobal.cryptoBlockSize, NLFSE_BUFFER_TAG);
            NLSEGetPlainBlock(FltObjects->Instance,
                FltObjects->FileObject,
                (Data->Iopb->IrpFlags&IRP_PAGING_IO)?TRUE:FALSE,
                &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset,
                p2pCtx->psCtx->encryptExt->paddingData,
                p2pCtx->psCtx->encryptExt->paddingLen,
                p2pCtx->psCtx->encryptExt->key,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock,
                &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockValidLength);
            PaddingOffset = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.QuadPart%nlfseGlobal.cryptoBlockSize;
            RtlZeroMemory((p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock+PaddingOffset), (nlfseGlobal.cryptoBlockSize-PaddingOffset));
            encrypt_buffer(p2pCtx->psCtx->encryptExt->key,
                NLSE_KEY_LENGTH_IN_BYTES,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock,
                nlfseGlobal.cryptoBlockSize);
            PaddingOffset = 0;
            if (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart > (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart + nlfseGlobal.cryptoBlockSize))
            {
                // Empty block is needed
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart + nlfseGlobal.cryptoBlockSize;
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = (ULONG)(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart - p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart);
                if(0 == (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength%nlfseGlobal.cryptoBlockSize))
                {
                    // Padding is ZERO
                    p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = 0;
                    RtlZeroMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, 16);
                }
                else
                {
                    // Need to calculate padding
                    BlockStartOffset = (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize;
                    PaddingOffset = (ULONG)(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength%nlfseGlobal.cryptoBlockSize);
                    p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = (nlfseGlobal.cbcBlockSize - PaddingOffset%nlfseGlobal.cbcBlockSize)%nlfseGlobal.cbcBlockSize;
                    CalculatePaddingInEmptyBlock(p2pCtx->psCtx->encryptExt->key, BlockStartOffset, PaddingOffset, p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength);
                }
            }
            else if (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart == (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart + nlfseGlobal.cryptoBlockSize))
            {
                // Don't need empty block
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = 0;
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = 0;
                // Padding is ZERO
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = 0;
                RtlZeroMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, 16);
            }
            else
            {
                // Don't need empty block
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = 0;
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = 0;
                // Need to calculate padding
                PaddingOffset = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart%nlfseGlobal.cryptoBlockSize;
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = (nlfseGlobal.cbcBlockSize - PaddingOffset%nlfseGlobal.cbcBlockSize)%nlfseGlobal.cbcBlockSize;
                RtlCopyMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData,
                    p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock+PaddingOffset,
                    p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength);
            }
        }
    }
    else
    {   // ** File size shrinking ** //
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart = 0;
        p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength = 0;
        if(0 == (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart%nlfseGlobal.cbcBlockSize))
        {
            // Padding is ZERO
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = 0;
            RtlZeroMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, 16);
        }
        else
        {
            // Read
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart = (p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize;
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock = ExAllocatePoolWithTag(NonPagedPool, nlfseGlobal.cryptoBlockSize, NLFSE_BUFFER_TAG);
            NLSEGetEncryptBlock(FltObjects->Instance,
                FltObjects->FileObject,
                (Data->Iopb->IrpFlags&IRP_PAGING_IO)?TRUE:FALSE,
                &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset,
                (ULONG)(p2pCtx->psCtx->encryptExt->fileRealLength - p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart),
                p2pCtx->psCtx->encryptExt->paddingData,
                p2pCtx->psCtx->encryptExt->paddingLen,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock,
                &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockValidLength);
            PaddingOffset = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart%nlfseGlobal.cryptoBlockSize;
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength = (nlfseGlobal.cbcBlockSize - PaddingOffset%nlfseGlobal.cbcBlockSize)%nlfseGlobal.cbcBlockSize;
            RtlCopyMemory(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock+PaddingOffset,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength);
            ExFreePoolWithTag(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock, NLFSE_BUFFER_TAG);
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock = NULL;
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset.QuadPart = 0;
        }
    }

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_calcpad);
#endif

_exit:
#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PreSetEndOfFile:  %d ms\n", pfc_total.diff.LowPart);
        if(NULL != p2pCtx)
        {
            DbgPrint("             Old Length:    0x%08X 0x%08X\n",
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.HighPart,
                p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeBeforeSet.LowPart);
        }
        DbgPrint("             New Length:    0x%08X 0x%08X\n",
            ((FILE_END_OF_FILE_INFORMATION *)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->EndOfFile.HighPart,
            ((FILE_END_OF_FILE_INFORMATION *)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->EndOfFile.LowPart);
        DbgPrint("             Get Context:   %d ms\n", pfc_ctx.diff.LowPart);
        DbgPrint("             Get File Size: %d ms\n", pfc_getsize.diff.LowPart);
        DbgPrint("             Calc Padding:  %d ms\n", pfc_calcpad.diff.LowPart);
    }
#endif
    if(volCtx)   FltReleaseContext(volCtx); volCtx=NULL;
    if(!NeedCB && NULL!=p2pCtx)
    {
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx);
        if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock) ExFreePoolWithTag(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock, NLFSE_BUFFER_TAG);
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG);
        p2pCtx = NULL;
    }
    return p2pCtx;
}/*NLFSEPreSetEndOfFile*/

/*************************************************************************
MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreRead(
                       __inout PFLT_CALLBACK_DATA Data,
                       __in_opt PCFLT_RELATED_OBJECTS FltObjects,
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
    PFLT_IO_PARAMETER_BLOCK   iopb = Data->Iopb;
    FLT_PREOP_CALLBACK_STATUS retValue = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PVOID                     newBuf = NULL;
    PMDL                      newMdl = NULL;
    NLFSE_PVOLUME_CONTEXT     volCtx = NULL;
    NLFSE_PINSTANCE_CONTEXT   instCtx = NULL;
    NLFSE_PPRE_2_POST_CONTEXT p2pCtx=NULL;
    NTSTATUS                  status;
    PNLFSE_STREAM_CONTEXT     pContext=NULL;
    ULONG                     readLen = iopb->Parameters.Read.Length;
    ULONG                     originalLen = readLen;
    LARGE_INTEGER             newOffset; //new read offset
    ULONG                     newSize; //new read size
    ULONG                     cryptoSize; //crypto size 


#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_ctx = {0};
    PfStart(&pfc_total);
#endif

    // Don't handle stream file
    if(NULL==FltObjects)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    try
    {
        //  If they are trying to read ZERO bytes, then don't do anything and
        //  we don't need a post-operation callback.
        if (readLen == 0)
            leave;

        //we only care about paging IO
        if (!(iopb->IrpFlags & (IRP_NOCACHE | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO)))
            leave;

#if NLSE_DEBUG_PERFORMANCE_COUNT
        PfStart(&pfc_ctx);
#endif
        //  Get our volume context.
        status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx );
        if (!NT_SUCCESS(status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreRead: Error getting volume context, status=%x\n", status );
            leave;
        }

        //NLFSEPrintFileNameIfEqual(Data, FltObjects, &nlfseTestFile);
        //pContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);
		status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pContext );
        if(!NT_SUCCESS(status) || pContext == NULL)
        {
            //we don't need a post-operation callback 
            //if the file is not what we were interested and
            //doesn't need to be encrypted
            leave;
        }

#if NLSE_DEBUG_PERFORMANCE_COUNT
        PfEnd(&pfc_ctx);
        logPF = TRUE;
#endif

        //There is per stream context and it needs to be handled. 

        //
        //  If this is a non-cached I/O we need to round the length up to the
        //  sector size for this device.  We must do this because the file
        //  systems do this and we need to make sure our buffer is as big
        //  as they are expecting.
        //
        if (FlagOn(IRP_NOCACHE,iopb->IrpFlags))
        {
            originalLen = (ULONG)ROUND_TO_SIZE(originalLen,volCtx->SectorSize);
        }

        //decide new read offset for CBC cipher block support
        //first, be aligned with cypher block size
        newOffset=iopb->Parameters.Read.ByteOffset;
        newOffset.QuadPart /= nlfseGlobal.cryptoBlockSize;
        newOffset.QuadPart *= nlfseGlobal.cryptoBlockSize;
        //second, be aligned with volume sector boundary
        newOffset.QuadPart /= volCtx->SectorSize;
        newOffset.QuadPart *= volCtx->SectorSize;    

        //decide read buffer size for CBC cipherjblock support
        //first, size of data started from read offset to the original read offset,
        //and plus the orignal data size
        newSize=(ULONG)(iopb->Parameters.Read.ByteOffset.QuadPart);
        newSize-=(ULONG)newOffset.QuadPart;
        newSize+=iopb->Parameters.Read.Length;
        //round up to cypher block size
        newSize=ROUND_TO_SIZE(newSize, nlfseGlobal.cryptoBlockSize);
        cryptoSize=newSize;
        //round the length up to the volume sector size
        newSize=ROUND_TO_SIZE(newSize, volCtx->SectorSize);
        readLen=newSize;

        //
        //  Allocate nonPaged memory for the buffer we are swapping to.
        //  If we fail to get the memory, just don't swap buffers on this
        //  operation.
        //
        newBuf = ExAllocatePoolWithTag( NonPagedPool, readLen, NLFSE_BUFFER_TAG );
        if (newBuf == NULL)
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreReadBuffers: Failed to allocate %d bytes\n", readLen);
            leave;
        }
        RtlZeroMemory(newBuf, readLen);

        //
        //  We only need to build a MDL for IRP operations.  We don't need to
        //  do this for a FASTIO operation since the FASTIO interface has no
        //  parameter for passing the MDL to the file system.
        //
        if (FlagOn(Data->Flags,FLTFL_CALLBACK_DATA_IRP_OPERATION))
        {
            //
            //  Allocate a MDL for the new allocated memory.  If we fail
            //  the MDL allocation then we won't swap buffer for this operation
            //
            newMdl = IoAllocateMdl( newBuf, readLen, FALSE, FALSE, NULL );
            if (newMdl == NULL)
            {
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreReadBuffers: Failed to allocate MDL\n");
                leave;
            }

            //
            //  setup the MDL for the non-paged pool we just allocated
            //
            MmBuildMdlForNonPagedPool( newMdl );
        }

        //Get the instance context.
        status = FltGetInstanceContext(FltObjects->Instance, &instCtx );
        if (!NT_SUCCESS( status ))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreReadBuffers: Failed to get instance context 0x%x\n", status);
            leave;
        }

        //  We are ready to swap buffers, get a pre2Post context structure.
        //  We need it to pass the volume context and the allocate memory
        //  buffer to the post operation callback.
        p2pCtx=ExAllocateFromNPagedLookasideList(&nlfseGlobal.pre2PostContextList);
        if (p2pCtx == NULL)
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreReadBuffers: Failed to allocate pre2Post context structure\n");
            leave;
        }
        RtlZeroMemory(p2pCtx, sizeof( NLFSE_PRE_2_POST_CONTEXT));

        //  Update the buffer pointers and MDL address, mark we have changed
        //  something.
        iopb->Parameters.Read.ReadBuffer = newBuf;
        iopb->Parameters.Read.MdlAddress = newMdl;
        iopb->Parameters.Read.ByteOffset = newOffset;
        iopb->Parameters.Read.Length = readLen;
        FltSetCallbackDataDirty( Data );

        //
        //
        //  Pass state to our post-operation callback.
        //
        p2pCtx->SwappedBuffer = newBuf;
        p2pCtx->VolCtx = volCtx;
        p2pCtx->streamCtx = pContext;
        p2pCtx->instanceCtx=instCtx;
        p2pCtx->newOffset = newOffset;
        p2pCtx->newSize = newSize;
        p2pCtx->cryptoSize = cryptoSize;
        p2pCtx->originalLen = originalLen;

        *CompletionContext = p2pCtx;

        //
        //  Return we want a post-operation callback
        //
        retValue = FLT_PREOP_SUCCESS_WITH_CALLBACK;

    }
    finally
    {
        //  If we don't want a post-operation callback, then cleanup state.
        if (retValue != FLT_PREOP_SUCCESS_WITH_CALLBACK)
        {
            if (newBuf != NULL)  ExFreePoolWithTag( newBuf, NLFSE_BUFFER_TAG );
            if (newMdl != NULL)  IoFreeMdl( newMdl );
            if (volCtx != NULL)  FltReleaseContext( volCtx );
            if (instCtx)         FltReleaseContext( instCtx );
            if(pContext != NULL) FltReleaseContext( pContext );
        }
#if NLSE_DEBUG_PERFORMANCE_COUNT
        if(logPF)
        {
            PfEnd(&pfc_total);
            DbgPrint("[NLPFLog] PreRead: %d ms\n", pfc_total.diff.LowPart);
            DbgPrint("             Read Offset:  0x%08X 0x%08X\n", Data->Iopb->Parameters.Read.ByteOffset.HighPart, Data->Iopb->Parameters.Read.ByteOffset.LowPart);
            DbgPrint("             Read Length:  %d bytes\n", Data->Iopb->Parameters.Read.Length);
            DbgPrint("             Get Context:  %d ms\n", pfc_ctx.diff.LowPart);
        }
#endif
    }

    return retValue;
} /*-- NLFSEOpCallbackPreRead --*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostRead(
                        __inout PFLT_CALLBACK_DATA Data,
                        __in PCFLT_RELATED_OBJECTS FltObjects,
                        __in PVOID CompletionContext,
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
    PVOID origBuf = NULL;
    PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;
    FLT_POSTOP_CALLBACK_STATUS retValue = FLT_POSTOP_FINISHED_PROCESSING;
    NLFSE_PPRE_2_POST_CONTEXT p2pCtx = CompletionContext;
    BOOLEAN cleanupAllocatedBuffer = TRUE;


#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_decrypt = {0};
    PfStart(&pfc_total);
#endif

    //
    //  This system won't draining an operation with swapped buffers, verify
    //  the draining flag is not set.
    //
    ASSERT(!FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING));

    try
    {
        //This is not the file that we are interested in
        if(p2pCtx == NULL)
            leave;

#if NLSE_DEBUG_PERFORMANCE_COUNT
        logPF = TRUE;
#endif
        //
        //  If the operation failed or the count is zero, there is no data to
        //  copy so just return now.
        //
        if (!NT_SUCCESS(Data->IoStatus.Status) || (Data->IoStatus.Information == 0))
            leave;

        //
        //  We need to copy the read data back into the users buffer.  Note
        //  that the parameters passed in are for the users original buffers
        //  not our swapped buffers.
        //
        if (iopb->Parameters.Read.MdlAddress != NULL)
        {
            //  There is a MDL defined for the original buffer, get a
            //  system address for it so we can copy the data back to it.
            //  We must do this because we don't know what thread context
            //  we are in.
            origBuf = MmGetSystemAddressForMdlSafe( iopb->Parameters.Read.MdlAddress, NormalPagePriority );
            if (origBuf == NULL)
            {
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PostReadBuffers: failed to get system address for MDL: %p\n", iopb->Parameters.Read.MdlAddress );
                //  If we failed to get a SYSTEM address, mark that the read
                //  failed and return.
                Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                Data->IoStatus.Information = 0;
                leave;
            }
        }
        else if (FlagOn(Data->Flags,FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) || FlagOn(Data->Flags,FLTFL_CALLBACK_DATA_FAST_IO_OPERATION))
        {
            //  If this is a system buffer, just use the given address because
            //      it is valid in all thread contexts.
            //  If this is a FASTIO operation, we can just use the
            //      buffer (inside a try/except) since we know we are in
            //      the correct thread context (you can't pend FASTIO's).
            origBuf = iopb->Parameters.Read.ReadBuffer;
        }
        else
        {
            //  They don't have a MDL and this is not a system buffer
            //  or a fastio so this is probably some arbitrary user
            //  buffer.  We can not do the processing at DPC level so
            //  try and get to a safe IRQL so we can do the processing.
            if (FltDoCompletionProcessingWhenSafe( Data,
                FltObjects,
                CompletionContext,
                Flags,
                NLSEPostReadDecryptionWhenSafe,
                &retValue ))
            {
                    //  This operation has been moved to a safe IRQL, the called
                    //  routine will do (or has done) the freeing so don't do it
                    //  in our routine.
                    cleanupAllocatedBuffer = FALSE;
            }
            else
            {
                //  We are in a state where we can not get to a safe IRQL and
                //  we do not have a MDL.  There is nothing we can do to safely
                //  copy the data back to the users buffer, fail the operation
                //  and return.  This shouldn't ever happen because in those
                //  situations where it is not safe to post, we should have
                //  a MDL.
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PostReadBuffers: Unable to post to a safe IRQL\n");
                Data->IoStatus.Status = STATUS_UNSUCCESSFUL;
                Data->IoStatus.Information = 0;
            }
            leave;
        }

        //  We either have a system buffer or this is a fastio operation
        //  so we are in the proper context.  Do decryption and copy the data 
        //  back to the original buffer handling an exception.
        try
        {
            //set really read in data size
            p2pCtx->realReadSize=(ULONG)Data->IoStatus.Information;

            //reset to the orignal read length
            if( Data->IoStatus.Information > p2pCtx->originalLen)
                Data->IoStatus.Information = p2pCtx->originalLen;

#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfStart(&pfc_decrypt);
#endif
            // decrypt
            if(!NLSEDecryptionAtPostRead(Data, 
                FltObjects, 
                p2pCtx,
                origBuf, 
                p2pCtx->SwappedBuffer))
            {
#if NLSE_DEBUG_PERFORMANCE_COUNT
                PfEnd(&pfc_decrypt);
#endif
                Data->IoStatus.Status = STATUS_UNSUCCESSFUL;
                Data->IoStatus.Information = 0;
                leave;
            }
#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfEnd(&pfc_decrypt);
#endif
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            //  The copy failed, return an error, failing the operation.
            Data->IoStatus.Status = GetExceptionCode();
            Data->IoStatus.Information = 0;
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PostReadBuffers: Invalid user buffer, oldB=%p, status=%x\n", origBuf, Data->IoStatus.Status);
        }
    }
    finally
    {
        //cleaning up
        if(cleanupAllocatedBuffer)
        {
            if(p2pCtx)
            {
                if(p2pCtx->SwappedBuffer) ExFreePoolWithTag( p2pCtx->SwappedBuffer, NLFSE_BUFFER_TAG );
                if(p2pCtx->VolCtx)        FltReleaseContext( p2pCtx->VolCtx );
                if(p2pCtx->streamCtx)     FltReleaseContext( p2pCtx->streamCtx );
                if(p2pCtx->instanceCtx)   FltReleaseContext(p2pCtx->instanceCtx);
                ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList, p2pCtx );
            }
        }
#if NLSE_DEBUG_PERFORMANCE_COUNT
        if(logPF)
        {
            PfEnd(&pfc_total);
            DbgPrint("[NLPFLog] PostRead: %d ms\n", pfc_total.diff.LowPart);
            DbgPrint("             Read Length: %d bytes\n", (ULONG)(Data->IoStatus.Information));
            DbgPrint("             Decrypt: %d ms\n", pfc_decrypt.diff.LowPart);
        }
#endif
    }

    return retValue;
}/*--NLFSEOpCallbackPostRead--*/

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
    FLT_PREOP_CALLBACK_STATUS retStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    NTSTATUS                  Status    = STATUS_SUCCESS;
    PPRETOPOST_CONTEXT        p2pCtx    = NULL;
    NLFSE_PVOLUME_CONTEXT     volCtx    = NULL;
    PNLFSE_STREAM_CONTEXT     psCtx     = NULL;
    PUCHAR                    pbOrigWriteBuffer = NULL;
    BOOLEAN                   IsBufferedIO = FALSE;
    BOOLEAN                   IsFastIO     = FALSE;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_ctx = {0};
    NLPERFORMANCE_COUNTER   pfc_read = {0};
    NLPERFORMANCE_COUNTER   pfc_encrypt = {0};
    PfStart(&pfc_total);
#endif


    // Sanity check
    ASSERT((NULL!=FltObjects));

    // Don't handle ZERO byte write
    if (0 == Data->Iopb->Parameters.Write.Length)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    IsBufferedIO = (Data->Iopb->IrpFlags &  (IRP_NOCACHE | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))?FALSE:TRUE;
    IsFastIO     = FLT_IS_FASTIO_OPERATION(Data);

    // Ignore cached I/O if it is not fast I/O
    // fast I/O is only be denied for encrypted file (so the driver needs to get per-stream context)
    if(IsBufferedIO && !IsFastIO)
        goto _exit;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_ctx);
#endif

    // Try to get volume context
    Status = FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &volCtx);
    if (!NT_SUCCESS(Status))
    {
        volCtx = NULL;
        goto _exit;
    }

    // Try to get per-stream context
    // If fail to find per-stream context, don't handle this IRP
    //psCtx = NLFSEFindExistingContext(volCtx, FltObjects->FileObject);
	Status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &psCtx );
    if(!NT_SUCCESS(Status) || NULL == psCtx)
        goto _exit;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_ctx);
    logPF = TRUE;
#endif


    if(IsFastIO)
    {
        Data->IoStatus.Information = 0;
        retStatus = FLT_PREOP_DISALLOW_FASTIO;
        goto _exit;
    }

    // Only handle non-cached/paging I/O
    if (IsBufferedIO)
        goto _exit;

    if(psCtx->IgnoreWrite)
        goto _exit;

    // Allocate PRE TO POST buffer
    p2pCtx = ExAllocatePoolWithTag(NonPagedPool, sizeof(PRETOPOST_CONTEXT), NLFSE_BUFFER_TAG);
    if(NULL == p2pCtx)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreWrite: Failed to allocate pre2Post context structure\n");
        goto _exit;
    }
    RtlZeroMemory(p2pCtx, sizeof(PRETOPOST_CONTEXT));
    p2pCtx->SectorSize = volCtx->SectorSize;
    p2pCtx->psCtx      = psCtx; psCtx = NULL;
    p2pCtx->ProcessId  = FltGetRequestorProcessId(Data);
    p2pCtx->MajorFunction = Data->Iopb->MajorFunction;
    p2pCtx->MinorFunction = Data->Iopb->MinorFunction;
    p2pCtx->Parameters.Write.OriginalOffset.QuadPart = Data->Iopb->Parameters.Write.ByteOffset.QuadPart;
    p2pCtx->Parameters.Write.OriginalLength          = ROUND_TO_SIZE(Data->Iopb->Parameters.Write.Length, p2pCtx->SectorSize);

    // Build new MDL and write buffer
    p2pCtx->Parameters.Write.NewOffset.QuadPart = ROUND_TO_SIZE_PRE(Data->Iopb->Parameters.Write.ByteOffset.QuadPart, nlfseGlobal.cryptoBlockSize);
    p2pCtx->Parameters.Write.NewLength          = (ULONG)(Data->Iopb->Parameters.Write.ByteOffset.QuadPart - p2pCtx->Parameters.Write.NewOffset.QuadPart) + p2pCtx->Parameters.Write.OriginalLength;
    // Round to sector size
    p2pCtx->Parameters.Write.NewLength          = ROUND_TO_SIZE(p2pCtx->Parameters.Write.NewLength, p2pCtx->SectorSize);
    // Round to crypto block size
    p2pCtx->Parameters.Write.NewLength          = ROUND_TO_SIZE(p2pCtx->Parameters.Write.NewLength, nlfseGlobal.cryptoBlockSize);
    p2pCtx->Parameters.Write.SwapBuffer = ExAllocatePoolWithTag(NonPagedPool, p2pCtx->Parameters.Write.NewLength, NLFSE_BUFFER_TAG);
    if(NULL == p2pCtx->Parameters.Write.SwapBuffer)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreWrite: Failed to allocate pre2Post swap buffer\n");
        goto _exit;
    }
    RtlZeroMemory(p2pCtx->Parameters.Write.SwapBuffer, p2pCtx->Parameters.Write.NewLength);

    // Get current file size
    //FOH_GET_FILE_SIZE_VIA_SYNCIO(FltObjects->Instance, FltObjects->FileObject, &p2pCtx->Parameters.Write.FSizeBeforeWrite);
    p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart = p2pCtx->psCtx->encryptExt->fileRealLength;

    //  We only need to build a MDL for IRP operations.  We don't need to
    //  do this for a FASTIO operation because it is a waste of time since
    //  the FASTIO interface has no parameter for passing the MDL to the
    //  file system.
    if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_IRP_OPERATION))
    {
        try
        {
            //  Allocate a MDL for the new allocated memory.  If we fail
            //  the MDL allocation then we won't swap buffer for this operation
            p2pCtx->Parameters.Write.SwapMdl = IoAllocateMdl(p2pCtx->Parameters.Write.SwapBuffer, p2pCtx->Parameters.Write.NewLength, FALSE, FALSE, NULL);
            if(NULL != p2pCtx->Parameters.Write.SwapMdl)
            {
                //  setup the MDL for the non-paged pool we just allocated
                MmBuildMdlForNonPagedPool(p2pCtx->Parameters.Write.SwapMdl);
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Data->IoStatus.Status      = GetExceptionCode();
            Data->IoStatus.Information = 0;
            retStatus                  = FLT_PREOP_COMPLETE;
            goto _exit;
        }
        if(NULL == p2pCtx->Parameters.Write.SwapMdl)
        {
            Data->IoStatus.Status      = STATUS_INSUFFICIENT_RESOURCES;
            Data->IoStatus.Information = 0;
            retStatus = FLT_PREOP_COMPLETE;
            goto _exit;
        }
    }

    // Get original write buffer
    try
    {
        if (Data->Iopb->Parameters.Write.MdlAddress != NULL)
            pbOrigWriteBuffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Write.MdlAddress, NormalPagePriority );
        else  //  There was no MDL defined, use the given buffer address.          
            pbOrigWriteBuffer = Data->Iopb->Parameters.Write.WriteBuffer;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        //  The encryption failed, return an error, failing the operation.
        Data->IoStatus.Status      = GetExceptionCode();
        Data->IoStatus.Information = 0;
        retStatus                  = FLT_PREOP_COMPLETE;
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreWrite: Invalid user buffer, oldB=%p,status=%x\n", pbOrigWriteBuffer, Data->IoStatus.Status);
        goto _exit;
    }
    if(NULL == pbOrigWriteBuffer)
    {
        Data->IoStatus.Status      = STATUS_INSUFFICIENT_RESOURCES;
        Data->IoStatus.Information = 0;
        retStatus = FLT_PREOP_COMPLETE;
        goto _exit;
    }

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_read);
#endif
    // Reset write data
    // Get plain block
    if(p2pCtx->Parameters.Write.NewLength > nlfseGlobal.cryptoBlockSize)
    {
        // More than one crypto block
        // Check if the first block should be updated
        if(p2pCtx->Parameters.Write.OriginalOffset.QuadPart != p2pCtx->Parameters.Write.NewOffset.QuadPart)
        {
            // We need to update first block
            Status = NLSEGetPlainBlock(FltObjects->Instance,
                FltObjects->FileObject,
                (Data->Iopb->IrpFlags&IRP_PAGING_IO)?TRUE:FALSE,
                &p2pCtx->Parameters.Write.NewOffset,
                p2pCtx->psCtx->encryptExt->paddingData,
                p2pCtx->psCtx->encryptExt->paddingLen,
                p2pCtx->psCtx->encryptExt->key,
                p2pCtx->Parameters.Write.SwapBuffer,
                NULL);
        }

        // Check if the last block should be updated
        if( (p2pCtx->Parameters.Write.OriginalOffset.QuadPart+p2pCtx->Parameters.Write.OriginalLength) != (p2pCtx->Parameters.Write.NewOffset.QuadPart+p2pCtx->Parameters.Write.NewLength))
        {
            // We need to update last block
            Status = NLSEGetPlainBlock(FltObjects->Instance,
                FltObjects->FileObject,
                (Data->Iopb->IrpFlags&IRP_PAGING_IO)?TRUE:FALSE,
                &p2pCtx->Parameters.Write.NewOffset,
                p2pCtx->psCtx->encryptExt->paddingData,
                p2pCtx->psCtx->encryptExt->paddingLen,
                p2pCtx->psCtx->encryptExt->key,
                p2pCtx->Parameters.Write.SwapBuffer+(p2pCtx->Parameters.Write.NewLength-nlfseGlobal.cryptoBlockSize),
                NULL);
        }
    }
    else
    {
        // One crypto block
        // Just update this block
        Status = NLSEGetPlainBlock(FltObjects->Instance,
            FltObjects->FileObject,
            (Data->Iopb->IrpFlags&IRP_PAGING_IO)?TRUE:FALSE,
            &p2pCtx->Parameters.Write.NewOffset,
            p2pCtx->psCtx->encryptExt->paddingData,
            p2pCtx->psCtx->encryptExt->paddingLen,
            p2pCtx->psCtx->encryptExt->key,
            p2pCtx->Parameters.Write.SwapBuffer,
            NULL);
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_read);
    PfStart(&pfc_encrypt);
#endif
    RtlCopyMemory((p2pCtx->Parameters.Write.SwapBuffer+(p2pCtx->Parameters.Write.OriginalOffset.QuadPart - p2pCtx->Parameters.Write.NewOffset.QuadPart)),
        pbOrigWriteBuffer,
        p2pCtx->Parameters.Write.OriginalLength);
    encrypt_buffer(p2pCtx->psCtx->encryptExt->key,
        NLSE_KEY_LENGTH_IN_BYTES,
        p2pCtx->Parameters.Write.NewOffset.QuadPart,
        p2pCtx->Parameters.Write.SwapBuffer,
        p2pCtx->Parameters.Write.NewLength);

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_encrypt);
#endif

    // Re-calculate write length
    p2pCtx->Parameters.Write.NewWriteLength = p2pCtx->Parameters.Write.NewLength;
    if((p2pCtx->Parameters.Write.OriginalOffset.QuadPart+p2pCtx->Parameters.Write.OriginalLength) > p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart)
    {
        // File is growing
        if((p2pCtx->Parameters.Write.NewOffset.QuadPart+p2pCtx->Parameters.Write.NewLength)
            > (p2pCtx->Parameters.Write.OriginalOffset.QuadPart+p2pCtx->Parameters.Write.OriginalLength))
            p2pCtx->Parameters.Write.NewWriteLength = (ULONG)(p2pCtx->Parameters.Write.OriginalOffset.QuadPart
                                                              + p2pCtx->Parameters.Write.OriginalLength
                                                              - p2pCtx->Parameters.Write.NewOffset.QuadPart);
    }
    else
    {
        // File size is not growing
        if((p2pCtx->Parameters.Write.NewOffset.QuadPart+p2pCtx->Parameters.Write.NewLength) > p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart)
        {
            p2pCtx->Parameters.Write.NewWriteLength = (ULONG)(p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart - p2pCtx->Parameters.Write.NewOffset.QuadPart);
            p2pCtx->Parameters.Write.NewWriteLength = ROUND_TO_SIZE(p2pCtx->Parameters.Write.NewWriteLength, p2pCtx->SectorSize);
        }
    }

    ASSERT( 0 == (p2pCtx->Parameters.Write.NewWriteLength%p2pCtx->SectorSize) );

    // OKay, all the preparation has been done
    // Swap buffer, and let file system write the data
    Data->Iopb->Parameters.Write.WriteBuffer = p2pCtx->Parameters.Write.SwapBuffer;
    Data->Iopb->Parameters.Write.MdlAddress  = p2pCtx->Parameters.Write.SwapMdl;
    Data->Iopb->Parameters.Write.ByteOffset.QuadPart  = p2pCtx->Parameters.Write.NewOffset.QuadPart;
    Data->Iopb->Parameters.Write.Length               = p2pCtx->Parameters.Write.NewWriteLength;
    FltSetCallbackDataDirty(Data);
    *CompletionContext = p2pCtx; p2pCtx=NULL;
    retStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

_exit:
    if(volCtx != NULL) FltReleaseContext(volCtx);    volCtx=NULL;
    if(psCtx != NULL)  FltReleaseContext( psCtx ); psCtx =NULL;
    // Free p2pCtx
    if(NULL != p2pCtx)
    {
        if(p2pCtx->Parameters.Write.SwapBuffer!=NULL) ExFreePoolWithTag(p2pCtx->Parameters.Write.SwapBuffer, NLFSE_BUFFER_TAG);
        if(p2pCtx->Parameters.Write.SwapMdl!=NULL) IoFreeMdl(p2pCtx->Parameters.Write.SwapMdl);
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx); p2pCtx->psCtx=NULL;
        RtlZeroMemory(p2pCtx, sizeof(PRETOPOST_CONTEXT));
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG); p2pCtx=NULL;
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PreWrite: %d ms\n", pfc_total.diff.LowPart);
        DbgPrint("             Write Offset: 0x%08X 0x%08X\n", Data->Iopb->Parameters.Write.ByteOffset.HighPart, Data->Iopb->Parameters.Write.ByteOffset.LowPart);
        DbgPrint("             Write Length: %d bytes\n", Data->Iopb->Parameters.Write.Length);
        DbgPrint("             Get Context:  %d ms\n", pfc_ctx.diff.LowPart);
        DbgPrint("             Read Data:    %d ms\n", pfc_read.diff.LowPart);
        DbgPrint("             Encrypt Data: %d ms\n", pfc_encrypt.diff.LowPart);
    }
#endif
    return retStatus;
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
    NTSTATUS                Status    = STATUS_SUCCESS;
    PPRETOPOST_CONTEXT      p2pCtx    = (PPRETOPOST_CONTEXT)CompletionContext;
    PNLFSE_ADS_WORKITEM     workItem  = NULL;
    KIRQL                   OldIRQL;
    LARGE_INTEGER           FinalWriteEnd  = {0};
    ULONG                   FinalWrittenLength = 0;
    ULONG                   PaddingOffset = 0;
    ULONG                   PaddingLength = 0;
    UCHAR                   PaddingData[NLSE_PADDING_DATA_LEN] = {0};
    BOOLEAN                 NeedUpdateAds = FALSE;
    BOOLEAN                 FileSizeGrow  = FALSE;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_update_pad = {0};
    NLPERFORMANCE_COUNTER   pfc_queue_pad = {0};
    PfStart(&pfc_total);
#endif


    UNREFERENCED_PARAMETER( Flags );

    // No context
    if(NULL == p2pCtx)
        goto _exit;
    // Write fail or Zero byte is written
    if(!NT_SUCCESS(Data->IoStatus.Status)
        || 0==Data->IoStatus.Information)
        goto _exit;

    FinalWrittenLength = (ULONG)Data->IoStatus.Information;
    FinalWriteEnd.QuadPart = p2pCtx->Parameters.Write.NewOffset.QuadPart + FinalWrittenLength;
    FileSizeGrow = (FinalWriteEnd.QuadPart > p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart)?TRUE:FALSE;
    if(FileSizeGrow)
        p2pCtx->psCtx->encryptExt->fileRealLength = FinalWriteEnd.QuadPart;

    if(FinalWriteEnd.QuadPart <= p2pCtx->Parameters.Write.OriginalOffset.QuadPart)
    {
        // Actually, no data is written
        Data->IoStatus.Information = 0;
        goto _exit;
    }

    // Recalculate write length return to App
    Data->IoStatus.Information = min((ULONG_PTR)(FinalWriteEnd.QuadPart - p2pCtx->Parameters.Write.OriginalOffset.QuadPart), p2pCtx->Parameters.Write.OriginalLength);

#if NLSE_DEBUG_PERFORMANCE_COUNT
    logPF = TRUE;
    PfStart(&pfc_update_pad);
#endif

    // Calculate padding
    if(FinalWriteEnd.QuadPart > ((p2pCtx->Parameters.Write.FSizeBeforeWrite.QuadPart/nlfseGlobal.cryptoBlockSize)*nlfseGlobal.cryptoBlockSize))
    {
        // Padding is changed, recalculate
        if(0 == (FinalWriteEnd.QuadPart%nlfseGlobal.cbcBlockSize))
        {
            PaddingLength = 0;
        }
        else
        {
            PaddingOffset = FinalWrittenLength;
            PaddingLength = ROUND_TO_SIZE_PADDING(FinalWrittenLength, nlfseGlobal.cbcBlockSize);
            RtlCopyMemory(PaddingData, p2pCtx->Parameters.Write.SwapBuffer+PaddingOffset, PaddingLength);
        }

        KeAcquireSpinLock(&p2pCtx->psCtx->encryptExtLock, &OldIRQL);
        if(p2pCtx->psCtx->encryptExt->paddingLen!=PaddingLength
            || (0!=PaddingLength && PaddingLength!=RtlCompareMemory(PaddingData, p2pCtx->psCtx->encryptExt->paddingData, PaddingLength))
            )
        {
            p2pCtx->psCtx->encryptExt->paddingLen = PaddingLength;
            RtlCopyMemory((PUCHAR)p2pCtx->psCtx->encryptExt->paddingData, PaddingData, NLSE_PADDING_DATA_LEN);
            NeedUpdateAds = TRUE;
        }
        KeReleaseSpinLock(&p2pCtx->psCtx->encryptExtLock, OldIRQL);
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_update_pad);
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_queue_pad);
#endif
    if(NeedUpdateAds)
    {
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
            workItem->ProcessId                 = p2pCtx->ProcessId;
            workItem->SectorSize                = p2pCtx->SectorSize;
            workItem->hostFilePath.MaximumLength= sizeof(WCHAR)*(2*MAX_PATH);
            workItem->hostFilePath.Length       = p2pCtx->psCtx->FileName.Length;
            workItem->hostFilePath.Buffer       = &(workItem->hostFilePathBuffer[0]);
            RtlCopyMemory(workItem->hostFilePathBuffer, p2pCtx->psCtx->FileName.Buffer, p2pCtx->psCtx->FileName.Length);
            RtlCopyMemory(&workItem->encryptExt, p2pCtx->psCtx->encryptExt, sizeof(NLFSE_ENCRYPT_EXTENSION));
            NLSEQueueAdsWorkItem(workItem); workItem=NULL;
        }
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_queue_pad);
#endif

_exit:
    if(NULL != workItem) ExFreePoolWithTag(workItem, NLFSE_BUFFER_TAG);
    if(NULL != p2pCtx)
    {
        if(p2pCtx->Parameters.Write.SwapBuffer!=NULL) ExFreePoolWithTag(p2pCtx->Parameters.Write.SwapBuffer, NLFSE_BUFFER_TAG);
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx); p2pCtx->psCtx=NULL;
        RtlZeroMemory(p2pCtx, sizeof(PRETOPOST_CONTEXT));
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG); p2pCtx=NULL;
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PostWrite: %d ms\n", pfc_total.diff.LowPart);
        DbgPrint("             Written Length: %d bytes\n", FinalWrittenLength);
        DbgPrint("             Update Padding: %d ms\n", pfc_update_pad.diff.LowPart);
        DbgPrint("             Queue WkItem:   %d ms\n", pfc_queue_pad.diff.LowPart);
    }
#endif
    return FLT_POSTOP_FINISHED_PROCESSING;
}/*--NLFSEOpCallbackPostWrite--*/

VOID
NLSEPendingPreCreateIOWorkItemRoutine(
                                      __in PFLT_GENERIC_WORKITEM WorkItem,
                                      __in PFLT_FILTER Filter,
                                      __in PVOID Context
                                      )
/*++                                                      
Routine Description:

This WorkItem routine is called in the system thread context to process
the pended pre-create I/O in this mini filter's cancel safe queue. 
The first I/O in the queue is processed and then we return

Arguments:
WorkItem - Unused.
Filter - Unused.
Context - Context information.

Return Value:
None.
--*/
{
    PFLT_CALLBACK_DATA Data = (PFLT_CALLBACK_DATA)Context;
    NLSE_PENDING_IO_QUEUE_PCONTEXT queueCtx;
    NLFSE_PIRP_ENTRY irpEntry=NULL;
    NTSTATUS Status;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    PfStart(&pfc_total);
#endif

    __try
    {
        UNREFERENCED_PARAMETER( WorkItem );
        UNREFERENCED_PARAMETER( Filter );

        if (Data)
        {
            queueCtx = (NLSE_PENDING_IO_QUEUE_PCONTEXT) Data->QueueContext[0];

#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfStart(&pfc_total);
#endif
            NLSEHandlePendedIO(queueCtx, Data);	
#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfEnd(&pfc_total);
            DbgPrint("[NLPFLog] PreCreateEvaluation: %d ms\n", pfc_total.diff.LowPart);
#endif
            
            //  Free the extra storage that was allocated for this I/O.
            ExFreeToNPagedLookasideList( &nlfseGlobal.queueContextLookaside, queueCtx );
        }

        //  Clean up
        FltFreeGenericWorkItem(WorkItem);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
}/*--NLSEPendingPreCreateIOWorkItemRoutine--*/

NTSTATUS
NLSEPreCreateAndPostWorkItem(__in PFLT_CALLBACK_DATA Data
                             )
/*++
Routine Description:
PreCreate calls this routine to create and insert a work item to
be processed 

Arguments:
Data - Supplies the callback data for the operation that is being
inserted into the queue.

Return Value:
STATUS_SUCCESS if the function completes successfully.  Otherwise a valid
NTSTATUS code is returned.
--*/
{
    PFLT_GENERIC_WORKITEM WorkItem = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    //  Queue a work item 
    WorkItem = FltAllocateGenericWorkItem();
    
    if (WorkItem)
    {
        Status = FltQueueGenericWorkItem( WorkItem,
                                          nlfseGlobal.filterHandle,
                                          NLSEPendingPreCreateIOWorkItemRoutine,
                                          DelayedWorkQueue,
                                          Data );
        
        if (!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "[NLSE!NLSEPreCreateAndPostWorkItem: Failed to queue the work item (Status = 0x%x)\n", Status);
            FltFreeGenericWorkItem( WorkItem );
        }
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return Status;
}/*--NLSEPreCreateAndPostWorkItem--*/

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
BOOLEAN
NLSEIsNxlFile(
              __in PCUNICODE_STRING FileName
              )
{
    UNICODE_STRING NxlSuffix;
    UNICODE_STRING InSuffix;

    RtlInitUnicodeString(&NxlSuffix, L".nxl");
    if(FileName->Length < NxlSuffix.Length)
        return FALSE;

    InSuffix.Length         = NxlSuffix.Length;
    InSuffix.MaximumLength  = NxlSuffix.Length;
    InSuffix.Buffer         = FileName->Buffer + ((FileName->Length - NxlSuffix.Length)/sizeof(WCHAR));

    if(0 == RtlCompareUnicodeString(&NxlSuffix, &InSuffix, TRUE))
        return TRUE;

    return FALSE;
}


NTSTATUS
IsFileUnderDRMDirectory (
                         __in PFLT_CALLBACK_DATA Data,
                         __in PFLT_FILTER Filter,
                         __in PFLT_VOLUME Volume,
                         __out PBOOLEAN IsDRM
                         )
{
    NTSTATUS                    Status;
    PFLT_FILE_NAME_INFORMATION  FileNameInfo;
    UNICODE_STRING              Path;
    NLFSE_PVOLUME_CONTEXT       VolumeContext;
    WCHAR                       DriveLetter;


    *IsDRM = FALSE;


    //  Get volume's drive letter
    Status = FltGetVolumeContext(Filter, Volume, &VolumeContext);
    if(!NT_SUCCESS(Status))
        return Status;

    DriveLetter = VolumeContext->Name.Buffer[0];
    FltReleaseContext(VolumeContext);

    // Get file name
    Status = FltGetFileNameInformation(Data,
                                       FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT,
                                       &FileNameInfo);
    if(!NT_SUCCESS(Status))
        return Status;

    try {

        Status = FltParseFileNameInformation(FileNameInfo);
        if(!NT_SUCCESS(Status))
            goto try_exit;

        if( 0 != FileNameInfo->Share.Length ||      // File on remote share
            0 == FileNameInfo->Volume.Length ||     // File not on local volume
            0 != FileNameInfo->Stream.Length)       // ADS
            goto try_exit;

        //
        //  Get path without volume name
        //
        Path.Length = FileNameInfo->Name.Length - FileNameInfo->Volume.Length;
        Path.Buffer = FileNameInfo->Name.Buffer + FileNameInfo->Volume.Length / sizeof(WCHAR);
        Path.MaximumLength = Path.Length;

        Status = NLSEInDRMDirectory (DriveLetter, &Path, IsDRM);

try_exit:
        NOTHING;
    }
    finally {

        FltReleaseFileNameInformation(FileNameInfo);
        FileNameInfo = NULL;
    }

    return Status;
}

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreCreate (__inout PFLT_CALLBACK_DATA Data,
                          __in PCFLT_RELATED_OBJECTS FltObjects,
                          __deref_out_opt PVOID *CompletionContext)
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
    PIO_SECURITY_CONTEXT      securityCtx=NULL;
    FLT_PREOP_CALLBACK_STATUS returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK; 
    UNICODE_STRING            nameToUse; //operated file's name
    UNICODE_STRING            parentDir; //operated file's path
    UNICODE_STRING            volName;   //operated file's volume name
    NLFSE_PIRP_ENTRY          irpEntry = NULL;  //IRP entry passed post op
    PNLFSE_STREAM_CONTEXT     pContext=NULL;  
    NLFSE_PVOLUME_CONTEXT     volCtx = NULL;
    BOOLEAN                   bNotFile; //If true, the operated object not a file
    BOOLEAN                   bWinEncrypted = FALSE;
    BOOLEAN                   bWinCompressed= FALSE;
    NTSTATUS                  status;
    BOOLEAN                   bNtfsReservedFile = FALSE;
    ULONG                     CreateOptions     = 0;
    ULONG                     CreateDisposition = 0;
    BOOLEAN                   bOverwrite        = FALSE;
    BOOLEAN			  CreateWithEFS_Compress = FALSE;
    BOOLEAN                   bChkFileExist     = FALSE;
    ULONG                     pid = 0;
    BOOLEAN                   IsDRM;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_get_filename = {0};
    NLPERFORMANCE_COUNTER   pfc_chk_rsvfile = {0};
    NLPERFORMANCE_COUNTER   pfc_chk_exist = {0};
    NLPERFORMANCE_COUNTER   pfc_get_fsize = {0};
    NLPERFORMANCE_COUNTER   pfc_chk_enc = {0};
    PfStart(&pfc_total);
#endif

    /* Determine if the log file must be set. */
    if( nlseLoggingFlags != 0 && nlfseGlobal.log_file_set == FALSE )
    {
        ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
        if( nlfseGlobal.log_file_set == FALSE )
        {
            UNICODE_STRING log_file;
            nlfseGlobal.log_file_set = TRUE;
            RtlInitUnicodeString(&log_file,L"\\??\\C:\\Program Files\\NextLabs\\System Encryption\\diags\\logs\\sefw.klog");
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

    /************************************************************************/
    // Pre-check create parameters
    /************************************************************************/
    CreateOptions     = Data->Iopb->Parameters.Create.Options & 0x00FFFFFF;
    CreateDisposition = (Data->Iopb->Parameters.Create.Options & 0xFF000000) >> 24;
    if (FILE_CREATE == CreateDisposition)
    {
        // If it is a new create file with EFS/Compress flag, we don't need to handle it
        if(Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED
            || Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_COMPRESSED)
		{
			CreateWithEFS_Compress = TRUE;
		}
        //return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }
    // Don't handle directory
    if(CreateOptions & FILE_DIRECTORY_FILE
        || Data->Iopb->Parameters.Create.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    //Initialization
    RtlZeroMemory(&nameToUse,sizeof(UNICODE_STRING));
    RtlZeroMemory(&parentDir,sizeof(UNICODE_STRING));
    RtlZeroMemory(&volName,sizeof(UNICODE_STRING));
    securityCtx=Data->Iopb->Parameters.Create.SecurityContext;


    //
    //  Sigh! I don't know where to add the check for white list
    //  So just add to here temporarily 
    //
    status = IsFileUnderDRMDirectory ( Data,
                                       FltObjects->Filter,
                                       FltObjects->Volume,
                                       &IsDRM
                                       );

    if(!NT_SUCCESS(status) || !IsDRM)
    {
        //
        //  Since we use whitelist here,
        //  any file not in whitelist directories will not be handled
        //
        goto   NLSEPreCreateOperationCallback_EXIT;
    }


#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_get_filename);
#endif
    if(!NLFSEAllocateAndGetFileName(Data, 
        FltObjects,
        &nameToUse, 
        &parentDir,
        &volName) ||
        !nameToUse.Buffer || 
        !nameToUse.Length)
    {
        //failed to get file name and path; do nothing
        goto   NLSEPreCreateOperationCallback_EXIT;
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_get_filename);
#endif

    if(NLSEIsNxlFile(&nameToUse))
    {
        // Don't Handle NXL file
        goto   NLSEPreCreateOperationCallback_EXIT;
    }

    if(RtlPrefixUnicodeString(&NextLabsDir, &parentDir, TRUE) || RtlPrefixUnicodeString(&WindowsDir, &parentDir, TRUE))
    {
        // Don't handle file under NextLabs/Windows directory
        goto   NLSEPreCreateOperationCallback_EXIT;
    }

#if defined(_WIN64)
    if(RtlPrefixUnicodeString(&NextLabsDir86, &parentDir, TRUE))
    {
        // Don't handle file under NextLabs/Windows directory
        goto   NLSEPreCreateOperationCallback_EXIT;
    }
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_chk_rsvfile);
#endif
    bNtfsReservedFile = IsNTFSReservedFile(&parentDir, &nameToUse);
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_chk_rsvfile);
#endif
#define SHOW_SPECIAL_FILE 0
#if SHOW_SPECIAL_FILE
    /************************************************************************/
    /* This debug code is to show all special files                         */
    /************************************************************************/
    if(NULL!=nameToUse.Buffer
        && 0 != nameToUse.Length
        && L'$' == nameToUse.Buffer[0])
    {
        DbgPrint("CreateFile: %wZ%wZ\n", &parentDir, &nameToUse);
        DbgPrint("    Reserved: %s\n", bNtfsReservedFile?"Yes":"No");
        DbgPrint("    Volume:   %wZ\n", &volName);
        DbgPrint("    Parent:   %wZ\n", &parentDir);
        DbgPrint("    FileName: %wZ\n", &nameToUse);
    }
#endif
    // Don't handle NTFS Reserved File
    if(bNtfsReservedFile)
    {
        goto NLSEPreCreateOperationCallback_EXIT;
    }

    /* If the Policy Controller is down and the given file resides in a DRM (encrypted)
     * directory, then deny access to the file.
     */
    if( nlfseGlobal.bEnable == FALSE )
    {
      BOOLEAN enc_dir = FALSE;
      PFLT_FILE_NAME_INFORMATION fni = NULL;

      status = FltGetFileNameInformation(Data,FLT_FILE_NAME_NORMALIZED|FLT_FILE_NAME_QUERY_DEFAULT,&fni);
      if( NT_SUCCESS(status) )
      {
			status = NLSEIsPathEncrypted(FltObjects,&fni->Name,&enc_dir);
			FltReleaseFileNameInformation(fni);
			if( NT_SUCCESS(status) && enc_dir == TRUE )
			{
				NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
				"NLSE!PreCreate: PC down - deny parent dir %wZ\n",&parentDir);
				Data->IoStatus.Status = STATUS_ACCESS_DENIED;
				returnStatus = FLT_PREOP_COMPLETE;
				goto NLSEPreCreateOperationCallback_EXIT;
			}
      }
    }/* end - if PC down */


	//get volume context
    status=FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &volCtx); 
    if(!NT_SUCCESS(status))
    {
        volCtx=NULL;
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreCreate: Fail to get volume context status=0x%0x\n", status);
        goto NLSEPreCreateOperationCallback_EXIT;
    }

    irpEntry = NLFSEAllocateIRPEntry(FltObjects,
		volCtx,
        &parentDir,
        &nameToUse);

    if(irpEntry == NULL)
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "Can't Allocate IRP ENTRY\n");
        goto NLSEPreCreateOperationCallback_EXIT;
    }

    //Check if a backup process
    if(Data->Iopb->Parameters.Create.Options & 0x00ffffff & FILE_OPEN_FOR_BACKUP_INTENT)
    {
        if(IsShadowVolume(FltObjects, volCtx))
        {
            //ignore backup process
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "NLSE!PreCreate: ignore backup folder=%wZ unc_path=%wZ\n", &irpEntry->fileParentDir, &irpEntry->fileName);
            goto   NLSEPreCreateOperationCallback_EXIT;
        }
    }

    //Check if file/directory exist already; handle exist file/dir
    if(CreateDisposition==FILE_OVERWRITE
        || CreateDisposition==FILE_OVERWRITE_IF)
        bOverwrite = TRUE;
    
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_chk_exist);
#endif
    bChkFileExist = NLFSECheckFileExist(
		FltObjects,
		volCtx,
		irpEntry,
		bOverwrite,
		&pContext,
		&bNotFile,
		&bWinEncrypted,
		&bWinCompressed);
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_chk_exist);
#endif

    if(bChkFileExist)
    {
        if(bWinEncrypted)
        {
            // Ignore any EFS encrypted file/directory
            goto NLSEPreCreateOperationCallback_EXIT;
        }

        if(bNotFile)
        {
            //not a file; ignore it
            NLFSEFreeIRPEntry(irpEntry);
            irpEntry = NULL;
            goto NLSEPreCreateOperationCallback_EXIT;
        } 

        /* If the Policy Controller is down access to a document which already has
        * an SE per-stream context should be denied.  Files w/o per-stream context
        * are handled by the policy path which acquires the Policy Controller key.
        */
        if( pContext && !nlfseGlobal.bEnable )
        {
            Data->IoStatus.Status = STATUS_ACCESS_DENIED;
            returnStatus = FLT_PREOP_COMPLETE;
            goto NLSEPreCreateOperationCallback_EXIT;
        }

        if(pContext)
        {
            // If the per-stream context already exist
            // Try to get PC key again
            // So that driver knows if requester process is trusted or not
            pid = FltGetRequestorProcessId(Data);
            status = NLSEUpdateCurrentPCKey(pid, FALSE);
            if(STATUS_SUCCESS != status)
            {
                Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                returnStatus = FLT_PREOP_COMPLETE;
                goto NLSEPreCreateOperationCallback_EXIT;
            }
        }

        //An existing file
        irpEntry->bExist=TRUE;
        if(pContext)
        {
#if NLSE_DEBUG_PERFORMANCE_COUNT
            logPF = TRUE;
#endif
            // Get file size
#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfStart(&pfc_get_fsize);
#endif
            FOH_GET_FILE_SIZE_BY_NAME(
				FltObjects->Filter,
				FltObjects->Instance,
				&irpEntry->fileName,
				&pContext->FileSize);
#if NLSE_DEBUG_PERFORMANCE_COUNT
            PfEnd(&pfc_get_fsize);
#endif
            //The resulted file object will be attched with per stream context.
            //We don't need to call post operation callback 
            if(NLSEIsAccessFromRemote(securityCtx))
            {
                //NLSE encrypted file accessed from remote; not allow
                Data->IoStatus.Status=STATUS_ACCESS_DENIED;
                returnStatus=FLT_PREOP_COMPLETE;
            }
            else if(Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE)
            {
                //For NLSE encrypted file to be deleted,
                //we need to update per-stream context accordingly at post-create
                *CompletionContext = irpEntry;
                //irpEntry is passed to post-create; don't free it
                irpEntry=NULL;
                returnStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
            }
            goto NLSEPreCreateOperationCallback_EXIT;
        }

        //An Existing file that may or may not be NLSE encrypted
        if (irpEntry->pEncryptExtension == NULL)
        {
            //An existing file doesn't have NLSE ADS; no further interception
            goto NLSEPreCreateOperationCallback_EXIT;
        }
        else
        {
            //An existing file has NLSE ADS
            if(NLSEIsAccessFromRemote(securityCtx))
            {
                //NLSE encrypted file accessed from remote; not allow
                Data->IoStatus.Status=STATUS_ACCESS_DENIED;
                returnStatus=FLT_PREOP_COMPLETE;
            }
        }    
    }


#if NLSE_DEBUG_PERFORMANCE_COUNT
    logPF = TRUE;
#endif
    //check DRM attribute
    if (!FLT_IS_IRP_OPERATION( Data ))
    {
        // Since Fast I/O operations cannot be queued, we do DRM 
        // evaluation here. Compare to see if this is the file 
        // that need to be encrypted
#if NLSE_DEBUG_PERFORMANCE_COUNT
        PfStart(&pfc_chk_enc);
#endif
        returnStatus = NLFSECheckFileEncryption(FltObjects, irpEntry, Data);
#if NLSE_DEBUG_PERFORMANCE_COUNT
        PfEnd(&pfc_chk_enc);
#endif
        if(returnStatus != FLT_PREOP_SUCCESS_WITH_CALLBACK)
        {
            //do nothing in post op;
        }
        else
        {
            //setup stream context in post op
            *CompletionContext = irpEntry;
            //irpEntry is passed to post-create; don't free it
            irpEntry=NULL;
        } 
    }
    else
    {
        //Pending this i/o and do DRM evaluation in worker thread
        returnStatus = PendingIOForPolicyEvaluation(irpEntry, Data, FltObjects);
        if(returnStatus != FLT_PREOP_PENDING)
        {
            //failed to pending this io operation. ignore it then
            returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;	  
        }
        else
        {
            //irpEntry is inserted into pending queue; don't free it
            irpEntry = NULL;
        }
    }

NLSEPreCreateOperationCallback_EXIT:
#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PreCreate: %d ms\n", pfc_total.diff.LowPart);
        DbgPrint("             Get File Name:        %d ms\n", pfc_get_filename.diff.LowPart);
        DbgPrint("             Check Reserved File:  %d ms\n", pfc_chk_rsvfile.diff.LowPart);
        DbgPrint("             Check File Exist:     %d ms\n", pfc_chk_exist.diff.LowPart);
        DbgPrint("             Get File Size:        %d ms\n", pfc_get_fsize.diff.LowPart);
        DbgPrint("             Check File Encrypt:   %d ms\n", pfc_chk_enc.diff.LowPart);
    }
#endif
	if(nameToUse.Buffer) NLFSEFreeUnicodeNameString(&nameToUse);
    if(parentDir.Buffer) NLFSEFreeUnicodeNameString(&parentDir);
    if(volName.Buffer) NLFSEFreeUnicodeNameString(&volName);

	if(pContext)	FltReleaseContext( pContext );
    if(volCtx)		FltReleaseContext(volCtx);
    if(irpEntry)	NLFSEFreeIRPEntry(irpEntry);

    return returnStatus;
} /*--NLFSEOpCallbackPreCreate--*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostCreate (__inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __in_opt PVOID CompletionContext,
                           __in FLT_POST_OPERATION_FLAGS Flags)
{
    FILE_BASIC_INFORMATION fileBasicInfo;
    NLFSE_PIRP_ENTRY       irpEntry = NULL; //IRP entry passed from pre op
    NTSTATUS               status=STATUS_SUCCESS;
    BOOLEAN                bEFS=TRUE;
    BOOLEAN                bCompressed=TRUE;
    NLFSE_PVOLUME_CONTEXT  volCtx = NULL;
    PNLFSE_STREAM_CONTEXT  pStreamHandleContext = NULL;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    PfStart(&pfc_total);
#endif

    //Only handle the IRP with completion context
    if(NULL==CompletionContext)
        return FLT_POSTOP_FINISHED_PROCESSING;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    logPF = TRUE;
#endif

    //Get irpEntry record and check status
    irpEntry = (NLFSE_PIRP_ENTRY)CompletionContext;
    if(!NT_SUCCESS(Data->IoStatus.Status)             // Fail to open file
        || STATUS_REPARSE == Data->IoStatus.Status    // It's a re-parse point
        || NULL == FltObjects->FileObject->FsContext  // NULL FsContext
        )
    {
      goto _exit;
    }

    //For a new file, check if it is EFS or Compressed.
    //If yes, we can't NLSE encrypt it. Therefore, don't
    //attach per stream context
    if(!irpEntry->bExist)
    {
        CheckWinEFSOrCompressed(FltObjects, &bEFS, &bCompressed);
        if(bEFS) goto _exit;
    }

    // If it is a new created file, write NLSE Stream to the file in PostCreate
    // Get volume context
    status=FltGetVolumeContext(FltObjects->Filter, FltObjects->Volume, &volCtx);
    if(!NT_SUCCESS(status)) goto _exit;

    // Get Stream Context
    //pStreamHandleContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);
	status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pStreamHandleContext );

    if (!NT_SUCCESS( status ))
    //if(NULL==pStreamContext)
	{
		//try to allocate and attach a new StreamContext to it
		if(!irpEntry->bExist)
		{
			//new file, if need encrypt, allocate an encryp extion, otherwise return.
			if(!irpEntry->NeedEncrypt) goto _exit;
			//file need to be encrypted; create encryption extension
			irpEntry->pEncryptExtension=NLFSEAllocateEncryptExtension(0);
			if(NULL == irpEntry->pEncryptExtension) goto _exit;
			//Get data encryption key
			NLSEGenDataEncryptionKey(irpEntry->pEncryptExtension);
		}
		
		//create the context for the stream
		status = FltAllocateContext( FltObjects->Filter,
                                 FLT_STREAM_CONTEXT,
                                 sizeof(NLFSE_STREAM_CONTEXT),
                                 NonPagedPool,
                                 &pStreamHandleContext );
		if (!NT_SUCCESS(status))
		{
			NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
				"NLSE!PostCreateOperation: Error creating stream context, status=%x\n",
				status);
			
			pStreamHandleContext = NULL;
			goto _exit;
		}
		else
		{
			if(NlfseInitPerStreamHandleContext(
							&irpEntry->fileName,
							irpEntry->pEncryptExtension,
							&pStreamHandleContext))
			{
				//don't let encryptExt free with irpEntry 
				irpEntry->pEncryptExtension = NULL;
				
				//NLFSELinkContext( volCtx, FltObjects->FileObject, irpEntry, &pStreamContext ))
				pStreamHandleContext->Stream = FsRtlGetPerStreamContextPointer(FltObjects->FileObject);
				// set the context
				(VOID) FltSetStreamContext( FltObjects->Instance,
												  FltObjects->FileObject,
												  FLT_SET_CONTEXT_KEEP_IF_EXISTS,
												  pStreamHandleContext,
												  NULL );

				//
				//  Normally we would check the results of FltSetStreamHanleContext
				//  for a variety of error cases. However, The only error status 
				//  that could be returned, in this case, would tell us that
				//  contexts are not supported.  Even if we got this error,
				//  we just want to release the context now and that will free
				//  this memory if it was not successfully set.
			}
			else
			{
				goto _exit;
			}
			
		}

		//For being deleted NLSE encrypted file
		if(irpEntry->bExist)
		{
			if( (Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE) ||
			   (FILE_DELETE_ON_CLOSE & Data->IoStatus.Information) == FILE_DELETE_ON_CLOSE)
			{
			  //set delete flag so that ADS worker thread won't need to 
			  //update its ADS
			  ExAcquireFastMutex(&pStreamHandleContext->deleteFlagLock);
			  pStreamHandleContext->bDelete=TRUE;
			  ExReleaseFastMutex(&pStreamHandleContext->deleteFlagLock);
			}
		}

		//make sure we have just create the context
		if(NULL==pStreamHandleContext)goto _exit;
	}
	else
	{
		//stream context already exist
		if((Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE) ||
		   (FILE_DELETE_ON_CLOSE & Data->IoStatus.Information) == FILE_DELETE_ON_CLOSE )
		{
		  //set delete flag so that ADS worker thread won't need to 
		  //update its ADS
		  ExAcquireFastMutex(&pStreamHandleContext->deleteFlagLock);
		  pStreamHandleContext->bDelete=TRUE;
		  ExReleaseFastMutex(&pStreamHandleContext->deleteFlagLock);

		}
	}

    // Get file size when create the file
    status = FOH_GET_FILE_SIZE_BY_HANDLE(FltObjects->Instance, FltObjects->FileObject, &pStreamHandleContext->FileSize);
    if(!NT_SUCCESS(status)) pStreamHandleContext->FileSize.QuadPart = 0;
    pStreamHandleContext->encryptExt->fileRealLength = pStreamHandleContext->FileSize.QuadPart;

    if(!irpEntry->bExist)
    {
        if(NULL!=pStreamHandleContext->encryptExt)
            NLSECreateAdsAtPostCreate(FltObjects->Filter, FltObjects->Instance, &irpEntry->fileName, volCtx, pStreamHandleContext->encryptExt);
    }

_exit:
    if(pStreamHandleContext)	FltReleaseContext(pStreamHandleContext);
    if(volCtx)			FltReleaseContext(volCtx);
    if(irpEntry)		NLFSEFreeIRPEntry(irpEntry);

#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PostCreate: %d ms\n", pfc_total.diff.LowPart);
    }
#endif
    return FLT_POSTOP_FINISHED_PROCESSING;
}/*--NLFSEOpCallbackPostCreate--*/

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreQueryInfo (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             )
{
    FLT_PREOP_CALLBACK_STATUS retStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

    // The goal of this Callback is to filter streams, so we only handle FileStreamInformation
    if(FileStreamInformation!=Data->Iopb->Parameters.QueryFileInformation.FileInformationClass)
        goto _exit;

    retStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
_exit:
    return retStatus;
} /*--NLFSEOpCallbackPreQueryInfo--*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostQueryInfo (
                              __inout PFLT_CALLBACK_DATA Data,
                              __in PCFLT_RELATED_OBJECTS FltObjects,
                              __in PVOID CompletionContext,
                              __in FLT_POST_OPERATION_FLAGS Flags
                              )
{
    FLT_POSTOP_CALLBACK_STATUS retStatus        = FLT_POSTOP_FINISHED_PROCESSING;
    PFILE_STREAM_INFORMATION   streamInfoBuf    = NULL;
    ULONG                      streamInfoBufLen = 0;

    // Sanity check
    ASSERT(Data->Iopb->MajorFunction == IRP_MJ_QUERY_INFORMATION);
    // If the query fail, don't do anything
    if(Data->IoStatus.Status != STATUS_SUCCESS)
        return FLT_POSTOP_FINISHED_PROCESSING;
    // If it's not querying stream information, don't do anything
    if(FileStreamInformation!=Data->Iopb->Parameters.QueryFileInformation.FileInformationClass)
        return FLT_POSTOP_FINISHED_PROCESSING;

    // Remove our ADS from the result
    if(RemoveNLAdsFromQueryResult((PFILE_STREAM_INFORMATION)Data->Iopb->Parameters.QueryFileInformation.InfoBuffer))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "NLSE!NLFSEOpCallbackPostQueryInfo: ADS record is removed from query result\n");
    }

    return retStatus;
} /*--NLFSEOpCallbackPostQueryInfo--*/

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreSetInfo (__inout PFLT_CALLBACK_DATA Data,
                           __in_opt PCFLT_RELATED_OBJECTS FltObjects,
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
    FLT_PREOP_CALLBACK_STATUS   returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK; 
    NTSTATUS                    status;
    PFLT_FILE_NAME_INFORMATION  sourceFileNameInformation=NULL;
    FILE_INFORMATION_CLASS      InformationClass;
    PVOID                       p2pCtx = NULL;
    BOOLEAN                     nlseDriverEnabled = TRUE;
    PNLFSE_STREAM_CONTEXT       pStreamHandleContext     = NULL;
    NLFSE_PVOLUME_CONTEXT       volCtx            = NULL;
    LARGE_INTEGER               origFileSize;
    LARGE_INTEGER               newFileSize;


    // Don't handle stream file
    if(NULL==FltObjects)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    // The MJ Function is not correct
    if(IRP_MJ_SET_INFORMATION != Data->Iopb->MajorFunction)
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    //  Get our volume context and Stream Context
    status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx);
    if (!NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!NLFSEOpCallbackPreSetInfo: fail to get volume context, err=%08x\n", status);
        goto _exit;
    }
    //StreamContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);
	status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pStreamHandleContext );
    if((!NT_SUCCESS(status) || NULL == pStreamHandleContext) &&
		(Data->Iopb->Parameters.SetFileInformation.FileInformationClass != FileRenameInformation))


    {
        goto _exit;
    }

    // Handle request
    switch (Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        // Handle change file size
    case FileEndOfFileInformation:  // Change file size
        p2pCtx = NLFSEPreSetEndOfFile(Data, FltObjects);
        *CompletionContext = p2pCtx;
        returnStatus = (NULL!=p2pCtx)?FLT_PREOP_SUCCESS_WITH_CALLBACK:FLT_PREOP_SUCCESS_NO_CALLBACK;
        break;

        // Handle DELETE
    case FileDispositionInformation:
        p2pCtx = NLSEPreSetFileDisposition(Data, FltObjects);
        *CompletionContext = p2pCtx;
        returnStatus = (NULL!=p2pCtx)?FLT_PREOP_SUCCESS_WITH_CALLBACK:FLT_PREOP_SUCCESS_NO_CALLBACK;
        break;

        // Handle RENAME
    case FileRenameInformation:
        ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
        nlseDriverEnabled = nlfseGlobal.bEnable;
        ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
		{
			PFLT_FILE_NAME_INFORMATION   targetFileNameInfo;
			PFILE_RENAME_INFORMATION ri = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
			status=FltGetDestinationFileNameInformation(FltObjects->Instance,
			    FltObjects->FileObject,
			    ri->RootDirectory,
			    ri->FileName,
			    ri->FileNameLength,
			    FLT_FILE_NAME_NORMALIZED|
			    FLT_FILE_NAME_QUERY_DEFAULT,
			    &targetFileNameInfo);
			if(STATUS_SUCCESS == status)
			{
				status = FltParseFileNameInformation(targetFileNameInfo);
				if(STATUS_SUCCESS == status)
				{
					if(NLSEIsNxlFile(&targetFileNameInfo->Name))
					{
						// Don't Handle NXL file
						FltReleaseFileNameInformation(targetFileNameInfo);
						break;
					}

					if(NLSECheckDirEncryptionAttribute(FltObjects,&targetFileNameInfo->ParentDir,TRUE))
					{
						// check if untrust
						ULONG pid = FltGetRequestorProcessId(Data);
						status = NLSEUpdateCurrentPCKey(pid, FALSE);
						if(STATUS_SUCCESS != status)
						{
							Data->IoStatus.Status = STATUS_ACCESS_DENIED;
							returnStatus = FLT_PREOP_COMPLETE;
						}
					}

				}

				FltReleaseFileNameInformation(targetFileNameInfo);
			}
			if(returnStatus == FLT_PREOP_COMPLETE) break;
		}

		if( !nlfseGlobal.bEnable )
		{
			NTSTATUS enc_status;
			PFILE_RENAME_INFORMATION ri = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
			UNICODE_STRING file;
			BOOLEAN is_enc = FALSE;

			ASSERT( ri != NULL );
			file.Length = (USHORT) ri->FileNameLength;
			file.MaximumLength = (USHORT) ri->FileNameLength;
			file.Buffer = ri->FileName;
			enc_status = NLSEIsPathEncrypted(FltObjects,&file,&is_enc);
			if( NT_SUCCESS(enc_status) && is_enc )
			{
				Data->IoStatus.Status = STATUS_ACCESS_DENIED;
				returnStatus = FLT_PREOP_COMPLETE;
				break;
			}
		}

		if(nlseDriverEnabled) // Only do it when the driver is enabled
		{
			if (FLT_IS_IRP_OPERATION(Data))
			{
				p2pCtx=NLSEPreFileRename(Data, FltObjects);
				*CompletionContext = p2pCtx;
				returnStatus = (NULL!=p2pCtx)?FLT_PREOP_SUCCESS_WITH_CALLBACK:FLT_PREOP_SUCCESS_NO_CALLBACK;
			}
			else
			{
				returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
			}
		}
		break;

        // Handle SET BASIC INFORMATION
    case FileBasicInformation:
        if(
            (((PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
            || 
            (((PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->FileAttributes & FILE_ATTRIBUTE_COMPRESSED)
            )
        {
            // The file should not be encrypted/compressed
            ((PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->FileAttributes &= (~FILE_ATTRIBUTE_ENCRYPTED);
            ((PFILE_BASIC_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->FileAttributes &= (~FILE_ATTRIBUTE_COMPRESSED);
            FltSetCallbackDataDirty(Data);
        }
        break;

    default:                        // We don't care about other action
        returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
        break;
    }

_exit:
    if(pStreamHandleContext) FltReleaseContext(pStreamHandleContext); pStreamHandleContext=NULL;
    if(volCtx) FltReleaseContext(volCtx); volCtx=NULL;
    return returnStatus;
} /*--NLFSEOpCallbackPreSetInfo--*/

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetEndOfFileWhenSafe (
    __inout PFLT_CALLBACK_DATA Data,
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in PVOID CompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    FLT_POSTOP_CALLBACK_STATUS retStatus = FLT_POSTOP_FINISHED_PROCESSING;
    NTSTATUS                   Status    = STATUS_SUCCESS;
    PPRETOPOST_CONTEXT         p2pCtx    = (PPRETOPOST_CONTEXT)CompletionContext;
    ULONG                      ulWritten = 0;
    PUCHAR                     EmptyBlock= NULL;
    LARGE_INTEGER              EmptyBlockOffset = {0};
    ULONG                      EmptyDataLength = 0;
    PNLFSE_ADS_WORKITEM        workItem = NULL;
    KIRQL                      OldIRQL;
    FILE_END_OF_FILE_INFORMATION   feof = {0};
    FILE_POSITION_INFORMATION  fpi = {0};
    BOOLEAN                    NeedResetFileSize = FALSE;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    BOOLEAN                 logPF = FALSE;
    ULONG                   pfc_emptyblock_num = 0;
    NLPERFORMANCE_COUNTER   pfc_total = {0};
    NLPERFORMANCE_COUNTER   pfc_write_current = {0};
    NLPERFORMANCE_COUNTER   pfc_write_empty = {0};
    NLPERFORMANCE_COUNTER   pfc_reset_size = {0};
    NLPERFORMANCE_COUNTER   pfc_update_pad = {0};
    PfStart(&pfc_total);
#endif

    if(NULL == p2pCtx)
        goto _exit;

#if NLSE_DEBUG_PERFORMANCE_COUNT
    logPF = TRUE;
    PfStart(&pfc_total);
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_write_current);
#endif
    // If it is necessary, update last block with valid data
    if(NULL != p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock)
    {
        // Need to update last valid block
        fpi.CurrentByteOffset.QuadPart = FltObjects->FileObject->CurrentByteOffset.QuadPart;
        Status = FOH_SYNC_WRITE(FltObjects->Instance,
            FltObjects->FileObject,
            &p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlockOffset,
            nlfseGlobal.cryptoBlockSize,
            p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock,
            IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO,
            &ulWritten);
        if (!NT_SUCCESS(Status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!NLFSEOpCallbackPostSetEndOfFileWhenSafe: failed to update last valid block, status=%08x\n", Status);
        }
        FltObjects->FileObject->CurrentByteOffset.QuadPart = fpi.CurrentByteOffset.QuadPart;
        NeedResetFileSize = TRUE;
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_write_current);
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_write_empty);
#endif
    // Fill empty blocks
    if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength)
    {
        EmptyBlockOffset.QuadPart = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataOffset.QuadPart;
        EmptyBlock = ExAllocatePoolWithTag(NonPagedPool, nlfseGlobal.maxWriteBlockSize, NLFSE_BUFFER_TAG);
        if(NULL != EmptyBlock)
        {
            NeedResetFileSize = TRUE;
            EmptyDataLength = ROUND_TO_SIZE(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.EmptyDataLength, nlfseGlobal.cryptoBlockSize);
#if NLSE_DEBUG_PERFORMANCE_COUNT
            pfc_emptyblock_num = EmptyDataLength / nlfseGlobal.cryptoBlockSize;
#endif
            while(EmptyDataLength > 0)
            {
                // Both of EmptyDataLength and nlfseGlobal.maxWriteBlockSize are round to cryptoBlockSize
                // So WriteLength is also round to cryptoBlockSize
                ULONG WriteLength = min(EmptyDataLength, nlfseGlobal.maxWriteBlockSize);
                RtlZeroMemory(EmptyBlock, nlfseGlobal.maxWriteBlockSize);
                encrypt_buffer(p2pCtx->psCtx->encryptExt->key, NLSE_KEY_LENGTH_IN_BYTES, EmptyBlockOffset.QuadPart, EmptyBlock, WriteLength);

                // Write empty data to file
                fpi.CurrentByteOffset.QuadPart = FltObjects->FileObject->CurrentByteOffset.QuadPart;
                // So WriteLength is also round to cryptoBlockSize
                Status = FOH_SYNC_WRITE(FltObjects->Instance,
                    FltObjects->FileObject,
                    &EmptyBlockOffset,
                    WriteLength,
                    EmptyBlock,
                    IRP_NOCACHE|IRP_PAGING_IO|IRP_SYNCHRONOUS_PAGING_IO,
                    &ulWritten);
                FltObjects->FileObject->CurrentByteOffset.QuadPart = fpi.CurrentByteOffset.QuadPart;

                /* Write file, so the offset should not be updated. */
                if( !NT_SUCCESS(Status) )
                {
                    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!NLFSEOpCallbackPostSetEndOfFileWhenSafe: failed to fill empty blocks, status=%08x\n", Status);
                    break;
                }

                EmptyBlockOffset.QuadPart += WriteLength;
                EmptyDataLength -= WriteLength;
            }

            // Free buffer
            ExFreePoolWithTag(EmptyBlock, NLFSE_BUFFER_TAG);
        }
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_write_empty);
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_reset_size);
#endif
    if(NeedResetFileSize)
    {
        feof.EndOfFile.QuadPart = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart;
        FltSetInformationFile(FltObjects->Instance, FltObjects->FileObject, &feof, sizeof(FILE_END_OF_FILE_INFORMATION), FileEndOfFileInformation);
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_reset_size);
#endif


#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfStart(&pfc_update_pad);
#endif
    // Update stream
    KeAcquireSpinLock(&p2pCtx->psCtx->encryptExtLock, &OldIRQL);
    p2pCtx->psCtx->encryptExt->paddingLen     = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingLength;
    RtlCopyMemory(p2pCtx->psCtx->encryptExt->paddingData, p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.PaddingData, NLSE_PADDING_DATA_LEN);
    p2pCtx->psCtx->encryptExt->fileRealLength = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart;
    KeReleaseSpinLock(&p2pCtx->psCtx->encryptExtLock, OldIRQL);

    // Queue ADS work item
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
        workItem->ProcessId                 = p2pCtx->ProcessId;
        workItem->SectorSize                = p2pCtx->SectorSize;
        workItem->hostFilePath.MaximumLength= sizeof(WCHAR)*(2*MAX_PATH);
        workItem->hostFilePath.Length       = p2pCtx->psCtx->FileName.Length;
        workItem->hostFilePath.Buffer       = &(workItem->hostFilePathBuffer[0]);
        RtlCopyMemory(workItem->hostFilePathBuffer, p2pCtx->psCtx->FileName.Buffer, p2pCtx->psCtx->FileName.Length);
        RtlCopyMemory(&workItem->encryptExt, p2pCtx->psCtx->encryptExt, sizeof(NLFSE_ENCRYPT_EXTENSION));
        NLSEQueueAdsWorkItem(workItem);
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    PfEnd(&pfc_update_pad);
#endif

_exit:
    if(NULL!=p2pCtx)
    {
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx);
        if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock) ExFreePoolWithTag(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock, NLFSE_BUFFER_TAG);
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG);
        p2pCtx = NULL;
    }
#if NLSE_DEBUG_PERFORMANCE_COUNT
    if(logPF)
    {
        PfEnd(&pfc_total);
        DbgPrint("[NLPFLog] PostSetEndOfFile: %d ms\n", pfc_total.diff.LowPart);
        DbgPrint("             Update Current Block:        %d ms\n", pfc_write_current.diff.LowPart);
        DbgPrint("             Update Empty Block(%d*%d):   %d ms\n", pfc_emptyblock_num, nlfseGlobal.cryptoBlockSize, pfc_write_empty.diff.LowPart);
        DbgPrint("             Reset File Size:             %d ms\n", pfc_reset_size.diff.LowPart);
        DbgPrint("             Update Padding:              %d ms\n", pfc_update_pad.diff.LowPart);
    }
#endif
    return retStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetEndOfFile(__inout PFLT_CALLBACK_DATA Data,
                                __in PCFLT_RELATED_OBJECTS FltObjects,
                                __in PVOID CompletionContext,
                                __in FLT_POST_OPERATION_FLAGS Flags)
{
    FLT_POSTOP_CALLBACK_STATUS retStatus = FLT_POSTOP_FINISHED_PROCESSING;
    PPRETOPOST_CONTEXT         p2pCtx    = (PPRETOPOST_CONTEXT)CompletionContext;

    if(NULL == p2pCtx)
        return FLT_POSTOP_FINISHED_PROCESSING;

    if(!NT_SUCCESS(Data->IoStatus.Status)
        || STATUS_REPARSE==Data->IoStatus.Status
        || NULL==p2pCtx->psCtx)
    {
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx);
        if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock)
            ExFreePoolWithTag(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock, NLFSE_BUFFER_TAG);
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG);
        p2pCtx = NULL;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
    p2pCtx->psCtx->encryptExt->fileRealLength = p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.FileSizeAfterSet.QuadPart;

    if (!FltDoCompletionProcessingWhenSafe( Data,
        FltObjects,
        CompletionContext,
        Flags,
        NLFSEOpCallbackPostSetEndOfFileWhenSafe,
        &retStatus))
    {

        Data->IoStatus.Status = STATUS_UNSUCCESSFUL;
        Data->IoStatus.Information = 0;
        // Fail to execute complete callback func
        // Release p2p context
        if(p2pCtx->psCtx) FltReleaseContext(p2pCtx->psCtx);
        if(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock)
            ExFreePoolWithTag(p2pCtx->Parameters.SetInformationFile.Information.EndOfFile.LastBlock, NLFSE_BUFFER_TAG);
        ExFreePoolWithTag(p2pCtx, NLFSE_BUFFER_TAG);
        p2pCtx = NULL;
    }

    return retStatus;
}

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetInfo (__inout PFLT_CALLBACK_DATA Data,
                            __in PCFLT_RELATED_OBJECTS FltObjects,
                            __in_opt PVOID CompletionContext,
                            __in FLT_POST_OPERATION_FLAGS Flags)
{
    NTSTATUS status;
    FILE_INFORMATION_CLASS       InformationClass;
    PFLT_FILE_NAME_INFORMATION   targetFileNameInfo;
    PFLT_FILE_NAME_INFORMATION   sourceFileNameInfo;
    FILE_RENAME_INFORMATION      *renameInfo;
    NLFSE_PPRE_2_POST_CONTEXT    p2pCtx=NULL;
    FILE_END_OF_FILE_INFORMATION *fileEOFInfo;
    BOOLEAN                      bReleaseResource=TRUE;
    BOOLEAN                      bEFS=FALSE;
    BOOLEAN                      bCompressed=FALSE;
    PNLFSE_ADS_WORKITEM          workItem = NULL;
    KIRQL                        OldIRQL;

    //we only handle the IRP with completion context
    if(CompletionContext==NULL)
        return FLT_POSTOP_FINISHED_PROCESSING;

    __try
    {
        InformationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;
        if(InformationClass == FileEndOfFileInformation)
        {
            return NLFSEOpCallbackPostSetEndOfFile(Data, FltObjects, CompletionContext, Flags);
        }
        else if(InformationClass == FileDispositionInformation)
        {
            p2pCtx= (NLFSE_PPRE_2_POST_CONTEXT)CompletionContext;
            if(NT_SUCCESS(Data->IoStatus.Status))
            {
                //set delete flag so that ADS worker thread won't need to 
                //update its ADS
                ExAcquireFastMutex(&p2pCtx->streamCtx->deleteFlagLock);
                p2pCtx->streamCtx->bDelete=TRUE;
                ExReleaseFastMutex(&p2pCtx->streamCtx->deleteFlagLock);	
            }
            if(p2pCtx)
            {
                if(p2pCtx->VolCtx) FltReleaseContext( p2pCtx->VolCtx );
                if(p2pCtx->streamCtx) FltReleaseContext( p2pCtx->streamCtx);
                if(p2pCtx->instanceCtx) FltReleaseContext(p2pCtx->instanceCtx);
                if(p2pCtx->fltInstance)
                {
                    FltObjectDereference(p2pCtx->fltInstance);
                }	
                ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList, p2pCtx );	
            }        
        }
        else if(InformationClass == FileRenameInformation)
        {
            if(NT_SUCCESS(Data->IoStatus.Status) && STATUS_REPARSE != Data->IoStatus.Status)
            {
                p2pCtx= (NLFSE_PPRE_2_POST_CONTEXT)CompletionContext;
                //Get sourceFileName
                sourceFileNameInfo=p2pCtx->fileNameInfo;
                //rename operation; check if the target file should be encrypted
                renameInfo = (FILE_RENAME_INFORMATION*)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;	 

                //get target file name
                status=FltGetDestinationFileNameInformation(FltObjects->Instance,
                    FltObjects->FileObject,
                    renameInfo->RootDirectory,
                    renameInfo->FileName,
                    renameInfo->FileNameLength,
                    FLT_FILE_NAME_NORMALIZED|
                    FLT_FILE_NAME_QUERY_DEFAULT,
                    &targetFileNameInfo);
                if(STATUS_SUCCESS != status)
                {
                    FltReleaseFileNameInformation(sourceFileNameInfo);
                    ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList, p2pCtx );	
                    return FLT_POSTOP_FINISHED_PROCESSING;
                }
                else
                {
                    status = FltParseFileNameInformation(targetFileNameInfo);
                    if(STATUS_SUCCESS != status)
                    {
                        FltReleaseFileNameInformation(targetFileNameInfo);
                        FltReleaseFileNameInformation(sourceFileNameInfo);
                        ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList, p2pCtx );	
                        return FLT_POSTOP_FINISHED_PROCESSING;
                    }
                }

                // Since the file has been renamed, we should change 
                //per-stream context accordingly
                NLFSEUpdateStreamHandleContextFileName(FltObjects,
                    p2pCtx->streamCtx, 
                    targetFileNameInfo);

                //Check if the source has been encrypted;
                if(!p2pCtx->bSourceEncrypted)
                {
                    //Check if the target is EFS encrypted
                    CheckWinEFSOrCompressed(FltObjects, &bEFS, &bCompressed);
                    if(!bEFS)
                    {
                        //If already EFS encrypted, don't NLSE encrypt it
                        //Check if the target file should be encrypted; 
                        //if yes, encrypt it
                        if(!(Flags & FLTFL_POST_OPERATION_DRAINING) && FLT_IS_IRP_OPERATION(Data))
                        {
                            NLFSECheckAndEncryptRenamedFile(FltObjects,
                                Data,
                                targetFileNameInfo,
                                sourceFileNameInfo,
                                &bReleaseResource);
                        }
                    }
                }
                else
                {
                    CheckWinEFSOrCompressed(FltObjects, &bEFS, &bCompressed);
                    if(!bEFS)
                    {
						//Check if the target file should be decrypted;
						//If yes, decrypt it
						//TODO
					}
                    //Source has been NLSE encrypted. 
                    //TBD: handle the target is set as EFS encrypted
                }

                //clean up
                if(p2pCtx->streamCtx) FltReleaseContext( p2pCtx->streamCtx);
                if(bReleaseResource)
                {
                    //This will be released at worker thread that encrypts target file
                    FltReleaseFileNameInformation(targetFileNameInfo);
                }
                FltReleaseFileNameInformation(sourceFileNameInfo);
                ExFreeToNPagedLookasideList( &nlfseGlobal.pre2PostContextList, p2pCtx );
            }
        }//rename operation
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        //The targetFileNameInfo returned by
        //FltGetDestinationFileNameInformation() above might not be needed
        //anymore, so ideally we should detect that case here and call
        //FltReleaseFileNameInformation() to release it.  But it's too risky
        //to fix this now since we're at the end of a release.  So we'll just
        //leave it not-released and let fltMgr.sys leak memory with pool tag
        //"FMfn".  It should be okay since exception doesn't happen that often
        //anyway.
        //--- ayuen, 8/9/2011
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    if(bReleaseResource)
    {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
    else
    {
        return FLT_POSTOP_MORE_PROCESSING_REQUIRED;
    }
}/*--NLFSEOpCallbackPostSetInfo--*/

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreFsControl (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             )
{
    FLT_PREOP_CALLBACK_STATUS  fltStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
    PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
    NTSTATUS                   status    = STATUS_SUCCESS;
    NLFSE_PVOLUME_CONTEXT      volCtx = NULL;
    PNLFSE_STREAM_CONTEXT      pStreamHandleContext=NULL;
    UNICODE_STRING             dirName;

    UNREFERENCED_PARAMETER(CompletionContext);

    if(IRP_MN_USER_FS_REQUEST != Data->Iopb->MinorFunction)
        goto _exit;

    // Only handle EFS encrypt and Windows compression
    if(FSCTL_SET_ENCRYPTION != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode
        && FSCTL_ENCRYPTION_FSCTL_IO != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode
        && FSCTL_SET_COMPRESSION != Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode)
        goto _exit;

    try
    {
        // a. Get our volume context.
        status = FltGetVolumeContext( FltObjects->Filter, FltObjects->Volume, &volCtx );
        if (!NT_SUCCESS(status))
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLSE!PreFsControl: Error getting volume context, err=%x\n", status);
            leave;
        }

        // b. Find our per-stream handle context
        //pStreamContext=NLFSEFindExistingContext(volCtx, FltObjects->FileObject);    
		status = FltGetStreamContext( FltObjects->Instance,
                                        FltObjects->FileObject,
                                        &pStreamHandleContext );
        if(!NT_SUCCESS(status) || pStreamHandleContext == NULL)
        {
            //This it's not a NLSE encrypted file
            if(KeGetCurrentIrql() == PASSIVE_LEVEL)
            {
                //don't block encrypt/compress request
                if(!NLSEIsDirFileObj(FltObjects))
                    leave;

                //This is a directory; check if it is NLSE DRM directory
                //Get its name first
                status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &nameInfo );
                if (!NT_SUCCESS( status ))
                    leave;

                status = FltParseFileNameInformation( nameInfo );
                if(!NT_SUCCESS( status ))
                    leave;
                else if(nameInfo->Stream.Length)
                    leave;	// ignore all stream
                else if(nameInfo->Share.Length)
                    leave;	// ignore all remote access

                RtlInitUnicodeString(&dirName, NULL);
                NLSEComposeDirNameNoVolume(&dirName, &nameInfo->FinalComponent, &nameInfo->ParentDir);
                //Check if the directory is a NLSE DRM directory
                if(!NLSECheckDirEncryptionAttribute(FltObjects, &dirName, FALSE))
                {
                    //Not NLSE DRM directory; don't block encrypt/compress request
                    NLFSEFreeUnicodeNameString(&dirName);
                    leave;
                }
                NLFSEFreeUnicodeNameString(&dirName);
            }
            else
            {
                //At a not safe IRQ level; no further interception
                //don't block encrypt/compress request
                leave;
            }
        }

        // c. If the file is encrypted by NLSE, 
        //no FSCTL_SET_ENCRYPTION/FSCTL_SET_COMPRESSION is allowed
        if(FSCTL_SET_ENCRYPTION == Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode
            || FSCTL_ENCRYPTION_FSCTL_IO == Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode)
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreFsControl: No FsControl::FSCTL_SET_ENCRYPTION/FSCTL_ENCRYPTION_FSCTL_IO is allowed on a NLSE encrypted file/directory\n");
        }
        if(FSCTL_SET_COMPRESSION == Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode)
        {
            NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, "NLFSE!PreFsControl: No FsControl::FSCTL_SET_COMPRESSION is allowed on a NLSE encrypted file/directory\n");
        }
        Data->IoStatus.Status = STATUS_SUCCESS; //STATUS_INVALID_PARAMETER; 
        Data->IoStatus.Information = 0;
        fltStatus = FLT_PREOP_COMPLETE;
    }
    finally
    {
        if(nameInfo)               FltReleaseFileNameInformation( nameInfo ); nameInfo=NULL;
        if(volCtx != NULL)         FltReleaseContext( volCtx ); volCtx = NULL;
        if(pStreamHandleContext != NULL) FltReleaseContext( pStreamHandleContext ); pStreamHandleContext = NULL;
    }

_exit:
    return fltStatus;
}/*--NLFSEOpCallbackPreFsControl--*/

//////////////////////////////////////////////////////////////////



