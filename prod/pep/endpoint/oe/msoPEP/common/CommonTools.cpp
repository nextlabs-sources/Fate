#include "stdafx.h"
#include "CommonTools.h"

#include <shellapi.h>

#include "boost\algorithm\string.hpp"

#include "log.h"
#include "Hook.h"

#ifndef NLUNITTEST
HRESULT SinkAppEvent(_Inout_ IUnknown* pEventDispatcher, _Inout_opt_ IUnknown* pEventReceiver, _In_ const IID& iid, _Inout_ LPDWORD pDwSinkCookie)
{NLONLY_DEBUG
    HRESULT hr = E_FAIL;
    if (NULL != pEventReceiver)
    {
        DWORD dwSinkCookie = 0;
        hr = AtlAdvise(pEventDispatcher, pEventReceiver, iid, &dwSinkCookie);
        if (NULL != pDwSinkCookie)
        {
            *pDwSinkCookie = dwSinkCookie;
        }
    }
    return hr;
}

HRESULT GetAttachments(CComPtr<IDispatch> spDisp, struct Outlook::Attachments * * Attachments)
{NLONLY_DEBUG
    HRESULT hr = E_FAIL;
    if (NULL != spDisp)
    {
        CComPtr<Outlook::_MailItem> spCurMailItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__MailItem, (void**)&spCurMailItem);
        if (SUCCEEDED(hr) && spCurMailItem)
        {
            spCurMailItem->get_Attachments(Attachments);
            return hr;
        }
        CComPtr<Outlook::_AppointmentItem> spCurAppItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__AppointmentItem, (void**)&spCurAppItem);
        if (SUCCEEDED(hr) && spCurAppItem)
        {
            hr = spCurAppItem->get_Attachments(Attachments);

            return hr;
        }
        CComPtr<Outlook::_TaskItem> spCurTaskItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__TaskItem, (void**)&spCurTaskItem);
        if (SUCCEEDED(hr) && spCurTaskItem)
        {
            hr = spCurTaskItem->get_Attachments(Attachments);

            return hr;
        }
        CComPtr<Outlook::_TaskRequestItem> spCurTaskReqItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__TaskRequestItem, (void**)&spCurTaskReqItem);
        if (SUCCEEDED(hr) && spCurTaskReqItem)
        {
            hr = spCurTaskReqItem->get_Attachments(Attachments);

            return hr;
        }
        CComPtr<Outlook::_MeetingItem> spCurMeetItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__MeetingItem, (void**)&spCurMeetItem);
        if (SUCCEEDED(hr) && spCurMeetItem)
        {
            hr = spCurMeetItem->get_Attachments(Attachments);

            return hr;
        }
 #if ((defined WSO2K16) || (defined WSO2K13) || (defined WSO2K10))
        CComPtr<Outlook::_SharingItem> spCurShareItem = 0;
        hr = spDisp->QueryInterface(Outlook::IID__SharingItem, (void**)&spCurShareItem);
        if (SUCCEEDED(hr) && spCurShareItem)
        {
            hr = spCurShareItem->get_Attachments(Attachments);

            return hr;
        }
#endif
    }
    return hr;
}
#endif /*NLUNITTEST*/

bool IsFullLocalPath(_In_ const std::wstring& kwstrPath, bool bIsFolder)
{
    bool bIsFullPath = false;
    // full local path is start with: c:\ **** \ ***
    size_t stLength = kwstrPath.length();
    if (3 <= stLength)
    {
        if ((isalpha(kwstrPath[0])) &&
            (L':' == kwstrPath[1])  &&
            (L'\\' == kwstrPath[2])
           )
        {
            if (bIsFolder)
            {
                bIsFullPath = (L'\\' == kwstrPath[stLength - 1]);   // folder must end with "\"
            }
            else
            {
                bIsFullPath = (4 <= stLength);
            }
        }
    }

    return bIsFullPath;
}

bool IsFileInSpecifyFolder(_In_ const std::wstring& kwstrFilePath, _In_ const std::wstring& kwstrFolderPath)
{
    return kwstrFolderPath.empty() ? false : (0 == _wcsnicmp(kwstrFilePath.c_str(), kwstrFolderPath.c_str(), kwstrFolderPath.length()));
}

std::wstring NLNewGUID()
{
    wchar_t wszGuid[65] = { 0 };
    GUID guid = { 0 };
    HRESULT hr = ::CoCreateGuid(&guid);
    if (SUCCEEDED(hr))
    {
        swprintf_s(wszGuid, 64, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}"
            , guid.Data1
            , guid.Data2
            , guid.Data3
            , guid.Data4[0], guid.Data4[1]
            , guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
            , guid.Data4[6], guid.Data4[7]
            );
        return wszGuid;
    }
    return L"";
}

std::wstring GetSuffixFromFileName(_In_ const std::wstring& wstrFileName)
{
    std::wstring wstrRet = L"";
    std::wstring::size_type nPos = wstrFileName.rfind(L".");
    if (nPos != std::wstring::npos)
    {
        wstrRet = wstrFileName.substr(nPos + 1, wstrFileName.size() - nPos - 1);
    }
    return wstrRet;
}

std::wstring GetFilePath(const std::wstring& wstrFilePath)
{
    std::wstring wstrRet = L"";
    std::wstring::size_type nPos = wstrFilePath.rfind(L"\\");
    if (nPos != std::wstring::npos)
    {
        wstrRet = wstrFilePath.substr(0, nPos + 1);
    }
    return wstrRet;
}

std::wstring GetFileName(const std::wstring& wstrFilePath)
{
    std::wstring wstrRet = L"";
    std::wstring::size_type nPos = wstrFilePath.rfind(L"\\");
    if (nPos != std::wstring::npos)
    {
        wstrRet = wstrFilePath.substr(nPos + 1, wstrFilePath.length() - 1);
    }
    return wstrRet;
}

std::wstring GetFileNameWithoutSuffix(_In_ const std::wstring& wstrFilePath)
{
    std::wstring::size_type nPos1 = wstrFilePath.rfind(L"\\");
    size_t stStart = 0;
    if (nPos1 != std::wstring::npos)
    {
        stStart = nPos1 + 1;
    }
    size_t stLen = wstrFilePath.length() - stStart;
    std::wstring::size_type nPos2 = wstrFilePath.rfind(L".");
    if (nPos2 != std::wstring::npos)
    {
        stLen = nPos2 - stStart;
    }
    return wstrFilePath.substr(stStart, stLen);
}

std::wstring CreateAUniqueSubFolder(_In_ const std::wstring& kwstrFolder)
{
    std::wstring wstrUniqueFolder = L"";
    BOOL bRet = FALSE;
    for (int i = 0; i < 5; ++i)
    {
        wstrUniqueFolder = kwstrFolder + NLNewGUID() + L"\\";
        ::SetLastError(ERROR_SUCCESS);
        bRet = ::CreateDirectoryW(wstrUniqueFolder.c_str(), NULL);
        DWORD dwLastError = ::GetLastError();
        if ((!bRet) && (ERROR_ALREADY_EXISTS == dwLastError))
        {
            continue;
        }
        else
        {
            NLPRINT_DEBUGVIEWLOG(L"Create unique folder:[%s] under:[%s] [%s] with last error:[%d]\n", wstrUniqueFolder.c_str(), kwstrFolder.c_str(), bRet ? L"Success" : L"Failed", dwLastError);
            break;
        }
    }
    return bRet ? wstrUniqueFolder : L"";
}

// 0: create time, 1: modify time, 2: access time
bool GetFileSystemTime(_In_ const std::wstring& kwstrFileFullPath, _Inout_ SYSTEMTIME* pStuSysTime, const int knTimeType)
{
    if (NULL == pStuSysTime)
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!parameters error, pStuSysTime is NULL\n");
        return false;
    }

    if ((0 > knTimeType) || (3 < knTimeType))
    {
        NLPRINT_DEBUGVIEWLOG(L"!!!parameters error, nTimeType:[%d] is wrong\n", knTimeType);
        return false;
    }

    if (kwstrFileFullPath.empty() || (!PathFileExistsW(kwstrFileFullPath.c_str())))
    {
        NLPRINT_DEBUGVIEWLOG(L"Get file create time failed, file path:[%s] is empty or not exist\n", kwstrFileFullPath.c_str());
        return false;
    }

    bool bRet = false;
    WIN32_FIND_DATA FindFileData = { 0 };
    ::SetLastError(ERROR_SUCCESS);
    HANDLE hFind = FindFirstFile(kwstrFileFullPath.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        FindClose(hFind);

        FILETIME* pstuFileTime = NULL;
        if (0 == knTimeType)
        {
            pstuFileTime = &FindFileData.ftCreationTime;
        }
        else if (1 == knTimeType)
        {
            pstuFileTime = &FindFileData.ftLastWriteTime;
        }
        else if (2 == knTimeType)
        {
            pstuFileTime = &FindFileData.ftLastAccessTime;
        }
        if (NULL != pstuFileTime)
        {
            bRet = FileTimeToSystemTime(pstuFileTime, pStuSysTime) ? true : false;    // warning C4800
        }
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"Failed to get file:[%s] attribute with last error:[%d]\n", kwstrFileFullPath.c_str(), ::GetLastError());
    }
    return bRet;
}

int CompareSysTime(_In_ const SYSTEMTIME& kstuFirstSysTime, _In_ const SYSTEMTIME& kstuSecondSysTime)
{
    // length = 5 + 2*5 + 3 + 6 = 24
    // kstuFirstSysTime.wYear;          // 5
    // kstuFirstSysTime.wMonth;         // 2
    // kstuFirstSysTime.wDay;           // 2
    // kstuFirstSysTime.wHour;          // 2
    // kstuFirstSysTime.wMinute;        // 2
    // kstuFirstSysTime.wSecond;        // 2
    // kstuFirstSysTime.wMilliseconds;  // 3

    const int nLen = 32;
    wchar_t wszFirstSysTime[nLen] = { 0 };

    swprintf_s(wszFirstSysTime, nLen - 1, L"%05d-%02d-%02d-%02d-%02d-%02d-%03d", kstuFirstSysTime.wYear, kstuFirstSysTime.wMonth, kstuFirstSysTime.wDay,
        kstuFirstSysTime.wHour, kstuFirstSysTime.wMinute, kstuFirstSysTime.wSecond, kstuFirstSysTime.wMilliseconds);

    wchar_t wszSecondSysTime[nLen] = { 0 };
    swprintf_s(wszSecondSysTime, nLen - 1, L"%05d-%02d-%02d-%02d-%02d-%02d-%03d", kstuSecondSysTime.wYear, kstuSecondSysTime.wMonth, kstuSecondSysTime.wDay,
        kstuSecondSysTime.wHour, kstuSecondSysTime.wMinute, kstuSecondSysTime.wSecond, kstuSecondSysTime.wMilliseconds);

    int nRet = _wcsicmp(wszFirstSysTime, wszSecondSysTime);
    NLPRINT_DEBUGVIEWLOG(L"Compare system time [%s] with [%s] as [%d]\n", wszFirstSysTime, wszSecondSysTime, nRet);

    return nRet;
}

std::wstring ComvertSysTimeToString(_In_ const SYSTEMTIME& kstuSysTime)
{
    // length = 5 + 2*5 + 3 + 6 = 24
    // kstuFirstSysTime.wYear;          // 5
    // kstuFirstSysTime.wMonth;         // 2
    // kstuFirstSysTime.wDay;           // 2
    // kstuFirstSysTime.wHour;          // 2
    // kstuFirstSysTime.wMinute;        // 2
    // kstuFirstSysTime.wSecond;        // 2
    // kstuFirstSysTime.wMilliseconds;  // 3

    const int nLen = 32;
    wchar_t wszSysTime[nLen] = { 0 };
    swprintf_s(wszSysTime, nLen - 1, L"%05d-%02d-%02d-%02d-%02d-%02d-%03d", kstuSysTime.wYear, kstuSysTime.wMonth, kstuSysTime.wDay,
        kstuSysTime.wHour, kstuSysTime.wMinute, kstuSysTime.wSecond, kstuSysTime.wMilliseconds);

    return wszSysTime;
}

bool DeleteFolderOrFile(_In_ const std::wstring& kwstrPath, _In_ const bool kbAllowToRecycleBin)
{
    if (kwstrPath.empty())
    {
        return FALSE;
    }

    size_t stSrcPathLen = kwstrPath.length();
    wchar_t* pwchSrcFolder = new wchar_t[stSrcPathLen + 2]; // For folder need end with two L'\0', this is why need +2.
    wmemset(pwchSrcFolder, 0, stSrcPathLen + 2);
    wcsncpy_s(pwchSrcFolder, stSrcPathLen + 1, kwstrPath.c_str(), _TRUNCATE);

    SHFILEOPSTRUCT FileOp;
    ZeroMemory(&FileOp, sizeof(SHFILEOPSTRUCT));
    FileOp.fFlags |= FOF_SILENT;         // do not show schedule
    FileOp.fFlags |= FOF_NOERRORUI;      // do not show error report
    FileOp.fFlags |= FOF_NOCONFIRMATION; // delete directly without confirmation

    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = pwchSrcFolder;        // The delete folders, split by two '\0'
    FileOp.pTo = NULL;

    if (kbAllowToRecycleBin)
    {
        FileOp.fFlags |= FOF_ALLOWUNDO; // delete to recycle bin
    }

    // Delete, return 0 is success.
    int nRet = SHFileOperation(&FileOp);
    delete[] pwchSrcFolder;
    pwchSrcFolder = NULL;

    return (0 == nRet);
}

std::wstring NLGetLongFilePathEx(_In_ const std::wstring& kwstrFileShortPath)
{
    std::wstring wstrFileLongPath = NLGetLongFilePath(kwstrFileShortPath);
    if (wstrFileLongPath.empty())
    {
        return kwstrFileShortPath;
    }
    return wstrFileLongPath;
}

std::wstring NLGetLongFilePath(_In_ const std::wstring& kwstrFileShortPath)
{
    /** check parameters */
    if (kwstrFileShortPath.empty())
    {
        return L"";
    }

    /** Get long path */
    const int knLen = 1023;
    wchar_t* pwchLongTempPath = new wchar_t[knLen + 1];
    wmemset(pwchLongTempPath, 0, knLen);

    ::SetLastError(ERROR_SUCCESS);
    DWORD dwBufferLength = GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, knLen);
    if ((knLen) < dwBufferLength)
    {
        /** the buffer is too small and get long path again */
        delete[] pwchLongTempPath;

        pwchLongTempPath = new wchar_t[dwBufferLength + 1];
        dwBufferLength = GetLongPathNameW(kwstrFileShortPath.c_str(), pwchLongTempPath, dwBufferLength);
    }

    if (0 == dwBufferLength)
    {
        if (PathFileExistsW(kwstrFileShortPath.c_str()))    // Most error is cause by path not exist and no need record
        {
            NLPRINT_DEBUGVIEWLOG(L"!!!Error, get long file path from:[%s] failed with last error:[%d]\n", kwstrFileShortPath.c_str(), ::GetLastError());
        }
    }

    std::wstring wstrFileLongPath = L"";
    if (NULL != pwchLongTempPath)
    {
        wstrFileLongPath = pwchLongTempPath;
        delete[] pwchLongTempPath;
    }

    return wstrFileLongPath;
}

bool GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
    static DWORD sMajor = 0;
    static DWORD sMinor = 0;

    if (sMajor == 0 && sMinor == 0)
    {
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
        // If that fails, try using the OSVERSIONINFO structure.
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

#pragma warning( push )
#pragma warning( disable: 4996 )    // Function GetVersionEx is deprecated, we can use IsWindows10OrGreater to instead, but this only can used in win10+ OS
        bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
        if (!bOsVersionInfoEx)
        {
            // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            if (!GetVersionEx((OSVERSIONINFO *)&osvi))
            {
                return false;
            }
        }
#pragma warning( pop )

        sMajor = osvi.dwMajorVersion;
        sMinor = osvi.dwMinorVersion;

    }

    //5,0 win2k, 5,1 winxp
    dwMajor = sMajor;
    dwMinor = sMinor;

	DP((L"GetOsVersion dwMajor=%d, dwMinor=%d\n", dwMajor, dwMinor));

    return true;
}

// versions: 6.2
bool IsWin10(void)
{
    static DWORD dwXPMajor = 0;
    static DWORD dwXPMinor = 0;
    if ((6 < dwXPMajor) || ((6 == dwXPMajor) && (2 <= dwXPMinor)))
    {
        return true;
    }
    else if ((0 != dwXPMajor) || (0 != dwXPMinor))
    {
        return false;
    }

    if (GetOSInfo(dwXPMajor, dwXPMinor) && ((6 < dwXPMajor) || ((6 == dwXPMajor) && (2 <= dwXPMinor))))
    {
        return true;
    }
    return false;
}

std::wstring GetDefaultShellFolderCache()
{
	std::wstring wstrDefaultShellFolder= GetShellFolder(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders", L"Cache");
    if (!wstrDefaultShellFolder.empty())
    {
		const std::wstring wstrUserProfileKey = L"%USERPROFILE%";
		int nUserProfilePos= wstrDefaultShellFolder.find(wstrUserProfileKey);
		if (nUserProfilePos==0)
		{
			const wchar_t* pEnvUserProfile = _wgetenv(L"USERPROFILE");
			if (NULL!=pEnvUserProfile)
			{
                wstrDefaultShellFolder.replace(0, wstrUserProfileKey.length(), pEnvUserProfile);
			}
		}     
    }
	else
	{
		const wchar_t* pEnvUserProfile = _wgetenv(L"USERPROFILE");
		if (NULL!=pEnvUserProfile)
		{
			std::wstring wstrCachePath = IsWin10() ? L"\\AppData\\Local\\Microsoft\\Windows\\INetCache" : L"\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files";
			wstrDefaultShellFolder= pEnvUserProfile + wstrCachePath;
		}
	}

	return wstrDefaultShellFolder;
}

std::wstring GetContentWordFolderFromOutlookTempPath()
{
    std::wstring wstrContentWordFolder = GetShellFolder(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", L"Cache");
    
	if (wstrContentWordFolder.empty()){	
         wstrContentWordFolder = GetDefaultShellFolderCache();
	}

	if (!wstrContentWordFolder.empty())
	{
		size_t stLength = wstrContentWordFolder.length();
		if (L'\\' != wstrContentWordFolder[stLength-1])
		{
			wstrContentWordFolder += L"\\";
		}
		wstrContentWordFolder += L"content.word\\";
	}
   
    return wstrContentWordFolder;
}

std::wstring GetContentMsoFolderFromOutlookTempPath()
{
	std::wstring wstrContentMsoFolder = GetShellFolder(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", L"Cache");
	
	if (wstrContentMsoFolder.empty()){
		 wstrContentMsoFolder = GetDefaultShellFolderCache();
	}

	if (!wstrContentMsoFolder.empty())
	{
		size_t stLength = wstrContentMsoFolder.length();
		if (L'\\' != wstrContentMsoFolder[stLength-1])
		{
			wstrContentMsoFolder += L"\\";
		}
		wstrContentMsoFolder += L"content.mso\\";
	}
   
	return wstrContentMsoFolder;
}

EMNLOE_IMAGETYPE GetOutlookImageType()
{
	return (sizeof(int*)==8) ? emImageTypex64 : emImageTypex86;
}

EMNLOE_IMAGETYPE GetOSImageType()
{
	size_t nPointSize = sizeof(int*);
	if (nPointSize==8)
	{
		return emImageTypex64;
	}
	else
	{
		BOOL bWow64Process = FALSE;
	    IsWow64Process(GetCurrentProcess(), &bWow64Process);
		if (bWow64Process)
		{
			return emImageTypex64;
		}
		else
		{
			return emImageTypex86;
		}
	}
}

EMNLOE_PLATEFORMVERSION GetPlateformVersion()
{
	return IsWin10() ? emPlateformWin10 : emPlateformWin7;
}

std::wstring GetShellFolder(HKEY hRootKey, const wchar_t* wszPath, const wchar_t* wszName)
{
	std::wstring wstrShellFoler;
	HKEY hKeyShellFolder = NULL;
	LONG lResult = RegOpenKeyExW(hRootKey, wszPath, 0, KEY_ALL_ACCESS, &hKeyShellFolder);
	if((ERROR_SUCCESS == lResult) && (hKeyShellFolder!=NULL))
	{
		wchar_t wszShellFolder[MAX_PATH+1]={0};
        DWORD dwFolderLen = MAX_PATH * sizeof(wszShellFolder[0]);
		lResult = RegQueryValueExW(hKeyShellFolder, wszName, 0, NULL,(LPBYTE)wszShellFolder, &dwFolderLen);
		if (lResult==ERROR_SUCCESS)
		{
           wstrShellFoler = wszShellFolder;  
		}

		RegCloseKey(hKeyShellFolder);
		hKeyShellFolder = NULL;
	}

	return wstrShellFoler;
}

std::wstring GetNLTempFileFolder(_In_ const EMNLOE_TEMPLOCATION kemTempLocation)
{
    std::wstring wstrOETempParentFolderName = L"";
    std::wstring wstrOETempSubFolderName = L"";
    if (emTempLocationDump == kemTempLocation)
    {
        wstrOETempParentFolderName = L"NLOutlookEnforcer";
        wstrOETempSubFolderName = L"Dumps";
    }
    else if (emTempLocationOverTimeLog == kemTempLocation)
    {
        wstrOETempParentFolderName = L"NLOutlookEnforcer";
        wstrOETempSubFolderName = L"OvertimeLogs";
    }
    else
    {
        return L"";
    }

    std::wstring wstrTempFolder = L"";
    wchar_t wszDirAppData[2048] = { 0 };
    if (SHGetSpecialFolderPath(NULL, wszDirAppData, CSIDL_LOCAL_APPDATA, FALSE))
    {
        wstrTempFolder = wszDirAppData;
        wstrTempFolder += L"\\Temp\\";
        CreateDirectoryW(wstrTempFolder.c_str(), NULL);

        wstrTempFolder += wstrOETempParentFolderName + L"\\";
        CreateDirectoryW(wstrTempFolder.c_str(), NULL);

        wstrTempFolder += wstrOETempSubFolderName + L"\\";
        CreateDirectoryW(wstrTempFolder.c_str(), NULL);
    }
    return wstrTempFolder;
}

unsigned int GetItemCountInTheFolder(_In_ const std::wstring& kwstrFolder)
{
    unsigned int unItemCount = 0;
    if (kwstrFolder.empty())
    {
        NLPRINT_DEBUGVIEWLOG(L"Get file from failed, the input folder path is empty\n");
        return unItemCount;
    }

    std::wstring wstrStanderFolder = kwstrFolder;
    size_t stLen = wstrStanderFolder.length();
    if (L'\\' != wstrStanderFolder[stLen - 1])
    {
        wstrStanderFolder += L"\\";
    }

    if (!PathFileExistsW(wstrStanderFolder.c_str()))
    {
        NLPRINT_DEBUGVIEWLOG(L"Get file from:[%s] failed, the input folder path not exist\n", wstrStanderFolder.c_str());
        return unItemCount;
    }

    std::wstring wstrFilter = wstrStanderFolder + L"*";
    WIN32_FIND_DATA FindFileData;

    ::SetLastError(ERROR_SUCCESS);
    HANDLE hFind = FindFirstFile(wstrFilter.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if (0 != wcscmp(L".", FindFileData.cFileName) && 0 != wcscmp(L"..", FindFileData.cFileName))
            {
                ++unItemCount;
            }
        } while (FindNextFile(hFind, &FindFileData));
        FindClose(hFind);
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"Fail to enum file in:[%s] with last error:[%d]\n", kwstrFolder.c_str(), ::GetLastError());
    }
    return unItemCount;
}

void DeleteEmptyFolderInTheFolder(_In_ const std::wstring& kwstrFolder)
{
    if (kwstrFolder.empty())
    {
        NLPRINT_DEBUGVIEWLOG(L"Get file from failed, the input folder path is empty\n");
		return;
    }

    std::wstring wstrStanderFolder = kwstrFolder;
    size_t stLen = wstrStanderFolder.length();
    if (L'\\' != wstrStanderFolder[stLen - 1])
    {
        wstrStanderFolder += L"\\";
    }

    if (!PathFileExistsW(wstrStanderFolder.c_str()))
    {
        NLPRINT_DEBUGVIEWLOG(L"Get file from:[%s] failed, the input folder path not exist\n", wstrStanderFolder.c_str());
    }

    std::wstring wstrFilter = wstrStanderFolder + L"*";
    WIN32_FIND_DATA FindFileData;

    ::SetLastError(ERROR_SUCCESS);
    HANDLE hFind = FindFirstFile(wstrFilter.c_str(), &FindFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if (0 != wcscmp(L".", FindFileData.cFileName) && 0 != wcscmp(L"..", FindFileData.cFileName))
            {
                if (FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
                {
                    RemoveDirectory((wstrStanderFolder + FindFileData.cFileName).c_str());
                }
            }
        } while (FindNextFile(hFind, &FindFileData));
        FindClose(hFind);
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"Fail to enum file in:[%s] with last error:[%d]\n", kwstrFolder.c_str(), ::GetLastError());
    }
}

std::wstring ConverSysTimeToString(_In_ const SYSTEMTIME& kstuSysTime)
{
    const int nLen = 32;
    wchar_t wszSysTime[nLen] = { 0 };
    swprintf_s(wszSysTime, nLen - 1, L"%05d-%02d-%02d-%02d-%02d-%02d-%03d", kstuSysTime.wYear, kstuSysTime.wMonth, kstuSysTime.wDay,
        kstuSysTime.wHour, kstuSysTime.wMinute, kstuSysTime.wSecond, kstuSysTime.wMilliseconds);

    return wszSysTime;
}

DWORD NLGetAbs(DWORD dwFirst, DWORD dwSecond)
{
    if (dwFirst >= dwSecond)
    {
        return dwFirst - dwSecond;
    }
    else
    {
        return dwSecond - dwFirst;
    }
}

DWORD StringToDword(_In_ const std::wstring& kwstrIn)
{
    static const DWORD kdwRax = 10;

    // First get the legal string length
    size_t stLegalLen = 0;
    for (stLegalLen = 0; stLegalLen < kwstrIn.length(); ++stLegalLen)
    {
        if (!isdigit(kwstrIn[stLegalLen]))
        {
            break;
        }
        continue;
    }

    DWORD dwOut = 0;
    size_t stIndex = stLegalLen - 1;
    DWORD dwDigit = 1;
    while (0 < stLegalLen)
    {
        dwOut += (kwstrIn[stIndex] - '0') * dwDigit;

        if (0 == stIndex)
        {
            break;
        }
        --stIndex;
        dwDigit *= kdwRax;
    }
    return dwOut;
}

SYSTEMTIME ConvertStringToSysTime(_In_ std::wstring& kwstrTime, _In_ const std::wstring kwstrSep)
{
    // length = 5 + 2*5 + 3 + 6 = 24
    // kstuFirstSysTime.wYear;          // 5
    // kstuFirstSysTime.wMonth;         // 2
    // kstuFirstSysTime.wDay;           // 2
    // kstuFirstSysTime.wHour;          // 2
    // kstuFirstSysTime.wMinute;        // 2
    // kstuFirstSysTime.wSecond;        // 2
    // kstuFirstSysTime.wMilliseconds;  // 3
    SYSTEMTIME stuSysTime = { 0 };
    std::vector<std::wstring> vectemp;
    boost::algorithm::split(vectemp,kwstrTime,boost::algorithm::is_any_of(kwstrSep));
    if (7 <= vectemp.size())
    {
         stuSysTime.wYear     = StringToDword(vectemp[0]);
         stuSysTime.wMonth    = StringToDword(vectemp[1]);
         stuSysTime.wDay      = StringToDword(vectemp[2]);;
         stuSysTime.wHour     = StringToDword(vectemp[3]);;
         stuSysTime.wMinute   = StringToDword(vectemp[4]);;
         stuSysTime.wSecond   = StringToDword(vectemp[5]);;
         stuSysTime.wMilliseconds = StringToDword(vectemp[6]);
    }
    return stuSysTime;
}

bool IsSameSysTime(_In_ const SYSTEMTIME& kstuFirstTime, _In_ const SYSTEMTIME& kstuSecondTime, _In_ const DWORD kdwTimeInterval)
{NLONLY_DEBUG
    bool bSame = false;
    if ((kstuFirstTime.wYear == kstuSecondTime.wYear) &&
        (kstuFirstTime.wMonth == kstuSecondTime.wMonth) &&
        (kstuFirstTime.wDay == kstuSecondTime.wDay) &&
        (kstuFirstTime.wHour == kstuSecondTime.wHour) &&
        (kstuFirstTime.wMinute == kstuSecondTime.wMinute)
        )
    {
        DWORD dwFirstMilliseconds = kstuFirstTime.wSecond * 1000 + kstuFirstTime.wMilliseconds;
        DWORD dwSecondMilliseconds = kstuSecondTime.wSecond * 1000 + kstuSecondTime.wMilliseconds;
        
        DWORD dwAbs = NLGetAbs(dwFirstMilliseconds, dwSecondMilliseconds);
        bSame = (dwAbs < (kdwTimeInterval * 1000));
    }
    return bSame;
}

BOOL IfNeedCheckModifyFlag(LPCWSTR wszFileName)
{
    const wchar_t* szSupportTypes[] = { L".docx", L".doc", L".dot", L".docm", L".dotx", L".dotm",
        L".xlsx", L".xls", L".xlsm", L".xlt", L".xltm", L".xltx",
        L".pptx", L".ppt", L".pot", L".potx", L".potm", L".potx", L".pptm",
        L".pdf",
    };
    if (NULL != wszFileName)
    {
        size_t nFileNameLen = wcslen(wszFileName);
        for (int i = 0; i < sizeof(szSupportTypes) / sizeof(szSupportTypes[0]); i++)
        {
            const wchar_t* pFileType = szSupportTypes[i];
            size_t nFileTypeLen = wcslen(pFileType);

            if ((nFileNameLen>nFileTypeLen) &&
                (_wcsicmp(wszFileName+nFileNameLen-nFileTypeLen, pFileType) == 0))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


bool IsSameBinaryFile(_In_ const std::wstring& kwstrFirstFile, _In_ const std::wstring& kwstrSecondFile)
{NLONLY_DEBUG
    if (kwstrSecondFile.empty() || kwstrSecondFile.empty())
    {
        return false;
    }
    if (0 == _wcsicmp(kwstrFirstFile.c_str(), kwstrSecondFile.c_str()))
    {
        return true;
    }

    bool bBinarySame = true;

    HANDLE hFirstFile = CreateFile(kwstrFirstFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hSecondFile = CreateFile(kwstrSecondFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ((INVALID_HANDLE_VALUE != hFirstFile) && (INVALID_HANDLE_VALUE != hSecondFile))
    {
        DWORD dwFirstFileSize = GetFileSize(hFirstFile, NULL);
        DWORD dwSecondFileSize = GetFileSize(hSecondFile, NULL);
        if (dwFirstFileSize == dwSecondFileSize)
        {
			static const int nBufLen = 2048;
			char* pchFirstBuffer = new char[nBufLen];
			char* pchSecondBuffer = new char[nBufLen];

			DWORD dwFirstBytesRead = 0;
			DWORD dwSecondBytesRead = 0;
            for (DWORD i = 0; i < dwFirstFileSize; i += nBufLen)
            {
                memset(pchFirstBuffer, 0, nBufLen);
                memset(pchSecondBuffer, 0, nBufLen);
                
                if (ReadFile(hFirstFile, pchFirstBuffer, nBufLen, &dwFirstBytesRead, NULL))
                {
                    if (ReadFile(hSecondFile, pchSecondBuffer, nBufLen, &dwSecondBytesRead, NULL))
                    {
                        if (0 == memcmp(pchFirstBuffer, pchSecondBuffer, nBufLen))
                        {
                            continue;
                        }
                        else
                        {
                            bBinarySame = false;
                            break;
                        }
                    }
                    else
                    {
                        NLPRINT_DEBUGVIEWLOG(L"Read file:[%s] failed", kwstrSecondFile.c_str());
                        break;
                    }
                }
                else
                {
                    NLPRINT_DEBUGVIEWLOG(L"Read file:[%s] failed", kwstrFirstFile.c_str());
                    break;
                }
            }
			if (NULL != pchFirstBuffer)
			{
				delete[] pchFirstBuffer;
			}
			if (NULL != pchSecondBuffer)
			{
				delete[] pchSecondBuffer;
			}
        }
        else
        {
            bBinarySame = false;
            NLPRINT_DEBUGVIEWLOG(L"The file size between [%s] and [%s] not the same", kwstrFirstFile.c_str(), kwstrSecondFile.c_str());
        }
    }
    else
    {
        NLPRINT_DEBUGVIEWLOG(L"Try to open file:[%s] and [%s] to read failed\n", kwstrFirstFile.c_str(), kwstrSecondFile.c_str());
    }

    if (INVALID_HANDLE_VALUE != hFirstFile)
    {
        CloseHandle(hFirstFile);
    }
    if (INVALID_HANDLE_VALUE != hSecondFile)
    {
        CloseHandle(hSecondFile);
    }

    return bBinarySame;
}

// https://stackoverflow.com/questions/7011071/detect-32-bit-or-64-bit-of-windows
//Since Windows XP, see https://msdn.microsoft.com/en-us/library/windows/desktop/ms724340%28v=vs.85%29.aspx
typedef void (WINAPI *LPFN_GetNativeSystemInfo) (_Out_ LPSYSTEM_INFO lpSystemInfo);
//LPFN_GetNativeSystemInfo fnGetNativeSystemInfo;
BOOL is64bitOS()
{
	SYSTEM_INFO sysInfo = {0};
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"GetNativeSystemInfo");
	if(fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(&sysInfo);
	}else
	{
		GetSystemInfo(&sysInfo);
	}
	// PROCESSOR_ARCHITECTURE_IA64 = 6, PROCESSOR_ARCHITECTURE_AMD64 = 9
	return (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)||(sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64);
}


/*
IsWow64Process Determines whether the specified process is running under WOW64.
so a 32-bit application can detect whether it is running under WOW64 (on 64-bit Windows).

+------------+---------------------+---------------------+-----------------------+
|            | 32-bit win          | 64-bit win          | old win (only 32-bit) |
+------------+---------------------+---------------------+-----------------------+
| 32-bit app | Wow64Process(FALSE) | Wow64Process(TRUE)  | IsWow64Process N.A.   |
|            | sizeof(void*)=4     | sizeof(void*)=4     | sizeof(void*)=4       |
+------------+---------------------+---------------------+-----------------------+
| 64-bit app | Not run             | Wow64Process(FALSE) | Not run               |
|            |                     | sizeof(void*)=8     |                       |
+------------+---------------------+---------------------+-----------------------+

*/
BOOL IsOS64Bit()
{
	// if we don't depend on what the the compiler already knows, dynamically detect the bitness of a program
	//size_t nPointerSize = sizeof(void*);
	//_tprintf(_T("nPointerSize=%d\n"), (int)nPointerSize);

	//if (8 == nPointerSize) // #if _WIN64
	//{
	//	return TRUE;
	//}else if (4 == nPointerSize) // #elif _WIN32
	//{
	//	// balabala...
	//}

#if _WIN64
	// compiled as a 64-bit application and 64-bit programs run only on Win64, so no need to use a API call when the compiler already knows.
	return TRUE;
#elif _WIN32

	typedef BOOL (WINAPI *LPFN_IsWow64Process) (HANDLE, PBOOL);

	BOOL isWow64 = FALSE, isSuccessful;
	
	// IsWow64Process is not available on all supported versions of Windows (Since Windows XP with SP2, including 32-bit and 64-bit). 
	// Use GetModuleHandle to get a handle to the DLL that contains the function and GetProcAddress to get a pointer to the function if available.
	LPFN_IsWow64Process fnIsWow64Process = (LPFN_IsWow64Process) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if(fnIsWow64Process)
	{
		isSuccessful = fnIsWow64Process(GetCurrentProcess(), &isWow64);
		if (!isSuccessful)
		{
			//handle error
		}
	} // else  version < Windows XP with SP2

	return FALSE;
#else
	assert(0);
	return FALSE;
#endif
}


template<size_t _Size = MAX_PATH> // MAX_PATH
class RegQuerier
{
public:
	LSTATUS QueryString(HKEY hKey, LPCWSTR lpszValueName)
	{
		DWORD dwRegType = REG_NONE;
		DWORD dwValueLength = _countof(wszValueData);
		wszValueData[0] = '\0';
		LSTATUS lResult = RegQueryValueExW(hKey, lpszValueName, 0, &dwRegType,(LPBYTE)wszValueData, &dwValueLength);
		if (lResult != ERROR_SUCCESS)
		{
			wszValueData[0] = '\0';
		}
		return lResult;
	}

	LPCWSTR Buffer() const{ return wszValueData; }
	LPWSTR Buffer() { return wszValueData; }
	size_t Length() const { return _Size; }
private:
	union{
		wchar_t wszValueData[_Size];
		wchar_t dwValueData;
	};
};

static LSTATUS RegQueryDWORD(HKEY hKey, LPCWSTR lpszValueName, DWORD& dwValueData)
{
	DWORD dwRegType = REG_NONE;
	DWORD dwValueLength = sizeof(DWORD);
	LSTATUS lResult = RegQueryValueExW(hKey, lpszValueName, 0, &dwRegType,(LPBYTE)&dwValueData, &dwValueLength);
	return lResult;
}

void PrintRunningInfo(CComPtr<Outlook::_Application> spOutlookApp)
{
	std::wstring swLine;
	LPCWSTR pszModuleBitness, pszOutlookName, pwzOutlookVersion;
	const size_t nPointerSize = sizeof(void*);

	// if we don't depend on what the the compiler already knows, dynamically detect the bitness of a program

	if (8 == nPointerSize) // #if _WIN64
	{
		pszModuleBitness = L"64-bit";
	}else if (4 == nPointerSize) // #elif _WIN32
	{
		pszModuleBitness = L"32-bit";
	} else // #endif
	{
		pszModuleBitness = L"unknown";
	}
	CComBSTR sbsOutlookVersion;
	HRESULT hr = spOutlookApp->get_Version(&sbsOutlookVersion);
	if (SUCCEEDED(hr))
	{
		// e.g. As to Outlook Professional Plus 2013 32-bit, it is 15.0.0.4937
		pwzOutlookVersion =  sbsOutlookVersion.m_str;

		//Outlook 2003		11.0.5510.0	 
		//Outlook 2003 SP1	11.0.6353.0		KB842532
		//Outlook 2003 SP2	11.0.6565.0		KB887616
		//Outlook 2003 SP3	11.0.8169.0		KB923618
		//Outlook 2007		12.0.4518.1014	 
		//Outlook 2007 SP1	12.0.6212.1000	KB936982
		//Outlook 2007 SP2	12.0.6423.1000	KB953195
		//Outlook 2007 SP3	12.0.6607.1000	KB2526086
		//Outlook 2010		14.0.4760.1000	 
		//Outlook 2010 SP1	14.0.6029.1000	KB2460049
		//Outlook 2010 SP2	14.0.7015.1000	KB2687455
		//Outlook 2013		15.0.4420.1000	 
		//Outlook 2013 SP1	15.0.4569.1506	KB2817430
		//Outlook 2016		16.0.4229.1003
		const int nFirstVer = StrToIntW(pwzOutlookVersion);
		switch(nFirstVer)
		{
			case 11: pszOutlookName = L"Outlook 2003" ; break;
			case 12: pszOutlookName = L"Outlook 2007" ; break;
			case 14: pszOutlookName = L"Outlook 2010" ; break;
			case 15: pszOutlookName = L"Outlook 2013" ; break;
			case 16: pszOutlookName = L"Outlook 2016" ; break;
			default: pszOutlookName = L"Outlook" ; break;
		}
	}else
	{
		pwzOutlookVersion = L"Outlook ErrVer";
	}


	// Starting with Windows 8.1, the GetVersion(Ex) API is deprecated and will detect the
	// application as Windows 8 (kernel version 6.2) until an application manifest is included
	// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn302074.aspx


	RegQuerier<64> queryProductName;
	RegQuerier<64> queryCSDVersion;
	RegQuerier<16> queryCurrentVersion;
	RegQuerier<16> queryCurrentBuild;

	HKEY hKeyCurrentVersion = NULL; //only operate on the 64-bit registry view (KEY_WOW64_64KEY is ignored by 32-bit Windows)
	LONG lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ | KEY_WOW64_64KEY, &hKeyCurrentVersion);
	if((ERROR_SUCCESS == lResult) && (hKeyCurrentVersion != NULL))
	{
		queryProductName.QueryString(hKeyCurrentVersion, L"ProductName");  // e.g. Windows 7 Enterprise
		queryCSDVersion.QueryString(hKeyCurrentVersion, L"CSDVersion");  // e.g. Service Pack 1

		// CurrentMinorVersionNumber present in registry starting with Windows 10, If CurrentMinorVersionNumber not present then use CurrentVersion
		DWORD dwMajor = 0, dwMinor = 0;
		if(ERROR_FILE_NOT_FOUND == RegQueryDWORD(hKeyCurrentVersion, L"CurrentMajorVersionNumber", dwMajor)) // e.g. 0xa for windows 10
		{
			queryCurrentVersion.QueryString(hKeyCurrentVersion, L"CurrentVersion");  // e.g.  6.1
		}else
		{
			RegQueryDWORD(hKeyCurrentVersion, L"CurrentMinorVersionNumber", dwMinor); // e.g. 0x0 for windows 10
			_snwprintf(queryCurrentVersion.Buffer(), queryCurrentVersion.Length(), L"%u.%u", dwMajor, dwMinor);
		}

		queryCurrentBuild.QueryString(hKeyCurrentVersion, L"CurrentBuild");  // e.g. 7601

		RegCloseKey(hKeyCurrentVersion);
	}

	logd(L"Application: %s, %s (%s), OS: %s %s (%s.%s) %s\n", pszModuleBitness, pszOutlookName, pwzOutlookVersion, 
		queryProductName.Buffer(), queryCSDVersion.Buffer(), queryCurrentVersion.Buffer(), queryCurrentBuild.Buffer()
		, is64bitOS() ? L"x64" : L"x86");
}