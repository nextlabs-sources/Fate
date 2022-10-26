#include "stdafx.h"
#include "HookedConfSessPart.h"

//INSTANCE_DEFINE( CHookedConfSessPart );
//
//void CHookedConfSessPart::Hook( void* pIUccConferenceSessionParticipant )
//{
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipant, 7, (void*)Hooked_CreateParticipantEndpoint );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipant, 4, (void*)Hooked_SetProperty );
//    
//    DoHook( pIUccConferenceSessionParticipant );
//}
//
//
//
//HRESULT __stdcall CHookedConfSessPart::Hooked_CreateParticipantEndpoint( 
//    IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant, 
//    /*[in]*/ BSTR bstrEndpointUri,
//    /*[in]*/ BSTR bstrEndpointId,
//    /*[in]*/ enum UCC_CONFERENCE_ENTITY_TYPE enType,
///*[in]*/ struct IUccContext * pContext,
///*[out,retval]*/ struct IUccConferenceSessionParticipantEndpoint * * ppParticipantEndpoint )
//{
//    Func_CreateParticipantEndpoint pOrgFunc = (Func_CreateParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipant, Hooked_CreateParticipantEndpoint ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipant );
//        pOrgFunc = (Func_CreateParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipant, Hooked_CreateParticipantEndpoint ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipant, 
//            bstrEndpointUri, bstrEndpointId, enType, pContext, ppParticipantEndpoint );
//
//      /*  if( S_OK == hr )
//        {
//            CConfSessPartEndpoint::GetInstance()->Hook( PVOID(*ppParticipantEndpoint) );
//        }*/
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedConfSessPart::Hooked_SetProperty (
//    IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant,
//    /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_PROPERTY enPropertyId,
//    /*[in]*/ VARIANT vPropertyValue,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    OutputDebugString( L"CHookedConfSessPart::Hooked_SetProperty" );
//    Func_SetProperty pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipant, Hooked_SetProperty ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipant );
//        pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipant, Hooked_SetProperty ));
//    }
//
//    vPropertyValue.bstrVal;
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipant, 
//             enPropertyId, vPropertyValue, pOperationContext );
//    }
//    return hr;
//}