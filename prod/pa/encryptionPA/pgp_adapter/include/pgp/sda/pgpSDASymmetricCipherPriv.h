/*____________________________________________________________________________
        Copyright (C) 2002 PGP Corporation
        All rights reserved.

        $Id$
____________________________________________________________________________*/
#ifndef Included_pgpSymmetricCipherPriv_h	/* [ */
#define Included_pgpSymmetricCipherPriv_h

#include "pgpBase.h"
#include "pgpMemoryMgr.h"
#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"


PGP_BEGIN_C_DECLARATIONS

/*____________________________________________________________________________
	a "virtual function table" for a cipher class.
	
	Use of this structure is discouraged. Use formal API where possible.
____________________________________________________________________________*/

struct PGPCipherVTBL
{
	PGPChar const *		name;
	PGPCipherAlgorithm	algorithm;
	PGPUInt		blocksize;
	PGPUInt		keysize;
	PGPUInt		context_size;
	PGPUInt		context_align;

	void			(*initKey)(void *priv, void const *key);
	void			(*encrypt)(void *priv, void const *in, void *out);
	void			(*decrypt)(void *priv, void const *in, void *out);
	void			(*wash)(void *priv, void const *buf, PGPSize len);
	void			(*rollback)(void *priv, PGPSize bytes);
};

PGPBoolean	pgpsdaSymmetricCipherIsValid( const PGPSymmetricCipherContext * ref);


PGPCipherVTBL const  *	pgpsdaCipherGetVTBL (PGPCipherAlgorithm alg);

PGPUInt32	pgpsdaCountSymmetricCiphers( void );

PGPMemoryMgrRef	pgpsdaGetSymmetricCipherMemoryMgr(
							PGPSymmetricCipherContextRef ref);

/* Get the default ciphers for operations */
PGPCipherVTBL const   *	pgpCipherDefaultKey( PGPEnv const *env);
PGPCipherVTBL const  *	pgpCipherDefaultKeyV3( PGPEnv const *env);

/* Static buffer size for keys */
#define PGP_CIPHER_MAXKEYSIZE	32	/* 256 bits */
#define PGP_CIPHER_MAXBLOCKSIZE	16	/* 128 bits */

/* Internal only ciphers, for decrypting PKCS-12 files */
#define kPGPCipherAlgorithm_RC2_40		100
#define kPGPCipherAlgorithm_RC2_128		101

/* Internal, buggy version of Twofish, for compatibility with an old
 * pre 2002 Solaris version which had byte order problems
 */
#define kPGPCipherAlgorithm_BadTwofish256	102

/* Internal for TLS */
#define kPGPCipherAlgorithm_Arc4_128	103

PGPError	pgpsdaNewSymmetricCipherContextInternal(PGPMemoryMgrRef memoryMgr,
					PGPCipherAlgorithm algorithm,
					PGPSymmetricCipherContextRef *outRef );
PGPError 	pgpsdaSymmetricCipherDecryptInternal(PGPSymmetricCipherContextRef ref,
					const void *in, void *out);
PGPError 	pgpsdaSymmetricCipherEncryptInternal(PGPSymmetricCipherContextRef ref,
					const void *in, void *out);
					
#if PGP_PLUGGABLECIPHERS
PGPError	pgpCipherAddPluginCiphers(PGPContextRef	context,
					void *vvtbls, PGPUInt32	nCiphers );
PGPError	pgpCipherRemovePluginCiphers( void );
#endif

#define ALG_IS_AES( encalg ) \
	(encalg == kPGPCipherAlgorithm_AES128 || \
	encalg == kPGPCipherAlgorithm_AES192 || \
	encalg == kPGPCipherAlgorithm_AES256)

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpSymmetricCipherPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
