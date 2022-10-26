#include "StdAfx.h"
#include "NTXLBS_Bubble.h"
#include "Actions.h"
#include <list>

#define WM_SHOWNOTIFICATIONTAB	WM_USER + 5 //Send a message to "manage" to show the notification tab.

extern std::list<BUBBLE> g_listBubbles;
extern CRITICAL_SECTION g_CriSection;

typedef struct MonthMap 
{
	wstring strInput;
	wstring strNumber;
}MONTHMAP;

static void CloseAllBubbles()
{
	std::list<BUBBLE>::iterator iter;
	EnterCriticalSection(&g_CriSection);
	for(iter = g_listBubbles.begin(); iter != g_listBubbles.end();)
	{//Check all the bubbles.
		BUBBLE bubble = (*iter);

		bubble.pBubble->CloseBubble();
	
		if(bubble.pBubble && bubble.pBubble->IsClosed())
		{
			delete bubble.pBubble;
			iter = g_listBubbles.erase(iter);
			//iter = g_listBubbles.begin();	
			continue;
		} 
		iter++;
	}
	LeaveCriticalSection(&g_CriSection);
}

const static MONTHMAP g_szMonth[] = {
	wstring(L"Jan"), wstring(L"1"), \
	wstring(L"Feb"), wstring(L"2"), \
	wstring(L"Mar"), wstring(L"3"), \
	wstring(L"Apr"), wstring(L"4"), \
	wstring(L"May"), wstring(L"5"), \
	wstring(L"Jun"), wstring(L"6"), \
	wstring(L"Jul"), wstring(L"7"), \
	wstring(L"Aug"), wstring(L"8"), \
	wstring(L"Sep"), wstring(L"9"), \
	wstring(L"Oct"), wstring(L"10"), \
	wstring(L"Nov"), wstring(L"11"), \
	wstring(L"Dec"), wstring(L"12"), \
};

CNTXLBS_Bubble::HourSwitch CNTXLBS_Bubble::s_arrayHourSwitch[] = {
	wstring(L"00"), wstring(L"12"), wstring(L"am"), \
	wstring(L"01"), wstring(L"1"), wstring(L"am"),	\
	wstring(L"02"), wstring(L"2"), wstring(L"am"), \
	wstring(L"03"), wstring(L"3"), wstring(L"am"), \
	wstring(L"04"), wstring(L"4"), wstring(L"am"), \
	wstring(L"05"), wstring(L"5"), wstring(L"am"),	\
	wstring(L"06"), wstring(L"6"), wstring(L"am"), \
	wstring(L"07"), wstring(L"7"), wstring(L"am"),	\
	wstring(L"08"), wstring(L"8"), wstring(L"am"), \
	wstring(L"09"), wstring(L"9"), wstring(L"am"),	\
	wstring(L"10"), wstring(L"10"), wstring(L"am"), \
	wstring(L"11"), wstring(L"11"), wstring(L"am"),	\
	wstring(L"12"), wstring(L"12"), wstring(L"pm"), \
	wstring(L"13"), wstring(L"1"), wstring(L"pm"),	\
	wstring(L"14"), wstring(L"2"), wstring(L"pm"), \
	wstring(L"15"), wstring(L"3"), wstring(L"pm"),	\
	wstring(L"16"), wstring(L"4"), wstring(L"pm"), \
	wstring(L"17"), wstring(L"5"), wstring(L"pm"),	\
	wstring(L"18"), wstring(L"6"), wstring(L"pm"), \
	wstring(L"19"), wstring(L"7"), wstring(L"pm"),	\
	wstring(L"20"), wstring(L"8"), wstring(L"pm"), \
	wstring(L"21"), wstring(L"9"), wstring(L"pm"),	\
	wstring(L"22"), wstring(L"10"), wstring(L"pm"), \
	wstring(L"23"), wstring(L"11"), wstring(L"pm"),	\
};

#define BUBBLE_TITLE			L"Enterprise Data Protection - Control Panel"
#define TEXT_LEFT				50
#define TEXT_RIGHT				10
#define TEXT_TOP				28
#define	TEXT_BOTTOM_MARGIN		5
#define TITLE_LEFT				50
#define TITLE_TOP				10
#define LOGO_X					10
#define LOGO_Y					25

BOOL CNTXLBS_Bubble::FormatTimeStr(wstring& strTime, wstring& strFormated)
{
	//	get month and date
	wstring::size_type npos = strTime.find(L" ");
	wstring::size_type npos_2 = strTime.find(L" ", npos + 1);
	wstring::size_type npos_3 = strTime.find(L" ", npos_2 + 1);

	if(npos == wstring::npos || npos_2 == wstring::npos || npos_3 == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring month = strTime.substr(npos + 1, npos_2 - npos - 1);
	wstring date = strTime.substr(npos_2 + 1, npos_3 - npos_2 - 1);

	//	check if date is "04" -> "4"
// 	if (date[0] == L'0')
// 	{
// 		date = date.substr(1, 1);
// 	}

	wstring monthDate;
	wstring numMonth(L"");

	for(int m = 0; m < _countof(g_szMonth); m++)
	{
		if(_wcsicmp(g_szMonth[m].strInput.c_str(), month.c_str()) == 0)
		{
			numMonth = g_szMonth[m].strNumber;
			break;
		}
	}
	if(numMonth.length() > 0)
	{
		monthDate = numMonth + L"/" + date;
	}
	else
	{
		monthDate = month + wstring(L" ") + date;
	}

	//	get year
	npos = strTime.rfind(L" ");
	if(npos == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring year = strTime.substr(npos + 1, strTime.length() - npos - 1);

	if(year.length() == 4)
	{
		year = year.substr(2, 2);
	}
	//	get hour/minutes/seconds, it is 16:20:25
	npos = strTime.find(L" ", npos_3 + 1);
	if(npos == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring hourMinSeconds = strTime.substr(npos_3 + 1, npos - npos_3 - 1);
	//	change it to 4:20PM
	//	get hour
	npos = hourMinSeconds.find(L":");
	npos_2 = hourMinSeconds.find(L":", npos + 1);

	if(npos == wstring::npos || npos_2 == wstring::npos)
	{
		strFormated = strTime;
		return FALSE;
	}

	wstring hour = hourMinSeconds.substr(0, npos);

	g_log.Log(CELOG_DEBUG, L"24 hour is %s\n", hour.c_str());

	//	switch hour from 24 hour to 12 hour
	//	and determine if it is AM or PM
	wstring strAMPM;
	for (DWORD i = 0; i < sizeof(s_arrayHourSwitch)/sizeof(HourSwitch); i++)
	{
		if ( s_arrayHourSwitch[i].str24Hour == hour )
		{
			hour = s_arrayHourSwitch[i].str12tHour;
			strAMPM = s_arrayHourSwitch[i].strAMPM;
			break;
		}
	}
	g_log.Log(CELOG_DEBUG, L"12 hour is %s, %s\n", hour.c_str(), strAMPM.c_str());

	//	get minutes and seconds
	wstring minSeconds = hourMinSeconds.substr(npos + 1, hourMinSeconds.length() - npos - 1);

	//	target format is "May 27, 2010  4:20PM"
	strFormated = monthDate + wstring(L"/") + year + wstring(L"  ") + hour + wstring(L":") + minSeconds + wstring(L" ") + strAMPM;

	return TRUE;
}

typedef struct _IPCREQUEST
{
	ULONG ulSize;
	WCHAR methodName [64];
	WCHAR params [7][256];
}IPCREQUEST, *PIPCREQUEST;

CNTXLBS_Bubble::CNTXLBS_Bubble(void)
{
	m_nPercentAlpha = 80;

	m_hFont = CreateFontW(-12,
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

	m_hMsgFont = CreateFontW(-12,
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

	m_pInfo = NULL;
	
	m_clrClose = RGB(130, 130, 130);
	m_clrBorder = RGB(118, 118, 118);
	m_clrShadow = RGB(160, 160, 160);

	//Close button position
	m_nRightMargin = 20;
	m_nTopMargin = TITLE_TOP + 3;

	m_nCloseWidth = 8;//"Close" button width
	m_nCloseHeight = 8;//"Close" button height

	//The width and height of bubble.
	m_nWidth = 200;
	m_nHeight = 150;

	m_bClosed = false;
    m_bFirst = false;

	m_nPercentAlpha = 60;

	m_nBorderLeftMargin = 0;
	m_nBorderRightMargin = 0;
	m_nBorderTopMargin = 0;
	m_nBorderBottomMargin = 0;

	m_hBubbleIcon = NULL;

	m_nRoundWidth = 0;
	m_nRoundHeight = 0;

	m_nRoundWidthShadow = 0;
	m_nRoundHeightShadow = 0;

	SetVGradientBk(RGB(240, 240, 240), RGB(240, 240, 240));

	m_hCallingWnd = NULL;
    m_pHyperLink = NULL;
}

CNTXLBS_Bubble::~CNTXLBS_Bubble(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = 0;
	}

	if(m_hMsgFont)
	{
		DeleteObject(m_hMsgFont);
		m_hMsgFont = 0;
	}

}

LRESULT CNTXLBS_Bubble::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CNTXLBS_Bubble>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	
//	HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ALLOWICON));
	
	m_hBubbleIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	

	m_hCloseIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_CLOSEICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);


	RECT rc;
	GetWindowRect(&rc);
	rc.right = rc.left + m_nWidth;
	rc.bottom = rc.top + m_nHeight;

	SetWindowPos( NULL, 0, 0, m_nWidth, m_nHeight, SWP_NOZORDER | SWP_NOMOVE );

/*	HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 30,30);

	SetWindowRgn(rgn, TRUE);
	DeleteObject(rgn);*/
		

	//Compute the size of bubble which was needed to show all message.
	HDC dc = GetDC();
	HFONT hOldFont = (HFONT)SelectObject(dc, m_hMsgFont);

	RECT rcText;
	rcText.left = 0;
	rcText.right = m_nWidth - TEXT_LEFT - TEXT_RIGHT - m_nBorderRightMargin;
	rcText.top = 0;

    std::wstring newstrMsg = m_BubbleInfo.strMsg;
    do 
    {
        std::wstring::size_type iBeginLocation = newstrMsg.find(L"<a");

        if(iBeginLocation == std::wstring::npos)
        {
            break;
        }

        std::wstring::size_type iEndLocation = newstrMsg.find(L">", iBeginLocation);
        if(iEndLocation == std::wstring::npos)
        {
            break;
        }

        std::wstring::size_type iOppositedLocation = newstrMsg.find(L"</a>", iEndLocation);
        if(iOppositedLocation == std::wstring::npos)
        {
            break;
        }

        newstrMsg.erase(iOppositedLocation, 4);
        newstrMsg.erase(iBeginLocation, iEndLocation - iBeginLocation + 1);
    } while (TRUE);

    DrawTextExW(dc, (LPWSTR)newstrMsg.c_str(), (int)newstrMsg.length(), &rcText, DT_CALCRECT | DT_WORDBREAK, NULL);

	SelectObject(dc, hOldFont);
	ReleaseDC(dc);

	if( TEXT_LEFT + TEXT_RIGHT + rcText.right + m_nBorderRightMargin > m_nWidth)
	{//It means there is a word which extends the "width" of bubble, enlarge the width.
		m_nWidth = TEXT_LEFT + TEXT_RIGHT + rcText.right + m_nBorderRightMargin;
	}

	if( TEXT_TOP + rcText.bottom + TEXT_BOTTOM_MARGIN + m_nBorderBottomMargin > m_nHeight)
	{//We need to enlarge the bubble if there are too many characters
		m_nHeight = TEXT_TOP + rcText.bottom + TEXT_BOTTOM_MARGIN + m_nBorderBottomMargin;
	}

// 	int nWidth = GetSystemMetrics(SM_CXFULLSCREEN);
// 	int nHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	RECT rcScreen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	int nWidth = rcScreen.right;
	int nHeight = rcScreen.bottom;

	SetWindowPos(HWND_TOPMOST, nWidth - m_nWidth - 20, nHeight - m_nHeight, m_nWidth, m_nHeight, 0);

	::SetWindowLongPtr ( m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtr ( m_hWnd, GWL_EXSTYLE ) | WS_EX_NOACTIVATE );

	//Calculate the rect of "CLOSE" button.
	GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_rcClose.left = rc.right - m_nRightMargin;
	m_rcClose.top = rc.top + m_nTopMargin;
	m_rcClose.right = m_rcClose.left + m_nCloseWidth;
	m_rcClose.bottom = m_rcClose.top + m_nCloseHeight;

//	::SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, ::GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// Make this window 60% alpha
//	::SetLayeredWindowAttributes(m_hWnd, 0, (BYTE)((255 * m_nPercentAlpha) / 100), LWA_ALPHA);
	//Make the background color as transparent
//	::SetLayeredWindowAttributes(m_hWnd, m_clrBk, 0, LWA_COLORKEY);

	//AnimateWindow
	AnimateWindow(m_hWnd, 500, AW_SLIDE | AW_VER_NEGATIVE);

 	m_Shadow.SetRound(m_nRoundWidthShadow, m_nRoundHeightShadow);
 	m_Shadow.Create(m_hWnd);	
 	m_Shadow.SetDarkness(180);
	

	bHandled = TRUE;
	return 1;
}

LRESULT CNTXLBS_Bubble::OnPrintClient(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDC hdc = (HDC)wParam;

	DrawBubble(hdc);

	::ReleaseDC(m_hWnd, hdc);

	return 0;
}

LRESULT CNTXLBS_Bubble::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	PAINTSTRUCT ps; 
	HDC hdc; 

	hdc = BeginPaint(&ps); 

	DrawBubble(hdc);	

	EndPaint(&ps);

	return 0;
}

void CNTXLBS_Bubble::DrawBubble(HDC hdc)
{
	DrawBk(hdc);

//Draw logo
	if(m_hBubbleIcon)
	{
		DrawIconEx(hdc, LOGO_X, LOGO_Y, m_hBubbleIcon, 32, 32, 0, 0, DI_NORMAL);
	}


	//Draw Close button
	if(m_hCloseIcon)
	{
		DrawIconEx(hdc, m_nWidth - m_nRightMargin, m_nTopMargin, m_hCloseIcon, 8, 8, 0, 0, DI_NORMAL);
	}

	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	COLORREF oldClr = SetTextColor(hdc, m_clrClose);
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);
	//Draw title
	SetTextColor(hdc, RGB(0, 51, 153));
	TextOut(hdc, TITLE_LEFT, TITLE_TOP, m_BubbleInfo.strTitle.c_str(), (int)m_BubbleInfo.strTitle.length());

	//Draw Message
	int nLeftMargin = TEXT_LEFT;
	int nTopMargin = TEXT_TOP;
	int nRCWidth = m_nWidth - nLeftMargin - TEXT_RIGHT - m_nBorderRightMargin - 40;
	int nRCHeight = m_nHeight - nTopMargin - m_nBorderBottomMargin - TEXT_BOTTOM_MARGIN;
	RECT rc = {nLeftMargin, nTopMargin, nLeftMargin + nRCWidth, nTopMargin + nRCHeight};
	
    if (!m_bFirst)
    {
        CreateWindowEx(0, WC_LINK, m_BubbleInfo.strMsg.c_str(), WS_VISIBLE | WS_CHILD, rc.left, rc.top, rc.right, rc.bottom, m_hWnd, NULL, g_hInst, NULL);

        m_bFirst = true;
    }

	SelectObject(hdc, hOldFont);
	SetTextColor(hdc, oldClr);
	SetBkMode(hdc, nOldMode);
}

void CNTXLBS_Bubble::ParseInfo()
{
	if(m_pInfo)
	{
		PIPCREQUEST pInfo = (PIPCREQUEST)m_pInfo;
		m_BubbleInfo.strTitle = wstring(BUBBLE_TITLE);

        if (m_pHyperLink != NULL)
        {
            m_BubbleInfo.strMsg = m_pHyperLink;
        }
        else
        {
            m_BubbleInfo.strMsg = wstring(pInfo->params[3]);
        }

		//	full parse
		ParseNotify(*m_pInfo, &m_ParsedInfo);
	}
}


void CNTXLBS_Bubble::ParseNotify(NOTIFY_INFO& notify, NotificationInfo* pInfo)
{
		//	parse time
		//	original one is: Monday May 27 16:20:25 CST 2010
		wstring time = notify.params[0];

		CNTXLBS_Bubble::FormatTimeStr(time, pInfo->time);

		//	parse enforcement
		pInfo->enforcement = DENY; // default to DENY
	
		if (notify.params[1][0] == L'D')
			pInfo->enforcement = DENY;
		else if (notify.params[1][0] == L'A')
			pInfo->enforcement = ALLOW;

		//	parse action
		pInfo->action = &(notify.params[1][1]);

		//	parse source resource title
		pInfo->file = notify.params[2];

		//	parse notification message string
		pInfo->message = notify.params[3];

	return;
}

extern CNotifyDlg* g_pDetailDlg;

void CNTXLBS_Bubble::ShowDetailedDialog()
{
#if 0
	if(g_pDetailDlg)
	{
		if(!g_pDetailDlg->m_hWnd)
		{
			g_pDetailDlg->Create(GetForegroundWindow());
		}

		if(g_pDetailDlg->m_hWnd)
		{
			g_pDetailDlg->SetNotifyInfo(m_ParsedInfo);

			g_pDetailDlg->FillData();
			g_pDetailDlg->ShowWindow(SW_SHOW);
			g_pDetailDlg->CenterWindow();


			g_pDetailDlg->ShowDetails(0);//Show the details

			CloseBubble();
		}
	}
#else
	if(m_hCallingWnd)
	{
		::PostMessageW(m_hCallingWnd, WM_SHOWNOTIFICATIONTAB, m_nIDNotification, 0);
	}
	CloseBubble();
#endif
}

LRESULT CNTXLBS_Bubble::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	if(PtInRect(&m_rcClose, pt))
	{
		//	::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
		CloseAllBubbles();
	}
	else
	{
		ShowDetailedDialog();
	}
	return 0;
}

LRESULT CNTXLBS_Bubble::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // TODO: Add your message handler code here and/or call default

    HDC hdc = (HDC)wParam;
    ::SetBkMode(hdc, TRANSPARENT);
    ::SetTextColor(hdc, RGB(0, 0, 0)); 

    HWND hWnd = (HWND)lParam;
    ::SendMessage(hWnd, WM_SETFONT, (WPARAM)m_hMsgFont, MAKELPARAM(TRUE, 0));

    bHandled = true;

    return (INT_PTR)CreateSolidBrush(RGB(240, 240, 240)); 
}

LRESULT CNTXLBS_Bubble::OnNMClickSyslink(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled)
{
    PNMLINK pNMLink = (PNMLINK)lParam;

    if (wcslen(pNMLink->item.szUrl) > 0)
    {
        ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOW);
    }

    return 0;
}