// Removable.cpp : implementation file
//

#include "stdafx.h"


#include "Obligation.h"


#include "Removable.h"
#include "WinAD.h"
#include "fileprocess.h"



// CRemovable dialog

IMPLEMENT_DYNAMIC(CRemovable, CDialog)

CRemovable::CRemovable(COBEmail* pEmail,CWnd* pParent /*=NULL*/)
	: CDialog(CRemovable::IDD, pParent)
	,m_pEmail(pEmail)
	, m_wstrDisplay(_T(""))
{
	ASSERT(pEmail != NULL);
}

CRemovable::~CRemovable()
{
}

void CRemovable::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_REDISPLAY, m_wstrDisplay);
	DDX_Control(pDX, IDC_REMOVABLEFILELIST, m_ctlFileList);
}


BEGIN_MESSAGE_MAP(CRemovable, CDialog)
	ON_BN_CLICKED(ID_RESUBMIT, &CRemovable::OnBnClickedResubmit)
	ON_BN_CLICKED(IDC_AddFile, &CRemovable::OnBnClickedAddfile)
	ON_BN_CLICKED(IDC_DelFile, &CRemovable::OnBnClickedDelfile)
	ON_LBN_SELCHANGE(IDC_REMOVABLEFILELIST, &CRemovable::OnLbnSelchangeRemovablefilelist)
END_MESSAGE_MAP()

 
// CRemovable message handlers

BOOL CRemovable::OnInitDialog()
{
	CDialog::OnInitDialog();
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	this->SetIcon(hIcon,TRUE);
	// TODO:  Add extra initialization here
	m_wstrDisplay = L"Sharing this information using Removable Media needs approval, The system\r\n";
	m_wstrDisplay +=L"will start a workflow for approval. Please fill in  all the information  \r\n";
	m_wstrDisplay +=L"and submit for approval. An email will be send to you upon approval";
	UpdateData(FALSE);
	GetDlgItem(IDC_DelFile)->EnableWindow(FALSE);
	m_ctlFileList.AddString(g_BaseArgument.wstrDenyFile.c_str());
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRemovable::OnBnClickedResubmit()
{
	// TODO: Add your control notification handler code here
	CString strRecipients;
	GetDlgItem(IDC_RMRECIPIENTS)->GetWindowText(strRecipients);
	if(strRecipients.IsEmpty())
	{
		MessageBox(L"Recipients can't empty! please separate it by ;",L"Error");
		return ;
	}
	g_BaseArgument.wstrRecipientUser = strRecipients;

	CString strPurpose;
	GetDlgItem(IDC_REMOVABLEPURPOSE)->GetWindowText(strPurpose);
	if(strPurpose.IsEmpty())
	{
		MessageBox(L"Purpose can't empty!",L"Error");
		return ;
	}
	g_BaseArgument.wstrPurpose = strPurpose;

	if(m_ctlFileList.GetCount() < 1)
	{
		MessageBox(L"File can't empty",L"Error");
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

void CRemovable::OnBnClickedAddfile()
{
	// TODO: Add your control notification handler code here
	CString strFilePath;
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

void CRemovable::OnBnClickedDelfile()
{
	// TODO: Add your control notification handler code here
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		m_ctlFileList.DeleteString(nsel);
	}
}

void CRemovable::OnLbnSelchangeRemovablefilelist()
{
	// TODO: Add your control notification handler code here
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		GetDlgItem(IDC_DelFile)->EnableWindow(TRUE);
	}
}
