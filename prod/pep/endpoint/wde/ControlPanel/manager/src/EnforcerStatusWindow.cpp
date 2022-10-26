#include "StdAfx.h"
#include "EnforcerStatusWindow.h"

#define ENFORCERSTATUS_LEFT			20
#define ENFORCERSTATUS_TOP			20
#define ENFORCERSTATUS_BOTTOM       20
#define ENFORCERSTATUS_TAB			10
#define ENFORCERSTATUS_INTERVAL		30
#define ENFORCERSTATUS_HEIGHTMARGIN	2

CEnforcerStatusWindow::CEnforcerStatusWindow(void)
{
	m_hEnforcerNameFont = CreateFontW(18,
										0,
										0,
										0,
										FW_BOLD,
										0,
										0,
										0,
										DEFAULT_CHARSET,
										OUT_DEFAULT_PRECIS,
										CLIP_DEFAULT_PRECIS,
										CLEARTYPE_QUALITY,
										DEFAULT_PITCH | FF_DONTCARE,
										L"Arial"
										);

	m_hEnforcerStatusFont =  CreateFontW(18,
											0,
											0,
											0,
											FW_BOLD,
											TRUE,
											0,
											0,
											DEFAULT_CHARSET,
											OUT_DEFAULT_PRECIS,
											CLIP_DEFAULT_PRECIS,
											CLEARTYPE_QUALITY,
											DEFAULT_PITCH | FF_DONTCARE,
											L"Arial"
											);

	m_clrEnforcerName = RGB(0, 0, 0);
	m_clrEnforcerStatus = RGB(0, 0, 0);
}

CEnforcerStatusWindow::~CEnforcerStatusWindow(void)
{
	if(m_hEnforcerNameFont)
	{
		DeleteObject(m_hEnforcerNameFont);
		m_hEnforcerNameFont = NULL;
	}

	if(m_hEnforcerStatusFont)
	{
		DeleteObject(m_hEnforcerStatusFont);
		m_hEnforcerStatusFont = NULL;
	}
}

LRESULT CEnforcerStatusWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	PAINTSTRUCT ps; 
	HDC hdc; 

	hdc = BeginPaint(&ps); 

	DrawBk(hdc);

	vector<ENFORCERSTATUS>::iterator iter;
	
	int x = ENFORCERSTATUS_LEFT;
	int y = ENFORCERSTATUS_TOP;

	COLORREF oldClr = SetTextColor(hdc, m_clrEnforcerName);
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hEnforcerNameFont);

	for( iter = m_vEnforcers.begin(); iter != m_vEnforcers.end(); iter++)
	{
		//Draw enforcer name
		SetTextColor(hdc, m_clrEnforcerName);
		SelectObject(hdc, m_hEnforcerNameFont);

		TextOutW(hdc, x, y, (*iter).strEnforcerName.c_str(), (int)(*iter).strEnforcerName.length());

		SIZE len;
		GetTextExtentPoint32W(hdc, (*iter).strEnforcerName.c_str(), (int)(*iter).strEnforcerName.length(), &len);

		y += len.cy + ENFORCERSTATUS_HEIGHTMARGIN;

		//Draw enforcer status
		SetTextColor(hdc, m_clrEnforcerStatus);
		SelectObject(hdc, m_hEnforcerStatusFont);

		TextOutW(hdc, x + ENFORCERSTATUS_TAB, y, (*iter).strEnforcerStatus.c_str(), (int)(*iter).strEnforcerStatus.length());

		GetTextExtentPoint32W(hdc, (*iter).strEnforcerStatus.c_str(), (int)(*iter).strEnforcerStatus.length(), &len);

		y += len.cy + ENFORCERSTATUS_HEIGHTMARGIN;

		y += ENFORCERSTATUS_INTERVAL;
	}

	SelectObject(hdc, hOldFont);
	SetTextColor(hdc, oldClr);

	EndPaint(&ps);

	return 0;
}

void CEnforcerStatusWindow::SetEnforcerStatusInfo(vector<ENFORCERSTATUS>& vEnforcers)
{
	m_vEnforcers = vEnforcers;
}

LRESULT CEnforcerStatusWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	 CAxDialogImpl<CEnforcerStatusWindow>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	return TRUE;
}

void CEnforcerStatusWindow::ComputePosition(IN OUT int& x, IN OUT int& y)
{
	x; //reserved

	HDC hdc = GetDC();
	if(!hdc)
	{
		return;
	}

	if(m_vEnforcers.size() == 0)
	{
		return;
	}

	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hEnforcerNameFont);

	wstring strTemp(L"Compute text height");

	SIZE len;
	GetTextExtentPoint32W(hdc, strTemp.c_str(), (int)strTemp.length(), &len);

	int nItemHeight;
	nItemHeight = len.cy;

	SelectObject(hdc, m_hEnforcerStatusFont);
	
	GetTextExtentPoint32W(hdc, strTemp.c_str(), (int)strTemp.length(), &len);

	nItemHeight += ENFORCERSTATUS_HEIGHTMARGIN;
	nItemHeight += len.cy;

	SelectObject(hdc, hOldFont);

	ReleaseDC(hdc);

	y -= (int)(m_vEnforcers.size() - 1) * (nItemHeight + ENFORCERSTATUS_INTERVAL) + nItemHeight + ENFORCERSTATUS_TOP + ENFORCERSTATUS_BOTTOM;
	
}

void CEnforcerStatusWindow::ShowEnforcerStatus(int x, int y)
{
	if(m_hWnd)
	{
		ComputePosition(x, y);

		RECT rc;
		GetWindowRect(&rc);

		SetWindowPos(HWND_TOPMOST, x, y, rc.right - rc.left, y, SWP_SHOWWINDOW);
	}
}