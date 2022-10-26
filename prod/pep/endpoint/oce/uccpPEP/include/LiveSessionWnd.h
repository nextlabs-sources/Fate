#pragma once
#include "stdafx.h"
#include <vector>
#include <string>
#include "celog.h"
using namespace std;


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
class CLiveSessionWnd
{
private:
	CLiveSessionWnd(void);
	~CLiveSessionWnd(void);
public:
	static CLiveSessionWnd* GetInstance();
public:

	//	important comment!!!!!!!!!!!!!!!!!!!
	//	below four functions and structure HWND_SESSION will not be expanded in future
	//	because using hwnd to combine sessions is not reliable
	void AddSessionForWnd(HWND hwnd, CComPtr<IUccSession> s, UCC_SESSION_TYPE dwType);
	CComPtr<IUccSession> GetSessionForWnd(HWND hwnd, UCC_SESSION_TYPE dwType);
	/*
	return null if can't find a session of out_Type, by in_s of in_type.
	we need this function because sometimes we can't get hwnd, because we can only get the correct hwnd when we are sure the chat windows is current foreground active window.
	*/
	CComPtr<IUccSession> GetSessionForSession(CComPtr<IUccSession> in_s, UCC_SESSION_TYPE in_type, UCC_SESSION_TYPE out_Type);
	void RemoveSessionForWnd(HWND hwnd, CComPtr<IUccSession> s);



	

	//	because using hwnd to combine sessions are not a good idea.
	//	using conf ID is a reliable idea, so please using below functions which use conf ID to expand OCE feature.

	////////////////////////////////////////////////////////////
	//	add IUccSession and conf ID as pair
	void AddConfID(const wstring& wstrConfID, IUccConferenceSession* pConferenceSession);
	
	////////////////////////////////////////////////////////////
	//	create a new item of CONF_ID_SESSION and add IUccSession and IUccConferenceSession as pair
	void AddConfSessionPair(CComPtr<IUccSession> pConfSession, IUccConferenceSession* pConferenceSession);
	
	////////////////////////////////////////////////////////////
	//	get IUccSession by conf ID
	CComPtr<IUccSession> GetSessionByConfID(const wstring& confID);

	////////////////////////////////////////////////////////////
	//	this function is used to indicate that the conference session is combined with a share type session, as the share type session 
	//	uses the conf ID that belong to the conference session.
	BOOL SetShareSessionFlag(const wstring& wstrConfID);
	
	////////////////////////////////////////////////////////////
	//	this function is used to remove the item of a CONF_ID_SESSION which is identified by conf ID
	BOOL RemoveSessionItem(IUccConferenceSession* pConfSession);

	////////////////////////////////////////////////////////////
	//	get bContainShareSession by IUccSession pointer
	BOOL GetShareFlagByConfSessionPointer(CComPtr<IUccSession> pConfSession);

private:

	CRITICAL_SECTION m_cs_hwnd_sessions;

	typedef struct
	{
		HWND hwnd;
		CComPtr<IUccSession> pConfSession;
		CComPtr<IUccSession> pAVSession;
	}HWND_SESSION;
	vector<HWND_SESSION> m_hwnd_sessions;


	//////////////////////////////////////////////////////////////////////////
	//	bind IUccSession, IUccConferenceSession and conf ID
	typedef struct
	{
		wstring confID;							//	conf ID of conf session
		CComPtr<IUccSession> pConfSession;				//	the IUccSession of conf session
		IUccConferenceSession* pConferenceSession;	//	the IUccConferenceSession pointer of conf session
		BOOL bContainShareSession;	//	whether or not the conference session is combined with a share session
	}CONF_ID_SESSION;

	vector<CONF_ID_SESSION> m_vect_confID_session;
};
