#pragma once


/************************************************************************

only for status management.

set and get current status.

seems thread lock is needed.

/************************************************************************/

class CLogMgr
{
public:
	CLogMgr(void)
	{

	}

	virtual ~CLogMgr(void)
	{

	}

	/*

	switch on/off verbose log in EDP Manager.

	notice that, you can only call this function when PC is not running.

	*/
	virtual BOOL SetVerlogStatus(BOOL bEnable) = 0;


	/*
	
	get verbose log status of EDP Manager
	
	*/
	virtual BOOL GetVerlogStatus(BOOL& bEnable) = 0;
};
