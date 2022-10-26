// KeyMnaagementConsumer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

// NextLabs SDK
#include <cetype.h>
#include <cesdk.h>
#include <ceservice.h>
#include "KeyManagementConsumer.h"
#include "KeyManagementConsumerPrivate.h"

#define KEYMGMT L"NL_KM_CLIENT"

// Possible handy function for cestring.cpp?
static CEString CEStringDup(CEString s)
{
    return CEM_AllocateString(CEM_GetString(s));
}

static wchar_t tohex(int i)
{
    static const wchar_t hexdigits[] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'a', L'b', L'c', L'd', L'e', L'f' };
    
    return hexdigits[i];
}

static int fromhex(wchar_t c)
{
    if (c >= L'0' && c <= L'9')
    {
        return (c-L'0');
    }
    else
    {
        return (c-L'a'+10);
    }
}

CEString hexify(unsigned char *buf, int len)
{
    wchar_t *hexified = new wchar_t[(2 * len) + 1];
    for (int i = 0 ; i < len; i++)
    {
        hexified[2*i] = tohex(buf[i] / 16);
        hexified[2*i+1] = tohex(buf[i] % 16);
    }

    hexified[2 * len] = L'\0';

    CEString str = CEM_AllocateString(hexified);
    delete[] hexified;
    return str;
}

CEString hexifyHash(CEKeyID *keyId)
{
    return hexify(keyId->hash, KM_HASH_LEN);
}

bool unhexify(CEString hexed, unsigned char *buf, int size)
{
    const nlchar *hex = CEM_GetString(hexed);
    size_t hexlen = nlstrlen(hex);

    // Is the destination big enough?
    if (hexlen % 2 == 1 || hexlen/2 > static_cast<size_t>(size))
    {
        return false;
    }

    memset(buf, '\0', size);

    for (size_t i = 0; i < hexlen; i+=2)
    {
        buf[i/2] = static_cast<unsigned char> (fromhex(hex[i]) * 16 + fromhex(hex[i+1]));
    }

    return true;
}


// Yup.  Global.  Not thread safe.  Deal with it.
static CEString exception = NULL;

static void setExceptionString(CEString ex)
{
    if (exception != NULL)
    {
        CEM_FreeString(exception);
    }
    exception = CEStringDup(ex);
}

CEString CEKey_GetLastException()
{
    return exception;
}

CEResult_t CEKey_CreateKeyRing(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 processID, CEint32 timeout)
{
    if (EMPTY_CESTRING(keyRingName))
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;

    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void*[5];
    request[0] = (void *)CEM_AllocateString(L"CREATERING");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sissi");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // Cleanup
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    delete (CEint32 *)request[4];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }
    return result;
}

CEResult_t CEKey_DeleteKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKeyID id, CEint32 processID, CEint32 timeout) 
{
    if (EMPTY_CESTRING(keyRingName))
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void *[7];
    request[0] = (void *)CEM_AllocateString(L"DELETEKEY");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)hexifyHash(&id);
    request[5] = (void *)new CEint32(id.timestamp);
    request[6] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sisssii");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    CEM_FreeString((CEString)request[4]);
    delete (CEint32 *)request[5];
    delete (CEint32 *)request[6];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }
    return result;
}

CEResult_t CEKey_DeleteKeyRing(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 processID, CEint32 timeout)
{
    if (EMPTY_CESTRING(keyRingName))
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void *[5];
    request[0] = (void *)CEM_AllocateString(L"DELETERING");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sissi");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    delete (CEint32 *)request[4];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }
    return result;
}

CEResult_t CEKey_GenerateKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 keyLen, CEKeyID *keyID, CEint32 processID, CEint32 timeout)
{
    if (EMPTY_CESTRING(keyRingName) || keyID == NULL || keyLen < 16 || keyLen > 32)
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void *[6];
    request[0] = (void *)CEM_AllocateString(L"GENERATEKEY");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)new CEint32(keyLen);
    request[5] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sissii");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    delete (CEint32 *)request[4];
    delete (CEint32 *)request[5];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }
        else
        {
            unhexify((CEString)response[2], keyID->hash, KM_HASH_LEN);
            keyID->timestamp = *(CEint32 *)response[3];
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }
    return result;
}

CEResult_t CEKey_GetKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKeyID id, CEKey *key, CEint32 processID, CEint32 timeout)
{
    if (EMPTY_CESTRING(keyRingName) || key == NULL)
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void *[7];
    request[0] = (void *)CEM_AllocateString(L"GETKEY");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)hexifyHash(&id);
    request[5] = (void *)new CEint32(id.timestamp);
    request[6] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sisssii");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    CEM_FreeString((CEString)request[4]);
    delete (CEint32 *)request[5];
    delete (CEint32 *)request[6];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }
        else
        {
            key->struct_version = *(CEint32 *)response[2];
            unhexify((CEString)response[3], key->id.hash, KM_HASH_LEN);
            key->id.timestamp = *(CEint32 *)response[4];

            key->keylen = *(CEint32 *)response[6];

            if (key->keylen > KM_KEY_LEN) 
            {
                // Returned key is too large.  We don't really have an appropriate
                // error message
                result = CE_RESULT_VERSION_MISMATCH;
            }
            else
            {
                decryptCEKeyBytes(password, passwordLen, (CEString)response[5], key->key, KM_KEY_LEN);
            }
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }
    return result;
}

CEResult_t CEKey_ListKeyRings(CEHandle handle, const unsigned char *password, size_t passwordLen, CEKeyRing *keyRings, CEint32 *size, CEint32 processID, CEint32 timeout)
{
    if (size == NULL || (*size != 0 && keyRings == NULL))
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new (std::nothrow)void *[4];
    if(request == NULL)
	return CE_RESULT_GENERAL_FAILED;
    request[0] = (void *)CEM_AllocateString(L"GETALLRINGS");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)new(std::nothrow) CEint32(processID);


    CEString fmt = CEM_AllocateString(L"sisi");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    delete (CEint32 *)request[3];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }
        else
        {
            CEint32 origSize = *size;

            *size = *(CEint32 *)response[2];

            if (origSize < *size)
            {
                // Array is not big enough
                return CE_RESULT_INSUFFICIENT_BUFFER;
            }

            for (int i = 0; i < *size; i++)
            {
                keyRings[i].name = CEStringDup((CEString)response[i+3]);
                keyRings[i].size = 0;
                keyRings[i].keyIDs = NULL;
            }
        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }

    return result;
}

CEResult_t CEKey_ListKeys(CEHandle handle, const unsigned char *password, size_t passwordLen, CEKeyRing *keyRing, CEint32 processID, CEint32 timeout)
{
    if (keyRing == NULL)
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new(std::nothrow) void *[5];
    if(request == NULL)
		return CE_RESULT_GENERAL_FAILED;
    request[0] = (void *)CEM_AllocateString(L"GETALLKEYS");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRing->name);
    request[4] = (void *)new(std::nothrow) CEint32(processID);
    CEString fmt = CEM_AllocateString(L"sissi");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    delete (CEint32 *)request[4];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }
        else
        {
            CEint32 origSize = keyRing->size;

            keyRing->size = *(CEint32 *)response[2];

            if (origSize < keyRing->size)
            {
                // Array is not big enough
                return CE_RESULT_INSUFFICIENT_BUFFER;
            }

            for (int i = 0; i < keyRing->size; i++)
            {
                unhexify((CEString)response[i*2+3], keyRing->keyIDs[i].hash, KM_HASH_LEN);
                keyRing->keyIDs[i].timestamp = *(CEint32 *)response[i*2+4];
            }

        }

        ServiceResponseFree(response);
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }

    return result;
}

CEResult_t CEKey_SetKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKey key, CEint32 processID, CEint32 timeout)
{
    if (EMPTY_CESTRING(keyRingName))
    {
        return CE_RESULT_INVALID_PARAMS;
    }

    CEString command = CEM_AllocateString(KEYMGMT);

    CEint32 *nonce;
    CEString encryptedNonce;
    createEncryptionPayload(password, passwordLen, &nonce, &encryptedNonce);

    void **request = new void *[10];
    request[0] = (void *)CEM_AllocateString(L"SETKEY");
    request[1] = nonce;
    request[2] = encryptedNonce;
    request[3] = (void *)CEStringDup(keyRingName);
    request[4] = (void *)new CEint32(key.struct_version);
    request[5] = (void *)hexifyHash(&key.id);
    request[6] = (void *)new CEint32(key.id.timestamp);
    request[7] = (void *)hexify(key.key, KM_KEY_LEN);
    request[8] = (void *)new CEint32(key.keylen);
    request[9] = (void *)new CEint32(processID);

    CEString fmt = CEM_AllocateString(L"sissisisii");

    void **response;

    CEResult_t result = ServiceInvoke(handle, command, fmt, request, &response, timeout);

    // clean up
    CEM_FreeString((CEString)request[0]);
    delete (CEint32 *)request[1];
    CEM_FreeString((CEString)request[2]);
    CEM_FreeString((CEString)request[3]);
    delete (CEint32 *)request[4];
    CEM_FreeString((CEString)request[5]);
    delete (CEint32 *)request[6];
    CEM_FreeString((CEString)request[7]);
    delete (CEint32 *)request[8];
    delete (CEint32 *)request[9];
    delete[] request;

    CEM_FreeString(command);
    CEM_FreeString(fmt);

    if (result == CE_RESULT_SUCCESS)
    {
        result = (CEResult_t)*(CEint32 *)response[1];

        if (result != CE_RESULT_SUCCESS) 
        {
            setExceptionString((CEString)response[2]);
        }
    }
    else
    {
        setExceptionString(CEM_AllocateString(L"Unknown ServiceInvoke error"));
    }

    return result;
}



