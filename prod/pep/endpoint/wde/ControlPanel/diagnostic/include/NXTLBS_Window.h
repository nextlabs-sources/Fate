#pragma once

#include "resource.h"

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlhost.h>
#pragma warning(pop)

#include "shellapi.h"


class CNXTLBS_Window: public CAxDialogImpl<CNXTLBS_Window>
{
public:
	CNXTLBS_Window()
	{
		m_nShadowLeftMargin = 6;
		m_nShadowTopMargin = 6;
		m_nShadowRightMargin = 1;
		m_nShadowBottomMargin = 1;

		m_nBorderLeftMargin = 1;
		m_nBorderTopMargin = 1;
		m_nBorderRightMargin = 5;
		m_nBorderBottomMargin = 5;

		m_nRoundWidth = 3;
		m_nRoundHeight = 3;

		m_nRoundWidthShadow = 5;
		m_nRoundHeightShadow = 5;

		m_clrBk = RGB(240, 241, 247);
		m_clrShadow = RGB(100, 100, 100);
		m_clrBorder = RGB(0, 0, 0);

		m_nPercentAlpha = 100;//60% alpha, 0 means TRANSPARENT, 100 means NON-TRANSPARENT.

		//Close button
		m_nRightMargin = 30;
		m_nTopMargin = 15;
		m_nCloseWidth = 15;//"Close" button width
		m_nCloseHeight = 18;//"Close" button height
		m_clrClose = RGB(130, 130, 130);
		memset(m_szText, 0, sizeof(m_szText));
		wcsncpy_s(m_szText, sizeof(m_szText)/sizeof(wchar_t), L"X", _TRUNCATE);
		m_hCloseFont = CreateFontW(-20,
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
			g_szFont
			);

		m_hFont = CreateFontW(-16,
								0,
								0,
								0,
								FW_NORMAL,
								0,
								0,
								0,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								CLEARTYPE_QUALITY,
								DEFAULT_PITCH | FF_DONTCARE,
								g_szFont
								);

		m_hFontUnderline = CreateFontW(-16,
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

		m_rcLink.left = 20;
		m_rcLink.top = 45 + 40;
		m_rcLink.right = m_rcLink.left + 40;
		m_rcLink.bottom = m_rcLink.top + 20;

	}

	virtual ~CNXTLBS_Window()
	{
		if(m_hCloseFont)
		{
			DeleteObject(m_hCloseFont);
			m_hCloseFont = 0;
		}
		if(m_hFont)
		{
			DeleteObject(m_hFont);
			m_hFont = NULL;
		}

		if(m_hFontUnderline)
		{
			DeleteObject(m_hFontUnderline);
			m_hFontUnderline = NULL;
		}
	}

	enum { IDD = IDD_LOGPROMPT};

	BEGIN_MSG_MAP(CNXTLBS_Window)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		CHAIN_MSG_MAP(CAxDialogImpl<CNXTLBS_Window>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;

		::SetWindowLongW(m_hWnd, GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		// Make this window 70% alpha
		::SetLayeredWindowAttributes(m_hWnd, 0, (BYTE)((255 * m_nPercentAlpha) / 100), LWA_ALPHA);


		//	MoveWindow(100, 100, 100, 100);
		CenterWindow(m_hWnd);	

		//Calculate the rect of "CLOSE" button.
		RECT rc;
		GetWindowRect(&rc);
		ScreenToClient(&rc);

		m_rcClose.left = rc.right - m_nRightMargin;
		m_rcClose.top = rc.top + m_nTopMargin;
		m_rcClose.right = m_rcClose.left + m_nCloseWidth;
		m_rcClose.bottom = m_rcClose.top + m_nCloseHeight;

		SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

		return CAxDialogImpl<CNXTLBS_Window>::OnInitDialog(uMsg, wParam, lParam, bHandled);  // Let the system set the focus
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
		int nOldMode = SetBkMode(hdc, TRANSPARENT);
		int textLen = (int)(wcslen(m_szText));	//	suppress warning in windows 64
		COLORREF hOldClr = SetTextColor(hdc, m_clrClose);
		HFONT hOldFont = (HFONT)SelectObject(hdc, m_hCloseFont);
		TextOut(hdc, rc.right - m_nRightMargin, rc.top + m_nTopMargin, m_szText, textLen);
		SelectObject(hdc, hOldFont);
		SetTextColor(hdc, hOldClr);
		SetBkMode(hdc, nOldMode);
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// TODO: Add your message handler code here and/or call default
		PAINTSTRUCT ps; 
		HDC hdc; 
		hdc = BeginPaint(&ps); 
		
		DrawBk(hdc);
		
		//Draw text
		wchar_t szTitle[200] = {0};
		LoadStringW(g_hInstance, IDS_LOGTITILE, szTitle, 200);

		HFONT hOldFont = (HFONT)SelectObject(hdc, m_hCloseFont);
		int nOldMode = SetBkMode(hdc, TRANSPARENT);
		COLORREF oldClr = SetTextColor(hdc, m_clrClose);

		TextOutW(hdc, 40, 15, szTitle, (int)(wcslen(szTitle)) );

		SelectObject(hdc, m_hFont);

		wchar_t szMsg[1024] = {0};
		LoadStringW(g_hInstance, IDS_LOGPROMPT, szMsg, 1024);
		RECT rc;
		GetWindowRect(&rc);
		ScreenToClient(&rc);

		rc.left += 20;
		rc.top += 45;
		rc.right -= 10;

		DrawTextExW(hdc, szMsg, (int)(wcslen(szMsg)), &rc, DT_WORDBREAK | DT_LEFT |DT_TOP, NULL);
		
		//Draw link
		wchar_t szLink[100] = {0};
		LoadStringW(g_hInstance, IDS_LOGLINK, szLink, 100);
		SetTextColor(hdc, RGB(0, 0, 255));
		SelectObject(hdc, m_hFontUnderline);

		TextOutW(hdc, m_rcLink.left, m_rcLink.top, szLink, (int)(wcslen(szLink)));

		SelectObject(hdc, hOldFont);
		SetBkMode(hdc, nOldMode);
		SetTextColor(hdc, oldClr);

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		POINT pt;

		pt.x=LOWORD(lParam);
		pt.y=HIWORD(lParam);

		if(PtInRect(&m_rcClose, pt) || PtInRect(&m_rcLink, pt))
		{
			SetCursor(LoadCursorW(NULL, IDC_HAND));
		}
		else
		{
			SetCursor(LoadCursorW(NULL, IDC_ARROW) );
		}
		return 0;
	}


	void SetZipLocation(const wstring& ZipLocation)
	{
		m_szZipLocation = ZipLocation;
	}


	void PickUp()
	{
		//	we show user the explorer folder contain zipped file
		wstring::size_type nPos = m_szZipLocation.rfind(L"\\");
		if(nPos != wstring::npos)
		{
			wstring folder = m_szZipLocation.substr(0, nPos);
			ShellExecute(NULL, L"explore", folder.c_str(), NULL, NULL, SW_SHOW);
		}

		::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
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
		else if (PtInRect(&m_rcLink, pt))
		{
			PickUp();
		}
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DestroyWindow();
		//EndDialog(1);	
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

	HFONT m_hFont;
	HFONT m_hFontUnderline;

	RECT m_rcLink;

	//	zip location
	wstring m_szZipLocation;
};