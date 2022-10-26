/*____________________________________________________________________________
	Copyright (C) 2007 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpBase32_h	/* [ */
#define Included_pgpBase32_h

#include "pgpPFLErrors.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * Base 32 encoding
 */

/**
 * Determine the storage requirement for the encoded string created from
 * 'inlen' bytes.
 *
 * NOTE: This function does NOT return + 1 to account for NULL termination
 *
 * @param	inlen				unencoded data length
 * @return	encoded string length (not including NULL terminator)
 */

PGPSize
PGPBase32GetEncodeLength(
	PGPSize			inlen);

/**
 * Encode inBuf bytes of inLen length, storing the resulting encoded data in
 * outString. This function will NULL terminate the encoded string.
 *
 * NOTE:	outString is assumed to be at least
 *			PGPBase32GetEncodeLength(inLen) + 1 bytes in length to account for
 *			NULL termination.
 *
 * @param	inBuf				data to encode
 * @param	inLen				number of bytes in inBuf to encode
 * @param	outString			buffer which will receive the encoded string
 * @return	PGPError
 */

PGPError
PGPBase32Encode(
	const PGPByte	*inBuf,
	PGPSize			inLen,
	PGPChar8		*outString);

/*
 * Base 32 decoding
 */

/**
 * Determine the storage requirement for the decoded string created from
 * 'inlen' bytes.
 *
 * NOTE: This function does NOT return + 1 to account for NULL termination
 *
 * @param	inlen				encoded data length
 * @return	decoded string length (not including NULL terminator)
 */

PGPSize
PGPBase32GetDecodeLength(
	PGPSize			inlen);

/**
 * Decode in bytes of inlen length, storing the resulting decoded data in out.
 * This function will NULL terminate the decoded string.
 *
 * NOTE:	out is assumed to be at least
 *			PGPBase32GetDecodeLength(inlen) + 1 bytes in length to account for
 *			NULL termination.
 *
 * @param	in		data to decode
 * @param	inlen	number of bytes to decode
 * @param	out		buffer which will receive the decoded string
 * @param	outlen	decoded string length (not including NULL terminator)
 * @return	PGPError
 */

PGPError
PGPBase32Decode(
	const PGPChar8	*in,
	PGPSize			inlen,
	PGPByte			*out,
	PGPSize			*outlen);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpBase32_h */

