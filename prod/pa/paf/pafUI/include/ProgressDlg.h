#ifndef __NEXTLABS_PROGRESS_DIALOG_H__
#define __NEXTLABS_PROGRESS_DIALOG_H__
#include "resource.h"       // main symbols

#include <atlhost.h>
#include "global.h"
#include "ctrlBase.h"
namespace PrograssBase
{
#define  MAX_RANGE 100 
#define  MIN_RANGE	0 
#define  STEP_TIP_DATA 5
#define	 IDT_TIMER_EVENT	10
} ;
using namespace PrograssBase ;
class CProgressDlg : 
	public CAxDialogImpl<CProgressDlg>
{
public:
	CProgressDlg() {
		m_iCount = 0 ;	
	} ;

	~CProgressDlg() {
		
		if( this->m_hWnd )
		{
			KillTimer( IDT_TIMER_EVENT ) ;
			this->DestroyWindow() ;
		}
	} ;

	enum { IDD = IDD_PROGRESS_DIALOG };

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAxDialogImpl<CProgressDlg>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
	LRESULT ShowDialog( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption, ULONG _lPercent ) ;
	LRESULT HideDialog( VOID ) ;
	LRESULT EndPrograssDlg(VOID) ;
	static VOID CALLBACK TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
private:
	static HWND m_hProgress ;
	static INT m_iCount ;

	 /*
	thread for progress bar...
	*//*
	HANDLE m_hThread ;
	static DWORD WINAPI ProgressThreadProc( LPVOID lpParam )  ;*/
} ;
#endif