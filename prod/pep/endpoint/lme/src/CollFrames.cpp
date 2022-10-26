#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollFrames.h"
#include "CollWindow.h"


//////////////////////////////////////////////////////////////////////////
// Frames
INSTANCE_DEFINE( CHookedCollFrames );

void CHookedCollFrames::Hook( void* pFrames )
{
    SubstituteOrgFuncWithNew( pFrames, 13, (void*)New_CollFrames_get_Item);
    //SubstituteOrgFuncWithNew( pFrames, 14, (void*)New_CollFrames_get_Count);
   // SubstituteOrgFuncWithNew( pFrames, 15, (void*)New_CollFrames_get_NewEnum);
   // SubstituteOrgFuncWithNew( pFrames, 16, (void*)New_CollFrames_Empty);
    SubstituteOrgFuncWithNew( pFrames, 17, (void*)New_CollFrames_Add);
   // SubstituteOrgFuncWithNew( pFrames, 18, (void*)New_CollFrames_Remove);
   // SubstituteOrgFuncWithNew( pFrames, 19, (void*)New_CollFrames_Insert);
  //  SubstituteOrgFuncWithNew( pFrames, 20, (void*)New_CollFrames_Exists);
    SubstituteOrgFuncWithNew( pFrames, 21, (void*)New_CollFrames_Lookup);
    DoHook( pFrames );
}

HRESULT __stdcall CHookedCollFrames::New_CollFrames_get_Item (
											ICollaborateFrames* pObject,
											long varParam,
											struct IBaseObject * * varItem
											)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollFrames_get_Item pFunc = (Old_CollFrames_get_Item)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_Item ));
		if( pFunc )
		{
			return   pFunc(pObject,varParam, varItem);
		}

	}
	__try
	{
		return 	  my_CollFrames_get_Item(pObject,varParam, varItem);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollFrames::my_CollFrames_get_Item (
	ICollaborateFrames* pObject,
	long varParam,
struct IBaseObject * * varItem
	)
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollFrames_get_Item pFunc = (Old_CollFrames_get_Item)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_Item ));
    if( !pFunc )
    {
        GetInstance()->Hook( pObject );
        pFunc = (Old_CollFrames_get_Item)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_Item ));
    }

    if( pFunc )
    {
        hr = pFunc(pObject,varParam, varItem);
    }	

	if(SUCCEEDED(hr) && (*varItem) != NULL)
	{
		IBaseObject* pObject_l = (*varItem);
		BSTR bstrType = NULL;
		pObject_l->get_TypeOf(&bstrType);
		
		if( bstrType && wcscmp(bstrType,L"ICollaborateWindow") == 0)
		{
			//HookWindow((ICollaborateWindow*)pObject);
            CHookedCollWindow::GetInstance()->Hook( PVOID(pObject_l) );
		}
		//hr = S_OK;
	}
	return hr;
}
//HRESULT __stdcall CHookedCollFrames::New_CollFrames_get_Count (ICollaborateFrames* pObject,
//											/*[out,retval]*/ long * varCount )
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrames_get_Count pFunc = (Old_CollFrames_get_Count)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_Count ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pObject );
//        pFunc = (Old_CollFrames_get_Count)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_Count ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pObject,varCount );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollFrames::New_CollFrames_get_NewEnum (
//											  ICollaborateFrames* pObject,
//											  IUnknown * * ppUnkEnum
//											  ) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrames_get_NewEnum pFunc = (Old_CollFrames_get_NewEnum)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_NewEnum ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pObject );
//        pFunc = (Old_CollFrames_get_NewEnum)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_get_NewEnum ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pObject,ppUnkEnum );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollFrames::New_CollFrames_Empty ( ICollaborateFrames* pObject) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrames_Empty pFunc = (Old_CollFrames_Empty)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Empty ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pObject );
//        pFunc = (Old_CollFrames_Empty)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Empty ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pObject );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollFrames::New_CollFrames_Add (
									  ICollaborateFrames* pObject,
									  IBaseObject * pNewItem ) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollFrames_Add pFunc = (Old_CollFrames_Add)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Add ));
		if( pFunc )
		{
			return  pFunc( pObject, pNewItem );
		}

	}
	__try
	{
		return 	my_CollFrames_Add( pObject, pNewItem );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollFrames::my_CollFrames_Add (
	ICollaborateFrames* pObject,
	IBaseObject * pNewItem ) 
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollFrames_Add pFunc = (Old_CollFrames_Add)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Add ));
    if( !pFunc )
    {
        GetInstance()->Hook( pObject );
        pFunc = (Old_CollFrames_Add)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Add ));
    }

    if( pFunc )
    {
        hr = pFunc( pObject, pNewItem );
    }

	long lcount =0;
	BSTR strName = NULL;
	hr = pObject->get_Count(&lcount);
	hr = pObject->get_TypeOf(&strName);

	if(SUCCEEDED(hr) && pNewItem != NULL)
	{

		BSTR bstrType = NULL;
		hr = pNewItem->get_TypeOf(&bstrType);
		
		if( bstrType && wcscmp(bstrType,L"ICollaborateWindow") == 0)
		{
			ICollaborateWindow* pWindow = (ICollaborateWindow*)pNewItem;
			BSTR strTitle=NULL;
			hr = pWindow->get_Title(&strTitle);
			BSTR strDirectory=NULL;
			hr = pWindow->get_WorkingDirectory(&strDirectory);
            CHookedCollWindow::GetInstance()->Hook( PVOID(pNewItem) );
            //HookWindow((ICollaborateWindow*)pNewItem);
		}
		/*else if( bstrType && wcscmp(bstrType,L"ICollaborateFrame")==0)
		{
            CHookedCollFrame::GetInstance()->Hook( PVOID( pNewItem ) );
		}*/
        else
        {
            hr = S_OK;
        }
		hr = S_OK;
	}
	return hr;

}

//HRESULT __stdcall CHookedCollFrames::New_CollFrames_Remove (ICollaborateFrames* pObject,
//										 /*[in]*/ VARIANT varKeyOrItem )
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrames_Remove pFunc = (Old_CollFrames_Remove)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Remove ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pObject );
//        pFunc = (Old_CollFrames_Remove)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Remove ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pObject, varKeyOrItem );
//    }
//
//	return hr;
//}

HRESULT __stdcall CHookedCollFrames::New_CollFrames_Insert (
	ICollaborateFrames* pObject,
	VARIANT varKey,
	IBaseObject * * ppEntity 
	) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollFrames_Insert pFunc = (Old_CollFrames_Insert)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Insert ));
		if( pFunc )
		{
			return  pFunc( pObject, varKey, ppEntity );
		}

	}
	__try
	{
		return  my_CollFrames_Insert( pObject, varKey, ppEntity );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollFrames::my_CollFrames_Insert (
	ICollaborateFrames* pObject,
	VARIANT varKey,
	IBaseObject * * ppEntity 
	) 
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollFrames_Insert pFunc = (Old_CollFrames_Insert)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Insert ));
    if( !pFunc )
    {
        GetInstance()->Hook( pObject );
        pFunc = (Old_CollFrames_Insert)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Insert ));
    }

    if( pFunc )
    {
        hr = pFunc( pObject, varKey, ppEntity );
    }

	if(SUCCEEDED(hr) && (*ppEntity) != NULL)
	{
		IBaseObject* pEntity = (*ppEntity);
		BSTR strType=NULL;
		hr = pEntity->get_TypeOf(&strType);
		
		if(strType&&wcscmp(strType,L"ICollaborateWindow") ==0)
		{
			//HookWindow((ICollaborateWindow*)pEntity);
            CHookedCollWindow::GetInstance()->Hook( PVOID(pEntity) );
		}
		hr = S_OK;
	}
	return hr;
}

//HRESULT __stdcall CHookedCollFrames::New_CollFrames_Exists (ICollaborateFrames* pObject,
//										 /*[in]*/ VARIANT varKey,
//										 /*[out,retval]*/ VARIANT_BOOL * pfExists ) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrames_Exists pFunc = (Old_CollFrames_Exists)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Exists ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pObject );
//        pFunc = (Old_CollFrames_Exists)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Exists ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pObject, varKey, pfExists );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollFrames::New_CollFrames_Lookup (
	ICollaborateFrames* pObject,
	VARIANT varKey,
	IBaseObject * * ppEntity
	)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollFrames_Lookup pFunc = (Old_CollFrames_Lookup)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Lookup ));
		if( pFunc )
		{
			return   pFunc( pObject, varKey, ppEntity );
		}

	}
	__try
	{
		return my_CollFrames_Lookup( pObject, varKey, ppEntity );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollFrames::my_CollFrames_Lookup (
	ICollaborateFrames* pObject,
	VARIANT varKey,
	IBaseObject * * ppEntity
	)
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
    Old_CollFrames_Lookup pFunc = (Old_CollFrames_Lookup)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Lookup ));
    if( !pFunc )
    {
        GetInstance()->Hook( pObject );
        pFunc = (Old_CollFrames_Lookup)(GetInstance()->GetOrgFunc( (void*)pObject, New_CollFrames_Lookup ));
    }

    if( pFunc )
    {
        hr = pFunc( pObject, varKey, ppEntity );
    }

	if(SUCCEEDED(hr) && (*ppEntity) != NULL)
	{
		IBaseObject* pEntity = (*ppEntity);
		BSTR strType=NULL;
		hr = pEntity->get_TypeOf(&strType);
		
		if(strType&&wcscmp(strType,L"ICollaborateWindow") ==0)
		{
            CHookedCollWindow::GetInstance()->Hook( PVOID(pEntity) );
		}
	}
	return hr;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// hook frame
//INSTANCE_DEFINE( CHookedCollFrame );
//
//void CHookedCollFrame::Hook( void* pFrame )
//{
//    SubstituteOrgFuncWithNew( pFrame, 13, (void*)Hooked_get_Handle);
//    SubstituteOrgFuncWithNew( pFrame, 14, (void*)Hooked_get_IsSnapshot);
//    SubstituteOrgFuncWithNew( pFrame, 15, (void*)New_CollFrame_Open);
//    SubstituteOrgFuncWithNew( pFrame, 16, (void*)New_CollFrame_Close);
//    DoHook( pFrame );
//}
//HRESULT __stdcall CHookedCollFrame::New_CollFrame_Open(ICollaborateFrame* pFrame,VARIANT_BOOL fSnapshot)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollFrame_Open pFunc = (Old_CollFrame_Open)(GetInstance()->GetOrgFunc( (void*)pFrame, New_CollFrame_Open ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pFrame );
//        pFunc = (Old_CollFrame_Open)(GetInstance()->GetOrgFunc( (void*)pFrame, New_CollFrame_Open ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pFrame, fSnapshot );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollFrame::New_CollFrame_Close(ICollaborateFrame* pFrame)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollFrame_Close pFunc = (Old_CollFrame_Close)(GetInstance()->GetOrgFunc( (void*)pFrame, New_CollFrame_Close ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pFrame );
//        pFunc = (Old_CollFrame_Close)(GetInstance()->GetOrgFunc( (void*)pFrame, New_CollFrame_Close ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pFrame );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollFrame::Hooked_get_Handle (ICollaborateFrame* pFrame,
//                                     /*[out,retval]*/ long * varHandle ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_get_Handle pFunc = (Func_get_Handle)(GetInstance()->GetOrgFunc( (void*)pFrame, Hooked_get_Handle ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pFrame );
//        pFunc = (Func_get_Handle)(GetInstance()->GetOrgFunc( (void*)pFrame, Hooked_get_Handle ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pFrame, varHandle );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedCollFrame::Hooked_get_IsSnapshot (ICollaborateFrame* pFrame,
//                                         /*[out,retval]*/ VARIANT_BOOL * varIsSnapshot )
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_get_IsSnapshot pFunc = (Func_get_IsSnapshot)(GetInstance()->GetOrgFunc( (void*)pFrame, Hooked_get_IsSnapshot ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pFrame );
//        pFunc = (Func_get_IsSnapshot)(GetInstance()->GetOrgFunc( (void*)pFrame, Hooked_get_IsSnapshot ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pFrame, varIsSnapshot );
//    }
//    return hr;
//}
//////////////////////////////////////////////////////////////////////////
