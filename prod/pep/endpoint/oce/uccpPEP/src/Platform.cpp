#include "stdafx.h"
#include "Platform.h"
#include "Blocks.h"
#include <process.h>

#pragma warning(push)
#pragma warning(disable : 4100)
#include "brain.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4819)  // We won't fix code page issue in MADCHOOK header file, just ignore it here
#include "madCHook_helper.h"
#pragma warning(pop)

#include "lib_detach.hpp"
CLibDetach g_Detached;


#include "eframework\auto_disable\auto_disable.hpp"
nextlabs::recursion_control hook_control;

#include "LiveSessionWnd.h"

#include "celog.h"
extern CELog g_log;

#define  IM_CHAT_WND		L"IMWindowClass"


//////////////////////////////////////////////////////////////////////////
//	get conf id out from uri, for different type of uri, please set different gruu_type.
#define GRUU_CONF_APP_SHARING L"app:conf:applicationsharing:id:"
#define GRUU_CONF_AUDIO_VEDIO L"app:conf:audio-video:id:"
static BOOL GetConfIDFromUri(const wstring& wstrURI, const wstring& gruu_type, wstring& confID)
{
	//	get conference id from URI, e.g app:conf:audio-video:id:872D02FE0341334DA842437FE71E5FC3
	wstring::size_type pos = wstrURI.find(gruu_type);
	if (pos != wstring::npos)
	{
		confID = wstrURI.substr(pos + gruu_type.length(), wstrURI.length() - (pos + gruu_type.length()));		
		g_log.Log(CELOG_DEBUG, L"conference id %s\n", confID.c_str());
		return TRUE;
	}
		
	return FALSE;	
}


static HWND FindActiveChatWnd()
{
	HWND hWnd = GetForegroundWindow();

	bool bFound = false;
	if(hWnd)
	{
		wchar_t szClassName[500] = {0};
		GetClassNameW(hWnd, szClassName, 500);

		if(_wcsicmp(IM_CHAT_WND, szClassName) == 0)
		{
			g_log.Log(CELOG_DEBUG, L"Found the IMWindowClass window with GetForegroundWindow, handle: %d\n", hWnd);
			bFound = true;
		}
	}
	if(bFound)
	{
		return hWnd;
	}

	hWnd = FindWindowW(IM_CHAT_WND, NULL);

	g_log.Log(CELOG_DEBUG, L"Try to find chat window with %s, result: %d\n", IM_CHAT_WND, hWnd);

	return hWnd;
}

static bool OCEIsDisabled(void)
{
	return ( hook_control.is_disabled() || g_Detached.GetDetachFlag() );
}



static void exception_cb( NLEXCEPT_CBINFO* cb_info )
{
	hook_control.process_disable(); /* prevent recursion when handling an exception */
	
	if( cb_info != NULL )
	{
		wchar_t comp_root[MAX_PATH * 2] = {0}; // component root for HTTPE
		if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\Office Communicator Enforcer",comp_root,_countof(comp_root)) == true )
		{
			wcsncat_s(comp_root,_countof(comp_root),L"\\diags\\dumps",_TRUNCATE);
			wcsncpy_s(cb_info->dump_root,_countof(cb_info->dump_root),comp_root,_TRUNCATE);
			cb_info->use_dump_root = 1;
		}
	}

}/* exception_cb */



/* UCC_E_SIP_AUTHENTICATION_FAILED
*
* Indicates that authentication failed (ID 71).  This is defined in the UCC
* SDK (UccApiErr.h) but is not avaialble as part of the OCE build partly due
* to OCE using a Beta UCC SDK release.
*/
#ifndef UCC_E_SIP_AUTHENTICATION_FAILED
#define UCC_E_SIP_AUTHENTICATION_FAILED ((HRESULT)0x80EE0011L)
#endif

MAP_PLATFORM gMapPlatform;
Mutex gMutexMapPlatform;

MAP_ENDPOINT gMapEndpoint;
Mutex gMutexMapEndpoint;

MAP_SESSIONMAN gMapSessionMan;
Mutex gMutexMapSessionMan;

MAP_SESSION gMapSession;
Mutex gMutexMapSession;

MAP_IM_SESSION gMapIMSession;
Mutex gMutexMapIMSession;

MAP_SUBSCRIPTION_MGR gMapSubscriptionMgr;
Mutex gMutexMapSubscriptionMgr;

MAP_SUBSCRIPTION gMapSubscription;
Mutex gMutexMapSubscription;

MAP_SESSION_PARTICIPANT gMapSessionParticipant;
Mutex gMutexMapSessionParticipant;

MAP_CONF_SESSION gMapConfSession;
Mutex gMutexMapConfSession;

MAP_CONN_POINT_CONTAINER gMapConnPointContainer;
Mutex gMutexMapMapConnPointContainer;

MAP_CONN_POINT gMapConnPoint;
Mutex gMutexMapMapConnPoint;

MAP_SESSION_MANAGER_EVENTS gMapSessionManEvents;
Mutex gMutexMapSessionManEvents;
/*==========================================================================*
* Interanl Global variables and functions scoped in this file.             *
*==========================================================================*/
namespace {
	PolicyEval ocePolicyEval;

	std::wstring GetDisclaimer(CComPtr<IUccCollection> pParticipants, int nCount)
	{
		std::wstring strDisclaimer;
		for(int i=1; i<=nCount; i++) {
			CComVariant vtItem;
			vtItem.vt = VT_DISPATCH;
			pParticipants->get_Item(i, &vtItem);
			CComQIPtr<IUccSessionParticipant> pParticipant = vtItem.pdispVal;
			BSTR bstrUri = NULL;IUccUri* pUri = NULL;
			HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);

			g_log.Log(CELOG_DEBUG, L"Get disclaimer, participant: %s\n", bstrUri);

			if(SUCCEEDED(hr)&&bstrUri) {	
				BSTR curDisclaimer=NULL;
				ocePolicyEval.FetchDisclaimer(bstrUri, &curDisclaimer);
				if(curDisclaimer) {
					//We got a disclaimer, append to the exisiting disclaimer
					strDisclaimer+=curDisclaimer;
					strDisclaimer+=L"\t\n";
					SysFreeString(curDisclaimer);
					curDisclaimer=NULL;
				}
			}
			SysFreeString(bstrUri);
		}
		return strDisclaimer;
	}

	//Some utility functions
	void GetWindowTitle (HWND hWnd, WCHAR* pszTitle, int maxCount)
	{
		//HWND hWnd = GetForegroundWindow();
		pszTitle[0] = L'\0';
		::GetWindowTextW(hWnd, pszTitle, maxCount);
	}

	// if add participant failed(invite deny), then should sub 1 from total num of participants(title of window)
	bool ResetWindowTitleForRemove(HWND winHandle)
	{
		WCHAR title[1024] = {0};
		GetWindowTitle(winHandle, title, 1024);

		WCHAR titleprev[1024] = {0};
		int posprev = 0;
		int num = -1;
		WCHAR titlenext[1024] = {0};
		int posnext = 0;
		for(size_t i = 0; i < wcslen(title); i++)
		{
			if(title[i] >= '0' && title[i] <= '9')
			{
				if(-1 == num)
					swscanf_s(title + i, L"%d", &num);
			}
			else
			{
				if(-1 == num)
				{
					titleprev[posprev++] = title[i];
				}
				else
				{
					titlenext[posnext++] = title[i];
				}
			}
		}
		if(num >= 1)
		{
			num--;
			WCHAR newtitle[1024] = {0};
			wsprintf(newtitle, _T("%s%d%s"), titleprev, num, titlenext);
			::SetWindowTextW(winHandle, newtitle);
			
			g_log.Log(CELOG_DEBUG, _T("ResetWindowTitleForRemove: window's title was changed to %s\n"), newtitle);
			return true;
		}
		return false;
	}

	void GetCurrentSessionParticipantNum(HWND winHandle, int &num)
	{
		WCHAR title[1024]={0};
		GetWindowTitle(winHandle, title, 1024);
		num=0;

		INT_PTR nNumIndex = 0;
		for(size_t i = 0; i < wcslen(title); i++)
		{
			if(title[i] >= '0' && title[i] <= '9' )
			{
				nNumIndex = i;
				break;
			}
		}
		swscanf_s(title + nNumIndex,L"%d", &num);
		g_log.Log(CELOG_DEBUG, _T("GetCurrentSessionParticipantNum: %d from title %s, window: %d\n"), num, title, winHandle);
	}

	//Do policy evaluation on incoming session
	bool DoEvalOnIncomingSession(CComPtr<IUccSession> pSession) 
	{
		UCC_SESSION_TYPE sessionType;

		pSession->get_Type(&sessionType);				
		if(sessionType == UCCST_INSTANT_MESSAGING) 
			g_log.Log(CELOG_DEBUG, _T("Session(%p) is UCCST_INSTANT_MESSAGING\n"), pSession);
		else if(sessionType == UCCST_APPLICATION) {
			//The application session here can be either of group session or 
			//live meeting session. So we let it allow here and handle it 
			//later when we have a good picture of it. 
			g_log.Log(CELOG_DEBUG, _T("Session(%p) is UCCST_APPLICATION\n"), pSession);
			return true;
		}
		// Added By Jacky.Dong 2011-12-08
		else if(sessionType == UCCST_APPLICATION_SHARING)
		{
			// Sharing evaluation is unilateral, needn't to do evaluation in receiver
			g_log.Log(CELOG_DEBUG, _T("Session(%p) is UCCST_APPLICATION_SHARING\n"), pSession);
			return true;
		}

		CComPtr<IUccCollection> pParticipants; 
		CComQIPtr<IUccSessionParticipant> pParticipant;
		long numParticipants;
		bool bAllow=true;
		pSession->get_Participants(&pParticipants );
		pParticipants->get_Count(&numParticipants);

		g_log.Log(CELOG_DEBUG, _T("Session(%p) type (%d), participants count (%d)\n"), pSession, sessionType, numParticipants);

		for(int i=1; i<=numParticipants; i++) 
		{
			CComVariant vtItem;
			vtItem.vt = VT_DISPATCH;
			pParticipants->get_Item(i, &vtItem);
			pParticipant = vtItem.pdispVal;
			VARIANT_BOOL bLocal;
			if(pParticipant->get_IsLocal(&bLocal)==S_OK)
			{
				if(bLocal)
				{
					g_log.Log(CELOG_DEBUG, _T("Session's participant is local\n"));
					continue;
				}
			}
			BSTR bstrUri = NULL;
			IUccUri* pUri = NULL;
			HRESULT hr = pParticipant->get_Uri(&pUri);
			pUri->get_AddressOfRecord(&bstrUri);
			if(SUCCEEDED(hr) && bstrUri) 
			{
				//No any obligations (including log, notifier, warn, disclaimer) on peer-to-peer incoming session
				if(!ocePolicyEval.EvalAddParticipant(bstrUri, sessionType, false))
				{
					bAllow = false;
				}
				g_log.Log(CELOG_DEBUG, _T("Session(%p) has non-local participant(%d): %s\n"), 
					pSession, numParticipants-1, bstrUri);
			}
			SysFreeString(bstrUri);
		}
		return bAllow;
	}
}

HRESULT __stdcall My_NewPlatform_Initialize (
	IUccPlatform* This,
	BSTR bstrApplicationName,
struct IUccContext * pContext )
{	
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_PLATFORM_INITIALIZE OldInitialize = NULL;

		{
			(&gMutexMapPlatform)->lock();
			MAP_PLATFORM::iterator it = gMapPlatform.find(This);
			if(it != gMapPlatform.end())
			{
				OldInitialize = it->second.Initialize;
			}
			(&gMutexMapPlatform)->unlock();
		}

		if(!OldInitialize)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Initialize\n",
				This);
			g_log.Log(CELOG_DEBUG, Error);
			return S_FALSE;
		}

		return OldInitialize(
			This,
			bstrApplicationName,
			pContext);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	char Error[256]={0};
	FUNC_PLATFORM_INITIALIZE OldInitialize = NULL;

	{
		MUTEX _mutex_(&gMutexMapPlatform);
		MAP_PLATFORM::iterator it = gMapPlatform.find(This);
		if(it != gMapPlatform.end())
		{
			OldInitialize = it->second.Initialize;
		}
	}

	if(!OldInitialize)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Initialize\n",
			This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldInitialize(
		This,
		bstrApplicationName,
		pContext);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_Initialize %p, %ws return %x\n",
		This,
		bstrApplicationName?bstrApplicationName:L"NULL",
		hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall My_NewPlatform_CreateEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
struct IUccContext * pContext,
struct IUccEndpoint * * ppEndpoint 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_PLATFORM_CREATEENDPOINT OldCreateEndpoint = NULL;

		{
			(&gMutexMapPlatform)->lock();
			MAP_PLATFORM::iterator it = gMapPlatform.find(This);
			if(it != gMapPlatform.end())
			{
				OldCreateEndpoint = it->second.CreateEndpoint;
			}
			(&gMutexMapPlatform)->unlock();
		}

		if(!OldCreateEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateEndpoint(
			This,
			eType,
			pUri,
			bstrEndpointId,
			pContext,
			ppEndpoint);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	char Error[256]={0};
	FUNC_PLATFORM_CREATEENDPOINT OldCreateEndpoint = NULL;

	{
		MUTEX _mutex_(&gMutexMapPlatform);
		MAP_PLATFORM::iterator it = gMapPlatform.find(This);
		if(it != gMapPlatform.end())
		{
			OldCreateEndpoint = it->second.CreateEndpoint;
		}
	}

	if(!OldCreateEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCreateEndpoint(
		This,
		eType,
		pUri,
		bstrEndpointId,
		pContext,
		ppEndpoint);

	BSTR bstrEndpointUri = NULL;
	pUri->get_UserAtHost(&bstrEndpointUri);
	sprintf_s(Error,sizeof(Error),"My_NewPlatform_CreateEndPoint(%p,%ws,%ws,%p,%p) return %x\n",
		This,
		bstrEndpointUri? bstrEndpointUri:L"NULL",
		bstrEndpointId? bstrEndpointId:L"NULL",
		pContext,
		ppEndpoint? (*ppEndpoint):NULL,
		hr);
	g_log.Log(CELOG_DEBUG, Error);

	if(bstrEndpointUri)
	{
		// EndPointSip is user's sign-in address and it's first character often is upper, this make some mistake times,
		// so, here, we lower the EndPointSip manually.
		CComBSTR combstr(bstrEndpointUri);
		if(!combstr)
		{
			ocePolicyEval.SetEndPointSip(bstrEndpointUri);
		}
		else
		{
			if (FAILED(combstr.ToLower()))
			{
				ocePolicyEval.SetEndPointSip(bstrEndpointUri);
			}
			else
			{
				ocePolicyEval.SetEndPointSip(combstr.Detach());
			}
		}
		g_log.Log(CELOG_DEBUG, L"Real endpoint: %s\n", ocePolicyEval.GetEndPointSip());
	}
	else if(bstrEndpointId)
		ocePolicyEval.SetEndPointSip(bstrEndpointId);
	SysFreeString(bstrEndpointUri);

	if(hr == S_OK && ppEndpoint)
	{
		IUccEndpoint* pEndPoint = *ppEndpoint;
		HookEndpoint(pEndPoint);
	}
	return hr;
}

HRESULT __stdcall My_NewPlatform_CreateProxyEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccEndpoint * pControllingEndpoint,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_PLATFORM_CREATEPROXYENDPOINT OldCreateProxyEndpoint = NULL;

		{
			(&gMutexMapPlatform)->lock();
			MAP_PLATFORM::iterator it = gMapPlatform.find(This);
			if(it != gMapPlatform.end())
			{
				OldCreateProxyEndpoint = it->second.CreateProxyEndpoint;
			}
			(&gMutexMapPlatform)->unlock();
		}

		if(!OldCreateProxyEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateProxyEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateProxyEndpoint(
			This,
			eType,
			pControllingEndpoint,
			pUri,
			bstrEndpointId,
			pContext,
			ppEndpoint);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_PLATFORM_CREATEPROXYENDPOINT OldCreateProxyEndpoint = NULL;

	{
		MUTEX _mutex_(&gMutexMapPlatform);
		MAP_PLATFORM::iterator it = gMapPlatform.find(This);
		if(it != gMapPlatform.end())
		{
			OldCreateProxyEndpoint = it->second.CreateProxyEndpoint;
		}
	}

	if(!OldCreateProxyEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateProxyEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCreateProxyEndpoint(
		This,
		eType,
		pControllingEndpoint,
		pUri,
		bstrEndpointId,
		pContext,
		ppEndpoint);

	BSTR bstrEndpointUri = NULL;
	pUri->get_AddressOfRecord(&bstrEndpointUri);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_CreateProxyEndpoint(%p,%p,%ws,%ws,%p,%p) return %x\n",
		This,
		pControllingEndpoint,
		bstrEndpointUri? bstrEndpointUri:L"NULL",
		bstrEndpointId? bstrEndpointId:L"NULL",
		pContext,
		ppEndpoint? (*ppEndpoint):NULL,
		hr);
	SysFreeString(bstrEndpointUri);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall My_NewPlatform_Shutdown(
	IUccPlatform* This,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_PLATFORM_SHUTDOWN OldShutdown = NULL;

		{
			(&gMutexMapPlatform)->lock();
			MAP_PLATFORM::iterator it = gMapPlatform.find(This);
			if(it != gMapPlatform.end())
			{
				OldShutdown = it->second.Shutdown;
			}
			(&gMutexMapPlatform)->unlock();
		}

		if(!OldShutdown)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Shutdown\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldShutdown(
			This,
			pOperationContext);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_PLATFORM_SHUTDOWN OldShutdown = NULL;

	{
		MUTEX _mutex_(&gMutexMapPlatform);
		MAP_PLATFORM::iterator it = gMapPlatform.find(This);
		if(it != gMapPlatform.end())
		{
			OldShutdown = it->second.Shutdown;
		}
	}

	if(!OldShutdown)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Shutdown\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldShutdown(
		This,
		pOperationContext);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_Shutdown(%p,%p) return %x\n",
		This,pOperationContext,hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}


HRESULT __stdcall Try_NewPlatform_Initialize(
	IUccPlatform* This,
	BSTR bstrApplicationName,
	struct IUccContext * pContext )
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewPlatform_Initialize beg\n");

	__try
	{
		res = My_NewPlatform_Initialize(		This,
			bstrApplicationName,
			pContext ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewPlatform_Initialize end\n");
	return res;
}

HRESULT __stdcall Try_NewPlatform_CreateEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewPlatform_CreateEndpoint beg\n");

	__try
	{
		res = My_NewPlatform_CreateEndpoint(
			This,
			eType,
			pUri,
			bstrEndpointId,
			pContext,
			ppEndpoint);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewPlatform_CreateEndpoint end\n");
	return res;
}

HRESULT __stdcall Try_NewPlatform_CreateProxyEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccEndpoint * pControllingEndpoint,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewPlatform_CreateProxyEndpoint beg\n");

	__try
	{
		res = My_NewPlatform_CreateProxyEndpoint(
			This,
			eType,
			pControllingEndpoint,
			pUri,
			bstrEndpointId,
			pContext,
			ppEndpoint );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewPlatform_CreateProxyEndpoint end\n");
	return res;
}

HRESULT __stdcall Try_NewPlatform_Shutdown(
	IUccPlatform* This,
	struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewPlatform_Shutdown beg\n");
	
	__try
	{
		res = My_NewPlatform_Shutdown(
			This,
			pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewPlatform_Shutdown end\n");
	return res;
}

HRESULT __stdcall My_NewPlatform_Enable(
									 IUccEndpoint* This
									 )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_ENDPOINT_ENABLE OldEnable = NULL;

	{
		MUTEX _mutex_(&gMutexMapEndpoint);
		MAP_ENDPOINT::iterator it = gMapEndpoint.find(This);
		if(it != gMapEndpoint.end())
		{
			OldEnable = it->second.Enable;
		}
	}

	if(!OldEnable)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Enable\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldEnable(
		This
		);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_Enable(%p) return %x\n",
		This,hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall My_NewPlatform_Disable(
									  IUccEndpoint* This
									  )
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_ENDPOINT_DISABLE OldDisable = NULL;

	{
		MUTEX _mutex_(&gMutexMapEndpoint);
		MAP_ENDPOINT::iterator it = gMapEndpoint.find(This);
		if(it != gMapEndpoint.end())
		{
			OldDisable = it->second.Disable;
		}
	}

	if(!OldDisable)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Disable\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldDisable(
		This
		);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_Disable(%p) return %x\n",
		This,hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall Try_NewPlatform_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewPlatform_QueryInterface beg\n");

		__try
		{
			res = My_NewPlatform_QueryInterface(		This,
				riid,
				ppvObj ) ;
		}
		__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
		{
			;
		}

		g_log.Log(CELOG_ERR,"Try_NewPlatform_QueryInterface end\n");
		return res;
}


HRESULT __stdcall My_NewPlatform_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

		{
			(&gMutexMapEndpoint)->lock();
			MAP_ENDPOINT::iterator it = gMapEndpoint.find((IUccEndpoint*)This);
			if(it != gMapEndpoint.end())
			{
				OldQueryInterface = it->second.QueryInterface;
			}
			(&gMutexMapEndpoint)->unlock();
		}

		if(!OldQueryInterface)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}	

		return OldQueryInterface(		This,
			riid,
			ppvObj ) ;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

	{
		MUTEX _mutex_(&gMutexMapEndpoint);
		MAP_ENDPOINT::iterator it = gMapEndpoint.find((IUccEndpoint*)This);
		if(it != gMapEndpoint.end())
		{
			OldQueryInterface = it->second.QueryInterface;
		}
	}

	if(!OldQueryInterface)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}	

	wchar_t ClassId[256] = {0};
	StringFromGUID2(riid,ClassId,sizeof(ClassId)/sizeof(wchar_t));
	if(memcmp(&IID_IUccSessionManager,&riid,sizeof(riid))==0)
	{
		wcscpy_s(ClassId,sizeof(ClassId)/sizeof(wchar_t),L"IID_IUccSessionManager");
	}
	else if(memcmp(&IID_IUccServerSignalingSettings,&riid,sizeof(riid))==0)
	{
		wcscpy_s(ClassId,sizeof(ClassId)/sizeof(wchar_t),L"IID_IUccServerSignalingSettings");
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccServerSignalingSettings\n"));
	}
	else if(memcmp(&IID_IUccDiagnosticReportingSettings,&riid,sizeof(riid))==0)
	{
		wcscpy_s(ClassId,sizeof(ClassId)/sizeof(wchar_t),L"IID_IUccDiagnosticReportingSettings");
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccDiagnosticReportingSettings\n"));
	}
	else if(memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0)
	{
		wcscpy_s(ClassId,sizeof(ClassId)/sizeof(wchar_t),L"IID_IConnectionPointContainer");
		g_log.Log(CELOG_DEBUG, _T("Query IID_IConnectionPointContaine\n"));
	}

	HRESULT hr = OldQueryInterface(
		This,
		riid,
		ppvObj
		);

	sprintf_s(Error,sizeof(Error),"My_NewPlatform_QueryInterface(%p,%ws) return %x\n",
		This,
		ClassId,
		hr);
	//g_log.Log(CELOG_DEBUG, Error);

	if(hr == S_OK)
	{
		if(memcmp(&IID_IUccSessionManager,&riid,sizeof(riid))==0) 
		{
			IUccSessionManager* pUccSessionManager = (IUccSessionManager*)(*ppvObj);
			HookSessionManager(pUccSessionManager);
		}
		else if (_wcsnicmp(ClassId, L"{fb194526-2e78-4cca-927e-e967bf0101f7}", 
			wcslen(L"{fb194526-2e78-4cca-927e-e967bf0101f7}")) == 0) {
				IUccSubscriptionManager *pSubscriptionManager = (IUccSubscriptionManager *)(*ppvObj);
				HookSubscriptionManager(pSubscriptionManager);
		} else if (memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0) {
			IConnectionPointContainer* pConnectionPointContainer = (IConnectionPointContainer*)(*ppvObj);
			HookConnectionPointContainer(pConnectionPointContainer);
		} 

	}
	return hr;	
}

HRESULT __stdcall My_NewSession_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

		{
			(&gMutexMapSession)->lock();
			MAP_SESSION::iterator it = gMapSession.find((IUccSession*)This);
			if(it != gMapSession.end())
			{
				OldQueryInterface = it->second.QueryInterface;
			}
			(&gMutexMapSession)->unlock();
		}

		if(!OldQueryInterface)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface #####\\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldQueryInterface(
			This,
			riid,
			ppvObj);

	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	
	char Error[256]={0};
	FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

	{
		MUTEX _mutex_(&gMutexMapSession);
		MAP_SESSION::iterator it = gMapSession.find((IUccSession*)This);
		if(it != gMapSession.end())
		{
			OldQueryInterface = it->second.QueryInterface;
		}
	}

	g_log.Log(CELOG_DEBUG, L"In My_NewSession_QueryInterface, \"this\": %p, next QueryInterface: %p\n", This, OldQueryInterface);

	if(!OldQueryInterface) {
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		//We need to hook IUccSession for this session in order to get the oldQueryInterface
		HookSession((IUccSession*)This);
		//Add this Window's entry in the SessionWindowMap
		ocePolicyEval.AddSessionWindowPair(GetForegroundWindow(), (IUccSession*)This);
		{
			MUTEX _mutex_(&gMutexMapSession);
			MAP_SESSION::iterator it = gMapSession.find((IUccSession*)This);
			if(it != gMapSession.end())
			{
				OldQueryInterface = it->second.QueryInterface;
			}
		}
	}	


	wchar_t ClassId[256] = {0};
	StringFromGUID2(riid,ClassId,sizeof(ClassId)/sizeof(wchar_t));
	if((memcmp(&IID_IUccInstantMessagingSession,&riid,sizeof(riid))==0))
	{
		wcscpy_s(ClassId,sizeof(ClassId)/sizeof(wchar_t),L"IID_IUccInstantMessagingSession");
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccInstantMessagingSession\n"));
	}
	else if ((memcmp(&IID_IUccAudioVideoSession,&riid,sizeof(riid))==0))
	{
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccAudioVideoSession\n"));
	}
	else if ((memcmp(&IID_IUccConferenceSession,&riid,sizeof(riid))==0))
	{
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccConferenceSession\n"));
	}
	else if ((memcmp(&IID_IUccApplicationSharingSession,&riid,sizeof(riid))==0))
	{
		g_log.Log(CELOG_DEBUG, _T("Query IID_IUccApplicationSharingSession\n"));
	}
	else if (memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0)
	{
		g_log.Log(CELOG_DEBUG, _T("Query IID_IConnectionPointContainer\n"));
	}
	

	
	sprintf_s(Error,sizeof(Error),"My_NewSession_QueryInterface(%p,%ws)\n",
		This,
		ClassId);
	g_log.Log(CELOG_DEBUG, Error);

	HRESULT hr = OldQueryInterface(
		This,
		riid,
		ppvObj
		);

	sprintf_s(Error,sizeof(Error),"My_NewSession_QueryInterface(%p,%ws) return %x\n",
		This,
		ClassId,
		hr);
	g_log.Log(CELOG_DEBUG, Error);

	if(hr == S_OK) {
		if((memcmp(&IID_IUccInstantMessagingSession,&riid,sizeof(riid))==0))
		{
			CComPtr<IUccInstantMessagingSession> pUccInstantMessagingSession = (IUccInstantMessagingSession*)(*ppvObj);
			HookIMSession(pUccInstantMessagingSession);
		} else if (_wcsnicmp(ClassId, L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", 
			wcslen(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}")) == 0) {
				IUccConferenceSession *pConferenceSession = (IUccConferenceSession *)(*ppvObj);
				HookConferenceSession(pConferenceSession);
		} else if (memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0) {
			g_log.Log(CELOG_DEBUG, _T("In Session query interface IConnectionPointContainer\n"));
			IConnectionPointContainer* pConnectionPointContainer = (IConnectionPointContainer*)(*ppvObj);
			HookConnectionPointContainer(pConnectionPointContainer);
		} 
	}
	return hr;	
}

HRESULT __stdcall My_NewSessionMan_CreateSession(
	IUccSessionManager* This,
	enum UCC_SESSION_TYPE enSessionType,
struct IUccContext * pContext,
struct IUccSession * * ppSession 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSIONMAN_CREATESESSION OldCreateSession = NULL;

		{
			(&gMutexMapSessionMan)->lock();
			MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
			if(it != gMapSessionMan.end())
			{
				OldCreateSession = it->second.CreateSession;
			}
			(&gMutexMapSessionMan)->unlock();
		}

		if(!OldCreateSession)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateSession\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateSession(	
			This,
			enSessionType,
			pContext,
			ppSession );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSIONMAN_CREATESESSION OldCreateSession = NULL;

	{
		MUTEX _mutex_(&gMutexMapSessionMan);
		MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
		if(it != gMapSessionMan.end())
		{
			OldCreateSession = it->second.CreateSession;
		}
	}

	if(!OldCreateSession)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateSession\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	// Add By Jacky.Dong 2011-11-21
	// ----------------------------
	// Get the real sessionType, if bModifySessionType is true, that should modify the sessionType.
	bool bModifySessionType = false;
	// ----------------------------
	if(pContext) {
		IUccProperty *pProperty;
		if((pContext)->get_Property(UCCSC_ASSOCIATED_CONFERENCE_SESSION, &pProperty)
			== S_OK && pProperty)
		{
			g_log.Log(CELOG_DEBUG, _T("CreateSession--Conext: UCCSC_ASSOCIATED_CONFERENCE_SESSION: %p\n"),
			(pProperty));
			// Add By Jacky.Dong 2011-11-21
			// ----------------------------
			// Get the real sessionType, if bModifySessionType is true, that should modify the sessionType.
			bModifySessionType = true;
			// ----------------------------
		}
		if((pContext)->get_Property(UCCSC_TRANSFER_CONTEXT, &pProperty)
			== S_OK && pProperty) 
			g_log.Log(CELOG_DEBUG, _T("CreateSession--Conext: UCCSC_TRANSFER_CONTEXT: %p\n"),
			(pProperty));
	}

	HRESULT hr = OldCreateSession(
		This,
		enSessionType,
		pContext,
		ppSession
		);

	/*WCHAR SessionType[256]={0};
	GetSessionTypeString(SessionType,enSessionType);

	sprintf_s(Error,sizeof(Error),"My_NewSessionManager_CreateSession(%p,%s,%p,%p) return %x\n",
	This,
	SessionType,
	pContext,
	ppSession? (*ppSession):NULL,
	hr);

	g_log.Log(CELOG_DEBUG, Error);*/

	if(hr == S_OK && ppSession)
	{
		WCHAR SessionType[256]={0};
		GetSessionTypeString(SessionType,enSessionType);
		CComPtr<IUccSession> pUccSession = *ppSession;
		UCC_SESSION_TYPE uccSType;
		pUccSession->get_Type(&uccSType);
		g_log.Log(CELOG_DEBUG, _T("My_NewSessionManager_CreateSession: %p type=%s, session type = %d\n"), pUccSession, SessionType, uccSType);
		// Add By Jacky.Dong 2011-11-21
		// ----------------------------
		// Get the real sessionType, if bModifySessionType is true, that should modify the sessionType.
		if(true == bModifySessionType)
		{
			HWND winHandle = FindActiveChatWnd();
			ocePolicyEval.ModifySessionType(winHandle, uccSType);
		}
		// ----------------------------
		HookSession(pUccSession);
		//Add this thread's entry in the SessionThreadMap
		ocePolicyEval.AddSessionWindowPair(GetForegroundWindow(), pUccSession);


		//	comment by Ben, 2011-12-14
		//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
		//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
		//	in the existing code, there are some evaluations already,
		//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
		//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
		//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
		//	then we have two cases:
		//	case 1,
		//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
		//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
		//	case 2,
		//	on the other side, if an audio/video session is advised, 
		//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
		//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
		CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
		ins->AddSessionForWnd(GetForegroundWindow(), pUccSession, uccSType);

		if(enSessionType == UCCST_CONFERENCE) 
		{
			CComPtr<IUccConferenceSession> pConfSession;
			GUID guidValue; 
			if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", &guidValue) == NOERROR ) //IUccConferenceSession
			{
					pUccSession->QueryInterface(guidValue, (void **)(&pConfSession));
					HookConferenceSession(pConfSession);
					if (pConfSession)
					{
						//	we store "pUccSession" and pConfSession as a pair
						//	in future we'll query *pUccSession* by pConfSession
						ins->AddConfSessionPair(pUccSession, pConfSession);
					}
					
			} 
			else
			{
				g_log.Log(CELOG_DEBUG, _T("Failed to query Conference interface\n"));			
			}
		}
	}
	return hr;
}

HRESULT __stdcall My_NewSessionMan_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

		{
			(&gMutexMapSessionMan)->lock();
			MAP_SESSIONMAN::iterator it = gMapSessionMan.find((IUccSessionManager*)This);
			if(it != gMapSessionMan.end())
			{
				OldQueryInterface = it->second.QueryInterface;
			}
			(&gMutexMapSessionMan)->unlock();
		}

		if(!OldQueryInterface)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldQueryInterface(
			This,
			riid,
			ppvObj
			);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionMan_QueryInterface\n"));
	char Error[256]={0};
	FUNC_IUNKNOWN_QUERYINTERFACE OldQueryInterface = NULL;

	{
		MUTEX _mutex_(&gMutexMapSessionMan);
		MAP_SESSIONMAN::iterator it = gMapSessionMan.find((IUccSessionManager*)This);
		if(it != gMapSessionMan.end())
		{
			OldQueryInterface = it->second.QueryInterface;
		}
	}

	if(!OldQueryInterface)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old QueryInterface\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldQueryInterface(
		This,
		riid,
		ppvObj
		);

	return hr;
}

HRESULT __stdcall My_NewSessionMan_RegisterSessionDescriptionEvaluator(
	IUccSessionManager* This,
struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR OldRegisterSessionDescriptionEvaluator = NULL;

		{
			(&gMutexMapSessionMan)->lock();
			MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
			if(it != gMapSessionMan.end())
			{
				OldRegisterSessionDescriptionEvaluator = it->second.RegisterSessionDescriptionEvaluator;
			}
			(&gMutexMapSessionMan)->unlock();
		}

		if(!OldRegisterSessionDescriptionEvaluator)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old RegisterSessionDescriptionEvaluator\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldRegisterSessionDescriptionEvaluator(
			This,
			pSessionDescriptionEvaluator
			);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR OldRegisterSessionDescriptionEvaluator = NULL;

	{
		MUTEX _mutex_(&gMutexMapSessionMan);
		MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
		if(it != gMapSessionMan.end())
		{
			OldRegisterSessionDescriptionEvaluator = it->second.RegisterSessionDescriptionEvaluator;
		}
	}

	if(!OldRegisterSessionDescriptionEvaluator)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old RegisterSessionDescriptionEvaluator\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldRegisterSessionDescriptionEvaluator(
		This,
		pSessionDescriptionEvaluator
		);

	sprintf_s(Error,sizeof(Error),"My_NewSessionManager_RegisterSessionDescriptionEvaluator(%p) return %x\n",
		This,hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall My_NewSessionMan_UnregisterSessionDescriptionEvaluator(
	IUccSessionManager* This
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR OldUnregisterSessionDescriptionEvaluator = NULL;

		{
			(&gMutexMapSessionMan)->lock();
			MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
			if(it != gMapSessionMan.end())
			{
				OldUnregisterSessionDescriptionEvaluator = it->second.UnregisterSessionDescriptionEvaluator;
			}
			(&gMutexMapSessionMan)->unlock();
		}

		if(!OldUnregisterSessionDescriptionEvaluator)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old UnregisterSessionDescriptionEvaluator\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldUnregisterSessionDescriptionEvaluator(
			This
			);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR OldUnregisterSessionDescriptionEvaluator = NULL;

	{
		MUTEX _mutex_(&gMutexMapSessionMan);
		MAP_SESSIONMAN::iterator it = gMapSessionMan.find(This);
		if(it != gMapSessionMan.end())
		{
			OldUnregisterSessionDescriptionEvaluator = it->second.UnregisterSessionDescriptionEvaluator;
		}
	}

	if(!OldUnregisterSessionDescriptionEvaluator)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old UnregisterSessionDescriptionEvaluator\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldUnregisterSessionDescriptionEvaluator(
		This
		);

	sprintf_s(Error,sizeof(Error),"My_NewSessionManager_UnregisterSessionDescriptionEvaluator(%p) return %x\n",
		This,hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall Try_NewSessionMan_CreateSession(
	IUccSessionManager* This,
	enum UCC_SESSION_TYPE enSessionType,
struct IUccContext * pContext,
struct IUccSession * * ppSession 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionMan_CreateSession beg\n");

	__try
	{
		res = My_NewSessionMan_CreateSession(	
						This,
						enSessionType,
						pContext,
						ppSession );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionMan_CreateSession end\n");
	return res;
}

HRESULT __stdcall Try_NewSessionMan_RegisterSessionDescriptionEvaluator(
	IUccSessionManager* This,
struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionMan_RegisterSessionDescriptionEvaluator beg\n");

	__try
	{
		res = My_NewSessionMan_RegisterSessionDescriptionEvaluator(	
					This,
					pSessionDescriptionEvaluator );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionMan_RegisterSessionDescriptionEvaluator end\n");
	return res;
}


HRESULT __stdcall Try_NewSessionMan_UnregisterSessionDescriptionEvaluator(
	IUccSessionManager* This
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionMan_UnregisterSessionDescriptionEvaluator beg\n");

	__try
	{
		res = My_NewSessionMan_UnregisterSessionDescriptionEvaluator(	
			This);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionMan_UnregisterSessionDescriptionEvaluator end\n");
	return res;
}

HRESULT __stdcall Try_NewSessionMan_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionMan_QueryInterface beg\n");

	__try
	{
		res = My_NewSessionMan_QueryInterface(	
						This,
						riid,
						ppvObj);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionMan_QueryInterface end\n");
	return res;
}

HRESULT __stdcall My_NewSession_CreateParticipant (
	IUccSession* This,
	struct IUccUri * pUri,
	struct IUccContext * pContext,
	struct IUccSessionParticipant * * ppParticipant 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSION_CREATEPARTICIPANT OldCreateParticipant = NULL;

		{
			(&gMutexMapSession)->lock();
			MAP_SESSION::iterator it = gMapSession.find(This);
			if(it != gMapSession.end())
			{
				OldCreateParticipant = it->second.CreateParticipant;
			}
			(&gMutexMapSession)->unlock();
		}

		if(!OldCreateParticipant)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateParticipant\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateParticipant(
			This,
			pUri,
			pContext,
			ppParticipant );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSION_CREATEPARTICIPANT OldCreateParticipant = NULL;

	{
		MUTEX _mutex_(&gMutexMapSession);
		MAP_SESSION::iterator it = gMapSession.find(This);
		if(it != gMapSession.end())
		{
			OldCreateParticipant = it->second.CreateParticipant;
		}
	}

	if(!OldCreateParticipant)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old CreateParticipant\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	if(pContext) {
		IUccProperty *pProperty;
		if((pContext)->get_Property(UCCSC_ASSOCIATED_CONFERENCE_SESSION, &pProperty)
			== S_OK && pProperty) 
			g_log.Log(CELOG_DEBUG, _T("CreateParticipant--Conext: UCCSC_ASSOCIATED_CONFERENCE_SESSION: %p\n"),
			(pProperty));
		if((pContext)->get_Property(UCCSC_TRANSFER_CONTEXT, &pProperty)
			== S_OK && pProperty) 
			g_log.Log(CELOG_DEBUG, _T("CreateParticipant--Conext: UCCSC_TRANSFER_CONTEXT: %p\n"),
			(pProperty));
	}

	HRESULT hr = OldCreateParticipant(
		This,
		pUri,
		pContext,
		ppParticipant
		);

	BSTR bstrUri = NULL;
	pUri->get_AddressOfRecord(&bstrUri);

	//	my observation:
	//	suppose the code is run on A.
	//	on these situation it will be here
	//	A send audio call to B (before B accept the invitation)
	//	bstrUri is B. the session is a AVD session
	sprintf_s(Error,sizeof(Error),"My_NewSession_CreateParticipant(%p,%ws,%p,%p) return %x\n",
		This,
		bstrUri?bstrUri:L"NULL",
		pContext,
		ppParticipant?(*ppParticipant):NULL,
		hr);
	g_log.Log(CELOG_DEBUG, Error);
	SysFreeString(bstrUri);

	if (ppParticipant)
	{
		HookSessionParticipant(*ppParticipant);
	}
	
	return hr;
}

HRESULT __stdcall My_NewSession_AddParticipant (
	IUccSession* This,
struct IUccSessionParticipant * pParticipant,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSION_ADDPARTICIPANT OldAddParticipant = NULL;

		{
			(&gMutexMapSession)->lock();
			MAP_SESSION::iterator it = gMapSession.find(This);
			if(it != gMapSession.end())
			{
				OldAddParticipant = it->second.AddParticipant;
			}
			(&gMutexMapSession)->unlock();
		}

		if(!OldAddParticipant)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old AddParticipant\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldAddParticipant (
			This,
			pParticipant,
			pOperationContext );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSION_ADDPARTICIPANT OldAddParticipant = NULL;

	{
		MUTEX _mutex_(&gMutexMapSession);
		MAP_SESSION::iterator it = gMapSession.find(This);
		if(it != gMapSession.end())
		{
			OldAddParticipant = it->second.AddParticipant;
		}
	}

	if(!OldAddParticipant)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old AddParticipant\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}


	wchar_t Uri[256]={0};
	if(pParticipant)
	{
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_Value(&bstrUri); // Modified By Jacky.Dong 2011-11-21 Get the Uri value.
		if(SUCCEEDED(hr)&&bstrUri) wcscpy_s(Uri,sizeof(Uri)/sizeof(wchar_t),bstrUri);
		SysFreeString(bstrUri);
	}

	enum UCC_SESSION_TYPE SessionType;
	This->get_Type(&SessionType);

	HWND winHandle = FindActiveChatWnd();

	g_log.Log(CELOG_DEBUG, _T("Add participant %s to session %p, session type: %d, window handle: %d\n"), Uri, This, SessionType, winHandle);


	//	try to get conf id if the session type is UCCST_APPLICATION_SHARING
	if ( SessionType == UCCST_APPLICATION_SHARING )  
	{
		wstring confID;
		if ( GetConfIDFromUri(Uri, GRUU_CONF_APP_SHARING, confID) )
		{
			CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
			BOOL res = ins->SetShareSessionFlag(confID);

			if (res)
			{
				CComPtr<IUccSession> pConfSession = ins->GetSessionByConfID(confID);
				if (pConfSession)
				{
					if (!ocePolicyEval.DoShareEvalOnSession(pConfSession))
					{
						return E_FAIL;
					}
				}
			}
		}
	}
	
	
	// Added By Jacky.Dong 2011-11-23
	// ------------------------------
	// Do Invite Evaluation when all participant have joined the group and before all kinds of other Evaluation.
	bool bInvite = ocePolicyEval.DoInviteEvaluation(Uri, SessionType);
	if(!bInvite)
	{
		// OldAddParticipant(This, pParticipant, pOperationContext); ???
		bool removeres = ResetWindowTitleForRemove(winHandle);
		g_log.Log(CELOG_DEBUG, _T("Add participant(%s) failed, and then remove %s...\n"), Uri, removeres ? _T("success") : _T("failed"));
		return E_FAIL;
	}
	// ------------------------------
	
	if(SessionType != UCCST_INSTANT_MESSAGING) {
		if(!ocePolicyEval.IsInGroupChatSession(winHandle)) {
			//Not in the IM group of outgoing session
			g_log.Log(CELOG_DEBUG, _T("Not in IM group of outgoing session\n"));
			bool bAllow;
			bool bDummy;
			if(wcsstr(Uri, L"app:conf:audio-video:") && SessionType == UCCST_AUDIO_VIDEO) {
				g_log.Log(CELOG_DEBUG, _T("This is an AVDCALL leaded by %s\n"), Uri);
				bAllow=ocePolicyEval.DoGroupNonIMEvalOnActiveIncomingSession(UCCST_AUDIO_VIDEO, Uri, bDummy);
				if(!bAllow) {
					//We return E_FAIL instead of S_OK. So that the IM windows can still invite more people. 

					//	comment by Ben, 2011-12-16
					//	old code return E_FAIL can't deny completely, the conversation can still be possible to establish, 
					//	so, let's leave conference session
					{
						//	get conference id from audio video session URI, e.g app:conf:audio-video:id:872D02FE0341334DA842437FE71E5FC3
						wstring wstrURI(Uri);
						wstring wstridName(L"app:conf:audio-video:id:");
						wstring::size_type pos = wstrURI.find(wstridName);
						if (pos != wstring::npos)
						{
							wstring confID = wstrURI.substr(pos + wstridName.length(), wstrURI.length() - (pos + wstridName.length()));
							g_log.Log(CELOG_DEBUG, L"conference id %s\n", confID.c_str());
							//	get IUccSession pointer by conf ID
							CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
							CComPtr<IUccSession> pMySession = ins->GetSessionByConfID(confID);
							if (pMySession)
							{								
								//the local participant needs to leave 
								//from this communication
								GUID GuidValue; 
								IUccConferenceSession *pConfSession = NULL;
								if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
									&GuidValue) == NOERROR ) 
								{
									pMySession->QueryInterface(GuidValue, (void **)(&pConfSession));
								}
								if (pConfSession)
								{
									g_log.Log(CELOG_DEBUG, _T("leave conf Session(%p)\n"), pConfSession);
									pConfSession->Leave(NULL);
								}
							}
						}
					}
					return E_FAIL; //UCC_E_SIP_AUTHENTICATION_FAILED;
				}
			} else if (SessionType == UCCST_APPLICATION_SHARING) {
				// Do Sharing Evaluation, Added By Jacky.Dong 2011-11-23
				g_log.Log(CELOG_DEBUG, _T("AddParticipant: Sharing with %s\n"), Uri);

				bAllow = ocePolicyEval.EvalAddParticipant(Uri, SessionType);
				if(!bAllow) {
					return E_FAIL;
				}
			} else if (!(wcsstr(Uri, L";gruu;opaque=app:conf:focus:id:") && SessionType == UCCST_APPLICATION)) {
				//Group livemeeting is evaluated at CreateProcess
				//When the condition 
				//(wcsstr(Uri, L";gruu;opaque=app:conf:focus:id:") && SessionType == UCCST_APPLICATION)
				//is met, it might be invitation to IM chat.
				g_log.Log(CELOG_DEBUG, _T("This is an %s including %s\n"), 
					SessionType == UCCST_APPLICATION?L"Livemeeting":L"AVDCALL",
					Uri);

				g_log.Log(CELOG_DEBUG, L"Conference call (chat or voice/video) (AddParticipant)\n");
				bAllow = ocePolicyEval.EvalAddParticipant(Uri,SessionType);

				if(!bAllow) {
					return E_FAIL;
				}
			}
		} else if(wcsstr(Uri, L"app:conf:audio-video:") &&
			SessionType == UCCST_AUDIO_VIDEO){
				if(winHandle==NULL)
					g_log.Log(CELOG_DEBUG, _T("winHandle is NULL\n"));

				g_log.Log(CELOG_DEBUG, L"Try to do evaluation for audio-video conference call, window: %d\n", winHandle);
				//For group AVD. Try to initiate a group AVD session.
				ocePolicyEval.ModifySessionType(winHandle, UCCST_AUDIO_VIDEO);
				//we do policy for the case that group IM changed to AVDCALL
				bool bAllow=ocePolicyEval.DoGroupChatEvaluation(winHandle, Uri);
				if(!bAllow) {
					//We return E_FAIL instead of S_OK. So that the IM windows can still invite more people. 
					return E_FAIL; 
				}
				else
				{
					//	get conference id from audio video session URI, e.g app:conf:audio-video:id:872D02FE0341334DA842437FE71E5FC3
					wstring wstrURI(Uri);
					wstring wstridName(L"app:conf:audio-video:id:");
					wstring::size_type pos = wstrURI.find(wstridName);
					if (pos != wstring::npos)
					{
						wstring confID = wstrURI.substr(pos + wstridName.length(), wstrURI.length() - (pos + wstridName.length()));
						g_log.Log(CELOG_DEBUG, L"conference id %s\n", confID.c_str());
						//	get IUccSession pointer by conf ID
						CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
						CComPtr<IUccSession> pMySession = ins->GetSessionByConfID(confID);
						if (pMySession)
						{
							bool b_allow = ocePolicyEval.DoEvalOnSession(pMySession, CE_ACTION_AVD);
							if(!b_allow)
							{
								//If not allowed, the local participant needs to leave 
								//from this communication
								GUID GuidValue; 
								IUccConferenceSession *pConfSession = NULL;
								if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
									&GuidValue) == NOERROR ) 
								{
									pMySession->QueryInterface(GuidValue, (void **)(&pConfSession));
								}
								if (pConfSession)
								{
									g_log.Log(CELOG_DEBUG, _T("leave conf Session(%p)\n"), pConfSession);
									pConfSession->Leave(NULL);
								}
							}
						}
						
					}
										
				}
		} else { //In the group session
			int numParticipant;
			
			GetCurrentSessionParticipantNum(winHandle, numParticipant);

			g_log.Log(CELOG_DEBUG, L"Add a new participant in the group: %s, num: %d", Uri, numParticipant);

			if(numParticipant==0)
				numParticipant=2; //peer-to-peer
			ocePolicyEval.UpdateGroupParticipantNum(winHandle,numParticipant);
			if(ocePolicyEval.IsSessionTypeChanged(winHandle, This, Uri)) {
				//The session type changed and no new participant added
				//We reevaluate this session with new type.
				g_log.Log(CELOG_DEBUG, _T("Group Session changed\n"));
				bool bAllow=ocePolicyEval.DoGroupChatEvaluation(winHandle, Uri);
				if(!bAllow) {
					return S_OK; 
				}	
			} else {
				bool bAddNow=false;
				ocePolicyEval.AddParticipantToGroupChat(winHandle, Uri, This, 
					pParticipant,
					pOperationContext,
					OldAddParticipant,
					bAddNow);
				// in group, if the middle ones should be deny, the result is the last one be denied.
				if(true) { // (ocePolicyEval.IsTimeToDoEvaluation(winHandle)) {
					g_log.Log(CELOG_DEBUG, L"Send message... group chat: %s", Uri);

					bool bUserCancel = FALSE;
					IUccConferenceSession* pConfSession = NULL;
					HWND h_group_chat_wnd = NULL;
					bool bAllow=ocePolicyEval.DoGroupChatEvaluation_V2(winHandle, Uri, bUserCancel, &pConfSession, &h_group_chat_wnd);

					if(!bAllow) {
						// if a participant get deny result, should remove from its group.
						ResetWindowTitleForRemove(winHandle);

						if(bUserCancel && pConfSession) 
						{
							g_log.Log(CELOG_DEBUG, _T("user cancel, leave the IUccConferenceSession [%p]\n"), pConfSession);

							pConfSession->Leave(NULL);

							if (h_group_chat_wnd)
							{
								g_log.Log(CELOG_DEBUG, _T("user cancel, close window [%d]\n"), h_group_chat_wnd);
								::PostMessage(h_group_chat_wnd, WM_CLOSE, NULL, NULL);
							}
						}

						return E_FAIL; 
					}
				} else {
					g_log.Log(CELOG_DEBUG, _T("Check addnow.\n")); 
					if(!bAddNow)
						return S_OK; //Not time to do evaluation, hold adding. 
					//else
					//This group session might be started from an existing peer-to-peer session. 
					//This participant might be in the previous session already (See 
					//InitGroupSession function). Call the real participant so OC can 
					//add the next one. 
					g_log.Log(CELOG_DEBUG, _T("Call the real participant so OC can add the next one.\n")); 
				}
			}
		}
	} else { // SessionType == UCCST_INSTANT_MESSAGING
		if(!wcsstr(Uri, L"app:conf:chat:")){
			//peer-to-peer IM
			bool bAllow = true;
			g_log.Log(CELOG_DEBUG, L"Send message... peer to peer IM, IUccSession: %p\n", This);
			bAllow = ocePolicyEval.EvalAddParticipant(Uri,SessionType);
			
			g_log.Log(CELOG_DEBUG, _T("Add %s to session %p, IUccSessionParticipant: %p, IUccContext: %p\n"), Uri, This, pParticipant, pOperationContext);
			
			if(!bAllow) {
				return E_FAIL; 
			}
			
		} else {
			wchar_t szTitle[512] = {0};
			if (winHandle)
			{
				GetWindowTextW(winHandle, szTitle, 500);
			}
			
			g_log.Log(CELOG_DEBUG, L"AddParticipant: %s, session: %p, type: %d, window handle: %d, window title: %s\n", Uri, This, SessionType, winHandle, szTitle);
			ocePolicyEval.ModifySessionType(winHandle, UCCST_INSTANT_MESSAGING);
			//It might be the end of atomic operation of setting up an incoming IM group session 
			ocePolicyEval.SetActiveGroupIncomingSesion(NULL);


			bool bAllow=ocePolicyEval.DoGroupChatEvaluation(winHandle, Uri);

			if(!bAllow) {
				g_log.Log(CELOG_DEBUG, L"denied IM after voice call in group chat\n");
				return E_FAIL; 
			}
		}
	}

	HRESULT hr = OldAddParticipant(
		This,
		pParticipant,
		pOperationContext
		);

	g_log.Log(CELOG_DEBUG, _T("Add participant result: %d\n"), hr);

	return hr;
}

HRESULT __stdcall My_NewSession_RemoveParticipant (
	IUccSession* This,
struct IUccSessionParticipant * pParticipant,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSION_REMOVEPARTICIPANT OldRemoveParticipant = NULL;

		{
			(&gMutexMapSession)->lock();
			MAP_SESSION::iterator it = gMapSession.find(This);
			if(it != gMapSession.end())
			{
				OldRemoveParticipant = it->second.RemoveParticipant;
			}
			(&gMutexMapSession)->unlock();
		}

		if(!OldRemoveParticipant)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old RemoveParticipant\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldRemoveParticipant (
			This,
			pParticipant,
			pOperationContext );
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSION_REMOVEPARTICIPANT OldRemoveParticipant = NULL;

	{
		MUTEX _mutex_(&gMutexMapSession);
		MAP_SESSION::iterator it = gMapSession.find(This);
		if(it != gMapSession.end())
		{
			OldRemoveParticipant = it->second.RemoveParticipant;
		}
	}

	if(!OldRemoveParticipant)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old RemoveParticipant\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	wchar_t Uri[256]={0};
	if(pParticipant)
	{
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) wcscpy_s(Uri,sizeof(Uri)/sizeof(wchar_t),bstrUri);
		SysFreeString(bstrUri);
	}

	HRESULT hr = OldRemoveParticipant(
		This,
		pParticipant,
		pOperationContext
		);

	sprintf_s(Error,sizeof(Error),"My_NewSession_RemoveParticipant(%p,%p(%ws),%p) return %x\n",
		This,
		pParticipant,
		Uri,
		pOperationContext,
		hr);
	g_log.Log(CELOG_DEBUG, Error);
	return hr;
}

HRESULT __stdcall My_NewSession_Terminate (
										IUccSession* This,
										enum UCC_REJECT_OR_TERMINATE_REASON enTerminateReason,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[256]={0};
		FUNC_SESSION_TERMINATE OldTerminate = NULL;

		{
			(&gMutexMapSession)->lock();
			MAP_SESSION::iterator it = gMapSession.find(This);
			if(it != gMapSession.end())
			{
				OldTerminate = it->second.Terminate;
			}
			(&gMutexMapSession)->unlock();
		}

		if(!OldTerminate)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Terminate\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldTerminate  (
			This,
			enTerminateReason,
			pOperationContext );
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[256]={0};
	FUNC_SESSION_TERMINATE OldTerminate = NULL;

	{
		MUTEX _mutex_(&gMutexMapSession);
		MAP_SESSION::iterator it = gMapSession.find(This);
		if(it != gMapSession.end())
		{
			OldTerminate = it->second.Terminate;
		}
	}

	if(!OldTerminate)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! %p hasn't old Terminate\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	CComPtr<IUccCollection> pParticipants; 
	CComQIPtr<IUccSessionParticipant> pParticipant;
	long numParticipants;
	This->get_Participants(&pParticipants );
	pParticipants->get_Count(&numParticipants);
	for(int i=1; i<=numParticipants; i++) {
		CComVariant vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = vtItem.pdispVal;
		BSTR bstrUri = NULL;IUccUri* pUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
		if(SUCCEEDED(hr) && bstrUri) {
			g_log.Log(CELOG_DEBUG, _T("Session(%p) has participant(%d): %s\n"), This, numParticipants,
				bstrUri);
		}
		SysFreeString(bstrUri);
	}

	HRESULT hr = OldTerminate(
		This,
		enTerminateReason,
		pOperationContext
		);

	//Remove this thread's entry in the SessionThreadMap
	ocePolicyEval.RemoveSessionWindowPair(GetForegroundWindow(), This);
	//Remove this thread's copy evaluation result  
	ocePolicyEval.RemoveCachedCopyEvalResults(GetForegroundWindow());
	//Remove this session from incomingGroupSet if it exists in it
	ocePolicyEval.RemoveCachedIncommingGroupSession(This);

	//	comment by Ben, 2011-12-14
	//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
	//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
	//	in the existing code, there are some evaluations already,
	//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
	//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
	//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
	//	then we have two cases:
	//	case 1,
	//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
	//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
	//	case 2,
	//	on the other side, if an audio/video session is advised, 
	//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
	//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
	//	so, here, the code below is to remove data from CLiveSessionWnd
	CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
	ins->RemoveSessionForWnd(GetForegroundWindow(), This);

	enum UCC_SESSION_TYPE enType;
	This->get_Type(&enType);
	g_log.Log(CELOG_DEBUG, _T("My_NewSession_Terminate(%p)\n"), This);
	return hr;
}

HRESULT __stdcall Try_NewSession_CreateParticipant (
	IUccSession* This,
	struct IUccUri * pUri,
	struct IUccContext * pContext,
	struct IUccSessionParticipant * * ppParticipant 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSession_CreateParticipant beg\n");

	__try
	{
		res = My_NewSession_CreateParticipant (
					This,
					pUri,
					pContext,
					ppParticipant );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSession_CreateParticipant end\n");
	return res;
}

HRESULT __stdcall Try_NewSession_AddParticipant (
	IUccSession* This,
struct IUccSessionParticipant * pParticipant,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSession_AddParticipant beg\n");

	__try
	{
		res = My_NewSession_AddParticipant (
					This,
					pParticipant,
					pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSession_AddParticipant end\n");
	return res;
}

HRESULT __stdcall Try_NewSession_RemoveParticipant (
	IUccSession* This,
struct IUccSessionParticipant * pParticipant,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSession_RemoveParticipant beg\n");
	
	__try
	{
		res = My_NewSession_RemoveParticipant (
					This,
					pParticipant,
					pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSession_RemoveParticipant end\n");
	return res;
}

HRESULT __stdcall Try_NewSession_Terminate (
	IUccSession* This,
	enum UCC_REJECT_OR_TERMINATE_REASON enTerminateReason,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSession_Terminate beg\n");
	
	__try
	{
		res = My_NewSession_Terminate  (
					This,
					enTerminateReason,
					pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSession_Terminate end\n");
	return res;
}

HRESULT __stdcall Try_NewSession_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSession_QueryInterface beg\n");
	
	__try
	{
		res = My_NewSession_QueryInterface(
					This,
					riid,
					ppvObj);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSession_QueryInterface end\n");
	return res;
}

HRESULT __stdcall My_NewIM_Session_SendMessage (
	IUccInstantMessagingSession* This,
	BSTR bstrContentType,
	BSTR bstrMessage,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_IM_SESSION_SENDMESSAGE OldSendMessage = NULL;

		{
			(&gMutexMapIMSession)->lock();
			MAP_IM_SESSION::iterator it = gMapIMSession.find(This);
			if(it != gMapIMSession.end())
			{
				OldSendMessage = it->second.SendMessage;
			}
			(&gMutexMapIMSession)->unlock();
		}

		if(!OldSendMessage)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! IMSession %p hasn't old SendMessage\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldSendMessage (
			This,
			bstrContentType,
			bstrMessage,
			pOperationContext );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_IM_SESSION_SENDMESSAGE OldSendMessage = NULL;

	{
		MUTEX _mutex_(&gMutexMapIMSession);
		MAP_IM_SESSION::iterator it = gMapIMSession.find(This);
		if(it != gMapIMSession.end())
		{
			OldSendMessage = it->second.SendMessage;
		}
	}

	if(!OldSendMessage)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! IMSession %p hasn't old SendMessage\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	if(pOperationContext) {
		IUccContext *pContext;
		if(pOperationContext->get_Context(&pContext)==S_OK && pContext) {
			IUccProperty *pProperty;
			if((pContext)->get_Property(UCCSC_ASSOCIATED_CONFERENCE_SESSION, &pProperty)
				== S_OK && pProperty) 
				g_log.Log(CELOG_DEBUG, _T("SendMessage--Conext: UCCSC_ASSOCIATED_CONFERENCE_SESSION: %p\n"),
				(pProperty));
			if((pContext)->get_Property(UCCSC_TRANSFER_CONTEXT, &pProperty)
				== S_OK && pProperty) 
				g_log.Log(CELOG_DEBUG, _T("SendMessage--Conext: UCCSC_TRANSFER_CONTEXT: %p\n"),
				(pProperty));
		}
	}

	HRESULT qResult;
	CComPtr<IUccSession> pIuccSession=NULL;
	qResult= This->QueryInterface(IID_IUccSession, (void **)&pIuccSession);
	if(qResult != S_OK) {
		g_log.Log(CELOG_DEBUG, _T("QueryInterface failed due to %d\n"), qResult);
		pIuccSession=NULL;
	} else 	
		ocePolicyEval.AddSessionWindowPair(GetForegroundWindow(), pIuccSession);

	UCC_SESSION_TYPE uccType = UCCST_INSTANT_MESSAGING;
	if(pIuccSession)
	{
		pIuccSession->get_Type(&uccType);
	}
	g_log.Log(CELOG_DEBUG, L"Enter My_NewIM_Session_SendMessage, session: %p, content type: %s, session type: %d\n", This, bstrContentType, uccType);

//	g_log.Log(CELOG_DEBUG, _T("Send message type:%s\n msg:%s\n in session:%p\n"), bstrContentType, bstrMessage, This);
	

	CComPtr<IUccCollection> pParticipants; 
	CComQIPtr<IUccSessionParticipant> pParticipant;
	long numParticipants;
	wstring disclaimer=L"";
	
	bool bAllow=true;

	//Do policy evaluation
	if(wcsstr(bstrContentType, L"text/rtf")
		|| NULL != wcsstr(bstrContentType, L"text/html")) { // In OC2007 R2, the disclaimer obligation should be send via content tyle 'text/html', Modified By Jacky.Dong 2011-12-08
		//Check if we need to add a disclaimer
		if(pIuccSession != NULL) {
			pIuccSession->get_Participants(&pParticipants );
			pParticipants->get_Count(&numParticipants);

			g_log.Log(CELOG_DEBUG, L"My_NewIM_Session_SendMessage, participant count in current session: %d\n", numParticipants);

			disclaimer = GetDisclaimer(pParticipants, numParticipants);
		}
	} else if(wcsstr(bstrMessage,L"Application-Name: File Transfer")) {
		//Check if file transfer is allowed
		//Get the transferred file
		wchar_t fileName[256]={0};
		wchar_t *fileNamePos=wcsstr(bstrMessage, L"Application-File: ");
		if(fileNamePos) {
			fileNamePos+=wcslen(L"Application-File: ");
			wchar_t *fileNameEndPos=wcsstr(fileNamePos, L"Application-Fi");
			rsize_t fileNameLen=fileNameEndPos-fileNamePos-2;
			if(fileNameEndPos && fileNameLen>0) {
				wcsncpy_s(fileName, 256, fileNamePos, fileNameLen);
				fileName[fileNameLen]=L'\0';
				g_log.Log(CELOG_DEBUG, _T("file transfer: tid=%d fileName=%s--\n"), GetCurrentThreadId(), 
					fileName);
			}
		}

		if(wcslen(fileName) > 0) {
			//we got the file name.
			//go through each participant and check if he/she is allowed to receive the file 
			if(pIuccSession != NULL) {
				pIuccSession->get_Participants(&pParticipants );
				pParticipants->get_Count(&numParticipants);
				g_log.Log(CELOG_DEBUG, L"My_NewIM_Session_SendMessage (File Transfer), participant count in current session: %d\n", numParticipants);

				for(int i=1; i<=numParticipants; i++) {
					CComVariant vtItem;
					vtItem.vt = VT_DISPATCH;
					pParticipants->get_Item(i, &vtItem);
					pParticipant = vtItem.pdispVal;
					BSTR bstrUri = NULL;IUccUri* pUri = NULL;
					HRESULT hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
					if(SUCCEEDED(hr)&&bstrUri) {
						g_log.Log(CELOG_DEBUG, L"My_NewIM_Session_SendMessage (File Transfer), participant: %s, file path: %s\n", bstrUri, fileName);

						bAllow=ocePolicyEval.EvalTransferFile(bstrUri, fileName);
					}
					SysFreeString(bstrUri);

					if(!bAllow) {
						//Show message box
						WCHAR msg[1024];
						_snwprintf_s(msg, sizeof(msg), L"Transfer file to [%s] is restricted!", bstrUri);   
						/*if(MessageBox(NULL, msg, L"Compliant Enterprise Communicator Enforcer", MB_SERVICE_NOTIFICATION) == 0) {
						sprintf_s(Error,sizeof(Error),"Faile to call MessageBox due to %d\n",
						GetLastError);
						}*/
						return E_FAIL; //UCC_E_SIP_AUTHENTICATION_FAILED;
					}
				}

				//Check if we need to add a disclaimer
				disclaimer = GetDisclaimer(pParticipants, numParticipants);
			}
		}
	} else if(wcsstr(bstrContentType, L"multipart/alternative")) {//group chat

	//	g_log.Log(CELOG_DEBUG, _T("Send message type:%s\n msg:%s\n in session:%p\n"), bstrContentType, bstrMessage, This);

		HWND winHandle=GetForegroundWindow();
		ocePolicyEval.StoreGroupSendMsgFunc(winHandle, 
			OldSendMessage, 
			This, pOperationContext);
	}

	HRESULT hr = OldSendMessage(
		This,
		bstrContentType,
		bstrMessage,
		pOperationContext
		);

	if(!disclaimer.empty()) {
		//peer-to-peer IM
		bool bSendDisclaimer=ocePolicyEval.IsIMSessionFirstMsg(This);
		if(bSendDisclaimer) {
			//Only send disclaimer with the first message of this IM session
			BSTR msgType = SysAllocString(L"text/plain");
			BSTR msg=SysAllocString(disclaimer.c_str());
			This->SendMessageW(msgType, msg, pOperationContext);
			SysFreeString(msgType);
			SysFreeString(msg);
		}
	} 

	if(wcsstr(bstrContentType, L"multipart/alternative")) {
		HWND winHandle=GetForegroundWindow();
		ocePolicyEval.SendGroupDisclaimer(winHandle, This, bstrContentType, pOperationContext);
	}
	return hr;
}

HRESULT __stdcall Try_NewIM_Session_SendMessage (
	IUccInstantMessagingSession* This,
	BSTR bstrContentType,
	BSTR bstrMessage,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewIM_Session_SendMessage beg\n");
	
	__try
	{
		res = My_NewIM_Session_SendMessage (
						This,
						bstrContentType,
						bstrMessage,
						pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewIM_Session_SendMessage end\n");
	return res;
}

HRESULT __stdcall My_NewSubscription_AddPresentity(
	IUccSubscription *This,
	struct IUccPresentity *pPresentity )
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SUBSCRIPTION_ADDPRESENTITY OldAddPresentity = NULL;

		g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_AddPresentity\n"));
		{
			(&gMutexMapSubscription)->lock();

			MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
			if(it != gMapSubscription.end())
			{
				OldAddPresentity = it->second.AddPresentity;
			}
			(&gMutexMapSubscription)->unlock();
		}

		if(!OldAddPresentity)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! AddPresentity %p hasn't old AddPresentity\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldAddPresentity(
			This,
			pPresentity);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SUBSCRIPTION_ADDPRESENTITY OldAddPresentity = NULL;

	g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_AddPresentity\n"));
	{
		MUTEX _mutex_(&gMutexMapSubscription);

		MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
		if(it != gMapSubscription.end())
		{
			OldAddPresentity = it->second.AddPresentity;
		}
	}

	if(!OldAddPresentity)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! AddPresentity %p hasn't old AddPresentity\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	BSTR bstrUri=NULL;IUccUri* pUri = NULL;
	HRESULT res = pPresentity->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
	if(SUCCEEDED(res)&&bstrUri) {
		g_log.Log(CELOG_DEBUG, _T("Add presentity %s\n"), bstrUri);
		if(wcsstr(bstrUri, L"john.tyler")) {
			g_log.Log(CELOG_DEBUG, _T("Not allow to add presentity %s\n"), bstrUri);
			return E_FAIL;
		}
	}

	HRESULT hr = OldAddPresentity(
		This,
		pPresentity);

	return hr;	
}

HRESULT __stdcall My_NewSubscription_AddCategoryName(
	IUccSubscription *This,
	BSTR bstrName
	)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SUBSCRIPTION_ADDCATEGORYNAME OldAddCategoryName = NULL;

		g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_AddCategoryName\n"));
		{
			(&gMutexMapSubscription)->lock();

			MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
			if(it != gMapSubscription.end())
			{
				OldAddCategoryName = it->second.AddCategoryName;
			}

			(&gMutexMapSubscription)->unlock();
		}

		if(!OldAddCategoryName)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! AddCategoryName %p hasn't old AddCategoryName\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}
		return OldAddCategoryName (	
			This,
			bstrName );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SUBSCRIPTION_ADDCATEGORYNAME OldAddCategoryName = NULL;

	g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_AddCategoryName\n"));
	{
		MUTEX _mutex_(&gMutexMapSubscription);

		MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
		if(it != gMapSubscription.end())
		{
			OldAddCategoryName = it->second.AddCategoryName;
		}
	}

	if(!OldAddCategoryName)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! AddCategoryName %p hasn't old AddCategoryName\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}


	//HRESULT res = pPresentity->get_Uri(&bstrUri);
	//if(SUCCEEDED(res)&&bstrUri) {
	//g_log.Log(CELOG_DEBUG, _T("Add categoryName %s\n"), bstrUri);
	if(wcsstr(bstrName, L"calendarData")) {
		//g_log.Log(CELOG_DEBUG, _T("Not allow to add calendar %s\n"), bstrName);
		//return E_FAIL;
	}
	//}

	HRESULT hr = OldAddCategoryName(
		This,
		bstrName);

	CComPtr<IUccCollection> pPresentityCollection;
	CComQIPtr<IUccPresentity> pPresentity;
	HRESULT res=This->get_Presentities(&pPresentityCollection);
	if(SUCCEEDED(res) && pPresentityCollection) {
		long count=0;
		pPresentityCollection->get_Count(&count);
		for(int i=1; i<=count; i++) {
			CComVariant variant;
			variant.vt = VT_DISPATCH;
			res=pPresentityCollection->get_Item(i, &variant);
			if(SUCCEEDED(res) && variant.pdispVal) {
				pPresentity = variant.pdispVal;
				BSTR bstrUri=NULL;IUccUri* pUri = NULL;
				res=pPresentity->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
				if(SUCCEEDED(res) && bstrUri) {
					g_log.Log(CELOG_DEBUG, _T("Add categoryName %s for presentity(%d) %s\n"), bstrName, i, bstrUri);
				}
			} else
				g_log.Log(CELOG_DEBUG, _T("Failed to get item %d from presentity collection\n"), i);
		}
	}
	return hr;	
}

HRESULT __stdcall Try_NewSubscription_AddPresentity(
	IUccSubscription *This,
struct IUccPresentity *pPresentity
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSubscription_AddPresentity beg\n");

	__try
	{
		res = My_NewSubscription_AddPresentity(
						This,
						pPresentity);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSubscription_AddPresentity end\n");
	return res;
}

HRESULT __stdcall Try_NewSubscription_AddCategoryName (
	IUccSubscription *This,
	BSTR bstrName 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSubscription_AddCategoryName beg\n");
	
	__try
	{
		res = My_NewSubscription_AddCategoryName (	
						This,
						bstrName );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSubscription_AddCategoryName end\n");
	return res;
}

HRESULT __stdcall Try_NewConfSession_Enter (
	IUccConferenceSession *This,
	struct IUccUri * pUri,
struct IUccOperationContext * pOperationContext
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConfSession_Enter beg\n");
	
	__try
	{
		res = My_NewConfSession_Enter (	
						This,
						pUri,
						pOperationContext);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConfSession_Enter end\n");
	return res;
}


HRESULT __stdcall My_NewConfSession_Enter (
	IUccConferenceSession *This,
	struct IUccUri * pUri,
struct IUccOperationContext * pOperationContext)
{
	BSTR bstrConfURI;
	pUri->get_Value(&bstrConfURI);

	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONFSESSION_ENTER OldEnter = NULL;


		g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_Enter(%p): %s\n"), This, bstrConfURI);

		{
			(&gMutexMapConfSession)->lock();

			MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
			if(it != gMapConfSession.end())
			{
				OldEnter = it->second.Enter;
			}

			(&gMutexMapConfSession)->unlock();
		}

		if(!OldEnter)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! OldEnter %p hasn't old OldEnter\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldEnter (	
			This,
			pUri,
			pOperationContext);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONFSESSION_ENTER OldEnter = NULL;


	g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_Enter(%p): %s\n"), This, bstrConfURI);

	{
		MUTEX _mutex_(&gMutexMapConfSession);

		MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
		if(it != gMapConfSession.end())
		{
			OldEnter = it->second.Enter;
		}
	}

	if(!OldEnter)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! OldEnter %p hasn't old OldEnter\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}


	bool bAllow=true;
	const wchar_t *endPos=wcsstr(bstrConfURI, L";gruu;opaque=app:conf:focus:id:");
	if(endPos) 
	{
		//This is a peer-to-peer conference IM/AVDCALL for live meeting conference 
		//living meeting comes with an IM window
		//Get conference leader
		WCHAR confLeader[1024];
		wcsncpy_s(confLeader, bstrConfURI, endPos-bstrConfURI);
		confLeader[endPos-bstrConfURI]=L'\0';
		g_log.Log(CELOG_DEBUG, L"conf leader: %s", confLeader);

		CComPtr<IUccSession> pIuccSession;
		HRESULT qResult= This->QueryInterface(IID_IUccSession, (void **)&pIuccSession);
		if(qResult == S_OK) 
		{
			CComPtr<IUccSessionParticipant> pParticipant;
			pIuccSession->get_LocalParticipant(&pParticipant);
			BSTR bstrUri = NULL;
			IUccUri* pUri2 = NULL;
			pParticipant->get_Uri(&pUri2);
			pUri2->get_AddressOfRecord(&bstrUri);
			g_log.Log(CELOG_DEBUG, L"local participant: %s", bstrUri);

			//Let's get its conference ID
			WCHAR confMeetingID[1024];
			confMeetingID[0]=L'\0';
			endPos=wcsstr(bstrConfURI, L";gruu;opaque=app:conf:focus:id:");
			if(endPos != NULL)
				endPos+=wcslen(L";gruu;opaque=app:conf:focus:id:");
			else {
				endPos=wcsstr(bstrConfURI, L";gruu;opaque=app:conf:audio-video:id:");
				if(endPos != NULL)
					endPos+=wcslen(L";gruu;opaque=app:conf:audio-video:id:");
			}
			if(endPos != NULL) {
				size_t len=wcslen(endPos);
				wcsncpy_s(confMeetingID, endPos, len);
				confMeetingID[len]=L'\0';

				//	we get the conference id here,
				g_log.Log(CELOG_DEBUG, L"conference id %s\n", confMeetingID);
				//	add conf ID to IUccSession pointer, we find IUccSession by IUccConferenceSession pointer
				CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
				wstring wstrConfID(confMeetingID);
				ins->AddConfID(wstrConfID, This);
			}

			if(wcscmp(bstrUri, confLeader)) 
			{
				//The leader is different from local participant, this is a incoming session
				//Let first check if this session has been stored. 
				bool bStored=ocePolicyEval.FindIncomingSessionAndCheckEvalResult(confMeetingID, bAllow);
				if(bStored)
				{
					g_log.Log(CELOG_DEBUG, _T("We find cached result (%s) is %d\n"), confMeetingID, bAllow?1:0);
				}
				if(!bStored) 
				{
					//We need to evaluation to see if these two can communicate in order to prevent
					//the IM window. No any obligations (including log, notifier, warn, disclaimer)
					bAllow=ocePolicyEval.EvalAddParticipant(confLeader, UCCST_INSTANT_MESSAGING);
				}
			}
			SysFreeString(bstrUri);
		}
	}

	if(!bAllow) 
		return S_OK;

	HRESULT hr = OldEnter(
		This,
		pUri,
		pOperationContext);

	if(hr == S_OK) {
		int numParticipant;
		HWND winHandle=GetForegroundWindow();
		GetCurrentSessionParticipantNum(winHandle, numParticipant);
		
		if(numParticipant >= 1)
		{
			ocePolicyEval.InitGroupSession(winHandle, This, numParticipant, bstrConfURI);
		}

		if(numParticipant==0)
			numParticipant=2; //peer-to-peer

		CComPtr<IUccSession> pSession;
		This->QueryInterface(IID_IUccSession, (void **)(&pSession));
		CComPtr<IUccCollection> pParticipants; 
		long numParticipants;
		pSession->get_Participants(&pParticipants );
		pParticipants->get_Count(&numParticipants);
		g_log.Log(CELOG_DEBUG, _T("In conference, there are %d participants, session: %p\n"), numParticipants, pSession);
	}

	SysFreeString(bstrConfURI);

	return hr;	
}

HRESULT __stdcall Try_NewConfSession_Leave (
	IUccConferenceSession *This,
struct IUccOperationContext * pOperationContext
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConfSession_Leave beg\n");

	__try
	{
		res = My_NewConfSession_Leave (		This,
			pOperationContext);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConfSession_Leave end\n");
	return res;
}


HRESULT __stdcall My_NewConfSession_Leave (
										IUccConferenceSession *This,
struct IUccOperationContext * pOperationContext)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONFSESSION_LEAVE OldLeave = NULL;

		g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_Leave(%p)\n"), This);
		{
			(&gMutexMapConfSession)->lock();

			MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
			if(it != gMapConfSession.end())
			{
				OldLeave = it->second.Leave;
			}

			(&gMutexMapConfSession)->unlock();
		}

		if(!OldLeave)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! OldLeave %p hasn't old OldLeave\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldLeave(
			This,
			pOperationContext);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONFSESSION_LEAVE OldLeave = NULL;

	g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_Leave(%p)\n"), This);
	{
		MUTEX _mutex_(&gMutexMapConfSession);

		MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
		if(it != gMapConfSession.end())
		{
			OldLeave = it->second.Leave;
		}
	}

	if(!OldLeave)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! OldLeave %p hasn't old OldLeave\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldLeave(
		This,
		pOperationContext);

	ocePolicyEval.RemoveGroupChat(GetForegroundWindow());

	CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
	ins->RemoveSessionItem(This);
	ins = NULL;

	return hr;	
}

HRESULT __stdcall Try_NewConfSession_SetProperty (
	IUccConferenceSession *This,
	enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
	VARIANT vPropertyValue,
struct IUccOperationContext * pOperationContext
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConfSession_SetProperty beg\n");
	
	__try
	{
		res = My_NewConfSession_SetProperty (		This,
			enPropertyId,
			vPropertyValue,
			pOperationContext) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConfSession_SetProperty end\n");
	return res;
}


HRESULT __stdcall My_NewConfSession_SetProperty (
	IUccConferenceSession *This,
	enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
	VARIANT vPropertyValue,
struct IUccOperationContext * pOperationContext)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONFSESSION_SETPROPERTY OldSetProperty = NULL;

		g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_SetProperty\n"));
		{
			(&gMutexMapConfSession)->lock();

			MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
			if(it != gMapConfSession.end())
			{
				OldSetProperty = it->second.SetProperty;
			}

			(&gMutexMapConfSession)->unlock();
		}

		if(!OldSetProperty)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! OldSetProperty %p hasn't old OldSetProperty\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		HRESULT hr = OldSetProperty(
			This,
			enPropertyId,
			vPropertyValue,
			pOperationContext);

		return hr;	
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONFSESSION_SETPROPERTY OldSetProperty = NULL;

	g_log.Log(CELOG_DEBUG, _T("In My_NewConfSession_SetProperty\n"));
	{
		MUTEX _mutex_(&gMutexMapConfSession);

		MAP_CONF_SESSION::iterator it = gMapConfSession.find(This);
		if(it != gMapConfSession.end())
		{
			OldSetProperty = it->second.SetProperty;
		}
	}

	if(!OldSetProperty)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! OldSetProperty %p hasn't old OldSetProperty\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldSetProperty(
		This,
		enPropertyId,
		vPropertyValue,
		pOperationContext);

	return hr;	
}

void HookPlatform(IUccPlatform* pUccPlatform)
{
	if(!pUccPlatform) return;

	MUTEX _mutex_(&gMutexMapPlatform);

	char Error[256]={0};
	if(gMapPlatform.find(pUccPlatform) != gMapPlatform.end())
	{
		sprintf_s(Error,sizeof(Error),"pUccPlatform %p has been hooked\n",
			pUccPlatform);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pUccPlatform);

	OLD_PLATFORM_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_PLATFORM_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[3];
	memcpy(&(Functions.Initialize),&Pointer,sizeof(ULONG));
	Functions.OriInitialize = Functions.Initialize;

	Pointer = Vtable[4];
	memcpy(&(Functions.CreateEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriCreateEndpoint = Functions.CreateEndpoint;

	Pointer = Vtable[5];
	memcpy(&(Functions.CreateProxyEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriCreateProxyEndpoint = Functions.CreateProxyEndpoint;

	Pointer = Vtable[6];
	memcpy(&(Functions.Shutdown),&Pointer,sizeof(ULONG));
	Functions.OriShutdown = Functions.Shutdown;

	for(MAP_PLATFORM::const_iterator it = gMapPlatform.begin(); it != gMapPlatform.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			// 这里用||，严格的做法应该是记录哪些相同，不同的还是应该HOOK，相同的直接赋值即可
			if( it->second.OriCreateEndpoint == Functions.OriCreateEndpoint ||
				it->second.OriCreateProxyEndpoint == Functions.OriCreateProxyEndpoint ||
				it->second.OriInitialize == Functions.OriInitialize ||
				it->second.OriShutdown == Functions.OriShutdown
				)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccPlatform %p has same vtable with %p, so we do not need to hook\n",
				pUccPlatform,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			// 因为在My_NewSession_Xxx中需要根据pSession寻找Old_Xxx，所以需要把这一项加上，但是不Hook
			OLD_PLATFORM_FUNCS Function = it->second;
			gMapPlatform.insert(std::make_pair(pUccPlatform,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook Platform %p\n", pUccPlatform);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriInitialize,(PVOID)Try_NewPlatform_Initialize ,(PVOID*)&Functions.Initialize ) ;
	HookCode( (PVOID)Functions.OriCreateEndpoint, (PVOID)Try_NewPlatform_CreateEndpoint, (PVOID*)&Functions.CreateEndpoint);
	HookCode( (PVOID)Functions.OriCreateProxyEndpoint, (PVOID)Try_NewPlatform_CreateProxyEndpoint, (PVOID*)&Functions.CreateProxyEndpoint);
	HookCode( (PVOID)Functions.OriShutdown, (PVOID)Try_NewPlatform_Shutdown, (PVOID*)&Functions.Shutdown);

	gMapPlatform.insert(std::make_pair(pUccPlatform,Functions));
}

void HookEndpoint(IUccEndpoint* pUccEndpoint)
{
	if(!pUccEndpoint) 
		return;

	MUTEX _mutex_(&gMutexMapEndpoint);

	char Error[256]={0};
	if(gMapEndpoint.find(pUccEndpoint) != gMapEndpoint.end())
	{
		sprintf_s(Error,sizeof(Error),"pUccEndpoint %p has been hooked\n",
			pUccEndpoint);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pUccEndpoint);

	OLD_ENDPOINT_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_ENDPOINT_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[3];
	memcpy(&(Functions.Enable),&Pointer,sizeof(ULONG));
	Functions.OriEnable = Functions.Enable;

	Pointer = Vtable[4];
	memcpy(&(Functions.Disable),&Pointer,sizeof(ULONG));
	Functions.OriDisable = Functions.Disable;

	Pointer = Vtable[0];
	memcpy(&(Functions.QueryInterface),&Pointer,sizeof(ULONG));
	Functions.OriQueryInterface = Functions.QueryInterface;

	for(MAP_ENDPOINT::const_iterator it = gMapEndpoint.begin();
		it != gMapEndpoint.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			// 这里用||，严格的做法应该是记录哪些相同，不同的还是应该HOOK，相同的直接赋值即可
			if( it->second.OriDisable == Functions.OriDisable ||
				it->second.OriEnable == Functions.OriEnable ||
				it->second.OriQueryInterface == Functions.OriQueryInterface
				)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccEndpoint %p has same vtable with %p, so we do not need to hook\n",
				pUccEndpoint,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			// 因为在My_NewSession_Xxx中需要根据pSession寻找Old_Xxx，所以需要把这一项加上，但是不Hook
			OLD_ENDPOINT_FUNCS Function = it->second;
			gMapEndpoint.insert(std::make_pair(pUccEndpoint,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook Endpoint %p\n", pUccEndpoint);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriQueryInterface,(PVOID)Try_NewPlatform_QueryInterface ,(PVOID*)&Functions.QueryInterface ) ;

	gMapEndpoint.insert(std::make_pair(pUccEndpoint,Functions));
}

void HookSessionManager(IUccSessionManager* pUccSessionManager)
{
	if(!pUccSessionManager) return;

	MUTEX _mutex_(&gMutexMapSessionMan);

	char Error[256]={0};
	if(gMapSessionMan.find(pUccSessionManager) != gMapSessionMan.end())
	{
		sprintf_s(Error,sizeof(Error),"SesionManager %p has been hooked\n",
			pUccSessionManager);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pUccSessionManager);	

	OLD_SESSIONMAN_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SESSIONMAN_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[3];
	memcpy(&(Functions.CreateSession),&Pointer,sizeof(ULONG));
	Functions.OriCreateSession = Functions.CreateSession;

	Pointer = Vtable[4];
	memcpy(&(Functions.RegisterSessionDescriptionEvaluator),&Pointer,sizeof(ULONG));
	Functions.OriRegisterSessionDescriptionEvaluator = Functions.RegisterSessionDescriptionEvaluator;

	Pointer = Vtable[5];
	memcpy(&(Functions.UnregisterSessionDescriptionEvaluator),&Pointer,sizeof(ULONG));
	Functions.OriUnregisterSessionDescriptionEvaluator = Functions.UnregisterSessionDescriptionEvaluator;

	Pointer = Vtable[0];
	memcpy(&(Functions.QueryInterface),&Pointer,sizeof(ULONG));
	Functions.OriQueryInterface = Functions.QueryInterface;

	for(MAP_SESSIONMAN::const_iterator it = gMapSessionMan.begin();
		it != gMapSessionMan.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			// 这里用||，严格的做法应该是记录哪些相同，不同的还是应该HOOK，相同的直接赋值即可
			if( it->second.OriCreateSession == Functions.OriCreateSession ||
				it->second.OriQueryInterface == Functions.OriQueryInterface ||
				it->second.OriRegisterSessionDescriptionEvaluator == Functions.OriRegisterSessionDescriptionEvaluator ||
				it->second.OriUnregisterSessionDescriptionEvaluator  == Functions.OriUnregisterSessionDescriptionEvaluator
				)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccSessionManager %p has same vtable with %p, so we do not need to hook\n",
				pUccSessionManager,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			// 因为在My_NewSession_Xxx中需要根据pSession寻找Old_Xxx，所以需要把这一项加上，但是不Hook
			OLD_SESSIONMAN_FUNCS Function = it->second;
			gMapSessionMan.insert(std::make_pair(pUccSessionManager,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook SessionManager %p\n", pUccSessionManager);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriCreateSession,(PVOID)Try_NewSessionMan_CreateSession ,(PVOID*)&Functions.CreateSession ) ;
	HookCode( (PVOID)Functions.OriRegisterSessionDescriptionEvaluator,(PVOID)Try_NewSessionMan_RegisterSessionDescriptionEvaluator ,(PVOID*)&Functions.RegisterSessionDescriptionEvaluator ) ;
	HookCode( (PVOID)Functions.OriUnregisterSessionDescriptionEvaluator,(PVOID)Try_NewSessionMan_UnregisterSessionDescriptionEvaluator ,(PVOID*)&Functions.UnregisterSessionDescriptionEvaluator ) ;
	HookCode( (PVOID)Functions.OriQueryInterface,(PVOID)Try_NewSessionMan_QueryInterface ,(PVOID*)&Functions.QueryInterface ) ;

	gMapSessionMan.insert(std::make_pair(pUccSessionManager,Functions));
}

void HookSession(IUccSession* pUccSession)
{
	if(!pUccSession) return;

	MUTEX _mutex_(&gMutexMapSession);

	char Error[256]={0};
	if(gMapSession.find(pUccSession) != gMapSession.end())
	{
		sprintf_s(Error,sizeof(Error),"Sesion %p has been hooked\n",
			pUccSession);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pUccSession);	

	g_log.Log(CELOG_DEBUG, L"Virtual table of IUccSession, %p\n", Vtable);

	OLD_SESSION_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SESSION_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[0];
	memcpy(&(Functions.QueryInterface),&Pointer,sizeof(ULONG));
	Functions.OriQueryInterface = Functions.QueryInterface;

	Pointer = Vtable[7];
	memcpy(&(Functions.CreateParticipant),&Pointer,sizeof(ULONG));
	Functions.OriCreateParticipant = Functions.CreateParticipant;

	Pointer = Vtable[8];
	memcpy(&(Functions.AddParticipant),&Pointer,sizeof(ULONG));
	Functions.OriAddParticipant = Functions.AddParticipant;

	Pointer = Vtable[9];
	memcpy(&(Functions.RemoveParticipant),&Pointer,sizeof(ULONG));
	Functions.OriRemoveParticipant = Functions.RemoveParticipant;

	Pointer = Vtable[10];
	memcpy(&(Functions.Terminate),&Pointer,sizeof(ULONG));
	Functions.OriTerminate = Functions.Terminate;

	bool Duplicate = false;
	MAP_SESSION::iterator it;
	for(it = gMapSession.begin(); it != gMapSession.end(); ++it)
	{
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
			break;
		}
		else
		{
			
			if( it->second.OriAddParticipant == Functions.OriAddParticipant ||
				it->second.OriCreateParticipant == Functions.OriCreateParticipant ||
				it->second.OriQueryInterface == Functions.OriQueryInterface ||
				it->second.OriRemoveParticipant == Functions.OriRemoveParticipant ||
				it->second.OriTerminate == Functions.OriTerminate
				)
			{
				Duplicate = true;
				break;
		}
		}
	}

		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccSession %p has same vtable with %p, so we do not need to hook\n",
				pUccSession,static_cast<IUccSession*>(it->first));
			g_log.Log(CELOG_DEBUG, Error);
			

		OLD_SESSION_FUNCS myFunction = it->second;

		gMapSession.insert(std::make_pair(pUccSession,myFunction));
			return;
		}

		/*
		
		this is a workaround for a crash issue.
		
		the issue is:
		when OC starts, it load OCE, then if before any other OC operation we start an Audio/Video conversation with 
		any peer, OC will crash.
		
		the reason I observe is: the function pointer of IUccSession::QueryInterface we get is incorrect.
		
		however, if we start a Instance Message conversation before Audio/Video conversation, 
		the function pointer of IUccSession::QueryInterface we get is correct.
		
		I don't know why the function pointer is incorrect if start A/V call before Instanct Message conversation.
		that means I don't know how to really solve this issue, but I just workaround by add an offset to the
		function pointer, by which I believe I get then correct function pointer of IUccSession::QueryInterface.
		
		the offset I use is got by refer to function pointer of IUccSession::Terminate, which is 
		always correct from my observation.
		
		*/
		FUNC_IUNKNOWN_QUERYINTERFACE tmpOriQueryInterface = Functions.OriQueryInterface;
		Functions.OriQueryInterface = reinterpret_cast<FUNC_IUNKNOWN_QUERYINTERFACE>(reinterpret_cast<size_t>(Functions.OriTerminate) - (0x66AEABF5 - 0x66AE76D2));
		g_log.Log(CELOG_DEBUG, "set ori query interface from [%p] to [%p] before hook\n", tmpOriQueryInterface, Functions.OriQueryInterface);

	HookCode( (PVOID)Functions.OriQueryInterface,(PVOID)Try_NewSession_QueryInterface,(PVOID*)&Functions.QueryInterface ) ;
	HookCode( (PVOID)Functions.OriCreateParticipant,(PVOID)Try_NewSession_CreateParticipant,(PVOID*)&Functions.CreateParticipant ) ;
	HookCode( (PVOID)Functions.OriAddParticipant,(PVOID)Try_NewSession_AddParticipant,(PVOID*)&Functions.AddParticipant ) ;
	HookCode( (PVOID)Functions.OriRemoveParticipant,(PVOID)Try_NewSession_RemoveParticipant,(PVOID*)&Functions.RemoveParticipant ) ;
	HookCode( (PVOID)Functions.OriTerminate,(PVOID)Try_NewSession_Terminate,(PVOID*)&Functions.Terminate ) ;

	gMapSession.insert(std::make_pair(pUccSession,Functions));

	sprintf_s(Error,sizeof(Error),"Hook Session %p, next QueryInterface %p, ori QueryInterface %p\n", pUccSession, Functions.QueryInterface, Functions.OriQueryInterface);
	g_log.Log(CELOG_DEBUG, Error);

	sprintf_s(Error,sizeof(Error),"Hook Session %p, next Terminate %p, ori Terminate %p\n", pUccSession, Functions.Terminate, Functions.OriTerminate);
	g_log.Log(CELOG_DEBUG, Error);
}

void HookSubscriptionManager(IUccSubscriptionManager *pSubscriptionManager)
{
	if(!pSubscriptionManager) return;

	MUTEX _mutex_(&gMutexMapSubscriptionMgr);

	char Error[256]={0};
	if(gMapSubscriptionMgr.find(pSubscriptionManager) != gMapSubscriptionMgr.end())
	{
		sprintf_s(Error,sizeof(Error),"SubscriptionMgr %p has been hooked\n",
			pSubscriptionManager);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pSubscriptionManager);	

	OLD_SUBSCRIPTION_MGR_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SUBSCRIPTION_MGR_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[3];
	memcpy(&(Functions.CreateSubscription),&Pointer,sizeof(ULONG));
	Functions.OriCreateSubscription  = Functions.CreateSubscription ;

	for(MAP_SUBSCRIPTION_MGR::const_iterator it = gMapSubscriptionMgr.begin();
		it != gMapSubscriptionMgr.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			if( it->second.OriCreateSubscription == Functions.OriCreateSubscription)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccSubscriptionManager %p has same vtable with %p, so we do not need to hook\n",
				pSubscriptionManager,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			OLD_SUBSCRIPTION_MGR_FUNCS Function = it->second;
			gMapSubscriptionMgr.insert(std::make_pair(pSubscriptionManager,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook SubscriptionManager %p\n", pSubscriptionManager);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriCreateSubscription,(PVOID)Try_NewSubscriptionMgr_CreateSubscription ,(PVOID*)&Functions.CreateSubscription ) ;

	gMapSubscriptionMgr.insert(std::make_pair(pSubscriptionManager,Functions));
}

HRESULT __stdcall My_NewSubscription_CreatePresentity(
	IUccSubscription *This,
	struct IUccUri * pPresentityUri,
	struct IUccContext * pContext,
	struct IUccPresentity * * ppPresentity )
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SUBSCRIPTION_CREATEPRESENTITY OldCreatePresentity = NULL;

		//g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_CreatePresentity\n"));
		{
			(&gMutexMapSubscription)->lock();

			MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
			if(it != gMapSubscription.end())
			{
				OldCreatePresentity = it->second.CreatePresentity;
			}

			(&gMutexMapSubscription)->unlock();
		}

		if(!OldCreatePresentity)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! CreatePresentity %p hasn't old CreatePresentity\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		HRESULT hr = OldCreatePresentity(
			This,
			pPresentityUri,
			pContext,
			ppPresentity);

		return hr;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SUBSCRIPTION_CREATEPRESENTITY OldCreatePresentity = NULL;

	//g_log.Log(CELOG_DEBUG, _T("In My_NewSubscription_CreatePresentity\n"));
	{
		MUTEX _mutex_(&gMutexMapSubscription);

		MAP_SUBSCRIPTION::iterator it = gMapSubscription.find(This);
		if(it != gMapSubscription.end())
		{
			OldCreatePresentity = it->second.CreatePresentity;
		}
	}

	if(!OldCreatePresentity)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! CreatePresentity %p hasn't old CreatePresentity\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCreatePresentity(
		This,
		pPresentityUri,
		pContext,
		ppPresentity);

	return hr;
}

HRESULT __stdcall My_NewSubscriptionMgr_CreateSubscription(
	IUccSubscriptionManager *This,
struct IUccContext * pContext,
struct IUccSubscription * * ppSubscription)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SUBSCRIPTIONMGR_CREATESUBSCRIPTION OldCreateSubscription = NULL;
		//g_log.Log(CELOG_DEBUG, _T("In My_NewSubscriptionMgr_CreateSubscription\n"));

		{
			(&gMutexMapSubscriptionMgr)->lock();

			MAP_SUBSCRIPTION_MGR::iterator it = gMapSubscriptionMgr.find(This);
			if(it != gMapSubscriptionMgr.end())
			{
				OldCreateSubscription = it->second.CreateSubscription;
			}

			(&gMutexMapSubscriptionMgr)->unlock();
		}

		if(!OldCreateSubscription)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! CreateSubscription %p hasn't old CreatePresentity\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateSubscription(
			This,
			pContext,
			ppSubscription);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SUBSCRIPTIONMGR_CREATESUBSCRIPTION OldCreateSubscription = NULL;
	//g_log.Log(CELOG_DEBUG, _T("In My_NewSubscriptionMgr_CreateSubscription\n"));

	{
		MUTEX _mutex_(&gMutexMapSubscriptionMgr);

		MAP_SUBSCRIPTION_MGR::iterator it = gMapSubscriptionMgr.find(This);
		if(it != gMapSubscriptionMgr.end())
		{
			OldCreateSubscription = it->second.CreateSubscription;
		}
	}

	if(!OldCreateSubscription)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! CreateSubscription %p hasn't old CreatePresentity\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCreateSubscription(
		This,
		pContext,
		ppSubscription);

	if(hr == S_OK && ppSubscription) {
		//IUccSubscription *pSubscription = *ppSubscription;
		//HookSubscription(pSubscription);
	}
	return hr;
}


HRESULT __stdcall Try_NewSubscription_CreatePresentity(
	IUccSubscription *This,
	struct IUccUri * pPresentityUri,
	struct IUccContext * pContext,
	struct IUccPresentity * * ppPresentity )
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSubscription_CreatePresentity beg\n");

	__try
	{
		res = My_NewSubscription_CreatePresentity(
						This,
						pPresentityUri,
						pContext,
						ppPresentity);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSubscription_CreatePresentity end\n");
	return res;
}

HRESULT __stdcall Try_NewSubscriptionMgr_CreateSubscription (
	IUccSubscriptionManager *This,
struct IUccContext * pContext,
struct IUccSubscription * * ppSubscription 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSubscriptionMgr_CreateSubscription beg\n");
	
	__try
	{
		res = My_NewSubscriptionMgr_CreateSubscription (	
						This,
						pContext,
						ppSubscription );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSubscriptionMgr_CreateSubscription end\n");
	return res;
}

HRESULT __stdcall Try_NewSessionParticipant_CreateParticipantEndpoint(
	IUccSessionParticipant *This,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
struct IUccContext * pContext,
struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_CreateParticipantEndpoint beg\n");

	__try
	{
		res = My_NewSessionParticipant_CreateParticipantEndpoint(	
						This,
						pUri,
						bstrEndpointId,
						pContext,
						ppParticipantEndpoint );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_CreateParticipantEndpoint end\n");
	return res;
}

HRESULT __stdcall My_NewSessionParticipant_CreateParticipantEndpoint(
	IUccSessionParticipant *This,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
struct IUccContext * pContext,
struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint)
{

	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SESSIONPARTICIPANT_CREATEPARTICIPANTENDPOINT OldCreateParticipantEndpoint = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_CreateParticipantEndpoint\n"));

		{
			(&gMutexMapSessionParticipant)->lock();

			MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
			if(it != gMapSessionParticipant.end())
			{
				OldCreateParticipantEndpoint = it->second.CreateParticipantEndpoint;
			}
			(&gMutexMapSessionParticipant)->unlock();
		}

		if(!OldCreateParticipantEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! CreateParticipantEndpoint %p hasn't old OldCreateParticipantEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCreateParticipantEndpoint(
			This,
			pUri,
			bstrEndpointId,
			pContext,
			ppParticipantEndpoint);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SESSIONPARTICIPANT_CREATEPARTICIPANTENDPOINT OldCreateParticipantEndpoint = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_CreateParticipantEndpoint\n"));

	{
		MUTEX _mutex_(&gMutexMapSessionParticipant);

		MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
		if(it != gMapSessionParticipant.end())
		{
			OldCreateParticipantEndpoint = it->second.CreateParticipantEndpoint;
		}
	}

	if(!OldCreateParticipantEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! CreateParticipantEndpoint %p hasn't old OldCreateParticipantEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCreateParticipantEndpoint(
		This,
		pUri,
		bstrEndpointId,
		pContext,
		ppParticipantEndpoint);

	if(hr == S_OK && ppParticipantEndpoint) {

	}
	return hr;

}


HRESULT __stdcall Try_NewSessionParticipant_CopyParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pInputParticipantEndpoint,
struct IUccContext * pContext,
struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_CopyParticipantEndpoint beg\n");
	
	__try
	{
		res = My_NewSessionParticipant_CopyParticipantEndpoint(
							This,
							pInputParticipantEndpoint,
							pContext,
							ppParticipantEndpoint );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_CopyParticipantEndpoint end\n");
	return res;
}


HRESULT __stdcall My_NewSessionParticipant_CopyParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pInputParticipantEndpoint,
struct IUccContext * pContext,
struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
	)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SESSIONPARTICIPANT_COPYPARTICIPANTENDPOINT OldCopyParticipantEndpoint = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_CopyParticipantEndpoint\n"));

		{
			(&gMutexMapSessionParticipant)->lock();

			MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
			if(it != gMapSessionParticipant.end())
			{
				OldCopyParticipantEndpoint = it->second.CopyParticipantEndpoint;
			}

			(&gMutexMapSessionParticipant)->unlock();
		}

		if(!OldCopyParticipantEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! CopyParticipantEndpoint %p hasn't old OldCopyParticipantEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldCopyParticipantEndpoint(
			This,
			pInputParticipantEndpoint,
			pContext,
			ppParticipantEndpoint); 
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SESSIONPARTICIPANT_COPYPARTICIPANTENDPOINT OldCopyParticipantEndpoint = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_CopyParticipantEndpoint\n"));

	{
		MUTEX _mutex_(&gMutexMapSessionParticipant);

		MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
		if(it != gMapSessionParticipant.end())
		{
			OldCopyParticipantEndpoint = it->second.CopyParticipantEndpoint;
		}
	}

	if(!OldCopyParticipantEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! CopyParticipantEndpoint %p hasn't old OldCopyParticipantEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldCopyParticipantEndpoint(
		This,
		pInputParticipantEndpoint,
		pContext,
		ppParticipantEndpoint); 

	if(hr == S_OK && ppParticipantEndpoint) {
		HWND winHandle=GetForegroundWindow();
		if(ocePolicyEval.IsInGroupChatSession(winHandle))
			ocePolicyEval.AddExistingMemberToGroupChatSession(winHandle, This);
		/*------------For debugging purpose
		IUccSession *pSession;
		UCC_SESSION_TYPE sessionType;
		This->get_Session(&pSession);
		pSession->get_Type(&sessionType);
		BSTR bstr;
		This->get_Uri(&bstr);
		if(sessionType == UCCST_CONFERENCE) 
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) new session(%p) is in type %s\n"),
		bstr, pSession, _T("Conference"));
		else if(sessionType == UCCST_APPLICATION) {
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) new session(%p) is in type %s\n"),
		bstr, pSession, _T("Application"));
		IUccCollection *pParticipants; 
		long numParticipants;

		pSession->get_Participants(&pParticipants );
		pParticipants->get_Count(&(numParticipants));
		IUccSessionParticipant *pParticipant; 
		for(int i=1; i<=numParticipants; i++) {
		VARIANT vtItem;
		vtItem.vt = VT_DISPATCH;
		pParticipants->get_Item(i, &vtItem);
		pParticipant = (IUccSessionParticipant*) vtItem.pdispVal;
		BSTR bstrUri = NULL;
		HRESULT hr = pParticipant->get_Uri(&bstrUri);
		if(SUCCEEDED(hr)&&bstrUri) {
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) new session(%p)\n"),
		bstrUri, pSession);					
		}
		SysFreeString(bstrUri);
		}
		}

		IUccSessionParticipant *pParticipant;
		pInputParticipantEndpoint->get_Participant(&pParticipant);
		pParticipant->get_Session(&pSession);
		pSession->get_Type(&sessionType);
		pParticipant->get_Uri(&bstr);
		if(sessionType == UCCST_CONFERENCE)
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) old session(%p) is in type %s\n"),
		bstr,pSession, _T("Conference"));
		else if(sessionType == UCCST_APPLICATION)
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) old session(%p) is in type %s\n"),
		bstr,pSession, _T("Application"));
		else if(sessionType == UCCST_INSTANT_MESSAGING)
		g_log.Log(CELOG_DEBUG, _T("In CopyParticipantEndpoint: participant(%s) old session(%p) is in type %s\n"),
		bstr,pSession, _T("IM"));
		------------end For debugging purpose--*/

	}
	return hr;
}

HRESULT __stdcall Try_NewSessionParticipant_AddParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_AddParticipantEndpoint beg\n");
	
	__try
	{
		res = My_NewSessionParticipant_AddParticipantEndpoint(	
								This,
								pParticipantEndpoint,
								pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_AddParticipantEndpoint end\n");
	return res;
}

HRESULT __stdcall My_NewSessionParticipant_AddParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
struct IUccOperationContext * pOperationContext 
	)
{

	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SESSIONPARTICIPANT_ADDPARTICIPANTENDPOINT OldAddParticipantEndpoint = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_AddParticipantEndpoint\n"));

		{
			(&gMutexMapSessionParticipant)->lock();

			MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
			if(it != gMapSessionParticipant.end())
			{
				OldAddParticipantEndpoint = it->second.AddParticipantEndpoint;
			}

			(&gMutexMapSessionParticipant)->unlock();
		}

		if(!OldAddParticipantEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! AddParticipantEndpoint %p hasn't old OldAddParticipantEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldAddParticipantEndpoint(
			This,
			pParticipantEndpoint,
			pOperationContext);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SESSIONPARTICIPANT_ADDPARTICIPANTENDPOINT OldAddParticipantEndpoint = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_AddParticipantEndpoint\n"));

	{
		MUTEX _mutex_(&gMutexMapSessionParticipant);

		MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
		if(it != gMapSessionParticipant.end())
		{
			OldAddParticipantEndpoint = it->second.AddParticipantEndpoint;
		}
	}

	if(!OldAddParticipantEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! AddParticipantEndpoint %p hasn't old OldAddParticipantEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldAddParticipantEndpoint(
		This,
		pParticipantEndpoint,
		pOperationContext);

	if(hr == S_OK ) {

	}
	return hr;

}

HRESULT __stdcall Try_NewSessionParticipant_RemoveParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
struct IUccOperationContext * pOperationContext 
	)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_RemoveParticipantEndpoint beg\n");

	__try
	{
		res = My_NewSessionParticipant_RemoveParticipantEndpoint(	
						This,
						pParticipantEndpoint,
						pOperationContext );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionParticipant_RemoveParticipantEndpoint end\n");
	return res;
}


HRESULT __stdcall My_NewSessionParticipant_RemoveParticipantEndpoint(
	IUccSessionParticipant *This,
struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
struct IUccOperationContext * pOperationContext 
	)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SESSIONPARTICIPANT_REMOVEPARTICIPANTENDPOINT OldRemoveParticipantEndpoint = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_RemoveParticipantEndpoint\n"));

		{
			(&gMutexMapSessionParticipant)->lock();

			MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
			if(it != gMapSessionParticipant.end())
			{
				OldRemoveParticipantEndpoint = it->second.RemoveParticipantEndpoint;
			}

			(&gMutexMapSessionParticipant)->unlock();
		}

		if(!OldRemoveParticipantEndpoint)
		{
			sprintf_s(Error,sizeof(Error),"Oops!!! RemoveParticipantEndpoint %p hasn't old OldRemoveParticipantEndpoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		HRESULT hr = OldRemoveParticipantEndpoint(
			This,
			pParticipantEndpoint,
			pOperationContext);

		return hr;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SESSIONPARTICIPANT_REMOVEPARTICIPANTENDPOINT OldRemoveParticipantEndpoint = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionParticipant_RemoveParticipantEndpoint\n"));

	{
		MUTEX _mutex_(&gMutexMapSessionParticipant);

		MAP_SESSION_PARTICIPANT::iterator it = gMapSessionParticipant.find(This);
		if(it != gMapSessionParticipant.end())
		{
			OldRemoveParticipantEndpoint = it->second.RemoveParticipantEndpoint;
		}
	}

	if(!OldRemoveParticipantEndpoint)
	{
		sprintf_s(Error,sizeof(Error),"Oops!!! RemoveParticipantEndpoint %p hasn't old OldRemoveParticipantEndpoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldRemoveParticipantEndpoint(
		This,
		pParticipantEndpoint,
		pOperationContext);

	if(hr == S_OK ) {

	}
	return hr;
}




HRESULT	__stdcall My_NewConnPointContainer_FindConnectionPoint(
	IConnectionPointContainer *This,
	REFIID riid,
	IConnectionPoint ** ppCP)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT OldFindConnectionPoint = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewConnPointContainer_FindConnectionPoint\n"));

		{
			(&gMutexMapMapConnPointContainer)->lock();

			MAP_CONN_POINT_CONTAINER::iterator it = gMapConnPointContainer.find(This);
			if(it != gMapConnPointContainer.end())
			{
				OldFindConnectionPoint = it->second.FindConnectionPoint;
			}

			(&gMutexMapMapConnPointContainer)->unlock();
		}

		if(!OldFindConnectionPoint) {
			sprintf_s(Error,sizeof(Error),"Oops!!! FindConnectionPoint %p hasn't old OldFindConnectionPoint\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		return OldFindConnectionPoint(
			This,
			riid,
			ppCP);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT OldFindConnectionPoint = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewConnPointContainer_FindConnectionPoint\n"));

	{
		MUTEX _mutex_(&gMutexMapMapConnPointContainer);

		MAP_CONN_POINT_CONTAINER::iterator it = gMapConnPointContainer.find(This);
		if(it != gMapConnPointContainer.end())
		{
			OldFindConnectionPoint = it->second.FindConnectionPoint;
		}
	}

	if(!OldFindConnectionPoint) {
		sprintf_s(Error,sizeof(Error),"Oops!!! FindConnectionPoint %p hasn't old OldFindConnectionPoint\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		HookConnectionPointContainer(This);
		{
			MUTEX _mutex_(&gMutexMapMapConnPointContainer);

			MAP_CONN_POINT_CONTAINER::iterator it = gMapConnPointContainer.find(This);
			if(it != gMapConnPointContainer.end())
			{
				OldFindConnectionPoint = it->second.FindConnectionPoint;
			}
		}
	}

	HRESULT hr = OldFindConnectionPoint(
		This,
		riid,
		ppCP);

	/*wchar_t ClassId[256] = {0};
	StringFromGUID2(riid,ClassId,sizeof(ClassId)/sizeof(wchar_t));
	if(_wcsnicmp(ClassId, L"{90bbe2b8-fb81-4e70-ae47-92b75431d4c1}", 
	wcslen(L"{90bbe2b8-fb81-4e70-ae47-92b75431d4c1}")) == 0) {
	g_log.Log(CELOG_DEBUG, _T("FindConnectionPoint for _IUccSessionManagerEvents\n"));
	}*/

	if(hr==S_OK && ppCP!=NULL && *ppCP!=NULL) {
		IConnectionPoint *pCP=*ppCP;
		HookConnectionPoint(pCP);
	}
	return hr;
}

HRESULT	__stdcall Try_NewConnPointContainer_FindConnectionPoint(
	IConnectionPointContainer *This,
	REFIID riid,
	IConnectionPoint ** ppCP)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConnPointContainer_FindConnectionPoint beg\n");

	__try
	{
		res = My_NewConnPointContainer_FindConnectionPoint(		This,
			riid,
			ppCP ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConnPointContainer_FindConnectionPoint end\n");
	return res;
}




HRESULT	__stdcall My_NewConnPointContainer_EnumConnectionPoints(
	IConnectionPointContainer *This,
	IEnumConnectionPoints ** ppEnum)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS OldEnumConnectionPoints = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewConnPointContainer_EnumConnectionPoints\n"));

		{
			(&gMutexMapMapConnPointContainer)->lock();

			MAP_CONN_POINT_CONTAINER::iterator it = gMapConnPointContainer.find(This);
			if(it != gMapConnPointContainer.end())
			{
				OldEnumConnectionPoints = it->second.EnumConnectionPoints;
			}

			(&gMutexMapMapConnPointContainer)->unlock();
		}

		if(!OldEnumConnectionPoints) {
			sprintf_s(Error,sizeof(Error),"Oops!!! EnumConnectionPoints %p hasn't old EnumConnectionPoints\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}

		HRESULT hr = OldEnumConnectionPoints(
			This,
			ppEnum);

		g_log.Log(CELOG_DEBUG, _T("EnumConnectionPoints return %d\n"), hr);

		return hr;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS OldEnumConnectionPoints = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewConnPointContainer_EnumConnectionPoints\n"));

	{
		MUTEX _mutex_(&gMutexMapMapConnPointContainer);

		MAP_CONN_POINT_CONTAINER::iterator it = gMapConnPointContainer.find(This);
		if(it != gMapConnPointContainer.end())
		{
			OldEnumConnectionPoints = it->second.EnumConnectionPoints;
		}
	}

	if(!OldEnumConnectionPoints) {
		sprintf_s(Error,sizeof(Error),"Oops!!! EnumConnectionPoints %p hasn't old EnumConnectionPoints\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	HRESULT hr = OldEnumConnectionPoints(
		This,
		ppEnum);

	g_log.Log(CELOG_DEBUG, _T("EnumConnectionPoints return %d\n"), hr);

	return hr;
}


HRESULT	__stdcall Try_NewConnPointContainer_EnumConnectionPoints(
	IConnectionPointContainer *This,
	IEnumConnectionPoints ** ppEnum)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConnPointContainer_EnumConnectionPoints beg\n");
	
	__try
	{
		res = My_NewConnPointContainer_EnumConnectionPoints(		This,
			ppEnum ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConnPointContainer_EnumConnectionPoints end\n");
	return res;
}

HRESULT	__stdcall My_NewConnPoint_Advise(
									  IConnectionPoint *This,
									  IUnknown * pUnk,
									  DWORD * pdwCookie)
{

	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_CONNPOINT_ADVISE OldAdvise = NULL;
		//g_log.Log(CELOG_DEBUG, _T("In NewConnPoint_Advise\n"));

		{
			(&gMutexMapMapConnPoint)->lock();

			MAP_CONN_POINT::iterator it = gMapConnPoint.find(This);
			if(it != gMapConnPoint.end())
			{
				OldAdvise = it->second.Advise;
			}

			(&gMutexMapMapConnPoint)->unlock();
		}

		if(!OldAdvise) {
			sprintf_s(Error,sizeof(Error),"Oops!!! Advise %p hasn't old OldAdvise\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}
		return OldAdvise(		This,
			pUnk,
			pdwCookie ) ;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_CONNPOINT_ADVISE OldAdvise = NULL;
	//g_log.Log(CELOG_DEBUG, _T("In NewConnPoint_Advise\n"));

	{
		MUTEX _mutex_(&gMutexMapMapConnPoint);

		MAP_CONN_POINT::iterator it = gMapConnPoint.find(This);
		if(it != gMapConnPoint.end())
		{
			OldAdvise = it->second.Advise;
		}
	}

	if(!OldAdvise) {
		sprintf_s(Error,sizeof(Error),"Oops!!! Advise %p hasn't old OldAdvise\n",This);
//		g_log.Log(CELOG_DEBUG, Error);
		HookConnectionPoint(This);
		{
			MUTEX _mutex_(&gMutexMapMapConnPoint);

			MAP_CONN_POINT::iterator it = gMapConnPoint.find(This);
			if(it != gMapConnPoint.end())
			{
				OldAdvise = it->second.Advise;
			}
		}
	}

	//Do policy evaluation
	CComPtr<IConnectionPointContainer> pCPC;
	HRESULT hr=This->GetConnectionPointContainer(&pCPC);
	if(hr==S_OK) {
		IID   iid;
		if(This->GetConnectionInterface(&iid) == S_OK) {
			wchar_t ClassId[256] = {0};
			StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(wchar_t));
			g_log.Log(CELOG_DEBUG, L"My_NewConnPoint_Advise, class id: %s\n", ClassId);
			if(_wcsnicmp(ClassId, L"{fdcb6c19-66cb-492a-9f4e-996d92a50407}",//_IUccInstantMessagingSessionEvents
				wcslen(L"{fdcb6c19-66cb-492a-9f4e-996d92a50407}")) == 0 ) { 
					//Advise _IUccInstantMessagingSessionEvents
					//Get IUccSession for policy evaluation
					CComPtr<IUccInstantMessagingSession> pIMSession;
					hr=pCPC->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession)); 
					if(hr==S_OK) 
						HookIMSession(pIMSession);
					CComPtr<IUccSession> pSession;
					hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
					g_log.Log(CELOG_DEBUG, _T("Advise IUccIMSessionEvents for session(%p), vtable: %p \n"), pSession, (*(PVOID**)(IUccSession*)pSession));	
					if(hr==S_OK) {
						HookSession(pSession);

						g_log.Log(CELOG_DEBUG, L"Receive message... peer to peer\n");
						bool bAllow=DoEvalOnIncomingSession(pSession);
						if(!bAllow)
							return E_FAIL;
					}
			} else if(_wcsnicmp(ClassId, L"{ab81ffb3-80a5-4fe9-ac37-b06d5d631be1}", //IUccSessionEvents
				wcslen(L"{ab81ffb3-80a5-4fe9-ac37-b06d5d631be1}")) == 0 ) { 
					//Advise IUccSessionEvents 
					//Get IUccSession for policy evaluation
					CComPtr<IUccSession> pSession = NULL;

					hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
					g_log.Log(CELOG_DEBUG, _T("Advise IUccSessionEvents for session(%p), vtable: %p \n"), pSession, (*(PVOID**)(IUccSession*)pSession));	

					if(hr==S_OK) {
						/*************************************************************
						The virtual table of pSession here is different with other
						places. we regard the first function as QueryInterface, but
						the address of the "QueryInterface" is different.
						Once we hooked this function, OCE will always crash.

						We will also hook IUccSession in My_NewSessionMan_CreateSession,
						there:
						vtable: 0x22db0eb4, QueryInterface: 0x22db0ae1
						this is correct, we can hook "QueryInterface".
						but here:
						vtable: 0x22db0f4c, QueryInterface: 0x22d7d334
						this is not correct. OCE will crash once we hooked 0x22d7d334
						the tricky is: The addresses of all other functions in these 2 
						virtual tables are same, only the address of first function is 
						different.


						Kevin

						**************************************************************/

						//		HookSession(pSession);

						g_log.Log(CELOG_DEBUG, L"Advise IUccSessionEvents (receive message)\n");
						bool bAllow=DoEvalOnIncomingSession(pSession);
						if(!bAllow)
							return E_FAIL;
					}
			} else if (_wcsnicmp(ClassId, L"{9b48f770-4eac-4967-bcac-283b73557860}",	//	_IUccSessionParticipantEvents
				wcslen(L"{9b48f770-4eac-4967-bcac-283b73557860}")) == 0 ||
				_wcsnicmp(ClassId, L"{c819c292-f45f-4854-ae66-c9e3cfe45195}",	//	_IUccConferenceSessionParticipantEvents
				wcslen(L"{c819c292-f45f-4854-ae66-c9e3cfe45195}")) == 0) 
			{
				CComQIPtr<IUccSessionParticipant> pParticipant;
				GUID guidValue;
				if(CLSIDFromString(L"{c520c114-7ebf-4a14-8416-d17adfa86227}", &guidValue) == NOERROR) //	IUccSessionParticipant
				{
					//	try to get interface of IUccSessionParticipant from the connection pointer container
					hr=pCPC->QueryInterface(guidValue, (void **)(&pParticipant));
					if(hr == S_OK) 
					{
						CComPtr<IUccSession> pSession;
						UCC_SESSION_TYPE sessionType;
						pParticipant->get_Session(&pSession);
						pSession->get_Type(&sessionType);
						HookSession(pSession);
						//Collection group IM information on incoming session
						if(sessionType == UCCST_CONFERENCE) 
						{
							//	comment in 2011-12-21
							//	check if the conf session is combined with a share session
							//	if yes, then should evaluate on share
							CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
							if (ins->GetShareFlagByConfSessionPointer(pSession))
							{
								bool b_allow = ocePolicyEval.DoShareEvalOnSession(pSession);
								if(!b_allow)
								{
									//If not allowed, the local participant needs to leave 
									//from this communication
									GUID GuidValue; 
									IUccConferenceSession *pConfSession = NULL;
									if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
										&GuidValue) == NOERROR ) 
									{
										pSession->QueryInterface(GuidValue, (void **)(&pConfSession));
									}
									if (pConfSession)
									{
										g_log.Log(CELOG_DEBUG, _T("leave conf Session(%p) when advise UCCST_CONFERENCE session(%p)\n"), pConfSession, pSession);
										pConfSession->Leave(NULL);
									}
								}
							}
							


							//	my observation:
							//	suppose the code is run on A.
							//	on these situation OC will advice both _IUccSessionParticipantEvents(happen first) and _IUccConferenceSessionParticipantEvents(happen later)
							//	1,
							//	A is IM with B, then A invite C, when C accept the conversation, then we are here.
							//	in this case, the participant belong to a conference type session. the session contains 3 participants, 
							//	it's same session for both _IUccSessionParticipantEvents(happen first) and _IUccConferenceSessionParticipantEvents(happen later).
							//	2,
							//	A is IM with B, then A invite C, at the time invite occurs(before C accept the conversation), then we are here twice(if we consider 
							//	_IUccSessionParticipantEvents and _IUccConferenceSessionParticipantEvents, we are here 4 times -- each event twice), 
							//	the participants number is 1 at the first time and that participant is local user, the number is 2 at the second time
							//	they belong to the same session.
							//	3,
							//	above case is true for Audio/Video call

							//	suppose the code is run on B.
							//	on these situation OC will advice _IUccConferenceSessionParticipantEvents and we are here
							//	A is Audio with B, then A invite C, at the time invite occurs(before C accept the conversation), we are here twice, and the participants 
							//	in the session are always 2. this means B and A has automatically joined the new conference session
							//	when C accept the invitation, we are here again, the participants in the session are 3, this means C joins the session

							//	suppose the code is run on C.
							//	on these situation OC will advice _IUccConferenceSessionParticipantEvents and we are here
							//	A is IM with B, then A invite C, only when C accept the conversation, then we are here.
							//	we are here only once, the participants in the session are 3
							bool bAllow=ocePolicyEval.DoGroupEvalOnIncomingSession(pSession);	
							if(!bAllow) 
							{
								//If not allowed, the local participant needs to leave 
								//from this communication
								GUID GuidValue; 
								IUccConferenceSession *pConfSession = NULL;
								if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
									&GuidValue) == NOERROR ) 
								{
										pSession->QueryInterface(GuidValue, (void **)(&pConfSession));
								}
								if (pConfSession)
								{
									pConfSession->Leave(NULL);
								}
							}
							else
							{
								//	comment by Ben, 2011-12-14
								//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
								//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
								//	in the existing code, there are some evaluations already,
								//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
								//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
								//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
								//	then we have two cases:
								//	case 1,
								//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
								//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
								//	case 2,
								//	on the other side, if an audio/video session is advised, 
								//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
								//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
								//	here we are in case 1,
								//	so, let's check CLiveSessionWnd to see if we have or not have audio/video session for the same hwnd
								CComPtr<IUccSession> pAVSession = ins->GetSessionForSession(pSession, UCCST_CONFERENCE, UCCST_AUDIO_VIDEO);
								if (pAVSession)
								{
									bool b_allow = ocePolicyEval.DoEvalOnSession(pSession, CE_ACTION_AVD);
									if(!b_allow)
									{
										//If not allowed, the local participant needs to leave 
										//from this communication
										GUID GuidValue; 
										IUccConferenceSession *pConfSession = NULL;
										if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
											&GuidValue) == NOERROR ) 
										{
											pSession->QueryInterface(GuidValue, (void **)(&pConfSession));
										}
										if (pConfSession)
										{
											g_log.Log(CELOG_DEBUG, _T("leave conf Session(%p) when advise UCCST_CONFERENCE session(%p)\n"), pConfSession, pSession);
											pConfSession->Leave(NULL);
										}
									}
								}
							}
						}		
						CComPtr<IUccCollection> pParticipants; 
						long numParticipants;
						pSession->get_Participants(&pParticipants );
						pParticipants->get_Count(&numParticipants);
						if(sessionType == UCCST_INSTANT_MESSAGING) 
						{
							CComPtr<IUccInstantMessagingSession> pIMSession;
							pSession->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession));
							g_log.Log(CELOG_DEBUG, _T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_INSTANT_MESSAGING\n"), pSession, numParticipants);
						}
						else if (sessionType == UCCST_AUDIO_VIDEO)
						{
							//	comment by Ben, 2011-12-14
							//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
							//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
							//	in the existing code, there are some evaluations already,
							//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
							//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
							//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
							//	then we have two cases:
							//	case 1,
							//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
							//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
							//	case 2,
							//	on the other side, if an audio/video session is advised, 
							//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
							//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
							//	here we are in case 1,
							//	so, let's check CLiveSessionWnd to see if we have or not have audio/video session for the same hwnd
							//	here we are in case 2,
							//	so, let's check CLiveSessionWnd to see if we have or not have conference session for the same hwnd
							CLiveSessionWnd* ins = CLiveSessionWnd::GetInstance();
							CComPtr<IUccSession> pMyConfSession = ins->GetSessionForSession(pSession, UCCST_AUDIO_VIDEO, UCCST_CONFERENCE);
							if(pMyConfSession)
							{
								bool bAllow = ocePolicyEval.DoEvalOnSession(pMyConfSession, CE_ACTION_AVD);
								if(!bAllow) 
								{
									//If not allowed, the local participant needs to leave 
									//from this communication
									GUID GuidValue; 
									CComPtr<IUccConferenceSession> pConfSession = NULL;
									if(CLSIDFromString(L"{f8c1eee0-4272-4dd6-8986-786358e6a0e3}", //	IUccConferenceSession
										&GuidValue) == NOERROR ) 
									{
										pMyConfSession->QueryInterface(GuidValue, (void **)(&pConfSession));
									}
									if (pConfSession)
									{
										g_log.Log(CELOG_DEBUG, _T("leave conf Session(%p) when advise UCCST_AUDIO_VIDEO session(%p)\n"), pConfSession, pSession);
										pConfSession->Leave(NULL);
									}
								}
							}
						}
						else if(sessionType == UCCST_CONFERENCE)
						{
							g_log.Log(CELOG_DEBUG, _T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_CONFERENCE\n"), pSession, numParticipants);
						}
						else if(sessionType == UCCST_APPLICATION) 
						{
							g_log.Log(CELOG_DEBUG, _T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_APPLICATION\n"), pSession, numParticipants);
							//When add more people to an existing outgoing group chat, the title won't change 
							//to the new number of participant until all adding participants done. So
							//we will do policy evaluation here and if allows, add participant here.
							HWND winHandle=GetForegroundWindow();
							int realNumParticipant;
							GetCurrentSessionParticipantNum(winHandle, realNumParticipant);
							ocePolicyEval.UpdateGroupParticipantNum(winHandle,realNumParticipant);
							if(ocePolicyEval.IsTimeToDoEvaluation(winHandle)) {
								g_log.Log(CELOG_DEBUG, _T("Time to evaluate group chat. \n"));	
								ocePolicyEval.DoGroupChatEvaluation(winHandle, L"dummyParticipant");
								//We don't need to take any action based on the evaluation result. 
								//If it allows, the participant has been added by "DoGroupChatEvaluation";
								//if not allows, we don't need to do any thing. 
							}
						}
						g_log.Log(CELOG_DEBUG, _T("Advise IUccSessionParticipantEvents for session(%p) \n"), pSession);	
						for(int i=1; i<=numParticipants; i++) {
							CComVariant vtItem;
							vtItem.vt = VT_DISPATCH;
							pParticipants->get_Item(i, &vtItem);
							pParticipant = vtItem.pdispVal;
							VARIANT_BOOL bLocal;
							if(pParticipant->get_IsLocal(&bLocal)==S_OK)
								if(bLocal)
									continue;
							BSTR bstrUri = NULL;IUccUri* pUri = NULL;
							HRESULT Hr = pParticipant->get_Uri(&pUri);pUri->get_AddressOfRecord(&bstrUri);
							if(SUCCEEDED(Hr) && bstrUri) {
								g_log.Log(CELOG_DEBUG, _T("Advise Session(%p) has non-local participant(%d): %s\n"), 
									pSession, numParticipants-1, bstrUri);							
								SysFreeString(bstrUri);
							}
						}
					}
				}
			}
			else
			{
				;
			}
		}
	}
	hr = OldAdvise(
		This,
		pUnk,
		pdwCookie);

	IID   iid;
	if(This->GetConnectionInterface(&iid) == S_OK) {
		wchar_t ClassId[256] = {0};
		StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(wchar_t));
		//	g_log.Log(CELOG_DEBUG, _T("Advise for %s\n"), ClassId);
	}

	/*	if(_wcsnicmp(ClassId, L"{90bbe2b8-fb81-4e70-ae47-92b75431d4c1}", 
	wcslen(L"{90bbe2b8-fb81-4e70-ae47-92b75431d4c1}")) == 0) {
	g_log.Log(CELOG_DEBUG, _T("Advise for _IUccSessionManagerEvents\n"));
	GUID guidValue; 
	if(CLSIDFromString(L"{90bbe2b8-fb81-4e70-ae47-92b75431d4c1}", 
	&guidValue) == NOERROR ) {
	_IUccSessionManagerEvents *pSessionManEvents;
	(*pUnk).QueryInterface(guidValue, (void **)(&pSessionManEvents));
	//HookSessionManagerEvents((void *)pSessionManEvents);
	HookSessionManagerEvents((void *)pUnk);
	pSessionManEvents->Release();
	}
	} */


	return hr;
}

HRESULT	__stdcall Try_NewConnPoint_Advise(
	IConnectionPoint *This,
	IUnknown * pUnk,
	DWORD * pdwCookie)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewConnPoint_Advise beg\n");

	__try
	{
		res = My_NewConnPoint_Advise(		This,
			pUnk,
			pdwCookie ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewConnPoint_Advise end\n");
	return res;
}


HRESULT	__stdcall My_NewSessionManEvents_OnInCommingSession(
	_IUccSessionManagerEvents *This,
	IUccEndpoint *pEndPoint, 
	IUccIncomingSessionEvent *pEventData)
{
	if (OCEIsDisabled())
	{
		char Error[1024]={0};
		FUNC_SESSION_MAN_EVENTS_ONINCOMMINGSESSION OldOnInCommingSession = NULL;
		g_log.Log(CELOG_DEBUG, _T("In My_NewSessionManEvents_OnInCommingSession\n"));

		{
			(&gMutexMapSessionManEvents)->lock();

			MAP_SESSION_MANAGER_EVENTS::iterator it = gMapSessionManEvents.find(This);
			if(it != gMapSessionManEvents.end())
			{
				OldOnInCommingSession = it->second.OnInCommingSession;
			}

			(&gMutexMapSessionManEvents)->unlock();
		}

		if(!OldOnInCommingSession) {
			sprintf_s(Error,sizeof(Error),"Oops!!! OnInCommingSessionAdvise %p hasn't old OnInCommingSession\n",This);
			g_log.Log(CELOG_DEBUG, Error);
			return E_FAIL;
		}
		return OldOnInCommingSession(		This,
			pEndPoint,
			pEventData ) ;
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);
	char Error[1024]={0};
	FUNC_SESSION_MAN_EVENTS_ONINCOMMINGSESSION OldOnInCommingSession = NULL;
	g_log.Log(CELOG_DEBUG, _T("In My_NewSessionManEvents_OnInCommingSession\n"));

	{
		MUTEX _mutex_(&gMutexMapSessionManEvents);

		MAP_SESSION_MANAGER_EVENTS::iterator it = gMapSessionManEvents.find(This);
		if(it != gMapSessionManEvents.end())
		{
			OldOnInCommingSession = it->second.OnInCommingSession;
		}
	}

	if(!OldOnInCommingSession) {
		sprintf_s(Error,sizeof(Error),"Oops!!! OnInCommingSessionAdvise %p hasn't old OnInCommingSession\n",This);
		g_log.Log(CELOG_DEBUG, Error);
		return E_FAIL;
	}

	CComPtr<IUccSession> pSession=NULL;
	CComPtr<IUccSessionParticipant> pInvitingParticipant;
	HRESULT hr=pEventData->get_Inviter(&pInvitingParticipant);
	hr=pEventData->get_Session(&pSession);
	if(hr == S_OK && pSession) {
		HookSession(pSession);
	}
	return E_FAIL;


	hr = OldOnInCommingSession(This, pEndPoint, pEventData);

	g_log.Log(CELOG_DEBUG, _T("OnInCommingSession return %d\n"), hr);

	return hr;
}

HRESULT	__stdcall Try_NewSessionManEvents_OnInCommingSession(
	_IUccSessionManagerEvents *This,
	IUccEndpoint *pEndPoint, 
	IUccIncomingSessionEvent *pEventData)
{
	HRESULT res = S_FALSE;
	g_log.Log(CELOG_ERR,"Try_NewSessionManEvents_OnInCommingSession beg\n");

	__try
	{
		res = My_NewSessionManEvents_OnInCommingSession(		This,
			pEndPoint,
			pEventData ) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}

	g_log.Log(CELOG_ERR,"Try_NewSessionManEvents_OnInCommingSession end\n");
	return res;
}

void HookIMSession(IUccInstantMessagingSession* pIMSession)
{
	if(!pIMSession) return;

	MUTEX _mutex_(&gMutexMapIMSession);
	char Error[256]={0};
	MAP_IM_SESSION::const_iterator it=gMapIMSession.find(pIMSession);
	if(it != gMapIMSession.end())
	{
		sprintf_s(Error,sizeof(Error),"pIMSession %p has been hooked\n",
			pIMSession);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	//This is new IM session. If there is disclaimer, it should be sent out with
	//the first message of this session. 
	ocePolicyEval.AddToIMFirstMsg(pIMSession);

	PVOID* Vtable = (*(PVOID**)pIMSession);	
	OLD_IM_SESSION_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_IM_SESSION_FUNCS));

	Functions.Vtable = Vtable;

	PVOID pointer = Vtable[3];
	memcpy(&(Functions.SendMessage),&pointer,sizeof(ULONG));
	Functions.OriSendMessage = Functions.SendMessage;

	for(it = gMapIMSession.begin(); it != gMapIMSession.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			// 这里用||，严格的做法应该是记录哪些相同，不同的还是应该HOOK，相同的直接赋值即可
			if( it->second.OriSendMessage == Functions.OriSendMessage)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pIMSession %p has same vtable with %p, so we do not need to hook\n",
				pIMSession,static_cast<IUccInstantMessagingSession*>(it->first));
			g_log.Log(CELOG_DEBUG, Error);
			// 因为在My_NewSession_Xxx中需要根据pSession寻找Old_Xxx，所以需要把这一项加上，但是不Hook
			gMapIMSession.insert(std::make_pair(pIMSession,it->second));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook pIMSession %p\n", pIMSession);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriSendMessage,(PVOID)Try_NewIM_Session_SendMessage ,(PVOID*)&Functions.SendMessage ) ;

	gMapIMSession.insert(std::make_pair(pIMSession,Functions));
}

void HookSubscription(IUccSubscription *pSubscription)
{
	if(pSubscription == NULL) 
		return;

	MUTEX _mutex_(&gMutexMapSubscription);

	char Error[256]={0};
	if(gMapSubscription.find(pSubscription) != gMapSubscription.end())
	{
		sprintf_s(Error,sizeof(Error),"Subscription %p has been hooked\n",
			pSubscription);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pSubscription);	

	OLD_SUBSCRIPTION_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SUBSCRIPTION_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[3];
	memcpy(&(Functions.CreatePresentity),&Pointer,sizeof(ULONG));
	Functions.OriCreatePresentity = Functions.CreatePresentity;

	Pointer = Vtable[4];
	memcpy(&(Functions.AddPresentity),&Pointer,sizeof(ULONG));
	Functions.OriAddPresentity = Functions.AddPresentity;

	Pointer = Vtable[7];
	memcpy(&(Functions.AddCategoryName),&Pointer,sizeof(ULONG));
	Functions.OriAddCategoryName = Functions.AddCategoryName;

	for(MAP_SUBSCRIPTION::const_iterator it = gMapSubscription.begin();
		it != gMapSubscription.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			if( it->second.OriCreatePresentity == Functions.OriCreatePresentity ||
				it->second.OriAddPresentity == Functions.OriAddPresentity ||
				it->second.OriAddCategoryName == Functions.OriAddCategoryName)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pUccSubscription %p has same vtable with %p, so we do not need to hook\n",
				pSubscription,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			OLD_SUBSCRIPTION_FUNCS Function = it->second;
			gMapSubscription.insert(std::make_pair(pSubscription,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook Subscription %p\n", pSubscription);
	g_log.Log(CELOG_DEBUG, Error);


	HookCode( (PVOID)Functions.OriCreatePresentity,(PVOID)Try_NewSubscription_CreatePresentity ,(PVOID*)&Functions.CreatePresentity ) ;
	HookCode( (PVOID)Functions.OriAddPresentity,(PVOID)Try_NewSubscription_AddPresentity ,(PVOID*)&Functions.AddPresentity ) ;
	HookCode( (PVOID)Functions.OriAddCategoryName,(PVOID)Try_NewSubscription_AddCategoryName ,(PVOID*)&Functions.AddCategoryName ) ;

	gMapSubscription.insert(std::make_pair(pSubscription,Functions));
}

//Initialization for policy evaluation
bool InitPolicyEval(WCHAR *processName)
{
	ocePolicyEval.SetupProcessName(processName);
	return TRUE;
}

//Evaluate if it is allowed to copy data to clipboard for the current endPointSip
bool PolicyEvalCopyAction()
{
	if(ocePolicyEval.EvalCopyAction())
		return true;
	return false;
}

//Evaluate if it is allowed to live meeting 
bool PolicyEvalLivemeeting(wchar_t *confURI)
{
	if(ocePolicyEval.EvalLivemeeting(confURI))
		return true;
	return false;
}

//Add thread id and open file info mapping for later policy evaluation
void AddFileMappingForPolicyEval(DWORD tid, LPCWSTR fileName, HANDLE handle)
{
	ocePolicyEval.AddFileInfoMapping(tid, fileName, handle);
}

//Add a thread and its copy evaluation result pair to cachedCopyEvalResults
void CacheCopyEvalResult(HWND hWnd, bool bAllow)
{
	ocePolicyEval.CacheCopyEvalResult(hWnd, bAllow);
}

//Fetch the cached copy eval result of a thread
//When this function reture false, it means that the result is unknown
//If the last eval time is older than 10 sec, discard the result and return false;
bool GetCachedCopyEvalResult(HWND hWnd, bool &bAllow)
{
	return ocePolicyEval.GetCachedCopyEvalResult(hWnd, bAllow);
}

void HookSessionParticipant(IUccSessionParticipant *pSessionParticipant)
{
	g_log.Log(CELOG_DEBUG, _T("Hook session participant\n"));

	if(!pSessionParticipant) return;

	MUTEX _mutex_(&gMutexMapSessionParticipant);

	char Error[256]={0};
	if(gMapSessionParticipant.find(pSessionParticipant) != gMapSessionParticipant.end())
	{
		sprintf_s(Error, sizeof(Error), "MapSessionParticipant %p has been hooked\n", &gMapSessionParticipant);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pSessionParticipant);	

	OLD_SESSION_PARTICIPANT_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SESSION_PARTICIPANT_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[9];
	memcpy(&(Functions.CreateParticipantEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriCreateParticipantEndpoint  = Functions.CreateParticipantEndpoint;

	Pointer = Vtable[10];
	memcpy(&(Functions.CopyParticipantEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriCopyParticipantEndpoint  = Functions.CopyParticipantEndpoint;

	Pointer = Vtable[11];
	memcpy(&(Functions.AddParticipantEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriAddParticipantEndpoint = Functions.AddParticipantEndpoint;

	Pointer = Vtable[12];
	memcpy(&(Functions.RemoveParticipantEndpoint),&Pointer,sizeof(ULONG));
	Functions.OriRemoveParticipantEndpoint  = Functions.RemoveParticipantEndpoint;

	for(MAP_SESSION_PARTICIPANT::const_iterator it = gMapSessionParticipant.begin();
		it != gMapSessionParticipant.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			if( it->second.OriAddParticipantEndpoint == Functions.OriAddParticipantEndpoint ||
				it->second.OriCopyParticipantEndpoint == Functions.OriCopyParticipantEndpoint ||
				it->second.OriCreateParticipantEndpoint == Functions.OriCreateParticipantEndpoint ||
				it->second.OriRemoveParticipantEndpoint == Functions.OriRemoveParticipantEndpoint)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"pSessionParticipant %p has same vtable with %p, so we do not need to hook\n",
				pSessionParticipant,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			OLD_SESSION_PARTICIPANT_FUNCS Function = it->second;
			gMapSessionParticipant.insert(std::make_pair(pSessionParticipant,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook SubscriptionManager %p\n", pSessionParticipant);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriAddParticipantEndpoint,(PVOID)Try_NewSessionParticipant_AddParticipantEndpoint ,(PVOID*)&Functions.AddParticipantEndpoint ) ;
	HookCode( (PVOID)Functions.OriCopyParticipantEndpoint, (PVOID)Try_NewSessionParticipant_CopyParticipantEndpoint, (PVOID*)&Functions.CopyParticipantEndpoint);
	HookCode( (PVOID)Functions.OriCreateParticipantEndpoint, (PVOID)Try_NewSessionParticipant_CreateParticipantEndpoint, (PVOID*)&Functions.CreateParticipantEndpoint);
	HookCode( (PVOID)Functions.OriRemoveParticipantEndpoint, (PVOID)Try_NewSessionParticipant_RemoveParticipantEndpoint, (PVOID*)&Functions.RemoveParticipantEndpoint);


	gMapSessionParticipant.insert(std::make_pair(pSessionParticipant,Functions));
	g_log.Log(CELOG_DEBUG, _T("Hook session participant done\n"));

}

void HookConferenceSession(IUccConferenceSession *pConfSession)
{
	g_log.Log(CELOG_DEBUG, _T("In HookConferenceSession\n"));

	if(!pConfSession) return;

	MUTEX _mutex_(&gMutexMapConfSession);

	char Error[256]={0};
	if(gMapConfSession.find(pConfSession) != gMapConfSession.end())
	{
		sprintf_s(Error,sizeof(Error),"pConfSession %p has been hooked\n",
			pConfSession);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pConfSession);	

	OLD_CONF_SESSION_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_CONF_SESSION_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[4];
	memcpy(&(Functions.Enter),&Pointer,sizeof(ULONG));
	Functions.OriEnter  = Functions.Enter;

	Pointer = Vtable[5];
	memcpy(&(Functions.Leave),&Pointer,sizeof(ULONG));
	Functions.OriLeave  = Functions.Leave;

	Pointer = Vtable[6];
	memcpy(&(Functions.SetProperty),&Pointer,sizeof(ULONG));
	Functions.OriSetProperty = Functions.SetProperty;

	for(MAP_CONF_SESSION::const_iterator it = gMapConfSession.begin();
		it != gMapConfSession.end(); ++it)
	{
		bool Duplicate = false;
		if(it->second.Vtable == Vtable)
		{
			Duplicate = true;
		}
		else
		{
			if( it->second.Enter == Functions.OriEnter ||
				it->second.Leave == Functions.OriLeave ||
				it->second.SetProperty == Functions.SetProperty)
				Duplicate = true;
		}
		if(Duplicate)
		{
			sprintf_s(Error,sizeof(Error),"gMapConfSession %p has same vtable with %p, so we do not need to hook\n",
				pConfSession,it->first);
			g_log.Log(CELOG_DEBUG, Error);
			OLD_CONF_SESSION_FUNCS Function = it->second;
			gMapConfSession.insert(std::make_pair(pConfSession,Function));
			return;
		}
	}

	sprintf_s(Error,sizeof(Error),"Hook SubscriptionManager %p\n", pConfSession);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.OriEnter,(PVOID)Try_NewConfSession_Enter ,(PVOID*)&Functions.Enter ) ;
	HookCode( (PVOID)Functions.OriLeave,(PVOID)Try_NewConfSession_Leave ,(PVOID*)&Functions.Leave ) ;
	HookCode( (PVOID)Functions.OriSetProperty,(PVOID)Try_NewConfSession_SetProperty ,(PVOID*)&Functions.SetProperty ) ;

	gMapConfSession.insert(std::make_pair(pConfSession,Functions));
}

void HookConnectionPointContainer(IConnectionPointContainer* pConnectionPointContainer)
{
	if(!pConnectionPointContainer) return;

	MUTEX _mutex_(&gMutexMapMapConnPointContainer);

	char Error[256]={0};
	if(gMapConnPointContainer.find(pConnectionPointContainer) != gMapConnPointContainer.end())
	{
		sprintf_s(Error,sizeof(Error),"ConnectionPointContainer %p has been hooked\n",
			pConnectionPointContainer);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pConnectionPointContainer);	

	OLD_CONNPOINTCONTAINER_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_CONNPOINTCONTAINER_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[4];
	memcpy(&(Functions.FindConnectionPoint),&Pointer,sizeof(ULONG));
	Functions.oriFindConnectionPoint = Functions.FindConnectionPoint;

	Pointer = Vtable[3];
	memcpy(&(Functions.EnumConnectionPoints),&Pointer,sizeof(ULONG));
	Functions.oriEnumConnectionPoints = Functions.EnumConnectionPoints;

	for(MAP_CONN_POINT_CONTAINER::const_iterator it = gMapConnPointContainer.begin();
		it != gMapConnPointContainer.end(); ++it) {
			bool Duplicate = false;
			if(it->second.Vtable == Vtable)
			{
				Duplicate = true;
			} else {
				if(it->second.oriFindConnectionPoint == Functions.oriFindConnectionPoint ||
					it->second.oriEnumConnectionPoints == Functions.oriEnumConnectionPoints)
					Duplicate = true;
			}
			if(Duplicate)
			{
				sprintf_s(Error,sizeof(Error),"pIConnectionPointContainer %p has same vtable with %p, so we do not need to hook\n",
					pConnectionPointContainer,it->first);
		//		g_log.Log(CELOG_DEBUG, Error);
				OLD_CONNPOINTCONTAINER_FUNCS Function = it->second;
				gMapConnPointContainer.insert(std::make_pair(pConnectionPointContainer,Function));
				return;
			}
	}

	sprintf_s(Error,sizeof(Error),"Hook ConnectionPointContainer %p\n", pConnectionPointContainer);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.oriFindConnectionPoint,(PVOID)Try_NewConnPointContainer_FindConnectionPoint ,(PVOID*)&Functions.FindConnectionPoint ) ;
	HookCode( (PVOID)Functions.oriEnumConnectionPoints, (PVOID)Try_NewConnPointContainer_EnumConnectionPoints, (PVOID*)&Functions.EnumConnectionPoints);


	gMapConnPointContainer.insert(std::make_pair(pConnectionPointContainer,Functions));
}

void HookConnectionPoint(IConnectionPoint *pConnectionPoint)
{
	if(!pConnectionPoint) return;

	MUTEX _mutex_(&gMutexMapMapConnPoint);

	char Error[256]={0};
	if(gMapConnPoint.find(pConnectionPoint) != gMapConnPoint.end())
	{
		sprintf_s(Error,sizeof(Error),"ConnectionPoint %p has been hooked\n",
			pConnectionPoint);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pConnectionPoint);	

	OLD_CONNPOINT_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_CONNPOINT_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[5];
	memcpy(&(Functions.Advise),&Pointer,sizeof(ULONG));
	Functions.oriAdvise = Functions.Advise;

	for(MAP_CONN_POINT::const_iterator it = gMapConnPoint.begin();
		it != gMapConnPoint.end(); ++it) {
			bool Duplicate = false;
			if(it->second.Vtable == Vtable)
			{
				Duplicate = true;
			} else {
				if(it->second.oriAdvise == Functions.oriAdvise)
					Duplicate = true;
			}
			if(Duplicate)
			{
				sprintf_s(Error,sizeof(Error),"pIConnectionPoint %p has same vtable with %p, so we do not need to hook\n",
					pConnectionPoint,it->first);
	//			g_log.Log(CELOG_DEBUG, Error);
				OLD_CONNPOINT_FUNCS Function = it->second;
				gMapConnPoint.insert(std::make_pair(pConnectionPoint,Function));
				return;
			}
	}

	sprintf_s(Error,sizeof(Error),"Hook ConnectionPoint %p\n", pConnectionPoint);
	g_log.Log(CELOG_DEBUG, Error);

	HookCode( (PVOID)Functions.oriAdvise,(PVOID)Try_NewConnPoint_Advise ,(PVOID*)&Functions.Advise ) ;


	gMapConnPoint.insert(std::make_pair(pConnectionPoint,Functions));
}

void HookSessionManagerEvents(_IUccSessionManagerEvents *pSessionManEvents)
{
	if(!pSessionManEvents) return;

	MUTEX _mutex_(&gMutexMapSessionManEvents);

	char Error[256]={0};
	if(gMapSessionManEvents.find(pSessionManEvents) != gMapSessionManEvents.end())
	{
		sprintf_s(Error,sizeof(Error),"SessionManagerEvents %p has been hooked\n",
			pSessionManEvents);
		g_log.Log(CELOG_DEBUG, Error);
		return;
	}

	PVOID* Vtable = (*(PVOID**)pSessionManEvents);	

	OLD_SESSION_MANAGER_EVENTS_FUNCS Functions;
	memset(&Functions,0,sizeof(OLD_SESSION_MANAGER_EVENTS_FUNCS));

	Functions.Vtable = Vtable;

	PVOID Pointer = Vtable[8];
	memcpy(&(Functions.OnInCommingSession),&Pointer,sizeof(ULONG));
	Functions.oriOnInCommingSession = Functions.OnInCommingSession;

	for(MAP_SESSION_MANAGER_EVENTS::const_iterator it = gMapSessionManEvents.begin();
		it != gMapSessionManEvents.end(); ++it) {
			bool Duplicate = false;
			if(it->second.Vtable == Vtable)
			{
				Duplicate = true;
			} else {
				if(it->second.oriOnInCommingSession == Functions.oriOnInCommingSession)
					Duplicate = true;
			}
			if(Duplicate)
			{
				sprintf_s(Error,sizeof(Error),"pIUccSessionManagerEvents %p has same vtable with %p, so we do not need to hook\n",
					pSessionManEvents,it->first);
				g_log.Log(CELOG_DEBUG, Error);
				OLD_SESSION_MANAGER_EVENTS_FUNCS Function = it->second;
				gMapSessionManEvents.insert(std::make_pair(pSessionManEvents,Function));
				return;
			}
	}

	g_log.Log(CELOG_DEBUG, _T("Hook SessionManagerEvents %p\n"), pSessionManEvents);

	HookCode( (PVOID)Functions.oriOnInCommingSession,(PVOID)Try_NewSessionManEvents_OnInCommingSession ,(PVOID*)&Functions.OnInCommingSession ) ;

	gMapSessionManEvents.insert(std::make_pair(pSessionManEvents,Functions));
}

// Do SetClipboardData Evaluation, Added By Jacky.Dong 2011-11-23
bool DoSetClipboardDataEval()
{
	bool bAllow;
	
	HWND hWnd = GetForegroundWindow();
	bool bResultUnknown = GetCachedCopyEvalResult(hWnd, bAllow);
	g_log.Log(CELOG_DEBUG, _T("DoSetClipboardDataEval cached result: %s, hwnd [%d]\n"), bResultUnknown ? L"Unknown" : L"Known", hWnd);
	if(bResultUnknown)
	{
		g_log.Log(CELOG_DEBUG, _T("DoSetClipboardDataEval: Do Copy Content policy evaluation\n"));

		bAllow = PolicyEvalCopyAction();
		CacheCopyEvalResult(hWnd, bAllow);
	}

	g_log.Log(CELOG_DEBUG, _T("DoSetClipboardDataEval: %s\n"), bAllow ? L"Allowed" : L"Denied");
    return bAllow;
}