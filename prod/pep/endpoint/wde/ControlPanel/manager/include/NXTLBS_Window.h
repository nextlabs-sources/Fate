#pragma once

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlhost.h>
#pragma warning(pop)

template<class T>
class CNXTLBS_Window: public CAxDialogImpl<T>
{
public:
	CNXTLBS_Window()
	{
		m_nShadowLeftMargin = 3;
		m_nShadowTopMargin = 3;
		m_nShadowRightMargin = 1;
		m_nShadowBottomMargin = 1;

		m_nBorderLeftMargin = 1;
		m_nBorderTopMargin = 1;
		m_nBorderRightMargin = 2;
		m_nBorderBottomMargin = 2;

		m_nRoundWidth = 3;
		m_nRoundHeight = 3;

		m_nRoundWidthShadow = 5;
		m_nRoundHeightShadow = 5;

		m_clrBk = RGB(240, 241, 247);
		m_clrBorder = RGB(224, 224, 224);
		m_clrShadow = RGB(190, 190, 190);

		m_nPercentAlpha = 60;//60% alpha, 0 means TRANSPARENT, 100 means NON-TRANSPARENT.

		//Close button
		m_nRightMargin = 50;
		m_nTopMargin = 20;
		m_nCloseWidth = 20;//"Close" button width
		m_nCloseHeight = 30;//"Close" button height
		m_clrClose = RGB(255, 0, 0);
		memset(m_szText, 0, sizeof(m_szText));
		wcsncpy_s(m_szText, sizeof(m_szText)/sizeof(wchar_t), L"X", _TRUNCATE);
		m_hCloseFont = CreateFontW(18,
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

		
	}

	virtual ~CNXTLBS_Window()
	{
		if(m_hCloseFont)
		{
			DeleteObject(m_hCloseFont);
			m_hCloseFont = 0;
		}
	}

	//the sub class should implement the Message Map, so we don't need it here.
// 	BEGIN_MSG_MAP(T)
// 		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
// 		MESSAGE_HANDLER(WM_PAINT, OnPaint)
// 		CHAIN_MSG_MAP(CAxDialogImpl<T>)
// 		REFLECT_NOTIFICATIONS();
// 	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;

		RECT rc;
		GetWindowRect(&rc);

		HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 30,30);
		SetWindowRgn(rgn, TRUE);
		DeleteObject(rgn);


		::SetWindowLongW(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		// Make this window 70% alpha
		::SetLayeredWindowAttributes(m_hWnd, 0, (BYTE)((255 * m_nPercentAlpha) / 100), LWA_ALPHA);


		//	MoveWindow(100, 100, 100, 100);
		CenterWindow(m_hWnd);	

		//Calculate the rect of "CLOSE" button.
		GetWindowRect(&rc);
		ScreenToClient(&rc);

		m_rcClose.left = rc.right - m_nRightMargin;
		m_rcClose.top = rc.top + m_nTopMargin;
		m_rcClose.right = m_rcClose.left + m_nCloseWidth;
		m_rcClose.bottom = m_rcClose.top + m_nCloseHeight;

		return CAxDialogImpl<T>::OnInitDialog(uMsg, wParam, lParam, bHandled);  // Let the system set the focus
	}

	virtual void DrawBk(HDC hdc)
	{
		RECT rc; 
		GetWindowRect(&rc);
		ScreenToClient(&rc);

		//Draw background color
		HBRUSH hBrush = CreateSolidBrush(m_clrBk);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

		FillRect(hdc, &rc, hBrush);
		SelectObject(hdc, hOldBrush);

		DeleteObject(hBrush);

		//Draw shadow
		HPEN hPen = CreatePen(PS_SOLID, 10, m_clrShadow);
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		RoundRect(hdc, rc.left + m_nShadowLeftMargin, rc.top + m_nShadowTopMargin, rc.right - m_nShadowRightMargin, rc.bottom - m_nShadowBottomMargin, m_nRoundWidthShadow, m_nRoundHeightShadow);
		DeleteObject(hPen);

		//Draw border
		hPen = CreatePen(PS_SOLID, 1, m_clrBorder);
		SelectObject(hdc, hPen);

		RoundRect(hdc, rc.left + m_nBorderLeftMargin, rc.top + m_nBorderTopMargin, rc.right - m_nBorderRightMargin, rc.bottom - m_nBorderBottomMargin, m_nRoundWidth, m_nRoundHeight);
		DeleteObject(hPen);

		SelectObject(hdc, hOldPen);

		//Draw Close button
// 		COLORREF hOldClr = SetTextColor(hdc, m_clrClose);
// 		HFONT hOldFont = (HFONT)SelectObject(hdc, m_hCloseFont);
// 		TextOut(hdc, rc.right - m_nRightMargin, rc.top + m_nTopMargin, m_szText, wcslen(m_szText));
// 		SelectObject(hdc, hOldFont);
// 		SetTextColor(hdc, hOldClr);
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// TODO: Add your message handler code here and/or call default
		PAINTSTRUCT ps; 
		HDC hdc; 
		hdc = BeginPaint(&ps); 
		
		DrawBk(hdc);
		
		EndPaint(&ps);
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt;

		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);

		if(PtInRect(&m_rcClose, pt))
		{
			SetCursor(LoadCursorW(NULL, IDC_HAND));
		}
		else
		{
			SetCursor(LoadCursorW(NULL, IDC_ARROW) );
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt;
		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);

		if(PtInRect(&m_rcClose, pt))
		{
			::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
		}
		return 0;
	}
protected:
	int m_nPercentAlpha;
	COLORREF m_clrBk;
	COLORREF m_clrShadow;
	COLORREF m_clrBorder;

	int m_nShadowLeftMargin;//The margin to window
	int m_nShadowRightMargin;
	int m_nShadowTopMargin;
	int m_nShadowBottomMargin;

	int m_nBorderLeftMargin;
	int m_nBorderRightMargin;
	int m_nBorderTopMargin;
	int m_nBorderBottomMargin;
	int m_nRoundWidth;
	int m_nRoundHeight;

	int m_nRoundWidthShadow;
	int m_nRoundHeightShadow;

	//Close button, this is for left-top point
	int m_nRightMargin;//the position to right border.
	int m_nTopMargin;//the position to top border.
	int m_nCloseWidth;//The area of "click responding"
	int m_nCloseHeight;
	COLORREF m_clrClose;
	wchar_t m_szText[100];
	HFONT m_hCloseFont;
	RECT m_rcClose;

};