#include "stdafx.h"
#include <Commctrl.h>
#include "SCrollStatic.h"
#include <windowsx.h>


CSCrollStatic::CSCrollStatic() :m_hWnd(NULL)
{
	m_hyperSatic.SetYExtend(TRUE);
}


CSCrollStatic::~CSCrollStatic()
{
}

BOOL CSCrollStatic::Attach(HWND hWnd)
{
	if (m_hWnd==NULL)
	{
		//attach
		m_hWnd = hWnd;
#if defined(_WIN64)
		SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		m_wndProcOld = (WNDPROC)SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)ListControlWndProc);
#else
		SetWindowLongPtrW(m_hWnd, GWL_USERDATA, (LONG)(INT_PTR)this);
		m_wndProcOld = (WNDPROC)(INT_PTR)SetWindowLongPtrW(m_hWnd, GWL_WNDPROC, (LONG)(INT_PTR)ListControlWndProc);
#endif

		//create the static window
		RECT rcListControl = { 0 };
		::GetClientRect(m_hWnd, &rcListControl);
		m_hyperSatic.Create(m_hWnd, m_wstrText.c_str(), 0, 0, rcListControl.right, rcListControl.bottom);

		return TRUE;
	}
	return FALSE;
}

void CSCrollStatic::SetWindowText(const wchar_t* wszText)
{
	m_wstrText = wszText;
	m_hyperSatic.SetWindowTextW(wszText);

	//added new item for list box
	ListBox_ResetContent(m_hWnd);
	const int nTextLine = m_hyperSatic.GetTextLineCount();
	for (int iItem = 0; iItem < nTextLine; iItem++)
	{
		ListBox_AddString(m_hWnd, L"");
	}

	//move hypertext window
	PositionTextWindow();
}

void CSCrollStatic::PositionTextWindow()
{
	const int nItemCount = ListBox_GetCount(m_hWnd);

	RECT rcItemBegin = { 0 };
	ListBox_GetItemRect(m_hWnd, 0, &rcItemBegin);
	RECT rcItemEnd = { 0 };
	ListBox_GetItemRect(m_hWnd, nItemCount - 1, &rcItemEnd);

	RECT rcText = { rcItemBegin.left, rcItemBegin.top, rcItemEnd.right, rcItemEnd.bottom };


	RECT  rcWindow;
	// GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcWindow);

	if (rcWindow.right -  rcWindow.left <=  rcText.right)
	{
		int nOffset = rcText.right - (rcWindow.right-rcWindow.left);
		rcText.left -= nOffset;
		rcText.right -= nOffset;
		//DP((L"testx PosTextWindow, offset textRect:%d", nOffset));
	}
	//move
	m_hyperSatic.MoveWindow(rcText.left, rcText.top, rcText.right - rcText.left, rcText.bottom - rcText.top);

	::InvalidateRect(m_hWnd, NULL, TRUE);

}

LRESULT CALLBACK CSCrollStatic::ListControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN64)
	CSCrollStatic* pListView = (CSCrollStatic*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
#else
	CSCrollStatic* pListView = (CSCrollStatic*)(INT_PTR)GetWindowLongPtrW(hwnd, GWL_USERDATA);
#endif
	if (NULL == pListView)
	{
		return FALSE;
	}
	else
	{
		return pListView->ProcessWndMessage(hwnd, message, wParam, lParam);
	}
}

LRESULT CSCrollStatic::ProcessWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DRAWITEM:
		return OnOwneDraw((DRAWITEMSTRUCT*)lParam);
		break;


	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_MOUSEWHEEL:
	case WM_VSCROLL:
	case WM_HSCROLL:
		if (1)
		{
			long lRet = CallWindowProcW(m_wndProcOld, hWnd, message, wParam, lParam);
			PositionTextWindow();
			return lRet;
		}
		break;

	default:
		return m_wndProcOld(hWnd, message, wParam, lParam);
		break;

	}
}

LRESULT CSCrollStatic::OnOwneDraw(DRAWITEMSTRUCT* pDrawItemStruct)
{
	if (pDrawItemStruct->hwndItem == m_hyperSatic.GetHWND())
	{
		m_hyperSatic.OnDrawItem(pDrawItemStruct);
	}

	return TRUE;
}

LRESULT CSCrollStatic::OnMeasureItem(MEASUREITEMSTRUCT* pMeasureItem)
{
	//pMeasureItem->itemHeight =15;
	pMeasureItem->itemHeight = m_hyperSatic.GetLineHeight();
	int ContentHeight = m_hyperSatic.GetLineHeight() * m_hyperSatic.GetTextLineCount();
	RECT rcWnd;
	//::GetWindowRect(m_hWnd, &rcWnd);
	::GetClientRect(m_hWnd, &rcWnd);
	int WindowHeight = rcWnd.bottom - rcWnd.top;	
	if (ContentHeight>WindowHeight)
	{
		int nWidth = (rcWnd.right - rcWnd.left) + 15;
		::SendMessage(m_hWnd, LB_SETHORIZONTALEXTENT, (WPARAM)(nWidth), 0);
	} 
	return TRUE;
}


