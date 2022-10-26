#pragma once

#include "actionMenu.h"
#include "ProductInfoDlg.h"

const UINT DESTINY_SYSTEM_TRAY_MSG = ::RegisterWindowMessage (_T("DESTINY_SYSTEM_TRAY_MSG"));

extern CActionMenu g_ActionMenu_L;
extern CActionMenu g_ActionMenu_R;
extern CActionMenu* g_pActionMenu_CurActived;

// typedef struct struEnforcerStatus
// {
// 	char szName[100];
// 	char szDLL[MAX_PATH + 1];
// }ENFORCERSTATUS;


/*
*/
class CEDPMgr
{
public:
	static CEDPMgr& GetInstance();

	/*
	
	initialize.
	
	*/
	BOOL Init();
	
	/*
	
	un-initialize.
	
	*/
	void Quit();


	/*
	
	show product information window.
	
	*/
	void ShowProductInfo(int x, int y);


	/*
	
	show enforcer status window.
	
	*/
	void ShowEnforcerStatus();


	/*
	
	get celog.
	
	
	*/
	CELog& GetCELog()
	{
		if (!m_bLogInit)
		{
			if (edp_manager::CCommonUtilities::InitLog(m_log, EDPM_MODULE_MAIN))
			{
				m_bLogInit = TRUE;
				m_log.Log(CELOG_DEBUG, L"init log in EDP Manager succeed\n");
			}
		}
		return m_log;
	}

protected:
	CEDPMgr(void);
	~CEDPMgr(void);


protected:
	CProductInfoDlg* m_pInfoDlg;

	//	celog
	CELog m_log;
	BOOL m_bLogInit;
};
