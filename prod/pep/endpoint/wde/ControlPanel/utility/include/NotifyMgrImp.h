#pragma once
#include "NotifySettingMgr.h"
#include "NotificationInfo.h"
#include "MutexHelper.h"

class CNotifyMgrImp :
	public CNotifyMgr
{
public:
	CNotifyMgrImp(void);
	virtual ~CNotifyMgrImp(void);

	/*

	set and get notification display level

	there are three levels	--	display all, display blocking only, display none.

	*/
	virtual BOOL SetNotifyDisplayLever(NOTIFY_DIS_LEVEL eLevel);
	virtual BOOL GetNotifyDisplayLever(NOTIFY_DIS_LEVEL& eLevel);

	/*

	set and get notification duration

	there are four levels	--	user close, 15 seconds, 30 seconds and 45 seconds.

	*/
	virtual BOOL SetNotifyDisplayDuration(NOTIFY_DIS_DURATION eDuration);
	virtual BOOL GetNotifyDisplayDuration(NOTIFY_DIS_DURATION& eDuration);


	/*

	add and get notify history.

	history max number is controlled by \\Software\\Nextlabs\\EDP Manager\\MaxNotifyCache in registry, 
	default value is 300 is no registry.

	all notification is added into notifymgr, notifymgr manage notification history.

	*/
	virtual BOOL AddNotify(NOTIFY_INFO& notify);
	virtual BOOL GetNotifyHistory(NotifyInfo* pNotifies, int nCount);
	virtual int GetNotifyCount();


private:

	/*
	
	not used

	*/
	BOOL ParseNotify(NOTIFY_INFO& notify);

	static BOOL FormatTimeStr(wstring& strTime, OUT wstring& strFormated);

	void NotifyMgrInit();
	BOOL m_bNotifyMgrInited;
	DWORD m_dwNotifyHistoryMax;
	Mutex m_Mutex;
	

	NotificationVector m_NotificationArray;

	typedef struct
	{
		wstring str24Hour;
		wstring str12tHour;
		wstring strAMPM;
	}HourSwitch;

	static HourSwitch s_arrayHourSwitch[24];

	CRITICAL_SECTION m_csList;
};
