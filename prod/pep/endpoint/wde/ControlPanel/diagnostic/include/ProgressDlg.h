#pragma once
#include "Resource.h"

class CWaitProgressDlg: 
	public CAxDialogImpl<CWaitProgressDlg>
{
public:
	CWaitProgressDlg(void);
	~CWaitProgressDlg(void);

	enum { IDD = IDD_WAITPROGRESSDLG  };



	BEGIN_MSG_MAP(CWaitProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public: 
	void ShowStatusText(LPCWSTR lpszText);
	void CloseProgressDlg();
	void SetDefaultStatusText(LPCWSTR lpszText);
	
protected:
	wchar_t m_szDefaultStatusText[1024];
	
};
