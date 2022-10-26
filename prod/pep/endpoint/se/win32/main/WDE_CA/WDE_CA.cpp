// WDE_CA.cpp : Defines the initialization routines for the DLL.
//
/*
      NOTIFYICONDATA ndi = { sizeof(ndi), this->m_hWnd, SYSTRAY_ID };
      Shell_NotifyIcon(NIM_DELETE,&ndi);
*/
#include "stdafx.h"
#include "WDE_CA.h"
#include <Winsvc.h>
#include <Msiquery.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable: 4100)
//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CWDE_CAApp

BEGIN_MESSAGE_MAP(CWDE_CAApp, CWinApp)
END_MESSAGE_MAP()


// CWDE_CAApp construction

CWDE_CAApp::CWDE_CAApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWDE_CAApp object

CWDE_CAApp theApp;


// CWDE_CAApp initialization

BOOL CWDE_CAApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: void fnWriteToInstallrLogFile(MSIHANDLE hInstall, TCHAR * tchMessage)
//	Purpose: Writes to the installer log file
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
void fnWriteToInstallerLogFile(MSIHANDLE hInstall, TCHAR * tchMessage)
{
	PMSIHANDLE hRec;
	TCHAR tchBuffer[MAX_PATH];
	DWORD dwLength;
	TCHAR tchFullErrorMessage[MAX_PATH];

	lstrcpy(tchFullErrorMessage,TEXT("Custom Action Logging "));
#pragma warning(push)
#pragma warning(disable: 6204)
	lstrcat(tchFullErrorMessage,tchMessage);
#pragma warning(pop)
	dwLength = lstrlen(tchFullErrorMessage);
	if ( dwLength <= 0 )
		return;

	hRec = MsiCreateRecord(1);                  
	
	if (!hRec)
		return;

	MsiRecordSetString(hRec, 1, tchFullErrorMessage);        
	MsiRecordSetString(hRec, 0, TEXT("[1]"));       
	MsiFormatRecord(hInstall, hRec, tchBuffer, &dwLength); 
	MsiProcessMessage(hInstall,INSTALLMESSAGE_INFO, hRec);                 

}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: fnSayHello(MSIHANDLE hInstall)
//	Purpose: Test function
//	Author: Michael Byrns
//  History: Created 11 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnSayHello(MSIHANDLE hInstall)
{
	MessageBox(NULL,TEXT("WDE_CA Hello!!!"),TEXT("Installer Debug"),MB_OK);
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnShutdown20ComplianceAgentService(MSIHANDLE hInstall)
//  Purpose: Attempts shut down the legacy ComplianceAgentService
//	Author: Michael Byrns 
//  History: Created 11 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnShutdown20ComplianceAgentService(MSIHANDLE hInstall)
{
	UINT uResult;
	DWORD dwServiceStatus;
	MsiSetProperty(hInstall,TEXT("BADPASSWORD"),TEXT("1"));
	int iTimeout = 0;


	fnQueryServiceStatus(hInstall, TEXT("ComplianceAgentService"),&dwServiceStatus);
	if ( dwServiceStatus == SERVICE_STOPPED )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnShutdown20ComplianceAgentService ComplianceAgentService was stopped"));
		MsiSetProperty(hInstall,TEXT("BADPASSWORD"),TEXT("0"));
		MsiSetProperty(hInstall,TEXT("CanGoToNextDialog"),TEXT("1"));
		return ERROR_SUCCESS;
	}

	if ( dwServiceStatus != SERVICE_STOPPED )
		fnWriteToInstallerLogFile(hInstall, TEXT("fnShutdown20ComplianceAgentService ComplianceAgentService is not stopped"));


	uResult =  MsiDoAction(hInstall,TEXT("Stop20AgentService"));
	if (uResult != ERROR_SUCCESS )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnShutdown20ComplianceAgentService MsiDoAction failed for StopAgentService"));
		return ERROR_SUCCESS;
	}

	Sleep(1000);

	fnQueryServiceStatus(hInstall, TEXT("ComplianceAgentService"),&dwServiceStatus);

	while ( dwServiceStatus != SERVICE_STOPPED )
	{
		fnQueryServiceStatus(hInstall, TEXT("ComplianceAgentService"),&dwServiceStatus);
		if ( dwServiceStatus == SERVICE_STOPPED )
			break;
		Sleep(500);
		iTimeout++;
		if ( iTimeout > 200 )
			break;
	}

	if ( dwServiceStatus != SERVICE_STOPPED )  
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnShutdown20ComplianceAgentService ComplianceAgentService did not stop"));
		MsiSetProperty(hInstall,TEXT("BADPASSWORD"),TEXT("1"));
	}

	if ( dwServiceStatus == SERVICE_STOPPED )  
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnShutdown20ComplianceAgentService ComplianceAgentService stopped"));
		MsiSetProperty(hInstall,TEXT("BADPASSWORD"),TEXT("0"));
		MsiSetProperty(hInstall,TEXT("CanGoToNextDialog"),TEXT("1"));
	}
	
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnQueryServiceStatus(MSIHANDLE hInstall,TCHAR * tchServiceName, DWORD * dwServiceStatus)
//  Purpose: Queries the named service
//	Author: Michael Byrns 
//  History: Created 11 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnQueryServiceStatus(MSIHANDLE hInstall, TCHAR * tchServiceName,DWORD * dwServiceStatus)
{
    SERVICE_STATUS ssStatus; 
	SC_HANDLE schService;
	SC_HANDLE schSCManager;

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnQueryServiceStatus schSCManager = NULL"));
		return ERROR_INSTALL_FAILURE;
	}

	// this could be a potential problem in the future.  need to see the total plan for this service in the future.
    schService = OpenService(schSCManager,tchServiceName,SERVICE_QUERY_STATUS);     
	if (schService == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnQueryServiceStatus schService = NULL"));
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;;
	}

	if ( !QueryServiceStatus(schService,&ssStatus) )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnQueryServiceStatus QueryServiceStatus = FALSE"));
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;
	}

	*dwServiceStatus = ssStatus.dwCurrentState;
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return ERROR_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnControlService(MSIHANDLE hInstall,TCHAR * tchServiceName, DWORD dwControl)
//  Purpose: Controls the named service
//	Author: Michael Byrns 
//  History: Created 11 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnControlService(MSIHANDLE hInstall,TCHAR * tchServiceName, DWORD dwControl)
{

    SERVICE_STATUS ssStatus; 
	SC_HANDLE schService;
	SC_HANDLE schSCManager;

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnControlService schSCManager = NULL"));
		return ERROR_INSTALL_FAILURE;
	}

	// this could be a potential problem in the future.  need to see the total plan for this service in the future.
    schService = OpenService(schSCManager,tchServiceName,SERVICE_QUERY_STATUS);     
	if (schService == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnControlService schService = NULL"));
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;
	}

     if (ControlService(schService, dwControl, &ssStatus) == 0 )
    {
		fnWriteToInstallerLogFile(hInstall, TEXT("fnControlService schService = 0"));
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;
    }

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnStream20AgentControllerToTemp(MSIHANDLE hInstall)
//	Purpose: Extract the AgentController.dll to TEMP for later possible use
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnStream20AgentControllerToTemp( MSIHANDLE hInstall)
{
	TCHAR * tchTempFolder;
	TCHAR tchExtractedFileName[MAX_PATH];
//	DWORD dwBufferSize = MAX_PATH;
	UINT uResult;
	MSIHANDLE hView;
	MSIHANDLE hDatabase;
	PMSIHANDLE hRecord;
	TCHAR tchSQLQuery[MAX_PATH] = TEXT("SELECT * FROM Binary WHERE Name='AgentController.dll'");
	HANDLE hFile;
	TCHAR tchBuffer[1023];
	DWORD dwBytesRead = 1024;
	DWORD dwBytesWritten;

#pragma warning( push )
#pragma warning( disable : 4996 )
	tchTempFolder = getenv("TEMP");
#pragma warning( pop )
	if ( tchTempFolder == NULL )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp getenv failed"));
		return ERROR_INSTALL_FAILURE;
	}
	lstrcpy(tchExtractedFileName,tchTempFolder);
	lstrcat(tchExtractedFileName,TEXT("\\AgentController.dll"));


	hDatabase = MsiGetActiveDatabase(hInstall);
	if (!hDatabase)
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp MsiGetActiveDatabase failed"));
		return ERROR_INSTALL_FAILURE;
	}

	uResult = MsiDatabaseOpenView(hDatabase, tchSQLQuery, &hView);
	if (uResult != ERROR_SUCCESS )
	{
		MsiCloseHandle(hDatabase);
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp MsiDatabaseOpenView failed"));
		return ERROR_INSTALL_FAILURE;
	}


    // Execute the view before getting the record
    uResult = MsiViewExecute(hView, NULL);  
    if(uResult != ERROR_SUCCESS)
	{
        MsiCloseHandle(hView);
        MsiCloseHandle(hDatabase);
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp MsiViewExecute failed"));
        return ERROR_INSTALL_FAILURE;
	}

	uResult = MsiViewFetch(hView, &hRecord);
	if ( uResult != ERROR_SUCCESS )
	{
		MsiCloseHandle(hView);
		MsiCloseHandle(hDatabase);
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp MsiViewFetch failed"));
		return ERROR_INSTALL_FAILURE;
	}

	hFile = CreateFile(tchExtractedFileName, GENERIC_WRITE, 0, 0,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp CreateFile failed"));
		MsiCloseHandle(hView);
		MsiCloseHandle(hDatabase);
		return ERROR_INSTALL_FAILURE;
	}


    while (dwBytesRead > 0)
	{
#pragma warning( push )
#pragma warning( disable : 6386 )
		uResult = MsiRecordReadStream(hRecord, 2, tchBuffer, &dwBytesRead);
#pragma warning( pop )
		if ( uResult == ERROR_SUCCESS )
		{
			WriteFile(hFile, tchBuffer, dwBytesRead, &dwBytesWritten, NULL);
		}
		else
		{
			fnWriteToInstallerLogFile(hInstall, TEXT("fnStreamFileToTemp MsiRecordReadStream failed"));
			CloseHandle(hFile);
			MsiCloseHandle(hView);
			MsiCloseHandle(hDatabase);
			return ERROR_INSTALL_FAILURE;
		}
	}
	CloseHandle(hFile);
	MsiCloseHandle(hView);
	MsiCloseHandle(hDatabase);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnClose20ComplianceAgentNotify(MSIHANDLE hInstall)
//  Purpose: Posts a message to Control Agent Notify to shut down
//	Author: Michael Byrns 
//  History: Created 21 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnClose20ComplianceAgentNotify(MSIHANDLE hInstall)
{
	HWND hWnd;
	hWnd = FindWindow(TEXT("DESTINYNOTIFY"),NULL);
	if ( hWnd )
		PostMessage(hWnd,WM_QUIT,0,0);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnFind20ComplianceAgentNotify(MSIHANDLE hInstall)
//	Purpose: Find the handle to the legacy CAN for 2.0 major upgrade
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnFind20ComplianceAgentNotify(MSIHANDLE hInstall)
{
	HWND hWnd;
	hWnd = FindWindow(TEXT("DESTINYNOTIFY"),NULL);
	if ( hWnd )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnFind20ComplianceAgentNotify compliance agent running"));
		MsiSetProperty(hInstall,TEXT("DESTINYNOTIFYRUNNING"),TEXT("1"));
	}
	
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnFindDestiny20(MSIHANDLE hInstall)
//	Purpose: See if this is a Destiny 20 release
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnFindDestiny20(MSIHANDLE hInstall)
{
	TCHAR tchKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B48A80E2-DDC4-420B-A136-A827B878C65B}");
	HKEY hKey;

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnFindDestiny20 found version 20"));
		MsiSetProperty(hInstall,TEXT("DESTINY20"),TEXT("1"));
		RegCloseKey(hKey);
	}
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnCleanup20TempFiles(MSIHANDLE hInstall)
//	Purpose: Deletes any files extracted to the temp folder from the binary table
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnCleanup20TempFiles(MSIHANDLE hInstall)
{
	TCHAR * tchTempFolder;
	TCHAR tchExtractedFileName[MAX_PATH];
#pragma warning( push )
#pragma warning( disable : 4996 )
	tchTempFolder = getenv("TEMP");
#pragma warning( pop )
	if ( tchTempFolder == NULL )
		return ERROR_SUCCESS;

	lstrcpy(tchExtractedFileName,tchTempFolder);
	lstrcat(tchExtractedFileName,TEXT("\\AgentController.dll"));
	DeleteFile(tchExtractedFileName);

	return ERROR_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnDelete20EnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Deletes the 20 Enforcer Driver Service
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnDelete20EnforcerDriver(MSIHANDLE hInstall)
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	DWORD dwServiceStatus;

	fnQueryServiceStatus(hInstall, TEXT("ProcDetect"),&dwServiceStatus);

	if ( dwServiceStatus != SERVICE_STOPPED )
		fnControlService(hInstall,TEXT("ProcDetect"), SERVICE_CONTROL_STOP);

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDelete20EnforcerDriver schSCManager = NULL"));
		return ERROR_INSTALL_FAILURE;
	}

	schService = OpenService(schSCManager,TEXT("ProcDetect"),SERVICE_ALL_ACCESS);     
	if (schService == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDelete20EnforcerDriver schService = NULL"));
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;
	}

	 if (!DeleteService(schService))
	 {
		 fnWriteToInstallerLogFile(hInstall, TEXT("fnDelete20EnforcerDriver DeleteService failed"));
	 }


	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnInstallEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Installs the new driver
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnInstallEnforcerDriver(MSIHANDLE hInstall)
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	DWORD dwBufferSize = MAX_PATH;
	UINT uResult;
	TCHAR tchDriverPath[MAX_PATH];
	DWORD dwResult;

	
	uResult = MsiGetProperty(hInstall,TEXT("CustomActionData"),tchDriverPath,&dwBufferSize);
	if (uResult != ERROR_SUCCESS )
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnInstallEnforcerDriver CustomActionData failed"));
		return ERROR_SUCCESS;
	}
	lstrcat(tchDriverPath,TEXT("driver\\PROCDETECT.SYS"));


	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		int dwResult_l = GetLastError();
		TCHAR tchError[MAX_PATH];
		TCHAR tchBuffer[MAX_PATH] = TEXT("fnInstallEnforcerDriver schService == NULL Error code = ");
		_itoa_s(dwResult_l,tchError,10);
		lstrcat(tchBuffer,tchError);
		fnWriteToInstallerLogFile(hInstall, tchBuffer);

		return ERROR_INSTALL_FAILURE;
	}

	// In the 3.5 WDE, we failed to delete a service.  So, now we try to open it to see if it exists before we try to install
	schService = OpenService(schSCManager,TEXT("ProcDetect"),SERVICE_CHANGE_CONFIG);     
	if (schService == NULL) 
	{
		// If it doesn't exist, we install it.  Log the error code before we continue, though, in case it's another issue
		int dwResult_l = GetLastError();
		TCHAR tchError[MAX_PATH];
		TCHAR tchBuffer[MAX_PATH] = TEXT("Could not find existing service.  Will try to install.  Error code = ");
		_itoa_s(dwResult_l,tchError,10);
		lstrcat(tchBuffer,tchError);
		fnWriteToInstallerLogFile(hInstall, tchBuffer);

		schService = CreateService(schSCManager,
								   TEXT("ProcDetect"),
							       TEXT("ProcDetect"),
							       SC_MANAGER_ALL_ACCESS,
							       SERVICE_KERNEL_DRIVER,
							       SERVICE_SYSTEM_START,
							       SERVICE_ERROR_NORMAL,
							       tchDriverPath,
							       TEXT("File System"),
							       NULL,
							       NULL,
							       NULL,
							       NULL);

		if ( schService == NULL )
		{
			dwResult_l = GetLastError();
			TCHAR tchError_l[MAX_PATH];
			TCHAR tchBuffer_l[MAX_PATH] = TEXT("fnInstallEnforcerDriver schService == NULL Error code = ");
			_itoa_s(dwResult_l,tchError_l,10);
			lstrcat(tchBuffer_l,tchError_l);
			fnWriteToInstallerLogFile(hInstall, tchBuffer_l);
#pragma warning(push)
#pragma warning(disable: 6387)
			CloseServiceHandle(schService);
#pragma warning(pop)
			CloseServiceHandle(schSCManager);
			return ERROR_INSTALL_FAILURE;
		}
	} else {
		// We found an existing service.  Let's just try to change config
		dwResult = ChangeServiceConfig(schService,
										SERVICE_KERNEL_DRIVER,
										SERVICE_SYSTEM_START,
										SERVICE_ERROR_NORMAL,
										tchDriverPath,
										TEXT("File System"),
										NULL,
										NULL,
										NULL,
										NULL,
										TEXT("ProcDetect"));

		if ( dwResult == 0 )
		{
			dwResult = GetLastError();
			TCHAR tchError[MAX_PATH];
			TCHAR tchBuffer[MAX_PATH] = TEXT("fnInstallEnforcerDriver - failed to change service configuration. Error code = ");
			_itoa_s(dwResult,tchError,10);
			lstrcat(tchBuffer,tchError);
			fnWriteToInstallerLogFile(hInstall, tchBuffer);
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return ERROR_INSTALL_FAILURE;
		}
	}




	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnBackupEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Backs up the enforcer driver settings to get around a design flaw
//  with Compliance Agent and upgrades.  I only care about the image path.  The rest
//  of the settings are not location specific.
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnBackupEnforcerDriver(MSIHANDLE hInstall)
{
	HKEY hInKey,hOutKey;
	DWORD dwBufferSize,dwType;
	DWORD dwBuffer;
	DWORD dwDisposition;
	TCHAR tchBuffer[MAX_PATH];
	TCHAR tchInKey [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect");
	TCHAR tchOutKey [] = TEXT("Software\\Classes\\Installer\\Properties\\{203ED06D-B2BC-4A6B-AE3B-934367CC5EE5}");
	TCHAR tchInKey1 [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect\\Security");
	TCHAR tchInKey2 [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect\\Enum");


	RegCreateKeyEx( HKEY_LOCAL_MACHINE,tchOutKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hOutKey,&dwDisposition);
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("ImagePath"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("ImagePath"), 0, dwType, (LPBYTE)tchBuffer, dwBufferSize);
		RegCloseKey(hInKey);
	}

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey1, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("Security"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("Security"), 0, dwType, (LPBYTE)tchBuffer, dwBufferSize);
		RegCloseKey(hInKey);
	}

	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey2, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("0"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("0"), 0, dwType, (LPBYTE)tchBuffer, dwBufferSize);
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("Count"), 0, &dwType, (LPBYTE)&dwBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("Count"), 0, dwType, (LPBYTE)&dwBuffer, dwBufferSize);
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("NextInstance"), 0, &dwType, (LPBYTE)&dwBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("NextInstance"), 0, dwType, (LPBYTE)&dwBuffer, dwBufferSize);

		RegCloseKey(hInKey);
	}
	RegCloseKey(hOutKey);



	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnRestoreEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Restores the enforcer driver settings to get around a design flaw
//  with Compliance Agent and upgrades.  I only care about the image path.  The rest
//  of the settings are not location specific.
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnRestoreEnforcerDriver(MSIHANDLE hInstall)
{
	HKEY hInKey,hOutKey;
	DWORD dwBufferSize,dwType;
	DWORD dwBuffer;
	DWORD dwDisposition;
	TCHAR tchBuffer[MAX_PATH];
	TCHAR tchOutKey [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect");
	TCHAR tchOutKey1 [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect\\Security");
	TCHAR tchOutKey2 [] = TEXT("System\\CurrentControlSet\\Services\\ProcDetect\\Enum");
	TCHAR tchInKey [] = TEXT("Software\\Classes\\Installer\\Properties\\{203ED06D-B2BC-4A6B-AE3B-934367CC5EE5}");
	TCHAR tchDisplayName[] = TEXT("ProcDetect");
	TCHAR tchGroup[] = TEXT("File System");


	RegCreateKeyEx( HKEY_LOCAL_MACHINE,tchOutKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hOutKey,&dwDisposition);
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("ImagePath"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("ImagePath"), 0, dwType, (LPBYTE)tchBuffer, lstrlen(tchBuffer));
		RegSetValueEx( hOutKey, TEXT("DisplayName"), 0, REG_SZ, (LPBYTE)tchDisplayName, lstrlen(tchDisplayName));
		RegSetValueEx( hOutKey, TEXT("Group"), 0, REG_SZ, (LPBYTE)tchGroup, lstrlen(tchGroup));
		dwBuffer = 1;
		RegSetValueEx( hOutKey, TEXT("Type"), 0, REG_DWORD, (LPBYTE)&dwBuffer, sizeof(DWORD));
		RegSetValueEx( hOutKey, TEXT("Start"), 0, REG_DWORD, (LPBYTE)&dwBuffer, sizeof(DWORD));
		RegSetValueEx( hOutKey, TEXT("ErrorControl"), 0, REG_DWORD, (LPBYTE)&dwBuffer, sizeof(DWORD));
		RegCloseKey(hInKey);
	}
	RegCloseKey(hOutKey);

	RegCreateKeyEx( HKEY_LOCAL_MACHINE,tchOutKey1,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hOutKey,&dwDisposition);
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("Security"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("Security"), 0, dwType, (LPBYTE)tchBuffer, dwBufferSize);
	}
	RegCloseKey(hOutKey);


	RegCreateKeyEx( HKEY_LOCAL_MACHINE,tchOutKey2,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hOutKey,&dwDisposition);
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, tchInKey, 0, KEY_QUERY_VALUE, &hInKey) == ERROR_SUCCESS)
	{
		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("0"), 0, &dwType, (LPBYTE)tchBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("0"), 0, dwType, (LPBYTE)tchBuffer, dwBufferSize);

		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("Count"), 0, &dwType, (LPBYTE)&dwBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("Count"), 0, dwType, (LPBYTE)&dwBuffer, dwBufferSize);

		dwBufferSize = MAX_PATH;
		RegQueryValueEx( hInKey, TEXT("NextInstance"), 0, &dwType, (LPBYTE)&dwBuffer, &dwBufferSize);
		RegSetValueEx( hOutKey, TEXT("NextInstance"), 0, dwType, (LPBYTE)&dwBuffer, dwBufferSize);

		RegCloseKey(hInKey);
	}
	RegCloseKey(hOutKey);

	return ERROR_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnStartEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Starts the Enforcer driver service
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnStartEnforcerDriver(MSIHANDLE hInstall)
{

//    SERVICE_STATUS ssStatus; 
	SC_HANDLE schService;
	SC_HANDLE schSCManager;

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnQueryServiceStatus schSCManager = NULL"));
		return ERROR_INSTALL_FAILURE;
	}

    schService = OpenService(schSCManager,TEXT("ProcDetect"),SERVICE_QUERY_STATUS);     
	if (schService == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnQueryServiceStatus schService = NULL"));
		CloseServiceHandle(schSCManager);
		return ERROR_INSTALL_FAILURE;;
	}

	StartService(schService,NULL,NULL);

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return ERROR_SUCCESS;

}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnStopEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Stops the Enforcer driver service
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnStopEnforcerDriver(MSIHANDLE hInstall)
{
	fnControlService(hInstall,TEXT("ProcDetect"),SERVICE_CONTROL_STOP);
	return ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//	Function Name: UINT fnDeleteEnforcerDriver(MSIHANDLE hInstall)
//	Purpose: Deletes the Enforcer driver service
//	Author: Michael Byrns
//  History: Created 12 October 2007
//////////////////////////////////////////////////////////////////////////////////
UINT fnDeleteEnforcerDriver(MSIHANDLE hInstall)
{
	SC_HANDLE schService;
	SC_HANDLE schSCManager;
	DWORD dwServiceStatus;

	fnQueryServiceStatus(hInstall, TEXT("ProcDetect"),&dwServiceStatus);

	if ( dwServiceStatus != SERVICE_STOPPED )
		fnControlService(hInstall,TEXT("ProcDetect"), SERVICE_CONTROL_STOP);

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);  
 	if (schSCManager == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDeleteEnforcerDriver schSCManager = NULL"));
		return ERROR_SUCCESS;
	}

	schService = OpenService(schSCManager,TEXT("ProcDetect"),SERVICE_ALL_ACCESS);     
	if (schService == NULL) 
	{
		fnWriteToInstallerLogFile(hInstall, TEXT("fnDeleteEnforcerDriver schService = NULL"));
		CloseServiceHandle(schSCManager);
		return ERROR_SUCCESS;
	}

	 if (!DeleteService(schService))
	 {
		 fnWriteToInstallerLogFile(hInstall, TEXT("fnDeleteEnforcerDriver DeleteService failed"));
	 }


	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return ERROR_SUCCESS;
}

