#pragma once
#include "NLType.h"

template<class TKey, class TValue>
bool GetValueFromMapByKey(_In_ const std::map<TKey, TValue>& kmapObject, _In_ const TKey tkeyInput, _Inout_ TValue& tvalueOutput)
{
    bool bRet = false;
#if 0
    try
    {
        // C++11 support .at method. It will throw out_of_range exception if the key is not exist
        tvalueOutput = kmapObject.at(tkeyInput);
        bRet = true;
    }
    catch (const std::out_of_range& /*oor*/)    // Note: oor.what() return value is char* not wchar_t*
    {
        NLPRINT_DEBUGVIEWLOG(L"Cannot find source file path in cache with out_of_range exception.\n");
    }
#else
    std::map<TKey, TValue>::const_iterator kItr = kmapObject.find(tkeyInput);
    std::map<TKey, TValue>::const_iterator kItrEnd = kmapObject.end();
    if (kItr != kItrEnd)
    {
        tvalueOutput = kItr->second;
        bRet = true;
    }
#endif
    return bRet;
}

#ifndef NLUNITTEST
template<class TComObjet>
CComPtr<IUnknown> GetComObjectIUnknownPointer()
{
    CComPtr<IUnknown> spIUnknown = NULL;

    CComObject<TComObjet>* spDisp = NULL;
    HRESULT hr = CComObject<TComObjet>::CreateInstance(&spDisp);
    if (SUCCEEDED(hr) && (NULL != spDisp))
    {
        hr = spDisp->QueryInterface(IID_IUnknown, (void**)&spIUnknown);
        if (FAILED(hr))
        {
            spIUnknown = NULL;
        }
    }
    return spIUnknown;
}
#endif /*NLUNITTEST*/

#ifndef NLUNITTEST
HRESULT SinkAppEvent(_Inout_ IUnknown* pEventDispatcher, _Inout_opt_ IUnknown* pEventReceiver, _In_ const IID& iid, _Inout_ LPDWORD pDwSinkCookie);

HRESULT GetAttachments(CComPtr<IDispatch> spDisp, struct Outlook::Attachments * * Attachments);
#endif /*NLUNITTEST*/

bool IsFullLocalPath(_In_ const std::wstring& kwstrPath, bool bIsFolder);

bool IsFileInSpecifyFolder(_In_ const std::wstring& kwstrFilePath, _In_ const std::wstring& kwstrFolderPath);

std::wstring NLNewGUID();

std::wstring GetSuffixFromFileName(_In_ const std::wstring& wstrFileName);  // return suffix without ".", c:\kaka\a.docx =>docx

std::wstring GetFilePath(_In_ const std::wstring& wstrFilePath);            // return folder with "\", c:\kaka\a.docx => c:\kaka\

std::wstring GetFileName(_In_ const std::wstring& wstrFilePath);            // return file name with suffix, , c:\kaka\a.docx => a.docx

std::wstring GetFileNameWithoutSuffix(_In_ const std::wstring& wstrFilePath);    // return file name without suffix, c:\kaka\a.docx => a

std::wstring CreateAUniqueSubFolder(_In_ const std::wstring& kwstrFolder);

// knTimeType: 0: create time, 1: modify time, 2: access time
bool GetFileSystemTime(_In_ const std::wstring& kwstrFileFullPath, _Inout_ SYSTEMTIME* pStuSysTime, const int knTimeType);

std::wstring ComvertSysTimeToString(_In_ const SYSTEMTIME& kstuSysTime);

int CompareSysTime(_In_ const SYSTEMTIME& kstuFirstSysTime, _In_ const SYSTEMTIME& kstuSecondSysTime);

bool DeleteFolderOrFile(_In_ const std::wstring& kwstrPath, _In_ const bool kbAllowToRecycleBin);

// If failed return kwstrFileShortPath
std::wstring NLGetLongFilePathEx(_In_ const std::wstring& kwstrFileShortPath);

// If failed return empty string
std::wstring NLGetLongFilePath(_In_ const std::wstring& kwstrFileShortPath);

bool GetOSInfo(DWORD& dwMajor, DWORD& dwMinor);

bool IsWin10(void); // version: 6.2, vista 6.1, win7 6.0

EMNLOE_IMAGETYPE GetOutlookImageType();
EMNLOE_IMAGETYPE GetOSImageType();
EMNLOE_PLATEFORMVERSION GetPlateformVersion();
std::wstring GetDefaultShellFolderCache();
std::wstring GetContentWordFolderFromOutlookTempPath();
std::wstring GetContentMsoFolderFromOutlookTempPath();
std::wstring GetShellFolder(HKEY hRootKey, const wchar_t* wszPath, const wchar_t* wszName);

std::wstring GetNLTempFileFolder(_In_ const EMNLOE_TEMPLOCATION kemTempLocation);

unsigned int GetItemCountInTheFolder(_In_ const std::wstring& kwstrFolder);

void DeleteEmptyFolderInTheFolder(_In_ const std::wstring& kwstrFolder);

std::wstring ConverSysTimeToString(_In_ const SYSTEMTIME& kstuSysTime);

SYSTEMTIME ConvertStringToSysTime(_In_ std::wstring& kwstrTime, _In_ const std::wstring kwstrSep = L"-");

DWORD NLGetAbs(DWORD dwFirst, DWORD dwSecond);

DWORD StringToDword(_In_ const std::wstring& kwstrIn);

bool IsSameSysTime(_In_ const SYSTEMTIME& kstuFirstTime, _In_ const SYSTEMTIME& kstuSecondTime, _In_ const DWORD kdwTimeInterval = 1);

BOOL IfNeedCheckModifyFlag(LPCWSTR wszFileName);

bool IsSameBinaryFile(_In_ const std::wstring& kwstrFirstFile, _In_ const std::wstring& kwstrSecondFile);

void PrintRunningInfo(CComPtr<Outlook::_Application> spOutlookApp);