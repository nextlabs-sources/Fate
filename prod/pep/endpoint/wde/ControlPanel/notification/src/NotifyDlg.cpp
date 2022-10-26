#include "StdAfx.h"
#include "NotifyDlg.h"
#include "EDPMgrUtilities.h"
#include "Actions.h"

extern HINSTANCE g_hInst;

CNotifyDlg::CNotifyDlg(void)
{
}

CNotifyDlg::~CNotifyDlg(void)
{
}

LRESULT CNotifyDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CNotifyDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;


	m_listNotifications.SubclassWindow(GetDlgItem(IDC_LIST_NOTIFICATION));
	m_listNotifications.ForceMeasureItemMessage();

	//	init list view control
	m_listNotifications.Initialize(500);

	HICON hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, FALSE);
	}

	hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, TRUE);
	}

	CenterWindow();

	return 1;
}

LRESULT CNotifyDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_listNotifications.DeleteAllItems();
	m_listNotifications.UnsubclassWindow();

	return 1;
}

void CNotifyDlg::FillData()
{
	m_listNotifications.DeleteAllItems();


	//Set icons
	HICON hIconExpand = LoadIconW(g_hInst, MAKEINTRESOURCE(IDI_EXPAND));
	HICON hIconShrunk = LoadIconW(g_hInst, MAKEINTRESOURCE(IDI_SHRUNK));

	m_listNotifications.SetExpandedIcon(hIconExpand, hIconExpand);
	m_listNotifications.SetShrunkIcon(hIconShrunk, hIconShrunk);

	m_listNotifications.SetBkColor(RGB(255, 255, 255));

	smart_ptr<NL_NOTIFICATIONS_ITEM> spItem(new NL_NOTIFICATIONS_ITEM);


	spItem->strTime = m_info.time;		//	time string

	SUBITEM subItem;																		
	subItem.strKey = L"Notification";		//	notification string
	subItem.strValue = m_info.message;

	SUBITEM subItem1;
	subItem1.strKey = L"Enforcement";	//	deny or allow
	subItem1.strValue = (m_info.enforcement == DENY) ? L"Deny" : L"Allow";

	spItem->vSubItems.push_back(subItem);
	spItem->vSubItems.push_back(subItem1);


	spItem->detail.strName = L"Detail";

	SUBITEM detail0;
	detail0.strKey = L"Action";	//	action
	detail0.strValue = m_info.action;

	SUBITEM detail1;
	detail1.strKey = L"File";	//	source title
	detail1.strValue = m_info.file;



	spItem->detail.vValues.push_back(detail0);
	spItem->detail.vValues.push_back(detail1);


	m_listNotifications.InsertItemEx((LPARAM)&spItem, 0);
	
}

LRESULT CNotifyDlg::OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
	return 0;
}

LRESULT CNotifyDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
//	DestroyWindow();
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CNotifyDlg::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	return 0;
}

void CNotifyDlg::ShowDetails(int nIndex)
{
	m_listNotifications.ShowDetails(nIndex);
}
