/*
 * Created on Mar 12, 2010
 *
 * All sources, binaries and HTML pages (C) copyright 2010 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */

#ifndef KEYMANAGEMENTCONSUMERPRIVATE_H
#define KEYMANAGEMENTCONSUMERPRIVATE_H

#include <wincrypt.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Some handy utility functions for mangling and unmangling keys
 */
KEYMANAGEMENTEXPORT
CEString hexify(unsigned char *buf, int len);

KEYMANAGEMENTEXPORT
CEString hexifyHash(CEKeyID *keyId);

KEYMANAGEMENTEXPORT
bool unhexify(CEString hexed, unsigned char *buf, int size);

HCRYPTPROV getCryptProvider();

void releaseCryptProvider(HCRYPTPROV hCryptProv);

void createEncryptionPayload(const unsigned char * password, size_t passwordLen, CEint32 **nonce, CEString *encryptedNonce);

void decryptCEKeyBytes(const unsigned char * password, size_t passwordLen, CEString responseData, unsigned char *key, int size);

#ifdef __cplusplus
}
#endif


#endif
