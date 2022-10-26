#include "StdAfx.h"
#include "ThreadHelper.h"

CThreadHelper::CThreadHelper(ThreadProcType pThreadProc, PVOID pParam)
{
	m_pThreadProc = pThreadProc;
	m_pParam = pParam;

	m_handle = NULL;
	m_tid = 0;
	m_bRunning = FALSE;

	m_event = NULL;
}

CThreadHelper::~CThreadHelper(void)
{
}

BOOL CThreadHelper::Create()
{
	MUTEX mutex(&m_Mutex);


	if (!m_pThreadProc)
	{
		return FALSE;
	}

	m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_event)
	{
		return FALSE;
	}

	m_handle = CreateThread (
		0, // Security attributes
		0, // Stack size
		my_ThreadProc,
		this,
		CREATE_SUSPENDED,
		&m_tid);

	if (NULL == m_handle)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CThreadHelper::Resume()
{
	MUTEX mutex(&m_Mutex);


	ResumeThread (m_handle);

	//	make sure the thread is running before resume exit.
	WaitForSingleObject(m_event, INFINITE);

	return TRUE;
}

BOOL CThreadHelper::Kill()
{
	MUTEX mutex(&m_Mutex);

	if (!m_bRunning)
	{
		//	thread is not running, can't be killed
		return FALSE;
	}

	//	this will seriously impact user, we don't call terminate
	//	we will fix it in next phase
	//TerminateThread(m_handle, 0);

	m_bRunning = FALSE;

	return TRUE;
}

BOOL CThreadHelper::IsRunning()
{
	MUTEX mutex(&m_Mutex);

	return m_bRunning;
}

DWORD CThreadHelper::my_ThreadProc(LPVOID lpParameter)
{
	if (!lpParameter)
	{
		return 0;
	}

	//	thread is running
	CThreadHelper* pthis = (CThreadHelper*)lpParameter;


	pthis->m_bRunning = TRUE;

	SetEvent(pthis->m_event);

	

	DWORD res = pthis->m_pThreadProc(pthis->m_pParam);

	
	pthis->m_bRunning = FALSE;

	return res;
}
