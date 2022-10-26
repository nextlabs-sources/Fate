// HookDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Coference.h"
#include "CollManager.h"
#include "AppShare.h"
//#include "UccpPlatform.h"
#include "Platform.h"
#include "HookedFlash.h"
#include "HookedLmDoc.h"
#include "MsgHook.h"
#include "Commdlg.h"
#include "shellapi.h"
#include "shlobj.h"
#include <algorithm>

//#include <eframework/platform/ignore_application.hpp>
//#include "Communication.h"
#ifndef HANDOUT_WND_NAME
#define HANDOUT_WND_NAME L"HANDOUTS"
#endif
#ifndef HANDOUT_DROP_WND_CLASS
#define HANDOUT_DROP_WND_CLASS L"SysListView32"
#endif
#ifndef HANDOUT_DOWNLOAD_TITLE_PREFIX
#define HANDOUT_DOWNLOAD_TITLE_PREFIX	L"SELECT A DESTINATION FOR "
#endif
#ifndef HANDOUT_DOWNLOAD_TITLE_END
#define HANDOUT_DOWNLOAD_TITLE_END		L" THEN CLICK OK"
#endif 
#ifdef _MANAGED
#pragma managed(push, off)
#endif
#pragma comment( lib, "Oleacc.lib" )
// dhook export function
LONG (WINAPI* gDetourTransactionBegin)() = NULL;
LONG (WINAPI* gDetourTransactionCommit)() = NULL;
LONG (WINAPI* gDetourUpdateThread)(HANDLE hThread) = NULL;
LONG (WINAPI* gDetourAttach)(PVOID *ppPointer,PVOID pDetour) = NULL;
LONG (WINAPI* gDetourDetach)(PVOID *ppPointer,PVOID pDetour) = NULL;
CRITICAL_SECTION g_csPolicyInstance;

HRESULT WINAPI my_CoCreateInstanceEx(
									 IN REFCLSID                    rclsid,
									 IN IUnknown    *               punkOuter, // only relevant locally
									 IN DWORD                       dwClsCtx,
									 IN COSERVERINFO *              pServerInfo,
									 IN DWORD                       dwCount,
									 IN OUT MULTI_QI    *           pResults
									 );
HRESULT __stdcall my_CoCreateInstance(
									  REFCLSID rclsid,
									  LPUNKNOWN pUnkOuter,
									  DWORD dwClsContext,
									  REFIID riid,
									  LPVOID * ppv
									  );
HANDLE gThreadHandle = NULL;
HANDLE gThreadStop = NULL;

static int	g_nInter=0;

static HRESULT (__stdcall* Old_CoCreateInstance)(
	REFCLSID rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD dwClsContext,
	REFIID riid,
	LPVOID * ppv
	) = CoCreateInstance;

static HRESULT (__stdcall* Old_CoCreateInstanceEx )(
	IN REFCLSID                    Clsid,
	IN IUnknown    *               punkOuter, // only relevant locally
	IN DWORD                       dwClsCtx,
	IN COSERVERINFO *              pServerInfo,
	IN DWORD                       dwCount,
	IN OUT MULTI_QI    *           pResults
	) = CoCreateInstanceEx;

static HRESULT (__stdcall* Old_CoGetClassObject)(
	REFCLSID rclsid,  DWORD dwClsContext,  LPVOID pvReserved,
	REFIID riid,  LPVOID FAR* ppv
	) = CoGetClassObject;

HRESULT WINAPI New_CoCreateInstanceEx(
									  IN REFCLSID                    rclsid,
									  IN IUnknown    *               punkOuter, // only relevant locally
									  IN DWORD                       dwClsCtx,
									  IN COSERVERINFO *              pServerInfo,
									  IN DWORD                       dwCount,
									  IN OUT MULTI_QI    *           pResults
									  )
{
	HRESULT hr = S_FALSE ;
	if( LMEIsDisabled() == true )
	{

		if( Old_CoCreateInstanceEx )
		{
			hr = Old_CoCreateInstanceEx(
				rclsid,
				punkOuter,
				dwClsCtx,
				pServerInfo,
				dwCount, 
				pResults
				);
		}

	}
	__try
	{
		return   my_CoCreateInstanceEx(
			rclsid,
			punkOuter,
			dwClsCtx,
			pServerInfo,
			dwCount, 
			pResults
			);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW( (L"New_CoCreateInstanceEx exception")) ;	;
	}
	return hr;
}
HRESULT WINAPI my_CoCreateInstanceEx(
									 IN REFCLSID                    rclsid,
									 IN IUnknown    *               punkOuter, // only relevant locally
									 IN DWORD                       dwClsCtx,
									 IN COSERVERINFO *              pServerInfo,
									 IN DWORD                       dwCount,
									 IN OUT MULTI_QI    *           pResults
									 )
{	 
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT Ret = Old_CoCreateInstanceEx(
		rclsid,
		punkOuter,
		dwClsCtx,
		pServerInfo,
		dwCount, 
		pResults
		);

	OutputDebugString( TEXT("New_CoCreateInstanceEx") );

	if( !pResults )
	{
		return Ret;
	}

	IUnknown* ppv = pResults->pItf;    

	if( Ret == S_OK)
	{  
		if( IsEqualCLSID( UCCP::CLSID_UccPlatform, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			//UCCP::IUccPlatform* pUccPlaform = (UCCP::IUccPlatform*)(*ppv);
			//ShowInfo( TEXT("branch HookPlatform called successfully.") );
			CHookedPlatform::GetInstance()->Hook( PVOID(ppv) );
		}

		else if( IsEqualCLSID( CONFAPI::CLSID_OfficeConferencing, rclsid ) )// && memcmp(&CONFAPI::IID_IConferencingCenter,&riid,sizeof(riid))==0 )
		{
			//CONFAPI::IConferencingCenter* pCofCenter = (CONFAPI::IConferencingCenter*)(*ppv);
			//CHookedConfCenter::GetInstance()->Hook( PVOID(ppv) );//HookConferenceCenter(pCofCenter);
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccPropertyCollection, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
//			int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccOperationContext, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			//UCCP::IUccOperationContext* pOperationContext = (UCCP::IUccOperationContext*)(*ppv);
			//CHookedOperationContext::GetInstance()->Hook( PVOID(pOperationContext) );
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingRequest, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingResponse, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( CLSID_Document, rclsid )/*CLSID_LMDocument == rclsid*/ )
		{
		}
		else if( IsEqualCLSID( MODI::CLSID_LMDocView, rclsid ) )
		{

//			int nTest = 0;
		}
		else if( IsEqualCLSID( ImportUtilLib::CLSID_DocConverter, rclsid ) )
		{
			CHookedDocConverter::GetInstance()->Hook( ppv  );
		}
		else if( IsEqualCLSID( LMC_Coll::CLSID_Collaborate, rclsid ) )
		{
			//ICollaborateManager* pManager=(ICollaborateManager*)(*ppv);
			CHookedCollManager::GetInstance()->Hook( PVOID( ppv ) );
		}
		else if( IsEqualCLSID( LMC_AppShare::CLSID_AppShare, rclsid ) )
		{
			//ICollaborateAppShare* pAppShare = (ICollaborateAppShare*)(*ppv);
			CHookedCollAppShare::GetInstance()->Hook( PVOID(ppv) ) ;
		}  
		else if( IsEqualCLSID( CLSID_ShockwaveFlash, rclsid ) )
		{
			//CHookedFlash::GetInstance()->Hook( ppv );
		}        
		else
		{
	//		int nTest = 0;
		}
	}
	return Ret;
}

HRESULT __stdcall New_CoCreateInstance(
									   REFCLSID rclsid,
									   LPUNKNOWN pUnkOuter,
									   DWORD dwClsContext,
									   REFIID riid,
									   LPVOID * ppv
									   )
{
	HRESULT hr = S_FALSE ;
	if( LMEIsDisabled() == true )
	{

		if( Old_CoCreateInstance )
		{
			hr = Old_CoCreateInstance(
				rclsid,
				pUnkOuter,
				dwClsContext,
				riid,
				ppv
				);
		}

	}
	__try
	{
		return   my_CoCreateInstance(
			rclsid,
			pUnkOuter,
			dwClsContext,
			riid,
			ppv
			);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall my_CoCreateInstance(
									  REFCLSID rclsid,
									  LPUNKNOWN pUnkOuter,
									  DWORD dwClsContext,
									  REFIID riid,
									  LPVOID * ppv
									  )
{   
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	//    const GUID CLSID_LMDocument =	    {0x3eacf900,0x8e51,0x4895,{0x8b,0xd7,0x4c,0x9c,0x1c,0x92,0x7d,0xaa}};  
	/*if( CLSID_LMDocument == rclsid )
	{
	return E_FAIL;
	}*/

	//OutputDebugString( TEXT("New_CoCreateInstance") );

	HRESULT Ret = Old_CoCreateInstance(
		rclsid,
		pUnkOuter,
		dwClsContext,
		riid,
		ppv
		);

	// hook interface
	if( Ret == S_OK)
	{  
		if( IsEqualCLSID( UCCP::CLSID_UccPlatform, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			UCCP::IUccPlatform* pUccPlaform = (UCCP::IUccPlatform*)(*ppv);
			//ShowInfo( TEXT("branch HookPlatform called successfully.") );
			CHookedPlatform::GetInstance()->Hook( PVOID(pUccPlaform) );
		}

		if( IsEqualCLSID( CONFAPI::CLSID_OfficeConferencing, rclsid ) )// && memcmp(&CONFAPI::IID_IConferencingCenter,&riid,sizeof(riid))==0 )
		{
			CONFAPI::IConferencingCenter* pCofCenter = (CONFAPI::IConferencingCenter*)(*ppv);
			CHookedConfCenter::GetInstance()->Hook( PVOID(pCofCenter) );//HookConferenceCenter(pCofCenter);
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccPropertyCollection, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccOperationContext, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		UCCP::IUccOperationContext* pOperationContext = (UCCP::IUccOperationContext*)(*ppv);
			//CHookedOperationContext::GetInstance()->Hook( PVOID(pOperationContext) );
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingRequest, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingResponse, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( CLSID_Document, rclsid )/*CLSID_LMDocument == rclsid*/ )
		{
			/*IUnknown* pUnknown = (IUnknown*)(*ppv);
			IDocument* pDoc = 0;
			pUnknown->QueryInterface( MODI::IID_IDocument, (void**)&pDoc );
			int nTest =0;*/
			//CHookedLmDoc::GetInstance()->Hook( *ppv  );
		}
		else if( IsEqualCLSID( MODI::CLSID_LMDocView, rclsid ) )
		{

	//		int nTest = 0;
		}
		else if( IsEqualCLSID( ImportUtilLib::CLSID_DocConverter, rclsid ) )
		{
			CHookedDocConverter::GetInstance()->Hook( *ppv  );
		}
		else if( IsEqualCLSID( LMC_Coll::CLSID_Collaborate, rclsid ) )
		{
			ICollaborateManager* pManager=(ICollaborateManager*)(*ppv);
			CHookedCollManager::GetInstance()->Hook( PVOID( pManager ) );
		}
		else if( IsEqualCLSID( LMC_AppShare::CLSID_AppShare, rclsid ) )
		{
			ICollaborateAppShare* pAppShare = (ICollaborateAppShare*)(*ppv);
			CHookedCollAppShare::GetInstance()->Hook( PVOID(pAppShare) ) ;
		}  
		else if( IsEqualCLSID( CLSID_ShockwaveFlash, rclsid ) )
		{
			//CHookedFlash::GetInstance()->Hook( *ppv );
		}        
		else
		{
			//IShockwaveFlash
			//DocConverter
//			int nTest = 0;
		}
	}
	return Ret;
}




HRESULT __stdcall New_CoGetClassObject(
									   REFCLSID rclsid,  DWORD dwClsContext,  LPVOID pvReserved,
									   REFIID riid,  LPVOID FAR* ppv
									   )
{     
//	const GUID CLSID_LMDocument =
//	{0x3eacf900,0x8e51,0x4895,{0x8b,0xd7,0x4c,0x9c,0x1c,0x92,0x7d,0xaa}};  
	/*if( CLSID_LMDocument == rclsid )
	{
	return E_FAIL;
	}*/
	HRESULT Ret = Old_CoGetClassObject(
		rclsid,
		dwClsContext,
		pvReserved,
		riid,
		ppv
		);

	OutputDebugString( L"New_CoGetClassObject" );
	// hook interface
	if( Ret == S_OK)
	{
		if( IsEqualCLSID( UCCP::CLSID_UccPlatform, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			UCCP::IUccPlatform* pUccPlaform = (UCCP::IUccPlatform*)(*ppv);
			//ShowInfo( TEXT("branch HookPlatform called successfully.") );
			CHookedPlatform::GetInstance()->Hook( PVOID(pUccPlaform) );
		}
		else if( IsEqualCLSID( CONFAPI::CLSID_OfficeConferencing, rclsid ) )// && memcmp(&CONFAPI::IID_IConferencingCenter,&riid,sizeof(riid))==0 )
		{
			CONFAPI::IConferencingCenter* pCofCenter = (CONFAPI::IConferencingCenter*)(*ppv);
			CHookedConfCenter::GetInstance()->Hook( PVOID(pCofCenter) );//HookConferenceCenter(pCofCenter);
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccPlatform, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			UCCP::IUccPlatform* pUccPlaform = (UCCP::IUccPlatform*)(*ppv);
			CHookedPlatform::GetInstance()->Hook( PVOID(pUccPlaform) );
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccPropertyCollection, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccOperationContext, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
			// UCCP::IUccOperationContext* pOperationContext = (UCCP::IUccOperationContext*)(*ppv);
			//CHookedOperationContext::GetInstance()->Hook( PVOID(pOperationContext) );
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingRequest, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( UCCP::CLSID_UccSignalingResponse, rclsid ) )// && memcmp(&UCCP::IID_IUccPlatform,&riid,sizeof(riid))==0)
		{
	//		int nTest = 0;
		}
		else if( IsEqualCLSID( CLSID_Document, rclsid )/*CLSID_LMDocument == rclsid*/ )
		{
			/*IUnknown* pUnknown = (IUnknown*)(*ppv);
			IDocument* pDoc = 0;
			pUnknown->QueryInterface( MODI::IID_IDocument, (void**)&pDoc );
			int nTest =0;*/
			//CHookedLmDoc::GetInstance()->Hook( *ppv  );
		}
		else if( IsEqualCLSID( MODI::CLSID_LMDocView, rclsid ) )
		{

	//		int nTest = 0;
		}
		else if( IsEqualCLSID( ImportUtilLib::CLSID_DocConverter, rclsid ) )
		{
			CHookedDocConverter::GetInstance()->Hook( *ppv  );
		}
		else if( IsEqualCLSID( LMC_Coll::CLSID_Collaborate, rclsid ) )
		{
			ICollaborateManager* pManager=(ICollaborateManager*)(*ppv);
			CHookedCollManager::GetInstance()->Hook( PVOID( pManager ) );
		}
		else if( IsEqualCLSID( LMC_AppShare::CLSID_AppShare, rclsid ) )
		{
			ICollaborateAppShare* pAppShare = (ICollaborateAppShare*)(*ppv);
			CHookedCollAppShare::GetInstance()->Hook( PVOID(pAppShare) ) ;
		}  
		else if( IsEqualCLSID( CLSID_ShockwaveFlash, rclsid ) )
		{
		//	CHookedFlash::GetInstance()->Hook( *ppv );
		}        
		else
		{
			//IShockwaveFlash
			//DocConverter
	//		int nTest = 0;
		}
	}
	return Ret;
}
static PIDLIST_ABSOLUTE (WINAPI* Old_SHBrowseForFolderW)(          LPBROWSEINFOW lpbi) ;
PIDLIST_ABSOLUTE WINAPI my_SHBrowseForFolderW(          LPBROWSEINFOW lpbi ) ;
PIDLIST_ABSOLUTE WINAPI New_SHBrowseForFolderW(          LPBROWSEINFOW lpbi )
{
	PIDLIST_ABSOLUTE hr = NULL ;
	if( LMEIsDisabled() == true )
	{

		if( Old_SHBrowseForFolderW )
		{
			hr = Old_SHBrowseForFolderW(lpbi) ;
		}

	}
	__try
	{
		return   my_SHBrowseForFolderW(lpbi) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
BOOL WINAPI  GetObjectRole( IAccessible* _iacc,VARIANT* _pVt )
{
	BOOL bRet = FALSE ;
	VARIANT VT ;
	VT.vt = VT_I4;
	VT.lVal = CHILDID_SELF ;
	if ( _iacc == NULL )
	{
		return bRet ;
	}
	if ( FAILED(_iacc->get_accRole( VT,_pVt ) ) )
	{
		return bRet ;
	}
	DPW((L"Current Object Role:[%d]",_pVt->lVal));
	return TRUE ;

}
BOOL WINAPI  GetObjectRole( IAccessible* _iacc,VARIANT childID,VARIANT* _pVt )
{
	BOOL bRet = FALSE ;
	if ( _iacc == NULL )
	{
		return bRet ;
	}
	if ( FAILED(_iacc->get_accRole( childID,_pVt ) ) )
	{
		return bRet ;
	}
	DPW((L"Current Object Rolesssss:[%d]",_pVt->lVal));
	return TRUE ;

}

BOOL WINAPI  GetObjectState( IAccessible* _iacc,VARIANT* _pVt ) 
{
	BOOL bRet = FALSE ;
	VARIANT VT ;
	VariantClear( _pVt ) ;
	VT.vt = VT_I4;
	VT.lVal = CHILDID_SELF ;
	if ( _iacc == NULL )
	{
		return bRet ;
	}
	if ( FAILED(_iacc->get_accState( VT,_pVt ) ) )
	{
		return bRet ;
	}
	DPW((L"Current Object State:[%d]",_pVt->lVal));
	return TRUE ;
}
BOOL WINAPI  GetObjectState( IAccessible* _iacc,VARIANT childID,VARIANT* _pVt ) 
{
	BOOL bRet = FALSE ;
	VariantClear( _pVt ) ;

	if ( _iacc == NULL )
	{
		return bRet ;
	}
	if ( FAILED(_iacc->get_accState( childID,_pVt ) ) )
	{
		

		return bRet ;
	}
	DPW((L"Current Object State:sssss[%d]",_pVt->lVal));
	return TRUE ;
}
std::wstring WINAPI  GetObjectName( CComPtr<IAccessible> _iacc )
{
	std::wstring sRet ;
	BSTR bStr = NULL ;
	if (_iacc==NULL)
	{
		return sRet ;
	}	
	VARIANT VT ;
	VariantInit( &VT ) ;
	VT.vt = VT_I4;
	VT.lVal = CHILDID_SELF ;
	if ( !FAILED(_iacc->get_accName( VT, &bStr ) ) )
	{
		if ( bStr != NULL )
		{
			sRet = bStr ;
		}
		::SysFreeString( bStr ) ;
	}
	return sRet ;

}
std::wstring WINAPI  GetObjectDescription( IAccessible* _iacc,VARIANT childID  )
{
	std::wstring sRet ;
	BSTR bStr = NULL ;
	if (_iacc==NULL)
	{
		return sRet ;
	}	
	if ( !FAILED(_iacc->get_accDescription( childID, &bStr ) ) )
	{
		if ( bStr != NULL )
		{
			sRet = bStr ;
		}
		::SysFreeString( bStr ) ;
	}
	return sRet ;

}
std::list<std::wstring> WINAPI GetFileNameListFromAccCtrl(IAccessible* pIAcc,bool bChild)
{
	DPW((L"-------------------enter------------------------------------"));
	std::list<std::wstring> listRet ;
	std::wstring sCurStr ;
	std::wstring sChildStr ;
	CComPtr<IAccessible> pIACur = pIAcc ;
	sCurStr = GetObjectName( pIACur ) ;
	VARIANT vChild[MAX_PATH] ;
	LONG iChildCount = 0 ;
	for ( INT i = 0 ;i < MAX_PATH ; ++i )
	{
		VariantInit(&vChild[i]) ;
		vChild[i].vt = VT_I4;
		vChild[i].lVal = CHILDID_SELF ;
	} 

	if ( FAILED( AccessibleChildren( pIAcc, 0, MAX_PATH, vChild, &iChildCount ) ) ) 
	{
		return listRet ;
	}
	for ( INT i = 0 ; i< iChildCount ;++i )
	{
		if ( vChild[i].vt != VT_DISPATCH )
		{
			VARIANT role ;
			GetObjectRole( pIAcc,vChild[i],&role) ;
			if( role.lVal == ROLE_SYSTEM_CHECKBUTTON )
			{
				GetObjectState( pIAcc,vChild[i],&role) ;
				if( (STATE_SYSTEM_CHECKED &role.lVal)&& (role.lVal &STATE_SYSTEM_SELECTED))
				{
					std::wstring strDescrption = GetObjectDescription( pIAcc,vChild[i]) ;
					if( strDescrption.length()>0 )
					{
						INT iLen = (INT) wcslen(L"Name: ") ;
						std::wstring fileName =	strDescrption.substr(iLen,strDescrption.find(L",") -iLen) ;
						listRet.push_back(fileName) ;
					}
					
				}
			}
			continue ;
		} else {
			CComPtr<IAccessible> pIAEach = NULL ;
			CComPtr<IDispatch> pIDisp = vChild[i].pdispVal ;
			if ( FAILED( pIDisp->QueryInterface( IID_IAccessible, (void**)&pIAEach) ) ) 
			{
				continue;
			} 
			sChildStr = GetObjectName( pIAEach ) ;
			VARIANT state ;
			GetObjectRole(pIAEach, &state) ;
			GetObjectState(pIAEach,&state) ;
			if( sChildStr.length()>0 && _wcsnicmp(sChildStr.c_str(),sCurStr.c_str(),sChildStr.length()>sCurStr.length()?sChildStr.length():sCurStr.length()) == 0 )
			{
				listRet = GetFileNameListFromAccCtrl( pIAEach, TRUE) ;
				break ;
			}
			if(bChild == TRUE && sChildStr.length() ==0)
			{
				HWND hChild = NULL ;
				if( SUCCEEDED(WindowFromAccessibleObject(pIAEach, &hChild)))
				{
					wchar_t szBuf[MAX_PATH] = {0} ;
					if( hChild!= NULL )
					{
						::GetClassNameW( hChild,szBuf,MAX_PATH) ;
					}
					if( _wcsnicmp( szBuf,L"SysListView32",MAX_PATH ) == 0 )
					{
						listRet = GetFileNameListFromAccCtrl( pIAEach, TRUE) ;
						if( listRet.size()>0 )
						{
							break ;
						}
					}
				}
			}
		}
	}
	return listRet ;
}
std::list<std::wstring> WINAPI  GetFileListFromHandout(HWND hwnd)
{
	std::list<std::wstring> listRet ;
	CoInitialize(NULL) ;
	CComPtr<IAccessible> pIAcc = NULL ;
	
	if ( SUCCEEDED( AccessibleObjectFromWindow( hwnd,OBJID_WINDOW , IID_IAccessible,(void**)&pIAcc ) ) )
	{
		if( pIAcc!=NULL )
		{
			listRet = GetFileNameListFromAccCtrl( pIAcc, FALSE) ;
		}
	}
	CoUninitialize() ;
	return listRet ;
}
PIDLIST_ABSOLUTE WINAPI my_SHBrowseForFolderW(          LPBROWSEINFOW lpbi )
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	PIDLIST_ABSOLUTE pRet = Old_SHBrowseForFolderW( lpbi ) ;
	if(  pRet == NULL )
	{
		return pRet ;
	}
	wchar_t szWndName[MAX_PATH] = {0} ;
	GetWindowTextW( lpbi->hwndOwner, szWndName,MAX_PATH) ;
	if( wcslen( szWndName)>0 )
	{
		if( _wcsnicmp( szWndName, HANDOUT_WND_NAME, wcslen(HANDOUT_WND_NAME)) == 0 )
		{
			std::list<std::wstring> fileList  =GetFileListFromHandout(lpbi->hwndOwner) ;
			std::wstring strTitle = lpbi->lpszTitle ;
			std::transform(strTitle.begin(), strTitle.end(), strTitle.begin(), toupper);
			std::string::size_type start = strTitle.find(HANDOUT_DOWNLOAD_TITLE_PREFIX);
			if( start != std::string::npos )
			{
				std::wstring strSub = strTitle.substr( start + wcslen(HANDOUT_DOWNLOAD_TITLE_PREFIX) ) ;
				start = strSub.find( HANDOUT_DOWNLOAD_TITLE_END) ;
				if( start != std::string::npos )
				{
					strSub = strSub.substr( 0, start ) ;
					start =  strSub.find_last_of( L"." ) ;
					if(  start != std::string::npos )
					{
						strSub = strSub.substr( 0, start ) ;
						wchar_t szFolder[MAX_PATH] = {0} ;
						BOOL bRet= SHGetPathFromIDList( pRet,szFolder ) ;
						if( bRet == TRUE )
						{
							std::wstring strPath = szFolder ;
							if( fileList.size()<=0)
							{
								if( strPath.rfind( L"\\" ) != 0 )
								{
									strPath  += L"\\" +strSub ;
								}
								else
								{
									strPath +=strSub ; 
								}

								DPW((L"Shell Browser for Folder:Display name [%s], ",strPath.c_str())) ;
								//if( !DoAppEvaluate( LME_MAGIC_STRING,L"HANDOUTS", L"SHARE",strPath.c_str())  )
								if( !DoAppEvaluate( strPath.c_str() ,L"HANDOUTS",L"SHARE") )
								{
									pRet = NULL ;
								}
							}
							else
							{
								std::wstring tempPath ;//strPath = szFolder ;
								for( std::list<std::wstring>::iterator itor = fileList.begin() ; itor!= fileList.end() ;++itor )
								{
									if( strPath.rfind( L"\\" ) != 0 )
									{
										tempPath =strPath+ L"\\" +(*itor).c_str() ;
									}
									else
									{
										tempPath = strPath +(*itor).c_str() ; 
									}
									if( !DoAppEvaluate( tempPath.c_str() ,L"HANDOUTS",L"SHARE") )
									{
										pRet = NULL ;
										break ;
									}
								}
							}
							
						}
					}
				}

			}
			else
			{
				DPW((L"Not find")) ;
			}
		}
	}
	DPW((L"Shell Browser for Folder:Display name [%s], Title[%s]",lpbi->pszDisplayName,lpbi->lpszTitle)) ;
	return pRet ;
}
static PIDLIST_ABSOLUTE (WINAPI* Old_SHBrowseForFolderA)(          LPBROWSEINFOA lpbi) ;
PIDLIST_ABSOLUTE WINAPI my_SHBrowseForFolderA(          LPBROWSEINFOA lpbi );
PIDLIST_ABSOLUTE WINAPI New_SHBrowseForFolderA(          LPBROWSEINFOA lpbi )
{
	PIDLIST_ABSOLUTE hr = NULL ;
	if( LMEIsDisabled() == true )
	{

		if( Old_SHBrowseForFolderA )
		{
			hr = Old_SHBrowseForFolderA(lpbi) ;
		}

	}
	__try
	{
		return   my_SHBrowseForFolderA(lpbi) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
PIDLIST_ABSOLUTE WINAPI my_SHBrowseForFolderA(          LPBROWSEINFOA lpbi )
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	DPA(("Shell BrowserA for Folder:Display name [%s], Title[%s]",lpbi->pszDisplayName,lpbi->lpszTitle)) ;
	PIDLIST_ABSOLUTE pRet = Old_SHBrowseForFolderA( lpbi ) ;
	if(  pRet == NULL )
	{
		return pRet ;
	}
	DPA(("Shell Browser for Folder:Display name [%s], Title[%s]",lpbi->pszDisplayName,lpbi->lpszTitle)) ;
	return pRet ;
}
static BOOL (WINAPI* Old_GetOpenFileNameW)( LPOPENFILENAME lpofn ) ;
BOOL WINAPI my_GetOpenFileNameW(  LPOPENFILENAME lpofn ) ;

BOOL WINAPI New_GetOpenFileNameW( LPOPENFILENAME lpofn )
{
	BOOL hr = FALSE ;
	if( LMEIsDisabled() == true )
	{

		if( Old_GetOpenFileNameW )
		{
			hr = Old_GetOpenFileNameW(lpofn) ;
		}

	}
	__try
	{
		return   my_GetOpenFileNameW(lpofn) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
BOOL WINAPI my_GetOpenFileNameW( LPOPENFILENAME lpofn )
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	DPW((L"Get Open file name=======================================")) ;
	BOOL bRet = Old_GetOpenFileNameW(  lpofn ) ;
	if( bRet == FALSE ||lpofn->hwndOwner == NULL)
	{
		return bRet ;
	}
	wchar_t szWndName[MAX_PATH] = {0} ;
	GetWindowTextW( lpofn->hwndOwner, szWndName,MAX_PATH) ;
	if( wcslen( szWndName)>0 )
	{
		if( _wcsnicmp( szWndName, HANDOUT_WND_NAME, wcslen(HANDOUT_WND_NAME)) == 0 )
		{
			if( lpofn->lpstrFile != NULL )
			{
				wchar_t szPath[MAX_PATH] = {0} ;
				::wcsncpy_s( szPath, MAX_PATH, lpofn->lpstrFile, lpofn->nFileOffset ) ;
				szPath[ lpofn->nFileOffset]= '\0' ;
				INT nLen= (INT)wcslen( szPath ) ;
				if( szPath[nLen-1]!= '\\' )
				{
					wcsncat_s( szPath, MAX_PATH, L"\\", _TRUNCATE ) ;
				}
				wchar_t *Pt= lpofn->lpstrFile+lpofn->nFileOffset ;
				std::wstring szFileName ;
				INT iFileCount = 0 ;
				INT iAllowCount = 0 ;
				std::list< std::wstring> allowFileList ;
			//	BOOL bHasDeny = FALSE ;
				while(*Pt)
				{
					szFileName = szPath  ;
					szFileName = szFileName+Pt ;
					bRet = DoAppEvaluate( szFileName.c_str() ,L"HANDOUTS",L"SHARE") ;
					iFileCount ++ ;
					if( bRet == FALSE )
					{
						Pt += wcslen(Pt) +1 ;
						continue ;
					}
					else
					{
						iAllowCount++ ;
						szFileName = Pt ;
						::DPW( (L"Allow Opened file name:%s =========================",szFileName.c_str())) ;
						allowFileList.push_back( szFileName ) ;
					}
					Pt += wcslen(Pt) +1 ;
					
				}
				if( iFileCount != iAllowCount && !allowFileList.empty())
				{
					bRet = TRUE ;
					std::list< std::wstring>::iterator itor =  allowFileList.begin() ;
					Pt= lpofn->lpstrFile+lpofn->nFileOffset ;
					::ZeroMemory( Pt,  (lpofn->nMaxFile - lpofn->nFileOffset) *sizeof(wchar_t ) ) ;
					for( itor;itor!= allowFileList.end(); itor++ )
					{
						::DPW( (L"Push back to the file list:[%s]",(*itor).c_str())) ;
						memcpy( Pt,(*itor).c_str(), (*itor).length() * sizeof(TCHAR) ) ;
						Pt += (*itor).length() ;
						(*Pt) = NULL ;
						Pt += 1 ;
					}
				}
			}
		}
	}
	return bRet ;
}
static UINT (WINAPI* Old_DragQueryFileW)( HDROP hDrop,UINT iFile,LPWSTR lpszFile, UINT cch );
UINT WINAPI my_DragQueryFileW( HDROP hDrop,UINT iFile,LPWSTR lpszFile, UINT cch ) ;
UINT WINAPI New_DragQueryFileW( HDROP hDrop,UINT iFile,LPWSTR lpszFile, UINT cch )
{
	UINT hr = 0 ;
	if( LMEIsDisabled() == true )
	{

		if( Old_DragQueryFileW )
		{
			hr = Old_DragQueryFileW(hDrop, iFile, lpszFile, cch) ;
		}

	}
	__try
	{
		return   my_DragQueryFileW(hDrop, iFile, lpszFile, cch) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
UINT WINAPI my_DragQueryFileW( HDROP hDrop,UINT iFile,LPWSTR lpszFile, UINT cch )
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	DPW((L"Draged query enter")) ;
	UINT uRet = 0 ;
	POINT ptMousePos ;
	uRet = Old_DragQueryFileW(hDrop, iFile, lpszFile, cch) ;
	if(iFile == (UINT)(-1) ||  lpszFile == NULL )
	{
		DPW((L"Draged query exit")) ;
		return uRet ;
	} 
	DWORD dwPt = GetMessagePos() ;
	ptMousePos.x = LOWORD(dwPt) ;
	ptMousePos.y = HIWORD(dwPt) ;
	//::DragQueryPoint( hDrop,&ptMousePos ) ;
	HWND hwnd= WindowFromPoint( ptMousePos ) ;
	wchar_t szClassName[MAX_PATH] = {0} ;
	GetClassName( hwnd, szClassName,MAX_PATH) ;
	DPW((L"Draged query window[%s]",szClassName)) ;
	if( wcslen( szClassName)>0 )
	{
		if( _wcsnicmp( szClassName, HANDOUT_DROP_WND_CLASS, wcslen(HANDOUT_DROP_WND_CLASS)) == 0 )
		{
			HWND hParent = GetParent( hwnd ) ;
			if( hParent != NULL )
			{
				//if( FindWindowExW( hParent, NULL, L"WTL_ScrollBarContainerWindow", NULL ) != NULL )
				//{
				//	DPW((L"Find child window")) ;
				DPW((L"Draged GetParent window")) ;
				wchar_t szWndName[MAX_PATH] = {0} ;
				GetWindowTextW( hParent, szWndName,MAX_PATH) ;
				GetClassName( hParent, szClassName,MAX_PATH) ;

				DPW((L"Draged GetParent window[%s], class name",szWndName,szClassName)) ;
				if( wcslen( szWndName)>0 )
				{
					if( _wcsnicmp( szWndName, HANDOUT_WND_NAME, wcslen(HANDOUT_WND_NAME)) == 0 )
					{
						DPW((L"Draged file path[%s]",lpszFile)) ;
						BOOL bRet = DoAppEvaluate( lpszFile ,L"HANDOUTS",L"SHARE") ;
						if( bRet == FALSE )
						{
							uRet = 0 ;
						}
					}
				}
				//}
			}

		}
	}
	return uRet ;
}
static BOOL (WINAPI* Old_ShowWindow)( HWND hWnd, int nCmdShow);
BOOL WINAPI my_ShowWindow( HWND hWnd, int nCmdShow) ;
BOOL WINAPI try_ShowWindow( HWND hWnd, int nCmdShow) 
{
	BOOL bRet = 0 ;
	if( LMEIsDisabled() == true )
	{

		if( Old_DragQueryFileW )
		{
			bRet = Old_ShowWindow(hWnd, nCmdShow) ;
		}

	}
	__try
	{
		return   my_ShowWindow(hWnd, nCmdShow) ;
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return bRet;
}
BOOL WINAPI my_ShowWindow( HWND hWnd, int nCmdShow)
{
	BOOL bRet = FALSE ;
	if( Old_DragQueryFileW )
	{
		bRet = Old_ShowWindow(hWnd, nCmdShow) ;
	}
	wchar_t szBuf[MAX_PATH] = {0} ;
	GetWindowTextW(hWnd,szBuf,MAX_PATH );

	if( hWnd!= NULL )
	{
		DWORD dProcess = GetCurrentProcessId() ;
		DWORD dTd = ::GetWindowThreadProcessId( hWnd,&dProcess  ) ;
		CMsgHook::GetInstance()->SetMsgHook( dTd );
		
	}
	return TRUE ;
}

/*
* modified the LoadLibraryA or LoadLibraryW to capture the dll's CoCreateInstance function
*
*/

static HMODULE (__stdcall* Old_LoadLibraryA)(_In_ LPCSTR lpLibFileName )=LoadLibraryA;
static HMODULE (__stdcall* Old_LoadLibraryW) (_In_ LPCWSTR lpLibFileName)=LoadLibraryW;

HMODULE __stdcall New_LoadLibraryA(_In_ LPCSTR lpLibFileName)
{
	HMODULE nRet = Old_LoadLibraryA(lpLibFileName);
	WriteLog("loaddll.txt",lpLibFileName);
	return nRet;
}


HMODULE __stdcall New_LoadLibraryW(_In_ LPCWSTR lpLibFileName)
{
	HMODULE hRet = Old_LoadLibraryW(lpLibFileName);
	char strFile[512]="\0";
	sprintf_s(strFile,sizeof(strFile),"%ws",lpLibFileName);
	WriteLog("loaddll.txt",strFile);
	return hRet;
}

typedef struct _tagHOOKPROCINFO
{
	char*	pszModule ;					//The module name include the API .
	char*	pszOrigName ;				//Origin API Name .
	PVOID	pOldProc ;					//Old API Address.
	PVOID	*pNextProc ;					//New API Address .
	PVOID   pNewProc ;					//Callback API Adress
	HMODULE hDllInst ;					//Handle to LoadLibrary(pszModule) .
}HOOKPROCINFO,PHOOKPROCINFO ; 
HOOKPROCINFO myProcInfo[] =
{
	{"ole32.DLL", "CoCreateInstanceEx",   NULL, (PVOID *)&Old_CoCreateInstanceEx,   New_CoCreateInstanceEx,   NULL },
	{"comdlg32.DLL", "GetOpenFileNameW",   NULL, (PVOID *)&Old_GetOpenFileNameW,   New_GetOpenFileNameW,   NULL },
	{"shell32.DLL", "DragQueryFileW",   NULL, (PVOID *)&Old_DragQueryFileW,   New_DragQueryFileW,   NULL },
	{"shell32.DLL", "SHBrowseForFolderW",   NULL, (PVOID *)&Old_SHBrowseForFolderW,   New_SHBrowseForFolderW,   NULL },
	{"shell32.DLL", "SHBrowseForFolderA",   NULL, (PVOID *)&Old_SHBrowseForFolderA,   New_SHBrowseForFolderA,   NULL },
	{"user32.dll", "ShowWindow",   NULL, (PVOID *)&Old_ShowWindow,   try_ShowWindow,   NULL },

};
BOOL  StartHook() 
{
	InitializeMadCHook();
	//gDetourAttach(&(PVOID&)Old_CoCreateInstanceEx, New_CoCreateInstanceEx);
	for(int iCount = 0; iCount < sizeof(myProcInfo) / sizeof(HOOKPROCINFO); iCount++)
	{
		if(myProcInfo[iCount].pNewProc != NULL )
		{
			DPA(("Hook API:[%s]", myProcInfo[iCount].pszOrigName)) ;
			if(!HookAPI(myProcInfo[iCount].pszModule, myProcInfo[iCount].pszOrigName, myProcInfo[iCount].pNewProc, myProcInfo[iCount].pNextProc))
			{
				DPA(("Hook failure:[%s]", myProcInfo[iCount].pszOrigName)) ;
				return FALSE;
			}

		}

	}
	return true ;
}
VOID   EndHook()
{
	FinalizeMadCHook();
}
BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  dwReason,
					  LPVOID lpReserved
					  )
{
	lpReserved ;
	switch( dwReason ) 
	{ 
	case DLL_PROCESS_ATTACH:
		{
		/*	if( nextlabs::application::is_ignored() == true ) {
			return FALSE;
			}*/
			CCommonLog::Initialize();
			InitializeCriticalSection(&g_csPolicyInstance);
			DPA(("=>attach"));

			if( !IsProcess( L"PWConsole.exe" ) )
			{
				return true ;
			}

			StartHook() ;

			CMsgHook::GetInstance()->SetDllInst( hModule );     
			CMsgHook::GetInstance()->SetMsgHook( GetCurrentThreadId() );
		}
		break;

	case DLL_THREAD_ATTACH:
		{
			//CMsgHook::GetInstance()->SetMsgHook( GetCurrentThreadId() );
			// Do thread-specific initialization.
		}
		break;

	case DLL_THREAD_DETACH:
		{
			CMsgHook::GetInstance()->DoUnHookMSG( GetCurrentThreadId() );
		}
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		{
			DeleteCriticalSection(&g_csPolicyInstance);
			if( IsProcess( L"PWConsole.exe" ) )
			{
				EndHook() ;
			}
		}
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;
}


#ifdef _MANAGED
#pragma managed(pop)
#endif

