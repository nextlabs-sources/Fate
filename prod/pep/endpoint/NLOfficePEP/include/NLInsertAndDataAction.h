#pragma once

#include "stdafx.h"
#include "NLOfficePEP_Comm.h"

//////////////////////////////////////////////////////////////////////////
// excel insert object string
static const wchar_t* g_wszInsertObjectDialogCaption        = L"Object";
static const wchar_t* g_wszInsertObjectDialogClassName      = L"bosa_sdm_XL9";
static const wchar_t* g_wszInsertObjectEditControlCaption   = L"";
static const wchar_t* g_wszInsertObjectEditControlClassName = L"EDTBX";
static const int      g_nInsertObjectEditControlID          = 0X1C;
// office 2016 ppt insert object string
static const wchar_t* g_wszPPTInsertObjectDialogCaption        = L"";
static const wchar_t* g_wszPPTInsertObjectDialogClassName      = L"NetUICtrlNotifySink";
static const wchar_t* g_wszPPTInsertObjectEditControlCaption   = L"";
static const wchar_t* g_wszPPTInsertObjectEditControlClassName = L"RICHEDIT60W";
static const int      g_nPPTInsertObjectEditControlID          = 0X01;
static const int      g_nPPTInsertObjectNetUICtrlNotifySinkID  = 0X00;

// excel data form text/access string
static const wchar_t* g_wszDataActionDialogClassNameOne = L"bosa_sdm_XL9"; 
static const wchar_t* g_wszDataActionDialogClassNameTwo = L"#32770";

static const wchar_t* g_wszDataFromTextFirstDialogCaption   = L" Import Text File";
static const wchar_t* g_wszDataFromAccessFirstDialogCaption = L"Select Data Source";
static const wchar_t* g_wszDataFromAccessInsertExcelFirstCaption = L"Select Table";
static const wchar_t* g_wszDataFromTextNextDialogInternalCaption = L"Convert Text to Columns Wizard - Step 1 of 3";
static const wchar_t* g_wszDataFromTextNextDialogExternalCaption = L"Text Import Wizard - Step 1 of 3";
static const wchar_t* g_wszDataFromAccessNextDialogCaption       = L"Data Link Properties";
static const wchar_t* g_wszDataFromWebNextDialogCaption          = L"Import Data";
//////////////////////////////////////////////////////////////////////////

class CNLInsertAndDataAction
{
private:
	CNLInsertAndDataAction();
	~CNLInsertAndDataAction();

public:
	static CNLInsertAndDataAction& GetInstance();
	void initExcelInsertAndDataAction(); 
	void initPPTInsertAndDataAction();

    // PPT or Excel
    static LRESULT WINAPI InsertObjectEDTBXProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
	// for Excel
	static LRESULT WINAPI DataFromDialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT WINAPI InsertObjectProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
	
	// for PPT
    static LRESULT WINAPI PPTInsertObjectProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT WINAPI PPTInsertObjectOkButtonProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

// public for common
public:
	// generic judge if it is the dialog before to insert excel data from access/text dialog
	bool IsWidownPopUpAfterExcelDataEnd( HWND hWnd );

public:
	HWND JudgeParentHandleAndReturnChildHandleW( HWND hParent, const wstring& wstrParentCaption, const wstring& wstrParentClassName,
		const wstring& wstrChildCaption, const wstring& wstrChildClassName,
		const int nChildID );

	bool JudgeSpecifieHandleW( HWND hWnd, const wstring& wstrClassName, const wstring& wstrCaption );

public:
	void SetDataFromDialogClose( _In_ bool bDataDialogClose, _In_ const wstring& strFileName );
	bool IsDataFromDialogClose( _Out_ wstring& strFileName );
	bool IsDataFromDialogClose( );

	void SetExcelDataActionAllow( const wstring& wstrFilePath );
	bool GetExcelDataActionAllow( );

	void SetExcelInsertBrowseFilePath( const wstring& wstrBrowseFilePath );
	wstring GetExcelInsertBrowseFilePath( );

	void SetExcelOrPptInsertObjectWndProc( WNDPROC wpInsertObjectEDTBXDefaultWndProc, WNDPROC wpInsertObjectDialogDefaultWndProc );
	WNDPROC GetInsertObjectEDTBXDefaultWndProc( );
	WNDPROC GetInsertObjectDialogDefaultWndProc( );

	void SetExcelDataDialogWndProc( WNDPROC wpDataDialogDefaultWndProc );
	WNDPROC GetExcelDataDialogDefaultWndProc( );

	void SePPTInsertObjectOKButtonWndProc( WNDPROC wpPPTInsertObjectOKButtonWndProc );
	WNDPROC GetPPTInsertObjectOKButtonWndProc( );

private:
	inline bool NLIsRightCaption( _In_ const wstring& wstrCaption );
	inline bool NLIsRightClassName( _In_ const wstring& wstrClassName );

private:
	CRITICAL_SECTION m_csDataDialogClose;
	bool			       m_bDataDialogClose;
	wstring			     m_strFileName;

	CRITICAL_SECTION m_csExcelDataActionAllow;
	bool             m_bExcelDataActionAllow;
	
	CRITICAL_SECTION m_csBrowseFilePath;
	wstring          m_wstrBrowseFilePath;

	CRITICAL_SECTION m_csExcelOrPptInsertObjectWndProc;
	WNDPROC          m_wpInsertObjectEDTBXDefaultWndProc;    // excel insert
	WNDPROC          m_wpInsertObjectDialogDefaultWndProc;

	CRITICAL_SECTION m_csDataDialogWndProc;                  // excel Data
	WNDPROC	         m_wpDataDialogDefaultWndProc; 
	
	CRITICAL_SECTION m_csPPTInsertObjectOKButtonDefaultWndProc;
	WNDPROC	         m_wpPPTInsertObjectOKButtonWndProc;
	
	// for excel data from access/text dialog caption & class name
	set<wstring> m_setGenericCaption;
	set<wstring> m_setGenericClassName;
};
