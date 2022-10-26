/*
 * pgpThreads.h
 *
 * NAI generic thread code.
 *
 */

#ifndef Included_pgpThreads_h
#define Included_pgpThreads_h

#if PGP_MACINTOSH /* This threads lib is not supported by the Mac yet */

typedef void *		PGPMutex_t;

#define PGPMutexLock(x)
#define PGPMutexUnlock(x)
#define PGPMutexCreate(x, y)
#define PGPMutexDestroy(x)

#else

#include "pgpPFLConfig.h"
#include "pgpBase.h"

#ifdef _PGP_THREAD
# undef _PGP_THREAD
#endif

#ifdef _PGP_SEMAPHORE
# undef _PGP_SEMAPHORE
#endif

/*
 * On all platforms but Win32, cond variables can do broadcasts.
 * cond_init2 needs to be special for Win32 to support broadcasts.
 * Use cond_init if you never need broadcast.  Use cond_init2 if you
 * ONLY do broadcasts.
 */
#ifndef WIN32
# define PGPCondCreate2	PGPCondCreate
#endif

#if PGP_UNIX_SOLARIS

#include <thread.h>
#include <synch.h>

#define _PGP_THREAD
#define _PGP_SEMAPHORE
#undef HAVE_PTHREAD_CREATE
#define HAVE_PTHREAD_CREATE 0	/* we also have this, but don't want it */
#undef HAVE_SEMGET
#define HAVE_SEMGET 0		/* don't want this either */

typedef void	*(*VFP)();

/* solaris threads are preemptive */
#define PGPTHREAD_PREEMPTIVE	1

/* default attr states */
#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL

/* thread state - joinable or not */
#define PGPTHREAD_CREATE_JOINABLE 0
#define PGPTHREAD_CREATE_DETACHED THR_DETACHED

/* thread scope - who is in scheduling pool */
#define PGPTHREAD_SCOPE_PROCESS   0
#define PGPTHREAD_SCOPE_SYSTEM	THR_BOUND

/* mutex and condition variable scope - process or system */
#define PGPTHREAD_SHARE_PRIVATE   USYNC_THREAD
#define PGPTHREAD_SHARE_PROCESS   USYNC_PROCESS

typedef int		PGPThreadAttr_t;
typedef thread_t	PGPThread_t;
typedef int		PGPMutexAttr_t;
typedef mutex_t		PGPMutex_t;
typedef int	 	PGPCondAttr_t;
typedef cond_t		PGPCond_t;
typedef thread_key_t	PGPThreadKey_t;
typedef sema_t 		PGPSem_t;
typedef struct		PGPSemAttr_t {
	unsigned int count;
	int type;
	void *arg;
} PGPSemAttr_t;

extern PGPSemAttr_t PGPSemAttr_def;
#define PGPSemAttr_default &PGPSemAttr_def

#endif /* end PGP_UNIX_SOLARIS */

#if PGP_OSX

	#if HAVE_PTHREAD_CREATE
		#undef HAVE_PTHREAD_CREATE
		#define HAVE_PTHREAD_CREATE 1
	#endif

	#if HAVE_SEM_INIT
		#undef HAVE_SEM_INIT
		#define HAVE_SEM_INIT 0
	#endif

	#if HAVE_SEMGET
		#undef HAVE_SEMGET
		#define HAVE_SEMGET 0
	#endif

	#define _PGP_SEMAPHORE

	typedef int				PGPSem_t;
	typedef struct			PGPSemAttr_t
	{
		PGPInt32			initialValue;
		PGPInt32			maxValue;
	} PGPSemAttr_t;

	extern PGPSemAttr_t PGPSemAttr_def;
	#define PGPSemAttr_default &PGPSemAttr_def

#endif /* end PGP_OSX */

#if HAVE_PTHREAD_CREATE

#if PGP_UNIX_AIX
#include <sys/types.h>
#include <synch.h>
#endif
#include <pthread.h>

#define _PGP_THREAD

#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL

typedef void	*(*VFP)();

#if PGP_UNIX_LINUX	/* I think Linux pthreads are preemptive */
# define PGPTHREAD_PREEMPTIVE	1
#else
# define PGPTHREAD_PREEMPTIVE	0
#endif

/* default attr states */
#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL

/* thread state - joinable or not */
#define PGPTHREAD_CREATE_JOINABLE PTHREAD_CREATE_JOINABLE
#define PGPTHREAD_CREATE_DETACHED PTHREAD_CREATE_DETACHED

/* thread scope - who is in scheduling pool */
#define PGPTHREAD_SCOPE_PROCESS   PTHREAD_SCOPE_PROCESS
#define PGPTHREAD_SCOPE_SYSTEM	PTHREAD_SCOPE_SYSTEM

/* mutex and condition variable scope - process or system */
#define PGPTHREAD_SHARE_PRIVATE   0
#define PGPTHREAD_SHARE_PROCESS   0

typedef pthread_attr_t		PGPThreadAttr_t;
typedef pthread_t		PGPThread_t;
typedef pthread_mutex_t		PGPMutex_t;
typedef pthread_mutexattr_t	PGPMutexAttr_t;
typedef pthread_condattr_t	 	PGPCondAttr_t;
typedef pthread_cond_t		PGPCond_t;
typedef pthread_key_t		PGPThreadKey_t;

#endif /* end PTHREAD_CREATE */

#if HAVE_PTHREAD_ATTR_CREATE

#define _PGP_THREAD

#include <pthread.h>

/* dce threads are preemptive */
#define PGPTHREAD_PREEMPTIVE	1

#endif /* end HAVE_PTHREAD_ATTR_CREATE */

#if HAVE_SEM_INIT

#undef HAVE_SEMGET	/* prefer POSIX sem_init over semget */
#define HAVE_SEMGET 0

#define _PGP_SEMAPHORE

#if defined(PGP_UNIX_AIX) || defined(PGP_UNIX_LINUX)
#include <semaphore.h>
typedef sem_t PGPSem_t; /* not a struct under AIX or Linux */
#else
typedef struct sem_t PGPSem_t;
#endif

typedef struct			PGPSemAttr_t {
	int shared;
	int value;
} PGPSemAttr_t;

extern PGPSemAttr_t PGPSemAttr_def;
#define PGPSemAttr_default &PGPSemAttr_def

#endif

#if HAVE_SEMGET

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define _PGP_SEMAPHORE
#define _PGP_MUTEX_ONLY

typedef int  			PGPSem_t;
typedef struct			PGPSemAttr_t {
	int nsems;
	int type;
} PGPSemAttr_t;

/* mutexes from semaphores */
#ifndef _PGP_THREAD
typedef PGPSem_t		PGPMutex_t;
typedef int				PGPMutexAttr_t;
#endif

extern PGPSemAttr_t PGPSemAttr_def;
#define PGPSemAttr_default &PGPSemAttr_def

#endif /* HAVE_SEMGET */

#if PGP_WIN32
/***********************************
 *								 *
 * thread definitions for winnt	*
 *								 *
 ***********************************/
#include <windows.h>

#define _PGP_THREAD
#define _PGP_SEMAPHORE

typedef unsigned (__stdcall *VFP)(void *);
/* typedef DWORD WINAPI (*VFP)(LPVOID); */

/* winnt threads are preemptive */
#define PGPTHREAD_PREEMPTIVE	1

/* default attr states */
#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL

/* thread state - joinable or not */
#define PGPTHREAD_CREATE_JOINABLE	0
#define PGPTHREAD_CREATE_DETACHED	1

/* thread scope - who is in scheduling pool */
#define PGPTHREAD_SCOPE_PROCESS	0
#define PGPTHREAD_SCOPE_SYSTEM	1

/* mutex and condition variable scope - process or system */
#define PGPTHREAD_SHARE_PRIVATE	0
#define PGPTHREAD_SHARE_PROCESS	1

typedef int	PGPThreadAttr_t;
typedef HANDLE	PGPThread_t;
typedef int	PGPCondAttr_t;
typedef void	*PGPCond_t;
typedef int	PGPMutexAttr_t;
typedef HANDLE  PGPMutex_t;
typedef DWORD	PGPThreadKey_t;
typedef HANDLE	PGPSem_t;
typedef struct PGPSemAttr_t {
	int icount;
	int max;
	PFLChar *name;
} PGPSemAttr_t;

extern PGPSemAttr_t PGPSemAttr_def;
#define PGPSemAttr_default &PGPSemAttr_def

#endif /* winnt threads */


#if PGP_SYMBIAN

#define _PGP_THREAD
#define _PGP_SEMAPHORE

typedef int (*VFP)(void *);

/* symbian threads are preemptive */
#define PGPTHREAD_PREEMPTIVE	1

/* default attr states */
#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL
#define PGPSemAttr_default NULL

/* thread state - joinable or not */
#define PGPTHREAD_CREATE_JOINABLE	0
#define PGPTHREAD_CREATE_DETACHED	1

/* thread scope - who is in scheduling pool */
#define PGPTHREAD_SCOPE_PROCESS	0
#define PGPTHREAD_SCOPE_SYSTEM	1

/* mutex and condition variable scope - process or system */
#define PGPTHREAD_SHARE_PRIVATE	0
#define PGPTHREAD_SHARE_PROCESS	1

typedef int	PGPThreadAttr_t;
typedef void *	PGPThread_t;
typedef int	PGPCondAttr_t;
typedef void	*PGPCond_t;
typedef int	PGPMutexAttr_t;
typedef void *  PGPMutex_t;
typedef void *	PGPThreadKey_t;
typedef void *	PGPSem_t;
typedef int PGPSemAttr_t;


#endif /* symbian threads */

#ifndef _PGP_THREAD

typedef void	*(*VFP)();

/* default attr states */
#define PGPMutexAttr_default	NULL
#define PGPCondAttr_default	NULL

/* thread state - joinable or not */
#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 0
/* thread scope - who is in scheduling pool */
#define PTHREAD_SCOPE_PROCESS   0
#define PTHREAD_SCOPE_SYSTEM	0

/* mutex and condition variable scope - process or system */
#define PTHREAD_SHARE_PRIVATE   0
#define PTHREAD_SHARE_PROCESS   0

#ifndef _PGP_MUTEX_ONLY
typedef int	PGPMutexAttr_t;
typedef int	PGPMutex_t;
#endif
typedef int	PGPThreadAttr_t;
typedef int	PGPThread_t;
typedef int PGPCondAttr_t;
typedef int	PGPCond_t;
typedef int PGPThreadKey_t;

#endif /* no threads support */

#ifndef _PGP_SEMAPHORE

typedef int PGPSem_t;
typedef int PGPSemAttr_t;

#define PGPSemAttr_default 0

#endif /* no semaphores */

/*
 * PGPthreads
 */

PGP_BEGIN_C_DECLARATIONS

int PGPThreadAttrCreate( PGPThreadAttr_t *attr );
int PGPThreadAttrDestroy( PGPThreadAttr_t *attr );
int PGPThreadAttrGetDetachState( PGPThreadAttr_t *attr, int *detachstate );
int PGPThreadAttrSetDetachState( PGPThreadAttr_t *attr, int detachstate );

int PGPThreadCreate(PGPThread_t *t, PGPThreadAttr_t *attr, VFP func, void *arg);
void PGPThreadYield(void);
void PGPThreadExit(PGPThread_t tid);
void PGPThreadJoin( PGPThread_t tid, int *status );
void PGPThreadKill( PGPThread_t tid, int sig );

int PGPMutexCreate( PGPMutex_t *mp, PGPMutexAttr_t *attr );
int PGPMutexDestroy( PGPMutex_t *mp );
int PGPMutexLock( PGPMutex_t *mp );
int PGPMutexUnlock( PGPMutex_t *mp );
int PGPMutexTryLock( PGPMutex_t *mp );

int PGPSemCreate(PGPSem_t *ms, PGPSemAttr_t *attr);
int PGPSemWait(PGPSem_t *ms);
int PGPSemTryWait(PGPSem_t *ms);
int PGPSemPost(PGPSem_t *ms);
int PGPSemDestroy(PGPSem_t *ms);

int PGPCondCreate( PGPCond_t *cv, PGPCondAttr_t *attr );
#if PGP_WIN32
 int PGPCondCreate2( PGPCond_t *cv, PGPCondAttr_t *attr );
#endif
int PGPCondDestroy( PGPCond_t *cv );
int PGPCondWait( PGPCond_t *cv, PGPMutex_t *mp );
int PGPCondSignal( PGPCond_t *cv );
int PGPCondBroadcast( PGPCond_t *cv );

int PGPThreadKeyCreate(PGPThreadKey_t *key, void (*destructor)(void*value));
int PGPThreadGetSpecific(PGPThreadKey_t key, void **value);
int PGPThreadSetSpecific(PGPThreadKey_t key, void *value);
int PGPThreadKeyDelete(PGPThreadKey_t key);

PGP_END_C_DECLARATIONS

#endif /* PGP_MACINTOSH */
#endif /* Included_pgpThreads_h */
