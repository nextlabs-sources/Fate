#pragma once
#include "Sink.h"


SINGLETONCLASSDECLARE( CConfSessPartSink );
//class CConfSessPartSink : public CSinkBase<_IUccConferenceSessionParticipantEvents>
//{
////private:
////    CConfSessPartSink()
////    { 
////        m_pmpHooked = GetMapHooked(); 
////    }
////    ~CConfSessPartSink(){}
////
////    static CConfSessPartSink* m_pInstance;
////
////    static MapHooked* m_pmpHooked;
////
////public:
////
////    static CConfSessPartSink* GetInstance()
////    {
////        if( !m_pInstance )
////        {
////            m_pInstance = new CConfSessPartSink();
////        }
////        return m_pInstance;
////    }
//
//    SINGLETONCLASSDECLARE(CConfSessPartSink)
//
//public:    
//
//    static void HookConferenceSessionParticipantEvents( _IUccConferenceSessionParticipantEvents* p_IUccConferenceSessionParticipantEvents );
//
//    static HRESULT __stdcall NewHookedInvoke( 
//        PVOID pThis,
//        /* [in] */ DISPID dispIdMember,
//        /* [in] */ REFIID riid,
//        /* [in] */ LCID lcid,
//        /* [in] */ WORD wFlags,
//        /* [out][in] */ DISPPARAMS *pDispParams,
//        /* [out] */ VARIANT *pVarResult,
//        /* [out] */ EXCEPINFO *pExcepInfo,
//        /* [out] */ UINT* puArgErr);
//};