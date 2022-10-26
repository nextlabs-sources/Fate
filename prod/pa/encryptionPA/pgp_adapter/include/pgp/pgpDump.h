/*____________________________________________________________________________
	Copyright (C) 2004 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpDump_h	/* [ */
#define Included_pgpDump_h

#include "pgpPubTypes.h"
#include "pgpOptionList.h"

PGP_BEGIN_C_DECLARATIONS


enum	/* PGPDumpFlags */
{
	kPGPDumpFlags_None									= 0,
	kPGPDumpFlags_AlternateFormat						= (1UL << 0),
	kPGPDumpFlags_DumpIntegers							= (1UL << 1),
	kPGPDumpFlags_DumpLiteralPackets					= (1UL << 2),
	kPGPDumpFlags_DumpMarkerPackets						= (1UL << 3),
	kPGPDumpFlags_DumpPrivatePackets					= (1UL << 4),
	kPGPDumpFlags_UTCTime								= (1UL << 5),
} ;
typedef PGPFlags		PGPDumpFlags;


/*____________________________________________________________________________
	Produce a packet dump for the specified data
	The variable parameters are one or more
	PGPOptionListRef's which describe the inputs and outputs.
____________________________________________________________________________*/

PGPError 		PGPDump(PGPContextRef context, PGPDumpFlags flags,
							PGPOptionListRef firstOption, ...);


PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpDump_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
