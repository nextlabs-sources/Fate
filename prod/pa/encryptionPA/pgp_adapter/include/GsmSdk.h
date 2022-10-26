/*
Written by Derek Zheng
March 2008
*/

#ifndef _GSM_SDK_H_
#define _GSM_SDK_H_

#include "GsmType.h"
#include "GsmError.h"

typedef void (*GsmProcessCallbackF)( void *opaque, const char *what, int type);

/* GsmPassphraseCallbackF is implemented by user to get passphrase from UI when sign or decrypt data.
 * opaque -- User defined data
 * hint -- UserID of the key if not NLL
 * pass_info -- may one of three:
 *				 1. Standard passphrase requested:
				 "<keyid>"
				 2. Passphrase for symmetric key requested:
				 "<cipher_algo> <s2k_mode> <s2k_hash>"
				 3. PIN for a card requested.	-- not supported first
				 "<card_type> <chvno>"
 * prev_was_bad -- last passphrase was bad
 * passphrase -- the passphrase got from user(must be malloced in the callback if supplied)
 * return: ==0 -- contiue to try passphrase; < 0 cancel ; > 0 try passphrase once again
 */
typedef int (*GsmPassphraseCallbackF)( void *opaque, LPCTSTR hint, 
									   LPCTSTR pass_inf, int prev_was_bad, LPTSTR *passphrase);

/* GsmKeyServerStateChangedCallbackF may be implemented when communicating with keyserver.
 * It return 0 normally, otherwise it means user abort the operation.
 */
typedef int (*GsmKeyServerStateChangedCallbackF)(void *context, GsmKeyServerState state);

//typedef int (WINAPI *GsmZipPasswordCallbackF)(char *passwd, int size, const char *prompt, const char *name);

class CGsmSdk
{
public:
	CGsmSdk() {}
	virtual ~CGsmSdk() {}

	virtual GsmErrorT Init( LPCTSTR keyRingPath = NULL ) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual void Release( void ) { }

	virtual void SetPassCacheTTL(unsigned long ttl) { }

	// Key Management
	virtual GsmErrorT GenerateKey(GsmKeyGenParam &param, GsmProcessCallbackF processCb, void *context, GsmKeyID *keyId) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT DeleteKey(GsmKeyHandle handle) { return GSM_ERR_NOT_IMPLEMENTED; }	// delete the key from keyring
	virtual GsmErrorT DeleteKey(GsmKeyID keyId) { return GSM_ERR_NOT_IMPLEMENTED; }		// delete the key from keyring
	virtual GsmErrorT RevokeKey(GsmKeyID keyId, const char *passphrase) { return GSM_ERR_NOT_IMPLEMENTED; }	// let the key revoked

	// All key handle should be released by ReleaseKey
	virtual void ReleaseKey(GsmKeyHandle handle) { }
	// All key handle array should be released by ReleaseKeyArray
	virtual void ReleaseKeyArray(GsmKeyHandle *hKeys, int nKeys) { }

	virtual GsmErrorT EnumAllKeys(GsmKeyHandle **hKeys, int *nKey, int keyType) { return GSM_ERR_NOT_IMPLEMENTED; }

	virtual GsmErrorT FindKeyByKeyID(GsmKeyID keyId, GsmKeyHandle *handle) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT FindKeyByKeyFpr(LPCTSTR fpr, GsmKeyHandle *handle) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT FindKeyByUserID(LPCTSTR pszUserId, GsmKeyHandle **hKeys, int *nKey, int keyType, int bSubString) { return GSM_ERR_NOT_IMPLEMENTED; }

	// if buffer=NULL, GetKeyDataProperty will first return the actual dataSize
	virtual GsmErrorT GetKeyID(GsmKeyHandle handle, GsmKeyID *keyId) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT GetKeyDataProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, void *buffer, size_t bufferSize, size_t *dataSize) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT GetKeyNumericProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, int *result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT GetKeyTimeProperty(GsmKeyHandle handle, GsmKeyProperty whichProp, time_t *result) { return GSM_ERR_NOT_IMPLEMENTED; }

	virtual GsmErrorT ImportKey(LPCTSTR pszFilePath, GsmKeyHandle *hKey) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT ExportKey(GsmKeyHandle hKey, LPCTSTR pszFilePath) { return GSM_ERR_NOT_IMPLEMENTED; }

	// Sub Key Iterate not supplied first

	// Key Server management not supplied first
	virtual GsmErrorT SearchKeyFromKeyServerByKeyId(LPCTSTR pszServer, GsmKeyID keyId, GsmKeyHandle *hKey,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT SearchKeyFromKeyServerByEmail(LPCTSTR pszServer, LPCTSTR pszEmail, GsmKeyHandle **hKeys, int *nKeys,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT PublishKeyToKeyServer(LPCTSTR pszServer, GsmKeyID keyId,
		GsmKeyServerStateChangedCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }

	// buffer allocated by GSM must freed by FreeBufferGetFromGsm
	virtual void FreeBufferGetFromGsm(char  *buff) { }
	// Encryption
	//virtual GsmErrorT SymmEncryptBuffer(const char *plainBuf, char **cipherBuf, const char *passphrase) = 0;
	virtual GsmErrorT SymmEncryptBuffer(const char *plainBuf, size_t plainBufSize, 
									char **cipherBuf, size_t *cipherBufSize, LPCTSTR passphrase) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT SymmEncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, LPCTSTR passphrase) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT SymmEncryptFile(LPCTSTR filePath, LPCTSTR passphrase) { return GSM_ERR_NOT_IMPLEMENTED; }

	virtual GsmErrorT EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
			char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle encKey, 
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT EncryptBuffer(const char *plainBuf, size_t plainBufSize, 
			char **cipherBuf, size_t *cipherBufSize, GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }

	virtual GsmErrorT EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, GsmKeyHandle encKey, 
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT EncryptFile(LPCTSTR plainFile, LPCTSTR cipherFile, 
			GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT EncryptFile2(LPCTSTR filePath, 
			GsmKeyHandle *encKeys, int nKeys,
			GsmKeyHandle signKey = NULL, LPCTSTR preSignPass = NULL, 
			GsmPassphraseCallbackF cb = NULL, void *context = NULL) { return GSM_ERR_NOT_IMPLEMENTED; }

	//virtual int IsEncryptedBuffer(const char *buffer, size_t len) = 0;
	virtual BOOL IsEncryptedFile(LPCTSTR filePath) { return FALSE; }

	// Decryption
	virtual GsmErrorT DecryptVerifyBuffer(const char *cipherBuf, size_t cipherBufSize, 
									char **plainBuf, size_t *plainBufSize, LPCTSTR preDecryptPass, 
									GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT DecryptVerifyFile(LPCTSTR cipherFile, LPCTSTR plainFile, LPCTSTR preDecryptPass,
									GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT DecryptVerifyFile(LPCTSTR filePath, LPCTSTR preDecryptPass,
									GsmPassphraseCallbackF cb, void *context, GsmDecryptVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }

	// Signature
	virtual GsmErrorT SignBuffer(const char *plainBuf, size_t plainBufSize, 
								char **signBuf, size_t *signBufSize, GsmKeyHandle signKey, 
								LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT SignFile(LPCTSTR plainFile, LPCTSTR signFile, GsmKeyHandle signKey, 
								LPCTSTR preSignPass, GsmPassphraseCallbackF cb, void *context, int mode) { return GSM_ERR_NOT_IMPLEMENTED; }
	// Verification
	virtual GsmErrorT VerifyBuffer(const char *signBuf, size_t signBufSize, 
		char **plainBuf, size_t *plainBufSize, GsmVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT VerifyFile(LPCTSTR signFile, LPCTSTR plainFile, GsmVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT VerifyBufferDetachedSig(const char *dataBuf, size_t dataBufSize, 
		const char *sigBuf, size_t sigBufSize, GsmVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT VerifyFileDetachedSig(LPCTSTR dataFile, LPCTSTR sigFile, GsmVerifyResult &result) { return GSM_ERR_NOT_IMPLEMENTED; }

	virtual GsmErrorT CreateSDAFile(LPCTSTR *srcPaths, int nSrc, LPCTSTR destPath, LPCTSTR passwd, GsmSdaTargetPlatform platform) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual int IsSDAFile(LPCTSTR filePath) { return 0; }
	virtual GsmErrorT DecryptSDAFile(LPCTSTR filePath, LPCTSTR destDirPath, LPCTSTR passwd) { return GSM_ERR_NOT_IMPLEMENTED; }

#if 0
	//static void SetZipPasswdCb(GsmZipPasswordCallbackF passwdCb, void *ctx) { m_ZipPasswdCb = passwdCb; m_ZipPasswdCtx = ctx; }
	virtual GsmErrorT ZipFile(const char **srcFiles, int nSrc, const char *archive, const char *baseDir, int encrypt, GsmZipPasswordCallbackF zipPasswdCb);
	virtual GsmErrorT UnZipFile(const char *archive, const char *destDirPath, GsmZipPasswordCallbackF unzipPasswdCb);
#endif

	/* The follow 2 functions will prompt dialog to get credential */
	virtual GsmErrorT GetRecipients(HWND hParentWnd, LPVOID *hRecipients) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual void ReleaseRecipients(LPVOID hRecipients) {}
	virtual GsmErrorT GetPassword(HWND hParentWnd, LPTSTR *lpszPassword) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual void ReleasePassword(LPTSTR lpszPassword){}
	virtual GsmErrorT EncryptFile(LPCTSTR lpszPlainFile, LPCTSTR lpszCipherFile, BOOL bSymm, 
		LPVOID hRecipients, LPTSTR lpszPassword) { return GSM_ERR_NOT_IMPLEMENTED; }
	virtual GsmErrorT DecryptFile(HWND hParentWnd, LPCTSTR lpszCipherFile, LPCTSTR lpszPlainFile) { return GSM_ERR_NOT_IMPLEMENTED; }

protected:

public:
	//static GsmZipPasswordCallbackF m_ZipPasswdCb;
	//static void *m_ZipPasswdCtx;
	
};

#if 0
class GSMSDK_API CGsmSdkCreator
{
public:
	static CGsmSdk * CreateSdkInstance(GsmSdkTypeEnum sdkType);
	static void DestroySdkInstance(GsmSdkTypeEnum sdkType);
protected:
private:
	static CGsmSdk *m_pPgpSdk;	// Singleton
	static CGsmSdk *m_pGpgSdk;	// Singleton
};
#endif

#if 0
GSMSDK_API CGsmSdk * GsmCreateSdkInstance(GsmSdkTypeEnum sdkType, LPCTSTR lpszKeyStorePath);
GSMSDK_API void GsmDestroySdkInstance(CGsmSdk *pSdk);
#endif

#endif	// _GSM_SDK_H_