#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollDesktop.h"
#include "AppShare.h"
#include "PartDB.h"
#include "CollMonitors.h"
#include "CollWindows.h"

INSTANCE_DEFINE( CHookedCollDesktop );

void CHookedCollDesktop::Hook( void* pDesktop )
{
   // SubstituteOrgFuncWithNew( pDesktop, 13, (void*)New_CollDesktop_get_Themes );
    SubstituteOrgFuncWithNew( pDesktop, 23, (void*)New_Desktop_put_Active );
   // SubstituteOrgFuncWithNew( pDesktop, 24, (void*)New_Desktop_get_Monitors );
    SubstituteOrgFuncWithNew( pDesktop, 25, (void*)New_Desktop_get_Windows );
    SubstituteOrgFuncWithNew( pDesktop, 26, (void*)New_Desktop_get_CollaborateApp );
    //SubstituteOrgFuncWithNew( pDesktop, 27, (void*)New_Desktop_get_ShareProcess );
    SubstituteOrgFuncWithNew( pDesktop, 28, (void*)New_Desktop_put_ShareProcess );
   // SubstituteOrgFuncWithNew( pDesktop, 31, (void*)New_Desktop_put_ProcessId );
    //SubstituteOrgFuncWithNew( pDesktop, 33, (void*)New_Desktop_put_PrimaryWindow );
    DoHook( pDesktop );
}

//ICollaborateDesktop
//map_Desktop_Func g_map_DesktopFunc;
//Mutex			g_Mutex_DesktopFunc;
//////////////////////////////////////////////////////////////////////////
#if 1
//HRESULT __stdcall CHookedCollDesktop::New_CollDesktop_get_Themes(ICollaborateDesktop* pDesktop,
//											 /*[out, retval]*/ SAFEARRAY** varThemes)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollDesktop_get_Themes pFunc = (Old_CollDesktop_get_Themes)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_CollDesktop_get_Themes ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pDesktop );
//        pFunc = (Old_CollDesktop_get_Themes)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_CollDesktop_get_Themes ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pDesktop,varThemes);
//    }	
//
//	return hr;
//}

HRESULT __stdcall CHookedCollDesktop::New_Desktop_put_Active (ICollaborateDesktop* pDesktop,
										  /*[in]*/ VARIANT_BOOL varActive )
{
	
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_Desktop_put_Active pFunc = (Old_Desktop_put_Active)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_Active ));
		if( pFunc )
		{
			return pFunc(pDesktop,varActive);
		}

	}
	__try
	{
		return 	my_Desktop_put_Active(pDesktop,varActive);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW((L"New_Desktop_put_Active exception"));
	}
	return hr;
}
HRESULT __stdcall CHookedCollDesktop::my_Desktop_put_Active (ICollaborateDesktop* pDesktop,
										  /*[in]*/ VARIANT_BOOL varActive )
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
    HRESULT hr = E_NOTIMPL;
    Old_Desktop_put_Active pFunc = (Old_Desktop_put_Active)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_Active ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDesktop );
        pFunc = (Old_Desktop_put_Active)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_Active ));
    }

    if( varActive && CProcessDB::GetInstance()!= NULL ) 
    {
        const TCHAR* pName = CProcessDB::GetInstance()->GetName();
        DWORD nStrLen = CProcessDB::GetInstance()->GetNameLen();
        bool bAllow = false;

        std::wstring strInfo( pName );
        //strInfo += TEXT("\n\n");
        //strInfo += CPartDB::GetInstance()->GetPresenterAttendeeInfo();

        if( nStrLen )
        {
            if( _wcsicmp( GetNameWithoutPath( strInfo.c_str(), (DWORD)strInfo.length()), L"PWConsole.exe" ) == 0 )
            {
				//CE_ACTION_WM_SHARE
                bAllow = DoAppEvaluate( LME_MAGIC_STRING,TEXT( "DESKTOP" ),L"SHARE" );
            }
            else
            {
                bAllow = DoAppEvaluate( strInfo.c_str() ,L"APPLICATION",L"SHARE");//MsgBoxAllowOrDeny( strInfo.c_str(), L"Allow to share following process ?" );
            }
        }
        else
        {
            bAllow = DoAppEvaluate( LME_MAGIC_STRING,TEXT( "DESKTOP" ),L"SHARE" );//"[LM_LocalDeskTop]" ) );//MsgBoxAllowOrDeny( strInfo.c_str(), L"Allow to share Desktop ?" );
        }
        
        CProcessDB::GetInstance()->Reset();

        if( !bAllow )
        {
            pFunc = 0;
        }
    }

    if( pFunc )
    {
        hr = pFunc(pDesktop,varActive);
    }	

    return hr;
}
#endif

HRESULT __stdcall CHookedCollDesktop::New_Desktop_get_Monitors (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct ICollaborateMonitors * * varMonitors )
{

	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		 Old_Desktop_get_Monitors pFunc = (Old_Desktop_get_Monitors)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Monitors ));
		if( pFunc )
		{
			return  pFunc( pDesktop,varMonitors );
		}

	}
	__try
	{
		return 	 my_Desktop_get_Monitors( pDesktop,varMonitors );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW((L"New_Desktop_get_Monitors exception"));
	}
	return hr;
}
HRESULT __stdcall CHookedCollDesktop::my_Desktop_get_Monitors (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct ICollaborateMonitors * * varMonitors )
{
//	nextlabs::recursion_control_auto auto_disable(hook_control);
    HRESULT hr = E_NOTIMPL;
    Old_Desktop_get_Monitors pFunc = (Old_Desktop_get_Monitors)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Monitors ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDesktop );
        pFunc = (Old_Desktop_get_Monitors)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Monitors ));
    }

    if( pFunc )
    {
        hr = pFunc( pDesktop,varMonitors );
    }	

	if(SUCCEEDED(hr) && *varMonitors != NULL)
	{
		ICollaborateMonitors* pMonitors = (ICollaborateMonitors*)(*varMonitors);
        CHookedCollMonitors::GetInstance()->Hook( PVOID( pMonitors ) );
	}
	return hr;
}



HRESULT __stdcall CHookedCollDesktop::New_Desktop_get_Windows (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct ICollaborateWindows * * varWindows )
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		 Old_Desktop_get_Windows pFunc = (Old_Desktop_get_Windows)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Windows ));
		if( pFunc )
		{
			return  pFunc( pDesktop,varWindows );
		}

	}
	__try
	{
		return 	my_Desktop_get_Windows( pDesktop,varWindows );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}

HRESULT __stdcall CHookedCollDesktop::my_Desktop_get_Windows (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct ICollaborateWindows * * varWindows )
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_Desktop_get_Windows pFunc = (Old_Desktop_get_Windows)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Windows ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDesktop );
        pFunc = (Old_Desktop_get_Windows)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_Windows ));
    }

    if( pFunc )
    {
        hr = pFunc( pDesktop,varWindows );
    }	

	if(SUCCEEDED(hr)&&(*varWindows) != NULL)
	{
		ICollaborateWindows* pWindows = (*varWindows);
		BSTR strType = NULL;
		pWindows->get_TypeOf(&strType);
		if(strType&&wcscmp(strType,L"ICollaborateWindows") == 0 )
        {
            CHookedCollWindows::GetInstance()->Hook( PVOID(pWindows) );
        }
		//hr = S_OK;
	}
	return hr;
}

 

HRESULT __stdcall CHookedCollDesktop::New_Desktop_get_CollaborateApp (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct IBaseObject * * varCollaborateApp )
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		  Old_Desktop_get_CollaborateApp pFunc = (Old_Desktop_get_CollaborateApp)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_CollaborateApp ));
		if( pFunc )
		{
			return pFunc( pDesktop,varCollaborateApp );
		}

	}
	__try
	{
		return 	my_Desktop_get_CollaborateApp( pDesktop,varCollaborateApp );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW((L"New_Desktop_get_CollaborateApp exception"));
	}
	return hr;
}

HRESULT __stdcall CHookedCollDesktop::my_Desktop_get_CollaborateApp (ICollaborateDesktop* pDesktop,
/*[out,retval]*/ struct IBaseObject * * varCollaborateApp )
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_Desktop_get_CollaborateApp pFunc = (Old_Desktop_get_CollaborateApp)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_CollaborateApp ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDesktop );
		pFunc = (Old_Desktop_get_CollaborateApp)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_CollaborateApp ));
	}
	
    if( pFunc )
    {
        hr = pFunc( pDesktop,varCollaborateApp );
    }	

	if(SUCCEEDED(hr) && (*varCollaborateApp) != NULL)
	{
		IBaseObject* pApp = (*varCollaborateApp);
		// hook coll app
		BSTR strAppType = NULL;
		pApp->get_TypeOf(&strAppType);
		if(strAppType&&wcscmp(strAppType,L"ICollaborateAppShare")==0)
		{
//			ICollaborateAppShare* pAppShare = (ICollaborateAppShare*)(pApp);
            CHookedCollAppShare::GetInstance()->Hook( PVOID(pApp) );
		}
		//hr = S_OK;
	}
	return hr;
}
HRESULT __stdcall CHookedCollDesktop::New_Desktop_put_ShareProcess (ICollaborateDesktop* pDesktop,
												/*[in]*/ unsigned long varParam,
												/*[out,retval]*/ VARIANT_BOOL varShareProcess )
{

	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_Desktop_put_ShareProcess pFunc = (Old_Desktop_put_ShareProcess)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_ShareProcess ));
		if( pFunc )
		{
			return pFunc( pDesktop, varParam, varShareProcess );
		}

	}
	__try
	{
		return 	my_Desktop_put_ShareProcess( pDesktop, varParam, varShareProcess );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW((L"New_Desktop_get_CollaborateApp exception"));
	}
	return hr;
}
HRESULT __stdcall CHookedCollDesktop::my_Desktop_put_ShareProcess (ICollaborateDesktop* pDesktop,
												/*[in]*/ unsigned long varParam,
												/*[out,retval]*/ VARIANT_BOOL varShareProcess )
{
	HRESULT hr = E_NOTIMPL;

    Old_Desktop_put_ShareProcess pFunc = (Old_Desktop_put_ShareProcess)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_ShareProcess ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDesktop );
        pFunc = (Old_Desktop_put_ShareProcess)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_ShareProcess ));
    }
	ULONG lProcessID = varParam ;
	pDesktop->get_ProcessId( &lProcessID) ;
	BOOL bAllow  = TRUE ;
	if( lProcessID == 0 )
	{
		bAllow = DoAppEvaluate( LME_MAGIC_STRING,TEXT( "DESKTOP" ),L"SHARE" );
	}
	if( !bAllow )
	{
		pFunc = 0;
	}
    if( pFunc )
    {
		DPW((L"New_Desktop_put_ShareProcess3333333333333333333333333333333333333")) ;
        hr = pFunc( pDesktop, varParam, varShareProcess );
    }	
	return hr;
}

//HRESULT __stdcall CHookedCollDesktop::New_Desktop_get_ShareProcess (ICollaborateDesktop* pDesktop,
//												/*[in]*/ unsigned long varParam,
//												/*[out,retval]*/ VARIANT_BOOL * varShareProcess )
//{
//	HRESULT hr = E_NOTIMPL;
//
//    Old_Desktop_get_ShareProcess pFunc = (Old_Desktop_get_ShareProcess)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_ShareProcess ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pDesktop );
//        pFunc = (Old_Desktop_get_ShareProcess)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_get_ShareProcess ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pDesktop, varParam, varShareProcess );
//    }
//
//	return hr;
//}
#if 1
//HRESULT __stdcall CHookedCollDesktop::New_Desktop_put_ProcessId(ICollaborateDesktop* pDesktop,
//											/*[in]*/ unsigned long varProcessId)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_Desktop_put_ProcessId pFunc = (Old_Desktop_put_ProcessId)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_ProcessId ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pDesktop );
//        pFunc = (Old_Desktop_put_ProcessId)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_ProcessId ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pDesktop, varProcessId );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollDesktop::New_Desktop_put_PrimaryWindow (ICollaborateDesktop* pDesktop,
//												 /*[in]*/ long varPrimaryWindow ) 
//{
//	HRESULT hr = E_NOTIMPL;
//
//    Old_Desktop_put_PrimaryWindow pFunc = (Old_Desktop_put_PrimaryWindow)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_PrimaryWindow ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pDesktop );
//        pFunc = (Old_Desktop_put_PrimaryWindow)(GetInstance()->GetOrgFunc( (void*)pDesktop, New_Desktop_put_PrimaryWindow ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pDesktop, varPrimaryWindow );
//    }
//	return hr;
//}

#endif 
//////////////////////////////////////////////////////////////////////////
