#include "StdAfx.h"
#include "OfficeShare.h"
#include "CollMonitors.h"
#include "CollProcesses.h"

// hook monitors
//////////////////////////////////////////////////////////////////////////
INSTANCE_DEFINE( CHookedCollMonitors );

void CHookedCollMonitors::Hook( void* pMonitors )
{
    SubstituteOrgFuncWithNew( pMonitors, 22, (void*)New_CollMonitors_get_Primary);
    DoHook( pMonitors );
}


HRESULT __stdcall CHookedCollMonitors::New_CollMonitors_get_Primary(ICollaborateMonitors*pMonitors,ICollaborateMonitor** varPrimary)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollMonitors_get_Primary pFunc = (Old_CollMonitors_get_Primary)(GetInstance()->GetOrgFunc( (void*)pMonitors, New_CollMonitors_get_Primary ));
		if( pFunc )
		{
			return   pFunc( pMonitors, varPrimary );
		}

	}
	__try
	{
		return   my_CollMonitors_get_Primary( pMonitors, varPrimary );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}

HRESULT __stdcall CHookedCollMonitors::my_CollMonitors_get_Primary(ICollaborateMonitors*pMonitors,ICollaborateMonitor** varPrimary)
{	
//	nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_NOTIMPL;
	Old_CollMonitors_get_Primary pFunc = (Old_CollMonitors_get_Primary)(GetInstance()->GetOrgFunc( (void*)pMonitors, New_CollMonitors_get_Primary ));
	if( !pFunc )
	{
		GetInstance()->Hook( pMonitors );
		pFunc = (Old_CollMonitors_get_Primary)(GetInstance()->GetOrgFunc( (void*)pMonitors, New_CollMonitors_get_Primary ));
	}

    if( pFunc )
    {
        hr = pFunc( pMonitors, varPrimary );
    }

	if(SUCCEEDED(hr))
	{
		ICollaborateMonitor* pMonitor = (*varPrimary);
        BSTR* pName = 0;
        hr = pMonitor->get_Name( pName );
        CHookedCollMonitor::GetInstance()->Hook( PVOID( pMonitor ) );
	}
	return hr;
}
//////////////////////////////////////////////////////////////////////////

INSTANCE_DEFINE( CHookedCollMonitor );

void CHookedCollMonitor::Hook( void* pMonitor )
{
    SubstituteOrgFuncWithNew( pMonitor, 32, (void*)New_CollMonitor_get_Processes);
    //SubstituteOrgFuncWithNew( pMonitor, 31, (void*)NewCollMonitorPut_Share);

    //SubstituteOrgFuncWithNew( pMonitor, 14, (void*)Hooked_get_Animation);
    //SubstituteOrgFuncWithNew( pMonitor, 15, (void*)Hooked_put_Animation);
   DoHook( pMonitor );
}

HRESULT __stdcall CHookedCollMonitor::New_CollMonitor_get_Processes(ICollaborateMonitor* pMonitor,ICollaborateProcesses** varProcesses)
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		Old_CollMonitor_get_Processes pFunc = (Old_CollMonitor_get_Processes)(GetInstance()->GetOrgFunc( (void*)pMonitor, New_CollMonitor_get_Processes ));
		if( pFunc )
		{
			hr = pFunc( pMonitor, varProcesses );
		}

	}
	__try
	{
		return   hr = my_CollMonitor_get_Processes( pMonitor, varProcesses );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}

HRESULT __stdcall CHookedCollMonitor::my_CollMonitor_get_Processes(ICollaborateMonitor* pMonitor,ICollaborateProcesses** varProcesses)
{
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	HRESULT hr = E_FAIL;
	Old_CollMonitor_get_Processes pFunc = (Old_CollMonitor_get_Processes)(GetInstance()->GetOrgFunc( (void*)pMonitor, New_CollMonitor_get_Processes ));
	if( !pFunc )
	{
		GetInstance()->Hook( pMonitor );
		pFunc = (Old_CollMonitor_get_Processes)(GetInstance()->GetOrgFunc( (void*)pMonitor, New_CollMonitor_get_Processes ));
	}

    if( pFunc )
    {
        hr = pFunc( pMonitor, varProcesses );
    }

    if(SUCCEEDED(hr))
    {
        ICollaborateProcesses* pProcesses = (ICollaborateProcesses*)(*varProcesses);
        CHookedCollProcesses::GetInstance()->Hook( PVOID( pProcesses ) );
    }
    return hr;
}

//HRESULT __stdcall CHookedCollMonitor::NewCollMonitorPut_Share (ICollaborateMonitor* pMonitor,
//                                           /*[in]*/ VARIANT_BOOL varShare )
//{
//    HRESULT hr = E_FAIL;
//    CollMonitorPut_ShareFuc pFunc = (CollMonitorPut_ShareFuc)(GetInstance()->GetOrgFunc( (void*)pMonitor, NewCollMonitorPut_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pMonitor );
//        pFunc = (CollMonitorPut_ShareFuc)(GetInstance()->GetOrgFunc( (void*)pMonitor, NewCollMonitorPut_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pMonitor, varShare );
//    }
//
//    return hr;
//}

//HRESULT __stdcall CHookedCollMonitor::Hooked_get_Animation ( ICollaborateMonitor* pMonitor,
//                                               /*[out,retval]*/ VARIANT_BOOL * varAnimation )
//{
//    HRESULT hr = E_FAIL;
//    Func_get_Animation pFunc = (Func_get_Animation)(GetInstance()->GetOrgFunc( (void*)pMonitor, Hooked_get_Animation ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pMonitor );
//        pFunc = (Func_get_Animation)(GetInstance()->GetOrgFunc( (void*)pMonitor, Hooked_get_Animation ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pMonitor, varAnimation );
//    }
//
//    return hr;
//}

//HRESULT __stdcall CHookedCollMonitor::Hooked_put_Animation ( ICollaborateMonitor* pMonitor,
//                                               /*[in]*/ VARIANT_BOOL varAnimation )
//{
//    HRESULT hr = E_FAIL;
//    Func_put_Animation pFunc = (Func_put_Animation)(GetInstance()->GetOrgFunc( (void*)pMonitor, Hooked_put_Animation ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pMonitor );
//        pFunc = (Func_put_Animation)(GetInstance()->GetOrgFunc( (void*)pMonitor, Hooked_put_Animation ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pMonitor, varAnimation );
//    }
//
//    return hr;
//}
//////////////////////////////////////////////////////////////////////////
