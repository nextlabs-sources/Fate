/*========================PEPMan.cpp========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/8/2007                                                        *
 * Note   : Implementations of PEMMan class and external APIs.              *
 *==========================================================================*/
#define NLMODULE   PEPMAN
#define NLTRACELEVEL 3  

#include "brain.h"
#include "nlstrfuncs.h"
#include "cetype.h"
#include "PEPMan_internal.h"
#include "marshal.h"
#include "transport.h"

#if defined (Linux)
#include "linux_win.h"
#endif

using namespace PEPMAN;
using namespace std;

/*==========================================================================*
 * Interanl Global variables and functions scoped in this file.             *
 *==========================================================================*/
namespace {
//The only instance of PEP management 
PEPManagement pepMan;
}


/*==========================================================================*
 * PEPManagement backend receiving thread.                                  *
 * It does                                                                  *
 * 1. Receive the answers to the client threads' requests (stored in table  *
 *    "requests") from the socket                                           *
 * 2. Unpack the answers and find the corresponding client thread id        *
 * 3. Wake up the waiting client thead                                      *
 *==========================================================================*/
extern "C" 
void *PEPManReceivingThread(void *arg)
{
    size_t revLen;
  char *revBuf=NULL;
    
  pepMan.SetStatus(PEPManagement::RUNNING);

  if(!pepMan.IsJoinable())
    nlthread_detach(nlthread_self());

  while(1) {
    if(TRANSPORT_GetRecvLength(pepMan.GetSocketConn(), 
			       revLen)==CE_RESULT_SUCCESS) {
      revBuf = new char[revLen];
      if(revBuf) {
	if(TRANSPORT_Recvn(pepMan.GetSocketConn(), revLen, 
			   revBuf)==CE_RESULT_SUCCESS) {
	  if(pepMan.ReceiveAnswer(revBuf) != CE_RESULT_SUCCESS) {

	  } 
	  delete [] revBuf;
	} else {

	  delete [] revBuf;
	  break;
	}
      } else {

      }
    } else {

      break;
    }
  }

  pepMan.DoPreemptiveClose();

  if(pepMan.IsJoinable())
    nlthread_end();
  else 
    nlthread_detach_end();
  return NULL;
}

/*==========================================================================*
 * Member functions of class PEPManagement                                  *
 *==========================================================================*/
//PEPManagement constructor
PEPManagement::PEPManagement():connFd(0), receivingTid(0), numClients(0), 
			       status(PEPManagement::STOP), bJoinable(false)
{
  nlthread_mutex_init(&manMutex);
  nlthread_mutex_init(&uniqueReqIDMutex);
  pdpHost=NULL;
  uniqueReqID=0;
}  

//Set PDP server name
void PEPManagement::SetPDPHost(const nlchar *n)
{
  if(pdpHost) {
    delete [] pdpHost;
    pdpHost=NULL;
  }

  if(n==NULL)
    return;

  size_t len=nlstrlen(n);
  pdpHost = new nlchar [len+1];
  nlstrncpy_s(pdpHost, len+1, n, _TRUNCATE);
}

//A PEP called CECONN_Initialize RPC. The PEP didn't pick up the answer
//either because of timeout or some other unexpected error. This function
//will do a CECONN_Close RPC to free the memory on PDP side for this PEP.
CEResult_t PEPManagement::CleanInvalidConnRPC(CEHandle pdpSessionID)
{
  std::vector<void *> args;
  nlchar reqIDStr[100];
  args.reserve(RPC_MAX_NUM_ARGUMENTS);
#if defined (WIN32) || defined(_WIN64)
  nlsprintf(reqIDStr, _countof(reqIDStr), _T("%lu+%f"), 
	    nlthread_selfID(), NL_GetCurrentTimeInMillisec());
#else
  nlsprintf(reqIDStr, _T("%lu+%f"), 
	    nlthread_selfID(), NL_GetCurrentTimeInMillisec());
#endif
  CEString reqIDArg = CEM_AllocateString(reqIDStr);
  nlstring reqID(reqIDStr);
  size_t reqLen;

  //Construct input arguments vector
  args.push_back(reqIDArg);
  args.push_back(&pdpSessionID);

  //Marshal request 
  char *packed=Marshal_PackReqFunc(_T("CECONN_Close"), 
				   args, reqLen);
  if(!packed) {
    return CE_RESULT_INVALID_PARAMS;
  } 
    
  //Do RPC call
  nlstring reqFunc(100u,' ');
  CEResult_t reqResult=CE_RESULT_SUCCESS;
  vector<void *> reqOut;
  reqOut.reserve(RPC_MAX_NUM_ARGUMENTS);
  //We just want to send out the close this session request. So set timeout 
  //to 0
  CEResult_t result=RPCCall(reqID, packed, reqLen, NULL, 
			    reqFunc, reqResult, reqOut, 
			    0); 
  if(result != CE_RESULT_SUCCESS && result != CE_RESULT_TIMEDOUT) {
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
//For initialization, it needs to do 
//1. Creating the backend receiving thread.
//2. Establishing the socket connection. 
//If the initialization has been done already, increase the count number  
//of connected clients.                                      
//If the variable "bJoin" is true, the back-end receiving thread 
//is going to be joinable not detached. 
CEResult_t PEPManagement::DoInit(CEString pdp, bool bJoin)
{
  nlthread_mutex_lock(&manMutex);
  STATUS statusOnEntry = status;
  if (status == PEPManagement::INITIALIZING) {
      // Recursive call to DoInit.  We hooked some function that is called in DoInit and this triggered
      // another enforcer which is trying to connect to the PDP.  We should have a better error code here.
    nlthread_mutex_unlock(&manMutex);
    return CE_RESULT_RECURSIVE_INVOCATION;
  }
  if(status == PEPManagement::INITIALIZED ||
     status == PEPManagement::RUNNING) {
    //The initialization has been done.
    //Check if setting is consistant
    if(!(pdpHost==NULL && pdp==NULL)) {
      if(pdpHost==NULL && pdp) {
	nlthread_mutex_unlock(&manMutex);
	return CE_RESULT_CONN_FAILED;      
      } else if ((pdpHost && pdp==NULL) ||
		 (pdpHost && pdp && nlstrcmp(pdpHost, pdp->buf))) {
	nlthread_mutex_unlock(&manMutex);
	return CE_RESULT_CONN_FAILED;      
      }
    }
    ++numClients;
    nlthread_mutex_unlock(&manMutex);
    return CE_RESULT_SUCCESS;    
  } else {
    status = PEPManagement::INITIALIZING;
    numClients=1;
  }
  //Establishing the socket connection
  CEResult_t res;
  if(pdp && pdp->buf) {
    SetPDPHost(pdp->buf);
    char hostNameBuf[PEPMan_BUF_MAX];
    if(!nlstrtoascii(pdp->buf, hostNameBuf, (unsigned int)PEPMan_BUF_MAX)) {
      status = statusOnEntry;
      nlthread_mutex_unlock(&manMutex);
      return CE_RESULT_GENERAL_FAILED;
    }
    res=TRANSPORT_Cli_Initialize(connFd, hostNameBuf);
  } else 
    res=TRANSPORT_Cli_Initialize(connFd, NULL);
  if(res != CE_RESULT_SUCCESS) {
    status = statusOnEntry;
    SetPDPHost(NULL);
    nlthread_mutex_unlock(&manMutex);
    return res;
  }

  //Update the status
  status = PEPManagement::INITIALIZED;
  
  //Create the backend receiving thread
  bool resT;
  bJoinable=bJoin;

  if(bJoinable) {
    resT=nlthread_create(&receivingTid, 
			 (nlthread_func)(&PEPManReceivingThread), 
			NULL);
  }
  else {
    resT=nlthread_detach_create(&receivingTid, 
				(nlthread_detach_func)(&PEPManReceivingThread),
				NULL);
  }

  if(!resT) {
    if(connFd != 0) {
      TRANSPORT_Close(connFd);
      connFd=0;
    }
    status = PEPManagement::STOP;
    nlthread_mutex_unlock(&manMutex);
    bJoinable=false;
    return CE_RESULT_CONN_FAILED;
  }
  else
  {	  //wait backend receiving thread running 2016/12/05
	  int nWaitTimes = 0;
	  while ((status!=PEPManagement::RUNNING) && (nWaitTimes<3))
	  {
		  nWaitTimes++;
		  Sleep(50);
	  }
  }
  
  nlthread_mutex_unlock(&manMutex);
  return CE_RESULT_SUCCESS;
}

//Send the request for the client
//Block the client thread until the answer arrives or timeout
CEResult_t PEPManagement::RPCCall(nlstring &reqID,
				  char *reqStr, 
				  size_t reqLen, 
				  CEHandle ch,
				  nlstring &funcName, 
				  CEResult_t &reqResult, 
				  std::vector<void *> &outputs,
				  int timeout_in_millisec)
{
  Request req;
  CEResult_t result=CE_RESULT_SUCCESS;
  nlthread_timeout timeout;
  nlthread_cond_init(&req.cond);
  nlthread_mutex_init(&req.mutex);

  nlthread_mutex_lock(&manMutex);

  if(status != PEPManagement::RUNNING) {
    // Either not intialized or perhaps the receive thread hasn't got rolling yet.
    // Either way we can't make a call
    nlthread_mutex_unlock(&manMutex);
    return CE_RESULT_THREAD_NOT_INITIALIZED;
  }
  /*Initialize "req" */
  req.tid=nlthread_self();
  req.status=Request::WAITING;
  req.client=ch;
  req.retArgv.reserve(RPC_MAX_NUM_ARGUMENTS);

  /*Send client's request and add it to the table "requests"*/
  result=TRANSPORT_Sendn(connFd, reqLen, reqStr);
  if(result != CE_RESULT_SUCCESS) {
    nlthread_mutex_unlock(&manMutex); //release the lock to table "requests"
    return result;
  }
  if(requests.find(reqID) != requests.end()) {
  
  }
  requests[reqID]=&req;

  nlthread_mutex_unlock(&manMutex); //release the lock to table "requests"
  
  //Waiting for the answer
  nlthread_mutex_lock(&req.mutex);
  bool btimedout=false;
  while(req.status != Request::RETURNED) {
    if(timeout_in_millisec == CE_INFINITE) {
      nlthread_cond_wait(&req.cond, &req.mutex);
    } else {
      nlthread_maketimeout(&timeout, 
			   timeout_in_millisec/1000, 
			   timeout_in_millisec%1000);
      nlthread_cond_timedwait(&req.cond, &req.mutex, &timeout, &btimedout);
      if(btimedout) {
	result=CE_RESULT_TIMEDOUT; 
	if(!funcName.empty() && 
 	   nlstrncmp(funcName.c_str(),_T("CECONN_Initialize"), 17)==0
 	   && numClients > 0) {
 	  numClients--;
 	}	
	break;
      }
    }

    if(req.status == Request::DISCONNECTED) {
      result=CE_RESULT_CONN_FAILED;      
      if(!funcName.empty() && 
	 nlstrncmp(funcName.c_str(),_T("CECONN_Initialize"), 17)==0 &&
 	 numClients > 0) {
	numClients--;
       }      
      break;
    }
  }

  //Receive the result
  if(result == CE_RESULT_SUCCESS) {
    funcName=req.reqFuncName;
    reqResult=req.retCode;
    outputs=req.retArgv;
  }
  nlthread_mutex_unlock(&req.mutex);
  
  //Remove req from table 'requests' if it exists in the table
  if(result != CE_RESULT_SUCCESS) {
    std::map<nlstring, Request *>::iterator cit;
    nlthread_mutex_lock(&manMutex);
    cit=requests.find(reqID);
    if(cit != requests.end()) {
      nlthread_mutex_lock(&req.mutex);
      if(req.status == Request::RETURNED) {
	//The answer is missed. We re-pick it up.
	funcName=req.reqFuncName;
	reqResult=req.retCode;
	outputs=req.retArgv;
	result = CE_RESULT_SUCCESS;
      }
      requests.erase(cit);
      nlthread_mutex_unlock(&req.mutex);
    }
    nlthread_mutex_unlock(&manMutex);    
  }
  return result;
}

//check if the initialization has been done successfully.
//It returns CE_RESULT_SUCCESS, if PEPMan initalization has been
//done successfully.  
CEResult_t PEPManagement::IsRunning()
{
  nlthread_mutex_lock(&manMutex);
  if(status != PEPManagement::INITIALIZED &&
     status != PEPManagement::RUNNING) {
    //Not initialized. Return error
    nlthread_mutex_unlock(&manMutex);
    return CE_RESULT_CONN_FAILED;
  }
  nlthread_mutex_unlock(&manMutex);
  return CE_RESULT_SUCCESS;
}

//This function is called by the backend receiving thread 
//to receive the answer from socket
CEResult_t PEPManagement::ReceiveAnswer(char *revBuf)
{
  CEResult_t result = CE_RESULT_SUCCESS;
  vector<void *> answerArgs;
  nlstring answerFunc(100u, ' ');
  CEResult_t answerResult;
  nlchar *reqid;

  // Normally we would use RPC_MAX_NUM_ARGUMENTS, which is 20.  This
  // is sufficient for all regular SDK calls, but might not be for
  // Service calls, which can return arbitrary amounts of junk.
  // Unfortunatly, thanks to the Windows memory/dll model we will
  // break if we have to resize the vector in marshal, so we have to
  // make it "big enough".  There are other solutions (dynamic crt,
  // query "how many arguments", callback to add elements), but this
  // was chosen for reasons of speed and simplicity.
  answerArgs.reserve(1024);

  result=Marshal_UnPackFuncReply(revBuf, answerFunc, answerResult, 
				 answerArgs);
  if(result != CE_RESULT_SUCCESS) {
    return result;
  }
  if(answerArgs.size() <= 0) {
    return CE_RESULT_GENERAL_FAILED;
  } 
  //Get request client thread id
  if (Marsh_GetFuncSignature(answerFunc.c_str())->outputHasFixedFormat()) {
    reqid=(((CEString)answerArgs[0])->buf);  
  } else {
    // First item is the fmt string
    reqid=(((CEString)answerArgs[1])->buf);  
  }

  nlthread_mutex_lock(&manMutex);
  
  //Get request client's request data
  Request *creq=NULL;
  std::map<nlstring, Request *>::iterator cit;
  cit=requests.find(reqid);
  if(cit != requests.end()) {
    creq=cit->second;
    requests.erase(cit);
  }
  if(creq==NULL) {
    if(nlstrncmp(answerFunc.c_str(),_T("CECONN_Initialize"), 17)==0) {
      //Connection initialization failed, we need to tell PDP to 
      //free the handle for this PEP. 
      CEHandle pdpSessionID= *((CEHandle *)answerArgs[1]);
      Marshal_UnPackFree(answerFunc.c_str(), answerArgs,false);
      nlthread_mutex_unlock(&manMutex);
      CleanInvalidConnRPC(pdpSessionID);
    } else {
      Marshal_UnPackFree(answerFunc.c_str(), answerArgs,false);
      nlthread_mutex_unlock(&manMutex);
    }   
    return CE_RESULT_GENERAL_FAILED;
  }

  //Wake up the waiting client thread
  nlthread_mutex_lock(&creq->mutex);
  creq->retArgv=answerArgs;
  nlstrncpy_s(creq->reqFuncName, _countof(creq->reqFuncName), answerFunc.c_str(), _TRUNCATE); 
  creq->retCode=answerResult;
  creq->status=Request::RETURNED;
  nlthread_cond_signal(&creq->cond);    
  nlthread_mutex_unlock(&creq->mutex);
  nlthread_mutex_unlock(&manMutex);
  return CE_RESULT_SUCCESS;
}

//This function will decrese the number of clients by 1.                
//If the number of clients becomes 0, it will do the following,         
//1. Clear the table "requests".                   
//If the variable "bCloseSocket" is true, when the number of clients
//becomes 0, it will 
//1. Close the socket that cause the backend thread exit.                    
CEResult_t PEPManagement::DoClose(bool bCloseSocket)
{
  nlthread_mutex_lock(&manMutex);

  if(status != PEPManagement::INITIALIZED &&
     status != PEPManagement::RUNNING) {
    //Nothing running
    nlthread_mutex_unlock(&manMutex);
    if(bJoinable) {
      nlthread_join(receivingTid);
    }
    return CE_RESULT_SUCCESS;
  }

  if(numClients > 1) {
    //There are still other clients. Thus, we only deduct the number
    //of clients
    --numClients;   
    nlthread_mutex_unlock(&manMutex);
    if(bCloseSocket) {
      return CE_RESULT_SHUTDOWN_FAILED;
    } else
      return CE_RESULT_SUCCESS;
  } 
  numClients = 0;

  //Not to close the socket connection for the next possible 
  //immediate connection request unless the variable "bCloseSocket"
  //is true.
  if(bCloseSocket && connFd!=0) {
    TRANSPORT_Close(connFd);
    connFd = 0;
  }

  //Clear the table "requests"
  requests.clear();

  nlthread_mutex_unlock(&manMutex);  
  
  //If the back-end receving thread is joinable, we need to wait for
  //its end. 
  if(bJoinable) {
    nlthread_join(receivingTid);
  }
  return CE_RESULT_SUCCESS;
}

//This function 
//1. Exit the backend receiving thread.                                 
//2. Close the socket connection.                                       
CEResult_t PEPManagement::DoPreemptiveClose()
{
  nlthread_mutex_lock(&manMutex);
  if(status != PEPManagement::INITIALIZED &&
     status != PEPManagement::RUNNING) {
    //Nothing running
    nlthread_mutex_unlock(&manMutex);
    return CE_RESULT_SUCCESS;
  }

  //Close the socket connection
  if(connFd != 0) {
    TRANSPORT_Close(connFd);
    connFd = 0;  
  }

  //The connection down, waking up all those waiting threads
  Request *creq=NULL;
  std::map<nlstring, Request *>::iterator cit;
  for(cit=requests.begin(); cit != requests.end(); cit++) {
    creq=cit->second;
    creq->status=Request::DISCONNECTED;
    nlthread_mutex_lock(&creq->mutex);
    nlthread_cond_signal(&creq->cond);    
    nlthread_mutex_unlock(&creq->mutex);    
  }

  status = PEPManagement::STOP;
  nlthread_mutex_unlock(&manMutex);
  return CE_RESULT_SUCCESS;
}

//Return the unique request ID
unsigned long PEPManagement::GetUniqueRequestID() 
{
  int counter;

  nlthread_mutex_lock(&uniqueReqIDMutex);
  counter=uniqueReqID++;
  nlthread_mutex_unlock(&uniqueReqIDMutex);
  return counter;
}
/*==========================================================================*
 * Exported APIs
 *==========================================================================*/
/* =======================PEPMAN_RPCCall==================================*
 * This function send the request for the client. It will Block the client*
 * thread until the answer arrives or timeout.                            *
 *                                                                        *
 * Parameters:                                                            *
 * reqID  (input): ID string (in the format of "<tid>+time") of request   *
 * reqStr (input): client's request.                                      *
 * reqLen (input): the length of request                                  *
 * c (input): client's handler                                            *
 * funcName (output): the name of RPC                                     *
 * reqResult (output): RPC returned code                                  *
 * outputs (output): RPC returned parameters                              *
 *                                                                        *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if the RPC succeeds.                   *
 * =======================================================================*/
CEResult_t PEPMAN_RPCCall(nlstring &reqID, 
			  char *reqStr, 
			  size_t reqLen, 
			  CEHandle c,
			  nlstring &funcName, 
			  CEResult_t &reqResult, 
			  std::vector<void *> &outputs,
			  int timeout_in_millisec) 
{
  try {
      return pepMan.RPCCall(reqID, reqStr, reqLen, c, funcName, 
			  reqResult, outputs, timeout_in_millisec);
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}
/* =======================PEPMAN_Init=====================================*
 * This function does intializations. It includes,                        *
 * 1. Open log file                                                       *
 * 2. Creat the backend receiving thread.                                 *
 * 3. Creat the socket connection.                                        *
 * If the initialization has been done already, increat the count number  *
 * of connected clients.                                                  *
 *                                                                        *
 * Parameters:                                                            *
 * pdpHost (input): the PDP host name                                     *
 * socketFd (output): returned socket descriptor.                         *
 * bJoin (input): if the value is true, the back-end thread will be waited*
 *                for its termination.                                    *
 *                                                                        *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if it succeeds.                        *
 * =======================================================================*/
CEResult_t PEPMAN_Init(CEString pdpHost, nlsocket &socketFd, bool bJoin)
{
  try {
    /* If the initialization hasn't been done, return successfully at here */
    CEResult_t res=pepMan.DoInit(pdpHost, bJoin);
    if(res == CE_RESULT_SUCCESS) {
      socketFd=pepMan.GetSocketConn();
      return CE_RESULT_SUCCESS; 
    }
    return res;
  } catch (...) {
    return CE_RESULT_CONN_FAILED;
  }
}
/* =======================PEPMAN_Close====================================*
 * This function will decrese the number of clients by 1.                 *
 * If the number of clients becomes 0, it will do the following,          *
 * 1. Exit the backend receiving thread.                                  *
 * 2. Close the socket connection.                                        *
 * 3. Clear the table "requests".                                         *
 *                                                                        *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if it succeeds.                        *
 * =======================================================================*/
CEResult_t PEPMAN_Close(bool bCloseSocket )
{
  try {
    CEResult_t res=pepMan.DoClose(bCloseSocket);
    return res;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}
/* =======================PEPMAN_IsRunning================================*
 * This function will check if the initialization has been done           *
 * successfully.                                                          *
 *                                                                        *
 * Return:                                                                *
 *   It returns CE_RESULT_SUCCESS, if PEPMan initalization has been       *
 *   done successfully.                                                   *
 * =======================================================================*/
CEResult_t PEPMAN_IsRunning()
{
  try {
    CEResult_t res=pepMan.IsRunning();
    return res;
  } catch (...) {
    return CE_RESULT_GENERAL_FAILED;
  }
}
/* =======================PEPMAN_GetUniqueRequestID=======================*
 * This function will return the unique request ID within the process.    *
 *                                                                        *
 * Return:                                                                *
 *   Return the unique request ID within the process                      *
 * =======================================================================*/
unsigned long PEPMAN_GetUniqueRequestID()
{
  try {
    return pepMan.GetUniqueRequestID();
  } catch (...) {
    return 0;
  }
}
