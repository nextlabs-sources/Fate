#include "brain.h"
#include "nlthreadpool.h"

void* disposer(task_t t)
{
  TRACE(1,_T("processing task %d: %s\n"),t.taskid,(char*)(t.taskdata));
  return NULL;
}

void testnlthreadpool()
{
  task_t t;
  nlchar *buf[100];
  NLTP_init(10,&disposer, false);
  NL_sleep(1000);
  for(int i=0;i<100;i++)
    {
      nlchar *tempbuf = (nlchar*)malloc(20);
	  if(NULL != tempbuf)
	  {
		  memset(tempbuf,0,20);
		  nlsprintf(tempbuf, 20, _T("%d"),i);
		  buf[i] = tempbuf;
		  t.taskid = i;
		  t.taskdata = (void*)buf[i];
		  NLTP_doWork(t);
	  }
    }
  NL_sleep(4000);
  NLTP_Free();
  //memory leak
}
