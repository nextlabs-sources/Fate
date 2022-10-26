// Defines the entry point for the console application.
//

#include "..\include\\Main.h"
#include <strsafe.h>
#define OFFICE_PEP_PARAMETER L"--officepep"

#define ADOBE_PEP_PARAMETER L"--adobepep"


// return: 0 represent successful
//		       -1 represent invalid command
//		       1 represent execute failed
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR  lpCmdLine, int  nCmdShow)
{
	LPWSTR *argv;
	int nArgs;

	argv = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL == argv )
	{
		return 0;
	}

	//Print help message
	if ( nArgs == 1 || ( 0 == _wcsicmp ( argv[1], L"-h" ) || 0 == _wcsicmp ( argv[1], L"--help" ) || 0 == _wcsicmp ( argv[1], L"/?" ) ) )
	{
		PrintUsage ( );
		LocalFree(argv);
		return 0;
	}

	AddEnvironmentVariable();

	BOOL bHaveError = FALSE;

	//If need to handle all OE plugins
	if ( 0 == _wcsicmp ( argv[1], L"--oe" ) )
	{
		if ( !HandleAllOEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Explorer plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oeexplorer" ) )
	{
		if ( !HandleOEExplorerPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Msopep plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oemsopep" ) )
	{
		if ( !HandleOEMsopepPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Office plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oeoffice" ) )
	{
		if ( !HandleOEOfficePlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle all WDE plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wde" ) )
	{
		if ( !HandleAllWDEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle WDE Enhancement plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeenhancement" ) )
	{
		if ( !HandleWDEEnhancementPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle WDE IEpep plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeiepep" ) )
	{
		if ( !HandleWDEIEpepPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	// use office pep replace the overlay plugin from now
	else if ( 0 == _wcsicmp ( argv[1], OFFICE_PEP_PARAMETER) )
	{
		if ( !HandleOfficePEPPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle WDE outlook addin
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeoutlookaddin" ) )
	{
		if ( !HandleWDEOutlookAddin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	// use adobe pep to copy adobe plugins
	else if ( 0 == _wcsicmp ( argv[1], ADOBE_PEP_PARAMETER) )
	{
		if ( !HandleAdobePEPPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle all SE plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--se" ) )
	{
		if ( !HandleAllSEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle SE NlPortableEncryption plugin
	else if ( 0 == _wcsicmp ( argv[1], L"--senlpe" ) )
	{
		if ( !HandleSENLPE ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	else
	{
		printf ( "Error: Invalid command \n" );
		LocalFree(argv);
		return -1;
	}

	if ( bHaveError )
	{
		printf ( "Failed \n" );
		LocalFree(argv);
		return 1;
	}

	printf ( "Successful \n" );
	LocalFree(argv);
	return 0;
}

int wmain ( int argc, WCHAR* argv[] )
{
	//Print help message
	if ( argc == 1 || ( 0 == _wcsicmp ( argv[1], L"-h" ) || 0 == _wcsicmp ( argv[1], L"--help" ) || 0 == _wcsicmp ( argv[1], L"/?" ) ) )
	{
		PrintUsage ( );
		return 0;
	}

	BOOL bHaveError = FALSE;

	//If need to handle all OE plugins
	if ( 0 == _wcsicmp ( argv[1], L"--oe" ) )
	{
		if ( !HandleAllOEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Explorer plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oeexplorer" ) )
	{
		if ( !HandleOEExplorerPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Msopep plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oemsopep" ) )
	{
		if ( !HandleOEMsopepPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle OE Office plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--oeoffice" ) )
	{
		if ( !HandleOEOfficePlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle all WDE plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wde" ) )
	{
		if ( !HandleAllWDEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle WDE Enhancement plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeenhancement" ) )
	{
		if ( !HandleWDEEnhancementPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle WDE IEpep plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeiepep" ) )
	{
		if ( !HandleWDEIEpepPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	// use office pep replace the overlay plugin from now
	else if ( 0 == _wcsicmp ( argv[1], OFFICE_PEP_PARAMETER) )
	{
		if ( !HandleOfficePEPPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle WDE outlook addin
	else if ( 0 == _wcsicmp ( argv[1], L"--wdeoutlookaddin" ) )
	{
		if ( !HandleWDEOutlookAddin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	// use adobe pep to copy adobe plugins
	else if ( 0 == _wcsicmp ( argv[1], ADOBE_PEP_PARAMETER) )
	{
		if ( !HandleAdobePEPPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}

	//Need to handle all SE plugins
	else if ( 0 == _wcsicmp ( argv[1], L"--se" ) )
	{
		if ( !HandleAllSEPlugin ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	//Need to handle SE NlPortableEncryption plugin
	else if ( 0 == _wcsicmp ( argv[1], L"--senlpe" ) )
	{
		if ( !HandleSENLPE ( argv[2] ) )
		{
			bHaveError = TRUE;
		}
	}
	
	else
	{
		printf ( "Error: Invalid command \n" );
		return -1;
	}

	if ( bHaveError )
	{
		printf ( "Failed \n" );
		return 1;
	}

	printf ( "Successful \n" );
	return 0;
}

//Print help message
void PrintUsage ( )
{
	printf ( "NextLabs Application Plug-in Registration Utility\n" );
	printf ( "usage: NlRegisterPlugins <module> [<operation>]\n" );
	printf ( "  module     Specify module to register (or unregister).\n" );
	printf ( "      --oe              Register all Enforcer for Microsoft Outlook plug-ins.\n" );
	printf ( "                        Support Outlook 2003 and 2007. Same as --oeexplorer,\n" );
	printf ( "                        --oemsopep and --oeoffice.\n" );
	printf ( "      --oeexplorer      Register Microsoft Outlook plug-in of Enforcer for\n" );
	printf ( "                        Microsoft Outlook.\n" );
	printf ( "      --oemsopep        Register Microsoft Outlook plug-in of Enforcer for\n" );
	printf ( "                        Microsoft Outlook.\n" );
	printf ( "      --oeoffice        Register Microsoft Outlook plug-in of Enforcer for\n" );
	printf ( "                        Microsoft Outlook.\n" );
	printf ( "      --wde             Register all Desktop Enforcer plug-ins. Same as\n" );
	printf ( "                        --wdeenhancement, --wdeiepep, --adobepep \n" );
	printf ( "                        and --wdeoutlookaddin.\n" );
	printf ( "      --wdeenhancement  Register enhancement plug-ins of Desktop Enforcer\n" );
	printf ( "                        for Microsoft Windows.\n" );
	printf ( "      --wdeiepep        Register Internet Explorer plug-ins of Desktop\n" );
	printf ( "                        Enforcer for Microsoft Windows.\n" );
	printf ( "      --officepep       Register Microsoft Office plug-ins that implement\n" );
	printf ( "                        officepep feature. Require Desktop Enforcer for \n");
	printf ( "                        Microsoft Windows. Support Office 2003 and 2007.\n" );
	printf ( "      --adobepep        Register Adobe plug-ins. Support Acrobat Reader 9 and 10.\n");
	printf ( "      --wdeoutlookaddin Register Outlook add-in of Desktop\n" );
	printf ( "                        Enforcer for Microsoft Windows.\n" );
	printf ( "      --se              Register all System Encryption plug-ins. Same as\n" );
	printf ( "                        --senlpe.\n" );
	printf ( "      --senlpe          Register NlPortableEncryption plugin of System\n" );
	printf ( "                        Encryption for Microsoft Windows.\n" );
	printf ( "\n");
	printf ( "  operation  Register or unregister a module. Default is register.\n" );
	printf ( "      --register        Register the specified module.\n" );
	printf ( "      --unregister      Unregister the specified module.\n" );
}

//Register or unregister all OE plugins
BOOL HandleAllOEPlugin ( const WCHAR* arg )
{
	BOOL bHaveError = FALSE;

	if ( !HandleOEExplorerPlugin ( arg ) )
	{
		bHaveError = TRUE;
	}

	if ( !HandleOEMsopepPlugin ( arg ) )
	{
		bHaveError = TRUE;
	}
	if ( !HandleOEOfficePlugin ( arg ) )
	{
		bHaveError = TRUE;
	}

	if ( bHaveError )
	{
		return FALSE;
	}

	return TRUE;
}

//Register or unregister OE Explorer plugins
BOOL HandleOEExplorerPlugin ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Outlook Enforcer installation directory
	if ( !GetOEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bIs64bit = TRUE;

	//Get OS bit information
	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	std::wstring DllName;

	if ( bIs64bit )
	{
		DllName = InstallDir + RelatedOEExplorer64bitName;
	}
	else
	{
		DllName = InstallDir + RelatedOEExplorer32bitName;
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//Register or unregister OE Msopep plugins
BOOL HandleOEMsopepPlugin ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Outlook Enforcer installation directory
	if ( !GetOEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	int OutlookVersion = 0;
	BOOL bIs64Bit = FALSE;

	//Get Outlook version
	if ( !GetOutlookVersion ( OutlookVersion, bIs64Bit ) )
	{
		return FALSE;
	}

	std::wstring DllName;

	switch ( OutlookVersion )
	{
	//Outlook 2003
	case 11:
		{
			DllName = InstallDir + RelatedOEMsopep2K3Name;
		}
		break;

	//Outlook 2007
	case 12:
		{
			DllName = InstallDir + RelatedOEMsopep2K7Name;
		}
		break;

	case 14:
		{
			if ( bIs64Bit )
			{
				DllName = InstallDir + RelatedOEMsopep201064bitName;
			}
			else
			{
				DllName = InstallDir + RelatedOEMsopep201032bitName;
			}
		}
		break;

	default:
		{
			printf ( "Unknown Outlook version \n" );

			return FALSE;
		}
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//Register or unregister OE Office plugins
BOOL HandleOEOfficePlugin ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Outlook Enforcer installation directory
	if ( !GetOEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bIs64bit = FALSE;

	if ( !GetWordbitInfo ( bIs64bit) )
	{
		return FALSE;
	}

	std::wstring DllName;

	if ( bIs64bit )
	{
		DllName = InstallDir + RelatedOEOffice64bitName;
	}
	else
	{
		DllName = InstallDir + RelatedOEOffice32bitName;
	}

	//Need to register
	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	//Need to unregister
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//Register or unregister all WDE plugins
BOOL HandleAllWDEPlugin ( const WCHAR* arg )
{
	BOOL bHaveError = FALSE;

	if ( !HandleWDEEnhancementPlugin ( arg ) )
	{
		bHaveError = TRUE;
	}

	//if ( !HandleWDEIEpepPlugin ( arg ) )
	//{
	//	bHaveError = TRUE;
	//}

	if(!HandleOfficePEPPlugin(arg))	bHaveError = TRUE;

	if ( !HandleWDEOutlookAddin ( arg ) )
	{
		bHaveError = TRUE;
	}

	if ( !HandleAdobePEPPlugin (arg) )
	{

	}

	if ( bHaveError )
	{
		return FALSE;
	}

	return TRUE;
}

//Register or unregister WDE Enhancement plugins
BOOL HandleWDEEnhancementPlugin ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Control Panel installation directory
	if ( !GetControlPanelInstallDir( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bIs64bit = TRUE;

	//Get OS bit information
	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	std::wstring DllName;

	if ( bIs64bit )
	{
		DllName = InstallDir + RelatedWDEEnhancement64bitName;
	}
	else
	{
		DllName = InstallDir + RelatedWDEEnhancement32bitName;
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


//Register or unregister WDE IEpep plugins
BOOL HandleWDEIEpepPlugin ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Desktop Enforcer installation directory
	if ( !GetDEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bIs64bit = TRUE;

	//Get OS bit information
	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	BOOL bHaveError = FALSE;

	std::wstring DllName = InstallDir + RelatedWDEIEpep32bitName;

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		DisableNewMenuKey(true);
		SetPreviewPaneKey(false);
		if ( !RegisterModule ( DllName ) )
		{
			bHaveError = TRUE;
		}

		if ( bIs64bit )
		{
			DllName = InstallDir + RelatedWDEIEpep64bitName;

			if ( !RegisterModule ( DllName ) )
			{
				bHaveError = TRUE;
			}
		}

		if ( !bHaveError )
		{
			//Add register value to protect IEpep from disabled
			if ( !AddIEpepRegisterValue ( ) )
			{
				bHaveError = TRUE;
			}
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		SetPreviewPaneKey(true);
		if ( !UnregisterModule ( DllName ) )
		{
			bHaveError = TRUE;
		}

		if ( bIs64bit )
		{
			DllName = InstallDir + RelatedWDEIEpep64bitName;

			if ( !UnregisterModule ( DllName ) )
			{
				bHaveError = TRUE;
			}
		}
		if ( !bHaveError )
		{
			//Delete register value
			if ( !DeleteIEpepRegisterValue ( ) )
			{
				bHaveError = TRUE;
			}
		}
	}

	if ( bHaveError )
	{
		return FALSE;
	}

	return TRUE;
}

//Register or unregister office pep add-in
BOOL HandleOfficePEPPlugin (__in const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the Desktop Enforcer installation directory
	if ( !GetDEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	int WordVersion = 0;

	//Get Word version
	if ( !GetWordVersion ( WordVersion ) )
	{
		return FALSE;
	}

	std::wstring DllName;

	switch ( WordVersion )
	{
		// wed need to support office2003 overlay and
		// turn off other feature in code.

	case 11:	// word 2003
	case 12:	//Word 2007
		{
			DllName = InstallDir + RelatedOfficePEP32bitName;
		}
		break;
	case 14:	//Word 2010
	case 15:	//Word 2013
	case 16:	//Word 2016
		{
			BOOL bIs64bit = FALSE;

			if ( !GetWordbitInfo ( bIs64bit) )
			{
				return FALSE;
			}

			if ( bIs64bit )
			{
				DllName = InstallDir + RelatedOfficePepName;
			}
			else
			{
				DllName = InstallDir + RelatedOfficePEP32bitName;
			}
		}
		break;

	default:
		{
			printf ( "Unknown Word version \n" );

			return FALSE;
		}
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;

}

//Register or unregister WDE Outlook add-in
BOOL HandleWDEOutlookAddin ( const WCHAR* arg )
{
// 	std::wstring InstallDir;
// 
// 	//Get the Desktop Enforcer installation directory
// 	if ( !GetDEInstallDir ( InstallDir ) )
// 	{
// 		return FALSE;
// 	}
// 
// 	int OutlookVersion = 0;
// 	BOOL bIs64Bit = FALSE;
// 
// 	//Get Outlook version
// 	if ( !GetOutlookVersion ( OutlookVersion, bIs64Bit ) )
// 	{
// 		return FALSE;
// 	}
// 
// 	std::wstring DllName;
// 
// 	//64 bit
// 	if ( bIs64Bit )
// 	{
// 		DllName = InstallDir + RelatedWDEOutlook64bitName;
// 	}
// 	else
// 	{
// 		DllName = InstallDir + RelatedWDEOutlook32bitName;
// 	}
// 
// 	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
// 	{
// 		if ( RegisterModule ( DllName ) )
// 		{
// 			return TRUE;
// 		}
// 	}
// 	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
// 	{
// 		if ( UnregisterModule ( DllName ) )
// 		{
// 			return TRUE;
// 		}
// 	}

	return TRUE;
}

//Register or unregister all SE plugins
BOOL HandleAllSEPlugin ( const WCHAR* arg )
{
	BOOL bHaveError = FALSE;

	if ( !HandleSENLPE ( arg ) )
	{
		bHaveError = TRUE;
	}

	if ( bHaveError )
	{
		return FALSE;
	}

	return TRUE;
}

//Register or unregister SE NlPortableEncryption plugin
BOOL HandleSENLPE ( const WCHAR* arg )
{
	std::wstring InstallDir;

	//Get the SE installation directory
	if ( !GetSEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bIs64bit = TRUE;

	//Get OS bit information
	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	std::wstring DllName;

	if ( bIs64bit )
	{
		DllName = InstallDir + RelatedSEExplorer64bitName;
	}
	else
	{
		DllName = InstallDir + RelatedSEExplorer32bitName;
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
		if ( RegisterModule ( DllName ) )
		{
			return TRUE;
		}
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		if ( UnregisterModule ( DllName ) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//Get DE installation directory
BOOL GetDEInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Desktop Enforcer installation directory from registry \n" );
		return FALSE;
	}

	WCHAR DEInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof DEInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)DEInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Desktop Enforcer installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = DEInstallDir;

	return TRUE;
}


//Get Common installation directory
BOOL GetCommonInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\CommonLibraries", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Common installation directory from registry \n" );
		return FALSE;
	}

	WCHAR CommonInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof CommonInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)CommonInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Common installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = CommonInstallDir;

	return TRUE;
}

//Get Control Panel installation directory
BOOL GetControlPanelInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Control Panel installation directory from registry \n" );
		return FALSE;
	}

	WCHAR ControlPanelInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof ControlPanelInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"ControlPanelDir", NULL, NULL, (LPBYTE)ControlPanelInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Control Panel installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = ControlPanelInstallDir;

	return TRUE;
}

//Get OE installation directory
BOOL GetOEInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Outlook Enforcer installation directory from registry \n" );
		return FALSE;
	}

	WCHAR DEInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof DEInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)DEInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Outlook Enforcer installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = DEInstallDir;

	return TRUE;
}

BOOL GetOEBinDir ( std::wstring& OEBinDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Outlook Enforcer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get Outlook Enforcer installation directory from registry \n" );
		return FALSE;
	}

	WCHAR DEInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof DEInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"OutlookEnforcerDir", NULL, NULL, (LPBYTE)DEInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Outlook Enforcer installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	OEBinDir = DEInstallDir;

	return TRUE;
}


//Get SE installation directory
BOOL GetSEInstallDir ( std::wstring& InstallDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Enterprise DLP\\System Encryption", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't get System Encryption installation directory from registry \n" );
		return FALSE;
	}

	WCHAR SEInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof SEInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"InstallDir", NULL, NULL, (LPBYTE)SEInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get System Encryption installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	InstallDir = SEInstallDir;

	return TRUE;
}

//Get PC Tamper directory
BOOL GetPCTamperDir ( std::wstring& PCTamperDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Can't open Policy Controller registry key!!\n" );
		return FALSE;
	}

	WCHAR PCInstallDir[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof PCInstallDir;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"PolicyControllerDir", NULL, NULL, (LPBYTE)PCInstallDir, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Can't get Policy Controller installation directory from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	PCTamperDir = PCInstallDir;

	return TRUE;
}

//Get regsvr32.exe name
BOOL GetRegsvr32Name ( std::wstring& strRegsvr32Name )
{
	WCHAR Regsvr32Name[MAX_PATH] = { 0 };

	if ( 0 == GetSystemDirectoryW ( Regsvr32Name, MAX_PATH ) )
	{
		return FALSE;
	}

	strRegsvr32Name = Regsvr32Name;
	strRegsvr32Name += L"\\regsvr32.exe";

	return TRUE;
}

//Get Word version
BOOL GetWordVersion ( int& Version )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_CLASSES_ROOT, L"Word.Application\\CurVer", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		return FALSE;
	}

	WCHAR VersionInfo[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof VersionInfo;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, NULL, NULL, NULL, (LPBYTE)VersionInfo, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		return FALSE;
	}

	RegCloseKey ( hKey );

	if ( 0 == _wcsicmp ( VersionInfo, L"Word.Application.16" ) )
	{
		Version = 16;
	}
	else if ( 0 == _wcsicmp ( VersionInfo, L"Word.Application.15" ) )
	{
		Version = 15;
	}
	else if ( 0 == _wcsicmp ( VersionInfo, L"Word.Application.14" ) )
	{
		Version = 14;
	}
	else if ( 0 == _wcsicmp ( VersionInfo, L"Word.Application.12" ) )
	{
		Version = 12;
	}
	else if ( 0 == _wcsicmp ( VersionInfo, L"Word.Application.11" ) )
	{
		Version = 11;
	}
	else
	{
		Version = 0;
	}

	return TRUE;
}

//Get Word 32/64bit information
BOOL GetWordbitInfo ( BOOL& bIs64bit )
{
	bIs64bit = FALSE;

	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Winword.exe", 0, KEY_QUERY_VALUE, &hKey ) )
	{
		return FALSE;
	}

	WCHAR WinwordFileName[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof WinwordFileName;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, NULL, NULL, NULL, (LPBYTE)WinwordFileName, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		return FALSE;
	}

	RegCloseKey ( hKey );

	DWORD BinaryType = 0;

	if ( !GetBinaryTypeW ( WinwordFileName, &BinaryType ) )
	{
		return FALSE;
	}

	switch ( BinaryType )
	{
	case SCS_64BIT_BINARY:
		{
			bIs64bit = TRUE;
		}
		return TRUE;

	case SCS_32BIT_BINARY:
		{

		}
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

//Get Outlook version and 32/64 bit
BOOL GetOutlookVersion ( int& Version, BOOL& bIs64Bit )
{
	Version = 0;
	bIs64Bit = FALSE;

	WCHAR pszaOutlookQualifiedComponents[][MAX_PATH] =
	{
		L"{1E77DE88-BCAB-4C37-B9E5-073AF52DFD7A}",		// Outlook 2010
		L"{24AAE126-0911-478F-A019-07B875EB9996}",			// Outlook 2007
		L"{BC174BAD-2F53-4855-A1D5-0D575C19B1EA}"		// Outlook 2003
	};

	int nOutlookQualifiedComponents = _countof ( pszaOutlookQualifiedComponents );

	int i = 0;
	UINT ret = 0;
	DWORD dwValueBuf = 0;

	for ( i = 0; i < nOutlookQualifiedComponents; i++ )
	{
		ret = MsiProvideQualifiedComponentW ( pszaOutlookQualifiedComponents[i], L"outlook.x64.exe", (DWORD) INSTALLMODE_DEFAULT, NULL, &dwValueBuf );

		if ( ERROR_SUCCESS == ret )
		{
			break;
		}
	}

	if ( ERROR_SUCCESS != ret )
	{
		for ( i = 0; i < nOutlookQualifiedComponents; i++ )
		{
			ret = MsiProvideQualifiedComponentW ( pszaOutlookQualifiedComponents[i], L"outlook.exe", (DWORD) INSTALLMODE_DEFAULT, NULL, &dwValueBuf );

			if ( ERROR_SUCCESS == ret )
			{
				break;
			}
		}
	}
	else
	{
		bIs64Bit = TRUE;
	}

	if ( ERROR_SUCCESS == ret )
	{
		dwValueBuf += 1;

		WCHAR* pszTempPath = new WCHAR[dwValueBuf]();

		if ( ERROR_SUCCESS != MsiProvideQualifiedComponentW ( pszaOutlookQualifiedComponents[i], L"outlook.exe", (DWORD) INSTALLMODE_EXISTING, pszTempPath, &dwValueBuf ) )
		{
			delete []pszTempPath;
			return FALSE;
		}

		WCHAR* pszTempVer = new WCHAR[MAX_PATH]();

		dwValueBuf = MAX_PATH;

		if ( ERROR_SUCCESS != MsiGetFileVersionW ( pszTempPath, pszTempVer, &dwValueBuf, NULL, NULL ) )
		{
			delete []pszTempPath;
			delete []pszTempVer;
			return FALSE;
		}

		if ( 0 == wcsncmp ( L"11", pszTempVer, wcslen ( L"11" ) ) )
		{
			Version = 11;
		}
		else if ( 0 == wcsncmp ( L"12", pszTempVer, wcslen ( L"12" ) ) )
		{
			Version = 12;
		}
		else if ( 0 == wcsncmp ( L"14", pszTempVer, wcslen ( L"14" ) ) )
		{
			Version = 14;
		}

		delete []pszTempPath;
		delete []pszTempVer;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
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

		Typedef_IsWow64Process Fun_IsWow64Process = NULL;
		HMODULE hMod = GetModuleHandleW ( L"kernel32" );
		if(hMod !=NULL)	Fun_IsWow64Process = (Typedef_IsWow64Process)GetProcAddress (hMod , "IsWow64Process" );

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

//Register module
BOOL RegisterModule ( const std::wstring& ModuleName )
{
	std::wstring Regsvr32Name;

	if ( !GetRegsvr32Name ( Regsvr32Name ) )
	{
		return FALSE;
	}

	WCHAR CommandLine[MAX_PATH * 2] = { 0 };

	//Produce command line
	swprintf_s ( CommandLine, MAX_PATH * 2, L"\"%s\" /s \"%s\"", Regsvr32Name.c_str ( ), ModuleName.c_str ( ) );

	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	si.cb = sizeof STARTUPINFOW;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	//Register
	if ( !CreateProcessW ( NULL, CommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
	{
		return FALSE;
	}

	CloseHandle ( pi.hThread );

	//Wait until process exits
	if ( WAIT_FAILED == WaitForSingleObject ( pi.hProcess, INFINITE ) )
	{
		CloseHandle ( pi.hProcess );
		return FALSE;
	}

	DWORD dwExitCode = 0;

	//Get registration result
	if ( !GetExitCodeProcess ( pi.hProcess, &dwExitCode ) )
	{
		CloseHandle ( pi.hProcess );
		return FALSE;
	}

	CloseHandle ( pi.hProcess );

	wprintf ( L"Register %s, ret value: %lu\r\n", ModuleName.c_str(), dwExitCode );

	//Register successfully
	if ( 0 == dwExitCode )
	{
		return TRUE;
	}

	return FALSE;
}

//Unregister module
BOOL UnregisterModule ( const std::wstring& ModuleName )
{
	std::wstring Regsvr32Name;

	if ( !GetRegsvr32Name ( Regsvr32Name ) )
	{
		return FALSE;
	}

	WCHAR CommandLine[MAX_PATH * 2] = { 0 };

	//Produce command line
	swprintf_s ( CommandLine, MAX_PATH * 2, L"\"%s\" /s /u \"%s\"", Regsvr32Name.c_str ( ), ModuleName.c_str ( ) );

	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	si.cb = sizeof STARTUPINFOW;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	//Unregister
	if ( !CreateProcessW ( NULL, CommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
	{
		return FALSE;
	}

	CloseHandle ( pi.hThread );

	//Wait until process exits
	if ( WAIT_FAILED == WaitForSingleObject ( pi.hProcess, INFINITE ) )
	{
		CloseHandle ( pi.hProcess );
		return FALSE;
	}

	DWORD dwExitCode = 0;

	//Get unregistration result
	if ( !GetExitCodeProcess ( pi.hProcess, &dwExitCode ) )
	{
		CloseHandle ( pi.hProcess );
		return FALSE;
	}

	CloseHandle ( pi.hProcess );

	wprintf ( L"Unregister %s, ret value: %lu\r\n", ModuleName.c_str(), dwExitCode );

	//Unregister successfully
	if ( 0 == dwExitCode )
	{
		return TRUE;
	}

	return FALSE;
}


BOOL HandleAdobePEPPlugin (__in const WCHAR* arg )
{
	std::wstring InstallDir;
	std::wstring OEInstallDir;
	std::wstring adobeProductRegistry;
	std::wstring AdobePath;
	std::wstring temp;
	std::wstring sourcefile;
	std::wstring destfile;
	BOOL adobeSuccess = false;

	//Get the Desktop Enforcer installation directory
	if ( !GetDEInstallDir ( InstallDir ) )
	{
		return FALSE;
	}

	BOOL bGetOEDir = TRUE;
	if ( !GetOEBinDir ( OEInstallDir ) )
	{
		bGetOEDir = FALSE;
	}
	BOOL bIs64bit = TRUE;

	//Get OS bit information
	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	if (bIs64bit) {
		adobeProductRegistry = L"SOFTWARE\\Wow6432Node\\Adobe";
	} else {
		adobeProductRegistry = L"SOFTWARE\\Adobe";
	}

	if ( NULL == arg || 0 == _wcsicmp ( arg, RegisterCommandString ) )
	{
	//	sourcefile = InstallDir+L"bin\\NlAdobePEP32.api";

		temp = adobeProductRegistry + AcrobatReader10RegistryKey;
		printf("Finding Acrobat Reader 10.0 ... \n");
		if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
			adobeSuccess = true;
		}
		}

        temp = adobeProductRegistry + AcrobatReader11RegistryKey;
        printf("Finding Acrobat Reader 11.0 ... \n");
        if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
            adobeSuccess = true;
        }

		if (bGetOEDir)
		{
        if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}
		
		temp = adobeProductRegistry + AcrobatReaderDCRegistryKey;//install adobe dc
        printf("Finding Acrobat Reader DC ... \n");
        if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
            adobeSuccess = true;
        }

		if (bGetOEDir)
		{
        if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}

		temp = adobeProductRegistry + AcrobatReader2K15RegistryKey;//install adobe 2015
        printf("Finding Acrobat Reader 2015 ... \n");
        if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
            adobeSuccess = true;
        }

		if (bGetOEDir)
		{
        if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}

		temp = adobeProductRegistry + AcrobatReader2K17RegistryKey;//install adobe 2017
		printf("Finding Acrobat Reader 2017 ... \n");
		if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
			if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
				adobeSuccess = true;
			}
		}

		printf("Finding Acrobat Reader 9.0 ... \n");
		temp = adobeProductRegistry + AcrobatReader9RegistryKey;
		if (ReaderCopyPlugin(InstallDir + L"bin\\NLReaderPEP32.api", temp, L"NLReaderPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (ReaderCopyPlugin(OEInstallDir + L"bin\\CE_Reader32.api", temp, L"CE_Reader32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 11.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat11RegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 10.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat10RegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 9.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat9RegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}
		
		printf("Finding Adobe Acrobat 2015 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat2K15RegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 2017 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat2K17RegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
			if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
				adobeSuccess = true;
			}
		}

		printf("Finding Adobe Acrobat DC ... \n");
		temp = adobeProductRegistry + AdobeAcrobatDCRegistryKey;
		if (AcrobatCopyPlugin(InstallDir + L"bin\\NLAcrobatPEP32.api", temp, L"NLAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatCopyPlugin(OEInstallDir + L"bin\\CE_Acrobat32.api", temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		if ( AddWhiteListRegisterValue() )
		{
			AddWhiteListFile();
		}

		if ( AddWhiteListRegisterValueFor2017() )
		{
			AddWhiteListFileFor2017();
		}

		//AddAdobeOLReflashKeys();
	}
	else if ( 0 == _wcsicmp ( arg, UnregisterCommandString ) )
	{
		temp = adobeProductRegistry + AcrobatReader10RegistryKey;

		printf("Finding Acrobat Reader 10.0 ... \n");
		if (bGetOEDir)
		{
		if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
			adobeSuccess = true;
		}
		}

		if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
			adobeSuccess = true;
		}

        temp = adobeProductRegistry + AcrobatReader11RegistryKey;

        printf("Finding Acrobat Reader 11.0 ... \n");
		if (bGetOEDir)
		{
        if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}

		if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
            adobeSuccess = true;
        }
		
		temp = adobeProductRegistry + AcrobatReaderDCRegistryKey;
        printf("Finding Acrobat Reader DC ... \n");
        if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
            adobeSuccess = true;
        }

		if (bGetOEDir)
		{
		if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}

		temp = adobeProductRegistry + AcrobatReader2K15RegistryKey;

        printf("Finding Acrobat Reader 2015 ... \n");
		if (bGetOEDir)
		{
        if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
            adobeSuccess = true;
        }
		}

        if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
            adobeSuccess = true;
        }

		temp = adobeProductRegistry + AcrobatReader2K17RegistryKey;

		printf("Finding Acrobat Reader 2017 ... \n");
		if (bGetOEDir)
		{
			if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
				adobeSuccess = true;
			}
		}

		if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
			adobeSuccess = true;
		}

		printf("Finding Acrobat Reader 9.0 ... \n");
		temp = adobeProductRegistry + AcrobatReader9RegistryKey;
		if (ReaderDeletePlugin(temp, L"NlReaderPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (ReaderDeletePlugin(temp, L"CE_Reader32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 11.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat11RegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 10.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat10RegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 9.0 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat9RegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 2015 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat2K15RegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}

		printf("Finding Adobe Acrobat 2017 ... \n");
		temp = adobeProductRegistry + AdobeAcrobat2K17RegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
			if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
				adobeSuccess = true;
			}
		}

		printf("Finding Adobe Acrobat DC ... \n");
		temp = adobeProductRegistry + AdobeAcrobatDCRegistryKey;
		if (AcrobatDeletePlugin(temp, L"NlAcrobatPEP32.api")) {
			adobeSuccess = true;
		}

		if (bGetOEDir)
		{
		if (AcrobatDeletePlugin(temp, L"CE_Acrobat32.api")) {
			adobeSuccess = true;
		}
		}
		//DelAdobeOLReflashKeys();
	}


	return adobeSuccess;

}


BOOL GetAdobePath (std::wstring AdobeReg, std::wstring& AdobePathDir )
{
	HKEY hKey = 0;

	if ( ERROR_SUCCESS != RegOpenKeyExW ( HKEY_LOCAL_MACHINE, AdobeReg.c_str(), 0, KEY_QUERY_VALUE, &hKey ) )
	{
		printf ( "Fail to open %ls registry key !!\n", AdobeReg.c_str() );
		return FALSE;
	}

	WCHAR PathRead[MAX_PATH] = { 0 };
	DWORD BufferSize = sizeof PathRead;

	if ( ERROR_SUCCESS != RegQueryValueExW ( hKey, L"Path", NULL, NULL, (LPBYTE)PathRead, &BufferSize ) )
	{
		RegCloseKey ( hKey );
		printf ( "Fail to retrieve Adobe path from registry \n" );
		return FALSE;
	}

	RegCloseKey ( hKey );

	AdobePathDir = PathRead;

	return TRUE;
}

BOOL AcrobatCopyPlugin(std::wstring sourcefile, std::wstring dest, std::wstring dstfile) {//dstfile:NlAcrobatPEP32.api or CE_Acrobat32.api

	std::wstring AdobePath;
	std::wstring destfile;

	if (GetAdobePath(dest, AdobePath)) {
		destfile = AdobePath+L"Acrobat\\Plug_ins\\"+dstfile;
		if (!CopyFile(sourcefile.c_str(), destfile.c_str(), false)) {
			wprintf(L"Fail to copy to %ls!! source: %s\n", destfile.c_str(), sourcefile.c_str());
			return false;
		} else {
			printf("Adobe plugin copied to %ls.\n", destfile.c_str());
			return ProtectAdobeAPI(destfile);
		}
	} else return false;


}

BOOL ReaderCopyPlugin(std::wstring sourcefile, std::wstring dest, std::wstring dstfile) {//dstfile:NlReaderPEP32.api or CE_Reader32.api

	std::wstring AdobePath;
	std::wstring destfile;

	if (GetAdobePath(dest, AdobePath)) {
		destfile = AdobePath+L"Reader\\Plug_ins\\"+dstfile;
		if (!CopyFile(sourcefile.c_str(), destfile.c_str(), false)) {
			wprintf(L"Fail to copy to %ls!!, source: %s\n", destfile.c_str(), sourcefile.c_str());
			return false;
		} else {
			printf("Adobe plugin copied to %ls.\n", destfile.c_str());
			return ProtectAdobeAPI(destfile);
		}
	} else return false;


}


BOOL AcrobatDeletePlugin(std::wstring dest, std::wstring filename) {//NlAcrobatPEP32.api

	std::wstring AdobePath;
	std::wstring destfile;

	if (GetAdobePath(dest, AdobePath)) {
		destfile = AdobePath+L"Acrobat\\Plug_ins\\"+filename;
		if (!DeleteFile(destfile.c_str())) {
			printf("Fail to delete %ls!!\n", destfile.c_str());

			std::wstring newName = destfile + L"bak";
			if (MoveFileW(destfile.c_str(), newName.c_str()))
			{
				MoveFileExW(newName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			else
			{
				MoveFileExW(destfile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}

			return false;
		} else {
			printf("Adobe plugin deleted.\n");
			return true;
		}

	} else return false;


}

BOOL ReaderDeletePlugin(std::wstring dest, std::wstring filename) {//filename: NlReaderPEP32.api or CE_Reader32.api

	std::wstring AdobePath;
	std::wstring destfile;

	if (GetAdobePath(dest, AdobePath)) {
		destfile = AdobePath+L"Reader\\Plug_ins\\"+filename;
		if (!DeleteFile(destfile.c_str())) {
			printf("Fail to delete %ls!!\n", destfile.c_str());
			
			std::wstring newName = destfile + L"bak";
			if (MoveFileW(destfile.c_str(), newName.c_str()))
			{
				MoveFileExW(newName.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			else
			{
				MoveFileExW(destfile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}

			return false;
		} else {
			printf("Adobe plugin deleted.\n");
			return true;
		}

	} else return false;


}

BOOL ProtectAdobeAPI(const std::wstring& destfile) {

	HANDLE hFile;
	std::wstring PCTamperDir;
	std::string databuffer;
	DWORD dwBytestoWrite;

	std::string local_destfile(destfile.begin(), destfile.end());
	databuffer = "\nfile=ro," + local_destfile;
	dwBytestoWrite = static_cast<DWORD>(databuffer.length());

	GetPCTamperDir(PCTamperDir);
	PCTamperDir +=  L"config\\tamper_resistance\\WindowsDesktopEnforcer_TamperResistance.cfg";

	hFile = CreateFileW(PCTamperDir.c_str(), GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
        printf("Error in opening WDE tamper resistance config file!!\n");
		return false;
    }

	SetFilePointer(hFile,0,NULL,FILE_END);

	DWORD dwWritten;

	if (!WriteFile(hFile, databuffer.c_str(),dwBytestoWrite,&dwWritten,NULL)) {
		printf("Error in modifying WDE tamper resistance config file!!\n");
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);
	return true;

}

BOOL AddAdobeOLReflashKeys()
{
	HKEY hk = NULL;
	DWORD dwDisp;

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\OverLayInfo", 0, NULL, REG_OPTION_NON_VOLATILE,KEY_WRITE, NULL, &hk, &dwDisp))
	{
		return FALSE;
	}

	DWORD dwTime = 1;
	if (RegSetValueEx(hk,L"HeartBeat",0,REG_DWORD,(LPBYTE)&dwTime,(DWORD)sizeof(DWORD)))
	{
		RegCloseKey(hk);
		return FALSE;
	}

	RegCloseKey(hk);
	return TRUE;
}

BOOL DelAdobeOLReflashKeys()
{
   LONG lErrorCode = RegDeleteKey(HKEY_LOCAL_MACHINE,L"SOFTWARE\\NextLabs\\OverLayInfo");
   if (lErrorCode != ERROR_SUCCESS)
   {
		return FALSE;
   }
   return TRUE;
}


BOOL AddCbPepKeys()
{
	HKEY hKey = NULL;

	WCHAR GID[] = L"{7ABFB944-EB91-4E60-826E-BC9AB54DD6AB}";

	LONG lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"*\\shellex\\ContextMenuHandlers\\NLCommonBrowserEnforcer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}


	lRet = RegSetValueExW( hKey, NULL, 0, REG_SZ, (BYTE*)GID, sizeof(GID));

	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\ContextMenuHandlers\\NLCommonBrowserEnforcer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	lRet = RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)GID, sizeof(GID));

	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\CopyHookHandlers\\NLCommonBrowserEnforcer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	lRet = RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)GID, sizeof(GID));

	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Folder\\shellex\\ContextMenuHandlers\\NLCommonBrowserEnforcer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	lRet = RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)GID, sizeof(GID));

	RegCloseKey(hKey);

	return TRUE;

}

BOOL DeleteCbPepKeys()
{
	HKEY hKey = NULL;
	LONG lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"*\\shellex\\ContextMenuHandlers", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	RegDeleteKeyW(hKey, L"NLCommonBrowserEnforcer");
	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\ContextMenuHandlers", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	RegDeleteKeyW(hKey, L"NLCommonBrowserEnforcer");
	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Directory\\shellex\\CopyHookHandlers", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	RegDeleteKeyW(hKey, L"NLCommonBrowserEnforcer");
	RegCloseKey(hKey);

	lRet = RegCreateKeyExW(HKEY_CLASSES_ROOT, L"Folder\\shellex\\ContextMenuHandlers", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL);

	if (ERROR_SUCCESS != lRet)
	{
		return FALSE;
	}

	RegDeleteKeyW(hKey, L"NLCommonBrowserEnforcer");
	RegCloseKey(hKey);

	return TRUE;
}

//Add register value to protect IEpep from disabled
BOOL AddIEpepRegisterValue ( )
{
	HKEY hKey = NULL;

	LONG lRet = RegCreateKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ext", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	DWORD dwValue = 1;

	lRet = RegSetValueExW ( hKey, L"ListBox_Support_CLSID", 0, REG_DWORD, (BYTE*)&dwValue, sizeof ( dwValue ) );

	RegCloseKey ( hKey );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	lRet = RegCreateKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ext\\CLSID", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	WCHAR wcValue[] = L"1";

	lRet = RegSetValueExW ( hKey, IEpepCLSID, 0, REG_SZ, (BYTE*)wcValue, sizeof ( wcValue ) );

	RegCloseKey ( hKey );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	return TRUE;
}


//Delete register value
BOOL DeleteIEpepRegisterValue ( )
{
	HKEY hKey = NULL;

	LONG lRet = RegCreateKeyExW ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ext\\CLSID", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	lRet = RegDeleteValueW ( hKey, IEpepCLSID );

	RegCloseKey ( hKey );

	if ( ERROR_SUCCESS != lRet )
	{
		return FALSE;
	}

	return TRUE;
}

//Add Adobe Reader protected mode whitelist register value
BOOL AddWhiteListRegisterValue ( )
{
	HKEY hKey = NULL;
    LSTATUS lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Adobe\\Acrobat Reader\\11.0\\FeatureLockDown", 0, KEY_SET_VALUE, &hKey);
    if (lRet != ERROR_SUCCESS || hKey == NULL)
    {
        lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Adobe\\Acrobat Reader\\10.0\\FeatureLockDown", 0, KEY_SET_VALUE, &hKey);
    }
    if (lRet == ERROR_SUCCESS && hKey != NULL)
	{
		DWORD dwValue = 1;
		lRet = RegSetValueExW ( hKey, L"bUseWhitelistConfigFile", 0, REG_DWORD, (BYTE*)&dwValue, sizeof ( dwValue ) );
		RegCloseKey ( hKey );

		if ( ERROR_SUCCESS == lRet )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//Add Adobe Reader 2017 protected mode whitelist register value
BOOL AddWhiteListRegisterValueFor2017 ( )
{
	HKEY hKey = NULL;
	LSTATUS lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Adobe\\Acrobat Reader\\2017\\FeatureLockDown", 0, KEY_SET_VALUE, &hKey);
	if (lRet == ERROR_SUCCESS && hKey != NULL)
	{
		DWORD dwValue = 1;
		lRet = RegSetValueExW ( hKey, L"bUseWhitelistConfigFile", 0, REG_DWORD, (BYTE*)&dwValue, sizeof ( dwValue ) );
		RegCloseKey ( hKey );

		if ( ERROR_SUCCESS == lRet )
		{
			return TRUE;
		}
	}
	return FALSE;
}

//Add Adobe Reader protected mode whitelist file
BOOL AddWhiteListFile ( )
{
	BOOL bIs64bit = TRUE;

	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	//Get Adobe Reader X install path
	HKEY hKey = 0;
	LONG lRet = ERROR_SUCCESS;

	if ( bIs64bit )
	{
		lRet = RegOpenKeyExA ( HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader\\11.0\\InstallPath", 0, KEY_QUERY_VALUE, &hKey );
        if (lRet != ERROR_SUCCESS || hKey == NULL)  lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader\\10.0\\InstallPath", 0, KEY_QUERY_VALUE, &hKey);
	}
	else
	{
		lRet = RegOpenKeyExA ( HKEY_LOCAL_MACHINE, "SOFTWARE\\Adobe\\Acrobat Reader\\11.0\\InstallPath", 0, KEY_QUERY_VALUE, &hKey );
        if (lRet != ERROR_SUCCESS || hKey == NULL)   lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Adobe\\Acrobat Reader\\10.0\\InstallPath", 0, KEY_QUERY_VALUE, &hKey);
	}

	if ( ERROR_SUCCESS == lRet )
	{
		char AdobeReaderXInstallPath[MAX_PATH] = { 0 };
		DWORD BufferSize = sizeof AdobeReaderXInstallPath;

		if ( ERROR_SUCCESS != RegQueryValueExA ( hKey, NULL, NULL, NULL, (LPBYTE)AdobeReaderXInstallPath, &BufferSize ) )
		{
			RegCloseKey ( hKey );
			return FALSE;
		}
		RegCloseKey ( hKey );

		strcat_s ( AdobeReaderXInstallPath, MAX_PATH, "\\ProtectedModeWhitelistConfig.txt" );

		FILE* pFile = NULL;

		if ( 0 == fopen_s ( &pFile, AdobeReaderXInstallPath, "a" ) && NULL != pFile )
		{
			fputs ( "\nFILES_ALLOW_ANY=*\n", pFile );
			fputs ( "FILES_ALLOW_DIR_ANY=*", pFile );
			fclose ( pFile );

			return TRUE;
		}
	}

	return FALSE;
}

//Add Adobe Reader 2017 protected mode whitelist file
BOOL AddWhiteListFileFor2017 ( )
{
	BOOL bIs64bit = TRUE;

	if ( !GetOSbitInfo ( bIs64bit ) )
	{
		return FALSE;
	}

	//Get Adobe Reader 2017 install path
	HKEY hKey = 0;
	LONG lRet = ERROR_SUCCESS;

	if ( bIs64bit )
	{
		lRet = RegOpenKeyExA ( HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader\\2017\\InstallPath", 0, KEY_QUERY_VALUE, &hKey );
	}
	else
	{
		lRet = RegOpenKeyExA ( HKEY_LOCAL_MACHINE, "SOFTWARE\\Adobe\\Acrobat Reader\\2017\\InstallPath", 0, KEY_QUERY_VALUE, &hKey );
	}

	if ( ERROR_SUCCESS == lRet )
	{
		char AdobeReader2017InstallPath[MAX_PATH] = { 0 };
		DWORD BufferSize = sizeof AdobeReader2017InstallPath;

		if ( ERROR_SUCCESS != RegQueryValueExA ( hKey, NULL, NULL, NULL, (LPBYTE)AdobeReader2017InstallPath, &BufferSize ) )
		{
			RegCloseKey ( hKey );
			return FALSE;
		}
		RegCloseKey ( hKey );

		strcat_s ( AdobeReader2017InstallPath, MAX_PATH, "\\ProtectedModeWhitelistConfig.txt" );

		FILE* pFile = NULL;

		if ( 0 == fopen_s ( &pFile, AdobeReader2017InstallPath, "a" ) && NULL != pFile )
		{
			fputs ( "\nFILES_ALLOW_ANY=*\n", pFile );
			fputs ( "FILES_ALLOW_DIR_ANY=*", pFile );
			fclose ( pFile );

			return TRUE;
		}
	}

	return FALSE;
}

static BOOL IsWindows7()
{
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (GetVersionEx(&osver))
		return (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1)? TRUE: FALSE;
	else
		return FALSE;
}
// disable preview panel on win7 if install WDE
BOOL SetPreviewPaneKey(bool bEnable)
{
	if(!IsWindows7())	return TRUE;
	const wchar_t* szSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
	HKEY hKey = NULL;

	LONG lRet = RegCreateKeyExW ( HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );

	wchar_t szLog[1025]={0};
	StringCchPrintf(szLog,1024,L"------------ Create key of [Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer] , return value is %d.\n",lRet);
	OutputDebugStringW(szLog);
	if ( ERROR_SUCCESS == lRet )
	{
		DWORD dwValue = 1;
		if(bEnable) dwValue = 0;
		lRet = RegSetValueExW ( hKey, L"NoReadingPane", 0, REG_DWORD, (BYTE*)&dwValue, sizeof ( dwValue ) );
		StringCchPrintf(szLog,1024,L"------------ set value of [NoReadingPane] to [%d] , return value is %d.\n", dwValue,lRet);
		OutputDebugStringW(szLog);
		RegCloseKey ( hKey );
		if ( ERROR_SUCCESS == lRet )	return TRUE;
	}
	return FALSE;
}

BOOL DisableNewMenuKey(bool bDisable)
{
	const wchar_t* szSubKey = L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer";
	HKEY hKey = NULL;

	LONG lRet = RegCreateKeyExW ( HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL );

	if ( ERROR_SUCCESS == lRet )
	{
		DWORD dwValue = 1;
		if(bDisable) dwValue = 0;
		lRet = RegSetValueExW ( hKey, L"NewFileAllowed", 0, REG_DWORD, (BYTE*)&dwValue, sizeof ( dwValue ) );
		RegCloseKey ( hKey );
		if ( ERROR_SUCCESS == lRet )	return TRUE;
	}
	return FALSE;
}

void AddEnvironmentVariable()
{
	WCHAR* buffer = new WCHAR[32767]();
	DWORD dwRet = GetEnvironmentVariableW(L"Path", buffer, 32767);
	if (dwRet == 0)
	{
		delete[] buffer;
		return;
	}
	
	std::wstring installDir;
	if (GetCommonInstallDir(installDir))
	{
#ifdef _M_X64
		installDir = L";" + installDir + L"bin32;" + installDir + L"bin64";
#else
		installDir = L";" + installDir + L"bin32";
#endif

		wcscat_s(buffer, 32767, installDir.c_str());

		SetEnvironmentVariableW(L"Path", buffer);
	}

	delete[] buffer;
}
