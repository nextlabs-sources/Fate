#include "stdafx.h"
#include "ProgressDlg.h"

CProgressDlg::CProgressDlg():m_bTopMost(FALSE)
{
	memset(m_szDefaultStatusText, 0, sizeof(m_szDefaultStatusText));
}

CProgressDlg::~CProgressDlg()
{

}

LRESULT CProgressDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	CAxDialogImpl<CProgressDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	HICON hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, FALSE);
	}

	HWND hProgress = ::GetDlgItem(m_hWnd, IDC_PROGRESS1);
	::SendMessage(hProgress, PBM_SETMARQUEE, (WPARAM)TRUE, 50);

	SetDlgItemTextW(IDC_SHOWSTATUS, m_szDefaultStatusText);

	if(m_bTopMost)
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	CenterWindow();
	return TRUE;
}

LRESULT CProgressDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CProgressDlg::OnClose(UINT , WPARAM , LPARAM , BOOL& )
{
	return 0;
}

void CProgressDlg::ShowStatusText(LPCWSTR lpszText)
{
	if(lpszText)
	{
		SetDlgItemTextW(IDC_SHOWSTATUS, lpszText);
	}
}

void CProgressDlg::CloseProgressDlg()
{
	if(m_hWnd && ::IsWindow(m_hWnd))
	{
		EndDialog(1);
		m_hWnd = NULL;
	}
}

void CProgressDlg::SetDefaultStatusText(LPCWSTR lpszText)
{
	if(lpszText)
	{
		wcsncpy_s(m_szDefaultStatusText, sizeof(m_szDefaultStatusText) / sizeof(wchar_t) - 1, lpszText, _TRUNCATE);
	}
}

LRESULT CProgressDlg::OnUpdateStatus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	if(lParam)
	{
		LPCWSTR pwzText = (LPCWSTR)lParam;
		ShowStatusText(pwzText);
	}
	return 0;
}