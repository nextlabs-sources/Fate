/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpHex_h	/* [ */
#define Included_pgpHex_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS


PGPBoolean	pgpIsValidHexChar( PFLChar	theChar );

PGPByte		pgpHexCharToNibble( PFLChar nibble );

void		pgpBytesToHex( PGPByte const * keyBytes,
				PGPSize	numBytes, PGPBoolean add0x, PFLChar *outString );

PGPUInt pgpHexToBytes( const PFLChar * hex,
				PGPUInt numBinaryBytes, PGPByte * buffer );

PGPUInt32	pgpHexToPGPUInt32( const PFLChar *hex );

void		pgpPGPUInt32ToHex( const PGPUInt32 number,
				PGPBoolean add0x, PFLChar *outString );
	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHex_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
