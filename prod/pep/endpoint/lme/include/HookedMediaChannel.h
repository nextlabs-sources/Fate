#pragma once
//#include "HookBase.h"
//
//class CHookedMediaChannel:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedMediaChannel );
//public:
//    
//    void Hook( void* pMediaChannel );
//
//public:
//
//    typedef HRESULT (__stdcall *Func_get_MediaType)( IUccMediaChannel* pMediaChannel, UCC_MEDIA_TYPES* penMediaType );
//    static HRESULT __stdcall Hooked_get_MediaType( IUccMediaChannel* pMediaChannel, UCC_MEDIA_TYPES* penMediaType );
//
//    typedef HRESULT (__stdcall *Func_get_ActiveMedia) ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//    typedef HRESULT (__stdcall *Func_get_PreferredMedia) ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//    typedef HRESULT (__stdcall *Func_put_PreferredMedia) ( IUccMediaChannel* pMediaChannel, long plMediaDirections );
//    typedef HRESULT (__stdcall *Func_get_NegotiatedMedia) ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//
//    static HRESULT __stdcall Hooked_get_ActiveMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//    static HRESULT __stdcall Hooked_get_PreferredMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//    static HRESULT __stdcall Hooked_put_PreferredMedia ( IUccMediaChannel* pMediaChannel, long plMediaDirections );
//    static HRESULT __stdcall Hooked_get_NegotiatedMedia ( IUccMediaChannel* pMediaChannel, long * plMediaDirections );
//};