#ifndef SETUP_FILES_HEADER
#define SETUP_FILES_HEADER

#define COMMON_INSTALL_DLL_KEY "installer.dll"
#define COMMON_INSTALL_DLL_FILE FOLDER_TEMP ^ "installercommon.dll"

#define COMM_PROFILE_TEMPLATE_KEY "commprofile.tmplte"
#define COMM_PROFILE_TEMPLATE_FILE FOLDER_TEMP ^ "commprofile.template.xml"

#define ONLINE_HELP_INDEX_TEMPLATE_KEY "index.html.tmplte"
#define ONLINE_HELP_INDEX_TEMPLATE_FILE FOLDER_TEMP ^ "Index.template.html"

#define ONLINE_HELP_INFORMATION_TEMPLATE_KEY "information.html.tmplte"
#define ONLINE_HELP_INFORMATION_TEMPLATE_FILE FOLDER_TEMP ^ "Information.template.html"

#define ONLINE_HELP_NOTIFICATIONS_TEMPLATE_KEY "notifications.html.tmplte"
#define ONLINE_HELP_NOTIFICATIONS_TEMPLATE_FILE FOLDER_TEMP ^ "Notifications.template.html"

#define LOGGING_PROPERTIES_TEMPLATE_KEY "logging.properties"
#define LOGGING_PROPERTIES_TEMPLATE_FILE FOLDER_TEMP ^ "logging.template.properties"

#define PDP_STOP_DLL_KEY "pdpstop.dll"
#define PDP_STOP_DLL_FILE FOLDER_TEMP ^ "PDPStop.dll"

export prototype INT ExtractSetupFiles(HWND);
export prototype INT InstallNextlabsPublicCert(HWND);
export prototype INT UninstallNextlabsPublicCert(HWND);
export prototype INT InstallNLCC(HWND);
export prototype INT UninstallNLCC(HWND);
export prototype INT DisableServiceStartByName(HWND,STRING);
export prototype INT DisableServiceStart(HWND);
export prototype INT DeleteServiceByName(HWND,STRING);
export prototype INT DeleteServices(HWND);
export prototype INT DeleteFlagRegistryCheck(HWND);
export prototype INT backupFiles(HWND);
export prototype INT WriteBackupLocationToReg(HWND);
export prototype INT RecursiveDelete(HWND,STRING);

export prototype INT InsertDeleteInRunonce(HWND);
export prototype INT RestoreBackupAll(HWND);

export prototype INT DeleteSetupFile(HWND);
export prototype INT checkAndDelete(HWND,STRING);
export prototype INT CleanUp(HWND);

export prototype INT CheckAdminUser(HWND);
export prototype INT Uninstall_nlinjection(HWND);
export prototype INT Install_nlinjection(HWND);

export prototype INT AddSafeMode(HWND);
export prototype INT RemoveSafeMode(HWND);

export prototype INT InsertDateToRegistry(HWND);
export prototype INT createUninstallShortcut(HWND);


#endif
