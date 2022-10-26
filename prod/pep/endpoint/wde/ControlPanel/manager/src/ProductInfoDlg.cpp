#include "StdAfx.h"
#include "ProductInfoDlg.h"

CProductInfoDlg::CProductInfoDlg(void)
{
	m_nPercentAlpha = 80;
}

CProductInfoDlg::~CProductInfoDlg(void)
{
}

LRESULT CProductInfoDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CProductInfoDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	::SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// Make this window 60% alpha
	::SetLayeredWindowAttributes(m_hWnd, 0, (BYTE)((255 * m_nPercentAlpha) / 100), LWA_ALPHA);
	//Make the background color as transparent
	::SetLayeredWindowAttributes(m_hWnd, m_clrBk, 0, LWA_COLORKEY);


	return 1;
}

LRESULT CProductInfoDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 

	hdc = BeginPaint(&ps); 

	DrawBk(hdc);

	wchar_t szProductInfo[200] = {0};
	LoadStringW(g_hInstance, IDS_PRODUCTNAME, szProductInfo, 200);

	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldClr = SetTextColor(hdc, RGB(130, 130, 130));

	TextOutW(hdc, 20, 20, szProductInfo, (int)(wcslen(szProductInfo)));

	SetBkMode(hdc, nOldMode);
	SetTextColor(hdc, oldClr);

	EndPaint(&ps);

	return 0;
}
