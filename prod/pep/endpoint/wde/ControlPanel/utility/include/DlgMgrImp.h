#pragma once
#include "dlgmgr.h"

class CDlgMgrImp : public CDlgMgr
{
public:
	CDlgMgrImp(void);
	virtual ~CDlgMgrImp(void);

	virtual BOOL ShowModalDlg(MODAL_DLG_ID eDlgId);

	void RegDlgHandleForKB(HWND* pWnds, int nCount);

	BOOL GetDlgHandleForKB(HWND* phWnds, DWORD& dwCount);


private:
	vector<HWND> m_hWndsKB;
};
