// elevate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <ShellAPI.h>

BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

	}


	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;

}

BOOL IsWin7()
{
	DWORD dwMajor, dwMinor;
	return ( GetOSInfo(dwMajor, dwMinor) && dwMajor >= 6 )? TRUE: FALSE;
}

BOOL RunAsAdmin( HWND hWnd, LPTSTR lpFile, LPTSTR lpParameters )
{
	SHELLEXECUTEINFO   sei;
	ZeroMemory ( &sei, sizeof(sei) );

	sei.cbSize          = sizeof(SHELLEXECUTEINFOW);
	sei.hwnd            = hWnd;
	sei.fMask           = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
	sei.lpVerb          = _TEXT("runas");
	sei.lpFile          = lpFile;
	sei.lpParameters    = lpParameters;
	sei.nShow           = SW_HIDE;

	if ( ! ShellExecuteEx ( &sei ) )
	{
		printf( "Error: ShellExecuteEx failed 0x%x\n", GetLastError() );
		return FALSE;
	}
	return TRUE;
}


int _tmain(int argc, _TCHAR* argv[])
{
	if(argc != 1 || argv == NULL)
	{
		return 0;
	}

	wchar_t* pPath = argv[0];

	if(IsWin7())
	{
		RunAsAdmin(NULL, pPath, NULL);
	}
	else
	{
		ShellExecute(NULL, L"open", pPath, NULL, NULL, SW_HIDE);
	}

	return 0;
}

