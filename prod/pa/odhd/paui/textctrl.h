

#ifndef _TEXT_CTRL_H_
#define _TEXT_CTRL_H_
//#include <atlbase.h>
//#include <atlwin.h>
//#include <GdiPlus.h>
//#include <string>
//
//#include <atlapp.h>
//#include <atlctrls.h>
//#include <atlctrlx.h>
//#include <atlctrlw.h>
//#include <atldlgs.h>
//#include <atlmisc.h>
//
//class CStaticExImpl : public CWindowImpl<CStaticExImpl, CStatic>
//{
//public:
//    CStaticExImpl()
//    {
//        m_nWeight  = 14;
//        m_strFamily= L"Arial"; //L"Verdana"
//        m_color    = RGB(30, 30, 30);
//        m_bold     = FALSE;
//        m_italy    = FALSE;
//        m_underline= FALSE;
//    }
//
//    BEGIN_MSG_MAP(CStaticExImpl)
//        MESSAGE_HANDLER(WM_PAINT, OnPaint)
//        MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
//        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
//        DEFAULT_REFLECTION_HANDLER()
//    END_MSG_MAP()
//
//public:
//    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        RECT rc; GetClientRect(&rc);
//        HDC hDC = ::GetWindowDC(m_hWnd);
//
//        Gdiplus::FontFamily myfontFamily(m_strFamily.c_str());
//        Gdiplus::Font myfont(&myfontFamily, (Gdiplus::REAL)m_nWeight, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
//        Gdiplus::RectF  boundingBox(0, 0, (Gdiplus::REAL)WIDTH(rc),0);
//        Gdiplus::RectF  boundBox(0, 0, 0, 0);
//        Gdiplus::SolidBrush solidBrush(Gdiplus::Color(255, 0, 0));
//        Gdiplus::StringFormat sf;
//        sf.SetAlignment(Gdiplus::StringAlignmentNear);
//        Gdiplus::Graphics g(hDC);
//        g.MeasureString(m_strText.c_str(), -1, &myfont, boundingBox, &sf, &boundBox);
//        g.DrawString(m_strText.c_str(), -1, &myfont, boundBox, &sf, &solidBrush);
//        bHandled = TRUE;
//        return 0;
//    }
//    LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        RECT rc; GetClientRect(&rc);
//        HDC hDC = ::GetWindowDC(m_hWnd);
//        if(0!=lParam) m_strText = (LPCWSTR)lParam;
//        else m_strText = L"";
//
//        int nStyle = m_bold?Gdiplus::FontStyleBold:Gdiplus::FontStyleRegular;
//        if(m_italy) nStyle |= Gdiplus::FontStyleItalic;
//        if(m_underline) nStyle |= Gdiplus::FontStyleUnderline;
//        Gdiplus::FontFamily myfontFamily(m_strFamily.c_str());
//        Gdiplus::Font myfont(&myfontFamily, (Gdiplus::REAL)m_nWeight, nStyle, Gdiplus::UnitPixel);
//        Gdiplus::RectF  boundingBox(0, 0, (Gdiplus::REAL)WIDTH(rc),0);
//        Gdiplus::RectF  boundBox(0, 0, 0, 0);
//        Gdiplus::StringFormat sf;
//        sf.SetAlignment(Gdiplus::StringAlignmentNear);
//        Gdiplus::Graphics g(hDC);
//        g.MeasureString(m_strText.c_str(), -1, &myfont, boundingBox, &sf, &boundBox);
//        if(boundBox.Height > HEIGHT(rc))
//            SetWindowPos(HWND_TOP, 0, 0, (int)boundBox.Width, (int)boundBox.Height, SWP_NOMOVE);
//        return 0;
//    }
//    LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//    {
//        static HCURSOR hcur = LoadCursor ( NULL, IDC_HAND );
//        if ( NULL != hcur )
//        {
//            SetCursor ( hcur );
//            return TRUE;
//        }
//        else
//        {
//            SetMsgHandled(false);
//            return FALSE;
//        }
//    }
//
//protected:
//private:
//    COLORREF        m_color;
//    std::wstring    m_strFamily;
//    int             m_nWeight;
//    std::wstring    m_strText;
//    BOOL            m_bold;
//    BOOL            m_italy;
//    BOOL            m_underline;
//};
//


#endif