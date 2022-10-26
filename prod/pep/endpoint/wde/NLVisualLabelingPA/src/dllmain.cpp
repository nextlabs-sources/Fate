// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "celog.h"
#include "nlconfig.hpp"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"

HMODULE	    g_hModule = NULL;
CELog NLVisualLabelingPALog;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			NLVisualLabelingPALog.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
			NLVisualLabelingPALog.Enable();                              // enable log
			NLVisualLabelingPALog.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
			
			g_hModule = hModule;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
};
