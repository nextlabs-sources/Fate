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
	static CRITICAL_SECTION s_csPolicyInstance;
	static CRITICAL_SECTION s_csHandleNameCache;
	static CRITICAL_SECTION s_csDetachCritical;
};

