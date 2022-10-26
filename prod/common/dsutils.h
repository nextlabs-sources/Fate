// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc.,

// Redwood City CA,

// Ownership remains with Blue Jungle Inc, All rights reserved worldwide.
#ifndef _DSUTILS_H_
#define _DSUTILS_H_		1

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

#include "dstypes.h"
#include "dstdi.h"
#include "dsifs.h"

extern ULONG ntohl (ULONG netlong);
extern USHORT ntohs (USHORT netshort);
extern BOOL DSK_szTerminateString(char *szBuffer, ULONG ulBufLength, ULONG ulStringLength);
extern BOOL DSK_wzTerminateString(wchar_t *wzBuffer, ULONG ulBufLength, ULONG ulStringLength);
extern BOOL DSK_szCopyStringWithTerminate(char *szDst, ULONG ulDstSize, char *szSrc, ULONG ulSrcSize );
extern BOOL DSK_wzCopyStringWithTerminate(WCHAR *wzDst, ULONG ulDstSize, WCHAR *wzSrc, ULONG ulSrcSize);
extern BOOL DSK_InitSpinLock (PBJ_SPIN_LOCK pLock, ULONG ulLockType);
extern BOOL DSK_SpinLock(BJ_SPIN_LOCK *pLock);
extern BOOL DSK_SpinUnlock(BJ_SPIN_LOCK *pLock);
extern BOOL DSK_GetOSVersion(PULONG pulMajorVersion, PULONG pulMinorVersion,
							 PULONG pulBuildNumber, PVOID pContext);

extern BOOL LogSMBHeader(IN PVOID pData,IN ULONG ulDataSize);
extern BOOL ScreenSMBHeaderAndFillData(IN PVOID pData,IN ULONG ulDataSize,
									   IN ULONG ulRemoteAddr, IN ULONG ulLocalAddr,
									   IN OUT PIFS_TDI_SHAREDMEM pIFSTDIShare);

extern BOOL DSK_GetTDIInfo(IN PIFS_TDI_SHARED_OBJECT pTDISharedMem, 
						   OUT PFILE_OBJECT_CONTEXT pIFSFileCtx);
PVOID	
DSK_GetListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, ULONG ulObjectType);

PVOID	
DSK_GetListObjectTail(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, ULONG ulObjectType);

PVOID	
DSK_RemoveListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, 
						 PULONG pulObjectInQ, ULONG ulObjectType);

PVOID	
DSK_RemoveListObjectTail(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, 
						 PULONG pulObjectInQ, ULONG ulObjectType);

PVOID 
DSK_GetNextShareObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, 
						   int iNumOfNexts, BOOL fRemoveSharedObject, ULONG ulObjectType);

BOOL 
DSK_InsertListObjectHead(PDS_LIST_ENTRY pQList, PDS_LIST_ENTRY pdsObjectEntry,
						   PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ);

BOOL 
DSK_InsertListObjectTail(PDS_LIST_ENTRY pQList, PDS_LIST_ENTRY pdsObjectEntry,
						   PBJ_SPIN_LOCK pLock, PULONG pulObjectInQ);


BOOL DSK_RecoverIFSTDISharedQ(PIFS_TDI_SHAREDMEM pIFSTDISharedMem);

PVOID DSK_GetNextListObjectHead(PDS_LIST_ENTRY pQList, PBJ_SPIN_LOCK pLock, 
								int iNumOfNexts, BOOL fRemoveSharedObject, ULONG ulObjectType);


#endif	//#ifndef _DSUTILS_H_