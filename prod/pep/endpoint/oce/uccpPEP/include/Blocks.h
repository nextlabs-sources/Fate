#ifndef _BLOCKS_H_

#define _BLOCKS_H_



#include <Winsock2.h>



#pragma warning(push)

#pragma warning(disable: 6386)

#include <ws2tcpip.h>

#pragma warning(pop)



#include <hash_map>

#include <set>

#include <string>

#include "CEsdk.h"

#include "Platform.h"





#include "eframework\platform\cesdk_loader.hpp"





#pragma once

using namespace std;

#define OCE_EVALUATION_TIMEOUT 30 * 1000 // oce evaluation timeout, 30 seconds.





typedef struct {

	wstring fileName; //fileName with full path

	HANDLE fileHandle; //handle of file

	double openTime; //the timestamp of opening this file

} FileInfo;



typedef struct {

	double evalTime; //the timestamp of last copy evaluation 

	bool bAllow; //if true, the last evaluation is allow

} EvalResult;



typedef struct {

	std::set<wstring> disclaimers; //The disclaimers to the group

	FUNC_IM_SESSION_SENDMESSAGE sendMsgFunc; //The pointer to the func to send disclaimer

	CComPtr<IUccInstantMessagingSession> imSession; //The IM session for the group session

	struct IUccOperationContext * pOperationContext; //The operation context of IM session

	bool bSendMsg; //If it is true, disclaimer needs to be sent

} GroupDisclaimer;



typedef struct {

	wstring id; //The id of participant

	CComPtr<IUccSession> session; //the application session of the invitation to group session

	CComPtr<IUccSessionParticipant> pParticipant; //The pointer of participant

	struct IUccOperationContext * pOperationContext; //The operation context

	FUNC_SESSION_ADDPARTICIPANT addParticipantFunc; //The function of adding participant

	bool bAdded; //If true, this participant has been added to the group

} GroupParticipant;



typedef struct {

	HWND winHandle; // the handle of group session window

	IUccConferenceSession *confSession; //the pointer to the group session

	CComPtr<IUccSessionParticipant> localParticipant;

	enum UCC_SESSION_TYPE sessionType; //session type

	int numParticipants; //number of participants

	wstring confURI; //id of conference. sip:xxx;gruu;opaque=app:conf:focus:id:xxxx



	GroupDisclaimer groupDisclaimer; //disclaimer to group

	std::map<wstring, GroupParticipant> groupParticipants; // group participants



	std::set<wstring> warnObligations; //The warning obligations



	bool bFristEval; //If true, this is first time to do evaluation

	bool bLastEvalResult; //Store the most last eval result: true is Allow, false is Deny

} GroupSessionInfo;



class PolicyEval

{

private:

	BSTR endPointSip; //The SIP of current sign-in user

	CEHandle oceHandle; //connection handle to policy controller



	HANDLE disclaimerMsgMutex; //The mutex for disclaimer map

	stdext::hash_map <wstring, wstring> disclaimerMsgs; //disclaimer messages map



	HANDLE sessionWindowMapMutex; //The mutex for session and HWND mapping

	stdext::hash_map<HWND, CComPtr<IUccSession>> sessionWindowMap; //HWND and session mapping



	HANDLE cachedCopyEvalMutex; //The mutex for cached copy evaluation results

	stdext::hash_map<HWND, EvalResult> cachedCopyEvalResults; //the results of threads' copy eval



	HANDLE fileInfoMapMutex; //The mutex for file name and open file info mapping

	stdext::hash_map<wstring, FileInfo> fileInfoMap; //file name and open file info mapping



	HANDLE isIMFirstMsgSetMutex; //The mutex for the set that if first msg of IM session

	std::set<CComPtr<IUccInstantMessagingSession>> isIMFirstMsgSet; //the set that if first msg of IM session



	HANDLE groupSetMutex; //The mutex for the set of group chat

	std::map<HWND, GroupSessionInfo> groupSet; //the table of group chat

	HWND outstandWnd; //Store the group session that is currently handled



	HANDLE incomingGroupSetMutex; //The mutex for the set of group chat from incoming session

	std::map<CComPtr<IUccSession>, GroupSessionInfo> incomingGroupSet; //the table of group chat from incoming session

	CComPtr<IUccSession> activeIncomingSession; //Store the incoming group session that is currently handled

	enum UCC_SESSION_TYPE activeIncomingSessionType; //session type





	struct in_addr  hostIPAddr; //Host ip address



    BSTR currentWinLogonUserSid; //The SID of the current Windows Logon user.



	std::wstring processName; //the current process name including path



	bool bInitFailed; //if it is true, initialization failed; everything allowed



	//Get local host ip address

	bool GetHostIPAddr();



	//Get Windows logon user SID

	bool GetWinLogonUserSid();



	//Set up the connection to policy controller

	bool SetupConnection();



	//Handle Disclaimer obligation

	void HandleDisclaimer(std::map<wstring, wstring> &obligations,

						std::set<wstring> &disclaimerSet,

						std::set<wstring> &disclaimerPreKeys);



	//Handle warn obligation

	void HandleWarnObligation(std::map<wstring, wstring> &obligations,

							  std::set<wstring> &warnPreKeys,

							  std::set<wstring> &warnObligationSet,

							  int numParticipant,

							  bool &bPopupWindow,

							  bool &bUserCancel);

	

	//Parse the obligationthe disclaimer and warn message from policy evaluation result

	//If there is warn obligation and the user chose "Cancel", the policy evaluation

	//result will be change to "Deny". 

	void ParseObligation(std::set<wstring> &disclaimerSet,

						std::set<wstring> &warnObligationSet,

						CEEnforcement_t *pEnforcement,

						int realNumParticipant, 							   

						bool &bUserCancel, 

						bool &bPopupWindow,

						bool bIgnoreDisclaimer=false);



private:

	nextlabs::cesdk_loader m_sdkLoader;



public:

	//Constructor

	PolicyEval(); 



	//Destructor

	~PolicyEval();

	//Set up the connection to policy controller

	void SetupProcessName(WCHAR *name){

		if(name) processName=name;

	}



	//Assign the value of "in" to the member endPointSip

	void SetEndPointSip(BSTR in);


	BSTR GetEndPointSip() const{return endPointSip;};

	//Add a disclaimer message for a receipt

	void AddDisclaimer(BSTR receipt, const WCHAR *msg);



	//Fetch a disclaimer message for a receipt if the msg exists and remove it from 

	//the map after fetch it

	void FetchDisclaimer(BSTR receipt, BSTR *msg);



	//Evaluate if the current endPointSip can communicate with participant

	bool EvalAddParticipant(BSTR participant, enum UCC_SESSION_TYPE SessionType, bool bObligation=true);



	//Evaluate if the current endPointSip can transfer a file to the participant

	bool EvalTransferFile(BSTR participant, wchar_t * fileName); 



	//Evaluate if it is allowed to copy data to clipboard for this endPointSip

	bool EvalCopyAction();



	//Evaluate if livemeeting is allowed

	bool EvalLivemeeting(wchar_t *confURI);



	//Add a new thread and session pair to sessionThreadMap

	void AddSessionWindowPair(HWND w, CComPtr<IUccSession>);



	//Remove a thread and session pair to sessionThreadMap

	void RemoveSessionWindowPair(HWND w, CComPtr<IUccSession>);



	//Fetch the session of a thread

	CComPtr<IUccSession> GetSessionofWindow(HWND w);



	//Add thread ID and open file info mapping

	void AddFileInfoMapping(DWORD tid, LPCWSTR fileName, HANDLE handle);

	

	//Add a thread and its copy evaluation result pair to cachedCopyEvalResults

	void CacheCopyEvalResult(HWND hWnd, bool bAllow);



	//Remove a thread and its copy evaluation result pair 

	void RemoveCachedCopyEvalResults(HWND hWnd);



	//Fetch the cached copy eval result of a thread

	//When this function reture false, it means that the result is unknown

	//If the last eval time is older than 10 sec, discard the result and return false;

	bool GetCachedCopyEvalResult(HWND hWnd, bool &bAllow);



	//Evaluate if it is allowed to send message among the recipients

	bool EvalSendMsgAction(CComPtr<IUccSession> pSession, wstring &disclaimer);



	//Add to first IM session set

	void AddToIMFirstMsg(CComPtr<IUccInstantMessagingSession> pIMSession);



	//Is IM First message

	bool IsIMSessionFirstMsg(CComPtr<IUccInstantMessagingSession> pIMSession);



	//Evaluate if the new participant can in the group meeting of tid.

	//bool EvalAddParticipantToGroup(DWORD tid, BSTR participant, 

	//							   UCCP::UCC_SESSION_TYPE SessionType);



	//The procedure of invoking a group IM chat is

	//1. Create A new session (type=UCCST_CONFERENCE) including local participant

	//2. Call IUccConferenceSession::Enter. Here, we get the window handle (HWND),

	//   the window title to get the number of participant, and create a new 

	//   GroupdSessionInfo entry and add it to the groupSet.   

	void InitGroupSession(HWND winHandle, IUccConferenceSession *session,

						int numParticipant, BSTR bstrConfURI);

	//3. Create a new session (type=UCCST_INSTANT_MESSAGE), add a participant as 

	//"sip:<local-participant>;gruu;opaque=app:conf:chat:idxxx" and send message

	//including "This is a multi-part message". Here, we store this function information

	//to the GroupDisclaimer structure of this GroupSessionInfo. We will use this information

	//to send out the disclaimer later.

	void ModifySessionType(HWND winHandle, UCC_SESSION_TYPE t);

	// Added By Jacky.Dong 2011-11-21
	// ------------------------------
	//4. Get SessionType By HWND
	bool GetSessionTypeByHWND(HWND winHandle, UCC_SESSION_TYPE* sessionType);
	// ------------------------------

	void StoreGroupSendMsgFunc(HWND winHandle, FUNC_IM_SESSION_SENDMESSAGE f, 

								CComPtr<IUccInstantMessagingSession> imSession, 

								struct IUccOperationContext * pOperationContext);

	void SendGroupDisclaimer(HWND winHandle, CComPtr<IUccInstantMessagingSession> s, 

					BSTR bstrContentType, 

					struct IUccOperationContext * pOperationContext);

	//If a session's participant is calling CopyParticipantEndpoint, we check if this

	//session is in group chat set. If yes, this participant is the existing allowed member 

	//so we add this participant to this group 

	void AddExistingMemberToGroupChatSession(HWND &winHandle, CComPtr<IUccSessionParticipant> pParticipant);

	//4. Create a new session (type=UCCST_APPLICATION) for each following participant and

	//    call addParticipant to send out invitation. Here, we hold the call addparticipant 

	//    and store the information to GroupParticipant of this GroupSessionInfo. At adding

	//    the No. "numParticipant", we do group participant evaluation and send out the 

	//    disclaimer if it exists.  

	//    If this is the first time to do policy evaluation for this group and the result is

	//    deny (because two (or more) of them can't talk to each other), we cancell the whole

	//    conferce by calling IUccConferenceSession::Leave. 

	//    If this is not the first time to do policy evaluation, we won't add the new

	//    participant to the group. 

	bool IsInGroupChatSession(HWND &winHandle);

	bool IsSessionTypeChanged(HWND winHandle, CComPtr<IUccSession> session, WCHAR *newParticipantId);

	void AddParticipantToGroupChat(HWND winHandle, WCHAR *id, CComPtr<IUccSession> session, 

									struct IUccSessionParticipant * pParticipant,

									struct IUccOperationContext * pOperationContext,

									FUNC_SESSION_ADDPARTICIPANT addParticipantFunc,

									bool &bAddNow);

	void UpdateGroupParticipantNum(HWND winHandle, int num);

	bool IsTimeToDoEvaluation(HWND winHandle);

	bool DoGroupChatEvaluation(HWND winHandle, WCHAR *lastParticipant);


	/*
	
	add this function for bug 15968.

	this function is an updated version of DoGroupChatEvaluation.

	the difference between this function and DoGroupChatEvaluation is:
	
	it will output 
	
	bCancel -- if there is warn user obligation and user choose to cancel to operation
	*ppConfSession	-- current IUccConferenceSession interface pointer, caller need it to leave the session if user choose to cancel the operation
	*phCurrentWnd	-- current chat window, caller need it to close the window if user choose to cancel the operation, we need to close the window is a workaround, 
							if we don't close the window, there may be other issue coming up.
	
	*/
	bool DoGroupChatEvaluation_V2(HWND winHandle, WCHAR *lastParticipant, bool& bCancel, IUccConferenceSession** ppConfSession, HWND* phCurrentWnd );



	bool CheckURIWithoutIncomingSession(LPCWSTR pszURI);

	//5. When the group chat finished, call function IUccConferenceSession::Leave. Here,

	//   we remove the record from groupSet.

	void RemoveGroupChat(HWND windHandle);

	//Check if a session is already in outgoing group set "groupSet" and there is no changes on

	//number of participant.

	bool IsInOutgoingGroupSet(WCHAR *confMeetingID, long numParticipants);

	//Check if a session is already in outgoing group set "groupSet".

	bool IsInOutgoingGroupSet(WCHAR *confMeetingID);



	//Handling incoming session

	//At Advise _IUccSessionParticipantEvents, we do IM evaluation on the incoming session since

	//no matter the session is in which type, it come with IM window. 

	//If the session already exists, that means a new participant is added. We will do evaluation

	//based on its real type from record.

	bool DoGroupEvalOnIncomingSession(CComPtr<IUccSession> pSession);

	//At Advise _IUccSessionParticipantEvents, after DoGroupIMEvalOnIncomingSession allows, we

	//set the current session as active group incoming session. Later, at My_NewSession_AddParticipant

	//we set active one to NULL. 

	void SetActiveGroupIncomingSesion(CComPtr<IUccSession> pSession);

	//At My_NewSession_AddParticipant, we know the real type of active incoming group session. If it is not IM,

	//we do evaluation again.

	bool DoGroupNonIMEvalOnActiveIncomingSession(UCC_SESSION_TYPE type, BSTR bstrConfURI, bool &bCallSDKEval);

	//Remove this session from incomingGroupSet if it exists in it

	void RemoveCachedIncommingGroupSession(CComPtr<IUccSession> pSession);

	//Find the incoming session based on conference meeting id, and set its real type.

	void FindIncomingSessionAndSetType(WCHAR *confMeetingID, UCC_SESSION_TYPE realType);

	//Find the incoming session based on conference meeting id, and Check its previous Eval Result.

	//Return false if the incoming session is not found.

	bool FindIncomingSessionAndCheckEvalResult(WCHAR *confMeetingID, bool &bAllow);

	// Added By Jacky.Dong 2011-11-23
	// ------------------------------
	// Do Invite Evaluation
	bool DoInviteEvaluation(BSTR participant, enum UCC_SESSION_TYPE SessionType);
	// ------------------------------


	//	comment by Ben, 2011-12-14
	//	I don't want to modify existing code which was not written by me, and it's written in very old years, its logic is complicated, 
	//	that's why I don't want to modify them, what I want to do is to add some code that will not affect existing code.
	//	in the existing code, there are some evaluations already,
	//	but I want to add some extra evaluation code using my own code, because existing code don't cover all cases,
	//	e.g. bug 15769, the existing code will not do AVDCALL evaluation, that's why I add my code here.
	//	here the general idea is, we store conference session, audio/video session of the same hwnd together in CLiveSessionWnd
	//	then we have two cases:
	//	case 1,
	//	when conference session is advised, we check CLiveSessionWnd to see if we have or not have an audio/video session for the same hwnd.
	//	if we have, then, we evaluate AVDCALL against the conference session, because conference session has all participants we need.
	//	case 2,
	//	on the other side, if an audio/video session is advised, 
	//	we check CLiveSessionWnd to see if we have or not have conference session for the same hwnd.
	//	if we have, then, we evaluate AVDCALL against the conference session, not the audio/video session, because conference session has all participants we need.
	bool DoEvalOnSession(CComPtr<IUccSession> pSession, CEAction_t action);


	//	share evaluation is different from other action
	//	we only evaluate once and must use local participant as sender
	//	return value:
	//	true means allow, otherwise means deny
	//	input value:
	//	pSession -- it's a conference session, we evaluate local participant against other participants in that session
	bool DoShareEvalOnSession(CComPtr<IUccSession> pSession);
};



//Initialization for policy evaluation

bool InitPolicyEval(WCHAR *processName);



//Evaluate if it is allowed to copy data to clipboard for the current endPointSip

bool PolicyEvalCopyAction();



//Add thread id and open file info mapping for later policy evaluation

void AddFileMappingForPolicyEval(DWORD tid, LPCWSTR fileName, HANDLE handle);



//Add a thread and its copy evaluation result pair to cachedCopyEvalResults

void CacheCopyEvalResult(HWND hWnd, bool bAllow);



//Fetch the cached copy eval result of a thread

//When this function reture false, it means that the result is unknown

//If the last eval time is older than 10 sec, discard the result and return false;

bool GetCachedCopyEvalResult(HWND hWnd, bool &bAllow);



//Evaluate if it is allowed to livemeeting 

bool PolicyEvalLivemeeting(wchar_t *confURI);

#endif

