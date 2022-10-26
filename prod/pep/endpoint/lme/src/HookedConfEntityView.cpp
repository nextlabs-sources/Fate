#include "stdafx.h"
#include "HookedConfEntityView.h"
//
//INSTANCE_DEFINE( CHookedConfEntityView );
//
//void CHookedConfEntityView::Hook( void* pConfEntityView )
//{
//    SubstituteOrgFuncWithNew( pConfEntityView, 3, (void*)Hooked_get_Uri );
//    SubstituteOrgFuncWithNew( pConfEntityView, 4, (void*)Hooked_get_Type );
//    SubstituteOrgFuncWithNew( pConfEntityView, 5, (void*)Hooked_get_Properties );
//    DoHook( pConfEntityView );
//}
//
//HRESULT __stdcall CHookedConfEntityView::Hooked_get_Uri ( IUccConferenceEntityView* pConfEntityView, BSTR * pbstrUri )
//{
//    Func_get_Uri pOrgFunc = (Func_get_Uri)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Uri ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfEntityView );
//        pOrgFunc = (Func_get_Uri)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Uri ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfEntityView, pbstrUri );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedConfEntityView::Hooked_get_Type ( IUccConferenceEntityView* pConfEntityView, enum UCC_CONFERENCE_ENTITY_TYPE * penType )
//{
//    Func_get_Type pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Type ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfEntityView );
//        pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Type ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfEntityView, penType );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedConfEntityView::Hooked_get_Properties ( IUccConferenceEntityView* pConfEntityView, struct IUccReadOnlyPropertyCollection * * pVal )
//{
//    Func_get_Properties pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Properties ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfEntityView );
//        pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pConfEntityView, Hooked_get_Properties ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfEntityView, pVal );
//    }
//    return hr;
//}