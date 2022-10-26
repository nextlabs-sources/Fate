// NLEDPManager.cpp : Implementation of CNLEDPManager

#include "stdafx.h"
#include "NLEDPManager.h"
#include "nlconfig.hpp"
#include <string>
#include <fstream>
#include "utilities.h"
#include "celog.h"
CELog g_log;


#define PC_SERVICE_NAME L"ComplianceEnforcerService"
#define BUFFER_SIZE 2048

// CNLEDPManager
using namespace std;

void Initialize()
{
	static BOOL bInit = FALSE;
	if(!bInit)
	{
		if(edp_manager::CCommonUtilities::InitLog(g_log, EDPM_MODULE_ENHANCE))
		{
			g_log.Log(CELOG_DEBUG, L"Initlog in enhancement succeeded\n");
		}
		else
		{
			g_log.Log(CELOG_DEBUG, L"Initlog failed\n");
		}

		bInit = TRUE;
	}
}

STDMETHODIMP CNLEDPManager::SetDebugMode(SHORT bEnable, LONG* lRet)
{
	// TODO: Add your implementation code here
	Initialize();

	if(!lRet)
	{
		return E_POINTER;
	}

	//	switch on debug mode in registry, this is under local machine, nextlabs\\debugmode -- DWORD
	HKEY hNextlabs = NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"Software\\Nextlabs",0,KEY_SET_VALUE,&hNextlabs);
	if( rstatus != ERROR_SUCCESS )
	{
		*lRet = rstatus;
		g_log.Log(CELOG_DEBUG, L"RegOpenKeyExW in enhancement SetDebugMode failed, %d\n", rstatus);
		return E_ACCESSDENIED;
	}
	//	opened, set debugmode value
	DWORD dwDebugmode = bEnable ? 1 : 0;
	rstatus = RegSetValueEx(
						hNextlabs,
						L"debugmode",
						0,
						REG_DWORD,
						(BYTE*)&dwDebugmode,
						sizeof(dwDebugmode));
	if (rstatus)
	{
		//	error
		RegCloseKey(hNextlabs);

		*lRet = rstatus;
		g_log.Log(CELOG_DEBUG, L"RegSetValueEx in enhancement SetDebugMode failed, %d\n", rstatus);
		return E_ACCESSDENIED;
	}

	//	set Ok, close reg key handle and return true
	RegCloseKey(hNextlabs);

#ifdef _WIN64
	rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"Software\\Wow6432Node\\Nextlabs",0,KEY_SET_VALUE,&hNextlabs);
	if( rstatus != ERROR_SUCCESS )
	{
		*lRet = rstatus;
		g_log.Log(CELOG_DEBUG, L"RegOpenKeyExW in enhancement SetDebugMode failed, %d\n", rstatus);
		return E_ACCESSDENIED;
	}
	//	opened, set debugmode value
	dwDebugmode = bEnable ? 1 : 0;
	rstatus = RegSetValueEx(
		hNextlabs,
		L"debugmode",
		0,
		REG_DWORD,
		(BYTE*)&dwDebugmode,
		sizeof(dwDebugmode));
	if (rstatus)
	{
		//	error
		RegCloseKey(hNextlabs);

		*lRet = rstatus;
		g_log.Log(CELOG_DEBUG, L"RegSetValueEx in enhancement SetDebugMode failed, %d\n", rstatus);
		return E_ACCESSDENIED;
	}

	//	set Ok, close reg key handle and return true
	RegCloseKey(hNextlabs);
#endif 
	*lRet = 0;
	
	g_log.Log(CELOG_DEBUG, L"CNLEDPManager::SetDebugMode succeed\n");

	return S_OK;
}

STDMETHODIMP CNLEDPManager::StartPCService(LONG* lRet)
{
	// TODO: Add your implementation code here
	Initialize();
	// Get a handle to the SCM database. 
	SC_HANDLE schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_CONNECT);  // full access rights 

	if (NULL == schSCManager) 
	{
		*lRet = (LONG)GetLastError();
		g_log.Log(CELOG_DEBUG, L"OpenSCManager failed error code %d\n", *lRet);
		return E_ACCESSDENIED;
	}

	// Get a handle to the service.
	SC_HANDLE schService = OpenService( 
		schSCManager,         // SCM database 
		PC_SERVICE_NAME,            // name of service 
		SERVICE_START);  // full access 

	if (schService == NULL)
	{ 
		*lRet = (LONG)GetLastError();
		g_log.Log(CELOG_DEBUG, L"OpenService failed error code %d\n", *lRet);
		CloseServiceHandle(schSCManager);
		return E_ACCESSDENIED;
	}    

	// Attempt to start the service.
	if (!StartService(
					schService,  // handle to service 
					0,           // number of arguments 
					NULL) )      // no arguments 
	{
		*lRet = (LONG)GetLastError();

		

		//We found a problem, sometime, the API "StopPC" returns, but the "service" is still running.
		//So, we need to check the Error, try to start PC again if the error is ERROR_SERVICE_ALREADY_RUNNING
		//Try about 50 seconds.
		if(*lRet == ERROR_SERVICE_ALREADY_RUNNING)
		{

			g_log.Log(CELOG_DEBUG, L"StartService failed error code is \"already running\" \n");

			for( int i = 0; i < 100; i++)
			{
				if(StartService(
								schService,  // handle to service 
								0,           // number of arguments 
								NULL)
								)      // no arguments 
				{
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		
					*lRet = 0;

					g_log.Log(CELOG_DEBUG, L"StartService succeed\n");
					return S_OK;
	}
	else
	{
					if(GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
					{
						g_log.Log(CELOG_DEBUG, L"StartService failed error code is %d\n", *lRet);
						break;
					}
				}
				Sleep(500);	
			}
		}
		else
		{
			g_log.Log(CELOG_DEBUG, L"StartService failed error code is %d\n", *lRet);
		}

		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		
		return E_ACCESSDENIED; 
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	*lRet = 0;
	g_log.Log(CELOG_DEBUG, L"StartService succeed\n");
	return S_OK;
}

STDMETHODIMP CNLEDPManager::EnableAgentLog(SHORT bEnable, LONG* lRet)
{
	// TODO: Add your implementation code here
	Initialize();
	//	get logging.properties path first.
	//	get pc folder root path.
	wchar_t szPCDir[1024] = {0};
	if (!NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Policy Controller", szPCDir, 1024))
	{
		*lRet = (LONG)GetLastError();
		g_log.Log(CELOG_DEBUG, L"CNLEDPManager::EnableAgentLog GetComponentInstallPath failed %d\n", *lRet);
		return E_FAIL;
	}
	//	get logging.properties full file path
	wstring szPCLogProperties = (wstring)szPCDir + (wstring)L"config\\logging.properties";

	g_log.Log(CELOG_DEBUG, L"pc log properties file full path %s\n", szPCLogProperties.c_str());

	//	get temp logging.properties path
// 	wchar_t szTempPath[MAX_PATH] = {0};
// 	GetTempPath(MAX_PATH, szTempPath);
	wstring szTmpLogProperties = (wstring)szPCDir + (wstring)L"config\\logging_tmp.properties";

	//	open logging.properties as an input stream
	wifstream file(szPCLogProperties.c_str());

	//	open temp as an output stream
	wofstream temp_file(szTmpLogProperties.c_str());


	//	check if input stream is ok
	if(!file || !temp_file)
	{
		*lRet = (LONG)GetLastError();
		g_log.Log(CELOG_DEBUG, L"open pc log properties file or temp properties file failed, file handle [%d], temp file handle [%d]\n", file, temp_file);
		return E_FAIL ;
	}
	if( file.fail() || temp_file.fail())
	{
		*lRet = (LONG)GetLastError();
		return E_FAIL ;
	}

	//	try to read every line from input stream
	wstring strBuf;

	if (!file.eof())
	{
		//	read the first line first
		getline(  file, strBuf )  ;
	
		//	write it to temp file
		temp_file << strBuf;
	}
	
	//	read and write every line
	while( !file.eof())
	{
		//	as the file is not end, we need to write a newline
		temp_file << "\n";

		//	we are trying to read every line from input stream, and copy the line to output stream
		getline(  file, strBuf )  ;

		if( wcsstr(strBuf.c_str(), L"com.bluejungle.level = ") )
		{
			//	we find the line "com.bluejungle.level = "
			//	we need to modify it to "FINEST"/"SEVERE", so we modify content of this line, write the new data into output stream
			if (bEnable)
			{
				temp_file << L"com.bluejungle.level = FINEST";
			}
			else
			{
				temp_file << L"com.bluejungle.level = SEVERE";
			}
		}
		else if ( wcsstr(strBuf.c_str(), L"java.util.logging.FileHandler.limit = ") )
		{
			//	we find the line "java.util.logging.FileHandler.limit = "
			//	we need to modify it to "10000000"/"500000", so we modify content of this line, write the new data into output stream
			if (bEnable)
			{
				temp_file << L"java.util.logging.FileHandler.limit = 10000000";
			}
			else
			{
				temp_file << L"java.util.logging.FileHandler.limit = 500000";
			}
		}
		else
		{
			//	we don't modify this line, we only modify the line contain "com.bluejungle.level = " or
			//	contain "java.util.logging.FileHandler.limit = "
			temp_file << strBuf;
		}
	}
	//	loop finish, all data are copied to temp file
	file.close() ;
	temp_file.close();

	//	delete original logging.properties, and rename logging_temp.properites to logging.properties
	DeleteFile(szPCLogProperties.c_str());

	if (!MoveFileW(szTmpLogProperties.c_str(), szPCLogProperties.c_str()))
	{
		//	rename failed, return error
		*lRet = (LONG)GetLastError();
		g_log.Log(CELOG_DEBUG, L"rename pc log properties file from temp file failed\n");
		return E_FAIL;
	}

	//	ok, we finish agent log switch
	*lRet = 0;

	g_log.Log(CELOG_DEBUG, L"CNLEDPManager::EnableAgentLog succeed\n");

	return S_OK;
}

STDMETHODIMP CNLEDPManager::Decrypt(BSTR strPwd)
{
	Initialize();

	wstring strPassword = strPwd;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	si.wShowWindow = SW_HIDE;
	si.dwFlags =  STARTF_USESHOWWINDOW;

	wstring cmd;
	edp_manager::CCommonUtilities::GetPCInstallPath(cmd);
	cmd += wstring(L"bin\\decrypt.exe") + wstring(L" -p ") + strPassword;

	g_log.Log(CELOG_DEBUG, L"command is %s in DecryptViaTool\n", cmd.c_str());

	// Start the child process. 
	if( !CreateProcess( NULL,   // No module name (use command line)
		(LPWSTR)cmd.c_str(),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
		) 
	{
		g_log.Log(CELOG_DEBUG, L"CreateProcess failed (%d) in DecryptViaTool\n", GetLastError() );
		return S_FALSE;
	}

	// Wait until child process exits.
	g_log.Log(CELOG_DEBUG, L"before WaitForSingleObject in DecryptViaTool\n");
	WaitForSingleObject( pi.hProcess, INFINITE );
	g_log.Log(CELOG_DEBUG, L"after WaitForSingleObject in DecryptViaTool\n");

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	//return TRUE;



	return S_OK;
}
