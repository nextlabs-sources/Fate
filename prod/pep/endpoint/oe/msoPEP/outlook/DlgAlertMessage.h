#pragma once
#include <windows.h>
#include <string>
#include "HyperLinkStatic.h"
#include "resource.h"
#include "SCrollStatic.h"

class AlertMsg;
class AlertMsgs;

class CDlgAlertMessage
{
public:
	CDlgAlertMessage(DWORD dwDlgID, bool bAllow, const AlertMsgs& alertMsg);
	~CDlgAlertMessage();
	int DoModal(HWND hParent = NULL);

protected:
	static INT_PTR WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	INT_PTR MessageHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnOK();
	void OnInitDialog();
	BOOL OnOwnerDraw(DRAWITEMSTRUCT* lpDrawItemStruct);
	void SetHWnd(HWND hWnd) { m_hWnd = hWnd; }

	// load some icons including the dialog icon and associate deny and allow icons with the list-view
	void PrepareImages();
	void AddListViewItems(HWND hWndListView, const AlertMsgs &msgs);

private:
	bool m_bAllow;
	//std::wstring m_strHeader;
	//std::wstring m_strMessage;

	CHyperLinkStatic m_staticHeader;
	//CSCrollStatic m_ListViewMessage;
	
	DWORD m_dwDlgID;
	HWND m_hWnd;
	//HICON m_hTipIcon;

	HIMAGELIST hSmallImageList;   // Image list for ListViewCtrl

	CHyperLinkStatic m_staticTip;

	const AlertMsgs *m_alertMessages;
};

