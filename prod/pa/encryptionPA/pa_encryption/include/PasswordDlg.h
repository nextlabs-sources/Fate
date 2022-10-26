// PasswordDlg.h : Declaration of the CEncryptionDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "MyListView.h"

// CPasswordDlg

class CPasswordDlg : 
	public CAxDialogImpl<CPasswordDlg>
{
public:
	CPasswordDlg(BOOL bOptional)
	{
		m_bOptional = bOptional;

		m_wstrPassword = L"";
		m_wstrDescription1 = L"";
		m_wstrDescription2 = L"";
		m_bEncrypt = TRUE;
		m_brush = NULL ;
		m_brBanner = NULL ;
		m_hIcon = NULL ;
	}

	~CPasswordDlg()
	{
		m_listFiles.clear();
	}

	enum { IDD = IDD_PASSWORD };

	BEGIN_MSG_MAP(CPasswordDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtrlColorsStatic) 
		MESSAGE_HANDLER(WM_PAINT,OnPaint) ;
// 		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
// 		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_PASSWORD1, EN_CHANGE, OnEnChangePassword1)
		COMMAND_HANDLER(IDC_PASSWORD2, EN_CHANGE, OnEnChangePassword2)
		COMMAND_HANDLER(IDC_SYM_ENCRYPT, BN_CLICKED, OnBnClickedSymEncrypt)
		CHAIN_MSG_MAP(CAxDialogImpl<CPasswordDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	inline void set_Description1(std::wstring &wstrDescription) 
	{ 
		m_wstrDescription1 = wstrDescription; 
		if (::IsWindow(m_hWnd))
		{
			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_SYM_DESC1), m_wstrDescription1.c_str());
		}
	}
// 	inline void set_Description2(std::wstring &wstrDescription) 
// 	{ 
// 		m_wstrDescription2 = wstrDescription; 
// 		if (::IsWindow(m_hWnd))
// 		{
// 			::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_SYM_DESC2), m_wstrDescription2.c_str());
// 		}
// 	}
	
	inline BOOL get_Encrypt(void) { return IsDlgButtonChecked(IDC_SYM_ENCRYPT); }
	inline LPWSTR get_Password(void) { return (LPWSTR)m_wstrPassword.c_str(); }

	inline void put_FileName(std::wstring &wstrSrcFileName, std::wstring &wstrDisplayFileName) 
	{ 
		std::pair<std::wstring, std::wstring> filePair(wstrSrcFileName, wstrDisplayFileName);
		m_listFiles.push_back(filePair);  
	}

protected:
	void UpdateListView( void );
	VOID InitHeaderStyle(VOID) ;
	VOID InitControlStyle(VOID) ;
	VOID RedrawInfoIcon(HDC hDC) ;
	void ShowEditScrollbar(DWORD dwID);
private:
	std::wstring m_wstrPassword;
	std::wstring m_wstrDescription1;
	std::wstring m_wstrDescription2;
	HBRUSH m_brush; 
	HBRUSH m_brBanner; 
	CImage m_imgBackground ;
	BOOL m_bOptional;
	BOOL m_bEncrypt;
	HICON m_hIcon ;
	std::list<std::pair<std::wstring, std::wstring>> m_listFiles;
	CMyListView m_listView;

public:
	LRESULT OnEnChangePassword1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangePassword2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSymEncrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

// BOOL EA_GetPasswordFromUser(HWND hParentWnd, std::wstring &wstrPassword, BOOL bOptional);
