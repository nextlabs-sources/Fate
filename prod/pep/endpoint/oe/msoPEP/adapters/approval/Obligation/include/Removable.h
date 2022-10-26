#pragma once
#include "afxwin.h"


// CRemovable dialog

class CRemovable : public CDialog
{
	DECLARE_DYNAMIC(CRemovable)

public:
	CRemovable(COBEmail* pEmail,CWnd* pParent = NULL);   // standard constructor
	virtual ~CRemovable();

// Dialog Data
	enum { IDD = IDD_REMOVABLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	COBEmail* m_pEmail;
	CString m_wstrDisplay;
	CListBox m_ctlFileList;
public:
	afx_msg void OnBnClickedResubmit();
	afx_msg void OnBnClickedAddfile();
	afx_msg void OnBnClickedDelfile();
	afx_msg void OnLbnSelchangeRemovablefilelist();
};
