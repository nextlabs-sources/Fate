
#ifndef _ASSISTANT_DLH_H_
#define _ASSISTANT_DLH_H_
#include <atlbase.h>
#pragma warning(push)
#pragma warning(disable: 6387 6386)
#include <atlwin.h>
#pragma warning(pop)
#include <GdiPlus.h>
#include "uiglobal.h"
#include "resource.h"
#include "winhelp.h"
#include "viewdlg.h"
#include "viewdata.h"


class CAssistantDialog : public CDialogImpl<CAssistantDialog>,
	public CCtrlColor<CAssistantDialog, RGB(30,30,30)>   //, RGB(223,223,223)
{
public:
    typedef CCtrlColor<CAssistantDialog, RGB(30,30,30)> CCtrlColorBase;
	enum { IDD = IDD_PADLG };
    CAssistantDialog()
    {
        m_bLast = FALSE;
    }
BEGIN_MSG_MAP(CAssistantDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy) 
    MESSAGE_HANDLER(WM_ITEMREMOVE, OnItemRemove)  
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	CHAIN_MSG_MAP(CCtrlColorBase)
    REFLECT_NOTIFICATIONS()
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
END_MSG_MAP()

public:
#pragma warning(push)
#pragma warning(disable: 4100)
    virtual void ProcessItemRemove(int nItem){}
#pragma warning(pop)
    virtual void AddViewData(){}
    virtual void ReallocWindows(){}
    virtual void ProcessOK(){}
    virtual void ProcessCancel(){}

    void SetLastFlag(){m_bLast=TRUE;}
    void SetHelpUrl(LPCWSTR pwzHelpUrl){m_strHelpUrl = pwzHelpUrl;}

protected:
#pragma warning(push)
#pragma warning(disable: 4100 4245)
    static void SetClickWindow(HWND hWnd)
    {
        Sleep(100);
        mouse_event(MOUSEEVENTF_MOVE, -10, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_MOVE, 10, 0, 0, NULL);
    }
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if(LOWORD(wParam) == SC_CONTEXTHELP)
        {
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CAssistantDialog::SetClickWindow, m_hWnd, 0, NULL);
            if(hThread) CloseHandle(hThread);
            if(m_strHelpUrl.length()>0)
                ::ShellExecuteW(GetDesktopWindow(), L"open", m_strHelpUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
        else if(LOWORD(wParam) == SC_CLOSE)
        {
            EndDialog(IDCANCEL);
        }
        else
        {
            bHandled = FALSE;
        }

        bHandled = FALSE;
        return 0L;
    }

    void SetPathControlFormat()
    {
        LONG lStyle = ::GetWindowLongW(::GetDlgItem(m_hWnd, IDC_FILEPATH), GWL_STYLE);
        //lStyle &= ~
        lStyle |= SS_PATHELLIPSIS;
        ::SetWindowLong(::GetDlgItem(m_hWnd, IDC_FILEPATH), GWL_STYLE, lStyle);
    }
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        // Re allocate sub controls
        ReallocWindows();

        // Reset filepath style
        SetPathControlFormat();

        // Set view
        AddViewData();
        m_viewDlg.ResetView();
				
		return TRUE;
	}

    LRESULT OnItemRemove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int   nItem   = (int)wParam;
        ProcessItemRemove(nItem);
        return 0L;
    }

	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        ProcessOK();
		//EndDialog(wID);
		return 0;
	}
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
        ProcessCancel();
		//EndDialog(wID);
		return 0;
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_viewDlg.DestroyWindow();		
		return 0;
	}
#pragma warning(pop)
protected:
	CViewDlg	    m_viewDlg;
    BOOL            m_bLast;
    std::wstring    m_strHelpUrl;
};


#endif