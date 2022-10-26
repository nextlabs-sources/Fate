#pragma once

#include "stdafx.h"
#include "tabcfg.h"
#include "resource.h"
#include "NL_TabCtrl.h"

class CEDLPMgrDlg : 
	public CAxDialogImpl<CEDLPMgrDlg>
{
public:
	CEDLPMgrDlg();
	~CEDLPMgrDlg();

	enum { IDD = IDD_MAINDLG  };

	BEGIN_MSG_MAP(CEDLPMgrDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	//	MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDC_CANCEL, BN_CLICKED, OnClickedCancel)
		NOTIFY_HANDLER(IDC_FEATURES_TAB, TCN_SELCHANGE, OnTcnSelchangeTab1)
		CHAIN_MSG_MAP(CAxDialogImpl<CEDLPMgrDlg>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bRet = TRUE;
		for (DWORD i = 0; i < m_tabinfoPlus.size(); i++)
		{
			if (m_tabinfoPlus[i].hWnd && ::IsWindow(m_tabinfoPlus[i].hWnd))
			{
				if( -1 == SendMessageW(m_tabinfoPlus[i].hWnd, WM_NXTLBS_MYSUBMIT, NULL, NULL) )
				{
					bRet = FALSE;
				}
			}
		}

		if(bRet)
		{
			DestroyWindow();
		}
		return 0;
	}

	LRESULT OnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SendMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//EndDialog(wID);
		DestroyWindow();
		return 0;
	}

	LRESULT OnTcnSelchangeTab1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);


	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


public:
	void ShowTab(int nIndex);
	void ShowNotification(int nID);
private:


	CNL_TabCtrl* m_pTabCtrl;


	CTabCfg m_tabcfg;

	std::vector<CTabCfg::TabsInfo> m_tabinfo;

	typedef struct
	{
		CTabCfg::TabsInfo tabinfo;
		DWORD tabID;
		HWND hWnd;
	}TabsInfoPlus;

	std::vector<TabsInfoPlus> m_tabinfoPlus;

	HICON	m_hHelp;

	RECT m_rcHelp;
};
