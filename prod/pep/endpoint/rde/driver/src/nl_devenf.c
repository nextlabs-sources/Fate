/*******************************************************************************
 *
 * NextLabs Device Enforcer
 *
 * Filter PnP device as they are added to the system.  When a device should not
 * be used all IO direct to that device is denied.
 *
 *
 * Sequence:
 *
 *     Device arrival  -> WdfFltrEvtDeviceAdd() called.
 *                        Device information is placed on device list.
 *
 * Control Device:
 *
 *     User-mode waits for device arrival.  Device list can be retreived for
 *     evaluation.  Device(s) can be disabled.
 *
 *
 * Arrival State
 *
 *     The arrival state (device_arrival_event) is only signaled when a devce
 *     has arrived.  If any I/O is pended waiting for an arrival, the arrival
 *     state will not be signaled.  That is, the arrival state is only signaled
 *     when a device has arrived and there has not been a wait condition.
 *
 ******************************************************************************/
#ifdef _AMD64_
#define _X86AMD64_
#endif

#include <fltKernel.h>
#include <usbiodef.h>
#include <devguid.h>
#include <ntddk.h>
#include <wdf.h>
#include <wdmsec.h>

#include "nl_devenf.h"

NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject,
		      PUNICODE_STRING RegistryPath );
EVT_WDF_DRIVER_DEVICE_ADD WdfFltrEvtDeviceAdd;
EVT_WDF_DEVICE_SELF_MANAGED_IO_FLUSH EvtDeviceSelfManagedIoFlush;
EVT_WDF_IO_QUEUE_IO_DEFAULT EvtIoDefaultControlDevice;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtIoDeviceControlControlDevice;
EVT_WDF_OBJECT_CONTEXT_CLEANUP MyEvtCleanupCallback;

/* INIT - remove from kernel memory after running */
#ifdef ALLOC_PRAGMA
  #pragma alloc_text(INIT,DriverEntry)
#endif

/** DeviceContext
 *
 *  \brief Contextual information for a device which has arrived.
 */
typedef struct
{
  LIST_ENTRY list_entry;   /* List management */
  DeviceInfo dev_info;     /* Device information */
} DeviceContext;

/** DriverContext
 *
 *  \brief Contextual information for the nl_devenf driver instance.
 */
typedef struct
{
  KEVENT device_arrival_event;    /* Signal device arrival */
  WDFDEVICE control_device;       /* Control device */
  WDFQUEUE waitQueue;             /* 'wait for arrival' request queue */
  FAST_MUTEX mutex;               /* Mutex for device list */
  LIST_ENTRY device_list;         /* Device list */
  LONG device_number;             /* Device number - increment per arrival */
} DriverContext;

WDF_DECLARE_CONTEXT_TYPE(DeviceContext);
WDF_DECLARE_CONTEXT_TYPE(DriverContext);

static VOID MyEvtCleanupCallback( __in WDFOBJECT Object )
{
  KdPrint(("NLDevEnf: MyEvtCleanupCallback: Object = 0x%x\n",Object));
}

/** EvtIoDeviceControlControlDevice
 *
 *  Custom IO control of device.
 */
static VOID EvtIoDeviceControlControlDevice(__in WDFQUEUE Queue,
					     __in WDFREQUEST Request,
					     __in size_t OutputBufferLength,
					     __in size_t InputBufferLength,
					     __in ULONG IoControlCode )
{
  NTSTATUS status;
  size_t in_size = 0;            /* Input */
  PVOID in_buf = NULL;
  size_t out_size = 0;           /* Output */
  PVOID out_buf = NULL;
  DriverContext* drv_ctx = NULL; /* Driver Context */

  VERIFY_IS_IRQL_PASSIVE_LEVEL();

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  WDFVERIFY( drv_ctx != NULL );

  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IoControlCode 0x%x : bytes %d (in) %d (out)\n",
	   IoControlCode, InputBufferLength, OutputBufferLength));

  /***********************************************************************
   * Read input/output buffers and sizes
   **********************************************************************/
  status = WdfRequestRetrieveInputBuffer(Request,in_size,&in_buf,&in_size);
  if( !NT_SUCCESS(status) )
  {
    in_buf = NULL;
  }

  status = WdfRequestRetrieveOutputBuffer(Request,out_size,&out_buf,&out_size);
  if( !NT_SUCCESS(status) )
  {
    out_buf = NULL;
  }

  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: in_buf 0x%x in_size %d out_buf 0x%x out_size %d\n",
	   in_buf, in_size, out_buf, out_size));

  /***********************************************************************
   * Process I/O from user-mode.  Each IOCTL must complete its request.
   **********************************************************************/
  switch( IoControlCode )
  {
    case IOCTL_NL_DEVENF_GET_ALL_DEVICES:
    {
      status = STATUS_SUCCESS;
      ExAcquireFastMutex(&drv_ctx->mutex);
      __try
      {
	/* There are three cases:
	 *  (1) User-mode supplied buffer which is a multiple of sizeof(DeviceInfo).
	 *      Device information copied into the user-mode buffer until that space is
	 *      exceeded.  When the buffer is exceeded STATUS_MORE_ENTRIES is returned.
	 *  (2) User-mode did not supply a buffer and this is a request for device list
	 *      size.  The size is supplied as the information parameter of the completed
	 *      request.
	 *  (3) Received unexpected set of parameters such as input size.
	 *
	 *  Notes: Size required for input is a multiple of sizeof(DeviceInfo).  This is
	 *         due to the user-mode API use of required number of elements rather than
	 *         bytes for reading the device list.
	 */
	PLIST_ENTRY node = NULL;
	DeviceInfo* dev_info = (DeviceInfo*)out_buf;
	if( dev_info != NULL && out_size > 0 && (out_size % sizeof(DeviceInfo)) == 0 )
        {
	  size_t new_out_size = 0; /* output size in bytes */
	  for( node = drv_ctx->device_list.Flink ; node != (PLIST_ENTRY)&drv_ctx->device_list ; node = node->Flink )
	  {
	    DeviceContext* dev_ctx = (DeviceContext*)node;

	    /* If out of user-mode supplied space stop and indicate there is more to read */
	    if( (new_out_size + sizeof(DeviceInfo)) > out_size )
	    {
	      KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: STATUS_MORE_ENTRIES\n"));
	      status = STATUS_MORE_ENTRIES;
	      break;
	    }

	    KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: dev_info 0x%p\n",dev_info));
	    RtlCopyMemory(dev_info,&dev_ctx->dev_info,sizeof(DeviceInfo));
	    new_out_size += sizeof(DeviceInfo);
	    dev_info++;
	  }/* for */
	  out_size = new_out_size;
	}
	else if( dev_info == NULL )
	{
	  /* Insufficient size.  The output parameter indicates the require size such that
	   *  the requesting thread can allocate and issue a successful call.
	   */
	  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: dev_info NULL : calculating\n"));
	  out_size = 0;
	  for( node = drv_ctx->device_list.Flink ; node != (PLIST_ENTRY)&drv_ctx->device_list ; node = node->Flink )
	  {
	    DeviceContext* dev_ctx = (DeviceContext*)node;
	    KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: node @ 0x%p , adding %d bytes\n",
		     node, sizeof(DeviceInfo)));
	    out_size += sizeof(DeviceInfo);  /* calculate size */
	  }/* for */
	  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: dev_info NULL : out_size %d\n",out_size));
	}
	else
	{
	  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: STATUS_INVALID_PARAMETER\n"));
	  status = STATUS_INVALID_PARAMETER;
	  out_size = 0;
	}
      }/* __try */
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
	KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: STATUS_ACCESS_VIOLATION\n"));
	out_size = 0;
	status = STATUS_ACCESS_VIOLATION;
      }
      ExReleaseFastMutex(&drv_ctx->mutex);

      /* Inform user-mode of output size required by the list regardless if it fits in
       * the user supplied space.  This can be use for user-mode to request required
       * size for actual list retreival.
       */
      KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_GET_ALL_DEVICES: out_size %d\n",out_size));
      WdfRequestCompleteWithInformation(Request,status,(ULONG_PTR)out_size);
      break;
    }/* IOCTL_NL_DEVENF_GET_ALL_DEVICES */

    case IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE:
    {
      /* Forward to the 'wait for arrival' queue.  Cancel is handled transparently.
         When a device arrives all the requests in that queue are completed.
      */
      if( KeResetEvent(&drv_ctx->device_arrival_event) )
      {
	WdfRequestCompleteWithInformation(Request,status,(ULONG_PTR)0);  /* complete request */
      }
      else
      {
	WdfRequestForwardToIoQueue(Request,drv_ctx->waitQueue);                   /* pend request to waitQueue */
      }
      break;
    }/* IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE */

    case IOCTL_NL_DEVENF_TEST_DEVICE_ARRIVE:
    {
      NTSTATUS status = STATUS_SUCCESS;
      int* result = (int*)out_buf;
      WDFVERIFY( out_buf != NULL );
      __try
      {
	/* The device arrival state is reset as a side-effect of this test.  The
	   meaning of the event is that there has been a device change since the
	   last test occurred.
	*/

	*result = 0;     /* default: non-arrival state */
	if( KeReadStateEvent(&drv_ctx->device_arrival_event) )  /* signaled state? */
	{
	  *result = 1;   /* arrival state */
	}
      }/* __try */
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
	KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_TEST_DEVICE_ARRIVE: STATUS_ACCESS_VIOLATION\n"));
	status = STATUS_ACCESS_VIOLATION;
      }
      WdfRequestCompleteWithInformation(Request,status,(ULONG_PTR)sizeof(*result));
      break;
    }/* IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE */

    case IOCTL_NL_DEVENF_SET_DEVICE_STATE:
    {
      DeviceInfo* dev_info = (DeviceInfo*)in_buf;

      KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE\n"));

      ExAcquireFastMutex(&drv_ctx->mutex);
      __try
      {
	PLIST_ENTRY node = NULL;
	for( node = drv_ctx->device_list.Flink ; node != (PLIST_ENTRY)&drv_ctx->device_list ; node = node->Flink )
	{
	  DeviceContext* dev_ctx = (DeviceContext*)node;
	  SIZE_T size;

	  KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: compare\n"));
	  /* Is this the device from we're looking for? Comparison of interface name
	   * is the criteria.
	   */
	  size = RtlCompareMemory(&dev_ctx->dev_info.InterfaceName,
				  dev_info->InterfaceName,
				  sizeof(dev_ctx->dev_info.InterfaceName));
	  if( size == sizeof(dev_ctx->dev_info.InterfaceName) )
	  {
	    int initial_state = dev_ctx->dev_info.state;

	    KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: copy\n"));
	    /* Assign new device state and update user-mode context. */
	    dev_ctx->dev_info.state = dev_info->state;
	    RtlCopyMemory(&dev_ctx->dev_info.context,&dev_info->context,
			  sizeof(dev_info->context));

	    KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: set time\n"));
	    KeQuerySystemTime(&dev_ctx->dev_info.sys_time_changed);

	    KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: fail device?\n"));
	    /* If the enabled state has been cleared disable the device only if it has not
	       already been disabled.
	    */
	    if( (initial_state & NL_DEVENF_DEVICE_STATE_ENABLED) != 0 &&
		(dev_ctx->dev_info.state & NL_DEVENF_DEVICE_STATE_ENABLED) == 0 )
	    {
	      WDFDEVICE device = WdfObjectContextGetObject(dev_ctx);
	      WDFVERIFY( device != NULL );
	      if( device != NULL )
	      {
		KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: fail device\n"));
		WdfDeviceSetFailed(device,WdfDeviceFailedNoRestart);
	      }
	    }
	    break;
	  }
	}/* for */
      }/* __try */
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
	KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: IOCTL_NL_DEVENF_SET_DEVICE_STATE: STATUS_ACCESS_VIOLATION\n"));
	status = STATUS_ACCESS_VIOLATION;
      }
      ExReleaseFastMutex(&drv_ctx->mutex);
      WdfRequestCompleteWithInformation(Request,STATUS_SUCCESS,(ULONG_PTR)0);
      break;
    }/* IOCTL_NL_DEVENF_SET_DEVICE_STATE */

    default:
    {
      KdPrint(("NLDevEnf: EvtIoDeviceControlControlDevice: INVALID_DEVICE_REQUEST\n"));
      WdfRequestCompleteWithInformation(Request,STATUS_INVALID_DEVICE_REQUEST,(ULONG_PTR)0);
      break;
    }/* default */
  }
}/* EvtIoDeviceControlControlDevice */

/** EvtIoDefaultControlDevice
 *
 *  \brief IO bound for the control device comes here.
 */
static VOID EvtIoDefaultControlDevice(__in WDFQUEUE Queue,
				       __in WDFREQUEST Request )
{
  NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
  WDF_REQUEST_PARAMETERS req_params;

  VERIFY_IS_IRQL_PASSIVE_LEVEL();

  WDF_REQUEST_PARAMETERS_INIT(&req_params);
  WdfRequestGetParameters(Request,&req_params);

  switch( req_params.Type )
  {
    case WdfRequestTypeCreate:
    case WdfRequestTypeClose:
      status = STATUS_SUCCESS;
      break;

    default:
      status = STATUS_INVALID_DEVICE_REQUEST;
      break;
  }/* switch */

  WdfRequestCompleteWithInformation(Request,status,(ULONG_PTR)0);
}/* EvtIoDefaultControlDevice */

/** EvtDeviceSelfManagedIoFlush
 *
 *  \brief Remove a device from the device list.  SelfManagedIoFlush
 *         is issued after the device has been removed from the system.
 */
static VOID EvtDeviceSelfManagedIoFlush(__in WDFDEVICE device )
{
  DriverContext* drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  DeviceContext* dev_ctx = WdfObjectGetTypedContext(device,DeviceContext);
  PLIST_ENTRY node = NULL;

  VERIFY_IS_IRQL_PASSIVE_LEVEL();
  WDFVERIFY( drv_ctx != NULL && dev_ctx != NULL );

  if( drv_ctx == NULL || dev_ctx == NULL )
  {
    KdPrint(("NLDevEnf: EvtDeviceSelfManagedIoFlush: contexts are NULL\n"));
    return;
  }

  /* If the device exists in the DriverContext::device_list it will be
   * present in the list with a non-NULL LIST_ENTRY::Flink.  Remove the
   * device if it is present in the list.
   */
  ExAcquireFastMutex(&drv_ctx->mutex);
  if( dev_ctx->list_entry.Flink != NULL )
  {
    RemoveEntryList((PLIST_ENTRY)dev_ctx);
  }
  RtlZeroMemory(dev_ctx,sizeof(DeviceContext));
  ExReleaseFastMutex(&drv_ctx->mutex);

  KdPrint(("NLDevEnf: EvtDeviceSelfManagedIoFlush: complete\n"));

}/* EvtDeviceSelfManagedIoFlush */

/** IsDeviceRemovable
 *
 *  \brief Determine if a device is removable.
 *
 *  \param Device (in)     WDF device.
 *  \param removable (out) Indicates if the given device is removable.
 *
 *  \return STATUS_SUCCESS when the device is removable, otherwise an error value
 *          which evalutes false with NT_SUCCESS() macro.
 */
static NTSTATUS IsDeviceRemovable(__in WDFDEVICE Device ,
				   __out BOOLEAN* removable )
{
  WDFREQUEST request = NULL;
  WDFIOTARGET target = NULL;
  WDF_REQUEST_SEND_OPTIONS options;
  WDF_REQUEST_REUSE_PARAMS reuse;
  IO_STACK_LOCATION stack ;
  NTSTATUS status = STATUS_ACCESS_DENIED;
  DEVICE_CAPABILITIES caps;
  DEVICE_REMOVAL_POLICY removal_policy = RemovalPolicyExpectNoRemoval;
  ULONG out_size = 0;

  VERIFY_IS_IRQL_PASSIVE_LEVEL();
  WDFVERIFY( removable != NULL );

  *removable = FALSE; /* default is not removable */

  /**************************************************************************
   * > Win2K - Support for DevicePropertyRemovalPolicy
   *************************************************************************/
  out_size = sizeof(removal_policy);
  status = WdfDeviceQueryProperty(Device,DevicePropertyRemovalPolicy,
				  sizeof(removal_policy),(PVOID)&removal_policy,&out_size);
  if( NT_SUCCESS(status) && removal_policy != RemovalPolicyExpectNoRemoval )
  {
    *removable = TRUE;
    status = STATUS_SUCCESS;
    goto IsDeviceRemovable_complete;
  }

  /***************************************************************************
   * Win2K - DevicePropertyRemovalPolicy not supported.  Issue IRP_MJ_PNP
   * for IPR_MN_QUERY_CAPABILITIES to determine if the device can be removed.
   *
   * Get the I/O target, create a request, and send the request to the I/O
   * target to retreive DEVICE_CAPABILITIES.
   **************************************************************************/
  target = WdfDeviceGetIoTarget(Device);
  if( target == NULL )
  {
    goto IsDeviceRemovable_complete;
  }

  status = WdfRequestCreate(WDF_NO_OBJECT_ATTRIBUTES,target,&request);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfRequestCreate failed (0x%x)\n",status));
    goto IsDeviceRemovable_complete;
  }

  WDF_REQUEST_REUSE_PARAMS_INIT(&reuse,WDF_REQUEST_REUSE_NO_FLAGS,STATUS_NOT_SUPPORTED);
  status = WdfRequestReuse(request,&reuse);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfRequestReuse failed (0x%x)\n",status));
    goto IsDeviceRemovable_complete;
  }

  RtlZeroMemory(&caps,sizeof(DEVICE_CAPABILITIES));
  caps.Size     = sizeof(DEVICE_CAPABILITIES);
  caps.Version  =  1;
  caps.Address  = (ULONG)-1;
  caps.UINumber = (ULONG)-1;

  RtlZeroMemory(&stack,sizeof(stack));
  stack.MajorFunction = IRP_MJ_PNP;
  stack.MinorFunction = IRP_MN_QUERY_CAPABILITIES;
  stack.Parameters.DeviceCapabilities.Capabilities = &caps;

  WdfRequestWdmFormatUsingStackLocation(request,&stack);
  WDF_REQUEST_SEND_OPTIONS_INIT(&options,WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);
  if( WdfRequestSend(request,target,&options) == TRUE )
  {
    status = WdfRequestGetStatus(request);
    if( NT_SUCCESS(status) && caps.Removable )
    {
      KdPrint(("NLDevEnf: IsDeviceRemovable: Removable %d SupriseRemovalOK %d EjectSupported %d\n",
	       caps.Removable, caps.SurpriseRemovalOK, caps.EjectSupported));
      *removable = TRUE;  /* device is removable per DEVICE_CAPABILITIES */
    }
  }/* send request */

 IsDeviceRemovable_complete:

  if( request != NULL )
  {
    WdfObjectDelete(request);
  }

  return status;
}/* IsDeviceRemovable */

/** WdfFltrEvtDeviceAdd
 *
 *  \brief Add a device to be filtered.  Track the device for evaluation
 *         in the device list.
 */
static NTSTATUS WdfFltrEvtDeviceAdd(__in WDFDRIVER driver ,
				     __in PWDFDEVICE_INIT DeviceInit )
{
  NTSTATUS status = STATUS_ACCESS_DENIED;
  WDF_OBJECT_ATTRIBUTES wdfObjectAttr;
  WDFDEVICE wdfDevice = NULL;                     /* device handle */
  DriverContext* drv_ctx = NULL;                  /* driver context */
  DeviceContext* dev_ctx = NULL;                  /* device context */
  WDFSTRING string;
  UNICODE_STRING ustring;
  WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
  ULONG out_size = 0;                             /* data out size for property query */
  BOOLEAN removable = FALSE;                      /* device is removable? */
  BOOLEAN completed_pending = FALSE;              /* completed a pending wait request? */

  VERIFY_IS_IRQL_PASSIVE_LEVEL();

  /* Inform framework of driver type */
  WdfFdoInitSetFilter(DeviceInit);
  WdfDeviceInitSetDeviceType(DeviceInit,FILE_DEVICE_UNKNOWN);

  /* Register SelfManagedIoFlush to be notified after the device has been removed. */
  WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
  pnpPowerCallbacks.EvtDeviceSelfManagedIoFlush = EvtDeviceSelfManagedIoFlush;
  WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit,&pnpPowerCallbacks);

  //WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&wdfObjectAttr,DeviceContext);
  WDF_OBJECT_ATTRIBUTES_INIT(&wdfObjectAttr);
  WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&wdfObjectAttr,DeviceContext);

  wdfObjectAttr.EvtCleanupCallback   = MyEvtCleanupCallback;
  wdfObjectAttr.ExecutionLevel       = WdfExecutionLevelPassive;
  wdfObjectAttr.SynchronizationScope = WdfSynchronizationScopeDevice;
  status = WdfDeviceCreate(&DeviceInit,&wdfObjectAttr,&wdfDevice);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: WdfDeviceCreate failed (0x%x)\n",status));
    goto WdfFltrEvtDevieAdd_complete;
  }

  KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: WdfObjectReference\n"));
  WdfObjectReference(wdfDevice);

  /* Create and initialize device context */
  dev_ctx = WdfObjectGetTypedContext(wdfDevice,DeviceContext);
  WDFVERIFY( dev_ctx != NULL );

  KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: dev_ctx @ 0x%x\n",dev_ctx));

  RtlZeroMemory(dev_ctx,sizeof(DeviceContext));
  dev_ctx->dev_info.state = NL_DEVENF_DEVICE_STATE_ENABLED;

  /***************************************************************************
   * Read device properties
   **************************************************************************/
  status = IsDeviceRemovable(wdfDevice,&removable);
  if( NT_SUCCESS(status) && removable == FALSE )
  {
    /* This is a fixed device.  No need to acquire its properties.  Ignore it. */
    status = STATUS_SUCCESS;
    goto WdfFltrEvtDevieAdd_complete;
  }

  /* The below retreives information for removable devices */

  status = WdfStringCreate(NULL,WDF_NO_OBJECT_ATTRIBUTES,&string);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: WdfStringCreate failed (0x%x)\n",status));
    goto WdfFltrEvtDevieAdd_complete;
  }

  status = WdfDeviceCreateDeviceInterface(wdfDevice,(LPGUID)&GUID_DEVINTERFACE_USB_DEVICE,NULL);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: WdeDeviceCreateDeviceInterface failed (0x%x)\n",status));
    goto WdfFltrEvtDevieAdd_complete;
  }

  status = WdfDeviceRetrieveDeviceInterfaceString(wdfDevice,&GUID_DEVINTERFACE_USB_DEVICE,
						  NULL,string);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: WdeDeviceRetrieveDeviceInterfaceString failed (0x%x)\n",status));
    goto WdfFltrEvtDevieAdd_complete;
  }

  /*****************************************************************************
   * If the device is not removalbe do not create a deivce for it.  Filter
   * flag has already been set, so IO will skip this driver.
   ****************************************************************************/
  /* Compatible IDs */
  out_size = sizeof(dev_ctx->dev_info.CompatibleIDs);
  status = WdfDeviceQueryProperty(wdfDevice,DevicePropertyCompatibleIDs,
				  sizeof(dev_ctx->dev_info.CompatibleIDs),
				  (PVOID)dev_ctx->dev_info.CompatibleIDs,&out_size);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("WdfFltrEvtDevieAdd: DevicePropertyCompatibleIDs failed\n"));
    goto WdfFltrEvtDevieAdd_complete;
  }

  out_size = sizeof(dev_ctx->dev_info.BusTypeGuid);
  status = WdfDeviceQueryProperty(wdfDevice,DevicePropertyBusTypeGuid,
				  sizeof(dev_ctx->dev_info.BusTypeGuid),
				  (PVOID)&dev_ctx->dev_info.BusTypeGuid,&out_size);
  if( !NT_SUCCESS(status) )
  {
    /* Win2K does not support BusTypeGuid.  Leave as unknown (zero out) for
     * user-mode.  The class name handles this case.
     */
    if( RtlIsNtDdiVersionAvailable(NTDDI_WIN2K) == TRUE )
    {
      RtlZeroMemory(&dev_ctx->dev_info.BusTypeGuid,sizeof(dev_ctx->dev_info.BusTypeGuid));
    }
    else
    {
      KdPrint(("WdfFltrEvtDevieAdd: DevicePropertyBusTypeGuid failed\n"));
      goto WdfFltrEvtDevieAdd_complete;
    }
  }/* !NT_STATUS(status) */

  /* Class Name */
  out_size = sizeof(dev_ctx->dev_info.ClassName);
  status = WdfDeviceQueryProperty(wdfDevice,DevicePropertyClassName,
				  sizeof(dev_ctx->dev_info.ClassName),
				  (PVOID)dev_ctx->dev_info.ClassName,
				  &out_size);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("WdfFltrEvtDevieAdd: DevicePropertyClassName failed\n"));
    goto WdfFltrEvtDevieAdd_complete;
  }

  /* Enumerator Name */
  out_size = sizeof(dev_ctx->dev_info.EnumeratorName);
  status = WdfDeviceQueryProperty(wdfDevice,DevicePropertyEnumeratorName,
				  sizeof(dev_ctx->dev_info.EnumeratorName),
				  (PVOID)dev_ctx->dev_info.EnumeratorName,
				  &out_size);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("WdfFltrEvtDevieAdd: DevicePropertyEnumeratorName failed\n"));
    goto WdfFltrEvtDevieAdd_complete;
  }

  /* Setup Class GUID */
  out_size = sizeof(dev_ctx->dev_info.SetupClass);
  status = WdfDeviceQueryProperty(wdfDevice,DevicePropertyClassGuid,
				  sizeof(dev_ctx->dev_info.SetupClass),
				  (PVOID)dev_ctx->dev_info.SetupClass,
				  &out_size);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("WdfFltrEvtDevieAdd: DevicePropertyClassGuid failed\n"));
    goto WdfFltrEvtDevieAdd_complete;
  }

  WdfStringGetUnicodeString(string,&ustring);
  RtlCopyMemory(dev_ctx->dev_info.InterfaceName,ustring.Buffer,ustring.Length);

  KdPrint(("NLDevEnf: DEVICE ARRIVAL (Removable)\n"));
  KdPrint(("NLDevEnf: Interface        = %ws\n",dev_ctx->dev_info.InterfaceName));
  KdPrint(("NLDevEnf: Setup Class GUID = %ws\n",dev_ctx->dev_info.SetupClass));
  KdPrint(("NLDevEnf: Class Name       = %ws\n",dev_ctx->dev_info.ClassName));
  KdPrint(("NLDevEnf: Enumerator Name  = %ws\n",dev_ctx->dev_info.EnumeratorName));

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  WDFVERIFY( drv_ctx != NULL );

  /* Device number is to uniquely identify a device on the system.  Also indicates
   * the arrival order.
   */
  dev_ctx->dev_info.device_number = InterlockedIncrement(&drv_ctx->device_number);
  KeQuerySystemTime(&dev_ctx->dev_info.sys_time_arrival);

  ExAcquireFastMutex(&drv_ctx->mutex);
  InsertTailList((PLIST_ENTRY)&drv_ctx->device_list,(PLIST_ENTRY)dev_ctx);

  ExReleaseFastMutex(&drv_ctx->mutex);

  /* Complete any pending requests in the 'wait for arrival' queue.  Completion
   * of a pending wait will not signal the arrival state.  Signaled state only
   * occurs if a device arrives and does *not* complete a pending wait condition.
   */
  for( ;; )
  {
    WDFREQUEST request = NULL;
    status = WdfIoQueueRetrieveNextRequest(drv_ctx->waitQueue,&request);      /* next pended request */
    if( !NT_SUCCESS(status) )
    {
      break;
    }
    WdfRequestCompleteWithInformation(request,status,(ULONG_PTR)0);  /* complete wait request */
    completed_pending = TRUE;
  }

  /* Only signal arrival if no pending wait conditions where completed. */
  if( completed_pending == FALSE )
  {
    KeSetEvent(&drv_ctx->device_arrival_event,0,FALSE);  /* signal device arrival has occurred */
  }

 WdfFltrEvtDevieAdd_complete:

  KdPrint(("NLDevEnf: WdfFltrEvtDeviceAdd: complete\n"));

  return STATUS_SUCCESS;
}/* WdfFltrEvtDeviceAdd */

/* Permissions for Control Device */
DECLARE_CONST_UNICODE_STRING(SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R,
			     L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGW;;;WD)(A;;GR;;;RC)");

NTSTATUS DriverEntry(__in PDRIVER_OBJECT DriverObject,
		      __in PUNICODE_STRING RegistryPath )
{
  WDF_DRIVER_CONFIG config;
  NTSTATUS status = STATUS_ACCESS_DENIED;
  UNICODE_STRING ustring;
  WDFDRIVER Driver = NULL;
  PWDFDEVICE_INIT device_init = NULL;
  WDF_OBJECT_ATTRIBUTES object_attribs;
  WDF_IO_QUEUE_CONFIG ioQueueConfig;
  WDFQUEUE hQueue = NULL;
  DriverContext* drv_ctx = NULL;  /* DriverContext */

  KdPrint(("NLDevEnf: DriverEntry: begin\n"));

  WDF_DRIVER_CONFIG_INIT(&config,WdfFltrEvtDeviceAdd);
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&object_attribs,DriverContext);
  status = WdfDriverCreate(DriverObject,RegistryPath,&object_attribs,&config,&Driver);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: WdfDriverCreate failed (0x%x)\n", status));
    goto DriverEntry_complete;
  }

  drv_ctx = WdfObjectGetTypedContext(Driver,DriverContext);
  WDFVERIFY( drv_ctx != NULL );
  RtlZeroMemory(drv_ctx,sizeof(DriverContext));
  KeInitializeEvent(&drv_ctx->device_arrival_event,NotificationEvent,FALSE);
  ExInitializeFastMutex(&drv_ctx->mutex);
  InitializeListHead(&drv_ctx->device_list);
  drv_ctx->device_number = 0L;

  /*********************************************************************
   * Create control device:
   *   \Device\NLDevEnf
   *   \DosDevices\NLDevEnf (symbolic link)
   ********************************************************************/
  device_init = WdfControlDeviceInitAllocate(Driver,&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
  if( !NT_SUCCESS(status) )
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    KdPrint(("NLDevEnf: DriverEntry: WdfControlDeviceInitAllocate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NL_DEVENF_DEVICE_NAME);
  status = WdfDeviceInitAssignName(device_init,&ustring);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: DriverEntry: RtlInitUnicodeString failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_OBJECT_ATTRIBUTES_INIT(&object_attribs);
  object_attribs.ExecutionLevel = WdfExecutionLevelPassive;
  status = WdfDeviceCreate(&device_init,&object_attribs,&drv_ctx->control_device);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: DriverEntry: WdfDeviceCreate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  /* Control device IO queue */
  WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,WdfIoQueueDispatchParallel);
  ioQueueConfig.EvtIoDefault       = EvtIoDefaultControlDevice;
  ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControlControlDevice;
  status = WdfIoQueueCreate(drv_ctx->control_device,&ioQueueConfig,WDF_NO_OBJECT_ATTRIBUTES,&hQueue);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: DriverEntry: WdfIoQueueCreate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  /***********************************************************************************
   * I/O request for IOCTL_NL_DEVENF_WAIT_DEVICE_ARRIVE are placed here.  Cancelation
   * is transparent.  When a drive arrives the queue is drained and all requests are
   * completing indicating a device has arrived.
   *
   * Dispatch must be manual for the EvtDeviceAdd() to manually drain the queue.
   **********************************************************************************/
  WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig,WdfIoQueueDispatchManual);
  status = WdfIoQueueCreate(drv_ctx->control_device,&ioQueueConfig,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->waitQueue);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: DriverEntry: WdeIoQueueCreate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NL_DEVENF_SYMBOLIC_LINK_NAME);
  status = WdfDeviceCreateSymbolicLink(drv_ctx->control_device,&ustring);
  if( !NT_SUCCESS(status) )
  {
    KdPrint(("NLDevEnf: DriverEntry: WdeDeviceCreateSymbolicLink failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WdfControlFinishInitializing(drv_ctx->control_device);

  status = STATUS_SUCCESS;

 DriverEntry_complete:

  KdPrint(("NLDevEnf: DriverEntry: complete (0x%x)\n",status));

  return status;
}/* DriverEntry */
