

#include "stdafx.h"
#include "HdrProgDlg.h"
#include "log.h"



LRESULT CHdrProgDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CHdrProgDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	::SetWindowTextW(get_SafeTitleHwnd(), L"Running Document Inspector.");
	::SendMessageW(get_SafeProgHwnd(), PBM_SETRANGE, 0, MAKELPARAM(0, m_nRange));
	::SendMessageW(get_SafeProgHwnd(), PBM_SETSTEP, 1, 0);
	m_bShown = TRUE;
	m_bCancel= FALSE;

    DP((L"CHdrProgDlg::OnInitDialog (%d)\n", GetTickCount()));

	return 1;
}

LRESULT CHdrProgDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bShown = FALSE;
	return 0;//CAxDialogImpl<CHdrProgDlg>::OnDestroy(uMsg, wParam, lParam, bHandled);
}

LRESULT CHdrProgDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_bCancel= TRUE;
	EndDialog(IDCANCEL);
	return 0;
}

void    CHdrProgDlg::CloseProgWnd(INT_PTR nResult)
{
    m_bShown = FALSE;
	EndDialog((int)nResult);
}

BOOL    CHdrProgDlg::MakeStep(int nStep)
{
	int i = 0;
	if(!m_bShown)
		return FALSE;
	for(i=0; i<nStep; i++)
	{
		::SendMessageW(get_SafeProgHwnd(), PBM_STEPIT, 0, 0);
	}
	return TRUE;
}

#define TITLEPREFIX		L"Inspecting document for:\n"
void    CHdrProgDlg::SetInspectItem(LPCWSTR wzItem)
{
	WCHAR wzTitle[1025];	memset(wzTitle, 0, sizeof(wzTitle));
	wcsncpy_s(wzTitle, 1025, TITLEPREFIX, _TRUNCATE);
//	wcsncat(wzTitle, wzItem, 1024-(int)wcslen(TITLEPREFIX));
	::SetWindowTextW(get_SafeTitleHwnd(), wzTitle);

	HWND hWnd = ::GetDlgItem(m_hWnd, IDC_TITLE2);
	if(hWnd)
	{
		::SetWindowTextW(hWnd, wzItem);
	}

}