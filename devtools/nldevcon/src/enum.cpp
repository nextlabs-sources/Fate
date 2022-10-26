

#include <windows.h>
#include <stdio.h>

#include "nldevcon.hpp"
#include "enum.hpp"

static BOOL CompareClassName(__in const CDevRegProp& DevProps, __in_opt LPCWSTR ClassName);
static BOOL CompareClassGuid(__in const CDevRegProp& DevProps, __in_opt LPCWSTR ClassGuid);
static BOOL CompareHwIds(__in const CDevRegProp& DevProps, __in LPCWSTR HwId);
static int QueryDeviceCallback(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in const CDevRegProp& DevProps, __in DWORD Index, __in int* nFound);


int cmdQuery(__in LPCNLDEVCON_ENUM_RESTRICTIONS Restrictions)
{
    int nFound = 0;
    int nRet   = 0;
    nRet = EnumerateDevices(Restrictions, 0, (EnumCallbackFunc)QueryDeviceCallback, &nFound);

    if(0 == nFound) printf("No devices is found\n");
    else printf("\n%d %s been found\n", nFound, (nFound>1)?"devices have":"device has");
    return nRet;
}

int EnumerateDevices(__in LPCNLDEVCON_ENUM_RESTRICTIONS Restrictions, __in DWORD Flags, __in EnumCallbackFunc Callback, __in LPVOID Context)
{
    int                         nRet = 0;
    int                         devIndex = 0;
    HDEVINFO                    devs = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA             devInfo = {0};
    SP_DEVINFO_LIST_DETAIL_DATA devInfoListDetail = {0};

    devs = SetupDiGetClassDevsEx(NULL, NULL, NULL, (DIGCF_ALLCLASSES|Flags), NULL, NULL, NULL);
    if(devs == INVALID_HANDLE_VALUE)
        goto _exit;

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if(!SetupDiGetDeviceInfoListDetail(devs,&devInfoListDetail))
        goto _exit;

    devInfo.cbSize = sizeof(devInfo);
    for(devIndex=0; SetupDiEnumDeviceInfo(devs, devIndex, &devInfo); devIndex++)
    {
        CDevRegProp devProps(devs, &devInfo);

        // This device match the condition
        if(CompareClassName(devProps, Restrictions->ClassName)
            && CompareClassGuid(devProps, Restrictions->ClassGuid)
            && CompareHwIds(devProps, Restrictions->HwId))
        {
            nRet = Callback(devs, &devInfo, devProps, devIndex, Context);
            if(nRet < 0)
                goto _exit;
        }
    }

_exit:
    if(devs != INVALID_HANDLE_VALUE) SetupDiDestroyDeviceInfoList(devs);
    return nRet;
}

//////////////////////////////////////////////////////////////////////////////
// Local Routines
BOOL CompareClassName(__in const CDevRegProp& DevProps, __in_opt LPCWSTR ClassName)
{
    if(NULL==ClassName) return TRUE;
    if(0 == _wcsicmp(DevProps.m_class.c_str(), ClassName)) return TRUE;
    return FALSE;
}

BOOL CompareClassGuid(__in const CDevRegProp& DevProps, __in_opt LPCWSTR ClassGuid)
{
    if(NULL==ClassGuid) return TRUE;
    if(0 == _wcsicmp(DevProps.m_classguid.c_str(), ClassGuid)) return TRUE;
    return FALSE;
}

BOOL CompareHwIds(__in const CDevRegProp& DevProps, __in_opt LPCWSTR HwId)
{
    int i=0;

    if(NULL==HwId) return TRUE;

    for(i=0; i<(int)DevProps.m_hwIds.size(); i++)
    {
        if(0 == _wcsicmp(DevProps.m_hwIds[i].c_str(), HwId)) return TRUE;
    }
    for(i=0; i<(int)DevProps.m_compatIds.size(); i++)
    {
        if(0 == _wcsicmp(DevProps.m_compatIds[i].c_str(), HwId)) return TRUE;
    }

    return FALSE;
}

int QueryDeviceCallback(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in const CDevRegProp& DevProps, __in DWORD Index, __in int* nFound)
{
    int     i = 0;    
    WCHAR                   devID[MAX_DEVICE_ID_LEN];
    SP_DEVINFO_LIST_DETAIL_DATA_W devInfoListDetail;
    memset(devID, 0, sizeof(devID));

    UNREFERENCED_PARAMETER(Index);

    devInfoListDetail.cbSize = sizeof(devInfoListDetail);
    if((!SetupDiGetDeviceInfoListDetail(Devs,&devInfoListDetail)) ||
            (CM_Get_Device_ID_Ex(DevInfo->DevInst, devID, MAX_DEVICE_ID_LEN, 0, devInfoListDetail.RemoteMachineHandle)!=CR_SUCCESS))
    {
        return NLDEVCON_SUCCESS;
    }

    printf("\n");
    printf("%03d %S\n", *nFound, devID);
    printf("    Friendly Name: %S\n", DevProps.m_friendlyname.c_str());
    printf("    Class:         %S %S\n", DevProps.m_class.c_str(), DevProps.m_classguid.c_str());
    printf("    Manufacturer:  %S\n", DevProps.m_mfgname.c_str());
    printf("    Service:       %S\n", DevProps.m_svcname.c_str());
    printf("    Description:   %S\n", DevProps.m_devdesc.c_str());
    printf("    HardwareIDs:\n");
    for(i=0; i<(int)DevProps.m_hwIds.size(); i++)
        printf("            %S\n", DevProps.m_hwIds[i].c_str());
    printf("    Compatible IDs:\n");
    for(i=0; i<(int)DevProps.m_compatIds.size(); i++)
        printf("            %S\n", DevProps.m_compatIds[i].c_str());

    *nFound = *nFound +1;
    return NLDEVCON_SUCCESS;
}