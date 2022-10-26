#include "stdafx.h"
#include "SinkConfEntityView.h"

SINGLETONCLASSDEFINITION( CConfEntityViewSink );

//_IUccConferenceEntityViewEvents 
bool CConfEntityViewSink::RealNewInvoke( 
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
    case 0x000013ec://OnPropertyUpdated
        {
            return true;
        }
        break;
    }
    return true;
}