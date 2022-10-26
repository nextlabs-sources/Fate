// edpmgrutility.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "strsafe.h"
#include "include/enhancement.h"
#include "src/enhancement_i.c"

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

HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, _Out_ void ** ppv)
{
	HRESULT hr;
	if(IsWin7())
	{
	BIND_OPTS3 bo;
	WCHAR  wszCLSID[50];
	WCHAR  wszMonikerName[300];

	StringFromGUID2(rclsid, wszCLSID,   
		sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
		hr = StringCchPrintf(wszMonikerName,  
		sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
	if (FAILED(hr))
		{
			g_log.Log(CELOG_DEBUG, L"StringCchPrintf for \"enhancement\" failed, err: %x", hr);
		return hr;
		}
	memset(&bo, 0, sizeof(bo));
	bo.cbStruct = sizeof(bo);
	bo.hwnd = hwnd;
	bo.dwClassContext  = CLSCTX_LOCAL_SERVER;

		hr = CoGetObject(wszMonikerName, &bo, riid, ppv);
	}
	else
	{
		hr = CoCreateInstance(rclsid, NULL, CLSCTX_LOCAL_SERVER, riid, ppv);
	}
	
	if(FAILED(hr))
	{
		g_log.Log(CELOG_DEBUG, L"Create instance for \"enhancement\" failed, err: %x", hr);
	}

	return hr;
}

CComPtr<INLEDPManager> g_pIEDPMgr = NULL;
//Call COM to run "setdebugmode", it will pop up the UAC dialog if UAC was turned on.
int UAC_SetDebugMode(BOOL bEnable, HWND hParentWnd)
{
	int nRet = -1;
	HRESULT hr;
	//	hr = CoCreateInstance(CLSID_NLEDPManager, NULL, CLSCTX_ALL, IID_INLEDPManager, (void**)&pMgr);

	if(NULL == g_pIEDPMgr)
	{
		hr = CoCreateInstanceAsAdmin(hParentWnd, CLSID_NLEDPManager, IID_INLEDPManager, (void**)&g_pIEDPMgr);
	}

	if(g_pIEDPMgr)
	{
		LONG lRet = 0;
		hr = g_pIEDPMgr->SetDebugMode((SHORT)bEnable, &lRet);

			nRet = (int)lRet;
		}
	else
	{ 
		nRet = -2;
	}

	return nRet;
}

int UAC_StartPC(HWND hParentWnd)
{
	int nRet = -1;
	HRESULT hr;

	if(NULL == g_pIEDPMgr)
	{
		hr = CoCreateInstanceAsAdmin(hParentWnd, CLSID_NLEDPManager, IID_INLEDPManager, (void**)&g_pIEDPMgr);
	}

	if(g_pIEDPMgr)
	{
		LONG lRet = 0;
		hr = g_pIEDPMgr->StartPCService(&lRet);

			nRet = (int)lRet;
		}
	else
	{ 
		nRet = -2;
		g_log.Log(CELOG_DEBUG, L"g_pIEDPMgr is null after CoCreateInstanceAsAdmin\n");
	}

	return nRet;
}

int UAC_EnableAgentLog(BOOL bEnable, HWND hParentWnd)
{
	int nRet = -1;
	HRESULT hr;
	//	hr = CoCreateInstance(CLSID_NLEDPManager, NULL, CLSCTX_ALL, IID_INLEDPManager, (void**)&pMgr);

	if(NULL == g_pIEDPMgr)
	{
		hr = CoCreateInstanceAsAdmin(hParentWnd, CLSID_NLEDPManager, IID_INLEDPManager, (void**)&g_pIEDPMgr);
	}

	if(g_pIEDPMgr)
	{
		LONG lRet = 0;
		hr = g_pIEDPMgr->EnableAgentLog((SHORT)bEnable, &lRet);

			nRet = (int)lRet;
		}
	else
	{ 
		nRet = -2;
	}

	return nRet;
}

void UAC_Reset()
{
	g_pIEDPMgr = NULL;
}
