#include <fltKernel.h>
#include <wdf.h>
#include <wdfdriver.h>
#include <wdfrequest.h>

#include "nlcc.h"
#include "nlcc_types.h"
#include "nlcc_ioctl.h"

#define TRACE KdPrint

#define NLCC_PDP_RESPONSE_TAG   '0cln'  /* PDP response */

WDF_DECLARE_CONTEXT_TYPE(DriverContext);
WDF_DECLARE_CONTEXT_TYPE(RequestContext);

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_IO_QUEUE_IO_DEFAULT EvtIoPDPControlDevice;
EVT_WDF_IO_QUEUE_IO_DEFAULT EvtIoDefaultPEPControlDevice;

EVT_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreatePDP;
EVT_WDF_FILE_CLOSE EvtFileClosePDP;

EVT_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreatePEP;
EVT_WDF_FILE_CLOSE EvtFileClosePEP;

/** ForwardRequestToPDP
 *
 *  \brief Forward the policy request to the PDP.  This function will copy the request
 *         to the PDP request.
 *
 *         (1) Retreive the input/output buffers for the PEP/PDP request.
 *         (2) Forward the PEP request to the service wait IO queue.
 *         (3) Complete the PDP request.
 *
 *         If any of the above three operations fail, then both the PEP and PDP requests
 *         are completed with STATUS_INSUFFICIENT_RESOURCES.
 */
NTSTATUS ForwardRequestToPDP( __in WDFREQUEST pep_req , __in WDFREQUEST pdp_req )
{
  NTSTATUS status;
  DriverContext* drv_ctx = NULL;
  RequestContext* req_ctx = NULL;
  WDF_OBJECT_ATTRIBUTES attributes;
  PNLCC_QUERY ipc_req_from_pep = NULL;
  PNLCC_QUERY ipc_req_from_pdp = NULL;

  ASSERT( pep_req != NULL );
  ASSERT( pdp_req != NULL );

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  ASSERT( drv_ctx != NULL );

  /* Create a request context */
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes,RequestContext);
  status = WdfObjectAllocateContext(pep_req,&attributes,&req_ctx);
  if( status != STATUS_SUCCESS )
  {
    goto ForwardRequestToPDP_complete;
  }

  TRACE(("Complete PDP wait (pend req %x)\n",pdp_req));
  RtlZeroMemory(req_ctx,sizeof(RequestContext));

  /* Get input buffer from PEP request */
  status = WdfRequestRetrieveInputBuffer(pep_req,sizeof(NLCC_QUERY),&ipc_req_from_pep,NULL);
  if( status != STATUS_SUCCESS )
  {
    goto ForwardRequestToPDP_complete;
  }

  /* Get output buffer from PPD request */
  status = WdfRequestRetrieveOutputBuffer(pdp_req,sizeof(NLCC_QUERY),&ipc_req_from_pdp,NULL);
  if( status != STATUS_SUCCESS )
  {
    goto ForwardRequestToPDP_complete;
  }

  /* Increment the transaction ID.  Assign it to the PDP and PEP requests */
  ipc_req_from_pep->tx_id = InterlockedIncrement64(&drv_ctx->tx_id);
  req_ctx->tx_id = ipc_req_from_pep->tx_id;

  RtlCopyMemory(ipc_req_from_pdp,ipc_req_from_pep,sizeof(NLCC_QUERY));

  WdfSpinLockAcquire(drv_ctx->qServiceWaitlock);
  status = WdfRequestForwardToIoQueue(pep_req,drv_ctx->ioq_service_wait);
  WdfSpinLockRelease(drv_ctx->qServiceWaitlock);

  if( status != STATUS_SUCCESS )
  {
    KdPrint(("ForwardRequestToPDP: WdfRequestForwardToIoQueue failed (0x%x)\n",status));
    goto ForwardRequestToPDP_complete;
  }

  WdfRequestCompleteWithInformation(pdp_req,STATUS_SUCCESS,(ULONG_PTR)sizeof(NLCC_QUERY));

 ForwardRequestToPDP_complete:

  /* A critical error has occurred - no input/output buffer, cannot forward request, etc. - so both
   * PEP and PDP requests are failed.
   *
   * If no error has occurred, then both requests have been handled.  The PDP request would be
   * completed and the PEP request forwarded to the service wait queue.
   */
  if( status != STATUS_SUCCESS )
  {
    KdPrint(("ForwardRequestToPDP: failed (0x%x)\n",status));
    WdfRequestComplete(pep_req,STATUS_INSUFFICIENT_RESOURCES);
    WdfRequestComplete(pdp_req,STATUS_INSUFFICIENT_RESOURCES);
  }

  return status;
}/* ForwardRequestToPDP */

void HandlePEPRequest( __in WDFREQUEST Request )
{
  NTSTATUS status;
  PNLCC_QUERY preq = NULL;
  WDF_REQUEST_PARAMETERS Params;
  DriverContext* drv_ctx = NULL;
  WDFREQUEST req = NULL;

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  ASSERT( drv_ctx != NULL );

  WDF_REQUEST_PARAMETERS_INIT(&Params);
  WdfRequestGetParameters(Request,&Params);

  /* PEP is requesting service from the PDP */
  ASSERT( Params.Parameters.DeviceIoControl.IoControlCode == IOCTL_POLICY_QUERY );

  TRACE(("PDP: IOCTL_POLICY_QUERY\n"));

  /* Is there a PEP request available?  If not, then pend to the pep queue. */
  WdfSpinLockAcquire(drv_ctx->qlock);
  status = WdfIoQueueRetrieveNextRequest(drv_ctx->ioq_pdp_wait,&req);
  if( NT_SUCCESS(status) )
  {
    WdfSpinLockRelease(drv_ctx->qlock);
    status = ForwardRequestToPDP(Request,req);
  }
  else if( status == STATUS_NO_MORE_ENTRIES )
  {
    /* If there are no entries, so the PEP cannot be servied.  Place the request
     * in the PEP wait for service queue.
     */
    TRACE(("There are no waiting PDP reuqest.  Pend req to pep_wait\n"));
    status = WdfRequestForwardToIoQueue(Request,drv_ctx->ioq_pep_wait);
    WdfSpinLockRelease(drv_ctx->qlock);
    if( status != STATUS_SUCCESS )
    {
      WdfRequestComplete(Request,STATUS_INSUFFICIENT_RESOURCES);
    }
  }
  else
  {
    WdfSpinLockRelease(drv_ctx->qlock);
    ASSERT(0);
  }
}/* HandlePEPRequest */

void HandlePDPResponse( __in WDFREQUEST Request )
{
  NTSTATUS status;
  WDFREQUEST curr_req = NULL;
  WDFREQUEST next_req = NULL;
  DriverContext* drv_ctx = NULL;
  PNLCC_QUERY pdp_response = NULL;  /* pointer to PDP response buffer */
  PNLCC_QUERY pdp_response_copy = NULL;

  pdp_response_copy = (PNLCC_QUERY)ExAllocatePoolWithTag(NonPagedPool,sizeof(NLCC_QUERY),NLCC_PDP_RESPONSE_TAG);
  if( pdp_response_copy == NULL )
  {
    TRACE(("HandlePDPResponse: ExAllocatePoolWithTag failed\n"));
    WdfRequestComplete(Request,STATUS_INSUFFICIENT_RESOURCES);
    return;
  }

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  status = WdfRequestRetrieveInputBuffer(Request,sizeof(NLCC_QUERY),&pdp_response,NULL);
  if( status != STATUS_SUCCESS )
  {
    TRACE(("HandlePDPResponse: WdfRequestRetrieveInputBuffer failed (0x%x)\n",status));
    WdfRequestComplete(Request,STATUS_INVALID_PARAMETER);
    ExFreePoolWithTag(pdp_response_copy,NLCC_PDP_RESPONSE_TAG);
    return;
  }
  ASSERT( pdp_response != NULL );

  /* Copy out the response and complete the request.  There is no benefit to holding on to the
   * reqsponse request form the PDP.  Complete request to the PDP.
   */
  RtlCopyMemory(pdp_response_copy,pdp_response,sizeof(*pdp_response_copy));
  WdfRequestComplete(Request,STATUS_SUCCESS);  

  for( ; ; )
  {
    WDFREQUEST req = NULL;
    RequestContext* req_ctx = NULL;

	WdfSpinLockAcquire(drv_ctx->qServiceWaitlock);
    status = WdfIoQueueFindRequest(drv_ctx->ioq_service_wait,curr_req,NULL,NULL,&next_req);
	WdfSpinLockRelease(drv_ctx->qServiceWaitlock);

	if(NULL != curr_req)
	{
	  // Free the request got from previous round
	  WdfObjectDereference(curr_req);
	  curr_req = NULL;
	}
    if( status == STATUS_NO_MORE_ENTRIES )
    {
      break;
    }
    if( status == STATUS_NOT_FOUND )
    {
      curr_req = next_req = NULL;
      continue;
    }
    if( !NT_SUCCESS(status) )
    {
      ASSERT(0);
      status = STATUS_UNSUCCESSFUL;
      break;
    }

    ASSERT( next_req != NULL );
    req_ctx = WdfObjectGetTypedContext(next_req,RequestContext);
    ASSERT( req_ctx != NULL );

    /* Is this the matching request?  If not, then keep iterating. */
    if( req_ctx->tx_id != pdp_response_copy->tx_id )
    {
	  // Use the request we just got as the input parameter in next round
	  curr_req = next_req;
	  next_req = NULL;
      continue;
    }

	WdfSpinLockAcquire(drv_ctx->qServiceWaitlock);
    status = WdfIoQueueRetrieveFoundRequest(drv_ctx->ioq_service_wait,next_req,&req);
	WdfSpinLockRelease(drv_ctx->qServiceWaitlock);

    WdfObjectDereference(next_req);
	next_req = NULL;

    if( status == STATUS_SUCCESS )
    {
      PNLCC_QUERY pep_response = NULL;  /* pointer to PEP response buffer */

      ASSERT( req != NULL );
      if( WdfRequestRetrieveOutputBuffer(req,sizeof(NLCC_QUERY),&pep_response,NULL) == STATUS_SUCCESS )
      {
	ASSERT( pep_response != NULL );
	RtlCopyMemory(pep_response,pdp_response_copy,sizeof(NLCC_QUERY));
      }

      WdfRequestCompleteWithInformation(req,STATUS_SUCCESS,sizeof(NLCC_QUERY));  /* Complete request to PEP */
      break;
    }
  }/* for */
  ExFreePoolWithTag(pdp_response_copy,NLCC_PDP_RESPONSE_TAG);
}/* HandlePDPResponse */

VOID HandlePDPRequest( __in WDFREQUEST Request )
{
  NTSTATUS status;
  DriverContext* drv_ctx = NULL;
  WDFREQUEST req = NULL;

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  ASSERT( drv_ctx != NULL );
  TRACE(("PDP: IOCTL_POLICY_REQUEST\n"));

  /* Is there a PEP request available?  If not, then pend to the pep queue. */
  WdfSpinLockAcquire(drv_ctx->qlock);
  status = WdfIoQueueRetrieveNextRequest(drv_ctx->ioq_pep_wait,&req);
  if( status == STATUS_SUCCESS )
  {
    WdfSpinLockRelease(drv_ctx->qlock);
    status = ForwardRequestToPDP(req,Request);
  }
  else if( status == STATUS_NO_MORE_ENTRIES )
  {
    /* If there are no entries, so the PEP cannot be servied.  Place the request
     * in the PEP wait for service queue.
     */
    TRACE(("forward to pdp_wait\n"));
    status = WdfRequestForwardToIoQueue(Request,drv_ctx->ioq_pdp_wait);
    WdfSpinLockRelease(drv_ctx->qlock);
    if( status != STATUS_SUCCESS )
    {
      WdfRequestComplete(Request,STATUS_INSUFFICIENT_RESOURCES);
    }
  }
  else
  {
    WdfSpinLockRelease(drv_ctx->qlock);
    ASSERT(0);
  }

}/* HandlePDPRequest */

/** EvtIoDefaultControlDevice
 *
 *  \brief IO bound for the control device comes here.
 */
static VOID EvtIoPDPControlDevice( WDFQUEUE Queue , WDFREQUEST Request )
{
  DriverContext* drv_ctx = NULL;
  WDF_REQUEST_PARAMETERS Params;

  WDF_REQUEST_PARAMETERS_INIT(&Params);
  WdfRequestGetParameters(Request,&Params);

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  ASSERT( drv_ctx != NULL );

  switch( Params.Parameters.DeviceIoControl.IoControlCode )
  {
    /* The PDP is requesting a service from a PEP */
    case IOCTL_POLICY_REQUEST:
      HandlePDPRequest(Request);
      break;

      /* There is a policy response */
    case IOCTL_POLICY_RESPONSE:
      HandlePDPResponse(Request);
      break;

      /* PEP is requesting service from the PDP */
    case IOCTL_POLICY_QUERY:
      HandlePEPRequest(Request);
      break;

    default:
      WdfRequestComplete(Request,STATUS_INSUFFICIENT_RESOURCES);
      break;

  }/* switch */
  /* The request must have been completed by this point */
}/* EvtIoPDPControlDevice */

static VOID EvtIoDefaultPEPControlDevice( WDFQUEUE Queue , WDFREQUEST Request )
{
  NTSTATUS status;
  WDF_REQUEST_SEND_OPTIONS req_options;
  BOOLEAN rv;
  DriverContext* drv_ctx = NULL;
  WDFDEVICE dev = WdfIoQueueGetDevice(Queue);
  LONG count;

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  count = InterlockedCompareExchange(&drv_ctx->pdp_count,0,0);
  if( count <= 1 )
  {
    KdPrint(("PDP is down - complete request\n"));
    WdfRequestComplete(Request,STATUS_INSUFFICIENT_RESOURCES);
    return;
  }

  /* The IO target must always been open since it is managed by create/close of
   * the PEP device.
   */
  ASSERT( drv_ctx->iot != NULL );

  WDF_REQUEST_SEND_OPTIONS_INIT(&req_options,WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);
  WdfRequestFormatRequestUsingCurrentType(Request);
  TRACE(("PEP: Sending request (Request %x)\n",Request));

  WdfWaitLockAcquire(drv_ctx->iot_lock,NULL);
  rv = WdfRequestSend(Request,drv_ctx->iot,&req_options);
  WdfWaitLockRelease(drv_ctx->iot_lock);

  if( rv != TRUE )
  {
    TRACE(("PEP: OnPolicyRequest failed\n"));
    status = STATUS_UNSUCCESSFUL;
  }
}/* EvtIoDefaultPEPControlDevice */

EVT_WDF_DRIVER_UNLOAD EvtDriverUnload;

static VOID EvtDriverUnload( WDFDRIVER Driver )
{
  TRACE(("NLCC unload driver\n"));
}/* EvtDriverUnload */

VOID EvtDeviceFileCreatePDP( __in WDFDEVICE Device, __in WDFREQUEST Request, __in WDFFILEOBJECT FileObject )
{
  DriverContext* drv_ctx = NULL;

  KdPrint(("Open PDP device\n"));

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  KdPrint(("Open PDP device ++\n"));
  InterlockedIncrement(&drv_ctx->pdp_count);

  KdPrint(("Open PDP device done\n"));

  WdfRequestComplete(Request,STATUS_SUCCESS);
}/* EvtDeviceFileCreate */

/** EvtFileClose
 *
 *  \brief Disable pend of requests to the PDP device.  Clear the pending queues.
 */
VOID EvtFileClosePDP( __in  WDFFILEOBJECT FileObject )
{
  DriverContext* drv_ctx = NULL;

  KdPrint(("Close PDP device\n"));

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  KdPrint(("Close PDP device --\n"));
  InterlockedDecrement(&drv_ctx->pdp_count);

  KdPrint(("Close PDP device done\n"));

  WdfSpinLockAcquire(drv_ctx->qlock);
  WdfIoQueuePurgeSynchronously(drv_ctx->ioq_pep_wait);
  WdfIoQueueStart(drv_ctx->ioq_pep_wait);
  WdfSpinLockRelease(drv_ctx->qlock);

  WdfSpinLockAcquire(drv_ctx->qServiceWaitlock);
  WdfIoQueuePurgeSynchronously(drv_ctx->ioq_service_wait);
  WdfIoQueueStart(drv_ctx->ioq_service_wait);
  WdfSpinLockRelease(drv_ctx->qServiceWaitlock);
}/* EvtFileClose */

VOID EvtDeviceFileCreatePEP( __in WDFDEVICE Device, __in WDFREQUEST Request, __in WDFFILEOBJECT FileObject )
{
  NTSTATUS status;
  DriverContext* drv_ctx = NULL;
  UNICODE_STRING fileName;
  WDF_IO_TARGET_OPEN_PARAMS iot_params;

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  WdfWaitLockAcquire(drv_ctx->iot_lock,NULL);
  /* IO target is not yet opened */
  if( InterlockedCompareExchange(&drv_ctx->pep_count,0,0) > 0 )
  {
    status = STATUS_SUCCESS;
    goto EvtDeviceFileCreatePEP_complete;
  }

  KdPrint(("Creating IO target\n"));
  status = WdfIoTargetCreate(Device,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->iot);
  if( status != STATUS_SUCCESS )
  {
    KdPrint(("WdfIoTargetCreate failed (0x%x)\n",status));
    drv_ctx->iot = NULL;
    goto EvtDeviceFileCreatePEP_complete;
  }

  RtlInitUnicodeString(&fileName,NLCC_PDP_KDEVICE);
  WDF_IO_TARGET_OPEN_PARAMS_INIT_CREATE_BY_NAME(&iot_params,&fileName,STANDARD_RIGHTS_ALL);
  status = WdfIoTargetOpen(drv_ctx->iot,&iot_params);
  if( status != STATUS_SUCCESS )
  {
    KdPrint(("WdfIoTargetOpen failed (0x%x)\n",status));
    WdfObjectDelete(drv_ctx->iot);
    drv_ctx->iot = NULL;
    goto EvtDeviceFileCreatePEP_complete;
  }

 EvtDeviceFileCreatePEP_complete:
	if (status == STATUS_SUCCESS)
	{
		InterlockedIncrement(&drv_ctx->pep_count);
	}

  WdfWaitLockRelease(drv_ctx->iot_lock);
  WdfRequestComplete(Request,status);

}/* EvtDeviceFileCreate */

/** EvtFileClose
 *
 *  \brief Disable pend of requests to the PDP device.  Clear the pending queues.
 */
VOID EvtFileClosePEP( __in  WDFFILEOBJECT FileObject )
{
  DriverContext* drv_ctx = NULL;

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);

  WdfWaitLockAcquire(drv_ctx->iot_lock,NULL);
  if( InterlockedDecrement(&drv_ctx->pep_count) == 0 )
  {
    KdPrint(("Close IO target\n"));
    WdfObjectDelete(drv_ctx->iot);
  }
  WdfWaitLockRelease(drv_ctx->iot_lock);

}/* EvtFileClose */

NTSTATUS
DriverEntry( IN PDRIVER_OBJECT DriverObject , IN PUNICODE_STRING RegistryPath )
{
  NTSTATUS status;
  WDFDEVICE control_device = NULL;
  WDFDRIVER drv = NULL;
  UNICODE_STRING ustring;
  WDF_DRIVER_CONFIG cfg;
  WDF_FILEOBJECT_CONFIG fo_cfg;
  WDF_OBJECT_ATTRIBUTES object_attribs;
  PWDFDEVICE_INIT device_init = NULL;
  WDF_IO_QUEUE_CONFIG qcfg;
  PDEVICE_OBJECT dev = NULL;
  DriverContext* drv_ctx = NULL;

  TRACE(("NLCC: DriverEntry\n"));

  /*********************************************************************************************************
   * Create a non-PnP driver
   ********************************************************************************************************/
  WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&object_attribs,DriverContext);

  WDF_DRIVER_CONFIG_INIT(&cfg,NULL);
  cfg.DriverInitFlags = WdfDriverInitNonPnpDriver;
  cfg.DriverPoolTag   = (ULONG)'PEPU';
  cfg.EvtDriverUnload = EvtDriverUnload;
  status = WdfDriverCreate(DriverObject,RegistryPath,&object_attribs,&cfg,&drv);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry - WdfDriverCreate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  drv_ctx = WdfObjectGetTypedContext(WdfGetDriver(),DriverContext);
  RtlZeroMemory(drv_ctx,sizeof(DriverContext));
  drv_ctx->pdp_count = 0;
  drv_ctx->pep_count = 0;

  status = WdfWaitLockCreate(WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->iot_lock);
  if( status != STATUS_SUCCESS )
  {
    TRACE(("NLCC: DriverEntry - WdfWaitLockCreate failed (0x%x)\n",status));
    return status;
  }

  status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->qlock);
  if( status != STATUS_SUCCESS )
  {
    TRACE(("NLCC: DriverEntry - WdfSpinLockCreate failed (0x%x)\n",status));
    return status;
  }

  status = WdfSpinLockCreate(WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->qServiceWaitlock);
  if( status != STATUS_SUCCESS )
  {
	  TRACE(("NLCC: DriverEntry - WdfSpinLockCreate qServiceWaitlock failed (0x%x)\n",status));
	  return status;
  }

  /*********************************************************************************************************
   * Create PDP device
   ********************************************************************************************************/
  device_init = WdfControlDeviceInitAllocate(drv,&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
  if( device_init == NULL )
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NLCC_PDP_KDEVICE);
  status = WdfDeviceInitAssignName(device_init,&ustring);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: RtlInitUnicodeString failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_FILEOBJECT_CONFIG_INIT(&fo_cfg,EvtDeviceFileCreatePDP,EvtFileClosePDP,NULL);
  WdfDeviceInitSetFileObjectConfig(device_init,&fo_cfg,WDF_NO_OBJECT_ATTRIBUTES);

  WDF_OBJECT_ATTRIBUTES_INIT(&object_attribs);
  status = WdfDeviceCreate(&device_init,&object_attribs,&control_device);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("DriverEntry: WdfDeviceCreate failed for PDP device (0x%x)\n",status));
    goto DriverEntry_complete; 
  }

  dev = WdfDeviceWdmGetDeviceObject(control_device);
  dev->StackSize = 4;

  /**************************************************************************************
   * Create three queues - default, pdp, pep
   *
   * pdp and pep queues are manual dispatch.  All queues are not power managed.
   *************************************************************************************/
  WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&qcfg,WdfIoQueueDispatchParallel);
  qcfg.PowerManaged = WdfFalse;
  qcfg.EvtIoDefault = EvtIoPDPControlDevice;
  status = WdfIoQueueCreate(control_device,&qcfg,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->ioq_pdp_default);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfIoQueueCreate failed - ioq_pdp_default (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_IO_QUEUE_CONFIG_INIT(&qcfg,WdfIoQueueDispatchManual);
  qcfg.PowerManaged = WdfFalse;
  status = WdfIoQueueCreate(control_device,&qcfg,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->ioq_pdp_wait);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfIoQueueCreate failed - ioq_pdp_wait (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_IO_QUEUE_CONFIG_INIT(&qcfg,WdfIoQueueDispatchManual);
  qcfg.PowerManaged = WdfFalse;
  status = WdfIoQueueCreate(control_device,&qcfg,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->ioq_pep_wait);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfIoQueueCreate failed - ioq_pep_wait (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_IO_QUEUE_CONFIG_INIT(&qcfg,WdfIoQueueDispatchManual);
  qcfg.PowerManaged = WdfFalse;
  status = WdfIoQueueCreate(control_device,&qcfg,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->ioq_service_wait);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfIoQueueCreate failed - ioq_service_wait (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NLCC_PDP_KDEVICE_SYMBOLIC_LINK_NAME);
  status = WdfDeviceCreateSymbolicLink(control_device,&ustring);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdeDeviceCreateSymbolicLink failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }
  WdfControlFinishInitializing(control_device);

  /*********************************************************************************************************
   * Create PEP device
   ********************************************************************************************************/
  device_init = WdfControlDeviceInitAllocate(drv,&SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
  if( device_init == NULL )
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NLCC_QUERY_KDEVICE);
  status = WdfDeviceInitAssignName(device_init,&ustring);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: RtlInitUnicodeString failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  WDF_FILEOBJECT_CONFIG_INIT(&fo_cfg,EvtDeviceFileCreatePEP,EvtFileClosePEP,NULL);
  WdfDeviceInitSetFileObjectConfig(device_init,&fo_cfg,WDF_NO_OBJECT_ATTRIBUTES);

  WDF_OBJECT_ATTRIBUTES_INIT(&object_attribs);
  status = WdfDeviceCreate(&device_init,&object_attribs,&control_device);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfDeviceCreate failed for PEP device (0x%x)\n",status));
    goto DriverEntry_complete; 
  }
  
  /* A control device will have a stack size of 1 by default.  This is not sufficient
   * to foward I/O to another driver which the PEP device must be able to do.  It must
   * be able to forward an request to the PDP device which requries an additional
   * stack location.
   */
  dev = WdfDeviceWdmGetDeviceObject(control_device);
  dev->StackSize = 4;

  /* Control device IO queue */
  WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&qcfg,WdfIoQueueDispatchParallel);
  qcfg.PowerManaged = WdfFalse;
  qcfg.EvtIoDefault = EvtIoDefaultPEPControlDevice;
  status = WdfIoQueueCreate(control_device,&qcfg,WDF_NO_OBJECT_ATTRIBUTES,&drv_ctx->ioq_pep_default);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdfIoQueueCreate failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }

  RtlInitUnicodeString(&ustring,NLCC_QUERY_KSYMBOLIC_LINK_NAME);
  status = WdfDeviceCreateSymbolicLink(control_device,&ustring);
  if( !NT_SUCCESS(status) )
  {
    TRACE(("NLCC: DriverEntry: WdeDeviceCreateSymbolicLink failed (0x%x)\n",status));
    goto DriverEntry_complete;
  }
  WdfControlFinishInitializing(control_device);

  TRACE(("NLCC: DriverEntry - control devices constructed\n"));

  status = STATUS_SUCCESS;

 DriverEntry_complete:

  return status;
}/* DriverEntry */
