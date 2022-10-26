#include "../include/ApiHook.h"
#include <string>
#include <process.h>
#include "madCHook_helper.h"

LPMAPILOGONEX next_MAPILogonEx = NULL;

using namespace std;
const int BUFSIZE = 512;


#pragma warning(push)
#pragma warning(disable:4819 4996 4995)
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#pragma warning(pop)

bool Connect(HANDLE& h,DWORD dwReconectCnt=10)
{
	bool bRet = false;
	HANDLE hPipe = INVALID_HANDLE_VALUE; 

	DWORD dwProcID = GetCurrentProcessId();
	wchar_t szPipeName[1024] = {0};
	_snwprintf_s(szPipeName, 1024, _TRUNCATE, L"\\\\.\\pipe\\CEAdobe_%d", dwProcID);
	for(DWORD i = 0; i < dwReconectCnt; i++) 
	{ 
		hPipe = CreateFile( 
			szPipeName,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 
		if  (hPipe != INVALID_HANDLE_VALUE) 
		{
			h = hPipe;
			bRet = true;
			break; 
		}	
	} 

	return bRet;
}

int Send(HANDLE hPipe, const unsigned char *data, int len)
{
	DWORD dwMode = PIPE_READMODE_MESSAGE; 
	BOOL fSuccess = SetNamedPipeHandleState( 
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 
	if (!fSuccess) 
	{
		return GetLastError();
	}

	int totalWrite = 0;


	while ( len > 0 )
	{
		DWORD cbWritten = 0;
		fSuccess = WriteFile( 
			hPipe,                  // pipe handle 
			data + totalWrite,             // message 
			len <= BUFSIZE? len: BUFSIZE,              // message length 
			&cbWritten,             // bytes written 
			NULL);  

		if (!fSuccess) 
		{
			DWORD dwErr = GetLastError();
			return dwErr;
		}
		len -= cbWritten;
		totalWrite += cbWritten;
	}
	FlushFileBuffers(hPipe);
	return 0;
}

HRESULT WINAPI MyMAPILogonEx (ULONG_PTR ulUIParam, LPTSTR lpszProfileName, LPTSTR lpszPassword, ULONG ulFlags, LPMAPISESSION* lppSession)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	if (Connect(hPipe) && hPipe != INVALID_HANDLE_VALUE)
	{
		Send(hPipe, (const unsigned char*)"1", 1);
        OutputDebugStringW(L"CEAdobeTrm-- MyMAPILogonEx: Send begin!\n");
		CloseHandle(hPipe);
	}
	
	return next_MAPILogonEx(ulUIParam, lpszProfileName, lpszPassword, ulFlags, lppSession);
}

void Hook(void)
{
	InitializeMadCHook();
	
	HookAPI("MAPI32.dll", "MAPILogonEx", MyMAPILogonEx, (PVOID*) &next_MAPILogonEx, 0);
}

//Unhook Apis
void Unhook(void)
{	
	if (next_MAPILogonEx)
	{
		UnhookAPI((PVOID*)&next_MAPILogonEx);
	}

	FinalizeMadCHook();
}
