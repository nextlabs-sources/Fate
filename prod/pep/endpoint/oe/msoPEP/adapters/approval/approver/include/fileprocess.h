

#pragma once
#ifndef _FILE_PROCESS_H_
#define _FILE_PROCESS_H_
#include <string>

#if(_WIN32_WINNT < 0x0500)
#undef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#include <Sddl.h>
#else
#include <Sddl.h>
#endif
#pragma comment(lib, "Advapi32")

class FileProcess
{
public:
    FileProcess(){}
    ~FileProcess(){}

    static std::wstring ReplaceSubStr(std::wstring& strIn, LPCWSTR pwzSub, LPCWSTR pwzNew)
    {
        std::wstring::size_type stPos;
        std::wstring strSub = pwzSub;

        stPos = strIn.find(strSub);
        while(stPos != std::wstring::npos)
        {
            strIn.replace(stPos, strSub.size(), pwzNew);
            stPos = strIn.find(strSub, stPos);
        }

        return strIn;
    }
    static std::wstring ComposeFolderNameWithTimeStamp(LPCWSTR pwzUserName)
    {
        // Replace L" " with L"_"
        std::wstring strFolderName = pwzUserName;
        ReplaceSubStr(strFolderName, L" ", L"_");
        strFolderName += L"_";

        SYSTEMTIME  st;
        GetLocalTime(&st);

        // LocalTime: 20080101101010xxxx
        WCHAR wzLocalTime[19]; memset(wzLocalTime, 0, sizeof(wzLocalTime));
        _snwprintf_s(wzLocalTime, 19, _TRUNCATE, L"%04d%02d%02d%02d%02d%02d%04d", st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

        strFolderName += wzLocalTime;
        return strFolderName;
    }
};

#endif