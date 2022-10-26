
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

static int fake_rand( __inout int* in_rx )
{
  ULONG seed = 0;

  ASSERT( in_rx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_rx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  *in_rx = (int)RtlRandomEx(&seed); /* random ULONG w/o presisted key */

  return NL_CRYPTO_ERROR_SUCCESS;
}/* fake_create_key */

static int fake_encrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_key_size) const unsigned char* in_key ,
			 __in size_t in_key_size ,
			 __in const unsigned char* in_ivec ,
			 __out unsigned char* in_buf ,
			 __in size_t in_buf_size )
{
  size_t i = 0;

  ASSERT( in_ctx != NULL );
  ASSERT( in_key != NULL );
  ASSERT( in_buf != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  for( i = 0 ; i < in_buf_size ; i++ )
  {
    *in_buf ^= in_key[(i % in_key_size)];
    in_buf++;
  }
  return NL_CRYPTO_ERROR_SUCCESS;
}/* fake_encrypt */

static int fake_decrypt( __in struct nl_crypto_context* in_ctx ,
			 __in_bcount(in_key_size) const unsigned char* in_key ,
			 __in size_t in_key_size ,
			 __in const unsigned char* in_ivec ,
			 __out_bcount_full(in_buf_size) unsigned char* in_buf ,
			 __in size_t in_buf_size )
{
  size_t i = 0;

  ASSERT( in_ctx != NULL );
  ASSERT( in_key != NULL );
  ASSERT( in_buf != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  for( i = 0 ; i < in_buf_size ; i++ )
  {
    *in_buf ^= in_key[(i % in_key_size)];
    in_buf++;
  }
  return NL_CRYPTO_ERROR_SUCCESS;
}/* fake_decrypt */

int nl_crypto_initialize( __out struct nl_crypto_context* in_ctx ,
			  __in int in_options )
{
  ASSERT( in_ctx != NULL );
  ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

  if( in_ctx == NULL )
  {
    return NL_CRYPTO_ERROR_INVALID_ARGUMENT;
  }

  in_ctx->rand       = fake_rand;
  in_ctx->encrypt    = fake_encrypt;
  in_ctx->decrypt    = fake_decrypt;

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
