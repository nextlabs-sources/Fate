// pafUI.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "global.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	lpReserved;	//for warning C4100
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInst = hModule ;
		}
		break ;
	case DLL_THREAD_ATTACH:
		break ;
	case DLL_THREAD_DETACH:
		break ;
	case DLL_PROCESS_DETACH:
		break ;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

