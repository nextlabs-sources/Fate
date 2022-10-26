#include "stdafx.h"

#pragma warning( push )
#pragma warning( disable: 4995 4819 )
#include "eframework\platform\cesdk.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
#include "madCHook_helper.h"
#pragma warning( pop )

#include <boost/lexical_cast.hpp>

#pragma warning(push)
#pragma warning(disable: 6386 6031 6328 6258 6309 6387 6334 4267)  
#include <boost/asio.hpp>
#pragma warning(pop)

using namespace boost::asio;

#pragma warning( push )
#pragma warning( disable : 4996 6326 6246 6385 4328 )
#include <boost/xpressive/xpressive_dynamic.hpp>
#pragma warning(pop)
using namespace boost::xpressive;


#include "NLObMgr.h"
#include "NLInsertAndDataAction.h"
#include "NLSecondaryThreadForPDFMaker.h"

#include "obligations.h"
#include "dllmain.h"
#include "SvrAgent.h"
#include "TalkWithSCE.h"
#include "contentstorage.h"

#include "NLHookAPI.h"
#include "utils.h"
#include "ScreenCaptureAuxiliary.h"
nextlabs::recursion_control hook_control;

////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLHOOKAPI)
//////////////////////////////////////////////////////////////////////////

#define print_string(s)  s?s:" "
#define print_long_string(s) s?s:L" "
#define print_non_string(s) s?*s:0 

namespace{
    const WCHAR* DefaultText = L"NextLabs";
    const char*  charDisplay = "DISPLAY";
    const WCHAR* wcharDisplay = L"DISPLAY";

    const char* QueryFormat = "query:pid=";
    const char* QueryResultFormat = "^query=(\\w{0,})(;displaytext=(\\w{0,}))?";
    const int QueryLength = 64;

    const char* DenyFlag = "deny";
    const char* AllowFlag = "allow";
    const int QueryResultLength = 512;

    const char* SCEServerIP = "127.0.0.1";
    const USHORT SCEServerBasedPort = 20000;
} // ns anonymous

//////////////////////////////////////////////////////////////////////////
// for insert picture
const CLSID CLSID_FileSaveDialog ={0xC0B4E2F3,0xBA21,0x4773,{0x8D,0xBA,0x33,0x5E,0xC9,0x46,0xEB,0x8B}};
const CLSID CLSID_FileOpenDialog ={0xDC1C5A9C, 0xE88A, 0x4dde, {0xA5, 0xA1, 0x60,0xF8,0x2A,0x20,0xAE,0xF7}};
const IID		IID_IFileDialog			 ={0x42F85136, 0xDB7E, 0x439C, {0x85, 0xF1, 0xE4,0x07,0x5D,0x13,0x5F,0xC8}};
const IID		IID_IFileOpenDialog	 ={0xd57c7288,0xd4ad,0x4768,{0xbe,0x02,0x9d,0x96,0x95,0x32,0xd9,0x60}};
const IID		IID_IFileSaveDialog	 ={0x84bccd23,0x5fde,0x4cdb,{0xae,0xa4,0xaf,0x64,0xb8,0x3d,0x78,0xab}};

int CNxtHookAPI::m_nExcelEscape = 0;
map<LPVOID, LPVOID> CNxtHookAPI::m_mapHooks;
HCURSOR CNxtHookAPI::m_hMoveCursor = 0;
HCURSOR CNxtHookAPI::m_hCopyCursor = 0;
HCURSOR CNxtHookAPI::m_hMoveGroupCursor = 0;
HCURSOR CNxtHookAPI::m_hCopyGroupCursor = 0;
WORD CNxtHookAPI::m_wMoveCursorId = 272;
WORD CNxtHookAPI::m_wMoveGroupCursorId = 273;
WORD CNxtHookAPI::m_wCopyCursorId = 274;
WORD CNxtHookAPI::m_wCopyGroupCursorId = 275;
bool CNxtHookAPI::m_bDenyFlag = false;
bool CNxtHookAPI::m_bQueryPCFlag = true;
HHOOK CNxtHookAPI::m_hMouseLL = NULL;
HHOOK CNxtHookAPI::m_hMouse = NULL;
POINT CNxtHookAPI::m_ptOldCursor;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////define function type////////////////////////////////////////////

BOOL (WINAPI* NextBJPrintWindow)(HWND hwnd, HDC hdcBlt, UINT nFlags) = NULL;

BOOL (WINAPI* NextMoveFileExW)(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,DWORD dwFlags) = NULL;

BOOL (WINAPI* NextReplaceFileW)( LPCTSTR lpReplacedFileName, LPCTSTR lpReplacementFileName, LPCTSTR lpBackupFileName,
								DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved ) = NULL;

BOOL ( WINAPI* NextEnableWindow )( HWND hWnd, BOOL bEnable ) = NULL;
INT_PTR (WINAPI* NextDialogBoxIndirectParamW)(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,DLGPROC lpDialogFunc, LPARAM dwInitParam) = NULL;
int (WINAPI* NextMessageBoxExW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId) = NULL;
int (WINAPI* NextMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) = NULL;

HRESULT ( WINAPI* NextCoCreateInstance )( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv ) = NULL;

HRESULT (__stdcall* NextDllGetClassObject)(_In_  REFCLSID rclsid, _In_  REFIID   riid, _Out_ LPVOID   *ppv) = NULL;

HRESULT ( WINAPI* NextShow )( IFileDialog* pThis, HWND hwndOwner ) = NULL;

BOOL (WINAPI* NextShowWindow)(HWND hWnd, int nCmdShow)=NULL;

HANDLE (WINAPI* NextCreateFileW)( LPCWSTR lpFileName,
																 DWORD dwDesiredAccess,
																 DWORD dwShareMode,
																 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
																 DWORD dwCreationDisposition,
																 DWORD dwFlagsAndAttributes,
																 HANDLE hTemplateFile) = NULL;
HRESULT (WINAPI * NextStgOpenStorage)(_In_opt_  const WCHAR* pwcsName,
	_In_opt_ IStorage* pstgPriority,
	_In_ DWORD grfMode,
	_In_opt_z_ SNB snbExclude,
	_In_ DWORD reserved,
	 IStorage** ppstgOpen) = NULL;
HRESULT  (WINAPI *NextOleCreateFromFileEx)(IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,
										   IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
										   IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
										   OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
										   IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj)=NULL;

BOOL (WINAPI* NextCopyFileExW)(IN LPCTSTR lpExistingFileName, IN LPCTSTR lpNewFileName, IN LPPROGRESS_ROUTINE lpProgressRoutine,
							   IN LPVOID lpData, IN LPBOOL pbCancel, IN DWORD dwCopyFlags)=NULL;

HRESULT (WINAPI* NextRegisterDragDrop)(HWND hwnd, IDropTarget * pDropTarget)=NULL;
HRESULT (WINAPI* NextRevokeDragDrop)(HWND hwnd)=NULL;
void (WINAPI* NextDragAcceptFiles) ( HWND hWnd, BOOL fAccept )=NULL;
HRESULT (WINAPI* NextDoDragDrop)(IDataObject * pDataObject,  IDropSource * pDropSource, DWORD dwOKEffect, DWORD * pdwEffect)=NULL;
HRESULT (WINAPI* NextOleSetClipboard)(IDataObject * pDataObj)=NULL;
HRESULT (WINAPI* NextOleGetClipboard)(LPDATAOBJECT * ppDataObj)=NULL;
HANDLE  (WINAPI* NextSetClipboardData)(UINT uFormat, HANDLE hMem)=NULL;

HWND (WINAPI* NextCreateWindowExW)( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, 
																	 int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam
																	 ) = NULL;
int (WINAPI* NextStartDocW)( HDC hdc,CONST DOCINFOW* lpdi )=NULL;
int (WINAPI* NextEndPage)( HDC hdc) = NULL;
int (WINAPI* NextEndDoc)( HDC hdc ) = NULL ;
int (WINAPI* NextEscape)(HDC hdc, int nEscape, int cbInput, LPCSTR lpvIn, LPVOID lpvOut) = NULL;

ULONG (WINAPI* NextMAPISendMail)(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved) = NULL;

HCURSOR (WINAPI* NextLoadCursorW)(HINSTANCE hInstance, LPCWSTR lpCursorName) = NULL;
HCURSOR (WINAPI* NextSetCursor)(HCURSOR hCursor) = NULL;
//////////////////////////////////////////////////////////////////////////

typedef HRESULT (WINAPI* NextIDropTargetDrop) ( IDropTarget *pthis, IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect );
typedef HRESULT (WINAPI* NextQueryContinueDrag)(IDropSource *pThis, BOOL fEscapePressed,DWORD grfKeyState);
//////////////////////////////////////////////////////////////////////////
CNxtHookAPI::CNxtHookAPI(void)
{
	InitializeCriticalSection(&m_csEnableWindow);
	m_pairEnableWindow.first  = NULL;
	m_pairEnableWindow.second = false;

	InitializeCriticalSection(&m_csCopyFile);
}

CNxtHookAPI::~CNxtHookAPI(void)
{
	DeleteCriticalSection(&m_csEnableWindow);
	m_pairEnableWindow.first  = NULL;
	m_pairEnableWindow.second = false;

	DeleteCriticalSection(&m_csCopyFile);
}

CNxtHookAPI& CNxtHookAPI::NLGetInstance()	
{
	static CNxtHookAPI aObject;
	return aObject;
}

void CNxtHookAPI::Init( )
{
	m_mapHooks.clear();

    NLHookFunction("user32.dll", "PrintWindow", Try_PrintWindow, (PVOID*)&NextBJPrintWindow, L"BJPrintWindow");

	/*
		Hook CreateWindow for:
		1. Adobe PDFMake add-in, do deny convert action.
		2. Save the window handle for view overlay
	*/
	NLHookFunction( "user32.DLL", "CreateWindowExW", Try_BJCreateWindowExW, (PVOID*)&NextCreateWindowExW, L"CreateWindowExW" );

	// Hook EnableWindow to deny user change the custom tags
	NLHookFunction( "user32.DLL", "EnableWindow", Try_BJEnableWindow, (PVOID*)&NextEnableWindow, L"BJEnableWindow" );
	
	NLHookFunction("user32.DLL", "DialogBoxIndirectParamW", Try_BJDialogBoxIndirectParamW, (PVOID*)&NextDialogBoxIndirectParamW, L"BJDialogBoxIndirectParamW");
	NLHookFunction( "user32.DLL", "MessageBoxExW", Try_BJMessageBoxExW, (PVOID*)&NextMessageBoxExW, L"BJMessageBoxExW" );
    NLHookFunction( "user32.DLL", "MessageBoxW", Try_BJMessageBoxW, (PVOID*)&NextMessageBoxW, L"BJMessageBoxW" );
	// for BAE-wde first release ,no need to care about encrypt and golden tag , make it as comments

	/*
		Encrypt the file and synchronize the golden tags For:
		1. Word/Excel/PPT edit action
		2. Word/Excel copy action
	*/
	// NLHookFunction( "Kernel32.DLL", "ReplaceFileW", Try_BJReplaceFileW, (PVOID*)&NextReplaceFileW, L"BJReplaceFileW" );

	// NLHookFunction( "Kernel32.DLL", "MoveFileExW", Try_BJMoveFileExW, (PVOID*)&NextMoveFileExW, L"BJMoveFileExW" );


	if (IsWin10())
	{
		//for save as to MSProject include: CONVERT and COPY
		if (kVer2016 <= pep::getVersion())
		{
			NLHookFunction("comdlg32.DLL", "DllGetClassObject", Try_BJDllGetClassObject, (PVOID*)&NextDllGetClassObject, L"BJDllGetClassObject");
		}
		else
		{
			NLHookFunction("combase.DLL", "CoCreateInstance", Try_BJCoCreateInstance, (PVOID*)&NextCoCreateInstance, L"BJCoCreateInstance");
		}
	}
	else
	{
        // WIN7 do not support office 2019
		// On some platforms, such as citrix+win2012+office10, hook api CoCreateInstance will fail; so you need to judge if the hook fails, then hook another api
		if (kVer2016 <= pep::getVersion())
		{
			NLHookFunction("comdlg32.DLL", "DllGetClassObject", Try_BJDllGetClassObject, (PVOID*)&NextDllGetClassObject, L"BJDllGetClassObject");
			if (NextDllGetClassObject == NULL)
			{
				NLHookFunction("ole32.DLL", "CoCreateInstance", Try_BJCoCreateInstance, (PVOID*)&NextCoCreateInstance, L"BJCoCreateInstance");
			}
		}
		else
		{
			NLHookFunction("ole32.DLL", "CoCreateInstance", Try_BJCoCreateInstance, (PVOID*)&NextCoCreateInstance, L"BJCoCreateInstance");
			if (NextCoCreateInstance == NULL)
			{
				NLHookFunction("comdlg32.DLL", "DllGetClassObject", Try_BJDllGetClassObject, (PVOID*)&NextDllGetClassObject, L"BJDllGetClassObject");
			}
		}
	}
	
	// insert & data action: excel/PPT insert->object, data->from access, data->from text
	if ( !pep::isWordApp() )   
	{
		NLHookFunction( "user32.DLL","ShowWindow", Try_BJShowWindow,(PVOID*)&NextShowWindow, L"BJShowWindow" );
	}

	// fix bug: 35092
	if (IsWin10())
	{
		NLHookFunction("KernelBase.DLL", "CreateFileW", Try_BJCreateFileW, (PVOID*)&NextCreateFileW, L"BJCreateFileW");
	}
	else 
	{
		NLHookFunction("Kernel32.DLL", "CreateFileW", Try_BJCreateFileW, (PVOID*)&NextCreateFileW, L"BJCreateFileW");
	}


	/*
	1. word:drag/drop
	2. word/ppt: ctrl+c/v
	*/
	NLHookFunction("ole32.dll","OleCreateFromFileEx",try_BJOleCreateFromFileEx,(PVOID*)&NextOleCreateFromFileEx, L"BJOleCreateFromFileEx");
	NLHookFunction("ole32.dll", "StgOpenStorage", try_BJStgOpenStorage, (PVOID*)&NextStgOpenStorage, L"BJStgOpenStorage");
	/*
	*\ hook olecreatefromfileex can't get the exactly remote file path,so we need to hookup copyfileexw to 
	*	get the real remote file path and do eva.
	*/
	if (!pep::isWordApp())
	{
		NLHookFunction("kernel32.dll","CopyFileExW",try_BJCopyFileExW,(PVOID*)&NextCopyFileExW, L"BJCopyFileExW");
	}
	else
	{// for drag drop
		NLHookFunction ( "shell32.dll", "DragAcceptFiles", try_BJDragAcceptFiles, (PVOID*) &NextDragAcceptFiles,L"BJDragAcceptFiles");
	}

	if (pep::isPPtApp() || pep::isWordApp() )
	{
		// for drag/drop
		NLHookFunction("ole32.DLL","RegisterDragDrop",try_BJRegisterDragDrop,(PVOID*)&NextRegisterDragDrop, L"BJRegisterDragDrop");
		NLHookFunction("ole32.DLL","RevokeDragDrop",try_BJRevokeDragDrop,(PVOID*)&NextRevokeDragDrop, L"BJRevokeDragDrop");
	}

	//hook for drag content 
	//word/ppt: all
	//excel: from excel to ppt/word
	NLHookFunction("ole32.DLL","DoDragDrop",try_BJDoDragDrop, (PVOID*)&NextDoDragDrop, L"BJDoDragDrop");

	//
	// try hook olegetclipboard to handle Office's clipboard
	//
	NLHookFunction("ole32.dll", "OleGetClipboard", try_BJOleGetClipboard, (PVOID*)&NextOleGetClipboard, L"BJGetClipboard");

	//hook for drag content between excel files
	NLHookFunction("ole32.DLL", "OleSetClipboard",try_BJOleSetClipboard,(PVOID*)&NextOleSetClipboard, L"BJSetClipboard");

	// special for Excel content CUT_COPY
	if (pep::isExcelApp())
	{	
		// DialogBoxIndirectParamW use to close access deny dialog window.
		NLHookFunction("user32.dll", "SetClipboardData", try_BJSetClipboardData, (PVOID*)&NextSetClipboardData, L"BJSetClipboardData");
		NLHookFunction("Gdi32.DLL","Escape", try_BJEscape,(PVOID*)&NextEscape, L"Escape");
		NLHookFunction("user32.dll", "LoadCursorW", try_BJLoadCursorW, (PVOID*)&NextLoadCursorW, L"LoadCusorW");
		NLHookFunction("user32.dll", "SetCursor", try_BJSetCursor, (PVOID*)&NextSetCursor, L"SetCursor");
		HookMouseMsg();
	}

	//hook for print to attatch overlay
	NLHookFunction("Gdi32.DLL","StartDocW",/*BJStartDocW*/try_BJStartDocW,(PVOID*)&NextStartDocW,L"StartDocW");
	NLHookFunction("Gdi32.DLL","EndPage",/*BJEndPage*/try_BJEndPage,(PVOID*)&NextEndPage,L"EndPage");
	NLHookFunction("Gdi32.DLL","EndDoc",/*BJEndDoc*/try_BJEndDoc,(PVOID*)&NextEndDoc,L"EndDoc");

	// hook for send address to outlook when send email
	NLHookFunction("mapi32.DLL","MAPISendMail",try_BJMAPISendMail, (PVOID*)&NextMAPISendMail, L"MAPISendMail");

}

void CNxtHookAPI::UnInit()
{
    NLUnHookFunction((PVOID*)&NextBJPrintWindow, L"NextBJPrintWindow");

	NLUnHookFunction( (PVOID*)&NextCreateWindowExW, L"NextCreateWindowExW" );

	NLUnHookFunction( (PVOID*)&NextEnableWindow, L"NextEnableWindow" );
		
	// NLUnHookFunction( (PVOID*)&NextMoveFileExW, L"NextMoveFileExW" );

	// NLUnHookFunction( (PVOID*)&NextReplaceFileW, L"NextReplaceFileW" );
	
	if (NULL != NextCoCreateInstance)
	{
		NLUnHookFunction( (PVOID*)&NextCoCreateInstance, L"NextCoCreateInstance" );
	}

	if (NULL != NextDllGetClassObject)
	{
		NLUnHookFunction( (PVOID*)&NextDllGetClassObject, L"NextDllGetClassObject" );
	}
	
	NLUnHookFileDlg();	

	NLUnHookFunction( (PVOID*)&NextShowWindow, L"NextShowWindow" );

	NLUnHookFunction( (PVOID*)&NextCreateFileW, L"NextCreateFileW" );
	
	NLUnHookFunction((PVOID*)&NextStartDocW, L"NextStartDocW");
	NLUnHookFunction((PVOID*)&NextEndPage, L"NextEndPage");
	NLUnHookFunction((PVOID*)&NextEndDoc, L"NextEndDoc");
	NLUnHookFunction( (PVOID*)&NextOleCreateFromFileEx, L"NextOleCreateFromFileEx" );
	NLUnHookFunction((PVOID*)&NextRegisterDragDrop, L"BJRegisterDragDrop");
	NLUnHookFunction((PVOID*)&NextRevokeDragDrop, L"BJRevokeDragDrop");
	NLUnHookFunction((PVOID*) &NextDragAcceptFiles,L"BJDragAcceptFiles");
	NLUnHookFunction((PVOID*)&NextDoDragDrop, L"BJDoDragDrop");
	NLUnHookFunction((PVOID*)&NextCopyFileExW, L"BJCopyFileExW");
	NLUnHookFunction((PVOID*)&NextOleSetClipboard, L"BJOleSetClipboard");
	
	NLUnHookFunction((PVOID*)&NextMAPISendMail, L"BJMAPISendMail");

	// unhook code
	for (map<LPVOID, LPVOID>::iterator iter = m_mapHooks.begin(); iter != m_mapHooks.end(); ++iter)
	{
		if (iter->second)
		{
			UnhookCode(&iter->second);
		}
	}
	UnhookMouseMsg();
	UnhookMouseLLMsg();
}

bool CNxtHookAPI::Query(std::wstring& DisplayText, DWORD ProcessID)
{
    std::string strQuery = QueryFormat + boost::lexical_cast<std::string>(ProcessID);
    strQuery.resize(QueryLength);

    DWORD SessionID = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &SessionID);

    try
    {
        io_service ios;

        ip::tcp::socket sock(ios);
        ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), SCEServerBasedPort + static_cast<USHORT>(SessionID));

        sock.connect(ep);

        write(sock, buffer(strQuery));

        std::vector<char> str(QueryResultLength);

        read(sock, buffer(str));

        return Parse(DisplayText, &str[0]);
    }
    catch (...)
    {
        return true;			
    }
}

void CNxtHookAPI::NLHookFunction( _In_ LPCSTR pszModule, _In_ LPCSTR pszFuncName, _In_ PVOID pCallbackFunc, _In_ PVOID* ppNextHook, _In_ const wstring& wstrDebugString )
{
	if ( NULL != ppNextHook )
	{
		bool bHook = HookAPI( pszModule, pszFuncName, pCallbackFunc, ppNextHook );
		if ( bHook )
		{
			NLPRINT_DEBUGLOG( L"hook ShowWindow success: debug string[%s] \n", wstrDebugString.c_str() );
		}
		else
		{
			NLPRINT_DEBUGLOG( L"hook ShowWindow failed: debug string[%s] \n", wstrDebugString.c_str() );
			*ppNextHook = NULL;
		}
	}
}

void CNxtHookAPI::NLUnHookFunction( _Inout_ LPVOID* ppFunction, _In_ const wstring& wstrDebugString )
{
	if ( NULL != ppFunction && NULL != *ppFunction )
	{
		bool bUnHook = UnhookAPI( ppFunction );
		if( bUnHook )
		{
			NLPRINT_DEBUGLOG( L"Unhook function success: debug string[%s] \n", wstrDebugString.c_str() );
			*ppFunction = NULL;
		}
		else
		{
			NLPRINT_DEBUGLOG( L"Unhook function failed: debug string[%s] \n", wstrDebugString.c_str() );
		}
	}
}

bool CNxtHookAPI::Query(std::wstring& DisplayText, HWND hWnd)
{
    DWORD ProcessID = 0;

    HWND desktophWnd = GetDesktopWindow();
    if (NULL != hWnd && hWnd != desktophWnd)
    {
        GetWindowThreadProcessId(hWnd, &ProcessID);	
    }
    return Query(DisplayText, ProcessID);
}

bool CNxtHookAPI::Parse(std::wstring& DisplayText, const std::string& str)
{    
    cregex reg = cregex::compile(QueryResultFormat);

    cmatch what;

    try
    {
        regex_match(str.c_str(), what, reg);

        if (what.size() < 4)
        {
            return false;
        }

        const std::string QueryFlag = what[1];

        if (boost::algorithm::iequals(QueryFlag, AllowFlag))
        {
            return true;
        }
        else if (boost::algorithm::iequals(QueryFlag, DenyFlag))
        {
            DisplayText = ScreenCaptureAuxiliary::GetInstance().stringTowsting(what[3]);

            if (DisplayText.empty())
            {
                DisplayText = DefaultText;
            }
        }

        return false;
    }
    catch (...)
    {
        return false;			
    }
}

BOOL WINAPI CNxtHookAPI::Try_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
    return BJPrintWindow(hwnd, hdcBlt, nFlags);
}

BOOL WINAPI CNxtHookAPI::BJPrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
    if( hook_control.is_disabled() )
    {
        return NextBJPrintWindow(hwnd, hdcBlt, nFlags);
    }
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BOOL bRet = NextBJPrintWindow(hwnd, hdcBlt, nFlags);

    if (bRet)
    {
        std::wstring DisplayText;
        bool bAllow = Query(DisplayText, hwnd);
        if (!bAllow)
        {
           ScreenCaptureAuxiliary::GetInstance().ReplaceHDC(DisplayText,hdcBlt);
        }
    }

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////
// for excel close access deny dialog window
INT_PTR WINAPI CNxtHookAPI::Try_BJDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	if(!_AtlModule.connected())
    {
        return NextDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
    }

    __try
    {
        return BJDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
    }
    __except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
    {
        return NextDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
    }
}
INT_PTR WINAPI CNxtHookAPI::BJDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
    if( hook_control.is_disabled() )
    {
        return NextDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
    }
    nextlabs::recursion_control_auto auto_disable(hook_control);

    CNLObMgr& theNLObMgrIns = CNLObMgr::NLGetInstance();
    bool bContainsPreOpenEvaDenyActionResult = theNLObMgrIns.IsContainsPreOpenSpecifyEvaActionResult(kRtPCDeny);

    int nRet = 0;

    if (bContainsPreOpenEvaDenyActionResult)
    {
        // In fact return a fix number is simple but not safe, according test it works
        // The safe way is send the WM_CLOSE message to the dialog, but in this hook function it is more complex
		if (pep::isExcelApp())
        {
            nRet = 2;
        }
        NLPRINT_DEBUGLOG(L"Close access deny dialog because in current context it contains preopen evaluation deny action results, do no pop up dialog and return [%d] directly\n", nRet);
        return nRet;
    }

    NLPRINT_DEBUGLOG(L"Begin Pop up dialog, cdit:[%d]\n", (NULL == hDialogTemplate ? 0 : hDialogTemplate->cdit));
    nRet = NextDialogBoxIndirectParamW(hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam);
    NLPRINT_DEBUGLOG(L"Pop up dialog and return [%d]\n", nRet);
    return nRet;
}

int WINAPI CNxtHookAPI::Try_BJMessageBoxExW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId)
{
    if( !_AtlModule.connected() || !pep::isExcelApp())
    {
        return NextMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
    }

    __try
    {
        return BJMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
    }
    __except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
    {
        return NextMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
    }
}
int WINAPI CNxtHookAPI::BJMessageBoxExW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId)
{
    if( hook_control.is_disabled() )
    {
        return NextMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
    }
    nextlabs::recursion_control_auto auto_disable(hook_control);

    int nRet = IDOK;
    bool bNeedPreventDialog = BJCheckPreventMessageBox(hWnd, lpText, lpCaption, uType, &nRet);
    if (bNeedPreventDialog)
    {
        return nRet;
    }

    NLPRINT_DEBUGLOG(L"Begin Pop up message box\n");
    nRet = NextMessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
    NLPRINT_DEBUGLOG(L"Pop up message box and return [%d]\n", nRet);
    return nRet;
}

int WINAPI CNxtHookAPI::Try_BJMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    if( !_AtlModule.connected() || !pep::isExcelApp())
    {
        return NextMessageBoxW(hWnd, lpText, lpCaption, uType);
    }

    __try
    {
        return BJMessageBoxW(hWnd, lpText, lpCaption, uType);
    }
    __except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
    {
        return NextMessageBoxW(hWnd, lpText, lpCaption, uType);
    }
}

int WINAPI CNxtHookAPI::BJMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
    if( hook_control.is_disabled() )
    {
        return NextMessageBoxW(hWnd, lpText, lpCaption, uType);
    }
    nextlabs::recursion_control_auto auto_disable(hook_control);

    // Fix bug 64696
    int nRet = IDOK;
    bool bNeedPreventDialog = BJCheckPreventMessageBox(hWnd, lpText, lpCaption, uType, &nRet);
    if (bNeedPreventDialog)
    {
        return nRet;
    }

    NLPRINT_DEBUGLOG(L"Begin Pop up message box\n");
    nRet = NextMessageBoxW(hWnd, lpText, lpCaption, uType);
    NLPRINT_DEBUGLOG(L"Pop up message box and return [%d]\n", nRet);
    return nRet;
}

bool WINAPI CNxtHookAPI::BJCheckPreventMessageBox(HWND /*hWnd*/, LPCWSTR /*lpText*/, LPCWSTR lpCaption, UINT uType, _Out_ int* pnDialogRetOut)
{
    static const std::wstring kwstrCaption_MicrosoftWord = L"Microsoft Word";
    static const std::wstring kwstrCaption_MicrosoftExce = L"Microsoft Excel";
    static const std::wstring kwstrCaption_MicrosoftPPT = L"Microsoft PowerPoint";

    bool bNeedPreventRet = false;
    int nDialogRet = IDOK;
    if (NULL != lpCaption)
    {
        if (boost::algorithm::iequals(lpCaption, kwstrCaption_MicrosoftWord.c_str()) || 
            boost::algorithm::iequals(lpCaption, kwstrCaption_MicrosoftExce.c_str()) ||
            boost::algorithm::iequals(lpCaption, kwstrCaption_MicrosoftPPT.c_str()) )
        {
            CNLObMgr& theNLObMgrIns = CNLObMgr::NLGetInstance();
            bool bContainsPreOpenEvaDenyActionResult = theNLObMgrIns.IsContainsPreOpenSpecifyEvaActionResult(kRtPCDeny);

            if (bContainsPreOpenEvaDenyActionResult)
            {
                int nTypeUnify = 0x7 & uType;
                switch (nTypeUnify)
                {
                case MB_OK: { nDialogRet = IDOK; break;}
                case MB_OKCANCEL: { nDialogRet = IDOK; break;}
                case MB_ABORTRETRYIGNORE: { nDialogRet = IDIGNORE; break;}
                case MB_YESNOCANCEL: { nDialogRet = IDYES; break;}
                case MB_YESNO: { nDialogRet = IDYES; break;}
                case MB_RETRYCANCEL: { nDialogRet = IDCANCEL; break;}
                case  MB_CANCELTRYCONTINUE: { nDialogRet = IDCANCEL; break;}
                default: { nDialogRet = IDOK; break;}
                }
                bNeedPreventRet = true;
                NLPRINT_DEBUGLOG(L"Close access deny dialog because in current context it contains preopen evaluation deny action results, do no pop up dialog and return [%d] directly\n", nDialogRet);
            }
        }
    }
    if (NULL != pnDialogRetOut)
    {
        *pnDialogRetOut = nDialogRet;
    }
    return bNeedPreventRet;
}

////////////////////////////Do for excel/PPT insert & data action//////////////////////////////////////////////
BOOL WINAPI CNxtHookAPI::Try_BJShowWindow(HWND hWnd,int nCmdShow)
{
	if( !_AtlModule.connected() )
	{
		return NextShowWindow( hWnd, nCmdShow );
	}

	__try
	{
		return BJShowWindow( hWnd, nCmdShow );
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJShowWindow exception \n" );
		return NextShowWindow( hWnd, nCmdShow );
	}
}

BOOL CNxtHookAPI::BJShowWindow(HWND hWnd,int nCmdShow)
{
#if 0
	// For excel data/insert hook control is disable, we can't process it
	if( hook_control.is_disabled() )
	{	
		NLPRINT_DEBUGLOG( L"hook control is disabled .......... \n" );
		return NextShowWindow( hWnd, nCmdShow );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
#endif

	NLCELOG_ENTER
	// check the parameter
	if ( NULL == hWnd )
	{
		NLCELOG_RETURN_VAL( NextShowWindow( hWnd, nCmdShow ) );
	}	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	
	// get current file path
	wstring wstrCurFilePath = theObMgrIns.NLGetCurActiveFilePath();
	
	// close the default message box pop up when PPT edit and close without save
	if ( theObMgrIns.NLGetPPTUserCancelCloseFlag( wstrCurFilePath ) && pep::isPPtApp() )
	{
		wchar_t wszClassName[1024]={0};
		wchar_t wszCaption[1024]={0};
		GetClassNameW(  hWnd, wszClassName, 1023 );
		GetWindowTextW( hWnd, wszCaption,   1023 );

		bool bClose=false;
		OfficeVer ver = pep::getVersion();
		if ( kVer2007 == ver &&
			boost::algorithm::istarts_with( wszClassName, L"#32770" ) &&
			boost::algorithm::istarts_with( wszCaption,   L"Microsoft Office PowerPoint" ) )
		{
			bClose = true;
		}
		else if ( 
			(kVer2010 == ver || kVer2013 == ver || kVer2016 == ver) &&
			boost::algorithm::istarts_with( wszClassName, L"NUIDialog" ) &&
			boost::algorithm::istarts_with( wszCaption,   L"Microsoft PowerPoint" ) )
		{
			bClose = true;
		}

		if ( bClose )
		{
			theObMgrIns.NLSetPPTUserCancelCloseFlag( wstrCurFilePath, false );
			
			::PostMessage( hWnd, WM_CLOSE, 0, 0 );
			return NextShowWindow( hWnd, SW_HIDE );
		}
	}
	
	// process word/excel/ppt insert action
	// check insert action
	ActionFlag stuActionFlag;
	if ( !theObMgrIns.NLGetActionStartUpFlag( wstrCurFilePath, kOA_INSERT, &stuActionFlag ) )
	{
		NLCELOG_RETURN_VAL( NextShowWindow( hWnd, nCmdShow ) );
	}	
	NLPRINT_DEBUGLOG( L"insert action start up: current active file path[%s] \n", wstrCurFilePath.c_str() );

	InsertType emInsertType = stuActionFlag.unActionExtraFlag.emExtraInsertType;
	CNLInsertAndDataAction& theInsertObject = CNLInsertAndDataAction::GetInstance();
	
	if ( pep::isExcelApp() )
	{
		// do excel insert or data action	
		// Data->From Access->New Source, the main data dialog will destroy and then show it again
		if ( ( NULL == theInsertObject.GetInsertObjectEDTBXDefaultWndProc() )  &&  ( NULL == theInsertObject.GetInsertObjectDialogDefaultWndProc() ) && 
			 ( ( NULL == theInsertObject.GetExcelDataDialogDefaultWndProc() ) || 
			 ( ( NULL != theInsertObject.GetExcelDataDialogDefaultWndProc() )  && ( kInsertDataFromAccess_ExcelOnly == emInsertType ) && ( theInsertObject.IsDataFromDialogClose() ) // for new source button  
					       )
				    ) 
			 )
		{
			NLPRINT_DEBUGLOG( L"this is Excel insert action \n" );

			bool bIsHandleRight = false;
			HWND hParent = NULL;
			hParent = ::GetParent( hWnd );
			if ( NULL == hParent )
			{
				NLPRINT_DEBUGLOG( L"Local variables are: wstrCurFilePath=%ls, stuActionFlag=%p, emInsertType=%d \n", wstrCurFilePath.c_str(),&stuActionFlag,emInsertType );
				NLCELOG_RETURN_VAL( NextShowWindow(hWnd, nCmdShow) );
			}
			
			// we do insert object by hook showwindow and message loops, do deny in the message loops;
			// we do data from access/text by hook showwindow, createfilew and message loops, get file path in the create file and deny in show window, message loop used to set flags
			switch ( emInsertType )
			{
			case kInsertObj:
				{
					HWND hChildEDTBX = theInsertObject.JudgeParentHandleAndReturnChildHandleW( hParent, g_wszInsertObjectDialogCaption, g_wszInsertObjectDialogClassName,
						g_wszInsertObjectEditControlCaption, g_wszInsertObjectEditControlClassName, g_nInsertObjectEditControlID );

					if ( NULL != hChildEDTBX )
					{					
						theInsertObject.SetExcelOrPptInsertObjectWndProc( (WNDPROC)::SetWindowLongPtrW( hChildEDTBX, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::InsertObjectEDTBXProc ),
							(WNDPROC)::SetWindowLongPtrW( hParent, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::InsertObjectProc ) );

						if ( ( NULL==theInsertObject.GetInsertObjectEDTBXDefaultWndProc() ) || ( NULL==theInsertObject.GetInsertObjectDialogDefaultWndProc() ) )
						{
							NLPRINT_DEBUGLOG( L"data from access SetWindowLongPtrW Failed\n" );
#pragma chMSG( "If we set window long failed, revert the insert falg or not ?" )
						}
						else
						{
							NLPRINT_DEBUGLOG( L"set window long (PTR) succeed\n" );
							// revert the flag, excel insert object process end here, we do deny in the insert dialog message process
							stuActionFlag.unActionExtraFlag.emExtraInsertType = kInsertUnkown;
							theObMgrIns.NLSetActionStartUpFlag( wstrCurFilePath, kOA_INSERT, false, &stuActionFlag );
						}
					}					
				}
				break;
			case kDataFromText_ExcelOnly:
				{															
					if ( IsXp() )
					{
						bIsHandleRight = theInsertObject.JudgeSpecifieHandleW( hParent, g_wszDataActionDialogClassNameOne, g_wszDataFromTextFirstDialogCaption );
					}
					else if ( IsWin7() || IsWin10() )
					{
						bIsHandleRight = theInsertObject.JudgeSpecifieHandleW( hParent, g_wszDataActionDialogClassNameTwo, g_wszDataFromTextFirstDialogCaption );
					}
				}
				break;
			case kInsertDataFromAccess_ExcelOnly:
				{	
					if ( IsXp() )
					{	
						bIsHandleRight = theInsertObject.JudgeSpecifieHandleW( hParent, g_wszDataActionDialogClassNameOne, g_wszDataFromAccessFirstDialogCaption );
					}
					else if ( IsWin7() || IsWin10())
					{
						bIsHandleRight = theInsertObject.JudgeSpecifieHandleW( hParent, g_wszDataActionDialogClassNameTwo, g_wszDataFromAccessFirstDialogCaption );
					}
				}
				break;
			default:
				break;
			}
			
			NLPRINT_DEBUGLOG( L"---------bIsHandleRight:[%s] \n", bIsHandleRight?L"true":L"false" );
			if ( bIsHandleRight )
			{	
				// process data from access/text
                if ( NULL == theInsertObject.GetExcelDataDialogDefaultWndProc())
                {
                    theInsertObject.SetExcelDataDialogWndProc( (WNDPROC)::SetWindowLongPtrW( hParent, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::DataFromDialogProc ) );
                    if ( NULL == theInsertObject.GetExcelDataDialogDefaultWndProc() )
                    {
                        NLPRINT_DEBUGLOG( L"data from access SetWindowLongPtrW Failed\n" );
                    }

                    theInsertObject.SetDataFromDialogClose( false, L"" ); 
                    bIsHandleRight = false; 
                }
			}					
		}
		// if the file path is denied, we close the next dialog of data from text/access  
		else if ( ( theInsertObject.IsDataFromDialogClose() ) )
		{	
			HWND hParent = ::GetParent( hWnd );
			
			// the wizard dialog Caption of data from text is "Text Import Wizard - Step 1 of 3", but in fact, it's "Convert Text to Columns Wizard - Step 1 of 3".
			if ( theInsertObject.IsWidownPopUpAfterExcelDataEnd( hParent ) )
			{
				// here data from text, data from access end 
				BOOL bAllow = TRUE;
				if ( !theInsertObject.GetExcelDataActionAllow() )
				{
					::SendMessageW( hParent, WM_CLOSE, 0, 0 ); // deny data from text/access
					bAllow = FALSE;
				}
				theInsertObject.SetDataFromDialogClose( false, wstrCurFilePath );
				stuActionFlag.unActionExtraFlag.emExtraInsertType = kInsertUnkown;
				theObMgrIns.NLSetActionStartUpFlag( wstrCurFilePath, kOA_INSERT, false, &stuActionFlag ); // process data from access/text end
				
				NLPRINT_DEBUGLOG( L"Local variables are: wstrCurFilePath=%ls, stuActionFlag=%p, emInsertType=%d \n", wstrCurFilePath.c_str(),&stuActionFlag,emInsertType );
				NLCELOG_RETURN_VAL( bAllow ? NextShowWindow(hWnd, nCmdShow) : FALSE );
			}
		}
		NLPRINT_DEBUGLOG( L"Local variables are: wstrCurFilePath=%ls, stuActionFlag=%p, emInsertType=%d \n", wstrCurFilePath.c_str(),&stuActionFlag,emInsertType );
		NLCELOG_RETURN_VAL( NextShowWindow(hWnd, nCmdShow) );     
	}
	else if ( pep::isPPtApp() )
	{	
		if ( ( kInsertObj == emInsertType ) && ( NULL == theInsertObject.GetPPTInsertObjectOKButtonWndProc() ) )
		{			
			HWND hParent = ::GetParent( hWnd );
			if ( NULL != hParent )
			{
				// OK 0X01, RichEdit20W 0X0E                
			    HWND hChildOKButton = theInsertObject.JudgeParentHandleAndReturnChildHandleW( hParent, L"Insert Object", L"#32770", L"OK", L"Button", 0X01 );
				if ( NULL != hChildOKButton )
				{
					theInsertObject.SePPTInsertObjectOKButtonWndProc( (WNDPROC)SetWindowLongPtrW( hChildOKButton, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::PPTInsertObjectOkButtonProc ) );	

					if ( NULL == theInsertObject.GetPPTInsertObjectOKButtonWndProc() )
					{
						NLPRINT_DEBUGLOG( L"PPT insert object OK button SetWindowLongPtrW Failed\n" );
					}
					
					// revert the flag
					theObMgrIns.NLSetActionStartUpFlag( wstrCurFilePath, kOA_INSERT, false );
					NLPRINT_DEBUGLOG( L"Local variables are: wstrCurFilePath=%ls, stuActionFlag=%p, emInsertType=%d \n", wstrCurFilePath.c_str(),&stuActionFlag,emInsertType );
					NLCELOG_RETURN_VAL( NextShowWindow(hWnd, nCmdShow) );
				} 
                else if ( kVer2016 == pep::getVersion() || kVer2013 == pep::getVersion() ) // for PPt 2013\2016 insert object(which UI is different from PPT 2010).
                {
                    NLPRINT_DEBUGLOG(L"this is PPT insert object action \n");

                    // get parent(NetUIHWND) handle, used to listen click OK button, hWnd is the handle of NetUICtrlNotifySink
                    HWND hParent = NULL;
                    hParent = ::GetParent( hWnd );
                    if ( NULL == hParent )
                    {
                        NLPRINT_DEBUGLOG( L"Local variables are: wstrCurFilePath=%ls, stuActionFlag=%p, emInsertType=%d \n", wstrCurFilePath.c_str(),&stuActionFlag,emInsertType );
                        NLCELOG_RETURN_VAL( NextShowWindow(hWnd, nCmdShow) );
                    }

                    // get child -- RichEdit, used to get the file path.
                    HWND hChildEDTBX = theInsertObject.JudgeParentHandleAndReturnChildHandleW( hWnd, g_wszPPTInsertObjectDialogCaption, g_wszPPTInsertObjectDialogClassName,
                        g_wszPPTInsertObjectEditControlCaption, g_wszPPTInsertObjectEditControlClassName, g_nPPTInsertObjectEditControlID );

                    if ( NULL != hChildEDTBX )
                    {					
                        theInsertObject.SetExcelOrPptInsertObjectWndProc( (WNDPROC)::SetWindowLongPtrW( hChildEDTBX, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::InsertObjectEDTBXProc ),
                            (WNDPROC)::SetWindowLongPtrW( hParent, GWLP_WNDPROC, (LONG_PTR)CNLInsertAndDataAction::PPTInsertObjectProc ) );

                        if ( ( NULL==theInsertObject.GetInsertObjectEDTBXDefaultWndProc() ) || ( NULL==theInsertObject.GetInsertObjectDialogDefaultWndProc() ) )
                        {
                            NLPRINT_DEBUGLOG( L"data from access SetWindowLongPtrW Failed\n" );
#pragma chMSG( "If we set window long failed, revert the insert falg or not ?" )
                        }
                        else
                        {
                            NLPRINT_DEBUGLOG( L"set window long (PTR) succeed\n" );
                            // revert the flag, ppt insert object process end here, we do deny in the insert dialog message process
                            stuActionFlag.unActionExtraFlag.emExtraInsertType = kInsertUnkown;
                            theObMgrIns.NLSetActionStartUpFlag( wstrCurFilePath, kOA_INSERT, false, &stuActionFlag );
                        }
                    }
                }
			}			
		}
                
	}
	NLCELOG_RETURN_VAL( NextShowWindow(hWnd, nCmdShow) );
}

HANDLE WINAPI CNxtHookAPI::Try_BJCreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
																			 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if( !_AtlModule.connected() )
	{
		return NextCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	__try
	{
		return BJCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		return NextCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}
}

HANDLE CNxtHookAPI::BJCreateFileW( LPCWSTR lpFileName, 
								DWORD dwDesiredAccess, 
								DWORD dwShareMode, 
								LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								DWORD dwCreationDisposition, 
								DWORD dwFlagsAndAttributes, 
								HANDLE hTemplateFile)
{
	if( hook_control.is_disabled() )
	{
		return NextCreateFileW(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

	static std::wstring wstrCurProcessFolder = GetCurProcessFolderPath();
	std::wstring kwstrCreateFilePath = (NULL == lpFileName) ? L"" : lpFileName;

	// Check office file open rights
    if ((3 < kwstrCreateFilePath.length()) && (dwCreationDisposition == OPEN_EXISTING) && (dwDesiredAccess & GENERIC_READ))
	{
		const bool kbIsExcelFile = pep::isExcelFile(kwstrCreateFilePath);
		if (kbIsExcelFile)
		{
			bool isLocal = IsLocalDriver(kwstrCreateFilePath);
			/* For app data temp files:
            * If open a file from net place or outlook attachment, there is only temp, cannot ignore for this case
            * But in pep we cannot known what the function of the temp file, we need handle all and exclude by some special conditions according test
            * Note: do not ignore hidden files. See BUG: 64956, 64950, 64954
            */
            // boost::algorithm::icontains(kwstrCreateFilePath, L"\\AppData\\")
            if (/*boost::algorithm::icontains(kwstrCreateFilePath, L"\\AppData\\") ||*/
                boost::algorithm::icontains(kwstrCreateFilePath, L"~$") ||
                boost::algorithm::icontains(kwstrCreateFilePath, wstrCurProcessFolder) ||
                (!isLocal && boost::algorithm::icontains(kwstrCreateFilePath, L"\\DavWWWRoot\\")))
            {
                // Temp file, ignore
                // For excel deny these files, the process will be hanged in opening
                NLPRINT_DEBUGLOG(L"Ignore temp file:[%s] no need do preopen check for this file\n", kwstrCreateFilePath.c_str());
            }
            else
            {
                NLPRINT_DEBUGLOG(L"IsOfficeFileOpen:[%s:%s], CreateFileInfo: "
                    L"filePath:[%s], dwDesiredAccess:[0x%x], dwShareMode:[0x%x], lpSecurityAttributes:[0x%x], dwCreationDisposition:[0x%x], dwFlagsAndAttributes:[0x%x], hTemplateFile:[0x%x]\n", 
                    kbIsExcelFile ? L"true" : L"false", kwstrCreateFilePath.c_str(), lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

                bool bInited = theObMgrIns.NLGetObMgrInitializeFlag(kwstrCreateFilePath);
                if (!bInited)
                {
                    ActionResult emPreOpenEvaActionResult = theObMgrIns.NLGetPreOpenEvaActionResult(kwstrCreateFilePath, true, kRtPCAllow);
                    if (kRtUnknown == emPreOpenEvaActionResult)
                    {
                        // Only do once
                        // For word and excel, we cannot always to read tags and query policy at here, the process will be crash
                        // According test, first time read is save, we must cache it
                        emPreOpenEvaActionResult = theObMgrIns.NLGetEvaluationResult(kwstrCreateFilePath.c_str(), kOA_OPEN, L"");
                        if ((kRtPCDeny != emPreOpenEvaActionResult) && (kRtPCAllow != emPreOpenEvaActionResult))
                        {
                            // PPT ~ temp file cannot success evaluation and deny
                            NLPRINT_DEBUGLOG(L"!!!Please check, the file:[%s] preopen evalation result is:[0x%x], this is an unexpect result, default we remark the result as PCAllow\n", 
                                kwstrCreateFilePath.c_str(), emPreOpenEvaActionResult);
                            emPreOpenEvaActionResult = kRtPCAllow;
                        }
                        theObMgrIns.NLSetPreOpenEvaActionResult(kwstrCreateFilePath, emPreOpenEvaActionResult);
                    }

                    NLPRINT_DEBUGLOG(L"Do file:[%s] preopen evaluation in create file with policy result:[%d]\n", kwstrCreateFilePath.c_str(), emPreOpenEvaActionResult);
                    if (kRtPCDeny == emPreOpenEvaActionResult)
                    {
						NLPRINT_DEBUGLOG(L"by return ERROR_ACCESS_DENIED to deny :[%s]open\n", kwstrCreateFilePath.c_str());
						::SetLastError(ERROR_ACCESS_DENIED);
						return INVALID_HANDLE_VALUE;
					}
                }
            }
		}
	}
	
	// get current file path
	wstring wstrCurFilePath = theObMgrIns.NLGetCurActiveFilePath();

	// special for picture
	if (pep::isWordApp() && lpFileName != NULL && wcslen(lpFileName) > 4 && dwCreationDisposition == OPEN_EXISTING)
	{
		// for word insert file
		if (boost::icontains(lpFileName, L"\\Local\\Microsoft\\Windows\\INetCache\\Content.Word\\") &&
			(
				boost::iends_with(lpFileName, L".jpg") ||
				boost::iends_with(lpFileName, L".jpeg") ||
				boost::iends_with(lpFileName, L".png") ||
				boost::iends_with(lpFileName, L".bmp") ||
				boost::iends_with(lpFileName, L".gif") ||
				boost::iends_with(lpFileName, L".tif") ||
				boost::iends_with(lpFileName, L".tiff") 
			)

			)
		{
			if (kRtPCDeny == theObMgrIns.NLGetEvaluationResult(lpFileName, kOA_CONVERT, L""))
			{
				::DeleteFileW(lpFileName);
				::SetLastError(ERROR_ACCESS_DENIED);
				return INVALID_HANDLE_VALUE;
			}

		}
	}
	// special for picture
	else if (pep::isPPtApp() && lpFileName != NULL && wcslen(lpFileName) > 4 && dwCreationDisposition == OPEN_EXISTING )
	{
		// for ppt insert file
		if ( (
				boost::icontains(lpFileName, L"\\Local\\Microsoft\\Windows\\INetCache\\Content.MSO\\ppt\\") ||
				boost::icontains(lpFileName,L"\\Local\\Temp\\") ) 
			&&
			(
				boost::iends_with(lpFileName, L".jpg") ||
				boost::iends_with(lpFileName, L".jpeg") ||
				boost::iends_with(lpFileName, L".png") ||
				boost::iends_with(lpFileName, L".bmp") ||
				boost::iends_with(lpFileName, L".gif") ||
				boost::iends_with(lpFileName, L".tif") ||
				boost::iends_with(lpFileName, L".tiff") )
			) 
		{
			if (kRtPCDeny == theObMgrIns.NLGetEvaluationResult(lpFileName, kOA_CONVERT, L""))
			{
				::DeleteFileW(lpFileName);
				::SetLastError(ERROR_ACCESS_DENIED);
				return INVALID_HANDLE_VALUE;
			}

		}
	}
	// for legecy code
	else if (pep::isExcelApp())
	{
		ActionFlag stuActionFlag(kInsertUnkown);
		if (!theObMgrIns.NLGetActionStartUpFlag(wstrCurFilePath, kOA_INSERT, &stuActionFlag))
		{
			return NextCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		}
		InsertType emInsertType = stuActionFlag.unActionExtraFlag.emExtraInsertType;

		// check file path
		if (lpFileName != NULL && wcslen(lpFileName) > 4)
		{
			// data from text/access for win7
			if (pep::isExcelApp() && (kInsertDataFromAccess_ExcelOnly == emInsertType || kDataFromText_ExcelOnly == emInsertType))
			{
				CNLInsertAndDataAction& theInsertObject = CNLInsertAndDataAction::GetInstance();

				wstring wstrFileName = L"";
				if (theInsertObject.IsDataFromDialogClose(wstrFileName))
				{
					// Filter temporary file path
					// server share temporary file path: \\server\\PIPE\\, I can't accurate the server name and filter it
					wchar_t wszTmpFilePath[MAX_PATH + 1] = { 0 };
					::SHGetSpecialFolderPath(NULL, wszTmpFilePath, CSIDL_INTERNET_CACHE, false);

					const wstring wstrAutoexec = L"c:\\autoexec.bat";     // this only XP, net file use this file
					const wstring wstrDataReady = L"DBWID_DATA_READY";

					// we don't care the temporary files
					if (!_wcsnicmp(lpFileName, wszTmpFilePath, wcslen(wszTmpFilePath)) ||
						!_wcsnicmp(lpFileName, wstrAutoexec.c_str(), wstrAutoexec.length()) ||
						!_wcsnicmp(lpFileName, wstrDataReady.c_str(), wstrDataReady.length()))
					{
						// is temporary file path
						return NextCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
					}

					// judge the path, if it is valid
					bool bIsValidFilePath = false;
					wstring wstrFileNameFromPath = L"";
					if (!(wstrFileNameFromPath = GetNetFileNameByFilePath(lpFileName)).empty() && !PathFileExistsW(lpFileName))
					{
						// is a net valid file path, we don't attention the web page
						bIsValidFilePath = true;
					}
					else if (!(wstrFileNameFromPath = GetFileName(lpFileName)).empty())
					{
						if (PathFileExistsW(lpFileName) && !PathIsDirectoryW(lpFileName))
						{
							// is a local valid file path
							bIsValidFilePath = true;
						}
					}

					if (bIsValidFilePath)
					{
						bool bIsRightFilePath = true;
						if (IsWin7()|| IsWin10())
						{
							bIsRightFilePath = (0 == _wcsnicmp(wstrFileNameFromPath.c_str(), wstrFileName.c_str(), wstrFileName.length()));
						}

						if (bIsRightFilePath)
						{
							stuActionFlag.unActionExtraFlag.emExtraInsertType = kInsertUnkown;
							theObMgrIns.NLSetActionStartUpFlag(wstrCurFilePath, kOA_INSERT, true, &stuActionFlag); // process data from access/text continue but the insert type no used
							theInsertObject.SetExcelDataActionAllow(lpFileName);	// for excel data from text/access we do evaluation here and do deny at show window.
						}
					}
				}
			}
		}
	}
	return NextCreateFileW( lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////Deny user change the custom tags////////////////////////////////////////////
BOOL WINAPI CNxtHookAPI::Try_BJEnableWindow(HWND hWnd, BOOL bEnable)
{
	if( !_AtlModule.connected() )
	{
		return NextEnableWindow( hWnd, bEnable );
	}

	__try
	{
		return BJEnableWindow( hWnd, bEnable );
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJEnableWindow exception \n" );
		return NextEnableWindow( hWnd, bEnable );
	}
}

BOOL CNxtHookAPI::BJEnableWindow(HWND hWnd, BOOL bEnable)
{
	if( hook_control.is_disabled() )
	{
		return NextEnableWindow( hWnd, bEnable );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

NLCELOG_ENTER
	// check parameter
	if ( NULL == hWnd || !bEnable)
	{
		NLCELOG_RETURN_VAL( NextEnableWindow( hWnd, bEnable ) )
	}
	
	// check classify action flag of the current active file 
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrCurActiveFilePath = theObMgrIns.NLGetCurActiveFilePath();
	
	if ( !theObMgrIns.NLGetActionStartUpFlag( wstrCurActiveFilePath, kOA_CLASSIFY ) )
	{
		// no start up, the file not opened
		NLCELOG_RETURN_VAL( NextEnableWindow( hWnd, bEnable ) )
	}
	
	// check window if user change the file custom tags
	CNxtHookAPI& hookObj = CNxtHookAPI::NLGetInstance();
	wchar_t wszCaption[1024]  ={0};
	wchar_t wszClassName[1024]={0};
	
	::GetClassName( hWnd, wszClassName, 1023 );
	if ( 0 == _wcsicmp( L"Button", wszClassName ) )
	{
		::GetWindowText( hWnd, wszCaption, 1023 );

		bool bAddModify = true; //to distinguish Add/Modify button and Delete button
		bool bEditBtn   = false;
		if ( 0 == _wcsicmp( wszCaption, L"&Modify" ) || 0 == _wcsicmp( wszCaption, L"&Add" ) )
		{
			bEditBtn = true;				
		}
		else if ( 0 == _wcsicmp( wszCaption, L"&Delete" ) )
		{
			bAddModify = false;
			bEditBtn = true;
		}

		if ( bEditBtn )
		{
			HWND hWndP = GetParent( hWnd );
			if ( NULL == hWndP )
			{
				NLCELOG_RETURN_VAL( NextEnableWindow( hWnd, bEnable ) )
			}

			GetClassName( hWndP, wszClassName, 1023 );
			GetWindowText( hWndP, wszCaption, 1023 );
			if ( 0 == _wcsicmp( L"#32770", wszClassName) && 0 == _wcsicmp( L"Custom", wszCaption ) )
			{
				HWND hWndPP = GetParent( hWndP );
				if ( NULL != hWndPP )
				{
					GetClassName( hWndPP, wszClassName, 1023 );
					GetWindowText( hWndPP, wszCaption, 1023 );
					if ( 0 == _wcsicmp( L"#32770", wszClassName ) && boost::algorithm::iends_with( wszCaption, L" Properties" ) )
					{
						NLPRINT_DEBUGLOG( L"\n-------------enter enablewindow, do classify action \n" );
						//pass the button handle and its parent button and grandparent button handle
						bool bIsNeedEnableWindow = false;
						if ( !hookObj.NLGetEnableWindowChnageAttributeFlag( hWndPP, bIsNeedEnableWindow ) )
						{
							// the window has changed, we should do evaluation again
							bIsNeedEnableWindow = 
								(kRtPCDeny != theObMgrIns.NLGetEvaluationResult(wstrCurActiveFilePath.c_str(), kOA_CLASSIFY, L"", CE_NOISE_LEVEL_USER_ACTION, L"User"));
							hookObj.NLSetEnableWindowChnageAttributeFlag( hWndPP, bIsNeedEnableWindow ); // cache
							theObMgrIns.NLSetClassifyCustomTagsFlag( wstrCurActiveFilePath, bIsNeedEnableWindow ); // enable window means user can classify the custom tags 
						}
						bIsNeedEnableWindow ? bEnable = TRUE : bEnable = FALSE ;	// policy deny disable the button
					}
				}
			}				
		}	
	}
	NLCELOG_RETURN_VAL( NextEnableWindow( hWnd, bEnable ) )
}

void CNxtHookAPI::NLSetEnableWindowChnageAttributeFlag( _In_ HWND hWnd, _In_ bool bIsNeedEnable )
{
	EnterCriticalSection(&m_csEnableWindow);
	m_pairEnableWindow.first = hWnd;
	m_pairEnableWindow.second = bIsNeedEnable;
	LeaveCriticalSection(&m_csEnableWindow);
}

bool CNxtHookAPI::NLGetEnableWindowChnageAttributeFlag( _In_ HWND hWnd, _Out_ bool& bIsNeedEnable )
{
	bool bIsSameWindow = false;
	EnterCriticalSection(&m_csEnableWindow);
	bIsSameWindow = m_pairEnableWindow.first == hWnd;
	bIsNeedEnable = m_pairEnableWindow.second;
	LeaveCriticalSection(&m_csEnableWindow);
	return bIsSameWindow;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////Encrypt file and synchronize golden tags///////////////////////////////////////////
/*
	1. win7 Word/Excel/PPT edit
	2. win7 Word/Excel copy
	3. XP Word/Excel/PPT edit
	4. XP Excel copy
*/
BOOL WINAPI CNxtHookAPI::Try_BJReplaceFileW( LPCTSTR lpReplacedFileName, LPCTSTR lpReplacementFileName, LPCTSTR lpBackupFileName,
																						DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved )
{
	if( !_AtlModule.connected() )
	{
		return NextReplaceFileW( lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved );
	}

	__try
	{
		return BJReplaceFileW( lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved );
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJReplaceFileW exception \n" );
		return NextReplaceFileW( lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved );
	}
}

BOOL CNxtHookAPI::BJReplaceFileW( LPCTSTR lpReplacedFileName,   // source file
																 LPCTSTR lpReplacementFileName, // temp file
																 LPCTSTR lpBackupFileName,
																 DWORD dwReplaceFlags,
																 LPVOID lpExclude,
																 LPVOID lpReserved)
{
	if( hook_control.is_disabled() )
	{
		return NextReplaceFileW(lpReplacedFileName,lpReplacementFileName,lpBackupFileName,dwReplaceFlags,lpExclude,lpReserved);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if ( NULL == lpReplacedFileName || NULL == lpReplacementFileName )
	{
		return NextReplaceFileW(lpReplacedFileName,lpReplacementFileName,lpBackupFileName,dwReplaceFlags,lpExclude,lpReserved);
	}
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	if ( theObMgrIns.NLGetEncryptRequirementFlag( lpReplacedFileName ) )
	{ONLY_DEBUG
		NLPRINT_DEBUGLOG( L"---- Replace file path: lpReplacedFileName, source:[%s], lpReplacementFileName temp:[%s] \n", lpReplacedFileName, lpReplacementFileName );
		if ( theObMgrIns.NLEncryptFile( lpReplacementFileName ) )
		{
			bool bStart = theObMgrIns.NLStartSynchronizeGoldenTags( lpReplacedFileName, true, lpReplacementFileName );
			if ( bStart )
			{
				theObMgrIns.NLSetEncryptRequirementFlag( lpReplacedFileName, false );
			}
		}
	}
	return NextReplaceFileW(lpReplacedFileName,lpReplacementFileName,lpBackupFileName,dwReplaceFlags,lpExclude,lpReserved);
}


BOOL WINAPI CNxtHookAPI::Try_BJMoveFileExW(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags)
{
	if( !_AtlModule.connected())
	{
		return NextMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	}

	__try
	{
		return BJMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJMoveFileExW exception \n" );
		return NextMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	}
}

BOOL CNxtHookAPI::BJMoveFileExW( LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName,DWORD dwFlags )
{
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance( );
	if ( theObMgrIns.NLGetEncryptRequirementFlag( lpNewFileName ) )
	{
	NLPRINT_DEBUGLOG( L"---- Move File: lpExistingFileName, source:[%s], lpNewFileName temp:[%s] \n", lpExistingFileName, lpNewFileName );
	if ( theObMgrIns.NLEncryptFile( lpExistingFileName ) )
	{
		bool bStart = theObMgrIns.NLStartSynchronizeGoldenTags( lpNewFileName, true, lpExistingFileName );
		if ( bStart )
		{
			theObMgrIns.NLSetEncryptRequirementFlag( lpNewFileName, false );
		}
	}
	}
	return NextMoveFileExW(lpExistingFileName,lpNewFileName,dwFlags);
}
//////////////////////////////////////////////////////////////////////////

/////////////////////////IFileDialog interface, save as/insert dialog/////////////////////////////////////////////////
HRESULT WINAPI CNxtHookAPI::Try_BJCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv )
{
	if (!_AtlModule.connected())
	{
		return NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
	}

	__try
	{
		return BJCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJCoCreateInstance exception \n" );
		return NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
	}
}

HRESULT WINAPI CNxtHookAPI::BJCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv )
{
	HRESULT hr = NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
	if( SUCCEEDED(hr) && (*ppv) != NULL &&
		 ( ::IsEqualCLSID( rclsid, CLSID_FileSaveDialog )|| ::IsEqualCLSID( rclsid, CLSID_FileOpenDialog ) ) )
	{
		if( hook_control.is_disabled() )
		{
			return NextCoCreateInstance(rclsid,pUnkOuter,dwClsContext,riid,ppv);
		}
		nextlabs::recursion_control_auto auto_disable(hook_control);
		
		NLHookFileDlg( (IFileDialog*)(*ppv) );
	}
	return hr;
}

HRESULT __stdcall CNxtHookAPI::Try_BJDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if (!_AtlModule.connected())
	{
		return NextDllGetClassObject(rclsid, riid, ppv);
	}

	__try
	{
		return BJDllGetClassObject(rclsid, riid, ppv);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJDllGetClassObject exception \n" );
		return NextDllGetClassObject(rclsid, riid, ppv);
	}
}

HRESULT __stdcall CNxtHookAPI::BJDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	HRESULT hr = NextDllGetClassObject(rclsid, riid, ppv);
	if( SUCCEEDED(hr) && (*ppv) != NULL &&
		( ::IsEqualCLSID( rclsid, CLSID_FileSaveDialog )|| ::IsEqualCLSID( rclsid, CLSID_FileOpenDialog ) ) )
	{
		nextlabs::recursion_control_auto auto_disable(hook_control);
		
		if (::IsEqualCLSID(riid, IID_IClassFactory))
		{
			IClassFactory* iClassFactory = (IClassFactory*)(*ppv);
			IUnknown* pv = NULL;
			HRESULT thishr = iClassFactory->CreateInstance(NULL, IID_IFileDialog, (LPVOID*)&pv);
			if (SUCCEEDED(thishr))
			{
				NLHookFileDlg((IFileDialog*)(pv));
				pv->Release();
			}
		}
	}
	return hr;
}

bool CNxtHookAPI::NLHookFileDlg( IFileDialog* pObject )
{
	BOOL bRet = FALSE;
	if ( NULL == NextShow )
	{
		void* pAddress = NLGetVFuncAdd( (LPVOID)pObject, 3 );
		bRet = HookCode( pAddress,(PVOID)Try_BJShow,(PVOID*)&NextShow );
	}
	return bRet ? true : false;
}

void CNxtHookAPI::NLUnHookFileDlg( )
{
	if ( NULL != NextShow )
	{
		bool bUnHook = UnhookCode( (PVOID*)&NextShow );
		if( bUnHook )
		{
			NLPRINT_DEBUGLOG( L"unhook code show success\n" );
			NextShow = NULL;
		}
		else
		{
			NLPRINT_DEBUGLOG( L"unhook code show failed\n" );
		}
	}
	else
	{
		NLPRINT_DEBUGLOG( L"NextShow is NULL, no need unhook\n" );
	}
}

HRESULT WINAPI CNxtHookAPI::Try_BJShow( IFileDialog* pThis, HWND hwndOwner )
{
	if (!_AtlModule.connected())
 	{
 		return NextShow( pThis, hwndOwner );
 	}

	__try
	{
		return BJShow( pThis, hwndOwner );
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook BJShow exception \n" );
		return NextShow( pThis, hwndOwner );
	}
}

HRESULT WINAPI CNxtHookAPI::BJShow ( IFileDialog* pThis, HWND hwndOwner )
{ONLY_DEBUG
	if( hook_control.is_disabled() )
	{
        NLPRINT_DEBUGLOG(L"[~~New~~] !!!!!! Error: hook control is disabled \n");
		return NextShow( pThis, hwndOwner );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HRESULT hr = NextShow(pThis,hwndOwner);
	NLCELOG_ENTER
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrCurFilePath = theObMgrIns.NLGetCurActiveFilePath();

	
	NLPRINT_DEBUGLOG( L"copy action start up: file path[%s]\n", wstrCurFilePath.c_str() );
	// means user cancel it.
	if ( HRESULT_FROM_WIN32 ( ERROR_CANCELLED ) == hr )
	{
		NLPRINT_DEBUGLOG( L"User cancel Save as dialog \n" );
		NLCELOG_RETURN_VAL( hr )
	}

	wstring wstrDesFilePath = NLGetDesFilePathFormSaveAsDialog( pThis );

	if ( !wstrDesFilePath.empty() )
	{
		//NLPRINT_DEBUGLOG( L"--------------active pDoc:[0x%x] \n", getActiveDoc() );
		CComPtr<IDispatch> pDoc = theObMgrIns.NLGetFilePDocCache( wstrCurFilePath );

		ProcessResult stuResult;
		if ( isNewOfficeFile( wstrCurFilePath ) || GetSuffixFromFileName( wstrCurFilePath ) == GetSuffixFromFileName( wstrDesFilePath ) )
		{
			stuResult = theObMgrIns.NLProcessActionCommon( pDoc, wstrCurFilePath.c_str(), wstrDesFilePath.c_str(), kOA_COPY );
		} 
		else
		{
			stuResult = theObMgrIns.NLProcessActionCommon( pDoc, wstrCurFilePath.c_str(), NULL, kOA_CONVERT );
		}

		if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
		{
			// deny it
			NLPRINT_DEBUGLOG( L"!!!deny win7 save as action. wstrCurFilePath=%ls, wstrDesFilePath=%ls \n", wstrCurFilePath.c_str(), wstrDesFilePath.c_str() );
			NLCELOG_RETURN_VAL( HRESULT_FROM_WIN32 ( ERROR_CANCELLED ) )
		}
		else
		{
			NLPRINT_DEBUGLOG( L"Save as file success: source:[%s], destination:[%s] \n", wstrCurFilePath.c_str(), wstrDesFilePath.c_str() );
						
			// for bug24140, save as the original file and Bug24488 Open in IE save as just create the destination file, source file don't close anymore.
			if ( (wstrDesFilePath != wstrCurFilePath) && !isOpenInIE() )
			{
				theObMgrIns.NLSetSaveAsFlag( wstrDesFilePath, true, wstrCurFilePath );

				theObMgrIns.NLClearFileCache( wstrCurFilePath );

                theObMgrIns.NLSetCurActiveFilePath( wstrDesFilePath );

				// destination file will open but no open action here we initialize the destination file cache
				theObMgrIns.NLInitializeObMgr( wstrDesFilePath );

                NLPRINT_DEBUGLOG(L"[~~New~~] save as end, org current file [%s], new destination file [%s] \n", wstrCurFilePath.c_str(), wstrDesFilePath.c_str());

				//use for SCE	

				TalkWithSCE::GetInstance().CacheOpenedFile( wstrCurFilePath, EF_SUB, TAG(0) );

				TalkWithSCE::GetInstance().CacheOpenedFile( wstrDesFilePath, EF_ADD, TAG(0) );
			}
		}
	}

    if (pep::isExcelApp())
    {
        wstring wstrFilePath = NLGetFilePathFromFileOpenDialog(pThis);
        // only handle sharePoint insert such as http://rms-sp2013 and \\rms-sp2013\sites\, exclude local file operation.
        if (!wstrFilePath.empty() && !boost::istarts_with(wstrFilePath.c_str(),L"file:///"))
        {
           ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrFilePath.c_str(), NULL, kOA_INSERT );

           if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
           {
               // deny it
               NLPRINT_DEBUGLOG( L"!!!deny insert action. wstrFilePath=%ls \n", wstrCurFilePath.c_str() );
               NLCELOG_RETURN_VAL( HRESULT_FROM_WIN32 ( ERROR_CANCELLED ) )
           }

        }
    }

    // handle the case that insert file by "Insert->Object->Text from File..." in word from sharePoint.
    if (pep::isWordApp()) 
    {
         wstring wstrFilePath = NLGetFilePathFromFileOpenDialog(pThis);
         if (!wstrFilePath.empty() && !boost::istarts_with(wstrFilePath.c_str(),L"file:///"))
         {
             g_cacheFilePath = wstrFilePath;
         }
    }

	NLCELOG_RETURN_VAL( hr )
}

wstring CNxtHookAPI::NLGetDesFilePathFormSaveAsDialog( IFileDialog* pThis )
{//ONLY_DEBUG
	wstring wstrDesFilePath = L"";

	IFileSaveDialog* pSaveAsDlg = NULL;
	HRESULT hr = pThis->QueryInterface(IID_IFileSaveDialog,(void**)&pSaveAsDlg);
	if ( FAILED(hr) || NULL == pSaveAsDlg )
	{
		return wstrDesFilePath;
	}

	IShellItem* pItem = NULL;
	hr = pSaveAsDlg->GetResult( &pItem );

	if ( SUCCEEDED(hr) && NULL != pItem )
	{
		PWSTR pszFilePath = NULL;
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		if ( SUCCEEDED(hr) && NULL != pszFilePath )
		{
			wstrDesFilePath = pszFilePath;
			CoTaskMemFree( pszFilePath );
			pItem->Release();
		}
		else if (hr == E_INVALIDARG)
		{
			hr = pItem->GetDisplayName(SIGDN_URL, &pszFilePath);
			if (SUCCEEDED(hr) && NULL != pszFilePath)
			{
				wstrDesFilePath = pszFilePath;
				CoTaskMemFree(pszFilePath);
				pItem->Release();
			}
		}
	}
	pSaveAsDlg->Release();
	pSaveAsDlg = NULL;
	return wstrDesFilePath;
}

wstring CNxtHookAPI::NLGetFilePathFromFileOpenDialog( IFileDialog* pThis )
{
    wstring wstrFilePath = L"";

    IFileOpenDialog* pOpenDlg = NULL;
    HRESULT hr = pThis->QueryInterface(IID_IFileOpenDialog,(void**)&pOpenDlg);
    if ( FAILED(hr) || NULL == pOpenDlg )
    {
        return wstrFilePath;
    }

    IShellItem* pItem = NULL;
    hr = pOpenDlg->GetResult( &pItem );

    if ( SUCCEEDED(hr) && NULL != pItem )
    {
        PWSTR pszFilePath = NULL;

        hr = pItem->GetDisplayName(SIGDN_URL, &pszFilePath);
        if (SUCCEEDED(hr) && NULL != pszFilePath)
        {
            wstrFilePath = pszFilePath;
            CoTaskMemFree(pszFilePath);
            pItem->Release();
        }
        
    }
    pOpenDlg->Release();
    pOpenDlg = NULL;
    return wstrFilePath;
}

/*
*\  olecreatefromfile only can get the temp file
*  so we need to cache them in copyfileex function and use the real path
*	to do evaluation
*/
void CNxtHookAPI::NLCacheRemoteInsertFile(__in LPCWSTR lpExistingFileName, __in LPCWSTR lpNewFileName)
{
	if(lpExistingFileName == NULL || wcslen(lpExistingFileName) < 4 ||
		lpNewFileName == NULL || wcslen(lpNewFileName) < 4)
	{
		return ;
	}
	
	if(IsLocalDriver(lpExistingFileName))
	{
		return ;
	}

	static wchar_t szSHPath[MAX_PATH]={0};
	if(wcslen(szSHPath) < 1)
	{
		SHGetFolderPath(NULL,CSIDL_INTERNET_CACHE,NULL,0,szSHPath);
	}

	if(!boost::algorithm::istarts_with(lpNewFileName,szSHPath))
	{
		return ;
	}

	EnterCriticalSection(&m_csCopyFile);
	if(m_mapInsertFromFile.find(lpNewFileName) == m_mapInsertFromFile.end())
	{
		m_mapInsertFromFile[lpNewFileName]=lpExistingFileName;
	}
	LeaveCriticalSection(&m_csCopyFile);
}

bool CNxtHookAPI::NLGetRemoteFile(__in LPCWSTR lpExistingFileName,__out wstring& strOrigFile)
{
	bool bFind = false;
	EnterCriticalSection(&m_csCopyFile);
	map<wstring,wstring>::const_iterator iter = m_mapInsertFromFile.find(lpExistingFileName);
	if(iter != m_mapInsertFromFile.end())
	{
		strOrigFile = (*iter).second;
		m_mapInsertFromFile.clear();
		bFind = true;
	}
	LeaveCriticalSection(&m_csCopyFile);
	return bFind;
}

BOOL WINAPI CNxtHookAPI::try_BJCopyFileExW(__in LPCWSTR lpExistingFileName, __in LPCWSTR lpNewFileName,
										   __in LPPROGRESS_ROUTINE lpProgressRoutine, __in LPVOID lpData,
										   __in LPBOOL pbCancel, __in DWORD dwCopyFlags)
{
	if (!_AtlModule.connected())
	{
		return NextCopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
	}

	__try
	{
		return BJCopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook CopyFileExW exception \n" );
		return NextCopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
	}
}

BOOL WINAPI CNxtHookAPI::BJCopyFileExW(__in LPCWSTR lpExistingFileName, __in LPCWSTR lpNewFileName,
									   __in LPPROGRESS_ROUTINE lpProgressRoutine, __in LPVOID lpData,
									   __in LPBOOL pbCancel, __in DWORD dwCopyFlags )
{
	if (!_AtlModule.connected())
 	{
 		return NextCopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
 	}

	if( hook_control.is_disabled() )
	{
		return NextCopyFileExW(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,pbCancel,dwCopyFlags);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	CNxtHookAPI& theIns = CNxtHookAPI::NLGetInstance();
	theIns.NLCacheRemoteInsertFile(lpExistingFileName,lpNewFileName);

	return NextCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
}



HRESULT CNxtHookAPI::try_BJStgOpenStorage(const WCHAR * pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen)
{
 	if (!_AtlModule.connected())
	{
		return NextStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);

	}

	__try
	{
		return BJStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		NLPRINT_DEBUGLOG(L"!!!!!!Exception: hook tgOpenStorage exception \n");
		return NextStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
	}
}

HRESULT CNxtHookAPI::BJStgOpenStorage(const WCHAR * pwcsName, IStorage * pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage ** ppstgOpen)
{
	if (hook_control.is_disabled())
	{
		return NextStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (NULL != pwcsName)
    {
		if(boost::ends_with(pwcsName, L".MPT")&&boost::algorithm::icontains(pwcsName, L"Global"))
		{
			return NextStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
		}
		NLPRINT_DEBUGLOG(L"StgOpenStorage start to OPEN  file:[%s]\n", pwcsName);
		if(pep::isExcelApp() && kRtPCDeny == CNLObMgr::NLGetInstance().NLGetPreOpenEvaActionResult(pwcsName, true, kRtPCAllow))
		{
            ::SetLastError(ERROR_ACCESS_DENIED);
            return STG_E_ACCESSDENIED;
		}
		NLPRINT_DEBUGLOG(L"StgOpenStorage will CONVERT file:[%s]\n", pwcsName);
    }

	return NextStgOpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
}


/*
*\brief: word need to block twice only for paste.
*/
HRESULT WINAPI CNxtHookAPI::try_BJOleCreateFromFileEx(IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,
												   IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
												   IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
												   OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
												   IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj)
{
	if (!_AtlModule.connected())
	{
		return NextOleCreateFromFileEx(rclsid,lpszFileName,riid,dwFlags,renderopt,cFormats,rgAdvf,
			rgFormatEtc,lpAdviseSink,rgdwConnection,pClientSite,pStg,ppvObj);
	}

	__try
	{
		return BJOleCreateFromFileEx(rclsid,lpszFileName,riid,dwFlags,renderopt,cFormats,rgAdvf,
			rgFormatEtc,lpAdviseSink,rgdwConnection,pClientSite,pStg,ppvObj);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook OleCreateFromFileEx exception \n" );
		return NextOleCreateFromFileEx(rclsid,lpszFileName,riid,dwFlags,renderopt,cFormats,rgAdvf,
			rgFormatEtc,lpAdviseSink,rgdwConnection,pClientSite,pStg,ppvObj);
	}
}

static bool g_bNeedToDeny=false;
HRESULT CNxtHookAPI::BJOleCreateFromFileEx(IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,
										   IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,
										   IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,
										   OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,
										   IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj)
{

	if( hook_control.is_disabled() )
	{
		return NextOleCreateFromFileEx(rclsid,lpszFileName,riid,dwFlags,renderopt,cFormats,rgAdvf,
										rgFormatEtc,lpAdviseSink,rgdwConnection,pClientSite,pStg,ppvObj);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	NLCELOG_ENTER
	HRESULT hr = E_INVALIDARG;
	if(g_bNeedToDeny)
	{
		// copy paste ,second enter, deny directly.
		g_bNeedToDeny = false;
		SetLastError(5);
		NLCELOG_RETURN_VAL( hr )
	}

	CNxtHookAPI& theHookApi = CNxtHookAPI::NLGetInstance();
	static bool bIsWord = pep::isWordApp();
	static bool bIsExcel = pep::isExcelApp();

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	vector<wstring> vecInsertObj;
	wstring strDst = lpszFileName;
	theHookApi.NLGetRemoteFile(lpszFileName,strDst);
	vecInsertObj.push_back(strDst);

	ActionResult emOfficeRes = theObMgrIns.NLGetEvaluationResult(vecInsertObj,kOA_CONVERT,L"");
	if(emOfficeRes == kRtPCDeny)
	{
		if(bIsWord && !g_bNeedToDeny)
		{
			g_bNeedToDeny = true;
		}
		SetLastError(5);
		NLCELOG_RETURN_VAL( hr )
	}
	// oye: insert office file of outside into an opened file may cause APP to notify pep that file open and close
	//		but for us, those notification is rubbish, need to ignore it
    if( (bIsWord && pep::isWordFile(strDst) )   || 
        (bIsExcel&& pep::isExcelFile(strDst) )  )	
    {
        theObMgrIns.NLSetNoNeedHandleFlag(strDst, kOA_OPEN, true);
        theObMgrIns.NLSetNoNeedHandleFlag(strDst, kOA_CLOSE, true);
    }
	NLCELOG_RETURN_VAL( NextOleCreateFromFileEx(rclsid,lpszFileName,riid,dwFlags,renderopt,cFormats,rgAdvf,rgFormatEtc,lpAdviseSink,
		rgdwConnection,pClientSite,pStg,ppvObj) );
}

BOOL CNxtHookAPI::GetSourceFileByHROP ( const HDROP hDropSource,vector<wstring>& vecSourceFile)
{
	UINT uiRet = DragQueryFileW ( hDropSource, (UINT)-1, NULL, (UINT)0 );
	if ( 0 != uiRet )
	{
		//Get every file and do auto-wrapping
		for ( UINT uiFile = 0; uiFile < uiRet; uiFile++ )
		{
			UINT FileSize = DragQueryFileW ( hDropSource, uiFile, NULL, 0 );
			WCHAR* pFile = new WCHAR[FileSize + 1] ( );
			pFile[0] = NULL;
			DragQueryFileW ( hDropSource, uiFile, pFile, FileSize  + 1 );
			if(pFile[0] != NULL)
			{
				vecSourceFile.push_back(pFile);
			}
			delete []pFile;
		}
	}

	return FALSE;
}

void CNxtHookAPI::GetNewDropObjecct(const vector<wstring>& vecFiles,const HDROP hDropSource, HDROP& hDropNew)
{
	if(vecFiles.empty())	return;
	wstring strFileName;
	for(vector<wstring>::const_iterator it = vecFiles.begin();it != vecFiles.end();it++)
	{
		strFileName += (*it);
		strFileName.push_back ( 0 );
	}
	strFileName.push_back(0);
	PVOID pv = GlobalLock ( hDropSource );

	if ( NULL != pv )
	{
		hDropNew = (HDROP) GlobalAlloc ( GPTR, sizeof DROPFILES + strFileName.size () * sizeof WCHAR );

		if ( NULL != hDropNew )
		{
			PVOID pvNew = GlobalLock ( hDropNew ); 

			if ( NULL != pvNew )
			{
#pragma warning(suppress: 6386) //Suppress error 6386
				memcpy ( pvNew, pv, sizeof DROPFILES );											
				wmemcpy ( (wchar_t*) ( LPBYTE ( pvNew ) + sizeof DROPFILES ), strFileName.c_str ( ), strFileName.size ( ) );
				GlobalUnlock ( hDropNew  ); 
			}
			else
			{
				GlobalFree ( hDropNew );
				hDropNew = NULL;
			}
		}
		GlobalUnlock ( hDropSource );
	}
}

LRESULT CALLBACK CNxtHookAPI::SubclassProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData )
{

	if ( WM_DROPFILES == uMsg )
	{
		HDROP hDropNew = NULL;
		vector<wstring> vecDragFile;
		GetSourceFileByHROP ( (HDROP) wParam, vecDragFile );
		ActionResult emOfficeRes = kRtPCAllow;
		if(!vecDragFile.empty())
		{
			CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
			emOfficeRes = theObMgrIns.NLGetEvaluationResult(vecDragFile,kOA_CONVERT,L"");
		}
		if (emOfficeRes == kRtPCDeny)
		{		
			if(vecDragFile.empty())
			{
				SetLastError(5);
				return E_FAIL;
			}
			else
			{
				// re encapsulate the file list into hdrop
				GetNewDropObjecct(vecDragFile,(HDROP)wParam,hDropNew);
				DragFinish ( (HDROP) wParam );
				return DefSubclassProc ( hWnd, uMsg, (WPARAM)hDropNew, lParam );
			}
		}
		CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
		wstring strDst = theObMgrIns.NLGetCurActiveFilePath();
		theObMgrIns.NLSetNoNeedHandleFlag(strDst, kOA_CLOSE, true);
	}

	return DefSubclassProc ( hWnd, uMsg, wParam, lParam );
}

void WINAPI CNxtHookAPI::try_BJDragAcceptFiles(HWND hWnd, BOOL fAccept )
{
	if (!_AtlModule.connected())
	{
		return NextDragAcceptFiles(hWnd, fAccept);
	}

	__try
	{
		return BJDragAcceptFiles(hWnd, fAccept);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook DragAcceptFiles exception \n" );
		return NextDragAcceptFiles(hWnd, fAccept);
	}
}

void CNxtHookAPI::BJDragAcceptFiles(HWND hWnd, BOOL fAccept )
{
	NextDragAcceptFiles(hWnd,fAccept);
	if(NULL != hWnd && fAccept)
	{
		SetWindowSubclass(hWnd,SubclassProc,NULL,NULL);
	}
}

HRESULT CNxtHookAPI::BJIDropTargetDrop(IDropTarget *pDropTarget, IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect )
{

	// drop will call createfile so can not disable it

	//if( hook_control.is_disabled() )
	//{
	//	return NextIDropTargetDrop( pthis, pDataObj, grfKeyState, pt, pdwEffect );
	//}
	//nextlabs::recursion_control_auto auto_disable(hook_control);
	
	NLCELOG_ENTER
	NextIDropTargetDrop nextIDropTargetDrop = NULL;
	void* pAddress = NLGetVFuncAdd((LPVOID)pDropTarget, 6);
	map<LPVOID, LPVOID>::iterator iter = m_mapHooks.find(pAddress);
	if (iter == m_mapHooks.end() || !iter->second)
	{
		return S_FALSE;
	}
	nextIDropTargetDrop = (NextIDropTargetDrop)iter->second;

	HRESULT hr = S_OK;
	wstring strDragedFile=L"";
	wstring strSrcFile = L"";
	ActionResult emOfficeRes = kRtPCAllow;
	if(isContentData(pDataObj))
	{
		if (!GetOleContentClipboardDataSource(pDataObj, L"", strSrcFile))
		{
			CContextStorage storage = CContextStorage();
			storage.GetDragDropContentFileInfo(strSrcFile);
		}
		
		if (!strSrcFile.empty())
		{
			CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
			strDragedFile = theObMgrIns.NLGetCurActiveFilePath();
			if(!strDragedFile.empty())
			{
				emOfficeRes = theObMgrIns.NLGetEvaluationResult(strSrcFile.c_str(),kOA_PASTE,strDragedFile.c_str());
			}
		}
	}
	else
	{
	// First check for CF_HDROP
		FORMATETC formatetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		if ( pDataObj && SUCCEEDED(pDataObj->QueryGetData (&formatetc)) )
		{
			// CF_HDROP can be a file or a selection; if a filename is available, it is a File drag
			// Check both ANSI and WIDE filenames
			FORMATETC formatetc2 = { (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILENAMEW), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };	// WIDE filename
			FORMATETC formatetc3 = { (CLIPFORMAT)::RegisterClipboardFormat(CFSTR_FILENAMEA), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };	// ANSI filename
			STGMEDIUM stgm;

			// QueryGetData() returns TRUE (wrong info) even for a selection drag; 
			// Must call GetData() to see if the data is available
			if ( SUCCEEDED(pDataObj->GetData(&formatetc2, &stgm)) || SUCCEEDED(pDataObj->GetData(&formatetc3, &stgm)) )
			{
				// We don't actually need the filename, we just need to know it exists
				LPWSTR pFilename = (LPWSTR) GlobalLock (stgm.hGlobal);
				strDragedFile = pFilename;
				GlobalUnlock (stgm.hGlobal);
				// Cleanup
				ReleaseStgMedium (&stgm);
			}
		}
		if(!strDragedFile.empty())
		{
			vector<wstring> vecDragedFile;
			vecDragedFile.push_back(strDragedFile);
			CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
			emOfficeRes = theObMgrIns.NLGetEvaluationResult(vecDragedFile,kOA_CONVERT,L"");
		}

	}
	if(emOfficeRes == kRtPCAllow)
	{
		hr = nextIDropTargetDrop( pDropTarget, pDataObj, grfKeyState, pt, pdwEffect );
	}
	else 
	{
		SetLastError(5);
		hr = E_FAIL;
	}	
	NLCELOG_RETURN_VAL( hr )
}

HRESULT WINAPI CNxtHookAPI::try_BJRegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget)
{
	if (!_AtlModule.connected())
	{
		return NextRegisterDragDrop(hwnd, pDropTarget);
	}

	__try
	{
		return BJRegisterDragDrop(hwnd, pDropTarget);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook RegisterDragDrop exception \n" );
		return NextRegisterDragDrop(hwnd, pDropTarget);
	}
}

// old
//HRESULT CNxtHookAPI::BJRegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget)
//{
//	HRESULT hr = NextRegisterDragDrop(hwnd,pDropTarget);
//	if(SUCCEEDED(hr) && hwnd != NULL)
//	{
//		if( hook_control.is_disabled() )
//		{
//			return NextRegisterDragDrop(hwnd,pDropTarget);
//		}
//		nextlabs::recursion_control_auto auto_disable(hook_control);
//		EnterCriticalSection(&m_crsDropTarget);
//		map<HWND,IDropTarget*>::const_iterator it = m_mapDropTarget.find(hwnd);
//		if(it == m_mapDropTarget.end())
//		{
//			if(NextIDropTargetDrop == NULL)
//			{
//				void* pAddress = NLGetVFuncAdd((LPVOID)pDropTarget,3);
//				HookCode( pAddress,(PVOID)BJIDropTargetDrop ,(PVOID*)&NextIDropTargetDrop);
//			}
//			m_mapDropTarget[hwnd] = pDropTarget;
//		}
//		LeaveCriticalSection(&m_crsDropTarget);
//	}
//	return hr;
//}



// test for new
HRESULT CNxtHookAPI::BJRegisterDragDrop(HWND hwnd, IDropTarget * pDropTarget)
{
	HRESULT hr = NextRegisterDragDrop(hwnd, pDropTarget);
	if (SUCCEEDED(hr) && hwnd != NULL)
	{

		if (hook_control.is_disabled())
		{
			return NextRegisterDragDrop(hwnd, pDropTarget);
		}
		nextlabs::recursion_control_auto auto_disable(hook_control);
		void* pAddress = NLGetVFuncAdd((LPVOID)pDropTarget, 6);
		if (m_mapHooks.find(pAddress) == m_mapHooks.end())
		{
			LPVOID* pnextIDropTargetDrop = new LPVOID();
			if (HookCode(pAddress, (PVOID)BJIDropTargetDrop, (PVOID*)pnextIDropTargetDrop))
			{
				m_mapHooks[pAddress] = *pnextIDropTargetDrop;
			}
		}
	}
	return hr;
}




HRESULT WINAPI CNxtHookAPI::try_BJRevokeDragDrop(HWND hwnd)
{
	if (!_AtlModule.connected())
	{
		return NextRevokeDragDrop(hwnd);
	}

	__try
	{
		return BJRevokeDragDrop(hwnd);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook RevokeDragDrop exception \n" );
		return NextRevokeDragDrop(hwnd);
	}
}

HRESULT CNxtHookAPI::BJRevokeDragDrop(HWND hwnd)
{
	HRESULT hr = E_FAIL;
	if(hwnd != NULL)
	{
		if( hook_control.is_disabled() )
		{
			return NextRevokeDragDrop(hwnd);
		}
	}
	hr = NextRevokeDragDrop(hwnd);
	return hr;
}

HRESULT CNxtHookAPI::BJQueryContinueDrag(IDropSource* pSource, BOOL fEscapePressed, DWORD grfKeyState) 
{
	NextQueryContinueDrag nextQueryContinueDrag = NULL;
	void* pAddress = NLGetVFuncAdd((LPVOID)pSource, 3);
	map<LPVOID, LPVOID>::iterator iter = m_mapHooks.find(pAddress);
	if (iter == m_mapHooks.end() || !iter->second)
	{
		return S_FALSE;
	}
	nextQueryContinueDrag = (NextQueryContinueDrag)iter->second;
 	PPT::_pptApplicationPtr theApp((IDispatch*)pep::getApp());
	
	if (!pep::isPPtApp() || pep::getApp() == NULL)
	{
		return nextQueryContinueDrag(pSource, fEscapePressed, grfKeyState);
	}

	POINT CurPoint = {0, 0}; 
	RECT rct = {0, 0, 0, 0};

	if (GetCursorPos(&CurPoint))
	{
		DocumentWindowPtr pWin = NULL;
		HRESULT hr = theApp->get_ActiveWindow(&pWin);
		if (SUCCEEDED(hr))
		{
			long hWinView = NULL;
			hr = pWin->get_HWND(&hWinView);
			if (SUCCEEDED(hr))
			{
				HWND hCurH= reinterpret_cast<HWND>(hWinView);
				if (hCurH != NULL)
				{					
					if (GetWindowRect(hCurH, &rct) != NULL)
					{
						//the cursor is in the rect, so not try to eva.
						if (CurPoint.x > rct.left && CurPoint.x < rct.right
							&& CurPoint.y > rct.top && CurPoint.y < rct.bottom)
						{
							return nextQueryContinueDrag(pSource,fEscapePressed,grfKeyState);
						}
						else
						{
							CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
							nextlabs::Obligations obs;
							bool bAllow = true;
							wstring strFilePath = theObMgrIns.NLGetCurActiveFilePath();;

							if (!strFilePath.empty())
							{
								if(!theObMgrIns.GetPasteEvaResult(strFilePath,bAllow))
								{
									if(theObMgrIns.NLGetEvaluationResult(strFilePath.c_str(),kOA_PASTE,L"") == kRtPCDeny)	
									{
										bAllow = false;
									} 
									theObMgrIns.SetPasteEvaResult(strFilePath,bAllow);
								}
							}		
							if (!bAllow) 
							{
								return DRAGDROP_S_CANCEL ;
							}
						}
					}
				}
			}
		}
	}
	return nextQueryContinueDrag(pSource, fEscapePressed, grfKeyState);
}

HRESULT WINAPI CNxtHookAPI::try_BJDoDragDrop(IDataObject * pDataObject,  IDropSource * pDropSource, DWORD dwOKEffect, DWORD * pdwEffect)
{
	if( !_AtlModule.connected() )
	{
		return NextDoDragDrop(pDataObject, pDropSource, dwOKEffect, pdwEffect);
	}

	__try
	{
		return BJDoDragDrop(pDataObject, pDropSource, dwOKEffect, pdwEffect);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook DoDragDrop exception \n" );
		return NextDoDragDrop(pDataObject, pDropSource, dwOKEffect, pdwEffect);
	}
}

HRESULT CNxtHookAPI::BJDoDragDrop(IDataObject * pDataObject,  IDropSource * pDropSource, DWORD dwOKEffect, DWORD * pdwEffect)
{
	if( hook_control.is_disabled() )
	{
		return NextDoDragDrop(pDataObject,  pDropSource, dwOKEffect, pdwEffect);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	
	
	NLCELOG_ENTER
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring strFilePath = theObMgrIns.NLGetCurActiveFilePath();
	bool bAllow = true;
	
	if (pep::isPPtApp())
	{
		void* pAddress = NLGetVFuncAdd((LPVOID)pDropSource,3);
		if (m_mapHooks.find(pAddress) == m_mapHooks.end())
		{
			LPVOID* pnextQueryContinueDrag = new LPVOID();
			if (HookCode( pAddress,(PVOID)BJQueryContinueDrag ,(PVOID*)pnextQueryContinueDrag))
			{
				m_mapHooks[pAddress] = *pnextQueryContinueDrag;
			}
		}
		NLCELOG_RETURN_VAL( NextDoDragDrop(pDataObject, pDropSource, dwOKEffect, pdwEffect) )
	}
	nextlabs::Obligations obs;
	
	if (theObMgrIns.NLGetEvaluationResult(strFilePath.c_str(),kOA_PASTE,L"") == kRtPCDeny)
	{
		bAllow = false;
	} 
	if(!bAllow)
	{
		SetLastError(ERROR_ACCESS_DENIED);
		NLCELOG_RETURN_VAL( ERROR_ACCESS_DENIED )
	}

	NLCELOG_RETURN_VAL( NextDoDragDrop(pDataObject,  pDropSource, dwOKEffect, pdwEffect) )
}


HRESULT CNxtHookAPI::try_BJOleGetClipboard(LPDATAOBJECT * ppDataObj)
{
	if (!_AtlModule.connected())
	{
		return NextOleGetClipboard(ppDataObj);
	}

	__try
	{
		return BJOleGetClipboard(ppDataObj);
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		NLPRINT_DEBUGLOG(L"!!!!!!Exception: hook OleGetClipboard exception \n");
		return NextOleGetClipboard(ppDataObj);
	}
}



void queryFilesFromHdrop(HGLOBAL hGlobal, std::vector<std::wstring>& files)
{
	UINT fc = 0;
	// count how many
	fc = ::DragQueryFileW((HDROP)hGlobal, (UINT)-1, NULL, (UINT)0);
	if (0 == fc)
	{
		return;
	}

	files.clear();
	for (UINT i = 0; i < fc; i++)
	{
		// calc size
		int size = ::DragQueryFileW((HDROP)hGlobal, i, NULL, 0);

		if (0 == size)
		{
			continue;
		}
		// prepare buf
		wchar_t* buf = new wchar_t[size + 1];
		::ZeroMemory(buf, sizeof(wchar_t)*(size + 1));


		// read into buf
		int read = 0;
		read = ::DragQueryFileW((HDROP)hGlobal, i, buf, size + 1);
		if (read != size) {
			//shit
		}
		//wprintf(L"%s\n", buf);

		files.push_back(std::wstring(buf));
		delete[] buf;
	}

}




HRESULT CNxtHookAPI::BJOleGetClipboard(LPDATAOBJECT * ppDataObj)
{
	if (hook_control.is_disabled())
	{
		return NextOleGetClipboard(ppDataObj);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	// try to get CF_HDROP to handle drag file outside and drop in office exe

	// call real first
	HRESULT hr = NextOleGetClipboard(ppDataObj);
	if (ppDataObj == NULL || *ppDataObj == NULL) 
	{
		return hr;
	}
	// parse ppDataObj
	// Enumerate all supported Clipboard format
	IEnumFORMATETC * pEnumFormatEtc = NULL;
	hr = (*ppDataObj)->EnumFormatEtc(DATADIR_GET, &pEnumFormatEtc);
	if (FAILED(hr))
	{
		return hr;
	}
	// find CF_HDROP
	bool		bfind_cfhdrop = false;
	FORMATETC        etc;
	std::vector<std::wstring> files;
	while (S_FALSE != pEnumFormatEtc->Next(1, &etc, NULL))
	{
		if (etc.cfFormat != CF_HDROP)
		{
			continue;
		}
		bfind_cfhdrop = true;
		// for parse CF_HDROP
		STGMEDIUM stgmed;
		hr = (*ppDataObj)->GetData(&etc, &stgmed);
		if (hr != S_OK) {
			bfind_cfhdrop = false;
			break;
		}
		
		if (stgmed.tymed != TYMED_HGLOBAL) {
			bfind_cfhdrop = false;
			::ReleaseStgMedium(&stgmed);
			break;
		}
		bfind_cfhdrop = true;
		queryFilesFromHdrop(stgmed.hGlobal, files);		
		::ReleaseStgMedium(&stgmed);
	}
	// Release pEnumerateEtc
	pEnumFormatEtc->Release();
	pEnumFormatEtc = NULL;

	if (!bfind_cfhdrop) 
	{
		return hr;
	}

	// for found
	if (files.empty()) {
		return hr;
	}

	{// only debug
		NLPRINT_DEBUGLOG(L"Find hdrop files in BJOleGetClipboard,%d\n", files.size());
		for (size_t i = 0; i < files.size(); i++) {
			NLPRINT_DEBUGLOG(L"%d:%s\n", i, files[i].c_str());
		}
	}

	// query PC for insert action

	bool bpc_allow = true;
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

	for (size_t i = 0; i < files.size(); i++)
	{
		if (kRtPCDeny == theObMgrIns.NLGetEvaluationResult(files[i].c_str(), kOA_CONVERT, L"")) {
			bpc_allow = false;
			break;
		}
	}

	if (!bpc_allow)
	{
		(*ppDataObj)->Release();
		ppDataObj = NULL;
		// clear clipboard
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::CloseClipboard();
		::SetLastError(ERROR_ACCESS_DENIED);
		return E_FAIL;
	}

	return hr;
}


HRESULT WINAPI CNxtHookAPI::try_BJOleSetClipboard(IDataObject * pDataObj)
{
	if (!_AtlModule.connected())
	{
		return NextOleSetClipboard(pDataObj);
	}

	__try
	{
		return BJOleSetClipboard(pDataObj);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook OleSetClipboard exception \n" );
		return NextOleSetClipboard(pDataObj);
	}
}

HRESULT CNxtHookAPI::BJOleSetClipboard(IDataObject * pDataObj)
{
	if( hook_control.is_disabled() )
	{
		return NextOleSetClipboard(pDataObj);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrCurFilePath = theObMgrIns.NLGetCurActiveFilePath();
	nextlabs::Obligations obs;
	
	if (theObMgrIns.NLGetEvaluationResult(wstrCurFilePath.c_str(),kOA_PASTE,L"") == kRtPCDeny)
	{
		NLPRINT_DEBUGLOG( L"deny paste action, file path:[%s] \n", wstrCurFilePath.c_str() );
		pDataObj = NULL;
	} 
	return NextOleSetClipboard(pDataObj);
}

HANDLE CNxtHookAPI::try_BJSetClipboardData(UINT uFormat, HANDLE hMem)
{
	if (!_AtlModule.connected())
	{
		return NextSetClipboardData(uFormat, hMem);
	}

	__try
	{
		return BJSetClipboardData(uFormat, hMem);
	}
	__except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		NLPRINT_DEBUGLOG(L"!!!!!!Exception: hook user32!SetClipboarddata exception \n");
		return NextSetClipboardData(uFormat, hMem);
	}
}

HANDLE CNxtHookAPI::BJSetClipboardData(UINT uFormat, HANDLE hMem)
{
	if (hook_control.is_disabled())
	{
		return NextSetClipboardData(uFormat, hMem);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrCurFilePath = theObMgrIns.NLGetCurActiveFilePath();
	nextlabs::Obligations obs;


	if (theObMgrIns.NLGetEvaluationResult(wstrCurFilePath.c_str(), kOA_PASTE, L"") == kRtPCDeny)
	{
		NLPRINT_DEBUGLOG(L"deny paste action, file path:[%s] \n", wstrCurFilePath.c_str());
		::SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	return NextSetClipboardData(uFormat, hMem);

}

// CreateWindow: PDFMaker add-in do convert & Save window handle
HWND WINAPI CNxtHookAPI::Try_BJCreateWindowExW( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,	DWORD dwStyle,
																				 int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!_AtlModule.connected())
	{
		return NextCreateWindowExW( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam ); 
	}

	__try
	{
		return BJCreateWindowExW( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook CreateWindowW exception \n" );
		return NextCreateWindowExW( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	}
}

HWND CNxtHookAPI::BJCreateWindowExW( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,	DWORD dwStyle,
																		 int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if( hook_control.is_disabled() )
	{
		return NextCreateWindowExW( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HWND hWindow = NextCreateWindowExW( dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam );
	
	CNLSecondaryThreadForPDFMaker& thePDFMakerThread = CNLSecondaryThreadForPDFMaker::NLGetInstance();
	if ( thePDFMakerThread.NLGetThreadInitializeFlag() )
	{
		// adobe PDFMake add-in, deny convert office file to PDF file.
		if ( (long)lpClassName > 0xffff && NULL != lpWindowName    &&
				boost::algorithm::istarts_with( lpClassName, L"Afx:" ) &&
				0 == _wcsicmp( L"PDFMakerWindow", lpWindowName ) )
		{
			// adobe PAFMake do convert action
			CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
			
			// Get pDoc and file path
			IDispatch* pDoc = getActiveDoc();
			wstring wstrCurFilePath;
			getDocumentPathEx( wstrCurFilePath, pDoc );

			NLPRINT_DEBUGLOG( L"this is adobe PDFMaker add-in convert action, the file path is:[%s] \n",
				wstrCurFilePath.c_str() );
			ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( pDoc, wstrCurFilePath.c_str(), NULL, kOA_CONVERT );
			if ( kFSSuccess != stuResult.kFuncStat || 
				kPSDeny == stuResult.kPolicyStat )
			{
				NLPRINT_DEBUGLOG( L"this is adobe PDF make convert office file to PDF file and we deny it at here \n" );
				/*
					Set deny PDFMaker convert flag to activate the word thread to deny this convert action.
					If we return NULL here to deny the convert action the Office will crash. PDFMaker doesn't check it
				*/
				thePDFMakerThread.NLStartDenyPDFMakerConvert();
			}
		}
	}

 	wchar_t className[MAX_PATH]={0};
 	GetClassName(hWindow,className,MAX_PATH);
 	CNLOvelayView& theOV = CNLOvelayView::GetInstance();
 	if(wcscmp(className,L"_WwB")==0 || wcscmp(className,L"_WwG")==0||wcscmp(className, L"EXCEL7")==0||
 		wcscmp(className,L"childClass")==0||wcscmp(className,L"mdiClass")==0||wcscmp(className,L"screenClass")==0)
 	{
 		theOV.AddView(hWindow);
 	}
	return hWindow;
}

LPVOID CNxtHookAPI::NLGetVFuncAdd( LPVOID pObject, const unsigned int nOffSet )
{
	if ( NULL == pObject )
	{
		return NULL;
	}
	return (LPVOID)*((INT_PTR*)*(INT_PTR*)pObject+nOffSet); 
}

int WINAPI CNxtHookAPI::try_BJStartDocW( HDC hdc,CONST DOCINFOW* lpdi )
{
	if (!_AtlModule.connected())
	{
		return NextStartDocW(hdc,lpdi);
	}

	__try
	{
		return BJStartDocW(hdc,lpdi);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook StartDocW exception \n" );
		return NextStartDocW(hdc,lpdi);
	}
}
int WINAPI CNxtHookAPI::BJStartDocW( HDC hdc,CONST DOCINFOW* lpdi )
{
	if( hook_control.is_disabled() )
	{
        NLPRINT_DEBUGLOG(L"[~~New~~] !!!!!! Error: hook control is disabled \n");
		return NextStartDocW(hdc,lpdi);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	
	// get file path
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrPath = theObMgrIns.NLGetCurActiveFilePath();
    NLPRINT_DEBUGLOG(L"[~~New~~] current active path:[%s]\n", wstrPath.c_str());

	if ( wstrPath.empty() )
	{
		if ( pep::isPPtApp() )
		{
			wstrPath = theObMgrIns.NLGetPPTPrintActiveFilePath();
            NLPRINT_DEBUGLOG(L"[~~New~~] current PPT active path:[%s]\n", wstrPath.c_str());
		} 
	}
	NLPRINT_DEBUGLOG( L"-------HookStartDoc, active file path:[%s] \n", wstrPath.c_str() );

	// check file path
	if( wstrPath.empty() )
	{
		return NextStartDocW(hdc,lpdi);
	}
	
	// do overlay for Word/Excel/PPT 
	ProcessResult stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrPath.c_str(), NULL, kOA_PRINT );
	if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )	
	{
		::SetLastError( ERROR_ACCESS_DENIED );
		return 0;
	}

	CNLPrintOverlay& thePrintOverlayIns = CNLPrintOverlay::GetInstance();
	if (thePrintOverlayIns.IsDoOverlay())
	{
		thePrintOverlayIns.SetHDC(hdc);
		thePrintOverlayIns.StartGDIPlus();
	}

	return NextStartDocW(hdc,lpdi);
}

int WINAPI CNxtHookAPI::try_BJEndDoc( HDC hdc)
{
	if (!_AtlModule.connected())
	{
		return NextEndDoc(hdc);
	}

	__try
	{
		return BJEndDoc(hdc);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook EndDoc exception \n" );
		return NextEndDoc(hdc);
	}
}
int WINAPI CNxtHookAPI::BJEndDoc( HDC hdc )
{
	if( hook_control.is_disabled() )
	{
		return NextEndDoc(hdc);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	CNLPrintOverlay& thePrintOverlayIns = CNLPrintOverlay::GetInstance();
	if (thePrintOverlayIns.IsDoOverlay())
	{
		thePrintOverlayIns.releaseHDC(hdc);
		thePrintOverlayIns.ReleaseOverlayData();
		thePrintOverlayIns.ShutDownGDIPlus() ;
	}
	return NextEndDoc(hdc);
}
int WINAPI CNxtHookAPI::try_BJEndPage( HDC hdc)
{
	if (!_AtlModule.connected())
	{
		return NextEndPage(hdc);
	}

	__try
	{
		return BJEndPage(hdc);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook EndPage exception \n" );
		return NextEndPage(hdc);
	}
}
int WINAPI CNxtHookAPI::BJEndPage( HDC hdc)
{
	if( hook_control.is_disabled() )
	{
		return NextEndPage(hdc);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	CNLPrintOverlay& thePrintOverlayIns = CNLPrintOverlay::GetInstance();
	if(thePrintOverlayIns.IsDoOverlay() && thePrintOverlayIns.IsSameHDC(hdc) )
	{
		if (pep::isExcelApp())
		{
			// do print overlay
			if (0 == m_nExcelEscape)
			{
				thePrintOverlayIns.DoPrintOverlay();
			}
			m_nExcelEscape = 0;
		}
		else
		{
			thePrintOverlayIns.DoPrintOverlay();
		}
	}
	return NextEndPage(hdc);
}

int WINAPI CNxtHookAPI::try_BJEscape(HDC hdc, int nEscape, int cbInput, LPCSTR lpvIn, LPVOID lpvOut)
{
	if (!_AtlModule.connected())
	{
		return NextEscape(hdc, nEscape, cbInput, lpvIn, lpvOut);
	}

	__try
	{
		return BJEscape(hdc, nEscape, cbInput, lpvIn, lpvOut);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook Escape exception \n" );
		return NextEscape(hdc, nEscape, cbInput, lpvIn, lpvOut);
	}
}



int WINAPI CNxtHookAPI::BJEscape(HDC hdc, int nEscape, int cbInput, LPCSTR lpvIn, LPVOID lpvOut)
{

	if( hook_control.is_disabled() )
	{
		return NextEscape(hdc, nEscape, cbInput, lpvIn, lpvOut);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (pep::isExcelApp() && NEXTBAND == nEscape)
	{
		if(++m_nExcelEscape== 2)
		{

			CNLPrintOverlay& thePrintOverlayIns = CNLPrintOverlay::GetInstance();
			if(thePrintOverlayIns.IsDoOverlay() && thePrintOverlayIns.IsSameHDC(hdc) )
			{
				thePrintOverlayIns.DoPrintOverlay();
			}
		}
	}
	return NextEscape(hdc, nEscape, cbInput, lpvIn, lpvOut);

}

ULONG WINAPI CNxtHookAPI::try_BJMAPISendMail(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved)
{
	if (!_AtlModule.connected())
	{
		return NextMAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}
	__try
	{
		return BJMAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook MAPISendMail exception \n" );
		return NextMAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}
}
ULONG CNxtHookAPI::BJMAPISendMail(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved)
{
	static const int NEXT_MAX_PATH = 1024;
	if( hook_control.is_disabled() == true )
	{
		return NextMAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring strSrcPath = theObMgrIns.NLGetCurActiveFilePath();

	if (kRtPCDeny == theObMgrIns.NLGetEvaluationResult(strSrcPath.c_str(), kOA_SEND, L""))
	{
		return 0;
	}
	
	if(!strSrcPath.empty() && lpMessage != NULL &&
		lpMessage->lpszSubject != NULL && lpMessage->lpFiles != NULL &&
		lpMessage->lpFiles->lpszFileName != NULL)
	{
		KeyWords theNode;

		wchar_t strTemp[NEXT_MAX_PATH]={0};
		MultiByteToWideChar( CP_ACP, 0, (char*)lpMessage->lpszSubject,
			(int)strlen((char*)lpMessage->lpszSubject)+1, strTemp,   
			NEXT_MAX_PATH );

		theNode.strSubject = strTemp;
		theNode.uAttachmentCount = lpMessage->nFileCount;
		lpMapiFileDesc pFileDesc = lpMessage->lpFiles;
		MultiByteToWideChar( CP_ACP, 0, (char*)pFileDesc->lpszFileName,
			(int)strlen((char*)pFileDesc->lpszFileName)+1, strTemp,   
			NEXT_MAX_PATH );
		std::wstring strname = strTemp;

		if(strSrcPath.find(L"\\") == std::wstring::npos)
		{
			// get temp path because it's a new document but not be saved 
			// we need use temp filepath
			MultiByteToWideChar( CP_ACP, 0, (char*)pFileDesc->lpszPathName,
				strlen((char*)pFileDesc->lpszPathName)+1, strTemp,   
				NEXT_MAX_PATH );

			std::wstring wstrTemp1 = strTemp;
			size_t nPose = wstrTemp1.rfind(L"\\") + (size_t)1;
			std::wstring wstrTempName = wstrTemp1.substr(nPose); 

			LPCWSTR pSuffix = wcsrchr(strname.c_str(), L'.');
			LPCWSTR pSuffixTemp = wcsrchr(strTemp, L'.');

			if ((0 == _wcsicmp(pSuffixTemp, L".TMP")&& 0 == _wcsicmp(pSuffix, L".XLS"))
				||( 0 == _wcsicmp(pSuffix, L".PPT") && 0 != _wcsicmp(strname.c_str(), wstrTempName.c_str())))
			{
				wstrTemp1.replace(nPose, wstrTemp1.size()-nPose, strname);	
				::CopyFileW(strTemp, wstrTemp1.c_str(), FALSE);
			}
			strSrcPath = wstrTemp1;
		}
		theNode.vecFiles.push_back(FilePair(strname,strSrcPath));
		CTransferInfo::put_FileInfo(theNode.strSubject.c_str(),theNode.uAttachmentCount,theNode.vecFiles);
	}


	return NextMAPISendMail(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
}

HCURSOR WINAPI CNxtHookAPI::try_BJLoadCursorW( HINSTANCE hInstance, LPCWSTR lpCursorName )
{
	if (!_AtlModule.connected())
	{
		return NextLoadCursorW(hInstance, lpCursorName);
	}
	__try
	{
		return BJLoadCursorW(hInstance, lpCursorName);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook LoadCusorW exception \n" );
		return NextLoadCursorW(hInstance, lpCursorName);
	}
}

HCURSOR WINAPI CNxtHookAPI::BJLoadCursorW( HINSTANCE hInstance, LPCWSTR lpCursorName )
{
	if( hook_control.is_disabled() )
	{
		return NextLoadCursorW(hInstance, lpCursorName);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HCURSOR hCursor = NextLoadCursorW(hInstance, lpCursorName);
	if (hCursor)
	{
		if (m_wMoveCursorId == (WORD)lpCursorName)
		{
			m_hMoveCursor = hCursor;
		}
		else if(m_wCopyCursorId == (WORD)lpCursorName)
		{
			m_hCopyCursor = hCursor;
		}
		else if(m_wMoveGroupCursorId == (WORD)lpCursorName)
		{
			m_hMoveGroupCursor = hCursor;
		}
		else if(m_wCopyGroupCursorId == (WORD)lpCursorName)
		{
			m_hCopyGroupCursor = hCursor;
		}
	}
	return hCursor;	
}

HCURSOR WINAPI CNxtHookAPI::try_BJSetCursor( HCURSOR hCursor )
{
	if (!_AtlModule.connected())
	{
		return NextSetCursor(hCursor);
	}
	__try
	{
		return BJSetCursor(hCursor);
	}
	__except(  EXCEPTION_ACCESS_VIOLATION == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH )
	{
		NLPRINT_DEBUGLOG( L"!!!!!!Exception: hook SetCursor exception \n" );
		return NextSetCursor(hCursor);
	}
}

HCURSOR WINAPI CNxtHookAPI::BJSetCursor( HCURSOR hCursor )
{
	if( hook_control.is_disabled() )
	{
		return NextSetCursor(hCursor);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000f)
	{
		if(hCursor == m_hMoveCursor || hCursor == m_hCopyCursor || hCursor == m_hMoveGroupCursor || hCursor == m_hCopyGroupCursor)
		{
			HookMouseLLMsg();
			if (m_bQueryPCFlag)
			{
				CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
				wstring strFilePath = theObMgrIns.NLGetCurActiveFilePath();

				if (!strFilePath.empty())
				{
					if(theObMgrIns.NLGetEvaluationResult(strFilePath.c_str(),kOA_PASTE,L"") == kRtPCDeny)	
					{
						m_bDenyFlag = true;
					} 
				}
				m_bQueryPCFlag = false;
			}
			
		}
	}
	return NextSetCursor(hCursor);
}

LRESULT CALLBACK CNxtHookAPI::MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
			{
				GetCursorPos(&m_ptOldCursor);
			}
			break;
		default:
			break;
		}
	}

	return CallNextHookEx(m_hMouse, code, wParam, lParam);
}

LRESULT CALLBACK CNxtHookAPI::MouseLLProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		switch (wParam)
		{
		case WM_LBUTTONUP:
			{
				if (!m_bQueryPCFlag)
				{
					if (m_bDenyFlag)
					{
						SetCursorPos(m_ptOldCursor.x, m_ptOldCursor.y);
					}
					m_bQueryPCFlag = true;
					m_bDenyFlag = false;
				}
				UnhookMouseLLMsg();
			}
			break;
		default:
			break;
		}
	}
	
	return CallNextHookEx(m_hMouseLL, code, wParam, lParam);
}

void CNxtHookAPI::HookMouseLLMsg()
{
	if (NULL == m_hMouseLL)
	{
		m_hMouseLL = SetWindowsHookExW(WH_MOUSE_LL, MouseLLProc, g_hInstance, 0);
	}
}

void CNxtHookAPI::UnhookMouseLLMsg()
{
	if (m_hMouseLL)
	{
		UnhookWindowsHookEx(m_hMouseLL);
		m_hMouseLL = NULL;
	}
}

void CNxtHookAPI::HookMouseMsg()
{
	if (NULL == m_hMouse)
	{
		m_hMouse = SetWindowsHookExW(WH_MOUSE, MouseProc, 0, GetCurrentThreadId());
	}
}

void CNxtHookAPI::UnhookMouseMsg()
{
	if (m_hMouse)
	{
		UnhookWindowsHookEx(m_hMouse);
		m_hMouse = NULL;
	}
}



//////////////////////////////////////////////////////////////////////////


