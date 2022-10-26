#include "stdafx.h"
//#include "HookedAudioMediaChannel.h"
//
//INSTANCE_DEFINE( CHookedAudioMediaChannel );
//
//void CHookedAudioMediaChannel::Hook( void* pAudioMediaChannel )
//{
//    SubstituteOrgFuncWithNew( pAudioMediaChannel, 6, (void*)Hooked_put_IsMuted );
//    DoHook( pAudioMediaChannel );
//}
//
//
//HRESULT __stdcall CHookedAudioMediaChannel::Hooked_put_IsMuted(
//                                     IUccAudioMediaChannel* pAudioMediaChannel,
//                                     /*[in]*/ enum UCC_MEDIA_DIRECTIONS enDirection,
//                                     /*[in]*/ VARIANT_BOOL pbIsMuted ) 
//{
//    Func_put_IsMuted pOrgFunc = (Func_put_IsMuted)(GetInstance()->GetOrgFunc( (void*)pAudioMediaChannel, Hooked_put_IsMuted ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pAudioMediaChannel );
//        pOrgFunc = (Func_put_IsMuted)(GetInstance()->GetOrgFunc( (void*)pAudioMediaChannel, Hooked_put_IsMuted ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pAudioMediaChannel, enDirection, pbIsMuted );
//    }
//    return hr;
//}