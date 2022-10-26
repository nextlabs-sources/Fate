#pragma once

#include "NXTLBS_Window.h"
#include <string>
#include <vector>
#include "resource.h"

using namespace std;

typedef struct struEnforcerStatus
{
	wstring strEnforcerName;
	wstring strEnforcerStatus;
}ENFORCERSTATUS, *LPENFORCERSTATUS;

class CEnforcerStatusWindow:  public CNXTLBS_Window<CEnforcerStatusWindow>
{
public:
	CEnforcerStatusWindow(void);
	~CEnforcerStatusWindow(void);

	BEGIN_MSG_MAP(CEnforcerStatusWindow)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		CHAIN_MSG_MAP(CAxDialogImpl<CEnforcerStatusWindow>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	enum { IDD = IDD_ENFORCERSTATUS};

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//it will call the CNXTLBS_Window::OnPaint if sub-class doesn't implement it.
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
//		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		ShowWindow(SW_HIDE);
		return TRUE;
	}

public:
	void SetEnforcerStatusInfo(vector<ENFORCERSTATUS>& vEnforcers);
	void ShowEnforcerStatus(int x, int y);
protected:
	void ComputePosition(IN OUT int& x, IN OUT int& y);
protected:
	vector<ENFORCERSTATUS>	m_vEnforcers;
	HFONT					m_hEnforcerNameFont;
	HFONT					m_hEnforcerStatusFont;
	COLORREF				m_clrEnforcerName;
	COLORREF				m_clrEnforcerStatus;
};
