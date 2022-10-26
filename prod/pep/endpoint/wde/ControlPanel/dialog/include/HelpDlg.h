#pragma once

#include "resource.h"

class CHelpDlg: public CAxDialogImpl<CHelpDlg>
{
public:
	CHelpDlg(void);
	~CHelpDlg(void);

	enum { IDD = IDD_HELP  };

	BEGIN_MSG_MAP(CHelpDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		CHAIN_MSG_MAP(CAxDialogImpl<CHelpDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}


	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_Link.UnsubclassWindow();
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		EndDialog(0);
		return 0;
	}

	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	HFONT m_hFont;

	CHyperLink m_Link;
};

class CHelpDlgModeless: public CAxDialogImpl<CHelpDlgModeless>
{
public:
	CHelpDlgModeless(void);
	~CHelpDlgModeless(void);

	enum { IDD = IDD_HELP_MODALESS  };

	BEGIN_MSG_MAP(CHelpDlgModeless)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		CHAIN_MSG_MAP(CAxDialogImpl<CHelpDlgModeless>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		DestroyWindow();
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DestroyWindow();
		return 0;
	}

	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	HFONT m_hFont;

	CHyperLink m_Link;
};
