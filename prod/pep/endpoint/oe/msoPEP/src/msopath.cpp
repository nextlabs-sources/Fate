
#include "stdafx.h"
#include <windows.h>
#include <string>
#include <boost\algorithm\string.hpp>
#include "msopath.hpp"



CPath::CPath()
{
}

CPath::CPath(_In_ const std::wstring& strPath)
{
    Set(strPath);
}

CPath::~CPath()
{
}

void CPath::Set(_In_ const std::wstring& strPath)
{
    m_path = strPath;

    if(!boost::algorithm::contains(m_path, L"\\"))
    {
        m_filename  = m_path;
        m_path = L"";
        m_parentdir = L"";
    }
    else
    {
        std::wstring::size_type pos;

        pos = m_path.find_last_of(L'\\');
        m_filename = m_path.substr(pos+1);
        m_filename = CPath::ToFileName(m_filename);
        m_parentdir = m_path.substr(0, pos-1);

        pos = m_filename.find_last_of(L'.');
        m_wstrFileNameWithoutSuffix = m_filename.substr(0, pos);
        m_wstrFileSuffix = m_filename.substr(pos+1);
    }
}

std::wstring CPath::ToFileName(_In_ const std::wstring& strName)
{
    std::wstring strFileName;

    std::wstring::const_iterator it = strName.begin();
    for( ; it != strName.end(); ++it)
    {
        if(L'\\' != *it
            && L'/' != *it
            && L':' != *it
            && L'*' != *it
            && L'?' != *it
            && L'\"' != *it
            && L'<' != *it
            && L'>' != *it
            && L'|' != *it
            )
            strFileName += *it;
    }

    if(strFileName.empty()) strFileName = L"Untiled";
    return strFileName;
}


CDisplayName::CDisplayName()
{
}

CDisplayName::CDisplayName(_In_ LPCWSTR wzDisplayName)
{
    m_original = wzDisplayName;
    Parse(m_original);
}

CDisplayName::~CDisplayName()
{
}

std::wstring CDisplayName::GetFileNameFromPath(_In_ const std::wstring& strPath)
{
    std::wstring::size_type pos;

    if(!boost::algorithm::contains(strPath, L"\\"))
        return strPath;

    pos = strPath.find_last_of(std::wstring(L"\\"));
    return strPath.substr(pos+1);
}

void CDisplayName::ParseMagicDisplayName(_In_ const std::wstring& strMagic, _Out_ std::wstring& strTempPath, _Out_ std::wstring& strRealPath)
{
    std::wstring::size_type pos;
    const std::wstring  strMagicName(TEMP_MAGIC_NAME);
    
    pos = strMagic.find(strMagicName);

    strTempPath = strMagic.substr(0, pos);
    strRealPath = strMagic.substr(pos + strMagicName.length());
}

void CDisplayName::Parse(_In_ const std::wstring& strDisplayName)
{
    if(CPath::IsMagicPath(strDisplayName))
    {
        ParseMagicDisplayName(strDisplayName, m_temp, m_real);
        
        if(!m_temp.empty())
        {
            m_displayname = CDisplayName::GetFileNameFromPath(m_temp);
            m_filename = CPath::ToFileName(m_displayname);
        }
        else
        {
            if(!m_real.empty()) m_displayname = CDisplayName::GetFileNameFromPath(m_real);
            m_filename = CPath::ToFileName(m_displayname);
        }
    }
    else
    {
        m_temp  = strDisplayName;
        m_real  = strDisplayName;

        if(CPath::IsFullPath(strDisplayName))
        {
            m_displayname = CDisplayName::GetFileNameFromPath(strDisplayName);
            m_filename = CPath::ToFileName(m_displayname);
        }
        else
        {
            m_displayname = strDisplayName;
            m_filename = CPath::ToFileName(m_displayname);
        }
    }
}
