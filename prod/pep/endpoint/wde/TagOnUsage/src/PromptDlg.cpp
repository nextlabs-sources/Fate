#include "StdAfx.h"
#include "PromptDlg.h"

CPromptDlg::CPromptDlg(void)
{
	m_bEnd = FALSE;
	m_strPathText = L"";
}

CPromptDlg::~CPromptDlg(void)
{
}

LRESULT CPromptDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CPromptDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	SetDlgItemText(IDC_FILEPATH, m_strPathText.c_str());

	SetTimer(0, 100, NULL);
	CenterWindow();

	
	return 0;
}

LRESULT CPromptDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(m_bEnd)
	{
		KillTimer(0);
		EndDialog(0);
	}
	return 0;
}

LRESULT CPromptDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}


