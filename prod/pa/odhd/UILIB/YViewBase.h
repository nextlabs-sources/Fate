



#pragma once

#ifndef _Y_VIEW_BASE_H_

#define _Y_VIEW_BASE_H_

#include <vector>

#include "WinUtility.h"

#include "smart_ptr.h"





template<class TCell>

class YViewOffscreen

{

public:

    YViewOffscreen():m_hViewWnd(0),m_offscreenDC(0),m_offscreenBitmap(0),m_hOldBitmap(0)

    {

        m_offscreenSize.cx=0; m_offscreenSize.cy=0;

    }

    YViewOffscreen(HWND hViewWnd):m_hViewWnd(hViewWnd),m_offscreenDC(0),m_offscreenBitmap(0),m_hOldBitmap(0)

    {

        m_offscreenSize.cx=0; m_offscreenSize.cy=0;

    }

    virtual ~YViewOffscreen()

    {

        DeleteView();

        prepareDC();

    }



    inline void put_ViewHwnd(HWND hViewWnd){m_hViewWnd = hViewWnd;}



public:

    BOOL PrepareView()

    {

        assert(m_hViewWnd);

        for(std::vector<YLIB::smart_ptr<TCell>>::iterator it=m_cells.begin();

            it!=m_cells.end(); ++it)

        {

            int cx=0, cy=0;

            YLIB::smart_ptr<TCell> spCell = *it;

            if(0==spCell.get()) continue;

            cy = spCell->prepare_Cell(m_offscreenSize.cy);

            cx = spCell->get_Width();

            if(cx>0 && cy>0)

            {

                m_offscreenSize.cx = max(m_offscreenSize.cx, cx);

                m_offscreenSize.cy += cy;

            }

        }

        return TRUE;

    }



    BOOL CreateView(COLORREF color = RGB(255,255,255))

    {

        assert(m_hViewWnd);

        YLIB::WindowDC  parentDC(m_hViewWnd);

        if(0==m_offscreenSize.cx||0==m_offscreenSize.cy) return FALSE;

        if(NULL==m_offscreenDC && FALSE==prepareDC()) return FALSE;



        m_offscreenBitmap = CreateCompatibleBitmap(parentDC.get_DC(), m_offscreenSize.cx, m_offscreenSize.cy);

        if(NULL==m_offscreenBitmap)

        {

            memset(&m_offscreenSize, 0, sizeof(SIZE));

            return FALSE;

        }

        m_hOldBitmap = (HBITMAP)SelectObject(m_offscreenDC, m_offscreenBitmap);

        YLIB::YPaint::FillRectWithColor(m_offscreenDC, color, m_offscreenSize.cx, m_offscreenSize.cy);



        renderView();

        RelocateCtrls(0, 0);



        return TRUE;

    }



    void DeleteView()

    {
	if(NULL==m_hViewWnd) return;
        assert(m_hViewWnd);

        m_cells.clear();

        if(m_offscreenDC)

        {

            if(m_hOldBitmap) SelectObject(m_offscreenDC, (HGDIOBJ)m_hOldBitmap);

            if(m_offscreenBitmap) DeleteObject(m_offscreenBitmap); m_offscreenBitmap=0;

        }

        memset(&m_offscreenSize, 0, sizeof(SIZE));

    }



    void AddViewCell(YLIB::smart_ptr<TCell> spCell)

    {

        assert(m_hViewWnd);

        assert(0!=spCell.get());

        m_cells.push_back(spCell);

    }



    DWORD FindCellCtrl(HWND hWnd)

    {

        WORD wCellIndex = 0;

        WORD wCtrlIndex = 0xFFFF;

        DWORD dwRet = 0xFFFFFFFF;

        for(std::vector<YLIB::smart_ptr<TCell>>::iterator it=m_cells.begin();

            it!=m_cells.end(); ++it)

        {

            YLIB::smart_ptr<TCell> spCell = *it;

            wCtrlIndex = spCell->find_cellctrl(hWnd);

            if(0xFFFF!=wCtrlIndex)

            {

                dwRet = wCellIndex; dwRet <<= 16;

                dwRet |= wCtrlIndex;

                break;

            }

            wCellIndex++;

        }

        return dwRet;

    }



protected:

    BOOL prepareDC()

    {

        YLIB::WindowDC  wndDC(m_hViewWnd);

        if(NULL!=m_offscreenDC) return TRUE;

        m_offscreenDC = CreateCompatibleDC(wndDC.get_DC());

        return (NULL!=m_offscreenDC)?TRUE:FALSE;

    }



    BOOL releaseDC()

    {

        if(m_offscreenDC) ReleaseDC(m_hViewWnd, m_offscreenDC); m_offscreenDC = NULL;

    }



    void renderView()

    {

        for(std::vector<YLIB::smart_ptr<TCell>>::iterator it=m_cells.begin();

            it!=m_cells.end(); ++it)

        {

            YLIB::smart_ptr<TCell> spCell = *it;

            if(0!=spCell.get()) spCell->render_Cell();

        }

    }



    void RelocateCtrls(int cxOffset, int cyOffset)

    {

        for(std::vector<YLIB::smart_ptr<TCell>>::iterator it=m_cells.begin();

            it!=m_cells.end(); ++it)

        {

            YLIB::smart_ptr<TCell> spCell = *it;

            if(0!=spCell.get()) spCell->locate_Cell(cxOffset, cyOffset);

        }

    }



public:

    HDC     m_offscreenDC;

    HBITMAP m_offscreenBitmap;

    SIZE    m_offscreenSize;

    HWND    m_hViewWnd;

    HBITMAP m_hOldBitmap;

    std::vector<YLIB::smart_ptr<TCell>> m_cells;

};



#endif