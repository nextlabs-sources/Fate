/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	Convert data to/from big endian/little endian formats.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpEndianConversion_h	/* [ */
#define Included_pgpEndianConversion_h

#include "pgpPFLConfig.h"

#include "pgpBase.h"

enum PGPEndianType_
{
	kPGPBigEndian,
	kPGPLittleEndian,
	
	PGP_ENUM_FORCE( PGPEndianType_ )
};
PGPENUM_TYPEDEF( PGPEndianType_, PGPEndianType );

PGP_BEGIN_C_DECLARATIONS

/* use this if you need to make the endianness explicit */
PGPUInt16	PGPEndianToUInt16( PGPEndianType fromType, const PGPByte *	raw );
PGPUInt32	PGPEndianToUInt32( PGPEndianType fromType, const PGPByte *	raw );
#if PGP_HAVE64
PGPUInt64	PGPEndianToUInt64( PGPEndianType fromType, const PGPByte *	raw );
#endif

void		PGPUInt16ToEndian( PGPUInt16 num, PGPEndianType toType,
				PGPByte out[ sizeof( PGPUInt16 ) ] );
void		PGPUInt32ToEndian( PGPUInt32 num, PGPEndianType toType,
				PGPByte out[ sizeof( PGPUInt32 ) ] );
#if PGP_HAVE64
void		PGPUInt64ToEndian( PGPUInt64 num, PGPEndianType toType,
				PGPByte out[ sizeof( PGPUInt64 ) ] );
#endif

/* convert types to our standard storage format */
/* Do NOT assume what the format is */
void		PGPUInt16ToStorage( PGPUInt16 num,
				PGPByte out[ sizeof( PGPUInt16 ) ] );
void		PGPUInt32ToStorage( PGPUInt32 num,
				PGPByte out[ sizeof( PGPUInt32 ) ] );
#if PGP_HAVE64
void		PGPUInt64ToStorage( PGPUInt64 num,
				PGPByte out[ sizeof( PGPUInt64 ) ] );
#endif

/* convert from our standard storage format */
/* Do NOT assume what the format is */
PGPUInt16	PGPStorageToUInt16( PGPByte const in[ sizeof( PGPUInt16 ) ] );
PGPUInt32	PGPStorageToUInt32( PGPByte const in[ sizeof( PGPUInt32 ) ] );
#if PGP_HAVE64
PGPUInt64	PGPStorageToUInt64( PGPByte const in[ sizeof( PGPUInt64 ) ] );
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpEndianConversion_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
