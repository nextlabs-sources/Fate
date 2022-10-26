#pragma once

//hook Windows
//////////////////////////////////////////////////////////////////////////
class CHookedCollWindows: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollWindows );

public:

    void Hook( void* pWindows );

public:

    typedef HRESULT (__stdcall* Old_CollWindows_get_Item) (ICollaborateWindows* pWindows,
        /*[in]*/ long varParam,
    /*[out,retval]*/ struct IBaseObject * * varItem );
    typedef HRESULT (__stdcall* Old_CollWindows_get_Count) (ICollaborateWindows* pWindows,
        /*[out,retval]*/ long * varCount );
    typedef HRESULT (__stdcall* Old_CollWindows_get_NewEnum) (ICollaborateWindows* pWindows,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    typedef HRESULT (__stdcall* Old_CollWindows_Empty) ( ICollaborateWindows* pWindows) ;
    typedef HRESULT (__stdcall* Old_CollWindows_Add) (ICollaborateWindows* pWindows,
    /*[in]*/ struct IBaseObject * pNewItem ) ;
    typedef HRESULT (__stdcall* Old_CollWindows_Remove) (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKeyOrItem );
    typedef HRESULT (__stdcall* Old_CollWindows_Insert) (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    typedef HRESULT (__stdcall* Old_CollWindows_Exists) (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    typedef HRESULT (__stdcall* Old_CollWindows_Lookup) (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );

    static HRESULT __stdcall New_CollWindows_get_Item (
        ICollaborateWindows* pWindows,
        long varParam,
        IBaseObject * * varItem );

    static HRESULT __stdcall New_CollWindows_get_Count (ICollaborateWindows* pWindows,
        /*[out,retval]*/ long * varCount );
    static HRESULT __stdcall New_CollWindows_get_NewEnum (ICollaborateWindows* pWindows,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    static HRESULT __stdcall New_CollWindows_Empty ( ICollaborateWindows* pWindows) ;
    static HRESULT __stdcall New_CollWindows_Add ( ICollaborateWindows* pWindows, struct IBaseObject * pNewItem ) ;
    static HRESULT __stdcall New_CollWindows_Remove (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKeyOrItem );
    static HRESULT __stdcall New_CollWindows_Insert (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    static HRESULT __stdcall New_CollWindows_Exists (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    static HRESULT __stdcall New_CollWindows_Lookup (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );



	 static HRESULT __stdcall my_CollWindows_get_Item (
        ICollaborateWindows* pWindows,
        long varParam,
        IBaseObject * * varItem );

    static HRESULT __stdcall my_CollWindows_get_Count (ICollaborateWindows* pWindows,
        /*[out,retval]*/ long * varCount );
    static HRESULT __stdcall my_CollWindows_get_NewEnum (ICollaborateWindows* pWindows,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    static HRESULT __stdcall my_CollWindows_Empty ( ICollaborateWindows* pWindows) ;
    static HRESULT __stdcall my_CollWindows_Add ( ICollaborateWindows* pWindows, struct IBaseObject * pNewItem ) ;
    static HRESULT __stdcall my_CollWindows_Remove (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKeyOrItem );
    static HRESULT __stdcall my_CollWindows_Insert (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    static HRESULT __stdcall my_CollWindows_Exists (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    static HRESULT __stdcall my_CollWindows_Lookup (ICollaborateWindows* pWindows,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );
};
//////////////////////////////////////////////////////////////////////////
