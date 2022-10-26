// AutoEncryptDlg.h : Declaration of the CAutoEncryptDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "MyListView.h"

// CAutoEncryptDlg

class CAutoEncryptDlg : 
	public CAxDialogImpl<CAutoEncryptDlg>
{
public:
	CAutoEncryptDlg(BOOL bOptional)
	{
		m_bOptional = bOptional;

		m_bEncrypt = TRUE;
		m_wstrDescription1 = L"";
		m_wstrDescription2 = L"";
	}

	~CAutoEncryptDlg()
	{
		m_listFiles.clear();
	}

	enum { IDD = IDD_ENCRYPT_DIALOG };

	BEGIN_MSG_MAP(CAutoEncryptDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtrlColorsStatic) 
// 		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
// 		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		CHAIN_MSG_MAP(CAxDialogImpl<CAutoEncryptDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
// 	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
// 	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	inline BOOL get_Encrypt(void) { return IsDlgButtonChecked(IDC_ENCRYPT_CHECK); }
	inline void set_Description1(std::wstring &wstrDescription) 
	{ 
		m_wstrDescription1 = wstrDescription; 
		if (::IsWindow(m_hWnd))
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_CERT_DESC1), m_wstrDescription1.c_str());
		}
	}
// 	inline void set_Description2(std::wstring &wstrDescription) 
// 	{ 
// 		m_wstrDescription2 = wstrDescription; 
// 		if (::IsWindow(m_hWnd))
// 		{
// 			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_CERT_DESC2), m_wstrDescription2.c_str());
// 		}
// 	}

	inline void put_FileName(std::wstring &wstrSrcFileName, std::wstring &wstrDisplayFileName) 
	{ 
		std::pair<std::wstring, std::wstring> filePair(wstrSrcFileName, wstrDisplayFileName);
		m_listFiles.push_back(filePair); 
	}

protected:
	void UpdateListView( void );
	VOID InitHeaderStyle(VOID) ;
private:
	BOOL m_bOptional;

	BOOL m_bEncrypt;

	std::list<std::pair<std::wstring, std::wstring>> m_listFiles;
	CMyListView m_listView;

	std::wstring m_wstrDescription1;
	std::wstring m_wstrDescription2;
	CImage m_imgBackground ;
};

// BOOL EA_GetEncryptStatus(HWND hParentWnd, BOOL &bEncrypt, BOOL bOptional);

