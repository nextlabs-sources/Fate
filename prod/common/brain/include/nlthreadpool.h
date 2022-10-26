#ifndef __NL_THREAD_POOL
#define __NL_THREAD_POOL

#include "brain.h"
#include "nlthread.h"


typedef struct {
  int taskid;
  void* taskdata;
} task_t;

typedef void* (*NLTP_disposer)(task_t);

//Argument:
//bTraceStatus: if it is true, an array of boolean is going to created
//              for tracking each thread's status, idle or busy.
bool NLTP_init(_In_ int size, _In_ NLTP_disposer, _In_ bool bTrackStatus);
bool NLTP_doWork(_In_ task_t task);
void NLTP_Free();
//This function will wait until all threads in the pool are idle
//If the thread pool is initialized as not being tracked, this function
//will immediately return.
void NLTP_WaitThreadAllIdle();

#endif
