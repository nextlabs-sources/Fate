#include "stdafx.h"
#include "Sink.h"
 
HRESULT CSinkBase:: CallOriInvoke( PVOID pThis,
                                         /* [in] */ DISPID dispIdMember,
                                         /* [in] */ REFIID riid,
                                         /* [in] */ LCID lcid,
                                         /* [in] */ WORD wFlags,
                                         /* [out][in] */ DISPPARAMS *pDispParams,
                                         /* [out] */ VARIANT *pVarResult,
                                         /* [out] */ EXCEPINFO *pExcepInfo,
                                         /* [out] */ UINT *puArgErr )
{    
    CLock aLock( &m_cs );
    
    IDispatchInvokeFunc OldInvoke = (IDispatchInvokeFunc)GetOrgFunc( pThis, 6 );

    if( OldInvoke )
    {
        return OldInvoke( pThis, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr );
    }

    return E_FAIL;
}

void CSinkBase::HookEvents( PVOID pEvents )
{
    if( !pEvents )
    {
        return;
    }  

    CLock aLock(&m_cs);

    SubstituteOrgFuncWithNew( pEvents, 6, (void*)m_pNewInvoke );

    DoHook( pEvents );

    return;
    
    /*if( m_mpHooked.find( pEvents ) != m_mpHooked.end() )
    {
        return;
    }

    InvokeFunc Functions;
    memset(&Functions,0,sizeof(InvokeFunc));
    PVOID* Vtable = (*(PVOID**)pEvents);	

    MapHookedIt it = m_mpHooked.begin();

    bool bHooked = false;
    for( ; it != m_mpHooked.end(); ++it )
    {
        if( Vtable == (*it).second.pVtable )
        {
            bHooked = true;
            Functions = (*it).second;
            break;
        }
    }

    if( !bHooked )
    {
        Functions.pVtable = Vtable;
        PVOID Pointer = Vtable[6];
        memcpy(&(Functions.pnInvoke),&Pointer,sizeof(ULONG));
        Functions.poInvoke = Functions.pnInvoke;

        gDetourTransactionBegin();
        gDetourUpdateThread(GetCurrentThread());

        gDetourAttach(&(PVOID&)Functions.pnInvoke, m_pNewInvoke );

        gDetourTransactionCommit();  
    } 

    m_mpHooked.insert(
        std::make_pair(pEvents,Functions));*/
}

//nTh start from 1
PVOID CSinkBase::GetNthParam( DISPPARAMS *pDispParams, UINT nTh )
{
    return (PVOID)( ( pDispParams->rgvarg[ nTh - 1 ] ).pintVal );
}


//bool CSinkBase::RealNewInvoke( 
//    PVOID pEvents,
//    /* [in] */ DISPID dispIdMember,
//    /* [in] */ REFIID riid,
//    /* [in] */ LCID lcid,
//    /* [in] */ WORD wFlags,
//    /* [out][in] */ DISPPARAMS *pDispParams,
//    /* [out] */ VARIANT *pVarResult,
//    /* [out] */ EXCEPINFO *pExcepInfo,
//    /* [out] */ UINT *puArgErr)
//{
//    switch( dispIdMember )
//    {
//    case 0x0000076d:
//        {
//
//        }
//        break;
//
//    case 0x0000076c:
//        {
//
//        }
//        break;
//    }
//    return true;
//}