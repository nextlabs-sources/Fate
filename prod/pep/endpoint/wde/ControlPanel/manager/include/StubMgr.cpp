 #include "StdAfx.h"
#include "StubMgr.h"
#include "EDPMgrReqHandler.h"
#include "iipcrequesthandler.h"
#include "globals.h"

#pragma warning(push)
#pragma warning(disable:4251)
#include "IPCstub.h"
#pragma warning(pop)


#include "ipcproxy.h"


CStubMgr::CStubMgr(void)
{
	m_pStub = NULL;
	m_pRequestHandler = NULL;
	m_hStubThread = NULL;
}

CStubMgr::~CStubMgr(void)
{
}

CStubMgr& CStubMgr::GetInstance()
{
	static CStubMgr stubMgr;

	return stubMgr;
}

BOOL CStubMgr::StartStub()
{
	if (m_pStub)
	{
		//	already started
		return FALSE;
	}

	m_pStub = new IPCStub ();

	IIPCRequestHandler* ppHandlerArray [1];

	ppHandlerArray [0] = m_pRequestHandler = new CEDPMgrReqHandler;

	m_pStub->Init (ppHandlerArray, 1);

	m_hStubThread = ::CreateThread (NULL, 0, my_ThreadProc, (LPVOID) m_pStub, 0, NULL); 

	if (!m_hStubThread)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CStubMgr::StopStub()
{
	m_pStub->Stop();
	::WaitForSingleObject (m_hStubThread, 2000);
	return TRUE;
}

DWORD WINAPI CStubMgr::my_ThreadProc(LPVOID lpStub)
{
	((IPCStub*) lpStub)->Run();

	::ExitThread (0);
	return (0);
}
