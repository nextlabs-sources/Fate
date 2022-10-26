#include "StdAfx.h"
#include "SettingDlg.h"

#include "CEsdk.h"
#include "WarningPCPWDDlg.h"
#include "WarningPCUnexpectDlg.h"
#include "celog.h"

extern HINSTANCE g_hInst;

void __cdecl SaveSettingsThread( void* pArguments )
{
	if(!pArguments)
	{
		return;
	}

	CSettingDlg* pDlg = (CSettingDlg*)pArguments;

	if(pDlg->m_bProgressDlg)
	{//Wait for progress dialog
		for(int i = 0; i < 50; i++)
		{
			if(pDlg->IsProgressDlgRunning())
			{
				break;
			}
			Sleep(100);
		}
	}

	CoInitialize(0);
	//	submit display level -- all, blocking only, suppress
	pDlg->UpdateProgressDlgStatus(IDS_SAVEDISPLAYLEVEL);
	if (!pDlg->ProcessDisplayLevelSetting())
	{
		//	unexpected error
		pDlg->m_SettingError = SETTING_DISPLAYLEVEL_FAILED;
		goto MYEXIT;
	}

	//	submit display duration -- require user close, 15 seconds, 30 seconds and 45 seconds
	pDlg->UpdateProgressDlgStatus(IDS_SAVEDURATION);
	if (!pDlg->ProcessDisplayDurationSetting())
	{
		pDlg->m_SettingError = SETTING_DURATION_FAILED;
		goto MYEXIT;
	}

	//	submit verbose log setting
	if (!pDlg->ProcessVerboseLogSetting())
	{
		//	unexpected error
		goto MYEXIT;
	}

	pDlg->m_SettingError = SETTING_SUCCESS;

MYEXIT:
	CoUninitialize();
	pDlg->CloseProgressDlg();
}

CSettingDlg::CSettingDlg(void)
{
	m_hFont = CreateFontW(g_nFontSize,
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

	m_pProgressDlg = NULL;
}

CSettingDlg::~CSettingDlg(void)
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	if(m_pProgressDlg)
	{
		delete m_pProgressDlg;
		m_pProgressDlg = NULL;
	}
}

LRESULT CSettingDlg::OnBnClickedCheckEnableLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	

	//	get if user want to check or un-check verbose logging
	BOOL bChecked = IsDlgButtonChecked(IDC_CHECK_ENABLE_LOG);
	
	//	get if verbose log is enabled or disabled
	BOOL bEnabled = FALSE;
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	if (!edpUtilities.IsVerboseLogOn(bEnabled))
	{
		//	error happened
		return 0;
	}

	//	check if we need to change verbose log status
	//	if user' requirement is different with current status
	//	we need do to change verbose logs status later, and we disable edit control for password here
	BOOL bPasswordEnable = (bChecked == bEnabled) ? FALSE : TRUE;
	
	if (!edpUtilities.IsPCRunning())
	{//Don't need to show the password control if the PC is not running. 
		bPasswordEnable = FALSE;
	}
	CWindow pwdStatic = GetDlgItem(IDC_STATIC_PWD);
	pwdStatic.EnableWindow(bPasswordEnable);

	CWindow pwdWnd = GetDlgItem(IDC_EDIT_PWD);
	pwdWnd.EnableWindow(bPasswordEnable);

	//set focus for password edit control if it was enabled
	if(bPasswordEnable)
	{
		::SetFocus(pwdWnd.m_hWnd);
	}

	return 0;
}

LRESULT CSettingDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	m_curSettings.m_bEnableLogging = FALSE;
	//	get if verbose log is enabled or disabled
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	if (!edpUtilities.IsVerboseLogOn(m_curSettings.m_bEnableLogging))
	{
		//	error happened
		return 0;
	}

	//	have got verbose log' status, show status via check box
	DWORD dwCheck = m_curSettings.m_bEnableLogging ? BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton(IDC_CHECK_ENABLE_LOG, dwCheck);

	//	get level of notification display level
	m_curSettings.m_level = CEDPMUtilities::E_ALL;
	if(!edpUtilities.GetNotifyDisplayLever(m_curSettings.m_level))
	{
		//	error happened
		return 0;
	}
	
	//	have got display notification level, show level via radio box
	switch(m_curSettings.m_level)
	{
	case CEDPMUtilities::E_ALL:
		CheckDlgButton(IDC_RADIO_ALL, BST_CHECKED);
		break;
	case CEDPMUtilities::E_NONE:
		{
		CheckDlgButton(IDC_RADIO_SUPRESS, BST_CHECKED);
			//	below code is enable/disable display duration ........................
			EnableDisplayDuration(FALSE);
		}
		break;
	case CEDPMUtilities::E_BLOCK_ONLY:
		CheckDlgButton(IDC_RADIO_ONLY_BLOCK, BST_CHECKED);
		break;
	default:
		break;
	}
	
	//	get display duration setting
	m_curSettings.m_duration = CEDPMUtilities::E_USER_CLOSE;
	if (!edpUtilities.GetNotifyDuration(m_curSettings.m_duration))
	{
		//	error happened
		return 0;
	}

	//	init combo-control
	CWindow wndComboNotify = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);
	SendMessage(wndComboNotify.m_hWnd, CB_ADDSTRING, 0, (LPARAM)L"15 Seconds");
	SendMessage(wndComboNotify.m_hWnd, CB_ADDSTRING, 0, (LPARAM)L"30 Seconds");
	SendMessage(wndComboNotify.m_hWnd, CB_ADDSTRING, 0, (LPARAM)L"45 Seconds");

	SendMessage(wndComboNotify.m_hWnd, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);


	//	show display duration setting via radio box and combo-control
	if (m_curSettings.m_duration == CEDPMUtilities::E_USER_CLOSE)
	{
		CheckDlgButton(IDC_RADIO_REQUIRE_USER_CLOSE, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_DISPLAY_DURATION, BST_CHECKED);
		switch(m_curSettings.m_duration)
		{
		case CEDPMUtilities::E_15_SECONDS:
			SendMessage(wndComboNotify.m_hWnd, CB_SELECTSTRING, 0, (LPARAM)L"15 Seconds");
			break;
		case CEDPMUtilities::E_30_SECONDS:
			SendMessage(wndComboNotify.m_hWnd, CB_SELECTSTRING, 0, (LPARAM)L"30 Seconds");
			break;
		case CEDPMUtilities::E_45_SECONDS:
			SendMessage(wndComboNotify.m_hWnd, CB_SELECTSTRING, 0, (LPARAM)L"45 Seconds");
			break;
		default:
			break;
		}
	}

	//Set font for Radio and checkbox controls
	SendMessage(GetDlgItem(IDC_CHECK_ENABLE_LOG), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	SendMessage(GetDlgItem(IDC_RADIO_ALL), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	SendMessage(GetDlgItem(IDC_RADIO_ONLY_BLOCK), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	SendMessage(GetDlgItem(IDC_RADIO_SUPRESS), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	SendMessage(GetDlgItem(IDC_RADIO_REQUIRE_USER_CLOSE), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	SendMessage(GetDlgItem(IDC_RADIO_DISPLAY_DURATION), WM_SETFONT, (WPARAM)m_hFont, TRUE);
	return TRUE;
}

LRESULT CSettingDlg::OnSubmit(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	GetCurrentSetting();

	m_SettingError = SETTING_SUCCESS;

	m_bProgressDlg = FALSE;
	if(m_newSettings.m_bEnableLogging != m_curSettings.m_bEnableLogging)
	{
		m_bProgressDlg = TRUE;
	}

	HANDLE hThread = (HANDLE)_beginthread(SaveSettingsThread, 0, this);//Launch a thread to save settings.

	if(m_bProgressDlg)
	{
		//need to pop up progress dialog
		m_pProgressDlg = new CProgressDlg();
		if(m_pProgressDlg)
		{
			m_pProgressDlg->DoModal(m_hWnd);
			delete m_pProgressDlg;
			m_pProgressDlg = NULL;
		}

	}

	WaitForSingleObject(hThread, INFINITE);
	HandleError();

	return m_SettingError == SETTING_SUCCESS? 0: -1;
}


BOOL CSettingDlg::ProcessVerboseLogSetting()
{
	//	check if password edit control is enabled
	if(m_newSettings.m_bEnableLogging == m_curSettings.m_bEnableLogging)
	{
		return TRUE;
	}

	BOOL bSuccess = TRUE;
	//	the password editor control is enabled, this means we need to switch verbose log.
	//	we need to turn off pc if pc is running first. then restart pc after verbose log is switched.
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	if (edpUtilities.IsPCRunning())
	{
		//	yes, pc is running, we are stopping pc, get password from password edit control first.
		UpdateProgressDlgStatus(IDS_STOPPC);
		CEResult_t ceRes = edpUtilities.StopPC(m_newSettings.m_szPCPwd);

		
		switch(ceRes)
		{
		case CE_RESULT_SUCCESS:
			{
				//	stop success
				//	switch verbose log now
				UpdateProgressDlgStatus(IDS_SAVEVERBOSELOGGING);
				
				if(!edpUtilities.SetVerboseLogStatus(m_newSettings.m_bEnableLogging))
				{
					m_SettingError = SETTING_VERBOSELOG_FAILED;
					bSuccess = FALSE;
					//	error, unexpected case
					g_log.Log(CELOG_DEBUG, L"SetVerboseLogStatus failed\n");
				}
				else
				{
					//	verbose log is switched, restart pc now
					UpdateProgressDlgStatus(IDS_STARTPC);
					
					if (! edpUtilities.StartPC())
					{
						m_SettingError = SETTING_STARTPC_FAILED;
						bSuccess = FALSE;
						//	sorry, start pc failed, the most properly reason is no permission, see celog for more detail
						//	tell user via error box
						g_log.Log(CELOG_DEBUG, L"start pc failed in ProcessVerboseLogSetting\n");
					}
				}
				
				break;
			}
			break;
		case CE_RESULT_PERMISSION_DENIED:
			{
				m_SettingError = SETTING_STOPPC_WRONGPWD;
				bSuccess = FALSE;
			}
			break;
		case CE_RESULT_INVALID_PARAMS:
			{
				//	password invalid
				//	tell user invalid password
				m_SettingError = SETTING_STOPPC_WRONGPWD;
				bSuccess = FALSE;
			}
			break;
		default:
			{
				//	general error
				//	tell user meet general error to stop pc
				m_SettingError = SETTING_STOPPC_UNKNOWNERROR;
				bSuccess = FALSE;
			}
			break;
		}
	}
	else
	{
		//	no pc is not running, switch directly
		
		UpdateProgressDlgStatus(IDS_SAVEVERBOSELOGGING);
		if (!edpUtilities.SetVerboseLogStatus(m_newSettings.m_bEnableLogging))
		{
			m_SettingError = SETTING_VERBOSELOG_FAILED;
			bSuccess = FALSE;
		}
		
	}
	
	edpUtilities.ResetUACInstance();

	return bSuccess;
}


BOOL CSettingDlg::ProcessDisplayLevelSetting()
{
	if(m_newSettings.m_level == m_curSettings.m_level)
	{
		return TRUE;
	}

	//	always set (save) latest display level
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	if (!edpUtilities.SetNotifyDisplayLever(m_newSettings.m_level))
	{
		//	unexpected error
		return FALSE;
	}

	return TRUE;
}

BOOL CSettingDlg::ProcessDisplayDurationSetting()
{
	if(m_newSettings.m_duration == m_curSettings.m_duration)
	{
		return TRUE;
	}
	//	always set (save) latest display duration
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	if (!edpUtilities.SetNotifyDuration(m_newSettings.m_duration))
	{
		//	unexpected error
		return FALSE;
	}

	return TRUE;
}

LRESULT CSettingDlg::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDC dc = (HDC)wParam;
	
	SelectObject(dc, m_hFont);
	SetBkColor(dc,   g_clrHighlight); 

	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}
	return   (LRESULT)hbr; 
}

LRESULT CSettingDlg::OnBnClickedRadioAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_ALL, IDC_RADIO_SUPRESS, IDC_RADIO_ALL);

	//	below code is enable/disable display duration ........................
	EnableDisplayDuration(TRUE);

	return 0;
}

LRESULT CSettingDlg::OnBnClickedRadioOnlyBlock(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_ALL, IDC_RADIO_SUPRESS, IDC_RADIO_ONLY_BLOCK);

	//	below code is enable/disable display duration ........................
	EnableDisplayDuration(TRUE);
	return 0;
}

LRESULT CSettingDlg::OnBnClickedRadioSupress(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_ALL, IDC_RADIO_SUPRESS, IDC_RADIO_SUPRESS);

	//	below code is disable display duration ........................
	EnableDisplayDuration(FALSE);
	return 0;
}

LRESULT CSettingDlg::OnBnClickedRadioRequireUserClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_REQUIRE_USER_CLOSE, IDC_RADIO_DEFAULT_DURATION, IDC_RADIO_REQUIRE_USER_CLOSE);

	CWindow dispDuration = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);
	dispDuration.EnableWindow(FALSE);

	return 0;
}

LRESULT CSettingDlg::OnBnClickedRadioDisplayDuration(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_REQUIRE_USER_CLOSE, IDC_RADIO_DEFAULT_DURATION, IDC_RADIO_DISPLAY_DURATION);


	CWindow wndComboNotify = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);
	wndComboNotify.EnableWindow(TRUE);

	LONG_PTR dwCurSel = SendMessage(wndComboNotify.m_hWnd, CB_GETCURSEL, 0, 0);
	if (CB_ERR == dwCurSel)
	{
		//	did not initialized, we need to initialize display duration combox
		SendMessage(wndComboNotify.m_hWnd, CB_SELECTSTRING, 0, (LPARAM)L"30 Seconds");
	}	

	return 0;
}

void CSettingDlg::EnableDisplayDuration(BOOL bEnable)
{	
//	CWindow groupDuration = GetDlgItem(IDC_DURATION);
//	groupDuration.EnableWindow(bEnable);

	CWindow userCloseRadio = GetDlgItem(IDC_RADIO_REQUIRE_USER_CLOSE);
	userCloseRadio.EnableWindow(bEnable);

	CWindow durationRadio = GetDlgItem(IDC_RADIO_DISPLAY_DURATION);
	durationRadio.EnableWindow(bEnable);

	CWindow durationCombox = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);
	durationCombox.EnableWindow(bEnable);

	CWindow revertRadio = GetDlgItem(IDC_RADIO_DEFAULT_DURATION);
	revertRadio.EnableWindow(bEnable);
}


LRESULT CSettingDlg::OnBnClickedRadioDefaultDuration(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	CheckRadioButton(IDC_RADIO_REQUIRE_USER_CLOSE, IDC_RADIO_DEFAULT_DURATION, IDC_RADIO_DEFAULT_DURATION);

	CWindow wndComboNotify = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);

	wndComboNotify.EnableWindow(FALSE);

	return 0;
}

LRESULT CSettingDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	//Draw line
	RECT rcDlg;
	::GetWindowRect(m_hWnd, &rcDlg);
	ScreenToClient(&rcDlg);

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(123, 166, 198));
	HPEN hOld = (HPEN)SelectObject(hdc, hPen);

	HWND hLogging = GetDlgItem(IDC_LOGGINGLABEL);
	if(hLogging)
	{
		RECT rc;
		::GetWindowRect(hLogging, &rc);
		ScreenToClient(&rc);

		MoveToEx(hdc, rc.left + 58,rc.top + 8, NULL);
		LineTo(hdc, rcDlg.right - 18, rc.top + 8);	
	}

	HWND hNotifications = GetDlgItem(IDC_NOTIFICATIONSLABEL);
	if(hNotifications)
	{
		RECT rc;
		::GetWindowRect(hNotifications, &rc);
		ScreenToClient(&rc);

		MoveToEx(hdc, rc.left + 80,rc.top + 8, NULL);
		LineTo(hdc, rcDlg.right - 18, rc.top + 8);	
	}

	HWND hDuration = GetDlgItem(IDC_DURATIONLABEL);
	if(hDuration)
	{
		RECT rc;
		::GetWindowRect(hDuration, &rc);
		ScreenToClient(&rc);

		MoveToEx(hdc, rc.left + 60,rc.top + 8, NULL);
		LineTo(hdc, rcDlg.right - 18, rc.top + 8);	
	}

	SelectObject(hdc, hOld);
	::EndPaint(m_hWnd, &ps); 

	return 0;
}

LRESULT CSettingDlg::OnBnClickedOk(WORD , WORD , HWND , BOOL& )
{
	CWindow parent = GetParent();
	SendMessage(parent.m_hWnd, WM_COMMAND, IDOK, NULL);
	return 0;
}


void CSettingDlg::GetCurrentSetting()
{
	//Get the status of check box for "enable logging"
	m_newSettings.m_bEnableLogging = ::IsDlgButtonChecked(m_hWnd, IDC_CHECK_ENABLE_LOG);

	//Get the status of password edit control, enabled or disabled.
	HWND hEditPwdWnd = ::GetDlgItem(m_hWnd, IDC_EDIT_PWD);
	m_newSettings.m_bPassword = ::IsWindowEnabled(hEditPwdWnd);

	//Get PC password
	memset(m_newSettings.m_szPCPwd, 0, sizeof(m_newSettings.m_szPCPwd));
	::GetDlgItemText(m_hWnd, IDC_EDIT_PWD, m_newSettings.m_szPCPwd, sizeof(m_newSettings.m_szPCPwd)/sizeof(wchar_t) - 1);

	//	get latest display level setting 
	m_newSettings.m_level = CEDPMUtilities::E_ALL;
	if (IsDlgButtonChecked(IDC_RADIO_ALL))
	{
		m_newSettings.m_level = CEDPMUtilities::E_ALL;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_ONLY_BLOCK))
	{
		m_newSettings.m_level = CEDPMUtilities::E_BLOCK_ONLY;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_SUPRESS))
	{
		m_newSettings.m_level = CEDPMUtilities::E_NONE;
	}
	else
	{
		//	error case, unexpected, have not handled yet, log at least later.
	}


	//	get latest display duration setting 
	m_newSettings.m_duration = CEDPMUtilities::E_USER_CLOSE;
	if (IsDlgButtonChecked(IDC_RADIO_REQUIRE_USER_CLOSE))
	{
		m_newSettings.m_duration = CEDPMUtilities::E_USER_CLOSE;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_DISPLAY_DURATION))
	{
		CWindow wndComboNotify = GetDlgItem(IDC_COMBO_DISPLAY_DURATION);
		LONG_PTR dwCurSel = SendMessage(wndComboNotify.m_hWnd, CB_GETCURSEL, 0, 0);
		switch(dwCurSel)
		{
		case 0:
			m_newSettings.m_duration = CEDPMUtilities::E_15_SECONDS;
			break;
		case 1:
			m_newSettings.m_duration = CEDPMUtilities::E_30_SECONDS;
			break;
		case 2:
			m_newSettings.m_duration = CEDPMUtilities::E_45_SECONDS;
			break;
		default:
			break;
		}
	}
	else if (IsDlgButtonChecked(IDC_RADIO_DEFAULT_DURATION))
	{
		//	revert to default, set to 30 seconds
		m_newSettings.m_duration = CEDPMUtilities::E_30_SECONDS;
	}

	
}

void CSettingDlg::UpdateProgressDlgStatus(DWORD dwStringID)
{
	if(g_hInst && IsProgressDlgRunning())
	{
		wchar_t buf[1024] = {0};
		LoadStringW(g_hInst, dwStringID, buf, sizeof(buf) / sizeof(wchar_t) - 1);
		::SendMessage(m_pProgressDlg->m_hWnd, WM_PROGRESSDLG_UPDATESTATUS, 0, (LPARAM)buf);
	}
}

void CSettingDlg::CloseProgressDlg()
{
	if(IsProgressDlgRunning())
	{
		::SendMessage(m_pProgressDlg->m_hWnd, WM_PROGRESSDLG_CLOSE, 0, 0);
	}
}

BOOL CSettingDlg::IsProgressDlgRunning()
{
	if(m_pProgressDlg && m_pProgressDlg->m_hWnd && ::IsWindow(m_pProgressDlg->m_hWnd) && ::IsWindowVisible(m_pProgressDlg->m_hWnd))
	{
		return TRUE;
	}
	return FALSE;
}

void CSettingDlg::HandleError()
{
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	switch(m_SettingError)
	{
	case SETTING_STARTPC_FAILED:
		edpUtilities.ShowNoPermission_StartPC();
		break;
	case SETTING_VERBOSELOG_FAILED:
		edpUtilities.ShowNoPermission_SetVerboseLog();
		break;
	case SETTING_STOPPC_WRONGPWD:
		{
			CWarningPCPWDDlg pwdDlg;
			pwdDlg.DoModal();
		}
		break;
	case SETTING_STOPPC_UNKNOWNERROR:
		{
			CWarningPCUnexpectDlg unexpectDlg;
			unexpectDlg.DoModal();
		}
		break;
	case SETTING_STOPPC_NOPERMISSION:
		{
			if(g_hInst)
			{
				wchar_t buf[1024] = {0};
				LoadStringW(g_hInst, IDS_STOPPC_NOPERMISSION, buf, sizeof(buf) / sizeof(wchar_t) - 1);
				CWarningPCPWDDlg pwdDlg;
				pwdDlg.SetText(buf);
				pwdDlg.DoModal();
			}
			
		}
		break;
	default:
		break;
	}
}

LRESULT CSettingDlg::OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	static HBRUSH hbr = NULL;
	if(hbr == NULL)
	{
		hbr = CreateSolidBrush(g_clrHighlight);
	}
	return   (LRESULT)hbr; 
}
