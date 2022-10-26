// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"



#include "utilities.h"
CELog g_log;


BOOL APIENTRY DllMain( HMODULE /*hModule*/,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			if (edp_manager::CCommonUtilities::InitLog(g_log, EDPM_MODULE_UTILITIES))
			{
				g_log.Log(CELOG_DEBUG, L"init log in EDPM utilities succeed\n");
			}
			
			CoInitialize(0);
				
		}
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	case DLL_PROCESS_DETACH:
		{
			CoUninitialize();
		}
		break;
	}
	return TRUE;
}

