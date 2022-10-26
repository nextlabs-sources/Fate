

#ifndef _Y_UIHELP_H_
#define _Y_UIHELP_H_


#include <atlbase.h>
#include <atlwin.h>

template <class T, COLORREF t_crBrushColor>
class CPaintBkgnd : public CMessageMap
{
public:
	CPaintBkgnd() { m_hbrBkgnd = CreateSolidBrush(t_crBrushColor); }
	~CPaintBkgnd() { DeleteObject ( m_hbrBkgnd ); }

	BEGIN_MSG_MAP(CPaintBkgnd)
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


template <class T, COLORREF t_crTextColor=RGB(0,0,0), COLORREF t_crBkgnd=RGB(0,0,255)>  
class CCtrlColor  
{  
public:  
	CCtrlColor() { m_brBkgnd = CreateSolidBrush(t_crBkgnd);}  
	~CCtrlColor() { DeleteObject(m_brBkgnd);}  

	BEGIN_MSG_MAP(CCtrlColor)  
		MESSAGE_HANDLER(WM_CTLCOLORDLG,   OnCtlColor)  
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,   OnCtlColor)  
	END_MSG_MAP()  

	LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)  
	{  
		//T* pT = static_cast<T*>(this);  
		HDC  hDC  = (HDC)wParam;  
		HWND hWnd = (HWND)lParam;  

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