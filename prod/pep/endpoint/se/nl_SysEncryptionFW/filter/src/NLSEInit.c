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
#include "nl_crypto.h"
#include "nl_klog.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//Global variable
NLFSE_GLOBAL_DATA nlfseGlobal;
NL_KLOG           nlseKLog;
//variable scoped in this file
ULONG nlseLoggingFlags=0x00000000; 

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
    __in PUNICODE_STRING RegistryPath
    );

KSTART_ROUTINE NLSEAdsWorkerStart;
VOID
NLSEAdsWorkerStart(
                   __in PVOID pCtx
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

FLT_POSTOP_CALLBACK_STATUS
NLFSEOpCallbackPostReadWhenSafe (
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

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, InstanceSetup)
#pragma alloc_text(PAGE, CleanupVolumeContextCallback)
#pragma alloc_text(PAGE, CleanupInstanceContextCallback)
#pragma alloc_text(INIT, DriverEntry)
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
	NlfseStreamHandleContextCleanupCallback,
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

    PAGED_CODE();

    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

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

    try {
      status = NLFSESetInstanceContext(FltObjects);
      if(!NT_SUCCESS(status)) {
	//could not allocate an instance context, quit now
	leave;
      }

      //
      //  Allocate a volume context structure.
      //
      status = FltAllocateContext( FltObjects->Filter,
                                   FLT_VOLUME_CONTEXT,
                                   sizeof(NLFSE_VOLUME_CONTEXT),
                                   NonPagedPool,
                                   &ctx );
      if (!NT_SUCCESS(status)) {
	//
        //  We could not allocate a context, quit now
        //
	leave;
      }

      //
      //  Always get the volume properties, so I can get a sector size
      //
      status = FltGetVolumeProperties( FltObjects->Volume,
				       volProp,
                                       sizeof(volPropBuffer),
                                       &retLen );
      if (!NT_SUCCESS(status)) {
	leave;
      }

      //
      //  Save the sector size in the context for later use.  Note that
      //  we will pick a minimum sector size if a sector size is not
      //  specified.
      //
      ASSERT((volProp->SectorSize == 0) || 
	     (volProp->SectorSize >= MIN_SECTOR_SIZE));

      ctx->SectorSize = max(volProp->SectorSize,MIN_SECTOR_SIZE);

      //Init stream context list of this volume
      InitializeListHead( &ctx->StreamCtxList );
      ExInitializeResourceLite( &ctx->StreamCtxLock );

      //
      //  Init the buffer field (which may be allocated later).
      //
      ctx->Name.Buffer = NULL;

      //
      //  Get the storage device object we want a name for.
      //
      status = FltGetDiskDeviceObject( FltObjects->Volume, &devObj );
      if (NT_SUCCESS(status)) {
	//
	//  Try and get the DOS name.  If it succeeds we will have
	//  an allocated name buffer.  If not, it will be NULL
	//
	//status = RtlVolumeDeviceToDosName( devObj, &ctx->Name );
	status = IoVolumeDeviceToDosName( devObj, &ctx->Name );
      }

      //
      //  If we could not get a DOS name, get the NT name.
      //
      if (NT_SUCCESS(status))
      {
        // check if the ctx->Name is like "\\?\Volume{ae3644a0-9638-11df-aec6-00155d63712f}"
	ASSERT( ctx->Name.Buffer != NULL );
        if(NULL != ctx->Name.Buffer)
        {
            if(ctx->Name.Length >= 4 && 
                ctx->Name.Buffer[0] == L'\\' &&
                ctx->Name.Buffer[1] == L'\\' &&
                ctx->Name.Buffer[2] == L'?' &&
                ctx->Name.Buffer[3] == L'\\')
            {
                NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
                    "NLSE!InstanceSetup: IoVolumeDeviceToDosName return volume name, Name=\"%wZ\"\n",
                    &ctx->Name);
            }
        }
      }
      else
      {
	ASSERT(ctx->Name.Buffer == NULL);

	//
	//  Figure out which name to use from the properties
	//
	if (volProp->RealDeviceName.Length > 0) {
	  workingName = &volProp->RealDeviceName;
	} else if (volProp->FileSystemDeviceName.Length > 0) {
	  workingName = &volProp->FileSystemDeviceName;
	} else {
	  //
	  //  No name, don't save the context
	  //
	  status = STATUS_FLT_DO_NOT_ATTACH;
	  leave;
	}

	//
	//  Get size of buffer to allocate.  This is the length of the
	//  string plus room for a trailing colon.
	//
	size = workingName->Length + sizeof(WCHAR);

	//
	// Now allocate a buffer to hold this name
	// It is cleaned up at CleanupVolumeContextCallback
    // Disable preFast warning here because Release in different function
    // cause incorrect preFast warning 6014 - memory Leak
#pragma prefast(disable:6014, "Release this memory in different function cause incorrect preFast warning 6014 - memory Leak") 
	ctx->Name.Buffer = ExAllocatePoolWithTag( NonPagedPool,
                                                  size,
                                                  NLFSE_NAME_TAG );
#pragma prefast(enable:6014, "recover this warning")

	if (ctx->Name.Buffer == NULL) {
	  status = STATUS_INSUFFICIENT_RESOURCES;
	  leave;
	}

        //
	//  Init the rest of the fields
	//
	ctx->Name.Length = 0;
	ctx->Name.MaximumLength = size;

	//
	//  Copy the name in
	//
	RtlCopyUnicodeString( &ctx->Name,
			      workingName );

	//
	//  Put a trailing colon to make the display look good
	//
	RtlAppendUnicodeToString( &ctx->Name,
				  L":" );
      }

      //
      //  Set the context
      //
      status = FltSetVolumeContext( FltObjects->Volume,
                                    FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                                    ctx,
                                    NULL );

      //
      //  It is OK for the context to already be defined.
      //
      if (status == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
	status = STATUS_SUCCESS;
      }

      if(status != STATUS_SUCCESS) {
	//
	//  Log debug info
	//
	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR,
		    "NLSE!InstanceSetup: Failed. Real SectSize=0x%04x, Used SectSize=0x%04x, Name=\"%wZ\"\n",
		    volProp->SectorSize,
		    ctx->SectorSize,
		    &ctx->Name);
      } else {
	NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG,
		    "NLSE!InstanceSetup: Succeed. Real-SectSize=0x%04x, Used-SectSize=0x%04x, Name=\"%wZ\"\n",
		    volProp->SectorSize,
		    ctx->SectorSize,
		    &ctx->Name);
      }
    } finally {

        //
        //  Always release the context.  If the set failed, it will free the
        //  context.  If not, it will remove the reference added by the set.
        //  Note that the name buffer in the ctx will get freed by the context
        //  cleanup routine.
        //

        if (ctx) {

            FltReleaseContext( ctx );
        }

        //
        //  Remove the reference added to the device object by
        //  FltGetDiskDeviceObject.
        //

        if (devObj) {

            ObDereferenceObject( devObj );
        }
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

    if (ctx->Name.Buffer != NULL) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		  "NLSE:CleanupVolumeContextCallback: %wZ\n", &ctx->Name);
      ExFreePoolWithTag(ctx->Name.Buffer, NLFSE_NAME_TAG);
      ctx->Name.Buffer = NULL;
      ctx->Name.Length=0;
      ctx->Name.MaximumLength=0;
    }
    
    ExDeleteResourceLite( &ctx->StreamCtxLock );

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
    NTSTATUS        status;
    HANDLE          hThread=NULL;

#if (NLSE_DEBUG_DATA_VERIFY || NLSE_DEBUG_FAKE_FILE_KEY || NLSE_DEBUG_CRYPTO_PASSTHROUGH)
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

    //Initialize the global 
    RtlZeroMemory(&nlfseGlobal, sizeof(nlfseGlobal));
    nlfseGlobal.bEnable=FALSE;
    nlfseGlobal.adsWorkQueueSize=0L;
    nlfseGlobal.encryptWorkQueueSize=0L;
    nlfseGlobal.cryptoBlockSize=512; //512 bytes
    nlfseGlobal.cbcBlockSize=16; //16 bytes
    nlfseGlobal.maxWriteBlockSize=65536; //64K bytes -- IT MUST BE (N * cryptoBlockSize)
    ExInitializeFastMutex(&nlfseGlobal.currentPCKeyLock);
    nlfseGlobal.hasCurrentPCKey = FALSE;
    ExInitializeFastMutex(&nlfseGlobal.enableStatusMutex);
    InitializeListHead(&nlfseGlobal.adsWorkQueue);
    KeInitializeSpinLock(&nlfseGlobal.adsWorkQueueSpinLock);

    nlfseGlobal.log_file_set = FALSE;
    nlfseGlobal.CryptoInit = FALSE;
    nlfseGlobal.CryptoInitSuccess = TRUE;

    //  Get debug trace flags
    ReadDriverParameters( RegistryPath );

    if(nlfseGlobal.cryptoBlockSize % nlfseGlobal.cbcBlockSize != 0) {
      //cryptoBlockSize has to be multiplies of cbcBlockSize
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR,
		  "[NLSE]:cryptoBlockSize has to be multiplies of cbcBlockSize.\n" );
      return STATUS_INVALID_PARAMETER;
    }

    //  Init lookaside list used to allocate our context structure used to
    //  pass information from out preOperation callback to our postOperation
    //  callback.
    //
    ExInitializeNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList,
				     NULL,
				     NULL,
				     0,
                                     sizeof(NLSE_ENCRYPT_FILE_QUEUE_CONTEXT),
                                     NLSE_ENCRYPT_FILE_QUEUE_TAG,
                                     0 );
    ExInitializeNPagedLookasideList( &nlfseGlobal.pre2PostContextList,
				     NULL,
				     NULL,
				     0,
                                     sizeof(NLFSE_PRE_2_POST_CONTEXT),
                                     NLFSE_PRE_2_POST_TAG,
                                     0 );
    ExInitializeNPagedLookasideList( &nlfseGlobal.queueContextLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof( NLSE_PENDING_IO_QUEUE_CONTEXT ),
                                     NLFSE_QUEUE_CONTEXT_TAG,
                                     0 );
    ExInitializeNPagedLookasideList( &nlfseGlobal.irpEntryLookaside,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof( NLFSE_IRP_ENTRY ),
                                     NLFSE_IRP_ENTRY_TAG,
                                     0 );

    //Setup ADS work thread
    // Initialize the event
    KeInitializeEvent(&nlfseGlobal.adsWorkerThreadEvent, 
		      SynchronizationEvent, 
		      FALSE);
    // Start the worker threads.
    status=PsCreateSystemThread(&hThread,
				(ACCESS_MASK)0L,
				0,
				0,
				0,
				NLSEAdsWorkerStart, // Initial function to call
				NULL); // Parameter to pass
    if(!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: Start ads worker thread failed 0x%x.\n", status);
      return status;
    } 

    // Initialize DRM path list.
    status = NLSEDrmPathListInit();
    if(!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: DRM path list init failed 0x%x.\n", status);
      return status;
    } 

    //  Register with FltMgr
    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &nlfseGlobal.filterHandle );
    if(!NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: FltRegisterFilter failed 0x%x.\n", status);
      return status;      
    }

    //setup communication channel to user mode
    status = SetupCommunication();
    if (!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: Setup communiction failed 0x%x.\n", status);
      FltUnregisterFilter( nlfseGlobal.filterHandle );
      return status;
    }

 
    //  Start filtering i/o
    status = FltStartFiltering( nlfseGlobal.filterHandle );
    if (!NT_SUCCESS( status )) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: FltStartFiltering failed 0x%x.\n", status);
      FltUnregisterFilter( nlfseGlobal.filterHandle );
      return status;
    } 

    if(NT_SUCCESS(status)) {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_DEBUG, 
		 ("[NLSE]: Driver loaded successfully.\n") );
    } else {
      NL_KLOG_Log(&nlseKLog, NL_KLOG_LEVEL_ERR, 
		  "[NLSE]: Failed to load driver err=0x%x.\n",
		  status);
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

  //
  //  Delete lookaside list
  //
  ExDeleteNPagedLookasideList( &nlfseGlobal.fileEncryptCtxList );
  ExDeleteNPagedLookasideList( &nlfseGlobal.pre2PostContextList );
  ExDeleteNPagedLookasideList( &nlfseGlobal.queueContextLookaside);
  ExDeleteNPagedLookasideList( &nlfseGlobal.irpEntryLookaside);
    
  NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_DEBUG, "[NLSE]: Driver unloaded.\n" );

  NL_KLOG_Shutdown(&nlseKLog);
  return STATUS_SUCCESS;
}

//KLog callback in order to print log on dbgView
static BOOLEAN NLSEKLogCallBack(__in NL_KLOG_LEVEL logLevel, __in PCHAR msg)
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
    __in PUNICODE_STRING RegistryPath
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

  ZwClose(driverRegKey);

  kLogOps.level = nlseLoggingFlags;
  NL_KLOG_Initialize(&nlseKLog,&kLogOps);
}/* ReadDriverParameters */
