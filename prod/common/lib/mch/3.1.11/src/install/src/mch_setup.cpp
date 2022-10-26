
#pragma warning(disable: 4819)
#include <windows.h>
#include <stdio.h>
#include "madCHook_helper.h"

#define INJECT_DRIVER_NAME_W        L"NLInjection"
#define INJECT_DRIVER_DESC          L"NextLabs code injection driver."
#define INJECT_DRIVER_FILENAME64    L"nlinjection64.sys"
#define INJECT_DRIVER_FILENAME32    L"nlinjection32.sys"

static WCHAR g_drvpath64[MAX_PATH+1] = {0};
static WCHAR g_drvpath32[MAX_PATH+1] = {0};


BOOL WINAPI DllMain(
					_In_  HINSTANCE hDll,
					_In_  DWORD dwReason,
					_In_  LPVOID lpvReserved
					)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		::DisableThreadLibraryCalls((HMODULE)hDll);
		swprintf_s(g_drvpath64, MAX_PATH, L"c:\\windows\\system32\\drivers\\%s", INJECT_DRIVER_FILENAME64);
		swprintf_s(g_drvpath32, MAX_PATH, L"c:\\windows\\system32\\drivers\\%s", INJECT_DRIVER_FILENAME32);
		InitializeMadCHook();
		break;
	case DLL_PROCESS_DETACH:
		FinalizeMadCHook();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	default:
		break;
	}
	
	return TRUE;
}

extern "C"
int nextlabs_install(void)
{
  BOOL bRet;
  bRet = InstallInjectionDriver(INJECT_DRIVER_NAME_W, g_drvpath32, g_drvpath64, INJECT_DRIVER_DESC);
  if(!bRet)
  {
    return -1;
  }
  return 0;
}/* nextlabs_install */

extern "C"
int nextlabs_uninstall(void)
{
  BOOL bRet;
  bRet = UninstallInjectionDriver(INJECT_DRIVER_NAME_W);
  if(!bRet)
  {
    return -1;
  }
  return 0;
}/* nextlabs_uninstall */