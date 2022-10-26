#include "stdafx.h"
//#include "HookedSignalingChannel.h"
//
//INSTANCE_DEFINE( CHookedSignalingChannel );
//
//
//void CHookedSignalingChannel::Hook( void* pSignalingChannel )
//{
//    char* pTmp = (char*)pSignalingChannel + 4;
//    //SubstituteOrgFuncWithNew( pSignalingChannel, 3, (void*)Hooked_SendRequest );
//    //SubstituteOrgFuncWithNew( pSignalingChannel, 8, (void*)Hooked_SendRequest );
//    SubstituteOrgFuncWithNew( pSignalingChannel, 3, (void*)Hooked_SendRequest );    
//    DoHook( pSignalingChannel );
//
//    SubstituteOrgFuncWithNew( (void*)pTmp, 3, (void*)Hooked_SendRequest );
//    DoHook( (void*)pTmp );
//}
//
//HRESULT __stdcall CHookedSignalingChannel::Hooked_SendRequest (
//    IUccSignalingChannel* pSignalingChannel,
///*[in]*/ struct IUccSignalingRequest * pRequest,
///*[in]*/ struct IUccOperationContext * pOpContext )
//{
//    Func_SendRequest pOrgFunc = (Func_SendRequest)(GetInstance()->GetOrgFunc( (void*)pSignalingChannel, Hooked_SendRequest ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingChannel );
//        pOrgFunc = (Func_SendRequest)(GetInstance()->GetOrgFunc( (void*)pSignalingChannel, Hooked_SendRequest ));
//    }
//
//    /*if( !IsExecute( TEXT("CHookedSignalingChannel::Hooked_SendRequest") ) )
//    {
//        pOrgFunc = 0;
//    }*/
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingChannel, pRequest, pOpContext );
//    }
//    return hr;
//}