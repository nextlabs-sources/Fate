/* ====================================================================
 * Copyright (c) 2008 The OpenSSL Project. All rights reserved.
 *
 * Rights for redistribution and usage in source and binary
 * forms are granted according to the OpenSSL license.
 */

#include <stddef.h>

typedef void (*block128_f)(__in const unsigned char in[16],
			__out unsigned char out[16],
			__in const void *key);

typedef void (*cbc128_f)(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			 __in size_t len,  __in const void *key,
			 __inout unsigned char ivec[16], __in int enc);

void CRYPTO_cbc128_encrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __in block128_f block);
void CRYPTO_cbc128_decrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __in block128_f block);

void CRYPTO_ctr128_encrypt(__in_bcount(len)  const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __inout unsigned char ecount_buf[16],
			__inout unsigned int *num, __in block128_f block);

void CRYPTO_ofb128_encrypt(__in_bcount(len)  const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __inout int *num,
			__in block128_f block);

void CRYPTO_cfb128_encrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __inout int *num,
			__in int enc, __in block128_f block);
void CRYPTO_cfb128_8_encrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t length, __in const void *key,
			__inout unsigned char ivec[16], __inout int *num,
			__in int enc, __in block128_f block);
void CRYPTO_cfb128_1_encrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t bits, __in const void *key,
			__inout unsigned char ivec[16], __inout int *num,
			__in int enc, __in block128_f block);

size_t CRYPTO_cts128_encrypt_block(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16],__in  block128_f block);
size_t CRYPTO_cts128_encrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __in cbc128_f cbc);
size_t CRYPTO_cts128_decrypt_block(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __in block128_f block);
size_t CRYPTO_cts128_decrypt(__in_bcount(len) const unsigned char *in, __out_bcount_full(len) unsigned char *out,
			__in size_t len, __in const void *key,
			__inout unsigned char ivec[16], __in cbc128_f cbc);
