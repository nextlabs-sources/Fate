#pragma once

//#include "EnforcerStatusWindow.h"

class CSystrayMgr
{
public:
	static CSystrayMgr& GetInstance();

	/*

	add a icon to system tray bar.

	*/
	BOOL AddSystray(const DWORD dwIconID, const HWND hIconWnd, const UINT dwMsgID, HICON hIcon, WCHAR* pszTip);

	BOOL ModifySystray(LPCWSTR pszEnforcerStatus);

	BOOL ShowInfo();

//	void ShowEnforcerStatus(int x, int y);
protected:
	CSystrayMgr();
	~CSystrayMgr();

private:
	NOTIFYICONDATA m_nid;
//	CEnforcerStatusWindow* m_pEnforcerStatusWindow;
};