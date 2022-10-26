#include "stdafx.h"
#include "HookedConfSessPartEndPoint.h"
//
//INSTANCE_DEFINE( CConfSessPartEndpoint );
//
//void CConfSessPartEndpoint::Hook( void* pIUccConferenceSessionParticipantEndpoint )
//{
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 3, (void*)Hooked_get_State );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 4, (void*)Hooked_get_Type );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 5, (void*)Hooked_get_Channels );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 6, (void*)Hooked_CreateChannel );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 7, (void*)Hooked_AddChannel );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 8, (void*)Hooked_RemoveChannel );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 9, (void*)Hooked_UpdateChannels );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 10, (void*)Hooked_get_Properties );
//    SubstituteOrgFuncWithNew( pIUccConferenceSessionParticipantEndpoint, 11, (void*)Hooked_SetProperty );
//
//    DoHook( pIUccConferenceSessionParticipantEndpoint );
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_get_State (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//                                           /*[out,retval]*/ enum UCC_SESSION_ENTITY_STATE * penState )
//{
//    Func_get_State pOrgFunc = (Func_get_State)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_State ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_get_State)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_State ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, penState );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_get_Type (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//                                                           /*[out,retval]*/ enum UCC_CONFERENCE_ENTITY_TYPE * penType )
//{
//    Func_get_Type pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Type ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Type ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, penType );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_get_Channels (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
///*[out,retval]*/ struct IUccCollection * * ppChannels )
//{
//    Func_get_Channels pOrgFunc = (Func_get_Channels)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Channels ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_get_Channels)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Channels ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, ppChannels );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_CreateChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//                                               /*[in]*/ enum UCC_MEDIA_TYPES enMediaType,
///*[in]*/ struct IUccContext * pContext,
///*[out,retval]*/ struct IUccMediaChannel * * ppChannel )
//{
//    Func_CreateChannel pOrgFunc = (Func_CreateChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_CreateChannel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_CreateChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_CreateChannel ));
//    }
//
//    UCC_CONFERENCE_ENTITY_TYPE eConfType;
//
//    pIUccConferenceSessionParticipantEndpoint->get_Type( &eConfType );
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, enMediaType, pContext, ppChannel );
//       /* if( S_OK == hr )
//        {
//            CHookedMediaChannel::GetInstance()->Hook( PVOID( *ppChannel ) );
//        }*/
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_AddChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
///*[in]*/ struct IUccMediaChannel * pChannel ) 
//{
//    Func_AddChannel pOrgFunc = (Func_AddChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_AddChannel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_AddChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_AddChannel ));
//    }
//
//    UCC_MEDIA_TYPES eMediaType;
//    {
//      //  CHookedMediaChannel::GetInstance()->Hook( PVOID( pChannel ) );
//        
//        pChannel->get_MediaType( &eMediaType );
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, pChannel );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_RemoveChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
///*[in]*/ struct IUccMediaChannel * pChannel ) 
//{
//    Func_RemoveChannel pOrgFunc = (Func_RemoveChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_RemoveChannel ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_RemoveChannel)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_RemoveChannel ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, pChannel );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_UpdateChannels (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
///*[in]*/ struct IUccOperationContext * pOperationContext ) 
//{
//    Func_UpdateChannels pOrgFunc = (Func_UpdateChannels)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_UpdateChannels ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_UpdateChannels)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_UpdateChannels ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, pOperationContext );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_get_Properties (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
///*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal )
//{
//    Func_get_Properties pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Properties ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_get_Properties)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_get_Properties ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall  CConfSessPartEndpoint::Hooked_SetProperty (
//    IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_ENDPOINT_PROPERTY enPropertyId,
//    /*[in]*/ VARIANT vPropertyValue,
///*[in]*/ struct IUccOperationContext * pOperationContext ) 
//{
//    Func_SetProperty pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_SetProperty ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pIUccConferenceSessionParticipantEndpoint );
//        pOrgFunc = (Func_SetProperty)(GetInstance()->GetOrgFunc( (void*)pIUccConferenceSessionParticipantEndpoint, Hooked_SetProperty ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pIUccConferenceSessionParticipantEndpoint, enPropertyId, vPropertyValue, pOperationContext );
//    }
//    return hr;
//}
//
//
