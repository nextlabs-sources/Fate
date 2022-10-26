#include "stdafx.h"
#include "HookedConfSess.h"
//
//INSTANCE_DEFINE( CHookedConfSess );
//
//void CHookedConfSess::Hook( void* pConfSess )
//{
//    SubstituteOrgFuncWithNew( pConfSess, 3, (void*)/*Hooked_get_Properties*/ );
//    SubstituteOrgFuncWithNew( pConfSess, 4, (void*)Hooked_Enter );
//    SubstituteOrgFuncWithNew( pConfSess, 5, (void*)Hooked_Leave );
//    SubstituteOrgFuncWithNew( pConfSess, 6, (void*)Hooked_SetProperty );
//    DoHook( pConfSess );
//}
//
//HRESULT __stdcall CHookedConfSess::Hooked_get_Properties ( IUccConferenceSession* pConfSess,
///*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal )
//{
//    OutputDebugString( L"CHookedConfSess::Hooked_get_Properties" );
//    Func_get_Properties pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_get_Properties ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfSess );
//        pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_get_Properties ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfSess, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedConfSess::Hooked_Enter ( IUccConferenceSession* pConfSess,
//                                       /*[in]*/ BSTR bstrConfURI,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    Func_Enter pOrgFunc = (Func_Enter)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_Enter ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfSess );
//        pOrgFunc = (Func_Enter)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_Enter ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfSess, bstrConfURI, pOperationContext );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedConfSess::Hooked_Leave ( IUccConferenceSession* pConfSess,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    Func_Leave pOrgFunc = (Func_Leave)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_Leave ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfSess );
//        pOrgFunc = (Func_Leave)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_Leave ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfSess, pOperationContext );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedConfSess::Hooked_SetProperty ( IUccConferenceSession* pConfSess,
//                                             /*[in]*/ enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
//                                             /*[in]*/ VARIANT vPropertyValue,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    OutputDebugString( L"CHookedConfSess::Hooked_SetProperty" );
//    Func_SetProperty pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_SetProperty ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pConfSess );
//		pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pConfSess, Hooked_SetProperty ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pConfSess, enPropertyId, vPropertyValue, pOperationContext );
//    }
//    return hr;
//}