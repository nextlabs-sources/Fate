// EMailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Obligation.h"
#include "EMailDlg.h"
#include "WinAD.h"

#include "fileprocess.h"
// CEMailDlg dialog

IMPLEMENT_DYNAMIC(CEMailDlg, CDialog)

CEMailDlg::CEMailDlg(COBEmail* pEmail,CWnd* pParent /*=NULL*/)
	: CDialog(CEMailDlg::IDD, pParent)
	,m_pEmail(pEmail)
	, m_strDisplay(_T(""))
{
	ASSERT(m_pEmail != NULL);
}

CEMailDlg::~CEMailDlg()
{
}

void CEMailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DIAPLAY, m_strDisplay);
	DDX_Control(pDX, IDC_FILELIST, m_ctlFileList);
}


BEGIN_MESSAGE_MAP(CEMailDlg, CDialog)
	ON_BN_CLICKED(ID_EMALSUBMIT, &CEMailDlg::OnBnClickedEmalsubmit)
	ON_LBN_SELCHANGE(IDC_FILELIST, &CEMailDlg::OnLbnSelchangeFilelist)
	ON_BN_CLICKED(IDC_EMAILAddFile, &CEMailDlg::OnBnClickedEmailaddfile)
	ON_BN_CLICKED(IDC_EMAILDelFile, &CEMailDlg::OnBnClickedEmaildelfile)
	ON_BN_CLICKED(IDCANCEL, &CEMailDlg::OnBnClickedCancel)
	ON_LBN_SETFOCUS(IDC_FILELIST, &CEMailDlg::OnLbnSetfocusFilelist)
END_MESSAGE_MAP()


// CEMailDlg message handlers

BOOL CEMailDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	this->SetIcon(hIcon,TRUE);
	// TODO:  Add extra initialization here
	m_strDisplay = L"Sharing this information using EMail needs approval, The system will\r\n";
	m_strDisplay +=L"start a workflow for approval. Please fill in  all the information  \r\n";
	m_strDisplay +=L"and submit for approval. An email will be send to you upon approval";
	UpdateData(FALSE);

	GetDlgItem(IDC_EMAILDelFile)->EnableWindow(FALSE);
	m_ctlFileList.AddString(g_BaseArgument.wstrDenyFile.c_str());
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEMailDlg::OnBnClickedEmalsubmit()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CString strPurpose;
	GetDlgItem(IDC_EMAILPURPOSE)->GetWindowText(strPurpose);
	if(strPurpose.IsEmpty())
	{
		::MessageBoxW(NULL,L"Purpose can't empty!",L"Error",MB_OK);
		return ;
	}
	g_BaseArgument.wstrPurpose = strPurpose;
	
	if(m_ctlFileList.GetCount() < 1)
	{
		::MessageBoxW(NULL,L"File can't empty!",L"Error",MB_OK);
		return ;
	}
	CTime curTime = ::GetCurrentTime();
	ULONGLONG lTime = curTime.GetTime();
	//int npos=0;


	for(int n=0;n<m_ctlFileList.GetCount();n++)
	{
		CString strFile ;
		m_ctlFileList.GetText(n,strFile);
		// get file name 
		std::wstring wstrQuarPath;
		GetQuarantinePath(lTime++,strFile,wstrQuarPath);	
		std::wstring strOrig(strFile);
		g_BaseArgument.vecFile.push_back(FilePair(strOrig, wstrQuarPath));
		if(!FileProcess::CopyFileToDest(strOrig.c_str(),wstrQuarPath.c_str()))
		{
			MessageBox(L"Copy file failed! please check your security!",L"Error",MB_OK);

			// add by Tonny at 2008/9/11 for bug 6529
			// we should clear the g_BaseArgument's file vector
			g_BaseArgument.vecFile.clear();

			return ;
		}
	}
	if(!m_pEmail->SendEmail())
	{
		MessageBox(L"Send file failed,maybe user reject the action!",L"title",MB_OK);
		return;
	}
	OnOK();
}


void CEMailDlg::OnLbnSelchangeFilelist()
{
	// TODO: Add your control notification handler code here
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		GetDlgItem(IDC_EMAILDelFile)->EnableWindow(TRUE);
	}
}

void CEMailDlg::OnBnClickedEmailaddfile()
{
	// TODO: Add your control notification handler code here
	CString strFilePath ;
	CFileDialog theDlg(TRUE);
	if(theDlg.DoModal() == IDOK)
	{
		strFilePath = theDlg.GetPathName();
		if(m_ctlFileList.FindString(0,strFilePath) > -1)
		{
			CString strMsg = L"The file ";
			strMsg += strFilePath;
			strMsg += "already added. Do you want to replace it?";
			MessageBox(strMsg,NULL,MB_OKCANCEL);
			return;
		}
		m_ctlFileList.AddString(strFilePath);
	}
}

void CEMailDlg::OnBnClickedEmaildelfile()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EMAILDelFile)->EnableWindow(FALSE);
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		m_ctlFileList.DeleteString(nsel);
	}
}

void CEMailDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

void CEMailDlg::OnLbnSetfocusFilelist()
{
	// TODO: Add your control notification handler code here
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		GetDlgItem(IDC_EMAILDelFile)->EnableWindow(TRUE);
	}
}
