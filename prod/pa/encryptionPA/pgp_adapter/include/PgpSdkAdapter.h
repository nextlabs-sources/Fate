/*
Written by Derek Zheng
March 2008
*/

#ifndef _PGP_SDK_ADAPTER_H
#define _PGP_SDK_ADAPTER_H

#include "GsmSdk.h"
#include "PgpSdkFunc.h"

#define PGP_SDK_PUB_KEY_RING_NAME _T("pubring.pkr")
#define PGP_SDK_SEC_KEY_RING_NAME _T("secring.skr")

#define	kMaxServerNameLength			255
#define kMaxKeyStoreDNLength			255

typedef struct
{
	GsmProcessCallbackF cb;
	void *context;
} PgpProcessCallbackData;

typedef enum
{
	kPGPKeyServerType_PGPLDAP					= 100,
	kPGPKeyServerType_PGPLDAPS					= 101,
	kPGPKeyServerType_PGPVKD					= 102,  /* PGP Global Directory (PGP Universal Server 2.0) (over LDAP) */
	kPGPKeyServerType_X509LDAP					= 103,
	kPGPKeyServerType_X509LDAPS					= 104,
	kPGPKeyServerType_PGPHTTP					= 105,
	kPGPKeyServerType_PGPVKDS					= 106,	/* PGP Global Directory over LDAPS */
} PGPKeyServerType;

typedef struct PGPKeyServerEntry_
{
	PGPChar *				title;
	PGPChar				domain[kMaxServerNameLength + 1];
	PGPChar				serverDNS[kMaxServerNameLength + 1];
	PGPInt32				serverPort;		/* Used to be PGPUInt16; Promoted for ease of use in XML APIs */
	PGPKeyServerProtocol	protocol;
	PGPKeyServerClass		type;			/* This field should really be named 'class'. Left it as-is to avoid build breakage */
	PGPKeyServerType		keyserverType;
	PGPChar				keystoredn[kMaxKeyStoreDNLength + 1];
	PGPChar				authKeyIDString[kPGPMaxKeyIDStringSize];
	PGPChar				clientAuthKeyIDString[kPGPMaxKeyIDStringSize];
	PGPByte					*clientAuthCertIASN;
	PGPSize					clientAuthCertIASNSize;
	PGPPublicKeyAlgorithm	authAlg;
	PGPInt32				flags;
	PGPBoolean				trusted;
	PGPBoolean				isConfigServer;
	PGPInt32				identifier;
} PGPKeyServerEntry;

PGPError PgpSdkEncodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
PGPError PgpSdkDecodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
PGPError PgpSdkVerifyEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
PGPError PgpSdkKeyServerEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);

class CPgpSdkAdapter : public CGsmSdk
{
public:
	CPgpSdkAdapter() 
	{ 
		m_Inited = 0; 
		m_context = kInvalidPGPContextRef; 
		m_keyDB	= kInvalidPGPKeyDBRef;

		memset(m_szVersion, 0, 256*sizeof(TCHAR)); 
		m_pszPubKeyRingPath = NULL;
		m_pszSecKeyRingPath = NULL;

		m_ulTTL = 0;
	}

	~CPgpSdkAdapter() {}

	virtual GsmErrorT Init( LPCTSTR keyRingPath = NULL );
	virtual void Release( void );

	virtual void SetPassCacheTTL(unsigned long ttl);

	// Key Management
	virtual GsmErrorT GenerateKey(GsmKeyGenParam &param, GsmProcessCallbackF processCb, void *context, GsmKeyID *keyId);
	virtual GsmErrorT DeleteKey(GsmKeyHandle handle);	// delete the key from keyring
	virtual GsmErrorT DeleteKey(GsmKeyID keyId);	// delete the key from keyring
	virtual GsmErrorT RevokeKey(GsmKeyID keyId, LPCTSTR passphrase);	// let the key revoked

	// All key handle should be released by ReleaseKey
	virtual void ReleaseKey(GsmKeyHandle handle);
	virtual void ReleaseKeyArray(GsmKeyHandle *hKeys, int nKeys);

	virtual GsmErrorT EnumAllKeys(GsmKeyHandle **hKeys, int *nKey, int keyType);

	virtual GsmErrorT FindKeyByKeyID(GsmKeyID keyId, GsmKeyHandle *handle);
	virtual GsmErrorT FindKeyByKeyFpr(LPCTSTR fpr, GsmKeyHandle *handle);
	virtual GsmErrorT FindKeyByUserID(LPCTSTR pszUserId, GsmKeyHandle **hKeys, int *nKey, int keyType, int bSubString);

	virtual GsmErrorT GetKeyID(GsmKeyHandle handle, GsmKeyID *keyId);
	virtual GsmErrorT GetKeyDataProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, void *buffer, size_t bufferSize, size_t *dataSize);
	virtual GsmErrorT GetKeyNumericProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, int *result);
	virtual GsmErrorT GetKeyTimeProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, time_t *result);

	virtual GsmErrorT ImportKey(LPCTSTR pszFilePath, GsmKeyHandle *hKey);
	virtual GsmErrorT ExportKey(GsmKeyHandle hKey, LPCTSTR pszFilePath);

	// Sub Key Iterate not supplied first

	// Key Server management not supplied first
	virtual GsmErrorT SearchKeyFromKeyServerByKeyId(LPCTSTR pszServer, GsmKeyID keyId, GsmKeyHandle *hKey,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL);
	virtual GsmErrorT SearchKeyFromKeyServerByEmail(LPCTSTR pszServer, LPCTSTR pszEmail, GsmKeyHandle **hKeys, int *nKeys,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL);
	virtual GsmErrorT PublishKeyToKeyServer(LPCTSTR pszServer, GsmKeyID keyId,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL);

	virtual void FreeBufferGetFromGsm(char *buff);
	// Encryption
	// /virtual GsmErrorT SymmEncryptBuffer(const char *plainBuf, char **cipherBuf, const char *passphrase);
	virtual GsmErrorT SymmEncryptBuffer(const char *plainBuf, size_t plainBufSize, 
									char **cipherBuf, size_t *cipherBufSize, LPCTSTR passphrase);
	virtual GsmErrorT SymmEncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, LPCTSTR passphrase);
	virtual GsmErrorT SymmEncryptFile(LPCTSTR filePath, LPCTSTR passphrase);

	/* cb and context is not usable here, please use preSignPass to pass passphrase */
	virtual GsmErrorT EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
			char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle encKey, 
			GsmKeyHandle signKey, LPCTSTR preSignPass, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL);
	virtual GsmErrorT EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
			char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL);

	/* cb and context is not usable here, please use preSignPass to pass passphrase */
	virtual GsmErrorT EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, GsmKeyHandle encKey, 
			GsmKeyHandle signKey, LPCTSTR preSignPass, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL);

	virtual GsmErrorT EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, 
			GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL);

	virtual GsmErrorT EncryptFile2(LPCTSTR filePath, 
			GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL);

	//virtual int IsEncryptedBuffer(const char *buffer, size_t len);
	virtual BOOL IsEncryptedFile(LPCTSTR filePath);

	// Decryption
	/* cb and context is usable here */
	virtual GsmErrorT DecryptVerifyBuffer(const char *cipherBuf, size_t cipherBufSize, 
		char **plainBuf, size_t *plainBufSize, LPCTSTR preDecryptPass, 
		GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result);
	virtual GsmErrorT DecryptVerifyFile(LPCTSTR cipherFile, LPCTSTR plainFile, LPCTSTR preDecryptPass, 
		GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result);
	virtual GsmErrorT DecryptVerifyFile(LPCTSTR filePath, LPCTSTR preDecryptPass,
		GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result);

	// Signature
	/* cb and context is not usable here, please use preSignPass to pass passphrase */
	virtual GsmErrorT SignBuffer(const char *plainBuf, size_t plainBufSize, 
		char **signBuf, size_t *signBufSize, GsmKeyHandle signKey, 
		LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode);
	virtual GsmErrorT SignFile(LPCTSTR plainFile, LPCTSTR signFile, GsmKeyHandle signKey, 
		LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode);

	// Verification
	virtual GsmErrorT VerifyBuffer(const char *signBuf, size_t signBufSize, 
		char **plainBuf, size_t *plainBufSize, GsmVerifyResult &result);
	virtual GsmErrorT VerifyFile(LPCTSTR signFile, LPCTSTR plainFile, GsmVerifyResult &result);
	virtual GsmErrorT VerifyBufferDetachedSig(const char *dataBuf, size_t dataBufSize, 
		const char *sigBuf, size_t sigBufSize, GsmVerifyResult &result);
	virtual GsmErrorT VerifyFileDetachedSig(LPCTSTR dataFile, LPCTSTR sigFile, GsmVerifyResult &result);

#if 0
	virtual GsmErrorT CreateSDAFile(const char **srcPaths, int nSrc, 
		const char *destPath, const char *passwd, GsmSdaTargetPlatform platform);
	virtual int IsSDAFile(const char *filePath);
	virtual GsmErrorT DecryptSDAFile(const char *filePath, const char *destDirPath, const char *passwd);
#endif

	/* The follow 2 functions will prompt dialog to get credential */
#if 0
	virtual GsmErrorT GetRecipients(HWND hParentWnd, LPVOID *hRecipients) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual void ReleaseRecipients(LPVOID hRecipients) {}
	virtual GsmErrorT GetPassword(HWND hParentWnd, LPTSTR *lpszPassword) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual void ReleasePassword(LPTSTR lpszPassword);
	virtual GsmErrorT EncryptFile(LPCTSTR lpszPlainFile, LPCTSTR lpszCipherFile, BOOL bSymm, 
		LPVOID hRecipients, LPTSTR lpszPassword);
	virtual GsmErrorT DecryptFile(HWND hParentWnd, LPCTSTR lpszCipherFile, LPCTSTR lpszPlainFile);
#endif

protected:
	PGPError ConsoleAcquireEntropy(
		PGPContextRef 			context,
		PGPUInt32				entropyNeeded,
		PGPUInt32 *				pEntropyAcquired,
		PGPBoolean				bOutputProgress);
	PGPError GetActualRecipentKey(
		PGPKeyDBObjRef		key, 
		PGPKeyID	const	*keyIDArray,
		PGPUInt32			keyCount,
		PGPKeyDBObjRef		*outRecipientKey);

	PGPError ParseKeyserverString(
		LPCTSTR					keyserver,
		PGPKeyServerEntry **	outKeyserver);

	GsmErrorT ProcessKeyServerOperation(LPCTSTR pszServer, int operation, 
		GsmKeyID *keyId, LPCTSTR pszEmail, 
		GsmKeyHandle **hKeys, int *nKeys,
		GsmKeyServerStateChangedCallbackF cb, void *context);

	friend PGPError PgpSdkEncodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
	friend PGPError PgpSdkDecodeEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
	friend PGPError PgpSdkVerifyEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);
	friend PGPError PgpSdkKeyServerEventHandler(PGPContextRef context, PGPEvent *event, PGPUserValue userValue);


private:
	CPgpSdkFunc m_PgpSdkFunc;
	int m_Inited;	// set true after Init() called

	PGPContextRef m_context;
	PGPKeyDBRef	m_keyDB;

	TCHAR m_szVersion[256];
	LPTSTR m_pszPubKeyRingPath;
	LPTSTR m_pszSecKeyRingPath;

	unsigned long m_ulTTL;
};

typedef struct
{
	GsmPassphraseCallbackF cb;
	void *context;
	CPgpSdkAdapter *pgpSdk;
	void *recipientsData;
	int triedTimes;
	int bStopTry;	// cancel enter passphrase
	GsmDecryptVerifyResult *result;
} PgpPassphraseCallbackData;

typedef struct
{
	CPgpSdkAdapter *pgpSdk;
	GsmVerifyResult *result;
} PgpVerifyCallbackData;

#endif // _PGP_SDK_ADAPTER_H