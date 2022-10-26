#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollWindow.h"


//////////////////////////////////////////////////////////////////////////
INSTANCE_DEFINE( CHookedCollWindow );

void CHookedCollWindow::Hook( void* pWindow )
{
    SubstituteOrgFuncWithNew( pWindow, 14, (void*)New_CollWindow_Handle); 
    // SubstituteOrgFuncWithNew( pWindow, 17, (void*)New_CollWindow_Status);
    //SubstituteOrgFuncWithNew( pWindow, 24, (void*)New_CollWindow_get_Attach);
    //SubstituteOrgFuncWithNew( pWindow, 25, (void*)New_CollWindow_put_Attach);
    //SubstituteOrgFuncWithNew( pWindow, 26, (void*)New_CollWindow_get_Detach);
    //SubstituteOrgFuncWithNew( pWindow, 27, (void*)New_CollWindow_put_Detach);
    //SubstituteOrgFuncWithNew( pWindow, 28, (void*)New_CollWindow_IsActive);
    //SubstituteOrgFuncWithNew( pWindow, 29, (void*)New_CollWindow_get_CanShare);
    //SubstituteOrgFuncWithNew( pWindow, 30, (void*)New_CollWindow_get_Process);
    //SubstituteOrgFuncWithNew( pWindow, 31, (void*)New_CollWindow_get_LaunchType);
    //SubstituteOrgFuncWithNew( pWindow, 41, (void*)New_CollWindow_Open);
    //SubstituteOrgFuncWithNew( pWindow, 42, (void*)New_CollWindow_Close);
    //SubstituteOrgFuncWithNew( pWindow, 43, (void*)Hooked_BringToFront );
    DoHook( pWindow );
}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_get_Attach(ICollaborateWindow* pWindow,WindowStatus* varAttach)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//         Old_CollWindow_get_Attach pFunc = (Old_CollWindow_get_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Attach ));
//		if( pFunc )
//		{
//			     hr =    pFunc( pWindow, varAttach );
//		}
//
//	}
//	__try
//	{
//		return     my_CollWindow_get_Attach( pWindow, varAttach );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_get_Attach(ICollaborateWindow* pWindow,WindowStatus* varAttach)
//{
//	//	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_get_Attach pFunc = (Old_CollWindow_get_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Attach ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_get_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Attach ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varAttach );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_put_Attach(ICollaborateWindow* pWindow,WindowStatus varAttach)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//         Old_CollWindow_put_Attach pFunc = (Old_CollWindow_put_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Attach ));
//		if( pFunc )
//		{
//			     hr =    pFunc( pWindow, varAttach );
//		}
//
//	}
//	__try
//	{
//		return    my_CollWindow_put_Attach( pWindow, varAttach );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_put_Attach(ICollaborateWindow* pWindow,WindowStatus varAttach)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_put_Attach pFunc = (Old_CollWindow_put_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Attach ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_put_Attach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Attach ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varAttach );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollWindow::New_CollWindow_get_Detach(ICollaborateWindow* pWindow,WindowStatus* varDetach)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_get_Detach pFunc = (Old_CollWindow_get_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Detach ));
//		if( pFunc )
//		{
//			     hr =   pFunc( pWindow, varDetach );
//		}
//
//	}
//	__try
//	{
//		return   my_CollWindow_get_Detach( pWindow, varDetach );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_get_Detach(ICollaborateWindow* pWindow,WindowStatus* varDetach)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_get_Detach pFunc = (Old_CollWindow_get_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Detach ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_get_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Detach ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varDetach );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_put_Detach(ICollaborateWindow* pWindow,WindowStatus varDetach)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_put_Detach pFunc = (Old_CollWindow_put_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Detach ));
//		if( pFunc )
//		{
//			     hr =   pFunc( pWindow, varDetach );
//		}
//
//	}
//	__try
//	{
//		return   my_CollWindow_put_Detach( pWindow, varDetach );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_put_Detach(ICollaborateWindow* pWindow,WindowStatus varDetach)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_put_Detach pFunc = (Old_CollWindow_put_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Detach ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_put_Detach)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_put_Detach ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varDetach );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_IsActive(ICollaborateWindow* pWindow,VARIANT_BOOL* varIsActive)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_IsActive pFunc = (Old_CollWindow_IsActive)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_IsActive ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow, varIsActive );
//		}
//
//	}
//	__try
//	{
//		return   my_CollWindow_IsActive( pWindow, varIsActive );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_IsActive(ICollaborateWindow* pWindow,VARIANT_BOOL* varIsActive)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_IsActive pFunc = (Old_CollWindow_IsActive)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_IsActive ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_IsActive)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_IsActive ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varIsActive );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_Status(ICollaborateWindow* pWindow,WindowStatus* varStatus)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_Status pFunc = (Old_CollWindow_Status)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Status ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow, varStatus );
//		}
//
//	}
//	__try
//	{
//		return  my_CollWindow_Status( pWindow, varStatus );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_Status(ICollaborateWindow* pWindow,WindowStatus* varStatus)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_Status pFunc = (Old_CollWindow_Status)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Status ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_Status)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Status ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varStatus );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollWindow::New_CollWindow_Handle(ICollaborateWindow* pWindow,long* varHandle)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
        Old_CollWindow_Handle pFunc = (Old_CollWindow_Handle)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Handle ));
		if( pFunc )
		{
			     hr =  pFunc( pWindow, varHandle );
		}

	}
	__try
	{
		return  my_CollWindow_Handle( pWindow, varHandle );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		DPW((L"New_CollWindow_Handle exception"));
	}
	return hr;
}
HRESULT __stdcall CHookedCollWindow::my_CollWindow_Handle(ICollaborateWindow* pWindow,long* varHandle)
{	nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_FAIL;
    Old_CollWindow_Handle pFunc = (Old_CollWindow_Handle)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Handle ));
    if( !pFunc )
    {
        GetInstance()->Hook( pWindow );
        pFunc = (Old_CollWindow_Handle)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Handle ));
    }
    if( pFunc )
    {
        hr = pFunc( pWindow, varHandle );
    }

	return hr;
}
//
//HRESULT __stdcall CHookedCollWindow::New_CollWindow_get_LaunchType(ICollaborateWindow* pWindow,LaunchType* varLaunchType)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_get_LaunchType pFunc = (Old_CollWindow_get_LaunchType)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_LaunchType ));
//		if( pFunc )
//		{
//			     hr = pFunc( pWindow, varLaunchType );
//		}
//
//	}
//	__try
//	{
//		return  my_CollWindow_get_LaunchType( pWindow, varLaunchType );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_get_LaunchType(ICollaborateWindow* pWindow,LaunchType* varLaunchType)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_FAIL;
//    Old_CollWindow_get_LaunchType pFunc = (Old_CollWindow_get_LaunchType)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_LaunchType ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_get_LaunchType)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_LaunchType ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varLaunchType );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_get_Process(ICollaborateWindow* pWindow,ICollaborateProcess** varProcess)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_get_Process pFunc = (Old_CollWindow_get_Process)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Process ));
//		if( pFunc )
//		{
//			     hr = pFunc( pWindow, varProcess );
//		}
//
//	}
//	__try
//	{
//		return  my_CollWindow_get_Process( pWindow, varProcess );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_get_Process(ICollaborateWindow* pWindow,ICollaborateProcess** varProcess)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindow_get_Process pFunc = (Old_CollWindow_get_Process)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Process ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_get_Process)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_Process ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varProcess );
//    }
//	BSTR bstrTitle=NULL;
//	BSTR bstrWorkDirectory=NULL;
//	pWindow->get_Title(&bstrTitle);
//	pWindow->get_WorkingDirectory(&bstrWorkDirectory);
//
//	if(SUCCEEDED(hr) && (*varProcess) != NULL)
//	{
//		//ICollaborateProcess* pProcess = (*varProcess);
//	}
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_get_CanShare(ICollaborateWindow* pWindow,VARIANT_BOOL* varCanShare)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_get_CanShare pFunc = (Old_CollWindow_get_CanShare)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_CanShare ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow, varCanShare );
//		}
//
//	}
//	__try
//	{
//		return   my_CollWindow_get_CanShare( pWindow, varCanShare );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_get_CanShare(ICollaborateWindow* pWindow,VARIANT_BOOL* varCanShare)
//{	 nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindow_get_CanShare pFunc = (Old_CollWindow_get_CanShare)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_CanShare ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_get_CanShare)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_get_CanShare ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow, varCanShare );
//    }
//
//	BSTR bstrTitle=NULL;
//	BSTR bstrWorkDirectory=NULL;
//	pWindow->get_Title(&bstrTitle);
//	pWindow->get_WorkingDirectory(&bstrWorkDirectory);
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollWindow::New_CollWindow_Open(ICollaborateWindow* pWindow)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_Open pFunc = (Old_CollWindow_Open)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Open ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow );
//		}
//
//	}
//	__try
//	{
//		return  my_CollWindow_Open( pWindow );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//   HRESULT __stdcall CHookedCollWindow::my_CollWindow_Open(ICollaborateWindow* pWindow)
//{	 nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindow_Open pFunc = (Old_CollWindow_Open)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Open ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_Open)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Open ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow );
//    }
//
//	BSTR bstrTitle=NULL;
//	BSTR bstrWorkDirectory=NULL;
//	pWindow->get_Title(&bstrTitle);
//	pWindow->get_WorkingDirectory(&bstrWorkDirectory);
//	return hr;
//}

//HRESULT __stdcall CHookedCollWindow::New_CollWindow_Close(ICollaborateWindow* pWindow)
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Old_CollWindow_Close pFunc = (Old_CollWindow_Close)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Close ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow );
//		}
//
//	}
//	__try
//	{
//		return  my_CollWindow_Close( pWindow );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_CollWindow_Close(ICollaborateWindow* pWindow)
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//	HRESULT hr = E_NOTIMPL;
//    Old_CollWindow_Close pFunc = (Old_CollWindow_Close)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Close ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Old_CollWindow_Close)(GetInstance()->GetOrgFunc( (void*)pWindow, New_CollWindow_Close ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow );
//    }
//	BSTR bstrTitle=NULL;
//	BSTR bstrWorkDirectory=NULL;
//	pWindow->get_Title(&bstrTitle);
//	pWindow->get_WorkingDirectory(&bstrWorkDirectory);
//	return hr;
//}


//HRESULT __stdcall CHookedCollWindow::Hooked_BringToFront ( ICollaborateWindow* pWindow )
//{
//	HRESULT hr = E_NOTIMPL;
//	if( LMEIsDisabled() == true )
//	{
//        Func_BringToFront pFunc = (Func_BringToFront)(GetInstance()->GetOrgFunc( (void*)pWindow, Hooked_BringToFront ));
//		if( pFunc )
//		{
//			     hr =  pFunc( pWindow );
//		}
//
//	}
//	__try
//	{
//		return  my_BringToFront( pWindow );
//	}
//	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
//	{
//		;
//	}
//	return hr;
//}
//HRESULT __stdcall CHookedCollWindow::my_BringToFront ( ICollaborateWindow* pWindow )
//{	nextlabs::recursion_control_auto auto_disable(hook_control);
//    HRESULT hr = E_NOTIMPL;
//    Func_BringToFront pFunc = (Func_BringToFront)(GetInstance()->GetOrgFunc( (void*)pWindow, Hooked_BringToFront ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pWindow );
//        pFunc = (Func_BringToFront)(GetInstance()->GetOrgFunc( (void*)pWindow, Hooked_BringToFront ));
//    }
//    if( pFunc )
//    {
//        hr = pFunc( pWindow );
//    }
//    BSTR bstrTitle=NULL;
//    BSTR bstrWorkDirectory=NULL;
//    pWindow->get_Title(&bstrTitle);
//    pWindow->get_WorkingDirectory(&bstrWorkDirectory);
//    return hr;
//}
//////////////////////////////////////////////////////////////////////////