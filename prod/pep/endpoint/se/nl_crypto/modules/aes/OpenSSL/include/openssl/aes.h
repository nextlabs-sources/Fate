/* crypto/aes/aes.h -*- mode:C; c-file-style: "eay" -*- */
/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 */

#ifndef HEADER_AES_H
#define HEADER_AES_H

#include <openssl/opensslconf.h>

#ifdef OPENSSL_NO_AES
#error AES is disabled.
#endif

#include <stddef.h>

#define AES_ENCRYPT	1
#define AES_DECRYPT	0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

#ifdef  __cplusplus
extern "C" {
#endif

/* This should be a hidden type, but EVP requires that the size be known */
struct aes_key_st {
#ifdef AES_LONG
    unsigned long rd_key[4 *(AES_MAXNR + 1)];
#else
    unsigned int rd_key[4 *(AES_MAXNR + 1)];
#endif
    int rounds;
};
typedef struct aes_key_st AES_KEY;

const char *AES_options(void);

int AES_set_encrypt_key(__in const unsigned char *userKey, __in const int bits,
	__inout AES_KEY *key);
int AES_set_decrypt_key(__in const unsigned char *userKey, __in const int bits,
	__inout AES_KEY *key);

void AES_encrypt(__in const unsigned char *in, __out unsigned char *out,
	__inout const AES_KEY *key);
void AES_decrypt(__in const unsigned char *in, __out unsigned char *out,
	__inout const AES_KEY *key);

void AES_ecb_encrypt(__in const unsigned char *in, __out unsigned char *out,
	__inout const AES_KEY *key, __in const int enc);
void AES_cbc_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char *ivec, __in const int enc);
void AES_cfb128_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char *ivec, __inout int *num, __in const int enc);
void AES_cfb1_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char *ivec, __inout int *num, __in const int enc);
void AES_cfb8_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char *ivec, __inout int *num, __in const int enc);
void AES_ofb128_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char *ivec, __inout int *num);
void AES_ctr128_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
	__in size_t length, __inout const AES_KEY *key,
	__inout unsigned char ivec[AES_BLOCK_SIZE],
	__inout unsigned char ecount_buf[AES_BLOCK_SIZE],
	__inout unsigned int *num);
/* NB: the IV is _two_ blocks long */
void AES_ige_encrypt(__in_bcount(length) const unsigned char *in, __out_bcount_full(length) unsigned char *out,
		     __in size_t length, __inout const AES_KEY *key,
		     __inout unsigned char *ivec, __in const int enc);
/* NB: the IV is _four_ blocks long */
void AES_bi_ige_encrypt(__in_bcount(length) const unsigned char *in,__out_bcount_full(length) unsigned char *out,
			__in size_t length, __inout const AES_KEY *key,
			__inout const AES_KEY *key2, __inout const unsigned char *ivec,
			__in const int enc);

int AES_wrap_key(__inout AES_KEY *key, __inout const unsigned char *iv,
		__out unsigned char *out,
		__in  const unsigned char *in, __in unsigned int inlen);
int AES_unwrap_key(__inout AES_KEY *key, __inout const unsigned char *iv,
		__out unsigned char *out,
		__in const unsigned char *in, __in unsigned int inlen);


#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_AES_H */
