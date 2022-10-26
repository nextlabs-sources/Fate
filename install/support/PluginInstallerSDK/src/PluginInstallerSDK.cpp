// PluginInstallerSDK.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "PluginInstallerSDK.h"
#include <msiquery.h>
#include <stdlib.h>


wchar_t* policyControllerInstallDirectory;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		policyControllerInstallDirectory = new wchar_t[MAX_PATH_LENGTH + 1];
		policyControllerInstallDirectory[0] = '\0';

		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		delete []policyControllerInstallDirectory;
		break;
	}

    return TRUE;
}

BOOL userInterfaceDisplayed(MSIHANDLE hInstall)
{
	DWORD userInterfaceLevelBufferSize = 2;
	wchar_t* userInterfaceLevelBuffer = new wchar_t[userInterfaceLevelBufferSize];
	UINT result;
	wchar_t* errorAsString = NULL;

	result = MsiGetProperty(hInstall, L"UILevel", userInterfaceLevelBuffer, &userInterfaceLevelBufferSize);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Unknown error while trying to read the UILevel property.  Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return FALSE;
	}

	if ((wcscmp(userInterfaceLevelBuffer, L"2") == 0) || (wcscmp(userInterfaceLevelBuffer, L"3") == 0))
	{
		return FALSE;
	}
	else 
	{
		return TRUE;
	}
}

int loadPolicyControllerInstallPath(MSIHANDLE hInstall)
{
	LONG result;

	wchar_t* errorAsString = NULL;
	HKEY policyControllerKey;
	DWORD policyControllerInstallDirectorySize = MAX_PATH_LENGTH + 1;

	result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, POLICY_CONTROLLER_REG_SUBKEY, 0, KEY_QUERY_VALUE, &policyControllerKey);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);
		writeToInstallerLog(hInstall, L"Failed to open policy controller registry key.  Please ensure that the Policy Controller is installed.  Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;

		return ERROR_INSTALL_FAILURE;
	}

	result = RegQueryValueExW(policyControllerKey, POLICY_CONTROLLER_INSTALL_DIR_REG_VALUE_NAME, NULL, NULL, (BYTE*) policyControllerInstallDirectory, &policyControllerInstallDirectorySize);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);
		writeToInstallerLog(hInstall, L"Failed to read Policy Controller Installer registry value.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
	
		RegCloseKey(policyControllerKey);

		return ERROR_INSTALL_FAILURE;
	}

	RegCloseKey(policyControllerKey);
	
	return ERROR_SUCCESS;
}

UINT __stdcall initializeEnforcerInstall(MSIHANDLE hInstall)
{
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin initializeEnforcerInstall");

	result = setEnforcerPluginTargetPath(hInstall);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Failed to initialize enforcer installer. Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}

	result = setTamperResistenceConfigPath(hInstall);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Failed to initialize enforcer installer. Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}

	result = setPluginConfigPath(hInstall);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Failed to initialize enforcer installer. Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}

	result = setServiceInjectionPath(hInstall);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Failed to initialize enforcer installer. Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}

	writeToInstallerLog(hInstall, L"End initializeEnforcerInstall");

	return result;
}

UINT __stdcall setEnforcerPluginTargetPath(MSIHANDLE hInstall)
{
	wchar_t* pluginFolder;
	DWORD pluginFolderSize;
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setEnforcerPluginTargetPath");

	pluginFolder = new wchar_t[MAX_PATH_LENGTH + 1];
	pluginFolderSize = MAX_PATH_LENGTH;

	MsiGetPropertyW(hInstall, PLUGIN_FOLDER_PROPERTY_NAME, pluginFolder, &pluginFolderSize);
	result = setEnforcerPluginTargetPathWithPluginFolder(hInstall, pluginFolder);
	if (result != ERROR_SUCCESS)
	{
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10);  
		writeToInstallerLog(hInstall, L"Unknown error while trying to set the Enforcer Plugin Install Path.  Error Code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}

	delete []pluginFolder;

	writeToInstallerLog(hInstall, L"End setEnforcerPluginTargetPath");

	return ERROR_SUCCESS;
}

int setEnforcerPluginTargetPathWithPluginFolder(MSIHANDLE hInstall, wchar_t* pluginFolder)
{
	LONG result;

	wchar_t* errorAsString = NULL;
	wchar_t* targetPath;

	if (wcslen(policyControllerInstallDirectory) == 0)
	{
		result = loadPolicyControllerInstallPath(hInstall);
		if (result != ERROR_SUCCESS)
		{
			// Special case.  This may be do to the Policy Controller not being installed.  Therefore, we inform the end user
			if (userInterfaceDisplayed(hInstall))
			{
				MessageBox(GetForegroundWindow(), L"The Policy Controller could not be found.  Please ensure the Policy Controller is installed before installing this Enforcer plugin.", L"Policy Controller Not Found", MB_ICONERROR);
			}
		
			return result;
		}
	}

	targetPath = new wchar_t[wcslen(policyControllerInstallDirectory) + wcslen(pluginFolder) + 1];
	wcscpy_s(targetPath, wcslen(policyControllerInstallDirectory) + wcslen(pluginFolder) + 1, policyControllerInstallDirectory);
	wcscat_s(targetPath, wcslen(policyControllerInstallDirectory) + wcslen(pluginFolder) + 1, pluginFolder);

	result = MsiSetPropertyW(hInstall, L"INSTALLDIR", targetPath);
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set plugin install directory property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	return result;
}

/**
 *  This is a temporary function.  Ultimately, the tamper resistence
 *  config should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property TAMPER_RESISTENCE_CONFIG_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setTamperResistenceConfigPath(MSIHANDLE hInstall)
{
	LONG result;

	wchar_t* errorAsString = NULL;
	wchar_t* tamperResistenceConfigPath;

	if (wcslen(policyControllerInstallDirectory) == 0)
	{
		result = loadPolicyControllerInstallPath(hInstall);
		if (result != ERROR_SUCCESS)
		{
			// Special case.  This may be do to the Policy Controller not being installed.  Therefore, we inform the end user
			if (userInterfaceDisplayed(hInstall))
			{
				MessageBox(GetForegroundWindow(), L"The Policy Controller could not be found.  Please ensure the Policy Controller is installed before installing this Enforcer plugin.", L"Policy Controller Not Found", MB_ICONERROR);
			}
		
			return result;
		}
	}

	tamperResistenceConfigPath = new wchar_t[wcslen(policyControllerInstallDirectory) + wcslen(TAMPER_RESISTENCE_CONFIG_RELATIVE_PATH) + 1];
	wcscpy_s(tamperResistenceConfigPath, wcslen(policyControllerInstallDirectory) + wcslen(TAMPER_RESISTENCE_CONFIG_RELATIVE_PATH) + 1, policyControllerInstallDirectory);
	wcscat_s(tamperResistenceConfigPath, wcslen(policyControllerInstallDirectory) + wcslen(TAMPER_RESISTENCE_CONFIG_RELATIVE_PATH) + 1, TAMPER_RESISTENCE_CONFIG_RELATIVE_PATH);

	result = MsiSetPropertyW(hInstall, L"TAMPER_RESISTENCE_CONFIG_PATH", tamperResistenceConfigPath);
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set tamper resistence config path.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	return result;
}

/**
 *  This is a temporary function.  Ultimately, the plugin
 *  config should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property PLUGIN_CONFIG_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setPluginConfigPath(MSIHANDLE hInstall)
{
	LONG result;

	wchar_t* errorAsString = NULL;
	wchar_t* pluginConfigPath;

	if (wcslen(policyControllerInstallDirectory) == 0)
	{
		result = loadPolicyControllerInstallPath(hInstall);
		if (result != ERROR_SUCCESS)
		{
			// Special case.  This may be do to the Policy Controller not being installed.  Therefore, we inform the end user
			if (userInterfaceDisplayed(hInstall))
			{
				MessageBox(GetForegroundWindow(), L"The Policy Controller could not be found.  Please ensure the Policy Controller is installed before installing this Enforcer plugin.", L"Policy Controller Not Found", MB_ICONERROR);
			}
		
			return result;
		}
	}

	pluginConfigPath = new wchar_t[wcslen(policyControllerInstallDirectory) + wcslen(PLUGIN_CONFIG_RELATIVE_PATH) + 1];
	wcscpy_s(pluginConfigPath, wcslen(policyControllerInstallDirectory) + wcslen(PLUGIN_CONFIG_RELATIVE_PATH) + 1, policyControllerInstallDirectory);
	wcscat_s(pluginConfigPath, wcslen(policyControllerInstallDirectory) + wcslen(PLUGIN_CONFIG_RELATIVE_PATH) + 1, PLUGIN_CONFIG_RELATIVE_PATH);

	result = MsiSetPropertyW(hInstall, L"PLUGIN_CONFIG_PATH", pluginConfigPath);
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set plugin config path.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	return result;
}


/**
 *  This is a temporary function.  Ultimately, the service
 *  injection should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property SERVICE_INJECTION_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setServiceInjectionPath(MSIHANDLE hInstall)
{
	LONG result;

	wchar_t* errorAsString = NULL;
	wchar_t* serviceInjectionPath;

	if (wcslen(policyControllerInstallDirectory) == 0)
	{
		result = loadPolicyControllerInstallPath(hInstall);
		if (result != ERROR_SUCCESS)
		{
			// Special case.  This may be do to the Policy Controller not being installed.  Therefore, we inform the end user
			if (userInterfaceDisplayed(hInstall))
			{
				MessageBox(GetForegroundWindow(), L"The Policy Controller could not be found.  Please ensure the Policy Controller is installed before installing this Enforcer plugin.", L"Policy Controller Not Found", MB_ICONERROR);
			}
		
			return result;
		}
	}

	serviceInjectionPath = new wchar_t[wcslen(policyControllerInstallDirectory) + wcslen(SERVICE_INJECTION_RELATIVE_PATH) + 1];
	wcscpy_s(serviceInjectionPath, wcslen(policyControllerInstallDirectory) + wcslen(SERVICE_INJECTION_RELATIVE_PATH) + 1, policyControllerInstallDirectory);
	wcscat_s(serviceInjectionPath, wcslen(policyControllerInstallDirectory) + wcslen(SERVICE_INJECTION_RELATIVE_PATH) + 1, SERVICE_INJECTION_RELATIVE_PATH);

	result = MsiSetPropertyW(hInstall, L"SERVICE_INJECTION_PATH", serviceInjectionPath);
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set service injection path.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	return result;
}


int setPathFromProperty(MSIHANDLE hInstall, wchar_t* propertyName, wchar_t* pathName)
{
	wchar_t* propertyValue;
	DWORD propertyValueSize;
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setPathFromProperty");

	propertyValue = new wchar_t[MAX_PATH_LENGTH + 1];
	propertyValueSize = MAX_PATH_LENGTH;

	MsiGetPropertyW(hInstall, propertyName, propertyValue, &propertyValueSize);
	if (wcslen(propertyValue) == 0)
	{
		writeToInstallerLog(hInstall, L"Failed to find property value from which to set path.");
		return ERROR_INSTALL_FAILURE;
	}

	writeToInstallerLog(hInstall, L"Found property value.  Name=");
	writeToInstallerLog(hInstall, propertyName);
	writeToInstallerLog(hInstall, L" Value=");
	writeToInstallerLog(hInstall, propertyValue);

	writeToInstallerLog(hInstall, L"Setting property value to path, ");
	writeToInstallerLog(hInstall, pathName);
	
	result = MsiSetTargetPathW(hInstall, pathName, propertyValue);
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set directory from property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return result;
	}
				
	delete []propertyValue;

	writeToInstallerLog(hInstall, L"End setPathFromProperty");

	return ERROR_SUCCESS;
}

UINT __stdcall setEnforcerPluginTargetPathFromProperty(MSIHANDLE hInstall)
{
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setEnforcerPluginTargetPathFromProperty");

	result = setPathFromProperty(hInstall, PLUGIN_TARGET_PATH_PROPERTY_NAME, L"INSTALLDIR");
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set plugin install directory from property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	writeToInstallerLog(hInstall, L"End setEnforcerPluginTargetPath");

	return ERROR_SUCCESS;
}

/**
 *  This is a temporary function.  Ultimately, the tamper resistence
 *  config should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property TAMPER_RESISTENCE_CONFIG_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setTamperResistenceConfigPathFromProperty(MSIHANDLE hInstall)
{
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setTamperResistenceConfigPathFromProperty");

	result = setPathFromProperty(hInstall, TAMPER_RESISTENCE_CONFIG_PATH_PROPERTY_NAME, L"TAMPER_RESISTENCE_CONFIG_PATH");
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set tamper resistance config directory from property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	writeToInstallerLog(hInstall, L"End setTamperResistenceConfigPathFromProperty");

	return ERROR_SUCCESS;
}

/**
 *  This is a temporary function.  Ultimately, the plugin
 *  config should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property PLUGIN_CONFIG_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setPluginConfigPathFromProperty(MSIHANDLE hInstall)
{
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setPluginConfigPathFromProperty");

	result = setPathFromProperty(hInstall, PLUGIN_CONFIG_PATH_PROPERTY_NAME, L"PLUGIN_CONFIG_PATH");
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set plugin config directory from property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	writeToInstallerLog(hInstall, L"End setPluginConfigPathFromProperty");

	return ERROR_SUCCESS;
}

/**
 *  This is a temporary function.  Ultimately, the plugin
 *  config should come from the server once a plugin is registered at
 *  the server level.  For now, the plugin installer can call this
 *  function to set the property SERVICE_INJECTION_PATH to use
 *  to install the configuration file
 */
UINT __stdcall setServiceInjectionPathFromProperty(MSIHANDLE hInstall)
{
	wchar_t* errorAsString = NULL;
	int result;

	writeToInstallerLog(hInstall, L"Begin setServiceInjectionPathFromProperty");

	result = setPathFromProperty(hInstall, SERVICE_INJECTION_PATH_PROPERTY_NAME, L"SERVICE_INJECTION_PATH");
	if (result != ERROR_SUCCESS) {
		errorAsString = new wchar_t[129];
		_ltow_s(result, errorAsString, 129, 10); 
		writeToInstallerLog(hInstall, L"Failed to set service injection directory from property.  Error code: ");
		writeToInstallerLog(hInstall, errorAsString);
		delete []errorAsString;
		return ERROR_INSTALL_FAILURE;
	}
				
	writeToInstallerLog(hInstall, L"End setServiceInjectionPathFromProperty");

	return ERROR_SUCCESS;
}
