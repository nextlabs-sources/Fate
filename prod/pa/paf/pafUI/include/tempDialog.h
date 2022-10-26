#ifndef __TEMP_DIALOG_H__
#define __TEMP_DIALOG_H__
#include "resource.h"       // main symbols
#include <atlhost.h>

class CTempDlg : 
	public CAxDialogImpl<CTempDlg>
{
public:
	CTempDlg() {

	} ;

	~CTempDlg() {
	} ;

	enum { IDD = IDD_PROGRESS_DIALOG };

	BEGIN_MSG_MAP(CTempDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAxDialogImpl<CTempDlg>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
	{
		CAxDialogImpl<CTempDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
		bHandled = TRUE;
		return 1;  // Let the system set the focus
	}
private:

} ;
#endif