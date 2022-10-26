/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpMemoryMgrPriv_h	/* [ */
#define Included_pgpMemoryMgrPriv_h

#include "pgpMemoryMgr.h"

#define PGPMemoryMgrIsValid( memoryMgr )	\
				( IsntPGPError( PGPValidateMemoryMgr( memoryMgr ) ) )
				
PGP_BEGIN_C_DECLARATIONS


PGPError	pgpCreateStandardMemoryMgr( PGPMemoryMgrRef *newMemoryMgr );
PGPError	pgpDisposeStandardMemoryMgrUserValue( PGPUserValue userValue );
void		pgpFreeDefaultMemoryMgrList(void);

void		pgpLeaksTrackerDumpLeaksForWin32Mgr( PGPMemoryMgrRef memoryMgr );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpMemoryMgrPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
