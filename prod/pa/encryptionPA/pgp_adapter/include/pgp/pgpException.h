/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpException_h
#define Included_pgpException_h

#include "pgpPFLErrors.h"

#define ThrowPGPError_(x)	throw(((PGPError) (x)))
#define ThrowIfPGPError_(x)	if (IsPGPError((x))) ThrowPGPError_(x)

#endif
