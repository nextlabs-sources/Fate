#pragma once

#include "HookBase.h"

//class CHookedSignalingChannel:public CHookBase
//{
//    INSTANCE_DECLARE( CHookedSignalingChannel );
//public:
//
//    void Hook( void* pSignalingChannel );
//
//public:
//
//    typedef HRESULT (__stdcall* Func_SendRequest ) (
//        IUccSignalingChannel* pSignalingChannel,
//    /*[in]*/ struct IUccSignalingRequest * pRequest,
//    /*[in]*/ struct IUccOperationContext * pOpContext );
//
//    static HRESULT __stdcall Hooked_SendRequest (
//        IUccSignalingChannel* pSignalingChannel,
//    /*[in]*/ struct IUccSignalingRequest * pRequest,
//    /*[in]*/ struct IUccOperationContext * pOpContext );
//};