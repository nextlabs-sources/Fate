// MsgDlg.h : Declaration of the CEncryptionDlg

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>

#define ENCRYPTION_FAIL_STR			L"Encryption failed!"
#define DECRYPTION_FAIL_STR			L"Decryption failed!"

#define ENCRYPTION_CANCEL_STR		L"Encryption was canceled!"
#define DECRYPTION_CANCEL_STR		L"Decryption was canceled!"

#define ENCRYPTION_CONTINUE_STR		L"Policy allowed the original request to proceed in the absence of encryption."
#define ENCRYPTION_STOP_STR			L"Policy denied the original request in the absence of encryption."

#define DECRYPTION_CONTINUE_STR		L"Policy allowed the original request to proceed in the absence of decryption."
#define DECRYPTION_STOP_STR			L"Policy denied the original request in the absence of decryption."

#define DEFAULT_DESCRIPTION			L"Please contact you system administrator!"

// MsgDlg

class CMsgDlg : 
	public CAxDialogImpl<CMsgDlg>
{
public:
	CMsgDlg(EA_ObligationType obType = EA_ObligationType_None)
	{
		m_obType = obType;
		m_wstrStatus = L"";
		m_wstrDescription = L"";
	}

	~CMsgDlg()
	{
	}

	enum { IDD = IDD_MSG_DIALOG };

	BEGIN_MSG_MAP(CMsgDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		CHAIN_MSG_MAP(CAxDialogImpl<CMsgDlg>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	inline void set_Status(LPCWSTR lpwzStatus) { m_wstrStatus = lpwzStatus; }
	inline void set_Description(LPCWSTR lpwzDesc) { m_wstrDescription = lpwzDesc; /*m_wstrDescription += DEFAULT_DESCRIPTION;*/ }

private:
	EA_ObligationType m_obType;
	std::wstring m_wstrStatus;
	std::wstring m_wstrDescription;
};

void EA_MessageBox(HWND hOwnerWnd, LPCWSTR lpwzDesc);