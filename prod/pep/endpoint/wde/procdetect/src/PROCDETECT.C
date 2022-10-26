#include <ntiologc.h>
#include <ntifs.h>
#include "ipcpolicy.h"

#include "nlcc.h"
#include "nlcc_klib.h"

#define NT_DEVICE_NAME	        L"\\Device\\ProcessPEP"
#define DOS_DEVICE_NAME         L"\\DosDevices\\ProcessPEP"

int gPoolType = NonPagedPool;

__drv_dispatchType(IRP_MJ_CREATE)
__drv_dispatchType(IRP_MJ_CLEANUP)
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DriverDispatch;

DRIVER_UNLOAD DriverUnload;

/* Prototype required */
NTSTATUS ZwQueryInformationProcess(HANDLE ProcessHandle,
				   PROCESSINFOCLASS ProcessInformationClass,
				   PVOID ProcessInformation,
				   ULONG ProcessInformationLength,
				   PULONG ReturnLength);

/* Work item for async processing of RUN control.
 */
typedef struct
{
  PIO_WORKITEM work_item;              /* Work item for system worker thread */
  UNICODE_STRING process_image_path;   /* Proces image path to query for */
  HANDLE process_id;                   /* Process ID to query for */
} PROCDETECT_WI, *PPROCDETECT_WI;

PDEVICE_OBJECT          g_DeviceObject;

NLCC_HANDLE nlcc_handle;

VOID
DriverUnload(
         __in PDRIVER_OBJECT DriverObject
	);

NTSTATUS
DriverEntry(
            __in PDRIVER_OBJECT  DriverObject,
            __in PUNICODE_STRING RegistryPath
            );

NTSTATUS
DriverDispatch(
              __in PDEVICE_OBJECT DeviceObject,
              __in PIRP Irp
              );

VOID
NotifyLoadImage(
                __in PUNICODE_STRING  FullImageName,
                __in HANDLE  ProcessId, // where image is mapped
                __in PIMAGE_INFO  ImageInfo
                );

HANDLE
GetProcessHandleById(
                     HANDLE ProcessId
                     );

NTSTATUS
Request2Policy(
               __in PUNICODE_STRING FullImageName,
               __in HANDLE ProcessId,
               __in HANDLE ProcessHandle,
               OUT PULONG Allow
               );

NTSTATUS
SymbolicName2DeviceName(
                        __in PUNICODE_STRING SymbolicName,
                        OUT PUNICODE_STRING DeviceName
                        );
VOID
DevicePath2SymbolPath(
                      __in PUNICODE_STRING DevicePath,
                      __in OUT PUNICODE_STRING SymbolPath
                      );
BOOLEAN
IsExeFile(
          __in PUNICODE_STRING ImageName
          );

/************************************************************************/
/*      Routines                                                        */
/************************************************************************/
//#ifdef DBG
VOID
DriverUnload(
         __in PDRIVER_OBJECT DriverObject
         )
{
		if(nlcc_handle.dev_obj && nlcc_handle.fo_obj)
		{
			NLCC_KClose(&nlcc_handle);
		}
    KdPrint(("PD Unload Exit!\n"));
}
//#endif

NTSTATUS
DriverEntry(
            __in PDRIVER_OBJECT  DriverObject,
            __in PUNICODE_STRING RegistryPath
            )
{
    NTSTATUS        status;
    UNICODE_STRING  NtDeviceName;
    UNICODE_STRING  DosDeviceName;
    ULONG           i;

    if (RtlIsNtDdiVersionAvailable(0x06020000) )  // NTDDI_WIN8
    {
    	gPoolType = 512;  // NonPagedPoolNx	
    }
	
    // Globals
		//	open NLCC first
		KdPrint(("ProcDetect: Open NLCC\n"));
		status = NLCC_KOpen(&nlcc_handle);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("PROCDETECT! DriverEntry fail to NLCC_KOpen! return with error! [0x%0x]\n", status));
			return status;
		}
		KdPrint(("ProcDetect: Opened NLCC\n"));

    RtlInitUnicodeString(&NtDeviceName, NT_DEVICE_NAME);
    status = IoCreateDevice(DriverObject, 0, &NtDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &g_DeviceObject);
    if(!NT_SUCCESS(status))
    {
				KdPrint(("PROCDETECT! DriverEntry fail to create device!\n"));
				NLCC_KClose(&nlcc_handle);
        return status;
    }

    RtlInitUnicodeString(&DosDeviceName, DOS_DEVICE_NAME);
    status = IoCreateSymbolicLink(&DosDeviceName, &NtDeviceName);
    if(!NT_SUCCESS(status))
    {
				KdPrint(("PROCDETECT! DriverEntry fail to create symbol!\n"));
				NLCC_KClose(&nlcc_handle);
        if(g_DeviceObject) IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = NULL;
        return status;
    }

    status = PsSetLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)NotifyLoadImage);
    if(!NT_SUCCESS(status))
    {
        KdPrint(("PROCDETECT! DriverEntry fail to set notify load image routine!\n"));
				NLCC_KClose(&nlcc_handle);
        IoDeleteSymbolicLink(&DosDeviceName);
        if(g_DeviceObject) IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = NULL;
        return status;
    }

    g_DeviceObject->Flags |= DO_BUFFERED_IO;
    for ( i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        DriverObject->MajorFunction[i] = DriverDispatch; 
    }
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = DriverDispatch; //PD_Open;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = DriverDispatch; //PD_Cleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = DriverDispatch; //PD_IoControl;
    DriverObject->DriverUnload = NULL; //DriverUnload;

		KdPrint(("DriverEntry OK!\n"));
    return STATUS_SUCCESS;
}

NTSTATUS
DriverDispatch(
              __in PDEVICE_OBJECT DeviceObject,
              __in PIRP Irp
              )
{
    NTSTATUS	status = STATUS_SUCCESS;
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

/** ProcDetect_FreeWorkItem
 *
 *  \brief Free an allocated work item.
 */
static VOID ProcDetect_FreeWorkItem( __in PPROCDETECT_WI wi )
{
  if( wi != NULL )
  {
    if( wi->work_item )
    {
      IoFreeWorkItem(wi->work_item);
    }

    if( wi->process_image_path.Buffer )
    {
      ExFreePool(wi->process_image_path.Buffer);
    }

    ExFreePool(wi);
  }
}/* ProcDetect_FreeWorkItem */

IO_WORKITEM_ROUTINE ProcDetectWorkItemCallback;

/** ProcDetectWorkItemCallback
 *
 *  \brief Process work item for RUN policy decision
 */
VOID ProcDetectWorkItemCallback( __in PDEVICE_OBJECT device_object,
				 __in PVOID context )
{
  NTSTATUS             Status;
  HANDLE               ProcessHandle = NULL;
  ULONG                Allow;
  UNICODE_STRING       SystemAccountName;
  UNICODE_STRING       AccountName;
  WCHAR                AccountNameBuffer[260] = {0};
  PUNICODE_STRING image_path = NULL;
  PPROCDETECT_WI ctx = NULL;

  ctx = (PPROCDETECT_WI)context;

  ASSERT( ctx != NULL );
  ASSERT( ctx->process_image_path.Buffer != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  RtlInitUnicodeString(&SystemAccountName,L"SYSTEM");
  RtlZeroMemory(AccountNameBuffer,sizeof(AccountNameBuffer));
  RtlInitEmptyUnicodeString(&AccountName,AccountNameBuffer,sizeof(AccountNameBuffer));

  Status = GetProcessUserAccount(ctx->process_id,&AccountName);

  if( RtlCompareUnicodeString(&AccountName,&SystemAccountName,TRUE) == 0 )
  {
    KdPrint(("ProcDetectWorkItemCallback: We don't care about SYSTEM process!\n"));
    goto ProcDetectWorkItemCallback_complete;
  }

  ProcessHandle = GetProcessHandleById(ctx->process_id);
  if( ProcessHandle == NULL )
  {
    KdPrint(("ProcDetectWorkItemCallback: Fail to open process: PID=0x%x (%d), Name=%wZ\n",
	     ctx->process_id, ctx->process_id, &ctx->process_image_path));
    goto ProcDetectWorkItemCallback_complete;
  }

  image_path = (PUNICODE_STRING)ExAllocatePoolWithTag(PagedPool,
	  1024 * sizeof(wchar_t),
	  PROCDETECT_POOL_TAG);
  if( image_path == NULL )
  {
	goto ProcDetectWorkItemCallback_complete;
  }

  Status = ZwQueryInformationProcess(ProcessHandle,ProcessImageFileName,image_path,
	  (1024 * sizeof(wchar_t)) - sizeof(UNICODE_STRING),
	  NULL);
	if( !NT_SUCCESS(Status) )
	{
		goto ProcDetectWorkItemCallback_complete;
	}

	/* Notification of image load can occur in the context of the parent
	* process, so ProcessId is the PID of the arent.  This case must be
	* avoided for evaluation to avoid terminating the parent.  The image
	* path is always for the correct process, but the ProcessId can change
	* based on mapping behavior of OS.
	*/
  if( RtlCompareUnicodeString(&ctx->process_image_path,image_path,TRUE) != 0 )
  {
	  goto ProcDetectWorkItemCallback_complete;
  }

  KdPrint(("ProcDetectWorkItemCallback: [%4d] %wZ\n",
	   ctx->process_id, &ctx->process_image_path));

  Status = Request2Policy(&ctx->process_image_path, ctx->process_id, ProcessHandle, &Allow);
  KdPrint(("ProcDetectWorkItemCallback: %s : PID=0x%x (%d), Name=%wZ\n",
	   !Allow ? "DENY" : "ALLOW",
	   ctx->process_id, ctx->process_id, &ctx->process_image_path));
  if(NT_SUCCESS(Status) && !Allow)
  {
    Status = ZwTerminateProcess(ProcessHandle,0);
    KdPrint(("ProcDetectWorkItemCallback: ZwTerminateProcess (0x%x)\n",Status));
  }
  

 ProcDetectWorkItemCallback_complete:

	if (ProcessHandle)
	{
		ZwClose(ProcessHandle);
	}
	

	if( image_path != NULL )
	{
		ExFreePool(image_path);
	}

  ProcDetect_FreeWorkItem(ctx);

}/* ProcDetectWorkItemCallback */

VOID
NotifyLoadImage(
                __in PUNICODE_STRING  FullImageName,
                __in HANDLE  ProcessId, // where image is mapped
                __in PIMAGE_INFO  ImageInfo
                )
{
  // We don't care about the "SYSTEM" process
  if( IsExeFile(FullImageName) ) 
  {
	PWCHAR buf = NULL;
    PPROCDETECT_WI ctx = NULL;  /* ProcDetect work item */

    /* Set PID and copy image path */
    ctx = ExAllocatePoolWithTag(PagedPool,sizeof(PROCDETECT_WI),PROCDETECT_POOL_TAG);
    if( ctx == NULL )
    {
      return;
    }
    RtlZeroMemory(ctx,sizeof(PROCDETECT_WI));
    ctx->work_item = IoAllocateWorkItem(g_DeviceObject);
    if( ctx->work_item == NULL )
    {
      ProcDetect_FreeWorkItem(ctx);
      return;
    }
    ctx->process_id = ProcessId;

    KdPrint(("NotifyLoadImage: [%4d] %wZ\n", ProcessId, FullImageName));

    /* Copy full image path into context element */
    buf = ExAllocatePoolWithTag(PagedPool,FullImageName->MaximumLength,PROCDETECT_POOL_TAG);
    if( buf == NULL )
    {
      ProcDetect_FreeWorkItem(ctx);
      return;
    }
    RtlInitEmptyUnicodeString(&ctx->process_image_path,buf,FullImageName->MaximumLength);
    RtlCopyUnicodeString(&ctx->process_image_path,FullImageName);

    /* Post work item */
    IoQueueWorkItem(ctx->work_item,ProcDetectWorkItemCallback,CriticalWorkQueue,(PVOID)ctx);
  }
}/* NotifyLoadImage */

__checkReturn HANDLE
GetProcessHandleById(
                     __in HANDLE ProcessId
                     )
{
    NTSTATUS             Status;
    OBJECT_ATTRIBUTES    ObjAttrib;
    CLIENT_ID            Clientid;
    HANDLE               ProcessHandle;
    memset(&ObjAttrib, 0, sizeof(OBJECT_ATTRIBUTES));
    ObjAttrib.Length = sizeof(OBJECT_ATTRIBUTES);
    Clientid.UniqueProcess  = ProcessId;
    Clientid.UniqueThread   = 0;
    Status = ZwOpenProcess(&ProcessHandle, GENERIC_ALL, &ObjAttrib, &Clientid);
    return NT_SUCCESS(Status)?ProcessHandle:NULL;
}

NTSTATUS
Request2Policy(
               __in PUNICODE_STRING FullImageName,
               __in HANDLE ProcessId,
               __in HANDLE ProcessHandle,
               __out PULONG Allow
               )
{
    NTSTATUS                Status  = STATUS_SUCCESS;
    PNLCC_QUERY query_buf = NULL;
    PNLCC_QUERY request = NULL;
    PNLCC_QUERY response = NULL;
    UNICODE_STRING nlcc_attr_key;
    UNICODE_STRING nlcc_attr_value;
    unsigned char* buf = NULL;
    ULONG buf_size = 1024 / sizeof(wchar_t);

    // set default value
    *Allow     = 1;

    buf = (unsigned char*)ExAllocatePoolWithTag(gPoolType,1024,PROCDETECT_POOL_TAG);
    if( buf == NULL )
    {
        KdPrint(("Request2Policy! Fail to allocate query\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
	goto Request2Policy_complete;
    }

    query_buf = (PNLCC_QUERY)ExAllocatePoolWithTag(gPoolType,sizeof(NLCC_QUERY)*2,PROCDETECT_POOL_TAG);
    if( query_buf == NULL )
    {
        KdPrint(("Request2Policy! Fail to allocate query\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
	goto Request2Policy_complete;
    }

    request  = &query_buf[0];  /* request */
    response = &query_buf[1];  /* response */

    // Initialize
    NLCC_KInitializeQuery(request);
    NLCC_KInitializeQuery(response);

    /* Action */
    RtlInitUnicodeString(&nlcc_attr_key,L"action");
    RtlInitUnicodeString(&nlcc_attr_value,L"RUN");
    NLCC_KAddAttribute(request,&nlcc_attr_key,&nlcc_attr_value);

    /* Normalize path to symbolic (i.e., C:\...) from volume based for app and source */
    RtlInitEmptyUnicodeString(&nlcc_attr_value,(PWCHAR)buf,(USHORT)buf_size);
    DevicePath2SymbolPath(FullImageName,&nlcc_attr_value);

    /* Application */
    RtlInitUnicodeString(&nlcc_attr_key,L"application");
    NLCC_KAddAttribute(request,&nlcc_attr_key,&nlcc_attr_value);

    /* Source name */
    RtlInitUnicodeString(&nlcc_attr_key,L"source_name");
    NLCC_KAddAttribute(request,&nlcc_attr_key,&nlcc_attr_value);

    /* Source type */
    RtlInitUnicodeString(&nlcc_attr_key,L"source_type");
    RtlInitUnicodeString(&nlcc_attr_value,L"fso");
    NLCC_KAddAttribute(request,&nlcc_attr_key,&nlcc_attr_value);

    /* User ID (default unknown) */
    RtlInitUnicodeString(&nlcc_attr_key,L"user_id");
    RtlInitUnicodeString(&nlcc_attr_value,L"unknown");
    if( GetSID(ProcessHandle,buf,TokenUser,&buf_size,NULL) == STATUS_SUCCESS )
    {
      /* buf size is in bytes, not chars and buf is not terminated */
      wchar_t* p = (wchar_t*)buf;    
      p[buf_size/sizeof(wchar_t)] = (wchar_t)NULL;
      RtlInitUnicodeString(&nlcc_attr_value,(PCWSTR)buf);
    }
    NLCC_KAddAttribute(request,&nlcc_attr_key,&nlcc_attr_value);

    request->info.request.ip = 0; // localhost
    request->info.request.pid = (long)ProcessId;
    request->info.request.event_level = 3;

		if(nlcc_handle.dev_obj && nlcc_handle.fo_obj)
		{
			Status = NLCC_KQuery(&nlcc_handle,request,response,NULL);
		}

    if( Status == STATUS_SUCCESS && response->info.response.allow == NLCC_POLICY_RESULT_DENY )
    {
      *Allow = 0;
    }

 Request2Policy_complete:

    if( buf != NULL )
    {
      ExFreePoolWithTag(buf,PROCDETECT_POOL_TAG);
    }
    if( query_buf != NULL )
    {
      ExFreePoolWithTag(query_buf,PROCDETECT_POOL_TAG);
    }
    return Status;
}

NTSTATUS
SymbolicName2DeviceName(
                        __in PUNICODE_STRING SymbolicName,
                        __out PUNICODE_STRING DeviceName
                        )
{
    OBJECT_ATTRIBUTES   oa; 
    NTSTATUS            status; 
    HANDLE              h; 

    InitializeObjectAttributes(&oa, SymbolicName, OBJ_CASE_INSENSITIVE, 0, 0); 

    status = ZwOpenSymbolicLinkObject(&h, GENERIC_READ, &oa); 
    if (!NT_SUCCESS(status))
    {
        //KdPrint(("SymbolName2deviceName! Fial to OpenSymbolicLinkObject (%wZ)\n", SymbolicName));
        return status; 
    }

    status = ZwQuerySymbolicLinkObject(h, DeviceName, NULL);
    //KdPrint(("SymbolName2deviceName: '%wZ' -> '%wZ'\n", SymbolicName, DeviceName));
    ZwClose(h);
    return status;
}

VOID
DevicePath2SymbolPath(
                      __in PUNICODE_STRING DevicePath,
                      __inout PUNICODE_STRING SymbolPath
                      )
{
#define MAX_DRIVER_LETTER           26
#define MAX_DRIVER_LETTER_BUFSIZE   4
#define MAX_DRIVER_DEVNAME_SIZE     520
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    USHORT          i=0;
    UNICODE_STRING  DriverLetterName;
    UNICODE_STRING  DeviceDriverName;
    WCHAR          *DevicedriverNameBuffer = NULL;
    UNICODE_STRING  TempDriverName;
    WCHAR          *TempDriverNameBuffer   = NULL;
    WCHAR           DefaultLetter[7] = L"\\??\\C:";

    RtlInitUnicodeString(&DriverLetterName, DefaultLetter);
    DevicedriverNameBuffer = (WCHAR*)ExAllocatePoolWithTag(gPoolType, MAX_DRIVER_DEVNAME_SIZE, PROCDETECT_POOL_TAG);
    if(NULL==DevicedriverNameBuffer) goto FAIL_EXIT;
    TempDriverNameBuffer = (WCHAR*)ExAllocatePoolWithTag(gPoolType, MAX_DRIVER_DEVNAME_SIZE, PROCDETECT_POOL_TAG);
    if(NULL==TempDriverNameBuffer)   goto FAIL_EXIT;
    
    RtlInitEmptyUnicodeString(&DeviceDriverName, DevicedriverNameBuffer, sizeof(DevicedriverNameBuffer));
    RtlInitEmptyUnicodeString(&TempDriverName, TempDriverNameBuffer, sizeof(TempDriverNameBuffer));
    DeviceDriverName.MaximumLength = MAX_DRIVER_DEVNAME_SIZE;
    TempDriverName.MaximumLength   = MAX_DRIVER_DEVNAME_SIZE;
    for(i='A'; i<='Z'; i++)
    {
        DriverLetterName.Buffer[4] = i;
        Status = SymbolicName2DeviceName(&DriverLetterName, &DeviceDriverName);
        if(!NT_SUCCESS(Status)) continue;

        if(DevicePath->Length>DeviceDriverName.Length+2)
        {
            RtlZeroMemory(TempDriverNameBuffer, MAX_DRIVER_DEVNAME_SIZE);
            RtlCopyMemory(TempDriverNameBuffer, DevicePath->Buffer, DeviceDriverName.Length);
            TempDriverName.Length = DeviceDriverName.Length;
            if(RtlEqualUnicodeString(&TempDriverName, &DeviceDriverName, TRUE))
            {
                // we know what is the driver letter
                SymbolPath->Buffer[0] = i;
                SymbolPath->Buffer[1] = L':';
                RtlCopyMemory(&(SymbolPath->Buffer[2]), (DevicePath->Buffer+DeviceDriverName.Length/2), (DevicePath->Length-DeviceDriverName.Length));
                SymbolPath->Length = 4 + (DevicePath->Length-DeviceDriverName.Length);
                SymbolPath->Buffer[SymbolPath->Length/2] = 0;
                ExFreePool(TempDriverNameBuffer);
                ExFreePool(DevicedriverNameBuffer);
                return;
            }          
        }
    }

FAIL_EXIT:
    RtlCopyUnicodeString(SymbolPath, DevicePath);
    if(NULL!=TempDriverNameBuffer)   ExFreePool(TempDriverNameBuffer); 
    if(NULL!=DevicedriverNameBuffer) ExFreePool(DevicedriverNameBuffer);
}

__checkReturn BOOLEAN
IsExeFile(
          __in PUNICODE_STRING ImageName
          )
{
    if(ImageName && ImageName->Length > 8)  // L".EXE" length is 8
    {
        ULONG StrLen = ImageName->Length/2;
        if((L'E'== ImageName->Buffer[StrLen-1] || L'e'== ImageName->Buffer[StrLen-1])
            && (L'X'== ImageName->Buffer[StrLen-2] || L'x'== ImageName->Buffer[StrLen-2])
            && (L'E'== ImageName->Buffer[StrLen-3] || L'e'== ImageName->Buffer[StrLen-3])
            && L'.'== ImageName->Buffer[StrLen-4]
            )
        {
            return TRUE;
        }
    }
    return FALSE;
}
