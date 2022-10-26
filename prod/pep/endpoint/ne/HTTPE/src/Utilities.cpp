#include "stdafx.h"
#include <string>
#include <algorithm>
#include "utilities.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>
#include "MapperMngr.h"
#include "httpcollector.h"

#pragma warning(push)
#pragma warning(disable: 6387)
#pragma warning(disable: 6011)
#include <strsafe.h>
#pragma warning(pop)

#include <sddl.h>
#include <list>
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "APIHook.h"
#include "configure.h"
#include "Eval.h"

#pragma warning(push)
#pragma warning(disable: 6011)
#  include <boost/algorithm/string.hpp>
#pragma warning(pop)

#include "gziphelper.h"

#pragma warning(push)
#pragma warning(disable: 4800)
#include "eframework/platform/policy_controller.hpp"

#ifdef PEP_USE_LEGACY_IGNORE_CHECK
#include <eframework/platform/ignore_application.hpp>
#else
#include <eframework/policy/policy.hpp>
#endif

#pragma warning(pop)

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

#include "eframework/platform/cesdk_loader.hpp"
nextlabs::cesdk_loader cesdkLoader;

#pragma comment(lib, "Psapi.lib")

#define BUFSIZE 512
#define EVALCACHE_TIMEOUT		100000
#define CONTROL_MANAGER_UUID L"b67546e2-6dc7-4d07-aa8a-e1647d29d4d7"

static BOOL g_bDetached = FALSE;//This flag determines if the "detach(DLLMain)" was called or not.
static std::list<EVALCACHE> g_listEvalCache;
CELog g_log;
CAPIHook g_ApiHook ;

extern nextlabs::recursion_control hook_control;
extern BOOL g_bIgnoredByPolicy;


std::wstring GetCommonComponentsDir()
{
	wchar_t szDir[MAX_PATH] = {0};
	if(NLConfig::ReadKey(L"SOFTWARE\\NextLabs\\CommonLibraries\\InstallDir", szDir, MAX_PATH))
	{
		wchar_t* pTemp = wcsrchr(szDir, L'\\');
		if ( pTemp && !( * ( pTemp + 1 ) ) )
		{
			*pTemp = 0;
		}
#ifdef _WIN64
		wcsncat_s(szDir, MAX_PATH, L"\\bin64\\", _TRUNCATE);
#else
		wcsncat_s(szDir, MAX_PATH, L"\\bin32\\", _TRUNCATE);
#endif
		return szDir;
	}

	return L"";
}



BOOL IsIgnoredByPolicy()
{
#ifdef PEP_USE_LEGACY_IGNORE_CHECK
  bool is_ignored = nextlabs::application::is_ignored();
#else
  bool is_ignored = nextlabs::policy_monitor_application::is_ignored();
#endif

	if( is_ignored == true )
	{
		if(GetLastError() == ERROR_SUCCESS)
		{
			return TRUE;
		}
	}

	nextlabs::feature_manager feat;
	feat.open();

	/* Network control is enabled? */
	if( feat.is_enabled(NEXTLABS_FEATURE_NETWORK) == false )
	{
	  return TRUE;
	}

	return FALSE;
}

BOOL GetModuleName(LPWSTR lpszProcessName, int nLen, HMODULE hMod)
{
	if(!lpszProcessName || nLen > 1024)
		return FALSE;

	if(hMod)
		{
		DWORD dwCount = GetModuleFileNameW(hMod, lpszProcessName, nLen);
		return dwCount > 0 ? TRUE: FALSE;
		}

	static wchar_t filename[1025] = {0};
	if( *filename == 0 )//only call GetModuleFileNameW at the first time. Kevin 20089-8-20
	{
		GetModuleFileNameW(hMod, filename, 1024);
	}

	if( *filename != 0 )
	{
		memcpy(lpszProcessName, filename, nLen * sizeof(wchar_t));
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
	GetModuleName(filename, 1023, NULL);

	wchar_t* p = wcsrchr(filename, '\\');

	if(p && *(p + 1) != '\0')
	{
		if(_wcsicmp(p + 1, lpProcessName) == 0)
			return TRUE;
	}

	return FALSE;

}


bool IsPolicyControllerUp(void)
{
	return nextlabs::policy_controller::is_up();
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

	if( *(szVersion) == 0 )
	{
		LONG    lResult   = 0;
		HKEY    hKey      = 0;

		lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKey);
		if(ERROR_SUCCESS==lResult && NULL!=hKey)
		{
			DWORD dwType = 0, cbData=MAX_PATH;
			wchar_t  szData[MAX_PATH+1] = {0};  memset(szData, 0, sizeof(szData));
			lResult = RegQueryValueEx(hKey, L"Version", 0, &dwType, (LPBYTE)szData, &cbData);

			if(ERROR_SUCCESS == lResult && (*szData) != 0)
			{
				wcsncpy_s(szVersion, MAX_PATH, szData, _TRUNCATE);
			}
			RegCloseKey(hKey);
		}
	}

	return szVersion;
}

BOOL GetProcessSID(HANDLE hProcess, std::wstring& strSID)
{
	HANDLE hToken = NULL;
	if(!OpenProcessToken(hProcess, TOKEN_QUERY , &hToken))
	{
		return FALSE;
	}

	BOOL bSucceed = FALSE;
	PTOKEN_USER pTokenUser = NULL;
	DWORD len = 0;
	GetTokenInformation(hToken,TokenUser,NULL,0,&len);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		pTokenUser = (PTOKEN_USER) malloc (len);
		if( pTokenUser && GetTokenInformation(hToken,TokenUser,pTokenUser,len,&len) != FALSE )
		{
			WCHAR *pSid = NULL;
			if(ConvertSidToStringSidW(pTokenUser->User.Sid, &pSid) != FALSE)
			{
				strSID = std::wstring(pSid);
				bSucceed = TRUE;
				LocalFree(pSid);
			}

			free(pTokenUser);
			pTokenUser = NULL;
		}
	}
	CloseHandle(hToken);
	return bSucceed;
}

/** NLIsWellKnownSid
*
*  \brief Determine if the given SID is well known such as local/network service.
*  \return true when the SID is well known, otherwise false.
*/
bool NLIsWellKnownSid( const WCHAR* sid )
{
	if(!sid)
		return false;

	static const WCHAR* known_sids[] =
	{
		L"S-1-5-18",   /* Local System : OS Account      */
		L"S-1-5-19",   /* NT Authority : Local Service   */
		L"S-1-5-20"    /* NT Authority : Network Service */
	};

	for( int i = 0 ; i < _countof(known_sids) ; i++ )
	{
		if( wcscmp(sid,known_sids[i]) == 0 )
		{
			return true;
		}
	}
	return false;
}/* NLIsWellKnownSid */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>


#define BUFSIZE 512

BOOL GetFileNameFromHandle(HANDLE hFile, std::wstring& strFilePath) 
{
	BOOL bSuccess = FALSE;
	WCHAR pszFilename[MAX_PATH * 2 + 1] = {0};
	HANDLE hFileMap;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if( dwFileSizeLo == 0 && dwFileSizeHi == 0 )
	{
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile, 
		NULL, 
		PAGE_READONLY,
		0, 
		1,
		NULL);

	if (hFileMap) 
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileName (GetCurrentProcess(), 
				pMem, 
				pszFilename,
				MAX_PATH)) 
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[BUFSIZE] = {0};
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) 
				{
					TCHAR szName[BUFSIZE]= {0};
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do 
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, BUFSIZE))
						{
							size_t uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH) 
							{
								bFound = _tcsnicmp(pszFilename, szName, 
									uNameLen) == 0;

								if (bFound) 
								{
									// Reconstruct pszFilename using szTempFile
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH] = {0};
									StringCchPrintf(szTempFile,
										MAX_PATH,
										TEXT("%s%s"),
										szDrive,
										pszFilename+uNameLen);
									StringCchCopyN(pszFilename, MAX_PATH+1, szTempFile, _tcslen(szTempFile));
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}

	if(bSuccess)
	{
		strFilePath = std::wstring(pszFilename);
		wstring strHeader(L"\\Device\\LanmanRedirector");
		if(strFilePath.find(strHeader) == 0 )
		{
			strFilePath = strFilePath.replace(0, strHeader.length(), L"\\");
		}
	}

	return(bSuccess);
}

BOOL InitLog()
{
	// If debug mode is enabled write to log file as well
	if( NLConfig::IsDebugMode() == true )
	{
		/* Generate a path using the image name of the current process.  Set log policy for DebugView
		* and file on log instance.  Path will be [NextLabs]/Network Enforcer/diags/logs/.
		*/
		wchar_t image_name[MAX_PATH * 2 + 1] = {0};

		if( !GetModuleName(image_name, MAX_PATH * 2, NULL) )
		{
			return FALSE;
		}
		wchar_t* image_name_ptr = wcsrchr(image_name,'\\'); // get just image name w/o full path
		if( image_name_ptr == NULL )
		{
			image_name_ptr = image_name;                   // when failed use default image name
		}
		else
		{
			image_name_ptr++;                              // move past '\\'
		}

		wchar_t ne_install_path[MAX_PATH] = {0};
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Network Enforcer",ne_install_path,_countof(ne_install_path)) == true )
		{
			wchar_t log_file[MAX_PATH * 3 + 1] = {0};
			_snwprintf_s(log_file,_countof(log_file), _TRUNCATE,L"%s\\diags\\logs\\httpe_%s.txt",ne_install_path,image_name_ptr);

			wstring temp = wstring(log_file);
			std::string strLogPath = string( temp.begin(), temp.end() );
			g_log.SetPolicy( new CELogPolicy_File(strLogPath.c_str()) );
		}

		g_log.SetPolicy( new CELogPolicy_WinDbg() ); // output to DebugView
		g_log.Enable();                              // enable log
		g_log.SetLevel(CELOG_DEBUG);                 // log threshold to debug level
	}

	return TRUE;
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

std::wstring MyMultipleByteToWideChar(const std::string & strValue, UINT codepage)
{
  int nLen = MultiByteToWideChar(codepage, 0, strValue.c_str(), (int)strValue.length(), NULL, 0 );

	wchar_t* pBuf = new wchar_t[nLen + 1];
	if(!pBuf)
		return L"";

	memset(pBuf, 0, (nLen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(codepage, 0, strValue.c_str(), (int)strValue.length(), pBuf, nLen);
	
	std::wstring strResult(pBuf);

	if(pBuf)
	{
		delete []pBuf;
		pBuf = NULL;
	}
	return strResult;
}

BOOL IsSupportedProcess()
{
	//determine if the current process is SYSTEM
	std::wstring strSID;
	if(GetProcessSID(GetCurrentProcess(), strSID))
	{
		if(NLIsWellKnownSid(strSID.c_str()))
		{
			return FALSE;
		}
	}

	BOOL bRet = FALSE ;
	wchar_t  processname[MAX_PATH] = {0};

	GetModuleName(processname, MAX_PATH, NULL);
	g_log.Log(CELOG_DEBUG, L"HTTPE::Check if the current process is supported:[%s]\r\n",processname);

	LPCWSTR fileName = wcsrchr( processname, L'\\' ) ;
	if( fileName )
	{
		fileName = fileName+1 ;

		CComfigureImpl configure ;
		bRet = configure.CheckConfigureFile( L"HTTPE.ini", L".\\config" ) ;
		if( bRet == TRUE )
		{
			std::wstring strTemp =	fileName ;
			std::wstring::size_type index = strTemp.find( '.' ) ;
			if( index != std::wstring::npos) 
			{
				INIINFORM info ;
				std::wstring strApp = strTemp.substr(0, index) ;
				if(	  configure.IsHookEmpty() )
				{
					if( configure.IsNotSupportApp( strApp ) == TRUE)
					{
						g_log.Log(CELOG_DEBUG, L"HTTPE::This process is ignored by config file %s", processname);
						bRet = FALSE;
					}
					else
					{
						bRet = TRUE ;
					}
				}
				else
				{
					bRet = configure.IsSupportApp( strApp ) ;
				}
			}
		}
		else
		{
			bRet = TRUE ;
		}

	}

	if(bRet)
	{//Check if the current process was ignored by policy
		if(g_bIgnoredByPolicy)
			{
			g_log.Log(CELOG_DEBUG, L"HTTPE::This process was ignored by policy, %s ms\n", processname);
			return FALSE;
		}
	}
	

	return bRet ;
}

void HTTPE_Initialize()
{
	static BOOL bInit = FALSE;
	if(!bInit)
	{
		CMapperMngr::Instance();//generate a static variable here.
		CHttpCollector::CreateInstance();

		InitLog();
		if(IsSupportedProcess())
		{
			g_ApiHook.StartHook() ;

			//try to load the DLLs of SDK 
			std::wstring wstrDir = GetCommonComponentsDir();
			if(cesdkLoader.load(wstrDir.c_str()) == FALSE)
			{
				CPolicy::m_bSDK = FALSE;
			}
			else
			{
				CPolicy::m_bSDK = TRUE;
			}

			if (!g_ZlibLoader.LoadZlib())
			{
				//	load zlib failed
				return;
			}
			
		}
		bInit = TRUE;
	}
	
}

void HTTPE_Release()
{
	g_ApiHook.EndHook() ;

	if(CPolicy::m_bSDK)
	{
		if(cesdkLoader.is_loaded())
		{
			cesdkLoader.unload();
		}
	}
	g_ZlibLoader.FreeZlib();
}

unsigned int utf8_decode( char *s, unsigned int *pi )
{
	unsigned int c;
	int i = *pi;
	/* one digit utf-8 */
	if ((s[i] & 128)== 0 ) {
		c = (unsigned int) s[i];
		i += 1;
	} else if ((s[i] & 224)== 192 ) { /* 110xxxxx & 111xxxxx == 110xxxxx */
		c = (( (unsigned int) s[i] & 31 ) << 6) +
			( (unsigned int) s[i+1] & 63 );
		i += 2;
	} else if ((s[i] & 240)== 224 ) { /* 1110xxxx & 1111xxxx == 1110xxxx */
		c = ( ( (unsigned int) s[i] & 15 ) << 12 ) +
			( ( (unsigned int) s[i+1] & 63 ) << 6 ) +
			( (unsigned int) s[i+2] & 63 );
		i += 3;
	} else if ((s[i] & 248)== 240 ) { /* 11110xxx & 11111xxx == 11110xxx */
		c =  ( ( (unsigned int) s[i] & 7 ) << 18 ) +
			( ( (unsigned int) s[i+1] & 63 ) << 12 ) +
			( ( (unsigned int) s[i+2] & 63 ) << 6 ) +
			( (unsigned int) s[i+3] & 63 );
		i+= 4;
	} else if ((s[i] & 252)== 248 ) { /* 111110xx & 111111xx == 111110xx */
		c = ( ( (unsigned int) s[i] & 3 ) << 24 ) +
			( ( (unsigned int) s[i+1] & 63 ) << 18 ) +
			( ( (unsigned int) s[i+2] & 63 ) << 12 ) +
			( ( (unsigned int) s[i+3] & 63 ) << 6 ) +
			( (unsigned int) s[i+4] & 63 );
		i += 5;
	} else if ((s[i] & 254)== 252 ) { /* 1111110x & 1111111x == 1111110x */
		c = ( ( (unsigned int) s[i] & 1 ) << 30 ) +
			( ( (unsigned int) s[i+1] & 63 ) << 24 ) +
			( ( (unsigned int) s[i+2] & 63 ) << 18 ) +
			( ( (unsigned int) s[i+3] & 63 ) << 12 ) +
			( ( (unsigned int) s[i+4] & 63 ) << 6 ) +
			( (unsigned int) s[i+5] & 63 );
		i += 6;
	} else {
		c = '?';
		i++;
	}
	*pi = i;
	return c;
}


std::string UrlDecode(const std::string& src)
{
	std::string dst;

	std::string::size_type srclen = src.size();

	for (size_t i = 0; i < srclen; i++)
	{
		if (src[i] == '%')
		{
			if(isxdigit(static_cast<char>(src[i + 1])) && isxdigit(static_cast<char>(src[i + 2])))
			{
				char c1 = src[++i];
				char c2 = src[++i];
				c1 = c1 - 48 - ((c1 >= 'A') ? 7 : 0) - ((c1 >= 'a') ? 32 : 0);
				c2 = c2 - 48 - ((c2 >= 'A') ? 7 : 0) - ((c2 >= 'a') ? 32 : 0);
				dst += (unsigned char)(c1 * 16 + c2);
			}
		}
		else
			if (src[i] == '+')
			{
				dst += ' ';
			}
			else
			{
				dst += src[i];
			}
	}

	return dst;
}

const wchar_t* g_szSpecailPath[] = {//Ignore these pathes in CreateFile, Added by Kevin 2009-7-9
	L"\\\\.\\PIPE",
	L"\\\\.\\WMIDATADEVICE",
	L"\\\\?\\ROOT#SYSTEM#0000#",
	L"ROOT#SYSTEM#0000#",
	L"\\\\.\\PHYSICALDRIVE",
	L"CONIN$",
	L"CONOUT$",
	L"\\\\.\\"
};

const wchar_t* g_szFiltedPath[] = {
	L"\\Device\\LanmanRedirector\\"
};

const std::pair<int, std::wstring> g_szIgnoredFolders[] =	
{
	std::pair<int, std::wstring>(CSIDL_APPDATA,  L"\\Microsoft"),
	
};


CUtility::CUtility()
{}
CUtility::~CUtility()
{}
DWORD CUtility::GetVersionNumber(const std::wstring& szModuleName, const std::wstring& szKeyName)
{
	DWORD dNumb = 0 ;
	if( ! szModuleName.empty() ) 
	{
		ModuleVer::DllVersion::CModuleVersion ver ;
		if(ver.GetFileVersionInfo(szModuleName.c_str()) == FALSE )
		{
			return dNumb ;
		}
		std::wstring strRet  = ver.GetValue(szKeyName.c_str() ) ;
		if( strRet.length() >0 )
		{
			std::wstring::iterator itor = strRet.begin() ;
			for( itor ;itor!= strRet.end() ; itor ++ )
			{
				wchar_t temp = (*itor) ;
				if( temp == '.' )
				{
					strRet.erase( itor ) ;
					if( strRet.find( L".") )
					{
						itor = strRet.begin() ;
					}
				}
			}
			dNumb = static_cast<DWORD>(_wtof((wchar_t*) strRet.c_str() ) ) ;
		}
	}
	return dNumb ;
}
/*********************************************
function name: GetIgnoreFolderList
feature: This function was used to get the 
folders which were ignored.
**********************************************/
bool CUtility::GetIgnoreFolderList(std::map<std::wstring, int>& mapIgnoreFolders)
{
	static bool bDone = false;
	static std::map<std::wstring, int> mapFolders;
	if(!bDone)
	{
		wchar_t* pBuf = new wchar_t[MAX_PATH * 2];
		if(pBuf)
		{	
			for(int i = 0; i < _countof(g_szIgnoredFolders); i++)
			{
				memset(pBuf, 0, sizeof(wchar_t) * (MAX_PATH * 2));
				if(SHGetSpecialFolderPath(NULL, pBuf, g_szIgnoredFolders[i].first, NULL))
				{
					wcsncat_s(pBuf, MAX_PATH * 2, g_szIgnoredFolders[i].second.c_str(), _TRUNCATE);
					mapFolders[pBuf] = g_szIgnoredFolders[i].first;
				}
			}
			delete [] pBuf;
			pBuf = NULL;
		}

		bDone = true;
	}
	if(mapFolders.size() > 0)
	{
		mapIgnoreFolders = mapFolders;
		return true;
	}
	return false;
}

bool CUtility::IsSupportFileType(LPCWSTR lpFilePath)
{
	if(!lpFilePath)
		return false;

	std::wstring strFilePath(lpFilePath);
	std::transform(strFilePath.begin(), strFilePath.end(), strFilePath.begin(), towupper);

	int i = 0;

	std::map<std::wstring, int> mapIgnored;
	if( GetIgnoreFolderList(mapIgnored) )//Try to get the list of folders which were ignored
	{
		std::map<std::wstring, int>::iterator itr;

		for( itr = mapIgnored.begin(); itr != mapIgnored.end(); itr++)
		{
			std::pair<std::wstring, int> pairPath = (std::pair<std::wstring, int>)*itr;

			wstring tempFilePath(lpFilePath);
			if ( boost::algorithm::istarts_with(tempFilePath, pairPath.first) )
			{
				return false;
			}
		}
	}

	for(i = 0; i < _countof(g_szSpecailPath); i++)
	{
		if(strFilePath.find(g_szSpecailPath[i]) == 0 )
		{
			return false;
		}
	}

	return true;
}
bool CUtility::IsOfficeFile(LPCSTR pwzFile)
{
	LPCSTR pSuffix = strrchr(pwzFile, L'.');
	if(NULL == pSuffix) return FALSE;
	size_t dLen = strlen(pSuffix) ;
	if(0 == _strnicmp(pSuffix, ".XLT",dLen )
		||0 == _strnicmp(pSuffix,".XLTX",dLen)
		||0 == _strnicmp(pSuffix,".XLTM",dLen)
		||0 == _strnicmp(pSuffix,".XLSX",dLen)
		||0 == _strnicmp(pSuffix,".XLSM",dLen)
		||0 == _strnicmp(pSuffix,".XLAM",dLen)
		||0 == _strnicmp(pSuffix,".XLSB",dLen)
		||0 == _strnicmp(pSuffix,".DOC",dLen)
		||0 == _strnicmp(pSuffix,".DOT",dLen)
		||0 == _strnicmp(pSuffix,".DOCX",dLen)
		||0 == _strnicmp(pSuffix,".DOCM",dLen)
		||0 == _strnicmp(pSuffix,".DOTX",dLen)
		||0 == _strnicmp(pSuffix,".RTF",dLen)
		||0 == _strnicmp(pSuffix,".XLS",dLen)
		||0 == _strnicmp(pSuffix,".XLT",dLen)
		||0 == _strnicmp(pSuffix,".XLSX",dLen)
		||0 == _strnicmp(pSuffix,".XLSM",dLen)
		||0 == _strnicmp(pSuffix,".XLSB",dLen)
		||0 == _strnicmp(pSuffix,".XLTX",dLen)
		||0 == _strnicmp(pSuffix,".XLTM",dLen)
		||0 == _strnicmp(pSuffix,".PPT",dLen)
		||0 == _strnicmp(pSuffix,".POT",dLen)
		||0 == _strnicmp(pSuffix,".PPTX",dLen)
		||0 == _strnicmp(pSuffix,".PPTM",dLen)
		||0 == _strnicmp(pSuffix,".POTX",dLen)
		||0 == _strnicmp(pSuffix,".POTM",dLen)
		||0 == _strnicmp(pSuffix,".THMX",dLen)
		||0 == _strnicmp(pSuffix,".SLDM",dLen)
		||0 == _strnicmp(pSuffix,".SLDX",dLen)
		||0 == _strnicmp(pSuffix,".PPSX",dLen)
		||0 == _strnicmp(pSuffix,".PPSM",dLen)
		||0 == _strnicmp(pSuffix,".PPAM",dLen)
		)
	{

		return TRUE;
	}
	
	return FALSE ;
}


string AddressToString(const struct sockaddr* addr, int addr_len, bool with_port)
{
	addr_len;
	
	char portbuf[10] = {0};


	// Win2K fallback
	if (addr->sa_family != AF_INET)
		return "";
	char* s = inet_ntoa(((struct sockaddr_in*)addr)->sin_addr);
	if (!s)
		return "";

	string host = string(s);
	if (!with_port)
		return host;

	_snprintf_s(portbuf, 10, _TRUNCATE, ":%d", (int)ntohs(((struct sockaddr_in*)addr)->sin_port));
	return host + portbuf;
	
}


wstring GetPeerIP(SOCKET s)
{
	struct sockaddr  name ;
	INT iLen = sizeof(sockaddr)  ;
	if(getpeername( s, &name, &iLen )  == 0 )
	{
		string sPeerIP = AddressToString(&name, sizeof(sockaddr), false);
		return std::wstring(sPeerIP.begin(), sPeerIP.end());
	}
	return L"";
}
BOOL g_bDetach =  FALSE ;
void SetDetachFlag(BOOL bFlag)
{
	g_bDetach =	bFlag ;
}
BOOL GetDetachFlag()
{
	return	  g_bDetach ;
}

BOOL CEncoding::UTF8ToGB2312(const std::string &strInput, std::string &strOut)
{
	wstring strResult = MyMultipleByteToWideChar(strInput, CP_UTF8);
	strOut = MyWideCharToMultipleByte(strResult);

	return TRUE;
}

void exception_cb( NLEXCEPT_CBINFO* cb_info )
{
	hook_control.process_disable(); /* prevent recursion when handling an exception */

	if( cb_info != NULL )
	{
		wchar_t comp_root[MAX_PATH * 2] = {0}; // component root for HTTPE
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Network Enforcer",comp_root,_countof(comp_root)) == true )
		{
			wcsncat_s(comp_root,_countof(comp_root),L"\\diags\\dumps",_TRUNCATE);
			wcsncpy_s(cb_info->dump_root,_countof(cb_info->dump_root),comp_root,_TRUNCATE);
			cb_info->use_dump_root = 1;
		}
		g_log.Log(CELOG_EMERG,L"EXCEPTION 0x%X : PID %d TID %d : %hs [%d] \n",
			cb_info->code,GetCurrentProcessId(), GetCurrentThreadId(),
			cb_info->source_file,cb_info->source_line);
	}

}/* exception_cb */


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

		g_log.Log(CELOG_DEBUG, L"HTTPE::OS version, Major: %d, Minor: %d", sMajor, sMinor);
	}
	

	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;
	
}

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


void TryStringAppend(LPCVOID lpBuffer,  DWORD nNumberOfBytesToWrite)
{
	string sBuf1;
	sBuf1.append((char*)lpBuffer, nNumberOfBytesToWrite < CMapperMngr::MAX_CONTENT_SIZE? nNumberOfBytesToWrite: CMapperMngr::MAX_CONTENT_SIZE);
}

bool TestStringAppendError(LPCVOID lpBuffer,  DWORD nNumberOfBytesToWrite)
{
	__try
	{
		TryStringAppend( lpBuffer,   nNumberOfBytesToWrite);
	}
	__except(EXCEPTION_ACCESS_VIOLATION== GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		return true;
	}

	return false;
}