///////////////////////////////////////////////////////////////////////////////
//
//    (C) Copyright NextLabs, Inc.
//    All Rights Reserved
//
//
//    MODULE:
//
//      nlDevenfInstaller.cpp
//
//    ABSTRACT:
//
//      This code is to install a device enforcer driver. 
//
//    AUTHOR(S):
//
//      Nextlabs, Inc.
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <wdfinstaller.h>
#include <newdev.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <strsafe.h>

namespace {
/////////////////////
// Private GLOBALS //
/////////////////////
LPCTSTR   infFile;
LPCTSTR   hardwareID;
BOOLEAN   IsInstall;
WCHAR     INFFileWithPath[255];

///////////////
// CONSTANTS //
///////////////
#define USAGE_STRING "Usage: nlDevenfInstaller {-i|-r} inf hwid\n\n"  \
                     "  {-i|-r} -- Indicates whether this is an "  \
                     "installation or a removal. As to today, the " \
					 "remove functionality is not implemented.\n" \
					 "  inf -- Specifies an INF file with installation " \
					 "information for the device\n " \
                     " hwid -- Specifies a hardware ID for the device\n"

#define INSTALL_OR_REMOVE_OPT   1
#define INF_FILE_NAME_OPT       2
#define HWID_OPT                3
#define ARGUMENT_COUNT          4

//
// exit codes
//
#define EXIT_OK      (0)
#define EXIT_REBOOT  (1)
#define EXIT_FAIL    (2)
#define EXIT_USAGE   (3)

//Runtime loaded function names   
#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfA"
#endif

//Runtime loaded function pointer
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(__in HWND hwndParent,
                                                              __in LPCTSTR HardwareId,
                                                              __in LPCTSTR FullInfPath,
                                                              __in DWORD InstallFlags,
                                                              __out_opt PBOOL bRebootRequired
															  );
///////////////////////////////////
// Functions scoped in this file //
///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
//  ParseParameters
//
//      Subroutine to deal with parsing all of the parameters. It fills in some 
//      globals. 
//
//  INPUTS:
//
//      argc, argv...
//
//  OUTPUTS:
//
//      None
//
//  RETURNS:
//
//      TRUE if successful, FALSE otherwise
//
//  NOTES:
//
///////////////////////////////////////////////////////////////////////////////
BOOLEAN
ParseParameters(
    int argc, 
    WCHAR **argv
    ) 
{
    if (argc != ARGUMENT_COUNT) {
        printf(USAGE_STRING);
        return FALSE;
    }

    //
    // Install or remove?
    //
    if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-i") == 0) {
        IsInstall = TRUE;
    } else if (_wcsicmp(argv[INSTALL_OR_REMOVE_OPT], L"-r") == 0) {
        IsInstall = FALSE;
    } else {
        printf("Unknown option %ls\n", argv[1]);
        return FALSE;
    }

    //
    // INF file name ...
    //
    infFile = argv[INF_FILE_NAME_OPT];

    //
    // And the SYS...
    // 
    hardwareID = argv[HWID_OPT];

    return TRUE;
}

int doUpdate()
/*++

Routine Description:
    UPDATE
    update driver for existing device(s)

Arguments:
	None

Return Value:

    EXIT_xxxx

--*/
{
    HMODULE newdevMod = NULL;
    int failcode = EXIT_FAIL;
    UpdateDriverForPlugAndPlayDevicesProto UpdateFn;
    BOOL reboot = FALSE;
    LPCTSTR inf = NULL;
    DWORD flags = 0;
    DWORD res;
    TCHAR InfPath[MAX_PATH];

    //
    // Inf must be a full pathname
    //
    res = GetFullPathName(infFile,MAX_PATH,InfPath,NULL);
    if((res >= MAX_PATH) || (res == 0)) {
        //
        // inf pathname too long
        //
        return EXIT_FAIL;
    }
    if(GetFileAttributes(InfPath)==(DWORD)(-1)) {
        //
        // inf doesn't exist
        //
        return EXIT_FAIL;
    }
    inf = InfPath;
    flags |= INSTALLFLAG_FORCE;

    //
    // make use of UpdateDriverForPlugAndPlayDevices
    //
    newdevMod = LoadLibrary(TEXT("newdev.dll"));
    if(!newdevMod) {
        goto final;
    }
    UpdateFn = (UpdateDriverForPlugAndPlayDevicesProto)GetProcAddress(newdevMod,UPDATEDRIVERFORPLUGANDPLAYDEVICES);
    if(!UpdateFn)
    {
        goto final;
    }

    printf("Updating drivers for %ws from %ws\n", hardwareID, inf); 

    if(!UpdateFn(NULL,hardwareID,inf,flags,&reboot)) {
 	   printf("Updating drivers for %ws from %ws failed: err=%d\n", hardwareID, inf, GetLastError());
       goto final;
    }

    printf("Drivers installed successfully.\n");

    failcode = reboot ? EXIT_REBOOT : EXIT_OK;

final:

    if(newdevMod) {
        FreeLibrary(newdevMod);
    }

    return failcode;
}

int DoInstallation()
/*++

Routine Description:

    CREATE
    Creates a root enumerated devnode and installs drivers on it

Return Value:

    EXIT_xxxx

--*/
{
    HDEVINFO DeviceInfoSet = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    GUID ClassGUID;
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    TCHAR hwIdList[LINE_LEN+4];
    TCHAR InfPath[MAX_PATH];
    DWORD err;
    int failcode = EXIT_FAIL;
    BOOL reboot = FALSE;
    DWORD flags = 0;
    DWORD len;

    //
    // Inf must be a full pathname
    //
    if(GetFullPathName(infFile,MAX_PATH,InfPath,NULL) >= MAX_PATH) {
        //
        // inf pathname too long
        //
        return EXIT_FAIL;
    }
	printf("Get inf file path: %ws\n", InfPath);

    //
    // List of hardware ID's must be double zero-terminated
    //
    ZeroMemory(hwIdList,sizeof(hwIdList));
    if (FAILED(StringCchCopy(hwIdList,LINE_LEN,hardwareID))) {
        goto final;
    }

    //
    // Use the INF File to extract the Class GUID.
    //
    if (!SetupDiGetINFClass(InfPath,&ClassGUID,ClassName,sizeof(ClassName)/sizeof(ClassName[0]),0))
    {
        goto final;
    }

    //
    // Create the container for the to-be-created Device Information Element.
    //
    DeviceInfoSet = SetupDiCreateDeviceInfoList(&ClassGUID,0);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        goto final;
    }

    //
    // Now create the element.
    // Use the Class GUID and Name from the INF file.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiCreateDeviceInfo(DeviceInfoSet,
        ClassName,
        &ClassGUID,
        NULL,
        0,
        DICD_GENERATE_ID,
        &DeviceInfoData))
    {
        goto final;
    }

    //
    // Add the HardwareID to the Device's HardwareID property.
    //
    if(!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
        &DeviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)hwIdList,
        (lstrlen(hwIdList)+1+1)*sizeof(TCHAR)))
    {
        goto final;
    }

    //
    // Transform the registry element into an actual devnode
    // in the PnP HW tree.
    //
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        DeviceInfoSet,
        &DeviceInfoData))
    {
        goto final;
    }

    printf("Device node created. Install is complete when drivers are installed...\n");
    //
    // update the driver for the device we just created
    //
    failcode = doUpdate();

final:

    if (DeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    }

	if(failcode == EXIT_FAIL) 
		printf("Installation failed\n");

	return EXIT_OK;
}
}

void __cdecl wmain(int argc, WCHAR **argv) 
{

    //
    // Do all of the necessary initialization
    //
    if (!ParseParameters(argc, argv)) {
        //
        // Whoever failed printed out an appropriate error,
        //  just return.
        //
        return;
    }

     //
    // Let the user know what she's doing...
    //
    printf("You want to %s %ls. The INF file is located at %ls\n",
        IsInstall ? "INSTALL" : "REMOVE", 
        hardwareID,
        infFile);

    //
    // Take the appropriate action...
    //
    if (IsInstall) {
        DoInstallation();
    } else {
        //TODO: DoRemove();
    }

    return;

}

