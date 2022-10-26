#include "stdafx.h"
#include "criticalMngr.h"

CRITICAL_SECTION CcriticalMngr::s_csCGzipper ;
CRITICAL_SECTION CcriticalMngr::s_csFileInfo ;
CRITICAL_SECTION CcriticalMngr::s_csSocketInfo ;
CRITICAL_SECTION CcriticalMngr::s_csEncryptDecryptMap;
CRITICAL_SECTION CcriticalMngr::s_csDownload_HTTPS;
CRITICAL_SECTION CcriticalMngr::s_csSendList;
CRITICAL_SECTION CcriticalMngr::s_csRecvList;
CRITICAL_SECTION CcriticalMngr::s_csEvalFilename;
CRITICAL_SECTION CcriticalMngr::s_csPolicyInstance;

CcriticalMngr::CcriticalMngr()	   
{
	::InitializeCriticalSection(&s_csSendList);
	::InitializeCriticalSection(&s_csRecvList);
	::InitializeCriticalSection(&s_csCGzipper);
	::InitializeCriticalSection(&s_csFileInfo);
	::InitializeCriticalSection(&s_csSocketInfo);
	::InitializeCriticalSection(&s_csEncryptDecryptMap);
	::InitializeCriticalSection(&s_csDownload_HTTPS);
	::InitializeCriticalSection(&s_csEvalFilename);
	::InitializeCriticalSection(&s_csPolicyInstance);
}

CcriticalMngr::~CcriticalMngr() 
{
	::DeleteCriticalSection(&s_csSendList);
	::DeleteCriticalSection(&s_csRecvList);
	::DeleteCriticalSection(&s_csCGzipper);
	::DeleteCriticalSection(&s_csFileInfo);
	::DeleteCriticalSection(&s_csSocketInfo) ;
	::DeleteCriticalSection(&s_csEncryptDecryptMap);
	::DeleteCriticalSection(&s_csDownload_HTTPS) ;
	::DeleteCriticalSection(&s_csEvalFilename) ;
	::DeleteCriticalSection(&s_csPolicyInstance);
}