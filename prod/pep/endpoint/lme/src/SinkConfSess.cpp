#include "stdafx.h"
#include "SinkConfSess.h"

SINGLETONCLASSDEFINITION(CConfSessSink);

bool CConfSessSink::RealNewInvoke( 
    PVOID pEvents,
    /* [in] */ DISPID dispIdMember,
    /* [in] */ REFIID riid,
    /* [in] */ LCID lcid,
    /* [in] */ WORD wFlags,
    /* [out][in] */ DISPPARAMS *pDispParams,
    /* [out] */ VARIANT *pVarResult,
    /* [out] */ EXCEPINFO *pExcepInfo,
    /* [out] */ UINT *puArgErr)
{
	lcid ;
	pExcepInfo ;
	puArgErr ;
	pVarResult ;
	pDispParams ;
	wFlags ;
	riid ;
 //   _IUccConferenceSessionEvents* p__IUccConferenceSessionEvents  = (_IUccConferenceSessionEvents *)pEvents;

    switch( dispIdMember )
    {
    case 0x00000640://OnEnter
        {
            return true;
        }
        break;
        

    case 0x00000641://OnLeave
        {
            return true;
        }
        break;

    case 0x00000642://OnSetProperty
        {
            return true;
        }
        break;

    case 0x00000644://OnPropertyUpdated
        {
            return true;
        }
        break;
    }

    return true;
}