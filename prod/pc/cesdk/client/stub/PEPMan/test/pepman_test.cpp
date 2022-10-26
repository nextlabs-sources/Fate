
#include <time.h>
#include "brain.h"
#include "nlstrings.h"
#include "cetype.h"
#include "PEPMan.h"
#include "marshal.h"
#include "nlthread.h"
#include "transport.h"

#if defined (Linux)
#include "linux_win.h"
#endif

//Some globle variables scoped in this file
namespace {
enum {MAX_SUB_THREADS=500, MAX_REQUEST_PER_THREAD=2000,
      FLAG_DONE=1, FLAG_RUNNING=2, FLAG_JOINED=4};

struct child {
  int flag;
  nlthread_t tid;
  int index;
} childs[MAX_SUB_THREADS];

int numDone;
nlthread_mutex_t numDone_mutex;
nlthread_cond_t numDone_cond;

double dummyserver_time=0;
bool bServerReady=false;


extern "C" void *DummyServerThread(void *arg)
{
  nlthread_detach(nlthread_self());
  CEResult_t res=TRANSPORT_Serv_Initialize();
  int req_count=0;

  //Sleep 1 second
  NL_sleep(1*1000);
  bServerReady = true;

  TRANSPORT_QUEUE_ITEM q;
  if(res == CE_RESULT_SUCCESS) {
    while(res == CE_RESULT_SUCCESS) {
      res=TRANSPORT_Serv_GetNextRequest(&q);
      req_count++;  
      if(res == CE_RESULT_SUCCESS && q.buf) {
	vector<void *> argBuf;
	argBuf.reserve(RPC_MAX_NUM_ARGUMENTS);
	nlstring funcName(100u, ' ');
	nlstring reqID(512u, ' ');
	CEResult_t marshalRet=Marshal_UnPackReqFunc(q.buf, funcName, argBuf);
	if(marshalRet == CE_RESULT_SUCCESS) {
	  reqID = ((CEString)argBuf[0])->buf;
	  TRACE(4, 
	  _T("%d Request '%s' from '%s' expect request 'CELOGGING_SetNoiseLevel'\n"), 
		req_count, funcName.c_str(), 
		reqID.c_str());
	  if(*((CEint32 *)argBuf[1]) != 1) 
	    TRACE(0, _T("Request arg 1: %d expect 1"), 
		  *((CEint32 *)argBuf[1]));
	  if(*((CEint32 *)argBuf[2]) != 20) 
	    TRACE(0, _T("Request arg 2: %d expect 20"), 
		  *((CEint32 *)argBuf[1]));
	  Marshal_UnPackFree(funcName.c_str(), argBuf, true); 

	  /*Pack the reply*/
	  argBuf.clear();
	  size_t replyLen;
	  CEString reqIDStr=CEM_AllocateString(reqID.c_str()); 
	  argBuf.push_back(reqIDStr);
	  char *reply=Marshal_PackFuncReply(_T("CELOGGING_SetNoiseLevel"),
					    CE_RESULT_SUCCESS,
					    argBuf, replyLen);
	  if(reply) {
	    //Send out the reply
	    if(TRANSPORT_Sendn(q.sock, replyLen, reply) != CE_RESULT_SUCCESS)
	      TRACE(0,_T("TRANSPORT_Sendn failed\n"));
	    Marshal_PackFree(reply); 
	  } else 
	    TRACE(0, _T("Marshal_PackFuncReply failed\n"));
	  CEM_FreeString(reqIDStr);	  
	} else 
	  TRACE(0, _T("Marshal_UnPackReqFunc failed: %d\n"), marshalRet);
      } else if(res == CE_RESULT_SUCCESS && q.buf==NULL)
	TRACE(0, _T("TRANSPORT_Serv_GetNextRequest: return empty buf\n"));
      TRANSPORT_MemoryFree(q);
    }
  }
  nlthread_detach_end();
  return NULL;
}

extern "C" void *PEPTestThread(void *aptr)
{
  struct child *cptr=(struct child *)aptr;
  nlsocket socketFd;
  CEResult_t result=PEPMAN_Init(NULL, socketFd, false);
  CEHandle dummy=NULL;
  nlstring funcName(100u, ' ');
  CEResult_t reqResult;

  if(result == CE_RESULT_SUCCESS) {
    nlchar reqIDBuf[100];
    CEint32 session_id=1;
    CEint32 noise=20;
    size_t reqLen;
    for(int i=0; i<MAX_REQUEST_PER_THREAD; i++) {
#if defined (WIN32) || defined(_WIN64)
      nlsprintf(reqIDBuf, _countof(reqIDBuf), _T("%lu+%f"), 
		nlthread_selfID(), NL_GetCurrentTimeInMillisec());
#else
      nlsprintf(reqIDBuf, _T("%lu+%f"), 
		nlthread_selfID(), NL_GetCurrentTimeInMillisec());
#endif

      CEString reqIDStr=CEM_AllocateString(reqIDBuf); 
      vector<void *> argBuf;
      argBuf.reserve(RPC_MAX_NUM_ARGUMENTS);
      argBuf.push_back(reqIDStr);
      argBuf.push_back(&session_id);
      argBuf.push_back(&noise);
      char *packed=Marshal_PackReqFunc(_T("CELOGGING_SetNoiseLevel"), 
				       argBuf, reqLen);
      if(packed) {
	nlstring reqID(reqIDBuf);
	vector<void *> outputs;
	outputs.reserve(RPC_MAX_NUM_ARGUMENTS);
	TRACE(4, _T("Thread%d %s do rpc %d \n"), cptr->index, reqIDBuf,reqLen);
	result=PEPMAN_RPCCall(reqID, packed, 
			      reqLen, dummy, funcName, reqResult, outputs, 
			      5000);
	if(result == CE_RESULT_SUCCESS) {
	  TRACE(4, _T("Thread%d %s: RPC done successfully.\n"), 
		cptr->index, ((CEString)outputs[0])->buf);
	  if(reqResult != CE_RESULT_SUCCESS)
	    TRACE(4, _T("Thread%d: RPC doesn't return 'success'\n"), 
		  cptr->index);	    
	  Marshal_UnPackFree(_T("CELOGGING_SetNoiseLevel"), outputs, false); 
	} else
	  TRACE(6, _T("Thread%d: failed to PEPMAN_RPCCall %d\n"), 
		cptr->index, result);
	Marshal_PackFree(packed); 
	packed = NULL;
      } else
	TRACE(0, _T("Thread%d: Marshal_PackReqFunc failed\n"), cptr->index);
      CEM_FreeString(reqIDStr); 
      //if(i%1000 == 0)
      //TRACE(0, _T("Thread%d: request %d succeed\n"), cptr->index, i);

    }
    PEPMAN_Close(false);  
  } else 
    TRACE(1, _T("Failed%d to PEPMAN_Init: %d\n"), cptr->index, result); 
  
  nlthread_mutex_lock(&numDone_mutex);
  cptr->flag=FLAG_DONE;
  numDone++;  
  nlthread_cond_signal(&numDone_cond);
  nlthread_mutex_unlock(&numDone_mutex);
  nlthread_end();
  return cptr;
}
}
int main()
{
  int result;
  nlthread_t tid;
  int numLeftChilds;
  nlthread_timeout timeout;
  bool btimedout;
  
  //Create the backend receiving thread
  if(!nlthread_detach_create(&tid, 
			    (nlthread_detach_func)(&DummyServerThread), 
			     NULL)) {
    TRACE(0, _T("Can't create 'DummyServerThread'\n"));
    return 0;
  }
    
  while(!bServerReady);

  nlthread_mutex_init(&numDone_mutex);
  nlthread_cond_init(&numDone_cond);

  double start_time=NL_GetCurrentTimeInMillisec();

  for(int i=0; i<MAX_SUB_THREADS; i++) {
    result=nlthread_create(&tid, (nlthread_func)(&PEPTestThread), &childs[i]);
    if(!result) {
      TRACE(0, _T("Can't create no.%d thread\n"), i);
      exit(1);
    }
    childs[i].tid=tid;
    childs[i].index=i;
  }

  numLeftChilds=MAX_SUB_THREADS;

  while(numLeftChilds > 0) {
    TRACE(4, _T("Waiting for any one out of %d child threads done.\n"), 
	  numLeftChilds);
    for(int i=0; i<MAX_SUB_THREADS; i++) {
      if(childs[i].flag & FLAG_DONE)
	continue;
      if(childs[i].flag & FLAG_JOINED)
	continue;
    }
    nlthread_maketimeout(&timeout, 60, 0);
    nlthread_mutex_lock(&numDone_mutex);
    if(numDone == 0) 
      nlthread_cond_timedwait(&numDone_cond, &numDone_mutex, &timeout, 
			      &btimedout);
  
    for(int i=0; i<MAX_SUB_THREADS; i++) {
      if(childs[i].flag & FLAG_DONE) {
	nlthread_join(childs[i].tid);
      
	childs[i].flag=FLAG_JOINED;
	numDone--;
	numLeftChilds--;
	TRACE(4, _T("No.%d child thread %lu done\n"), i, childs[i].tid);
      } 
    }
    nlthread_mutex_unlock(&numDone_mutex);
  }
  nlthread_mutex_destroy(&numDone_mutex);
  nlthread_cond_destroy(&numDone_cond);
  double t2=NL_GetCurrentTimeInMillisec();
  double total_exec_time=t2-start_time;
  TRACE(0, _T("Execution time %f seconds\n "), 
	total_exec_time/1000.0);
  return 0;
}
