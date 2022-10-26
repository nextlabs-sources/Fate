// SampleEnforcerDlg.h : header file
//

#pragma once
#include "afxwin.h"

// Including the CE SDK header file
#include "CEsdk.h"

// CSampleEnforcerDlg dialog
class CSampleEnforcerDlg : public CDialog
{
// Construction
public:
	CSampleEnforcerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SAMPLEENFORCER_DIALOG };

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
	afx_msg void OnBnClickedEval();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedToButton();
	afx_msg void OnBnClickedFromButton();
	afx_msg void OnCbnSelendokForAction();
private:
	CString   m_FromFileName;
	CString   m_ToFileName;
	CComboBox m_ComboBox;
	CEHandle  m_ConnectHandle;
	int       m_Action;
	// Getting the Access Time, Create Time, and Modified Time of a
	// a file to showcase policy can be written on these properties.
	void getFileTime (const TCHAR * filename, long long &atime, long long &ctime, long long &mtime);

	
};
