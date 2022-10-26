#include "StdAfx.h"
#include "Hyperlink.h"
#include "strsafe.h"

#define PROP_OBJECT_PTR			MAKEINTATOM(ga.atom)

#define GWL_WNDPROC         (-4)

#pragma warning(disable:4311)
#pragma warning(disable:4312)

extern HINSTANCE g_hInstance;

HWND CNotificationBubble::g_hBubbleWnd = NULL;
CNotificationBubble::type_notify2 CNotificationBubble::notify2 = NULL;

class CGlobalAtom
{
public:
	CGlobalAtom(void)
	{ atom = GlobalAddAtom(TEXT("_Hyperlink_NextLabs_Object_Pointer_")
	TEXT("\\{AFEED740-CC6D-47c5-831D-9848FD916EEF}")); }
	~CGlobalAtom(void)
	{ DeleteAtom(atom); }

	ATOM atom;
};


static CGlobalAtom ga;


CHyperlink::CHyperlink(void)
{
	m_bHyperLink = FALSE;
	m_hLinkCursor = ::LoadCursor(NULL, IDC_HAND);
	m_nLineNumber = 1;
}

CHyperlink::~CHyperlink(void)
{
}


bool CHyperlink::IsInHyperLinkRect(int x, int y)
{
	if (x > m_HyperLinkRect.left && x < m_HyperLinkRect.right 
		&&y > m_HyperLinkRect.top && y < m_HyperLinkRect.bottom)
	{
		return true;
	}
	return false;
}

void CHyperlink::DrawMsg()
{

	HDC hdc = GetDC(m_hParentWnd);
	SetBkMode(hdc, TRANSPARENT);

	RECT rect;
	::GetWindowRect(m_hCtrlWnd,&rect);
	
	POINT point;
	point.x = rect.left;
	point.y = rect.top;
	::ScreenToClient(m_hParentWnd,&point);
	
	RECT TxtRect;
	TxtRect.left = point.x;
	TxtRect.right = point.x;
	TxtRect.top = point.y;
	TxtRect.bottom = point.y;
	SIZE size;  
	int LineNum = 0;

	HFONT hFont = CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Segoe UI"));
	HFONT hOldFont = (HFONT)::SelectObject(hdc,hFont);

	vector<DISPLAYMSGTYPE>::iterator itor;
	for (itor = m_vecMsg.begin(); itor != m_vecMsg.end(); itor++)
	{

		::GetTextExtentPoint32(hdc,itor->strMsg.c_str(),static_cast<int>(itor->strMsg.length()),&size); 

		TxtRect.left = TxtRect.right;
		TxtRect.right = TxtRect.left + size.cx;
		if (TxtRect.right - point.x > 400)
		{
			TxtRect.right = TxtRect.left + (size.cx/static_cast<int>(itor->strMsg.length()))*60;
		}
		TxtRect.bottom = TxtRect.top + size.cy;
		
		if (itor->bIsHttpAdd)
		{
			m_bHyperLink = TRUE;
			SetTextColor(hdc,RGB(0,0,255));
			DrawText(hdc,itor->strMsg.c_str(),static_cast<int>(itor->strMsg.length()),&TxtRect,DT_SINGLELINE);
			m_HyperLinkRect = TxtRect;
		}
		else
		{
			SetTextColor(hdc,RGB(0,0,0));
			DrawText(hdc,itor->strMsg.c_str(),static_cast<int>(itor->strMsg.length()),&TxtRect,DT_SINGLELINE);
		}

		if (itor->bIsAutoWrap)
		{
			TxtRect.top = TxtRect.bottom;
			TxtRect.right = point.x;
			LineNum++;
			if(LineNum > 4)
				break;
		}
		
	}
	::SelectObject(hdc,hOldFont);
}

void CHyperlink::CovertTextToHyperLink()
{

	if(m_hParentWnd == NULL)
	{
		return ;
	}


	m_pfnOrigParentProc = (WNDPROC)GetWindowLongPtrW(m_hParentWnd, GWL_WNDPROC);
	if (m_pfnOrigParentProc != _HyperlinkParentProc)
	{
		SetProp( m_hParentWnd, PROP_OBJECT_PTR, (HANDLE)this);
#if defined(_WIN64)
		SetWindowLongPtrW( m_hParentWnd, GWL_WNDPROC,(LONG_PTR)(WNDPROC)_HyperlinkParentProc);
#else
		SetWindowLongPtrW( m_hParentWnd, GWL_WNDPROC,(LONG)(INT_PTR)_HyperlinkParentProc);
#endif
	}
}


LRESULT CALLBACK CHyperlink::_HyperlinkParentProc(HWND hwnd, UINT message,
												  WPARAM wParam, LPARAM lParam)
{
	CHyperlink *pHyperLink = (CHyperlink *)GetProp(hwnd, PROP_OBJECT_PTR);
	if (pHyperLink == NULL)
	{
		return 0;
	}
	switch (message)
	{
	case WM_PAINT:
		{
			pHyperLink->DrawMsg();
			break;
		}
	case WM_DESTROY:
		{
			pHyperLink->m_bHyperLink = FALSE; 
			SetWindowLong(hwnd, GWL_WNDPROC, reinterpret_cast<LONG>(pHyperLink->m_pfnOrigParentProc));
			RemoveProp(hwnd, PROP_OBJECT_PTR);
			break;
		}
	case WM_MOUSEMOVE:
		{
			int xPos = LOWORD(lParam); 
			int yPos = HIWORD(lParam);
			if (pHyperLink->m_bHyperLink)
			{
				bool bIsInLink = pHyperLink->IsInHyperLinkRect(xPos,yPos);
				if (bIsInLink)
				{
					SetCursor(pHyperLink->m_hLinkCursor);
				}
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			int xPos = LOWORD(lParam); 
			int yPos = HIWORD(lParam);
			
			if (pHyperLink->m_bHyperLink)
			{
				bool bIsInLink = pHyperLink->IsInHyperLinkRect(xPos,yPos);
				if (bIsInLink)
				{
					pHyperLink->Navigate();
				}
			}
			break;
		}
	}
	return CallWindowProc(pHyperLink->m_pfnOrigParentProc, hwnd, message, wParam, lParam);
}

void CHyperlink::Navigate(void)
{
	SHELLEXECUTEINFO sei;
	::ZeroMemory(&sei,sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof( SHELLEXECUTEINFO );		// Set Size
	sei.lpVerb = TEXT( "open" );					// Set Verb
	sei.lpFile = m_strHttpAddr.c_str();							// Set Target To Open
	sei.nShow = SW_SHOWNORMAL;						// Show Normal

	ShellExecuteEx(&sei);
	
}



void CHyperlink::GetWrapLineMsg(_In_ wstring& strMsg,vector<DISPLAYMSGTYPE>& vecDisplayMsg,_Out_ int & nLineNum)
{
	if (strMsg.empty())
	{
		return;
	}
	size_t nWrapPos = 0;
	DISPLAYMSGTYPE TempMsg;
	wstring strTempOrg; 
	do 
	{
		nWrapPos = strMsg.find(L"\\n");
		if (nWrapPos != wstring::npos)
		{
			nLineNum += 1;
			strTempOrg = strMsg.substr(0, nWrapPos);
			if (!strTempOrg .empty())
			{
				TempMsg.bIsHttpAdd = FALSE;
				TempMsg.strMsg = strTempOrg;
				TempMsg.bIsAutoWrap = TRUE;
				vecDisplayMsg.push_back(TempMsg);
			}

			strMsg = strMsg.substr(nWrapPos + 2);

		}
		else
		{
			if (!strMsg.empty())
			{
				TempMsg.bIsHttpAdd = FALSE;
				TempMsg.strMsg = strMsg;
				TempMsg.bIsAutoWrap = FALSE;
				vecDisplayMsg.push_back(TempMsg);
			}
			break;
		}

	} while (TRUE);

}

BOOL CHyperlink::GetDisplayMsg(_In_ wstring& strOrg, _Out_ vector<DISPLAYMSGTYPE>& vecDisplayMsg, _Out_ wstring& strHttp,_Out_ int & nLineNum)
{
	BOOL bRet = FALSE;
	size_t nHttpFirstPos = strOrg.find(L"<a href=\"");
	size_t nHttpEndPos = 0;
	size_t nDisHttpEndPos = 0;
	wstring strDisHttpMsg = L"";
	wstring strFirstMsg = L"";
	wstring strLastMsg = L"";
	BOOL bHttpAutoWrap = FALSE;
	strHttp = L"";
	nLineNum = 1;
	if (nHttpFirstPos != wstring::npos)
	{
		nDisHttpEndPos = strOrg.find(L"</a>");
		if (nDisHttpEndPos != wstring::npos)
		{
			nHttpEndPos = strOrg.find(L"\">");
			if (nHttpEndPos != wstring::npos && nHttpEndPos > nHttpFirstPos && nDisHttpEndPos > nHttpEndPos)
			{
				strHttp = strOrg.substr(nHttpFirstPos + 9,nHttpEndPos - nHttpFirstPos - 9);
				strDisHttpMsg = strOrg.substr(nHttpEndPos + 2, nDisHttpEndPos - nHttpEndPos - 2);
				if (nHttpFirstPos != 0)
				{
					strFirstMsg = strOrg.substr(0,nHttpFirstPos);
				}
				if (strOrg.length() > nDisHttpEndPos + 4)
				{
					wstring strTemp = strOrg.substr(nDisHttpEndPos + 4, 2);
					if(0 == _wcsicmp(strTemp.c_str(),L"\\n"))
					{
						bHttpAutoWrap = TRUE;
						nLineNum += 1;
					}
					if (strOrg.length() > nDisHttpEndPos + 5 && bHttpAutoWrap)
					{
						strLastMsg = strOrg.substr(nDisHttpEndPos + 6);
					}
					else
					{
						strLastMsg = strOrg.substr(nDisHttpEndPos + 4);
					}

				}
				bRet = TRUE;
			}
		}
	}

	if (bRet == FALSE)
	{
		strFirstMsg = strOrg;
	}

	GetWrapLineMsg(strFirstMsg, vecDisplayMsg,nLineNum);

	if (bRet == TRUE)
	{
		DISPLAYMSGTYPE TempMsg;
		TempMsg.bIsHttpAdd = TRUE;
		TempMsg.bIsAutoWrap = bHttpAutoWrap;
		TempMsg.strMsg = strDisHttpMsg;
		vecDisplayMsg.push_back(TempMsg);
	}

	GetWrapLineMsg(strLastMsg, vecDisplayMsg,nLineNum);


	return bRet;
}


CNotificationBubble::CNotificationBubble()
{
	InitializeCriticalSection(&m_showbubbleCriticalSection);
}
CNotificationBubble::~CNotificationBubble()
{
	DeleteCriticalSection(&m_showbubbleCriticalSection);
}

void CNotificationBubble::ShowBubble(const wchar_t* pHyperLink, int Timeout)
{
	static bool bFirst = false;

	if (!bFirst)
	{
		::EnterCriticalSection(&m_showbubbleCriticalSection);
		if (!bFirst)
		{
			HANDLE hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

			CreateThread(NULL, 0, BubbleThreadProc, hEvent, 0, NULL);

			WaitForSingleObject(hEvent, INFINITE);
			CloseHandle(hEvent);

			bFirst = true;
		}    
		::LeaveCriticalSection(&m_showbubbleCriticalSection);
	}

	SendMessage(g_hBubbleWnd, WM_USER + 1, (WPARAM)pHyperLink, Timeout);
}

DWORD WINAPI CNotificationBubble::BubbleThreadProc(LPVOID lpParameter)
{
	WCHAR CurrentDir[MAX_PATH] = { 0 };

	GetModuleFileNameW(g_hInstance, CurrentDir, MAX_PATH);

    wstring strDeskTopEnforceDir(CurrentDir);
	size_t nPos = strDeskTopEnforceDir.rfind(L"\\Outlook Enforcer");
	if (nPos != wstring::npos)
	{
		strDeskTopEnforceDir = strDeskTopEnforceDir.substr(0,nPos + 1) + L"Control Panel\\bin";
	}

	WCHAR szDeskTopEnforceDir[MAX_PATH] = {0};
     
#ifdef _WIN64
	StringCchPrintf(szDeskTopEnforceDir,MAX_PATH,L"%s%s",strDeskTopEnforceDir.c_str(),L"\\notification.dll");
#else
	StringCchPrintf(szDeskTopEnforceDir,MAX_PATH,L"%s%s",strDeskTopEnforceDir.c_str(),L"\\notification32.dll");
#endif

	HMODULE hmodule = LoadLibraryW(szDeskTopEnforceDir);

	if (hmodule == NULL)
	{
		SetEvent((HANDLE)lpParameter);
		return 0;
	}

	notify2 = (type_notify2)GetProcAddress(hmodule, "notify2");

	if (notify2 == NULL)
	{   
		SetEvent((HANDLE)lpParameter);
		return 0;
	}


	WNDCLASSEX wcex = { 0 };

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;
	wcex.lpfnWndProc	= BubbleWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= L"BubbleClass";
	wcex.hIconSm		= NULL;

	RegisterClassExW(&wcex);

	g_hBubbleWnd = CreateWindowW(L"BubbleClass", NULL, 0, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);;

	SetEvent((HANDLE)lpParameter);

	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK CNotificationBubble::BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_USER + 1:
		{
			NOTIFY_INFO Info = { 0 };
			wcsncpy_s(Info.methodName, 64, L"ShowNotification", _TRUNCATE);
			wcsncpy_s(Info.params[0], 256, L"Fri Mar 06 11:36:47 CST 2015", _TRUNCATE);
			wcsncpy_s(Info.params[1], 256, L"ACOPY", _TRUNCATE);
			wcsncpy_s(Info.params[2], 256, L"file:///c:/fake", _TRUNCATE);
			wcsncpy_s(Info.params[3], 256, L"fake", _TRUNCATE);

			notify2(&Info, (int)lParam, 0, 0, (const WCHAR*)wParam);
		}

	case WM_PAINT:
		{
			PAINTSTRUCT ps = { 0 };
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
