#include "..\include\Encrypt.h"

#include <shlwapi.h>
#include "celog.h"
#pragma warning(push)
#pragma warning(disable:6334 6011 4996 4189 4100 4819)
#include "boost\format.hpp"
#pragma warning(pop)

#include "nlconfig.hpp"

#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 
#define CELOG_CUR_MODULE L"ADOBEPEP"
#define CELOG_CUR_FILE CELOG_FILEPATH_PROD_PEP_ENDPOINT_ADOBEPEP_FORCEADOBEPEP_SRC_ENCRYPT_CPP


typedef BOOL (*fnSE_IsEncrypted)( _In_ const wchar_t* path );
typedef BOOL (*fnSE_EncryptFile)( _In_ const wchar_t* path );

typedef BOOL (*fnSE_MarkFileAsDRMOneShot)( _In_ const wchar_t* path );

std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _M_IX86
		wcscat_s(szDir, MAX_PATH, L"\\bin32");
#else
		wcscat_s(szDir, MAX_PATH, L"\\bin64");
#endif
		return szDir;
	}
    CELOG_LOG(CELOG_DUMP, L"Local variables are: szDir=%ls \n", print_long_string(szDir) );

	return L"";
}

//is this file path a local file path or UNC file path
//false means not local path, like UNC path
#define MAX_PATHLEN 256
#pragma comment(lib, "mpr.lib")
bool IsLocalPath(LPCWSTR pszFileName)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: pszFileName=%ls \n",print_long_string(pszFileName) );

	if (pszFileName == NULL)
	{
		return false;
	}

	bool bRet = false;
	if (wcslen(pszFileName) >= 3 && pszFileName[1] == ':')
	{
		wchar_t drive[3] = {0};
		wcsncpy_s(drive, 3, pszFileName, 2);

		if (GetDriveTypeW(drive) == DRIVE_FIXED)
		{
			bRet = true;
		}
	}

	CELOG_LOG(CELOG_DUMP, L"Local variables are: bRet=%ls \n", bRet?L"TRUE":L"FALSE" );

	return bRet;
};


#ifdef _WIN64
static const wchar_t* g_szSeLibName = L"\\nl_sysenc_lib.dll";
#else
static const wchar_t* g_szSeLibName = L"\\nl_sysenc_lib32.dll";
#endif

static const char* g_szIsEncryptFuncName = "SE_IsEncrypted";
static const char* g_szEncryptFuncName = "SE_EncryptFile";
static const char* g_szSaveAsEncryptFuncName = "SE_MarkFileAsDRMOneShot";


CEncrypt::CEncrypt(void)
{
}

CEncrypt::~CEncrypt(void)
{
}


EncryptRetValueType CEncrypt::Encrypt(const wstring& strPath, bool bMarkBeforeCreate, bool bQueryOnly)
{
	CELOG_LOG(CELOG_DUMP, L" The Parameters are: strPath=%ls, bMarkBeforeCreate=%ls, bQueryOnly=%ls \n",strPath.c_str(),bMarkBeforeCreate?L"TRUE":L"FALSE",bQueryOnly?L"TRUE":L"FALSE" );

	// here ,we only care the local file
	if(!IsLocalPath(strPath.c_str()))	
	{
		return emEncryptError;
	}

	static fnSE_IsEncrypted gSE_IsEncrypted=NULL;
	static fnSE_EncryptFile gSE_EncryptFile=NULL;
	static fnSE_MarkFileAsDRMOneShot gSe_MarkEncryptedFile=NULL;

	CELOG_LOG(CELOG_DEBUG, L"try to encrypt file: %ls\n", strPath.c_str());
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
