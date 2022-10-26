/*========================TransCtrl.cpp=====================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 05/29/2007                                                      *
 * Note   : The APIs of transport control module.                           *
 *==========================================================================*/
#include "nlthread.h"
#include "transport.h"
#include "brain.h"
#include "CEsdk.h"
#define  NL_KIF_USER
#include "cekif.h"
#include "cekif_user.h"
#include "TransCtrl.h"
#include "TransCtrl_private.h"


using namespace std;

namespace {

TransCtrl transCtrl;

/*==========================================================================*
 * The backend thread to listen the request sent over the socket.           *
 *==========================================================================*/
extern "C" 
void *SocketListenThread(void *arg)
{
  TransCtrl *tc=(TransCtrl *)arg;
  TRANSPORT_QUEUE_ITEM *titem;
  NL_TRANSCTRL_QUEUE_ITEM *qitem;
 
  while(1) {
    //!!!remember to free this memory   
    titem = (TRANSPORT_QUEUE_ITEM*) calloc(1, sizeof(TRANSPORT_QUEUE_ITEM));
 
    //Check if the PDP is asked to stop. If yes, ingore this
    //request and exit from this thread
    nlthread_mutex_lock(&(transCtrl.stopMutex));
    if(transCtrl.bStopped) {
      free(titem);
      nlthread_mutex_unlock(&(transCtrl.stopMutex));	  
      break;
    }
    nlthread_mutex_unlock(&(transCtrl.stopMutex));	  

    //get request
    TRANSPORT_Serv_GetNextRequest(titem);
 
    //Check if the PDP is asked to stop. If yes, ingore this
    //request and exit from this thread
    nlthread_mutex_lock(&(transCtrl.stopMutex));
    if(transCtrl.bStopped) {
      TRANSPORT_MemoryFree(*titem);
      free(titem);
      nlthread_mutex_unlock(&(transCtrl.stopMutex));	  
      break;
    }
    nlthread_mutex_unlock(&(transCtrl.stopMutex));	  


    //!!!remember to free this memory   
    qitem = (NL_TRANSCTRL_QUEUE_ITEM*)malloc(sizeof(NL_TRANSCTRL_QUEUE_ITEM));
    if(qitem) {
      qitem->item = titem;
      qitem->source = NL_TRANSCTRL_QUEUE_ITEM::SOCKET;

      TRACE(0, _T("got a request over Socket\n"));
      //Add the request to dataQ and wake up the TRANSCTRL_GetNextRequest
      nlthread_mutex_lock(&(tc->dataQMutex));
      tc->dataQ.push(qitem);
      nlthread_cond_signal(&(tc->filledCond));    
      nlthread_mutex_unlock(&(tc->dataQMutex));
    }
  }
  
  nlthread_end();  
  TRACE(0, _T("End socket listen thread\n"));
  return NULL;
}

/*==========================================================================*
 * The backend thread to listen the request posted via KIF.                 *
 *==========================================================================*/
extern "C" 
void *KifListenThread(void *arg)
{
  TRACE(0, _T("IN kiflistenthread\n"));

  TransCtrl *tc=(TransCtrl *)arg;
  NL_KIF_QUEUE_ITEM *kitem;
  NL_TRANSCTRL_QUEUE_ITEM *qitem;
 
  while(1) {
    //!!!remember to free this memory   
    kitem = (NL_KIF_QUEUE_ITEM*) calloc(1, sizeof(NL_KIF_QUEUE_ITEM));

    if(kitem) {
      //Check if the PDP is asked to stop. If yes, ingore this
      //request and exit from this thread
      nlthread_mutex_lock(&(transCtrl.stopMutex));
      if(transCtrl.bStopped) {
	free(kitem);
	nlthread_mutex_unlock(&(transCtrl.stopMutex));	  
	break;
      }
      nlthread_mutex_unlock(&(transCtrl.stopMutex));	  

      //get request
      NL_KIF_GetNextKernelRequests(kitem->index, kitem->req);

      //Check if the PDP is asked to stop. If yes, ingore this
      //request and exit from this thread
      nlthread_mutex_lock(&(transCtrl.stopMutex));
      if(transCtrl.bStopped) {
	NL_KIF_RequestFree(kitem->req);
	free(kitem);
	nlthread_mutex_unlock(&(transCtrl.stopMutex));	  
	break;
      }
      nlthread_mutex_unlock(&(transCtrl.stopMutex));	  

      TRACE(0, _T("got a request over kif\n"));

      //!!!remember to free this memory   
      qitem =(NL_TRANSCTRL_QUEUE_ITEM*)malloc(sizeof(NL_TRANSCTRL_QUEUE_ITEM));
      if(qitem) {
	qitem->item = kitem;
	qitem->source = NL_TRANSCTRL_QUEUE_ITEM::KIF;

	//Add the request to dataQ and wake up the TRANSCTRL_GetNextRequest
	nlthread_mutex_lock(&(tc->dataQMutex));
	tc->dataQ.push(qitem);
	nlthread_cond_signal(&(tc->filledCond));    
	nlthread_mutex_unlock(&(tc->dataQMutex));
      }
    }
  }

  nlthread_end();  
  TRACE(0, _T("End kif listen thread\n"));
  return NULL;
}
}

/* =======================NL_TRANSCTRL_Serv_Initialize====================*
 * Initialize transport control module.                                   *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
CEResult_t NL_TRANSCTRL_Serv_Initialize()
{
  try {
    bool bresult;
    CEResult_t result;

    //initialize the dataQ
    while(!transCtrl.dataQ.empty())
      transCtrl.dataQ.pop();

    //initialize the transport layer
    result=TRANSPORT_Serv_Initialize();
    if(result != CE_RESULT_SUCCESS) 
      return result;

    //Initialize the KIF layer
    result=NL_KIF_Probe(_T("."));
    if(result != CE_RESULT_SUCCESS)
      return result;
    result=NL_KIF_Initialize();
    if(result != CE_RESULT_SUCCESS)
      return result;

    //spawn a thread to listen incoming requests over socket
    bresult=nlthread_create(&(transCtrl.socketListenTid),
			    (nlthread_func)(&SocketListenThread),
			    &transCtrl);
    if(!bresult) {
      TRACE(0, _T("Failed to create socket listening thread.\n"));
      return CE_RESULT_CONN_FAILED;
    }

    //spawn a thread to listen incoming requests over kif
    bresult=nlthread_create(&(transCtrl.kifListenTid),
				  (nlthread_func)(&KifListenThread),
				  &transCtrl);
    if(!bresult) {
      TRACE(0, _T("Failed to create kif listening thread.\n"));
      return CE_RESULT_CONN_FAILED;
    }
  } catch (std::exception &e) {
    TRACE(0, _T("NL_TRANSCTRL_Serv_Initialize failed due to '%s'\n"), 
	  e.what());
    return CE_RESULT_CONN_FAILED;
  } catch (...) {
    TRACE(0,_T("NL_TRANSCTRL_Serv_Initialize failed due to unknown reason\n"));
    return CE_RESULT_CONN_FAILED;
  }

  return CE_RESULT_SUCCESS;
}
/* =======================NL_TRANSCTRL_Serv_GetNextRequest================*
 * Get the next request from dataQ.                                       *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
CEResult_t NL_TRANSCTRL_Serv_GetNextRequest(NL_TRANSCTRL_QUEUE_ITEM **req)
{
  nlthread_mutex_lock(&transCtrl.dataQMutex);
  while(transCtrl.dataQ.empty())
    nlthread_cond_wait(&transCtrl.filledCond,&transCtrl.dataQMutex); 
  *req=transCtrl.dataQ.front();
  transCtrl.dataQ.pop();
  nlthread_mutex_unlock(&transCtrl.dataQMutex);
  return CE_RESULT_SUCCESS;
}

/* =======================NL_TRANSCTRL_MemoryFree=========================*
 * Free the memory allocated in this module                               *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
void NL_TRANSCTRL_MemoryFree(NL_TRANSCTRL_QUEUE_ITEM *req)
{
  if(req->source == NL_TRANSCTRL_QUEUE_ITEM::SOCKET) {
    TRANSPORT_QUEUE_ITEM *q=(TRANSPORT_QUEUE_ITEM *)req->item;
    TRANSPORT_MemoryFree(*q);
  } else {
    NL_KIF_QUEUE_ITEM *q=(NL_KIF_QUEUE_ITEM *)req->item;
    NL_KIF_RequestFree(q->req);
  }
  free(req->item);
  free(req);
}
/* =======================NL_TRANSCTRL_SHUTDOWN===========================*
 * Shutdown the two threads KifListenThread & SocketListenThread, of      *
 * TransCtrl.                                                             *
 *                                                                        *
 * Parameters:                                                            *
 * =======================================================================*/
void NL_TRANSCTRL_Shutdown()
{
  CEResult_t result;
  
  //Stop TransCtrl
  nlthread_mutex_lock(&(transCtrl.stopMutex));
  transCtrl.bStopped=true;
  result=NL_TRANSPORT_Shutdown();
  if(result != CE_RESULT_SUCCESS) 
    TRACE(0, _T("Failed to shutdown transport due to %d\n"), result);
  result=NL_KIF_Shutdown();
  if(result != CE_RESULT_SUCCESS) 
    TRACE(0, _T("Failed to shutdown kif due to %d\n"), result);
  nlthread_mutex_unlock(&(transCtrl.stopMutex));	  


  //Wait for the two listening sub-threads to stop
  TRACE(0, _T("Waiting for socket listen thread\n"));
  nlthread_join(transCtrl.socketListenTid);
  TRACE(0, _T("Waiting for kif listen thread\n"));
  nlthread_join(transCtrl.kifListenTid);  
}
