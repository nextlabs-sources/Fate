#include "stdafx.h"
#include "SinkConfMediaChannel.h"

SINGLETONCLASSDEFINITION( CConfMediaChannel );
//_IUccConferenceMediaChannelEvents 
bool CConfMediaChannel::RealNewInvoke( 
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
		pEvents	  ;
	riid		;
	lcid		;
	wFlags	;
	pDispParams	;
	pVarResult	;
	pExcepInfo	;
	puArgErr  ;
    switch( dispIdMember )
    {
    case 0x00000515://OnNegotiatedMediaChanged
        {
            return true;
        }
        break;

    case 0x00000516://OnMediaDeviceChanged
        {
            IUccPropertyUpdateEvent* pIUccPropertyUpdateEvent = (IUccPropertyUpdateEvent*)GetNthParam( pDispParams, 1 );
            LONG Val = 0;
            pIUccPropertyUpdateEvent->get_PropertyId( &Val );
            return true;
        }
        break;
    }
    return true;
}