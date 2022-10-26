#include <windows.h>
#include <string>
#include <tchar.h>
#include <list>
#include "resource.h"
#include <shlwapi.h>
#include <tlhelp32.h>

#ifdef _UNICODE
typedef std::wstring TString;
#else
typedef std::string TString;
#endif

TString GetCurrentDir();
void OutputLogInfo(const TCHAR * szLog);
int EnforcerStop(HINSTANCE hInstance, LPCTSTR lpCmdLine, int nCmdShow);
int StopEdpManagerForAllUser();
void TerminateProcessByID(DWORD dwID);
std::wstring GetRegValueString(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValueName);

class ComponentMgr
{
public:
	ComponentMgr();
	~ComponentMgr();

protected:
	ComponentMgr(const ComponentMgr&);
	std::list<TString> m_lstComponentFileName;
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR  lpCmdLine, int  nCmdShow)
{
	//
	TCHAR szLog[MAX_PATH];
	_stprintf_s(szLog, _T("PCStop.exe command:%s\n"), lpCmdLine);
	OutputLogInfo(szLog);

	//Create Process to execute EnforcerStop.exe
	int nStopPC = EnforcerStop(hInstance, lpCmdLine, nCmdShow);

	if (nStopPC==0)
	{
		//wait the commprofile.xml can be access
		std::wstring wstrPCDir = GetRegValueString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller", L"PolicyControllerDir");
		if (wstrPCDir.empty())
		{
			wstrPCDir = L"C:\\Program Files\\NextLabs\\Policy Controller\\";
		}
		std::wstring wstrBundlebinFile = wstrPCDir + L"config\\commprofile.xml";
		
		HANDLE hBundleBin = INVALID_HANDLE_VALUE;
		const int nMaxWait = 10;
		int WaitTime = 0;
		do 
		{
			WaitTime++;
			hBundleBin = CreateFileW(wstrBundlebinFile.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hBundleBin==INVALID_HANDLE_VALUE)
			{
				Sleep(1000);
			}
		} while ((hBundleBin==INVALID_HANDLE_VALUE) && (WaitTime<=nMaxWait) );

		if (hBundleBin==INVALID_HANDLE_VALUE)
		{
			OutputLogInfo(L"PCStop.exe return failed. because commprofile.xml can't be access.\n");
			return 	1603;
		}
		else
		{
			CloseHandle(hBundleBin);
		}
		

		//stop edpmanager
		StopEdpManagerForAllUser();
		return 0;
	}

	return nStopPC;
}


TString GetCurrentDir()
{
	TCHAR szPrepPath[MAX_PATH + 1] = { 0 };
	DWORD dwRst = GetModuleFileName(NULL, szPrepPath, MAX_PATH);

	if (dwRst > 0)
	{
		szPrepPath[dwRst] = 0;

		TCHAR szLog[200];
		_stprintf_s(szLog, _T("GetModuleFileName success:%s\n"), szPrepPath);
		//OutputLogInfo(szLog);

		TString strPrepPath = szPrepPath;
		size_t 	nPos = strPrepPath.find_last_of(_T('\\'));
		if (nPos != TString::npos)
		{
			return strPrepPath.substr(0, nPos + 1);
		}
	}
	else
	{
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("GetModuleFileNameFailed, LastError=%d\n"), GetLastError());
		//	OutputLogInfo(szLog);
	}


	return _T("");

}

void OutputLogInfo(const TCHAR * szLog)
{
	int nLogLen = _tcslen(szLog) + 100;
	TCHAR* szOutput = new TCHAR[nLogLen+1];
	_stprintf_s(szOutput, nLogLen,  _T("PCStop:%s"), szLog);
	OutputDebugString(szOutput);
	delete[] szOutput;
	szOutput = NULL;
	//_tprintf(szLog);
}

ComponentMgr::~ComponentMgr()
{
	OutputLogInfo(_T("Delete Component.\n"));

	std::list<TString>::iterator itCom = m_lstComponentFileName.begin();

	while (itCom != m_lstComponentFileName.end())
	{
		DeleteFile(itCom->c_str());

		itCom++;
	}

	m_lstComponentFileName.clear();	
}

int StopEdpManagerForAllUser()
{
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		OutputDebugString(L"CreateToolhelp32Snapshot  fail!\n");
		return 0;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return 0;
	}

	std::list<DWORD> lstDwEdpMgrProcessID;
	do
	{
		if (_tcsstr(pe32.szExeFile, _T("edpmanager")))
		{

			lstDwEdpMgrProcessID.push_back(pe32.th32ProcessID);

			wchar_t wzLog[1024];
			wsprintfW(wzLog, L"Get edpmanager: %u\n", pe32.th32ProcessID);
			OutputLogInfo(wzLog);

			//break;   may be more then one edgmanager.exe instance.
		}

	} while (Process32Next(hProcessSnap, &pe32));

	//terminate process
	std::list<DWORD>::iterator itEdpProcessID = lstDwEdpMgrProcessID.begin();
	while (itEdpProcessID != lstDwEdpMgrProcessID.end())
	{
		TerminateProcessByID(*itEdpProcessID);
		itEdpProcessID++;
	}
	
	CloseHandle(hProcessSnap);
}

void EnableDebugPriv()
{
	wchar_t szLog[1024];

	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

	LUID luid;
	if (!::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		wsprintfW(szLog, _T("LookupPrivilegeValue, lastError: %u\n"), GetLastError());
		OutputLogInfo(szLog);
		CloseHandle(hToken);
		return;
	}

	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!::AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL))
	{
		wsprintfW(szLog, _T("AdjustTokenPrivileges, lastError: %u\n"), GetLastError());
		OutputLogInfo(szLog);
		CloseHandle(hToken);
		return;
	}

	CloseHandle(hToken);
}


void TerminateProcessByID(DWORD ProcessId)
{
	//terminate process for anthoer user, we must adjust the Privilege of current process.
	EnableDebugPriv();

	if (ProcessId > 0)
	{
		HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

		if (hProc)
		{
			BOOL bTerminate = TerminateProcess(hProc, 0xffffffff);
			if (bTerminate)
			{
				OutputLogInfo(_T("Wait process to exit..."));
				WaitForSingleObject(hProc, 60 * 60 * 1000);
				Sleep(5 * 1000);
				OutputLogInfo(_T("process to exit success."));
			}
			else
			{
				OutputDebugString(L"close the TerminateProcessByID failed");
			}

			CloseHandle(hProc);
		}
		else
		{
			wchar_t szLog[1024];
			wsprintfW(szLog, L"Close process fail, lastError=%u\n", GetLastError());
			OutputDebugString(szLog);
		}
	}
}

std::wstring GetRegValueString(HKEY hkey, LPCWSTR lpSubKey, LPCWSTR lpValueName)
{
	//open subkey
	HKEY hSubKey = NULL;
	long lRegResult = RegOpenKeyEx(hkey, lpSubKey, 0, KEY_READ, &hSubKey);
	if ((lRegResult == ERROR_SUCCESS) && (hSubKey != NULL))
	{
		DWORD dwDataLen = 0;
		lRegResult = RegQueryValueExW(hSubKey, lpValueName, NULL, NULL, NULL, &dwDataLen);
		if (lRegResult==ERROR_SUCCESS)
		{
			wchar_t* pData = new wchar_t[dwDataLen / 2 + 1];
			memset(pData, 0, dwDataLen / 2 + 1);
			RegQueryValueExW(hSubKey, lpValueName, NULL, NULL, (LPBYTE)pData, &dwDataLen);
			std::wstring wstrValue = pData;
			delete[] pData;
			pData = NULL;
			return wstrValue;
		}	

		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}

	return L"";
}