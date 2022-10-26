

#pragma once
#ifndef _NL_DEVCON_REG_PROPERTY_HPP_
#define _NL_DEVCON_REG_PROPERTY_HPP_
#include <string>
#include <vector>

class CDevRegProp
{
public:
    CDevRegProp();
    CDevRegProp(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo);
    virtual ~CDevRegProp();
    void GetProps(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo);

public:
    static BOOL GetString(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out std::wstring& strValue);
    static BOOL GetMultiString(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out std::vector<std::wstring>& mstrValue);
    static BOOL GetInt(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out PDWORD pdwValue);
    static BOOL GetIntValueEx(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out PDWORD pdwPropType, __out LPBYTE* ppPropData, __out PDWORD pdwPropDataSize);

public:
    std::wstring    m_class;
    std::wstring    m_classguid;
    std::wstring    m_friendlyname;
    std::wstring    m_svcname;
    std::wstring    m_mfgname;
    std::wstring    m_devdesc;
    DWORD           m_devtype;
    DWORD           m_inststat;
    std::vector<std::wstring>   m_hwIds;
    std::vector<std::wstring>   m_compatIds;
};

#endif