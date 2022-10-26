/*
Written by Derek Zheng
March 2008
*/

#ifndef _GSM_TYPE_H_
#define _GSM_TYPE_H_

#ifdef GSMSDK_EXPORTS
#define GSMSDK_API __declspec(dllexport)
#else
#define GSMSDK_API __declspec(dllimport)
#endif

#define GSM_MAX_USER_NAME_LEN	128

/* zip compression levels */
#define GSM_Z_NO_COMPRESSION         0
#define GSM_Z_BEST_SPEED             1
#define GSM_Z_BEST_COMPRESSION       9
#define GSM_Z_DEFAULT_COMPRESSION  (-1)

typedef enum
{
	GSM_SDK_TYPE_UNKNOWN = -1,
	GSM_SDK_TYPE_PGP,
	//GSM_SDK_TYPE_GPG,
	GSM_SDK_TYPE_MS_EFS
	//GSM_SDK_TYPE_VOLTAGE,
	//GSM_SDK_TYPE_JAVA_CRYPTO
} GsmSdkTypeEnum;

typedef enum
{
	GSM_PUBKEY_ALGORITHM_UNKNOWN = -1,
	GSM_PUBKEY_ALGORITHM_RSA = 1,
	GSM_PUBKEY_ALGORITHM_RSA_E = 2,		// RSA Encrypt Only
	GSM_PUBKEY_ALGORITHM_RSA_S = 3,		// RSA Sign Only
	GSM_PUBKEY_ALGORITHM_ELG_E = 16,	// A.K.A.Diffie-Hellman Encrypt Only
	GSM_PUBKEY_ALGORITHM_DSA = 17,
	GSM_PUBKEY_ALGORITHM_EC_E = 18,		// EC Encrypt Only
	GSM_PUBKEY_ALGORITHM_EC_S = 19,		// EC Sign Only
	GSM_PUBKEY_ALGORITHM_ELG = 20
} GsmPubKeyAlgorithmEnum;

typedef enum
{
	GSM_KEY_BITS_1024 = 1024,
	GSM_KEY_BITS_2048 = 2048,
	GSM_KEY_BITS_4096 = 4096
} GsmKeyBitsEnum;

typedef enum {
	GSM_KEYGEN_NONE    = 0,
	GSM_KEYGEN_DSA_ELG = 1,	// DSA Sign, ELG Encrypt(subkey)
	GSM_KEYGEN_DSA_RSA = 2, // DSA Sign, RSA Encrypt(subkey)
	GSM_KEYGEN_DSA_SIG = 3,	// DSA Sign Only
	GSM_KEYGEN_RSA_SIG = 4, // RSA Sign Only
	GSM_KEYGEN_RSA     = 5, // ?
	GSM_KEYGEN_RSA_RSA = 6 /*PGP*/ // RSA Sign, RSA Encrypt(subkey)
} GsmKeyGenType;

typedef enum
{
	GSM_KEY_TYPE_ALL = 0,
	GSM_KEY_TYPE_PUBLIC,
	GSM_KEY_TYPE_SECRET,
	GSM_KEY_TYPE_ENCRYPT,
	GSM_KEY_TYPE_SIGN
} GsmKeyTypeEnum;

typedef enum
{
	GSM_SYMM_CIPHER_ALGO_UNKNOWN = -1,
	GSM_SYMM_CIPHER_ALGO_PLAINTEXT = 0,
	GSM_SYMM_CIPHER_ALGO_IDEA = 1,
	GSM_SYMM_CIPHER_ALGO_3DES = 2,
	GSM_SYMM_CIPHER_ALGO_CAST5 = 3,
	GSM_SYMM_CIPHER_ALGO_BLOWFISH = 4,
	GSM_SYMM_CIPHER_ALGO_AES128 = 7,
	GSM_SYMM_CIPHER_ALGO_AES192 = 8,
	GSM_SYMM_CIPHER_ALGO_AES256 = 9
} GsmSymmCipherAlgo;

/* The value should be same with PGPKeyDBObjProperty when adding new items */
typedef enum
{
	GsmKeyProperty_Invalid 						= 0,

	/* Generic numeric properties */
	GsmKeyProperty_ObjectType					= 20,

	/* Key boolean properties */	
	GsmKeyProperty_IsSecret 					= 100,
	GsmKeyProperty_IsAxiomatic					= 101,
	GsmKeyProperty_IsRevoked					= 102,
	GsmKeyProperty_IsDisabled					= 103,
	GsmKeyProperty_IsNotCorrupt					= 104,
	GsmKeyProperty_IsExpired					= 105,
	GsmKeyProperty_NeedsPassphrase				= 106,
	GsmKeyProperty_HasUnverifiedRevocation		= 107,
	GsmKeyProperty_CanEncrypt					= 108,
	GsmKeyProperty_CanDecrypt					= 109,
	GsmKeyProperty_CanSign						= 110,
	GsmKeyProperty_CanVerify					= 111,
	GsmKeyProperty_IsEncryptionKey				= 112,
	GsmKeyProperty_IsSigningKey					= 113,
	GsmKeyProperty_IsSecretShared				= 114,
	GsmKeyProperty_IsRevocable					= 115,
	GsmKeyProperty_HasThirdPartyRevocation		= 116,
	GsmKeyProperty_HasCRL						= 117,
	GsmKeyProperty_IsOnToken					= 118,
	GsmKeyProperty_IsStubKey					= 119,	/* used to filter dummy keys from inclusive key set */
	GsmKeyProperty_IsX509WrapperKey				= 120,	/* the key was created after the import of X.509 cert */
	GsmKeyProperty_AuthenticationRequired		= 121,	/* GsmKeyProperty_NeedsPassphrase and not cached */
	GsmKeyProperty_IsSubkeyOnToken				= 122,	/* is any of subkeys on token? Only valid for top key. */
	GsmKeyProperty_IsOnCSP						= 123,
	GsmKeyProperty_IsPassphraseCachedPersistent = 124,	/* is this key's passphrase in the persistent cache?*/
	GsmKeyProperty_IsSubkeyOnCSP				= 125, /* is any of subkeys on csp? Only valid for top key. */
	GsmKeyProperty_IsOnTokenAndKeyring			= 126, /* is the private portion of the key on the token and on the keyring */

	/* Key numeric properties */	
	GsmKeyProperty_AlgorithmID 					= 200,
	GsmKeyProperty_Bits							= 201,	
	GsmKeyProperty_Trust						= 202,
	GsmKeyProperty_Validity						= 203,
	GsmKeyProperty_LockingAlgorithmID			= 204,
	GsmKeyProperty_LockingBits					= 205,
	GsmKeyProperty_Flags						= 206,
	GsmKeyProperty_HashAlgorithmID				= 207,
	GsmKeyProperty_Version						= 208,
	GsmKeyProperty_KeyServerPreferences			= 209,
	GsmKeyProperty_TokenNum						= 210,
	GsmKeyProperty_Features						= 211,	

	/* Key time properties */	
	GsmKeyProperty_Creation 					= 300,
	GsmKeyProperty_Expiration					= 301,
	GsmKeyProperty_CRLThisUpdate				= 302,
	GsmKeyProperty_CRLNextUpdate				= 303,
	GsmKeyProperty_RevocationTime				= 305,

	/* Key data (variable sized) properties */
	GsmKeyProperty_Fingerprint					= 401,
	GsmKeyProperty_KeyID						= 402,
	//GsmKeyProperty_PreferredAlgorithms			= 403,
	//GsmKeyProperty_ThirdPartyRevocationKeyID 	= 404,
	//GsmKeyProperty_KeyData						= 405,
	//GsmKeyProperty_X509MD5Hash					= 406,
	//GsmKeyProperty_PreferredKeyServer			= 407,
	//GsmKeyProperty_PreferredCompressionAlgorithms = 408,
	//GsmKeyProperty_PreferredHashAlgorithms		= 409,
	//GsmKeyProperty_PreferredEmailEncodings		= 410,
	//GsmKeyProperty_FingerprintV3				= 415,

#if 0 // not support first
	/* SubKey boolean properties */	
	kPGPSubKeyProperty_IsRevoked				= 501,
	kPGPSubKeyProperty_IsNotCorrupt				= 502,
	kPGPSubKeyProperty_IsExpired				= 503,
	kPGPSubKeyProperty_NeedsPassphrase			= 504,
	kPGPSubKeyProperty_HasUnverifiedRevocation	= 505,
	kPGPSubKeyProperty_IsRevocable				= 506,
	kPGPSubKeyProperty_HasThirdPartyRevocation	= 507,
	kPGPSubKeyProperty_IsOnToken				= 508,
	kPGPSubKeyProperty_IsX509Certificate		= 509,
	kPGPSubKeyProperty_IsSecret 				= 510,
	kPGPSubKeyProperty_IsOnCSP					= 511,

	/* SubKey numeric properties */
	kPGPSubKeyProperty_AlgorithmID 				= 600,
	kPGPSubKeyProperty_Bits						= 601,	
	kPGPSubKeyProperty_LockingAlgorithmID		= 602,
	kPGPSubKeyProperty_LockingBits				= 603,
	kPGPSubKeyProperty_Version					= 604,
	kPGPSubKeyProperty_Flags					= 605,

	/* SubKey time properties */	
	kPGPSubKeyProperty_Creation 				= 700,
	kPGPSubKeyProperty_Expiration				= 701,
	kPGPSubKeyProperty_RevocationTime			= 702,

	/* SubKey data (variable sized) properties */	
	kPGPSubKeyProperty_KeyData					= 800,
	kPGPSubKeyProperty_KeyID					= 801,
	kPGPSubKeyProperty_X509Certificate			= 802,
	kPGPSubKeyProperty_Fingerprint				= 803,
#endif

	/* User ID boolean properties */	
	GsmKeyUserIDProperty_IsAttribute			= 900,
	GsmKeyUserIDProperty_IsRevoked				= 901,

	/* User ID numeric properties */	
	GsmKeyUserIDProperty_Validity 				= 1000,
	GsmKeyUserIDProperty_Confidence				= 1001,
	GsmKeyUserIDProperty_AttributeType			= 1002,

	/* No User ID time properties */	

	/* User ID data (variable sized) properties */
	GsmKeyUserIDProperty_Name 					= 1200,
	GsmKeyUserIDProperty_AttributeData			= 1201,
	GsmKeyUserIDProperty_CommonName				= 1202,
	GsmKeyUserIDProperty_EmailAddress				= 1203,
	GsmKeyUserIDProperty_SMIMEPreferredAlgorithms	= 1204,

#if 0 // not support first
	/* Signature boolean properties */
	GsmSigProperty_IsRevoked 					= 1300,
	GsmSigProperty_IsNotCorrupt				= 1301,
	GsmSigProperty_IsTried						= 1302,
	GsmSigProperty_IsVerified					= 1303,
	GsmSigProperty_IsMySig						= 1304,
	GsmSigProperty_IsExportable				= 1305,
	GsmSigProperty_HasUnverifiedRevocation		= 1306,
	GsmSigProperty_IsExpired					= 1307,
	GsmSigProperty_IsX509						= 1308,

	/* Signature numeric properties */
	GsmSigProperty_AlgorithmID					= 1400,
	GsmSigProperty_TrustLevel					= 1401,
	GsmSigProperty_TrustValue					= 1402,
	GsmSigProperty_Flags						= 1403,
	GsmSigProperty_X509Flags					= 1404,
	GsmSigProperty_HashAlgorithmID				= 1405,

	/* Signature time properties */
	GsmSigProperty_Creation 					= 1500,
	GsmSigProperty_Expiration					= 1501,

	/* Signature data (variable sized) properties */
	GsmSigProperty_KeyID 						= 1600,
	GsmSigProperty_X509Certificate				= 1601,
	GsmSigProperty_X509IASN					= 1602,
	GsmSigProperty_X509LongName				= 1603,
	GsmSigProperty_X509IssuerLongName			= 1604,
	GsmSigProperty_X509DNSName					= 1605,
	GsmSigProperty_X509IPAddress				= 1606,
	GsmSigProperty_X509DERDName				= 1607,
	GsmSigProperty_RegularExpression			= 1608,
	GsmSigProperty_X509SN						= 1609,
#endif
} GsmKeyProperty;

typedef enum
{
	GSM_SIGN_MODE_NORMAL = 0,
	GSM_SIGN_MODE_DETACH,
	GSM_SIGN_MODE_CLEAR		// for text mode only
} GsmSignModeEnum;

typedef enum
{
	GsmKeyServerState_Invalid					= 0,
	GsmKeyServerState_Opening					= 1,
	GsmKeyServerState_Querying					= 2,
	GsmKeyServerState_ReceivingResults			= 3,
	GsmKeyServerState_ProcessingResults			= 4,
	GsmKeyServerState_Uploading					= 5,
	GsmKeyServerState_Deleting					= 6,
	GsmKeyServerState_Disabling					= 7,
	GsmKeyServerState_Closing					= 8,
	GsmKeyServerState_TLSUnableToSecureConnection = 9,
	GsmKeyServerState_TLSConnectionSecured		= 10
} GsmKeyServerState;

typedef enum 
{
	GsmSdaTargetPlatform_None				= 0,
	GsmSdaTargetPlatform_Win32				= 1,
	GsmSdaTargetPlatform_Linux				= 2,
	GsmSdaTargetPlatform_Solaris			= 3,
	GsmSdaTargetPlatform_AIX				= 4,
	GsmSdaTargetPlatform_HPUX				= 5,
	GsmSdaTargetPlatform_MacOSX				= 6,

} GsmSdaTargetPlatform;

typedef struct  
{
	TCHAR szKey[36];
} GsmKeyID;

typedef void * GsmKeyHandle;

typedef struct tagGsmKeyGenParam 
{
	LPTSTR m_pszUser;
	LPTSTR m_pszComment;
	LPTSTR m_szEmail;
	int m_iKeyGenType;
	int m_iKeyBits;
	unsigned int m_uiExpireDays;
	LPTSTR m_pszPassphrase;
	// TODO: add others
} GsmKeyGenParam;

typedef enum
{
	GSM_DECRYPT_RESULT_NO_DEC = 0,		/* does not decrypt */
	GSM_DECRYPT_RESULT_OK = 1,
	GSM_DECRYPT_RESULT_NO_SECKEY = 2,
	GSM_DECRYPT_RESULT_UNSUPPORT_ALGO = 3,
	GSM_DECRYPT_RESULT_OTHER = 4
} GsmDecryptResultType;

typedef enum
{
	GSM_VERIFY_RESULT_NO_VERIFY		= 0x0000,  /* does not verify */
	GSM_VERIFY_RESULT_SIGSUM_VALID	= 0x0001,  /* The signature is fully valid.  */
	GSM_VERIFY_RESULT_SIGSUM_GREEN	= 0x0002,  /* The signature is good.  */
	GSM_VERIFY_RESULT_SIGSUM_RED	= 0x0004,  /* The signature is bad.  */
	GSM_VERIFY_RESULT_KEY_REVOKED	= 0x0010,  /* One key has been revoked.  */
	GSM_VERIFY_RESULT_KEY_EXPIRED	= 0x0020,  /* One key has expired.  */
	GSM_VERIFY_RESULT_SIG_EXPIRED	= 0x0040,  /* The signature has expired.  */
	GSM_VERIFY_RESULT_KEY_MISSING	= 0x0080,  /* Can't verify: key missing.  */
	GSM_VERIFY_RESULT_CRL_MISSING	= 0x0100,  /* CRL not available.  */
	GSM_VERIFY_RESULT_CRL_TOO_OLD	= 0x0200,  /* Available CRL is too old.  */
	GSM_VERIFY_RESULT_BAD_POLICY	= 0x0400,  /* A policy was not met.  */
	GSM_VERIFY_RESULT_SYS_ERROR		= 0x0800   /* A system error occured.  */
} GsmVerifyResultType;


typedef struct tagGsmDecryptResult
{
	int	type;
	TCHAR szUnsupportedAlgo[32];
	unsigned bWrongKeyUsage;

	GsmKeyID noSecKeyId;		// the key ID which has no sec key

	/* The original file name of the plaintext message, if
	available.  */
	TCHAR szFileName[128];
} GsmDecryptResult;

typedef struct tagGsmVerifyResult
{
	int type;
	
	/* The key ID of the signature.  */
	GsmKeyID signKeyId;

	/* Signature creation time.  */
	unsigned long creationTs;

	/* Signature exipration time or 0.  */
	unsigned long expiresTs;

	/* Key should not have been used for signing.  */
	unsigned bWrongKeyUsage;

	/* The original file name of the plaintext message, if
	available.  */
	TCHAR szFileName[128];
} GsmVerifyResult;

typedef struct tagGsmDecryptVerifyResult
{
	GsmDecryptResult decryptResult;

	GsmVerifyResult verifyResult;
} GsmDecryptVerifyResult;

#endif // _