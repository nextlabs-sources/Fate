// PA_Demo.h : Declaration of the CPA_Demo

#pragma once

#include "resource.h"       // main symbols

#include <atlhost.h>
//#include "Policy.h"
#include "FileManger.h" 
// CPA_Demo

class CPA_Demo : 
	public CAxDialogImpl<CPA_Demo>
{
public:
	CPA_Demo()
	{
		::ZeroMemory( m_szSrcFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
		::ZeroMemory( m_szDestFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
	}

	~CPA_Demo()
	{
		::ZeroMemory( m_szSrcFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
		::ZeroMemory( m_szDestFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
	}

	enum { IDD = IDD_PA_DEMO };

	BEGIN_MSG_MAP(CPA_Demo)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedTestCopy)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		COMMAND_HANDLER(IDC_BUTTON_GET_SOURCE, BN_CLICKED, OnBnClickedButtonGetSource)
		COMMAND_HANDLER(IDC_BUTTON_GET_DESTINATION, BN_CLICKED, OnBnClickedButtonGetDestination)
		COMMAND_HANDLER(IDC_BUTTON3, BN_CLICKED, OnBnClickedMove)
		COMMAND_HANDLER(IDC_BUTTON_STUB, BN_CLICKED, OnBnClickedButtonStub)
		CHAIN_MSG_MAP(CAxDialogImpl<CPA_Demo>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

public:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
	LRESULT OnClickedTestCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) ;
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) ;
	VOID SetMiddlePosition()  ;

public:
	HINSTANCE m_hInst  ;

protected:
	BOOL OpenFileGetPath( wchar_t *pszFileName ) ;
	BOOL GetSaveFilePath( wchar_t *pszFilePath ) ;
	BOOL CheckSrc_DestFile() ;
	//BOOL EvaluateFile_byPolicy(VOID) ;
public:
	LRESULT OnBnClickedButtonGetSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonGetDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
private:
	wchar_t m_szSrcFileName[MAX_PATH+1] ;
	wchar_t m_szDestFileName[MAX_PATH+1] ;
	CFileManager m_FileMngr ;
	HWND m_hOutPutWnd  ;
    
public:
	LRESULT OnBnClickedMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonStub(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};


