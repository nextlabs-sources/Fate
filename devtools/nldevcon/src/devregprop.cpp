

#include <Windows.h>

#include "nldevcon.hpp"
#include "devregprop.hpp"

CDevRegProp::CDevRegProp()
{
}

CDevRegProp::CDevRegProp(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo)
{
    GetProps(Devs, DevInfo);
}

CDevRegProp::~CDevRegProp()
{
}

void CDevRegProp::GetProps(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo)
{    
    m_class         = L"";
    m_classguid     = L"";
    m_friendlyname  = L"";
    m_svcname       = L"";
    m_mfgname       = L"";
    m_devdesc       = L"";
    m_devtype       = 0;
    m_inststat      = 0;
    m_hwIds.clear();
    m_compatIds.clear();

    // SPDRP_CLASS (REG_SZ): Get the device setup class
    GetString(Devs, DevInfo, SPDRP_CLASS, m_class);

    // SPDRP_CLASSGUID (REG_SZ): Get the GUID that represents the device setup class of a device
    GetString(Devs, DevInfo, SPDRP_CLASSGUID, m_classguid);

    // SPDRP_HARDWAREID (REG_MULTI_SZ): Get the list of hardware IDs for a device
    GetMultiString(Devs, DevInfo, SPDRP_HARDWAREID, m_hwIds);

    // SPDRP_COMPATIBLEIDS (REG_MULTI_SZ): Get the list of compatible IDs for a device
    GetMultiString(Devs, DevInfo, SPDRP_COMPATIBLEIDS, m_compatIds);

    // SPDRP_FRIENDLYNAME (REG_SZ): Get the friendly name of a device
    GetString(Devs, DevInfo, SPDRP_FRIENDLYNAME, m_friendlyname);

    // SPDRP_DEVICEDESC (REG_SZ): Get the description of a device
    GetString(Devs, DevInfo, SPDRP_DEVICEDESC, m_devdesc);

    // SPDRP_DEVTYPE (REG_DWORD): Retrieves a DWORD value that represents the device's type
    GetInt(Devs, DevInfo, SPDRP_DEVTYPE, &m_devtype);

    // SPDRP_INSTALL_STATE (REG_DWORD): Retrieves a DWORD value that indicates the installation state of a device
    GetInt(Devs, DevInfo, SPDRP_INSTALL_STATE, &m_inststat);

    // SPDRP_SERVICE (REG_SZ): Get the service name for a device
    GetString(Devs, DevInfo, SPDRP_SERVICE, m_svcname);

    // SPDRP_MFG (REG_SZ): Get the name of the device manufacturer
    GetString(Devs, DevInfo, SPDRP_MFG, m_mfgname);
}

BOOL CDevRegProp::GetString(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out std::wstring& strValue)
{
    BOOL   bRet           = FALSE;
    LPBYTE pbPropData     = NULL;
    DWORD  dwPropDataSize = 0;
    DWORD  dwPropType     = 0;

    if(!GetIntValueEx(Devs, DevInfo, Prop, &dwPropType, &pbPropData, &dwPropDataSize))
        return FALSE;
    if(REG_SZ != dwPropType)
        goto _exit;

    strValue = (WCHAR*)pbPropData;
    bRet = TRUE;

_exit:
    if(pbPropData) delete []pbPropData; pbPropData=NULL;
    return bRet;
}

BOOL CDevRegProp::GetMultiString(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out std::vector<std::wstring>& mstrValue)
{
    BOOL   bRet           = FALSE;
    LPBYTE pbPropData     = NULL;
    LPWSTR wzProp         = NULL;
    DWORD  dwPropDataSize = 0;
    DWORD  dwPropType     = 0;

    if(!GetIntValueEx(Devs, DevInfo, Prop, &dwPropType, &pbPropData, &dwPropDataSize))
        return FALSE;
    if(REG_MULTI_SZ != dwPropType)
        goto _exit;

    bRet = TRUE;
    wzProp = (LPWSTR)pbPropData;
    while(L'\0' != wzProp[0])
    {
        std::wstring strValue = wzProp;
        mstrValue.push_back(strValue);
        wzProp += strValue.length()+1;
    }

_exit:
    if(pbPropData) delete []pbPropData; pbPropData=NULL;
    return bRet;
}

BOOL CDevRegProp::GetInt(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out PDWORD pdwValue)
{
    BOOL   bRet           = FALSE;
    LPBYTE pbPropData     = NULL;
    DWORD  dwPropDataSize = 0;
    DWORD  dwPropType     = 0;

    if(!GetIntValueEx(Devs, DevInfo, Prop, &dwPropType, &pbPropData, &dwPropDataSize))
        return FALSE;
    if(REG_DWORD != dwPropType)
        goto _exit;

    *pdwValue = *((PDWORD)pbPropData);
    bRet = TRUE;

_exit:
    if(pbPropData) delete []pbPropData; pbPropData=NULL;
    return bRet;
}

BOOL CDevRegProp::GetIntValueEx(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in DWORD Prop, __out PDWORD pdwPropType, __out LPBYTE* ppPropData, __out PDWORD pdwPropDataSize)
{
    DWORD  dwPropType     = 0;
    DWORD  dwPropDataSize = 0;
    LPBYTE pbPropData     = NULL;
    
    *ppPropData      = NULL;
    *pdwPropDataSize = 0;
    *pdwPropType     = 0;

    // Get required buffer size
    SetupDiGetDeviceRegistryPropertyW(Devs, DevInfo, Prop, &dwPropType, NULL, 0, &dwPropDataSize);
    if(0==dwPropDataSize) return FALSE;

    dwPropDataSize += (sizeof(WCHAR)*2);
    pbPropData = new BYTE[dwPropDataSize+sizeof(WCHAR)*2];
    if(NULL == pbPropData) return FALSE;
    memset(pbPropData, 0, sizeof(pbPropData));

    if(!SetupDiGetDeviceRegistryPropertyW(Devs, DevInfo, Prop, &dwPropType, pbPropData, dwPropDataSize+sizeof(WCHAR)*2, NULL))
    {
        delete []pbPropData;
        return FALSE;
    }

    pbPropData[dwPropDataSize]   = 0;
    pbPropData[dwPropDataSize+1] = 0;
    pbPropData[dwPropDataSize+2] = 0;
    pbPropData[dwPropDataSize+3] = 0;
    *ppPropData      = pbPropData;
    *pdwPropDataSize = dwPropDataSize;
    *pdwPropType     = dwPropType;
    return TRUE;
}