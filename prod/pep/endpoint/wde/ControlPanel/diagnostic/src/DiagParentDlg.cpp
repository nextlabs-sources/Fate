#include "StdAfx.h"
#include "DiagParentDlg.h"

#include "EDPMgrUtilities.h"


#include "PWDMgr.h"
#include "CEsdk.h"
#include "StopPCDlg.h"
#include "ZipLocationDlg.h"
#include "CollectProgreeDlg.h"
#include "CollectZipLog.h"
#include "CompleteDlg.h"
#include "ReqlogWarningDlg.h"
#include "NXTLBS_Window.h"
#include "WarningLoactionDlg.h"
#include "WarningPCPWDDlg.h"
#include "WarningPCUnexpectDlg.h"


HFONT g_hFont = NULL;
HFONT g_hBold = NULL;
HFONT g_hSmallFont = NULL;
extern HINSTANCE g_hInstance;
wchar_t g_szPassword[256] = {0};

void StartPCThread(void* pArguments)
{
#pragma warning(push)
#pragma warning(disable: 4311)
	BOOL bNeedPC = (BOOL)pArguments;
#pragma warning(pop)

	//	turn off verbose log
	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();

	CoInitialize(0);
	if (!utilities.IsPCRunning())//PC is not running
	{
		BOOL bOn = FALSE;
		utilities.IsVerboseLogOn(bOn);
		if(bOn)//check if the "verbose logging" is enabled
		{
			HANDLE hLog = CreateEventW(NULL, FALSE, TRUE,  EVENT_SETTING_LOG);

			utilities.SetVerboseLogStatus(FALSE);//Disable the "verbose logging" if it is enabled.
			g_log.Log(CELOG_DEBUG, L"diagnosticor SetVerboseLogStatus off in OnDestroy when pc is not running\n");		
			if(hLog)
			{
				CloseHandle(hLog);
			}
		}
	}

	//	check if pc was running before diagnostic
	if (bNeedPC)
	{
		//	if, pc was running, start pc before we exit
		HANDLE hPC = CreateEventW(NULL, FALSE, TRUE,  EVENT_STARTING_PC);

		BOOL bRet = utilities.StartPC();
		if(hPC)
		{
			CloseHandle(hPC);
		}
		if (!bRet)
		{
			utilities.ShowNoPermission_StartPC();
			//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
			//	tell user via error box
		}
	}

	utilities.ResetUACInstance();
	CoUninitialize();
}

void __cdecl StopPCThread( void* pArguments )
{
	if(pArguments)
	{
		//	submit password to stop PC
		CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
		CEResult_t ret = utilities.StopPC(g_szPassword);
		
		::PostMessage((HWND)pArguments, WM_HANDLE_STOPPC, (WPARAM)ret, 0);
	}
}

CDiagParentDlg::CDiagParentDlg(void)
{
	m_diagnStatus = E_NONE;
	m_pStopPCDlg = NULL;
	m_pLocationDlg = NULL;
	m_pProgressDlg = NULL;
	m_pCompleteDlg = NULL;
	m_pVerboseLogDlg = NULL;
	m_PCWasRun = TRUE;
	m_pLinkToZipfile = NULL;
	m_pProgressDlg = NULL;

	if(g_hFont == NULL)
	{
		g_hFont = CreateFontW(-12,
								0,
								0,
								0,
								FW_NORMAL,
								0,
								0,
								0,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								CLEARTYPE_QUALITY,
								DEFAULT_PITCH | FF_DONTCARE,
								g_szFont
								);
	}

	if(g_hSmallFont == NULL)
	{
		g_hSmallFont = CreateFontW(-12,
			0,
			0,
			0,
			FW_NORMAL,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			g_szFont
			);
	}


	if(g_hBold == NULL)
	{
		g_hBold = CreateFontW(-14,
								0,
								0,
								0,
								FW_BOLD,
								0,
								0,
								0,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								CLEARTYPE_QUALITY,
								DEFAULT_PITCH | FF_DONTCARE,
								g_szFont
								);
	}
}

CDiagParentDlg::~CDiagParentDlg(void)
{
	//	free sub dialogs
	if (m_pStopPCDlg)
	{
		delete(m_pStopPCDlg);
	}
	if (m_pLocationDlg)
	{
		delete(m_pLocationDlg);
	}
	if (m_pProgressDlg)
	{
		delete(m_pProgressDlg);
	}
	if (m_pCompleteDlg)
	{
		delete(m_pCompleteDlg);
	}
	if (m_pVerboseLogDlg)
	{
		delete m_pVerboseLogDlg;
	}

	//	check if the link to zipped file has been closed
	if (m_pLinkToZipfile)
	{
		if (m_pLinkToZipfile->m_hWnd)
		{
			//	no, the link to zipped file has not been closed,
			//	we need to close it.
			::SendMessage(m_pLinkToZipfile->m_hWnd, WM_CLOSE, NULL, NULL);
		}
		delete m_pLinkToZipfile;
		m_pLinkToZipfile = NULL;
	}

	if(g_hFont)
	{
		DeleteObject(g_hFont);
		g_hFont = NULL;
	}

	if(g_hBold)
	{
		DeleteObject(g_hBold);
		g_hBold = NULL;
	}

	if(g_hSmallFont)
	{
		DeleteObject(g_hSmallFont);
		g_hSmallFont = NULL;
	}
}

LRESULT CDiagParentDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	m_bIsStoppingPC = FALSE;

	HICON hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, FALSE);
	}

	hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, TRUE);
	}

	//	create sub dialogs first
	if (!m_pStopPCDlg)
	{
		m_pStopPCDlg = new CStopPCDlg;
		if (!m_pStopPCDlg)
		{
			return FALSE;
		}
		m_pStopPCDlg->Create(m_hWnd);
	}
	if (!m_pLocationDlg)
	{
		m_pLocationDlg = new CZipLocationDlg;

		if (!m_pLocationDlg)
		{
			return FALSE;
		}
		m_pLocationDlg->Create(m_hWnd);
	}
	if (!m_pProgressDlg)
	{
		m_pProgressDlg = new CCollectProgreeDlg;
		if (!m_pProgressDlg)
		{
			return 0;
		}
		m_pProgressDlg->Create(m_hWnd);
	}
	if (!m_pCompleteDlg)
	{
		m_pCompleteDlg = new CCompleteDlg;
		if (!m_pCompleteDlg)
		{
			return 0;
		}
		m_pCompleteDlg->Create(m_hWnd);
	}
	if (!m_pVerboseLogDlg)
	{
		m_pVerboseLogDlg = new CReqlogWarningDlg;
		if (!m_pVerboseLogDlg)
		{
			return 0;
		}
		m_pVerboseLogDlg->Create(m_hWnd);
	}

	//	move sub dialogs to fit parent dialog client portion.
	RECT childDlgRect;
	childDlgRect.left = 0;
	childDlgRect.right = 650;
	childDlgRect.top = 25;
	childDlgRect.bottom = 250;
	m_pStopPCDlg->MoveWindow(&childDlgRect);
	m_pLocationDlg->MoveWindow(&childDlgRect);
	m_pCompleteDlg->MoveWindow(&childDlgRect);
	m_pVerboseLogDlg->MoveWindow(&childDlgRect);
	m_pProgressDlg->MoveWindow(&childDlgRect);

	m_pStopPCDlg->ShowWindow(SW_HIDE);
	m_pLocationDlg->ShowWindow(SW_HIDE);
	m_pProgressDlg->ShowWindow(SW_HIDE);


	m_pCompleteDlg->ShowWindow(SW_HIDE);
	m_pVerboseLogDlg->ShowWindow(SW_HIDE);

	//	button
	CWindow hNext = GetDlgItem(IDC_NEXT);
	CWindow hBack = GetDlgItem(IDC_BACK);
	CWindow hCancel = GetDlgItem(IDC_CANCEL);
	CWindow hOK = GetDlgItem(IDC_BUTTON_OK);

	//	check if verbose log is enabled
	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
	BOOL bOn = FALSE;
	utilities.IsVerboseLogOn(bOn);
	if (!bOn)
	{
		//	verbose log is disabled, tell user to enable logging first, and try again.

		m_diagnStatus = E_VERBOSE_LOG_DLG;

		hCancel.EnableWindow(TRUE);
		hCancel.ShowWindow(SW_SHOW);

		SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Enable Logging");
		::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
		m_pVerboseLogDlg->ShowWindow(SW_SHOW);

		if (utilities.IsPCRunning())
		{
			m_PCWasRun = TRUE;
		}
		else
		{
			m_PCWasRun = FALSE;
		}
		return TRUE;
	}

	//	check if PC is running
	if (utilities.IsPCRunning())
	{
		//	pc is running, tell user to stop pc first
		m_PCWasRun = TRUE;

		m_diagnStatus = E_STOPPC_DLG;

		//	button
		hBack.EnableWindow(FALSE);
		hNext.EnableWindow(TRUE);
		hCancel.EnableWindow(TRUE);

		//hBack.ShowWindow(SW_SHOW);
		hNext.ShowWindow(SW_SHOW);
		hCancel.ShowWindow(SW_SHOW);

		SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Provide a password");
		::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
		m_pStopPCDlg->ShowWindow(SW_SHOW);
	}
	else
	{
		//	pc is not running or it is stopped, ask user to specify logs' target location
		m_PCWasRun = FALSE;

		m_diagnStatus = E_LOCATION_DLG;

		ShowLocationDlg();
	}

	//bring this dialog to topmost
 	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return TRUE;
}


/*



*/
LRESULT CDiagParentDlg::OnBnClickedNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	switch(m_diagnStatus)
	{
	case E_STOPPC_DLG:
		{
			//	current dialog is stop pc dialog,
			//	submit password to stop PC. if stop succeed, close current dialog and show specify zip location dialog.
			//	if stop failed, show message box according to error code of stopping PC.

			//	get password from edit control
			CWindow PWDControl = m_pStopPCDlg->GetDlgItem(IDC_PASSWORD);


			//	memory for password is allocated inside GetWindowText.
			wchar_t* pwd = NULL;
			
			if (!PWDControl.GetWindowText(pwd))
			{
				return 0;
			}

			if(pwd == NULL)
			{
				CWarningPCPWDDlg pwdDlg;
				wchar_t buf[1024] = {0};
				LoadStringW(g_hInstance, IDS_EMPTYPASSWORD, buf, sizeof(buf) / sizeof(wchar_t) - 1);
				pwdDlg.SetText(buf);
				pwdDlg.DoModal();
				return 0;
			}

			memset(g_szPassword, 0, sizeof(g_szPassword));
			wcsncpy_s(g_szPassword, sizeof(g_szPassword)/sizeof(wchar_t), pwd, _TRUNCATE);

			

			if(g_hInstance)
			{
				wchar_t buf[1024] = {0};
				LoadStringW(g_hInstance, IDS_STOPPC, buf, sizeof(buf) / sizeof(wchar_t) - 1);
				m_pStopPCDlg->SetStatusText(buf);
				m_pStopPCDlg->ShowWaitingInfo();

			}
			
			m_bIsStoppingPC = TRUE;

			_beginthread(StopPCThread, 0, (void*)m_hWnd);//Popup progress dialog.

			::EnableWindow(GetDlgItem(IDC_NEXT), FALSE);
			::EnableWindow(GetDlgItem(IDC_CANCEL), FALSE);

		}
		break;	//	stop pc dialog status
	case E_LOCATION_DLG:
		{
			//	user click next when he submitted zipping location,
			//	we need to get zip location, hide location dialog box, and hide parent dialog by itself.
			//	show user progress dialog, and starting zip all logs to specified location

			//	get zip location, 
			//	dummy code, because we are not using edit control but browser to get the zip location, 
			//	so we use dummy code here, changed later.
			//wchar_t wszLocation[1024] = L"d:\\test\\Diagnostics.zip";

			wchar_t wszLocation[1024] = {0};
			m_pLocationDlg->GetDlgItemTextW(IDC_LOCATION, wszLocation, 1024);

			//	save location
			m_szZipLocation = wstring(wszLocation);

			if (wstring::npos == m_szZipLocation.find(L".zip"))
			{
				//	there is no .zip, strange
				m_szZipLocation += wstring(L".zip");
				g_log.Log(CELOG_DEBUG, L"no .zip in location name - %s ,we add one\n", m_szZipLocation.c_str());
			}

			//	set location to progress dialog
			CWindow hLocation = m_pProgressDlg->GetDlgItem(IDC_LOCATION);
			hLocation.SetWindowText(m_szZipLocation.c_str());

			//	set on cancel to progress dialog, progress dialog will call this callback to notify that user cancel event.
			m_pProgressDlg->SetOnCancel(OnCancelDiagnostic, this);

			//	set on completed to progress dialog, progress dialog will call this callback to notify diagnostic complete event.
			m_pProgressDlg->SetOnCompleted(OnCompletedDiagnostic, this);

			//	show user progress dialog,
			CWindow hNext = GetDlgItem(IDC_NEXT);
			hNext.ShowWindow(SW_HIDE);
			m_pLocationDlg->ShowWindow(SW_HIDE);//Hide the previous child dialog
			//	
			SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Diagnostics Collection");
			::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
			m_pProgressDlg->ShowWindow(SW_SHOW);

			m_diagnStatus = E_PROGRESS_DLG;		

			::SendMessage( m_hWnd, DM_SETDEFID, IDC_CANCEL, 0); 
			::SetFocus(GetDlgItem(IDC_CANCEL));
		}
		break;	//	location dialog
	default:
		{
			//	unexpected case
			g_log.Log(CELOG_DEBUG, L"diagnosticor next button event in unexpected case\n");
		}
		break;
	}

	return 0;
}

LRESULT CDiagParentDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
	utilities;

	switch(m_diagnStatus)
	{
	case E_STOPPC_DLG:
		{
			//	user cancel input password to stop pc,
			//	we just finish diagnostic process without doing nothing.
			SendMessage(WM_CLOSE);
			return 0;
		}
		break;
	case E_LOCATION_DLG:
		{
			//	user cancel input location to zip logs,
			//	we finish diagnostic process with checking if need to restart pc.
			//if (m_PCWasRun)
			//{
			//	//	pc was running before diagnostic, should restart pc
			//	if (!utilities.StartPC())
			//	{
			//		//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
			//		//	tell user via error box
			//		g_log.Log(CELOG_DEBUG, L"diagnosticor can't start pc in cancel location dialog\n");
			//		return 0;
			//	}
			//}
			SendMessage(WM_CLOSE);
			return 0;
		}
	case E_COMPLETE_DLG:
		{
			//	user cancel getting to location of zip logs,
			//	we finish diagnostic without doing nothing
			//	check if pc was running before diagnostic
			//if (m_PCWasRun)
			//{
			//	//	if, pc was running, start pc before show complete dialog
			//	if (!utilities.StartPC())
			//	{
			//		//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
			//		//	tell user via error box
			//		g_log.Log(CELOG_DEBUG, L"diagnosticor can't start pc in cancel on complete dialog\n");
			//	}
			//	
			//}
			SendMessage(WM_CLOSE);
			return 0;			
		}
		break;
	case E_PROGRESS_DLG:
		{
			//	user cancel diagnostics
			OnCancelDiagnostic(this);
		}
		break;
	case E_VERBOSE_LOG_DLG:
		SendMessage(WM_CLOSE);
		return 0;
	default:
		{
			//	error case, this case MUST never take place.
			g_log.Log(CELOG_DEBUG, L"diagnosticor cancel button event in unexpected case\n");
			return 0;
		}
		break;
	}

	return 0;
}

void CDiagParentDlg::OnCancelDiagnostic(PVOID param)
{
	if (!param)
	{
		return;
	}
	
	CDiagParentDlg* pthis = (CDiagParentDlg*) param;

	CCollectZipLog& diagnosticor = CCollectZipLog::GetInstance();

	//	cancel diagnostic, check if cancel succeed.
	if (diagnosticor.Cancel())
	{
		////	cancel succeed, hide progress dialog and show location dialog
		//pthis->m_pProgressDlg->ShowWindow(SW_HIDE);
		//
		//pthis->m_diagnStatus = E_LOCATION_DLG;
		
		//CWindow hNext = pthis->GetDlgItem(IDC_NEXT);
		//hNext.ShowWindow(SW_SHOW);

		//pthis->SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Specify a Location");
		//::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
		//::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
		//pthis->ShowWindow(SW_SHOW);


		//	I was trying to show user location UI to give user another chance to specify location and try again
		//	but, as we have not a very good cancel method -- we can't use TerminateThread -- we don't want user to try again
		//	to decrease the software error chance, so when user cancel, we exit diagnostic wizard directly.
		//if (pthis->m_PCWasRun)
		//{
		//	//	pc was running before diagnostic, should restart pc
		//	CEDPMUtilities& edpUtilites = CEDPMUtilities::GetInstance();
		//	if (!edpUtilites.StartPC())
		//	{
		//		//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
		//		//	tell user via error box
		//		g_log.Log(CELOG_DEBUG, L"diagnosticor can't start pc in OnCancelDiagnostic\n");
		//		return;
		//	}
		//}
		pthis->SendMessage(WM_CLOSE);
		return;
	}
	else
	{
		//	cancel failed, maybe the diagnostic already completed
		//	doing nothing here.
	}

	return;
}

void CDiagParentDlg::OnCompletedDiagnostic(PVOID param, DWORD res)
{
	if (!param)
	{
		return;
	}

	

	CDiagParentDlg* pthis = (CDiagParentDlg*) param;

	//	diagnostic is completed, check if it is succeeded or error.
	if (CCollectZipLog::E_ERROR == (CCollectZipLog::DIAGNSOTIC_ERROR_CODE)res)
	{
		//	error happened
		//	hide progress dialog, show tell user message box that error happened.
		pthis->m_pProgressDlg->ShowWindow(SW_HIDE);

		//	show message box
		CWarningLoactionDlg warnDlg;
		warnDlg.DoModal();

		//	after message box, show location dialog again
		pthis->m_diagnStatus = E_LOCATION_DLG;

		CWindow hNext = pthis->GetDlgItem(IDC_NEXT);
		hNext.ShowWindow(SW_SHOW);

		//	
		pthis->SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Specify a Location");
		::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
		::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
		pthis->ShowWindow(SW_SHOW);
	
		return;
	}
	else
	{
		//	diagnostic succeed complete


		g_log.Log(CELOG_DEBUG, L"succeed in CDiagParentDlg::OnCompletedDiagnostic\n");

		//	hide progress dialog, show complete dialog
		pthis->m_pProgressDlg->ShowWindow(SW_HIDE);
		pthis->m_pLocationDlg->ShowWindow(SW_HIDE);


		//	show and hide button
		CWindow hNext = pthis->GetDlgItem(IDC_NEXT);
		CWindow hBack = pthis->GetDlgItem(IDC_BACK);
		CWindow hCancel = pthis->GetDlgItem(IDC_CANCEL);
		CWindow hFinish = pthis->GetDlgItem(IDC_FINISH);

		hBack.EnableWindow(FALSE);
		hNext.EnableWindow(TRUE);
		hCancel.EnableWindow(TRUE);
		hFinish.EnableWindow(TRUE);

		hNext.ShowWindow(SW_HIDE);
		//hBack.ShowWindow(SW_SHOW);
		hCancel.ShowWindow(SW_HIDE);
		hFinish.ShowWindow(SW_SHOW);		

		::SendMessage( pthis->m_hWnd, DM_SETDEFID, IDC_FINISH, 0); 
		hFinish.SetFocus();
		

		//	show complete dialog
		//	
		pthis->SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Specify Location");
		::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
		::ShowWindow(pthis->GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);
		pthis->m_pCompleteDlg->ShowWindow(SW_SHOW);

		//	show complete dialog
		pthis->m_diagnStatus = E_COMPLETE_DLG;
		pthis->ShowWindow(SW_SHOW);
	}

	return;
	}


LRESULT CDiagParentDlg::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if(m_pStopPCDlg && m_pStopPCDlg->m_hWnd)
	{
		m_pStopPCDlg->SetEditFocus();
	}
	return 0;
}

LRESULT CDiagParentDlg::OnBnClickedFinish(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	switch(m_diagnStatus)
	{
	case E_COMPLETE_DLG:
		{
			//	check and start pc in on destroy


			////	check if pc was running before diagnostic
			//if (m_PCWasRun)
			//{
			//	//	if, pc was running, start pc before we exit
			//	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
			//	if (!utilities.StartPC())
			//	{
			//		//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
			//		//	tell user via error box
			//		g_log.Log(CELOG_DEBUG, L"diagnosticor can't start pc in OnBnClickedFinish on complete dialog\n");
			//	}
			//}

			DestroyWindow();

			//	show user target location folder directly
			wstring command = wstring(L"/select,") + m_szZipLocation;
			ShellExecute(NULL, NULL, L"explorer.exe", command.c_str(), NULL, SW_SHOW);
		}
		break;
	default:
		{
			//	error case, this case MUST never take place.
			g_log.Log(CELOG_DEBUG, L"diagnosticor finish button event in unexpected case\n");
		}
		break;
	}
	return 0;
}

LRESULT CDiagParentDlg::OnBnClickedButtonOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	switch(m_diagnStatus)
	{
	case E_VERBOSE_LOG_DLG:
		{

			DestroyWindow();
		}
		break;
	default:
		{
			//	error case, this case MUST never take place.
			g_log.Log(CELOG_DEBUG, L"diagnosticor ok button event in unexpected case\n");
		}
		break;
	}
	return 0;
}

LRESULT CDiagParentDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(m_bIsStoppingPC)
	{
		return 0;
	}
	switch(m_diagnStatus)
	{
	case E_PROGRESS_DLG:
		{
			//	user cancel diagnostics
			g_log.Log(CELOG_DEBUG, L"user cancel diagnostics in OnClose\n");
			OnCancelDiagnostic(this);
		}
		break;
	default:
		break;
	}



	//if (m_PCWasRun)
	//{
	//	//	if, pc was running, start pc before exit
	//	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
	//	if (!utilities.StartPC())
	//	{
	//		//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
	//		//	tell user via error box
	//		g_log.Log(CELOG_DEBUG, L"diagnosticor can't start pc in OnClose\n");
	//	}
	//}

	DestroyWindow();
	return 0;
}

LRESULT CDiagParentDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{

	HDC dc = (HDC)wParam;
	HWND hWnd = (HWND)lParam;

	hWnd;	
	//		SetTextColor(dc, RGB(255, 0, 255));
	SelectObject(dc, g_hBold);

//		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 
	SetBkMode(dc, TRANSPARENT);


	return   (LRESULT)GetStockObject(HOLLOW_BRUSH);
}

BOOL CDiagParentDlg::ShowLocationDlg()
{
	//	get button
	CWindow hNext = GetDlgItem(IDC_NEXT);
	CWindow hBack = GetDlgItem(IDC_BACK);
	CWindow hCancel = GetDlgItem(IDC_CANCEL);

	//	enable button
	hBack.EnableWindow(FALSE);
	hNext.EnableWindow(FALSE);
	hCancel.EnableWindow(TRUE);

	hBack.ShowWindow(SW_HIDE);
	hNext.ShowWindow(SW_SHOW);
	hCancel.ShowWindow(SW_SHOW);

	//	set text
	SetDlgItemText(IDC_STATIC_CHILD_TITLE, L"Specify a Location");
	::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC_CHILD_TITLE), SW_SHOW);

	//	show dialog
	m_pLocationDlg->ShowWindow(SW_SHOW);

	::SendMessage( m_hWnd, DM_SETDEFID, IDC_CANCEL, 0); 
	hCancel.SetFocus();

	return TRUE;
}

LRESULT CDiagParentDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	
	//	turn off verbose log
#pragma warning(push)
#pragma warning(disable: 4312)
	_beginthread(StartPCThread, 0, (void*)m_PCWasRun);
#pragma warning(pop)

	return 0;
}


LRESULT CDiagParentDlg::OnHandleStopPC(UINT , WPARAM wParam, LPARAM , BOOL& )
{
	//	check result
	
	CEResult_t ret = (CEResult_t)wParam;
	switch(ret)
	{
	case CE_RESULT_SUCCESS:
		{
			//	stop pc succeed
			//	we are going to close stop pc dialog and show specify zip location dialog.
			m_pStopPCDlg->ShowWindow(SW_HIDE);

			//	remember password first
			CPWDMgr& pwdMgr = CPWDMgr::GetInstance();
			pwdMgr.setpwd(g_szPassword);

			//	show location dialog
			ShowLocationDlg();

			m_diagnStatus = E_LOCATION_DLG;

			m_bIsStoppingPC = FALSE;
			return 0;
		}
		break;
	case CE_RESULT_PERMISSION_DENIED:
	case CE_RESULT_INVALID_PARAMS:
		{
			//	invalid password
			//	tell user invalid password
			m_pStopPCDlg->ShowWaitingInfo(FALSE);
			CWarningPCPWDDlg pwdDlg;
			pwdDlg.DoModal();
		}
		break;
	default:
		{
			//	general error
			//	tell user meet general error
			m_pStopPCDlg->ShowWaitingInfo(FALSE);
			CWarningPCUnexpectDlg unexpectDlg;
			unexpectDlg.DoModal();
		}
		break;
	}
	::EnableWindow(GetDlgItem(IDC_NEXT), TRUE);
	::EnableWindow(GetDlgItem(IDC_CANCEL), TRUE);
	

	m_bIsStoppingPC = FALSE;
	return 0;
}
