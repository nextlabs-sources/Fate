// nl_autowrapperdlg.h : header file
//

#pragma once
#include "afxwin.h"


// CNL_AutoUnwrapperDlg dialog
class CNL_AutoUnwrapperDlg : public CDialog
{
// Construction
public:
	CNL_AutoUnwrapperDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NL_AUTOWRAPPER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOpen();
    afx_msg void OnBnClickedSave();
    CStatic m_notifyicon;
    CStatic m_notifytext;
    CButton m_btnOpen;
    CButton m_btnSave;
    CButton m_btnCancel;

	void InitResattrMngr() ;
    void LockButton(BOOL bLock=TRUE);


};
	
