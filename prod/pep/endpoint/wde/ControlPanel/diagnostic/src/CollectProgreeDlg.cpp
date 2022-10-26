#include "StdAfx.h"
#include "CollectProgreeDlg.h"

#include "EDPMgrUtilities.h"
#include "CollectZipLog.h"

#include "PWDMgr.h"
#include "CompleteDlg.h"

#ifdef _X86_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#endif

CCollectProgreeDlg::CCollectProgreeDlg()
{
	m_pOnCancel = 0;
	m_pOnCompleted = 0;

	m_pOnCancelParam = NULL;
	m_pOnCompletedParam = NULL;
}

CCollectProgreeDlg::~CCollectProgreeDlg(void)
{
}

void CCollectProgreeDlg::SetOnCancel(OnCancelType pOnCancel, PVOID param)
{
	m_pOnCancel = pOnCancel;
	m_pOnCancelParam = param;
}

void CCollectProgreeDlg::SetOnCompleted(OnCompletedType pOnCompleted, PVOID param)
{
	m_pOnCompleted = pOnCompleted;
	m_pOnCompletedParam = param;
}



LRESULT CCollectProgreeDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	HWND hProgress = ::GetDlgItem(m_hWnd, IDC_PROGRESS1);
	::SendMessage(hProgress, PBM_SETMARQUEE, (WPARAM)TRUE, 50);
	return TRUE;
}

void CCollectProgreeDlg::OnCompleted(PVOID param, DWORD res)
{
	if (!param)
	{
		return;
	}

	CCollectProgreeDlg* pthis = (CCollectProgreeDlg*)param;

	if (pthis->m_pOnCompleted)
	{
		pthis->m_pOnCompleted(pthis->m_pOnCompletedParam, res);
		return;
	}

	return;
}
LRESULT CCollectProgreeDlg::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if (TRUE == (BOOL)wParam)
	{
		CCollectZipLog& diagnosticor = 	CCollectZipLog::GetInstance();

		//	get location
		CWindow location = GetDlgItem(IDC_LOCATION);
		wchar_t* pszLocation = NULL;
		location.GetWindowText(pszLocation);

		//	get password
		CPWDMgr& pwdMgr = CPWDMgr::GetInstance();
		wstring pwd;
		pwdMgr.getpwd(pwd);

		//	start
		if (!diagnosticor.CollectAndZip(pwd, pszLocation, OnCompleted, this))
		{
			//	the diagnostic is not started correctly
			//	return false

			//	error case, this case MUST never take place.
			g_log.Log(CELOG_DEBUG, L"diagnosticor CollectAndZip failed\n");

			return FALSE;
		}
	}

	return 0;
}

LRESULT CCollectProgreeDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	if (m_pOnCancel)
	{
		m_pOnCancel(m_pOnCancelParam);
	}

	return 0;
}


