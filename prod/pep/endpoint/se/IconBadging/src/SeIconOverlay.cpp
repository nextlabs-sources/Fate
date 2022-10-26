// SeIconOverlay.cpp : Implementation of CSeIconOverlay

#include "stdafx.h"
#include "SeIconOverlay.h"
#include "nl_sysenc_lib.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

extern HINSTANCE    g_hInstance;
static WCHAR        g_wzDll[2*MAX_PATH+1] = {0};

static BOOL GetIconDllPath();

#define UPDATE_EXPIRE_TIME  10 // seconds


// CSeIconOverlay
STDMETHODIMP CSeIconOverlay::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int* pIndex, DWORD* pdwFlags)
{
    if(!GetIconDllPath()) return E_FAIL;

	// The Icon File Path -- it's this DLL
    wcsncpy_s(pwszIconFile, cchMax, g_wzDll, _TRUNCATE);

	// Use first icon in the resource
	*pIndex     = 0;
	*pdwFlags   = ISIOI_ICONFILE|ISIOI_ICONINDEX;
	return S_OK;
}

// IShellIconOverlayIdentifier::GetPriority
// returns the priority of this overlay 0 being the highest. 
STDMETHODIMP CSeIconOverlay::GetPriority(int* pPriority)
{
	// we want highest priority 
	*pPriority=0;
	return S_OK;
}

// IShellIconOverlayIdentifier::IsMemberOf
// Returns whether the object should have this overlay or not 
STDMETHODIMP CSeIconOverlay::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
    std::wstring strFile = pwszPath;

    UNREFERENCED_PARAMETER(dwAttrib);

    if(NULL==pwszPath || L'\0'==pwszPath[0])
        return S_FALSE;

    if(boost::algorithm::iends_with(strFile, L".nxl"))
    {
        return S_FALSE;
    }

	BOOL bDirectory = PathIsDirectory(pwszPath);
    //if(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
	if(bDirectory)
    {
        if(g_drm.IsEncryptedDirectory(pwszPath)) return S_OK;
    }
    else
    {
        if(SE_IsEncrypted(pwszPath)) return S_OK;
    }


	return S_FALSE;
}

BOOL GetIconDllPath()
{
    if(L'\0' != g_wzDll[0])
        return TRUE;

    if(0 == ::GetModuleFileNameW((HMODULE)g_hInstance, g_wzDll, 2*MAX_PATH))
        return FALSE;

    return TRUE;
}

// CDRMPathReader
////////////////////////////////////////////////////////////////////////////////////////////
CDRMPathReader::CDRMPathReader()
{
    memset(&m_ftWrite, 0, sizeof(FILETIME));
    memset(&m_stLastUpdate, 0, sizeof(SYSTEMTIME));
    ::InitializeCriticalSection(&m_cs);
}

CDRMPathReader::~CDRMPathReader()
{
    ::DeleteCriticalSection(&m_cs);
}

BOOL CDRMPathReader::IsEncryptedDirectory(_In_ LPCWSTR wzDir)
{
    int  i    = 0;
    std::wstring wstrPath(wzDir);

    UpdateDRMLists();

    if(!boost::algorithm::iends_with(wstrPath, L"\\")) wstrPath += L"\\";
    ::EnterCriticalSection(&m_cs);
    for(i=0; i<(int)m_vecDRMs.size(); i++)
    {
        if(boost::algorithm::istarts_with(wstrPath, m_vecDRMs[i].c_str()))
        {
            ::LeaveCriticalSection(&m_cs);
            return TRUE;
        }
    }
    ::LeaveCriticalSection(&m_cs);

    return FALSE;
}

BOOL CDRMPathReader::GetDRMConfigFile()
{
    LONG    lResult = 0;
    HKEY    hKey    = NULL;
    WCHAR   wzPath[MAX_PATH+1] = {0};
    DWORD   dwPathLength = MAX_PATH*sizeof(WCHAR);
    DWORD   dwType = REG_SZ;

    memset(wzPath, 0, sizeof(WCHAR)*(MAX_PATH+1));
    lResult = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Enterprise DLP\\System Encryption", 0, KEY_READ, &hKey);
    if(ERROR_SUCCESS != lResult)
        return FALSE;

    lResult = ::RegQueryValueExW(hKey, L"InstallDir", 0, &dwType, (LPBYTE)wzPath, &dwPathLength);
    if(ERROR_SUCCESS == lResult)
    {
        m_strConfigFile = wzPath;
        if(!boost::algorithm::iends_with(m_strConfigFile, L"\\")) m_strConfigFile += L"\\";
        m_strConfigFile += L"config\\SystemEncryption.cfg";
    }

    ::RegCloseKey(hKey);
    return (ERROR_SUCCESS==lResult)?TRUE:FALSE;
}

BOOL CDRMPathReader::IsFileChanged()
{
    BOOL            bRet    = FALSE;
    FILETIME        ftWrite = {0};
    HANDLE hFile = INVALID_HANDLE_VALUE;

    hFile = ::CreateFileW(m_strConfigFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile)
        goto _exit;

    bRet = ::GetFileTime(hFile, NULL, NULL, &ftWrite);
    if(!bRet) goto _exit;

    if(0 != CompareFileTime(&m_ftWrite, &ftWrite))
    {
        m_ftWrite.dwHighDateTime = ftWrite.dwHighDateTime;
        m_ftWrite.dwLowDateTime  = ftWrite.dwLowDateTime;
        bRet = TRUE;
    }

_exit:
    CloseHandle(hFile);
    return bRet;
}

BOOL CDRMPathReader::NeedToUpdate()
{
    SYSTEMTIME      stCurrentTime = {0};
    WORD            wEclapse      = 0;

    ::GetSystemTime(&stCurrentTime);
    if(stCurrentTime.wYear != m_stLastUpdate.wYear
        || stCurrentTime.wMonth != m_stLastUpdate.wMonth
        || stCurrentTime.wDay != m_stLastUpdate.wDay
        || stCurrentTime.wHour != m_stLastUpdate.wHour
        )
        return TRUE;

    if(stCurrentTime.wMinute < m_stLastUpdate.wMinute)
    {
        return TRUE;
    }
    else
    {
        wEclapse = stCurrentTime.wSecond + ((stCurrentTime.wMinute>m_stLastUpdate.wMinute)?60:0) - m_stLastUpdate.wSecond;
        if(wEclapse > UPDATE_EXPIRE_TIME)
        {
            memcpy(&m_stLastUpdate, &stCurrentTime, sizeof(SYSTEMTIME));
            return TRUE;
        }
    }

    return FALSE;
}

BOOL CDRMPathReader::UpdateDRMLists()
{
    std::ifstream ifile;
    BOOL          bFirstLine = TRUE;


    if(0==m_strConfigFile.length() && !GetDRMConfigFile()) return FALSE;
    //if(!NeedToUpdate()) return TRUE;
    if(!IsFileChanged()) return TRUE;

    ifile.open(m_strConfigFile.c_str());
    if(!ifile.is_open()) return FALSE;

    ::EnterCriticalSection(&m_cs);
    m_vecDRMs.clear();
    while(!ifile.eof())
    {
        std::string strLine;
        std::getline(ifile, strLine);
        if(strLine.length() < 7)
        {
            if(bFirstLine) bFirstLine = FALSE;
            continue;
        }
        if(bFirstLine)
        {
            bFirstLine = FALSE;
            // Remove UTF-8 header
            strLine = strLine.substr(3);
        }

        if(boost::algorithm::istarts_with(strLine, "DRMDir=")) 
		{
			strLine = strLine.substr(7);
			if(strLine.empty()) continue;

			std::wstring wstrDRM(strLine.begin(), strLine.end());
			if(!boost::algorithm::iends_with(wstrDRM, L"\\")) wstrDRM += L"\\";
			m_vecDRMs.push_back(wstrDRM);
		}
		else if (boost::algorithm::istarts_with(strLine, "DRMDirFW=")) 
		{
			strLine = strLine.substr(9);
			if(strLine.empty()) continue;

			std::wstring wstrDRM(strLine.begin(), strLine.end());
			if(!boost::algorithm::iends_with(wstrDRM, L"\\")) wstrDRM += L"\\";
			m_vecDRMs.push_back(wstrDRM);
		}
		else
		{
			continue;
		}
    }
    ::LeaveCriticalSection(&m_cs);

    return TRUE;
}