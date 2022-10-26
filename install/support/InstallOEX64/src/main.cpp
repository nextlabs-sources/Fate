// InstallOEX64.cpp : Defines the entry point for the console application.
#include "stdafx.h"
#include <stdlib.h>
#include <fstream>
#include "InstallOEX64.h"

#pragma comment(linker,"/subsystem:\"windows\" /entry:\"wmainCRTStartup\"")
// return:	0 represent successful
int wmain ( int argc, WCHAR* argv[] )
{
	////Print Upage
	if ( argc == 1 || ( 0 == _wcsicmp ( argv[1], L"-h" ) || 0 == _wcsicmp ( argv[1], L"--help" ) || 0 == _wcsicmp ( argv[1], L"/?" ) ) )
	{
		PrintUsage ( );
		return 0;
	}
	//If to install
	if ( 0 == _wcsicmp ( argv[1], L"--Install" ) )
	{
		CheckOSVersion();
	}

	else if ( 0 == _wcsicmp ( argv[1], L"--UnInstall" ) )
	{
		UnInstallPlgIn();
	}

	else
	{
		printf ( "Error: Invalid command \n" );
		return -1;
	}

	return 0;
	
}

void PrintUsage ( )
{
	printf ( "NextLabs Application    For OE Plug-in  Utility\n" );
	printf ( "      --Install        Install the Plug-in.\n" );
	printf ( "      --UnInstall      UnInstall the Plug-in.\n" );
}
//Get NextLabs installation directory
BOOL GetNextLabsInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE,  L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Outlook Enforcer installation directory from registry" );
		return FALSE;
	}

	WCHAR InstallDirTemp[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof InstallDirTemp;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)InstallDirTemp, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Outlook Enforcer installation directory from registry" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = InstallDirTemp;

	return TRUE;
}

//Get OE installation directory
BOOL GetOEInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Outlook Enforcer installation directory from registry" );
		return FALSE;
	}

	WCHAR InstallDirTemp[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof InstallDirTemp;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)InstallDirTemp, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Outlook Enforcer installation directory from registry" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = InstallDirTemp;

	return TRUE;
}

//Get PC installation directory
BOOL GetPCInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Policy Controller installation directory from registry" );
		return FALSE;
	}

	WCHAR InstallDirTemp[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof InstallDirTemp;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"PolicyControllerDir", NULL, NULL, (LPBYTE)InstallDirTemp, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Policy Controller installation directory from registry" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = InstallDirTemp;

	return TRUE;
}

//Get PCPluginConfigDir installation directory
BOOL GetPCPluginConfigDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get PCPluginConfigDir installation directory from registry" );
		return FALSE;
	}

	WCHAR InstallDirTemp[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof InstallDirTemp;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"PluginConfigDir", NULL, NULL, (LPBYTE)InstallDirTemp, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get PCPluginConfigDir installation directory from registry" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = InstallDirTemp;

	return TRUE;
}

//Get PCTamperResistanceConfigDir installation directory
BOOL GetPCTamperResistanceConfigDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get PCTamperResistanceConfigDir installation directory from registry" );
		return FALSE;
	}

	WCHAR InstallDirTemp[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof InstallDirTemp;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"TamperResistanceConfigDir", NULL, NULL, (LPBYTE)InstallDirTemp, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get PCTamperResistanceConfigDir installation directory from registry" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = InstallDirTemp;

	return TRUE;
}

//Set PCPluginConfigDir installation directory
BOOL ModifyPCPluginConfigDir ( )
{
	HKEY hKey = 0;
	std::wstring str2set;

	if(GetPCPluginConfigDir(str2set))
	{
		str2set.replace (str2set.find_first_of(L"Program Files (x86)"), 18 ,L"Program Files");
		if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
		{
			printf ( "Can't get PCPluginConfigDir installation directory from registry" );
			return FALSE;
		}

		const BYTE * dataset = (BYTE*)str2set.c_str();
		DWORD length = static_cast<DWORD>(str2set.length());
		if(ERROR_SUCCESS != RegSetValueExW(hKey, L"PluginConfigDir", 0, REG_SZ, dataset, length))
		{
			printf ( "Can't modify PCPluginConfigDir value from registry" );
			return FALSE;
		}

		RegCloseKey ( hKey );
		return TRUE;
	}
	return false;

}

//Set PCTamperResistanceConfigDir installation directory
BOOL ModifyPCTamperResistanceConfigDir ( )
{
	HKEY hKey = 0;
	std::wstring str2set;

	if(GetPCTamperResistanceConfigDir(str2set))
	{
		str2set.replace (str2set.find_first_of(L"Program Files (x86)"), 18 ,L"Program Files");
		if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
		{
			printf ( "Can't get TamperResistanceConfigDir installation directory from registry" );
			return FALSE;
		}

		const BYTE * dataset = (BYTE*)str2set.c_str();
		DWORD length = static_cast<DWORD>(str2set.length());
		if(ERROR_SUCCESS != RegSetValueExW(hKey, L"TamperResistanceConfigDir", 0, REG_SZ, dataset, length))
		{
			printf ( "Can't modify TamperResistanceConfigDir value from registry" );
			return FALSE;
		}

		RegCloseKey ( hKey );
		return TRUE;
	}
	return false;
}

//Get Operating System 32/64 bit information
BOOL GetOSbitInfo ( BOOL& bIs64bit )
{
	#ifdef _M_X64
		bIs64bit = TRUE;
		return TRUE;
	#else
		bIs64bit = FALSE;

		typedef BOOL (WINAPI *Typedef_IsWow64Process) ( HANDLE, PBOOL );

        HMODULE hModule = GetModuleHandleW ( L"kernel32" );
        if (hModule == NULL)
        {
            return FALSE;
        }
		Typedef_IsWow64Process Fun_IsWow64Process = (Typedef_IsWow64Process)GetProcAddress ( hModule, "IsWow64Process" );

		if ( NULL != Fun_IsWow64Process )
		{
			if ( !Fun_IsWow64Process ( GetCurrentProcess ( ), &bIs64bit ) )
			{
				return FALSE;
			}

			return TRUE;
		}

		return TRUE;
	#endif
}

void CheckOSVersion() 
{ 
	OSVERSIONINFO Version; 
	ZeroMemory(&Version,sizeof(OSVERSIONINFO)); 
	Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); 
	GetVersionEx(&Version);         
	if (Version.dwPlatformId==VER_PLATFORM_WIN32_NT)  
	{ 
			if((Version.dwMajorVersion==5)&&(Version.dwMinorVersion==0)) //WIN2K
			{ 
				printf ( "OS's WIN2k" );	
				return ; 
			}
			else if((Version.dwMajorVersion==5)&&(Version.dwMinorVersion>0)) //WINXP
			{ 
				printf ( "OS's WINXP" );	
				return ; 
			}
			else if((Version.dwMajorVersion==6)&&(Version.dwMinorVersion==0)) //VISTA 
			{
				printf ( "OS's VISTA" );	
				return ; 			
			}		
			else if((Version.dwMajorVersion>=6)) //WIN7
			{
				BOOL bIs64bit = TRUE;
				GetOSbitInfo ( bIs64bit );
				//Get OS bit information
				if ( bIs64bit )
				{
					std::wstring PCPluginConfigDir;
					std::wstring PCTamperResistanceConfigDir;
					std::wstring OEInstallPath;

					if(!GetOEInstallDir(OEInstallPath))
					{
						printf ( "Can't get OE installation directory from registry" );
					}
					if(!GetPCPluginConfigDir(PCPluginConfigDir))
					{
						printf ( "Can't get PCPluginConfigDir installation directory from registry" );
					}
					if(!GetPCTamperResistanceConfigDir(PCTamperResistanceConfigDir))
					{
						printf ( "Can't get PCTamperResistanceConfigDir installation directory from registry" );
					}
					
					std::wstring StrTemp1 = PCPluginConfigDir;
					std::wstring StrTemp2 = PCTamperResistanceConfigDir;
					
					PCPluginConfigDir += L"nl_OE_plugin.cfg";
					PCTamperResistanceConfigDir += L"OutlookEnforcer_TamperResistance.cfg";
					std::wstring StrPCPluginConfigDirPath = StrTemp1.replace(StrTemp1.find_first_of(L"Program Files (x86)"), 19, L"Program Files");
					std::wstring StrPCTamperResistanceConfigDirPath = StrTemp2.replace(StrTemp2.find_first_of(L"Program Files (x86)"), 19, L"Program Files");
					
					FilesCopy(PCPluginConfigDir, StrPCPluginConfigDirPath);
					FilesCopy(PCTamperResistanceConfigDir, StrPCTamperResistanceConfigDirPath);

					std::wstring nlOEPluginFileContent;
					std::wstring OutlookEnforcerTamperResistanceFileContent;
					SetnlOEPluginFileContent(OEInstallPath, nlOEPluginFileContent);
					SetOutlookEnforcerTamperResistanceFileContent(OEInstallPath, OutlookEnforcerTamperResistanceFileContent);
					StrPCPluginConfigDirPath += L"nl_OE_plugin.cfg"; 
					StrPCTamperResistanceConfigDirPath += L"OutlookEnforcer_TamperResistance.cfg";
					ModifyFiles(StrPCPluginConfigDirPath, nlOEPluginFileContent);
					ModifyFiles(StrPCTamperResistanceConfigDirPath, OutlookEnforcerTamperResistanceFileContent);
				}
				else //32bits
				{
					return;
				}
			}		
	} 
	else //other OS
	{
			return ; 
	}
}

BOOL ModifyFiles ( const std::wstring& SourcePath, const std::wstring& TextContent)
{
	std::wfstream file(SourcePath.c_str(), std::ios::out);
	if(!file)
	{
		printf("File open fail!");
		return false;
	}
	file << TextContent.c_str();
	file.close();
	return true;
}

//Set nl_OE_plugin.cfg file content
void SetnlOEPluginFileContent(const std::wstring installPath, std::wstring& nlOEPluginFileContent)
{
	nlOEPluginFileContent = installPath + L"bin\\OEService.dll";
}

//Set OutlookEnforcer_TamperResistance.cfg file content
void SetOutlookEnforcerTamperResistanceFileContent(const std::wstring installPath, std::wstring& OutlookEnforcerTamperResistanceFileContent)
{
	std::wstring _installPath = installPath;
	if(_installPath[_installPath.length()-1] == L'/' || _installPath[_installPath.length()-1] == L'\\')
	{	
		_installPath.erase(_installPath.length()-1, 1);
	}
	std::wstring line0 = L"file=ro," + _installPath;
	line0 += L"\n";
	std::wstring line1 = L"file=ro," + _installPath + L"\\bin";
	line1 += L"\n";
	std::wstring line2 = L"file=ro," + _installPath + L"\\config";
	line2 += L"\n";
	std::wstring line3 = L"file=ro," + _installPath + L"\\README.TXT";
	line3 += L"\n";
	std::wstring line4 = L"registry=HKEY_CLASSES_ROOT\\*\\shellex\\ContextMenuHandlers\\CE_Explorer";
	line4 += L"\n";
	std::wstring line5 = L"registry=HKEY_CLASSES_ROOT\\Directory\\shellex\\ContextMenuHandlers\\CE_Explorer";
	line5 += L"\n";
	std::wstring line6 = L"registry=HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Office\\Outlook\\Addins\\msoPEP.msoObj.1";
	line6 += L"\n";
	std::wstring line7 = L"registry=HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Office\\Excel\\Addins\\CEOffice.Office";
	line7 += L"\n";
	std::wstring line8 = L"registry=HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Office\\PowerPoint\\Addins\\CEOffice.Office";
	line8 += L"\n";
	std::wstring line9 = L"registry=HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Office\\Word\\Addins\\CEOffice.Office";
	line9 += L"\n";
	std::wstring line10 = L"registry=HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer";
	line10 += L"\n";
	OutlookEnforcerTamperResistanceFileContent = line0 + line1 + line2 + line3 + line4 + line4 + line5 + line6 + line7 + line8 + line9 + line10;
}

//Files copy 
void FilesCopy(const std::wstring& SourcePath, const std::wstring& DestinationPath)
{
	if(0 == ::CopyFile(SourcePath.c_str(),DestinationPath.c_str(),TRUE))
	{
		printf("Files Copy Error");
	}

}
//UnInstall PlgIn
void UnInstallPlgIn()
{
	std::wstring PCInstallerDir ;

	if(!GetPCInstallDir(PCInstallerDir))
	{
		printf ( "Can't get PCPluginConfigDir installation directory from registry.PC have'n install yet" );
	}

	if(PCInstallerDir[PCInstallerDir.length()-1]!=L'/' && PCInstallerDir[PCInstallerDir.length()-1]!=L'\\')
	{	
		PCInstallerDir+=L"\\";
	}
  	std::wstring wstrOEPluginCfg = PCInstallerDir + L"config\\plugin\\nl_OE_plugin.cfg";
	std::wstring wstrOETamperCfg = PCInstallerDir + L"config\\tamper_resistance\\OutlookEnforcer_TamperResistance.cfg";

	if( 0 == ::DeleteFile(wstrOEPluginCfg.c_str()))
	{
		printf("Files Del Error");
	}
	if( 0 == ::DeleteFile(wstrOETamperCfg.c_str()))
	{
		printf("Files Del Error");
	}
}