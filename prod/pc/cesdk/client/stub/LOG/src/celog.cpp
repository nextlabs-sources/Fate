/*============================celog.cpp=====================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 3/25/2008                                                       *
 * Note   : Implementations of SDK CELOGGING_XXX APIs.                      *
 *==========================================================================*/
#define NLMODULE   CELOG
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
enum {CELOG_STRING_LENGTH_MAX=1024};

CEResult_t CSCINVOKE_CheckInputs(CEHandle handle, 
				 CEString logIdentifier,
				 CEString assistantName,
				 nlchar **attributesStrs,
				 CEint32 numAttributes)
{
  if(handle == NULL) {
    TRACE(0, 
  _T("The 'handle' passed to 'CSCINVOKE_CELOGGING_LogObligationData' is NULL.\n"));
    return CE_RESULT_NULL_CEHANDLE;
  }

  if(!(logIdentifier && logIdentifier->length!=0 && logIdentifier->buf)) {
    TRACE(0, 
  _T("The 'logIdentifier' passed to 'CSCINVOKE_CELOGGING_LogObligationData' is empty.\n"));
    return CE_RESULT_EMPTY_SOURCE;
  }
  
  if(!(assistantName && assistantName->length!=0 && assistantName->buf)) {
    TRACE(0, 
  _T("The 'assistantName' passed to 'CSCINVOKE_CELOGGING_LogObligationData' is empty.\n"));
    return CE_RESULT_EMPTY_SOURCE;
  }

  //Check attributes
  for(int i=0; i < numAttributes; i++) {
    if(attributesStrs[i]==NULL) {
      TRACE(0, 
	    _T("The %dth string of 'attributesStrs' passed to 'CSCINVOKE_CELOGGING_LogObligationData' is empty.\n"), i);
      return CE_RESULT_EMPTY_ATTR_KEY;
    }
  }
  return CE_RESULT_SUCCESS;
}

/*! CELOGGING_LogAssistantData
 *
 * \brief This assistant logging obligation. This function will be called by the Policy Assistant (or by multiple Policy Assistants).
 * 
 * \param logIdentifier (in): Taken from the obligation information.  Note that this is actually a long integer, 
 * \param asistantName: The name of the assistant (e.g. "CE Encryption Assistant"
 * \param assistantOption: Any options or arguments given to the policy assistant
 * \param assistantDescription: A long description of the assistant (necessary?)
 * \param assistantUserActions: A description of the actions the user took (e.g. "Encrypted using 128bit Rijndael" or "User cancelled action"
 *
 * \param optAttributes (optional): Additional data.  These are unstructured key/value pairs representing any additional information that this particular Policy Assistant would like presented in the log (e.g. "Encryption Time = 3.1s")
 *
 * \return Result of logging.
 *
 * \sa CELOGGING_LogAssistantData
 */ 
CEResult_t
CELOGGING_LogAssistantData(CEHandle handle,
                           CEString logIdentifier,
                           CEString assistantName,
                           CEString assistantOptions,
                           CEString assistantDescription,
                           CEString assistantUserActions,
                           CEAttributes *optAttributes)
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
    args.push_back(logIdentifier);
    args.push_back(assistantName);
    args.push_back(assistantOptions);
    args.push_back(assistantDescription);
    args.push_back(assistantUserActions);
    args.push_back(optAttributes);
    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CELOG_LogAssistantData"),
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
      TRACE(1, 
	    _T("CELOGGING_LogAssistantData: PEPMAN_RPCCall failed errno=%d\n"),
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
    TRACE(0, _T("CELOGGING_LogAssistantData failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CELOGGING_LogAssistantData failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

}

}
/*! CELOGGING_LogDecision
 *
 * \brief This, in combination with LOGDECISION custom obligation, provides
 *        a way to log user decision. 
 * 
 * \param handle (in)          connection handle from the CONN_initialize API
 * \param cookie (in)          data returned through LOGDECISION obligation
 * \param userResponse (in)    CEAllow if the user chose to continue with the action,
 *                             CEDeny otherwise
 * \param optAttributes (in)   any additional attributes that should be added to the
 *                             log entry
 * \return Result of logging decision.
 *
 * \sa CELOGGING_LogDecision
 */ 
CEResult_t 
CELOGGING_LogDecision(CEHandle handle, 
		      CEString cookie, 
		      CEResponse_t userResponse, 
		      CEAttributes * optAttributes)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_NULL_CEHANDLE;  

    std::vector<void *> args;
    nlchar reqIDStr[100];
    args.reserve(RPC_MAX_NUM_ARGUMENTS);
    nlsprintf(reqIDStr, _countof(reqIDStr),  _T("%lu+%lu"), 
	      nlthread_selfID(), 
	      PEPMAN_GetUniqueRequestID());
    CEString reqIDArg = CEM_AllocateString(reqIDStr);
    nlstring reqID(reqIDStr);
    size_t reqLen;

    //Construct input arguments vector
    args.push_back(reqIDArg);
    args.push_back(&(handle->sessionID));
    args.push_back(cookie);
    args.push_back(&userResponse);
    args.push_back(optAttributes);
    //Marshal request 
    char *packed=Marshal_PackReqFunc(_T("CELOG_LogDecision"),
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
      TRACE(1, _T("CELOGGING_LogDecision: PEPMAN_RPCCall failed errno=%d\n"),
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
    TRACE(0, _T("CELOGGING_LogDecision failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CELOGGING_LogDecision failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

}

/*! CELOGGING_LogObligationData
 *
 * \brief This assistant logging obligation. This function will be called by the Policy Assistant (or by multiple Policy Assistants).
 * 
 * \param logIdentifier (in): Taken from the obligation information.  Note that this is actually a long integer, 
 * \param obligationName: The name of the obligation (e.g. "CE Encryption Assistant"
 * \param attributes (in): These are unstructured key/value pairs representing information that this particular Policy Assistant would like presented in the log. Currently, only the first three attributes will be assigned the fields in the log.
 *
 * \return Result of logging.
 *
 * \sa CELOGGING_LogObligationData
 */ 
CEResult_t
CELOGGING_LogObligationData(CEHandle handle,
                           CEString logIdentifier,
                           CEString assistantName,
                           CEAttributes *attributes)
{
  CEResult_t result;

  try {
    if(handle == NULL) 
      return CE_RESULT_NULL_CEHANDLE; 

    CEString aOption = NULL;
    CEString aDes = NULL;
    CEString aAct = NULL;
    
    if(attributes && attributes->count >= 1){     
      aOption = attributes->attrs[0].value;
      if(attributes->count >= 2) {
	aDes = attributes->attrs[1].value;
	if(attributes->count >= 3)
	  aAct= attributes->attrs[2].value;
      }	
    }
    result=CELOGGING_LogAssistantData(handle,
				      logIdentifier,
				      assistantName,
				      aOption, //assistantOptions
				      aDes, //assistantDescription
				      aAct, //assistantUserActions
				      NULL); //optAttributes

 
    //NOTE: Don't understand the logic??!! Ask Scott & Alan :-)

  } catch (std::exception &e) {
    TRACE(0, _T("CELOGGING_LogObligationData failed due to '%s'\n"), e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {
    TRACE(0, _T("CELOGGING_LogObligationData failed due to unknown reason\n"));
    return CE_RESULT_GENERAL_FAILED;
  }
  return result;      

}
/* ------------------------------------------------------------------------
 * CSCINVOKE_CELOGGING_LogObligationData
 *
 * This function is the wrapper of CELOGGING_LogObligationData in C/C++ for
 * C# one.
 * 
 * Arguments : ptr(INPUT): pointer to the array of strings to be freed.
 *             numAttributes: the number of strings in array attributesStrs
 * ------------------------------------------------------------------------
 */ 
CEResult_t CSCINVOKE_CELOGGING_LogObligationData(CEHandle handle,
						 CEString logIdentifier,
						 CEString assistantName,
						 nlchar **attributesStrs,
						 CEint32 numAttributes)
{
  try{
    CEResult_t result=CE_RESULT_SUCCESS;
    CEAttributes attributes;
    
    //Checking inputs
    result=CSCINVOKE_CheckInputs(handle, logIdentifier, assistantName,
				 attributesStrs, numAttributes);

    //Construct attributes
    if(numAttributes==0 || attributesStrs==NULL) {
      attributes.attrs = NULL;
      attributes.count = 0;      
    } else {
      attributes.attrs = new CEAttribute[numAttributes/2];
      attributes.count = numAttributes/2;
      for(int i=0, j=0; i<numAttributes; i++, j++) {
	CEString key = new struct _CEString();
	key->length = nlstrlen(attributesStrs[i]);
	key->buf = new nlchar[key->length+1];
	nlstrncpy_s(key->buf, key->length+1, attributesStrs[i], _TRUNCATE);
	attributes.attrs[j].key=key;

	i++;

	CEString value = new struct _CEString();
	value->length = nlstrlen(attributesStrs[i]);
	value->buf = new nlchar[value->length+1];
	nlstrncpy_s(value->buf, value->length+1, attributesStrs[i],
		    _TRUNCATE);
	attributes.attrs[j].value=value; 
      }     
    }

    //Call the C/C++ one
    result=CELOGGING_LogObligationData(handle,
				       logIdentifier,
				       assistantName,
				       &attributes);
    //Clean up
    //Destruct attributes
    for(int i=0; i<attributes.count; i++) {
      delete attributes.attrs[i].key;
      delete attributes.attrs[i].value;
    }
    delete [] attributes.attrs;

    //return
    return result;
  } catch (std::exception &e) {
    TRACE(0, _T("CELOGGING_LogObligationData failed due to '%s'\n"), 
	  e.what());
    return CE_RESULT_GENERAL_FAILED;
  } catch (...) {    
    TRACE(0, 
	  _T("CELOGGING_LogObligationData failed due to unknow exception.\n"));
    return CE_RESULT_GENERAL_FAILED;
  }

}
