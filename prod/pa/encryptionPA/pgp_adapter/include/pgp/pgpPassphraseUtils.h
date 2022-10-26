/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpPassphraseUtils_h	/* [ */
#define Included_pgpPassphraseUtils_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPUInt32	pgpEstimatePassphraseQuality(const PFLChar *passphrase );

/* helper Unicode classification function */
void		pgpCharEntropyCategoryInfo( PGPUInt c, PGPUInt *category_length, PGPUInt *category_index );
/* new method to estimate the entropy */
PGPUInt 	pgpEstimateTrueEntropyBits( const PGPUInt16 *passphrase, PGPUInt length );

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpPassphraseUtils_h */
