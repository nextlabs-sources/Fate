#ifndef _DSKERNELTYPES_H_
#define _DSKERNELTYPES_H_	1
#ifdef WIN32
#include <ntifs.h>

#elif LINUX
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>        /* inodes and friends */
#include <linux/types.h>     /* uint32_t */
#include <linux/slab.h>      /* kmalloc and kfree */
#include <linux/time.h>      /* struct timeval */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <linux/moduleparam.h>
#include <linux/workqueue.h>  /* work queue */
#else
#include <linux/tqueue.h>    /* task queues */
#endif

#include <linux/wait.h>      /* wait_queue_head_t */
#include <linux/kmod.h>      /* call_usermodehelper */
#include <linux/interrupt.h> /* tasklets */
#include <linux/delay.h>     /* time delay routines */
#include <asm/uaccess.h>     /* user-space translations */
#include <asm/atomic.h>      /* atomic_t variables */
#include <asm/signal.h>      /* handle inappropriate signal of event */

#endif	//#ifdef WIN32 ... #elif LINUX ...

#define DS_PASSIVE_LEVEL_LOCK		0
#define DS_APC_LEVEL_LOCK			1
#define DS_DISPATCH_LEVEL_LOCK		2

//Common structure for all kernel drivers cross OS’s(Linux, Windows etc.)
//This support to >= DISPATCH_LEVEL in Windows and handle up to interrupt 
//level in Linux
typedef struct _BJ_SPIN_LOCK
{
	UINT32				ulLockLevel;
#ifdef WIN32
	KIRQL				kIRQSave;
	//For Spin Lock that in DISPATCH_LEVEL
	KSPIN_LOCK			kLock;
#if WINVER >= 0x0501	//WinXP or above
	KLOCK_QUEUE_HANDLE	kLockQueue;
#endif	//	#if WINVER >= 0x0501

	//For Spin Lock that in APC_LEVEL
	FAST_MUTEX			fFastMutex;
#elif LINUX
	ULONG				lock_flags;
	spinlock_t			kLock;
#endif //#ifdef WIN32 ... #elif LINUX ... 
}BJ_SPIN_LOCK, *PBJ_SPIN_LOCK;

#endif	//#ifndef _DSKERNELTYPES_H_