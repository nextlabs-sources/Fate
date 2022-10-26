/*========================nlthread.cpp======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/15/2007                                                       *
 * Note   : For cross platform purpose, nlthread provides an OS abstration  *
 *          interface for thread managment and synchronization handling.    *
 *          This file includes the MS Windows version of implementation of  *
 *          exported nlthread APIs.                                         *
 *==========================================================================*/

//define for Minimum System required as Windows NT 4.0
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <process.h>
#include <windows.h>

#include "nlthread.h"

#define NLTRACELEVEL 1  

//Fetche the thread's own handle
nlthread_t nlthread_self()
{
  return GetCurrentThread();
}

//Fetche the thread's own identifier
unsigned long nlthread_selfID()
{
  return GetCurrentThreadId();
}

//Create a thread
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//On Windows, the thread created by this function has to call 
//"nlthread_end" to terminate itself, and the creator has to 
//call "nlthread_join" to close the thread handle. 
bool nlthread_create(nlthread_t *tid, nlthread_func func, void *arg)
{
  unsigned int thread_id = 0;
  *tid=(HANDLE)_beginthreadex(NULL, 0, func, arg,0, &thread_id);
  if(tid == NULL)
    return false;
  return true;
}

//Create a thread detaching from creator
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//On Linux, this function is same as "nlthread_create". To detach a 
//thread from its creator, it needs to call "nlthread_detach"
bool nlthread_detach_create(nlthread_t *tid, nlthread_detach_func f, void *arg)
{
  *tid=(HANDLE)_beginthread(f, 0, arg);
  if(tid == NULL)
    return false;
  return true;
}

//Detach the thread from its parent
//This is a NOP on MS Windows platform
bool nlthread_detach(nlthread_t tid) 
{
  return true;
}

//Wait for a given thread to terminate
bool nlthread_join(nlthread_t tid) 
{
  WaitForSingleObject(tid, INFINITE);
  CloseHandle(tid);
  return true;
}

//A thread teminate itself
void nlthread_end()
{
  _endthreadex(0);
}

//A detach thread teminates itself
//On Windows, a thread created by "nlthread_detach_thread" has to call 
//this function to close the thread handle. 
void nlthread_detach_end()
{
  _endthread();
}

bool nlthread_cs_init( nlthread_cs_t* cs )
{
  InitializeCriticalSection(cs);
  return true;
}/* nlthread_cs_init */

bool nlthread_cs_delete( nlthread_cs_t* cs )
{
  DeleteCriticalSection(cs);
  return true;
}/* nlthread_cs_delete */

bool nlthread_cs_enter( nlthread_cs_t* cs )
{
  EnterCriticalSection(cs);
  return true;
}/* nlthread_cs_enter */

bool nlthread_cs_leave( nlthread_cs_t* cs )
{
  LeaveCriticalSection(cs);
  return true;
}/* nlthread_cs_leave */

bool nlthread_cs_tryenter( nlthread_cs_t* cs )
{
  return TryEnterCriticalSection(cs);
}/* nlthread_cs_tryenter */

//Dynamically allocate a mutex variable
bool nlthread_mutex_init(nlthread_mutex_t *m)
{
  *m=CreateMutex(NULL, false, NULL);
  if(*m==NULL) return false;
  return true;
}

//Free a mutex variable that is dynamically allocated
bool nlthread_mutex_destroy(nlthread_mutex_t *m)
{
  return CloseHandle(*m);
}

//Lock a mutex
bool nlthread_mutex_lock(nlthread_mutex_t *m)
{
  DWORD res=WaitForSingleObject(*m, INFINITE);
  if(res != WAIT_OBJECT_0) return false;
  return true;
}

//Try lock a mutex
bool nlthread_mutex_trylock(nlthread_mutex_t *m)
{
  DWORD res=WaitForSingleObject(*m,0);
  if(res != WAIT_OBJECT_0) return false;
  return true;
}

//Unlock a mutex
bool nlthread_mutex_unlock(nlthread_mutex_t *m)
{
  return ReleaseMutex(*m);  
}

//Dynamically allocate a condition variable
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_init(nlthread_cond_t *c)
{
  *c=CreateEvent(NULL, false, false, NULL);
  if(*c==NULL) return false;
  return true;
}

//Free a condition variable that is dynamically allocated
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_destroy(nlthread_cond_t *c)
{
  return CloseHandle(*c);
}

//Wake up one thread waiting on the condition
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_signal(nlthread_cond_t *c)
{
  return SetEvent(*c);
}

//Wait for a condition to be true with timeout.
//m (input): the mutex that protects the condition. It the caller doesn't
//           own the mutex, this function will fail.
//c (input): the condition variable. On MS Windows, the condition variable
//           is an event
//t (input): specifies the timeout
//btimeout (output): if it is true, it means that the condition hasn't been 
//                   satisfied in the specified amount of time
//Return: return false if the caller doesn't own the mutex or timeout; 
//        otherwise, return true.
bool nlthread_cond_timedwait(nlthread_cond_t *c, nlthread_mutex_t *m, 
			     nlthread_timeout *t, bool *btimeout)
{
  *btimeout=false;

  DWORD ret1 = SignalObjectAndWait(*m,*c,*t,FALSE);
  
  DWORD ret2 = WaitForSingleObject (*m,INFINITE);

  if(ret1==WAIT_TIMEOUT)
    *btimeout = true;

  //return false if timeout or errors
  if(ret1 == WAIT_ABANDONED || ret2 == WAIT_ABANDONED || *btimeout)
    return false;

  return true;
  /*
  //This is just a temperal implementation. It is not atomic.
  if(ReleaseMutex(*m)) {
    DWORD res=WaitForSingleObject(*c, *t);
    if(res != WAIT_FAILED || res != WAIT_TIMEOUT) 
      res=WaitForSingleObject(*m, *t);

    if(res == WAIT_TIMEOUT)
      *btimeout=true;
  
    if(res == WAIT_FAILED) return false;
    return true;
  } 

  //The caller doesn't own the mutex 
  return false;*/
}

//Wait for a condition to be true.
//m (input): the mutex that protects the condition. It the caller doesn't
//           own the mutex, this function will fail.
//c (input): the condition variable. On MS Windows, the condition variable
//           is an event
//Return: return false if the caller doesn't own the mutex or timeout; 
//        otherwise, return true.
bool nlthread_cond_wait(nlthread_cond_t *c, nlthread_mutex_t *m)
{

  DWORD ret1 = SignalObjectAndWait(*m,*c,INFINITE,FALSE);

  DWORD ret2 = WaitForSingleObject (*m,INFINITE);

  //return false if timeout or errors
  if(ret1 == WAIT_ABANDONED || ret2 == WAIT_ABANDONED)
    return false;

  return true;
}


//To  obtain a timeout value in nlthrea_timout structure
//t (output): on Linux it is the returned timespec structure; on
//            MS Window, it is just the intergers of second and millisecond
//time_in_second: specify the timeout value in seconds
//time_in_millisec: specify the timeout value in milliseconds
void nlthread_maketimeout(nlthread_timeout *t, 
			  int time_in_second,
			  int time_in_millisec)
{
    // To avoid integer overflow (and to satisify an automated code checking tool)
    // we are capping the maximum timeout at one hour
    if (time_in_millisec < 0 || time_in_millisec >= 1000)
    {
        time_in_millisec = 0;
    }

    if (time_in_second < 0 || time_in_second >= 3600)
    {
        time_in_second = 3600;
    }

    *t=time_in_second * 1000 + time_in_millisec;
}

//initialize a semaphore with an initial value
void nlthread_sem_init(nlthread_sem_t *sem,int initial)
{
  *sem = CreateSemaphore(NULL,initial,10000,NULL);  //10000 indicates max value
}

//release a semaphore
bool nlthread_sem_post(nlthread_sem_t *sem)
{
  if(ReleaseSemaphore(*sem,1,NULL))   //1 indicates increase semaphore by 1
    return true;
  return false;
}

//wait on a semaphore
bool nlthread_sem_wait(nlthread_sem_t *sem)
{
  DWORD ret = WaitForSingleObject(*sem,INFINITE);

  if(ret == WAIT_ABANDONED)
    return false;

  return true;
}

//close a semaphore
//On Window and Linux, this function is NOP
bool nlthread_sem_close(nlthread_sem_t *sem)
{
  return true;
}
