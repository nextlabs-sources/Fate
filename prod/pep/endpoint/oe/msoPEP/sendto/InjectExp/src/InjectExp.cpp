// InjectExp.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Hook.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    UNREFERENCED_PARAMETER(lpReserved);
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hModule;
			SetHook();
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			UnsetHook();
		}
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

