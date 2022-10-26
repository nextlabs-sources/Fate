
#ifndef _ASSISTANT_DLH_H_
#define _ASSISTANT_DLH_H_

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlbase.h>
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
        m_bLast   = FALSE;
        m_bInited = FALSE;
    }
BEGIN_MSG_MAP(CAssistantDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy) 
    MESSAGE_HANDLER(WM_ITEMREMOVE, OnItemRemove)
    MESSAGE_HANDLER(WM_ITEMCHECK, OnItemCheck)
    MESSAGE_HANDLER(WM_ITEMCBNSELECT, OnItemCbnSelect)   
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	CHAIN_MSG_MAP(CCtrlColorBase)
    REFLECT_NOTIFICATIONS()
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
END_MSG_MAP()

public:
    virtual void ProcessItemRemove(int nItem){UNREFERENCED_PARAMETER(nItem);}
    virtual void ProcessItemCheck(int nItem, BOOL bChecked){UNREFERENCED_PARAMETER(nItem);UNREFERENCED_PARAMETER(bChecked);}
    virtual void ProcessItemCbnSelect(int nItem, int nSel){UNREFERENCED_PARAMETER(nItem);UNREFERENCED_PARAMETER(nSel);}
    virtual void AddViewData(){}
    virtual void ReallocWindows(){}
    virtual void ProcessOK(){}
    virtual void ProcessCancel(){}

    void SetLastFlag(){
        m_bLast=TRUE;
        if(m_bInited) ::SetWindowTextW(::GetDlgItem(m_hWnd, IDOK), L"Send Email");
    }
    void SetHelpUrl(LPCWSTR pwzHelpUrl){m_strHelpUrl = pwzHelpUrl;}

protected:
    static void SetClickWindow(HWND hWnd)
    {
        UNREFERENCED_PARAMETER(hWnd);
        Sleep(100);
        mouse_event(MOUSEEVENTF_MOVE, (DWORD)-10, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
        mouse_event(MOUSEEVENTF_MOVE, 10, 0, 0, NULL);
    }
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
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
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        // Re allocate sub controls
        ReallocWindows();

        // Set view
        AddViewData();
        m_viewDlg.ResetView();

        m_bInited = TRUE;
		return TRUE;
	}
	//LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	//{
	//	WCHAR wzText[] = L"C:\\Text\\black_sheep_jump_to_the_wall\\white_cat_climb_on_the_tree\\dog.docx";
	//	//EndDialog(wID);
	//	HDC hDC = GetWindowDC();
	//	Gdiplus::PointF pf(20, 20);
	//	Gdiplus::FontFamily myfontFamily(L"Verdana");
	//	Gdiplus::Font myfont(&myfontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//	Gdiplus::RectF  boundingBox(20, 30, 150,0);
	//	Gdiplus::RectF  boundBox(0, 0, 0,0);
	//	Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 0, 0));
	//	Gdiplus::StringFormat sf;
	//	sf.SetAlignment(Gdiplus::StringAlignmentNear);
	//	//sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	//	//sf.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);

	//	Gdiplus::Graphics g(hDC);
	//	g.MeasureString(wzText, -1, &myfont, boundingBox, &sf, &boundBox);
	//	g.DrawString(wzText, -1, &myfont, boundBox, &sf, &solidBrush);
	//	
	//	return 0;

    LRESULT OnItemRemove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        int   nItem   = (int)wParam;
        m_viewDlg.CleanView();
        ProcessItemRemove(nItem);
        return 0L;
    }
    LRESULT OnItemCheck(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        int   nItem   = (int)(wParam>>16)&0x0000FFFF;
        BOOL  bChecked= (BOOL)(wParam&0x00000001);
        ProcessItemCheck(nItem, bChecked);
        return 0L;
    }
    LRESULT OnItemCbnSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        int   nItem = (int)(wParam>>16)&0x0000FFFF;
        int   nSel  = (int)(wParam&0x0000FFFF);
        ProcessItemCbnSelect(nItem, nSel);
        return 0L;
    }

	//}
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wNotifyCode);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        ProcessOK();
		EndDialog(wID);
		return 0;
	}
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wNotifyCode);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        ProcessCancel();
		EndDialog(wID);
		return 0;
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
		m_viewDlg.DestroyWindow();		
		return 0;
	}

protected:
	CViewDlg	    m_viewDlg;
    BOOL            m_bLast;
    std::wstring    m_strHelpUrl;
    BOOL            m_bInited;
    //Gdiplus::Font   m_fontNormal;
    //Gdiplus::Font   m_fontBold;
};


#endif