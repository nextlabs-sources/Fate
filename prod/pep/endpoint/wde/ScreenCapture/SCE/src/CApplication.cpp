#include "CApplication.h"
#include "evaluate.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable: 4995) 
#include "celog.h"
#pragma warning(pop)

extern CELog  g_log;

namespace SCEClient
{

namespace
{
	const int NL_PATH_BUF_SIZE = 1024;
}

bool CApplication::QueryAllAppOfCurrentSession(std::string& DisplayText) const
{
	std::vector<std::wstring> paths = GetProcessesOfCurrentSession();

	for(std::vector<std::wstring>::const_iterator cit = paths.begin(); cit != paths.end(); ++cit)
	{
		if (!EvaluateApp(cit->c_str()))
		{
			g_log.Log(CELOG_DEBUG, L"Denied in QueryAllAppOfCurrentSession, evaluating %s\n", cit->c_str());

			return false;
		}		
	}

	return true;
}

bool CApplication::QueryApp(DWORD ProcessID, std::string& DisplayText) const
{
	DisplayText.clear(); 

	if(0 == ProcessID)
	{
		return QueryAllAppOfCurrentSession(DisplayText);
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);

	if (NULL == hProcess)
	{
		return true;
	}

	wchar_t pathImage[NL_PATH_BUF_SIZE];
	;
	bool bAllow = true;

	if(GetProcessImageFileNameW(hProcess, pathImage, NL_PATH_BUF_SIZE) > 0)
	{
		if(DeviceNameToDriveName(pathImage))
		{
			bAllow = EvaluateApp(pathImage);

			if (!bAllow)
			{
				g_log.Log(CELOG_DEBUG, L"Denied in QueryApp, evaluating %s\n", pathImage);
			}
		}
	}

	CloseHandle(hProcess);

	return bAllow;
}

std::vector<std::wstring> CApplication::GetProcessesOfCurrentSession() const
{
	std::vector<std::wstring> paths;

	// Take a snapshot of all processes in the system.
	HANDLE hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if( INVALID_HANDLE_VALUE == hProcessSnap )
	{
		g_log.Log(CELOG_DEBUG, L"CreateToolhelp32Snapshot failed , last error : %d\n", GetLastError());

		return paths;
	}

	PROCESSENTRY32W pe32 = { 0 };

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof( PROCESSENTRY32W );

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if( !Process32FirstW( hProcessSnap, &pe32 ) )
	{
		g_log.Log(CELOG_DEBUG, L"Process32FirstW failed , last error : %d\n", GetLastError());

		CloseHandle( hProcessSnap );
		return paths;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		DWORD ProcessSessionID = 0;

		ProcessIdToSessionId(pe32.th32ProcessID, &ProcessSessionID);

		if(m_SessionID == ProcessSessionID)
		{
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);

			if(hProcess != NULL)
			{
				wchar_t pathImage[NL_PATH_BUF_SIZE];
				;
				if(GetProcessImageFileNameW(hProcess, pathImage, NL_PATH_BUF_SIZE) > 0)
				{
					if(DeviceNameToDriveName(pathImage))
					{
						paths.push_back(pathImage);
					}
				}

				CloseHandle(hProcess);
			}
		}
	}
	while( Process32NextW( hProcessSnap, &pe32 ) );

	CloseHandle(hProcessSnap);

	return paths;
}

BOOL CApplication::DeviceNameToDriveName(LPWSTR pszFilename) const
{
	BOOL bFound = FALSE;

	// Translate path with device name to drive letters.
	wchar_t szTemp[NL_PATH_BUF_SIZE] = { 0 };

	if (GetLogicalDriveStringsW(NL_PATH_BUF_SIZE-1, szTemp)) 
	{
		wchar_t szName[NL_PATH_BUF_SIZE] = { 0 };
		wchar_t szDrive[3] = L" :";
		wchar_t* p = szTemp;

		do 
		{
			// Copy the drive letter to the template string
			*szDrive = *p;

			// Look up each device name
			if (QueryDosDeviceW(szDrive, szName, NL_PATH_BUF_SIZE))
			{
				size_t uNameLen = wcslen(szName);

				if (uNameLen < MAX_PATH) 
				{
					bFound = _wcsnicmp(pszFilename, szName, uNameLen) == 0;

					if (bFound) 
					{
						// Reconstruct pszFilename using szTempFile
						// Replace device path with DOS path
						wchar_t szTempFile[MAX_PATH] = { 0 };

						StringCchPrintfW(szTempFile, MAX_PATH, L"%s%s", szDrive, pszFilename+uNameLen);
						StringCchCopyNW(pszFilename, MAX_PATH+1, szTempFile, wcslen(szTempFile));
					}
				}
			}

			// Go to the next NULL character.
			while (*p++);

		} while (!bFound && *p); // end of string
	}

	return bFound;
}

}