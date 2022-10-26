/*============================nlca_threadpool.cpp===========================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou Nolan                                                *
 * Date   : 10/28/2008                                                      *
 * Note   : NLCA threa pool implementation.                                 *
 *==========================================================================*/
#include <objbase.h>
#include "nlca_threadpool.hpp"
#include "brain.h"
#include "celog.h"

using namespace NLCA;

namespace{
CELog *calog;
}

ThreadPool::ThreadPool()
{

}

ThreadPool::~ThreadPool()
{

}

bool ThreadPool::Initialize(int numThreads)
{
  HANDLE pThread=NULL;
  NLCAThread* caThread=NULL;

  //Initialize members
  InitializeCriticalSection(&_cs);
  _taskCountSema=CreateSemaphore(NULL,           // default security attributes
				 0,              // initial count
				  MAX_SEMA_COUNT,  // maximum count
				  NULL);          // unnamed semaphore
  if(_taskCountSema == NULL) {
#if 0
    calog->Log(CELOG_ERR, _T("Failed to create task count semaphonre: %d\n"),
	      WSAGetLastError());
#endif
    return false;
  }
  _abortSignal=CreateEventA(NULL,TRUE,FALSE,NULL);
  if(_abortSignal==NULL) {
#if 0
    calog->Log(CELOG_ERR, _T("Failed to create abort signal: %d\n"),
	      WSAGetLastError());
#endif
    return false;
  }
  
  //Construct pool
  _poolSize= numThreads;
  _pool.reserve(numThreads*2);
  for(int i=0; i<(int)_poolSize;i++) {
    caThread= new NLCAThread;
    if(caThread) {
      unsigned int thread_id = 0;
      caThread->SetMyThreadPool(this);
      pThread=(HANDLE)_beginthreadex(NULL,0,ThreadPool::WorkerProc,
				     caThread,0,&thread_id);
      if(pThread) {
	caThread->SetThread(pThread);
	_pool.push_back(caThread);
      } else {
#if 0
	calog->Log(CELOG_ERR, _T("Failed to start nlca worker thread: %d\n"),
		 WSAGetLastError());
#endif
	delete caThread;
	return false;
      }
    }
  }
  return true;
}

unsigned int __stdcall ThreadPool::WorkerProc(void *pParam)
{
  NLCAThread *pThis = reinterpret_cast<NLCAThread *>(pParam);

  // Initialize COM support.  Required for IFilter.
  HRESULT hr = CoInitializeEx(0,COINIT_MULTITHREADED);
  if( !SUCCEEDED(hr) )
  {
    _endthreadex(1);
  }

  if ( pThis && pThis->_myThreadPool ) {
    //Initialize wait objects
    HANDLE waitObjs[2] = { 0 };
    DWORD numWaitObjs = _countof(waitObjs);
    DWORD waitResult;
    waitObjs[0] = pThis->_myThreadPool->_abortSignal;
    waitObjs[1] = pThis->_myThreadPool->_taskCountSema;

    while(1) {
      //calog->Log(CELOG_DEBUG, _T("Threadpool worker wait\n"));
      waitResult=WaitForMultipleObjects(numWaitObjs, waitObjs, FALSE, INFINITE);

      //calog->Log(CELOG_DEBUG, _T("Threadpool worker wakeup\n"));
      if(waitResult == WAIT_OBJECT_0) {
	//calog->Log(CELOG_DEBUG, _T("Threadpool worker abort\n")); 
	break;
      } else if (waitResult == WAIT_OBJECT_0+1) {
#if 0
	calog->Log(CELOG_DEBUG, 
		  _T("Threadpool worker (0x%x): get a task\n"), pThis->_t); 
#endif
	Client *c=pThis->_myThreadPool->FetchTask(pThis);
	if(c) {
	  //execute the task
	  c->task->Run(c);
	  //End==execute the task

	  //Remove task from thread pool.
	  //Need mutex to exclusivly remove it (see cancel task)
	  EnterCriticalSection(&(pThis->_myThreadPool->_cs));
	  EnterCriticalSection(&c->_cs);
	  //Note: helding two mutex (in-order): 1. threadpool's 2. client's
	  //First, need to clean up client event
	  if(c->event && c->task) {
	    c->task->DeleteTaskEvent(c->event);
	    c->event=NULL;
	  }
	  //2, need to clean up client's task
	  if(c->task) {
	    delete c->task;
	    c->task=NULL;
	  }
	  //3, unassign this worker thread's client
	  pThis->_client=NULL;
	  LeaveCriticalSection(&c->_cs);
	  //Free the memory of this client
	  delete c;
	  LeaveCriticalSection(&(pThis->_myThreadPool->_cs));
	}
      }
      //calog->Log(CELOG_DEBUG, _T("Threadpool worker wait for next command\n"));     
    }
  }
  CoUninitialize(); // teardown of COM
  _endthreadex(0);
  return NULL;
}

bool ThreadPool::Shutdown()
{
  EnterCriticalSection(&_cs);
  //We first remove all pending tasks
  //Since we are going to exit, we don't don't bother to free
  //the memory.
  _tasks.clear();

  //Cancel all running tasks
  std::vector<NLCAThread *>::iterator it=_pool.begin();
  std::vector<NLCAThread *>::iterator eit=_pool.end();
  for(; it!=eit; ++it) {
    if((*it)->_client && (*it)->_client->task)
      (*it)->_client->task->Cancel((*it)->_client);
  }  
  LeaveCriticalSection(&_cs);

  //Signal all the threads
  SetEvent(_abortSignal);

  //Check if the worker thread are terminated
  int trial=3;
  DWORD waitResult;
  HANDLE *aThread=new HANDLE[_poolSize];
  for(int i=0; i<(int)_poolSize; i++) 
    aThread[i]=(_pool[i])->_t;
  while(trial>0) {
    // Wait for all threads to terminate
    waitResult=WaitForMultipleObjects(_poolSize, aThread, TRUE, 60000);
    if(waitResult != WAIT_TIMEOUT) {
      break;
    }
    trial--;
    Sleep(5000);
  }

  if(trial==0)
  {
    //calog->Log(CELOG_ERR, _T("Couldn't shutdown threadpool. Force abort!\n"));
  }

  //clean up 
  for(int i=0; i<(int)_poolSize; i++) 
    delete _pool[i];
  _pool.clear();
  DeleteCriticalSection(&_cs);
  CloseHandle(_abortSignal);
  CloseHandle(_taskCountSema);
  delete [] aThread;
  return true;  
}

void ThreadPool::AddTask(Client *c) 
{
  EnterCriticalSection(&_cs);
  if(c) {
    Client *c1=new Client;
    //copy the client and add to the tasks list
    if(c1) {
      //TBD: Client::event should not be typeless. So we can construct
      //Client::event instance. Currently, it is null when the task
      //is added. 
      c1->task=(c->task)?(c->task)->CloneTask():NULL;
      c1->s=c->s;
      c1->bAbort=c->bAbort;
      _tasks.insert(_tasks.begin(),c1);
      //Signal thread pool
      if (!ReleaseSemaphore(_taskCountSema,
			    1,            // increase count by one
			    NULL) ) {     // not interested in previous count
#if 0
	calog->Log(CELOG_ERR, _T("ReleaseSemaphore error: %d\n"), 
		   GetLastError());
#endif
      }
    }
  }
  LeaveCriticalSection(&_cs);
}

Client *ThreadPool::FetchTask(NLCAThread *pWorker) 
{
  Client *c=NULL;
  EnterCriticalSection(&_cs);
  while(_tasks.size() > 0) {
    c=_tasks.back();
    _tasks.pop_back();
    pWorker->_client=c;
    //calog->Log(CELOG_DEBUG, _T("Threadpool fetch a task 0x%x\n"), c);
    //todo: 
    //check the client is still valid
    //if not, fetch the next one
    break;      
  }
  LeaveCriticalSection(&_cs);
  return c;
}

bool ThreadPool::CancelTask(Client *c) 
{
  bool bSucceed=false;

  EnterCriticalSection(&_cs);
  std::vector<Client *>::iterator it=_tasks.begin();
  std::vector<Client *>::iterator eit=_tasks.end();
  
  //Check if in the pending queue
  for(; it != eit; ++it) {
    if((*it)->IsSameClient(*c)) {
      delete (*it); //free the memory of this client
      _tasks.erase(it);
      bSucceed=true;
#if 0
      calog->Log(CELOG_DEBUG, 
		 _T("Cancel task (socket 0x%x) when it is still pending\n"),
		 c->s);
#endif
      break;
    }
  }

  if(!bSucceed) {
    //Not in the pending queue
    //Check if it is being executed in the pool
    std::vector<NLCAThread *>::iterator cit=_pool.begin();
    std::vector<NLCAThread *>::iterator ceit=_pool.end();
    for(; cit!=ceit; ++cit) {
      if((*cit)->_client && (*cit)->_client->IsSameClient(*c)) {
	(*cit)->_client->task->Cancel((*cit)->_client);
	bSucceed=true;
#if 0
	calog->Log(CELOG_DEBUG, 
		   _T("Cancel running task (socket 0x%x)\n"),
		   (*cit)->_client->s);
#endif
	break;
      }
    }
  }
  LeaveCriticalSection(&_cs);
  if(!bSucceed) {
#if 0
    calog->Log(CELOG_DEBUG, 
	       _T("Cancel task(client 0x%x) failed: might be done already\n"),
	       c);
#endif
  } 
  return bSucceed;
}

void SetNLCAThreadPoolLog(CELog *lg)
{
  calog=lg;
}

NLCA::ThreadPool *NLCA_ThreadPool_Initialize(int numThreads)
{
  ThreadPool *tp=new ThreadPool;
  if(tp) {
    if(tp->Initialize(numThreads))
      return tp;
    delete tp;
  }
  return NULL;
}

bool NLCA_ThreadPool_Shutdown(NLCA::ThreadPool *tp)
{
  if(tp) {
    if(tp->Shutdown()) {
      delete tp;
      return true;
    }
    delete tp;
  }     
      
  return false;
}

void NLCA_ThreadPool_AddTask(NLCA::ThreadPool *tp, Client *c)
{
  if(tp && c)
    tp->AddTask(c);
}

bool NLCA_ThreadPool_CancelTask(NLCA::ThreadPool *tp, Client *c)
{
  if(tp && c)
    return tp->CancelTask(c);
  return true;
}

Client *NLCA_InitializeAClient()
{
  Client* lc = new Client();
  return lc;
}

void NLCA_FreeAClient(Client *c)
{
  if(c)
    delete c;
}
