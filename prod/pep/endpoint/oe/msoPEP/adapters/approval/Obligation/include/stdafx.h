// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#pragma warning(push)
#pragma warning(disable:6387)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#pragma warning(pop)


#include <afxdisp.h>        // MFC Automation classes

#include "OBEmail.h"

#include <string>

#include <vector>





typedef std::pair<std::wstring, std::wstring>   FilePair;   // First is source path, Last is quarantine path
typedef std::vector<FilePair>                   FileVector;
/*
 * base argument
*/

typedef struct _base_argument
{
	// get from CO
	std::wstring wstrApprType;	// approval type: EMail attachment,FTP or Removable media
	// ftp determined using AD,the others get from CO
	std::wstring wstrApprEMail;	//Approver Email Address;
	std::wstring wstrApprSid;	// Approver Sid
	// EMail get from CO,the others get from user input
	std::wstring wstrRecipientUser;	//Who the contact is to whom the information is sent,maybe it's a list separated by ;
	// ftp: user input or drop down from config file ,the others get from CO
	std::wstring wstrCustomer;	//Which customer the information belongs to
	// get from user input
	std::wstring wstrPurpose;	
	// get from C0
	std::wstring wstrCusetomerKey;	// just for removable media
	// get from CO
	std::wstring wstrEncryptionPwd;	//only if removable's key is not passed from CO.
	// get from CO
	std::wstring wstrApprDir;		//Where to store the file when it is approved?
	// get from CO
	std::wstring wstrQuarantineDir;	//Where to store the file while approval is pending?
	// get from CO
	std::wstring wstrArchivalID;	//If email sent to approver needs to be archived, optional 
	// get send from CO
	std::wstring wstrCurUserName;
	// get sender address from AD
	std::wstring wstrCurAddress;
	// cur user sid
	std::wstring wstrCurSID;
	// income file form policy controller
	std::wstring wstrDenyFile;
	//
	FileVector approverVector;
	// file 
	FileVector vecFile;
	
}BaseArgument;
extern BaseArgument g_BaseArgument;



extern bool CheckEmailValiable(const std::wstring str);
extern void GetQuarantinePath(/*IN*/ULONGLONG lTime,/*IN*/const CString& strOrigPath,/*OUT*/std::wstring& strQuarPath);
#include "../../ylib/log.h"
#include "../../ylib/security.h"

#include "log.h"

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


