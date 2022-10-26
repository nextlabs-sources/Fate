// oeinstca.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <msi.h>
#include <msiquery.h>
#include <stdio.h>
#include <Winreg.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <string>
#include <fstream>
#include <iostream>
#include <tlhelp32.h>
#include "windows.h"
#include "tchar.h"
#include "conio.h"
#include "stdio.h"


using namespace std;

#define MAX_BUFFER 1024
#define FILENAME_COMM L"commprofile.xml"
#define FILENAME_INJECTION L"injection.ini"
#define FILENAME_OEINI L"OutlookEnforcer.ini"
#define PRODUCT_NAME L"NextLabs Endpoint Enforcers"
#define DELAYDELETE_DIR _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager") // Registry key
#define DELAYDELETE_KEY _T("PendingFileRenameOperations") // Registry key

UINT __stdcall ResetDelayDelete(MSIHANDLE hInstall);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//Note:  Messagebox can not use in defered execution since not be able to get UILevel property
UINT _stdcall MessageAndLogging(MSIHANDLE hInstall, BOOL bLogOnly, const WCHAR* wstrMsg )
{
	if(bLogOnly == FALSE && hInstall!= NULL)
	{
		INT nUILevel =0;
		WCHAR wstrTemp[2] = {0};
		DWORD dwBufsize = 0;
		
		dwBufsize = sizeof(wstrTemp)/sizeof(WCHAR);	
		if(ERROR_SUCCESS == MsiGetProperty(hInstall, TEXT("UILevel"), wstrTemp, &dwBufsize))
		{
			nUILevel = _wtoi(wstrTemp);
		}

		if(nUILevel > 2)
		{
			MessageBox(GetForegroundWindow(),(LPCWSTR) wstrMsg, (LPCWSTR)PRODUCT_NAME, MB_OK|MB_ICONWARNING);	
		}
	}

	//add log here
	PMSIHANDLE hRecord = MsiCreateRecord(1);
	if(hRecord !=NULL)
	{
		MsiRecordSetString(hRecord, 0, wstrMsg);
		// send message to running installer
		MsiProcessMessage(hInstall, INSTALLMESSAGE_INFO, hRecord);
		MsiCloseHandle(hRecord);
	}

	
	return ERROR_SUCCESS;
}//return service current status, or return 0 for service not existed


BOOL SHCopy(LPCWSTR from, LPCWSTR to, BOOL bDeleteFrom)
{
	SHFILEOPSTRUCT fileOp = {0};
	WCHAR newFrom[MAX_PATH];
	WCHAR newTo[MAX_PATH];

	if(bDeleteFrom)
		fileOp.wFunc = FO_MOVE;
	else
		fileOp.wFunc = FO_COPY;

	fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

	wcscpy_s(newFrom, from);
	newFrom[wcslen(from) + 1] = NULL;
	fileOp.pFrom = newFrom;

	wcscpy_s(newTo, to);
	newTo[wcslen(to) + 1] = NULL;
	fileOp.pTo = newTo;

	int result = SHFileOperation(&fileOp);

	return result == 0;
}

BOOL SHDelete(LPCWSTR strFile)
{
	SHFILEOPSTRUCT fileOp = { 0 };
	WCHAR newFrom[MAX_PATH] = { 0 };

	fileOp.wFunc = FO_DELETE;
	fileOp.fFlags = FOF_MULTIDESTFILES | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;

	wcscpy_s(newFrom, strFile);
	newFrom[wcslen(strFile) + 1] = NULL;
	fileOp.pFrom = newFrom;
	fileOp.pTo = NULL;
	fileOp.fAnyOperationsAborted = true;

	int result = SHFileOperation(&fileOp);

	return result == 0;
}

void ReplaceString(std::wstring &s, const std::wstring &to_find, const std::wstring &replace_with)
{
	std::wstring result;
	std::wstring::size_type pos = 0;
	while(1)
	{
		std::wstring::size_type next = s.find(to_find, pos);
		result.append(s, pos, next-pos);
		if(next != std::wstring::npos)
		{
			result.append(replace_with);
			pos = next + to_find.size();
		}
		else
			break;
	}
	s.swap(result);
	return;	
}


int ReplaceStringInFile(wfstream &inFile, wfstream &outFile, const std::wstring &to_find, const std::wstring &repl_with)
{	
	wchar_t strReplace[MAX_BUFFER];	
	while(!inFile.eof())
	{
		inFile.getline(strReplace,MAX_BUFFER,'\n');
		wstring s;
		s = strReplace;
		if(!s.empty())
		{
			ReplaceString( s, to_find, repl_with) ;	
		}
		outFile <<s <<endl;
	}
	return 1;
}


//*****************************************************************************************************
//				START MSI CUSTOM ACTION FUNCTION HERE
//*****************************************************************************************************

//restore files in upgrade install
UINT __stdcall RestoreFiles(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start Restore log and conf files. "));

	WCHAR wstrInstallDir[MAX_PATH+1] = {0};	
	WCHAR wstrTempNXPCDir[MAX_PATH+1] = {0};
	WCHAR wstrTempConfDir[MAX_PATH+1] = {0};
	WCHAR wstrInstallPCDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrInstallConfDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrInstallJservice[MAX_PATH + 1] = { 0 };
	WCHAR wstrMsg[512] = { 0 };
	DWORD dwErrorCode = 0;
	
	
	//get current Installdir from MSI
	DWORD dwBufsize = sizeof(wstrInstallDir)/sizeof(WCHAR);
	UINT uiRetCode =  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwBufsize);
	if(ERROR_SUCCESS != uiRetCode)
	{	
		swprintf_s(wstrMsg, 512, L"NXPCLOG: Failed to get install directory from MSI. Error Code: %d", uiRetCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_SUCCESS;
	}
	
	if(wstrInstallDir[wcslen(wstrInstallDir)-1]!= L'\\')
	{
		wcscat_s(wstrInstallDir, _countof(wstrInstallDir),  L"\\");
	}

	//get file from temp
	WCHAR wstrTempDir[MAX_PATH+1] = {0};
	DWORD dwRetVal = GetTempPath(MAX_PATH+1,  wstrTempDir);                                 
    if ((dwRetVal > MAX_PATH+1) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("NXPCLOG: Failed to get temp path in this computer."));
		return ERROR_SUCCESS;
    }

	WCHAR wstrTempDirLong[MAX_PATH + 1] = { 0 };
	dwRetVal = GetLongPathName(wstrTempDir, (LPTSTR)&wstrTempDirLong, MAX_PATH + 1);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("NXPCLOG: Failed to get temp long path in this computer."));
		return ERROR_SUCCESS;
	}
	wcscpy_s(wstrTempDir, MAX_PATH + 1, wstrTempDirLong);

	if (wstrTempDir[wcslen(wstrTempDir) - 1] != L'\\')
	{
		wcscat_s(wstrTempDir, _countof(wstrTempDir),  L"\\");
	}

	wcscpy_s(wstrTempNXPCDir, MAX_PATH+1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir,L"NxPCFile\\"); 

	//Restore  logid.dat	
	wcscpy_s(wstrInstallPCDir, MAX_PATH+1, wstrInstallDir);	
	wcscat_s(wstrInstallPCDir, L"Policy Controller\\");

	WCHAR wstrFile[MAX_PATH + 1] = { 0 };
	WCHAR wstrDest[MAX_PATH + 1] = { 0 };
	wcscpy_s(wstrFile, MAX_PATH + 1, wstrTempNXPCDir);
	wcscat_s(wstrFile,  L"logs\\logid.dat");	
	wcscpy_s(wstrDest, MAX_PATH + 1, wstrInstallPCDir);
	wcscat_s(wstrDest, L"logs\\logid.dat");
	SHCopy(wstrFile, wstrDest, TRUE);

	//restore config.dat
	wcscpy_s(wstrInstallConfDir, MAX_PATH+1, wstrInstallDir);					
	wcscat_s(wstrInstallConfDir, L"Policy Controller\\config\\");
	
	wcscpy_s(wstrFile, MAX_PATH+1, wstrTempNXPCDir);	
	wcscat_s(wstrFile,  L"config\\config.dat");	
	wcscpy_s(wstrDest, MAX_PATH + 1, wstrInstallPCDir);
	wcscat_s(wstrDest, L"config\\config.dat");
	SHCopy(wstrFile, wstrDest, TRUE);

	//restore config2.dat
	wcscpy_s(wstrInstallConfDir, MAX_PATH+1, wstrInstallDir);					
	wcscat_s(wstrInstallConfDir, L"Policy Controller\\config\\");
	
	wcscpy_s(wstrFile, MAX_PATH+1, wstrTempNXPCDir);	
	wcscat_s(wstrFile,  L"config\\config2.dat");	
	wcscpy_s(wstrDest, MAX_PATH + 1, wstrInstallPCDir);
	wcscat_s(wstrDest, L"config\\config2.dat");
	SHCopy(wstrFile, wstrDest, TRUE);

	//restore logging.properties
	wcscpy_s(wstrFile, MAX_PATH + 1, wstrTempNXPCDir);
	wcscat_s(wstrFile, L"config\\logging.properties");
	wcscpy_s(wstrDest, MAX_PATH + 1, wstrInstallPCDir);
	wcscat_s(wstrDest, L"config\\logging.properties");
	SHCopy(wstrFile, wstrDest, TRUE);

	//Restore JService							
	WCHAR wstrTempJserviceDir[MAX_PATH + 1] = { 0 };
	wcscpy_s(wstrTempJserviceDir, MAX_PATH + 1, wstrTempNXPCDir);
	wcscat_s(wstrTempJserviceDir, L"jservice\\*");

	wcscpy_s(wstrInstallJservice, MAX_PATH + 1, wstrInstallDir);
	wcscat_s(wstrInstallJservice, L"Policy Controller\\jservice\\");

	SHCopy(wstrTempJserviceDir, wstrInstallJservice, TRUE);

	//Restore tamper_resistance							
	wcscat_s(wstrInstallConfDir, L"tamper_resistance\\");	
	wcscat_s(wstrTempNXPCDir,  L"tamper_resistance\\*"); 
	SHCopy(wstrTempNXPCDir, wstrInstallConfDir, TRUE);
	
	//clean up 	
	wcscpy_s(wstrTempNXPCDir, MAX_PATH+1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile"); 
	RemoveDirectory(wstrTempNXPCDir);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Restore files done."));

    return ERROR_SUCCESS;

}

//backup current files before upgrade install
UINT __stdcall BackupFiles(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start back up log files. "));

	HKEY hKey = NULL;
	WCHAR wstrInstPC[MAX_PATH+1] = {0};
	WCHAR wstrInstallLogDir[MAX_PATH+1] = {0};
	WCHAR wstrInstallConfDir[MAX_PATH+1] = {0};
	WCHAR wstrTempDir[MAX_PATH+1] = {0};
	WCHAR wstrTempNXPCDir[MAX_PATH+1] = {0};	
	WCHAR wstrMsg[512] = {0};
	DWORD dwErrorCode = 0;
	
	//get PC installed path
	BOOL bFoundInstDir =FALSE;
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\"),
									0, 
									KEY_READ, 
									&hKey))
	{		
		DWORD dwBufsize = sizeof(wstrInstPC)*sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("PolicyControllerDir"),
											NULL, 
											NULL, 
											(LPBYTE)wstrInstPC, 
											&dwBufsize))
		{			
				bFoundInstDir = TRUE;			
		}
		RegCloseKey(hKey);
	}
	
	if(!bFoundInstDir)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("The previous install path does not found. "));
		return ERROR_SUCCESS;
	}

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH+1,  wstrTempDir);                                 
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_SUCCESS;
    }

	WCHAR wstrTempDirLong[MAX_PATH + 1] = { 0 };
	dwRetVal = GetLongPathName(wstrTempDir, (LPTSTR)&wstrTempDirLong, MAX_PATH + 1);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("NXPCLOG: Failed to get temp long path in this computer."));
		return ERROR_SUCCESS;
	}
	wcscpy_s(wstrTempDir, MAX_PATH + 1, wstrTempDirLong);

	if(wstrTempDir[wcslen(wstrTempDir)-1]!= L'\\')
	{
		wcscat_s(wstrTempDir, L"\\");
	}

	//copy logid.dat
	wcscpy_s(wstrTempNXPCDir, MAX_PATH + 1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile\\");

	wcscpy_s(wstrInstallLogDir, MAX_PATH + 1, wstrInstPC);
	wcscat_s(wstrInstallLogDir,  L"logs\\logid.dat");
	wcscat_s(wstrTempNXPCDir, L"logs\\logid.dat");
	SHCopy(wstrInstallLogDir, wstrTempNXPCDir, FALSE);

	//copy config.dat
	wcscpy_s(wstrTempNXPCDir, MAX_PATH + 1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile\\");

	wcscpy_s(wstrInstallConfDir, MAX_PATH + 1, wstrInstPC);
	wcscat_s(wstrInstallConfDir, L"config\\config.dat");
	wcscat_s(wstrTempNXPCDir, L"config\\config.dat");
	SHCopy(wstrInstallConfDir, wstrTempNXPCDir, FALSE);

	//copy  config2.dat	
	wcscpy_s(wstrTempNXPCDir, MAX_PATH + 1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile\\");

	wcscpy_s(wstrInstallConfDir, MAX_PATH + 1, wstrInstPC);
	wcscat_s(wstrInstallConfDir, L"config\\config2.dat");
	wcscat_s(wstrTempNXPCDir, L"config\\config2.dat");
	SHCopy(wstrInstallConfDir, wstrTempNXPCDir, FALSE);

	//copy logging.properties
	wcscpy_s(wstrTempNXPCDir, MAX_PATH + 1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile\\");

	wcscpy_s(wstrInstallConfDir, MAX_PATH + 1, wstrInstPC);
	wcscat_s(wstrInstallConfDir, L"config\\logging.properties");
	wcscat_s(wstrTempNXPCDir, L"config\\logging.properties");
	SHCopy(wstrInstallConfDir, wstrTempNXPCDir, FALSE);
	
	//copy tamper_resistance
	wcscpy_s(wstrTempNXPCDir, MAX_PATH + 1, wstrTempDir);
	wcscat_s(wstrTempNXPCDir, L"NxPCFile\\");

	wcscpy_s(wstrInstallConfDir, MAX_PATH + 1, wstrInstPC);
	wcscat_s(wstrInstallConfDir, L"config\\tamper_resistance\\*");
	wcscat_s(wstrTempNXPCDir, L"tamper_resistance\\");
	SHCopy(wstrInstallConfDir, wstrTempNXPCDir, FALSE);	

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Back up Conf file done."));

    return ERROR_SUCCESS;
}

// when install a new endpoint client, we will check:
// 1. there is a injection.ini with MSI file. then we will use this one
// 2. if there is none, we will use the one inside installer
//
// when upgrade, we will do:
// 1. there is an injection.ini with MSI file. then we will use this one to replace
// 2. if there is none, we will use the existing one (not the one inside installer)
UINT __stdcall FindInjectionINIFile(MSIHANDLE hInstall)
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrTemp[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	UINT uiRet = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;
	BOOL bFindFile = FALSE;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start checking file injection.ini"));

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrTemp);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, FALSE, TEXT("Failed to get TEMP path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, L"NxPCFile\\");

	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(wstrTemp,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING | CREATE_NEW,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (hTempFile == INVALID_HANDLE_VALUE)
	{
		if (!CreateDirectory(wstrTemp, NULL))
		{
			dwErrorCode = GetLastError();
			if (dwErrorCode != ERROR_ALREADY_EXISTS)
			{
				swprintf_s(wstrMsg, 128, L"Failed to create temp path. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
				return ERROR_INSTALL_FAILURE;
			}
		}
	}
	CloseHandle(hTempFile);

	// Move file from source to temp
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, FILENAME_INJECTION);
	SetFileAttributes(wstrTemp, FILE_ATTRIBUTE_NORMAL);
	//Clean up temp file first
	DeleteFile(wstrTemp);

	if (bFindFile == FALSE)
	{
		// Check if file exists in current directory
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		uiRet = 0;
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);

		uiRet = MsiGetProperty(hInstall, TEXT("CURRENTDIRECTORY"), wstrSourceDir, &dwPathBuffer);
		if (uiRet != ERROR_SUCCESS)
		{
			dwErrorCode = GetLastError();
			swprintf_s(wstrMsg, 128, L"Failed to get current directory from installer. Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);

			return ERROR_INSTALL_FAILURE;
		}

		//Check if file exist
		if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
		{
			wcscat_s(wstrSourceDir, L"\\");
		}
		wcscat_s(wstrSourceDir, FILENAME_INJECTION);

		if (GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("The installer could not find the commprofile.xml file in the MSI folder."));
		}
		else
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use commprofile.xml from MSI folder"));
			bFindFile = TRUE;
		}
	}

	if (bFindFile == FALSE)
	{
		// Check File in installer folder c:\program files\nextlabs\policy controller\service
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);
		uiRet = 0;
		uiRet = MsiGetProperty(hInstall, TEXT("INSTALLDIR"), wstrSourceDir, &dwPathBuffer);
		if (uiRet == ERROR_SUCCESS)
		{
			if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
			{
				wcscat_s(wstrSourceDir, L"\\");
			}
			wcscat_s(wstrSourceDir, L"Policy Controller\\service\\");
			wcscat_s(wstrSourceDir, FILENAME_INJECTION);
			MessageAndLogging(hInstall, TRUE, wstrSourceDir);

			// exist in Policy Controller folder
			if ((GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES))
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find injection.ini from Program Files\\nextlabs\\policy controller\\service folder"));
			}
			else
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use injection.ini from existing Program Files\\nextlabs\\policy controller\\service folder"));
				bFindFile = TRUE;
			}
		}
	}

	if (bFindFile == FALSE)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find injection.ini both MSI folder and existing PC\\service folder. Let's use default one."));
		return ERROR_INSTALL_FAILURE;
	}

	// start copying to temp
	if (CopyFile(wstrSourceDir, wstrTemp, FALSE) == FALSE) //Failed
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to copy file to temp path. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE;
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Checking file injection.ini done.  Status: Good."));

	return ERROR_SUCCESS;
}

// same logic as above function
UINT __stdcall FindOutlookEnforcerINIFile(MSIHANDLE hInstall)
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrTemp[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	UINT uiRet = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;
	BOOL bFindFile = FALSE;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start checking file OutlookEnforcer.ini"));

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrTemp);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, FALSE, TEXT("Failed to get TEMP path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, L"NxPCFile\\");

	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(wstrTemp,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING | CREATE_NEW,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (hTempFile == INVALID_HANDLE_VALUE)
	{
		if (!CreateDirectory(wstrTemp, NULL))
		{
			dwErrorCode = GetLastError();
			if (dwErrorCode != ERROR_ALREADY_EXISTS)
			{
				swprintf_s(wstrMsg, 128, L"Failed to create temp path. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
				return ERROR_INSTALL_FAILURE;
			}
		}
	}
	CloseHandle(hTempFile);

	// Move file from source to temp
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, FILENAME_OEINI);
	SetFileAttributes(wstrTemp, FILE_ATTRIBUTE_NORMAL);
	//Clean up temp file first
	DeleteFile(wstrTemp);

	if (bFindFile == FALSE)
	{
		// Check if file exists in current directory
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		uiRet = 0;
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);

		uiRet = MsiGetProperty(hInstall, TEXT("CURRENTDIRECTORY"), wstrSourceDir, &dwPathBuffer);
		if (uiRet != ERROR_SUCCESS)
		{
			dwErrorCode = GetLastError();
			swprintf_s(wstrMsg, 128, L"Failed to get current directory from installer. Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);

			return ERROR_INSTALL_FAILURE;
		}

		//Check if file exist
		if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
		{
			wcscat_s(wstrSourceDir, L"\\");
		}
		wcscat_s(wstrSourceDir, FILENAME_OEINI);

		if (GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("The installer could not find the OutlookEnforcer.ini file in the MSI folder."));
		}
		else
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use OutlookEnforcer.ini from MSI folder"));
			bFindFile = TRUE;
		}
	}

	if (bFindFile == FALSE)
	{
		// Check File in installer folder c:\program files\nextlabs\outlook enforcer\config
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);
		uiRet = 0;
		uiRet = MsiGetProperty(hInstall, TEXT("INSTALLDIR"), wstrSourceDir, &dwPathBuffer);
		if (uiRet == ERROR_SUCCESS)
		{
			if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
			{
				wcscat_s(wstrSourceDir, L"\\");
			}
			wcscat_s(wstrSourceDir, L"Outlook Enforcer\\config\\");
			wcscat_s(wstrSourceDir, FILENAME_OEINI);
			MessageAndLogging(hInstall, TRUE, wstrSourceDir);

			// exist in Outlook Enforcer folder
			if ((GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES))
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find OutlookEnforcer.ini from Program Files\\nextlabs\\Outlook Enforcer\\config folder"));
			}
			else
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use OutlookEnforcer.ini from existing Program Files\\nextlabs\\Outlook Enforcer\\config folder"));
				bFindFile = TRUE;
			}
		}
	}

	if (bFindFile == FALSE)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find OutlookEnforcer.ini in both MSI folder and existing outlook enforcer folder. Let's use default one."));
		return ERROR_INSTALL_FAILURE;
	}

	// start copying to temp
	if (CopyFile(wstrSourceDir, wstrTemp, FALSE) == FALSE) //Failed
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to copy file to temp path. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE;
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Checking file outlookenforcer.ini done.  Status: Good."));

	return ERROR_SUCCESS;
}

// call in defered execution in system context
UINT __stdcall CopyInjectionINIFile(MSIHANDLE hInstall) //run in defered execution
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start copy injection.ini."));
	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir) / sizeof(WCHAR);
	if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only

		return ERROR_INSTALL_FAILURE;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}

	wstring wstrInstPC = wstrInstallDir;
	wstrInstPC += L"Policy Controller\\";

	wcscat_s(wstrInstallDir, L"Policy Controller\\service\\");
	wcscat_s(wstrInstallDir, FILENAME_INJECTION);

	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrSourceDir);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
	{
		wcscat_s(wstrSourceDir, MAX_PATH + 1, L"\\");
	}
	wcscat_s(wstrSourceDir, L"NxPCFile\\");
	wcscat_s(wstrSourceDir, FILENAME_INJECTION);

	//prevent read only file already existed
	SetFileAttributes(wstrInstallDir, FILE_ATTRIBUTE_NORMAL);

	//Move file from Temp to Install Directory
	if (CopyFile(wstrSourceDir, wstrInstallDir, FALSE) == FALSE)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Copy injection.ini file failed. Error Code: %d", dwErrorCode);

		//print log only
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrSourceDir);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrInstallDir);
		return ERROR_INSTALL_FAILURE;
	}

	//Clean up file
	DeleteFile(wstrSourceDir);
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Copy file injection.ini success."));

	return ERROR_SUCCESS;
}

// call in defered execution in system context
UINT __stdcall CopyOutlookEnforcerINIFile(MSIHANDLE hInstall) //run in defered execution
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start copy OutlookEnforcer.ini."));
	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir) / sizeof(WCHAR);
	if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only

		return ERROR_INSTALL_FAILURE;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}

	wstring wstrInstOE = wstrInstallDir;
	wstrInstOE += L"Outlook Enforcer\\";

	wcscat_s(wstrInstallDir, L"Outlook Enforcer\\config\\");

	if ((GetFileAttributes(wstrInstallDir) == INVALID_FILE_ATTRIBUTES))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: we guess outlook enforcer is not installed, wont copy anything"));
		return ERROR_INSTALL_FAILURE;
	}

	wcscat_s(wstrInstallDir, FILENAME_OEINI);

	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrSourceDir);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
	{
		wcscat_s(wstrSourceDir, MAX_PATH + 1, L"\\");
	}
	wcscat_s(wstrSourceDir, L"NxPCFile\\");
	wcscat_s(wstrSourceDir, FILENAME_OEINI);

	//prevent read only file already existed
	SetFileAttributes(wstrInstallDir, FILE_ATTRIBUTE_NORMAL);

	//Move file from Temp to Install Directory
	if (CopyFile(wstrSourceDir, wstrInstallDir, FALSE) == FALSE)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Copy OutlookEnforcer.ini file failed. Error Code: %d", dwErrorCode);

		//print log only
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrSourceDir);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrInstallDir);
		return ERROR_INSTALL_FAILURE;
	}

	//Clean up file
	DeleteFile(wstrSourceDir);
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Copy file OutlookEnforcer.ini success."));

	return ERROR_SUCCESS;
}

// call in defered execution in system context
// new installation, copy plugins\* to PC\jservice
// upgrade
//		if plugins is not-existed, backup PC\jservice and copy back
//		if plugins is empty, delete PC\jservice\*
//		if plugins is not empty, do not backup PC\jservice and copy plugins\* to PC\jservice
UINT __stdcall FindPluginsFile(MSIHANDLE hInstall)
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrTemp[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	UINT uiRet = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;
	BOOL bFindFile = FALSE;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start checking file injection.ini"));

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrTemp);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, FALSE, TEXT("Failed to get TEMP path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, L"NxPCFile\\");

	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(wstrTemp,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING | CREATE_NEW,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (hTempFile == INVALID_HANDLE_VALUE)
	{
		if (!CreateDirectory(wstrTemp, NULL))
		{
			dwErrorCode = GetLastError();
			if (dwErrorCode != ERROR_ALREADY_EXISTS)
			{
				swprintf_s(wstrMsg, 128, L"Failed to create temp path. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
				return ERROR_INSTALL_FAILURE;
			}
		}
	}
	CloseHandle(hTempFile);

	// Move file from source to temp
	if (wstrTemp[wcslen(wstrTemp) - 1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}
	wcscat_s(wstrTemp, L"jservice");
	//Clean up temp file first
	SHDelete(wstrTemp);

	if (bFindFile == FALSE)
	{
		// Check if file exists in current directory
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		uiRet = 0;
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);

		uiRet = MsiGetProperty(hInstall, TEXT("CURRENTDIRECTORY"), wstrSourceDir, &dwPathBuffer);
		if (uiRet != ERROR_SUCCESS)
		{
			dwErrorCode = GetLastError();
			swprintf_s(wstrMsg, 128, L"Failed to get current directory from installer. Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);

			return ERROR_INSTALL_FAILURE;
		}

		//Check if file exist
		if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
		{
			wcscat_s(wstrSourceDir, L"\\");
		}
		wcscat_s(wstrSourceDir, L"plugins");

		if (GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("The installer could not find the plugins in the MSI folder."));
		}
		else
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use plugins from MSI folder"));
			wcscat_s(wstrSourceDir, L"\\*");
			bFindFile = TRUE;
		}
	}

	if (bFindFile == FALSE)
	{
		// Check File in installer folder c:\program files\nextlabs\policy controller\service
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);
		uiRet = 0;
		uiRet = MsiGetProperty(hInstall, TEXT("INSTALLDIR"), wstrSourceDir, &dwPathBuffer);
		if (uiRet == ERROR_SUCCESS)
		{
			if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
			{
				wcscat_s(wstrSourceDir, L"\\");
			}
			wcscat_s(wstrSourceDir, L"Policy Controller\\jservice\\*");
			MessageAndLogging(hInstall, TRUE, wstrSourceDir);
			bFindFile = TRUE;
		}
	}

	if (bFindFile == FALSE)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find plugins in both MSI folder and existing PC\\jservice folder. Let's use default one."));
		return ERROR_INSTALL_FAILURE;
	}
	//copy JService							
	SHCopy(wstrSourceDir, wstrTemp, FALSE);

	// nlcc is an exception in JService, we need to delete this from backup folder
	WCHAR wstrNLCCDll[MAX_PATH + 1] = { 0 };
	WCHAR wstrNLCCProperty[MAX_PATH + 1] = { 0 };
	WCHAR wstrNLCCJar[MAX_PATH + 1] = { 0 };

	wcscpy_s(wstrNLCCDll, MAX_PATH + 1, wstrTemp);
	wcscat_s(wstrNLCCDll, L"jar\\nlcc\\nlcc_dispatcher.dll");
	SHDelete(wstrNLCCDll);

	wcscpy_s(wstrNLCCJar, MAX_PATH + 1, wstrTemp);
	wcscat_s(wstrNLCCJar, L"jar\\nlcc\\NLCCService.jar");
	SHDelete(wstrNLCCJar);

	wcscpy_s(wstrNLCCProperty, MAX_PATH + 1, wstrTemp);
	wcscat_s(wstrNLCCProperty, L"config\\NLCCService.properties");
	SHDelete(wstrNLCCProperty);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Checking file plugins done.  Status: Good."));

	return ERROR_SUCCESS;
}

UINT __stdcall CopyPluginsFile(MSIHANDLE hInstall) //run in defered execution
{
	WCHAR wstrSourceDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = 0;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start copy Plugins."));
	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir) / sizeof(WCHAR);
	if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only

		return ERROR_INSTALL_FAILURE;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}

	wcscat_s(wstrInstallDir, L"Policy Controller\\jservice\\");

	if ((GetFileAttributes(wstrInstallDir) == INVALID_FILE_ATTRIBUTES))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: we guess poli is not installed, wont copy anything"));
		return ERROR_INSTALL_FAILURE;
	}

	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH + 1, wstrSourceDir);
	if ((dwRetVal > MAX_PATH + 1) || (dwRetVal == 0))
	{
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
		return ERROR_INSTALL_FAILURE;
	}

	// verify temp path exists
	if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
	{
		wcscat_s(wstrSourceDir, MAX_PATH + 1, L"\\");
	}
	WCHAR wstrSourceDir2[MAX_PATH + 1] = { 0 };
	wcscpy_s(wstrSourceDir2, wstrSourceDir);

	wcscat_s(wstrSourceDir, L"NxPCFile\\jservice\\*");
	wcscat_s(wstrSourceDir2, L"NxPCFile\\jservice\\");

	//Copy file from Temp to Install Directory
	SHCopy(wstrSourceDir, wstrInstallDir, TRUE);

	//Clean up file
	RemoveDirectory(wstrSourceDir2);
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Copy plugins success."));

	return ERROR_SUCCESS;
}

//CAFindCommFile commprofile.xml, call in immediate excution
UINT __stdcall FindConfigFile(MSIHANDLE hInstall )
{
	WCHAR wstrSourceDir[MAX_PATH+1] = {0};
	WCHAR wstrTemp[MAX_PATH+1] = {0};
	DWORD dwPathBuffer = 0;
	UINT uiRet = 0;
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;
	BOOL bFindFile = FALSE;

	// call to find and backup injection.ini first
	FindInjectionINIFile(hInstall);
	FindOutlookEnforcerINIFile(hInstall);
	FindPluginsFile(hInstall);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start checking file commprofile.xml"));

	// Check File in installer folder c:\program files\nextlabs\policy controller\config
	ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
	dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);
	uiRet = 0;
	uiRet = MsiGetProperty(hInstall, TEXT("INSTALLDIR"), wstrSourceDir, &dwPathBuffer);
	if (uiRet == ERROR_SUCCESS)
	{
		if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
		{
			wcscat_s(wstrSourceDir, L"\\");
		}
		wcscat_s(wstrSourceDir, L"Policy Controller\\config\\");
		wcscat_s(wstrSourceDir, FILENAME_COMM);
		MessageAndLogging(hInstall, TRUE, wstrSourceDir);

		// exist in Policy Controller folder
		if ((GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't find from Program Files\\nextlabs\\policy controller\\config folder"));
		}
		else
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use commprofile.xml from existing Program Files\\nextlabs\\policy controller\\config folder"));
			bFindFile = TRUE;
		}
	}

	if (bFindFile == FALSE)
	{
		// Check if file exists in current directory
		ZeroMemory(wstrSourceDir, sizeof(wstrSourceDir));
		uiRet = 0;
		dwPathBuffer = sizeof(wstrSourceDir) / sizeof(WCHAR);

		uiRet = MsiGetProperty(hInstall, TEXT("CURRENTDIRECTORY"), wstrSourceDir, &dwPathBuffer);
		if (uiRet != ERROR_SUCCESS)
		{
			dwErrorCode = GetLastError();
			swprintf_s(wstrMsg, 128, L"Failed to get current directory from installer. Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);

			return ERROR_INSTALL_FAILURE;
		}

		//Check if file exist
		if (wstrSourceDir[wcslen(wstrSourceDir) - 1] != L'\\')
		{
			wcscat_s(wstrSourceDir, L"\\");
		}
		wcscat_s(wstrSourceDir, FILENAME_COMM);

		if (GetFileAttributes(wstrSourceDir) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			MessageAndLogging(hInstall, FALSE, TEXT("The installer could not find the commprofile.xml file in the install folder. Contact Administrator to obtain this file."));
			return ERROR_INSTALL_FAILURE;
		}

		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Use commprofile.xml from MSI folder"));
		bFindFile = TRUE;
	}

	//get temp path
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH+1, wstrTemp);                                 
    if ((dwRetVal > MAX_PATH+1) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, FALSE, TEXT("Failed to get TEMP path in this computer."));
        return ERROR_INSTALL_FAILURE;
    }
	
	// verify temp path exists
	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	hTempFile = CreateFile(	wstrTemp,
							GENERIC_READ,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING|CREATE_NEW,
							FILE_FLAG_BACKUP_SEMANTICS,
							NULL);
		
	if ( hTempFile == INVALID_HANDLE_VALUE ) 
	{
		if (!CreateDirectory(wstrTemp, NULL))
		{
			dwErrorCode = GetLastError();
			if ( dwErrorCode != ERROR_ALREADY_EXISTS )
			{
				swprintf_s(wstrMsg, 128, L"Failed to create temp path. Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
				return ERROR_INSTALL_FAILURE;
			}
		}		
	}
	CloseHandle(hTempFile);
	
	//Move file from source to temp
	if(wstrTemp[wcslen(wstrTemp)-1] != L'\\')
	{
		wcscat_s(wstrTemp, L"\\");
	}	
	wcscat_s(wstrTemp, FILENAME_COMM);

	SetFileAttributes(wstrTemp, FILE_ATTRIBUTE_NORMAL);
	
	if (CopyFile(wstrSourceDir, wstrTemp, FALSE) == FALSE) //Failed
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to copy file to temp path. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, FALSE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE; 
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Checking file commprofile.xml done.  Status: Good."));

    return ERROR_SUCCESS;
}

//CACopyCommfile, call in defered execution in system context
UINT __stdcall CopyConfigFile(MSIHANDLE hInstall ) //run in defered execution
{
	WCHAR wstrSourceDir[MAX_PATH+1] = {0};
	WCHAR wstrInstallDir[MAX_PATH+1] = {0};
	WCHAR wstrTemp[MAX_PATH+1] = {0};
	DWORD dwPathBuffer = 0;
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;

	// call to copy injection.ini first
	CopyInjectionINIFile(hInstall);
	CopyOutlookEnforcerINIFile(hInstall);
	CopyPluginsFile(hInstall);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start copy commprofile.xml."));
	//get current Installdir from MSI
	dwPathBuffer = sizeof(wstrInstallDir)/sizeof(WCHAR);
	if(ERROR_SUCCESS !=  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only
	
		return ERROR_INSTALL_FAILURE;
	}

	if(wstrInstallDir[wcslen(wstrInstallDir)-1]!= L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}	

	wstring wstrInstPC = wstrInstallDir;
	wstrInstPC += L"Policy Controller\\";

	wcscat_s(wstrInstallDir, L"Policy Controller\\config\\");
	wcscat_s(wstrInstallDir, FILENAME_COMM);
	
	//get file from temp
	DWORD dwRetVal = 0;
	dwRetVal = GetTempPath(MAX_PATH+1, wstrSourceDir);                                 
    if ((dwRetVal > MAX_PATH+1) || (dwRetVal == 0))
    {
		MessageAndLogging(hInstall, TRUE, TEXT("Failed to get temp path in this computer."));
        return ERROR_INSTALL_FAILURE;
    }

	if(wstrSourceDir[wcslen(wstrSourceDir)-1]!= L'\\')
	{
		wcscat_s(wstrSourceDir, MAX_PATH+1,  L"\\");
	}
	wcscat_s(wstrSourceDir,  FILENAME_COMM);

	//prevent read only file already existed
	SetFileAttributes(wstrInstallDir, FILE_ATTRIBUTE_NORMAL); 
	
	//Move file from Temp to Install Directory
	if(CopyFile(wstrSourceDir, wstrInstallDir, FALSE)== FALSE)
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Copy commprofile.xml file failed. Error Code: %d", dwErrorCode);

		//print log only
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrSourceDir);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrInstallDir);
		return ERROR_INSTALL_FAILURE; 
	}

	//Clean up file
	DeleteFile(wstrSourceDir);
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Copy file commprofile.xml success."));

	//------start to change log
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: start update logging.properties."));
	wstring strLogPath = wstrInstPC;

	ReplaceString(strLogPath, L"\\", L"//");
	
	wstring strInFileName = wstrInstPC;
	strInFileName += L"config\\logging.template.properties";
	
	wstring strOutFileName = wstrInstPC;
	strOutFileName += L"config\\logging.properties";

	wfstream wfileIn1;
	wfstream wfileOut1; 

	wfileIn1.open(strInFileName, ios::in);
	if (wfileIn1.bad())
	{
		return ERROR_SUCCESS;
	}
	wfileOut1.open(strOutFileName, ios::out);
	if (wfileOut1.bad())
	{
		return ERROR_SUCCESS;
	}

	ReplaceStringInFile(wfileIn1, wfileOut1, L"[BLUEJUNGLE_HOME]", strLogPath);
	
	wfileIn1.close();
	wfileOut1.close();

	DeleteFile(strInFileName.c_str());
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Set logging.properties success."));
    return ERROR_SUCCESS;

}

UINT __stdcall SetLogProperty(MSIHANDLE hInstall )
{
	//return ERROR_SUCCESS;
	WCHAR wstrInstPC[MAX_PATH+1] = {0};
	HKEY hKey = NULL;
	BOOL bFoundInstDir =FALSE;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start to config log setting."));
	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
									TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\"),
									0, 
									KEY_READ, 
									&hKey))
	{		
		DWORD dwBufsize = sizeof(wstrInstPC)*sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, 			
											TEXT("PolicyControllerDir"),
											NULL, 
											NULL, 
											(LPBYTE)wstrInstPC, 
											&dwBufsize))
		{			
				bFoundInstDir = TRUE;			
		}
		RegCloseKey(hKey);
	}
	
	if(!bFoundInstDir)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("The PolicyControllerDir does not found. "));
		return ERROR_SUCCESS;
	}

	
	wstring strLogPath = wstrInstPC;
	// strLogPath += L"logs\\";
	ReplaceString(strLogPath, L"\\", L"//");
	
	wstring strInFileName = wstrInstPC;
	strInFileName += L"config\\logging.template.properties";
	
	wstring strOutFileName = wstrInstPC;
	strOutFileName += L"config\\logging.properties";

	wfstream wfileIn1;
	wfstream wfileOut1; 

	wfileIn1.open(strInFileName, ios::in);
	if (wfileIn1.bad())
	{
		return ERROR_SUCCESS;
	}
	wfileOut1.open(strOutFileName, ios::out);
	if (wfileOut1.bad())
	{
		return ERROR_SUCCESS;
	}

	ReplaceStringInFile(wfileIn1, wfileOut1, L"[BLUEJUNGLE_HOME]", strLogPath);
	
	wfileIn1.close();
	wfileOut1.close();

	DeleteFile(strInFileName.c_str());

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Set log config success."));

	return ERROR_SUCCESS;
}


UINT __stdcall MyTest(MSIHANDLE hInstall )
{
	MessageAndLogging(hInstall, FALSE, L"Hello world, I am here # 1 " );	
	return ERROR_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// for bug 32712, the file was used by outlook, we had to remove after reboot otherwise, we need to have user kill outlook.
bool WINAPI FindOutlookProcess()
{
    bool bOutlookRunning = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_wcsicmp(entry.szExeFile, L"outlook.exe") == 0)
            {
                bOutlookRunning = true;
                break;
            }
        }
    }

    CloseHandle(snapshot);
    return bOutlookRunning;
}

bool DelFolder(const wchar_t *cFilePath, MSIHANDLE hInstall)
{
    bool bDel = true;
	WIN32_FIND_DATA data;
    HANDLE hFind;
    wchar_t cFullPath[512] = { 0 };
    wchar_t cNewPath[512] = { 0 };
    wchar_t wstrMsg[128] = { 0 };
    wsprintfW(cFullPath, L"%s\\*.*", cFilePath);
    hFind = FindFirstFile(cFullPath, &data);
    do
    {
        if ((!wcscmp(L".", data.cFileName)) || (!wcscmp(L"..", data.cFileName)))
        {
            continue;
        }

        if (data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
        {
            wsprintfW(cNewPath, L"%s\\%s", cFilePath, data.cFileName);
            DelFolder(cNewPath,hInstall);
        }
        else
        {
            wsprintfW(cFullPath, L"%s\\%s", cFilePath, data.cFileName);

			wchar_t wstrFileName[MAX_PATH + 1] = { 0 };
			wcscpy_s(wstrFileName, MAX_PATH + 1, cFullPath);
			_wcslwr_s(wstrFileName, wcslen(wstrFileName) + 1);

			WCHAR wstrMsg[512] = { 0 };
			swprintf_s(wstrMsg, 512, L"NXPCLOG: delete the file: %s", cFullPath);
			MessageAndLogging(hInstall, TRUE, wstrMsg);

			bool bDelayDelete = true;
			wchar_t* pStrFind = wcsstr(wstrFileName, L"celog");
			wchar_t* pStrFindCommonBin = wcsstr(wstrFileName, L"common\\bin");
			wchar_t* pStrFindDesktopBin = wcsstr(wstrFileName, L"desktop enforcer\\bin");
			if (nullptr == pStrFind && nullptr == pStrFindCommonBin && nullptr == pStrFindDesktopBin)
			{
				if (!DeleteFile(cFullPath))
				{
					// DeleteFile failed, delay delete
					MessageAndLogging(hInstall, TRUE, L"DeleteFile failed, call MoveFile to delay delete");
					bDel = false;
				}
				else
				{
					// File is deleted and not in the delay delete list: common\bin and desktop enforcer\bin
					bDelayDelete = false;
				}
			}

			if (bDelayDelete == true)
			{
				MoveFileEx(cFullPath, 0, MOVEFILE_DELAY_UNTIL_REBOOT);
				bDel = false;
			}
        }

    } while (FindNextFile(hFind, &data));

    if (!RemoveDirectory(cFilePath))
    {
        MoveFileEx(cFilePath, 0, MOVEFILE_DELAY_UNTIL_REBOOT);
    }

    return bDel;
}

//////////////////////////////////////////////////////////////////////////

//this function has to be called in defered execution in system context
UINT __stdcall DeleteFolderAfterReboot(MSIHANDLE hInstall)
{
	WCHAR wstrInstallDir[MAX_PATH+1] = {0};
	DWORD dwPathBuffer = MAX_PATH+1;
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start clean up pending delete folders."));	
	dwPathBuffer = sizeof(wstrInstallDir)/sizeof(WCHAR);
	if(ERROR_SUCCESS !=  MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_INSTALL_FAILURE;
	}
	
    WCHAR wstrTmp1[MAX_PATH + 1] = { 0 };
    wcscpy_s(wstrTmp1, MAX_PATH + 1, wstrInstallDir);
    wcscat_s(wstrTmp1, L"diags");
    bool bOutlookRunning = FindOutlookProcess();
    if (bOutlookRunning)   //for pending reboot deletion
    {
        DelFolder(wstrTmp1, hInstall);
    }
    else
    {
        SHDelete(wstrTmp1);
    }

    WCHAR wstrTmp2[MAX_PATH + 1] = { 0 };
    wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
    wcscat_s(wstrTmp2, L"Outlook Enforcer");
    SHDelete(wstrTmp2);

	// Delete the whole "Program Files\NextLabs folder"
	// Except the files which name contains "celog", delay deleting
	// also Except the files under Common\bin, Desktop Enforcer\bin, delay deleting
	wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
	wcscat_s(wstrTmp2, L"Rights Management");
	if (GetFileAttributes(wstrTmp2) == INVALID_FILE_ATTRIBUTES)
	{
		DelFolder(wstrInstallDir, hInstall);
	}
	else
	{
		wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
		wcscat_s(wstrTmp2, L"Outlook Enforcer");
		DelFolder(wstrTmp2, hInstall);

		wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
		wcscat_s(wstrTmp2, L"Policy Controller");
		DelFolder(wstrTmp2, hInstall);

		wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
		wcscat_s(wstrTmp2, L"Desktop Enforcer");
		DelFolder(wstrTmp2, hInstall);

		wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
		wcscat_s(wstrTmp2, L"diags");
		DelFolder(wstrTmp2, hInstall);

		wcscpy_s(wstrTmp2, MAX_PATH + 1, wstrInstallDir);
		wcscat_s(wstrTmp2, L"Common");
		DelFolder(wstrTmp2, hInstall);
	}


	//  Delete the File: "celog2.dll" & "celog232.dll"
	MessageAndLogging(hInstall, TRUE, TEXT("Delay removing ('celog' & 'celog2') | ('celog32' & 'celog232') binary file in System folder..."));

	PVOID OldValue = NULL;
	//  Disable redirection immediately prior to the native API
	//  function call.
	if (Wow64DisableWow64FsRedirection(&OldValue))
	{
		// x64 Binary files in x64 system
		MoveFileEx(TEXT("C:\\Windows\\System32\\celog.dll"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		MoveFileEx(TEXT("C:\\Windows\\System32\\celog2.dll"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		//  Immediately re-enable redirection. Note that any resources
		//  associated with OldValue are cleaned up by this call.
		if (FALSE == Wow64RevertWow64FsRedirection(OldValue))
		{
			//  Failure to re-enable redirection should be considered
			//  a criticial failure and execution aborted.
			MessageAndLogging(hInstall, TRUE, TEXT("Fail to re-enable redirection in x64 System..."));
		}

	}

	// x86 Binary file in x86|x64 system
	MoveFileEx(TEXT("C:\\Windows\\System32\\celog32.dll"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	MoveFileEx(TEXT("C:\\Windows\\System32\\celog232.dll"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Clean up pending delete folder completed."));
	return ERROR_SUCCESS;
}

//this function has to be called in defered execution in system context
UINT __stdcall DeleteJServiceFolder(MSIHANDLE hInstall)
{
	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	DWORD dwPathBuffer = MAX_PATH + 1;
	WCHAR wstrMsg[128] = { 0 };
	DWORD dwErrorCode = 0;

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start delete jservice folder."));
	dwPathBuffer = sizeof(wstrInstallDir) / sizeof(WCHAR);
	if (ERROR_SUCCESS != MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwPathBuffer))
	{
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get install directory from installer. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_INSTALL_FAILURE;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, L"\\");
	}

	wcscat_s(wstrInstallDir, L"Policy Controller\\jservice\\");
	SHDelete(wstrInstallDir);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Clean up pending delete folder completed."));
	return ERROR_SUCCESS;
}

UINT __stdcall SetEnv(MSIHANDLE hInstall, const wchar_t *cPath)
{
    MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start to Set Environment Variable. "));

	WCHAR wstrData[1024] = {0};
	WCHAR wstrMsg[128] = {0};
	DWORD dwErrorCode = 0;

	wcscpy_s(wstrData, MAX_PATH + 1, cPath);
	MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrData);

	DWORD buffSize = 65535;
	WCHAR buffer[65535] = {0};
    if (GetEnvironmentVariable(L"Path", buffer, buffSize) == NULL)
    {
		dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to get current Environment variable. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE;
    }
   
	wstring strPath;
	strPath = buffer;
	strPath +=  L";";
	strPath += wstrData;

	MessageAndLogging(hInstall, TRUE, (LPCWSTR)strPath.c_str());

	BOOL bSet = false;
	bSet = SetEnvironmentVariable(L"Path", (LPCWSTR)strPath.c_str());

	if (!bSet) 
    {
        dwErrorCode = GetLastError();
		swprintf_s(wstrMsg, 128, L"Failed to set environment variable. Error Code: %d", dwErrorCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		return ERROR_INSTALL_FAILURE;
    }
	

	 MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Set Environment Variable completed. "));

	return ERROR_SUCCESS;
}

UINT __stdcall RefreshEnv(MSIHANDLE hInstall)
{
	HKEY hKey = NULL;
	WCHAR wstrInstPC[MAX_PATH + 1] = { 0 };
	WCHAR wstrPCBin[MAX_PATH + 1] = { 0 };
	WCHAR wstrCommBin32[MAX_PATH + 1] = { 0 };
	WCHAR wstrCommBin64[MAX_PATH + 1] = { 0 };
	WCHAR wstrMsg[512] = { 0 };
	DWORD dwErrorCode = 0;

	//get PC installed path
	BOOL bFoundInstDir = FALSE;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller\\"),
		0,
		KEY_READ,
		&hKey))
	{
		DWORD dwBufsize = sizeof(wstrInstPC)*sizeof(WCHAR);
		if (ERROR_SUCCESS == RegQueryValueEx(hKey,
			TEXT("InstallDir"),
			NULL,
			NULL,
			(LPBYTE)wstrInstPC,
			&dwBufsize))
		{
			bFoundInstDir = TRUE;
		}
		RegCloseKey(hKey);
	}

	if (bFoundInstDir)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("The install path was found. "));

		wcscpy_s(wstrCommBin32, MAX_PATH + 1, wstrInstPC);
		wcscat_s(wstrCommBin32, L"Common\\bin32\\");

		wcscpy_s(wstrCommBin64, MAX_PATH + 1, wstrInstPC);
		wcscat_s(wstrCommBin64, L"Common\\bin64\\");

		wcscpy_s(wstrPCBin, MAX_PATH + 1, wstrInstPC);
		wcscat_s(wstrPCBin, L"Policy Controller\\bin\\");

		SetEnv(hInstall, wstrPCBin);
		SetEnv(hInstall, wstrCommBin32);
		SetEnv(hInstall, wstrCommBin64);
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start to Refresh Environment Variable. "));

	DWORD dwReturnValue = 0;
	SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Environment"), SMTO_ABORTIFHUNG, 5000, &dwReturnValue);

	if (dwReturnValue == 0)
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Refresh Environment Variable failed. "));
		return ERROR_SUCCESS;
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Refresh Environment Variable completed. "));

	return ERROR_SUCCESS;
}

UINT __stdcall ResetServiceStatus(MSIHANDLE hInstall)
{
	// this function is called when upgrading
	// the purpose is to make sure nltamper/nlcc/nlinjection/procdetect drivers can still work after reboot
	// actually, it is a bug for installshield
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Reset Service Status started. "));

	HKEY hKey = NULL;
	DWORD dwErrorCode = 0;
	WCHAR wstrData[MAX_PATH + 1] = { 0 };

	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\procdetect]
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\services\\procdetect\\"),
		0,
		KEY_ALL_ACCESS,
		&hKey))
	{
		DWORD dwBufsize = sizeof(wstrData)*sizeof(WCHAR);
		dwErrorCode = RegQueryValueEx(hKey,
			TEXT("DeleteFlag"),
			NULL,
			NULL,
			(LPBYTE)wstrData,
			&dwBufsize);

		if (ERROR_SUCCESS == dwErrorCode)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: procdetect service is marked to delete. "));

			// service is marked to delete
			// delete this flag
			dwErrorCode = RegDeleteValue(hKey, TEXT("DeleteFlag"));
			if (ERROR_SUCCESS == dwErrorCode)
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: procdetect serivce now is restored."));
			}
			else
			{
				WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
				swprintf_s(wstrMsg, 128, L"Failed to RegDeleteValue(DeleteFlag). Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
			}
		}
		else
		{
			WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
			swprintf_s(wstrMsg, 128, L"Failed to RegQueryValueEx(DeleteFlag). Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}


		// reset the Start Mode
		// "Start"=dword:00000002
		DWORD value = 2;
		if (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("Start"), 0, REG_DWORD, (const BYTE*)&value, sizeof(value)))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: procdetect serivce start mode now is restored."));
		}

		RegCloseKey(hKey);
	}

	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\nlcc]
	hKey = NULL;
	dwErrorCode = 0;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\services\\nlcc\\"),
		0,
		KEY_ALL_ACCESS,
		&hKey))
	{
		DWORD dwBufsize = sizeof(wstrData)*sizeof(WCHAR);
		dwErrorCode = RegQueryValueEx(hKey,
			TEXT("DeleteFlag"),
			NULL,
			NULL,
			(LPBYTE)wstrData,
			&dwBufsize);

		if (ERROR_SUCCESS == dwErrorCode)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlcc service is marked to delete. "));

			// service is marked to delete
			// delete this flag
			dwErrorCode = RegDeleteValue(hKey, TEXT("DeleteFlag"));
			if (ERROR_SUCCESS == dwErrorCode)
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlcc serivce now is restored."));
			}
			else
			{
				WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
				swprintf_s(wstrMsg, 128, L"Failed to RegDeleteValue(DeleteFlag). Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
			}
		}
		else
		{
			WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
			swprintf_s(wstrMsg, 128, L"Failed to RegQueryValueEx(DeleteFlag). Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}

		// reset the Start Mode
		// "Start"=dword:00000001
		DWORD value = 1;
		if (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("Start"), 0, REG_DWORD, (const BYTE*)&value, sizeof(value)))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlcc serivce start mode now is restored."));
		}

		RegCloseKey(hKey);
	}

	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\nlinjection]
	hKey = NULL;
	dwErrorCode = 0;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\services\\nlinjection\\"),
		0,
		KEY_ALL_ACCESS,
		&hKey))
	{
		DWORD dwBufsize = sizeof(wstrData)*sizeof(WCHAR);
		dwErrorCode = RegQueryValueEx(hKey,
			TEXT("DeleteFlag"),
			NULL,
			NULL,
			(LPBYTE)wstrData,
			&dwBufsize);

		if (ERROR_SUCCESS == dwErrorCode)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlinjection service is marked to delete. "));

			// service is marked to delete
			// delete this flag
			dwErrorCode = RegDeleteValue(hKey, TEXT("DeleteFlag"));
			if (ERROR_SUCCESS == dwErrorCode)
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlinjection serivce now is restored."));
			}
			else
			{
				WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
				swprintf_s(wstrMsg, 128, L"Failed to RegDeleteValue(DeleteFlag). Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
			}
		}
		else
		{
			WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
			swprintf_s(wstrMsg, 128, L"Failed to RegQueryValueEx(DeleteFlag). Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}

		// reset the Start Mode
		// "Start"=dword:00000001
		DWORD value = 1;
		if (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("Start"), 0, REG_DWORD, (const BYTE*)&value, sizeof(value)))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: nlinjection serivce start mode now is restored."));
		}

		RegCloseKey(hKey);
	}

	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\NLTamper]
	hKey = NULL;
	dwErrorCode = 0;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\services\\NLTamper\\"),
		0,
		KEY_ALL_ACCESS,
		&hKey))
	{
		DWORD dwBufsize = sizeof(wstrData)*sizeof(WCHAR);
		dwErrorCode = RegQueryValueEx(hKey,
			TEXT("DeleteFlag"),
			NULL,
			NULL,
			(LPBYTE)wstrData,
			&dwBufsize);

		if (ERROR_SUCCESS == dwErrorCode)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: NLTamper service is marked to delete. "));

			// service is marked to delete
			// delete this flag
			dwErrorCode = RegDeleteValue(hKey, TEXT("DeleteFlag"));
			if (ERROR_SUCCESS == dwErrorCode)
			{
				MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: NLTamper serivce now is restored."));
			}
			else
			{
				WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
				swprintf_s(wstrMsg, 128, L"Failed to RegDeleteValue(DeleteFlag). Error Code: %d", dwErrorCode);
				MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
			}
		}
		else
		{
			WCHAR wstrMsg[MAX_PATH + 1] = { 0 };
			swprintf_s(wstrMsg, 128, L"Failed to RegQueryValueEx(DeleteFlag). Error Code: %d", dwErrorCode);
			MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);
		}

		// reset the Start Mode
		// "Start"=dword:00000003
		DWORD value = 3;
		if (ERROR_SUCCESS == RegSetValueEx(hKey, TEXT("Start"), 0, REG_DWORD, (const BYTE*)&value, sizeof(value)))
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: NLTamper serivce start mode now is restored."));
		}

		RegCloseKey(hKey);
	}

	ResetDelayDelete(hInstall);

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Reset Service Status finished. "));
	return ERROR_SUCCESS;
}

UINT __stdcall ResetInjectionPath(MSIHANDLE hInstall)
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start Reset Injection Driver Path in config files. "));

	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrMsg[512] = { 0 };
	DWORD dwErrorCode = 0;

	//get current Installdir from MSI
	DWORD dwBufsize = sizeof(wstrInstallDir) / sizeof(WCHAR);
	UINT uiRetCode = MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwBufsize);
	if (ERROR_SUCCESS != uiRetCode)
	{
		swprintf_s(wstrMsg, 512, L"NXPCLOG: Failed to get install directory from MSI. Error Code: %d", uiRetCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_SUCCESS;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, _countof(wstrInstallDir), L"\\");
	}

	// file content
	wstring wstrHookContentOfacrobat_exe_ini = L"Hook=";
	wstring wstrHookContentOfacroRd32_exe_ini = L"Hook=";

	// the files we need to replace
	wstring wstracrobat_exe_ini = wstrInstallDir;
	wstracrobat_exe_ini += L"Policy Controller\\service\\injection\\acrobat.exe.ini";
	wstring wstracroRd32_exe_ini = wstrInstallDir;
	wstracroRd32_exe_ini += L"Policy Controller\\service\\injection\\acroRd32.exe.ini";

	// the content we need to replace with
	wstring wstrOEAdobePEPTrm32 = wstrInstallDir;
	wstrOEAdobePEPTrm32 += L"Outlook Enforcer\\bin\\CE_AdobePEPTrm32.dll";
	wstring wstrWDEAdobePEPTrm32 = wstrInstallDir;
	wstrWDEAdobePEPTrm32 += L"Desktop Enforcer\\bin\\AdobePEPTrm32.dll";

	// if OE is installed
	if ((GetFileAttributes(wstrOEAdobePEPTrm32.c_str()) == INVALID_FILE_ATTRIBUTES))
	{
		// can't find this dll file under OE path, then we think OE is not installed
		wstrHookContentOfacrobat_exe_ini += wstrWDEAdobePEPTrm32;
		wstrHookContentOfacroRd32_exe_ini += wstrWDEAdobePEPTrm32;
	}
	else
	{
		// find this dll file under OE path, then we think OE is installed
		wstrHookContentOfacrobat_exe_ini += wstrOEAdobePEPTrm32;
		wstrHookContentOfacroRd32_exe_ini += wstrOEAdobePEPTrm32;
	}

	MessageAndLogging(hInstall, TRUE, wstrInstallDir);//log only	
	MessageAndLogging(hInstall, TRUE, wstracrobat_exe_ini.c_str());//log only	
	MessageAndLogging(hInstall, TRUE, wstrHookContentOfacrobat_exe_ini.c_str());//log only	
	MessageAndLogging(hInstall, TRUE, wstracroRd32_exe_ini.c_str());//log only	
	MessageAndLogging(hInstall, TRUE, wstrHookContentOfacroRd32_exe_ini.c_str());//log only	

	// now, replace the content in the _exe_ini files
	wfstream wfile1;
	wfstream wfile2;

	wfile1.open(wstracrobat_exe_ini, ios::out | ios::trunc);
	if (wfile1.bad())
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't open acrobat.exe.ini to write."));
		return ERROR_SUCCESS;
	}

	wfile1 << wstrHookContentOfacrobat_exe_ini << endl;
	wfile1.close();

	wfile2.open(wstracroRd32_exe_ini, ios::out | ios::trunc);
	if (wfile2.bad())
	{
		MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't open acrobat.exe.ini to write."));
		return ERROR_SUCCESS;
	}

	wfile2 << wstrHookContentOfacroRd32_exe_ini << endl;
	wfile2.close();

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Reset Injection Driver Path in config files done."));

	return ERROR_SUCCESS;

}


UINT __stdcall ResetDelayDelete(MSIHANDLE hInstall)
{
	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Start Reset Delay Delete."));

	WCHAR wstrInstallDir[MAX_PATH + 1] = { 0 };
	WCHAR wstrMsg[512] = { 0 };
	DWORD dwErrorCode = 0;

	//get current Installdir from MSI
	DWORD dwBufsize = sizeof(wstrInstallDir) / sizeof(WCHAR);
	UINT uiRetCode = MsiGetProperty(hInstall, TEXT("CustomActionData"), wstrInstallDir, &dwBufsize);
	if (ERROR_SUCCESS != uiRetCode)
	{
		swprintf_s(wstrMsg, 512, L"NXPCLOG: Failed to get install directory from MSI. Error Code: %d", uiRetCode);
		MessageAndLogging(hInstall, TRUE, (LPCWSTR)wstrMsg);//log only	
		return ERROR_SUCCESS;
	}

	if (wstrInstallDir[wcslen(wstrInstallDir) - 1] != L'\\')
	{
		wcscat_s(wstrInstallDir, _countof(wstrInstallDir), L"\\");
	}

	LONG lResult = 0;
	HKEY hKey = NULL;
	LPTSTR lpValues = NULL;
	LPTSTR lpValue = NULL;
	LPTSTR lpNewValues = NULL;
	LPTSTR lpNewValue = NULL;
	DWORD cbValues = 0;
	DWORD cbNewValues = 0;
	DWORD cbNewValue = 0;
	BOOL bFound = FALSE;

	__try
	{
		// OPEN THE REGISTRY KEY
		//
		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			DELAYDELETE_DIR,
			0,
			KEY_ALL_ACCESS,
			&hKey
		);
		if (ERROR_SUCCESS != lResult)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't open registry PendingFileRenameOperations."));
			return ERROR_SUCCESS;
		}

		// READ THE REG_MULTI_SZ VALUES
		//
		// Get size of the buffer for the values
		lResult = RegQueryValueEx(
			hKey,
			DELAYDELETE_KEY,
			NULL,
			NULL,
			NULL,
			&cbValues
		);
		if (ERROR_SUCCESS != lResult)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't open size of registry PendingFileRenameOperations."));
			return ERROR_SUCCESS;
		}

		// Allocate the buffer
		lpValues = (LPTSTR)malloc(cbValues);
		if (NULL == lpValues)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't new value buffer of registry PendingFileRenameOperations."));
			return ERROR_SUCCESS;
		}

		// Get the values
		lResult = RegQueryValueEx(
			hKey,
			DELAYDELETE_KEY,
			NULL,
			NULL,
			(LPBYTE)lpValues,
			&cbValues
		);
		if (ERROR_SUCCESS != lResult)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't read value of registry PendingFileRenameOperations."));
			return ERROR_SUCCESS;
		}


		// SHOW THE VALUES
		//
		MessageAndLogging(hInstall, TRUE, TEXT("======== value of PendingFileRenameOperations ======="));
		lpValue = lpValues;
		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 1)
		{
			// Show one value
			MessageAndLogging(hInstall, TRUE, lpValue);
		}
		MessageAndLogging(hInstall, TRUE, TEXT("###### end ######"));

		// INSERT A NEW VALUE AFTER A SPECIFIC VALUE IN THE LIST OF VALUES
		//
		// Allocate a new buffer for the old values plus the new one
		cbNewValues = cbValues + cbNewValue;
		lpNewValues = (LPTSTR)malloc(cbNewValues);
		if (NULL == lpNewValues)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't allocate memory"));
			return ERROR_SUCCESS;
		}
		memset(lpNewValues, 0, cbNewValues);

		cbNewValues = 0;

		// Find the value after which we will insert the new one
		lpValue = lpValues;
		lpNewValue = lpNewValues;
		for (; '\0' != *lpValue; lpValue += _tcslen(lpValue) + 2)
		{
			bFound = false;
			if (NULL != _tcsstr(lpValue, TEXT("\\procdetect.sys")))
			{
				bFound = true;
			}

			if (bFound == false)
			{
				// Copy the current value to the target buffer
				memcpy(lpNewValue, lpValue, (_tcslen(lpValue) + 1) * sizeof(TCHAR));

				// This is not the value we want, continue to the next one
				lpNewValue += _tcslen(lpValue) + 2;
				cbNewValues += _tcslen(lpValue) + 2;
			}
		}
	//	*lpNewValue = *lpValue;
		
		cbNewValues++;

		// SHOW THE NEW VALUES
		//
		lpNewValue = lpNewValues;
		for (; '\0' != *lpNewValue; lpNewValue += _tcslen(lpNewValue) + 2)
		{
			// Show one value
			MessageAndLogging(hInstall, TRUE, lpNewValue);
		}
		MessageAndLogging(hInstall, TRUE, TEXT("###### end ######"));

		// WRITE THE NEW VALUES BACK TO THE KEY
		//
		MessageAndLogging(hInstall, TRUE, TEXT("RegSetValueEx"));
		lResult = RegSetValueEx(
			hKey,
			DELAYDELETE_KEY,
			NULL,
			REG_MULTI_SZ,
			(LPBYTE)lpNewValues,
			cbNewValues * sizeof(WCHAR)
		);
		if (ERROR_SUCCESS != lResult)
		{
			MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: can't allocate memory"));
			return ERROR_SUCCESS;
		}
	}
	__finally
	{
		// Clean up    
		//
		if (NULL != lpValues) { free(lpValues); }
		if (NULL != lpNewValues) { free(lpNewValues); }
		if (NULL != hKey) { RegCloseKey(hKey); }
	}

	MessageAndLogging(hInstall, TRUE, TEXT("******** NXPCLOG: Reset Delay Delete done."));

	return ERROR_SUCCESS;
}
