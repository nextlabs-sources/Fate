#pragma once

//#include <ocidl.h>
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

// platform
typedef HRESULT (__stdcall *FUNC_PLATFORM_INITIALIZE)(
	IUccPlatform* This,
	BSTR bstrApplicationName,
	struct IUccContext* pContext 
);

typedef HRESULT (__stdcall* FUNC_PLATFORM_CREATEENDPOINT)(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
);

typedef HRESULT (__stdcall* FUNC_PLATFORM_CREATEPROXYENDPOINT)(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccEndpoint * pControllingEndpoint,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
);

typedef HRESULT (__stdcall*  FUNC_PLATFORM_SHUTDOWN)(
	IUccPlatform* This,
	struct IUccOperationContext * pOperationContext 
);

HRESULT __stdcall My_NewPlatform_Initialize(
	IUccPlatform* This,
	BSTR bstrApplicationName,
	struct IUccContext * pContext 
);

HRESULT __stdcall My_NewPlatform_CreateEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
);

HRESULT __stdcall My_NewPlatform_CreateProxyEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccEndpoint * pControllingEndpoint,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
);

HRESULT __stdcall My_NewPlatform_Shutdown(
	IUccPlatform* This,
	struct IUccOperationContext * pOperationContext 
);

HRESULT __stdcall Try_NewPlatform_Initialize(
	IUccPlatform* This,
	BSTR bstrApplicationName,
	struct IUccContext * pContext 
	);

HRESULT __stdcall Try_NewPlatform_CreateEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
	);

HRESULT __stdcall Try_NewPlatform_CreateProxyEndpoint(
	IUccPlatform* This,
	enum UCC_ENDPOINT_TYPE eType,
	struct IUccEndpoint * pControllingEndpoint,
	struct IUccUri * pUri,
	BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccEndpoint * * ppEndpoint 
	);

HRESULT __stdcall Try_NewPlatform_Shutdown(
	IUccPlatform* This,
	struct IUccOperationContext * pOperationContext 
	);



struct OLD_PLATFORM_FUNCS
{
	void* Vtable;
	FUNC_PLATFORM_INITIALIZE OriInitialize;
	FUNC_PLATFORM_INITIALIZE Initialize;
	FUNC_PLATFORM_CREATEENDPOINT OriCreateEndpoint;
	FUNC_PLATFORM_CREATEENDPOINT CreateEndpoint;
	FUNC_PLATFORM_CREATEPROXYENDPOINT OriCreateProxyEndpoint;
	FUNC_PLATFORM_CREATEPROXYENDPOINT CreateProxyEndpoint;
	FUNC_PLATFORM_SHUTDOWN OriShutdown;
	FUNC_PLATFORM_SHUTDOWN Shutdown;
};

typedef std::map<IUccPlatform*,OLD_PLATFORM_FUNCS> MAP_PLATFORM;

extern MAP_PLATFORM gMapPlatform;
extern Mutex gMutexMapPlatform;

// IUccEndpoint
typedef HRESULT (__stdcall *FUNC_ENDPOINT_ENABLE)(
	IUccEndpoint* This
);

typedef HRESULT (__stdcall *FUNC_ENDPOINT_DISABLE)(
	IUccEndpoint* This
);

HRESULT __stdcall My_NewPlatform_Enable(
	IUccEndpoint* This
);

HRESULT __stdcall My_NewPlatform_Disable(
	IUccEndpoint* This
);

HRESULT __stdcall My_NewPlatform_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
);

HRESULT __stdcall Try_NewPlatform_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	);

struct OLD_ENDPOINT_FUNCS
{
	void* Vtable;
	FUNC_IUNKNOWN_QUERYINTERFACE OriQueryInterface;
	FUNC_IUNKNOWN_QUERYINTERFACE QueryInterface;
	FUNC_ENDPOINT_ENABLE OriEnable;
	FUNC_ENDPOINT_ENABLE Enable;
	FUNC_ENDPOINT_DISABLE OriDisable;
	FUNC_ENDPOINT_DISABLE Disable;
};

typedef std::map<IUccEndpoint*,OLD_ENDPOINT_FUNCS> MAP_ENDPOINT;

extern MAP_ENDPOINT gMapEndpoint;
extern Mutex gMutexMapEndpoint;

// IUccSessionParticipant
typedef HRESULT (__stdcall *FUNC_SESSIONPARTICIPANT_CREATEPARTICIPANTENDPOINT) (
	IUccSessionParticipant *This,
	struct IUccUri * pUri,
    BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
);

typedef HRESULT (__stdcall *FUNC_SESSIONPARTICIPANT_COPYPARTICIPANTENDPOINT) (
	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pInputParticipantEndpoint,
    struct IUccContext * pContext,
    struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
);

typedef HRESULT (__stdcall *FUNC_SESSIONPARTICIPANT_ADDPARTICIPANTENDPOINT) (
	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
    struct IUccOperationContext * pOperationContext 
);

typedef HRESULT (__stdcall *FUNC_SESSIONPARTICIPANT_REMOVEPARTICIPANTENDPOINT) (
 	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
    struct IUccOperationContext * pOperationContext 
);

HRESULT __stdcall My_NewSessionParticipant_CreateParticipantEndpoint(
	IUccSessionParticipant *This,
	struct IUccUri * pUri,
    BSTR bstrEndpointId,
    struct IUccContext * pContext,
    struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
);

	HRESULT __stdcall Try_NewSessionParticipant_CreateParticipantEndpoint(
		IUccSessionParticipant *This,
		struct IUccUri * pUri,
		BSTR bstrEndpointId,
	struct IUccContext * pContext,
	struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
		);

HRESULT __stdcall My_NewSessionParticipant_CopyParticipantEndpoint(
	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pInputParticipantEndpoint,
    struct IUccContext * pContext,
    struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
);

	HRESULT __stdcall Try_NewSessionParticipant_CopyParticipantEndpoint(
		IUccSessionParticipant *This,
	struct IUccSessionParticipantEndpoint * pInputParticipantEndpoint,
	struct IUccContext * pContext,
	struct IUccSessionParticipantEndpoint * * ppParticipantEndpoint 
		);

HRESULT __stdcall My_NewSessionParticipant_AddParticipantEndpoint(
	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
    struct IUccOperationContext * pOperationContext 
);

	HRESULT __stdcall Try_NewSessionParticipant_AddParticipantEndpoint(
		IUccSessionParticipant *This,
	struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
	struct IUccOperationContext * pOperationContext 
		);

HRESULT __stdcall My_NewSessionParticipant_RemoveParticipantEndpoint(
 	IUccSessionParticipant *This,
    struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
    struct IUccOperationContext * pOperationContext 
);

	HRESULT __stdcall Try_NewSessionParticipant_RemoveParticipantEndpoint(
		IUccSessionParticipant *This,
	struct IUccSessionParticipantEndpoint * pParticipantEndpoint,
	struct IUccOperationContext * pOperationContext 
		);

struct OLD_SESSION_PARTICIPANT_FUNCS
{
	void* Vtable;
	FUNC_SESSIONPARTICIPANT_CREATEPARTICIPANTENDPOINT OriCreateParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_CREATEPARTICIPANTENDPOINT CreateParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_COPYPARTICIPANTENDPOINT OriCopyParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_COPYPARTICIPANTENDPOINT CopyParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_ADDPARTICIPANTENDPOINT OriAddParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_ADDPARTICIPANTENDPOINT AddParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_REMOVEPARTICIPANTENDPOINT OriRemoveParticipantEndpoint;
	FUNC_SESSIONPARTICIPANT_REMOVEPARTICIPANTENDPOINT RemoveParticipantEndpoint;
};

typedef std::map<IUccSessionParticipant*,OLD_SESSION_PARTICIPANT_FUNCS> MAP_SESSION_PARTICIPANT;

extern MAP_SESSION_PARTICIPANT gMapSessionParticipant;
extern Mutex gMutexMapSessionParticipant;

// IUccSessionManager
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

HRESULT __stdcall My_NewSessionMan_CreateSession(
	IUccSessionManager* This,
	enum UCC_SESSION_TYPE enSessionType,
    struct IUccContext * pContext,
    struct IUccSession * * ppSession 
);

HRESULT __stdcall My_NewSessionMan_RegisterSessionDescriptionEvaluator(
	IUccSessionManager* This,
	struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
);

HRESULT __stdcall My_NewSessionMan_UnregisterSessionDescriptionEvaluator(
	IUccSessionManager* This
);

HRESULT __stdcall My_NewSessionMan_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
);

HRESULT __stdcall Try_NewSessionMan_CreateSession(
	IUccSessionManager* This,
	enum UCC_SESSION_TYPE enSessionType,
struct IUccContext * pContext,
struct IUccSession * * ppSession 
	);

HRESULT __stdcall Try_NewSessionMan_RegisterSessionDescriptionEvaluator(
	IUccSessionManager* This,
struct _IUccSessionDescriptionEvaluator * pSessionDescriptionEvaluator 
	);

HRESULT __stdcall Try_NewSessionMan_UnregisterSessionDescriptionEvaluator(
	IUccSessionManager* This
	);

HRESULT __stdcall Try_NewSessionMan_QueryInterface(
	IUnknown* This,
	const IID & riid,
	void **ppvObj
	);

struct OLD_SESSIONMAN_FUNCS
{
	void* Vtable;
	FUNC_IUNKNOWN_QUERYINTERFACE OriQueryInterface;
	FUNC_IUNKNOWN_QUERYINTERFACE QueryInterface;
	FUNC_SESSIONMAN_CREATESESSION OriCreateSession;
	FUNC_SESSIONMAN_CREATESESSION CreateSession;
	FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR OriRegisterSessionDescriptionEvaluator;
	FUNC_SESSIONMAN_REGISTERSESSIONDESCRIPTIONEVALUATOR RegisterSessionDescriptionEvaluator;
	FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR OriUnregisterSessionDescriptionEvaluator;
	FUNC_SESSIONMAN_UNREGISTERSESSIONDESCRIPTIONEVALUTOR UnregisterSessionDescriptionEvaluator;
};

typedef std::map<IUccSessionManager*,OLD_SESSIONMAN_FUNCS> MAP_SESSIONMAN;

extern MAP_SESSIONMAN gMapSessionMan;
extern Mutex gMutexMapSessionMan;

// IUccSession
//typedef HRESULT (__stdcall *FUNC_SESSION_GET_TYPE)(
//	IUccSession* This,
//	enum UCC_SESSION_TYPE * penType
//);
//
//typedef HRESULT (__stdcall *FUNC_SESSION_GET_CONTEXT)(
//	IUccSession* This,
//	struct IUccContext * * ppContext 
//);
//
//typedef HRESULT (__stdcall *FUNC_SESSION_GET_LOCALPARTICIPANT)(
//	IUccSession* This,
//	struct IUccSessionParticipant * * ppParticipant 
//);
//
//typedef HRESULT (__stdcall *FUNC_SESSION_GET_PARTICIPANTS)(
//	IUccSession* This,
//	struct IUccCollection * * ppParticipants 
//);

typedef HRESULT (__stdcall *FUNC_SESSION_CREATEPARTICIPANT)(
	IUccSession* This,
	struct IUccUri * pUri,
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
	enum UCC_REJECT_OR_TERMINATE_REASON enTerminateReason,
	struct IUccOperationContext * pOperationContext 
);

typedef HRESULT (__stdcall *FUNC_IM_SESSION_SENDMESSAGE)(
	IUccInstantMessagingSession* This,
	BSTR bstrContentType,
	BSTR bstrMessage,
	struct IUccOperationContext * pOperationContext
);

HRESULT __stdcall My_NewSession_CreateParticipant (
	IUccSession* This,
	struct IUccUri * pUri,
	struct IUccContext * pContext,
	struct IUccSessionParticipant * * ppParticipant 
);

HRESULT __stdcall My_NewSession_AddParticipant (
	IUccSession* This,
	struct IUccSessionParticipant * pParticipant,
	struct IUccOperationContext * pOperationContext 
);

HRESULT __stdcall My_NewSession_RemoveParticipant (
	IUccSession* This,
	struct IUccSessionParticipant * pParticipant,
	struct IUccOperationContext * pOperationContext 
);

HRESULT __stdcall My_NewSession_Terminate (
	IUccSession* This,
	enum UCC_REJECT_OR_TERMINATE_REASON enTerminateReason,
	struct IUccOperationContext * pOperationContext 
);


HRESULT __stdcall My_NewSession_QueryInterface(
		IUnknown* This,
		const IID & riid,
		void **ppvObj
		);

	HRESULT __stdcall Try_NewSession_CreateParticipant (
		IUccSession* This,
		struct IUccUri * pUri,
		struct IUccContext * pContext,
		struct IUccSessionParticipant * * ppParticipant 
		);

	HRESULT __stdcall Try_NewSession_AddParticipant (
		IUccSession* This,
		struct IUccSessionParticipant * pParticipant,
		struct IUccOperationContext * pOperationContext 
		);

	HRESULT __stdcall Try_NewSession_RemoveParticipant (
		IUccSession* This,
	struct IUccSessionParticipant * pParticipant,
	struct IUccOperationContext * pOperationContext 
		);

	HRESULT __stdcall Try_NewSession_Terminate (
		IUccSession* This,
		enum UCC_REJECT_OR_TERMINATE_REASON enTerminateReason,
		struct IUccOperationContext * pOperationContext 
		);

	HRESULT __stdcall Try_NewSession_QueryInterface(
		IUnknown* This,
		const IID & riid,
		void **ppvObj
		);

struct OLD_IM_SESSION_FUNCS
{
	void* Vtable;
	FUNC_IM_SESSION_SENDMESSAGE OriSendMessage; 
	FUNC_IM_SESSION_SENDMESSAGE SendMessage;
};

struct OLD_SESSION_FUNCS
{
	void* Vtable;	// 如果找到了vtable，不能再hook，否则会形成没有退出机制的递归
	FUNC_IUNKNOWN_QUERYINTERFACE OriQueryInterface;
	FUNC_IUNKNOWN_QUERYINTERFACE QueryInterface;
	FUNC_SESSION_CREATEPARTICIPANT OriCreateParticipant;
	FUNC_SESSION_CREATEPARTICIPANT CreateParticipant;
	FUNC_SESSION_ADDPARTICIPANT OriAddParticipant;
	FUNC_SESSION_ADDPARTICIPANT AddParticipant;	
	FUNC_SESSION_REMOVEPARTICIPANT OriRemoveParticipant;
	FUNC_SESSION_REMOVEPARTICIPANT RemoveParticipant;	
	FUNC_SESSION_TERMINATE OriTerminate;
	FUNC_SESSION_TERMINATE Terminate;	

	struct OLD_IM_SESSION_FUNCS IMSessionFuncs;
};
typedef std::map<CComPtr<IUccSession>,OLD_SESSION_FUNCS> MAP_SESSION;

extern MAP_SESSION gMapSession;
extern Mutex gMutexMapSession;

typedef std::map<CComPtr<IUccInstantMessagingSession>,OLD_IM_SESSION_FUNCS> MAP_IM_SESSION;

extern MAP_IM_SESSION gMapIMSession;
extern Mutex gMutexMapIMSession;

//SubscriptionMgr
typedef HRESULT (__stdcall *FUNC_SUBSCRIPTIONMGR_CREATESUBSCRIPTION) (
	IUccSubscriptionManager *This,
    struct IUccContext * pContext,
    struct IUccSubscription * * ppSubscription 
);

HRESULT __stdcall My_NewSubscriptionMgr_CreateSubscription (
	IUccSubscriptionManager *This,
    struct IUccContext * pContext,
    struct IUccSubscription * * ppSubscription 
);

HRESULT __stdcall Try_NewSubscriptionMgr_CreateSubscription (
	IUccSubscriptionManager *This,
	struct IUccContext * pContext,
	struct IUccSubscription * * ppSubscription 
);



struct OLD_SUBSCRIPTION_MGR_FUNCS 
{
	void *Vtable;
	FUNC_SUBSCRIPTIONMGR_CREATESUBSCRIPTION OriCreateSubscription;
	FUNC_SUBSCRIPTIONMGR_CREATESUBSCRIPTION CreateSubscription;
};

typedef std::map<IUccSubscriptionManager *, OLD_SUBSCRIPTION_MGR_FUNCS> MAP_SUBSCRIPTION_MGR;

//Subscription
typedef HRESULT (__stdcall *FUNC_SUBSCRIPTION_CREATEPRESENTITY) (
	IUccSubscription *This,
	struct IUccUri * pPresentityUri,
	struct IUccContext * pContext,
	struct IUccPresentity * * ppPresentity
	);

typedef HRESULT (__stdcall *FUNC_SUBSCRIPTION_ADDPRESENTITY) (
	IUccSubscription *This,
	struct IUccPresentity * pPresentity 
);

typedef HRESULT (__stdcall *FUNC_SUBSCRIPTION_ADDCATEGORYNAME) (
	IUccSubscription *This,
	BSTR bstrName 
);

HRESULT __stdcall My_NewSubscription_CreatePresentity(
	IUccSubscription *This,
	struct IUccUri * pPresentityUri,
	struct IUccContext * pContext,
	struct IUccPresentity * * ppPresentity
);

HRESULT __stdcall My_NewSubscription_AddPresentity(
	IUccSubscription *This,
	struct IUccPresentity *pPresentity
);

HRESULT __stdcall My_NewSubscription_AddCategoryName (
	IUccSubscription *This,
    BSTR bstrName 
);


HRESULT __stdcall Try_NewSubscription_CreatePresentity(
	IUccSubscription *This,
	struct IUccUri * pPresentityUri,
	struct IUccContext * pContext,
	struct IUccPresentity * * ppPresentity
);

HRESULT __stdcall Try_NewSubscription_AddPresentity(
	IUccSubscription *This,
struct IUccPresentity *pPresentity
	);

HRESULT __stdcall Try_NewSubscription_AddCategoryName (
	IUccSubscription *This,
	BSTR bstrName 
	);

struct OLD_SUBSCRIPTION_FUNCS 
{
	void *Vtable;
	FUNC_SUBSCRIPTION_CREATEPRESENTITY OriCreatePresentity;
	FUNC_SUBSCRIPTION_CREATEPRESENTITY CreatePresentity;
	FUNC_SUBSCRIPTION_ADDPRESENTITY OriAddPresentity;
	FUNC_SUBSCRIPTION_ADDPRESENTITY AddPresentity;
	FUNC_SUBSCRIPTION_ADDCATEGORYNAME OriAddCategoryName;
	FUNC_SUBSCRIPTION_ADDCATEGORYNAME AddCategoryName;
};

typedef std::map<IUccSubscription *, OLD_SUBSCRIPTION_FUNCS> MAP_SUBSCRIPTION;

//IUccConfSession
typedef HRESULT (__stdcall *FUNC_CONFSESSION_ENTER) (
	IUccConferenceSession *This,
	struct IUccUri * pUri,
    struct IUccOperationContext * pOperationContext
);

typedef HRESULT (__stdcall *FUNC_CONFSESSION_LEAVE) (
	IUccConferenceSession *This,
	struct IUccOperationContext * pOperationContext
);

typedef HRESULT (__stdcall *FUNC_CONFSESSION_SETPROPERTY) (
	IUccConferenceSession *This,
	enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
    VARIANT vPropertyValue,
    struct IUccOperationContext * pOperationContext
);

HRESULT __stdcall My_NewConfSession_Enter (
	IUccConferenceSession *This,
	struct IUccUri * pUri,
    struct IUccOperationContext * pOperationContext
);

	HRESULT __stdcall Try_NewConfSession_Enter (
		IUccConferenceSession *This,
		struct IUccUri * pUri,
	struct IUccOperationContext * pOperationContext
		);

HRESULT __stdcall My_NewConfSession_Leave (
	IUccConferenceSession *This,
	struct IUccOperationContext * pOperationContext
);

	HRESULT __stdcall Try_NewConfSession_Leave (
		IUccConferenceSession *This,
	struct IUccOperationContext * pOperationContext
		);

HRESULT __stdcall My_NewConfSession_SetProperty (
	IUccConferenceSession *This,
	enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
    VARIANT vPropertyValue,
    struct IUccOperationContext * pOperationContext
);

	HRESULT __stdcall Try_NewConfSession_SetProperty (
		IUccConferenceSession *This,
		enum UCC_CONFERENCE_SESSION_PROPERTY enPropertyId,
		VARIANT vPropertyValue,
	struct IUccOperationContext * pOperationContext
		);


struct OLD_CONF_SESSION_FUNCS 
{
	void *Vtable;
	FUNC_CONFSESSION_ENTER OriEnter;
	FUNC_CONFSESSION_ENTER Enter;
	FUNC_CONFSESSION_LEAVE OriLeave;
	FUNC_CONFSESSION_LEAVE Leave;
	FUNC_CONFSESSION_SETPROPERTY OriSetProperty;
	FUNC_CONFSESSION_SETPROPERTY SetProperty;
};

typedef std::map<IUccConferenceSession *, OLD_CONF_SESSION_FUNCS> MAP_CONF_SESSION;

//IConnectionPointContainer
typedef HRESULT (__stdcall *FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT) (
	IConnectionPointContainer *This,
	REFIID riid,
	IConnectionPoint ** ppCP
);

typedef HRESULT (__stdcall *FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS) (
	IConnectionPointContainer *This,
	IEnumConnectionPoints ** ppEnum
);
struct OLD_CONNPOINTCONTAINER_FUNCS
{
	void *Vtable;
	FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT oriFindConnectionPoint;
	FUNC_CONNPOINTCONTAINER_FINDCONNECTIONPOINT FindConnectionPoint;
	FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS oriEnumConnectionPoints;
	FUNC_CONNPOINTCONTAINER_ENUMCONNECTIONPOINTS EnumConnectionPoints;
};

typedef std::map<IConnectionPointContainer *, OLD_CONNPOINTCONTAINER_FUNCS> MAP_CONN_POINT_CONTAINER;

// IConnectionPoint
typedef HRESULT (__stdcall *FUNC_CONNPOINT_ADVISE) (
	IConnectionPoint *This,
	IUnknown * pUnk,
	DWORD * pdwCookie
);

struct OLD_CONNPOINT_FUNCS
{
	void *Vtable;
	FUNC_CONNPOINT_ADVISE oriAdvise;
	FUNC_CONNPOINT_ADVISE Advise;
};

typedef std::map<IConnectionPoint *, OLD_CONNPOINT_FUNCS> MAP_CONN_POINT;

//IUccSessionManagerEvents
typedef HRESULT (__stdcall *FUNC_SESSION_MAN_EVENTS_ONINCOMMINGSESSION) (
	_IUccSessionManagerEvents *This,
	IUccEndpoint *pEndPoint, 
    IUccIncomingSessionEvent *pEventData
);

typedef HRESULT (__stdcall *FUNC_SESSION_MAN_EVENTS_ONOUTGOINGSESSION) (
	_IUccSessionManagerEvents *This,
    IUccEndpoint *peventSource,
    IUccOutgoingSessionEvent *peventData
);

struct OLD_SESSION_MANAGER_EVENTS_FUNCS
{
	void *Vtable;
	FUNC_SESSION_MAN_EVENTS_ONINCOMMINGSESSION oriOnInCommingSession;
	FUNC_SESSION_MAN_EVENTS_ONINCOMMINGSESSION OnInCommingSession;
};

typedef std::map<_IUccSessionManagerEvents *, OLD_SESSION_MANAGER_EVENTS_FUNCS> MAP_SESSION_MANAGER_EVENTS;
//typedef std::map<void *, OLD_SESSION_MANAGER_EVENTS_FUNCS> MAP_SESSION_MANAGER_EVENTS;


void HookPlatform(IUccPlatform* pUccPlatform);
void HookEndpoint(IUccEndpoint* pUccEndpoint);
void HookSessionManager(IUccSessionManager* pUccSessionManager);
void HookSession(IUccSession* pUccSession);
void HookIMSession(IUccInstantMessagingSession* pIMSession);
void HookSubscriptionManager(IUccSubscriptionManager *pSubscriptionManager);
void HookSubscription(IUccSubscription *pSubscription);
void HookConferenceSession(IUccConferenceSession *pConfSession);
void HookSessionParticipant(IUccSessionParticipant *pSessionParticipant);
void HookConnectionPointContainer(IConnectionPointContainer *p);
void HookConnectionPoint(IConnectionPoint *pConnectionPoint);
void HookSessionManagerEvents(_IUccSessionManagerEvents *pSessionManEvents);

bool DoSetClipboardDataEval(); // Do SetClipboardData Evaluation, Added By Jacky.Dong 2011-11-23