#ifndef PLUGININSTALLERSDK_H
#define PLUGININSTALLERSDK_H

#include <msi.h>

// internal functions/defines
#define POLICY_CONTROLLER_REG_SUBKEY L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller"
#define POLICY_CONTROLLER_INSTALL_DIR_REG_VALUE_NAME L"InstallDir"

#define TAMPER_RESISTENCE_CONFIG_RELATIVE_PATH L"Policy Controller\\config\\tamper_resistance"
#define PLUGIN_CONFIG_RELATIVE_PATH L"Policy Controller\\config\\plugin"
#define SERVICE_INJECTION_RELATIVE_PATH L"Policy Controller\\service\\injection"

#define LOGGING_PREFIX_MESSAGE L"Plugin Installer SDK: "
#define LOGGING_PREFIX_MESSAGE_SIZE wcslen(LOGGING_PREFIX_MESSAGE)

#define MAX_PATH_LENGTH 255

void writeToInstallerLog(MSIHANDLE hInstall, wchar_t* message);
int setEnforcerPluginTargetPathWithPluginFolder(MSIHANDLE hInstall, wchar_t* pluginFolder);
int loadPolicyControllerInstallPath(MSIHANDLE hInstall);
int setPathFromProperty(MSIHANDLE hInstall, wchar_t* propertyName, wchar_t* pathName);

// exported functions
#define PLUGIN_FOLDER_PROPERTY_NAME L"PLUGIN_FOLDER"
#define PLUGIN_TARGET_PATH_PROPERTY_NAME L"PLUGIN_TARGET_PATH"
#define TAMPER_RESISTENCE_CONFIG_PATH_PROPERTY_NAME L"TAMPER_RESISTENCE_CONFIG_PATH_PROPERTY"
#define PLUGIN_CONFIG_PATH_PROPERTY_NAME L"PLUGIN_CONFIG_PATH_PROPERTY"
#define SERVICE_INJECTION_PATH_PROPERTY_NAME L"SERVICE_INJECTION_PATH_PROPERTY"

UINT __stdcall setEnforcerPluginTargetPathFromProperty(MSIHANDLE);
UINT __stdcall setTamperResistenceConfigPathFromProperty(MSIHANDLE); 
UINT __stdcall setPluginConfigPathFromProperty(MSIHANDLE); 
UINT __stdcall setServiceInjectionPathFromProperty(MSIHANDLE); 

UINT __stdcall initializeEnforcerInstall(MSIHANDLE);
UINT __stdcall setEnforcerPluginTargetPath(MSIHANDLE);
UINT __stdcall setTamperResistenceConfigPath(MSIHANDLE); 
UINT __stdcall setPluginConfigPath(MSIHANDLE); 
UINT __stdcall setServiceInjectionPath(MSIHANDLE); 




#endif 
