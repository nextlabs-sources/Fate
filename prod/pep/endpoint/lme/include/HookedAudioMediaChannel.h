#pragma once

//class CHookedAudioMediaChannel:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedAudioMediaChannel );
//public:
//
//    void Hook( void* pAudioMediaChannel );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_put_IsMuted )(
//        IUccAudioMediaChannel* pAudioMediaChannel,
//        /*[in]*/ enum UCC_MEDIA_DIRECTIONS enDirection,
//        /*[in]*/ VARIANT_BOOL pbIsMuted ) ;
//
//    static HRESULT __stdcall Hooked_put_IsMuted(
//        IUccAudioMediaChannel* pAudioMediaChannel,
//        /*[in]*/ enum UCC_MEDIA_DIRECTIONS enDirection,
//        /*[in]*/ VARIANT_BOOL pbIsMuted ) ;
//};