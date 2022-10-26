#pragma once



/*

user use CEDPNotifyMgr to notify nl_notification.dll(or custom notification dll if notify.ini specify custom notification dll).

which notification dll(nl_notification.dll or custom notification dll) is using is transprant to user.

*/

class CEDPNotifyMgr
{
public:
	static CEDPNotifyMgr& GetInstance();

	/*
	
	notify user that a notification.

	*/
	bool ShowNotify(PVOID notifyInfo, int nIDNotification);

private:
	CEDPNotifyMgr(void);
	~CEDPNotifyMgr(void);

	bool ReadConfig(char* pNotifyType, int nTypeLen, char* pNotifyPath, int nPathLen);
	void ParseNotifyConfig(const int nType, const char* pNotifyPath, IN OUT char* pFullPath, int nLen);
};

