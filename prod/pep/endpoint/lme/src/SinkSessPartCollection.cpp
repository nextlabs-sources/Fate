#include "stdafx.h"
#include "SinkSessPartCollection.h"
#include "PartDB.h"

SINGLETONCLASSDEFINITION( CSessPartCollectionSink );

bool CSessPartCollectionSink::RealNewInvoke( 
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
	pEvents	;
riid		;	 
lcid	;
wFlags		;
pDispParams ;
pVarResult	;
pExcepInfo		;
puArgErr   ;

    switch( dispIdMember )
    {
    case 0x00000834://OnParticipantAdded
        {
            OutputDebugString( TEXT("OnParticipantAdded") );
            return true;
        }
        break;
    case 0x00000835://OnParticipantRemoved
        {
            IUccSessionParticipantCollectionEvent* pEventData = (IUccSessionParticipantCollectionEvent*)GetNthParam( pDispParams, 1 );
            CComPtr<IUccSessionParticipant> pSessPart = 0;
            pEventData->get_Participant( &pSessPart );

            CPartDB::GetInstance()->RemoveNonLocalPart( pSessPart );
            ////HRESULT OnParticipantRemoved(
            //[in] IUccSession* pEventSource, 
            //    [in] IUccSessionParticipantCollectionEvent* pEventData);

            return true;
        }
        break;
    }
    return true;
}