/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	
	Interface for all PGPIO objects.  Note that PGPIO doesn't include the
	notion of files, nor are there creation routines.  Only subclasses
	can be instantiated.  See appropriate subclass header files for creation
	routines.  See PGPIO.h for more info.
	
	Notes:
		Unlike C++, one needs to cast PGPIO subclass types to PGPIORef to keep
	the compiler happy when calling the routines in this file.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpIO_h	/* [ */
#define Included_pgpIO_h

#include "pgpPFLConfig.h"
#include "pgpMemoryMgr.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

typedef struct PGPIO	PGPIO;
typedef struct PGPIO *	PGPIORef;

#define	kInvalidPGPIORef			((PGPIORef) NULL)
#define PGPIORefIsValid( ref )		( (ref) != kInvalidPGPIORef )

/* frees any and all subclasses of PGPIO */
PGPError	PGPFreeIO( PGPIORef ref );


/* after reading, file position advances by number of bytes read */
PGPError	PGPIORead( PGPIORef ref, PGPSize requestCount,
						void *buffer, PGPSize *bytesRead );
						
/* after writing, file position advances by number of bytes read */
PGPError	PGPIOWrite( PGPIORef ref, PGPSize requestCount,
						const void *buffer );
						
/* set file position to this position */
PGPError	PGPIOSetPos( PGPIORef ref, PGPFileOffset newPos );
						
/* get current file position */
PGPError	PGPIOGetPos( PGPIORef ref, PGPFileOffset *curPos );
						
/* get the logical end of file (the file size) */
PGPError	PGPIOGetEOF( PGPIORef ref, PGPFileOffset *eof );

#if PGPIO_EOF
/* set the logical end of file (the file size) */
/* excess data is removed or the file is grown if needed */
/* it is illegal to set the EOF less than the current file pos */
PGPError	PGPIOSetEOF( PGPIORef ref, PGPFileOffset eof );
#endif

/* flush to backing store */
PGPError	PGPIOFlush( PGPIORef ref );


/* returns TRUE if currently at EOF */
PGPBoolean	PGPIOIsAtEOF( PGPIORef ref );

PGPMemoryMgrRef	PGPIOGetMemoryMgr( PGPIORef ref );

PGPBoolean	PGPIOIsValid( PGPIORef ref );
#define PGPValidateIO( ref )	PGPValidateParam( PGPIOIsValid( ref ) )


#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIO_h	*/


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
