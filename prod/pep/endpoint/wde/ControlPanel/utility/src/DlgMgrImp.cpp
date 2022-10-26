#include "StdAfx.h"
#include "DlgMgrImp.h"
#include "NoPermissionDlg.h"

#define NO_PERMISSION_START_PC	L"The current account does not have sufficient privilege to start Enforcers."
#define NO_PERMISSION_SET_LOG   L"The current account does not have sufficient privilege to set Verbose Logging."

CDlgMgrImp::CDlgMgrImp(void)
{
}

CDlgMgrImp::~CDlgMgrImp(void)
{
}



BOOL CDlgMgrImp::ShowModalDlg(MODAL_DLG_ID eDlgId)
{
	switch(eDlgId)
	{
	case E_NO_PERMISSION_SET_LOG:
		{
			CNoPermissionDlg noPermissionDlg;
			noPermissionDlg.MySetString(NO_PERMISSION_SET_LOG);
			noPermissionDlg.DoModal();
		}
		break;
	case E_NO_PERMISSION_START_PC:
		{
			CNoPermissionDlg noPermissionDlg;
			noPermissionDlg.MySetString(NO_PERMISSION_START_PC);
			noPermissionDlg.DoModal();
		}
		break;
	default:
		{
			g_log.Log(CELOG_DEBUG, L"ShowModalDlg unknown eDlgId\n");
		}
		break;
	}
	

	return TRUE;
}

void CDlgMgrImp::RegDlgHandleForKB(HWND* pWnds, int nCount)
{
	if(!pWnds)
	{
		return;
	}

	for(int i = 0; i < nCount; i++)
	{
		//	push them to save
		m_hWndsKB.push_back(pWnds[i]);
		g_log.Log(CELOG_DEBUG, L"RegDlgHandleForKB for one hwnd......\n");
	}
}

BOOL CDlgMgrImp::GetDlgHandleForKB(HWND* phWnds, DWORD& dwCount)
{
	if (dwCount < m_hWndsKB.size())
	{
		//	memory not enough
		return FALSE;
	}


	//Remove all the "closed windows handle"
	for(vector<HWND>::iterator itr = m_hWndsKB.begin(); itr != m_hWndsKB.end(); )
	{
		if( !( (*itr) != NULL && ::IsWindow( (*itr)) ) )//maybe this window was closed already
		{
			itr = m_hWndsKB.erase(itr);
			//itr = m_hWndsKB.begin();
			continue;
		}
		itr++;
	}

	dwCount = (DWORD)m_hWndsKB.size();
	for(DWORD i = 0; i < dwCount; i++)
	{
		//	get them out
		phWnds[i] = m_hWndsKB[i];
	}

	return TRUE;
}



