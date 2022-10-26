

#pragma once
#ifndef _Y_VIEW_CTRLS_H_
#define _Y_VIEW_CTRLS_H_

class YViewControl
{
public:
    YViewControl(HANDLE hParent, HFONT hFont):m_hWnd(NULL),m_hParent(hParent),m_hFont(hFont)
    {
        memset(&m_rect, 0, sizeof(RECT));
        CreateControl();
    }
    virtual ~YViewControl()
    {
        if(m_hWnd) SendMessageW(m_hWnd, WM_CLOSE, 0, 0);
    }

    inline HWND get_hWnd(){return m_hWnd;}
    inline 

protected:
    virtual void CreateControl() = 0;

private:
    HANDLE  m_hParent;
    HANDLE  m_hWnd;
    HFONT   m_hFont;
    RECT    m_rect;
    std::wstring m_strName;
};

class YViewButton : public YViewControl
{
public:
    YViewButton(HANDLE hParent, HFONT hFont):YViewControl(hParent, hFont){}
protected:
     void CreateControl()
     {
         m_hWnd = CreateWindow(L"BUTTON", L"Remove", WS_CHILD,
                                 0,
                                 0,
                                 1,
                                 1,
                                 m_hParent, NULL, g_hInst, NULL);
         if(m_hFont) SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);
     }
};

class YViewCheckBox : public YViewControl
{
public:
    YViewButton(HANDLE hParent, HFONT hFont):YViewControl(hParent, hFont){}
protected:
    void CreateControl()
    {
        m_hWnd = CreateWindow(L"BUTTON", L"", WS_CHILD|BS_CHECKBOX,
            0,
            0,
            1,
            1,
            m_hParent, NULL, g_hInst, NULL);
        if(m_hFont) SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);
    }
};


#endif