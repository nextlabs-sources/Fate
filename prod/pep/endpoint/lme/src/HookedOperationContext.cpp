#include "stdafx.h"
#include "HookedOperationContext.h"

//INSTANCE_DEFINE( CHookedOperationContext );
//
//
//void CHookedOperationContext::Hook( void* pOperationContext )
//{
//    SubstituteOrgFuncWithNew( pOperationContext, 3, (void*)Hooked_Initialize );
//    SubstituteOrgFuncWithNew( pOperationContext, 4, (void*)Hooked_get_OperationId );
//    //SubstituteOrgFuncWithNew( pOperationContext, 5, (void*)Hooked_get_Context );
//    DoHook( pOperationContext );
//}
//
//HRESULT __stdcall CHookedOperationContext::Hooked_Initialize (
//    IUccOperationContext* pOperationContext,
//    /*[in]*/ long lOperationId,
///*[in]*/ struct IUccContext * pOperation )
//{
//    Func_Initialize pOrgFunc = (Func_Initialize)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_Initialize ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pOperationContext );
//        pOrgFunc = (Func_Initialize)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_Initialize ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pOperationContext, lOperationId, pOperation );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedOperationContext::Hooked_get_OperationId (
//    IUccOperationContext* pOperationContext,
//    /*[out,retval]*/ long * plOperationId )
//{
//    Func_get_OperationId pOrgFunc = (Func_get_OperationId)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_get_OperationId ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pOperationContext );
//        pOrgFunc = (Func_get_OperationId)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_get_OperationId ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pOperationContext, plOperationId );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedOperationContext::Hooked_get_Context (
//    IUccOperationContext* pOperationContext,
///*[out,retval]*/ struct IUccContext * * ppContext )
//{
//    Func_get_Context pOrgFunc = (Func_get_Context)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_get_Context ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( pOperationContext );
//        pOrgFunc = (Func_get_Context)(GetInstance()->GetOrgFunc( (void*)pOperationContext, Hooked_get_Context ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( pOperationContext, ppContext );
//    }
//    return hr;
//}