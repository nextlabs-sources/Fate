#pragma once

#include <windows.h>

typedef void (*MainFunction)(void); //prototype of the windows service main function

class CWindowsService
{
private:
	static MainFunction main_function;
	static wchar_t        windows_service_name[128 + 1];
	static wchar_t        windows_service_path[MAX_PATH + 1];
private:
	CWindowsService();
	~CWindowsService();
private:
	static SERVICE_STATUS        windows_service_status; 
	static SERVICE_STATUS_HANDLE windows_service_status_handle;
	static VOID WINAPI WindowsServiceMain(DWORD /* dwArgc */, LPTSTR* /* lpszArgv */);
	static VOID WINAPI WindowsServiceControlHandler(DWORD fdwControl);
public:
	static CWindowsService* get_instance();
public:
	//initialize windows service base information
	void initialize(_In_ const wchar_t* windows_service_name, _In_ const wchar_t* windows_service_path, _In_ MainFunction main_function_ptr);
	//windows service install
	bool install();
	//windows service uninstall
	bool uninstall();
	// begin/start a windows service
	bool begin();
	// end/stop a windows service
	bool end();
	// start windows service control dispatcher
	bool ctrl_dispatch();
};