#ifndef SETUP_FILES_HEADER
#define SETUP_FILES_HEADER 

#define ENFORCER_DEVICE_DRIVER_SYS_KEY "EnforcerDeviceDriver"
#define ENFORCER_DEVICE_DRIVER_SYS_FILE FOLDER_TEMP ^ "nl_SysEncryption.sys"

#define ENFORCER_DEVICE_DRIVER_INF_KEY "EnforcerDeviceDriverInf"
#define ENFORCER_DEVICE_DRIVER_INF_FILE FOLDER_TEMP ^ "NLSE.inf"

export prototype INT ExtractSetupFiles(HWND);      
export prototype INT InsertDeleteInRunonce(HWND);


#endif
