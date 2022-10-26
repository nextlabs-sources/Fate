#pragma once
#include "Resource.h"
#define WM_PROGRESSDLG_UPDATESTATUS			WM_USER + 120
#define WM_PROGRESSDLG_CLOSE				WM_USER + 121

class CProgressDlg: 
	public CAxDialogImpl<CProgressDlg>
{
public:
	CProgressDlg(void);
	~CProgressDlg(void);

	enum { IDD = IDD_PROGRESSDLG  };



	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_PROGRESSDLG_UPDATESTATUS, OnUpdateStatus)
		MESSAGE_HANDLER(WM_PROGRESSDLG_CLOSE, OnCloseDlg)
		
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CloseProgressDlg();
		return 0;
	}
	LRESULT OnUpdateStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public: 
	void ShowStatusText(LPCWSTR lpszText);
	void CloseProgressDlg();
	void SetDefaultStatusText(LPCWSTR lpszText);
	void SetTopMost(BOOL bTopMost = TRUE){m_bTopMost = bTopMost;}
protected:
	wchar_t m_szDefaultStatusText[1024];
	BOOL m_bTopMost;
};
