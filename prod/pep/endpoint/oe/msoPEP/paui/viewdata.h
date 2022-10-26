

#ifndef _VIEW_DATA_H_
#define _VIEW_DATA_H_

#pragma warning(push)
#pragma warning(disable: 6386)

#include <atlbase.h>
#include <atlwin.h>
#pragma warning(pop)

#include <GdiPlus.h>
#include <vector>
#include "uiglobal.h"
#include "resource.h"
#include "winhelp.h"
#include "smart_ptr.h"


class CViewItemData
{
public:
    CViewItemData(){}
    CViewItemData(const CViewItemData& v){UNREFERENCED_PARAMETER(v);}
    CViewItemData& operator = (const CViewItemData& v){UNREFERENCED_PARAMETER(v);}
    BOOL operator == (const CViewItemData& v){UNREFERENCED_PARAMETER(v);return TRUE;}
    virtual ~CViewItemData(){}
};

class CDmData : public CViewItemData
{
public:
    CDmData()
    {
        bMultiUser  = TRUE;
        bMultiClient= TRUE;
        bChecked    = FALSE;
		bSelectable =FALSE;
        strRecipient= L"";
        attachInfo.clear();
    }
    CDmData(const CDmData &d)
    {
        bMultiUser  = d.bMultiUser;
        bMultiClient= d.bMultiClient;
        bChecked    = d.bChecked;
		bSelectable = d.bSelectable;
        strRecipient= d.strRecipient;
        attachInfo  = d.attachInfo;
    }
    CDmData& operator= (const CDmData& d)
    {
        bMultiUser  = d.bMultiUser;
        bMultiClient= d.bMultiClient;
        bChecked    = d.bChecked;
        strRecipient= d.strRecipient;
        attachInfo  = d.attachInfo;
        return *this;
    }
    virtual ~CDmData(){attachInfo.clear();}

public:
    BOOL                bMultiUser;
    BOOL                bMultiClient;
    BOOL                bChecked;
	BOOL				bSelectable;
    std::wstring        strRecipient;
    STRINGPAIRVECTOR    attachInfo;
};

class CTagData : public CViewItemData
{
public:
    CTagData()
    {
        nSelect = 0;
    }
    CTagData(const CTagData& t)
    {
        nSelect    = t.nSelect;
        strFile    = t.strFile;
        vecClients = t.vecClients;
    }
    CTagData& operator = (const CTagData& t)
    {
        nSelect    = t.nSelect;
        strFile    = t.strFile;
        vecClients = t.vecClients;
        return (*this);
    }
    virtual ~CTagData()
    {
    }

    int             nSelect;
    std::wstring    strFile;
    STRINGVECTOR    vecClients;
};

class CInterData : public CViewItemData
{
public:
    CInterData()
    {
    }
    CInterData(LPCWSTR pwzFile)
    {
        strFile = NULL==pwzFile?L"":pwzFile;
    }
    CInterData(const CInterData& i)
    {
        strFile = i.strFile;
    }
    CInterData& operator = (const CInterData& i)
    {
        strFile = i.strFile;
        return (*this);
    }
    virtual ~CInterData()
    {
    }

    std::wstring strFile;
};

class CViewItemDlg : public CDialogImpl<CViewItemDlg>,
    public CCtrlColor<CViewItemDlg, RGB(30,30,30), RGB(255,255,255)>
{
public:
    typedef CCtrlColor<CViewItemDlg, RGB(30,30,30), RGB(255,255,255)> CCtrlColorBase;
    enum { IDD = IDD_PAVIEWITEM };

    CViewItemDlg()
    {
        m_nIndex  = 0;
        m_bHide   = TRUE;
        m_nHideHeight   = MYMARGIN;
        m_nExpandHeight = m_nHideHeight;
    }

    virtual ~CViewItemDlg()
    {
    }

    BEGIN_MSG_MAP(CViewItemDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(IDC_ITEMREMOVE, OnItemRemove)
        COMMAND_ID_HANDLER(IDC_ITEMHIDESHOW, OnItemHideShow)
        COMMAND_ID_HANDLER(IDC_ITEMCHECK, OnItemCheck)
        COMMAND_ID_HANDLER(IDC_ITEMCOMBO, OnItemCombo)
        CHAIN_MSG_MAP(CCtrlColorBase)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        // Set font
        SendMessage(WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
        // Set data
        // Re-aalocate windows
        RealocWindow();
        
        return TRUE;
    }

    virtual void SetData(CViewItemData& data)
    {
        UNREFERENCED_PARAMETER(data);
    }

    virtual void RealocWindow()
    {
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        for (std::vector<HWND>::iterator it=m_tmpHwnds.begin(); it!=m_tmpHwnds.end(); ++it)
        {
            ::DestroyWindow(*it);
        }
        m_tmpHwnds.clear();
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMHIDESHOW));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMCHECK));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMMULTIUSER));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMMULTICLIENT));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT));
        //::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON));
        return 0L;
    }

    void OnFinalMessage(HWND hWnd)
    {
        UNREFERENCED_PARAMETER(hWnd);
        delete this;
    }

    LRESULT OnItemRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        if(BN_CLICKED==wNotifyCode)
        {
            m_bHide = TRUE;
            HWND hParent = ::GetParent(m_hWnd);
            ::SendMessage(hParent, WM_ITEMREMOVE, (WPARAM)m_nIndex, (LPARAM)m_hWnd);
        }
        return 0L;
    }

    virtual void ShowHiddenItem(BOOL bSHow)
    {
        UNREFERENCED_PARAMETER(bSHow);
    }

    LRESULT OnItemHideShow(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wNotifyCode);
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        //if(BN_CLICKED==wNotifyCode)
        {
            HWND hParent = ::GetParent(m_hWnd);
            if(m_bHide)
            {
                m_bHide = FALSE;
                ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMHIDESHOW), L"Hide");
                for (std::vector<HWND>::iterator it=m_tmpHwnds.begin(); it!=m_tmpHwnds.end(); ++it)
                    ::ShowWindow(*it, SW_SHOWNORMAL);
                ::ShowWindow(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE), SW_SHOWNORMAL);
                //::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, m_nWidth, m_nExpandHeight, SWP_NOMOVE|SWP_SHOWWINDOW);
            }
            else
            {
                m_bHide = TRUE;
                ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMHIDESHOW), L"Detail");
                for (std::vector<HWND>::iterator it=m_tmpHwnds.begin(); it!=m_tmpHwnds.end(); ++it)
                    ::ShowWindow(*it, SW_HIDE);
                ::ShowWindow(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE), SW_HIDE);
                //::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, m_nWidth, m_nHideHeight, SWP_NOMOVE|SWP_SHOWWINDOW);
            }
            ::SendMessage(hParent, WM_ITEMHIDESHOW, 0, 0);
        }
        return 0L;
    }

    LRESULT OnItemCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        HWND hParent = ::GetParent(m_hWnd);
        DWORD dwParam = m_nIndex; dwParam <<= 16;

        if(BN_CLICKED==wNotifyCode)
        {
            if(GetChecked())
            {
                // Checked
                dwParam |= 0x00000001;
                ::SendMessage(hParent, WM_ITEMCHECK, (WPARAM)dwParam, (LPARAM)m_hWnd);
            }
            else
            {
                // Unchecked
                ::SendMessage(hParent, WM_ITEMCHECK, (WPARAM)dwParam, (LPARAM)m_hWnd);
            }
        }

        return 0L;
    }

    LRESULT OnItemCombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(wID);
        UNREFERENCED_PARAMETER(hWndCtl);
        UNREFERENCED_PARAMETER(bHandled);
        HWND hParent = ::GetParent(m_hWnd);
        int  nSel = GetComboSel();
        DWORD dwParam = m_nIndex; dwParam <<= 16;
        dwParam |= nSel;
        if(CBN_SELCHANGE == wNotifyCode)
        {
            ::SendMessage(hParent, WM_ITEMCBNSELECT, (WPARAM)dwParam, (LPARAM)m_hWnd);
        }
        return 0L;
    }

    int GetRealHeight()
    {
        return m_bHide?m_nHideHeight:m_nExpandHeight;
    }

    BOOL GetChecked()
    {
        return (BST_CHECKED == SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), BM_GETCHECK, 0, 0));
    }

    int GetComboSel()
    {
        int nSel = 0;
        
        nSel = (int)SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO), CB_GETCURSEL, 0, 0);

        return nSel;
    }

public:
    static int  m_nX;
    static int  m_nWidth;

public:
    int         m_nIndex;
    BOOL        m_bHide;
    int         m_nHideHeight;
    int         m_nExpandHeight;
    std::vector<HWND> m_tmpHwnds;
};
//typedef std::vector<smart_ptr<CViewItemDlg>>    VIEWITEMVECTOR;
typedef std::vector<CViewItemDlg*>    VIEWITEMVECTOR;

//////////////////////////////////////////////////////////////////////////
class CDmItemDlg : public CViewItemDlg
{
public:
    CDmItemDlg() : CViewItemDlg()
    {
    }
    CDmItemDlg(CDmData& dm) : CViewItemDlg()
    {
        m_dmData = dm;
		m_dmData.bSelectable=dm.bSelectable;
    }
    virtual ~CDmItemDlg()
    {
    }

    void RealocWindow()
    {
        int nLeft = 0, nRight = 0, nTop = 0;
        HDC hDC = GetWindowDC();

        // Normal Area
        if(m_dmData.bMultiClient)
        {
            ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMMULTICLIENT), HWND_TOP, nLeft, m_nHideHeight, SMALLICON, SMALLICON, SWP_NOSIZE|SWP_SHOWWINDOW);
            m_nHideHeight += SMALLICON + LINESPACE;
        }
        if(m_dmData.bMultiUser)
        {
            ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMMULTIUSER), HWND_TOP, nLeft, m_nHideHeight, SMALLICON, SMALLICON, SWP_NOSIZE|SWP_SHOWWINDOW);
            m_nHideHeight += SMALLICON;
        }
        if(m_nHideHeight<(MYMARGIN+SMALLICON*2+LINESPACE))
            m_nHideHeight = (MYMARGIN+SMALLICON*2+LINESPACE);
        m_nHideHeight += DBLINESPACE;

        nLeft += SMALLICON; nLeft += DBLINESPACE;
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), HWND_TOP, nLeft, MYMARGIN+2, CHECKBOXCX, CHECKBOXCX, SWP_NOSIZE|SWP_SHOWWINDOW);
		nLeft += CHECKBOXCX;
		if(m_dmData.bSelectable==FALSE)
		{
			::EnableWindow(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), FALSE);
		}
		nRight = m_nWidth - LINESPACE - HIDESHOWBTNCX - MYMARGIN - DBLINESPACE;
        nLeft += DBLINESPACE;

		//
		RECT rcCheckBox;
		::GetWindowRect(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), &rcCheckBox);
		POINT ptRB = {rcCheckBox.right, rcCheckBox.bottom};
		POINT ptRT = {rcCheckBox.right, rcCheckBox.top};
		::ScreenToClient(m_hWnd, &ptRB);
		::ScreenToClient(m_hWnd, &ptRT);
		nLeft = ptRB.x +2;

        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), HWND_TOP, nLeft, ptRT.y + (ptRB.y-ptRT.y-13)/2, (nRight-nLeft), SMALLICON, SWP_SHOWWINDOW);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETFONT, (WPARAM)g_fntBold, (LPARAM)TRUE);
		HFONT  hFont = NULL;
		hFont = CreateFont(16,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("Segoe UI"));
		::SendMessage( ::GetDlgItem(m_hWnd,IDC_ITEMRECIPIENT), WM_SETFONT, (WPARAM)hFont, MAKELONG(TRUE,0) );
        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), m_dmData.strRecipient.c_str());
	
        nLeft = nRight + LINESPACE;

        // Hidden Area
        m_nExpandHeight = m_nHideHeight;
        int nSize = (int)m_dmData.attachInfo.size();
        if(nSize > 0)
        {
            HWND hAttach=0, hClientName=0, hClientValue=0;
            RECT rcClient;
            ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMHIDESHOW), HWND_TOP, nLeft, MYMARGIN+2, HIDESHOWBTNCX, HIDESHOWBTNCY, SWP_SHOWWINDOW);
            ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMHIDESHOW), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
            for (STRINGPAIRVECTOR::iterator it=m_dmData.attachInfo.begin(); it!=m_dmData.attachInfo.end(); ++it)
            {
                nLeft   = SMALLICON + DBLINESPACE + CHECKBOXCX + DBLINESPACE + LINESPACE;
                nRight  = m_nWidth - LINESPACE - HIDESHOWBTNCX - MYMARGIN - DBLINESPACE;
                nTop    = m_nExpandHeight;
                hAttach = CreateWindowW(L"STATIC", (*it).first.c_str(), WS_CHILD|/*WS_VISIBLE|*/SS_LEFTNOWORDWRAP|SS_ENDELLIPSIS, nLeft, nTop, (nRight-nLeft), FONTCY, m_hWnd, NULL, g_hInstance, 0);
                ::SendMessage(hAttach, WM_SETFONT, (WPARAM)g_fntBold, (LPARAM)TRUE);
                m_nExpandHeight += FONTCY+LINESPACE;
                nTop    += FONTCY+LINESPACE;
                hClientName = CreateWindowW(L"STATIC", L"Client: ", WS_CHILD|/*WS_VISIBLE|*/SS_LEFTNOWORDWRAP, nLeft+MYMARGIN, nTop, 30, FONTCY, m_hWnd, NULL, g_hInstance, 0);
                ::SendMessage(hClientName, WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);

                memset(&rcClient, 0, sizeof(RECT));
                rcClient.left = nLeft+MYMARGIN+30;  rcClient.right = nRight;
                rcClient.top  = nTop;               rcClient.bottom= nTop + 10;
                HGDIOBJ fntOld = SelectObject(hDC, (HGDIOBJ)g_fntNormal);
                ::DrawText(hDC, (*it).second.c_str(), -1, &rcClient, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);
                SelectObject(hDC, fntOld);
                hClientValue = CreateWindowW(L"STATIC", (*it).second.c_str(), WS_CHILD|/*WS_VISIBLE|*/SS_LEFT,
                    rcClient.left, rcClient.top, WIDTH(rcClient), HEIGHT(rcClient), m_hWnd, NULL, g_hInstance, 0);
                ::SendMessage(hClientValue, WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
                m_nExpandHeight += DBLINESPACE + HEIGHT(rcClient);

                m_tmpHwnds.push_back(hAttach);
                m_tmpHwnds.push_back(hClientName);
                m_tmpHwnds.push_back(hClientValue);
            }

            m_nExpandHeight += DBLINESPACE;
            nLeft = /*rcClient.right*/ (m_nWidth - LINESPACE - HIDESHOWBTNCX - MYMARGIN - DBLINESPACE) - BUTTONCX-DBLINESPACE*2;
            nTop  = m_nExpandHeight;
            ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE), HWND_TOP, nLeft, nTop, BUTTONCX, BUTTONCY, SWP_HIDEWINDOW);
            m_nExpandHeight += DBLINESPACE + BUTTONCY;
        }
    }

public:
    CDmData     m_dmData;
};

class CTagItemDlg : public CViewItemDlg
{
public:
    CTagItemDlg() : CViewItemDlg()
    {
    }
    CTagItemDlg(CTagData& tag) : CViewItemDlg()
    {
        m_tagData = tag;
    }
    virtual ~CTagItemDlg()
    {
    }
    void RealocWindow()
    {
        int nLeft = 0, nTextHeight = 0;
        HDC hDC = GetWindowDC();

        // Set Icon
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON), HWND_TOP, nLeft, m_nHideHeight, SMALLICON, SMALLICON, SWP_NOSIZE|SWP_SHOWWINDOW);
        nLeft += SMALLICON + DBLINESPACE;
        const WCHAR* pwzSuffix = wcsrchr(m_tagData.strFile.c_str(), L'.');
        if(pwzSuffix)
        {
            pwzSuffix++;
            if(0==_wcsicmp(pwzSuffix, L"doc")
                || 0==_wcsicmp(pwzSuffix, L"docx")
                )
            {
                // word
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                                         MAKEINTRESOURCE(IDB_WORDICON),
                                         IMAGE_BITMAP,
                                         SMALLICON,
                                         SMALLICON,
                                         LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"xls")
                || 0==_wcsicmp(pwzSuffix, L"xlsx")
                )
            {
                // excel
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                                        MAKEINTRESOURCE(IDB_EXECICON),
                                        IMAGE_BITMAP,
                                        SMALLICON,
                                        SMALLICON,
                                        LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"ppt")
                || 0==_wcsicmp(pwzSuffix, L"pptx")
                )
            {
                // power point
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_PWPTICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"pdf"))
            {
                // pdf
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_PDFICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else
            {
                //
            }
        }

        // Calculate/Set the text rectangle
        const WCHAR* pwzFileName = m_tagData.strFile.c_str();
        const WCHAR* pwzTmp = wcsrchr(m_tagData.strFile.c_str(), '\\');
        if(pwzTmp) pwzFileName = pwzTmp + 1;

        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        rcClient.left = nLeft; rcClient.top = m_nHideHeight; rcClient.bottom = rcClient.top + 10;
        rcClient.right= m_nWidth - LINESPACE - CMBOWIDTH - MYMARGIN - DBLINESPACE;
        HGDIOBJ fntOld = SelectObject(hDC, (HGDIOBJ)g_fntNormal);
        ::DrawText(hDC, pwzFileName, -1, &rcClient, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);
        SelectObject(hDC, fntOld);
        nTextHeight = HEIGHT(rcClient);
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), HWND_TOP, nLeft, m_nHideHeight, WIDTH(rcClient), HEIGHT(rcClient), SWP_SHOWWINDOW);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETTEXT, (WPARAM)0, (LPARAM)pwzFileName);

        // Set the combo box
        nLeft = m_nWidth - CMBOWIDTH - MYMARGIN - DBLINESPACE;;
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO), HWND_TOP, nLeft, m_nHideHeight, CMBOWIDTH, CMBOHEIGHT, SWP_SHOWWINDOW);
        //::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO), CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TAG_NOT_CLIENT_RELATED);
        for (STRINGVECTOR::iterator it=m_tagData.vecClients.begin(); it!=m_tagData.vecClients.end(); ++it)
        {
            ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO), CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(*it).c_str());
        }
        SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMCOMBO), CB_SETCURSEL, 0, 0);

        // Set height
        m_nHideHeight += (nTextHeight>CMBOHEIGHT)?nTextHeight:CMBOHEIGHT + DBLINESPACE*2;
        m_nExpandHeight=m_nHideHeight;
    }

public:
    CTagData     m_tagData;
};

class CInterItemDlg : public CViewItemDlg
{
public:
    CInterItemDlg() : CViewItemDlg()
    {
    }
    CInterItemDlg(CInterData& inter) : CViewItemDlg()
    {
        m_interData = inter;
    }
    virtual ~CInterItemDlg()
    {
    }
    void RealocWindow()
    {
        int nLeft = 0, nTextHeight = 0;
        HDC hDC = GetWindowDC();

        // Set Icon
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON), HWND_TOP, nLeft, m_nHideHeight, SMALLICON, SMALLICON, SWP_NOSIZE|SWP_SHOWWINDOW);
        nLeft += SMALLICON + DBLINESPACE;
        const WCHAR* pwzSuffix = wcsrchr(m_interData.strFile.c_str(), L'.');
        if(pwzSuffix)
        {
            pwzSuffix++;
            if(0==_wcsicmp(pwzSuffix, L"doc")
                || 0==_wcsicmp(pwzSuffix, L"docx")
                )
            {
                // word
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_WORDICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"xls")
                || 0==_wcsicmp(pwzSuffix, L"xlsx")
                )
            {
                // excel
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_EXECICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"ppt")
                || 0==_wcsicmp(pwzSuffix, L"pptx")
                )
            {
                // power point
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_PWPTICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else if(0==_wcsicmp(pwzSuffix, L"pdf"))
            {
                // pdf
                ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMFILEICON),
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    (LPARAM)::LoadImageW(g_hInstance,
                    MAKEINTRESOURCE(IDB_PDFICON),
                    IMAGE_BITMAP,
                    SMALLICON,
                    SMALLICON,
                    LR_DEFAULTCOLOR));
            }
            else
            {
                //
            }
        }

        // Calculate/Set the text rectangle
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        rcClient.left = nLeft; rcClient.top = m_nHideHeight; rcClient.bottom = rcClient.top + 10;
        rcClient.right= m_nWidth - MYMARGIN - DBLINESPACE;
        HGDIOBJ fntOld = SelectObject(hDC, (HGDIOBJ)g_fntNormal);
        ::DrawText(hDC, m_interData.strFile.c_str(), -1, &rcClient, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);
        SelectObject(hDC, fntOld);
        nTextHeight = HEIGHT(rcClient);
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), HWND_TOP, nLeft, m_nHideHeight, WIDTH(rcClient), HEIGHT(rcClient), SWP_SHOWWINDOW);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETTEXT, (WPARAM)0, (LPARAM)m_interData.strFile.c_str());

        // Set height
        m_nHideHeight += (nTextHeight>SMALLICON)?nTextHeight:SMALLICON + DBLINESPACE*2;
        m_nExpandHeight= m_nHideHeight;
    }

public:
    CInterData     m_interData;
};
//Mail Attribute Parsing
class CMapData : public CViewItemData
{
public:
    CMapData()
    {
        bChecked    = FALSE;
        strRecipient= L"";
    }
    CMapData(const CDmData &d)
    {
        bChecked    = d.bChecked;
        strRecipient= d.strRecipient;
    }
    CMapData& operator= (const CMapData& d)
    {
        bChecked    = d.bChecked;
        strRecipient= d.strRecipient;
        return *this;
    }
    virtual ~CMapData(){}

public:

    BOOL                bChecked;
    std::wstring        strRecipient;
};
class CMapItemDlg : public CViewItemDlg
{
public:
    CMapItemDlg() : CViewItemDlg()
    {
	}
	CMapItemDlg(CMapData& mapData) : CViewItemDlg()
	{
		m_mapData = mapData;
	}
	virtual ~CMapItemDlg()
	{
	}

	void RealocWindow()
	{
		int nLeft = 0, nRight = 0;

		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMMULTICLIENT), HWND_TOP, -nLeft, -m_nHideHeight, SMALLICON, SMALLICON,SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW /*SWP_NOSIZE|SWP_SHOWWINDOW*/);
		m_nHideHeight += SMALLICON /*+ LINESPACE*/;

		if(m_nHideHeight<(MYMARGIN+SMALLICON+LINESPACE))
			m_nHideHeight = (MYMARGIN+SMALLICON+LINESPACE);
		//m_nHideHeight += DBLINESPACE;

		nLeft += SMALLICON; nLeft += DBLINESPACE;
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
		//::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMCHECK), HWND_TOP, nLeft, MYMARGIN+2, CHECKBOXCX, CHECKBOXCX, /*SWP_NOSIZE|*/SWP_HIDEWINDOW);
		nRight = m_nWidth - LINESPACE - HIDESHOWBTNCX - MYMARGIN - DBLINESPACE;
		//nLeft += CHECKBOXCX; nLeft += DBLINESPACE;
		::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), HWND_TOP, nLeft, MYMARGIN, (nRight-nLeft), SMALLICON, SWP_SHOWWINDOW);
		::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), WM_SETFONT, (WPARAM)g_fntBold, (LPARAM)TRUE);
		::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMRECIPIENT), m_mapData.strRecipient.c_str());
		nLeft = nRight + LINESPACE;
	}

public:
   CMapData  m_mapData ;
};
#endif