#pragma warning(push)
#pragma warning(disable: 4005)
#define _WIN32_WINNT 0x0600
#pragma warning(pop)
#include <windows.h>
#include <string>
#include <tchar.h>
#include <list>
#include <shlwapi.h> 
//#include <shellapi.h>

#ifdef _UNICODE
typedef std::wstring TString;
#else
typedef std::string TString;
#endif


#pragma comment(lib, "shlwapi.lib")


typedef HRESULT (__stdcall *DllRegisterServer_Fun)(void);
typedef HRESULT (__stdcall *DllUnregisterServer_Fun)(void);

void ParseCommandLine(const TCHAR* lpCmdLine);
bool IsUninstallMode();
int UnInstall();
int Install();
bool IsRunOn64BitWindows();
void OutputLoginfo(const TCHAR* szLog);
TString GetRegValueString(HKEY  hkey, LPCTSTR lpSubKey, LPCTSTR lpValueName);
TString GetOEInstallDir();
HRESULT UnRegisterModule(const TCHAR* szModule);
HRESULT RegisterModule(const TCHAR* szModule);
TString GetAdobeReaderInstallPath(const TCHAR* szVersion);
TString GetAdobeAcrobatInstallPath(const TCHAR* szVersion);
void    OEPluginDeleteFile(const TCHAR* szFile);

void OEPluginCopyFile(const TCHAR* srcFile, const TCHAR* szDst, BOOL bFailIfExist);
				

const TCHAR* g_szOffice2k7InstallRegKey = _T("SOFTWARE\\Microsoft\\Office\\14.0\\Outlook\\InstallRoot");

const TCHAR* g_szDefaultOEInstallDir = _T("C:\\Program Files\\NextLabs\\");


const TCHAR* g_szNextLabsRegKey = _T("SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller");

const TCHAR* g_szAdobeReaderRegKey64 = _T("SOFTWARE\\Wow6432Node\\Adobe\\Acrobat Reader\\");
const TCHAR* g_szAdobeReaderRegKey32 = _T("SOFTWARE\\Adobe\\Acrobat Reader\\");

const TCHAR* g_szAdobeAcrobatRegKey64 = _T("SOFTWARE\\Wow6432Node\\Adobe\\Adobe Acrobat\\");
const TCHAR* g_szAdobeAcrobatRegKey32 = _T("SOFTWARE\\Adobe\\Adobe Acrobat\\");

const TCHAR* g_szAdobeVersion[] = { _T("9.0"), _T("10.0"), _T("11.0") };


const TCHAR* g_szUninstallMode = _T("uninstall");
TString g_strMode; // install or uninstall
TString g_strOEInstallDir;


enum OfficeVersion
{
	OFFICE_NONE,
	OFFICE2007,
	OFFICE2010,
	OFFICE2013,
	OFFICE2016,
};

class CurrectDirSwitcher
{
public:
	CurrectDirSwitcher(const TCHAR* newCurrentDir);
	~CurrectDirSwitcher();

protected:
	TString m_strOldCurrentDir;
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR  lpCmdLine, int  nCmdShow)
{
    //parse command
	ParseCommandLine(lpCmdLine);

	//
	if (IsUninstallMode())
	{
		return UnInstall();
	}
	else
	{
		return Install();
	}
}




bool CallRegsvrExE(wchar_t *strPara)
{
	if(strPara == NULL)
	{
		return false;
	}
	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	wchar_t strExE[] = L"C:\\Windows\\System32\\regsvr32.exe";
	wchar_t szLog[MAX_PATH*2] = {0}; 
    // Start the child process. 
    if( !CreateProcess( strExE,   // No module name (use command line)
        strPara,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
		_stprintf_s(szLog, _T("CreateProcess:%s failed. module can't be load,GetLastError = [%d]\n"), strPara,GetLastError());
		OutputLoginfo(szLog);
       return false;
    }

	 WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );


	return true;
	
}
void ParseCommandLine(const TCHAR* lpCmdLine)
{
	if (NULL == lpCmdLine)
	{
		return;
	}

   //split command
	std::list<TString> lstCmd;
	const TCHAR* szParam = lpCmdLine;
	while (*szParam)
	{
		const TCHAR* szParamEnd = _tcsstr(szParam, _T(" "));
		if (szParamEnd)
		{
			if (szParamEnd - szParam > 1)
			{
				TString strParam(szParam, szParamEnd);
				lstCmd.push_back(strParam);
			}

			szParam = szParamEnd + 1;
		}
		else
		{
			lstCmd.push_back(szParam);
			break;
		}


	}

	//parse
	std::list<TString>::iterator itCmd = lstCmd.begin();
	while (itCmd != lstCmd.end())
	{
		if (_tcsicmp(itCmd->c_str(), _T("/uninstall"))==0)
		{
			g_strMode = g_szUninstallMode;
		}
		
		itCmd++;
	}
	

	//
	TCHAR szLog[100];
	_stprintf_s(szLog, _T("mode=%s\n"), g_strMode.c_str());
	OutputLoginfo(szLog);
}


void OutputLoginfo(const TCHAR * szLog)
{
	int nLogLen = _tcslen(szLog) + 100;
	TCHAR* szOutput = new TCHAR[nLogLen + 1];
	_stprintf_s(szOutput, nLogLen, _T("OEPlugin:%s"), szLog);
	OutputDebugString(szOutput);
	delete[] szOutput;
	szOutput = NULL;
}

//the return value will be convert to low case
std::wstring GetOutlookExePathName()
{
	const wchar_t* wszOutlookAppPathKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE";
	wchar_t wszPathName[1024]={0};

	//get path from HK_CURRENT_USER
	DWORD dwcbDirLen = 1024 * sizeof(TCHAR);
	long lRegGetValue = RegGetValue(HKEY_CURRENT_USER, wszOutlookAppPathKey,NULL, RRF_RT_REG_SZ, NULL, wszPathName, &dwcbDirLen);
	if ((lRegGetValue == ERROR_SUCCESS) && (wszPathName[0]!=0) )
	{
		_tcslwr_s(wszPathName);
		return wszPathName; //RegGetValue will get the terminating null character;
	}
	else
	{
		//get path from HK_LOCAL_MACHINE
		lRegGetValue = RegGetValue(HKEY_LOCAL_MACHINE, wszOutlookAppPathKey, NULL, RRF_RT_REG_SZ, NULL, wszPathName, &dwcbDirLen);
		if ((lRegGetValue == ERROR_SUCCESS) && (wszPathName[0] != 0))
		{
			_tcslwr_s(wszPathName);
			return wszPathName;
		}
	}

	return L"";
}

//use IMAGE_FILE_MACHINE_I386, IMAGE_FILE_MACHINE_AMD64, IMAGE_FILE_MACHINE_IA64 to check the return value
WORD GetImageMachineType(const wchar_t* wszFilePathName)
{
	WORD dwMachine = 0;
	const int PE_POINTER_OFFSET = 60;
	const int MACHINE_OFFSET = 4;
	const size_t nReadLenForHeader = 4096;
	unsigned char chData[nReadLenForHeader] = { 0 };
	FILE* pFile = NULL;
	errno_t nErr = _wfopen_s(&pFile, wszFilePathName, L"rb");
	if (nErr == 0 && pFile)
	{
		size_t	nReadLen = fread(chData, 1, nReadLenForHeader, pFile);
		if (nReadLen == nReadLenForHeader)
		{
			int nPeHeaderOffset = *(int*)(chData + PE_POINTER_OFFSET);

			dwMachine = *(WORD*)(chData + nPeHeaderOffset + MACHINE_OFFSET);
		}
		fclose(pFile);
		pFile = NULL;
	}
	return dwMachine;
}


OfficeVersion GetOfficeVersion()
{
	//get outlook file path name, the name will be convert to low case
	std::wstring wstrOutlookPathName = GetOutlookExePathName();
	if (!wstrOutlookPathName.empty())
	{
		if (wstrOutlookPathName.find(L"office12") != TString::npos)
		{
			::OutputDebugStringW(L"GetOfficeVersion: 2007.\n");
			return OFFICE2007;
		}
		else if (wstrOutlookPathName.find(L"office14") != TString::npos)
		{
			::OutputDebugStringW(L"GetOfficeVersion: 2010.\n");
			return OFFICE2010;
		}
		else if (wstrOutlookPathName.find(L"office15") != TString::npos)
		{
			::OutputDebugStringW(L"GetOfficeVersion: 2013.\n");
			return OFFICE2013;
		}
		else if (wstrOutlookPathName.find(L"office16") != TString::npos)
		{
			::OutputDebugStringW(L"GetOfficeVersion: 2016.\n");
			return OFFICE2016;
		}
	}

	return OFFICE_NONE;
}

//32 or 64 bit
WORD GetOfficeMachineType()
{
	//get outlook file path name, the name will be convert to low case
	std::wstring wstrOutlookPathName = GetOutlookExePathName();
	if (!wstrOutlookPathName.empty())
	{
		return GetImageMachineType(wstrOutlookPathName.c_str());
	}

	return IMAGE_FILE_MACHINE_I386;	 //default value
}

BOOL IsRunOn64BitOffice()
{
	return GetOfficeMachineType() == IMAGE_FILE_MACHINE_AMD64;
}
	
bool IsUninstallMode()
{
	return (_tcsicmp(g_strMode.c_str(), g_szUninstallMode) == 0);
}

int UnInstall()
{
	int nRet = 0;

	//get OE install dir
	TString strOEInstallDir = GetOEInstallDir();

	OfficeVersion ver = GetOfficeVersion();
	BOOL  b64BitOffice = IsRunOn64BitOffice();
	TString strOutlookPlugins;
	if (ver == OFFICE2010)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ?  L"bin64\\mso2010PEP.dll" : L"bin\\mso2010PEP32.dll";
	}
	else if (ver == OFFICE2013)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ? L"bin64\\mso2013PEP.dll" :  L"bin\\mso2013PEP32.dll";
	}
	else if (ver == OFFICE2016)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ? L"bin64\\mso2016PEP.dll" : L"bin\\mso2016PEP32.dll";
	}
	


	UnRegisterModule(strOutlookPlugins.c_str());

	//office plugins
	TString strOfficePlugin = strOEInstallDir + (b64BitOffice ? L"bin64\\CEOffice.dll" : L"bin\\CEOffice32.dll");
	UnRegisterModule(strOfficePlugin.c_str());


	//Explorer plugins
	TString strExplorerPlugin = strOEInstallDir + (IsRunOn64BitWindows() ? _T("bin\\CE_Explorer.dll") : _T("bin\\CE_Explorer32.dll"));
	UnRegisterModule(strExplorerPlugin.c_str());

	//adobe reader plugins
	TCHAR szLog[MAX_PATH * 2];
	int nAdobeVersinCount = sizeof(g_szAdobeVersion) / sizeof(g_szAdobeVersion[0]);
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strReaderInstappPath = GetAdobeReaderInstallPath(g_szAdobeVersion[i]);
		if (strReaderInstappPath.length())
		{
			TString strReaderPlugin = strReaderInstappPath + _T("\\plug_ins\\CE_Reader32.api");
			OEPluginDeleteFile(strReaderPlugin.c_str());
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Reader install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLoginfo(szLog);
		}
		
	}

	//adobe acrobat
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strAcrobatInstappPath = GetAdobeAcrobatInstallPath(g_szAdobeVersion[i]);
		if (strAcrobatInstappPath.length())
		{
			TString strAcrobatPlugin = strAcrobatInstappPath + _T("\\plug_ins\\CE_Acrobat32.api");
			OEPluginDeleteFile(strAcrobatPlugin.c_str());
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Acrobat install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLoginfo(szLog);
		}
	}

	return nRet;
}

void OEPluginDeleteFile(const TCHAR* szFile)
{
	if (szFile == NULL)
	{
		return ;
	}

	if (!PathFileExists(szFile))
	{
		return;
	}

	TCHAR szLog[MAX_PATH * 2];
	BOOL bDelete = DeleteFile(szFile);
	if (bDelete)
	{
		_stprintf_s(szLog, _T("Delete file:%s success.\n"), szFile);
		OutputLoginfo(szLog);
	}
	else
	{
		TCHAR szNewDir[MAX_PATH * 2] = { 0 };
		DWORD nTime = ::GetTickCount();
		_stprintf_s(szNewDir, _T("%s_%d"), szFile, nTime);

		if (MoveFile(szFile, szNewDir))
		{
			
			if (!MoveFileEx(szNewDir, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
			{
				_stprintf_s(szLog, _T("MoveFileEx:%s, lastError=%d fail\n"), szNewDir, GetLastError());
				OutputLoginfo(szLog);
			}
		}
		else
		{
			_stprintf_s(szLog, _T("MoveFileEx:%s, lastError=%d fail\n"), szFile, GetLastError());
			OutputLoginfo(szLog);
		}
	}
}




HRESULT UnRegisterModule(const TCHAR* szModule)
{
	wchar_t szTest[MAX_PATH*2] = {0};
	_stprintf_s(szTest, L"  /u /s \"%s\"",szModule);
	OutputDebugString(szTest);

	if(!CallRegsvrExE(szTest))
		return S_FALSE;
	return S_OK;

}

HRESULT RegisterModule(const TCHAR* szModule)
{
	
	wchar_t szTest[MAX_PATH*2] = {0};
	_stprintf_s(szTest, L"  /s \"%s\"",szModule);
	OutputDebugString(szTest);

	if(!CallRegsvrExE(szTest))
		return S_FALSE;
	
	return S_OK;

}


int Install()
{
	int nRet = 0;

	//get OE install dir
	TString strOEInstallDir = GetOEInstallDir();

	OfficeVersion ver = GetOfficeVersion();
	BOOL  b64BitOffice = IsRunOn64BitOffice();
	TString strOutlookPlugins;
	if (ver == OFFICE2010)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ? L"bin64\\mso2010PEP.dll" : L"bin\\mso2010PEP32.dll";
	}
	else if (ver == OFFICE2013)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ? L"bin64\\mso2013PEP.dll" : L"bin\\mso2013PEP32.dll";
	}
	else if (ver == OFFICE2016)
	{
		strOutlookPlugins = strOEInstallDir;
		strOutlookPlugins += b64BitOffice ? L"bin64\\mso2016PEP.dll" : L"bin\\mso2016PEP32.dll";
	}
	
	//outlook plugins
	RegisterModule(strOutlookPlugins.c_str());

	//office plugins
	TString strOfficePlugin = strOEInstallDir + (b64BitOffice ? L"bin64\\CEOffice.dll" : L"bin\\CEOffice32.dll");
	RegisterModule(strOfficePlugin.c_str());

	//Explorer plugins
	TString strExplorerPlugin = strOEInstallDir + (IsRunOn64BitWindows() ? _T("bin\\CE_Explorer.dll") : _T("bin\\CE_Explorer32.dll"));
	RegisterModule(strExplorerPlugin.c_str());

	//adobe reader plugins
	TCHAR szLog[MAX_PATH * 2];
	int nAdobeVersinCount = sizeof(g_szAdobeVersion) / sizeof(g_szAdobeVersion[0]);
	TString strSrcReaderPlugin = strOEInstallDir + _T("bin\\CE_Reader32.api");
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strReaderInstappPath = GetAdobeReaderInstallPath(g_szAdobeVersion[i]);
		if (strReaderInstappPath.length())
		{
			TString strReaderPlugin = strReaderInstappPath + _T("\\plug_ins\\CE_Reader32.api");
			OEPluginCopyFile(strSrcReaderPlugin.c_str(), strReaderPlugin.c_str(), FALSE);
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Reader install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLoginfo(szLog);
		}

	}

	//adobe acrobat
	TString strSrcAcrobatPlugin = strOEInstallDir + _T("bin\\CE_Acrobat32.api");
	for (int i = 0; i < nAdobeVersinCount; i++)
	{
		TString strAcrobatInstappPath = GetAdobeAcrobatInstallPath(g_szAdobeVersion[i]);
		if (strAcrobatInstappPath.length())
		{
			TString strAcrobatPlugin = strAcrobatInstappPath + _T("\\plug_ins\\CE_Acrobat32.api");
			OEPluginCopyFile(strSrcAcrobatPlugin.c_str(), strAcrobatPlugin.c_str(), FALSE);
		}
		else
		{
			_stprintf_s(szLog, _T("Can't get Acrobat install path for Version:%s\n"), g_szAdobeVersion[i]);
			OutputLoginfo(szLog);
		}
	}

	return nRet;
}

void OEPluginCopyFile(const TCHAR* srcFile, const TCHAR* szDst, BOOL bFailIfExist)
{
	BOOL b = CopyFile(srcFile, szDst, bFailIfExist);

	TCHAR szLog[MAX_PATH * 3];
	_stprintf_s(szLog, _T("CopyFile %s, src=%s, dst=%s\n"), b ? _T("Success.") : _T("Failed."), srcFile, szDst);
	OutputLoginfo(szLog);
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


bool IsRunOn64BitWindows()
{
	//check if the application is 32 or 64 bit.
	const void * const p = NULL;
	if (sizeof(p) == 4)
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

TString GetOEInstallDir()
{

	HKEY hUninstallKey = NULL;
	TString strOEInstallDir;

	long lRegOpen = ERROR_SUCCESS;
	if (IsRunOn64BitWindows())
	{
		lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szNextLabsRegKey, 0, KEY_READ|KEY_WOW64_64KEY, &hUninstallKey);
	}
	else
	{
		lRegOpen = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szNextLabsRegKey, 0, KEY_READ, &hUninstallKey);
	}

	if ((lRegOpen == ERROR_SUCCESS) && (hUninstallKey != NULL))
	{
		strOEInstallDir = GetRegValueString(hUninstallKey, L"", _T("InstallDir"));

		if (strOEInstallDir.length() == 0)
		{
			OutputLoginfo(_T("Get OE Install Dir from register failed. use default value.\n"));
			strOEInstallDir = g_szDefaultOEInstallDir;
		}

		if (strOEInstallDir[strOEInstallDir.length() - 1] != _T('\\'))
		{
			strOEInstallDir += _T("\\");
		}

		strOEInstallDir += _T("Outlook Enforcer\\");

		OutputLoginfo(strOEInstallDir.c_str());
	}

	return strOEInstallDir;
}

TString GetAdobeReaderInstallPath(const TCHAR* szVersion)
{
    TString strReaderRegisterKey = IsRunOn64BitWindows() ? g_szAdobeReaderRegKey64 : g_szAdobeReaderRegKey32;
	strReaderRegisterKey += szVersion;
	strReaderRegisterKey += _T("\\InstallPath");
	TString strPath = GetRegValueString(HKEY_LOCAL_MACHINE, strReaderRegisterKey.c_str(), NULL);

	TCHAR szLog[MAX_PATH * 2];
	_stprintf_s(szLog, _T("AdobeReader %s, InstallPath:%s\n"), szVersion, strPath.c_str());
	OutputLoginfo(szLog);

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
	OutputLoginfo(szLog);

	return strPath;
}
#if 0


TString GetDesktopEnforcerInstallPath()
{
	const TCHAR* szDERegKey = IsRunOn64BitWindows() ? g_szDERegKey64 : g_szDERegKey32;
	TString strPath = GetRegValueString(HKEY_LOCAL_MACHINE, szDERegKey, _T("InstallDir"));

	if (strPath.length() == 0)
	{
		OutputLoginfo(_T("Get Desktop Enforcer Install Dir from register failed. use default value.\n"));
		strPath = IsRunOn64BitWindows() ? g_szDefaultDesktopEnforcerDir64 : g_szDefaultDesktopEnforcerDir32;
	}

	if (strPath[strPath.length() - 1] != _T('\\'))
	{
		strPath += _T('\\');
	}

	OutputLoginfo(strPath.c_str());

	return strPath;

}

#endif

CurrectDirSwitcher::CurrectDirSwitcher(const TCHAR* newCurrentDir)
{
	DWORD dwCurDirLen = GetCurrentDirectory(0, NULL);

	TCHAR* szCurrentDirBuf = new TCHAR[dwCurDirLen + 1];
    dwCurDirLen = GetCurrentDirectory(dwCurDirLen + 1, szCurrentDirBuf);
	szCurrentDirBuf[dwCurDirLen] = 0;

	m_strOldCurrentDir = szCurrentDirBuf;
	delete[] szCurrentDirBuf;
	szCurrentDirBuf = NULL;

	SetCurrentDirectory(newCurrentDir);
}

CurrectDirSwitcher::~CurrectDirSwitcher()
{
	SetCurrentDirectory(m_strOldCurrentDir.c_str());
}
