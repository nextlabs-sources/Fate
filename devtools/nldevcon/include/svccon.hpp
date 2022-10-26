

#pragma once
#ifndef _NL_DEVCON_SVC_CONFIG_HPP_
#define _NL_DEVCON_SVC_CONFIG_HPP_
#include <string>
#include <vector>

class CSvcInfo
{
public:
    std::wstring    m_name;
    std::wstring    m_dispname;
    std::wstring    m_description;
    std::wstring    m_imagepath;
    DWORD           m_starttype;
    DWORD           m_svctype;
    std::vector<std::wstring>   m_depends;
};

class CSvcCon
{
public:
    CSvcCon();
    virtual ~CSvcCon();

public:
    BOOL Open(__in LPCWSTR wzName);
    BOOL Create(__in LPCWSTR wzName, __in BOOL bFailIfExist=TRUE);
    BOOL Delete(__in LPCWSTR wzName);

public:
    BOOL Start();
    BOOL Stop();
    BOOL Status();

    BOOL BootStart();
    BOOL SystemStart();
    BOOL AutoStart();
    BOOL ManualStart();
    BOOL Disable();

private:
    CSvcInfo    m_info;
};

#endif