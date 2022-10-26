#include "StdAfx.h"
#include <stdlib.h>
#include "WindowsService.h"

MainFunction CWindowsService::main_function = NULL;
wchar_t      CWindowsService::windows_service_name[128 + 1] = {0};
wchar_t      CWindowsService::windows_service_path[MAX_PATH + 1] = {0};

SERVICE_STATUS        CWindowsService::windows_service_status;
SERVICE_STATUS_HANDLE CWindowsService::windows_service_status_handle;

CWindowsService::CWindowsService(){}

CWindowsService::~CWindowsService(){}

CWindowsService* CWindowsService::get_instance()
{
	static CWindowsService instance;
	return &instance;
}

void CWindowsService::initialize(_In_ const wchar_t* windows_service_name_ptr, _In_ const wchar_t* windows_service_path_ptr, _In_ const MainFunction main_function_ptr)
{
	static bool initialized = false;
	if(initialized)
	{
		wprintf_s(L"CWindowsService error: windows service only can be initialized one time...\n");
		wprintf_s(L"current windows service information\n");
		wprintf_s(L"name: %s\n", windows_service_name);
		wprintf_s(L"path: %s\n", windows_service_path);
		wprintf_s(L"func: %p\n", main_function);
		wprintf_s(L"\n");
		return;
	}

	memset(windows_service_name, 0, sizeof(windows_service_name));
	wcsncpy_s(windows_service_name, _countof(windows_service_name), windows_service_name_ptr, _TRUNCATE);
	memset(windows_service_path, 0, sizeof(windows_service_path));
	wcsncpy_s(windows_service_path, _countof(windows_service_path), windows_service_path_ptr, _TRUNCATE);
	main_function = NULL;
	main_function = main_function_ptr;

	initialized = true;
}

bool CWindowsService::install()
{
	if(0 == wcslen(windows_service_name) || 0 == wcslen(windows_service_path))
	{
		wprintf(L"CWindowsService error: un-initialized windows service object...\n");
		return false;
	}

	bool res = false;

	SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if(NULL != sc_manager)
	{
		wchar_t windows_service_path_ex[MAX_PATH] = {0};
		_snwprintf_s(windows_service_path_ex, MAX_PATH, _TRUNCATE, L"%s -service", windows_service_path);
		SC_HANDLE sc_service = CreateService(
			sc_manager, // SCManager database
			windows_service_name, // name of service
			windows_service_name, // service name to display
			SERVICE_ALL_ACCESS, // desired access
			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, // service type
			SERVICE_AUTO_START, // start type
			SERVICE_ERROR_NORMAL, // error control type
			windows_service_path_ex, // service's binary
			NULL, // no load ordering group
			NULL, // no tag identifier
			NULL, // no dependencies
			NULL, // LocalSystem account
			NULL); // no password
		if(NULL != sc_service) 
		{
			wprintf(L"CWindowsService: CreateService[%s] successful...\n", windows_service_name);

			// when user shutdown this windows service,
			// this windows service will restart itself as remedy.
			// --------------------------------------------------------------------------
			SC_ACTION sc_action[3];
			sc_action[0].Delay = 10 * 1000; // 10 seconds suspending
			sc_action[0].Type = SC_ACTION_RESTART;
			sc_action[1].Delay = 0;
			sc_action[1].Type = SC_ACTION_NONE;
			sc_action[2].Delay = 0;
			sc_action[2].Type = SC_ACTION_NONE;

			SERVICE_FAILURE_ACTIONS service_failure_actions = {0};
			service_failure_actions.lpRebootMsg = NULL;
			service_failure_actions.dwResetPeriod = 60 * 60 * 24;
			service_failure_actions.cActions = 3;
			service_failure_actions.lpsaActions = sc_action;
			service_failure_actions.lpCommand = NULL;
			if(ChangeServiceConfig2(sc_service, SERVICE_CONFIG_FAILURE_ACTIONS, &service_failure_actions))
			{
				wprintf(L"CWindowsService: CrashRemedy[%s] successful...\n", windows_service_name);
				res = true;
			}
			else
			{
				wprintf(L"CWindowsService error[%ld]: ChangeServiceConfig2[%s] failed...\n", GetLastError(), windows_service_name);
			}
			// --------------------------------------------------------------------------

			CloseServiceHandle(sc_service); 
		}
		else
		{
			wprintf_s(L"CWindowsService error[%ld]: CreateService[%s] failed...\n", GetLastError(), windows_service_name);
		}
		CloseServiceHandle(sc_manager);
	}
	else
	{
		wprintf_s(L"CWindowsService error[%ld]: OpenSCManager failed...\n", GetLastError());
	}

	return res;
}

bool CWindowsService::uninstall()
{
	if(0 == wcslen(windows_service_name))
	{
		wprintf(L"CWindowsService error: un-initialized windows service object...\n");
		return false;
	}

	bool res = false;

	SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(NULL != sc_manager) 
	{
		SC_HANDLE sc_service = OpenService(sc_manager, windows_service_name, SERVICE_ALL_ACCESS);
		if(NULL != sc_service) 
		{
			if(DeleteService(sc_service))
			{
				wprintf_s(L"CWindowsService: DeleteService[%s] successful...\n", windows_service_name);
				res = true;
			}
			else 
			{
				wprintf_s(L"CWindowsService error[%ld]: DeleteService[%s] failed...\n", GetLastError(), windows_service_name);
			}
			CloseServiceHandle(sc_service); 
		}
		else
		{
			wprintf_s(L"CWindowsService error[%ld]: OpenService failed...\n", GetLastError());
		}
		CloseServiceHandle(sc_manager);
	}
	else
	{
		wprintf_s(L"CWindowsService error[%ld]: OpenSCManager failed...\n", GetLastError());
	}

	return res;
}

bool CWindowsService::begin()
{
	if(0 == wcslen(windows_service_name))
	{
		wprintf_s(L"CWindowsService error: un-initialized windows service object...\n");
		return false;
	}

	bool res = false;

	SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(NULL != sc_manager) 
	{
		SC_HANDLE sc_service = OpenService(sc_manager, windows_service_name, SERVICE_ALL_ACCESS);
		if(NULL != sc_service) 
		{
			if(StartService(sc_service, 0, NULL))
			{
				wprintf_s(L"CWindowsService: StartService[%s] successful...\n", windows_service_name);
				res = true;
			}
			else
			{
				wprintf_s(L"CWindowsService error[%ld]: StartService[%s] failed...\n", GetLastError(), windows_service_name);
			}
			CloseServiceHandle(sc_service); 
		}
		else
		{
			wprintf_s(L"CWindowsService error[%ld]: OpenService failed...\n", GetLastError());
		}
		CloseServiceHandle(sc_manager);
	}
	else
	{
		wprintf_s(L"CWindowsService error[%ld]: OpenSCManager failed...\n", GetLastError());
	}

	return res;
}

bool CWindowsService::end()
{
	if(0 == wcslen(windows_service_name))
	{
		wprintf_s(L"CWindowsService error: un-initialized windows service object...\n");
		return false;
	}

	bool res = false;
	
	SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(NULL != sc_manager) 
	{
		SC_HANDLE sc_service = OpenService(sc_manager, windows_service_name, SERVICE_ALL_ACCESS);
		if(NULL != sc_service) 
		{
			SERVICE_STATUS status;
			if(ControlService(sc_service, SERVICE_CONTROL_STOP, &status))
			{
				wprintf_s(L"CWindowsService: StopService[%s] successful...\n", windows_service_name);
				res = true;
			}
			else
			{
				wprintf_s(L"CWindowsService error[%ld]: StopService[%s] failed...\n", GetLastError(), windows_service_name);
			}
			CloseServiceHandle(sc_service); 
		}
		else
		{
			wprintf_s(L"CWindowsService error[%ld]: OpenService failed...\n", GetLastError());
		}
		CloseServiceHandle(sc_manager);
	}
	else
	{
		wprintf_s(L"CWindowsService error[%ld]: OpenSCManager failed...\n", GetLastError());
	}

	return res;
}

bool CWindowsService::ctrl_dispatch()
{
	if(0 == wcslen(windows_service_name))
	{
		wprintf_s(L"CWindowsService error: un-initialized windows service object...\n");
		return false;
	}

	SERVICE_TABLE_ENTRY service_table_entry[2];
	service_table_entry[0].lpServiceName = windows_service_name;
	service_table_entry[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)WindowsServiceMain;
	service_table_entry[1].lpServiceName = NULL;
	service_table_entry[1].lpServiceProc = NULL;
	if(!StartServiceCtrlDispatcher(service_table_entry))
	{
		wprintf_s(L"CWindowsService error[%ld]: StartServiceCtrlDispatcher[%s] failed...\n", GetLastError(), windows_service_name);
		return false;
	}
	return true;
}

VOID WINAPI CWindowsService::WindowsServiceMain(DWORD /* dwArgc */, LPTSTR* /* lpszArgv */)
{
	if(0 == wcslen(windows_service_name) || NULL == main_function)
	{
		wprintf_s(L"CWindowsService error: un-initialized windows service object...\n");
		return;
	}
	
	windows_service_status.dwServiceType = SERVICE_WIN32;
	windows_service_status.dwCurrentState = SERVICE_START_PENDING;
	windows_service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	windows_service_status.dwWin32ExitCode = 0;
	windows_service_status.dwServiceSpecificExitCode = 0;
	windows_service_status.dwCheckPoint = 0;
	windows_service_status.dwWaitHint = 0;

	// register service control handler function, here is function WindowsServiceControlHandler.
	windows_service_status_handle = RegisterServiceCtrlHandler(windows_service_name, WindowsServiceControlHandler);
	if(NULL != windows_service_status_handle) 
	{
		windows_service_status.dwCurrentState = SERVICE_RUNNING; 
		windows_service_status.dwCheckPoint = 0; 
		windows_service_status.dwWaitHint = 0;  
		if(SetServiceStatus(windows_service_status_handle, &windows_service_status))
		{ 
			wprintf_s(L"notice: SetServiceStatus successfully...");

			// you can do something here as windows service running.
			main_function();
		}
		else
		{
			wprintf_s(L"CWindowsService error[%ld]: SetServiceStatus failed...\n", GetLastError());
		}
	}
	else
	{
		wprintf_s(L"CWindowsService error[%ld]: RegisterServiceCtrlHandler failed...\n", GetLastError());
	}
}

VOID WINAPI CWindowsService::WindowsServiceControlHandler(DWORD fdwControl)
{
	bool change_status = true;
	switch(fdwControl)
	{
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		{
			windows_service_status.dwWin32ExitCode = 0;
			windows_service_status.dwCurrentState = SERVICE_STOPPED;
			windows_service_status.dwCheckPoint = 0;
			windows_service_status.dwWaitHint = 0;
		}
		break;
	case SERVICE_CONTROL_PAUSE:
		{
			windows_service_status.dwCurrentState = SERVICE_PAUSED;
		}
		break;
	case SERVICE_CONTROL_CONTINUE:
		{
			windows_service_status.dwCurrentState = SERVICE_RUNNING;
		}
		break;
	case SERVICE_CONTROL_INTERROGATE:
		{
			change_status = false;
		}
		break;
	default:
		{
			change_status = false;
			wprintf_s(L"CWindowsService error: Unknow windows service control, value is %d...\n", fdwControl);
		}
		break;
	}
	if(change_status) // if needed, changed windows service's status
	{
		if(!SetServiceStatus(windows_service_status_handle, &windows_service_status))
		{
			wprintf_s(L"CWindowsService error[%ld]: SetServiceStatus failed...\n", GetLastError());
		}
	}
}