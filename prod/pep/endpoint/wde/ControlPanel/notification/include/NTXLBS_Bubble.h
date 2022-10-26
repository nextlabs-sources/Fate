#pragma once
#include "Resource.h"
#include "NXTLBS_Window.h"
#include <string>

#include "NotifyDlg.h"
#pragma warning(push)
#pragma warning(disable: 4189 4244 6211)
#include "WndShadow.h"
#pragma warning(pop)

using namespace std;

#pragma warning(push)
#pragma warning(disable: 4553 4100 4996 6386 6401 6211 6400 6244)
#include "atlapp.h"
#include "atlctrls.h"
#include "atlctrlx.h"
#pragma warning(pop)

typedef struct struBUBBLEINFO 
{
	wstring strTitle;
	wstring strMsg;
}BUBBLEINFO;


class CNTXLBS_Bubble: public CNXTLBS_Window<CNTXLBS_Bubble>
{
public:
	CNTXLBS_Bubble(void);
	~CNTXLBS_Bubble(void);

	BEGIN_MSG_MAP(CNTXLBS_Bubble)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPrintClient)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CAxDialogImpl<CNTXLBS_Bubble>)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        NOTIFY_CODE_HANDLER(NM_CLICK, OnNMClickSyslink)
        CHAIN_MSG_MAP(CAxDialogImpl<CNTXLBS_Bubble>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	enum { IDD = IDD_BUBBLE};

public:

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//it will call the CNXTLBS_Window::OnPaint if sub-class doesn't implement it.
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPrintClient(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DestroyWindow();
		m_bClosed = true;
		return 0;
	}
	
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_Shadow.Destroy();
		if(m_hBubbleIcon)
		{
			DestroyIcon(m_hBubbleIcon);
		}
		if(m_hCloseIcon)
		{
			DestroyIcon(m_hCloseIcon);
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	/*
	
	parameter:

	pInfo	-	notification information
	
	*/
    void SetInfo(void* pInfo, int nIDNotification, const WCHAR* pHyperLink = NULL){m_pInfo = (NOTIFY_INFO*)pInfo; m_nIDNotification = nIDNotification;
                                                                                   m_pHyperLink = pHyperLink; ParseInfo();}
	bool IsClosed()const{return m_bClosed;}
	void CloseBubble()
	{
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
	}

	void SetDefaultBubbleSize(int nWidth, int nHeight){m_nWidth = nWidth; m_nHeight = nHeight;}

	void SetCallingWnd(HWND hWnd){m_hCallingWnd = hWnd;}
protected:
	/*
	
	original notify
	
	*/
	typedef struct  
	{
		ULONG ulSize;
		WCHAR methodName [64];
		WCHAR params [7][256];
	}NOTIFY_INFO;


	void ParseInfo();

	void ParseNotify(NOTIFY_INFO& notify, NotificationInfo* pInfo);

	void DrawBubble(HDC hdc);


	void ShowDetailedDialog();

	static BOOL FormatTimeStr(wstring& strTime, wstring& strFormated);

	
protected:
	HICON m_hBubbleIcon;
	HICON m_hCloseIcon;
	HFONT m_hFont;
	HFONT m_hMsgFont;

	NOTIFY_INFO* m_pInfo;
	NotificationInfo m_ParsedInfo;
    const WCHAR* m_pHyperLink;

	BUBBLEINFO m_BubbleInfo;

	int m_nWidth;
	int m_nHeight;

	bool m_bClosed;

    bool m_bFirst;

	typedef struct
	{
		wstring str24Hour;
		wstring str12tHour;
		wstring strAMPM;
	}HourSwitch;

	static HourSwitch s_arrayHourSwitch[24];

	CWndShadow m_Shadow;

	int m_nIDNotification;

	HWND m_hCallingWnd;

public:
    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNMClickSyslink(WPARAM wParam, LPNMHDR lParam, BOOL& bHandled);
};

typedef struct struBubble 
{
	CNTXLBS_Bubble* pBubble;
	int nEntryTime;
	int nDuration;//0 means user has to close it manually,
}BUBBLE;
