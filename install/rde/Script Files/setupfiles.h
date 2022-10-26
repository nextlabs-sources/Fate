#ifndef SETUP_FILES_HEADER
#define SETUP_FILES_HEADER 

#define ENFORCER_DEVICE_DRIVER_INSTALLER_DLL_KEY "EnforcerDeviceDriverInstaller"
#define ENFORCER_DEVICE_DRIVER_INSTALLER_DLL_FILE FOLDER_TEMP ^ "nlPnpDriverInstaller.dll"

#define ENFORCER_DEVICE_DRIVER_COINSTALLER_DLL_KEY "EnforcerDeviceDriverCoInstaller"
#define ENFORCER_DEVICE_DRIVER_COINSTALLER_DLL_FILE FOLDER_TEMP ^ "WdfCoInstaller01007.dll"

#define ENFORCER_DEVICE_DRIVER_SYS_KEY "EnforcerDeviceDriver"
#define ENFORCER_DEVICE_DRIVER_SYS_FILE FOLDER_TEMP ^ "nl_devenf.sys"

#define ENFORCER_DEVICE_DRIVER_INF_KEY "EnforcerDeviceDriverInf"
#define ENFORCER_DEVICE_DRIVER_INF_FILE FOLDER_TEMP ^ "nl_devenf.inf"

#define ENFORCER_DEVICE_DRIVER_CAT_KEY "EnforcerDeviceDriverCat"
#define ENFORCER_DEVICE_DRIVER_CAT_FILE FOLDER_TEMP ^ "nl_devenf.cat"

export prototype INT ExtractSetupFiles(HWND);     

export prototype INT AddLicenseToPC(HWND);
	
#endif
