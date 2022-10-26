// zip_adapter.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "zip_adapter.h"

#include "log.h"

// add by tonny 
#include <vector>
#include <ShellAPI.h>

// zip_utils
#include <zip.h>
#include <unzip.h>

#import "MSXML3.dll" named_guids 
using namespace MSXML2;
using namespace std;
// end at here


#ifdef _MANAGED
#pragma managed(push, off)
#endif

WCHAR g_wzZipAdapterPath[MAX_PATH];
HINSTANCE g_hInstance = NULL;

wchar_t errbuf[256];

static LPWSTR GetBaseFileName(LPWSTR lpwzFileName);

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
			g_hInstance = (HINSTANCE)hModule;
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			g_hInstance = NULL;
		}
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif


//////////////////////////////////////////////////////////////////////////
// unzip file
ADAPTER_BASE_API int	UnZipFile(const wchar_t* strFilePath,const wchar_t* strTempFolder)
{
    std::wstring str1;
    int rv = 0;
	if(strFilePath==NULL || strTempFolder == NULL)	return EA_E_BADPARAM;
	if(!PathFileExistsW(strFilePath))	return EA_E_WRONGPATH;	// file doesn't exist.

	HZIP hzip = OpenZip((const TCHAR *)strFilePath, NULL); // no password
	if (hzip == NULL) {
	    _snwprintf(errbuf, 255, L"Failed to open zip file %s\n", strFilePath);
	    DPW((errbuf));
	    return EA_E_WRONGPATH;
	}

	ZIPENTRY ze;
	ZRESULT zrv = GetZipItem(hzip, -1, &ze);
	if (zrv != ZR_OK) {
	    _snwprintf(errbuf, 255, L"Failed to get information from %s (error code %d)\n", strFilePath, zrv);
	    DPW((errbuf));
	    rv = EA_E_GENERAL;
	    goto end;
	}
	int numItems = ze.index;

	// create target folder
	BOOL bRV = CreateDirectoryW(strTempFolder, NULL);
	if (bRV == FALSE) {
	    _snwprintf(errbuf, 255, L"Failed to create folder [%s] (error code %d)\n", strTempFolder, GetLastError());
	    DPW((errbuf));
	    rv = EA_E_GENERAL;
	    goto end;
	}

	for (int i=0; i<numItems; i++) {
	    zrv = GetZipItem(hzip, i, &ze);
	    if (zrv != ZR_OK) {
		_snwprintf(errbuf, 255, L"Failed to get info from archive [%s] item [%d] (error code %d)\n",
			   strFilePath,i, zrv);
		DPW((errbuf));
		rv = EA_E_GENERAL;
		goto end;
	    }
	    std::wstring str = std::wstring(strTempFolder) + std::wstring(L"/") + std::wstring(ze.name);
	    
	    zrv = UnzipItem(hzip, i, str.c_str());
	    if (zrv != ZR_OK) {
		_snwprintf(errbuf, 255, L"Failed to unzip from archive [%s] item [%d] (error code %d)\n",
			   strFilePath,i, zrv);
		DPW((errbuf));
		rv = EA_E_GENERAL;
		goto end;
	    }
	}
 end:
	CloseZip(hzip);
	
	return rv;
}

// recursively scan folder and add files to fileVec
DWORD scanFolder(vector<wstring>& fileVec, wstring &name)
{
    HANDLE hFind;
    WIN32_FIND_DATAW FindData;
    wstring actualSearch = name + L"\\*.*";

    // Find the first file
    hFind = FindFirstFileW(actualSearch.c_str(), &FindData);

    // Look for more
    do
	{
	    //If no files are found
	    if (hFind == INVALID_HANDLE_VALUE)
		{
		    //Done checking this folder
		    return 0;
		}
	    
	    wstring fileName (FindData.cFileName);
	    wstring fullPath = name + L"\\" + fileName;
	    
	    //If it is a self-reference
	    if (fileName == L"." || fileName == L"..")
		{
		    //Skip to next file
		    continue;
		}
	    //If it is a folder
	    else if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		    scanFolder(fileVec, fullPath);
		}
		//If it is a file
	    else
		{
		    fileVec.push_back(fullPath.c_str());
		}
	}	while (FindNextFileW(hFind, &FindData));

	FindClose(hFind);

	return 0;
}

std::wstring chopPrefix(std::wstring str, std::wstring prefix) {
    if (str.substr(0, prefix.length()) == prefix) {
	return str.substr(prefix.length(), str.length() - prefix.length());
    } else {
		wstring::size_type nIndex = str.rfind(L"\\");
		if(nIndex != wstring::npos)
			return str.substr(nIndex + 1, str.length() - nIndex - 1);

	return str;
    }
}

// Zip all the files and folders in strFileVec into strZipFile
// if password is not NULL, it will be used to encrypt the archive
// if strZipFile is A.zip, and strFileVec[i] is A\filename, this function will chop "A\" from the file name in the zip archive to be created
int ZipFileEx(const std::vector<std::wstring>& strFileVec,const wchar_t* strZipFile, const char* password)
{
    std::wstring strArchive = strZipFile;
    std::wstring prefix = strArchive.substr(0, strArchive.find(L".zip")) + std::wstring(L"\\");

    int rv = 0;
    if(strFileVec.empty() || strZipFile==NULL)	return EA_E_BADPARAM;

    HZIP hzip = CreateZip((const TCHAR *)strZipFile, password);
    if (hzip == NULL) {
	_snwprintf(errbuf, 255, L"Failed to create zip file %s\n", strZipFile);
	DPW((errbuf));
	return EA_E_GENERAL;
    }

    // explore folders recursively and add files into vector
    vector<wstring> expandedFileVec;
    for(size_t i=0;i<strFileVec.size();i++) {
	DWORD attr = GetFileAttributes(strFileVec[i].c_str());
	if (attr != INVALID_FILE_ATTRIBUTES) {
	    if ((attr & FILE_ATTRIBUTE_DIRECTORY) > 0) {
	    wstring name = strFileVec[i];
	    scanFolder(expandedFileVec, name);
	    } else {
		// file
		expandedFileVec.push_back(strFileVec[i].c_str());
	    }
	}
    }

    for(size_t i=0;i<expandedFileVec.size();i++) {
	// see if it's a folder
	std::wstring chopped = chopPrefix(std::wstring(expandedFileVec[i].c_str()), prefix);

	wchar_t buf[1024] = {0};
	_snwprintf_s(buf, 1024, _TRUNCATE, L"try to zip file: %s, chopped: %s, prefix: %s\r\n", expandedFileVec[i].c_str(), chopped.c_str(), prefix.c_str());
	DPW((buf));

	ZRESULT zrv = ZipAdd(hzip, chopped.c_str(), (const TCHAR *)(expandedFileVec[i].c_str()));

	if (zrv != ZR_OK) {
	    _snwprintf(errbuf, 255, L"Failed to add file %s to zip archive (error code %d)\n", expandedFileVec[i].c_str(), zrv);
	    DPW((errbuf));
	    rv = EA_E_GENERAL;
	    goto end;
	}
    }
    rv = 0; // succeeded
 end:
    CloseZip(hzip);

    return rv;
}

ADAPTER_BASE_API int	ZipFile(const std::vector<std::wstring>& strFileVec,const wchar_t* strZipFile)
{
    return ZipFileEx(strFileVec, strZipFile, NULL);
}

// Encrypt file using password and archive it in zip file
// * Important Note * This function is untested.  Implementation was changed from using 7zip library to zip_utils on 64 bit platform.
// It must be tested with encryption policy assistant when this is merged into mainline
ADAPTER_BASE_API EA_Error WINAPI Encrypt(EncryptionAdapterData *lpData)
{
	struct _stat stat_buffer;
	std::wstring wstrTmpPath = L"";
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

	if (!lpData->encryptContext.bSymm)
	{
		DPW((L"EncryptionAdapter: Doesn't support asymmetric encryption for 7-ZIP!\n"));
		LoadStringW(g_hInstance, IDS_STR_UNSUPPORTED, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_UNSUPPORTED;
	}

	if (lpData->encryptContext.wstrPassword.empty())
	{
		DPW((L"EncryptionAdapter: Should supply password for symmetric encryption!\n"));
		LoadStringW(g_hInstance, IDS_STR_NOPASSWD, wzStringResource, 512);
		lpData->wstrErrorInfo = wzStringResource;
		return EA_E_BADPASS;
	}

	if (_wstat(lpData->wstrSrcFile.c_str(), &stat_buffer) != 0)
	{
		WCHAR wzError[1024];

		DPW((L"EncryptionAdapter: source file: %s does not exist!\n", lpData->wstrSrcFile));

		LoadStringW(g_hInstance, IDS_STR_SRC_NOTEXIST, wzStringResource, 512);
		_snwprintf_s(wzError, 1024, _TRUNCATE, wzStringResource, lpData->wstrSrcFile.c_str());
		lpData->wstrErrorInfo = wzError;
		return EA_E_WRONGPATH;
	}

	vector<wstring> vecFiles;
	vecFiles.push_back(lpData->wstrSrcFile);
	wstring wstrArchive = lpData->wstrDstFolder;
	if (lpData->wstrDstFolder.c_str()[lpData->wstrDstFolder.size()] != '\\' && 
	    lpData->wstrDstFolder.c_str()[lpData->wstrDstFolder.size()] != '/')
	    {
		wstrArchive += L"\\";
	    }
	wstrArchive += lpData->wstrBaseFileName.c_str();
	wstrArchive += L".zip";
	
	char *charPassword = new char[lpData->encryptContext.wstrPassword.length() + 2];
	_snprintf(charPassword, lpData->encryptContext.wstrPassword.length() + 1, "%ls", lpData->encryptContext.wstrPassword.c_str()); 
	
	EA_Error rv = (EA_Error)ZipFileEx(vecFiles, wstrArchive.c_str(), charPassword);

	if( rv == EA_OK )
	{
		lpData->wstrActualDstFile = wstrArchive.c_str() ;
	}

	delete[] charPassword;
	
	return rv;
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	DWORD   dwDisposition	= 0;
	LONG    lResult			= 0;
	HKEY    hKeyNextlabs  = NULL;
	HKEY    hKeyEncryption  = NULL;
	HKEY	hKeyZipAdapter	= NULL;
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));
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

		lResult = RegCreateKeyEx( hKeyEncryption, L"ZipAdapter",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyZipAdapter,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: ZipAdapter\n"));

		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);
			return E_UNEXPECTED;
		}

// 		dwVal = 1;
// 		RegSetValueExW(hKeyZipAdapter, L"Symmetric", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
// 		dwVal = 0;
// 		RegSetValueExW(hKeyZipAdapter, L"Asymmetric", 0, REG_DWORD, (const BYTE*)&dwVal, sizeof(DWORD));
		//sprintf_s(szVal, MAX_PATH, "Enterprise DLP Office PEP"/*"Compliant Enterprise Office PEP"*/);
		lpwzVal = GetBaseFileName(g_wzZipAdapterPath);
		RegSetValueExW(hKeyZipAdapter, L"DLLName", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));
		lpwzVal = L".zip";
		RegSetValueExW(hKeyZipAdapter, L"Extension", 0, REG_SZ, (const BYTE*)lpwzVal, (1+(DWORD)wcslen(lpwzVal))*sizeof(WCHAR));

		if(NULL != hKeyZipAdapter) RegCloseKey(hKeyZipAdapter);
		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);

		hKeyZipAdapter   = NULL;
		hKeyEncryption = NULL;

	return S_OK;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));

	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs\\Encryption\\ZipAdapter");
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
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
