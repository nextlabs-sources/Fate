#include "stdafx.h"
#include "criticalMngr.h"

CRITICAL_SECTION CcriticalMngr::s_csPolicyInstance;
CRITICAL_SECTION CcriticalMngr::s_csHandleNameCache;
CRITICAL_SECTION CcriticalMngr::s_csDetachCritical;



CcriticalMngr::CcriticalMngr()	   
{
	::InitializeCriticalSection(&s_csPolicyInstance);
	::InitializeCriticalSection(&s_csHandleNameCache);
	::InitializeCriticalSection(&s_csDetachCritical);
}

CcriticalMngr::~CcriticalMngr() 
{
	::DeleteCriticalSection(&s_csPolicyInstance);
	::DeleteCriticalSection(&s_csHandleNameCache);
	::DeleteCriticalSection(&s_csDetachCritical);
}

