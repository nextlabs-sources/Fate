

#ifndef _VIEW_DATA_H_
#define _VIEW_DATA_H_
#include <atlbase.h>
#pragma warning(push)
#pragma warning(disable: 6387 6386)
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
#pragma warning(push)
#pragma warning(disable: 4100)
    CViewItemData(const CViewItemData& v){}
    CViewItemData& operator = (const CViewItemData& v){}
    BOOL operator == (const CViewItemData& v){return TRUE;}
#pragma warning(pop)
    virtual ~CViewItemData(){}
};

class CHdrData : public CViewItemData
{
public:
    CHdrData()
    {
		bBtnVisible = FALSE;
		bFiltered = FALSE;
    }
    CHdrData(const CHdrData &h)
    {
        strTitle = h.strTitle;
        strBody  = h.strBody;
        nStatus  = h.nStatus;
		bBtnVisible = h.bBtnVisible;
		bFiltered = h.bFiltered;
    }
    CHdrData& operator= (const CHdrData& h)
    {
        strTitle = h.strTitle;
        strBody  = h.strBody;
        nStatus  = h.nStatus;
		bBtnVisible = h.bBtnVisible;
		bFiltered = h.bFiltered;
        return *this;
    }
    virtual ~CHdrData(){}

public:
    std::wstring        strTitle;
    std::wstring        strBody;
    int                 nStatus;
	BOOL				bBtnVisible;
	BOOL				bFiltered;
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
		m_nCatIndex = 0;
        m_bHide   = TRUE;
        m_nHeight = MYMARGIN;
		m_hFontBtnRemoveAll = CreateFontW(15,
			0,
			0,
			0,
			400,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Arial"
			); 
    }

    virtual ~CViewItemDlg()
    {
		if (m_hFontBtnRemoveAll!=NULL)
		{
			DeleteObject(m_hFontBtnRemoveAll);
			m_hFontBtnRemoveAll = NULL;
		}	
    }

    BEGIN_MSG_MAP(CViewItemDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(IDC_ITEMREMOVE, OnItemRemove)
        CHAIN_MSG_MAP(CCtrlColorBase)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // Set font
        SendMessage(WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
		if (m_hFontBtnRemoveAll!=NULL)
		{
			::SendMessage(GetDlgItem(IDC_ITEMREMOVE), WM_SETFONT, (WPARAM)m_hFontBtnRemoveAll, (LPARAM)TRUE);
		}	
        // Set data
        // Re-aalocate windows
        RealocWindow();
        
        return TRUE;
    }

    virtual void SetData(CViewItemData& data)
    {
    }

    virtual void RealocWindow()
    {
    }

    virtual void ResetData(LPCWSTR pwzTitle, LPCWSTR pwzBody, int nStatus)
    {
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMICON));
        ::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE));
        ::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMTITLE));
        ::DestroyWindow(::GetDlgItem(m_hWnd, IDC_ITEMBODY));
        return 0L;
    }

    void OnFinalMessage(HWND hWnd)
    {
        delete this;
    }

    LRESULT OnItemRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if(BN_CLICKED==wNotifyCode)
        {
            HWND hParent = ::GetParent(m_hWnd);
            ::SendMessage(hParent, WM_ITEMREMOVE, (WPARAM)m_nCatIndex, (LPARAM)m_hWnd);
        }
        return 0L;
    }

    int GetRealHeight()
    {
        return m_nHeight;
    }

public:
    static int  m_nX;
    static int  m_nWidth;

public:
    int         m_nIndex;
	int			m_nCatIndex;
    BOOL        m_bHide;
    int         m_nHeight;
	HFONT       m_hFontBtnRemoveAll;
};
typedef std::vector<CViewItemDlg*>    VIEWITEMVECTOR;

//////////////////////////////////////////////////////////////////////////
class CHdrItemDlg : public CViewItemDlg
{
public:
    CHdrItemDlg() : CViewItemDlg()
    {
    }
    CHdrItemDlg(CHdrData& hdr) : CViewItemDlg()
    {
        m_hdrData = hdr;
    }
    virtual ~CHdrItemDlg()
    {
    }

    void ResetData(LPCWSTR pwzTitle, LPCWSTR pwzBody, int nStatus)
    {
        m_hdrData.strTitle = pwzTitle;
        m_hdrData.strBody  = pwzBody;
        m_hdrData.nStatus  = nStatus;
        m_nHeight = MYMARGIN;
        RealocWindow();
    }

    void RealocWindow()
    {
        int nLeft = 0, nRight = 0, nTop = 0;
        RECT rcClient;
        HDC hDC = GetWindowDC();

        // Set splitter
        if(0!=m_nIndex) ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_SPLITTER), HWND_TOP, nLeft, m_nHeight, m_nWidth, 1, SWP_SHOWWINDOW);
        m_nHeight += DBLINESPACE;

        // Set Icon
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMICON), HWND_TOP, nLeft, m_nHeight, SMALLICON, SMALLICON, SWP_NOSIZE|SWP_SHOWWINDOW);
        m_nHeight += SMALLICON + LINESPACE;
        UINT nIcon = IDB_NOHD;
        if(m_hdrData.nStatus == HAVEHD) nIcon = IDB_HASHD;
        else if(m_hdrData.nStatus == REMOVEFAIL || m_hdrData.nStatus == INSPECTFAIL) nIcon = IDB_FAIL;
        else nIcon = IDB_NOHD;
        ::SendMessageW(::GetDlgItem(m_hWnd, IDC_ITEMICON),
            STM_SETIMAGE,
            IMAGE_BITMAP,
            (LPARAM)::LoadImageW(g_hInstance,
            MAKEINTRESOURCE(nIcon),
            IMAGE_BITMAP,
            SMALLICON,
            SMALLICON,
            LR_DEFAULTCOLOR));

        // Set info title
        nLeft += SMALLICON; nLeft += DBLINESPACE;
        nRight = m_nWidth - LINESPACE - MYMARGIN - DBLINESPACE;
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMTITLE), HWND_TOP, nLeft, MYMARGIN+2, (nRight-nLeft), SMALLICON, SWP_SHOWWINDOW);
        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMTITLE), m_hdrData.strTitle.c_str());
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMTITLE), WM_SETFONT, (WPARAM)g_fntBold, (LPARAM)TRUE);
        nLeft += LINESPACE;

        // Set info body
        rcClient.left = nLeft; rcClient.right = nRight;
        rcClient.top  = m_nHeight; rcClient.bottom = rcClient.top + 10;
        HGDIOBJ fntOld = SelectObject(hDC, (HGDIOBJ)g_fntNormal);
        ::DrawText(hDC, m_hdrData.strBody.c_str(), -1, &rcClient, DT_CALCRECT|DT_LEFT|DT_WORDBREAK);
        SelectObject(hDC, fntOld);
        ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMBODY), HWND_TOP, nLeft, m_nHeight, WIDTH(rcClient), HEIGHT(rcClient), SWP_SHOWWINDOW);
        ::SendMessage(::GetDlgItem(m_hWnd, IDC_ITEMBODY), WM_SETFONT, (WPARAM)g_fntNormal, (LPARAM)TRUE);
        ::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_ITEMBODY), m_hdrData.strBody.c_str());
        m_nHeight += DBLINESPACE + HEIGHT(rcClient);

        // Set Remove button
		if(m_hdrData.bBtnVisible && (HAVEHD==m_hdrData.nStatus || REMOVEFAIL==m_hdrData.nStatus))
        {
            nLeft = nRight - BUTTONCX-DBLINESPACE*2;
            nTop  = m_nHeight;
            ::SetWindowPos(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE), HWND_TOP, nLeft, nTop, BUTTONCX, BUTTONCY, SWP_SHOWWINDOW);
            m_nHeight += DBLINESPACE + BUTTONCY;
            if(REMOVEFAIL==m_hdrData.nStatus)
                ::EnableWindow(::GetDlgItem(m_hWnd, IDC_ITEMREMOVE), FALSE);
        }
    }

public:
    CHdrData     m_hdrData;
};

#endif