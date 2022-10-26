#pragma once
/*
Manager the critical section
*/


class CcriticalMngr
{
public:
	CcriticalMngr() ;
	~CcriticalMngr() ;
public:
	static CRITICAL_SECTION s_csSendList;
	static CRITICAL_SECTION s_csRecvList;
	static CRITICAL_SECTION s_csCGzipper;
	static CRITICAL_SECTION s_csFileInfo ;
	static CRITICAL_SECTION s_csSocketInfo ;
	static CRITICAL_SECTION s_csEncryptDecryptMap;
	static CRITICAL_SECTION s_csDownload_HTTPS;
	static CRITICAL_SECTION s_csEvalFilename;
	static CRITICAL_SECTION s_csPolicyInstance;
};