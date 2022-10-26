/*++

Module Name:

    NLSEInit.c

Abstract:
  Initialiation of system Encryption in kernel mode

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSEDrmPathList.h"
#include "NLSEDrmFileList.h"
#include "nlperf.h"
#include "nlprocess.h"

//Global variable
NLFSE_GLOBAL_DATA nlfseGlobal;
NL_KLOG           nlseKLog;
//variable scoped in this file
ULONG nlseLoggingFlags=0x00000000;
ULONG             g_SystemProcessId = 4;

static  ULONG     g_pnOffset = 0;



/** NLSE_DRIVER_PARAMETERS
 *
 *  \brief Runtime parameters for NLSE.
 */
typedef struct
{
  ULONG maxDRMFileOneShotCount;         /* max. count of one-shot DRM file
                                           list */
} NLSE_DRIVER_PARAMS, *PNLSE_DRIVER_PARAMS;

/*************************************************************************
    Prototypes
*************************************************************************/

NTSTATUS
InstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
CleanupVolumeContextCallback(
    __in PFLT_CONTEXT Context,
    __in FLT_CONTEXT_TYPE ContextType
    );

VOID
CleanupStreamContextCallback(
                             __in PFLT_CONTEXT  Context,
                             __in FLT_CONTEXT_TYPE  ContextType
                             );

VOID
CleanupInstanceContextCallback (
    __in PFLT_CONTEXT Context,
    __in FLT_CONTEXT_TYPE ContextType
    );

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    );

NTSTATUS
FilterUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    );

VOID
InstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
ReadDriverParameters (
    __in PUNICODE_STRING RegistryPath,
    __out PNLSE_DRIVER_PARAMS params
    );

static
__drv_requiresIRQL(PASSIVE_LEVEL)
ULONG
NLGetSystemNameOffset(
                      );

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
const CHAR*
NLGetCurrentProcessName(
                        );

/*************************************************************************
    IRP Callback Prototypes
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreRead(
                       __inout PFLT_CALLBACK_DATA Data,
                       __in PCFLT_RELATED_OBJECTS FltObjects,
                       __deref_out_opt PVOID *CompletionContext
                       );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostRead(
                        __inout PFLT_CALLBACK_DATA Data,
                        __in PCFLT_RELATED_OBJECTS FltObjects,
                        __in PVOID CompletionContext,
                        __in FLT_POST_OPERATION_FLAGS Flags
                        );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreWrite(
                        __inout PFLT_CALLBACK_DATA Data,
                        __in PCFLT_RELATED_OBJECTS FltObjects,
                        __deref_out_opt PVOID *CompletionContext
                        );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostWrite(
                         __inout PFLT_CALLBACK_DATA Data,
                         __in PCFLT_RELATED_OBJECTS FltObjects,
                         __in PVOID CompletionContext,
                         __in FLT_POST_OPERATION_FLAGS Flags
                         );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreCreate (
                          __inout PFLT_CALLBACK_DATA Data,
                          __in PCFLT_RELATED_OBJECTS FltObjects,
                          __deref_out PVOID *CompletionContext
                          );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostCreate (
                           __inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __in PVOID CompletionContext,
                           __in FLT_POST_OPERATION_FLAGS Flags
                           );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreNwQueryOpen (
                               __inout PFLT_CALLBACK_DATA Data,
                               __in PCFLT_RELATED_OBJECTS FltObjects,
                               __deref_out PVOID *CompletionContext
                               );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreQueryInfo (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostQueryInfo (
                              __inout PFLT_CALLBACK_DATA Data,
                              __in PCFLT_RELATED_OBJECTS FltObjects,
                              __in PVOID CompletionContext,
                              __in FLT_POST_OPERATION_FLAGS Flags
                              );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreSetInfo (
                           __inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __deref_out PVOID *CompletionContext
                           );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostSetInfo (
                            __inout PFLT_CALLBACK_DATA Data,
                            __in PCFLT_RELATED_OBJECTS FltObjects,
                            __in PVOID CompletionContext,
                            __in FLT_POST_OPERATION_FLAGS Flags
                            );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreFsControl (
                             __inout PFLT_CALLBACK_DATA Data,
                             __in PCFLT_RELATED_OBJECTS FltObjects,
                             __deref_out PVOID *CompletionContext
                             );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreDirectoryCtrl (
                                 __inout PFLT_CALLBACK_DATA Data,
                                 __in PCFLT_RELATED_OBJECTS FltObjects,
                                 __deref_out PVOID *CompletionContext
                                 );

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostDirectoryCtrl (
                                  __inout PFLT_CALLBACK_DATA Data,
                                  __in PCFLT_RELATED_OBJECTS FltObjects,
                                  __in PVOID CompletionContext,
                                  __in FLT_POST_OPERATION_FLAGS Flags
                                  );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreFlushBuffers (
                                __inout PFLT_CALLBACK_DATA Data,
                                __in PCFLT_RELATED_OBJECTS FltObjects,
                                __deref_out PVOID *CompletionContext
                                );

FLT_PREOP_CALLBACK_STATUS
NLFSEOpCallbackPreCleanup (
                           __inout PFLT_CALLBACK_DATA Data,
                           __in PCFLT_RELATED_OBJECTS FltObjects,
                           __deref_out PVOID *CompletionContext
                           );

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, InstanceSetup)
#pragma alloc_text(PAGE, CleanupVolumeContextCallback)
#pragma alloc_text(PAGE, CleanupInstanceContextCallback)
#pragma alloc_text(PAGE, CleanupStreamContextCallback)
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, NLGetSystemNameOffset)
#pragma alloc_text(INIT, ReadDriverParameters)
#pragma alloc_text(PAGE, FilterUnload)
#pragma alloc_text(PAGE, InstanceTeardownStart)
#endif


//  Operation we currently care about.
CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
  { IRP_MJ_CREATE,
    0,
    NLFSEOpCallbackPreCreate,
    NLFSEOpCallbackPostCreate },

  { IRP_MJ_NETWORK_QUERY_OPEN,
    0,
    NLFSEOpCallbackPreNwQueryOpen,
    NULL },

  { IRP_MJ_QUERY_INFORMATION,
    0,
    NLFSEOpCallbackPreQueryInfo,
    NLFSEOpCallbackPostQueryInfo },

  { IRP_MJ_SET_INFORMATION,
    0,
    NLFSEOpCallbackPreSetInfo,
    NLFSEOpCallbackPostSetInfo },

  { IRP_MJ_READ,
    0,
    NLFSEOpCallbackPreRead,
    NLFSEOpCallbackPostRead },

  { IRP_MJ_WRITE,
    0,
    NLFSEOpCallbackPreWrite,
    NLFSEOpCallbackPostWrite },

  { IRP_MJ_FILE_SYSTEM_CONTROL,
    0,
    NLFSEOpCallbackPreFsControl,
    NULL },

  { IRP_MJ_DIRECTORY_CONTROL,
    0,
    NLFSEOpCallbackPreDirectoryCtrl,
    NLFSEOpCallbackPostDirectoryCtrl },

  { IRP_MJ_FLUSH_BUFFERS,
    0,
    NLFSEOpCallbackPreFlushBuffers,
    NULL },

  { IRP_MJ_CLEANUP,
    0,
    NLFSEOpCallbackPreCleanup,
    NULL },

  { IRP_MJ_OPERATION_END }
};

//  Context definitions we currently care about.  Note that the system will
//  create a lookAside list for the volume context because an explicit size
//  of the context is specified.
CONST FLT_CONTEXT_REGISTRATION ContextNotifications[] = {
  { FLT_VOLUME_CONTEXT,
    0,
    CleanupVolumeContextCallback,
    sizeof(NLFSE_VOLUME_CONTEXT),
    NLFSE_CONTEXT_TAG },

  { FLT_INSTANCE_CONTEXT,
    0,
    CleanupInstanceContextCallback,
    sizeof( NLFSE_INSTANCE_CONTEXT ),
    NLFSE_CONTEXT_TAG },

  { FLT_STREAM_CONTEXT,
	0,
	CleanupStreamContextCallback,
	sizeof( NLFSE_STREAM_CONTEXT ),
	NLFSE_CONTEXT_TAG },

  { FLT_CONTEXT_END }
};

//  This defines what we want to filter with FltMgr
CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
#if defined(DBG)
    0,                                  //  Flags
#else
    FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP,
#endif
    ContextNotifications,               //  Context
    Callbacks,                          //  Operation callbacks
    FilterUnload,                       //  MiniFilterUnload
    InstanceSetup,                      //  InstanceSetup
    NULL,                               //  InstanceQueryTeardown
    InstanceTeardownStart,              //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete
    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
//                      Routines
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//setup communication channel to user mode
static NTSTATUS
SetupCommunication()
{
  OBJECT_ATTRIBUTES    ObjectAttributes;
  NTSTATUS             status=STATUS_SUCCESS;
  PSECURITY_DESCRIPTOR sd = NULL;
  OBJECT_ATTRIBUTES    oa;
  UNICODE_STRING       uniString;

  status  = FltBuildDefaultSecurityDescriptor(&sd,FLT_PORT_ALL_ACCESS);
  if( !NT_SUCCESS(status) ) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		"NLSE: FltBuildDefaultSecurityDescriptor failed (0x%x)\n",
		status);
    return status;
  } else {
    //grant unconditional access to everyone
#pragma prefast(disable:6248, "Setting a SECURITY_DESCRIPTOR's DACL to NULL will result in an unprotected object")
    status = RtlSetDaclSecurityDescriptor( sd, TRUE, NULL, FALSE );
#pragma prefast(enable:6248, "re-enable this warning")
    if(!NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		  "NLSE!SetupCommunication: SetDaclSecurityDescriptor(0x%x)\n",
		  status);
      return status;
    }
  }


  RtlInitUnicodeString(&uniString,NLSE_PORT_NAME);
  InitializeObjectAttributes(&oa,
			     &uniString,
			     OBJ_KERNEL_HANDLE|OBJ_CASE_INSENSITIVE,
			     NULL,
			     sd);
  status = FltCreateCommunicationPort(nlfseGlobal.filterHandle,
				      &nlfseGlobal.serverPort,
				      &oa,
				      NULL,
				      NLSEClientConnect,
				      NLSEClientDisconnect,
				      NLSEClientMessage,
				      NLSE_MAX_PORT_CONNECTION);
  if( !NT_SUCCESS(status) ) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		"NLSe: FltCreateCommunicationPort failed (0x%x)\n",
		status);
    return status;
  }
 
  FltFreeSecurityDescriptor(sd);
  
  return status;
}

NTSTATUS
NLFSESetInstanceContext(__in PCFLT_RELATED_OBJECTS fltObjects)
{
  NLFSE_PINSTANCE_CONTEXT InstCtx = NULL;
  NTSTATUS Status = STATUS_SUCCESS;
 
  //
  //  Allocate and initialize the instance context.
  //
  Status = FltAllocateContext( fltObjects->Filter,
			       FLT_INSTANCE_CONTEXT,
			       sizeof( NLFSE_INSTANCE_CONTEXT ),
			       NonPagedPool,
                               &InstCtx );

  if (!NT_SUCCESS( Status )) {
    NL_KLOG_Log(&nlseKLog,
		NL_KLOG_LEVEL_ERR, 
		"[NLSE]: Failed to allocate instance context (Volume = %p, Instance = %p, Status = 0x%x)\n",
		fltObjects->Volume,
		fltObjects->Instance,
		Status);
    goto InstanceSetupCleanup;
  }

  //  Initialize other members of the instance context.
  InstCtx->Instance = fltObjects->Instance;
  KeInitializeEvent( &InstCtx->TeardownEvent, NotificationEvent, FALSE );

  //  Set the instance context.
  Status = FltSetInstanceContext( fltObjects->Instance,
				  FLT_SET_CONTEXT_KEEP_IF_EXISTS,
				  InstCtx,
				  NULL );

  if( !NT_SUCCESS( Status )) {
    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		"[NLSE]: Failed to set instance context (Volume = %p, Instance = %p, Status = 0x%x)\n",
		fltObjects->Volume,
		fltObjects->Instance,
		Status );

    goto InstanceSetupCleanup;
  }

InstanceSetupCleanup:
  if ( InstCtx != NULL ) {
    FltReleaseContext( InstCtx );
  }

  return Status;
}

NTSTATUS
InstanceSetup (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume.

    By default we want to attach to all volumes.  This routine will try and
    get a "DOS" name for the given volume.  If it can't, it will try and
    get the "NT" name for the volume (which is what happens on network
    volumes).  If a name is retrieved a volume context will be created with
    that name.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Flags describing the reason for this attach request.

Return Value:

    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach

--*/
{
    PDEVICE_OBJECT devObj = NULL;
    NLFSE_PVOLUME_CONTEXT ctx = NULL;
    NTSTATUS status;
    ULONG retLen;
    PUNICODE_STRING workingName;
    USHORT size;
    UCHAR volPropBuffer[sizeof(FLT_VOLUME_PROPERTIES)+512];
    PFLT_VOLUME_PROPERTIES volProp = (PFLT_VOLUME_PROPERTIES)volPropBuffer;
    UNICODE_STRING nlseShadowPrefix = {0};
    UNICODE_STRING SymantecSnapshotPrefix = {0};

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    RtlInitUnicodeString(&nlseShadowPrefix, L"\\Device\\HarddiskVolumeShadowCopy");
    RtlInitUnicodeString(&SymantecSnapshotPrefix, L"\\Device\\SymantecSnapshot");

    /* Attach ONLY to disks */
    if( VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM )
    {
        return STATUS_FLT_DO_NOT_ATTACH;
    }

    /* Attach ONLY to NTFS volumes */
    if( VolumeFilesystemType != FLT_FSTYPE_NTFS )
    {
        return STATUS_FLT_DO_NOT_ATTACH;
    }

    // Currently we only support Volume whose SectorSize is MIN_SECTOR_SIZE (512) bytes
    status = FltGetVolumeProperties( FltObjects->Volume, volProp, sizeof(volPropBuffer), &retLen );
    if (!NT_SUCCESS(status) || NULL==volProp || (MIN_SECTOR_SIZE!=volProp->SectorSize))
    {
        return STATUS_FLT_DO_NOT_ATTACH;
    }

    // Don't attach to RemovableMedia, Floppy, RemoteDevice, VirtualVolume
    if( (FILE_REMOVABLE_MEDIA & volProp->DeviceCharacteristics)
        || (FILE_FLOPPY_DISKETTE & volProp->DeviceCharacteristics)
        || (FILE_REMOTE_DEVICE & volProp->DeviceCharacteristics)
        || (FILE_VIRTUAL_VOLUME & volProp->DeviceCharacteristics)
        )
    {
        return STATUS_FLT_DO_NOT_ATTACH;
    }

    try
    {
        status = NLFSESetInstanceContext(FltObjects);
        if(!NT_SUCCESS(status))
        {
            //could not allocate an instance context, quit now
            leave;
        }

        //  Allocate a volume context structure.
        status = FltAllocateContext( FltObjects->Filter, FLT_VOLUME_CONTEXT, sizeof(NLFSE_VOLUME_CONTEXT), NonPagedPool, &ctx );
        if (!NT_SUCCESS(status))
        {
            //  We could not allocate a context, quit now
            leave;
        }
        RtlZeroMemory(ctx, sizeof(NLFSE_VOLUME_CONTEXT));

        // Init directory info list        
        NLInitDList(&ctx->DInfoList);

        // Init file name info cache
        NLFNInfoCacheInit(&ctx->FNInfoCache);

        // Get Volume Name
        //  Figure out which name to use from the properties
        if (volProp->RealDeviceName.Length > 0)
        {
            workingName = &volProp->RealDeviceName;
        }
        else if (volProp->FileSystemDeviceName.Length > 0)
        {
            workingName = &volProp->FileSystemDeviceName;
        }
        else
        {
            //  No name, don't save the context
            status = STATUS_FLT_DO_NOT_ATTACH;
            leave;
        }

        if(RtlPrefixUnicodeString(&nlseShadowPrefix, workingName, TRUE))
        {
            //  Shadow Volume, don't attach
            status = STATUS_FLT_DO_NOT_ATTACH;
            leave;
        }

        if(RtlPrefixUnicodeString(&SymantecSnapshotPrefix, workingName, TRUE))
        {
            //  Symantec Snapshot Volume, don't attach
            status = STATUS_FLT_DO_NOT_ATTACH;
            leave;
        }

#pragma prefast(disable:6014, "Release this memory in different function cause incorrect preFast warning 6014 - memory Leak") 
        ctx->Name.Buffer= ExAllocatePoolWithTag(NonPagedPool, workingName->MaximumLength, NLFSE_BUFFER_TAG);
#pragma prefast(enable:6014, "Recover this warning")
        if(NULL == ctx->Name.Buffer)
        {
            //  We could not allocate buffer to hold volume name, quit now
            leave;
        }
        RtlZeroMemory(ctx->Name.Buffer, workingName->MaximumLength);
        RtlCopyMemory(ctx->Name.Buffer, workingName->Buffer, workingName->Length);
        ctx->Name.Length         = workingName->Length;
        ctx->Name.MaximumLength  = workingName->MaximumLength;

        // Get Volume SectorSize
        ctx->SectorSize = max(volProp->SectorSize,MIN_SECTOR_SIZE);


        //  Init the buffer field (which may be allocated later).
        ctx->DosName.Buffer = NULL;

        //  Get the storage device object we want a name for.
        status = FltGetDiskDeviceObject( FltObjects->Volume, &devObj );
        if (NT_SUCCESS(status))
        {
            //  Try and get the DOS name.  If it succeeds we will have
            //  an allocated name buffer.  If not, it will be NULL
            status = IoVolumeDeviceToDosName( devObj, &ctx->DosName );
        }

        if( !NT_SUCCESS(status) ||
            ctx->DosName.Length < (2*sizeof(WCHAR)) ||
            NULL == ctx->DosName.Buffer
            )
        {
            // No DOS Name? Don't Attach
            status = STATUS_FLT_DO_NOT_ATTACH;
            leave;
        }

        ctx->DosName.Buffer[0] = RtlUpcaseUnicodeChar(ctx->DosName.Buffer[0]);
        if( (':' != ctx->DosName.Buffer[1]) ||
            (ctx->DosName.Buffer[0]<L'A' && ctx->DosName.Buffer[0]>L'Z')
            )
        {
            // Not normal drive letter "C:" to "Z:"? Don't Attach
            status = STATUS_FLT_DO_NOT_ATTACH;
            leave;
        }

        //  Set the context
        status = FltSetVolumeContext( FltObjects->Volume, FLT_SET_CONTEXT_KEEP_IF_EXISTS, ctx, NULL );

        //  It is OK for the context to already be defined.
        if (status == STATUS_FLT_CONTEXT_ALREADY_DEFINED)
        {
            status = STATUS_SUCCESS;
        }

        if(status != STATUS_SUCCESS)
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Fail to attach to volume\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Name:       %wZ\n", &ctx->Name);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Dos Name:   %wZ\n", &ctx->DosName);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       SectorSize: %d\n", ctx->SectorSize);
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Attach to volume\n");
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Name:       %wZ\n", &ctx->Name);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       Dos Name:   %wZ\n", &ctx->DosName);
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       SectorSize: %d\n", ctx->SectorSize);
            
            if(L'C' == ctx->DosName.Buffer[0])
            {
                // This is C: drive, open config file
                NTSTATUS        NewStatus;
                UNICODE_STRING  ConfigFileName;

                RtlInitUnicodeString(&ConfigFileName,
                                     L"\\??\\C:\\Program Files\\NextLabs\\System Encryption\\config\\SystemEncryption.cfg");
                NewStatus = NxReadNonDrmList ( FltObjects->Filter,
                                               FltObjects->Instance,
                                               &ConfigFileName);
                if(!NT_SUCCESS(NewStatus))
                {
                    KdPrint(("Fail to read SE cofnig file when attach to Volume C:\n"));
                }
            }
        }
    }
    finally
    {
        //  Always release the context.  If the set failed, it will free the
        //  context.  If not, it will remove the reference added by the set.
        //  Note that the name buffer in the ctx will get freed by the context
        //  cleanup routine.
        if (ctx) FltReleaseContext( ctx );

        //  Remove the reference added to the device object by
        //  FltGetDiskDeviceObject.
        if (devObj) ObDereferenceObject( devObj );
    }

    return status;
}

//
//  Instance Context cleanup routine.
//
VOID
CleanupInstanceContextCallback (
    __in PFLT_CONTEXT Context,
    __in FLT_CONTEXT_TYPE ContextType
    )
/*++

Routine Description:

    FltMgr calls this routine immediately before it deletes the context.

Arguments:

    Context - Pointer to the minifilter driver's portion of the context.

    ContextType - Type of context. Must be one of the following values:
        FLT_FILE_CONTEXT (Microsoft Windows Vista and later only.),
        FLT_INSTANCE_CONTEXT, FLT_STREAM_CONTEXT, FLT_STREAMHANDLE_CONTEXT,
        FLT_TRANSACTION_CONTEXT (Windows Vista and later only.), and
        FLT_VOLUME_CONTEXT

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( ContextType );

    PAGED_CODE();

    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"NLSE: NLSE!CleanupInstanceContextCallback\n");
}

static VOID
CleanupVolumeContextCallback(
    __in PFLT_CONTEXT Context,
    __in FLT_CONTEXT_TYPE ContextType
    )
/*++

Routine Description:

    The given context is being freed.
    Free the allocated name buffer if there one.

Arguments:

    Context - The context being freed

    ContextType - The type of context this is

Return Value:

    None

--*/
{
    NLFSE_PVOLUME_CONTEXT ctx = Context;

    PAGED_CODE();

    UNREFERENCED_PARAMETER( ContextType );

    ASSERT(ContextType == FLT_VOLUME_CONTEXT);

    NLFNInfoCacheDestroy(&ctx->FNInfoCache);
    NLFreeDList(&ctx->DInfoList);

    if (ctx->Name.Buffer != NULL)
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "NLSE:CleanupVolumeContextCallback: %wZ\n", &ctx->Name);
      ExFreePool(ctx->Name.Buffer);
      ctx->Name.Buffer = NULL;
      ctx->Name.Length=0;
      ctx->Name.MaximumLength=0;
    }
    if (ctx->DosName.Buffer != NULL)
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, "NLSE:CleanupVolumeContextCallback: %wZ\n", &ctx->DosName);
      ExFreePool(ctx->DosName.Buffer);
      ctx->DosName.Buffer = NULL;
      ctx->DosName.Length=0;
      ctx->DosName.MaximumLength=0;
    }
}

VOID
CleanupStreamContextCallback(
                             __in_opt PFLT_CONTEXT  Context,
                             __in FLT_CONTEXT_TYPE  ContextType
                             )
{
	PNLFSE_STREAM_CONTEXT StmContext = Context;

    PAGED_CODE();

	if(ContextType != FLT_STREAM_CONTEXT) return;
	if(Context == NULL) return;

    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"\n");
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"[NLSE] Stream Context is being destroyed\n");
    if(NULL != StmContext->FileName.Buffer)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Name: %wZ\n", &StmContext->FileName);
		ExFreePoolWithTag(StmContext->FileName.Buffer, NLFSE_CONTEXT_TAG);
		StmContext->FileName.Buffer = NULL;
        StmContext->FileName.Length = 0;
    }
    if(NULL != StmContext->encryptExt)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG,"       File Size: %I64d\n", StmContext->encryptExt->fileRealLength);
		NLFSEFreeEncryptExtension(StmContext->encryptExt);
		StmContext->encryptExt = NULL;
    }
}

VOID
InstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS FltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is been deleted.

Return Value:

    None.

--*/
{
    NLFSE_PINSTANCE_CONTEXT InstCtx = 0;
    NTSTATUS Status;
    NLFSE_PVOLUME_CONTEXT volCtx = NULL;

    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"[NLSE]: InstanceTeardownStart\n");

    if(NT_SUCCESS(FltGetVolumeContext(FltObjects->Filter,
				      FltObjects->Volume,
				      &volCtx)))
	{
		//Oney Commit: NLFSEDeleteAllContexts(volCtx);//if needed, oney will add this
      FltReleaseContext(volCtx);
    }

    //
    //  Get a pointer to the instance context.
    //
    Status = FltGetInstanceContext( FltObjects->Instance,
                                    &InstCtx );
    if (!NT_SUCCESS(Status)) {
        ASSERT( !"Instance Context is missing" );
        return;
    }

    //
    //  Signal the worker thread if it is pended.
    //
    KeSetEvent( &InstCtx->TeardownEvent, 0, FALSE );

    //
    //  Cleanup
    //
    FltReleaseContext( InstCtx );

} /*--InstanceTeardownStart --*/

/*************************************************************************
    Initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine.  This registers with FltMgr and
    initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Status of the operation

--*/
{
    NLSE_DRIVER_PARAMS drvParams;
    NTSTATUS        status;

#if (NLSE_DEBUG_DATA_VERIFY || NLSE_DEBUG_FAKE_FILE_KEY || NLSE_DEBUG_FAKE_PC_KEY || NLSE_DEBUG_CRYPTO_PASSTHROUGH)
#  if !defined(DBG)
#    error NLSE ERROR - Cannot set debug options for a release driver.
#  endif
#endif

#if defined(DBG)
    DbgPrint("********************************************************************************\n");
    DbgPrint("* NextLabs System Encryption Driver\n");
    DbgPrint("*\n");
    DbgPrint("* WARNING: The driver is compiled with debug parameters\n");
    DbgPrint("*\n");

#if NLSE_DEBUG_FAKE_FILE_KEY
    DbgPrint("*   NLSE_DEBUG_FAKE_FILE_KEY on\n");
#else
    DbgPrint("*   NLSE_DEBUG_FAKE_FILE_KEY off\n");
#endif

#if NLSE_DEBUG_FAKE_PC_KEY
    DbgPrint("*   NLSE_DEBUG_FAKE_PC_KEY on\n");
#else
    DbgPrint("*   NLSE_DEBUG_FAKE_PC_KEY off\n");
#endif

#if NLSE_DEBUG_DATA_VERIFY
    DbgPrint("*   NLSE_DEBUG_DATA_VERIFY on\n");
#else
    DbgPrint("*   NLSE_DEBUG_DATA_VERIFY off\n");
#endif

#if NLSE_DEBUG_DATA_VERIFY_BUGCHECK
    DbgPrint("*   NLSE_DEBUG_DATA_VERIFY_BUGCHECK on\n");
#else
    DbgPrint("*   NLSE_DEBUG_DATA_VERIFY_BUGCHECK off\n");
#endif

#if NLSE_DEBUG_CRYPTO_PASSTHROUGH
    DbgPrint("*   NLSE_DEBUG_CRYPTO_PASSTHROUGH on\n");
#else
    DbgPrint("*   NLSE_DEBUG_CRYPTO_PASSTHROUGH off\n");
#endif

#if NLSE_DEBUG_PERFORMANCE_COUNT
    DbgPrint("*   NLSE_DEBUG_PERFORMANCE_COUNT on\n");
#else
    DbgPrint("*   NLSE_DEBUG_PERFORMANCE_COUNT off\n");
#endif

    DbgPrint("*\n");
    DbgPrint("********************************************************************************\n");
#endif /* defined(DBG) */

    g_SystemProcessId = (ULONG)PsGetCurrentProcessId();
    g_pnOffset        = NLGetSystemNameOffset();

    // Init performance counter
    NLPerfInit();

    status = NxInitNonDrmData();
    if(!NT_SUCCESS(status))
    {
        KdPrint(("Fail to initialize NonDRMData\n"));
    }

    //Initialize the global 
    RtlZeroMemory(&nlfseGlobal, sizeof(nlfseGlobal));
    nlfseGlobal.bEnable=FALSE;
    nlfseGlobal.encryptWorkQueueSize=0L;
    nlfseGlobal.cryptoBlockSize=512; //512 bytes
    nlfseGlobal.cbcBlockSize=16; //16 bytes
    nlfseGlobal.maxWriteBlockSize=65536; //64K bytes -- IT MUST BE (N * cryptoBlockSize)
    ExInitializeFastMutex(&nlfseGlobal.currentPCKeyLock);
    nlfseGlobal.hasCurrentPCKey = FALSE;
    ExInitializeFastMutex(&nlfseGlobal.enableStatusMutex);

    nlfseGlobal.log_file_set = FALSE;
	nlfseGlobal.CryptoInit = FALSE;
	nlfseGlobal.CryptoInitSuccess = TRUE;

    //  Get debug trace flags
    ReadDriverParameters( RegistryPath, &drvParams );

    if(nlfseGlobal.cryptoBlockSize % nlfseGlobal.cbcBlockSize != 0) {
      //cryptoBlockSize has to be multiplies of cbcBlockSize
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		  "[NLSE]:cryptoBlockSize has to be multiplies of cbcBlockSize.\n" );
      return STATUS_INVALID_PARAMETER;
    }


    // Create process monitor
    status = ProcessMgrCreate();
    if(!NT_SUCCESS(status))
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "[NLSE]:cannot start process manager (%p).\n", status);
        return status;
    }


    // Initialize DRM path list.
    status = NLSEDrmPathListInit();
    if(!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: DRM path list init failed 0x%x.\n", status);
      ProcessMgrClose();
      return status;
    } 

    // Initialize one-shot DRM file list.
    status = NLSEDrmFileOneShotListInit(drvParams.maxDRMFileOneShotCount);
    if(!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: one-shot DRM file list init failed 0x%x.\n", status);
      ProcessMgrClose();
      return status;
    } 

    //  Register with FltMgr
    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &nlfseGlobal.filterHandle );
    if(!NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: FltRegisterFilter failed 0x%x.\n", status);
      ProcessMgrClose();
      return status;      
    }

    //setup communication channel to user mode
    status = SetupCommunication();
    if (!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: Setup communiction failed 0x%x.\n", status);
      FltUnregisterFilter( nlfseGlobal.filterHandle );
      ProcessMgrClose();
      return status;
    }

 
    //  Start filtering i/o
    status = FltStartFiltering( nlfseGlobal.filterHandle );
    if (!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: FltStartFiltering failed 0x%x.\n", status);
      FltUnregisterFilter( nlfseGlobal.filterHandle );
      ProcessMgrClose();
      return status;
    } 

  /*  NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		"[NLSE]: DriverEntry: loading nl_crypto\n");
    if( nl_crypto_initialize(&nlfseGlobal.crypto_ctx,0x0) != 
	NL_CRYPTO_ERROR_SUCCESS )
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: DriverEntry: nl_crypto_initialize failed\n");
    }

    if( nlfseGlobal.crypto_ctx.rand == NULL ||
	nlfseGlobal.crypto_ctx.encrypt == NULL ||
	nlfseGlobal.crypto_ctx.decrypt == NULL )
    {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: DriverEntry: nl_crypto_initialize failed (callback NULL)\n");
      status = STATUS_INSUFFICIENT_RESOURCES;
    }
*/
    if(NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		 ("[NLSE]: Driver loaded successfully.\n") );
    } else {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: Failed to load driver err=0x%x.\n",
		  status);      
      ProcessMgrClose();
    }

    return status;
}


NTSTATUS
FilterUnload (
    __in FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    Called when this mini-filter is about to be unloaded.  We unregister
    from the FltMgr and then return it is OK to unload

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns the final status of this operation.

--*/
{
  int i;

  PAGED_CODE();

  UNREFERENCED_PARAMETER( Flags );

  //Deny non-mandatory unload (e.g. "fltmc unload xxx" command) if PC is
  //running.  (Mandatory unload (e.g. "sc stop xxx" command) is always denied
  //by FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP flag in
  //FilterRegistration struct.)
  if (nlfseGlobal.bEnable)
  {
    return STATUS_FLT_DO_NOT_DETACH;
  }

  NxCleanupNonDrmData();
    
  //Shutdown one-shot DRM file list.
  NLSEDrmFileOneShotListShutdown();

  //Shutdown DRM path list.
  NLSEDrmPathListShutdown();

  //Close communication port
  for(i=0; i<NLSE_MAX_PORT_CONNECTION; i++) {
    if(nlfseGlobal.clientPorts[i].port != NULL) {
      FltCloseClientPort(nlfseGlobal.filterHandle,
			 &((PFLT_PORT)nlfseGlobal.clientPorts[i].port));
      nlfseGlobal.clientPorts[i].port=NULL;
    }
  }
  FltCloseCommunicationPort(nlfseGlobal.serverPort);
  nlfseGlobal.serverPort = NULL;

  //Disable driver's functionality
  ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
  nlfseGlobal.bEnable = FALSE;
  ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);

  //  Unregister from FLT mgr
  FltUnregisterFilter( nlfseGlobal.filterHandle );
  nlfseGlobal.filterHandle = NULL;

  // Close process manager
  ProcessMgrClose();

  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, "[NLSE]: Driver unloaded.\n" );

  NL_KLOG_Shutdown(&nlseKLog);
  return STATUS_SUCCESS;
}

//KLog callback in order to print log on dbgView
static BOOLEAN NLSEKLogCallBack(__in NL_KLOG_LEVEL logLevel, __in_z PCHAR msg)
{
  ASSERT( msg != NULL );
  if((ULONG)logLevel <= nlseLoggingFlags)
  {
    /* The input message already has newline */
    DbgPrint("%s",msg);
  }
  return TRUE;
}

__drv_requiresIRQL(PASSIVE_LEVEL)
VOID
ReadDriverParameters (
    __in PUNICODE_STRING RegistryPath,
    __out PNLSE_DRIVER_PARAMS params
    )
/*++

Routine Description:

    This routine tries to read the driver-specific parameters from
    the registry.  These values will be found in the registry location
    indicated by the RegistryPath passed in.

Arguments:

    RegistryPath - the path key passed to the driver during driver entry.

Return Value:

    None.

--*/
{
  OBJECT_ATTRIBUTES attributes;
  HANDLE driverRegKey;
  NTSTATUS status;
  ULONG resultLength;
  UNICODE_STRING valueName;
  UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( LONG )] = {0};
  NL_KLOG_OPTIONS kLogOps;

  KdPrint(("NLSE: Reading configuration: %wZ\n",RegistryPath));

  // Initialize Logging
  RtlSecureZeroMemory(&kLogOps,sizeof(kLogOps));
  kLogOps.size = sizeof(kLogOps);
  kLogOps.queue_size = 100; /* 100 elements */
  kLogOps.filter = NLSEKLogCallBack;

  //  Definitions to display log messages.  The registry DWORD entry:
  //  "hklm\system\CurrentControlSet\Services\NLSysEncryption\DebugFlags" 
  //defines the logging level
  //  Open the desired registry key
  InitializeObjectAttributes(&attributes,RegistryPath,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			     NULL,NULL);

  status = ZwOpenKey(&driverRegKey,KEY_READ,&attributes );
  if (!NT_SUCCESS( status )) {
    return;
  }

  // Read the given value from the registry.
  RtlInitUnicodeString( &valueName, L"DebugFlags" );
  status = ZwQueryValueKey( driverRegKey,
			    &valueName,
			    KeyValuePartialInformation,
			    buffer,
			    sizeof(buffer),
			    &resultLength );

  if (NT_SUCCESS( status )) {

    ULONG new_log_flags;

    new_log_flags = *((PULONG) &(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
    if( new_log_flags > nlseLoggingFlags )
    {
      nlseLoggingFlags = new_log_flags;
    }
  }


  // Default configuration
  params->maxDRMFileOneShotCount        = 200;

  // maxDRMFileOneShotCount
  RtlInitUnicodeString( &valueName, L"maxDRMFileOneShotCount" );
  status = ZwQueryValueKey( driverRegKey,
			    &valueName,
			    KeyValuePartialInformation,
			    buffer,
			    sizeof(buffer),
			    &resultLength );
  if (NT_SUCCESS( status )) {
    params->maxDRMFileOneShotCount = *((PULONG) &(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
  }

  ZwClose(driverRegKey);

  kLogOps.level = nlseLoggingFlags;
  NL_KLOG_Initialize(&nlseKLog,&kLogOps);
}/* ReadDriverParameters */

__drv_requiresIRQL(PASSIVE_LEVEL)
ULONG
NLGetSystemNameOffset(
                      )
{
    static CHAR* SystemName = "System";    
    PEPROCESS    curproc    = NULL;
    ULONG        i          = 0;

    curproc = PsGetCurrentProcess();
    for( i = 0; i < 3*PAGE_SIZE; i++ )
    {
        if(0 == strncmp(SystemName, (PCHAR)curproc + i, 6))
            return i;
    }
    return 0;
}

__checkReturn
__drv_requiresIRQL(PASSIVE_LEVEL)
const CHAR*
NLGetCurrentProcessName(
                        )
{
    PEPROCESS   curproc = NULL;
    if(0 == g_pnOffset) return NULL;
    curproc = PsGetCurrentProcess();
    if(NULL == curproc) return NULL;
    return ((const CHAR*)curproc + g_pnOffset);
}