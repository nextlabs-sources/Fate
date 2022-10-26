// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "EDPMgrDlg.h"
#include "StopEnfDlg.h"
#include "EDPMgrUtilities.h"
#include "HelpDlg.h"

#include "ipcproxy.h"


HINSTANCE g_hInst;

//	main edp manager dialog
CEDLPMgrDlg* g_dlg = NULL;


//	stop enforcer dialog
CStopEnfDlg* g_StopEnfDlg = NULL;

//	celog
CELog g_log;


CHelpDlgModeless* g_help = NULL;

void StartPCThread(void* pArgument)
{
	pArgument;

	CoInitialize(0);

	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();

	HANDLE hPC = CreateEventW(NULL, FALSE, TRUE,  EVENT_STARTING_PC);

	BOOL bRet = edpUtilities.StartPC();
	edpUtilities.ResetUACInstance();

	if(hPC)
	{
		CloseHandle(hPC);
	}

	if (!bRet)
	{
		edpUtilities.ShowNoPermission_StartPC();
		//	start service failed
		g_log.Log(CELOG_DEBUG, L"start pc failed\n");
	}

	CoUninitialize();
}

/* Load the online help URL
*
* Errors loading the URL will lead to the help URL being null.
* See menu item behavior later in WndProc() function to determine
* how a null URL is handled
*/
#define MAX_FILENAME_LENGTH 32767+1  /* MAX Unicode path length */

static BOOL LoadOnlineHelpURL(wstring& strURL)
{
#if 1
	//	our new online url
	strURL = L"http://www.nextlabs.com/html/?q=www.nextlabs.com/support/training/tutorial/EDP_Enforcer";
	return TRUE;
#else
	//	our old implementation
	static wstring helpUrl;
	if (helpUrl.length())
	{
		//	we already get helpUrl, directly use it
		strURL = helpUrl;
		return TRUE;
	}

	//	we have no helpUrl yet, get it...............

	LONG result;
	HKEY hKey = NULL; 
	DWORD policyControllerDirSize = MAX_FILENAME_LENGTH;
	TCHAR* policyControllerDir = new TCHAR[MAX_FILENAME_LENGTH];

	result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
		"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
		0,KEY_QUERY_VALUE,&hKey);

	if( result != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"RegOpenKeyExA failed in LoadOnlineHelpURL, err code %d\n", result);
		return FALSE;
	}

	result = RegQueryValueEx(hKey,                   
		L"PolicyControllerDir", 
		NULL,                  
		NULL,                  
		(LPBYTE)policyControllerDir,       
		&policyControllerDirSize);         

	RegCloseKey(hKey);

	if( result == ERROR_SUCCESS )
	{
		result = _tcscat_s(policyControllerDir, MAX_FILENAME_LENGTH, L"\\help\\index.html");
		if( result == ERROR_SUCCESS )
		{
			g_log.Log(CELOG_DEBUG, L"LoadOnlineHelpURL succeed, dir %s\n", policyControllerDir);
			helpUrl = wstring(policyControllerDir);
		}
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"RegQueryValueEx failed in LoadOnlineHelpURL, err code %d\n", result);
	}

	if (policyControllerDir != NULL)
	{
		delete[] policyControllerDir;
		policyControllerDir = NULL;
	}

	strURL = helpUrl;
	return TRUE;  
#endif
}












BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInst = hModule;

			//	init celog
			if (edp_manager::CCommonUtilities::InitLog(g_log, EDPM_MODULE_DLG))
			{
				g_log.Log(CELOG_DEBUG, L"init log in EDPM dialog succeed\n");
			}
		}
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	case DLL_PROCESS_DETACH:
		{
			if(g_dlg)
			{
				delete g_dlg;
				g_dlg = NULL;
			}
			if (g_StopEnfDlg)
			{
				delete g_StopEnfDlg;
				g_StopEnfDlg = NULL;
			}
		}
		break;
	}
	return TRUE;
}

BOOL DoAction(DWORD dwIndex)
{
	int nIndex = (int)dwIndex;
	int nParam = 0;
	if(dwIndex > 1000)
	{
		nIndex = (int)HIWORD(dwIndex);
		nParam = (int)LOWORD(dwIndex);
	}

	switch(nIndex)
	{
	case 0:
	case 20:
		//	below is main edp manager dialog process.....................................
		{
			if(g_dlg && !g_dlg->m_hWnd)
			{//It means the dialog was closed by user. delete the dialog.
				delete g_dlg;
				g_dlg = NULL;
			}

			if(g_dlg == NULL)
			{
				g_dlg = new CEDLPMgrDlg();
				if(!g_dlg)
				{
					return FALSE;
				}
				g_dlg->Create(NULL);
			}
			if(dwIndex == 20 )
			{
				g_dlg->ShowTab(1);
			}
			else
			{
				g_dlg->ShowTab(0);
			}
			g_dlg->CenterWindow();
			g_dlg->ShowWindow(SW_SHOW);

		}
		break;
	case 1:
		//	below is start pc process........................................................
		{
			_beginthread(StartPCThread, 0, NULL);
		}
		break;
	case 2:
		//	below is stop pc process........................................................
		{
			CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
			
			//	check if pc is running
			if (!edpUtilities.IsPCRunning())
			{
				//	no, pc is not running, 
				//	do nothing
				g_log.Log(CELOG_DEBUG, L"edpm dialog stop pc interface, pc is not running\n");
				break;
			}

			g_log.Log(CELOG_DEBUG, L"Try to show StopPC dialog\n");

			//	pc is running, 
			//	we wizard user to stop pc...........................
			if (g_StopEnfDlg && !g_StopEnfDlg->m_hWnd)
			{
				delete g_StopEnfDlg;
				g_StopEnfDlg = NULL;
			}

			if(g_StopEnfDlg == NULL)
			{
				g_StopEnfDlg = new CStopEnfDlg();
				if(!g_StopEnfDlg)
				{
					return FALSE;
				}
				g_StopEnfDlg->Create(NULL);
			}

			g_StopEnfDlg->CenterWindow();
			g_StopEnfDlg->ShowWindow(SW_SHOW);

			//	done, we call API to register them
			HWND szWnds[1] = {0};
			szWnds[0] = g_StopEnfDlg->m_hWnd;
			edpUtilities.RegDlgHandleForKB(szWnds, 1);
		}
		break;
	case 3:
		//	below is url help link process........................................................
		{
			wstring strURL;
			if (!LoadOnlineHelpURL(strURL))
			{
				//	load URL failed, in this case, show help dialog
				g_log.Log(CELOG_DEBUG, L"load help URL failed, in this case, show help dialog\n");
				CHelpDlg dlg;
				dlg.DoModal();
			}
			else
			{
				//	load URL succeed, show it
				ShellExecute(NULL, L"open", strURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;
	case 4:
	        {
		  const WCHAR *LOGONEVENT = L"handleLogonEvent";
		  const int IPCPROXY_REQUEST_TIMEOUT = 5000;
		  const WCHAR *IPC_CM_REQUEST_HANDLER = L"com.bluejungle.destiny.agent.controlmanager.CMRequestHandler";

		  IPCProxy pIPCProxy;

		  g_log.Log(CELOG_DEBUG, L"Try to call IPCProxy to update policy\r\n");

		  if (pIPCProxy.Init(IPC_CM_REQUEST_HANDLER))
                  {
		    IPCREQUEST request, response;
		    memset (&request, 0, sizeof (IPCREQUEST));
		    request.ulSize = sizeof (IPCREQUEST);

		    wcsncpy_s (request.methodName, _countof(request.methodName), LOGONEVENT, _TRUNCATE);
		    wcsncpy_s (request.params[0], _countof(request.params[0]), L"Nobody\\ForcingHeartbeat", _TRUNCATE);

		    memset (&response, 0, sizeof (IPCREQUEST));
		    BOOL bRet = pIPCProxy.Invoke(&request,&response,IPCPROXY_REQUEST_TIMEOUT);

			g_log.Log(CELOG_DEBUG, L"The result of Invoke(), ret value: %d, IPC_CM_REQUEST_HANDLER: %s, IPCPROXY_REQUEST_TIMEOUT: %d,LOGONEVENT: %s, param 0: %s.\r\n", bRet, IPC_CM_REQUEST_HANDLER, IPCPROXY_REQUEST_TIMEOUT, LOGONEVENT, request.params[0]);
		  }
		  else
		  {
			g_log.Log(CELOG_DEBUG, L"Init() returns FALSE\r\n");
		  }
		  break;
		}
	case 5:
		//	show about dialog................................
		{
			if(g_help && !g_help->m_hWnd)
			{
				//It means the dialog was closed by user. delete the dialog.
				delete g_help;
				g_help = NULL;
			}

			if(g_help == NULL)
			{
				g_help = new CHelpDlgModeless();
				if(!g_help)
				{
					return FALSE;
				}
				g_help->Create(NULL);

				//	as we need these dialog and its child dialog can respond to keyboard input, 
				//	we need to register them............................
				HWND szWnds[1] = {0};
				szWnds[0] = g_help->m_hWnd;

				//	done, we call API to register them
				CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
				edpUtilities.RegDlgHandleForKB(szWnds, 1);
			}

			g_help->CenterWindow();
			g_help->ShowWindow(SW_SHOW);

			break;
		}
	case 1000://show notification
		{
			g_log.Log(CELOG_DEBUG, L"Show the correct notification since user clicks a bubble. id: %d\n", nParam);
			if(g_dlg && !g_dlg->m_hWnd)
			{//It means the dialog was closed by user. delete the dialog.
				delete g_dlg;
				g_dlg = NULL;
			}

			if(g_dlg == NULL)
			{
				g_dlg = new CEDLPMgrDlg();
				if(!g_dlg)
				{
					return FALSE;
				}
				g_dlg->Create(NULL);
			}
			
			g_dlg->ShowTab(2);
			g_dlg->ShowNotification(nParam);

			g_dlg->CenterWindow();
			g_dlg->ShowWindow(SW_SHOW);
		}
		break;
	default:
		//	unknown event....................
		{
			//	do nothing
		}
		break;
	}

	return TRUE;
}

/*

this is exported function tell user verbose logging status


*/
void __stdcall QueryStatus(char* pBuf, int nLen)
{
	HANDLE hLog = OpenEventW(READ_CONTROL, FALSE, EVENT_SETTING_LOG);
	if(hLog)
	{
		strncpy_s(pBuf, nLen, "Disabling", _TRUNCATE);
		CloseHandle(hLog);
		return;
	}

	BOOL bLog = FALSE;
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	edpUtilities.IsVerboseLogOn(bLog);

	if ( bLog )
	{
		//	this means we finish diagnostics
		strncpy_s(pBuf, nLen, "Enabled", _TRUNCATE);
	}
	else
	{
		strncpy_s(pBuf, nLen, "Disabled", _TRUNCATE);
	}
}


