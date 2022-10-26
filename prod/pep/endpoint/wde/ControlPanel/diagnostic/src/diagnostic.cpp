// diagnostic.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "EDPMgrUtilities.h"
#include "ReqlogWarningDlg.h"
#include "StopPCDlg.h"
#include "ZipLocationDlg.h"
#include "DiagParentDlg.h"


CDiagParentDlg* g_pDiagDlg = NULL;



/*

this is exported function wizard user to collect logs.


*/
BOOL DoAction(DWORD dwIndex)
{
	switch(dwIndex)
	{
	case 0:
		//	wizard user to collect logs
		{
			if(g_pDiagDlg && !g_pDiagDlg->m_hWnd)
			{//It means user closed the dialog
				delete g_pDiagDlg;
				g_pDiagDlg = NULL;
			}

			if (g_pDiagDlg == NULL)
			{
				g_pDiagDlg = new CDiagParentDlg;
				if(!g_pDiagDlg)
				{
					return FALSE;
				}
				g_pDiagDlg->Create(NULL);

				//	new dialog has been created, we need to register it for keyboard input.
				CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();


				HWND szWnds[1024] = {0};

				szWnds[0] = g_pDiagDlg->m_hWnd;

				edpUtilities.RegDlgHandleForKB(szWnds, 1);

				g_log.Log(CELOG_DEBUG, L"diagnostics reg one modaless dialog handle for keyboard input\n");
			}

			g_pDiagDlg->CenterWindow();
			g_pDiagDlg->ShowWindow(SW_SHOW);
		}
		break;
	default:
		break;
	}

	return TRUE;
}


/*

this is exported function tell user diagnostics status


*/
void __stdcall QueryStatus(char* pBuf, int nLen)
{
	if ( !g_pDiagDlg || (g_pDiagDlg && !g_pDiagDlg->m_hWnd) )
	{
		//	this means we finish diagnostics
		strncpy_s(pBuf, nLen, "Not Running", _TRUNCATE);
	}
	else
	{
		strncpy_s(pBuf, nLen, "Running", _TRUNCATE);
	}
}
