#include "stdafx.h"
#include "SinkSessPart.h"

//_IUccSessionParticipantEvents
SINGLETONCLASSDEFINITION( CSessPartSink );
 
bool CSessPartSink::RealNewInvoke( 
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
    case 0x000001f4://OnStateChanged
        {
            return true;
        }
        break;
    case 0x000001f5://OnAddParticipantEndpoint
        {
            return true;
        }
        break;

    case 0x0000076c://OnRemoveParticipantEndpoint
        {
            return true; 
        }
        break;
    }
    return true;
}