#include "stdafx.h"
#include <Windows.h>

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"

CELog  g_log;    

#include "evaluate.h"
#include "SCEServer.h" 
#include "PrtSrnKey.h"

#define CUR_DESKTIOP_REG_DIR	L"Control Panel\\Desktop"
#define LL_HOOKS_TIMEOUT_KEY	L"LowLevelHooksTimeout"	
#define MAX_TIMEOUT				5000

namespace
{
	const int RegisterPrtScnTimerID = 1;
	const int RegisterPrtScnTimerElapse = 1000;
}

BOOL WINAPI PrtScnKeyConfigValue()
{
	WCHAR szConfigPath[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, szConfigPath, MAX_PATH);

	WCHAR* pLastBackslash = wcsrchr(szConfigPath, L'\\');
	if (pLastBackslash != NULL)
	{
		*pLastBackslash = L'\0';
		pLastBackslash = wcsrchr(szConfigPath, L'\\');
		if (pLastBackslash != NULL)
		{
			*pLastBackslash = L'\0';
			pLastBackslash = wcsrchr(szConfigPath, L'\\');
			if (pLastBackslash != NULL)
			{
				*pLastBackslash = L'\0';
			}
		}
	}

	wcscat_s(szConfigPath, MAX_PATH, L"\\Policy Controller\\service\\injection.ini");

	return GetPrivateProfileIntW(L"info", L"EnablePrtScnKey", 0, szConfigPath);
};
BOOL WINAPI CheckAndCreateLLKPTimeLimit()
{
	HKEY hKey  ;
	//	try to open it first
	LONG rstatus = RegOpenKeyExW(HKEY_CURRENT_USER,CUR_DESKTIOP_REG_DIR,0,KEY_CREATE_SUB_KEY|KEY_SET_VALUE | KEY_QUERY_VALUE,&hKey);
	if (rstatus != ERROR_SUCCESS)
	{
		g_log.Log(CELOG_DEBUG, L"Open registry key failure!" ) ;
		return FALSE;
	}
	DWORD dValue = 0 ;
	DWORD dSize = sizeof(DWORD) ;
	rstatus = RegQueryValueExW(hKey,LL_HOOKS_TIMEOUT_KEY,NULL,NULL,(LPBYTE)&dValue,&dSize);
	if( rstatus == ERROR_SUCCESS )
	{
		if( dValue >= MAX_TIMEOUT )
		{
			g_log.Log(CELOG_DEBUG, L"Query registry key has been exist!" ) ;
			RegCloseKey(hKey);
			return TRUE;
		}
	}
	dValue = MAX_TIMEOUT ;
	rstatus = RegSetValueExW(hKey,LL_HOOKS_TIMEOUT_KEY,0,REG_DWORD,(const BYTE*)&dValue,dSize);
	if( rstatus != ERROR_SUCCESS )
	{
		g_log.Log(CELOG_DEBUG, L"Set key failure!!" ) ;
		RegCloseKey(hKey);
		return FALSE;
	}
	RegCloseKey(hKey);
	return TRUE ;
}

void ServerThread()
{
	DWORD SessionID = 0;

	ProcessIdToSessionId(GetCurrentProcessId(), &SessionID);

	USHORT PortNum = SCE::SCEServerBasedPort + static_cast<USHORT>(SessionID);

	while(true)
	{
		try
		{
			boost::shared_ptr<SCE::SCEServer> ThisInstance = SCE::SCEServer::Create(PortNum);

			ThisInstance->Run();
		}
		catch (...)
		{
			g_log.Log(CELOG_DEBUG, "SCE server terminate unexpectedly\n");

			if (!SCE::SCEServer::Release())
			{
				g_log.Log(CELOG_DEBUG, L"Failed to release SCE server\n");
			}
		}
	}
}

void RegisterPrtScn()
{
	RegisterHotKey(NULL, 1, 0, VK_SNAPSHOT);
	RegisterHotKey(NULL, 2, MOD_ALT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 3, MOD_CONTROL, VK_SNAPSHOT);
	RegisterHotKey(NULL, 4, MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 5, MOD_ALT | MOD_CONTROL, VK_SNAPSHOT);
	RegisterHotKey(NULL, 6, MOD_ALT | MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 7, MOD_CONTROL | MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 8, MOD_ALT | MOD_CONTROL | MOD_SHIFT, VK_SNAPSHOT);

	RegisterHotKey(NULL, 9,  MOD_WIN, VK_SNAPSHOT);
	RegisterHotKey(NULL, 10, MOD_WIN | MOD_ALT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 11, MOD_WIN | MOD_CONTROL, VK_SNAPSHOT);
	RegisterHotKey(NULL, 12, MOD_WIN | MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 13, MOD_WIN | MOD_ALT | MOD_CONTROL, VK_SNAPSHOT);
	RegisterHotKey(NULL, 14, MOD_WIN | MOD_ALT | MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 15, MOD_WIN | MOD_CONTROL | MOD_SHIFT, VK_SNAPSHOT);
	RegisterHotKey(NULL, 16, MOD_WIN | MOD_ALT | MOD_CONTROL | MOD_SHIFT, VK_SNAPSHOT);
}

void InitLog()
{
	//output to DebugView
	g_log.SetPolicy(new CELogPolicy_WinDbg()); 

	//enable log
	g_log.Enable();                              

	//log threshold to debug level
	g_log.SetLevel(CELOG_DEBUG);      
}

int MessageLoop()
{
	MSG msg = { 0 };

	while(GetMessage(&msg, NULL, 0, 0))
	{
		switch (msg.message)
		{
		case WM_HOTKEY:
			{		
				switch (msg.wParam)
				{
				case 2:
				case 5:
				case 6:
				case 8:
				case 10:
				case 13:
				case 14:
				case 16:
					{
						SCE::CPrtSrnKey PrtScnKey(FALSE);
						PrtScnKey.Handle();
					}
					
					break;

				default:
					{
						SCE::CPrtSrnKey PrtScnKey;
						PrtScnKey.Handle();
					}
					
					break;
				}
			}
			break;

		case WM_TIMER:
			RegisterPrtScn();
			break;

		default:
			break;
		}
	}

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	InitLog();

	boost::thread t1(ServerThread);

	CheckAndCreateLLKPTimeLimit() ;

	if (PrtScnKeyConfigValue())
	{
		RegisterPrtScn();
		SetTimer(NULL, RegisterPrtScnTimerID, RegisterPrtScnTimerElapse, NULL);
	}

	return MessageLoop();
}