#include "stdafx.h"
#include "OfficeShare.h"
#include "CollWindows.h"
#include "CollWindow.h"

INSTANCE_DEFINE( CHookedCollWindows );
void CHookedCollWindows::Hook( void* pWindows )
{
    SubstituteOrgFuncWithNew( pWindows, 13, (void*)New_CollWindows_get_Item);
    //SubstituteOrgFuncWithNew( pWindows, 14, (void*)New_CollWindows_get_Count);
   // SubstituteOrgFuncWithNew( pWindows, 15, (void*)New_CollWindows_get_NewEnum);
   // SubstituteOrgFuncWithNew( pWindows, 16, (void*)New_CollWindows_Empty);
    SubstituteOrgFuncWithNew( pWindows, 17, (void*)New_CollWindows_Add );
   // SubstituteOrgFuncWithNew( pWindows, 18, (void*)New_CollWindows_Remove);
  //  SubstituteOrgFuncWithNew( pWindows, 19, (void*)New_CollWindows_Insert);
   // SubstituteOrgFuncWithNew( pWindows, 20, (void*)New_CollWindows_Exists);
    SubstituteOrgFuncWithNew( pWindows, 21, (void*)New_CollWindows_Lookup);
    DoHook( pWindows );
}

HRESULT __stdcall CHookedCollWindows::New_CollWindows_get_Item (
	ICollaborateWindows* pObjects,
	long varParam,
	IBaseObject * * varItem
	)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
         Old_CollWindows_get_Item pFunc = (Old_CollWindows_get_Item)(GetInstance()->GetOrgFunc( (void*)pObjects, New_CollWindows_get_Item ));
		if( pFunc )
		{
			     hr =   pFunc( pObjects, varParam, varItem );
		}

	}
	__try
	{
		return   my_CollWindows_get_Item( pObjects, varParam, varItem );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollWindows::my_CollWindows_get_Item (
	ICollaborateWindows* pObjects,
	long varParam,
	IBaseObject * * varItem
	)
{	nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollWindows_get_Item pFunc = (Old_CollWindows_get_Item)(GetInstance()->GetOrgFunc( (void*)pObjects, New_CollWindows_get_Item ));
    if( !pFunc )
    {
        GetInstance()->Hook( pObjects );
        pFunc = (Old_CollWindows_get_Item)(GetInstance()->GetOrgFunc( (void*)pObjects, New_CollWindows_get_Item ));
    }
    if( pFunc )
    {
        hr = pFunc( pObjects, varParam, varItem );
    }

	if(SUCCEEDED(hr) && (*varItem) != NULL)
	{
		IBaseObject* pObject = (*varItem);
		BSTR bstrType = NULL;
		pObject->get_TypeOf(&bstrType);
		
		if( bstrType && wcscmp(bstrType,L"ICollaborateWindow") == 0)
		{
			//OutputDebugStringA("have some error at windows!\r\n");
		}
	}
	return hr;
}

//HRESULT __stdcall CHookedCollWindows::New_CollWindows_get_Count (ICollaborateWindows* pWindows,
//											/*[out,retval]*/ long * varCount )
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindows_get_Count pFunc = (Old_CollWindows_get_Count)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_get_Count ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindows );
//        pFunc = (Old_CollWindows_get_Count)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_get_Count ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindows, varCount );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindows::New_CollWindows_get_NewEnum (
//	ICollaborateWindows* pWindows,
//	IUnknown * * ppUnkEnum
//	) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindows_get_NewEnum pFunc = (Old_CollWindows_get_NewEnum)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_get_NewEnum ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindows );
//        pFunc = (Old_CollWindows_get_NewEnum)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_get_NewEnum ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindows, ppUnkEnum );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollWindows::New_CollWindows_Empty ( ICollaborateWindows* pWindows) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindows_Empty pFunc = (Old_CollWindows_Empty)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Empty ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindows );
//        pFunc = (Old_CollWindows_Empty)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Empty ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindows );
//    }
//	return hr;
//}


HRESULT __stdcall CHookedCollWindows::New_CollWindows_Add (  
									 ICollaborateWindows* pWindows,
									 struct IBaseObject * pNewItem ) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
        Old_CollWindows_Add pFunc = (Old_CollWindows_Add)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Add ));
		if( pFunc )
		{
			     hr =   pFunc( pWindows, pNewItem );
		}

	}
	__try
	{
		return   my_CollWindows_Add( pWindows, pNewItem );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollWindows::my_CollWindows_Add (  
									 ICollaborateWindows* pWindows,
									 struct IBaseObject * pNewItem ) 
{	nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollWindows_Add pFunc = (Old_CollWindows_Add)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Add ));
    if( !pFunc )
    {
        GetInstance()->Hook( pWindows );
        pFunc = (Old_CollWindows_Add)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Add ));
    }
    if( pFunc )
    {
        hr = pFunc( pWindows, pNewItem );
    }

	//long lcount =0;
	//BSTR strName = NULL;
	//pWindows->get_Count(&lcount);
	//pWindows->get_TypeOf(&strName);

	if(SUCCEEDED(hr) && pNewItem != NULL)
	{
		BSTR bstrType = NULL;

        ICollaborateWindow* pWindow = 0;

        pNewItem->QueryInterface( IID_ICollaborateWindow, (void**)&pWindow );

        if( pWindow )pWindow->get_Title( &bstrType );

		pNewItem->get_TypeOf(&bstrType);       
		
		if(bstrType && wcscmp(bstrType,L"ICollaborateWindow") == 0)
		{
	//		ICollaborateWindow* pWindow = (ICollaborateWindow*)pNewItem;
            CHookedCollWindow::GetInstance()->Hook( PVOID(pNewItem) );
		}
	}
	return hr;
}

//HRESULT __stdcall CHookedCollWindows::New_CollWindows_Remove (ICollaborateWindows* pWindows,
//										 /*[in]*/ VARIANT varKeyOrItem )
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindows_Remove pFunc = (Old_CollWindows_Remove)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Remove ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindows );
//        pFunc = (Old_CollWindows_Remove)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Remove ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindows, varKeyOrItem );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollWindows::New_CollWindows_Insert (
	ICollaborateWindows* pWindows,
	VARIANT varKey,
	IBaseObject * * ppEntity 
	) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
       Old_CollWindows_Insert pFunc = (Old_CollWindows_Insert)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Insert ));
		if( pFunc )
		{
			     hr =   pFunc( pWindows, varKey, ppEntity );
		}

	}
	__try
	{
		return  my_CollWindows_Insert( pWindows, varKey, ppEntity );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollWindows::my_CollWindows_Insert (
	ICollaborateWindows* pWindows,
	VARIANT varKey,
	IBaseObject * * ppEntity 
	) 
{	nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollWindows_Insert pFunc = (Old_CollWindows_Insert)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Insert ));
    if( !pFunc )
    {
        GetInstance()->Hook( pWindows );
        pFunc = (Old_CollWindows_Insert)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Insert ));
    }
    if( pFunc )
    {
        hr = pFunc( pWindows, varKey, ppEntity );
    }

	if(SUCCEEDED(hr) && (*ppEntity) != NULL)
	{
		IBaseObject* pEntity = (*ppEntity);
		BSTR strType=NULL;
		pEntity->get_TypeOf(&strType);
		
		if(strType&&wcscmp(strType,L"ICollaborateWindow") ==0)
		{
            CHookedCollWindow::GetInstance()->Hook( PVOID(pEntity) );
		}
	}
	return hr;
}
//
//HRESULT __stdcall CHookedCollWindows::New_CollWindows_Exists (ICollaborateWindows* pWindows,
//										 /*[in]*/ VARIANT varKey,
//										 /*[out,retval]*/ VARIANT_BOOL * pfExists ) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindows_Exists pFunc = (Old_CollWindows_Exists)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Exists ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindows );
//        pFunc = (Old_CollWindows_Exists)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Exists ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindows, varKey, pfExists );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollWindows::New_CollWindows_Lookup (
	ICollaborateWindows* pWindows,
	VARIANT varKey,
	IBaseObject * * ppEntity
	)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
       Old_CollWindows_Lookup pFunc = (Old_CollWindows_Lookup)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Lookup ));
		if( pFunc )
		{
			     hr =   pFunc( pWindows, varKey, ppEntity );
		}

	}
	__try
	{
		return  my_CollWindows_Lookup( pWindows, varKey, ppEntity );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollWindows::my_CollWindows_Lookup (
	ICollaborateWindows* pWindows,
	VARIANT varKey,
	IBaseObject * * ppEntity
	)
{	 nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;

	char strInfo[512]="\0";
	{
		if(varKey.intVal == 3768)
		{
			BSTR strName = NULL;
			pWindows->get_TypeOf(&strName);
			long lcount = 0;
			pWindows->get_Count(&lcount);
			sprintf_s(strInfo,sizeof(strInfo),"PID eq 3768's Windows name is %ws , count have %d !\r\n",strName,lcount);
			OutPutLog(strInfo);
		}
	}

    Old_CollWindows_Lookup pFunc = (Old_CollWindows_Lookup)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Lookup ));
    if( !pFunc )
    {
        GetInstance()->Hook( pWindows );
        pFunc = (Old_CollWindows_Lookup)(GetInstance()->GetOrgFunc( (void*)pWindows, New_CollWindows_Lookup ));
    }
    if( pFunc )
    {
        hr = pFunc( pWindows, varKey, ppEntity );
    }

	BSTR strType = NULL;
	pWindows->get_TypeOf(&strType);
	sprintf_s(strInfo,sizeof(strInfo),"Lookup's object's name is:%ws!\r\n",strType);

	if(SUCCEEDED(hr) && (*ppEntity) != NULL)
	{
		IBaseObject* pEntity = (*ppEntity);
		BSTR strType_l=NULL;
		pEntity->get_TypeOf(&strType_l);
		
		if(strType&&wcscmp(strType_l,L"ICollaborateWindow") ==0)
		{
            CHookedCollWindow::GetInstance()->Hook( PVOID(pEntity) );
		}
	}
	return hr;
}
//////////////////////////////////////////////////////////////////////////