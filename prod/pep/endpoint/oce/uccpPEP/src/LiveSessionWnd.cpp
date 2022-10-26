#include "LiveSessionWnd.h"
extern CELog g_log;



CLiveSessionWnd::CLiveSessionWnd(void)
{
		
	InitializeCriticalSection(&m_cs_hwnd_sessions);
}

CLiveSessionWnd::~CLiveSessionWnd(void)
{
	DeleteCriticalSection(&m_cs_hwnd_sessions);
}

CLiveSessionWnd* CLiveSessionWnd::GetInstance()
{
	static CLiveSessionWnd ins;
	return &ins;
}

void CLiveSessionWnd::AddSessionForWnd(HWND hwnd, CComPtr<IUccSession> s, UCC_SESSION_TYPE dwType)
{
	EnterCriticalSection(&m_cs_hwnd_sessions);


	DWORD i = 0;
	for (; i < m_hwnd_sessions.size(); i++)
	{
		if (m_hwnd_sessions[i].hwnd == hwnd)
		{
			switch(dwType)
			{
			case UCCST_AUDIO_VIDEO:
				if (m_hwnd_sessions[i].pAVSession != s)
				{
					g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we update the hwnd (%d) and session (0x%x), A/V\n", hwnd, s);
				}
				
				m_hwnd_sessions[i].pAVSession = s;
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return;
			case UCCST_CONFERENCE:
				if (m_hwnd_sessions[i].pConfSession != s)
				{
					g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we update the hwnd (%d) and session (0x%x), Conference\n", hwnd, s);
				}
				
				m_hwnd_sessions[i].pConfSession = s;
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return;
			default:
				//	don't support this type session,
				//	return directly
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return;
			}
		}
	}
	if (i == m_hwnd_sessions.size())
	{
		//	we have no hwnd, we need to add it
		HWND_SESSION hwndsession;
		hwndsession.hwnd = hwnd;
		hwndsession.pAVSession = NULL;
		hwndsession.pConfSession = NULL;
		switch(dwType)
		{
		case UCCST_AUDIO_VIDEO:
			g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we add the hwnd (%d) and session (0x%x), A/V\n", hwnd, s);
			hwndsession.pAVSession = s;
			break;
		case UCCST_CONFERENCE:
			g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we add the hwnd (%d) and session (0x%x), Conference\n", hwnd, s);
			hwndsession.pConfSession = s;
			break;
		default:
			//	don't support this type session,
			break;
		}
		m_hwnd_sessions.push_back(hwndsession);
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);
}

void CLiveSessionWnd::RemoveSessionForWnd(HWND hwnd, CComPtr<IUccSession> s)
{
	EnterCriticalSection(&m_cs_hwnd_sessions);

	vector<HWND_SESSION>::iterator it = m_hwnd_sessions.begin();

	for (; it != m_hwnd_sessions.end(); it++)
	{
		if ((*it).hwnd == hwnd)
		{
			if ((*it).pAVSession == s)
			{
				g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] hwnd[%d] session [%x]is stored, let's NULL AV session\n", hwnd, s);
				(*it).pAVSession = NULL;
			}
			if ((*it).pConfSession == s)
			{
				g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] hwnd[%d] session [%x]is stored, let's NULL Conf session\n", hwnd, s);
				(*it).pConfSession = NULL;
			}
			if (!(*it).pAVSession && !(*it).pConfSession)
			{
				g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] session in hwnd[%d] are all NULL, let's remove the hwnd\n", hwnd);
				m_hwnd_sessions.erase(it);
			}
			break;
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);
}

CComPtr<IUccSession> CLiveSessionWnd::GetSessionForWnd(HWND hwnd, UCC_SESSION_TYPE dwType)
{
	EnterCriticalSection(&m_cs_hwnd_sessions);

	
	for (DWORD i = 0; i < m_hwnd_sessions.size(); i++)
	{
		if (m_hwnd_sessions[i].hwnd == hwnd)
		{
			switch(dwType)
			{
			case UCCST_AUDIO_VIDEO:
				g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we get the hwnd (%d) and session (0x%x), A/V\n", hwnd, m_hwnd_sessions[i].pAVSession);
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return m_hwnd_sessions[i].pAVSession;
			case UCCST_CONFERENCE:
				g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we get the hwnd (%d) and session (0x%x), Conference\n", hwnd, m_hwnd_sessions[i].pConfSession);
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return m_hwnd_sessions[i].pConfSession;
			default:
				//	don't support this type session,
				//	return directly
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return NULL;
			}
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);
	return NULL;
}

CComPtr<IUccSession> CLiveSessionWnd::GetSessionForSession(CComPtr<IUccSession> in_s, UCC_SESSION_TYPE in_type, UCC_SESSION_TYPE out_Type)
{
	if (!in_s)
	{
		return NULL;
	}

	EnterCriticalSection(&m_cs_hwnd_sessions);

	g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] try to get session by input session (0x%x) input type (%d)\n", in_s, in_type);

	for (DWORD i = 0; i < m_hwnd_sessions.size(); i++)
	{
		if ( (in_s == m_hwnd_sessions[i].pAVSession) || (in_s == m_hwnd_sessions[i].pConfSession) )
		{
			g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] find the input session, [i = %d]\n", i);

			switch(in_type)
			{
			case UCCST_AUDIO_VIDEO:
				{
					if (in_s == m_hwnd_sessions[i].pAVSession)
					{
						//	matched, we support only out_Type of UCCST_CONFERENCE for now
						if (UCCST_CONFERENCE == out_Type)
						{
							g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we get conference session (0x%x) out\n", m_hwnd_sessions[i].pConfSession);
							LeaveCriticalSection(&m_cs_hwnd_sessions);
							return m_hwnd_sessions[i].pConfSession;
						}
						else
						{
							//	don't support this type session,
							//	return directly
							LeaveCriticalSection(&m_cs_hwnd_sessions);
							return NULL;
						}
					}
					else
					{
						//	don't match, return null
						LeaveCriticalSection(&m_cs_hwnd_sessions);
						return NULL;
					}
				}
				break;
			case UCCST_CONFERENCE:
				{
					if (in_s == m_hwnd_sessions[i].pConfSession)
					{
						//	matched, we support only out_Type of UCCST_AUDIO_VIDEO for now
						if (UCCST_AUDIO_VIDEO == out_Type)
						{
							g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] we get A/V session (0x%x) out\n", m_hwnd_sessions[i].pAVSession);
							LeaveCriticalSection(&m_cs_hwnd_sessions);
							return m_hwnd_sessions[i].pAVSession;
						}
						else
						{
							//	don't support this type session,
							//	return directly
							LeaveCriticalSection(&m_cs_hwnd_sessions);
							return NULL;
						}
					}
					else
					{
						//	don't match, return null
						LeaveCriticalSection(&m_cs_hwnd_sessions);
						return NULL;
					}
				}
				break;
			default:
				//	don't support this type session,
				//	return directly
				LeaveCriticalSection(&m_cs_hwnd_sessions);
				return NULL;
			}
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);
	return NULL;
}

void CLiveSessionWnd::AddConfID(const wstring& wstrConfID, IUccConferenceSession* pConferenceSession)
{
	if (!pConferenceSession)
	{
		return;
	}

	EnterCriticalSection(&m_cs_hwnd_sessions);
	
	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].pConferenceSession == pConferenceSession)
		{
			// if the ConferenceSession exist, modify its confID to the value of wstrConfID
			m_vect_confID_session[i].confID = wstrConfID;
			
			g_log.Log(CELOG_DEBUG, L"[CLiveSessionWnd] AddConfID id (%s), ConfSession (%p)\n", wstrConfID.c_str(), pConferenceSession);
			break;
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);
	
	return;
}

void CLiveSessionWnd::AddConfSessionPair(CComPtr<IUccSession> pConfSession, IUccConferenceSession* pConferenceSession)
{
	if (!pConfSession || !pConferenceSession)
	{
		return;
	}

	EnterCriticalSection(&m_cs_hwnd_sessions);

	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].pConfSession == pConfSession)
		{
			//	it contains an conf session pointer that equal with the new created conf session point
			//	we initialize again
			m_vect_confID_session[i].bContainShareSession = FALSE;
			m_vect_confID_session[i].confID.clear();
			m_vect_confID_session[i].pConfSession = pConfSession;
			m_vect_confID_session[i].pConferenceSession = pConferenceSession;

			//	ok, we are done
			LeaveCriticalSection(&m_cs_hwnd_sessions);
			g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] AddConfSessionPair (update) session (%p), ConfSession (%p) out\n", pConfSession, pConferenceSession);
			return;
		}
	}
	
	//	the pUccSession is not found in our vector, need to add a new one into it
	CONF_ID_SESSION confIdSession;
	confIdSession.bContainShareSession = FALSE;
	confIdSession.pConfSession = pConfSession;
	confIdSession.pConferenceSession = pConferenceSession;

	m_vect_confID_session.push_back(confIdSession);

	LeaveCriticalSection(&m_cs_hwnd_sessions);
	
	g_log.Log(CELOG_DEBUG, "[CLiveSessionWnd] AddConfSessionPair session (%p), ConfSession (%p)\n", pConfSession, pConferenceSession);

	return;
}

CComPtr<IUccSession> CLiveSessionWnd::GetSessionByConfID(const wstring& confID)
{
	EnterCriticalSection(&m_cs_hwnd_sessions);

	CComPtr<IUccSession> pSession = NULL;
	
	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].confID == confID)
		{
			pSession = m_vect_confID_session[i].pConfSession;
			break;
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);

	g_log.Log(CELOG_DEBUG, L"[CLiveSessionWnd] GetSessionByConfID confID (%s), get Session (%p) out\n", confID.c_str(), pSession);

	return pSession;
}

BOOL CLiveSessionWnd::SetShareSessionFlag(const wstring& wstrConfID)
{
	BOOL bFlag = FALSE;
	
	EnterCriticalSection(&m_cs_hwnd_sessions);
	
	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].confID == wstrConfID)
		{
			m_vect_confID_session[i].bContainShareSession = TRUE;
			
			bFlag = TRUE;
			break;
		}
	}
	
	LeaveCriticalSection(&m_cs_hwnd_sessions);

	g_log.Log(CELOG_DEBUG, L"[CLiveSessionWnd] SetShareSessionFlag confID (%s) %s\n", wstrConfID.c_str(), bFlag == TRUE ? L"Success!" : L"Failed!");
	
	return bFlag;
}

BOOL CLiveSessionWnd::RemoveSessionItem(IUccConferenceSession* pConfSession)
{
	BOOL bFlag = FALSE;
	
	EnterCriticalSection(&m_cs_hwnd_sessions);
	
	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].pConferenceSession == pConfSession)
		{
			m_vect_confID_session[i].bContainShareSession = FALSE;
			m_vect_confID_session[i].confID.clear();
			if(NULL != m_vect_confID_session[i].pConfSession)
			{
				m_vect_confID_session[i].pConfSession = NULL;
			}
			if(NULL != m_vect_confID_session[i].pConferenceSession)
			{
				m_vect_confID_session[i].pConferenceSession = NULL;
			}
			m_vect_confID_session.erase(m_vect_confID_session.begin() + i);
			
			bFlag = TRUE;
			break;
		}
	}
	
	LeaveCriticalSection(&m_cs_hwnd_sessions);

	g_log.Log(CELOG_DEBUG, L"[CLiveSessionWnd] RemoveSessionItem ConfSession(%p) %s\n", pConfSession, bFlag == TRUE ? L"Success!" : L"Failed!");
	
	return bFlag;
}

BOOL CLiveSessionWnd::GetShareFlagByConfSessionPointer(CComPtr<IUccSession> pConfSession)
{
	EnterCriticalSection(&m_cs_hwnd_sessions);

	BOOL bShareFlag = FALSE;

	for (DWORD i = 0; i < m_vect_confID_session.size(); i++)
	{
		if (m_vect_confID_session[i].pConfSession == pConfSession)
		{
			bShareFlag = m_vect_confID_session[i].bContainShareSession;
			break;
		}
	}

	LeaveCriticalSection(&m_cs_hwnd_sessions);

	g_log.Log(CELOG_DEBUG, L"[CLiveSessionWnd] GetShareFlagByConfID IUccSession pointer (%p), bShareFlag (%d) out\n", pConfSession, bShareFlag);

	return bShareFlag;
}
