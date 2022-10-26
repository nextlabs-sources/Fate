

/************************************************************************/

/* HDR UI                                                               */

/*     -- Gavin Ye                                                      */

/*        02/25/2008                                                    */

/************************************************************************/

#pragma once

#ifndef _WIN_UTILITY_H_

#define _WIN_UTILITY_H_

#include <assert.h>



namespace YLIB

{

    class YWinUtility

    {

    public:

        static void GetScreenSize(SIZE* pSize)

        {

            pSize->cx = GetSystemMetrics(SM_CXMAXIMIZED);

            pSize->cy = GetSystemMetrics(SM_CYMAXIMIZED);

        }



        static void CenterWindows(RECT& rc)

        {

            SIZE size; memset(&size, 0, sizeof(SIZE));

            GetScreenSize(&size);

            rc.left = (size.cx - (rc.right-rc.left))/2;

            rc.top  = (size.cy - (rc.bottom-rc.top))/2;

            rc.right  += rc.left;

            rc.bottom += rc.top;

        }



        static int CountTextAreaHeightByWidth(LPCWSTR pwzText, HDC hDC, HFONT hFont, int width, int maxheight)

        {

            RECT    rc    = {0, 0, width, 1000};

            HFONT   hOldFont = NULL;
#pragma warning(push)
#pragma warning(disable:6387)
            if(NULL==hFont) hOldFont=(HFONT)SelectObject(hDC, hFont);
#pragma warning(pop)
            DrawTextEx(hDC, (LPWSTR)pwzText, -1, &rc,

                       DT_CALCRECT|DT_LEFT|DT_NOPREFIX|DT_WORDBREAK, NULL);

            if(NULL!=hOldFont) SelectObject(hDC, hOldFont);

            if(maxheight && rc.bottom>maxheight) rc.bottom=maxheight;

            return (int)rc.bottom;

        }

        static int CountTextAreaHeightByWidthNoWdBrk(LPCWSTR pwzText, HDC hDC, HFONT hFont, int width, int maxheight)

        {

            RECT    rc    = {0, 0, width, 1000};

            HFONT   hOldFont = NULL;
#pragma warning(push)
#pragma warning(disable:6387)
            if(NULL==hFont) hOldFont=(HFONT)SelectObject(hDC, hFont);
#pragma warning(pop)
            DrawTextEx(hDC, (LPWSTR)pwzText, -1, &rc,

                DT_CALCRECT|DT_LEFT|DT_NOPREFIX, NULL);

            if(NULL!=hOldFont) SelectObject(hDC, hOldFont);

            if(maxheight && rc.bottom>maxheight) rc.bottom=maxheight;

            return (int)rc.bottom;

        }

    };



    class WindowDC

    {

    public:

        WindowDC(HWND hWnd):m_hWnd(hWnd)

        {

            assert(hWnd);

            m_hDC = 0;

            m_hDC = GetWindowDC(hWnd);

        }

        ~WindowDC()

        {

            if(m_hDC) ReleaseDC(m_hWnd, m_hDC);

        }

        inline HDC get_DC(){return m_hDC;}

    private:

        HWND    m_hWnd;

        HDC     m_hDC;

    };



    class YPaint

    {

    public:

        static void FillRectWithColor(HDC hDC, COLORREF color, int cx, int cy)

        {

            HBRUSH hb = CreateSolidBrush(color);

            RECT rcBitmap; memset(&rcBitmap, 0, sizeof(RECT));

            rcBitmap.right  = cx;

            rcBitmap.bottom = cy;

            FillRect(hDC, &rcBitmap, hb);

            DeleteObject(hb);

        }

    };



    class WndRect

    {

    public:

        WndRect(HWND hWnd):m_cx(0),m_cy(0)

        {

            memset(&m_rect, 0, sizeof(RECT));

            if(hWnd) GetClientRect(hWnd, &m_rect);

            m_cx = m_rect.right - m_rect.left;

            m_cy = m_rect.bottom- m_rect.top;

        }

        void Resize(int cx, int cy)

        {

            m_cx += cx; if(0>m_cx) m_cx=0;

            m_cy += cy; if(0>m_cy) m_cy=0;

            m_rect.right = m_rect.left + m_cx;

            m_rect.bottom= m_rect.top + m_cy;

        }

        RECT    m_rect;

        int     m_cx;

        int     m_cy;

    };

}



#endif