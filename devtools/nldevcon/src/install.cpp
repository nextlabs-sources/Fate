

#include <windows.h>
#include <stdio.h>

#include "nldevcon.hpp"
#include "enum.hpp"


typedef struct _REMOVE_CONTEXT
{
    BOOL bReboot;
    int  nIndex;
}REMOVE_CONTEXT, *PREMOVE_CONTEXT;
static int RemoveDeviceCallback(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in const CDevRegProp& DevProps, __in DWORD Index, __in PREMOVE_CONTEXT Context);


int cmdUpdate(__in LPCWSTR wzInfPath, __in LPCWSTR wzHwId)
{
    int     nRet        = NLDEVCON_FAIL_TO_UPDATE;
    DWORD   dwFlags     = 0;
    BOOL    bReboot     = FALSE;
    HMODULE hNewDevMod  = NULL;
    UpdateDriverForPlugAndPlayDevicesProto UpdateFn = NULL;

    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wzInfPath))
        goto _exit;

    hNewDevMod = ::LoadLibraryW(L"newdev.dll");
    if(NULL == hNewDevMod)
        goto _exit;

    UpdateFn = (UpdateDriverForPlugAndPlayDevicesProto)GetProcAddress(hNewDevMod, UPDATEDRIVERFORPLUGANDPLAYDEVICES);
    if(NULL == UpdateFn)
        goto _exit;
    
    if(!UpdateFn(NULL, wzHwId, wzInfPath, dwFlags, &bReboot))
        goto _exit;

    nRet = bReboot ? NLDEVCON_SUCCESS_NEED_REBOOT : NLDEVCON_SUCCESS;

_exit:
    return nRet;
}

BOOL GetValidPath(__in LPCWSTR wzInfPath, __out_ecount_full(nSize) WCHAR* wzFullInfPath, __in int nSize)
{
    int srcLen = (int)wcslen(wzInfPath);
    WCHAR wzCurDir[MAX_PATH+1] = {0};

    memset(wzFullInfPath, 0, sizeof(WCHAR)*nSize);

    if(srcLen > 3)
    {
        if(L'\\'==wzInfPath[0] && L'\\'==wzInfPath[1])
        {
            // Network path, wrong
            return FALSE;
        }
        else if(L':'==wzInfPath[1] && L'\\'==wzInfPath[2])
        {
            wcsncpy_s(wzFullInfPath, nSize, wzInfPath, _TRUNCATE);
            return TRUE;
        }
    }

    GetCurrentDirectory(MAX_PATH, wzCurDir);
    srcLen = (int)wcslen(wzCurDir);
    if(L'\\' != wzCurDir[srcLen-1])
    {
        wzCurDir[srcLen] = L'\\';
        wzCurDir[srcLen+1] = L'\0';
    }
    std::wstring strFullPath(wzCurDir);
    strFullPath += wzInfPath;
    wcsncpy_s(wzFullInfPath, nSize, strFullPath.c_str(), _TRUNCATE);
    return TRUE;
}

int cmdInstall(__in LPCWSTR wzInfPath, __in LPCWSTR wzHwId)
{
    int             nRet          = NLDEVCON_FAIL_TO_INSTALL;
    HDEVINFO        DeviceInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID            ClassGUID;
    WCHAR           ClassName[MAX_CLASS_NAME_LEN] = {0};
    WCHAR           hwIdList[LINE_LEN+4] = {0};
    WCHAR           wzFullInfPath[MAX_PATH+1] = {0};

    if(!GetValidPath(wzInfPath, wzFullInfPath, MAX_PATH))
        goto _exit;

    if(INVALID_FILE_ATTRIBUTES==GetFileAttributes(wzFullInfPath))
        goto _exit;

    // The hwIdList must end with L"\0\0"
    ZeroMemory(hwIdList, sizeof(hwIdList));
    wcsncpy_s(hwIdList, LINE_LEN, wzHwId, _TRUNCATE);

    // Use the INF File to extract the Class GUID.
    if (!SetupDiGetINFClassW(wzFullInfPath, &ClassGUID, ClassName, sizeof(ClassName)/sizeof(ClassName[0]), 0))
        goto _exit;

    // Create the container for the to-be-created Device Information Element.
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
        goto _exit;

    // Now create the element.
    // Use the Class GUID and Name from the INF file.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet, ClassName, &ClassGUID, NULL, 0, DICD_GENERATE_ID, &DeviceInfoData))
        goto _exit;

    // Add the HardwareID to the Device's HardwareID property.
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet, &DeviceInfoData, SPDRP_HARDWAREID, (LPBYTE)hwIdList, (lstrlen(hwIdList)+1+1)*sizeof(TCHAR)))
        goto _exit;

    // Transform the registry element into an actual devnode in the PnP HW tree.
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE, DeviceInfoSet, &DeviceInfoData))
        goto _exit;

    nRet = cmdUpdate(wzFullInfPath, wzHwId);

_exit:    
    if (DeviceInfoSet != INVALID_HANDLE_VALUE) SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    if(NLDEVCON_SUCCESS == nRet)
        printf("Device has been installed succefully\n");
    else if(NLDEVCON_SUCCESS_NEED_REBOOT == nRet)
        printf("Device has been installed succefully, but reboot is required\n");
    else
        printf("Fail to install this device\n");
    return nRet;
}

int cmdUninstall(__in LPCWSTR wzHwId)
{
    int             nRet  = NLDEVCON_FAIL_TO_UNINSTALL; 
    REMOVE_CONTEXT  rmCtx = {FALSE, 0};
    NLDEVCON_ENUM_RESTRICTIONS Restrictions = {NULL, NULL, wzHwId};
    
    nRet = EnumerateDevices(&Restrictions, 0, (EnumCallbackFunc)RemoveDeviceCallback, &rmCtx);
    if(NLDEVCON_SUCCESS==nRet && rmCtx.bReboot)
        nRet = NLDEVCON_SUCCESS_NEED_REBOOT;

    return nRet;
}

int RemoveDeviceCallback(__in HDEVINFO Devs, __in PSP_DEVINFO_DATA DevInfo, __in const CDevRegProp& DevProps, __in DWORD Index, __in PREMOVE_CONTEXT Context)
{
    SP_REMOVEDEVICE_PARAMS  rmdParams;
    SP_DEVINSTALL_PARAMS    devParams;
    BOOL                    bRemoved = FALSE;
    BOOL                    bReboot  = FALSE;
    int                     i        = 0;

    // need hardware ID before trying to remove, as we wont have it after
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

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    if(!SetupDiSetClassInstallParams(Devs,DevInfo,&rmdParams.ClassInstallHeader,sizeof(rmdParams)) ||
       !SetupDiCallClassInstaller(DIF_REMOVE, Devs, DevInfo))
    {
        // failed to invoke DIF_REMOVE
        bRemoved = FALSE;
    }
    else
    {
        bRemoved = TRUE;
        // see if device needs reboot
        devParams.cbSize = sizeof(devParams);
        if(SetupDiGetDeviceInstallParams(Devs,DevInfo,&devParams) && (devParams.Flags & (DI_NEEDRESTART|DI_NEEDREBOOT)))
        {
            // reboot required
            Context->bReboot = TRUE;
            bReboot = TRUE;
        }
    }
    
    printf("\n");
    if(bRemoved)
    {
        if(!bReboot) printf("%03d ID=%S -> Removed\n", Context->nIndex, devID);
        else printf("%03d ID=%S -> Removed, reboot is required\n", Context->nIndex, devID);
    }
    else
    {
        printf("%03d ID=%S -> Fail to remove\n", Context->nIndex, devID);
    }
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

    Context->nIndex++;
    return NLDEVCON_SUCCESS;
}