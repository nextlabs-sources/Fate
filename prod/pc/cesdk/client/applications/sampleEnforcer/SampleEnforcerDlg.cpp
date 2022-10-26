// SampleEnforcerDlg.cpp : implementation file
//
// Compliant Enterprise Enforcer sample code - a basic enforcement sample to show how to use CE SDK
// .9233

#include "stdafx.h"
#include "SampleEnforcer.h"
#include "SampleEnforcerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Including the CE SDK header file
#include "CEsdk.h"

// Handy number to convert the FILETIME (100-naosecond since 1/1/1601) to CE Policy time (ms since 1/1/1970)
#define SECONDS_TO_EPOCH 11644473600
#define HUNDRED_NANOSEC  10000000

// Mapping the Action enum to a user-friendly readable string
typedef struct {
	CEAction_t action;
	LPTSTR     actionName;
} actionTuple;

static actionTuple myActionMap[] = 
{
	{ CE_ACTION_READ,             _T("Read") },
	{ CE_ACTION_DELETE,           _T("Delete") },
	{ CE_ACTION_MOVE,             _T("Move") },
	{ CE_ACTION_COPY,             _T("Copy") },
	{ CE_ACTION_WRITE,            _T("Write") },
	{ CE_ACTION_RENAME,           _T("Rename") },
	{ CE_ACTION_CHANGE_ATTR_FILE, _T("Change File Attributes") },
	{ CE_ACTION_CHANGE_SEC_FILE,  _T("Change File Security") },
	{ CE_ACTION_PRINT_FILE,       _T("Print") },
	{ CE_ACTION_PASTE_FILE,       _T("Paste") },
	{ CE_ACTION_EMAIL_FILE,       _T("Email") },
	{ CE_ACTION_IM_FILE,          _T("Instant Message") },
	{ CE_ACTION_EXPORT,           _T("Export") },
	{ CE_ACTION_IMPORT,           _T("Import") },
	{ CE_ACTION_CHECKIN,          _T("Check in") },
	{ CE_ACTION_CHECKOUT,         _T("Check out") },
	{ CE_ACTION_ATTACH,           _T("Attach") },
	{ CE_ACTION_RUN,              _T("Run") }
};


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSampleEnforcerDlg dialog




CSampleEnforcerDlg::CSampleEnforcerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSampleEnforcerDlg::IDD, pParent)
	, m_FromFileName(_T("Enter From Filename here"))
	, m_ToFileName(_T("Enter To Filename here"))
    , m_Action(CE_ACTION_READ)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSampleEnforcerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_FromFileName);
	DDX_Text(pDX, IDC_EDIT2, m_ToFileName);
	DDX_Control(pDX, IDC_COMBO1, m_ComboBox);
	DDX_CBIndex(pDX, IDC_COMBO1, m_Action);
}

BEGIN_MESSAGE_MAP(CSampleEnforcerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CSampleEnforcerDlg::OnBnClickedEval)
	ON_BN_CLICKED(IDCANCEL, &CSampleEnforcerDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_BUTTON2, &CSampleEnforcerDlg::OnBnClickedToButton)
	ON_BN_CLICKED(IDC_BUTTON1, &CSampleEnforcerDlg::OnBnClickedFromButton)
	ON_CBN_SELENDOK(IDC_COMBO1, &CSampleEnforcerDlg::OnCbnSelendokForAction)
END_MESSAGE_MAP()


// CSampleEnforcerDlg message handlers

BOOL CSampleEnforcerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Initialization the Action list 
	for (int i = 0; i < sizeof (myActionMap) / sizeof (actionTuple); i++) 
	{
		m_ComboBox.AddString (myActionMap[i].actionName);
	}
	m_ComboBox.SelectString(0, myActionMap[0].actionName);
	m_Action = 0;

	// Connecting to the Policy Decision Point (PDP) engine
	TCHAR appPath [MAX_PATH];

	memset (appPath, 0, sizeof (appPath));
	::GetModuleFileName (NULL, appPath, sizeof (appPath) / sizeof (TCHAR));

	CEApplication sampleEnforcerApp;
	sampleEnforcerApp.appName   = CEM_AllocateString (_T("CE Sample Enforcer"));
	sampleEnforcerApp.appPath   = CEM_AllocateString (appPath);
	sampleEnforcerApp.appURL    = NULL;  // This is not a web application

	CEUser sampleEnforcerUser;
	sampleEnforcerUser.userID   = NULL;
	sampleEnforcerUser.userName = NULL;

	CEString PDPHost            = CEM_AllocateString (_T("127.0.0.1"));	// Use the local PDP service

	CEResult_t result = CECONN_Initialize (sampleEnforcerApp, sampleEnforcerUser, PDPHost, &m_ConnectHandle, 3000);

	if (result != CE_RESULT_SUCCESS)
	{
		MessageBox (_T("Cannot connect to the Policy Controller service. Exiting..."), _T("Error"), MB_OK);
		OnCancel();
	}

	CEM_FreeString (sampleEnforcerApp.appName);
	CEM_FreeString (sampleEnforcerApp.appPath);
	CEM_FreeString (PDPHost);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSampleEnforcerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSampleEnforcerDlg::OnPaint()
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
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSampleEnforcerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSampleEnforcerDlg::OnBnClickedEval()
{
	CEEnforcement_t enforcement;
	CEResult_t      result;
	CEAttributes    fromAttributes;
	CEAttribute     fromAttr[3];
	CEAttributes    toAttributes;
	CEAttribute     toAttr[3];	
	CEString        fromFilename;
	CEString        toFilename;

	fromFilename    = CEM_AllocateString (m_FromFileName.GetString());
	toFilename      = CEM_AllocateString (m_ToFileName.GetString());

	// Show case of supplying the attributes of the file resource
	long long atime;
	long long ctime;
	long long mtime;
	const int numAttrs = 3;

	TCHAR accessTime  [20];
	TCHAR createTime  [20];
	TCHAR modifiedTime[20];

	getFileTime (m_FromFileName.GetString(), atime, ctime, mtime);

	_stprintf_s (accessTime,   sizeof (accessTime)   / sizeof (TCHAR), _T("%lld"), atime);
	_stprintf_s (createTime,   sizeof (createTime)   / sizeof (TCHAR), _T("%lld"), ctime);
	_stprintf_s (modifiedTime, sizeof (modifiedTime) / sizeof (TCHAR), _T("%lld"), mtime);

	fromAttr[0].key   = CEM_AllocateString (CE_ATTR_LASTACCESS_TIME);
	fromAttr[0].value = CEM_AllocateString (accessTime);
	fromAttr[1].key   = CEM_AllocateString (CE_ATTR_CREATE_TIME);
	fromAttr[1].value = CEM_AllocateString (createTime);
	fromAttr[2].key   = CEM_AllocateString (CE_ATTR_LASTWRITE_TIME);
	fromAttr[2].value = CEM_AllocateString (modifiedTime);

	fromAttributes.attrs = fromAttr;
	fromAttributes.count = numAttrs;

	getFileTime (m_ToFileName.GetString(), atime, ctime, mtime);

	_stprintf_s (accessTime,   sizeof (accessTime)   / sizeof (TCHAR), _T("%lld"), atime);
	_stprintf_s (createTime,   sizeof (createTime)   / sizeof (TCHAR), _T("%lld"), ctime);
	_stprintf_s (modifiedTime, sizeof (modifiedTime) / sizeof (TCHAR), _T("%lld"), mtime);

	toAttr[0].key   = CEM_AllocateString (CE_ATTR_LASTACCESS_TIME);
	toAttr[0].value = CEM_AllocateString (accessTime);
	toAttr[1].key   = CEM_AllocateString (CE_ATTR_CREATE_TIME);
	toAttr[1].value = CEM_AllocateString (createTime);
	toAttr[2].key   = CEM_AllocateString (CE_ATTR_LASTWRITE_TIME);
	toAttr[2].value = CEM_AllocateString (modifiedTime);

	toAttributes.attrs = toAttr;
	toAttributes.count = numAttrs;

	// Calling the PDP Policy controller to get the decision
	result = CEEVALUATE_CheckFile (m_ConnectHandle, (CEAction_t) (m_Action + 1), 
								   fromFilename,
								   &fromAttributes,
								   toFilename,
								   &toAttributes,
								   0,
								   NULL,
								   NULL,
								   CETrue,
								   CE_NOISE_LEVEL_USER_ACTION,
								   &enforcement,
								   CE_INFINITE);

	if (result == CE_RESULT_SUCCESS) 
	{
		switch (enforcement.result) {
			case CEAllow:
				MessageBox(_T("Operation is Allowed."), _T("Evaluation Result"), MB_OK);
				break;
			case CEDeny:
				MessageBox(_T("Operation is Denied."), _T("Evaluation Result"), MB_OK);
				break;
		}
	}
	else
	{
		TCHAR message[128];
		_stprintf_s (message, sizeof (message) / sizeof (TCHAR), _T("Evaluation failed. Error code: %d\n"), result);
		MessageBox  (message, _T("Evaluation Result"), MB_OK);
	}

	// Cleaning up
	for (int i = 0; i < numAttrs; i++)
	{
		CEM_FreeString (fromAttr[i].key);
		CEM_FreeString (fromAttr[i].value);
		CEM_FreeString (toAttr[i].key);
		CEM_FreeString (toAttr[i].value);
	}

	CEEVALUATE_FreeEnforcement (enforcement);
	CEM_FreeString (fromFilename);
	CEM_FreeString (toFilename);

	return;
}

void CSampleEnforcerDlg::OnBnClickedExit()
{
	CECONN_Close (m_ConnectHandle, 1000);
	OnOK();
}

// Browse button for the To Filename
void CSampleEnforcerDlg::OnBnClickedToButton()
{
	// List all the file from the Open File Dialog box
	TCHAR szFilters[] =_T("All files(*.*)|*.*|");
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY,
					szFilters,
					NULL,
					0);
					
	if (dlg.DoModal() == IDOK) 
	{
		m_ToFileName = dlg.GetPathName();
	}
	UpdateData(FALSE);
	return;
}

// Browse button for the From Filename
void CSampleEnforcerDlg::OnBnClickedFromButton()
{
	// List all the file from the Open File Dialog box
	TCHAR szFilters[] =_T("All files(*.*)|*.*|");
	CFileDialog dlg(TRUE,
					NULL,
					NULL,
					OFN_HIDEREADONLY,
					szFilters,
					NULL,
					0);
					
	if (dlg.DoModal() == IDOK) 
	{
		m_FromFileName = dlg.GetPathName();
	}
	UpdateData(FALSE);
	return;
}

void CSampleEnforcerDlg::OnCbnSelendokForAction()
{
	// User selected the Action
	m_Action = m_ComboBox.GetCurSel();
}

void CSampleEnforcerDlg::getFileTime (const TCHAR * filename, long long &atime, long long &ctime, long long &mtime)
{
	atime = 0;
	ctime = 0;
	mtime = 0;
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;

	// Sample code to retrieve the file attributes so that policy can be applied to these properties
	// Converting time 100-nanosecond 1/1/1601 to millisecond 1/1/1970

	if (::GetFileAttributesEx (filename, GetFileExInfoStandard, &fileAttrData))
	{
		atime = ((long long) fileAttrData.ftLastAccessTime.dwHighDateTime << 32) +
						     fileAttrData.ftLastAccessTime.dwLowDateTime;
		ctime = ((long long) fileAttrData.ftCreationTime.dwHighDateTime   << 32) +
						     fileAttrData.ftCreationTime.dwLowDateTime;
		mtime = ((long long) fileAttrData.ftLastWriteTime.dwHighDateTime  << 32) +
						     fileAttrData.ftLastWriteTime.dwLowDateTime;

		atime -= (SECONDS_TO_EPOCH * HUNDRED_NANOSEC);
		ctime -= (SECONDS_TO_EPOCH * HUNDRED_NANOSEC);
		mtime -= (SECONDS_TO_EPOCH * HUNDRED_NANOSEC);

		atime = atime / 10000;
		ctime = ctime / 10000;
		mtime = mtime / 10000;
	}
}