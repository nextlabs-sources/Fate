


#pragma once

#ifndef _HDRPROG_DLG_H_
#define _HDRPROG_DLG_H_

#include "resource.h"       // main symbols
#pragma warning(push)
#pragma warning(disable: 6387 6386)
#include <atlhost.h>
#pragma warning(pop)
class CHdrProgDlg : 
	public CAxDialogImpl<CHdrProgDlg>
{
public:
	CHdrProgDlg(){m_bShown = FALSE; m_bCancel=FALSE;};
	~CHdrProgDlg(){};

	enum { IDD = IDD_PROGRESSDLG };

	BEGIN_MSG_MAP(CHdrProgDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDC_DIPCANCEL, BN_CLICKED, OnClickedCancel)
		CHAIN_MSG_MAP(CAxDialogImpl<CHdrProgDlg>)
	END_MSG_MAP()

public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	BOOL    MakeStep(int nStep = 1);
	void    SetInspectItem(LPCWSTR wzItem);
	void    CloseProgWnd(INT_PTR nResult = IDOK);

	inline void put_ParentHwnd(HWND hParent){m_hParent = hParent;};
	inline void put_ProgRange(int nRange){m_nRange = nRange;};
	inline HWND get_ParentHwnd(){return m_hParent;};
	inline HWND get_SafeHwnd(){return this->m_hWnd;};
	inline HWND get_SafeTitleHwnd(){return ::GetDlgItem(this->m_hWnd, IDC_DIPTITLE);};
	inline HWND get_SafeProgHwnd(){return ::GetDlgItem(this->m_hWnd, IDC_DIPPROG);};
	inline BOOL get_Shown(){return m_bShown;};
	inline BOOL get_Canceled(){return m_bCancel;};

private:
	int		m_nRange;
	HWND	m_hParent;
	BOOL	m_bShown;
	BOOL	m_bCancel;
};



#endif