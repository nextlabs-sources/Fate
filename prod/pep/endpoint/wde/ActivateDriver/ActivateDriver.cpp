// ActivateDriver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable:4819)
#include "madCHook_helper.h"
#pragma warning(pop)

#define INJECT_DRIVER_NAME		L"NLInjection"
#define INJECT_DRIVER32			L"nlinjection32.sys"
#define INJECT_DRIVER64			L"nlinjection64.sys"
#define INJECT_DESCRIPTION		L"Nextlabs injection driver"

void help()
{
	printf("ActivateDriver help:\nActivateDriver /s | /d [/u]\n    /s Permanently install\n    /d Dynamically install\n    /u uninstall\n");
}
int _tmain(int argc, _TCHAR* argv[])
{
	if ( !(argc == 2 || argc == 3) )
	{
		help();
	}

	if(argc == 2)// Activate driver
	{
		if(_wcsicmp(argv[1], L"/s") == 0)
		{
			BOOL bRet = InstallInjectionDriver(INJECT_DRIVER_NAME, INJECT_DRIVER32, INJECT_DRIVER64, INJECT_DESCRIPTION);
			printf("%s to activate driver, last error: %d\n", bRet? "Succeeded": "Failed", GetLastError());
		}
		else if (_wcsicmp(argv[1], L"/d") == 0)
		{
			BOOL bRet = LoadInjectionDriver(INJECT_DRIVER_NAME, INJECT_DRIVER32, INJECT_DRIVER64);
			printf("%s to activate driver, last error: %d\n", bRet? "Succeeded": "Failed", GetLastError());
		}
		else
		{
			help();
		}
	}

	if(argc == 3)//deactivate driver
	{
		if(_wcsicmp(argv[1], L"/s") == 0)
		{
			BOOL bRet = UninstallInjectionDriver(INJECT_DRIVER_NAME);
			printf("%s to deactivate driver, last error: %d\n", bRet? "Succeeded": "Failed", GetLastError());
		}
		else if (_wcsicmp(argv[1], L"/d") == 0)
		{
			BOOL bRet = StopInjectionDriver(INJECT_DRIVER_NAME);
			printf("%s to stop driver, last error: %d\n", bRet? "Succeeded": "Failed", GetLastError());
		}
		else
		{
			help();
		}
	}
	
	
	return 0;
}

