// OEInstallPrep.cpp : Defines the entry point for the console application.
//

#pragma warning(push)
#pragma warning(disable: 4005)
#define _WIN32_WINNT 0x0600
#pragma warning(pop)
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string>
#include <Psapi.h>
#include "resource.h"
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <shlwapi.h>
#include "strsafe.h"
#include <shellapi.h>
#include <shlobj.h> 
#include "tlhelp32.h"


#define BUFSIZE 4096

#ifdef _UNICODE
typedef std::wstring TString;
#else
typedef std::string TString;
#endif


#pragma comment(lib, "Psapi.lib")
#pragma comment( linker, "/subsystem:\"windows\"  /entry:\"wmainCRTStartup\" " )


#pragma comment(lib, "shlwapi.lib")

enum emProductType
{
	OE_PRODUCT,
	PC_PRODUCT
};

enum emPolicyDirType
{
	NEXTLABS_DIRECTORY,
	PC_DIRECTORY
};

enum emProcessType
{
	PROC_NAME_EDPMANAGER,
	PROC_NAME_NLCONTEXTMAGR
};


void ParseCommandLine(int argc, _TCHAR* argv[]);
int StopPC();
void OutputLogInfo(const TCHAR * szLog);
bool IsSilentMode();
int DeleteOEFile();
void DeleteRegister();
TString GetOEInstallDir();
int DeleteDirectory(const TCHAR* szDir);
void UnregisterComponent(const TCHAR* szComponent);
bool IsRunOn64BitWindows();
TString GetAdobeReaderInstallPath(const TCHAR* szVersion);
TString GetAdobeAcrobatInstallPath(const TCHAR* szVersion);
TString GetOEInstallPrepPath();
int DeleteAdobeComponent();
TString GetProductCode(emProductType ProductType);
void RemoveInstallInfo(emProductType ProductType);
bool FailedDeleteFile(const TCHAR* szFileFullName);
int DeleteDestopEnforcer();
DWORD GetProcessID(emProcessType ProcType);
TString GetDesktopEnforcerInstallPath();
int EnforcerStop(HINSTANCE hInstance, LPCTSTR lpCmdLine, int nCmdShow);
int OEPluginDeleteFile(const TCHAR* szFile);
TString GetRegValueString(HKEY hkey, LPCTSTR lpSubKey, LPCTSTR lpValueName);
TString GetTempDirectory();
TString GetPCInstallPath(emPolicyDirType DirType);
int DeletePCFile();
int DeleteCommonFile();
TString GetCommonInstallPath();
BOOL GetEnvValue(_In_ const TString& strEnvName,_Out_  TString & strEnvValue);
BOOL SetEnvValue(_In_ const TString& strEnvName,_In_  TString & strEnvValue);
void RemoveEnv();

BOOL RemoveEnvValue(const TString& strEnvName, const TString &strEnvValue);
int RemoveDriveFile();
int RemoveLinkFile();

void CloseProc(DWORD ProcessId);
void CloseNLContextMgr();
void WriteRegValue();

int DeleteService();



TString g_strMode;  //silent or other.
TString g_strPCPassword;  //password of pc
TString g_strOEInstallDir;
bool    g_bWriteRoot;
const TCHAR* g_szDefaultOEInstallDir64 = _T("C:\\Program Files (x86)\\NextLabs\\Outlook Enforcer\\");
const TCHAR* g_szDefaultOEInstallDir32 = _T("C:\\Program Files\\NextLabs\\Outlook Enforcer\\");

const TCHAR* g_szDefaultDesktopEnfrocerDir64 = _T("C:\\Program Files (x86)\\NextLabs\\Desktop Enforcer\\");
const TCHAR* g_szDefaultDesktopEnforcerDir32 = _T("C:\\Program Files\\NextLabs\\Desktop Enforcer\\");

const TCHAR* g_szOERegKey64 = _T("SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Outlook Enforcer");
const TCHAR* g_szOERegKey32 = _T("SOFTWARE\\NextLabs\\Compliant Enterprise\\Outlook Enforcer");

const TCHAR* g_szDERegKey64 = _T("SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Desktop Enforcer");
const TCHAR* g_szDERegKey32 = _T("SOFTWARE\\NextLabs\\Compliant Enterprise\\Desktop Enforcer");

const TCHAR* g_szAdobeReaderRegKey64 = _T("SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader\\");
const TCHAR* g_szAdobeReaderRegKey32 = _T("SOFTWARE\\Adobe\\Acrobat Reader\\");

const TCHAR* g_szAdobeAcrobatRegKey64 = _T("SOFTWARE\\Wow6432Node\\Adobe\\Adobe Acrobat\\");
const TCHAR* g_szAdobeAcrobatRegKey32 = _T("SOFTWARE\\Adobe\\Adobe Acrobat\\");

const TCHAR* g_szAdobeVersion[] = { _T("9.0"), _T("10.0"), _T("11.0") };

const TCHAR* g_szProdcutCodeRegKey64 = _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
const TCHAR* g_szProdcutCodeRegKey32 = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
const TCHAR* g_szProdcutPCCodeRegKey64 = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");


const TCHAR* g_szDefaultPCInstallDir64 = _T("C:\\Program Files (x86)\\NextLabs\\Policy Controller\\");
const TCHAR* g_szDefaultPCInstallDir32 = _T("C:\\Program Files\\NextLabs\\Policy Controller\\");

const TCHAR* g_szDefaultNextLabsInstallDir64 = _T("C:\\Program Files (x86)\\NextLabs\\");
const TCHAR* g_szDefaultNextLabsInstallDir32 = _T("C:\\Program Files\\NextLabs\\");


const TCHAR* g_szPCRegKey64 = _T("SOFTWARE\\Wow6432Node\\NextLabs\\Compliant Enterprise\\Policy Controller");
const TCHAR* g_szPCRegKey32 = _T("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller");

const TCHAR* g_szNextLabsRegKey64 = _T("SOFTWARE\\Wow6432Node\\NextLabs");
const TCHAR* g_szNextLabsRegKey32 = _T("SOFTWARE\\NextLabs");


const TCHAR* g_szCommonRegKey64 = _T("SOFTWARE\\Wow6432Node\\NextLabs\\CommonLibraries");
const TCHAR* g_szCommonRegKey32 = _T("SOFTWARE\\NextLabs\\CommonLibraries");

const TCHAR* g_szDefaultCommonInstallDir64 = _T("C:\\Program Files (x86)\\NextLabs\\Common\\");
const TCHAR* g_szDefaultCommonInstallDir32 = _T("C:\\Program Files\\NextLabs\\Common\\");


const TCHAR* g_szDefaultPCDiagsInstallDir64 = _T("C:\\Program Files (x86)\\NextLabs\\diags\\");
const TCHAR* g_szDefaultPCDiagsInstallDir32 = _T("C:\\Program Files\\NextLabs\\diags\\");


const TCHAR* g_szDefaultPGBFile = _T("C:\\Nextlabs\\");


const TCHAR* g_szDefaultDriveTamperFile = _T("C:\\Windows\\System32\\drivers\\nl_tamper.sys");
const TCHAR* g_szDefaultDriveNLCCFile = _T("C:\\Windows\\System32\\drivers\\nlcc.sys");
const TCHAR* g_szDefaultDriveinjectionFile = _T("C:\\Windows\\System32\\drivers\\nlinjection.sys");




const TCHAR* g_szCurrentControlSetServicsRegKey = _T("SYSTEM\\CurrentControlSet\\services\\ComplianceEnforcerService");
const TCHAR* g_szCurrentControlSetMinimalServicsRegKey = _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\ComplianceEnforcerService");
const TCHAR* g_szCurrentControlSetNetworkServicsRegKey = _T("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\ComplianceEnforcerService");

const TCHAR* g_szControlSet001ServicsRegKey = _T("SYSTEM\\ControlSet001\\services\\ComplianceEnforcerService");
const TCHAR* g_szControlSet001MinimalServicsRegKey = _T("SYSTEM\\ControlSet001\\Control\\SafeBoot\\Minimal\\ComplianceEnforcerService");
const TCHAR* g_szControlSet001NetworkServicsRegKey = _T("SYSTEM\\ControlSet001\\Control\\SafeBoot\\Network\\ComplianceEnforcerService");

const TCHAR* g_szControlSet002ServicsRegKey = _T("SYSTEM\\ControlSet002\\services\\ComplianceEnforcerService");
const TCHAR* g_szControlSet002MinimalServicsRegKey = _T("SYSTEM\\ControlSet002\\Control\\SafeBoot\\Minimal\\ComplianceEnforcerService");
const TCHAR* g_szControlSet002NetworkServicsRegKey = _T("SYSTEM\\ControlSet002\\Control\\SafeBoot\\Network\\ComplianceEnforcerService");

class ComponentMgr
{
public:
	ComponentMgr();
	~ComponentMgr();

protected:
	ComponentMgr(const ComponentMgr&);
	std::list<TString> m_lstComponentFileName;
};

int _tmain(int argc, _TCHAR* argv[])
{
	int nRet = ERROR_SUCCESS;
	//extrace compo//depot/Fate/main/install/pcinstalltool/OEInstallPrep/src/OEInstallPrep.rcnent dependent
	ComponentMgr componentMgr;

	//parse command line
	ParseCommandLine(argc, argv);

	//get oe directory
	g_strOEInstallDir = GetOEInstallDir();
	if (g_strOEInstallDir.length()<=0)
	{
		OutputLogInfo(_T("Get OE install directory failed. user default path\n"));
	    g_strOEInstallDir = IsRunOn64BitWindows() ? g_szDefaultOEInstallDir64 : g_szDefaultOEInstallDir32;
	}
	wchar_t szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("OE install dir:%s\n"), g_strOEInstallDir.c_str());
	OutputLogInfo(szLog);

	//stop pc
	if (StopPC())
	{
		OutputLogInfo(_T("StopPC failed. OEInstallPrep exit.\n"));
		return ERROR_SUCCESS;
	}
	 
	//delete oe file
	if (DeleteOEFile() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	//delete adobe component
	if (DeleteAdobeComponent() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	//delete Destop Enforcer
	if (DeleteDestopEnforcer() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	CloseNLContextMgr();
	//delete PC File
	if (DeletePCFile() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	//delete PC env parameter
	RemoveEnv();

	//delete register info
	DeleteRegister();

	
	//delete service
	if (DeleteService() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	//delete Drive file
	if (RemoveDriveFile() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	//delete link file
	if (RemoveLinkFile() == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	//delete MSI install information
	RemoveInstallInfo(OE_PRODUCT);

	RemoveInstallInfo(PC_PRODUCT);


	if (nRet == ERROR_SUCCESS_REBOOT_REQUIRED&&g_bWriteRoot)
	{
		OutputDebugString(L"start write registy table!!\n");
		WriteRegValue();
	}

#if 0


	if (IDYES == MessageBox(NULL,L"In order to unInstall file, you had better restart machine, please click YES to start!\n",L"NextLabs",MB_YESNO))
	{
		ShellExecute(NULL,_T("open"),L"shutdown.exe",L" -r -t 0",NULL,SW_HIDE);
	}
	
#endif

	_stprintf_s(szLog, _T("OEInstallPrep finish !! the return value is :%s,The value is [%d]\n"), nRet == ERROR_SUCCESS ? L"ERROR_SUCCESS" : L"ERROR_SUCCESS_REBOOT_REQUIRED",nRet);
	OutputLogInfo(szLog);
	return nRet;
}





void ParseCommandLine(int argc, _TCHAR* argv[])
{
	g_bWriteRoot = false;
	for (int i = 1; i < argc; i++)
	{
		TString strParam = argv[i];
		if (_tcsicmp(strParam.c_str(), _T("-m")) == 0)
		{
			if (argc>i+1)
			{
				g_strMode = argv[i + 1];
				i++;
			}

		}
		else if (_tcsicmp(strParam.c_str(), _T("-p")) == 0)
		{
			if (argc > i + 1)
			{
				g_strPCPassword = argv[i + 1];
				i++;
			}
		}
		else if(_tcsicmp(strParam.c_str(), _T("-writeregistry")) == 0)
		{
			g_bWriteRoot = true;
		}
	}

	//output command line
	TCHAR szLog[100];
	_stprintf_s(szLog, _T("Command: mode=%s, pass=%s\n"), g_strMode.c_str(), g_strPCPassword.c_str());
	OutputLogInfo(szLog);

}

int StopPC()
{
	
	TString strCommandLine;
	if (IsSilentMode())
	{
		strCommandLine = _T("-p ");
		strCommandLine += g_strPCPassword;
	}
	int nRet = EnforcerStop(GetModuleHandle(NULL), L"-p 82q0958213434128503412uir0euwqr0urlksdajf", SW_SHOW);

	if (nRet == 1603)
	{
		nRet = EnforcerStop(GetModuleHandle(NULL), strCommandLine.c_str(), SW_SHOW);
	}
	return nRet;

}

bool IsSilentMode()
{
	return (_tcsicmp(g_strMode.c_str(), _T("silent")) == 0) && (g_strPCPassword.length() > 0);
}

void OutputLogInfo(const TCHAR * szLog)
{
	int nLogLen = _tcslen(szLog) + 100;
	TCHAR* szOutput = new TCHAR[nLogLen + 1];
	_stprintf_s(szOutput, nLogLen, _T("OEInstallPrep:%s"), szLog);
	OutputDebugString(szOutput);
	delete[] szOutput;
	szOutput = NULL;
	//_tprintf(szLog);
}

int DeleteOEFile()
{
	int nRet = ERROR_SUCCESS;

	//unregister component
	OutputLogInfo(_T("Begin Unregister OE component.\n"));
	TString strOeDllPath = g_strOEInstallDir + _T("bin\\mso2013PEP32.dll");
	UnregisterComponent(strOeDllPath.c_str());

	TString strCEOfficeDllPath = g_strOEInstallDir + _T("bin\\CEOffice32.dll");
	UnregisterComponent(strCEOfficeDllPath.c_str());

	TString strCEExplorerPath32 = g_strOEInstallDir + _T("bin\\CE_Explorer32.dll");
	UnregisterComponent(strCEExplorerPath32.c_str());

	TString strCEExplorerPath64 = g_strOEInstallDir + _T("bin\\CE_Explorer.dll");
	UnregisterComponent(strCEExplorerPath64.c_str());


	//delete file
	OutputLogInfo(_T("Begin Delete OE files:\n"));
	if (DeleteDirectory(g_strOEInstallDir.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	//64bits. Outlook install will copy some PC files to "C:\\Program Files (x86)\\NextLabs\\
	//so we will delete it
	if (IsRunOn64BitWindows())
	{
		TString strNextLabsDirectory = g_strOEInstallDir.substr(0,g_strOEInstallDir.rfind(L"\\Outlook Enforcer") + 1);
		if (DeleteDirectory(strNextLabsDirectory.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
		{
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
		}
	}

	return nRet;
}




int DeleteService()
{
	int nRet = ERROR_SUCCESS;
	TCHAR szLog[MAX_PATH * 2];
	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szCurrentControlSetServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szCurrentControlSetServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szCurrentControlSetNetworkServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szCurrentControlSetNetworkServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szCurrentControlSetMinimalServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szCurrentControlSetMinimalServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet001ServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet001ServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet001MinimalServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet001MinimalServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet001NetworkServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet001NetworkServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	
	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet002ServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet002ServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet002MinimalServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet002MinimalServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (RegDeleteTree(HKEY_LOCAL_MACHINE, g_szControlSet002NetworkServicsRegKey) == ERROR_SUCCESS)
	{
		_stprintf_s(szLog, _T("Delete Register:%s\n"), g_szControlSet002NetworkServicsRegKey);
		OutputLogInfo(szLog);
		if (nRet != ERROR_SUCCESS_REBOOT_REQUIRED)
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	return nRet;
}

void DeleteRegister()
{
	//OE

	bool bIs64bits = IsRunOn64BitWindows();
	const TCHAR* szOERegister = bIs64bits ? g_szOERegKey64 : g_szOERegKey32;
	RegDeleteTree(HKEY_LOCAL_MACHINE, szOERegister);

	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("Delete Register:%s\n"), szOERegister);
	OutputLogInfo(szLog);

	//Desktop enforcer
	const TCHAR* szDERegister = bIs64bits ? g_szDERegKey64 : g_szDERegKey32;
	RegDeleteTree(HKEY_LOCAL_MACHINE, szDERegister);

	_stprintf_s(szLog, _T("Delete Register:%s\n"), szDERegister);
	OutputLogInfo(szLog);



	//NextLabs
	const TCHAR* szNextLabsRegister = bIs64bits ? g_szNextLabsRegKey64 : g_szNextLabsRegKey32;
	
	RegDeleteTree(HKEY_LOCAL_MACHINE, szNextLabsRegister);
	_stprintf_s(szLog, _T("Delete Register:%s\n"), szNextLabsRegister);
	OutputLogInfo(szLog);
	
	if (bIs64bits)
	{
		long lRegOpen = ERROR_SUCCESS;
		HKEY hUninstallKey = NULL;
		lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE", 0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, &hUninstallKey);
		if (lRegOpen == ERROR_SUCCESS&&hUninstallKey != NULL)
		{
			RegDeleteTree(hUninstallKey,L"NextLabs");
			_stprintf_s(szLog, _T("Delete Register:HKEY_LOCAL_MACHINE\\SOFTWARE\\NextLabs  \n"));
			OutputLogInfo(szLog);
		}
	}


}


TString GetOEInstallDir()
{
	TString strInstallDir;

	const TCHAR* szOeRegisterKey = IsRunOn64BitWindows() ? g_szOERegKey64 : g_szOERegKey32;
	HKEY hOEKey = NULL;
	long lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szOeRegisterKey, 0, KEY_READ, &hOEKey);
	if ((lRegOpen == ERROR_SUCCESS) && (hOEKey != NULL))
	{
		DWORD dwType;
		TCHAR szDir[MAX_PATH];
		DWORD dwcbDirLen = MAX_PATH * sizeof(TCHAR);
		long lRegGetValue = RegGetValue(hOEKey, NULL, _T("InstallDir"), RRF_RT_REG_SZ, &dwType, szDir, &dwcbDirLen);

		if ((lRegGetValue == ERROR_SUCCESS))
		{
			szDir[dwcbDirLen / sizeof(TCHAR)] = 0;
			strInstallDir = szDir;
		}
		else
		{
			TCHAR szLog[200];
			_stprintf_s(szLog, _T("RegGetValue failed. LastError=%d\n"), GetLastError());
			OutputLogInfo(szLog);
		}

		RegCloseKey(hOEKey);
		hOEKey = NULL;
	}
	else
	{
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("RegOpenKeyEx failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
	}

	return strInstallDir;
}

int DeleteDirectory(const TCHAR* szDir)
{
	int nRet = ERROR_SUCCESS;
	WIN32_FIND_DATA findData;
	ZeroMemory(&findData, sizeof(findData));

	//delete the file of the directory
	TString strFindName = szDir;
	strFindName += _T("*.*");
	HANDLE hFindFile = FindFirstFile(strFindName.c_str(), &findData);
	while (hFindFile != INVALID_HANDLE_VALUE)
	{
		if ((findData.cFileName[0] != _T('.')))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				TString strDirName = szDir;
				strDirName += findData.cFileName;
				strDirName += _T("\\");
				if (DeleteDirectory(strDirName.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
				{
					nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
				}
			}
			else
			{
					TString strFileFullName = szDir;
					strFileFullName += findData.cFileName;
					if (OEPluginDeleteFile(strFileFullName.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
					{
						nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
					}
			}
		}
	
		BOOL bFindNext = FindNextFile(hFindFile, &findData);
		if (!bFindNext)
		{
			break;
		}
	}

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		hFindFile = INVALID_HANDLE_VALUE;
	}

	//delete the directory itself
	BOOL bDeleteDir = RemoveDirectory(szDir);
	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("RemoveDirectory:%s, Result:%s, lastError=%d\n"), szDir, bDeleteDir ? _T("Success") : _T("Failed"), GetLastError());
	OutputLogInfo(szLog);

	if (!bDeleteDir)
	{
		if (!MoveFileEx(szDir,NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
		{
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
			_stprintf_s(szLog, _T("MoveFileEx:%s, lastError=%d fail\n"), szDir,  GetLastError());
			OutputLogInfo(szLog);
		}
		else
		{
			_stprintf_s(szLog, _T("MoveFileEx:%s, lastError=%d success\n"), szDir,  GetLastError());
			OutputLogInfo(szLog);
		}
	}

	return nRet;
}

void UnregisterComponent(const TCHAR* szComponent)
{
	//log
	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("Unregister:%s\n"), szComponent);
	OutputLogInfo(szLog);

	bool bRet = true;

	const TCHAR* szRegsvr32Path = _T("C:\\Windows\\System32\\regsvr32.exe");
	TCHAR szCommandLine[2 * MAX_PATH];
	_stprintf_s(szCommandLine, _T("\"%s\" /u /s \"%s\""), szRegsvr32Path, szComponent);
	
	//start PCStop.exe
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	BOOL bCreateProcess = CreateProcess(NULL, szCommandLine, NULL, NULL, false, 0, NULL, NULL, &startupInfo, &processInfo);

	if (!bCreateProcess)
	{
		_stprintf_s(szLog, _T("CreateProcess for Regsvr32.exe failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
		bRet = false;
	}
	else
	{
		//wait PCStop.exe to exit
		//OutputLogInfo(_T("Start Regsvr32.exe success.\nNow wait Regsvr32.exe to exit...\n"));
		WaitForSingleObject(processInfo.hProcess, 30 * 1000);
		//OutputLogInfo(_T("Regsvr32.exe returned.\n"));
		//close handle
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
}


bool IsRunOn64BitWindows()
{
	//check if the application is 32 or 64 bit.
	const void * const p  = NULL;
	if (sizeof(p)==4)
	{
		//32 bit
		BOOL bWow64 = FALSE;
		IsWow64Process(GetCurrentProcess(), &bWow64);
		return bWow64;
	}
	else 
	{
		return true;
	}
}

TString GetAdobeReaderInstallPath(const TCHAR* szVersion)
{
	TString strReaderRegisterKey = IsRunOn64BitWindows() ? g_szAdobeReaderRegKey64 : g_szAdobeReaderRegKey32;
	strReaderRegisterKey += szVersion;
	strReaderRegisterKey += _T("\\InstallPath");
	TString strPath = GetRegValueString(HKEY_LOCAL_MACHINE, strReaderRegisterKey.c_str(), NULL);

	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("AdobeReader %s, InstallPath:%s\n"), szVersion, strPath.c_str());
	OutputLogInfo(szLog);

	return strPath;
}

TString GetAdobeAcrobatInstallPath(const TCHAR* szVersion)
{
	TString strAcrobatRegisterKey = IsRunOn64BitWindows() ? g_szAdobeAcrobatRegKey64 : g_szAdobeAcrobatRegKey32;
	strAcrobatRegisterKey += szVersion;
	strAcrobatRegisterKey += _T("\\InstallPath");
	TString strPath = GetRegValueString(HKEY_LOCAL_MACHINE, strAcrobatRegisterKey.c_str(), NULL);

	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("AdobeReader %s, InstallPath:%s\n"), szVersion, strPath.c_str());
	OutputLogInfo(szLog);

	return strPath;
}
TString GetOEInstallPrepPath()
{
	TCHAR szPrepPath[MAX_PATH + 1] = { 0 };
	DWORD dwRst = GetModuleFileName(NULL, szPrepPath, MAX_PATH);

	if (dwRst > 0)
	{
		szPrepPath[dwRst] = 0;

		TCHAR szLog[200];
		_stprintf_s(szLog, _T("GetModuleFileName success:%s\n"), szPrepPath );
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

int DeleteAdobeComponent()
{
	OutputLogInfo(_T("Begin delete Adobe files.\n"));
	int nRet = ERROR_SUCCESS;
	//adobe reader plugins
	TCHAR szLog[MAX_PATH * 2];
	int nAdobeVersinCount = sizeof(g_szAdobeVersion) / sizeof(g_szAdobeVersion[0]);
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strReaderInstappPath = GetAdobeReaderInstallPath(g_szAdobeVersion[i]);
		if (strReaderInstappPath.length())
		{
			TString strReaderPlugin = strReaderInstappPath + _T("\\plug_ins\\CE_Reader32.api");
			if (OEPluginDeleteFile(strReaderPlugin.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
			{
				nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
			}
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Reader install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLogInfo(szLog);
		}

	}

	//adobe acrobat
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strAcrobatInstappPath = GetAdobeAcrobatInstallPath(g_szAdobeVersion[i]);
		if (strAcrobatInstappPath.length())
		{
			TString strAcrobatPlugin = strAcrobatInstappPath + _T("\\plug_ins\\CE_Acrobat32.api");
			if (OEPluginDeleteFile(strAcrobatPlugin.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
			{
				nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
			}
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Acrobat install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLogInfo(szLog);
		}
	}
	return nRet;
}


//get product code of OE. we use this to clean the msi install information

TString GetProductCode(emProductType ProductType)
{

	WCHAR szLog[1024] = {0};
	_stprintf_s(szLog, _T("CGet Product Code ProductType is [%d]\n"), ProductType);
	OutputLogInfo(szLog);

	TString strProductCode;
	const TCHAR* strUninstallRegKey = NULL;
	
	if (ProductType == OE_PRODUCT)
	{
		strUninstallRegKey = IsRunOn64BitWindows() ? g_szProdcutCodeRegKey64 : g_szProdcutCodeRegKey32;
	}
	
	if(ProductType == PC_PRODUCT)
	{
		strUninstallRegKey = IsRunOn64BitWindows() ? g_szProdcutPCCodeRegKey64 : g_szProdcutCodeRegKey32;
	}


	OutputDebugString(strUninstallRegKey);
	HKEY hUninstallKey = NULL;


	long lRegOpen = ERROR_SUCCESS;
	if (IsRunOn64BitWindows()&&ProductType == PC_PRODUCT)
	{
		lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strUninstallRegKey, 0, KEY_READ|KEY_WOW64_64KEY, &hUninstallKey);
	}
	else
	{
		lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strUninstallRegKey, 0, KEY_READ, &hUninstallKey);
	}
	
	if ((lRegOpen == ERROR_SUCCESS) && (hUninstallKey != NULL))
	{
		
		DWORD dwIndex = 0;
		while (true)
		{
			DWORD dwCCName = 400;
			TCHAR szName[400];
			LONG lRegEnumResult = RegEnumKeyEx(hUninstallKey, dwIndex++, szName, &dwCCName, 0, NULL, NULL, NULL);
					
			if (lRegEnumResult == ERROR_NO_MORE_ITEMS)
			{
				
				break;
			}
			else if (lRegEnumResult == ERROR_SUCCESS)
			{

				DWORD dwType;
				TCHAR szDisplayName[MAX_PATH];
				DWORD dwcbDisplayNameLen = MAX_PATH * sizeof(TCHAR);
				long lRegGetValue = RegGetValue(hUninstallKey, szName, _T("DisplayName"), RRF_RT_REG_SZ, &dwType, szDisplayName, &dwcbDisplayNameLen);
	
				if (lRegGetValue == ERROR_SUCCESS)
				{
					if(ProductType == OE_PRODUCT)
					{	
						if (_tcsicmp(szDisplayName, _T("OutlookEnforcer")) == 0)
						{
							strProductCode = szName;

							_stprintf_s(szLog, _T("The product code of OE:%s\n"), szName);
							OutputLogInfo(szLog);
							break;
						}
					}

					if(ProductType == PC_PRODUCT)
					{	
						if (_tcsicmp(szDisplayName, _T("NextLabs Policy Controller")) == 0)
						{
							strProductCode = szName;

							_stprintf_s(szLog, _T("The product code of OE:%s\n"), szName);
							OutputLogInfo(szLog);
							break;
						}
					}
					
				}

			}

		}
		
		RegCloseKey(hUninstallKey);
		hUninstallKey = NULL;
	}
	else
	{
		_stprintf_s(szLog, _T("RegOpenKeyEx failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
	}


	return strProductCode;

}

void RemoveInstallInfo(emProductType ProductType)
{
	TCHAR szLog[200];
	_stprintf_s(szLog, _T("Begin Remove Install information. ProductType is =%d\n"), ProductType);
	OutputLogInfo(szLog);

	TString strProductCode = GetProductCode(ProductType);

	if (strProductCode.length() > 0)
	{
		TString strCommandLine = /*GetOEInstallPrepPath()*/GetTempDirectory() + _T("MSIZap.exe");
		strCommandLine += _T(" T! ") + strProductCode;
	
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(processInfo));

		TCHAR szCommand[MAX_PATH];
		_stprintf_s(szCommand, strCommandLine.c_str());
		OutputLogInfo(strCommandLine.c_str());
		BOOL bCreateProcess = CreateProcess(NULL, szCommand, NULL, NULL, false, 0, NULL, NULL, &startupInfo, &processInfo);

		if (!bCreateProcess)
		{
			_stprintf_s(szLog, _T("CreateProcess for MSIZap.exe failed. LastError=%d\n"), GetLastError());
			OutputLogInfo(szLog);
		}
		else
		{
			//wait PCStop.exe to exit
			OutputLogInfo(_T("Start MSIZap.exe success.\nNow wait MSIZap.exe to exit...\n"));
			DWORD dwWait = WaitForSingleObject(processInfo.hProcess, 60 * 1000);
			if (dwWait == WAIT_OBJECT_0)
			{		
			    OutputLogInfo(_T("Clean OE install information success.\n"));		
			}
			else
			{
				OutputLogInfo(_T("Clean OE install information failed.\n"));
			}

			//close handle
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}
	}
	else
	{
		_stprintf_s(szLog, _T("Get product code of  failed. can't remove install information. ProductType is =%d\n"), ProductType);
		OutputLogInfo(szLog);

	}

}

bool FailedDeleteFile(const TCHAR* szFileFullName)
{
	bool bCancel = false;
	TString strFile = szFileFullName;

	if ( (strFile.find(_T("mso2013PEP32.dll"), 0) != TString::npos) ||
		(strFile.find(_T("adaptermanager32.dll"), 0) != TString::npos) || 
		(strFile.find(_T("boost_regex-vc90-mt-1_43.dll"), 0) != TString::npos) ||
		(strFile.find(_T("odhd201332.dll"), 0) != TString::npos) ||
		(strFile.find(_T("odhd201032.dll"), 0) != TString::npos )
		)
	{
		MessageBox(NULL, _T("Delete file failed. Please close Microsoft Outlook first, then click OK button to continue."), _T("Delete File Failed"), MB_OK);
	}
	else if ((strFile.find(_T("InjectExp32.dll"), 0) != TString::npos) ||
		     (strFile.find(_T("InjectExp.dll"), 0) != TString::npos) ||
		     (strFile.find(_T("CE_Explorer.dll"), 0) != TString::npos) ||
			 (strFile.find(_T("CE_Explorer32.dll"), 0) != TString::npos)
		   )
	{
		MessageBox(NULL, _T("Delete file failed. Please Close Windows Explorer first, then click OK button to continue."), _T("Delete File Failed"), MB_OK);
	}
	else if ((strFile.find(_T("CEOffice32.dll"), 0) != TString::npos) )
	{
		MessageBox(NULL, _T("Delete file failed. Please Close Microsoft Office(Word,Excel,PowerPoint) first, then click OK button to continue."), _T("Delete File Failed"), MB_OK);
	}
	else if ( (strFile.find(_T("CE_Reader32.api"),0) != TString::npos) ||
		      (strFile.find(_T("CE_Acrobat32.api"), 0) != TString::npos) ||
			  (strFile.find(_T("CE_AdobePEPTrm32.dll"), 0) != TString::npos)
		    )
	{
		MessageBox(NULL, _T("Delete file failed. Please Close Adobe Reader/Acrobat first, then click OK button to continue."), _T("Delete File Failed"), MB_OK);
	}
	else
	{
		bCancel = true;
	}

	


	return bCancel;
}

int  DeleteDestopEnforcer()
{
	int nRet = ERROR_SUCCESS;
	OutputLogInfo(_T("Begin Delete Destop Enforcer.\n"));
	//Terminate Desktop Enforcer
	DWORD dwProcessID = GetProcessID(PROC_NAME_EDPMANAGER);
	TCHAR szLog[200];
	_stprintf_s(szLog, _T("Desktop Enforcer ProcessID:%d\n"), dwProcessID);
	OutputLogInfo(szLog);

	//
	if (dwProcessID > 0)
	{
		HANDLE hDesktopEnforcer = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, dwProcessID);

		if (hDesktopEnforcer)
		{
			while (true)
			{
				BOOL bTerminate = TerminateProcess(hDesktopEnforcer, 0xffffffff);
				if (bTerminate)
				{
					OutputLogInfo(_T("Terminate Desktop Enforcer Success.\n"));
					OutputLogInfo(_T("Wait Desktop Enforcer to exit..."));
					WaitForSingleObject(hDesktopEnforcer, 60 * 60 * 1000);
					Sleep(10 * 1000);
					OutputLogInfo(_T("Desktop Enforcer to exit success."));
					break;
				}
				else
				{
					MessageBox(NULL, _T("Please close the Desktop Enforcer, then click OK button to continue.\n"), _T("Terminate Desktop Enforcer Failed"), MB_OK);
				}
			}
			
			CloseHandle(hDesktopEnforcer);
		}
	}
	else
	{
		OutputLogInfo(_T("Failed to get process ID of Desktop Enforcer.\n"));
	}


	//delete Desktop Enforcer file
	OutputLogInfo(_T("Delete Desktop Enforcer files...\n"));
	TString strDesktopEnforcerInstallDir = GetDesktopEnforcerInstallPath();
	if (strDesktopEnforcerInstallDir.length() <= 0)
	{
		strDesktopEnforcerInstallDir = IsRunOn64BitWindows() ? g_szDefaultDesktopEnfrocerDir64 : g_szDefaultDesktopEnforcerDir32;
	}
	if (DeleteDirectory(strDesktopEnforcerInstallDir.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (DeleteDirectory(g_szDefaultPGBFile) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}
	return nRet;

}

DWORD GetProcessID(emProcessType ProcType)
{

	DWORD ProcId = 0;
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		OutputDebugString(L"CreateToolhelp32Snapshot  fail!\n");
		return 0;
	}

	pe32.dwSize = sizeof( PROCESSENTRY32 );

	if( !Process32First( hProcessSnap, &pe32 ) )
	{
		CloseHandle( hProcessSnap );   

		return 0;
	}

	do
	{

		if (ProcType == PROC_NAME_EDPMANAGER)
		{
			if (_tcsstr(pe32.szExeFile, _T("edpmanager")))
			{
				ProcId = pe32.th32ProcessID;
				break;
			}
		}
		if (ProcType == PROC_NAME_NLCONTEXTMAGR)
		{
			if (_tcsstr(pe32.szExeFile, _T("nlcontextmgr")))
			{
				ProcId = pe32.th32ProcessID;
				break;
			}
		}


	} while( Process32Next( hProcessSnap, &pe32 ) );

	CloseHandle( hProcessSnap );
	return  ProcId;

}

TString GetDesktopEnforcerInstallPath()
{
	TString strInstallDir;

	const TCHAR* szDERegisterKey = IsRunOn64BitWindows() ? g_szDERegKey64 : g_szDERegKey32;
	HKEY hDEKey = NULL;
	long lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDERegisterKey, 0, KEY_READ, &hDEKey);
	if ((lRegOpen == ERROR_SUCCESS) && (hDEKey != NULL))
	{
		DWORD dwType;
		TCHAR szDir[MAX_PATH];
		DWORD dwcbDirLen = MAX_PATH * sizeof(TCHAR);
		long lRegGetValue = RegGetValue(hDEKey, NULL, _T("InstallDir"), RRF_RT_REG_SZ, &dwType, szDir, &dwcbDirLen);

		if ((lRegGetValue == ERROR_SUCCESS))
		{
			szDir[dwcbDirLen / sizeof(TCHAR)] = 0;
			strInstallDir = szDir;
		}
		else
		{
			TCHAR szLog[200];
			_stprintf_s(szLog, _T("RegGetValue failed. LastError=%d\n"), GetLastError());
			OutputLogInfo(szLog);
		}

		RegCloseKey(hDEKey);
		hDEKey = NULL;
	}
	else
	{
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("RegOpenKeyEx failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
	}

	return strInstallDir;
}

ComponentMgr::ComponentMgr()
{
	
	OutputLogInfo(_T("Extract Component...\n"));

	bool bExtractResult = true;

	DWORD dwComponentResID[] = {  IDR_RCDATA_MSIZAP };

	TString strComponentFileName[] = {_T("MSIZap.exe") };

	HMODULE hResModule = GetModuleHandle(NULL);
	TString strDestDir = GetTempDirectory();//GetOEInstallPrepPath();
	int nComponentCount = sizeof(dwComponentResID) / sizeof(dwComponentResID[0]);
	for (int iComponent = 0; iComponent < nComponentCount; iComponent++)
	{
		//
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("Extract:%s\n"), strComponentFileName[iComponent].c_str());
		OutputLogInfo(szLog);

		HRSRC hResource = FindResource(hResModule, MAKEINTRESOURCE(dwComponentResID[iComponent]), RT_RCDATA);
		if (NULL == hResource)
		{
			bExtractResult = false;
			break;
		}
		HGLOBAL hResGlobal = LoadResource(hResModule, hResource);
		if (hResGlobal == NULL)
		{
			return ;
		}
		LPVOID pResData = LockResource(hResGlobal);
		DWORD dwResSize = SizeofResource(hResModule, hResource);

		TString strFileName = strDestDir + strComponentFileName[iComponent];
#pragma  warning(push)
#pragma  warning(disable: 4996)
		FILE* pFileRes = _tfopen(strFileName.c_str(), _T("wb"));
#pragma	 warning(pop)
		if (NULL == pFileRes)
		{
			bExtractResult = false;
			OutputLogInfo(_T("Extrace component file failed.\n"));
			break;
		}
		fwrite(pResData, 1, dwResSize, pFileRes);
		fflush(pFileRes);
		fclose(pFileRes);
		pFileRes = NULL;

		m_lstComponentFileName.push_back(strFileName);
	}
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

int OEPluginDeleteFile(const TCHAR* szFile)
{
	int nRet = ERROR_SUCCESS;
	if (szFile == NULL)
	{
		return nRet;
	}
	
	if (!PathFileExists(szFile))
	{
		return nRet;
	}
	
	TCHAR szLog[MAX_PATH * 2];
	BOOL bDelete = DeleteFile(szFile);
	if (bDelete)
	{
		_stprintf_s(szLog, _T("Delete file:%s success.\n"), szFile);
		OutputLogInfo(szLog);
		return nRet;
	}
	else
	{
		_stprintf_s(szLog, _T("DeleteFile file:%s failed. last Error=%d\n"), szFile, GetLastError());
		OutputLogInfo(szLog);
	}



	bDelete = MoveFileEx(szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	if (bDelete)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
		_stprintf_s(szLog, _T("MoveFileEx file:%s success. we use MoveFileEx with MOVEFILE_DELAY_UNTIL_REBOOT flag to delay delete it when reboot.\n"), szFile);
		OutputLogInfo(szLog);
	}
	else
	{
		_stprintf_s(szLog, _T("MoveFileEx file:%s failed. last Error=%d\n"), szFile, GetLastError());
		OutputLogInfo(szLog);
	}
	return nRet;
}

TString GetRegValueString(HKEY hkey, LPCTSTR lpSubKey, LPCTSTR lpValueName)
{
	TString strValue;
	DWORD dwCBData = 0;
	long lGetValueResult = RegGetValue(hkey, lpSubKey, lpValueName, RRF_RT_REG_SZ, NULL, NULL, &dwCBData);
	if ((lGetValueResult == ERROR_SUCCESS) && dwCBData)
	{
		TCHAR* szValue = new TCHAR[dwCBData / 2 + 1];
		lGetValueResult = RegGetValue(hkey, lpSubKey, lpValueName, RRF_RT_REG_SZ, NULL, szValue, &dwCBData);
		if (lGetValueResult == ERROR_SUCCESS)
		{
			szValue[dwCBData / 2] = 0;
			strValue = szValue;
		}

		delete[] szValue;
		szValue = NULL;
	}

	return strValue;
}

TString GetTempDirectory()
{
	DWORD dwLen = GetTempPath(0, NULL);

	if (dwLen)
	{
		TCHAR* szTempPath = new TCHAR[dwLen+1];

		dwLen = GetTempPath(dwLen, szTempPath);
		szTempPath[dwLen] = 0;

		TString strTempPath = szTempPath;
		delete[] szTempPath;
		szTempPath = NULL;

		return strTempPath;
	}

	return _T("");
	
}



TString GetPCInstallPath(emPolicyDirType DirType)
{
	TString strInstallDir;

	const TCHAR* szPCRegisterKey = IsRunOn64BitWindows() ? g_szPCRegKey64 : g_szPCRegKey32;
	HKEY hDEKey = NULL;
	long lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPCRegisterKey, 0, KEY_READ, &hDEKey);
	if ((lRegOpen == ERROR_SUCCESS) && (hDEKey != NULL))
	{
		DWORD dwType;
		TCHAR szDir[MAX_PATH];
		DWORD dwcbDirLen = MAX_PATH * sizeof(TCHAR);

		long lRegGetValue = ERROR_SUCCESS;
		if (DirType == NEXTLABS_DIRECTORY)
		{
			lRegGetValue = RegGetValue(hDEKey, NULL, _T("InstallDir"), RRF_RT_REG_SZ, &dwType, szDir, &dwcbDirLen);
		}
		else
		{
			lRegGetValue = RegGetValue(hDEKey, NULL, _T("PolicyControllerDir"), RRF_RT_REG_SZ, &dwType, szDir, &dwcbDirLen);
		}	

		if ((lRegGetValue == ERROR_SUCCESS))
		{
			szDir[dwcbDirLen / sizeof(TCHAR)] = 0;
			strInstallDir = szDir;
		}
		else
		{
			TCHAR szLog[200];
			_stprintf_s(szLog, _T("RegGetValue failed. LastError=%d\n"), GetLastError());
			OutputLogInfo(szLog);
		}

		RegCloseKey(hDEKey);
		hDEKey = NULL;
	}
	else
	{
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("RegOpenKeyEx failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
	}

	return strInstallDir;
}



int DeletePCFile()
{
	int nRet = ERROR_SUCCESS;
	OutputLogInfo(_T("Begin Delete PC Folder.\n"));
	emPolicyDirType DirType = NEXTLABS_DIRECTORY;
	TString strNextLabsInstallDir = GetPCInstallPath(DirType);
	if (strNextLabsInstallDir.length() <= 0)
	{
		strNextLabsInstallDir = IsRunOn64BitWindows() ? g_szDefaultNextLabsInstallDir64 : g_szDefaultNextLabsInstallDir32;
	}
	if (DeleteDirectory(strNextLabsInstallDir.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}
	return nRet;

}


TString GetCommonInstallPath()
{
	TString strInstallDir;

	const TCHAR* szCommonRegisterKey = IsRunOn64BitWindows() ? g_szCommonRegKey64 : g_szCommonRegKey32;
	HKEY hDEKey = NULL;
	long lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCommonRegisterKey, 0, KEY_READ, &hDEKey);
	if ((lRegOpen == ERROR_SUCCESS) && (hDEKey != NULL))
	{
		DWORD dwType;
		TCHAR szDir[MAX_PATH];
		DWORD dwcbDirLen = MAX_PATH * sizeof(TCHAR);
		long lRegGetValue = RegGetValue(hDEKey, NULL, _T("InstallDir"), RRF_RT_REG_SZ, &dwType, szDir, &dwcbDirLen);

		if ((lRegGetValue == ERROR_SUCCESS))
		{
			szDir[dwcbDirLen / sizeof(TCHAR)] = 0;
			strInstallDir = szDir;
		}
		else
		{
			TCHAR szLog[200];
			_stprintf_s(szLog, _T("RegGetValue failed. LastError=%d\n"), GetLastError());
			OutputLogInfo(szLog);
		}

		RegCloseKey(hDEKey);
		hDEKey = NULL;
	}
	else
	{
		TCHAR szLog[200];
		_stprintf_s(szLog, _T("RegOpenKeyEx failed. LastError=%d\n"), GetLastError());
		OutputLogInfo(szLog);
	}

	return strInstallDir;
}



int DeleteCommonFile()
{
	OutputLogInfo(_T("Begin Delete PC Folder.\n"));
	int nRet = ERROR_SUCCESS;
	TString strCommonInstallDir = GetCommonInstallPath();
	if (strCommonInstallDir.length() <= 0)
	{
		strCommonInstallDir = IsRunOn64BitWindows() ? g_szDefaultCommonInstallDir64 : g_szDefaultCommonInstallDir32;
	}
	if (DeleteDirectory(strCommonInstallDir.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}


	if (!strCommonInstallDir.empty()&& strCommonInstallDir.rfind(L"\\Common") != TString::npos)
	{
		TString strNextLabs = strCommonInstallDir.substr(0,strCommonInstallDir.rfind(L"\\Common")+1);
		TString strDiags = strNextLabs + L"diags\\";
		if (DeleteDirectory(strDiags.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
		{
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
		}

		TString strBackUp = strNextLabs + L"backup\\";
		if (DeleteDirectory(strBackUp.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
		{
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
		}
	}
	return nRet;
}




void RemoveEnv()
{
	OutputDebugString(L"Start RemoveEnv ............\n");
	
	TString strPCInstallDir = GetPCInstallPath(PC_DIRECTORY);
	if (strPCInstallDir.length() <= 0)
	{
		strPCInstallDir = IsRunOn64BitWindows() ? g_szDefaultPCInstallDir64 : g_szDefaultPCInstallDir32;
	}

	strPCInstallDir += L"bin\\";

	TString strEnvName = L"Path";
	RemoveEnvValue(strEnvName,strPCInstallDir);
	
}




BOOL RemoveEnvValue(const TString& strEnvName, const TString &strEnvValue)
{
	TString strKey = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";

	HKEY hKey;
	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey.c_str(), 0L, KEY_ALL_ACCESS, &hKey);
	if ((lRet != ERROR_SUCCESS) || (hKey == NULL))
	{
		wchar_t strlog[MAX_PATH] = {0};
		StringCbPrintf(strlog,MAX_PATH,L"RegOpenKeyEx key is [%s] fail! The LastError is [%d]\n",strKey.c_str(),lRet);
		::OutputDebugString(strlog);
		return FALSE;
	}

	DWORD dwType = 0, dw = 0;
	lRet = RegQueryValueEx(hKey, strEnvName.c_str(), NULL, &dwType, NULL, &dw);
	if ((lRet != ERROR_SUCCESS))
	{
		wchar_t strlog[MAX_PATH] = {0};
		StringCbPrintf(strlog,MAX_PATH,L"RegQueryValueEx key is [%s] fail! The LastError is [%d]\n",strEnvName.c_str(),lRet);
		::OutputDebugString(strlog);
		return FALSE;
	}

	TCHAR *szEnvValue = new TCHAR[dw + 1];
	ZeroMemory(szEnvValue,sizeof(TCHAR)*(dw+1));
	lRet = RegQueryValueEx(hKey, strEnvName.c_str(), NULL, &dwType, (BYTE*)szEnvValue, &dw);
	if ((lRet != ERROR_SUCCESS))
	{
		wchar_t strlog[MAX_PATH] = {0};
		StringCbPrintf(strlog,MAX_PATH,L"RegQueryValueEx key is [%s] fail! The LastError is [%d]\n",strEnvName.c_str(),lRet);
		::OutputDebugString(strlog);
		delete []szEnvValue;
		return FALSE;
	}

	TString strReplaceValue = L";";
	strReplaceValue += strEnvValue;
	TString strCurrentEnvValue = szEnvValue;
	delete []szEnvValue;

	wchar_t strlg[5000] = {0};
	StringCbPrintf(strlg,5000,L"strCurrentEnvValue is [%s],strReplaceValue is [%s] ----\n",strCurrentEnvValue.c_str(),strReplaceValue.c_str());
	::OutputDebugString(strlg);
	boost::replace_all(strCurrentEnvValue,strReplaceValue,L"");
	lRet = RegSetValueEx(hKey, strEnvName.c_str(), NULL, REG_EXPAND_SZ,
		(BYTE * const)(LPCSTR)strCurrentEnvValue.c_str(), (strCurrentEnvValue.length()+1)*sizeof(TCHAR));
	if ((lRet != ERROR_SUCCESS))
	{
		wchar_t strlog[MAX_PATH] = {0};
		StringCbPrintf(strlog,MAX_PATH,L"RegSetValueEx key is [%s] fail! The LastError is [%d]\n",strCurrentEnvValue.c_str(),lRet);
		::OutputDebugString(strlog);
		return FALSE;
	}

	RegCloseKey(hKey);

	DWORD dwRet;
	SendMessageTimeout(HWND_BROADCAST,WM_SETTINGCHANGE,0,
		(LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000,&dwRet);

	return TRUE;
}


int RemoveDriveFile()
{
	int nRet = ERROR_SUCCESS;
	PVOID OldValue = NULL;
	bool bOS64bits =  IsRunOn64BitWindows();
	if (bOS64bits)
	{
		Wow64DisableWow64FsRedirection(&OldValue);
	}

	if (OEPluginDeleteFile(g_szDefaultDriveTamperFile) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}
	if (OEPluginDeleteFile(g_szDefaultDriveNLCCFile) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}
	if (OEPluginDeleteFile(g_szDefaultDriveinjectionFile) == ERROR_SUCCESS_REBOOT_REQUIRED)
	{
		nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
	}

	if (bOS64bits)
	{
		Wow64RevertWow64FsRedirection(OldValue);
	}
	return nRet;
}


int RemoveLinkFile()
{
	int nRet = ERROR_SUCCESS;
	TCHAR szPath[MAX_PATH] = {0};  
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, szPath);
	if (SUCCEEDED(hr))
	{
		TString strFilePath(szPath);
		strFilePath += L"\\NextLabs\\";
		if (DeleteDirectory(strFilePath.c_str()) == ERROR_SUCCESS_REBOOT_REQUIRED)
		{
			nRet = ERROR_SUCCESS_REBOOT_REQUIRED;
		}
	}
	else
	{
		::OutputDebugString(L"SHGetFolderPath fail! \n");
	}
	return nRet;
}

void CloseNLContextMgr()
{
	DWORD ProcessId = GetProcessID(PROC_NAME_NLCONTEXTMAGR);
	CloseProc(ProcessId);
}


void CloseProc(DWORD ProcessId)
{
	if (ProcessId > 0)
	{
		HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);

		if (hProc)
		{
			while (true)
			{
				BOOL bTerminate = TerminateProcess(hProc, 0xffffffff);
				if (bTerminate)
				{
					OutputLogInfo(_T("Terminate nlcontextmgr Success.\n"));
					OutputLogInfo(_T("Wait nlcontextmgr to exit..."));
					WaitForSingleObject(hProc, 60 * 60 * 1000);
					Sleep(10 * 1000);
					OutputLogInfo(_T("nlcontextmgr to exit success."));
					break;
				}
				else
				{
					OutputDebugString(L"close the nlcontextmgr.exe fail");
				}
			}

			CloseHandle(hProc);
		}
		else
		{	
			OutputDebugString(L"Close process fail CloseNLContextMgr!@");
		}
	}
	else
	{
		OutputLogInfo(_T("Failed to get process ID of nlcontextmgr.\n"));
	}

}

void WriteRegValue()
{
	HKEY hk = NULL;
	DWORD dwDisp;
	wchar_t strlog[MAX_PATH] = {0};
	int nValue = 1;

	if (IsRunOn64BitWindows())
	{
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\OEInstallTool", 0, NULL, REG_OPTION_NON_VOLATILE,KEY_WRITE|KEY_WOW64_64KEY, NULL, &hk, &dwDisp))
		{
			StringCbPrintf(strlog,MAX_PATH,L"RegCreateKeyEx key is HKEY_LOCAL_MACHINE\\SOFTWARE\\OEInstallTool fail! The LastError is [%d]\n",GetLastError());
			::OutputDebugString(strlog);
			return ;
		}
	}
	else
	{
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\OEInstallTool", 0, NULL, REG_OPTION_NON_VOLATILE,KEY_WRITE, NULL, &hk, &dwDisp))
		{
			StringCbPrintf(strlog,MAX_PATH,L"RegCreateKeyEx key is HKEY_LOCAL_MACHINE\\SOFTWARE\\OEInstallTool fail! The LastError is [%d]\n",GetLastError());
			::OutputDebugString(strlog);
			return ;
		}
	}

	

	if (RegSetValueEx(hk,L"NeedToReboot",0,REG_DWORD,(LPBYTE)&nValue,(DWORD)sizeof(DWORD)))
	{
		StringCbPrintf(strlog,MAX_PATH,L"RegSetValueEx key is HKEY_LOCAL_MACHINE\\SOFTWARE\\OEInstallTool\\NeedToReboot fail! The LastError is [%d]\n",GetLastError());
		::OutputDebugString(strlog);
	}

	RegCloseKey(hk);
}





