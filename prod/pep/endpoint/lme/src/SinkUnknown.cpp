#include "stdafx.h"
#include "SinkUnknown.h"

SINGLETONCLASSDEFINITION( CUnknownSink );

bool CUnknownSink::RealNewInvoke( 
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
	pEvents ;
    dispIdMember;	
	riid;
	lcid	;
	wFlags	;
	pDispParams	  ;
	pVarResult	;
	pExcepInfo		;
	puArgErr		 ;
    return true;
}