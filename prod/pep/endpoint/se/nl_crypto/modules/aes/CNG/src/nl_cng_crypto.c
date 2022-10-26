#include <wdm.h>
#include <bcrypt.h>
#include "nl_crypto.h"

#ifdef NLSE_DEBUG_PERFORMANCE

static LARGE_INTEGER g_Total_SetKey_Time;
static LARGE_INTEGER g_Total_CBC_Encrypt_Time;
static LARGE_INTEGER g_Total_CBC_Encrypt_count;
static LARGE_INTEGER g_Total_SetKey_count;


typedef struct _NLPERFORMANCE_COUNTER
{
	LARGE_INTEGER   start;
	LARGE_INTEGER   end;
	LARGE_INTEGER   diff;
	LARGE_INTEGER   freq;
}NLPERFORMANCE_COUNTER, *PNLPERFORMANCE_COUNTER;

VOID
PfStart(
				__out PNLPERFORMANCE_COUNTER ppfc
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
			__out PNLPERFORMANCE_COUNTER ppfc
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



// If the object length that the crypto provider tells us is less than this
// minium, use this mininum number instead.
#define NL_MIN_BCRYPT_OBJECT_LENGTH     1024UL  // in bytes

typedef struct _NL_CNG_MODULE_CONTEXT
{
  BCRYPT_ALG_HANDLE hAesAlg;
  ULONG cbKeyObject;
} NL_CNG_MODULE_CONTEXT, *PNL_CNG_MODULE_CONTEXT;

typedef struct _NL_CNG_KEY_CONTEXT
{
  BCRYPT_KEY_HANDLE hKey;
  UCHAR keyObject[0];       // start of key object
} NL_CNG_KEY_CONTEXT, *PNL_CNG_KEY_CONTEXT;



#define NL_CNG_MODULE_CONTEXT_TAG   'tmCN'
#define NL_CNG_BLOB_TAG             'tbCN'





NTSTATUS DriverEntry( __in struct _DRIVER_OBJECT  *DriverObject,
		      __in PUNICODE_STRING  RegistryPath )
{
  return STATUS_SUCCESS;
}/* DriverEntry */

NTSTATUS DllInitialize( __in PUNICODE_STRING  RegistryPath )
{
  return STATUS_SUCCESS;
}/* DllInitialize */

NTSTATUS DllUnload(void)
{
  return STATUS_SUCCESS;
}/* DllUnload */


__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
static int cng_crypto_decrypt(  __in struct nl_crypto_context* in_ctx ,
		  __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
		  __in const unsigned char* in_ivec ,
		  __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
		  __in size_t in_buf_size )
{

	ULONG cbData = 0;
	UCHAR tempIV[16];

	ASSERT( in_ctx != NULL );
	ASSERT( in_buf != NULL );

	//Copy ivec to a temp variable, 16 which is the key size, hence the hard coding..
	RtlCopyMemory(tempIV,in_ivec,16);


	//Decrypt the buffer
	if(BCryptDecrypt(((const NL_CNG_KEY_CONTEXT *)in_crypto_key)->hKey, in_buf, in_buf_size, NULL,tempIV,16,in_buf, in_buf_size, &cbData, 0))
	{
		DbgPrint("BCryptDecrypt fail\n");
		return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
	}
	
	DbgPrint("[decrypted] length [%d]\n", cbData);

	return NL_CRYPTO_ERROR_SUCCESS;
}/*cng_crypto_decrypt*/

__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
static int cng_crypto_encrypt( __in struct nl_crypto_context* in_ctx ,
		  __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
		  __in const unsigned char* in_ivec ,
		  __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
		  __in size_t in_buf_size )
{

	ULONG cbData = 0;
	UCHAR tempIV[16];

	ASSERT( in_ctx != NULL );
	ASSERT( in_buf != NULL );

	//Allocate and copy ivec to a temp variable
	RtlCopyMemory(tempIV,in_ivec,16);

	//Encrypt the buffer
	DbgPrint("[Encrypting now]\n");
	if(BCryptEncrypt(((const NL_CNG_KEY_CONTEXT *)in_crypto_key)->hKey, in_buf, in_buf_size, NULL,tempIV,16,in_buf, in_buf_size, &cbData, 0))
	{
		DbgPrint("BCryptEncrypt fail\n");
		return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
	}
	DbgPrint("[encrypted] length [%d]\n", cbData);

	return NL_CRYPTO_ERROR_SUCCESS;
}/*cng_crypto_encrypt*/


__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
static int cng_crypto_rand( __in struct nl_crypto_context* in_ctx ,
                            __out int* in_rx )
{
  BCRYPT_ALG_HANDLE hAesAlg = ((PNL_CNG_MODULE_CONTEXT) in_ctx->module_context)->hAesAlg;

  ASSERT( in_rx != NULL );

  if( in_rx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  if(!NT_SUCCESS(BCryptGenRandom( hAesAlg,(PUCHAR)in_rx,sizeof(int), 0)))
  {
	  return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
  }


  return NL_CRYPTO_ERROR_SUCCESS;
}
/* cng_crypto_rand */


__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
int cng_set_crypto_key( __in struct nl_crypto_context* in_ctx ,
                        __in_bcount(in_key_size) const unsigned char* in_key ,
                        __in size_t in_key_size ,
                        __in BOOLEAN in_is_encrypt ,
                        __out_bcount_full(in_ctx->crypto_key_size) void *out_crypto_key )
{
	PNL_CNG_MODULE_CONTEXT pModuleContext = in_ctx->module_context;
	BCRYPT_KEY_HANDLE	hKey =NULL;
	PUCHAR pbKeyObject = NULL;
	PUCHAR pbBlob = NULL;
	PNL_CNG_KEY_CONTEXT pKeyCtx = out_crypto_key;


	pbBlob = ExAllocatePoolWithTag(NonPagedPool, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + in_key_size, NL_CNG_BLOB_TAG);
	
	if(!pbBlob)
	{
		DbgPrint("ExAllocatePoolWithTag 2 fail\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	
	//	fill key data blob
	RtlZeroMemory(pbBlob, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + in_key_size);
	((PBCRYPT_KEY_DATA_BLOB_HEADER)pbBlob)->cbKeyData = in_key_size;
	((PBCRYPT_KEY_DATA_BLOB_HEADER)pbBlob)->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
	((PBCRYPT_KEY_DATA_BLOB_HEADER)pbBlob)->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
	
	RtlCopyMemory(pbBlob + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), in_key, in_key_size);
	
	//	import key
    pbKeyObject = pKeyCtx->keyObject;
	if(BCryptImportKey(pModuleContext->hAesAlg, NULL, BCRYPT_KEY_DATA_BLOB, &hKey, pbKeyObject, pModuleContext->cbKeyObject, pbBlob, sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + in_key_size, 0))
	{
		DbgPrint("BCryptImportKey fail\n");
        ExFreePoolWithTag(pbBlob, NL_CNG_BLOB_TAG);
		return NL_CRYPTO_ERROR_GENERAL_FAILURE;
	}

    ExFreePoolWithTag(pbBlob, NL_CNG_BLOB_TAG);
	pKeyCtx->hKey = hKey ;

	return NL_CRYPTO_ERROR_SUCCESS;
}

__drv_maxIRQL(DISPATCH_LEVEL)
__checkReturn
int cng_destroy_crypto_key(  __in struct nl_crypto_context* in_ctx ,
                             __inout_bcount(in_ctx->crypto_key_size) void *in_crypto_key )
{
	PNL_CNG_MODULE_CONTEXT pModuleContext = in_ctx->module_context;
	PNL_CNG_KEY_CONTEXT pKeyCtx = in_crypto_key;

	RtlSecureZeroMemory(&pKeyCtx->keyObject, pModuleContext->cbKeyObject);

	if(!NT_SUCCESS(BCryptDestroyKey(pKeyCtx->hKey)))
	{
		return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
	}

    return NL_CRYPTO_ERROR_SUCCESS;

}

__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
int nl_crypto_initialize( __out struct nl_crypto_context* in_ctx ,
			  __in int in_options )
{
  BCRYPT_ALG_HANDLE hAesAlg = NULL;
  ULONG cbKeyObject = 0;
  ULONG cbData;
  PNL_CNG_MODULE_CONTEXT pModuleContext = NULL;

	ASSERT( in_ctx != NULL );
	ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }


	if(!NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAesAlg, BCRYPT_AES_ALGORITHM, NULL, BCRYPT_PROV_DISPATCH)))
	{
		return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
	}

	if(!NT_SUCCESS(BCryptSetProperty(hAesAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC),0)))
	{
		DbgPrint("BCryptSetProperty fail\n");
        BCryptCloseAlgorithmProvider(hAesAlg, 0);
        hAesAlg = NULL;
		return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
	}

	if(!NT_SUCCESS(BCryptGetProperty(hAesAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObject, sizeof(ULONG), &cbData, 0)))
    {
		DbgPrint("BCryptGetProperty fail\n");
        BCryptCloseAlgorithmProvider(hAesAlg, 0);
        hAesAlg = NULL;
		return  NL_CRYPTO_ERROR_GENERAL_FAILURE;
    }

        if (cbKeyObject < NL_MIN_BCRYPT_OBJECT_LENGTH)
        {
          DbgPrint("BCRYPT_OBJECT_LENGTH returned by system is %lu bytes which is less than %lu bytes.  Using %lu bytes instead.", cbKeyObject, NL_MIN_BCRYPT_OBJECT_LENGTH, NL_MIN_BCRYPT_OBJECT_LENGTH);
          cbKeyObject = NL_MIN_BCRYPT_OBJECT_LENGTH;
        }

	in_ctx->rand				=	cng_crypto_rand;
	in_ctx->init_crypto_key		=	cng_set_crypto_key;
	in_ctx->encrypt				=	cng_crypto_encrypt;
	in_ctx->decrypt				=	cng_crypto_decrypt;
	in_ctx->destroy_crypto_key	=	cng_destroy_crypto_key;
	in_ctx->crypto_key_size		=	sizeof(NL_CNG_KEY_CONTEXT) + cbKeyObject;
	in_ctx->block_size			=	16;

        pModuleContext = ExAllocatePoolWithTag(NonPagedPool, sizeof(NL_CNG_MODULE_CONTEXT), NL_CNG_MODULE_CONTEXT_TAG);
        if (pModuleContext == NULL)
        {
          return NL_CRYPTO_ERROR_GENERAL_FAILURE;
        }

        pModuleContext->hAesAlg = hAesAlg;
        pModuleContext->cbKeyObject = cbKeyObject;
        in_ctx->module_context = pModuleContext;

  return NL_CRYPTO_ERROR_SUCCESS;
}/* nl_crypto_initialize */

__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
int nl_crypto_shutdown( __in struct nl_crypto_context* in_ctx )
{
  PNL_CNG_MODULE_CONTEXT pModuleContext;

  ASSERT( in_ctx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  pModuleContext = in_ctx->module_context;
  BCryptCloseAlgorithmProvider(pModuleContext->hAesAlg, 0);
  ExFreePoolWithTag(pModuleContext, NL_CNG_MODULE_CONTEXT_TAG);

  return NL_CRYPTO_ERROR_SUCCESS;
}/* nl_crypto_shutdown */
