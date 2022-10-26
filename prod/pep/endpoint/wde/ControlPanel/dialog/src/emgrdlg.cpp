// emgrdlg.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SummaryDlg.h"
#include "NotifyDlg.h"
#include "settingdlg.h"

CSummaryDlg* pSummaryDlg = NULL;
CNotifyDlg* pNotifyDlg = NULL;
CSettingDlg* pSettingDlg = NULL;

HWND DoTab(DWORD dwIndex, HWND hWnd, LPRECT pRc)
{
	switch(dwIndex)
	{
	case 0:
		{
			if(pSummaryDlg)
			{
				delete pSummaryDlg;
				pSummaryDlg = NULL;
			}
			pSummaryDlg = new CSummaryDlg();
			if(pSummaryDlg)
			{
				DWORD dwStart = GetTickCount();
				pSummaryDlg->Create(hWnd);
				pSummaryDlg->MoveWindow(pRc->left, pRc->top, pRc->right - pRc->left, pRc->bottom - pRc->top, TRUE);
				pSummaryDlg->ShowWindow(SW_HIDE);
				g_log.Log(CELOG_DEBUG, L"Initialize Summary dialog: %d ms\n", GetTickCount() - dwStart);
				return pSummaryDlg->m_hWnd;
			}
		}
		break;
	case 1:
		{
			if(pSettingDlg)
			{
				delete pSettingDlg;
				pSettingDlg = NULL;
			}
			pSettingDlg = new CSettingDlg();
			if(pSettingDlg)
			{
				DWORD dwStart = GetTickCount();
				pSettingDlg->Create(hWnd);
				pSettingDlg->MoveWindow(pRc->left, pRc->top, pRc->right - pRc->left, pRc->bottom - pRc->top, TRUE);
				pSettingDlg->ShowWindow(SW_HIDE);
				g_log.Log(CELOG_DEBUG, L"Initialize Setting dilog: %d ms\n", GetTickCount() - dwStart);
				return pSettingDlg->m_hWnd;
			}
		}
		break;
	case 2:
		{
			if(pNotifyDlg)
			{
				delete pNotifyDlg;
				pNotifyDlg = NULL;
			}
			pNotifyDlg = new CNotifyDlg();
			if(pNotifyDlg)
			{
				DWORD dwStart = GetTickCount();
				pNotifyDlg->Create(hWnd);
				pNotifyDlg->MoveWindow(pRc->left, pRc->top, pRc->right - pRc->left, pRc->bottom - pRc->top, TRUE);
				pNotifyDlg->ShowWindow(SW_HIDE);
				g_log.Log(CELOG_DEBUG, L"Initialize Notify dialog: %d ms\n", GetTickCount() - dwStart);
				return pNotifyDlg->m_hWnd;
			}
		}
		break;
	default:
		break;
	}



	return 0;
}

