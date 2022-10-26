
#ifndef _WIN_HELP_H_
#define _WIN_HELP_H_

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlbase.h>
#include <atlwin.h>
#pragma warning(pop)

template <class T>
class COwnerDraw
{
public:
    BEGIN_MSG_MAP(COwnerDraw<T>)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(WM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(WM_DELETEITEM, OnDeleteItem)
        ALT_MSG_MAP(1)
        MESSAGE_HANDLER(OCM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(OCM_MEASUREITEM, OnMeasureItem)
        MESSAGE_HANDLER(OCM_COMPAREITEM, OnCompareItem)
        MESSAGE_HANDLER(OCM_DELETEITEM, OnDeleteItem)
    END_MSG_MAP()
};

template <class T, COLORREF t_crBrushColor>
class CErasBkgnd : public CMessageMap
{
public:
	CErasBkgnd() { m_hbrBkgnd = CreateSolidBrush(t_crBrushColor); }
	~CErasBkgnd() { DeleteObject ( m_hbrBkgnd ); }

	BEGIN_MSG_MAP(CErasBkgnd)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T*   pT = static_cast<T*>(this);
		HDC  dc = (HDC) wParam;
		RECT rcClient;
		pT->GetClientRect ( &rcClient );
		FillRect ( dc, &rcClient, m_hbrBkgnd );
		return 1;    // we painted the background
	}

protected:
	HBRUSH m_hbrBkgnd;
};

class CTransBkgnd : public CMessageMap
{
public:
    CTransBkgnd() {}
    ~CTransBkgnd() {}

    BEGIN_MSG_MAP(CErasBkgnd)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        return 1;    // we painted the background
    }

protected:
    HBRUSH m_hbrBkgnd;
};

template <class T, COLORREF t_crTextColor=RGB(0,0,0), COLORREF t_crBkgnd=RGB(0,0,255)>  
class CCtrlColor  
{  
public:  
	CCtrlColor() {
        if(t_crBkgnd==RGB(0,0,255))
            m_brBkgnd = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        else
            m_brBkgnd = CreateSolidBrush(t_crBkgnd);
    }  
	~CCtrlColor() { DeleteObject(m_brBkgnd);}  

	BEGIN_MSG_MAP(CCtrlColor)  
		MESSAGE_HANDLER(WM_CTLCOLORDLG,   OnCtlColor)  
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,   OnCtlColor)  
	END_MSG_MAP()  

	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)  
	{  
        UNREFERENCED_PARAMETER(lParam);
		//T* pT = static_cast<T*>(this);  
		HDC  hDC  = (HDC)wParam;  

		if(uMsg==WM_CTLCOLORDLG)  
		{  
			bHandled   =   TRUE;  
			return (LRESULT)m_brBkgnd;  
		}  
		else if(uMsg==WM_CTLCOLORSTATIC)  
		{  
			SetBkMode(hDC,TRANSPARENT);  
			SetTextColor(hDC,t_crTextColor);  
			bHandled   =   TRUE;  
			return (LRESULT)m_brBkgnd;  
		}  
		else  
			return   0;  
	}  

protected:  
	HBRUSH   m_brBkgnd;  
};



#endif