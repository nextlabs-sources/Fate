#include "stdafx.h"
#include "SinkConfSessPart.h"
#include "PartDB.h"

//CConfSessPartSink* CConfSessPartSink::m_pInstance = 0;
//CSinkBase<_IUccConferenceSessionParticipantEvents>::MapHooked* CConfSessPartSink::m_pmpHooked = 0;
SINGLETONCLASSDEFINITION( CConfSessPartSink );

bool CConfSessPartSink::RealNewInvoke( 
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
//    _IUccConferenceSessionParticipantEvents* p_IUccConferenceSessionParticipantEvents = (_IUccConferenceSessionParticipantEvents*)pEvents;
    switch( dispIdMember )
    {
    case 0x000006a4:  //OnSetProperty
        {
            //ShowInfo( TEXT("NewConferenceSessionParticipantEvents_OnSetProperty called." ) );
            OutputDebugString( TEXT("OnSetProperty" ) );
            return true;
        }
        break;
    case 0x000006a5: //OnPropertyUpdated
        {
            OutputDebugString( TEXT("OnPropertyUpdated") );
            //ShowInfo( TEXT("NewConferenceSessionParticipantEvents_OnPropertyUpdated called." ) );

            CParticipant aParticipant;

      //      IUnknown* pUnk = *((pDispParams->rgvarg)->ppunkVal);
            IUccConferenceSessionParticipant* pPart = 0;
            //HRESULT hr1 = pUnk->QueryInterface( DIID__IUccConferenceSessionParticipantEndpointEvents, (void**)&pPart );
            pPart = (IUccConferenceSessionParticipant*)GetNthParam( pDispParams, 2 );

            IUccReadOnlyPropertyCollection* pCollection = 0;
            pPart->get_Properties( &pCollection );

            IUccProperty* pProperty1 = 0;
            pCollection->get_Property( UCCCPPID_DISPLAY_NAME, &pProperty1 );
            BSTR pDisplayName = 0;
            pProperty1->get_StringValue( &pDisplayName );

            std::wstring strResult(pDisplayName);

            strResult += TEXT( "\n Role: " );            

            CComPtr<IUccSessionParticipant> pSessPart = 0;
            HRESULT hr1 = pPart->QueryInterface( IID_IUccSessionParticipant, (void**)&pSessPart );
            aParticipant.SetPart( pSessPart );
            //IUccConferenceSessionParticipant* pPart = (pDispParams->rgvarg)->ppunkVal;//sizeof( IUccPropertyUpdateEvent ) );//((IUccConferenceSessionParticipant*)pDispParams+1);
            IUccReadOnlyPropertyCollection* pPropertyCollection = 0;
            pPart->get_Properties( &pPropertyCollection );
            IUccProperty* pProperty = 0;
            BSTR pName = 0;//new OLECHAR[1024];

            //HRESULT hr1 = S_FALSE;
            long lId = 0;
            long lVal = 0;
            hr1 = pPropertyCollection->get_Property( UCCCPPID_ROLE , &pProperty ); 
            hr1 = pProperty->get_Id( &lId );
            hr1 = pProperty->get_NumericValue( &lVal );
            UCC_CONFERENCE_PARTICIPANT_ROLE eRole = UCC_CONFERENCE_PARTICIPANT_ROLE(lVal);

            switch( eRole )
            {
            case UCCCPR_PARTICIPANT: 
                {
                    strResult += TEXT( "UCCCPR_PARTICIPANT\n" );
                    aParticipant.SetRole( TEXT("Participant") );
                }
                break;
            case UCCCPR_ADMIN:
                {
                    strResult += TEXT( "UCCCPR_ADMIN\n" );
                    aParticipant.SetRole( TEXT("Admin      ") );
                }
                break;
            case UCCCPR_UNDEFINED: 
                {
                    strResult += TEXT( "UCCCPR_UNDEFINED\n" ); 
                    aParticipant.SetRole( TEXT("Undefined Role") );
                }
                break;
            }

            
            //strResult += pName;
            strResult += TEXT(" URI: ");
            hr1 = pPropertyCollection->get_Property( UCCCPEPID_ENDPOINT_URI , &pProperty ); 
            //hr1 = pProperty->get_Id( &lId );
            hr1 = pProperty->get_StringValue( &pName );
            
            if( wcsncmp( pName, TEXT("sip:"), 4 ) == 0 )
            {
                aParticipant.SetSIP( pName+4 );
            }
            else
            {
                aParticipant.SetSIP( pName );
            }
            aParticipant.SetName( pDisplayName );
            VARIANT_BOOL bIsLocal = false;
            pSessPart->get_IsLocal( &bIsLocal ) ;
            if( bIsLocal )
            {                
                CPartDB::GetInstance()->SetLocalPart( aParticipant );
				//CE_ACTION_WM_JOIN
                if( !DoEvaluate(LME_MAGIC_STRING,L"JOIN") )
                {
                    DWORD     id=   ::GetCurrentProcessId(); 
                    HANDLE   hProc = ::OpenProcess(   PROCESS_ALL_ACCESS,   TRUE,id ); 
                    
                    //ExitProcess(1234);
                    TerminateProcess( hProc, 0x7200);
                    
                    return true;
                }
            }
            else
            {          
                CPartDB::GetInstance()->AddNonLocalPart( aParticipant );

                if( !CPartDB::GetInstance()->GetLocalPart().GetName().empty() )
                {
					//CE_ACTION_WM_JOIN
                    if( !DoEvaluate(LME_MAGIC_STRING,L"JOIN") )
                    {
                        DWORD     id=   ::GetCurrentProcessId(); 
                        HANDLE   hProc = ::OpenProcess(   PROCESS_ALL_ACCESS,   TRUE,id ); 
                        TerminateProcess( hProc, 0x7200);
                        return false;
                    }
                }
            }
            strResult.append( pName, wcslen( pName ) );
            /*hr1 = pPropertyCollection->get_Property( UCCCPEPID_ROLE , &pProperty ); 
            hr1 = pProperty->get_Id( &lId );
            hr1 = pProperty->get_StringValue( &pName );
            strResult += pName;
            hr1 = pPropertyCollection->get_Property( UCCCSPID_CONFERENCE_DATA_URI , &pProperty ); 
            hr1 = pProperty->get_Id( &lId );
            hr1 = pProperty->get_StringValue( &pName );
            strResult += pName;
            hr1 = pPropertyCollection->get_Property( UCCCPPID_ROLE , &pProperty );  */
            /*HRESULT hr1 = pProperty->get_Name( &pName );
            strResult += pName;

            pPropertyCollection->get_Property( UCCCPPID_DISPLAY_NAME , &pProperty );        
            hr1 = pProperty->get_Name( &pName );
            strResult += TEXT(" Name: ");
            strResult += pName;

            pPropertyCollection->get_Property( UCCCPEPID_ENDPOINT_URI , &pProperty );        
            hr1 = pProperty->get_Name( &pName );
            strResult += TEXT(" UCCCPEPID_ENDPOINT_URI: ");
            strResult += pName;*/

            //ShowInfo(strResult.c_str());
            //DemoShowInfo( strResult.c_str() );
        }
        break;

    case 0x000006a6://OnAddEndpoint
        {
            //ShowInfo( TEXT("NewConferenceSessionParticipantEvents_OnAddEndpoint called.") );
            OutputDebugString( TEXT("OnAddEndpoint") );
            return true;
        }
        break;
    case 0x000006a7://OnRemoveEndpoint
        {
            //ShowInfo( TEXT("NewConferenceSessionParticipantEvents_OnRemoveEndpoint called.") );
            return true;
        }
        break;

    case 0x000006a8://OnReconcile
        {
            OutputDebugString( TEXT("OnReconcile") );
            //ShowInfo( TEXT("NewConferenceSessionParticipantEvents_OnReconcile called.") );
            return true;
        }
        break;
    }   

    return true;
}

//void CConfSessPartSink::HookEvents( PVOID pEvents )
//{
//    if( !pEvents )
//    {
//        return;
//    }  
//
//    CLock aLock(GetInstance()->GetCS());
//
//    if( m_pmpHooked->find( pEvents ) != m_pmpHooked->end() )
//    {
//        return;
//    }
//
//    InvokeFunc Functions;
//    memset(&Functions,0,sizeof(InvokeFunc));
//    PVOID* Vtable = (*(PVOID**)pEvents);	
//
//    MapHookedIt it = m_pmpHooked->begin();
//
//    bool bHooked = false;
//    for( ; it != m_pmpHooked->end(); ++it )
//    {
//        if( Vtable == (*it).second.pVtable )
//        {
//            bHooked = true;
//            Functions = (*it).second;
//            break;
//        }
//    }
//
//    if( !bHooked )
//    {
//        Functions.pVtable = Vtable;
//        PVOID Pointer = Vtable[6];
//        memcpy(&(Functions.pnInvoke),&Pointer,sizeof(ULONG));
//        Functions.poInvoke = Functions.pnInvoke;
//
//        gDetourTransactionBegin();
//        gDetourUpdateThread(GetCurrentThread());
//
//        gDetourAttach(&(PVOID&)Functions.pnInvoke, NewHookedInvoke );
//
//        gDetourTransactionCommit();  
//    } 
//
//    m_pmpHooked->insert(
//        std::make_pair(pEvents,Functions));
//}
//
