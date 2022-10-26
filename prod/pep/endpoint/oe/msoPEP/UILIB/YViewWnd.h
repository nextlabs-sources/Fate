/************************************************************************/
/* HDR UI                                                               */
/*     -- Gavin Ye                                                      */
/*        02/25/2008                                                    */
/************************************************************************/

#pragma once
#ifndef _Y_VIEW_WND_H_
#define _Y_VIEW_WND_H_

#include <assert.h>
#include <windows.h>
#include "smart_ptr.h"
#include "WinUtility.h"
#include "YWinBase.h"
#include "YViewBase.h"

#define VIEWWND_CLASSNAME               L"Y_VIEW_WND_CLASS"

class YScrollBar
{
public:
    YScrollBar(HWND hParent):m_hScroll(0),m_hParent(hParent),m_bVisible(FALSE)
    {
    }

    virtual ~YScrollBar()
    {
        if(m_hScroll) SendMessageW(m_hScroll, WM_CLOSE, 0, 0);
        m_hScroll=0; m_hParent = 0;
    }

    inline void put_Visible(BOOL bVisible){m_bVisible=bVisible;}//EnableWindow(m_hScroll, bVisible?TRUE:FALSE);ShowWindow(m_hScroll, bVisible?SW_SHOW:SW_HIDE);}
    inline BOOL get_Visible(){return m_bVisible;}
    inline HWND get_ScrollHwnd(){return m_hScroll;}
    static int  get_CYHScroll(){return GetSystemMetrics(SM_CYHSCROLL);}
    static int  get_CXVScroll(){return GetSystemMetrics(SM_CXVSCROLL);}

protected:
    HWND    m_hScroll;
    HWND    m_hParent;
    BOOL    m_bVisible;
};

class YHScrollBar : public YScrollBar
{
public:
    YHScrollBar(HWND hParent) : YScrollBar(hParent)
    {
        YLIB::WndRect parentWndRect(hParent);
        m_hScroll = CreateWindow(L"SCROLLBAR",
            L"",
            WS_CHILD|SBS_HORZ|SBS_BOTTOMALIGN,
            0, parentWndRect.m_rect.bottom-YScrollBar::get_CYHScroll(),
            parentWndRect.m_cx, YScrollBar::get_CYHScroll(),
            hParent, NULL, YBaseWnd::get_Instance(), NULL);
        EnableWindow(m_hScroll, FALSE);
    }
};

class YVScrollBar : public YScrollBar
{
public:
    YVScrollBar(HWND hParent) : YScrollBar(hParent)
    {
        YLIB::WndRect parentWndRect(hParent);
        m_hScroll = CreateWindow(L"SCROLLBAR",
            L"",
            WS_CHILD|SBS_VERT|SBS_RIGHTALIGN,
            0, parentWndRect.m_rect.bottom-YScrollBar::get_CXVScroll(),
            parentWndRect.m_cx, YScrollBar::get_CXVScroll(),
            hParent, NULL, YBaseWnd::get_Instance(), NULL);
        EnableWindow(m_hScroll, FALSE);
    }
};

class YScrollBars
{
public:
    YScrollBars():m_hParent(0)
    {
    }
    void Create(HWND hParent)
    {
        assert(hParent);
        spVScroll = smart_ptr<YVScrollBar>(new YVScrollBar(hParent));
        spHScroll = smart_ptr<YHScrollBar>(new YHScrollBar(hParent));
    }
    void RelocateScrollBars()
    {
        return;//if(NULL == m_hParent) return;
        YLIB::WndRect parentWndRect(m_hParent);
        if(spVScroll->get_Visible() && spHScroll->get_Visible())
        {
            MoveWindow(spVScroll->get_ScrollHwnd(),
                       parentWndRect.m_cx-YScrollBar::get_CXVScroll(), 0,
                       YScrollBar::get_CXVScroll(), parentWndRect.m_cy-YScrollBar::get_CYHScroll(), TRUE);
            MoveWindow(spHScroll->get_ScrollHwnd(),
                       0, parentWndRect.m_cy-YScrollBar::get_CYHScroll(), 
                       parentWndRect.m_cx-YScrollBar::get_CXVScroll(), YScrollBar::get_CYHScroll(), TRUE);
        }
        else if(spVScroll->get_Visible())
        {
            MoveWindow(spVScroll->get_ScrollHwnd(),
                parentWndRect.m_cx-YScrollBar::get_CXVScroll(), 0,
                YScrollBar::get_CXVScroll(), parentWndRect.m_cy, TRUE);
        }
        else if(spHScroll->get_Visible())
        {
            MoveWindow(spHScroll->get_ScrollHwnd(),
                0, parentWndRect.m_cy-YScrollBar::get_CYHScroll(), 
                parentWndRect.m_cx, YScrollBar::get_CYHScroll(), TRUE);
        }
        else
        {
            // do nothing
        }
    }
    smart_ptr<YVScrollBar> spVScroll;
    smart_ptr<YHScrollBar> spHScroll;
    HWND                   m_hParent;
};

template <class TCell>
class YViewWnd : public YBaseWnd,
    public YViewOffscreen<TCell>
{
public:
    YViewWnd():m_pScrollBars(0),m_hbWhite(NULL)
    {
        memset(&m_viewOffset, 0, sizeof(SIZE));
    }
    virtual ~YViewWnd()
    {
        if(m_hbWhite) DeleteObject(m_hbWhite);
        m_hbWhite = NULL;
    }

    MESSAGE_PROCESS_START()
        HANDLE_MESSAGE(WM_CREATE, OnCreate)
        HANDLE_MESSAGE(WM_LBUTTONDOWN, OnClickWnd)
        HANDLE_MESSAGE(WM_DESTROY, OnDestory)
        HANDLE_MESSAGE(WM_ERASEBKGND, OnErasBkGnd)
        HANDLE_MESSAGE(WM_PAINT, OnPaint)
        HANDLE_MESSAGE(WM_VSCROLL, OnVScroll)
        HANDLE_MESSAGE(WM_HSCROLL, OnHScroll)
        HANDLE_MESSAGE(WM_SIZE, OnSize)
        HANDLE_MESSAGE(WM_MOUSEWHEEL, OnMouseWheel)
        HANDLE_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        HANDLE_CLICK_MESSAGE(OnCtrlClick)
    MESSAGE_PROCESS_END()

public:
    inline HWND get_Hwnd(){return m_hWnd;}
    //inline void put_NormalFont(HFONT hFont){m_view.put_NormalFont(hFont);}
    //inline void put_BoldFont(HFONT hFont){m_view.put_BoldFont(hFont);}
    //inline void put_ButtonFont(HFONT hFont){m_view.put_ButtonFont(hFont);}

public:
    virtual BOOL Create(HWND hParent, RECT& rect)
    {
        BOOL bRet = YBaseWnd::Create(VIEWWND_CLASSNAME, L"", rect, NULL, WS_EX_CLIENTEDGE, WS_CHILD|WS_VISIBLE, hParent);
        if(!bRet || NULL==m_hWnd) return FALSE;
        put_ViewHwnd(m_hWnd);
        prepareDC();
        m_pScrollBars = new YScrollBars;
        m_pScrollBars->Create(m_hWnd);
        return TRUE;
    }

    void CleanView()
    {
        memset(&m_viewOffset, 0, sizeof(SIZE));
        m_offscreenSize.cx=0; m_offscreenSize.cy=0;
    }

    void UpdateView()
    {
        YLIB::WndRect   wndRect(m_hWnd);
        UpdateVScroll();
        m_pScrollBars->RelocateScrollBars();
        InvalidateRect(m_hWnd, &(wndRect.m_rect), TRUE);
        UpdateWindow(m_hWnd);
    }

protected:
    virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;
    }
    virtual LRESULT OnClickWnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;
    }
    virtual LRESULT OnDestory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        bCalldefaultHandler = FALSE;
        if(m_pScrollBars) delete m_pScrollBars;
        return 0L;
    }
    virtual LRESULT OnErasBkGnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
       bCalldefaultHandler = FALSE;
       return 0L;
   }
    virtual LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;
    }
    virtual LRESULT OnCtrlClick(HWND hWnd, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;
    }
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        bCalldefaultHandler = FALSE;
        ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        if(m_offscreenSize.cy > HIWORD(lParam)) m_pScrollBars->spVScroll->put_Visible(TRUE);
        if(m_offscreenSize.cx > LOWORD(lParam)) m_pScrollBars->spHScroll->put_Visible(TRUE);
        if(m_pScrollBars) m_pScrollBars->RelocateScrollBars();
        return 0L;
    }
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        PAINTSTRUCT ps;
        HDC         hDC     = 0;
        SIZE        viewSize;
        YLIB::WndRect   wndRect(m_hWnd);

        // Begin Paint
        bCalldefaultHandler = FALSE;
        hDC = BeginPaint(m_hWnd, &ps);

        // Do your painting here
        // update scroll bar first
        UpdateScrollBar();

        // check view size
        if(0==m_offscreenDC || 0==m_offscreenBitmap) goto _exit;
        if(0==m_offscreenSize.cx || 0==m_offscreenSize.cy) goto _exit;
        wndRect.Resize(m_pScrollBars->spVScroll->get_Visible()?(0-YScrollBar::get_CXVScroll()):0,
                       m_pScrollBars->spHScroll->get_Visible()?(0-YScrollBar::get_CYHScroll()):0);
        if(0==wndRect.m_cx || 0==wndRect.m_cy) goto _exit;
        viewSize.cx = m_offscreenSize.cx - m_viewOffset.cx;
        viewSize.cy = m_offscreenSize.cy - m_viewOffset.cy;
        if(0==viewSize.cx || 0==viewSize.cy) goto _exit;

        BitBlt(hDC, 0, 0, min(wndRect.m_cx, viewSize.cx), min(wndRect.m_cy, viewSize.cy), m_offscreenDC, m_viewOffset.cx, m_viewOffset.cy, SRCCOPY);

_exit:
        // End Paint
        EndPaint(m_hWnd, &ps);
        return (LRESULT)TRUE;
    }

    void    UpdateScrollBar()
    {
        RECT rcClient; memset(&rcClient, 0, sizeof(RECT));
        GetClientRect(m_hWnd, &rcClient);
        SIZE sizeClient; sizeClient.cx=rcClient.right-rcClient.left; sizeClient.cy=rcClient.bottom-rcClient.top;
        BOOL bNewVScroll=FALSE, bNewHScroll=FALSE;

        int nViewHeight = m_offscreenSize.cy;
        if(nViewHeight > sizeClient.cy)
            bNewVScroll = TRUE;
        //int nWidth = m_offscreenSize.cx;
        //if(nWidth > sizeClient.cx)
        //    bNewHScroll = TRUE;

        if(bNewHScroll!=m_pScrollBars->spHScroll->get_Visible() || bNewVScroll!=m_pScrollBars->spVScroll->get_Visible())
        {
            if(TRUE==bNewHScroll && TRUE==bNewVScroll)
            {
                MoveWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), rcClient.right-YScrollBar::get_CXVScroll(), 0, YScrollBar::get_CXVScroll(), sizeClient.cy-YScrollBar::get_CYHScroll(), TRUE);
                MoveWindow(m_pScrollBars->spHScroll->get_ScrollHwnd(), 0, rcClient.bottom-YScrollBar::get_CYHScroll(), sizeClient.cx-YScrollBar::get_CXVScroll(), YScrollBar::get_CYHScroll(), TRUE);
            }
            else
            {
                if(TRUE==bNewVScroll)
                    MoveWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), rcClient.right-YScrollBar::get_CXVScroll(), 0, YScrollBar::get_CXVScroll(), sizeClient.cy, TRUE);
                else
                    MoveWindow(m_pScrollBars->spHScroll->get_ScrollHwnd(), 0, rcClient.bottom-YScrollBar::get_CYHScroll(), sizeClient.cx, YScrollBar::get_CYHScroll(), TRUE);
            }
            EnableWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), bNewVScroll?TRUE:FALSE);
            ShowWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), bNewVScroll?SW_SHOWNORMAL:SW_HIDE);
            m_pScrollBars->spVScroll->put_Visible(bNewVScroll);
            EnableWindow(m_pScrollBars->spHScroll->get_ScrollHwnd(), bNewHScroll?TRUE:FALSE);
            ShowWindow(m_pScrollBars->spHScroll->get_ScrollHwnd(), bNewHScroll?SW_SHOWNORMAL:SW_HIDE);
            m_pScrollBars->spHScroll->put_Visible(bNewHScroll);
            UpdateVScroll();
        }
    }
    LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        LRESULT lResult = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        bCalldefaultHandler = FALSE;

        int  nSbCode = LOWORD(wParam);
        int  nCurPos = HIWORD(wParam);
        YLIB::WndRect   wndRect(m_hWnd);

        switch(nSbCode)
        {
        case SB_TOP:
            m_viewOffset.cy = 0;
            break;
        case SB_BOTTOM:
            m_viewOffset.cy = m_offscreenSize.cy-wndRect.m_cy+1;
            break;
        case SB_PAGEDOWN:
            m_viewOffset.cy += m_pScrollBars->spVScroll->get_Visible()?(wndRect.m_cy-YScrollBar::get_CYHScroll()):wndRect.m_cy;
            if(m_viewOffset.cy>m_offscreenSize.cy-wndRect.m_cy) m_viewOffset.cy=(m_offscreenSize.cy-wndRect.m_cy+1);
            break;
        case SB_PAGEUP:
            m_viewOffset.cy -= m_pScrollBars->spVScroll->get_Visible()?(wndRect.m_cy-YScrollBar::get_CYHScroll()):wndRect.m_cy;
            if(m_viewOffset.cy<0) m_viewOffset.cy=0;
            break;
        case SB_LINEDOWN:
            m_viewOffset.cy += 20;
            if(m_viewOffset.cy>m_offscreenSize.cy-wndRect.m_cy) m_viewOffset.cy=(m_offscreenSize.cy-wndRect.m_cy+1);
            break;
        case SB_LINEUP:
            m_viewOffset.cy -= 20;
            if(m_viewOffset.cy<0) m_viewOffset.cy=0;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            m_viewOffset.cy = nCurPos;
            break;
        case SB_ENDSCROLL:
            break;
        default:
            break;
        }

        UpdateVScroll();
        RelocateCtrls(m_viewOffset.cx, m_viewOffset.cy);
        InvalidateRect(m_hWnd, &(wndRect.m_rect), TRUE);
        UpdateWindow(m_hWnd);

        return lResult;
    }
    void UpdateVScroll()
    {
        RECT rcClient; GetClientRect(m_hWnd, &rcClient);
        SIZE clientSize = {rcClient.right-rcClient.left, rcClient.bottom-rcClient.top};
        if(m_offscreenSize.cy > clientSize.cy)
        {
            if(!IsWindowEnabled(m_pScrollBars->spVScroll->get_ScrollHwnd()))
            {
                EnableWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), TRUE);
                ShowWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), SW_SHOW);
            }

            int nMin = 0;
            int nMax = m_offscreenSize.cy;
            int nPage= clientSize.cy;
            int nPos = m_viewOffset.cy;

            // get current scrollbar values
            SCROLLINFO si;
            ZeroMemory(&si,sizeof(SCROLLINFO));
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask = SIF_ALL;
            GetScrollInfo(m_pScrollBars->spVScroll->get_ScrollHwnd(), SB_CTL, &si);

            if ((nMin != si.nMin)
                || (nMax != si.nMax)
                || (nPage != (int)si.nPage)
                || (nPos != si.nPos))
            {
                si.nMin = nMin;
                si.nMax = nMax;
                si.nPage= nPage;
                si.nPos = nPos;
                SetScrollInfo(m_pScrollBars->spVScroll->get_ScrollHwnd(), SB_CTL, &si, TRUE);
            }
        }
        else
        {
            if(IsWindowEnabled(m_pScrollBars->spVScroll->get_ScrollHwnd()))
            {
                EnableWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), FALSE);
                ShowWindow(m_pScrollBars->spVScroll->get_ScrollHwnd(), SW_HIDE);
            }
        }
    }
    LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        LRESULT lResult = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        bCalldefaultHandler = FALSE;

        int nSbCode = LOWORD(wParam);
        int nCurPos = HIWORD(wParam);
        YLIB::WndRect   wndRect(m_hWnd);

        switch(nSbCode)
        {
        case SB_LEFT:
            m_viewOffset.cx = 0;
            break;
        case SB_RIGHT:
            m_viewOffset.cx = m_offscreenSize.cx;
            break;
        case SB_PAGELEFT:
            m_viewOffset.cx -= m_pScrollBars->spHScroll->get_Visible()?(wndRect.m_cx-YScrollBar::get_CXVScroll()):wndRect.m_cx;
            if(m_viewOffset.cx<0) m_viewOffset.cx=0;
            break;
        case SB_PAGERIGHT:
            m_viewOffset.cx += m_pScrollBars->spHScroll->get_Visible()?(wndRect.m_cx-YScrollBar::get_CXVScroll()):wndRect.m_cx;
            if(m_viewOffset.cx>m_offscreenSize.cx) m_viewOffset.cx=m_offscreenSize.cx;
            break;
        case SB_LINELEFT:
            m_viewOffset.cx -= 20;
            if(m_viewOffset.cx<0) m_viewOffset.cx=0;
            break;
        case SB_LINERIGHT:
            m_viewOffset.cx += 20;
            if(m_viewOffset.cx>m_offscreenSize.cx) m_viewOffset.cx=m_offscreenSize.cx;
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            m_viewOffset.cx = nCurPos;
            break;
        case SB_ENDSCROLL:
            break;
        default:
            break;
        }

        return lResult;
    }
    LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bCalldefaultHandler)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bCalldefaultHandler);
        return 0L;
    }
protected:
    SIZE         m_viewOffset;

private:
    YScrollBars* m_pScrollBars;
    HBRUSH       m_hbWhite;
};


#endif