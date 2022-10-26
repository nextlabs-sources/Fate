#pragma once
#include "HookBase.h"
//
//
//class CConfSessPartEndpoint:public CHookBase
//{
//    INSTANCE_DECLARE( CConfSessPartEndpoint );
//public:
//
//    void Hook( void* pIUccConferenceSessionParticipantEndpoint );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_get_State )(IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[out,retval]*/ enum UCC_SESSION_ENTITY_STATE * penState );
//    typedef HRESULT (__stdcall* Func_get_Type )(IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[out,retval]*/ enum UCC_CONFERENCE_ENTITY_TYPE * penType );
//    typedef HRESULT (__stdcall* Func_get_Channels )(IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[out,retval]*/ struct IUccCollection * * ppChannels ) ;
//    typedef HRESULT (__stdcall* Func_CreateChannel) (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[in]*/ enum UCC_MEDIA_TYPES enMediaType,
//    /*[in]*/ struct IUccContext * pContext,
//    /*[out,retval]*/ struct IUccMediaChannel * * ppChannel );
//    typedef HRESULT (__stdcall* Func_AddChannel) (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccMediaChannel * pChannel ) ;
//    typedef HRESULT (__stdcall* Func_RemoveChannel )(IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccMediaChannel * pChannel ) ;
//    typedef HRESULT (__stdcall* Func_UpdateChannels) (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccOperationContext * pOperationContext ) ;
//    typedef HRESULT (__stdcall* Func_get_Properties) (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal );
//    typedef HRESULT (__stdcall* Func_SetProperty )(IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_ENDPOINT_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext ) ;
//
//    static HRESULT __stdcall Hooked_get_State (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[out,retval]*/ enum UCC_SESSION_ENTITY_STATE * penState );
//    static HRESULT __stdcall Hooked_get_Type (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[out,retval]*/ enum UCC_CONFERENCE_ENTITY_TYPE * penType );
//    static HRESULT __stdcall Hooked_get_Channels (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[out,retval]*/ struct IUccCollection * * ppChannels ) ;
//    static HRESULT __stdcall Hooked_CreateChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[in]*/ enum UCC_MEDIA_TYPES enMediaType,
//    /*[in]*/ struct IUccContext * pContext,
//    /*[out,retval]*/ struct IUccMediaChannel * * ppChannel );
//    static HRESULT __stdcall Hooked_AddChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccMediaChannel * pChannel ) ;
//    static HRESULT __stdcall Hooked_RemoveChannel (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccMediaChannel * pChannel ) ;
//    static HRESULT __stdcall Hooked_UpdateChannels (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[in]*/ struct IUccOperationContext * pOperationContext ) ;
//    static HRESULT __stdcall Hooked_get_Properties (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//    /*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal );
//    static HRESULT __stdcall Hooked_SetProperty (IUccConferenceSessionParticipantEndpoint* pIUccConferenceSessionParticipantEndpoint, 
//        /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_ENDPOINT_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext ) ;
//};