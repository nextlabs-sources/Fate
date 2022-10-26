// attrmgr.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <list>
#include "Office2k7_attrs.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

wchar_t g_szCurrentDir[MAX_PATH + 1] = {0};
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /* lpReserved */
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			GetModuleFileNameW(hModule, g_szCurrentDir, MAX_PATH);
			wchar_t* p = wcsrchr(g_szCurrentDir, L'\\');
			if(p)
				*p = L'\0';
			LoadOffice2k7Dll();
			break;
		}

	case DLL_PROCESS_DETACH:
		{
			FreeOffice2k7Dll();
			break;
		}
	}

    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

