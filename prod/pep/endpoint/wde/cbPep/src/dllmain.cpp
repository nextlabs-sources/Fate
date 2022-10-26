// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "cbPep_i.h"
#include "dllmain.h"
#include "nxtmgr.h"

#include "celog.h"
#include "nlconfig.hpp"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"

CcbPepModule _AtlModule;
CELog cbPepLog;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;

	if (dwReason == DLL_PROCESS_DETACH)
	{
		CNxtMgr::Instance()->Uninit();
	}
	else if (dwReason == DLL_PROCESS_ATTACH)
	{
		cbPepLog.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
		cbPepLog.Enable();                              // enable log
		cbPepLog.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
	}

	return _AtlModule.DllMain(dwReason, lpReserved); 
}
