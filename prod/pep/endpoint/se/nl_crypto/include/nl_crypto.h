/*******************************************************************************
 *
 * NextLabs Crypto Interface
 *
 * Abstract implementation of symmetric encryption.
 *
 ******************************************************************************/

#ifndef __NL_CRYPTO_H__
#define __NL_CRYPTO_H__

#ifdef NTDDI_VERSION
#  include <wdm.h>
#endif

/* Errors */
enum
{
  NL_CRYPTO_ERROR_SUCCESS,           /* success */
  NL_CRYPTO_ERROR_GENERAL_FAILURE,   /* standard faulure */
  NL_CRYPTO_ERROR_INVALID_ARGUMENT,  /* invalid parameter */
  NL_CRYPTO_ERROR_NOT_SUPPORTED      /* unsupported option/method/function */
};

/* Context */
struct nl_crypto_context
{

  /** rand
   *
   *  \brief Create a random number which is implementation dependent.  Interpretation
   *         of the object is implementation defined.  For example, this may be an
   *         integer.
   *
   *  \param rx (out) Random generated integer.
   *
   *  \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise NL_CRYPTO_ERROR_XXX value.
   *
   *  \notes This is optionally implemented by the encryption module.
   */
  __drv_requiresIRQL(PASSIVE_LEVEL)
  __checkReturn
  int (*rand)( __in struct nl_crypto_context* in_ctx ,
			   __out int* in_rx );

  /** init_crypto_key
   *
   *  \brief Initializes the opaque crypto key for encryption or decryption based on a raw key.
   *
   *  \param in_ctx (in)      Context.
   *  \param in_key (in)      Key.  This parameter must not be NULL.
   *  \param in_key_size (in) Key size in bytes.
   *  \param in_is_encrypt (in) TRUE if encrypt, FALSE if decrypt.
   *  \param out_crypto_key (out) Buffer for opaque crypto key structure.  The size of
   *                          this buffer is in_ctx->crypto_key_size.
   *
   *  \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise NL_CRYPTO_ERROR_XXX value.
   */
  __drv_maxIRQL(DISPATCH_LEVEL)
  __checkReturn
  int (*init_crypto_key)( __in struct nl_crypto_context* in_ctx ,
                          __in_bcount(in_key_size) const unsigned char* in_key ,
                          __in size_t in_key_size ,
                          __in BOOLEAN in_is_encrypt ,
                          __out_bcount_full(in_ctx->crypto_key_size) void *out_crypto_key );

  /** destroy_crypto_key
   *
   *  \brief Securely destroys the crypto key.
   *
   *  \param in_ctx (in)      Context.
   *  \param in_crypto_key (in-out) Buffer for opaque crypto key structure.  The size of
   *                          this buffer is in_ctx->crypto_key_size.
   *
   *  \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise NL_CRYPTO_ERROR_XXX value.
   */
  __drv_maxIRQL(DISPATCH_LEVEL)
  __checkReturn
  int (*destroy_crypto_key)( __in struct nl_crypto_context* in_ctx ,
                             __inout_bcount(in_ctx->crypto_key_size) void *in_crypto_key );

  /** encrypt
   *
   *  \brief Encrypt a user provided buffer.
   *
   *  \param in_ctx (in)      Context.
   *  \param in_crypto_key (in) Crypto key.  This parameter must not be NULL.
   *  \param in_ivec (in)     Initial vector.
   *  \param in_buf (in-out)  Buffer to encrypt.  This parameter must not be NULL.
   *  \param in_buf_size (in) Size of buffer to encrypt in bytes.
   *
   *  \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise NL_CRYPTO_ERROR_XXX value.
   *
   *  \notes The size of the buffer must be in units of the block size required by the
   *         implementation.
   */
  __drv_maxIRQL(DISPATCH_LEVEL)
  __checkReturn
  int (*encrypt)( __in struct nl_crypto_context* in_ctx ,
		  __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
		  __in const unsigned char* in_ivec ,
		  __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
		  __in size_t in_buf_size );

  /** decrypt
   *
   *  \brief Decrypt a user provided buffer.
   *
   *  \param in_ctx (in)      Context.
   *  \param in_crypto_key (in) Crypto key.  This parameter must not be NULL.
   *  \param in_ivec (in)     Initial vector.
   *  \param in_buf (in-out)  Buffer to encrypt.  This parameter must not be NULL.
   *  \param in_buf_size (in) Size of buffer to decrypt in bytes.
   *
   *  \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise NL_CRYPTO_ERROR_XXX value.
   *
   *  \notes The size of the buffer must be in units of the block size required by the
   *         implementation.
   */
  __drv_maxIRQL(DISPATCH_LEVEL)
  __checkReturn
  int (*decrypt)( __in struct nl_crypto_context* in_ctx ,
		  __in_bcount(in_ctx->crypto_key_size) const void *in_crypto_key ,
		  __in const unsigned char* in_ivec ,
		  __inout_bcount_full(in_buf_size) unsigned char* in_buf ,
		  __in size_t in_buf_size );

  /* Size of opaque Crypto key structure that this implementation uses */
  size_t crypto_key_size;

  /* Block size in bytes */
  int block_size;

  /* Implementation defined context */
  void* module_context;

};/* nl_crypto_context */

/** nl_crypto_initialize
 *
 * \param in_context  (out) Context for instance.
 * \param in_options  (in)  Implementation defined options.
 *
 * \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise a NL_CRYPTO_ERROR_XXX value.
 */
__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
int nl_crypto_initialize( __out struct nl_crypto_context* in_context ,
			  __in int in_options );

/** nl_crypto_shutdown
 *
 * \brief Shutdown crypto interface.
 *
 * \param in_context (in) Context for instance.
 *
 * \return NL_CRYPTO_ERROR_SUCCESS on success, otherwise a NL_CRYPTO_ERROR_XXX value.
 */
__drv_requiresIRQL(PASSIVE_LEVEL)
__checkReturn
int nl_crypto_shutdown( __in struct nl_crypto_context* in_context );

#endif /* __NL_CRYPTO_H__ */
