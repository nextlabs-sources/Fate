#pragma once
#include "Helper.h"

// IUnknown
typedef HRESULT (__stdcall *FUNC_IUNKNOWN_QUERYINTERFACE)(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
);

typedef ULONG (__stdcall* FUNC_IUNKNOWN_ADDREF)(
	IUnknown* This
);

typedef ULONG (__stdcall* FUNC_IUNKNOWN_RELEASE)(
	IUnknown* This
);

typedef HRESULT (__stdcall* IDispatchInvokeFunc )
( 
 PVOID pThis,
        /* [in] */ DISPID dispIdMember,
        /* [in] */ REFIID riid,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [out][in] */ DISPPARAMS *pDispParams,
        /* [out] */ VARIANT *pVarResult,
        /* [out] */ EXCEPINFO *pExcepInfo,
        /* [out] */ UINT *puArgErr
) ;

void QueryEvents( IUnknown* pUnkSink, void* pCPC );

class CHookedPlatform: public CHookBase
{
    INSTANCE_DECLARE( CHookedPlatform );
public:

    void Hook( void* pPlatform );

public:

    // platform
    typedef HRESULT (__stdcall *FUNC_PLATFORM_INITIALIZE)(
        IUccPlatform* This,
        BSTR bstrApplicationName,
    struct IUccContext* pContext 
        );

    typedef HRESULT (__stdcall* FUNC_PLATFORM_CREATEENDPOINTCONTEXT)(
        IUccPlatform* This,
   PVOID ppContext 
        );

    typedef HRESULT (__stdcall* FUNC_PLATFORM_CREATEENDPOINT)(
        IUccPlatform* This,
        BSTR bstrEndpointUri,
        BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccEndpoint * * ppEndpoint 
        );

    typedef HRESULT (__stdcall* FUNC_PLATFORM_CREATEPROXYENDPOINT)(
        IUccPlatform* This,
    struct IUccEndpoint * pControllingEndpoint,
        BSTR bstrEndpointUri,
        BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccEndpoint * * ppEndpoint 
        );

    typedef HRESULT (__stdcall*  FUNC_PLATFORM_SHUTDOWN)(
        IUccPlatform* This,
    struct IUccOperationContext * pOperationContext 
        );

    static HRESULT __stdcall NewPlatform_Initialize(
        IUccPlatform* This,
        BSTR bstrApplicationName,
    struct IUccContext * pContext 
        );

    static HRESULT __stdcall NewPlatform_CreateEndPointContext(
        IUccPlatform* This,
   PVOID ppContext 
        );

    static HRESULT __stdcall NewPlatform_CreateEndpoint(
        IUccPlatform* This,
        BSTR bstrEndpointUri,
        BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccEndpoint * * ppEndpoint 
        );

    static HRESULT __stdcall NewPlatform_CreateProxyEndpoint(
        IUccPlatform* This,
    struct IUccEndpoint * pControllingEndpoint,
        BSTR bstrEndpointUri,
        BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccEndpoint * * ppEndpoint 
        );

    static HRESULT __stdcall NewPlatform_Shutdown(
        IUccPlatform* This,
    struct IUccOperationContext * pOperationContext 
        );
};

class CHookedEndpoint: public CHookBase
{
    INSTANCE_DECLARE( CHookedEndpoint );
public:

    void Hook( void* pEndPoint );

public:

    typedef HRESULT (__stdcall *FUNC_ENDPOINT_ENABLE)(
        IUccEndpoint* This,
    struct IUccOperationContext * pOperationContext
        );

    typedef HRESULT (__stdcall *FUNC_ENDPOINT_DISABLE)(
        IUccEndpoint* This,
    struct IUccOperationContext * pOperationContext
        );

    static HRESULT __stdcall NewEndpoint_Enable(
        IUccEndpoint* This,
    struct IUccOperationContext * pOperationContext
        );

    static HRESULT __stdcall NewEndpoint_Disable(
        IUccEndpoint* This,
    struct IUccOperationContext * pOperationContext
        );

    static HRESULT __stdcall NewEndpoint_QueryInterface(
        IUnknown* This,
        const IID & riid,
        void **ppvObj
        );
};

// IUccSessionManager
class CHookedSessMgr: public CHookBase
{
    INSTANCE_DECLARE( CHookedSessMgr );
public:

    void Hook( void* pSessMgr );

public:

    typedef HRESULT (__stdcall *FUNC_SESSIONMAN_CREATESESSIONCONTEXT)(
        IUccSessionManager* This,
        enum UCC_SESSION_TYPE enSessionType,
    struct IUccContext * * ppContext 
        );

    typedef HRESULT (__stdcall *FUNC_SESSIONMAN_CREATESESSION)(
        IUccSessionManager* This,
        enum UCC_SESSION_TYPE enSessionType,
    struct IUccContext * pContext,
    struct IUccSession * * ppSession 
        );

    typedef HRESULT (__stdcall *FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR)(
        IUccSessionManager* This,
    struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
        );

    typedef HRESULT (__stdcall* FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR)(
        IUccSessionManager* This
        );

    static HRESULT __stdcall NewSessionMan_CreateSessionContext(
        IUccSessionManager* This,
        enum UCC_SESSION_TYPE enSessionType,
    struct IUccContext * * ppContext 
        );

    static HRESULT __stdcall NewSessionMan_CreateSession(
        IUccSessionManager* This,
        enum UCC_SESSION_TYPE enSessionType,
    struct IUccContext * pContext,
    struct IUccSession * * ppSession 
        );

    static HRESULT __stdcall NewSessionMan_RegisterSessionDescriptionEvaluator(
        IUccSessionManager* This,
    struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
        );

    static HRESULT __stdcall NewSessionMan_UnregisterSessionDescriptionEvaluator(
        IUccSessionManager* This
        );
};

// IUccSession
class CHookedSess: public CHookBase
{
    INSTANCE_DECLARE( CHookedSess );
public:

    void Hook( void* pSess );

public:

    typedef HRESULT (__stdcall *FUNC_SESSION_CREATEPARTICIPANTCONTEXT)(
        IUccSession* This,
    struct IUccContext * * ppContext 
        );

    typedef HRESULT (__stdcall *FUNC_SESSION_CREATEPARTICIPANT)(
        IUccSession* This,
        BSTR bstrUri,
    struct IUccContext * pContext,
    struct IUccSessionParticipant * * ppParticipant 
        );

    typedef HRESULT (__stdcall *FUNC_SESSION_ADDPARTICIPANT)(
        IUccSession* This,
    struct IUccSessionParticipant * pParticipant,
    struct IUccOperationContext * pOperationContext 
        );

    typedef HRESULT (__stdcall *FUNC_SESSION_REMOVEPARTICIPANT)(
        IUccSession* This,
    struct IUccSessionParticipant * pParticipant,
    struct IUccOperationContext * pOperationContext 
        );

    typedef HRESULT (__stdcall *FUNC_SESSION_TERMINATE)(
        IUccSession* This,
        enum UCC_TERMINATE_REASON enTerminateReason,
    struct IUccOperationContext * pOperationContext 
        );


    static HRESULT __stdcall NewSession_CreateParticipantContext (
        IUccSession* This,
    struct IUccContext * * ppContext 
        );

    static HRESULT __stdcall NewSession_CreateParticipant (
        IUccSession* This,
        BSTR bstrUri,
    struct IUccContext * pContext,
    struct IUccSessionParticipant * * ppParticipant 
        );

    static HRESULT __stdcall NewSession_AddParticipant (
        IUccSession* This,
    struct IUccSessionParticipant * pParticipant,
    struct IUccOperationContext * pOperationContext 
        );

    static HRESULT __stdcall NewSession_RemoveParticipant (
        IUccSession* This,
    struct IUccSessionParticipant * pParticipant,
    struct IUccOperationContext * pOperationContext 
        );

    static HRESULT __stdcall NewSession_Terminate (
        IUccSession* This,
        enum UCC_TERMINATE_REASON enTerminateReason,
    struct IUccOperationContext * pOperationContext 
        );
};

// IConnectionPoint
class CHookedConnPoint: public CHookBase
{
    INSTANCE_DECLARE( CHookedConnPoint );
public:

    void Hook( void* pConnPoint );

public:

    typedef HRESULT (__stdcall *FUNC_CONNPOINT_ADVISE) 
        (
        IConnectionPoint *This,
        IUnknown * pUnk,
        DWORD * pdwCookie
        );

    typedef HRESULT (__stdcall *FUNC_CONNPOINT_UNADVISE) 
        (
        IConnectionPoint *This,
        DWORD * pdwCookie
        );

    static HRESULT	__stdcall NewConnPoint_Advise( IConnectionPoint *This,
        IUnknown * pUnk,
        DWORD * pdwCookie );
    static HRESULT	__stdcall NewConnPoint_Unadvise( IConnectionPoint *This,
        DWORD * pdwCookie );
};

class CHookedIMSess: public CHookBase
{
    INSTANCE_DECLARE( CHookedIMSess );
public:

    void Hook( void* pIMSess );

public:

    typedef HRESULT (__stdcall *FUNC_IM_SESSION_SENDMESSAGE)(
        IUccInstantMessagingSession* This,
        BSTR bstrContentType,
        BSTR bstrMessage,
    struct IUccOperationContext * pOperationContext
        );

    static HRESULT __stdcall NewIM_Session_SendMessage (
        IUccInstantMessagingSession* This,
        BSTR bstrContentType,
        BSTR bstrMessage,
    struct IUccOperationContext * pOperationContext 
        );

    struct OLD_IM_SESSION_FUNCS
    {
        void* Vtable;
        FUNC_IM_SESSION_SENDMESSAGE OriSendMessage; 
        FUNC_IM_SESSION_SENDMESSAGE SendMessage;
    };
};

//IConnectionPointContainer
class CHookedConnPointContainer: public CHookBase
{
    INSTANCE_DECLARE( CHookedConnPointContainer );
public:

    void Hook( void* pConnPointContainer );

public:

    typedef HRESULT (__stdcall *FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT) 
        (
        IConnectionPointContainer *This,
        REFIID riid,
        IConnectionPoint ** ppCP
        );

    typedef HRESULT (__stdcall *FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS) (
        IConnectionPointContainer *This,
        IEnumConnectionPoints ** ppEnum
        );

    static HRESULT	__stdcall NewConnPointContainer_FindConnectionPoint(
        IConnectionPointContainer *This,
        REFIID riid,
        IConnectionPoint ** ppCP);
    static HRESULT	__stdcall NewConnPointContainer_EnumConnectionPoints(
        IConnectionPointContainer *This,
        IEnumConnectionPoints ** ppEnum);
};

void HookPlatform(IUccPlatform* pUccPlatform);
void HookEndpoint(IUccEndpoint* pUccEndpoint);
void HookSessionManager(IUccSessionManager* pUccSessionManager);
void HookSession(IUccSession* pUccSession);
void HookIMSession(IUccInstantMessagingSession* pIMSession);
void HookConnectionPointContainer(IConnectionPointContainer *p);
void HookConnectionPoint(IConnectionPoint *pConnectionPoint);
void HookUccConferenceSessionParticipant( IUccConferenceSessionParticipant* pIUccConferenceSessionParticipant );

