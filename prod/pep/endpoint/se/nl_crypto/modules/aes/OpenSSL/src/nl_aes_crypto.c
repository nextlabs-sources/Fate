
#include <ntifs.h>
#include <wdm.h>
#include <limits.h>

#include "openssl/aes.h"

#include "nl_crypto.h"

// The declaration of RtlRandom() in ntifs.h does not contain the proper
// __drv_maxIRQL() annotation, so we add it here ourselves.
__drv_maxIRQL(APC_LEVEL)
__range(<, MAXLONG)
NTSYSAPI
ULONG
NTAPI
RtlRandom (
    __inout PULONG Seed
    );

#ifdef NLSE_DEBUG_PERFORMANCE

static LARGE_INTEGER g_Total_SetDecryptKey_Time;
static LARGE_INTEGER g_Total_CBC_Encrypt_Time;
static LARGE_INTEGER g_Total_CBC_Encrypt_count;
static LARGE_INTEGER g_Total_SetDecryptKey_count;

typedef struct _NLPERFORMANCE_COUNTER
{
	LARGE_INTEGER   start;
	LARGE_INTEGER   end;
	LARGE_INTEGER   diff;
	LARGE_INTEGER   freq;
}NLPERFORMANCE_COUNTER, *PNLPERFORMANCE_COUNTER;

VOID
PfStart(
				PNLPERFORMANCE_COUNTER ppfc
				)
{
	ppfc->start = KeQueryPerformanceCounter(&ppfc->freq);
	ppfc->freq.QuadPart = ppfc->freq.QuadPart/1000000; // frequency per microseconds
}

/*
It gets time diff in microseconds
*/
VOID
PfEnd(
			PNLPERFORMANCE_COUNTER ppfc
			)
{
	ppfc->end = KeQueryPerformanceCounter(NULL);
	// Diff time is in microseconds.
	if(0 != ppfc->freq.QuadPart)
		ppfc->diff.QuadPart = (ppfc->end.QuadPart - ppfc->start.QuadPart)/ppfc->freq.QuadPart;
	else
		ppfc->diff.QuadPart = ppfc->end.QuadPart - ppfc->start.QuadPart;
}

#endif


// Functions to test the AES code from OpenSSL.
// All test vectors came from <http://en.wikipedia.org/w/index.php?title=Advanced_Encryption_Standard&oldid=331927138>
#if 0

static void AESTest1(void)
{
  const unsigned char key[] = {
    0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x08,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x12
  };
  const unsigned char dec[AES_BLOCK_SIZE] = {
    0x50, 0x68, 0x12, 0xA4, 0x5F, 0x08, 0xC8, 0x89,
    0xB9, 0x7F, 0x59, 0x80, 0x03, 0x8B, 0x83, 0x59
  };
  const unsigned char enc[AES_BLOCK_SIZE] = {
    0xD8, 0xF5, 0x32, 0x53, 0x82, 0x89, 0xEF, 0x7D,
    0x06, 0xB5, 0x06, 0xA4, 0xFD, 0x5B, 0xE9, 0xC9
  };
  unsigned char out[AES_BLOCK_SIZE];
  AES_KEY aesKey;
  int i;

  DbgPrint(__FUNCTION__ "\n");

  // encrypt
  AES_set_encrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlZeroMemory(out, sizeof out);
  AES_encrypt(dec, out, &aesKey);
  if (RtlCompareMemory(out, enc, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": encrypt out matches enc\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": encrypt out doesn't match enc\n");
  }

  // decrypt
  AES_set_decrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlZeroMemory(out, sizeof out);
  AES_decrypt(enc, out, &aesKey);
  if (RtlCompareMemory(out, dec, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": decrypt out matches dec\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": decrypt out doesn't match dec\n");
  }
}

static void AESTest2(void)
{
  const unsigned char key[256 / CHAR_BIT] = {
    0x95, 0xA8, 0xEE, 0x8E, 0x89, 0x97, 0x9B, 0x9E,
    0xFD, 0xCB, 0xC6, 0xEB, 0x97, 0x97, 0x52, 0x8D,
    0x43, 0x2D, 0xC2, 0x60, 0x61, 0x55, 0x38, 0x18,
    0xEA, 0x63, 0x5E, 0xC5, 0xD5, 0xA7, 0x72, 0x7E
  };
  const int keyBitSize[3] = {128, 192, 256};
  const unsigned char in[AES_BLOCK_SIZE] = {
    0x4E, 0xC1, 0x37, 0xA4, 0x26, 0xDA, 0xBF, 0x8A,
    0xA0, 0xBE, 0xB8, 0xBC, 0x0C, 0x2B, 0x89, 0xD6
  };
  const unsigned char enc[3][AES_BLOCK_SIZE] = {
    {0xD9, 0xB6, 0x5D, 0x12, 0x32, 0xBA, 0x01, 0x99,
     0xCD, 0xBD, 0x48, 0x7B, 0x2A, 0x1F, 0xD6, 0x46},
    {0xB1, 0x8B, 0xB3, 0xE7, 0xE1, 0x07, 0x32, 0xBE,
     0x13, 0x58, 0x44, 0x3A, 0x50, 0x4D, 0xBB, 0x49},
    {0x2F, 0x9C, 0xFD, 0xDB, 0xFF, 0xCD, 0xE6, 0xB9,
     0xF3, 0x7E, 0xF8, 0xE4, 0x0D, 0x51, 0x2C, 0xF4}
  };
  const unsigned char dec[3][AES_BLOCK_SIZE] = {
    {0x95, 0x70, 0xC3, 0x43, 0x63, 0x56, 0x5B, 0x39,
     0x35, 0x03, 0xA0, 0x01, 0xC0, 0xE2, 0x3B, 0x65},
    {0x29, 0xDF, 0xD7, 0x5B, 0x85, 0xCE, 0xE4, 0xDE,
     0x6E, 0x26, 0xA8, 0x08, 0xCD, 0xC2, 0xC9, 0xC3},
    {0x11, 0x0A, 0x35, 0x45, 0xCE, 0x49, 0xB8, 0x4B,
     0xBB, 0x7B, 0x35, 0x23, 0x61, 0x08, 0xFA, 0x6E}
  };
  unsigned char out[AES_BLOCK_SIZE];
  int j;

  DbgPrint(__FUNCTION__ "\n");

  for (j = 0; j < 3; j++)
  {
    AES_KEY aesKey;
    int i;

    DbgPrint(__FUNCTION__ ": key size = %d bits\n", keyBitSize[j]);

    // encrypt
    AES_set_encrypt_key(key, keyBitSize[j], &aesKey);
    RtlZeroMemory(out, sizeof out);
    AES_encrypt(in, out, &aesKey);
    if (RtlCompareMemory(out, enc[j], sizeof out) == sizeof out)
    {
      DbgPrint(__FUNCTION__ ": encrypt out matches enc\n");
    }
    else
    {
      DbgPrint(__FUNCTION__ ": encrypt out doesn't match enc\n");
    }

    // decrypt
    AES_set_decrypt_key(key, keyBitSize[j], &aesKey);
    RtlZeroMemory(out, sizeof out);
    AES_decrypt(in, out, &aesKey);
    if (RtlCompareMemory(out, dec[j], sizeof out) == sizeof out)
    {
      DbgPrint(__FUNCTION__ ": decrypt out matches dec\n");
    }
    else
    {
      DbgPrint(__FUNCTION__ ": decrypt out doesn't match dec\n");
    }
  }
}

static void AESTest3(void)
{
  const unsigned char key[256 / CHAR_BIT] = {
    0x95, 0xA8, 0xEE, 0x8E, 0x89, 0x97, 0x9B, 0x9E,
    0xFD, 0xCB, 0xC6, 0xEB, 0x97, 0x97, 0x52, 0x8D,
    0x43, 0x2D, 0xC2, 0x60, 0x61, 0x55, 0x38, 0x18,
    0xEA, 0x63, 0x5E, 0xC5, 0xD5, 0xA7, 0x72, 0x7E
  };
  const int keyBitSize[3] = {128, 192, 256};
  const unsigned char in[AES_BLOCK_SIZE] = {
    0x4E, 0xC1, 0x37, 0xA4, 0x26, 0xDA, 0xBF, 0x8A,
    0xA0, 0xBE, 0xB8, 0xBC, 0x0C, 0x2B, 0x89, 0xD6
  };
  const unsigned char enc[3][AES_BLOCK_SIZE] = {
    {0xD9, 0xB6, 0x5D, 0x12, 0x32, 0xBA, 0x01, 0x99,
     0xCD, 0xBD, 0x48, 0x7B, 0x2A, 0x1F, 0xD6, 0x46},
    {0xB1, 0x8B, 0xB3, 0xE7, 0xE1, 0x07, 0x32, 0xBE,
     0x13, 0x58, 0x44, 0x3A, 0x50, 0x4D, 0xBB, 0x49},
    {0x2F, 0x9C, 0xFD, 0xDB, 0xFF, 0xCD, 0xE6, 0xB9,
     0xF3, 0x7E, 0xF8, 0xE4, 0x0D, 0x51, 0x2C, 0xF4}
  };
  const unsigned char dec[3][AES_BLOCK_SIZE] = {
    {0x95, 0x70, 0xC3, 0x43, 0x63, 0x56, 0x5B, 0x39,
     0x35, 0x03, 0xA0, 0x01, 0xC0, 0xE2, 0x3B, 0x65},
    {0x29, 0xDF, 0xD7, 0x5B, 0x85, 0xCE, 0xE4, 0xDE,
     0x6E, 0x26, 0xA8, 0x08, 0xCD, 0xC2, 0xC9, 0xC3},
    {0x11, 0x0A, 0x35, 0x45, 0xCE, 0x49, 0xB8, 0x4B,
     0xBB, 0x7B, 0x35, 0x23, 0x61, 0x08, 0xFA, 0x6E}
  };
  unsigned char buf[AES_BLOCK_SIZE];
  int j;

  DbgPrint(__FUNCTION__ "\n");

  for (j = 0; j < 3; j++)
  {
    AES_KEY aesKey;
    int i;

    DbgPrint(__FUNCTION__ ": key size = %d bits\n", keyBitSize[j]);

    // encrypt
    AES_set_encrypt_key(key, keyBitSize[j], &aesKey);
    RtlCopyMemory(buf, in, sizeof buf);
    AES_encrypt(buf, buf, &aesKey);
    if (RtlCompareMemory(buf, enc[j], sizeof buf) == sizeof buf)
    {
      DbgPrint(__FUNCTION__ ": encrypt buf matches enc\n");
    }
    else
    {
      DbgPrint(__FUNCTION__ ": encrypt buf doesn't match enc\n");
    }

    // decrypt
    AES_set_decrypt_key(key, keyBitSize[j], &aesKey);
    RtlCopyMemory(buf, in, sizeof buf);
    AES_decrypt(buf, buf, &aesKey);
    if (RtlCompareMemory(buf, dec[j], sizeof buf) == sizeof buf)
    {
      DbgPrint(__FUNCTION__ ": decrypt buf matches dec\n");
    }
    else
    {
      DbgPrint(__FUNCTION__ ": decrypt buf doesn't match dec\n");
    }
  }
}

#endif



// Functions to test AES CBC from OpenSSL.
// All test vectors came from <http://csrc.nist.gov/groups/STM/cavp/documents/aes/KAT_AES.zip>
#if 0

static void AESCBCTest1(void)
{
  const unsigned char key[] = {
    0x10, 0xa5, 0x88, 0x69, 0xd7, 0x4b, 0xe5, 0xa3,
    0x74, 0xcf, 0x86, 0x7c, 0xfb, 0x47, 0x38, 0x59
  };
  const unsigned char iv[AES_BLOCK_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  const unsigned char plainText[AES_BLOCK_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
  const unsigned char cipherText[AES_BLOCK_SIZE] = {
    0x6d, 0x25, 0x1e, 0x69, 0x44, 0xb0, 0x51, 0xe0,
    0x4e, 0xaa, 0x6f, 0xb4, 0xdb, 0xf7, 0x84, 0x65
  };
  unsigned char ivec[AES_BLOCK_SIZE];
  unsigned char out[AES_BLOCK_SIZE];
  AES_KEY aesKey;
  int i;

  DbgPrint(__FUNCTION__ "\n");

  // encrypt to different buffer
  AES_set_encrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlCopyMemory(ivec, iv, sizeof ivec);
  RtlZeroMemory(out, sizeof out);
  AES_cbc_encrypt(plainText, out, sizeof plainText, &aesKey, ivec,
                  AES_ENCRYPT);
  if (RtlCompareMemory(out, cipherText, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": encrypt out matches cipherText\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": encrypt out doesn't match cipherText\n");
  }

  // encrypt to same buffer
  AES_set_encrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlCopyMemory(ivec, iv, sizeof ivec);
  RtlCopyMemory(out, plainText, sizeof out);
  AES_cbc_encrypt(out, out, sizeof out, &aesKey, ivec,
                  AES_ENCRYPT);
  if (RtlCompareMemory(out, cipherText, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": encrypt out matches cipherText\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": encrypt out doesn't match cipherText\n");
  }

  // decrypt to different buffer
  AES_set_decrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlCopyMemory(ivec, iv, sizeof ivec);
  RtlZeroMemory(out, sizeof out);
  AES_cbc_encrypt(cipherText, out, sizeof cipherText, &aesKey, ivec,
                  AES_DECRYPT);
  if (RtlCompareMemory(out, plainText, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": decrypt out matches plainText\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": decrypt out doesn't match plainText\n");
  }

  // decrypt to same buffer
  AES_set_decrypt_key(key, RTL_BITS_OF(key), &aesKey);
  RtlCopyMemory(ivec, iv, sizeof ivec);
  RtlCopyMemory(out, cipherText, sizeof out);
  AES_cbc_encrypt(out, out, sizeof out, &aesKey, ivec,
                  AES_DECRYPT);
  if (RtlCompareMemory(out, plainText, sizeof out) == sizeof out)
  {
    DbgPrint(__FUNCTION__ ": decrypt out matches plainText\n");
  }
  else
  {
    DbgPrint(__FUNCTION__ ": decrypt out doesn't match plainText\n");
  }
}

#endif



static ULONG randomSeed;
static FAST_MUTEX randomSeedLock;

NTSTATUS DriverEntry( __in struct _DRIVER_OBJECT  *DriverObject,
		      __in PUNICODE_STRING  RegistryPath )
{
  return STATUS_SUCCESS;
}/* DriverEntry */

NTSTATUS DllInitialize( __in PUNICODE_STRING  RegistryPath )
{
  LARGE_INTEGER perfCounter;
  int i;

  // Init the global random seed.
  perfCounter = KeQueryPerformanceCounter(NULL);
  randomSeed = perfCounter.LowPart ^ perfCounter.HighPart;

  // Init the global random seed lock.
  ExInitializeFastMutex(&randomSeedLock);

  RtlSecureZeroMemory(&perfCounter, sizeof perfCounter);
  return STATUS_SUCCESS;
}/* DllInitialize */

NTSTATUS DllUnload(void)
{
  return STATUS_SUCCESS;
}/* DllUnload */

__drv_maxIRQL(APC_LEVEL)
__checkReturn
static int aes_crypto_rand( __in struct nl_crypto_context* in_ctx ,
                            __out int* in_rx )
{
  ASSERT( in_rx != NULL );
  ASSERT( KeGetCurrentIrql() <= APC_LEVEL );

  if( in_rx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  ExAcquireFastMutex(&randomSeedLock);
  *in_rx = (int)RtlRandom(&randomSeed); /* random ULONG w/o presisted key */
  ExReleaseFastMutex(&randomSeedLock);

  return NL_CRYPTO_ERROR_SUCCESS;
}/* aes_crypto_rand */

__checkReturn
static int aes_crypto_init_key( __in struct nl_crypto_context* in_ctx ,
                                __in_bcount(in_key_size) const unsigned char* in_key ,
                                size_t in_key_size ,
                                BOOLEAN in_is_encrypt ,
                                __out_bcount_full(in_ctx->crypto_key_size) void *out_crypto_key )
{
  AES_KEY *pAesKey;

#ifdef NLSE_DEBUG_PERFORMANCE
  NLPERFORMANCE_COUNTER pfc_AES_set_decrypt_key;
#endif

  ASSERT( in_ctx != NULL );
  ASSERT( in_key != NULL );
  ASSERT( in_ctx->crypto_key_size == sizeof *pAesKey );
  ASSERT( out_crypto_key != NULL );

  // encrypt_buffer() and decrypt_buffer() in NLSE filter driver uses an IV
  // whose size is the same size as the key size, while AES_cbc_encrypt()
  // below assumes that the IV size is AES_BLOCK_SIZE.  Hence it won't work
  // for us if the key size is not AES_BLOCK_SIZE.
  ASSERT(in_key_size == AES_BLOCK_SIZE);

  pAesKey = out_crypto_key;

  if( in_is_encrypt )
  {
    if( AES_set_encrypt_key( in_key, in_key_size * CHAR_BIT, pAesKey ) != 0 )
    {
      return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
    }
  }
  else
  {
#ifdef NLSE_DEBUG_PERFORMANCE
    g_Total_SetDecryptKey_count.QuadPart++;
    KdPrint(("AES_set_decrypt_key count total = %I64d\n", g_Total_SetDecryptKey_count.QuadPart));

    PfStart(&pfc_AES_set_decrypt_key);
#endif

    if( AES_set_decrypt_key( in_key, in_key_size * CHAR_BIT, pAesKey ) != 0 )
    {
#ifdef NLSE_DEBUG_PERFORMANCE
      PfEnd(&pfc_AES_set_decrypt_key);

      if (pfc_AES_set_decrypt_key.diff.QuadPart)
      {
        g_Total_SetDecryptKey_Time.QuadPart += pfc_AES_set_decrypt_key.diff.QuadPart;
        KdPrint(("AES_set_decrypt_key elasped time total = %I64d microseconds\n", g_Total_SetDecryptKey_Time.QuadPart));
      }
#endif

      return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
    }
#ifdef NLSE_DEBUG_PERFORMANCE
    PfEnd(&pfc_AES_set_decrypt_key);

    if (pfc_AES_set_decrypt_key.diff.QuadPart)
    {
      g_Total_SetDecryptKey_Time.QuadPart += pfc_AES_set_decrypt_key.diff.QuadPart;
      KdPrint(("AES_set_decrypt_key elasped time total = %I64d microseconds\n", g_Total_SetDecryptKey_Time.QuadPart));
    }
#endif
  }

  return NL_CRYPTO_ERROR_SUCCESS;
}

static int aes_crypto_destroy_key( __in struct nl_crypto_context* in_ctx ,
                                   __inout_bcount(in_ctx->crypto_key_size) void *in_crypto_key )
{
  AES_KEY *pAesKey;

  ASSERT( in_ctx != NULL );
  ASSERT( in_ctx->crypto_key_size == sizeof *pAesKey );
  ASSERT( in_crypto_key != NULL );

  pAesKey = in_crypto_key;
  RtlSecureZeroMemory( pAesKey, sizeof *pAesKey );
  return NL_CRYPTO_ERROR_SUCCESS;
}

__checkReturn
static int aes_crypto_encrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
			 __in_bcount(AES_BLOCK_SIZE) const unsigned char* in_ivec ,
			 __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
			 __in size_t in_buf_size )
{
  const AES_KEY *pAesKey;
  unsigned char ivec[AES_BLOCK_SIZE];

  ASSERT( in_ctx != NULL );
  ASSERT( in_crypto_key != NULL );
  ASSERT( in_buf != NULL );
  ASSERT( in_buf_size % AES_BLOCK_SIZE == 0 );

  pAesKey = in_crypto_key;

  memcpy(ivec, in_ivec, sizeof ivec);
  AES_cbc_encrypt(in_buf, in_buf, in_buf_size, pAesKey, ivec, AES_ENCRYPT);
  RtlSecureZeroMemory(ivec, sizeof ivec);

  return NL_CRYPTO_ERROR_SUCCESS;
}/* fake_encrypt */

__checkReturn
static int aes_crypto_decrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
			 __in_bcount(AES_BLOCK_SIZE) const unsigned char* in_ivec ,
			 __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
			 size_t in_buf_size )
{
  const AES_KEY *pAesKey;
  unsigned char ivec[AES_BLOCK_SIZE];
  
#ifdef NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc_AES_cbc_encrypt;
#endif

  ASSERT( in_ctx != NULL );
  ASSERT( in_crypto_key != NULL );
  ASSERT( in_buf != NULL );
  ASSERT( in_buf_size % AES_BLOCK_SIZE == 0 );

  pAesKey = in_crypto_key;

  memcpy(ivec, in_ivec, sizeof ivec);

#ifdef NLSE_DEBUG_PERFORMANCE
	g_Total_CBC_Encrypt_count.QuadPart++;
	KdPrint(("AES_cbc_encrypt count total = %I64d\n", g_Total_CBC_Encrypt_count.QuadPart));
	PfStart(
		&pfc_AES_cbc_encrypt
		);	
#endif

  AES_cbc_encrypt(in_buf, in_buf, in_buf_size, pAesKey, ivec, AES_DECRYPT);

#ifdef NLSE_DEBUG_PERFORMANCE
	PfEnd(
		&pfc_AES_cbc_encrypt
		);

	if (pfc_AES_cbc_encrypt.diff.QuadPart)
	{
		g_Total_CBC_Encrypt_Time.QuadPart += pfc_AES_cbc_encrypt.diff.QuadPart;
		KdPrint(("AES_cbc_encrypt elasped time total = %I64d milliseconds\n", g_Total_CBC_Encrypt_Time.QuadPart));
	}
#endif

  RtlSecureZeroMemory(ivec, sizeof ivec);

  return NL_CRYPTO_ERROR_SUCCESS;
}/* fake_decrypt */

__drv_requiresIRQL(PASSIVE_LEVEL)
int nl_crypto_initialize( __out struct nl_crypto_context* in_ctx ,
			  __in int in_options )
{
  ASSERT( in_ctx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  in_ctx->rand       = aes_crypto_rand;
  in_ctx->init_crypto_key       = aes_crypto_init_key;
  in_ctx->destroy_crypto_key    = aes_crypto_destroy_key;
  in_ctx->encrypt    = aes_crypto_encrypt;
  in_ctx->decrypt    = aes_crypto_decrypt;

  in_ctx->crypto_key_size   = sizeof(AES_KEY);
  in_ctx->block_size = AES_BLOCK_SIZE;

  in_ctx->module_context = NULL;     /* no context */

  return NL_CRYPTO_ERROR_SUCCESS;
}/* nl_crypto_initialize */

__drv_requiresIRQL(PASSIVE_LEVEL)
int nl_crypto_shutdown( __in struct nl_crypto_context* in_ctx )
{
  ASSERT( in_ctx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  return NL_CRYPTO_ERROR_SUCCESS;
}/* nl_crypto_shutdown */
