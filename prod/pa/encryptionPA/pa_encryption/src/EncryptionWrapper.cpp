#include "stdafx.h"

#include "pa_encryption.h"
#include "EncryptionWrapper.h"

#include "log.h"

#include "AdapterBase.h"
#include "nlconfig.hpp"

#define PA_ENCRYPT_CONF		L"\\encryption_adapters.conf"
#define PA_ENCRYPT_CONF32		L"\\encryption_adapters32.conf"

typedef EA_Error (WINAPI *funcEncryptionAdapter)(EncryptionAdapterData *lpData);

static LPWSTR GetBaseFileName(LPWSTR lpwzFileName);
// static void GetDefaultAdapterFromConf( LPEncryptAdapters lpAdapters );
static void GetRandomFolderName(std::wstring &wstrFolderName);

static LPEncryptAdapters s_lpEncryptAdapters = NULL;
static CRITICAL_SECTION s_criticalSection;

extern HINSTANCE g_hInstance;
extern WCHAR g_wzDllPath[MAX_PATH];

typedef struct _tagErrorMessage
{
	EA_Error errorCode;
	LPCWSTR	lpwzErrorStr;
} ErrorMessage;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH + 1] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
		wchar_t* pTemp = wcsrchr(szDir, L'\\');
		if ( pTemp && !( * ( pTemp + 1 ) ) )
		{
			*pTemp = 0;
		}

		return szDir;
	}

	return L"";
}

static ErrorMessage s_aErrorMessage[] = 
{
	{EA_E_CANCELED,			L"User canceled the operation!"},
	{EA_E_UNINITIALISED,	L"The Encryption Adapter is uninitialized yet!"},
	{EA_E_INIT,				L"Failed to initialize the Encryption Adapter!"},
	{EA_E_UNSUPPORTED,		L"The Encryption Adapter does not support the operation!"},
	{EA_E_NORESOURCE,		L"Failed to allocate memory resource!"},
	{EA_E_NULLPTR,			L"Invalid pointer!"},
	{EA_E_BADPARAM,			L"Unacceptable parameters passed!"},
	{EA_E_BADEMAIL,			L"The Email address is invalid! It must contain '@'!"},
	{EA_E_BADPASS,			L"Bad password!"},
	{EA_E_NOPUBKEY,			L"None of public key found!"},
	{EA_E_NOSECKEY,			L"None of secret key found!"},
	{EA_E_KEYUNAUTHORIZED,	L"The encryption key is unauthorized!"},
	{EA_E_ENCRYPT,			L"Unknown failed reason during encrypting file!"},
	{EA_E_DECRYPT,			L"Unknown failed reason during decrypting file!"},
	{EA_E_WRONGPATH,		L"Invalid file path!"},
	{EA_E_FILEEXISTED,		L"File already exists!"},
	{EA_E_FILENOPERMISSION,	L"No permission to do the file operation!"},
	{EA_E_GENERAL,			L"Unknown error!"}
};

/* Load all registered encryption adapter keys. It should be called in DLLMain */
PA_STATUS LoadRegisteredAdapters( void )
{
#ifdef USING_REGISTRY
	HKEY    hKeyEncryption  = NULL;
	HKEY    hKeyAdapter		= NULL;
	WCHAR   wzKeyName[MAX_PATH+1];memset(wzKeyName, 0, sizeof(wzKeyName));
	DWORD	dwIndex			= 0;
	DWORD   dwDisposition	= 0;
	WCHAR	wzAdapterKeyName[MAX_PATH+1];memset(wzAdapterKeyName, 0, sizeof(wzAdapterKeyName));
	WCHAR	wzValue[MAX_PATH+1];memset(wzValue, 0, sizeof(wzValue));
	DWORD	dwLength		= MAX_PATH;
#endif

	if (s_lpEncryptAdapters)
	{
		return PA_SUCCESS;
	}

	s_lpEncryptAdapters = new EncryptAdapters;
	if (!s_lpEncryptAdapters)
	{
		DP((L"new EncryptAdapters failed!\n"));
		return PA_ERROR;
	}

#ifndef USING_REGISTRY
	WCHAR wzConfFilePath[MAX_PATH+1];memset(wzConfFilePath, 0, sizeof(wzConfFilePath));
	WCHAR wzSectionNames[1024];memset(wzSectionNames, 0, sizeof(wzSectionNames));
	WCHAR wzSectionName[128];memset(wzSectionName, 0, sizeof(wzSectionName));
	WCHAR wzValueName[128];memset(wzValueName, 0, sizeof(wzValueName));
	LPWSTR lpwzSecStart = NULL;

	std::wstring strCommonDir = GetCommonComponentsDir();
#ifdef _WIN64
	_snwprintf_s(wzConfFilePath, MAX_PATH, _TRUNCATE, L"%s\\config\\%s", strCommonDir.c_str(), PA_ENCRYPT_CONF);
#else
	_snwprintf_s(wzConfFilePath, MAX_PATH, _TRUNCATE, L"%s\\config\\%s", strCommonDir.c_str(), PA_ENCRYPT_CONF32);
#endif

	GetPrivateProfileSectionNamesW(wzSectionNames, 1024, wzConfFilePath);
	lpwzSecStart = wzSectionNames;
	for (; *lpwzSecStart != 0; )
	{
		EncryptAdapter adapter;

		wcsncpy_s(wzSectionName, 128, lpwzSecStart, _TRUNCATE);

		lpwzSecStart += (wcslen(lpwzSecStart) + 1);
		adapter.wstrAdapterName = wzSectionName;

		GetPrivateProfileStringW(wzSectionName, L"DLLName", L"", wzValueName, 128, wzConfFilePath);
		adapter.wstrDLLPath = g_wzDllPath;
		adapter.wstrDLLPath += wzValueName;

		GetPrivateProfileStringW(wzSectionName, L"Extension", L"", wzValueName, 128, wzConfFilePath);
		adapter.wstrExtension = wzValueName;

		s_lpEncryptAdapters->listAdapters.push_back(adapter);
	}

#else
	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs\\Encryption");
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_READ, &hKeyEncryption);//fix bug 7588
	if(ERROR_SUCCESS != lResult)     // get Nextlabs\PAF\Encryption key
	{
		DP((L"LoadRegisteredAdapters::Fail to open key: %s\n", wzKeyName));
		return PA_ERROR;
	}

//	GetDefaultAdapterFromConf(s_lpEncryptAdapters);

	do 
	{
		lResult = RegEnumKey(hKeyEncryption, dwIndex++, wzAdapterKeyName, MAX_PATH);
		if (lResult == ERROR_SUCCESS)
		{
			EncryptAdapter adapter;
			adapter.wstrAdapterName = wzAdapterKeyName;

			DWORD dwValue = 0;
			DWORD dwType = 0;

			lResult = RegOpenKeyEx(hKeyEncryption, wzAdapterKeyName, 0, KEY_READ, &hKeyAdapter);//fix bug 7588
			if (lResult != ERROR_SUCCESS)
			{
				continue;
			}

			dwLength = MAX_PATH;
			lResult = RegQueryValueEx(hKeyAdapter, L"DLLName", NULL, &dwType, (LPBYTE)wzValue, &dwLength);
			adapter.wstrDLLPath = g_wzDllPath;
			adapter.wstrDLLPath += wzValue;
			if (!dwLength)
			{
				RegCloseKey(hKeyAdapter);
				hKeyAdapter = NULL;
				continue;
			}

			dwType = 0;
			dwLength = MAX_PATH;
			lResult = RegQueryValueEx(hKeyAdapter, L"Extension", NULL, &dwType, (LPBYTE)wzValue, &dwLength);
			adapter.wstrExtension = wzValue;

			RegCloseKey(hKeyAdapter);
			hKeyAdapter = NULL;

			s_lpEncryptAdapters->listAdapters.push_back(adapter);
		}
	} while (lResult == ERROR_SUCCESS);

	if (hKeyEncryption)
	{
		RegCloseKey(hKeyEncryption);
		hKeyEncryption = NULL;
	}
#endif

	InitializeCriticalSection(&s_criticalSection);

	return PA_SUCCESS;
}

/* Unload all registered encryption adapter keys. It should be called in DLLMain */
PA_STATUS UnloadRegisteredAdapters( void )
{
	if (s_lpEncryptAdapters)
	{
		AdapterList::iterator iterAdapter;
		for (iterAdapter = s_lpEncryptAdapters->listAdapters.begin();
			iterAdapter != s_lpEncryptAdapters->listAdapters.end(); iterAdapter++)
		{
			if ((*iterAdapter).hAdapterDLL)
			{
				FreeLibrary((*iterAdapter).hAdapterDLL);
				(*iterAdapter).hAdapterDLL = NULL;
			}
		}

		s_lpEncryptAdapters->listAdapters.clear();
		delete s_lpEncryptAdapters;
		s_lpEncryptAdapters = NULL;

		DeleteCriticalSection(&s_criticalSection);
	}

	return PA_SUCCESS;
}

PA_STATUS EncryptionWrapper(LPEA_AssistantData lpAssistant)
{
	EncryptAdapter *pAdapter = NULL;
	funcEncryptionAdapter fEncryptionAdapter = NULL;
	AdapterList::iterator iterAdapter;
	LPWSTR lpwzDefaultAdapter = NULL;
	BOOL bSymm = TRUE;
	EA_Error err = EA_OK;
	LPEA_EncryptAssistantData lpEncryptAssistant = NULL;

	if (lpAssistant->currObType == EA_ObligationType_SymmetricEncryptionAssistant)
	{
		lpEncryptAssistant = &(lpAssistant->symmAssistantData);
	}
	else
	{
		lpEncryptAssistant = &(lpAssistant->certAssistantData);
	}

	if (!s_lpEncryptAdapters || s_lpEncryptAdapters->listAdapters.empty())
	{
		DP((L"EncryptionWrapper: None of Encryption Adapter is installed!\n"));

		lpAssistant->wstrErrorMessage = L"None of Encryption Adapter is installed!";
		return PA_ERROR;
	}

	/* Use the adapter specified by obligation */
	if (!lpEncryptAssistant->obligation.wstrEncryptAdapterName.empty())
	{
		lpwzDefaultAdapter = (LPWSTR)lpEncryptAssistant->obligation.wstrEncryptAdapterName.c_str();
	}

	if (lpEncryptAssistant->obligation.obType == EA_ObligationType_SymmetricEncryptionAssistant)
	{
		bSymm = TRUE;
	}
	else
	{
		bSymm = FALSE;
	}

	if (lpwzDefaultAdapter)
	{
		for (iterAdapter = s_lpEncryptAdapters->listAdapters.begin();
			iterAdapter != s_lpEncryptAdapters->listAdapters.end(); iterAdapter++)
		{
			if (!_wcsnicmp((*iterAdapter).wstrAdapterName.c_str(), lpwzDefaultAdapter, wcslen(lpwzDefaultAdapter)))
			{
				pAdapter = &(*iterAdapter);
				break;
			}
		}
	}

	if (!pAdapter)
	{
		DP((L"EncryptionWrapper: The encryption adapter is not installed!\n"));
		if (lpEncryptAssistant->obligation.obType == EA_ObligationType_SymmetricEncryptionAssistant)
		{
			lpAssistant->wstrErrorMessage = L"Encryption adapter is not found for \"File Encryption Assistant\" obligation!";
		}
		else
		{
			lpAssistant->wstrErrorMessage = L"Encryption adapter is not found for \"Certificate Encryption Assistant\" obligation!";
		}

		return PA_ERROR;
	}

// 	EnterCriticalSection(&s_criticalSection);
	if (!pAdapter->hAdapterDLL)
	{
		pAdapter->hAdapterDLL = LoadLibraryW(pAdapter->wstrDLLPath.c_str());
	}
// 	LeaveCriticalSection(&s_criticalSection);

	if (!pAdapter->hAdapterDLL)
	{
		DP((L"EncryptionWrapper: Can't load the encryption adapter dll: %s!\n",
			pAdapter->wstrDLLPath.c_str()));
		// TODO: Prompt error message
		WCHAR wzMessage[1024];
		_snwprintf_s(wzMessage, 1024, _TRUNCATE, L"Failed to load %s's DLL file: %s!", pAdapter->wstrAdapterName.c_str(), pAdapter->wstrDLLPath.c_str());

		lpAssistant->wstrErrorMessage = wzMessage;

		return PA_ERROR;
	}

	fEncryptionAdapter = (funcEncryptionAdapter)GetProcAddress(pAdapter->hAdapterDLL, "Encrypt");
	if (!fEncryptionAdapter)
	{
		DP((L"EncryptionWrapper: Invalid encryption adapter dll: %s! It should supply EncryptionAdapter interface!\n",
			pAdapter->wstrAdapterName.c_str()));
		WCHAR wzMessage[1024];
		_snwprintf_s(wzMessage, 1024, _TRUNCATE, L"Invalid encryption adapter DLL: %s!", pAdapter->wstrDLLPath.c_str());
		lpAssistant->wstrErrorMessage = wzMessage;

		return PA_ERROR;
	}

	EncryptionAdapterData data;
	data.encryptContext.bSymm = bSymm;
	data.encryptContext.wstrEncrypterInfo = lpEncryptAssistant->wstrSenderEmail;
	data.encryptContext.vecDecrypterInfos = lpEncryptAssistant->vecRecipients;
	data.encryptContext.wstrPassword = lpEncryptAssistant->wstrPassword;

	BOOL bDeleteFile = FALSE;
	std::wstring wstrTmpFilePath = L"";
	struct _stat stat_buffer;
	std::list<EA_FileData>::iterator iterFile;
	for (iterFile = lpEncryptAssistant->listFiles.begin(); 
		iterFile != lpEncryptAssistant->listFiles.end(); iterFile++)
	{
		data.wstrSrcFile = (*iterFile).wstrSrcFile;
		data.wstrDstFolder = (*iterFile).wstrTmpDstFolder;
		data.wstrBaseFileName = (*iterFile).wstrBaseFileName;
		wstrTmpFilePath = data.wstrDstFolder;
		wstrTmpFilePath += L"\\";
		wstrTmpFilePath += data.wstrBaseFileName.c_str();
		wstrTmpFilePath += pAdapter->wstrExtension.c_str();

		bDeleteFile = FALSE;

		DPW((L"Encryption: Check whether dst file %s exists!...", wstrTmpFilePath.c_str()));
		if (_wstat(wstrTmpFilePath.c_str(), &stat_buffer) == 0)
		{
			/* If the encrypted file exists, create a random temp folder to hold the new one. */
			std::wstring wstrRandomString = L"";
			GetRandomFolderName(wstrRandomString);

			data.wstrDstFolder = (*iterFile).wstrTmpDstFolder;
			data.wstrDstFolder += L"\\";
			data.wstrDstFolder += wstrRandomString;

			CreateDirectoryW(data.wstrDstFolder.c_str(), NULL);

			DPW((L"Yes!\n"));
			DPW((L"Encryption: Create new temp directory %s\n", data.wstrDstFolder.c_str()));
		}
		DPW((L"No!\n"));

		// Maintain the same file name with the original file name
 		if (lpAssistant->bMaintainFileNameAfterEncrypted)
		{
			std::wstring wstrTemp = data.wstrDstFolder;
			if (wstrTemp[wstrTemp.length()-1] != L'\\')
					wstrTemp += L"\\";
			wstrTemp += data.wstrBaseFileName;

			if (_wstat(wstrTemp.c_str(), &stat_buffer) == 0)
			{
				/* If the copied file exists, create a random temp folder to hold the new one. */
				std::wstring wstrRandomString = L"";
				GetRandomFolderName(wstrRandomString);

				wstrTemp = data.wstrDstFolder;
				if (wstrTemp[wstrTemp.length()-1] != L'\\')
					wstrTemp += L"\\";
				wstrTemp += wstrRandomString.c_str();
				wstrTemp += L"\\";
				
				CreateDirectoryW(wstrTemp.c_str(), NULL);

				wstrTemp += data.wstrBaseFileName;
			}

			BOOL bRet = CopyFileW(data.wstrSrcFile.c_str(), wstrTemp.c_str(), FALSE);
			DPW((L"Encryption: Copy File from %s to %s.\n", data.wstrSrcFile.c_str(), wstrTemp.c_str()));
			if (!bRet)
			{
				DPW((L"CopyFileW failed!(error=0x%x)\n", GetLastError()));
			}
			data.wstrSrcFile = wstrTemp;
			bDeleteFile = TRUE;
		}

		// Show which file is being encrypted.
		if (lpAssistant->pProgressDlg)
		{
			WCHAR wzDescription[1024];
			WCHAR wzResource[512];

			LoadStringW(g_hInstance, IDS_PROGRESS_ENCRYPTING, wzResource, 512);
			_snwprintf_s(wzDescription, 1024, _TRUNCATE, wzResource, (*iterFile).wstrFileDisplayName.c_str());
			lpAssistant->pProgressDlg->set_Description(wzDescription);
		}

		DPW((L"Start encrypting file %s...\n", (*iterFile).wstrFileDisplayName.c_str()));
		err = fEncryptionAdapter(&data);
		DPW((L"File %s is encrypted to %s.(error=%d)\n", (*iterFile).wstrFileDisplayName.c_str(), data.wstrActualDstFile.c_str(), err));

		if (bDeleteFile)
		{
			DeleteFileW(data.wstrSrcFile.c_str());
		}

		if (err != EA_OK)
		{
			lpAssistant->wstrErrorMessage = data.wstrErrorInfo.c_str();

			return PA_ERROR;
		}

		(*iterFile).wstrActualDstFile = data.wstrActualDstFile;
		(*iterFile).bFileNameChanged = TRUE;
		(*iterFile).bResult = TRUE;

		if ((*iterFile).pOriginalObjectInfo)
		{
			/* The parameters below will be passed out of PA. */
			(*iterFile).pOriginalObjectInfo->bFileNameChanged = TRUE;
			(*iterFile).pOriginalObjectInfo->lPARet = 0;
			/*
				Modified by chellee on 14/10/08 ;6:32
				mark Code:(*iterFile).pOriginalObjectInfo->strRetName = data.wstrActualDstFile;
			*/
			::wcsncpy_s( (*iterFile).pOriginalObjectInfo->strRetName, MAX_PATH, data.wstrActualDstFile.c_str(), _TRUNCATE ) ;
			
		}
	}

	return PA_SUCCESS;
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

static void GetRandomFolderName(std::wstring &wstrFolderName)
{
	WCHAR wzFolderName[MAX_PATH];
	DWORD dwCurrentTick = 0;

	dwCurrentTick = GetTickCount();
	_snwprintf_s(wzFolderName, MAX_PATH, _TRUNCATE, L"%8x", dwCurrentTick);

	wstrFolderName = wzFolderName;
}