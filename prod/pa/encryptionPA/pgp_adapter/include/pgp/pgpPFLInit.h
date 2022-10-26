/*____________________________________________________________________________
	Copyright (C) 2007 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpPFLInit_h	/* [ */
#define Included_pgpPFLInit_h

#include "pgpBase.h"
PGP_BEGIN_C_DECLARATIONS

/**
 * Initialize the PFL Library.
 * You only need to call this if you don't use the PGP SDK,
 * as the SDK Initialization will perform this task for you.
 */
PGPError pflInit(void);

/**
 * Shutdown the PFL Library
 * You dont need to call this if you initialize and shutdown the PGP SDK.
 */
PGPError pflShutdown(void);

PGP_END_C_DECLARATIONS
#endif /* ] Included_pgpPFLInit_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
