// pgp_adapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "pgp_adapter.h"
#include "log.h"

#include "GsmSdk.h"
#include "PgpSdkAdapter.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define PGP_SDK_DLL			L"PGPsdk.dll"
#define PGP_SDK_NL_DLL		L"PGPsdkNL.dll"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance = NULL;
WCHAR g_wzPGPAdapterPath[MAX_PATH];
static CRITICAL_SECTION s_criticalSection;

extern HINSTANCE g_hPGPSdkDll;
extern HINSTANCE g_hPGPSdkNLDll;

static int ValidateEmailAddress(LPEncryptionAdapterData lpData);
static LPWSTR GetBaseFileName(LPWSTR lpwzFileName);

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
 
BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            DPW((L"Call IsWow64Process function error!(error=0x%x)\n", GetLastError()));
        }
    }
    return bIsWow64;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	UNUSED(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			WCHAR wzFullDLLPath[MAX_PATH];
			WCHAR wzCurrentPath[MAX_PATH];
            WCHAR wzInstallPath[MAX_PATH] = {0};

			g_hInstance = (HINSTANCE)hModule;
			GetModuleFileNameW(hModule, g_wzPGPAdapterPath, MAX_PATH);
			GetModuleFileNameW(hModule, wzCurrentPath, MAX_PATH);
			(wcsrchr(wzCurrentPath, L'\\'))[1] = 0;

            if(IsWow64())
            {
                GetSystemWow64DirectoryW(wzInstallPath, MAX_PATH);
            }
            else
            {
                GetSystemDirectoryW(wzInstallPath, MAX_PATH);
            }
			if (!g_hPGPSdkDll)
			{
				_snwprintf_s(wzFullDLLPath, MAX_PATH, _TRUNCATE, L"%s\\%s", wzInstallPath, PGP_SDK_DLL);
				g_hPGPSdkDll = LoadLibraryW(wzFullDLLPath);
				if (!g_hPGPSdkDll)
				{
					DPW((L"Load library %s failed!(error=0x%x), next try to load dll in my module.\n", wzFullDLLPath, GetLastError()));
                    _snwprintf_s(wzFullDLLPath, MAX_PATH, _TRUNCATE, L"%s\\%s", wzCurrentPath, PGP_SDK_DLL);                   
                    g_hPGPSdkDll = LoadLibraryW(wzFullDLLPath);
                    if (!g_hPGPSdkDll)
                    {
                        DPW((L"Load library %s failed!(error=0x%x)\n", wzFullDLLPath, GetLastError()));
                        return FALSE;
                    }
				}
			}

			if (!g_hPGPSdkNLDll)
			{
				_snwprintf_s(wzFullDLLPath, MAX_PATH, _TRUNCATE, L"%s\\%s", wzInstallPath, PGP_SDK_NL_DLL);
				g_hPGPSdkNLDll = LoadLibraryW(wzFullDLLPath);
				if (!g_hPGPSdkNLDll)
				{
                    DPW((L"Load library %s failed!(error=0x%x), next try to load dll in my module.\n", wzFullDLLPath, GetLastError()));
                    _snwprintf_s(wzFullDLLPath, MAX_PATH, _TRUNCATE, L"%s\\%s", wzCurrentPath, PGP_SDK_NL_DLL);
                    g_hPGPSdkNLDll = LoadLibraryW(wzFullDLLPath);
                    if (!g_hPGPSdkNLDll)
                    {
					    FreeLibrary(g_hPGPSdkDll);
					    g_hPGPSdkDll = NULL;

					    DPW((L"Load library %s failed!(error=0x%x)\n", wzFullDLLPath, GetLastError()));
					    return FALSE;
                    }
				}
			}

			InitializeCriticalSection(&s_criticalSection);
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			g_hInstance = NULL;
			DeleteCriticalSection(&s_criticalSection);

			if (g_hPGPSdkDll)
			{
				FreeLibrary(g_hPGPSdkDll);
				g_hPGPSdkDll = NULL;
			}

			if (g_hPGPSdkNLDll)
			{
				FreeLibrary(g_hPGPSdkNLDll);
				g_hPGPSdkNLDll = NULL;
			}
		}
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

ADAPTER_BASE_API EA_Error WINAPI Encrypt(EncryptionAdapterData *lpData)
{
	CGsmSdk *pGsmSdk = NULL;
	struct _stat stat_buffer;
	WCHAR wzDstFileName[MAX_PATH];
	WCHAR wzStringResource[512];

	if (!lpData)
	{
		return EA_E_NULLPTR;
	}

	if (lpData->wstrSrcFile.empty() || lpData->wstrBaseFileName.empty())
	{
		LoadStringW(g_hInstance, IDS_STR_NOSRC, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPARAM;
	}

	lpData->wstrActualDstFile = L"";

	if (lpData->encryptContext.bSymm && lpData->encryptContext.wstrPassword.empty())
	{
		DPW((L"EncryptionAdapter: password should not be empty for symmetric encryption!\n"));
		LoadStringW(g_hInstance, IDS_STR_NOPASSWD, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPASS;
	}

	if (!lpData->encryptContext.bSymm && lpData->encryptContext.wstrEncrypterInfo.empty())
	{
		DPW((L"EncryptionAdapter: sender's email should not be empty for asymmetric encryption!\n"));
		LoadStringW(g_hInstance, IDS_STR_NO_ENCRYPTER, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPARAM;
	}

	if (!lpData->encryptContext.bSymm && ValidateEmailAddress(lpData) != 0)
	{
		DPW((L"EncryptionAdapter: Contain invalid email address!\n"));
		LoadStringW(g_hInstance, IDS_STR_WRONG_EMAIL, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADEMAIL;
	}

	if (_wstat(lpData->wstrSrcFile.c_str(), &stat_buffer) != 0)
	{
		WCHAR wzError[1024];

		DPW((L"EncryptionAdapter: source file: %s does not exist!\n", lpData->wstrSrcFile));

		LoadStringW(g_hInstance, STR_SRC_NOTEXIST, wzStringResource, 512);
		_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, lpData->wstrSrcFile.c_str());
		lpData->wstrErrorInfo = wzError;

		return EA_E_WRONGPATH;
	}

	if (lpData->wstrDstFolder.c_str()[lpData->wstrDstFolder.size()] == '\\' || 
		lpData->wstrDstFolder.c_str()[lpData->wstrDstFolder.size()] == '/')
	{
		_snwprintf_s(wzDstFileName, MAX_PATH, _TRUNCATE, L"%s%s.pgp", lpData->wstrDstFolder.c_str(), lpData->wstrBaseFileName.c_str());
	}
	else
	{
		_snwprintf_s(wzDstFileName, MAX_PATH, _TRUNCATE, L"%s\\%s.pgp", lpData->wstrDstFolder.c_str(), lpData->wstrBaseFileName.c_str());
	}

	GsmErrorT err = GSM_ERR_NO_ERROR;
	pGsmSdk = new CPgpSdkAdapter();
	if (pGsmSdk)
	{
		WCHAR szPersonalPath[MAX_PATH];
		memset(szPersonalPath, 0, sizeof(WCHAR)*MAX_PATH);

		HRESULT hr = SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPersonalPath);
		if (FAILED(hr))
		{
			EnterCriticalSection(&s_criticalSection);
			err = pGsmSdk->Init(NULL);
			LeaveCriticalSection(&s_criticalSection);
		}
		else
		{
			PathAppendW(szPersonalPath, L"PGP");

			EnterCriticalSection(&s_criticalSection);
			err = pGsmSdk->Init(szPersonalPath);
			LeaveCriticalSection(&s_criticalSection);
		}

		if (err)
		{
			DPW((L"Initialise PGP SDK failed(key store=%s)(err=%d)\n", szPersonalPath, err));

			LoadStringW(g_hInstance, IDS_STR_INIT_ERROR, wzStringResource, 512);
			lpData->wstrErrorInfo = wzStringResource;

			delete pGsmSdk;
			pGsmSdk = NULL;
			return EA_E_INIT;
		}
	}
	else
	{
		DPW((L"EncryptionAdapter: new CPgpSdkAdapter failed!\n"));

		LoadStringW(g_hInstance, IDS_STR_NORESOURCE, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_NORESOURCE;
	}

	if (lpData->encryptContext.bSymm)
	{
		err = pGsmSdk->SymmEncryptFile(lpData->wstrSrcFile.c_str(), wzDstFileName, 
				lpData->encryptContext.wstrPassword.c_str());
		EnterCriticalSection(&s_criticalSection);
		pGsmSdk->Release();
		LeaveCriticalSection(&s_criticalSection);
		delete pGsmSdk;

		if (err != GSM_ERR_NO_ERROR)
		{
			WCHAR wzError[1024];

			DPW((L"EncryptionAdapter: SymmEncryptFile failed(%s -> %s)!\n",
				lpData->wstrSrcFile.c_str(), wzDstFileName));
			if (GSM_ERR_FILE_NOT_FOUND == err)
			{
				LoadStringW(g_hInstance, STR_SRC_NOTEXIST, wzStringResource, 512);
				_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, 
						lpData->wstrSrcFile.c_str());
				lpData->wstrErrorInfo = wzError;

				return EA_E_WRONGPATH;
			}
			else
			{
				LoadStringW(g_hInstance, IDS_STR_ENCRYPT_FAILED, wzStringResource, 512);
				_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, 
						lpData->wstrSrcFile.c_str(), wzDstFileName);
			}

			lpData->wstrErrorInfo = wzError;

			return EA_E_ENCRYPT;
		}

		lpData->wstrEncryptionType = L"PGP AES256";
	}
	else
	{
		GsmKeyHandle *hEncryptKeys = NULL;
		size_t nEncryptKeys = 2 + lpData->encryptContext.vecDecrypterInfos.size();
		int nActualEncryptKeys = 0;
		GsmKeyHandle *hKeys = NULL;
		int nKeys = 0;

		hEncryptKeys = (GsmKeyHandle *)malloc(sizeof(GsmKeyHandle) * nEncryptKeys);
		if (!hEncryptKeys)
		{
			DPW((L"EncryptionAdapter: malloc GsmKeyHandle failed!\n"));

			LoadStringW(g_hInstance, IDS_STR_NORESOURCE, wzStringResource, 512);
			lpData->wstrErrorInfo = wzStringResource;

			EnterCriticalSection(&s_criticalSection);
			pGsmSdk->Release();
			LeaveCriticalSection(&s_criticalSection);
			delete pGsmSdk;
			return EA_E_NORESOURCE;
		}

		memset(hEncryptKeys, 0, sizeof(GsmKeyHandle) * nEncryptKeys);

		err = pGsmSdk->FindKeyByUserID(lpData->encryptContext.wstrEncrypterInfo.c_str(), &hKeys, &nKeys, GSM_KEY_TYPE_ENCRYPT, 0);
		if (err || !nKeys)
		{
			WCHAR wzError[1024];
			DPW((L"EncryptionAdapter: no public key matched for %s!\n", lpData->encryptContext.wstrEncrypterInfo.c_str()));

			LoadStringW(g_hInstance, IDS_STR_NOKEY, wzStringResource, 512);
			_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, lpData->encryptContext.wstrEncrypterInfo.c_str());
			lpData->wstrErrorInfo = wzError;

			pGsmSdk->ReleaseKeyArray(hEncryptKeys, nActualEncryptKeys);
			EnterCriticalSection(&s_criticalSection);
			pGsmSdk->Release();
			LeaveCriticalSection(&s_criticalSection);
			delete pGsmSdk;
			return EA_E_NOPUBKEY;
		}
		else
		{
			hEncryptKeys[nActualEncryptKeys++] = hKeys[0];
			for (int i = 1; i < nKeys; i++)
			{
				pGsmSdk->ReleaseKey(hKeys[i]);
			}
			free(hKeys);
			hKeys = NULL;
		}

		StringVector::iterator iterEmail;
		for (iterEmail = lpData->encryptContext.vecDecrypterInfos.begin(); iterEmail != lpData->encryptContext.vecDecrypterInfos.end(); iterEmail++)
		{
			err = pGsmSdk->FindKeyByUserID((*iterEmail).c_str(), &hKeys, &nKeys, GSM_KEY_TYPE_ENCRYPT, 0);
			if (err || !nKeys)
			{
				WCHAR wzError[1024];

				DPW((L"EncryptionAdapter: no public key matched for %s!\n", (*iterEmail).c_str()));

				LoadStringW(g_hInstance, IDS_STR_NOKEY, wzStringResource, 512);
				_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, (*iterEmail).c_str());
				lpData->wstrErrorInfo = wzError;

				pGsmSdk->ReleaseKeyArray(hEncryptKeys, nActualEncryptKeys);
				EnterCriticalSection(&s_criticalSection);
				pGsmSdk->Release();
				LeaveCriticalSection(&s_criticalSection);
				delete pGsmSdk;
				return EA_E_NOPUBKEY;
			}
			else
			{
				hEncryptKeys[nActualEncryptKeys++] = hKeys[0];
				for (int i = 1; i < nKeys; i++)
				{
					pGsmSdk->ReleaseKey(hKeys[i]);
				}
				free(hKeys);
				hKeys = NULL;
			}
		}

		err = pGsmSdk->EncryptFile(lpData->wstrSrcFile.c_str(), wzDstFileName, hEncryptKeys, 
				nActualEncryptKeys);
		pGsmSdk->ReleaseKeyArray(hEncryptKeys, nActualEncryptKeys);
		EnterCriticalSection(&s_criticalSection);
		pGsmSdk->Release();
		LeaveCriticalSection(&s_criticalSection);
		delete pGsmSdk;
		if (err != GSM_ERR_NO_ERROR)
		{
			WCHAR wzError[1024];

			DPW((L"EncryptionAdapter: EncryptFile failed(%s -> %s)!\n",
				lpData->wstrSrcFile.c_str(), wzDstFileName));

			if (GSM_ERR_FILE_NOT_FOUND == err)
			{
				LoadStringW(g_hInstance, STR_SRC_NOTEXIST, wzStringResource, 512);
				_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, 
					lpData->wstrSrcFile.c_str());
				lpData->wstrErrorInfo = wzError;

				return EA_E_WRONGPATH;
			}
			else
			{
				LoadStringW(g_hInstance, IDS_STR_ENCRYPT_FAILED, wzStringResource, 512);
				_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, 
					lpData->wstrSrcFile.c_str(), wzDstFileName);
			}

			lpData->wstrErrorInfo = wzError;

			return EA_E_ENCRYPT;
		}

		lpData->wstrEncryptionType = L"PGP Asymmetric Encryption";
	}

	lpData->wstrActualDstFile = wzDstFileName;

	return EA_OK;
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	DWORD   dwDisposition	= 0;
	LONG    lResult			= 0;
	HKEY	hKeyNextlabs = NULL;
	HKEY    hKeyEncryption  = NULL;
	HKEY	hKeyPGPAdapter	= NULL;
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
// 	DWORD   dwVal			= 0;
// 	WCHAR   wzVal[MAX_PATH+1];    memset(wzVal, 0, sizeof(wzVal));
	LPWSTR	lpwzVal = NULL;

		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs");

		lResult = RegCreateKeyEx( HKEY_LOCAL_MACHINE, wzKeyName,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyNextlabs,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: Nextlabs\n"));
			return E_UNEXPECTED;
		}

		lResult = RegCreateKeyEx( hKeyNextlabs, L"Encryption",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyEncryption,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: Encryption\n"));

		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);
			return E_UNEXPECTED;
		}


		lResult = RegCreateKeyEx( hKeyEncryption, L"PGPAdapter",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyPGPAdapter,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: PGPAdapter\n"));

		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);
			return E_UNEXPECTED;
		}

		lpwzVal = GetBaseFileName(g_wzPGPAdapterPath);
		RegSetValueExW(hKeyPGPAdapter, L"DLLName", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));
		lpwzVal = L".pgp";
		RegSetValueExW(hKeyPGPAdapter, L"Extension", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));

		if(NULL != hKeyPGPAdapter) RegCloseKey(hKeyPGPAdapter);
		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);

		hKeyPGPAdapter   = NULL;
		hKeyEncryption = NULL;

	return S_OK;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));

	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs\\Encryption\\PGPAdapter");
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
}

static int ValidateEmailAddress(LPEncryptionAdapterData lpData)
{
	size_t ret = lpData->encryptContext.wstrEncrypterInfo.find(L'@');

	if ( ret < 0)
	{
		DPW((L"ValidateEmailAddress: Invalid sender email address(%s)!\n", lpData->encryptContext.wstrEncrypterInfo.c_str()));
		return -1;
	}

	StringVector::iterator iterEmail;
	for (iterEmail = lpData->encryptContext.vecDecrypterInfos.begin(); iterEmail != lpData->encryptContext.vecDecrypterInfos.end(); iterEmail++)
	{
		ret = (*iterEmail).find(L'@');
		if (ret < 0)
		{
			DPW((L"ValidateEmailAddress: Invalid recipient email address(%s)!\n", (*iterEmail).c_str()));
			return -1;
		}
	}

	return 0;
}

static LPWSTR GetBaseFileName(LPWSTR lpwzFileName)
{
	LPWSTR lpwzBaseFileName = NULL;

	lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '\\');
	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '/');
	}

	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)lpwzFileName;
	}
	else
	{
		lpwzBaseFileName++;
	}

	return lpwzBaseFileName;
}
