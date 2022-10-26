// AddTagsDlg.h : Declaration of the CAddTagsDlg

#pragma once

#include "resource.h"       // main symbols

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlhost.h>
#pragma warning(pop)

#include "nl_listview_combobox.h"
#include "NXTLBS_ListView.h"
#include "ImageBase.h"

// CAddTagsDlg
class CAddTagsMgr;

class CAddTagsDlg : 
	public CAxDialogImpl<CAddTagsDlg>
{
public:
	CAddTagsDlg()
	{
		m_pMgr = NULL;
		m_hImglist = NULL;
	}

	~CAddTagsDlg()
	{
		if(m_hFontBold)
		{
			DeleteObject(m_hFontBold);
		}
		if(m_hDefaultFontComboBox)
		{
			DeleteObject(m_hDefaultFontComboBox);
		}
		m_lvTags.UnsubclassWindow();
	}

	enum { IDD = IDD_ADDTAGSDLG };

BEGIN_MSG_MAP(CAddTagsDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_NEXT, BN_CLICKED, OnBnClickedNext)
	MESSAGE_HANDLER(WM_CLOSE, OnClose)
	MESSAGE_HANDLER(WM_UPDATE_ITEM_COUNT, OnUpdateItemCount)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtrlColorsStatic) 
	MESSAGE_HANDLER(WM_PAINT, OnPaintDialog)
	COMMAND_HANDLER(IDC_CANCEL, BN_CLICKED, OnBnClickedCancel)
	REFLECT_NOTIFICATIONS();
	CHAIN_MSG_MAP(CAxDialogImpl<CAddTagsDlg>)
	
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		wNotifyCode; hWndCtl; bHandled; wID;
	//	EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		wNotifyCode; hWndCtl; bHandled;wID;
	//	EndDialog(wID);
		return 0;
	}

public:
	void ShowFilePath(LPCWSTR lpszFilePath);
	void UpdateCount(unsigned uCount);
	BOOL UpdateItem(unsigned& nTotalCount, BOOL bCheckTagCount = FALSE);
	const std::wstring GetFilePath(){return m_strFilePath;};
	void howEditScrollbar( DWORD dwID);

private:
	void InitHeader();
protected:
	std::wstring m_strFilePath;
	CNL_ListView_ComboBox m_lvTags;
	CAddTagsMgr* m_pMgr;
	CNXTLBS_ListView	m_lvFile;
	HIMAGELIST			m_hImglist;
	HFONT				m_hFontBold;
	HFONT				m_hDefaultFontComboBox;
	CImage m_imgBackground ;
public:
	LRESULT OnBnClickedNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateItemCount(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaintDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};


