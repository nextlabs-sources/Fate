#pragma once
//#include "HookBase.h"
//
//class CHookedConfEntityView: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedConfEntityView );
//    //IUccConferenceEntityView
//
//public:
//
//    void Hook( void* pConfEntityView );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_get_Uri )( IUccConferenceEntityView* pConfEntityView, BSTR * pbstrUri ) ;
//    typedef HRESULT (__stdcall* Func_get_Type )( IUccConferenceEntityView* pConfEntityView, enum UCC_CONFERENCE_ENTITY_TYPE * penType ) ;
//    typedef HRESULT (__stdcall* Func_get_Properties )( IUccConferenceEntityView* pConfEntityView, struct IUccReadOnlyPropertyCollection * * pVal ) ;
//
//    static HRESULT __stdcall Hooked_get_Uri ( IUccConferenceEntityView* pConfEntityView, BSTR * pbstrUri );
//    static HRESULT __stdcall Hooked_get_Type ( IUccConferenceEntityView* pConfEntityView, enum UCC_CONFERENCE_ENTITY_TYPE * penType );
//    static HRESULT __stdcall Hooked_get_Properties ( IUccConferenceEntityView* pConfEntityView, struct IUccReadOnlyPropertyCollection * * pVal );
//};