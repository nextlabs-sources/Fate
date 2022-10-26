#include "stdafx.h"
#include "SinkConfEntityViewCollectionEvents.h"

SINGLETONCLASSDEFINITION( CConfEntityViewCollectionSink );
//_IUccConferenceEntityViewCollectionEvents

bool CConfEntityViewCollectionSink::RealNewInvoke( 
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
	lcid ;
	puArgErr ;
	pVarResult ;
	wFlags ;
	riid ;
	pExcepInfo ;
    switch( dispIdMember )
    {
    case 0x00001388://OnEntityViewAdded
        {
            return true;
        }
        break;

    case 0x00001389://OnEntityViewRemoved
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