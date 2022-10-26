// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "EDPMgrUtilities.h"
#include "DiagParentDlg.h"

HINSTANCE g_hInstance; 

extern CDiagParentDlg* g_pDiagDlg;

CELog g_log;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hModule;


			//	init celog
			if (edp_manager::CCommonUtilities::InitLog(g_log, EDPM_MODULE_DIAGS))
			{
				g_log.Log(CELOG_DEBUG, L"EDPM diagnostic init log succeed\n");
			}
			
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			if(g_pDiagDlg)
			{
				if(g_pDiagDlg->m_hWnd)
				{
					DestroyWindow(g_pDiagDlg->m_hWnd);
				}
				delete g_pDiagDlg;
				g_pDiagDlg = NULL;
			}
		}
		break;
	}
	return TRUE;
}

