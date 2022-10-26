/*
Defines the basic type for the encryption adapter,and the standard interface.
The Encryption Assistant will use this interface to encrypt file with 
3rd-party encryption SDK.
*/

#ifndef __ADAPTER_BASE_H
#define __ADAPTER_BASE_H

#ifdef ADAPTER_BASE_EXPORTS
#define ADAPTER_BASE_API __declspec(dllexport)
#else
#define ADAPTER_BASE_API __declspec(dllimport)
#endif

#include <string>
#include <vector>
#include <wchar.h>

/*
 * Define the error code enum that should be returned from encryption adapter
 */
typedef enum
{
	EA_OK	= 0,
	EA_E_CANCELED = -1, 

	EA_E_UNINITIALISED	= -2,
	EA_E_INIT			= -3,
	EA_E_UNSUPPORTED	= -4,
	EA_E_NORESOURCE		= -5,
	EA_E_NULLPTR		= -6,
	EA_E_BADPARAM		= -7,
	EA_E_BADEMAIL		= -8,

	EA_E_BADPASS		= -20,
	EA_E_NOPUBKEY		= -21,
	EA_E_NOSECKEY		= -22,
	EA_E_KEYUNAUTHORIZED= -23, 
	EA_E_ENCRYPT		= -24,
	EA_E_DECRYPT		= -25,

	EA_E_WRONGPATH		= -50,
	EA_E_FILEEXISTED		= -51,
	EA_E_FILENOPERMISSION	= -52,

	EA_E_GENERAL		= -100
} EA_Error;

typedef std::vector<std::wstring> StringVector;


typedef struct _EncryptionContext
{
	/*[in]bSymm=True, do symmetric encryption; otherwise do asymmetric encryption. */
	BOOL			bSymm;
	/*[in]The sender or encrypter's email or user name string. For asymmetric encryption.*/
	std::wstring	wstrEncrypterInfo;
	/*[in]The recipients or decrypters' email or user name strings. For asymmetric encryption.*/
	StringVector	vecDecrypterInfos;
	/*[in]The password supplied to do symmetric encryption. */
	std::wstring	wstrPassword;

	_EncryptionContext()
	{
		bSymm = TRUE;

		wstrEncrypterInfo = L"";
		wstrPassword = L"";
	}
} EncryptionContext, *LPEncryptionContext;


/*
* Define the encryption adapter parameter structure that should be passed into adapter.
*/
typedef struct _EncryptionAdapterData
{
	/* [in]The source file that should be encrypted */
	std::wstring	wstrSrcFile;
	/* [in]The destination base file name after be encrypted.
	   (Not include the extend and the folder path) */
	std::wstring	wstrBaseFileName;
	/* [in]The destination folder path that the encrypted file should be put */
	std::wstring	wstrDstFolder;

	/* Contain encryption credential information */
	EncryptionContext encryptContext;

	/* [out]The the actual encrypted file path returned, constructed by 
	   wstrDstFolder + wstrBaseFileName + .extend */
	std::wstring	wstrActualDstFile;
	/* [out]When Encrypt return error, it contains
	   error messages. */
	std::wstring	wstrErrorInfo;
	/* [out]The encryption type. For example, ZIPCrypto, Voltage AES128CBC, etc. */
	std::wstring	wstrEncryptionType;
	_EncryptionAdapterData()
	{
		wstrSrcFile = L"";
		wstrBaseFileName = L"";
		wstrDstFolder = L"";
		
		wstrActualDstFile = L"";
		wstrErrorInfo = L"";
		wstrEncryptionType = L"";
	}
} EncryptionAdapterData, *LPEncryptionAdapterData;

/* 
 * The adapter function to do encryption/decryption with 3rd-party encryption SDK.
 * It must be exported by each encryption adapter.
 */
ADAPTER_BASE_API EA_Error WINAPI Encrypt(EncryptionAdapterData *lpData);

#endif //__ADAPTER_BASE_H