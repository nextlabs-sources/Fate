/*
 * Created on Mar 10, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

#ifndef KEYMANAGEMENTCONSUMER_H
#define KEYMANAGEMENTCONSUMER_H

#ifdef KEYMANAGEMENTDLL
#define KEYMANAGEMENTEXPORT __declspec(dllexport)
#else
#define KEYMANAGEMENTEXPORT __declspec(dllimport)
#endif

#include <cesdk.h>

#define KM_HASH_LEN 32
#define KM_KEY_LEN 32
#define KM_MAX_KEYSTORE_NAME 16

#define DEFAULT_KM_TIMEOUT (5*1000)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned char hash[KM_HASH_LEN];
    CEint32 timestamp;
} CEKeyID;

typedef struct {
    CEint32 struct_version;
    CEKeyID id;
    unsigned char key[KM_KEY_LEN];
    CEint32 keylen;
} CEKey;

typedef struct {
    CEString name;
    CEint32 size;
    CEKeyID *keyIDs;
} CEKeyRing;

/*! CEKEY_GetLastException
 * 
 * \brief Return the last exception string set by a CEKEY command.
 *
 * When CEKey function return an error it usually sets an exception string explaining the
 * details.  This function lets you read that string.  Note that this code is not even
 * remotely thread safe.
 */
KEYMANAGEMENTEXPORT
CEString CEKey_GetLastException();

/*! CEKEY_GetKey
 *
 * \brief Reads a key, specified by key ring name and ID, from the key store.
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of key ring containing key (e.g. NL_SE_LOCAL)
 * \param id (in) key id.    request latest key by making all fields 0 in the id
 * \param key (out)          returned key
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_GetKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKeyID id, CEKey *key, CEint32 processID, CEint32 timeout);

/*! CEKEY_CreateKeyRing
 *
 * \brief Creates a new key ring
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of new key ring
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_CreateKeyRing(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 processID, CEint32 timeout);

/*! CEKEY_SetKey
 *
 * \brief Creates a new key in the specified key ring and sets its value
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of new key ring
 * \param key (in)           new key
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_SetKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKey key, CEint32 processID, CEint32 timeout);

/*! CEKEY_GenerateKey
 *
 * \brief Creates a new key in the specified key ring with a randomly
 *        generated value
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of new key ring
 * \param keyLen (in)        length of new key
 * \param key (out)          id of new key
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_GenerateKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 keyLen, CEKeyID *keyID, CEint32 processID, CEint32 timeout);

/*! CEKEY_DeleteKey
 *
 * \brief Deletes a key by id and key ring name
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of key ring
 * \param id (in)            key id
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_DeleteKey(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEKeyID id, CEint32 processID, CEint32 timeout); 

/*! CEKEY_DeleteKeyRing
 *
 * \brief Deletes a key ring by name
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRingName (in)   name of key ring
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_DeleteKeyRing(CEHandle handle, const unsigned char *password, size_t passwordLen, CEString keyRingName, CEint32 processID, CEint32 timeout);

/*! CEKEY_ListKeyRings
 *
 * \brief List all key rings
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRings (out)     array of size size
 * \param size (in/out)      size of keyRings array/number of keyRings
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 *
 * If keyRings is NULL then size will be set to the number of key rings.  The
 * next call should be made with a buffer of size * sizeof(CEKeyRing).
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_ListKeyRings(CEHandle handle, const unsigned char *password, size_t passwordLen, CEKeyRing *keyRings, CEint32 *size, CEint32 processID, CEint32 timeout);

/*! CEKEY_ListKeys
 *
 * \brief Obtain a list of keys in a key ring
 *
 * \param handle (in)        connection handle from CONN_Initialize call
 * \param password (in)      password used to identify ourselves to KM Service
 * \param passwordLen (in)   size of password in bytes
 * \param keyRing (in/out)   key ring
 * \param processID (in)     id of calling process
 * \param timeout (in)       sdk timeout
 *
 * Before calling this function the caller must allocate the buffer in
 * keyRing->keys according to keyRing->size.  It is not possible to determine
 * the size of a key ring directly.  Note that you can not specify the key ring
 * by name.  This implies that the CEKeyRing used here comes from CE_KeyListKeyRings
 */
KEYMANAGEMENTEXPORT
CEResult_t CEKey_ListKeys(CEHandle handle, const unsigned char *password, size_t passwordLen, CEKeyRing *keyRing, CEint32 processID, CEint32 timeout);

#ifdef __cplusplus
}
#endif

#endif

