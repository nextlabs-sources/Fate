#pragma once
#include "Resource.h"

#define WM_HANDLE_STOPPC WM_USER + 130

typedef enum UI_TYPE{UI_BASIC = 0, UI_STOPPING};

class CStopEnfDlg: 
	public CAxDialogImpl<CStopEnfDlg>
{
public:
	CStopEnfDlg(void);
	~CStopEnfDlg(void);

	enum { IDD = IDD_STOPENFORCER  };



	BEGIN_MSG_MAP(CStopEnfDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_HANDLE_STOPPC, OnHandleStopPC)
		COMMAND_HANDLER(IDC_BUTTON_CANCEL_STOPENF, BN_CLICKED, OnBnClickedButtonCancelStopenf)
		COMMAND_HANDLER(IDC_BUTTON_SUBMIT_STOPENF, BN_CLICKED, OnBnClickedButtonSubmitStopenf)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandleStopPC(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonCancelStopenf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSubmitStopenf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

public:
	void SwitchUI(UI_TYPE nType);
protected:
	BOOL m_bIsStoppingPC;
};
