// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "criticalMngr.h"

CcriticalMngr g_criticalMgr;
HMODULE g_hMod = NULL;

BOOL g_bIgnoredByPolicy = FALSE;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	lpReserved ;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hMod = hModule;
		g_bIgnoredByPolicy = IsIgnoredByPolicy();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			SetDetachFlag( TRUE ) ;
			HTTPE_Release();
			
		}
		break;
	}
	return TRUE;
}

