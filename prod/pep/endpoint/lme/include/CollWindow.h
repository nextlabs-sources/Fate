#pragma once

//////////////////////////////////////////////////////////////////////////
class CHookedCollWindow: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollWindow );

public:

    void Hook( void* pWindow );

public:

    typedef HRESULT (__stdcall* Old_CollWindow_get_Attach)(ICollaborateWindow* pWindow,WindowStatus* varAttach);
    typedef HRESULT (__stdcall* Old_CollWindow_put_Attach)(ICollaborateWindow* pWindow,WindowStatus varAttach);
    typedef HRESULT (__stdcall* Old_CollWindow_get_Detach)(ICollaborateWindow* pWindow,WindowStatus* varDetach);
    typedef HRESULT (__stdcall* Old_CollWindow_put_Detach)(ICollaborateWindow* pWindow,WindowStatus varDetach);
    typedef HRESULT (__stdcall* Old_CollWindow_IsActive)(ICollaborateWindow* pWindow,VARIANT_BOOL* varIsActive);
    typedef HRESULT (__stdcall* Old_CollWindow_Status)(ICollaborateWindow* pWindow,WindowStatus* varStatus);
    typedef HRESULT (__stdcall* Old_CollWindow_Handle)(ICollaborateWindow* pWindow,long* varHandle);
    typedef HRESULT (__stdcall* Old_CollWindow_get_LaunchType)(ICollaborateWindow* pWindow,LaunchType* varLaunchType);


    static HRESULT __stdcall New_CollWindow_get_Attach(ICollaborateWindow* pWindow,WindowStatus* varAttach);
    static HRESULT __stdcall New_CollWindow_put_Attach(ICollaborateWindow* pWindow,WindowStatus varAttach);
    static HRESULT __stdcall New_CollWindow_get_Detach(ICollaborateWindow* pWindow,WindowStatus* varDetach);
    static HRESULT __stdcall New_CollWindow_put_Detach(ICollaborateWindow* pWindow,WindowStatus varDetach);
    static HRESULT __stdcall New_CollWindow_IsActive(ICollaborateWindow* pWindow,VARIANT_BOOL* varIsActive);
    static HRESULT __stdcall New_CollWindow_Status(ICollaborateWindow* pWindow,WindowStatus* varStatus);
    static HRESULT __stdcall New_CollWindow_Handle(ICollaborateWindow* pWindow,long* varHandle);
    static HRESULT __stdcall New_CollWindow_get_LaunchType(ICollaborateWindow* pWindow,LaunchType* varLaunchType);

    typedef HRESULT (__stdcall* Old_CollWindow_get_CanShare)(ICollaborateWindow* pWindow,VARIANT_BOOL* varCanShare);
    typedef HRESULT (__stdcall* Old_CollWindow_get_Process)(ICollaborateWindow* pWindow,ICollaborateProcess** varProcess);
    typedef HRESULT (__stdcall* Old_CollWindow_Open)(ICollaborateWindow* pWindow);
    typedef HRESULT (__stdcall* Old_CollWindow_Close)(ICollaborateWindow* pWindow);

    static HRESULT __stdcall New_CollWindow_get_CanShare(ICollaborateWindow* pWindow,VARIANT_BOOL* varCanShare);
    static HRESULT __stdcall New_CollWindow_get_Process(ICollaborateWindow* pWindow,ICollaborateProcess** varProcess);
    static HRESULT __stdcall New_CollWindow_Open(ICollaborateWindow* pWindow);
    static HRESULT __stdcall New_CollWindow_Close(ICollaborateWindow* pWindow);

    typedef HRESULT (__stdcall* Func_get_Title) (ICollaborateWindow* pWindow,
        /*[out,retval]*/ BSTR * varTitle ) ;

    static HRESULT __stdcall Hooked_get_Title (ICollaborateWindow* pWindow,
        /*[out,retval]*/ BSTR * varTitle ) ;

    typedef HRESULT (__stdcall* Func_BringToFront ) ( ICollaborateWindow* pWindow );

    static HRESULT __stdcall Hooked_BringToFront ( ICollaborateWindow* pWindow ) ;





	 static HRESULT __stdcall my_CollWindow_get_Attach(ICollaborateWindow* pWindow,WindowStatus* varAttach);
    static HRESULT __stdcall my_CollWindow_put_Attach(ICollaborateWindow* pWindow,WindowStatus varAttach);
    static HRESULT __stdcall my_CollWindow_get_Detach(ICollaborateWindow* pWindow,WindowStatus* varDetach);
    static HRESULT __stdcall my_CollWindow_put_Detach(ICollaborateWindow* pWindow,WindowStatus varDetach);
    static HRESULT __stdcall my_CollWindow_IsActive(ICollaborateWindow* pWindow,VARIANT_BOOL* varIsActive);
    static HRESULT __stdcall my_CollWindow_Status(ICollaborateWindow* pWindow,WindowStatus* varStatus);
    static HRESULT __stdcall my_CollWindow_Handle(ICollaborateWindow* pWindow,long* varHandle);
    static HRESULT __stdcall my_CollWindow_get_LaunchType(ICollaborateWindow* pWindow,LaunchType* varLaunchType);
	 static HRESULT __stdcall my_CollWindow_get_CanShare(ICollaborateWindow* pWindow,VARIANT_BOOL* varCanShare);
    static HRESULT __stdcall my_CollWindow_get_Process(ICollaborateWindow* pWindow,ICollaborateProcess** varProcess);
    static HRESULT __stdcall my_CollWindow_Open(ICollaborateWindow* pWindow);
    static HRESULT __stdcall my_CollWindow_Close(ICollaborateWindow* pWindow);

	 static HRESULT __stdcall my_get_Title (ICollaborateWindow* pWindow,
        /*[out,retval]*/ BSTR * varTitle ) ;
	   static HRESULT __stdcall my_BringToFront ( ICollaborateWindow* pWindow ) ;
};
//////////////////////////////////////////////////////////////////////////