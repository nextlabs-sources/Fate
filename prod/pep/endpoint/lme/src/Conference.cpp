#include "stdafx.h"
#include "Coference.h"
#include "PartDb.h"

//////////////////////////////////////////////////////////////////////////
//HRESULT __stdcall CHookedConfCenter::New_QueryInterface(
//									 IUnknown* This,
//									 const IID & riid,
//									 void **ppvObj
//									 )
//{
//    HRESULT hr = E_NOTIMPL;
//    FUNC_IUNKNOWN_QUERYINTERFACE pFunc = (FUNC_IUNKNOWN_QUERYINTERFACE)(GetInstance()->GetOrgFunc( (void*)This, New_QueryInterface ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( This );
//        pFunc = (FUNC_IUNKNOWN_QUERYINTERFACE)(GetInstance()->GetOrgFunc( (void*)This, New_QueryInterface ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(This,riid,ppvObj );
//    }	
//	return hr;
//}

//HRESULT __stdcall CHookedConfCenter::New_Invoke(
//							 IDispatch* This,
//							 long dispidMember, 
//							 GUID* riid, 
//							 unsigned long lcid, 
//							 unsigned short wFlags, 
//							 DISPPARAMS* pdispparams, 
//							 VARIANT* pvarResult, 
//							 EXCEPINFO* pexcepinfo, 
//							 unsigned int* puArgErr)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_Invoke pFunc = (Old_Invoke)(GetInstance()->GetOrgFunc( (void*)This, New_Invoke ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( This );
//        pFunc = (Old_Invoke)(GetInstance()->GetOrgFunc( (void*)This, New_Invoke ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(This, dispidMember, riid,lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr  );
//    }
//
//	return S_OK;
//}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

// start at here
INSTANCE_DEFINE(CHookedConfCenter);
void CHookedConfCenter::Hook( void* pConfCenter )
{    
    //SubstituteOrgFuncWithNew( pConfCenter, 0, (void*)New_QueryInterface);
    //SubstituteOrgFuncWithNew( pConfCenter, 6, (void*)New_Invoke);
    //SubstituteOrgFuncWithNew( pConfCenter, 7, (void*)New_SetClientName);
    //SubstituteOrgFuncWithNew( pConfCenter, 8, (void*)New_ShowConfigUI);
    //SubstituteOrgFuncWithNew( pConfCenter, 9, (void*)New_GetCombinedCapabilities);
    //SubstituteOrgFuncWithNew( pConfCenter, 10, (void*)New_GetDefaultProvider);
    //SubstituteOrgFuncWithNew( pConfCenter, 11, (void*)New_GetProviderByID);
    //SubstituteOrgFuncWithNew( pConfCenter, 12, (void*)New_GetProviders);
    //SubstituteOrgFuncWithNew( pConfCenter, 13, (void*)New_GetConference);
    //SubstituteOrgFuncWithNew( pConfCenter, 14, (void*)New_CreateConferenceFromXML);
    SubstituteOrgFuncWithNew( pConfCenter, 15, (void*)New_SendInvitation);
    //SubstituteOrgFuncWithNew( pConfCenter, 16, (void*)New_Schedule);
    //SubstituteOrgFuncWithNew( pConfCenter, 17, (void*)New_Initialize);
    //SubstituteOrgFuncWithNew( pConfCenter, 18, (void*)New_ShutDown);
    //SubstituteOrgFuncWithNew( pConfCenter, 19, (void*)New_SetPreferredUILanguage);
    DoHook( pConfCenter );
}

//HRESULT __stdcall CHookedConfCenter::New_SetClientName (
//									 IConferencingCenter* pThis,
//									 BSTR bstrApplicationName )
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_SetClientName pFunc = (Old_SetClientName)(GetInstance()->GetOrgFunc( (void*)pThis, New_SetClientName ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_SetClientName)(GetInstance()->GetOrgFunc( (void*)pThis, New_SetClientName ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis,bstrApplicationName);
//    }	
//
//    return hr;
//}

//HRESULT __stdcall CHookedConfCenter::New_ShowConfigUI ( IConferencingCenter* pThis) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_ShowConfigUI pFunc = (Old_ShowConfigUI)(GetInstance()->GetOrgFunc( (void*)pThis, New_ShowConfigUI ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_ShowConfigUI)(GetInstance()->GetOrgFunc( (void*)pThis, New_ShowConfigUI ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis);
//    }	
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_GetCombinedCapabilities (
//											IConferencingCenter* pThis,
//											enum ConfCapabilities * pCapabilities ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_GetCombinedCapabilities pFunc = (Old_GetCombinedCapabilities)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetCombinedCapabilities ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_GetCombinedCapabilities)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetCombinedCapabilities ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, pCapabilities);
//    }	
//
//    return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_GetDefaultProvider (
//										IConferencingCenter* pThis,
//										enum ConfCapabilities capability,
//										struct IConferencingProvider * * ppProvider ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_GetDefaultProvider pFunc = (Old_GetDefaultProvider)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetDefaultProvider ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_GetDefaultProvider)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetDefaultProvider ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, capability, ppProvider );
//    }	
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_GetProviderByID (
//									   IConferencingCenter* pThis,
//									   /*[in]*/ BSTR bstrProviderID,
//										struct IConferencingProvider * * ppProvider ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_GetProviderByID pFunc = (Old_GetProviderByID)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetProviderByID ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_GetProviderByID)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetProviderByID ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, bstrProviderID, ppProvider );
//    }	
//
//	//////////////////////////////////////////////////////////////////////////
//	// hook on provider
//	//if(SUCCEEDED(hr) && (*ppProvider) != NULL)
//	//{
//	//	IConferencingProvider* pProvider = (*ppProvider);
//	//	BSTR bstrId=NULL;
//	//	HRESULT hr = pProvider->get_ID(&bstrId);
// //       CHookedConfProvider::GetInstance()->Hook( PVOID(pProvider) );// HookConferencingProvider(pProvider);
//	//}
//	//////////////////////////////////////////////////////////////////////////
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_GetProviders (
//									IConferencingCenter* pThis,
//									struct IConferencingProviders * * pProviders ) 
//{
//    OutputDebugString( TEXT("CHookedConfCenter::New_GetProviders") );
//    HRESULT hr = E_NOTIMPL;
//    Old_GetProviders pFunc = (Old_GetProviders)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetProviders ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_GetProviders)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetProviders ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, pProviders );
//    }
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_GetConference (
//									 /*[in]*/IConferencingCenter* pThis,
//									 /*[in]*/ BSTR bstrProviderID,
//									 /*[in]*/ BSTR bstrConferenceID,
//									/*[out,retval]*/ struct IConference * * ppConference ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_GetConference pFunc = (Old_GetConference)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetConference ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_GetConference)(GetInstance()->GetOrgFunc( (void*)pThis, New_GetConference ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, bstrProviderID, bstrConferenceID, ppConference );
//    }
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_CreateConferenceFromXML (
//												IConferencingCenter* pThis,
//												/*[in]*/ BSTR bstrConferenceDataXML,
//												/*[out,retval]*/ struct IConference * * ppConference ) 
//{	
//    HRESULT hr = E_NOTIMPL;
//    Old_CreateConferenceFromXML pFunc = (Old_CreateConferenceFromXML)(GetInstance()->GetOrgFunc( (void*)pThis, New_CreateConferenceFromXML ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_CreateConferenceFromXML)(GetInstance()->GetOrgFunc( (void*)pThis, New_CreateConferenceFromXML ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, bstrConferenceDataXML, ppConference );
//    }
//
//	return hr;
//}

HRESULT __stdcall CHookedConfCenter::New_SendInvitation (
									  IConferencingCenter* pThis,
									  /*[in]*/ struct IConference * pConference,
										/*[in]*/ enum ConfInvitationType type,
									/*[in]*/ enum TextFormat format )
{	
	
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_SendInvitation pFunc = (Old_SendInvitation)(GetInstance()->GetOrgFunc( (void*)pThis, New_SendInvitation ));
		if( pFunc )
		{
			hr = pFunc(pThis, pConference, type, format );
		}

	}
	__try
	{
		return   my_SendInvitation(pThis, pConference, type, format );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedConfCenter::my_SendInvitation (
	IConferencingCenter* pThis,
/*[in]*/ struct IConference * pConference,
	/*[in]*/ enum ConfInvitationType type,
	/*[in]*/ enum TextFormat format )
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
	Old_SendInvitation pFunc = (Old_SendInvitation)(GetInstance()->GetOrgFunc( (void*)pThis, New_SendInvitation ));
	if( !pFunc )
	{
		GetInstance()->Hook( pThis );
		pFunc = (Old_SendInvitation)(GetInstance()->GetOrgFunc( (void*)pThis, New_SendInvitation ));
	}

    if( !DoEvaluate( LME_MAGIC_STRING,L"MEETING" ) )//!MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to Invite?" )) 
    {                        
        return hr;
    }

    if( pFunc )
    {
        hr = pFunc(pThis, pConference, type, format );
    }
	return hr;
}

//HRESULT __stdcall CHookedConfCenter::New_Schedule (
//								IConferencingCenter* pThis,
//								/*[in]*/ struct IConference * pConference,
//								/*[in]*/ enum TextFormat format )
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_Schedule pFunc = (Old_Schedule)(GetInstance()->GetOrgFunc( (void*)pThis, New_Schedule ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_Schedule)(GetInstance()->GetOrgFunc( (void*)pThis, New_Schedule ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, pConference, format );
//    }
//
//	return hr;
//}
//HRESULT __stdcall CHookedConfCenter::New_Initialize ( IConferencingCenter* pThis)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_Initialize pFunc = (Old_Initialize)(GetInstance()->GetOrgFunc( (void*)pThis, New_Initialize ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_Initialize)(GetInstance()->GetOrgFunc( (void*)pThis, New_Initialize ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedConfCenter::New_ShutDown ( IConferencingCenter* pThis)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_ShutDown pFunc = (Old_Initialize)(GetInstance()->GetOrgFunc( (void*)pThis, New_ShutDown ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_Initialize)(GetInstance()->GetOrgFunc( (void*)pThis, New_ShutDown ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedConfCenter::New_SetPreferredUILanguage (
//	IConferencingCenter* pThis,
//	unsigned short langid ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_SetPreferredUILanguage pFunc = (Old_SetPreferredUILanguage)(GetInstance()->GetOrgFunc( (void*)pThis, New_SetPreferredUILanguage ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pThis );
//        pFunc = (Old_SetPreferredUILanguage)(GetInstance()->GetOrgFunc( (void*)pThis, New_SetPreferredUILanguage ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pThis, langid );
//    }
//
//	return hr;
//}

