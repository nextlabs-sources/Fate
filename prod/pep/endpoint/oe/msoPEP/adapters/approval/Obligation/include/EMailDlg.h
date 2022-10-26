#pragma once
#include "afxwin.h"


// CEMailDlg dialog
class COBEmail;
class CEMailDlg : public CDialog
{
	DECLARE_DYNAMIC(CEMailDlg)

public:
	CEMailDlg(COBEmail* pEmail,CWnd* pParent = NULL);   // standard constructor
	virtual ~CEMailDlg();

// Dialog Data
	enum { IDD = IDD_EMAILDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	COBEmail*	m_pEmail;
	CString m_strDisplay;
	CListBox m_ctlFileList;	
public:
	afx_msg void OnBnClickedEmalsubmit();
	afx_msg void OnLbnSelchangeFilelist();
public:
	afx_msg void OnBnClickedEmailaddfile();
	afx_msg void OnBnClickedEmaildelfile();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnLbnSetfocusFilelist();
};
