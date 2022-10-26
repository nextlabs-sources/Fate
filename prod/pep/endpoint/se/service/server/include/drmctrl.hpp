
#pragma once
#ifndef __DRM_CONTROLLER_H__
#define __DRM_CONTROLLER_H__
#include <list>
#include <string>
#include <boost\algorithm\string.hpp>


class is_sub
{
public:
    is_sub(LPCWSTR wzParent){wstrParent=wzParent; if(!boost::algorithm::iends_with(wstrParent, L"\\")) wstrParent += L"\\";}
    bool operator() (const std::wstring& value)
    {
        std::wstring v = value;
        if(!boost::algorithm::iends_with(v, L"\\")) v += L"\\";
        return boost::algorithm::istarts_with(v, wstrParent);
    }
    void setparent(LPCWSTR wzParent){wstrParent=wzParent; if(!boost::algorithm::iends_with(wstrParent, L"\\")) wstrParent += L"\\";}
private:
    std::wstring wstrParent;
};

class is_parent
{
public:
    is_parent(LPCWSTR wzSub){wstrSub=wzSub; if(!boost::algorithm::iends_with(wstrSub, L"\\")) wstrSub += L"\\";}
    bool operator() (const std::wstring& value)
    {
        std::wstring v = value;
        if(!boost::algorithm::iends_with(v, L"\\")) v += L"\\";
        return boost::algorithm::istarts_with(wstrSub, v);
    }
    void setsub(LPCWSTR wzSub){wstrSub=wzSub; if(!boost::algorithm::iends_with(wstrSub, L"\\")) wstrSub += L"\\";}
private:
    std::wstring wstrSub;
};

class CDRMCtrl
{
public:
    CDRMCtrl();
    virtual ~CDRMCtrl();

    BOOL Open(_In_ LPCWSTR wzCfgFile);
    void Close();

    BOOL Commit();
    BOOL SetSharedKeyRing(_In_ LPCWSTR wzKeyRing, _In_ BOOL AutoCommit);
    BOOL AddPath(_In_ LPCWSTR wzPath, _In_ BOOL FastWrite, _In_ BOOL AutoCommit);
    BOOL RemovePath(_In_ LPCWSTR wzPath, _In_ BOOL FastWrite, _In_ BOOL AutoCommit);

    inline std::wstring GetSharedKeyRing(){return m_skn;}
    inline std::list<std::wstring>& GetDrmList(){return m_drmpaths;}
    inline std::list<std::wstring>& GetFwDrmList(){return m_fwdrmpaths;}

protected:
    BOOL HaveUtf8Bom(_In_ LPCWSTR wzFile);
    BOOL RemoveUtf8Bom(_In_ LPCWSTR wzFile);
    BOOL WideToUtf8(_In_ const std::wstring& strLine, _Out_ std::string& strUtf8Line);
    BOOL Utf8ToWide(_In_ const std::string& strUtf8Line, _Out_ std::wstring& strLine);

    BOOL ConflictPath(_In_ const std::wstring& wstrPath, _In_ const std::list<std::wstring>& Paths);
    BOOL ExistPath(_In_ const std::wstring& wstrPath, _In_ const std::list<std::wstring>& Paths);

    BOOL AddPathToList(_In_ const std::wstring& wstrPath, _In_ std::list<std::wstring>& Paths);
    BOOL RemovePathFromList(_In_ const std::wstring& wstrPath, _In_ std::list<std::wstring>& Paths);

    std::string BuildSharedKeyRingLine(const std::wstring& wstrKeyRing);
    std::string BuildDrmPathLine(const std::wstring& wstrPath);
    std::string BuildNonDrmPathLine(const std::wstring& wstrPath);
    std::string BuildFwDrmPathLine(const std::wstring& wstrPath);

    BOOL WriteLine(HANDLE hFile, const std::string& strLine);
    BOOL EnsureFileExist(LPCWSTR wzFile);

private:
    std::list<std::wstring> m_drmpaths;   // drm paths
    CRITICAL_SECTION        m_drmpaths_lock;
    std::list<std::wstring> m_nondrmpaths;   // drm paths
    CRITICAL_SECTION        m_nondrmpaths_lock;
    std::list<std::wstring> m_fwdrmpaths; // fast-write drm paths
    CRITICAL_SECTION        m_fwdrmpaths_lock;
    std::wstring            m_skn;        // Shared key name
    std::wstring            m_file;
};

#endif