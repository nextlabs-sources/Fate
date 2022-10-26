/*____________________________________________________________________________
        Copyright (C) 2002 PGP Corporation
        All rights reserved.

        $Id$
____________________________________________________________________________*/

#ifndef Included_pgpCBCPriv_h	/* [ */
#define Included_pgpCBCPriv_h

#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"


PGP_BEGIN_C_DECLARATIONS


#define PGP_CBC_MAXBLOCKSIZE 20



/*____________________________________________________________________________
	internal glue routine follow; use is discouraged
____________________________________________________________________________*/

PGPSize		pgpsdaCBCGetKeySize( PGPCBCContextRef ref );
PGPSize		pgpsdaCBCGetBlockSize( PGPCBCContextRef ref );

PGPError 	pgpsdaCBCDecryptInternal(PGPCBCContextRef ref, const void *in,
					PGPSize bytesIn, void *	out);
PGPError 	pgpCBCEncryptInternal(PGPCBCContextRef ref, const void *in,
					PGPSize bytesIn, void *	out);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCBCPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
