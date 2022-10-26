#pragma once
#include <Windows.h>
#include <map>
#include "HookBase.h"


class CInvokeTypedef
{
public:

    typedef HRESULT (__stdcall* IDispatchInvokeFunc )
        ( 
        PVOID pThis,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr
        ) ;

    /*typedef struct tagInvokeFunc
    {
        void* pVtable;
        IDispatchInvokeFunc poInvoke;
        IDispatchInvokeFunc pnInvoke;

        tagInvokeFunc():pVtable(0), poInvoke(0), pnInvoke(0){}
    }InvokeFunc; */    
};

class CSinkBase: public CInvokeTypedef, protected CHookBase
{
protected:
    CSinkBase() { /*InitializeCriticalSection(&m_cs);*/ }
    ~CSinkBase(){ /*DeleteCriticalSection(&m_cs);*/ } 

    //typedef std::map< PVOID, InvokeFunc > MapHooked;
    //typedef MapHooked::iterator MapHookedIt;

protected:/*

    MapHooked* GetMapHooked(){ return &m_mpHooked; }
    CRITICAL_SECTION* GetCS(){ return &m_cs; }*/

    HRESULT CallOriInvoke( PVOID pThis,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr );

public:
     void HookEvents( PVOID pEvents );

     static PVOID GetNthParam( DISPPARAMS *pDispParams, UINT nTh );

protected:
    IDispatchInvokeFunc m_pNewInvoke;  

    virtual bool RealNewInvoke( PVOID pThis,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr ) = 0;

private:    
    //MapHooked m_mpHooked;
    //CRITICAL_SECTION m_cs;
};

#define SINGLETONCLASSDECLARE( ClassName )\
class ClassName : public CSinkBase\
{\
    private:\
    ClassName()\
    {  m_pNewInvoke = NewHookedInvoke; }\
    ~##ClassName(){}\
    static ClassName* m_pInstance;\
public:\
    static ClassName* GetInstance()\
    {\
        if( !m_pInstance )\
        {\
            m_pInstance = new ClassName();\
        }\
        return m_pInstance;\
    }\
private:\
    virtual bool RealNewInvoke( PVOID pThis,\
 DISPID dispIdMember,\
 REFIID riid,\
 LCID lcid,\
 WORD wFlags,\
 DISPPARAMS *pDispParams,\
 VARIANT *pVarResult,\
 EXCEPINFO *pExcepInfo,\
 UINT *puArgErr );\
public:\
    static HRESULT __stdcall NewHookedInvoke(\
    PVOID pEvents,\
    DISPID dispIdMember,\
    REFIID riid,\
    LCID lcid,\
    WORD wFlags,\
    DISPPARAMS *pDispParams,\
    VARIANT *pVarResult,\
    EXCEPINFO *pExcepInfo,\
    UINT* puArgErr)\
    {\
        HRESULT hr = E_NOTIMPL;\
        if( GetInstance()->RealNewInvoke( pEvents,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr ) )\
            hr = GetInstance()->CallOriInvoke( pEvents, dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr );\
        return hr;\
    }\
}

#define SINGLETONCLASSDEFINITION( ClassName )  ClassName* ClassName::m_pInstance = 0
