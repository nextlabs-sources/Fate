#include "stdafx.h"
#include "NL_TabCtrl.h"

CNL_TabCtrl::CNL_TabCtrl()
{
	m_clrBk = RGB(255, 255, 255);
	m_clrText = RGB(0, 0, 0);

	m_hDefaultFont = CreateFontW(-14,
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
									L"Segoe UI"
									);

	m_hFont = m_hDefaultFont;

}

CNL_TabCtrl::~CNL_TabCtrl()
{
	if(m_hDefaultFont)
	{
		DeleteObject(m_hDefaultFont);
		m_hDefaultFont = NULL;
	}
}

LRESULT CNL_TabCtrl::OnDrawItem(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

	if (uMsg != OCM_DRAWITEM)
	{
		return S_FALSE;
	}

	if (lpDrawItem->CtlType == ODT_TAB)
	{
		if (lpDrawItem->itemAction & ODA_DRAWENTIRE)
		{
			HBRUSH hbr = NULL;
			if((UINT)GetCurSel() == lpDrawItem->itemID)
			{
				hbr = CreateSolidBrush(m_clrBk);
			}
			else
			{
				hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			}
			//draw bk
			RECT rc;
			rc.left = lpDrawItem->rcItem.left;
			rc.top = lpDrawItem->rcItem.top;
			rc.right = lpDrawItem->rcItem.right;
			rc.bottom = lpDrawItem->rcItem.bottom;
			FillRect(lpDrawItem->hDC, &rc, hbr);
			DeleteObject (hbr);

			//Draw text
			SetBkMode(lpDrawItem->hDC, TRANSPARENT);
			COLORREF oldColor = SetTextColor(lpDrawItem->hDC, m_clrText);
				
			TCITEM tci;
			memset(&tci, 0, sizeof(tci));
			wchar_t buf[256] = {0};
			tci.mask = TCIF_TEXT;
			tci.pszText = buf;
			tci.cchTextMax = sizeof(buf)/sizeof(wchar_t) - 1;
			TabCtrl_GetItem(m_hWnd, lpDrawItem->itemID, &tci);

			HFONT hOld = (HFONT)SelectObject(lpDrawItem->hDC, m_hFont);
			
			DrawTextW(lpDrawItem->hDC, buf, (int)wcslen(buf), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE );
			SetTextColor(lpDrawItem->hDC, oldColor);
			SelectObject(lpDrawItem->hDC, hOld);

		}
	}
	return 0;
}

LRESULT CNL_TabCtrl::OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}