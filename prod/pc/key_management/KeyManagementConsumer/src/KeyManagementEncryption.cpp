#include <windows.h>
#include <wincrypt.h>
#include <time.h>
#include <cetype.h>
#include <cesdk.h>
#include "KeyManagementConsumer.h"
#include "KeyManagementConsumerPrivate.h"

#define ONE_INSTANCE_CRYPT_PROV

#if defined(ONE_INSTANCE_CRYPT_PROV)
static HCRYPTPROV g_hCryptProv = NULL;
#endif

static HCRYPTPROV createCryptProvider()
{
    HCRYPTPROV hCryptProv = NULL;

    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_NEWKEYSET))
    {
        DWORD err = GetLastError();
        if (err == NTE_EXISTS)
        {
            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, 0))
            {
                hCryptProv = NULL;
            }
        }
        else
        {
            hCryptProv = NULL;
        }

    }

    return hCryptProv;
}

HCRYPTPROV getCryptProvider()
{
#if defined(ONE_INSTANCE_CRYPT_PROV)
    if (g_hCryptProv == NULL)
    {
        g_hCryptProv = createCryptProvider();
    }
    return g_hCryptProv;
#else
    return createCryptProvider();
#endif
}

void releaseCryptProvider(HCRYPTPROV hCryptProv)
{
#if !defined(ONE_INSTANCE_CRYPT_PROV)
    CryptReleaseContext(hCryptProv, 0);
#endif
}

static HCRYPTKEY createCryptKey(HCRYPTPROV prov, const unsigned char * password, size_t passwordLen)
{
    struct INTPLAINTEXTKEYBLOB {
        BLOBHEADER bh;
        DWORD dwKeyLen;
    };
    
    BYTE *passwordData = new BYTE[sizeof(INTPLAINTEXTKEYBLOB) + passwordLen];
    memset(passwordData, 0, sizeof(INTPLAINTEXTKEYBLOB) + passwordLen);

    INTPLAINTEXTKEYBLOB * pKeyBlob = reinterpret_cast<INTPLAINTEXTKEYBLOB *> (passwordData);
    pKeyBlob->bh.bType = PLAINTEXTKEYBLOB;
    pKeyBlob->bh.bVersion = CUR_BLOB_VERSION;
    pKeyBlob->bh.aiKeyAlg = CALG_AES_128;
    pKeyBlob->dwKeyLen = static_cast<DWORD>(passwordLen);

    memcpy(passwordData + sizeof(INTPLAINTEXTKEYBLOB), password, passwordLen);

    HCRYPTKEY hKey = NULL;

    CryptImportKey(prov, (const BYTE *)passwordData,static_cast<DWORD> (sizeof(INTPLAINTEXTKEYBLOB) + passwordLen), NULL, 0, &hKey);

    DWORD dwMode = CRYPT_MODE_ECB;
    CryptSetKeyParam(hKey, KP_MODE, (BYTE *)&dwMode, 0);

    delete[] passwordData;

    return hKey;

}

static void destroyCryptKey(HCRYPTPROV prov, HCRYPTKEY hKey)
{
    CryptDestroyKey(hKey);
    releaseCryptProvider(prov);
}

void createEncryptionPayload(const unsigned char *password, size_t passwordLen, CEint32 **nonce, CEString *encryptedNonce)
{
#define NONCE_LEN 16
    unsigned char buf[NONCE_LEN * 2];
    HCRYPTPROV cryptProvider = getCryptProvider();
    HCRYPTKEY cryptKey = createCryptKey(cryptProvider, password, passwordLen);

    *nonce = new CEint32(static_cast<CEint32>(time(NULL)));

    _snprintf_s((char *)buf,NONCE_LEN * 2, _TRUNCATE, "%0*d", NONCE_LEN, **nonce);

    DWORD dataLen = NONCE_LEN;
    //DWORD *outbuf;
    //DWORD outbufSize;
    CryptEncrypt(cryptKey, NULL, TRUE, 0, buf, &dataLen, sizeof(buf));

    *encryptedNonce = hexify(buf, dataLen);

    // clean up key created by createCryptKey()
    destroyCryptKey(cryptProvider, cryptKey);
    return;
}

void decryptCEKeyBytes(const unsigned char *password, size_t passwordLen, CEString responseData, unsigned char *key, int size)
{
    HCRYPTPROV cryptProvider = getCryptProvider();
    HCRYPTKEY cryptKey = createCryptKey(cryptProvider, password, passwordLen);

    int binaryResponseSize = static_cast<int>(nlstrlen(CEM_GetString(responseData))/2);
    unsigned char *binaryResponse = new unsigned char[binaryResponseSize];

    unhexify(responseData, binaryResponse, binaryResponseSize);

    DWORD dSize = binaryResponseSize;

    // Decryption happens in place
    CryptDecrypt(cryptKey, NULL, TRUE, 0, binaryResponse, &dSize);

    memcpy(key, binaryResponse, dSize);

    // clean up key created by createCryptKey()
    destroyCryptKey(cryptProvider, cryptKey);
    delete[] binaryResponse;
    
    return;
}
