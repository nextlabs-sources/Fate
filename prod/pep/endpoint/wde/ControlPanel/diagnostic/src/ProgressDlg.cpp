#include "stdafx.h"
#include "ProgressDlg.h"

CWaitProgressDlg::CWaitProgressDlg()
{
	memset(m_szDefaultStatusText, 0, sizeof(m_szDefaultStatusText));
}

CWaitProgressDlg::~CWaitProgressDlg()
{

}

LRESULT CWaitProgressDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CAxDialogImpl<CWaitProgressDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	HWND hProgress = ::GetDlgItem(m_hWnd, IDC_PROGRESS1);
	::SendMessage(hProgress, PBM_SETMARQUEE, (WPARAM)TRUE, 50);

	SetDlgItemTextW(IDC_SHOWSTATUS, m_szDefaultStatusText);
	

	CenterWindow();
	return TRUE;
}

LRESULT CWaitProgressDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CWaitProgressDlg::OnClose(UINT , WPARAM , LPARAM , BOOL& )
{
	return 0;
}

void CWaitProgressDlg::ShowStatusText(LPCWSTR lpszText)
{
	if(lpszText)
	{
		SetDlgItemTextW(IDC_SHOWSTATUS, lpszText);
	}
}

void CWaitProgressDlg::CloseProgressDlg()
{
	if(m_hWnd && ::IsWindow(m_hWnd))
	{
		EndDialog(1);
		m_hWnd = NULL;
	}
}

void CWaitProgressDlg::SetDefaultStatusText(LPCWSTR lpszText)
{
	if(lpszText)
	{
		wcsncpy_s(m_szDefaultStatusText, sizeof(m_szDefaultStatusText) / sizeof(wchar_t) - 1, lpszText, _TRUNCATE);
	}
}