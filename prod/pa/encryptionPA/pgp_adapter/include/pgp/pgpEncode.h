/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	This file contains the prototypes for functions which encode/decode files
	and buffers.

	$Id$
____________________________________________________________________________*/

#ifndef Included_pgpEncode_h	/* [ */
#define Included_pgpEncode_h

#include "pgpPubTypes.h"
#include "pgpTLS.h"

#if PGP_MACINTOSH
#pragma options align=mac68k
#endif

/*____________________________________________________________________________
	PGP Events
	
	The PGPEvent structure is used to notify clients of the encode API of
	various events. Each event is denoted by an event type:
____________________________________________________________________________*/

enum PGPEventType_
{
	kPGPEvent_NullEvent			=  0,		/* Nothing happened */
	kPGPEvent_InitialEvent		=  1,		/* Final event */
	kPGPEvent_FinalEvent		=  2,		/* Final event */
	kPGPEvent_ErrorEvent		=  3,		/* An error occurred */
	kPGPEvent_WarningEvent		=  4,		/* Warning event */
	kPGPEvent_EntropyEvent		=  5,		/* More entropy is needed */
	kPGPEvent_PassphraseEvent	=  6,		/* A passphrase is needed */
	kPGPEvent_InsertKeyEvent	=  7,		/* Smart card must be inserted */
	kPGPEvent_AnalyzeEvent		=  8,		/* Initial analysis event,
											   before any output */
	kPGPEvent_RecipientsEvent	=  9,		/* Recipient list report,
											   before any output */
	kPGPEvent_KeyFoundEvent		= 10,		/* Key packet found */
	kPGPEvent_OutputEvent		= 11,		/* Output specification needed */
	kPGPEvent_SignatureEvent	= 12,		/* Signature status report */
	kPGPEvent_BeginLexEvent		= 13,		/* Initial event per lexical unit*/
	kPGPEvent_EndLexEvent		= 14,		/* Final event per lexical unit */
	kPGPEvent_RecursionEvent	= 15,		/* Notification of recursive
											   job creation */
	kPGPEvent_DetachedSignatureEvent = 16,	/* Need input for verification of
											   detached signature */
	kPGPEvent_KeyGenEvent		= 17,		/* Key generation progress */
	
	kPGPEvent_KeyServerEvent	= 18,		/* Key Server progress */
	kPGPEvent_KeyServerSignEvent= 19,		/* Key Server passphrase */
	kPGPEvent_KeyServerTLSEvent	= 20,		/* Key Server TLS event */
	kPGPEvent_KeyServerIdleEvent= 21,		/* Idle during keyserver call */
	
	kPGPEvent_SocketsIdleEvent	= 22,		/* Idle during sockets */
	kPGPEvent_DecryptionEvent	= 23,		/* Decryption data report */
	kPGPEvent_EncryptionEvent	= 24,		/* Encryption data report */

	kPGPEvent_ToBeSignedEvent	= 25,		/* To-be-signed hash */

	PGP_ENUM_FORCE( PGPEventType_ )
};
PGPENUM_TYPEDEF( PGPEventType_, PGPEventType );


/* PGP Analyze event callback codes */

enum PGPAnalyzeType_
{
	kPGPAnalyze_Encrypted			=  0,	/* Encrypted message */
	kPGPAnalyze_Signed				=  1,	/* Signed message */
	kPGPAnalyze_DetachedSignature	=  2,	/* Detached signature */
	kPGPAnalyze_Key					=  3,	/* Key data */
	kPGPAnalyze_Unknown				=  4,	/* Non-pgp message */
	kPGPAnalyze_X509Certificate		=  5,	/* X.509 certificate */
	kPGPAnalyze_SMIMEBody			=  6,	/* SMIME body */

	PGP_ENUM_FORCE( PGPAnalyzeType_ )
};
PGPENUM_TYPEDEF( PGPAnalyzeType_, PGPAnalyzeType );

/* Individual event information structs, combined as a union in PGPEvent */

typedef struct PGPEventNullData_
{
	PGPFileOffset			bytesWritten;
	PGPFileOffset			bytesTotal;
} PGPEventNullData;

typedef struct PGPEventErrorData_
{
	PGPError				error;
	void				   *errorArg;
} PGPEventErrorData;

typedef struct PGPEventWarningData_
{
	PGPError				warning;
	void				   *warningArg;
} PGPEventWarningData;

typedef struct PGPEventEntropyData_
{
	PGPUInt32				entropyBitsNeeded;
} PGPEventEntropyData;

typedef struct PGPEventPassphraseData_
{
	PGPBoolean				fConventional;
	PGPKeySetRef			keyset;
	const PGPByte			*ESKs;
	PGPSize					ESKsLength;
} PGPEventPassphraseData;

typedef struct PGPEventRecipientsData_
{
	PGPKeySetRef			recipientSet;
	PGPUInt32				conventionalPassphraseCount;
	PGPUInt32				keyCount;
	PGPKeyID const *		keyIDArray;
} PGPEventRecipientsData;

typedef struct PGPEventKeyFoundData_
{
	PGPKeyDBRef				keyDB;
} PGPEventKeyFoundData;

typedef struct PGPEventSignatureData_
{
	PGPKeyID				signingKeyID;
	PGPKeyDBObjRef			signingKey;
	PGPBoolean				checked;
	PGPBoolean				verified;
	PGPBoolean				keyRevoked;
	PGPBoolean				keyDisabled;
	PGPBoolean				keyExpired;
	PGPBoolean				keyMeetsValidityThreshold;
	PGPValidity				keyValidity;
	PGPTime					creationTime;
	PGPUInt32				expirationPeriod;
	PGPHashAlgorithm		hashAlgorithm;
	PGPChar8 const *		charset;
} PGPEventSignatureData;

typedef struct PGPEventDecryptionData_
{
	PGPCipherAlgorithm		cipherAlgorithm;
	PGPByte					*sessionKey;
	PGPSize					sessionKeyLength;
	PGPUInt32				keyCount;	/* keyids of keys that can decrypt, */
	PGPKeyID const *		keyIDArray;	/* a subset of keys in PGPEventRecipientsData */
	PGPChar8 const *		charset;
} PGPEventDecryptionData;

typedef struct PGPEventEncryptionData_
{
	PGPCipherAlgorithm		cipherAlgorithm;
	PGPByte					*sessionKey;
	PGPSize					sessionKeyLength;
} PGPEventEncryptionData;

typedef struct PGPEventAnalyzeData_
{
	PGPAnalyzeType			sectionType;
} PGPEventAnalyzeData;

typedef struct PGPEventOutputData_
{
	PGPUInt32				messageType;
	PGPChar8			   *suggestedName;
	PGPBoolean				forYourEyesOnly;
} PGPEventOutputData;

typedef struct PGPEventBeginLexData_
{
	PGPUInt32				sectionNumber;
	PGPSize					sectionOffset;
} PGPEventBeginLexData;

typedef struct PGPEventEndLexData_
{
	PGPUInt32				sectionNumber;
} PGPEventEndLexData;

typedef struct PGPEventKeyGenData_
{
	PGPUInt32				state;
} PGPEventKeyGenData;

typedef struct PGPEventKeyServerData_
{
	PGPKeyServerRef			keyServerRef;
	PGPUInt32				state;			/* PGPKeyServerState */
} PGPEventKeyServerData;

typedef struct PGPEventKeyServerSignData_
{
	PGPKeyServerRef			keyServerRef;
} PGPEventKeyServerSignData;

typedef struct PGPEventKeyServerTLSData_
{
	PGPKeyServerRef			keyServerRef;
	PGPUInt32				state;			/* PGPKeyServerState */
	PGPtlsSessionRef		tlsSession;
} PGPEventKeyServerTLSData;

typedef struct PGPEventKeyServerIdleData_
{
	PGPKeyServerRef			keyServerRef;
} PGPEventKeyServerIdleData;

typedef struct PGPEventToBeSignedData_
{
	PGPKeyID				keyID;
	PGPHashAlgorithm		hashAlg;
	PGPByte					hash[512/8];
	PGPSize					hashSize;
} PGPEventToBeSignedData;

/*
 * The following events have no event-specific data defined for them:
 *	kPGPEvent_InsertKeyEvent
 *	kPGPEvent_RecursionEvent
 *	kPGPEvent_DetachedSignatureEvent
 *	kPGPEvent_InitialEvent
 *	kPGPEvent_FinalEvent
 *	kPGPEvent_SocketsIdleEvent
 */

/* Union of all event data structures above */
typedef union PGPEventData_
{
	PGPEventNullData			nullData;
	PGPEventErrorData			errorData;
	PGPEventWarningData			warningData;
	PGPEventEntropyData			entropyData;
	PGPEventPassphraseData		passphraseData;
	PGPEventRecipientsData		recipientsData;
	PGPEventKeyFoundData		keyFoundData;
	PGPEventSignatureData		signatureData;
	PGPEventDecryptionData		decryptionData;
	PGPEventEncryptionData		encryptionData;
	PGPEventAnalyzeData			analyzeData;
	PGPEventOutputData			outputData;
	PGPEventBeginLexData		beginLexData;
	PGPEventEndLexData			endLexData;
	PGPEventKeyGenData			keyGenData;
	PGPEventKeyServerData		keyServerData;
	PGPEventKeyServerSignData	keyServerSignData;
	PGPEventKeyServerTLSData	keyServerTLSData;
	PGPEventKeyServerIdleData	keyServerIdleData;
	PGPEventToBeSignedData		tbsData;
} PGPEventData;

/* Refs to internal "job" structure */
typedef struct PGPJob *				PGPJobRef;

#define	kInvalidPGPJobRef			((PGPJobRef) NULL)
#define PGPJobRefIsValid( ref )		( (ref) != kInvalidPGPJobRef )

/* PGPEvent structure */

struct PGPEvent
{
	PGPVersion				 version;		/* Version of event structure */
	struct PGPEvent_		*nextEvent;		/* Allow lists of events */
	PGPJobRef				 job;			/* Associated with what job */
	PGPEventType			 type;			/* Type of event */
	PGPEventData			 data;			/* Event specific data */
};
typedef struct PGPEvent PGPEvent;


#if PGP_MACINTOSH
#pragma options align=reset
#endif

PGP_BEGIN_C_DECLARATIONS

/*
**	Functions to encode and decode. The variable parameters are one or more
**	PGPOptionListRef's which describe the inputs, outputs, and options.
*/

PGPError 		PGPEncode(PGPContextRef context,
							PGPOptionListRef firstOption, ...);
PGPError 		PGPDecode(PGPContextRef context,
							PGPOptionListRef firstOption, ...);

PGPError 		PGPAddJobOptions(PGPJobRef job,
							PGPOptionListRef firstOption, ...);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpEncode_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
