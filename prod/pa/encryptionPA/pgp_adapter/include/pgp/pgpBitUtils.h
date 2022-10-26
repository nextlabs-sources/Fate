/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpBitUtils_h	/* [ */
#define Included_pgpBitUtils_h

#include "pgpBase.h"
#include "pgpDebug.h"

typedef PGPUInt32	PGPBitIndex;
typedef PGPUInt32	PGPBitCount;

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

void			pgpSetBit(PGPByte *bitmap, PGPBitIndex bitIndex);
void			pgpSetBitRange(PGPByte *bitmap, PGPBitIndex startBitIndex,
						PGPBitCount numBits);

void			pgpClearBit(PGPByte *bitmap, PGPBitIndex bitIndex);
void			pgpClearBitRange(PGPByte *bitmap, PGPBitIndex startBitIndex,
						PGPBitCount numBits);

PGPBoolean		pgpTestBit(PGPByte *bitmap, PGPBitIndex bitIndex);
PGPBoolean		pgpTestAndClearBit(PGPByte *bitmap, PGPBitIndex bitIndex);
PGPBoolean		pgpTestAndSetBit(PGPByte *bitmap, PGPBitIndex bitIndex);

PGPBitCount		pgpCountAndClearBitRange(PGPByte *bitmap,
						PGPBitIndex startBitIndex, PGPBitCount numBits);
PGPBitCount		pgpCountAndSetBitRange(PGPByte *bitmap,
						PGPBitIndex startBitIndex, PGPBitCount numBits);
PGPBitCount		pgpCountClearBitsInRange(PGPByte *bitmap,
						PGPBitIndex startBitIndex, PGPBitCount numBits);
PGPBitCount		pgpCountSetBitsInRange(PGPByte *bitmap,
						PGPBitIndex startBitIndex, PGPBitCount numBits);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpBitUtils_h */
