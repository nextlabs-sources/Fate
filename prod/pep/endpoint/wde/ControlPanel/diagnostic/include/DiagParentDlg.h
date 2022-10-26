#pragma once

#include "Resource.h"
#include <string>
#include "ProgressDlg.h"

#define WM_HANDLE_STOPPC		WM_USER + 130

using namespace std;


/*

this class is parent dialog for all other dialogs used in diagnostic process.

it is a modal dialog, its child dialogs are modaless dialogs.

we control diagnostic process through this parent dialog.

*/

class CStopPCDlg;
class CZipLocationDlg;
class CCollectProgreeDlg;
class CCompleteDlg;
class CReqlogWarningDlg;
class CNXTLBS_Window;

class CDiagParentDlg: public CAxDialogImpl <CDiagParentDlg>
{
private:

	typedef enum
	{
		E_NONE,
		E_VERBOSE_LOG_DLG,
		E_STOPPC_DLG,
		E_LOCATION_DLG,
		E_PROGRESS_DLG,
		E_COMPLETE_DLG
	}DIAGN_STATUS;

	DIAGN_STATUS m_diagnStatus;

	//	they are modaless, and child dialog
	CStopPCDlg* m_pStopPCDlg;
	CZipLocationDlg* m_pLocationDlg;
	CCollectProgreeDlg* m_pProgressDlg;	
	CCompleteDlg* m_pCompleteDlg;
	CReqlogWarningDlg* m_pVerboseLogDlg;

	BOOL m_PCWasRun;

	/*
	
	callback called when diagnostic is canceled by user from progress dialog.
	
	*/
	static void OnCancelDiagnostic(PVOID param);

	/*

	callback called when diagnostic is completed.

	*/
	static void OnCompletedDiagnostic(PVOID param, DWORD res);


	/*
	
	this is the bubble-like window which can link to the zipped file folder

	*/
	CNXTLBS_Window* m_pLinkToZipfile;
	wstring m_szZipLocation;


	/*
	
	call these function when switch between child-dialogs
	
	*/
	BOOL ShowLocationDlg();

public:

	enum { IDD = IDD_PARENTDLG };


	CDiagParentDlg(void);
	~CDiagParentDlg(void);
	BEGIN_MSG_MAP(CDiagParentDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_NEXT, BN_CLICKED, OnBnClickedNext)
		COMMAND_HANDLER(IDC_CANCEL, BN_CLICKED, OnBnClickedCancel)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_HANDLE_STOPPC, OnHandleStopPC)
#if 1
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
#endif

		COMMAND_HANDLER(IDC_FINISH, BN_CLICKED, OnBnClickedFinish)
		COMMAND_HANDLER(IDC_BUTTON_OK, BN_CLICKED, OnBnClickedButtonOk)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CAxDialogImpl<CDiagParentDlg>)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedFinish(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHandleStopPC(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

protected:
	BOOL m_bIsStoppingPC;
};
