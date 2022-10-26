#include "StdAfx.h"
#include <vector>
#include <string>
#include <shellapi.h>

#include "CompleteDlg.h"
#include "edpmgrutilities.h"
#include "utilities.h"
#include "DelLogConfirmDlg.h"


using namespace std;


CCompleteDlg::CCompleteDlg(void)
{
	m_hFont = CreateFontW(-14,
							0,
							0,
							0,
							FW_NORMAL,
							0,
							1,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);

	m_pDelLogDlg = NULL;
}

CCompleteDlg::~CCompleteDlg(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	if (m_pDelLogDlg && !m_pDelLogDlg->m_hWnd)
	{
		delete m_pDelLogDlg;
		m_pDelLogDlg = NULL;
	}
}

LRESULT CCompleteDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	wchar_t strHere[100] = {0};
	LoadStringW(g_hInstance, IDS_HERE, strHere, 100);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_CLICKHERE), &rc);
	ScreenToClient(&rc);

	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldClr = SetTextColor(hdc, RGB(0, 0, 255));
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

	DrawTextExW(hdc, strHere, (int)(wcslen(strHere)), &rc, DT_WORDBREAK | DT_LEFT |DT_TOP, NULL);

	SetTextColor(hdc, oldClr);
	SetBkMode(hdc, nOldMode);
	SelectObject(hdc, hOldFont);

	::EndPaint(m_hWnd, &ps); 

	return 0;
}

LRESULT CCompleteDlg::OnLButtonUp(UINT , WPARAM , LPARAM lParam, BOOL& )
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_CLICKHERE), &rc);
	ScreenToClient(&rc);

	if(PtInRect(&rc, pt))
	{
		OnDeleteLogs();
	}

	return 0;
}

LRESULT CCompleteDlg::OnMouseMove(UINT , WPARAM , LPARAM lParam, BOOL& )
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_CLICKHERE), &rc);
	ScreenToClient(&rc);

	if(PtInRect(&rc, pt))
	{
		SetCursor(LoadCursorW(NULL, IDC_HAND));
	}
	else
	{
		SetCursor(LoadCursorW(NULL, IDC_ARROW));
	}

	return 0;
}

void CCompleteDlg::OnDeleteLogs()
{
	if (m_pDelLogDlg && !m_pDelLogDlg->m_hWnd)
	{
		delete m_pDelLogDlg;
		m_pDelLogDlg = NULL;
	}
	if (!m_pDelLogDlg)
	{
		m_pDelLogDlg = new CDelLogConfirmDlg;
		if (!m_pDelLogDlg)
		{
			return;
		}
		m_pDelLogDlg->DoModal();
	}
}
LRESULT CCompleteDlg::OnBnClickedButtonRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if (m_pDelLogDlg && !m_pDelLogDlg->m_hWnd)
	{
		delete m_pDelLogDlg;
		m_pDelLogDlg = NULL;
	}
	if (!m_pDelLogDlg)
	{
		m_pDelLogDlg = new CDelLogConfirmDlg;
		if (!m_pDelLogDlg)
		{
			return 0;
		}
		m_pDelLogDlg->DoModal();
	}
	return 0;
}
