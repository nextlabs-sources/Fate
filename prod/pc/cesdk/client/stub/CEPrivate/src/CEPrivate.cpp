/*==========================CEPrivate.cpp===================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by Next Labs,    *
 * Sam Mateo CA, Ownership remains with Next Labs Inc,                      *
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/31/2007                                                       *
 * Note   : Implementations of SDK CEP_XXX APIs.                            *
 *==========================================================================*/
#define NLMODULE   CEPrivate
#define NLTRACELEVEL 3  

#include <time.h>
#include "brain.h"
#include "cetype.h"
#include "marshal.h"
#include "PEPMan.h"
#include "nlthread.h"

#if defined (Linux)
#include "linux_win.h"
#ifdef nlsprintf
#undef nlsprintf
#define nlsprintf snprintf
#endif
#endif

namespace {
bool StopPDP_CheckInputs(CEHandle handle, CEString password)
{
  if(handle == NULL) {
    TRACE(0, 
	  _T("The 'handle' passed to 'CEP_StopPDP' is NULL.\n"));
    return false;
  }

  if(!(password && password->length!=0 && password->buf)) {
    TRACE(0, 
  _T("The 'password' passed to 'CEP_StopPDP' is empty.\n"));
    return false;
  }
  return true;
}

bool GetChallenge_CheckInputs(CEHandle handle, CEString challenge)
{
  if(handle == NULL) {
    TRACE(0, 
	  _T("The 'handle' passed to 'CEP_GetChallenge' is NULL.\n"));
    return false;
  }

  if(!(challenge && challenge->length!=0 && challenge->buf)) {
    TRACE(0, 
  _T("The 'challenge' passed to 'CEP_GetChallenge' is empty.\n"));
    return false;
  }
  return true;
}
}

/* ------------------------------------------------------------------------
 * CEResult_t CEP_StopPDP(CEHandle handle, CEString password);
 *
 * Stop the PDP. 
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * password (INPUT): password string to stop PDP
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEP_StopPDP(CEHandle handle, 
		       CEString password,
		       CEint32 timeout_in_milliseconds) 
{
  try {
    if(timeout_in_milliseconds == 0) {
      return CE_RESULT_TIMEDOUT;
    }

    if(!StopPDP_CheckInputs(handle, password)) {
      return CE_RESULT_INVALID_PARAMS;
    }

    CEString paddedPassword;

    // CE?  Glad you asked.  The CE tells the PC that we should check the password (it's stripped
    // off on the other side, naturally).  If the password doesn't start "CE" then it is assumed
    // to be valid without being checked (see CEP_StopPDPWithoutPassword)!
    //
    // Leaving aside the danger of such a backdoor, it might have been advisable to have different
    // "stop pdp" functions or, perhaps, an extra argument indicating whether or not the password
    // should be checked.  Note that every piece of new code that is written that checks against
    // the password will need to perform this same magic or else its password will be accepted
    // regardless of whether or not it is correct
    nlstring prefix(_T("CE"));
    prefix+=CEM_GetString(password);
    paddedPassword = CEM_AllocateString(prefix.c_str());

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%lu"), 
	      nlthread_selfID(), 
	      PEPMAN_GetUniqueRequestID());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    CEString userIDArg = CEM_AllocateString(handle->userID);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(&(handle->sessionID));
    args.push_back(paddedPassword);

    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CEP_StopPDP"), 
				     args, reqLen);
    if(!packed) {
      TRACE(1, 
	    _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
      CEM_FreeString(reqIDArg);
      CEM_FreeString(userIDArg);
      CEM_FreeString(paddedPassword);
      return CE_RESULT_GENERAL_FAILED;
    } 
    
    //Do RPC call
    nlstring reqFunc(100, ' ');
    CEResult_t reqResult;
    vector<void *> reqOut;
    reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
    CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
				     reqFunc, reqResult, reqOut, 
				     timeout_in_milliseconds);
    if(result != CE_RESULT_SUCCESS) {
      TRACE(1, _T("CEP_StopPDP: PEPMAN_RPCCall failed errno=%d\n"),
	    result);
      Marshal_PackFree(packed); 
      CEM_FreeString(reqIDArg);
      CEM_FreeString(userIDArg);
      CEM_FreeString(paddedPassword);
      return result;
    }

    //Cleaning up
    Marshal_PackFree(packed); 
    Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
    CEM_FreeString(reqIDArg);
    CEM_FreeString(userIDArg);
	CEM_FreeString(paddedPassword);
    
    //Return result
    return reqResult;    
  } catch (std::exception &e) {
    TRACE(0, _T("CEP_StopPDP failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CEP_StopPDP failed due to unknow exception\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEResult_t CEP_StopPDP(CEHandle handle, CEString password);
 *
 * Stop the PDP. 
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * password (INPUT): password string to stop PDP
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEP_StopPDPWithoutPassword(CEHandle handle, 
		       CEString password,
		       CEint32 timeout_in_milliseconds) 
{
  try {
    if(timeout_in_milliseconds == 0) {
      return CE_RESULT_TIMEDOUT;
    }

    if(!StopPDP_CheckInputs(handle, password)) {
      return CE_RESULT_INVALID_PARAMS;
    }

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%f"), nlthread_selfID(), 
	      NL_GetCurrentTimeInMillisec());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    CEString userIDArg = CEM_AllocateString(handle->userID);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(&(handle->sessionID));
    args.push_back(password);

    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CEP_StopPDP"), 
				     args, reqLen);
    if(!packed) {
      TRACE(1, 
	    _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
      CEM_FreeString(reqIDArg);
      CEM_FreeString(userIDArg);
      return CE_RESULT_GENERAL_FAILED;
    } 
    
    //Do RPC call
    nlstring reqFunc(100, ' ');
    CEResult_t reqResult;
    vector<void *> reqOut;
    reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
    CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
				     reqFunc, reqResult, reqOut, 
				     timeout_in_milliseconds);
    if(result != CE_RESULT_SUCCESS) {
      TRACE(1, _T("CEP_StopPDP: PEPMAN_RPCCall failed errno=%d\n"),
	    result);
      Marshal_PackFree(packed); 
      CEM_FreeString(reqIDArg);
      CEM_FreeString(userIDArg);
      return result;
    }

    //Cleaning up
    Marshal_PackFree(packed); 
    Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
    CEM_FreeString(reqIDArg);
    CEM_FreeString(userIDArg);
    
    //Return result
    return reqResult;    
  } catch (std::exception &e) {
    TRACE(0, _T("CEP_StopPDP failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CEP_StopPDP failed due to unknow exception\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
}

/* ------------------------------------------------------------------------
 * CEResult_t CEP_GetChallenge(CEHandle handle, CEString challenge)
 *
 * Get the PDP challenge. 
 * 
 * Arguments : 
 * handle (INPUT): Handle from the CONN_Initialize()
 * challenge (OUTPUT): returned challenge
 *
 * ------------------------------------------------------------------------
 */ 
CEResult_t CEP_GetChallenge(CEHandle handle, 
			    CEString challenge,
			    CEint32 timeout_in_milliseconds) 
{
  try {
    if(timeout_in_milliseconds == 0) {
      return CE_RESULT_TIMEDOUT;
    }

   if(!GetChallenge_CheckInputs(handle, challenge)) {
      return CE_RESULT_INVALID_PARAMS;
    }

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%f"), 
	      nlthread_selfID(), NL_GetCurrentTimeInMillisec());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    CEString userIDArg = CEM_AllocateString(handle->userID);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(userIDArg);

    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CEP_GetChallenge"), 
				     args, reqLen);
    if(!packed) {
      TRACE(1, 
	    _T("Function 'Marshal_PackReqFunc' failed to pack the request\n"));
      return CE_RESULT_INVALID_PARAMS;
    } 
    
    //Do RPC call
    nlstring reqFunc(100, ' ');
    CEResult_t reqResult;
    vector<void *> reqOut;
    reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
    CEResult_t result=PEPMAN_RPCCall(reqID, packed, reqLen, handle, 
				     reqFunc, reqResult, reqOut, 
				     timeout_in_milliseconds);
    if(result != CE_RESULT_SUCCESS) {
      TRACE(1, _T("CEP_GetChallenge: PEPMAN_RPCCall failed errno=%d\n"),
	    result);
      Marshal_PackFree(packed); 
      CEM_FreeString(reqIDArg);
      CEM_FreeString(userIDArg);
      return result;
    }

    //Assign the output
    challenge = new struct _CEString();
    challenge->length=((CEString)reqOut[1])->length; 
    challenge->buf = new nlchar[challenge->length+1];
    nlstrncpy_s(challenge->buf, challenge->length+1, ((CEString)reqOut[1])->buf, _TRUNCATE);

    //Cleaning up
    Marshal_PackFree(packed); 
    Marshal_UnPackFree(reqFunc.c_str(), reqOut, false); 
    CEM_FreeString(reqIDArg);
    CEM_FreeString(userIDArg);
    
    //Return result
    return reqResult;    
  } catch (std::exception &e) {
    TRACE(0, _T("CEP_GetChallenge failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CEP_GetChallenge failed due to unknow exception.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
}
