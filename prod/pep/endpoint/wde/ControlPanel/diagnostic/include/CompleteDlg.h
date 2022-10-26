#pragma once

#include "resource.h"       // main symbols


class CDelLogConfirmDlg;
class CCompleteDlg: public CAxDialogImpl<CCompleteDlg>
{
public:
	CCompleteDlg();
	~CCompleteDlg(void);


	enum { IDD = IDD_COMPLETEDLG };

	BEGIN_MSG_MAP(CCompleteDlg)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDC_BUTTON_REMOVE, BN_CLICKED, OnBnClickedButtonRemove)
		CHAIN_MSG_MAP(CAxDialogImpl<CCompleteDlg>)
	END_MSG_MAP()

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		HDC dc = (HDC)wParam;
		HWND hWnd = (HWND)lParam;

		hWnd;	
		//	SetTextColor(dc, RGB(0, 0, 255));
		SelectObject(dc, g_hFont);

		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 


		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}

protected:
	void OnDeleteLogs();
protected:
	HFONT m_hFont;

	CDelLogConfirmDlg* m_pDelLogDlg;
public:
	LRESULT OnBnClickedButtonRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

