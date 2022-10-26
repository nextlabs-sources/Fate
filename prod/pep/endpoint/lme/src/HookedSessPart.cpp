#include "stdafx.h"
#include "HookedSessPart.h"

//INSTANCE_DEFINE( CHookedSessPart );
//
//
//void CHookedSessPart::Hook( void* pSessPart )
//{
//    SubstituteOrgFuncWithNew( pSessPart, 11, (void*)Hooked_AddParticipantEndpoint );
//    SubstituteOrgFuncWithNew( pSessPart, 12, (void*)Hooked_RemoveParticipantEndpoint );
//    DoHook( pSessPart );
//}
//
//HRESULT __stdcall CHookedSessPart::Hooked_AddParticipantEndpoint (IUccSessionParticipant* pSessPart,
///*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    OutputDebugString(L"CHookedSessPart::Hooked_AddParticipantEndpoint");
//    Func_AddParticipantEndpoint pOrgFunc = (Func_AddParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pSessPart, Hooked_AddParticipantEndpoint ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSessPart );
//        pOrgFunc = (Func_AddParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pSessPart, Hooked_AddParticipantEndpoint ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSessPart, pParticipantEndpoint, pOperationContext );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSessPart::Hooked_RemoveParticipantEndpoint (IUccSessionParticipant* pSessPart,
///*[in]*/ struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
///*[in]*/ struct IUccOperationContext * pOperationContext )
//{
//    Func_RemoveParticipantEndpoint pOrgFunc = (Func_RemoveParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pSessPart, Hooked_AddParticipantEndpoint ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSessPart );
//        pOrgFunc = (Func_RemoveParticipantEndpoint)(GetInstance()->GetOrgFunc( (void*)pSessPart, Hooked_AddParticipantEndpoint ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSessPart, pParticipantEndpoint, pOperationContext );
//    }
//    return hr;
//}
