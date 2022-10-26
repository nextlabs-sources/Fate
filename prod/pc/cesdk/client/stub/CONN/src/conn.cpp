/*==========================conn.cpp========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/22/2007                                                       *
 * Note   : Implementations of SDK CECONN_XXX APIs.                         *
 *==========================================================================*/
#define NLMODULE   CONN
#define NLTRACELEVEL 3  

#include <time.h>
#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#pragma warning(push)
#pragma warning(disable : 6386)
#include <ws2tcpip.h>
#pragma warning(pop)
#endif
#if defined (Linux) || defined (Darwin)
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include "brain.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"
#include "nlthread.h"

namespace {
enum {CECONN_STRING_LENGTH_MAX=1024};

#ifdef LINUX
#define USE_LOOPBACK_FOR_LOCALHOST_ADDR
#endif

//Get local host IP address
bool GetHostIPAddress(int &ip)
{
#ifdef USE_LOOPBACK_FOR_LOCALHOST_ADDR
  ip = (int)0x0100007f;
#else
  char hostName[1024];
  struct addrinfo hints, *res;
  struct in_addr hostIPAddr;
  int err;

  if(gethostname(hostName, 1024) != 0) {
    TRACE(0, _T("Failed to get host name: error=%d\n"), errno);
    return false;
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  if ((err = getaddrinfo(hostName, NULL, &hints, &res)) != 0) {
    TRACE(0, _T("Failed to get host ip: error=%d\n"), err);
    return false;
  }

  hostIPAddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
  ip=(int)(hostIPAddr.s_addr);

  //printf("host ip address : %s %d\n", inet_ntoa(hostIPAddr), ip);

  freeaddrinfo(res);

#endif
  return true;  
}

//Free Handle 
//Have to do it here because MS local/global allocated memory restriction
void FreeHandle(CEHandle handle)
{
  if(handle->type) {
    delete [] handle->type;
    handle->type=NULL;
  } 

  if(handle->appName) {
    delete [] handle->appName;
    handle->appName=NULL;
  } 

  if(handle->binaryPath) {
    delete [] handle->binaryPath;
    handle->binaryPath=NULL;
  }

  if(handle->userName) {
    delete [] handle->userName;
    handle->userName=NULL;
  }

  if(handle->userID) {
    delete [] handle->userID;
    handle->userID=NULL;
  }

  if(handle->fingerprint) {
    delete [] handle->fingerprint;
    handle->fingerprint=NULL;
  }

  delete handle;
}

//Do CECONN_Initialize RPC
CEResult_t CallInitRPC(CEString type,
		       CEString appName, 
		       CEString binaryPath,
		       CEString userName,
		       CEString userID,
		       CEHandle connectHandle,
		       CEint32  timeout_in_millisec)
{
  std::vector<void *> args;
  nlchar reqIDStr[100];
  args.reserve(RPC_MAX_NUM_ARGUMENTS);
  nlsprintf(reqIDStr, 100, _T("%lu+%lu"), 
	    nlthread_selfID(), PEPMAN_GetUniqueRequestID());
  CEString reqIDArg = CEM_AllocateString(reqIDStr);
  nlstring reqID(reqIDStr);
  size_t reqLen;

  //Construct input arguments vector
  args.push_back(reqIDArg);
  args.push_back(type);
  args.push_back(appName);
  args.push_back(binaryPath);
  args.push_back(userName);
  args.push_back(userID);
  args.push_back(&connectHandle->hostIPAddress);
  //Marshal request 
  char *packed=Marshal_PackReqFunc(_T("CECONN_Initialize"), 
				   args, reqLen);
  if(!packed) {
    TRACE(1, 
	  _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
    return CE_RESULT_INVALID_PARAMS;
  } else 
    TRACE(4,_T("CECONN_Initialize request long %d bytes\n"), reqLen);
    
  //Do RPC call
  nlstring reqFunc(100u,' ');
  CEResult_t reqResult;
  vector<void *> reqOut;
  reqFunc=_T("CECONN_Initialize");
  reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
  CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, connectHandle, 
				   reqFunc, reqResult, reqOut, 
				   timeout_in_millisec);

  if(result != CE_RESULT_SUCCESS) {
    TRACE(1, _T("CECONN_Initialize: PEPMAN_RPCCall failed errno=%d\n"),
	  result);
    Marshal_PackFree(packed); 
    reqOut.clear(); 
    CEM_FreeString(reqIDArg);
    return result;
  }

  //Assign returned value: session id on server side
  if(reqResult == CE_RESULT_SUCCESS)
    connectHandle->sessionID = *((unsigned long long *)reqOut[1]);

  //Cleaning up
  Marshal_PackFree(packed); 
  Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
  CEM_FreeString(reqIDArg);
  return reqResult;
}

CEResult_t DoInitConn(nlsocket &socketFd, CEString pdpHostName, bool bJoin)
{
  //Setup the socket connection
  CEResult_t result;
  if(pdpHostName && pdpHostName->length != 0)
    result=PEPMAN_Init(pdpHostName, socketFd, bJoin); 
  else
    result=PEPMAN_Init(NULL, socketFd, bJoin); 
    
  NL_sleep(2);

  return result;
}

CEResult_t DoCloseRPC(CEHandle handle, CEint32 timeout_in_millisec)
{
  std::vector<void *> args;
  nlchar reqIDStr[100];
  args.reserve(RPC_MAX_NUM_ARGUMENTS);
  nlsprintf(reqIDStr, 100, _T("%lu+%lu"), 
	    nlthread_selfID(), PEPMAN_GetUniqueRequestID());
  CEString reqIDArg = CEM_AllocateString(reqIDStr);
  nlstring reqID(reqIDStr);
  size_t reqLen;

  TRACE(4, 
	_T("DoCloseRPC a=%s b=%s un=%s ui=%s t=%d\n"),
	handle->appName, handle->binaryPath, handle->userName, 
	handle->userID, timeout_in_millisec);
  //Construct input arguments vector
  args.push_back(reqIDArg);
  args.push_back(&(handle->sessionID));

  //Marshal request 
  char *packed=Marshal_PackReqFunc(_T("CECONN_Close"), 
				   args, reqLen);
  if(!packed) {
    TRACE(1, 
	  _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
    return CE_RESULT_INVALID_PARAMS;
  } else 
    TRACE(4,_T("CECONN_Initialize request long %d bytes\n"), reqLen);
    
  //Do RPC call
  nlstring reqFunc(100u,' ');
  CEResult_t reqResult;
  vector<void *> reqOut;
  reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
  CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
				   reqFunc, reqResult, reqOut, 
				   timeout_in_millisec);
  if(result != CE_RESULT_SUCCESS) {
    TRACE(1, _T("CECONN_CloseRPC: PEPMAN_RPCCall failed errno=%d\n"),
	  result);
    Marshal_PackFree(packed); 
    CEM_FreeString(reqIDArg);
    return result;
  }

  //Cleaning up
  Marshal_PackFree(packed); 
  Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
  CEM_FreeString(reqIDArg);
  return reqResult;  
}
/* ------------------------------------------------------------------------
 * CECONNInitialize()
 *
 * Initializes the connection between the client to the Policy Decision
 * Point Server.
 * 
 * Arguments : app    : the application assoicate with the client PEP
 *             user   : a user in the application
 *             pdpHostName (INPUT): the PDP host name. If it is NULL, it is
 *                                 going to connect to the server on
 *                                 local machine.
 *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
 *             for this RPC
 *             bJoin (INPUT): If it is true, the enforcer back-end receiving
 *                            thread need to be waitied. 
 * 
 * Output    : connectHandle: connection handle for subsequent call
 * 
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CECONNInitialize (CEString type,
		  CEApplication &app, 
		  CEUser &user, 
		  CEString pdpHostName,
		  CEHandle * connectHandle,
		  CEint32 timeout_in_millisec,
		  bool bJoin) 
{
  CEResult_t result;
  nlsocket socketFd;

  if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) {
    TRACE(0, 
	  _T("Invalid timeout value '%d'\n"),timeout_in_millisec);    
    return CE_RESULT_INVALID_PARAMS;
  }

  if(timeout_in_millisec == 0) {
    return CE_RESULT_TIMEDOUT;
  }

  /* type is a required, non-nil, parameter */
  if( type == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  *connectHandle=NULL;

  /*if(app.appName == NULL || (app.appName && app.appName->length==0)) {
    TRACE(0, _T("The name of application can't be empty\n"));
    return CE_RESULT_INVALID_PARAMS;
    }*/

  result=DoInitConn(socketFd, pdpHostName, bJoin);
  if(result == CE_RESULT_SUCCESS) {
    CEHandle h= new struct _CEHandle();

    //assign type
    if(type && type->length != 0) {
      h->type = new (nothrow) nlchar [type->length+1];
      nlstrncpy_s(h->type, type->length+1, type->buf, _TRUNCATE);
    } else 
      type=NULL;

    //assign application name
    if(app.appName && app.appName->length != 0) {
      h->appName = new nlchar [app.appName->length+1];
      nlstrncpy_s(h->appName, app.appName->length+1, app.appName->buf,
                  _TRUNCATE);
    } else 
      app.appName=NULL;

    //assign binaryPath
    if(app.appPath && app.appPath->length != 0) {
      h->binaryPath = new nlchar [app.appPath->length+1];
      nlstrncpy_s(h->binaryPath, app.appPath->length+1, app.appPath->buf,
                  _TRUNCATE);
    } else
      app.appPath=NULL;

    //Assig user name
    if(user.userName && user.userName->length != 0) {
      h->userName = new nlchar [user.userName->length+1];
      nlstrncpy_s(h->userName, user.userName->length+1, user.userName->buf,
                  _TRUNCATE);
    } else
      user.userName=NULL;

    //Assign user ID
    CEString realUserID=user.userID;
    if(user.userID && user.userID->length != 0) {
      h->userID = new nlchar [user.userID->length+1];
      nlstrncpy_s(h->userID, user.userID->length+1, user.userID->buf,
                  _TRUNCATE);
    } else {
      h->userID = new (nothrow) nlchar [CECONN_STRING_LENGTH_MAX];
      if (h->userID != NULL)
      {
          if(NL_getUserId (h->userID, CECONN_STRING_LENGTH_MAX) != BJ_OK) {
    	TRACE(0, _T("Failed to get the current user ID\n"));
    	nlstrcpy_s(h->userID, CECONN_STRING_LENGTH_MAX, _T("-1"));
          }	
          realUserID=CEM_AllocateString(h->userID);
      }
    }

    //Get fingerprint
    nlstring appPathName;
    if(app.appPath && app.appPath->length != 0) {
      appPathName=app.appPath->buf;
      h->fingerprint = new nlchar[CECONN_STRING_LENGTH_MAX];
      if(!NL_GetFingerprint(appPathName.c_str(),
			    h->fingerprint, 
			    CECONN_STRING_LENGTH_MAX)) {
	TRACE(0, _T("Failed to get the fingerprint of application '%s'\n"), 
	      appPathName.c_str());
	delete [] h->fingerprint;
	h->fingerprint=NULL;
      }
    } else
      h->fingerprint=NULL;      
      
    //Get host ip address
    GetHostIPAddress(h->hostIPAddress);

    //The rest members of CEHandle
    h->tID=nlthread_selfID();
    h->tHandle=nlthread_self();
    h->clientSfd = socketFd;

    result=CallInitRPC(type,
		       app.appName, 
		       app.appPath, 
		       user.userName, 
		       realUserID,
		       h,
		       timeout_in_millisec);

    if(user.userID == NULL)
      CEM_FreeString(realUserID);

    if(result != CE_RESULT_SUCCESS) 
    {
      FreeHandle(h);
    }
    else 
      *connectHandle = h;
  } 
  return result;  
}
}

/* ------------------------------------------------------------------------
 * CECONN_Initialize()
 *
 * Initializes the connection between the client to the Policy Decision
 * Point Server.
 * 
 * Arguments : app    : the application assoicate with the client PEP
 *             user   : a user in the application
 *             pdpHostName (INPUT): the PDP host name. If it is NULL, it is
 *                                 going to connect to the server on
 *                                 local machine.
 *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
 *             for this RPC
 * 
 * Output    : connectHandle: connection handle for subsequent call
 * 
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CECONN_Initialize (CEApplication app, 
		   CEUser user, 
		   CEString pdpHostName,
		   CEHandle * connectHandle,
		   CEint32 timeout_in_millisec) 
{
  CEResult_t result;

  try {
    CEString type = CEM_AllocateString(_T("CESDK"));
    result=CECONNInitialize (type,
			     app, 
			     user, 
			     pdpHostName,
			     connectHandle,
			     timeout_in_millisec,
			     false); 
    CEM_FreeString(type);
  } catch (std::exception &e) {
    TRACE(0, _T("CECONN_Initialize failed due to '%s'\n"), e.what());
    return CE_RESULT_CONN_FAILED;
  } catch (...) {
    TRACE(0, _T("CECONN_Initialize failed due to unknown reason\n"));
    return CE_RESULT_CONN_FAILED;
  }
  return result;  
}
/* ------------------------------------------------------------------------
 * CECONN_DLL_Activate
 *
 * Activate the connection between the dll client to the Policy Decision
 * Point Server.
 * 
 * Arguments : app    : the application assoicate with the client PEP
 *             user   : a user in the application
 *             pdpHostName (INPUT): the PDP host name. If it is NULL, it is
 *                                 going to connect to the server on
 *                                 local machine.
 *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
 *             for this RPC
 * 
 * Output    : connectHandle: connection handle for subsequent call
 * 
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CECONN_DLL_Activate(CEApplication app, 
		    CEUser user, 
		    CEString pdpHostName,
		    CEHandle * connectHandle,
		    CEint32 timeout_in_millisec) 
{
  CEResult_t result;

  try {
    CEString type = CEM_AllocateString(_T("SDK"));
    result=CECONNInitialize (type,
			     app, 
			     user, 
			     pdpHostName,
			     connectHandle,
			     timeout_in_millisec,
			     true); 
    CEM_FreeString(type);
  } catch (std::exception &e) {
    TRACE(0, _T("CECONN_DLL_Activate failed due to '%s'\n"), e.what());
    return CE_RESULT_CONN_FAILED;
  } catch (...) {
    TRACE(0, _T("CECONN_DLL_Activate failed due to unknown reason\n"));
    return CE_RESULT_CONN_FAILED;
  }
  return result;  
}

/* ------------------------------------------------------------------------
 * CECONN_Close()
 *
 * Close the connection between the client and the Policy Decision
 * Point Server.
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 *             
 * ------------------------------------------------------------------------
 */ 

CEResult_t CECONN_Close (CEHandle handle, CEint32 timeout_in_millisec)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_NULL_CEHANDLE;

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) {
      TRACE(0, 
	    _T("Invalid timeout value '%d'\n"),timeout_in_millisec);    
      return CE_RESULT_INVALID_PARAMS;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }

    result=PEPMAN_IsRunning();
    if(result != CE_RESULT_SUCCESS) 
      return result;
    
    result=DoCloseRPC(handle, timeout_in_millisec);
    if(result != CE_RESULT_SUCCESS) 
      return result;

    result=PEPMAN_Close(false); //not to close socket

    if(handle)
      FreeHandle(handle);
    
  } catch (std::exception &e) {
    TRACE(0, _T("CECONN_Close failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CECONN_Close failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
  return result;    
}
/* ------------------------------------------------------------------------
 * CECONN_DLL_Deactivate
 *
 * Deactivate the connection between the dll client and the Policy Decision
 * Point Server.
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 *             
 * ------------------------------------------------------------------------
 */ 
CEResult_t CECONN_DLL_Deactivate (CEHandle handle, CEint32 timeout_in_millisec)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_INVALID_PARAMS;

    if(!(timeout_in_millisec == CE_INFINITE || timeout_in_millisec >=0)) {
      TRACE(0, 
	    _T("Invalid timeout value '%d'\n"),timeout_in_millisec);    
      return CE_RESULT_INVALID_PARAMS;
    }

    if(timeout_in_millisec == 0) {
      return CE_RESULT_TIMEDOUT;
    }

   //Send close rpc, so pdp can clean up this session
    DoCloseRPC(handle, timeout_in_millisec);

    //Shutdown PEPMan
    result=PEPMAN_Close(true); //true, close socket to shutdown PEPMan

    if(handle)
      FreeHandle(handle);    
  } catch (std::exception &e) {
    TRACE(0, _T("CECONN_DLL_Deactivate failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CECONN_DLL_Deactivate failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
  return result;    
}
/* ------------------------------------------------------------------------
 * CSCINVOKE_CECONN_Initialize()
 *
 * Initializes the connection between the client to the Policy Decision
 * Point Server.
 * 
 * Arguments : appName : the name of the application assoicated with 
 *                       the client PEP
 *             appPath: the path to the application assocated with the client
 *                      PEP
 *             userName : the name of the user of the application
 *             userID: the ID of the user of the application
 *             pdpHostName (INPUT): the PDP host name. If it is NULL, it is
 *                                 going to connect to the server on
 *                                 local machine.
 *             timeout_in_millisec (INPUT): Desirable timeout in milliseconds 
 *             for this RPC
 * 
 * Output    : connectHandle: connection handle for subsequent call
 * 
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CSCINVOKE_CECONN_Initialize (CEString appName,
			     CEString appPath,
			     CEString userName,
			     CEString userID,
			     CEString pdpHostName,
			     CEHandle * connectHandle,
			     CEint32 timeout_in_millisec) 
{
  CEApplication app;
  CEUser user;
  
  app.appName=appName;
  app.appPath=appPath;
  app.appURL=NULL;
  
  user.userName=userName;
  user.userID=userID;
  
  return CECONN_Initialize(app, user, pdpHostName, connectHandle, 
                           timeout_in_millisec);
}
