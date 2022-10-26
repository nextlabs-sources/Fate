#include "stdafx.h"
#include "SinkConfMediaChannelCollection.h"

SINGLETONCLASSDEFINITION( CConfMediaChannelCollection );

bool CConfMediaChannelCollection::RealNewInvoke( 
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
 //   _IUccConferenceMediaChannelCollectionEvents* p_IUccConferenceMediaChannelCollectionEvents  = (_IUccConferenceMediaChannelCollectionEvents *)pEvents;

    switch( dispIdMember )
    {
    case 0x00000898://OnChannelAdded
        {
            IUccMediaChannelCollectionEvent* //pIUccMediaChannelCollectionEvent = (IUccMediaChannelCollectionEvent*)GetNthParam( pDispParams, 2 );
            pIUccMediaChannelCollectionEvent = (IUccMediaChannelCollectionEvent*)GetNthParam( pDispParams, 1 );
            IUccMediaChannel* ppMediaChannel = 0;
            pIUccMediaChannelCollectionEvent->get_MediaChannel( &ppMediaChannel );
            UCC_MEDIA_TYPES eMediaTypes;
            (ppMediaChannel)->get_MediaType( &eMediaTypes );
            return true;
        }
        break;

    case 0x00000899://OnChannelRemoved
        {
            return true;
        }
        break;
    }

    return true;
}
