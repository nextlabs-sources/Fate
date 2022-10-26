#define WINVER _WIN32_WINNT_WINXP
#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <process.h>
#include <fltuser.h>
#include "NLSELib.h"
#include "NLSESDKWrapper.h"
#include "celog.h"

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

#define CELOG_CUR_MODULE L"NLSEPlugin"
#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENCRYPTION_USER_NLSEPLUGIN_CPP

#define NLSE_SERVICE_NAME   L"NLSysEncryption"

/** NLSEPluginContext
 *
 *  Context information for NLSE plug-in.
 */
typedef struct
{
  HANDLE th;            /* Thread handle */
  HANDLE reqPort;          /* Filter communication port */
  HANDLE cancel_event;  /* Cancel support for PluginUnload */
} NLSEPluginContext;



/************************************************************************/
// Define DEBUG Flags
/************************************************************************/
#ifdef _DEBUG

/************************************************************************/
// In debug mode, developer can enable/disable these flags
/************************************************************************/
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE

#else

/************************************************************************/
// In release mode, these flags should always be FALSE
// DON'T change them
/************************************************************************/
#define NLSE_DEBUG_FAKE_PC_KEY              FALSE

#endif /* _DEBUG */

#if (NLSE_DEBUG_FAKE_PC_KEY)
#  ifndef _DEBUG
#    error NLSE ERROR - Cannot set debug options for a release library.
#  endif
#endif



//NLSE service should be started at boot time in order to
//have full control of the system.
//Here, we check if the service started
static bool StartNLSEService()
{
  SERVICE_STATUS_PROCESS ssStatus; 
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  DWORD dwOldCheckPoint; 
  DWORD dwStartTickCount;
  DWORD dwWaitTime;
  DWORD dwBytesNeeded;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager(NULL,    // local computer
			       NULL,    // servicesActive database 
			       SC_MANAGER_ALL_ACCESS);  // full access rights  
  if (NULL == schSCManager) {
    CELOG_LOG(CELOG_CRITICAL,
	      L"EnableNLSEService: OpenSCManager failed (%d)\n", 
	      GetLastError());
    return false;
  }

  // Get a handle to the service.
  schService = OpenService(schSCManager,         // SCM database 
			   NLSE_SERVICE_NAME,   // name of service 
			   SERVICE_ALL_ACCESS);  // full access 
  if (schService == NULL) {
    CELOG_LOG(CELOG_CRITICAL,
	     L"EnableNLSEService: OpenService failed (%d)\n", GetLastError()); 
    CloseServiceHandle(schSCManager);
    return false;
  }   

  // Check the service status 
  if (!QueryServiceStatusEx(schService, // handle to service 
			    SC_STATUS_PROCESS_INFO,     // information level
			    (LPBYTE) &ssStatus,         // address of structure
			    sizeof(SERVICE_STATUS_PROCESS), //size of structure
			    &dwBytesNeeded )) {
    // size needed if buffer is too small
    CELOG_LOG(CELOG_CRITICAL,
	     L"EnableNLSEService: QueryServiceStatusEx failed (%d)\n", 
	     GetLastError());
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    return false; 
  }

  if(ssStatus.dwCurrentState == SERVICE_STOPPED || 
     ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
    //The service is not running; failed to enable its functionality
    CELOG_LOG(CELOG_DEBUG,
	     NLSE_SERVICE_NAME L" service has been stopped.\n");
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    return false; 
  }

  // Save the tick count and initial checkpoint.
  dwStartTickCount = GetTickCount();
  dwOldCheckPoint = ssStatus.dwCheckPoint;

  //The service is in start pending; wait for its start
  while (ssStatus.dwCurrentState == SERVICE_START_PENDING)  { 
    // Do not wait longer than the wait hint. A good interval is 
    // one-tenth the wait hint, but no less than 1 second and no 
    // more than 10 seconds. 
 
    dwWaitTime = ssStatus.dwWaitHint / 10;

    if( dwWaitTime < 1000 )
      dwWaitTime = 1000;
    else if ( dwWaitTime > 10000 )
      dwWaitTime = 10000;

    Sleep( dwWaitTime );

    // Check the status again. 
    if (!QueryServiceStatusEx(schService,             // handle to service 
			      SC_STATUS_PROCESS_INFO, // info level
			      (LPBYTE) &ssStatus,     // address of structure
			      sizeof(SERVICE_STATUS_PROCESS), // size 
			      &dwBytesNeeded ) )  { 
      // if buffer too small
      CELOG_LOG(CELOG_CRITICAL,
	       L"QueryServiceStatusEx failed (%d)\n", GetLastError());
      break; 
    }
 
    if ( ssStatus.dwCheckPoint > dwOldCheckPoint ) {
      // Continue to wait and check.
      dwStartTickCount = GetTickCount();
      dwOldCheckPoint = ssStatus.dwCheckPoint;
    } else {
      if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint) {
        // No progress made within the wait hint.
        break;
      }
    }
  }

  // Determine whether the service is running.
  if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
    CELOG_LOG(CELOG_INFO,
	     NLSE_SERVICE_NAME L" Service started.\n");
  } else { 
    CELOG_LOG(CELOG_ERROR,NLSE_SERVICE_NAME L" Service not started. \n");
    CELOG_LOG(CELOG_ERROR,L"  Current State: %d\n", ssStatus.dwCurrentState); 
    CELOG_LOG(CELOG_ERROR,L"  Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
    CELOG_LOG(CELOG_ERROR,L"  Check Point: %d\n", ssStatus.dwCheckPoint); 
    CELOG_LOG(CELOG_ERROR,L"  Wait Hint: %d\n", ssStatus.dwWaitHint); 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
    return false;
  } 

  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);

  return true;
}

static void __cdecl nlse_plugin_worker( LPVOID in_context )
{
  assert( in_context != NULL );
  CELOG_LOG(CELOG_DEBUG,L"nlse plugin worker started\n");
  NLSEPluginContext* context = (NLSEPluginContext*)in_context;
  NLSE_KERNEL_REQUEST req;
  NLSE_KERNEL_REQUEST_RESPONSE replyMsg;
  OVERLAPPED io;
  ULONG reqLen=sizeof(FILTER_MESSAGE_HEADER)+sizeof(int)+sizeof(NLSE_MESSAGE);
  ULONG replyLen=sizeof(FILTER_REPLY_HEADER)+sizeof(NLSE_MESSAGE);
  PNLSE_KERNEL_REQUEST pReq;
  HRESULT hr;
  BOOLEAN bFree=FALSE;

  if( context == NULL )
  {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin context is NULL\n");
    _endthreadex(ERROR_INVALID_PARAMETER);
  }

  memset(&io,0x00,sizeof(io));
  io.hEvent = CreateEventA(NULL,TRUE,FALSE,NULL);
  if(io.hEvent == NULL) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin CreateEventA failed\n");
    _endthreadex(ERROR_NOT_ENOUGH_MEMORY);
  }

  /* Condition of non-NULL context is to shut up prefast warning.  It cannot determine that
   * context value is checked in conditional above this point.
   */
  for( ; context != NULL ; ) {
    bFree=FALSE;
    SecureZeroMemory(&req,sizeof(NLSE_KERNEL_REQUEST));
    hr = FilterGetMessage(context->reqPort,
			  (PFILTER_MESSAGE_HEADER)&req,
			  reqLen,
			  &io);
    if( hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING) ) {
      break;
    }

    /* Wait for cancel or event from nltamper driver */
    HANDLE wait_handles[] = { context->cancel_event , io.hEvent };
    DWORD wait_result = WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE);

    /* If cancellation occurs from the unload callback the cancel event will be
     * in a signaled stage.  Test for signaled state.
     *
     * WaitForMultipleObjects returns the lowest 
     * array index which is the cancel event.
     */
    if( wait_result == WAIT_OBJECT_0 ) {
      CancelIo(context->reqPort);
      if( io.hEvent != NULL )
      {
	CloseHandle(io.hEvent);
      }
      break;
    }

    if( wait_result == (WAIT_OBJECT_0 + 1) ) {     /* IO event */
      DWORD out_size = 0;
      if(GetOverlappedResult(context->reqPort,&io,&out_size,FALSE) != TRUE) {
	DWORD err=GetLastError();
	if(err != ERROR_INSUFFICIENT_BUFFER) {
	  CELOG_LOG(CELOG_ERROR, 
		   L"NLSELib!worker: GetOverlappedResult failed %d\n", 
		   err);
	  ResetEvent( io.hEvent );
	  continue;
	} else {
	  //allocate a big buffer and get message again
	  pReq = (PNLSE_KERNEL_REQUEST)malloc(out_size);
	  SecureZeroMemory(pReq,out_size);

	  /* Annotations indicate that parameter 4 must not be NULL which is incorrect per
	   * documentation.  It is optional when caller requests overalpped/async i/o. 
	   */
#pragma warning( push )
#pragma warning( disable : 6309 )
	  hr = FilterGetMessage(context->reqPort, 
				(PFILTER_MESSAGE_HEADER)pReq, 
				out_size,
				NULL);	
#pragma warning( pop )
	  if(hr != S_OK) {
	    CELOG_LOG(CELOG_ERROR, 
		     L"NLSELib!worker: GetMessage 2nd time err=0x%x\n", 
		     hr);
	    ResetEvent( io.hEvent );
	    SecureZeroMemory(pReq,out_size);
	    free(pReq);
	    continue;
	  }	
	  bFree=TRUE;
	}
      }else {
	/* IO completed with success */
	pReq=&req;
      }

      //Handling kernel request
      //1. Initialize reply
      SecureZeroMemory(&replyMsg,sizeof(replyMsg));
      replyMsg.replyHeader.Status=0;
      replyMsg.replyHeader.MessageId=pReq->header.MessageId;
      //2. Handle each type of request
      if(pReq->type == NLSE_KERNEL_REQ_GET_KEY) {  
#if NLSE_DEBUG_FAKE_PC_KEY
        memset(replyMsg.msg.key, 'a', NLE_KEY_LENGTH_IN_BYTES);
        replyMsg.msg.result=NLSE_KERNEL_REQ_RESULT_SUCCESS;
#else
	if(!CESDKConnect()) {
	  replyMsg.msg.result=NLSE_KERNEL_REQ_RESULT_SDK_FAILURE;
	} else {
	  CEResult_t ceRet=CESDKKeyManagementAPI(&pReq->msg,
					       &(replyMsg.msg));
	  if(ceRet == CE_RESULT_SUCCESS) {
	    replyMsg.msg.result=NLSE_KERNEL_REQ_RESULT_SUCCESS;
	  } else if(ceRet == CE_RESULT_CONN_FAILED ||
		    ceRet == CE_RESULT_TIMEDOUT){
	    replyMsg.msg.result=NLSE_KERNEL_REQ_RESULT_SDK_FAILURE;
	  } else {
	    //TODO: other errors?
	    replyMsg.msg.result=NLSE_KERNEL_REQ_RESULT_UNTRUSTED;
	  }
	}
#endif /* NLSE_DEBUG_FAKE_PC_KEY */
      }
      //3. Reply
      hr=FilterReplyMessage(context->reqPort,
			    (PFILTER_REPLY_HEADER)&replyMsg,
			    replyLen);
      SecureZeroMemory(&replyMsg,sizeof(replyMsg));
      if(hr != S_OK ) {
	CELOG_LOG(CELOG_ERROR, 
		 L"NLSELib!worker: Reply err=0x%X msgID=0x%I64X\n",
		 hr, pReq->header.MessageId);
      } else {
	CELOG_LOG(CELOG_DEBUG,
		 L"NLSELib!worker: Replied pid=%lu len=%lu msgID=0x%I64X\n", 
		 pReq->msg.pid, replyLen, pReq->header.MessageId);
      }
      if(bFree)
      {
	SecureZeroMemory(pReq,out_size);
	free(pReq);
      }
      ResetEvent( io.hEvent );
    }
  }/* for */
  _endthreadex(ERROR_SUCCESS);
}/* nlse_plugin_worker */

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
  int                result = 0;
  HRESULT            hr;
  NLSE_PORT_CONTEXT  portCtx;
  NLSEPluginContext* context = NULL;
  nextlabs::high_resolution_timer ht;
  ULONG              pcPid=0; 

#ifdef _DEBUG
  CELOG_LOG(CELOG_WARNING, L"********************************************************************************\n");
  CELOG_LOG(CELOG_WARNING, L"* NextLabs System Encryption Plugin Library\n");
  CELOG_LOG(CELOG_WARNING, L"*\n");
  CELOG_LOG(CELOG_WARNING, L"* WARNING: The library is compiled with debug parameters\n");
  CELOG_LOG(CELOG_WARNING, L"*\n");

#if NLSE_DEBUG_FAKE_PC_KEY
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_PC_KEY on\n");
#else
  CELOG_LOG(CELOG_WARNING, L"*   NLSE_DEBUG_FAKE_PC_KEY off\n");
#endif

  CELOG_LOG(CELOG_WARNING, L"*\n");
  CELOG_LOG(CELOG_WARNING, L"********************************************************************************\n");
#endif /* _DEBUG */

  CELOG_LOG(CELOG_INFO,L"nlse plugin starting\n");

  nextlabs::feature_manager feat;
  feat.open();
  if( feat.is_enabled(NEXTLABS_FEATURE_ENCRYPTION_SYSTEM) == false )
  {
    CELOG_LOG(CELOG_INFO,L"No license for SE\n");
    return 0;
  }

  assert( in_context != NULL );
  if( in_context == NULL ) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin in_context == NULL\n");
    return 0;
  }

  if(CESDKInit() == false) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin CESDKInit failed\n");
    return 0;
  }

  context=(NLSEPluginContext*)malloc(sizeof(NLSEPluginContext));
  if( context == NULL ) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin malloc failed\n");
    return 0;  /* out of memory */
  }

  *in_context = (void*)context;
  memset(context,0x00,sizeof(NLSEPluginContext));

  /* Cancel event in a non-signaled state */
  context->cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);
  if( context->cancel_event == NULL )
  {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin CreateEventA failed\n");
    goto PluginEntry_complete;
  }

  /* Ensure the filter is started */
  if(!StartNLSEService()) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin cannot start " NLSE_SERVICE_NAME L" service\n");
    goto PluginEntry_complete;
  }

  CELOG_LOG(CELOG_DEBUG,L"nlse plugin: connecting to filter port.\n");
  //set up request port from kernel
  portCtx.portTag=NLSE_PORT_TAG_MAIN_REQUEST;
  hr = FilterConnectCommunicationPort(NLSE_PORT_NAME,
				      0,
				      &portCtx,
				      sizeof(portCtx),
				      NULL,
				      &(context->reqPort));
  if( IS_ERROR(hr)  && context->reqPort != NULL) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin FilterConnectCommunicationPort failed\n");
    goto PluginEntry_complete;
  }

  /* Start worker thread for file system event notifications */
  context->th = (HANDLE)_beginthread(nlse_plugin_worker,0,context);
  if( context->th == NULL ) {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin CreateThread failed\n");
  }

  CELOG_LOG(CELOG_DEBUG,L"nlse plugin: connecting to Policy Controller\n");

  //try to set up sdk connection
  CESDKConnect();

  //Enable NLSE filter driver functionality
  CELOG_LOG(CELOG_DEBUG,L"nlse plugin: enabling filter\n");
  if(NLSEUserCmd_EnableFilter()) {
    CELOG_LOG(CELOG_DEBUG,L"nlse plugin: enabled filter\n");
    result = 1;
    pcPid=GetCurrentProcessId();
    if(NLSEUserCmd_SetIgnoreProcessByPID(pcPid)) {
      CELOG_LOG(CELOG_DEBUG,L"nlse plugin: exempt Policy Controler (%d)\n",
		pcPid);
    }
  } else {
    CELOG_LOG(CELOG_CRITICAL,L"nlse plugin: failed to enable filter\n");
  }
 PluginEntry_complete:
  if( result == 0 ) {   /* cleanup on failure */
    CESDKDisconnect();

    if( context->reqPort != INVALID_HANDLE_VALUE ) {
      CloseHandle(context->reqPort);
    }
    if( context->cancel_event != INVALID_HANDLE_VALUE &&
	context->cancel_event != NULL ) {
      CloseHandle(context->cancel_event);
    }
    free(context);
    context = NULL;
  }

  ht.stop();
  CELOG_LOG(CELOG_INFO,L"nlse plugin %s (%fms)\n",
	    result == 0 ? L"failed" : L"succeed",
	    ht.diff());

  return result;
}/* PluginEntry */

/*****************************************************************************
 * PluginUnload
 ****************************************************************************/
extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
  nextlabs::high_resolution_timer ht;
  ULONG                           pcPid=0;

  CELOG_LOG(CELOG_DEBUG,L"nlse plugin unloading\n");
  CESDKDisconnect();

  pcPid=GetCurrentProcessId();
  if(NLSEUserCmd_UnsetIgnoreProcessByPID(pcPid)) {
    CELOG_LOG(CELOG_DEBUG,L"nlse plugin: unset ignore pid (%d)\n",
	      pcPid);
  }

  //Disable NLSE filter driver's functionality
  CELOG_LOG(CELOG_DEBUG,L"nlse plugin unloading - disable filter\n");
  if(!NLSEUserCmd_DisableFilter()) {
    CELOG_LOG(CELOG_DEBUG,L"nlse plugin unload: can't disable filter\n");
    return 0;
  }

  if( in_context == NULL ) {
    CELOG_LOG(CELOG_ERROR,L"nlse plugin unload: in_context is NULL\n");
    return 0;
  }

  NLSEPluginContext* context = (NLSEPluginContext*)in_context;
  
  /* Signal worker thread to complete */
  CELOG_LOG(CELOG_DEBUG,L"nlse plugin unloading - signal worker\n");
  SetEvent(context->cancel_event);
  WaitForSingleObject(context->th,INFINITE);

  //clean up
  CloseHandle(context->th);
  CloseHandle(context->reqPort);
  CloseHandle(context->cancel_event);
  delete in_context;
  
  ht.stop();
  CELOG_LOG(CELOG_DEBUG,L"nlse plugin unload succeed (%fms)\n",ht.diff());
  return 1;
}/* PluginUnload */
