#pragma once

#include "resource.h"       // main symbols



class CReqlogWarningDlg: public CAxDialogImpl<CReqlogWarningDlg>
{
public:
	CReqlogWarningDlg(void);
	~CReqlogWarningDlg(void);

	enum { IDD = IDD_ENABLELOGDLG };

	BEGIN_MSG_MAP(CReqlogWarningDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		CHAIN_MSG_MAP(CAxDialogImpl<CReqlogWarningDlg>)
	END_MSG_MAP()
public:

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

protected:
	HFONT m_hFont;
};

