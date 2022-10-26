



#pragma once

#ifndef _Y_VIEW_CELL_H_

#define _Y_VIEW_CELL_H_

#include "WinUtility.h"



class YViewCell

{

public:

    YViewCell(HWND hViewWnd, HDC hViewDC, int nWidth):m_hViewDC(hViewDC),m_hViewWnd(hViewWnd),m_nWidth(nWidth),m_nOffset(0)

    {

        assert(hViewWnd);

        assert(hViewDC);

    }

    virtual ~YViewCell()

    {

    }



public:

    virtual int  get_Height() = 0;

    virtual int  get_Width() = 0;

    virtual int  prepare_Cell(int cyOffset) = 0;

    virtual void render_Cell() = 0;

    virtual int  locate_Cell(int cxOffset, int cyOffset) = 0;

    virtual WORD find_cellctrl(HWND hWnd) = 0;

    virtual int get_ExpandHeight()
    {
        return m_nHeight;
    }



protected:

    HDC     m_hViewDC;

    HWND    m_hViewWnd;

    int     m_nOffset;

    int     m_nWidth;

    int     m_nHeight;

};



#endif