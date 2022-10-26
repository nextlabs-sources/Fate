/*++

Module Name:

  NLSECrypto.c

Abstract:
  Calling crypto API for encryption/decryption in kernel mode. Also,
  handling CBC cypher block mode. 

Environment:

    Kernel mode

--*/
#include "NLSEStruct.h"

//
//  Global variables
//
extern NLFSE_GLOBAL_DATA nlfseGlobal;

#ifdef	NLSE_DEBUG_PERFORMANCE
static size_t g_TotalBufferSize;
static LARGE_INTEGER g_TotalDecrypt_time;
#endif

BOOLEAN decrypt_buffer( __in_bcount(keyLen) char  *decryptKey, 
			       __in size_t keyLen,
			       __in ULONGLONG startOffset,
			       __inout PVOID cryptoBuffer, 
			       __in size_t cryptoSize
			       )

{
#if NLSE_DEBUG_DATA_VERIFY
  ULONG i = 0;
  PUCHAR p = (PUCHAR)cryptoBuffer;
  for( i = 0 ; i < cryptoSize ; i++ )
  {
    /* Padding value may be 0x00, so ignore that.  Looking for non-padding and non-know
     * data size.
     */
    if( (UCHAR)p[i] != 0 && (UCHAR)p[i] != (UCHAR)'b' )
    {
      KdPrint(("NLSE: NLSE_DEBUG_DATA_VERIFY: Verification failed (0x%X,0x%X,0x%X,0x%X)\n",
	       (ULONG_PTR)cryptoBuffer,       /* Base address of memory */
	       (ULONG_PTR)i,                  /* Offset of mismatch in this block */
	       (ULONG_PTR)startOffset,        /* Offset of this block in the file */
	       (ULONG_PTR)p[i]));
#if NLSE_DEBUG_DATA_VERIFY_BUGCHECK
      KeBugCheckEx(0xBEEFFEED,
		   (ULONG_PTR)cryptoBuffer,       /* Base address of memory */
		   (ULONG_PTR)i,                  /* Offset of mismatch in this block */
		   (ULONG_PTR)startOffset,        /* Offset of this block in the file */
		   (ULONG_PTR)p[i]);              /* Value of unexpected byte */
#endif
    }
  }
#elif NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing*/
#else

#ifdef	NLSE_DEBUG_PERFORMANCE
	NLPERFORMANCE_COUNTER pfc_decryptbuffer;	
#endif

  /* Allocate initial vector and use the current offset as its value */
  UCHAR ivec[NLE_KEY_LENGTH_IN_BYTES];
  PUCHAR cryptoBufferThisRun;
  void *pCryptoKey;

  ASSERT( NLE_KEY_LENGTH_IN_BYTES >= keyLen );

#ifdef	NLSE_DEBUG_PERFORMANCE
  if(cryptoSize > 0)
  {
	  g_TotalBufferSize += cryptoSize;
	  KdPrint(("decrypt_buffer total = %lu\n", g_TotalBufferSize ));
  }
  PfStart(
	  &pfc_decryptbuffer
	  );
#endif
  
  /* Allocate and init crypto key */
  pCryptoKey = ExAllocatePoolWithTag(NonPagedPool,
                                     nlfseGlobal.crypto_ctx.crypto_key_size,
                                     NLSE_CRYPTOKEY_TAG);
  if (pCryptoKey == NULL)
  {
    return FALSE;
  }

  if (nlfseGlobal.crypto_ctx.init_crypto_key(&nlfseGlobal.crypto_ctx,
                                             decryptKey, keyLen, FALSE,
                                             pCryptoKey)
      != NL_CRYPTO_ERROR_SUCCESS)
  {
    ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);
    return FALSE;
  }

  RtlZeroMemory(ivec,keyLen);
  cryptoBufferThisRun = cryptoBuffer;

  while (cryptoSize > 0)
  {
    size_t cryptoSizeThisRun = min(cryptoSize, nlfseGlobal.cryptoBlockSize);

    RtlCopyMemory(ivec,&startOffset,sizeof(startOffset));
    nlfseGlobal.crypto_ctx.decrypt(&nlfseGlobal.crypto_ctx,   /* context */
                                   pCryptoKey,                /* key */
                                   ivec,                      /* ivec */
                                   cryptoBufferThisRun,   /* caller's buffer */
                                   cryptoSizeThisRun);
    cryptoBufferThisRun += cryptoSizeThisRun;
    startOffset += cryptoSizeThisRun;
    cryptoSize -= cryptoSizeThisRun;
  }

  RtlSecureZeroMemory(ivec,keyLen);

  /* Securely destroy and free crypto key */
  nlfseGlobal.crypto_ctx.destroy_crypto_key(&nlfseGlobal.crypto_ctx,
                                            pCryptoKey);
  ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);

#ifdef	NLSE_DEBUG_PERFORMANCE
  PfEnd(
	  &pfc_decryptbuffer
	  );
  if (pfc_decryptbuffer.diff.QuadPart )
  {
	  g_TotalDecrypt_time.QuadPart += pfc_decryptbuffer.diff.QuadPart;
	  KdPrint(("decrypt_buffer time total = %I64d microseconds\n", g_TotalDecrypt_time.QuadPart));
  }
#endif

#endif
  return TRUE;
}

BOOLEAN encrypt_buffer(__in_bcount(keyLen) char  *encryptKey, 
			       __in size_t keyLen,
			       __in ULONGLONG startOffset,
			       __inout_bcount(cryptoSize) PVOID cryptoBuffer, 
			       __in size_t cryptoSize
			       )
{
#if NLSE_DEBUG_DATA_VERIFY
  RtlFillMemory(cryptoBuffer,cryptoSize,(UCHAR)'b');
#elif NLSE_DEBUG_CRYPTO_PASSTHROUGH
  /* Do nothing */
#else
  /* Allocate initial vector and use the current offset as its value */
  UCHAR ivec[NLE_KEY_LENGTH_IN_BYTES];
  PUCHAR cryptoBufferThisRun;
  void *pCryptoKey;

  ASSERT( NLE_KEY_LENGTH_IN_BYTES >= keyLen );

  /* Allocate and init crypto key */
  pCryptoKey = ExAllocatePoolWithTag(NonPagedPool,
                                     nlfseGlobal.crypto_ctx.crypto_key_size,
                                     NLSE_CRYPTOKEY_TAG);
  if (pCryptoKey == NULL)
  {
    return FALSE;
  }

  if (nlfseGlobal.crypto_ctx.init_crypto_key(&nlfseGlobal.crypto_ctx,
                                             encryptKey, keyLen, TRUE,
                                             pCryptoKey)
      != NL_CRYPTO_ERROR_SUCCESS)
  {
    ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);
    return FALSE;
  }

  RtlZeroMemory(ivec,keyLen);
  cryptoBufferThisRun = cryptoBuffer;

  while (cryptoSize > 0)
  {
    size_t cryptoSizeThisRun = min(cryptoSize, nlfseGlobal.cryptoBlockSize);

    RtlCopyMemory(ivec,&startOffset,sizeof(startOffset));
    nlfseGlobal.crypto_ctx.encrypt(&nlfseGlobal.crypto_ctx,   /* context */
                                   pCryptoKey,                /* key */
                                   ivec,                      /* ivec */
                                   cryptoBufferThisRun,   /* caller's buffer */
                                   cryptoSizeThisRun);
    cryptoBufferThisRun += cryptoSizeThisRun;
    startOffset += cryptoSizeThisRun;
    cryptoSize -= cryptoSizeThisRun;
  }

  RtlSecureZeroMemory(ivec,keyLen);

  /* Securely destroy and free crypto key */
  nlfseGlobal.crypto_ctx.destroy_crypto_key(&nlfseGlobal.crypto_ctx,
                                            pCryptoKey);
  ExFreePoolWithTag(pCryptoKey, NLSE_CRYPTOKEY_TAG);

#endif
  return TRUE;
}
