/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpRMWOLock_h	/* [ */
#define Included_pgpRMWOLock_h

#if PGP_WIN32
# include <windows.h>
#endif

#include "pgpTypes.h"
#include "pgpThreads.h"

PGP_BEGIN_C_DECLARATIONS

/* These files implement a Read Many/Write Once lock. To use, simply
initialize a lock for the resource and surround your reading and writing of it
with the appropriate calls. */

#if PGP_UNIX_SOLARIS	/* solaris has a system impl. of this */
 typedef rwlock_t PGPRMWOLock;
#elif PGP_UNIX_DARWIN || (PGP_UNIX_LINUX && (__USE_UNIX98 || __USE_XOPEN2K))
	typedef pthread_rwlock_t PGPRMWOLock;
#elif PGP_MACINTOSH
	typedef PGPByte PGPRMWOLock;
#else
 typedef struct PGPRMWOLock {
	PGPMutex_t	mutex;
	PGPSem_t	blockedReaders;
	PGPSem_t	blockedWriters;
	PGPUInt16	activeReaders;
	PGPUInt16	waitingReader;
	PGPUInt16	activeWriters;
	PGPUInt16	waitingWriter;
 } PGPRMWOLock;
#endif

#if PGP_MACINTOSH

# define InitializePGPRMWOLock(x)
# define DeletePGPRMWOLock(x)
# define PGPRMWOLockTryReading(x)	TRUE
# define PGPRMWOLockStartReading(x)
# define PGPRMWOLockStopReading(x)
# define PGPRMWOLockTryWriting(x)	TRUE
# define PGPRMWOLockStartWriting(x)
# define PGPRMWOLockStopWriting(x)

#else

void		InitializePGPRMWOLock(PGPRMWOLock * inLock);
void		DeletePGPRMWOLock(PGPRMWOLock * inLock);
PGPBoolean	PGPRMWOLockTryReading(PGPRMWOLock * inLock);	/* TRUE if acquired lock */
void		PGPRMWOLockStartReading(PGPRMWOLock * inLock);
void		PGPRMWOLockStopReading(PGPRMWOLock * inLock);
PGPBoolean	PGPRMWOLockTryWriting(PGPRMWOLock * inLock);	/* TRUE if acquired lock */
void		PGPRMWOLockStartWriting(PGPRMWOLock * inLock);
void		PGPRMWOLockStopWriting(PGPRMWOLock * inLock);

#endif /* PGP_MACINTOSH */

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRMWOLock_h */
