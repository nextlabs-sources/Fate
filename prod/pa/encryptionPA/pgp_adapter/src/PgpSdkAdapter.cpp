/*
Written by Derek Zheng
March 2008
*/

#pragma once

#include "stdafx.h"

#define GSM_ERR_SOURCE_DEFAULT	GSM_ERR_SOURCE_PGP

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <string>

#ifndef WIN32
#include <unistd.h>
#else 
#include <direct.h>
#include <io.h>
#endif

#include "log.h"
#include "passcache.h"
#include "PgpSdkAdapter.h"

#define FAIL(_msg)  {\
	NL_Trace( __FILE__, __LINE__,__FUNCTION__, _msg); \
	err = kPGPError_SelfTestFailed; \
	goto done; }

#ifdef _DEBUG

#define PGP_NEED_TRACE_FUNC

#define CKNULL(_p) if(NULL == (_p)) {\
	NL_Trace( __FILE__, __LINE__,__FUNCTION__,"Failed with Null value"); \
	err = kPGPError_OutOfMemory; \
	goto done; }

#define CKERR  if(IsPGPError(err)) {\
	char _errstr[256];				\
	m_PgpSdkFunc.fPGPGetErrorString( err, 256, _errstr); \
	NL_Trace( __FILE__, __LINE__,__FUNCTION__,"Failed with PGPError %d %s", err, _errstr); \
	goto done; }


#else

#define CKNULL(_p) if(NULL == (_p)) {\
	err = kPGPError_OutOfMemory; \
	goto done; }

#define CKERR  if(IsPGPError(err)) {\
	goto done; }

#endif

// keyserver operations
#define GETVKDKEY					0x01
#define SENDKEY						0x02
#define	DISABLEKEY					0x03
#define DELETEKEY					0x04

typedef struct
{
	GsmKeyServerStateChangedCallbackF cb;
	void *cbData;
} GsmKeyServerStateCbData;

///////////////////////////////////////////////////////////////
static GsmErrorT ConvertPgpError2GsmError(PGPError err);
static int makeDirectory(LPCTSTR dirname);
static PGPError genKeyEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
static void FormatFingerprintString(char *p, PGPByte *inBuffp, PGPSize len );
//static LPCTSTR hash_algor_table(int algor);
static LPCTSTR cipher_algor_table(int algor);
//static LPCTSTR key_algor_table(int keytype);
//static LPCTSTR compression_algor_table(int algor);

#if 0
static PGPError sdaEventHandlerProcPtr(PGPContextRef context,
	struct PGPsdaEvent *event,
	PGPUserValue userValue);

static PGPError sdaDecodeEventHandlerProc(PGPContextRef context,
	struct PGPsdaEvent *event,
	PGPUserValue userValue);
#endif

///////////////////////////////////////////////////////////

GsmErrorT CPgpSdkAdapter::Init( LPCTSTR keyRingPath )
{
	PGPError			err = kPGPError_NoErr;
	PGPFileSpecRef 		pubKeysFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef		privKeysFileSpec 	= kInvalidPGPFileSpecRef;
	PGPUInt32			numKeys = 0;
	LPTSTR				pszDir = (LPTSTR)keyRingPath;

	if (m_Inited)
	{
		return 0;
	}

	m_Inited = 0;

	if (!m_PgpSdkFunc.InitializePgpSdk())
	{
		return GsmErrorMake(GSM_ERR_NO_DLL);
	}

	if (pszDir)
	{
		if (pszDir[wcslen(pszDir)] == _T('/') || pszDir[wcslen(pszDir)] == _T('\\'))
		{
			pszDir[wcslen(pszDir)] = 0;
		}
		if (makeDirectory(pszDir))
		{
			return GsmErrorMake(GSM_ERR_CREATE_PATH);
		}
	}
	else
	{
		pszDir = _T(".");
	}

	m_pszPubKeyRingPath = (LPTSTR)malloc((wcslen(pszDir) + wcslen(PGP_SDK_PUB_KEY_RING_NAME)+16)*sizeof(TCHAR));
	if (!m_pszPubKeyRingPath)
	{
		return GsmErrorMake(GSM_ERR_NO_RESOURCE);
	}

	m_pszSecKeyRingPath = (LPTSTR)malloc((wcslen(pszDir) + wcslen(PGP_SDK_SEC_KEY_RING_NAME)+16)*sizeof(TCHAR));
	if (!m_pszSecKeyRingPath)
	{
		free(m_pszPubKeyRingPath);
		m_pszPubKeyRingPath = NULL;
		return GsmErrorMake(GSM_ERR_NO_RESOURCE);
	}

	_snwprintf_s(m_pszPubKeyRingPath, wcslen(pszDir) + wcslen(PGP_SDK_PUB_KEY_RING_NAME)+16, _TRUNCATE, _T("%s\\%s"), pszDir, PGP_SDK_PUB_KEY_RING_NAME);
	_snwprintf_s(m_pszSecKeyRingPath, wcslen(pszDir) + wcslen(PGP_SDK_SEC_KEY_RING_NAME)+16, _TRUNCATE, _T("%s\\%s"), pszDir, PGP_SDK_SEC_KEY_RING_NAME);

	err = m_PgpSdkFunc.fPGPsdkInit( kPGPFlags_ForceLocalExecution | kPGPFlags_SuppressCacheThread ); CKERR;
	
	m_PgpSdkFunc.fPGPGetPGPsdkVersionString((PGPChar16 *)m_szVersion);
	
	PGPFlags flags;
	err = m_PgpSdkFunc.fPGPGetFeatureFlags(kPGPFeatures_ImplementationSelector, &flags); CKERR;
	if (PGPFeatureExists(flags, kPGPFeatureMask_IsDebugBuild))
	{
		DP((_T("PGP SDK version %s debug build\n"), m_szVersion));
	}

	err = m_PgpSdkFunc.fPGPNewContext( kPGPsdkAPIVersion, &m_context ); CKERR;

	m_PgpSdkFunc.fPGPsdkSetLanguage(NULL, kPGPLanguage_Default);

	//err = ConsoleAcquireEntropy(m_context, m_PgpSdkFunc.fPGPGlobalRandomPoolGetMinimumEntropy()/8, NULL, FALSE); CKERR;

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)m_pszPubKeyRingPath, &pubKeysFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)m_pszSecKeyRingPath, &privKeysFileSpec); CKERR;

	/* Open KeyRing */
	err = m_PgpSdkFunc.fPGPOpenKeyDBFile( m_context,
		PGPOpenKeyDBFileOptions(kPGPOpenKeyDBFileOptions_Create | kPGPOpenKeyDBFileOptions_Mutable),
		pubKeysFileSpec,
		privKeysFileSpec,
		&m_keyDB ); CKERR;
	//if(!m_PgpSdkFunc.fPGPKeyDBIsMutable(m_keyDB)) FAIL("KeyDB was not open mutable\n");

	err = m_PgpSdkFunc.fPGPCountKeysInKeyDB(m_keyDB, &numKeys); CKERR;
	DP((_T("CPgpSdkAdapter::Init: keyDB has %d keys\n"), numKeys));
	
	err = m_PgpSdkFunc.fPGPsdkNetworkLibInit(kPGPFlags_ForceLocalExecution | kPGPFlags_SuppressCacheThread); CKERR;
	err = m_PgpSdkFunc.fPGPKeyServerInit(); CKERR;

	passcache_initialize();

	m_Inited = 1;

done:
	if( PGPFileSpecRefIsValid(pubKeysFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(pubKeysFileSpec);

	if( PGPFileSpecRefIsValid(privKeysFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(privKeysFileSpec);

	if (IsPGPError(err))
	{
		if( PGPKeyDBRefIsValid( m_keyDB ) )
		{
			m_PgpSdkFunc.fPGPFreeKeyDB( m_keyDB );
		}

		if (PGPContextRefIsValid( m_context ))
		{
			m_PgpSdkFunc.fPGPFreeContext(m_context);
		}
		m_PgpSdkFunc.fPGPsdkCleanup();

		if (m_pszPubKeyRingPath)
		{
			free(m_pszPubKeyRingPath);
			m_pszPubKeyRingPath = NULL;
		}

		if (m_pszSecKeyRingPath)
		{
			free(m_pszSecKeyRingPath);
			m_pszSecKeyRingPath = NULL;
		}
	}

	return ConvertPgpError2GsmError(err);
}



void CPgpSdkAdapter::Release( void )
{
	if (!m_Inited)
	{
		return;
	}

	m_Inited = 0;

	passcache_flushall();

	m_PgpSdkFunc.fPGPKeyServerCleanup();
	m_PgpSdkFunc.fPGPsdkNetworkLibCleanup();

	if( PGPKeyDBRefIsValid( m_keyDB ) )
	{
		m_PgpSdkFunc.fPGPFlushKeyDB(m_keyDB);
		m_PgpSdkFunc.fPGPFreeKeyDB( m_keyDB );
	}

	if (PGPContextRefIsValid( m_context ))
	{
		m_PgpSdkFunc.fPGPFreeContext(m_context);
	}
	m_PgpSdkFunc.fPGPsdkCleanup();

	if (m_pszPubKeyRingPath)
	{
		free(m_pszPubKeyRingPath);
		m_pszPubKeyRingPath = NULL;
	}

	if (m_pszSecKeyRingPath)
	{
		free(m_pszSecKeyRingPath);
		m_pszSecKeyRingPath = NULL;
	}
}

void CPgpSdkAdapter::SetPassCacheTTL(unsigned long ttl)
{
	m_ulTTL = ttl;
}

PGPError CPgpSdkAdapter::ConsoleAcquireEntropy(PGPContextRef 			context,
							   PGPUInt32				entropyNeeded,
							   PGPUInt32 *				pEntropyAcquired,
							   PGPBoolean				bOutputProgress)
{
	PGPError				err						= kPGPError_NoErr;
	PGPUInt32				entropyAcquired			= 0;
	time_t					start					= 0;
	time_t					current					= 0;


	if (entropyNeeded == 0)
		return (kPGPError_NoErr);

	if (bOutputProgress)
		fprintf(stdout, "\t%d more bits needed\n\tCollecting from system state",
			entropyNeeded - entropyAcquired);

	time (&start);
	while (entropyAcquired < entropyNeeded)
	{
		entropyAcquired += m_PgpSdkFunc.fPGPContextReserveRandomBytes (context, entropyNeeded);
		err = m_PgpSdkFunc.fPGPGlobalRandomPoolAddSystemState (); CKERR;

		if (bOutputProgress)
			fprintf(stdout, "."),fflush(stdout);

		time (&current);
		if (current > (start + 100))
			break;
	}

	if (bOutputProgress)
		fprintf(stdout, " OK\n");

	if (entropyAcquired < entropyNeeded)
		return (kPGPError_OutOfEntropy);

done:
	if (IsntPGPError (err))
	{
		if (pEntropyAcquired)
			*pEntropyAcquired = entropyAcquired;
	}

	return (err);
}



// Key Management
#define kPGPKeyPropertyFlags_None 0
GsmErrorT CPgpSdkAdapter::GenerateKey(GsmKeyGenParam &param, GsmProcessCallbackF processCb, void *context, GsmKeyID *keyId)
{
	PGPError			err			= kPGPError_NoErr;
	PGPKeyDBObjRef		key 		= kInvalidPGPKeyDBObjRef;
	PGPKeyDBObjRef		encryptionSubKey 	= kInvalidPGPKeyDBObjRef;
	PGPKeyDBRef			newKeyDB				= kInvalidPGPKeyDBRef;
	PGPOptionListRef	optionList				= kInvalidPGPOptionListRef;
	PGPOptionListRef	optionListEntropy		= kInvalidPGPOptionListRef;
	PGPOptionListRef	optionSubList				= kInvalidPGPOptionListRef;
	PGPOptionListRef	optionSubListEntropy		= kInvalidPGPOptionListRef;

	PGPPublicKeyAlgorithm	keyType		= kPGPPublicKeyAlgorithm_Invalid;
	PGPUInt32				keySize	 	= 0;
	PGPUInt32				keyFlags				= kPGPKeyPropertyFlags_None;
	PGPPublicKeyAlgorithm	encryptionSubKeyType	= kPGPPublicKeyAlgorithm_Invalid;
	PGPUInt32				encryptionSubKeySize	= 0;
	PGPUInt32				encryptionSubkeyFlags	= kPGPKeyPropertyFlags_None;
	TCHAR					szUserId[256];
	PGPChar					keyIDString[kPGPMaxKeyIDStringSize];
	PgpProcessCallbackData	cbData;
	PGPKeyID				pgpKeyID;
	PGPCipherAlgorithm 		preferredCiphers[] 
		= {kPGPCipherAlgorithm_AES256, kPGPCipherAlgorithm_AES192, kPGPCipherAlgorithm_AES128,
		kPGPCipherAlgorithm_CAST5, kPGPCipherAlgorithm_3DES, kPGPCipherAlgorithm_IDEA, kPGPCipherAlgorithm_Twofish256};
	PGPCompressionAlgorithm preferredAlgorithms[] 
		= {kPGPCompressionAlgorithm_ZLIB, kPGPCompressionAlgorithm_BZIP2, 
			kPGPCompressionAlgorithm_ZIP, kPGPCompressionAlgorithm_None};
	PGPHashAlgorithm preferredHashes[]
		= {kPGPHashAlgorithm_SHA256, kPGPHashAlgorithm_SHA384, kPGPHashAlgorithm_SHA512, 
		kPGPHashAlgorithm_SHA, kPGPHashAlgorithm_RIPEMD160, kPGPHashAlgorithm_MD5};

	assert(NULL != processCb);

	if (NULL != keyId)
	{
		memset(keyId, 0, sizeof(GsmKeyID));
	}

	if (!m_Inited)
	{
		return GsmErrorMake(GSM_ERR_SDK_UNINTIALISED);
	}

	cbData.cb = processCb;
	cbData.context = context;

	if (!m_Inited)
	{
		return GsmErrorMake(GSM_ERR_SDK_UNINTIALISED);
	}

	if (!param.m_pszUser || !param.m_szEmail || !param.m_pszPassphrase)
	{
		return GsmErrorMake(GSM_ERR_INV_PARAMETER);
	}

	/* don't support signingSubKeyType temporarily */
	switch (param.m_iKeyGenType)
	{
	case GSM_KEYGEN_DSA_ELG:
		keyType = kPGPPublicKeyAlgorithm_DSA;
		keyFlags = kPGPKeyPropertyFlags_UsageSign;
		keySize = 1024;
		encryptionSubKeyType = kPGPPublicKeyAlgorithm_ElGamal;
		encryptionSubkeyFlags = kPGPKeyPropertyFlags_UsageEncrypt;
		encryptionSubKeySize = param.m_iKeyBits;
		break;
	case GSM_KEYGEN_DSA_SIG:
		keyType = kPGPPublicKeyAlgorithm_DSA;
		keyFlags = kPGPKeyPropertyFlags_UsageSign;
		keySize = param.m_iKeyBits;
		encryptionSubKeyType = kPGPPublicKeyAlgorithm_Invalid;
		encryptionSubKeySize = 0;
		break;
	case GSM_KEYGEN_RSA_SIG:
		keyType = kPGPPublicKeyAlgorithm_RSA;
		keyFlags = kPGPKeyPropertyFlags_UsageSign;
		keySize = param.m_iKeyBits;
		encryptionSubKeyType = kPGPPublicKeyAlgorithm_Invalid;
		encryptionSubKeySize = 0;
		break;
	case GSM_KEYGEN_RSA:
		keyType = kPGPPublicKeyAlgorithm_RSA;
		keyFlags = 0;
		keySize = param.m_iKeyBits;
		encryptionSubKeyType = kPGPPublicKeyAlgorithm_Invalid;
		encryptionSubKeySize = 0;
		break;
	case GSM_KEYGEN_RSA_RSA:
		keyType = kPGPPublicKeyAlgorithm_RSA;
		keyFlags = kPGPKeyPropertyFlags_UsageSign;
		keySize = param.m_iKeyBits;
		encryptionSubKeyType = kPGPPublicKeyAlgorithm_RSA;
		encryptionSubkeyFlags = kPGPKeyPropertyFlags_UsageEncrypt;
		encryptionSubKeySize = param.m_iKeyBits;
		break;
	default:
		return GsmErrorMake(GSM_ERR_PUBKEY_ALGO);
	}

	if (!param.m_pszComment)
	{
		_snwprintf_s(szUserId, _countof(szUserId), _TRUNCATE, _T("%s <%s>"), param.m_pszUser, param.m_szEmail);
	}
	else
	{
		_snwprintf_s(szUserId, _countof(szUserId), _TRUNCATE, _T("%s (%s) <%s>"), param.m_pszUser, param.m_pszComment, param.m_szEmail);
	}

	err = m_PgpSdkFunc.fPGPNewKeyDB (m_context, &newKeyDB); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;
	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionListEntropy); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOKeyDBRef (m_context, newKeyDB),
		m_PgpSdkFunc.fPGPOKeyGenName (m_context, szUserId, wcslen(szUserId)*sizeof(TCHAR)),
		m_PgpSdkFunc.fPGPOPassphrase (m_context, (PGPChar16 *)param.m_pszPassphrase),
		m_PgpSdkFunc.fPGPOCachePassphrase (m_context, ~(PGPUInt32)0, TRUE), // this has a tricky warning on 64 bit because kPGPMaxTimeInterval is 0xffffffff (32 bit), not 0xffffffffffffffff (64 bit).  I think this is acceptable, as long as PGPOCachePassphrase treats 0xffffffff (32 bit) as max.
		m_PgpSdkFunc.fPGPOExpiration (m_context, param.m_uiExpireDays),
		m_PgpSdkFunc.fPGPOEventHandler (m_context, genKeyEventHandler, (PGPUserValue)&cbData),
		//m_PgpSdkFunc.fPGPOKeyGenUseExistingEntropy (m_context, FALSE),
		m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

	if (param.m_iKeyGenType != GSM_KEYGEN_RSA)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
			m_PgpSdkFunc.fPGPOPreferredAlgorithms (m_context, preferredCiphers, sizeof(preferredCiphers)),
			m_PgpSdkFunc.fPGPOPreferredCompressionAlgorithms (m_context, preferredAlgorithms, sizeof(preferredAlgorithms)),
			m_PgpSdkFunc.fPGPOPreferredHashAlgorithms (m_context, preferredHashes, sizeof(preferredHashes)),
			m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;
	}

	err = m_PgpSdkFunc.fPGPAppendOptionList (optionListEntropy,
		m_PgpSdkFunc.fPGPOKeyGenParams (m_context, keyType, keySize),
		m_PgpSdkFunc.fPGPOKeyGenFast (m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

	if (keyFlags)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList (optionListEntropy,
			m_PgpSdkFunc.fPGPOKeyFlags (m_context, keyFlags),
			m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;
	}

	/* Check for sufficient random bits */
	//entropyNeeded = m_PgpSdkFunc.fPGPGetKeyEntropyNeeded (m_context, optionListEntropy, m_PgpSdkFunc.fPGPOLastOption (m_context));

	//err = ConsoleAcquireEntropy(m_context, entropyNeeded, NULL, TRUE); CKERR;

	DP((_T("\tGenerate Main Key (Encrypt and Sign)(UserId=%s) \n"), szUserId));
	err = m_PgpSdkFunc.fPGPGenerateKey( m_context, &key,
		optionList, optionListEntropy,
		/* must be terminated with this though */
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	DPA(("done!\n"));

	err = m_PgpSdkFunc.fPGPGetKeyID (key, &pgpKeyID); CKERR;
	err = m_PgpSdkFunc.fPGPGetKeyIDString (&pgpKeyID, kPGPKeyIDString_Abbreviated, keyIDString); CKERR;
	
	/* cache the passphrase */
	passcache_put((LPCTSTR)keyIDString, param.m_pszPassphrase, m_ulTTL);

	if (keyId)
	{
		wcsncpy_s(keyId->szKey, 36, (LPCTSTR)keyIDString, _TRUNCATE);
	}

	if (encryptionSubKeyType != kPGPPublicKeyAlgorithm_Invalid)
	{
		err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionSubList); CKERR;
		err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionSubListEntropy); CKERR;

		err = m_PgpSdkFunc.fPGPAppendOptionList (optionSubList,
			m_PgpSdkFunc.fPGPOKeyGenMasterKey (m_context, key),
			m_PgpSdkFunc.fPGPOPassphrase (m_context, (PGPChar16 *)param.m_pszPassphrase),
			//m_PgpSdkFunc.fPGPOCachePassphrase (m_context, kPGPMaxTimeInterval, TRUE),
			m_PgpSdkFunc.fPGPOExpiration (m_context, param.m_uiExpireDays),
			m_PgpSdkFunc.fPGPOEventHandler (m_context, genKeyEventHandler, (PGPUserValue)&cbData),
			//m_PgpSdkFunc.fPGPOKeyGenUseExistingEntropy (m_context, FALSE),
			m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

		err = m_PgpSdkFunc.fPGPAppendOptionList (optionSubListEntropy,
			m_PgpSdkFunc.fPGPOKeyGenParams (m_context, encryptionSubKeyType, encryptionSubKeySize),
			m_PgpSdkFunc.fPGPOKeyGenFast (m_context, TRUE),
			m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

		if (encryptionSubkeyFlags)
		{
			err = m_PgpSdkFunc.fPGPAppendOptionList (optionSubListEntropy,
				m_PgpSdkFunc.fPGPOKeyFlags (m_context, encryptionSubkeyFlags),
				m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;
		}

		//entropyNeeded = m_PgpSdkFunc.fPGPGetKeyEntropyNeeded (m_context, optionSubListEntropy, m_PgpSdkFunc.fPGPOLastOption (m_context));
		//err = ConsoleAcquireEntropy (m_context, entropyNeeded, NULL, TRUE); CKERR;

		DPA(("\tGenerate Encryption Sub Key \n"));
		err = m_PgpSdkFunc.fPGPGenerateSubKey (m_context, &encryptionSubKey, 
			optionSubList, optionSubListEntropy, 
			m_PgpSdkFunc.fPGPOLastOption (m_context)); /* CKERR */
		DPA(("done!\n"));

		err = m_PgpSdkFunc.fPGPGetKeyID (encryptionSubKey, &pgpKeyID); CKERR;
		err = m_PgpSdkFunc.fPGPGetKeyIDString (&pgpKeyID, kPGPKeyIDString_Abbreviated, keyIDString); CKERR;

		/* cache the passphrase */
		passcache_put((LPCTSTR)keyIDString, param.m_pszPassphrase, m_ulTTL);
	}

	/* If everything worked, copy the new key to the destination key DB */
	err = m_PgpSdkFunc.fPGPCopyKeys (m_PgpSdkFunc.fPGPPeekKeyDBRootKeySet (newKeyDB), m_keyDB, NULL); CKERR;
	m_PgpSdkFunc.fPGPFlushKeyDB(m_keyDB);
		
done:
	if (PGPOptionListRefIsValid (optionSubList))
		m_PgpSdkFunc.fPGPFreeOptionList (optionSubList);
	if (PGPOptionListRefIsValid (optionSubListEntropy))
		m_PgpSdkFunc.fPGPFreeOptionList (optionSubListEntropy);

	if (PGPOptionListRefIsValid (optionListEntropy))
		m_PgpSdkFunc.fPGPFreeOptionList (optionListEntropy);
	if (PGPOptionListRefIsValid (optionList))
		m_PgpSdkFunc.fPGPFreeOptionList (optionList);

	if (PGPKeyDBRefIsValid (newKeyDB))
		m_PgpSdkFunc.fPGPFreeKeyDB (newKeyDB);

	return ConvertPgpError2GsmError(err);
}
GsmErrorT CPgpSdkAdapter::DeleteKey(GsmKeyHandle handle)
{
	PGPError err = kPGPError_NoErr;

	err = m_PgpSdkFunc.fPGPDeleteKeyDBObj((PGPKeyDBObjRef)handle);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::DeleteKey(GsmKeyID keyId)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	GsmErrorT			gsmErr;
	
	gsmErr = FindKeyByKeyID(keyId, (GsmKeyHandle *)&key);
	if (gsmErr)
	{
		return gsmErr;
	}

	err = m_PgpSdkFunc.fPGPDeleteKeyDBObj(key); CKERR;

done:

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::RevokeKey(GsmKeyID keyId, LPCTSTR passphrase)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	GsmErrorT			gsmErr;

	assert(NULL != passphrase);

	gsmErr = FindKeyByKeyID(keyId, (GsmKeyHandle *)&key);
	if (gsmErr)
	{
		return gsmErr;
	}

	err = m_PgpSdkFunc.fPGPRevoke(key, 
		m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)passphrase ),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:

	return ConvertPgpError2GsmError(err);
}

// All key handle should be released by ReleaseKey
void CPgpSdkAdapter::ReleaseKey(GsmKeyHandle handle)
{
	UNUSED(handle);
	return;
}

void CPgpSdkAdapter::ReleaseKeyArray(GsmKeyHandle *hKeys, int nKeys)
{
	UNUSED(nKeys);
	if (hKeys)
	{
		free(hKeys);
	}

	return;
}

GsmErrorT CPgpSdkAdapter::EnumAllKeys(GsmKeyHandle **hKeys, int *nKey, int keyType)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	PGPKeySetRef		keyset	= kInvalidPGPKeySetRef;
	PGPKeyListRef		keyList		= kInvalidPGPKeyListRef;
	PGPKeyIterRef		iter	 	= kInvalidPGPKeyIterRef;
	int					keyCount = 0;
	PGPBoolean			bProp = FALSE;
	PGPUInt32			numKeys = 0;

	assert(NULL != hKeys && NULL != nKey);

	*hKeys = NULL;
	*nKey = 0;

	err = m_PgpSdkFunc.fPGPNewKeySet(m_keyDB, &keyset); CKERR;
	err = m_PgpSdkFunc.fPGPCountKeys(keyset, &numKeys); CKERR;
	if (!numKeys)
	{
		goto done;
	}

	/* Check KeyRing Sigs */
	/* NOTE: We must perform the KeyRing Sig check for expiration dates to work */
	err = m_PgpSdkFunc.fPGPCheckKeyRingSigs(keyset, m_keyDB, TRUE, NULL,NULL  );CKERR;

	/* Create a key list in  the set */
	err = m_PgpSdkFunc.fPGPOrderKeySet( keyset, kPGPKeyOrdering_KeyID, TRUE, &keyList ); CKERR;

	/* make an iterator */
	err = m_PgpSdkFunc.fPGPNewKeyIter( keyList, &iter); CKERR;

	*hKeys = (GsmKeyHandle *)calloc (numKeys, sizeof *hKeys); CKNULL(*hKeys);

	for(keyCount = 0; IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( iter, kPGPKeyDBObjType_Key, &key) ); )
	{
		if (keyType == GSM_KEY_TYPE_ENCRYPT || keyType == GSM_KEY_TYPE_SIGN)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsExpired, &bProp);
			if (bProp)
			{
				continue;
			}

			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsRevoked, &bProp);
			if (bProp)
			{
				continue;
			}

			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsDisabled, &bProp);
			if (bProp)
			{
				continue;
			}
		}

		if (keyType == GSM_KEY_TYPE_SIGN)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_CanSign, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_ENCRYPT)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_CanEncrypt, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_PUBLIC)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsSecret, &bProp);
			if (!bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_SECRET)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsSecret, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else
		{
			(*hKeys)[keyCount] = (GsmKeyHandle)key;
			keyCount++;
		}
	}

	*nKey = keyCount;

done:
	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	if( PGPKeyListRefIsValid (keyList) )
		m_PgpSdkFunc.fPGPFreeKeyList(keyList);

	if( PGPKeySetRefIsValid (keyset) )
		m_PgpSdkFunc.fPGPFreeKeySet(keyset);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::FindKeyByKeyID(GsmKeyID keyId, GsmKeyHandle *handle)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	PGPKeyID			pgpKeyID;

	assert(NULL != handle);

	*handle = NULL;

	err = m_PgpSdkFunc.fPGPNewKeyIDFromString((const PGPChar *)keyId.szKey, kPGPPublicKeyAlgorithm_Invalid, &pgpKeyID); CKERR;

	err = m_PgpSdkFunc.fPGPFindKeyByKeyID(m_keyDB, &pgpKeyID, &key); CKERR;

	*handle = (GsmKeyHandle)key;

done:

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::FindKeyByKeyFpr(LPCTSTR fpr, GsmKeyHandle *handle)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	PGPUInt32			numKeys = 0;
	PGPKeySetRef		keyset	= kInvalidPGPKeySetRef;
	PGPKeyListRef		keyList		= kInvalidPGPKeyListRef;
	PGPKeyIterRef		iter	 	= kInvalidPGPKeyIterRef;
	PGPFilterRef		filter		= kInvalidPGPFilterRef;

	assert(NULL != fpr && NULL != handle);

	*handle = NULL;

	/* create a search filter */
	err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter(m_context,  kPGPKeyProperty_Fingerprint,
		fpr, wcslen(fpr)*sizeof(TCHAR),
		kPGPMatchCriterion_Equal, &filter); CKERR;

	/* search for keys */
	err = m_PgpSdkFunc.fPGPFilterKeyDB(m_keyDB, filter, &keyset); CKERR;

	/* how many did we find ?*/
	err = m_PgpSdkFunc.fPGPCountKeys(keyset, &numKeys); CKERR;
	DP((_T("FindKeyByKeyFpr(fpr=%s) match %d keys\n"), fpr, numKeys));
	if(numKeys == 0)
	{
		err = kPGPError_ItemNotFound;
		goto done;
	}

	/* Check KeyRing Sigs */
	/* NOTE: We must perform the KeyRing Sig check for expiration dates to work */
	err = m_PgpSdkFunc.fPGPCheckKeyRingSigs(keyset, m_keyDB, TRUE, NULL,NULL  );CKERR;

	/* Create a key list in  the set */
	err = m_PgpSdkFunc.fPGPOrderKeySet( keyset, kPGPKeyOrdering_KeyID, TRUE, &keyList ); CKERR;

	/* make an iterator */
	err = m_PgpSdkFunc.fPGPNewKeyIter( keyList, &iter); CKERR;

	for(; IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( iter, kPGPKeyDBObjType_Key, &key) );)
	{
		TCHAR		tempBuf[256];
		PGPKeyID	tempID;

		m_PgpSdkFunc.fPGPGetKeyID(key, &tempID);
		m_PgpSdkFunc.fPGPGetKeyIDString( &tempID, kPGPKeyIDString_Abbreviated, (PGPChar16 *)tempBuf);
		DP((_T("FindKeyByKeyFpr(fpr=%s) match keyID %s\n"), fpr, tempBuf));
		*handle = (GsmKeyHandle)key;
	}

done:
	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	if( PGPKeyListRefIsValid (keyList) )
		m_PgpSdkFunc.fPGPFreeKeyList(keyList);

	if( PGPKeySetRefIsValid (keyset) )
		m_PgpSdkFunc.fPGPFreeKeySet(keyset);

	if( PGPFilterRefIsValid( filter ) )
		m_PgpSdkFunc.fPGPFreeFilter( filter );

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::FindKeyByUserID(LPCTSTR pszUserId, GsmKeyHandle **hKeys, int *nKey, int keyType, int bSubString)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyDBObjRef		key = kInvalidPGPKeyDBObjRef;
	PGPUInt32			numKeys = 0;
	PGPKeySetRef		keyset	= kInvalidPGPKeySetRef;
	PGPKeyListRef		keyList		= kInvalidPGPKeyListRef;
	PGPKeyIterRef		iter	 	= kInvalidPGPKeyIterRef;
	PGPFilterRef		filter		= kInvalidPGPFilterRef;
	PGPFilterRef		filter1		= kInvalidPGPFilterRef;
	PGPFilterRef		filter2		= kInvalidPGPFilterRef;
	int					keyCount = 0;
	PGPBoolean			bProp = FALSE;
	PGPMatchCriterion	matchCriterion = kPGPMatchCriterion_SubString;

	assert(NULL != pszUserId && NULL != hKeys && NULL != nKey);

	*hKeys = NULL;
	*nKey = 0;

	if (bSubString)
	{
		matchCriterion = kPGPMatchCriterion_SubString;
	}
	else
	{
		matchCriterion = kPGPMatchCriterion_Equal;
	}

	/* create a search filter */
	err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter(m_context,  kPGPUserIDProperty_Name,
		pszUserId, wcslen(pszUserId)*sizeof(TCHAR),
		matchCriterion, &filter); CKERR;
	err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter(m_context,  kPGPUserIDProperty_AttributeData,
		pszUserId, wcslen(pszUserId)*sizeof(TCHAR),
		matchCriterion, &filter1); CKERR;
	err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter(m_context,  kPGPUserIDProperty_EmailAddress,
		pszUserId, wcslen(pszUserId)*sizeof(TCHAR),
		matchCriterion, &filter2); CKERR;

	err = m_PgpSdkFunc.fPGPUnionFilters(filter,filter1, &filter); CKERR;
	filter1 = kInvalidPGPFilterRef;
	err = m_PgpSdkFunc.fPGPUnionFilters(filter,filter2, &filter); CKERR;
	filter2 = kInvalidPGPFilterRef;

	/* search for keys */
	err = m_PgpSdkFunc.fPGPFilterKeyDB(m_keyDB, filter, &keyset); CKERR;

	err = m_PgpSdkFunc.fPGPCountKeys(keyset, &numKeys); CKERR;
	if (!numKeys)
	{
		err = kPGPError_ItemNotFound;
		goto done;
	}

	/* Check KeyRing Sigs */
	/* NOTE: We must perform the KeyRing Sig check for expiration dates to work */
	err = m_PgpSdkFunc.fPGPCheckKeyRingSigs(keyset, m_keyDB, TRUE, NULL,NULL  );CKERR;

	/* Create a key list in  the set */
	err = m_PgpSdkFunc.fPGPOrderKeySet( keyset, kPGPKeyOrdering_KeyID, TRUE, &keyList ); CKERR;

	/* make an iterator */
	err = m_PgpSdkFunc.fPGPNewKeyIter( keyList, &iter); CKERR;

	*hKeys = (GsmKeyHandle *)calloc (numKeys, sizeof *hKeys); CKNULL(*hKeys);

	for(keyCount = 0; IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( iter, kPGPKeyDBObjType_Key, &key) ); )
	{
		m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsExpired, &bProp);
		if (bProp)
		{
			continue;
		}

		if (keyType == GSM_KEY_TYPE_SIGN)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_CanSign, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_ENCRYPT)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_CanEncrypt, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_PUBLIC)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsSecret, &bProp);
			if (!bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else if (keyType == GSM_KEY_TYPE_SECRET)
		{
			m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsSecret, &bProp);
			if (bProp)
			{
				(*hKeys)[keyCount] = (GsmKeyHandle)key;
				keyCount++;
			}
		}
		else
		{
			(*hKeys)[keyCount] = (GsmKeyHandle)key;
			keyCount++;
		}
	}

	*nKey = keyCount;

done:
	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	if( PGPKeyListRefIsValid (keyList) )
		m_PgpSdkFunc.fPGPFreeKeyList(keyList);

	if( PGPKeySetRefIsValid (keyset) )
		m_PgpSdkFunc.fPGPFreeKeySet(keyset);

	if( PGPFilterRefIsValid( filter ) )
		m_PgpSdkFunc.fPGPFreeFilter( filter );

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::GetKeyID(GsmKeyHandle handle, GsmKeyID *keyId)
{
	PGPError			err = kPGPError_NoErr;
	PGPKeyID			pgpKeyID;
	PGPChar	keyIDString[kPGPMaxKeyIDStringSize]; memset(keyIDString, 0, sizeof(keyIDString));

	assert(NULL != keyId && NULL != handle);

	memset((void *)keyId->szKey, 0, 36);
	err = m_PgpSdkFunc.fPGPGetKeyID ((PGPKeyDBObjRef)handle, &pgpKeyID); CKERR;
	err = m_PgpSdkFunc.fPGPGetKeyIDString (&pgpKeyID, kPGPKeyIDString_Abbreviated, keyIDString); CKERR;
	wcsncpy_s(keyId->szKey, 36, (LPCTSTR)keyIDString, _TRUNCATE);

done:
	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::GetKeyDataProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, void *buffer, size_t bufferSize, size_t *dataSize)
{
	PGPError			err = kPGPError_NoErr;

	if (whichProp == GsmKeyProperty_Fingerprint)
	{
		//PGPByte data[512];
		PGPSize dataLen;
		PGPChar data[512];

		memset(data, 0, 64);

		err = m_PgpSdkFunc.fPGPGetKeyDBObjDataProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, data, 512*sizeof(PGPChar), &dataLen); CKERR;
		if (dataSize)
		{
			*dataSize = dataLen;
		}
		if (buffer)
		{
			char szTmpBuffer[512];
			char szTmpBuffer2[512];
			memset(szTmpBuffer, 0, 512);
			memset(szTmpBuffer2, 0, 512);
			WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)data, -1, szTmpBuffer, 512, NULL, NULL);
			FormatFingerprintString((char *)szTmpBuffer2, (PGPByte *)szTmpBuffer, strlen(szTmpBuffer));
			MultiByteToWideChar(CP_ACP, 0, szTmpBuffer2, -1, (LPWSTR)buffer, (int)(dataLen/sizeof(WCHAR))); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.
		}
	}
	else if (whichProp == GsmKeyProperty_KeyID)
	{
		PGPKeyID pgpKeyId;
		PGPChar	keyIDShort[kPGPMaxKeyIDStringSize];

		err = m_PgpSdkFunc.fPGPGetKeyDBObjDataProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, &pgpKeyId, sizeof(pgpKeyId), dataSize); CKERR;
		if(buffer)
		{
			err = m_PgpSdkFunc.fPGPGetKeyIDString (&pgpKeyId, kPGPKeyIDString_Abbreviated, keyIDShort); CKERR;
			wcsncpy_s((wchar_t *)buffer, bufferSize, (LPCTSTR)keyIDShort, _TRUNCATE);
		}
	}
	else if(whichProp >= GsmKeyUserIDProperty_Name && whichProp < 1300)
	{
		PGPKeyDBObjRef priUserID = kInvalidPGPKeyDBObjRef;

		/* Get the primary user ID */
		err = m_PgpSdkFunc.fPGPGetPrimaryUserID ((PGPKeyDBObjRef)handle, &priUserID); CKERR;
	
		if (!buffer)
		{
			bufferSize = 1024;
		}
		err = m_PgpSdkFunc.fPGPGetKeyDBObjDataProperty(priUserID, (PGPKeyDBObjProperty)whichProp, buffer, bufferSize, dataSize); CKERR;
	}
	else
	{
		err = kPGPError_BadParams;
	}

done:

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::GetKeyNumericProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, int *result)
{
	PGPError			err = kPGPError_NoErr;

	if (whichProp >= GsmKeyProperty_IsSecret && whichProp < GsmKeyProperty_AlgorithmID)
	{
		err = m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, (PGPBoolean *)result);
	}
	else if(whichProp >= GsmKeyProperty_AlgorithmID && whichProp < GsmKeyProperty_Creation)
	{
		err = m_PgpSdkFunc.fPGPGetKeyDBObjNumericProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, result);
	}
	else if (whichProp >= GsmKeyUserIDProperty_IsAttribute && whichProp < GsmKeyUserIDProperty_Name)
	{
		PGPKeyDBObjRef priUserID = kInvalidPGPKeyDBObjRef;

		/* Get the primary user ID */
		err = m_PgpSdkFunc.fPGPGetPrimaryUserID ((PGPKeyDBObjRef)handle, &priUserID); CKERR;
		if (whichProp >= GsmKeyUserIDProperty_IsAttribute && whichProp < GsmKeyUserIDProperty_Validity)
		{
			err = m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, (PGPBoolean *)result);
		}
		else
		{
			err = m_PgpSdkFunc.fPGPGetKeyDBObjNumericProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, result);
		}
	}
	else
	{
		err = kPGPError_BadParams;
	}

done:

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::GetKeyTimeProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, time_t *result)
{
	PGPError			err = kPGPError_NoErr;

	if (whichProp >= GsmKeyProperty_Creation && whichProp < GsmKeyProperty_Fingerprint)
	{
		err = m_PgpSdkFunc.fPGPGetKeyDBObjTimeProperty((PGPKeyDBObjRef)handle, (PGPKeyDBObjProperty)whichProp, (PGPTime *)result);
	}
	else
	{
		err = kPGPError_BadParams;
	}

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::ImportKey(LPCTSTR pszFilePath, GsmKeyHandle *hKey)
{
	PGPError				err						= kPGPError_NoErr;
	PGPKeyDBRef				importKeyDB	 	= kInvalidPGPKeyDBRef;
	PGPFileSpecRef			importFileSpec 	= kInvalidPGPFileSpecRef;
	PGPUInt32				numKeys = 0;
	PGPKeySetRef			keyset	= kInvalidPGPKeySetRef;
	PGPKeyListRef			keyList		= kInvalidPGPKeyListRef;
	PGPKeyIterRef			iter	 	= kInvalidPGPKeyIterRef;
	PGPBoolean				bProp = FALSE;
	PGPKeyDBObjRef			key = kInvalidPGPKeyDBObjRef;
	PGPKeyID				pgpKeyId;

	assert(NULL != pszFilePath);

	if (hKey)
	{
		*hKey = NULL;
	}

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)pszFilePath, &importFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPImport( m_context, &importKeyDB,
		m_PgpSdkFunc.fPGPOInputFile( m_context, importFileSpec ),
		m_PgpSdkFunc.fPGPOInputFormat( m_context, kPGPInputFormat_PGP),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	// copy  keys into the New DB
	err = m_PgpSdkFunc.fPGPNewKeySet( importKeyDB, &keyset ); CKERR;
	err = m_PgpSdkFunc.fPGPCountKeys (keyset, &numKeys); CKERR;
	DPW((_T("Import %d keys from %s\n"), numKeys, pszFilePath));

	/* Check KeyRing Sigs */
	/* NOTE: We must perform the KeyRing Sig check for expiration dates to work */
	err = m_PgpSdkFunc.fPGPCheckKeyRingSigs(keyset, importKeyDB, TRUE, NULL,NULL  );CKERR;

	/* Create a key list in  the set */
	err = m_PgpSdkFunc.fPGPOrderKeySet( keyset, kPGPKeyOrdering_KeyID, TRUE, &keyList ); CKERR;

	/* make an iterator */
	err = m_PgpSdkFunc.fPGPNewKeyIter( keyList, &iter); CKERR;

	for(; IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( iter, kPGPKeyDBObjType_Key, &key) ); )
	{
		m_PgpSdkFunc.fPGPGetKeyDBObjBooleanProperty(key, kPGPKeyProperty_IsExpired, &bProp);
		if (bProp)
		{
			continue;
		}
		m_PgpSdkFunc.fPGPGetKeyID(key, &pgpKeyId);
		break;
	}

	err = m_PgpSdkFunc.fPGPCopyKeys(keyset, m_keyDB, NULL);
	m_PgpSdkFunc.fPGPFlushKeyDB(m_keyDB);

	if (hKey)
	{
		err = m_PgpSdkFunc.fPGPFindKeyByKeyID(m_keyDB, &pgpKeyId, &key); CKERR;
		*hKey = (GsmKeyHandle)key;
	}

done:
	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	if( PGPKeyListRefIsValid (keyList) )
		m_PgpSdkFunc.fPGPFreeKeyList(keyList);

	if( PGPKeySetRefIsValid (keyset) )
		m_PgpSdkFunc.fPGPFreeKeySet(keyset);

	if( PGPKeyDBRefIsValid( importKeyDB ) )
	{
		m_PgpSdkFunc.fPGPFreeKeyDB( importKeyDB );
	}

	if( PGPFileSpecRefIsValid(importFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(importFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::ExportKey(GsmKeyHandle hKey, LPCTSTR pszFilePath)
{
	PGPError				err						= kPGPError_NoErr;
	PGPOptionListRef		optionList				= kInvalidPGPOptionListRef;
	PGPFileSpecRef			outputFileSpec			= kInvalidPGPFileSpecRef;

	assert(NULL != hKey && NULL != pszFilePath);

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)pszFilePath, &outputFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOExportKeyDBObj(m_context, (PGPKeyDBObjRef)hKey),
		m_PgpSdkFunc.fPGPOExportPrivateKeys(m_context,FALSE),
		m_PgpSdkFunc.fPGPOExportPrivateSubkeys(m_context,FALSE),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOExportFormat (m_context, kPGPExportFormat_Basic),
		m_PgpSdkFunc.fPGPOOutputFile (m_context, outputFileSpec),
		m_PgpSdkFunc.fPGPOVersionString (m_context, (PGPChar16 *)m_szVersion),
		m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPExport (m_context, optionList, m_PgpSdkFunc.fPGPOLastOption (m_context)); CKERR;

done:
	if (PGPFileSpecRefIsValid (outputFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec (outputFileSpec);
	if (PGPOptionListRefIsValid (optionList))
		m_PgpSdkFunc.fPGPFreeOptionList (optionList);

	return ConvertPgpError2GsmError(err);
}

void CPgpSdkAdapter::FreeBufferGetFromGsm(char *buff)
{
	if (buff)
	{
		m_PgpSdkFunc.fPGPFreeData(buff);
	}
}

// Key Server management
GsmErrorT CPgpSdkAdapter::SearchKeyFromKeyServerByKeyId(LPCTSTR pszServer, GsmKeyID keyId, GsmKeyHandle *hKey,
														GsmKeyServerStateChangedCallbackF cb, void *context)
{
	GsmErrorT err = 0;
	GsmKeyHandle *hKeys = NULL;
	int nKeys = 0;

	assert(pszServer && hKey);

	*hKey = NULL;

	err = ProcessKeyServerOperation(pszServer, GETVKDKEY, &keyId, NULL, &hKeys, &nKeys, cb, context);
	if (!err && nKeys)
	{
		*hKey = hKeys[0];
	}

	return err;
}

GsmErrorT CPgpSdkAdapter::SearchKeyFromKeyServerByEmail(LPCTSTR pszServer, LPCTSTR pszEmail, GsmKeyHandle **hKeys, int *nKeys,
														GsmKeyServerStateChangedCallbackF cb, void *context)
{
	GsmErrorT err = 0;

	assert(pszServer && pszEmail && hKeys && nKeys);

	err = ProcessKeyServerOperation(pszServer, GETVKDKEY, NULL, pszEmail, hKeys, nKeys, cb, context);

	return err;
}

GsmErrorT CPgpSdkAdapter::PublishKeyToKeyServer(LPCTSTR pszServer, GsmKeyID keyId,
												GsmKeyServerStateChangedCallbackF cb, void *context)
{
	GsmErrorT err = 0;

	assert(pszServer);

	err = ProcessKeyServerOperation(pszServer, SENDKEY, &keyId, NULL, NULL, NULL, cb, context);

	return err;
}

// Encryption
//GsmErrorT CPgpSdkAdapter::SymmEncryptBuffer(const char *plainBuf, char **cipherBuf, const char *passphrase)
GsmErrorT CPgpSdkAdapter::SymmEncryptBuffer(const char *plainBuf, size_t plainBufSize, 
							char **cipherBuf, size_t *cipherBufSize, LPCTSTR passphrase)
{
	PGPError err = kPGPError_NoErr;

	assert(NULL != plainBuf && NULL != cipherBuf && NULL != cipherBufSize);

	*cipherBuf = NULL;
	*cipherBufSize = 0;

	if (!passphrase || !passphrase[0])
	{
		return GsmErrorMake(GSM_ERR_BAD_PASSPHRASE);
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputBuffer ( m_context, plainBuf, plainBufSize ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)cipherBuf, MAX_PGPSize, cipherBufSize),
		m_PgpSdkFunc.fPGPOCipherAlgorithm(m_context, kPGPCipherAlgorithm_AES256),
		m_PgpSdkFunc.fPGPOConventionalEncrypt ( m_context, 
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)passphrase), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::SymmEncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, LPCTSTR passphrase)
{
	PGPError err = kPGPError_NoErr;
	PGPFileSpecRef 	plainFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef	cipherFileSpec 	= kInvalidPGPFileSpecRef;

	assert(NULL != plainFile && NULL != cipherFile);

	if (!passphrase || !passphrase[0])
	{
		return GsmErrorMake(GSM_ERR_BAD_PASSPHRASE);
	}

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)plainFile, &plainFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)cipherFile, &cipherFileSpec); CKERR;

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, plainFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, cipherFileSpec),
		m_PgpSdkFunc.fPGPOCipherAlgorithm(m_context, kPGPCipherAlgorithm_AES256),
		m_PgpSdkFunc.fPGPOConventionalEncrypt ( m_context, 
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)passphrase), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		//m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(cipherFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(cipherFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::SymmEncryptFile(LPCTSTR filePath, LPCTSTR passphrase)
{
	PGPError err = kPGPError_NoErr;
	PGPFileSpecRef 	fileSpec 	= kInvalidPGPFileSpecRef;
	PGPChar *pszCipherBuffer = NULL;
	PGPSize uiCipherBufferSize = 0;
	FILE *hFile = NULL;

	assert(NULL != filePath);

	if (!passphrase || !passphrase[0])
	{
		return GsmErrorMake(GSM_ERR_BAD_PASSPHRASE);
	}

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (PGPChar16 *)filePath, &fileSpec); CKERR;

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, fileSpec ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)&pszCipherBuffer, MAX_PGPSize, &uiCipherBufferSize),
		m_PgpSdkFunc.fPGPOCipherAlgorithm(m_context, kPGPCipherAlgorithm_AES256),
		m_PgpSdkFunc.fPGPOConventionalEncrypt ( m_context, 
		m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)passphrase), 
		m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		//m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if( PGPFileSpecRefIsValid(fileSpec))
	{
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);
		fileSpec 	= kInvalidPGPFileSpecRef;
	}

	if (_wfopen_s(&hFile, filePath, _T("wb")) != 0 || hFile == NULL)
	{
		return GsmErrorMake(GSM_ERR_FILE_OPEN);
	}

	fwrite(pszCipherBuffer, 1, uiCipherBufferSize, hFile);
	fclose(hFile);

done:
	if( PGPFileSpecRefIsValid(fileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);

	return ConvertPgpError2GsmError(err);
}


GsmErrorT CPgpSdkAdapter::EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
										char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle encKey, 
										GsmKeyHandle signKey, LPCTSTR preSignPass, 
										GsmPassphraseCallbackF cb, void *context)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPKeySetRef			encryptSet	= kInvalidPGPKeySetRef;
	PGPOptionListRef		encodeOptions	= kInvalidPGPOptionListRef;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != plainBuf && NULL != cipherBuf && NULL != encKey);

	*cipherBuf = NULL;
	*cipherBufSize = 0;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &encodeOptions); CKERR;

	/* create encryption keyset */
	err = m_PgpSdkFunc.fPGPNewOneKeySet((PGPKeyDBObjRef)encKey, &encryptSet);  CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPOForYourEyesOnly(m_context,TRUE),
		m_PgpSdkFunc.fPGPODataIsASCII ( m_context, FALSE),
		//m_PgpSdkFunc.fPGPOOutputLineEndType ( context, kPGPLineEnd_LF),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (signKey && preSignPass)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
			m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
				m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)preSignPass), 
				m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputBuffer ( m_context, plainBuf, plainBufSize ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)cipherBuf, MAX_PGPSize, cipherBufSize),
		m_PgpSdkFunc.fPGPOEncryptToKeySet(m_context, encryptSet),
		m_PgpSdkFunc.fPGPOOutputFormat(m_context, kPGPOutputFormat_PGP),
		encodeOptions,	
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if( PGPKeySetRefIsValid (encryptSet) )
		m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

	if(PGPOptionListRefIsValid(encodeOptions))
		m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
						char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle *encKeys, int nKeys,
						GsmKeyHandle signKey, LPCTSTR preSignPass, 
						GsmPassphraseCallbackF cb, void *context)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPKeySetRef			encryptSet	= kInvalidPGPKeySetRef;
	PGPOptionListRef		encodeOptions	= kInvalidPGPOptionListRef;
	int						i = 0;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != plainBuf && NULL != cipherBuf && NULL != encKeys && nKeys);

	*cipherBuf = NULL;
	*cipherBufSize = 0;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &encodeOptions); CKERR;

	/* create encryption keyset */
	err = m_PgpSdkFunc.fPGPNewOneKeySet((PGPKeyDBObjRef)encKeys[0], &encryptSet);  CKERR;
	for (i = 1; i < nKeys; i++)
	{
		m_PgpSdkFunc.fPGPAddKey((PGPKeyDBObjRef)encKeys[i], encryptSet);
	}

	err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPOForYourEyesOnly(m_context,TRUE),
		m_PgpSdkFunc.fPGPODataIsASCII ( m_context, FALSE),
		//m_PgpSdkFunc.fPGPOOutputLineEndType ( context, kPGPLineEnd_LF),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (signKey && preSignPass)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
			m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (PGPChar16 *)preSignPass), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputBuffer ( m_context, plainBuf, plainBufSize ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)cipherBuf, MAX_PGPSize, cipherBufSize),
		m_PgpSdkFunc.fPGPOEncryptToKeySet(m_context, encryptSet),
		m_PgpSdkFunc.fPGPOOutputFormat(m_context, kPGPOutputFormat_PGP),
		encodeOptions,	
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if( PGPKeySetRefIsValid (encryptSet) )
		m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

	if(PGPOptionListRefIsValid(encodeOptions))
		m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, GsmKeyHandle encKey, 
							  GsmKeyHandle signKey, LPCTSTR preSignPass, 
							  GsmPassphraseCallbackF cb, void *context)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPKeySetRef			encryptSet	= kInvalidPGPKeySetRef;
	PGPOptionListRef		encodeOptions	= kInvalidPGPOptionListRef;
	PGPFileSpecRef 	plainFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef	cipherFileSpec 	= kInvalidPGPFileSpecRef;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != plainFile && NULL != cipherFile && NULL != encKey);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)plainFile, &plainFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)cipherFile, &cipherFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &encodeOptions); CKERR;

	/* create encryption keyset */
	err = m_PgpSdkFunc.fPGPNewOneKeySet((PGPKeyDBObjRef)encKey, &encryptSet);  CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPOForYourEyesOnly(m_context,TRUE),
		//m_PgpSdkFunc.fPGPODataIsASCII ( m_context, FALSE),
		//m_PgpSdkFunc.fPGPOOutputLineEndType ( context, kPGPLineEnd_LF),
		//m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (signKey && preSignPass)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
			m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
				m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preSignPass), 
				m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, plainFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, cipherFileSpec),
		m_PgpSdkFunc.fPGPOEncryptToKeySet(m_context, encryptSet),
		m_PgpSdkFunc.fPGPOOutputFormat(m_context, kPGPOutputFormat_PGP),
		encodeOptions,
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if( PGPKeySetRefIsValid (encryptSet) )
		m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

	if(PGPOptionListRefIsValid(encodeOptions))
		m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(cipherFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(cipherFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, 
					  GsmKeyHandle *encKeys, int nKeys,
					  GsmKeyHandle signKey, LPCTSTR preSignPass, 
					  GsmPassphraseCallbackF cb, void *context)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPKeySetRef			encryptSet	= kInvalidPGPKeySetRef;
	PGPOptionListRef		encodeOptions	= kInvalidPGPOptionListRef;
	PGPFileSpecRef 	plainFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef	cipherFileSpec 	= kInvalidPGPFileSpecRef;
	int				i = 0;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != plainFile && NULL != cipherFile && NULL != encKeys && nKeys);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)plainFile, &plainFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)cipherFile, &cipherFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &encodeOptions); CKERR;

	/* create encryption keyset */
	err = m_PgpSdkFunc.fPGPNewOneKeySet((PGPKeyDBObjRef)encKeys[0], &encryptSet);  CKERR;
	for (i = 1; i < nKeys; i++)
	{
		m_PgpSdkFunc.fPGPAddKey((PGPKeyDBObjRef)encKeys[i], encryptSet);
	}

	err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPOForYourEyesOnly(m_context,TRUE),
		//m_PgpSdkFunc.fPGPODataIsASCII ( m_context, FALSE),
		//m_PgpSdkFunc.fPGPOOutputLineEndType ( context, kPGPLineEnd_LF),
		//m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (signKey && preSignPass)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
			m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preSignPass), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, plainFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, cipherFileSpec),
		m_PgpSdkFunc.fPGPOEncryptToKeySet(m_context, encryptSet),
		m_PgpSdkFunc.fPGPOOutputFormat(m_context, kPGPOutputFormat_PGP),
		encodeOptions,
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if( PGPKeySetRefIsValid (encryptSet) )
		m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

	if(PGPOptionListRefIsValid(encodeOptions))
		m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(cipherFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(cipherFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::EncryptFile2(LPCTSTR filePath, 
					  GsmKeyHandle *encKeys, int nKeys,
					  GsmKeyHandle signKey, LPCTSTR preSignPass, 
					  GsmPassphraseCallbackF cb, void *context)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPKeySetRef			encryptSet	= kInvalidPGPKeySetRef;
	PGPOptionListRef		encodeOptions	= kInvalidPGPOptionListRef;
	PGPFileSpecRef 			fileSpec 	= kInvalidPGPFileSpecRef;
	int						i = 0;
	PGPChar					*pszCipherBuffer = NULL;
	PGPSize					uiCipherBufferSize = 0;
	FILE					*hFile = NULL;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != filePath && NULL != encKeys && nKeys);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)filePath, &fileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &encodeOptions); CKERR;

	/* create encryption keyset */
	err = m_PgpSdkFunc.fPGPNewOneKeySet((PGPKeyDBObjRef)encKeys[0], &encryptSet);  CKERR;
	for (i = 1; i < nKeys; i++)
	{
		m_PgpSdkFunc.fPGPAddKey((PGPKeyDBObjRef)encKeys[i], encryptSet);
	}

	err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPOForYourEyesOnly(m_context,TRUE),
		//m_PgpSdkFunc.fPGPODataIsASCII ( m_context, FALSE),
		//m_PgpSdkFunc.fPGPOOutputLineEndType ( context, kPGPLineEnd_LF),
		//m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (signKey && preSignPass)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(encodeOptions, 
			m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preSignPass), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, fileSpec ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)&pszCipherBuffer, MAX_PGPSize, &uiCipherBufferSize),
		m_PgpSdkFunc.fPGPOEncryptToKeySet(m_context, encryptSet),
		m_PgpSdkFunc.fPGPOOutputFormat(m_context, kPGPOutputFormat_PGP),
		encodeOptions,
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if( PGPFileSpecRefIsValid(fileSpec))
	{
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);
		fileSpec 	= kInvalidPGPFileSpecRef;
	}

	if (_wfopen_s(&hFile, filePath, _T("wb")) != 0 || !hFile)
	{
		if( PGPKeySetRefIsValid (encryptSet) )
			m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

		if(PGPOptionListRefIsValid(encodeOptions))
			m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

		return GsmErrorMake(GSM_ERR_FILE_OPEN);
	}

	fwrite(pszCipherBuffer, 1, uiCipherBufferSize, hFile);
	fclose(hFile);

done:
	if( PGPKeySetRefIsValid (encryptSet) )
		m_PgpSdkFunc.fPGPFreeKeySet(encryptSet);

	if(PGPOptionListRefIsValid(encodeOptions))
		m_PgpSdkFunc.fPGPFreeOptionList(encodeOptions);

	if( PGPFileSpecRefIsValid(fileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);

	return ConvertPgpError2GsmError(err);
}
#if 0
int CPgpSdkAdapter::IsEncryptedBuffer(const char *buffer, size_t len)
{
	unsigned char ucPgpHead[5] = {0xA8, 0x03, 0x50, 0x47, 0x50};

	if (!buffer && len < 5)
	{
		return 0;
	}

	if (!strncmp((const char *)buffer, "-----BEGIN PGP MESSAGE-----", 27))
	{
		return 1;
	}
	else if (!memcmp(buffer, ucPgpHead, 5))
	{
		return 1;
	}

	return 0;
}
#endif

BOOL CPgpSdkAdapter::IsEncryptedFile(LPCTSTR filePath)
{
	FILE *hFile = NULL;
	unsigned char szHead[64];
	size_t len;
	unsigned char ucPgpHead[5] = {0xA8, 0x03, 0x50, 0x47, 0x50};

	if (!filePath)
	{
		return 0;
	}

	if (_wfopen_s(&hFile, filePath, L"r") != 0 || !hFile)
	{
		return 0;
	}

	memset(szHead, 0, 64);
	len = fread(szHead, 1, 32, hFile);
	fclose(hFile);
	hFile = NULL;
	if (!strncmp((const char *)szHead, "-----BEGIN PGP MESSAGE-----", 27))
	{
		return 1;
	}
	else if (!memcmp(szHead, ucPgpHead, 5))
	{
		return 1;
	}



	return 0;
}


// Decryption
GsmErrorT CPgpSdkAdapter::DecryptVerifyBuffer(const char *cipherBuf, size_t cipherBufSize, 
											char **plainBuf, size_t *plainBufSize, LPCTSTR preDecryptPass, 
											GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpPassphraseCallbackData cbData;

	assert(NULL != cipherBuf && NULL != plainBuf && NULL != plainBufSize);

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));

	*plainBuf = NULL;
	*plainBufSize = 0;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	cbData.cb = cb;
	cbData.context = context;
	cbData.pgpSdk = this;
	cbData.recipientsData = NULL;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputBuffer(m_context, cipherBuf, cipherBufSize),
		m_PgpSdkFunc.fPGPOInputFormat(m_context, kPGPInputFormat_PGP),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		preDecryptPass ? m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preDecryptPass) : m_PgpSdkFunc.fPGPONullOption(m_context),
		//m_PgpSdkFunc.fPGPOPassThroughIfUnrecognized(m_context, TRUE),
		//m_PgpSdkFunc.fPGPOPassThroughKeys(m_context, TRUE),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkDecodeEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer(m_context, (void**)plainBuf, MAX_PGPSize, plainBufSize),
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if (cbData.recipientsData)
	{
		//PGPFreeKeySet(((PGPEventRecipientsData *)cbData.recipientsData)->recipientSet);
		m_PgpSdkFunc.fPGPFreeData(cbData.recipientsData);
		cbData.recipientsData = NULL;
	}

	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::DecryptVerifyFile(LPCTSTR cipherFile, LPCTSTR plainFile, LPCTSTR preDecryptPass, 
									GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpPassphraseCallbackData cbData;
	PGPFileSpecRef 			plainFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef			cipherFileSpec 	= kInvalidPGPFileSpecRef;

	UNUSED(context);
	UNUSED(cb);

	assert(NULL != cipherFile && NULL != plainFile);

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)plainFile, &plainFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)cipherFile, &cipherFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	
	cbData.cb = cb;
	cbData.context = context;
	cbData.pgpSdk = this;
	cbData.recipientsData = NULL;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputFile ( m_context, cipherFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, plainFileSpec),
		m_PgpSdkFunc.fPGPOInputFormat(m_context, kPGPInputFormat_PGP),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		preDecryptPass ? m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preDecryptPass) : m_PgpSdkFunc.fPGPONullOption(m_context),
		//m_PgpSdkFunc.fPGPOPassThroughIfUnrecognized(m_context, TRUE),
		//m_PgpSdkFunc.fPGPOPassThroughKeys(m_context, TRUE),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkDecodeEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if (cbData.recipientsData)
	{
		//PGPFreeKeySet(((PGPEventRecipientsData *)cbData.recipientsData)->recipientSet);
		m_PgpSdkFunc.fPGPFreeData(cbData.recipientsData);
		cbData.recipientsData = NULL;
	}

	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(cipherFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(cipherFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::DecryptVerifyFile(LPCTSTR filePath, LPCTSTR preDecryptPass,
							GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpPassphraseCallbackData cbData;
	PGPFileSpecRef 			fileSpec 	= kInvalidPGPFileSpecRef;
	PGPChar					*pszPlainBuffer = NULL;
	PGPSize					uiPlainBufferSize = 0;
	FILE					*hFile = NULL;

	assert(NULL != filePath);

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)filePath, &fileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	cbData.cb = cb;
	cbData.context = context;
	cbData.pgpSdk = this;
	cbData.recipientsData = NULL;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputFile ( m_context, fileSpec ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer( m_context, (void **)&pszPlainBuffer, MAX_PGPSize, &uiPlainBufferSize),
		m_PgpSdkFunc.fPGPOInputFormat(m_context, kPGPInputFormat_PGP),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		preDecryptPass ? m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preDecryptPass) : m_PgpSdkFunc.fPGPONullOption(m_context),
		//m_PgpSdkFunc.fPGPOPassThroughIfUnrecognized(m_context, TRUE),
		//m_PgpSdkFunc.fPGPOPassThroughKeys(m_context, TRUE),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkDecodeEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPOSendNullEvents(m_context, 100),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	if( PGPFileSpecRefIsValid(fileSpec))
	{
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);
		fileSpec 	= kInvalidPGPFileSpecRef;
	}

	if (_wfopen_s(&hFile, filePath, L"r") != 0 || !hFile)
	{
		if (cbData.recipientsData)
		{
			//PGPFreeKeySet(((PGPEventRecipientsData *)cbData.recipientsData)->recipientSet);
			m_PgpSdkFunc.fPGPFreeData(cbData.recipientsData);
			cbData.recipientsData = NULL;
		}

		if(PGPOptionListRefIsValid(optionList))
			m_PgpSdkFunc.fPGPFreeOptionList(optionList);

		return GsmErrorMake(GSM_ERR_FILE_OPEN);
	}

	fwrite(pszPlainBuffer, 1, uiPlainBufferSize, hFile);
	fclose(hFile);

done:
	if (cbData.recipientsData)
	{
		//PGPFreeKeySet(((PGPEventRecipientsData *)cbData.recipientsData)->recipientSet);
		m_PgpSdkFunc.fPGPFreeData(cbData.recipientsData);
		cbData.recipientsData = NULL;
	}

	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	if( PGPFileSpecRefIsValid(fileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(fileSpec);

	return ConvertPgpError2GsmError(err);
}
// Signature
GsmErrorT CPgpSdkAdapter::SignBuffer(const char *plainBuf, size_t plainBufSize, 
									 char **signBuf, size_t *signBufSize, GsmKeyHandle signKey, 
									 LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		signOption	= kInvalidPGPOptionListRef;

	UNUSED(cb);
	UNUSED(context);
	
	assert(NULL != plainBuf && NULL != signBuf && NULL != signBufSize && NULL != signKey);

	*signBuf = NULL;
	*signBufSize = 0;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &signOption); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(signOption, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPODataIsASCII ( m_context, TRUE),
		//m_PgpSdkFunc.fPGPOCharsetString(m_context, "us-ascii"),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(signOption, 
		m_PgpSdkFunc.fPGPORawPGPInput(m_context, FALSE),
		m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
			m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preSignPass), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (mode == GSM_SIGN_MODE_DETACH)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(signOption,
			m_PgpSdkFunc.fPGPODetachedSig(m_context, m_PgpSdkFunc.fPGPOLastOption( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}
	else if (mode == GSM_SIGN_MODE_CLEAR)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(signOption,
			m_PgpSdkFunc.fPGPOClearSign(m_context, TRUE),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	//*signBuf = (char *)m_PgpSdkFunc.fPGPNewData( m_PgpSdkFunc.fPGPGetDefaultMemoryMgr(), 1024, kPGPMemoryMgrFlags_Clear); CKNULL(*signBuf);

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputBuffer ( m_context, plainBuf, plainBufSize ),
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer(m_context, (void**)signBuf, MAX_PGPSize, signBufSize),
		signOption,	
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if(PGPOptionListRefIsValid(signOption))
		m_PgpSdkFunc.fPGPFreeOptionList(signOption);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::SignFile(LPCTSTR plainFile, LPCTSTR signFile, GsmKeyHandle signKey, 
						   LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		signOption	= kInvalidPGPOptionListRef;
	PGPFileSpecRef 			plainFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef			signFileSpec 	= kInvalidPGPFileSpecRef;

	UNUSED(cb);
	UNUSED(context);

	assert(NULL != plainFile && NULL != signFile && NULL != signKey);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)plainFile, &plainFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)signFile, &signFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList(m_context, &signOption); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(signOption, 
		m_PgpSdkFunc.fPGPOEventHandler( m_context, PgpSdkEncodeEventHandler, this),
		//m_PgpSdkFunc.fPGPODataIsASCII ( m_context, TRUE),
		//m_PgpSdkFunc.fPGPOCharsetString(m_context, "us-ascii"),
		m_PgpSdkFunc.fPGPOArmorOutput(m_context, TRUE),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	err = m_PgpSdkFunc.fPGPAppendOptionList(signOption, 
		m_PgpSdkFunc.fPGPORawPGPInput(m_context, FALSE),
		m_PgpSdkFunc.fPGPOSignWithKey ( m_context, (PGPKeyDBObjRef)signKey,
		m_PgpSdkFunc.fPGPOPassphrase( m_context, (const PGPChar16 *)preSignPass), 
		m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

	if (mode == GSM_SIGN_MODE_DETACH)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(signOption,
			m_PgpSdkFunc.fPGPODetachedSig(m_context, m_PgpSdkFunc.fPGPOLastOption( m_context ) ),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}
	else if (mode == GSM_SIGN_MODE_CLEAR)
	{
		err = m_PgpSdkFunc.fPGPAppendOptionList(signOption,
			m_PgpSdkFunc.fPGPODataIsASCII ( m_context, TRUE),
			//m_PgpSdkFunc.fPGPOCharsetString(m_context, "us-ascii"),
			m_PgpSdkFunc.fPGPOClearSign(m_context, TRUE),
			m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;
	}

	err =  m_PgpSdkFunc.fPGPEncode( m_context, 
		m_PgpSdkFunc.fPGPOInputFile ( m_context, plainFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, signFileSpec),
		signOption,	
		m_PgpSdkFunc.fPGPOLastOption( m_context ) ); CKERR;

done:
	if(PGPOptionListRefIsValid(signOption))
		m_PgpSdkFunc.fPGPFreeOptionList(signOption);

	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(signFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(signFileSpec);

	return ConvertPgpError2GsmError(err);
}


// Verification
GsmErrorT CPgpSdkAdapter::VerifyBuffer(const char *signBuf, size_t signBufSize, 
									   char **plainBuf, size_t *plainBufSize, GsmVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpVerifyCallbackData	cbData;

	assert(NULL != signBuf && NULL != plainBuf && NULL != plainBufSize);

	*plainBuf = NULL;
	*plainBufSize = 0;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));
	cbData.pgpSdk = this;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputBuffer(m_context, signBuf, signBufSize),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkVerifyEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPOAllocatedOutputBuffer(m_context, (void**)plainBuf, MAX_PGPSize, plainBufSize),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::VerifyFile(LPCTSTR signFile, LPCTSTR plainFile, GsmVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpVerifyCallbackData	cbData;
	PGPFileSpecRef 			signFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef			plainFileSpec 	= kInvalidPGPFileSpecRef;

	assert(NULL != signFile && NULL != plainFile);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)signFile, &signFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)plainFile, &plainFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));
	cbData.pgpSdk = this;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputFile ( m_context, signFileSpec ),
		m_PgpSdkFunc.fPGPOOutputFile( m_context, plainFileSpec),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkVerifyEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	if( PGPFileSpecRefIsValid(plainFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(plainFileSpec);

	if( PGPFileSpecRefIsValid(signFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(signFileSpec);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::VerifyBufferDetachedSig(const char *dataBuf, size_t dataBufSize, 
												  const char *sigBuf, size_t sigBufSize, GsmVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpVerifyCallbackData	cbData;

	assert(NULL != dataBuf && NULL != sigBuf);

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));
	cbData.pgpSdk = this;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputBuffer(m_context, sigBuf, sigBufSize),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkVerifyEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPODetachedSig(m_context, 
			m_PgpSdkFunc.fPGPOInputBuffer ( m_context, dataBuf, dataBufSize ), 
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	return ConvertPgpError2GsmError(err);
}

GsmErrorT CPgpSdkAdapter::VerifyFileDetachedSig(LPCTSTR dataFile, LPCTSTR sigFile, GsmVerifyResult &result)
{
	PGPError				err 		= kPGPError_NoErr;
	PGPOptionListRef		optionList = kInvalidPGPOptionListRef;
	PgpVerifyCallbackData	cbData;
	PGPFileSpecRef 			dataFileSpec 	= kInvalidPGPFileSpecRef;
	PGPFileSpecRef			sigFileSpec 	= kInvalidPGPFileSpecRef;

	assert(NULL != dataFile && NULL != sigFile);

	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)dataFile, &dataFileSpec); CKERR;
	err = m_PgpSdkFunc.fPGPNewFileSpecFromFullPath(m_context, (const PGPChar16 *)sigFile, &sigFileSpec); CKERR;

	err = m_PgpSdkFunc.fPGPNewOptionList (m_context, &optionList); CKERR;

	memset(&cbData, 0, sizeof(cbData));
	memset(&result, 0, sizeof(result));
	cbData.pgpSdk = this;
	cbData.result = &result;
	err = m_PgpSdkFunc.fPGPAppendOptionList (optionList,
		m_PgpSdkFunc.fPGPOInputFile ( m_context, sigFileSpec ),
		m_PgpSdkFunc.fPGPOKeyDBRef(m_context, m_keyDB),
		m_PgpSdkFunc.fPGPOEventHandler(m_context, PgpSdkVerifyEventHandler, (void *)&cbData), 
		m_PgpSdkFunc.fPGPODetachedSig(m_context, 
			m_PgpSdkFunc.fPGPOInputFile ( m_context, dataFileSpec ),
			m_PgpSdkFunc.fPGPOLastOption ( m_context ) ),
		m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

	err = m_PgpSdkFunc.fPGPDecode(m_context, optionList, m_PgpSdkFunc.fPGPOLastOption(m_context)); CKERR;

done:
	if(PGPOptionListRefIsValid(optionList))
		m_PgpSdkFunc.fPGPFreeOptionList(optionList);

	if( PGPFileSpecRefIsValid(dataFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(dataFileSpec);

	if( PGPFileSpecRefIsValid(sigFileSpec))
		m_PgpSdkFunc.fPGPFreeFileSpec(sigFileSpec);

	return ConvertPgpError2GsmError(err);
}

#if 0
GsmErrorT CPgpSdkAdapter::CreateSDAFile(const char **srcPaths, int nSrc, 
										const char *destPath, const char *passwd, 
										GsmSdaTargetPlatform platform)
{
	PGPsdaContextRef sdaContext	= kInvalidPGPsdaContextRef;
	PGPError err = kPGPError_NoErr;
	PGPChar szRootPath[MAX_PATH];
	PGPChar *pszPos = NULL;

	if (!srcPaths || !nSrc || !destPath || !passwd)
	{
		return GsmErrorMake(GSM_ERR_NULL_PTR);
	}

	err = m_PgpSdkFunc.fPGPNewSDAContext(m_context, &sdaContext); CKERR;

	for (int i = 0; i < nSrc; i++)
	{
		if (srcPaths[i])
		{
			strncpy((char *)szRootPath, srcPaths[i], MAX_PATH);
			if (szRootPath[strlen(szRootPath)-1] == '\\'
				|| szRootPath[strlen(szRootPath)-1] == '/')
			{
				szRootPath[strlen(szRootPath)-1] = 0;
			}
			pszPos = strrchr(szRootPath, '\\');
			if (!pszPos)
			{
				pszPos = strrchr(szRootPath, '/');
			}
			if (!pszPos || *(pszPos-1) == ':')
			{
				err = m_PgpSdkFunc.fPGPsdaImportObject(sdaContext, szRootPath, NULL, TRUE); CKERR;
			}
			else
			{
				*(pszPos+1) = 0;
				err = m_PgpSdkFunc.fPGPsdaImportObject(sdaContext, (PGPChar *)srcPaths[i], szRootPath, TRUE); CKERR;
			}
		}
	}

	err = m_PgpSdkFunc.fPGPsdaCreate(sdaContext, kPGPCipherAlgorithm_AES256, kPGPHashAlgorithm_SHA,
		kPGPCompressionAlgorithm_ZIP, kPGPsdaCompressionLevel_Speed, (PGPsdaTargetPlatform)platform,
		FALSE, kInvalidPGPKeySetRef, (PGPChar *)passwd, (PGPChar *)destPath, sdaEventHandlerProcPtr, NULL);

done:
	if (PGPsdaContextRefIsValid(sdaContext))
		m_PgpSdkFunc.fPGPFreeSDAContext(sdaContext);

	return ConvertPgpError2GsmError(err);
}

int CPgpSdkAdapter::IsSDAFile(const char *filePath)
{
	PGPError err = kPGPError_NoErr;

	err = m_PgpSdkFunc.fPGPsdaVerify((PGPChar *)filePath);

	return (err == kPGPError_NoErr);
}

GsmErrorT CPgpSdkAdapter::DecryptSDAFile(const char *filePath, const char *destDirPath, const char *passwd)
{
	PGPError err = kPGPError_NoErr;

	if (!filePath || !destDirPath || !passwd)
	{
		return GsmErrorMake(GSM_ERR_NULL_PTR);
	}

	err = m_PgpSdkFunc.fPGPsdaDecrypt((PGPChar *)filePath, (PGPChar8 *)passwd, 
		(PGPChar *)destDirPath, FALSE, FALSE, sdaDecodeEventHandlerProc, m_context);

	return ConvertPgpError2GsmError(err);
}
#endif

//////////////////////////////////////////////////////////////
static GsmErrorT ConvertPgpError2GsmError(PGPError err)
{
	GsmErrorCode gsmErrCode = GSM_ERR_NO_ERROR;

	switch (err)
	{
	case kPGPError_NoErr:
		gsmErrCode = GSM_ERR_NO_ERROR;
		break;
	case kPGPError_OutOfMemory:
		gsmErrCode = GSM_ERR_NO_RESOURCE;
		break;
	case kPGPError_UnknownPublicKeyAlgorithm:
		gsmErrCode = GSM_ERR_PUBKEY_ALGO;
		break;

	case kPGPError_KeyPacketTruncated:
	case kPGPError_UnknownKeyVersion:
	case kPGPError_MalformedKeyModulus:
	case kPGPError_MalformedKeyExponent:
	case kPGPError_RSAPublicModulusIsEven:
	case kPGPError_RSAPublicExponentIsEven:
	case kPGPError_MalformedKeyComponent:
	case kPGPError_KeyTooLarge:
	case kPGPError_PublicKeyTooSmall:
	case kPGPError_PublicKeyTooLarge:
	case kPGPError_PublicKeyUnimplemented:
	case kPGPError_CRLPacketTruncated:
		gsmErrCode = GSM_ERR_BAD_PUBKEY;
		break;
	case kPGPError_CorruptPrivateKey:
	case kPGPError_SecretKeyNotFound:
		gsmErrCode = GSM_ERR_BAD_SECKEY;
		break;
	case kPGPError_BadSignature:
	case kPGPError_UnknownSignatureType:
	case kPGPError_BadSignatureSize:
	case kPGPError_SignatureBitsWrong:
	case kPGPError_ExtraDateOnSignature:
	case kPGPError_TruncatedSignature:
	case kPGPError_MalformedSignatureInteger:
	case kPGPError_UnknownSignatureAlgorithm:
	case kPGPError_ExtraSignatureMaterial:
	case kPGPError_UnknownSignatureVersion:
	case kPGPError_RevocationKeyNotFound:
		gsmErrCode = GSM_ERR_BAD_SIGNATURE;
		break;
	case kPGPError_KeyUnusableForEncryption:
		gsmErrCode = GSM_ERR_NO_PUBKEY;
		break;
	case kPGPError_BadPassphrase:
	case kPGPError_MissingPassphrase:
		gsmErrCode = GSM_ERR_BAD_PASSPHRASE;
		break;
	case kPGPError_BadCipherNumber:
		gsmErrCode = GSM_ERR_CIPHER_ALGO;
		break;
	case kPGPError_KeyUnusableForSignature:
	case kPGPError_KeyUnusableForDecryption:
	case kPGPError_NoDecryptionKeyFound:
		gsmErrCode = GSM_ERR_NO_SECKEY;
		break;
	case kPGPError_ServerInProgress:
	case kPGPError_ServerOperationNotSupported:
	case kPGPError_ServerInvalidProtocol:
	case kPGPError_ServerRequestFailed:
	case kPGPError_ServerOpen:
	case kPGPError_ServerNotOpen:
	case kPGPError_ServerKeyAlreadyExists:
	case kPGPError_ServerNotInitialized:
	case kPGPError_ServerPartialAddFailure:
	case kPGPError_ServerCorruptKeyBlock:
	case kPGPError_ServerUnknownResponse:
	case kPGPError_ServerTimedOut:
	case kPGPError_ServerOpenFailed:
	case kPGPError_ServerAuthorizationRequired:
	case kPGPError_ServerAuthorizationFailed:
	case kPGPError_ServerSearchFailed:
	case kPGPError_ServerPartialSearchResults:
	case kPGPError_ServerBadKeysInSearchResults:
	case kPGPError_ServerKeyFailedPolicy:
	case kPGPError_ServerOperationRequiresTLS:
	case kPGPError_ServerNoStaticStorage:
	case kPGPError_ServerCertNotFound:
		gsmErrCode = GSM_ERR_KEYSERVER;
		break;
	case kPGPError_BadParams:
	case kPGPError_InvalidFilterParameter:
	case kPGPError_UnknownFilterType:
		gsmErrCode = GSM_ERR_INV_PARAMETER;
		break;

	case kPGPError_FileNotFound:
		gsmErrCode = GSM_ERR_FILE_NOT_FOUND;
		break;
	case kPGPError_CantOpenFile:
		gsmErrCode = GSM_ERR_FILE_OPEN;
		break;
	case kPGPError_ItemNotFound:
		gsmErrCode = GSM_ERR_NOT_FOUND;
		break;
	case kPGPError_DeCompressionFailed:
	case kPGPError_AsciiParseIncomplete:
		gsmErrCode = GSM_ERR_SDK_BAD_CONTENT;
		break;
	default:
		gsmErrCode = GSM_ERR_GENERAL;
		break;
	}

	return gsmErrCode;//GsmErrorMake(gsmErrCode);
}

static int makeDirectory(LPCTSTR dirname)
{
	int retval;

#ifdef WIN32
	retval = _wmkdir(dirname);
#else 
	retval =  mkdir(dirname, 0777);
#endif

	return (retval == 0?0: (errno == EEXIST?0:-1));
}

static PGPError genKeyEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue)
{
	PGPError	err	= kPGPError_NoErr;
	PgpProcessCallbackData *cbData = (PgpProcessCallbackData *)userValue;

	if (!PGPContextRefIsValid (context) || !event || !cbData)
	{
		return kPGPError_BadParams;
	}

	switch (event->type)
	{
	case kPGPEvent_InitialEvent:
		/* Not fired */
		DPA(("genKeyEventHandler: Generating key...\n"));
		break;

	case kPGPEvent_FinalEvent:
		/* Not fired */
		DPA(("Key generation complete.\n"));
		break;

	case kPGPEvent_KeyGenEvent:
		/* Show progress */
		DPA(("."));
		if (cbData->cb)
		{
			cbData->cb(cbData->context, NULL, (int)event->data.keyGenData.state);
		}
		
		break;

	case kPGPEvent_EntropyEvent:
		/* Entropy event not fired (but also not needed?) */
		DPA(("Entropy requested...\n"));
		err = kPGPError_OutOfEntropy;
		break;

	default:
		DPA((" unhandled event %u\n"));
		break;
	}

	return err;
}

/* Format a Fingerprint for printing */

static void FormatFingerprintString(char *p, PGPByte *inBuffp, PGPSize len )
{
	static char hexDigit[] = "0123456789ABCDEF";
	int			strIndex;
	if(len == 20)
	{
		for(strIndex = 0 ; strIndex < 20 ; strIndex++)
		{
			*p++ = hexDigit[inBuffp[strIndex]>>4];
			*p++ = hexDigit[inBuffp[strIndex]&0xF];
			if((strIndex == 1) || (strIndex == 3) || (strIndex == 5)
				|| (strIndex == 7) || (strIndex == 11) ||
				(strIndex == 13)
				|| (strIndex == 13) || (strIndex == 15) ||
				(strIndex == 17))
				*p++ = ' ';
			else if(strIndex == 9)
			{
				*p++ = ' ';
				*p++ = ' ';
			}
		}
	}
	else
	{
		for(strIndex = 0 ; strIndex < 16 ; strIndex++)
		{
			*p++ = hexDigit[inBuffp[strIndex]>>4];
			*p++ = hexDigit[inBuffp[strIndex]&0xF];
			if((strIndex == 1) || (strIndex == 3) || (strIndex == 5)
				|| (strIndex == 9) || (strIndex == 11) || (strIndex == 13))
				*p++ = ' ';
			else if(strIndex == 7)
			{
				*p++ = ' ';
				*p++ = ' ';
			}
		}
	}
	*p++ = '\0';
}

PGPError PgpSdkEncodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue)
{
	PGPError				err 		= kPGPError_NoErr;
	CPgpSdkAdapter *pgpSdk = (CPgpSdkAdapter *)userValue;

	UNUSED(context);

	assert(context != NULL);
	assert(event != NULL);
	assert(userValue != NULL);

	switch (event->type)
	{
		case kPGPEvent_InitialEvent:
			DPA(("Encode --> Init Event:  \n"));
			break;
		case kPGPEvent_FinalEvent:
			DPA(("Encode --> Final Event\n"));
			break;
		case kPGPEvent_NullEvent:
		{
			PGPEventNullData  *d = &event->data.nullData;
			int progress =  (int)(( (float) d->bytesWritten /  d->bytesTotal) * 100);

			DPA(("%d ", progress));
			break;
		}
		case kPGPEvent_WarningEvent:
		{
			PGPEventWarningData *d = &event->data.warningData;
			char _errstr[256];
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->warning, 256, _errstr);
			DP((_T("Encode --> Warning Event: %d(%s) \n"), d->warning, _errstr));
			break;
		}
		case kPGPEvent_ErrorEvent:
		{
			PGPEventErrorData *d = &event->data.errorData;
			char _errstr[256];
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->error, 256, _errstr);
			DP((_T("Encode --> Error Event: %d(%s) \n"), d->error, _errstr));
			break;
		}
		case kPGPEvent_EncryptionEvent:
		{
			PGPEventEncryptionData  *d = &event->data.encryptionData;
			LPCTSTR cipher =  cipher_algor_table(d->cipherAlgorithm);

			DP((_T("Encode --> Data is encrypted with the %s Algorithm\n"), cipher));
			DPA(("    Session Key: [%d] ", (int)d->sessionKeyLength));
			for( unsigned int i=0; i < d->sessionKeyLength; i++ ) 
			{
				DPA(("%02x%s", d->sessionKey[i], (((i+1)%4) ? "":" ")));
			}
			DPA(("\n"));
			break;
		}
#if 0
		case kPGPEvent_EntropyEvent:
			DPA(("Entropy requested...\n"));
			break;
		case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData  *d = &event->data.passphraseData;
			PGPUInt32 numKeys = 0;
			PGPKeyIterRef keyIter = kInvalidPGPKeyIterRef;
			PGPKeyDBObjRef  theKey = kInvalidPGPKeyDBObjRef;
			PGPKeyID		keyID;

			DPA(("--> Passphrase Event: %s\n", d->fConventional?"Conventional":""));
			if(d->keyset)
			{
				err = pgpSdk->m_PgpSdkFunc.fPGPCountKeys(d->keyset, &numKeys);
				DPA(("    %d key%s: ", numKeys, numKeys==1?"":"s"));
				if (numKeys > 0)
				{
					err = pgpSdk->m_PgpSdkFunc.fPGPNewKeyIterFromKeySet(d->keyset, &keyIter); 
					if (IsntPGPError(err))
					{
						while (IsntPGPError( pgpSdk->m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( keyIter, kPGPKeyDBObjType_Key, &theKey)))
						{
							PGPChar	keyIDString[kPGPMaxKeyIDStringSize];
							err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyID (theKey, &keyID);
							err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString (&keyID, kPGPKeyIDString_Abbreviated, keyIDString);
						}
					}
					if( PGPKeyIterRefIsValid( keyIter ) )
						pgpSdk->m_PgpSdkFunc.fPGPFreeKeyIter( keyIter );
				}
			}
			break;
		}
#endif

		default:
			break;
	}

	return err;
}

static 	LPCTSTR analyzeEventTxt[] = {
	_T("Encrypted message"), 
	_T("Signed message"),
	_T("Detached signature"),
	_T("Key data "),
	_T("Non-pgp message"),
	_T("X.509 certificate"),
	_T("SMIME body")} ;

PGPError PgpSdkDecodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue)
{
	PGPError				err 		= kPGPError_NoErr;
	PgpPassphraseCallbackData *cbData = (PgpPassphraseCallbackData *)userValue;
	CPgpSdkAdapter *pgpSdk;

	assert(context != NULL);
	assert(event != NULL);
	assert(userValue != NULL);

	pgpSdk = (CPgpSdkAdapter *)cbData->pgpSdk;

	switch (event->type)
	{
	case kPGPEvent_InitialEvent:
		DPA(("Decode --> Init Event:  \n"));
		break;
	case kPGPEvent_FinalEvent:
		DPA(("Decode --> Final Event\n"));
		break;
	case kPGPEvent_NullEvent:
		{
			PGPEventNullData  *d = &event->data.nullData;
			int progress =  (int)(( (float) d->bytesWritten /  d->bytesTotal) * 100);

			DPA(("%d ", progress));
			break;
		}
	case kPGPEvent_WarningEvent:
		{
			PGPEventWarningData *d = &event->data.warningData;
			char _errstr[256];
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->warning, 256, _errstr);
			DP((_T("Decode --> Warning Event: %d(%s) \n"), d->warning, _errstr));
			break;
		}
	case kPGPEvent_ErrorEvent:
		{
			PGPEventErrorData *d = &event->data.errorData;
			char _errstr[256];
			GsmDecryptResult *result = &(cbData->result->decryptResult);
			if (result->type == GSM_DECRYPT_RESULT_NO_DEC)
			{
				result->type = GSM_DECRYPT_RESULT_OTHER;
			}
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->error, 256, _errstr);
			DP((_T("Decode --> Error Event: %d(%s) \n"), d->error, _errstr));
			break;
		}
	case kPGPEvent_DecryptionEvent:
		{
			PGPEventDecryptionData  *d = &event->data.decryptionData;
			LPCTSTR cipher = cipher_algor_table(d->cipherAlgorithm);
			GsmDecryptResult *result = &(cbData->result->decryptResult);

			result->type = GSM_DECRYPT_RESULT_OK;
			DP((_T("Decode --> Data was encrypted with the %s Algorithm\n"), cipher));
			DPA(("    Session Key: [%d] ", (int)d->sessionKeyLength));
			for( unsigned int i=0; i < d->sessionKeyLength; i++ ) 
			{
				DPA(("%02x%s", d->sessionKey[i], (((i+1)%4) ? "":" ")));
			}
			DPA(("\n"));
			break;
		}
	case kPGPEvent_AnalyzeEvent:
		{
			PGPEventAnalyzeData *d = &event->data.analyzeData;
			DP((_T("Decode --> Analyzing Type %d data: %s\n"), d->sectionType,
				(d->sectionType <= kPGPAnalyze_SMIMEBody ?analyzeEventTxt[d->sectionType]: _T("Unknown"))));
		}
		break;

	case kPGPEvent_BeginLexEvent:
		{
			PGPEventBeginLexData *d = &event->data.beginLexData;
			DPA(("Decode --> Begin decoding section %d\n",d->sectionNumber));
		}
		break;

	case kPGPEvent_EndLexEvent:
		{
			PGPEventEndLexData *d = &event->data.endLexData;
			DPA(("Decode --> End decoding section %d\n", d->sectionNumber));
		}	
		break;

	case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData  *d = &event->data.passphraseData;
			PGPUInt32 numKeys = 0;
			PGPKeyIterRef keyIter = kInvalidPGPKeyIterRef;
			PGPKeyDBObjRef  theKey = kInvalidPGPKeyDBObjRef;
			//PGPKeyDBObjRef	decryptKey 	= kInvalidPGPKeyDBObjRef;
			PGPKeyID		keyID;
			PGPChar	keyIDString[kPGPMaxKeyIDStringSize];
			PGPEventRecipientsData	*recipdata = (PGPEventRecipientsData *)cbData->recipientsData;
			LPTSTR passGot = NULL;

			DPA(("Decode --> Passphrase Event: %s\n", d->fConventional?"Conventional":""));

			if (cbData->bStopTry)
			{
				err = kPGPError_UserAbort;
				break;
			}

			if (!cbData->cb || cbData->triedTimes > 3)
			{
				err = kPGPError_BadPassphrase;
				break;
			}

			if (d->fConventional)
			{
				if (d->ESKsLength > 6 && d->ESKs[3] == 4)
				{
					int ret = 0;
					TCHAR info[16];

					cbData->triedTimes++;

					memset(info, 0, 16*sizeof(TCHAR));
					/* call user callback here */
					_snwprintf_s(info, 16, _TRUNCATE, _T("%d %d %d"), d->ESKs[4], d->ESKs[5], d->ESKs[6]);
					ret = cbData->cb(cbData->context, NULL, info, 0, &passGot);

					if(passGot)
					{
						cbData->bStopTry = ret;
						err = pgpSdk->m_PgpSdkFunc.fPGPAddJobOptions( event->job,
							pgpSdk->m_PgpSdkFunc.fPGPOPassphraseBuffer( context, (const PGPChar16 *)passGot, wcslen( passGot ) ),
							pgpSdk->m_PgpSdkFunc.fPGPOLastOption( context ) );
						free(passGot);
					}
					else
					{
						cbData->bStopTry = 1;
					}
				}
				else
				{
					err = kPGPError_UserAbort;
					break;
				}
			}

			if(d->keyset)
			{
				err = pgpSdk->m_PgpSdkFunc.fPGPCountKeys(d->keyset, &numKeys);
				DPA(("    %d key%s: ", numKeys, numKeys==1?"":"s"));
				if (numKeys > 0)
				{
					err = pgpSdk->m_PgpSdkFunc.fPGPNewKeyIterFromKeySet(d->keyset, &keyIter); 
					if (IsntPGPError(err))
					{
						if (IsntPGPError( pgpSdk->m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( keyIter, kPGPKeyDBObjType_Key, &theKey)))
						{
							err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyID (theKey, &keyID);

							if (!cbData->triedTimes)
							{
								err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString (&keyID, kPGPKeyIDString_Abbreviated, keyIDString);
								/* get cached passphrase */
								passGot = (LPTSTR)passcache_get((LPCTSTR)keyIDString);
							}

							err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString (&keyID, kPGPKeyIDString_Full, keyIDString);

							{
								TCHAR szUserId[256];
								size_t dataSize = 0;
								int ret = 0;

								cbData->triedTimes++;

								if(!passGot)
								{
									err = pgpSdk->GetKeyDataProperty(theKey, GsmKeyUserIDProperty_Name, szUserId, 256, &dataSize);
									/* call user callback here */
									ret = cbData->cb(cbData->context, szUserId, (LPCTSTR)keyIDString, 0, &passGot);
								}
								
								if(passGot)
								{
									cbData->bStopTry = ret;
									err = pgpSdk->m_PgpSdkFunc.fPGPAddJobOptions( event->job,
										pgpSdk->m_PgpSdkFunc.fPGPOPassphraseBuffer( context, (const PGPChar16 *)passGot, wcslen( passGot ) ),
										pgpSdk->m_PgpSdkFunc.fPGPOLastOption( context ) );

									err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString (&keyID, kPGPKeyIDString_Abbreviated, keyIDString);
									/* put cached passphrase */
									passcache_put((LPCTSTR)keyIDString, passGot, pgpSdk->m_ulTTL);
								}
								else
								{
									cbData->bStopTry = 1;
								}
							}
						}
					}
					if (passGot)
					{
						free(passGot);
					}
					if( PGPKeyIterRefIsValid( keyIter ) )
						pgpSdk->m_PgpSdkFunc.fPGPFreeKeyIter( keyIter );
				}
				else
				{
					GsmDecryptResult *result = &(cbData->result->decryptResult);

					result->type = GSM_DECRYPT_RESULT_NO_SECKEY;

					if (recipdata && recipdata->keyCount > 0)
					{
						err = pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString (&(recipdata->keyIDArray[0]), kPGPKeyIDString_Abbreviated, keyIDString);
						wcsncpy_s(result->noSecKeyId.szKey, 36, (LPCTSTR)keyIDString, _TRUNCATE);
					}
				}
			}
			break;
		}

		case kPGPEvent_OutputEvent:
			{
				PGPEventOutputData *d = &event->data.outputData;
				DPA(("Decode --> Output Event: %s\n", d->suggestedName ? d->suggestedName:""));
			}
			break;
		case kPGPEvent_DetachedSignatureEvent:
			{
				DPA(("Decode --> Detached Signature Event:\n"));
			}
			break;
		case  kPGPEvent_SignatureEvent:
			{
				PGPEventSignatureData *d = &event->data.signatureData;
				GsmVerifyResult *result = &(cbData->result->verifyResult);
				PGPChar keyIDstr[kPGPMaxKeyIDStringSize];
				memset(keyIDstr, 0, sizeof(keyIDstr));

				DPA(("Decode --> Signature Event:\n"));

				memset(result, 0, sizeof(GsmVerifyResult));
				
				pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString( &d->signingKeyID, kPGPKeyIDString_Abbreviated, keyIDstr);
				wcsncpy_s(result->signKeyId.szKey, 36, (LPCTSTR)keyIDstr, _TRUNCATE);
				result->creationTs = (unsigned long)d->creationTime;
				if (d->expirationPeriod)
				{
					result->expiresTs = (unsigned long)(d->creationTime + d->expirationPeriod);
				}

				if (d->verified)
				{
					result->type = GSM_VERIFY_RESULT_SIGSUM_VALID | GSM_VERIFY_RESULT_SIGSUM_GREEN;
				}
				else
				{
					result->type = GSM_VERIFY_RESULT_SIGSUM_RED;
					if (d->keyExpired)
					{
						result->type = GSM_VERIFY_RESULT_KEY_EXPIRED;
					}
					else if (d->keyRevoked)
					{
						result->type = GSM_VERIFY_RESULT_KEY_REVOKED;
					}
					else if (d->signingKey == NULL)
					{
						result->type = GSM_VERIFY_RESULT_KEY_MISSING;
					}
				}
			}
			break;
		case kPGPEvent_RecipientsEvent:
			{
				PGPEventRecipientsData  *d = &event->data.recipientsData;
				PGPEventRecipientsData	*recipdata = NULL;

				DPA(("Decode --> Recipients Event:\n"));

				recipdata = (PGPEventRecipientsData*)pgpSdk->m_PgpSdkFunc.fPGPNewData( 
						pgpSdk->m_PgpSdkFunc.fPGPGetDefaultMemoryMgr(), sizeof(PGPEventRecipientsData) ,  0 );

				if(recipdata) {
					recipdata->recipientSet = d->recipientSet;
					recipdata->conventionalPassphraseCount = d->conventionalPassphraseCount;
					recipdata->keyCount 	= d->keyCount;
					recipdata->keyIDArray 	= d->keyIDArray;

					pgpSdk->m_PgpSdkFunc.fPGPIncKeySetRefCount(d->recipientSet);

					cbData->recipientsData = (void *)recipdata;
				}
			}
			break;
	default:
		break;
	}

	return err;
}

PGPError PgpSdkVerifyEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue)
{
	PGPError				err 		= kPGPError_NoErr;
	PgpVerifyCallbackData *cbData = (PgpVerifyCallbackData *)userValue;
	CPgpSdkAdapter *pgpSdk;

	UNUSED(context);

	assert(context != NULL);
	assert(event != NULL);
	assert(userValue != NULL);

	pgpSdk = (CPgpSdkAdapter *)cbData->pgpSdk;

	switch (event->type)
	{
	case kPGPEvent_InitialEvent:
		DPA(("Verify --> Init Event:  \n"));
		break;
	case kPGPEvent_FinalEvent:
		DPA(("Verify --> Final Event\n"));
		break;
	case kPGPEvent_NullEvent:
		{
			PGPEventNullData  *d = &event->data.nullData;
			int progress =  (int)(( (float) d->bytesWritten /  d->bytesTotal) * 100);

			DPA(("%d% ", progress));
			break;
		}
	case kPGPEvent_WarningEvent:
		{
			PGPEventWarningData *d = &event->data.warningData;
			char _errstr[256];
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->warning, 256, _errstr);
			DP((_T("Verify --> Warning Event: %d(%s) \n"), d->warning, _errstr));
			break;
		}
	case kPGPEvent_ErrorEvent:
		{
			PGPEventErrorData *d = &event->data.errorData;
			char _errstr[256];
			GsmVerifyResult *result = cbData->result;
			if (result->type == GSM_VERIFY_RESULT_NO_VERIFY)
			{
				result->type = GSM_VERIFY_RESULT_SYS_ERROR;
			}
			pgpSdk->m_PgpSdkFunc.fPGPGetErrorString( d->error, 256, _errstr);
			DP((_T("Verify --> Error Event: %d(%s) \n"), d->error, _errstr));
			break;
		}

	case kPGPEvent_AnalyzeEvent:
		{
			PGPEventAnalyzeData *d = &event->data.analyzeData;
			DP((_T("Verify --> Analyzing Type %d data: %s\n"), d->sectionType,
				(d->sectionType <= kPGPAnalyze_SMIMEBody ?analyzeEventTxt[d->sectionType]: _T("Unknown"))));
		}
		break;

	case kPGPEvent_BeginLexEvent:
		{
			PGPEventBeginLexData *d = &event->data.beginLexData;
			DPA(("Verify --> Begin decoding section %d\n",d->sectionNumber));
		}
		break;

	case kPGPEvent_EndLexEvent:
		{
			PGPEventEndLexData *d = &event->data.endLexData;
			DPA(("Verify --> End decoding section %d\n", d->sectionNumber));
		}	
		break;

	case kPGPEvent_OutputEvent:
		{
			PGPEventOutputData *d = &event->data.outputData;
			DPA(("Verify --> Output Event: %s\n", d->suggestedName ? d->suggestedName:""));
		}
		break;
	case kPGPEvent_DetachedSignatureEvent:
		{
			DPA(("Verify --> Detached Signature Event:\n"));
		}
		break;
	case  kPGPEvent_SignatureEvent:
		{
			PGPEventSignatureData *d = &event->data.signatureData;
			GsmVerifyResult *result = cbData->result;
			PGPChar keyIDstr[kPGPMaxKeyIDStringSize];
			memset(keyIDstr, 0, sizeof(keyIDstr));

			DPA(("Verify --> Signature Event:\n"));

			memset(result, 0, sizeof(GsmVerifyResult));

			pgpSdk->m_PgpSdkFunc.fPGPGetKeyIDString( &d->signingKeyID, kPGPKeyIDString_Abbreviated, keyIDstr);
			wcsncpy_s(result->signKeyId.szKey, 36, (LPCTSTR)keyIDstr, _TRUNCATE);
			result->creationTs = (unsigned long)d->creationTime;
			if (d->expirationPeriod)
			{
				result->expiresTs = (unsigned long)(d->creationTime + d->expirationPeriod);
			}

			if (d->verified)
			{
				result->type = GSM_VERIFY_RESULT_SIGSUM_VALID | GSM_VERIFY_RESULT_SIGSUM_GREEN;
			}
			else
			{
				result->type = GSM_VERIFY_RESULT_SIGSUM_RED;
				if (d->keyExpired)
				{
					result->type = GSM_VERIFY_RESULT_KEY_EXPIRED;
				}
				else if (d->keyRevoked)
				{
					result->type = GSM_VERIFY_RESULT_KEY_REVOKED;
				}
				else if (d->signingKey == NULL)
				{
					result->type = GSM_VERIFY_RESULT_KEY_MISSING;
				}
			}
		}
		break;
	default:
		break;
	}

	return err;
}

// static LPCTSTR hash_algor_table(int algor)
// {
// 	switch (algor )
// 	{
// 	case kPGPHashAlgorithm_MD5: 		return _T("MD5");
// 	case kPGPHashAlgorithm_SHA: 		return _T("SHA-1");
// 	case kPGPHashAlgorithm_RIPEMD160: 	return _T("RIPE-MD-160");
// 	case kPGPHashAlgorithm_SHA256:		return _T("SHA-128");
// 	case kPGPHashAlgorithm_SHA384:		return _T("SHA-384");
// 	case kPGPHashAlgorithm_SHA512:		return _T("SHA-512");						
// 	default:							return _T("Invalid");
// 	}
// }


static LPCTSTR cipher_algor_table(int algor)
{
	switch (algor )
	{
	case kPGPCipherAlgorithm_IDEA: 		return _T("IDEA");
	case kPGPCipherAlgorithm_3DES: 		return _T("3-DES");
	case kPGPCipherAlgorithm_CAST5: 	return _T("CAST-5");
	case kPGPCipherAlgorithm_AES128: 	return _T("AES-128");
	case kPGPCipherAlgorithm_AES192: 	return _T("AES-192");
	case kPGPCipherAlgorithm_AES256: 	return _T("AES-256");	
	case kPGPCipherAlgorithm_Blowfish:  return _T("BlowFish");
	case kPGPCipherAlgorithm_Twofish256: return _T("TwoFish");
	default:	 return _T("Invalid");
	}
}

// static LPCTSTR key_algor_table(int keytype)
// {
// 	switch(keytype)
// 	{
// 	case kPGPPublicKeyAlgorithm_RSA:			return _T("RSA");
// 	case kPGPPublicKeyAlgorithm_RSAEncryptOnly:	return _T("RSA Encrypt Only");
// 	case kPGPPublicKeyAlgorithm_RSASignOnly:	return _T("RSA Sign Only");
// 	case kPGPPublicKeyAlgorithm_ElGamal:		return _T("Elgamal");
// 	case kPGPPublicKeyAlgorithm_DSA:			return _T("DSA");
// 	case kPGPPublicKeyAlgorithm_ECEncrypt:		return _T("EC-Enc");
// 	case kPGPPublicKeyAlgorithm_ECSign:			return _T("EC-Sign");
// 	default:									return _T("Invalid");
// 	}
// }

// static LPCTSTR compression_algor_table(int algor)
// {
// 	switch(algor)
// 	{
// 	case kPGPCompressionAlgorithm_None:			return _T("None");
// 	case kPGPCompressionAlgorithm_ZLIB:			return _T("ZLIB");
// 	case kPGPCompressionAlgorithm_ZIP:			return _T("Zip");
// 	case kPGPCompressionAlgorithm_BZIP2:		return _T("BZip2");
// 	default:									return _T("Invalid");
// 	}
// }

PGPError CPgpSdkAdapter::GetActualRecipentKey(
								 PGPKeyDBObjRef		key, 
								 PGPKeyID	const	*keyIDArray,
								 PGPUInt32			keyCount,
								 PGPKeyDBObjRef		*outRecipientKey)
{
	PGPError		err				= kPGPError_NoErr;
	PGPKeyDBObjRef  topKey			= kInvalidPGPKeyDBObjRef;
	PGPKeyDBObjRef  subKey			= kInvalidPGPKeyDBObjRef;
	PGPKeyDBObjRef  foundKey		= kInvalidPGPKeyDBObjRef;
	PGPKeyIterRef   iter			= kInvalidPGPKeyIterRef;
	PGPUInt32		i;

	if(!outRecipientKey) 
		return(kPGPError_BadParams);

	if(keyCount > 0)
	{
		PGPKeyID  keyID;

		topKey = m_PgpSdkFunc.fPGPPeekKeyDBObjKey(key);

		m_PgpSdkFunc.fPGPGetKeyID( topKey, &keyID );

		/* check top key   */
		for(i = 0;i < keyCount; i++)
			if( m_PgpSdkFunc.fPGPCompareKeyIDs( &keyIDArray[i], &keyID) == 0 )
			{
				foundKey = topKey;
				goto done;
			}

			/* check all subkeys   */
			err = m_PgpSdkFunc.fPGPNewKeyIterFromKeyDB(m_PgpSdkFunc.fPGPPeekKeyDBObjKeyDB(topKey) , &iter); CKERR;

			/* Enumerate SubKeys   */
			for( m_PgpSdkFunc.fPGPKeyIterSeek( iter, topKey );
				IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj(iter, kPGPKeyDBObjType_SubKey, &subKey )); )
			{
				m_PgpSdkFunc.fPGPGetKeyID( subKey, &keyID );

				for(i = 0;i < keyCount; i++)
					if( m_PgpSdkFunc.fPGPCompareKeyIDs( &keyIDArray[i], &keyID) == 0 )
					{
						foundKey = subKey;
						goto done;
					}
			}
	}

done:
	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	*outRecipientKey = foundKey;

	return err;
}

PGPError CPgpSdkAdapter::ParseKeyserverString(
	LPCTSTR			keyserver,
	PGPKeyServerEntry **	outKeyserver)
{
	PGPError				err = kPGPError_NoErr;
	PGPKeyServerEntry *		entry = NULL;
	PGPKeyServerProtocol	protocol = kPGPKeyServerProtocol_LDAP;
	PGPBoolean				defaultPort = TRUE;
	PGPSize					size = 0;
	LPTSTR					p = NULL;
	LPTSTR					protocolString = NULL;
	LPTSTR					serverString = NULL;
	LPTSTR					portString = NULL;
	LPTSTR					pathString = NULL;
	int						port = 0;
	
	assert(keyserver);
	assert(outKeyserver);
	
	entry = (PGPKeyServerEntry *)m_PgpSdkFunc.fPGPNewData(m_PgpSdkFunc.fPGPPeekContextMemoryMgr(m_context), sizeof(PGPKeyServerEntry), kPGPMemoryMgrFlags_Clear);
	if (!entry)
		return kPGPError_OutOfMemory;
	
	p = (LPTSTR)wcsstr(keyserver, L"://");
	if (!p)
		protocol = kPGPKeyServerProtocol_LDAP;
	else
	{
		size = (p - keyserver) + 1;
		protocolString = (LPTSTR)m_PgpSdkFunc.fPGPNewData(m_PgpSdkFunc.fPGPPeekContextMemoryMgr(m_context), size*sizeof(TCHAR), 0);
		if (!protocolString)
			goto done;
		
		memcpy(protocolString, keyserver, (size - 1)*size*sizeof(TCHAR));
		protocolString[size - 1] = 0;
		
		if (!_wcsicmp(protocolString, L"ldap"))
			protocol = kPGPKeyServerProtocol_LDAP;
		else if (!_wcsicmp(protocolString, L"ldaps"))
			protocol = kPGPKeyServerProtocol_LDAPS;
		else if (!_wcsicmp(protocolString, L"http"))
			protocol = kPGPKeyServerProtocol_HTTP;
		else if (!_wcsicmp(protocolString, L"https"))
			protocol = kPGPKeyServerProtocol_HTTPS;
		else
			protocol = kPGPKeyServerProtocol_LDAP;
	}
	
	if (p)
	{
		p += wcslen(L"://");
		size = wcslen(p) + 1;
		serverString = (LPTSTR)m_PgpSdkFunc.fPGPNewData(m_PgpSdkFunc.fPGPPeekContextMemoryMgr(m_context), size*sizeof(TCHAR), 0);
		if (!serverString)
			goto done;
		
		memcpy(serverString, p, (size - 1)*sizeof(TCHAR));
		serverString[size - 1] = 0;
	}
	else
	{
		size = wcslen(keyserver) + 1;
		serverString = (LPTSTR)m_PgpSdkFunc.fPGPNewData(m_PgpSdkFunc.fPGPPeekContextMemoryMgr(m_context), size*sizeof(TCHAR), 0);
		if (!serverString)
			goto done;
		
		memcpy((void *)serverString, (void *)keyserver, (size - 1)*sizeof(TCHAR));
		serverString[size - 1] = 0;
	}
	
	if ((p = wcschr(serverString, L':')) != NULL)
	{
		portString = p + 1;
		*p = L'\0';
		
		if ((p = wcschr(portString, L'/')) !=NULL)
		{
			pathString = p + 1;
			*p = L'\0';
		}
		
		port = _wtoi(portString);
		
		switch (port)
		{
			case kPGPldap_DefaultPort:
				defaultPort = (protocol == kPGPKeyServerProtocol_LDAP ? TRUE : FALSE);
				break;
			case kPGPldap_DefaultSecurePort:
				defaultPort = (protocol == kPGPKeyServerProtocol_LDAPS ? TRUE : FALSE);
				break;
			case 80:
				defaultPort = (protocol == kPGPKeyServerProtocol_HTTP ? TRUE : FALSE);
				break;
			default:
				defaultPort = FALSE;
				break;
		}
	}
	else if ((p = wcschr(serverString, L'/')) !=NULL)
	{
		pathString = p + 1;
		*p = L'\0';
	}
	else
		serverString[size - 1] = L'\0';
	
	switch (protocol)
	{
		default:
		case kPGPKeyServerProtocol_LDAP:
			port = kPGPldap_DefaultPort;
			break;
		case kPGPKeyServerProtocol_LDAPS:
			port = kPGPldap_DefaultSecurePort;
			break;
		case kPGPKeyServerProtocol_HTTP:
			port = 80;
			break;
		case kPGPKeyServerProtocol_HTTPS:
			protocol = kPGPKeyServerProtocol_HTTP;
			port = 443;
			break;
	}
	
	entry->title = (PGPChar*)serverString;
	entry->domain[0] = 0;			/* Ditching this */
	wcsncpy_s((LPTSTR)entry->serverDNS, kMaxServerNameLength, serverString, _TRUNCATE);
	entry->serverPort = port;
	entry->protocol = protocol;
	entry->type = kPGPKeyServerClass_PGP;
	
	if (pathString)
		wcsncpy_s((LPTSTR)entry->keystoredn, kMaxKeyStoreDNLength, pathString, _TRUNCATE);
	else
		entry->keystoredn[0] = 0;
	
	entry->clientAuthKeyIDString[0] = 0;
	entry->authKeyIDString[0] = 0;
	//entry->authAlg = 0;
	//entry->flags = 0;
	
	if (IsntPGPError(err))
		*outKeyserver = entry;
	
done:

	if (protocolString)
		m_PgpSdkFunc.fPGPFreeData(protocolString);
	
	if (IsPGPError(err))
	{
		if (serverString)
			m_PgpSdkFunc.fPGPFreeData(serverString);
		
		if (entry)
			m_PgpSdkFunc.fPGPFreeData(entry);
	}
	
	return err;
}

GsmErrorT CPgpSdkAdapter::ProcessKeyServerOperation(LPCTSTR pszServer, int operation, 
									GsmKeyID *keyId, LPCTSTR pszEmail, 
									GsmKeyHandle **hKeys, int *nKeys,
									GsmKeyServerStateChangedCallbackF cb, void *context)
{
	PGPError				err			= kPGPError_NoErr;
	PGPtlsContextRef		tlsContext  = kInvalidPGPtlsContextRef;
	PGPKeyServerRef			server		= kInvalidPGPKeyServerRef;
	PGPtlsSessionRef		tls			= kInvalidPGPtlsSessionRef;
	PGPFilterRef			filter		= kInvalidPGPFilterRef;
	PGPKeyDBRef				keyDB	 	= kInvalidPGPKeyDBRef;
	PGPKeyIterRef			iter	 	= kInvalidPGPKeyIterRef;
	PGPKeyServerEntry		*entry		= NULL;
	PGPKeyID				pgpKeyID;
	PGPUInt32				uNumKeys	= 0;
	PGPUInt32				keyCount	= 0;
	PGPKeyDBObjRef			theKey		= kInvalidPGPKeyDBObjRef;
	PGPKeySetRef			keySet		= kInvalidPGPKeySetRef;
	PGPKeySetRef			keysetFailed	= kInvalidPGPKeySetRef;
	PGPKeyServerThreadStorageRef	previousStorage;
	GsmKeyServerStateCbData data;

	if (!pszServer)
	{
		return GSM_ERR_GENERAL;
	}

	data.cb = cb;
	data.cbData = context;

	m_PgpSdkFunc.fPGPKeyServerCreateThreadStorage(&previousStorage);

	err = ParseKeyserverString(pszServer, &entry); CKERR;

	err = m_PgpSdkFunc.fPGPNewKeyServer (m_context,
		entry->type,
		&server,
		m_PgpSdkFunc.fPGPONetHostName (m_context, entry->serverDNS, (PGPUInt16)entry->serverPort),
		m_PgpSdkFunc.fPGPOKeyServerProtocol (m_context, entry->protocol),
		m_PgpSdkFunc.fPGPOKeyServerAccessType (m_context, kPGPKeyServerAccessType_Normal),
		m_PgpSdkFunc.fPGPOKeyServerKeySpace (m_context, kPGPKeyServerKeySpace_Normal),
		m_PgpSdkFunc.fPGPOKeyServerKeyStoreDN (m_context, (const PGPChar16 *)&entry->keystoredn),
		m_PgpSdkFunc.fPGPOLastOption (m_context));	CKERR;

	m_PgpSdkFunc.fPGPSetKeyServerEventHandler(server, PgpSdkKeyServerEventHandler, (void *)&data);

	if ((entry->protocol == kPGPKeyServerProtocol_LDAPS) ||
		(entry->protocol == kPGPKeyServerProtocol_HTTPS))
	{
		err = m_PgpSdkFunc.fPGPNewTLSContext(m_context, &tlsContext);	CKERR;
		err = m_PgpSdkFunc.fPGPNewTLSSession (tlsContext, &tls);	CKERR;
	}

	DP((_T("    Opening connection to %s\n"),entry->serverDNS));
	err = m_PgpSdkFunc.fPGPKeyServerOpen( server, tls ); CKERR;

	switch (operation)
	{
	case GETVKDKEY:
		{
			assert(hKeys && nKeys);
			*hKeys = NULL;
			*nKeys = 0;

			if (pszEmail)
			{
				err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter(m_context, kPGPUserIDProperty_EmailAddress,
					pszEmail, wcslen(pszEmail)*sizeof(pszEmail),
					kPGPMatchCriterion_SubString, &filter); CKERR;
			}
			else
			{
				err = m_PgpSdkFunc.fPGPNewKeyIDFromString((const PGPChar16 *)keyId->szKey, kPGPPublicKeyAlgorithm_Invalid, &pgpKeyID); CKERR;
				err = m_PgpSdkFunc.fPGPNewKeyDBObjDataFilter (m_context, 
					kPGPKeyProperty_KeyID, &pgpKeyID, sizeof(PGPKeyID), 
					kPGPMatchCriterion_Equal, &filter); CKERR;
			}

			err = m_PgpSdkFunc.fPGPQueryKeyServer (server, filter, &keyDB); CKERR;

			err = m_PgpSdkFunc.fPGPCountKeysInKeyDB (keyDB, &uNumKeys); CKERR;
			DP((_T("Get %d matched keys from keyserver(%s)\n"), uNumKeys, pszServer));
			*nKeys = uNumKeys;
			if (uNumKeys)
			{
				PGPChar	keyIDString[kPGPMaxKeyIDStringSize];

				*hKeys = (GsmKeyHandle *)calloc (uNumKeys, sizeof *hKeys); CKNULL(*hKeys);
				memset(*hKeys, 0, uNumKeys*sizeof(*hKeys));
				
				//m_PgpSdkFunc.fPGPCopyKeys (m_PgpSdkFunc.fPGPPeekKeyDBRootKeySet (keyDB), m_keyDB, NULL);

				err = m_PgpSdkFunc.fPGPNewKeyIterFromKeyDB( keyDB, &iter); CKERR;
				while( IsntPGPError( m_PgpSdkFunc.fPGPKeyIterNextKeyDBObj( iter, kPGPKeyDBObjType_Key, &theKey) ) )
				{
					//(*hKeys)[keyCount++] = (GsmKeyHandle)theKey;
					err = m_PgpSdkFunc.fPGPCopyKeyDBObj (theKey, m_keyDB, (PGPKeyDBObjRef *)&((*hKeys)[keyCount]));
					err = m_PgpSdkFunc.fPGPGetKeyID ((PGPKeyDBObjRef)(*hKeys)[keyCount], &pgpKeyID); 
					err = m_PgpSdkFunc.fPGPGetKeyIDString (&pgpKeyID, kPGPKeyIDString_Abbreviated, keyIDString); 
					keyCount++;
				}
			}
		}
		break;
		
	case SENDKEY:
		{
			FindKeyByKeyID(*keyId, (GsmKeyHandle *)&theKey);
			if (theKey != kInvalidPGPKeyDBObjRef)
			{
				err = m_PgpSdkFunc.fPGPNewOneKeySet(theKey, &keySet); CKERR;
				err = m_PgpSdkFunc.fPGPUploadToKeyServer (server,
					keySet, &keysetFailed);	CKERR;
			}
		}
		break;

	case DISABLEKEY:
		{
			FindKeyByKeyID(*keyId, (GsmKeyHandle *)&theKey);
			if (theKey != kInvalidPGPKeyDBObjRef)
			{
				err = m_PgpSdkFunc.fPGPNewOneKeySet(theKey, &keySet); CKERR;
				err = m_PgpSdkFunc.fPGPDisableFromKeyServer (server,
					keySet, &keysetFailed); CKERR;
			}
		}
		break;

	case DELETEKEY:
		{
			FindKeyByKeyID(*keyId, (GsmKeyHandle *)&theKey);
			if (theKey != kInvalidPGPKeyDBObjRef)
			{
				err = m_PgpSdkFunc.fPGPNewOneKeySet(theKey, &keySet); CKERR;
				err = m_PgpSdkFunc.fPGPDeleteFromKeyServer (server,
					keySet, &keysetFailed); CKERR;
			}
		}
		break;

	default:
		break;
	}

done:

	if(PGPKeyServerRefIsValid(server))
		m_PgpSdkFunc.fPGPKeyServerClose(server);  

	if( PGPtlsSessionRefIsValid( tls ) )
		m_PgpSdkFunc.fPGPFreeTLSSession( tls );

	if(PGPKeyServerRefIsValid(server))
		m_PgpSdkFunc.fPGPFreeKeyServer(server);

	if( PGPtlsContextRefIsValid(tlsContext))
		m_PgpSdkFunc.fPGPFreeTLSContext(tlsContext);

	if( PGPFilterRefIsValid( filter ) )
		m_PgpSdkFunc.fPGPFreeFilter( filter );

	if( PGPKeySetRefIsValid (keySet) )
		m_PgpSdkFunc.fPGPFreeKeySet(keySet);

	if (PGPKeySetRefIsValid (keysetFailed))
		m_PgpSdkFunc.fPGPFreeKeySet (keysetFailed);

	if( PGPKeyIterRefIsValid( iter ) )
		m_PgpSdkFunc.fPGPFreeKeyIter( iter );

	if( PGPKeyDBRefIsValid( keyDB ) )
		m_PgpSdkFunc.fPGPFreeKeyDB( keyDB );

	if (entry)
	{
		if (entry->title)
		{
			m_PgpSdkFunc.fPGPFreeData(entry->title);
		}
		m_PgpSdkFunc.fPGPFreeData(entry);
	}

	m_PgpSdkFunc.fPGPKeyServerDisposeThreadStorage(previousStorage);

	return ConvertPgpError2GsmError(err);
}

PGPError PgpSdkKeyServerEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue)
{
	GsmKeyServerStateCbData *stateCb = (GsmKeyServerStateCbData *)userValue;

	UNUSED(context);

	switch (event->type)
	{
	case kPGPEvent_KeyServerEvent:
		{
			PGPEventKeyServerData data = event->data.keyServerData;
			DPA(("Ker Server State: %d\n", data.state));

			if (stateCb && stateCb->cb)
			{
				if (stateCb->cb(stateCb->cbData, (GsmKeyServerState)data.state))
				{
					return kPGPError_UserAbort;
				}
			}
		}
		break;
	default:
		break;
	}

	return kPGPError_NoErr;
}

#if 0
static PGPError sdaEventHandlerProcPtr(PGPContextRef context,
	struct PGPsdaEvent *event,
	PGPUserValue userValue)
{
	switch (event->type)
	{
	case kPGPsdaEvent_NullEvent:
		{
			/* show process */
			PGPsdaEventNullData  *d = &event->data.nullData;
			int progress =  (int)(( (float) d->bytesWritten /  d->bytesTotal) * 100);

			DPA(("%d ", progress));
		}
		break;
	case kPGPsdaEvent_InitialEvent:
		DPA(("SDA Event --> Init Event:  \n"));
		break;
	case kPGPsdaEvent_FinalEvent:
		DPA(("SDA Event --> Final Event:  \n"));
		break;
	case kPGPsdaEvent_AnalyzeEvent:
		{
			PGPsdaEventAnalyzeData *d = &event->data.analyzeData;
			DP((_T("Verify --> Analyzing Type %d data: %s\n"), d->sectionType,
				(d->sectionType <= kPGPAnalyze_SMIMEBody ?analyzeEventTxt[d->sectionType]: _T("Unknown"))));
		}
		break;
	case kPGPsdaEvent_BeginLexEvent:
		{
			PGPsdaEventBeginLexData *d = &event->data.beginLexData;
			DPA(("Decode --> Begin decoding section %d\n",d->sectionNumber));
		}
		break;
	case kPGPsdaEvent_EndLexEvent:
		{
			PGPsdaEventEndLexData *d = &event->data.endLexData;
			DPA(("Verify --> End decoding section %d\n", d->sectionNumber));
		}
		break;
	case kPGPsdaEvent_ErrorEvent:
		{
			PGPsdaEventErrorData *d = &event->data.errorData;
			
			DPA(("Decode --> Error Event: %d \n", d->error));
		}
		break;
	case kPGPsdaEvent_WarningEvent:
		{
			PGPsdaEventWarningData *d = &event->data.warningData;
			DPA(("Decode --> Warning Event: %d \n", d->warning));
		}
		break;
	case kPGPsdaEvent_NewObjectEvent:
		{
			PGPsdaEventNewObjectData *d = &event->data.newObjectData;
			DP((_T("Process name: %s; internal name %s, type %d \n"), 
				d->name, d->internalName, d->type));
			//d->internalName = (PGPChar *)malloc(strlen(d->name)+1);
			//strcpy(d->internalName, d->name);
		}
		break;
	case kPGPsdaEvent_AskFileEvent:
		{
			PGPsdaEventAskFileData *d = &event->data.askFileData;
			DP((_T("Old file name: %s \n"), d->oldName));
		}
		break;
	case kPGPsdaEvent_FreeFileEvent:
		{
			PGPsdaEventFreeFileData *d = &event->data.freeFileData;
			DP((_T("New file name: %s \n"), d->newName));
		}
		break;
	case kPGPsdaEvent_AskADKEvent:
		{
			PGPsdaEventAskADKData *d = &event->data.askADKData;
			DPA(("kPGPsdaEvent_AskADKEvent \n"));
		}
		break;
	case kPGPsdaEvent_FreeADKEvent:
		{
			PGPsdaEventFreeADKData *d = &event->data.freeADKData;
			DPA(("kPGPsdaEvent_FreeADKEvent \n"));
		}
		break;
	default:
		break;
	}

	return kPGPError_NoErr;
}

static PGPError sdaDecodeEventHandlerProc(PGPContextRef context,
	struct PGPsdaEvent *event,
	PGPUserValue userValue)
{
	switch (event->type)
	{
	case kPGPsdaEvent_NullEvent:
		{
			/* show process */
			PGPsdaEventNullData  *d = &event->data.nullData;
			int progress =  (int)(( (float) d->bytesWritten /  d->bytesTotal) * 100);

			DPA(("%d ", progress));
		}
		break;
	case kPGPsdaEvent_InitialEvent:
		DPA(("SDA Event --> Init Event:  \n"));
		break;
	case kPGPsdaEvent_FinalEvent:
		DPA(("SDA Event --> Final Event:  \n"));
		break;
	case kPGPsdaEvent_AnalyzeEvent:
		{
			PGPsdaEventAnalyzeData *d = &event->data.analyzeData;
			DPA(("Verify --> Analyzing Type %d data: %s\n", d->sectionType,
				(d->sectionType <= kPGPAnalyze_SMIMEBody ?analyzeEventTxt[d->sectionType]: "Unknown")));
		}
		break;
	case kPGPsdaEvent_BeginLexEvent:
		{
			PGPsdaEventBeginLexData *d = &event->data.beginLexData;
			DPA(("Decode --> Begin decoding section %d\n",d->sectionNumber));
		}
		break;
	case kPGPsdaEvent_EndLexEvent:
		{
			PGPsdaEventEndLexData *d = &event->data.endLexData;
			DPA(("Verify --> End decoding section %d\n", d->sectionNumber));
		}
		break;
	case kPGPsdaEvent_ErrorEvent:
		{
			PGPsdaEventErrorData *d = &event->data.errorData;

			DPA(("Decode --> Error Event: %d \n", d->error));
		}
		break;
	case kPGPsdaEvent_WarningEvent:
		{
			PGPsdaEventWarningData *d = &event->data.warningData;
			DPA(("Decode --> Warning Event: %d \n", d->warning));
		}
		break;
	case kPGPsdaEvent_NewObjectEvent:
		{
			/* Need to set the file name on process bar window */
			PGPsdaEventNewObjectData *d = &event->data.newObjectData;
			DPA(("Process name: %s; internal name %s, type %d \n", 
				d->name, d->internalName, d->type));
		}
		break;
	case kPGPsdaEvent_AskFileEvent:
		{
			// TODO: How to get new file name? 
			PGPsdaEventAskFileData *d = &event->data.askFileData;
			DPA(("Old file name: %s \n", d->oldName));
		}
		break;
	case kPGPsdaEvent_FreeFileEvent:
		{
			PGPsdaEventFreeFileData *d = &event->data.freeFileData;
			DPA(("New file name: %s \n", d->newName));
		}
		break;
	case kPGPsdaEvent_AskADKEvent:
		{
			PGPsdaEventAskADKData *d = &event->data.askADKData;
			DPA(("kPGPsdaEvent_AskADKEvent \n"));
		}
		break;
	case kPGPsdaEvent_FreeADKEvent:
		{
			PGPsdaEventFreeADKData *d = &event->data.freeADKData;
			DPA(("kPGPsdaEvent_FreeADKEvent \n"));
		}
		break;
	default:
		break;
	}

	return kPGPError_NoErr;
}
#endif
