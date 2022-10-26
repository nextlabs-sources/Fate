// nl_autowrapperdlg.cpp : implementation file
//

#include "stdafx.h"
#include "nl_autounwrapper.h"
#include "nl_autounwrapperdlg.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include "nl_sysenc_lib.h"
#include "nl_sysenc_lib_fw.h"
#include "NextLabsEncryption_Types.h"

#include "nextlabs_feature_manager.hpp"
#include "nextlabs_features.h"
#include "eframework/resattr/resattr_loader.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static nextlabs::resattr_loader g_resattr_loader;

static std::wstring g_strTempDir;
static std::wstring g_strFilePath;
static std::wstring g_strFileDir;
static std::wstring g_strFileName;
static std::wstring g_strFileExt;
static std::wstring g_strOrigFileName;
static std::wstring g_strOrigFileExt;

_Check_return_
static
BOOL
GetOriginalFileName(
                    _In_ LPCWSTR wzFile,
                    _Out_ std::wstring& strOrigFile
                    )
{
    NextLabsFile_Header_t   hdrinfo = {0};
    
    if(!SE_GetFileInfo(wzFile, SE_FileInfo_NextLabs, &hdrinfo))
        return FALSE;
    
    strOrigFile = hdrinfo.orig_file_name;

	if(strOrigFile.empty())
	{
		std::wstring temp(wzFile);
		std::wstring::size_type nStart = temp.rfind('\\');
		std::wstring::size_type nEnd = temp.rfind('.');
		if(nStart != std::wstring::npos && nEnd != std::wstring::npos && nEnd > nStart)
		{
			strOrigFile = temp.substr(nStart + 1, nEnd - nStart - 1);
		}
	}
    return TRUE;
}

static bool is_local_disk( _In_ const wchar_t* in_path )
{
  if( wcslen(in_path) < 3 )
  {
    return false;
  }

  wchar_t dletter[32] = {0};
  wcsncpy_s(dletter,_countof(dletter),in_path,3);

  UINT dt = GetDriveType(dletter);

  bool result = false;
  switch( dt )
  {
  case DRIVE_UNKNOWN:
  case DRIVE_NO_ROOT_DIR:
  case DRIVE_REMOTE:
    result = false;
    break;

  case DRIVE_REMOVABLE:
  case DRIVE_FIXED:
  case DRIVE_CDROM:
  case DRIVE_RAMDISK:
    result = true;
    break;
  }
  return result;
}/* is_local_disk */

_Check_return_
static
BOOL
ProcessCommandLine(
                   )
{
    LPWSTR*     szArglist   = NULL;
    int         nArgs       = 0;
    std::wstring::size_type pos = std::wstring::npos;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if(NULL == szArglist) return FALSE;

    if(nArgs < 2)
    {
        MessageBoxW(NULL, L"You must select a file.", L"NextLabs Encryption Error", MB_OK);
        return FALSE;
    }

    g_strFilePath = szArglist[1];
    if(!boost::algorithm::iends_with(g_strFilePath, L".nxl"))
    {
        MessageBoxW(NULL, L"Input file is not a NXL file.", L"NextLabs Encryption Error", MB_OK);
        return FALSE;
    }

    if(!GetOriginalFileName(g_strFilePath.c_str(), g_strOrigFileName))
    {
        MessageBoxW(NULL, L"Input file is not a VALID NXL file.", L"NextLabs Encryption Error", MB_OK);
        return FALSE;
    }

    pos = g_strFilePath.find_last_of(L'\\');
    if(std::wstring::npos == pos)
    {
        MessageBoxW(NULL, L"Input file path is not valid.", L"NextLabs Encryption Error", MB_OK);
        return FALSE;
    }
    g_strFileDir = g_strFilePath.substr(0, pos+1);
    g_strFileName= g_strFilePath.substr(pos+1);
    g_strFileName= g_strFileName.substr(0, g_strFileName.find_last_of(L'.'));

    pos = g_strOrigFileName.find_last_of(L'.');
    if(std::wstring::npos != pos) g_strOrigFileExt = g_strOrigFileName.substr(pos+1);

    // Check file name
    if(!g_strOrigFileExt.empty())
    {
        std::wstring strTempExt1 = L"."; strTempExt1 += g_strOrigFileExt;
        std::wstring strTempExt2 = L" "; strTempExt2 += g_strOrigFileExt;

        if(!boost::algorithm::iends_with(g_strFileName, strTempExt1.c_str()))
        {
            if(boost::algorithm::iends_with(g_strFileName, strTempExt2.c_str()))
            {
                g_strFileName = g_strFileName.substr(0, g_strFileName.length()-strTempExt2.length());
                g_strFileName += strTempExt1;
            }
            else
            {
                g_strFileName += strTempExt1;
            }
        }
    }
    
    return TRUE;
}

_Check_return_
static
BOOL
GetTempDir(
           )
{
    WCHAR       wzTempDir[MAX_PATH+1] = {0};

	GetTempPathW(MAX_PATH, wzTempDir);
    g_strTempDir = wzTempDir;
    if(g_strTempDir.empty())
    {
        g_strTempDir = L"C:\\Temp";
        if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(g_strTempDir.c_str()))
        {
            if(!::CreateDirectoryW(g_strTempDir.c_str(), NULL))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

_Check_return_
static
int
UnWrapFile(
           _In_ LPCWSTR wzSrc,
           _In_ LPCWSTR wzDest,
           _In_ BOOL SwitchToLocalKey
           )
{
    BOOL bRet = FALSE;
    BOOL bSameFile = FALSE;
    WCHAR* pEnd = NULL;
    WCHAR wzTargetDir[MAX_PATH+1];
    NextLabsEncryptionFile_Header_t hdrinfo;
    if(!SE_GetFileInfo(g_strFilePath.c_str(), SE_FileInfo_NextLabsEncryption, &hdrinfo))
        return -1;

    bSameFile = (0 == _wcsicmp(wzSrc, wzDest))?TRUE:FALSE;

    if( (hdrinfo.flags & NLEF_REQUIRES_LOCAL_ENCRYPTION) )
    {
        //
        // Is destination a SE DRM directory?
        //
        memset(wzTargetDir, 0, sizeof(WCHAR)*(MAX_PATH+1));
        wcsncpy_s(wzTargetDir, MAX_PATH, wzDest, _TRUNCATE);
        pEnd = wcsrchr(wzTargetDir, L'\\');
        if(NULL != pEnd) *pEnd=L'\0';
        if(SE_IsEncryptedFW(wzTargetDir))
        {
            return -2;
        }

        bRet = SE_UnwrapToEncryptedFile(wzSrc, wzDest, SwitchToLocalKey);
    }
    else
    {        
        //
        // Unwrap files
        //
        bRet = SE_UnwrapToPlainFile(wzSrc, wzDest);
    }
    if(!bRet && !bSameFile) ::DeleteFileW(wzDest);

    return bRet?0:(-3);
}

static
std::wstring
GetNumberExtension(
                   _In_ int i
                   )
{
    WCHAR wzBuf[64] = {0};
    _snwprintf_s(wzBuf, 64, _TRUNCATE, L" (%d)", i);
    return std::wstring(wzBuf);
}

static
std::wstring
GetUniqueFileName(
                  _In_ const std::wstring& strFile
                  )
{
    int          i = 0;
    std::wstring strOut(strFile);
    std::wstring strDir;
    std::wstring strName;
    std::wstring strExt;
    std::wstring::size_type pos = std::wstring::npos;

    pos = strFile.find_last_of(L'\\');
    if(std::wstring::npos == pos)
        return strOut;
    strDir = strFile.substr(0, pos+1);

    strName = strFile.substr(pos+1);
    pos = strName.find_last_of(L'.');
    if(std::wstring::npos != pos)
    {
        strExt  = strName.substr(pos);
        strName = strName.substr(0, pos);
    }

    do
    {
        // See if a file with this filename exists.
        HANDLE hFile0 = CreateFileW(strOut.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile0 == INVALID_HANDLE_VALUE)
        {
            // A file with this filename doesn't exist.  Use this filename.
            break;
        }

        // A file with this filename exists.  Try to turn off read-only
        // attribute on the file (if set) and delete it.
        CloseHandle(hFile0);

        DWORD dwAttributes = GetFileAttributes(strOut.c_str());

        if (dwAttributes != INVALID_FILE_ATTRIBUTES)
        {
            if (dwAttributes & FILE_ATTRIBUTE_READONLY)
            {
                SetFileAttributes(strOut.c_str(),
                                  dwAttributes & ~FILE_ATTRIBUTE_READONLY);
            }

            if (DeleteFile(strOut.c_str()))
            {
                // File is successfully deleted.  Use this filename.
                break;
            }

            // File cannot be deleted.  Need to restore the read-only
            // attribute on the file if it was set before.
            if (dwAttributes & FILE_ATTRIBUTE_READONLY)
            {
                SetFileAttributes(strOut.c_str(), dwAttributes);
            }
        }

        // Generate the next filename.
        strOut = strDir;
        strOut += strName;
        strOut += GetNumberExtension(i++);
        strOut += strExt;
    }while(i<10000);

    return strOut;
}


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


// Cnl_AutoWrapperDlg dialog




CNL_AutoUnwrapperDlg::CNL_AutoUnwrapperDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNL_AutoUnwrapperDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNL_AutoUnwrapperDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_NOTIFY_ICON, m_notifyicon);
    DDX_Control(pDX, IDC_NOTIFY_TEXT, m_notifytext);
    DDX_Control(pDX, IDC_OPEN, m_btnOpen);
    DDX_Control(pDX, IDC_SAVE, m_btnSave);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CNL_AutoUnwrapperDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCANCEL, &CNL_AutoUnwrapperDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_OPEN, &CNL_AutoUnwrapperDlg::OnBnClickedOpen)
    ON_BN_CLICKED(IDC_SAVE, &CNL_AutoUnwrapperDlg::OnBnClickedSave)
END_MESSAGE_MAP()


// Cnl_AutoWrapperDlg message handlers

BOOL CNL_AutoUnwrapperDlg::OnInitDialog()
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
#pragma warning(push)
#pragma warning(disable: 6031)  //warning C6031: Return value ignored -- we won't handle warning from MS code
		strAboutMenu.LoadString(IDS_ABOUTBOX);
#pragma warning(pop)
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

	nextlabs::feature_manager feat;
	feat.open();

    CString strText;
	if( feat.is_enabled(NEXTLABS_FEATURE_ENCRYPTION_PORTABLE) == false )
	{
        // License is not valid, change UI
        m_notifyicon.SetIcon(LoadIcon(NULL, IDI_ERROR));
        if(strText.LoadStringW(IDS_LICENSE_WARNING))
            m_notifytext.SetWindowTextW(strText.GetBuffer());
        else
            m_notifytext.SetWindowTextW(L"NextLabs System Encryption cannot be started. This machine does not have an active license.");
        m_btnOpen.ShowWindow(SW_HIDE);
        m_btnSave.ShowWindow(SW_HIDE);
        m_btnCancel.SetWindowTextW(L"OK");

        return TRUE;
    }

    if(strText.LoadStringW(IDS_OPEN_NOTIFY))
        m_notifytext.SetWindowTextW(strText.GetBuffer());
    else
        m_notifytext.SetWindowTextW(L"Would you like to open the file or save it to your computer?");

	// TODO: Add extra initialization here
    if(!ProcessCommandLine())
    {
        OnCancel();
        return FALSE;
    }

    if(is_local_disk(g_strFilePath.c_str()))
    {
        //
        // The button should be unwrap
        //
        m_btnSave.SetWindowTextW(L"Unwrap");
        if(strText.LoadStringW(IDS_OPEN_NOTIFY2))
            m_notifytext.SetWindowTextW(strText.GetBuffer());
        else
            m_notifytext.SetWindowTextW(L"Would you like to open the file or unwrap it on your computer?");
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNL_AutoUnwrapperDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNL_AutoUnwrapperDlg::OnPaint()
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
HCURSOR CNL_AutoUnwrapperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CNL_AutoUnwrapperDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    OnCancel();
}

void CNL_AutoUnwrapperDlg::OnBnClickedOpen()
{
    std::wstring strTempFile;
    DWORD        dwAttributes = 0;

    LockButton();

    // TODO: Add your control notification handler code here
    if(!GetTempDir())
    {
		SE_DisplayErrorMessage(SE_GetLastError( ));
        //MessageBoxW(L"Cannot find temp directory, please save this file to local computer.", L"Error", MB_OK|MB_ICONERROR);
        LockButton(FALSE);
        return;
    }

    strTempFile = g_strTempDir;
    if(!boost::algorithm::iends_with(strTempFile, L"\\")) strTempFile += L"\\";
    strTempFile += g_strFileName;
    strTempFile = GetUniqueFileName(strTempFile);

	// Dummy Code to let nl_autounwrapper.exe to be trusted.
	HANDLE hFile0 = CreateFileW(strTempFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile0 != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile0);
	}


    if(0 != UnWrapFile(g_strFilePath.c_str(), strTempFile.c_str(), FALSE))
    {
		SE_DisplayErrorMessage(SE_GetLastError( ));
        //MessageBoxW(L"Cannot generate temp file, please try to save this file to local computer.", L"Error", MB_OK|MB_ICONERROR);
        LockButton(FALSE);
        return;
    }

    // Set file attribute to READONLY, so user cannot modify it
    dwAttributes = GetFileAttributes(strTempFile.c_str());
    if(INVALID_FILE_ATTRIBUTES == dwAttributes)
    {
		SE_DisplayErrorMessage(SE_GetLastError( ));
        //MessageBoxW(L"Cannot generate temp file, please try to save this file to local computer.", L"Error", MB_OK|MB_ICONERROR);
        LockButton(FALSE);
        return;
    }
    if(0 == (dwAttributes&FILE_ATTRIBUTE_READONLY))
    {
        dwAttributes |= FILE_ATTRIBUTE_READONLY;
        SetFileAttributes(strTempFile.c_str(), dwAttributes);
    }

	// Dummy Code to let nlSysEncryption.exe to be trusted.
	HANDLE hFile = CreateFileW(strTempFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

    // We have temp file now, try to open it
    //
    // Don't specify the operation as "open".  For some reason, specifying
    // "open" causes ShellExecuteW() to return SE_ERR_NOASSOC for .pdf files
    // on some KLA machines with Adobe Reader 9.4.5 installed.  Not specifying
    // an operation avoids the problem.
    ShellExecuteW(NULL, NULL, strTempFile.c_str(), NULL, NULL, SW_SHOWNORMAL);
    
    // quit
    OnOK();
}

void CNL_AutoUnwrapperDlg::OnBnClickedSave()
{
    // TODO: Add your control notification handler code here
    OPENFILENAMEW   ofn = {0};
    WCHAR           wzSavedFile[MAX_PATH+1] = {0};
    WCHAR           wzFilter[MAX_PATH+2] = {0};
    static WCHAR*   pwzTitle = L"Save NXL File";
    std::wstring    strFilter= L"*.";
    std::wstring    strInitFile= g_strFileDir;
    
    LockButton();
    
    if(g_strOrigFileExt.empty())
        strFilter += L"*";
    else
        strFilter += g_strOrigFileExt;

    // If NLEF_REQUIRES_LOCAL_ENCRYPTION is not set, copy the tags from the
    // .nxl file to the unwrapped file later.
    //
    // If NLEF_REQUIRES_LOCAL_ENCRYPTION is set, we won't need to copy the
    // tags since SE_UnwrapToEncryptedFile() will copy the NLT header (hence
    // the tags inside) in the .nxl file to the NLT header in the unwrapped
    // file.
    NextLabsEncryptionFile_Header_t hdrInfo;
    ResourceAttributeManager *mgr = NULL;
    ResourceAttributes *attrs = NULL;

    if(!SE_GetFileInfo(g_strFilePath.c_str(), SE_FileInfo_NextLabsEncryption, &hdrInfo))
        return;

    if(!(hdrInfo.flags & NLEF_REQUIRES_LOCAL_ENCRYPTION))
    {
        g_resattr_loader.ensure_loaded();

        if (g_resattr_loader.is_loaded() && (0 != g_resattr_loader.m_fns.CreateAttributeManager(&mgr) || NULL == mgr))
        {
            //OutputDebugStringW(L"Create the Attribute Manager error\n");
            mgr = NULL;
        }

        if (g_resattr_loader.is_loaded() && (0 != g_resattr_loader.m_fns.AllocAttributes(&attrs) || NULL == attrs))
        {
            //OutputDebugStringW(L"Use Attrmngr alloc Attributes list error\n");
        }
        if(!mgr || !attrs)
        {
            LockButton(FALSE);
            return ;
        }
        if(attrs)
        {
            int nRet = g_resattr_loader.m_fns.ReadResourceAttributesW(mgr, g_strFilePath.c_str(), attrs);
            if(!nRet)
            {
                LockButton(FALSE);
                return ;
            }
        }
    }

    strInitFile += g_strOrigFileName;
    memset(wzSavedFile, 0, sizeof(wzSavedFile));
	wcsncpy_s(wzSavedFile, MAX_PATH, strInitFile.c_str(), _TRUNCATE);
	
	
    memset(wzFilter, 0, sizeof(wzFilter));
    wcsncpy_s(wzFilter, MAX_PATH, strFilter.c_str(), _TRUNCATE);

    memset(&ofn, 0, sizeof(OPENFILENAMEW));
    ofn.lStructSize     = sizeof(OPENFILENAMEW); 
    ofn.hwndOwner       = GetSafeHwnd();
	ofn.lpstrInitialDir = g_strFileDir.c_str();
    ofn.lpstrDefExt     = g_strOrigFileExt.empty()?NULL:g_strOrigFileExt.c_str(); 
    ofn.lpstrFilter     = wzFilter; 
    ofn.lpstrFile       = wzSavedFile; 
    ofn.nMaxFile        = MAX_PATH; 
    ofn.lpstrFileTitle  = NULL; 
    ofn.nMaxFileTitle   = 0; 
    ofn.lpstrInitialDir = NULL; 
    ofn.Flags           = OFN_LONGNAMES | OFN_OVERWRITEPROMPT; 
    ofn.lpstrTitle      = pwzTitle;

    if(!GetSaveFileNameW(&ofn))
    {
        LockButton(FALSE);
        OnCancel();
        return;
    }

    int nRet = UnWrapFile(g_strFilePath.c_str(), wzSavedFile, TRUE);
    if(0 != nRet)
    {
        if(-2 == nRet)
        {
            MessageBoxW(L"Action Denied: One or more files is already NextLabs encrypted.  You cannot store these file(s) in folders marked for NextLabs Heavy Write encryption.", L"NextLabs Encryption Error", MB_OK|MB_ICONERROR);
        }
        else
        {
            MessageBoxW(L"Fail to save this file.", L"NextLabs Encryption Error", MB_OK|MB_ICONERROR);
        }
	}
	else if( attrs!= NULL)
	{
		int size = g_resattr_loader.m_fns.GetAttributeCount(attrs);
		if( size>0)
		{
			ResourceAttributes *pAttr = NULL;
			for (int i = 0; i < size; ++i)
			{
				std::wstring tagName = (WCHAR *)g_resattr_loader.m_fns.GetAttributeName(attrs, i);
				if( tagName.find_first_of(L"NXL_") == 0 )
				{
					continue ;
				}
				if( pAttr== NULL)
				{
					if (0 != g_resattr_loader.m_fns.AllocAttributes(&pAttr) || NULL == pAttr)
					{
						//OutputDebugStringW(L"Alloc the real added attribute error\n");
					}
				}
				WCHAR *tagValue = (WCHAR *)g_resattr_loader.m_fns.GetAttributeValue(attrs, i);
				g_resattr_loader.m_fns.AddAttributeW(pAttr, tagName.c_str(),tagValue);
			}
			
			if (pAttr!= NULL && 0 == g_resattr_loader.m_fns.WriteResourceAttributesW(mgr, wzSavedFile,pAttr))
			{
				//::OutputDebugStringW(L"Add the tag back to the saved file error:" ) ;
			}
			if(pAttr!= NULL)
			{
				g_resattr_loader.m_fns.FreeAttributes(pAttr);
			}
		}
		g_resattr_loader.m_fns.FreeAttributes(attrs);
		g_resattr_loader.m_fns.CloseAttributeManager(mgr) ;
		mgr = NULL;
	}
	// Quit
    LockButton(FALSE);
    OnOK();
}


void CNL_AutoUnwrapperDlg::LockButton(BOOL bLock)
{
    m_btnOpen.EnableWindow(!bLock);
    m_btnSave.EnableWindow(!bLock);
    m_btnCancel.EnableWindow(!bLock);
}
