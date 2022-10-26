#include <windows.h>
#include <commonutils.hpp>

#include "basepep.h"


nextlabs::CDllModule* gDllModule = NULL;

HMODULE g_hModule = NULL; 

/***********************************************************************
// DllMain
//
// Entry point
***********************************************************************/
BOOL APIENTRY DllMain(
	HMODULE hModule,  //A handle to the DLL module. The value is the base address of the DLL. 
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
    g_hModule = hModule;
	UNREFERENCED_PARAMETER(lpReserved);       
	if (DLL_PROCESS_ATTACH == ul_reason_for_call)
	{
		gDllModule = new nextlabs::CBasePep(hModule);
		return gDllModule->DispatchEvent(DLL_PROCESS_ATTACH);
	}
	else if (DLL_PROCESS_DETACH == ul_reason_for_call)
	{
		BOOL rt = FALSE;
		rt = gDllModule->DispatchEvent(DLL_PROCESS_DETACH);
		// release 
		delete gDllModule;
		gDllModule = NULL;
		return rt;
	}
	else
	{
		return gDllModule->DispatchEvent(ul_reason_for_call);
	}

}/* DllMain */

