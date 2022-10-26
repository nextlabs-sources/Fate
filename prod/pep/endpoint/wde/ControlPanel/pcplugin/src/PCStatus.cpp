// PCStatus.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "policy_controller.hpp"

#define EVENT_STARTING_PC			L"NXTLBS_STARTING_PC"
#define EVENT_STOPPING_PC			L"NXTLBS_STOPPING_PC"

void __stdcall QueryStatus(char* pBuf, int nLen)
{
	HANDLE hEvent = OpenEventW(READ_CONTROL, TRUE, EVENT_STARTING_PC);
	if(hEvent)
	{
		strncpy_s(pBuf, nLen, "Starting", _TRUNCATE);
		CloseHandle(hEvent);
		return;
	}

	hEvent = OpenEventW(READ_CONTROL, TRUE, EVENT_STOPPING_PC);
	if(hEvent)
	{
		strncpy_s(pBuf, nLen, "Stopping", _TRUNCATE);
		CloseHandle(hEvent);
		return;
	}

	if(nextlabs::policy_controller::is_up())
	{
		strncpy_s(pBuf, nLen, "Running", _TRUNCATE);
	}
	else
	{
		strncpy_s(pBuf, nLen, "Not Running", _TRUNCATE);
	}

}
