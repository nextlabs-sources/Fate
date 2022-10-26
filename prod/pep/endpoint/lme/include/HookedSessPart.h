#pragma once
#include "HookBase.h"

//class CHookedSessPart: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedSessPart );
//public:
//
//    void Hook( void* pSessPart );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_AddParticipantEndpoint) (IUccSessionParticipant* pSessPart,
//    /*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    typedef HRESULT (__stdcall* Func_RemoveParticipantEndpoint) (IUccSessionParticipant* pSessPart,
//    /*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//
//    static HRESULT __stdcall Hooked_AddParticipantEndpoint (IUccSessionParticipant* pSessPart,
//    /*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//    static HRESULT __stdcall Hooked_RemoveParticipantEndpoint (IUccSessionParticipant* pSessPart,
//    /*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
//    /*[in]*/ struct IUccOperationContext * pOperationContext );
//};