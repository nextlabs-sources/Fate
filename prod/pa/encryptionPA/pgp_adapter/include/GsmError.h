/*
Written by Derek Zheng
March 2008
*/

#ifndef _GSM_ERROR_H_
#define _GSM_ERROR_H_

typedef enum
{
	GSM_ERR_SOURCE_UNKNOWN = 0,
	GSM_ERR_SOURCE_PGP = 1,
	GSM_ERR_SOURCE_GPG = 2,
	GSM_ERR_SOURCE_VOLTAGE = 3,
	GSM_ERR_SOURCE_MS_CRYPTO = 4,
	GSM_ERR_SOURCE_JAVA_CRYPTO = 5,

	GSM_ERR_SOURCE_DIM = 256
} GsmErrorSource;

typedef enum
{
	GSM_ERR_NO_ERROR = 0,
	GSM_ERR_GENERAL = 1,
	GSM_ERR_NO_DLL = 2,
	GSM_ERR_NO_RESOURCE = 3,
	GSM_ERR_PUBKEY_ALGO = 4,
	GSM_ERR_DIGEST_ALGO = 5,
	GSM_ERR_BAD_PUBKEY = 6,
	GSM_ERR_BAD_SECKEY = 7,
	GSM_ERR_BAD_SIGNATURE = 8,
	GSM_ERR_NO_PUBKEY = 9,
	GSM_ERR_CHECKSUM = 10,
	GSM_ERR_BAD_PASSPHRASE = 11,
	GSM_ERR_CIPHER_ALGO = 12,
	GSM_ERR_KEYRING_OPEN = 13,
	GSM_ERR_INV_ATTR = 14,
	GSM_ERR_NO_VALUE = 15,
	GSM_ERR_NO_USER_ID = 16,
	GSM_ERR_NO_SECKEY = 17,
	GSM_ERR_WRONG_SECKEY = 18,
	GSM_ERR_INV_ENGINE = 19,
	//GSM_ERR_FILE_NOT_FOUND = 20,
	GSM_ERR_NOT_FOUND = 21,
	GSM_ERR_SYNTAX = 22,
	GSM_ERR_TRUSTDB = 23,
	GSM_ERR_BAD_CERT = 24,
	GSM_ERR_KEYSERVER = 25,
	GSM_ERR_FILE_OPEN = 26,
	GSM_ERR_SYSTEM	= 27,
	GSM_ERR_NETWORK = 28,
	GSM_ERR_NOT_ENCRYPTED = 29,
	GSM_ERR_NOT_SUPPORTED = 30,
	GSM_ERR_NOT_IMPLEMENTED = 31,
	GSM_ERR_INV_HANDLE = 32,
	GSM_ERR_INV_PARAMETER = 33,
	GSM_ERR_CANCELED = 34,
	GSM_ERR_TIMEOUT = 35,
	GSM_ERR_DECRYPT_FAILED = 36,
	GSM_ERR_CREATE_PATH = 37,
	GSM_ERR_SDK_UNINTIALISED = 38,
	GSM_ERR_SDK_BAD_CONTENT = 39,
	GSM_ERR_GPG_NOT_FOUND = 40,
	GSM_ERR_NULL_PTR = 41,
	GSM_ERR_FILE_EXISTS = 42,
	GSM_ERR_FILE_NOT_FOUND = 43,
	
	GSM_ERR_ZIP_GENERAL = 60,
	GSM_ERR_ZIP_UNSUPPORTEDFORMAT	= 61,

	GSM_ERR_UNZIP_GENERAL	= 80,
	GSM_ERR_UNZIP_UNSUPPORTEDFORMAT	= 81,
	GSM_ERR_UNZIP_DATA		= 82,
	GSM_ERR_UNZIP_CRC		= 83,


	GSM_ERR_CODE_DIM = 65536
} GsmErrorCode;

typedef unsigned int GsmErrorT;

#define GSM_ERR_CODE_MASK	(GSM_ERR_CODE_DIM - 1)

/* Bits 17 to 24 are reserved.  */

/* We use the upper 8 bits of GsmError for error sources.  */
#define GSM_ERR_SOURCE_MASK	(GSM_ERR_SOURCE_DIM - 1)
#define GSM_ERR_SOURCE_SHIFT	24

static inline GsmErrorT
GsmErrorMakeBySource (GsmErrorSource source, GsmErrorCode code)
{
	return code == GSM_ERR_NO_ERROR ? GSM_ERR_NO_ERROR
		: (((source & GSM_ERR_SOURCE_MASK) << GSM_ERR_SOURCE_SHIFT)
		| (code & GSM_ERR_CODE_MASK));
}


/* The user should define GSM_ERR_SOURCE_DEFAULT before including this
file to specify a default source for gpg_error.  */
#ifndef GSM_ERR_SOURCE_DEFAULT
#define GSM_ERR_SOURCE_DEFAULT	GSM_ERR_SOURCE_UNKNOWN
#endif

static inline GsmErrorT
GsmErrorMake (GsmErrorCode code)
{
	return GsmErrorMakeBySource (GSM_ERR_SOURCE_DEFAULT, code);
}

#endif // _GSM_ERROR_H_