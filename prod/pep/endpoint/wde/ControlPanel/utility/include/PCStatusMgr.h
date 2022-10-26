#pragma once

#include <string>

enum _CEResult_t;

class CPCStatusMgr
{
public:
	CPCStatusMgr(void)
	{
	}

	virtual ~CPCStatusMgr(void)
	{
	}


	virtual _CEResult_t StopPC(wchar_t* pszPwd) = 0;

	virtual BOOL StartPC() = 0;

	virtual BOOL IsPCRunning() = 0;

	virtual void ResetUAC() = 0;
};
