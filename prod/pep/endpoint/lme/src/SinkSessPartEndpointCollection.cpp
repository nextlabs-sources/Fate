#include "stdafx.h"
#include "SinkSessPartEndpointCollection.h"

//_IUccSessionParticipantEndpointCollectionEvents

SINGLETONCLASSDEFINITION( CSessPartEndpintCollectionSink );

bool CSessPartEndpintCollectionSink::RealNewInvoke( 
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
    case 0x000008fc: //OnEndpointAdded
        {
            OutputDebugString( TEXT("OnEndpointAdded") );
            IUccSessionParticipant* pIUccSessionParticipant = (IUccSessionParticipant*)GetNthParam( pDispParams, 2 );
            BSTR pstrUri = 0;
            pIUccSessionParticipant->get_Uri( &pstrUri );
            VARIANT_BOOL bIsLocal = FALSE;
            pIUccSessionParticipant->get_IsLocal( &bIsLocal );

            IUccContext* pContext = 0;
            pIUccSessionParticipant->get_Context( & pContext );
            ///pContext->get_Property( 

            CComPtr<IUccSession> pSession = 0;
            pIUccSessionParticipant->get_Session( &pSession );

            UCC_SESSION_TYPE eSessType;
            pSession->get_Type( &eSessType );



            IUccSessionParticipantEndpointCollectionEvent* pIUccSessionParticipantEndpointCollectionEvent 
                = (IUccSessionParticipantEndpointCollectionEvent *)GetNthParam( pDispParams, 1 ); 
            IUccSessionParticipantEndpoint* pEndPoint = 0;
            pIUccSessionParticipantEndpointCollectionEvent->get_Endpoint( &pEndPoint );
            BSTR pstrId = 0;
            pEndPoint->get_Id( &pstrId );
            pEndPoint->get_Uri( &pstrId );
            

            //CSessPartSink::GetInstance()->HookEvents( PVOID( pIUccSessionParticipant ) );
            //pIUccSessionParticipant->
            return true; 
        }
        break;

    case 0x000008fd://OnEndpointRemoved
        {
            return true;
        }
        break;
    }
    return true;
}