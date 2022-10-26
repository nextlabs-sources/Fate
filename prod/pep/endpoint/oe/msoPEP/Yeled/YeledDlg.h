// YeledDlg.h : Declaration of the CYeledDlg

#pragma once

#include "resource.h"       // main symbols
#include "..\paui\uiglobal.h"

#include <atlhost.h>
#include "../common/policy.h"


#define NOTI_OBLIGATION	L"MAIL_NOTIFICATION"
#define OBLIGATION_NAME	L"EMailNotification"
#define VIOLATION_INFO	L"Violation"
#define LOGID_INFO		L"LogId"
#define WARNMSG_INFO	L"WarningMessage"
#define JUSTI_INFO		L"Justification"
#define NOTE_URL		L"Help URL"

#define WARN_RESULT_ONE	L"Warn and send"
#define WARN_RESULT_SEC	L"Justify and send"

#ifndef OBLIGATION_URL_DEFAULT
#define OBLIGATION_URL_DEFAULT L"http://www.nextlabs.com/"
#endif

typedef enum _emWarnType
{
	emKnown,
	emWarn1,
	emWarn2
} emWarnType;
// CYeledDlg



class CYeledDlg : 
	public CAxDialogImpl<CYeledDlg>
{
public:
	CYeledDlg(const MAILNOTIFIATTR& theMailAttr)
		:m_hTipWnd(NULL)
	{
		m_theMailAttr.IsValid = theMailAttr.IsValid;
		m_theMailAttr.strViolation = theMailAttr.strViolation;
		m_theMailAttr.strLogID  = theMailAttr.strLogID;
		m_theMailAttr.strWMsg = theMailAttr.strWMsg;
		m_theMailAttr.strJust = L"";
		m_theMailAttr.strWarnType = theMailAttr.strWarnType;
		m_theMailAttr.strUrl = theMailAttr.strUrl;

		m_bSend = true;
	}

	~CYeledDlg()
	{
	}

	enum { IDD = IDD_DLGYELED };

BEGIN_MSG_MAP(CYeledDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_WJUST, EN_CHANGE, OnEnChangeWjust)
	HANDLE_SYSCOMMAND_MESSAGE(SC_CONTEXTHELP, OnHelpContext)
	CHAIN_MSG_MAP(CAxDialogImpl<CYeledDlg>)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CAxDialogImpl<CYeledDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		bHandled = TRUE;
		SetPosition();
		return 1;  // Let the system set the focus
	}

	LRESULT OnHelpContext(BOOL& bCalldefaultHandler)
	{
		if(!m_theMailAttr.strUrl.empty())
			ShellExecuteW(GetDesktopWindow(), L"open", m_theMailAttr.strUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);

		bCalldefaultHandler = FALSE;
		return 0L;
	}
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		UNREFERENCED_PARAMETER(wNotifyCode);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(bHandled);
		// get the Just Info
		if(_wcsicmp(m_theMailAttr.strWarnType.c_str(),WARN_RESULT_SEC) == 0)
		{
			wchar_t szBuf[1024]={0};
			::GetWindowTextW(::GetDlgItem(m_hWnd,IDC_WJUST),szBuf,1024);
			m_theMailAttr.strJust = szBuf;
			if(m_theMailAttr.strJust.length() < 10)
			{
				MessageBox(L"You need to input the Justification");
				return 0;
			}
		}
		m_bSend = true;
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		UNREFERENCED_PARAMETER(wNotifyCode);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(bHandled);
		m_bSend = false;
		EndDialog(wID);
		return 0;
	}

	bool IsSend()
	{
		return m_bSend;
	}
	void DoNotification(HWND hMainWnd)
	{
		if(_wcsicmp(m_theMailAttr.strWarnType.c_str(),WARN_RESULT_ONE) != 0 &&
					_wcsicmp(m_theMailAttr.strWarnType.c_str(),WARN_RESULT_SEC) != 0)	return;

		this->DoModal(hMainWnd);

		if(IsSend())
		{

			std::vector<std::wstring> value;
			std::wstring strValue = VIOLATION_INFO;
			strValue += L" : ";
			strValue += m_theMailAttr.strViolation;
			value.push_back(strValue);

			strValue = WARNMSG_INFO;
			strValue += L" : ";
			strValue += m_theMailAttr.strWMsg;
			value.push_back(strValue);

			if(m_theMailAttr.strJust.size() >= 10)
			{
				strValue = JUSTI_INFO;
				strValue += L" : ";
				strValue += m_theMailAttr.strJust;
				value.push_back(strValue);
			}
			PolicyCommunicator::SetLogID(m_theMailAttr.strLogID.c_str());
			PolicyCommunicator::WriteReportLog(OBLIGATION_NAME,value);
			
		}
	}

	void AddToolTip(HWND hDlg,HWND hCtrlWnd,LPCTSTR szTipText)
	{
		UNREFERENCED_PARAMETER(szTipText);
		if(m_hTipWnd != NULL)	return ;
		try
		{
			m_hTipWnd = CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,NULL,WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP,
				CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hDlg,NULL,g_hInstance,NULL);
			::SetWindowPos(m_hTipWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
			::SendMessage(m_hTipWnd,TTM_ACTIVATE,TRUE,0);

			TOOLINFO ti;
			ZeroMemory(&ti,sizeof(ti));
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND|TTF_SUBCLASS;
			ti.hwnd = hDlg;
			ti.hinst = g_hInstance;
			ti.uId = (UINT)hCtrlWnd;
			ti.lpszText = L"Just a test!!!!!!!!!!!!!!!";
			::SendMessage(m_hTipWnd,TTM_ADDTOOL,0,(LPARAM)&ti);
			::SendMessage(m_hTipWnd,TTM_SETMAXTIPWIDTH,0,300);
			::SendMessage(m_hTipWnd,TTM_SETDELAYTIME,TTDT_AUTOPOP,4000);
		}
		catch (...)
		{
			
		}
	}
private:
	MAILNOTIFIATTR	m_theMailAttr;
	bool	m_bSend;

	HWND m_hTipWnd;
private:
	void SetPosition()
	{
		RECT rcWinClient, rcUserClient;
		int  nCY = 0;

		// Set Icon
		HICON hIcon = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIN), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// Set whole windows position
		SetWindowPos(HWND_TOP, 0, 0, WINCX, WINCY, SWP_NOMOVE);
		GetClientRect(&rcWinClient);
		rcUserClient.top   = rcUserClient.left = MYMARGIN;
		rcUserClient.right = rcWinClient.right - MYMARGIN;
		rcUserClient.bottom= rcWinClient.bottom - MYMARGIN;
		nCY = rcUserClient.top;

#define STATIC_HEIGHT	20
		DWORD dwWidth = WIDTH(rcUserClient);
		// Set main icon and text
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_YMAIL), HWND_TOP, rcUserClient.left, 5, BIGICON, BIGICON, SWP_SHOWWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_VIOSTATIC), HWND_TOP, rcUserClient.left+BIGICON+MYMARGIN/2, nCY+3, dwWidth -(BIGICON+MYMARGIN/2), STATIC_HEIGHT, SWP_SHOWWINDOW);
		::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_VIOSTATIC), m_theMailAttr.strViolation.c_str());
		nCY += BIGICON;

		// set tip
		//AddToolTip(m_hWnd,::GetDlgItem(m_hWnd,IDC_VIOSTATIC),L"just a test");


		// Set Horz Etched Line
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_YHORZETCHED), HWND_TOP, rcUserClient.left, nCY, dwWidth, LINESPACE, SWP_SHOWWINDOW);
		nCY += LINESPACE;
		nCY += LINESPACE;

		DWORD dwHeight = rcUserClient.bottom - rcUserClient.top - nCY - BUTTONCY - LINESPACE;

		// hidden the tag name
		::ShowWindow(::GetDlgItem(m_hWnd,IDC_JSTATIC),SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd,IDC_SYELED4),SW_HIDE);

		::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_WMSG), m_theMailAttr.strWMsg.c_str());

		if(_wcsicmp(m_theMailAttr.strWarnType.c_str(),WARN_RESULT_ONE) == 0)
		{
			// hide the justification infomation.
			HWND hEJust = ::GetDlgItem(m_hWnd,IDC_WJUST);
			::ShowWindow(hEJust,SW_HIDE);		

			::SetWindowPos(::GetDlgItem(m_hWnd,IDC_WMSG),HWND_TOP,rcUserClient.left,nCY,dwWidth,dwHeight,SWP_SHOWWINDOW);
		}
		else if(_wcsicmp(m_theMailAttr.strWarnType.c_str(),WARN_RESULT_SEC) == 0)
		{

			DWORD nHigh = dwHeight - 2*LINESPACE;
			nHigh /= 2;
			::SetWindowPos(::GetDlgItem(m_hWnd,IDC_WMSG),HWND_TOP,rcUserClient.left,nCY,WIDTH(rcUserClient),nHigh,SWP_SHOWWINDOW);
			nCY += nHigh;

			nCY += LINESPACE;
			::SetWindowPos(::GetDlgItem(m_hWnd,IDC_WJUST),HWND_TOP,rcUserClient.left,nCY,WIDTH(rcUserClient),nHigh,SWP_SHOWWINDOW);
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK),FALSE);
		}
		// Set Button
		nCY = rcUserClient.bottom-BUTTONCY;

		DWORD dwXButton = 110;
		DWORD dwSep = (rcUserClient.right-rcUserClient.left)/2;
		//::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.right-dwXButton*2-10, nCY, dwXButton, BUTTONCY, SWP_SHOWWINDOW);
		//::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.right-dwXButton, nCY, dwXButton, BUTTONCY, SWP_SHOWWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDOK), HWND_TOP, rcUserClient.left + dwSep - 115, nCY, dwXButton, BUTTONCY, SWP_SHOWWINDOW);
		::SetWindowPos(::GetDlgItem(m_hWnd, IDCANCEL), HWND_TOP, rcUserClient.left + dwSep + 5, nCY, dwXButton, BUTTONCY, SWP_SHOWWINDOW);
	}

public:
	LRESULT OnEnChangeWjust(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		wchar_t szBuf[1024]={0};
		::GetWindowTextW(::GetDlgItem(m_hWnd,IDC_WJUST),szBuf,1024);
		if(wcslen(szBuf) >= 10)
		{
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK),TRUE);
		}		
		else
		{
			::EnableWindow(::GetDlgItem(m_hWnd, IDOK),FALSE);
		}
		return S_OK;
	}
};


