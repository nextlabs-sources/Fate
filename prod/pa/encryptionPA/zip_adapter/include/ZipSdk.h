/*
Written by Derek Zheng
June 2008
*/

#ifndef __ZIP_SDK_H
#define __ZIP_SDK_H

// #include "GsmType.h"
#include "GsmError.h"

#include <vector>
#include <xstring>

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 Int64;
typedef unsigned __int64 UInt64;
#else
typedef long long int Int64;
typedef unsigned long long int UInt64;
#endif

typedef std::vector<std::wstring> StringVector;

typedef HRESULT (WINAPI *funcZipPasswordCallback)(LPVOID context, BSTR *password);
typedef HRESULT (WINAPI *funcZipProcessCallback)(LPVOID context, UInt64 uiCompleteValue, UInt64 uiTotalSize);

typedef enum 
{
	ZIP_COMPRESS_FORMAT_UNKNOWN = 0,
	ZIP_COMPRESS_FORMAT_ZIP = 1,
	ZIP_COMPRESS_FORMAT_BZIP2 = 2,
	ZIP_COMPRESS_FORMAT_RAR = 3,
	ZIP_COMPRESS_FORMAT_ARJ = 4,
	ZIP_COMPRESS_FORMAT_Z = 5,
	ZIP_COMPRESS_FORMAT_LZH = 6,
	ZIP_COMPRESS_FORMAT_7Z = 7,
	ZIP_COMPRESS_FORMAT_CAB = 8,
	ZIP_COMPRESS_FORMAT_NISS = 0x9,
	ZIP_COMPRESS_FORMAT_LZMA = 0xA,
	ZIP_COMPRESS_FORMAT_COM = 0xE5,
	ZIP_COMPRESS_FORMAT_WIM = 0xE6,
	ZIP_COMPRESS_FORMAT_ISO = 0xE7,
	ZIP_COMPRESS_FORMAT_CHM = 0xE9,
	ZIP_COMPRESS_FORMAT_SPLIT = 0xEA,
	ZIP_COMPRESS_FORMAT_RPM = 0xEB,
	ZIP_COMPRESS_FORMAT_DEB = 0xEC,
	ZIP_COMPRESS_FORMAT_CPIO = 0xED,
	ZIP_COMPRESS_FORMAT_TAR = 0xEE,
	ZIP_COMPRESS_FORMAT_GZIP = 0xEF
} ZipCompressFormat;

typedef enum
{
	ZIP_COMPRESS_LEVEL_STORE = 0,
	ZIP_COMPRESS_LEVEL_FASTEST = 1,
	ZIP_COMPRESS_LEVEL_FAST = 3,
	ZIP_COMPRESS_LEVEL_NORMAL = 5,
	ZIP_COMPRESS_LEVEL_MAXIMUM = 7,
	ZIP_COMPRESS_LEVEL_ULTRA = 9
} ZipCompressLevel;

typedef enum
{
	ZIP_CIPHER_ALGO_ZIPCRYPTO = 0,
	ZIP_CIPHER_ALGO_AES128 = 1,
	ZIP_CIPHER_ALGO_AES192 = 2,
	ZIP_CIPHER_ALGO_AES256 = 3,

	ZIP_CIPHER_ALGO_MAX
} ZipCipherAlgorithm;

typedef enum
{
	ZIP_OVERWRITE_MODE_AskBefore,
	ZIP_OVERWRITE_MODE_WithoutPrompt,
	ZIP_OVERWRITE_MODE_SkipExisting,
	ZIP_OVERWRITE_MODE_AutoRename,
	ZIP_OVERWRITE_MODE_AutoRenameExisting
} ZipOverwriteMode;

typedef enum
{
	ZIP_OVERWRITE_ANSWER_Yes,
	ZIP_OVERWRITE_ANSWER_YesToAll,
	ZIP_OVERWRITE_ANSWER_No,
	ZIP_OVERWRITE_ANSWER_NoToAll,
	ZIP_OVERWRITE_ANSWER_AutoRename,
	ZIP_OVERWRITE_ANSWER_Cancel
} ZipOverwriteAnswer;

typedef HRESULT (WINAPI *funcZipOverWirtePromptCallback)(LPVOID context, ZipOverwriteAnswer *overwriteAnswer);

typedef struct tagZipCompressParam
{
	StringVector vecFiles;
	std::wstring wstrArchive;
	std::wstring wstrComments;	// Just for .zip
	ZipCompressFormat eFormat;	// Just for .zip
	ZipCompressLevel eLevel;	// Just for .zip
	BOOL bEncrypt;				// Just for .zip & .7z
	ZipCipherAlgorithm eAlgo;	// Just for .zip
	LPWSTR wzPassword;
	funcZipPasswordCallback fZipPasswordCb;
	LPVOID lpPasswdContxt;
	funcZipProcessCallback fZipProcessCallback;
	LPVOID lpProcessContxt;
} ZipCompressParam, *LPZipCompressParam;

typedef struct tagZipUnCompressParam
{
	std::wstring wstrArchive;
	std::wstring wstrDstFolder;
	ZipCompressFormat eFormat;
	ZipOverwriteMode eOverwriteMode;
	std::wstring wstrComments;
	std::wstring wstrPassword;
	funcZipPasswordCallback fZipPasswordCb;
	LPVOID lpPasswdContxt;
	funcZipOverWirtePromptCallback fZipOverWirtePromptCallback;
	LPVOID lpOverWirtePromptContext;
	funcZipProcessCallback fZipProcessCallback;
	LPVOID lpProcessContxt;
} ZipUnCompressParam, *LPZipUnCompressParam;

class CZipSdk
{
public:
	CZipSdk() {}
	~CZipSdk() { Release(); }

	virtual GsmErrorT Init( void );
	virtual void Release( void );

	virtual GsmErrorT CompressFile(ZipCompressParam &param);
	virtual GsmErrorT UnCompressFile(ZipUnCompressParam &param);

	virtual GsmErrorT ReadComments(std::wstring &wstrArchive, std::wstring &wstrComments);
	virtual GsmErrorT WriteComments(std::wstring &wstrArchive, std::wstring &wstrComments);

private:

};

#if 0
GSMSDK_API CZipSdk * CreateZipSdkInstance( void );
GSMSDK_API void DestroyZipSdkInstance( CZipSdk *pZipSdk );
#endif

#endif