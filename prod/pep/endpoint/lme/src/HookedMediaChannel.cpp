#include "stdafx.h"
#include "HookedMediaChannel.h"
//#include "HookedAudioMediaChannel.h"
//
//INSTANCE_DEFINE( CHookedMediaChannel );
//
//
//void CHookedMediaChannel::Hook( void* pMediaChannel )
//{
//    SubstituteOrgFuncWithNew( pMediaChannel, 3, (void*)Hooked_get_MediaType );
//    SubstituteOrgFuncWithNew( pMediaChannel, 4, (void*)Hooked_get_ActiveMedia );
//    SubstituteOrgFuncWithNew( pMediaChannel, 5, (void*)Hooked_get_PreferredMedia );
//    SubstituteOrgFuncWithNew( pMediaChannel, 6, (void*)Hooked_put_PreferredMedia );
//    SubstituteOrgFuncWithNew( pMediaChannel, 7, (void*)Hooked_get_NegotiatedMedia );
//    DoHook( pMediaChannel );
//
//    //IUccAudioMediaChannel* pAudioMediaChannel = 0;
//    //if( S_OK == ((IUccMediaChannel*)pMediaChannel)->QueryInterface( IID_IUccAudioMediaChannel, (void**)&pAudioMediaChannel ) )
//    //{
//    //    CHookedAudioMediaChannel::GetInstance()->Hook( PVOID(pAudioMediaChannel) );
//    //}
//}
//
//HRESULT __stdcall CHookedMediaChannel::Hooked_get_MediaType( IUccMediaChannel* pMediaChannel, UCC_MEDIA_TYPES* penMediaType )
//{    
//    Func_get_MediaType pOrgFunc = (Func_get_MediaType)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_MediaType ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pMediaChannel );
//        pOrgFunc = (Func_get_MediaType)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_MediaType ));
//    }
//    
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pMediaChannel, penMediaType );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedMediaChannel::Hooked_get_ActiveMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections )
//{
//    Func_get_ActiveMedia pOrgFunc = (Func_get_ActiveMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_ActiveMedia ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pMediaChannel );
//        pOrgFunc = (Func_get_ActiveMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_ActiveMedia ));
//    }
//
//    UCC_MEDIA_TYPES eMediaType;
//    pMediaChannel->get_MediaType( &eMediaType );
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pMediaChannel, plMediaDirections );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedMediaChannel::Hooked_get_PreferredMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections )
//{
//    Func_get_PreferredMedia pOrgFunc = (Func_get_PreferredMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_PreferredMedia ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pMediaChannel );
//        pOrgFunc = (Func_get_PreferredMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_PreferredMedia ));
//    }
//
//    UCC_MEDIA_TYPES eMediaType;
//    pMediaChannel->get_MediaType( &eMediaType );
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pMediaChannel, plMediaDirections );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedMediaChannel::Hooked_put_PreferredMedia ( IUccMediaChannel* pMediaChannel, long plMediaDirections )
//{
//    Func_put_PreferredMedia pOrgFunc = (Func_put_PreferredMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_put_PreferredMedia ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pMediaChannel );
//        pOrgFunc = (Func_put_PreferredMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_put_PreferredMedia ));
//    }
//
//    UCC_MEDIA_TYPES eMediaType;
//    pMediaChannel->get_MediaType( &eMediaType );
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pMediaChannel, plMediaDirections );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedMediaChannel::Hooked_get_NegotiatedMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections )
//{
//    Func_get_NegotiatedMedia pOrgFunc = (Func_get_NegotiatedMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_NegotiatedMedia ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pMediaChannel );
//        pOrgFunc = (Func_get_NegotiatedMedia)(GetInstance()->GetOrgFunc( (void*)pMediaChannel, Hooked_get_NegotiatedMedia ));
//    }
//
//    UCC_MEDIA_TYPES eMediaType;
//    pMediaChannel->get_MediaType( &eMediaType );
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pMediaChannel, plMediaDirections );
//    }
//    return hr;
//}
