#include <queue>
#include "nlthreadpool.h"

using namespace std;

namespace {
int nltp_poolsize = 0;   //size of the thread pool
nlthread_mutex_t  nltp_qmutex;
nlthread_cond_t   nltp_qcond; 
queue<task_t>  nltp_taskqueue;   //thread queue
NLTP_disposer  nltp_disposer;

nlthread_mutex_t  nltp_statusq_mutex;
bool *nltp_statusq=NULL; //array of thread status, busy->true

//now, hard-coded the worker thread logic here
void* nltp_worker(void* arg)
{
  nlthread_detach(nlthread_self());

  size_t thread_index=(size_t)arg;

  task_t t;
  while(1) {
    //wait for the new task coming in
    nlthread_mutex_lock(&nltp_qmutex);
    while(nltp_taskqueue.size()==0)
      nlthread_cond_wait(&nltp_qcond,&nltp_qmutex);
 
    //get the task at the front of the queue
    t = nltp_taskqueue.front();
    nltp_taskqueue.pop();
    if(nltp_statusq) {
      //update the status to busy
      nlthread_mutex_lock(&nltp_statusq_mutex);
      nltp_statusq[thread_index]=true;
      nlthread_mutex_unlock(&nltp_statusq_mutex);
    }
    nlthread_mutex_unlock(&nltp_qmutex);

    //debug
    TRACE(1,_T("thread(%lu) got a new task:%d\n"),nlthread_self(),t.taskid);

    //process the task
    nltp_disposer(t);

    if(nltp_statusq) {
      //update the status to idle
      nlthread_mutex_lock(&nltp_statusq_mutex);
      nltp_statusq[thread_index]=false;
      nlthread_mutex_unlock(&nltp_statusq_mutex);
    }
  }
  
  nlthread_detach_end();
  return NULL;
}
}

//Argument:
//bTraceStatus: if it is true, an array of boolean is going to created
//              for tracking each thread's status, idle or busy.
bool NLTP_init(int size,NLTP_disposer disposer_thread, bool bTrackStatus)
{
  if (size <= 0 || size > MAXIMUM_WAIT_OBJECTS) {
      TRACE(1,_T("NLTP_init called with illegal thread pool size.\n"));
    return false;
  }

  nlthread_t  tid;
  nltp_poolsize = size;
  nltp_disposer = disposer_thread;

  if(bTrackStatus)
    nltp_statusq= new bool[(size_t) nltp_poolsize];
  
  nlthread_mutex_init(&nltp_qmutex);
  nlthread_mutex_init(&nltp_statusq_mutex);
  nlthread_cond_init(&nltp_qcond);

  for(size_t i=0;i<(size_t)nltp_poolsize;i++) {
    if(bTrackStatus)
      nltp_statusq[i]=false;
    if(!nlthread_detach_create(&tid,
			       (nlthread_detach_func)(&nltp_worker),
			       (void *)i)) {
      TRACE(1,_T("Can't create worker thread for thread pool.\n"));
      if(nltp_statusq)
	delete [] nltp_statusq;
      return false;
    }
  }

  return true;
}

bool NLTP_doWork(task_t task)
{
  nlthread_mutex_lock(&nltp_qmutex);
  nltp_taskqueue.push(task);
  //signal the workers 
  nlthread_cond_signal(&nltp_qcond);
  nlthread_mutex_unlock(&nltp_qmutex);
  
  return true;
}

void NLTP_Free()
{
  if(nltp_statusq)
    delete [] nltp_statusq;
  nlthread_mutex_destroy(&nltp_qmutex);
  nlthread_mutex_destroy(&nltp_statusq_mutex);
  nlthread_cond_destroy(&nltp_qcond);  
}

//This function will wait until all threads in the pool are idle
//If the thread pool is initialized as not being tracked, this function
//will immediately return.
void NLTP_WaitThreadAllIdle()
{
  if(!nltp_statusq)
    return;

  bool bReturn=true;
  while(1) {
    nlthread_mutex_lock(&nltp_statusq_mutex);
    for(int i=0;i<nltp_poolsize;i++) {
      if(nltp_statusq[i]) {
	TRACE(0, _T("No.%d thread is still busy\n"), i);
	bReturn=false;
	break;
      }
    }  
    nlthread_mutex_unlock(&nltp_statusq_mutex);
    if(bReturn)
      break;
    bReturn=true;
    NL_sleep(5000);
  }
}
