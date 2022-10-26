#pragma once
//#include "HookBase.h"
//
//class CHookedOperationContext: public CHookBase
//{
//    INSTANCE_DECLARE( CHookedOperationContext );
//public:
//
//    void Hook( void* pOperationContext );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_Initialize )(
//        IUccOperationContext* pOperationContext,
//        /*[in]*/ long lOperationId,
//    /*[in]*/ struct IUccContext * pOperation ) ;
//    typedef HRESULT (__stdcall* Func_get_OperationId )(
//        IUccOperationContext* pOperationContext,
//        /*[out,retval]*/ long * plOperationId );
//    typedef HRESULT (__stdcall* Func_get_Context )(
//        IUccOperationContext* pOperationContext,
//    /*[out,retval]*/ struct IUccContext * * ppContext );
//
//    static HRESULT __stdcall Hooked_Initialize (
//        IUccOperationContext* pOperationContext,
//        /*[in]*/ long lOperationId,
//    /*[in]*/ struct IUccContext * pOperation );
//    static HRESULT __stdcall Hooked_get_OperationId (
//        IUccOperationContext* pOperationContext,
//        /*[out,retval]*/ long * plOperationId );
//    static HRESULT __stdcall Hooked_get_Context (
//        IUccOperationContext* pOperationContext,
//    /*[out,retval]*/ struct IUccContext * * ppContext );
//};