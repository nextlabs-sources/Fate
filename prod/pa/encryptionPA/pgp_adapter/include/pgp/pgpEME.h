/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpEME_h	/* [ */
#define Included_pgpEME_h

#include "pgpSymmetricCipher.h"


PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	An EME context requires use of a symmetric cipher which has
	been created (but whose key has not been set).  The symmetric
	cipher must have a block size of 16 bytes.  An error will
	be returned if this condition does not hold.

	After the call, the EMEContextRef "owns" the symmetric ref
	and will dispose of it properly (even if an error occurs).
	The caller should no longer reference it.
____________________________________________________________________________*/

PGPError 	PGPNewEMEContext( PGPSymmetricCipherContextRef ref,
					PGPEMEContextRef *outRef );

/*____________________________________________________________________________
	Disposal clears all data in memory before releasing it.
____________________________________________________________________________*/

PGPError 	PGPFreeEMEContext( PGPEMEContextRef ref );

/*____________________________________________________________________________
	Make an exact copy, including current state.  Original is not changed.
____________________________________________________________________________*/

PGPError 	PGPCopyEMEContext( PGPEMEContextRef ref, PGPEMEContextRef *outRef );

/*____________________________________________________________________________
	Key the EME context.  Key size is that of underlying symmetric cipher.
____________________________________________________________________________*/

PGPError 	PGPInitEME( PGPEMEContextRef ref, const void *key );

/*____________________________________________________________________________
	Call repeatedly to process arbitrary amounts of data.  Each call must
	have bytesIn be a multiple of the cipher block size.  offset is the
	offset in 512-byte blocks from the front of the file.  nonce is a per-file
	constant which should be unique among all files that are encrypted with
	the samem key.
____________________________________________________________________________*/

PGPError 	PGPEMEEncrypt( PGPEMEContextRef ref, const void *in,
					PGPSize bytesIn, void *out, PGPUInt64 offset,
					PGPUInt64 nonce );
					
PGPError 	PGPEMEDecrypt( PGPEMEContextRef ref, const void *in,
					PGPSize bytesIn, void *out, PGPUInt64 offset,
					PGPUInt64 nonce );

/*____________________________________________________________________________
	Determine key and block size for EME mode.  Block size is fixed and
	is independent of cipher block size.
____________________________________________________________________________*/

PGPError 	PGPEMEGetSizes( PGPEMEContextRef ref,
					PGPSize *keySize, PGPSize *blockSize );

/*____________________________________________________________________________
	Get the symmetric cipher being used for this EME context.
____________________________________________________________________________*/

PGPError 	PGPEMEGetSymmetricCipher( PGPEMEContextRef ref,
					PGPSymmetricCipherContextRef *outRef );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpEME_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
