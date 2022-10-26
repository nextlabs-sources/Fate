#pragma once
#include "HookBase.h"
//
//class CHookedSignalingMessage:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedSignalingMessage );
//public:
//
//    void Hook( void* pSignalingMsg );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_get_Type  )(
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ enum UCC_SIGNALING_MESSAGE_TYPE * pVal );
//
//    typedef HRESULT (__stdcall* Func_get_Headers  )(
//        IUccSignalingMessage* pSignalingMsg,
//    /*[out,retval]*/ struct IUccCollection * * pVal ) ;
//
//    typedef HRESULT (__stdcall* Func_get_Body ) (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ BSTR * pVal );
//
//    typedef HRESULT (__stdcall* Func_put_Body  )(
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR pVal ) ;
//
//    typedef HRESULT (__stdcall* Func_get_ContentType ) (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ BSTR * pContentType ) ;
//
//    typedef HRESULT (__stdcall* Func_put_ContentType  )(
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR pContentType ) ;
//
//    typedef HRESULT (__stdcall* Func_AddHeader ) (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR bstrName,
//        /*[in]*/ BSTR bstrValue,
//    /*[out,retval]*/ struct IUccSignalingHeader * * ppHeader ) ;
//
//    typedef HRESULT (__stdcall* Func_RemoveHeader  )(
//        IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ struct IUccSignalingHeader * pHeader );
//
//    typedef HRESULT (__stdcall* Func_FindHeaders )(
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR bstrName,
//    /*[out,retval]*/ struct IUccCollection * * ppHeaders );
//
//
//
//
//    static HRESULT __stdcall Hooked_get_Type (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ enum UCC_SIGNALING_MESSAGE_TYPE * pVal );
//
//    static HRESULT __stdcall Hooked_get_Headers (
//        IUccSignalingMessage* pSignalingMsg,
//    /*[out,retval]*/ struct IUccCollection * * pVal ) ;
//
//    static HRESULT __stdcall Hooked_get_Body (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ BSTR * pVal );
//
//    static HRESULT __stdcall Hooked_put_Body (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR pVal ) ;
//
//    static HRESULT __stdcall Hooked_get_ContentType (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[out,retval]*/ BSTR * pContentType ) ;
//
//    static HRESULT __stdcall Hooked_put_ContentType (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR pContentType ) ;
//
//    static HRESULT __stdcall Hooked_AddHeader (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR bstrName,
//        /*[in]*/ BSTR bstrValue,
//    /*[out,retval]*/ struct IUccSignalingHeader * * ppHeader ) ;
//
//    static HRESULT __stdcall Hooked_RemoveHeader (
//        IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ struct IUccSignalingHeader * pHeader );
//
//    static HRESULT __stdcall Hooked_FindHeaders (
//        IUccSignalingMessage* pSignalingMsg,
//        /*[in]*/ BSTR bstrName,
//    /*[out,retval]*/ struct IUccCollection * * ppHeaders );
//};