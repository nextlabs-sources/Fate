#pragma once

//////////////////////////////////////////////////////////////////////////
class CHookedCollProcesses: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollProcesses );

public:

    void Hook( void* pProcesses );

public:

    typedef HRESULT (__stdcall* Old_CollProcesses_get_Item) (ICollaborateProcesses* pProcesses,
        long varParam,
        IBaseObject * * varItem );
    typedef HRESULT (__stdcall* Old_CollProcesses_Add) (ICollaborateProcesses* pProcesses,
        IBaseObject * pNewItem ) ;
    typedef HRESULT (__stdcall* Func_CollProcesses_Remove) (ICollaborateProcesses* pProcesses,
        VARIANT varKeyOrItem);

    static HRESULT __stdcall New_CollProcesses_get_Item (ICollaborateProcesses* pProcesses,
        long varParam,
        IBaseObject * * varItem );
    static HRESULT __stdcall New_CollProcesses_Add (ICollaborateProcesses* pProcesses,
        IBaseObject * pNewItem);
    static HRESULT __stdcall New_CollProcesses_Remove (ICollaborateProcesses* pProcesses,
        VARIANT varKeyOrItem);
    static HRESULT __stdcall NewCollProcessesPut_Share (ICollaborateProcesses* pProcesses,
        /*[in]*/ VARIANT_BOOL varShare );
    typedef HRESULT (__stdcall* CollProcessesPut_ShareFunc ) (ICollaborateProcesses* pProcesses,
        /*[in]*/ VARIANT_BOOL varShare );



	  static HRESULT __stdcall my_CollProcesses_get_Item (ICollaborateProcesses* pProcesses,
        long varParam,
        IBaseObject * * varItem );
    static HRESULT __stdcall my_CollProcesses_Add (ICollaborateProcesses* pProcesses,
        IBaseObject * pNewItem);
    static HRESULT __stdcall my_CollProcesses_Remove (ICollaborateProcesses* pProcesses,
        VARIANT varKeyOrItem);
    static HRESULT __stdcall my_CollProcessesPut_Share (ICollaborateProcesses* pProcesses,
        /*[in]*/ VARIANT_BOOL varShare );
};

//////////////////////////////////////////////////////////////////////////
//class CHookedCollProcess: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedCollProcess );
//
//public:
//
//    void Hook( void* pProcess );
//
//public:
//
//    typedef HRESULT (__stdcall* Old_CollProcess_get_Id)(ICollaborateProcess* pProcess,unsigned long* varId);
//    typedef HRESULT (__stdcall* Old_CollProcess_get_CollaborateApp)(ICollaborateProcess* pProcess,ICollaborateApp** varCollaborateApp);
//    typedef HRESULT (__stdcall* Old_CollProcess_get_Share)(ICollaborateProcess* pProcess,VARIANT_BOOL* varShare);
//    typedef HRESULT (__stdcall* Old_CollProcess_put_Share)(ICollaborateProcess* pProcess,VARIANT_BOOL varShare);
//    typedef HRESULT (__stdcall* Old_CollProcess_get_Process)(ICollaborateProcess* pProcess,BSTR* varProcess);
//    typedef HRESULT (__stdcall* Old_CollProcess_get_Windows)(ICollaborateProcess* pProcess,ICollaborateWindows** varWindows);
//
//    static HRESULT __stdcall New_CollProcess_get_Id(ICollaborateProcess* pProcess,unsigned long* varId);
//    static HRESULT __stdcall New_CollProcess_get_CollaborateApp(ICollaborateProcess* pProcess,ICollaborateApp** varCollaborateApp);
//    static HRESULT __stdcall New_CollProcess_get_Share(ICollaborateProcess* pProcess,VARIANT_BOOL* varShare);
//    static HRESULT __stdcall New_CollProcess_put_Share(ICollaborateProcess* pProcess,VARIANT_BOOL varShare);
//    static HRESULT __stdcall New_CollProcess_get_Process(ICollaborateProcess* pProcess,BSTR* varProcess);
//    static HRESULT __stdcall New_CollProcess_get_Windows(ICollaborateProcess* pProcess,ICollaborateWindows** varWindows);
//    typedef HRESULT (__stdcall* CollProcessGet_CollaborateAppFunc) (
//        ICollaborateProcess* pProcess,
//    /*[out,retval]*/ struct ICollaborateApp * * varCollaborateApp );
//    static HRESULT __stdcall NewCollProcessGet_CollaborateApp (
//        ICollaborateProcess* pProcess,
//    /*[out,retval]*/ struct ICollaborateApp * * varCollaborateApp );
//};
//////////////////////////////////////////////////////////////////////////