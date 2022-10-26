#ifndef __UI_MAIN_FRAME_DIALOG_H__
#define __UI_MAIN_FRAME_DIALOG_H__

#include "resource.h"       // main symbols

#include <atlhost.h>
#include "global.h"
#include "ProgressDlg.h"
#include <list>
#pragma warning(push)
#pragma warning(disable: 6387)
#include <string>
#pragma warning(pop)
// CMainFrame
#include "CommunicatorPanel.h"
#include "tempDialog.h"
/*
#define the macro for the ProcessMessageMap ;
*/
#define USER_MESSAGE_HANDER( func ) \
	if( uMsg > WM_USER ) \
		{\
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
		return TRUE; \
		}
#define SYSCOMMAND_MESSAGE_HANDLER(id, func)\
    if(WM_SYSCOMMAND==uMsg && id==LOWORD(wParam))\
	{\
	/*BOOL 	for warning C6246*/   bHandled = TRUE; \
	lResult = func(bHandled);\
	return bHandled;\
	}
namespace MAINFRAME
{
	typedef VOID( CALLBACK* OnClichNextType)( PVOID,LONG status, HWND _hParent ) ;
	typedef void (CALLBACK* ONCLICKCANCELBT)(PVOID, HWND _hParent) ;
	typedef enum _tagDLGSTATUS
	{
		DS_NEXT,
		DS_OK,
		DS_FINISHED,
		DS_SENDMAIL,
		DS_DEFAULT
	}*PDLGSTATUS,DLGSTATUS ;
	/*
	Define the text for the ui 
	*/
	const wchar_t m_szNextBuffer[] = L"Next" ;
	const wchar_t m_szOKBuffer[] = L"Ok" ;
	const wchar_t m_szSendMail[] = L"Send E-mail" ;
	const wchar_t m_szClassName[] = L"NXTLABS_PA_WINDOW_CLASS" ;
	typedef struct _tagMAINBTCALLBACK
	{
		OnClichNextType  pMainBtFunc;
		PVOID pData ;
	}*PMAINBTCALLBACK,MAINBTCALLBACK ;

	typedef struct _tagCANCELBTCALLBACK
	{
		ONCLICKCANCELBT  pMainBtFunc;
		PVOID pData ;
	}*PCANCELBTCALLBACK,CANCELBTCALLBACK ;

} ;
class CMainFrame : 
	public CAxDialogImpl<CMainFrame>
{
public:
	CMainFrame() ;

	~CMainFrame() ;

	enum { IDD = IDD_MAINFRAME };
	

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(ID_NEXT_FINISHED, BN_CLICKED, OnClickedNext)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		SYSCOMMAND_MESSAGE_HANDLER( SC_CONTEXTHELP,OnHelp ) 
		USER_MESSAGE_HANDER( OnProcessUserMessage ) ;
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
			CHAIN_MSG_MAP(CAxDialogImpl<CMainFrame>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
		LRESULT OnClickedNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) ;
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) ;
		LRESULT ShowProgressPanel( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption ) ;
		LRESULT EndProgressPanel( void ) ;

		LRESULT ReleaseMainWnd(int nExitID=IDOK) ;
		INT_PTR	CreateMainWindow(	const HWND _hChildWnd ,
									const BOOL _bIsModel ,
									const wchar_t* _pszHelpURL,
									const wchar_t* _strBtName,
									const INT _ibtStatus,
									const HWND _hParent,
									const wchar_t* _pszTitle, 
									const wchar_t* _pszDescription  ) ;
		LRESULT Change_PA_Panel(	const HWND _hNewPanelWnd ,
									const wchar_t* _pszHelpURL,
									const wchar_t* _pszTitle ,
									const wchar_t* _pszDescription  ) ;
		LRESULT SetNext_OKCallBack( MAINFRAME::OnClichNextType pFunc ,PVOID) ;
		LRESULT SetCancelCallBack( MAINFRAME::ONCLICKCANCELBT pFunc, PVOID ) ;
		//	LRESULT ShowMainDialog( BOOL _isDoModel ) ;
		LRESULT OnProcessUserMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
		/*
		create a temp window
		*/
		HWND GetWindowHandle(VOID) ;
private:
	HRESULT InitImageRef(void) ;
	LRESULT OnNextProcess(  WPARAM wParam, LPARAM lParam, BOOL& bHandled) ; 
	LRESULT OnOKProcess(  WPARAM wParam, LPARAM lParam, BOOL& bHandled) ; 
	VOID ProcessDlgStatus( INT _status, INT IDCtrl, LPARAM lParam  ) ;
	LRESULT AutoJustWindow() ;
	VOID AutoJustControl( UINT ID, INT iGapWidth, INT iGapHeight ) ;
	VOID ProcessBtStatus( INT _status, INT IDCtrl ) ;
	VOID SetMiddlePosition() ;
	VOID DoZBufLog( HWND hCurrWnd ) ;
	VOID DoZBufLog_byOrder( HWND hCurrWnd ) ;
private:
	INT_PTR m_dResult ;
	CImage m_imgBackGround ;
	CBaseWindow *m_hCommunicatorPanel ;
	
	/*
	progress panel.
	*/
	CProgressDlg * m_dlgProgress ;
	
	MAINFRAME::MAINBTCALLBACK m_mainBtStrcture ;
	MAINFRAME::CANCELBTCALLBACK m_cancelBtStrcture ;
	//std::list< MAINFRAME::ONCLICKCANCELBT > m_listCancel ;
	/*
	define the status of dialog.
	*/
	MAINFRAME::DLGSTATUS m_dlgStatus ;
	INT m_iBtStatus ;
	HWND m_hChildPanel ;
	std::wstring m_strHelpURL ;

	BOOL	m_bDoModal ; 
	wchar_t	m_BtName[MAX_PATH] ;


	/*
	The temperory dialog
	*/
	CTempDlg m_tmpDlg ;
public:
	HWND m_hParent ;
	DWORD m_dwCreatedTime;
	LRESULT OnHelp( BOOL& bHandled);

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	PVOID get_PAObjectPtr(void) ;
};


#endif
