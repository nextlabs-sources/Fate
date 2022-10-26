// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "OutlookAddin.h"
#include "dllmain.h"
#include "Hook.h"

#include "celog.h"
#include "nlconfig.hpp"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"

COutlookAddinModule _AtlModule;

CELog OutlookAddinLog;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	//DWORD lenBaseName=0;
	//hInstance;
	//g_hInstance = hInstance;

	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			OutlookAddinLog.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
			OutlookAddinLog.Enable();                              // enable log
			OutlookAddinLog.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
			
			OutlookAddinLog.Log(CELOG_DEBUG,L"outlook enter --------------------");
			//::OutputDebugStringW(L"outlook enter --------------------") ;
			StartStopHook(true); 
			break;
		}
	case DLL_PROCESS_DETACH:
		{	
			StartStopHook(false);
		}
		break;
	}

	return _AtlModule.DllMain(dwReason, lpReserved); 
}
