#pragma once
//
//class CHookedConfSessPart:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedConfSessPart );
//public:
//
//    void Hook( void* pIUccConferenceSessionParticipant );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_CreateParticipantEndpoint)( IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant, /*[in]*/ BSTR bstrEndpointUri,
//        /*[in]*/ BSTR bstrEndpointId,
//        /*[in]*/ enum UCC_CONFERENCE_ENTITY_TYPE enType,
//    /*[in]*/ struct IUccContext * pContext,
//    /*[out,retval]*/ struct IUccConferenceSessionParticipantEndpoint * * ppParticipantEndpoint );
//
//    static HRESULT __stdcall Hooked_CreateParticipantEndpoint( IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant, /*[in]*/ BSTR bstrEndpointUri,
//        /*[in]*/ BSTR bstrEndpointId,
//        /*[in]*/ enum UCC_CONFERENCE_ENTITY_TYPE enType,
//    /*[in]*/ struct IUccContext * pContext,
//    /*[out,retval]*/ struct IUccConferenceSessionParticipantEndpoint * * ppParticipantEndpoint );
//
//    typedef HRESULT (__stdcall* Func_SetProperty) (
//        IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant,
//        /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//
//    static HRESULT __stdcall Hooked_SetProperty (
//        IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant,
//        /*[in]*/ enum UCC_CONFERENCE_PARTICIPANT_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//
//    
//};