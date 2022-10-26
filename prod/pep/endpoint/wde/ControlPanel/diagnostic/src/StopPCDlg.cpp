#include "StdAfx.h"
#include "StopPCDlg.h"
#include "EDPMgrUtilities.h"
#include "CEsdk.h"
#include "ZipLocationDlg.h"
#include "PWDMgr.h"



CStopPCDlg::CStopPCDlg(void)
{
}

CStopPCDlg::~CStopPCDlg(void)
{
}


/*

when user edit password for Enterprise DLP,
enable "Next >" button. On default, it is disabled.

*/
LRESULT CStopPCDlg::OnEnChangePassword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CAxDialogImpl<CStopPCDlg>::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
/*	HWND hParent = GetParent( );
	HWND hNext = ::GetDlgItem(hParent, IDC_NEXT);

	wchar_t szPsw[101] = {0};
	GetDlgItemTextW(IDC_PASSWORD, szPsw, 100);
	if(wcslen(szPsw) > 0)
	{//Enable the "next button" if user types any characters
	::EnableWindow(hNext, TRUE);
	}
	else
	{
		::EnableWindow(hNext, FALSE);
	}
*/
	return 0;
}

LRESULT CStopPCDlg::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if (TRUE == (BOOL)wParam)
	{
		CWindow pwdWindow = GetDlgItem(IDC_PASSWORD);
		pwdWindow.EnableWindow(TRUE);
	}
	else
	{
		CWindow pwdWindow = GetDlgItem(IDC_PASSWORD);
		pwdWindow.EnableWindow(FALSE);
	}

	return 0;
}

LRESULT CStopPCDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	CWindow pwdWindow = GetDlgItem(IDC_PASSWORD);
	pwdWindow.EnableWindow(FALSE);

	return TRUE;
}

void CStopPCDlg::ShowWaitingInfo(BOOL bShow/* = TRUE */)
{
	::SendMessage(GetDlgItem(IDC_WAIT_PROGRESS), PBM_SETMARQUEE, (WPARAM)TRUE, 50);
	::ShowWindow(GetDlgItem(IDC_WAIT_PROGRESS), bShow? SW_SHOW: SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_WAIT_STATUS), bShow? SW_SHOW: SW_HIDE);
}

void CStopPCDlg::SetStatusText(LPCWSTR pwzText)
{
	if(pwzText)
	{
		SetDlgItemTextW(IDC_WAIT_STATUS, pwzText);
	}
}

