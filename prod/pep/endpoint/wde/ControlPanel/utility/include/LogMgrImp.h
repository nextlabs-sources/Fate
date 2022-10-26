#pragma once

#include "LogMgr.h"

/************************************************************************

only for status management.

set and get current status.

seems thread lock is needed.

/************************************************************************/

class CLogMgrImp: public CLogMgr
{
public:
	CLogMgrImp(void);
	virtual ~CLogMgrImp(void);


	/*
	
	switch on/off verbose log in EDP Manager.

	notice that, you can only call this function when PC is not running.
	
	*/
	virtual BOOL SetVerlogStatus(BOOL bEnable);

	virtual BOOL GetVerlogStatus(BOOL& bEnable);

private:
	BOOL EnableAgentLog(BOOL bOn);
	BOOL EnableDebugModeReg(BOOL bOn);
	BOOL SetVerboseLogRegistry(BOOL bOn);


private:

	BOOL m_bInited;

	BOOL m_bVerlogEnabled;

	const static LONG BUFFER_SIZE = 2048  ;
};
