

#pragma once
#ifndef __MSO_PATH_HPP__
#define __MSO_PATH_HPP__
#include <string>


class CPath
{
public:
    CPath();
    CPath(_In_ const std::wstring& strPath);
    ~CPath();

    void Set(_In_ const std::wstring& strPath);

    static std::wstring ToFileName(_In_ const std::wstring& strName);

    static inline BOOL IsMagicPath(_In_ const std::wstring& strPath)
    {
        return boost::algorithm::contains(strPath, TEMP_MAGIC_NAME)?TRUE:FALSE;
    }

    static inline BOOL IsFullPath(_In_ const std::wstring& strPath)
    {
        return boost::algorithm::contains(strPath, L"\\")?TRUE:FALSE;
    }

    inline std::wstring GetPath() {return m_path;}
    inline std::wstring GetFileName() {return m_filename;}
    inline std::wstring GetParentDir() {return m_parentdir;}

    inline std::wstring GetFileNameWithoutSuffix() {return m_wstrFileNameWithoutSuffix;}
    inline std::wstring GetFileSuffix() {return m_wstrFileSuffix;}

private:
    std::wstring    m_path;
    std::wstring    m_parentdir;
    std::wstring    m_filename;

    std::wstring    m_wstrFileNameWithoutSuffix;
    std::wstring    m_wstrFileSuffix;
};


class CDisplayName
{
public:
    CDisplayName();
    CDisplayName(_In_ LPCWSTR wzDisplayName);
    ~CDisplayName();

    static std::wstring GetFileNameFromPath(_In_ const std::wstring& strPath);

    inline std::wstring GetOriginalPath() {return m_original;}
    inline std::wstring GetTempPath() {return m_temp;}
    inline std::wstring GetRealPath() {return m_real;}
    inline std::wstring GetDisplayName() {return m_displayname;}
    inline std::wstring GetFileName() {return m_filename;}

protected:
    void Parse(_In_ const std::wstring& strDisplayName);
    void ParseMagicDisplayName(_In_ const std::wstring& strMagic, _Out_ std::wstring& strTempPath, _Out_ std::wstring& strRealPath);

private:
    std::wstring    m_original;
    std::wstring    m_temp;
    std::wstring    m_real;
    std::wstring    m_displayname;
    std::wstring    m_filename;
};

#endif