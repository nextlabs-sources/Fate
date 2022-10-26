#include "StdAfx.h"
#include "HelpDlg.h"
#include "EDPMgrUtilities.h"

CHelpDlg::CHelpDlg(void)
{
	m_hFont = CreateFontW(-14,
							0,
							0,
							0,
							FW_BOLD,
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
}

CHelpDlg::~CHelpDlg(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}

LRESULT CHelpDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CHelpDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	SetIcon(LoadIconW(g_hInst, MAKEINTRESOURCEW(IDI_EDPMHELP)) );

	m_Link.SubclassWindow(GetDlgItem(IDC_WEBLINK));

	CenterWindow();



	//	below is getting and setting version and install date.......................
	//	in fact, they are wde version and wde install date
	vector<pair<wstring, wstring>> v_version;
	vector<pair<wstring, wstring>> v_lastDate;

	CEDPMUtilities& edpmgr_utility = CEDPMUtilities::GetInstance();
	edpmgr_utility.GetComponentsVersion(v_version);
	edpmgr_utility.GetComponentsLastUpdatedDate(v_lastDate);

	for (DWORD i = 0; i < v_version.size(); i++)
	{
		if (v_version[i].first == wstring(L"Desktop Enforcer"))
		{
			//	this is desktop enforcer, we get the version
			wstring strVersion = wstring(L"Version: ") + v_version[i].second;
			SetDlgItemText(IDC_STATIC_VERSION, strVersion.c_str());
		}

		if (v_lastDate[i].first == wstring(L"Desktop Enforcer"))
		{
			//	this is desktop enforcer, we get the install date

			//	20100528 -> 05/28/2010 
			wstring strTargetMonth = v_lastDate[i].second.substr(4, 2);
			if (L'0' == strTargetMonth[0])
			{
				strTargetMonth = strTargetMonth.substr(1, 1);
			}

			wstring strTargetDate = v_lastDate[i].second.substr(6, 2) ;
			if(L'0' == strTargetDate[0])
			{
				strTargetDate = strTargetDate.substr(1, 1);
			}

			wstring strTargetYear = v_lastDate[i].second.substr(0, 4);


			wstring strDate = wstring(L"Date Installed: ") + strTargetMonth + wstring(L"/") + strTargetDate +  wstring(L"/") + strTargetYear;;
			SetDlgItemText(IDC_STATIC_DATE, strDate.c_str());
		}
	}

	return 0;
}


LRESULT CHelpDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;
	HWND hWnd = (HWND)lParam;

	hWnd;	
//	SetTextColor(dc, RGB(0, 0, 255));
	SelectObject(dc, m_hFont);
	
	SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 

	//	ReleaseDC(dc);
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 

}

LRESULT CHelpDlgModeless::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;
	HWND hWnd = (HWND)lParam;

	hWnd;	
	//	SetTextColor(dc, RGB(0, 0, 255));
	SelectObject(dc, m_hFont);

	SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 

	//	ReleaseDC(dc);
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 

}


CHelpDlgModeless::CHelpDlgModeless(void)
{
	m_hFont = CreateFontW(-14,
		0,
		0,
		0,
		FW_BOLD,
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
}

CHelpDlgModeless::~CHelpDlgModeless(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}

LRESULT CHelpDlgModeless::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CHelpDlgModeless>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	SetIcon(LoadIconW(g_hInst, MAKEINTRESOURCEW(IDI_EDPMHELP)) );

	m_Link.SubclassWindow(GetDlgItem(IDC_WEBLINK));

	CenterWindow();



	//	below is getting and setting version and install date.......................
	//	in fact, they are wde version and wde install date
	vector<pair<wstring, wstring>> v_version;
	vector<pair<wstring, wstring>> v_lastDate;

	CEDPMUtilities& edpmgr_utility = CEDPMUtilities::GetInstance();
	edpmgr_utility.GetComponentsVersion(v_version);
	edpmgr_utility.GetComponentsLastUpdatedDate(v_lastDate);

	for (DWORD i = 0; i < v_version.size(); i++)
	{
		if (v_version[i].first == wstring(L"Desktop Enforcer"))
		{
			//	this is desktop enforcer, we get the version
			wstring strVersion = wstring(L"Version: ") + v_version[i].second;
			SetDlgItemText(IDC_STATIC_VERSION, strVersion.c_str());
		}

		if (v_lastDate[i].first == wstring(L"Desktop Enforcer"))
		{
			//	this is desktop enforcer, we get the install date

			//	20100528 -> 05/28/2010 
			wstring strTargetMonth = v_lastDate[i].second.substr(4, 2);
			if (L'0' == strTargetMonth[0])
			{
				strTargetMonth = strTargetMonth.substr(1, 1);
			}

			wstring strTargetDate = v_lastDate[i].second.substr(6, 2) ;
			if(L'0' == strTargetDate[0])
			{
				strTargetDate = strTargetDate.substr(1, 1);
			}

			wstring strTargetYear = v_lastDate[i].second.substr(0, 4);


			wstring strDate = wstring(L"Date Installed: ") + strTargetMonth + wstring(L"/") + strTargetDate +  wstring(L"/") + strTargetYear;;
			SetDlgItemText(IDC_STATIC_DATE, strDate.c_str());
		}
	}

	return TRUE;
}
