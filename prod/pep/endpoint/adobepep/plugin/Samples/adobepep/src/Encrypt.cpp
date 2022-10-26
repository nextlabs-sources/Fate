#include "..\include\Encrypt.h"
#include "utilities.h"
#include <shlwapi.h>
#include "celog.h"
#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\format.hpp"
#pragma warning(pop)

typedef BOOL (*fnSE_IsEncrypted)( _In_ const wchar_t* path );
typedef BOOL (*fnSE_EncryptFile)( _In_ const wchar_t* path );

typedef BOOL (*fnSE_MarkFileAsDRMOneShot)( _In_ const wchar_t* path );

#ifdef _WIN64
static const wchar_t* g_szSeLibName = L"\\nl_sysenc_lib.dll";
#else
static const wchar_t* g_szSeLibName = L"\\nl_sysenc_lib32.dll";
#endif

#define CELOG_CUR_MODULE L"ADOBEPEP"
#undef CELOG_CUR_FILE
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_PLUGIN_SAMPLESADOBEPEP_SRC_ENCRYPT_CPP

static const char* g_szIsEncryptFuncName = "SE_IsEncrypted";
static const char* g_szEncryptFuncName = "SE_EncryptFile";
static const char* g_szSaveAsEncryptFuncName = "SE_MarkFileAsDRMOneShot";


extern BOOL bReaderXProtectedMode;

CEncrypt::CEncrypt(void)
{
}

CEncrypt::~CEncrypt(void)
{
}


EncryptRetValueType CEncrypt::Encrypt(const wstring& strPath, bool bMarkBeforeCreate, bool bQueryOnly)
{
	// here ,we only care the local file
	if(!IsLocalPath(strPath.c_str()) ||  bReaderXProtectedMode)	
	{
		CELOG_LOG(CELOG_DEBUG, L"Ignore calling CEncrypt::Encrypt(), path: %s, protected mode: %d", strPath.c_str(), bReaderXProtectedMode);
		return emEncryptError;
	}

	static fnSE_IsEncrypted gSE_IsEncrypted=NULL;
	static fnSE_EncryptFile gSE_EncryptFile=NULL;
	static fnSE_MarkFileAsDRMOneShot gSe_MarkEncryptedFile=NULL;

	CELOG_LOG(CELOG_DEBUG, L"try to encrypt file: %s\n", strPath.c_str());
	if(gSE_EncryptFile == NULL)
	{
		std::wstring strCommonPath = GetCommonComponentsDir() ;
		strCommonPath += g_szSeLibName;
		HMODULE hMod = LoadLibrary(strCommonPath.c_str());

		if(hMod!= NULL)
		{
			gSE_IsEncrypted = (fnSE_IsEncrypted)GetProcAddress(hMod,g_szIsEncryptFuncName);
			gSE_EncryptFile = (fnSE_EncryptFile)GetProcAddress(hMod,g_szEncryptFuncName);
			gSe_MarkEncryptedFile = (fnSE_MarkFileAsDRMOneShot)GetProcAddress(hMod,g_szSaveAsEncryptFuncName);
		}

		if (gSE_EncryptFile == NULL ||
			gSE_IsEncrypted == NULL || 
			gSe_MarkEncryptedFile == NULL )
		{
			CELOG_LOG(CELOG_DEBUG, L"CEncrypt::Encrypt fail to load function\n");
			return emEncryptError;
		}
		else
		{
			CELOG_LOG(CELOG_DEBUG, L"CEncrypt::Encrypt succeed to load function\n");
		}
	}
	
	if (true==bQueryOnly)
	{
		BOOL bEncrypted=gSE_IsEncrypted(strPath.c_str());
		
		CELOG_LOG(CELOG_DEBUG, L"file [%s] is encrypted: %d\n", strPath.c_str(), bEncrypted);
		return bEncrypted? emIsEncrytFile : emNotEncrytFile;
	}
	

	if(true==bMarkBeforeCreate)
	{
		//不是直接加密
		//先删除，再标记
		if(PathFileExists(strPath.c_str()))
		{
			// delete it first
			CELOG_LOG(CELOG_DEBUG, L"CEncrypt::Encrypt to delete the file first\n");
			if(!::DeleteFileW(strPath.c_str()))
			{
				//删除不掉，就不可能在生成之前标记为加密状态了，直接返回错误
				CELOG_LOG(CELOG_DEBUG, L"CEncrypt::Encrypt delete the file fail\n");
				return emEncryptError;
			}
		}

		CELOG_LOG(CELOG_DEBUG, L"try to mark encryption for file: %s\n", strPath.c_str());

		BOOL BRet = gSe_MarkEncryptedFile(strPath.c_str());

		CELOG_LOG(CELOG_DEBUG, L"Mark result: %d\n", BRet);

		if (BRet==TRUE)
		{
			//标记成功
			
			return emEncryptSuccess;
		}
		else
		{
			//标记失败
			
			return emEncryptError;
		}
	}
	else
	{
		
		if(!gSE_IsEncrypted(strPath.c_str()))
		{
			//没有加密，那就加密
			if(!gSE_EncryptFile(strPath.c_str()))
			{

				CELOG_LOG(CELOG_DEBUG, L"fail to encrypt file [%s]\n", strPath.c_str());
				return emEncryptError;
			}
		}
		
		CELOG_LOG(CELOG_DEBUG, L"succeed to encrypt file [%s]\n", strPath.c_str());

		return emEncryptSuccess;
	}
}
