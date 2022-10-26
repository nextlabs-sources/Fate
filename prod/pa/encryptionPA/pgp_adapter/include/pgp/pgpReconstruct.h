/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.

	$Id$
____________________________________________________________________________*/
#ifndef Included_pgpReconstruct_h	/* [ */
#define Included_pgpReconstruct_h

#include "pgpPubTypes.h"
#include "pgpKeyServer.h"

PGP_BEGIN_C_DECLARATIONS

#define kPGPRecon_NumShares			5
#define kPGPRecon_Threshold			3
/* note that while the max prompt length is defined here, this max only applies
   to the PGPReconPrompts struct which is used for passing prompts into and
   out of the key reconstruction APIs.  The version 2 KRB format implemented
   for PGP 9.7.0 has variable-length prompt strings.  Thus, the KRB format
   does not impose a max length.  If 256 bytes proves to be too little, we can
   just increase the kPGPRecon_MaxPromptLen #define without necessitating a
   change to the KRB format.  Variable-length prompts were not implemented 
   at the API level in order to preserve compatibility with existing code
   that calls into the key reconstruction APIs. */
#define kPGPRecon_MaxPromptLen		( 256 - 1 )
#define kPGPRecon_MaxURLLen			( 256 - 1 )
#define kPGPRecon_MaxPassLen		( 256 - 1 )
#define kPGPRecon_MaxUserNameLen	( 128 - 1 )
#define kPGPRecon_MaxPasswordLen	( 128 - 1 )

#define kPGPRecon_WrapKeySize		16
#define kPGPRecon_ArmoredHashSize	( ( ( ( kPGPRecon_NumShares * kPGPRecon_WrapKeySize / 3 ) + 1 ) * 4 ) + 1 )


typedef struct PGPReconContext *			PGPReconContextRef;

#define	kInvalidPGPReconContextRef			((PGPReconContextRef) NULL)
#define PGPReconContextRefIsValid( ref )	( (ref) != kInvalidPGPReconContextRef )

typedef char PGPReconPrompts[kPGPRecon_NumShares][kPGPRecon_MaxPromptLen + 1];

typedef char PGPReconPasses[kPGPRecon_NumShares][kPGPRecon_MaxPassLen + 1];

typedef PGPError (*PGPReconstructEventHandler)(PGPContextRef recon,
				PGPEvent *event, PGPUserValue userValue);

/* these are for parsing version 1 blobs (which this code supports) */
#define kPGPRecon_MaxPromptLenV1	( 96 - 1 )
typedef char PGPReconPromptsV1[kPGPRecon_NumShares][kPGPRecon_MaxPromptLenV1 + 1];


/*	inAuthUser and inAuthPass are not needed if the server class
	is kPGPKeyServerClass_PGP.	*/
	PGPError
PGPNewReconstruct(
	PGPKeyDBObjRef				inTargetKey,
	PGPUTF8						*inAuthUser,		/* can be NULL */
	PGPUTF8						*inAuthPass,		/* can be NULL */
	PGPReconstructEventHandler	inHandler,
	PGPUserValue				inUserValue,
	PGPReconContextRef			*outRef );

/* This variant of PGPNewReconstruct() allows the caller to specify NULL for the
   target key.  This allows reconstruction of keys from local blobs even if we
   do not have a copy of the public key. */
	PGPError
PGPNewReconstructEx(
	PGPContextRef				inContext,
	PGPKeyDBObjRef				inTargetKey,		/* can be NULL */
	PGPUTF8						*inAuthUser,		/* can be NULL */
	PGPUTF8						*inAuthPass,		/* can be NULL */
	PGPReconstructEventHandler	inHandler,
	PGPUserValue				inUserValue,
	PGPReconContextRef			*outRef );

/*	This is only needed if you have to change the event handler after
	allocating the PGPReconContextRef */
	PGPError
PGPSetReconstructionEventHandler(
	PGPReconContextRef			reconRef,
	PGPReconstructEventHandler	inHandler,
	PGPUserValue				inUserValue );
	

/*	I don't think it makes sense to support split keys for reconstruction,
	so we only take a passphrase below */
	PGPError
PGPMakeReconstruction(
	PGPReconContextRef			reconRef,
	PGPReconPrompts 			inPromptInfo,
	PGPReconPasses				inPassInfo,
	PGPUTF8						*inPassphrase );

	PGPError
PGPGetReconstruction(
	PGPReconContextRef			reconRef,
	PGPByte 					**reconData,	/* must be freed by caller */
	PGPSize 					*reconDataSize );

/*	Sends the blob of reconstruction data to an LDAP server or a PGP Keyserver */
	PGPError
PGPSendReconstruction(
	PGPReconContextRef			reconRef );

/*	Extracts the prompts and the number of hash repetitions from a blob of
	reconstruction data */
	PGPError
PGPGetReconstructionPromptsFromData(
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPReconPrompts				outPromptInfo,
	PGPUInt16					*outHashReps );

	PGPError
PGPGetReconstructionPromptsV1FromData(
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPReconPromptsV1			outPromptInfo,
	PGPUInt16					*outHashReps );

/*	Extracts the keyid from a blob of reconstruction data */
	PGPError
PGPGetReconstructionKeyIDFromData(
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPKeyID*					outKeyID );

/*	Downloads the prompts (and the number of hash repetitions internally) from
	an LDAP server or a PGP Keyserver */
	PGPError
PGPGetReconstructionPrompts(
	PGPReconContextRef			reconRef,
	PGPReconPrompts				outPromptInfo );

	PGPError
PGPGetReconstructionPromptsV1(
	PGPReconContextRef			reconRef,
	PGPReconPromptsV1			outPromptInfo );

/*	Create a V1-version blob from a V2 blob.  Caller should free outReconDataV1
    with PGPFreeData() when done. */
	PGPError
PGPMakeReconstructionDataV1FromData(
	PGPContextRef				inContext,
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPByte						**outReconDataV1,
	PGPSize						*outReconDataSizeV1 );

	PGPError
PGPMakeReconstructionPassesHash(
	PGPReconContextRef			reconRef,
	PGPReconPasses				inPassInfo,
	PGPUInt16					hashReps,
	char						armoredPassKey[kPGPRecon_ArmoredHashSize] );

	PGPError
PGPVerifyReconstructionPassesHash(
	PGPReconContextRef			reconRef,
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	const char					armoredPassKey[kPGPRecon_ArmoredHashSize] );

/*	Downloads the blob of key reconstruction data from an LDAP server or a
	PGP Keyserver */
	PGPError
PGPGetReconstructionData(
	PGPReconContextRef			reconRef,
	PGPReconPasses				inPassInfo,
	PGPByte						**outReconData,	/* must be freed by caller */
	PGPSize						*outReconSize );

/*	Reconstructed private key will be returned in
	outReconstructedKey if successful.  The imported
	key will have no passphrase and thus the user must
	then be forced to choose a new passphrase.		*/
	PGPError
PGPReconstruct(
	PGPReconContextRef			reconRef,
	PGPReconPasses				inPassInfo,
	PGPByte						*inReconData,
	PGPSize						inReconDataSize,
	PGPKeyDBRef					*outReconstructedKey );

	PGPError
PGPFreeReconstruct(
	PGPReconContextRef			reconRef );

	PGPError
PGPSetReconstructionServerURL(
	PGPReconContextRef			reconRef,
	PGPUTF8						*pszServerURL,
	PGPKeyServerClass			dwServerType );

/*	Given a reconstruction context, return the PGP context associated
	with it */
	PGPContextRef
PGPGetReconContext(
	PGPReconContextRef			reconRef );

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpReconstruct_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
