#include "stdafx.h"
#include "NLOfficePEP_Comm.h"
#include "NLObMgr.h"
#include "dllmain.h"
#include "NLInsertAndDataAction.h"


// max URL path length ( IE: 2038 )
#define MAX_PATHLENGTH 2048

#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLINSERTANDDATAACTION)
//////////////////////////////////////////////////////////////////////////


CNLInsertAndDataAction::CNLInsertAndDataAction( )
{
	InitializeCriticalSection(&m_csDataDialogClose);       // kim add for Data->From text/access dialog close
	InitializeCriticalSection(&m_csExcelDataActionAllow);  // kim add for insert action deny flag. 
	InitializeCriticalSection(&m_csBrowseFilePath);
	InitializeCriticalSection(&m_csExcelOrPptInsertObjectWndProc); 
	InitializeCriticalSection(&m_csDataDialogWndProc);
	
	// for insert object
	m_wstrBrowseFilePath = L"";
	m_wpInsertObjectEDTBXDefaultWndProc  = NULL;
	m_wpInsertObjectDialogDefaultWndProc = NULL;

	// for data action
	m_bExcelDataActionAllow = true;
	m_bDataDialogClose = false;
	m_wpDataDialogDefaultWndProc = NULL;

	InitializeCriticalSection(&m_csPPTInsertObjectOKButtonDefaultWndProc);
	m_wpPPTInsertObjectOKButtonWndProc = NULL;

	// to do generic deny for excel data from access
	// caption
	m_setGenericCaption.insert( g_wszDataFromTextFirstDialogCaption );
	m_setGenericCaption.insert( g_wszDataFromAccessFirstDialogCaption );
	m_setGenericCaption.insert( g_wszDataFromAccessInsertExcelFirstCaption );
	m_setGenericCaption.insert( g_wszDataFromTextNextDialogInternalCaption );
	m_setGenericCaption.insert( g_wszDataFromTextNextDialogExternalCaption );
	m_setGenericCaption.insert( g_wszDataFromAccessNextDialogCaption );
	m_setGenericCaption.insert( g_wszDataFromWebNextDialogCaption );
	
	// class name
	m_setGenericClassName.insert( g_wszDataActionDialogClassNameOne );
	m_setGenericClassName.insert( g_wszDataActionDialogClassNameTwo );
}

CNLInsertAndDataAction::~CNLInsertAndDataAction( )
{
	DeleteCriticalSection(&m_csDataDialogClose);
	DeleteCriticalSection(&m_csExcelDataActionAllow);
	DeleteCriticalSection(&m_csBrowseFilePath);
	DeleteCriticalSection(&m_csExcelOrPptInsertObjectWndProc);
	DeleteCriticalSection(&m_csDataDialogWndProc);

	// for PPT insert object
	DeleteCriticalSection(&m_csPPTInsertObjectOKButtonDefaultWndProc);
}

CNLInsertAndDataAction& CNLInsertAndDataAction::GetInstance()
{
	static CNLInsertAndDataAction gInstance;
	return gInstance;
}

//////////////////////////////////////////////////////////////////////////
// for Excel or PPT
LRESULT WINAPI CNLInsertAndDataAction::InsertObjectEDTBXProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    CNLInsertAndDataAction& theObject = CNLInsertAndDataAction::GetInstance();

    if ( NULL == theObject.GetInsertObjectEDTBXDefaultWndProc() )
    {
        NLPRINT_DEBUGLOG( L"there are error into my insert object EDTBX message function\n" );
        return -1;
    }

    switch ( nMsg )
    {
    case WM_KEYDOWN:
        {
            // Shielding enter key
            if ( VK_RETURN == wParam )
            {
                return 0;
            }			
        }
        break;
    case WM_SETTEXT:
        {
            if ( (LPARAM)0 != lParam )
            {
                // only click browse button and then the pop dialog closed, this message will send. 			
                theObject.SetExcelInsertBrowseFilePath( (LPWSTR)lParam );
            }
        }
        break;
    default:
        break;
    }
    return CallWindowProc( theObject.GetInsertObjectEDTBXDefaultWndProc(), hWnd, nMsg, wParam, lParam );
}

//////////////////////////////////////////////////////////////////////////
// for PPT
void CNLInsertAndDataAction::initPPTInsertAndDataAction()
{
	SePPTInsertObjectOKButtonWndProc( NULL );
}

void CNLInsertAndDataAction::SePPTInsertObjectOKButtonWndProc( WNDPROC wpPPTInsertObjectOKButtonWndProc )
{
	EnterCriticalSection( &m_csPPTInsertObjectOKButtonDefaultWndProc );
	m_wpPPTInsertObjectOKButtonWndProc = wpPPTInsertObjectOKButtonWndProc;
	LeaveCriticalSection( &m_csPPTInsertObjectOKButtonDefaultWndProc );
}

WNDPROC CNLInsertAndDataAction::GetPPTInsertObjectOKButtonWndProc( )
{
	WNDPROC wpPPTInsertObjectOKButtonWndProc = NULL;
	EnterCriticalSection( &m_csPPTInsertObjectOKButtonDefaultWndProc );
	wpPPTInsertObjectOKButtonWndProc = m_wpPPTInsertObjectOKButtonWndProc;
	LeaveCriticalSection( &m_csPPTInsertObjectOKButtonDefaultWndProc );
	return wpPPTInsertObjectOKButtonWndProc;
}

LRESULT WINAPI CNLInsertAndDataAction::PPTInsertObjectOkButtonProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CNLInsertAndDataAction& theObject = CNLInsertAndDataAction::GetInstance();

	if ( NULL == theObject.GetPPTInsertObjectOKButtonWndProc() )
	{
		NLPRINT_DEBUGLOG( L"there are error into my PPT insert object OK button message process\n" );
		return -1;
	}

	switch ( nMsg )
	{
	case WM_LBUTTONUP:
		{
			HWND hEdit = ::GetDlgItem( ::GetParent( hWnd ), 0X0E ); // RichEdit20W control
			if ( NULL == hEdit || !IsWindowVisible( hEdit ) )
			{
				break;
			}

			wchar_t wszFilePath[MAX_PATHLENGTH] = { 0 };
			::GetWindowTextW( hEdit, wszFilePath, MAX_PATHLENGTH-1 );
			
			// 1. exist local file path or net path( http://, ftp:// )
			if ( PathFileExistsW( wszFilePath ) || !GetNetFileNameByFilePath( wszFilePath ).empty() )
			{
				if ( pep::isPPtApp() )
				{
					CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
					ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wszFilePath, NULL, kOA_INSERT );
					if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
					{
						::PostMessageW( ::GetParent( hWnd ), WM_CLOSE, 0, 0 );
						return 0;		
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return CallWindowProc( theObject.GetPPTInsertObjectOKButtonWndProc(), hWnd, nMsg, wParam, lParam);
}

LRESULT WINAPI CNLInsertAndDataAction::PPTInsertObjectProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
    CNLInsertAndDataAction& theObject = CNLInsertAndDataAction::GetInstance();

    if (  NULL == theObject.GetInsertObjectDialogDefaultWndProc() )
    {
        NLPRINT_DEBUGLOG( L"there are error into my insert object  message function\n" );
        return -1;
    }

    switch ( nMsg )
	{
	case WM_KEYDOWN:
		{
			// Shielding spaces and enter key
			if ( VK_SPACE == wParam || VK_RETURN == wParam )
			{
				return 0;
			}			
		}
	case WM_LBUTTONDOWN:
		{	
			if ( !IsWindowVisible( ::GetDlgItem( hWnd, g_nPPTInsertObjectNetUICtrlNotifySinkID ) ) )  // NetUICtrlNotifySink.
			{
				break;
			}

			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            if ( IsInOKRect( hWnd,pt ) )
			{
				// clicked OK button
				vector<wstring> vecObjectPath;
				wstring wstrInsertPath;
				wstring wstrBrowseFilePath = theObject.GetExcelInsertBrowseFilePath();
				if ( !wstrBrowseFilePath.empty() )
				{			
					vecObjectPath.push_back( wstrBrowseFilePath );	
					wstrInsertPath = wstrBrowseFilePath;
				}
				
				NLPRINT_DEBUGLOG( L"The insert file path is:[%s] \n", wstrInsertPath.c_str() );
				if ( pep::isPPtApp() )
				{
					CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
					ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrInsertPath.c_str(), NULL, kOA_INSERT );
					if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
					{
                        // get the handle of Insert Object.
                        HWND hParent = ::GetParent(hWnd); 
                        if (hParent != NULL)
                        {
                            ::PostMessageW(hParent, WM_CLOSE, 0, 0);
                        }
                        
						return 0;	
					}
				}
			}
		}
		break;
	default:
		break;
	}

    return CallWindowProc( theObject.GetInsertObjectDialogDefaultWndProc(), hWnd, nMsg, wParam, lParam );
}

//////////////////////////////////////////////////////////////////////////
// for excel
void CNLInsertAndDataAction::initExcelInsertAndDataAction()
{
	// for insert object
	SetExcelInsertBrowseFilePath( L"" );
	SetExcelOrPptInsertObjectWndProc( NULL, NULL );
	SetExcelDataDialogWndProc( NULL );

	// for data action
	SetDataFromDialogClose( false, L"" );
	SetExcelDataActionAllow( L"" );
	SetExcelDataDialogWndProc( NULL );
}

LRESULT WINAPI CNLInsertAndDataAction::InsertObjectProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
	CNLInsertAndDataAction& theObject = CNLInsertAndDataAction::GetInstance();

	if (  NULL == theObject.GetInsertObjectDialogDefaultWndProc() )
	{
		NLPRINT_DEBUGLOG( L"there are error into my insert object  message function\n" );
		return -1;
	}

	switch ( nMsg )
	{
	case WM_KEYDOWN:
		{
			// Shielding spaces and enter key
			if ( VK_SPACE == wParam || VK_RETURN == wParam )
			{
				return 0;
			}			
		}
	case WM_LBUTTONDOWN:
		{	
			if ( !IsWindowVisible( ::GetDlgItem( hWnd, g_nInsertObjectEditControlID ) ) )
			{
				break;
			}

			POINT pt = { LOWORD(lParam), HIWORD(lParam) };  
            if ( IsInOKRect( hWnd, pt ) )
			{
				// clicked OK button
				vector<wstring> vecObjectPath;
				wstring wstrInsertPath;
				wstring wstrBrowseFilePath = theObject.GetExcelInsertBrowseFilePath();
				if ( !wstrBrowseFilePath.empty() )
				{			
					vecObjectPath.push_back( wstrBrowseFilePath );	
					wstrInsertPath = wstrBrowseFilePath;
				}
				else
				{
					/* Question: why we can not always get the insert file path from the edit box ?
					*  Answer: because there is a excel bug, user choose a insert file by browse and then change the file path.
					*          Click ok insert the changed file, but in fact the file we select by browse will be insert not the file we changed
					*/
					wchar_t wszFilePath[MAX_PATHLENGTH] = {0};
					::GetWindowTextW( hWnd, wszFilePath, MAX_PATHLENGTH-1 );
					vecObjectPath.push_back( wszFilePath );	
					wstrInsertPath = wszFilePath;
				}
				
				NLPRINT_DEBUGLOG( L"The insert file path is:[%s] \n", wstrInsertPath.c_str() );
				if ( pep::isExcelApp() )
				{
					CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
					ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrInsertPath.c_str(), NULL, kOA_INSERT );
					if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
					{
						::PostMessageW( hWnd, WM_CLOSE, 0, 0 );
						return 0;	
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return CallWindowProc( theObject.GetInsertObjectDialogDefaultWndProc(), hWnd, nMsg, wParam, lParam );
}

LRESULT WINAPI CNLInsertAndDataAction::DataFromDialogProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    OutputDebugStringW(L"=====access DataFromDialogProc====");
	CNLInsertAndDataAction& theObject = CNLInsertAndDataAction::GetInstance();

	if ( NULL == theObject.GetExcelDataDialogDefaultWndProc() )
	{
		NLPRINT_DEBUGLOG( L"there are error into my data from dialog message function\n" );
		return -1;
	}

	switch ( nMsg )
	{
	case WM_DESTROY:
		{
			wchar_t szFileName[MAX_PATHLENGTH] = {0}; 
			if ( IsWin7() || IsWin10()) 
			{
				::GetWindowTextW( GetDlgItem( hWnd, 0X47C ), szFileName, MAX_PATHLENGTH-1 );
				NLPRINT_DEBUGLOG( L"Note: the insert file path is:[%s] \n", szFileName );
			}	

			theObject.SetDataFromDialogClose( true, szFileName );	
		}
		break;
	default:
		break;
	}
	return CallWindowProc( theObject.GetExcelDataDialogDefaultWndProc(), hWnd, nMsg, wParam, lParam);
}

HWND CNLInsertAndDataAction::JudgeParentHandleAndReturnChildHandleW( HWND hParent, const wstring& wstrParentCaption, const wstring& wstrParentClassName,
																						const wstring& wstrChildCaption, const wstring& wstrChildClassName,
																						const int nChildID )
{
	if ( NULL == hParent )
	{
		return NULL;
	}

	if ( !JudgeSpecifieHandleW( hParent, wstrParentClassName,  wstrParentCaption ) )
	{
		return NULL;
	}

	HWND hChild = NULL;
	hChild = ::GetDlgItem( hParent, nChildID );
	if ( NULL == hChild )
	{
		return NULL;
	}

	if ( !JudgeSpecifieHandleW( hChild, wstrChildClassName,  wstrChildCaption ) )
	{
		return NULL;
	}
	return hChild;
}

bool CNLInsertAndDataAction::JudgeSpecifieHandleW( HWND hWnd, const wstring& wstrClassName, const wstring& wstrCaption )
{
	if ( NULL == hWnd )
	{
		return false;
	}

	wchar_t wszCaption[1024] = {0};
	wchar_t wszClassName[1024] = {0};

	::GetWindowTextW( hWnd, wszCaption, 1023 );
	::GetClassNameW( hWnd, wszClassName, 1023 );
	
	NLPRINT_DEBUGLOG( L"hWnd:[0x%x], Caption: get[%s],judge[%s], ClassName: get[%s],judge[%s] \n", hWnd, 
		wszCaption, wstrCaption.c_str(), wszClassName, wstrClassName.c_str() );

	if ( ( !wstrClassName.empty() && _wcsicmp( wszClassName, wstrClassName.c_str() ) ) ||
		( !wstrCaption.empty()   && _wcsicmp( wszCaption, wstrCaption.c_str() ) ) ) 
	{
        OutputDebugStringW(L"=====allen ning return false====");
		return false;
	}
    OutputDebugStringW(L"=====allen ning return true====");
	return true;
}

bool CNLInsertAndDataAction::IsWidownPopUpAfterExcelDataEnd( HWND hWnd )
{ONLY_DEBUG
	if ( NULL == hWnd )
	{
		return false;
	}

	wchar_t wszCaption[1024] = {0};
	wchar_t wszClassName[1024] = {0};

	::GetWindowTextW( hWnd, wszCaption, 1023 );
	::GetClassNameW( hWnd, wszClassName, 1023 );

	NLPRINT_DEBUGLOG( L"hWnd:[0x%x], Caption:[%s], ClassName:[%s] \n", hWnd, wszCaption, wszClassName );

	return ( NLIsRightCaption( wszCaption ) && NLIsRightClassName( wszClassName ) );
}

void CNLInsertAndDataAction::SetExcelOrPptInsertObjectWndProc( WNDPROC wpInsertObjectEDTBXDefaultWndProc, WNDPROC wpInsertObjectDialogDefaultWndProc )
{
	EnterCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
	m_wpInsertObjectEDTBXDefaultWndProc = wpInsertObjectEDTBXDefaultWndProc;
	m_wpInsertObjectDialogDefaultWndProc = wpInsertObjectDialogDefaultWndProc;
	LeaveCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
}
WNDPROC CNLInsertAndDataAction::GetInsertObjectEDTBXDefaultWndProc( )
{
	WNDPROC wpInsertObjectEDTBXDefaultWndProc = NULL;
	EnterCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
	wpInsertObjectEDTBXDefaultWndProc = m_wpInsertObjectEDTBXDefaultWndProc;
	LeaveCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
	return wpInsertObjectEDTBXDefaultWndProc;
}
WNDPROC CNLInsertAndDataAction::GetInsertObjectDialogDefaultWndProc( )
{
	WNDPROC wpInsertObjectDialogDefaultWndProc = NULL;
	EnterCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
	wpInsertObjectDialogDefaultWndProc = m_wpInsertObjectDialogDefaultWndProc;
	LeaveCriticalSection( &m_csExcelOrPptInsertObjectWndProc );
	return wpInsertObjectDialogDefaultWndProc;
}

void CNLInsertAndDataAction::SetExcelDataDialogWndProc( WNDPROC wpDataDialogDefaultWndProc )
{
	EnterCriticalSection( &m_csDataDialogWndProc );
	m_wpDataDialogDefaultWndProc = wpDataDialogDefaultWndProc;
	LeaveCriticalSection( &m_csDataDialogWndProc );
}

WNDPROC CNLInsertAndDataAction::GetExcelDataDialogDefaultWndProc( )
{
	WNDPROC wpDataDialogDefaultWndProc = NULL;
	EnterCriticalSection( &m_csDataDialogWndProc );
	wpDataDialogDefaultWndProc = m_wpDataDialogDefaultWndProc;
	LeaveCriticalSection( &m_csDataDialogWndProc );
	return wpDataDialogDefaultWndProc;
}

void CNLInsertAndDataAction::SetExcelInsertBrowseFilePath( const wstring& wstrBrowseFilePath )
{
	EnterCriticalSection( &m_csBrowseFilePath );
	m_wstrBrowseFilePath = wstrBrowseFilePath;
	LeaveCriticalSection( &m_csBrowseFilePath );
}

wstring CNLInsertAndDataAction::GetExcelInsertBrowseFilePath( )
{
	wstring wstrBrowseFilePath = L"";
	EnterCriticalSection( &m_csBrowseFilePath );
	wstrBrowseFilePath = m_wstrBrowseFilePath;
	LeaveCriticalSection( &m_csBrowseFilePath );
	return wstrBrowseFilePath;
}

void CNLInsertAndDataAction::SetExcelDataActionAllow( const wstring& wstrFilePath )
{
	EnterCriticalSection( &m_csExcelDataActionAllow );
	if ( wstrFilePath.empty() )
	{
		m_bExcelDataActionAllow = true;
	}
	else if ( pep::isExcelApp() )
	{
		CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
		ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrFilePath.c_str(), NULL, kOA_INSERT );		
		m_bExcelDataActionAllow = ( kFSSuccess == stuResult.kFuncStat && kPSDeny != stuResult.kPolicyStat );
	}
	LeaveCriticalSection( &m_csExcelDataActionAllow );
}

bool CNLInsertAndDataAction::GetExcelDataActionAllow( )
{
	bool bExcelInsertActionAllow = true;
	EnterCriticalSection( &m_csExcelDataActionAllow );
	bExcelInsertActionAllow = m_bExcelDataActionAllow;
	LeaveCriticalSection( &m_csExcelDataActionAllow );
	return bExcelInsertActionAllow;
}

void CNLInsertAndDataAction::SetDataFromDialogClose( _In_ bool bDataDialogClose, _In_ const wstring& strFileName )
{
	EnterCriticalSection( &m_csDataDialogClose );
	m_bDataDialogClose = bDataDialogClose;
	m_strFileName = strFileName;
	LeaveCriticalSection( &m_csDataDialogClose );
}

bool CNLInsertAndDataAction::IsDataFromDialogClose( _Out_ wstring& strFileName )
{
	bool bDataDialogClose = false;
	EnterCriticalSection( &m_csDataDialogClose );
	bDataDialogClose = m_bDataDialogClose;
	strFileName = m_strFileName;
	LeaveCriticalSection( &m_csDataDialogClose );
	return bDataDialogClose;
}

bool CNLInsertAndDataAction::IsDataFromDialogClose( )
{
	bool bDataDialogClose = false;
	EnterCriticalSection( &m_csDataDialogClose );
	bDataDialogClose = m_bDataDialogClose;
	LeaveCriticalSection( &m_csDataDialogClose );
	return bDataDialogClose;
}

inline bool CNLInsertAndDataAction::NLIsRightCaption( _In_ const wstring& wstrCaption )
{
	return  m_setGenericCaption.end() != m_setGenericCaption.find( wstrCaption );
}

inline bool CNLInsertAndDataAction::NLIsRightClassName( _In_ const wstring& wstrClassName )
{
	return m_setGenericClassName.end() != m_setGenericClassName.find( wstrClassName );
}