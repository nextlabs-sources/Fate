/*========================PEPMan_internal.h=================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/5/2007                                                        *
 * Note   : Some internal class and data structure of module PEP management *
 *==========================================================================*/
#ifndef __CE_CLIENT_PEPMAN_INTERNAL_H
#define __CE_CLIENT_PEPMAN_INTERNAL_H

#include <string>
#include <stdexcept>
#include <map>
#include <vector>
#include "nlstrings.h"
#include "CEsdk.h"
#include "nlthread.h"
#include "nltypes.h"

namespace PEPMAN {
using namespace std;
#define PEPMan_BUF_MAX 1024

/*==========================================================================*
 * Structure of Request                                                     *
 *==========================================================================*/
struct Request{
  enum STATUS {WAITING, RETURNED, DISCONNECTED};
  nlchar reqFuncName[PEPMan_BUF_MAX]; //the name of request function
  nlthread_t tid; //The request thread id
  
  /* The answer to request */
  CEResult_t retCode; // The return code
  vector<void *> retArgv; //the vector of function returned arguments
  STATUS status; //The status of this requests

  nlthread_cond_t cond; //the conidtion variable when the answer arrives
  nlthread_mutex_t mutex; //The mutext to protect answer buffer;

  CEHandle client; //A reference to client thread 

  //Constructor
  Request(){}

  //Destructor
  ~Request() { 
    nlthread_cond_destroy(&cond); 
    nlthread_mutex_destroy(&mutex);}
};
typedef struct Request Request;
/*==========================================================================*
 * Class of PEPManagement
 *==========================================================================*/
class PEPManagement {
 public:
  enum STATUS {INITIALIZING, INITIALIZED, RUNNING, DISCONNECTED, STOP};

 private:
  //The table of all the current requests from clients
  std::map<nlstring, Request *> requests; 

  //The name of the host
  //If it is NULL, it means local machine
  nlchar *pdpHost;

  //The mutext of shared members (i.e. table "requests",
  //"status", and "numClients")
  nlthread_mutex_t manMutex;

  //To generate unique request ID 
  unsigned long uniqueReqID;

  //The mutext on the unique request ID
  nlthread_mutex_t uniqueReqIDMutex;

  //The socket descriptor
  nlsocket connFd;

  //The receiving thread id
  nlthread_t receivingTid;

  //The number of clients. 
  //When the function "CECONN_Close" is called, if the value of 
  //"count" is 0, the socket connection should be closed and the
  //PEP backend receiving thread should exit.  
  int numClients;

  //The status of PEP management module
  STATUS status;

  //If this is true, the back-end receiving thread is joinable.
  //By default, it is false.
  bool bJoinable;

  //Set PDP server name
  void SetPDPHost(const nlchar *n);

  //A PEP called CECONN_Initialize RPC. The PEP didn't pick up the answer
  //either because of timeout or some other unexpected error. This function
  //will do a CECONN_Close RPC to free the memory on PDP side for this PEP.
  CEResult_t CleanInvalidConnRPC(CEHandle pdpSessionID);

 public:
  //PEPManagement constructor
  PEPManagement();

  //PEPManagement destructor
  ~PEPManagement(){
    if(pdpHost)
      delete [] pdpHost;
    nlthread_mutex_destroy(&manMutex);
    nlthread_mutex_destroy(&uniqueReqIDMutex);
  }

  /*Member access function*/
  nlsocket GetSocketConn() {return connFd;}
  STATUS GetStatus() {return status;}
  void SetStatus(STATUS s) { status = s; }

  /*For initialization, it needs to do
    1. Creat the backend receiving thread.
    2. Creat the socket connection. */
  //If the variable "bJoin" is true, the back-end receiving thread 
  //is going to be joinable not detached. 
  CEResult_t DoInit(CEString pdpHost, bool bJoin);

  //Send the request for the client
  //Block the client thread until the answer arrives or timeout
  CEResult_t RPCCall(nlstring &reqID,
		     char *reqStr, 
		     size_t reqLen, 
		     CEHandle c,
		     nlstring &funcName, 
		     CEResult_t &reqResult, 
		     std::vector<void *> &outputs,
		     int timeout_in_millisec);

  //This function is called by the backend receiving thread 
  //to receive the answer from socket
  CEResult_t ReceiveAnswer(char *revBuf);

  //This function will decrese the number of clients by 1.                
  //If the number of clients becomes 0, it will do the following,         
  //1. Clear the table "requests".                   
  //If the variable "bCloseSocket" is true, when the number of clients
  //becomes 0, it will 
  //1. Close the socket that cause the backend thread exit.                    
  CEResult_t DoClose(bool bCloseSocket);

  //This function 
  //1. Exit the backend receiving thread.                                 
  //2. Close the socket connection.                                       
  //3. Clear the table "requests".                                       
  //4. decrease the numClients to 0
  CEResult_t DoPreemptiveClose();

  //check if the initialization has been done successfully.
  //It returns CE_RESULT_SUCCESS, if PEPMan initalization has been
  //done successfully.   
  CEResult_t IsRunning();

  //Return the value of member "bJoinable"
  bool IsJoinable() { return bJoinable;}

  //Return the unique request ID
  unsigned long GetUniqueRequestID();
};
}
#endif /* PEP_admin_internal.h */
