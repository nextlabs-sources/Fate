#pragma once

//hook Frames
//////////////////////////////////////////////////////////////////////////
class CHookedCollFrames: public CHookBase
{
    INSTANCE_DECLARE( CHookedCollFrames );

public:

    void Hook( void* pFrames );

public:

    typedef HRESULT (__stdcall* Old_CollFrames_get_Item) (ICollaborateFrames* pObject,
        /*[in]*/ long varParam,
    /*[out,retval]*/ struct IBaseObject * * varItem );
    typedef HRESULT (__stdcall* Old_CollFrames_get_Count) (ICollaborateFrames* pObject,
        /*[out,retval]*/ long * varCount );
    typedef HRESULT (__stdcall* Old_CollFrames_get_NewEnum) (ICollaborateFrames* pObject,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    typedef HRESULT (__stdcall* Old_CollFrames_Empty) ( ICollaborateFrames* pObject) ;
    typedef HRESULT (__stdcall* Old_CollFrames_Add) (ICollaborateFrames* pObject,
    /*[in]*/ struct IBaseObject * pNewItem ) ;
    typedef HRESULT (__stdcall* Old_CollFrames_Remove) (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKeyOrItem );
    typedef HRESULT (__stdcall* Old_CollFrames_Insert) (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    typedef HRESULT (__stdcall* Old_CollFrames_Exists) (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    typedef HRESULT (__stdcall* Old_CollFrames_Lookup) (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );


    static HRESULT __stdcall New_CollFrames_get_Item (ICollaborateFrames* pObject,
        /*[in]*/ long varParam,
    /*[out,retval]*/ struct IBaseObject * * varItem );
    static HRESULT __stdcall New_CollFrames_get_Count (ICollaborateFrames* pObject,
        /*[out,retval]*/ long * varCount );
    static HRESULT __stdcall New_CollFrames_get_NewEnum (ICollaborateFrames* pObject,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    static HRESULT __stdcall New_CollFrames_Empty ( ICollaborateFrames* pObject) ;
    static HRESULT __stdcall New_CollFrames_Add (ICollaborateFrames* pObject,
    /*[in]*/ struct IBaseObject * pNewItem ) ;
    static HRESULT __stdcall New_CollFrames_Remove (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKeyOrItem );
    static HRESULT __stdcall New_CollFrames_Insert (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    static HRESULT __stdcall New_CollFrames_Exists (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    static HRESULT __stdcall New_CollFrames_Lookup (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );


	 static HRESULT __stdcall my_CollFrames_get_Item (ICollaborateFrames* pObject,
        /*[in]*/ long varParam,
    /*[out,retval]*/ struct IBaseObject * * varItem );
    static HRESULT __stdcall my_CollFrames_get_Count (ICollaborateFrames* pObject,
        /*[out,retval]*/ long * varCount );
    static HRESULT __stdcall my_CollFrames_get_NewEnum (ICollaborateFrames* pObject,
        /*[out,retval]*/ IUnknown * * ppUnkEnum ) ;
    static HRESULT __stdcall my_CollFrames_Empty ( ICollaborateFrames* pObject) ;
    static HRESULT __stdcall my_CollFrames_Add (ICollaborateFrames* pObject,
    /*[in]*/ struct IBaseObject * pNewItem ) ;
    static HRESULT __stdcall my_CollFrames_Remove (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKeyOrItem );
    static HRESULT __stdcall my_CollFrames_Insert (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity ) ;
    static HRESULT __stdcall my_CollFrames_Exists (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
        /*[out,retval]*/ VARIANT_BOOL * pfExists ) ;
    static HRESULT __stdcall my_CollFrames_Lookup (ICollaborateFrames* pObject,
        /*[in]*/ VARIANT varKey,
    /*[out,retval]*/ struct IBaseObject * * ppEntity );
};


//////////////////////////////////////////////////////////////////////////

//class CHookedCollFrame: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedCollFrame );
//
//public:
//
//    void Hook( void* pFrame );
//
//public:
//
//    typedef HRESULT (__stdcall* Old_CollFrame_Open)(ICollaborateFrame* pFrame,VARIANT_BOOL fSnapshot);
//    static HRESULT __stdcall New_CollFrame_Open(ICollaborateFrame* pFrame,VARIANT_BOOL fSnapshot);
//    typedef HRESULT (__stdcall* Old_CollFrame_Close)(ICollaborateFrame* pFrame);
//    static HRESULT __stdcall New_CollFrame_Close(ICollaborateFrame* pFrame);
//
//    typedef HRESULT (__stdcall* Func_get_Handle) (ICollaborateFrame* pFrame,
//        /*[out,retval]*/ long * varHandle ) ;
//    typedef HRESULT (__stdcall* Func_get_IsSnapshot) (ICollaborateFrame* pFrame,
//        /*[out,retval]*/ VARIANT_BOOL * varIsSnapshot );
//
//    static HRESULT __stdcall Hooked_get_Handle (ICollaborateFrame* pFrame,
//        /*[out,retval]*/ long * varHandle ) ;
//    static HRESULT __stdcall Hooked_get_IsSnapshot (ICollaborateFrame* pFrame,
//        /*[out,retval]*/ VARIANT_BOOL * varIsSnapshot ) ;
//};

//////////////////////////////////////////////////////////////////////////

