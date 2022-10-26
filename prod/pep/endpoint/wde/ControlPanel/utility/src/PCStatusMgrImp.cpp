#include "StdAfx.h"
#include "PCStatusMgrImp.h"
#include "cesdk.h"
#include "CEPrivate.h"
#include "SysUtils.h"
#include <shellapi.h>
#include "private_loader.h"
#include "localinfo.h"
#include "policy_controller.hpp"
#include "nlconfig.hpp"

#define PC_SERVICE_NAME L"ComplianceEnforcerService"

CPCStatusMgrImp::CPCStatusMgrImp(void)
{
	m_ceHandle = NULL;
	m_psdkLoader = NULL;
	m_pPrivateLoader = NULL;
}

CPCStatusMgrImp::~CPCStatusMgrImp(void)
{
}

/*

implementation:

we check if pc is running before stop pc, if pc is already stopped, 
we return directly. 

otherwise, we stop pc and if we stop succeed, we disconnect pc, otherwise, we keep pc
connected, pc connected equal to pc is still running.

*/
_CEResult_t CPCStatusMgrImp::StopPC(wchar_t* pszPwd)
{
	if(!pszPwd)
	{
		return CE_RESULT_INVALID_PARAMS;
	}

	if (!IsPCRunning())
	{
		//	pc already stopped, return OK
		return CE_RESULT_SUCCESS;
	}

	DWORD dwStart = GetTickCount();
	if(!ConnectPC())
	{
		g_log.Log(CELOG_DEBUG, L"Connect PC failed\n");
		return CE_RESULT_CONN_FAILED;
	}

	g_log.Log(CELOG_DEBUG, L"ConnectPC takes %d ms\n", GetTickCount() - dwStart);

	CEString pwd = m_psdkLoader->fns.CEM_AllocateString(pszPwd);

	//	stopping pc	.....................................................
	dwStart = GetTickCount();
	CEResult_t res = m_pPrivateLoader->f_CEP_StopPDP_t(m_ceHandle, 
		pwd,
		30000);

	g_log.Log(CELOG_DEBUG, L"StopPC takes %d ms\n", GetTickCount() - dwStart);

	m_psdkLoader->fns.CEM_FreeString(pwd);

		//	pc is stopped, 
		//	disconnect to pc
		DisconnectPC();

	if(res == CE_RESULT_SUCCESS)
	{
	//	waiting for pc is not running.................................
	//	we must assure that pc is not running before we return.
	//	as stop pc can not be stopped immediately.
	//	we only wait for 30 seconds, we don't want to hang forever if pc has bug
	DWORD dwCounts = 0;
	while (dwCounts < 150)
	{
		if (!nextlabs::policy_controller::is_up())
		{
			//	yes, pc is stopped
			//	we can return
			Sleep(500);

			g_log.Log(CELOG_DEBUG, L"pc has been stopped.....\n");

			break;
		}
		
		//	pc is still running, sleep and try later.
		Sleep(200);

		dwCounts++;

		g_log.Log(CELOG_DEBUG, L"wait for pc stopping.....\n");
	}
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"pc can't be stopped, error code %d.....\n", res);
	}

	return res;
}

BOOL CPCStatusMgrImp::ConnectPC()
{
	g_log.Log(CELOG_DEBUG, L"try to ConnectPC.....\n");

	if (m_ceHandle)
	{
		//	we already connected to pc.
		return TRUE;
	}

	if (!m_psdkLoader)
	{
		g_log.Log(CELOG_DEBUG, L"new cesdk_loader\n");
		m_psdkLoader = new cesdk_loader;
	}
	if (!m_pPrivateLoader)
	{
		g_log.Log(CELOG_DEBUG, L"new CPrivateLoader\n");
		m_pPrivateLoader = new CPrivateLoader;
	}

	wstring installPath;
	if (!m_psdkLoader->is_loaded())
	{
		installPath = GetCommonComponentsDir();

		//	load sdk before connect to pc 
		BOOL ret = m_psdkLoader->load(installPath.c_str());
		if (!ret)
		{
			//	load failed
			g_log.Log(CELOG_DEBUG, L"bin path of EDP Manager: %s, load sdk dlls failed\n", installPath.c_str());
			return FALSE;
		}

		g_log.Log(CELOG_DEBUG, L"m_psdkLoader->load succeed, %s\n", installPath.c_str());
	}
	
	if (!m_pPrivateLoader->IsLoaded())
	{
		//	load private 
		BOOL ret = m_pPrivateLoader->Load(installPath.c_str());
		if (!ret)
		{
			//	load private interface failed
			g_log.Log(CELOG_DEBUG, L"load private sdk failed, %s\n", installPath.c_str());
			return FALSE;
		}

		g_log.Log(CELOG_DEBUG, L"m_pPrivateLoader->load succeed\n");
	}

	//	connect to pc
	CLocalInfo& localInfo = CLocalInfo::GetInstance();
	wstring userName;
	wstring userSID;
	localInfo.GetUserInfo(userSID, userName);

	wstring appName;
	wstring appPath;
	localInfo.GetAppInfo(appName, appPath);

	CEUser user;
	memset(&user, 0, sizeof(user));
	user.userID = m_psdkLoader->fns.CEM_AllocateString(userSID.c_str());
	user.userName = m_psdkLoader->fns.CEM_AllocateString(userName.c_str());


	CEApplication app;
	memset(&app, 0, sizeof(app));
	app.appName = m_psdkLoader->fns.CEM_AllocateString(appName.c_str());
	app.appPath = m_psdkLoader->fns.CEM_AllocateString(appPath.c_str());

	CEResult_t res = m_psdkLoader->fns.CECONN_Initialize(app, user, NULL, &m_ceHandle, 10000);

	BOOL bSucceed = TRUE;

	if (res != CE_RESULT_SUCCESS || !m_ceHandle)
	{
		//	connect failed,
		//	check if we need to try to connect again
		DWORD dwStartTick = ::GetTickCount();
		DWORD dwTimeout = 30000;	//	we try 30 seconds for max.


		//	check first
		while (res == CE_RESULT_CONN_FAILED)
		{
			//	sleep first before retry
			Sleep(500);
		
			//	yes, we need to try again
			if (::GetTickCount() - dwStartTick > dwTimeout)
			{
				//	timeout 
				g_log.Log(CELOG_DEBUG, L"sdk init failed and timeout for 30 seconds res %d\n", res);
		m_ceHandle = NULL;
				bSucceed = FALSE;
				goto FUN_EXIT;
			}

			//	connect pc again
			res = m_psdkLoader->fns.CECONN_Initialize(app, user, NULL, &m_ceHandle, 10000);
			
			g_log.Log(CELOG_DEBUG, L"sdk init res %d, cehandle %d\n", res, m_ceHandle);
		}


		bSucceed = (res == CE_RESULT_SUCCESS) ? TRUE : FALSE;
		m_ceHandle = (bSucceed == TRUE) ? m_ceHandle : NULL;
		goto FUN_EXIT;
	}

FUN_EXIT:
	m_psdkLoader->fns.CEM_FreeString(user.userID);
	m_psdkLoader->fns.CEM_FreeString(user.userName);
	m_psdkLoader->fns.CEM_FreeString(app.appName);
	m_psdkLoader->fns.CEM_FreeString(app.appPath);


	return bSucceed;
}

BOOL CPCStatusMgrImp::DisconnectPC()
{
	if (!m_ceHandle)
	{
		//	pc have not been connected.
		return TRUE;
	}

	//	disconnect to pc
	m_psdkLoader->fns.CECONN_Close(m_ceHandle, 30000);
	m_ceHandle = NULL;

	//	unload sdk
#if 0
	//	comment by benjamin, 
	//	don't know why if we unload sdk here, edp manager will crash strangely. keep the code, fix me if you can.
	m_psdkLoader->unload();

	m_pPrivateLoader->Unload();
#endif 

	return TRUE;
}

/*

implementation:

we check if pc is running before start pc, if pc is already running, 
we return directly. 

otherwise, we start pc.

*/
BOOL CPCStatusMgrImp::StartPC()
{
	if (IsPCRunning())
	{
		//	pc already started, return OK
		g_log.Log(CELOG_DEBUG, L"user request start pc, but pc is already running, return success\n");
		return TRUE;
	}

	//	start service.
	//	ShellExecute(NULL, NULL, L"sc.exe", L"start ComplianceEnforcerService", NULL, SW_HIDE);

	//	we use server manage API to start service
	if (!StartPCService())
	{
		//	start service failed
		g_log.Log(CELOG_DEBUG, L"start pc service failed, return false\n");
		return FALSE;
	}

	//	waiting for pc is running.................................
	//	we must assure that pc is running before we return.
	//	as start pc can not be started immediately.
	//	we only wait for 60 seconds, we don't want to hang forever if pc has bug
	DWORD dwCounts = 0;
	while (dwCounts < 300)
	{
		if (nextlabs::policy_controller::is_up())
		{
			//	yes, pc is started
			//	we can return
			g_log.Log(CELOG_DEBUG, L"pc has started\n");
			break;
		}

		//	pc is still running, sleep and try later.
		Sleep(200);

		dwCounts++;

		g_log.Log(CELOG_DEBUG, L"wait for pc staring.....\n");
	}

	return (dwCounts == 300) ? FALSE : TRUE;
}


/*
	
	implementation:

	we determine whether pc is running by -- connect to it -- call sdk initialize.
	if sdk initialize return success, we think it is running, otherwise, we think it is not running.

	another case is, we already connected to pc, then, we think pc is running -- because if pc is not running, 
	we can not connect to pc.

	you may say -- how about user stop pc after we connect to pc?	--	in this case, we assume user can stop pc only through edp manager,
	so, we will know if user stop pc, when pc is stopped, we will set status to disconnected.
	
	*/
BOOL CPCStatusMgrImp::IsPCRunning()
{
	BOOL bUp = nextlabs::policy_controller::is_up();

	return bUp;
}

BOOL CPCStatusMgrImp::StartPCService()
{
#if 1
	return UAC_StartPC(NULL) == 0? TRUE: FALSE;
#else
	SERVICE_STATUS_PROCESS ssStatus; 
	DWORD dwOldCheckPoint; 
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	g_log.Log(CELOG_DEBUG, L"enter StartPCService\n");

	// Get a handle to the SCM database. 
	SC_HANDLE schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_CONNECT);  // full access rights 

	if (NULL == schSCManager) 
	{
		g_log.Log(CELOG_DEBUG, L"OpenSCManager failed (%d)\n", GetLastError());
		return FALSE;
	}

	// Get a handle to the service.
	SC_HANDLE schService = OpenService( 
		schSCManager,         // SCM database 
		PC_SERVICE_NAME,            // name of service 
		SERVICE_START);  // full access 

	if (schService == NULL)
	{ 
		g_log.Log(CELOG_DEBUG, L"OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return FALSE;
	}    

	// Attempt to start the service.
	if (!StartService(
		schService,  // handle to service 
		0,           // number of arguments 
		NULL) )      // no arguments 
	{
		g_log.Log(CELOG_DEBUG, L"StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		return FALSE; 
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"Service start pending...\n");
	}


	if (!QueryServiceStatusEx( 
		schService,             // handle to service 
		SC_STATUS_PROCESS_INFO, // info level
		(LPBYTE) &ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded ) )              // if buffer too small
	{
		g_log.Log(CELOG_DEBUG, L"QueryServiceStatusEx first time failed, error code %d\n", GetLastError());

		//	fix me, this can cause a bug.
		//	check msdn to find out what value should we return here
		return TRUE; 
	}

	// Save the tick count and initial checkpoint.
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
	{ 
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if( dwWaitTime < 1000 )
			dwWaitTime = 1000;
		else if ( dwWaitTime > 10000 )
			dwWaitTime = 10000;

		Sleep( dwWaitTime );

		// Check the status again. 
		if (!QueryServiceStatusEx( 
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE) &ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // if buffer too small
		{
			g_log.Log(CELOG_DEBUG, L"QueryServiceStatusEx failed in loop, so we break loop...\n");
			break; 
		}

		if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
		{
			// The service is making progress.
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				g_log.Log(CELOG_DEBUG, L"no progress made within the wait hint, so we break loop...\n");
				break;
			}
		}
	} 

	// Determine whether the service is running
	BOOL ret = FALSE;
	if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
	{
		ret = TRUE;
		printf("Service started successfully.\n"); 
	}
	else 
	{ 
		ret = FALSE;
		g_log.Log(CELOG_DEBUG, L"Service not started. \n");
		g_log.Log(CELOG_DEBUG, L"Current State: %d\n", ssStatus.dwCurrentState); 
		g_log.Log(CELOG_DEBUG, L"Exit Code: %d\n", ssStatus.dwWin32ExitCode); 
		g_log.Log(CELOG_DEBUG, L"Check Point: %d\n", ssStatus.dwCheckPoint); 
		g_log.Log(CELOG_DEBUG, L"Wait Hint: %d\n", ssStatus.dwWaitHint); 
	} 

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	return ret;
#endif
}

void CPCStatusMgrImp::ResetUAC()
{
	UAC_Reset();
}
