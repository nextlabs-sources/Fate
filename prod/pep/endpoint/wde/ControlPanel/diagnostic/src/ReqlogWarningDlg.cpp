#include "StdAfx.h"
#include "ReqlogWarningDlg.h"

CReqlogWarningDlg::CReqlogWarningDlg(void)
{
	m_hFont = CreateFontW(-12,
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
}

CReqlogWarningDlg::~CReqlogWarningDlg(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}


LRESULT CReqlogWarningDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	EndDialog(0);
	return 0;
}

LRESULT CReqlogWarningDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;
	HWND hWnd = (HWND)lParam;

	hWnd;	
//	SetTextColor(dc, RGB(0, 0, 255));
	SelectObject(dc, g_hFont);

	SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 

	
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 

}

LRESULT CReqlogWarningDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	wchar_t strEnable[101] = {0};
	GetDlgItemTextW(IDC_ENABLELOGGING, strEnable, 100);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_ENABLELOGGING), &rc);
	ScreenToClient(&rc);

	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldClr = SetTextColor(hdc, RGB(0, 0, 255));
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

	DrawTextExW(hdc, strEnable, (int)(wcslen(strEnable)), &rc, DT_WORDBREAK | DT_LEFT |DT_TOP, NULL);

	SetTextColor(hdc, oldClr);
	SetBkMode(hdc, nOldMode);
	SelectObject(hdc, hOldFont);

	::EndPaint(m_hWnd, &ps); 

	return 0;
}

LRESULT CReqlogWarningDlg::OnLButtonUp(UINT , WPARAM , LPARAM lParam, BOOL& )
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_ENABLELOGGING), &rc);
	ScreenToClient(&rc);

	if(PtInRect(&rc, pt))
	{
#ifdef _WIN64
		HMODULE hMainDlg = GetModuleHandleW(L"edpmdlg.dll");
#else
		HMODULE hMainDlg = GetModuleHandleW(L"edpmdlg32.dll");
#endif
		
		if(hMainDlg)
		{
			typedef BOOL (*DoActionType) (DWORD dwIndex);
			DoActionType lfDoAction = (DoActionType)GetProcAddress(hMainDlg, "DoAction");
			if(lfDoAction)
			{
				lfDoAction(20);
				::SendMessage(GetParent().m_hWnd, WM_CLOSE, 0, 0);
			}
		}
	}
	
	return 0;
}

LRESULT CReqlogWarningDlg::OnMouseMove(UINT , WPARAM , LPARAM lParam, BOOL& )
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_ENABLELOGGING), &rc);
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
