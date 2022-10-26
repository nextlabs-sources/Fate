/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpIOUtilities_h	/* [ */
#define Included_pgpIOUtilities_h

#include "pgpIO.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Copy bytes from one I/O to another. 
	Caller will want to call PGPIOSetPos to set file mark first.
____________________________________________________________________________*/
PGPError	PGPCopyIO( PGPIORef fromIO, PGPFileOffset numBytes, PGPIORef toIO );

/* same as PGPCopyIO(), but use user-supplied buffer */
PGPError	PGPCopyIOUsingBuffer( PGPIORef fromIO,
				PGPFileOffset numBytes, PGPIORef toIO,
				void *buffer, PGPSize bufferSize );
				


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIOUtilities_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
