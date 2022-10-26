/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Abstract superclass for all proxy objects.
	
	A pgpProxyIO uses another object to do the real IO, but performs
	additional processing on top of that.  Proxy objects could be 
	chained.  Examples of possible proxy objects include:
		memory buffer and/or readahead for faster file I/O
		checksum computations
		merging two fork files into one virtual fork
		etc
	
	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpProxyIO_h	/* [ */
#define Included_pgpProxyIO_h

/* base class */
#include "pgpIO.h"


typedef struct PGPProxyIO		PGPProxyIO;
typedef PGPProxyIO *			PGPProxyIORef;


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



/* note: one would normally always create a subclass;
this is just here for debug testing */
PGPError	PGPNewProxyIO( PGPIORef io,
				PGPBoolean ownsProxy, PGPProxyIORef *outRef );


PGPBoolean	PGPProxyIOIsValid( PGPProxyIORef ref );
#define PGPValidateProxyIO( ref )	PGPValidateParam( PGPProxyIOIsValid( ref ) )



#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpProxyIO_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
