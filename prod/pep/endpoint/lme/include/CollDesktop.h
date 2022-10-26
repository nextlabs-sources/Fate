#pragma once

//////////////////////////////////////////////////////////////////////////
// ICollaborateDesktop
class CHookedCollDesktop: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollDesktop );

public:

    void Hook( void* pDesktop );

public:

    typedef HRESULT (__stdcall* Old_CollDesktop_get_Themes)(ICollaborateDesktop* pDesktop,/*[out, retval]*/ SAFEARRAY** varThemes);
    static HRESULT __stdcall New_CollDesktop_get_Themes(ICollaborateDesktop* pDesktop,/*[out, retval]*/ SAFEARRAY** varThemes);


    typedef HRESULT (__stdcall* Old_Desktop_put_Active)(ICollaborateDesktop* pDesktop,
        /*[in]*/ VARIANT_BOOL varActive);
    static HRESULT __stdcall New_Desktop_put_Active (ICollaborateDesktop* pDesktop,
        /*[in]*/ VARIANT_BOOL varActive );

    typedef HRESULT (__stdcall* Old_Desktop_get_Monitors) (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateMonitors * * varMonitors );
    static HRESULT __stdcall New_Desktop_get_Monitors (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateMonitors * * varMonitors );

    typedef HRESULT (__stdcall* Old_Desktop_get_Windows) (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateWindows * * varWindows );
    static HRESULT __stdcall New_Desktop_get_Windows (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateWindows * * varWindows );


    typedef HRESULT (__stdcall* Old_Desktop_get_CollaborateApp) (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct IBaseObject * * varCollaborateApp );
    static HRESULT __stdcall New_Desktop_get_CollaborateApp (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct IBaseObject * * varCollaborateApp );


    typedef HRESULT (__stdcall* Old_Desktop_get_ShareProcess) (ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varParam,
        /*[out,retval]*/ VARIANT_BOOL * varShareProcess );
    static HRESULT __stdcall New_Desktop_get_ShareProcess (ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varParam,
        /*[out,retval]*/ VARIANT_BOOL * varShareProcess );
    typedef HRESULT (__stdcall* Old_Desktop_put_ShareProcess) (ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varParam,
        /*[out,retval]*/ VARIANT_BOOL  varShareProcess );
    static HRESULT __stdcall New_Desktop_put_ShareProcess (ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varParam,
        /*[out,retval]*/ VARIANT_BOOL varShareProcess );


    typedef HRESULT (__stdcall* Old_Desktop_put_ProcessId) (ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varProcessId );
    static HRESULT __stdcall New_Desktop_put_ProcessId(ICollaborateDesktop* pDesktop,
        /*[in]*/ unsigned long varProcessId);


    typedef HRESULT (__stdcall* Old_Desktop_put_PrimaryWindow) (ICollaborateDesktop* pDesktop,
        /*[in]*/ long varPrimaryWindow ) ;
    static HRESULT __stdcall New_Desktop_put_PrimaryWindow (ICollaborateDesktop* pDesktop,
        /*[in]*/ long varPrimaryWindow ) ;




	static HRESULT __stdcall my_CollDesktop_get_Themes(ICollaborateDesktop* pDesktop,/*[out, retval]*/ SAFEARRAY** varThemes);
	static HRESULT __stdcall my_Desktop_put_Active (ICollaborateDesktop* pDesktop,
		/*[in]*/ VARIANT_BOOL varActive );
	static HRESULT __stdcall my_Desktop_get_Monitors (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateMonitors * * varMonitors );
	 static HRESULT __stdcall my_Desktop_get_Windows (ICollaborateDesktop* pDesktop,
    /*[out,retval]*/ struct ICollaborateWindows * * varWindows );
	static HRESULT __stdcall my_Desktop_get_CollaborateApp (ICollaborateDesktop* pDesktop,
	/*[out,retval]*/ struct IBaseObject * * varCollaborateApp );
	static HRESULT __stdcall my_Desktop_get_ShareProcess (ICollaborateDesktop* pDesktop,
		/*[in]*/ unsigned long varParam,
		/*[out,retval]*/ VARIANT_BOOL * varShareProcess );
	static HRESULT __stdcall my_Desktop_put_ShareProcess (ICollaborateDesktop* pDesktop,
		/*[in]*/ unsigned long varParam,
		/*[out,retval]*/ VARIANT_BOOL varShareProcess );
	static HRESULT __stdcall my_Desktop_put_ProcessId(ICollaborateDesktop* pDesktop,
		/*[in]*/ unsigned long varProcessId);
	static HRESULT __stdcall my_Desktop_put_PrimaryWindow (ICollaborateDesktop* pDesktop,
		/*[in]*/ long varPrimaryWindow ) ;
};
//////////////////////////////////////////////////////////////////////////
