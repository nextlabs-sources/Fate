#include "stdafx.h"
#include "HyperLinkStatic.h"
#include <Shellapi.h>

HFONT CHyperLinkStatic::m_hFontDefault = NULL;
HFONT CHyperLinkStatic::m_hFontLinkInit = NULL;
HFONT CHyperLinkStatic::m_hFontDefaultBold = NULL;
COLORREF CHyperLinkStatic::m_clrDefault = RGB(0, 0, 0);
COLORREF CHyperLinkStatic::m_clrLinkInit = RGB(0, 0, 255);
COLORREF CHyperLinkStatic::m_clrLinkClicked = RGB(0, 128, 128);
const TextFormatInfo CHyperLinkStatic::m_textFormatInfo[] = { { TEXT_FORMAT_BOLD, L"<b>", L"</b>" },
														      { TEXT_FORMAT_HYPERLINK, L"<a ", L"</a>" } };

CHyperLinkStatic::CHyperLinkStatic()
{
	m_hWnd = NULL;
	m_wndProcOld = NULL;
	ZeroMemory(&m_rcWindow, sizeof(RECT));
	m_bFirstWordBold= FALSE;
	m_nLineCount = 0;
	m_bYExtend = FALSE;
	m_hbrBackground = NULL;
	m_nLineHeight = 0;
}


CHyperLinkStatic::~CHyperLinkStatic()
{
	ClearContentInfo();
}

void CHyperLinkStatic::Init()
{
#if defined(_WIN64)
	LONG_PTR dwOldStyle = 0;
	dwOldStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	LONG_PTR dwNewStyle = dwOldStyle | SS_OWNERDRAW | SS_NOTIFY;
	SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwNewStyle);

	SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	m_wndProcOld = (WNDPROC)SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)StaticWndProc);
#else
	LONG dwOldStyle = 0;
	dwOldStyle = GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	LONG dwNewStyle = dwOldStyle | SS_OWNERDRAW | SS_NOTIFY;
	SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwNewStyle);

	SetWindowLongPtrW(m_hWnd, GWL_USERDATA, (LONG)(INT_PTR)this);
	m_wndProcOld = (WNDPROC)(INT_PTR)SetWindowLongPtrW(m_hWnd, GWL_WNDPROC, (LONG)(INT_PTR)StaticWndProc);
#endif

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

	//get m_hbrBackground
	HWND hWndParent = ::GetParent(m_hWnd);
	if (hWndParent)
	{
		m_hbrBackground = (HBRUSH)GetClassLongPtr(hWndParent, GCLP_HBRBACKGROUND);
	}
}

BOOL CHyperLinkStatic::Attach(HWND hWnd)
{
	if (NULL==m_hWnd)
	{
		m_hWnd = hWnd;

        Init();

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

        Init();
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
	
	//extract content
	ExtractContentInfo();

	//calculate content position
	CalculateContnetPosition();
}

void CHyperLinkStatic::CalculateContnetPosition()
{
	//get dc
	HDC hDCStaticWnd = GetDC(m_hWnd);

	const RECT rcTextBound = { 0, 0, m_rcWindow.right - 2, m_rcWindow.bottom - 2 };
	RECT rcText = rcTextBound;

	std::list<StaticContentInfo*>::iterator itContent = m_lstContentInfo.begin();
	int nContentIndex = 0;
	m_nLineCount = 1;
	while (itContent != m_lstContentInfo.end())
	{
		StaticContentInfo* pContentInfo = *itContent;
		nContentIndex++;

		//set font and text color
		pContentInfo->hFont = pContentInfo->bIsHyperLink ? m_hFontLinkInit : (pContentInfo->bBold) ? m_hFontDefaultBold : m_hFontDefault;
		pContentInfo->colorText = m_clrDefault;
		ChangeDCObj<HFONT> changeDCFont(hDCStaticWnd, pContentInfo->hFont);
		ChangeDCObj<COLORREF> changeDCColor(hDCStaticWnd, pContentInfo->colorText);

		//calculate the rect
		::DrawText(hDCStaticWnd, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);

		//if outside the right edge of the text bound, begin a new line
		if ((rcText.right > rcTextBound.right) || (pContentInfo->strText == L"\n"))
		{
			//the last line, we show "..." at the end
			if ((!GetYExtend()) && (rcText.bottom + (rcText.bottom - rcText.top) > rcTextBound.bottom))
			{
				const int nLenghtForEllipsis = 12;
				rcText.right = rcTextBound.right - nLenghtForEllipsis;
				pContentInfo->rcText = rcText;

				//add ellipsis
				RECT rcEllipsis = rcText;
				rcEllipsis.right += nLenghtForEllipsis;
				rcEllipsis.left = rcText.right;
				std::list<StaticContentInfo*>::iterator itEllipsis = itContent;
				itEllipsis++;
				StaticContentInfo* pEllipsis = new StaticContentInfo();
				pEllipsis->bIsHyperLink = FALSE;
				pEllipsis->strText = L"... ";
				pEllipsis->bIsClicked = FALSE;
				pEllipsis->bBold = FALSE;
				pEllipsis->rcText = rcEllipsis;
				m_lstContentInfo.insert(itEllipsis, pEllipsis);

				break;
			}
			else
			{
				//move to the next line
				::OffsetRect(&rcText, 0, rcText.bottom - rcText.top + 1);
				rcText.left = rcTextBound.left;
				rcText.right = rcTextBound.right;

                //set line height
				m_nLineHeight = (rcText.bottom-rcText.top+1)>m_nLineHeight ? (rcText.bottom-rcText.top+1) : m_nLineHeight;
				//calculate the rect again, we assure this time it doesn't exceed the bound
				::DrawText(hDCStaticWnd, pContentInfo->strText.c_str(), -1, &rcText, DT_SINGLELINE | DT_CALCRECT);
	

				//
				m_nLineCount++;
			}
		}

		pContentInfo->rcText = rcText;

		//offset the rect
		::OffsetRect(&rcText, rcText.right - rcText.left, 0);

		itContent++;
	}


	::ReleaseDC(m_hWnd, hDCStaticWnd);
	hDCStaticWnd = NULL;

}



LRESULT CHyperLinkStatic::OnDrawItem(DRAWITEMSTRUCT* pDrawStruct)
{
	if (m_hbrBackground)
	{
		::FillRect(pDrawStruct->hDC, &m_rcWindow, m_hbrBackground);
	}
	::SetBkMode(pDrawStruct->hDC, TRANSPARENT);
	std::list<StaticContentInfo*>::iterator itContent = m_lstContentInfo.begin();
	while (itContent != m_lstContentInfo.end())
	{
		StaticContentInfo* pContentInfo = *itContent;

		//set font and text color
		ChangeDCObj<HFONT> changeDCFont(pDrawStruct->hDC, pContentInfo->hFont);
		ChangeDCObj<COLORREF> changeDCColor(pDrawStruct->hDC, pContentInfo->bIsHyperLink ? (pContentInfo->bIsClicked ? m_clrLinkClicked : m_clrLinkInit) : pContentInfo->colorText);


		//draw text
		::DrawText(pDrawStruct->hDC, pContentInfo->strText.c_str(), -1, &pContentInfo->rcText, DT_SINGLELINE);

		itContent++;
	}

	return TRUE;
}

LRESULT CALLBACK CHyperLinkStatic::StaticWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN64)
	CHyperLinkStatic* PStatic = (CHyperLinkStatic*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
#else
    CHyperLinkStatic* PStatic = (CHyperLinkStatic*)(INT_PTR)GetWindowLongPtrW(hwnd, GWL_USERDATA);
#endif
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
	m_hFontDefault = CreateFontIndirectW(&lfDefault);

	//default bold
	LOGFONTW lfBold= {0};
	memcpy(&lfBold, &lfDefault, sizeof(LOGFONTW) );
	lfBold.lfWeight = FW_BOLD;
	m_hFontDefaultBold = CreateFontIndirectW(&lfBold);

	//link font
	LOGFONTW lfLink= {0};
	memcpy(&lfLink, &lfDefault, sizeof(LOGFONTW) );
	lfLink.lfUnderline = TRUE;
	m_hFontLinkInit = CreateFontIndirectW(&lfLink);
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

	size_t nFindPos = 0;
	BOOL bFindFmt = FALSE;
	do 
	{
		std::wstring wstrTextBeforeFmtText, wstrFmtText;
		TEXT_FORMAT textFmt;
		bFindFmt = FindFormatText(m_strText, nFindPos, wstrTextBeforeFmtText, wstrFmtText, textFmt);
		if (bFindFmt)
		{
			ExtractContentInfo(wstrTextBeforeFmtText, FALSE, L"");
			if (textFmt==TEXT_FORMAT_HYPERLINK)
			{
				ExtractHyperlinkContentInfo(wstrFmtText);
			}
			else if (textFmt==TEXT_FORMAT_BOLD)
			{
				ExtractBoldFontContentInfo(wstrFmtText);
			}
		}

	} while (bFindFmt);

    if (nFindPos<m_strText.length())
    {
		ExtractContentInfo(m_strText.substr(nFindPos), FALSE, L"");
    }
}

void CHyperLinkStatic::ExtractContentInfo(const std::wstring& strText, BOOL bIsHyperLink/* =FALSE */, const std::wstring& strUrl/* =L"" */, BOOL bBold/* =FALSE */)
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
				AddContentInfo(strText.substr(nFindPos, nPos - nFindPos), bIsHyperLink, strUrl, bBold);
				nFindPos = nPos;
			}

			//insert the space
			AddContentInfo(L" ", bIsHyperLink, strUrl, bBold);
			nFindPos += 1;
		}
		else
		{
			break;
		}

	} while ((nFindPos>0) && (nFindPos<strText.length()));

	if (nFindPos<strText.length())
	{
		AddContentInfo(strText.substr(nFindPos), bIsHyperLink, strUrl, bBold);
	}

}

void CHyperLinkStatic::ExtractBoldFontContentInfo(const std::wstring& strText)
{
	std::wstring wstrBoldFont = strText.substr(3, strText.length() - 7 );
	ExtractContentInfo(wstrBoldFont, FALSE, L"", TRUE);
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

void CHyperLinkStatic::AddContentInfo(const std::wstring& strText, BOOL bHyperLink, const std::wstring& strLinkUrl /* = L"" */, BOOL bBold /* = FALSE */)
{
	//action:we need to process the new line inner strText. e.g.("\nabc\ndefg\n")

    WCHAR chNewLine = L'\n';

	size_t nFindIndex = 0;
	size_t nPosNewLine = strText.find(chNewLine, nFindIndex);
	if(nPosNewLine != std::wstring::npos)
	{
		//add text before new line
		std::wstring wstrBeforeNewLineText = strText.substr(0, nPosNewLine);
		if(!wstrBeforeNewLineText.empty())
		{
			StaticContentInfo* pContentInfo = new StaticContentInfo();
			pContentInfo->bIsHyperLink = bHyperLink;
			pContentInfo->strText = wstrBeforeNewLineText;
			pContentInfo->strLinkUrl = strLinkUrl;
			pContentInfo->bIsClicked = FALSE;
			pContentInfo->bBold = bBold;
			m_lstContentInfo.push_back(pContentInfo);
		}


        //add the new line char
		{
			StaticContentInfo* pContentInfo = new StaticContentInfo();
			pContentInfo->bIsHyperLink = bHyperLink;
			pContentInfo->strText = chNewLine;
			pContentInfo->strLinkUrl = strLinkUrl;
			pContentInfo->bIsClicked = FALSE;
			pContentInfo->bBold = bBold;
			m_lstContentInfo.push_back(pContentInfo);
		}

		//add text after new line
		if(nPosNewLine<strText.length()-1)
		{
			std::wstring wstrAfterNewLineText = strText.substr(nPosNewLine+1);
			if(!wstrAfterNewLineText.empty())
			{
				AddContentInfo(wstrAfterNewLineText, bHyperLink, strLinkUrl, bBold);
			}
		}
	}
	else
	{
		StaticContentInfo* pContentInfo = new StaticContentInfo();
		pContentInfo->bIsHyperLink = bHyperLink;
		pContentInfo->strText = strText;
		pContentInfo->strLinkUrl = strLinkUrl;
		pContentInfo->bIsClicked = FALSE;
		pContentInfo->bBold = bBold;
		m_lstContentInfo.push_back(pContentInfo);
	}
}

void CHyperLinkStatic::OnMoseMove(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
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
	UNREFERENCED_PARAMETER(wParam);
	const int nXPos = LOWORD(lParam);
	const int nYPos = HIWORD(lParam);

     StaticContentInfo* pContent = GetContentInfoByMousePos(nXPos, nYPos);
	if (pContent && pContent->bIsHyperLink)
	{
		ShellExecuteW(0, L"open", pContent->strLinkUrl.c_str(), 0, 0, SW_SHOW);
		
		SetNeighborContentToClicked(pContent);

		if (m_hbrBackground==NULL)
		{
			HWND hWndParent = GetParent(m_hWnd);
			::InvalidateRect(hWndParent, NULL, TRUE);
		}

		::InvalidateRect(m_hWnd, &(m_rcWindow), TRUE);
	}

	//send this message to parent window
	::SendMessage(GetParent(m_hWnd), WM_LBUTTONDOWN, wParam, lParam);
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

void CHyperLinkStatic::SetFirstWordBoldFont(BOOL bBold)
{
	m_bFirstWordBold = bBold;
	if(m_hWnd)
	{
		::InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

BOOL CHyperLinkStatic::FindFormatText(const std::wstring& refWstrText, size_t& nFindPos, std::wstring& refWstrTextBeforeFmtText,
	                                  std::wstring& refFmtText, TEXT_FORMAT& refTextFmt)
{
	refTextFmt = TEXT_FORMAT_NONE;
	//find the nearest format
	BOOL bFmtFind = FALSE;
	size_t nFmtResultBegin = std::wstring::npos;
	size_t nFmtResultEnd = std::wstring::npos;
	TEXT_FORMAT textFmtResult;
	for (int nFmtIndex = 0; nFmtIndex < sizeof(m_textFormatInfo) / sizeof(m_textFormatInfo[0]); nFmtIndex++)
	{
	    const TextFormatInfo* pFmtInfo  = &(m_textFormatInfo[nFmtIndex]);
        
		size_t nFmtBegin = refWstrText.find(pFmtInfo->wstrFormatBegin.c_str(), nFindPos);
		if (nFmtBegin != std::wstring::npos)
		{
			size_t nFmtEnd = refWstrText.find(pFmtInfo->wstrFormatEnd.c_str(), nFmtBegin + pFmtInfo->wstrFormatBegin.length());
			if (nFmtEnd != std::wstring::npos)
			{
				if ((!bFmtFind) || (nFmtBegin < nFmtResultBegin))
				{
					nFmtResultBegin = nFmtBegin;
					nFmtResultEnd = nFmtEnd + pFmtInfo->wstrFormatEnd.length();
					textFmtResult = pFmtInfo->fmtType;
					bFmtFind = TRUE;
				}
			}
		}

	}

	//get the text, and output data
	if (bFmtFind)
	{
		refWstrTextBeforeFmtText = refWstrText.substr(nFindPos, nFmtResultBegin - nFindPos);
		refFmtText = refWstrText.substr(nFmtResultBegin, nFmtResultEnd - nFmtResultBegin);
		refTextFmt = textFmtResult;
		nFindPos = nFmtResultEnd;
	}


	return bFmtFind;
}