#include "stdafx.h"
#include "criticalMngr.h"

CRITICAL_SECTION CcriticalMngr::s_csConhandleEval ;
CRITICAL_SECTION CcriticalMngr::s_csDelHandleNameCache ;
CRITICAL_SECTION CcriticalMngr::s_csFtpConn ;
CRITICAL_SECTION CcriticalMngr::s_csHandleNameCache ;
CRITICAL_SECTION CcriticalMngr::s_csMPMgr ;
CRITICAL_SECTION CcriticalMngr::s_csPathContentCache ;
CRITICAL_SECTION CcriticalMngr::s_csThreadContent ;
CRITICAL_SECTION CcriticalMngr::s_csThreadSocket ;
CRITICAL_SECTION CcriticalMngr::s_csSocketBufEvalCache ;
CRITICAL_SECTION CcriticalMngr::s_csDenyWriteFileHandle ;
CRITICAL_SECTION CcriticalMngr::s_csDetached;
CRITICAL_SECTION CcriticalMngr::s_csEvalCache;
CRITICAL_SECTION CcriticalMngr::s_csDelete;
CRITICAL_SECTION CcriticalMngr::s_csVerdictCache;
CRITICAL_SECTION CcriticalMngr::s_csPolicyInstance;

CcriticalMngr::CcriticalMngr()	   
{
	::InitializeCriticalSection(&s_csFtpConn);
	::InitializeCriticalSection(&s_csMPMgr);
	::InitializeCriticalSection(&s_csPathContentCache);
	::InitializeCriticalSection(&s_csHandleNameCache);
	::InitializeCriticalSection(&s_csSocketBufEvalCache);
	::InitializeCriticalSection(&s_csThreadContent);
	::InitializeCriticalSection(&s_csThreadSocket);
	::InitializeCriticalSection(&s_csConhandleEval);
	::InitializeCriticalSection(&s_csDelHandleNameCache);
	::InitializeCriticalSection(&s_csDenyWriteFileHandle);
	::InitializeCriticalSection(&s_csDetached);
	::InitializeCriticalSection(&s_csEvalCache);
	::InitializeCriticalSection(&s_csDelete);
	::InitializeCriticalSection(&s_csVerdictCache);
	::InitializeCriticalSection(&s_csPolicyInstance);
}

CcriticalMngr::~CcriticalMngr() 
{
	::DeleteCriticalSection(&s_csFtpConn);
	::DeleteCriticalSection(&s_csMPMgr);
	::DeleteCriticalSection(&s_csPathContentCache);
	::DeleteCriticalSection(&s_csHandleNameCache);
	::DeleteCriticalSection(&s_csSocketBufEvalCache);
	::DeleteCriticalSection(&s_csThreadContent);
	::DeleteCriticalSection(&s_csThreadSocket);
	::DeleteCriticalSection(&s_csConhandleEval);
	::DeleteCriticalSection(&s_csDelHandleNameCache);
	::DeleteCriticalSection(&s_csDenyWriteFileHandle);
	::DeleteCriticalSection(&s_csDetached);
	::DeleteCriticalSection(&s_csEvalCache);
	::DeleteCriticalSection(&s_csDelete);
	::DeleteCriticalSection(&s_csVerdictCache);
	::DeleteCriticalSection(&s_csPolicyInstance);
}