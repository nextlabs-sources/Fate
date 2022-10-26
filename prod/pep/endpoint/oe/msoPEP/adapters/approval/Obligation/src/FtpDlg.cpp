// FtpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Obligation.h"
#include "FtpDlg.h"
#include "WinAD.h"

#include "fileprocess.h"

#include <fstream>
#include <comdef.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFtpDlg dialog
CFtpDlg::CFtpDlg(COBEmail* pEmail,CWnd* pParent /*=NULL*/)
	: CDialog(CFtpDlg::IDD, pParent)
	,m_pEmail(pEmail)
	, m_strDisplay(_T(""))
	, m_wstrFtpPurpose(_T(""))
	, m_strApprovers(_T(""))
	, m_strSubject(_T(""))
{
#ifdef _RELEASE
	ASSERT(m_pEmail != NULL);
#endif
}

void CFtpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DISPLAY, m_strDisplay);
	DDX_Text(pDX, IDC_FTPPURPOSE, m_wstrFtpPurpose);
	DDX_Control(pDX, IDC_FTPFILE, m_ctlFileList);
	DDX_Control(pDX, IDC_FTPCUSTOMER, m_ctlComBox);
	DDX_Text(pDX, IDC_APPROVERS, m_strApprovers);
	DDX_Text(pDX, IDC_SUBJECT, m_strSubject);
}

BEGIN_MESSAGE_MAP(CFtpDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_SUBMIT, &CFtpDlg::OnBnClickedSubmit)
	ON_BN_CLICKED(IDC_FTPADDFILE, &CFtpDlg::OnBnClickedFtpaddfile)
	ON_BN_CLICKED(IDC_FTPDELETEFILE, &CFtpDlg::OnBnClickedFtpdeletefile)
	ON_LBN_SELCHANGE(IDC_FTPFILE, &CFtpDlg::OnLbnSelchangeFtpfile)
END_MESSAGE_MAP()


// CFtpDlg message handlers

BOOL CFtpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	this->SetIcon(hIcon,TRUE);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_strDisplay = L"Sharing this information using FTP needs approval,The system will \r\n";
	m_strDisplay +=L"start a workflow for approval. Please fill in all the information \r\n";
	m_strDisplay +=L"and submit for approval. An email will be send to you upon approval";
	m_strApprovers=m_baseArgumentFlex.wstrApprovers.c_str();
	if(_waccess(m_baseArgumentFlex.wstrMessageFile.c_str(),0)==0)
	{
		HANDLE hOpenFile=(HANDLE)::CreateFile(m_baseArgumentFlex.wstrMessageFile.c_str(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hOpenFile!=INVALID_HANDLE_VALUE)
		{
			DWORD dwFileSize=::GetFileSize(hOpenFile,NULL);
			if(dwFileSize!=0xFFFFFFFF)
			{
				char *pCh=new char[dwFileSize+1];
				if(pCh!=NULL)
				{
					::ZeroMemory(pCh,dwFileSize+1);
					(void)::ReadFile(hOpenFile,pCh,dwFileSize,&dwFileSize,NULL);
					int iMinSize=::MultiByteToWideChar(CP_UTF8,NULL,pCh,-1,NULL,0);
					WCHAR *pwCh=new WCHAR[iMinSize+1];
					if(pwCh!=NULL)
					{
						::ZeroMemory(pwCh,iMinSize+1);
						::MultiByteToWideChar(CP_UTF8,NULL,pCh,-1,pwCh,iMinSize);
						WCHAR*pNewLine=::wcschr(pwCh,L'\n');
						if(pNewLine)
						{
							this->m_wstrFtpPurpose=pNewLine+1;
							pNewLine[0]=0;
							m_strSubject=pwCh;
						}
						else
						{
							m_strSubject = pwCh;
						}
						delete[] pwCh;
					}
					delete[] pCh;
				}
			}
			::CloseHandle(hOpenFile);
		}
		
		/*std::wstring strSubject;
		std::wifstream fin(m_baseArgumentFlex.wstrMessageFile.c_str());
		std::getline(fin,strSubject);
		if(fin.fail()==false)
		{
			if(strSubject.length())
				m_strSubject = strSubject.c_str();
		}
		std::wstring strBody,strBodyTemp;
		while(std::getline(fin,strBodyTemp))
		{
			strBody +=strBodyTemp;
			strBody += L"\n";
		}
		fin.close();
		fin.clear();
		this->m_wstrFtpPurpose=strBody.c_str();*/
	}
	UpdateData(FALSE);
	GetDlgItem(IDC_FTPDELETEFILE)->EnableWindow(FALSE);

	GetDlgItem(IDC_FTPRECIPIENT)->SetWindowText(m_baseArgumentFlex.wstrRecipients.c_str());
	InitCustomerList();

	if(m_baseArgumentFlex.wstrSource.length())
	{
		int iPosBegin=0,iPosEnd=0;
		while(iPosEnd!=-1)
		{
			iPosEnd=(int)m_baseArgumentFlex.wstrSource.find(L";",iPosBegin,1);
			std::wstring wstrOneSource=m_baseArgumentFlex.wstrSource.substr(iPosBegin,iPosEnd-iPosBegin);
			iPosBegin=iPosEnd+1;
			if(wstrOneSource.length())
				m_ctlFileList.AddString(wstrOneSource.c_str());
		}
		
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFtpDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	::BringWindowToTop(this->m_hWnd);
	::SetWindowPos(this->m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	//this->SetFocus();
	//::SetWindowPos(this->m_hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFtpDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CFtpDlg::ValidateRecipientsAddresss(std::wstring &strRecipients)
{
	int iPosBegin=0,iPosEnd=0;
	while(iPosEnd!=-1)
	{
		iPosEnd=(int)strRecipients.find(L";",iPosBegin,1);
		std::wstring strAddress=strRecipients.substr(iPosBegin,iPosEnd-iPosBegin);
		iPosBegin=iPosEnd+1;
		if(strAddress.length())
		{
			std::wstring::size_type iPos=strAddress.find(L"@",0,1);
			if(iPos==std::wstring::npos)
				return FALSE;
			iPos=strAddress.find(L".",iPos,1);
			if(iPos==std::wstring::npos)
				return FALSE;
		}
	}
	return TRUE;

}

void CFtpDlg::OnBnClickedSubmit()
{
	// TODO: Add your control notification handler code here
	::SetWindowPos(this->m_hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	UpdateData(TRUE);
	/*CString strCustomer;
	GetDlgItem(IDC_FTPCUSTOMER)->GetWindowText(strCustomer);
	if(strCustomer.IsEmpty())
	{
		MessageBoxW(L"Ftp Customer can't empty!",L"Error",MB_OK);
		return ;
	}
	m_baseArgumentFlex.wstrSelectedApprover = strCustomer;*/

	CString strRecipients;
	GetDlgItem(IDC_FTPRECIPIENT)->GetWindowText(strRecipients);
	if(strRecipients.IsEmpty())
	{
		MessageBoxW(L"The Recipient(s) can't be empty!",L"Error",MB_OK);
		return ;
	}
	m_baseArgumentFlex.wstrRecipients = strRecipients;
	if(ValidateRecipientsAddresss(m_baseArgumentFlex.wstrRecipients)==FALSE)
	{
		MessageBox(L"There is malformed email address for the Recipient(s) field.",L"Error",MB_OK);
		return;
	}

	if(m_wstrFtpPurpose.IsEmpty())
	{
		MessageBoxW(L"The Purpose can't be empty!",L"Error",MB_OK);
		return ;
	}
	m_baseArgumentFlex.wstrPurpose = m_wstrFtpPurpose;
	m_baseArgumentFlex.wstrSubject =this->m_strSubject;
	if( m_ctlFileList.GetCount() < 1)
	{
		MessageBox(L"The Files can't be empty!",L"Error");
		return ;
	}

	CTime curTime = ::GetCurrentTime();
	(void)curTime.GetTime();
	//int npos=0;

	if(m_ctlFileList.GetCount()==0)
		return;

	for(int n=0;n<m_ctlFileList.GetCount();n++)
	{
		CString strFile ;
		m_ctlFileList.GetText(n,strFile);
		// get file name 
		/*std::wstring wstrQuarPath;
		GetQuarantinePath(lTime++,strFile,wstrQuarPath);	
		std::wstring strOrig(strFile);
		g_BaseArgument.vecFile.push_back(FilePair(strOrig, wstrQuarPath));*/
		m_baseArgumentFlex.vFiles.push_back(std::wstring(strFile));
		//if(!FileProcess::CopyFileToDest(strOrig.c_str(),wstrQuarPath.c_str()))
		//{
		//	MessageBox(L"Copy file failed! please check your security!",L"Error",MB_OK);

		//	// add by Tonny at 2008/9/11 for bug 6529
		//	// we should clear the g_BaseArgument's file vector
		//	g_BaseArgument.vecFile.clear();

		//	return ;
		//}
	}
	if(!m_pEmail->SendEmail(m_baseArgumentFlex))
	{
		MessageBox(L"Send request email to approver(s) failed. Maybe the address of approver is wrong!",L"Fail to Email",MB_OK);
		return;
	}
	OnOK();
}

// Init Customerlist
bool CFtpDlg::InitCustomerList(void)
{
	return true;
	
}

void CFtpDlg::OnBnClickedFtpaddfile()
{
	// TODO: Add your control notification handler code here
	CFileDialog theDlg(TRUE);
	CString strFile;
	if(theDlg.DoModal() == IDOK)
	{
		strFile = theDlg.GetPathName();
		if(m_ctlFileList.FindString(0,strFile) > -1)
		{
			CString strMsg = L"The file ";
			strMsg += strFile;
			strMsg += "already added. Do you want to replace it?";
			MessageBox(strMsg,NULL,MB_OKCANCEL);
			return;
		}
		m_ctlFileList.AddString(strFile);
		//m_ctlFileList.SetHorizontalExtent(1000);
	}
}

void CFtpDlg::OnBnClickedFtpdeletefile()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_FTPDELETEFILE)->EnableWindow(FALSE);
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		m_ctlFileList.DeleteString(nsel);
	}
}

void CFtpDlg::OnLbnSelchangeFtpfile()
{
	// TODO: Add your control notification handler code here
	int nsel = m_ctlFileList.GetCurSel();
	if(nsel >= 0 && nsel < m_ctlFileList.GetCount())
	{
		GetDlgItem(IDC_FTPDELETEFILE)->EnableWindow(TRUE);
	}
}
void CFtpDlg::SetBaseArgument(BaseArgumentFlex &baseArgumentFlex)
{
	m_baseArgumentFlex=baseArgumentFlex;
}
