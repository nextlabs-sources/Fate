#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
class CHookedCollMonitors: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollMonitors );

public:

    void Hook( void* pMonitors );

public:

    typedef HRESULT (__stdcall* Old_CollMonitors_get_Primary)(ICollaborateMonitors*pMonitors,ICollaborateMonitor** varPrimary);

    static HRESULT __stdcall New_CollMonitors_get_Primary(ICollaborateMonitors*pMonitors,ICollaborateMonitor** varPrimary);
	static HRESULT __stdcall my_CollMonitors_get_Primary(ICollaborateMonitors*pMonitors,ICollaborateMonitor** varPrimary);
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
class CHookedCollMonitor: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollMonitor );

public:

    void Hook( void* pMonitor );

public:

    typedef HRESULT (__stdcall* Old_CollMonitor_get_Processes)(ICollaborateMonitor* pMonitor,ICollaborateProcesses** varProcesses);

    static HRESULT __stdcall New_CollMonitor_get_Processes(ICollaborateMonitor* pMonitor,ICollaborateProcesses** varProcesses);

    typedef HRESULT (__stdcall* CollMonitorPut_ShareFuc) (ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varShare );
    static HRESULT __stdcall NewCollMonitorPut_Share (ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varShare ) ;

    typedef HRESULT (__stdcall* Func_get_Animation )( ICollaborateMonitor* pMonitor,
        /*[out,retval]*/ VARIANT_BOOL * varAnimation );
    typedef HRESULT (__stdcall* Func_put_Animation )( ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varAnimation );

    static HRESULT __stdcall Hooked_get_Animation ( ICollaborateMonitor* pMonitor,
        /*[out,retval]*/ VARIANT_BOOL * varAnimation );
    static HRESULT __stdcall Hooked_put_Animation ( ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varAnimation );




	 static HRESULT __stdcall my_CollMonitor_get_Processes(ICollaborateMonitor* pMonitor,ICollaborateProcesses** varProcesses);
	  static HRESULT __stdcall my_CollMonitorPut_Share (ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varShare ) ;
	static HRESULT __stdcall my_get_Animation ( ICollaborateMonitor* pMonitor,
        /*[out,retval]*/ VARIANT_BOOL * varAnimation );
    static HRESULT __stdcall my_put_Animation ( ICollaborateMonitor* pMonitor,
        /*[in]*/ VARIANT_BOOL varAnimation );
};
//////////////////////////////////////////////////////////////////////////
