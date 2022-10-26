#include "stdafx.h"
//#include "HookedSignalingMessage.h"
//
//
//INSTANCE_DEFINE( CHookedSignalingMessage );
//
//
//void CHookedSignalingMessage::Hook( void* pSignalingMsg )
//{
//    SubstituteOrgFuncWithNew( pSignalingMsg, 3, (void*)Hooked_get_Type );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 4, (void*)Hooked_get_Headers );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 5, (void*)Hooked_get_Body );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 6, (void*)Hooked_put_Body );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 7, (void*)Hooked_get_ContentType );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 8, (void*)Hooked_put_ContentType );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 9, (void*)Hooked_AddHeader );
//    SubstituteOrgFuncWithNew( pSignalingMsg, 10, (void*)Hooked_RemoveHeader );;
//    SubstituteOrgFuncWithNew( pSignalingMsg, 11, (void*)Hooked_FindHeaders );
//    DoHook( pSignalingMsg );
//}
//
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_get_Type (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[out,retval]*/ enum UCC_SIGNALING_MESSAGE_TYPE * pVal )
//{
//    Func_get_Type pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Type ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_get_Type)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Type ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_get_Headers (
//    IUccSignalingMessage* pSignalingMsg,
///*[out,retval]*/ struct IUccCollection * * pVal ) 
//{
//    Func_get_Headers pOrgFunc = (Func_get_Headers)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Headers ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_get_Headers)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Headers ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_get_Body (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[out,retval]*/ BSTR * pVal )
//{
//    Func_get_Body pOrgFunc = (Func_get_Body)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Body ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_get_Body)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_Body ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_put_Body (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ BSTR pVal ) 
//{
//    Func_put_Body pOrgFunc = (Func_put_Body)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_put_Body ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_put_Body)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_put_Body ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pVal );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_get_ContentType (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[out,retval]*/ BSTR * pContentType ) 
//{
//    Func_get_ContentType pOrgFunc = (Func_get_ContentType)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_ContentType ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_get_ContentType)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_get_ContentType ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pContentType );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_put_ContentType (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ BSTR pContentType ) 
//{
//    Func_put_ContentType pOrgFunc = (Func_put_ContentType)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_put_ContentType ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_put_ContentType)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_put_ContentType ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pContentType );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_AddHeader (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ BSTR bstrName,
//    /*[in]*/ BSTR bstrValue,
///*[out,retval]*/ struct IUccSignalingHeader * * ppHeader ) 
//{
//    Func_AddHeader pOrgFunc = (Func_AddHeader)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_AddHeader ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_AddHeader)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_AddHeader ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, bstrName, bstrValue, ppHeader );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_RemoveHeader (
//    IUccSignalingMessage* pSignalingMsg,
///*[in]*/ struct IUccSignalingHeader * pHeader )
//{
//    Func_RemoveHeader pOrgFunc = (Func_RemoveHeader)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_RemoveHeader ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_RemoveHeader)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_RemoveHeader ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, pHeader );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSignalingMessage::Hooked_FindHeaders (
//    IUccSignalingMessage* pSignalingMsg,
//    /*[in]*/ BSTR bstrName,
///*[out,retval]*/ struct IUccCollection * * ppHeaders )
//{
//    Func_FindHeaders pOrgFunc = (Func_FindHeaders)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_FindHeaders ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pSignalingMsg );
//        pOrgFunc = (Func_FindHeaders)(GetInstance()->GetOrgFunc( (void*)pSignalingMsg, Hooked_FindHeaders ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pSignalingMsg, bstrName, ppHeaders );
//    }
//    return hr;
//}