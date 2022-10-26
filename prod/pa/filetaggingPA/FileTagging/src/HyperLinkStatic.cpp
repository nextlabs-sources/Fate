#include "stdafx.h"
#include "HyperLinkStatic.h"
#include <Shellapi.h>

HFONT CHyperLinkStatic::m_hFontDefault = NULL;
HFONT CHyperLinkStatic::m_hFontLinkInit = NULL;
COLORREF CHyperLinkStatic::m_clrDefault = RGB(0, 0, 0);
COLORREF CHyperLinkStatic::m_clrLinkInit = RGB(0, 0, 255);
COLORREF CHyperLinkStatic::m_clrLinkClicked = RGB(0, 128, 128);

CHyperLinkStatic::CHyperLinkStatic()
{
	m_hWnd = NULL;
	m_wndProcOld = NULL;
	ZeroMemory(&m_rcWindow, sizeof(RECT));
}


CHyperLinkStatic::~CHyperLinkStatic()
{
	ClearContentInfo();
}

BOOL CHyperLinkStatic::Attach(HWND hWnd)
{
	if (NULL==m_hWnd)
	{
		m_hWnd = hWnd;

		//modify window style to make it ownerdraw and receive notify
		LONG_PTR dwOldStyle = 0;
		dwOldStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
		LONG_PTR dwNewStyle = dwOldStyle | SS_OWNERDRAW | SS_NOTIFY;
		SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwNewStyle);

		SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		m_wndProcOld = (WNDPROC)SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)StaticWndProc);

		//
		RECT rcWnd;
		::GetWindowRect(m_hWnd, &rcWnd);
		m_rcWindow.left = m_rcWindow.top = 0;
		m_rcWindow.right = rcWnd.right - rcWnd.left;
		m_rcWindow.bottom = rcWnd.bottom - rcWnd.top;

		//Create font
		if (m_hFontDefault == NULL)
		{
			CreateFont(GetParent(m_hWnd));
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CHyperLinkStatic::Create(HWND hParent, const WCHAR* wszText, int nX, int nY, int nWidth, int nHeight)
{
	if (NULL==m_hWnd)
	{
		SetWindowText(wszText);

		const DWORD dwStyle = WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY;
		m_hWnd = ::CreateWindowW(L"STATIC", NULL, dwStyle, nX, nY, nWidth, nHeight, hParent, NULL, NULL, NULL);

		SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		m_wndProcOld = (WNDPROC)SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)StaticWndProc);

		MoveWindow(nX, nY, nWidth, nHeight);

		//Create font
		if (m_hFontDefault==NULL)
		{
			CreateFont(hParent);
		}

		//set default font
		::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hFontDefault, MAKELPARAM(1, 0));
	}
	
	return m_hWnd != NULL;
}

void CHyperLinkStatic::MoveWindow(int nX, int nY, int nWidth, int nHeight)
{
	m_rcWindow.left = m_rcWindow.top = 0;
	m_rcWindow.right = nWidth;
	m_rcWindow.bottom = nHeight;

	::MoveWindow(m_hWnd, nX, nY, nWidth, nHeight, TRUE);
}

void CHyperLinkStatic::SetWindowText(const WCHAR* wszText)
{
	m_strText = wszText;
	ExtractContentInfo();
}

LRESULT CHyperLinkStatic::OnDrawItem(DRAWITEMSTRUCT* pDrawStruct)
{
	const RECT rcTextBound = { 0, 0, m_rcWindow.right - 2, m_rcWindow.bottom - 2 };

	RECT rcText = rcTextBound;

	std::list<StaticContentInfo*>::iterator itContent = m_lstContentInfo.begin();
	while (itContent != m_lstContentInfo.end())
	{
		StaticContentInfo* pContentInfo = *itContent;

		//set font and text color
		ChangeDCObj<HFONT> changeDCFont(pDrawStruct->hDC, pContentInfo->bIsHyperLink ? m_hFontLinkInit : m_hFontDefault);
		ChangeDCObj<COLORREF> changeDCColor(pDrawStruct->hDC, pContentInfo->bIsHyperLink ? (pContentInfo->bIsClicked ? m_clrLinkClicked : m_clrLinkInit) : m_clrDefault);

		//calculate the rect
		::DrawText(pDrawStruct->hDC, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);

		//if outside the right edge of the text bound, begin a new line
		if (rcText.right>rcTextBound.right)
		{
			//the last line, we show "..." at the end
			if (rcText.bottom + (rcText.bottom - rcText.top) > rcTextBound.bottom)
			{
				::DrawText(pDrawStruct->hDC, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE);
				RECT rcEllipsis = rcText;
				rcEllipsis.left = rcTextBound.right - 12;
				const WCHAR szEllipsis[] = L"... ";
				::DrawText(pDrawStruct->hDC, szEllipsis, -1, &rcEllipsis, DT_SINGLELINE);
				break;
			}
			else
			{
				//move to the next line
				::OffsetRect(&rcText, 0, rcText.bottom - rcText.top + 1);
				rcText.left = rcTextBound.left;
				rcText.right = rcTextBound.right;

				//calculate the rect again, we assure this time it doesn't exceed the bound
				::DrawText(pDrawStruct->hDC, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);
			}	
		}

		//draw it
		::DrawText(pDrawStruct->hDC, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE);
		pContentInfo->rcText = rcText;

		//offset the rect
		::OffsetRect(&rcText, rcText.right - rcText.left, 0);

		itContent++;
	}

	return TRUE;
}

LRESULT CALLBACK CHyperLinkStatic::StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CHyperLinkStatic* PStatic = (CHyperLinkStatic*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (NULL == PStatic)
	{
		return FALSE;
	}
	else
	{
		return PStatic->ProcessWndMessage(message, wParam, lParam);
	}
}

LRESULT CHyperLinkStatic::ProcessWndMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
		OnLButtonDown(wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		OnMoseMove(wParam, lParam);
		break;

	}
	return CallWindowProcW(m_wndProcOld, GetHWND(), message, wParam, lParam);
}


void CHyperLinkStatic::CreateFont(HWND hParent)
{
	//default font
	m_hFontDefault = (HFONT)::SendMessage(hParent, WM_GETFONT, 0, 0);

	LOGFONTW lfDefault;
	GetObjectW(m_hFontDefault, sizeof(lfDefault), &lfDefault);

	//link font
	lfDefault.lfUnderline = TRUE;
	m_hFontLinkInit = CreateFontIndirectW(&lfDefault);
}

void CHyperLinkStatic::ClearContentInfo()
{
	std::list<StaticContentInfo*>::iterator itContentInfo = m_lstContentInfo.begin();
	while (itContentInfo != m_lstContentInfo.end())
	{
		delete *itContentInfo;
		*itContentInfo = NULL;
		itContentInfo++;
	}

	m_lstContentInfo.clear();
}

void CHyperLinkStatic::ExtractContentInfo()
{
	ClearContentInfo();

	const WCHAR* wszHyperlinkBegin = L"<a ";
	const WCHAR* wszHyperLinkEnd = L"</a>";

	size_t nHyperBeginPos = std::wstring::npos;
	size_t nHyperEndPos = std::wstring::npos;
	size_t nFindPos = 0;
	do
	{
	    nHyperBeginPos = m_strText.find(wszHyperlinkBegin, nFindPos);
		if (nHyperBeginPos != std::wstring::npos)
		{
		    nHyperEndPos = m_strText.find(wszHyperLinkEnd, nHyperBeginPos);
			if (nHyperEndPos != std::wstring::npos )
			{
				ExtractContentInfo(m_strText.substr(nFindPos, nHyperBeginPos - nFindPos), FALSE, L"");
				ExtractHyperlinkContentInfo(m_strText.substr(nHyperBeginPos, nHyperEndPos + wcslen(wszHyperLinkEnd) - nHyperBeginPos));
				nFindPos = nHyperEndPos + wcslen(wszHyperLinkEnd);
			}
		}
		
	} while ((nHyperBeginPos!=std::wstring::npos) && 
		     (nHyperEndPos!=std::wstring::npos) );

	ExtractContentInfo(m_strText.substr(nFindPos), FALSE, L"");
}

void CHyperLinkStatic::ExtractContentInfo(const std::wstring& strText, BOOL bIsHyperLink/* =FALSE */, const std::wstring& strUrl/* =L"" */)
{
	size_t nPos = 0;
	size_t nFindPos = 0;
	const WCHAR wChSpace = L' ';
	do 
	{
		nPos = strText.find(wChSpace, nFindPos);
		if (nPos != std::wstring::npos)
		{
			if (nPos>nFindPos)
			{
				AddContentInfo(strText.substr(nFindPos, nPos - nFindPos), bIsHyperLink, strUrl);
				nFindPos = nPos;
			}

			//insert the space
			AddContentInfo(L" ", bIsHyperLink, strUrl);
			nFindPos += 1;
		}
		else
		{
			break;
		}

	} while ((nFindPos>0) && (nFindPos<strText.length()));

	if (nFindPos<strText.length())
	{
		AddContentInfo(strText.substr(nFindPos), bIsHyperLink, strUrl);
	}

}

void CHyperLinkStatic::ExtractHyperlinkContentInfo(const std::wstring& strText)
{
	std::wstring wstrLinkUrl = L"";
	std::wstring wstrShowText = L"";

	//find link url
	{
		const WCHAR* szHrefBegin = L"href=\"";
		const WCHAR* szHrefEnd = L"\"";
		size_t nPosHrefBegin = strText.find(szHrefBegin);
		if (nPosHrefBegin != std::wstring::npos)
		{
			size_t nPosHrefEnd = strText.find(szHrefEnd, nPosHrefBegin + wcslen(szHrefBegin));
			if (nPosHrefEnd != std::wstring::npos)
			{
				wstrLinkUrl = strText.substr(nPosHrefBegin + wcslen(szHrefBegin), nPosHrefEnd - nPosHrefBegin - wcslen(szHrefBegin));
			}
		}
	}


	//find show text
	{
		const WCHAR* szShowTextBegin = L">";
		const WCHAR* szShowTextEnd = L"</a>";
		size_t nPosShowTextBegin = strText.find(szShowTextBegin);
		if (nPosShowTextBegin != std::wstring::npos)
		{
			size_t nPosShowTextEnd = strText.find(szShowTextEnd, nPosShowTextBegin);
			if (nPosShowTextEnd != std::wstring::npos)
			{
				wstrShowText = strText.substr(nPosShowTextBegin + wcslen(szShowTextBegin), nPosShowTextEnd - nPosShowTextBegin - wcslen(szShowTextBegin));
			}
		}
	}

	//
	if ((!wstrShowText.empty()) && (!wstrLinkUrl.empty()))
	{
		ExtractContentInfo(wstrShowText, TRUE, wstrLinkUrl);
	}
	else
	{
		//format is error, we treat ti as normal text
		ExtractContentInfo(strText, FALSE, L"");
	}	
}

void CHyperLinkStatic::AddContentInfo(const std::wstring& strText, BOOL bHyperLink, const std::wstring& strLinkUrl/* =L"" */)
{
	StaticContentInfo* pContentInfo = new StaticContentInfo();
	pContentInfo->bIsHyperLink = bHyperLink;
	pContentInfo->strText = strText;
	pContentInfo->strLinkUrl = strLinkUrl;
	pContentInfo->bIsClicked = FALSE;

	m_lstContentInfo.push_back(pContentInfo);
}

void CHyperLinkStatic::OnMoseMove(WPARAM wParam, LPARAM lParam)
{
	const int nXPos = LOWORD(lParam);
	const int nYPos = HIWORD(lParam);

	const StaticContentInfo* pContent = GetContentInfoByMousePos(nXPos, nYPos);
	if (pContent && pContent->bIsHyperLink)
	{
		SetCursor(LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_HAND)));
	}
}

void CHyperLinkStatic::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	const int nXPos = LOWORD(lParam);
	const int nYPos = HIWORD(lParam);

     StaticContentInfo* pContent = GetContentInfoByMousePos(nXPos, nYPos);
	if (pContent && pContent->bIsHyperLink)
	{
		ShellExecuteW(0, L"open", pContent->strLinkUrl.c_str(), 0, 0, SW_SHOW);
		
		SetNeighborContentToClicked(pContent);
		::InvalidateRect(m_hWnd, &(m_rcWindow), TRUE);
	}
}

StaticContentInfo* CHyperLinkStatic::GetContentInfoByMousePos(int nX, int nY)
{
	POINT ptMouse;
	ptMouse.x = nX;
	ptMouse.y = nY;

	std::list<StaticContentInfo*>::iterator itContent = m_lstContentInfo.begin();
	while (itContent != m_lstContentInfo.end())
	{
	    StaticContentInfo* pContentInfo = *itContent;
		if (PtInRect(&(pContentInfo->rcText), ptMouse))
		{
			return pContentInfo;
		}
		itContent++;
	}

	return NULL;

}

void CHyperLinkStatic::SetNeighborContentToClicked(StaticContentInfo* pContentInfo)
{
    //find the node
	std::list<StaticContentInfo*>::iterator itContentFind = m_lstContentInfo.end();
	std::list<StaticContentInfo*>::iterator itContent = m_lstContentInfo.begin();
	while (itContent != m_lstContentInfo.end())
	{
	    if (*itContent == pContentInfo)
	    {
			itContentFind = itContent;
			break;
	    }
		itContent++;
	}

	if (itContentFind == m_lstContentInfo.end())
	{
		return;
	}

	//set itself to true
	(*itContentFind)->bIsClicked = TRUE;

	//find Neighbor
	std::list<StaticContentInfo*>::iterator itNext = itContentFind;
	while (itNext != m_lstContentInfo.end())
	{
		if ((*itNext)->bIsHyperLink)
		{
			(*itNext)->bIsClicked = TRUE;
		}
		else
		{
			break;
		}

		itNext++;
	}

	std::list<StaticContentInfo*>::iterator itPrev = itContentFind;
	while (itPrev != m_lstContentInfo.begin())
	{
		if ((*itPrev)->bIsHyperLink)
		{
			(*itPrev)->bIsClicked = TRUE;
		}
		else
		{
			break;
		}
		
		itPrev--;
	}
}