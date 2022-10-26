#pragma once

#include <string>
#include <vector>
using namespace std;

struct NotifyInfo
{
	wchar_t time[100];
	wchar_t action[50];
	wchar_t file[MAX_PATH];
	wchar_t message[512];
	int     enforcement;        //	store DENY/ALLOW
};

class CNotifyMgr
{
public:

	CNotifyMgr(void)
	{
	}

	virtual ~CNotifyMgr(void)
	{
	}


	/*
	
	set and get notification display level

	there are three levels	--	display all, display blocking only, display none.
	
	*/
	typedef enum
	{
		E_ALL,
		E_BLOCK_ONLY,
		E_NONE
	}NOTIFY_DIS_LEVEL;
	virtual BOOL SetNotifyDisplayLever(NOTIFY_DIS_LEVEL eLevel) = 0;
	virtual BOOL GetNotifyDisplayLever(NOTIFY_DIS_LEVEL& eLevel) = 0;

	/*

	set and get notification duration

	there are four levels	--	user close, 15 seconds, 30 seconds and 45 seconds.

	*/
	typedef enum
	{
		E_USER_CLOSE,
		E_15_SECONDS,
		E_30_SECONDS,
		E_45_SECONDS
	}NOTIFY_DIS_DURATION;
	virtual BOOL SetNotifyDisplayDuration(NOTIFY_DIS_DURATION eDuration) = 0;
	virtual BOOL GetNotifyDisplayDuration(NOTIFY_DIS_DURATION& eDuration) = 0;

	/*

	add notify to history.

	history max number is controlled by \\Software\\Nextlabs\\EDP Manager\\MaxNotifyCache in registry, 
	default value is 300 is no registry.

	all notification is added into notifymgr, notifymgr manage notification history.

	*/
	typedef struct  
	{
		ULONG ulSize;
		WCHAR methodName [64];
		WCHAR params [7][256];
	}NOTIFY_INFO;
	virtual BOOL AddNotify(NOTIFY_INFO& notify) = 0;;

	/*
	
	get notify history.
	
	*/


	struct NotificationInfo
	{
		wstring time;
		wstring action;
		wstring file;
		wstring message;
		int     enforcement;        //	store DENY/ALLOW
	};
	typedef std::vector< NotificationInfo* > NotificationVector;
	virtual int GetNotifyCount() = 0;
	virtual BOOL GetNotifyHistory(NotifyInfo* pNotifies, int nCount) = 0;
};
