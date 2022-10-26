/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

/* base class */
#include "pgpFileIO.h"
#include <stdio.h>

typedef struct PGPStdFileIO		PGPStdFileIO;
typedef PGPStdFileIO *			PGPStdFileIORef;


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Caller loses ownership of the FILE * after calling PGPNewStdFileIO().
	
	If autoClose is TRUE, then the FILE * is automatically closed via fclose()
	when the PGPFileIO is destroyed.
	
	if autoClose is FALSE, then the caller *may not* use the FILE * until
	the PGPFileIO is destroyed, at which point ownership of the the FILE *
	reverts back to the caller.  The caller can't assume any particular
	state of the FILE * at that point however (e.g file position, eof, etc).
____________________________________________________________________________*/
PGPError	PGPNewStdFileIO( PGPMemoryMgrRef context,
				FILE *file, PGPBoolean autoClose, PGPStdFileIORef *outRef );


PGPBoolean	PGPStdFileIOIsValid( PGPStdFileIORef ref );
#define PGPValidateStdFileIO( ref )	\
		PGPValidateParam( PGPStdFileIOIsValid( ref ) )



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
