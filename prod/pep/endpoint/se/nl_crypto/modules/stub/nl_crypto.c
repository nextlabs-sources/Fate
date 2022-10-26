
#include <ntifs.h>
#include <wdm.h>

#include "nl_crypto.h"

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

static int stub_rand( __in void* in_obj )
{
  ULONG* ul = (ULONG*)in_obj;
  ULONG seed = 0;

  ASSERT( in_obj != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_obj == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  *ul = RtlRandomEx(&seed); /* random ULONG w/o presisted key */

  return NL_CRYPTO_ERROR_SUCCESS;
}/* stub_create_key */

static int stub_encrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_key_size) const unsigned char* in_key ,
			 __in size_t in_key_size ,
			 __in const unsigned char* in_ivec ,
			 __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
			 __in size_t in_buf_size )
{
  return NL_CRYPTO_ERROR_SUCCESS;
}/* stub_encrypt */

static int stub_decrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_key_size) const unsigned char* in_key ,
			 __in size_t in_key_size ,
			 __in const unsigned char* in_ivec ,
			 __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
			 __in size_t in_buf_size )
{
  return NL_CRYPTO_ERROR_SUCCESS;
}/* stub_decrypt */

int nl_crypto_initialize( __out struct nl_crypto_context* in_ctx ,
			  __in int in_options )
{
  ASSERT( in_ctx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  in_ctx->rand       = stub_rand;
  in_ctx->encrypt    = stub_encrypt;
  in_ctx->decrypt    = stub_decrypt;

  in_ctx->block_size = 1;            /* block size must be at least one byte */

  in_ctx->module_context = NULL;     /* no context */

  return NL_CRYPTO_ERROR_SUCCESS;
}/* nl_crypto_initialize */

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
