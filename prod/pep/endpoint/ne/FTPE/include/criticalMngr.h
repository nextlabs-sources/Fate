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
	static CRITICAL_SECTION s_csFtpConn;
	static CRITICAL_SECTION s_csMPMgr ;
	static CRITICAL_SECTION s_csPathContentCache;
	static CRITICAL_SECTION s_csHandleNameCache;
	static CRITICAL_SECTION s_csSocketBufEvalCache;
	static CRITICAL_SECTION s_csThreadContent;
	static CRITICAL_SECTION s_csThreadSocket;
	static CRITICAL_SECTION s_csConhandleEval;
	static CRITICAL_SECTION s_csDelHandleNameCache;
	static CRITICAL_SECTION s_csDenyWriteFileHandle;
	static CRITICAL_SECTION s_csDetached;//this is used to control the flag of "unhookAPI".
	static CRITICAL_SECTION s_csEvalCache;
	static CRITICAL_SECTION s_csDelete;
	static CRITICAL_SECTION s_csVerdictCache;
	static CRITICAL_SECTION s_csPolicyInstance;
};