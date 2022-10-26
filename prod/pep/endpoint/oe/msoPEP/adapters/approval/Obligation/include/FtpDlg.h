// FtpDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ISBListBox.h"

// CFtpDlg dialog
class CFtpDlg : public CDialog
{
// Construction
public:
	CFtpDlg(COBEmail* pEmail,CWnd* pParent = NULL);	// standard constructor
	~CFtpDlg()
	{
		if(m_baseArgumentFlex.wstrMessageFile.length()&&_waccess(m_baseArgumentFlex.wstrMessageFile.c_str(),0)==0)
			::DeleteFile(m_baseArgumentFlex.wstrMessageFile.c_str());
	};
// Dialog Data
	enum { IDD = IDD_OBLIGATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
public:
	void SetBaseArgument(BaseArgumentFlex &baseArgumentFlex);
private:
	BaseArgumentFlex m_baseArgumentFlex;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	// display the policy
	CString m_strDisplay;
	COBEmail* m_pEmail;
	// save the ftp purpose
	CString m_wstrFtpPurpose;
	CISBListBox m_ctlFileList;
private:
	afx_msg void OnBnClickedSubmit();
	// Init Customerlist
	bool InitCustomerList(void);

	afx_msg void OnBnClickedFtpaddfile();
	afx_msg void OnBnClickedFtpdeletefile();

	BOOL ValidateRecipientsAddresss(std::wstring &strRecipients);
private:
	CComboBox m_ctlComBox;
	afx_msg void OnLbnSelchangeFtpfile();
	CString m_strApprovers;
public:
	CString m_strSubject;
};
