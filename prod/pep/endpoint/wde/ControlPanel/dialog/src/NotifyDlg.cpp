#include "StdAfx.h"
#include "NotifyDlg.h"



extern HINSTANCE g_hInst;

#ifdef _X86_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#endif


BOOL GetOSInfo(DWORD& dwMajor, DWORD& dwMinor)
{
	static DWORD sMajor = 0;
	static DWORD sMinor = 0;

	if(sMajor == 0 && sMinor == 0)
	{
		OSVERSIONINFOEX osvi;
		BOOL bOsVersionInfoEx;

		// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
		//
		// If that fails, try using the OSVERSIONINFO structure.

		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
		if( !bOsVersionInfoEx )
		{
			// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
				return FALSE;
		}

		sMajor = osvi.dwMajorVersion;
		sMinor = osvi.dwMinorVersion;

	}


	//5,0 win2k, 5,1 winxp
	dwMajor = sMajor;
	dwMinor = sMinor;

	return TRUE;

}

BOOL IsWin7()
{
	DWORD dwMajor, dwMinor;
	return ( GetOSInfo(dwMajor, dwMinor) && dwMajor >= 6 )? TRUE: FALSE;
}

CNotifyDlg::CNotifyDlg(void)
{
	m_hFontBig = CreateFontW(g_nFontSizeBig,
							0,
							0,
							0,
							FW_NORMAL,
							0,
							0,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);

	m_hFont = CreateFontW(g_nFontSize,
							0,
							0,
							0,
							FW_NORMAL,
							0,
							0,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);
	m_hThread = 0;
}

CNotifyDlg::~CNotifyDlg(void)
{
	if(m_hFontBig)
	{
		DeleteObject(m_hFontBig);
		m_hFontBig = NULL;
	}

	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}

LRESULT CNotifyDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CNotifyDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	m_listNotifications.SubclassWindow(GetDlgItem(IDC_LIST_NOTIFICATIONS));
	m_listNotifications.ForceMeasureItemMessage();
	m_listNotifications.SetBkColor(g_clrHighlight);
	m_listNotifications.SetFont(m_hFont);
	m_listNotifications.SetHeaderFont(m_hFont);
	m_listNotifications.EnableHeader(FALSE);
	m_listNotifications.SetExtendedStyle(m_listNotifications.GetExtendedStyle() | LVS_EX_INFOTIP | LVS_EX_LABELTIP );

	//the main dialog of EDPManager is TOPMOST, so we need to set the ToolTip as TopMost too, otherwise the tooltip will be covered by
	//dialog, and user can't see the tooltip.
	HWND hWnd = ListView_GetToolTips(m_listNotifications.m_hWnd);
	if(hWnd)
	{
		::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;    
	lvColumn.cchTextMax = 0;
	lvColumn.pszText = L"Date/Time";
	lvColumn.cx = 130;
	m_listNotifications.InsertColumn(0, &lvColumn);

	lvColumn.pszText = L"Description";
	lvColumn.cx = 320;
	m_listNotifications.InsertColumn(1, &lvColumn);

	

	SetDlgItemTextW(IDC_DESCRIPTION, L"");
	SetDlgItemTextW(IDC_ENFORCEMENT, L"");
	SetDlgItemTextW(IDC_ACTION, L"");

	return 1;
}

LRESULT CNotifyDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_NotifyArray.clear();
	m_listNotifications.DeleteAllItems();
	m_listNotifications.UnsubclassWindow();

	return 1;
}

void CNotifyDlg::FillData()
{

	//	get notification history, then insert it one by one
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	m_NotifyArray.clear();
	edpUtilities.GetNotifyHistory(m_NotifyArray);

	//for test
//		FillData_test();

	m_listNotifications.DeleteAllItems();

	int nIndex = 0;
	for (int i = (int)m_NotifyArray.size() - 1; i >= 0 ; i--)
	{
		LVITEM item;
		memset (&item, 0, sizeof (item));
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = nIndex;
		item.lParam = i;

		item.cchTextMax = sizeof(m_NotifyArray[i].time) / sizeof(wchar_t);
		item.pszText = (LPWSTR)m_NotifyArray[i].time;    
		
		m_listNotifications.InsertItem(&item);
		m_listNotifications.SetItemText(nIndex, 1, m_NotifyArray[i].message);

		
		nIndex++;
	}

	//Set the 1st item as the default.
	if(m_listNotifications.GetItemCount() > 0)
	{
		m_listNotifications.SetSelectedItem(0);
		ShowDetails(m_listNotifications.GetItemCount() - 1);
	}
}



void CNotifyDlg::FillData_test()
{
	m_NotifyArray.clear();
	for(int i = 0; i < 300; i++)
	{
		wchar_t szIndex[1000] = {0};
		_snwprintf_s(szIndex, 1000, _TRUNCATE, L"time (%d)", i);


		NotifyInfo info;
		memset(&info, 0, sizeof(info));

		memcpy(info.time, szIndex, wcslen(szIndex) * sizeof(wchar_t));
		_snwprintf_s(info.message, _countof(info.message), _TRUNCATE, L"Description %d This action was denied.123456789 123456789 123456789 123456789 123456789123456789 00", i);
		info.enforcement = 1;
		wcsncpy_s(info.action, 50, L"OPEN", _TRUNCATE);

		m_NotifyArray.push_back(info);
		
		
	}

}
LRESULT CNotifyDlg::OnBnClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	ShowNotifications();

	return 0;
}

LRESULT CNotifyDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CWindow parent = GetParent();
	SendMessage(parent.m_hWnd, WM_COMMAND, IDOK, NULL);
	return 0;
}

LRESULT CNotifyDlg::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	BOOL bShow = (BOOL)wParam;
	if(bShow)
	{
		ShowNotifications();
	}
	return 0;
}

void CNotifyDlg::ShowNotifications()
{
	g_log.Log(CELOG_DEBUG, L"Try to show notifications, it will take some seconds.\n");

	FillData();
}

LRESULT CNotifyDlg::OnShowNotification(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	g_log.Log(CELOG_DEBUG, L"[Notify dialog] Show notification, id: %d\n", (int)lParam);
	if(m_listNotifications.GetItemCount() >= (int)lParam)
	{
		m_listNotifications.SetSelectedItem(m_listNotifications.GetItemCount() - (int)lParam);
		ShowDetails((int)lParam - 1);
	}
	return 0;
}

LRESULT CNotifyDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;

	HWND hWnd = (HWND)lParam;

	if(hWnd == GetDlgItem(IDC_NOTIFICATIONS_LABEL) )
	{
		SetTextColor(dc, RGB(0, 51, 153));
		SelectObject(dc, m_hFontBig);
	}
	else
	{
		SelectObject(dc, m_hFont);
	}

	SetBkColor(dc,   g_clrHighlight); 

	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}
	return   (LRESULT)hbr; 
}

LRESULT CNotifyDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	//Draw line
	RECT rcList;
	::GetWindowRect(GetDlgItem(IDC_LIST_NOTIFICATIONS), &rcList);
	ScreenToClient(&rcList);

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(123, 166, 198));
	HPEN hOld = (HPEN)SelectObject(hdc, hPen);

	HWND hDetail = GetDlgItem(IDC_NOTIFICATIONDETAIL);
	if(hDetail)
	{
		RECT rc;
		::GetWindowRect(hDetail, &rc);
		ScreenToClient(&rc);

		MoveToEx(hdc, rc.left + 58,rc.top + 8, NULL);
		LineTo(hdc, rcList.right, rc.top + 8);	
	}

	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps); 

	return 0;
}

LRESULT CNotifyDlg::OnLvnItemchangedList(int , LPNMHDR pNMHDR, BOOL& )
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	if(pNMLV && pNMLV->uNewState == 3)
	{
		LVITEM item;
		memset (&item, 0, sizeof (item));
		item.mask = LVIF_PARAM;
		item.iItem = pNMLV->iItem;

		if(m_listNotifications.GetItem(&item))
		{
			ShowDetails((int)item.lParam);
		}
	}
	return 0;
}

void CNotifyDlg::ShowDetails(int nIndex)
{
	for( unsigned i = 0; i < m_NotifyArray.size(); i++)
	{
		if(i == (unsigned)nIndex)
		{
			SetDlgItemTextW(IDC_DESCRIPTION, m_NotifyArray[i].message);
			if(m_NotifyArray[i].enforcement == DENY)
			{
				SetDlgItemTextW(IDC_ENFORCEMENT, L"Deny");
			}
			else
			{
				SetDlgItemTextW(IDC_ENFORCEMENT, L"Allow");
			}
			SetDlgItemTextW(IDC_ACTION, m_NotifyArray[i].action);
			SetDlgItemTextW(IDC_FILEPATH, m_NotifyArray[i].file);
		}
	}
}

LRESULT CNotifyDlg::OnLvnGetInfoTipList(int , LPNMHDR pNMHDR, BOOL& )
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	// TODO: Add your control notification handler code here

	if(pGetInfoTip)
	{
		if(!IsWin7())
		{
			LVITEM item;
			memset (&item, 0, sizeof (item));
			item.mask = LVIF_TEXT;
			item.iItem = pGetInfoTip->iItem;
			item.iSubItem = 1;
			wchar_t buf[1024] = {0};
			item.pszText = buf;
			item.cchTextMax = 1024;

			if(m_listNotifications.GetItem(&item))
			{
				wcsncpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, buf, _TRUNCATE);
			}
		}
		else{
			wcsncpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, L"", _TRUNCATE);
		}

	}
	return 0;
}

LRESULT CNotifyDlg::OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}
	return   (LRESULT)hbr; 
}
