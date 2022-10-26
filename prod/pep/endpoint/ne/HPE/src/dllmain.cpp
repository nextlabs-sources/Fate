// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#pragma warning(push)
#pragma warning(disable: 4800)

#ifdef PEP_USE_LEGACY_IGNORE_CHECK
#include <eframework/platform/ignore_application.hpp>
#else
#include <eframework/policy/policy.hpp>
#endif

#pragma warning(pop)

#include "eframework/platform/cesdk_loader.hpp"
#include "configure.h"
#include "Eval.h"

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"

#include "criticalMngr.h"

CcriticalMngr g_criticalMgr;

HMODULE	  CFilterRes::m_hInst = NULL ;

FILTER::PROCESSINFO proInfo[] =
{
	{L"",0},
};

static BOOL g_bIgnoredByPolicy = FALSE;

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

static BOOL GetProcessSID(HANDLE hProcess, std::wstring& strSID)
{
	HANDLE hToken = NULL;
	if(!OpenProcessToken(hProcess, TOKEN_QUERY , &hToken))
	{
		return FALSE;
	}

	BOOL bSucceed = FALSE;
	PTOKEN_USER pTokenUser = NULL;
	DWORD len;
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
static bool NLIsWellKnownSid( const WCHAR* sid )
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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	lpReserved;
	CFilterRes::SetCurrentInstance( hModule ) ;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_bIgnoredByPolicy = IsIgnoredByPolicy();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		HPE_Finalize();
		break;
	}
	return TRUE;
}
BOOL CFilterRes::IsSupportedProcess() 
{
	BOOL bRet = FALSE ;
	wchar_t  processname[MAX_PATH] = {0};
	GetCurrentProcessName(processname, MAX_PATH, NULL);
	DP((L"HPE::Check if IsSupportedProcess Process Name:[%s]\n",processname)) ;

	LPCWSTR fileName = wcsrchr( processname, L'\\' ) ;

	
	//determine if the current process is SYSTEM
	std::wstring strSID;
	if(GetProcessSID(GetCurrentProcess(), strSID))
	{
		if(NLIsWellKnownSid(strSID.c_str()))
		{
			return FALSE;
		}
	}


	if( fileName && *(fileName + 1) != '\0')
	{
		fileName = fileName+1 ;

		//determine if the current process was ignored.
		for(int i = 0; i < _countof(proInfo); i++)
		{
			if( _wcsicmp( fileName,proInfo[i].pProcName ) == 0 ) 
			{
				DP((L"This process is ignored by HPE: %s\n", processname));
				return FALSE;
			}

		}

		CComfigureImpl configure ;
		bRet = configure.CheckConfigureFile( L"HPE.ini", L".\\config" ) ;
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
						DPW((L"HPE::This process is ignored by config file: %s", processname));
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

	//for bug11396
	if(bRet && !IsProcess(L"ftpte.exe")&&!IsProcess(L"rundll32.exe"))
	{
		if(g_bIgnoredByPolicy)
			{
			DP((L"HPE::This process was ignored by policy, [%s]\n", processname));
			return FALSE;
		}
	}

	return bRet ;
}
