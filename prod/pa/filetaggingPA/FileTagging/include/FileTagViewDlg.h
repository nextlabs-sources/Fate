#pragma once

#include <atlhost.h>
#include "Resource.h"

class CFileTagViewDlg:public CAxDialogImpl<CFileTagViewDlg>
{
public:
	CFileTagViewDlg(void);
	~CFileTagViewDlg(void);

	enum { IDD = IDD_VIEWRESETTAGSDLG };

	BEGIN_MSG_MAP(CFileTagViewDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)

		COMMAND_HANDLER(IDC_REMOVE, BN_CLICKED, OnBnClickedRemove)

		NOTIFY_HANDLER(IDC_TAGSLIST, LVN_ITEMCHANGED, OnLvnItemchangedListtags)
//		NOTIFY_HANDLER(IDC_TAGSLIST, NM_KILLFOCUS, OnKillFocus)
		NOTIFY_HANDLER(IDC_TAGSLIST, NM_SETFOCUS, OnSetFocus)
		NOTIFY_HANDLER(IDC_TAGSLIST, LVN_COLUMNCLICK, OnColumnClick)

		CHAIN_MSG_MAP(CAxDialogImpl<CFileTagViewDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnLvnItemchangedListtags(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);

	void SetFileName(LPCWSTR pszFileName);
	void SetFileTagMgr(CFileTagMgr* pMgr){m_pMgr = pMgr;};
protected:
	BOOL  ShowTags(LPCWSTR pszFileName);
	std::list<std::wstring> GetSelectedTags();
	std::wstring			m_strFileName;
	CFileTagMgr*			m_pMgr;
	int						m_nSortType;
	int						m_nSortColumn;
	BOOL					m_bSort;
public:
	LRESULT OnBnClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
