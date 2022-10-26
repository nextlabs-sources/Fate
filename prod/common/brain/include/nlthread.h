/*========================nlthread.h========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Jungle., *
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/12/2007                                                       *
 * Note   : For cross platform purpose, nlthread provides an OS abstration  *
 *          interface for thread managment and synchronization handling.    *
 *          This file includes the nlthread data structure definitions and  *
 *          the declarations of exported APIs.                              *
 *==========================================================================*/

#ifndef __CE_NLTHREAD_H
#define __CE_NLTHREAD_H

#if defined (Linux) || defined (Darwin)
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
typedef pthread_t nlthread_t;
typedef pthread_mutex_t nlthread_cs_t;    /* CS is pthread_mutex */
typedef pthread_mutex_t nlthread_mutex_t;
typedef pthread_cond_t nlthread_cond_t;
typedef struct timespec nlthread_timeout;
typedef void *(*nlthread_func)(void *);
typedef void *(*nlthread_detach_func)(void *);
#endif //End #ifdef Linux || Darwin

#if defined (Darwin)
typedef sem_t*  nlthread_sem_t;
#endif

#if defined (Linux) 
typedef sem_t  nlthread_sem_t;
#endif

#if defined (WIN32) || defined (_WIN64)
#include <windows.h>
#include <winbase.h>
typedef HANDLE nlthread_t;
typedef CRITICAL_SECTION nlthread_cs_t;
typedef HANDLE nlthread_mutex_t;
typedef HANDLE nlthread_cond_t;
typedef int nlthread_timeout;
typedef unsigned int (__stdcall *nlthread_func)(void *);
typedef void (__cdecl *nlthread_detach_func)(void *);
typedef HANDLE nlthread_sem_t;
#endif //End #ifdef WIN32

//Exported thread APIs
//Fetche the thread's own handle
_Check_return_ _Ret_ nlthread_t nlthread_self();

//Fetche the thread's own identifier
_Check_return_ unsigned long nlthread_selfID();

//Create a thread
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//On Windows, the thread created by this function has to call 
//"nlthread_end" to terminate itself, and the creator has to 
//call "nlthread_join" to close the thread handle. 
bool nlthread_create(_Out_ nlthread_t *tid, _In_ nlthread_func f, _In_opt_ void *arg);

//Create a thread detaching from creator
//tid (output): returned thread identifier
//func (input) : pointer to thread function
//args (input) : pointer to arguments of thread function
//On Linux, this function is same as "nlthread_create". To detach a 
//thread from its creator, it needs to call "nlthread_detach"
bool nlthread_detach_create(_Out_ nlthread_t *tid, 
			    _In_ nlthread_detach_func f, 
			    _In_opt_ void *arg);

//Detach the thread from its parent
//This is a NOP on MS Windows platform
bool nlthread_detach(_In_ nlthread_t tid);

//Wait for a given thread to terminate
bool nlthread_join(_In_ nlthread_t tid); 

//A thread teminates itself
//On Linux, it is NOP.
void nlthread_end();

//A detach thread teminates itself
//On Linux, it is NOP.
//On Windows, a thread created by "nlthread_detach_thread" has to call 
//this function to close the thread handle. 
void nlthread_detach_end();

/*******************************************************************
 * Critical Sections
 ******************************************************************/

/** nlthread_cs_init
 *
 *  \brief Create a critical section object.
 *
 *  \param cs Critical section.
 */
bool nlthread_cs_init(_Out_ nlthread_cs_t* cs );

/** nlthread_cs_delete
 *
 *  \brief Delete a critical section object.
 *
 *  \param cs Critical section.
 */
bool nlthread_cs_delete(_In_ nlthread_cs_t* cs );

/** nlthread_cs_enter
 *
 *  \brief Enter a critical section.
 *
 *  \param cs Critical section.
 */
bool nlthread_cs_enter(_In_  nlthread_cs_t* cs );

/** nlthread_cs_leave
 *
 *  \brief Leave a critical section.
 *
 *  \param cs Critical section.
 */
bool nlthread_cs_leave(_In_  nlthread_cs_t* cs );

/** nlthread_cs_tryenter
 *
 *  \brief Try to enter a critical section.
 *
 *  \param cs Critical section.
 */
bool nlthread_cs_tryenter(_In_  nlthread_cs_t* m );

//Dynamically allocate a mutex variable
bool nlthread_mutex_init(_Out_ nlthread_mutex_t *m);

//Free a mutex variable that is dynamically allocated
bool nlthread_mutex_destroy(_In_ nlthread_mutex_t *m);

//Lock a mutex
bool nlthread_mutex_lock(_In_ nlthread_mutex_t *m);

//Try lock a mutex
bool nlthread_mutex_trylock(_In_ nlthread_mutex_t *m);

//Unlock a mutex
bool nlthread_mutex_unlock(_In_ nlthread_mutex_t *m);

//Dynamically allocate a condition variable
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_init(_Out_ nlthread_cond_t *c);

//Free a condition variable that is dynamically allocated
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_destroy(_In_ nlthread_cond_t *c);

//Wake up one thread waiting on the condition
//On MS Windows platform, a condition variable is an event
bool nlthread_cond_signal(_In_ nlthread_cond_t *c);

//Wait for a condition to be true  with timeout.
//m (input): the mutex that protects the condition. It the caller doesn't
//           own the mutex, this function will fail.
//c (input): the condition variable. On MS Windows, the condition variable
//           is an event
//t (input): specifies the timeout
//btimeout (output): if it is true, it means that the condition hasn't been 
//                   satisfied in the specified amount of time
//Return: return false if the caller doesn't own the mutex or timeout; 
//        otherwise, return true.
bool nlthread_cond_timedwait(_In_ nlthread_cond_t *c, _In_ nlthread_mutex_t *m, 
			     _In_ nlthread_timeout *t, _Out_ bool *btimeout);

//Wait for a condition to be true.
bool nlthread_cond_wait(_In_ nlthread_cond_t *c, _In_ nlthread_mutex_t *m);


//To  obtain a timeout value in nlthrea_timout structure
//t (output): on Linux it is the returned timespec structure; on
//            MS Window, it is just the intergers of second and millisecond
//time_in_second: specify the timeout value in seconds
//time_in_millisec: specify the timeout value in milliseconds
void nlthread_maketimeout(_Out_ nlthread_timeout *t, 
			  _In_ int time_in_second,
			  _In_ int time_in_millisec);

//initialize a semaphore with an initial value
void nlthread_sem_init(_Out_ nlthread_sem_t *sem,int initial);


//release a semaphore
bool nlthread_sem_post(_In_ nlthread_sem_t *sem);


//wait on a semaphore
bool nlthread_sem_wait(_In_ nlthread_sem_t *sem);

//close a semaphore
//On Window and Linux, this function is NOP
bool nlthread_sem_close(_In_ nlthread_sem_t *sem);

#endif
