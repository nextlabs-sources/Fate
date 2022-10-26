#include "stdafx.h"
#include <list>
#include <string>
#include "eframework/platform/policy_controller.hpp"
#include "eframework/platform/cesdk_loader.hpp"
#include "MapperMgr.h"
#include "Eval.h"
#include "apihook.h"
#include "FilterRes.h"
#include "criticalMngr.h"

#pragma warning(push)
#pragma warning(disable: 6386)
#include <Ws2tcpip.h>
#pragma warning(pop)

#define CONTROL_MANAGER_UUID L"b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"

static BOOL g_bDetached = FALSE;//This flag determines if the "detach(DLLMain)" was called or not.

extern nextlabs::recursion_control hpe_hook_control;
nextlabs::cesdk_loader cesdkLoader;

CAPIHook apiHook ;
static BOOL g_bInit = FALSE;

static std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}

BOOL GetCurrentProcessName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static wchar_t filename[1025] = {0};
	if( wcslen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameW(hMod, filename, 1024);
	}

	if(wcslen(filename) > 0)
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
		return TRUE;
	}

	return FALSE;
}

BOOL GetCurrentProcessName2(LPSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
	{
		DWORD dwCount = GetModuleFileNameA(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
	}

	static char filename[1025] = {0};
	if( strlen(filename) == 0)//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameA(hMod, filename, 1024);
	}

	if(strlen(filename) > 0)
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(char));
		return TRUE;
	}

	return FALSE;
}

BOOL IsProcess(LPCWSTR lpProcessName)
{
	if(!lpProcessName)
	{
		return FALSE;
	}

	wchar_t filename[1024] = {0};
	GetCurrentProcessName(filename, 1023, NULL);
	
	wchar_t* p = wcsrchr(filename, '\\');

	if(p && *(p + 1) != '\0')
	{
		if(_wcsicmp(p + 1, lpProcessName) == 0)
			return TRUE;
	}

	return FALSE;

}

int GetIEVersionNum()
{
	int nIEVersion = 0;
	std::wstring strNum = GetIEVersionNum_str();
	if(strNum.length() > 1)
	{
		strNum[1] = '\0';
		nIEVersion = _wtoi(strNum.c_str());
	}
	return nIEVersion;
}

std::wstring GetIEVersionNum_str()
{
	static wchar_t szVersion[MAX_PATH+1] = {0};

	if(wcslen(szVersion) == 0)
	{
	LONG    lResult   = 0;
	HKEY    hKey      = 0;

		lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKey);
	if(ERROR_SUCCESS==lResult && NULL!=hKey)
	{
		DWORD dwType = 0, cbData=MAX_PATH;
			wchar_t  szData[MAX_PATH+1] = {0};  memset(szData, 0, sizeof(szData));
		lResult = RegQueryValueEx(hKey, L"Version", 0, &dwType, (LPBYTE)szData, &cbData);

			if(ERROR_SUCCESS == lResult && wcslen(szData) > 0)
		{
				wcsncpy_s(szVersion, MAX_PATH, szData, _TRUNCATE);
		}
		RegCloseKey(hKey);
	}
	}

	return szVersion;
}

void InitDetachCritical()
{
	static BOOL bInit = FALSE;
	if(!bInit)
	{
		bInit = TRUE;
	}
}

void SetDetachFlag(BOOL bFlag)
{
	EnterCriticalSection(&CcriticalMngr::s_csDetachCritical);
	g_bDetached = bFlag;
	LeaveCriticalSection(&CcriticalMngr::s_csDetachCritical);
}

BOOL GetDetachFlag()
{
	EnterCriticalSection(&CcriticalMngr::s_csDetachCritical);
	BOOL bDetached = g_bDetached;
	LeaveCriticalSection(&CcriticalMngr::s_csDetachCritical);
	return bDetached;
}

bool IsPolicyControllerUp(void)
{
	return nextlabs::policy_controller::is_up();
}

std::string MyWideCharToMultipleByte(const std::wstring & strValue)
{
        int nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0, NULL, NULL); 

	char* pBuf = new char[nLen + 1];
	if(!pBuf)
		return "";
	memset(pBuf, 0, nLen +1);
	nLen = WideCharToMultiByte(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen, NULL, NULL); 

	std::string strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

void exception_cb( NLEXCEPT_CBINFO* cb_info )
{
	hpe_hook_control.process_disable();

	if( cb_info != NULL )
	{
		wchar_t comp_root[MAX_PATH * 2] = {0}; // component root for HTTPE
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Network Enforcer",comp_root,_countof(comp_root)) == true )
		{
			wcsncat_s(comp_root,_countof(comp_root),L"\\diags\\dumps",_TRUNCATE);
			wcsncpy_s(cb_info->dump_root,_countof(cb_info->dump_root),comp_root,_TRUNCATE);
			cb_info->use_dump_root = 1;
		}
		DPW ( (L"EXCEPTION 0x%X : PID %d TID %d : %hs [%d] \n",
			cb_info->code,GetCurrentProcessId(), GetCurrentThreadId(),
			cb_info->source_file,cb_info->source_line) );
	}
}/* exception_cb */

/*************************************************************
On windows 7, we will get the path like below for \\hz-srv01
\device\mup\hz-srv01.
It only happens on windows 7.
Here, we convert \device\mup\hz-srv01 to \\hz-srv01

*************************************************************/
void ConvertUNCPath(wstring& strPath)
{

	wstring strTemp = strPath;
	transform(strTemp.begin(), strTemp.end(), strTemp.begin(), tolower);

	const wstring strPattern(L"\\device\\mup");
	if(strTemp.find(strPattern) == 0)
	{
		strPath = strPath.replace(0, strPattern.length(), L"\\");
	}

}

static std::wstring g_szIgnoreIP[] = {L"127.0.0.1"};
bool IsIgnoredIP(LPCWSTR lpIP)
{
	if(!lpIP)
		return false;

	for(int i = 0; i < _countof(g_szIgnoreIP); i++)
	{
		if(_wcsicmp(lpIP, g_szIgnoreIP[i].c_str()) == 0)
		{
			return true;
		}
	}
	return false;
}

string AddressToString(const struct sockaddr* addr, int addr_len, bool with_port)
{
	addr_len;
	char portbuf[NI_MAXSERV] = {0};

#if 1
	{
		// Win2K fallback
		if (addr->sa_family != AF_INET)
			return "";
		char* s = inet_ntoa(((struct sockaddr_in*)addr)->sin_addr);
		if (!s)
			return "";

		string host = string(s);
		if (!with_port)
			return host;

		_snprintf_s(portbuf,32, _TRUNCATE, ":%d", (int)ntohs(((struct sockaddr_in*)addr)->sin_port));
		return host + portbuf;
	}

#else
	int res = getnameinfo(addr, addr_len, hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if (res) // Should never fail
		return "";

	string host = string(hostbuf);
	string port = string(portbuf);

	// IPv6 uses colons as separator, need to enclose address
	// to avoid ambiguity if also showing port
	if (with_port && addr->sa_family == AF_INET6)
		host = "[" + host + "]";

	if (with_port)
		return host + ":" + port;
	else
		return host;
#endif
}

BOOL CheckNetworkAccess( SOCKET s,const wchar_t* lpIP,const  wchar_t* lpPort) 
{
	s;
	BOOL bRet = TRUE ;
	if(( lpIP == NULL)||( lpPort == NULL  ))
	{
		return bRet ;
	}
	
	CMapperMgr& mapperIns = CMapperMgr::Instance();
	wstring strIPPort = lpIP ;
	strIPPort = strIPPort +L":";
	strIPPort = strIPPort + lpPort ;
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>> listOpenFiles;
	if(!mapperIns.GetAllOpenFiles(strIPPort, listOpenFiles))
	{
		//	there is a previous deny evaluation on this ip:port for the opened file
		DPW((L"Deny Netword Access by has a previous deny evaluation"));
		return FALSE;
	}

	FTP_EVAL_INFO evalInfo;

	FTPE_STATUS status = FTPE_SUCCESS;
	for(std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = listOpenFiles.begin(); it != listOpenFiles.end(); ++it)
	{
		evalInfo.pszServerIP =	lpIP ;
		evalInfo.pszServerPort =	lpPort ;
		evalInfo.pszSrcFileName = MyMultipleByteToWideChar((*it)->strFile);

		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		evalInfo.pszDestFileName = L"server://"+evalInfo.pszServerIP + L":" + evalInfo.pszServerPort ;

		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));

		status = pPolicy->QuerySingleFilePolicy( CPolicy::m_networkAccess, evalInfo , enforcement ) ;

		CEResponse_t response = enforcement.result;

		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

		pPolicy->Release() ;
		if(status == FTPE_SUCCESS)
		{
			switch(response)
			{
			case CEAllow:
				break ;
			case CEDeny:
				{
					/*	modified in 2011/Mar/21, for bug 13831, remove "IsProcess(L"iexplore.exe") && 6 != GetIEVersionNum()", let the cached data to be removed when timeout -- 5minutes   */
					/*	modified by ben, on 22-07-2009, add for flashfxp, when it is flashfxp remove the file and closesocket if hpe deny it, because flashfxp will not close file handle itself.
					*	modified by ben, on 20-08-2009, delay remove from closefile to here for coreftp, #9772
					*/
					if( IsProcess(L"Opera.exe") || IsProcess(L"Explorer.exe") || IsProcess(L"flashfxp.exe") || IsProcess(L"coreftp.exe") )//determine if the current process is IE6, fix bug9642, kevin 2009-8-10
					{
						CMapperMgr& ins = CMapperMgr::Instance();
						ins.RemoveItemByFileName((*it)->strFile);
					}
					DPW((L"Deny Network Access"));
					return FALSE;
				}
				break ;
			default:
				break;
			}
		}
	}

	return bRet ;
}


void HPE_Init()
{
	if(!g_bInit)
	{
		InitDetachCritical();
		CCommonLog::Initialize();
		if(CFilterRes::IsSupportedProcess())
		{
			std::wstring wstrDir = GetCommonComponentsDir();
			if(cesdkLoader.load(wstrDir.c_str()) == FALSE)
			{
				CPolicy::m_bSDK = FALSE;
			}
			else
			{
				CPolicy::m_bSDK = TRUE;
			}

			apiHook.StartHook() ;
		}
		g_bInit = TRUE;
	}
}

void HPE_Finalize()
{
	if(g_bInit)
	{
		SetDetachFlag(TRUE);//it means this DDL was unloaded.

		static BOOL bInit_Detach = FALSE;
		if(!bInit_Detach)
		{
			apiHook.EndHook() ;

			if (cesdkLoader.is_loaded())
			{
				cesdkLoader.unload();
			}

			bInit_Detach = TRUE;

		}
	}
}

std::wstring MyMultipleByteToWideChar(const std::string & strValue)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), NULL, 0 );
	DWORD dError = 0 ;
	if( nLen == 0 )
	{
		dError = ::GetLastError() ;
		if( dError == ERROR_INVALID_FLAGS )
		{
			nLen = MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );
		}
	}
	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	dError = ::GetLastError() ;
	if( dError == ERROR_INVALID_FLAGS )
	{
		MultiByteToWideChar(CP_ACP, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	}
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}
