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
 *          This file includes the Linux version of implementation of       *
 *          exported nlthread APIs.                                         *
 *==========================================================================*/
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/resource.h>
#include "nlthread.h"

//Fetche the thread's own handle
nlthread_t nlthread_self()
{
  return pthread_self();
}

//Fetche the thread's own identifier
unsigned long nlthread_selfID()
{
  return pthread_self();
}

//Create a thread
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//The stack size of the new thread is only 16K bytes.
bool nlthread_create(nlthread_t *tid, nlthread_func func, void *arg)
{
  pthread_attr_t tattr;
  size_t stacksize;
  int res;
  /* Not setting stacksize 
   setting a thread stack size to minimum 16K bytes
  res=pthread_attr_init(&tattr);
  if(res != 0) {
    printf("can't initialize thread attribute due to: %s\n", strerror(res));
    return false;
  }    
  stacksize = (PTHREAD_STACK_MIN); //defualt stack size 16K bytes
  res = pthread_attr_setstacksize(&tattr, stacksize);  
  if(res != 0) {
    printf("can't set thread stack size due to: %s\n", strerror(res));
    pthread_attr_destroy(&tattr);
    return false;
    } 
    res=pthread_create(tid, &tattr, func, arg);*/
  res=pthread_create(tid, NULL, func, arg);
  /*struct rlimit rlim;
  long n = getrlimit(RLIMIT_STACK, &rlim);
  double h_lim = (double)rlim.rlim_max;
  double s_lim = (double)rlim.rlim_cur;
  printf("\nStacksize limits: hard: %10.1lf Kb Soft: %10.1lf Kb\n",
  h_lim/1024., s_lim/1024.);*/
  //pthread_attr_destroy(&tattr);
  if(res == 0)
    return true;
  printf("can't create thread due to: %s\n", strerror(res));
  return false;
}

//Create a thread detaching from creator
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//On Linux, this function is same as "nlthread_create". To detach a 
//thread from its creator, it needs to call "nlthread_detach"
bool nlthread_detach_create(nlthread_t *tid, nlthread_detach_func f, void *arg)
{
  return nlthread_create(tid, f, arg);
}

//Detach the thread from its parent
bool nlthread_detach(nlthread_t tid)
{
  return pthread_detach(tid);
}

//Wait for a given thread to terminate
bool nlthread_join(nlthread_t tid) 
{
    int res = 0;
    if (tid != 0) {
        void *status;
        int res=pthread_join(tid, &status);
    }
    return res == 0;
}

//A thread teminate itself
//On Linux, it is NOP.
void nlthread_end()
{
  return;
}

//A detach thread teminates itself
//On Linux, it is NOP.
void nlthread_detach_end()
{
  return;
}

/* Critical sections are mutex objects for Linux */

bool nlthread_cs_init( nlthread_cs_t* cs )
{
  pthread_mutexattr_t attr;

  if( pthread_mutexattr_init(&attr) != 0 )
  {
    return false;
  }

  /* Critical section must be recursive */
  if( pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE) != 0 )
  {
    pthread_mutexattr_destroy(&attr);
    return false;
  }

  int res = pthread_mutex_init(cs,&attr);

  bool status = false;
  if( res == 0 )
  {
    status = true;
  }

  pthread_mutexattr_destroy(&attr);

  return status;
}/* nlthread_cs_init */

bool nlthread_cs_delete( nlthread_cs_t* cs )
{
  return nlthread_mutex_destroy(cs);
}/* nlthread_cs_delete */

bool nlthread_cs_enter( nlthread_cs_t* cs )
{
  return nlthread_mutex_lock(cs);
}/* nlthread_cs_enter */

bool nlthread_cs_leave( nlthread_cs_t* cs )
{
  return nlthread_mutex_unlock(cs);
}/* nlthread_cs_leave */

bool nlthread_cs_tryenter( nlthread_cs_t* cs )
{
  return nlthread_mutex_unlock(cs);
}/* nlthread_cs_tryenter */

//Dynamically allocate a mutex variable
bool nlthread_mutex_init(nlthread_mutex_t *m)
{
  int res=pthread_mutex_init(m, NULL);
  if(res==0) return true;
  return false;
}

//Free a mutex variable that is dynamically allocated
bool nlthread_mutex_destroy(nlthread_mutex_t *m)
{
  int res=pthread_mutex_destroy(m);
  if(res == 0) return true;
  return false;
}

//Lock a mutex
bool nlthread_mutex_lock(nlthread_mutex_t *m)
{
  int res=pthread_mutex_lock(m);
  if(res==0) return true;
  return false;
}

//Try lock a mutex
bool nlthread_mutex_trylock( nlthread_mutex_t *m )
{
  int res = pthread_mutex_trylock(m);
  if( res == 0 )
    return true;
  return false;
}

//Unlock a mutex
bool nlthread_mutex_unlock(nlthread_mutex_t *m)
{
  int res=pthread_mutex_unlock(m);
  if(res == 0) return true;
  return false;
}


//Dynamically allocate a condition variable
bool nlthread_cond_init(nlthread_cond_t *c)
{
  int res=pthread_cond_init(c, NULL);
  if(res == 0) return true;
  return false;
}

//Free a condition variable that is dynamically allocated
bool nlthread_cond_destroy(nlthread_cond_t *c)
{
  int res=pthread_cond_destroy(c);
  if(res == 0) return true;
  return false;
}

//Wake up one thread waiting on the condition
bool nlthread_cond_signal(nlthread_cond_t *c)
{
  int res=pthread_cond_signal(c);
  if(res==0) return true;
  return false;
}

//Wait for a condition to be true with timeout.
//m (input): the mutex that protects the condition. It the caller doesn't
//           own the mutex, this function will fail.
//c (input): the condition variable.
//t (input): specifies the timeout
//btimeout (output): if it is true, it means that the condition hasn't been 
//                   satisfied in the specified amount of time
//Return: return false if the caller doesn't own the mutex or timeout; 
//        otherwise, return true.
bool nlthread_cond_timedwait(nlthread_cond_t *c, nlthread_mutex_t *m, 
			    nlthread_timeout *t, bool *btimeout)
{
  int res=pthread_cond_timedwait(c, m, t);
  if(res == ETIMEDOUT) {
    *btimeout=true;
    return false;
  }
  *btimeout = false;
  if(res == 0) return true;
  return false;  
}

//Wait for a condition to be true.
//m (input): the mutex that protects the condition. It the caller doesn't
//           own the mutex, this function will fail.
//c (input): the condition variable.
//Return: return false if the caller doesn't own the mutex or timeout; 
//        otherwise, return true.
bool nlthread_cond_wait(nlthread_cond_t *c, nlthread_mutex_t *m)
{
  int res=pthread_cond_wait(c, m);
  
  if(res == 0) return true;
    return false;
}

//To  obtain a timeout value in nlthrea_timout structure
//t (output): on Linux it is the returned timespec structure; on
//            MS Window, it is just the intergers of second and millisecond
//time_in_second: specify the timeout value in seconds
//time_in_millisec: specify the timeout value in milliseconds
void nlthread_maketimeout(nlthread_timeout *timeout, 
			  int time_in_second,
			  int time_in_millisec)
{
  struct timeval now;
  
  /*Get the current time */
  gettimeofday(&now, NULL);
  timeout->tv_sec = now.tv_sec;
  timeout->tv_nsec = now.tv_usec * 1000;
  
  /* add offset to get timeout value, "time_in_second" seconds */
  timeout->tv_sec += time_in_second;
  timeout->tv_nsec += time_in_millisec * 1000000;
}

//initialize a semaphore with an initial value
void nlthread_sem_init(nlthread_sem_t *sem,int initial)
{
  sem_init(sem,1,initial);    //1 indicates pshared
}

//release a semaphore
bool nlthread_sem_post(nlthread_sem_t *sem)
{
  if(sem_post(sem)==0)
    return true;
  return false;
}

//wait on a semaphore
bool nlthread_sem_wait(nlthread_sem_t *sem)
{
  if(sem_wait(sem)==0)
    return true;
  return false;
}

//close a semaphore
//On Window and Linux, this function is NOP
bool nlthread_sem_close(nlthread_sem_t *sem)
{
  return true;
}
