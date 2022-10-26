#include "stdafx.h"
#include "SinkMediaChannel.h"

SINGLETONCLASSDEFINITION( CMediaChannelSink );

bool CMediaChannelSink::RealNewInvoke( 
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
            return true;
        }
        break;
    }

    return true;
}