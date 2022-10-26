#pragma once


#ifndef OUTLOOK_HYPERLINK_H
#define OUTLOOK_HYPERLINK_H

#include <vector>
#include <string>
using namespace std;


typedef struct _DISPLAYMSGTYPE
{
	wstring strMsg;
	BOOL    bIsHttpAdd;
	BOOL bIsAutoWrap;

}DISPLAYMSGTYPE,*LPDISPLAYMSGTYPE;


class CHyperlink
{
public:
	CHyperlink(void);
	~CHyperlink(void);

public:
	void CovertTextToHyperLink();

	static LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message,
		WPARAM wParam, LPARAM lParam);

	void DrawMsg();
	bool IsInHyperLinkRect(int x, int y);
	void Navigate();
	void GetWrapLineMsg(_In_ wstring& strMsg,vector<DISPLAYMSGTYPE>& vecDisplayMsg,_Out_ int & nLineNum);
	BOOL GetDisplayMsg(_In_ wstring& strOrg, _Out_ vector<DISPLAYMSGTYPE>& vecDisplayMsg, _Out_ wstring& strHttp,_Out_ int & nLineNum);


public:
	WNDPROC  m_pfnOrigCtlProc;
	WNDPROC  m_pfnOrigParentProc;
	HWND     m_hCtrlWnd;
	HWND     m_hParentWnd;
	RECT     m_HyperLinkRect;
	BOOL     m_bHyperLink;
	wstring  m_strHttpAddr;

	HCURSOR  m_hLinkCursor;
	vector<DISPLAYMSGTYPE> m_vecMsg;
	int      m_nLineNumber;

};

class CNotificationBubble
{
public:
	CNotificationBubble();
	~CNotificationBubble();
	void ShowBubble(const wchar_t* pHyperLink, int Timeout);

	static HWND g_hBubbleWnd;

	typedef struct  
	{
		ULONG ulSize;
		WCHAR methodName [64];
		WCHAR params [7][256];
	}NOTIFY_INFO;

	typedef void (__stdcall* type_notify2)(NOTIFY_INFO* pInfo, int nDuration, HWND hCallingWnd, void* pReserved, const WCHAR* pHyperLink);

	static type_notify2 notify2;
	CRITICAL_SECTION m_showbubbleCriticalSection;
	static LRESULT CALLBACK BubbleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI BubbleThreadProc(LPVOID lpParameter);
	
};


#endif