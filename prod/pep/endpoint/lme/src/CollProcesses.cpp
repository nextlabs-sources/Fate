#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollProcesses.h"

INSTANCE_DEFINE( CHookedCollProcesses );

void CHookedCollProcesses::Hook( void* pProcesses )
{
   /* SubstituteOrgFuncWithNew( pProcesses, 13, (void*)New_CollProcesses_get_Item);
    SubstituteOrgFuncWithNew( pProcesses, 17, (void*)New_CollProcesses_Add);*/
   // SubstituteOrgFuncWithNew( pProcesses, 18, (void*)New_CollProcesses_Remove);
   // SubstituteOrgFuncWithNew( pProcesses, 25, (void*)NewCollProcessesPut_Share);
    DoHook( pProcesses );
}
//HRESULT __stdcall CHookedCollProcesses::New_CollProcesses_get_Item (ICollaborateProcesses* pProcesses,
//											  long varParam,
//											  IBaseObject * * varItem )
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//   Old_CollProcesses_get_Item pFunc = (Old_CollProcesses_get_Item)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_get_Item ));
//		if( pFunc )
//		{
//			     hr = pFunc( pProcesses, varParam, varItem );
//		}
//
//	}
//	__try
//	{
//		return my_CollProcesses_get_Item( pProcesses, varParam, varItem );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollProcesses::my_CollProcesses_get_Item (ICollaborateProcesses* pProcesses,
//											  long varParam,
//											  IBaseObject * * varItem )
//{	
//	//nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollProcesses_get_Item pFunc = (Old_CollProcesses_get_Item)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_get_Item ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcesses );
//        pFunc = (Old_CollProcesses_get_Item)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_get_Item ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcesses, varParam, varItem );
//    }
//
//	if(SUCCEEDED(hr) && (*varItem) != NULL)
//	{
//		IBaseObject* pObject = (*varItem);
//		BSTR bstrType = NULL;
//		HRESULT hr = pObject->get_TypeOf(&bstrType);
//
//		BSTR bstrProcess=NULL;
//		if(bstrType&&wcscmp(bstrType,L"ICollaborateWindow") == 0)
//		{
//			OutputDebugStringA("have some error at here!\r\n");
//		}
//		else if(bstrType&&wcscmp(bstrType,L"ICollaborateProcess")==0)
//		{
//			ICollaborateProcess* pProcess = (ICollaborateProcess* )(*varItem);
//			pProcess->get_Process(&bstrProcess);
//		}
//		hr = S_OK;
//	}
//	return hr;
//}

//HRESULT __stdcall CHookedCollProcesses::NewCollProcessesPut_Share (ICollaborateProcesses* pProcesses,
//                                             /*[in]*/ VARIANT_BOOL varShare )
//{
//    HRESULT hr = E_NOTIMPL;
//    CollProcessesPut_ShareFunc pFunc = (CollProcessesPut_ShareFunc)(GetInstance()->GetOrgFunc( (void*)pProcesses, NewCollProcessesPut_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcesses );
//        pFunc = (CollProcessesPut_ShareFunc)(GetInstance()->GetOrgFunc( (void*)pProcesses, NewCollProcessesPut_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcesses, varShare );
//    }   
//    return hr;
//}

//HRESULT __stdcall CHookedCollProcesses::New_CollProcesses_Add (ICollaborateProcesses* pProcesses,
//										 IBaseObject * pNewItem)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//   Old_CollProcesses_Add pFunc = (Old_CollProcesses_Add)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_Add ));
//		if( pFunc )
//		{
//			     hr = pFunc( pProcesses, pNewItem );
//		}
//
//	}
//	__try
//	{
//		return my_CollProcesses_Add( pProcesses, pNewItem );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollProcesses::my_CollProcesses_Add (ICollaborateProcesses* pProcesses,
//										 IBaseObject * pNewItem)
//{	
//	//nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollProcesses_Add pFunc = (Old_CollProcesses_Add)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_Add ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcesses );
//        pFunc = (Old_CollProcesses_Add)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_Add ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcesses, pNewItem );
//    }  
//
//    long lcount =0;
//    BSTR strName = NULL;
//    pProcesses->get_Count(&lcount);
//    pProcesses->get_TypeOf(&strName);
//
//    if(SUCCEEDED(hr) && pNewItem != NULL)
//    {
//        BSTR bstrType = NULL;
//        HRESULT hr = pNewItem->get_TypeOf(&bstrType);            
//        if( bstrType&&wcscmp(bstrType,L"ICollaborateProcess")==0 )
//        {
//            ICollaborateProcess* pProcess = (ICollaborateProcess* )(pNewItem);
//            ULONG lID = 0;
//            pProcess->get_Id(&lID);
//            BSTR bstrProcess=NULL;
//            pProcess->get_Process(&bstrProcess);
//            ICollaborateWindows* pWindows  = NULL;
//            hr = pProcess->get_Windows(&pWindows);
//
//            if(SUCCEEDED(hr))
//            {
//                //HookCollaborateWindows(pWindows);
//            }
//        }
//    }
//   
//	return hr;
//}

//Conflict with others
//HRESULT __stdcall CHookedCollProcesses::New_CollProcesses_Remove (ICollaborateProcesses* pProcesses,
//                                            VARIANT varKeyOrItem)
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_CollProcesses_Remove pFunc = (Func_CollProcesses_Remove)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_Remove ));
//    if( !pFunc )
//    {
//		return hr ;
//        GetInstance()->Hook( pProcesses );
//        pFunc = (Func_CollProcesses_Remove)(GetInstance()->GetOrgFunc( (void*)pProcesses, New_CollProcesses_Remove ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcesses, varKeyOrItem );
//    }  
//
//    {
//        long lcount =0;
//        BSTR strName = NULL;
//        HRESULT hr = pProcesses->get_Count(&lcount);
//        hr = pProcesses->get_TypeOf(&strName);
//    }
//   
//    return hr;
//}

// hook process
//////////////////////////////////////////////////////////////////////////
//INSTANCE_DEFINE( CHookedCollProcess );
//
//void CHookedCollProcess::Hook( void* pProcess )
//{
//    SubstituteOrgFuncWithNew( pProcess, 13, (void*)New_CollProcess_get_Id);
//    SubstituteOrgFuncWithNew( pProcess, 14, (void*)New_CollProcess_get_CollaborateApp);
//    SubstituteOrgFuncWithNew( pProcess, 15, (void*)New_CollProcess_get_Share);
//    SubstituteOrgFuncWithNew( pProcess, 16, (void*)New_CollProcess_put_Share);
//    SubstituteOrgFuncWithNew( pProcess, 17, (void*)New_CollProcess_get_Process);
//    SubstituteOrgFuncWithNew( pProcess, 19, (void*)New_CollProcess_get_Windows);
//    DoHook( pProcess );
//}
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_get_Id(ICollaborateProcess* pProcess,unsigned long* varId)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_get_Id pFunc = (Old_CollProcess_get_Id)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Id ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_get_Id)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Id ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varId );
//    }  
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_get_CollaborateApp(ICollaborateProcess* pProcess,ICollaborateApp** varCollaborateApp)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_get_CollaborateApp pFunc = (Old_CollProcess_get_CollaborateApp)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_CollaborateApp ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_get_CollaborateApp)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_CollaborateApp ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varCollaborateApp );
//    }  
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_get_Share(ICollaborateProcess* pProcess,VARIANT_BOOL* varShare)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_get_Share pFunc = (Old_CollProcess_get_Share)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_get_Share)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varShare );
//    }  
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_put_Share(ICollaborateProcess* pProcess,VARIANT_BOOL varShare)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_put_Share pFunc = (Old_CollProcess_put_Share)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_put_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_put_Share)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_put_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varShare );
//    }  
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_get_Process(ICollaborateProcess* pProcess,BSTR* varProcess)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_get_Process pFunc = (Old_CollProcess_get_Process)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Process ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_get_Process)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Process ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varProcess );
//    }  
//	return hr;
//}
//
//
//HRESULT __stdcall CHookedCollProcess::New_CollProcess_get_Windows(ICollaborateProcess* pProcess,ICollaborateWindows** varWindows)
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_CollProcess_get_Windows pFunc = (Old_CollProcess_get_Windows)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Windows ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (Old_CollProcess_get_Windows)(GetInstance()->GetOrgFunc( (void*)pProcess, New_CollProcess_get_Windows ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varWindows );
//    }  
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollProcess::NewCollProcessGet_CollaborateApp (
//    ICollaborateProcess* pProcess,
///*[out,retval]*/ struct ICollaborateApp * * varCollaborateApp )
//{
//    HRESULT hr = E_NOTIMPL;
//    CollProcessGet_CollaborateAppFunc pFunc = (CollProcessGet_CollaborateAppFunc)(GetInstance()->GetOrgFunc( (void*)pProcess, NewCollProcessGet_CollaborateApp ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pProcess );
//        pFunc = (CollProcessGet_CollaborateAppFunc)(GetInstance()->GetOrgFunc( (void*)pProcess, NewCollProcessGet_CollaborateApp ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pProcess, varCollaborateApp );
//    }  
//    return hr;
//}
//////////////////////////////////////////////////////////////////////////