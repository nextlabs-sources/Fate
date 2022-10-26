/*
 * SysUtils.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */

#pragma once
#ifndef _SYSUTILS_H_
#define _SYSUTILS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>
#include <string>
#include <vector>
using namespace std;

#pragma warning(disable: 4505)  // Although some functions in this file are not called
                                // we still keep them. Disable warning 4505
//---------------------------------------------------------------------------
// ModuleFromAddress
//
// Returns the HMODULE that contains the specified memory address
//---------------------------------------------------------------------------
static HMODULE ModuleFromAddress(PVOID pv) 
{
	MEMORY_BASIC_INFORMATION mbi;

	return ((::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
	        ? (HMODULE) mbi.AllocationBase : NULL);
}

//---------------------------------------------------------------------------
// GetProcessHostFullName
//
// Return the path and the name of the current process
//---------------------------------------------------------------------------
static BOOL GetProcessHostFullName(WCHAR* pszFullFileName)
{
	DWORD dwResult = 0;
	::ZeroMemory((PBYTE)pszFullFileName, sizeof(WCHAR) * MAX_PATH);
	if (TRUE != ::IsBadReadPtr((PBYTE)pszFullFileName, sizeof(WCHAR) * MAX_PATH))
		dwResult = ::GetModuleFileNameW(
			NULL,                   // handle to module
			pszFullFileName,        // file name of module
			MAX_PATH                // size of buffer
			);

	return (dwResult != 0);
}

static BOOL MyGetCurrentDirectory(char* pszFullFileName, int nLen)
{
	static char szCurDirectory[MAX_PATH + 1] = {0};

	if(strlen(szCurDirectory) == 0)
	{
		::GetModuleFileNameA(
					NULL,                   // handle to module
					szCurDirectory,        // file name of module
					MAX_PATH                // size of buffer
					);

		char* pTemp = strrchr(szCurDirectory, '\\');
		if(pTemp)
		{
			*(++pTemp) = 0;
		}
	}
	
	
	strncpy_s(pszFullFileName, nLen, szCurDirectory, _TRUNCATE);
	

	return strlen(pszFullFileName) > 0? TRUE: FALSE;
}

//---------------------------------------------------------------------------
// GetProcessHostName
//
// Return the name of the current process
//---------------------------------------------------------------------------
static BOOL GetProcessHostName(WCHAR* pszFullFileName)
{
	BOOL  bResult;
	bResult = GetProcessHostFullName(pszFullFileName);
	return bResult;
}
/**
 * gets the original file name of the running executable.
 * @return true if original file name is retrieved successfully
 */
static BOOL GetOriginalExeName (WCHAR *pszwOriginalExeName,size_t pszwOriginalExeName_count) 
{
    BOOL  bResult = FALSE;
    WCHAR szwFullName [MAX_PATH];

    bResult = ::GetModuleFileName(
        NULL,                   // handle to module
        szwFullName,        // file name of module
        MAX_PATH                // size of buffer
        );

    if (bResult)
    {
        DWORD versionInfoSize, handle;
        LPVOID versionInfo = NULL;

        // Resetting bResult here because, the subsequent call 
        // may fail and the pszwOriginalExeName will not be valid
        // e.g. our PolicyAuthor.exe ;->
        bResult = FALSE;

        versionInfoSize = ::GetFileVersionInfoSize (szwFullName, &handle);

        if (versionInfoSize > 0)
        {
            versionInfo = malloc (versionInfoSize); 
            if (versionInfo != NULL)
            {
				::GetFileVersionInfo (szwFullName, 0, versionInfoSize, versionInfo);
                DWORD vLen,langD;

                LPVOID retbuf=NULL;

                wchar_t fileEntry[256];

                _snwprintf_s(fileEntry, _countof(fileEntry), _TRUNCATE,L"\\VarFileInfo\\Translation");
                bResult = ::VerQueryValue (versionInfo, _T("\\VarFileInfo\\Translation"), &retbuf, (UINT *)&vLen);
                if (bResult && vLen==4) 
                {
                    memcpy(&langD, retbuf, 4);            
                    _snwprintf_s(fileEntry, _countof(fileEntry), _TRUNCATE, L"\\StringFileInfo\\%02X%02X%02X%02X\\%s",
                        (langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24, 
                        (langD & 0xff0000)>>16, _T("OriginalFilename"));            
                }
                else 
                {
		  _snwprintf_s(fileEntry, _countof(fileEntry), _TRUNCATE,L"\\StringFileInfo\\%04X04B0\\%s", GetUserDefaultLangID(), _T("OriginalFilename"));
                }
                bResult = ::VerQueryValue (versionInfo, fileEntry, &retbuf, (UINT *)&vLen);
		if(bResult && retbuf != NULL && vLen > 0) {
		  wcsncpy_s (pszwOriginalExeName, 
			     pszwOriginalExeName_count, 
			     (TCHAR*) retbuf, _TRUNCATE);
		}
                free (versionInfo);
            } // if
        }
    }
    return bResult;
}

//
// Converts a string to a boolean value
//
static BOOL StrToBool(const char* pszValue)
{
	return ( (0 == _stricmp("YES", pszValue)) ||
		     (0 == _stricmp("Y", pszValue)) ||
		     (0 == _stricmp("TRUE", pszValue)) );
}

//
// Converts a boolean value to a string
//
static void BoolToStr(BOOL bValue, char* pszResult)
{
	bValue ? strncpy_s(pszResult, MAX_PATH, "Yes", _TRUNCATE) : 
	         strncpy_s(pszResult, MAX_PATH, "No", _TRUNCATE);
}

//
// Trims leading spaces and control characters from a string
//
static void TrimLeft(
	const char*  pszParam,
	char*        pszResult
	)
{
	char szBuffer[MAX_PATH] = {0};

	if ( (TRUE != ::IsBadStringPtrA(pszParam, MAX_PATH)) &&
		 (strlen(pszParam) > 0) )
	{
		DWORD dwIndex = 0;
		while ( (dwIndex < strlen(pszParam)) && (pszParam[dwIndex] == ' ') )
			dwIndex++;
		if (dwIndex < strlen(pszParam))
			strncpy_s(szBuffer, MAX_PATH, &pszParam[dwIndex], _TRUNCATE);
	} // if
	strncpy_s(pszResult, MAX_PATH, szBuffer, _TRUNCATE);
}
	
//
// Trims trailing spaces and control characters from a string
//
static void TrimRight(
	const char*  pszParam,
	char*        pszResult
	)
{
	char szBuffer[MAX_PATH] = {0};

	if ( (TRUE != ::IsBadStringPtrA(pszParam, MAX_PATH)) &&
		 (strlen(pszParam) > 0) )
	{
		size_t nIndex = strlen(pszParam) - 1;
		while ( (nIndex >= 0) && (pszParam[nIndex] == ' ') )
			nIndex--;
		if (nIndex >= 0)
		{
			memcpy(
				(PBYTE)szBuffer, 
				(PBYTE)pszParam, 
				(nIndex + 1)
				); 
			szBuffer[nIndex+1] = 0;
		} // if
	} // if
	strncpy_s(pszResult, MAX_PATH, szBuffer, _TRUNCATE);
}

//
// Trims leading and trailing spaces and control characters from a string
//
static void Trim(
	const char*  pszParam,
	char*        pszResult
	)
{
	TrimLeft(pszParam, pszResult);
	TrimRight(pszParam, pszResult);
}



//
// Return next entry of an comma separated string
//
static BOOL GetNextCommaSeparatedString(
	const char*  pszParam,
	char*        pszResult,
	DWORD        dwLength,
	LONG_PTR*        pnCommaPos
	)
{
	*pnCommaPos = -1;
	BOOL   bResult = FALSE;
	char*  pdest;
	strncpy_s(pszResult, dwLength, "\0", _TRUNCATE);

	if (strlen(pszParam) > 0)
	{
		::ZeroMemory((PBYTE)pszResult, dwLength);

		pdest = strstr((char *)pszParam, ",");
		if (pdest)
			*pnCommaPos = pdest - pszParam - 1;
		else
			*pnCommaPos = strlen(pszParam);
		memcpy(
			(PBYTE)pszResult, 
			(PBYTE)pszParam, 
			((*pnCommaPos) + 1)
			); 
		(*pnCommaPos)++;

		Trim(pszResult, pszResult);

		bResult = TRUE;
	} // if

	return bResult;
}

static BOOL AnsiToUnicode(
     LPCSTR  pszAnsiBuff,
     LPWSTR lpWideCharStr,
     int    cchWideChar)
{
    if (pszAnsiBuff)
    {
        int iRet = MultiByteToWideChar(
            CP_ACP,         // code page
            0,              // character-type options
            pszAnsiBuff,    // string to map
            (int)(strlen(pszAnsiBuff) + 1), // number of bytes in string
            lpWideCharStr,   // wide-character buffer
            cchWideChar    // size of buffer
            );
        lpWideCharStr[cchWideChar - 1] = L'\0'; // append 0 unicode at the end of the string
        return (0 != iRet);
    }
    else 
    {
        lpWideCharStr [0] = 0;
        return FALSE;
    }
}


//---------------------------------------------------------------------------
// LoadLibraryFromCurrentModuleDir
// 
// Loads the specified DLL from the same directory as the current module
//---------------------------------------------------------------------------
#if _MSC_VER >= 1300    // for VC 7.0
   // from ATL 7.0 sources
   #ifndef _delayimp_h
   extern "C" IMAGE_DOS_HEADER __ImageBase;
   #endif
#endif

typedef struct {
		DWORD perm;
		TCHAR *str;
} PermAndStr;

//
// Return a string containing the description of the specified access rights
// You need to deleted the return string once done with it
// This function is intended to be used for debugging, especially for OnCreateFile
//
static TCHAR* getFileAccessString(DWORD access) {

	std::wstring result = L"";
	int i, c;
	BOOL combiFound = FALSE;

	if (!combiFound) {
		PermAndStr perms[] = {
			{ GENERIC_READ, L"GENERIC_READ" },
			{ GENERIC_WRITE, L"GENERIC_WRITE" },
			{ GENERIC_EXECUTE, L"GENERIC_EXECUTE" },

			{ DELETE, L"DELETE" },
			{ READ_CONTROL, L"READ_CONTROL" },
			{ SYNCHRONIZE, L"SYNCHRONIZE" },
			{ WRITE_DAC, L"WRITE_DAC" },
			{ WRITE_OWNER, L"WRITE_OWNER" },
			{ FILE_READ_DATA, L"FILE_READ_DATA" },
			{ FILE_LIST_DIRECTORY, L"FILE_LIST_DIRECTORY" },
			{ FILE_WRITE_DATA, L"FILE_WRITE_DATA" },
			{ FILE_ADD_FILE, L"FILE_ADD_FILE" },
			{ FILE_APPEND_DATA, L"FILE_APPEND_DATA" },
			{ FILE_ADD_SUBDIRECTORY, L"FILE_ADD_SUBDIRECTORY" },
			{ FILE_CREATE_PIPE_INSTANCE, L"FILE_CREATE_PIPE_INSTANCE" },
			{ FILE_READ_EA, L"FILE_READ_EA" },
			{ FILE_WRITE_EA, L"FILE_WRITE_EA" },
			{ FILE_EXECUTE, L"FILE_EXECUTE" },
			{ FILE_TRAVERSE, L"FILE_TRAVERSE" },
			{ FILE_DELETE_CHILD, L"FILE_DELETE_CHILD" },
			{ FILE_READ_ATTRIBUTES, L"FILE_READ_ATTRIBUTES" },
			{ FILE_WRITE_ATTRIBUTES, L"FILE_WRITE_ATTRIBUTES" },
			{ FILE_ALL_ACCESS, L"FILE_ALL_ACCESS" },
		};
		for (i = 0, c = sizeof(perms) / sizeof(PermAndStr); i < c; i++) {
			if ((access & perms[i].perm) == perms[i].perm) {
				result += perms[i].str;
				result += L" ";
			}
		}
	}

	if (result.length() == 0) {
		WCHAR permStr[1024];
		_snwprintf_s(permStr, _countof(permStr), _TRUNCATE, L"0x%x %d", access, access);
		result = L"UNKNOWN (";
		result += permStr;
		result += L")";
	}
	TCHAR* cResult = new TCHAR[result.length() + 1];
	wcsncpy_s(cResult, MAX_PATH, result.c_str(), _TRUNCATE);
	
	return cResult;
}

//
// Return a string of the specified CreationDisposition
// DON'T DELETE THE RETURNED STRING
//
static TCHAR* getCreationDispositionString(DWORD creatDisp) {
	
	PermAndStr cds[] = {
		{ CREATE_ALWAYS, L"CREATE_ALWAYS" },
		{ CREATE_NEW, L"CREATE_NEW" },
		{ OPEN_ALWAYS, L"OPEN_ALWAYS" },
		{ OPEN_EXISTING, L"OPEN_EXISTING" },
		{ TRUNCATE_EXISTING, L"TRUNCATE_EXISTING" }
	};
	int i, c;
	for (i = 0, c = sizeof(cds) / sizeof(PermAndStr); i < c; i++) {
		if (creatDisp == cds[i].perm) {
			return cds[i].str;
		}
	}
	return L"UNKNOWN";
}


typedef struct {
	int i;
	WCHAR *str;
} IntAndString;

static TCHAR* GetShowWindowParamString(int nCmdShow) {
	IntAndString paramToString[] = {
		{ SW_FORCEMINIMIZE, L"SW_FORCEMINIMIZE" },
		{ SW_HIDE, L"SW_HIDE" },
		{ SW_MAXIMIZE, L"SW_MAXIMIZE"} ,
		{ SW_MINIMIZE, L"SW_MINIMIZE" },
		{ SW_RESTORE, L"SW_RESTORE" }, 
		{ SW_SHOW, L"SW_SHOW" },
		{ SW_SHOWDEFAULT, L"SW_SHOWDEFAULT" },
		{ SW_SHOWMAXIMIZED, L"SW_SHOWMAXIMIZED" },
		{ SW_SHOWMINIMIZED, L"SW_SHOWMINIMIZED" },
		{ SW_SHOWMINNOACTIVE, L"SW_SHOWMINNOACTIVE" },
		{ SW_SHOWNA, L"SW_SHOWNA" },
		{ SW_SHOWNOACTIVATE, L"SW_SHOWNOACTIVATE" },
		{ SW_SHOWNORMAL, L"SW_SHOWNORMAL" }
	};

	for (int i = 0; i < (sizeof(paramToString) / sizeof(IntAndString)); i++) {
		if (nCmdShow == paramToString[i].i) {
			return paramToString[i].str;
		}
	}

	return L"Unknown";
}

//
// Parse a string returned by the CDM_GETFILEPATH message and transform it into a list of files
// examples of such strings:
// 1 file selected: C:\Program Files\Common Files\Java\Update\Base Images\j2re1.4.2-b28\core1.zip
// several files: C:\cygwin\bin\"convert-gdbm" "clearn.exe" "cmp.exe" "comm.exe"

static vector<wstring>* ParseDialogFileList(WCHAR* s) {
    if (s == NULL) {
        return NULL;
    }

    // Let's test for the easy case of single file
    if (NULL == wcschr(s, L'"')) {
        vector<wstring>* v = new vector<wstring>();
        v->push_back(s);
        return v;
    }

    // Let's make a copy of the original string, we are going to mess with it
    WCHAR *str = new WCHAR[wcslen(s) + 1];
    wcsncpy_s(str, wcslen(s)+1, s, _TRUNCATE);

    vector<wstring>* v = new vector<wstring>();
    WCHAR *start = wcschr(str, L'"'); 
    while (start != NULL) {
        *start = L'\0'; start++;
        WCHAR *dir = str;

        WCHAR *end = wcschr(start, L'"'); 
        *end = L'\0';

        WCHAR path[MAX_PATH];
        if ((wcslen(dir) + wcslen(start)) < MAX_PATH) {
            _snwprintf_s(path, _countof(path), _TRUNCATE, L"%s%s", dir, start);
            v->push_back(path);
        } else {
            CEDPMgr& edpMgr = CEDPMgr::GetInstance();
			edpMgr.GetCELog().Log(CELOG_DEBUG, L"ParseDialogFileList: file name too long, ignored\n");
        }
        start = wcschr(end + 1, L'"');
    }

	delete [] str;
    return v;
}

#endif //_SYSUTILS_H_

//--------------------- End of the file -------------------------------------
