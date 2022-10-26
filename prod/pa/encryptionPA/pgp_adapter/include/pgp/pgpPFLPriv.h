/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpPFLPriv_h	/* [ */
#define Included_pgpPFLPriv_h

#include "pgpPFLConfig.h"

#include "pgpDebug.h"
#include "pgpPFLErrors.h"

#define PGPValidateParam( expr )	\
	if ( ! (expr ) )	\
	{\
		pgpAssert( expr );\
		return( kPGPError_BadParams );\
	}

#define PGPValidatePtr( ptr )	\
			PGPValidateParam( (ptr) != NULL && \
					pgpDebugPtrReadAccess( ptr, sizeof(size_t) ) )

#endif /* ] Included_pgpPFLPriv_h */
