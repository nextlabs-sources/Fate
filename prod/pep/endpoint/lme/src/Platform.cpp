#include "stdafx.h"
#include "Platform.h"
#include "Blocks.h"
#include <string>
#include "PartDB.h"
#include "HookedConfSess.h"


//Some utility functions
void GetWindowTitle (HWND hWnd, WCHAR* pszTitle, int maxCount)
{
    //HWND hWnd = GetForegroundWindow();
    pszTitle[0] = L'\0';
    ::GetWindowTextW(hWnd, pszTitle, maxCount);
}

void GetCurrentSessionParticipantNum(HWND winHandle, int &num)
{
    WCHAR title[1024]={0};
    GetWindowTitle(winHandle, title, 1024);
    num=0;
    swscanf_s(title,L"%d", &num);
    //TRACE(0, _T("GetCurrentSessionParticipantNum: %d from title %s\n"), num, title);
}

INSTANCE_DEFINE( CHookedPlatform );

void CHookedPlatform::Hook( void* pPlatform )
{
  //  SubstituteOrgFuncWithNew( pPlatform, 3, (void*)NewPlatform_Initialize);
  //  SubstituteOrgFuncWithNew( pPlatform, 4, (void*)NewPlatform_CreateEndPointContext);
    SubstituteOrgFuncWithNew( pPlatform, 5, (void*)NewPlatform_CreateEndpoint);
    //SubstituteOrgFuncWithNew( pPlatform, 6, (void*)NewPlatform_CreateProxyEndpoint);
    //SubstituteOrgFuncWithNew( pPlatform, 7, (void*)NewPlatform_Shutdown);
    DoHook( pPlatform );
    OutputDebugString(L"CHookedPlatform::Hook");
}

//HRESULT __stdcall CHookedPlatform::NewPlatform_Initialize (
//    IUccPlatform* This,
//    BSTR bstrApplicationName,
//struct IUccContext * pContext )
//{
//    FUNC_PLATFORM_INITIALIZE pOrgFunc = (FUNC_PLATFORM_INITIALIZE)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_Initialize ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_PLATFORM_INITIALIZE)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_Initialize ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, bstrApplicationName, pContext );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedPlatform::NewPlatform_CreateEndPointContext(
//    IUccPlatform* This,
//PVOID ppContext 
//    )
//{
//	if( This == NULL )
//	{
//		::OutputDebugStringW( L"This is null" ) ;
//	}
////	::OutputDebugStringW( L"111111111111111" ) ;
//    FUNC_PLATFORM_CREATEENDPOINTCONTEXT pOrgFunc = (FUNC_PLATFORM_CREATEENDPOINTCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateEndPointContext ));
//    if( !pOrgFunc )
//    {
////		::OutputDebugStringW( L"2222222222222222222" ) ;
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_PLATFORM_CREATEENDPOINTCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateEndPointContext ));
//    }
////::OutputDebugStringW( L"33333333333333333" ) ;
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//		::OutputDebugStringW( L"444444444444" ) ;
//        hr = pOrgFunc( This, ppContext );
//    }
////	::OutputDebugStringW( L"5555555555555" ) ;
//    return hr;
//}

HRESULT __stdcall CHookedPlatform::NewPlatform_CreateEndpoint(
    IUccPlatform* This,
    BSTR bstrEndpointUri,
    BSTR bstrEndpointId,
struct IUccContext * pContext,
struct IUccEndpoint * * ppEndpoint 
    )
{
    FUNC_PLATFORM_CREATEENDPOINT pOrgFunc = (FUNC_PLATFORM_CREATEENDPOINT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateEndpoint ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_PLATFORM_CREATEENDPOINT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateEndpoint ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, bstrEndpointUri, bstrEndpointId, pContext, ppEndpoint );
    }

    OutputDebugString( L"CHookedPlatform::NewPlatform_CreateEndpoint:" );
    OutputDebugString( bstrEndpointUri );

    if(hr == S_OK && ppEndpoint)
    {
        IUccEndpoint* pEndPoint = *ppEndpoint;
        CHookedEndpoint::GetInstance()->Hook( PVOID(pEndPoint) );// HookEndpoint();
    }
    return hr;
}

//HRESULT __stdcall CHookedPlatform::NewPlatform_CreateProxyEndpoint(
//    IUccPlatform* This,
//struct IUccEndpoint * pControllingEndpoint,
//    BSTR bstrEndpointUri,
//    BSTR bstrEndpointId,
//struct IUccContext * pContext,
//struct IUccEndpoint * * ppEndpoint 
//    )
//{
//    FUNC_PLATFORM_CREATEPROXYENDPOINT pOrgFunc = (FUNC_PLATFORM_CREATEPROXYENDPOINT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateProxyEndpoint ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_PLATFORM_CREATEPROXYENDPOINT)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_CreateProxyEndpoint ));
//    }
//
//    
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pControllingEndpoint, bstrEndpointUri, bstrEndpointId, pContext, ppEndpoint );
//    }
//
//    OutputDebugString( L"CHookedPlatform::NewPlatform_CreateProxyEndpoint:" );
//    OutputDebugString( bstrEndpointUri );
//
//
//    return hr;
//}

//HRESULT __stdcall CHookedPlatform::NewPlatform_Shutdown(
//    IUccPlatform* This,
//struct IUccOperationContext * pOperationContext 
//    )
//{
//    FUNC_PLATFORM_SHUTDOWN pOrgFunc = (FUNC_PLATFORM_SHUTDOWN)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_Shutdown ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_PLATFORM_SHUTDOWN)(GetInstance()->GetOrgFunc( (void*)This, NewPlatform_Shutdown ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pOperationContext );
//    }
//    return hr;
//}

INSTANCE_DEFINE( CHookedEndpoint );

void CHookedEndpoint::Hook( void* pEndpoint )
{
    //SubstituteOrgFuncWithNew( pEndpoint, 3, (void*)NewEndpoint_Enable);
    //SubstituteOrgFuncWithNew( pEndpoint, 4, (void*)NewEndpoint_Disable);
    SubstituteOrgFuncWithNew( pEndpoint, 0, (void*)NewEndpoint_QueryInterface);
    DoHook( pEndpoint );
}
//
//HRESULT __stdcall CHookedEndpoint::NewEndpoint_Enable( 
//    IUccEndpoint* This,
//struct IUccOperationContext * pOperationContext )
//{
//    BSTR bstrUri = 0;
//
//    This->get_Uri( &bstrUri );
//    FUNC_ENDPOINT_ENABLE pOrgFunc = (FUNC_ENDPOINT_ENABLE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_Enable ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_ENDPOINT_ENABLE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_Enable ));
//    }
//
//    OutputDebugString( TEXT("CHookedEndpoint::NewEndpoint_Enable") );
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pOperationContext );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedEndpoint::NewEndpoint_Disable(
//    IUccEndpoint* This,
//struct IUccOperationContext * pOperationContext )
//{
//    FUNC_ENDPOINT_DISABLE pOrgFunc = (FUNC_ENDPOINT_DISABLE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_Disable ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_ENDPOINT_DISABLE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_Disable ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pOperationContext );
//    }
//    return hr;
//}

HRESULT __stdcall CHookedEndpoint::NewEndpoint_QueryInterface(
    IUnknown* This,
    const IID & riid,
    void **ppvObj )
{
    FUNC_IUNKNOWN_QUERYINTERFACE pOrgFunc = (FUNC_IUNKNOWN_QUERYINTERFACE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_QueryInterface ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_IUNKNOWN_QUERYINTERFACE)(GetInstance()->GetOrgFunc( (void*)This, NewEndpoint_QueryInterface ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, riid, ppvObj );
    }

    TCHAR ClassId[256] = {0};
    StringFromGUID2(riid,ClassId,sizeof(ClassId)/sizeof(TCHAR));
    if((memcmp(&IID_IUccSessionManager,&riid,sizeof(riid))==0))
    {
        wcscpy_s(ClassId,sizeof(ClassId)/sizeof(TCHAR),L"IID_IUccSessionManager");
    }
    else if((memcmp(&IID_IUccServerSignalingSettings,&riid,sizeof(riid))==0))
    {
        wcscpy_s(ClassId,sizeof(ClassId)/sizeof(TCHAR),L"IID_IUccServerSignalingSettings");
    }
    else if((memcmp(&IID_IUccDiagnosticReportingSettings,&riid,sizeof(riid))==0))
    {
        wcscpy_s(ClassId,sizeof(ClassId)/sizeof(TCHAR),L"IID_IUccDiagnosticReportingSettings");
    }
    else if(memcmp(&IID_IUccServiceOperationManager,&riid,sizeof(riid))==0)
    {
        wcscpy_s(ClassId,sizeof(ClassId)/sizeof(TCHAR),L"IID_IUccServiceOperationManager");
    }
    else if(memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0)
    {
        wcscpy_s(ClassId,sizeof(ClassId)/sizeof(TCHAR),L"IID_IConnectionPointContainer");
        //TRACE(0, _T("Query IID_IConnectionPointContaine\n"));
    }

    if( hr == S_OK ) 
    {
        if( memcmp( &IID_IUccSessionManager,&riid, sizeof(riid) ) == 0  )
        {
            IUccSessionManager* pUccSessionManager = (IUccSessionManager*)(*ppvObj);
            //HookSessionManager(pUccSessionManager);
            CHookedSessMgr::GetInstance()->Hook( PVOID(pUccSessionManager) );
        }
        else if(memcmp(&IID_IUccServiceOperationManager,&riid,sizeof(riid))==0)
        {
    //        IUccServiceOperationManager* pServiceOperationManager = (IUccServiceOperationManager*)(*ppvObj);
            //HookServiceOperationManager(pServiceOperationManager);
        } 
        else if (_wcsnicmp(ClassId, L"{72dbffa6-340b-4358-a100-12c8e20d428b}", 
            wcslen(L"{72dbffa6-340b-4358-a100-12c8e20d428b}")) == 0) 
        {
  //          IUccSubscriptionManager *pSubscriptionManager = (IUccSubscriptionManager *)(*ppvObj);
            //HookSubscriptionManager(pSubscriptionManager);
        } 
        else if (memcmp(&IID_IConnectionPointContainer,&riid,sizeof(riid))==0) 
        {
            IConnectionPointContainer* pConnectionPointContainer = (IConnectionPointContainer*)(*ppvObj);
            //HookConnectionPointContainer(pConnectionPointContainer);
            CHookedConnPointContainer::GetInstance()->Hook( PVOID(pConnectionPointContainer) );
        } 
    }
    return hr;	
}

INSTANCE_DEFINE( CHookedSessMgr );

void CHookedSessMgr::Hook( void* pSessMgr )
{
   // SubstituteOrgFuncWithNew( pSessMgr, 3, (void*)NewSessionMan_CreateSessionContext);
    SubstituteOrgFuncWithNew( pSessMgr, 4, (void*)NewSessionMan_CreateSession);
    //SubstituteOrgFuncWithNew( pSessMgr, 5, (void*)NewSessionMan_RegisterSessionDescriptionEvaluator);
    //SubstituteOrgFuncWithNew( pSessMgr, 6, (void*)NewSessionMan_UnregisterSessionDescriptionEvaluator);
    DoHook( pSessMgr );
}

//HRESULT __stdcall CHookedSessMgr::NewSessionMan_CreateSessionContext(
//    IUccSessionManager* This,
//    enum UCC_SESSION_TYPE enSessionType,
//struct IUccContext * * ppContext 
//    )
//{
//    FUNC_SESSIONMAN_CREATESESSIONCONTEXT pOrgFunc = (FUNC_SESSIONMAN_CREATESESSIONCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_CreateSessionContext ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSIONMAN_CREATESESSIONCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_CreateSessionContext ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, enSessionType, ppContext );
//    }
//    return hr;
//}

HRESULT __stdcall CHookedSessMgr::NewSessionMan_CreateSession(
    IUccSessionManager* This,
    enum UCC_SESSION_TYPE enSessionType,
struct IUccContext * pContext,
struct IUccSession * * ppSession 
    )
{
    char Error[256]={0};

    FUNC_SESSIONMAN_CREATESESSION pOrgFunc = (FUNC_SESSIONMAN_CREATESESSION)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_CreateSession ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_SESSIONMAN_CREATESESSION)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_CreateSession ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, enSessionType, pContext, ppSession );
    }

    char SessionType[256]={0};
    GetSessionTypeString(SessionType,enSessionType);

    sprintf_s(Error,sizeof(Error),"NewSessionManager_CreateSession(%p,%s,%p,%p) return %x\n",
        This,
        SessionType,
        pContext,
        ppSession? (*ppSession):NULL,
        hr);

	OutputDebugStringA(Error);
// 	if( enSessionType == UCCST_AUDIO_VIDEO )
// 	{
// 		if( !DoEvaluate( LME_MAGIC_STRING,L"VOICE" ) )
// 		{     
// 			return E_NOTIMPL ;
// 		}
// 	}
    if(hr == S_OK && ppSession)
    {
        IUccSession* pUccSession = *ppSession;
        if( enSessionType == UCCST_INSTANT_MESSAGING )
        {
            IUccInstantMessagingSession* pIMSess = 0;
            hr = pUccSession->QueryInterface( IID_IUccInstantMessagingSession, (void**)(&pIMSess) );
            CHookedIMSess::GetInstance()->Hook( PVOID(pIMSess) );
        }

     /*   if( enSessionType == UCCST_CONFERENCE )
        {
            IUccConferenceSession* pConfSess = 0;
            hr = pUccSession->QueryInterface( IID_IUccConferenceSession, (void**)(&pConfSess) );
            CPartDB::GetInstance()->SetSession( pUccSession );
            CHookedConfSess::GetInstance()->Hook( PVOID( pConfSess )  );
        }*/
        //else
        {
            CHookedSess::GetInstance()->Hook( PVOID( pUccSession )  );//HookSession(pUccSession);
        }
        //IUccContext* pContext = NULL;
        //pUccSession->CreateParticipantContext(&pContext);
    }
    return hr;
}

//HRESULT __stdcall CHookedSessMgr::NewSessionMan_RegisterSessionDescriptionEvaluator(
//    IUccSessionManager* This,
//struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
//    )
//{
//    FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR pOrgFunc = (FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_RegisterSessionDescriptionEvaluator ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_RegisterSessionDescriptionEvaluator ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pSessionDescriptionEvaluator );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedSessMgr::NewSessionMan_UnregisterSessionDescriptionEvaluator(
//    IUccSessionManager* This
//    )
//{
//    FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR pOrgFunc = (FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_UnregisterSessionDescriptionEvaluator ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR)(GetInstance()->GetOrgFunc( (void*)This, NewSessionMan_UnregisterSessionDescriptionEvaluator ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This );
//    }
//    return hr;
//}


INSTANCE_DEFINE( CHookedSess );

void CHookedSess::Hook( void* pSess )
{
    //SubstituteOrgFuncWithNew( pSess, 7, (void*)NewSession_CreateParticipantContext);
    //SubstituteOrgFuncWithNew( pSess, 8, (void*)NewSession_CreateParticipant);
    SubstituteOrgFuncWithNew( pSess, 9, (void*)NewSession_AddParticipant);
    //SubstituteOrgFuncWithNew( pSess, 10, (void*)NewSession_RemoveParticipant);
    //SubstituteOrgFuncWithNew( pSess, 11, (void*)NewSession_Terminate);
    DoHook( pSess );
}
//HRESULT __stdcall CHookedSess::NewSession_CreateParticipantContext (
//    IUccSession* This,
//struct IUccContext * * ppContext 
//    )
//{
//    FUNC_SESSION_CREATEPARTICIPANTCONTEXT pOrgFunc = (FUNC_SESSION_CREATEPARTICIPANTCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_CreateParticipantContext ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSION_CREATEPARTICIPANTCONTEXT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_CreateParticipantContext ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, ppContext );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedSess::NewSession_CreateParticipant (
//    IUccSession* This,
//    BSTR bstrUri,
//struct IUccContext * pContext,
//struct IUccSessionParticipant * * ppParticipant 
//    )
//{
//    FUNC_SESSION_CREATEPARTICIPANT pOrgFunc = (FUNC_SESSION_CREATEPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_CreateParticipant ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSION_CREATEPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_CreateParticipant ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, bstrUri, pContext, ppParticipant );
//    }
//    return hr;
//}

HRESULT __stdcall CHookedSess::NewSession_AddParticipant (
    IUccSession* This,
struct IUccSessionParticipant * pParticipant,
struct IUccOperationContext * pOperationContext 
    )
{
    FUNC_SESSION_ADDPARTICIPANT pOrgFunc = (FUNC_SESSION_ADDPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_AddParticipant ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_SESSION_ADDPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_AddParticipant ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, pParticipant, pOperationContext );
    }

    TCHAR Uri[256]={0};
    if(pParticipant)
    {
        BSTR bstrUri = NULL;
        pParticipant->get_Uri(&bstrUri);
        if(SUCCEEDED(hr)&&bstrUri) wcscpy_s(Uri,sizeof(Uri)/sizeof(TCHAR),bstrUri);
        SysFreeString(bstrUri);
    }

    enum UCC_SESSION_TYPE SessionType;
    This->get_Type(&SessionType);
    char SessionTypeString[256]={0};
    GetSessionTypeString(SessionTypeString,SessionType);
	DPW((L"New session participant added:[%s]", Uri)) ;
    bool Block = IsBlock(Uri,SessionType);
    if(Block)
    {
        //sprintf_s(Error,sizeof(Error),"NewSession_AddParticipant(%p(%s),%p(%ws),%p) BLOCKED!!!\n",
        //	This,
        //	SessionTypeString,
        //	pParticipant,
        //	Uri,
        //	pOperationContext
        //	);
        ////Show message box
        //WCHAR msg[1024];
        //swprintf(msg, sizeof(msg)/sizeof(WCHAR),L"You cannot send message to [%s]!", Uri);
        //MessageBoxW(GetForegroundWindow(), msg, L"Error", MB_OK);     
        return -1;
    }

    return hr;
}

//HRESULT __stdcall CHookedSess::NewSession_RemoveParticipant (
//    IUccSession* This,
//struct IUccSessionParticipant * pParticipant,
//struct IUccOperationContext * pOperationContext 
//    )
//{
//    FUNC_SESSION_REMOVEPARTICIPANT pOrgFunc = (FUNC_SESSION_REMOVEPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_RemoveParticipant ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSION_REMOVEPARTICIPANT)(GetInstance()->GetOrgFunc( (void*)This, NewSession_RemoveParticipant ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, pParticipant, pOperationContext );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedSess::NewSession_Terminate (
//    IUccSession* This,
//    enum UCC_TERMINATE_REASON enTerminateReason,
//struct IUccOperationContext * pOperationContext 
//    )
//{
//    FUNC_SESSION_TERMINATE pOrgFunc = (FUNC_SESSION_TERMINATE)(GetInstance()->GetOrgFunc( (void*)This, NewSession_Terminate ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_SESSION_TERMINATE)(GetInstance()->GetOrgFunc( (void*)This, NewSession_Terminate ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, enTerminateReason, pOperationContext );
//    }
//    return hr;
//}


INSTANCE_DEFINE( CHookedIMSess );

void CHookedIMSess::Hook( void* pIMSess )
{
    SubstituteOrgFuncWithNew( pIMSess, 3, (void*)NewIM_Session_SendMessage );
    DoHook( pIMSess );
}

HRESULT __stdcall CHookedIMSess::NewIM_Session_SendMessage (
    IUccInstantMessagingSession* This,
    BSTR bstrContentType,
    BSTR bstrMessage,
struct IUccOperationContext * pOperationContext 
    )
{
    FUNC_IM_SESSION_SENDMESSAGE pOrgFunc = (FUNC_IM_SESSION_SENDMESSAGE)(GetInstance()->GetOrgFunc( (void*)This, NewIM_Session_SendMessage ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_IM_SESSION_SENDMESSAGE)(GetInstance()->GetOrgFunc( (void*)This, NewIM_Session_SendMessage ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, bstrContentType, bstrMessage, pOperationContext );
    }

    if(pOperationContext) {
        IUccContext *pContext;
        if(pOperationContext->get_Context(&pContext)==S_OK && pContext) {
            IUccProperty *pProperty;
            if((pContext)->get_Property(UCCSC_ASSOCIATED_CONFERENCE_SESSION, &pProperty)
                == S_OK && pProperty) 
                //TRACE(0, _T("SendMessage--Conext: UCCSC_ASSOCIATED_CONFERENCE_SESSION: %p\n"),
                //(pProperty));
                if((pContext)->get_Property(UCCSC_TRANSFER_CONTEXT, &pProperty)
                    == S_OK && pProperty) 
                {
                    //TRACE(0, _T("SendMessage--Conext: UCCSC_TRANSFER_CONTEXT: %p\n"),
                    //(pProperty));
                }
        }
    }

    ////TRACE(0, _T("Send message type:%s\n msg:%s\n in session:%p\n"), 
    //	bstrContentType, bstrMessage, This);

    IUccCollection *pParticipants; 
    IUccSessionParticipant *pParticipant;
    IUccSession *pIuccSession;
    long numParticipants;
    std::wstring disclaimer=L"";
    BSTR curDisclaimer=NULL;
    HRESULT qResult;
    bool bAllow=true;
//    DWORD tid=GetCurrentThreadId();

    //Do policy evaluation
    if(wcsstr(bstrContentType, L"text/rtf")) {
        //Check if we need to add a disclaimer
        qResult= This->QueryInterface(IID_IUccSession, (void **)&pIuccSession);
        if(qResult != S_OK) {
            //TRACE(0, _T("QueryInterface failed due to %d\n"), qResult);
        } else {
            pIuccSession->get_Participants(&pParticipants );
            pParticipants->get_Count(&numParticipants);
            for(int i=1; i<=numParticipants; i++) {
                VARIANT vtItem;
                vtItem.vt = VT_DISPATCH;
                pParticipants->get_Item(i, &vtItem);
                pParticipant = (IUccSessionParticipant*) vtItem.pdispVal;
                BSTR bstrUri = NULL;
                pParticipant->get_Uri(&bstrUri);
                if(SUCCEEDED(hr)&&bstrUri) 
                {
                    //ocePolicyEval.FetchDisclaimer(bstrUri, &curDisclaimer);
                    if(curDisclaimer) {
                        //We got a disclaimer, append to the exisiting disclaimer
                        disclaimer+=curDisclaimer;
                        SysFreeString(curDisclaimer);
                        curDisclaimer=NULL;
                    }
                }
                SysFreeString(bstrUri);
            }
            pIuccSession->Release();
        }
    }
    else if(wcsstr(bstrMessage,L"Application-Name: File Transfer")) 
    {
        //Check if file transfer is allowed
        //Get the transferred file
        TCHAR fileName[256]={0};
        TCHAR *fileNamePos=wcsstr(bstrMessage, L"Application-File: ");
        if(fileNamePos) 
        {
            fileNamePos+=wcslen(L"Application-File: ");
            TCHAR *fileNameEndPos=wcsstr(fileNamePos, L"Application-Fi");
            rsize_t fileNameLen=fileNameEndPos-fileNamePos-2;
            if(fileNameEndPos && fileNameLen>0) 
            {
                wcsncpy_s(fileName, 256, fileNamePos, fileNameLen);
                fileName[fileNameLen]=L'\0';
                //TRACE(0, _T("file transfer: tid=%d fileName=%s--\n"), GetCurrentThreadId(), 
                //fileName);
            }
        }

        if(wcslen(fileName) > 0) 
        {
            //we got the file name.
            //go through each participant and check if he/she is allowed to receive the file 
            qResult= This->QueryInterface(IID_IUccSession, (void **)&pIuccSession);
            if(qResult != S_OK) 
            {
                //TRACE(0, _T("QueryInterface failed due to %d\n"), qResult);
            } 
            else 
            {
                pIuccSession->get_Participants(&pParticipants );
                pParticipants->get_Count(&numParticipants);
                for(int i=1; i<=numParticipants; i++) 
                {
                    VARIANT vtItem;
                    vtItem.vt = VT_DISPATCH;
                    pParticipants->get_Item(i, &vtItem);
                    pParticipant = (IUccSessionParticipant*) vtItem.pdispVal;
                    BSTR bstrUri = NULL;
                    HRESULT hr_l = pParticipant->get_Uri(&bstrUri);
                    if(SUCCEEDED(hr_l)&&bstrUri) 
                    {
                        //bAllow=ocePolicyEval.EvalTransferFile(bstrUri, fileName);
                    }
                    SysFreeString(bstrUri);

                    if(!bAllow) 
                    {
                        //Show message box
                        WCHAR msg[1024];
                        _snwprintf_s(msg, sizeof(msg), L"Transfer file to [%s] is restricted!", bstrUri);   
                        /*if(MessageBox(NULL, msg, L"Compliant Enterprise Communicator Enforcer", MB_SERVICE_NOTIFICATION) == 0) {
                        sprintf_s(Error,sizeof(Error),"Faile to call MessageBox due to %d\n",
                        GetLastError);
                        }*/
                        return (HRESULT)-1; //UCC_E_SIP_AUTHENTICATION_FAILED;
                    }
                }
                pIuccSession->Release();
            }
        }
    } 
    else if(wcsstr(bstrContentType, L"multipart/alternative")) 
    {
        HWND winHandle=GetForegroundWindow();
        /*ocePolicyEval.StoreGroupSendMsgFunc(winHandle, 
        OldSendMessage, 
        This, pOperationContext);*/
        //When add more people to an exisiting group chat, the title won't change 
        //to the new number of participant until all adding participants done. So
        //we will do policy evaluation here and if allows, add participant here.
        int numParticipant;
        GetCurrentSessionParticipantNum(winHandle, numParticipant);
        /*ocePolicyEval.UpdateGroupParticipantNum(winHandle,numParticipant);
        if(ocePolicyEval.IsTimeToDoEvaluation(winHandle)) {
        bool bAllow=ocePolicyEval.DoGroupChatEvaluation(winHandle, 
        L"dummyParticipant");
        if(!bAllow) {
        return S_OK; 
        }				
        }*/
    }


    if(!disclaimer.empty()) {
        //peer-to-peer IM
        //bool bSendDisclaimer=ocePolicyEval.IsIMSessionFirstMsg(This);
        //if(bSendDisclaimer) {
        //    //Only send disclaimer with the first message of this IM session
        //    BSTR msgType = SysAllocStringTEXT("text/plain");
        //    BSTR msg=SysAllocString(disclaimer.c_str());
        //    This->SendMessageW(msgType, msg, pOperationContext);
        //    SysFreeString(msgType);
        //    SysFreeString(msg);
        //}
    } 

    if(wcsstr(bstrContentType, L"multipart/alternative")) 
    {
   //     HWND winHandle=GetForegroundWindow();
        //ocePolicyEval.SendGroupDisclaimer(winHandle, This, bstrContentType, pOperationContext);
    }
    return hr;
}

INSTANCE_DEFINE( CHookedConnPoint );


void CHookedConnPoint::Hook( void* pConnPoint )
{
    SubstituteOrgFuncWithNew( pConnPoint, 5, (void*)NewConnPoint_Advise);
    SubstituteOrgFuncWithNew( pConnPoint, 6, (void*)NewConnPoint_Unadvise);
    DoHook( pConnPoint );
}
HRESULT	__stdcall CHookedConnPoint::NewConnPoint_Advise( IConnectionPoint *This,
                                                        IUnknown * pUnk,
                                                        DWORD * pdwCookie )
{
    //ShowInfo( TEXT("NewConnPoint_Advise called.") );
  //  char Error[1024]={0};
    ////TRACE(0, _T("In NewConnPoint_Advise\n"));
    FUNC_CONNPOINT_ADVISE pOrgFunc = (FUNC_CONNPOINT_ADVISE)(GetInstance()->GetOrgFunc( (void*)This, NewConnPoint_Advise ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_CONNPOINT_ADVISE)(GetInstance()->GetOrgFunc( (void*)This, NewConnPoint_Advise ));
    }    

    //Do policy evaluation
    IConnectionPointContainer * pCPC;

    HRESULT hr=This->GetConnectionPointContainer(&pCPC);
    if(hr==S_OK) 
    {
        {
            //IUccSignalingChannel* pSignalingChannel = 0;
            //pCPC->QueryInterface( IID_IUccSignalingChannel, (void**)&pSignalingChannel );
            //if( pSignalingChannel )
            //{
            //    CHookedSignalingChannel::GetInstance()->Hook( PVOID( pSignalingChannel ) );
            //    /*_IUccSignalingChannelEvents* pSigEvents = 0;
            //    if( hr = p_IUccSessionParticipantEvents->QueryInterface( DIID__IUccSignalingChannelEvents, (void**)&pSigEvents ) )
            //    {
            //    int i = 0;
            //    }*/
            //}

            //IUccSignalingMessage* pSignalingMsg = 0;
            //pCPC->QueryInterface( IID_IUccSignalingMessage, (void**)&pSignalingMsg );
            //if( pSignalingMsg )
            //{
            //    CHookedSignalingMessage::GetInstance()->Hook( PVOID( pSignalingMsg ) );
            //    /*_IUccSignalingChannelEvents* pSigEvents = 0;
            //    if( hr = p_IUccSessionParticipantEvents->QueryInterface( DIID__IUccSignalingChannelEvents, (void**)&pSigEvents ) )
            //    {
            //    int i = 0;
            //    }*/
            //}
        }

        IID   iid;

        if(This->GetConnectionInterface(&iid) == S_OK) 
        {
            TCHAR ClassId[256] = {0};
            StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(TCHAR));
            if(_wcsnicmp(ClassId, L"{6853A6B5-88FA-46D2-B5DA-758DAB4E0065}",//_IUccInstantMessagingSessionEvents
                wcslen(L"{6853A6B5-88FA-46D2-B5DA-758DAB4E0065}")) == 0 ) 
            { 
                //Advise _IUccInstantMessagingSessionEvents
                //Get IUccSession for policy evaluation
                IUccInstantMessagingSession *pIMSession;

                hr=pCPC->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession)); 
                if(hr==S_OK) 
                {
                    CHookedIMSess::GetInstance()->Hook( (PVOID)pIMSession );//HookIMSession(pIMSession);
                }
                IUccSession *pSession;
                hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
                //TRACE(0, _T("Advise IUccIMSessionEvents for session(%p) \n"), pSession);	

                if(hr==S_OK)
                {
                    CHookedSess::GetInstance()->Hook( PVOID( pSession )  );
                    //HookSession(pSession);
                    //bool bAllow=DoEvalOnIncomingSession(pSession);
                    pSession->Release();
                    //if(!bAllow)
                    //return -1;
                }
            }
            else if(_wcsnicmp(ClassId, L"{DD8B1B9A-B1C4-4065-91B4-2152D73CD679}", //IUccSessionEvents
                wcslen(L"{DD8B1B9A-B1C4-4065-91B4-2152D73CD679}")) == 0 ) 
            { 
                //Advise IUccSessionEvents 
                //Get IUccSession for policy evaluation
                IUccSession *pSession;
                hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
                //TRACE(0, _T("Advise IUccSessionEvents for session(%p) \n"), pSession);	

                if(hr==S_OK) 
                {
                    CHookedSess::GetInstance()->Hook( PVOID( pSession ) );//HookSession(pSession);
                    //bool bAllow=DoEvalOnIncomingSession(pSession);
                    pSession->Release();
                    //if(!bAllow)
                    //return -1;
                }
            } 
            else if(_wcsnicmp(ClassId, L"{A6217CF2-4555-4f6f-AD24-B8676D54900B}", 
                wcslen(L"{A6217CF2-4555-4f6f-AD24-B8676D54900B}")) == 0 ||
                _wcsnicmp(ClassId, L"{163D51BA-62A0-432c-B1D9-667F58E86130}", 
                wcslen(L"{163D51BA-62A0-432c-B1D9-667F58E86130}")) == 0) 
            {
                //Advise _IUccSessionParticipantEvents 	
                IUccSessionParticipant *pParticipant;
                GUID guidValue;

                if(CLSIDFromString(L"{13E7C9BE-E38B-4EC7-A13D-7A70F6101B31}", &guidValue) == NOERROR) 
                {
                    hr=pCPC->QueryInterface(guidValue, (void **)(&pParticipant));

                    if(hr == S_OK) 
                    {
                        IUccSession *pSession;
                        UCC_SESSION_TYPE sessionType;

                        pParticipant->get_Session(&pSession);

                        pSession->get_Type(&sessionType);
                        CHookedSess::GetInstance()->Hook( PVOID( pSession ) );//HookSession(pSession);
                        //Collection group IM information on incoming session
                        if(sessionType == UCCST_CONFERENCE) 
                        {

                            //bool bAllow=ocePolicyEval.DoGroupEvalOnIncomingSession(pSession);	

                            //if(!bAllow) 
                            //{
                            //    //If not allowed, the local participant needs to leave 
                            //    //from this communication

                            //    GUID guidValue; 
                            //    IUccConferenceSession *pConfSession;

                            //    if(CLSIDFromStringTEXT("{4e69a403-433c-47f8-a3fd-a9fa67efb522}", 
                            //        &guidValue) == NOERROR ) 
                            //    {
                            //            pSession->QueryInterface(guidValue, (void **)(&pConfSession));
                            //    }
                            //    pConfSession->Leave(NULL);
                            //}
                        }

                        IUccCollection *pParticipants; 
                        long numParticipants;
                        pSession->get_Participants(&pParticipants );
                        pParticipants->get_Count(&numParticipants);
                        if(sessionType == UCCST_INSTANT_MESSAGING) 
                        {
                            IUccInstantMessagingSession* pIMSession;
                            pSession->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession));
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_INSTANT_MESSAGING\n"), pSession, numParticipants);
                        }
                        else if(sessionType == UCCST_CONFERENCE)
                        {
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_CONFERENCE\n"), pSession, numParticipants);
                        }
                        else if(sessionType == UCCST_APPLICATION)
                        {
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_APPLICATION\n"), pSession, numParticipants);
                            //TRACE(0, _T("Advise IUccSessionParticipantEvents for session(%p) \n"), pSession);	
                        }

                        for(int i=1; i<=numParticipants; i++) 
                        {
                            VARIANT vtItem;
                            vtItem.vt = VT_DISPATCH;
                            pParticipants->get_Item(i, &vtItem);
                            pParticipant = (IUccSessionParticipant*) vtItem.pdispVal;

                            BSTR bstrUri = NULL;
                            HRESULT hr_l = pParticipant->get_Uri(&bstrUri);
                            if(SUCCEEDED(hr_l) && bstrUri) 
                            {
                                static TCHAR strTmp[1024] = {0};
                                swprintf_s( strTmp, 1024, L"Participant Entered: %s", (TCHAR*)bstrUri );
                                //ShowInfo( strTmp );
                                //TRACE(0, _T("Advise Session(%p) has non-local participant(%d): %s\n"), 
                                //pSession, numParticipants-1, bstrUri);							
                                SysFreeString(bstrUri);
                            }
                            VARIANT_BOOL bLocal;
                            if(pParticipant->get_IsLocal(&bLocal)==S_OK)
                                if(bLocal)
                                    continue;                                
                        }
                        pParticipant->Release();
                    }
                }
            }
        }
    }

    hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, pUnk, pdwCookie );
    }

    QueryEvents( pUnk, This );

    IID   iid;
    if(This->GetConnectionInterface(&iid) == S_OK) 
    {
        TCHAR ClassId[256] = {0};
        StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(TCHAR));
        //TRACE(0, _T("Advise for %s\n"), ClassId);
    }

    /*	if(_wcsnicmp(ClassId, L"{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}", 
    wcslen(L"{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}")) == 0) {
    //TRACE(0, _T("Advise for _IUccSessionManagerEvents\n"));
    GUID guidValue; 
    if(CLSIDFromStringTEXT("{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}", 
    &guidValue) == NOERROR ) {
    _IUccSessionManagerEvents *pSessionManEvents;
    (*pUnk).QueryInterface(guidValue, (void **)(&pSessionManEvents));
    //HookSessionManagerEvents((void *)pSessionManEvents);
    HookSessionManagerEvents((void *)pUnk);
    pSessionManEvents->Release();
    }
    } */

    return hr;
}

HRESULT	__stdcall CHookedConnPoint::NewConnPoint_Unadvise( IConnectionPoint *This,
                                                          DWORD * pdwCookie )
{
   // char Error[1024]={0};
    ////TRACE(0, _T("In NewConnPoint_Advise\n"));
    FUNC_CONNPOINT_UNADVISE pOrgFunc = (FUNC_CONNPOINT_UNADVISE)(GetInstance()->GetOrgFunc( (void*)This, NewConnPoint_Unadvise ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_CONNPOINT_UNADVISE)(GetInstance()->GetOrgFunc( (void*)This, NewConnPoint_Unadvise ));
    }   

    //Do policy evaluation
    IConnectionPointContainer * pCPC;

    HRESULT hr=This->GetConnectionPointContainer(&pCPC);
    if(hr==S_OK) 
    {
        IID   iid;

        if(This->GetConnectionInterface(&iid) == S_OK) 
        {
            TCHAR ClassId[256] = {0};
            StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(TCHAR));
            if(_wcsnicmp(ClassId, L"{6853A6B5-88FA-46D2-B5DA-758DAB4E0065}",//_IUccInstantMessagingSessionEvents
                wcslen(L"{6853A6B5-88FA-46D2-B5DA-758DAB4E0065}")) == 0 ) 
            { 
                //Advise _IUccInstantMessagingSessionEvents
                //Get IUccSession for policy evaluation
                IUccInstantMessagingSession *pIMSession;

                hr=pCPC->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession)); 
                if(hr==S_OK) 
                {
                    CHookedIMSess::GetInstance()->Hook( (PVOID)pIMSession );//HookIMSession(pIMSession);
                }
                IUccSession *pSession;
                hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
                //TRACE(0, _T("Advise IUccIMSessionEvents for session(%p) \n"), pSession);	

                if(hr==S_OK)
                {
                    CHookedSess::GetInstance()->Hook( PVOID( pSession ) );//HookSession(pSession);
                    //bool bAllow=DoEvalOnIncomingSession(pSession);
                    pSession->Release();
                    //if(!bAllow)
                    //return -1;
                }
            }
            else if(_wcsnicmp(ClassId, L"{DD8B1B9A-B1C4-4065-91B4-2152D73CD679}", //IUccSessionEvents
                wcslen(L"{DD8B1B9A-B1C4-4065-91B4-2152D73CD679}")) == 0 ) 
            { 
                //Advise IUccSessionEvents 
                //Get IUccSession for policy evaluation
                IUccSession *pSession;
                hr=pCPC->QueryInterface(IID_IUccSession, (void **)(&pSession)); 
                //TRACE(0, _T("Advise IUccSessionEvents for session(%p) \n"), pSession);	

                if(hr==S_OK) 
                {
                    CHookedSess::GetInstance()->Hook( PVOID( pSession ) );//HookSession(pSession);
                    //bool bAllow=DoEvalOnIncomingSession(pSession);
                    pSession->Release();
                    //if(!bAllow)
                    //return -1;
                }
            } 
            else if(_wcsnicmp(ClassId, L"{A6217CF2-4555-4f6f-AD24-B8676D54900B}", 
                wcslen(L"{A6217CF2-4555-4f6f-AD24-B8676D54900B}")) == 0 ||
                _wcsnicmp(ClassId, L"{163D51BA-62A0-432c-B1D9-667F58E86130}", 
                wcslen(L"{163D51BA-62A0-432c-B1D9-667F58E86130}")) == 0) 
            {
                //Advise _IUccSessionParticipantEvents 	
                IUccSessionParticipant *pParticipant;
                GUID guidValue;

                if(CLSIDFromString(L"{13E7C9BE-E38B-4EC7-A13D-7A70F6101B31}", &guidValue) == NOERROR) 
                {
                    hr=pCPC->QueryInterface(guidValue, (void **)(&pParticipant));

                    if(hr == S_OK) 
                    {
                        IUccSession *pSession;
                        UCC_SESSION_TYPE sessionType;

                        pParticipant->get_Session(&pSession);

                        pSession->get_Type(&sessionType);
                        CHookedSess::GetInstance()->Hook( PVOID( pSession ) );//HookSession(pSession);
                        //Collection group IM information on incoming session
                        if(sessionType == UCCST_CONFERENCE) 
                        {
                            //bool bAllow=ocePolicyEval.DoGroupEvalOnIncomingSession(pSession);	

                            //if(!bAllow) 
                            //{
                            //    //If not allowed, the local participant needs to leave 
                            //    //from this communication

                            //    GUID guidValue; 
                            //    IUccConferenceSession *pConfSession;

                            //    if(CLSIDFromStringTEXT("{4e69a403-433c-47f8-a3fd-a9fa67efb522}", 
                            //        &guidValue) == NOERROR ) 
                            //    {
                            //            pSession->QueryInterface(guidValue, (void **)(&pConfSession));
                            //    }
                            //    pConfSession->Leave(NULL);
                            //}
                        }

                        IUccCollection *pParticipants; 
                        long numParticipants;
                        pSession->get_Participants(&pParticipants );
                        pParticipants->get_Count(&numParticipants);
                        if(sessionType == UCCST_INSTANT_MESSAGING) 
                        {
                            IUccInstantMessagingSession* pIMSession;
                            pSession->QueryInterface(IID_IUccInstantMessagingSession, (void **)(&pIMSession));
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_INSTANT_MESSAGING\n"), pSession, numParticipants);
                        }
                        else if(sessionType == UCCST_CONFERENCE)
                        {
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_CONFERENCE\n"), pSession, numParticipants);
                        }
                        else if(sessionType == UCCST_APPLICATION)
                        {
                            //TRACE(0,_T("Advise IUccSessionParticipantEvents for Session(%p %d) is UCCST_APPLICATION\n"), pSession, numParticipants);
                            //TRACE(0, _T("Advise IUccSessionParticipantEvents for session(%p) \n"), pSession);	
                        }

                        for(int i=1; i<=numParticipants; i++) 
                        {
                            VARIANT vtItem;
                            vtItem.vt = VT_DISPATCH;
                            pParticipants->get_Item(i, &vtItem);
                            pParticipant = (IUccSessionParticipant*) vtItem.pdispVal;

                            BSTR bstrUri = NULL;
                            HRESULT hr_l = pParticipant->get_Uri(&bstrUri);
                            if(SUCCEEDED(hr_l) && bstrUri) 
                            {
                                static TCHAR strTmp[1024] = {0};
                                //swprintf_s( strTmp, 1024, L"Participant Left: %s", (TCHAR*)bstrUri );
                                //ShowInfo( strTmp );
                                //TRACE(0, _T("Advise Session(%p) has non-local participant(%d): %s\n"), 
                                //pSession, numParticipants-1, bstrUri);							
                                SysFreeString(bstrUri);
                            }
                            VARIANT_BOOL bLocal;
                            if(pParticipant->get_IsLocal(&bLocal)==S_OK)
                                if(bLocal)
                                    continue;                            
                        }
                        pParticipant->Release();
                    }
                }
            }
        }
    }    

    hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, pdwCookie );
    }

    IID   iid;
    if(This->GetConnectionInterface(&iid) == S_OK) 
    {
        TCHAR ClassId[256] = {0};
        StringFromGUID2(iid,ClassId,sizeof(ClassId)/sizeof(TCHAR));
        //TRACE(0, _T("Advise for %s\n"), ClassId);
    }

    /*	if(_wcsnicmp(ClassId, L"{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}", 
    wcslen(L"{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}")) == 0) {
    //TRACE(0, _T("Advise for _IUccSessionManagerEvents\n"));
    GUID guidValue; 
    if(CLSIDFromStringTEXT("{cb3b93ff-7a90-4504-915a-2cd2c6e8a0e8}", 
    &guidValue) == NOERROR ) {
    _IUccSessionManagerEvents *pSessionManEvents;
    (*pUnk).QueryInterface(guidValue, (void **)(&pSessionManEvents));
    //HookSessionManagerEvents((void *)pSessionManEvents);
    HookSessionManagerEvents((void *)pUnk);
    pSessionManEvents->Release();
    }
    } */

    return hr;
}




INSTANCE_DEFINE( CHookedConnPointContainer );


void CHookedConnPointContainer::Hook( void* pConnPointContainer )
{
   // SubstituteOrgFuncWithNew( pConnPointContainer, 3, (void*)NewConnPointContainer_EnumConnectionPoints);
    SubstituteOrgFuncWithNew( pConnPointContainer, 4, (void*)NewConnPointContainer_FindConnectionPoint);    
    DoHook( pConnPointContainer );
}
HRESULT	__stdcall CHookedConnPointContainer::NewConnPointContainer_FindConnectionPoint(
    IConnectionPointContainer *This,
    REFIID riid,
    IConnectionPoint ** ppCP)
{
    FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT pOrgFunc = (FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT)(
        GetInstance()->GetOrgFunc( (void*)This, NewConnPointContainer_FindConnectionPoint ));
    if( !pOrgFunc )
    {
        GetInstance()->Hook( This );
        pOrgFunc = (FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT)(GetInstance()->GetOrgFunc( (void*)This, NewConnPointContainer_FindConnectionPoint ));
    }

    HRESULT hr = E_NOTIMPL;
    if( pOrgFunc )
    {
        hr = pOrgFunc( This, riid, ppCP );
    }

    if( hr==S_OK && ppCP!=NULL && *ppCP!=NULL ) 
    {
        IConnectionPoint *pCP=*ppCP;
        CHookedConnPoint::GetInstance()->Hook( PVOID(pCP) );
    }
    return hr;
}

//HRESULT	__stdcall CHookedConnPointContainer::NewConnPointContainer_EnumConnectionPoints(
//    IConnectionPointContainer *This,
//    IEnumConnectionPoints ** ppEnum)
//{
//    FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS pOrgFunc = (FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS)(
//        GetInstance()->GetOrgFunc( (void*)This, NewConnPointContainer_EnumConnectionPoints ));
//    if( !pOrgFunc )
//    {
//        GetInstance()->Hook( This );
//        pOrgFunc = (FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS)(GetInstance()->GetOrgFunc( (void*)This, NewConnPointContainer_EnumConnectionPoints ));
//    }
//
//    HRESULT hr = E_NOTIMPL;
//    if( pOrgFunc )
//    {
//        hr = pOrgFunc( This, ppEnum );
//    }
//
//    return hr;
//}

void QueryEvents( IUnknown* pUnkSink, void* pCPC )
{
    std::wstring strResult;
    //std::basic_string<TCHAR> strResult;
    bool bKnown = false;

    _IUccApplicationSessionParticipantEvents* pUccAppSessPartEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccApplicationSessionParticipantEvents, (void**)&pUccAppSessPartEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("_IUccApplicationSessionParticipantEvents ");
        //ShowInfoTEXT("_IUccApplicationSessionParticipantEvents registered.");
        return;
    }

    _IUccAudioMediaChannelEvents* pUccAudioMediaChannelEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccAudioMediaChannelEvents, (void**)&pUccAudioMediaChannelEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccAudioMediaChannelEvents ");
        return;
    }

    _IUccAudioVideoMediaChannelEvents* pUccVideoMediaChannelEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccAudioVideoMediaChannelEvents, (void**)&pUccVideoMediaChannelEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccAudioVideoMediaChannelEvents ");
        return;
    }

    _IUccAudioVideoSessionEvents* pUccAudioVideoSessionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccAudioVideoSessionEvents, (void**)&pUccAudioVideoSessionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccAudioVideoSessionEvents ");
        return;
    }

    _IUccAudioVideoSessionParticipantEvents* pUccAudioVideoSessionPartEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccAudioVideoSessionParticipantEvents, (void**)&pUccAudioVideoSessionPartEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccAudioVideoSessionParticipantEvents ");
        return;
    }

    _IUccCategoryContextEvents* pUccCategoryContextEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccCategoryContextEvents, (void**)&pUccCategoryContextEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccCategoryContextEvents ");
        return;
    }

    _IUccCategoryInstanceEvents* p_IUccCategoryInstanceEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccCategoryInstanceEvents, (void**)&p_IUccCategoryInstanceEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccCategoryInstanceEvents ");
        return;
    }

    _IUccConferenceEntityViewCollectionEvents* p_IUccConferenceEntityViewCollectionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceEntityViewCollectionEvents, (void**)&p_IUccConferenceEntityViewCollectionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceEntityViewCollectionEvents ");
        //HookConfEntityViewCollectionEvents( p_IUccConferenceEntityViewCollectionEvents );
        CConfEntityViewCollectionSink::GetInstance()->HookEvents( PVOID(p_IUccConferenceEntityViewCollectionEvents) );
        return;
    }

    _IUccConferenceEntityViewEvents* p_IUccConferenceEntityViewEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceEntityViewEvents, (void**)&p_IUccConferenceEntityViewEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceEntityViewEvents ");

        IUccConferenceEntityView* pConfEntityView = 0;

        IConnectionPointContainer * pCPC1;

        ((IConnectionPoint*)pCPC)->GetConnectionPointContainer(&pCPC1);

        pCPC1->QueryInterface( IID_IUccConferenceEntityView, (void**)(&pConfEntityView) );

        //CHookedConfEntityView::GetInstance()->Hook( pConfEntityView );

        CConfEntityViewSink::GetInstance()->HookEvents( PVOID(p_IUccConferenceEntityViewEvents) );
        return;
    }

    _IUccConferenceManagerSessionEvents* p_IUccConferenceManagerSessionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceManagerSessionEvents, (void**)&p_IUccConferenceManagerSessionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceManagerSessionEvents ");
        return;
    }

    _IUccConferenceMediaChannelCollectionEvents* p_IUccConferenceMediaChannelCollectionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceMediaChannelCollectionEvents, (void**)&p_IUccConferenceMediaChannelCollectionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceMediaChannelCollectionEvents ");
        CConfMediaChannelCollection::GetInstance()->HookEvents( PVOID( p_IUccConferenceMediaChannelCollectionEvents ) );
        return;
    }

    _IUccConferenceMediaChannelEvents* p_IUccConferenceMediaChannelEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceMediaChannelEvents, (void**)&p_IUccConferenceMediaChannelEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceMediaChannelEvents ");
        CConfMediaChannel::GetInstance()->HookEvents( PVOID( p_IUccConferenceMediaChannelEvents ) );
        return;
    }

    _IUccConferenceSessionEvents* p_IUccConferenceSessionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceSessionEvents, (void**)&p_IUccConferenceSessionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceSessionEvents ");
        //HookConfSessEvents( p_IUccConferenceSessionEvents );
        CConfSessSink::GetInstance()->HookEvents( PVOID(p_IUccConferenceSessionEvents) );
        return;
    }

    /*_IUccConferenceSessionParticipantEndpointEvents* p_IUccConferenceSessionParticipantEndpointEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceSessionParticipantEndpointEvents, (void**)&p_IUccConferenceSessionParticipantEndpointEvents ) ) 
    {
    ShowInfoTEXT("_IUccConferenceSessionParticipantEndpointEvents registered");        
    }*/

    _IUccConferenceSessionParticipantEvents* p_IUccConferenceSessionParticipantEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccConferenceSessionParticipantEvents, (void**)&p_IUccConferenceSessionParticipantEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccConferenceSessionParticipantEvents ");
        //HookConferenceSessionParticipantEvents( p_IUccConferenceSessionParticipantEvents );
        IUccMediaChannel* pConfSessPart = 0;

        IConnectionPointContainer * pCPC1;

        ((IConnectionPoint*)pCPC)->GetConnectionPointContainer(&pCPC1);

        pCPC1->QueryInterface( IID_IUccConferenceSessionParticipant, (void**)(&pConfSessPart) );

       // CHookedConfSessPart::GetInstance()->Hook( pConfSessPart );
        CConfSessPartSink::GetInstance()->HookEvents( (PVOID)p_IUccConferenceSessionParticipantEvents );
        return;
    }

    _IUccContactEvents* p_IUccContactEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccContactEvents, (void**)&p_IUccContactEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccContactEvents ");
        return;
    }

    _IUccContainerEvents* p_IUccContainerEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccContainerEvents, (void**)&p_IUccContainerEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccContainerEvents ");
        return;
    }

    _IUccContainerMemberEvents* p_IUccContainerMemberEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccContainerMemberEvents, (void**)&p_IUccContainerMemberEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccContainerMemberEvents ");
        return;
    }

    _IUccEndpointEvents* p_IUccEndpointEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccEndpointEvents, (void**)&p_IUccEndpointEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccEndpointEvents ");
        return;
    }

    _IUccGroupEvents* p_IUccGroupEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccGroupEvents, (void**)&p_IUccGroupEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccGroupEvents ");
        return;
    }

    _IUccInstantMessagingSessionEvents* p_IUccInstantMessagingSessionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccInstantMessagingSessionEvents, (void**)&p_IUccInstantMessagingSessionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccInstantMessagingSessionEvents ");
        return;
    }

    _IUccInstantMessagingSessionParticipantEvents* p_IUccInstantMessagingSessionParticipantEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccInstantMessagingSessionParticipantEvents, (void**)&p_IUccInstantMessagingSessionParticipantEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccInstantMessagingSessionParticipantEvents ");
        return;
    }

    _IUccMediaChannelCollectionEvents* p_IUccMediaChannelCollectionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccMediaChannelCollectionEvents, (void**)&p_IUccMediaChannelCollectionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccMediaChannelCollectionEvents ");
        return;
    }

    /*_IUccMediaChannelDevicesEvents * p_IUccMediaChannelDevicesEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccMediaChannelDevicesEvents, (void**)&p_IUccMediaChannelDevicesEvents ) ) 
    {
    ShowInfoTEXT("_IUccMediaChannelDevicesEvents registered");
    }*/

    //_IUccMediaChannelEvents* p_IUccMediaChannelEvents = 0;
    //if( S_OK == pUnkSink->QueryInterface(  DIID__IUccMediaChannelEvents, (void**)&p_IUccMediaChannelEvents ) ) 
    //{
    //    bKnown = true;
    //    strResult += TEXT("+_IUccMediaChannelEvents ");
    //    IUccMediaChannel* pMediaChannel = 0;

    //    IConnectionPointContainer * pCPC1;

    //    HRESULT hr = ((IConnectionPoint*)pCPC)->GetConnectionPointContainer(&pCPC1);

    ///*    pCPC1->QueryInterface( IID_IUccMediaChannel, (void**)(&pMediaChannel) );

    //    CHookedMediaChannel::GetInstance()->Hook( pMediaChannel );
    //    CMediaChannelSink::GetInstance()->HookEvents( PVOID(p_IUccMediaChannelEvents) );*/
    //    return;
    //}

    _IUccMediaDeviceManagerEvents* p_IUccMediaDeviceManagerEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccMediaDeviceManagerEvents, (void**)&p_IUccMediaDeviceManagerEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccMediaDeviceManagerEvents ");
        return;
    }

    /*_IUccMediaEndpointEvents* p_IUccMediaEndpointEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccMediaEndpointEvents, (void**)&p_IUccMediaEndpointEvents ) ) 
    {
    ShowInfoTEXT("_IUccMediaEndpointEvents registered");
    }*/

    _IUccOperationManagerEvents* p_IUccOperationManagerEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccOperationManagerEvents, (void**)&p_IUccOperationManagerEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccOperationManagerEvents ");
        return;
    }

    _IUccPlatformEvents* p_IUccPlatformEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccPlatformEvents, (void**)&p_IUccPlatformEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccPlatformEvents ");
        return;
    }

    _IUccPresentityEvents* p_IUccPresentityEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccPresentityEvents, (void**)&p_IUccPresentityEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccPresentityEvents ");
        return;
    }

    _IUccPublicationEvent* p_IUccPublicationEvent = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccPublicationEvent, (void**)&p_IUccPublicationEvent ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccPublicationEvent ");
        return;
    }

    _IUccPublicationManagerEvents* p_IUccPublicationManagerEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccPublicationManagerEvents, (void**)&p_IUccPublicationManagerEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccPublicationManagerEvents ");
        return;
    }

    _IUccServerSignalingSettingsEvents* p_IUccServerSignalingSettingsEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccServerSignalingSettingsEvents, (void**)&p_IUccServerSignalingSettingsEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccServerSignalingSettingsEvents ");
        return;
    }

    _IUccSessionCallControlEvents* p_IUccSessionCallControlEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionCallControlEvents, (void**)&p_IUccSessionCallControlEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionCallControlEvents ");
        return;
    }

    _IUccSessionDescriptionEvaluator* p_IUccSessionDescriptionEvaluator = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID__IUccSessionDescriptionEvaluator, (void**)&p_IUccSessionDescriptionEvaluator ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionDescriptionEvaluator ");
        return;
    }

    _IUccSessionEvents * p_IUccSessionEvents  = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionEvents , (void**)&p_IUccSessionEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionEvents  ");
        CSessSink::GetInstance()->HookEvents( PVOID(p_IUccSessionEvents) );
        return;
    }

    _IUccSessionManagerEvents* p_IUccSessionManagerEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionManagerEvents, (void**)&p_IUccSessionManagerEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionManagerEvents ");
        return;
    }

    _IUccSessionParticipantCollectionEvents* p_IUccSessionParticipantCollectionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionParticipantCollectionEvents, (void**)&p_IUccSessionParticipantCollectionEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionParticipantCollectionEvents ");    

        CSessPartCollectionSink::GetInstance()->HookEvents( PVOID(p_IUccSessionParticipantCollectionEvents) );
        return;
    }

    _IUccSessionParticipantEndpointCollectionEvents* p_IUccSessionParticipantEndpointCollectionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionParticipantEndpointCollectionEvents, (void**)&p_IUccSessionParticipantEndpointCollectionEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionParticipantEndpointCollectionEvents ");
        CSessPartEndpintCollectionSink::GetInstance()->HookEvents( PVOID(p_IUccSessionParticipantEndpointCollectionEvents) );
        return;
    }

    _IUccSessionParticipantEvents* p_IUccSessionParticipantEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSessionParticipantEvents, (void**)&p_IUccSessionParticipantEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSessionParticipantEvents ");

        IUccSessionParticipant* pSessPart = 0;
        IConnectionPointContainer * pCPC1;

        ((IConnectionPoint*)pCPC)->GetConnectionPointContainer(&pCPC1);

        pCPC1->QueryInterface( IID_IUccSessionParticipant, (void**)(&pSessPart) );

     /*   IUccSignalingChannel* pSignalingChannel = 0;
        pSessPart->QueryInterface( IID_IUccSignalingChannel, (void**)&pSignalingChannel );*/
  //      if( pSignalingChannel )
  //      {
  //          CHookedSignalingChannel::GetInstance()->Hook( PVOID( pSignalingChannel ) );
  //          /*_IUccSignalingChannelEvents* pSigEvents = 0;
  //          if( hr = p_IUccSessionParticipantEvents->QueryInterface( DIID__IUccSignalingChannelEvents, (void**)&pSigEvents ) )
  //          {
  //          int i = 0;
		//	}*/
		//}
	/*	if( pSessPart )
		{
			CHookedSessPart::GetInstance()->Hook( PVOID(pSessPart) );
		}*/
		CSessPartSink::GetInstance()->HookEvents( PVOID(p_IUccSessionParticipantEvents) );
        return;
    }

    _IUccSignalingChannelEvents* p_IUccSignalingChannelEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSignalingChannelEvents, (void**)&p_IUccSignalingChannelEvents  ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSignalingChannelEvents ");
        return;
    }

    _IUccSubscriptionEvents* p_IUccSubscriptionEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccSubscriptionEvents, (void**)&p_IUccSubscriptionEvents ) ) 
    {
        bKnown = true;
        strResult += TEXT("+_IUccSubscriptionEvents ");
        return;
    }

    /*_IUccUserSearchQueryEvents* p_IUccUserSearchQueryEvents = 0;
    if( S_OK == pUnkSink->QueryInterface(  DIID__IUccUserSearchQueryEvents, (void**)&p_IUccUserSearchQueryEvents ) ) 
    {
    ShowInfoTEXT("_IUccUserSearchQueryEvents registered");
    }*/

    //AppShare
    IBaseObject* pIBaseObject = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IBaseObject, (void**)&pIBaseObject ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IBaseObject registered"));
        return;
    }

    ICollaborateAppShare* pICollaborateAppShare = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_ICollaborateAppShare, (void**)&pICollaborateAppShare ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("ICollaborateAppShare registered"));
        return;
    }

    ICollaborateScraper* pICollaborateScraper = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_ICollaborateScraper, (void**)&pICollaborateScraper ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("ICollaborateScraper registered"));
        return;
    }

    ICollaborateScraperFile* pICollaborateScraperFile = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_ICollaborateScraperFile, (void**)&pICollaborateScraperFile ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("ICollaborateScraperFile registered"));
        return;
    }

    IEventScraperState* pIEventScraperState = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventScraperState, (void**)&pIEventScraperState ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventScraperState registered"));
        return;
    }

    IEventScraperQoS* pIEventScraperQoS = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventScraperQoS, (void**)&pIEventScraperQoS ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventScraperQoS registered"));
        return;
    }

    IEventScraperController* pIEventScraperController = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventScraperController, (void**)&pIEventScraperController ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventScraperController registered"));
        return;
    }

    IEventViewerState* pIEventViewerState = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventViewerState, (void**)&pIEventViewerState ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventViewerState registered"));
        return;
    }

    IEventViewerCaps* pIEventViewerCaps = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventViewerCaps, (void**)&pIEventViewerCaps ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventViewerCaps registered"));
        return;
    }

    IEventViewerQoS* pIEventViewerQoS = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventViewerQoS, (void**)&pIEventViewerQoS ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventViewerQoS registered"));
        return;
    }

    //Collaborate
    IBaseEvent* pIBaseEvent = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IBaseEvent, (void**)&pIBaseEvent ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IBaseEvent registered"));
        return;
    }

    IDesktopEvent* pIDesktopEvent = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IDesktopEvent, (void**)&pIDesktopEvent ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IDesktopEvent registered"));
        return;
    }

    IEventDisplaySize* pIEventDisplaySize = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventDisplaySize, (void**)&pIEventDisplaySize ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventDisplaySize registered"));
        return;
    }

    IEventRescindControl* pIEventRescindControl = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventRescindControl, (void**)&pIEventRescindControl ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventRescindControl registered"));
        return;
    }

    IEventApplicationClosed* pIEventApplicationClosed = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventApplicationClosed, (void**)&pIEventApplicationClosed ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventApplicationClosed registered"));
        return;
    }

    IEventSnapshotTaken* pIEventSnapshotTaken = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventSnapshotTaken, (void**)&pIEventSnapshotTaken ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventSnapshotTaken registered"));
        return;
    }

    IEventSnapshotClosed* pIEventSnapshotClosed = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventSnapshotClosed, (void**)&pIEventSnapshotClosed ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventSnapshotClosed registered"));
        return;
    }

    IEventFramePosChanged* pIEventFramePosChanged = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventFramePosChanged, (void**)&pIEventFramePosChanged ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventFramePosChanged registered"));
        return;
    }

    IEventSnapshotBufferChanged* pIEventSnapshotBufferChanged = 0;
    if( S_OK == pUnkSink->QueryInterface(  IID_IEventSnapshotBufferChanged, (void**)&pIEventSnapshotBufferChanged ) ) 
    {
        bKnown = true;
        ShowInfo(TEXT("IEventSnapshotBufferChanged registered"));
        return;
    }

    if( !bKnown )
    {
        //__asm int 3;
        CUnknownSink::GetInstance()->HookEvents( PVOID(pUnkSink) );
    }


    if( !strResult.empty() )
    {
        ShowInfo( strResult.c_str() );
        strResult.clear();
    }
}
