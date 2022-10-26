#include "stdafx.h"
#include "SinkSess.h"

SINGLETONCLASSDEFINITION( CSessSink );

bool CSessSink::RealNewInvoke( 
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
    case 0x00000190://OnAddParticipant
        {
            OutputDebugString( TEXT("OnAddParticipant") );
            return true;
        }
        break;


    case 0x00000191://OnRemoveParticipant
        {
            return true;
        }
        break;

    case 0x00000194://OnTerminate
        {
            return true;
        }
        break;
    }


    return true;
}