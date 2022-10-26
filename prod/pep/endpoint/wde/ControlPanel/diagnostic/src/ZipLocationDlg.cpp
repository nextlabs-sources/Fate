#include "StdAfx.h"
#include "ZipLocationDlg.h"
#include "CollectProgreeDlg.h"

#include "EDPMgrUtilities.h"
#include <CommDlg.h>


CZipLocationDlg::CZipLocationDlg()
{
}

CZipLocationDlg::~CZipLocationDlg(void)
{
}

LRESULT CZipLocationDlg::OnBrowser(WORD , WORD , HWND , BOOL& )
{
	OPENFILENAME ofn;      // 
	WCHAR szFile[MAX_PATH]; //     

	// init
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	//
	//
	ofn.lpstrFile[0] = _T('\0');
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Zip\0*.zip\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;


	if( GetSaveFileNameW(&ofn))
	{
		wstring strFilePath = wstring(szFile);
		if(strFilePath.find(L".") == wstring::npos)
		{
			strFilePath = strFilePath + L".zip";
		}
		
		::SetWindowTextW(GetDlgItem(IDC_LOCATION), strFilePath.c_str());
	}
	return 0;
}
LRESULT CZipLocationDlg::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	return 0;
}

LRESULT CZipLocationDlg::OnEnChangeLocation(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CAxDialogImpl<CZipLocationDlg>::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	
	//	enable next button only when user have specified location
	g_log.Log(CELOG_DEBUG, L"location is specified by on browser\n");

	HWND hParent = GetParent( );
	HWND hNext = ::GetDlgItem(hParent, IDC_NEXT);
	::EnableWindow(hNext, TRUE);

	::SendMessage( hParent, DM_SETDEFID, IDC_NEXT, 0); 
	::SetFocus(hNext);
	
	return 0;
}
