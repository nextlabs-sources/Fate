#include <fltKernel.h>
#include <wdf.h>
#include <wdfdriver.h>
#include <wdfrequest.h>

#include "nlcc.h"
#include "nlcc_klib.h"
#include "nlcc_ioctl.h"

#define NLCC_IS_DOUBLE_TERM(p) ( *((p) + 0) == (wchar_t)NULL && *((p) + 1) == (wchar_t)NULL )

__drv_maxIRQL(PASSIVE_LEVEL)
__checkReturn
NTSTATUS NLCC_KOpen( __in PNLCC_HANDLE in_handle )
{
  NTSTATUS status;
  UNICODE_STRING fileName;

  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
  ASSERT( in_handle != NULL );

  if( in_handle == NULL )
  {
    return STATUS_INVALID_PARAMETER;
  }

  RtlInitUnicodeString(&fileName,NLCC_QUERY_KDEVICE);
  status = IoGetDeviceObjectPointer(&fileName,GENERIC_ALL,&in_handle->fo_obj,&in_handle->dev_obj);
  return status;
}/* NLCC_Initialize */

__drv_maxIRQL(APC_LEVEL)
NTSTATUS NLCC_KClose( __in PNLCC_HANDLE in_handle )
{
  ASSERT( KeGetCurrentIrql() <= APC_LEVEL );
  ASSERT( in_handle != NULL );
  ASSERT( in_handle->dev_obj != NULL );

  if( in_handle == NULL )
  {
    return STATUS_INVALID_PARAMETER;
  }

  ObDereferenceObject(in_handle->fo_obj);

  return STATUS_SUCCESS;
}/* NLCC_Close */

/*
 * I/O completion routine for sending requests to NLCC driver.
 *
 * We use a completion routine when we send an I/O request to NLCC driver.
 * This is to stop I/O Manager from freeing our request when the NLCC driver
 * completes the request.  The routine does so by always returning
 * STATUS_MORE_PROCESSING_REQUIRED.  Instead, the request is always freed when
 * we explicitly call IoCompleteRequest() after potentially cancelling the
 * request.  This prevents a race condition and a crash when the following
 * sequence occurs:
 * 1. We call IoCallDriver() to send the request to NLCC driver.
 * 2. NLCC, hence IoCallDriver(), returns STATUS_PENDING.
 * 3. We call KeWaitForSingleObject(), which returns STATUS_TIMEOUT.
 * 4. NLCC completes the request.  Hence I/O Manager frees the request.
 * 5. We call IoCancelIrp() trying to cancel the request.
 * 6. BSOD.
 *
 * For more details, please see the discussion in:
 * Programming the Microsoft Windows Driver Model, 2nd Ed.
 * -> Chapter 5: The I/O Request Packet 
 *    -> Cancelling I/O Requests 
 *       -> Cancelling IRPs You Create or Handle
 *          -> Cancelling Your Own Synchronous IRP
 */
static IO_COMPLETION_ROUTINE KQuery_OnComplete;
static NTSTATUS KQuery_OnComplete( __in PDEVICE_OBJECT dev_obj ,
                                   __in PIRP irp ,
                                   __in PVOID event )
{
  if (irp->PendingReturned)
  {
    KeSetEvent((PKEVENT)event,IO_NO_INCREMENT,FALSE);
  }
  return STATUS_MORE_PROCESSING_REQUIRED;
}

__drv_maxIRQL(APC_LEVEL)
NTSTATUS NLCC_KQuery( __in PNLCC_HANDLE in_handle ,
		      __in PNLCC_QUERY in_request ,
		      __in PNLCC_QUERY out_response ,
		      __in_opt PLARGE_INTEGER in_timeout )
{
  NTSTATUS status;
  PIRP request = NULL;
  KEVENT complete_event;
  IO_STATUS_BLOCK iosb;

  ASSERT( KeGetCurrentIrql() <= APC_LEVEL );
  ASSERT( in_handle != NULL );
  ASSERT( in_request != NULL );
  ASSERT( out_response != NULL );

  if( in_handle == NULL || in_request == NULL || out_response == NULL )
  {
    return STATUS_INVALID_PARAMETER;
  }

  KeInitializeEvent(&complete_event,NotificationEvent,FALSE); /* unsignaled event */
  request = IoBuildDeviceIoControlRequest(IOCTL_POLICY_QUERY,in_handle->dev_obj,
					  in_request,sizeof(*in_request),      /* input */
					  out_response,sizeof(*out_response),  /* output */
					  FALSE,&complete_event,&iosb);
  if( request == NULL )
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto NLCC_Query_complete;
  }

  IoSetCompletionRoutine(request,KQuery_OnComplete,&complete_event,TRUE,TRUE,TRUE);

  /* call NLCC */
  status = IoCallDriver(in_handle->dev_obj,request);

  /* wait for the I/O to complete if I/O was pended */
  if( status == STATUS_PENDING )
  {
    status = KeWaitForSingleObject((PVOID)&complete_event,Executive,KernelMode,FALSE,in_timeout);
    if( status == STATUS_TIMEOUT )
    {
      IoCancelIrp(request);
      KeWaitForSingleObject(&complete_event,Executive,KernelMode,FALSE,NULL);
      IoCompleteRequest(request, IO_NO_INCREMENT);
      goto NLCC_Query_complete;
    }
  }

  IoCompleteRequest(request, IO_NO_INCREMENT);

  /* Status from the result of the I/O request */
  status = iosb.Status;

  NLCC_Query_complete:

  return status;
}/* NLCC_KQuery */

VOID NLCC_KInitializeQuery( __in PNLCC_QUERY in_query )
{
  ASSERT( in_query != NULL );

  in_query->size = sizeof(NLCC_QUERY);
  in_query->tx_id = 0;

  /* Must be double terminated */
  in_query->attributes[0] = (wchar_t)NULL;
  in_query->attributes[1] = (wchar_t)NULL;

}/* NLCC_KInitializeQuery */

NTSTATUS NLCC_KAddAttribute( __in PNLCC_QUERY in_query ,
			     __in const PUNICODE_STRING in_key ,
			     __in const PUNICODE_STRING in_value )
{
  NTSTATUS status = STATUS_SUCCESS;
  size_t curr_space_bytes = 0;
  size_t req_space_bytes = 0;
  wchar_t* p = in_query->attributes;  /* current position */

  ASSERT( in_query != NULL );
  ASSERT( in_key != NULL );
  ASSERT( in_value != NULL );

  if( in_query == NULL || in_key == NULL || in_value == NULL )
  {
    return STATUS_INVALID_PARAMETER;
  }

  /* Find last double term */
  for( ; ; )
  {
    if( NLCC_IS_DOUBLE_TERM(p) )
    {
      break;
    }
    p++;
  }

  /* If this is the first attribute is should be set a the start of the attributes
   * buffer, otherwise the double term has been detected and this is *not* the start
   * of that buffer.  Moving to the next location is required.
   */
  if( p != in_query->attributes )
  {
    p++;
  }

  curr_space_bytes = sizeof(in_query->attributes) - (p - in_query->attributes) * sizeof(wchar_t);

  /* Requires space for key, term, value, and double termination.  Going to pack
   * the following: [key][NULL][value][NULL][NULL]
   */
  req_space_bytes =
    in_key->Length + sizeof(wchar_t) +            /* key + term */
    in_value->Length + (2 * sizeof(wchar_t));     /* value + double term */

  /* Is there sufficient space to pack the attribute? */
  if( req_space_bytes >= curr_space_bytes )
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto NLCC_KAddAttribute_complete;
  }

  /* Pack the key and advance the current pointer past the key location including
   * the term separater.
   */
  RtlCopyMemory(p,in_key->Buffer,in_key->Length);
  p += (in_key->Length / sizeof(wchar_t));
  *p = (wchar_t)NULL;
  p++;

  RtlCopyMemory(p,in_value->Buffer,in_value->Length);
  p += (in_value->Length / sizeof(wchar_t));
  *(p + 0) = (wchar_t)NULL;
  *(p + 1) = (wchar_t)NULL;

 NLCC_KAddAttribute_complete:

  return status;
}/* NLCC_KAddAttribute */
