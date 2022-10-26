/*========================TransCtrl_private.h===============================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 5/30/2007                                                       *
 * Note   : internal class and data structure of module TransCtrl.          *
 *==========================================================================*/
#ifndef __CE_TRANSCTRL_PRIVATE_H
#define __CE_TRANSCTRL_PRIVATE_H

#include <queue>
#include "TransCtrl.h"

class TransCtrl {
public:
  //The queue that holds the data from kif and transport layer.
  std::queue<NL_TRANSCTRL_QUEUE_ITEM *> dataQ; 

  //The condition variable when the queue is not empty
  nlthread_cond_t  filledCond;

  //The mutex to protect dataQ
  nlthread_mutex_t dataQMutex;

  //Variables to stop TransCtrl
  nlthread_mutex_t  stopMutex; //mutex for notifying the stop of TransCtrl
  bool bStopped;    //stop flag 
  
  //Two sub listening threads
  nlthread_t socketListenTid;
  nlthread_t kifListenTid;

  TransCtrl() {
    nlthread_cond_init(&filledCond);
    nlthread_mutex_init(&dataQMutex);
    nlthread_mutex_init(&stopMutex);  
    bStopped=false; 
  }

  ~TransCtrl() {
    nlthread_cond_destroy(&filledCond);
    nlthread_mutex_destroy(&dataQMutex);
    nlthread_mutex_destroy(&stopMutex);
  }
};

#endif /* end of TRANSCTRL_private.h*/
