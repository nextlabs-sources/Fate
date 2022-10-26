/*==========================protect.cpp=====================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 8/16/2007                                                       *
 * Note   : Implementations of SDK CEPROTECT_XXX APIs.                      *
 *==========================================================================*/
#define NLMODULE   CEPROTECT
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
#include "linux_win.h"
#endif
#include <errno.h>
#include "brain.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"
#include "nlthread.h"
#if defined (Linux) || defined (Darwin)
#ifdef nlsprintf
#undef nlsprintf
#define nlsprintf snprintf
#endif
#endif

namespace {
enum {CEPROTECT_STRING_LENGTH_MAX=1024};
}
/* ------------------------------------------------------------------------
 * CEPROTECT_LockKey()
 *
 * Protect a registry key from being modified by other process 
 * 
 * Arguments : handle (INPUT) : connection handle from the CONN_initialize API
 * 
 *             root (INPUT)   : Root of the registry group
 *
 *             key  (INPUT)   : Key to be protected in the format of 
 *                          level1\\level2\\level3\\key
 * 
 * Note      : Applicable only to Window platform
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEPROTECT_LockKey (CEHandle handle, CEKeyRoot_t root, 
			      CEString key)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_NULL_CEHANDLE;  

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"), 
	      nlthread_selfID(), 
	      PEPMAN_GetUniqueRequestID());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(&(handle->sessionID));
    args.push_back(&root);
    args.push_back(key);
    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CEPROTECT_LockKey"),
				     args, reqLen);
    if(!packed) {
      TRACE(1, 
	    _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
      return CE_RESULT_INVALID_PARAMS;
    }
    //Do RPC call
    nlstring reqFunc(100u,' ');
    CEResult_t reqResult;
    vector<void *> reqOut;
    reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
    result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
			  reqFunc, reqResult, reqOut, 
			  CE_INFINITE);
    if(result != CE_RESULT_SUCCESS) {
      TRACE(1, _T("CEPROTECT_LockKey: PEPMAN_RPCCall failed errno=%d\n"),
	    result);
      Marshal_PackFree(packed); 
      reqOut.clear(); 
      CEM_FreeString(reqIDArg);
      return result;
    }

    //Cleaning up
    Marshal_PackFree(packed); 
    Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
    CEM_FreeString(reqIDArg);
    return reqResult;    
  } catch (std::exception &e) {
    TRACE(0, _T("CEPROTECT_LockKey failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CEPROTECT_LockKey failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

}
/* ------------------------------------------------------------------------
 * CEPROTECT_UnlockKey()
 *
 * Undo the protect of a registry key in the system 
 * 
 * Arguments : handle     : connection handle from the CONN_initialize API
 * 
 *             root       : Root of the registry group
 *
 *             key        : Key to be protected in the format of 
 *                          level1\\level2\\level3\\key
 * 
 * Note      : Applicable only to Window platform
 * ------------------------------------------------------------------------
 */ 
CEResult_t
CEPROTECT_UnlockKey (CEHandle handle, CEKeyRoot_t root, CEString key)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_NULL_CEHANDLE;  

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"), 
	      nlthread_selfID(), 
	      PEPMAN_GetUniqueRequestID());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(&(handle->sessionID));
    args.push_back(&root);
    args.push_back(key);
    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CEPROTECT_UnlockKey"),
				     args, reqLen);
    if(!packed) {
      TRACE(1, 
	    _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
      return CE_RESULT_INVALID_PARAMS;
    }
    //Do RPC call
    nlstring reqFunc(100u,' ');
    CEResult_t reqResult;
    vector<void *> reqOut;
    reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
    result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
			  reqFunc, reqResult, reqOut, 
			  CE_INFINITE);
    if(result != CE_RESULT_SUCCESS) {
      TRACE(1, _T("CEPROTECT_UnlockKey: PEPMAN_RPCCall failed errno=%d\n"),
	    result);
      Marshal_PackFree(packed); 
      reqOut.clear(); 
      CEM_FreeString(reqIDArg);
      return result;
    }

    //Cleaning up
    Marshal_PackFree(packed); 
    Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
    CEM_FreeString(reqIDArg);
    return reqResult;    
  } catch (std::exception &e) {
    TRACE(0, _T("CEPROTECT_UnlockKey failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CEPROTECT_UnlockKey failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

}

