/*++

Module Name:

    NLSEComm.c

Abstract:
  Process Communication between user mode and kernel

Environment:
  Kernel mode

--*/
#include "NLSEStruct.h"
#include "NLSEUtility.h"
#include "NLSERawAccess.h"
#include "NLSEDrmPathList.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;
extern NL_KLOG nlseKLog;
extern ULONG nlseLoggingFlags;

//key management
//If the nlfseGlobal.currentPCKey is expired, send a request to get
//the current PC key. 
NTSTATUS
NLSEUpdateCurrentPCKey(
                       __in ULONG pid,
                       __in BOOLEAN UseCache
                       )
{
    LARGE_INTEGER                 currentTime;
    ULONG                         currentSecond;
    PNLSE_KERNEL_REQUEST          req=NULL;
    PNLSE_KERNEL_REQUEST_RESPONSE response=NULL;
    ULONG                         responseLen;
    NTSTATUS                      status = STATUS_SUCCESS;
    LARGE_INTEGER                 timeout;	
    PNLSE_PORT_CONTEXT            portCtx = NULL;
    STRING                        tmpString;

#ifdef NLSE_DEBUG_FAKE_PC_KEY
    RtlFillMemory(nlfseGlobal.currentPCKey,NLSE_KEY_LENGTH_IN_BYTES,'a');
    nlfseGlobal.hasCurrentPCKey = TRUE;
    return STATUS_SUCCESS;
#endif

    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

    // get current time
    currentTime = GetCurrentTime();
    RtlTimeToSecondsSince1980(&currentTime,&currentSecond);

    // Use cache: Check if the cached PC key expired
    if(UseCache && (currentSecond <= nlfseGlobal.keyRefreshTime))
    {
        status = nlfseGlobal.hasCurrentPCKey?STATUS_SUCCESS:STATUS_GENERIC_COMMAND_FAILED;
        goto _exit;
    }

    // Otherwise, try to get PC key from Policy Controller
    req=ExAllocatePoolWithTag(PagedPool, sizeof(NLSE_KERNEL_REQUEST), NLSE_MESSAGE_TAG);
    if(req==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!UpdateCurrentPCKey: failed to allocate request\n"));
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    responseLen=sizeof(FILTER_REPLY_HEADER)+sizeof(NLSE_MESSAGE);
    response=ExAllocatePoolWithTag(PagedPool, responseLen, NLSE_MESSAGE_TAG);
    if(response==NULL)
    {
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!UpdateCurrentPCKey: failed to allocate response\n"));
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto _exit;
    }
    RtlZeroMemory(response,responseLen);
    RtlZeroMemory(req,sizeof(NLSE_KERNEL_REQUEST));

    portCtx=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
    if(portCtx->port == NULL)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, ("NLSE!UpdateCurrentPCKey: main port not connected\n"));
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_PORT_DISCONNECTED;
        goto _exit;
    }

    //compose request
    req->type=NLSE_KERNEL_REQ_GET_KEY;
    RtlInitString(&tmpString, NLSE_KEY_RING_LOCAL);
    RtlCopyMemory(req->msg.keyRingName, tmpString.Buffer, tmpString.Length);
    req->msg.pid=pid;
    responseLen=sizeof(NLSE_MESSAGE);
    timeout.QuadPart=NLSE_DELAY_ONE_SECOND*5;

    //send request and get response
    status = FltSendMessage(nlfseGlobal.filterHandle,
        &((PFLT_PORT)portCtx->port),
        &req->type,
        sizeof(int)+sizeof(NLSE_MESSAGE),
        &response->msg,
        &responseLen,
        &timeout);
    if(status != STATUS_SUCCESS)
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey: FltSendMessage failed 0x%x\n", status);
        if(status == STATUS_TIMEOUT)
        {
            //Use a longer refresh time if the error is a timeout, because timeout
            //may be caused by the PC side being too busy.
            nlfseGlobal.keyRefreshTime = currentSecond + 20;
        }
        else
        {
            nlfseGlobal.keyRefreshTime = currentSecond + 1;
        }
        goto _exit;
    }

    if(responseLen == sizeof(NLSE_MESSAGE))
    {
        if(response->msg.result == NLSE_KERNEL_REQ_RESULT_SUCCESS)
        {
            RtlCopyMemory(&(nlfseGlobal.currentPCKeyID), &(response->msg.keyID), sizeof(NLSE_KEY_ID));
            RtlCopyMemory(nlfseGlobal.currentPCKey, response->msg.key, NLSE_KEY_LENGTH_IN_BYTES);
            nlfseGlobal.hasCurrentPCKey = TRUE;
            nlfseGlobal.keyRefreshTime = currentSecond + NLSE_KEY_CACHE_TIME;
        }
        else
        {
            NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey: Get key request failed %d\n", response->msg.result);
            //Erase the stored current key regardless of the reason of failure
            //(comm error, no key exists in key ring, etc.)
            nlfseGlobal.hasCurrentPCKey = FALSE;
            nlfseGlobal.keyRefreshTime = currentSecond + 1;
            status = STATUS_GENERIC_COMMAND_FAILED;
        }
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    }
    else
    {
        ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
        ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
        NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, "NLSE!UpdateCurrentPCKey: wrong size response %d\n", responseLen);
        nlfseGlobal.keyRefreshTime = currentSecond + 1;
        status = STATUS_INVALID_BUFFER_SIZE;
    }

_exit:
    return status;
}

//Get the PC key using PC key ID
//return TRUE on success
BOOLEAN NLSEGetPCKeyByID(__in    char         *keyRingName,
			 __in    NLSE_KEY_ID  *keyID,
			 __in    ULONG        pid,
			 __inout char  *key)
{
  PNLSE_KERNEL_REQUEST          req=NULL;
  PNLSE_KERNEL_REQUEST_RESPONSE response=NULL;
  ULONG                         responseLen;
  NTSTATUS                      status;
  LARGE_INTEGER                 timeout;	
  PNLSE_PORT_CONTEXT            portCtx = NULL;

#ifdef NLSE_DEBUG_FAKE_PC_KEY
  RtlFillMemory(key,NLSE_KEY_LENGTH_IN_BYTES,'a');
  return TRUE;
#endif

  //Sending a request to user mode
  req=ExAllocatePoolWithTag(NonPagedPool,
			    sizeof(NLSE_KERNEL_REQUEST),
			    NLSE_MESSAGE_TAG);
  if(req==NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	      ("NLSE!GetPCKeyByID: failed to allocate request\n"));
    return FALSE;
  }
  responseLen=sizeof(FILTER_REPLY_HEADER)+sizeof(NLSE_MESSAGE);
  response=ExAllocatePoolWithTag(NonPagedPool,
				 responseLen,
				 NLSE_MESSAGE_TAG);
  if(response==NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	      ("NLSE!GetPCKeyByID: failed to allocate response\n"));
    ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
    return FALSE;
  }
  RtlZeroMemory(response,responseLen);
  RtlZeroMemory(req,sizeof(NLSE_KERNEL_REQUEST));

  portCtx=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
  if(portCtx->port == NULL) {
    ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
    ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	      ("NLSE!GetPCKeyByID: main port not connected\n"));
    return FALSE;
  }

  //compose request
  req->type=NLSE_KERNEL_REQ_GET_KEY;
  RtlCopyMemory(req->msg.keyRingName,
		keyRingName,
		NLSE_KEY_RING_NAME_MAX_LEN*sizeof(char));
  RtlCopyMemory(&(req->msg.keyID),
		keyID,
		sizeof(NLSE_KEY_ID));
  req->msg.pid=pid;
  responseLen=sizeof(NLSE_MESSAGE);
  req->header.ReplyLength=sizeof(NLSE_MESSAGE);
  timeout.QuadPart=NLSE_DELAY_ONE_SECOND*5;

  //send request and get response
  status = FltSendMessage(nlfseGlobal.filterHandle,
			  &((PFLT_PORT)portCtx->port),
			  &req->type,
			  sizeof(int)+sizeof(NLSE_MESSAGE),
			  &response->msg,
			  &responseLen,
			  &timeout);
  if(status != STATUS_SUCCESS) {
    ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
    ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!GetPCKeyByID: FltSendMessage failed 0x%x\n", 
		status);
    return FALSE;
  }

  if(responseLen == sizeof(NLSE_MESSAGE)) {
    if(response->msg.result == NLSE_KERNEL_REQ_RESULT_SUCCESS) {
      RtlCopyMemory(key, response->msg.key, NLSE_KEY_LENGTH_IN_BYTES);
    } else {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		  "NLSE!GetPCKeyByID: Get key request failed %d\n", 
		  response->msg.result);
      ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
      ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
      return FALSE;
    }
  } else {
    ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
    ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
		"NLSE!GetPCKeyByID: wrong size response %d %d\n", 
		responseLen,
		sizeof(NLSE_MESSAGE));
    return FALSE;
  }
  
  ExFreePoolWithTag(req, NLSE_MESSAGE_TAG);
  ExFreePoolWithTag(response, NLSE_MESSAGE_TAG);
  return TRUE;
}
//end - key management

/** ClientMessage
 *
 *  \brief Handle commands from user-model FilterXXX calls.
 */
NTSTATUS NLSEClientMessage( __in PVOID ConnectionCookie,
			    __in_opt PVOID InputBuffer,
			    __in ULONG InputBufferSize,
			    __out PVOID OutputBuffer,
			    __in ULONG OutputBufferSize,
			    __in PULONG ReturnOutputBufferLength )
{
  PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionCookie;
  NTSTATUS           status = STATUS_SUCCESS;
  NLSE_USER_COMMAND  *cmd = NULL;

  if( InputBuffer == NULL || InputBufferSize != sizeof(NLSE_USER_COMMAND) ) {
    return STATUS_INVALID_PARAMETER;
  }

  cmd = (PNLSE_USER_COMMAND)InputBuffer;

  if(portCtx->portTag == NLSE_PORT_TAG_MAIN_CMD) {
    //command to main port 
    switch( cmd->type )
      {
      case NLSE_USER_COMMAND_ENABLE_FILTER:
	{
	  ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
	  nlfseGlobal.bEnable=TRUE;
	  ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
	  break;
	}
      case NLSE_USER_COMMAND_DISABLE_FILTER:
	{
	  ExAcquireFastMutex(&nlfseGlobal.enableStatusMutex);
	  nlfseGlobal.bEnable=FALSE;
	  ExReleaseFastMutex(&nlfseGlobal.enableStatusMutex);
	  break;
	}
      default:
	break;
      }/* switch */
  } else if(portCtx->portTag == NLSE_PORT_TAG_DRM) {
    //DRM command
    switch( cmd->type )
      {
      case NLSE_USER_COMMAND_SET_IGNORE_PROCESS_BY_PID:
	{
	  break;
	}	
      case NLSE_USER_COMMAND_UNSET_IGNORE_PROCESS_BY_PID:
	{
	  break;
	}	
      case NLSE_USER_COMMAND_SET_DRM_FILE:
	{
	  break;
	}
      case NLSE_USER_COMMAND_CREATE_FILE_RAW:
        {
          NLSECreateFileRaw(&cmd->msg);
          break;
        }
      case NLSE_USER_COMMAND_READ_FILE_RAW:
        {
          NLSEReadFileRaw(&cmd->msg);
          break;
        }
      case NLSE_USER_COMMAND_WRITE_FILE_RAW:
        {
          NLSEWriteFileRaw(&cmd->msg);
          break;
        }
      case NLSE_USER_COMMAND_CLOSE_FILE_RAW:
        {
          NLSECloseFileRaw(&cmd->msg);
          break;
        }
      case NLSE_USER_COMMAND_SET_DRM_PATHS:
        {
          status = NLSEDrmPathListSet(cmd->msg.params.setDrmPaths.numPaths,
                                      cmd->msg.params.setDrmPaths.paths);
          break;
        }
      default:
	break;
      }/* switch */
  }

  return status;
}/* ClientMessage */

/** CheckAndSetDebugMode
 */
__drv_requiresIRQL(PASSIVE_LEVEL)
CheckAndSetDebugMode(VOID)
{
  OBJECT_ATTRIBUTES attributes;
  HANDLE driverRegKey;
  NTSTATUS status;
  ULONG resultLength;
  UNICODE_STRING valueName;
  UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( LONG )] = {0};
  UNICODE_STRING nl_sw_path;
  ULONG debug_mode = 0;

  RtlInitUnicodeString(&nl_sw_path,L"\\Registry\\Machine\\Software\\NextLabs");
  InitializeObjectAttributes(&attributes,&nl_sw_path,OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			     NULL,NULL);
  status = ZwOpenKey(&driverRegKey,KEY_READ,&attributes);
  if( NT_SUCCESS(status) )
  {
    // Read the given value from the registry.
    RtlInitUnicodeString(&valueName,L"DebugMode");
    status = ZwQueryValueKey(driverRegKey,&valueName,
			     KeyValuePartialInformation,
			     buffer,sizeof(buffer),
			     &resultLength);
    if( NT_SUCCESS(status) )
    {
      debug_mode = *((PULONG) &(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
    }
    ZwClose(driverRegKey);
  }

  if( debug_mode )
  {
    NL_KLOG_SetLevel(&nlseKLog,NL_KLOG_LEVEL_DEBUG);
    nlseLoggingFlags = NL_KLOG_LEVEL_DEBUG;
  }
  else
  {
    NL_KLOG_SetLevel(&nlseKLog,NL_KLOG_LEVEL_ERR);
    nlseLoggingFlags = NL_KLOG_LEVEL_ERR;
  }
}/* CheckAndSetDebugMode */

NTSTATUS NLSEClientConnect(__in PFLT_PORT ClientPort,
			   __in PVOID ServerPortCookie,
			   __in PVOID ConnectionContext,
			   __in ULONG SizeOfContext,
			   __out PVOID *ConnectionCookie )
{
  PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionContext;
  int                i;

  CheckAndSetDebugMode();

  //Sanity checking
  if(portCtx == NULL) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	      ("NLSE!NLSEClientConnect: connection context is NULL\n")); 
    return STATUS_INVALID_PARAMETER;
  }
  if(portCtx->portTag != NLSE_PORT_TAG_MAIN_REQUEST &&
     portCtx->portTag != NLSE_PORT_TAG_MAIN_CMD &&
     portCtx->portTag != NLSE_PORT_TAG_DRM) {
    NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	      ("NLSE!NLSEClientConnect: wrong port tag in context\n")); 
    return STATUS_INVALID_PARAMETER;
  }

  //assign port
  if(portCtx->portTag == NLSE_PORT_TAG_MAIN_REQUEST) {
    if(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].port != NULL) {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	       ("NLSE!ClientConnect: main request port has been connected\n"));
      return STATUS_INVALID_PARAMETER;
    }
    *ConnectionCookie=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX]);
    nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].port=ClientPort;
    nlfseGlobal.clientPorts[NLSE_PORT_MAIN_REQUEST_INDEX].portTag=NLSE_PORT_TAG_MAIN_REQUEST;
  }else if(portCtx->portTag == NLSE_PORT_TAG_MAIN_CMD) {
    if(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].port != NULL) {
      NL_KLOG_Log(&nlseKLog,NL_KLOG_LEVEL_ERR, 
	       ("NLSE!ClientConnect: main cmd port has been connected\n"));
      return STATUS_INVALID_PARAMETER;
    }
    *ConnectionCookie=&(nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX]);
    nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].port=ClientPort;
    nlfseGlobal.clientPorts[NLSE_PORT_MAIN_CMD_INDEX].portTag=NLSE_PORT_TAG_MAIN_CMD;
  } else {
    for(i=NLSE_PORT_MAIN_CMD_INDEX+1; i<NLSE_MAX_PORT_CONNECTION; i++) {
      if(nlfseGlobal.clientPorts[i].port == NULL) {
	//find an unassigned port and assign it to the new connection
	nlfseGlobal.clientPorts[i].port = ClientPort;
	nlfseGlobal.clientPorts[i].portTag=NLSE_PORT_TAG_DRM;
	*ConnectionCookie=&(nlfseGlobal.clientPorts[i]);
	break;
      }
    }
  }
  
  return STATUS_SUCCESS;
}/* ClientConnect */

VOID NLSEClientDisconnect( __in PVOID ConnectionCookie )
{
  PNLSE_PORT_CONTEXT portCtx=(PNLSE_PORT_CONTEXT)ConnectionCookie;
  int                i;
  
  for(i=0; i<NLSE_MAX_PORT_CONNECTION; i++) {
    if(&nlfseGlobal.clientPorts[i] == portCtx) {
      //find the corresponding port context and free it
      FltCloseClientPort(nlfseGlobal.filterHandle,
                         (PFLT_PORT *) &nlfseGlobal.clientPorts[i].port);
      break;
    }
  }
}/* ClientDisconnect */
