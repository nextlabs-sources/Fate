#pragma once

#include "stdafx.h"

#include <MAPI.h>

#include <Hlink.h>

#include "NLOfficePEP_Comm.h"



#define NLHOOKAPIINSTANCE ( CNxtHookAPI::NLGetInstance() )



class CNxtHookAPI

{

private:

	CNxtHookAPI(void);

	~CNxtHookAPI(void);

public:

	static CNxtHookAPI& NLGetInstance();

	void Init();

	void UnInit();

    static bool Query(std::wstring& DisplayText, DWORD ProcessID);

private:

	void NLHookFunction( _In_ LPCSTR pszModule, _In_ LPCSTR pszFuncName, _In_ PVOID  pCallbackFunc, _In_ PVOID* ppNextHook, _In_ const wstring& wstrDebugString ); 

	void NLUnHookFunction( _Inout_ LPVOID* ppFunction, _In_ const wstring& wstrDebugString );

private:
    //used for screen capture.
    static bool Query(std::wstring& DisplayText, HWND hWnd);
    static bool Parse(std::wstring& DisplayText, const std::string& str);


//////////////////////////////////////////////////////////////////////////

private:
    static BOOL WINAPI Try_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);
    static BOOL WINAPI BJPrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);

	/*
		Hook CreateWindow:
		1. Adobe PDFMake add-in, do deny convert action.
		2. Save the window handle for view overlay
	*/

private:

	static HWND WINAPI Try_BJCreateWindowExW( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,	DWORD dwStyle,

		int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);



	static HWND WINAPI BJCreateWindowExW( DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName,	DWORD dwStyle,

		int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);



	/* Function: hook UI and get the file path

	       excel: insert object & data from access & data from text

	       PPT:   insert object        

	*/

private:

	static BOOL WINAPI Try_BJShowWindow(HWND hWnd,int nCmdShow);

	static BOOL WINAPI BJShowWindow(HWND hWnd,int nCmdShow);

	static INT_PTR WINAPI Try_BJDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,DLGPROC lpDialogFunc, LPARAM dwInitParam);
    static INT_PTR WINAPI BJDialogBoxIndirectParamW(HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent,DLGPROC lpDialogFunc, LPARAM dwInitParam);

	static bool WINAPI BJCheckPreventMessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, _Out_ int* pnDialogRetOut);
    static int WINAPI Try_BJMessageBoxExW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId);
    static int WINAPI BJMessageBoxExW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId);
    static int WINAPI Try_BJMessageBoxW(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
    static int WINAPI BJMessageBoxW(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

	static HANDLE WINAPI Try_BJCreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,

		DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);



	static HANDLE WINAPI BJCreateFileW( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,

		DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);	



	// Function: classify->change custom tags

private:

	// for office file, deny user change the custom tags

	static BOOL WINAPI Try_BJEnableWindow(HWND hWnd, BOOL bEnable);

	static BOOL WINAPI BJEnableWindow(HWND hWnd, BOOL bEnable);



	void NLSetEnableWindowChnageAttributeFlag( _In_ HWND hWnd, _In_  bool  bIsNeedEnable );

	bool NLGetEnableWindowChnageAttributeFlag( _In_ HWND hWnd, _Out_ bool& bIsNeedEnable );

	pair<HWND,bool>  m_pairEnableWindow;

	CRITICAL_SECTION m_csEnableWindow;

	

	// Function: 

	//		encrypt file and synchronize golden tags at Word/Excel/PPT 

	//		edit action and Word/Excel 

	//		copy action.

	// for Word/Excel/PPT Save & Word/Excel Copy: 

	//		office work flow write text into temp file and then replace temp file to source file

	// for XP word2007 save as we hook "MoveFile" to encrypt the file and synchronize the golden tags

private:

	static BOOL WINAPI Try_BJMoveFileExW( LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,DWORD dwFlags);

	static BOOL WINAPI BJMoveFileExW(LPCTSTR lpExistingFileName,LPCTSTR lpNewFileName,DWORD dwFlags);



	static BOOL WINAPI Try_BJReplaceFileW( LPCTSTR lpReplacedFileName, LPCTSTR lpReplacementFileName, LPCTSTR lpBackupFileName,

		DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved );



	static BOOL WINAPI BJReplaceFileW( LPCTSTR lpReplacedFileName, LPCTSTR lpReplacementFileName, LPCTSTR lpBackupFileName,

		DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved );



	// Function: IFileDialog interface, win7: word/excel/PPT save as and word insert.

private:

	// hook this function we will get IFileDialog interface for all save as/insert dialog

	static HRESULT WINAPI Try_BJCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv );



	static HRESULT WINAPI BJCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv );

	
	static HRESULT __stdcall Try_BJDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

	static HRESULT __stdcall BJDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

	// hook code in it

	static bool NLHookFileDlg( IFileDialog* pObject );

	static void NLUnHookFileDlg( );

	

	// this function used to process win7 save as action

	static HRESULT WINAPI Try_BJShow( IFileDialog* pThis, HWND hwndOwner );



	static HRESULT WINAPI BJShow( IFileDialog* pThis, HWND hwndOwner );



	static wstring NLGetDesFilePathFormSaveAsDialog( IFileDialog* pThis );

	
    static wstring NLGetFilePathFromFileOpenDialog( IFileDialog* pThis );

	// Function: drag drop: copy content, insert

private:

	static HRESULT WINAPI try_BJStgOpenStorage(_In_opt_  const WCHAR* pwcsName,
		_In_opt_ IStorage* pstgPriority,
		_In_ DWORD grfMode,
		_In_opt_z_ SNB snbExclude,
		_In_ DWORD reserved,
		 IStorage** ppstgOpen);


	static HRESULT WINAPI BJStgOpenStorage(_In_opt_  const WCHAR* pwcsName,
		_In_opt_ IStorage* pstgPriority,
		_In_ DWORD grfMode,
		_In_opt_z_ SNB snbExclude,
		_In_ DWORD reserved,
		 IStorage** ppstgOpen);

	


	static HRESULT WINAPI BJOleCreateFromFileEx( IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,

																								IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,

																								IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,

																								OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,

																								IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj );



	static HRESULT WINAPI try_BJOleCreateFromFileEx( IN REFCLSID rclsid, IN LPCOLESTR lpszFileName, IN REFIID riid,

																								IN DWORD dwFlags, IN DWORD renderopt, IN ULONG cFormats, IN DWORD* rgAdvf,

																								IN LPFORMATETC rgFormatEtc, IN IAdviseSink FAR* lpAdviseSink,

																								OUT DWORD FAR* rgdwConnection, IN LPOLECLIENTSITE pClientSite,

																								IN LPSTORAGE pStg, OUT LPVOID FAR* ppvObj );

	

	/*

	*\ hook olecreatefromfileex can't get the exactly remote file path,so we need to hookup copyfileexw to 

	*	get the real remote file path and do eva.

	*/

	static BOOL WINAPI try_BJCopyFileExW( IN LPCTSTR lpExistingFileName, IN LPCTSTR lpNewFileName, IN LPPROGRESS_ROUTINE lpProgressRoutine,

																				IN  LPVOID lpData, IN LPBOOL pbCancel, IN DWORD dwCopyFlags );

	static BOOL WINAPI BJCopyFileExW( IN LPCTSTR lpExistingFileName, IN LPCTSTR lpNewFileName, IN LPPROGRESS_ROUTINE lpProgressRoutine,

																		IN  LPVOID lpData, IN LPBOOL pbCancel, IN DWORD dwCopyFlags );



	static HRESULT WINAPI try_BJRegisterDragDrop( HWND hwnd, IDropTarget * pDropTarget );

	static HRESULT WINAPI BJRegisterDragDrop( HWND hwnd, IDropTarget * pDropTarget );



	static HRESULT WINAPI try_BJRevokeDragDrop( HWND hwnd );

	static HRESULT WINAPI BJRevokeDragDrop( HWND hwnd );



	static void WINAPI try_BJDragAcceptFiles ( HWND hWnd, BOOL fAccept );

	static void WINAPI BJDragAcceptFiles ( HWND hWnd, BOOL fAccept );



	static HRESULT WINAPI try_BJDoDragDrop( IDataObject * pDataObject,  IDropSource * pDropSource, DWORD dwOKEffect, DWORD * pdwEffect );

	static HRESULT WINAPI BJDoDragDrop( IDataObject * pDataObject,  IDropSource * pDropSource, DWORD dwOKEffect, DWORD * pdwEffect );


	
	// ole32!olegetclipboard

	static HRESULT WINAPI  try_BJOleGetClipboard( LPDATAOBJECT * ppDataObj);

	static HRESULT WINAPI  BJOleGetClipboard(LPDATAOBJECT * ppDataObj);



	static HRESULT WINAPI try_BJOleSetClipboard( IDataObject * pDataObj );

	static HRESULT WINAPI BJOleSetClipboard( IDataObject * pDataObj );


	//  user32!setclipboarddata is special for hook user ctrl c/ ctrl x  at Excel 
	static HANDLE  WINAPI try_BJSetClipboardData(	_In_ UINT uFormat,	_In_opt_ HANDLE hMem);

	static HANDLE  WINAPI BJSetClipboardData(_In_ UINT uFormat, _In_opt_ HANDLE hMem);

	

	static HRESULT WINAPI BJQueryContinueDrag( IDropSource *pThis, BOOL fEscapePressed,DWORD grfKeyState );
	
	static HRESULT WINAPI BJIDropTargetDrop( IDropTarget *pDropTarget , IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect );



// Function: for print overlay

private:

	static int WINAPI try_BJStartDocW( HDC hdc,CONST DOCINFOW* lpdi );

	static int WINAPI BJStartDocW( HDC hdc,CONST DOCINFOW* lpdi );



	static int WINAPI try_BJEndDoc( HDC hdc );

	static int WINAPI BJEndDoc( HDC hdc );



	static int WINAPI try_BJEndPage( HDC hdc);

	static int WINAPI BJEndPage( HDC hdc);


	static int WINAPI try_BJEscape(HDC hdc, int nEscape, int cbInput, LPCSTR lpvIn, LPVOID lpvOut);
	
	static int WINAPI BJEscape(HDC hdc, int nEscape, int cbInput, LPCSTR lpvIn, LPVOID lpvOut);

	static ULONG WINAPI try_BJMAPISendMail(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved);

	static ULONG WINAPI BJMAPISendMail(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags, ULONG ulReserved);
	
	static HCURSOR WINAPI try_BJLoadCursorW(HINSTANCE hInstance, LPCWSTR lpCursorName);
	
	static HCURSOR WINAPI BJLoadCursorW(HINSTANCE hInstance, LPCWSTR lpCursorName);

	static HCURSOR WINAPI try_BJSetCursor(HCURSOR hCursor);

	static HCURSOR WINAPI BJSetCursor(HCURSOR hCursor);

// private common tools

private:

	static LPVOID NLGetVFuncAdd( LPVOID pObject, const unsigned int nOffSet );

	void NLCacheRemoteInsertFile( IN LPCWSTR lpExistingFileName, IN LPCWSTR lpNewFileName );

	bool NLGetRemoteFile( IN LPCWSTR lpExistingFileName,OUT wstring& strOrigFile );

	static BOOL GetSourceFileByHROP ( const HDROP hDropSource,vector<wstring>& vecSourceFile );

	static void GetNewDropObjecct( const vector<wstring>& vecFiles,const HDROP hDropSource, HDROP& hDropNew );

	static LRESULT CALLBACK SubclassProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData );

	static void HookMouseLLMsg();

	static void UnhookMouseLLMsg();

	static void HookMouseMsg();

	static void UnhookMouseMsg();

	static LRESULT CALLBACK MouseLLProc(int code, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam);

private:

		CRITICAL_SECTION	m_csCopyFile;

		map<wstring,wstring>	m_mapInsertFromFile;

		static int m_nExcelEscape;

		static map<LPVOID, LPVOID> m_mapHooks;

		static HCURSOR m_hMoveCursor;

		static HCURSOR m_hCopyCursor;

		static HCURSOR m_hMoveGroupCursor;

		static HCURSOR m_hCopyGroupCursor;

		static WORD m_wMoveCursorId;

		static WORD m_wCopyCursorId;

		static WORD m_wMoveGroupCursorId;

		static WORD m_wCopyGroupCursorId;

		static bool m_bDenyFlag;

		static bool m_bQueryPCFlag;

		static HHOOK m_hMouseLL;

		static HHOOK m_hMouse;

		static POINT m_ptOldCursor;
};

