// nlstorage_service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Services.h"
#include "CriticalSections.h"
#include "WindowsService.h"

#include <iostream>
using namespace std;

#define NLSTORAGE_SERVICE_NAME L"nlstorage_service"

wchar_t windows_service_path[MAX_PATH + 1] = {0};
wchar_t windows_service_log_path[MAX_PATH + 1] = {0};

void log(wchar_t* log_msg);
void windows_service_main_function(void);
void windows_service_manipulate(wchar_t* argv);

int _tmain(int argc, wchar_t* argv[])
{
	CCriticalSections::Init();

	if(1 == argc)
	{
		CServices::StartServices();
	}
	else if(argc == 2)
	{
		windows_service_manipulate(argv[1]);
	}
	else
	{
		wprintf_s(L"error: illegal argument...\n");
	}

	CCriticalSections::Delete();

	return 0;
}

void log(wchar_t* log_msg)
{
	// get log path: current exe file path plus string ".log"
	while(wcslen(windows_service_log_path) == 0)
	{
		GetModuleFileName(NULL, windows_service_log_path, MAX_PATH);
		wcsncat_s(windows_service_log_path, MAX_PATH, L".log", _TRUNCATE);
		wprintf_s(L"notice: log path - %s...\n", windows_service_log_path);
	}

	FILE* log = NULL;
	errno_t err = _wfopen_s(&log, windows_service_log_path, L"a");
	if(0 == err && NULL != log)
	{
		SYSTEMTIME system_time;
		ZeroMemory(&system_time, sizeof(SYSTEMTIME));
		GetLocalTime(&system_time);

		fwprintf_s(log, L"%04d-%02d-%02d %02d:%02d:%02d.%03d - %s\n",
			system_time.wYear, system_time.wMonth, system_time.wDay, system_time.wHour, system_time.wMinute, system_time.wSecond, system_time.wMilliseconds,
			log_msg);

		fclose(log);
	}
}

void windows_service_main_function(void)
{
	wchar_t log_msg[256] = {0};
	_snwprintf_s(log_msg,256, _TRUNCATE, L"notice: windows service[%s] running...", NLSTORAGE_SERVICE_NAME);
	log(log_msg);
	
	CServices::StartServices();
}

void windows_service_manipulate(wchar_t* argv)
{
	// windows service exe file path - current exe file
	GetModuleFileName(NULL, windows_service_path, MAX_PATH);
	wchar_t path_log_str[MAX_PATH] = {0};
	_snwprintf_s(path_log_str, MAX_PATH, _TRUNCATE, L"notice: app path - %s...", windows_service_path);
	log(path_log_str);
	
	wchar_t log_msg[256] = {0};

	//   windows service operation    //
	//--------------------------------//
	// -i: install windows service.   //
	// -u: uninstall windows service. //
	// -b: begin windows service.     //
	// -e: end windows service.       //
	// -service: windows service.     //
	//--------------------------------//
	CWindowsService* windows_service = CWindowsService::get_instance();
	windows_service->initialize(NLSTORAGE_SERVICE_NAME, windows_service_path, windows_service_main_function);
	if(0 == wcscmp(L"-i", argv))
	{
		if(windows_service->install())
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"notice: windows service[%s] install success...", NLSTORAGE_SERVICE_NAME);
		}
		else
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"error[%d]: windows service[%s] install failed...", GetLastError(), NLSTORAGE_SERVICE_NAME);
		}
	}
	else if(0 == wcscmp(L"-u", argv))
	{
		if(windows_service->uninstall())
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"notice: windows service[%s] uninstall success...", NLSTORAGE_SERVICE_NAME);
		}
		else
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"error[%d]: windows service[%s] uninstall failed...", GetLastError(), NLSTORAGE_SERVICE_NAME);
		}
	}
	else if(0 == wcscmp(L"-b", argv))
	{
		if(windows_service->begin())
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"notice: windows service[%s] begin/start success...", NLSTORAGE_SERVICE_NAME);
		}
		else
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"error[%d]: windows service[%s] begin/start failed...", GetLastError(), NLSTORAGE_SERVICE_NAME);
		}
	}
	else if(0 == wcscmp(L"-e", argv))
	{
		if(windows_service->end())
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"notice: windows service[%s] end/stop success...", NLSTORAGE_SERVICE_NAME);
		}
		else
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"error[%d]: windows service[%s] end/stop failed...", GetLastError(), NLSTORAGE_SERVICE_NAME);
		}
	}
	else if(0 == wcscmp(L"-service", argv))
	{
		if(windows_service->ctrl_dispatch())
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"notice: windows service[%s] ctrl_dispatch success...", NLSTORAGE_SERVICE_NAME);
		}
		else
		{
			_snwprintf_s(log_msg, 256, _TRUNCATE, L"error[%d]: windows service[%s] ctrl_dispatch failed...", GetLastError(), NLSTORAGE_SERVICE_NAME);
		}
	}
	else
	{
		_snwprintf_s(log_msg, 256, _TRUNCATE, L"error: unknow argument received...");
	}
	windows_service = NULL;

	log(log_msg);
}