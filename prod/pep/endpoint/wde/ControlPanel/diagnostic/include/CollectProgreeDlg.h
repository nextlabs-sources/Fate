#pragma once

#include "resource.h"       // main symbols



class CCollectProgreeDlg: public CAxDialogImpl<CCollectProgreeDlg>
{


public:

	CCollectProgreeDlg();
	~CCollectProgreeDlg(void);


	enum { IDD = IDD_PROGRESSDLG };
	
	/*
	
	set on cancel to progress dialog, progress dialog will call this callback to notify that user cancel event.
	
	*/
	typedef void (*OnCancelType)(PVOID param);
	void SetOnCancel(OnCancelType pOnCancel, PVOID param);


	/*
	
	set on completed to progress dialog, progress dialog will call this callback to notify diagnostic complete event.
	
	*/
	typedef void (*OnCompletedType)(PVOID param, DWORD res);
	void SetOnCompleted(OnCompletedType pOnCompleted, PVOID param);


	BEGIN_MSG_MAP(CCollectProgreeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnBnClickedCancel)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
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

private:
	OnCancelType m_pOnCancel;
	PVOID m_pOnCancelParam;

	OnCompletedType m_pOnCompleted;
	PVOID m_pOnCompletedParam;

	static void OnCompleted(PVOID param, DWORD res);
};

