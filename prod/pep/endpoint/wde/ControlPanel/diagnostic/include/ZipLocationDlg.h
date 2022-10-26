#pragma once



#include "resource.h"       // main symbols



class CZipLocationDlg: public CAxDialogImpl<CZipLocationDlg>
{
public:
	CZipLocationDlg();
	~CZipLocationDlg(void);

	enum { IDD = IDD_LOCATIONDLG };

	BEGIN_MSG_MAP(CZipLocationDlg)
		COMMAND_HANDLER(IDC_BROWSER, BN_CLICKED, OnBrowser)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		COMMAND_HANDLER(IDC_LOCATION, EN_CHANGE, OnEnChangeLocation)
		CHAIN_MSG_MAP(CAxDialogImpl<CZipLocationDlg>)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	END_MSG_MAP()

	LRESULT OnBrowser(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		HDC dc = (HDC)wParam;
		HWND hWnd = (HWND)lParam;

		if (hWnd == GetDlgItem(IDC_LOCATION).m_hWnd)
		{
			return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
		}
		//	SetTextColor(dc, RGB(0, 0, 255));
		SelectObject(dc, g_hFont);

		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 


		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}
	LRESULT OnEnChangeLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

