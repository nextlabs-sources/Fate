/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpBase64_h	/* [ */
#define Included_pgpBase64_h

#include "pgpPFLErrors.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * Base 64 encoding
 */
PGPSize
PGPBase64GetEncodeLength(
	PGPSize			inlen);

PGPError
PGPBase64Encode(
	const PGPByte	*inBuf,
	PGPSize			inLen,
	PGPChar8		*outString);

/*
 * Base 64 decoding
 */
PGPSize
PGPBase64GetDecodeLength(
	PGPSize		inlen);

PGPError
PGPBase64Decode(
	const PGPChar8	*in,
	PGPSize			inlen,
	PGPByte			*out,
	PGPSize			*outlen);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpBase64_h */

