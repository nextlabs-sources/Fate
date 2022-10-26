#pragma once

//class CHookedConfSess : public CHookBase
//{
//    INSTANCE_DECLARE( CHookedConfSess );
//public:
//
//    void Hook( void* pConfSess );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_get_Properties )( IUccConferenceSession* pConfSess,
//    /*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal );
//    typedef HRESULT (__stdcall* Func_Enter )( IUccConferenceSession* pConfSess,
//        /*[in]*/ BSTR bstrConfURI,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    typedef HRESULT (__stdcall* Func_Leave )( IUccConferenceSession* pConfSess,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    typedef HRESULT (__stdcall* Func_SetProperty )( IUccConferenceSession* pConfSess,
//        /*[in]*/ enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//
//    static HRESULT __stdcall Hooked_get_Properties ( IUccConferenceSession* pConfSess,
//    /*[out,retval]*/ struct IUccReadOnlyPropertyCollection * * pVal );
//    static HRESULT __stdcall Hooked_Enter ( IUccConferenceSession* pConfSess,
//        /*[in]*/ BSTR bstrConfURI,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    static HRESULT __stdcall Hooked_Leave ( IUccConferenceSession* pConfSess,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    static HRESULT __stdcall Hooked_SetProperty ( IUccConferenceSession* pConfSess,
//        /*[in]*/ enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
//        /*[in]*/ VARIANT vPropertyValue,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//};