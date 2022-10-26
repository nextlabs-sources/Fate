#pragma once

#include <atlhost.h>
#include "Resource.h"
#include "datatype.h"

#include "NL_ListView_ComboBox.h"
#include "NXTLBS_ListView.h"

class CFileTagMgr;
class CFileTagPanel:
	public CAxDialogImpl<CFileTagPanel>
{
public:
	CFileTagPanel(void);
	~CFileTagPanel(void);

	enum { IDD = IDD_FILETAGPANEL };

	BEGIN_MSG_MAP(CFileTagPanel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaintDialog)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC,OnCtrlColorsStatic) 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackGround)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		REFLECT_NOTIFICATIONS();
		CHAIN_MSG_MAP(CAxDialogImpl<CFileTagPanel>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaintDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
	
	static void CALLBACK OnClickOK(PVOID pData, LONG status, HWND hParent ) ;

public:
	BOOL CheckTagValues();//Kevin 2008-10-6
	BOOL PrepareItem(FILETAG_ITEM& item);
	void UpdatePannel();
	BOOL GetTagValues(std::list<smart_ptr<FILETAG_PAIR>>* pList);
	std::list<std::wstring> GetFiles();

	void DrawLine(int nLeft, int nTop, int nWidth);
	void UpdateButtons();
	void SetFileTagMgr(CFileTagMgr* pMgr){m_pMgr = pMgr;}
	CFileTagMgr* GetFileTagMgr(){return m_pMgr;};

	HIMAGELIST			m_hImglist;
	CNXTLBS_ListView	m_lvFile;
	CNL_ListView_ComboBox	m_lvTags;
	HFONT				m_hFontBold;
	HWND				m_hParentWnd;
	CFileTagMgr*		m_pMgr;
private:
	VOID InitHeaderStyle(VOID) ;
	void ShowEditScrollbar(DWORD dwID);
	CImage m_imgBackground ;
};
