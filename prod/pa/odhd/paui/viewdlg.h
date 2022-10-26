


#ifndef _VIEW_DLG_H_
#define _VIEW_DLG_H_
#include <atlbase.h>
#pragma warning(push)
#pragma warning(disable: 6387 6386)
#include <atlwin.h>
#pragma warning(pop)
#include <GdiPlus.h>
#include "uiglobal.h"
#include "resource.h"
#include "winhelp.h"
#include "viewdata.h"
#include "textctrl.h"
#include "smart_ptr.h"

class CViewDlg : public CDialogImpl<CViewDlg>,
	public CCtrlColor<CViewDlg, RGB(30,30,30), RGB(255,255,255)>
{
public:
	typedef CCtrlColor<CViewDlg, RGB(30,30,30), RGB(255,255,255)> CCtrlColorBase;
	enum { IDD = IDD_PAVIEWDLG };

    CViewDlg()
    {
        m_bVScrollVisible = FALSE;
        m_bHScrollVisible = FALSE;
        m_nVScrollMax     = 0;
        m_nHScrollMax     = 0;
        m_nVScrollPos     = 0;
        m_nHScrollPos     = 0;
        m_nViewSize.cx    = 0;
        m_nViewSize.cy    = 0;
    }

	BEGIN_MSG_MAP(CViewDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        MESSAGE_HANDLER(WM_ITEMREMOVE, OnItemRemove)
		CHAIN_MSG_MAP(CCtrlColorBase)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

public:
	void ResetView()
	{
		RECT  rcWin;

		// Get Current Win attributes
        ::GetClientRect(m_hWnd, &rcWin);

		// Compare        
		if(HEIGHT(rcWin) < m_nViewSize.cy)
		{
			m_nVScrollMax     = m_nViewSize.cy;
			if(!m_bVScrollVisible)
			{
                m_bVScrollVisible = TRUE;
                ::ShowScrollBar(m_hWnd, SB_VERT, m_bVScrollVisible);
			}
		}
		else
		{
			if(m_bVScrollVisible)
			{
				m_nVScrollMax     = 0;
				m_nVScrollPos     = 0;
                ScrollWindow(0, 0);
                m_bVScrollVisible = FALSE;
                ::ShowScrollBar(m_hWnd, SB_VERT, m_bVScrollVisible);
			}
		}
		if(WIDTH(rcWin) < m_nViewSize.cx)
		{
			m_nHScrollMax     = m_nViewSize.cx;
			if(!m_bHScrollVisible)
			{
                m_bHScrollVisible = TRUE;
                ::ShowScrollBar(m_hWnd, SB_HORZ, m_bHScrollVisible);
			}
		}
		else
		{
			if(m_bHScrollVisible)
			{
				m_nHScrollMax     = 0;
				m_nHScrollPos     = 0;
                ScrollWindow(0, 0);
                m_bHScrollVisible = FALSE;
                ::ShowScrollBar(m_hWnd, SB_HORZ, m_bHScrollVisible);
			}
		}

		// Set VScrollBar
		SCROLLINFO si;
		si.cbSize= sizeof(SCROLLINFO); 
		si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS; 
		si.nMin  = 0; 
        si.nMax  = m_nVScrollMax;//(0!=m_nVScrollMax)?(m_nVScrollMax+HEIGHT(rcWin)):0; 
		si.nPage = HEIGHT(rcWin); 
		si.nPos  = 0; 
		SetScrollInfo(SB_VERT, &si, TRUE);
		si.cbSize= sizeof(SCROLLINFO); 
		si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS; 
		si.nMin  = 0; 
		si.nMax  = m_nHScrollMax; 
		si.nPage = WIDTH(rcWin); 
		si.nPos  = 0; 
		SetScrollInfo(SB_HORZ, &si, TRUE);

        ShowWindow(SW_HIDE);
        ShowWindow(SW_SHOW);
    }

    void CleanView()
    {
        for(VIEWITEMVECTOR::iterator it=m_vecItem.begin(); it!=m_vecItem.end(); ++it)
        {
            (*it)->DestroyWindow();
        }
        m_vecItem.clear();
        m_nViewSize.cy = 0;
        ResetView();
    }

    BOOL AddItem(CHdrData& hdrData, int iCatIndex)
    {
        CViewItemDlg* spViewItem = new CHdrItemDlg(hdrData);
        m_vecItem.push_back(spViewItem);
        int i = (int)m_vecItem.size() - 1;
        m_vecItem[i]->m_nIndex = i;
		m_vecItem[i]->m_nCatIndex = iCatIndex;
        m_vecItem[i]->Create(m_hWnd, 0);
        int nItemHeight = m_vecItem[i]->GetRealHeight();
        m_vecItem[i]->SetWindowPos(HWND_TOP, CViewItemDlg::m_nX, m_nViewSize.cy, CViewItemDlg::m_nWidth, nItemHeight, SWP_SHOWWINDOW);
        m_nViewSize.cy += nItemHeight;
        return TRUE;
    }

    void ResetViewItem(int iCatIndex, LPCWSTR pwzTitle, LPCWSTR pwzBody, int nStatus)
    {
        int nItemHeight = 0;
        CViewItemDlg*  pViewItem = NULL;
        
        m_nViewSize.cy    = 0;
        for (VIEWITEMVECTOR::iterator it=m_vecItem.begin(); it!=m_vecItem.end(); ++it)
        {
            pViewItem = *it;

			if (pViewItem->m_nCatIndex == iCatIndex)
				pViewItem->ResetData(pwzTitle, pwzBody, nStatus);

            nItemHeight = pViewItem->GetRealHeight();
            pViewItem->SetWindowPos(HWND_TOP, CViewItemDlg::m_nX, m_nViewSize.cy, CViewItemDlg::m_nWidth, nItemHeight, SWP_SHOWWINDOW);
            m_nViewSize.cy += nItemHeight;
        }
        ResetView();
    }

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
        ::ShowScrollBar(m_hWnd, SB_BOTH, FALSE);
		m_bVScrollVisible = m_bHScrollVisible = FALSE;
		m_nVScrollMax = m_nHScrollMax = m_nVScrollPos = m_nHScrollPos = m_nViewSize.cx = m_nViewSize.cy = 0;
        m_nViewSize.cx = CViewItemDlg::m_nWidth;
        m_nViewSize.cy = 0;
		
		return TRUE;
    }
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        for (VIEWITEMVECTOR::iterator it=m_vecItem.begin(); it!=m_vecItem.end(); ++it)
        {
            (*it)->DestroyWindow();
        }
        m_vecItem.clear();
        return 0L;
    }
    LRESULT OnItemRemove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HWND hParent = ::GetParent(m_hWnd);
        ::SendMessage(hParent, WM_ITEMREMOVE, wParam, lParam);
        return 0L;
    }
    LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        RECT  rcWin;
        GetClientRect(&rcWin);
        LRESULT lRet = 0;
        UINT nSBCode = (UINT)LOWORD(wParam); 
        UINT nPos    = (UINT)HIWORD(wParam); 
        
		SCROLLINFO scrollinfo;
        memset(&scrollinfo, 0, sizeof(SCROLLINFO));
        scrollinfo.cbSize = sizeof(SCROLLINFO);
        scrollinfo.fMask  = SIF_ALL;
        CDialogImpl::GetScrollInfo(SB_VERT, &scrollinfo);
		switch (nSBCode)
		{ 
		case SB_BOTTOM: 
			ScrollWindow(0, (scrollinfo.nPos-scrollinfo.nMax)); 
			scrollinfo.nPos = scrollinfo.nMax; 
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL); 
			break; 
		case SB_TOP: 
			ScrollWindow(0, (scrollinfo.nPos-scrollinfo.nMin)); 
            scrollinfo.nPos = scrollinfo.nMin;
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
			break; 
		case SB_LINEUP: 
			scrollinfo.nPos -= 10; 
			if (scrollinfo.nPos<0)
			{ 
                scrollinfo.nPos = scrollinfo.nMin; 
                SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
				break; 
            }
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
            ScrollWindow(0, 10);
			break; 
		case SB_LINEDOWN: 
			scrollinfo.nPos += 10; 
			if (scrollinfo.nPos>scrollinfo.nMax-HEIGHT(rcWin)) 
			{ 
                scrollinfo.nPos = scrollinfo.nMax; 
                SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
				break; 
            } 
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
			ScrollWindow(0,-10); 
			break; 
        case SB_PAGEUP:
            scrollinfo.nPos -= HEIGHT(rcWin); 
            if (scrollinfo.nPos<0)
            { 
                ScrollWindow(0, HEIGHT(rcWin)+scrollinfo.nPos);
                scrollinfo.nPos = scrollinfo.nMin; 
                SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
                break; 
            }
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
            ScrollWindow(0, HEIGHT(rcWin));
			break; 
        case SB_PAGEDOWN: 
            scrollinfo.nPos += HEIGHT(rcWin); 
            if (scrollinfo.nPos>scrollinfo.nMax-HEIGHT(rcWin)) 
            {
                int nSpec = scrollinfo.nPos - (scrollinfo.nMax-HEIGHT(rcWin));
                ScrollWindow(0,nSpec-HEIGHT(rcWin)); 
                scrollinfo.nPos = scrollinfo.nMax; 
                SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
                break; 
            } 
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
            ScrollWindow(0,-(HEIGHT(rcWin)));
			break; 
		case SB_ENDSCROLL:
			break; 
		case SB_THUMBPOSITION: 
        case SB_THUMBTRACK: 
            ScrollWindow(0, (scrollinfo.nPos-nPos)); 
            scrollinfo.nPos = nPos;
            SetScrollInfo(SB_VERT, &scrollinfo, SIF_ALL);
			break; 
		}
		return lRet;
	}
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        LRESULT lRet = 0L;
        UINT nSBCode = (UINT)LOWORD(wParam); 
        UINT nPos    = (UINT)HIWORD(wParam); 
        
        SCROLLINFO scrollinfo;
        memset(&scrollinfo, 0, sizeof(SCROLLINFO));
        scrollinfo.cbSize = sizeof(SCROLLINFO);
        scrollinfo.fMask  = SIF_ALL;
		GetScrollInfo(SB_HORZ, &scrollinfo);
		switch (nSBCode)
		{ 
		case SB_LEFT:
			ScrollWindow((scrollinfo.nPos-scrollinfo.nMin), 0); 
			scrollinfo.nPos = scrollinfo.nMin; 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			break; 
		case SB_RIGHT: 
			ScrollWindow((scrollinfo.nPos-scrollinfo.nMax), 0); 
			scrollinfo.nPos = scrollinfo.nMax; 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			break; 
		case SB_LINELEFT: 
			scrollinfo.nPos -= 1; 
			if (scrollinfo.nPos<0)
			{ 
				scrollinfo.nPos = scrollinfo.nMin; 
				break; 
			} 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			ScrollWindow(10,0); 
			break; 
		case SB_LINERIGHT: 
			scrollinfo.nPos += 1; 
			if (scrollinfo.nPos>scrollinfo.nMax) 
			{ 
				scrollinfo.nPos = scrollinfo.nMax; 
				break; 
			} 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			ScrollWindow(-10,0); 
			break; 
		case SB_PAGELEFT: 
			scrollinfo.nPos -= 5; 
			if (scrollinfo.nPos<0)
			{ 
				scrollinfo.nPos = scrollinfo.nMin; 
				break; 
			} 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			ScrollWindow(5, 0); 
			break; 
		case SB_PAGERIGHT: 
			scrollinfo.nPos += 5; 
			if (scrollinfo.nPos>scrollinfo.nMax) 
			{ 
				scrollinfo.nPos = scrollinfo.nMax; 
				break; 
			} 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			ScrollWindow(-5, 0); 
			break; 
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK: 
			ScrollWindow((scrollinfo.nPos-nPos), 0); 
			scrollinfo.nPos = nPos; 
			SetScrollInfo(SB_HORZ,&scrollinfo,SIF_ALL); 
			break; 
		case SB_ENDSCROLL: 
			break; 
		}
		return lRet;
	}
protected:
private:
    BOOL    m_bVScrollVisible;
    BOOL    m_bHScrollVisible;
	int		m_nVScrollMax;
	int		m_nHScrollMax;
	int		m_nVScrollPos;
	int		m_nHScrollPos;
	SIZE	m_nViewSize;

    VIEWITEMVECTOR  m_vecItem;
};


#endif